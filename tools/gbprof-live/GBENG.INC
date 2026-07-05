/***********************************************************************
 *
 * GBPROF v1.5j -- opcode and memory profiler for GBROW
 *
 * User modes:
 *   1 DIV+ROW8 FULL   normal game, draw every frame
 *   2 DIV+ROW8 FS2    normal game, draw 1 of 2
 *   3 DIV+ROW8 FS3    normal game, draw 1 of 3
 *   4 OPCODE CORE     no renderer/blit, opcode profiling enabled
 *   5 MEMORY CORE     no renderer/blit, data-memory profiling enabled
 *   6 BOTH FS2        opcode + memory profiling with FS2 display
 *   0 CLEAR SAMPLE    reset the current two-second profile sample
 *
 * Profiling intentionally measures frequencies, not host CPU time. The
 * counters add overhead, so FPS in modes 4-6 must not be compared directly
 * with the non-profiled GBFS build.
 *
 ***********************************************************************/

#include <geos.h>
#include <graphics.h>
#include <geode.h>
#include <heap.h>
#include <file.h>
#include <timer.h>
#include <input.h>
#include "GBENG.H"
#include "GBPDATA.H"

#define ENABLE_SOUND 0
#define ENABLE_LCD 1
#define PEANUT_GB_12_COLOUR 0
#define PEANUT_GB_HIGH_LCD_ACCURACY 0
#define PEANUT_GB_USE_INTRINSICS 0
#define PEANUT_GB_IS_LITTLE_ENDIAN 1
#define PGB_DISABLE_RTC 1
#define PGB_DISABLE_SERIAL 1

#define GB_WIDTH                    160
#define GB_HEIGHT                   144
#define GB_4BPP_BYTES_PER_LINE       80
#define GB_4BPP_FRAME_BYTES       11520
#define GB_MONO_BYTES_PER_LINE       20
#define GB_MONO_FRAME_BYTES        2880
#define GB_MAX_STEPS_PER_FRAME    30000

#define MODE_CORE                     1
#define MODE_ORIGINAL_PACK            2
#define MODE_TABLE_PACK               3
#define MODE_TABLE_FULL               4
#define MODE_BLIT_4BPP                5
#define MODE_DIV_CORE                 6
#define MODE_DIV_FULL                 7
#define MODE_ROW8_PACK                8
#define MODE_ROW8_FULL                9
#define MODE_DIV_ROW8_FULL           10
#define MODE_DIV_ROW8_FS2            11
#define MODE_DIV_ROW8_FS3            12
#define MODE_PROFILE_OPCODE_CORE      13
#define MODE_PROFILE_MEMORY_CORE      14
#define MODE_PROFILE_BOTH_FS2         15

#define STAGE_UI_READY                0
#define STAGE_ALLOC_4BPP              1
#define STAGE_ALLOC_MONO              2
#define STAGE_ALLOC_BANK0             3
#define STAGE_ALLOC_BANK1             4
#define STAGE_OPEN_FILE               5
#define STAGE_READ_BANK0              6
#define STAGE_READ_BANK1              7
#define STAGE_VALIDATE_ROM            8
#define STAGE_CALL_INIT               9
#define STAGE_FIRST_FRAME            10
#define STAGE_RUNNING                11
#define STAGE_FAILED                100

/*
 * These variables must be declared before PGBLCD.H because the fast direct
 * macros below are expanded inside the core.
 */
static byte *gbFastBank0 = NULL;
static byte *gbFastBank1 = NULL;
static Boolean gbRenderEnabled = TRUE;
static Boolean gbPackEnabled = TRUE;
static Boolean gbUseFastRenderer = FALSE;
static Boolean gbUseRow8Renderer = FALSE;
static Boolean gbDivFastEnabled = FALSE;
static Boolean gbDivSignatureValid = FALSE;
static dword gbDivFastHits = 0;

#define PGB_DIV_SUPER_ENABLED(gb) \
    (gbDivFastEnabled && gbDivSignatureValid)

#define PGB_DIV_SUPER_HIT() \
    (gbDivFastHits++)

/*
 * Keep these hooks as single expressions.  Borland 4.52 emits a scope
 * record for every do/while macro block; injecting those records into the
 * already enormous Peanut-GB switch can make the old GEOS glue linker lose
 * the owning scope and abort in borland.c.
 */
#define PGB_PROFILE_OPCODE(opcode)                                  \
    ((gbProfileOpcodes)                                              \
        ? GBProfileOpcodeHit((byte)(opcode))                         \
        : (void)0)

#define PGB_PROFILE_CB(opcode)                                      \
    ((gbProfileOpcodes)                                              \
        ? GBProfileCBHit((byte)(opcode))                             \
        : (void)0)

#define PGB_PROFILE_READ(address)                                   \
    ((gbProfileMemory)                                               \
        ? GBProfileReadAddress((word)(address))                      \
        : (void)0)

#define PGB_PROFILE_WRITE(address)                                  \
    ((gbProfileMemory)                                               \
        ? GBProfileWriteAddress((word)(address))                     \
        : (void)0)


struct gb_s;

static void GBCoreErrorDirect(struct gb_s *gb,
                              const int error,
                              const unsigned short address);
static void GBLcdLine(struct gb_s *gb,
                      const unsigned char *pixels,
                      const unsigned char line);
static void GBFastDrawLine(struct gb_s *gb);
static void GBTableDrawLine(struct gb_s *gb);
static void GBRow8DrawLine(struct gb_s *gb);

static byte GBFetch8Slow(struct gb_s *gb);
static byte GBPeek8Slow(struct gb_s *gb);
static byte GBStackPop8Profiled(struct gb_s *gb);
static void GBStackPush8Profiled(struct gb_s *gb, byte value);
static byte GBHighRead8Profiled(struct gb_s *gb, byte offset);


#define PGB_FETCH8(gb)                                               \
    (((gb)->cpu_reg.pc.reg < 0x4000)                                \
        ? gbFastBank0[(gb)->cpu_reg.pc.reg++]                        \
        : GBFetch8Slow((gb)))

#define PGB_PEEK8(gb)                                                \
    (((gb)->cpu_reg.pc.reg < 0x4000)                                \
        ? gbFastBank0[(gb)->cpu_reg.pc.reg]                          \
        : GBPeek8Slow((gb)))

#define PGB_STACK_POP8(gb) \
    GBStackPop8Profiled((gb))

#define PGB_STACK_PUSH8(gb, value) \
    GBStackPush8Profiled((gb), (byte)(value))

#define PGB_HIGH_READ8(gb, offset) \
    GBHighRead8Profiled((gb), (byte)(offset))

/*
 * Fast 32 KiB direct ROM access. FlappyBird.gb has two 16 KiB banks.
 * This removes a C function call from every ROM fetch/read.
 */
#define PGB_ROM_READ(gb, addr)                                      \
    (((unsigned long)(addr) < 0x4000UL)                             \
        ? gbFastBank0[(word)(addr)]                                 \
        : gbFastBank1[(word)((unsigned long)(addr) & 0x3FFFUL)])

#define PGB_CART_RAM_READ(gb, addr) (0xFF)
#define PGB_CART_RAM_WRITE(gb, addr, val) ((void)0)

#define PGB_REPORT_ERROR(gb, error, addr)                            \
    GBCoreErrorDirect((gb), (int)(error), (unsigned short)(addr))

/*
 * When MODE_CORE is active this becomes FALSE, so Peanut-GB returns from
 * __gb_draw_line before tile/sprite rendering begins.
 */
#define PGB_LCD_AVAILABLE(gb) (gbRenderEnabled)

#define PGB_LCD_DRAW_LINE(gb, pixels, line)                          \
    GBLcdLine((gb), (pixels), (unsigned char)(line))

#include "PGBROW.H"


static byte
GBFetch8Slow(struct gb_s *gb)
{
    word address;

    address = gb->cpu_reg.pc.reg++;

    if (address < 0x8000) {
        return gbFastBank1[address & 0x3FFF];
    }

    return __gb_read(gb, address);
}


static byte
GBPeek8Slow(struct gb_s *gb)
{
    word address;

    address = gb->cpu_reg.pc.reg;

    if (address < 0x8000) {
        return gbFastBank1[address & 0x3FFF];
    }

    return __gb_read(gb, address);
}



static byte
GBStackPop8Profiled(struct gb_s *gb)
{
    word address;

    address = gb->cpu_reg.sp.reg++;

    if ((address >= 0xC000) && (address < 0xE000)) {
        if (gbProfileMemory) {
            GBProfileReadAddress(address);
        }
        return gb->wram[address - 0xC000];
    }

    /* __gb_read performs the profile hook for the slow path. */
    return __gb_read(gb, address);
}


static void
GBStackPush8Profiled(struct gb_s *gb,
                     byte value)
{
    word address;

    address = --gb->cpu_reg.sp.reg;

    if ((address >= 0xC000) && (address < 0xE000)) {
        if (gbProfileMemory) {
            GBProfileWriteAddress(address);
        }
        gb->wram[address - 0xC000] = value;
        return;
    }

    /* __gb_write performs the profile hook for the slow path. */
    __gb_write(gb, address, value);
}


static byte
GBHighRead8Profiled(struct gb_s *gb,
                    byte offset)
{
    word address;

    address = (word)(0xFF00 | offset);

    if ((offset < 0x10) || (offset > 0x3F)) {
        if (gbProfileMemory) {
            GBProfileReadAddress(address);
        }
        return gb->hram_io[offset];
    }

    /* __gb_read performs the profile hook for the slow path. */
    return __gb_read(gb, address);
}



typedef struct {
    Bitmap header;
    byte pixels[GB_4BPP_FRAME_BYTES];
} GB4FrameBuffer;

typedef struct {
    Bitmap header;
    byte pixels[GB_MONO_FRAME_BYTES];
} GBMonoFrameBuffer;


/***********************************************************************
 * Runtime state.
 ***********************************************************************/

static struct gb_s gbCore;

static const byte gbShadeMap[4] = {
    C_WHITE,
    C_RED,
    C_GREEN,
    C_BLACK
};

/*
 * 4-pixel decoder:
 *   input index = high nibble from tile plane 0, low nibble from plane 1
 *   output word = four GEOS 4-bpp pixels (two bytes, little-endian)
 */
static word gbBgWordLUT[256];
static byte gbBgMaskLUT[256];
static byte gbLastBGP = 0xFF;

/*
 * Hardware-like first-ten sprite selection, prepared once at line zero.
 * 144 scanlines x 10 sprite indices = 1440 bytes.
 */
static byte gbSpriteCount[GB_HEIGHT];
static byte gbSpriteList[GB_HEIGHT * 10];

static WindowHandle gbWindow = 0;
static Boolean gbStarted = FALSE;
static Boolean gbRunning = FALSE;
static Boolean gbWorkQueued = FALSE;

static word gbStage = STAGE_UI_READY;
static word gbFailureCode = 0;
static enum gb_init_error_e gbInitResult = GB_INIT_INVALID_MAX;

static word gbMode = MODE_DIV_ROW8_FS2;
static byte gbSkipPhase = 0;
static word gbCoreErrors = 0;
static word gbLinesDrawn = 0;
static word gbSteps = 0;

static word gbMeasuredGuestFPS = 0;
static word gbMeasuredBlitFPS = 0;
static word gbGuestFramesInPeriod = 0;
static word gbBlitsInPeriod = 0;
static dword gbPeriodStart = 0;

static MemHandle gb4Handle = 0;
static GB4FrameBuffer *gb4Frame = NULL;

static MemHandle gbMonoHandle = 0;
static GBMonoFrameBuffer *gbMonoFrame = NULL;

static MemHandle gbBank0Handle = 0;
static MemHandle gbBank1Handle = 0;

static FileHandle gbRomFile = 0;
static Boolean gbDirPushed = FALSE;


/***********************************************************************
 * Helpers.
 ***********************************************************************/

static void
GBZeroBytes(byte *buffer, word count)
{
    while (count != 0) {
        *buffer++ = 0;
        count--;
    }
}


static void
GBWordToText(word value, char *buffer)
{
    char reverse[6];
    word count;
    word index;

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    count = 0;

    while ((value != 0) && (count < 5)) {
        reverse[count] = (char)('0' + (value % 10));
        value /= 10;
        count++;
    }

    for (index = 0; index < count; index++) {
        buffer[index] = reverse[count - index - 1];
    }

    buffer[count] = '\0';
}


static void
GBDwordToText(dword value, char *buffer)
{
    char reverse[11];
    word count;
    word index;

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    count = 0;

    while ((value != 0) && (count < 10)) {
        reverse[count] = (char)('0' + (value % 10L));
        value /= 10L;
        count++;
    }

    for (index = 0; index < count; index++) {
        buffer[index] = reverse[count - index - 1];
    }

    buffer[count] = '\0';
}



static const char *
GBBuildText(void)
{
#ifdef DO_ERROR_CHECKING
    return "EC";
#else
    return "NC";
#endif
}


static const char *
GBModeText(void)
{
    switch (gbMode) {
    case MODE_CORE:
        return "1 CORE";
    case MODE_ORIGINAL_PACK:
        return "2 ORIGINAL";
    case MODE_TABLE_PACK:
        return "3 TABLE PACK";
    case MODE_TABLE_FULL:
        return "4 TABLE FULL";
    case MODE_BLIT_4BPP:
        return "5 BLIT 4BPP";
    case MODE_DIV_CORE:
        return "6 DIV CORE";
    case MODE_DIV_FULL:
        return "7 DIV FULL";
    case MODE_ROW8_PACK:
        return "8 ROW8 PACK";
    case MODE_ROW8_FULL:
        return "9 ROW8 FULL";
    case MODE_DIV_ROW8_FULL:
        return "1 DIV+ROW8 FULL";
    case MODE_DIV_ROW8_FS2:
        return "2 DIV+ROW8 FS2";
    case MODE_DIV_ROW8_FS3:
        return "3 DIV+ROW8 FS3";
    case MODE_PROFILE_OPCODE_CORE:
        return "4 OPCODE CORE";
    case MODE_PROFILE_MEMORY_CORE:
        return "5 MEMORY CORE";
    case MODE_PROFILE_BOTH_FS2:
        return "6 BOTH FS2";
    }

    return "?";
}


static const char *
GBStageText(void)
{
    switch (gbStage) {
    case STAGE_UI_READY:
        return "UI READY";
    case STAGE_ALLOC_4BPP:
        return "ALLOC 4BPP";
    case STAGE_ALLOC_MONO:
        return "ALLOC MONO";
    case STAGE_ALLOC_BANK0:
        return "ALLOC BANK 0";
    case STAGE_ALLOC_BANK1:
        return "ALLOC BANK 1";
    case STAGE_OPEN_FILE:
        return "OPEN FLAPPY.GB";
    case STAGE_READ_BANK0:
        return "READ BANK 0";
    case STAGE_READ_BANK1:
        return "READ BANK 1";
    case STAGE_VALIDATE_ROM:
        return "VALIDATE ROM";
    case STAGE_CALL_INIT:
        return "CALL GB_INIT";
    case STAGE_FIRST_FRAME:
        return "FIRST FRAME";
    case STAGE_RUNNING:
        return "RUNNING";
    case STAGE_FAILED:
        return "FAILED";
    }

    return "UNKNOWN";
}


/***********************************************************************
 * Buffer initialization.
 ***********************************************************************/

static void
GBInit4Buffer(void)
{
    if (gb4Frame == NULL) {
        return;
    }

    gb4Frame->header.B_width = GB_WIDTH;
    gb4Frame->header.B_height = GB_HEIGHT;
    gb4Frame->header.B_compact = BMC_UNCOMPACTED;
    gb4Frame->header.B_type = BMF_4BIT;

    GBZeroBytes(gb4Frame->pixels, GB_4BPP_FRAME_BYTES);
}


static void
GBInitMonoBuffer(void)
{
    word y;
    word x;
    word rowBase;

    if (gbMonoFrame == NULL) {
        return;
    }

    gbMonoFrame->header.B_width = GB_WIDTH;
    gbMonoFrame->header.B_height = GB_HEIGHT;
    gbMonoFrame->header.B_compact = BMC_UNCOMPACTED;
    gbMonoFrame->header.B_type = BMF_MONO;

    /*
     * Static checker pattern. Polarity is irrelevant to the throughput test.
     */
    for (y = 0; y < GB_HEIGHT; y++) {
        rowBase = (word)(y * GB_MONO_BYTES_PER_LINE);

        for (x = 0; x < GB_MONO_BYTES_PER_LINE; x++) {
            gbMonoFrame->pixels[rowBase + x] =
                (byte)(((x + y) & 1) ? 0xAA : 0x55);
        }
    }
}


/***********************************************************************
 * Resource lifecycle.
 ***********************************************************************/

static void
GBCloseFileAndPath(void)
{
    if (gbRomFile != 0) {
        FileClose(gbRomFile, FALSE);
        gbRomFile = 0;
    }

    if (gbDirPushed) {
        FilePopDir();
        gbDirPushed = FALSE;
    }
}


static void
GBFreeMemory(void)
{
    if (gbBank1Handle != 0) {
        if (gbFastBank1 != NULL) {
            MemUnlock(gbBank1Handle);
        }

        gbFastBank1 = NULL;
        MemFree(gbBank1Handle);
        gbBank1Handle = 0;
    }

    if (gbBank0Handle != 0) {
        if (gbFastBank0 != NULL) {
            MemUnlock(gbBank0Handle);
        }

        gbFastBank0 = NULL;
        MemFree(gbBank0Handle);
        gbBank0Handle = 0;
    }

    if (gbMonoHandle != 0) {
        if (gbMonoFrame != NULL) {
            MemUnlock(gbMonoHandle);
        }

        gbMonoFrame = NULL;
        MemFree(gbMonoHandle);
        gbMonoHandle = 0;
    }

    if (gb4Handle != 0) {
        if (gb4Frame != NULL) {
            MemUnlock(gb4Handle);
        }

        gb4Frame = NULL;
        MemFree(gb4Handle);
        gb4Handle = 0;
    }
}


static void
GBFail(word code)
{
    gbFailureCode = code;
    gbStage = STAGE_FAILED;
    gbRunning = FALSE;
    gbWorkQueued = FALSE;

    GBCloseFileAndPath();
}


/***********************************************************************
 * Peanut-GB hooks.
 ***********************************************************************/

static void
GBCoreErrorDirect(struct gb_s *gb,
                  const int error,
                  const unsigned short address)
{
    (void)gb;
    (void)error;
    (void)address;

    gbCoreErrors++;
}


static void
GBLcdLine(struct gb_s *gb,
          const unsigned char *pixels,
          const unsigned char line)
{
    word x;
    word rowBase;
    byte left;
    byte right;

    (void)gb;

    if ((!gbPackEnabled) ||
        (gb4Frame == NULL) ||
        (line >= GB_HEIGHT)) {

        return;
    }

    rowBase = (word)((word)line * GB_4BPP_BYTES_PER_LINE);

    for (x = 0; x < GB_WIDTH; x += 2) {
        left = gbShadeMap[pixels[x] & LCD_COLOUR];
        right = gbShadeMap[pixels[x + 1] & LCD_COLOUR];

        gb4Frame->pixels[rowBase + (x >> 1)] =
            (byte)((left << 4) | right);
    }

    gbLinesDrawn++;
}



/***********************************************************************
 * Direct packed 4-bpp renderer.
 *
 * Differences from the stock Peanut renderer:
 *   - no temporary pixels[160] scanline;
 *   - no automatic 160-byte zero fill on every line;
 *   - background pixels are produced two at a time directly into GEOS
 *     4-bpp bytes;
 *   - sprites update the destination nibble in place;
 *   - a 20-byte bit mask preserves OBJ-behind-BG priority.
 *
 * It is intentionally conservative: window and sprite behaviour remain
 * compatible with the low-accuracy stock renderer used by this port.
 ***********************************************************************/

static void
GBBuildBackgroundLUT(struct gb_s *gb)
{
    word index;
    byte plane0;
    byte plane1;
    byte colour0;
    byte colour1;
    byte colour2;
    byte colour3;
    byte shade0;
    byte shade1;
    byte shade2;
    byte shade3;
    byte firstByte;
    byte secondByte;
    byte mask;

    for (index = 0; index < 256; index++) {
        plane0 = (byte)(index >> 4);
        plane1 = (byte)(index & 0x0F);

        colour0 =
            (byte)(((plane0 >> 3) & 1)
            | (((plane1 >> 3) & 1) << 1));

        colour1 =
            (byte)(((plane0 >> 2) & 1)
            | (((plane1 >> 2) & 1) << 1));

        colour2 =
            (byte)(((plane0 >> 1) & 1)
            | (((plane1 >> 1) & 1) << 1));

        colour3 =
            (byte)((plane0 & 1)
            | ((plane1 & 1) << 1));

        shade0 =
            gbShadeMap[gb->display.bg_palette[colour0] & LCD_COLOUR];
        shade1 =
            gbShadeMap[gb->display.bg_palette[colour1] & LCD_COLOUR];
        shade2 =
            gbShadeMap[gb->display.bg_palette[colour2] & LCD_COLOUR];
        shade3 =
            gbShadeMap[gb->display.bg_palette[colour3] & LCD_COLOUR];

        firstByte = (byte)((shade0 << 4) | shade1);
        secondByte = (byte)((shade2 << 4) | shade3);

        /*
         * The 486 is little-endian: low byte is written first.
         */
        gbBgWordLUT[index] =
            (word)((word)firstByte | ((word)secondByte << 8));

        mask = 0;

        if (colour0 != 0) {
            mask |= 0x08;
        }
        if (colour1 != 0) {
            mask |= 0x04;
        }
        if (colour2 != 0) {
            mask |= 0x02;
        }
        if (colour3 != 0) {
            mask |= 0x01;
        }

        gbBgMaskLUT[index] = mask;
    }

    gbLastBGP = gb->hram_io[IO_BGP];
}


static void
GBBuildSpriteLists(struct gb_s *gb)
{
    word y;
    word slot;
    word height;
    int sprite;
    int top;
    int bottom;
    int line;
    byte oy;
    byte ox;

    for (y = 0; y < GB_HEIGHT; y++) {
        gbSpriteCount[y] = 0;
    }

    height =
        (gb->hram_io[IO_LCDC] & LCDC_OBJ_SIZE) ? 16 : 8;

    /*
     * Scan OAM from low to high and retain the first ten sprites for each
     * line, which matches the Game Boy's line-selection limit.
     */
    for (sprite = 0; sprite < NUM_SPRITES; sprite++) {
        oy = gb->oam[4 * sprite + 0];
        ox = gb->oam[4 * sprite + 1];

        if ((ox == 0) || (ox >= 168)) {
            continue;
        }

        top = (int)oy - 16;
        bottom = top + (int)height;

        if ((bottom <= 0) || (top >= GB_HEIGHT)) {
            continue;
        }

        if (top < 0) {
            top = 0;
        }

        if (bottom > GB_HEIGHT) {
            bottom = GB_HEIGHT;
        }

        for (line = top; line < bottom; line++) {
            slot = gbSpriteCount[line];

            if (slot < 10) {
                gbSpriteList[(word)line * 10 + slot] =
                    (byte)sprite;
                gbSpriteCount[line] = (byte)(slot + 1);
            }
        }
    }
}


static void
GBTableDrawLine(struct gb_s *gb)
{
    byte bgMask[20];
    byte *row;
    word *rowWords;
    byte ly;
    byte lcdc;
    byte bgY;
    byte scx;
    byte bgX;
    byte offset;
    byte available;
    byte needed;
    byte plane0;
    byte plane1;
    byte next0;
    byte next1;
    byte lowMask;
    byte index;
    byte mask4;
    byte idx;
    byte py;
    byte bit;
    byte colour;
    byte shade;
    byte maskByte;
    byte oy;
    byte ox;
    byte ot;
    byte of;
    byte t1;
    byte t2;
    byte sourceBit;
    byte spriteCount;
    byte spriteNumber;
    word bgMap;
    word winMap;
    word tile;
    word nextTile;
    word rowBase;
    word x;
    word i;
    word startX;
    word winX;
    int listPos;
    int sx;

    if ((!gbRenderEnabled) || (gb4Frame == NULL)) {
        return;
    }

    if (gb->direct.frame_skip && !gb->display.frame_skip_count) {
        return;
    }

    ly = gb->hram_io[IO_LY];

    if (ly >= GB_HEIGHT) {
        return;
    }

    lcdc = gb->hram_io[IO_LCDC];

    if (gbLastBGP != gb->hram_io[IO_BGP]) {
        GBBuildBackgroundLUT(gb);
    }

    if (ly == 0) {
        GBBuildSpriteLists(gb);
    }

    rowBase = (word)((word)ly * GB_4BPP_BYTES_PER_LINE);
    row = &gb4Frame->pixels[rowBase];
    rowWords = (word *)row;

    for (i = 0; i < 20; i++) {
        bgMask[i] = 0;
    }

    /*
     * Background in 40 groups of four pixels.
     */
    if (lcdc & LCDC_BG_ENABLE) {
        bgY = (byte)(ly + gb->hram_io[IO_SCY]);
        scx = gb->hram_io[IO_SCX];

        bgMap =
            ((lcdc & LCDC_BG_MAP) ? VRAM_BMAP_2 : VRAM_BMAP_1)
            + ((word)(bgY >> 3) * 0x20);

        for (x = 0; x < GB_WIDTH; x += 4) {
            bgX = (byte)(x + scx);
            offset = (byte)(bgX & 7);

            idx = gb->vram[bgMap + (bgX >> 3)];

            if (lcdc & LCDC_TILE_SELECT) {
                tile = VRAM_TILES_1 + ((word)idx << 4);
            } else {
                tile = VRAM_TILES_2
                     + ((word)((byte)(idx + 0x80)) << 4);
            }

            tile += (word)((bgY & 7) << 1);
            t1 = gb->vram[tile];
            t2 = gb->vram[tile + 1];

            if (offset <= 4) {
                plane0 =
                    (byte)((t1 >> (4 - offset)) & 0x0F);
                plane1 =
                    (byte)((t2 >> (4 - offset)) & 0x0F);
            } else {
                available = (byte)(8 - offset);
                needed = (byte)(4 - available);
                lowMask = (byte)((1 << available) - 1);

                idx =
                    gb->vram[bgMap
                    + (((byte)(bgX + available)) >> 3)];

                if (lcdc & LCDC_TILE_SELECT) {
                    nextTile =
                        VRAM_TILES_1 + ((word)idx << 4);
                } else {
                    nextTile =
                        VRAM_TILES_2
                        + ((word)((byte)(idx + 0x80)) << 4);
                }

                nextTile += (word)((bgY & 7) << 1);
                next0 = gb->vram[nextTile];
                next1 = gb->vram[nextTile + 1];

                plane0 =
                    (byte)(((t1 & lowMask) << needed)
                    | (next0 >> (8 - needed)));

                plane1 =
                    (byte)(((t2 & lowMask) << needed)
                    | (next1 >> (8 - needed)));
            }

            index = (byte)((plane0 << 4) | plane1);
            rowWords[x >> 2] = gbBgWordLUT[index];

            mask4 = gbBgMaskLUT[index];

            if (x & 4) {
                bgMask[x >> 3] |= mask4;
            } else {
                bgMask[x >> 3] = (byte)(mask4 << 4);
            }
        }
    } else {
        shade =
            gbShadeMap[gb->display.bg_palette[0] & LCD_COLOUR];
        index = (byte)((shade << 4) | shade);

        for (i = 0; i < GB_4BPP_BYTES_PER_LINE; i++) {
            row[i] = index;
        }
    }

    /*
     * Window remains a conservative pixel path. Most gameplay frames in the
     * test ROM do not use it, while this preserves correctness for menus.
     */
    if ((lcdc & LCDC_WINDOW_ENABLE) &&
        (ly >= gb->display.WY) &&
        (gb->hram_io[IO_WX] <= 166)) {

        startX =
            (gb->hram_io[IO_WX] < 7)
                ? 0
                : (word)(gb->hram_io[IO_WX] - 7);

        winMap =
            (lcdc & LCDC_WINDOW_MAP) ? VRAM_BMAP_2 : VRAM_BMAP_1;

        winMap +=
            (word)((gb->display.window_clear >> 3) * 0x20);

        py = (byte)(gb->display.window_clear & 7);

        for (x = startX; x < GB_WIDTH; x++) {
            winX = x - startX;
            idx = gb->vram[winMap + (winX >> 3)];

            if (lcdc & LCDC_TILE_SELECT) {
                tile = VRAM_TILES_1 + ((word)idx << 4);
            } else {
                tile = VRAM_TILES_2
                     + ((word)((byte)(idx + 0x80)) << 4);
            }

            tile += (word)(py << 1);
            bit = (byte)(7 - (winX & 7));

            colour =
                (byte)(((gb->vram[tile] >> bit) & 1)
                | (((gb->vram[tile + 1] >> bit) & 1) << 1));

            shade =
                gbShadeMap[
                    gb->display.bg_palette[colour] & LCD_COLOUR
                ];

            if (x & 1) {
                row[x >> 1] =
                    (byte)((row[x >> 1] & 0xF0) | shade);
            } else {
                row[x >> 1] =
                    (byte)((row[x >> 1] & 0x0F) | (shade << 4));
            }

            maskByte = (byte)(0x80 >> (x & 7));

            if (colour != 0) {
                bgMask[x >> 3] |= maskByte;
            } else {
                bgMask[x >> 3] &= (byte)(~maskByte);
            }
        }

        gb->display.window_clear++;
    }

    /*
     * Only the preselected sprites touching this scanline are visited.
     */
    if (lcdc & LCDC_OBJ_ENABLE) {
        spriteCount = gbSpriteCount[ly];

        for (listPos = (int)spriteCount - 1;
             listPos >= 0;
             listPos--) {

            spriteNumber =
                gbSpriteList[(word)ly * 10 + (word)listPos];

            oy = gb->oam[4 * spriteNumber + 0];
            ox = gb->oam[4 * spriteNumber + 1];
            ot = (byte)(gb->oam[4 * spriteNumber + 2]
                 & ((lcdc & LCDC_OBJ_SIZE) ? 0xFE : 0xFF));
            of = gb->oam[4 * spriteNumber + 3];

            py = (byte)(ly - oy + 16);

            if (of & OBJ_FLIP_Y) {
                py =
                    (byte)(((lcdc & LCDC_OBJ_SIZE) ? 15 : 7) - py);
            }

            tile =
                VRAM_TILES_1
                + ((word)ot << 4)
                + ((word)py << 1);

            t1 = gb->vram[tile];
            t2 = gb->vram[tile + 1];

            for (i = 0; i < 8; i++) {
                sx = (int)ox - 8 + (int)i;

                if ((sx < 0) || (sx >= GB_WIDTH)) {
                    continue;
                }

                sourceBit =
                    (of & OBJ_FLIP_X)
                        ? (byte)i
                        : (byte)(7 - i);

                colour =
                    (byte)(((t1 >> sourceBit) & 1)
                    | (((t2 >> sourceBit) & 1) << 1));

                if (colour == 0) {
                    continue;
                }

                maskByte = (byte)(0x80 >> (sx & 7));

                if ((of & OBJ_PRIORITY) &&
                    (bgMask[(word)sx >> 3] & maskByte)) {

                    continue;
                }

                if (of & OBJ_PALETTE) {
                    shade =
                        gbShadeMap[
                            gb->display.sp_palette[colour + 4]
                            & LCD_COLOUR
                        ];
                } else {
                    shade =
                        gbShadeMap[
                            gb->display.sp_palette[colour]
                            & LCD_COLOUR
                        ];
                }

                if (sx & 1) {
                    row[(word)sx >> 1] =
                        (byte)((row[(word)sx >> 1] & 0xF0)
                               | shade);
                } else {
                    row[(word)sx >> 1] =
                        (byte)((row[(word)sx >> 1] & 0x0F)
                               | (shade << 4));
                }
            }
        }
    }

    gbLinesDrawn++;
}



/*
 * Load one background tile row from the current map column.
 * The caller supplies all temporaries so the macro stays in the hot loop.
 */
#define GB_ROW8_LOAD_TILE_ROW(column, outPlane0, outPlane1)           \
    do {                                                              \
        idx = mapRow[(column)];                                       \
        tile = tileDataBase                                           \
             + ((word)((byte)(idx ^ tileIndexXor)) << 4)             \
             + tileRowOffset;                                         \
        (outPlane0) = vram[tile];                                    \
        (outPlane1) = vram[tile + 1];                                \
    } while (0)

/*
 * Eight-pixel scanline loop specialized by a compile-time scroll offset.
 * Twenty iterations produce the complete 160-pixel background.
 */
#define GB_ROW8_RENDER_SHIFT(shiftValue)                              \
    do {                                                              \
        mapColumn = (byte)(scx >> 3);                                \
        outWords = rowWords;                                          \
        maskOut = bgMask;                                             \
        GB_ROW8_LOAD_TILE_ROW(mapColumn, t1, t2);                     \
        for (i = 0; i < 20; i++) {                                   \
            mapColumn = (byte)((mapColumn + 1) & 31);                \
            GB_ROW8_LOAD_TILE_ROW(mapColumn, next0, next1);           \
            plane0 = (byte)(((word)t1 << (shiftValue))               \
                     | ((word)next0 >> (8 - (shiftValue))));         \
            plane1 = (byte)(((word)t2 << (shiftValue))               \
                     | ((word)next1 >> (8 - (shiftValue))));         \
            index = (byte)((plane0 & 0xF0) | (plane1 >> 4));         \
            *outWords++ = gbBgWordLUT[index];                         \
            maskHigh = (byte)(gbBgMaskLUT[index] << 4);              \
            index = (byte)((plane0 << 4) | (plane1 & 0x0F));         \
            *outWords++ = gbBgWordLUT[index];                         \
            *maskOut++ = (byte)(maskHigh | gbBgMaskLUT[index]);      \
            t1 = next0;                                               \
            t2 = next1;                                               \
        }                                                             \
    } while (0)


static void
GBRow8DrawLine(struct gb_s *gb)
{
    byte bgMask[20];
    byte *row;
    byte *vram;
    byte *mapRow;
    byte *maskOut;
    word *rowWords;
    word *outWords;
    byte ly;
    byte lcdc;
    byte bgY;
    byte scx;
    byte offset;
    byte plane0;
    byte plane1;
    byte next0;
    byte next1;
    byte index;
    byte maskHigh;
    byte idx;
    byte mapColumn;
    byte tileIndexXor;
    byte py;
    byte bit;
    byte colour;
    byte shade;
    byte maskByte;
    byte oy;
    byte ox;
    byte ot;
    byte of;
    byte t1;
    byte t2;
    byte sourceBit;
    byte spriteCount;
    byte spriteNumber;
    word bgMap;
    word winMap;
    word tile;
    word tileDataBase;
    word tileRowOffset;
    word rowBase;
    word x;
    word i;
    word startX;
    word winX;
    int listPos;
    int sx;

    if ((!gbRenderEnabled) || (gb4Frame == NULL)) {
        return;
    }

    if (gb->direct.frame_skip && !gb->display.frame_skip_count) {
        return;
    }

    ly = gb->hram_io[IO_LY];

    if (ly >= GB_HEIGHT) {
        return;
    }

    lcdc = gb->hram_io[IO_LCDC];

    if (gbLastBGP != gb->hram_io[IO_BGP]) {
        GBBuildBackgroundLUT(gb);
    }

    if (ly == 0) {
        GBBuildSpriteLists(gb);
    }

    rowBase = (word)((word)ly * GB_4BPP_BYTES_PER_LINE);
    row = &gb4Frame->pixels[rowBase];
    rowWords = (word *)row;
    vram = gb->vram;

    /*
     * Background in twenty groups of eight pixels.
     *
     * SCX & 7 is constant for the whole scanline. Select one expanded loop
     * before entering the hot path. Each iteration loads only the next tile
     * row, reuses the previous row and writes all eight pixels as two words.
     */
    if (lcdc & LCDC_BG_ENABLE) {
        bgY = (byte)(ly + gb->hram_io[IO_SCY]);
        scx = gb->hram_io[IO_SCX];
        offset = (byte)(scx & 7);
        tileRowOffset = (word)((bgY & 7) << 1);

        bgMap =
            ((lcdc & LCDC_BG_MAP) ? VRAM_BMAP_2 : VRAM_BMAP_1)
            + ((word)(bgY >> 3) * 0x20);

        mapRow = &vram[bgMap];

        if (lcdc & LCDC_TILE_SELECT) {
            tileDataBase = VRAM_TILES_1;
            tileIndexXor = 0;
        } else {
            tileDataBase = VRAM_TILES_2;
            tileIndexXor = 0x80;
        }

        switch (offset) {
        case 0:
            GB_ROW8_RENDER_SHIFT(0);
            break;
        case 1:
            GB_ROW8_RENDER_SHIFT(1);
            break;
        case 2:
            GB_ROW8_RENDER_SHIFT(2);
            break;
        case 3:
            GB_ROW8_RENDER_SHIFT(3);
            break;
        case 4:
            GB_ROW8_RENDER_SHIFT(4);
            break;
        case 5:
            GB_ROW8_RENDER_SHIFT(5);
            break;
        case 6:
            GB_ROW8_RENDER_SHIFT(6);
            break;
        default:
            GB_ROW8_RENDER_SHIFT(7);
            break;
        }
    } else {
        shade =
            gbShadeMap[gb->display.bg_palette[0] & LCD_COLOUR];
        index = (byte)((shade << 4) | shade);

        for (i = 0; i < GB_4BPP_BYTES_PER_LINE; i++) {
            row[i] = index;
        }

        for (i = 0; i < 20; i++) {
            bgMask[i] = 0;
        }
    }

    /*
     * Window remains a conservative pixel path. Most gameplay frames in the
     * test ROM do not use it, while this preserves correctness for menus.
     */
    if ((lcdc & LCDC_WINDOW_ENABLE) &&
        (ly >= gb->display.WY) &&
        (gb->hram_io[IO_WX] <= 166)) {

        startX =
            (gb->hram_io[IO_WX] < 7)
                ? 0
                : (word)(gb->hram_io[IO_WX] - 7);

        winMap =
            (lcdc & LCDC_WINDOW_MAP) ? VRAM_BMAP_2 : VRAM_BMAP_1;

        winMap +=
            (word)((gb->display.window_clear >> 3) * 0x20);

        py = (byte)(gb->display.window_clear & 7);

        for (x = startX; x < GB_WIDTH; x++) {
            winX = x - startX;
            idx = vram[winMap + (winX >> 3)];

            if (lcdc & LCDC_TILE_SELECT) {
                tile = VRAM_TILES_1 + ((word)idx << 4);
            } else {
                tile = VRAM_TILES_2
                     + ((word)((byte)(idx + 0x80)) << 4);
            }

            tile += (word)(py << 1);
            bit = (byte)(7 - (winX & 7));

            colour =
                (byte)(((vram[tile] >> bit) & 1)
                | (((vram[tile + 1] >> bit) & 1) << 1));

            shade =
                gbShadeMap[
                    gb->display.bg_palette[colour] & LCD_COLOUR
                ];

            if (x & 1) {
                row[x >> 1] =
                    (byte)((row[x >> 1] & 0xF0) | shade);
            } else {
                row[x >> 1] =
                    (byte)((row[x >> 1] & 0x0F) | (shade << 4));
            }

            maskByte = (byte)(0x80 >> (x & 7));

            if (colour != 0) {
                bgMask[x >> 3] |= maskByte;
            } else {
                bgMask[x >> 3] &= (byte)(~maskByte);
            }
        }

        gb->display.window_clear++;
    }

    /*
     * Only the preselected sprites touching this scanline are visited.
     */
    if (lcdc & LCDC_OBJ_ENABLE) {
        spriteCount = gbSpriteCount[ly];

        for (listPos = (int)spriteCount - 1;
             listPos >= 0;
             listPos--) {

            spriteNumber =
                gbSpriteList[(word)ly * 10 + (word)listPos];

            oy = gb->oam[4 * spriteNumber + 0];
            ox = gb->oam[4 * spriteNumber + 1];
            ot = (byte)(gb->oam[4 * spriteNumber + 2]
                 & ((lcdc & LCDC_OBJ_SIZE) ? 0xFE : 0xFF));
            of = gb->oam[4 * spriteNumber + 3];

            py = (byte)(ly - oy + 16);

            if (of & OBJ_FLIP_Y) {
                py =
                    (byte)(((lcdc & LCDC_OBJ_SIZE) ? 15 : 7) - py);
            }

            tile =
                VRAM_TILES_1
                + ((word)ot << 4)
                + ((word)py << 1);

            t1 = vram[tile];
            t2 = vram[tile + 1];

            for (i = 0; i < 8; i++) {
                sx = (int)ox - 8 + (int)i;

                if ((sx < 0) || (sx >= GB_WIDTH)) {
                    continue;
                }

                sourceBit =
                    (of & OBJ_FLIP_X)
                        ? (byte)i
                        : (byte)(7 - i);

                colour =
                    (byte)(((t1 >> sourceBit) & 1)
                    | (((t2 >> sourceBit) & 1) << 1));

                if (colour == 0) {
                    continue;
                }

                maskByte = (byte)(0x80 >> (sx & 7));

                if ((of & OBJ_PRIORITY) &&
                    (bgMask[(word)sx >> 3] & maskByte)) {

                    continue;
                }

                if (of & OBJ_PALETTE) {
                    shade =
                        gbShadeMap[
                            gb->display.sp_palette[colour + 4]
                            & LCD_COLOUR
                        ];
                } else {
                    shade =
                        gbShadeMap[
                            gb->display.sp_palette[colour]
                            & LCD_COLOUR
                        ];
                }

                if (sx & 1) {
                    row[(word)sx >> 1] =
                        (byte)((row[(word)sx >> 1] & 0xF0)
                               | shade);
                } else {
                    row[(word)sx >> 1] =
                        (byte)((row[(word)sx >> 1] & 0x0F)
                               | (shade << 4));
                }
            }
        }
    }

    gbLinesDrawn++;
}


static void
GBFastDrawLine(struct gb_s *gb)
{
    if (gbUseRow8Renderer) {
        GBRow8DrawLine(gb);
    } else {
        GBTableDrawLine(gb);
    }
}


#undef GB_ROW8_RENDER_SHIFT
#undef GB_ROW8_LOAD_TILE_ROW



static Boolean
GBValidateDivSignature(void)
{
    static const byte expected[36] = {
        0xF5, 0xCB, 0x15, 0xCB, 0x14, 0xCB, 0x11, 0xCB, 0x10, 0xC5, 0x79, 0x9B, 0x4F, 0x78, 0x9A, 0x47, 0x3F, 0x38, 0x08, 0xC1, 0xF1, 0x3D, 0xB7, 0x20, 0xE7, 0x18, 0x09, 0x33, 0x33, 0xF1, 0x3D, 0x37, 0x20, 0xDE, 0x18, 0x00
    };
    word index;

    if (gbFastBank0 == NULL) {
        return FALSE;
    }

    for (index = 0; index < 36; index++) {
        if (gbFastBank0[0x23CE + index] != expected[index]) {
            return FALSE;
        }
    }

    return TRUE;
}


/***********************************************************************
 * Guest execution and measurement.
 ***********************************************************************/

static Boolean
GBRunOneFrameSafe(void)
{
    gbCore.gb_frame = 0;
    gbLinesDrawn = 0;

    for (gbSteps = 0;
         gbSteps < GB_MAX_STEPS_PER_FRAME;
         gbSteps++) {

        __gb_step_cpu(&gbCore);

        if (gbCore.gb_frame) {
            return TRUE;
        }
    }

    return FALSE;
}


static Boolean
GBRunOneFrameFast(void)
{
    gbLinesDrawn = 0;
    gbSteps = 0;

    gb_run_frame(&gbCore);
    return TRUE;
}



static void
GBResetMeasurement(void)
{
    gbMeasuredGuestFPS = 0;
    gbMeasuredBlitFPS = 0;
    gbGuestFramesInPeriod = 0;
    gbBlitsInPeriod = 0;
    gbPeriodStart = TimerGetCount();
    GBProfileReset(gbDivFastHits);
}


static Boolean
GBUpdateMeasurement(void)
{
    dword now;
    dword elapsed;

    now = TimerGetCount();
    elapsed = now - gbPeriodStart;

    /* Two-second sample window. PC/GEOS timer ticks at 60 Hz. */
    if (elapsed < 120L) {
        return FALSE;
    }

    gbMeasuredGuestFPS =
        (word)(((dword)gbGuestFramesInPeriod * 60L) / elapsed);

    gbMeasuredBlitFPS =
        (word)(((dword)gbBlitsInPeriod * 60L) / elapsed);

    GBProfileSnapshot(elapsed, gbDivFastHits);

    gbGuestFramesInPeriod = 0;
    gbBlitsInPeriod = 0;
    gbPeriodStart = now;

    return TRUE;
}


/***********************************************************************
 * Drawing. The dynamic path never clears the whole window.
 ***********************************************************************/

static void
GBDrawStaticScreen(GStateHandle gstate)
{
    GrSetAreaColor(gstate, CF_INDEX, C_WHITE, 0, 0);
    GrFillRect(gstate, 0, 0, 559, 199);

    GrSetTextColor(gstate, CF_INDEX, C_BLACK, 0, 0);
    GrSetFont(gstate, FID_DTC_URW_SANS, MakeWWFixed(11.0));
    GrDrawText(gstate, 8, 8, "GB PROFILER v1.5j", 0);

    if (gb4Frame != NULL) {
        GrDrawImage(gstate,
                    8,
                    32,
                    IBS_1,
                    (const Bitmap *)gb4Frame);
    }

    GrSetLineColor(gstate, CF_INDEX, C_BLACK, 0, 0);
    GrDrawRect(gstate, 7, 31, 168, 176);

    GrSetFont(gstate, FID_DTC_URW_SANS, MakeWWFixed(8.0));

    GrDrawText(gstate, 178, 28, "BUILD:", 0);
    GrDrawText(gstate, 178, 44, "STATE:", 0);
    GrDrawText(gstate, 178, 60, "MODE:", 0);
    GrDrawText(gstate, 178, 76, "PROFILE:", 0);
    GrDrawText(gstate, 178, 92, "GUEST FPS:", 0);
    GrDrawText(gstate, 178, 108, "BLIT FPS:", 0);
    GrDrawText(gstate, 178, 124, "OP/s:", 0);
    GrDrawText(gstate, 178, 140, "READ/s:", 0);
    GrDrawText(gstate, 178, 156, "WRITE/s:", 0);
    GrDrawText(gstate, 178, 172, "DIV/s:", 0);
    GrDrawText(gstate, 178, 188, "CORE ERR:", 0);
}


static void
GBDrawDynamicStatus(GStateHandle gstate)
{
    char text[11];

    /* Clear values and the profiler panel, not the game bitmap. */
    GrSetAreaColor(gstate, CF_INDEX, C_WHITE, 0, 0);
    GrFillRect(gstate, 238, 0, 559, 199);

    GrSetTextColor(gstate, CF_INDEX, C_BLACK, 0, 0);
    GrSetFont(gstate, FID_DTC_URW_SANS, MakeWWFixed(8.0));

    GrDrawText(gstate, 240, 28, GBBuildText(), 0);
    GrDrawText(gstate, 240, 44, GBStageText(), 0);
    GrDrawText(gstate, 240, 60, GBModeText(), 0);
    GrDrawText(gstate, 240, 76, GBProfileText(), 0);

    GBWordToText(gbMeasuredGuestFPS, text);
    GrDrawText(gstate, 278, 92, text, 0);

    GBWordToText(gbMeasuredBlitFPS, text);
    GrDrawText(gstate, 278, 108, text, 0);

    GBDwordToText(gbMeasuredOpsPerSec, text);
    GrDrawText(gstate, 260, 124, text, 0);

    GBDwordToText(gbMeasuredReadsPerSec, text);
    GrDrawText(gstate, 260, 140, text, 0);

    GBDwordToText(gbMeasuredWritesPerSec, text);
    GrDrawText(gstate, 260, 156, text, 0);

    GBDwordToText(gbMeasuredDivPerSec, text);
    GrDrawText(gstate, 260, 172, text, 0);

    GBWordToText(gbCoreErrors, text);
    GrDrawText(gstate, 278, 188, text, 0);

    GBProfileDrawPanel(gstate);

}


static void
GBRedrawAll(void)
{
    GStateHandle gstate;

    if (gbWindow == 0) {
        return;
    }

    gstate = GrCreateState(gbWindow);
    GBDrawStaticScreen(gstate);
    GBDrawDynamicStatus(gstate);
    GrDestroyState(gstate);
}


static void
GBRedrawStatus(void)
{
    GStateHandle gstate;

    if (gbWindow == 0) {
        return;
    }

    gstate = GrCreateState(gbWindow);
    GBDrawDynamicStatus(gstate);
    GrDestroyState(gstate);
}


static void
GBBlit4(void)
{
    GStateHandle gstate;

    if ((gbWindow == 0) || (gb4Frame == NULL)) {
        return;
    }

    gstate = GrCreateState(gbWindow);
    GrDrawImage(gstate,
                8,
                32,
                IBS_1,
                (const Bitmap *)gb4Frame);
    GrDestroyState(gstate);
}


static void
GBBlitMono(void)
{
    GStateHandle gstate;

    if ((gbWindow == 0) || (gbMonoFrame == NULL)) {
        return;
    }

    gstate = GrCreateState(gbWindow);
    GrDrawImage(gstate,
                8,
                32,
                IBS_1,
                (const Bitmap *)gbMonoFrame);
    GrDestroyState(gstate);
}


/***********************************************************************
 * Core reset.
 ***********************************************************************/

static Boolean
GBResetCore(void)
{
    GBZeroBytes((byte *)&gbCore, (word)sizeof(gbCore));

    gbInitResult =
        gb_init(&gbCore, NULL, NULL, NULL, NULL, NULL);

    if (gbInitResult != GB_INIT_NO_ERROR) {
        return FALSE;
    }

    gb_init_lcd(&gbCore, NULL);
    gbCore.direct.joypad_state.joypad = 0xFF;

    gbCoreErrors = 0;
    return TRUE;
}


/***********************************************************************
 * Frame-skip phase. The guest always executes a complete frame. Only
 * renderer and GEOS transfer are suppressed on skipped display frames.
 ***********************************************************************/

static Boolean
GBTakeDisplayFrame(byte divisor)
{
    Boolean renderNow;

    renderNow = (gbSkipPhase == 0);

    gbSkipPhase++;
    if (gbSkipPhase >= divisor) {
        gbSkipPhase = 0;
    }

    return renderNow;
}


/***********************************************************************
 * Work scheduling.
 ***********************************************************************/

Boolean
GBEngineRequestWork(void)
{
    if ((!gbWorkQueued) &&
        gbRunning &&
        (gbWindow != 0) &&
        (gbStage == STAGE_RUNNING)) {

        gbWorkQueued = TRUE;
        return TRUE;
    }

    return FALSE;
}


Boolean
GBEngineNeedNextStage(void)
{
    return (Boolean)(gbStarted &&
                     (gbWindow != 0) &&
                     (gbStage != STAGE_RUNNING) &&
                     (gbStage != STAGE_FAILED));
}


static void
GBSetMode(word mode)
{
    if ((mode < MODE_CORE) ||
        (mode > MODE_PROFILE_BOTH_FS2)) {

        return;
    }

    gbMode = mode;

    gbUseRow8Renderer =
        ((mode == MODE_ROW8_PACK) ||
         (mode == MODE_ROW8_FULL) ||
         (mode == MODE_DIV_ROW8_FULL) ||
         (mode == MODE_DIV_ROW8_FS2) ||
         (mode == MODE_DIV_ROW8_FS3) ||
         (mode == MODE_PROFILE_BOTH_FS2));

    gbDivFastEnabled =
        ((mode == MODE_DIV_CORE) ||
         (mode == MODE_DIV_FULL) ||
         (mode == MODE_DIV_ROW8_FULL) ||
         (mode == MODE_DIV_ROW8_FS2) ||
         (mode == MODE_DIV_ROW8_FS3) ||
         (mode == MODE_PROFILE_OPCODE_CORE) ||
         (mode == MODE_PROFILE_MEMORY_CORE) ||
         (mode == MODE_PROFILE_BOTH_FS2));

    gbProfileOpcodes =
        ((mode == MODE_PROFILE_OPCODE_CORE) ||
         (mode == MODE_PROFILE_BOTH_FS2));

    gbProfileMemory =
        ((mode == MODE_PROFILE_MEMORY_CORE) ||
         (mode == MODE_PROFILE_BOTH_FS2));

    gbDivFastHits = 0;
    gbSkipPhase = 0;

    GBResetMeasurement();
    GBRedrawAll();
}


/***********************************************************************
 * Staged startup.
 ***********************************************************************/

void
GBEngineExpose(WindowHandle win, GStateHandle gstate)
{
    gbWindow = win;

    GBDrawStaticScreen(gstate);
    GBDrawDynamicStatus(gstate);

    if (!gbStarted) {
        gbStarted = TRUE;
    } else if (gbStage == STAGE_RUNNING) {
        gbRunning = TRUE;
        GBResetMeasurement();
    }
}


void
GBEngineNextStage(void)
{
    word bytesRead;

    if ((gbWindow == 0) ||
        (gbStage == STAGE_RUNNING) ||
        (gbStage == STAGE_FAILED)) {

        return;
    }

    gbStage++;
    GBRedrawStatus();

    switch (gbStage) {
    case STAGE_ALLOC_4BPP:
        gb4Handle =
            MemAlloc((word)sizeof(GB4FrameBuffer),
                     HF_SWAPABLE,
                     HAF_STANDARD);

        if (gb4Handle == 0) {
            GBFail(8);
            break;
        }

        gb4Frame =
            (GB4FrameBuffer *)MemLock(gb4Handle);

        if (gb4Frame == NULL) {
            GBFail(9);
            break;
        }

        GBInit4Buffer();
        break;

    case STAGE_ALLOC_MONO:
        gbMonoHandle =
            MemAlloc((word)sizeof(GBMonoFrameBuffer),
                     HF_SWAPABLE,
                     HAF_STANDARD);

        if (gbMonoHandle == 0) {
            GBFail(6);
            break;
        }

        gbMonoFrame =
            (GBMonoFrameBuffer *)MemLock(gbMonoHandle);

        if (gbMonoFrame == NULL) {
            GBFail(7);
            break;
        }

        GBInitMonoBuffer();
        break;

    case STAGE_ALLOC_BANK0:
        gbBank0Handle =
            MemAlloc(16384, HF_SWAPABLE, HAF_STANDARD);

        if (gbBank0Handle == 0) {
            GBFail(10);
            break;
        }

        gbFastBank0 = (byte *)MemLock(gbBank0Handle);

        if (gbFastBank0 == NULL) {
            GBFail(11);
        }
        break;

    case STAGE_ALLOC_BANK1:
        gbBank1Handle =
            MemAlloc(16384, HF_SWAPABLE, HAF_STANDARD);

        if (gbBank1Handle == 0) {
            GBFail(12);
            break;
        }

        gbFastBank1 = (byte *)MemLock(gbBank1Handle);

        if (gbFastBank1 == NULL) {
            GBFail(13);
        }
        break;

    case STAGE_OPEN_FILE:
        FilePushDir();
        gbDirPushed = TRUE;

        FileSetStandardPath(SP_APPLICATION);

        gbRomFile =
            FileOpen("ExtrApps\\FLAPPY.GB",
                     FILE_ACCESS_R | FILE_DENY_W);

        if (gbRomFile == 0) {
            GBFail(20);
        }
        break;

    case STAGE_READ_BANK0:
        bytesRead =
            FileRead(gbRomFile, gbFastBank0, 16384, FALSE);

        if (bytesRead != 16384) {
            GBFail(21);
        }
        break;

    case STAGE_READ_BANK1:
        bytesRead =
            FileRead(gbRomFile, gbFastBank1, 16384, FALSE);

        GBCloseFileAndPath();

        if (bytesRead != 16384) {
            GBFail(22);
        }
        break;

    case STAGE_VALIDATE_ROM:
        if ((gbFastBank0[0x0104] != 0xCE) ||
            (gbFastBank0[0x0148] != 0x00)) {

            GBFail(23);
            break;
        }

        gbDivSignatureValid = GBValidateDivSignature();

        if (!gbDivSignatureValid) {
            GBFail(24);
        }
        break;

    case STAGE_CALL_INIT:
        if (!GBResetCore()) {
            GBFail((word)(30 + gbInitResult));
        }
        break;

    case STAGE_FIRST_FRAME:
        gbRenderEnabled = TRUE;
        gbPackEnabled = TRUE;
        gbUseFastRenderer = FALSE;

        if (!GBRunOneFrameSafe()) {
            GBFail(40);
            break;
        }

        gbStage = STAGE_RUNNING;
        gbRunning = TRUE;
        gbFailureCode = 0;
        gbMode = MODE_DIV_ROW8_FS2;
        gbUseRow8Renderer = TRUE;
        gbDivFastEnabled = TRUE;
        gbProfileOpcodes = FALSE;
        gbProfileMemory = FALSE;
        gbDivFastHits = 0;
        gbSkipPhase = 0;

        GBResetMeasurement();
        GBRedrawAll();
        return;
    }

    GBRedrawStatus();

    if ((gbStage != STAGE_RUNNING) &&
        (gbStage != STAGE_FAILED)) {

    }
}


/***********************************************************************
 * Maximum-throughput worker.
 ***********************************************************************/

void
GBEngineWork(void)
{
    Boolean updateStatus;
    Boolean renderThisFrame;

    gbWorkQueued = FALSE;

    if ((!gbRunning) ||
        (gbWindow == 0) ||
        (gbStage != STAGE_RUNNING)) {

        return;
    }

    switch (gbMode) {
    case MODE_CORE:
        gbDivFastEnabled = FALSE;
        gbUseRow8Renderer = FALSE;
        gbRenderEnabled = FALSE;
        gbPackEnabled = FALSE;
        gbUseFastRenderer = FALSE;

        if (!GBRunOneFrameFast()) {
            GBFail(40);
            GBRedrawStatus();
            return;
        }

        gbGuestFramesInPeriod++;
        break;

    case MODE_ORIGINAL_PACK:
        gbDivFastEnabled = FALSE;
        gbUseRow8Renderer = FALSE;
        gbRenderEnabled = TRUE;
        gbPackEnabled = TRUE;
        gbUseFastRenderer = FALSE;

        if (!GBRunOneFrameFast()) {
            GBFail(40);
            GBRedrawStatus();
            return;
        }

        gbGuestFramesInPeriod++;
        break;

    case MODE_TABLE_PACK:
        gbDivFastEnabled = FALSE;
        gbUseRow8Renderer = FALSE;
        gbRenderEnabled = TRUE;
        gbPackEnabled = TRUE;
        gbUseFastRenderer = TRUE;

        if (!GBRunOneFrameFast()) {
            GBFail(40);
            GBRedrawStatus();
            return;
        }

        gbGuestFramesInPeriod++;
        break;

    case MODE_TABLE_FULL:
        gbDivFastEnabled = FALSE;
        gbUseRow8Renderer = FALSE;
        gbRenderEnabled = TRUE;
        gbPackEnabled = TRUE;
        gbUseFastRenderer = TRUE;

        if (!GBRunOneFrameFast()) {
            GBFail(40);
            GBRedrawStatus();
            return;
        }

        gbGuestFramesInPeriod++;
        GBBlit4();
        gbBlitsInPeriod++;
        break;

    case MODE_DIV_CORE:
        gbDivFastEnabled = TRUE;
        gbUseRow8Renderer = FALSE;
        gbRenderEnabled = FALSE;
        gbPackEnabled = FALSE;
        gbUseFastRenderer = FALSE;

        if (!GBRunOneFrameFast()) {
            GBFail(40);
            GBRedrawStatus();
            return;
        }

        gbGuestFramesInPeriod++;
        break;

    case MODE_DIV_FULL:
        gbDivFastEnabled = TRUE;
        gbUseRow8Renderer = FALSE;
        gbRenderEnabled = TRUE;
        gbPackEnabled = TRUE;
        gbUseFastRenderer = TRUE;

        if (!GBRunOneFrameFast()) {
            GBFail(40);
            GBRedrawStatus();
            return;
        }

        gbGuestFramesInPeriod++;
        GBBlit4();
        gbBlitsInPeriod++;
        break;

    case MODE_ROW8_PACK:
        gbDivFastEnabled = FALSE;
        gbUseRow8Renderer = TRUE;
        gbRenderEnabled = TRUE;
        gbPackEnabled = TRUE;
        gbUseFastRenderer = TRUE;

        if (!GBRunOneFrameFast()) {
            GBFail(40);
            GBRedrawStatus();
            return;
        }

        gbGuestFramesInPeriod++;
        break;

    case MODE_ROW8_FULL:
        gbDivFastEnabled = FALSE;
        gbUseRow8Renderer = TRUE;
        gbRenderEnabled = TRUE;
        gbPackEnabled = TRUE;
        gbUseFastRenderer = TRUE;

        if (!GBRunOneFrameFast()) {
            GBFail(40);
            GBRedrawStatus();
            return;
        }

        gbGuestFramesInPeriod++;
        GBBlit4();
        gbBlitsInPeriod++;
        break;

    case MODE_DIV_ROW8_FULL:
        gbDivFastEnabled = TRUE;
        gbUseRow8Renderer = TRUE;
        gbRenderEnabled = TRUE;
        gbPackEnabled = TRUE;
        gbUseFastRenderer = TRUE;

        if (!GBRunOneFrameFast()) {
            GBFail(40);
            GBRedrawStatus();
            return;
        }

        gbGuestFramesInPeriod++;
        GBBlit4();
        gbBlitsInPeriod++;
        break;

    case MODE_DIV_ROW8_FS2:
        gbDivFastEnabled = TRUE;
        gbUseRow8Renderer = TRUE;
        gbPackEnabled = TRUE;
        gbUseFastRenderer = TRUE;
        renderThisFrame = GBTakeDisplayFrame(2);
        gbRenderEnabled = renderThisFrame;

        if (!GBRunOneFrameFast()) {
            GBFail(40);
            GBRedrawStatus();
            return;
        }

        gbGuestFramesInPeriod++;
        if (renderThisFrame) {
            GBBlit4();
            gbBlitsInPeriod++;
        }
        break;

    case MODE_DIV_ROW8_FS3:
        gbDivFastEnabled = TRUE;
        gbUseRow8Renderer = TRUE;
        gbPackEnabled = TRUE;
        gbUseFastRenderer = TRUE;
        renderThisFrame = GBTakeDisplayFrame(3);
        gbRenderEnabled = renderThisFrame;

        if (!GBRunOneFrameFast()) {
            GBFail(40);
            GBRedrawStatus();
            return;
        }

        gbGuestFramesInPeriod++;
        if (renderThisFrame) {
            GBBlit4();
            gbBlitsInPeriod++;
        }
        break;

    case MODE_PROFILE_OPCODE_CORE:
    case MODE_PROFILE_MEMORY_CORE:
        gbDivFastEnabled = TRUE;
        gbUseRow8Renderer = FALSE;
        gbRenderEnabled = FALSE;
        gbPackEnabled = FALSE;
        gbUseFastRenderer = FALSE;

        if (!GBRunOneFrameFast()) {
            GBFail(40);
            GBRedrawStatus();
            return;
        }

        gbGuestFramesInPeriod++;
        break;

    case MODE_PROFILE_BOTH_FS2:
        gbDivFastEnabled = TRUE;
        gbUseRow8Renderer = TRUE;
        gbPackEnabled = TRUE;
        gbUseFastRenderer = TRUE;
        renderThisFrame = GBTakeDisplayFrame(2);
        gbRenderEnabled = renderThisFrame;

        if (!GBRunOneFrameFast()) {
            GBFail(40);
            GBRedrawStatus();
            return;
        }

        gbGuestFramesInPeriod++;
        if (renderThisFrame) {
            GBBlit4();
            gbBlitsInPeriod++;
        }
        break;

    case MODE_BLIT_4BPP:
        gbDivFastEnabled = FALSE;
        gbUseRow8Renderer = FALSE;
        GBBlit4();
        gbBlitsInPeriod++;
        break;
    }

    updateStatus = GBUpdateMeasurement();

    if (updateStatus) {
        GBRedrawStatus();
    }
}


/***********************************************************************
 * Reset and keyboard.
 ***********************************************************************/

void
GBEngineReset(void)
{
    if ((gbStage != STAGE_RUNNING) ||
        (gbFastBank0 == NULL)) {

        return;
    }

    gbRunning = FALSE;
    gbWorkQueued = FALSE;
    gbDivFastHits = 0;
    gbSkipPhase = 0;
    gbUseRow8Renderer =
        ((gbMode == MODE_ROW8_PACK) ||
         (gbMode == MODE_ROW8_FULL) ||
         (gbMode == MODE_DIV_ROW8_FULL) ||
         (gbMode == MODE_DIV_ROW8_FS2) ||
         (gbMode == MODE_DIV_ROW8_FS3) ||
         (gbMode == MODE_PROFILE_BOTH_FS2));

    gbDivFastEnabled =
        ((gbMode == MODE_DIV_CORE) ||
         (gbMode == MODE_DIV_FULL) ||
         (gbMode == MODE_DIV_ROW8_FULL) ||
         (gbMode == MODE_DIV_ROW8_FS2) ||
         (gbMode == MODE_DIV_ROW8_FS3) ||
         (gbMode == MODE_PROFILE_OPCODE_CORE) ||
         (gbMode == MODE_PROFILE_MEMORY_CORE) ||
         (gbMode == MODE_PROFILE_BOTH_FS2));

    gbProfileOpcodes =
        ((gbMode == MODE_PROFILE_OPCODE_CORE) ||
         (gbMode == MODE_PROFILE_BOTH_FS2));

    gbProfileMemory =
        ((gbMode == MODE_PROFILE_MEMORY_CORE) ||
         (gbMode == MODE_PROFILE_BOTH_FS2));

    GBInit4Buffer();

    if (!GBResetCore()) {
        GBFail((word)(30 + gbInitResult));
        GBRedrawStatus();
        return;
    }

    gbRenderEnabled = TRUE;

    if (!GBRunOneFrameSafe()) {
        GBFail(40);
        GBRedrawStatus();
        return;
    }

    gbRunning = TRUE;
    gbFailureCode = 0;
    GBResetMeasurement();

    GBRedrawAll();
}


word
GBEngineKey(word character, word flags)
{
    byte key;
    byte mask;
    Boolean pressed;

    key = (byte)character;
    mask = 0;

    if (flags & CF_FIRST_PRESS) {
        switch (key) {
        case '1':
            GBSetMode(MODE_DIV_ROW8_FULL);
            return GB_ENGINE_KEY_HANDLED;

        case '2':
            GBSetMode(MODE_DIV_ROW8_FS2);
            return GB_ENGINE_KEY_HANDLED;

        case '3':
            GBSetMode(MODE_DIV_ROW8_FS3);
            return GB_ENGINE_KEY_HANDLED;

        case '4':
            GBSetMode(MODE_PROFILE_OPCODE_CORE);
            return GB_ENGINE_KEY_HANDLED;

        case '5':
            GBSetMode(MODE_PROFILE_MEMORY_CORE);
            return GB_ENGINE_KEY_HANDLED;

        case '6':
            GBSetMode(MODE_PROFILE_BOTH_FS2);
            return GB_ENGINE_KEY_HANDLED;

        case '0':
            GBResetMeasurement();
            GBRedrawStatus();
            return GB_ENGINE_KEY_HANDLED;

        case 'r':
        case 'R':
            return GB_ENGINE_KEY_RESET;

        case 'q':
        case 'Q':
        case VC_ESCAPE:
            gbRunning = FALSE;
            gbWorkQueued = FALSE;
            return GB_ENGINE_KEY_QUIT;
        }
    }

    switch (key) {
    case VC_RIGHT:
        mask = JOYPAD_RIGHT;
        break;

    case VC_LEFT:
        mask = JOYPAD_LEFT;
        break;

    case VC_UP:
        mask = JOYPAD_UP;
        break;

    case VC_DOWN:
        mask = JOYPAD_DOWN;
        break;

    case 'z':
    case 'Z':
    case VC_BLANK:
        mask = JOYPAD_A;
        break;

    case 'x':
    case 'X':
        mask = JOYPAD_B;
        break;

    case VC_ENTER:
        mask = JOYPAD_START;
        break;

    case VC_TAB:
        mask = JOYPAD_SELECT;
        break;
    }

    if ((mask != 0) &&
        (gbStage == STAGE_RUNNING) &&
        (gbMode != MODE_BLIT_4BPP)) {

        pressed = ((flags & CF_RELEASE) == 0);

        if (pressed) {
            gbCore.direct.joypad_state.joypad &=
                (byte)(~mask);
        } else {
            gbCore.direct.joypad_state.joypad |= mask;
        }

        return GB_ENGINE_KEY_HANDLED;
    }

    return GB_ENGINE_KEY_UNHANDLED;
}


/***********************************************************************
 * Lifecycle.
 ***********************************************************************/

void
GBEngineContentClosing(void)
{
    gbWindow = 0;
    gbWorkQueued = FALSE;
}


void
GBEngineDetach(void)
{
    gbRunning = FALSE;
    gbWindow = 0;
    gbWorkQueued = FALSE;
    gbDivFastEnabled = FALSE;
    gbProfileOpcodes = FALSE;
    gbProfileMemory = FALSE;
    gbSkipPhase = 0;

    GBCloseFileAndPath();
    GBFreeMemory();
}

/***********************************************************************
 * Profiler implementation.
 *
 * Kept in the same Borland object/code resource as the emulator core.
 * The hot opcode and memory hooks therefore remain ordinary near/far C
 * calls instead of GEOS cross-resource entry-point calls.
 ***********************************************************************/
#include "GBPDATA.INC"

Boolean gbProfileOpcodes = FALSE;
Boolean gbProfileMemory = FALSE;

static dword gbOpcodeCount[256];
static dword gbCBCount[256];
static dword gbOpcodeTotal = 0;
static dword gbCBTotal = 0;
static dword gbMemoryReadCount[GB_MEMORY_GROUPS];
static dword gbMemoryWriteCount[GB_MEMORY_GROUPS];
static dword gbMemoryReadTotal = 0;
static dword gbMemoryWriteTotal = 0;
static dword gbDivHitsAtPeriodStart = 0;

dword gbMeasuredOpsPerSec = 0;
dword gbMeasuredCBPerSec = 0;
dword gbMeasuredReadsPerSec = 0;
dword gbMeasuredWritesPerSec = 0;
dword gbMeasuredDivPerSec = 0;

word gbTopOpcode[GB_PROFILE_TOP_OPS];
dword gbTopOpcodeRate[GB_PROFILE_TOP_OPS];
byte gbTopOpcodePercent[GB_PROFILE_TOP_OPS];

word gbTopCB[GB_PROFILE_TOP_CB];
dword gbTopCBRate[GB_PROFILE_TOP_CB];

word gbTopMemoryGroup[GB_PROFILE_TOP_MEM];
dword gbTopMemoryReadRate[GB_PROFILE_TOP_MEM];
dword gbTopMemoryWriteRate[GB_PROFILE_TOP_MEM];

void
GBProfileOpcodeHit(byte opcode)
{
    gbOpcodeCount[opcode]++;
    gbOpcodeTotal++;
}

void
GBProfileCBHit(byte opcode)
{
    gbCBCount[opcode]++;
    gbCBTotal++;
}

static byte
GBMemoryGroup(word address)
{
    if (address < 0x8000) return 0;
    if (address < 0xA000) return 1;
    if (address < 0xC000) return 2;
    if (address < 0xE000) return 3;
    if (address < 0xFE00) return 4;
    if (address < 0xFEA0) return 5;
    if (address < 0xFF00) return 6;
    if (address < 0xFF80) return 7;
    if (address < 0xFFFF) return 8;
    return 9;
}

void
GBProfileReadAddress(word address)
{
    byte group = GBMemoryGroup(address);
    gbMemoryReadCount[group]++;
    gbMemoryReadTotal++;
}

void
GBProfileWriteAddress(word address)
{
    byte group = GBMemoryGroup(address);
    gbMemoryWriteCount[group]++;
    gbMemoryWriteTotal++;
}

static void
GBResetRawCounters(void)
{
    word index;

    for (index = 0; index < 256; index++) {
        gbOpcodeCount[index] = 0;
        gbCBCount[index] = 0;
    }

    for (index = 0; index < GB_MEMORY_GROUPS; index++) {
        gbMemoryReadCount[index] = 0;
        gbMemoryWriteCount[index] = 0;
    }

    gbOpcodeTotal = 0;
    gbCBTotal = 0;
    gbMemoryReadTotal = 0;
    gbMemoryWriteTotal = 0;
}

static void
GBClearSnapshot(void)
{
    word index;

    gbMeasuredOpsPerSec = 0;
    gbMeasuredCBPerSec = 0;
    gbMeasuredReadsPerSec = 0;
    gbMeasuredWritesPerSec = 0;
    gbMeasuredDivPerSec = 0;

    for (index = 0; index < GB_PROFILE_TOP_OPS; index++) {
        gbTopOpcode[index] = 0xFFFF;
        gbTopOpcodeRate[index] = 0;
        gbTopOpcodePercent[index] = 0;
    }

    for (index = 0; index < GB_PROFILE_TOP_CB; index++) {
        gbTopCB[index] = 0xFFFF;
        gbTopCBRate[index] = 0;
    }

    for (index = 0; index < GB_PROFILE_TOP_MEM; index++) {
        gbTopMemoryGroup[index] = 0xFFFF;
        gbTopMemoryReadRate[index] = 0;
        gbTopMemoryWriteRate[index] = 0;
    }
}

static Boolean
GBIndexSelected(const word *indices, word count, word candidate)
{
    word index;

    for (index = 0; index < count; index++) {
        if (indices[index] == candidate) return TRUE;
    }
    return FALSE;
}

void
GBProfileReset(dword currentDivHits)
{
    gbDivHitsAtPeriodStart = currentDivHits;
    GBResetRawCounters();
    GBClearSnapshot();
}

void
GBProfileSnapshot(dword elapsed, dword currentDivHits)
{
    word rank;
    word index;
    word bestIndex;
    dword bestCount;
    dword count;
    dword total;
    dword divDelta;

    if (elapsed == 0) return;

    gbMeasuredOpsPerSec = (gbOpcodeTotal * 60L) / elapsed;
    gbMeasuredCBPerSec = (gbCBTotal * 60L) / elapsed;
    gbMeasuredReadsPerSec = (gbMemoryReadTotal * 60L) / elapsed;
    gbMeasuredWritesPerSec = (gbMemoryWriteTotal * 60L) / elapsed;

    divDelta = currentDivHits - gbDivHitsAtPeriodStart;
    gbMeasuredDivPerSec = (divDelta * 60L) / elapsed;
    gbDivHitsAtPeriodStart = currentDivHits;
    total = gbOpcodeTotal;

    for (rank = 0; rank < GB_PROFILE_TOP_OPS; rank++) {
        bestIndex = 0xFFFF;
        bestCount = 0;
        for (index = 0; index < 256; index++) {
            if (!GBIndexSelected(gbTopOpcode, rank, index) &&
                gbOpcodeCount[index] > bestCount) {
                bestIndex = index;
                bestCount = gbOpcodeCount[index];
            }
        }
        gbTopOpcode[rank] = bestIndex;
        if (bestIndex != 0xFFFF) {
            gbTopOpcodeRate[rank] = (bestCount * 60L) / elapsed;
            if (total != 0) {
                gbTopOpcodePercent[rank] =
                    (byte)((bestCount * 100L) / total);
            }
        }
    }

    for (rank = 0; rank < GB_PROFILE_TOP_CB; rank++) {
        bestIndex = 0xFFFF;
        bestCount = 0;
        for (index = 0; index < 256; index++) {
            if (!GBIndexSelected(gbTopCB, rank, index) &&
                gbCBCount[index] > bestCount) {
                bestIndex = index;
                bestCount = gbCBCount[index];
            }
        }
        gbTopCB[rank] = bestIndex;
        if (bestIndex != 0xFFFF) {
            gbTopCBRate[rank] = (bestCount * 60L) / elapsed;
        }
    }

    for (rank = 0; rank < GB_PROFILE_TOP_MEM; rank++) {
        bestIndex = 0xFFFF;
        bestCount = 0;
        for (index = 0; index < GB_MEMORY_GROUPS; index++) {
            count = gbMemoryReadCount[index] + gbMemoryWriteCount[index];
            if (!GBIndexSelected(gbTopMemoryGroup, rank, index) &&
                count > bestCount) {
                bestIndex = index;
                bestCount = count;
            }
        }
        gbTopMemoryGroup[rank] = bestIndex;
        if (bestIndex != 0xFFFF) {
            gbTopMemoryReadRate[rank] =
                (gbMemoryReadCount[bestIndex] * 60L) / elapsed;
            gbTopMemoryWriteRate[rank] =
                (gbMemoryWriteCount[bestIndex] * 60L) / elapsed;
        }
    }

    GBResetRawCounters();
}


static void
GBPDwordToText(dword value, char *buffer)
{
    char reverse[11];
    word count = 0;
    word index;

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while ((value != 0) && (count < 10)) {
        reverse[count++] = (char)('0' + (value % 10));
        value /= 10;
    }

    for (index = 0; index < count; index++) {
        buffer[index] = reverse[count - index - 1];
    }
    buffer[count] = '\0';
}

static void
GBPWordToText(word value, char *buffer)
{
    GBPDwordToText((dword)value, buffer);
}

static void
GBPByteToHex(byte value, char *buffer)
{
    static const char digits[] = "0123456789ABCDEF";

    buffer[0] = digits[value >> 4];
    buffer[1] = digits[value & 0x0F];
    buffer[2] = '\0';
}

static const char *
GBPMemoryGroupText(word group)
{
    switch (group) {
    case 0: return "ROM";
    case 1: return "VRAM";
    case 2: return "CART";
    case 3: return "WRAM";
    case 4: return "ECHO";
    case 5: return "OAM";
    case 6: return "BAD";
    case 7: return "IO";
    case 8: return "HRAM";
    case 9: return "IE";
    }
    return "?";
}

const char *
GBProfileText(void)
{
    if (gbProfileOpcodes && gbProfileMemory) return "OP+MEM";
    if (gbProfileOpcodes) return "OPCODE";
    if (gbProfileMemory) return "MEMORY";
    return "OFF";
}

void
GBProfileDrawPanel(GStateHandle gstate)
{
    char text[11];
    char hex[3];
    word index;
    word y;

    GrSetFont(gstate, FID_DTC_URW_SANS, MakeWWFixed(7.0));

    if (gbProfileMemory && !gbProfileOpcodes) {
        GrDrawText(gstate, 316, 8, "DATA MEMORY", 0);
        GrDrawText(gstate, 316, 20, "AREA", 0);
        GrDrawText(gstate, 366, 20, "R/s", 0);
        GrDrawText(gstate, 458, 20, "W/s", 0);

        for (index = 0; index < GB_PROFILE_TOP_MEM; index++) {
            y = (word)(34 + index * 13);
            if (gbTopMemoryGroup[index] == 0xFFFF) {
                GrDrawText(gstate, 316, y, "--", 0);
                continue;
            }
            GrDrawText(gstate, 316, y,
                       GBPMemoryGroupText(gbTopMemoryGroup[index]), 0);
            GBPDwordToText(gbTopMemoryReadRate[index], text);
            GrDrawText(gstate, 366, y, text, 0);
            GBPDwordToText(gbTopMemoryWriteRate[index], text);
            GrDrawText(gstate, 458, y, text, 0);
        }

        GrDrawText(gstate, 316, 120, "FETCH BYTES EXCLUDED", 0);
        GrDrawText(gstate, 316, 136, "4 OPCODE CORE", 0);
        GrDrawText(gstate, 316, 150, "5 MEMORY CORE", 0);
        GrDrawText(gstate, 316, 164, "6 BOTH + FS2", 0);
        GrDrawText(gstate, 316, 178, "0 CLEAR SAMPLE", 0);
        GrDrawText(gstate, 316, 192, "UNIQUE: GBPROF.GEO", 0);
        return;
    }

    GrDrawText(gstate, 316, 8, "TOP OPCODES", 0);
    GrDrawText(gstate, 316, 20, "OP", 0);
    GrDrawText(gstate, 344, 20, "PER SEC", 0);
    GrDrawText(gstate, 424, 20, "%", 0);

    for (index = 0; index < GB_PROFILE_TOP_OPS; index++) {
        y = (word)(34 + index * 13);
        if (gbTopOpcode[index] == 0xFFFF) {
            GrDrawText(gstate, 316, y, "--", 0);
            continue;
        }
        GBPByteToHex((byte)gbTopOpcode[index], hex);
        GrDrawText(gstate, 316, y, hex, 0);
        GBPDwordToText(gbTopOpcodeRate[index], text);
        GrDrawText(gstate, 344, y, text, 0);
        GBPWordToText((word)gbTopOpcodePercent[index], text);
        GrDrawText(gstate, 424, y, text, 0);
        GrDrawText(gstate, 444, y, "%", 0);
    }

    if (gbProfileOpcodes) {
        GrDrawText(gstate, 316, 142, "TOP CB", 0);
        for (index = 0; index < GB_PROFILE_TOP_CB; index++) {
            y = (word)(156 + index * 13);
            if (gbTopCB[index] == 0xFFFF) {
                GrDrawText(gstate, 316, y, "CB--", 0);
                continue;
            }
            GrDrawText(gstate, 316, y, "CB", 0);
            GBPByteToHex((byte)gbTopCB[index], hex);
            GrDrawText(gstate, 332, y, hex, 0);
            GBPDwordToText(gbTopCBRate[index], text);
            GrDrawText(gstate, 370, y, text, 0);
        }
    } else {
        GrDrawText(gstate, 316, 142, "4 OPCODE CORE", 0);
        GrDrawText(gstate, 316, 156, "5 MEMORY CORE", 0);
        GrDrawText(gstate, 316, 170, "6 BOTH + FS2", 0);
        GrDrawText(gstate, 316, 184, "0 CLEAR / GBPROF.GEO", 0);
    }
}

#ifndef _GBPDATA_H_
#define _GBPDATA_H_

#include <geos.h>
#include <graphics.h>

#define GB_PROFILE_TOP_OPS             8
#define GB_PROFILE_TOP_CB              3
#define GB_PROFILE_TOP_MEM             6
#define GB_MEMORY_GROUPS              10

extern Boolean gbProfileOpcodes;
extern Boolean gbProfileMemory;

extern dword gbMeasuredOpsPerSec;
extern dword gbMeasuredCBPerSec;
extern dword gbMeasuredReadsPerSec;
extern dword gbMeasuredWritesPerSec;
extern dword gbMeasuredDivPerSec;

extern word gbTopOpcode[GB_PROFILE_TOP_OPS];
extern dword gbTopOpcodeRate[GB_PROFILE_TOP_OPS];
extern byte gbTopOpcodePercent[GB_PROFILE_TOP_OPS];

extern word gbTopCB[GB_PROFILE_TOP_CB];
extern dword gbTopCBRate[GB_PROFILE_TOP_CB];

extern word gbTopMemoryGroup[GB_PROFILE_TOP_MEM];
extern dword gbTopMemoryReadRate[GB_PROFILE_TOP_MEM];
extern dword gbTopMemoryWriteRate[GB_PROFILE_TOP_MEM];

extern void GBProfileOpcodeHit(byte opcode);
extern void GBProfileCBHit(byte opcode);
extern void GBProfileReadAddress(word address);
extern void GBProfileWriteAddress(word address);
extern void GBProfileReset(dword currentDivHits);
extern void GBProfileSnapshot(dword elapsed, dword currentDivHits);
extern const char *GBProfileText(void);
extern void GBProfileDrawPanel(GStateHandle gstate);

#endif

#ifndef _GBENG_H_
#define _GBENG_H_

#include <geos.h>

#define GB_ENGINE_KEY_UNHANDLED  0
#define GB_ENGINE_KEY_HANDLED    1
#define GB_ENGINE_KEY_RESET      2
#define GB_ENGINE_KEY_QUIT       3

extern void GBEngineExpose(WindowHandle win, GStateHandle gstate);
extern void GBEngineNextStage(void);
extern void GBEngineWork(void);
extern void GBEngineReset(void);
extern word GBEngineKey(word character, word flags);
extern void GBEngineContentClosing(void);
extern void GBEngineDetach(void);

extern Boolean GBEngineRequestWork(void);
extern Boolean GBEngineNeedNextStage(void);

#endif

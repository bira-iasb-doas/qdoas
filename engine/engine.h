#ifndef _ENGINE_H_GUARD
#define _ENGINE_H_GUARD

#include "doas.h"

typedef SPEC_INFO ENGINE_CONTEXT;                                               // QDOAS ---> SPEC_INFO will be the engine_context in the future

ENGINE_CONTEXT *EngineCreateContext(void);
void            EngineDestroyContext(ENGINE_CONTEXT *pEngineContext);
void            EngineSetProject(ENGINE_CONTEXT *pEngineContext);
int             EngineSetFile(ENGINE_CONTEXT *pEngineContext,const char *fileName);
void            EngineReadFile(ENGINE_CONTEXT *pEngineContext,int indexRecord,double *x,double *y);

#endif

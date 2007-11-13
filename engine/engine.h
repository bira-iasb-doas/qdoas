#ifndef _ENGINE_H_GUARD
#define _ENGINE_H_GUARD

#include "doas.h"

EXTERN UCHAR DOAS_dbgFile[MAX_PATH_LEN+1];

ENGINE_CONTEXT *EngineCreateContext(void);
void            EngineResetContext(ENGINE_CONTEXT *pEngineContext);
void            EngineDestroyContext(ENGINE_CONTEXT *pEngineContext);
void            EngineSetProject(ENGINE_CONTEXT *pEngineContext);
RC              EngineSetFile(ENGINE_CONTEXT *pEngineContext,const char *fileName);
RC              EngineReadFile(ENGINE_CONTEXT *pEngineContext,int indexRecord,INT dateFlag,INT localCalDay);
RC              EngineRequestBeginBrowseSpectra(ENGINE_CONTEXT *pEngineContext,const char *spectraFileName,unsigned int action);

#endif

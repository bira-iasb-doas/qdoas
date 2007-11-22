#ifndef _ENGINE_H_GUARD
#define _ENGINE_H_GUARD

#include "doas.h"

// =======================
// DEFINITION OF CONSTANTS
// =======================

// ===================
// GLOBAL DECLARATIONS
// ===================

EXTERN UCHAR          ENGINE_dbgFile[MAX_PATH_LEN+1];                           // debug file
EXTERN ENGINE_CONTEXT ENGINE_contextRef;                                        // copy of the engine context for the automatic search of the reference spectrum
EXTERN double         ENGINE_localNoon;                                         // local noon

// ==========
// PROTOTYPES
// ==========

ENGINE_CONTEXT *EngineCreateContext(void);
void            EngineResetContext(ENGINE_CONTEXT *pEngineContext,INT closeFiles);
void            EngineDestroyContext(ENGINE_CONTEXT *pEngineContext);
RC              EngineCopyContext(ENGINE_CONTEXT *pEngineContextTarget,ENGINE_CONTEXT *pEngineContextSource);
void            EngineSetProject(ENGINE_CONTEXT *pEngineContext);
RC              EngineSetFile(ENGINE_CONTEXT *pEngineContext,const char *fileName);
RC              EngineReadFile(ENGINE_CONTEXT *pEngineContext,int indexRecord,INT dateFlag,INT localCalDay);
RC              EngineRequestBeginBrowseSpectra(ENGINE_CONTEXT *pEngineContext,const char *spectraFileName,unsigned int action);
RC              EngineNewRef(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

#endif

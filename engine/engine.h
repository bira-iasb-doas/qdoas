#ifndef _ENGINE_H_GUARD
#define _ENGINE_H_GUARD

#include "doas.h"

// =======================
// DEFINITION OF CONSTANTS
// =======================

// ===================
// GLOBAL DECLARATIONS
// ===================

EXTERN INT            NDET;                                                     // size of the detector

EXTERN UCHAR          ENGINE_dbgFile[MAX_PATH_LEN+1];                           // debug file
EXTERN ENGINE_CONTEXT ENGINE_contextRef;                                        // copy of the engine context for the automatic search of the reference spectrum
EXTERN double         ENGINE_localNoon;                                         // local noon
EXTERN XSCONV         ENGINE_contextToolConvolution,                            // engine context for the convolution tool
                      ENGINE_contextToolRing,                                   // engine context for the ring tool
                      ENGINE_contextToolUndersampling;                          // engine context for the undersampling tool


// ==========
// PROTOTYPES
// ==========

RC              EngineCopyContext(ENGINE_CONTEXT *pEngineContextTarget,ENGINE_CONTEXT *pEngineContextSource);
RC              EngineSetProject(ENGINE_CONTEXT *pEngineContext);
RC              EngineSetFile(ENGINE_CONTEXT *pEngineContext,const char *fileName);
RC              EngineReadFile(ENGINE_CONTEXT *pEngineContext,int indexRecord,INT dateFlag,INT localCalDay);
RC              EngineRequestBeginBrowseSpectra(ENGINE_CONTEXT *pEngineContext,const char *spectraFileName);
RC              EngineRequestEndBrowseSpectra(ENGINE_CONTEXT *pEngineContext);
RC              EngineNewRef(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

RC              EngineEndCurrentSession(ENGINE_CONTEXT *pEngineContext);
ENGINE_CONTEXT *EngineCreateContext(void);
RC              EngineDestroyContext(ENGINE_CONTEXT *pEngineContext);


#endif

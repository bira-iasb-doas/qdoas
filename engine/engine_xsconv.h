
#include "doas.h"

#ifndef _ENGINE_XSCONV_
#define _ENGINE_XSCONV_

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// =======================
// DEFINITION OF CONSTANTS
// =======================

typedef XSCONV ENGINE_XSCONV_CONTEXT;

// ===================
// GLOBAL DECLARATIONS
// ===================

// ==========
// PROTOTYPES
// ==========

ENGINE_XSCONV_CONTEXT *EngineXsconvCreateContext(void);
RC                     EngineXsconvDestroyContext(ENGINE_XSCONV_CONTEXT *pEngineContext);
RC                     XSCONV_Convolution(ENGINE_XSCONV_CONTEXT *pEngineContext,void *responseHandle);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

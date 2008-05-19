#ifndef _ENGINE_XSCONV_
#define _ENGINE_XSCONV_

#include "doas.h"

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

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

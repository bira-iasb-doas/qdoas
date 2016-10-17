#ifndef APEX_H
#define APEX_H

#include "doas.h"

// initial length of allocated buffers
#define APEX_INIT_LENGTH 2048

#ifdef __cplusplus
extern "C" {
#endif

  int apex_init(const char *reference_file, ENGINE_CONTEXT *pEngineContext);

  int apex_set(ENGINE_CONTEXT *pEngineContext);

  int apex_read(ENGINE_CONTEXT *pEngineContext, int record);

  int apex_get_reference(const char *filename, int i_crosstrack, double *lambda, double *spectrum, int *n_wavel);

  void apex_clean(void);

#ifdef __cplusplus
}
#endif

#endif

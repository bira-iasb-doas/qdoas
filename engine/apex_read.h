#ifndef APEX_H
#define APEX_H

#include "doas.h"

#ifdef __cplusplus
extern "C" {
#endif

  int apex_set(ENGINE_CONTEXT *pEngineContext);

  int apex_read(ENGINE_CONTEXT *pEngineContext, int record);

  int apex_get_reference(const char *filename, int i_crosstrack, double *lambda, double *spectrum);

  void apex_clean(void);

#ifdef __cplusplus
}
#endif

#endif

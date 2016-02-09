#ifndef TROPOMI_READ_H
#define TROPOMI_READ_H

#include "doas.h"
#include "tropomi.h"

#ifdef __cplusplus
extern "C" {
#endif

  // load reference spectra, set NDET[] and use_row[] arrays.
  int tropomi_init(const char *ref_filename, ENGINE_CONTEXT *pEngineContext);

  int tropomi_read(ENGINE_CONTEXT *pEngineContext,int recordNo);

  int tropomi_set(ENGINE_CONTEXT *pEngineContext);

  int tropomi_get_reference(const char* filename, int pixel, double *lambda, double *spectrum, double *sigma, int *n_wavel);

  void tropomi_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif

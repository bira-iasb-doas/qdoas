#ifndef SVD_H
#define SVD_H

#include "doas.h"
#include "spectral_range.h"

typedef struct _svd {
  int DimC,DimL,DimP,NF,NP,nFit,Z; // gaps and analysis window limits in pixels units

  double LFenetre[MAX_FEN][2], // gaps and analysis window limits in wavelength units (nm)
    **A,**U,**V,*W,**P,         // SVD matrices
    **covar,
    *SigmaSqr;
  doas_spectrum *specrange;     // gaps and analysis window limits in pixels units
} SVD;

RC SVD_Bksb(double **u,double *w,double **v,int m,int n,double *b,double *x);
RC SVD_Dcmp(double **a,int m,int n,double *w,double **v,double *SigmaSqr,double **covar);

void SVD_Free(const char *callingFunctionShort,SVD *pSvd);
RC   SVD_LocalAlloc(const char *callingFunctionShort,SVD *pSvd);

#endif

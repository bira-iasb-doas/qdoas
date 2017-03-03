#ifndef SVD_H
#define SVD_H

struct svd {
  double **U,**V,*W; // SVD matrices
};

int SVD_Bksb(const struct svd *svd, int m, int n, const double *b, double *x);
int SVD_Dcmp(struct svd *svd, int m, int n, double *SigmaSqr,double **covar);

#endif

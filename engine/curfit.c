
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  NON-LINEAR LEAST-SQUARES FIT
//  Name of module    :  CURFIT.C
//  Creation date     :  These routines were existing in the DOS version and
//                       have been completed and improved for WinDOAS
//
//  Reference
//
//         Data Reduction and Error Analysis for the Physical Sciences
//         Philip R. Bevington
//         Associate professor of physics - Case Western Reserve University
//         McGraw-Hill Book Company
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//  The QT version of the program has been developed jointly by the Belgian
//  Institute for Space Aeronomy (BIRA-IASB) and the Science and Technology
//  company (S[&]T) - Copyright (C) 2007
//
//      BIRA-IASB                                   S[&]T
//      Belgian Institute for Space Aeronomy        Science [&] Technology
//      Avenue Circulaire, 3                        Postbus 608
//      1180     UCCLE                              2600 AP Delft
//      BELGIUM                                     THE NETHERLANDS
//      caroline.fayt@aeronomie.be                  info@stcorp.nl
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS :
//
//  Fchisq - evaluate the chi square of the fit
//
//  CurfitMatinv - this routine inverts a square symmetric matrix and calculates
//                 its determinant, substituting the inverse matrix into the same
//                 array as the original one.
//
//  CurfitError - includes in the error message the name of the parameter
//                responsible of the error
//
//  CurfitNumDeriv - evaluate the derivative of a general function to a fitted
//                   non linear parameter
//
//  CurfitDerivFunc
//
//       Evaluate the partial derivatives of the fitting function in non linear parameters.
//       If possible, derivatives are calculated analytically in order to avoid two evaluations
//       of the fitting function.
//
//  Curfit - make a least-squares fit to a non-linear function with a
//           linearization of the fitting function
//
//  ----------------------------------------------------------------------------

#include "doas.h"

// -----------------------------------------------------------------------------
// FUNCTION      Fchisq
// -----------------------------------------------------------------------------
// PURPOSE       Evaluate the chi square of the fit as
//
//                 chisqr=sum((Y-YFIT)**2/sygmaY**2)/nFree in instrumental weighting mode
//                 chisqr=sum((Y-Yfit)**2)/nFree in no weighting mode
//                 chisqr=sum((Y-Yfit)**2)/Y/nFree in statistical weighting mode
//
// INPUT         mode    - determine the method of weighting least-squares fit
//               nFree   - the number of degrees of freedom
//
//                 PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL (instrumental) weight(i)=1./sigmay(i)**2
//                 PRJCT_ANLYS_FIT_WEIGHTING_NONE         (no weighting) weight(i)=1.
//                 PRJCT_ANLYS_FIT_WEIGHTING_STATISTICAL  (statistical)  weight(i)=1./Y(i)
//
//               Y       - data points for dependent variable
//               Yfit    - vector of calculated values of Y
//               sigmaY  - standard deviations on Y data points to weight the fit
//               nY      - number of data points in Y
//
// RETURN        the calculated chi square
// -----------------------------------------------------------------------------

double Fchisq(int mode,int nFree,double *Y,double *Yfit,double *sigmaY,int nY)
 {
 	// Declarations

  double chisq, weight;
  int k, mode2use;

  // Initializations

  chisq=(double) 0.;
  mode2use=(sigmaY==NULL)?PRJCT_ANLYS_FIT_WEIGHTING_NONE:mode;                  // the instrumental mode is selected but there are no errors to weight
                                                                                // the fit -> use no weighting mode

  if (nFree<=0)                                                                 // the test should be done before entering this function
   return((double)0.);

  // Accumulate the chi square

  switch ( mode2use )
  {
// -----------------------------------------------------------------------------
// case PRJCT_ANLYS_FIT_WEIGHTING_STATISTICAL :                                 // statistical weighting method
//
//  for (k=0;k<nY;k++)                                                          // This option is disabled from the original version
//   {                                                                          // As the input Y should be set to zero,
//    weight=(Y[k]!=(double)0.)?(double)1./fabs(Y[k]):(double)1.;               // PRJCT_ANLYS_FIT_WEIGHTING_STATISTICAL and PRJCT_ANLYS_FIT_WEIGHTING_NONE
//    chisq+=(weight*(Y[k]-Yfit[k])*(Y[k]-Yfit[k]));                            // should be the same
//   }
//
// break;
// -----------------------------------------------------------------------------
   case PRJCT_ANLYS_FIT_WEIGHTING_NONE :                                        // no weighting method

    for (k=0;k<nY;k++)
     if ((fabs(Y[k]-Yfit[k])<=1.e16) && (fabs(Y[k]-Yfit[k])>=1.e-16))           // in order to avoid precision error
      chisq+=((Y[k]-Yfit[k])*(Y[k]-Yfit[k]));

   break;
// -----------------------------------------------------------------------------
   case PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL :                                // instrumental weighting method

    for (k=0;k<nY;k++)
      {
       weight=(sigmaY[k]!=(double)0.)?(double)1./(sigmaY[k]*sigmaY[k]):(double)1.;
       chisq+=(weight*(Y[k]-Yfit[k])*(Y[k]-Yfit[k]));
      }

   break;
// -----------------------------------------------------------------------------
  }

  // The chi square should account for the number of degrees of freedom of the system

  return ((double)chisq/nFree);
 }

// -----------------------------------------------------------------------------
// FUNCTION      CurfitMatinv
// -----------------------------------------------------------------------------
// PURPOSE       This routine inverts a square symmetric matrix and calculates
//               its determinant, substituting the inverse matrix into the same
//               array as the original one.
//
// INPUT         array   - the original matrix to inverse
//               nOrder  - the degree of the matrix (order of its determinant)
//
// OUTPUT        array   - the inverse of the input matrix
//
// RETURN        ERROR_ID_NO if there is no allocation problem
//               ERROR_ID_ALLOC if the allocation of a vector failed
// -----------------------------------------------------------------------------

RC CurfitMatinv(double **array,int nOrder,double *pDet)
 {
 	// Declarations

  int    *ik, *jk, i, j, k, l;                                                  // indexes for loops and array
  double  amax,                                                                 // the largest element of a matrix
          saveArray;                                                            // temporary variable for swapping elements of the matrix
  RC      rc;                                                                   // return code

  // Initializations

  *pDet=(double)1.;                                                             // to avoid division by 0
  jk=NULL;
  rc=ERROR_ID_NO;

  // Allocate buffers for indexes of selected rows or columns of the matrix

  if (((ik=(int *)MEMORY_AllocBuffer("CurfitMatinv","ik",nOrder,sizeof(int),0,MEMORY_TYPE_INT))==NULL) ||
      ((jk=(int *)MEMORY_AllocBuffer("CurfitMatinv","jk",nOrder,sizeof(int),0,MEMORY_TYPE_INT))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
   	// Reorganize the matrix in order to have the largest element in the diagonal
   	// in order to improve the computational precision

    for (k=0;k<nOrder;k++)
     {
     	// search for the largest element in the matrix array(k:nOrder,k:nOrder)

      do
       {
        amax=(double)0.;

        for (i=k;i<nOrder;i++)
         for (j=k;j<nOrder;j++)
          if (fabs(amax)<=fabs(array[i][j]))
           {
           	amax=array[i][j];
           	ik[k]=i;
           	jk[k]=j;
           }

        if (amax==(double)0.)
         {
         	*pDet=(double)0.;                                                     // a error message will be returned by the calling function
         	return rc;
         }

        // Reorganize the matrix in order to have the largest element in array[k][k] (make it a diagonal element)

        i=ik[k];

        if (i>k)
         {
          for (j=0;j<nOrder;j++)
           {
            saveArray =(double)array[k][j];
            array[k][j]=(double)array[i][j];
            array[i][j]=(double)-saveArray;
           }
         }

        j=jk[k];

        if ((j>k) && (i>=k))
         {
          for (i=0;i<nOrder;i++)
           {
            saveArray =(double)array[i][k];
            array[i][k]=(double)array[i][j];
            array[i][j]=(double)-saveArray;
           }
         }
       }
      while ((ik[k]<k) || (jk[k]<k));

      // Gauss-Jordan elimination : accumulate elements of inverse matrix

      for (i=0;i<nOrder;i++)
       if (i!=k)
        array[i][k]/=(double)(-amax);

      for (i=0;i<nOrder;i++)
        {
         if (i!=k)
          {
           for (j=0;j<nOrder;j++)
            if (j!=k)
             array[i][j]+=(array[i][k]*array[k][j]);
          }
        }

      for (j=0;j<nOrder;j++)
       if (j!=k)
        array[k][j]/=(double)amax;

      array[k][k]=(double)1./amax;
      *pDet*=(double)amax;
     }

    // Restore the ordering of the matrix

    for (l=0;l<nOrder;l++)
     {
      k=nOrder-l-1;
      j=ik[k];

      if (j>k)
       {
        for (i=0;i<nOrder;i++)
         {
          saveArray =(double)array[i][k];
          array[i][k]=(double)-array[i][j];
          array[i][j]=(double)saveArray;
         }
       }

      i=jk[k];

      if (i>k)
       {
        for (j=0;j<nOrder;j++)
          {
           saveArray =(double)array[k][j];
           array[k][j]=(double)-array[i][j];
           array[i][j]=(double)saveArray;
          }
       }
     }
   }

  // Release the allocated buffers

  if (ik!=NULL)
   MEMORY_ReleaseBuffer("CurfitMatinv","ik",ik);
  if (jk!=NULL)
   MEMORY_ReleaseBuffer("CurfitMatinv","jk",jk);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      CurfitError
// -----------------------------------------------------------------------------
// PURPOSE       Includes in the error message the name of the parameter
//               responsible of the error
//
// INPUT         indexError - the index of the parameter to search for
//               p, deltap  - the non linear parameters and delta values
//
// OUTPUT        string  - the final error message
//
// RETURN        a pointer to the final error message
// -----------------------------------------------------------------------------

UCHAR *CurfitError(UCHAR *string,INDEX indexError,double *p,double *deltap)
 {
 	// Declarations

  CROSS_REFERENCE *pTabCross;                                                   // settings of a parameter to fit
  UCHAR param[MAX_ITEM_NAME_LEN+1];                                             // the name of the parameter
  INDEX i;                                                                      // browse parameters to fit

  // Initializations

  memset(param,0,MAX_ITEM_NAME_LEN+1);
  memset(string,0,MAX_ITEM_TEXT_LEN+1);

  // Browse parameters to fit

  for (i=0;i<Feno->NTabCross;i++)
   {
    pTabCross=&Feno->TabCross[i];

    if (pTabCross->FitShift==indexError)
     strcpy(param,"Shift");
    else if (pTabCross->FitStretch==indexError)
     strcpy(param,"Stretch");
    else if (pTabCross->FitStretch2==indexError)
     strcpy(param,"Stretch2");
    else if (pTabCross->FitScale==indexError)
     strcpy(param,"Scale");
    else if (pTabCross->FitScale2==indexError)
     strcpy(param,"Scale2");
    else if (pTabCross->FitParam==indexError)
     strcpy(param,"Predefined");

    if (strlen(param))
     {
      sprintf(string,"(%s error for %s (%.le,%.le))",param,WorkSpace[pTabCross->Comp].symbolName,p[indexError],deltap[indexError]);
      break;
     }
   }

  // Return

  return (UCHAR *)string;
 }

// -----------------------------------------------------------------------------
// FUNCTION      CurfitNumDeriv
// -----------------------------------------------------------------------------
// PURPOSE       Evaluate the derivative of a general function to a fitted non linear parameter
//
// INPUT         X       - wavelengths or pixels according to the selected shift unit
//               specX   - the spectrum to evaluate
//               srefX   - the control spectrum (also called reference spectrum)
//               nX      - the size of previous vectors (depending on the size of the detector)
//
//                   !!! use whole vectors (0..NDET-1) in order to avoid side
//                   !!! effects if the shift is fitted in ANALYSE_Function
//
//               Y       - the data to fit (vector of zeros to fit the residual)
//               sigmaY  - standard deviations for Y data points
//               nY      - number of data points in Y
//
//               P       - values of parameters fitted linearly in the fitting function
//               A       - values of parameters to fit non linearly
//               deltaA  - increments for non linear parameters in A
//
//               indexA  - index of the non linear parameter selected for the calculation of the derivative
//
// OUTPUT        deriv   - partial derivative of the function to the selected non linear parameter
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_DIVISION_BY_0 if the increment of the selected non linear parameter is 0
//               ERROR_ID_NO if successful
// -----------------------------------------------------------------------------

RC CurfitNumDeriv(double *X, double *specX, double *srefX, int nX, double *Y, double *sigmaY, int nY,
                  double *P, double *A, double *deltaA,int indexA,double **deriv)
 {
  // Declarations

  int      i;                                                                   // browse pixels
  double   Aj, Dj;                                                              // resp. the selected non linear parameter and its increment
  double  *Yfit1, *Yfit2;                                                       // results of the fitting function evaluated resp. for Aj-Dj et Aj+Dj
  RC       rc;                                                                  // return code

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("CurfitNumDeriv",DEBUG_FCTTYPE_APPL);
  #endif

  // Initializations

  Yfit2=NULL;
  rc=ERROR_ID_NO;

  // Buffers allocation

  if (((Yfit1=MEMORY_AllocDVector("CurfitNumDeriv","Yfit1",0,nY-1))==NULL) ||
      ((Yfit2=MEMORY_AllocDVector("CurfitNumDeriv","Yfit2",0,nY-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
   	// Buffers initialization

    memcpy(Yfit1,ANALYSE_zeros,sizeof(double)*nY);
    memcpy(Yfit2,ANALYSE_zeros,sizeof(double)*nY);

    // Get the selected non linear parameter

    Aj =(double)A[indexA];
    Dj =(double)deltaA[indexA];

    // Evaluate function for Aj+Dj

    A[indexA]=(double)Aj+Dj;
    if ((rc=ANALYSE_Function(X,specX,srefX,nX,Y,sigmaY,Yfit2,nY,P,A))>=THREAD_EVENT_STOP)
     goto EndNumDeriv;

    // Evaluate function for Aj-Dj

    A[indexA]=(double)Aj-Dj;
    if ((rc=ANALYSE_Function(X,specX,srefX,nX,Y,sigmaY,Yfit1,nY,P,A))>=THREAD_EVENT_STOP)
     goto EndNumDeriv;

     // Calculate the partial derivative of the function for the non linear parameter

    if (Dj==0.)
    	rc=ERROR_SetLast("CurfitNumDeriv",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"delta");
    else
     for (i=0;i<nY;i++)
      deriv[indexA][i]=(Yfit2[i]-Yfit1[i])/(2.*Dj);

    A[indexA]=(double)Aj;
   }

  EndNumDeriv:

  // Release allocated buffers

  if (Yfit1!=NULL)
   MEMORY_ReleaseDVector("CurfitNumDeriv","Yfit1",Yfit1,0);
  if (Yfit2!=NULL)
   MEMORY_ReleaseDVector("CurfitNumDeriv","Yfit2",Yfit2,0);

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("CurfitNumDeriv",rc);
  #endif

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      CurfitDerivFunc
// -----------------------------------------------------------------------------
// PURPOSE       Evaluate the partial derivatives of the fitting function in non linear parameters.
//               If possible, derivatives are calculated analytically in order to avoid two evaluations
//               of the fitting function.
//
// INPUT         X       - wavelengths or pixels according to the selected shift unit
//               specX   - the spectrum to evaluate
//               srefX   - the control spectrum (also called reference spectrum)
//               nX      - the size of previous vectors (depending on the size of the detector)
//
//                   !!! use whole vectors (0..NDET-1) in order to avoid side
//                   !!! effects if the shift is fitted in ANALYSE_Function
//
//               Y       - the data to fit (vector of zeros to fit the residual)
//               sigmaY  - standard deviations for Y data points
//               Yfit    - vector of calculated values of Y
//               nY      - number of data points in Y
//
//               P       - values of parameters fitted linearly in the fitting function
//               A       - values of parameters to fit non linearly
//               deltaA  - increments for non linear parameters in A
//
// OUTPUT        deriv   - partial derivative of the function to the selected non linear parameter
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_DIVISION_BY_0 if the increment of the selected non linear parameter is 0
//               ERROR_ID_NO if successful
// -----------------------------------------------------------------------------

RC CurfitDerivFunc(double *X, double *specX, double *srefX, int nX, double *Y, double *sigmaY, double *Yfit,int nY,
                   double *P, double *A, double *deltaA,double **deriv)
 {
  // Declarations

  CROSS_REFERENCE *TabCross;                                                    // the list of cross sections involved in the fitting
  int i,j0;                                                                     // indexes for loops and arrays
  RC rc;                                                                        // return code

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("CurfitDerivFunc",DEBUG_FCTTYPE_APPL);
  #endif

  // Initializations

  TabCross=Feno->TabCross;
  j0=(SvdPDeb+SvdPFin)/2;
  rc=ERROR_ID_NO;

  for (i=0;i<Feno->NTabCross;i++)
   {
    // ===============================================
    // Numeric derivatives in the following situations
    // ===============================================

    //    concentrations of the molecules in the case of SVD+Marquardt analysis method
    //    predefined parameters as offset, undersampling, raman, common residual are fitted linearly

    if (((Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVDMARQUARDT) &&
         (TabCross[i].FitConc!=ITEM_NONE) && ((rc=CurfitNumDeriv(X,specX,srefX,nX,Y,sigmaY,nY,P,A,deltaA,TabCross[i].FitConc,deriv))>=THREAD_EVENT_STOP)) ||

    // SVD : concentrations of molecules fitted linearly
    //       second derivatives of non linear parameters (predefined parameters, shift, stretch)
    //         calculated numerically

        ((TabCross[i].FitParam!=ITEM_NONE) &&
       (((i!=Feno->indexOffsetConst) &&
         (i!=Feno->indexOffsetOrder1) &&
         (i!=Feno->indexOffsetOrder2) &&
         (i!=Feno->indexCommonResidual) &&
         (i!=Feno->indexUsamp1) &&
         (i!=Feno->indexUsamp2) &&
         (i!=Feno->indexRing1)) ||
         (Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD)) &&
        ((rc=CurfitNumDeriv(X,specX,srefX,nX,Y,sigmaY,nY,P,A,deltaA,TabCross[i].FitParam,deriv))>=THREAD_EVENT_STOP)) ||

    //    derivatives of the fitting function in shift, stretch and scaling are always numeric undependantly on the method of analysis

        ((TabCross[i].FitShift!=ITEM_NONE) && ((rc=CurfitNumDeriv(X,specX,srefX,nX,Y,sigmaY,nY,P,A,deltaA,TabCross[i].FitShift,deriv))>=THREAD_EVENT_STOP)) ||
        ((TabCross[i].FitStretch!=ITEM_NONE) && ((rc=CurfitNumDeriv(X,specX,srefX,nX,Y,sigmaY,nY,P,A,deltaA,TabCross[i].FitStretch,deriv))>=THREAD_EVENT_STOP)) ||
        ((TabCross[i].FitStretch2!=ITEM_NONE) && ((rc=CurfitNumDeriv(X,specX,srefX,nX,Y,sigmaY,nY,P,A,deltaA,TabCross[i].FitStretch2,deriv))>=THREAD_EVENT_STOP)) ||
        ((TabCross[i].FitScale!=ITEM_NONE) && ((rc=CurfitNumDeriv(X,specX,srefX,nX,Y,sigmaY,nY,P,A,deltaA,TabCross[i].FitScale,deriv))>=THREAD_EVENT_STOP)) ||
        ((TabCross[i].FitScale2!=ITEM_NONE) && ((rc=CurfitNumDeriv(X,specX,srefX,nX,Y,sigmaY,nY,P,A,deltaA,TabCross[i].FitScale2,deriv))>=THREAD_EVENT_STOP)))

     goto EndCurfitDerivFunc;

   }  // End for

  EndCurfitDerivFunc:

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("CurfitDerivFunc",rc);
  #endif

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      Curfit
// -----------------------------------------------------------------------------
// PURPOSE       Make a least-squares fit to a non-linear function with a
//               linearization of the fitting function
//
// INPUT         mode    - determine the method of weighting least-squares fit
//
//                 PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL (instrumental) weight(i)=1./sigmay(i)**2
//                 PRJCT_ANLYS_FIT_WEIGHTING_NONE         (no weighting) weight(i)=1.
//
//               nFree   - the number of degrees of freedom
//
//               X       - wavelengths or pixels according to the selected shift unit
//               specX   - the spectrum to evaluate
//               srefX   - the control spectrum (also called reference spectrum)
//               nX      - the size of previous vectors (depending on the size of the detector)
//
//                   !!! use whole vectors (0..NDET-1) in order to avoid side
//                   !!! effects if the shift is fitted in ANALYSE_Function
//
//               Y       - the data to fit (vector of zeros to fit the residual)
//               sigmaY  - standard deviations for Y data points
//               nY      - number of data points in Y
//
//               P       - initial values of parameters fitted linearly in the fitting function
//
//               A       - initial values of parameters to fit non linearly
//               deltaA  - increments for non linear parameters in A
//               minA    - lower limit for fitted parameters in A
//               maxA    - upper limit for fitted parameters in A
//               nA      - number of parameters in A
//
// OUTPUT        P       - parameters fitted linearly in the fitting function
//               A       - new values for non linearly fitted parameters
//               sigmaA  - standard deviations for non linear parameters in A
//               Yfit    - vector of calculated values of Y
//               pLambda - proportion of gradient search included
//               pChisqr - reduced chi sqare for fit
//               pNiter  - number of iterations
//
// RETURN        0 if the algorithm successfully converged
//               ERROR_ID_ALLOC if the allocation of a vector failed
//               THREAD_EVENT_STOP on user intervention
//               ERROR_ID_SQRT_ARG on sqrt argument error
//               ERROR_ID_MATINV if the inversion of the matrix of non linear parameters failed
//               ERROR_ID_CONVERGENCE if the algorithm can not converge
// -----------------------------------------------------------------------------

RC Curfit(int     mode,                                                         // I   method of weighting least-squares fit
          int     nFree,                                                        // I   the number of degrees of freedom
          double *X,                                                            // I   data points for independent variable
          double *specX,                                                        // I   the spectrum to evaluate
          double *srefX,                                                        // I   the control spectrum (also called reference spectrum)
          int     nX,                                                           // I   the size of previous vectors (depending on the size of the detector)
          double *Y,                                                            // I   the data to fit (vector of zeros to fit the residual)
          double *sigmaY,                                                       // I   standard deviations for Y data points
          int     nY,                                                           // I   number of data points in Y
          double *P,                                                            // I/O parameters fitted linearly in the fitting function (ANALYSE_Function)
          double *A,                                                            // I/O parameters to fit non linearly in this function
          double *deltaA,                                                       // I   increments for non linear parameters in A
          double *sigmaA,                                                       // O   standard deviations for non linear parameters in A
          double *minA,                                                         // I   lower limit for fitted parameters in A
          double *maxA,                                                         // I   upper limit for fitted parameters in A
          int     nA,                                                           // I   number of parameters in A
          double *Yfit,                                                         // O   vector of calculated values of Y
          double *pLambda,                                                      // O   proportion of gradient search included
          double *pChisqr,                                                      // O   reduced Chi square for fit (output)
          int    *pNiter)                                                       // O   number of iterations
 {
  // Declarations

  UCHAR    string[MAX_ITEM_TEXT_LEN+1];                                         // error message string

  int      i,j,k,                                                               // indexes for loops and arrays
           outOfRange,                                                          // flag set if a parameter is out of the defined range
           niter,                                                               // the number of iterations
           mode2use;                                                            // the weighting mode

  double  *weight,                                                              // weights defined according to the weighting mode
         **deriv,                                                               // derivative of the fitting function w.r.t. fitted non linear parameters
          *B,                                                                   // the fitted non linear parameters
           chisqr,chisq1,oldChisq,                                              // reduced chi square calculated at different steps of the algorithm
         **alpha,                                                               // matrix of crossed partial derivatives also called curvature matrix because of its relationship to the curvature of the chi square in the parameter space
         **array,                                                               // matrix alpha with diagonal terms controlled by a factor lambda and non-diagonal term normalized
          *beta;                                                                // the derivative of the chi square w.r.t. each non linear parameter to fit
  double   det;                                                                 // determinant of a matrix
  RC       rc;                                                                  // return code

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("Curfit",DEBUG_FCTTYPE_APPL);
  #endif

  // Initializations

  mode2use=(sigmaY==NULL)?PRJCT_ANLYS_FIT_WEIGHTING_NONE:mode;

  B=beta=NULL;
  alpha=array=deriv=NULL;

  chisqr=(double)0.;
  niter=0;

  // Allocate needed vectors and matrices

  if (((weight=(double *)MEMORY_AllocDVector("Curfit","weight",0,nY-1))==NULL) ||
      ((B=(double *)MEMORY_AllocDVector("Curfit","B",0,nA-1))==NULL) ||
      ((beta=(double *)MEMORY_AllocDVector("Curfit","beta",0,nA-1))==NULL) ||
      ((alpha=(double **)MEMORY_AllocDMatrix("Curfit","alpha",0,nA-1,0,nA-1))==NULL) ||
      ((array=(double **)MEMORY_AllocDMatrix("Curfit","array",0,nA-1,0,nA-1))==NULL) ||
      ((deriv=(double **)MEMORY_AllocDMatrix("Curfit","deriv",0,nY-1,0,nA-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    for (j=0;j<nA;j++)
     B[j]=(double)A[j];

    // Evaluate weights

    switch (mode2use)
     {
   // -------------------------------------------------------------------------
   // case PRJCT_ANLYS_FIT_WEIGHTING_STATISTICAL :                              // This option is disabled from the original version
   //  for (k=0;k<nY;k++)                                                       // As the input Y should be set to zero,
   //   weight[k]=(double)(Y[k]==0.)?1.:(1./fabs(Y[k]));                        // PRJCT_ANLYS_FIT_WEIGHTING_STATISTICAL and PRJCT_ANLYS_FIT_WEIGHTING_NONE
   // break;                                                                    // should be the same
   // -------------------------------------------------------------------------
      case PRJCT_ANLYS_FIT_WEIGHTING_NONE :
       memcpy(weight,ANALYSE_ones,sizeof(double)*nY);
      break;
   // -------------------------------------------------------------------------
      case PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL :
       for (k=0;k<nY;k++)
        weight[k]=(double)(sigmaY[k]!=0.)?1./(sigmaY[k]*sigmaY[k]):1.;
      break;
   // -------------------------------------------------------------------------
     }

    // Gradient search : evaluate alpha and beta matrices from the partial derivatives

    for (j=0;j<nA;j++)
     {
      beta[j]=(double)0.;
      for (k=0;k<=j;k++)
       alpha[j][k]=0.;
     }

    if (((rc=CurfitDerivFunc(X,specX,srefX,nX,Y,sigmaY,Yfit,nY,P,A,deltaA,deriv))>=THREAD_EVENT_STOP) ||
        ((rc=ANALYSE_Function(X,specX,srefX,nX,Y,sigmaY,Yfit,nY,P,A))>=THREAD_EVENT_STOP))

     goto EndCurfit;

    for (i=0;i<nY;i++)
      {
       for (j=0;j<nA;j++)
        {
         beta[j]+=(double)(weight[i]*(Y[i]-Yfit[i])*deriv[j][i]);

         for (k=0;k<=j;k++)
          alpha[j][k]+=(double)(weight[i]*deriv[j][i]*deriv[k][i]);
        }
      }

    for (j=0;j<nA;j++)
     for (k=0;k<=j;k++)
      alpha[k][j]=(double)alpha[j][k];

    // Evaluate the chi Square at starting point

    chisq1=(double)Fchisq(mode,nFree,Y,Yfit,sigmaY,nY);

    // Invert modified curvature matrix to find new parameters

    do
     {
      oldChisq=chisqr;

      for (j=0;j<nA;j++)
       {
        for (k=0;k<nA;k++)
         {
          if (alpha[j][j]*alpha[k][k]<=0.)
           {
            rc=ERROR_SetLast("Curfit1",ERROR_TYPE_WARNING,ERROR_ID_SQRT_ARG,CurfitError(string,(alpha[j][j]<=0.)?j:k,A,deltaA));
            goto EndCurfit;
           }

          array[j][k]=alpha[j][k]/sqrt(alpha[j][j]*alpha[k][k]);
         }

        array[j][j]=(double)1.+*pLambda;
       }

      if (((rc=CurfitMatinv(array,nA,&det))>=THREAD_EVENT_STOP) || (det==(double)0.))
       {
       	rc=ERROR_SetLast("Curfit",ERROR_TYPE_WARNING,ERROR_ID_MATINV);
        goto EndCurfit;
       }

      for (j=0;j<nA;j++)
       {
        B[j]=A[j];
        for (k=0;k<nA;k++)
         {
          if (alpha[j][j]*alpha[k][k]<=0.)
           {
            rc=ERROR_SetLast("Curfit2",ERROR_TYPE_WARNING,ERROR_ID_SQRT_ARG,CurfitError(string,(alpha[j][j]<=0.)?j:k,A,deltaA));
            goto EndCurfit;
           }

          B[j]+=(beta[k]*array[j][k]/sqrt(alpha[j][j]*alpha[k][k]));
         }
       }

      // If the Chi square increased,increase pLambda and try again

      if ((rc=ANALYSE_Function(X,specX,srefX,nX,Y,sigmaY,Yfit,nY,P,B))>=THREAD_EVENT_STOP) goto EndCurfit;

      chisqr=(double)Fchisq(mode,nFree,Y,Yfit,sigmaY,nY);
      if (chisq1<chisqr)
       *pLambda*=(double)10.;
      niter++;

      if (niter>100)
       {
       	rc=ERROR_SetLast("Curfit",ERROR_TYPE_WARNING,ERROR_ID_CONVERGENCE,100);
        goto EndCurfit;
       }
     }
    while ((chisq1<chisqr) && (chisqr!=oldChisq));

    // Verify if the fitted parameters are not out of range

    for (j=outOfRange=0;j<nA;j++)
     {
      if (minA[j]!=maxA[j])
       {
        if (B[j]>maxA[j])
         {
          B[j]=maxA[j];
          outOfRange++;
         }

        if (B[j]<minA[j])
         {
          B[j]=minA[j];
          outOfRange++;
         }
       }
     }

    if (outOfRange)
     {
      if ((rc=ANALYSE_Function(X,specX,srefX,nX,Y,sigmaY,Yfit,nY,P,B))>=THREAD_EVENT_STOP)
       goto EndCurfit;
      chisqr=(double)Fchisq(mode,nFree,Y,Yfit,sigmaY,nY);
     }

    // Evaluate parameters and uncertainties

    for (j=0;j<nA;j++)
     {
      A[j]=(double)B[j];

      if (array[j][j]/alpha[j][j]*chisqr<=0.)
       {
       	rc=ERROR_SetLast("Curfit3",ERROR_TYPE_WARNING,ERROR_ID_SQRT_ARG,CurfitError(string,j,A,deltaA));
        goto EndCurfit;
       }

      sigmaA[j]=(double)sqrt(array[j][j]/alpha[j][j]*chisqr);                   // standard deviations
     }

    *pLambda*=0.1;
   }

  EndCurfit :

  // Release the allocated vectors and matrices

  MEMORY_ReleaseDVector("Curfit","weight",weight,0);
  MEMORY_ReleaseDVector("Curfit","B",B,0);
  MEMORY_ReleaseDVector("Curfit","beta",beta,0);

  MEMORY_ReleaseDMatrix("Curfit","deriv",deriv,0,nA-1,0);
  MEMORY_ReleaseDMatrix("Curfit","alpha",alpha,0,nA-1,0);
  MEMORY_ReleaseDMatrix("Curfit","array",array,0,nA-1,0);

  // Return

  if (pNiter!=NULL)
   *pNiter=niter;
  if (pChisqr!=NULL)
   *pChisqr=chisqr;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("Curfit",rc);
  #endif

  return rc;
 }

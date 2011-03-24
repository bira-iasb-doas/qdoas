
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  ANALYSIS PROCESSING
//  Name of module    :  ANALYSIS.C
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
//  FUNCTIONS
//
//  =================
//  UTILITY FUNCTIONS
//  =================
//
//  FNPixel - get a pixel from a wavelength;
//  ShiftVector - apply shift and stretch on vector;
//  Norm - vector norm computation
//  OrthogonalizeVector - orthogonalize a column in A matrix to a set of other columns of A;
//  Orthogonalization - orthogonalization of matrix A processing;
//
//  AnalyseLoadVector - load a vector from file;
//  VECTOR_NormalizeVector - vector normalization;
//  ANALYSE_LinFit - use svd facilities for linear regressions;
//
//  ===============================
//  SVD WORKSPACE MEMORY MANAGEMENT
//  ===============================
//
//  SVD_Free - release allocated buffers used for SVD decomposition;
//  SVD_LocalAlloc - allocate SVD matrices for the current window;
//  AnalyseSvdGlobalAlloc - global allocations;
//
//  ===============
//  ANALYSIS METHOD
//  ===============
//
//  ANALYSE_SvdInit - all parameters initialization for best Shift and Stretch determination and concentrations computation;
//  Function - cross sections and spectrum alignment using spline fitting functions and new Yfit computation;
//  NumDeriv - derivatives computation;
//  DerivFunc - set derivatives for non linear parameters;
//  ANALYSE_CurFitMethod - make a least-square fit to a non linear function;
//
//  ANALYSE_AlignReference - align reference spectrum on etalon;
//  ANALYSE_Spectrum - spectrum record analysis;
//
//  ===============
//  DATA PROCESSING
//  ===============
//
//  ANALYSE_ResetData - release and reset all data used for a project analysis;
//
//  AnalyseLoadSlit - load slit function for fwhm correction;
//  AnalyseLoadCross - load cross sections data from cross sections type tab page;
//  AnalyseLoadContinuous - load continuous functions;
//  AnalyseLoadShiftStretch - load shift and stretch for cross sections implied in SVD decomposition;
//  AnalyseLoadPredefined - load predefined parameters;
//  AnalyseLoadGaps - load gaps defined in an analysis windows;
//  AnalyseLoadRef - load reference spectra;
//
//  ANALYSE_LoadData - load data from a project;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  ANALYSE_Alloc - all analysis buffers allocation and initialization;
//  ANALYSE_Free - release buffers used for analysis;
//
//  =============
//  UNDERSAMPLING
//  =============
//
//  ANALYSE_UsampGlobalAlloc - allocate buffers (not depending on analysis windows) for the calculation of the undersampling XS
//  ANALYSE_UsampLocalAlloc - allocate buffers (depending on analysis windows) for the calculation of the undersampling XS
//  ANALYSE_UsampLocalFree - release the buffers previously allocated by the ANALYSE_UsampLocalAlloc function
//  ANALYSE_UsampGlobalFree - release the buffers previously allocated by the ANALYSE_UsampGlobalAlloc function
//  ANALYSE_UsampBuild - build undersampling cross sections during analysis process;
//  ----------------------------------------------------------------------------

//
//  HISTORY
//
//  Version 2.03 (october 2000)
//  ---------------------------
//
//  15/01/2001 - ANALYSE_CurFitMethod : fix the concentration of a molecule to the
//                                      value found in a previous window;
//
//  17/01/2001 - Function : bug with the calculation of the offset from the
//                          initial values when the offset is not fitted;
//
//  18/01/2001 - ANALYSE_XsConvolution : division by 0 with I0 real-time convolution
//                                       using the wavelength dependent slit function
//                                       determined by Kurucz;
//


#include "mediate.h"
#include "engine.h"

// ===================
// GLOBAL DECLARATIONS
// ===================

ANALYSIS_WINDOWS  *ANLYS_windowsList;       // analysis windows list
LIST_ITEM         *ANLYS_itemList;          // list of items in ListView control owned by tab pages
PROJECT *PRJCT_itemList;

DoasCh *AnlysOrthogonal[ANLYS_ORTHOGONAL_TYPE_MAX]={"None","Differential XS"};
DoasCh *AnlysStretch[ANLYS_STRETCH_TYPE_MAX]={"None","1st order","2nd order"};
DoasCh *AnlysPolynome[ANLYS_POLY_TYPE_MAX]={"None","order 0","order 1","order 2","order 3","order 4","order 5"};
DoasCh *ANLYS_crossAction[ANLYS_CROSS_ACTION_MAX]={"None","Interpolate","Convolute Std","Convolute I0","Convolute Ring"}; /* "Detector t° dependent","Strato t° dependent",*/

INT    ANALYSE_plotKurucz,ANALYSE_plotRef,ANALYSE_indexLine;
// INT    ANALYSE_maxIter=0;

INT NFeno,                             // number of analysis windows
    DimC,                              // number of columns in SVD matrix == number of symbols to take into account for SVD decomposition
    Z,                                 // number of small windows to take into account
    (*Fenetre)[2];                     // list of small windows

INT SvdPDeb,SvdPFin,                   // analysis window limits
    LimMin,LimMax,LimN;

WRK_SYMBOL *WorkSpace;                 // list of symbols in a project
INT NWorkSpace;
FENO *TabFeno,*Feno;                   // list of analysis windows in a project

double **U,*x,*Lambda,
        *ANALYSE_pixels,
        *ANALYSE_splineX,              // abscissa used for spectra, in the units selected by user
        *ANALYSE_splineX2,             // in pixels units, second derivatives of corresponding wavelengths
        *ANALYSE_absolu,               // residual spectrum
        *ANALYSE_secX,                 // residual spectrum + the contribution of a cross section for fit display
        *ANALYSE_t,                    // residual transmission in Marquardt-Levenberg not linear method
        *ANALYSE_tc,                   // residual transmission in Marquardt-Levenberg not linear method
        *ANALYSE_xsTrav,               // temporary buffer for processing on cross sections
        *ANALYSE_xsTrav2,              // cross sections second derivatives
        *ANALYSE_shift,
        *ANALYSE_zeros,
        *ANALYSE_ones;
double   ANALYSE_nFree;                // number of free degrees
double   ANALYSE_oldLatitude;

MATRIX_OBJECT ANALYSIS_slit,ANALYSIS_slitK,ANALYSIS_broAmf,O3TD;
USAMP  ANALYSE_usampBuffers;

// ===================
// STATIC DECLARATIONS
// ===================

INT NOrtho,
   *OrthoSet,
    ANALYSE_ignoreAll,
    hFilterSpecLog,hFilterRefLog,
    NDET,
                                       // description of an analysis windows

    Dim,                               // security for border effects
    DimL,                              // number of lines in SVD matrix == total number of pixels to take into account for SVD decomposition
    DimP,
    NF,NP,FAST;                        // number of non linear parameters to fit

PRJCT_FILTER *ANALYSE_plFilter,*ANALYSE_phFilter;
PRJCT_ANLYS  *pAnalysisOptions;        // analysis options
PRJCT_SLIT   *pSlitOptions;            // slit function options
PRJCT_KURUCZ *pKuruczOptions;          // Kurucz options
PRJCT_USAMP  *pUsamp;                  // undersampling options

double **A,**V,*W,**P,                 // SVD matrices
       **covar,
        *SigmaSqr,
        *Fitp,
        *FitDeltap,
        *FitMinp,
        *FitMaxp,
        *a,
        *b,
        *Sigma,
        *SplineSpec,
        *SplineRef,
         StretchFact1,StretchFact2,
         Square,
         ZM,TDET;

// QDOAS ??? INDEX AnalyseChildWindows[MAX_MDI_WINDOWS];


// Internal variables

INT KuruczUseRef=0;   // 0 if spectrum is shifted, 1 if reference is shifted
int debugResetFlag=0;
int analyseDebugMask=0;
int analyseDebugVar=0;
INDEX analyseIndexRecord;

// =================
// UTILITY FUNCTIONS
// =================

void AnalyseGetFenoLim(FENO *pFeno,INDEX *pLimMin,INDEX *pLimMax)
 {
  INT (*fenetre)[2],
        nFenetre,
        deb,fin,Dim;

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("AnalyseGetFenoLim",DEBUG_FCTTYPE_UTIL);
  #endif

  deb=Dim=0;
  fin=NDET-1;

  if (!pFeno->hidden)
   {
    fenetre=pFeno->svd.Fenetre;
    nFenetre=pFeno->svd.Z;

    deb=fenetre[0][0];
    fin=fenetre[nFenetre-1][1];

    Dim=0;

    if (!pFeno->hidden && (ANALYSE_plFilter->type!=PRJCT_FILTER_TYPE_NONE) && (ANALYSE_plFilter->type!=PRJCT_FILTER_TYPE_ODDEVEN))
     Dim+=(int)(ANALYSE_plFilter->filterSize*sqrt(ANALYSE_plFilter->filterNTimes));
    if (((!pFeno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((pFeno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
          (ANALYSE_phFilter->type!=PRJCT_FILTER_TYPE_NONE) && (ANALYSE_phFilter->type!=PRJCT_FILTER_TYPE_ODDEVEN))
     Dim+=(int)(ANALYSE_phFilter->filterSize*sqrt(ANALYSE_phFilter->filterNTimes));

    Dim=max(Dim,pAnalysisOptions->securityGap);

//    if ((deb-Dim<0) || (fin+Dim>=NDET))
//     Dim=min(deb,NDET-fin-1);
   }

  *pLimMin=max(deb-Dim,0);
  *pLimMax=min(fin+Dim,NDET-1);

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("AnalyseGetFenoLim",0);
  #endif
 }

// ---------------------------------------
// FNPixel : Get a pixel from a wavelength
// ---------------------------------------

RC FNPixel(double *lambdaVector,double lambdaValue,INT npts,INT pixelSelection)
 {
 	// Declarations

  INDEX klo,khi,rc;

  // Initialization

  rc=0;

  if (lambdaValue<=lambdaVector[0])
   rc=0;
  else if (lambdaValue>=lambdaVector[npts-1])
   rc=npts-1;
  else
   {
   	rc=(npts-1)>>1;

    for (klo=0,khi=npts-1;khi-klo>1;)
     {
      rc=(khi+klo)>>1;

      if (fabs(lambdaVector[rc]-lambdaValue)<EPSILON)
       break;

      if (lambdaVector[rc]>lambdaValue)
       khi=rc;
      else
       klo=rc;
     }

    if (fabs(lambdaVector[rc]-lambdaValue)>EPSILON)
     {
      switch(pixelSelection)
       {
     // --------------------------------------------------------------------------
       	case PIXEL_BEFORE :
       	 if ((rc>0) && (lambdaVector[rc]>lambdaValue))
       	  rc--;
       	break;
     // --------------------------------------------------------------------------
       	case PIXEL_AFTER :
       	 if ((rc<npts-1) && (lambdaVector[rc]<lambdaValue))
       	  rc++;
       	break;
     // --------------------------------------------------------------------------
       	case PIXEL_CLOSEST :

        	if ((rc>0) && (lambdaVector[rc]>lambdaValue) &&
             (lambdaVector[rc]-lambdaValue>lambdaValue-lambdaVector[rc-1]))
          rc--;
         else if ((rc<npts-1) && (lambdaVector[rc]<lambdaValue) &&
                  (lambdaValue-lambdaVector[rc]>lambdaVector[rc+1]-lambdaValue))
          rc++;

       	break;
     // --------------------------------------------------------------------------
       	default :
       	break;
     // --------------------------------------------------------------------------
       }
     }
   }

  // Return

  return rc;
 }


// ---------------------------------------------------------------------------------------
// OrthogonalizeVector : Orthogonalize a column in A matrix to a set of other columns of A
// ---------------------------------------------------------------------------------------

void OrthogonalizeVector(INT *OrthoSet,double *NormSet,INT NOrthoSet,INDEX indexColumn)
 {
  // Declarations

  INDEX indexOrthoSet,                   // browse OrthoSet vector
        indexSvd,                        // index of column in A SVD matrix
        i;
  double dot,norm;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("OrthogonalizeVector",DEBUG_FCTTYPE_UTIL);
  #endif

  // Orthogonalization

  for (indexOrthoSet=0;indexOrthoSet<NOrthoSet;indexOrthoSet++)

   if ((norm=NormSet[indexOrthoSet])!=(double)0.)
    {
     indexSvd=Feno->TabCross[OrthoSet[indexOrthoSet]].IndSvdA;
     norm=(double)1./norm;

     for (i=1,dot=(double)0.;i<=DimL;i++)
      dot+=A[indexSvd][i]*A[indexColumn][i];

     for (i=1;i<=DimL;i++)
      A[indexColumn][i]-=dot*A[indexSvd][i]*norm;
    }

//  #if defined(__DEBUG_) && __DEBUG_
//  {
//   INDEX j,l;
//   double dot;
//
//   DEBUG_Print(DOAS_logFile,"Orthogonalization :\n");
//
//   for (i=0;i<NOrthoSet;i++)
//    {
//     for (l=1,dot=(double)0.;l<=DimL;l++)
//      dot+=A[Feno->TabCross[OrthoSet[i]].IndSvdA][l]*A[indexColumn][l];
//
//     for (j=0;j<Feno->NTabCross;j++)
//      if (Feno->TabCross[j].IndSvdA==indexColumn)
//       break;
//
//     DEBUG_Print(DOAS_logFile,"%s-%s : Norm %g-%g dot %g\n",
//                 WorkSpace[Feno->TabCross[OrthoSet[i]].Comp].symbolName,
//                 WorkSpace[Feno->TabCross[j].Comp].symbolName,
//                 NormSet[i],VECTOR_Norm(A[Feno->TabCross[j].IndSvdA],DimL),dot);
//    }
//  }
//  #endif

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("OrthogonalizeVector",0);
  #endif
 }

void OrthogonalizeToCross(INT indexCross,double *NormSet,INT currentNOrtho)
 {
  // Declarations

  CROSS_REFERENCE *pTabCross;
  INDEX indexTabCross;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("OrthogonalizeToCross",DEBUG_FCTTYPE_UTIL);
  #endif

  for (indexTabCross=0;indexTabCross<Feno->NTabCross;indexTabCross++)
   {
    pTabCross=&Feno->TabCross[indexTabCross];

    if (pTabCross->IndOrthog==indexCross)
     {
      OrthoSet[currentNOrtho]=indexCross;
      NormSet[currentNOrtho]=VECTOR_Norm(A[Feno->TabCross[indexCross].IndSvdA],DimL);

      if ((ANALYSE_phFilter->filterFunction==NULL) ||
         (!Feno->hidden && !ANALYSE_phFilter->hpFilterAnalysis) ||
         ((Feno->hidden==1) && !ANALYSE_phFilter->hpFilterCalib))
       OrthogonalizeVector(OrthoSet,NormSet,currentNOrtho+1,pTabCross->IndSvdA);
      else
       OrthogonalizeVector(&OrthoSet[NOrtho],&NormSet[NOrtho],currentNOrtho-NOrtho+1,pTabCross->IndSvdA);

      OrthogonalizeToCross(indexTabCross,NormSet,currentNOrtho+1);
     }
   }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("OrthogonalizeToCross",0);
  #endif
 }

// ------------------------------------------------------------
// Orthogonalization : Orthogonalization of matrix A processing
// ------------------------------------------------------------

void Orthogonalization(void)
 {
  // Declarations

  INDEX indexOrthoSet,indexTabCross;
  CROSS_REFERENCE *pTabCross;
  double NormSet[MAX_FIT];

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("Orthogonalization",DEBUG_FCTTYPE_UTIL);
  #endif

  // Orthogonal base

  memcpy(NormSet,ANALYSE_ones,NOrtho*sizeof(double));

  for (indexOrthoSet=1;indexOrthoSet<NOrtho;indexOrthoSet++)
   {
    OrthogonalizeVector(OrthoSet,NormSet,indexOrthoSet,Feno->TabCross[OrthoSet[indexOrthoSet]].IndSvdA);
//    VECTOR_NormalizeVector(A[Feno->TabCross[OrthoSet[indexOrthoSet]].IndSvdA],DimL,&norm /* Vector norm before normalisation */,"Orthogonalization ");
    NormSet[indexOrthoSet]=VECTOR_Norm(A[Feno->TabCross[OrthoSet[indexOrthoSet]].IndSvdA],DimL);
   }

  // Orthogonalization to base only

  if ((ANALYSE_phFilter->filterFunction==NULL) ||
     (!Feno->hidden && !ANALYSE_phFilter->hpFilterAnalysis) ||
     ((Feno->hidden==1) && !ANALYSE_phFilter->hpFilterCalib))

   for (indexTabCross=0;indexTabCross<Feno->NTabCross;indexTabCross++)
    {
     pTabCross=&Feno->TabCross[indexTabCross];

     if (pTabCross->IndSvdA && (pTabCross->IndOrthog==ORTHOGONAL_BASE))
      OrthogonalizeVector(OrthoSet,NormSet,NOrtho,pTabCross->IndSvdA);
    }

  // Orthogonalization to base plus another vector

  for (indexTabCross=0;indexTabCross<Feno->NTabCross;indexTabCross++)
   {
    pTabCross=&Feno->TabCross[indexTabCross];

    if (pTabCross->IndSvdA && (pTabCross->IndOrthog==ORTHOGONAL_BASE))
     OrthogonalizeToCross(indexTabCross,NormSet,NOrtho);
   }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("Orthogonalization",0);
  #endif
 }

// =======================================
// Real time interpolation and convolution
// =======================================

// Correction of a cross section by the effective temperature
// Test 24/01/2002

RC TemperatureCorrection(double *xs,double *A,double *B,double *C,double *newXs,double T)
 {
  INDEX j;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("TemperatureCorrection",DEBUG_FCTTYPE_UTIL);
  #endif

  memcpy(newXs,ANALYSE_zeros,sizeof(double)*NDET);

  for (j=LimMin;j<=LimMax;j++)
   newXs[j]=xs[j]+(T-241)*A[j]+(T-241)*(T-241)*B[j]+(T-241)*(T-241)*(T-241)*C[j];

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("TemperatureCorrection",0);
  #endif

  return 0;
 }

// -----------------------------------------------
// ShiftVector : Apply shift and stretch on vector
// -----------------------------------------------

RC ShiftVector(double *source,double *deriv,double *target,
               double DSH,double DST,double DST2,                           // first shift and stretch
               double DSH_,double DST_,double DST2_,                        // second shift and stretch
               double *Param,INT fwhmDir,INT kuruczFlag)
 {
  // Declarations

  CROSS_REFERENCE *TabCross;
  INDEX i,j;
  double j0,lambda0;
  double x0,y,fwhm,deltaX;
  INT fwhmFlag;
  RC rc;

  // Initializations

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ShiftVector",DEBUG_FCTTYPE_APPL);
  #endif

  memcpy(ANALYSE_shift,ANALYSE_zeros,sizeof(double)*NDET);
  fwhmFlag=((Feno->analysisType==ANALYSIS_TYPE_FWHM_NLFIT) && (fwhmDir!=0) && (Param!=NULL))?1:0;
  TabCross=Feno->TabCross;

  j0=(double)(SvdPDeb+SvdPFin)*0.5;
  lambda0=(fabs(j0-floor(j0))<(double)0.1)?
                              (double)ANALYSE_splineX[(INDEX)j0]:
                              (double)0.5*(ANALYSE_splineX[(INDEX)floor(j0)]+ANALYSE_splineX[(INDEX)floor(j0+1.)]);

  rc=ERROR_ID_NO;



  // Buffer allocation for second derivative

  for (j=LimMin;j<=LimMax;j++)
   {                                                        // !! p'=p-(DSH+DST*(p-p0)+DST2*(p-p0)^2
    // Second shift and stretch                             //    p''=p'-(DSH'+DST'*(p'-p0')+DST2'*(p'-p0')^2
                                                            // with   p=ANALYSE_splineX (Lambda if unit is nm;pixels if unit is pixels)
    x0=(ANALYSE_splineX[j]-lambda0);        //        p0'=p0-DSH
    y=ANALYSE_splineX[j]-(DSH_+DST_*x0+DST2_*x0*x0);

    // First shift and stretch

    x0=(y-lambda0+DSH_);
    ANALYSE_shift[j]=y-(DSH+DST*x0*StretchFact1+DST2*x0*x0*StretchFact2);

    // Fit difference of resolution between spectrum and reference

    if (fwhmFlag)
     {
      fwhm=(double)0.;

      deltaX=(ANALYSE_splineX[j]-lambda0);

      if (Feno->indexFwhmConst!=ITEM_NONE)
       fwhm+=(TabCross[Feno->indexFwhmConst].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmConst].FitParam]:(double)TabCross[Feno->indexFwhmConst].InitParam;
      if (Feno->indexFwhmOrder1!=ITEM_NONE)
       fwhm+=((TabCross[Feno->indexFwhmOrder1].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmOrder1].FitParam]:(double)TabCross[Feno->indexFwhmOrder1].InitParam)*deltaX*TabCross[Feno->indexFwhmOrder1].Fact;
      if (Feno->indexFwhmOrder2!=ITEM_NONE)
       fwhm+=((TabCross[Feno->indexFwhmOrder2].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmOrder2].FitParam]:(double)TabCross[Feno->indexFwhmOrder2].InitParam)*deltaX*deltaX*TabCross[Feno->indexFwhmOrder2].Fact;

      // Apply shift and stretch

      if ((rc=((fwhm!=(double)0.) && (((fwhmDir>0) && (fwhm>(double)0.)) || ((fwhmDir<0) && (fwhm<(double)0.)))) ?
                XSCONV_TypeGauss(ANALYSE_splineX,source,deriv,ANALYSE_shift[j],(ANALYSE_splineX[j+1]-ANALYSE_splineX[j]),&target[j],fabs(fwhm),(double)0.,SLIT_TYPE_GAUSS):
                SPLINE_Vector(ANALYSE_splineX,source,deriv,NDET,&ANALYSE_shift[j],&target[j],1,pAnalysisOptions->interpol,"ShiftVector "))!=ERROR_ID_NO)

       break;
     }
   }

  if (kuruczFlag)
   {
    // Declarations

    double slitParam,slitParam2,slitParam3,slitParam4;
    XS xsNew;

    // Initializations

    slitParam=(Feno->indexFwhmParam[0]!=ITEM_NONE)?((TabCross[Feno->indexFwhmParam[0]].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmParam[0]].FitParam]:(double)TabCross[Feno->indexFwhmParam[0]].InitParam):(double)0.;

    if (pKuruczOptions->fwhmType==SLIT_TYPE_INVPOLY)
     slitParam2=pKuruczOptions->invPolyDegree;
    else if (((pKuruczOptions->fwhmType==SLIT_TYPE_ERF) || (pKuruczOptions->fwhmType==SLIT_TYPE_VOIGT)) && (Feno->indexFwhmParam[1]!=ITEM_NONE))
     slitParam2=(TabCross[Feno->indexFwhmParam[1]].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmParam[1]].FitParam]:(double)TabCross[Feno->indexFwhmParam[1]].InitParam;
    else
     slitParam2=(double)0.;

    if ((pKuruczOptions->fwhmType==SLIT_TYPE_VOIGT) && (Feno->indexFwhmParam[2]!=ITEM_NONE))
     slitParam3=(TabCross[Feno->indexFwhmParam[2]].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmParam[2]].FitParam]:(double)TabCross[Feno->indexFwhmParam[2]].InitParam;
    else
     slitParam3=(double)0.;

    if ((pKuruczOptions->fwhmType==SLIT_TYPE_VOIGT) && (Feno->indexFwhmParam[3]!=ITEM_NONE))
     slitParam4=(TabCross[Feno->indexFwhmParam[3]].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmParam[3]].FitParam]:(double)TabCross[Feno->indexFwhmParam[3]].InitParam;
    else
     slitParam4=(double)0.;

    // Interpolation

    if ((slitParam==(double)0.) ||
      (((pKuruczOptions->fwhmType==SLIT_TYPE_ERF) || (pKuruczOptions->fwhmType==SLIT_TYPE_INVPOLY)) &&
        (slitParam2==(double)0.)))

     rc=SPLINE_Vector(KURUCZ_buffers.hrSolar.lambda,KURUCZ_buffers.hrSolar.vector,KURUCZ_buffers.hrSolar.deriv2,KURUCZ_buffers.hrSolar.NDET,
                         ANALYSE_shift,source,NDET,pAnalysisOptions->interpol,"ShiftVector ");

    // Convolution

    else
     {
      if ((pKuruczOptions->fwhmType==SLIT_TYPE_GAUSS) || (pKuruczOptions->fwhmType==SLIT_TYPE_ERF))
       {
        double F,G,w,a,sigma,delta,step;
        INDEX i;
        INT ndemi;

        ndemi=pKURUCZ_fft->fftSize>>1;
        step=(pKURUCZ_fft->fftIn[pKURUCZ_fft->oldSize]-pKURUCZ_fft->fftIn[1])/(pKURUCZ_fft->oldSize-1.);

        sigma=slitParam*0.5;
        a=sigma/sqrt(log(2.));
        delta=slitParam2*0.5;

        w=(double)PI/step;
        F=exp(-a*a*w*w*0.25);
        G=(pKuruczOptions->fwhmType==SLIT_TYPE_GAUSS)?(double)1.:sin(w*delta)/(w*delta);

        pKURUCZ_fft->invFftIn[1]=pKURUCZ_fft->fftOut[1];
        pKURUCZ_fft->invFftIn[2]=pKURUCZ_fft->fftOut[2]*F*G;

        for (i=2;i<=ndemi;i++)
         {
          w=(double)PI*(i-1.)/(ndemi*step);

          F=(double)exp(-a*a*w*w*0.25);
          G=(double)(pKuruczOptions->fwhmType==SLIT_TYPE_GAUSS)?(double)1.:(double)sin(w*delta)/(w*delta);

          pKURUCZ_fft->invFftIn[(i<<1) /* i*2 */-1]=pKURUCZ_fft->fftOut[(i<<1) /* i*2 */-1]*F*G;      // Real part
          pKURUCZ_fft->invFftIn[(i<<1) /* i*2 */]=pKURUCZ_fft->fftOut[(i<<1) /* i*2 */]*F*G;          // Imaginary part
         }

        realft(pKURUCZ_fft->invFftIn,pKURUCZ_fft->invFftOut,pKURUCZ_fft->fftSize,-1);

        for (i=1;i<=pKURUCZ_fft->fftSize;i++)
         pKURUCZ_fft->invFftOut[i]/=step;

        SPLINE_Deriv2(pKURUCZ_fft->fftIn+1,pKURUCZ_fft->invFftOut+1,pKURUCZ_fft->invFftIn+1,pKURUCZ_fft->oldSize,"ShiftVector ");

        memcpy(&source[LimMin],&ANALYSE_shift[LimMin],sizeof(double)*LimN);

        if ((pAnalysisOptions->units==PRJCT_ANLYS_UNITS_NANOMETERS) ||
           !(rc=SPLINE_Vector(ANALYSE_splineX,Lambda,ANALYSE_splineX2,NDET,&source[LimMin],&ANALYSE_shift[LimMin],LimN,pAnalysisOptions->interpol,"ShiftVector ")))

         SPLINE_Vector(pKURUCZ_fft->fftIn+1,pKURUCZ_fft->invFftOut+1,pKURUCZ_fft->invFftIn+1,pKURUCZ_fft->oldSize,
                         &ANALYSE_shift[LimMin],&target[LimMin],LimN,pAnalysisOptions->interpol,"ShiftVector ");
       }
      else
       {
        XS slitXs;
        SLIT slitOptions;
        INT slitType;

        memset(&slitXs,0,sizeof(XS));

        xsNew.lambda=ANALYSE_shift;
        xsNew.vector=target;
        xsNew.deriv2=NULL;
        xsNew.NDET=NDET;

//        slitType=pKuruczOptions->fwhmType;

        if ((slitType=pKuruczOptions->fwhmType)==SLIT_TYPE_VOIGT)
         {
          slitOptions.slitType=slitType;
          slitOptions.slitFile[0]=0;
          slitOptions.slitParam=slitParam;
          slitOptions.slitParam2=slitParam2;
          slitOptions.slitParam3=slitParam3;
          slitOptions.slitParam4=slitParam4;

          memcpy(&source[LimMin],&ANALYSE_shift[LimMin],sizeof(double)*LimN);
          rc=XSCONV_LoadSlitFunction(&slitXs,&slitOptions,NULL,&slitType);
         }

        if (!rc && ((pAnalysisOptions->units==PRJCT_ANLYS_UNITS_NANOMETERS) ||
           !(rc=SPLINE_Vector(ANALYSE_splineX,Lambda,ANALYSE_splineX2,NDET,&source[LimMin],&ANALYSE_shift[LimMin],LimN,pAnalysisOptions->interpol,"ShiftVector "))))

         rc=XSCONV_TypeStandard(&xsNew,LimMin,LimMax+1,&KURUCZ_buffers.hrSolar,
                               (slitType==SLIT_TYPE_FILE)?&slitXs:NULL,&KURUCZ_buffers.hrSolar,NULL,
                                slitType,slitParam*3.,slitParam,slitParam2,slitParam3,slitParam4);

        XSCONV_Reset(&slitXs);
       }
     }

    if (hFilterRefLog && !(rc=SPLINE_Vector(KURUCZ_buffers.lambdaF,KURUCZ_buffers.solarF,KURUCZ_buffers.solarF2,NDET+2*KURUCZ_buffers.solarFGap,ANALYSE_shift+LimMin,source+LimMin,LimN,pAnalysisOptions->interpol,"ShiftVector ")))
     {
      for (i=LimMin;(i<=LimMax) && (source[i]>(double)0.) && (target[i]>(double)0.);i++)
       target[i]=log(target[i]/source[i]);

      if (i<=LimMax)
       rc=ERROR_SetLast("ShiftVector",ERROR_TYPE_WARNING,ERROR_ID_LOG,analyseIndexRecord);
     }
   }
  else if (!fwhmFlag)
  {
   rc=SPLINE_Vector(ANALYSE_splineX,source,deriv,NDET,&ANALYSE_shift[LimMin],&target[LimMin],LimN,pAnalysisOptions->interpol,"ShiftVector ");
  }

  // Return

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ShiftVector",rc);
  #endif

  return rc;
 }

// -----------------------------------------------------------------------------------
// ANALYSE_XsInterpolation : Interpolation of all cross sections in an analysis window
// -----------------------------------------------------------------------------------

RC ANALYSE_XsInterpolation(FENO *pTabFeno,double *newLambda)
 {
  // Declarations

  CROSS_REFERENCE *pTabCross;
  double *filtCross,*filtDeriv2;
  INDEX indexTabCross,i;
  MATRIX_OBJECT *pXs;
  INT oldNl;
  RC rc;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ANALYSE_XsInterpolation",DEBUG_FCTTYPE_APPL);
  #endif

  // Initializations

  filtCross=filtDeriv2=NULL;
  rc=ERROR_ID_NO;
  oldNl=0;

  // Browse cross sections

  for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
   {
    pTabCross=&pTabFeno->TabCross[indexTabCross];

    if ((((WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) &&   // take only cross sections into account
       ((pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING) ||
        (pTabCross->crossAction==ANLYS_CROSS_ACTION_INTERPOLATE))) ||
       ((WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_PREDEFINED) &&   // take only cross sections into account
        (pTabCross->crossAction==ANLYS_CROSS_ACTION_INTERPOLATE))))
     {
      pXs=&WorkSpace[pTabCross->Comp].xs;

      // Buffer allocation

      if (oldNl!=pTabFeno->NDET)
       {
        if (filtCross!=NULL)
         MEMORY_ReleaseDVector("ANALYSE_XsInterpolation ","filtCross",filtCross,0);
        if (filtDeriv2!=NULL)
         MEMORY_ReleaseDVector("ANALYSE_XsInterpolation ","filtDeriv2",filtDeriv2,0);

        if (((filtCross=MEMORY_AllocDVector("ANALYSE_XsInterpolation ","filtCross",0,pTabFeno->NDET-1))==NULL) ||
            ((filtDeriv2=MEMORY_AllocDVector("ANALYSE_XsInterpolation ","filtDeriv2",0,pTabFeno->NDET-1))==NULL))
         {
          rc=ERROR_ID_ALLOC;
          break;
         }

        oldNl=pTabFeno->NDET;
       }

      // Interpolate cross section

      if (!strcasecmp(WorkSpace[pTabCross->Comp].symbolName,"O3TD"))
       {
        for (i=2;i<pXs->nc;i++)

          if ((rc=SPLINE_Vector(pXs->matrix[0],
                                   pXs->matrix[i],
                                   pXs->deriv2[i],
                                   pXs->nl,newLambda,
                                   O3TD.matrix[i],pTabFeno->NDET,pAnalysisOptions->interpol,"ANALYSE_XsInterpolation "))!=0)
           break;

        if (rc)
         break;
       }

      if ((pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING) ||
         ((pXs->nl==pTabFeno->NDET) && VECTOR_Equal(pXs->matrix[0],newLambda,pTabFeno->NDET,(double)1.e-7)))          // wavelength scale is the same as new one

       memcpy(filtCross,pXs->matrix[1],sizeof(double)*pTabFeno->NDET);

      else
       if ((rc=SPLINE_Vector(pXs->matrix[0],pXs->matrix[1],pXs->deriv2[1],pXs->nl,newLambda,filtCross,pTabFeno->NDET,pAnalysisOptions->interpol,"ANALYSE_XsInterpolation "))!=0)           // interpolation processing
        break;

      //
      // FILTERING :
      //
      // Filter after interpolation (because cross sections can be high resoluted and real-time convoluted
      // The filtering before interpolation can be better to avoid interpolation error (because filtered
      // cross sections are smoother than non filtered ones) but for that it's important that cross sections are
      // pre-interpolated on the same grid as the reference one (but it's not the generality in WinDOAS).
      //
      // So ->>>>> FIRST INTERPOLATE, THEN FILTER
      //

      // Low-pass filtering of the original cross section

      if (!pTabFeno->hidden && pTabCross->filterFlag && (ANALYSE_plFilter->filterFunction!=NULL) &&
         ((rc=FILTER_Vector(ANALYSE_plFilter,filtCross,filtCross,pTabFeno->NDET,PRJCT_FILTER_OUTPUT_LOW))!=0))

       break;

      // High-pass filtering of the original cross section

      if ((pTabCross->IndOrthog!=ITEM_NONE) && (ANALYSE_phFilter->filterFunction!=NULL) &&
         ((!pTabFeno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((pTabFeno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
         ((rc=FILTER_Vector(ANALYSE_phFilter,filtCross,filtCross,pTabFeno->NDET,PRJCT_FILTER_OUTPUT_HIGH_SUB))!=0))

       break;

      // Reuse original cross section

      memcpy(pTabCross->vector,filtCross,sizeof(double)*pTabFeno->NDET);

      // Second derivatives computation

      if ((rc=SPLINE_Deriv2((pAnalysisOptions->units==PRJCT_ANLYS_UNITS_PIXELS)?ANALYSE_pixels:newLambda,
                      pTabCross->vector,pTabCross->Deriv2+1,pTabFeno->NDET,"ANALYSE_XsInterpolation "))!=0)
       break;

     }
   }

  // Return

  if (filtCross!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_XsInterpolation ","filtCross",filtCross,0);
  if (filtDeriv2!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_XsInterpolation ","filtDeriv2",filtDeriv2,0);

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ANALYSE_XsInterpolation",rc);
  #endif

  return rc;
 }

RC AnalyseConvoluteXs(FENO *pTabFeno,INDEX indexSymbol,INT action,double conc,
                      MATRIX_OBJECT *pXs,
                      MATRIX_OBJECT *pSlit,INT slitType,double *slitParam1,double *slitParam2,double *slitParam3,double *slitParam4,
                      double *newlambda,double *output,INDEX indexlambdaMin,INDEX indexlambdaMax)
 {
  // Declarations

  XS xsI0,xshr,xsSlit,hrSolar;
  double *IcVector;
  INDEX i,j;
  RC rc;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("AnalyseConvoluteXs",DEBUG_FCTTYPE_APPL);
  #endif


  // Initializations

  memset(&xsI0,0,sizeof(XS));
  memset(&xshr,0,sizeof(XS));
  memset(&xsSlit,0,sizeof(XS));
  IcVector=NULL;

  rc=ERROR_ID_NO;

  memcpy(output,ANALYSE_zeros,sizeof(double)*NDET);

  if (action==ANLYS_CROSS_ACTION_CONVOLUTE_I0)
   {
    if ((IcVector=MEMORY_AllocDVector("AnalyseConvoluteXs ","IcVector",0,NDET-1))==NULL)
     rc=ERROR_ID_ALLOC;
    else
     {
     	// Get high resolution Solar spectrum

      if (pKuruczOptions->fwhmFit && (!pTabFeno->hidden && (pTabFeno->useKurucz!=ANLYS_KURUCZ_NONE)))
       memcpy(&hrSolar,&KURUCZ_buffers.hrSolar,sizeof(XS));
      else
       {
        hrSolar.lambda=ANALYSIS_slitK.matrix[0];
        hrSolar.vector=ANALYSIS_slitK.matrix[1];
        hrSolar.deriv2=ANALYSIS_slitK.deriv2[1];
        hrSolar.NDET=ANALYSIS_slitK.nl;
       }

      if (!(rc=XSCONV_Alloc(&xsI0,hrSolar.NDET,1)) &&
          !(rc=XSCONV_Alloc(&xshr,hrSolar.NDET,1)) &&
          !(rc=SPLINE_Vector(pXs->matrix[0],pXs->matrix[1],pXs->deriv2[1],pXs->nl,           // interpolation of XS on the grid of the high resolution solar spectrum
                                hrSolar.lambda,xshr.vector,xshr.NDET,pAnalysisOptions->interpol,"AnalyseConvoluteXs ")))
       {
        memcpy(xsI0.lambda,hrSolar.lambda,xshr.NDET*sizeof(double));               // solar spectrum corrected by the cross section
        memcpy(xshr.lambda,hrSolar.lambda,xshr.NDET*sizeof(double));

        for (i=0;(i<xshr.NDET) && !rc;i++)
         {
          xsI0.vector[i]=(double)hrSolar.vector[i]*exp(-xshr.vector[i]*conc);
          xshr.vector[i]=hrSolar.vector[i];
         }

        if (!(rc=SPLINE_Deriv2(xshr.lambda,xshr.vector,xshr.deriv2,xshr.NDET,"AnalyseConvoluteXs ")))
         rc=SPLINE_Deriv2(xsI0.lambda,xsI0.vector,xsI0.deriv2,xsI0.NDET,"AnalyseConvoluteXs ");
       }
     }
   }
  else
   {
    xshr.lambda=pXs->matrix[0];
    xshr.vector=pXs->matrix[1];
    xshr.deriv2=pXs->deriv2[1];
    xshr.NDET=pXs->nl;
   }

  if (!rc)
   {
   	if (action==ANLYS_CROSS_ACTION_CONVOLUTE_I0)
   	 memcpy(IcVector,ANALYSE_zeros,sizeof(double)*NDET);

    if (pSlit!=NULL)
     {
     	if (slitType!=SLIT_TYPE_FILE)
       {
        for (j=indexlambdaMin;(j<indexlambdaMax) && !rc;j++)
         if (!(rc=XSCONV_RealTimeXs(&xshr,(action==ANLYS_CROSS_ACTION_CONVOLUTE_I0)?&xsI0:NULL,
               NULL,IcVector,
               newlambda,NDET,j,j+1,ANALYSE_xsTrav,slitType,
              (slitParam1!=NULL)?slitParam1[0]:(double)0.,
              (slitParam2!=NULL)?slitParam2[0]:(double)0.,
              (slitParam3!=NULL)?slitParam3[0]:(double)0.,
              (slitParam4!=NULL)?slitParam4[0]:(double)0.)))

          output[j]=ANALYSE_xsTrav[j];
       }
     	else if (pSlit->nc==2)
     	 {
        xsSlit.lambda=(double *)pSlit->matrix[1]+1;
        xsSlit.vector=(double *)pSlit->matrix[2]+1;
        xsSlit.deriv2=(double *)pSlit->deriv2[2]+1;
        xsSlit.NDET=pSlit->nl;

        rc=XSCONV_RealTimeXs(&xshr,(action==ANLYS_CROSS_ACTION_CONVOLUTE_I0)?&xsI0:NULL,
                             &xsSlit,IcVector,
                             newlambda,NDET,indexlambdaMin,indexlambdaMax,output,
                             slitType,*slitParam1,*slitParam2,*slitParam3,*slitParam4);
       }
      else if (!(rc=XSCONV_Alloc(&xsSlit,pSlit->nl-1,1)))
       {
       	xsSlit.lambda=(double *)pSlit->matrix[1]+2;   // base of slit matrix is 1 and there is a header line
       	xsSlit.NDET=pSlit->nl-1;

   {        // TEMPORARY !!!
   	FILE *fp;
   	DoasCh fileName[MAX_ITEM_TEXT_LEN+1];
   	DoasCh *ptr;

    FILES_RebuildFileName(fileName,WorkSpace[indexSymbol].crossFileName,1);
//   	strcpy(fileName,WorkSpace[indexSymbol].crossFileName);
   	ptr=strrchr(fileName,'.');
   	if (ptr!=NULL)
   	 strcpy(ptr,".xs_2DConv");
   	else
   	 strcat(fileName,".xs_2DConv");

   	fp=fopen(fileName,"w+t");

    // !!!   	for (j=indexlambdaMin;(j<indexlambdaMax) && !rc;j++)
        for (j=0;(j<NDET) && !rc;j++)
       	 {
       	  for (i=0;i<xsSlit.NDET;i++)
       	   xsSlit.vector[i]=(double)VECTOR_Table2(pSlit->matrix,pSlit->nl,pSlit->nc,xsSlit.lambda[i],newlambda[j]);

          if (!(rc=SPLINE_Deriv2(xsSlit.lambda,xsSlit.vector,xsSlit.deriv2,xsSlit.NDET,"AnalyseConvoluteX")) &&
              !(rc=XSCONV_RealTimeXs(&xshr,(action==ANLYS_CROSS_ACTION_CONVOLUTE_I0)?&xsI0:NULL,
                 &xsSlit,IcVector,
                 newlambda,NDET,j,j+1,ANALYSE_xsTrav,slitType,
                (double)0.,(double)0.,(double)0.,(double)0.)))
           {
            output[j]=ANALYSE_xsTrav[j];
            fprintf(fp,"%.3lf %14.6le\n",newlambda[j],output[j]);
           }
         }

       // temporary


   	fclose(fp);
   }

        XSCONV_Reset(&xsSlit);
       }
     }
    else

     for (j=indexlambdaMin;(j<indexlambdaMax) && !rc;j++)
      if (!(rc=XSCONV_RealTimeXs(&xshr,(action==ANLYS_CROSS_ACTION_CONVOLUTE_I0)?&xsI0:NULL,
            NULL,IcVector,
            newlambda,NDET,j,j+1,ANALYSE_xsTrav,slitType,
           (slitParam1!=NULL)?slitParam1[j]:(double)0.,
           (slitParam2!=NULL)?slitParam2[j]:(double)0.,
           (slitParam3!=NULL)?slitParam3[j]:(double)0.,
           (slitParam4!=NULL)?slitParam4[j]:(double)0.)))

       output[j]=ANALYSE_xsTrav[j];

    if (action==ANLYS_CROSS_ACTION_CONVOLUTE_I0)
     for (j=indexlambdaMin;(j<indexlambdaMax) && !rc;j++)
      {
      if ((IcVector[j]==(double)0.) || (conc==(double)0.))
       rc=ERROR_SetLast("AnalyseConvoluteXs",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"I0-Convolution with undefined concentration");
      else if ((double)output[j]/IcVector[j]<=(double)0.)
       rc=ERROR_SetLast("AnalyseConvoluteXs",ERROR_TYPE_WARNING,ERROR_ID_LOG,-1);
      else
       output[j]=(double)log(output[j]/IcVector[j])/conc;
      }
   }

  // Return

  if (action==ANLYS_CROSS_ACTION_CONVOLUTE_I0)
   {
    XSCONV_Reset(&xsI0);
    XSCONV_Reset(&xshr);
   }

  if (IcVector!=NULL)
   MEMORY_ReleaseDVector("AnalyseConvoluteXs ","IcVector",IcVector,0);

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("AnalyseConvoluteXs",rc);
  #endif

  return rc;
 }

// -----------------------------------------------------------------------
// XSCONV_TypeGauss : Gaussian convolution with variable half way up width
// -----------------------------------------------------------------------

RC XSCONV_TypeGauss(double *lambda,double *Spec,double *SDeriv2,double lambdaj,
                    double dldj,double *SpecConv,double fwhm,double slitParam2,INT slitType)
 {
  // Declarations

  double h,oldF,newF,Lim,ld_inc,ldi,dld,a,delta,
         lambdaMax,SpecOld, SpecNew,sigma,
         crossFIntegral, FIntegral;

  RC rc;

  // Initializations

  fwhm=fabs(fwhm);
  crossFIntegral=FIntegral=(double)0.;
  sigma=fwhm*0.5;
  a=sigma/sqrt(log(2.));
  delta=slitParam2*0.5;

  Lim=(double)2.*fwhm;

  if ((ld_inc=(double)fwhm/3.)>dldj)
   ld_inc=dldj;

  h=(double)ld_inc*0.5;

  // Browse wavelengths in the final calibration vector

  ldi=lambdaj-Lim;
  lambdaMax=lambdaj+Lim;

  // Search for first pixel in high resolution cross section in the wavelength range delimited by slit function

  dld = -(ldi-lambdaj);

  if (slitType==SLIT_TYPE_GAUSS)
//   oldF=(double)exp(-4.*log(2.)*(dld*dld)/(fwhm*fwhm));
   rc=XSCONV_FctGauss(&oldF,fwhm,ld_inc,dld);
  else if (slitType==SLIT_TYPE_INVPOLY)
   oldF=(double)pow(sigma,(double)slitParam2)/(pow(dld,(double)slitParam2)+pow(sigma,(double)slitParam2));
  else if (slitType==SLIT_TYPE_ERF)
   oldF=(double)(ERF_GetValue((dld+delta)/a)-ERF_GetValue((dld-delta)/a))/(4.*delta);

  rc=SPLINE_Vector(lambda,Spec,SDeriv2,NDET,&ldi,&SpecOld,1,SPLINE_CUBIC,"XSCONV_TypeGauss ");

  while (!rc && (ldi<=lambdaMax))
   {
    ldi += (double) ld_inc;
    dld = -(ldi-lambdaj);

    if (slitType==SLIT_TYPE_GAUSS)
//     newF=(double)exp(-4.*log(2.)*(dld*dld)/(fwhm*fwhm));
     rc=XSCONV_FctGauss(&newF,fwhm,ld_inc,dld);
    else if (slitType==SLIT_TYPE_INVPOLY)
     newF=(double)pow(sigma,(double)slitParam2)/(pow(dld,(double)slitParam2)+pow(sigma,(double)slitParam2));
    else if (slitType==SLIT_TYPE_ERF)
     newF=(double)(ERF_GetValue((dld+delta)/a)-ERF_GetValue((dld-delta)/a))/(4.*delta);

    if ((rc=SPLINE_Vector(lambda,Spec,SDeriv2,NDET,&ldi,&SpecNew,1,SPLINE_CUBIC,"XSCONV_TypeGauss "))!=0)
     break;

    crossFIntegral += (SpecOld*oldF+SpecNew*newF)*h;
    FIntegral      += (oldF+newF)*h;

    oldF=newF;
    SpecOld=SpecNew;
   }

  *SpecConv=(FIntegral!=(double)0.)?(double)crossFIntegral/FIntegral:(double)1.;

  // Return

  return rc;
 }

// -------------------------------------------------------------------------------
// ANALYSE_XsConvolution : Real time convolution of high resolution cross sections
// -------------------------------------------------------------------------------

RC ANALYSE_XsConvolution(FENO *pTabFeno,double *newlambda,
                         MATRIX_OBJECT *pSlit,INT slitType,
                         double *slitParam1,double *slitParam2,
                         double *slitParam3,double *slitParam4)
 {
  // Declarations

  MATRIX_OBJECT matrix,*pXs;
  CROSS_REFERENCE *pTabCross;
  double *raman,*solar;
  INDEX indexTabCross,j,indexlambdaMin,indexlambdaMax;
  RC rc;

  // Initializations

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ANALYSE_XsConvolution",DEBUG_FCTTYPE_APPL);
  #endif

  memset(&matrix,0,sizeof(MATRIX_OBJECT));
  raman=solar=NULL;
  rc=ERROR_ID_NO;

  AnalyseGetFenoLim(pTabFeno,&indexlambdaMin,&indexlambdaMax);
  indexlambdaMax++;

  if (pTabFeno->xsToConvolute)

   for (indexTabCross=0;(indexTabCross<pTabFeno->NTabCross) && !rc;indexTabCross++)
    {
     pTabCross=&pTabFeno->TabCross[indexTabCross];
     pXs=&WorkSpace[pTabCross->Comp].xs;

     if ((WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) &&   // take only cross sections into account
        ((pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE) ||
         (pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0) ||
         (pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_RING)))
      {
       memcpy(pTabCross->vector,ANALYSE_zeros,sizeof(double)*NDET);

       if ((pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE) || (pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0))

        rc=AnalyseConvoluteXs(pTabFeno,pTabCross->Comp,pTabCross->crossAction,pTabCross->I0Conc,pXs,pSlit,slitType,slitParam1,slitParam2,slitParam3,slitParam4,
                              newlambda,pTabCross->vector,indexlambdaMin,indexlambdaMax);

       else if ((pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_RING) &&
               !(rc=MATRIX_Allocate(&matrix,pXs->nl,2,pXs->basel,pXs->basec,1,"ANALYSE_XsConvolution ")))
        {
         // Temporary buffers allocation

         if (((raman=MEMORY_AllocDVector("ANALYSE_XsConvolution ","raman",0,NDET-1))==NULL) ||
             ((solar=MEMORY_AllocDVector("ANALYSE_XsConvolution ","solar",0,NDET-1))==NULL))

          rc=ERROR_ID_ALLOC;

         else
          {
           // Raman spectrum

           memcpy(matrix.matrix[0],pXs->matrix[0],sizeof(double)*pXs->nl);   // lambda
           memcpy(matrix.matrix[1],pXs->matrix[2],sizeof(double)*pXs->nl);   // Raman spectrum
           memcpy(matrix.deriv2[1],pXs->deriv2[2],sizeof(double)*pXs->nl);     // Second derivative of the Ramanspectrum

           if ((rc=AnalyseConvoluteXs(pTabFeno,pTabCross->Comp,ANLYS_CROSS_ACTION_CONVOLUTE,(double)0.,&matrix,pSlit,slitType,slitParam1,slitParam2,slitParam3,slitParam4,
                                      newlambda,raman,indexlambdaMin,indexlambdaMax))!=ERROR_ID_NO)
            break;

           // Solar spectrum

           memcpy(matrix.matrix[1],pXs->matrix[3],sizeof(double)*pXs->nl);   // Raman spectrum
           memcpy(matrix.deriv2[1],pXs->deriv2[3],sizeof(double)*pXs->nl);     // Second derivative of the Ramanspectrum

           if ((rc=AnalyseConvoluteXs(pTabFeno,pTabCross->Comp,ANLYS_CROSS_ACTION_CONVOLUTE,(double)0.,&matrix,pSlit,slitType,slitParam1,slitParam2,slitParam3,slitParam4,
                                      newlambda,solar,indexlambdaMin,indexlambdaMax))!=ERROR_ID_NO)
            break;

           // Calculate Raman/Solar

           for (j=indexlambdaMin;j<indexlambdaMax;j++)
            pTabCross->vector[j]=(solar[j]!=(double)0.)?(double)raman[j]/solar[j]:(double)0.;
          }
        }

       // Low-pass filtering

       if  (rc ||
          (!pTabFeno->hidden && pTabCross->filterFlag && (ANALYSE_plFilter->filterFunction!=NULL) &&
          ((rc=FILTER_Vector(ANALYSE_plFilter,pTabCross->vector,pTabCross->vector,NDET,PRJCT_FILTER_OUTPUT_LOW))!=ERROR_ID_NO)) ||

       // High-pass filtering

          ((pTabCross->IndOrthog!=ITEM_NONE) && (ANALYSE_phFilter->filterFunction!=NULL) &&
          ((!pTabFeno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((pTabFeno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
          ((rc=FILTER_Vector(ANALYSE_phFilter,pTabCross->vector,pTabCross->vector,NDET,PRJCT_FILTER_OUTPUT_HIGH_SUB))!=ERROR_ID_NO)) ||

       // Interpolation

          ((rc=SPLINE_Deriv2((pAnalysisOptions->units==PRJCT_ANLYS_UNITS_PIXELS)?ANALYSE_pixels:newlambda,
                       pTabCross->vector,pTabCross->Deriv2+1,NDET,"ANALYSE_XsConvolution "))!=ERROR_ID_NO))

        break;
      }
    }

  // Release allocated buffers

  if (raman!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_XsConvolution ","raman",raman,0);
  if (solar!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_XsConvolution ","solar",solar,0);

  MATRIX_Free(&matrix,"ANALYSE_XsConvolution ");

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ANALYSE_XsConvolution",rc);
  #endif

  // Return

  return rc;
 }

// --------------------------------------------
// AnalyseLoadVector : Load a vector from file
// --------------------------------------------

RC AnalyseLoadVector(DoasCh *function,DoasCh *fileName,double *lambda,double *vector,INT refFlag,INT *pNewSize)
 {
  // Declarations

  FILE *fp;
  DoasCh string[MAX_ITEM_TEXT_LEN+1],fullFileName[MAX_ITEM_TEXT_LEN+1],*str;
  INT day,month,year,hour,min,sec;
  struct time refTime;
  INDEX i;
  RC rc;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("AnalyseLoadVector",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  memset(string,0,MAX_ITEM_TEXT_LEN+1);
  rc=ERROR_ID_NO;

  if (strlen(FILES_RebuildFileName(fullFileName,fileName,1)) && (vector!=NULL))
   {
    if ((fp=fopen(fullFileName,"rt"))==NULL)
     rc=ERROR_SetLast("function",ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,fullFileName);
    else
     {
      for (i=0;(i<NDET) && fgets(string,MAX_ITEM_TEXT_LEN,fp);)

       if ((strchr(string,';')==NULL) && (strchr(string,'*')==NULL))
        {
         sscanf(string,"%lf %lf",&lambda[i],&vector[i]);
         i++;
        }
       else if (refFlag)
        {
         if ((str=strstr(string,"Zm"))!=NULL)
          sscanf(str,"Zm : %lf",&TabFeno[NFeno].Zm);
         else if ((str=strstr(string,"SZA"))!=NULL)
          sscanf(str,"SZA : %lf",&TabFeno[NFeno].Zm);
         else if ((str=strstr(string,"TDet"))!=NULL)
          sscanf(str,"TDet : %lf",&TabFeno[NFeno].TDet);
         else if (((str=strchr(string,'/'))!=NULL) && (*(str+3)=='/') && (*(str+11)==':') && (*(str+14)==':'))
          {
           sscanf(str-2,"%02d/%02d/%d %02d:%02d:%02d",&day,&month,&year,&hour,&min,&sec);

           refTime.ti_hour=(unsigned char)hour;
           refTime.ti_min=(unsigned char)min;
           refTime.ti_sec=(unsigned char)sec;

           TabFeno[NFeno].refDate.da_day=(char)day;
           TabFeno[NFeno].refDate.da_mon=(char)month;
           TabFeno[NFeno].refDate.da_year=(short)year;

           TabFeno[NFeno].Tm=(double)ZEN_NbSec(&TabFeno[NFeno].refDate,&refTime,0);
           TabFeno[NFeno].TimeDec=(double)hour+min/60.;
          }
        }

      if (pNewSize!=NULL)
       *pNewSize=i;

      fclose(fp);
     }
   }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("AnalyseLoadVector",rc);
  #endif

  // Return

  return rc;
 }

// ----------------------------------------------------------
// ANALYSE_LinFit : Use svd facilities for linear regressions
// ----------------------------------------------------------

RC ANALYSE_LinFit(SVD *pSvd,INT Npts,INT Degree,double *a,double *sigma,double *b,double *x)
 {
  // Declarations

  double *Norm,*bSig,mean;
  INDEX i,j;
  RC rc;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ANALYSE_LinFit",DEBUG_FCTTYPE_UTIL);
  #endif

  // Initializations

  Norm=bSig=NULL;
  bSig=NULL;
  rc=ERROR_ID_NO;

  // Buffers allocation

  if (Degree==0)
   {
    for (mean=(double)0.,i=1;i<=Npts;i++)
     mean+=b[i];

    x[1]=mean/Npts;
   }
  else if (((Norm=(double *)MEMORY_AllocDVector("ANALYSE_LinFit ","Norm",1,Degree+1))==NULL) ||
           ((bSig=(double *)MEMORY_AllocDVector("ANALYSE_LinFit ","bSig",1,Npts))==NULL))
   rc=ERROR_ID_ALLOC;
  else
   {
    // Fill svd matrix

    for (i=1;i<=Degree+1;i++)
     for (j=1;j<=Npts;j++)
      pSvd->A[i][j]=(double)pow(a[j],(double)(i-1));

    // Solution of the system weighted by errors

    for (j=1;j<=Npts;j++)
     bSig[j]=b[j];

    if (sigma!=NULL)
     for (j=1;j<=Npts;j++)
      {
       for (i=1;i<=Degree+1;i++)
        pSvd->A[i][j]/=sigma[j];
       bSig[j]/=sigma[j];
      }

    // Normalize columns in svd matrix

    for (i=1;i<=Degree+1;i++)
     if ((rc=VECTOR_NormalizeVector(pSvd->A[i],Npts,&Norm[i],"ANALYSE_LinFit "))!=ERROR_ID_NO)
      goto EndANALYSE_LinFit;

    // SVD decomposition and backsubstitution

    if (((rc=SVD_Dcmp(pSvd->A,Npts,Degree+1,pSvd->W,pSvd->V,pSvd->SigmaSqr,NULL))==ERROR_ID_NO) &&
        ((rc=SVD_Bksb(pSvd->A,pSvd->W,pSvd->V,Npts,Degree+1,bSig,x))==ERROR_ID_NO))

     for (i=1;i<=Degree+1;i++)
      x[i]/=Norm[i];
   }

  // Return

  EndANALYSE_LinFit :

  if (Norm!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_LinFit ","Norm",Norm,1);
  if (bSig!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_LinFit ","bSig",bSig,1);

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ANALYSE_LinFit",rc);
  #endif

  return rc;
 }

// ===========================
// FWHM ADJUSTMENT AND FITTING
// ===========================

// ---------------------------------------------------------------------------------------------------------
// AnalyseFwhmCorrectionT : resolution adjustment between spectrum and reference regarding their temperature
// ---------------------------------------------------------------------------------------------------------

RC AnalyseFwhmCorrectionT(double *Spectre,double *Sref,double *SpecTrav,double *RefTrav)
 {
  // Declarations

  double specFwhm,refFwhm,newFwhm,*xsTrav,*xsTrav2;
  INT slitType;
  INDEX j;
  RC rc;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("AnalyseFwhmCorrectionT",DEBUG_FCTTYPE_APPL);
  #endif

  // Initializations

  slitType=(pSlitOptions->slitFunction.slitType==SLIT_TYPE_ERF_T_FILE)?SLIT_TYPE_ERF:SLIT_TYPE_GAUSS;
  xsTrav=xsTrav2=NULL;
  rc=ERROR_ID_NO;

  // Accept only temperature dependent files for slit functions

  if ((pSlitOptions->slitFunction.slitType!=SLIT_TYPE_GAUSS_T_FILE) && (pSlitOptions->slitFunction.slitType!=SLIT_TYPE_ERF_T_FILE))
   rc=ERROR_SetLast("AnalyseFwhmCorrectionT",ERROR_TYPE_FATAL,ERROR_ID_SLIT_T);

  // Buffer allocation

  else if (((xsTrav=(double *)MEMORY_AllocDVector("AnalyseFwhmCorrectionT ","xsTrav",0,NDET-1))==NULL) ||
           ((xsTrav2=(double *)MEMORY_AllocDVector("AnalyseFwhmCorrectionT ","xsTrav2",0,NDET-1))==NULL))
   rc=ERROR_ID_ALLOC;

  // Second derivatives computation for spectrum and reference

  else if (!(rc=SPLINE_Deriv2(Lambda,Spectre,SplineSpec,NDET,"AnalyseFwhmCorrectionT (Lambda) ")) &&
           !(rc=SPLINE_Deriv2(Lambda,Sref,SplineRef,NDET,"AnalyseFwhmCorrectionT (Lambda) ")))
   {
    memcpy(xsTrav,ANALYSE_zeros,sizeof(double)*NDET);
    VECTOR_Init(xsTrav2,(pSlitOptions->slitFunction.slitType==SLIT_TYPE_GAUSS_T_FILE)?(double)0.:pSlitOptions->slitFunction.slitParam2,NDET);

    // Fwhm ajustment between spectrum and reference

    for (j=LimMin;j<=LimMax;j++)
     {
      // Retrieve fwhm from fwhm table wavelength and temperature dependent

      specFwhm=(double)VECTOR_Table2(ANALYSIS_slit.matrix,ANALYSIS_slit.nl,ANALYSIS_slit.nc,TDET,Lambda[j]);
      refFwhm=(double)VECTOR_Table2(ANALYSIS_slit.matrix,ANALYSIS_slit.nl,ANALYSIS_slit.nc,Feno->TDet,Lambda[j]);
      newFwhm=(specFwhm!=refFwhm)?sqrt(fabs(specFwhm*specFwhm-refFwhm*refFwhm)):specFwhm;
      xsTrav[j]=(specFwhm>=refFwhm)?specFwhm:refFwhm;

      // Apply fwhm correction on spectrum with the lower resolution

      if (Feno->analysisType==ANALYSIS_TYPE_FWHM_CORRECTION)
       {
        // Case 1 : reference has highest resolution => degrade reference

        if (specFwhm>refFwhm)
         rc=XSCONV_TypeGauss(Lambda,Sref,SplineRef,Lambda[j],(Lambda[j+1]-Lambda[j]),&RefTrav[j],newFwhm,(double)0.,slitType);

        // Case 2 : spectrum has highest resolution => degrade spectrum

        else if (specFwhm<refFwhm)
         rc=XSCONV_TypeGauss(Lambda,Spectre,SplineSpec,Lambda[j],(Lambda[j+1]-Lambda[j]),&SpecTrav[j],newFwhm,(double)0.,slitType);
       }
     }
   }

  // Real time convolution for high resolution cross sections

  if (!rc && Feno->xsToConvolute)
   rc=ANALYSE_XsConvolution(Feno,Lambda,NULL,slitType,xsTrav,xsTrav2,xsTrav,xsTrav2);

  // Return

  if (xsTrav!=NULL)
   MEMORY_ReleaseDVector("AnalyseFwhmCorrectionT ","xsTrav",xsTrav,0);
  if (xsTrav2!=NULL)
   MEMORY_ReleaseDVector("AnalyseFwhmCorrectionT ","xsTrav2",xsTrav2,0);

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("AnalyseFwhmCorrectionT",rc);
  #endif

  return rc;
 }

// ----------------------------------------------------------------------------------------------------------
// AnalyseFwhmCorrectionK : resolution adjustment between spectrum and reference using fwhms fitted by Kurucz
// ----------------------------------------------------------------------------------------------------------

RC AnalyseFwhmCorrectionK(double *Spectre,double *Sref,double *SpecTrav,double *RefTrav)
 {
  // Declarations

  double specFwhm,refFwhm,*xsTrav;
  INDEX j;
  RC rc;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("AnalyseFwhmCorrectionK",DEBUG_FCTTYPE_APPL);
  #endif

  // Initializations

  xsTrav=NULL;

  // Function specified for fitting fwhm in Kurucz should be supported

  if ((pKuruczOptions->fwhmType!=SLIT_TYPE_GAUSS) && (pKuruczOptions->fwhmType!=SLIT_TYPE_ERF))
   rc=ERROR_SetLast("AnalyseFwhmCorrectionK",ERROR_TYPE_FATAL,ERROR_ID_SLIT);

  // Buffer allocation

  else if ((xsTrav=(double *)MEMORY_AllocDVector("AnalyseFwhmCorrectionK ","xsTrav",0,NDET-1))==NULL)
   rc=ERROR_ID_ALLOC;

  // Second derivatives computation for spectrum and reference

  else if (!(rc=SPLINE_Deriv2(Lambda,Spectre,SplineSpec,NDET,"AnalyseFwhmCorrectionK (Lambda) ")) &&
           !(rc=SPLINE_Deriv2(Lambda,Sref,SplineRef,NDET,"AnalyseFwhmCorrectionK (Lambda) ")))
   {
    memcpy(xsTrav,ANALYSE_zeros,sizeof(double)*NDET);

    // Fwhm ajustment between spectrum and reference

    for (j=LimMin;(j<=LimMax) && !rc;j++)
     {
      // Retrieve fwhm from fwhm vectors build by Kurucz procedure

      specFwhm=KURUCZ_buffers.fwhmVector[0][j];
      refFwhm=Feno->fwhmVector[0][j];

      if ((specFwhm<=(double)0.) || (refFwhm<=(double)0.))
       rc=ERROR_SetLast("AnalyseFwhmCorrectionK",ERROR_TYPE_WARNING,ERROR_ID_SQRT_ARG);

      // Case 1 : reference has highest resolution => degrade reference

      else if (specFwhm>refFwhm)
       {
        xsTrav[j]=specFwhm;
        specFwhm=sqrt(specFwhm*specFwhm-refFwhm*refFwhm);
        rc=XSCONV_TypeGauss(Lambda,Sref,SplineRef,Lambda[j],(Lambda[j+1]-Lambda[j]),&RefTrav[j],specFwhm,
                           (Feno->fwhmVector[1]!=NULL)?Feno->fwhmVector[1][j]:(double)0.,pKuruczOptions->fwhmType);
       }

      // Case 2 : spectrum has highest resolution => degrade spectrum

      else if (specFwhm<refFwhm)
       {
        xsTrav[j]=refFwhm;
        specFwhm=sqrt(refFwhm*refFwhm-specFwhm*specFwhm);
        rc=XSCONV_TypeGauss(Lambda,Spectre,SplineSpec,Lambda[j],(Lambda[j+1]-Lambda[j]),&SpecTrav[j],specFwhm,
                           (Feno->fwhmVector[1]!=NULL)?Feno->fwhmVector[1][j]:(double)0.,pKuruczOptions->fwhmType);
       }

      // Case 3 : spectrum and reference have the same resolution

      else
       xsTrav[j]=specFwhm;
     }
   }

  // Real time convolution for high resolution cross sections

  if (!rc && Feno->xsToConvolute)
   rc=ANALYSE_XsConvolution(Feno,Lambda,NULL,pKuruczOptions->fwhmType,xsTrav,KURUCZ_buffers.fwhmVector[1],KURUCZ_buffers.fwhmVector[2],KURUCZ_buffers.fwhmVector[3]);

  // Return

  if (xsTrav!=NULL)
   MEMORY_ReleaseDVector("AnalyseFwhmCorrectionK ","xsTrav",xsTrav,0);

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("AnalyseFwhmCorrectionK",rc);
  #endif

  return rc;
 }

// ===============================
// SVD WORKSPACE MEMORY MANAGEMENT
// ===============================

// ===============================
// SVD WORKSPACE MEMORY MANAGEMENT
// ===============================

// ----------------------------------------------------------------------
// ANALYSE_SvdFree : Release allocated buffers used for SVD decomposition
// ----------------------------------------------------------------------

void ANALYSE_SvdFree(DoasCh *callingFunctionShort,SVD *pSvd)
 {
  // Declaration

  DoasCh functionNameShort[MAX_STR_SHORT_LEN+1];

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ANALYSE_SvdFree",DEBUG_FCTTYPE_MEM);
  #endif

  // Initialization

  memset(functionNameShort,0,MAX_STR_SHORT_LEN+1);

  // Build complete function name

  if (strlen(callingFunctionShort)<=MAX_STR_SHORT_LEN-strlen("ANALYSE_SvdFree via  "))
   sprintf(functionNameShort,"ANALYSE_SvdFree via %s ",callingFunctionShort);
  else
   sprintf(functionNameShort,"ANALYSE_SvdFree ");

  // Release allocated buffers

  if (pSvd->A!=NULL)
   MEMORY_ReleaseDMatrix(functionNameShort,"A",pSvd->A,0,pSvd->DimC,1);
  if (pSvd->U!=NULL)
   MEMORY_ReleaseDMatrix(functionNameShort,"U",pSvd->U,0,pSvd->DimC,1);
  if (pSvd->P!=NULL)
   MEMORY_ReleaseDMatrix(functionNameShort,"P",pSvd->P,0,pSvd->DimP,1);
  if (pSvd->V!=NULL)
   MEMORY_ReleaseDMatrix(functionNameShort,"V",pSvd->V,1,pSvd->DimC,1);
  if (pSvd->W!=NULL)
   MEMORY_ReleaseDVector(functionNameShort,"W",pSvd->W,1);
  if (pSvd->SigmaSqr!=NULL)
   MEMORY_ReleaseDVector(functionNameShort,"SigmaSqr",pSvd->SigmaSqr,0);
  if (pSvd->covar!=NULL)
   MEMORY_ReleaseDMatrix(functionNameShort,"covar",pSvd->covar,1,pSvd->DimC,1);

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ANALYSE_SvdFree",0);
  #endif
 }

// --------------------------------------------------------------------
//  : Allocate SVD matrices for the current window
// --------------------------------------------------------------------

RC ANALYSE_SvdLocalAlloc(DoasCh *callingFunctionShort,SVD *pSvd)
 {
  // Declarations

  DoasCh functionNameShort[MAX_STR_SHORT_LEN+1];
  INDEX i,j;
  RC rc;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ANALYSE_SvdLocalAlloc",DEBUG_FCTTYPE_MEM);
  #endif

  // Initializations

  memset(functionNameShort,0,MAX_STR_SHORT_LEN+1);
  rc=ERROR_ID_NO;

  // Build complete function name

  if (strlen(callingFunctionShort)<=MAX_STR_SHORT_LEN-strlen("ANALYSE_SvdLocalAlloc via  "))
   sprintf(functionNameShort,"ANALYSE_SvdLocalAlloc via %s ",callingFunctionShort);
  else
   sprintf(functionNameShort,"ANALYSE_SvdLocalAlloc ");

  // Allocation

  if (pSvd->DimC && pSvd->DimL)
   {
    if (((pSvd->A=(double **)MEMORY_AllocDMatrix("ANALYSE_SvdLocalAlloc","A",1,pSvd->DimL,0,pSvd->DimC))==NULL) ||
        ((pSvd->U=(double **)MEMORY_AllocDMatrix("ANALYSE_SvdLocalAlloc","U",1,pSvd->DimL,0,pSvd->DimC))==NULL) ||
        ((pSvd->V=(double **)MEMORY_AllocDMatrix("ANALYSE_SvdLocalAlloc","V",1,pSvd->DimC,1,pSvd->DimC))==NULL) ||
        ((pSvd->covar=(double **)MEMORY_AllocDMatrix("ANALYSE_SvdLocalAlloc","covar",1,pSvd->DimC,1,pSvd->DimC))==NULL) ||
        ((pSvd->W=(double *)MEMORY_AllocDVector("ANALYSE_SvdLocalAlloc","W",1,pSvd->DimC))==NULL) ||
        ((pSvd->SigmaSqr=(double *)MEMORY_AllocDVector("ANALYSE_SvdLocalAlloc","SigmaSqr",0,pSvd->DimC))==NULL) ||
        ((pSvd->DimP>0) && ((pSvd->P=(double **)MEMORY_AllocDMatrix("ANALYSE_SvdLocalAlloc","P",1,pSvd->DimL,0,pSvd->DimP))==NULL)))

     rc=ERROR_ID_ALLOC;

    else

    // Initializations

     {
     	for (i=1;i<=pSvd->DimC;i++)
     	 {
     	 	for (j=1;j<=pSvd->DimC;j++)
     	 	 pSvd->V[i][j]=pSvd->covar[i][j]=(double)0.;
     	 	pSvd->W[i]=pSvd->SigmaSqr[i]=(double)0.;
     	 }

      for (i=1;i<=pSvd->DimL;i++)
       pSvd->A[0][i]=pSvd->U[0][i]=(double)0.;

      if (pSvd->P!=NULL)
       for (i=1;i<=pSvd->DimL;i++)
        pSvd->P[0][i]=(double)0.;

      pSvd->SigmaSqr[0]=(double)0.;
     }
   }
  else
   rc=ERROR_SetLast(functionNameShort,ERROR_TYPE_FATAL,ERROR_ID_ALLOC,"DimC or DimL is zero !");

  // Return

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ANALYSE_SvdLocalAlloc",rc);
  #endif

  return rc;
 }

// ------------------------------------------
// AnalyseSvdGlobalAlloc : Global allocations
// ------------------------------------------

RC AnalyseSvdGlobalAlloc(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  INDEX i;
  RC rc;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("AnalyseSvdGlobalAlloc",DEBUG_FCTTYPE_MEM);
  #endif

  // Initialization

  rc=ERROR_ID_NO;

  // Allocation

  if (((Fitp=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","Fitp",0,MAX_FIT*4))==NULL)  ||
      ((FitDeltap=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","FitDeltap",0,MAX_FIT*4))==NULL) ||
      ((FitMinp=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","FitMinp",0,MAX_FIT*4))==NULL) ||
      ((FitMaxp=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","FitMaxp",0,MAX_FIT*4))==NULL) ||
      ((a=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","a",1,NDET))==NULL) ||
      ((b=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","b",1,NDET))==NULL) ||
      ((x=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","x",0,MAX_FIT))==NULL) ||
      ((Sigma=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","Sigma",0,MAX_FIT))==NULL) ||
      ((ANALYSE_shift=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","ANALYSE_shift",0,NDET-1))==NULL) ||
      ((ANALYSE_pixels=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","ANALYSE_pixels",0,NDET-1))==NULL) ||
      ((ANALYSE_splineX=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","ANALYSE_splineX",0,NDET-1))==NULL) ||
      ((ANALYSE_splineX2=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","ANALYSE_splineX2",0,NDET-1))==NULL) ||
      ((ANALYSE_absolu=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","ANALYSE_absolu",0,NDET))==NULL) ||
      ((ANALYSE_t=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","ANALYSE_t",0,NDET))==NULL) ||
      ((ANALYSE_tc=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","ANALYSE_tc",0,NDET))==NULL) ||
      ((ANALYSE_xsTrav=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","ANALYSE_xsTrav",0,NDET))==NULL) ||
      ((ANALYSE_xsTrav2=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","ANALYSE_xsTrav2",1,NDET))==NULL) ||
      ((ANALYSE_secX=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","ANALYSE_secX",0,NDET))==NULL) ||
      ((SplineSpec=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","SplineSpec",0,NDET-1))==NULL) ||
      ((SplineRef=(double *)MEMORY_AllocDVector("AnalyseSvdGlobalAlloc ","SplineRef",0,NDET-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else

  // Initializations

   {
    Sigma[0]=x[0]=(double)0.;

    for (i=0;i<NDET;i++)
     ANALYSE_pixels[i]=(double)(i+1);
   }

  // Return

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("AnalyseSvdGlobalAlloc",rc);
  #endif

  return rc;
 }

// ===============
// ANALYSIS METHOD
// ===============

// --------------------------------------------------------------------------
// ANALYSE_SvdInit : All parameters initialization for best Shift and Stretch
//                   determination and concentrations computation
// --------------------------------------------------------------------------

RC ANALYSE_SvdInit(SVD *pSvd)
 {
  // Declarations

  CROSS_REFERENCE *pTabCross;
  double deltaX,norm,norm1,norm2,*vector,*lambda,swap,temp;
  INDEX i,j,k,l;
  double j0,lambda0;
  RC rc;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ANALYSE_SvdInit",DEBUG_FCTTYPE_APPL);
  #endif

  // Initializations

  memcpy(ANALYSE_splineX,(pAnalysisOptions->units==PRJCT_ANLYS_UNITS_NANOMETERS)?Lambda:ANALYSE_pixels,sizeof(double)*NDET);

  if (!(rc=SPLINE_Deriv2(ANALYSE_pixels,Lambda,ANALYSE_splineX2,NDET,"ANALYSE_SvdInit ")))
   {
    OrthoSet=Feno->OrthoSet;
    NOrtho=Feno->NOrtho;

    A=pSvd->A;
    U=pSvd->U;
    V=pSvd->V;
    W=pSvd->W;
    P=pSvd->P;

    SigmaSqr=pSvd->SigmaSqr;
    covar=pSvd->covar;

    DimL=pSvd->DimL;
    DimC=pSvd->DimC;
    DimP=pSvd->DimP;
    NF=pSvd->NF;
    NP=pSvd->NP;

    temp=(double)0.;

    if (!Feno->hidden && (ANALYSE_plFilter->type!=PRJCT_FILTER_TYPE_NONE))
     temp+=ANALYSE_plFilter->filterEffWidth;

//    for high-pass filters, don't account for the filter width in the calculation of the number of freedom
//    if ((ANALYSE_phFilter->type!=PRJCT_FILTER_TYPE_NONE) && (ANALYSE_phFilter->type!=PRJCT_FILTER_TYPE_ODDEVEN))
//     temp+=ANALYSE_phFilter->filterEffWidth;

    ANALYSE_nFree=floor(DimL/((temp!=(double)0.)?temp:(double)1.)+0.5)-pSvd->nFit;

    if (ANALYSE_nFree<=(double)0.)
     rc=ERROR_SetLast("SvdInit",ERROR_TYPE_FATAL,ERROR_ID_NFREE);
    else
     {
      Fenetre=pSvd->Fenetre;
      Z=pSvd->Z;

      SvdPDeb=Fenetre[0][0];
      SvdPFin=Fenetre[Z-1][1];

      j0=(double)(SvdPDeb+SvdPFin)*0.5;
      lambda0=(fabs(j0-floor(j0))<(double)0.1)?
                                  (double)ANALYSE_splineX[(INDEX)j0]:
                                  (double)0.5*(ANALYSE_splineX[(INDEX)floor(j0)]+ANALYSE_splineX[(INDEX)floor(j0+1.)]);

      Dim=0;

      if (!Feno->hidden && (ANALYSE_plFilter->type!=PRJCT_FILTER_TYPE_NONE) && (ANALYSE_plFilter->type!=PRJCT_FILTER_TYPE_ODDEVEN))
       Dim+=(int)((ANALYSE_plFilter->filterSize)*sqrt(ANALYSE_plFilter->filterNTimes));
      if (((!Feno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((Feno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
            (ANALYSE_phFilter->type!=PRJCT_FILTER_TYPE_NONE) && (ANALYSE_phFilter->type!=PRJCT_FILTER_TYPE_ODDEVEN))
       Dim+=(int)((ANALYSE_phFilter->filterSize)*sqrt(ANALYSE_phFilter->filterNTimes));

      Dim=max(Dim,pAnalysisOptions->securityGap); // !!!!!!!!!!!!!!!!!

//      if ((SvdPDeb-Dim<0) || (SvdPFin+Dim>=NDET))
//       Dim=min(SvdPDeb,NDET-SvdPFin-1);

      LimMin=max(SvdPDeb-Dim,0);
      LimMax=min(SvdPFin+Dim,NDET-1);

      LimN=LimMax-LimMin+1;

      // Set non linear normalization factors

      for (j=0,k=1,norm1=norm2=(double)0.;(j<Z);j++)
       for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
        {
         deltaX=(double)(ANALYSE_splineX[l]-lambda0)*(ANALYSE_splineX[l]-lambda0);

         norm1+=deltaX;
         norm2+=deltaX*deltaX;
        }

      for (j=LimMin,StretchFact1=StretchFact2=(double)0.;j<=LimMax;j++)
       {
        deltaX=(ANALYSE_splineX[j]-lambda0);

        deltaX=ANALYSE_splineX[j]-lambda0-Feno->Stretch*deltaX-Feno->Stretch2*deltaX*deltaX;
        deltaX*=deltaX;

        StretchFact1+=deltaX;
        StretchFact2+=deltaX*deltaX;
       }

      if ((norm1<=(double)0.) || (norm2<=(double)0.) ||
          (StretchFact1<=(double)0.) || (StretchFact2<=(double)0.))

       rc=ERROR_SetLast("SvdInit",ERROR_TYPE_FATAL,ERROR_ID_SQRT_ARG);

      else
       {
        StretchFact1=(double)1./sqrt(StretchFact1);
        StretchFact2=(double)1./sqrt(StretchFact2);

        for (i=0,norm1=(double)sqrt(norm1),norm2=(double)sqrt(norm2);i<Feno->NTabCross;i++)
         {
          pTabCross=&Feno->TabCross[i];

          // Normalization of non linear parameters

          if ((i==Feno->indexOffsetOrder1) || (i==Feno->indexFwhmOrder1))
           pTabCross->Fact=norm1;
          else if ((i==Feno->indexOffsetOrder2) || (i==Feno->indexFwhmOrder2))
           pTabCross->Fact=norm2;

          pTabCross->InitStretch/=StretchFact1;
          pTabCross->InitStretch2/=StretchFact2;

          // Fill, 'Fit' vectors with data on parameters to fit

       // ---------------------------------------------------------------------------
          if ((Feno->analysisMethod!=PRJCT_ANLYS_METHOD_SVD) && (pTabCross->FitConc!=ITEM_NONE))
           {
            if (pTabCross->InitConc!=(double)0.)
             {
              lambda=WorkSpace[pTabCross->Comp].xs.matrix[0];    // NB : reuse original matrix because of possible real time
              vector=WorkSpace[pTabCross->Comp].xs.matrix[1];    //      convolution of high resolution cross sections

              for (j=0,norm=(double)0.;j<WorkSpace[pTabCross->Comp].xs.nl;j++)
               if ((lambda[j]>=Feno->Lambda[SvdPDeb]) && (lambda[j]<=Feno->Lambda[SvdPFin]))  // compute norm only in the analysis window lambda range otherwise nonsense
                norm+=vector[j]*vector[j];
             }
            else
             norm=(double)1.;

            Fitp[pTabCross->FitConc]=(norm!=(double)0.)?pTabCross->InitConc*sqrt(norm):(double)0.;
            FitDeltap[pTabCross->FitConc]=pTabCross->DeltaConc;
            FitMinp[pTabCross->FitConc]=(double)0.;
            FitMaxp[pTabCross->FitConc]=(double)0.;
           }
       // ---------------------------------------------------------------------------
          if ((pTabCross->FitParam!=ITEM_NONE) && !pTabCross->IndSvdP)
           {
            Fitp[pTabCross->FitParam]=pTabCross->InitParam;
            FitDeltap[pTabCross->FitParam]=pTabCross->DeltaParam;
            FitMinp[pTabCross->FitParam]=pTabCross->MinParam;
            FitMaxp[pTabCross->FitParam]=pTabCross->MaxParam;
           }
       // ---------------------------------------------------------------------------
          if (pTabCross->FitShift!=ITEM_NONE)
           {
            Fitp[pTabCross->FitShift]=pTabCross->InitShift;
            FitDeltap[pTabCross->FitShift]=pTabCross->DeltaShift;
            FitMinp[pTabCross->FitShift]=pTabCross->MinShift;
            FitMaxp[pTabCross->FitShift]=pTabCross->MaxShift;
           }
       // ---------------------------------------------------------------------------
          if (pTabCross->FitStretch!=ITEM_NONE)
           {
            Fitp[pTabCross->FitStretch]=pTabCross->InitStretch;
            FitDeltap[pTabCross->FitStretch]=pTabCross->DeltaStretch;
            FitMinp[pTabCross->FitStretch]=(double)0.;
            FitMaxp[pTabCross->FitStretch]=(double)0.;
           }
       // ---------------------------------------------------------------------------
          if (pTabCross->FitStretch2!=ITEM_NONE)
           {
            Fitp[pTabCross->FitStretch2]=pTabCross->InitStretch2;
            FitDeltap[pTabCross->FitStretch2]=pTabCross->DeltaStretch2;
            FitMinp[pTabCross->FitStretch2]=(double)0.;
            FitMaxp[pTabCross->FitStretch2]=(double)0.;
           }
       // ---------------------------------------------------------------------------
          if (pTabCross->FitScale!=ITEM_NONE)
           {
            Fitp[pTabCross->FitScale]=pTabCross->InitScale;
            FitDeltap[pTabCross->FitScale]=pTabCross->DeltaScale;
            FitMinp[pTabCross->FitScale]=(double)0.;
            FitMaxp[pTabCross->FitScale]=(double)0.;
           }
       // ---------------------------------------------------------------------------
          if (pTabCross->FitScale2!=ITEM_NONE)
           {
            Fitp[pTabCross->FitScale2]=pTabCross->InitScale2;
            FitDeltap[pTabCross->FitScale2]=pTabCross->DeltaScale2;
            FitMinp[pTabCross->FitScale2]=(double)0.;
            FitMaxp[pTabCross->FitScale2]=(double)0.;
           }
       // ---------------------------------------------------------------------------
         }

        for (i=0;i<NF;i++)
         {
          if ((FitMinp[i]!=(double)0.) && (FitMinp[i]==FitMaxp[i]))
           FitMinp[i]=-FitMaxp[i];
          if (FitMinp[i]>FitMaxp[i])
           {
            swap=FitMinp[i];
            FitMinp[i]=FitMaxp[i];
            FitMaxp[i]=swap;
           }
         }

        FAST=!NP;
       }
     }
   }

  // Return

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ANALYSE_SvdInit",rc);
  #endif

  return rc;
 }

// ----------------------------------------------------------
// ANALYSE_AlignReference : Align reference spectrum on etalon
// ----------------------------------------------------------

RC ANALYSE_AlignReference(ENGINE_CONTEXT *pEngineContext,INT refFlag,INT saveFlag,void *responseHandle)

//
//  refFlag==0 : GB, file mode selection        refFlag==2 : GOME, refN
//  refFlag==1 : GB, automatic mode selection   refFlag==3 : GOME, refS
//

 {
  // Declarations

  FENO *RefTabFeno;                                                             // copy of analysis windows for reference use
  SVD *pSvd;                                                                    // pointer to svd environment of an analysis window
  CROSS_RESULTS *pResults;                                                      // pointer to the set of results relative to a symbol
  INDEX WrkFeno,                                                                // index on analysis windows
        indexLine,indexColumn,                                                  // position in the spreadsheet for information to write
        indexPage,
        i,j;                                                                    // indexes for loops and arrays

  double *Spectre,*Sref,                                                        // raw spectrum
          x0,lambda0;
  plot_data_t spectrumData[2];
  DoasCh string[MAX_ITEM_TEXT_LEN+1],tabTitle[MAX_ITEM_TEXT_LEN+1];
  RC rc;                                                                        // return code


  // Initializations

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ANALYSE_AlignReference",DEBUG_FCTTYPE_APPL);
  #endif

  // Initializations

  indexColumn=2;
  rc=ERROR_ID_NO;

  // Buffers allocation

  if ((RefTabFeno=(FENO *)MEMORY_AllocBuffer("ANALYSE_AlignReference ","RefTabFeno",MAX_FENO,sizeof(FENO),0,MEMORY_TYPE_STRUCT))==NULL)
   rc=ERROR_ID_ALLOC;

  else
   {
    memcpy(RefTabFeno,TabFeno,sizeof(FENO)*MAX_FENO);
    memcpy(ANALYSE_absolu,ANALYSE_zeros,sizeof(double)*NDET);
    memcpy(ANALYSE_t,ANALYSE_zeros,sizeof(double)*NDET);
    memcpy(ANALYSE_tc,ANALYSE_zeros,sizeof(double)*NDET);

    for (WrkFeno=0;(WrkFeno<NFeno)&&!rc;WrkFeno++)
     {
     	indexPage=(pEngineContext->satelliteFlag)?plotPageRef:WrkFeno+plotPageAnalysis;

      Feno=&RefTabFeno[WrkFeno];
      Feno->Shift=Feno->Stretch=Feno->Stretch2=(double)0.;
      pResults=&Feno->TabCrossResults[Feno->indexSpectrum];
      NDET=Feno->NDET;

      memcpy(Feno->Lambda,Feno->LambdaRef,sizeof(double)*NDET);

      if (refFlag==2)
       {
        Sref=Feno->SrefN;
        Lambda=Feno->Lambda;
        Feno->refNormFact=Feno->refNormFactN;
       }
      else if (refFlag==3)
       {
        Sref=Feno->SrefS;
        Lambda=Feno->Lambda;
        Feno->refNormFact=Feno->refNormFactS;
       }
      else
       {
        Sref=Feno->Sref;
        Lambda=Feno->Lambda;
       }

      if (!Feno->hidden && (Feno->useKurucz!=ANLYS_KURUCZ_NONE) && (Feno->useKurucz!=ANLYS_KURUCZ_SPEC) && (Feno->useKurucz!=ANLYS_KURUCZ_REF_AND_SPEC) &&
        ((!refFlag && (Feno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE)) ||
          (refFlag && (Feno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC))) &&
           Feno->useEtalon && !VECTOR_Equal(Feno->SrefEtalon,Sref,NDET,(double)0.) &&
         ((refFlag!=3) || !VECTOR_Equal(Feno->SrefN,Sref,NDET,(double)0.)))
       {
        memcpy(Lambda,Feno->LambdaK,sizeof(double)*NDET);

        // Pointers initializations

        Spectre=Feno->SrefEtalon;

        Feno->Decomp=TabFeno[WrkFeno].Decomp=1;
        Feno->amfFlag=0;

        pSvd=&Feno->svd;

        Feno->indexReference=ITEM_NONE;

        // Initialize global variables

        if (((rc=ANALYSE_SvdInit(&Feno->svd))!=ERROR_ID_NO) ||
            ((rc=ANALYSE_CurFitMethod(Spectre,                     // etalon reference spectrum
                                      NULL,                        // error on raw spectrum
                                      Sref,                        // reference spectrum
                                     &Square,                      // returned stretch order 2
                                      NULL,                        // number of iterations in Curfit
                                     (double)1.,(double)1.))>=THREAD_EVENT_STOP))

         goto EndAlignReference;

        if (pAnalysisOptions->units==PRJCT_ANLYS_UNITS_NANOMETERS)
          for (j=0,lambda0=Lambda[(SvdPDeb+SvdPFin)/2];j<NDET;j++) // This is used only for spectra display
           {
            x0=Lambda[j]-lambda0;
            Lambda[j]=Lambda[j]-(pResults->Shift+(pResults->Stretch+pResults->Stretch2*x0)*x0);
           }

        if (refFlag==2)
         {
          TabFeno[WrkFeno].ShiftS=TabFeno[WrkFeno].ShiftN=pResults->Shift;
          TabFeno[WrkFeno].StretchS=TabFeno[WrkFeno].StretchN=pResults->Stretch;
          TabFeno[WrkFeno].Stretch2S=TabFeno[WrkFeno].Stretch2N=pResults->Stretch2;
         }
        else if (refFlag==3)
         {
          TabFeno[WrkFeno].ShiftS=pResults->Shift;
          TabFeno[WrkFeno].StretchS=pResults->Stretch;
          TabFeno[WrkFeno].Stretch2S=pResults->Stretch2;
         }
        else
         {
          TabFeno[WrkFeno].Shift=pResults->Shift;
          TabFeno[WrkFeno].Stretch=pResults->Stretch;
          TabFeno[WrkFeno].Stretch2=pResults->Stretch2;
         }

        // Display fit

        if (Feno->displayRefEtalon && pEngineContext->project.spectra.displaySpectraFlag)
         {
          memcpy(ANALYSE_secX,ANALYSE_zeros,sizeof(double)*NDET);

          for (i=SvdPDeb;i<SvdPFin;i++)
           ANALYSE_secX[i]=exp(log(Spectre[i])+ANALYSE_absolu[i]);

          sprintf(tabTitle,"%s results (%d/%d)",Feno->windowName,pEngineContext->indexRecord,pEngineContext->recordNumber);
          sprintf(string,"Alignment Ref1/Ref2");

          mediateAllocateAndSetPlotData(&spectrumData[0],"Measured",&Lambda[SvdPDeb],&Spectre[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
          mediateAllocateAndSetPlotData(&spectrumData[1],"Calculated",&Lambda[SvdPDeb],&ANALYSE_secX[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
          mediateResponsePlotData(indexPage,spectrumData,2,Spectrum,forceAutoScale,string,"Wavelength (nm)","Intensity", responseHandle);
          mediateResponseLabelPage(indexPage,pEngineContext->fileInfo.fileName, "Reference", responseHandle);
          mediateReleasePlotData(&spectrumData[1]);
          mediateReleasePlotData(&spectrumData[0]);

          if (pEngineContext->satelliteFlag)
           {
           	ANALYSE_plotRef=1;
           	indexLine=ANALYSE_indexLine;

            mediateResponseCellInfo(plotPageRef,indexLine++,indexColumn,responseHandle,"ALIGNMENT REF1/REF2 IN","%s",Feno->windowName);

            mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Shift Ref1/Ref2","%#10.3e +/- %#10.3e",pResults->Shift,pResults->SigmaShift);
            mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Stretch Ref1/Ref2","%#10.3e +/-%#10.3e",pResults->Stretch,pResults->SigmaStretch);
            mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Stretch2 Ref1/Ref2","%#10.3e +/- %#10.3e",pResults->Stretch2,pResults->SigmaStretch2);

            ANALYSE_indexLine=indexLine+1;
           }
         }

        TabFeno[WrkFeno].Decomp=1;
       }
     }
   }

  // Return

  EndAlignReference :

  if (RefTabFeno!=NULL)
   MEMORY_ReleaseBuffer("ANALYSE_AlignReference ","RefTabFeno",RefTabFeno);

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ANALYSE_AlignReference",rc);
  #endif

  return rc;
 }

RC AnalyseSaveResiduals(DoasCh *fileName,ENGINE_CONTEXT *pEngineContext)
 {
  RC rc;
  DoasCh *fileNamePtr,*ptr,resFile[MAX_ITEM_TEXT_LEN+1],ext[MAX_ITEM_TEXT_LEN+1];
  FILE *fp;
  INDEX i,j;

  rc=ERROR_ID_NO;

  FILES_RebuildFileName(resFile,fileName,1);

  if ((fileNamePtr=strrchr(resFile,PATH_SEP))==NULL)                 // extract output file name without path
   fileNamePtr=resFile;
  else
   fileNamePtr++;

  if (!strlen(fileNamePtr) && ((ptr=strrchr(pEngineContext->fileInfo.fileName,PATH_SEP))!=NULL))
   {
    strcpy(fileNamePtr,ptr+1);
    if ((ptr=strrchr(fileNamePtr,'.'))!=NULL)
     *ptr=0;

    sprintf(ext,"_%s.%s",Feno->windowName,FILES_types[FILE_TYPE_RES].fileExt);
    strcat(fileNamePtr,ext);
   }

  if ((fp=fopen(resFile,"a+t"))==NULL)
   rc=ERROR_SetLast("AnalyseSaveResiduals",ERROR_TYPE_FATAL,ERROR_ID_FILE_OPEN,resFile);
  else
   {
    if (!STD_FileLength(fp))
     {
      fprintf(fp,"0 0 0 ");

      for (j=0;j<Z;j++)
       for (i=Fenetre[j][0];i<=Fenetre[j][1];i++)
        fprintf(fp,"%.14le ",Feno->Lambda[i]);

      fprintf(fp,"\n");
     }

    fprintf(fp,"%-5d %.3lf %-8.4lf ",pEngineContext->indexRecord,pEngineContext->recordInfo.Zm,
           (double)ZEN_FNCaljda(&pEngineContext->recordInfo.Tm)+ZEN_FNCaldti(&pEngineContext->recordInfo.Tm)/24.);

    for (j=0;j<Z;j++)
     for (i=Fenetre[j][0];i<=Fenetre[j][1];i++)
      fprintf(fp,"%.14le ",ANALYSE_absolu[i]);
    fprintf(fp,"\n");
    fclose(fp);
   }

  return rc;
 }

// --------------------------------------------------------------------------------------------------------
// Function : Cross sections and spectrum alignment using spline fitting functions and new Yfit computation
// --------------------------------------------------------------------------------------------------------

RC ANALYSE_Function ( double *lambda,double *X, double *Y, INT ndet, double *Y0, double *SigmaY, double *Yfit, int Npts,
              double *fitParamsC, double *fitParamsF )
 {
  // Declarations

  double *XTrav,*YTrav,*newXsTrav,offset,deltaX,tau;
  CROSS_REFERENCE *TabCross,*pTabCross;
  MATRIX_OBJECT slit0;
  INT NewDimC,offsetOrder;
  INDEX i,j,k,l,m,indexSvdA,indexSvdP,polyOrder,polyFlag;
  double j0,lambda0;
  RC rc;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ANALYSE_Function",DEBUG_FCTTYPE_APPL);
  #endif

  // Initializations

  TabCross=Feno->TabCross;
  XTrav=YTrav=newXsTrav=NULL;
  memset(&slit0,0,sizeof(MATRIX_OBJECT));

  polyFlag=0;
  NewDimC=DimC;

  j0=(double)(SvdPDeb+SvdPFin)*0.5;
  lambda0=(fabs(j0-floor(j0))<(double)0.1)?
                              (double)ANALYSE_splineX[(INDEX)j0]:
                              (double)0.5*(ANALYSE_splineX[(INDEX)floor(j0)]+ANALYSE_splineX[(INDEX)floor(j0+1.)]);

  rc=ERROR_ID_NO;

  // Calculate the mean

  for (j=0,k=1,Feno->xmean=(double)0.;(j<Z);j++)
   for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
    Feno->xmean+=(double)X[l];

  Feno->xmean/=Npts;

  // Real time convolution for Kurucz

  if ((Feno->hidden==1) && Feno->xsToConvolute && pKuruczOptions->fwhmFit)

   rc=ANALYSE_XsConvolution(Feno,Lambda,&slit0,pKuruczOptions->fwhmType,
                           (TabCross[Feno->indexFwhmParam[0]].FitParam!=ITEM_NONE)?&fitParamsF[TabCross[Feno->indexFwhmParam[0]].FitParam]:&TabCross[Feno->indexFwhmParam[0]].InitParam,
                           (TabCross[Feno->indexFwhmParam[1]].FitParam!=ITEM_NONE)?&fitParamsF[TabCross[Feno->indexFwhmParam[1]].FitParam]:&TabCross[Feno->indexFwhmParam[1]].InitParam,
                           (TabCross[Feno->indexFwhmParam[2]].FitParam!=ITEM_NONE)?&fitParamsF[TabCross[Feno->indexFwhmParam[2]].FitParam]:&TabCross[Feno->indexFwhmParam[2]].InitParam,
                           (TabCross[Feno->indexFwhmParam[3]].FitParam!=ITEM_NONE)?&fitParamsF[TabCross[Feno->indexFwhmParam[3]].FitParam]:&TabCross[Feno->indexFwhmParam[3]].InitParam);

  // Don't take fixed concentrations into account for singular value decomposition

  for (i=0;i<Feno->NTabCross && (NewDimC==DimC);i++)
   if ((Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) && (TabCross[i].FitConc==0) &&
       (TabCross[i].DeltaConc==(double)0.) && TabCross[i].IndSvdA && (TabCross[i].IndSvdA<=NewDimC))

    NewDimC=TabCross[i].IndSvdA-1;

  // Buffers allocation

  if (((XTrav=MEMORY_AllocDVector("Function ","XTrav",0,Npts-1))==NULL) ||                  // raw spectrum
      ((YTrav=MEMORY_AllocDVector("Function ","YTrav",0,Npts-1))==NULL) ||                  // reference spectrum
      ((newXsTrav=MEMORY_AllocDVector("Function ","newXsTrav",0,NDET-1))==NULL))            // shifted and stretched cross section

   rc=ERROR_ID_ALLOC;

  else
   {
    memcpy(newXsTrav,ANALYSE_zeros,sizeof(double)*NDET);

    // ==============
    // CROSS SECTIONS
    // ==============

    // ----------------
    // Build svd matrix
    // ----------------

    if (Feno->Decomp)
     {
      for (i=0;i<Feno->NTabCross;i++)

       if ((indexSvdA=TabCross[i].IndSvdA)>0)
        {
         pTabCross=&TabCross[i];

         // Fill SVD matrix with predefined components

         if (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_PREDEFINED)
          {
           if (i==Feno->indexOffsetConst)
            for (j=0,k=1;(j<Z);j++)
             for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
              A[indexSvdA][k]=(double)1.;
           else if (i==Feno->indexOffsetOrder1)
            for (j=0,k=1;(j<Z);j++)
             for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
              A[indexSvdA][k]=(double)(ANALYSE_splineX[l]-lambda0);
           else if (i==Feno->indexOffsetOrder2)
            for (j=0,k=1;(j<Z);j++)
             for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
              A[indexSvdA][k]=(double)(ANALYSE_splineX[l]-lambda0)*(ANALYSE_splineX[l]-lambda0);
           else if ((i==Feno->indexCommonResidual) || (i==Feno->indexUsamp1) || (i==Feno->indexUsamp2))
            for (j=0,k=1;(j<Z);j++)
             for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
              A[indexSvdA][k]=(Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD)?
                               -pTabCross->vector[l]:pTabCross->vector[l];
           else if (i==Feno->indexRing1)
            for (j=0,k=1;(j<Z);j++)
             for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
              A[indexSvdA][k]=(Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD)?
                               -pTabCross->vector[l]:pTabCross->vector[l]/Y[l];      // !!! Assumed Raman spectrum
          }

         // Fill SVD matrix with polynomial components

         else if (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CONTINUOUS)
          {
          	if ((strlen(WorkSpace[pTabCross->Comp].symbolName)==2) && (WorkSpace[pTabCross->Comp].symbolName[0]=='x'))
          	 {
          	  polyOrder=WorkSpace[pTabCross->Comp].symbolName[1]-'0';
          	  polyFlag=1;
          	 }
          	else if ((strlen(WorkSpace[pTabCross->Comp].symbolName)==4) && (WorkSpace[pTabCross->Comp].symbolName[2]=='x'))
          	 {
          	  polyOrder=WorkSpace[pTabCross->Comp].symbolName[3]-'0';
          	  polyFlag=-1;
          	 }
          	else if ((strlen(WorkSpace[pTabCross->Comp].symbolName)==5) &&
          	         (WorkSpace[pTabCross->Comp].symbolName[0]=='o') &&
          	         (WorkSpace[pTabCross->Comp].symbolName[1]=='f') &&
          	         (WorkSpace[pTabCross->Comp].symbolName[2]=='f') &&
          	         (WorkSpace[pTabCross->Comp].symbolName[3]=='l'))
          	 {
          	 	polyOrder=WorkSpace[pTabCross->Comp].symbolName[4]-'0';
          	  polyFlag=0;
          	 }
          	else
          	 polyOrder=ITEM_NONE;

          	if (polyFlag || polyOrder)
          	 {
          	 	if (!polyOrder)
              for (j=0,k=1;(j<Z);j++)
               for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
                A[indexSvdA][k]=pTabCross->vector[l];
          	 	else if (polyFlag>=0)
              for (j=0,k=1;(j<Z);j++)
               for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
                A[indexSvdA][k]=pTabCross->vector[l]=A[indexSvdA-1][k]*ANALYSE_splineX[l]; // (ANALYSE_splineX[l]-lambda0);
             else if (polyFlag==-1)
              for (j=0,k=1;(j<Z);j++)
               for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
                A[indexSvdA][k]=pTabCross->vector[l]=A[indexSvdA-1][k]/ANALYSE_splineX[l];
          	 }
          	else if (Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD)               // linear offset, SVD method -> normalized w.r.t. the spectrum
          	 {
              for (j=0,k=1;(j<Z);j++)
               for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
                A[indexSvdA][k]=pTabCross->vector[l]=(fabs(X[l])>(double)1.e-6)?(double)-Feno->xmean/X[l]:(double)0.;
          	 }
          	else                                                                 // linear offset, Marquadt+SVD method -> normalized w.r.t. the reference
          	 {
              for (j=0,k=1;(j<Z);j++)
               for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
                A[indexSvdA][k]=pTabCross->vector[l]=(fabs(Y[l])>(double)1.e-6)?(double)Feno->xmean/Y[l] :(double)0.;
          	 }
          }

         // Fill SVD matrix with cross sections

         else if (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS)
          {
           // Use substitution vectors for cross sections because of AMF correction

           memcpy(ANALYSE_xsTrav,pTabCross->vector,sizeof(double)*NDET);
           memcpy(ANALYSE_xsTrav2+1,pTabCross->Deriv2+1,sizeof(double)*NDET);

           // --------------
           // AMF correction
           // --------------

           if ((Feno->amfFlag && ((rc=OUTPUT_GetWveAmf(&Feno->TabCrossResults[i],ZM,Lambda,ANALYSE_xsTrav,ANALYSE_xsTrav2))!=0)) ||

           // ---------------------------------------
           // Wavelength alignment for cross sections
           // ---------------------------------------

              (!strcasecmp(WorkSpace[pTabCross->Comp].symbolName,"O3TD") &&
              ((rc=TemperatureCorrection(ANALYSE_xsTrav,
                                         O3TD.matrix[2],
                                         O3TD.matrix[3],
                                         O3TD.matrix[4],
                                         newXsTrav,
                                        (pTabCross->FitShift!=ITEM_NONE)?(double)fitParamsF[pTabCross->FitShift]:(double)pTabCross->InitShift))!=ERROR_ID_NO)) ||

              (strcasecmp(WorkSpace[pTabCross->Comp].symbolName,"O3TD") &&

              ((rc=ShiftVector(ANALYSE_xsTrav /* (0:NDET-1) */,ANALYSE_xsTrav2+1 /* (1:NDET) */,newXsTrav /* (0:NDET-1) */,
                              (pTabCross->FitShift!=ITEM_NONE)?(double)fitParamsF[pTabCross->FitShift]:(double)pTabCross->InitShift,
                              (pTabCross->FitStretch!=ITEM_NONE)?(double)fitParamsF[pTabCross->FitStretch]:(double)pTabCross->InitStretch,
                              (pTabCross->FitStretch2!=ITEM_NONE)?(double)fitParamsF[pTabCross->FitStretch2]:(double)pTabCross->InitStretch2,
                               Feno->Shift,Feno->Stretch,Feno->Stretch2,
                               NULL,0,0))!=ERROR_ID_NO)))

            goto EndFunction;

           else
            {
             for (j=0,k=1;(j<Z);j++)
              for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
               A[indexSvdA][k]=newXsTrav[l];
            }
          }
        }

      // ---------------------------------
      // Orthogonalization of columns of A
      // ---------------------------------

      for (i=0;i<Feno->NTabCross;i++)

       if ((indexSvdA=TabCross[i].IndSvdA)>0)
        {
         pTabCross=&TabCross[i];

         if ((WorkSpace[pTabCross->Comp].type!=WRK_SYMBOL_PREDEFINED) ||
            (// (Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVDMARQUARDT) &&
             (i==Feno->indexRing1) ||
             (i==Feno->indexUsamp1) ||
             (i==Feno->indexUsamp2)))

          if ((rc=VECTOR_NormalizeVector(A[indexSvdA],Npts,&pTabCross->Fact,WorkSpace[pTabCross->Comp].symbolName))!=ERROR_ID_NO)  // normalize vectors of A before orthogonalization
           goto EndFunction;
        }

      Orthogonalization();

      // ----------------------------------------------------
      // Cross sections correction with non linear parameters
      // ----------------------------------------------------

      for (i=0;i<Feno->NTabCross;i++)
       {
        pTabCross=&TabCross[i];

        if ((indexSvdA=pTabCross->IndSvdA)>0)
         {
          // ----------------------------------------------------
          // Cross sections correction with non linear parameters
          // ----------------------------------------------------

          if ((WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) &&
             ((pTabCross->FitScale!=ITEM_NONE) || (pTabCross->InitScale!=(double)0.) ||
              (pTabCross->FitScale2!=ITEM_NONE) || (pTabCross->InitScale2!=(double)0.)))
           {
            for (j=0,k=1;(j<Z);j++)
             for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
              {
               offset=(double)1.;
               deltaX=(double)(ANALYSE_splineX[l]-lambda0);

               offset+=((pTabCross->FitScale!=ITEM_NONE)?(double)fitParamsF[pTabCross->FitScale]:(double)pTabCross->InitScale)*deltaX;
               offset+=((pTabCross->FitScale2!=ITEM_NONE)?(double)fitParamsF[pTabCross->FitScale2]:(double)pTabCross->InitScale2)*deltaX*deltaX;

               A[indexSvdA][k]*=offset*pTabCross->Fact;
              }

            if ((rc=VECTOR_NormalizeVector(A[indexSvdA],Npts,&pTabCross->Fact,"Function 2 "))!=ERROR_ID_NO)
             goto EndFunction;
           }

          // -------------------------------------
          // Make a copy of matrix A into matrix U
          // -------------------------------------

          memcpy((char *)(U[indexSvdA]+1),(char *)(A[indexSvdA]+1),sizeof(double)*Npts);

          for (l=1;l<=DimL;l++)
           U[indexSvdA][l]*=pTabCross->Fact;   // Multiply column of U by normalization factor
         }
       }

      if ((Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) && (SigmaY!=NULL))
       for (indexSvdA=0;indexSvdA<=DimC;indexSvdA++)
        for (k=1;k<=DimL;k++)
         A[indexSvdA][k]/=SigmaY[k-1];

      // -----------------
      // SVD Decomposition
      // -----------------

      #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_SVD_) && __DEBUG_DOAS_SVD_
      if (((analyseDebugMask&DEBUG_FCTTYPE_MATH)!=0) && analyseDebugVar)        // SVD is classified as a math function
       {
       	DEBUG_Print("+++ SVD decomposition (%s window)\n",Feno->windowName);

        for (j=0;j<Feno->NTabCross;j++)
         if (TabCross[j].IndSvdA)
          DEBUG_Print("+++ %-2d %s\n",TabCross[j].IndSvdA,WorkSpace[TabCross[j].Comp].symbolName);

        DEBUG_PrintVar("Before SVD_Dcmp",U,1,DimL,0,DimC,NULL);
        DEBUG_PrintVar("Before SVD_Dcmb",A,1,DimL,0,DimC,NULL);
       }
      #endif

      if ((Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) &&
         ((rc=SVD_Dcmp(A,Npts,NewDimC /* don't take fixed concentrations into account */ ,W,V,SigmaSqr,covar))!=ERROR_ID_NO))
       goto EndFunction;

      if (FAST && (SigmaY==NULL) && (!Feno->offlFlag || (Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVDMARQUARDT)))
       Feno->Decomp=0;
     }

    // ========
    // SPECTRUM
    // ========

    // ---------------------------------
    // Wavelength alignment for spectrum
    // ---------------------------------

    if (((rc=ShiftVector(X /* (0:NDET-1) */,SplineSpec /* (0:NDET-1) */,newXsTrav /* (0:NDET-1) */,
                        (Feno->indexSpectrum!=ITEM_NONE)?((TabCross[Feno->indexSpectrum].FitShift!=ITEM_NONE)?(double)fitParamsF[TabCross[Feno->indexSpectrum].FitShift]:(double)TabCross[Feno->indexSpectrum].InitShift):(double)0.,
                        (Feno->indexSpectrum!=ITEM_NONE)?((TabCross[Feno->indexSpectrum].FitStretch!=ITEM_NONE)?(double)fitParamsF[TabCross[Feno->indexSpectrum].FitStretch]:(double)TabCross[Feno->indexSpectrum].InitStretch):(double)0.,
                        (Feno->indexSpectrum!=ITEM_NONE)?((TabCross[Feno->indexSpectrum].FitStretch2!=ITEM_NONE)?(double)fitParamsF[TabCross[Feno->indexSpectrum].FitStretch2]:(double)TabCross[Feno->indexSpectrum].InitStretch2):(double)0.,
                        (double)0.,(double)0.,(double)0.,
                         fitParamsF,-1,0))!=ERROR_ID_NO) ||

         (Feno->useUsamp && (pUsamp->method==PRJCT_USAMP_AUTOMATIC) && ((rc=ANALYSE_UsampBuild(2,ITEM_NONE))!=ERROR_ID_NO)))

       goto EndFunction;

    #if defined(__DEBUG_) && __DEBUG_  && defined(__DEBUG_DOAS_SHIFT_) && __DEBUG_DOAS_SHIFT_
    if (((analyseDebugMask&DEBUG_FCTTYPE_MATH)!=0) && analyseDebugVar &&
         (Feno->indexSpectrum!=ITEM_NONE) &&
        ((TabCross[Feno->indexSpectrum].FitShift!=ITEM_NONE) || (TabCross[Feno->indexSpectrum].InitShift!=(double)0.)))
     DEBUG_PrintVar("Interpolation of the spectrum",ANALYSE_splineX,LimMin,LimMax,X,LimMin,LimMax,SplineSpec,LimMin,LimMax,newXsTrav,LimMin,LimMax,NULL);
    #endif

    // ------------------------------
    // Low pass filtering on spectrum
    // ------------------------------

    // Filter real time only when fitting difference of resolution between spectrum and reference

    if ((Feno->analysisType==ANALYSIS_TYPE_FWHM_NLFIT) && (ANALYSE_plFilter->filterFunction!=NULL) &&
       ((rc=FILTER_Vector(ANALYSE_plFilter,&newXsTrav[LimMin],&newXsTrav[LimMin],LimN,PRJCT_FILTER_OUTPUT_LOW))!=0))
     {
     	rc=ERROR_SetLast("EndFunction",ERROR_TYPE_WARNING,ERROR_ID_ANALYSIS,analyseIndexRecord,"Filter");
      goto EndFunction;
     }

    // -------------------------------
    // Spectrum correction with offset
    // -------------------------------

    if (Feno->analysisMethod!=PRJCT_ANLYS_METHOD_SVDMARQUARDT)
     {
      offsetOrder=-1;

      if ((Feno->indexOffsetConst!=ITEM_NONE) && ((TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE) || (TabCross[Feno->indexOffsetConst].InitParam!=(double)0.)))
       offsetOrder=0;
      if ((Feno->indexOffsetOrder1!=ITEM_NONE) && ((TabCross[Feno->indexOffsetOrder1].FitParam!=ITEM_NONE) || (TabCross[Feno->indexOffsetOrder1].InitParam!=(double)0.)))
       offsetOrder=1;
      if ((Feno->indexOffsetOrder2!=ITEM_NONE) && ((TabCross[Feno->indexOffsetOrder2].FitParam!=ITEM_NONE) || (TabCross[Feno->indexOffsetOrder2].InitParam!=(double)0.)))
       offsetOrder=2;

      if (offsetOrder>=0)
       {
        for (i=LimMin;i<=LimMax;i++)
         {
          deltaX=(double)(ANALYSE_splineX[i]-lambda0);

          offset=(TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE)?(double)fitParamsF[TabCross[Feno->indexOffsetConst].FitParam]:(double)TabCross[Feno->indexOffsetConst].InitParam;

          if (offsetOrder>=1)
           offset+=((TabCross[Feno->indexOffsetOrder1].FitParam!=ITEM_NONE)?(double)fitParamsF[TabCross[Feno->indexOffsetOrder1].FitParam]/TabCross[Feno->indexOffsetOrder1].Fact:(double)TabCross[Feno->indexOffsetOrder1].InitParam)*deltaX;
          if (offsetOrder>=2)
           offset+=((TabCross[Feno->indexOffsetOrder2].FitParam!=ITEM_NONE)?(double)fitParamsF[TabCross[Feno->indexOffsetOrder2].FitParam]/TabCross[Feno->indexOffsetOrder2].Fact:(double)TabCross[Feno->indexOffsetOrder2].InitParam)*deltaX*deltaX;

          newXsTrav[i]-=(double)offset*Feno->xmean;
         }
       }
     }

    if ((Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) && (Feno->indexRing1!=ITEM_NONE))
     {
      for (i=LimMin;i<=LimMax;i++)
       newXsTrav[i]-=((double)(TabCross[Feno->indexRing1].FitParam!=ITEM_NONE)?
                      (double)fitParamsF[TabCross[Feno->indexRing1].FitParam]:
                      (double)TabCross[Feno->indexRing1].InitParam)*TabCross[Feno->indexRing1].vector[i]/TabCross[Feno->indexRing1].Fact;
     }

    // -------------------------------
    // High-pass filtering on spectrum
    // -------------------------------

    if ((Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) && !hFilterSpecLog &&  // logarithms are not calculated and filtered before entering this function
      (((rc=VECTOR_Log(&newXsTrav[LimMin],&newXsTrav[LimMin],LimN,"ANLYSE_Function (Spec) "))!=0) ||
       ((ANALYSE_phFilter->filterFunction!=NULL) &&
       ((!Feno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((Feno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
       ((rc=FILTER_Vector(ANALYSE_phFilter,&newXsTrav[LimMin],&newXsTrav[LimMin],LimN,PRJCT_FILTER_OUTPUT_HIGH_SUB))!=0))))

     goto EndFunction;

    // ----------------------------
    // Transfer to working variable
    // ----------------------------

    for (j=0,k=0;(j<Z);j++)
     for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
      XTrav[k]=newXsTrav[l];

    // =========
    // REFERENCE
    // =========

    // ----------------------------------
    // Wavelength alignment for reference
    // ----------------------------------

    if ((rc=ShiftVector(Y,SplineRef,newXsTrav,
                       (Feno->indexReference!=ITEM_NONE)?((TabCross[Feno->indexReference].FitShift!=ITEM_NONE)?(double)fitParamsF[TabCross[Feno->indexReference].FitShift]:(double)TabCross[Feno->indexReference].InitShift):(double)0.,
                       (Feno->indexReference!=ITEM_NONE)?((TabCross[Feno->indexReference].FitStretch!=ITEM_NONE)?(double)fitParamsF[TabCross[Feno->indexReference].FitStretch]:(double)TabCross[Feno->indexReference].InitStretch):(double)0.,
                       (Feno->indexReference!=ITEM_NONE)?((TabCross[Feno->indexReference].FitStretch2!=ITEM_NONE)?(double)fitParamsF[TabCross[Feno->indexReference].FitStretch2]:(double)TabCross[Feno->indexReference].InitStretch2):(double)0.,
                        (double)0.,(double)0.,(double)0.,
                        fitParamsF,1,(Feno->analysisType==ANALYSIS_TYPE_FWHM_KURUCZ)?1:0))!=ERROR_ID_NO)

       goto EndFunction;

    #if defined(__DEBUG_) && __DEBUG_  && defined(__DEBUG_DOAS_SHIFT_) && __DEBUG_DOAS_SHIFT_
    if (((analyseDebugMask&DEBUG_FCTTYPE_MATH)!=0) && analyseDebugVar &&
         (Feno->indexReference!=ITEM_NONE) &&
        ((TabCross[Feno->indexReference].FitShift!=ITEM_NONE) || (TabCross[Feno->indexReference].InitShift!=(double)0.)))
     DEBUG_PrintVar("Interpolation of the reference",ANALYSE_splineX,LimMin,LimMax,Y,LimMin,LimMax,SplineRef,LimMin,LimMax,newXsTrav,LimMin,LimMax,NULL);
    #endif

    // -------------------------------
    // Low pass filtering on reference
    // -------------------------------

    // Filter real time only when fitting difference of resolution between spectrum and reference

    if ((Feno->analysisType==ANALYSIS_TYPE_FWHM_NLFIT) && (ANALYSE_plFilter->filterFunction!=NULL) &&
       ((rc=FILTER_Vector(ANALYSE_plFilter,&newXsTrav[LimMin],&newXsTrav[LimMin],LimN,PRJCT_FILTER_OUTPUT_LOW))!=0))
     {
     	rc=ERROR_SetLast("EndFunction",ERROR_TYPE_WARNING,ERROR_ID_ANALYSIS,analyseIndexRecord,"Filter");
      goto EndFunction;
     }

    // ----------------------------------------------
    // Reference correction with non linear parameters
    // ----------------------------------------------

    if ((Feno->analysisMethod!=PRJCT_ANLYS_METHOD_SVDMARQUARDT) &&
        (Feno->indexSol!=ITEM_NONE) &&
       ((TabCross[Feno->indexSol].FitParam!=ITEM_NONE) ||
       ((TabCross[Feno->indexSol].InitParam!=(double)0.)&&(TabCross[Feno->indexSol].InitParam!=(double)1.))))

     for (i=LimMin;i<=LimMax;i++)
      newXsTrav[i]=pow(newXsTrav[i],(TabCross[Feno->indexSol].FitParam!=ITEM_NONE)?(double)fitParamsF[TabCross[Feno->indexSol].FitParam]:(double)TabCross[Feno->indexSol].InitParam);

    // --------------------------------
    // High pass filtering on reference
    // --------------------------------

    // logarithms are not calculated and filtered before entering this function

    if ((Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) && !hFilterRefLog &&  // logarithms are not calculated and filtered before entering this function
      (((rc=VECTOR_Log(&newXsTrav[LimMin],&newXsTrav[LimMin],LimN,"ANLYSE_Function (Ref) "))!=0) ||
       ((ANALYSE_phFilter->filterFunction!=NULL) &&
       ((!Feno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((Feno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
       ((rc=FILTER_Vector(ANALYSE_phFilter,&newXsTrav[LimMin],&newXsTrav[LimMin],LimN,PRJCT_FILTER_OUTPUT_HIGH_SUB))!=0))))

     goto EndFunction;

    // ----------------------------
    // Transfer to working variable
    // ----------------------------

    for (j=0,k=0;(j<Z);j++)
     for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
      YTrav[k]=newXsTrav[l];



    //
    // OPTICAL THICKNESS FITTING (SVD)
    //

    if (Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD)
     {
      // ---------------------
      // SVD back substitution
      // ---------------------

      // N.B. : YTrav -> reference spectrum, shifted and stretched
      //        XTrav -> raw spectrum, shifted and stretched

      for (j=0,k=1;j<Z;j++)
       for (i=Fenetre[j][0];i<=Fenetre[j][1];i++,k++)
        {
         b[k]=YTrav[k-1]-XTrav[k-1];

         for (l=NewDimC+1;l<=DimC;l++)
          {
           for (m=0;m<Feno->NTabCross;m++)
            if (TabCross[m].IndSvdA==l)
             break;

           b[k]-=U[l][k]*fitParamsC[l];
          }

         if (SigmaY!=NULL)
          b[k]/=SigmaY[k-1];
        }

      #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_SVD_) && __DEBUG_DOAS_SVD_
      if (((analyseDebugMask&DEBUG_FCTTYPE_MATH)!=0) && analyseDebugVar)        // SVD is classified as a math function
       DEBUG_PrintVar("Before SVD_Bksb",XTrav,0,DimL-1,YTrav,0,DimL-1,b,1,DimL,NULL);
       DEBUG_PrintVar("Before SVD_Bksb",U,1,DimL,0,DimC,NULL);
       DEBUG_PrintVar("Before SVD_Bksb",fitParamsC,0,DimC,NULL);
      #endif

      if ((rc=SVD_Bksb(A,W,V,Npts,NewDimC /* don't take fixed concentrations into account */, b, fitParamsC ))!=ERROR_ID_NO)  // Solve system A*fitParamsF=b
       goto EndFunction;

      #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_SVD_) && __DEBUG_DOAS_SVD_
      if (((analyseDebugMask&DEBUG_FCTTYPE_MATH)!=0) && analyseDebugVar)        // SVD is classified as a math function
       DEBUG_PrintVar("After SVD_Bksb",fitParamsC,0,DimC,NULL);
      #endif

      // ------------------------------------------------
      // Yfit computation with the solution of the system
      // ------------------------------------------------

      for (j=0,k=1;j<Z;j++)
       for (i=Fenetre[j][0];i<=Fenetre[j][1];i++,k++)
        {
         for (l=0;l<Feno->NTabCross;l++)
          {
      //   if (!Feno->hidden && (TabCross[l].IndSvdA>0) && (TabCross[l].IndSvdA<=NewDimC))
      //   {
      //  		FILE *fp;
      //  		fp=fopen("toto.dat","a+t");
      //  		fprintf(fp,"%d %d %g\n",l,TabCross[l].IndSvdA,TabCross[l].Fact);
      //  		fclose(fp);
      //  	}

           if ((TabCross[l].IndSvdA>0) && (l!=Feno->indexRing1))
            XTrav[k-1]+=(TabCross[l].IndSvdA<=NewDimC)?fitParamsC[TabCross[l].IndSvdA]*U[TabCross[l].IndSvdA][k]/TabCross[l].Fact:

            fitParamsC[TabCross[l].IndSvdA]*U[TabCross[l].IndSvdA][k];
          }

         Yfit[k-1]=YTrav[k-1]-XTrav[k-1]; // NB : logarithm test on YTrav has been made in the previous loop
        }
     }

    //
    // INTENSITY FITTING (Marquardt-Levenberg + SVD)
    //

    else if (Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVDMARQUARDT)
     {
      // ---------------
      // Fill SVD matrix
      // ---------------

      for (j=0,k=1;j<Z;j++)
       for (i=Fenetre[j][0];i<=Fenetre[j][1];i++,k++)
        {
         for (tau=(double)0.,l=0;l<Feno->NTabCross;l++)
          {
           pTabCross=&TabCross[l];

           if (((indexSvdA=pTabCross->IndSvdA)>0) && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS))
            tau+=(pTabCross->FitConc>ITEM_NONE)?(double)fitParamsF[pTabCross->FitConc]*A[pTabCross->IndSvdA][k]:  // Conc non linear fitting
                  fitParamsC[pTabCross->IndSvdA]*U[pTabCross->IndSvdA][k];
          }

         if (-tau>(double)700.)
          {
          	rc=ERROR_SetLast("Function",ERROR_TYPE_WARNING,ERROR_ID_OVERFLOW);
           goto EndFunction;
          }
         else if (YTrav[k-1]==(double)0.)
          {
          	rc=ERROR_SetLast("Function",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0);
           goto EndFunction;
          }

         ANALYSE_t[i]=exp(-tau); // exp(-tau)
         ANALYSE_tc[i]=XTrav[k-1]/YTrav[k-1];  // I/I0
        }

      // ---------------------------------------
      // SVD decomposition and back substitution
      // ---------------------------------------

      for (l=0;l<Feno->NTabCross;l++)
       {
        pTabCross=&TabCross[l];

        if (((indexSvdA=pTabCross->IndSvdA)>0) && ((indexSvdP=pTabCross->IndSvdP)>0))
         for (j=0,k=1;j<Z;j++)
          for (i=Fenetre[j][0];i<=Fenetre[j][1];i++,k++)
           {
            // Polynomial

            if ((WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CONTINUOUS) && (WorkSpace[pTabCross->Comp].symbolName[0]!='o')) // Polynomial
             P[indexSvdP][k]=ANALYSE_t[i]*A[indexSvdA][k];

            // Linear offset normalized w.r.t. the reference spectrum and other parameters (Ring ...)

            else
             P[indexSvdP][k]=A[indexSvdA][k];
           }
       }

/*      if ((rc=Regres(P,XTrav,NULL,Npts,DimP,pAnalysisOptions->fitWeighting,Yfit,
                    (TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE)?TabCross[Feno->indexOffsetConst].IndSvdP-1:ITEM_NONE,
                     b+1,SigmaSqr+1,&tau))!=ERROR_ID_NO)

       goto EndFunction; */

      if (((rc=SVD_Dcmp(P,Npts,DimP,W,V,SigmaSqr,covar))!=ERROR_ID_NO) ||
          ((rc=SVD_Bksb(P,W,V,Npts,DimP,&ANALYSE_tc[Fenetre[0][0]]-1,b))!=ERROR_ID_NO))      // Solve system P*b=b

       goto EndFunction;

      // ------------------------------------------------
      // Yfit computation with the solution of the system
      // ------------------------------------------------

      for (i=0;i<Feno->NTabCross;i++)
       if (((indexSvdA=TabCross[i].IndSvdA)>0) && ((indexSvdP=TabCross[i].IndSvdP)>0))
        fitParamsC[indexSvdA]=b[indexSvdP];

      for (j=0,k=1;j<Z;j++)
       for (i=Fenetre[j][0];i<=Fenetre[j][1];i++,k++)
        {
         for (l=0,tau=offset=(double)0.;l<Feno->NTabCross;l++)
          if (((indexSvdA=TabCross[l].IndSvdA)>0) && ((indexSvdP=TabCross[l].IndSvdP)>0))
           {
            if ((WorkSpace[TabCross[l].Comp].type==WRK_SYMBOL_CONTINUOUS) && (WorkSpace[TabCross[l].Comp].symbolName[0]!='o'))      // Polynomial
             tau+=b[indexSvdP]*A[indexSvdA][k];
            else                                                                // Offset and other parameters (ring...)
             offset+=b[indexSvdP]*A[indexSvdA][k];
           }

          ANALYSE_tc[i]-=offset;            // I/I0 - offset/I0
          ANALYSE_t[i]*=tau;                // tau*exp(-optical depth)

          Yfit[k-1]=ANALYSE_t[i]-ANALYSE_tc[i];
        }

      #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_SVD_) && __DEBUG_DOAS_SVD_
      if (((analyseDebugMask&DEBUG_FCTTYPE_MATH)!=0) && analyseDebugVar)        // SVD is classified as a math function
       DEBUG_PrintVar("After the fit",ANALYSE_t,0,NDET-1,ANALYSE_tc,0,NDET-1,NULL);
      #endif
     }
   }

  // Release allocated buffers

  EndFunction :

  if (XTrav!=NULL)
   MEMORY_ReleaseDVector("Function ","XTrav",XTrav,0);
  if (YTrav!=NULL)
   MEMORY_ReleaseDVector("Function ","YTrav",YTrav,0);
  if (newXsTrav!=NULL)
   MEMORY_ReleaseDVector("Function ","newXsTrav",newXsTrav,0);

  // Return

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ANALYSE_Function",rc);
  #endif

  return rc;
 }

/*                                                                           */
/*  ANALYSE_CurFitMethod ( Spectre, Spreflog, Absolu, Square ) :             */
/*  ==========================================================               */
/*                                                                           */
/*         Make a least-square fit to a non linear function whose non-       */
/*         linear parameters are shifts and stretches and linear para-       */
/*         meters are the concentrations implied in Beer-Lambert's law       */
/*         and are computed by singular value decomposition of cross         */
/*         sections matrix.                                                  */
/*                                                                           */


#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC ANALYSE_CurFitMethod(double *Spectre,          // raw spectrum
                        double *SigmaSpec,        // error on raw spectrum
                        double *Sref,             // reference spectrum
                        double *Chisqr,           // chi square
                        INT    *pNiter,           // number of iterations
                        double  speNormFact,
                        double  refNormFact)
 {
  // Declarations

  CROSS_REFERENCE *TabCross,*pTabCross;
  CROSS_RESULTS *pResults;
  double OldChisqr,                                      // chi square a step before
        *Y0,                                              // vector to fit; deduced from measurements
        *SigmaY,
        *Yfit,                                           // vector fitted
        *Deltap,                                         // increments for parameters
        *fitParamsF,
        *fitParamsC,
        *Sigmaa,                                         // errors on parameters
        *SpecTrav,*RefTrav,                              // substitution vectors
         Lamda,                                          // scaling factor used by curfit (not related to wavelength scale)
         scalingFactor;

  int i,j,k,l;                                             // indexes for loops and arrays
  INDEX indexFeno,indexFeno2;
  int useErrors;
  int niter;
  RC rc;                                                 // return code

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ANALYSE_CurFitMethod",DEBUG_FCTTYPE_APPL);
  #endif

  // Initializations

  TabCross=Feno->TabCross;                               // symbol cross reference
  useErrors=((Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) && (pAnalysisOptions->fitWeighting!=PRJCT_ANLYS_FIT_WEIGHTING_NONE) && (SigmaSpec!=NULL) && (Feno->SrefSigma!=NULL))?1:0;

  fitParamsC=fitParamsF=Deltap=Sigmaa=Y0=SpecTrav=RefTrav=SigmaY=NULL;          // pointers
  hFilterSpecLog=0;
  hFilterRefLog=0;
  rc=ERROR_ID_NO;                                      // return code

  /*  ==================  */
  /*  Buffers allocation  */
  /*  ==================  */

  if (((Yfit=(double *)MEMORY_AllocDVector("ANALYSE_CurFitMethod ","YFit",0,DimL-1))==NULL) ||
      ((fitParamsC=(double *)MEMORY_AllocDVector("ANALYSE_CurFitMethod ","fitParamsC",0,DimC))==NULL) ||
      ((NF!=0) && (((fitParamsF=(double *)MEMORY_AllocDVector("ANALYSE_CurFitMethod ","fitParamsF",0,NF-1))==NULL) ||
                   ((Deltap=(double *)MEMORY_AllocDVector("ANALYSE_CurFitMethod ","Deltap",0,NF-1))==NULL) ||
                   ((Sigmaa=(double *)MEMORY_AllocDVector("ANALYSE_CurFitMethod ","Sigmaa",0,NF-1))==NULL))) ||
      ((Y0=(double *)MEMORY_AllocDVector("ANALYSE_CurFitMethod ","Y0",0,DimL-1))==NULL) ||
      (useErrors && ((SigmaY=(double *)MEMORY_AllocDVector("ANALYSE_CurFitMethod ","SigmaY",0,DimL-1))==NULL)) ||
      ((SpecTrav=(double *)MEMORY_AllocDVector("ANALYSE_CurFitMethod ","SpecTrav",0,NDET-1))==NULL) ||
      ((RefTrav=(double *)MEMORY_AllocDVector("ANALYSE_CurFitMethod ","RefTrav",0,NDET-1))==NULL))

   rc=ERROR_ID_ALLOC;       // NB : call filter one time for determining the best Dim (security for filtering and interpolation)

  else
   {
    // Initializations

    memcpy(SpecTrav,Spectre,sizeof(double)*NDET);
    if (SigmaY!=NULL)
     memcpy(SigmaY,ANALYSE_zeros,sizeof(double)*DimL);
    memcpy(RefTrav,Sref,sizeof(double)*NDET);

    memcpy(Yfit,ANALYSE_zeros,sizeof(double)*DimL);
    memcpy(Y0,ANALYSE_zeros,sizeof(double)*DimL);

    if (NF)
     memcpy(Sigmaa,ANALYSE_ones,sizeof(double)*(NF-1));

    // ----------------------------------------------
    // Fwhm adjustment between spectrum and reference
    // ----------------------------------------------

    if (!Feno->hidden && (Feno->useKurucz!=ANLYS_KURUCZ_SPEC))
     {
      // Resolution adjustment using fwhm(lambda) found by Kurucz procedure for spectrum and reference

      if (pKuruczOptions->fwhmFit && (Feno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC))
       rc=AnalyseFwhmCorrectionK(Spectre,Sref,SpecTrav,RefTrav);

      // Resolution adjustment regarding spectrum and reference temperatures

      else if (((Feno->useKurucz==ANLYS_KURUCZ_NONE) || !pKuruczOptions->fwhmFit) &&
               ((Feno->analysisType==ANALYSIS_TYPE_FWHM_CORRECTION) ||
             (((pSlitOptions->slitFunction.slitType==SLIT_TYPE_GAUSS_T_FILE) ||
               (pSlitOptions->slitFunction.slitType==SLIT_TYPE_ERF_T_FILE)) &&
                Feno->xsToConvolute)))
       rc=AnalyseFwhmCorrectionT(Spectre,Sref,SpecTrav,RefTrav);

      if (rc)
       goto EndCurFitMethod;
     }

    // -----------------------------
    // Filter spectrum and reference
    // -----------------------------

    // Low pass filtering

    if ((ANALYSE_plFilter->filterFunction!=NULL) &&                   // low pass filtering is requested
        (Feno->analysisType!=ANALYSIS_TYPE_FWHM_NLFIT) &&     // doesn't fit the resolution (FWHM) between the reference and the spectrum as a non linear parameter
        !Feno->hidden &&                                      // low pass filtering is disabled for calibration in order not to degrade the spectrum to calibrate

      (((rc=FILTER_Vector(ANALYSE_plFilter,&SpecTrav[LimMin],&SpecTrav[LimMin],LimN,PRJCT_FILTER_OUTPUT_LOW))!=0) ||
       ((rc=FILTER_Vector(ANALYSE_plFilter,&RefTrav[LimMin],&RefTrav[LimMin],LimN,PRJCT_FILTER_OUTPUT_LOW))!=0)))
     {
     	rc=ERROR_SetLast("ANALYSE_CurFitMethod",ERROR_TYPE_WARNING,ERROR_ID_ANALYSIS,analyseIndexRecord,"Filter");
      goto EndCurFitMethod;
     }

    // High pass filtering (spectrum)

    if ((ANALYSE_phFilter->filterFunction!=NULL) &&           // high pass filtering is requested
       ((!Feno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((Feno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
        (Feno->analysisType!=ANALYSIS_TYPE_FWHM_NLFIT) &&     // doesn't fit the resolution (FWHM) between the reference and the spectrum as a non linear parameter
        (Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) &&     // only implemented in optical density fitting

                                                              // if offset is applied on spectrum, filter spectrum and reference at each iteration in Function
                                                              // otherwise, filter logarithms

       ((Feno->indexOffsetConst==ITEM_NONE) || (TabCross[Feno->indexOffsetConst].FitParam==ITEM_NONE)) &&
       ((Feno->indexOffsetOrder1==ITEM_NONE) || (TabCross[Feno->indexOffsetOrder1].FitParam==ITEM_NONE)) &&
       ((Feno->indexOffsetOrder2==ITEM_NONE) || (TabCross[Feno->indexOffsetOrder2].FitParam==ITEM_NONE)))
     {
      hFilterSpecLog=1;

      if (((rc=VECTOR_Log(&SpecTrav[LimMin],&SpecTrav[LimMin],LimN,"ANALYSE_CurFitMethod (Ref) "))!=0) ||           // !!!
          ((rc=FILTER_Vector(ANALYSE_phFilter,&SpecTrav[LimMin],&SpecTrav[LimMin],LimN,PRJCT_FILTER_OUTPUT_HIGH_SUB))!=0))        // !!!

       goto EndCurFitMethod;
     }

    // High pass filtering (Reference)

    if ((ANALYSE_phFilter->filterFunction!=NULL) &&                   // high pass filtering is requested
       ((!Feno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((Feno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&

        (Feno->analysisType!=ANALYSIS_TYPE_FWHM_NLFIT) &&     // doesn't fit the resolution (FWHM) between the reference and the spectrum as a non linear parameter
        (Feno->analysisType!=ANALYSIS_TYPE_FWHM_KURUCZ) &&
        (Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) &&       // only implemented in optical density fitting
       ((Feno->indexSol==ITEM_NONE) || (TabCross[Feno->indexSol].FitParam==ITEM_NONE)))
     {
      hFilterRefLog=1;

      if (((rc=VECTOR_Log(&RefTrav[LimMin],&RefTrav[LimMin],LimN,"ANALYSE_CurFitMethod (Ref) "))!=0) ||
          ((rc=FILTER_Vector(ANALYSE_phFilter,&RefTrav[LimMin],&RefTrav[LimMin],LimN,PRJCT_FILTER_OUTPUT_HIGH_SUB))!=0))

       goto EndCurFitMethod;
     }

    // High pass filtering (high resolution kurucz spectrum)
/*
    if ((KURUCZ_buffers.lambdaF!=NULL) && (KURUCZ_buffers.solarF!=NULL) && (KURUCZ_buffers.solarF2!=NULL) &&
        (ANALYSE_phFilter->filterFunction!=NULL) && (Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) &&
        (Feno->analysisType==ANALYSIS_TYPE_FWHM_KURUCZ) &&
       ((Feno->indexSol==ITEM_NONE) || (TabCross[Feno->indexSol].FitParam==ITEM_NONE)))
     {
      if (!KURUCZ_buffers.solarFGap || (KURUCZ_buffers.lambdaF[0]==(double)0.))
       {
        memcpy(&KURUCZ_buffers.lambdaF[KURUCZ_buffers.solarFGap],Lambda,sizeof(double)*NDET);
        step=(Lambda[NDET-1]-Lambda[0])/NDET;

        for (i=0;i<KURUCZ_buffers.solarFGap;i++)
         {
          KURUCZ_buffers.lambdaF[i]=Lambda[0]-step*(KURUCZ_buffers.solarFGap-i);
          KURUCZ_buffers.lambdaF[NDET+KURUCZ_buffers.solarFGap+i]=Lambda[NDET-1]+step*(i+1);
         }

        if (((rc=SPLINE_Vector(KURUCZ_buffers.hrSolar.lambda,KURUCZ_buffers.hrSolar.vector,KURUCZ_buffers.hrSolar.deriv2,KURUCZ_buffers.hrSolar.NDET,
                                  KURUCZ_buffers.lambdaF,KURUCZ_buffers.solarF,NDET+2*KURUCZ_buffers.solarFGap,pAnalysisOptions->interpol,"ANALYSE_CurFitMethod "))!=0) ||
            ((rc=FILTER_Vector(ANALYSE_phFilter,KURUCZ_buffers.solarF,KURUCZ_buffers.solarF,NDET+2*KURUCZ_buffers.solarFGap,PRJCT_FILTER_OUTPUT_LOW))!=0) ||
            ((rc=SPLINE_Deriv2(KURUCZ_buffers.lambdaF,KURUCZ_buffers.solarF,KURUCZ_buffers.solarF2,NDET+2*KURUCZ_buffers.solarFGap,"KURUCZ_Alloc (solarF) "))!=0))


           goto EndCurFitMethod;
       }

      hFilterRefLog=1;
     }           */

    for (j=0,k=1,Feno->ymean=(double)0.;(j<Z);j++)
     for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
      Feno->ymean+=(double)RefTrav[l];

    Feno->ymean/=DimL;

    // ---------------------------------
    // Calculation of second derivatives
    // ---------------------------------

    if (((rc=SPLINE_Deriv2(ANALYSE_splineX,SpecTrav,SplineSpec,NDET,"ANALYSE_CurFitMethod (ANALYSE_splineX) "))!=0) ||
        ((rc=SPLINE_Deriv2(ANALYSE_splineX,RefTrav,SplineRef,NDET,"ANALYSE_CurFitMethod (ANALYSE_splineX) "))!=0))

     goto EndCurFitMethod;

    // --------------------------------
    // Initialization of concentrations
    // --------------------------------

    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (!TabFeno[indexFeno].hidden &&
         (Feno==&TabFeno[indexFeno]))
      break;

    for (i=0;i<Feno->NTabCross;i++)                        // parameters initialization
     {
      indexFeno2=ITEM_NONE;

      if (TabCross[i].IndSvdA)
       {
        fitParamsC[TabCross[i].IndSvdA]=TabCross[i].InitConc;

        if ((WorkSpace[TabCross[i].Comp].type==WRK_SYMBOL_CROSS) && (indexFeno<NFeno) &&
            (TabCross[i].FitFromPrevious==1) && (TabCross[i].InitConc==(double)0.) &&
          (((Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) && (TabCross[i].FitConc==0)) ||
           ((Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVDMARQUARDT) && (TabCross[i].FitConc==ITEM_NONE))))
         {
          for (indexFeno2=indexFeno-1;indexFeno2>=0;indexFeno2--)
           if (!TabFeno[indexFeno2].hidden)
            {
             for (j=0;j<TabFeno[indexFeno2].NTabCross;j++)
              if (TabFeno[indexFeno2].TabCross[j].Comp==TabCross[i].Comp)
               {
                double scalingFactor;

                scalingFactor=(double)1.;

                if (!strcasecmp(WorkSpace[TabCross[i].Comp].symbolName,"bro") &&
                    (ANALYSIS_broAmf.matrix!=NULL) &&
                    !SPLINE_Vector(ANALYSIS_broAmf.matrix[0],ANALYSIS_broAmf.matrix[1],ANALYSIS_broAmf.deriv2[1],
                                      ANALYSIS_broAmf.nl,&ZM,&scalingFactor,1,SPLINE_CUBIC,"ANALYSE_CurFitMethod "))

                 fitParamsC[TabCross[i].IndSvdA]=TabFeno[indexFeno2].TabCrossResults[j].SlntCol*scalingFactor;
                else
                 fitParamsC[TabCross[i].IndSvdA]=TabFeno[indexFeno2].TabCrossResults[j].SlntCol;
                break;
               }

             if (j<TabFeno[indexFeno2].NTabCross)
              break;
            }
         }
       }
     }

    if (useErrors)
     {
      for (j=0,k=0;(j<Z);j++)
       for (i=Fenetre[j][0];i<=Fenetre[j][1];i++,k++)
        if ((SpecTrav[i]==(double)0.) || (RefTrav[i]==(double)0.))
        	rc=ERROR_SetLast("ANALYSE_CurFitMethod",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"try to divide errors by a zero");
        else
         SigmaY[k]=(double)sqrt((SigmaSpec[i]*SigmaSpec[i])/(SpecTrav[i]*SpecTrav[i])+
                                (Feno->SrefSigma[i]*Feno->SrefSigma[i])/(RefTrav[i]*RefTrav[i]));

      if (rc!=0)
       goto EndCurFitMethod;
     }


    if ((NF==0) && ((rc=ANALYSE_Function(ANALYSE_splineX,SpecTrav,RefTrav,NDET,Y0,SigmaY,Yfit,DimL,fitParamsC,fitParamsF))<THREAD_EVENT_STOP))
     *Chisqr=(double)Fchisq(pAnalysisOptions->fitWeighting,(int)ANALYSE_nFree,Y0,Yfit,SigmaY,DimL);
    else if (NF)
     {
      for ( i=0; i<NF; i++ ) { fitParamsF[i] = Fitp[i]; Deltap[i] = FitDeltap[i]; }

      /*  ==============  */
      /*  Loop on Chisqr  */
      /*  ==============  */

      *Chisqr    = (double) 0.;
       Lamda     = (double) 0.001;

       niter=0;

       do
       {
          OldChisqr = *Chisqr;

          if ((rc=Curfit(pAnalysisOptions->fitWeighting,(int)ANALYSE_nFree,ANALYSE_splineX,SpecTrav,RefTrav,NDET,Y0,SigmaY,DimL,
                  fitParamsC,fitParamsF,Deltap,Sigmaa,FitMinp,FitMaxp,NF,Yfit,&Lamda,Chisqr,pNiter))>=THREAD_EVENT_STOP)
           break;

          for ( i=0; i<NF; i++ ) Deltap[i] *= 0.4;
          niter++;
       }
       while ( ( *Chisqr != 0. ) && ( fabs(*Chisqr-OldChisqr)/(*Chisqr) > pAnalysisOptions->convergence ) && (Feno->hidden || !pAnalysisOptions->maxIterations || (niter<pAnalysisOptions->maxIterations)) );

      if (pNiter!=NULL)
       *pNiter=niter;
     }

    if (rc<THREAD_EVENT_STOP)
     {
      /*  ====================  */
      /*  Residual Computation  */
      /*  ====================  */

      for ( j=k=0; j<Z; j++ )
       for ( i=Fenetre[j][0]; i<=Fenetre[j][1]; i++, k++ )
        {
         ANALYSE_absolu[i]  =  (Yfit[k]-Y0[k]);
         if (Feno->analysisMethod!=PRJCT_ANLYS_METHOD_SVD)
          ANALYSE_t[i]=(ANALYSE_tc[i]!=(double)0.)?(double)1.+ANALYSE_absolu[i]/ANALYSE_tc[i]:(double)0.;
        }

      scalingFactor=(pAnalysisOptions->fitWeighting==PRJCT_ANLYS_FIT_WEIGHTING_NONE)?(*Chisqr):(double)1.;

      for (i=0;i<Feno->NTabCross;i++)
       {
        pResults=&Feno->TabCrossResults[i];
        pTabCross=&TabCross[i];

/*
if (!Feno->hidden)
{
	FILE *fp;
	fp=fopen("toto.dat","a+t");
	fprintf(fp,"%-20s %-2d %-2d %-2d %-2d\n",
	            WorkSpace[pTabCross->Comp].symbolName,
	            pTabCross->FitConc,pTabCross->FitParam,pTabCross->IndSvdA,pTabCross->IndSvdP);
	fclose(fp);
}
*/

        /*  ==================================  */
        /*  Concentrations Scaling for Display  */
        /*  ==================================  */

        if (pTabCross->IndSvdA) // Cross section, polynomial, linear offset, undersampling
         {
         	// Fitting using SVD -> in SVD+Marquardt, polynomial is also fitted linearly !

          if (((Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) && (pTabCross->FitParam==ITEM_NONE)) || pTabCross->IndSvdP)
           {
            pResults->SlntCol=x[pTabCross->IndSvdA]=(pTabCross->FitConc!=0)?fitParamsC[pTabCross->IndSvdA]/pTabCross->Fact:fitParamsC[TabCross[i].IndSvdA];
            pResults->SlntErr=Sigma[pTabCross->IndSvdA]=(pTabCross->FitConc!=0)?(double)sqrt(SigmaSqr[pTabCross->IndSvdA]*scalingFactor)/pTabCross->Fact:(double)0.;

            if (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CONTINUOUS)
             {
            	 if (!Feno->hidden)
            	  {
              	 // Intensity fitting but polynomial is fitted linearly

                if ((pTabCross->IndSvdP) && (fabs(refNormFact)>EPSILON))
                	pResults->SlntCol*=(double)speNormFact/refNormFact;

                // SVD

                else if ((fabs(speNormFact)>EPSILON) && (refNormFact/speNormFact>EPSILON))
                 {
                 	pResults->SlntCol=-pResults->SlntCol;

                 	if (!strcasecmp(WorkSpace[pTabCross->Comp].symbolName,"x0"))
                 	 pResults->SlntCol-=(double)log(refNormFact/speNormFact);
              	  }
               }
            	}
           }
          else  // cross sections in SVD+Marquardt method or Raman in SVD method
           {
            pResults->SlntCol=x[pTabCross->IndSvdA] = (pTabCross->FitConc!=ITEM_NONE)?fitParamsF[pTabCross->FitConc] / pTabCross->Fact:fitParamsC[TabCross[i].IndSvdA];
            pResults->SlntErr=Sigma[pTabCross->IndSvdA] = (pTabCross->FitConc!=ITEM_NONE)?(double) Sigmaa[pTabCross->FitConc] / pTabCross->Fact:(double)0.;

            if ((WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CONTINUOUS) && (fabs(refNormFact)>EPSILON))
             pResults->SlntCol*=(double)speNormFact/refNormFact;
           }
         }

        /*  =============  */
        /*  Store results  */
        /*  =============  */

        else if (pTabCross->FitParam!=ITEM_NONE)
         pResults->Param=(double)fitParamsF[pTabCross->FitParam]/pTabCross->Fact;
        else
         pResults->Param=pTabCross->InitParam;

        pResults->Shift = ( pTabCross->FitShift != ITEM_NONE ) ? (double) fitParamsF[pTabCross->FitShift] : pTabCross->InitShift;
        pResults->Stretch = ( pTabCross->FitStretch != ITEM_NONE ) ? (double) fitParamsF[pTabCross->FitStretch]*StretchFact1 : pTabCross->InitStretch*StretchFact1;
        pResults->Stretch2 = ( pTabCross->FitStretch2 != ITEM_NONE ) ? (double) fitParamsF[pTabCross->FitStretch2]*StretchFact2 : pTabCross->InitStretch2*StretchFact2;
        pResults->Scale = ( pTabCross->FitScale != ITEM_NONE ) ? (double) fitParamsF[pTabCross->FitScale] : pTabCross->InitScale;
        pResults->Scale2 = ( pTabCross->FitScale2 != ITEM_NONE ) ? (double) fitParamsF[pTabCross->FitScale2] : pTabCross->InitScale2;

        pResults->SigmaParam = (pTabCross->FitParam != ITEM_NONE) ? Sigmaa[(!pTabCross->IndSvdA)?pTabCross->FitParam:pTabCross->IndSvdA]/pTabCross->Fact : (double)1.;
        pResults->SigmaShift = (pTabCross->FitShift != ITEM_NONE) ? Sigmaa[pTabCross->FitShift] : (double)1.;
        pResults->SigmaStretch = (pTabCross->FitStretch != ITEM_NONE) ? Sigmaa[pTabCross->FitStretch]*StretchFact1 : (double)1.;
        pResults->SigmaStretch2 = (pTabCross->FitStretch2 != ITEM_NONE) ? Sigmaa[pTabCross->FitStretch2]*StretchFact2 : (double)1.;
        pResults->SigmaScale = (pTabCross->FitScale != ITEM_NONE) ? Sigmaa[pTabCross->FitScale] : (double)1.;
        pResults->SigmaScale2 = (pTabCross->FitScale2 != ITEM_NONE) ? Sigmaa[pTabCross->FitScale2] : (double)1.;
       }
     }
   }

  /*  ===========  */
  /*  Free Memory  */
  /*  ===========  */

  EndCurFitMethod :

  for (i=0;i<Feno->NTabCross;i++)
   {
    pTabCross=&TabCross[i];

    pTabCross->InitParam*=pTabCross->Fact;
    pTabCross->InitStretch*=StretchFact1;
    pTabCross->InitStretch2*=StretchFact2;
   }

  if (Sigmaa!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_CurFitMethod ","Sigmaa",Sigmaa,0);
  if (fitParamsC!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_CurFitMethod ","fitParamsC",fitParamsC,0);
  if (fitParamsF!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_CurFitMethod ","fitParamsF",fitParamsF,0);
  if (Deltap!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_CurFitMethod ","Deltap",Deltap,0);
  if (Yfit!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_CurFitMethod ","Yfit",Yfit,0);
  if (Y0!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_CurFitMethod ","Y0",Y0,0);
  if (SigmaY!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_CurFitMethod ","SigmaY",SigmaY,0);
  if (SpecTrav!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_CurFitMethod ","SpecTrav",SpecTrav,0);
  if (RefTrav!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_CurFitMethod ","RefTrav",RefTrav,0);

  // Return

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ANALYSE_CurFitMethod",rc);
  #endif

  return rc;
 }

// -------------------------------------------
// ANALYSE_Spectrum : Spectrum record analysis
// -------------------------------------------

RC ANALYSE_Spectrum(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
 {
  // Declarations

  PROJECT *pProject;                                                            // pointer to the project part of the engine context
  PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  CROSS_REFERENCE *TabCross;                 // list of symbols hold by a analysis window
  CROSS_RESULTS *Results;                    // corresponding results
  FENO *pTabFeno;
  DoasCh windowTitle[MAX_ITEM_TEXT_LEN+1];    // window title for graphs
  DoasCh tabTitle[MAX_ITEM_TEXT_LEN+1];
  DoasCh graphTitle[MAX_ITEM_TEXT_LEN+1];     // graph title
  INDEX WrkFeno,j;                             // index on analysis windows
  INDEX i,k,l;                               // indexes for loops and arrays

  double j0,lambda0;

  double *Spectre,                           // raw spectrum
         *SpectreK,                          // spectrum shifted on new calibration build by Kurucz
         *LambdaK,                           // calibration found by Kurucz
         *Sref,                              // reference spectrum
         *Trend,                             // fitted trend
         *offset,                            // fitted linear offset
          maxOffset,
          newVal,
          speNormFact,
          factTemp;                          // normalization factor

  INT NbFeno,Niter,
      displayFlag,                           // number of MDI child windows used for display analysis fits
      useKurucz,                             // flag set if Kurucz should be applied on spectra
      saveFlag;
// QDOAS ???   FILE *fp;                     // pointer to temporary file

  INDEX indexPage,indexLine,indexColumn;
  RC  rc;                                    // return code
  int nrc;

  plot_data_t spectrumData[2];
  INT n,n2;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ANALYSE_Spectrum",DEBUG_FCTTYPE_APPL);
  #endif

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;
  pProject=&pEngineContext->project;
  pInstrumental=&pProject->instrumental;

  memcpy(ANALYSE_t,ANALYSE_zeros,sizeof(double)*NDET);
  memcpy(ANALYSE_tc,ANALYSE_zeros,sizeof(double)*NDET);

  speNormFact=(double)1.;
  ZM=pRecord->Zm;
  TDET=pRecord->TDet;

  saveFlag=(INT)pEngineContext->project.spectra.displayDataFlag;
  SpectreK=LambdaK=Sref=Trend=offset=NULL;
  useKurucz=0;

  NbFeno=0;
  nrc=0;

  rc=ERROR_ID_NO;

  // Buffers allocation

  if (((Spectre=(double *)MEMORY_AllocDVector("ANALYSE_Spectrum ","Spectre",0,NDET-1))==NULL) ||
      ((Sref=(double *)MEMORY_AllocDVector("ANALYSE_Spectrum ","Sref",0,NDET-1))==NULL) ||
      ((Trend=(double *)MEMORY_AllocDVector("ANALYSE_Spectrum ","Trend",0,NDET-1))==NULL) ||
      ((offset=(double *)MEMORY_AllocDVector("ANALYSE_Spectrum ","offset",0,NDET-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Spectrum normalization

    memcpy(Spectre,pBuffers->spectrum,sizeof(double)*NDET);

    if (((rc=VECTOR_NormalizeVector(Spectre-1,NDET,&speNormFact,"ANALYSE_Spectrum (Spectrum) "))!=ERROR_ID_NO) ||
        (pRecord->useErrors && ((rc=VECTOR_NormalizeVector(pBuffers->sigmaSpec-1,NDET,&factTemp,"ANALYSE_Spectrum (sigmaSpec) "))!=ERROR_ID_NO)))
     goto EndAnalysis;

    // Apply Kurucz on spectrum

    for (WrkFeno=0;WrkFeno<NFeno;WrkFeno++)
     if (!TabFeno[WrkFeno].hidden &&
        ((TabFeno[WrkFeno].useKurucz==ANLYS_KURUCZ_REF_AND_SPEC) ||
         (TabFeno[WrkFeno].useKurucz==ANLYS_KURUCZ_SPEC)))

      useKurucz++;

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
     {
      if (((SpectreK=(double *)MEMORY_AllocDVector("ANALYSE_Spectrum ","SpectreK",0,NDET-1))==NULL) ||
          ((LambdaK=(double *)MEMORY_AllocDVector("ANALYSE_Spectrum ","LambdaK",0,NDET-1))==NULL))

       rc=ERROR_ID_ALLOC;

      else
       {
        memcpy(SpectreK,Spectre,sizeof(double)*NDET);

        if (!(rc=KURUCZ_Spectrum(pBuffers->lambda,LambdaK,SpectreK,KURUCZ_buffers.solar,pBuffers->instrFunction,
                                 1,"Calibration applied on spectrum",KURUCZ_buffers.fwhmPolySpec,KURUCZ_buffers.fwhmVector,KURUCZ_buffers.fwhmDeriv2,saveFlag,
                                 KURUCZ_buffers.indexKurucz,responseHandle)))

         for (WrkFeno=0,pTabFeno=&TabFeno[WrkFeno];WrkFeno<NFeno;pTabFeno=&TabFeno[++WrkFeno])
          if (!pTabFeno->hidden && (pTabFeno->useKurucz==ANLYS_KURUCZ_SPEC))
           {
            memcpy(pTabFeno->LambdaK,LambdaK,sizeof(double)*NDET);
            memcpy(pTabFeno->Lambda,LambdaK,sizeof(double)*NDET);

            if ((rc=KURUCZ_ApplyCalibration(pTabFeno,LambdaK))!=ERROR_ID_NO)
             goto EndAnalysis;
           }

        memcpy(SpectreK,Spectre,sizeof(double)*NDET); // !!!
       }

      if (rc>=THREAD_EVENT_STOP)
       goto EndAnalysis;
     }

    pRecord->BestShift=(double)0.;

    if (THRD_id==THREAD_TYPE_ANALYSIS)
     {
      // Browse analysis windows

      n=0,n2=0;

      for (WrkFeno=0;(WrkFeno<NFeno) && (rc!=THREAD_EVENT_STOP);WrkFeno++)
       {
       	indexPage=WrkFeno+plotPageAnalysis;
        Feno=&TabFeno[WrkFeno];

        Feno->rc=(!Feno->hidden && VECTOR_Equal(Spectre,Feno->Sref,NDET,(double)1.e-7))?-1:ERROR_ID_NO;

        sprintf(windowTitle,"Analysis results for %s window",Feno->windowName);
        sprintf(tabTitle,"%s results (%d/%d)",Feno->windowName,pEngineContext->indexRecord,pEngineContext->recordNumber);

        displayFlag=Feno->displaySpectrum+                                      //  force display spectrum
                    Feno->displayResidue+                                       //  force display residue
                    Feno->displayTrend+                                         //  force display trend
                    Feno->displayRefEtalon+                                     //  force display alignment of reference on etalon
                    Feno->displayFits+                                          //  force display fits
                    Feno->displayPredefined+                                    //  force display predefined parameters
                    Feno->displayRef;

        if (displayFlag)
         mediateResponseLabelPage(indexPage,pEngineContext->fileInfo.fileName,tabTitle,responseHandle);

        if (!Feno->hidden && (Feno->rcKurucz==ERROR_ID_NO) &&
          ((Feno->useKurucz==ANLYS_KURUCZ_SPEC) || !Feno->rc))
         {
          memcpy(ANALYSE_absolu,ANALYSE_zeros,sizeof(double)*NDET);

          if (Feno->amfFlag ||
            ((Feno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC) && Feno->xsToConvolute) ||
             (Feno->offlFlag && (Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD)))   // fit a linear offset using the inverse of the spectrum

           Feno->Decomp=1;

          // Local variables initializations

          Niter=0;
          NbFeno++;
          TabCross=Feno->TabCross;
          Results=Feno->TabCrossResults;

          // Reference spectrum

          memcpy(Sref,Feno->Sref,sizeof(double)*NDET);
          Lambda=Feno->LambdaK;

          if ((rc=ANALYSE_SvdInit(&Feno->svd))!=ERROR_ID_NO)
           goto EndAnalysis;

          // Global variables initializations

          if ((Feno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) &&
               pEngineContext->satelliteFlag)
           {
            if ((fabs(ANALYSE_oldLatitude)>(double)360.) ||
               ((ANALYSE_oldLatitude>=(double)0.) && (pRecord->latitude<(double)0.)) ||
               ((ANALYSE_oldLatitude<(double)0.) && (pRecord->latitude>=(double)0.)))
             {
              if (pRecord->latitude>=(double)0.)
               {
                Feno->Shift=Feno->ShiftN;
                Feno->Stretch=Feno->StretchN;
                Feno->Stretch2=Feno->Stretch2N;
                Feno->refNormFact=Feno->refNormFactN;

                memcpy(Feno->Sref,Feno->SrefN,sizeof(double)*NDET);

                if (!Feno->useKurucz)
                 memcpy(Feno->LambdaK,Feno->LambdaN,sizeof(double)*NDET);

               }
              else
               {
                Feno->Shift=Feno->ShiftS;
                Feno->Stretch=Feno->StretchS;
                Feno->Stretch2=Feno->Stretch2S;
                Feno->refNormFact=Feno->refNormFactS;

                memcpy(Feno->Sref,Feno->SrefS,sizeof(double)*NDET);

                if (!Feno->useKurucz)
                 memcpy(Feno->LambdaK,Feno->LambdaS,sizeof(double)*NDET);
               }

              // Undersampling

              if (!Feno->useKurucz &&
                (((rc=KURUCZ_ApplyCalibration(Feno,Feno->LambdaK))!=ERROR_ID_NO) ||
                 ((rc=ANALYSE_SvdInit(&Feno->svd))!=ERROR_ID_NO)))

               goto EndAnalysis;

              if (Feno->useUsamp && (THRD_id!=THREAD_TYPE_KURUCZ))
               {
                ANALYSE_UsampLocalFree();

                if (((rc=ANALYSE_UsampLocalAlloc(0))!=ERROR_ID_NO) ||
                    ((rc=ANALYSE_UsampBuild(2,ITEM_NONE))!=ERROR_ID_NO))
                 goto EndAnalysis;
               }
             }

            //if (pEngineContext->project.spectra.displaySpectraFlag)
            // {
            //  sprintf(tabTitle,"%s results (%d/%d)",Feno->windowName,pEngineContext->indexRecord,pEngineContext->recordNumber);
            //
            //  mediateAllocateAndSetPlotData(&spectrumData[0],"Measured",&Feno->LambdaK[SvdPDeb],&Feno->Sref[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
            //  mediateResponsePlotData(indexPage,&spectrumData[0],1,Spectrum,forceAutoScale,"Selected reference","Wavelength (nm)","Intensity", responseHandle);
            //  mediateResponseLabelPage(indexPage, pEngineContext->fileInfo.fileName, tabTitle, responseHandle);
            //  mediateReleasePlotData(&spectrumData[0]);
            //
            //  if (pEngineContext->project.spectra.displayDataFlag)
            //   {
            //    Feno->displayLineIndex=mediateRequestDisplaySpecInfo(pEngineContext,indexPage,responseHandle);
            //    indexLine=Feno->displayLineIndex+1;
            //    indexColumn=2;
            //
            //    mediateResponseCellDataString(indexPage,indexLine++,indexColumn,"Ref Selection",responseHandle);
            //    mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Ref File","%s",GDP_BIN_orbitFiles[Feno->satelliteRef[0].indexFile].gdpBinFileName);
            //    mediateResponseCellDataString(indexPage,indexLine,indexColumn,"Record",responseHandle);
            //    mediateResponseCellDataString(indexPage,indexLine,indexColumn+1,"Pixel number",responseHandle);
            //    mediateResponseCellDataString(indexPage,indexLine,indexColumn+2,"Pixel type",responseHandle);
            //    mediateResponseCellDataString(indexPage,indexLine,indexColumn+3,"SZA",responseHandle);
            //    mediateResponseCellDataString(indexPage,indexLine,indexColumn+4,"Lat",responseHandle);
            //    mediateResponseCellDataString(indexPage,indexLine,indexColumn+5,"Lon",responseHandle);
            //
            //    indexLine++;
            //
            //    for (indexRef=0;indexRef<OUTPUT_nRec;indexRef++)
            //     {
            //     	pRef=&Feno->satelliteRef[indexRef];
            //
            //      mediateResponseCellDataInteger(indexPage,indexLine,indexColumn,pRef->indexRecord+1,responseHandle);
            //      mediateResponseCellDataInteger(indexPage,indexLine,indexColumn+1,pRef->pixelNumber,responseHandle);
            //      mediateResponseCellDataInteger(indexPage,indexLine,indexColumn+2,pRef->pixelType,responseHandle);
            //      mediateResponseCellDataDouble(indexPage,indexLine,indexColumn+3,pRef->sza,responseHandle);
            //      mediateResponseCellDataDouble(indexPage,indexLine,indexColumn+4,pRef->latitude,responseHandle);
            //      mediateResponseCellDataDouble(indexPage,indexLine,indexColumn+5,pRef->longitude,responseHandle);
            //
            //      indexLine++;
            //     }
            //
            //    Feno->displayLineIndex=indexLine+1;
            //   }
            //
            //  //if (Feno->displayRefEtalon)
            //  // {
            //  //  sprintf(string,"Alignment Ref1/Ref2");
            //  //
            //  //  mediateAllocateAndSetPlotData(&spectrumData[0],"Measured",&Lambda[SvdPDeb],&Spectre[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
            //  //  mediateAllocateAndSetPlotData(&spectrumData[1],"Calculated",&Lambda[SvdPDeb],&ANALYSE_secX[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
            //  //  mediateResponsePlotData(indexPage,spectrumData,2,Spectrum,forceAutoScale,string,"Wavelength (nm)","Intensity", responseHandle);
            //  //  mediateResponseLabelPage(indexPage,pEngineContext->fileInfo.fileName, "Reference", responseHandle);
            //  //  mediateReleasePlotData(&spectrumData[1]);
            //  //  mediateReleasePlotData(&spectrumData[0]);
            //  // }
            // }
           }

          // Reference spectrum

          memcpy(Sref,Feno->Sref,sizeof(double)*NDET);
          Lambda=Feno->LambdaK;

          // Display spectrum in the current analysis window

          if (strlen(pRecord->Nom))
           sprintf(windowTitle,"Analysis of %s in %s window",pRecord->Nom,Feno->windowName);
          else
           sprintf(windowTitle,"Analysis of spectrum %d/%d in %s window",pEngineContext->indexRecord,pEngineContext->recordNumber,Feno->windowName);

          if (Feno->displaySpectrum)
           {
            mediateAllocateAndSetPlotData(&spectrumData[0],"Spectrum",&Feno->LambdaK[SvdPDeb],&Spectre[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
            mediateAllocateAndSetPlotData(&spectrumData[1],"Reference",&Feno->LambdaK[SvdPDeb],&Sref[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
            mediateResponsePlotData(indexPage,spectrumData,2,Spectrum,forceAutoScale,"Spectrum and reference","Wavelength (nm)","", responseHandle);
            mediateReleasePlotData(&spectrumData[1]);
            mediateReleasePlotData(&spectrumData[0]);
           }

          // Analysis method

          #if defined(__DEBUG_) && __DEBUG_
          DEBUG_Start(ENGINE_dbgFile,"Test",(analyseDebugMask=DEBUG_FCTTYPE_MATH|DEBUG_FCTTYPE_APPL),5,(analyseDebugVar=DEBUG_DVAR_YES),0); // !debugResetFlag++);
          #endif

/*             ((Feno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC) &&
            (((rc=SPLINE_Deriv2(LambdaK,Spectre,SplineSpec,NDET,"Spline(Spectre) "))!=ERROR_ID_NO) ||
             ((rc=SPLINE_Vector(LambdaK,Spectre,SplineSpec,NDET,Lambda,SpectreK,NDET,pAnalysisOptions->interpol,"ANALYSE_Spectrum "))!=ERROR_ID_NO))) || */
          if ((rc=ANALYSE_CurFitMethod((Feno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC)?SpectreK:Spectre,     // raw spectrum
                                       (pRecord->useErrors)?pBuffers->sigmaSpec:NULL,                     // error on raw spectrum
                                        Sref,                                                             // reference spectrum
                                       &Feno->chiSquare,                                                  // returned stretch order 2
                                       &Niter,
                                       speNormFact,
                                       Feno->refNormFact))==THREAD_EVENT_STOP)                                       // number of iterations in Curfit

           goto EndAnalysis;  // !!!! Bypass the DEBUG_Stop

          else if (rc>THREAD_EVENT_STOP)
           Feno->rc=rc;

          #if defined(__DEBUG_) && __DEBUG_
          DEBUG_Stop("Test");
          analyseDebugMask=0;
          #endif

          pRecord->BestShift+=(double)Feno->TabCrossResults[Feno->indexSpectrum].Shift;
          Feno->nIter=Niter;

          // Output analysis results to temporary file

          if (Feno->analysisMethod!=PRJCT_ANLYS_METHOD_SVD)
           {
            Feno->chiSquare=(double)0.;

            for (j=SvdPDeb;j<=SvdPFin;j++)
             if (ANALYSE_t[j]>(double)0.)
              Feno->chiSquare+=(double)log(ANALYSE_t[j])*log(ANALYSE_t[j]);

            Feno->chiSquare/=ANALYSE_nFree;
           }

          Feno->RMS=(Feno->chiSquare>(double)0.)?sqrt(Feno->chiSquare):(double)0.;

          // Display residual spectrum

          if  (Feno->displayResidue)
           {
            if (Feno->analysisMethod!=PRJCT_ANLYS_METHOD_SVD)
             for (j=SvdPDeb;j<=SvdPFin;j++)
              ANALYSE_absolu[j]=(ANALYSE_tc[j]!=(double)0.)?ANALYSE_absolu[j]/ANALYSE_tc[j]:(double)0.;

           	sprintf(graphTitle,"%s (%.2le)",(Feno->analysisMethod!=PRJCT_ANLYS_METHOD_SVD)?"Normalized Residual":"Residual",Feno->RMS);

            mediateAllocateAndSetPlotData(&spectrumData[0],(Feno->analysisMethod!=PRJCT_ANLYS_METHOD_SVD)?"Normalized Residual":"Residual",&Feno->LambdaK[SvdPDeb],&ANALYSE_absolu[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
            mediateResponsePlotData(indexPage,spectrumData,1,Residual,0,graphTitle,"Wavelength (nm)","", responseHandle);
            mediateReleasePlotData(&spectrumData[0]);
           }

          if (Feno->analysisMethod!=PRJCT_ANLYS_METHOD_SVD)
           for (j=SvdPDeb;j<=SvdPFin;j++)
            ANALYSE_absolu[j]=(ANALYSE_t[j]>(double)0.)?log(ANALYSE_t[j]):(double)0.;

          if (strlen(Feno->residualsFile) &&
            ((rc=AnalyseSaveResiduals(Feno->residualsFile,pEngineContext))!=ERROR_ID_NO))

           goto EndAnalysis;

          if  (Feno->displayResidue && (Feno->analysisMethod!=PRJCT_ANLYS_METHOD_SVD))
           {
            mediateAllocateAndSetPlotData(&spectrumData[0],"OD residual",&Feno->LambdaK[SvdPDeb],&ANALYSE_absolu[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
            mediateResponsePlotData(indexPage,spectrumData,1,Residual,0,"OD residual","Wavelength (nm)","", responseHandle);
            mediateReleasePlotData(&spectrumData[0]);
           }

          // Store fits

          memcpy(ANALYSE_secX,ANALYSE_zeros,sizeof(double)*NDET);
          memcpy(Trend,ANALYSE_zeros,sizeof(double)*NDET);
          memcpy(offset,ANALYSE_zeros,sizeof(double)*NDET);
          maxOffset=(double)0.;

          // Display Offset

          if  (Feno->displayPredefined &&
             ((Feno->indexOffsetConst!=ITEM_NONE) ||
              (Feno->indexOffsetOrder1!=ITEM_NONE) ||
              (Feno->indexOffsetOrder2!=ITEM_NONE)) &&

             ((TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE) ||
              (TabCross[Feno->indexOffsetOrder1].FitParam!=ITEM_NONE) ||
              (TabCross[Feno->indexOffsetOrder2].FitParam!=ITEM_NONE) ||
              (TabCross[Feno->indexOffsetConst].InitParam!=(double)0.) ||
              (TabCross[Feno->indexOffsetOrder1].InitParam!=(double)0.) ||
              (TabCross[Feno->indexOffsetOrder2].InitParam!=(double)0.)))
           {
            j0=(double)(SvdPDeb+SvdPFin)*0.5;
            lambda0=(fabs(j0-floor(j0))<(double)0.1)?
                                        (double)ANALYSE_splineX[(INDEX)j0]:
                                        (double)0.5*(ANALYSE_splineX[(INDEX)floor(j0)]+ANALYSE_splineX[(INDEX)floor(j0+1.)]);

            for (j=0,k=1;j<Z;j++)
             for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)       // log(I+offset)=log(I)+log(1+offset/I)
              {
               newVal=(double)1.-Feno->xmean*(Results[Feno->indexOffsetConst].Param+
                              Results[Feno->indexOffsetOrder1].Param*(ANALYSE_splineX[l]-lambda0)+
                              Results[Feno->indexOffsetOrder2].Param*(ANALYSE_splineX[l]-lambda0)*(ANALYSE_splineX[l]-lambda0))/Spectre[l];

               ANALYSE_absolu[l]+=((newVal>(double)0.)?log(newVal):(double)0.)-ANALYSE_secX[l];
               ANALYSE_secX[l]=((newVal>(double)0.)?log(newVal):(double)0.);
              }

            mediateAllocateAndSetPlotData(&spectrumData[0],"Measured offset",&Feno->LambdaK[SvdPDeb],&ANALYSE_absolu[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
            mediateAllocateAndSetPlotData(&spectrumData[1],"Calculated offset",&Feno->LambdaK[SvdPDeb],&ANALYSE_secX[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
            mediateResponsePlotData(indexPage,spectrumData,2,Residual,allowFixedScale,"Offset","Wavelength (nm)","", responseHandle);
            mediateReleasePlotData(&spectrumData[1]);
            mediateReleasePlotData(&spectrumData[0]);
           }

          // Display fits

          for (i=0;i<Feno->NTabCross;i++)

           if (TabCross[i].IndSvdA)
            {
             if (((WorkSpace[TabCross[i].Comp].type==WRK_SYMBOL_CROSS) ||
                  (WorkSpace[TabCross[i].Comp].type==WRK_SYMBOL_PREDEFINED)) &&
                   Feno->displayFits && TabCross[i].display)
              {
            	  if (Feno->indexRing1==i)
            	   {
                 for (j=0,k=1;j<Z;j++)
                  for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
                   {
                    newVal=(Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVD)?
                            log(1+x[TabCross[i].IndSvdA]*U[TabCross[i].IndSvdA][k]/Spectre[l]):
                            log(1-x[TabCross[i].IndSvdA]*U[TabCross[i].IndSvdA][k]*Sref[l]/Spectre[l]);

                    ANALYSE_absolu[l]+=newVal-ANALYSE_secX[l];
                    ANALYSE_secX[l]=newVal;
                   }

                 sprintf(graphTitle,"%s (%.2le)",WorkSpace[TabCross[i].Comp].symbolName,Results[i].SlntCol);
            	   }
              	else
              	 {
                 for (j=0,k=1;j<Z;j++)
                  for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
                   {
                    newVal=x[TabCross[i].IndSvdA]*U[TabCross[i].IndSvdA][k];

                    ANALYSE_absolu[l]+=newVal-ANALYSE_secX[l];
                    ANALYSE_secX[l]=newVal;
                   }

                 sprintf(graphTitle,"%s (%.2le)",WorkSpace[TabCross[i].Comp].symbolName,Results[i].SlntCol);
                }

               mediateAllocateAndSetPlotData(&spectrumData[0],"Measured",&Feno->LambdaK[SvdPDeb],&ANALYSE_absolu[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
               mediateAllocateAndSetPlotData(&spectrumData[1],"Calculated",&Feno->LambdaK[SvdPDeb],&ANALYSE_secX[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
               mediateResponsePlotData(indexPage,spectrumData,2,Residual,0,graphTitle,"Wavelength (nm)","", responseHandle);
               mediateReleasePlotData(&spectrumData[1]);
               mediateReleasePlotData(&spectrumData[0]);
              }
             else if ((WorkSpace[TabCross[i].Comp].type==WRK_SYMBOL_CONTINUOUS) && Feno->displayTrend)
              {
              	if (((char)tolower(WorkSpace[TabCross[i].Comp].symbolName[0])=='x') ||
                   ((char)tolower(WorkSpace[TabCross[i].Comp].symbolName[2])=='x'))

                for (j=0,k=1;j<Z;j++)
                 for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
                  Trend[l]+=x[TabCross[i].IndSvdA]*U[TabCross[i].IndSvdA][k];

               else if ((WorkSpace[TabCross[i].Comp].symbolName[0]=='o') ||
                        (WorkSpace[TabCross[i].Comp].symbolName[1]=='f') ||
                        (WorkSpace[TabCross[i].Comp].symbolName[2]=='f') ||
                        (WorkSpace[TabCross[i].Comp].symbolName[3]=='l'))
                {
                 for (j=0,k=1;j<Z;j++)
                  for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
                   offset[l]+=x[TabCross[i].IndSvdA]*U[TabCross[i].IndSvdA][k];

                 for (j=0,k=1;j<Z;j++)
                  for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
                   if (fabs(offset[l])>maxOffset)
                    maxOffset=fabs(offset[l]);
                }
              }
            }

          // Display Trend

          if (Feno->displayTrend)
           {
            for (j=0,k=1;j<Z;j++)
             for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
              ANALYSE_absolu[l]+=Trend[l]-ANALYSE_secX[l];

            mediateAllocateAndSetPlotData(&spectrumData[0],"Measured",&Feno->LambdaK[SvdPDeb],&ANALYSE_absolu[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
            mediateAllocateAndSetPlotData(&spectrumData[1],"Calculated",&Feno->LambdaK[SvdPDeb],&Trend[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
            mediateResponsePlotData(indexPage,spectrumData,2,Residual,allowFixedScale,"Polynomial","Wavelength (nm)","", responseHandle);
            mediateReleasePlotData(&spectrumData[1]);
            mediateReleasePlotData(&spectrumData[0]);

            if (maxOffset>(double)0.)
             {
             	if (Feno->analysisMethod==PRJCT_ANLYS_METHOD_SVDMARQUARDT)
               for (j=0,k=1;j<Z;j++)
                for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
                 offset[l]=-offset[l];           // inverse the sign in order to have the same display as in SVD method

              for (j=0,k=1;j<Z;j++)
               for (l=Fenetre[j][0];l<=Fenetre[j][1];l++,k++)
                ANALYSE_absolu[l]+=offset[l]-Trend[l];

              mediateAllocateAndSetPlotData(&spectrumData[0],"Measured",&Feno->LambdaK[SvdPDeb],&ANALYSE_absolu[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
              mediateAllocateAndSetPlotData(&spectrumData[1],"Calculated",&Feno->LambdaK[SvdPDeb],&offset[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
              mediateResponsePlotData(indexPage,spectrumData,2,Residual,allowFixedScale,"Linear offset","Wavelength (nm)","", responseHandle);
              mediateReleasePlotData(&spectrumData[1]);
              mediateReleasePlotData(&spectrumData[0]);
             }
           }  // end displayTrend

          if (!Feno->rc)
           nrc++;

          if (displayFlag && saveFlag)
           {
           	indexLine=Feno->displayLineIndex;
            indexColumn=2;

           	mediateResponseCellDataString(indexPage,indexLine,indexColumn,tabTitle,responseHandle);

            indexLine+=2;

           	mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"OD ChiSquare","%.5le",Feno->chiSquare);
           	mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"RMS Residual","%.5le",Feno->RMS);
           	mediateResponseCellInfo(indexPage,indexLine,indexColumn,responseHandle,"Iterations","%d",Niter);

           	indexLine+=2;

           	mediateResponseCellDataString(indexPage,indexLine,indexColumn+1,"[CONC/Param]",responseHandle);
           	mediateResponseCellDataString(indexPage,indexLine,indexColumn+2,"Shift",responseHandle);
           	mediateResponseCellDataString(indexPage,indexLine,indexColumn+3,"Stretch",responseHandle);

           	indexLine++;

            for (i=0;i<Feno->NTabCross;i++)
             {
              mediateResponseCellDataString(indexPage,indexLine,indexColumn,WorkSpace[TabCross[i].Comp].symbolName,responseHandle);
           // -------------------------------------------------------------------
              if (TabCross[i].IndSvdA)
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+1,responseHandle,"%10.3e +/-%10.3e",Results[i].SlntCol,Results[i].SlntErr);
              else if (TabCross[i].FitParam!=ITEM_NONE)
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+1,responseHandle,"%10.3e +/-%10.3e",Results[i].Param,Results[i].SigmaParam);
              else
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+1,responseHandle,"%10.3e",Results[i].Param);
           // -------------------------------------------------------------------
              if (TabCross[i].FitShift!=ITEM_NONE)
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+2,responseHandle,"%10.3e +/-%10.3e",Results[i].Shift,Results[i].SigmaShift);
              else
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+2,responseHandle,"%10.3e",Results[i].Shift);
           // -------------------------------------------------------------------
              if (TabCross[i].FitStretch!=ITEM_NONE)
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+3,responseHandle,"%10.3e +/-%10.3e",Results[i].Stretch,Results[i].SigmaStretch);
              else
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+3,responseHandle,"%10.3e",Results[i].Stretch);
           // -------------------------------------------------------------------
//              if (TabCross[i].FitStretch2!=ITEM_NONE)
//               fprintf(fp,"%10.3e +/-%10.3e\t",Results[i].Stretch2,Results[i].SigmaStretch2);
//              else
//               fprintf(fp,"%10.3e\t\t",Results[i].Stretch2);
//           // -------------------------------------------------------------------
//              if (TabCross[i].FitScale!=ITEM_NONE)
//               fprintf(fp,"%10.3e +/-%10.3e\t",Results[i].Scale,Results[i].SigmaScale);
//              else
//               fprintf(fp,"%10.3e\t\t",Results[i].Scale);
//           // -------------------------------------------------------------------
//              if (TabCross[i].FitScale2!=ITEM_NONE)
//               fprintf(fp,"%10.3e +/-%10.3e\t",Results[i].Scale2,Results[i].SigmaScale2);
//              else
//               fprintf(fp,"%10.3e\t\t",Results[i].Scale2);
           // -------------------------------------------------------------------
              indexLine++;
             }
           }
         }
       }
     }

    if (NbFeno)
     pRecord->BestShift/=(double)NbFeno;

    ANALYSE_oldLatitude=pRecord->latitude;

    if ((pEngineContext->lastSavedRecord!=pEngineContext->indexRecord) && ((THRD_id==THREAD_TYPE_KURUCZ) || nrc) &&
      (((THRD_id==THREAD_TYPE_ANALYSIS) && pProject->asciiResults.analysisFlag) || ((THRD_id==THREAD_TYPE_KURUCZ) && pProject->asciiResults.calibFlag)))

     rc=OUTPUT_SaveResults(pEngineContext);
   }

  // Return

  EndAnalysis :

  if (Spectre!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_Spectrum ","Spectre",Spectre,0);
  if (SpectreK!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_Spectrum ","SpectreK",SpectreK,0);
  if (LambdaK!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_Spectrum ","LambdaK",LambdaK,0);
  if (Sref!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_Spectrum ","Sref",Sref,0);
  if (Trend!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_Spectrum ","Trend",Trend,0);
  if (offset!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_Spectrum ","offset",offset,0);

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ANALYSE_Spectrum",rc);
  #endif

//  TO SIMULATE ERROR ON SPECTRA
//  if ((pEngineContext->indexRecord%2)==0)
//   rc=ERROR_SetLast("ShiftVector",ERROR_TYPE_WARNING,ERROR_ID_LOG,analyseIndexRecord);

  return rc;
 }

// ===============
// DATA PROCESSING
// ===============

// --------------------------------------------------------------------------
// ANALYSE_ResetData : Release and reset all data used for a project analysis
// --------------------------------------------------------------------------

void ANALYSE_ResetData(void)
 {
  // Declarations

  CROSS_REFERENCE *pTabCross;
  FENO *pTabFeno;
  INDEX indexWorkSpace,indexFeno,indexTabCross,indexParam;

  // Filter

  if (ANALYSE_plFilter->filterFunction!=NULL)
   {
    MEMORY_ReleaseDVector("ANALYSE_ResetData ","FILTER_function",ANALYSE_plFilter->filterFunction,1);
    ANALYSE_plFilter->filterFunction=NULL;
   }

  if (ANALYSE_phFilter->filterFunction!=NULL)
   {
    MEMORY_ReleaseDVector("ANALYSE_ResetData ","FILTER_function",ANALYSE_phFilter->filterFunction,1);
    ANALYSE_phFilter->filterFunction=NULL;
   }

  // List of all symbols in a project

  for (indexWorkSpace=0;indexWorkSpace<NWorkSpace;indexWorkSpace++)
   MATRIX_Free(&WorkSpace[indexWorkSpace].xs,"ANALYSE_ResetData");

  memset(WorkSpace,0,sizeof(WRK_SYMBOL)*MAX_SYMB);
  NWorkSpace=0;

  // List of analysis windows in a project

  for (indexFeno=0;indexFeno<MAX_FENO;indexFeno++)
   {
    pTabFeno=&TabFeno[indexFeno];

    // Reference spectra and wavelength scale

    if (pTabFeno->SrefEtalon!=NULL)
     MEMORY_ReleaseDVector("ANALYSE_ResetData ","SrefEtalon",pTabFeno->SrefEtalon,0);
    if (pTabFeno->Sref!=NULL)
     MEMORY_ReleaseDVector("ANALYSE_ResetData ","Sref",pTabFeno->Sref,0);
    if (pTabFeno->SrefSigma!=NULL)
     MEMORY_ReleaseDVector("ANALYSE_ResetData ","SrefSigma",pTabFeno->SrefSigma,0);
    if (pTabFeno->SrefN!=NULL)
     MEMORY_ReleaseDVector("ANALYSE_ResetData ","SrefN",pTabFeno->SrefN,0);
    if (pTabFeno->SrefS!=NULL)
     MEMORY_ReleaseDVector("ANALYSE_ResetData ","SrefS",pTabFeno->SrefS,0);
    if (pTabFeno->LambdaN!=NULL)
     MEMORY_ReleaseDVector("ANALYSE_ResetData ","LambdaN",pTabFeno->LambdaN,0);
    if (pTabFeno->LambdaS!=NULL)
     MEMORY_ReleaseDVector("ANALYSE_ResetData ","LambdaS",pTabFeno->LambdaS,0);
    if (pTabFeno->Lambda!=NULL)
     MEMORY_ReleaseDVector("ANALYSE_ResetData ","Lambda",pTabFeno->Lambda,0);
    if (pTabFeno->LambdaK!=NULL)
     MEMORY_ReleaseDVector("ANALYSE_ResetData ","LambdaK",pTabFeno->LambdaK,0);
    if (pTabFeno->LambdaRef!=NULL)
     MEMORY_ReleaseDVector("ANALYSE_ResetData ","LambdaRef",pTabFeno->LambdaRef,0);
    if (pTabFeno->satelliteRef!=NULL)
     MEMORY_ReleaseBuffer("ANALYSE_ResetData ","satelliteRef",pTabFeno->satelliteRef);

    // SVD matrices

    SVD_Free("ANALYSE_ResetData",&pTabFeno->svd);

    // Coefficients for building polynomial fitting fwhm

    for (indexParam=0;(indexParam<MAX_KURUCZ_FWHM_PARAM);indexParam++)
     {
      if (pTabFeno->fwhmPolyRef[indexParam]!=NULL)
       MEMORY_ReleaseDVector("ANALYSE_ResetData ","fwhmPolyRef",pTabFeno->fwhmPolyRef[indexParam],0);
      if (pTabFeno->fwhmVector[indexParam]!=NULL)
       MEMORY_ReleaseDVector("ANALYSE_ResetData ","fwhmVector",pTabFeno->fwhmVector[indexParam],0);
      if (pTabFeno->fwhmDeriv2[indexParam]!=NULL)
       MEMORY_ReleaseDVector("ANALYSE_ResetData ","fwhmDeriv2",pTabFeno->fwhmDeriv2[indexParam],0);
     }

    // Cross sections and derivatives

    for (indexTabCross=0;indexTabCross<MAX_FIT;indexTabCross++)
     {
      pTabCross=&pTabFeno->TabCross[indexTabCross];

      if (pTabCross->vector!=NULL)
       MEMORY_ReleaseDVector("ANALYSE_ResetData ","vector",pTabCross->vector,0);
      if (pTabCross->Deriv2!=NULL)
       MEMORY_ReleaseDVector("ANALYSE_ResetData ","Deriv2",pTabCross->Deriv2,1);
     }

    memset(pTabFeno,0,sizeof(FENO));

    pTabFeno->Shift=pTabFeno->ShiftN=pTabFeno->ShiftS=
    pTabFeno->Stretch=pTabFeno->StretchN=pTabFeno->StretchS=
    pTabFeno->Stretch2=pTabFeno->Stretch2N=pTabFeno->Stretch2S=(double) 0.;
    pTabFeno->refNormFact=pTabFeno->refNormFactN=pTabFeno->refNormFactS=(double)1.;

    pTabFeno->refMaxdoasSelectionMode=ANLYS_MAXDOAS_REF_SZA;

    pTabFeno->indexSpectrum=
    pTabFeno->indexReference=
    pTabFeno->indexFwhmParam[0]=
    pTabFeno->indexFwhmParam[1]=
    pTabFeno->indexFwhmParam[2]=
    pTabFeno->indexFwhmParam[3]=
    pTabFeno->indexFwhmConst=
    pTabFeno->indexFwhmOrder1=
    pTabFeno->indexFwhmOrder2=
    pTabFeno->indexSol=
    pTabFeno->indexOffsetConst=
    pTabFeno->indexOffsetOrder1=
    pTabFeno->indexOffsetOrder2=
    pTabFeno->indexCommonResidual=
    pTabFeno->indexUsamp1=
    pTabFeno->indexUsamp2=
    pTabFeno->indexRing1=ITEM_NONE;

    pTabFeno->indexRefMorning=
    pTabFeno->indexRefAfternoon=
    pTabFeno->indexRef=ITEM_NONE;

    pTabFeno->ZmRefMorning=
    pTabFeno->ZmRefAfternoon=
    pTabFeno->Zm=
    pTabFeno->TDet=
    pTabFeno->Tm=
    pTabFeno->TimeDec=(double)9999.;

    memset(&pTabFeno->refDate,0,sizeof(SHORT_DATE));

    // Cross reference

    for (indexTabCross=0;indexTabCross<MAX_FIT;indexTabCross++)
     {
      pTabCross=&pTabFeno->TabCross[indexTabCross];
   // -------------------------------------------
      pTabCross->IndOrthog=
      pTabCross->FitConc=
      pTabCross->FitParam=
      pTabCross->FitShift=
      pTabCross->FitStretch=
      pTabCross->FitStretch2=
      pTabCross->FitScale=
      pTabCross->FitScale2=
      pTabCross->Comp=
      pTabCross->amfType=ITEM_NONE;
   // -------------------------------------------
      pTabCross->TypeStretch=
      pTabCross->TypeScale=(INT)0;
   // -------------------------------------------
      pTabCross->display=(DoasCh)0;
   // -------------------------------------------
      pTabCross->InitConc=
      pTabCross->InitParam=
      pTabCross->InitShift=
      pTabCross->InitStretch=
      pTabCross->InitStretch2=
      pTabCross->InitScale=
      pTabCross->InitScale2=
      pTabCross->DeltaConc=
      pTabCross->DeltaParam=
      pTabCross->DeltaShift=
      pTabCross->DeltaStretch=
      pTabCross->DeltaStretch2=
      pTabCross->DeltaScale=
      pTabCross->DeltaScale2=(double)0.;
   // -------------------------------------------
      pTabCross->MinParam=
      pTabCross->MinShift=(double)-99.;
      pTabCross->MaxParam=
      pTabCross->MaxShift=(double)99.;
   // -------------------------------------------
      pTabCross->Fact=(double)1.;
     }
   }

  // Kurucz buffers

  KURUCZ_Free();
  ANALYSE_UsampGlobalFree();

  // Output part

  OUTPUT_ResetData();

  NFeno=0;

  // Release global buffers

  if (Fitp!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","Fitp",Fitp,0);
  if (FitDeltap!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","FitDeltap",FitDeltap,0);
  if (FitMinp!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","FitMinp",FitMinp,0);
  if (FitMaxp!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","FitMaxp",FitMaxp,0);
  if (a!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","a",a,1);
  if (b!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","b",b,1);
  if (x!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","x",x,0);
  if (Sigma!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","Sigma",Sigma,0);
  if (ANALYSE_shift!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","ANALYSE_shift",ANALYSE_shift,0);
  if (ANALYSE_pixels!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","ANALYSE_pixels",ANALYSE_pixels,0);
  if (ANALYSE_splineX!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","ANALYSE_splineX",ANALYSE_splineX,0);
  if (ANALYSE_splineX2!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","ANALYSE_splineX2",ANALYSE_splineX2,0);
  if (ANALYSE_absolu!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","ANALYSE_absolu",ANALYSE_absolu,0);
  if (ANALYSE_t!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","ANALYSE_t",ANALYSE_t,0);
  if (ANALYSE_tc!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","ANALYSE_tc",ANALYSE_tc,0);
  if (ANALYSE_xsTrav!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","ANALYSE_xsTrav",ANALYSE_xsTrav,0);
  if (ANALYSE_xsTrav2!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","ANALYSE_xsTrav2",ANALYSE_xsTrav2,1);
  if (ANALYSE_secX!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","ANALYSE_secX",ANALYSE_secX,0);
  if (SplineSpec!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","SplineSpec",SplineSpec,0);
  if (SplineRef!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","SplineRef",SplineRef,0);
  if (ANALYSE_zeros!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","ANALYSE_zeros",ANALYSE_zeros,0);
  if (ANALYSE_ones!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_ResetData ","ANALYSE_ones",ANALYSE_ones,0);

  Fitp=
  FitDeltap=
  FitMinp=
  FitMaxp=
  a=
  b=
  x=
  Sigma=
  ANALYSE_shift=
  ANALYSE_pixels=
  ANALYSE_splineX=
  ANALYSE_splineX2=
  ANALYSE_absolu=
  ANALYSE_secX=
  ANALYSE_t=
  ANALYSE_tc=
  ANALYSE_xsTrav=
  ANALYSE_xsTrav2=
  SplineSpec=
  SplineRef=
  ANALYSE_zeros=
  ANALYSE_ones=NULL;

  MATRIX_Free(&ANALYSIS_slit,"ANALYSE_ResetData (2)");
  MATRIX_Free(&ANALYSIS_slitK,"ANALYSE_ResetData (3)");
  MATRIX_Free(&ANALYSIS_broAmf,"ANALYSE_ResetData");
  MATRIX_Free(&O3TD,"ANALYSE_ResetData");
 }

// --------------------------------------------------------
// ANALYSE_LoadSlit : Load slit function for fwhm correction
// --------------------------------------------------------

RC ANALYSE_LoadSlit(PRJCT_SLIT *pSlit)
 {
  // Declarations

  INT   xsToConvolute,xsToConvoluteI0;
  SLIT *pSlitFunction;
  INDEX indexFeno;
  RC    rc;

  // Initializations

  xsToConvolute=xsToConvoluteI0=0;
  pSlitFunction=&pSlit->slitFunction;
  rc=ERROR_ID_NO;

  // Search for high resolution cross section to convolute real time

  for (indexFeno=0;indexFeno<NFeno;indexFeno++)
   if (!pKuruczOptions->fwhmFit ||
      (!TabFeno[indexFeno].hidden && (TabFeno[indexFeno].useKurucz==ANLYS_KURUCZ_NONE)))
    {
     xsToConvolute+=TabFeno[indexFeno].xsToConvolute+TabFeno[indexFeno].useUsamp;
     xsToConvoluteI0+=TabFeno[indexFeno].xsToConvoluteI0;
    }

  if (xsToConvolute || (pSlitOptions->fwhmCorrectionFlag))
   {
    // Slit type selection

    if ((pSlitFunction->slitType==SLIT_TYPE_FILE) ||
        (pSlitFunction->slitType==SLIT_TYPE_GAUSS_FILE) ||
        (pSlitFunction->slitType==SLIT_TYPE_INVPOLY_FILE) ||
        (pSlitFunction->slitType==SLIT_TYPE_ERF_FILE) ||
//        (pSlitFunction->slitType==SLIT_TYPE_VOIGT_FILE) ||
        (pSlitFunction->slitType==SLIT_TYPE_GAUSS_T_FILE) ||
        (pSlitFunction->slitType==SLIT_TYPE_ERF_T_FILE))
     {
      // Load file

      if (!strlen(pSlitFunction->slitFile))
       rc=ERROR_SetLast("ANALYSE_LoadSlit",ERROR_TYPE_FATAL,ERROR_ID_MSGBOX_FIELDEMPTY,"Slit File");
      else
        rc=MATRIX_Load(pSlitFunction->slitFile,&ANALYSIS_slit,1 /* line base */,1 /* column base */,0,0,
                      -9999.,9999.,
                     ((pSlitFunction->slitType==SLIT_TYPE_GAUSS_T_FILE) ||
                      (pSlitFunction->slitType==SLIT_TYPE_ERF_T_FILE))?0:1,0,"ANALYSE_LoadSlit ");
     }
   }

  if (!rc && xsToConvoluteI0)
   {
    if (!strlen(pSlit->kuruczFile))
     rc=ERROR_SetLast("ANALYSE_LoadSlit",ERROR_TYPE_FATAL,ERROR_ID_MSGBOX_FIELDEMPTY,"Slit Kurucz File");
    else
     rc=MATRIX_Load(pSlit->kuruczFile,&ANALYSIS_slitK,0,0,0,2,-9999.,9999.,1,0,"ANALYSE_LoadSlit ");
   }

  // Return

  return rc;
 }

RC ANALYSE_CheckLambda(WRK_SYMBOL *pWrkSymbol,double *lambda,DoasCh *callingFunction)
 {
  // Declarations

  DoasCh fileName[MAX_ITEM_TEXT_LEN+1];
  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  FILES_RebuildFileName(fileName,pWrkSymbol->crossFileName,1);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// AnalyseLoadCross : Load cross sections data from cross sections type tab page
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_LoadCross
// -----------------------------------------------------------------------------
// PURPOSE       Load data from the molecules pages
// -----------------------------------------------------------------------------

RC ANALYSE_LoadCross(ENGINE_CONTEXT *pEngineContext,ANALYSIS_CROSS *crossSectionList,INT nCross,INT hidden,double *lambda)
 {
 	// Declarations

 	CROSS_REFERENCE *pEngineCross;                                                // pointer of the current cross section in the engine list
 	ANALYSIS_CROSS *pCross;                                                       // pointer of the current cross section in the mediate list
 	FENO *pTabFeno;                                                               // pointer to the current analysis window
  DoasCh *pOrthoSymbol[MAX_FIT],                                                 // for each cross section in list, hold cross section to use for orthogonalization
        *symbolName;
  INDEX indexSymbol,indexSvd,                                                   // resp. indexes of item in list and of symbol
        firstTabCross,endTabCross,indexTabCross,i;                              // indexes for browsing list of cross sections symbols
  SZ_LEN fileLength,symbolLength;                                               // length in characters of file name and symbol name
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a general description of a symbol
 	RC rc;

 	// Debug

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("ANALYSE_LoadCross",DEBUG_FCTTYPE_CONFIG);
  #endif

 	// Initializations

  pWrkSymbol=NULL;
  pTabFeno=&TabFeno[NFeno];
  firstTabCross=pTabFeno->NTabCross;
 	rc=ERROR_ID_NO;

 	for (indexTabCross=0;
 	    (indexTabCross<nCross) && (pTabFeno->NTabCross<MAX_FIT) && !rc;
 	     indexTabCross++)
 	 {
 	 	pEngineCross=&pTabFeno->TabCross[pTabFeno->NTabCross];
 	 	pCross=&crossSectionList[indexTabCross];

    // Get cross section name from analysis properties dialog box

    symbolName=pCross->symbol;
    symbolLength=strlen(symbolName);
    fileLength=strlen(pCross->crossSectionFile);

    // Search for symbol in list

    for (indexSymbol=0;indexSymbol<NWorkSpace;indexSymbol++)
     {
      pWrkSymbol=&WorkSpace[indexSymbol];

      if ((pWrkSymbol->type==WRK_SYMBOL_CROSS) &&
          (strlen(pWrkSymbol->symbolName)==symbolLength) &&
          (strlen(pWrkSymbol->crossFileName)==fileLength) &&
          (strlen(pWrkSymbol->amfFileName)==strlen(pCross->amfFile)) &&
          !strcasecmp(pWrkSymbol->symbolName,symbolName) &&
          !strcasecmp(pWrkSymbol->crossFileName,pCross->crossSectionFile) &&
          !strcasecmp(pWrkSymbol->amfFileName,pCross->amfFile))

       break;
     }

    // Add a new cross section

    if ((indexSymbol==NWorkSpace) && (NWorkSpace<MAX_SYMB))
     {
      // Allocate a new symbol

      pWrkSymbol=&WorkSpace[indexSymbol];

      pWrkSymbol->type=WRK_SYMBOL_CROSS;
      strcpy(pWrkSymbol->symbolName,symbolName);
      strcpy(pWrkSymbol->crossFileName,pCross->crossSectionFile);
      strcpy(pWrkSymbol->amfFileName,pCross->amfFile);

      // Load cross section from file

      if (((strlen(pWrkSymbol->symbolName)==strlen("1/Ref")) && !strcasecmp(pWrkSymbol->symbolName,"1/Ref")) ||
          !(rc=MATRIX_Load(pCross->crossSectionFile,&pWrkSymbol->xs,0 /* line base */,0 /* column base */,0,0,
                          (pCross->crossType==ANLYS_CROSS_ACTION_NOTHING)?(double)0.:lambda[0]-7.,      // max(lambda[0]-7.,(double)290.), - changed on october 2006
                          (pCross->crossType==ANLYS_CROSS_ACTION_NOTHING)?(double)0.:lambda[NDET-1]+7., // min(lambda[NDET-1]+7.,(double)600.), - changed on october 2006
                          (pCross->crossType!=ANLYS_CROSS_ACTION_NOTHING)?1:0,1,"ANALYSE_LoadCross ")))
       {
        if (!strcasecmp(pWrkSymbol->symbolName,"O3TD"))
         rc=MATRIX_Allocate(&O3TD,NDET,pWrkSymbol->xs.nc,0,0,0,"ANALYSE_LoadCross");

        NWorkSpace++;
       }
     }

    if ((rc==ERROR_ID_NO) && (indexSymbol<NWorkSpace) && (pTabFeno->NTabCross<MAX_FIT))
     {
      // Allocate vectors for cross section and its second derivative for analysis processing

      if (((pEngineCross->vector=(double *)MEMORY_AllocDVector("ANALYSE_LoadCross ","vector",0,NDET-1))==NULL) ||
          ((pEngineCross->Deriv2=(double *)MEMORY_AllocDVector("ANALYSE_LoadCross ","Deriv2",1,NDET))==NULL))

       rc=ERROR_ID_ALLOC;

      else
       {
        pEngineCross->crossAction=pCross->crossType;
        pEngineCross->amfType=pCross->amfType;
        pEngineCross->filterFlag=pCross->requireFilter;

        if ((pEngineCross->crossAction==ANLYS_CROSS_ACTION_NOTHING) && (pTabFeno->gomeRefFlag || pEngineContext->refFlag))
         rc=ANALYSE_CheckLambda(pWrkSymbol,lambda,"ANALYSE_LoadCross ");

        if (rc==ERROR_ID_NO)
         {
          pEngineCross->Comp=indexSymbol;
          pEngineCross->IndSvdA=++pTabFeno->svd.DimC;
          pTabFeno->xsToConvolute+=((pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE) ||
                                    (pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0) ||
                                    (pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_RING))?1:0;

          pTabFeno->xsToConvoluteI0+=(pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0)?1:0;

          pOrthoSymbol[pTabFeno->NTabCross]=pCross->orthogonal;

          pEngineCross->display=pCross->requireFit;                    // fit display
          pEngineCross->InitConc=pCross->initialCc;                    // initial concentration
          pEngineCross->FitConc=pCross->requireCcFit;                  // modify concentration
          pEngineCross->FitFromPrevious=pCross->constrainedCc;

          pEngineCross->DeltaConc=(pEngineCross->FitConc)?pCross->deltaCc:(double)0.;   // delta on concentration
          pEngineCross->I0Conc=(pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0)?pCross->ccIo:(double)0.;

          // Swap columns of original matrix A in order to have in the end of the matrix, cross sections with fixed concentrations

          if (pEngineCross->FitConc!=0)   // the difference between SVD and Marquardt+SVD hasn't to be done yet but later
           {
            for (i=pTabFeno->NTabCross-1;i>=0;i--)
             if (((indexSvd=pTabFeno->TabCross[i].IndSvdA)!=0) && !pTabFeno->TabCross[i].FitConc)
              {
               pTabFeno->TabCross[i].IndSvdA=pEngineCross->IndSvdA;
               pEngineCross->IndSvdA=indexSvd;
              }

            if (pTabFeno->analysisMethod!=PRJCT_ANLYS_METHOD_SVD)     // In the intensity fitting method, FitConc is an index
             pEngineCross->FitConc=pTabFeno->svd.NF++;                   // in the non linear parameters vectors

            pTabFeno->svd.nFit++;
           }
          else if (pTabFeno->analysisMethod!=PRJCT_ANLYS_METHOD_SVD)
           pEngineCross->FitConc=ITEM_NONE;                              // so if the parameter hasn't to be fitted, index is ITEM_NONE

          pTabFeno->NTabCross++;
         }
       }
     }
 	 }

  // Orthogonalization data

  if (rc==ERROR_ID_NO)
   {
    pEngineCross=pTabFeno->TabCross;

    for (indexTabCross=firstTabCross,endTabCross=pTabFeno->NTabCross;indexTabCross<endTabCross;indexTabCross++)
     {
      symbolLength=strlen(pOrthoSymbol[indexTabCross]);

      // No orthogonalization

      if ((symbolLength==4) && !strcasecmp(pOrthoSymbol[indexTabCross],"None"))
       pEngineCross[indexTabCross].IndOrthog=ITEM_NONE;

      // Orthogonalization to orthogonal base

      else if ((symbolLength==15) && !strcasecmp(pOrthoSymbol[indexTabCross],"Differential XS"))
       pEngineCross[indexTabCross].IndOrthog=ORTHOGONAL_BASE;

      // Orthogonalization to another cross section

      else
       {
        // Search for symbol in list

        for (indexSymbol=firstTabCross;indexSymbol<endTabCross;indexSymbol++)
         if ((indexTabCross!=indexSymbol) &&
             (symbolLength==strlen(WorkSpace[pEngineCross[indexSymbol].Comp].symbolName)) &&
             !strcasecmp(pOrthoSymbol[indexTabCross],WorkSpace[pEngineCross[indexSymbol].Comp].symbolName))
          break;

        pEngineCross[indexTabCross].IndOrthog=(indexSymbol<endTabCross)?indexSymbol:ITEM_NONE;
       }
     }

    for (indexTabCross=firstTabCross,endTabCross=pTabFeno->NTabCross;indexTabCross<endTabCross;indexTabCross++)
     {
      // Symbol should be set to be orthogonalized to base

      if (pEngineCross[indexTabCross].IndOrthog>=0)
       {
        // if orthogonalization in succession, orthogonalization is ignored

/* !!!!!        if (pEngineCross[pEngineCross[indexTabCross].IndOrthog].IndOrthog>=0)  // != ORTHOGONAL_BASE
         {
          THRD_Error(ERROR_TYPE_WARNING,ERROR_ID_ORTHOGONAL_CASCADE,"",WorkSpace[pEngineCross[indexTabCross].Comp].symbolName);
          pEngineCross[indexTabCross].IndOrthog=ITEM_NONE;
         }

        // Force to be orthogonalized to base

        else  */
         {
          if (pEngineCross[pEngineCross[indexTabCross].IndOrthog].IndOrthog==ITEM_NONE)
           {
           	rc=ERROR_SetLast("ANALYSE_LoadCross",ERROR_TYPE_WARNING,ERROR_ID_ORTHOGONAL_BASE,
                              WorkSpace[pEngineCross[pEngineCross[indexTabCross].IndOrthog].Comp].symbolName,
                              WorkSpace[pEngineCross[indexTabCross].Comp].symbolName);

            pEngineCross[pEngineCross[indexTabCross].IndOrthog].IndOrthog=ORTHOGONAL_BASE;
           }
         }
       }
     }
   }

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionStop("ANALYSE_LoadCross",rc);
  #endif

  // Return

  return rc;
 }

// -------------------------------------------------
// ANALYSE_LoadLinear : Load continuous functions
// -------------------------------------------------

RC ANALYSE_LoadLinear(ANALYSE_LINEAR_PARAMETERS *linearList,INT nLinear)
 {
  // Declarations

  INDEX indexItem,indexSymbol,indexOrder;                                       // indexes for loops and arrays
  CROSS_REFERENCE *pTabCross;                                                   // pointer to an element of the symbol cross reference table of an analysis window
  CROSS_RESULTS *pResults;                                                      // pointer to results
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a general description of a symbol
  DoasCh buttonText[10];                                                         // term in polynomial
  ANALYSE_LINEAR_PARAMETERS *pList;                                             // pointer to description of an item in list
  FENO *pTabFeno;                                                               // pointer to description of the current analysis window
  INDEX indexSvd,indexTabCross;                                                 // extra index for swapping
  INT polyFlag;                                                                 // polynomial flag (-1 for invpoly, 0 for offset, 1 for poly)
  INT polyOrder,baseOrder;                                                      // polynomial order, base order
  RC rc;                                                                        // return code

  // Initializations

  pTabFeno=&TabFeno[NFeno];
  rc=ERROR_ID_NO;

  // Browse lines

  for (indexItem=0;(indexItem<nLinear) && !rc;indexItem++)
   {
    pList=&linearList[indexItem];

    if (!strcasecmp(pList->symbolName,"Polynomial (x)"))
     polyFlag=1;
    else if (!strcasecmp(pList->symbolName,"Polynomial (1/x)"))
     polyFlag=-1;
    else
     polyFlag=0;

    polyOrder=pList->polyOrder;
    baseOrder=pList->baseOrder;

    if (!polyFlag && (polyOrder>=0))
     pTabFeno->offlFlag++;

    for (indexOrder=0;indexOrder<=polyOrder;indexOrder++)
     {
      // Set symbol name

      if (polyFlag!=0)
       sprintf(buttonText,(polyFlag==1)?"x%d":"1/x%d",indexOrder);
      else
       sprintf(buttonText,"offl%d",indexOrder);

      // Search for symbol in list

      for (indexSymbol=0;indexSymbol<NWorkSpace;indexSymbol++)
       {
        pWrkSymbol=&WorkSpace[indexSymbol];

        if ((pWrkSymbol->type==WRK_SYMBOL_CONTINUOUS) &&
            !strcasecmp(pWrkSymbol->symbolName,buttonText))

         break;
       }

      if ((indexSymbol==NWorkSpace) && (NWorkSpace<MAX_SYMB))
       {
        // Allocate a new symbol (but do not allocate a vector !)

        pWrkSymbol=&WorkSpace[indexSymbol];

        pWrkSymbol->type=WRK_SYMBOL_CONTINUOUS;
        strcpy(pWrkSymbol->symbolName,buttonText);

        NWorkSpace++;
       }

      if ((indexSymbol<NWorkSpace) && (pTabFeno->NTabCross<MAX_FIT))
       {
        pTabCross=&pTabFeno->TabCross[pTabFeno->NTabCross];
        pResults=&pTabFeno->TabCrossResults[pTabFeno->NTabCross];

        if ((pTabCross->vector=(double *)MEMORY_AllocDVector("ANALYSE_LoadLinear ","vector",0,NDET-1))==NULL)
         rc=ERROR_ID_ALLOC;

        else
         {
          memcpy(pTabCross->vector,ANALYSE_ones,sizeof(double)*NDET);

          if (polyFlag && (baseOrder>=indexOrder))
           {
            pTabFeno->OrthoSet[pTabFeno->NOrtho++]=pTabFeno->NTabCross;
            pTabCross->IndOrthog=ITEM_NONE;
           }
          else if (polyFlag)
           pTabCross->IndOrthog=ORTHOGONAL_BASE;
          else
           pTabCross->IndOrthog=ITEM_NONE;

          pTabCross->Comp=indexSymbol;
          pTabCross->IndSvdA=++pTabFeno->svd.DimC;
          pTabCross->crossAction=ANLYS_CROSS_ACTION_NOTHING;

          pTabFeno->NTabCross++;

          // Swap columns of original matrix A in order to have in the end of the matrix, cross sections with fixed concentrations

          for (indexTabCross=pTabFeno->NTabCross-2;indexTabCross>=0;indexTabCross--)
           if (((indexSvd=pTabFeno->TabCross[indexTabCross].IndSvdA)!=0) &&
                (pTabFeno->TabCross[indexTabCross].FitConc==0) &&
                (pTabFeno->TabCross[indexTabCross].DeltaConc==(double)0.))
            {
             pTabFeno->TabCross[indexTabCross].IndSvdA=pTabCross->IndSvdA;
             pTabCross->IndSvdA=indexSvd;
            }

          if (pTabFeno->analysisMethod==PRJCT_ANLYS_METHOD_SVDMARQUARDT)        // Marquardt-Levenberg + SVD
           pTabCross->IndSvdP=++(pTabFeno->svd.DimP);

          pTabFeno->svd.nFit++;

          // Results

          pResults->StoreSlntCol=pList->storeFit;                               // flag set if slant column is to be written into output file
          pResults->StoreSlntErr=pList->storeError;                             // flag set if error on slant column is to be written into output file
         }
     	 }
     }
   }

  // Return

  return rc;
 }

// ------------------------------------------------------------------------------------------------
// ANALYSE_LoadShiftStretch : Load shift and stretch for cross sections implied in SVD decomposition
// ------------------------------------------------------------------------------------------------

RC ANALYSE_LoadShiftStretch(ANALYSIS_SHIFT_STRETCH *shiftStretchList,INT nShiftStretch)
 {
  // Declarations

  INDEX indexItem,indexSymbol,indexTabCross,indexSymbolInList,indexCross,       // indexes for loops and arrays
        indexShift,indexStretch,indexStretch2,indexScale,indexScale2;
  DoasCh *symbol;                                                                // copy of list of symbols
  CROSS_REFERENCE *pTabCross;                                                   // pointer to an element of the symbol cross reference table of an analysis window
  CROSS_RESULTS *pResults;                                                      // pointer to results part relative to the symbol
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a general description of a symbol
  SZ_LEN symbolLength;                                                          // length in characters of a symbol name
  ANALYSIS_SHIFT_STRETCH *pList;                                                // pointer to description of an item in list
  FENO *pTabFeno;                                                               // pointer to description of the current analysis window
  INT oldNF;
  RC rc;                                                                        // return code

  // Initializations

  pTabFeno=&TabFeno[NFeno];
  indexShift=indexStretch=indexStretch2=indexScale=indexScale2=oldNF=pTabFeno->svd.NF;
  rc=ERROR_ID_NO;

  for (indexItem=0;(indexItem<nShiftStretch) && !rc;indexItem++)
   {
    pList=&shiftStretchList[indexItem];

    for (indexSymbolInList=0;indexSymbolInList<pList->nSymbol;indexSymbolInList++)
     {
     	symbol=pList->symbol[indexSymbolInList];
     	symbolLength=strlen(symbol);

     	// Search for the symbols in the cross sections list

     	for (indexCross=0;indexCross<pTabFeno->NTabCross;indexCross++)
     	 {
     	 	pWrkSymbol=&WorkSpace[pTabFeno->TabCross[indexCross].Comp];

        if ((strlen(pWrkSymbol->symbolName)==symbolLength) &&
            !strcasecmp(pWrkSymbol->symbolName,symbol))

         break;
        }

      if (indexCross<pTabFeno->NTabCross)
       indexSymbol=pTabFeno->TabCross[indexCross].Comp;
      else
       {
        for (indexSymbol=0;indexSymbol<NWorkSpace;indexSymbol++)
         {
          pWrkSymbol=&WorkSpace[indexSymbol];

          if ((strlen(pWrkSymbol->symbolName)==symbolLength) &&
              !strcasecmp(pWrkSymbol->symbolName,symbol))

           break;
         }
       }

      // Allocate a new symbol

      if ((indexSymbol==NWorkSpace) && (NWorkSpace<MAX_SYMB))
       {
        pWrkSymbol=&WorkSpace[indexSymbol];

        pWrkSymbol->type=WRK_SYMBOL_SPECTRUM;
        strcpy(pWrkSymbol->symbolName,symbol);

        NWorkSpace++;
       }

      if (indexSymbol<NWorkSpace)
       {
        // Search for symbol in symbol cross reference

        for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
         if (pTabFeno->TabCross[indexTabCross].Comp==indexSymbol)
          break;

        // Add symbol into symbol cross reference

        if ((indexTabCross==pTabFeno->NTabCross) && (pTabFeno->NTabCross<MAX_FIT))
         {
          if ((symbolLength==strlen(SYMB_itemCrossList[SYMBOL_PREDEFINED_SPECTRUM].name)) && !strcasecmp(symbol,SYMB_itemCrossList[SYMBOL_PREDEFINED_SPECTRUM].name))
           pTabFeno->indexSpectrum=pTabFeno->NTabCross;
          else if ((symbolLength==strlen(SYMB_itemCrossList[SYMBOL_PREDEFINED_REF].name)) && !strcasecmp(symbol,SYMB_itemCrossList[SYMBOL_PREDEFINED_REF].name))
           pTabFeno->indexReference=pTabFeno->NTabCross;

          pTabFeno->TabCross[indexTabCross].Comp=indexSymbol;
          pTabFeno->NTabCross++;
         }

        // Load shift and stretch parameters into symbol cross reference

        if (indexTabCross<pTabFeno->NTabCross)
         {
          pTabCross=&pTabFeno->TabCross[indexTabCross];
          pResults=&pTabFeno->TabCrossResults[indexTabCross];

          // Shift

          pTabCross->InitShift=pList->shInit;                                   // initial value for shift

          if (pList->shFit!=0)
           indexShift=pTabCross->FitShift=(!indexSymbolInList)?                 // flag set when shift is to be fit
            pTabFeno->svd.NF++:indexShift;

          pTabCross->DeltaShift=(pTabCross->FitShift!=ITEM_NONE)?
           pList->shDelta:(double)0.;                                           // delta value for shift

          pTabCross->MinShift=pList->shMin;                                     // minimum value for shift
          pTabCross->MaxShift=pList->shMax;                                     // maximum value for shift

          // Stretch

          pTabCross->InitStretch=pList->stInit;                                 // initial value for stretch order 1
          pTabCross->InitStretch2=pList->stInit2;                               // initial value for stretch order 2

          pTabCross->TypeStretch=pList->stFit;                                  // type of stretch method

          if (pTabCross->TypeStretch!=ANLYS_STRETCH_TYPE_NONE)
           indexStretch=pTabCross->FitStretch=(!indexSymbolInList)?          // flag set when stretch is to be fit
            pTabFeno->svd.NF++:indexStretch;

          if (pTabCross->TypeStretch==ANLYS_STRETCH_TYPE_SECOND_ORDER)
           indexStretch2=pTabCross->FitStretch2=(!indexSymbolInList)?        // flag set when stretch is to be fit
            pTabFeno->svd.NF++:indexStretch2;

          pTabCross->DeltaStretch=(pTabCross->FitStretch!=ITEM_NONE)?        // delta value for stretch order 1
           pList->stDelta:(double)0.;
          pTabCross->DeltaStretch2=(pTabCross->FitStretch2!=ITEM_NONE)?      // delta value for stretch order 2
           pList->stDelta2:(double)0.;

          // Scaling

          pTabCross->InitScale=pList->scInit;                                   // initial value for scale order 1
          pTabCross->InitScale2=pList->scInit2;                                 // initial value for scale order 2
          pTabCross->TypeScale=pList->scFit;                                    // type of scale method

          if (pTabCross->TypeScale!=ANLYS_STRETCH_TYPE_NONE)
           indexScale=pTabCross->FitScale=(!indexSymbolInList)?                 // flag set when scale is to be fit
            pTabFeno->svd.NF++:indexScale;

          if (pTabCross->TypeScale==ANLYS_STRETCH_TYPE_SECOND_ORDER)
           indexScale2=pTabCross->FitScale2=(!indexSymbolInList)?               // flag set when scale is to be fit
            pTabFeno->svd.NF++:indexScale2;

          pTabCross->DeltaScale=(pTabCross->FitScale!=ITEM_NONE)?               // delta value for scale order 1
           pList->scDelta:(double)0.;
          pTabCross->DeltaScale2=(pTabCross->FitScale2!=ITEM_NONE)?             // delta value for scale order 2
           pList->scDelta2:(double)0.;

          if (pTabCross->IndSvdA)
           {
            if (pTabCross->FitShift!=ITEM_NONE)
             pTabFeno->svd.NP++;
            if (pTabCross->FitStretch!=ITEM_NONE)
             pTabFeno->svd.NP++;
            if (pTabCross->FitStretch2!=ITEM_NONE)
             pTabFeno->svd.NP++;
            if (pTabCross->FitScale!=ITEM_NONE)
             pTabFeno->svd.NP++;
            if (pTabCross->FitScale2!=ITEM_NONE)
             pTabFeno->svd.NP++;
           }

          pResults->StoreShift=((!indexSymbolInList) && (pList->shStore==1))?(DoasCh)1:(DoasCh)0;                  // flag set if shift is to be written into output file
          pResults->StoreStretch=((!indexSymbolInList) && (pList->stStore==1))?(DoasCh)1:(DoasCh)0;                // flag set if stretch is to be written into output file
          pResults->StoreScale=((!indexSymbolInList) && (pList->scStore==1))?(DoasCh)1:(DoasCh)0;                  // flag set if scaling is to be written into output file
          pResults->StoreError=((!indexSymbolInList) && (pList->errStore==1))?(DoasCh)1:(DoasCh)0;                 // flag set if errors on linear parameters are to be written into output file
         }
       }
     }
   }

  // Return

  pTabFeno->svd.nFit+=(pTabFeno->svd.NF-oldNF);

  return rc;
 }

// --------------------------------------------------
// AnalyseLoadPredefined : Load predefined parameters
// --------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC ANALYSE_LoadNonLinear(ENGINE_CONTEXT *pEngineContext,ANALYSE_NON_LINEAR_PARAMETERS *nonLinearList,INT nNonLinear,double *lambda)
 {
  // Declarations

  INDEX indexItem,indexSymbol,indexTabCross,indexSvd;                           // indexes for loops and arrays
  DoasCh *symbol;                                                                // browse symbols
  CROSS_REFERENCE *pTabCross;                                                   // pointer to an element of the symbol cross reference table of an analysis window
  CROSS_RESULTS *pResults;                                                      // pointer to results part relative to the symbol
  ANALYSE_NON_LINEAR_PARAMETERS *pListItem;                                     // pointer to the current item in the non linear parameters list
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a general description of a symbol
  SZ_LEN symbolLength,fileLength;                                               // length in characters of a symbol name
  FENO *pTabFeno;                                                               // pointer to description of the current analysis window
  RC rc,rcTmp;                                                                  // return code

  // Initializations

  pWrkSymbol=NULL;
  pTabFeno=&TabFeno[NFeno];
  rc=ERROR_ID_NO;

  for (indexItem=0;(indexItem<nNonLinear) && !rc;indexItem++)
   {
   	pListItem=&nonLinearList[indexItem];

   	if (pListItem->fitFlag || (pListItem->initialValue!=(double)0.))
   	 {
      if ((pTabFeno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) || strnicmp(pListItem->symbolName,"offset",6))
       {
        symbol=pListItem->symbolName;
        symbolLength=strlen(symbol);
        fileLength=strlen(pListItem->crossFileName);
        rcTmp=0;

        // Search for symbol in list

        for (indexSymbol=0;indexSymbol<NWorkSpace;indexSymbol++)
         {
          pWrkSymbol=&WorkSpace[indexSymbol];

          if ((pWrkSymbol->type==WRK_SYMBOL_PREDEFINED) &&
              (strlen(pWrkSymbol->symbolName)==symbolLength) &&
              (strlen(pWrkSymbol->crossFileName)==fileLength) &&
              !strcasecmp(pWrkSymbol->symbolName,pListItem->symbolName) &&
              !strcasecmp(pWrkSymbol->crossFileName,pListItem->crossFileName))
           break;
         }

        // Allocate a new symbol

        if ((indexSymbol==NWorkSpace) && (NWorkSpace<MAX_SYMB))
         {
          pWrkSymbol=&WorkSpace[indexSymbol];

          pWrkSymbol->type=WRK_SYMBOL_PREDEFINED;
          strcpy(pWrkSymbol->symbolName,symbol);
          strcpy(pWrkSymbol->crossFileName,pListItem->crossFileName);

          // Load cross section from file

           NWorkSpace++;
         }

        if ((indexSymbol<NWorkSpace) && ((indexTabCross=pTabFeno->NTabCross)<MAX_FIT) &&
           ((strcasecmp(symbol,"SFP 1") && strcasecmp(symbol,"SFP 2") &&
             strcasecmp(symbol,"SFP 3") && strcasecmp(symbol,"SFP 4")) ||
            (pKuruczOptions->fwhmFit &&
           ((pKuruczOptions->fwhmType==SLIT_TYPE_ERF) || (pKuruczOptions->fwhmType==SLIT_TYPE_VOIGT) || strcasecmp(symbol,"SFP 2")) &&
           ((pKuruczOptions->fwhmType==SLIT_TYPE_VOIGT) || (strcasecmp(symbol,"SFP 3") && strcasecmp(symbol,"SFP 4"))))))
         {
          // Add symbol into symbol cross reference

          if ((symbolLength==strlen("SFP 1")) && !strcasecmp(symbol,"SFP 1"))
           pTabFeno->indexFwhmParam[0]=pTabFeno->NTabCross;
          else if ((symbolLength==strlen("SFP 2")) && !strcasecmp(symbol,"SFP 2"))
           pTabFeno->indexFwhmParam[1]=pTabFeno->NTabCross;
          else if ((symbolLength==strlen("SFP 3")) && !strcasecmp(symbol,"SFP 3"))
           pTabFeno->indexFwhmParam[2]=pTabFeno->NTabCross;
          else if ((symbolLength==strlen("SFP 4")) && !strcasecmp(symbol,"SFP 4"))
           pTabFeno->indexFwhmParam[3]=pTabFeno->NTabCross;
//          else if ((symbolLength==strlen("Fwhm (Constant)")) && !strcasecmp(symbol,"Fwhm (Constant)"))
//           pTabFeno->indexFwhmConst=pTabFeno->NTabCross;
//          else if ((symbolLength==strlen("Fwhm (Order 1)")) && !strcasecmp(symbol,"Fwhm (Order 1)"))
//           pTabFeno->indexFwhmOrder1=pTabFeno->NTabCross;
//          else if ((symbolLength==strlen("Fwhm (order 2)")) && !strcasecmp(symbol,"Fwhm (Order 2)"))
//           pTabFeno->indexFwhmOrder2=pTabFeno->NTabCross;
          else if ((symbolLength==strlen("Sol")) && !strcasecmp(symbol,"Sol"))
           pTabFeno->indexSol=pTabFeno->NTabCross;
          else if ((symbolLength==strlen("Offset (Constant)")) && !strcasecmp(symbol,"Offset (Constant)"))
           pTabFeno->indexOffsetConst=pTabFeno->NTabCross;
          else if ((symbolLength==strlen("Offset (Order 1)")) && !strcasecmp(symbol,"Offset (Order 1)"))
           pTabFeno->indexOffsetOrder1=pTabFeno->NTabCross;
          else if ((symbolLength==strlen("Offset (Order 2)")) && !strcasecmp(symbol,"Offset (Order 2)"))
           pTabFeno->indexOffsetOrder2=pTabFeno->NTabCross;
          else if ((symbolLength==strlen(SYMB_itemCrossList[SYMBOL_PREDEFINED_COM].name)) &&
                   !strcasecmp(symbol,SYMB_itemCrossList[SYMBOL_PREDEFINED_COM].name) &&
                   !pTabFeno->hidden)

           pTabFeno->indexCommonResidual=pTabFeno->NTabCross;

          else if ((symbolLength==strlen(SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP1].name)) &&
                   !strcasecmp(symbol,SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP1].name) &&
                   !pTabFeno->hidden)

           pTabFeno->indexUsamp1=pTabFeno->NTabCross;

          else if ((symbolLength==strlen(SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP2].name)) &&
                   !strcasecmp(symbol,SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP2].name) &&
                   !pTabFeno->hidden)

           pTabFeno->indexUsamp2=pTabFeno->NTabCross;

          else if ((symbolLength==strlen(SYMB_itemCrossList[SYMBOL_PREDEFINED_RING1].name)) &&
                   !strcasecmp(symbol,SYMB_itemCrossList[SYMBOL_PREDEFINED_RING1].name) &&
                   !pTabFeno->hidden)
           pTabFeno->indexRing1=pTabFeno->NTabCross;
          else
           rcTmp=1;

          if (!rcTmp)
           {
            pTabFeno->TabCross[indexTabCross].Comp=indexSymbol;

            // Load parameters into cross reference structures

            pTabCross=&pTabFeno->TabCross[indexTabCross];
            pResults=&pTabFeno->TabCrossResults[indexTabCross];
            pTabCross->InitParam=pListItem->initialValue;

            // DOAS -> the parameters are fitted non linearly (except undersampling, see further NF--)
            // Marquardt-Levenberg method -> the parameters are fitted linearly

            if ((pTabFeno->analysisMethod==PRJCT_ANLYS_METHOD_SVDMARQUARDT) &&
               ((pTabFeno->indexOffsetConst==pTabFeno->NTabCross) ||
                (pTabFeno->indexOffsetOrder1==pTabFeno->NTabCross) ||
                (pTabFeno->indexOffsetOrder2==pTabFeno->NTabCross) ||
                (pTabFeno->indexCommonResidual==pTabFeno->NTabCross) ||
                (pTabFeno->indexUsamp1==pTabFeno->NTabCross) ||
                (pTabFeno->indexUsamp2==pTabFeno->NTabCross) ||
                (pTabFeno->indexRing1==pTabFeno->NTabCross)) &&
              (((pTabCross->FitParam=((pListItem->fitFlag)?1:0))!=ITEM_NONE) ||
                (pTabCross->InitParam!=(double)0.)))
             {
              pTabCross->IndSvdA=++pTabFeno->svd.DimC;
              pTabCross->IndSvdP=++pTabFeno->svd.DimP;
             }
            else
             pTabCross->FitParam=(pListItem->fitFlag)?pTabFeno->svd.NF++:ITEM_NONE;

            if (pTabCross->FitParam!=ITEM_NONE)  // Increase the number of parameters to fit
             pTabFeno->svd.nFit++;               //    -> this information is useful to calculate the number of degrees of freedom

            pTabCross->MinParam=pListItem->minValue;
            pTabCross->MaxParam=pListItem->maxValue;
            pTabCross->DeltaParam=(pTabCross->FitParam!=ITEM_NONE)?pListItem->deltaValue:(double)0.;

            if ((pTabCross->FitParam==ITEM_NONE) && (pTabCross->InitParam==(double)0.))
             {
              if (pTabFeno->indexCommonResidual==pTabFeno->NTabCross)
               pTabFeno->indexCommonResidual=ITEM_NONE;
              else if (pTabFeno->indexUsamp1==pTabFeno->NTabCross)
               pTabFeno->indexUsamp1=ITEM_NONE;
              else if (pTabFeno->indexUsamp2==pTabFeno->NTabCross)
               pTabFeno->indexUsamp2=ITEM_NONE;
              else if (pTabFeno->indexRing1==pTabFeno->NTabCross)
               pTabFeno->indexRing1=ITEM_NONE;
              else if (pTabFeno->indexFwhmParam[2]==pTabFeno->NTabCross)
               pTabFeno->indexFwhmParam[2]=pTabFeno->indexFwhmParam[3]=ITEM_NONE;
             }

            // Load the cross section file if any

            if (((pTabFeno->indexCommonResidual==pTabFeno->NTabCross) ||          // common residual
                 (pTabFeno->indexUsamp1==pTabFeno->NTabCross) ||                  // undersampling phase 1
                 (pTabFeno->indexUsamp2==pTabFeno->NTabCross) ||                  // undersampling phase 2
                 (pTabFeno->indexRing1==pTabFeno->NTabCross)) &&                  // Raman spectrum
                ((pTabCross->FitParam!=ITEM_NONE) || (pTabCross->InitParam!=(double)0.)))
             {
              if ((pTabFeno->indexCommonResidual==pTabFeno->NTabCross) ||
                  (pTabFeno->indexRing1==pTabFeno->NTabCross) ||
                  (pUsamp->method==PRJCT_USAMP_FILE))
               {
                if (((rc=MATRIX_Load(pListItem->crossFileName,&pWrkSymbol->xs,0 /* line base */,0 /* column base */,0,0,
                     (double)0.,(double)0.,
                    ((pTabFeno->indexRing1==pTabFeno->NTabCross))?1:0,0,"ANALYSE_LoadNonLinear "))!=0)

                 ||((pTabFeno->gomeRefFlag || pEngineContext->refFlag) &&
                    (pTabFeno->indexRing1!=pTabFeno->NTabCross) &&                               // only the Raman spectrum should be interpolated on the
                   ((rc=ANALYSE_CheckLambda(pWrkSymbol,lambda,"ANALYSE_LoadNonLinear "))!=0))    // grid of the reference spectrum
                 )

                 goto EndLoadPredefined;
               }
              else
               pTabFeno->useUsamp=1;

              if (((pTabCross->vector=(double *)MEMORY_AllocDVector("ANALYSE_LoadNonLinear ","vector",0,NDET-1))==NULL) ||
                 (((pTabFeno->indexRing1==pTabFeno->NTabCross)) &&
                  ((pTabCross->Deriv2=(double *)MEMORY_AllocDVector("ANALYSE_LoadNonLinear ","Deriv2",1,NDET))==NULL)))     // for Raman interpolation

               rc=ERROR_ID_ALLOC;
              else if (rc==ERROR_ID_NO)
               {
                if ((pTabFeno->indexCommonResidual==pTabFeno->NTabCross) ||
                    (pTabFeno->indexRing1==pTabFeno->NTabCross) ||
                    (pUsamp->method==PRJCT_USAMP_FILE))

                 memcpy(pTabCross->vector,pWrkSymbol->xs.matrix[1],sizeof(double)*NDET);
                else
                 memcpy(pTabCross->vector,ANALYSE_zeros,sizeof(double)*NDET);

//                memcpy(pTabCross->vector,(pUsamp->method==PRJCT_USAMP_FILE)?pWrkSymbol->xs.matrix[1]:ANALYSE_zeros,sizeof(double)*NDET);

                if (((pTabFeno->analysisMethod!=PRJCT_ANLYS_METHOD_SVDMARQUARDT) || (pTabCross->FitParam==ITEM_NONE)))
                 pTabCross->IndSvdA=++pTabFeno->svd.DimC;

                pTabCross->crossAction=((pTabFeno->indexRing1==pTabFeno->NTabCross)) ? ANLYS_CROSS_ACTION_INTERPOLATE : ANLYS_CROSS_ACTION_NOTHING;
                pTabCross->display=(DoasCh)pTabFeno->displayPredefined;

                // DOAS fitting : only the Raman spectrum is fitted non linearly, other parameters are considered as cross sections

                if ((pTabFeno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) && (pTabFeno->indexRing1!=pTabFeno->NTabCross))
                 {
                  pTabCross->InitConc=pTabCross->InitParam;
                  pTabCross->DeltaConc=pTabCross->DeltaParam;
                  pTabCross->FitConc=(pTabCross->FitParam!=ITEM_NONE)?1:0;

                  // Swap columns of original matrix A in order to have in the end of the matrix, cross sections with fixed concentrations

                  if (pTabCross->FitConc!=0)
                   {
                    for (indexTabCross=pTabFeno->NTabCross-1;indexTabCross>=0;indexTabCross--)
                     if (((indexSvd=pTabFeno->TabCross[indexTabCross].IndSvdA)!=0) &&
                          !pTabFeno->TabCross[indexTabCross].FitConc)
                      {
                       pTabFeno->TabCross[indexTabCross].IndSvdA=pTabCross->IndSvdA;
                       pTabCross->IndSvdA=indexSvd;
                      }
                   }

                  if (pTabCross->FitParam!=ITEM_NONE)
                   pTabFeno->svd.NF--;

                  pTabCross->FitParam=ITEM_NONE;
                  pTabCross->InitParam=(double)0.;
                 }
                else
                 {
                  pTabCross->InitConc=(double)0.;
                  pTabCross->FitConc=(pTabFeno->analysisMethod==PRJCT_ANLYS_METHOD_SVD)?0:ITEM_NONE;
                  pTabCross->DeltaConc=(double)0.;
                 }
               }
             }

            if ((pTabCross->FitParam!=ITEM_NONE) || (pTabCross->InitParam!=(double)0.))
             {
              pResults->StoreParam=pListItem->storeFit;
              pResults->StoreParamError=pListItem->storeError;
             }
            else
             {
              pResults->StoreSlntCol=pListItem->storeFit;
              pResults->StoreSlntErr=pListItem->storeError;
             }

            if (((pTabFeno->indexOffsetConst==pTabFeno->NTabCross) ||
                 (pTabFeno->indexOffsetOrder1==pTabFeno->NTabCross) ||
                 (pTabFeno->indexOffsetOrder2==pTabFeno->NTabCross)) && pTabFeno->offlFlag &&
                ((pTabCross->FitParam!=ITEM_NONE) || (fabs(pTabCross->InitConc)>(double)1.e-6)))

             rcTmp=rc=ERROR_SetLast("ANALYSE_LoadNonLinear",ERROR_TYPE_FATAL,ERROR_ID_OPTIONS,"Offset (linear <-> non linear fit)",pTabFeno->windowName);

            pTabFeno->NTabCross++;
           }
         }
       }
     }
   }

  EndLoadPredefined :

  // Return

  return rc;
 }

// ----------------------------------------------------------
// AnalyseLoadGaps : Load gaps defined in an analysis windows
// ----------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC ANALYSE_LoadGaps(ENGINE_CONTEXT *pEngineContext,ANALYSIS_GAP *gapList,INT nGaps,double *lambda,double lambdaMin,double lambdaMax)
 {
  // Declarations

  ANALYSIS_GAP *pGap;
  INT (*Fenetre)[2],Z,DimL;
  INDEX indexItem,indexWindow,i;
  double swap,lambda1,lambda2,(*LFenetre)[2];
  FENO *pTabFeno;
  RC rc;

  // Initializations

  pTabFeno=&TabFeno[NFeno];
  LFenetre=pTabFeno->svd.LFenetre;    // gaps in wavelength units
  Fenetre=pTabFeno->svd.Fenetre;      // gaps in pixels units
  rc=ERROR_ID_NO;
  Z=DimL=0;

  // Get window limits

  lambda1=lambdaMin;
  lambda2=lambdaMax;

  pTabFeno->lambdaMinK=
  pTabFeno->lambdaMaxK=0;

  if (lambda1==lambda2)
   rc=ERROR_SetLast("ANALYSIS_LoadGaps",ERROR_TYPE_FATAL,ERROR_ID_GAPS,lambdaMin,lambdaMax);
  else
   {
    if (pTabFeno->lambdaMinK>pTabFeno->lambdaMaxK)
     {
      swap=pTabFeno->lambdaMinK;
      pTabFeno->lambdaMinK=pTabFeno->lambdaMaxK;
      pTabFeno->lambdaMaxK=swap;
     }

    LFenetre[Z][0]=min(lambda1,lambda2);
    LFenetre[Z][1]=max(lambda1,lambda2);

    // Get gaps

    for (indexItem=0,Z=1;(indexItem<nGaps);indexItem++)
     {
     	pGap=&gapList[indexItem];

      lambda1=pGap->minimum;
      lambda2=pGap->maximum;

      if (lambda2<lambda1)
       {
        swap=lambda2;
        lambda2=lambda1;
        lambda1=swap;
       }

      for (indexWindow=0;indexWindow<Z;indexWindow++)
       if ((lambda1>LFenetre[indexWindow][0]) && (lambda2<LFenetre[indexWindow][1]))
        break;

      if (indexWindow==Z)
       rc=ERROR_SetLast("ANALYSIS_LoadGaps",ERROR_TYPE_WARNING,ERROR_ID_GAPS,pGap->minimum,pGap->maximum);
      else if (Z<MAX_FEN)
       {
        for (i=Z;i>indexWindow;i--)
         {
          LFenetre[i][0]=LFenetre[i-1][0];
          LFenetre[i][1]=LFenetre[i-1][1];
         }

        LFenetre[indexWindow][1]=lambda1;
        LFenetre[indexWindow+1][0]=lambda2;

        Z++;
       }
     }

    if (pTabFeno->gomeRefFlag || pEngineContext->refFlag)
     {
      for (indexWindow=0;indexWindow<Z;indexWindow++)
       {
        Fenetre[indexWindow][0]=FNPixel(lambda,LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
        Fenetre[indexWindow][1]=FNPixel(lambda,LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

        DimL+=(Fenetre[indexWindow][1]-Fenetre[indexWindow][0]+1);
       }

      pTabFeno->svd.DimL=DimL;
     }

    pTabFeno->svd.Z=Z;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_LoadOutput
// -----------------------------------------------------------------------------
// PURPOSE       Load output part relative to a cross section from analysis windows properties
//
// INPUT         outputList   options from the output page of analysis windows properties
//               nOutput      number of entries in the list before
//               hidden     1 for the calibration options, 0 for analysis options
//
// OUTPUT        pAmfFlag   non zero if wavelength dependent AMF have to be accounted for
//
// RETURN        return code
// -----------------------------------------------------------------------------

RC ANALYSE_LoadOutput(ANALYSIS_OUTPUT *outputList,INT nOutput)
 {
  // Declarations

  ANALYSIS_OUTPUT *pOutput;
  INDEX indexOutput,indexTabCross;
  CROSS_REFERENCE *TabCross,*pTabCross;                                         //  symbol cross reference
  CROSS_RESULTS   *TabCrossResults,*pResults;                                   //  results stored per symbol in previous list
  FENO *pTabFeno;
  RC rc;

  // Initializations

  pTabFeno=&TabFeno[NFeno];
  TabCross=pTabFeno->TabCross;
  TabCrossResults=pTabFeno->TabCrossResults;
  rc=ERROR_ID_NO;

  pTabFeno->amfFlag=0;

  // Browse output

  for (indexOutput=0;indexOutput<nOutput;indexOutput++)
   {
   	pOutput=&outputList[indexOutput];

   	// Search for the equivalence of symbols between the molecules and output pages
   	// Probably, that indexOutput==indexTabCross would be OK but it's safer

   	for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
  	 	if ((strlen(pOutput->symbol)==strlen(WorkSpace[TabCross[indexTabCross].Comp].symbolName)) &&
  	 	    !strcasecmp(pOutput->symbol,WorkSpace[TabCross[indexTabCross].Comp].symbolName))
  	 	 break;

  	 // Symbol found

  	 if (indexTabCross<pTabFeno->NTabCross)
  	  {
  	  	pTabCross=&TabCross[indexTabCross];
  	  	pResults=&TabCrossResults[indexTabCross];

  	  	if (!(rc=OUTPUT_ReadAmf(pOutput->symbol,
  	  	                        WorkSpace[pTabCross->Comp].amfFileName,
  	  	                        pTabCross->amfType,&pResults->indexAmf)))
       {
        // Load fields dependent on AMF

        if ((pResults->indexAmf!=ITEM_NONE) && (OUTPUT_AmfSpace!=NULL))
         {
          if (OUTPUT_AmfSpace[pResults->indexAmf].type==ANLYS_AMF_TYPE_WAVELENGTH)
           pTabFeno->amfFlag++;

       	  pResults->StoreAmf=pOutput->amf;                                      // flag set if AMF is to be written into output file
          pResults->StoreVrtCol=pOutput->vertCol;                               // flag set if vertical column is to be written into output file
          pResults->StoreVrtErr=pOutput->vertErr;                               // flag set if error on vertical column is to be written into output file
          pResults->VrtFact=pOutput->vertFactor;
          pResults->ResCol=(double)pOutput->resCol;                             // residual column
         }
       }

      pResults->StoreSlntCol=pOutput->slantCol;                                 // flag set if slant column is to be written into output file
      pResults->StoreSlntErr=pOutput->slantErr;                                 // flag set if error on slant column is to be written into output file
      pResults->SlntFact=pOutput->slantFactor;
  	  }
   }

  // Return

  return rc;
 }

// ---------------------------------------
// AnalyseLoadRef : Load reference spectra
// ---------------------------------------

RC ANALYSE_LoadRef(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  FENO *pTabFeno;
  double *Sref;
  double *SrefEtalon,*lambdaRef,*lambdaRefEtalon;
  DoasCh *ptr;
  RC rc;

  // Initializations

  pTabFeno=&TabFeno[NFeno];
  SrefEtalon=pTabFeno->Sref=pTabFeno->SrefEtalon=NULL;
  pTabFeno->Zm=(double)-1.;
  pTabFeno->TDet=(double)0.;
  lambdaRef=lambdaRefEtalon=NULL;
  pTabFeno->useEtalon=0;
  pTabFeno->displayRef=0;
  pEngineContext->refFlag=0;

  pTabFeno->gomeRefFlag=((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GDP_ASCII)&&
                         (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GDP_BIN) &&
                         (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_HDF) &&
                         (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS) &&
                         (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) &&
                         (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GOME2))?1:0;

           //
           // in the case of satellites measurements :
           //
           //      gomeRefFlag=0 means that irradiance is used as etalon spectrum
           //      gomeRefFlag=1 means that a reference spectrum is given
           //
           // in other formats, gomeRefFlag is always equal to 1 even though no
           // reference is given (tropospheric measurements)
           //

  memset(pTabFeno->refFile,0,MAX_ITEM_TEXT_LEN+1);
  rc=0;

  // Allocate memory for reference spectra

  if (((Sref=pTabFeno->Sref=(double *)MEMORY_AllocDVector("ANALYSE_LoadRef ","Sref",0,NDET-1))==NULL) ||
      ((SrefEtalon=pTabFeno->SrefEtalon=(double *)MEMORY_AllocDVector("ANALYSE_LoadRef ","SrefEtalon",0,NDET))==NULL) ||

     (((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)) &&
      ((pTabFeno->SrefSigma=(double *)MEMORY_AllocDVector("ANALYSE_LoadRef ","SrefSigma",0,NDET))==NULL)) ||

      ((lambdaRef=(double *)MEMORY_AllocDVector("ANALYSE_LoadRef ","lambdaRef",0,NDET))==NULL) ||
      ((lambdaRefEtalon=(double *)MEMORY_AllocDVector("ANALYSE_LoadRef ","lambdaRefEtalon",0,NDET))==NULL) ||

      ((pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) &&
      ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&
     (((pTabFeno->SrefN=(double *)MEMORY_AllocDVector("ANALYSE_LoadRef ","SrefN",0,NDET-1))==NULL) ||
      ((pTabFeno->SrefS=(double *)MEMORY_AllocDVector("ANALYSE_LoadRef ","SrefS",0,NDET-1))==NULL) ||
      ((pTabFeno->LambdaN=(double *)MEMORY_AllocDVector("ANALYSE_LoadRef ","LambdaN",0,NDET-1))==NULL) ||
      ((pTabFeno->LambdaS=(double *)MEMORY_AllocDVector("ANALYSE_LoadRef ","LambdaS",0,NDET-1))==NULL))))

   rc=ERROR_ID_ALLOC;

  // Load reference spectra

  else if (((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
            (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD) ||
            (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_BIRA)) &&
            (strlen(pTabFeno->ref1) && !strrchr(pTabFeno->ref1,PATH_SEP)))
   {
   	strcpy(pTabFeno->refFile,pTabFeno->ref1);
    pTabFeno->gomeRefFlag=0;
    pEngineContext->refFlag++;
   }
  else if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MKZY) && !strlen(pTabFeno->ref1) && !strlen(pTabFeno->ref2))
   {
   	pTabFeno->gomeRefFlag=0;
   	pEngineContext->refFlag++;
   }
  else
   {
    // ====
    // Ref1
    // ====

    memcpy(lambdaRefEtalon,pTabFeno->LambdaRef,sizeof(double)*NDET);
    memcpy(SrefEtalon,ANALYSE_ones,sizeof(double)*NDET);

    if ((pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC) &&                             // if the wavelength calibration procedure is applied on the measured
        (pTabFeno->useKurucz!=ANLYS_KURUCZ_REF_AND_SPEC) &&                     // spectrum, the ref1 has no sense.
         strlen(pTabFeno->ref1) &&

       ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) ||
      (((ptr=strrchr(pTabFeno->ref1,'.'))!=NULL) &&
        (strlen(ptr)==4) && !strcasecmp(ptr,".ref"))) &&

       !(rc=AnalyseLoadVector("ANALYSE_LoadRef (SrefEtalon) ",pTabFeno->ref1,lambdaRefEtalon,SrefEtalon,1,NULL)) &&
       !(rc=THRD_SpectrumCorrection(pEngineContext,SrefEtalon)) &&
       !(rc=VECTOR_NormalizeVector(SrefEtalon-1,NDET,&pTabFeno->refNormFact,"ANALYSE_LoadRef (SrefEtalon) ")))
     {
      pTabFeno->displayRef=pTabFeno->useEtalon=pTabFeno->gomeRefFlag=1;
      strcpy(pTabFeno->refFile,pTabFeno->ref1);
     }

    // ====
    // Ref2
    // ====

    memcpy(lambdaRef,pTabFeno->LambdaRef,sizeof(double)*NDET);
    memcpy(Sref,SrefEtalon,sizeof(double)*NDET);

    if (!rc &&
       (pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC) &&
       (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE) &&
        strlen(pTabFeno->ref2) &&

       ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) ||
      (((ptr=strrchr(pTabFeno->ref2,'.'))!=NULL) &&
        (strlen(ptr)==4) && !strcasecmp(ptr,".ref"))) &&

      !(rc=AnalyseLoadVector("ANALYSE_LoadRef (Sref) ",pTabFeno->ref2,lambdaRef,Sref,1,NULL)) &&
      !(rc=THRD_SpectrumCorrection(pEngineContext,Sref)) &&
      !(rc=VECTOR_NormalizeVector(Sref-1,NDET,&pTabFeno->refNormFact,"ANALYSE_LoadRef (Sref) ")))
     {
      if (!pTabFeno->useEtalon)
       {
        memcpy(SrefEtalon,Sref,sizeof(double)*NDET);
        memcpy(lambdaRefEtalon,lambdaRef,sizeof(double)*NDET);
       }

      strcpy(pTabFeno->refFile,pTabFeno->ref2);
      pTabFeno->displayRef=pTabFeno->useEtalon=pTabFeno->gomeRefFlag=1;
     }

    if (!rc)
     memcpy(pTabFeno->LambdaRef,(pTabFeno->useEtalon)?lambdaRefEtalon:lambdaRef,sizeof(double)*NDET);
   }

  // Return

  if (lambdaRef!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_LoadRef ","lambdaRef",lambdaRef,0);
  if (lambdaRefEtalon!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_LoadRef ","lambdaRefEtalon",lambdaRefEtalon,0);

  return rc;
 }

// -----------------------------------------------------------------------------------------
// AnalyseSetAnalysisType : Set the type of analysis to apply in the current analysis window
// -----------------------------------------------------------------------------------------

void ANALYSE_SetAnalysisType(void)
 {
  // Declarations

  CROSS_REFERENCE *TabCross;
  FENO *pTabFeno;

  // Initializations

  pTabFeno=&TabFeno[NFeno];
  TabCross=pTabFeno->TabCross;
  pTabFeno->analysisType=ANALYSIS_TYPE_FWHM_NONE;

  // Fit fwhm in Kurucz procedure

  if ((pTabFeno->hidden) && pKuruczOptions->fwhmFit)
   pTabFeno->analysisType=ANALYSIS_TYPE_FWHM_KURUCZ;

  else if (!pTabFeno->hidden)
   {
    // Apply a correction of resolution on reference or spectrum based on the temperature of these spectra

    if (pSlitOptions->fwhmCorrectionFlag)
     pTabFeno->analysisType=ANALYSIS_TYPE_FWHM_CORRECTION;

    // Fit of the difference of resolution between spectrum and reference
    // At least fit of fwhm order 0 should be set

    else if ((pTabFeno->indexFwhmConst!=ITEM_NONE) && ((TabCross[pTabFeno->indexFwhmConst].FitParam!=ITEM_NONE) || (TabCross[pTabFeno->indexFwhmConst].InitParam!=(double)0.)))
     pTabFeno->analysisType=ANALYSIS_TYPE_FWHM_NLFIT;
   }
 }

// -------------------------------------------
// ANALYSE_LoadData : Load data from a project
// -------------------------------------------

RC ANALYSE_SetInit(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  RC rc;                           // return code

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ANALYSE_SetInit",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  ANALYSE_ignoreAll=0;
  pEngineContext->analysisRef.refAuto=pEngineContext->analysisRef.refLon=0;
  analyseIndexRecord=pEngineContext->indexRecord;
  memset(&pEngineContext->analysisRef,0,sizeof(ANALYSIS_REF));

  rc=ERROR_ID_NO;

  // Release all previously allocated buffers

  ANALYSE_plFilter=&pEngineContext->project.lfilter;
  ANALYSE_phFilter=&pEngineContext->project.hfilter;

  ANALYSE_plFilter->filterFunction=ANALYSE_phFilter->filterFunction=NULL;
  ANALYSE_plFilter->filterSize=ANALYSE_phFilter->filterSize=0;
  ANALYSE_plFilter->filterEffWidth=ANALYSE_phFilter->filterEffWidth=1.;

  ANALYSE_ResetData();

  // Allocate buffers for general use

  pAnalysisOptions=&pEngineContext->project.analysis;
  pSlitOptions=&pEngineContext->project.slit;
  pKuruczOptions=&pEngineContext->project.kurucz;
  pUsamp=&pEngineContext->project.usamp;

  if (pSlitOptions->fwhmCorrectionFlag && pKuruczOptions->fwhmFit)
   rc=ERROR_SetLast("ANALYSE_LoadData",ERROR_TYPE_FATAL,ERROR_ID_FWHM);
  else if (!(rc=FILTER_LoadFilter(&pEngineContext->project.lfilter)) &&   // low pass filtering
           !(rc=FILTER_LoadFilter(&pEngineContext->project.hfilter)) &&   // high pass filtering
           !(rc=AnalyseSvdGlobalAlloc(pEngineContext)))
   {
    if (((ANALYSE_zeros=(double *)MEMORY_AllocDVector("ANALYSE_LoadData ","ANALYSE_zeros",0,NDET-1))==NULL) ||
        ((ANALYSE_ones=(double *)MEMORY_AllocDVector("ANALYSE_LoadData ","ANALYSE_ones",0,NDET-1))==NULL))

     rc=ERROR_ID_ALLOC;

    else
     {
      VECTOR_Init(ANALYSE_zeros,(double)0.,NDET);
      VECTOR_Init(ANALYSE_ones,(double)1.,NDET);
     }
   }

  // Return

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ANALYSE_SetInit",(RC)rc);
  analyseDebugMask=0;
  #endif

  return rc;
 }

// ====================
// RESOURCES MANAGEMENT
// ====================

// ------------------------------------------------------------------
// ANALYSE_Alloc : All analysis buffers allocation and initialization
// ------------------------------------------------------------------

RC ANALYSE_Alloc(void)
 {
  // Declaration

  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Allocate all buffers need for analysis

  if (((WorkSpace=(WRK_SYMBOL *)MEMORY_AllocBuffer("ANALYSE_Alloc ","WorkSpace",MAX_SYMB,sizeof(WRK_SYMBOL),0,MEMORY_TYPE_STRUCT))==NULL) ||
      ((TabFeno=(FENO *)MEMORY_AllocBuffer("ANALYSE_Alloc ","TabFeno",MAX_FENO,sizeof(FENO),0,MEMORY_TYPE_STRUCT))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    memset(WorkSpace,0,sizeof(WRK_SYMBOL)*MAX_SYMB);
    memset(TabFeno,0,sizeof(FENO)*MAX_FENO);

    memset(&O3TD,0,sizeof(MATRIX_OBJECT));
    memset(&ANALYSIS_slit,0,sizeof(MATRIX_OBJECT));
    memset(&ANALYSIS_slitK,0,sizeof(MATRIX_OBJECT));
    memset(&ANALYSIS_broAmf,0,sizeof(MATRIX_OBJECT));
   }

  // Return

  return rc;
 }

// ------------------------------------------------
// ANALYSE_Free : Release buffers used for analysis
// ------------------------------------------------

void ANALYSE_Free(void)
 {
  if (WorkSpace!=NULL)
   MEMORY_ReleaseBuffer("ANALYSE_Free ","WorkSpace",WorkSpace);
  if (TabFeno!=NULL)
   MEMORY_ReleaseBuffer("ANALYSE_Free ","TabFeno",TabFeno);

  WorkSpace=NULL;
  TabFeno=NULL;
 }


// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_UsampBuild
// -----------------------------------------------------------------------------
// PURPOSE       Build undersampling cross sections during analysis process.
//
// INPUT         analysisFlag :
//
//                   0 : file reference selection mode
//                   1 : automatic reference selection mode
//                   2 : automatic undersampling mode
//
//               gomeFlag :
//
//                   0 : Ref1 is the irradiance spectrum
//                   1 : Ref1 is a user-defined spectrum
// -----------------------------------------------------------------------------
// DESCRIPTION
//
// This function is called from different functions during analysis process
// according to selected reference options and undersampling method.
// In order to avoid multiple creation of undersampling cross sections, the
// calling function set input flags.
//
// Available combinations :
//
// 0,1 : called only one time from ANALYSE_LoadData because reference is already
//       available (Ref1 is a user-defined spectrum);
//       no automatic reference selection.
//
// 0,0 : called from GOME_LoadAnalysis for every spectra file;
//       the Ref1 is the irradiance spectrum;
//       no automatic reference selection.
//
// 1,ITEM_NONE : in automatic reference selection mode, the undersampling
//               must be calculated before aligning Ref1 sur Ref2; this function
//               is called from GOME_LoadAnalysis.

//
// 2,ITEM_NONE : used several times during analysis process in the following
//               cases :
//
//               called from Function for every iteration
//               in automatic undersampling method
//               -> apply current calibration with current fitted shift
//               -> independent from the reference selection mode
//
//               also called from ANALYSE_Spectrum each time the reference
//               spectrum is changed because the calibration used to calculate
//               undersampling cross sections should account for possible shift
//               between Ref2 and Ref1;
//               so this function is called after ANALYSE_Spectrum.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC ANALYSE_UsampBuild(INT analysisFlag,INT gomeFlag)
 {
  // Declarations

  XS xsKurucz,xsSlit,slitXs;
  INDEX indexFeno,i,indexPixMin,indexPixMax,indexParam,j;
  INT slitType;
  double slitParam2,*lambda,*lambda2,slitParam[MAX_KURUCZ_FWHM_PARAM],lambda0,x0;
  FENO *pTabFeno,*pKuruczFeno;
  RC rc;

  // Initializations

  lambda2=NULL;
  pKuruczFeno=&TabFeno[KURUCZ_buffers.indexKurucz];
  memset(&xsSlit,0,sizeof(XS));
  memset(&slitXs,0,sizeof(XS));
  slitParam2=(double)0.;
  rc=ERROR_ID_NO;

  // Buffer allocation

  if (((lambda=(double *)MEMORY_AllocDVector("ANALYSE_UsampBuild ","lambda",0,NDET-1))==NULL) ||
      ((lambda2=(double *)MEMORY_AllocDVector("ANALYSE_UsampBuild ","lambda2",0,NDET-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else

   // Browse analysis windows

   for (indexFeno=0;(indexFeno<NFeno)&&!rc;indexFeno++)
    {
     pTabFeno=&TabFeno[indexFeno];

     // Check options combinations

     if (!pTabFeno->hidden && (pTabFeno->useUsamp) &&
        ((gomeFlag==ITEM_NONE) || (pTabFeno->gomeRefFlag==gomeFlag)) &&
       (((analysisFlag==0) && (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE) && (pUsamp->method==PRJCT_USAMP_FIXED)) ||
        ((analysisFlag==1) && (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) && (pUsamp->method==PRJCT_USAMP_FIXED)) ||
        ((analysisFlag==2) && (pTabFeno==Feno))))
      {
       // Build lambda for second phase

       memcpy(lambda,pTabFeno->LambdaK,sizeof(double)*NDET);

       if ((analysisFlag==2) && (pUsamp->method==PRJCT_USAMP_FIXED))
        {
        for (j=0,lambda0=pTabFeno->LambdaK[(SvdPDeb+SvdPFin)/2];j<NDET;j++)
         {
          x0=lambda[j]-lambda0;
          lambda[j]-=(pTabFeno->Shift+(pTabFeno->Stretch+pTabFeno->Stretch2*x0)*x0);
         }
        }

       memcpy(lambda2,ANALYSE_zeros,sizeof(double)*NDET);

       indexPixMin=0;
       indexPixMax=NDET;

       if (pUsamp->method==PRJCT_USAMP_FIXED)
        for (i=indexPixMin+1,lambda2[indexPixMin]=lambda[indexPixMin]-pUsamp->phase;i<indexPixMax;i++)
         lambda2[i]=lambda[i]-pUsamp->phase; //  (double)(1.-pUsamp->phase)*lambda[i-1]+pUsamp->phase*lambda[i];
       else
        {
         memcpy(lambda2,ANALYSE_shift,sizeof(double)*pTabFeno->NDET);

         if (pAnalysisOptions->units==PRJCT_ANLYS_UNITS_PIXELS)
          {
           for (;(indexPixMin<indexPixMax) && (ANALYSE_shift[indexPixMin]<1.e-3);indexPixMin++);
           for (;(indexPixMax>=indexPixMin) && (ANALYSE_shift[indexPixMax]<1.e-3);indexPixMax--);

           rc=SPLINE_Vector(ANALYSE_splineX,lambda,ANALYSE_splineX2,pTabFeno->NDET,&ANALYSE_shift[indexPixMin],
                              &lambda2[indexPixMin],(indexPixMax-indexPixMin+1),pAnalysisOptions->interpol,"ANALYSE_UsampBuild ");
          }
        }

       // Local initializations

       xsKurucz.vector=ANALYSE_usampBuffers.kuruczConvoluted[indexFeno];
       xsKurucz.deriv2=ANALYSE_usampBuffers.kuruczConvoluted2[indexFeno];
       xsKurucz.lambda=&ANALYSE_usampBuffers.hrSolar.lambda[ANALYSE_usampBuffers.lambdaRange[0][indexFeno]];
       xsKurucz.NDET=ANALYSE_usampBuffers.lambdaRange[1][indexFeno];

       // Not allowed combinations :
       //
       //     - fit slit function with calibration and apply calibration on spec only or on ref and spec;
       //     - fwhmCorrectionFlag and no calibration or fit slit function with the calibration


       if ((((pTabFeno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC) || (pTabFeno->useKurucz==ANLYS_KURUCZ_SPEC)) && pKuruczOptions->fwhmFit) ||
            ((!pTabFeno->useKurucz || pKuruczOptions->fwhmFit) && pSlitOptions->fwhmCorrectionFlag))

        rc=ERROR_SetLast("ANALYSE_UsampBuild",ERROR_TYPE_FATAL,ERROR_ID_OPTIONS,"Undersampling ",pTabFeno->windowName);


       // Convolution with user-defined slit function

       else
        {
         if (!pTabFeno->useKurucz ||                                         // don't apply calibration
             !pKuruczOptions->fwhmFit)                                       // apply calibration but don't fit the slit function
          {
           // Convolution with slit function from slit tab page of project properties

           if ((ANALYSIS_slit.matrix!=NULL) && ANALYSIS_slit.nl && ANALYSIS_slit.nc)
            {
             xsSlit.lambda=ANALYSIS_slit.matrix[ANALYSIS_slit.basec]+ANALYSIS_slit.basel;
             xsSlit.vector=ANALYSIS_slit.matrix[ANALYSIS_slit.basec+1]+ANALYSIS_slit.basel;
             xsSlit.deriv2=ANALYSIS_slit.deriv2[ANALYSIS_slit.basec+1]+ANALYSIS_slit.basel;

             xsSlit.NDET=ANALYSIS_slit.nl;
            }

           if (!(rc=XSCONV_TypeStandard(&xsKurucz,0,xsKurucz.NDET,&ANALYSE_usampBuffers.hrSolar,&xsSlit,&ANALYSE_usampBuffers.hrSolar,NULL,
                                        pSlitOptions->slitFunction.slitType,(double)2.*pSlitOptions->slitFunction.slitParam,
                                        pSlitOptions->slitFunction.slitParam,pSlitOptions->slitFunction.slitParam2,
                                        pSlitOptions->slitFunction.slitParam3,pSlitOptions->slitFunction.slitParam4)))

            rc=SPLINE_Deriv2(xsKurucz.lambda,xsKurucz.vector,xsKurucz.deriv2,xsKurucz.NDET,"ANALYSE_UsampBuild (2) ");
          }

         // Convolution with slit function resulting from Kurucz

         else
          {
           xsSlit.lambda=pTabFeno->LambdaK;
           xsSlit.vector=pTabFeno->fwhmVector[0];
           xsSlit.NDET=pTabFeno->NDET;


           // Build slit function and convolute high resolution solar spectrum

           switch(pKuruczOptions->fwhmType)
            {
          // ---------------------------------------------------------------------------
             case SLIT_TYPE_ERF :
              slitType=SLIT_TYPE_ERF_FILE;
             break;
          // ---------------------------------------------------------------------------
             case SLIT_TYPE_INVPOLY :
              slitType=SLIT_TYPE_INVPOLY_FILE;
             break;
          // ---------------------------------------------------------------------------
             case SLIT_TYPE_VOIGT :           // for the moment, the program doesn't support multiple
              slitType=SLIT_TYPE_VOIGT;       // parameters dependency
             break;
          // ---------------------------------------------------------------------------
             default :
              slitType=SLIT_TYPE_GAUSS_FILE;
             break;
          // ---------------------------------------------------------------------------
            }

           if (slitType==SLIT_TYPE_VOIGT)
            {
             SLIT slitOptions;
             INT slitType2;

             for (i=0;(i<xsKurucz.NDET) && !rc;i++)
              {
               for (indexParam=0;indexParam<MAX_KURUCZ_FWHM_PARAM;indexParam++)
                {
                if (pTabFeno->fwhmVector[indexParam]==NULL)
                 slitParam[indexParam]=(double)0.;
                else if (pTabFeno->fwhmDeriv2[indexParam]==NULL)
                 slitParam[indexParam]=pKuruczFeno->TabCross[pKuruczFeno->indexFwhmParam[indexParam]].InitParam;
                else if ((rc=SPLINE_Vector(pTabFeno->LambdaK,pTabFeno->fwhmVector[indexParam],pTabFeno->fwhmDeriv2[indexParam],pTabFeno->NDET,
                                              &xsKurucz.lambda[i],&slitParam[indexParam],1,pAnalysisOptions->interpol,"ANALYSE_UsampBuild "))!=ERROR_ID_NO)
                 break;
                }

               slitOptions.slitType=slitType;                                   // The program doesn't support yet Voigt function parameters dependency
               slitOptions.slitFile[0]=0;                                       // => build Voigt function pixel per pixel
               slitOptions.slitParam=slitParam[0];
               slitOptions.slitParam2=slitParam[1];
               slitOptions.slitParam3=slitParam[2];
               slitOptions.slitParam4=slitParam[3];

               if (!rc &&
                 (((rc=XSCONV_LoadSlitFunction(&slitXs,&slitOptions,&slitParam[0],&slitType2))!=0) ||
                  ((rc=XSCONV_TypeStandard(&xsKurucz,i,i+1,&ANALYSE_usampBuffers.hrSolar,&slitXs,&ANALYSE_usampBuffers.hrSolar,NULL,
                                           SLIT_TYPE_FILE,(double)3.*slitParam[0],(double)0.,(double)0.,(double)0.,(double)0.))!=0)))
                break;
              }
            }
           else
            {
             if (slitType!=SLIT_TYPE_GAUSS_FILE)
              for (i=0;i<pTabFeno->NDET;i++)
               slitParam2+=pTabFeno->fwhmVector[1][i];

             if ((xsSlit.deriv2=(double *)MEMORY_AllocDVector("ANALYSE_UsampBuild ","deriv2",0,pTabFeno->NDET-1))==NULL)
              rc=ERROR_ID_ALLOC;
             else if (!(rc=SPLINE_Deriv2(xsSlit.lambda,xsSlit.vector,xsSlit.deriv2,xsSlit.NDET,"ANALYSE_UsampBuild (Slit) ")))
              rc=XSCONV_TypeStandard(&xsKurucz,0,xsKurucz.NDET,&ANALYSE_usampBuffers.hrSolar,&xsSlit,&ANALYSE_usampBuffers.hrSolar,NULL,
                                      slitType,(double)0.,(double)0.,slitParam2/pTabFeno->NDET,(double)0.,(double)0.);

             if (xsSlit.deriv2!=NULL)
              MEMORY_ReleaseDVector("ANALYSE_UsampBuild ","deriv2",xsSlit.deriv2,0);
            }
          }

         // Pre-Interpolation on analysis window wavelength scale

         if (!rc &&
            !(rc=SPLINE_Deriv2(xsKurucz.lambda,xsKurucz.vector,xsKurucz.deriv2,xsKurucz.NDET,"ANALYSE_UsampBuild (1) ")))
          {
           memcpy(ANALYSE_usampBuffers.kuruczInterpolated[indexFeno],ANALYSE_zeros,sizeof(double)*pTabFeno->NDET);
           memcpy(ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno],ANALYSE_zeros,sizeof(double)*pTabFeno->NDET);

           indexPixMin=ANALYSE_usampBuffers.lambdaRange[2][indexFeno]=FNPixel(lambda,xsKurucz.lambda[0],pTabFeno->NDET,PIXEL_AFTER);
           indexPixMax=FNPixel(lambda,xsKurucz.lambda[xsKurucz.NDET-1],pTabFeno->NDET,PIXEL_BEFORE);
           ANALYSE_usampBuffers.lambdaRange[3][indexFeno]=indexPixMax-ANALYSE_usampBuffers.lambdaRange[2][indexFeno]+1;

           if (!(rc=SPLINE_Vector(xsKurucz.lambda,xsKurucz.vector,xsKurucz.deriv2,xsKurucz.NDET,
                                    &lambda[indexPixMin],&ANALYSE_usampBuffers.kuruczInterpolated[indexFeno][indexPixMin],(indexPixMax-indexPixMin+1),
                                     pAnalysisOptions->interpol,"ANALYSE_UsampBuild ")))

            rc=SPLINE_Deriv2(&lambda[indexPixMin],&ANALYSE_usampBuffers.kuruczInterpolated[indexFeno][indexPixMin],
                     &ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno][indexPixMin],(indexPixMax-indexPixMin+1),"ANALYSE_UsampBuild (3) ");
          }

         // Build undersampling correction

         if (!rc)

          rc=USAMP_BuildCrossSections
          ((pTabFeno->indexUsamp1!=ITEM_NONE)?&pTabFeno->TabCross[pTabFeno->indexUsamp1].vector[ANALYSE_usampBuffers.lambdaRange[2][indexFeno]]:NULL,
           (pTabFeno->indexUsamp2!=ITEM_NONE)?&pTabFeno->TabCross[pTabFeno->indexUsamp2].vector[ANALYSE_usampBuffers.lambdaRange[2][indexFeno]]:NULL,
            &lambda[ANALYSE_usampBuffers.lambdaRange[2][indexFeno]],
            &lambda2[ANALYSE_usampBuffers.lambdaRange[2][indexFeno]],
            &ANALYSE_usampBuffers.kuruczInterpolated[indexFeno][ANALYSE_usampBuffers.lambdaRange[2][indexFeno]],
            &ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno][ANALYSE_usampBuffers.lambdaRange[2][indexFeno]],
             ANALYSE_usampBuffers.lambdaRange[3][indexFeno],
             xsKurucz.lambda,
             xsKurucz.vector,
             xsKurucz.deriv2,
             xsKurucz.NDET,
             pTabFeno->analysisMethod);
        }
      }
    }

  // Return

  XSCONV_Reset(&slitXs);

  if (lambda!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_UsampBuild ","lambda",lambda,0);
  if (lambda2!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_UsampBuild ","lambda2",lambda2,0);

  return rc;
 }

// ==================
// BUFFERS ALLOCATION
// ==================

// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_UsampGlobalAlloc
// -----------------------------------------------------------------------------
// PURPOSE       allocate buffers (not depending on analysis windows) for the calculation of the undersampling XS
//
// INPUT         lambdaMin, lambdaMax : range of wavelengths for the kurucz spectrum
//               size                 : the size of the final wavelength calibration
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_USAMP if another called function returns an error;
//               ERROR_ID_NO on success
// -----------------------------------------------------------------------------

RC ANALYSE_UsampGlobalAlloc(double lambdaMin,double lambdaMax,INT size)
 {
  // Declarations

  DoasCh kuruczFile[MAX_ITEM_TEXT_LEN+1];
  FENO *pTabFeno;
  INDEX indexFeno;
  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Load high resolution kurucz spectrum
  // NB : if the high resolution spectrum is the same as used in Kurucz, don't reload it from file but only make a copy

  if ((strlen(pKuruczOptions->file)==strlen(pUsamp->kuruczFile)) &&
      !strcasecmp(pKuruczOptions->file,pUsamp->kuruczFile) &&
       KURUCZ_buffers.hrSolar.NDET)
   {
    if ((rc=XSCONV_Alloc(&ANALYSE_usampBuffers.hrSolar,KURUCZ_buffers.hrSolar.NDET,1))!=ERROR_ID_NO)
     rc=ERROR_ID_ALLOC;
    else
     {
      memcpy(ANALYSE_usampBuffers.hrSolar.lambda,KURUCZ_buffers.hrSolar.lambda,sizeof(double)*KURUCZ_buffers.hrSolar.NDET);
      memcpy(ANALYSE_usampBuffers.hrSolar.vector,KURUCZ_buffers.hrSolar.vector,sizeof(double)*KURUCZ_buffers.hrSolar.NDET);
      memcpy(ANALYSE_usampBuffers.hrSolar.deriv2,KURUCZ_buffers.hrSolar.deriv2,sizeof(double)*KURUCZ_buffers.hrSolar.NDET);
     }
   }
  else
   {
    FILES_RebuildFileName(kuruczFile,pUsamp->kuruczFile,1);

    if (!(rc=XSCONV_LoadCrossSectionFile(&ANALYSE_usampBuffers.hrSolar,kuruczFile,lambdaMin-7.,lambdaMax+7.,(double)0.,CONVOLUTION_CONVERSION_NONE)))
     rc=VECTOR_NormalizeVector(ANALYSE_usampBuffers.hrSolar.vector-1,ANALYSE_usampBuffers.hrSolar.NDET,NULL,"ANALYSE_UsampGlobalAlloc ");
   }

  if (rc!=ERROR_ID_NO)
   rc=ERROR_SetLast("ANALYSE_UsampGlobalAlloc",ERROR_TYPE_FATAL,ERROR_ID_USAMP,"ANALYSE_UsampGlobalAlloc (0)");

  // Buffers allocation

  else if (((ANALYSE_usampBuffers.lambdaRange[0]=(INT *)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","lambdaRange[0]",NFeno,sizeof(INT),0,MEMORY_TYPE_INT))==NULL) ||
           ((ANALYSE_usampBuffers.lambdaRange[1]=(INT *)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","lambdaRange[1]",NFeno,sizeof(INT),0,MEMORY_TYPE_INT))==NULL) ||
           ((ANALYSE_usampBuffers.lambdaRange[2]=(INT *)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","lambdaRange[2]",NFeno,sizeof(INT),0,MEMORY_TYPE_INT))==NULL) ||
           ((ANALYSE_usampBuffers.lambdaRange[3]=(INT *)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","lambdaRange[3]",NFeno,sizeof(INT),0,MEMORY_TYPE_INT))==NULL) ||
           ((ANALYSE_usampBuffers.kuruczConvoluted=(double **)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","kuruczConvoluted",NFeno,sizeof(double *),0,MEMORY_TYPE_DOUBLE))==NULL) ||
           ((ANALYSE_usampBuffers.kuruczConvoluted2=(double **)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","kuruczConvoluted2",NFeno,sizeof(double *),0,MEMORY_TYPE_DOUBLE))==NULL) ||
           ((ANALYSE_usampBuffers.kuruczInterpolated=(double **)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","kuruczInterpolated",NFeno,sizeof(double *),0,MEMORY_TYPE_DOUBLE))==NULL) ||
           ((ANALYSE_usampBuffers.kuruczInterpolated2=(double **)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","kuruczInterpolated2",NFeno,sizeof(double *),0,MEMORY_TYPE_DOUBLE))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
     {
      pTabFeno=&TabFeno[indexFeno];

      ANALYSE_usampBuffers.lambdaRange[0][indexFeno]=
      ANALYSE_usampBuffers.lambdaRange[1][indexFeno]=
      ANALYSE_usampBuffers.lambdaRange[2][indexFeno]=
      ANALYSE_usampBuffers.lambdaRange[3][indexFeno]=ITEM_NONE;

      ANALYSE_usampBuffers.kuruczConvoluted[indexFeno]=
      ANALYSE_usampBuffers.kuruczConvoluted2[indexFeno]=
      ANALYSE_usampBuffers.kuruczInterpolated[indexFeno]=
      ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno]=NULL;

      if (!pTabFeno->hidden && pTabFeno->useUsamp &&
        (((ANALYSE_usampBuffers.kuruczInterpolated[indexFeno]=(double *)MEMORY_AllocDVector("ANALYSE_UsampGlobalAlloc ","kuruczInterpolated",0,size-1))==NULL) ||
         ((ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno]=(double *)MEMORY_AllocDVector("ANALYSE_UsampGlobalAlloc ","kuruczInterpolated2",0,size-1))==NULL)))

       rc=ERROR_ID_ALLOC;

      else
       ANALYSE_usampBuffers.lambdaRange[0][indexFeno]=ANALYSE_usampBuffers.lambdaRange[1][indexFeno]=ITEM_NONE;
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_UsampLocalAlloc
// -----------------------------------------------------------------------------
// PURPOSE       allocate buffers (depending on analysis windows) for the calculation of the undersampling XS
//
// INPUT         gomeFlag = 0 : Ref1 is the irradiance spectrum
//                          1 : Ref1 is a user-defined spectrum
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO on success
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC ANALYSE_UsampLocalAlloc(INT gomeFlag)
 {
  // Declarations

  INT lambdaSize,endPixel;
  INDEX indexFeno;
  FENO *pTabFeno;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
   {
    pTabFeno=&TabFeno[indexFeno];

    if (!pTabFeno->hidden && pTabFeno->useUsamp
        && (pTabFeno->gomeRefFlag==gomeFlag)
        )
     {
      ANALYSE_usampBuffers.lambdaRange[0][indexFeno]=FNPixel(ANALYSE_usampBuffers.hrSolar.lambda,(double)pTabFeno->svd.LFenetre[0][0]-7.,ANALYSE_usampBuffers.hrSolar.NDET,PIXEL_AFTER);
      endPixel=FNPixel(ANALYSE_usampBuffers.hrSolar.lambda,(double)pTabFeno->svd.LFenetre[pTabFeno->svd.Z-1][1]+7.,ANALYSE_usampBuffers.hrSolar.NDET,PIXEL_BEFORE);

      lambdaSize=ANALYSE_usampBuffers.lambdaRange[1][indexFeno]=endPixel-ANALYSE_usampBuffers.lambdaRange[0][indexFeno]+1;

      if (((ANALYSE_usampBuffers.kuruczConvoluted[indexFeno]=(double *)MEMORY_AllocDVector("ANALYSE_UsampLocalAlloc ","kuruczConvoluted",0,lambdaSize-1))==NULL) ||
          ((ANALYSE_usampBuffers.kuruczConvoluted2[indexFeno]=(double *)MEMORY_AllocDVector("ANALYSE_UsampLocalAlloc ","kuruczConvoluted2",0,lambdaSize-1))==NULL))

       rc=ERROR_ID_ALLOC;
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_UsampLocalFree
// -----------------------------------------------------------------------------
// PURPOSE       release the buffers previously allocated by the ANALYSE_UsampLocalAlloc function
// -----------------------------------------------------------------------------

void ANALYSE_UsampLocalFree(void)
 {
  // Declarations

  INDEX indexFeno;

  // Release allocated buffers

  if (ANALYSE_usampBuffers.kuruczConvoluted!=NULL)
   {
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (!TabFeno[indexFeno].hidden && TabFeno[indexFeno].useUsamp && !TabFeno[indexFeno].gomeRefFlag)
      {
       if (ANALYSE_usampBuffers.kuruczConvoluted[indexFeno]!=NULL)
        {
         MEMORY_ReleaseDVector("ANALYSE_UsampLocalFree ","kuruczConvoluted",ANALYSE_usampBuffers.kuruczConvoluted[indexFeno],0);
         ANALYSE_usampBuffers.kuruczConvoluted[indexFeno]=NULL;
        }

       if (ANALYSE_usampBuffers.kuruczConvoluted2[indexFeno]!=NULL)
        {
         MEMORY_ReleaseDVector("ANALYSE_UsampLocalFree ","kuruczConvoluted2",ANALYSE_usampBuffers.kuruczConvoluted2[indexFeno],0);
         ANALYSE_usampBuffers.kuruczConvoluted2[indexFeno]=NULL;
        }
      }
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_UsampGlobalFree
// -----------------------------------------------------------------------------
// PURPOSE       release the buffers previously allocated by the ANALYSE_UsampGlobalAlloc function
// -----------------------------------------------------------------------------

void ANALYSE_UsampGlobalFree(void)
 {
  INDEX indexFeno;

  XSCONV_Reset(&ANALYSE_usampBuffers.hrSolar);

  if (ANALYSE_usampBuffers.lambdaRange[0]!=NULL)
   MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","lambdaRange[0]",ANALYSE_usampBuffers.lambdaRange[0]);
  if (ANALYSE_usampBuffers.lambdaRange[1]!=NULL)
   MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","lambdaRange[1]",ANALYSE_usampBuffers.lambdaRange[1]);
  if (ANALYSE_usampBuffers.lambdaRange[2]!=NULL)
   MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","lambdaRange[2]",ANALYSE_usampBuffers.lambdaRange[2]);
  if (ANALYSE_usampBuffers.lambdaRange[3]!=NULL)
   MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","lambdaRange[3]",ANALYSE_usampBuffers.lambdaRange[3]);

  if (ANALYSE_usampBuffers.kuruczConvoluted!=NULL)
   {
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (ANALYSE_usampBuffers.kuruczConvoluted[indexFeno]!=NULL)
      MEMORY_ReleaseDVector("ANALYSE_UsampGlobalFree ","kuruczConvoluted",ANALYSE_usampBuffers.kuruczConvoluted[indexFeno],0);

    MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","kuruczConvoluted",ANALYSE_usampBuffers.kuruczConvoluted);
   }

  if (ANALYSE_usampBuffers.kuruczConvoluted2!=NULL)
   {
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (ANALYSE_usampBuffers.kuruczConvoluted2[indexFeno]!=NULL)
      MEMORY_ReleaseDVector("ANALYSE_UsampGlobalFree ","kuruczConvoluted2",ANALYSE_usampBuffers.kuruczConvoluted2[indexFeno],0);

    MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","kuruczConvoluted2",ANALYSE_usampBuffers.kuruczConvoluted2);
   }

  if (ANALYSE_usampBuffers.kuruczInterpolated!=NULL)
   {
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (ANALYSE_usampBuffers.kuruczInterpolated[indexFeno]!=NULL)
      MEMORY_ReleaseDVector("ANALYSE_UsampGlobalFree ","kuruczInterpolated",ANALYSE_usampBuffers.kuruczInterpolated[indexFeno],0);

    MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","kuruczInterpolated",ANALYSE_usampBuffers.kuruczInterpolated);
   }

  if (ANALYSE_usampBuffers.kuruczInterpolated2!=NULL)
   {
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno]!=NULL)
      MEMORY_ReleaseDVector("ANALYSE_UsampGlobalFree ","kuruczInterpolated2",ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno],0);

    MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","kuruczInterpolated2",ANALYSE_usampBuffers.kuruczInterpolated2);
   }

  memset(&ANALYSE_usampBuffers,0,sizeof(USAMP));
 }

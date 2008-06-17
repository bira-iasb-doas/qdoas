
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  UNDERSAMPLING CORRECTION
//  Name of module    :  USAMP.C
//  Compiler          :  MinGW (GNU compiler)
//  Creation date     :  February 1999
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
//  ============================
//  UNDERSAMPLING CROSS SECTIONS
//  ============================
//
//  UsampBuildCrossSections - build undersampling cross sections from preconvoluted high resolution Kurucz spectrum;
//  UsampBuild - build undersampling cross sections (main function);
//  USAMP_BuildFromAnalysis - build undersampling cross sections during analysis process;
//  UsampWriteHeader - write options in the file header
//  UsampBuildFromTools - build undersampling cross sections from the tools dialog box
//
//  ==================
//  BUFFERS ALLOCATION
//  ==================
//
//  USAMP_GlobalAlloc - allocate buffers (not depending on analysis windows) for the calculation of the undersampling XS
//  USAMP_LocalAlloc - allocate buffers (depending on analysis windows) for the calculation of the undersampling XS
//  USAMP_LocalFree - release the buffers previously allocated by the USAMP_LocalAlloc function
//  USAMP_GlobalFree - release the buffers previously allocated by the USAMP_GlobalAlloc function
//
//  =======================================
//  UNDERSAMPLING PANEL MESSAGES PROCESSING
//  =======================================
//
//  UsampDlgInit - WM_INIT message processing;
//  UsampOK - OK command processing;
//  UsampCommand - WM_COMMAND message processing;
//
//  USAMP_WndProc - dispatch messages from the undersampling tool panel;
//
//  =============================
//  CONFIGURATION FILE MANAGEMENT
//  =============================
//
//  USAMP_ResetConfiguration - reset undersampling options
//  USAMP_LoadConfiguration - load data and input files needed for building undersampling sections;
//  USAMP_SaveConfiguration - save data needed for building undersampling cross sections;
//
// -----------------------------------------------------------------------------
//
//  REFERENCE
//
//  Kelly Chance, from Harvard Smithsonian Institute
//  ----------------------------------------------------------------------------

#include "doas.h"

// =====================
// CONSTANTS DEFINITIONS
// =====================

#define USAMP_SECTION "Undersampling"
#define USAMP_METHOD_MAX 2

// ===================
// STATIC DECLARATIONS
// ===================

DoasCh *usampAnalysisMethod[USAMP_METHOD_MAX]={"Optical density","Intensity fitting"};
USAMP  USAMP_buffers;
FFT    usampFFT;

// =========
// FUNCTIONS
// =========

// -----------------------------------------------------------------------------
// FUNCTION      UsampBuildCrossSections
// -----------------------------------------------------------------------------
// PURPOSE       build undersampling cross sections from preconvoluted high resolution Kurucz spectrum
//
// INPUT/OUTPUT  see below
//
// PRECONDITIONS y2a second derivatives vector should be beforehand computed with SPLINE_deriv2 for cubic interpolation;
//
// RETURN CODE   ERROR_ID_NO            if no error;
//               ERROR_ID_ALLOC         if the allocation of a buffer failed
//               ERROR_ID_BAD_ARGUMENTS if at least one argument is bad;
//               ERROR_ID_SPLINE        if non increasing abscissa found in xa vector;
// -----------------------------------------------------------------------------

RC UsampBuildCrossSections(double *phase1,                                      // OUTPUT : phase 1 calculation
                           double *phase2,                                      // OUTPUT : phase 2 calculation
                           double *gomeLambda,                                  // GOME calibration
                           double *gomeLambda2,                                 // shifted GOME calibration
                           double *kuruczInterpolated,                          // preconvoluted Kurucz spectrum interpolated on gome calibration
                           double *kuruczInterpolatedDeriv2,                    // interpolated Kurucz spectrum second derivatives
                           INT     nGome,                                       // size of GOME calibration
                           double *kuruczLambda,                                // Kurucz high resolution wavelength scale
                           double *kuruczConvolved,                             // preconvoluted Kurucz spectrum on high resolution wavelength scale
                           double *kuruczConvolvedDeriv2,                       // preconvoluted Kurucz second derivatives
                           INT     nKurucz,                                     // size of Kurucz vectors
                           INT     analysisMethod)                              // analysis method
 {
  // Declarations

  double *resample,*d2res,over,under;
  INDEX i,indexMin,indexMax;
  RC rc;

  // Initializations

  d2res=resample=NULL;
  rc=ERROR_ID_NO;

  for (indexMin=0;indexMin<nGome;indexMin++)
   if (gomeLambda2[indexMin]>(double)1.e-3)
    break;

  for (indexMax=nGome-1;indexMax>=0;indexMax--)
   if (gomeLambda2[indexMax]>(double)1.e-3)
    break;

  // Buffers allocation

  if ((nGome<=0) ||
      ((resample=MEMORY_AllocDVector("UsampBuildCrossSections ","resample",0,nGome-1))==NULL) ||
      ((d2res=MEMORY_AllocDVector("UsampBuildCrossSections ","d2res",0,nGome-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Phase 1 : calculate solar spectrum at GOME+phase positions, original and resampled

    if (phase1!=NULL)

     for (i=1,phase1[0]=(double)0.;(i<nGome) && !rc;i++)
      {
       if (gomeLambda2[i]<=(double)1.e-3)
        phase1[i]=(double)0.;
       else if (!(rc=SPLINE_Vector(kuruczLambda,kuruczConvolved,kuruczConvolvedDeriv2,nKurucz,&gomeLambda2[i],&over,1,SPLINE_CUBIC,"UsampBuildCrossSections ")) &&
                !(rc=SPLINE_Vector(gomeLambda,kuruczInterpolated,kuruczInterpolatedDeriv2,nGome,&gomeLambda2[i],&under,1,SPLINE_CUBIC,"UsampBuildCrossSections ")))

        phase1[i]=(analysisMethod==PRJCT_ANLYS_METHOD_SVD)?log(over/under):over-under;
      }

    // Phase 2 : calculate solar spectrum at GOME+phase positions

    if ((phase2!=NULL) &&
       !(rc=SPLINE_Vector(kuruczLambda,kuruczConvolved,kuruczConvolvedDeriv2,nKurucz,gomeLambda2,resample,nGome,SPLINE_CUBIC,"UsampBuildCrossSections ")) && // calculate solar spectrum at GOME positions
       !(rc=SPLINE_Deriv2(&gomeLambda2[indexMin],&resample[indexMin],&d2res[indexMin],(indexMax-indexMin+1),"USAMP_Build (resample 2) ")))

     for (i=0;(i<nGome) && !rc;i++)
      {
       over=kuruczInterpolated[i];

       if (gomeLambda2[i]<(double)1.e-3)
        phase2[i]=(double)0.;
       else if (!(rc=SPLINE_Vector(&gomeLambda2[indexMin],&resample[indexMin],&d2res[indexMin],(indexMax-indexMin+1),&gomeLambda[i],&under,1,SPLINE_CUBIC,"UsampBuildCrossSections ")))
        phase2[i]=(analysisMethod==PRJCT_ANLYS_METHOD_SVD)?log(over/under):over-under;
      }
   }

  // Buffers release

  if (resample!=NULL)
   MEMORY_ReleaseDVector("UsampBuildCrossSections ","resample",resample,0);
  if (d2res!=NULL)
   MEMORY_ReleaseDVector("UsampBuildCrossSections ","d2res",d2res,0);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      UsampBuild
// -----------------------------------------------------------------------------
// PURPOSE       build undersampling cross sections (main function)
//
// INPUT/OUTPUT  see below
//
// RETURN CODE   ERROR_ID_NO            if no error;
//               ERROR_ID_ALLOC         if the allocation of a buffer failed
//               ERROR_ID_BAD_ARGUMENTS if at least one argument is bad;
//               ERROR_ID_SPLINE        if non increasing abscissa found in xa vector;
// -----------------------------------------------------------------------------

RC UsampBuild(double *phase1,                                                   // OUTPUT : phase 1 calculation
              double *phase2,                                                   // OUTPUT : phase 2 calculation
              double *gomeLambda,                                               // GOME calibration
              INT     nGome,                                                    // size of GOME calibration
              double *kuruczLambda,                                             // Kurucz calibration
              double *kuruczSpectrum,                                           // Kurucz spectrum
              double *kuruczDeriv2,                                             // Kurucz second derivatives
              INT     nKurucz,                                                  // size of Kurucz vectors
              SLIT   *pSlit,                                                    // slit function
              double  fraction,                                                 // tunes the phase
              INT     analysisMethod)                                           // analysis method
 {
  // Declarations

  XS      xsnew,xshr,slitFunction;
  double *kuruczConvolved,*kuruczConvolvedDeriv2,*gomeLambda2,slitParam,
         *resample,*d2res;
  INT     slitType;
  INDEX   i;
  RC      rc;

  // Initializations

  kuruczConvolvedDeriv2=gomeLambda2=resample=d2res=NULL;
  memset(&slitFunction,0,sizeof(XS));
  slitType=pSlit->slitType;

  // Buffers allocation

  if (((kuruczConvolved=MEMORY_AllocDVector("UsampBuild ","kuruczConvolved",0,nKurucz))==NULL) ||
      ((kuruczConvolvedDeriv2=MEMORY_AllocDVector("UsampBuild ","kuruczConvolvedDeriv2",0,nKurucz))==NULL) ||
      ((gomeLambda2=MEMORY_AllocDVector("UsampBuild ","gomeLambda2",0,nGome))==NULL) ||
      ((resample=MEMORY_AllocDVector("UsampBuild ","resample",0,nGome))==NULL) ||
      ((d2res=MEMORY_AllocDVector("UsampBuild ","d2res",0,nGome))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    VECTOR_Init(kuruczConvolved,(double)0.,nKurucz);

    xsnew.lambda=kuruczLambda;
    xsnew.vector=kuruczConvolved;
    xsnew.deriv2=kuruczConvolvedDeriv2;
    xsnew.NDET=nKurucz;

    xshr.lambda=kuruczLambda;
    xshr.vector=kuruczSpectrum;
    xshr.deriv2=kuruczDeriv2;
    xshr.NDET=nKurucz;

    for (i=1,gomeLambda2[0]=gomeLambda[0]-fraction;i<nGome;i++)
     gomeLambda2[i]=gomeLambda[i]-fraction;
//     gomeLambda2[i]=(double)(1.-fraction)*gomeLambda[i-1]+fraction*gomeLambda[i];

    // Kurucz spectrum convolution with a gaussian

    if (!(rc=XSCONV_LoadSlitFunction(&slitFunction,pSlit,&slitParam,&slitType)) &&
//        !(rc=XSCONV_TypeStandardFFT(&usampFFT,slitType,pSlit->slitParam,pSlit->slitParam2,kuruczLambda,kuruczConvolved,nKurucz)) &&
        !(rc=XSCONV_TypeStandard(&xsnew,0,nKurucz,&xshr,&slitFunction,&xshr,NULL,slitType,(double)2.*slitParam,slitParam,pSlit->slitParam2,pSlit->slitParam3,pSlit->slitParam4)) &&

        !(rc=SPLINE_Deriv2(kuruczLambda,kuruczConvolved,kuruczConvolvedDeriv2,nKurucz,"UsampBuild (kuruczConvolved) ")) &&
        !(rc=SPLINE_Vector(kuruczLambda,kuruczConvolved,kuruczConvolvedDeriv2,nKurucz,gomeLambda,resample,nGome,SPLINE_CUBIC,"UsampBuild (kuruczConvolved) ")) && // calculate solar spectrum at GOME positions
        !(rc=SPLINE_Deriv2(gomeLambda,resample,d2res,nGome,"UsampBuild (resample 1) ")))

     rc=UsampBuildCrossSections(phase1,                     // OUTPUT : phase 1 calculation
                                phase2,                     // OUTPUT : phase 2 calculation
                                gomeLambda,                 // GOME calibration
                                gomeLambda2,                // shifted GOME calibration
                                resample,                   // preconvoluted Kurucz spectrum interpolated on gome calibration
                                d2res,                      // interpolated Kurucz spectrum second derivatives
                                nGome,                      // size of GOME calibration
                                kuruczLambda,               // Kurucz high resolution wavelength scale
                                kuruczConvolved,            // preconvoluted Kurucz spectrum on high resolution wavelength scale
                                kuruczConvolvedDeriv2,      // preconvoluted Kurucz second derivatives
                                nKurucz,                    // size of Kurucz vectors
                                analysisMethod);            // analysis method
   }

  // Buffers release

  XSCONV_Reset(&slitFunction);

  if (kuruczConvolved!=NULL)
   MEMORY_ReleaseDVector("UsampBuild ","kuruczConvolved",kuruczConvolved,0);
  if (kuruczConvolvedDeriv2!=NULL)
   MEMORY_ReleaseDVector("UsampBuild ","kuruczConvolvedDeriv2",kuruczConvolvedDeriv2,0);
  if (gomeLambda2!=NULL)
   MEMORY_ReleaseDVector("UsampBuild ","gomeLambda2",gomeLambda2,0);

  if (resample!=NULL)
   MEMORY_ReleaseDVector("UsampBuild ","resample",resample,0);
  if (d2res!=NULL)
   MEMORY_ReleaseDVector("UsampBuild ","d2res",d2res,0);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      USAMP_BuildFromAnalysis
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
RC USAMP_BuildFromAnalysis(INT analysisFlag,INT gomeFlag)
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

  if (((lambda=(double *)MEMORY_AllocDVector("USAMP_BuildFromAnalysis ","lambda",0,NDET-1))==NULL) ||
      ((lambda2=(double *)MEMORY_AllocDVector("USAMP_BuildFromAnalysis ","lambda2",0,NDET-1))==NULL))

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
                              &lambda2[indexPixMin],(indexPixMax-indexPixMin+1),pAnalysisOptions->interpol,"USAMP_BuildFromAnalysis ");
          }
        }

       // Local initializations

       xsKurucz.vector=USAMP_buffers.kuruczConvoluted[indexFeno];
       xsKurucz.deriv2=USAMP_buffers.kuruczConvoluted2[indexFeno];
       xsKurucz.lambda=&USAMP_buffers.hrSolar.lambda[USAMP_buffers.lambdaRange[0][indexFeno]];
       xsKurucz.NDET=USAMP_buffers.lambdaRange[1][indexFeno];

       // Not allowed combinations :
       //
       //     - fit slit function with calibration and apply calibration on spec only or on ref and spec;
       //     - fwhmCorrectionFlag and no calibration or fit slit function with the calibration


       if ((((pTabFeno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC) || (pTabFeno->useKurucz==ANLYS_KURUCZ_SPEC)) && pKuruczOptions->fwhmFit) ||
            ((!pTabFeno->useKurucz || pKuruczOptions->fwhmFit) && pSlitOptions->fwhmCorrectionFlag))

        rc=ERROR_SetLast("USAMP_BuildFromAnalysis",ERROR_TYPE_FATAL,ERROR_ID_OPTIONS,"Undersampling ",pTabFeno->windowName);


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

           if (!(rc=XSCONV_TypeStandard(&xsKurucz,0,xsKurucz.NDET,&USAMP_buffers.hrSolar,&xsSlit,&USAMP_buffers.hrSolar,NULL,
                                        pSlitOptions->slitFunction.slitType,(double)2.*pSlitOptions->slitFunction.slitParam,
                                        pSlitOptions->slitFunction.slitParam,pSlitOptions->slitFunction.slitParam2,
                                        pSlitOptions->slitFunction.slitParam3,pSlitOptions->slitFunction.slitParam4)))

            rc=SPLINE_Deriv2(xsKurucz.lambda,xsKurucz.vector,xsKurucz.deriv2,xsKurucz.NDET,"USAMP_BuildFromAnalysis (2) ");
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
                                              &xsKurucz.lambda[i],&slitParam[indexParam],1,pAnalysisOptions->interpol,"USAMP_BuildFromAnalysis "))!=ERROR_ID_NO)
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
                  ((rc=XSCONV_TypeStandard(&xsKurucz,i,i+1,&USAMP_buffers.hrSolar,&slitXs,&USAMP_buffers.hrSolar,NULL,
                                           SLIT_TYPE_FILE,(double)3.*slitParam[0],(double)0.,(double)0.,(double)0.,(double)0.))!=0)))
                break;
              }
            }
           else
            {
             if (slitType!=SLIT_TYPE_GAUSS_FILE)
              for (i=0;i<pTabFeno->NDET;i++)
               slitParam2+=pTabFeno->fwhmVector[1][i];

             if ((xsSlit.deriv2=(double *)MEMORY_AllocDVector("USAMP_BuildFromAnalysis ","deriv2",0,pTabFeno->NDET-1))==NULL)
              rc=ERROR_ID_ALLOC;
             else if (!(rc=SPLINE_Deriv2(xsSlit.lambda,xsSlit.vector,xsSlit.deriv2,xsSlit.NDET,"USAMP_BuildFromAnalysis (Slit) ")))
              rc=XSCONV_TypeStandard(&xsKurucz,0,xsKurucz.NDET,&USAMP_buffers.hrSolar,&xsSlit,&USAMP_buffers.hrSolar,NULL,
                                      slitType,(double)0.,(double)0.,slitParam2/pTabFeno->NDET,(double)0.,(double)0.);

             if (xsSlit.deriv2!=NULL)
              MEMORY_ReleaseDVector("USAMP_BuildFromAnalysis ","deriv2",xsSlit.deriv2,0);
            }
          }

         // Pre-Interpolation on analysis window wavelength scale

         if (!rc &&
            !(rc=SPLINE_Deriv2(xsKurucz.lambda,xsKurucz.vector,xsKurucz.deriv2,xsKurucz.NDET,"USAMP_BuildFromAnalysis (1) ")))
          {
           memcpy(USAMP_buffers.kuruczInterpolated[indexFeno],ANALYSE_zeros,sizeof(double)*pTabFeno->NDET);
           memcpy(USAMP_buffers.kuruczInterpolated2[indexFeno],ANALYSE_zeros,sizeof(double)*pTabFeno->NDET);

           indexPixMin=USAMP_buffers.lambdaRange[2][indexFeno]=FNPixel(lambda,xsKurucz.lambda[0],pTabFeno->NDET,PIXEL_AFTER);
           indexPixMax=FNPixel(lambda,xsKurucz.lambda[xsKurucz.NDET-1],pTabFeno->NDET,PIXEL_BEFORE);
           USAMP_buffers.lambdaRange[3][indexFeno]=indexPixMax-USAMP_buffers.lambdaRange[2][indexFeno]+1;

           if (!(rc=SPLINE_Vector(xsKurucz.lambda,xsKurucz.vector,xsKurucz.deriv2,xsKurucz.NDET,
                                    &lambda[indexPixMin],&USAMP_buffers.kuruczInterpolated[indexFeno][indexPixMin],(indexPixMax-indexPixMin+1),
                                     pAnalysisOptions->interpol,"USAMP_BuildFromAnalysis ")))

            rc=SPLINE_Deriv2(&lambda[indexPixMin],&USAMP_buffers.kuruczInterpolated[indexFeno][indexPixMin],
                     &USAMP_buffers.kuruczInterpolated2[indexFeno][indexPixMin],(indexPixMax-indexPixMin+1),"USAMP_BuildFromAnalysis (3) ");
          }

         // Build undersampling correction

         if (!rc)

          rc=UsampBuildCrossSections
          ((pTabFeno->indexUsamp1!=ITEM_NONE)?&pTabFeno->TabCross[pTabFeno->indexUsamp1].vector[USAMP_buffers.lambdaRange[2][indexFeno]]:NULL,
           (pTabFeno->indexUsamp2!=ITEM_NONE)?&pTabFeno->TabCross[pTabFeno->indexUsamp2].vector[USAMP_buffers.lambdaRange[2][indexFeno]]:NULL,
            &lambda[USAMP_buffers.lambdaRange[2][indexFeno]],
            &lambda2[USAMP_buffers.lambdaRange[2][indexFeno]],
            &USAMP_buffers.kuruczInterpolated[indexFeno][USAMP_buffers.lambdaRange[2][indexFeno]],
            &USAMP_buffers.kuruczInterpolated2[indexFeno][USAMP_buffers.lambdaRange[2][indexFeno]],
             USAMP_buffers.lambdaRange[3][indexFeno],
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
   MEMORY_ReleaseDVector("USAMP_BuildFromAnalysis ","lambda",lambda,0);
  if (lambda2!=NULL)
   MEMORY_ReleaseDVector("USAMP_BuildFromAnalysis ","lambda2",lambda2,0);

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      UsampWriteHeader
// -----------------------------------------------------------------------------
// PURPOSE       Write options in the file header
//
// INPUT         fp    : pointer to the output file
//               phase : phase of the undersampling
// -----------------------------------------------------------------------------

// QDOAS ??? void UsampWriteHeader(FILE *fp,INT phase)
// QDOAS ???  {
// QDOAS ???  	// Declaration
// QDOAS ???
// QDOAS ???   INT slitType;
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   slitType=USAMP_options.slitConv.slitType;
// QDOAS ???
// QDOAS ???   // Header
// QDOAS ???
// QDOAS ???   fprintf(fp,";\n");
// QDOAS ???   fprintf(fp,"; UNDERSAMPLING CROSS SECTION (Phase %d)\n",phase);
// QDOAS ???   fprintf(fp,"; High resolution Kurucz file : %s\n",USAMP_options.kuruczFile);
// QDOAS ???   fprintf(fp,"; Calibration file : %s\n",USAMP_options.calibrationFile);
// QDOAS ???
// QDOAS ???   fprintf(fp,"; Slit function type : %s\n",XSCONV_slitTypes[slitType]);
// QDOAS ???
// QDOAS ???   if ((slitType==SLIT_TYPE_FILE) || (slitType==SLIT_TYPE_GAUSS_FILE) || (slitType==SLIT_TYPE_INVPOLY_FILE) || (slitType==SLIT_TYPE_ERF_FILE))
// QDOAS ???    fprintf(fp,"; Slit function file : %s\n",USAMP_options.slitConv.slitFile);
// QDOAS ???   if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_ERF))
// QDOAS ???    fprintf(fp,"; FWHM : %lf\n",USAMP_options.slitConv.slitParam);
// QDOAS ???   if ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_INVPOLY_FILE))
// QDOAS ???    fprintf(fp,"; Polynomial degree : %d\n",(int)USAMP_options.slitConv.slitParam2);
// QDOAS ???   if ((slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_ERF_FILE))
// QDOAS ???    fprintf(fp,"; Boxcar width : %.3f\n",USAMP_options.slitConv.slitParam2);
// QDOAS ???
// QDOAS ???   if (slitType==SLIT_TYPE_VOIGT)
// QDOAS ???    {
// QDOAS ???     fprintf(fp,"; Gaussian FWHM (L) : %.3f\n",USAMP_options.slitConv.slitParam);
// QDOAS ???     fprintf(fp,"; Gaussian/Lorentz ratio (L) : %.3f\n",USAMP_options.slitConv.slitParam2);
// QDOAS ???     fprintf(fp,"; Gaussian FWHM (R) : %.3f\n",USAMP_options.slitConv.slitParam3);
// QDOAS ???     fprintf(fp,"; Gaussian/Lorentz ratio (R) : %.3f\n",USAMP_options.slitConv.slitParam4);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   fprintf(fp,"; Analysis method : %s\n",usampAnalysisMethod[USAMP_options.analysisMethod]);
// QDOAS ???   fprintf(fp,"; Shift : %g\n",USAMP_options.fraction);
// QDOAS ???
// QDOAS ???   fprintf(fp,";\n");
// QDOAS ???  }

// -----------------------------------------------------------------------------
// FUNCTION      UsampBuildFromTools
// -----------------------------------------------------------------------------
// PURPOSE       build undersampling cross sections from the tools dialog box
//
// INPUT         hwndUsamp : handle of the undersampling dialog box
// -----------------------------------------------------------------------------

// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? void UsampBuildFromTools(HWND hwndUsamp)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   DoasCh fileName1[MAX_ITEM_TEXT_LEN+1],fileName2[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   MATRIX_OBJECT calibrationMatrix,kuruczMatrix;
// QDOAS ???   HCURSOR hcurSave,hcursHourGlass;
// QDOAS ???   double *phase1,*phase2;
// QDOAS ???   INT hrN,fftSize,nSize;
// QDOAS ???   double *fftIn;
// QDOAS ???   FILE *fp1,*fp2;
// QDOAS ???   INDEX i;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   hcursHourGlass=LoadCursor(NULL,IDC_WAIT);
// QDOAS ???   hcurSave=SetCursor(hcursHourGlass);
// QDOAS ???
// QDOAS ???   memset(&calibrationMatrix,0,sizeof(MATRIX_OBJECT));
// QDOAS ???   memset(&kuruczMatrix,0,sizeof(MATRIX_OBJECT));
// QDOAS ???   memset(&usampFFT,0,sizeof(FFT));
// QDOAS ???   phase1=phase2=NULL;
// QDOAS ???   fp1=fp2=NULL;
// QDOAS ???
// QDOAS ???   // Files read out
// QDOAS ???
// QDOAS ???   if (!(rc=MATRIX_Load(USAMP_options.calibrationFile,&calibrationMatrix,0,0,0,0,(double)0.,(double)0.,0,1,"UsampBuildFromTools (calibration file) ")) &&
// QDOAS ???       !(rc=MATRIX_Load(USAMP_options.kuruczFile,&kuruczMatrix,0,0,0,0,(double)calibrationMatrix.matrix[0][0]-7.,
// QDOAS ???        (double)calibrationMatrix.matrix[0][calibrationMatrix.nl-1]+7.,1,1,"UsampBuildFromTools (Kurucz) ")) &&
// QDOAS ???       !(rc=ANALYSE_NormalizeVector(kuruczMatrix.matrix[1]-1,kuruczMatrix.nl,NULL,"UsampBuildFromTools ")))
// QDOAS ???    {
// QDOAS ???     hrN=usampFFT.oldSize=kuruczMatrix.nl;
// QDOAS ???     nSize=calibrationMatrix.nl;
// QDOAS ???     fftSize=usampFFT.fftSize=(int)((double)pow((double)2.,ceil(log((double)hrN)/log((double)2.))));
// QDOAS ???
// QDOAS ???     if (((phase1=MEMORY_AllocDVector("UsampBuildFromTools ","phase 1",0,nSize-1))==NULL) ||
// QDOAS ???         ((phase2=MEMORY_AllocDVector("UsampBuildFromTools ","phase 2",0,nSize-1))==NULL) ||
// QDOAS ???         ((fftIn=usampFFT.fftIn=(double *)MEMORY_AllocDVector("UsampBuildFromTools ","fftIn",1,fftSize))==NULL) ||
// QDOAS ???         ((usampFFT.fftOut=(double *)MEMORY_AllocDVector("UsampBuildFromTools ","fftOut",1,fftSize))==NULL) ||
// QDOAS ???         ((usampFFT.invFftIn=(double *)MEMORY_AllocDVector("UsampBuildFromTools ","invFftIn",1,fftSize))==NULL) ||
// QDOAS ???         ((usampFFT.invFftOut=(double *)MEMORY_AllocDVector("UsampBuildFromTools ","invFftOut",1,fftSize))==NULL))
// QDOAS ???
// QDOAS ???      rc=ERROR_ID_ALLOC;
// QDOAS ???
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???       memcpy(fftIn+1,kuruczMatrix.matrix[1],sizeof(double)*hrN);
// QDOAS ???
// QDOAS ???       for (i=hrN+1;i<=fftSize;i++)
// QDOAS ???        fftIn[i]=fftIn[2*hrN-i];
// QDOAS ???
// QDOAS ???       realft(usampFFT.fftIn,usampFFT.fftOut,fftSize,1);
// QDOAS ???
// QDOAS ???       memcpy(fftIn+1,kuruczMatrix.matrix[0],sizeof(double)*hrN);  // Reuse fftIn for high resolution wavelength safe keeping
// QDOAS ???
// QDOAS ???       rc=UsampBuild(phase1,                                       // OUTPUT : phase 1 calculation
// QDOAS ???                     phase2,                                       // OUTPUT : phase 2 calculation
// QDOAS ???                     calibrationMatrix.matrix[0],                  // GOME calibration
// QDOAS ???                     nSize,                                        // size of GOME calibration
// QDOAS ???                     kuruczMatrix.matrix[0],                       // Kurucz calibration
// QDOAS ???                     kuruczMatrix.matrix[1],                       // Kurucz spectrum
// QDOAS ???                     kuruczMatrix.deriv2[1],                       // Kurucz second derivatives
// QDOAS ???                     kuruczMatrix.nl,                              // size of Kurucz vectors
// QDOAS ???                    &USAMP_options.slitConv,                       // slit function
// QDOAS ???            (double) USAMP_options.fraction,                       // tunes the phase
// QDOAS ???                     USAMP_options.analysisMethod);                // analysis method
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Check if file exist
// QDOAS ???
// QDOAS ???     if (!rc &&
// QDOAS ???     ((((fp1=fopen(FILES_RebuildFileName(fileName1,USAMP_options.path,1),"rt"))!=NULL) &&
// QDOAS ???        (MSG_MessageBox(hwndUsamp,TOOL_USAMP_PHASE1_BUTTON,MENU_WINDOWS_TOOL_USAMP,IDS_MSGBOX_FILE,
// QDOAS ???         MB_YESNO|MB_ICONQUESTION,fileName1)==IDNO)) ||
// QDOAS ???      (((fp2=fopen(FILES_RebuildFileName(fileName2,USAMP_options.path2,1),"rt"))!=NULL) &&
// QDOAS ???        (MSG_MessageBox(hwndUsamp,TOOL_USAMP_PHASE2_BUTTON,MENU_WINDOWS_TOOL_USAMP,IDS_MSGBOX_FILE,
// QDOAS ???         MB_YESNO|MB_ICONQUESTION,fileName2)==IDNO))))
// QDOAS ???
// QDOAS ???      rc=IDS_MSGBOX_FILE;
// QDOAS ???
// QDOAS ???     if (fp1!=NULL)
// QDOAS ???      {
// QDOAS ???       fclose(fp1);
// QDOAS ???       fp1=NULL;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (fp2!=NULL)
// QDOAS ???      {
// QDOAS ???       fclose(fp2);
// QDOAS ???       fp2=NULL;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (!rc)
// QDOAS ???      {
// QDOAS ???       if ((fp1=fopen(fileName1,"w+t"))==NULL)
// QDOAS ???        MSG_MessageBox(hwndUsamp,IDOK,MENU_WINDOWS_TOOL_USAMP,ERROR_ID_FILE_OPEN,MB_OK|MB_ICONHAND,fileName1);
// QDOAS ???       else if ((fp2=fopen(fileName2,"w+t"))==NULL)
// QDOAS ???        MSG_MessageBox(hwndUsamp,IDOK,MENU_WINDOWS_TOOL_USAMP,ERROR_ID_FILE_OPEN,MB_OK|MB_ICONHAND,fileName2);
// QDOAS ???       else
// QDOAS ???        {
// QDOAS ???         if (!USAMP_options.noComment)
// QDOAS ???          {
// QDOAS ???           UsampWriteHeader(fp1,1);
// QDOAS ???           UsampWriteHeader(fp2,2);
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         for (i=0;i<nSize;i++)
// QDOAS ???          {
// QDOAS ???           fprintf(fp1,"%.14le %.14le\n",calibrationMatrix.matrix[0][i],phase1[i]);
// QDOAS ???           fprintf(fp2,"%.14le %.14le\n",calibrationMatrix.matrix[0][i],phase2[i]);
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         DRAW_Spectra(CHILD_WINDOW_SPECTRA,"Undersampling cross sections",
// QDOAS ???                     "","Wavelength (nm)","",NULL,0,
// QDOAS ???                     (double)0.,(double)0.,(double)0.,(double)0.,
// QDOAS ???                      calibrationMatrix.matrix[0],phase1,nSize,DRAW_COLOR1,0,nSize-1,PS_SOLID,"Phase 1",
// QDOAS ???                      calibrationMatrix.matrix[0],phase2,nSize,DRAW_COLOR2,0,nSize-1,PS_SOLID,"Phase 2",
// QDOAS ???                      0,1,1,1);
// QDOAS ???
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      MSG_MessageBox(hwndUsamp,IDOK,MENU_WINDOWS_TOOL_USAMP,ERROR_ID_USAMP,MB_OK|MB_ICONHAND,rc);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Buffers release
// QDOAS ???
// QDOAS ???   if (phase1!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("UsampBuildFromTools ","phase1",phase1,0);
// QDOAS ???   if (phase2!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("UsampBuildFromTools ","phase2",phase2,0);
// QDOAS ???
// QDOAS ???   if (usampFFT.fftIn!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("UsampBuildFromTools ","fftIn",usampFFT.fftIn,1);
// QDOAS ???   if (usampFFT.fftOut!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("UsampBuildFromTools ","fftOut",usampFFT.fftOut,1);
// QDOAS ???   if (usampFFT.invFftIn!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("UsampBuildFromTools ","invFftIn",usampFFT.invFftIn,1);
// QDOAS ???   if (usampFFT.invFftOut!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("UsampBuildFromTools ","invFftOut",usampFFT.invFftOut,1);
// QDOAS ???
// QDOAS ???   MATRIX_Free(&calibrationMatrix,"UsampBuildFromTools");
// QDOAS ???   MATRIX_Free(&kuruczMatrix,"UsampBuildFromTools");
// QDOAS ???
// QDOAS ???   SetCursor(hcurSave);
// QDOAS ???
// QDOAS ???   // Close files
// QDOAS ???
// QDOAS ???   if (fp1!=NULL)
// QDOAS ???    fclose(fp1);
// QDOAS ???   if (fp2!=NULL)
// QDOAS ???    fclose(fp2);
// QDOAS ???  }
// QDOAS ??? #endif

// ==================
// BUFFERS ALLOCATION
// ==================

// -----------------------------------------------------------------------------
// FUNCTION      USAMP_GlobalAlloc
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

RC USAMP_GlobalAlloc(double lambdaMin,double lambdaMax,INT size)
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
      !STD_Stricmp(pKuruczOptions->file,pUsamp->kuruczFile) &&
       KURUCZ_buffers.hrSolar.NDET)
   {
    if ((rc=XSCONV_Alloc(&USAMP_buffers.hrSolar,KURUCZ_buffers.hrSolar.NDET,1))!=ERROR_ID_NO)
     rc=ERROR_ID_ALLOC;
    else
     {
      memcpy(USAMP_buffers.hrSolar.lambda,KURUCZ_buffers.hrSolar.lambda,sizeof(double)*KURUCZ_buffers.hrSolar.NDET);
      memcpy(USAMP_buffers.hrSolar.vector,KURUCZ_buffers.hrSolar.vector,sizeof(double)*KURUCZ_buffers.hrSolar.NDET);
      memcpy(USAMP_buffers.hrSolar.deriv2,KURUCZ_buffers.hrSolar.deriv2,sizeof(double)*KURUCZ_buffers.hrSolar.NDET);
     }
   }
  else
   {
    FILES_RebuildFileName(kuruczFile,pUsamp->kuruczFile,1);

    if (!(rc=XSCONV_LoadCrossSectionFile(&USAMP_buffers.hrSolar,kuruczFile,lambdaMin-7.,lambdaMax+7.,(double)0.,CONVOLUTION_CONVERSION_NONE)))
     rc=ANALYSE_NormalizeVector(USAMP_buffers.hrSolar.vector-1,USAMP_buffers.hrSolar.NDET,NULL,"USAMP_GlobalAlloc ");
   }

  if (rc!=ERROR_ID_NO)
   rc=ERROR_SetLast("USAMP_GlobalAlloc",ERROR_TYPE_FATAL,ERROR_ID_USAMP,"USAMP_GlobalAlloc (0)");

  // Buffers allocation

  else if (((USAMP_buffers.lambdaRange[0]=(INT *)MEMORY_AllocBuffer("USAMP_GlobalAlloc ","lambdaRange[0]",NFeno,sizeof(INT),0,MEMORY_TYPE_INT))==NULL) ||
           ((USAMP_buffers.lambdaRange[1]=(INT *)MEMORY_AllocBuffer("USAMP_GlobalAlloc ","lambdaRange[1]",NFeno,sizeof(INT),0,MEMORY_TYPE_INT))==NULL) ||
           ((USAMP_buffers.lambdaRange[2]=(INT *)MEMORY_AllocBuffer("USAMP_GlobalAlloc ","lambdaRange[2]",NFeno,sizeof(INT),0,MEMORY_TYPE_INT))==NULL) ||
           ((USAMP_buffers.lambdaRange[3]=(INT *)MEMORY_AllocBuffer("USAMP_GlobalAlloc ","lambdaRange[3]",NFeno,sizeof(INT),0,MEMORY_TYPE_INT))==NULL) ||
           ((USAMP_buffers.kuruczConvoluted=(double **)MEMORY_AllocBuffer("USAMP_GlobalAlloc ","kuruczConvoluted",NFeno,sizeof(double *),0,MEMORY_TYPE_DOUBLE))==NULL) ||
           ((USAMP_buffers.kuruczConvoluted2=(double **)MEMORY_AllocBuffer("USAMP_GlobalAlloc ","kuruczConvoluted2",NFeno,sizeof(double *),0,MEMORY_TYPE_DOUBLE))==NULL) ||
           ((USAMP_buffers.kuruczInterpolated=(double **)MEMORY_AllocBuffer("USAMP_GlobalAlloc ","kuruczInterpolated",NFeno,sizeof(double *),0,MEMORY_TYPE_DOUBLE))==NULL) ||
           ((USAMP_buffers.kuruczInterpolated2=(double **)MEMORY_AllocBuffer("USAMP_GlobalAlloc ","kuruczInterpolated2",NFeno,sizeof(double *),0,MEMORY_TYPE_DOUBLE))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
     {
      pTabFeno=&TabFeno[indexFeno];

      USAMP_buffers.lambdaRange[0][indexFeno]=
      USAMP_buffers.lambdaRange[1][indexFeno]=
      USAMP_buffers.lambdaRange[2][indexFeno]=
      USAMP_buffers.lambdaRange[3][indexFeno]=ITEM_NONE;

      USAMP_buffers.kuruczConvoluted[indexFeno]=
      USAMP_buffers.kuruczConvoluted2[indexFeno]=
      USAMP_buffers.kuruczInterpolated[indexFeno]=
      USAMP_buffers.kuruczInterpolated2[indexFeno]=NULL;

      if (!pTabFeno->hidden && pTabFeno->useUsamp &&
        (((USAMP_buffers.kuruczInterpolated[indexFeno]=(double *)MEMORY_AllocDVector("USAMP_GlobalAlloc ","kuruczInterpolated",0,size-1))==NULL) ||
         ((USAMP_buffers.kuruczInterpolated2[indexFeno]=(double *)MEMORY_AllocDVector("USAMP_GlobalAlloc ","kuruczInterpolated2",0,size-1))==NULL)))

       rc=ERROR_ID_ALLOC;

      else
       USAMP_buffers.lambdaRange[0][indexFeno]=USAMP_buffers.lambdaRange[1][indexFeno]=ITEM_NONE;
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      USAMP_LocalAlloc
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
RC USAMP_LocalAlloc(INT gomeFlag)
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
      USAMP_buffers.lambdaRange[0][indexFeno]=FNPixel(USAMP_buffers.hrSolar.lambda,(double)pTabFeno->svd.LFenetre[0][0]-7.,USAMP_buffers.hrSolar.NDET,PIXEL_AFTER);
      endPixel=FNPixel(USAMP_buffers.hrSolar.lambda,(double)pTabFeno->svd.LFenetre[pTabFeno->svd.Z-1][1]+7.,USAMP_buffers.hrSolar.NDET,PIXEL_BEFORE);

      lambdaSize=USAMP_buffers.lambdaRange[1][indexFeno]=endPixel-USAMP_buffers.lambdaRange[0][indexFeno]+1;

      if (((USAMP_buffers.kuruczConvoluted[indexFeno]=(double *)MEMORY_AllocDVector("USAMP_LocalAlloc ","kuruczConvoluted",0,lambdaSize-1))==NULL) ||
          ((USAMP_buffers.kuruczConvoluted2[indexFeno]=(double *)MEMORY_AllocDVector("USAMP_LocalAlloc ","kuruczConvoluted2",0,lambdaSize-1))==NULL))

       rc=ERROR_ID_ALLOC;
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      USAMP_LocalFree
// -----------------------------------------------------------------------------
// PURPOSE       release the buffers previously allocated by the USAMP_LocalAlloc function
// -----------------------------------------------------------------------------

void USAMP_LocalFree(void)
 {
  // Declarations

  INDEX indexFeno;

  // Release allocated buffers

  if (USAMP_buffers.kuruczConvoluted!=NULL)
   {
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (!TabFeno[indexFeno].hidden && TabFeno[indexFeno].useUsamp && !TabFeno[indexFeno].gomeRefFlag)
      {
       if (USAMP_buffers.kuruczConvoluted[indexFeno]!=NULL)
        {
         MEMORY_ReleaseDVector("USAMP_LocalFree ","kuruczConvoluted",USAMP_buffers.kuruczConvoluted[indexFeno],0);
         USAMP_buffers.kuruczConvoluted[indexFeno]=NULL;
        }

       if (USAMP_buffers.kuruczConvoluted2[indexFeno]!=NULL)
        {
         MEMORY_ReleaseDVector("USAMP_LocalFree ","kuruczConvoluted2",USAMP_buffers.kuruczConvoluted2[indexFeno],0);
         USAMP_buffers.kuruczConvoluted2[indexFeno]=NULL;
        }
      }
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      USAMP_GlobalFree
// -----------------------------------------------------------------------------
// PURPOSE       release the buffers previously allocated by the USAMP_GlobalAlloc function
// -----------------------------------------------------------------------------

void USAMP_GlobalFree(void)
 {
  INDEX indexFeno;

  XSCONV_Reset(&USAMP_buffers.hrSolar);

  if (USAMP_buffers.lambdaRange[0]!=NULL)
   MEMORY_ReleaseBuffer("USAMP_GlobalFree ","lambdaRange[0]",USAMP_buffers.lambdaRange[0]);
  if (USAMP_buffers.lambdaRange[1]!=NULL)
   MEMORY_ReleaseBuffer("USAMP_GlobalFree ","lambdaRange[1]",USAMP_buffers.lambdaRange[1]);
  if (USAMP_buffers.lambdaRange[2]!=NULL)
   MEMORY_ReleaseBuffer("USAMP_GlobalFree ","lambdaRange[2]",USAMP_buffers.lambdaRange[2]);
  if (USAMP_buffers.lambdaRange[3]!=NULL)
   MEMORY_ReleaseBuffer("USAMP_GlobalFree ","lambdaRange[3]",USAMP_buffers.lambdaRange[3]);

  if (USAMP_buffers.kuruczConvoluted!=NULL)
   {
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (USAMP_buffers.kuruczConvoluted[indexFeno]!=NULL)
      MEMORY_ReleaseDVector("USAMP_GlobalFree ","kuruczConvoluted",USAMP_buffers.kuruczConvoluted[indexFeno],0);

    MEMORY_ReleaseBuffer("USAMP_GlobalFree ","kuruczConvoluted",USAMP_buffers.kuruczConvoluted);
   }

  if (USAMP_buffers.kuruczConvoluted2!=NULL)
   {
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (USAMP_buffers.kuruczConvoluted2[indexFeno]!=NULL)
      MEMORY_ReleaseDVector("USAMP_GlobalFree ","kuruczConvoluted2",USAMP_buffers.kuruczConvoluted2[indexFeno],0);

    MEMORY_ReleaseBuffer("USAMP_GlobalFree ","kuruczConvoluted2",USAMP_buffers.kuruczConvoluted2);
   }

  if (USAMP_buffers.kuruczInterpolated!=NULL)
   {
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (USAMP_buffers.kuruczInterpolated[indexFeno]!=NULL)
      MEMORY_ReleaseDVector("USAMP_GlobalFree ","kuruczInterpolated",USAMP_buffers.kuruczInterpolated[indexFeno],0);

    MEMORY_ReleaseBuffer("USAMP_GlobalFree ","kuruczInterpolated",USAMP_buffers.kuruczInterpolated);
   }

  if (USAMP_buffers.kuruczInterpolated2!=NULL)
   {
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (USAMP_buffers.kuruczInterpolated2[indexFeno]!=NULL)
      MEMORY_ReleaseDVector("USAMP_GlobalFree ","kuruczInterpolated2",USAMP_buffers.kuruczInterpolated2[indexFeno],0);

    MEMORY_ReleaseBuffer("USAMP_GlobalFree ","kuruczInterpolated2",USAMP_buffers.kuruczInterpolated2);
   }

  memset(&USAMP_buffers,0,sizeof(USAMP));
 }

// QDOAS ??? // =======================================
// QDOAS ??? // UNDERSAMPLING PANEL MESSAGES PROCESSING
// QDOAS ??? // =======================================
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        UsampDlgInit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         WM_INIT message processing
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndUsamp : handle of the undersampling dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void UsampDlgInit(HWND hwndUsamp)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   DoasCh string[20];
// QDOAS ???   HWND hwndConv,hwndMethod;
// QDOAS ???   INDEX indexItem;
// QDOAS ???   SLIT *pSlit;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   hwndConv=GetDlgItem(hwndUsamp,TOOL_USAMP_SLIT_TYPE);
// QDOAS ???   hwndMethod=GetDlgItem(hwndUsamp,TOOL_USAMP_ANALYSIS_METHOD);
// QDOAS ???   memcpy(&USAMP_buffer,&USAMP_options,sizeof(XSCONV));
// QDOAS ???   pSlit=&USAMP_buffer.slitConv;
// QDOAS ???
// QDOAS ???   // Center on parent window
// QDOAS ???
// QDOAS ???   DOAS_CenterWindow(hwndUsamp,GetWindow(hwndUsamp,GW_OWNER));
// QDOAS ???
// QDOAS ???   // Rebuild files names
// QDOAS ???
// QDOAS ???   FILES_RebuildFileName(USAMP_buffer.calibrationFile,USAMP_options.calibrationFile,1);
// QDOAS ???   FILES_RebuildFileName(USAMP_buffer.path,USAMP_options.path,1);
// QDOAS ???   FILES_RebuildFileName(USAMP_buffer.path2,USAMP_options.path2,1);
// QDOAS ???   FILES_RebuildFileName(USAMP_buffer.kuruczFile,USAMP_options.kuruczFile,1);
// QDOAS ???   FILES_RebuildFileName(USAMP_buffer.slitConv.slitFile,USAMP_options.slitConv.slitFile,1);
// QDOAS ???
// QDOAS ???   // Fill edit controls
// QDOAS ???
// QDOAS ???   SetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_PHASE1_FILE),USAMP_buffer.path);
// QDOAS ???   SetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_PHASE2_FILE),USAMP_buffer.path2);
// QDOAS ???   SetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_SOLAR_FILE),USAMP_buffer.kuruczFile);
// QDOAS ???   SetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_CALIBRATION_FILE),USAMP_buffer.calibrationFile);
// QDOAS ???
// QDOAS ???   SetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_SLIT_FILE),pSlit->slitFile);
// QDOAS ???   sprintf(string,"%g",pSlit->slitParam);
// QDOAS ???   SetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_GAUSS_WIDTH),string);
// QDOAS ???   sprintf(string,"%g",pSlit->slitParam2);
// QDOAS ???   SetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_INVPOLY),string);
// QDOAS ???   sprintf(string,"%g",pSlit->slitParam3);
// QDOAS ???   SetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_VOIGT_RGAUSS),string);
// QDOAS ???   sprintf(string,"%g",pSlit->slitParam4);
// QDOAS ???   SetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_VOIGT_RRATIO),string);
// QDOAS ???   sprintf(string,"%6.3f",USAMP_buffer.fraction);
// QDOAS ???   SetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_ANALYSIS_PHASE),string);
// QDOAS ???
// QDOAS ???   // Fill comboboxes
// QDOAS ???
// QDOAS ???   for (indexItem=0;indexItem<SLIT_TYPE_GAUSS_T_FILE;indexItem++)
// QDOAS ???    SendMessage(hwndConv,CB_ADDSTRING,0,(LPARAM)XSCONV_slitTypes[indexItem]);
// QDOAS ???   for (indexItem=0;indexItem<USAMP_METHOD_MAX;indexItem++)
// QDOAS ???    SendMessage(hwndMethod,CB_ADDSTRING,0,(LPARAM)usampAnalysisMethod[indexItem]);
// QDOAS ???
// QDOAS ???   SendMessage(hwndConv,CB_SETCURSEL,(WPARAM)pSlit->slitType,0);
// QDOAS ???   SendMessage(hwndMethod,CB_SETCURSEL,(WPARAM)USAMP_buffer.analysisMethod,0);
// QDOAS ???
// QDOAS ???   // Show/Hide File/Gauss fields
// QDOAS ???
// QDOAS ???   XSCONV_SlitType(hwndUsamp,TOOL_USAMP_SLIT_TYPE,pSlit,&USAMP_buffer.slitDConv);
// QDOAS ???
// QDOAS ???   // Check boxes
// QDOAS ???
// QDOAS ???   CheckDlgButton(hwndUsamp,TOOL_USAMP_NOCOMMENT,(USAMP_buffer.noComment!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        UsampOK
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         OK command processing
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndUsamp : handle of the undersampling dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void UsampOK(HWND hwndUsamp)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   DoasCh string[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   SLIT *pSlit;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pSlit=&USAMP_buffer.slitConv;
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Fields read out
// QDOAS ???
// QDOAS ???   memset(string,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???   pSlit->slitType=(DoasCh)SendMessage(GetDlgItem(hwndUsamp,TOOL_USAMP_SLIT_TYPE),CB_GETCURSEL,0,0);
// QDOAS ???   USAMP_buffer.analysisMethod=(DoasCh)SendMessage(GetDlgItem(hwndUsamp,TOOL_USAMP_ANALYSIS_METHOD),CB_GETCURSEL,0,0);
// QDOAS ???
// QDOAS ???   GetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_PHASE1_FILE),USAMP_buffer.path,MAX_ITEM_TEXT_LEN);
// QDOAS ???   GetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_PHASE2_FILE),USAMP_buffer.path2,MAX_ITEM_TEXT_LEN);
// QDOAS ???   GetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_SOLAR_FILE),USAMP_buffer.kuruczFile,MAX_ITEM_TEXT_LEN);
// QDOAS ???   GetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_CALIBRATION_FILE),USAMP_buffer.calibrationFile,MAX_ITEM_TEXT_LEN);
// QDOAS ???
// QDOAS ???   GetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_GAUSS_WIDTH),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   pSlit->slitParam=(double)fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_INVPOLY),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   pSlit->slitParam2=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_VOIGT_RGAUSS),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   pSlit->slitParam3=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_VOIGT_RRATIO),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   pSlit->slitParam4=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_ANALYSIS_PHASE),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   USAMP_buffer.fraction=atof(string);
// QDOAS ???   GetWindowText(GetDlgItem(hwndUsamp,TOOL_USAMP_SLIT_FILE),pSlit->slitFile,MAX_ITEM_TEXT_LEN);
// QDOAS ???
// QDOAS ???   USAMP_buffer.noComment=(DoasCh)(IsDlgButtonChecked(hwndUsamp,TOOL_USAMP_NOCOMMENT)==BST_CHECKED)?(DoasCh)1:(DoasCh)0;
// QDOAS ???
// QDOAS ???   // Check validity of fields
// QDOAS ???
// QDOAS ???   if (!strlen(USAMP_buffer.path))
// QDOAS ???    MSG_MessageBox(hwndUsamp,TOOL_USAMP_PHASE1_BUTTON,MENU_WINDOWS_TOOL_USAMP,(rc=IDS_MSGBOX_FIELDEMPTY),
// QDOAS ???                    MB_OK|MB_ICONHAND,"Output File (Phase 1)");
// QDOAS ???
// QDOAS ???   if (!strlen(USAMP_buffer.path2))
// QDOAS ???    MSG_MessageBox(hwndUsamp,TOOL_USAMP_PHASE2_BUTTON,MENU_WINDOWS_TOOL_USAMP,(rc=IDS_MSGBOX_FIELDEMPTY),
// QDOAS ???                    MB_OK|MB_ICONHAND,"Output File (Phase 2)");
// QDOAS ???
// QDOAS ???   else if (!strlen(USAMP_buffer.kuruczFile))
// QDOAS ???    MSG_MessageBox(hwndUsamp,TOOL_USAMP_SOLAR_BUTTON,MENU_WINDOWS_TOOL_USAMP,(rc=IDS_MSGBOX_FIELDEMPTY),
// QDOAS ???                    MB_OK|MB_ICONHAND,"Solar File");
// QDOAS ???
// QDOAS ???   else if (!strlen(USAMP_buffer.calibrationFile))
// QDOAS ???    MSG_MessageBox(hwndUsamp,TOOL_USAMP_CALIBRATION_BUTTON,MENU_WINDOWS_TOOL_USAMP,(rc=IDS_MSGBOX_FIELDEMPTY),
// QDOAS ???                    MB_OK|MB_ICONHAND,"Calibration File");
// QDOAS ???
// QDOAS ???   else if (((pSlit->slitType==SLIT_TYPE_FILE) || (pSlit->slitType==SLIT_TYPE_GAUSS_FILE) || (pSlit->slitType==SLIT_TYPE_INVPOLY_FILE)) &&
// QDOAS ???             !strlen(pSlit->slitFile))
// QDOAS ???    MSG_MessageBox(hwndUsamp,TOOL_USAMP_SLIT_BUTTON,MENU_WINDOWS_TOOL_USAMP,(rc=IDS_MSGBOX_FIELDEMPTY),
// QDOAS ???                    MB_OK|MB_ICONHAND,"Slit Function");
// QDOAS ???
// QDOAS ???   if (!rc)
// QDOAS ???    {
// QDOAS ???     if (!TLBAR_bSaveFlag)
// QDOAS ???      TLBAR_Enable(TRUE);
// QDOAS ???
// QDOAS ???     memcpy(&USAMP_options,&USAMP_buffer,sizeof(XSCONV));
// QDOAS ???
// QDOAS ???     FILES_ChangePath(USAMP_options.calibrationFile,USAMP_buffer.calibrationFile,1);
// QDOAS ???     FILES_ChangePath(USAMP_options.path,USAMP_buffer.path,1);
// QDOAS ???     FILES_ChangePath(USAMP_options.path2,USAMP_buffer.path2,1);
// QDOAS ???     FILES_ChangePath(USAMP_options.kuruczFile,USAMP_buffer.kuruczFile,1);
// QDOAS ???     FILES_ChangePath(USAMP_options.slitConv.slitFile,USAMP_buffer.slitConv.slitFile,1);
// QDOAS ???
// QDOAS ???     UsampBuildFromTools(hwndUsamp);
// QDOAS ???     EndDialog(hwndUsamp,TRUE);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        UsampCommand
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         WM_COMMAND command processing
// QDOAS ??? //
// QDOAS ??? // SYNTAX          usual syntax for Windows message processing;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? LRESULT CALLBACK UsampCommand(HWND hwndUsamp,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch((ULONG)GET_WM_COMMAND_ID(mp1,mp2))
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_USAMP_PHASE1_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndUsamp,USAMP_buffer.path,FILE_TYPE_ALL,FILE_MODE_SAVE,TOOL_USAMP_PHASE1_FILE,1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_USAMP_PHASE2_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndUsamp,USAMP_buffer.path2,FILE_TYPE_ALL,FILE_MODE_SAVE,TOOL_USAMP_PHASE2_FILE,1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_USAMP_SOLAR_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndUsamp,USAMP_buffer.kuruczFile,FILE_TYPE_CALIB_KURUCZ,FILE_MODE_OPEN,TOOL_USAMP_SOLAR_FILE,1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_USAMP_CALIBRATION_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndUsamp,USAMP_buffer.calibrationFile,FILE_TYPE_CALIB,FILE_MODE_OPEN,TOOL_USAMP_CALIBRATION_FILE,1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_USAMP_SLIT_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndUsamp,USAMP_buffer.slitConv.slitFile,FILE_TYPE_SLIT,FILE_MODE_OPEN,TOOL_USAMP_SLIT_FILE,1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_USAMP_SLIT_TYPE :
// QDOAS ???      XSCONV_SlitType(hwndUsamp,TOOL_USAMP_SLIT_TYPE,&USAMP_buffer.slitConv,&USAMP_buffer.slitDConv);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDOK :
// QDOAS ???      UsampOK(hwndUsamp);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDCANCEL :
// QDOAS ???      EndDialog(hwndUsamp,FALSE);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDHELP :
// QDOAS ???      WinHelp(hwndUsamp,DOAS_HelpPath,HELP_CONTEXT,HLP_TOOLS_USAMP_INTRO);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        USAMP_WndProc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         dispatch messages from undersampling tool panel
// QDOAS ??? //
// QDOAS ??? // SYNTAX          usual syntax for Windows message processing;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? LRESULT CALLBACK USAMP_WndProc(HWND hwndUsamp,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch (msg)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_INITDIALOG :
// QDOAS ???      UsampDlgInit(hwndUsamp);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_COMMAND :
// QDOAS ???      UsampCommand(hwndUsamp,msg,mp1,mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_HELP :
// QDOAS ???      WinHelp(hwndUsamp,DOAS_HelpPath,HELP_CONTEXT,HLP_TOOLS_USAMP_INTRO);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // =============================
// QDOAS ??? // CONFIGURATION FILE MANAGEMENT
// QDOAS ??? // =============================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      USAMP_ResetConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       reset undersampling options
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void USAMP_ResetConfiguration(void)
// QDOAS ???  {
// QDOAS ???   memset(&USAMP_options,0,sizeof(XSCONV));
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      USAMP_LoadConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       load data and input files needed for building undersampling cross sections
// QDOAS ??? //
// QDOAS ??? // INPUT         fileLine : the line to process
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC USAMP_LoadConfiguration(DoasCh *fileLine)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   DoasCh keyName[MAX_ITEM_NAME_LEN+1],     // key name for first part of project information
// QDOAS ???         text[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   RC    rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   memset(keyName,0,MAX_ITEM_NAME_LEN+1);
// QDOAS ???   memset(text,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???   rc=0;
// QDOAS ???
// QDOAS ???   if ((sscanf(fileLine,"%[^'=']=%[^'\n']",keyName,text)>=2) && !STD_Stricmp(keyName,USAMP_SECTION))
// QDOAS ???    {
// QDOAS ???     memset(&USAMP_options,0,sizeof(XSCONV));
// QDOAS ???     rc=1;
// QDOAS ???
// QDOAS ???     if (sscanf(text,"%[^','],%[^','],%[^','],%d,%lf,%lf,%[^','],%[^','],%d,%f,%d,%lf,%lf",
// QDOAS ???             (DoasCh *) USAMP_options.path,
// QDOAS ???             (DoasCh *) USAMP_options.path2,
// QDOAS ???             (DoasCh *) USAMP_options.calibrationFile,
// QDOAS ???               (INT *)&USAMP_options.slitConv.slitType,
// QDOAS ???            (double *)&USAMP_options.slitConv.slitParam,
// QDOAS ???            (double *)&USAMP_options.slitConv.slitParam2,
// QDOAS ???             (DoasCh *) USAMP_options.slitConv.slitFile,
// QDOAS ???             (DoasCh *)&USAMP_options.kuruczFile,
// QDOAS ???               (INT *)&USAMP_options.analysisMethod,
// QDOAS ???             (float *)&USAMP_options.fraction,
// QDOAS ???               (INT *)&USAMP_options.noComment,
// QDOAS ???            (double *)&USAMP_options.slitConv.slitParam3,
// QDOAS ???            (double *)&USAMP_options.slitConv.slitParam4)>=1)
// QDOAS ???      {
// QDOAS ???       FILES_CompactPath(USAMP_options.path,USAMP_options.path,1,1);
// QDOAS ???       FILES_CompactPath(USAMP_options.path2,USAMP_options.path2,1,1);
// QDOAS ???       FILES_CompactPath(USAMP_options.calibrationFile,USAMP_options.calibrationFile,1,1);
// QDOAS ???       FILES_CompactPath(USAMP_options.kuruczFile,USAMP_options.kuruczFile,1,1);
// QDOAS ???       FILES_CompactPath(USAMP_options.slitConv.slitFile,USAMP_options.slitConv.slitFile,1,1);
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      USAMP_SaveConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       save data needed for building undersampling cross sections
// QDOAS ??? //
// QDOAS ??? // INPUT         fp          : pointer to the current configuration file;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void USAMP_SaveConfiguration(FILE *fp)
// QDOAS ???  {
// QDOAS ???   fprintf(fp,"%s=%s,%s,%s,%d,%lf,%lf,%s,%s,%d,%f,%d,%.3f,%.3f\n",
// QDOAS ???               USAMP_SECTION,
// QDOAS ???               USAMP_options.path,
// QDOAS ???               USAMP_options.path2,
// QDOAS ???               USAMP_options.calibrationFile,
// QDOAS ???         (INT) USAMP_options.slitConv.slitType,
// QDOAS ???               USAMP_options.slitConv.slitParam,
// QDOAS ???               USAMP_options.slitConv.slitParam2,
// QDOAS ???               USAMP_options.slitConv.slitFile,
// QDOAS ???               USAMP_options.kuruczFile,
// QDOAS ???         (INT) USAMP_options.analysisMethod,
// QDOAS ???       (float) USAMP_options.fraction,
// QDOAS ???         (INT) USAMP_options.noComment,
// QDOAS ???               USAMP_options.slitConv.slitParam3,
// QDOAS ???               USAMP_options.slitConv.slitParam4);
// QDOAS ???  }

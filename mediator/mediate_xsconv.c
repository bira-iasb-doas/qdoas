
/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "../mediator/mediate_response.h"
#include "../mediator/mediate_types.h"
#include "../mediator/mediate_xsconv.h"

// ================
// STATIC VARIABLES
// ================

static UCHAR *mediateConvolutionTypesStr[CONVOLUTION_TYPE_MAX]=
 {
  "Interpolation only",
  "Standard convolution",
  "Convolution with I0 correction"
 };

static UCHAR *mediateConvolutionFileExt[CONVOLUTION_TYPE_MAX]=
   {
    "_none",                                                                      // CONVOLUTION_TYPE_NONE
    "_std",                                                                       // CONVOLUTION_TYPE_STANDARD
    "_i0",                                                                        // CONVOLUTION_TYPE_I0_CORRECTION
  //  "_ring"                         // CONVOLUTION_TYPE_RING
   };

UCHAR *mediateConvolutionFilterTypes[PRJCT_FILTER_TYPE_MAX]={"None","Kaiser","Boxcar","Gaussian","Triangular","Savitzky-Golay","Odd-even pixels correction","Binomial"};

// -------------------------------------------------
// mediateConvolutionSave : Save the convoluted cross section
// -------------------------------------------------

RC mediateConvolutionSave(void *engineContext)
 {
  // Declarations

  ENGINE_XSCONV_CONTEXT *pEngineContext=(ENGINE_XSCONV_CONTEXT*)engineContext;
  UCHAR fileName[MAX_ITEM_TEXT_LEN+1];
  PRJCT_FILTER *pLFilter,*pHFilter;
  SZ_LEN fileNameLength;
  UCHAR *ptr,*ptr2;
  FILE *fp;
  INDEX i,slitType;
  XS *pXs;
  INT nextraPixels;
  double *filterVector;
  INT nsize;
  RC rc;

  pXs=&pEngineContext->xsNew;
  nextraPixels=pEngineContext->nFilter;
  filterVector=pEngineContext->filterVector;

  pLFilter=&pEngineContext->lfilter;
  pHFilter=&pEngineContext->hfilter;
  nsize=pXs->NDET-nextraPixels;

  // Initializations

  strcpy(fileName,pEngineContext->path);
  fp=NULL;
  rc=ERROR_ID_NO;

  if (((ptr=strrchr(fileName,PATH_SEP))!=NULL) && !strlen(ptr+1))
   {
    // Concatenate file name to output path

    if ((ptr2=strrchr(pEngineContext->crossFile,PATH_SEP))!=NULL)
     ptr2++;
    else
     ptr2=pEngineContext->crossFile;

    sprintf(ptr,"%c%s",PATH_SEP,ptr2);

    // Replace file extension by the correct one

    if ((fileNameLength=strlen(fileName))!=0)
     {
      if ((ptr=strrchr(fileName,'.'))==NULL)
       {
        fileName[fileNameLength++]='.';
        fileName[fileNameLength]=0;
       }

      strcat(fileName,mediateConvolutionFileExt[pEngineContext->convolutionType]);
     }
   }

  // Save file

  if (!rc)
   {
    if ((fp=fopen(fileName,"w+t"))!=NULL)
     {
      if (!pEngineContext->noComment)
       {
        // Header

        fprintf(fp,";\n");
        fprintf(fp,"; High resolution cross section file : %s\n",pEngineContext->crossFile);
        fprintf(fp,"; Calibration file : %s\n",pEngineContext->calibrationFile);
        fprintf(fp,"; Shift applied : %g nm\n",pEngineContext->shift);
        fprintf(fp,"; Convolution type : %s\n",mediateConvolutionTypesStr[pEngineContext->convolutionType]);

        if (pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE)
         {
          if (pEngineContext->convolutionType==CONVOLUTION_TYPE_I0_CORRECTION)
           {
            fprintf(fp,"; Kurucz file : %s\n",pEngineContext->kuruczFile);
            fprintf(fp,"; Concentration in I0 (mol/cm**2) : %g\n",pEngineContext->conc);
           }

          fprintf(fp,"; Slit function type : %s\n",XSCONV_slitTypes[(slitType=pEngineContext->slitConv.slitType)]);

          if (((slitType=pEngineContext->slitConv.slitType)==SLIT_TYPE_FILE) ||
               (slitType==SLIT_TYPE_GAUSS_FILE) ||
               (slitType==SLIT_TYPE_INVPOLY_FILE) ||
//               (slitType==SLIT_TYPE_VOIGT_FILE) ||
               (slitType==SLIT_TYPE_ERF_FILE))
           fprintf(fp,"; Slit function file : %s\n",pEngineContext->slitConv.slitFile);
          if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_ERF))
           fprintf(fp,"; Gaussian FWHM : %.3f\n",pEngineContext->slitConv.slitParam);
          if ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_INVPOLY_FILE))
           fprintf(fp,"; Polynomial degree : %d\n",(int)pEngineContext->slitConv.slitParam2);
          if ((slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_ERF_FILE))
           fprintf(fp,"; Boxcar width : %.3f\n",pEngineContext->slitConv.slitParam2);

          if ((slitType== SLIT_TYPE_APOD) || (slitType== SLIT_TYPE_APODNBS))
           {
            fprintf(fp,"; Resolution : %.3lf\n",pEngineContext->slitConv.slitParam);
            fprintf(fp,"; Phase      : %.3lf\n",pEngineContext->slitConv.slitParam2);
           }

          if (slitType==SLIT_TYPE_VOIGT)
           {
            fprintf(fp,"; Gaussian FWHM (L) : %.3f\n",pEngineContext->slitConv.slitParam);
            fprintf(fp,"; Lorentz/Gauss ratio (L) : %.3f\n",pEngineContext->slitConv.slitParam2);
            fprintf(fp,"; Gaussian FWHM (R) : %.3f\n",pEngineContext->slitConv.slitParam3);
            fprintf(fp,"; Lorentz/Gauss ratio (R) : %.3f\n",pEngineContext->slitConv.slitParam4);
           }

          if (((slitType=pEngineContext->slitDConv.slitType)!=SLIT_TYPE_FILE) || (strlen(pEngineContext->slitDConv.slitFile)!=0))
           fprintf(fp,"; Deconvolution slit function type : %s",XSCONV_slitTypes[slitType]);
          if (((slitType==SLIT_TYPE_FILE) && strlen(pEngineContext->slitDConv.slitFile)) ||
               (slitType==SLIT_TYPE_GAUSS_FILE) ||
               (slitType==SLIT_TYPE_INVPOLY_FILE) ||
//               (slitType==SLIT_TYPE_VOIGT_FILE) ||
               (slitType==SLIT_TYPE_ERF_FILE))
           fprintf(fp,"; Deconvolution slit function file : %s\n",pEngineContext->slitDConv.slitFile);
          if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_ERF))
           fprintf(fp,"; Deconvolution slit function gaussian FWHM : %lf\n",pEngineContext->slitDConv.slitParam);
          if ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_INVPOLY_FILE))
           fprintf(fp,"; Deconvolution slit function polynomial degree : %d\n",(int)pEngineContext->slitDConv.slitParam2);
          if ((slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_ERF_FILE))
           fprintf(fp,"; Deconvolution slit function boxcar width : %lf\n",pEngineContext->slitDConv.slitParam2);

          if (slitType==SLIT_TYPE_VOIGT)
           {
            fprintf(fp,"; Gaussian FWHM (L) : %.3f\n",pEngineContext->slitDConv.slitParam);
            fprintf(fp,"; Lorentz/Gauss ratio (L) : %.3f\n",pEngineContext->slitDConv.slitParam2);
            fprintf(fp,"; Gaussian FWHM (R) : %.3f\n",pEngineContext->slitDConv.slitParam3);
            fprintf(fp,"; Lorentz/Gauss ratio (R) : %.3f\n",pEngineContext->slitDConv.slitParam4);
           }
         }

        // Low pass filtering

        if (pLFilter->type!=PRJCT_FILTER_TYPE_NONE)
         {
          fprintf(fp,"; Low-pass filtering\n");
          fprintf(fp,"; Filter applied : %s\n",mediateConvolutionFilterTypes[pLFilter->type]);
          fprintf(fp,"; Filter number of iterations : %d\n",pLFilter->filterNTimes);

          if (pLFilter->type==PRJCT_FILTER_TYPE_KAISER)
           {
            fprintf(fp,"; Cutoff frequency : %g\n",pLFilter->kaiserCutoff);
            fprintf(fp,"; Pass band : %g\n",pLFilter->kaiserPassBand);
            fprintf(fp,"; Tolerance : %g\n",pLFilter->kaiserTolerance);
           }
          else if (pLFilter->type==PRJCT_FILTER_TYPE_GAUSSIAN)
           fprintf(fp,"; Gaussian FWHM : %g\n",pLFilter->fwhmWidth);
          else if ((pLFilter->type==PRJCT_FILTER_TYPE_BOXCAR) || (pLFilter->type==PRJCT_FILTER_TYPE_TRIANGLE))
           fprintf(fp,"; Filter width : %d\n",pLFilter->filterWidth);
          else if (pLFilter->type==PRJCT_FILTER_TYPE_SG)
           {
            fprintf(fp,"; Filter width : %d\n",pLFilter->filterWidth);
            fprintf(fp,"; Filter order : %d\n",pLFilter->filterOrder);
           }
         }

        // High pass filtering

        if (pHFilter->type!=PRJCT_FILTER_TYPE_NONE)
         {
          fprintf(fp,"; High-pass filtering\n");
          fprintf(fp,"; Filter applied : %s\n",mediateConvolutionFilterTypes[pHFilter->type]);
          fprintf(fp,"; Filter number of iterations : %d\n",pHFilter->filterNTimes);

          if (pHFilter->type==PRJCT_FILTER_TYPE_KAISER)
           {
            fprintf(fp,"; Cutoff frequency : %g\n",pHFilter->kaiserCutoff);
            fprintf(fp,"; Pass band : %g\n",pHFilter->kaiserPassBand);
            fprintf(fp,"; Tolerance : %g\n",pHFilter->kaiserTolerance);
           }
          else if (pHFilter->type==PRJCT_FILTER_TYPE_GAUSSIAN)
           fprintf(fp,"; Gaussian FWHM : %g\n",pHFilter->fwhmWidth);
          else if ((pHFilter->type==PRJCT_FILTER_TYPE_BOXCAR) || (pHFilter->type==PRJCT_FILTER_TYPE_TRIANGLE))
           fprintf(fp,"; Filter width : %d\n",pHFilter->filterWidth);
          else if (pHFilter->type==PRJCT_FILTER_TYPE_SG)
           {
            fprintf(fp,"; Filter width : %d\n",pHFilter->filterWidth);
            fprintf(fp,"; Filter order : %d\n",pHFilter->filterOrder);
           }
         }

        fprintf(fp,";\n; Columns description :\n");
        fprintf(fp,"; Column 1 : calibration;\n");
        fprintf(fp,"; Column 2 : %s;\n",((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))?
                   "convoluted and filtered cross section" : "convoluted cross section");

        if ((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))
         fprintf(fp,"; Column 3 : convoluted only cross section.\n");
        fprintf(fp,";\n");
       }

      // Cross section

      if ((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))
       for (i=nextraPixels;i<nsize;i++)
        fprintf(fp,"%.14le %.14le %.14le\n",pXs->lambda[i],filterVector[i],pXs->vector[i]);
      else
       for (i=nextraPixels;i<nsize;i++)
        fprintf(fp,"%.14le %.14le\n",pXs->lambda[i],pXs->vector[i]);

      fclose(fp);
     }
    else
     rc=ERROR_SetLast("mediateConvolutionSave",ERROR_TYPE_FATAL,ERROR_ID_FILE_OPEN,fileName);
   }

  // Return

  return rc;
 }

// ----------------------------------------------
// mediateConvolutionCalculate : Main convolution function
// ----------------------------------------------

RC mediateConvolutionCalculate(void *engineContext,void *responseHandle)
 {
  // Declarations

  ENGINE_XSCONV_CONTEXT *pEngineContext=(ENGINE_XSCONV_CONTEXT*)engineContext;

  XS  XSCONV_slitFunction,                                                      // slit function used for convolution
      XSCONV_slitDFunction,                                                     // slit function used for deconvolution
      XSCONV_xshr,                                                              // high resolution cross section
     *pXsnew,                                                                   // convoluted cross section
      XSCONV_kurucz;                                                            // kurucz

  PRJCT_FILTER *plFilter,*phFilter;                                             // pointers to the low pass and high pass filtering parts of the engine context
  SLIT *pSlitConv,*pSlitDConv;                                                  // pointers to the convolution and deconvolution slit function parts of the engine context

  UCHAR windowTitle[MAX_ITEM_TEXT_LEN+1];
  double lambdaMin,lambdaMax,slitParam,slitParam2,slitWidth;

  INT slitType,slitType2,deconvFlag,dispConv,dispLFilter,dispHFilter,nGraph,iGraph;
  INT lowFilterType,highFilterType,nFilter;
  RC rc;

  // Slit function

  memset(&XSCONV_slitFunction,0,sizeof(XS));
  memset(&XSCONV_slitDFunction,0,sizeof(XS));
  memset(&XSCONV_xshr,0,sizeof(XS));
  memset(&XSCONV_kurucz,0,sizeof(XS));

  pSlitDConv=&pEngineContext->slitDConv;
  pSlitConv=&pEngineContext->slitConv;
  pXsnew=&pEngineContext->xsNew;

  XSCONV_Reset(pXsnew);

  slitType=pSlitConv->slitType;
  slitType2=pSlitDConv->slitType;
  slitParam=slitParam2=(double)0.;

  // Filtering

  plFilter=&pEngineContext->lfilter;
  phFilter=&pEngineContext->hfilter;

  lowFilterType=plFilter->type;            // low pass filtering
  highFilterType=phFilter->type;           // high pass filtering

  plFilter->filterFunction=phFilter->filterFunction=NULL;

//  if ((((lowFilterType=plFilter->type)!=PRJCT_FILTER_TYPE_NONE) &&
//        (lowFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) &&
//       ((rc=ANALYSE_LoadFilter(plFilter))!=0)) ||
//
//      (((highFilterType=phFilter->type)!=PRJCT_FILTER_TYPE_NONE) &&
//        (highFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) &&
//       ((rc=ANALYSE_LoadFilter(phFilter))!=0)))
//
//   goto EndConvolution;

  nFilter=0;

  if ((lowFilterType!=PRJCT_FILTER_TYPE_NONE) && (lowFilterType!=PRJCT_FILTER_TYPE_ODDEVEN))
   nFilter+=(int)(plFilter->filterWidth*sqrt(plFilter->filterNTimes)+0.5);
  if ((highFilterType!=PRJCT_FILTER_TYPE_NONE) && (highFilterType!=PRJCT_FILTER_TYPE_ODDEVEN))
   nFilter+=(int)(phFilter->filterWidth*sqrt(phFilter->filterNTimes)+0.5);

  pEngineContext->nFilter=nFilter;

  if (pEngineContext->filterVector!=NULL)
   MEMORY_ReleaseDVector("mediateConvolutionCalculate","filterVector",pEngineContext->filterVector,0);

  // Display control

  deconvFlag=((pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE) && (slitType!=SLIT_TYPE_GAUSS_FILE) && (slitType!=SLIT_TYPE_INVPOLY_FILE) && (slitType!=SLIT_TYPE_ERF_FILE) &&
            (((pSlitDConv->slitType==SLIT_TYPE_FILE) && (strlen(pSlitDConv->slitFile)!=0)) ||
            (((pSlitDConv->slitType==SLIT_TYPE_GAUSS) || (pSlitDConv->slitType==SLIT_TYPE_INVPOLY) || (pSlitDConv->slitType==SLIT_TYPE_ERF)) && (pSlitDConv->slitParam>0.))))?1:0;

  // Load calibration file and slit function

  if (!(rc=XSCONV_LoadCalibrationFile(pXsnew,pEngineContext->calibrationFile,nFilter)) &&
     (((lowFilterType==PRJCT_FILTER_TYPE_NONE) && (highFilterType==PRJCT_FILTER_TYPE_NONE)) ||
      ((pEngineContext->filterVector=(double *)MEMORY_AllocDVector("mediateConvolutionCalculate","filterVector",0,pXsnew->NDET-1))!=NULL)) &&
      ((pEngineContext->convolutionType==CONVOLUTION_TYPE_NONE) ||
     (!(rc=XSCONV_LoadSlitFunction(&XSCONV_slitFunction,&pEngineContext->slitConv,&slitParam,&slitType)) &&
      (!deconvFlag || !(rc=XSCONV_LoadSlitFunction(&XSCONV_slitDFunction,pSlitDConv,&slitParam2,&slitType2))))))
   {
    slitWidth=(double)2.*slitParam;

    // Window in wavelength

    if ((slitType!=SLIT_TYPE_FILE) || (pEngineContext->convolutionType==CONVOLUTION_TYPE_NONE))
     {
      lambdaMin=pXsnew->lambda[0]-slitWidth-1.;                     // add 1 nm
      lambdaMax=pXsnew->lambda[pXsnew->NDET-1]+slitWidth+1.;
     }
    else
     {
      lambdaMin=pXsnew->lambda[0]+XSCONV_slitFunction.lambda[0]-1.;                     // add 1 nm
      lambdaMax=pXsnew->lambda[pXsnew->NDET-1]+XSCONV_slitFunction.lambda[XSCONV_slitFunction.NDET-1]+1.;
     }

    if (deconvFlag)
     slitType=SLIT_TYPE_FILE;  // the resulting effective slit function works as a slit file type one

    // Determine effective slit function when a deconvolution slit function is given

    if ((!deconvFlag || !(rc=XSCONV_NewSlitFunction(pSlitConv,&XSCONV_slitFunction,slitParam,pSlitDConv,&XSCONV_slitDFunction,slitParam2))) &&

    // Load high resolution Kurucz file in convolution with I0 correction method

        ((pEngineContext->convolutionType!=CONVOLUTION_TYPE_I0_CORRECTION) ||
        !(rc=XSCONV_LoadCrossSectionFile(&XSCONV_kurucz,pEngineContext->kuruczFile,lambdaMin,lambdaMax,(double)0.,CONVOLUTION_CONVERSION_NONE))) &&
        !(rc=XSCONV_LoadCrossSectionFile(&XSCONV_xshr,pEngineContext->crossFile,lambdaMin,lambdaMax,(double)pEngineContext->shift,pEngineContext->conversionMode)))
     {
      dispConv=((pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE) ||
            (XSCONV_xshr.NDET!=pXsnew->NDET) ||
            !VECTOR_Equal(XSCONV_xshr.lambda,pXsnew->lambda,pXsnew->NDET,(double)1.e-7))?1:0;

      dispLFilter=(lowFilterType!=PRJCT_FILTER_TYPE_NONE)?1:0;
      dispHFilter=(highFilterType!=PRJCT_FILTER_TYPE_NONE)?1:0;

      nGraph=dispConv+dispLFilter+dispHFilter;
      iGraph=0;

      // -----------
      // Convolution
      // -----------

      switch(pEngineContext->convolutionType)
       {
     // ----------------------------------------------------------------------
        case CONVOLUTION_TYPE_NONE :
         rc=XSCONV_TypeNone(pXsnew,&XSCONV_xshr);
        break;
     // ----------------------------------------------------------------------
        case CONVOLUTION_TYPE_STANDARD :
         rc=XSCONV_TypeStandard(pXsnew,0,pXsnew->NDET,&XSCONV_xshr,&XSCONV_slitFunction,&XSCONV_xshr,NULL,slitType,slitWidth,slitParam,
                                 pEngineContext->slitConv.slitParam2,pEngineContext->slitConv.slitParam3,pEngineContext->slitConv.slitParam4);
        break;
     // ----------------------------------------------------------------------
        case CONVOLUTION_TYPE_I0_CORRECTION :
          rc=XSCONV_TypeI0Correction(pXsnew,&XSCONV_xshr,&XSCONV_kurucz,&XSCONV_slitFunction,pEngineContext->conc,slitType,slitWidth,slitParam,pEngineContext->slitConv.slitParam2,pEngineContext->slitConv.slitParam3,pEngineContext->slitConv.slitParam4);
        break;
     // ----------------------------------------------------------------------
     }

      // ------------------------------------------
      // Save resulting cross section and plot data
      // ------------------------------------------

      if (!rc && !(rc=mediateConvolutionSave(pEngineContext)) && dispConv)
       {
       	plot_data_t spectrumData[2];

       	sprintf(windowTitle,"Original cross section : %s",pEngineContext->crossFile);

        mediateAllocateAndSetPlotData(&spectrumData[0],XSCONV_xshr.lambda,XSCONV_xshr.vector,XSCONV_xshr.NDET,Line);
        mediateAllocateAndSetPlotData(&spectrumData[1],pXsnew->lambda+nFilter,pXsnew->vector+nFilter,pXsnew->NDET-2*nFilter,Line);
        mediateResponsePlotData(0,spectrumData,2,Spectrum,forceAutoScale,windowTitle,"Wavelength (nm)","",responseHandle);
        mediateResponseLabelPage(0,(pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE)?"Spectrum after convolution":"Spectrum after interpolation","",responseHandle);
        mediateReleasePlotData(spectrumData);
       }

      if (pEngineContext->filterVector!=NULL)
       memcpy(pEngineContext->filterVector,pXsnew->vector,pXsnew->NDET*sizeof(double));

      // ------------------
      // Low-Pass filtering
      // ------------------

      if ((lowFilterType!=PRJCT_FILTER_TYPE_NONE) && (pEngineContext->filterVector!=NULL) &&
        (((lowFilterType==PRJCT_FILTER_TYPE_ODDEVEN) && !(rc=FILTER_OddEvenCorrection(pXsnew->lambda,pXsnew->vector,pEngineContext->filterVector,pXsnew->NDET))) ||
         ((lowFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) && !(rc=FILTER_Vector(plFilter,pEngineContext->filterVector,pEngineContext->filterVector,pXsnew->NDET,PRJCT_FILTER_OUTPUT_LOW)))))
       {
       	#if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_

        SendMessage(CHILD_hwndFrame,WM_MDIACTIVATE,(WPARAM)CHILD_list[CHILD_WINDOW_SPECTRA].hwndChild,(LPARAM)0);
        sprintf(windowTitle,"Original cross section : %s",pEngineContext->crossFile);
        strcpy(graphTitle,"After low-pass filtering");

        DRAW_Spectra(CHILD_WINDOW_SPECTRA,windowTitle,graphTitle,"Wavelength (nm)","",NULL,0,
                    (double)0.,(double)0.,(double)0.,(double)0.,
                     pXsnew->lambda+nFilter,pXsnew->vector+nFilter,pXsnew->NDET-2*nFilter,DRAW_COLOR2,0,pXsnew->NDET-2*nFilter-1,PS_SOLID,"Not filtered",
                     pXsnew->lambda+nFilter,filterVector+nFilter,pXsnew->NDET-2*nFilter,DRAW_COLOR1,0,pXsnew->NDET-2*nFilter-1,PS_SOLID,"Filtered",
                     iGraph,1,nGraph,(iGraph==nGraph-1)?1:0);

        iGraph++;

        #endif
       }

      // -------------------
      // High-Pass filtering
      // -------------------

      if ((highFilterType!=PRJCT_FILTER_TYPE_NONE) && (highFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) && (pEngineContext->filterVector!=NULL) &&
         !(rc=FILTER_Vector(phFilter,pEngineContext->filterVector,pEngineContext->filterVector,pXsnew->NDET,PRJCT_FILTER_OUTPUT_HIGH_SUB+phFilter->filterAction)))
       {
       	#if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_

        SendMessage(CHILD_hwndFrame,WM_MDIACTIVATE,(WPARAM)CHILD_list[CHILD_WINDOW_SPECTRA].hwndChild,(LPARAM)0);
        sprintf(windowTitle,"Original cross section : %s",pEngineContext->crossFile);
        strcpy(graphTitle,"After high-pass filtering");

        DRAW_Spectra(CHILD_WINDOW_SPECTRA,windowTitle,graphTitle,"Wavelength (nm)","",NULL,0,
                    (double)0.,(double)0.,(double)0.,(double)0.,
                     pXsnew->lambda+nFilter,filterVector+nFilter,pXsnew->NDET-2*nFilter,DRAW_COLOR2,0,pXsnew->NDET-2*nFilter-1,PS_SOLID,"",
                     NULL,NULL,0,DRAW_COLOR1,0,0,PS_SOLID,"",
                     iGraph,1,nGraph,(iGraph==nGraph-1)?1:0);

        iGraph++;

        #endif
       }
     }
   }

  // Release allocated buffers

  XSCONV_Reset(&XSCONV_slitFunction);
  XSCONV_Reset(&XSCONV_slitDFunction);
  XSCONV_Reset(&XSCONV_xshr);
  XSCONV_Reset(&XSCONV_kurucz);

  if (plFilter->filterFunction!=NULL)
   {
    MEMORY_ReleaseDVector("mediateConvolutionCalculate","FILTER_function",plFilter->filterFunction,1);
    plFilter->filterFunction=NULL;
   }

  if (phFilter->filterFunction!=NULL)
   {
    MEMORY_ReleaseDVector("mediateConvolutionCalculate","FILTER_function",phFilter->filterFunction,1);
    phFilter->filterFunction=NULL;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestConvolution
// -----------------------------------------------------------------------------
// PURPOSE       Transfer the convolution options from the GUI to the engine
//
// RETURN        ERROR_ID_NO if no error found
// -----------------------------------------------------------------------------

void mediateRequestConvolution(void *engineContext,mediate_convolution_t *pMediateConvolution,void *responseHandle)
 {
 	// Declarations

  ENGINE_XSCONV_CONTEXT *pEngineContext = (ENGINE_XSCONV_CONTEXT*)engineContext;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  // General information

  pEngineContext->convolutionType=pMediateConvolution->general.convolutionType;
  pEngineContext->conversionMode=pMediateConvolution->general.conversionType;
  pEngineContext->shift=pMediateConvolution->general.shift;
  pEngineContext->conc=pMediateConvolution->general.conc;
  pEngineContext->noComment=pMediateConvolution->general.noheader;

  strcpy(pEngineContext->crossFile,pMediateConvolution->general.inputFile);                 // high resolution cross section file
  strcpy(pEngineContext->path,pMediateConvolution->general.outputFile);                     // output path
  strcpy(pEngineContext->calibrationFile,pMediateConvolution->general.calibrationFile);     // calibration file
  strcpy(pEngineContext->kuruczFile,pMediateConvolution->general.solarRefFile);             // Kurucz file used when I0 correction is applied

  // Description of the slit function

  setMediateSlit(&pEngineContext->slitConv,&pMediateConvolution->conslit);
  setMediateSlit(&pEngineContext->slitDConv,&pMediateConvolution->decslit);

  // Filtering configuration

  setMediateFilter(&pEngineContext->lfilter,&pMediateConvolution->lowpass);
  setMediateFilter(&pEngineContext->lfilter,&pMediateConvolution->highpass);
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateXsconvCreateContext
// -----------------------------------------------------------------------------
// PURPOSE       This function is called when one of the convolution tool is
//               started.  It creates a single context for safely accessing its
//               features through the mediator layer.  The engine context is
//               never destroyed before the user exits the program.
//
// RETURN        On success 0 is returned and the value of handleEngine is set,
//               otherwise -1 is retured and the value of handleEngine is undefined.
// -----------------------------------------------------------------------------

int mediateXsconvCreateContext(void **engineContext, void *responseHandle)
 {
 	ENGINE_XSCONV_CONTEXT *pEngineContext;

 	*engineContext=(void *)EngineXsconvCreateContext();
 	pEngineContext=(ENGINE_XSCONV_CONTEXT *)*engineContext;

  if (pEngineContext==NULL)
   ERROR_DisplayMessage(responseHandle);

  return (pEngineContext!=NULL)?0:-1;
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateXsconvDestroyContext
// -----------------------------------------------------------------------------
// PURPOSE       Destroy the engine context when the user exits the program.
//
// RETURN        Zero is returned on success, -1 otherwise.
// -----------------------------------------------------------------------------

int mediateXsconvDestroyContext(void *engineContext, void *responseHandle)
 {
  return (!EngineXsconvDestroyContext((ENGINE_XSCONV_CONTEXT *)engineContext))?0:-1;
 }

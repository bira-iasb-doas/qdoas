
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  KURUCZ PROCESSING
//  Name of module    :  KURUCZ.C
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
//  FUNCTIONS
//
//  =================
//  KURUCZ PROCEDURES
//  =================
//
//  KURUCZ_SearchReference - search for a reference spectrum in analysis windows on which Kurucz has already been applied;
//  KURUCZ_Spectrum - apply Kurucz for building a new wavelength scale to a spectrum;
//  KURUCZ_ApplyCalibration - apply the new calibration to cross sections to interpolate or to convolute and recalculate gaps;
//  KURUCZ_Reference - browse analysis windows and apply Kurucz if needed on reference spectrum;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  KURUCZ_Init - set the limits for each window of the calibration interval;
//  KURUCZ_Alloc - allocate all buffers needed for applying Kurucz procedures to a project;
//  KURUCZ_Free - release buffers allocated for Kurucz procedures;
//  ----------------------------------------------------------------------------

#include "mediate.h"
#include "engine.h"

// ================
// GLOBAL VARIABLES
// ================

KURUCZ KURUCZ_buffers[MAX_SWATHSIZE];
FFT *pKURUCZ_fft;
INT KURUCZ_indexLine=1;

// =================
// KURUCZ PROCEDURES
// =================

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_SearchReference
// ----------------------------------------------------------------------------
// PURPOSE         search for a reference spectrum in analysis windows on which Kurucz has already been applied
//
// INPUT           indexRefFeno   index of the analysis window with the reference to search for in other analysis windows;
//                 indexRefColumn index to account to the second dimension of FENO (new with OMI)
//
// RETURN          indexFeno      index of the analysis window in which reference has been found;
// ----------------------------------------------------------------------------

INDEX KuruczSearchReference(INDEX indexRefFeno,INDEX indexRefColumn)
 {
  // Declarations

  double *reference;
  INDEX indexFeno;
  FENO *pTabFeno;

  // Initialization

  reference=(TabFeno[indexRefColumn][indexRefFeno].useEtalon)?TabFeno[indexRefColumn][indexRefFeno].SrefEtalon:TabFeno[indexRefColumn][indexRefFeno].Sref;

  // Search for reference in analysis windows

  for (indexFeno=0;indexFeno<NFeno;indexFeno++)
   {
    pTabFeno=&TabFeno[indexRefColumn][indexFeno];

    if (!pTabFeno->hidden && pTabFeno->useKurucz && (indexFeno!=indexRefFeno) &&
         VECTOR_Equal((pTabFeno->useEtalon)?pTabFeno->SrefEtalon:pTabFeno->Sref, // Etalon has highest priority
                    reference,NDET,(double)1.e-7))

     break;
   }

  // Return

  return indexFeno;
 }

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_Spectrum
// ----------------------------------------------------------------------------
// PURPOSE         apply Kurucz for building a new wavelength scale to a spectrum
//
// INPUT           oldLambda      old calibration associated to reference;
//                 spectrum       spectrum to shift;
//                 reference      reference spectrum;
//                 instrFunction  instrumental function for correcting reference
//                 displayFlag    1 to display the results of the fit in the Graphs window
//                 windowTitle    title of the graph window
//                 saveFlag       1 to save the calibration results in the data Window
//                 indexFeno      index of the current sub-window
//
// OUTPUT          newLambda      new wavelength scale
//
// RETURN          return code
// ----------------------------------------------------------------------------      

RC KURUCZ_Spectrum(double *oldLambda,double *newLambda,double *spectrum,double *reference,double *instrFunction,
                   char displayFlag, const char *windowTitle,double **coeff,double **fwhmVector,double **fwhmDeriv2,INT saveFlag,INDEX indexFeno,void *responseHandle,INDEX indexFenoColumn)
 {
  // Declarations

  char            string[MAX_ITEM_TEXT_LEN+1],pageTitle[MAX_ITEM_TEXT_LEN+1];
  CROSS_REFERENCE *TabCross,*pTabCross;
  CROSS_RESULTS   *pResults,*Results;                                           // pointer to results associated to a symbol
  SVD             *svdFeno;                                                     // svd environments associated to list of little windows
  double          *VPix,*VSig,*Pcalib,                                          // polynomial coefficients computation
                  *shiftPoly,
                  *pixMid,*VLambda,*VShift,
                 **fwhm,**fwhmSigma,                                            // substitution vectors
                  *solar,                                                       // solar spectrum
                  *offset,                                                      // offset
                   Square;                                                      // Chi square returned by 'CurFitMethod'
  INT              Nb_Win,maxParam,                                             // number of little windows
                  *NIter,                                                       // number of iterations
                   oldNDET;
  INDEX            indexWindow,                                                 // browse little windows
                   indexParam,
                   indexTabCross,
                   indexCrossFit,
                   indexLine,indexColumn,                                       // position in the spreadsheet for information to write
                   i,j,k;                                                    // temporary indexes
  double j0,lambda0;
  RC               rc;                                                          // return code
  plot_data_t      spectrumData[2];
  KURUCZ *pKurucz;

  // Initializations

  pKurucz=&KURUCZ_buffers[indexFenoColumn];
  indexLine=(ANALYSE_swathSize==1)?1:KURUCZ_indexLine;
  indexColumn=2;
  solar=NULL;
  oldNDET    = NDET;

  if ((shiftPoly=(double *)MEMORY_AllocDVector("KURUCZ_Spectrum ","shiftPoly",0,NDET-1))==NULL)
    rc=ERROR_ID_ALLOC;
  else
   {
    // Use substitution variables

    Feno=&TabFeno[indexFenoColumn][pKurucz->indexKurucz];
    TabCross=Feno->TabCross;

    sprintf(pageTitle,"Kurucz");

    memcpy(Feno->LambdaK,oldLambda,sizeof(double)*NDET);
    rc=ANALYSE_XsInterpolation(Feno,oldLambda,indexFenoColumn);

    Results=Feno->TabCrossResults;
    pResults=&Feno->TabCrossResults[(Feno->indexSpectrum!=ITEM_NONE)?Feno->indexSpectrum:Feno->indexReference];

    rc=ERROR_ID_NO;

    VPix       = pKurucz->VPix;
    VSig       = pKurucz->VSig;
    Pcalib     = pKurucz->Pcalib;
    pixMid     = pKurucz->pixMid;
    VLambda    = pKurucz->VLambda;
    VShift     = pKurucz->VShift;

    svdFeno    = pKurucz->KuruczFeno[indexFeno].svdFeno;

    Nb_Win     = pKurucz->Nb_Win;
    fwhm       = pKurucz->fwhm;
    fwhmSigma  = pKurucz->fwhmSigma;
    offset     = pKurucz->offset;
    NIter      = pKurucz->NIter;

    NDET       = TabFeno[indexFenoColumn][indexFeno].NDET;

    for (maxParam=0;maxParam<MAX_KURUCZ_FWHM_PARAM;maxParam++)
     if ((fwhmVector[maxParam]!=NULL) && (Feno->indexFwhmParam[maxParam]!=ITEM_NONE))
      VECTOR_Init(fwhmVector[maxParam],TabCross[Feno->indexFwhmParam[maxParam]].InitParam,NDET);
     else
      break;

    // Instrumental correction

    memcpy(ANALYSE_absolu,ANALYSE_zeros,sizeof(double)*NDET);
    memcpy(offset,ANALYSE_zeros,sizeof(double)*NDET);

    if (instrFunction!=NULL)
     for (i=0;i<NDET;i++)
      spectrum[i]/=(double)instrFunction[i];

    // Always restart from the original calibration

    Lambda=newLambda;
    LambdaSpec=newLambda;
    memcpy(Lambda,oldLambda,sizeof(double)*NDET);
    memcpy(ANALYSE_secX,spectrum,sizeof(double)*NDET);

    // Set solar spectrum


    if ((solar=MEMORY_AllocDVector("KURUCZ_Spectrum ","solar",0,NDET))==NULL)
     rc=ERROR_ID_ALLOC;
    else if (!pKuruczOptions->fwhmFit)
     {
      if (pSlitOptions->slitFunction.slitType==SLIT_TYPE_NONE)
       {
       	if (pKurucz->hrSolar.nc<ANALYSE_swathSize+1)                                                                         
       	 rc=ERROR_SetLast("KURUCZ_Spectrum",ERROR_TYPE_FATAL,ERROR_ID_OMI_REFSIZE,"Solar spectrum");
        else if ((pKurucz->hrSolar.nl==NDET) && VECTOR_Equal(pKurucz->hrSolar.matrix[0],oldLambda,NDET,(double)1.e-7))
         memcpy(solar,pKurucz->hrSolar.matrix[1],sizeof(double)*NDET);
        else
         rc=SPLINE_Vector(pKurucz->hrSolar.matrix[0],pKurucz->hrSolar.matrix[1],pKurucz->hrSolar.deriv2[1],pKurucz->hrSolar.nl,
                             oldLambda,solar,NDET,pAnalysisOptions->interpol,"KURUCZ_Spectrum ");
       } 
      else // 20130208 : a high resolution spectrum is now loaded from the slit page of project properties and convolved
      	rc=ANALYSE_ConvoluteXs(NULL,ANLYS_CROSS_ACTION_CONVOLUTE,(double)0.,&pKurucz->hrSolar,&ANALYSIS_slit,&ANALYSIS_slit2,
                              pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2,
                              oldLambda,solar,0,NDET-1,0,pSlitOptions->slitFunction.slitWveDptFlag);
// 20130208 :     rc=SPLINE_Vector(pKurucz->hrSolar.matrix[0],pKurucz->hrSolar.matrix[kuruczIndexRow],pKurucz->hrSolar.deriv2[kuruczIndexRow],pKurucz->hrSolar.nl,
//                        oldLambda,solar,NDET,pAnalysisOptions->interpol,"KURUCZ_Spectrum ");
     }
    else
     memcpy(solar,reference,sizeof(double)*NDET);
     
    if (rc!=ERROR_ID_NO)
     goto EndKuruczSpectrum; 

    // Buffers for fits initialization

    if (pKurucz->crossFits.matrix!=NULL)
     for (indexTabCross=0;indexTabCross<pKurucz->crossFits.nc;indexTabCross++)
      memcpy(pKurucz->crossFits.matrix[indexTabCross],ANALYSE_zeros,sizeof(double)*NDET);

    memcpy(ANALYSE_t,ANALYSE_zeros,sizeof(double)*NDET);
    memcpy(ANALYSE_tc,ANALYSE_zeros,sizeof(double)*NDET);

    ANALYSE_plotKurucz=(pKurucz->displaySpectra || pKurucz->displayResidual || pKurucz->displayFit || pKurucz->displayShift)?1:0;

    if (ANALYSE_plotKurucz)
     {
     	if (ANALYSE_swathSize>1)
     	 {
     	 	sprintf(string,"Row %d/%d",indexFenoColumn+1,ANALYSE_swathSize);
     	  mediateResponseCellDataString(plotPageCalib,indexLine++,1,string,responseHandle);
     	 }

      if (!TabFeno[indexFenoColumn][indexFeno].hidden)
       mediateResponseCellInfo(plotPageCalib,indexLine++,indexColumn,responseHandle,"KURUCZ alignment for window ","%s",TabFeno[indexFenoColumn][indexFeno].windowName);
      else
       mediateResponseCellDataString(plotPageCalib,indexLine++,indexColumn,"Kurucz",responseHandle);

      mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,"Window",responseHandle);
      mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,"Pixel",responseHandle);
      mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,"Wavelength",responseHandle);
      mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,"Niter",responseHandle);
      mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,"Shift",responseHandle);

      if (pKuruczOptions->fwhmFit)
       for (indexParam=0;indexParam<maxParam;indexParam++)
        {
         sprintf(string,"SFP %d",indexParam+1);
         mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,string,responseHandle);
        }

      if ((Feno->indexOffsetConst!=ITEM_NONE) && (Feno->TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE))
       mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,"Offset",responseHandle);

      for (indexTabCross=0;indexTabCross<Feno->NTabCross;indexTabCross++)
       {
        pTabCross=&TabCross[indexTabCross];

        if (pTabCross->IndSvdA && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS))
         mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,WorkSpace[pTabCross->Comp].symbolName,responseHandle);
       }
     }

    // Browse little windows

    for (indexWindow=0,Square=(double)0.;(indexWindow<Nb_Win) && (rc<THREAD_EVENT_STOP);indexWindow++)
     {
      Feno->Decomp=1;
      NIter[indexWindow]=0;

      if (pKuruczOptions->fwhmFit)
       pKURUCZ_fft=&pKurucz->KuruczFeno[indexFeno].fft[indexWindow];

      // Global initializations

      #if defined(__DEBUG_) && __DEBUG_
      DEBUG_Start(ENGINE_dbgFile,"Kurucz",DEBUG_FCTTYPE_MATH|DEBUG_FCTTYPE_APPL,5,DEBUG_DVAR_YES,0); // !debugResetFlag++);
      #endif

      if (((rc=ANALYSE_SvdInit(&svdFeno[indexWindow]))!=ERROR_ID_NO) ||

      // Analysis method

          ((rc=ANALYSE_CurFitMethod(indexFenoColumn,                            // to change a little bit later for OMI
                                    spectrum,                                   // spectrum
                                    NULL,                                       // no error on previous spectrum
                                    solar,                                      // reference (Kurucz)
				    NULL,
                                   &Square,                                     // returned stretch order 2
                                   &NIter[indexWindow],
                                   (double)1.,(double)1.))>0))
       break;

      #if defined(__DEBUG_) && __DEBUG_
      DEBUG_Stop("Kurucz");
      #endif

      // Fill A SVD system

      pixMid[indexWindow+1]=(double)( spectrum_start(svdFeno[indexWindow].specrange)
                                      + spectrum_end(svdFeno[indexWindow].specrange) )*0.5;

      VSig[indexWindow+1]=pResults->SigmaShift;

      // not used anymore as we work only in nm now if (pKurucz->units==PRJCT_ANLYS_UNITS_PIXELS)
      // not used anymore as we work only in nm now  {
      // not used anymore as we work only in nm now   VShift[indexWindow+1]=(Feno->indexSpectrum!=ITEM_NONE)?(double)-pResults->Shift:(double)pResults->Shift;
      // not used anymore as we work only in nm now   VPix[indexWindow+1]=(double)pixMid[indexWindow+1];                      // -pResults->Shift and VLambda[indexWindow+1]=(double)oldLambda[(INDEX)pixMid[indexWindow+1]]
      // not used anymore as we work only in nm now
      // not used anymore as we work only in nm now   newPix=(double)pixMid[indexWindow+1]-VShift[indexWindow+1]+1.;
      // not used anymore as we work only in nm now                                                                           // if kurucz spectrum is passed as reference
      // not used anymore as we work only in nm now   if ((rc=SPLINE_Vector(ANALYSE_pixels,                                   // pixels vector
      // not used anymore as we work only in nm now                            oldLambda,                                     // wavelength scale
      // not used anymore as we work only in nm now                            ANALYSE_splineX2,                              // second derivatives
      // not used anymore as we work only in nm now                            NDET,
      // not used anymore as we work only in nm now                           &newPix,
      // not used anymore as we work only in nm now                           &VLambda[indexWindow+1],1,
      // not used anymore as we work only in nm now                            SPLINE_CUBIC,"KURUCZ_Spectrum "))!=ERROR_ID_NO)
      // not used anymore as we work only in nm now    break;
      // not used anymore as we work only in nm now  }
      // not used anymore as we work only in nm now else
       {
        VShift[indexWindow+1]=(Feno->indexSpectrum!=ITEM_NONE)?(double)-pResults->Shift:(double)pResults->Shift;
        VPix[indexWindow+1]=pixMid[indexWindow+1];
        VLambda[indexWindow+1]=(fabs(pixMid[indexWindow+1]-floor(pixMid[indexWindow+1]))<(double)0.1)?
                               (double)oldLambda[(INDEX)pixMid[indexWindow+1]]-VShift[indexWindow+1]:
                               (double)0.5*(oldLambda[(INDEX)floor(pixMid[indexWindow+1])]+oldLambda[(INDEX)floor(pixMid[indexWindow+1]+1.)])-VShift[indexWindow+1];
       }

      // Store fwhm for future use

      if (pKuruczOptions->fwhmFit)
       for (indexParam=0;indexParam<maxParam;indexParam++)
        {
        	if ((indexParam==1) && (pKuruczOptions->fwhmType==SLIT_TYPE_AGAUSS))
          fwhm[indexParam][indexWindow]=Feno->TabCrossResults[Feno->indexFwhmParam[indexParam]].Param;  // asymmetric factor can be negatif for asymmetric gaussian
         else
          fwhm[indexParam][indexWindow]=fabs(Feno->TabCrossResults[Feno->indexFwhmParam[indexParam]].Param);

         fwhmSigma[indexParam][indexWindow]=Feno->TabCrossResults[Feno->indexFwhmParam[indexParam]].SigmaParam;
        }


      // Store fit for display

      if (displayFlag)
       {
        if (pKurucz->method!=PRJCT_ANLYS_METHOD_SVD)
         for (i=SvdPDeb;i<=SvdPFin;i++)
          ANALYSE_secX[i]=exp(log(spectrum[i])+ANALYSE_absolu[i]/ANALYSE_tc[i]); // spectrum[i]+solar[i]*ANALYSE_absolu[i]/ANALYSE_tc[i];
        else
         for (i=SvdPDeb;i<=SvdPFin;i++)
          ANALYSE_secX[i]=exp(log(spectrum[i])+ANALYSE_absolu[i]);

        j0=(double)(SvdPDeb+SvdPFin)*0.5;
        lambda0=(fabs(j0-floor(j0))<(double)0.1)?
                                    (double)ANALYSE_splineX[(INDEX)j0]:
                                    (double)0.5*(ANALYSE_splineX[(INDEX)floor(j0)]+ANALYSE_splineX[(INDEX)floor(j0+1.)]);

        if ((Feno->indexOffsetConst!=ITEM_NONE) &&
            (Feno->indexOffsetOrder1!=ITEM_NONE) &&
            (Feno->indexOffsetOrder2!=ITEM_NONE) &&

           ((TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE) ||
            (TabCross[Feno->indexOffsetOrder1].FitParam!=ITEM_NONE) ||
            (TabCross[Feno->indexOffsetOrder2].FitParam!=ITEM_NONE) ||
            (TabCross[Feno->indexOffsetConst].InitParam!=(double)0.) ||
            (TabCross[Feno->indexOffsetOrder1].InitParam!=(double)0.) ||
            (TabCross[Feno->indexOffsetOrder2].InitParam!=(double)0.)))

         for (i=SvdPDeb;i<=SvdPFin;i++)
          {
           offset[i]=(double)1.-Feno->xmean*(Results[Feno->indexOffsetConst].Param+
                  Results[Feno->indexOffsetOrder1].Param*(ANALYSE_splineX[i]-lambda0)+
                  Results[Feno->indexOffsetOrder2].Param*(ANALYSE_splineX[i]-lambda0)*(ANALYSE_splineX[i]-lambda0))/spectrum[i];
           offset[i]=(offset[i]>(double)0.)?log(offset[i]):(double)0.;
          }

        if (pKurucz->crossFits.matrix!=NULL)

         for (indexTabCross=indexCrossFit=0;(indexTabCross<Feno->NTabCross) && (indexCrossFit<pKurucz->crossFits.nc);indexTabCross++)
          {
           pTabCross=&TabCross[indexTabCross];

           if (pTabCross->IndSvdA && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) && pTabCross->display)
            {
             for (i=SvdPDeb,k=1;i<=SvdPFin;i++,k++)
              pKurucz->crossFits.matrix[indexCrossFit][i]=x[pTabCross->IndSvdA]*U[pTabCross->IndSvdA][k];

             indexCrossFit++;
            }
          }
       }

      // Results safe keeping

      memcpy(pKurucz->KuruczFeno[indexFeno].results[indexWindow],Feno->TabCrossResults,sizeof(CROSS_RESULTS)*Feno->NTabCross);

      pKurucz->KuruczFeno[indexFeno].wve[indexWindow]=VLambda[indexWindow+1];
      pKurucz->KuruczFeno[indexFeno].chiSquare[indexWindow]=Square;
      pKurucz->KuruczFeno[indexFeno].rms[indexWindow]=(Square>(double)0.)?sqrt(Square):(double)0.;
      pKurucz->KuruczFeno[indexFeno].nIter[indexWindow]=NIter[indexWindow];
     }  // End for (indexWindow=...

    if (!rc)
     {
      SvdPDeb=spectrum_start(svdFeno[0].specrange);
      SvdPFin=spectrum_end(svdFeno[Nb_Win-1].specrange);

      // New wavelength scale (corrected calibration)
      // NB : we fit a polynomial in Lambda+shift point but it's possible to fit a polynomial in shift points by replacing
      //      VLambda by VShift in the following instruction

      if ((rc=ANALYSE_LinFit(&Feno->svd,Nb_Win,pKurucz->shiftDegree,VPix,NULL,VShift,Pcalib))!=ERROR_ID_NO)
      // VLambda not used anymore as we work only in nm now    (pKurucz->units==PRJCT_ANLYS_UNITS_PIXELS)?VLambda:VShift,Pcalib))!=ERROR_ID_NO) // !!! Change in the future VLambda -> VShift
       goto EndKuruczSpectrum;

      if (pKuruczOptions->fwhmFit)
       {
        for (indexParam=0;indexParam<maxParam;indexParam++)
         if (TabCross[Feno->indexFwhmParam[indexParam]].FitParam!=ITEM_NONE)
          {
           memcpy(coeff[indexParam],ANALYSE_zeros,sizeof(double)*(pKurucz->fwhmDegree+1));
           memcpy(fwhmVector[indexParam],ANALYSE_zeros,sizeof(double)*NDET);

           if ((rc=ANALYSE_LinFit(&pKurucz->svdFwhm,Nb_Win,pKurucz->fwhmDegree,VLambda,NULL,fwhm[indexParam]-1,coeff[indexParam]-1))!=ERROR_ID_NO)
            goto EndKuruczSpectrum;
          }

        if (rc!=ERROR_ID_NO)
         goto EndKuruczSpectrum;
       }

      for (i=0;i<NDET;i++)
       {
        shiftPoly[i]=Pcalib[pKurucz->shiftDegree+1];
        for (j=pKurucz->shiftDegree;j>=1;j--)
         shiftPoly[i]=shiftPoly[i]*(double)i+Pcalib[j];

        // not used anymore as we work only in nm now if (pKurucz->units==PRJCT_ANLYS_UNITS_NANOMETERS)
         Lambda[i]=oldLambda[i]-shiftPoly[i];    // only if you want to fit a polynomial in shift points and afterwards,
        // not used anymore as we work only in nm now else
        // not used anymore as we work only in nm now  Lambda[i]=shiftPoly[i];
       }

      if (displayFlag)
       {
        // Display complete fit

        if (pKurucz->displaySpectra)
         {
         	if (ANALYSE_swathSize==1)
         	 strcpy(string,"Complete fit");
         	else
         	 sprintf(string,"Complete fit (%d/%d)",indexFenoColumn+1,ANALYSE_swathSize);

          mediateAllocateAndSetPlotData(&spectrumData[0],"Spectrum",&Lambda[0],&spectrum[0],NDET,Line);
          mediateAllocateAndSetPlotData(&spectrumData[1],"Adjusted Kurucz",&Lambda[0],&ANALYSE_secX[0],NDET,Line);
          mediateResponsePlotData(plotPageCalib,spectrumData,2,Spectrum,forceAutoScale,string,"Wavelength (nm)","Intensity", responseHandle);
          mediateReleasePlotData(&spectrumData[1]);
          mediateReleasePlotData(&spectrumData[0]);
         }

        // Display residual

        if (pKurucz->displayResidual)
         {
         	if (ANALYSE_swathSize==1)
         	 strcpy(string,"Residual");
         	else
         	 sprintf(string,"Residual (%d/%d)",indexFenoColumn+1,ANALYSE_swathSize);

          if (pKurucz->method!=PRJCT_ANLYS_METHOD_SVD)
           for (j=SvdPDeb;j<=SvdPFin;j++)
            ANALYSE_absolu[j]=(ANALYSE_tc[j]!=(double)0.)?ANALYSE_absolu[j]/ANALYSE_tc[j]:(double)0.;

          mediateAllocateAndSetPlotData(&spectrumData[0],"Residual",&Lambda[0],&ANALYSE_absolu[0],NDET,Line);
          mediateResponsePlotData(plotPageCalib,spectrumData,1,Spectrum,forceAutoScale,string,"Wavelength (nm)","", responseHandle);
          mediateReleasePlotData(&spectrumData[0]);
         }

        memcpy(ANALYSE_secX,ANALYSE_zeros,sizeof(double)*NDET);

        // Display Offset

        if  (// Feno->displayOffset &&
            (Feno->indexOffsetConst!=ITEM_NONE) &&
            (Feno->indexOffsetOrder1!=ITEM_NONE) &&
            (Feno->indexOffsetOrder2!=ITEM_NONE) &&

           ((TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE) ||
            (TabCross[Feno->indexOffsetOrder1].FitParam!=ITEM_NONE) ||
            (TabCross[Feno->indexOffsetOrder2].FitParam!=ITEM_NONE) ||
            (TabCross[Feno->indexOffsetConst].InitParam!=(double)0.) ||
            (TabCross[Feno->indexOffsetOrder1].InitParam!=(double)0.) ||
            (TabCross[Feno->indexOffsetOrder2].InitParam!=(double)0.)))
         {
          for (j=SvdPDeb;j<=SvdPFin;j++)
           {
            ANALYSE_absolu[j]+=offset[j]-ANALYSE_secX[j];
            ANALYSE_secX[j]=offset[j];
           }

          if (pKurucz->displayFit)
           {
         	  if (ANALYSE_swathSize==1)
         	   strcpy(string,"Offset");
         	  else
         	   sprintf(string,"Offset (%d/%d)",indexFenoColumn+1,ANALYSE_swathSize);

            mediateAllocateAndSetPlotData(&spectrumData[0],"Measured",&Lambda[0],&ANALYSE_absolu[0],NDET,Line);
            mediateAllocateAndSetPlotData(&spectrumData[1],"Calculated",&Lambda[0],&ANALYSE_secX[0],NDET,Line);
            mediateResponsePlotData(plotPageCalib,spectrumData,2,Spectrum,forceAutoScale,string,"Wavelength (nm)","", responseHandle);
            mediateReleasePlotData(&spectrumData[1]);
            mediateReleasePlotData(&spectrumData[0]);
           }
         }

        // Display fits

        if (pKurucz->crossFits.matrix!=NULL)
         {
          for (indexTabCross=indexCrossFit=0;(indexTabCross<Feno->NTabCross) && (indexCrossFit<pKurucz->crossFits.nc);indexTabCross++)
           {
            pTabCross=&TabCross[indexTabCross];

            if (pTabCross->IndSvdA && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) && pTabCross->display)
             {
               for (j=SvdPDeb;j<=SvdPFin;j++)
                {
                 ANALYSE_absolu[j]+=pKurucz->crossFits.matrix[indexCrossFit][j]-ANALYSE_secX[j];
                 ANALYSE_secX[j]=pKurucz->crossFits.matrix[indexCrossFit][j];
                }

         	    if (ANALYSE_swathSize==1)
         	     sprintf(string,"%s fit",WorkSpace[pTabCross->Comp].symbolName);
         	    else
         	     sprintf(string,"%s fit (%d/%d)",WorkSpace[pTabCross->Comp].symbolName,indexFenoColumn+1,ANALYSE_swathSize);

              mediateAllocateAndSetPlotData(&spectrumData[0],"Measured",&Lambda[0],&ANALYSE_absolu[0],NDET,Line);
              mediateAllocateAndSetPlotData(&spectrumData[1],"Calculated",&Lambda[0],&ANALYSE_secX[0],NDET,Line);
              mediateResponsePlotData(plotPageCalib,spectrumData,2,Spectrum,forceAutoScale,string,"Wavelength (nm)","", responseHandle);
              mediateReleasePlotData(&spectrumData[1]);
              mediateReleasePlotData(&spectrumData[0]);

              indexCrossFit++;
             }
           }
         }

        // Display error on calibration

        if (pKurucz->displayShift)
         {
         	  if (ANALYSE_swathSize==1)
         	   strcpy(string,"Shift");
         	  else
         	   sprintf(string,"Shift (%d/%d)",indexFenoColumn+1,ANALYSE_swathSize);

          mediateAllocateAndSetPlotData(&spectrumData[0],"Polynomial fitting individual shift points",&Lambda[0],&shiftPoly[0],NDET,Line);
          mediateAllocateAndSetPlotData(&spectrumData[1],"Shift calculated in the individual small windows",VLambda+1,VShift+1,Nb_Win,Point);
          mediateResponsePlotData(plotPageCalib,spectrumData,2,Spectrum,forceAutoScale,string,"Wavelength (nm)","Shift (nm)", responseHandle);
          mediateReleasePlotData(&spectrumData[1]);
          mediateReleasePlotData(&spectrumData[0]);
         }

        // Display wavelength dependence of fwhm

        if (pKuruczOptions->fwhmFit)

         for (indexParam=0;(indexParam<maxParam) && (rc<THREAD_EVENT_STOP);indexParam++)
          {
         	 if (ANALYSE_swathSize==1)
         	  sprintf(string,"SFP %d",indexParam+1);
         	 else
         	  sprintf(string,"SFP %d (%d/%d)",indexParam+1,indexFenoColumn+1,ANALYSE_swathSize);

           if (TabCross[Feno->indexFwhmParam[indexParam]].FitParam!=ITEM_NONE)
            {
             for (i=0;i<NDET;i++)
              {
               fwhmVector[indexParam][i]=(double)coeff[indexParam][pKurucz->fwhmDegree];
               for (j=pKurucz->fwhmDegree-1;j>=0;j--)
                fwhmVector[indexParam][i]=fwhmVector[indexParam][i]*(double)Lambda[i]+coeff[indexParam][j];
              }

             if ((rc=SPLINE_Deriv2(Lambda,fwhmVector[indexParam],fwhmDeriv2[indexParam],NDET,"KURUCZ_Spectrum "))!=0)
              goto EndKuruczSpectrum;
            }

           if (pKurucz->displayShift)
            {
             mediateAllocateAndSetPlotData(&spectrumData[0],"Polynomial fitting individual FWHM points",&Lambda[0],&fwhmVector[indexParam][0],NDET,Line);
             mediateAllocateAndSetPlotData(&spectrumData[1],"FWHM calculated in the individual small windows",VLambda+1,fwhm[indexParam],Nb_Win,Point);
             mediateResponsePlotData(plotPageCalib,spectrumData,2,Spectrum,forceAutoScale,string,"Wavelength (nm)","SFP (nm)", responseHandle);
             mediateReleasePlotData(&spectrumData[1]);
             mediateReleasePlotData(&spectrumData[0]);
            }
          }

        mediateResponseLabelPage(plotPageCalib, "", pageTitle, responseHandle);

        for (indexWindow=0,indexLine+=1;indexWindow<Nb_Win;indexWindow++,indexLine++)
         {
          indexColumn=2;

          mediateResponseCellInfoNoLabel(plotPageCalib,indexLine,indexColumn++,responseHandle,"%2d/%d",indexWindow+1,Nb_Win);
          mediateResponseCellDataDouble(plotPageCalib,indexLine,indexColumn++,pixMid[indexWindow+1],responseHandle);
          mediateResponseCellDataDouble(plotPageCalib,indexLine,indexColumn++,VLambda[indexWindow+1],responseHandle);
          mediateResponseCellDataInteger(plotPageCalib,indexLine,indexColumn++,NIter[indexWindow],responseHandle);
          mediateResponseCellInfoNoLabel(plotPageCalib,indexLine,indexColumn++,responseHandle,"%10.3e+/-%10.3e",VShift[indexWindow+1],VSig[indexWindow+1]);

          for (indexParam=0;indexParam<maxParam;indexParam++)
           mediateResponseCellInfoNoLabel(plotPageCalib,indexLine,indexColumn++,responseHandle,"%10.3e+/-%10.3e",fwhm[indexParam][indexWindow],fwhmSigma[indexParam][indexWindow]);

          if ((Feno->indexOffsetConst!=ITEM_NONE) && (Feno->TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE))
           mediateResponseCellInfoNoLabel(plotPageCalib,indexLine,indexColumn++,responseHandle,"%10.3e+/-%10.3e",Feno->TabCrossResults[Feno->indexOffsetConst].Param,Feno->TabCrossResults[Feno->indexOffsetConst].SigmaParam);

          for (indexTabCross=0;indexTabCross<Feno->NTabCross;indexTabCross++)
           {
            pTabCross=&TabCross[indexTabCross];

            if (pTabCross->IndSvdA && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS))
             mediateResponseCellInfoNoLabel(plotPageCalib,indexLine,indexColumn++,responseHandle,"%10.3e+/-%10.3e",
             pKurucz->KuruczFeno[indexFeno].results[indexWindow][indexTabCross].SlntCol,pKurucz->KuruczFeno[indexFeno].results[indexWindow][indexTabCross].SlntErr);
             //Feno->TabCrossResults[indexTabCross].SlntCol,Feno->TabCrossResults[indexTabCross].SlntErr);
           }
         }
       }
     }
   }

  EndKuruczSpectrum :

  if (ANALYSE_swathSize>1)
   KURUCZ_indexLine=indexLine+1;

  if (solar!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Spectrum ","solar",solar,0);

  if (shiftPoly!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Spectrum ","shiftPoly",shiftPoly,0);

  // Return

  NDET=oldNDET;

  return rc;
 }

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_ApplyCalibration
// ----------------------------------------------------------------------------
// PURPOSE         Apply the new calibration to cross sections to interpolate
//                 or to convolute and recalculate gaps
//
// INPUT           pTabFeno      original calibration
//                 newLambda     the new wavelength calibration to apply
//
// RETURN          return code
// ----------------------------------------------------------------------------

RC KURUCZ_ApplyCalibration(FENO *pTabFeno,double *newLambda,INDEX indexFenoColumn)
{
  // Declarations

  MATRIX_OBJECT wveDptStretch;    // for slit function file type, the wavelength dependent stretch is saved as a slit matrix
  double slitParam2;
  INDEX indexWindow;
  INT newDimL = 0;
  RC rc;

  // Initializations

  memset(&wveDptStretch,0,sizeof(MATRIX_OBJECT));
  rc=0;

  // Rebuild gaps

  doas_spectrum *new_range = spectrum_new();
  for (indexWindow = 0; indexWindow < pTabFeno->svd.Z; indexWindow++)
   {
    int pixel_start = FNPixel(newLambda,pTabFeno->svd.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
    int pixel_end = FNPixel(newLambda,pTabFeno->svd.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

    spectrum_append(new_range, pixel_start, pixel_end);

    newDimL += pixel_end - pixel_start +1;
   }

  if (newDimL != pTabFeno->svd.DimL)
   { // reallocate complete SVD structure.
    SVD_Free("KURUCZ_ApplyCalibration ",&pTabFeno->svd);
    pTabFeno->svd.DimL=newDimL;
    SVD_LocalAlloc("KURUCZ_ApplyCalibration ",&pTabFeno->svd);
   } // only update specrange
  else if(pTabFeno->svd.specrange != NULL)
   spectrum_destroy(pTabFeno->svd.specrange);

  pTabFeno->svd.specrange = new_range;

  // Force decomposition

  pTabFeno->Decomp=1;
  slitParam2=(pKuruczOptions->fwhmType!=SLIT_TYPE_INVPOLY)?(double)0.:(double)pKuruczOptions->invPolyDegree;

  if (pTabFeno->xsToConvolute &&                                                                // slit function to convolute
    ((pTabFeno->useKurucz==ANLYS_KURUCZ_REF) || (pTabFeno->useKurucz==ANLYS_KURUCZ_SPEC)) &&    // calibration applied on the reference or the spectrum
      pKuruczOptions->fwhmFit &&                                                                // fit of the slit function
     (pKuruczOptions->fwhmType==SLIT_TYPE_FILE))                                                // slit function is file type
   {
   	if (MATRIX_Allocate(&wveDptStretch,NDET,3,0,0,1,"ShiftVector")!=0)
   	 rc=ERROR_ID_ALLOC;
   	else
   	 {
   	 	memcpy(wveDptStretch.matrix[0],newLambda,sizeof(double)*NDET);
   	 	memcpy(wveDptStretch.matrix[1],pTabFeno->fwhmVector[0],sizeof(double)*NDET);
   	 	memcpy(wveDptStretch.matrix[2],pTabFeno->fwhmVector[1],sizeof(double)*NDET);

   	 	if (!(rc=SPLINE_Deriv2(wveDptStretch.matrix[0],wveDptStretch.matrix[1],wveDptStretch.deriv2[1],wveDptStretch.nl,"XSCONV_TypeStandard ")))
   	 	 rc=SPLINE_Deriv2(wveDptStretch.matrix[0],wveDptStretch.matrix[2],wveDptStretch.deriv2[2],wveDptStretch.nl,"XSCONV_TypeStandard ");
   	 }
   }

  if (((pTabFeno->rcKurucz=ANALYSE_XsInterpolation(pTabFeno,newLambda,indexFenoColumn))!=ERROR_ID_NO) ||
       (pTabFeno->xsToConvolute && ((pTabFeno->useKurucz==ANLYS_KURUCZ_REF) || (pTabFeno->useKurucz==ANLYS_KURUCZ_SPEC)) &&
      ((pKuruczOptions->fwhmFit && (pKuruczOptions->fwhmType!=SLIT_TYPE_FILE) && ((pTabFeno->rcKurucz=ANALYSE_XsConvolution(pTabFeno,newLambda,NULL,NULL,pKuruczOptions->fwhmType,pTabFeno->fwhmVector[0],(pKuruczOptions->fwhmType!=SLIT_TYPE_INVPOLY)?pTabFeno->fwhmVector[1]:&slitParam2,indexFenoColumn,0))!=ERROR_ID_NO)) ||
       (pKuruczOptions->fwhmFit && (pKuruczOptions->fwhmType==SLIT_TYPE_FILE) && ((pTabFeno->rcKurucz=ANALYSE_XsConvolution(pTabFeno,newLambda,&KURUCZ_buffers[indexFenoColumn].slitFunction,&wveDptStretch,pKuruczOptions->fwhmType,&slitParam2,&slitParam2,indexFenoColumn,1))!=ERROR_ID_NO)) ||
      (!pKuruczOptions->fwhmFit && ((pTabFeno->rcKurucz=ANALYSE_XsConvolution(pTabFeno,newLambda,&ANALYSIS_slit,&ANALYSIS_slit2,pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2,indexFenoColumn,pSlitOptions->slitFunction.slitWveDptFlag))!=ERROR_ID_NO)))))

   rc=pTabFeno->rcKurucz;

  // Release allocated matrix object

  MATRIX_Free(&wveDptStretch,"XSCONV_TypeStandard ");

  // Return

  return rc;
 }

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_Reference
// ----------------------------------------------------------------------------
// PURPOSE         browse analysis windows and apply Kurucz if needed on reference spectrum
//
// INPUT           instrFunction  instrumental function to apply on reference spectrum
//                 refFlag        1 to apply Kurucz on the daily selected reference spectrum
//                 saveFlag       1 to save the calibration results in the data Window
//
//            in the case of satellites measurements :
//
//                 gomeRefFlag=0 means that irradiance is used as etalon spectrum
//                 gomeRefFlag=1 means that a reference spectrum is given
//
// RETURN          return code
// ----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC KURUCZ_Reference(double *instrFunction,INDEX refFlag,INT saveFlag,INT gomeFlag,void *responseHandle,INDEX indexFenoColumn)
{
  // Declarations

  FENO            *pTabFeno,*pTabRef;                               // browse analysis windows
  double          *reference;                                                   // reference spectrum to align on Kurucz

  INT              maxParam,
                   msgCount,
                   nKuruczFeno,
                   nBadKuruczFeno;
  INDEX            indexFeno,                                                   // browse analysis windows
                   indexRef;                                                    // index of another analysis window with the same reference spectrum
  KURUCZ *pKurucz;
  INDEX kuruczIndexRow;
  RC               rc;                                                          // return code

  // Initializations

  pKurucz=&KURUCZ_buffers[indexFenoColumn];

  rc=ERROR_ID_NO;
  msgCount=0;

  // Allocate buffers

  if ((reference=(double *)MEMORY_AllocDVector("KURUCZ_Reference ","spectrum",0,NDET-1))==NULL)
    rc=ERROR_ID_ALLOC;
  else
   {
    // Browse analysis windows and apply Kurucz alignment on the specified reference spectrum if needed

    for (indexFeno=0,nKuruczFeno=nBadKuruczFeno=0;indexFeno<NFeno;indexFeno++)
     {
      pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

      if (!pTabFeno->hidden && ((pTabFeno->useKurucz==ANLYS_KURUCZ_REF) || (pTabFeno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC)) &&
          (pTabFeno->gomeRefFlag==gomeFlag) &&
        ((!refFlag && (pTabFeno->useEtalon || (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE))) ||
         ((refFlag==1) && !pTabFeno->useEtalon)))

       nKuruczFeno++;
     }

    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     {
      pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

      if (!pTabFeno->hidden && ((pTabFeno->useKurucz==ANLYS_KURUCZ_REF) || (pTabFeno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC)) &&
          (pTabFeno->gomeRefFlag==gomeFlag) &&
        ((!refFlag && (pTabFeno->useEtalon || (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE))) ||
         ((refFlag==1) && !pTabFeno->useEtalon)))
       {
       	memcpy(reference,(pTabFeno->useEtalon)?pTabFeno->SrefEtalon:pTabFeno->Sref,sizeof(double)*pTabFeno->NDET);

       	kuruczIndexRow=(pKurucz->hrSolar.nc>indexFenoColumn+1)?indexFenoColumn+1:1;

       	if ((pTabFeno->NDET==pKurucz->hrSolar.nl) &&
       	     VECTOR_Equal(pKurucz->hrSolar.matrix[0],pTabFeno->LambdaRef,pTabFeno->NDET,(double)1.e-7) &&
       	     VECTOR_Equal(pKurucz->hrSolar.matrix[kuruczIndexRow],reference,pTabFeno->NDET,(double)1.e-7))

       	 pTabFeno->rcKurucz=ERROR_ID_NO;
       	else
       	 {
       	  // Apply instrumental corrections on reference spectrum

          if (((indexRef=KuruczSearchReference(indexFeno,indexFenoColumn))<NFeno) && (indexRef!=indexFeno) &&
              ((indexRef<indexFeno) ||
              ((refFlag && !pTabFeno->useEtalon) && ((TabFeno[indexFenoColumn][indexRef].refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE) || TabFeno[indexFenoColumn][indexRef].useEtalon))))
           {
            pTabRef=&TabFeno[indexFenoColumn][indexRef];
            pTabFeno->rcKurucz=pTabRef->rcKurucz;

            memcpy(pTabFeno->LambdaK,pTabRef->LambdaK,sizeof(double)*pTabFeno->NDET);

            if (pKuruczOptions->fwhmFit)
             {
              for (maxParam=0;maxParam<MAX_KURUCZ_FWHM_PARAM;maxParam++)
               if (pTabFeno->fwhmVector[maxParam]!=NULL)
                {
                 memcpy(pTabFeno->fwhmPolyRef[maxParam],pTabRef->fwhmPolyRef[maxParam],sizeof(double)*(pKuruczOptions->fwhmPolynomial+1));
                 memcpy(pTabFeno->fwhmVector[maxParam],pTabRef->fwhmVector[maxParam],sizeof(double)*NDET);
                 if (pTabFeno->fwhmDeriv2[maxParam]!=NULL)
                  memcpy(pTabFeno->fwhmDeriv2[maxParam],pTabRef->fwhmDeriv2[maxParam],sizeof(double)*NDET);
                }
               else
                break;
             }
           }
          else
           {
            // Apply Kurucz for building new calibration for reference

            if ((rc=pTabFeno->rcKurucz=KURUCZ_Spectrum(pTabFeno->LambdaRef,pTabFeno->LambdaK,reference,pKurucz->solar,instrFunction,
                 1,pTabFeno->windowName,pTabFeno->fwhmPolyRef,pTabFeno->fwhmVector,pTabFeno->fwhmDeriv2,saveFlag,indexFeno,responseHandle,indexFenoColumn))!=ERROR_ID_NO)

             goto EndKuruczReference;

           }

         if (!rc && !pTabFeno->rcKurucz)
          rc=KURUCZ_ApplyCalibration(pTabFeno,pTabFeno->LambdaK,indexFenoColumn);

          EndKuruczReference :

          if (rc>0)
           {
            if (pTabFeno->rcKurucz<=THREAD_EVENT_STOP)
             {
              rc=pTabFeno->rcKurucz;
              break;
             }
            else
             {
             	if (++nBadKuruczFeno==nKuruczFeno)
               {
                rc=pTabFeno->rcKurucz;
                break;
               }

              msgCount++;
             }
           }
         }

        if (pTabFeno->longPathFlag)                                             // !!! Anoop
         {
          memcpy(pTabFeno->SrefEtalon,ANALYSE_ones,sizeof(double)*NDET);
          memcpy(pTabFeno->Sref,ANALYSE_ones,sizeof(double)*NDET);
         }
       }
     }
   }

  if (rc<THREAD_EVENT_STOP)
   rc=ERROR_ID_NO;

  // Release allocated buffers

  if (reference!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Reference","reference",reference,0);

  // Return

  return rc;
 }

// ====================
// RESOURCES MANAGEMENT
// ====================

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_Init
// ----------------------------------------------------------------------------
// PURPOSE         set the limits for each window of the calibration interval
//
// INPUT           in the case of satellites measurements :
//
//                    gomeRefFlag=0 means that irradiance is used as etalon spectrum
//                    gomeRefFlag=1 means that a reference spectrum is given
//
// RETURN          return code
// ----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
void KURUCZ_Init(INT gomeFlag,INDEX indexFenoColumn)
 {
  // Declarations

  INDEX indexFeno,indexWindow;
  INT nbWin;
  double Lambda_min,Lambda_max,Win_size;
  FENO *pTabFeno;
  SVD *pSvd;

  // Initialization

  nbWin=KURUCZ_buffers[indexFenoColumn].Nb_Win;

  // Browse analysis windows

  for (indexFeno=0;indexFeno<NFeno;indexFeno++)
   {
    pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

    if ((pTabFeno->gomeRefFlag==gomeFlag) &&
        (KURUCZ_buffers[indexFenoColumn].KuruczFeno[indexFeno].svdFeno!=NULL))
     {
      Lambda_min=pKuruczOptions->lambdaLeft;
      Lambda_max=pKuruczOptions->lambdaRight;

      Win_size=(double)(Lambda_max-Lambda_min)/nbWin;

      for (indexWindow=0;indexWindow<nbWin;indexWindow++)
       {
        pSvd=&KURUCZ_buffers[indexFenoColumn].KuruczFeno[indexFeno].svdFeno[indexWindow];

        Lambda_max=Lambda_min+Win_size;

        int pixel_start=FNPixel(pTabFeno->LambdaRef,Lambda_min,pTabFeno->NDET,PIXEL_AFTER);
        int pixel_end=FNPixel(pTabFeno->LambdaRef,Lambda_max,pTabFeno->NDET,PIXEL_BEFORE);

        pSvd->specrange = spectrum_new();
        spectrum_append(pSvd->specrange, pixel_start, pixel_end);

        pSvd->DimL=pixel_end - pixel_start + 1;

        Lambda_min=Lambda_max;
       }
     }
   }
 }

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_Alloc
// ----------------------------------------------------------------------------
// PURPOSE         allocate all buffers needed for applying Kurucz procedures to a project
//
// INPUT           pProject     pointer to the current project
//                 lambda       original wavelength calibration
//                 indexKurucz  index of analysis window with Kurucz description for the current project;
//                 lambdaMin    lower limit of the wavelength calibration interval
//                 lambdaMax    upper limit of the wavelength calibration interval
//
// RETURN          return code
// ----------------------------------------------------------------------------

RC KURUCZ_Alloc(PROJECT *pProject,double *lambda,INDEX indexKurucz,double lambdaMin,double lambdaMax,INDEX indexFenoColumn)
 {
  // Declarations

  INT hFilterFlag;
  CROSS_REFERENCE *pTabCross;                                                   // cross sections list
  FENO            *pKuruczFeno,                                                 // analysis window with Kurucz description
                  *pTabFeno;

  char  kuruczFile[MAX_ITEM_TEXT_LEN+1],slitFile[MAX_ITEM_TEXT_LEN+1];
  INT    Nb_Win,shiftDegree,                                                    // substitution variables
         NTabCross,DimLMax;
  INDEX  i,indexFeno,indexWindow,indexParam,indexTabCross;                      // indexes for loops and arrays
  double Lambda_min,Lambda_max,                                                 // extrema in nm of a little window
         Win_size,step;                                                         // size of a little window in nm
  SVD   *pSvd,*pSvdFwhm;                                                        // pointers to svd environments
  KURUCZ *pKurucz;
  INDEX kuruczIndexRow;                                                                 // number of columns of the Kurucz Matrix
  RC rc;

  // Initializations

  pKurucz=&KURUCZ_buffers[indexFenoColumn];

  memset(&pKurucz->hrSolar,0,sizeof(MATRIX_OBJECT));
  memset(&pKurucz->slitFunction,0,sizeof(MATRIX_OBJECT));

  pKuruczFeno=&TabFeno[indexFenoColumn][indexKurucz];

  step=(double)0.;

  FILES_RebuildFileName(kuruczFile,(pKuruczOptions->fwhmFit)?pKuruczOptions->file:pSlitOptions->kuruczFile,1);
  FILES_RebuildFileName(slitFile,pKuruczOptions->slfFile,1);

  if ((hFilterFlag=((ANALYSE_phFilter->filterFunction!=NULL) &&                 // high pass filtering
                    (pKuruczFeno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) &&    // implemented only in DOAS fitting
                    (pKuruczFeno->analysisType==ANALYSIS_TYPE_FWHM_KURUCZ) &&   // for calibration with slit function fitting
                   ((pKuruczFeno->indexSol==ITEM_NONE) ||                       // doesn't fit the Sol non linear parameter
                    (pKuruczFeno->TabCross[pKuruczFeno->indexSol].FitParam==ITEM_NONE)))?1:0)==1)

   pKurucz->solarFGap=(int)floor(ANALYSE_phFilter->filterWidth*sqrt(ANALYSE_phFilter->filterNTimes)+0.5);

  // Load options from Kurucz tab page from project properties

  Nb_Win=pKuruczOptions->windowsNumber;
  shiftDegree=pKuruczOptions->shiftPolynomial;

  rc=ERROR_ID_NO;

  // Check validity of entry fields in Kurucz tab page of project properties panel

  if (pKuruczOptions->fwhmFit)
   {
    pKurucz->fwhmDegree=pKuruczOptions->fwhmPolynomial;

// QDOAS ???  TESTS TO MOVE TO THE GUI     if ((fwhmDegree<0) || (fwhmDegree>(UINT)10))
// QDOAS ???  TESTS TO MOVE TO THE GUI      rc=ERROR_SetLast("KURUCZ_Alloc",ERROR_TYPE_FATAL,ERROR_ID_MSGBOX_OUTOFRANGE,"\'Kurucz fwhm polynomial order\'",0,(UINT)10);

    if (pKuruczOptions->fwhmType==SLIT_TYPE_FILE)
     {
      // Load file

      if (!strlen(slitFile))
       rc=ERROR_SetLast("KURUCZ_Alloc",ERROR_TYPE_FATAL,ERROR_ID_MSGBOX_FIELDEMPTY,"Slit File");
      else
       rc=MATRIX_Load(slitFile,&pKurucz->slitFunction,0 /* line base */,0 /* column base */,0,0,
                     -9999.,9999.,1,0,"KURUCZ_Alloc");
     }

    if (rc!=ERROR_ID_NO)
     goto EndKuruczAlloc;
   }

// QDOAS ???  TESTS TO MOVE TO THE GUI  if (!strlen(pKuruczOptions->file))
// QDOAS ???  TESTS TO MOVE TO THE GUI   THRD_Error(ERROR_TYPE_FATAL,(rc=IDS_MSGBOX_FIELDEMPTY),"KURUCZ_Alloc ","Solar reference file");
// QDOAS ???  TESTS TO MOVE TO THE GUI  else if ((Nb_Win<=0) || (Nb_Win>(UINT)32767))
// QDOAS ???  TESTS TO MOVE TO THE GUI   THRD_Error(ERROR_TYPE_FATAL,(rc=THREAD_MSGBOX_OUTOFRANGE),"KURUCZ_Alloc ","\'Kurucz number of windows\'",1,(UINT)32767);
// QDOAS ???  TESTS TO MOVE TO THE GUI  else if ((shiftDegree<0) || (shiftDegree>(UINT)10))
// QDOAS ???  TESTS TO MOVE TO THE GUI   THRD_Error(ERROR_TYPE_FATAL,(rc=THREAD_MSGBOX_OUTOFRANGE),"KURUCZ_Alloc ","\'Kurucz shift polynomial order\'",0,(UINT)10);
// QDOAS ???  TESTS TO MOVE TO THE GUI  else if (pKuruczOptions->lambdaLeft>=pKuruczOptions->lambdaRight)
// QDOAS ???  TESTS TO MOVE TO THE GUI   THRD_Error(ERROR_TYPE_FATAL,(rc=ERROR_ID_GAPS),"KURUCZ_Alloc ",pKuruczOptions->lambdaLeft,pKuruczOptions->lambdaRight);
// QDOAS ???  TESTS TO MOVE TO THE GUI
// QDOAS ???  TESTS TO MOVE TO THE GUI  // Allocate buffers
// QDOAS ???  TESTS TO MOVE TO THE GUI
// QDOAS ???  TESTS TO MOVE TO THE GUI  else

  if (((pKurucz->KuruczFeno=(KURUCZ_FENO *)MEMORY_AllocBuffer("KURUCZ_Alloc ","KuruczFeno",NFeno,sizeof(KURUCZ_FENO),0,MEMORY_TYPE_STRUCT))==NULL) ||
      ((pKurucz->solar=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","solar",0,NDET-1))==NULL) ||           // solar spectrum
      ((pKurucz->offset=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","offset",0,NDET-1))==NULL) ||         // offset spectrum
      ((pKurucz->Pcalib=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","Pcalib",1,shiftDegree+1))==NULL) ||  // coefficients of the polynomial
      ((pKurucz->VLambda=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","VLambda",1,Nb_Win))==NULL) ||       // solution of the system
      ((pKurucz->pixMid=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","pixMid",1,Nb_Win))==NULL) ||         // pixels at the middle of little windows
      ((pKurucz->VShift=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","VShift",1,Nb_Win))==NULL) ||         // shift applied on pixels
      ((pKurucz->VSig=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","VSig",1,Nb_Win))==NULL) ||             // error on shift applied on pixels
      ((pKurucz->VPix=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","VPix",1,Nb_Win))==NULL) ||             // pixels with shift correction
      ((pKurucz->NIter=(INT *)MEMORY_AllocBuffer("KURUCZ_Alloc ","NIter",Nb_Win,sizeof(INT),0,MEMORY_TYPE_INT))==NULL) ||
      (hFilterFlag &&
       (((pKurucz->lambdaF=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","lambdaF",0,NDET+2*pKurucz->solarFGap-1))==NULL) ||
        ((pKurucz->solarF=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","solarF",0,NDET+2*pKurucz->solarFGap-1))==NULL) ||
        ((pKurucz->solarF2=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","solarF2",0,NDET+2*pKurucz->solarFGap-1))==NULL))))
   {
    rc=ERROR_ID_ALLOC;
    goto EndKuruczAlloc;
   } else {
    // null-initialize KuruczFeno structures.
    for (indexFeno=0;indexFeno<NFeno;indexFeno++) {
     memset(&pKurucz->KuruczFeno[indexFeno],0,sizeof(KURUCZ_FENO));
    }
  }

  if (hFilterFlag && pKurucz->solarFGap)
   {
    if (lambda[NDET-1]-lambda[0]+1==NDET)
     {
      for (i=0;i<NDET+2*pKurucz->solarFGap;i++)
       pKurucz->lambdaF[i]=(double)0.;
     }
    else
     {
      memcpy(&pKurucz->lambdaF[pKurucz->solarFGap],lambda,sizeof(double)*NDET);

      step=(lambda[NDET-1]-lambda[0])/NDET;

      for (i=0;i<pKurucz->solarFGap;i++)
       {
        pKurucz->lambdaF[i]=lambda[0]-step*(pKurucz->solarFGap-i);
        pKurucz->lambdaF[NDET+pKurucz->solarFGap+i]=lambda[NDET-1]+step*(i+1);
       }
     }
   }

  // Load and normalize solar spectrum

  if ( !strlen(kuruczFile) ) {
   rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_MSGBOX_FIELDEMPTY, "Solar Ref. File");
  } else if ( !(rc=XSCONV_LoadCrossSectionFile(&pKurucz->hrSolar,kuruczFile,lambdaMin-7.-step*pKurucz->solarFGap,lambdaMax+7.+step*pKurucz->solarFGap,(double)0.,CONVOLUTION_CONVERSION_NONE)))
   {
    kuruczIndexRow=(pKurucz->hrSolar.nc>indexFenoColumn+1)?indexFenoColumn+1:1;

   	               // If the fwhm of the slit function is fitted, then we can use the same high resolution solar
   	               // spectrum.  If we do not fit the slit function, the solar spectrum has to be preconvoluted.
   	               // For OMI, the number of rows is 60 and the number of preconvoluted spectra should be 60 too.
   	               // So in principle, a test if the slit function should be enough but why we couldn't use the
   	               // same spectrum ?

    if (((rc=VECTOR_NormalizeVector(pKurucz->hrSolar.matrix[kuruczIndexRow]-1,pKurucz->hrSolar.nl,NULL,"KURUCZ_Alloc "))!=ERROR_ID_NO) ||
        ((rc=SPLINE_Deriv2(pKurucz->hrSolar.matrix[0],pKurucz->hrSolar.matrix[kuruczIndexRow],pKurucz->hrSolar.deriv2[kuruczIndexRow],pKurucz->hrSolar.nl,"KURUCZ_Alloc "))!=ERROR_ID_NO))

     goto EndKuruczAlloc;

    memcpy(pKurucz->solar,ANALYSE_zeros,sizeof(double)*NDET);

    // Initialize other fields of global structure

    pKurucz->indexKurucz=indexKurucz;

    pKurucz->Nb_Win=Nb_Win;
    pKurucz->shiftDegree=shiftDegree;

    pKurucz->displayFit=(char)pKuruczOptions->displayFit;                // display fit flag
    pKurucz->displayResidual=(char)pKuruczOptions->displayResidual;      // display new calibration flag
    pKurucz->displayShift=(char)pKuruczOptions->displayShift;            // display shift in each pixel flag
    pKurucz->displaySpectra=(char)pKuruczOptions->displaySpectra;        // display shift in each pixel flag

    pKurucz->units=pProject->analysis.units;
    pKurucz->method=(char)pKuruczOptions->analysisMethod;

    // Allocate one svd environment for each little window

    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     {
      memset(&pKurucz->KuruczFeno[indexFeno],0,sizeof(KURUCZ_FENO));
      pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

      if ((pTabFeno->hidden==1) ||
         ((THRD_id!=THREAD_TYPE_KURUCZ) && !pTabFeno->hidden && pTabFeno->useKurucz))
       {
        Lambda_min=pKuruczOptions->lambdaLeft;
        Lambda_max=pKuruczOptions->lambdaRight;

        Win_size=(double)(Lambda_max-Lambda_min)/Nb_Win;
        DimLMax=2*NDET/Nb_Win+1;

        if ((pKurucz->KuruczFeno[indexFeno].Grid=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","Grid",0,Nb_Win-1))==NULL)
         rc=ERROR_ID_ALLOC;
        else if ((pKurucz->KuruczFeno[indexFeno].svdFeno=(SVD *)MEMORY_AllocBuffer("KURUCZ_Alloc ","svdFeno",Nb_Win,sizeof(SVD),0,MEMORY_TYPE_STRUCT))==NULL)                           // svd environments
         rc=ERROR_ID_ALLOC;
        else if (pKuruczOptions->fwhmFit && ((pKurucz->KuruczFeno[indexFeno].fft=(FFT *)MEMORY_AllocBuffer("KURUCZ_Alloc ","fft",Nb_Win,sizeof(FFT),0,MEMORY_TYPE_STRUCT))==NULL))                           // svd environments
         rc=ERROR_ID_ALLOC;
        else if ((pKurucz->KuruczFeno[indexFeno].chiSquare=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","chiSquare",0,Nb_Win-1))==NULL)
         rc=ERROR_ID_ALLOC;
        else if ((pKurucz->KuruczFeno[indexFeno].rms=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","rms",0,Nb_Win-1))==NULL)
         rc=ERROR_ID_ALLOC;
        else if ((pKurucz->KuruczFeno[indexFeno].wve=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","wve",0,Nb_Win-1))==NULL)
         rc=ERROR_ID_ALLOC;
        else if ((pKurucz->KuruczFeno[indexFeno].nIter=(int *)MEMORY_AllocBuffer("KURUCZ_Alloc ","nIter",Nb_Win,sizeof(int),0,MEMORY_TYPE_INT))==NULL)                           // svd environments
         rc=ERROR_ID_ALLOC;
        else if ((pKurucz->KuruczFeno[indexFeno].results=(CROSS_RESULTS **)MEMORY_AllocBuffer("KURUCZ_Alloc ","results",Nb_Win,sizeof(CROSS_RESULTS *),0,MEMORY_TYPE_STRUCT))==NULL)
         rc=ERROR_ID_ALLOC;

        if (rc)
         goto EndKuruczAlloc;
        else
         {
          if (pKurucz->KuruczFeno[indexFeno].svdFeno!=NULL)
           memset(pKurucz->KuruczFeno[indexFeno].svdFeno,0,Nb_Win*sizeof(SVD));
          if (pKurucz->KuruczFeno[indexFeno].fft!=NULL)
           memset(pKurucz->KuruczFeno[indexFeno].fft,0,Nb_Win*sizeof(FFT));
          if (pKurucz->KuruczFeno[indexFeno].results!=NULL)
           memset(pKurucz->KuruczFeno[indexFeno].results,0,Nb_Win*sizeof(CROSS_RESULTS *));
         }

        for (indexWindow=0;indexWindow<Nb_Win;indexWindow++)
         {
          pSvd=&pKurucz->KuruczFeno[indexFeno].svdFeno[indexWindow];
          memcpy(pSvd,&pKuruczFeno->svd,sizeof(SVD));
          pSvd->Z=1;
          pSvd->DimL=DimLMax;

          Lambda_max=Lambda_min+Win_size;
          pKurucz->KuruczFeno[indexFeno].Grid[indexWindow]=Lambda_max;

          if ((pKurucz->KuruczFeno[indexFeno].results[indexWindow]=(CROSS_RESULTS *)MEMORY_AllocBuffer("KURUCZ_Alloc ","KuruczFeno(results)",pKuruczFeno->NTabCross,sizeof(CROSS_RESULTS),0,MEMORY_TYPE_STRUCT))==NULL)
           {
            rc=ERROR_ID_ALLOC;
            goto EndKuruczAlloc;
           }
          else if ((rc=SVD_LocalAlloc("KURUCZ_Alloc (1)",pSvd))!=ERROR_ID_NO)
           goto EndKuruczAlloc;
          else if (pKuruczOptions->fwhmFit)
           {
            INT hrDeb,hrFin,hrN,fftSize;
            double *fftIn;
            FFT *pfft;
            INDEX i;

            pfft=&pKurucz->KuruczFeno[indexFeno].fft[indexWindow];

            hrDeb=FNPixel(pKurucz->hrSolar.matrix[0],Lambda_min-3.,pKurucz->hrSolar.nl,PIXEL_AFTER);
            hrFin=FNPixel(pKurucz->hrSolar.matrix[0],Lambda_max+3.,pKurucz->hrSolar.nl,PIXEL_BEFORE);

            if (hrDeb==hrFin)
             {
              rc=ERROR_ID_POW;
              goto EndKuruczAlloc;
             }

            hrN=pfft->oldSize=(hrFin-hrDeb+1);
            fftSize=pfft->fftSize=(int)pow((double)2.,ceil(log((double)hrN)/log((double)2.)));

            if (((fftIn=pfft->fftIn=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fftIn",1,fftSize))==NULL) ||
                ((pfft->fftOut=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fftOut",1,fftSize))==NULL) ||
                ((pfft->invFftIn=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","invFftIn",1,fftSize))==NULL) ||
                ((pfft->invFftOut=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","invFftOut",1,fftSize))==NULL))
             {
              rc=ERROR_ID_ALLOC;
              goto EndKuruczAlloc;
             }

            memcpy(fftIn+1,pKurucz->hrSolar.matrix[1]+hrDeb,sizeof(double)*hrN);   // When the slit function is fitted, we use a high resolution solar spectrum (2 columns only)

            for (i=hrN+1;i<=fftSize;i++)
             fftIn[i]=fftIn[2*hrN-i];

            realft(pfft->fftIn,pfft->fftOut,fftSize,1);

            memcpy(fftIn+1,pKurucz->hrSolar.matrix[0]+hrDeb,sizeof(double)*hrN);  // Reuse fftIn for high resolution wavelength safe keeping
           }

          Lambda_min=Lambda_max;
         }
       }
     }

    // Allocate svd environment for Kurucz analysis window

    pSvd=&pKuruczFeno->svd;

    pSvd->DimL=Nb_Win;
    pSvd->DimC=shiftDegree+1;

    if ((rc=SVD_LocalAlloc("KURUCZ_Alloc (2)",pSvd))!=ERROR_ID_NO)
     goto EndKuruczAlloc;

    // Allocate buffers for cross sections fits

    if (pKurucz->displayFit)
     {
      for (indexTabCross=0,NTabCross=0;indexTabCross<pKuruczFeno->NTabCross;indexTabCross++)
       {
        pTabCross=&pKuruczFeno->TabCross[indexTabCross];
        if (pTabCross->IndSvdA && pTabCross->display &&
           (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS))

         NTabCross++;
       }

      if (NTabCross && ((rc=MATRIX_Allocate(&pKurucz->crossFits,NDET,NTabCross,0,0,0,"KURUCZ_Alloc "))!=0))
       goto EndKuruczAlloc;
     }

    // Allocate buffers for coefficients of polynomials fitting fwhm

    if (pKuruczOptions->fwhmFit)
     {
      pSvdFwhm=&pKurucz->svdFwhm;
      pSvdFwhm->DimL=Nb_Win;
      pSvdFwhm->DimC=pKurucz->fwhmDegree+1;

      if ((rc=SVD_LocalAlloc("KURUCZ_Alloc (3)",pSvdFwhm))!=ERROR_ID_NO)
       goto EndKuruczAlloc;

      for (indexParam=0;(indexParam<MAX_KURUCZ_FWHM_PARAM) && !rc;indexParam++)
       {
        if (((pKurucz->fwhm[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhm",0,Nb_Win-1))==NULL) ||
            ((pKurucz->fwhmSigma[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhmSigma",0,Nb_Win-1))==NULL) ||
            ((pKurucz->fwhmPolySpec[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhmPolySpec",0,pKurucz->fwhmDegree))==NULL) ||
            ((pKurucz->fwhmVector[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhmVector",0,NDET-1))==NULL) ||
            ((pKurucz->fwhmDeriv2[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhmVector",0,NDET-1))==NULL))

         rc=ERROR_ID_ALLOC;

        else
         {
          memcpy(pKurucz->fwhmVector[indexParam],ANALYSE_zeros,sizeof(double)*NDET);

          for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
           {
            pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

            if ((pKuruczFeno->indexFwhmParam[indexParam]!=ITEM_NONE) &&
                 !pTabFeno->hidden && pTabFeno->useKurucz &&
              (((pTabFeno->fwhmPolyRef[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhmPolyRef",0,pKurucz->fwhmDegree))==NULL) ||
               ((pTabFeno->fwhmVector[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhmVector",0,NDET-1))==NULL) ||
               ((pKuruczFeno->TabCross[pKuruczFeno->indexFwhmParam[indexParam]].FitParam!=ITEM_NONE) &&
               ((pTabFeno->fwhmDeriv2[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhmDeriv2",0,NDET-1))==NULL))))

             rc=ERROR_ID_ALLOC;
           }
         }
       }
     }

    if (rc)
     goto EndKuruczAlloc;

    if (hFilterFlag && pKurucz->solarFGap && (lambda[NDET-1]-lambda[0]+1!=NDET) &&
     (((rc=SPLINE_Vector(pKurucz->hrSolar.matrix[0],pKurucz->hrSolar.matrix[kuruczIndexRow],pKurucz->hrSolar.deriv2[kuruczIndexRow],pKurucz->hrSolar.nl,
                            pKurucz->lambdaF,pKurucz->solarF,NDET+2*pKurucz->solarFGap,pAnalysisOptions->interpol,"KURUCZ_Alloc "))!=0) ||
      ((rc=FILTER_Vector(ANALYSE_phFilter,pKurucz->solarF,pKurucz->solarF,NDET+2*pKurucz->solarFGap,PRJCT_FILTER_OUTPUT_LOW))!=0) ||
      ((rc=SPLINE_Deriv2(pKurucz->lambdaF,pKurucz->solarF,pKurucz->solarF2,NDET+2*pKurucz->solarFGap,"KURUCZ_Alloc (solarF) "))!=0)))

     goto EndKuruczAlloc;

    KURUCZ_Init(1,indexFenoColumn);
   }

  // Return

  EndKuruczAlloc :

  return rc;
 }

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_Free
// ----------------------------------------------------------------------------
// PURPOSE         release buffers allocated for Kurucz procedures;
// ----------------------------------------------------------------------------

void KURUCZ_Free(void)
 {
  // Declarations

  INDEX indexWindow,indexParam,indexFeno,indexFenoColumn;
  KURUCZ_FENO *pKFeno;
  KURUCZ *pKurucz;

  for (indexFenoColumn=0;indexFenoColumn<ANALYSE_swathSize;indexFenoColumn++)
   {
   	pKurucz=&KURUCZ_buffers[indexFenoColumn];

    MATRIX_Free(&pKurucz->hrSolar,"KURUCZ_Free");
    MATRIX_Free(&pKurucz->slitFunction,"KURUCZ_Free");

    if (pKurucz->solar!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","solar",pKurucz->solar,0);
    if (pKurucz->lambdaF!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","lambdaF",pKurucz->lambdaF,0);
    if (pKurucz->solarF!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","solarF",pKurucz->solarF,0);
    if (pKurucz->solarF2!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","solarF2",pKurucz->solarF2,0);
    if (pKurucz->offset!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","offset",pKurucz->offset,0);
    if (pKurucz->Pcalib!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","Pcalib",pKurucz->Pcalib,1);
    if (pKurucz->VLambda!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","VLambda",pKurucz->VLambda,1);
    if (pKurucz->VShift!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","VShift",pKurucz->VShift,1);
    if (pKurucz->VSig!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","VSig",pKurucz->VSig,1);
    if (pKurucz->VPix!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","VPix",pKurucz->VPix,1);
    if (pKurucz->pixMid!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","pixMid",pKurucz->pixMid,1);
    if (pKurucz->NIter!=NULL)
     MEMORY_ReleaseBuffer("KURUCZ_Free ","NIter",pKurucz->NIter);

    for (indexParam=0;(indexParam<MAX_KURUCZ_FWHM_PARAM);indexParam++)
     {
      if (pKurucz->fwhm[indexParam]!=NULL)
       MEMORY_ReleaseDVector("KURUCZ_Free ","fwhm",pKurucz->fwhm[indexParam],0);
      if (pKurucz->fwhmSigma[indexParam]!=NULL)
       MEMORY_ReleaseDVector("KURUCZ_Free ","fwhmSigma",pKurucz->fwhmSigma[indexParam],0);
      if (pKurucz->fwhmPolySpec[indexParam]!=NULL)
       MEMORY_ReleaseDVector("KURUCZ_Free ","fwhmPolySpec",pKurucz->fwhmPolySpec[indexParam],0);
      if (pKurucz->fwhmVector[indexParam]!=NULL)
       MEMORY_ReleaseDVector("KURUCZ_Free ","fwhmVector",pKurucz->fwhmVector[indexParam],0);
      if (pKurucz->fwhmDeriv2[indexParam]!=NULL)
       MEMORY_ReleaseDVector("KURUCZ_Free ","fwhmDeriv2",pKurucz->fwhmDeriv2[indexParam],0);
     }

    SVD_Free("KURUCZ_Free (3)",&pKurucz->svdFwhm);
    MATRIX_Free(&pKurucz->crossFits,"KURUCZ_Free");

    if (pKurucz->KuruczFeno!=NULL)
     {
      for (indexFeno=0;indexFeno<NFeno;indexFeno++)
       {
        pKFeno=&pKurucz->KuruczFeno[indexFeno];

        // Grid

        if (pKFeno->Grid!=NULL)
         MEMORY_ReleaseDVector("KURUCZ_Free ","Grid",pKFeno->Grid,0);

        // svdFeno

        if (pKFeno->svdFeno!=NULL)
         {
          for (indexWindow=0;indexWindow<pKurucz->Nb_Win;indexWindow++)
           SVD_Free("KURUCZ_Free (1)",&pKFeno->svdFeno[indexWindow]);

          MEMORY_ReleaseBuffer("KURUCZ_Free ","svdFeno",pKFeno->svdFeno);
         }

        // fft

        if (pKFeno->fft!=NULL)
         {
          for (indexWindow=0;indexWindow<pKurucz->Nb_Win;indexWindow++)
           {
            if (pKFeno->fft[indexWindow].fftIn!=NULL)
             MEMORY_ReleaseDVector("KURUCZ_Free ","fftIn",pKFeno->fft[indexWindow].fftIn,1);
            if (pKFeno->fft[indexWindow].fftOut!=NULL)
             MEMORY_ReleaseDVector("KURUCZ_Free ","fftOut",pKFeno->fft[indexWindow].fftOut,1);
            if (pKFeno->fft[indexWindow].invFftIn!=NULL)
             MEMORY_ReleaseDVector("KURUCZ_Free ","invFftIn",pKFeno->fft[indexWindow].invFftIn,1);
            if (pKFeno->fft[indexWindow].invFftOut!=NULL)
             MEMORY_ReleaseDVector("KURUCZ_Free ","invFftOut",pKFeno->fft[indexWindow].invFftOut,1);
           }

          MEMORY_ReleaseBuffer("KURUCZ_Free ","fft",pKFeno->fft);
         }

        if (pKFeno->results!=NULL)
         {
          for (indexWindow=0;indexWindow<pKurucz->Nb_Win;indexWindow++)
           if (pKFeno->results[indexWindow]!=NULL)
            MEMORY_ReleaseBuffer("KURUCZ_Free ","results",pKFeno->results[indexWindow]);

          MEMORY_ReleaseBuffer("KURUCZ_Free ","results",pKFeno->results);
         }

        if (pKFeno->chiSquare!=NULL)
         MEMORY_ReleaseDVector("KURUCZ_Free ","chiSquare",pKFeno->chiSquare,0);
        if (pKFeno->wve!=NULL)
         MEMORY_ReleaseDVector("KURUCZ_Free ","wve",pKFeno->wve,0);
        if (pKFeno->rms!=NULL)
         MEMORY_ReleaseDVector("KURUCZ_Free ","rms",pKFeno->rms,0);
        if (pKFeno->nIter!=NULL)
         MEMORY_ReleaseBuffer("KURUCZ_Free ","nIter",pKFeno->nIter);
       }

      MEMORY_ReleaseBuffer("KURUCZ_Free ","KuruczFeno",pKurucz->KuruczFeno);
     }

    memset(pKurucz,0,sizeof(KURUCZ));
   }
 }

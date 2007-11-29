
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

KURUCZ KURUCZ_buffers;
FFT *pKURUCZ_fft;

// =================
// KURUCZ PROCEDURES
// =================

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_SearchReference
// ----------------------------------------------------------------------------
// PURPOSE         search for a reference spectrum in analysis windows on which Kurucz has already been applied
//
// INPUT           indexRefFeno   index of the analysis window with the reference to search for in other analysis windows;
//
// RETURN          indexFeno      index of the analysis window in which reference has been found;
// ----------------------------------------------------------------------------

INDEX KuruczSearchReference(INDEX indexRefFeno)
 {
  // Declarations

  double *reference;
  INDEX indexFeno;
  FENO *pTabFeno;

  // Initialization

  reference=(TabFeno[indexRefFeno].useEtalon)?TabFeno[indexRefFeno].SrefEtalon:TabFeno[indexRefFeno].Sref;

  // Search for reference in analysis windows

  for (indexFeno=0;indexFeno<NFeno;indexFeno++)
   {
    pTabFeno=&TabFeno[indexFeno];

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
                   UCHAR displayFlag,UCHAR *windowTitle,double **coeff,double **fwhmVector,double **fwhmDeriv2,INT saveFlag,INDEX indexFeno,void *responseHandle)
 {
  // Declarations

  UCHAR            string[MAX_ITEM_TEXT_LEN+1];
  CROSS_REFERENCE *TabCross,*pTabCross;
  CROSS_RESULTS   *pResults,*Results;                                           // pointer to results associated to a symbol
  SVD             *svdFeno;                                                     // svd environments associated to list of little windows
  double          *VPix,*VSig,*Pcalib,                                          // polynomial coefficients computation
                  *shiftPoly,
                  *pixMid,*VLambda,*VShift,*Grid,                               // display
                 **fwhm,**fwhmSigma,                                            // substitution vectors
                  *solar,                                                       // solar spectrum
                  *offset,                                                      // offset
                   newPix,
                   Square;                                                      // Chi square returned by 'CurFitMethod'
  INT              Nb_Win,maxParam,                                             // number of little windows
                   Niter,                                                       // number of iterations
                   oldNDET;
  INDEX            indexWindow,                                                 // browse little windows
                   indexParam,
                   indexTabCross,
                   indexCrossFit,
                   indexGraph,
                   i,j,j0,k;                                                    // temporary indexes
  RC               rc;                                                          // return code
  plot_data_t      spectrumData[2];

  // Initializations

  solar=NULL;
  oldNDET    = NDET;

  if ((shiftPoly=(double *)MEMORY_AllocDVector("KURUCZ_Spectrum ","shiftPoly",0,NDET-1))==NULL)
    rc=ERROR_ID_ALLOC;
  else
   {
    // Use substitution variables

    Feno=&TabFeno[KURUCZ_buffers.indexKurucz];
    TabCross=Feno->TabCross;

    memcpy(Feno->LambdaK,oldLambda,sizeof(double)*NDET);

    rc=ANALYSE_XsInterpolation(Feno,oldLambda);

    Results=Feno->TabCrossResults;
    pResults=&Feno->TabCrossResults[(Feno->indexSpectrum!=ITEM_NONE)?Feno->indexSpectrum:Feno->indexReference];
    rc=ERROR_ID_NO;

// QDOAS ???    if ((maxGraphV=PRJCT_itemList[KURUCZ_buffers.indexProject].spectra.maxGraphV)<1)
// QDOAS ???     maxGraphV=1;
// QDOAS ???    if ((maxGraphH=PRJCT_itemList[KURUCZ_buffers.indexProject].spectra.maxGraphH)<1)
// QDOAS ???     maxGraphH=1;
// QDOAS ???
// QDOAS ???    maxGraph=maxGraphV*maxGraphH;

    indexGraph=0;

    VPix       = KURUCZ_buffers.VPix;
    VSig       = KURUCZ_buffers.VSig;
    Pcalib     = KURUCZ_buffers.Pcalib;
    pixMid     = KURUCZ_buffers.pixMid;
    VLambda    = KURUCZ_buffers.VLambda;
    VShift     = KURUCZ_buffers.VShift;
    svdFeno    = KURUCZ_buffers.KuruczFeno[indexFeno].svdFeno;

    Nb_Win     = KURUCZ_buffers.Nb_Win;
    fwhm       = KURUCZ_buffers.fwhm;
    fwhmSigma  = KURUCZ_buffers.fwhmSigma;
    offset     = KURUCZ_buffers.offset;
    Grid       = KURUCZ_buffers.KuruczFeno[indexFeno].Grid;

    NDET       = TabFeno[indexFeno].NDET;

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
    memcpy(Lambda,oldLambda,sizeof(double)*NDET);
    memcpy(ANALYSE_secX,spectrum,sizeof(double)*NDET);

// QDOAS ???     #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???     if (saveFlag && ((fp=fopen(DOAS_tmpFile,"a+t"))!=NULL))
// QDOAS ???      {
// QDOAS ???       fprintf(fp,"\n%s\n\n",windowTitle);
// QDOAS ???       fprintf(fp,"Window\n(n,pix,lambda)\tNiter\tShift\t\t\t");
// QDOAS ???
// QDOAS ???       if (pKuruczOptions->fwhmFit)
// QDOAS ???        for (indexParam=0;indexParam<maxParam;indexParam++)
// QDOAS ???         fprintf(fp,"SFP %d\t\t\t",indexParam+1);
// QDOAS ???
// QDOAS ???       if ((Feno->indexOffsetConst!=ITEM_NONE) && (Feno->TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE))
// QDOAS ???        fprintf(fp,"Offset\t\t\t");
// QDOAS ???
// QDOAS ???       for (indexTabCross=0;indexTabCross<Feno->NTabCross;indexTabCross++)
// QDOAS ???        {
// QDOAS ???         pTabCross=&TabCross[indexTabCross];
// QDOAS ???
// QDOAS ???         if (pTabCross->IndSvdA && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS))
// QDOAS ???          fprintf(fp,"%s\t\t\t",WorkSpace[pTabCross->Comp].symbolName);
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       fprintf(fp,"\n");
// QDOAS ???       fclose(fp);
// QDOAS ???      }
// QDOAS ???     #endif

    // Set solar spectrum

    if ((solar=MEMORY_AllocDVector("KURUCZ_Spectrum ","solar",0,NDET-1))==NULL)
     rc=ERROR_ID_ALLOC;
    else if (!pKuruczOptions->fwhmFit)
     {
      if ((KURUCZ_buffers.hrSolar.NDET==NDET) && VECTOR_Equal(KURUCZ_buffers.hrSolar.lambda,oldLambda,NDET,(double)1.e-7))
       memcpy(solar,KURUCZ_buffers.hrSolar.vector,sizeof(double)*NDET);
      else
       rc=SPLINE_Vector(KURUCZ_buffers.hrSolar.lambda,KURUCZ_buffers.hrSolar.vector,KURUCZ_buffers.hrSolar.deriv2,KURUCZ_buffers.hrSolar.NDET,
                           oldLambda,solar,NDET,pAnalysisOptions->interpol,"KURUCZ_Spectrum ");
     }
    else
     memcpy(solar,reference,sizeof(double)*NDET);

    // Buffers for fits initialization

    if (KURUCZ_buffers.crossFits.matrix!=NULL)
     for (indexTabCross=0;indexTabCross<KURUCZ_buffers.crossFits.nc;indexTabCross++)
      memcpy(KURUCZ_buffers.crossFits.matrix[indexTabCross],ANALYSE_zeros,sizeof(double)*NDET);

    memcpy(ANALYSE_t,ANALYSE_zeros,sizeof(double)*NDET);
    memcpy(ANALYSE_tc,ANALYSE_zeros,sizeof(double)*NDET);

    // Browse little windows

    for (indexWindow=0,Square=(double)0.;(indexWindow<Nb_Win) && (rc<THREAD_EVENT_STOP);indexWindow++)
     {
      Feno->Decomp=1;
      Niter=0;

      if (pKuruczOptions->fwhmFit)
       pKURUCZ_fft=&KURUCZ_buffers.KuruczFeno[indexFeno].fft[indexWindow];

      // Global initializations

      #if defined(__DEBUG_) && __DEBUG_
      DEBUG_Start(ENGINE_dbgFile,"Kurucz",DEBUG_FCTTYPE_MATH|DEBUG_FCTTYPE_APPL,5,DEBUG_DVAR_YES,0); // !debugResetFlag++);
      #endif


      if (((rc=ANALYSE_SvdInit(&svdFeno[indexWindow]))!=ERROR_ID_NO) ||

      // Analysis method

          ((rc=ANALYSE_CurFitMethod(spectrum,                                   // spectrum
                                    NULL,                                       // no error on previous spectrum
                                    solar,                                      // reference (Kurucz)
                                   &Square,                                     // returned stretch order 2
                                   &Niter))>=THREAD_EVENT_STOP))                // number of iterations in Curfit

       break;

      #if defined(__DEBUG_) && __DEBUG_
      DEBUG_Stop("Kurucz");
      #endif

      // Fill A SVD system

      pixMid[indexWindow+1]=(double)floor((double)0.5*(svdFeno[indexWindow].Fenetre[0][0]+svdFeno[indexWindow].Fenetre[0][1])+0.5);
      VSig[indexWindow+1]=pResults->SigmaShift;

      if (KURUCZ_buffers.units==PRJCT_ANLYS_UNITS_PIXELS)
       {
        VShift[indexWindow+1]=(Feno->indexSpectrum!=ITEM_NONE)?(double)-pResults->Shift:(double)pResults->Shift;
        VPix[indexWindow+1]=(double)pixMid[indexWindow+1];                      // -pResults->Shift and VLambda[indexWindow+1]=(double)oldLambda[(INDEX)pixMid[indexWindow+1]]

        newPix=(double)pixMid[indexWindow+1]-VShift[indexWindow+1]+1.;
                                                                                // if kurucz spectrum is passed as reference
        if ((rc=SPLINE_Vector(ANALYSE_pixels,                                   // pixels vector
                                 oldLambda,                                     // wavelength scale
                                 ANALYSE_splineX2,                              // second derivatives
                                 NDET,
                                &newPix,
                                &VLambda[indexWindow+1],1,
                                 SPLINE_CUBIC,"KURUCZ_Spectrum "))!=ERROR_ID_NO)
         break;
       }
      else
       {
        VShift[indexWindow+1]=(Feno->indexSpectrum!=ITEM_NONE)?(double)-pResults->Shift:(double)pResults->Shift;
        VPix[indexWindow+1]=pixMid[indexWindow+1];
        VLambda[indexWindow+1]=(double)oldLambda[(INDEX)pixMid[indexWindow+1]]-VShift[indexWindow+1];
       }

      // Store fwhm for future use

      if (pKuruczOptions->fwhmFit)
       for (indexParam=0;indexParam<maxParam;indexParam++)
        {
         fwhm[indexParam][indexWindow]=fabs(Feno->TabCrossResults[Feno->indexFwhmParam[indexParam]].Param);
         fwhmSigma[indexParam][indexWindow]=Feno->TabCrossResults[Feno->indexFwhmParam[indexParam]].SigmaParam;
        }


     /* {
      		FILE *fp;
      		fp=fopen("toto.dat","a+t");
      		fprintf(fp,"%d  %.8le   %.8le  %.8le  %.8le\n",indexWindow,fwhm[0][indexWindow],fwhm[1][indexWindow],fwhm[2][indexWindow],fwhm[3][indexWindow]);
      		fclose(fp);
      	}       */


      // Store fit for display

      if (displayFlag)
       {
        if (KURUCZ_buffers.method!=PRJCT_ANLYS_METHOD_SVD)
         for (i=SvdPDeb;i<=SvdPFin;i++)
          ANALYSE_secX[i]=exp(log(spectrum[i])+ANALYSE_absolu[i]/ANALYSE_tc[i]); // spectrum[i]+solar[i]*ANALYSE_absolu[i]/ANALYSE_tc[i];
        else
         for (i=SvdPDeb;i<=SvdPFin;i++)
          ANALYSE_secX[i]=exp(log(spectrum[i])+ANALYSE_absolu[i]);

        if ((Feno->indexOffsetConst!=ITEM_NONE) &&
            (Feno->indexOffsetOrder1!=ITEM_NONE) &&
            (Feno->indexOffsetOrder2!=ITEM_NONE) &&

           ((TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE) ||
            (TabCross[Feno->indexOffsetOrder1].FitParam!=ITEM_NONE) ||
            (TabCross[Feno->indexOffsetOrder2].FitParam!=ITEM_NONE) ||
            (TabCross[Feno->indexOffsetConst].InitParam!=(double)0.) ||
            (TabCross[Feno->indexOffsetOrder1].InitParam!=(double)0.) ||
            (TabCross[Feno->indexOffsetOrder2].InitParam!=(double)0.)))

         for (i=SvdPDeb,j0=(SvdPDeb+SvdPFin)/2;i<=SvdPFin;i++)
          {
           offset[i]=(double)1.-Feno->xmean*(Results[Feno->indexOffsetConst].Param+
                  Results[Feno->indexOffsetOrder1].Param*(ANALYSE_splineX[i]-ANALYSE_splineX[j0])+
                  Results[Feno->indexOffsetOrder2].Param*(ANALYSE_splineX[i]-ANALYSE_splineX[j0])*(ANALYSE_splineX[i]-ANALYSE_splineX[j0]))/spectrum[i];
           offset[i]=(offset[i]>(double)0.)?log(offset[i]):(double)0.;
          }

        if (KURUCZ_buffers.crossFits.matrix!=NULL)

         for (indexTabCross=indexCrossFit=0;(indexTabCross<Feno->NTabCross) && (indexCrossFit<KURUCZ_buffers.crossFits.nc);indexTabCross++)
          {
           pTabCross=&TabCross[indexTabCross];

           if (pTabCross->IndSvdA && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) && pTabCross->display)
            {
             for (i=SvdPDeb,k=1;i<=SvdPFin;i++,k++)
              KURUCZ_buffers.crossFits.matrix[indexCrossFit][i]=x[pTabCross->IndSvdA]*U[pTabCross->IndSvdA][k];

             indexCrossFit++;
            }
          }
       }

      // Results safe keeping

      memcpy(KURUCZ_buffers.KuruczFeno[indexFeno].results[indexWindow],Feno->TabCrossResults,sizeof(CROSS_RESULTS)*Feno->NTabCross);
      KURUCZ_buffers.KuruczFeno[indexFeno].chiSquare[indexWindow]=Square;
      KURUCZ_buffers.KuruczFeno[indexFeno].rms[indexWindow]=(Square>(double)0.)?sqrt(Square):(double)0.;

      // Display results in 'data on spectra' mdi child window

// QDOAS ???       #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???       if (saveFlag && ((fp=fopen(DOAS_tmpFile,"a+t"))!=NULL))
// QDOAS ???        {
// QDOAS ???         fprintf(fp,"%#2d/%d %#4.0lf %#.1lf\t%#2d\t%#10.3e+/-%#10.3e\t",
// QDOAS ???                      indexWindow+1,Nb_Win,pixMid[indexWindow+1],VLambda[indexWindow+1],Niter,pResults->Shift,
// QDOAS ???                      pResults->SigmaShift);
// QDOAS ???
// QDOAS ???         for (indexParam=0;indexParam<maxParam;indexParam++)
// QDOAS ???          fprintf(fp,"%#10.3e +/-%#10.3e\t",Feno->TabCrossResults[Feno->indexFwhmParam[indexParam]].Param,Feno->TabCrossResults[Feno->indexFwhmParam[indexParam]].SigmaParam);
// QDOAS ???
// QDOAS ???         if ((Feno->indexOffsetConst!=ITEM_NONE) && (Feno->TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE))
// QDOAS ???          fprintf(fp,"%#10.3e +/-%#10.3e\t",Feno->TabCrossResults[Feno->indexOffsetConst].Param,Feno->TabCrossResults[Feno->indexOffsetConst].SigmaParam);
// QDOAS ???
// QDOAS ???         for (indexTabCross=0;indexTabCross<Feno->NTabCross;indexTabCross++)
// QDOAS ???          {
// QDOAS ???           pTabCross=&TabCross[indexTabCross];
// QDOAS ???
// QDOAS ???           if (pTabCross->IndSvdA && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS))
// QDOAS ???            fprintf(fp,"%#10.3e +/-%#10.3e\t",Feno->TabCrossResults[indexTabCross].SlntCol,Feno->TabCrossResults[indexTabCross].SlntErr);
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         fprintf(fp,"\n");
// QDOAS ???         fclose(fp);
// QDOAS ???
// QDOAS ???         THRD_LoadData();
// QDOAS ???        }
// QDOAS ???       #endif
     }

    if (rc<THREAD_EVENT_STOP) //if (THRD_id==THREAD_TYPE_ANALYSIS)
     {
      SvdPDeb=svdFeno[0].Fenetre[0][0];
      SvdPFin=svdFeno[Nb_Win-1].Fenetre[0][1];

      // New wavelength scale (corrected calibration)
      // NB : we fit a polynomial in Lambda+shift point but it's possible to fit a polynomial in shift points by replacing
      //      VLambda by VShift in the following instruction

      if ((rc=ANALYSE_LinFit(&Feno->svd,Nb_Win,KURUCZ_buffers.shiftDegree,VPix,NULL,
          (KURUCZ_buffers.units==PRJCT_ANLYS_UNITS_PIXELS)?VLambda:VShift,Pcalib))!=ERROR_ID_NO) // !!! Change in the future VLambda -> VShift
       goto EndKuruczSpectrum;

      if (pKuruczOptions->fwhmFit)
       {
        for (indexParam=0;indexParam<maxParam;indexParam++)
         if (TabCross[Feno->indexFwhmParam[indexParam]].FitParam!=ITEM_NONE)
          {
           memcpy(coeff[indexParam],ANALYSE_zeros,sizeof(double)*(KURUCZ_buffers.fwhmDegree+1));
           memcpy(fwhmVector[indexParam],ANALYSE_zeros,sizeof(double)*NDET);

           if ((rc=ANALYSE_LinFit(&KURUCZ_buffers.svdFwhm,Nb_Win,KURUCZ_buffers.fwhmDegree,VLambda,NULL,fwhm[indexParam]-1,coeff[indexParam]-1))!=ERROR_ID_NO)
            goto EndKuruczSpectrum;
          }

        if (rc!=ERROR_ID_NO)
         goto EndKuruczSpectrum;
       }

      for (i=0;i<NDET;i++)
       {
        shiftPoly[i]=Pcalib[KURUCZ_buffers.shiftDegree+1];
        for (j=KURUCZ_buffers.shiftDegree;j>=1;j--)
         shiftPoly[i]=shiftPoly[i]*(double)i+Pcalib[j];

        if (KURUCZ_buffers.units==PRJCT_ANLYS_UNITS_NANOMETERS)
         Lambda[i]=oldLambda[i]-shiftPoly[i];    // only if you want to fit a polynomial in shift points and afterwards,
        else
         Lambda[i]=shiftPoly[i];
       }

      if (displayFlag)
       {
        // Display complete fit

      //  if (KURUCZ_buffers.displaySpectra)
      //   {
      //    mediateAllocateAndSetPlotData(&spectrumData[0],&Lambda[SvdPDeb],&spectrum[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
      //    mediateAllocateAndSetPlotData(&spectrumData[1],&Lambda[SvdPDeb],&ANALYSE_secX[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
      //    mediateResponsePlotData(plotPageCalib,spectrumData,2,Spectrum,forceAutoScale,"Complete fit","Wavelength (nm)","Intensity", responseHandle);
      //    mediateResponseLabelPage(plotPageCalib, "", "", responseHandle);
      //    mediateReleasePlotData(spectrumData);
      //   }

        // Display residual

        if (KURUCZ_buffers.displayResidual)
         {
          if (KURUCZ_buffers.method!=PRJCT_ANLYS_METHOD_SVD)
           for (j=SvdPDeb;j<=SvdPFin;j++)
            ANALYSE_absolu[j]=(ANALYSE_tc[j]!=(double)0.)?ANALYSE_absolu[j]/ANALYSE_tc[j]:(double)0.;

          mediateAllocateAndSetPlotData(&spectrumData[0],&Lambda[SvdPDeb],&ANALYSE_absolu[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
          mediateResponsePlotData(plotPageCalib,spectrumData,1,Spectrum,forceAutoScale,"Residual","Wavelength (nm)","", responseHandle);
          mediateResponseLabelPage(plotPageCalib, "", "Kurucz", responseHandle);
          mediateReleasePlotData(spectrumData);
         }

     //   memcpy(ANALYSE_secX,ANALYSE_zeros,sizeof(double)*NDET);
     //
     //   // Display Offset
     //
     //   if  (// Feno->displayOffset &&
     //       (Feno->indexOffsetConst!=ITEM_NONE) &&
     //       (Feno->indexOffsetOrder1!=ITEM_NONE) &&
     //       (Feno->indexOffsetOrder2!=ITEM_NONE) &&
     //
     //      ((TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE) ||
     //       (TabCross[Feno->indexOffsetOrder1].FitParam!=ITEM_NONE) ||
     //       (TabCross[Feno->indexOffsetOrder2].FitParam!=ITEM_NONE) ||
     //       (TabCross[Feno->indexOffsetConst].InitParam!=(double)0.) ||
     //       (TabCross[Feno->indexOffsetOrder1].InitParam!=(double)0.) ||
     //       (TabCross[Feno->indexOffsetOrder2].InitParam!=(double)0.)))
     //    {
     //     for (j=SvdPDeb;j<=SvdPFin;j++)
     //      {
     //       ANALYSE_absolu[j]+=offset[j]-ANALYSE_secX[j];
     //       ANALYSE_secX[j]=offset[j];
     //      }
     //
     //     if (KURUCZ_buffers.displayFit)
     //      {
     //       mediateAllocateAndSetPlotData(&spectrumData[0],&Lambda[SvdPDeb],&ANALYSE_absolu[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
     //       mediateAllocateAndSetPlotData(&spectrumData[1],&Lambda[SvdPDeb],&ANALYSE_secX[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
     //       mediateResponsePlotData(plotPageCalib,spectrumData,2,Spectrum,forceAutoScale,"Offset","Wavelength (nm)","", responseHandle);
     //       mediateResponseLabelPage(plotPageCalib, "", windowTitle, responseHandle);
     //       mediateReleasePlotData(spectrumData);
     //      }
     //    }
     //
     //   // Display fits
     //
     //   if (KURUCZ_buffers.crossFits.matrix!=NULL)
     //    {
     //     for (indexTabCross=indexCrossFit=0;(indexTabCross<Feno->NTabCross) && (indexCrossFit<KURUCZ_buffers.crossFits.nc);indexTabCross++)
     //      {
     //       pTabCross=&TabCross[indexTabCross];
     //
     //       if (pTabCross->IndSvdA && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) && pTabCross->display)
     //        {
     //          for (j=SvdPDeb;j<=SvdPFin;j++)
     //           {
     //            ANALYSE_absolu[j]+=KURUCZ_buffers.crossFits.matrix[indexCrossFit][j]-ANALYSE_secX[j];
     //            ANALYSE_secX[j]=KURUCZ_buffers.crossFits.matrix[indexCrossFit][j];
     //           }
     //
     //         sprintf(string,"%s fit",WorkSpace[pTabCross->Comp].symbolName);
     //
     //         mediateAllocateAndSetPlotData(&spectrumData[0],&Lambda[SvdPDeb],&ANALYSE_absolu[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
     //         mediateAllocateAndSetPlotData(&spectrumData[1],&Lambda[SvdPDeb],&ANALYSE_secX[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
     //         mediateResponsePlotData(plotPageCalib,spectrumData,2,Spectrum,forceAutoScale,string,"Wavelength (nm)","", responseHandle);
     //         mediateResponseLabelPage(plotPageCalib, "", windowTitle, responseHandle);
     //         mediateReleasePlotData(spectrumData);
     //
     //         indexCrossFit++;
     //        }
     //      }
     //    }
     //
     //   // Display error on calibration
     //
     //   if (KURUCZ_buffers.displayShift)
     //    {
     //     mediateAllocateAndSetPlotData(&spectrumData[0],&Lambda[SvdPDeb],&ANALYSE_absolu[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
     //     mediateAllocateAndSetPlotData(&spectrumData[1],&Lambda[SvdPDeb],&shiftPoly[SvdPDeb],SvdPFin-SvdPDeb+1,Point);
     //     mediateResponsePlotData(plotPageCalib,spectrumData,2,Spectrum,forceAutoScale,"Shift applied","Wavelength (nm)","Shift (nm)", responseHandle);
     //     mediateResponseLabelPage(plotPageCalib, "", windowTitle, responseHandle);
     //     mediateReleasePlotData(spectrumData);
     //    }

        // Display wavelength dependence of fwhm

        if (pKuruczOptions->fwhmFit)

         for (indexParam=0;(indexParam<maxParam) && (rc<THREAD_EVENT_STOP);indexParam++)
          {
           sprintf(string,"SFP %d applied",indexParam+1);

           if (TabCross[Feno->indexFwhmParam[indexParam]].FitParam!=ITEM_NONE)
            {
             for (i=0;i<NDET;i++)
              {
               fwhmVector[indexParam][i]=(double)coeff[indexParam][KURUCZ_buffers.fwhmDegree];
               for (j=KURUCZ_buffers.fwhmDegree-1;j>=0;j--)
                fwhmVector[indexParam][i]=fwhmVector[indexParam][i]*(double)Lambda[i]+coeff[indexParam][j];
              }

             if ((rc=SPLINE_Deriv2(Lambda,fwhmVector[indexParam],fwhmDeriv2[indexParam],NDET,"KURUCZ_Spectrum "))!=0)
              goto EndKuruczSpectrum;
            }

           if (KURUCZ_buffers.displayShift)
            {
             mediateAllocateAndSetPlotData(&spectrumData[0],&Lambda[SvdPDeb],&fwhmVector[indexParam][SvdPDeb],SvdPFin-SvdPDeb+1,Line);
             mediateAllocateAndSetPlotData(&spectrumData[1],VLambda+1,fwhm[indexParam],Nb_Win,Point);
             mediateResponsePlotData(plotPageCalib,spectrumData,2,Spectrum,forceAutoScale,"FWHM","Wavelength (nm)","FWHM (nm)", responseHandle);
             mediateResponseLabelPage(plotPageCalib, "", windowTitle, responseHandle);
             mediateReleasePlotData(spectrumData);
            }
          }
       }
     }
   }

  EndKuruczSpectrum :

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

RC KURUCZ_ApplyCalibration(FENO *pTabFeno,double *newLambda)
 {
  // Declarations

  INDEX indexWindow;
  INT newDimL;
  RC rc;

  // Initialization

  rc=0;

  if (((pTabFeno->rcKurucz=ANALYSE_XsInterpolation(pTabFeno,newLambda))!=ERROR_ID_NO) ||
       (pTabFeno->xsToConvolute && ((pTabFeno->useKurucz==ANLYS_KURUCZ_REF) || (pTabFeno->useKurucz==ANLYS_KURUCZ_SPEC)) &&
      ((pKuruczOptions->fwhmFit && ((pTabFeno->rcKurucz=ANALYSE_XsConvolution(pTabFeno,newLambda,NULL,pKuruczOptions->fwhmType,pTabFeno->fwhmVector[0],pTabFeno->fwhmVector[1],pTabFeno->fwhmVector[2],pTabFeno->fwhmVector[3]))!=ERROR_ID_NO)) ||
      (!pKuruczOptions->fwhmFit && ((pTabFeno->rcKurucz=ANALYSE_XsConvolution(pTabFeno,newLambda,&ANALYSIS_slit,pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2,&pSlitOptions->slitFunction.slitParam3,&pSlitOptions->slitFunction.slitParam4))!=ERROR_ID_NO)))))

   rc=pTabFeno->rcKurucz;

  else
   {
    // Rebuild gaps

    for (indexWindow=newDimL=0;indexWindow<pTabFeno->svd.Z;indexWindow++)
     {
      pTabFeno->svd.Fenetre[indexWindow][0]=FNPixel(newLambda,pTabFeno->svd.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
      pTabFeno->svd.Fenetre[indexWindow][1]=FNPixel(newLambda,pTabFeno->svd.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

      newDimL+=(pTabFeno->svd.Fenetre[indexWindow][1]-pTabFeno->svd.Fenetre[indexWindow][0]+1);
     }

    if (newDimL!=pTabFeno->svd.DimL)
     {
      ANALYSE_SvdFree("KURUCZ_ApplyCalibration ",&pTabFeno->svd);
      pTabFeno->svd.DimL=newDimL;
      ANALYSE_SvdLocalAlloc("KURUCZ_ApplyCalibration ",&pTabFeno->svd);
     }

    // Force decomposition

    pTabFeno->Decomp=1;
   }

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
RC KURUCZ_Reference(double *instrFunction,INDEX refFlag,INT saveFlag,INT gomeFlag,void *responseHandle)
 {
  // Declarations

  UCHAR            windowTitle[MAX_ITEM_TEXT_LEN+1];                            // graph title
  CROSS_REFERENCE *TabCross;                                                    // pointer to a symbol description hold by an analysis window
  FENO            *pTabFeno,*pTabRef,*pTabKurucz;                               // browse analysis windows
  double          *reference;                                                   // reference spectrum to align on Kurucz

  INT              maxParam,
                   msgCount,
                   nKuruczFeno,
                   nBadKuruczFeno;
  UCHAR            pause;

  INDEX            indexFeno,                                                   // browse analysis windows
                   indexRef;                                                    // index of another analysis window with the same reference spectrum
  RC               rc;                                                          // return code

  // Initializations

  pause=(UCHAR)THRD_specInfo.project.spectra.displayPause;
  THRD_specInfo.project.spectra.displayPause=1;
  pTabKurucz=&TabFeno[KURUCZ_buffers.indexKurucz];
  TabCross=pTabKurucz->TabCross;
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
      pTabFeno=&TabFeno[indexFeno];

      if (!pTabFeno->hidden && ((pTabFeno->useKurucz==ANLYS_KURUCZ_REF) || (pTabFeno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC)) &&
          (pTabFeno->gomeRefFlag==gomeFlag) &&
        ((!refFlag && (pTabFeno->useEtalon || (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE))) ||
         ((refFlag==1) && !pTabFeno->useEtalon)))

       nKuruczFeno++;
     }

    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     {
      pTabFeno=&TabFeno[indexFeno];

      if (!pTabFeno->hidden && ((pTabFeno->useKurucz==ANLYS_KURUCZ_REF) || (pTabFeno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC)) &&
          (pTabFeno->gomeRefFlag==gomeFlag) &&
        ((!refFlag && (pTabFeno->useEtalon || (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE))) ||
         ((refFlag==1) && !pTabFeno->useEtalon)))
       {
       	memcpy(reference,(pTabFeno->useEtalon)?pTabFeno->SrefEtalon:pTabFeno->Sref,sizeof(double)*pTabFeno->NDET);

       	if ((pTabFeno->NDET==KURUCZ_buffers.hrSolar.NDET) &&
       	     VECTOR_Equal(KURUCZ_buffers.hrSolar.lambda,pTabFeno->LambdaRef,pTabFeno->NDET,(double)1.e-7) &&
       	     VECTOR_Equal(KURUCZ_buffers.hrSolar.vector,reference,pTabFeno->NDET,(double)1.e-7))

       	 pTabFeno->rcKurucz=ERROR_ID_NO;
       	else
       	 {
       	  // Apply instrumental corrections on reference spectrum

          if (((indexRef=KuruczSearchReference(indexFeno))<NFeno) && (indexRef!=indexFeno) &&
              ((indexRef<indexFeno) ||
              ((refFlag && !pTabFeno->useEtalon) && ((TabFeno[indexRef].refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE) || TabFeno[indexRef].useEtalon))))
           {
            pTabRef=&TabFeno[indexRef];
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
            // Set title of graph

// QDOAS ???            sprintf(windowTitle,"Calibration applied on project %s, window %s",PRJCT_itemList[KURUCZ_buffers.indexProject].name,pTabFeno->windowName);

            // Apply Kurucz for building new calibration for reference

            if ((pTabFeno->rcKurucz=KURUCZ_Spectrum(pTabFeno->LambdaRef,pTabFeno->LambdaK,reference,KURUCZ_buffers.solar,instrFunction,
                 1,pTabFeno->windowName,pTabFeno->fwhmPolyRef,pTabFeno->fwhmVector,pTabFeno->fwhmDeriv2,saveFlag,indexFeno,responseHandle))!=ERROR_ID_NO)

             goto EndKuruczReference;
           }

          if (!rc && !pTabFeno->rcKurucz)
           rc=KURUCZ_ApplyCalibration(pTabFeno,pTabFeno->LambdaK);

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
             	if ((++nBadKuruczFeno==nKuruczFeno)
// QDOAS ???               #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???               ||  (!msgCount && (THRD_delay==INFINITE) &&
// QDOAS ???                  ((rc=MSG_MessageBox(DOAS_hwndMain,ITEM_NONE,IDS_TITLE_ANALYSIS,ERROR_ID_KURUCZ,MB_YESNO|MB_ICONHAND,pTabFeno->windowName))==IDNO))
// QDOAS ???               #endif
                 )
               {
                rc=pTabFeno->rcKurucz;
                break;
               }

              msgCount++;
             }
           }
         }
       }
     }
   }

  if (rc<THREAD_EVENT_STOP)
   rc=ERROR_ID_NO;

  // Release allocated buffers

  if (reference!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Reference","reference",reference,0);

  THRD_specInfo.project.spectra.displayPause=pause;

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
void KURUCZ_Init(INT gomeFlag)
 {
  // Declarations

  INDEX indexFeno,indexWindow;
  INT (*Fenetre)[2],nbWin;
  double Lambda_min,Lambda_max,Win_size;
  FENO *pTabFeno;
  SVD *pSvd;

  // Initialization

  nbWin=KURUCZ_buffers.Nb_Win;

  // Browse analysis windows

  for (indexFeno=0;indexFeno<NFeno;indexFeno++)
   {
    pTabFeno=&TabFeno[indexFeno];

    if (
        (pTabFeno->gomeRefFlag==gomeFlag) &&
        (KURUCZ_buffers.KuruczFeno[indexFeno].svdFeno!=NULL))
     {
      Lambda_min=pKuruczOptions->lambdaLeft;
      Lambda_max=pKuruczOptions->lambdaRight;

      Win_size=(double)(Lambda_max-Lambda_min)/nbWin;

      for (indexWindow=0;indexWindow<nbWin;indexWindow++)
       {
        pSvd=&KURUCZ_buffers.KuruczFeno[indexFeno].svdFeno[indexWindow];
        Fenetre=pSvd->Fenetre;
        Lambda_max=Lambda_min+Win_size;

        Fenetre[0][0]=FNPixel(pTabFeno->LambdaRef,Lambda_min,pTabFeno->NDET,PIXEL_AFTER);
        Fenetre[0][1]=FNPixel(pTabFeno->LambdaRef,Lambda_max,pTabFeno->NDET,PIXEL_BEFORE);

        pSvd->DimL=Fenetre[0][1]-Fenetre[0][0]+1;
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

RC KURUCZ_Alloc(PROJECT *pProject,double *lambda,INDEX indexKurucz,double lambdaMin,double lambdaMax)
 {
  // Declarations

  INT hFilterFlag;
  CROSS_REFERENCE *pTabCross;                                                   // cross sections list
  FENO            *pKuruczFeno,                                                 // analysis window with Kurucz description
                  *pTabFeno;

  UCHAR  kuruczFile[MAX_ITEM_TEXT_LEN+1];
  INT    Nb_Win,shiftDegree,fwhmDegree,                                         // substitution variables
         NTabCross,DimLMax;
  INDEX  i,indexFeno,indexWindow,indexParam,indexTabCross;                      // indexes for loops and arrays
  double Lambda_min,Lambda_max,                                                 // extrema in nm of a little window
         Win_size,step;                                                         // size of a little window in nm
  SVD   *pSvd,*pSvdFwhm;                                                        // pointers to svd environments
  RC rc;

  // Initializations

  memset(&KURUCZ_buffers.hrSolar,0,sizeof(XS));

  pKuruczFeno=&TabFeno[indexKurucz];

  step=(double)0.;

  FILES_RebuildFileName(kuruczFile,pKuruczOptions->file,1);

  if ((hFilterFlag=((ANALYSE_phFilter->filterFunction!=NULL) &&                 // high pass filtering
                    (pKuruczFeno->analysisMethod==PRJCT_ANLYS_METHOD_SVD) &&    // implemented only in DOAS fitting
                    (pKuruczFeno->analysisType==ANALYSIS_TYPE_FWHM_KURUCZ) &&   // for calibration with slit function fitting
                   ((pKuruczFeno->indexSol==ITEM_NONE) ||                       // doesn't fit the Sol non linear parameter
                    (pKuruczFeno->TabCross[pKuruczFeno->indexSol].FitParam==ITEM_NONE)))?1:0)==1);

   KURUCZ_buffers.solarFGap=(int)floor(ANALYSE_phFilter->filterWidth*sqrt(ANALYSE_phFilter->filterNTimes)+0.5);

  // Load options from Kurucz tab page from project properties

  Nb_Win=pKuruczOptions->windowsNumber;
  shiftDegree=pKuruczOptions->shiftPolynomial;

  rc=ERROR_ID_NO;

  // Check validity of entry fields in Kurucz tab page of project properties panel

  if (pKuruczOptions->fwhmFit)
   {
    fwhmDegree=pKuruczOptions->fwhmPolynomial;

// QDOAS ???  TESTS TO MOVE TO THE GUI     if ((fwhmDegree<0) || (fwhmDegree>(UINT)10))
// QDOAS ???  TESTS TO MOVE TO THE GUI      rc=ERROR_SetLast("KURUCZ_Alloc",ERROR_TYPE_FATAL,ERROR_ID_MSGBOX_OUTOFRANGE,"\'Kurucz fwhm polynomial order\'",0,(UINT)10);

    if (rc!=ERROR_ID_NO)
     goto EndKuruczAlloc;

    KURUCZ_buffers.fwhmDegree=fwhmDegree;
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

       if (((KURUCZ_buffers.KuruczFeno=(KURUCZ_FENO *)MEMORY_AllocBuffer("KURUCZ_Alloc ","KuruczFeno",NFeno,sizeof(KURUCZ_FENO),0,MEMORY_TYPE_STRUCT))==NULL) ||
           ((KURUCZ_buffers.solar=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","solar",0,NDET-1))==NULL) ||           // solar spectrum
           ((KURUCZ_buffers.offset=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","offset",0,NDET-1))==NULL) ||         // offset spectrum
           ((KURUCZ_buffers.Pcalib=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","Pcalib",1,shiftDegree+1))==NULL) ||  // coefficients of the polynomial
           ((KURUCZ_buffers.VLambda=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","VLambda",1,Nb_Win))==NULL) ||       // solution of the system
           ((KURUCZ_buffers.pixMid=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","pixMid",1,Nb_Win))==NULL) ||         // pixels at the middle of little windows
           ((KURUCZ_buffers.VShift=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","VShift",1,Nb_Win))==NULL) ||         // shift applied on pixels
           ((KURUCZ_buffers.VSig=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","VSig",1,Nb_Win))==NULL) ||             // error on shift applied on pixels
           ((KURUCZ_buffers.VPix=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","VPix",1,Nb_Win))==NULL) ||             // pixels with shift correction
           (hFilterFlag &&
          (((KURUCZ_buffers.lambdaF=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","lambdaF",0,NDET+2*KURUCZ_buffers.solarFGap-1))==NULL) ||
           ((KURUCZ_buffers.solarF=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","solarF",0,NDET+2*KURUCZ_buffers.solarFGap-1))==NULL) ||
           ((KURUCZ_buffers.solarF2=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","solarF2",0,NDET+2*KURUCZ_buffers.solarFGap-1))==NULL))))
   {
    rc=ERROR_ID_ALLOC;
    goto EndKuruczAlloc;
   }

  if (hFilterFlag && KURUCZ_buffers.solarFGap)
   {
    if (lambda[NDET-1]-lambda[0]+1==NDET)
     memcpy(KURUCZ_buffers.lambdaF,ANALYSE_zeros,sizeof(double)*NDET+2*KURUCZ_buffers.solarFGap);
    else
     {
      memcpy(&KURUCZ_buffers.lambdaF[KURUCZ_buffers.solarFGap],lambda,sizeof(double)*NDET);

      step=(lambda[NDET-1]-lambda[0])/NDET;

      for (i=0;i<KURUCZ_buffers.solarFGap;i++)
       {
        KURUCZ_buffers.lambdaF[i]=lambda[0]-step*(KURUCZ_buffers.solarFGap-i);
        KURUCZ_buffers.lambdaF[NDET+KURUCZ_buffers.solarFGap+i]=lambda[NDET-1]+step*(i+1);
       }
     }
   }

  // Load and normalize solar spectrum

  if (!(rc=XSCONV_LoadCrossSectionFile(&KURUCZ_buffers.hrSolar,kuruczFile,lambdaMin-7.-step*KURUCZ_buffers.solarFGap,lambdaMax+7.+step*KURUCZ_buffers.solarFGap,(double)0.,CONVOLUTION_CONVERSION_NONE)))
   {
    if (((rc=ANALYSE_NormalizeVector(KURUCZ_buffers.hrSolar.vector-1,KURUCZ_buffers.hrSolar.NDET,NULL,"KURUCZ_Alloc "))!=ERROR_ID_NO) ||
        ((rc=SPLINE_Deriv2(KURUCZ_buffers.hrSolar.lambda,KURUCZ_buffers.hrSolar.vector,KURUCZ_buffers.hrSolar.deriv2,KURUCZ_buffers.hrSolar.NDET,"KURUCZ_Alloc "))!=ERROR_ID_NO))

     goto EndKuruczAlloc;

    memcpy(KURUCZ_buffers.solar,ANALYSE_zeros,sizeof(double)*NDET);

    // Initialize other fields of global structure

    KURUCZ_buffers.indexKurucz=indexKurucz;

    KURUCZ_buffers.Nb_Win=Nb_Win;
    KURUCZ_buffers.shiftDegree=shiftDegree;

    KURUCZ_buffers.displayFit=(UCHAR)pKuruczOptions->displayFit;                // display fit flag
    KURUCZ_buffers.displayResidual=(UCHAR)pKuruczOptions->displayResidual;      // display new calibration flag
    KURUCZ_buffers.displayShift=(UCHAR)pKuruczOptions->displayShift;            // display shift in each pixel flag
    KURUCZ_buffers.displaySpectra=(UCHAR)pKuruczOptions->displaySpectra;        // display shift in each pixel flag

    KURUCZ_buffers.units=pProject->analysis.units;
    KURUCZ_buffers.method=(UCHAR)pKuruczOptions->analysisMethod;

    // Allocate one svd environment for each little window

    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     {
      memset(&KURUCZ_buffers.KuruczFeno[indexFeno],0,sizeof(KURUCZ_FENO));
      pTabFeno=&TabFeno[indexFeno];

      if ((pTabFeno->hidden==1) ||
         ((THRD_id!=THREAD_TYPE_KURUCZ) && !pTabFeno->hidden && pTabFeno->useKurucz))
       {
        Lambda_min=pKuruczOptions->lambdaLeft;
        Lambda_max=pKuruczOptions->lambdaRight;

        Win_size=(double)(Lambda_max-Lambda_min)/Nb_Win;
        DimLMax=2*NDET/Nb_Win+1;

        if ((KURUCZ_buffers.KuruczFeno[indexFeno].Grid=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","Grid",0,Nb_Win-1))==NULL)
         rc=ERROR_ID_ALLOC;
        else if ((KURUCZ_buffers.KuruczFeno[indexFeno].svdFeno=(SVD *)MEMORY_AllocBuffer("KURUCZ_Alloc ","svdFeno",Nb_Win,sizeof(SVD),0,MEMORY_TYPE_STRUCT))==NULL)                           // svd environments
         rc=ERROR_ID_ALLOC;
        else if (pKuruczOptions->fwhmFit && ((KURUCZ_buffers.KuruczFeno[indexFeno].fft=(FFT *)MEMORY_AllocBuffer("KURUCZ_Alloc ","fft",Nb_Win,sizeof(FFT),0,MEMORY_TYPE_STRUCT))==NULL))                           // svd environments
         rc=ERROR_ID_ALLOC;
        else if ((KURUCZ_buffers.KuruczFeno[indexFeno].chiSquare=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","chiSquare",0,Nb_Win-1))==NULL)
         rc=ERROR_ID_ALLOC;
        else if ((KURUCZ_buffers.KuruczFeno[indexFeno].rms=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","rms",0,Nb_Win-1))==NULL)
         rc=ERROR_ID_ALLOC;
        else if ((KURUCZ_buffers.KuruczFeno[indexFeno].results=(CROSS_RESULTS **)MEMORY_AllocBuffer("KURUCZ_Alloc ","results",Nb_Win,sizeof(CROSS_RESULTS *),0,MEMORY_TYPE_STRUCT))==NULL)
         rc=ERROR_ID_ALLOC;

        if (rc)
         goto EndKuruczAlloc;
        else
         {
          if (KURUCZ_buffers.KuruczFeno[indexFeno].svdFeno!=NULL)
           memset(KURUCZ_buffers.KuruczFeno[indexFeno].svdFeno,0,Nb_Win*sizeof(SVD));
          if (KURUCZ_buffers.KuruczFeno[indexFeno].fft!=NULL)
           memset(KURUCZ_buffers.KuruczFeno[indexFeno].fft,0,Nb_Win*sizeof(FFT));
          if (KURUCZ_buffers.KuruczFeno[indexFeno].results!=NULL)
           memset(KURUCZ_buffers.KuruczFeno[indexFeno].results,0,Nb_Win*sizeof(CROSS_RESULTS *));
         }

        for (indexWindow=0;indexWindow<Nb_Win;indexWindow++)
         {
          pSvd=&KURUCZ_buffers.KuruczFeno[indexFeno].svdFeno[indexWindow];
          memcpy(pSvd,&pKuruczFeno->svd,sizeof(SVD));
          pSvd->Z=1;
          pSvd->DimL=DimLMax;

          Lambda_max=Lambda_min+Win_size;
          KURUCZ_buffers.KuruczFeno[indexFeno].Grid[indexWindow]=Lambda_max;

          if ((KURUCZ_buffers.KuruczFeno[indexFeno].results[indexWindow]=(CROSS_RESULTS *)MEMORY_AllocBuffer("KURUCZ_Alloc ","KuruczFeno(results)",pKuruczFeno->NTabCross,sizeof(CROSS_RESULTS),0,MEMORY_TYPE_STRUCT))==NULL)
           {
            rc=ERROR_ID_ALLOC;
            goto EndKuruczAlloc;
           }
          else if ((rc=ANALYSE_SvdLocalAlloc("KURUCZ_Alloc (1)",pSvd))!=ERROR_ID_NO)
           goto EndKuruczAlloc;
          else if (pKuruczOptions->fwhmFit)
           {
            INT hrDeb,hrFin,hrN,fftSize;
            double *fftIn;
            FFT *pfft;
            INDEX i;

            pfft=&KURUCZ_buffers.KuruczFeno[indexFeno].fft[indexWindow];

            hrDeb=FNPixel(KURUCZ_buffers.hrSolar.lambda,Lambda_min-3.,KURUCZ_buffers.hrSolar.NDET,PIXEL_AFTER);
            hrFin=FNPixel(KURUCZ_buffers.hrSolar.lambda,Lambda_max+3.,KURUCZ_buffers.hrSolar.NDET,PIXEL_BEFORE);

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

            memcpy(fftIn+1,KURUCZ_buffers.hrSolar.vector+hrDeb,sizeof(double)*hrN);

            for (i=hrN+1;i<=fftSize;i++)
             fftIn[i]=fftIn[2*hrN-i];

            realft(pfft->fftIn,pfft->fftOut,fftSize,1);

            memcpy(fftIn+1,KURUCZ_buffers.hrSolar.lambda+hrDeb,sizeof(double)*hrN);  // Reuse fftIn for high resolution wavelength safe keeping
           }

          Lambda_min=Lambda_max;
         }
       }
     }

    // Allocate svd environment for Kurucz analysis window

    pSvd=&pKuruczFeno->svd;

    pSvd->DimL=Nb_Win;
    pSvd->DimC=shiftDegree+1;

    if ((rc=ANALYSE_SvdLocalAlloc("KURUCZ_Alloc (2)",pSvd))!=ERROR_ID_NO)
     goto EndKuruczAlloc;

    // Allocate buffers for cross sections fits

    if (KURUCZ_buffers.displayFit)
     {
      for (indexTabCross=0,NTabCross=0;indexTabCross<pKuruczFeno->NTabCross;indexTabCross++)
       {
        pTabCross=&pKuruczFeno->TabCross[indexTabCross];
        if (pTabCross->IndSvdA && pTabCross->display &&
           (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS))

         NTabCross++;
       }

      if (NTabCross && ((rc=MATRIX_Allocate(&KURUCZ_buffers.crossFits,NDET,NTabCross,0,0,0,"KURUCZ_Alloc "))!=0))
       goto EndKuruczAlloc;
     }

    // Allocate buffers for coefficients of polynomials fitting fwhm

    if (pKuruczOptions->fwhmFit)
     {
      pSvdFwhm=&KURUCZ_buffers.svdFwhm;
      pSvdFwhm->DimL=Nb_Win;
      pSvdFwhm->DimC=KURUCZ_buffers.fwhmDegree+1;

      if ((rc=ANALYSE_SvdLocalAlloc("KURUCZ_Alloc (3)",pSvdFwhm))!=ERROR_ID_NO)
       goto EndKuruczAlloc;

      for (indexParam=0;(indexParam<MAX_KURUCZ_FWHM_PARAM) && !rc;indexParam++)
       {
        if (((KURUCZ_buffers.fwhm[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhm",0,Nb_Win-1))==NULL) ||
            ((KURUCZ_buffers.fwhmSigma[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhmSigma",0,Nb_Win-1))==NULL) ||
            ((KURUCZ_buffers.fwhmPolySpec[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhmPolySpec",0,KURUCZ_buffers.fwhmDegree))==NULL) ||
            ((KURUCZ_buffers.fwhmVector[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhmVector",0,NDET-1))==NULL) ||
            ((KURUCZ_buffers.fwhmDeriv2[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhmVector",0,NDET-1))==NULL))

         rc=ERROR_ID_ALLOC;

        else
         {
          memcpy(KURUCZ_buffers.fwhmVector[indexParam],ANALYSE_zeros,sizeof(double)*NDET);

          for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
           {
            pTabFeno=&TabFeno[indexFeno];

            if ((pKuruczFeno->indexFwhmParam[indexParam]!=ITEM_NONE) &&
                 !pTabFeno->hidden && pTabFeno->useKurucz &&
              (((pTabFeno->fwhmPolyRef[indexParam]=(double *)MEMORY_AllocDVector("KURUCZ_Alloc ","fwhmPolyRef",0,KURUCZ_buffers.fwhmDegree))==NULL) ||
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

    if (hFilterFlag && KURUCZ_buffers.solarFGap && (lambda[NDET-1]-lambda[0]+1!=NDET) &&
     (((rc=SPLINE_Vector(KURUCZ_buffers.hrSolar.lambda,KURUCZ_buffers.hrSolar.vector,KURUCZ_buffers.hrSolar.deriv2,KURUCZ_buffers.hrSolar.NDET,
                            KURUCZ_buffers.lambdaF,KURUCZ_buffers.solarF,NDET+2*KURUCZ_buffers.solarFGap,pAnalysisOptions->interpol,"KURUCZ_Spectrum "))!=0) ||
      ((rc=FILTER_Vector(ANALYSE_phFilter,KURUCZ_buffers.solarF,KURUCZ_buffers.solarF,NDET+2*KURUCZ_buffers.solarFGap,PRJCT_FILTER_OUTPUT_LOW))!=0) ||
      ((rc=SPLINE_Deriv2(KURUCZ_buffers.lambdaF,KURUCZ_buffers.solarF,KURUCZ_buffers.solarF2,NDET+2*KURUCZ_buffers.solarFGap,"KURUCZ_Alloc (solarF) "))!=0)))

     goto EndKuruczAlloc;

    KURUCZ_Init(1);
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

  INDEX indexWindow,indexParam,indexFeno;
  KURUCZ_FENO *pKFeno;

  XSCONV_Reset(&KURUCZ_buffers.hrSolar);

  if (KURUCZ_buffers.solar!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Free ","solar",KURUCZ_buffers.solar,0);
  if (KURUCZ_buffers.lambdaF!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Free ","lambdaF",KURUCZ_buffers.lambdaF,0);
  if (KURUCZ_buffers.solarF!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Free ","solarF",KURUCZ_buffers.solarF,0);
  if (KURUCZ_buffers.solarF2!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Free ","solarF2",KURUCZ_buffers.solarF2,0);
  if (KURUCZ_buffers.offset!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Free ","offset",KURUCZ_buffers.offset,0);
  if (KURUCZ_buffers.Pcalib!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Free ","Pcalib",KURUCZ_buffers.Pcalib,1);
  if (KURUCZ_buffers.VLambda!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Free ","VLambda",KURUCZ_buffers.VLambda,1);
  if (KURUCZ_buffers.VShift!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Free ","VShift",KURUCZ_buffers.VShift,1);
  if (KURUCZ_buffers.VSig!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Free ","VSig",KURUCZ_buffers.VSig,1);
  if (KURUCZ_buffers.VPix!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Free ","VPix",KURUCZ_buffers.VPix,1);
  if (KURUCZ_buffers.pixMid!=NULL)
   MEMORY_ReleaseDVector("KURUCZ_Free ","pixMid",KURUCZ_buffers.pixMid,1);

  for (indexParam=0;(indexParam<MAX_KURUCZ_FWHM_PARAM);indexParam++)
   {
    if (KURUCZ_buffers.fwhm[indexParam]!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","fwhm",KURUCZ_buffers.fwhm[indexParam],0);
    if (KURUCZ_buffers.fwhmSigma[indexParam]!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","fwhmSigma",KURUCZ_buffers.fwhmSigma[indexParam],0);
    if (KURUCZ_buffers.fwhmPolySpec[indexParam]!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","fwhmPolySpec",KURUCZ_buffers.fwhmPolySpec[indexParam],0);
    if (KURUCZ_buffers.fwhmVector[indexParam]!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","fwhmVector",KURUCZ_buffers.fwhmVector[indexParam],0);
    if (KURUCZ_buffers.fwhmDeriv2[indexParam]!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","fwhmDeriv2",KURUCZ_buffers.fwhmDeriv2[indexParam],0);
   }

  ANALYSE_SvdFree("KURUCZ_Free (3)",&KURUCZ_buffers.svdFwhm);
  MATRIX_Free(&KURUCZ_buffers.crossFits,"KURUCZ_Free");

  if (KURUCZ_buffers.KuruczFeno!=NULL)
   {
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     {
      pKFeno=&KURUCZ_buffers.KuruczFeno[indexFeno];

      // Grid

      if (pKFeno->Grid!=NULL)
       MEMORY_ReleaseDVector("KURUCZ_Free ","Grid",pKFeno->Grid,0);

      // svdFeno

      if (pKFeno->svdFeno!=NULL)
       {
        for (indexWindow=0;indexWindow<KURUCZ_buffers.Nb_Win;indexWindow++)
         ANALYSE_SvdFree("KURUCZ_Free (1)",&pKFeno->svdFeno[indexWindow]);

        MEMORY_ReleaseBuffer("KURUCZ_Free ","svdFeno",pKFeno->svdFeno);
       }

      // fft

      if (pKFeno->fft!=NULL)
       {
        for (indexWindow=0;indexWindow<KURUCZ_buffers.Nb_Win;indexWindow++)
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
        for (indexWindow=0;indexWindow<KURUCZ_buffers.Nb_Win;indexWindow++)
         if (pKFeno->results[indexWindow]!=NULL)
          MEMORY_ReleaseBuffer("KURUCZ_Free ","results",pKFeno->results[indexWindow]);

        MEMORY_ReleaseBuffer("KURUCZ_Free ","results",pKFeno->results);
       }

      if (pKFeno->chiSquare!=NULL)
       MEMORY_ReleaseDVector("KURUCZ_Free ","chiSquare",pKFeno->chiSquare,0);
      if (pKFeno->rms!=NULL)
       MEMORY_ReleaseDVector("KURUCZ_Free ","rms",pKFeno->rms,0);
     }

    MEMORY_ReleaseBuffer("KURUCZ_Free ","KuruczFeno",KURUCZ_buffers.KuruczFeno);
   }

  memset(&KURUCZ_buffers,0,sizeof(KURUCZ));
 }

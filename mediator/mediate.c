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


#include <stdlib.h>

#include "mediate.h"
#include "engine.h"

void mediateDisplayErrorMessage(void *responseHandle)
 {
 	// Declaration

 	ERROR_DESCRIPTION errorDescription;

 	// Get the last error message

 	while (ERROR_GetLast(&errorDescription)!=0)
 	 mediateResponseErrorMessage(errorDescription.errorFunction,errorDescription.errorString,errorDescription.errorType, responseHandle);
 }

// =================================
// CREATE/DESTROY THE ENGINE CONTEXT
// =================================

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestCreateEngineContext
// -----------------------------------------------------------------------------
// PURPOSE       This function is called once the first time the engine context
//               is requested.  The engine context is never destroyed before
//               the user exits the program.
// -----------------------------------------------------------------------------

int mediateRequestCreateEngineContext(void **engineContext, void *responseHandle)
 {
  if ((*engineContext=(void *)EngineCreateContext())==NULL)
   mediateDisplayErrorMessage(responseHandle);

  return 0;
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestDestroyEngineContext
// -----------------------------------------------------------------------------
// PURPOSE       Destroy the engine context when the user exits the program.
// -----------------------------------------------------------------------------

int mediateRequestDestroyEngineContext(void *engineContext, void *responseHandle)
 {
  EngineDestroyContext(engineContext);
  return 0;
 }

// ==============================================================
// TRANSFER OF PROJECT PROPERTIES FROM THE MEDIATOR TO THE ENGINE
// ==============================================================

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectSpectra
// -----------------------------------------------------------------------------
// PURPOSE       Spectra part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectSpectra(PRJCT_SPECTRA *pEngineSpectra,const mediate_project_spectra_t *pMediateSpectra)
 {
  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("setMediateProjectSpectra",DEBUG_FCTTYPE_CONFIG);
  #endif

 	// Control of what to display

  pEngineSpectra->displaySpectraFlag=pMediateSpectra->requireSpectra;
  pEngineSpectra->displayDataFlag=pMediateSpectra->requireData;
  pEngineSpectra->displayFitFlag=pMediateSpectra->requireFits;

  // Spectral record range

  pEngineSpectra->noMin=pMediateSpectra->recordNumberMinimum;
  pEngineSpectra->noMax=pMediateSpectra->recordNumberMaximum;

  // SZA (Solar Zenith Angle) range of interest

  pEngineSpectra->SZAMin=(float)pMediateSpectra->szaMinimum;
  pEngineSpectra->SZAMax=(float)pMediateSpectra->szaMaximum;
  pEngineSpectra->SZADelta=(float)pMediateSpectra->szaDelta;

  // QDOAS ??? to move to the instrumental page

  pEngineSpectra->namesFlag=pMediateSpectra->useNameFile;
  pEngineSpectra->darkFlag=pMediateSpectra->useDarkFile;

  // Geolocation

  pEngineSpectra->mode=pMediateSpectra->geo.mode;

  switch (pEngineSpectra->mode)
   {
 // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_CIRCLE :

     pEngineSpectra->radius=pMediateSpectra->geo.circle.radius;
     pEngineSpectra->longMin=(float)pMediateSpectra->geo.circle.centerLongitude;
     pEngineSpectra->latMin=(float)pMediateSpectra->geo.circle.centerLatitude;

    break;
 // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_RECTANGLE :

     pEngineSpectra->longMin=pMediateSpectra->geo.rectangle.westernLongitude;
     pEngineSpectra->longMax=pMediateSpectra->geo.rectangle.easternLongitude;
     pEngineSpectra->latMin=pMediateSpectra->geo.rectangle.southernLatitude;
     pEngineSpectra->latMax=pMediateSpectra->geo.rectangle.northernLatitude;

    break;
 // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_OBSLIST :
     pEngineSpectra->radius=pMediateSpectra->geo.sites.radius;
    break;
 // ----------------------------------------------------------------------------
   }

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_

  DEBUG_Print("displaySpectraFlag : %d\n",pEngineSpectra->displaySpectraFlag);
  DEBUG_Print("displayDataFlag : %d\n",pEngineSpectra->displayDataFlag);
  DEBUG_Print("displayFitFlag : %d\n",pEngineSpectra->displayFitFlag);
  DEBUG_Print("noMin - noMax : %d - %d\n",pEngineSpectra->noMin,pEngineSpectra->noMax);
  DEBUG_Print("SZAMin - SZAMax - SZADelta : %.3f %.3f %.3f\n",pEngineSpectra->SZAMin,pEngineSpectra->SZAMax,pEngineSpectra->SZADelta);
  DEBUG_Print("namesFlag : %d\n",pEngineSpectra->namesFlag);
  DEBUG_Print("darkFlag : %d\n",pEngineSpectra->darkFlag);

  switch (pEngineSpectra->mode)
   {
 // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_CIRCLE :
     DEBUG_Print("Geo mode circle (radius,long,lat) : %.3f, %.3f, %.3f\n",pEngineSpectra->radius,pEngineSpectra->longMin,pEngineSpectra->longMax);
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_RECTANGLE :
     DEBUG_Print("Geo mode rectangle (long range, lat range) : %.3f - %.3f, %.3f - %.3f\n",
                  pEngineSpectra->longMin,pEngineSpectra->longMax,pEngineSpectra->latMin,pEngineSpectra->latMax);
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_OBSLIST :
     DEBUG_Print("Geo mode sites : %.3f\n",pEngineSpectra->radius);
    break;
 // ----------------------------------------------------------------------------
   }

  DEBUG_FunctionStop("setMediateProjectSpectra",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectDisplay
// -----------------------------------------------------------------------------
// PURPOSE       Spectra part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectDisplay(PRJCT_SPECTRA *pEngineSpectra,const mediate_project_display_t *pMediateDisplay)
 {
  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("setMediateProjectDisplay",DEBUG_FCTTYPE_CONFIG);
  #endif

 	// Control of what to display

  pEngineSpectra->displaySpectraFlag=pMediateDisplay->requireSpectra;
  pEngineSpectra->displayDataFlag=pMediateDisplay->requireData;
  pEngineSpectra->displayFitFlag=pMediateDisplay->requireFits;

  // the list of selected display data (same as output fields) - TODO

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_

  DEBUG_Print("displaySpectraFlag : %d\n",pEngineSpectra->displaySpectraFlag);
  DEBUG_Print("displayDataFlag : %d\n",pEngineSpectra->displayDataFlag);
  DEBUG_Print("displayFitFlag : %d\n",pEngineSpectra->displayFitFlag);

  DEBUG_FunctionStop("setMediateProjectDisplay",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectSelection
// -----------------------------------------------------------------------------
// PURPOSE       Spectra part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectSelection(PRJCT_SPECTRA *pEngineSpectra,const mediate_project_selection_t *pMediateSelection)
 {
  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("setMediateProjectSelection",DEBUG_FCTTYPE_CONFIG);
  #endif

  // Spectral record range

  pEngineSpectra->noMin=pMediateSelection->recordNumberMinimum;
  pEngineSpectra->noMax=pMediateSelection->recordNumberMaximum;

  // SZA (Solar Zenith Angle) range of interest

  pEngineSpectra->SZAMin=(float)pMediateSelection->szaMinimum;
  pEngineSpectra->SZAMax=(float)pMediateSelection->szaMaximum;
  pEngineSpectra->SZADelta=(float)pMediateSelection->szaDelta;

  // Geolocation

  pEngineSpectra->mode=pMediateSelection->geo.mode;

  switch (pEngineSpectra->mode)
   {
 // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_CIRCLE :

     pEngineSpectra->radius=pMediateSelection->geo.circle.radius;
     pEngineSpectra->longMin=(float)pMediateSelection->geo.circle.centerLongitude;
     pEngineSpectra->latMin=(float)pMediateSelection->geo.circle.centerLatitude;

    break;
 // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_RECTANGLE :

     pEngineSpectra->longMin=pMediateSelection->geo.rectangle.westernLongitude;
     pEngineSpectra->longMax=pMediateSelection->geo.rectangle.easternLongitude;
     pEngineSpectra->latMin=pMediateSelection->geo.rectangle.southernLatitude;
     pEngineSpectra->latMax=pMediateSelection->geo.rectangle.northernLatitude;

    break;
 // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_OBSLIST :
     pEngineSpectra->radius=pMediateSelection->geo.sites.radius;
    break;
 // ----------------------------------------------------------------------------
   }

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_

  DEBUG_Print("noMin - noMax : %d - %d\n",pEngineSpectra->noMin,pEngineSpectra->noMax);
  DEBUG_Print("SZAMin - SZAMax - SZADelta : %.3f %.3f %.3f\n",pEngineSpectra->SZAMin,pEngineSpectra->SZAMax,pEngineSpectra->SZADelta);
  DEBUG_Print("namesFlag : REMOVED : %d\n",pEngineSpectra->namesFlag);
  DEBUG_Print("darkFlag : REMOVED : %d\n",pEngineSpectra->darkFlag);

  switch (pEngineSpectra->mode)
   {
 // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_CIRCLE :
     DEBUG_Print("Geo mode circle (radius,long,lat) : %.3f, %.3f, %.3f\n",pEngineSpectra->radius,pEngineSpectra->longMin,pEngineSpectra->longMax);
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_RECTANGLE :
     DEBUG_Print("Geo mode rectangle (long range, lat range) : %.3f - %.3f, %.3f - %.3f\n",
                  pEngineSpectra->longMin,pEngineSpectra->longMax,pEngineSpectra->latMin,pEngineSpectra->latMax);
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_OBSLIST :
     DEBUG_Print("Geo mode sites : %.3f\n",pEngineSpectra->radius);
    break;
 // ----------------------------------------------------------------------------
   }

  DEBUG_FunctionStop("setMediateProjectSelection",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Analysis part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectAnalysis(PRJCT_ANLYS *pEngineAnalysis,const mediate_project_analysis_t *pMediateAnalysis)
 {
 	#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_

  UCHAR *prjctAnlysMethods[PRJCT_ANLYS_METHOD_MAX]={"Optical density fitting","Intensity fitting (Marquardt-Levenberg+SVD)"};
  UCHAR *prjctAnlysFitWeighting[PRJCT_ANLYS_FIT_WEIGHTING_MAX]={"No weighting","Instrumental weighting"};
  UCHAR *prjctAnlysInterpol[PRJCT_ANLYS_INTERPOL_MAX]={"linear","spline"};

  DEBUG_FunctionBegin("setMediateProjectAnalysis",DEBUG_FCTTYPE_CONFIG);
  #endif

  pEngineAnalysis->method=pMediateAnalysis->methodType;                         // analysis method
  pEngineAnalysis->fitWeighting=pMediateAnalysis->fitType;                      // least-squares fit weighting
  pEngineAnalysis->units=PRJCT_ANLYS_UNITS_NANOMETERS;                          // units for shift and stretch : force nm with QDOAS !!!
  pEngineAnalysis->interpol=pMediateAnalysis->interpolationType;                // interpolation
  pEngineAnalysis->convergence=pMediateAnalysis->convergenceCriterion;          // convergence criterion
  pEngineAnalysis->securityGap=pMediateAnalysis->interpolationSecurityGap;      // security pixels to take in order to avoid interpolation problems at the edge of the spectral window

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_Print("Analysis method : %s\n",prjctAnlysMethods[pEngineAnalysis->method]);
  DEBUG_Print("Least-squares fit weighting : %s\n",prjctAnlysFitWeighting[pEngineAnalysis->fitWeighting]);
  DEBUG_Print("Interpolation method : %s\n",prjctAnlysInterpol[pEngineAnalysis->interpol]);
  DEBUG_Print("Convergence criteria : %g\n",pEngineAnalysis->convergence);
  DEBUG_Print("Number of security pixels : %d\n",pEngineAnalysis->securityGap);

  DEBUG_FunctionStop("setMediateProjectAnalysis",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectFiltering
// -----------------------------------------------------------------------------
// PURPOSE       Filtering part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectFiltering(PRJCT_FILTER *pEngineFilter,const mediate_filter_t *pMediateFilter)
 {
 	#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("setMediateProjectFiltering",DEBUG_FCTTYPE_CONFIG);
  #endif

  // Still to do by Caro

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionStop("setMediateProjectFiltering",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectCalibration
// -----------------------------------------------------------------------------
// PURPOSE       Calibration part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectCalibration(PRJCT_KURUCZ *pEngineCalibration,CALIB_FENO *pEngineCalibFeno,const mediate_project_calibration_t *pMediateCalibration)
 {
 	#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
 	UCHAR *prjctAnlysMethods[PRJCT_ANLYS_METHOD_MAX]={"Optical density fitting","Intensity fitting (Marquardt-Levenberg+SVD)"};
  DEBUG_FunctionBegin("setMediateProjectCalibration",DEBUG_FCTTYPE_CONFIG);
  #endif

  strcpy(pEngineCalibration->file,pMediateCalibration->solarRefFile);           // kurucz file

  pEngineCalibration->analysisMethod=pMediateCalibration->methodType;           // analysis method type
  pEngineCalibration->windowsNumber=pMediateCalibration->subWindows;            // number of windows
  pEngineCalibration->fwhmPolynomial=pMediateCalibration->sfpDegree;            // security gap in pixels numbers
  pEngineCalibration->shiftPolynomial=pMediateCalibration->shiftDegree;         // degree of polynomial to use

  pEngineCalibration->displayFit=pMediateCalibration->requireFits;              // display fit flag
  pEngineCalibration->displayResidual=pMediateCalibration->requireResidual;     // display new calibration flag
  pEngineCalibration->displayShift=pMediateCalibration->requireShiftSfp;        // display shift/Fwhm in each pixel
  pEngineCalibration->displaySpectra=pMediateCalibration->requireSpectra;       // display fwhm in each pixel
  pEngineCalibration->fwhmFit=(pMediateCalibration->lineShape>0)?1:0;           // force fit of fwhm while applying Kurucz
  pEngineCalibration->fwhmType=pMediateCalibration->lineShape;                  // type of slit function to fit
  pEngineCalibration->lambdaLeft=pMediateCalibration->wavelengthMin;            // minimum wavelength for the spectral interval
  pEngineCalibration->lambdaRight=pMediateCalibration->wavelengthMax;           // maximum wavelength for the spectral interval
  pEngineCalibration->invPolyDegree=pMediateCalibration->lorentzDegree;         // degree of the lorentzian

  // !!! tables will be loaded by mediateRequestSetAnalysisWindows !!!

  memcpy(&pEngineCalibFeno->crossSectionList,&pMediateCalibration->crossSectionList,sizeof(cross_section_list_t));
  memcpy(&pEngineCalibFeno->linear,&pMediateCalibration->linear,sizeof(struct anlyswin_linear));
  memcpy(pEngineCalibFeno->sfp,pMediateCalibration->sfp,sizeof(struct calibration_sfp)*4); // SFP1 .. SFP4
  memcpy(&pEngineCalibFeno->shiftStretchList,&pMediateCalibration->shiftStretchList,sizeof(shift_stretch_list_t));
  memcpy(&pEngineCalibFeno->outputList,&pMediateCalibration->outputList,sizeof(output_list_t));

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_Print("Analysis method : %s\n",prjctAnlysMethods[pMediateCalibration->methodType]);
  DEBUG_Print("Kurucz file : %s\n",pEngineCalibration->file);
  DEBUG_Print("Spectral range : %g - %g nm\n",pEngineCalibration->lambdaLeft,pEngineCalibration->lambdaRight);
  DEBUG_Print("Display %d %d %d %d (Spectra,Residual,Fit,Shift/SFP)\n",pEngineCalibration->displaySpectra,pEngineCalibration->displayResidual,pEngineCalibration->displayFit,pEngineCalibration->displayShift);
  DEBUG_Print("Number of windows: %d\n",pEngineCalibration->windowsNumber);
  DEBUG_Print("Degree of polynomials (shift/SFP) : %d - %d\n",pEngineCalibration->shiftPolynomial,pEngineCalibration->fwhmPolynomial);
  DEBUG_Print("Fit the slit function (slit function type) : %d (%d)\n",pEngineCalibration->fwhmFit,pEngineCalibration->fwhmType);
  DEBUG_FunctionStop("setMediateProjectCalibration",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectUndersampling
// -----------------------------------------------------------------------------
// PURPOSE       Undersampling part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectUndersampling(PRJCT_USAMP *pEngineUsamp,const mediate_project_undersampling_t *pMediateUsamp)
 {
 	#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("setMediateProjectUndersampling",DEBUG_FCTTYPE_CONFIG);
  #endif

  // Still to do by Caro

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionStop("setMediateProjectUndersampling",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectInstrumental
// -----------------------------------------------------------------------------
// PURPOSE       Instrumental part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectInstrumental(PRJCT_INSTRUMENTAL *pEngineInstrumental,const mediate_project_instrumental_t *pMediateInstrumental)
 {
 	INDEX indexCluster;

 	#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("setMediateProjectInstrumental",DEBUG_FCTTYPE_CONFIG);
  #endif

	 pEngineInstrumental->readOutFormat=(UCHAR)pMediateInstrumental->format;                           // File format
	 strcpy(pEngineInstrumental->observationSite,pMediateInstrumental->siteName); 		                   // Observation site
	 NDET=0;

	 switch (pEngineInstrumental->readOutFormat)
	  {
	// ----------------------------------------------------------------------------
	   case PRJCT_INSTR_FORMAT_ACTON :                                                                 // Acton (NILU)

	  	 NDET=1024;                                                                     // size of the detector

	  	 pEngineInstrumental->user=pMediateInstrumental->acton.niluType;                                // old or new format

	  	 strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->acton.calibrationFile);      // calibration file
	  	 strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->acton.instrFunctionFile);      // instrumental function file

	  	break;
	// ----------------------------------------------------------------------------
	  	case PRJCT_INSTR_FORMAT_LOGGER :                                                                // Logger (PDA,CCD or HAMAMATSU)

	  	 NDET=1024;                                                                     // size of the detector
	  	 pEngineInstrumental->azimuthFlag=(int)pMediateInstrumental->logger.flagAzimuthAngle;           // format including or not the azimuth angle
	  	 pEngineInstrumental->user=pMediateInstrumental->logger.spectralType;                           // spectrum type (offaxis or zenith)

	  	 strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->logger.calibrationFile);     // calibration file
	  	 strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->logger.instrFunctionFile);     // instrumental function file

	  	break;
	// ----------------------------------------------------------------------------
	  	case PRJCT_INSTR_FORMAT_ASCII :                                                                 // Format ASCII

	  	 NDET=pMediateInstrumental->ascii.detectorSize;                                 // size of the detector

     pEngineInstrumental->ascii.format=pMediateInstrumental->ascii.format;                          // format line or column
     pEngineInstrumental->ascii.szaSaveFlag=pMediateInstrumental->ascii.flagZenithAngle;            // 1 if the solar zenith angle information is saved in the file
     pEngineInstrumental->ascii.azimSaveFlag=pMediateInstrumental->ascii.flagAzimuthAngle;          // 1 if the solar azimuth angle information is saved in the file
     pEngineInstrumental->ascii.elevSaveFlag=pMediateInstrumental->ascii.flagElevationAngle;        // 1 if the viewing elevation angle information is saved in the file
     pEngineInstrumental->ascii.timeSaveFlag=pMediateInstrumental->ascii.flagTime;                  // 1 if the time information is saved in the file
     pEngineInstrumental->ascii.dateSaveFlag=pMediateInstrumental->ascii.flagDate;                  // 1 if the date information is saved in the file
     pEngineInstrumental->ascii.lambdaSaveFlag=pMediateInstrumental->ascii.flagWavelength;          // 1 if the wavelength calibration is saved with spectra in the file

	  	 strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->ascii.calibrationFile);      // calibration file
	  	 strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->ascii.instrFunctionFile);      // instrumental function file

	  	break;
	// ----------------------------------------------------------------------------
	  	case PRJCT_INSTR_FORMAT_PDAEGG :                                                                // PDA EG&G (sept. 94 until now)

	  	 NDET=1024;                                                                     // size of the detector

	  	 pEngineInstrumental->azimuthFlag=(int)pMediateInstrumental->pdaegg.flagAzimuthAngle;           // format including or not the azimuth angle
	  	 pEngineInstrumental->user=pMediateInstrumental->pdaegg.spectralType;                           // spectrum type (offaxis or zenith)

	  	 strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->pdaegg.calibrationFile);     // calibration file
	  	 strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->pdaegg.instrFunctionFile);     // instrumental function file

	  	break;
	// ----------------------------------------------------------------------------
	   case PRJCT_INSTR_FORMAT_PDASI_EASOE :                                                           // PDA SI (IASB)

	  	 NDET=1024;                                                                     // size of the detector

	  	 strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->pdasieasoe.calibrationFile);      // calibration file
	  	 strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->pdasieasoe.instrFunctionFile);      // instrumental function file

	  	break;
	// ----------------------------------------------------------------------------
	  	case PRJCT_INSTR_FORMAT_SAOZ_PCDNMOS :                                                          // SAOZ PCD/NMOS 512

	  	 NDET=512;                                                                      // size of the detector

     pEngineInstrumental->saoz.spectralRegion=pMediateInstrumental->saozvis.spectralRegion;         // spectral region (UV or visible)
     pEngineInstrumental->saoz.spectralType=pMediateInstrumental->saozvis.spectralType;             // spectral type (zenith sky or pointed measuremets

	  	 strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->saozvis.calibrationFile);    // calibration file
	  	 strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->saozvis.instrFunctionFile);    // instrumental function file

	  	break;
	// ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_EFM :                                                              // SAOZ EFM 1024

     NDET=1024;                                                                     // size of the detector

	  	 strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->saozefm.calibrationFile);    // calibration file
	  	 strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->saozefm.instrFunctionFile);    // instrumental function file

    break;
	// ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_RASAS :                                                                 // Format RASAS (INTA)

     NDET=1024;                                                                     // size of the detector

	  	 strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->rasas.calibrationFile);      // calibration file
	  	 strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->rasas.instrFunctionFile);      // instrumental function file

    break;
	// ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_NOAA :                                                                  // NOAA

     NDET=1024;                                                                     // size of the detector

	  	 strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->noaa.calibrationFile);       // calibration file
	  	 strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->noaa.instrFunctionFile);       // instrumental function file

    break;
	// ----------------------------------------------------------------------------
	   case PRJCT_INSTR_FORMAT_SCIA_PDS :                                                              // SCIAMACHY calibrated Level 1 data in PDS format

	    NDET=1024;

	  	 strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->sciapds.calibrationFile);    // calibration file
	  	 strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->sciapds.instrFunctionFile);    // instrumental function file

	  	 pEngineInstrumental->scia.sciaChannel=pMediateInstrumental->sciapds.channel;

	  	 for (indexCluster=SCIA_clusters[pEngineInstrumental->scia.sciaChannel][0];
	  	      indexCluster<=SCIA_clusters[pEngineInstrumental->scia.sciaChannel][1];
	  	      indexCluster++)

  	   pEngineInstrumental->scia.sciaCluster[indexCluster-SCIA_clusters[pEngineInstrumental->scia.sciaChannel][0]]=
  	    (pMediateInstrumental->sciapds.clusters[indexCluster])?1:0;

	  	 memcpy(pEngineInstrumental->scia.sciaReference,pMediateInstrumental->sciapds.sunReference,4);

    break;
	// ----------------------------------------------------------------------------
	   case PRJCT_INSTR_FORMAT_GDP_BIN :                                                               // GOME WinDOAS BINARY format

	    NDET=1024;                                                                     // Could be reduced by Set function

	  	 strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->gdpbin.calibrationFile);    // calibration file
	  	 strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->gdpbin.instrFunctionFile);    // instrumental function file

	  	 pEngineInstrumental->user=pMediateInstrumental->gdpbin.bandType;

    break;
	  }

	 // ------------------------

	 // Set the detector size (implicit to the selection of the file format)

/*  switch (pEngineInstrumental->readOutFormat)
   {
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_CCD_EEV :                          // CCD EEV 1340x400
     NDET=(pEngineInstrumental->detectorSize)?pEngineInstrumental->detectorSize:1340;
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_CCD_OHP_96 :                       // CCD OHP 96
    case PRJCT_INSTR_FORMAT_CCD_HA_94 :                        // CCD Harestua 94
    case PRJCT_INSTR_FORMAT_PDASI_EASOE :                      // PDA SI (IASB)
    case PRJCT_INSTR_FORMAT_GDP_ASCII :                        // GOME GDP ASCII format
    case PRJCT_INSTR_FORMAT_OMI :                              // OMI
	   case PRJCT_INSTR_FORMAT_GOME2 :                            // GOME2
     NDET=1024;
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_UOFT :                             // University of Toronto
     NDET=2000;
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC :                              // MFC Heidelberg
    case PRJCT_INSTR_FORMAT_MFC_STD :                          // MFC Heidelberg (ASCII)
    default :
     NDET=pEngineInstrumental->detectorSize;
    break;
 // ---------------------------------------------------------------------------
   } */

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionStop("setMediateProjectInstrumental",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectSlit
// -----------------------------------------------------------------------------
// PURPOSE       Slit part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectSlit(PRJCT_SLIT *pEngineSlit,const mediate_project_slit_t *pMediateSlit)
 {
 	#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("setMediateProjectSlit",DEBUG_FCTTYPE_CONFIG);
  #endif

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionStop("setMediateProjectSlit",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectOutput
// -----------------------------------------------------------------------------
// PURPOSE       Output (binary/ascii format) part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectOutput(PRJCT_RESULTS_ASCII *pEngineOutput,const mediate_project_output_t *pMediateOutput)
 {
 	#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("setMediateProjectOutput",DEBUG_FCTTYPE_CONFIG);
  #endif

  // Still to do by Caro

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionStop("setMediateProjectOutput",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestSetProject
// -----------------------------------------------------------------------------
// PURPOSE       Interface between the mediator and the engine for project properties
// -----------------------------------------------------------------------------

int mediateRequestSetProject(void *engineContext,
			     const mediate_project_t *project, void *responseHandle)
 {
 	// Declarations

	 ENGINE_CONTEXT *pEngineContext;                                               // engine context
	 PROJECT *pEngineProject;                                                      // project part of the engine

	 // Initializations

	 pEngineContext=(ENGINE_CONTEXT *)engineContext;
	 pEngineProject=(PROJECT *)&pEngineContext->project;

	 // Transfer projects options from the mediator to the engine

	 #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Start(ENGINE_dbgFile,"Project",DEBUG_FCTTYPE_CONFIG,5,DEBUG_DVAR_YES,0);
  #endif

  setMediateProjectDisplay(&pEngineProject->spectra,&project->display);
  setMediateProjectSelection(&pEngineProject->spectra,&project->selection);
  setMediateProjectAnalysis(&pEngineProject->analysis,&project->analysis);
  setMediateProjectFiltering(&pEngineProject->lfilter,&project->lowpass);
  setMediateProjectFiltering(&pEngineProject->hfilter,&project->highpass);
  setMediateProjectCalibration(&pEngineProject->kurucz,&pEngineContext->calibFeno,&project->calibration);
  setMediateProjectUndersampling(&pEngineProject->usamp,&project->undersampling);
  setMediateProjectInstrumental(&pEngineProject->instrumental,&project->instrumental);
  setMediateProjectSlit(&pEngineProject->slit,&project->slit);
  setMediateProjectOutput(&pEngineProject->asciiResults,&project->output);

	 // Allocate buffers requested by the project

  EngineSetProject(pEngineContext);

	 #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Stop("Project");
  #endif

  return 0;
 }

// =======================================================================
// TRANSFER OF ANALYSIS WINDOWS PROPERTIES FROM THE MEDIATOR TO THE ENGINE
// =======================================================================

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestSetAnalysisLinear
// -----------------------------------------------------------------------------
// PURPOSE       Load linear parameters
// -----------------------------------------------------------------------------

RC mediateRequestSetAnalysisLinear(struct anlyswin_linear *pLinear)
 {
  // Declarations

  ANALYSE_LINEAR_PARAMETERS linear[3];
  RC rc;

 	// Debug

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("mediateRequestSetAnalysisLinear",DEBUG_FCTTYPE_CONFIG);
  #endif

  // Polynomial (x)

  strcpy(linear[0].symbolName,"Polynomial (x)");

  linear[0].polyOrder=pLinear->xPolyOrder-1;
  linear[0].baseOrder=pLinear->xBaseOrder-1;
  linear[0].storeFit=pLinear->xFlagFitStore;
  linear[0].storeError=pLinear->xFlagErrStore;

  // Polynomial (1/x)

  strcpy(linear[1].symbolName,"Polynomial (1/x)");

  linear[1].polyOrder=pLinear->xinvPolyOrder-1;
  linear[1].baseOrder=pLinear->xinvBaseOrder-1;
  linear[1].storeFit=pLinear->xinvFlagFitStore;
  linear[1].storeError=pLinear->xinvFlagErrStore;

  // Linear offset

  strcpy(linear[2].symbolName,"Offset");

  linear[2].polyOrder=pLinear->offsetPolyOrder-1;
  linear[2].baseOrder=pLinear->offsetBaseOrder-1;
  linear[2].storeFit=pLinear->offsetFlagFitStore;
  linear[2].storeError=pLinear->offsetFlagErrStore;

  rc=ANALYSE_LoadLinear(linear,3);

  // Debug

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionStop("mediateRequestSetAnalysisLinear",rc);
  #endif

  // Return

  return rc;
 }

// Caro : int the future, replace structures anlyswin_nonlinear and calibration_sfp with the following one more flexible

// typedef struct _AnalyseNonLinearParameters
//  {
//  	UCHAR symbolName[MAX_ITEM_TEXT_LEN+1];
//  	UCHAR crossFileName[MAX_ITEM_TEXT_LEN+1];
//  	int fitFlag;
//  	double initialValue;
//  	double deltaValue;
//  	double minValue;
//  	double maxValue;
//  	int storeFit;
//  	int storeError;
//  }
// ANALYSE_NON_LINEAR_PARAMETERS;

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestSetAnalysisNonLinearCalib
// -----------------------------------------------------------------------------
// PURPOSE       Load non linear parameters for the calibration
// -----------------------------------------------------------------------------

#define NNONLINEAR_CALIB 4

RC mediateRequestSetAnalysisNonLinearCalib(struct calibration_sfp *nonLinearCalib,double *lambda)
 {
  // Declarations

  ANALYSE_NON_LINEAR_PARAMETERS nonLinear[NNONLINEAR_CALIB];
  INDEX indexNonLinear;
  RC rc;

 	// Debug

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("mediateRequestSetAnalysisNonLinearCalib",DEBUG_FCTTYPE_CONFIG);
  #endif

  // Initialization

  memset(nonLinear,0,sizeof(ANALYSE_NON_LINEAR_PARAMETERS)*NNONLINEAR_CALIB);

  for (indexNonLinear=0;indexNonLinear<NNONLINEAR_CALIB;indexNonLinear++)
   {
    nonLinear[indexNonLinear].minValue=nonLinear[indexNonLinear].maxValue=(double)0.;
    sprintf(nonLinear[indexNonLinear].symbolName,"SFP %d",indexNonLinear+1);

    nonLinear[indexNonLinear].fitFlag=nonLinearCalib[indexNonLinear].fitFlag;
    nonLinear[indexNonLinear].initialValue=nonLinearCalib[indexNonLinear].initialValue;
    nonLinear[indexNonLinear].deltaValue=nonLinearCalib[indexNonLinear].deltaValue;
    nonLinear[indexNonLinear].storeFit=nonLinearCalib[indexNonLinear].fitStore;
    nonLinear[indexNonLinear].storeError=nonLinearCalib[indexNonLinear].errStore;
   }

  rc=ANALYSE_LoadNonLinear(nonLinear,NNONLINEAR_CALIB,lambda);

  // Debug

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionStop("mediateRequestSetAnalysisNonLinearCalib",rc);
  #endif

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestSetAnalysisNonLinearDoas
// -----------------------------------------------------------------------------
// PURPOSE       Load non linear parameters
// -----------------------------------------------------------------------------

#define NNONLINEAR_DOAS 8

RC mediateRequestSetAnalysisNonLinearDoas(struct anlyswin_nonlinear *pNonLinear,double *lambda)
 {
  // Declarations

  ANALYSE_NON_LINEAR_PARAMETERS nonLinear[NNONLINEAR_DOAS];
  INDEX indexNonLinear;
  RC rc;

 	// Debug

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("mediateRequestSetAnalysisNonLinearDoas",DEBUG_FCTTYPE_CONFIG);
  #endif

  // Initialization

  memset(nonLinear,0,sizeof(ANALYSE_NON_LINEAR_PARAMETERS)*NNONLINEAR_DOAS);                 // this reset cross section file names
                                                                                // QDOAS MISSING FIELD !!!
  for (indexNonLinear=0;indexNonLinear<NNONLINEAR_DOAS;indexNonLinear++)
   nonLinear[indexNonLinear].minValue=nonLinear[indexNonLinear].maxValue=(double)0.;

  // Sol

  strcpy(nonLinear[0].symbolName,"Sol");

  nonLinear[0].fitFlag=pNonLinear->solFlagFit;
  nonLinear[0].initialValue=pNonLinear->solInitial;
  nonLinear[0].deltaValue=pNonLinear->solDelta;
  nonLinear[0].storeFit=pNonLinear->solFlagFitStore;
  nonLinear[0].storeError=pNonLinear->solFlagErrStore;

  // Offset (Constant)

  strcpy(nonLinear[1].symbolName,"Offset (Constant)");

  nonLinear[1].fitFlag=pNonLinear->off0FlagFit;
  nonLinear[1].initialValue=pNonLinear->off0Initial;
  nonLinear[1].deltaValue=pNonLinear->off0Delta;
  nonLinear[1].storeFit=pNonLinear->off0FlagFitStore;
  nonLinear[1].storeError=pNonLinear->off0FlagErrStore;

  // Offset (Order 1)

  strcpy(nonLinear[2].symbolName,"Offset (Order 1)");

  nonLinear[2].fitFlag=pNonLinear->off1FlagFit;
  nonLinear[2].initialValue=pNonLinear->off1Initial;
  nonLinear[2].deltaValue=pNonLinear->off1Delta;
  nonLinear[2].storeFit=pNonLinear->off1FlagFitStore;
  nonLinear[2].storeError=pNonLinear->off1FlagErrStore;

  // Offset (Order 2)

  strcpy(nonLinear[3].symbolName,"Offset (Order 2)");

  nonLinear[3].fitFlag=pNonLinear->off2FlagFit;
  nonLinear[3].initialValue=pNonLinear->off2Initial;
  nonLinear[3].deltaValue=pNonLinear->off2Delta;
  nonLinear[3].storeFit=pNonLinear->off2FlagFitStore;
  nonLinear[3].storeError=pNonLinear->off2FlagErrStore;

  // Com

  strcpy(nonLinear[4].symbolName,"Com");
  strcpy(nonLinear[4].crossFileName,pNonLinear->comFile);

  nonLinear[4].fitFlag=pNonLinear->comFlagFit;
  nonLinear[4].initialValue=pNonLinear->comInitial;
  nonLinear[4].deltaValue=pNonLinear->comDelta;
  nonLinear[4].storeFit=pNonLinear->comFlagFitStore;
  nonLinear[4].storeError=pNonLinear->comFlagErrStore;

  // Usamp1

  strcpy(nonLinear[5].symbolName,"Usamp1");
  strcpy(nonLinear[5].crossFileName,pNonLinear->usamp1File);

  nonLinear[5].fitFlag=pNonLinear->usamp1FlagFit;
  nonLinear[5].initialValue=pNonLinear->usamp1Initial;
  nonLinear[5].deltaValue=pNonLinear->usamp1Delta;
  nonLinear[5].storeFit=pNonLinear->usamp1FlagFitStore;
  nonLinear[5].storeError=pNonLinear->usamp1FlagErrStore;

  // Usamp2

  strcpy(nonLinear[6].symbolName,"Usamp2");
  strcpy(nonLinear[6].crossFileName,pNonLinear->usamp2File);

  nonLinear[6].fitFlag=pNonLinear->usamp2FlagFit;
  nonLinear[6].initialValue=pNonLinear->usamp2Initial;
  nonLinear[6].deltaValue=pNonLinear->usamp2Delta;
  nonLinear[6].storeFit=pNonLinear->usamp2FlagFitStore;
  nonLinear[6].storeError=pNonLinear->usamp2FlagErrStore;

  // Raman

  strcpy(nonLinear[7].symbolName,"Raman");
  strcpy(nonLinear[7].crossFileName,pNonLinear->ramanFile);

  nonLinear[7].fitFlag=pNonLinear->ramanFlagFit;
  nonLinear[7].initialValue=pNonLinear->ramanInitial;
  nonLinear[7].deltaValue=pNonLinear->ramanDelta;
  nonLinear[7].storeFit=pNonLinear->ramanFlagFitStore;
  nonLinear[7].storeError=pNonLinear->ramanFlagErrStore;

  rc=ANALYSE_LoadNonLinear(nonLinear,NNONLINEAR_DOAS,lambda);

  // Debug

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionStop("mediateRequestSetAnalysisNonLinearDoas",rc);
  #endif

  // Return

  return rc;
 }

int mediateRequestSetAnalysisWindows(void *engineContext,
				     int numberOfWindows,
				     const mediate_analysis_window_t *analysisWindows,
				     int operatingMode,
				     void *responseHandle)
 {
	 // Declarations

  double lambdaMin,lambdaMax;
  INT useKurucz,                                                                // flag set if Kurucz is to be used
      useUsamp,                                                                 // flag set if undersampling correction is requested
      saveFlag;
  INDEX indexKurucz;
	 ENGINE_CONTEXT *pEngineContext;                                               // engine context
	 mediate_analysis_window_t *pAnalysisWindows;                                  // pointer to the current analysis window from the user interface
	 mediate_analysis_window_t calibWindows;                                       // pointer to the calibration parameters
	 FENO *pTabFeno;                                                               // pointer to the description of an analysis window
	 INT indexFeno,i;                                                              // browse analysis windows
	 RC rc;                                                                        // return code

	 // Debug

	 #if defined(__DEBUG_) && __DEBUG_
	 WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a general description of a symbol
  CROSS_REFERENCE *pEngineCross;
  INT indexTabCross;
  DEBUG_Start(ENGINE_dbgFile,"Analysis windows",DEBUG_FCTTYPE_CONFIG,5,DEBUG_DVAR_NO,0);
  #endif

	 // Initializations

  lambdaMin=1000;
  lambdaMax=0;
	 pEngineContext=(ENGINE_CONTEXT *)engineContext;
  saveFlag=(INT)pEngineContext->project.spectra.displayDataFlag;
  useKurucz=useUsamp=0;
  indexKurucz=ITEM_NONE;

  THRD_id=THREAD_TYPE_ANALYSIS;  // QDOAS : force the thread type to Run Analysis (currently not the possibility to make the difference between  Run Analysis and Run Calibration)

  memset(&calibWindows,0,sizeof(mediate_analysis_window_t));

  memcpy(&calibWindows.crossSectionList,&pEngineContext->calibFeno.crossSectionList,sizeof(cross_section_list_t));
  memcpy(&calibWindows.linear,&pEngineContext->calibFeno.linear,sizeof(struct anlyswin_linear));
  memcpy(&calibWindows.shiftStretchList,&pEngineContext->calibFeno.shiftStretchList,sizeof(shift_stretch_list_t));
  memcpy(&calibWindows.outputList,&pEngineContext->calibFeno.outputList,sizeof(output_list_t));

  // Reinitialize all global variables used for the analysis, release old buffers and allocate new ones

	 rc=ANALYSE_SetInit(pEngineContext);

	 // Load analysis windows

  for (indexFeno=0;(indexFeno<numberOfWindows+1) && !rc;indexFeno++)            // if indexFeno==0, load calibration parameters
   {                                                                            // otherwise, load analysis windows from analysisWindows[indexFeno-1]
   	// Pointers initialization

   	pTabFeno=(FENO *)&TabFeno[NFeno];

   	pTabFeno->hidden=!indexFeno;
   	pAnalysisWindows=(!pTabFeno->hidden)?(mediate_analysis_window_t *)&analysisWindows[indexFeno-1]:(mediate_analysis_window_t *)&calibWindows;
    pTabFeno->NDET=NDET;

    if ((pTabFeno->hidden<2) && ((THRD_id==THREAD_TYPE_ANALYSIS) || (pTabFeno->hidden==1)))               // QDOAS : avoid the load of disabled analysis windows with hidden==2
     {
     	if (pTabFeno->hidden)
     	 {
     	  strcpy(pTabFeno->windowName,"Calibration description");                 // like WinDOAS
     	  pTabFeno->analysisMethod=pKuruczOptions->analysisMethod;
     	 }
     	else
     	 {
        // Load data from analysis windows panels

        strcpy(pTabFeno->windowName,pAnalysisWindows->name);
        strcpy(pTabFeno->residualsFile,pAnalysisWindows->residualFile);
        strcpy(pTabFeno->ref1,pAnalysisWindows->refOneFile);
        strcpy(pTabFeno->ref2,pAnalysisWindows->refTwoFile);

        if ((pTabFeno->refSpectrumSelectionMode=pAnalysisWindows->refSpectrumSelection)==ANLYS_REF_SELECTION_MODE_AUTOMATIC)
         {
          pTabFeno->refSZA=(double)pAnalysisWindows->refSzaCenter;
          pTabFeno->refSZADelta=(double)pAnalysisWindows->refSzaDelta;

          // MISSING FIELD !!! pTabFeno->refLatMin=pAnalysisWindows->refLatMin;
          // MISSING FIELD !!! pTabFeno->refLatMax=pAnalysisWindows->refLatMax;
          // MISSING FIELD !!! pTabFeno->refLonMin=pAnalysisWindows->refLonMin;
          // MISSING FIELD !!! pTabFeno->refLonMax=pAnalysisWindows->refLonMax;

          // MISSING FIELD !!! if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
          // MISSING FIELD !!!     (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))
          // MISSING FIELD !!!
          // MISSING FIELD !!!  memcpy(pTabFeno->gomePixelType,pAnalysisWindows->gomePixelType,4);
          // MISSING FIELD !!!
          // MISSING FIELD !!! else
          // MISSING FIELD !!!  memset(pTabFeno->gomePixelType,0,4);

          // MISSING FIELD !!! pTabFeno->nspectra=pAnalysisWindows->nspectra;

          ANALYSE_refSelectionFlag++;

          if ((fabs(pTabFeno->refLonMax-pTabFeno->refLonMin)>1.e-5) ) // && (fabs(pTabFeno->refLonMax-pTabFeno->refLonMin)<359.))
           ANALYSE_lonSelectionFlag++;
         }

        if (pEngineContext->project.spectra.displayFitFlag)
         {
          pTabFeno->displaySpectrum=pAnalysisWindows->requireSpectrum;
          pTabFeno->displayResidue=pAnalysisWindows->requireResidual;
          pTabFeno->displayTrend=pAnalysisWindows->requirePolynomial;
          pTabFeno->displayRefEtalon=pAnalysisWindows->requireRefRatio;
          pTabFeno->displayFits=pAnalysisWindows->requireFit;
          pTabFeno->displayPredefined=pAnalysisWindows->requirePredefined;
         }

        pTabFeno->useKurucz=pAnalysisWindows->kuruczMode;
        pTabFeno->analysisMethod=pAnalysisOptions->method;
        useKurucz+=pAnalysisWindows->kuruczMode;
       }

      pTabFeno->Decomp=1;

      // Wavelength scales read out

      if (((pTabFeno->Lambda==NULL) && ((pTabFeno->Lambda=MEMORY_AllocDVector("ANALYSE_LoadData ","Lambda",0,NDET-1))==NULL)) ||
          ((pTabFeno->LambdaK==NULL) && ((pTabFeno->LambdaK=MEMORY_AllocDVector("ANALYSE_LoadData ","LambdaK",0,NDET-1))==NULL)) ||
          ((pTabFeno->LambdaRef==NULL) && ((pTabFeno->LambdaRef=MEMORY_AllocDVector("ANALYSE_LoadData ","LambdaRef",0,NDET-1))==NULL)))
       {
        rc=ERROR_ID_ALLOC;
        break;
       }

      for (i=0;i<NDET;i++)
       pTabFeno->LambdaRef[i]=i;  // NB : for satellites measurements, irradiance is retrieved later from spectra files

//       if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GDP_ASCII) &&
//           (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GDP_BIN) &&
//           (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_HDF) &&
//           (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS))

        memcpy(pTabFeno->LambdaRef,pEngineContext->buffers.lambda,sizeof(double)*NDET);

      if (!(rc=ANALYSE_LoadRef(pEngineContext)) &&   // eventually, modify LambdaRef for continuous functions
          !(rc=ANALYSE_LoadCross(pAnalysisWindows->crossSectionList.crossSection,pAnalysisWindows->crossSectionList.nCrossSection,pTabFeno->hidden,pTabFeno->LambdaRef)) &&
          !(rc=mediateRequestSetAnalysisLinear(&pAnalysisWindows->linear)) &&

         // Caro : int the future, replace structures anlyswin_nonlinear and calibration_sfp with the following one more flexible
         //        mediateRequestSetAnalysisNonLinearDoas and mediateRequestSetAnalysisNonLinearCalib would be replaced by only one call to ANALYSE_LoadNonLinear

         ((!pTabFeno->hidden && !(rc=mediateRequestSetAnalysisNonLinearDoas(&pAnalysisWindows->nonlinear,pTabFeno->LambdaRef))) ||
           (pTabFeno->hidden && !(rc=mediateRequestSetAnalysisNonLinearCalib(pEngineContext->calibFeno.sfp,pTabFeno->LambdaRef)))) &&

          !(rc=ANALYSE_LoadShiftStretch(pAnalysisWindows->shiftStretchList.shiftStretch,pAnalysisWindows->shiftStretchList.nShiftStretch)) &&
           (pTabFeno->hidden ||
         (!(rc=ANALYSE_LoadGaps(pAnalysisWindows->gapList.gap,pAnalysisWindows->gapList.nGap,pTabFeno->LambdaRef,pAnalysisWindows->fitMinWavelength,pAnalysisWindows->fitMaxWavelength)) &&
          (!pTabFeno->gomeRefFlag || !(rc=ANALYSE_SvdLocalAlloc("ANALYSE_LoadData",&pTabFeno->svd)))
         )))
       {
        if (pTabFeno->hidden==1)
         indexKurucz=NFeno;
        else
         {
          useUsamp+=pTabFeno->useUsamp;

          if (pTabFeno->gomeRefFlag || MFC_refFlag)
           {
            memcpy(pTabFeno->Lambda,pTabFeno->LambdaRef,sizeof(double)*NDET);
            memcpy(pTabFeno->LambdaK,pTabFeno->LambdaRef,sizeof(double)*NDET);

            if (pTabFeno->LambdaRef[NDET-1]-pTabFeno->Lambda[0]+1!=NDET)
             rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef);
           }
         }

        ANALYSE_SetAnalysisType();

        if (!pTabFeno->hidden)
         {
          lambdaMin=min(lambdaMin,pTabFeno->LambdaRef[0]);
          lambdaMax=max(lambdaMax,pTabFeno->LambdaRef[NDET-1]);
         }
       }

      #if defined(__DEBUG_) && __DEBUG_
      DEBUG_Print("Cross sections of %s window (rc %d)\n",pTabFeno->windowName,rc);
      DEBUG_Print("NAME                      ACTION           A  P    ------------ conc --------------     ----------- param --------------     ----------- shift --------------     --------- stretch --------------     -------- stretch2 --------------\n");
      for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
       {
       	pEngineCross=&pTabFeno->TabCross[indexTabCross];
       	pWrkSymbol=&WorkSpace[pEngineCross->Comp];

       	DEBUG_Print("%-25s %-15s %2d %2d     %2d %14.3e %14.3e     %2d %14.3e %14.3e     %2d %14.3e %14.3e     %2d %14.3e %14.3e     %2d %14.3e %14.3e\n",
           	        pWrkSymbol->symbolName,ANLYS_crossAction[pEngineCross->crossAction],
           	        pEngineCross->IndSvdA,pEngineCross->IndSvdP,
                    pEngineCross->FitConc,pEngineCross->InitConc,pEngineCross->DeltaConc,
                    pEngineCross->FitParam,pEngineCross->InitParam,pEngineCross->DeltaParam,
                    pEngineCross->FitShift,pEngineCross->InitShift,pEngineCross->DeltaShift,
                    pEngineCross->FitStretch,pEngineCross->InitStretch,pEngineCross->DeltaStretch,
                    pEngineCross->FitStretch2,pEngineCross->InitStretch2,pEngineCross->DeltaStretch2);


        DEBUG_PrintVar("Cross section",pWrkSymbol->xs.matrix,0,pWrkSymbol->xs.nl-1,0,pWrkSymbol->xs.nc-1,NULL);
       }
      #endif

      NFeno++;
     }
	  }

  if (lambdaMin>=lambdaMax)
   {
    lambdaMin=pEngineContext->buffers.lambda[0];
    lambdaMax=pEngineContext->buffers.lambda[NDET-1];
   }
// QDOAS ???
// QDOAS ???     if (!rc && !(rc=AnalyseLoadSlit(pSlitOptions)) && (!pKuruczOptions->fwhmFit || !useKurucz))
// QDOAS ???      for (indexWindow=0;indexWindow<NFeno;indexWindow++)
// QDOAS ???       {
// QDOAS ???        pTabFeno=&TabFeno[indexWindow];
// QDOAS ???
// QDOAS ???        if (pTabFeno->xsToConvolute && /* pTabFeno->useEtalon && */ (pTabFeno->gomeRefFlag || MFC_refFlag) &&
// QDOAS ???          ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,&ANALYSIS_slit,pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2,&pSlitOptions->slitFunction.slitParam3,&pSlitOptions->slitFunction.slitParam4))!=0))
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???       }
// QDOAS ???

  if (!rc && ((THRD_id==THREAD_TYPE_KURUCZ) || useKurucz) &&
     !(rc=KURUCZ_Alloc(&pEngineContext->project,pEngineContext->buffers.lambda,indexKurucz,lambdaMin,lambdaMax)) &&
     !(rc=KURUCZ_Reference(pEngineContext->buffers.instrFunction,0,saveFlag,1)))

   rc=ANALYSE_AlignReference(0,saveFlag,responseHandle);

// QDOAS ???  if (!rc && useUsamp &&
// QDOAS ???      !(rc=USAMP_GlobalAlloc(lambdaMin,lambdaMax,NDET)) &&
// QDOAS ???      !(rc=USAMP_LocalAlloc(1)))
// QDOAS ???   rc=USAMP_BuildFromAnalysis(0,1);
// QDOAS ???
// QDOAS ??? // QDOAS ???     {
// QDOAS ??? // QDOAS ???      FILE *fp;
// QDOAS ??? // QDOAS ???
// QDOAS ??? // QDOAS ???      if ((fp=fopen(DOAS_broAmfFile,"rt"))!=NULL)
// QDOAS ??? // QDOAS ???       {
// QDOAS ??? // QDOAS ???        fclose(fp);
// QDOAS ??? // QDOAS ???        MATRIX_Load(DOAS_broAmfFile,&ANALYSIS_broAmf,0,0,0,0,-9999.,9999.,1,0,"ANALYSE_LoadData ");
// QDOAS ??? // QDOAS ???       }
// QDOAS ??? // QDOAS ???     }

	 if (rc!=ERROR_ID_NO)
	  mediateDisplayErrorMessage(responseHandle);

	 #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Stop("Analysis windows");
  #endif

  return 0;
 }

// ===============================================================
// TRANSFER OF THE LIST OF SYMBOLS FROM THE MEDIATOR TO THE ENGINE
// ===============================================================

int mediateRequestSetSymbols(void *engineContext,
			     int numberOfSymbols,
			     const mediate_symbol_t *symbols,
			     void *responseHandle)
 {
 	// Declarations

 	int indexSymbol;
 	RC rc;

 	// Initialization

 	rc=ERROR_ID_NO;

 	// Add symbols in the list

	 for (indexSymbol=0;(indexSymbol<numberOfSymbols) && !rc;indexSymbol++)
	  rc=SYMB_Add((UCHAR *)symbols[indexSymbol].name,(UCHAR *)symbols[indexSymbol].description);

 	// Check for error

 	if (rc)
 	 mediateDisplayErrorMessage(responseHandle);

 	// Return

  return rc;
 }

// =========================================================================
// TRANSFER OF THE LIST OF OBSERVATION SITES FROM THE MEDIATOR TO THE ENGINE
// =========================================================================

int mediateRequestSetSites(void *engineContext,
			   int numberOfSites,
			   const mediate_site_t *sites,
			   void *responseHandle)
{
  return 0;
}

// ==================
// BROWSING FUNCTIONS
// ==================

int mediateRequestBeginBrowseSpectra(void *engineContext,
				     const char *spectraFileName,
				     void *responseHandle)
 {
 	if (EngineRequestBeginBrowseSpectra((ENGINE_CONTEXT *)engineContext,spectraFileName,THREAD_TYPE_SPECTRA)!=0)
 	 mediateDisplayErrorMessage(responseHandle);

  return ((ENGINE_CONTEXT *)engineContext)->recordNumber;
 }

int mediateRequestGotoSpectrum(void *engineContext,
			       int recordNumber,
			       void *responseHandle)
 {
  ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;

  if (recordNumber > 0 && recordNumber <= pEngineContext->recordNumber) {
    pEngineContext->indexRecord = recordNumber;
    return recordNumber;
  }
  else {
    pEngineContext->indexRecord = pEngineContext->recordNumber + 1;
    return 0;
  }
 }

int mediateRequestNextMatchingSpectrum(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
 {
	 // Declarations

  PROJECT *pProject;                                                            // pointer to the project part of the engine context
  PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  SHORT_DATE  *pDay;                                                            // pointer to measurement date
  struct time *pTime;                                                           // pointer to measurement date
  char tmpString[80];                                                           // buffer for formatted strings
  int oldRec=pEngineContext->indexRecord;
  int rec = (pEngineContext->indexRecord)++;
  int indexLine,indexColumn;
  UCHAR *fileName;                                                              // the name of the current file
  plot_data_t spectrumData;
  RC rc;

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;
  pProject=&pEngineContext->project;
  pInstrumental=&pProject->instrumental;
  pDay=&pRecord->present_day;
  pTime=&pRecord->present_time;

  fileName=pEngineContext->fileInfo.fileName;

  indexLine=1;
  indexColumn=2;

  if (rec > pEngineContext->recordNumber) {
    return 0;
  }
  else {
    //-----------------
    // TEMPORARY COMMENT
    //-----------------
    // Caroline, I dont like the mediateAllocateAndSetPlotData stuff we added just before you left.
    // It works, but we are copying the data too many times. I think it would be better if you
    // adapt the system so that it justs allocates the data arrays and your engine code can
    // write directly into the allocated double* arrays. Right now, you read data, it gets copied
    // into the plot_data_t by the mediateAllocateAndSetPlotData function, and the mediateResponsePlotData
    // copies that into the structures managed by the responseHandle... we should eliminate one
    // of the copies.

//    do
//     {
//      if ((rc=EngineReadFile(pEngineContext,rec,0,0))!=ERROR_ID_NO)
//      	mediateDisplayErrorMessage(responseHandle);
//      rec++;
//     }
//    while ((rec<=pEngineContext->recordNumber) &&
//          ((fabs(pProject->spectra.SZAMin-pProject->spectra.SZAMax)>(double)1.e-4) ||
//          ((pRecord->Zm>=pProject->spectra.SZAMin) && (pRecord->Zm<=pProject->spectra.SZAMax))));

    rc=EngineReadFile(pEngineContext,rec,0,0);

    if (!rc && (rec<=pEngineContext->recordNumber) ) //&& (pRecord->Zm>=pProject->spectra.SZAMin) && (pRecord->Zm<=pProject->spectra.SZAMax))
     {
      sprintf(tmpString,"Spectrum (%d/%d)",pEngineContext->indexRecord-1,pEngineContext->recordNumber);

      if (pProject->spectra.displaySpectraFlag)
       {
        mediateAllocateAndSetPlotData(&spectrumData, pBuffers->lambda, pBuffers->spectrum, NDET, Line);
        mediateResponsePlotData(plotPageSpectrum, &spectrumData, 1, Spectrum, allowFixedScale, "Spectrum", "Wavelength (nm)", "Counts", responseHandle);
        mediateReleasePlotData(&spectrumData);
        mediateResponseLabelPage(plotPageSpectrum, fileName, tmpString, responseHandle);

        if (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
             (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_ACTON) ||
             (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE)) &&
             (pBuffers->darkCurrent!=NULL))
         {
          mediateAllocateAndSetPlotData(&spectrumData, pBuffers->lambda, pBuffers->darkCurrent, NDET, Line);
          mediateResponsePlotData(plotPageDarkCurrent, &spectrumData, 1, Spectrum, forceAutoScale, "Dark current", "Wavelength (nm)", "Counts", responseHandle);
          mediateReleasePlotData(&spectrumData);
          mediateResponseLabelPage(plotPageDarkCurrent, fileName, "Dark current", responseHandle);
         }

        if (pBuffers->sigmaSpec!=NULL)
         {
         	sprintf(tmpString,"Error (%d/%d)",pEngineContext->indexRecord-1,pEngineContext->recordNumber);

          mediateAllocateAndSetPlotData(&spectrumData, pBuffers->lambda, pBuffers->sigmaSpec, NDET, Line);
          mediateResponsePlotData(plotPageErrors, &spectrumData, 1, Spectrum, forceAutoScale, "Error", "Wavelength (nm)", "Counts", responseHandle);
          mediateReleasePlotData(&spectrumData);
          mediateResponseLabelPage(plotPageErrors, fileName, tmpString, responseHandle);
         }

        if (pBuffers->irrad!=NULL)
         {
          mediateAllocateAndSetPlotData(&spectrumData, pBuffers->lambda, pBuffers->irrad, NDET, Line);
          mediateResponsePlotData(plotPageIrrad, &spectrumData, 1, Spectrum, forceAutoScale, "Irradiance spectrum", "Wavelength (nm)", "Counts", responseHandle);
          mediateReleasePlotData(&spectrumData);
          mediateResponseLabelPage(plotPageIrrad, fileName, "Irradiance", responseHandle);
         }

        if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) &&
            (pBuffers->specMax!=NULL) &&
            (pRecord->NSomme>1))
         {
          mediateAllocateAndSetPlotData(&spectrumData, pBuffers->specMaxx, pBuffers->specMax,pRecord->rejected+pRecord->NSomme, Line);
          mediateResponsePlotData(plotPageSpecMax, &spectrumData, 1, SpecMax, allowFixedScale, "SpecMax", "Scans number", "Signal Maximum", responseHandle);
          mediateReleasePlotData(&spectrumData);
          mediateResponseLabelPage(plotPageSpecMax, fileName, "SpecMax", responseHandle);
         }
       }

      if (pProject->spectra.displayDataFlag)
       {
        mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"File","%s",

  //   QDOAS ???
  //   QDOAS ???     fprintf(fp,"Project\t\t\t%s\n",pProject->name);
  //   QDOAS ???
  //   QDOAS ???     fprintf(fp,"File\t\t\t%s\n",
  //   QDOAS ???           ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)||
  //   QDOAS ???            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)||
  //   QDOAS ???            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OPUS))?PATH_fileSpectra:
                      fileName);

        if (strlen(pInstrumental->instrFunction))
         {
          if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_ULB)
           mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Dark Current","%s",pInstrumental->instrFunction);
          else if (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC)
           mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Instr. function","%s",pInstrumental->instrFunction);
          else if (((pInstrumental->mfcMaskSpec!=0) && ((UINT)MFC_header.ty==pInstrumental->mfcMaskSpec)) ||
                   ((pInstrumental->mfcMaskSpec==0) &&
                   ((MFC_header.wavelength1==pInstrumental->mfcMaskInstr) ||
                    (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.))))
           mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Instr. function","%s",pInstrumental->instrFunction);
         }

  //   QDOAS ???
  //   QDOAS ???     if (strlen(pInstrumental->vipFile))
  //   QDOAS ???      {
  //   QDOAS ???       if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)
  //   QDOAS ???        fprintf(fp,"Dark current\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileDark,1));
  //   QDOAS ???       else if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) &&
  //   QDOAS ???             (((pInstrumental->mfcMaskSpec!=0) && (((UINT)MFC_header.ty==pInstrumental->mfcMaskSpec) || ((UINT)MFC_header.ty==pInstrumental->mfcMaskInstr))) ||
  //   QDOAS ???               ((pInstrumental->mfcMaskSpec==0) &&
  //   QDOAS ???               ((MFC_header.wavelength1==pInstrumental->mfcMaskInstr) ||
  //   QDOAS ???                (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.)))))
  //   QDOAS ???        fprintf(fp,"Dark current\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileDark,1));
  //   QDOAS ???       else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)
  //   QDOAS ???        fprintf(fp,"Straylight correction\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->vipFile,1));
  //   QDOAS ???       else
  //   QDOAS ???        fprintf(fp,"Interpixel\t\t\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->vipFile,1));
  //   QDOAS ???      }
  //   QDOAS ???
  //   QDOAS ???     if (strlen(pInstrumental->dnlFile))
  //   QDOAS ???      {
  //   QDOAS ???       if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)
  //   QDOAS ???        fprintf(fp,"Offset\t\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileOffset,1));
  //   QDOAS ???       else if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) &&
  //   QDOAS ???              (((MFC_header.ty!=0) &&
  //   QDOAS ???              (((UINT)MFC_header.ty==pInstrumental->mfcMaskSpec) ||
  //   QDOAS ???               ((UINT)MFC_header.ty==pInstrumental->mfcMaskDark) ||
  //   QDOAS ???               ((UINT)MFC_header.ty==pInstrumental->mfcMaskInstr))) ||
  //   QDOAS ???               ((pInstrumental->mfcMaskSpec==0) &&
  //   QDOAS ???               ((MFC_header.wavelength1==pInstrumental->mfcMaskDark) ||
  //   QDOAS ???                (MFC_header.wavelength1==pInstrumental->mfcMaskInstr)||
  //   QDOAS ???                (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.)))))
  //   QDOAS ???        fprintf(fp,"Offset\t\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileOffset,1));
  //   QDOAS ???       else
  //   QDOAS ???        fprintf(fp,"Non linearity\t\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->dnlFile,1));
  //   QDOAS ???      }
  //   QDOAS ???
         if (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GOME2)
          mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Date and Time","%02d/%02d/%d %02d:%02d:%02d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
  //   QDOAS ???     else
  //   QDOAS ???      mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Date and Time","%02d/%02d/%d %02d:%02d:%02d.%06d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec,GOME2_ms);

   //      sprintf(tmpString,"%.3f -> %.3f \n",pRecord->TimeDec,pRecord->localTimeDec);

         if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
             (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD))
          {
           pTime=&pRecord->startTime;
           mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Start time","%02d:%02d:%02d",pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
           pTime=&pRecord->endTime;
           mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"End time","%02d:%02d:%02d",pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
          }

         if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
          mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Record","%d/%d",pEngineContext->indexRecord-1,pEngineContext->recordNumber);
         else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI)
          {
          	if (pInstrumental->averageFlag)
            mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Record","%d/%d (%d spectra averaged)",
                    pEngineContext->indexRecord-1,pEngineContext->recordNumber,pRecord->omi.omiNumberOfSpectraPerTrack);
          	else
            mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Record","%d/%d (track %d/%d, spectrum %d/%d)",
                    pEngineContext->indexRecord-1,pEngineContext->recordNumber,
                    pRecord->omi.omiTrackIndex,pRecord->omi.omiNumberOfTracks,
                    pRecord->omi.omiTrackIndex,pRecord->omi.omiNumberOfSpectraPerTrack);
          }
         else
          mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Record","%d/%d",pEngineContext->indexRecord-1,pEngineContext->recordNumber);

         if (strlen(pRecord->Nom))
          mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Record name","%s",pRecord->Nom);

  //   QDOAS ???
  //   QDOAS ???     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
  //   QDOAS ???      {
  //   QDOAS ???       if (strlen(MFC_header.FileName))
  //   QDOAS ???        sprintf(tmpString,"Spectrum\t\t\t%s\n",MFC_header.FileName);
  //   QDOAS ???       if (strlen(MFC_header.specname))
  //   QDOAS ???        sprintf(tmpString,"Record name\t\t%s\n",MFC_header.specname);
  //   QDOAS ???       if (strlen(MFC_header.site))
  //   QDOAS ???        sprintf(tmpString,"Site\t\t\t%s\n",MFC_header.site);
  //   QDOAS ???       if (strlen(MFC_header.spectroname))
  //   QDOAS ???        sprintf(tmpString,"Spectro name\t\t%s\n",MFC_header.spectroname);
  //   QDOAS ???       if (strlen(MFC_header.scan_dev))
  //   QDOAS ???        sprintf(tmpString,"Scan device\t\t%s\n",MFC_header.scan_dev);
  //   QDOAS ???       if (strlen(MFC_header.first_line))
  //   QDOAS ???        sprintf(tmpString,"%s\n",MFC_header.first_line);
  //   QDOAS ???       if (strlen(MFC_header.spaeter))
  //   QDOAS ???        sprintf(tmpString,"%s\n",MFC_header.spaeter);
  //   QDOAS ???
  //   QDOAS ???       if (strlen(MFC_header.backgrnd))
  //   QDOAS ???        sprintf(tmpString,"Background\t\t%s\n",MFC_header.backgrnd);
  //   QDOAS ???
  //   QDOAS ???       sprintf(tmpString,"ty mask\t\t\t%d\n",MFC_header.ty);
  //   QDOAS ???      }

         if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
             (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))
          {
          	mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Orbit number","%d",pRecord->gome.orbitNumber+1);
          	mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Pixel number","%d",pRecord->gome.pixelNumber);
          	mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Pixel type","%d",pRecord->gome.pixelType);
          }
         else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
          mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Orbit number","%d",pRecord->scia.orbitNumber);

  //   QDOAS ???     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
  //   QDOAS ???      sprintf(tmpString,"Calibration parameters\t%.2f %.3e %.3e %.3e\n",pRecord->wavelength1,pRecord->dispersion[0],
  //   QDOAS ???                  pRecord->dispersion[1],pRecord->dispersion[2]);
  //   QDOAS ???
         mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Solar Zenith angle","%-.3f �",pRecord->Zm);
  //   QDOAS ???
         if ((pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GDP_ASCII) &&
             (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GDP_BIN) &&
             (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS) &&
             (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_OMI) &&
             (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GOME2))
          {
           mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Exposure time","%.3f sec",pRecord->Tint);
           mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Scans taken into account","%d",pRecord->NSomme);
           mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Elevation viewing angle","%.3f �",pRecord->elevationViewAngle);
           mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Azimuth viewing angle","%.3f �",pRecord->azimuthViewAngle);
          }
         else
          {
          	mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Solar Azimuth angle","%.3f �",pRecord->Azimuth);
          	mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Viewing Zenith angle","%.3f �",pRecord->zenithViewAngle);
          	mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Viewing Azimuth angle","%.3f �",pRecord->azimuthViewAngle);
          }

         if (pRecord->rejected>0)
          mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Rejected scans","%d",pRecord->rejected);
         if (pRecord->TDet!=(double)0.)
          mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Detector temperature","%.3f �",pRecord->TDet);

        if (pRecord->longitude!=(double)0.)
         mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Longitude","%.3f",pRecord->longitude);
        if (pRecord->latitude!=(double)0.)
         mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Latitude","%.3f",pRecord->latitude);
        if (pRecord->altitude!=(double)0.)
         mediateResponseCellInfo(plotPageSpectrum,indexLine++,indexColumn,responseHandle,"Altitude","%.3f",pRecord->altitude);

  //   QDOAS ???
  //   QDOAS ???     if (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
  //   QDOAS ???          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)) &&
  //   QDOAS ???         ((pRecord->aMoon!=(double)0.) || (pRecord->hMoon!=(double)0.) || (pRecord->fracMoon!=(double)0.)))
  //   QDOAS ???      {
  //   QDOAS ???       sprintf(tmpString,"Moon azimuthal angle\t%.3f �\n",pRecord->aMoon);
  //   QDOAS ???       sprintf(tmpString,"Moon elevation\t\t%.3f �\n",pRecord->hMoon);
  //   QDOAS ???       sprintf(tmpString,"Moon illuminated fraction\t%.3f �\n",pRecord->fracMoon);
  //   QDOAS ???      }
  //   QDOAS ???     else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
  //   QDOAS ???      {
  //   QDOAS ???       GOME_ORBIT_FILE *pOrbitFile;
  //   QDOAS ???       pOrbitFile=&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];
  //   QDOAS ???
  //   QDOAS ???       sprintf(tmpString,"Cloud fraction\t\t%.2f\n",(float)pOrbitFile->gdpBinSpectrum.cloudFraction*0.01);
  //   QDOAS ???       sprintf(tmpString,"Cloud top pressure\t\t%.2f\n",(float)pOrbitFile->gdpBinSpectrum.cloudTopPressure*0.01);
  //   QDOAS ???       sprintf(tmpString,"[O3 VCD]\t\t\t%.2f DU\n",(float)pOrbitFile->gdpBinSpectrum.o3*0.01);
  //   QDOAS ???       sprintf(tmpString,"[No2 VCD]\t\t%.2e mol/cm2\n",(float)pOrbitFile->gdpBinSpectrum.no2*1.e13);
  //   QDOAS ???      }
  //   QDOAS ???
  //   QDOAS ???     sprintf(tmpString,"\n\n");
  //   QDOAS ???     fclose(fp);
  //   QDOAS ???    }
       }
     }

    else
     rec=oldRec;
   }

  return rec;
 }

int mediateRequestNextMatchingBrowseSpectrum(void *engineContext,
					     void *responseHandle)
 {
  return mediateRequestNextMatchingSpectrum((ENGINE_CONTEXT *)engineContext,responseHandle);
 }

int mediateRequestEndBrowseSpectra(void *engineContext,
				   void *responseHandle)
 {
  // Close open files and release allocated buffers to reset the engine context

  EngineResetContext((ENGINE_CONTEXT *)engineContext,1);

  // Return

  return 0;
 }

int mediateRequestBeginAnalyseSpectra(void *engineContext,
				      const char *spectraFileName,
				      void *responseHandle)
 {
 	ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;

  if (EngineRequestBeginBrowseSpectra(pEngineContext,spectraFileName,THREAD_TYPE_ANALYSIS)!=ERROR_ID_NO)
   mediateDisplayErrorMessage(responseHandle);

  return ((ENGINE_CONTEXT *)engineContext)->recordNumber;
 }

int mediateRequestNextMatchingAnalyseSpectrum(void *engineContext,
					      void *responseHandle)
 {
 	// Declarations

  ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
 	INT indexNextRecord;

 	if (((indexNextRecord=mediateRequestNextMatchingSpectrum(pEngineContext,responseHandle))!=0) &&
	     ((ANALYSE_refSelectionFlag && !pEngineContext->satelliteFlag && (EngineNewRef(pEngineContext,responseHandle)!=ERROR_ID_NO)) ||
       (ANALYSE_Spectrum(pEngineContext,responseHandle)!=ERROR_ID_NO)))

   mediateDisplayErrorMessage(responseHandle);

  return indexNextRecord;
 }

int mediateRequestPrevMatchingAnalyseSpectrum(void *engineContext,
					      void *responseHandle)
 {
  return 0;
 }

int mediateRequestEndAnalyseSpectra(void *engineContext,
				    void *responseHandle)
 {
 	// Release all the buffers allocated for the analysis

  if (ANALYSE_refSelectionFlag)
   EngineResetContext((ENGINE_CONTEXT *)&ENGINE_contextRef,0);

  // Close open files and release allocated buffers to reset the engine context

  EngineResetContext((ENGINE_CONTEXT *)engineContext,1);

  // Return

  return 0;
 }


int mediateRequestBeginCalibrateSpectra(void *engineContext,
					const char *spectraFileName,
					void *responseHandle)
 {
 	if (EngineRequestBeginBrowseSpectra((ENGINE_CONTEXT *)engineContext,spectraFileName,THREAD_TYPE_KURUCZ)!=0)
 	 mediateDisplayErrorMessage(responseHandle);

 	return ((ENGINE_CONTEXT *)engineContext)->recordNumber;
 }

int mediateRequestNextMatchingCalibrateSpectrum(void *engineContext,
						void *responseHandle)
 {
  return mediateRequestNextMatchingSpectrum((ENGINE_CONTEXT *)engineContext,responseHandle);
 }

int mediateRequestPrevMatchingCalibrateSpectrum(void *engineContext,
						void *responseHandle)
 {
  return 0;
 }

int mediateRequestEndCalibrateSpectra(void *engineContext,
				      void *responseHandle)
 {
  // Close open files and release allocated buffers to reset the engine context

  EngineResetContext((ENGINE_CONTEXT *)engineContext,1);

  // Return

  return 0;
 }

#include <stdio.h>

int mediateRequestViewCrossSections(void *engineContext, double minWavelength, double maxWavelength,
                                    int nFiles, char **filenames, void *responseHandle)
{

  // Extract spectral (plot only) data and pass it back via the response handle.
  // This should be for ALL analysis windows that have been set, but in practice,
  // there will only be one...

  // TODO - temp for testing
  int i;
  plot_data_t dummy;
  double xData[] = { 0.0, 1.1, 2.2, 3.3, 4.4, 5.5 };
  double yData[] = { 3.0, 4.0, 3.5, 1.1, 1.4, 3.0 };

  mediateAllocateAndSetPlotData(&dummy, xData, yData, 6, Line);
  mediateResponsePlotData(0, &dummy, 1, Spectrum, 0, "First XS", "X-Label", "Y-label", responseHandle);
  mediateReleasePlotData(&dummy);

  for (i=0; i<6; ++i)
    yData[i] = yData[i] * yData[i];

  mediateAllocateAndSetPlotData(&dummy, xData, yData, 6, Line);
  mediateResponsePlotData(0, &dummy, 1, Spectrum, 0, "Second XS", "X-Label", "Y-label", responseHandle);
  mediateReleasePlotData(&dummy);

  for (i=0; i<nFiles; ++i) {
    printf("XS File '%s'\n", filenames[i]);
  }

  return 0;
}

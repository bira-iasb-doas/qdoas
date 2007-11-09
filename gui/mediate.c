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
// FUNCTION      setMediateProjectAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Analysis part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectAnalysis(PRJCT_ANLYS *pEngineAnalysis,const mediate_project_analysis_t *pMediateAnalysis)
 {
 	#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_

  UCHAR *prjctAnlysMethods[PRJCT_ANLYS_METHOD_MAX]={"Optical density fitting","Intensity fitting (Marquardt-Levenberg+SVD)"};
  UCHAR *prjctAnlysFitWeighting[PRJCT_ANLYS_FIT_WEIGHTING_MAX]={"No weighting","Instrumental weighting"};
  UCHAR *prjctAnlysUnits[PRJCT_ANLYS_UNITS_MAX]={"Pixels","Nanometers"};
  UCHAR *prjctAnlysInterpol[PRJCT_ANLYS_INTERPOL_MAX]={"linear","spline"};

  DEBUG_FunctionBegin("setMediateProjectAnalysis",DEBUG_FCTTYPE_CONFIG);
  #endif

  pEngineAnalysis->method=pMediateAnalysis->methodType;                         // analysis method
  pEngineAnalysis->fitWeighting=pMediateAnalysis->fitType;                      // least-squares fit weighting
  pEngineAnalysis->units=pMediateAnalysis->unitType;                            // units for shift and stretch
  pEngineAnalysis->interpol=pMediateAnalysis->interpolationType;                // interpolation
  pEngineAnalysis->convergence=pMediateAnalysis->convergenceCriterion;          // convergence criterion
  pEngineAnalysis->securityGap=pMediateAnalysis->interpolationSecurityGap;      // security pixels to take in order to avoid interpolation problems at the edge of the spectral window

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_Print("Analysis method : %s\n",prjctAnlysMethods[pEngineAnalysis->method]);
  DEBUG_Print("Least-squares fit weighting : %s\n",prjctAnlysFitWeighting[pEngineAnalysis->fitWeighting]);
  DEBUG_Print("Units for shift and stretch : %s\n",prjctAnlysUnits[pEngineAnalysis->units]);
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

void setMediateProjectFiltering(PRJCT_FILTER *pEngineFilter,const mediate_project_filtering_t *pMediateFilter)
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

void setMediateProjectCalibration(PRJCT_KURUCZ *pEngineCalibration,const mediate_project_calibration_t *pMediateCalibration)
 {
 	#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("setMediateProjectCalibration",DEBUG_FCTTYPE_CONFIG);
  #endif

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
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
     pEngineInstrumental->ascii.lembdaSaveFlag=pMediateInstrumental->ascii.flagWavelength;          // 1 if the wavelength calibration is saved with spectra in the file

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
// FUNCTION      setMediateProjectNasaAmes
// -----------------------------------------------------------------------------
// PURPOSE       Output (Nasa-ames format) part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectNasaAmes(PRJCT_RESULTS_NASA *pEngineNasaAmes,const mediate_project_nasa_ames_t *pMediateNasaAmes)
 {
 	#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("setMediateProjectNasaAmes",DEBUG_FCTTYPE_CONFIG);
  #endif

  // Still to do by Caro

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionStop("setMediateProjectNasaAmes",0);
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
  DEBUG_Start(DOAS_dbgFile,"Project",DEBUG_FCTTYPE_CONFIG,5,DEBUG_DVAR_YES,0);
  #endif

	 setMediateProjectSpectra(&pEngineProject->spectra,&project->spectra);
  setMediateProjectAnalysis(&pEngineProject->analysis,&project->analysis);
  setMediateProjectFiltering(&pEngineProject->lfilter,&project->lowpass);
  setMediateProjectFiltering(&pEngineProject->hfilter,&project->highpass);
  setMediateProjectCalibration(&pEngineProject->kurucz,&project->calibration);
  setMediateProjectUndersampling(&pEngineProject->usamp,&project->undersampling);
  setMediateProjectInstrumental(&pEngineProject->instrumental,&project->instrumental);
  setMediateProjectSlit(&pEngineProject->slit,&project->slit);
  setMediateProjectOutput(&pEngineProject->asciiResults,&project->output);
  setMediateProjectNasaAmes(&pEngineProject->nasaResults,&project->nasaames);

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
// FUNCTION      mediateRequestSetAnalysisCross
// -----------------------------------------------------------------------------
// PURPOSE       Load data from the molecules pages
// -----------------------------------------------------------------------------

RC mediateRequestSetAnalysisCross(cross_section_list_t *pCrossSectionList,INT hidden,double *lembda)
 {
 	// Declarations

 	CROSS_REFERENCE *pEngineCross;                                                // pointer of the current cross section in the engine list
 	struct anlyswin_cross_section *pMediateCross;                                 // pointer of the current cross section in the mediate list
 	FENO *pTabFeno;                                                               // pointer to the current analysis window
  UCHAR *pOrthoSymbol[MAX_FIT],                                                 // for each cross section in list, hold cross section to use for orthogonalization
        *symbolName;
  INDEX indexSymbol,indexSvd,                                                   // resp. indexes of item in list and of symbol
        firstTabCross,endTabCross,indexTabCross,i;                              // indexes for browsing list of cross sections symbols
  SZ_LEN fileLength,symbolLength;                                               // length in characters of file name and symbol name
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a general description of a symbol
 	RC rc;

 	// Debug

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("mediateRequestSetAnalysisCross",DEBUG_FCTTYPE_CONFIG);
  #endif

 	// Initializations

  pWrkSymbol=NULL;
  pTabFeno=&TabFeno[NFeno];
  firstTabCross=pTabFeno->NTabCross;
 	rc=ERROR_ID_NO;

 	for (indexTabCross=0;
 	    (indexTabCross<pCrossSectionList->nCrossSection) && (pTabFeno->NTabCross<MAX_FIT) && !rc;
 	     indexTabCross++)
 	 {
 	 	pEngineCross=&pTabFeno->TabCross[pTabFeno->NTabCross];
 	 	pMediateCross=&pCrossSectionList->crossSection[indexTabCross];

    // Get cross section name from analysis properties dialog box

    symbolName=pMediateCross->symbol;
    symbolLength=strlen(symbolName);
    fileLength=strlen(pMediateCross->crossSectionFile);

    // Search for symbol in list

    for (indexSymbol=0;indexSymbol<NWorkSpace;indexSymbol++)
     {
      pWrkSymbol=&WorkSpace[indexSymbol];

      if ((pWrkSymbol->type==WRK_SYMBOL_CROSS) &&
          (strlen(pWrkSymbol->symbolName)==symbolLength) &&
          (strlen(pWrkSymbol->crossFileName)==fileLength) &&
          !STD_Stricmp(pWrkSymbol->symbolName,symbolName) &&
          !STD_Stricmp(pWrkSymbol->crossFileName,pMediateCross->crossSectionFile))

       break;
     }

    // Add a new cross section

    if ((indexSymbol==NWorkSpace) && (NWorkSpace<MAX_SYMB))
     {
      // Allocate a new symbol

      pWrkSymbol=&WorkSpace[indexSymbol];

      pWrkSymbol->type=WRK_SYMBOL_CROSS;
      strcpy(pWrkSymbol->symbolName,symbolName);
      strcpy(pWrkSymbol->crossFileName,pMediateCross->crossSectionFile);

      // Load cross section from file

      if (((strlen(pWrkSymbol->symbolName)==strlen("1/Ref")) && !STD_Stricmp(pWrkSymbol->symbolName,"1/Ref")) ||
          !(rc=MATRIX_Load(pMediateCross->crossSectionFile,&pWrkSymbol->xs,0 /* line base */,0 /* column base */,0,0,
                          (pMediateCross->crossType==ANLYS_CROSS_ACTION_NOTHING)?(double)0.:lembda[0]-7.,      // max(lembda[0]-7.,(double)290.), - changed on october 2006
                          (pMediateCross->crossType==ANLYS_CROSS_ACTION_NOTHING)?(double)0.:lembda[NDET-1]+7., // min(lembda[NDET-1]+7.,(double)600.), - changed on october 2006
                          (pMediateCross->crossType!=ANLYS_CROSS_ACTION_NOTHING)?1:0,1,"mediateRequestSetAnalysisCross ")))
       {
        if (!STD_Stricmp(pWrkSymbol->symbolName,"O3TD"))
         rc=MATRIX_Allocate(&O3TD,NDET,pWrkSymbol->xs.nc,0,0,0,"ANALYSE_LoadCross");

        NWorkSpace++;
       }
     }

    if ((rc==ERROR_ID_NO) && (indexSymbol<NWorkSpace) && (pTabFeno->NTabCross<MAX_FIT))
     {
      // Allocate vectors for cross section and its second derivative for analysis processing

      if (((pEngineCross->vector=(double *)MEMORY_AllocDVector("mediateRequestSetAnalysisCross ","vector",0,NDET-1))==NULL) ||
          ((pEngineCross->Deriv2=(double *)MEMORY_AllocDVector("mediateRequestSetAnalysisCross ","Deriv2",1,NDET))==NULL))

       rc=ERROR_ID_ALLOC;

      else
       {
        pEngineCross->crossAction=pMediateCross->crossType;
        pEngineCross->filterFlag=pMediateCross->requireFilter;

        if ((pEngineCross->crossAction==ANLYS_CROSS_ACTION_NOTHING) && (pTabFeno->gomeRefFlag || MFC_refFlag))
         rc=ANALYSE_CheckLembda(pWrkSymbol,lembda,"mediateRequestSetAnalysisCross ");

        if (rc==ERROR_ID_NO)
         {
          pEngineCross->Comp=indexSymbol;
          pEngineCross->IndSvdA=++pTabFeno->svd.DimC;
          pTabFeno->xsToConvolute+=((pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE) ||
                                    (pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0) ||
                                    (pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_RING))?1:0;

          pTabFeno->xsToConvoluteI0+=(pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0)?1:0;

          if (!hidden)
           {
            pOrthoSymbol[pTabFeno->NTabCross]=pMediateCross->orthogonal;

            pEngineCross->display=pMediateCross->requireFit;                    // fit display
            pEngineCross->InitConc=pMediateCross->initialCc;                    // initial concentration
            pEngineCross->FitConc=pMediateCross->requireCcFit;                  // modify concentration

            pEngineCross->DeltaConc=(pEngineCross->FitConc)?pMediateCross->deltaCc:(double)0.;   // delta on concentration
            pEngineCross->I0Conc=(pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0)?pMediateCross->requireCcIo:(double)0.; // QDOAS !!! requireCcIo should be a double
           }

 // QDOAS !!! FOR LATER         rc=OUTPUT_LoadCross(pList,&pTabFeno->TabCrossResults[pTabFeno->NTabCross],&pTabFeno->amfFlag,hidden);

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

      if ((symbolLength==4) && !STD_Stricmp(pOrthoSymbol[indexTabCross],"None"))
       pEngineCross[indexTabCross].IndOrthog=ITEM_NONE;

      // Orthogonalization to orthogonal base

      else if ((symbolLength==15) && !STD_Stricmp(pOrthoSymbol[indexTabCross],"Differential XS"))
       pEngineCross[indexTabCross].IndOrthog=ORTHOGONAL_BASE;

      // Orthogonalization to another cross section

      else
       {
        // Search for symbol in list

        for (indexSymbol=firstTabCross;indexSymbol<endTabCross;indexSymbol++)
         if ((indexTabCross!=indexSymbol) &&
             (symbolLength==strlen(WorkSpace[pEngineCross[indexSymbol].Comp].symbolName)) &&
             !STD_Stricmp(pOrthoSymbol[indexTabCross],WorkSpace[pEngineCross[indexSymbol].Comp].symbolName))
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
           	rc=ERROR_SetLast("mediateRequestSetAnalysisCross",ERROR_TYPE_WARNING,ERROR_ID_ORTHOGONAL_BASE,
                              WorkSpace[pEngineCross[pEngineCross[indexTabCross].IndOrthog].Comp].symbolName,
                              WorkSpace[pEngineCross[indexTabCross].Comp].symbolName);

            pEngineCross[pEngineCross[indexTabCross].IndOrthog].IndOrthog=ORTHOGONAL_BASE;
           }
         }
       }
     }
   }

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionStop("mediateRequestSetAnalysisCross",rc);
  #endif

  // Return

  return rc;
 }

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

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestSetAnalysisLinear
// -----------------------------------------------------------------------------
// PURPOSE       Load linear parameters
// -----------------------------------------------------------------------------

#define NNONLINEAR 8

RC mediateRequestSetAnalysisNonLinear(struct anlyswin_nonlinear *pNonLinear,double *lembda)
 {
  // Declarations

  ANALYSE_NON_LINEAR_PARAMETERS nonLinear[NNONLINEAR];
  INDEX indexNonLinear;
  RC rc;

 	// Debug

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin("mediateRequestSetAnalysisNonLinear",DEBUG_FCTTYPE_CONFIG);
  #endif

  // Initialization

  memset(nonLinear,0,sizeof(ANALYSE_NON_LINEAR_PARAMETERS)*NNONLINEAR);                 // this reset cross section file names
                                                                                // QDOAS MISSING FIELD !!!

  for (indexNonLinear=0;indexNonLinear<NNONLINEAR;indexNonLinear++)
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

  nonLinear[4].fitFlag=pNonLinear->comFlagFit;
  nonLinear[4].initialValue=pNonLinear->comInitial;
  nonLinear[4].deltaValue=pNonLinear->comDelta;
  nonLinear[4].storeFit=pNonLinear->comFlagFitStore;
  nonLinear[4].storeError=pNonLinear->comFlagErrStore;

  // Usamp1

  strcpy(nonLinear[5].symbolName,"Usamp1");

  nonLinear[5].fitFlag=pNonLinear->usamp1FlagFit;
  nonLinear[5].initialValue=pNonLinear->usamp1Initial;
  nonLinear[5].deltaValue=pNonLinear->usamp1Delta;
  nonLinear[5].storeFit=pNonLinear->usamp1FlagFitStore;
  nonLinear[5].storeError=pNonLinear->usamp1FlagErrStore;

  // Usamp2

  strcpy(nonLinear[6].symbolName,"Usamp2");

  nonLinear[6].fitFlag=pNonLinear->usamp2FlagFit;
  nonLinear[6].initialValue=pNonLinear->usamp2Initial;
  nonLinear[6].deltaValue=pNonLinear->usamp2Delta;
  nonLinear[6].storeFit=pNonLinear->usamp2FlagFitStore;
  nonLinear[6].storeError=pNonLinear->usamp2FlagErrStore;

  // Raman

  strcpy(nonLinear[7].symbolName,"Raman");

  nonLinear[7].fitFlag=pNonLinear->ramanFlagFit;
  nonLinear[7].initialValue=pNonLinear->ramanInitial;
  nonLinear[7].deltaValue=pNonLinear->ramanDelta;
  nonLinear[7].storeFit=pNonLinear->ramanFlagFitStore;
  nonLinear[7].storeError=pNonLinear->ramanFlagErrStore;

  rc=ANALYSE_LoadNonLinear(nonLinear,NNONLINEAR,lembda);

  // Debug

  #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionStop("mediateRequestSetAnalysisNonLinear",rc);
  #endif

  // Return

  return rc;
 }

int mediateRequestSetAnalysisWindows(void *engineContext,
				     int numberOfWindows,
				     const mediate_analysis_window_t *analysisWindows,
				     void *responseHandle)
 {
	 // Declarations

	 ENGINE_CONTEXT *pEngineContext;                                               // engine context
	 mediate_analysis_window_t *pAnalysisWindows;                                  // pointer to the current analysis window from the user interface
	 FENO *pTabFeno;                                                               // pointer to the description of an analysis window
	 INT indexFeno,i;                                                              // browse analysis windows
	 INT useKurucz;
	 RC rc;                                                                        // return code

	 // Debug

	 #if defined(__DEBUG_) && __DEBUG_
	 WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a general description of a symbol
  CROSS_REFERENCE *pEngineCross;
  INT indexTabCross;
  DEBUG_Start(DOAS_dbgFile,"Analysis windows",DEBUG_FCTTYPE_CONFIG,5,DEBUG_DVAR_NO,0);
  #endif

	 // Initializations

	 pEngineContext=(ENGINE_CONTEXT *)engineContext;
	 rc=ANALYSE_SetInit(pEngineContext);

  for (indexFeno=0;(indexFeno<numberOfWindows) && !rc;indexFeno++)
   {
   	// Pointers initialization

   	pTabFeno=(FENO *)&TabFeno[NFeno];
   	pAnalysisWindows=(mediate_analysis_window_t *)&analysisWindows[indexFeno];
    pTabFeno->NDET=NDET;

    // Load data from analysis windows panels

    strcpy(pTabFeno->windowName,pAnalysisWindows->name);
    strcpy(pTabFeno->residualsFile,pAnalysisWindows->residualFile);
    strcpy(pTabFeno->ref1,pAnalysisWindows->refOneFile);
    strcpy(pTabFeno->ref2,pAnalysisWindows->refTwoFile);

    if (// MISSING FIELD !!! ((pTabFeno->hidden=pAnalysisWindows->hidden)==0) &&
        ((pTabFeno->refSpectrumSelectionMode=ANLYS_REF_SELECTION_MODE_FILE
        /* QDOAS problem with pAnalysisWindows->refSpectrumSelection */)==ANLYS_REF_SELECTION_MODE_AUTOMATIC))
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
    pTabFeno->analysisMethod=(pTabFeno->hidden==1)?pKuruczOptions->analysisMethod:pAnalysisOptions->method;
    pTabFeno->Decomp=1;

    useKurucz+=pAnalysisWindows->kuruczMode;

    // Wavelength scales read out

    if (((pTabFeno->Lembda==NULL) && ((pTabFeno->Lembda=MEMORY_AllocDVector("ANALYSE_LoadData ","Lembda",0,NDET-1))==NULL)) ||
        ((pTabFeno->LembdaK==NULL) && ((pTabFeno->LembdaK=MEMORY_AllocDVector("ANALYSE_LoadData ","LembdaK",0,NDET-1))==NULL)) ||
        ((pTabFeno->LembdaRef==NULL) && ((pTabFeno->LembdaRef=MEMORY_AllocDVector("ANALYSE_LoadData ","LembdaRef",0,NDET-1))==NULL)))
     {
      rc=ERROR_ID_ALLOC;
      break;
     }

    for (i=0;i<NDET;i++)
     pTabFeno->LembdaRef[i]=i;  // NB : for satellites measurements, irradiance is retrieved later from spectra files

//     if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GDP_ASCII) &&
//         (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GDP_BIN) &&
//         (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_HDF) &&
//         (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS))

      memcpy(pTabFeno->LembdaRef,pEngineContext->buffers.lembda,sizeof(double)*NDET);

     if (!(rc=ANALYSE_LoadRef(pTabFeno,pEngineContext)) &&   // eventually, modify LembdaRef for continuous functions
         !(rc=mediateRequestSetAnalysisCross(&pAnalysisWindows->crossSectionList,pTabFeno->hidden,pTabFeno->LembdaRef)) &&    //     !(rc=mediateRequestSetAnalysisLinear(pAnalysisWindows->listEntryPoint[TAB_TYPE_ANLYS_LINEAR])) &&
         !(rc=mediateRequestSetAnalysisLinear(&pAnalysisWindows->linear)) &&
         !(rc=mediateRequestSetAnalysisNonLinear(&pAnalysisWindows->nonlinear,pTabFeno->LembdaRef)) ) // &&

     //    !(rc=AnalyseLoadPredefined(pAnalysisWindows->listEntryPoint[TAB_TYPE_ANLYS_NOTLINEAR],pTabFeno->LembdaRef)) ) // &&

    //     !(rc=AnalyseLoadShiftStretch(pAnalysisWindows->listEntryPoint[TAB_TYPE_ANLYS_SHIFT_AND_STRETCH],pAnalysisWindows->listEntryPoint[TAB_TYPE_ANLYS_CROSS])) &&
    //
    //      (pTabFeno->hidden ||
    //
    //     (!(rc=AnalyseLoadGaps(pTabFeno->LembdaRef,pAnalysisWindows,pAnalysisWindows->listEntryPoint[TAB_TYPE_ANLYS_GAPS],pTabFeno)) &&
    //     (!pTabFeno->gomeRefFlag || !(rc=ANALYSE_SvdLocalAlloc("ANALYSE_LoadData",&pTabFeno->svd)))
    //    )))
      {
    //   if (pAnalysisWindows->hidden==1)
    //    indexKurucz=NFeno;
    //   else
    //    {
    //     useUsamp+=pTabFeno->useUsamp;
    //
    //     if (pTabFeno->gomeRefFlag || MFC_refFlag)
    //      {
    //       memcpy(pTabFeno->Lembda,pTabFeno->LembdaRef,sizeof(double)*NDET);
    //       memcpy(pTabFeno->LembdaK,pTabFeno->LembdaRef,sizeof(double)*NDET);
    //
    //       if (pTabFeno->LembdaRef[NDET-1]-pTabFeno->Lembda[0]+1!=NDET)
    //        rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LembdaRef);
    //      }
    //    }
    //
    //   AnalyseSetAnalysisType();
    //

    //
    //   if (!pTabFeno->hidden)
    //    {
    //     lembdaMin=min(lembdaMin,pTabFeno->LembdaRef[0]);
    //     lembdaMax=max(lembdaMax,pTabFeno->LembdaRef[NDET-1]);
    //    }
    //

       #if defined(__DEBUG_) && __DEBUG_
       DEBUG_Print("Cross sections of %s window\n",pTabFeno->windowName);
       for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
        {
        	pEngineCross=&pTabFeno->TabCross[indexTabCross];
        	pWrkSymbol=&WorkSpace[pEngineCross->Comp];

        	DEBUG_Print("%s %s %d %-14.3e %-14.3e %-14.3e\n",pWrkSymbol->symbolName,
        	        ANLYS_crossAction[pEngineCross->crossAction],
                 pEngineCross->FitConc,pEngineCross->InitConc,
                 pEngineCross->DeltaConc,pEngineCross->I0Conc);

         DEBUG_PrintVar("Cross section",pWrkSymbol->xs.matrix,0,pWrkSymbol->xs.nl-1,0,pWrkSymbol->xs.nc-1,NULL);
        }
       #endif

       NFeno++;
      }
	  }

	 if (rc!=ERROR_ID_NO)
	  mediateDisplayErrorMessage(responseHandle);


/*      char name[ANLYSWIN_NAME_BUFFER_LENGTH];
    int kuruczMode;
    int refSpectrumSelection;
    char refOneFile[FILENAME_BUFFER_LENGTH];
    char refTwoFile[FILENAME_BUFFER_LENGTH];
    char residualFile[FILENAME_BUFFER_LENGTH];
    double refSzaCenter;
    double refSzaDelta;
    int fitMinWavelength;
    int fitMaxWavelength;
    int requireSpectrum;
    int requirePolynomial;
    int requireFit;
    int requireResidual;
    int requirePredefined;
    int requireRefRatio;


  */

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
  int rec = (pEngineContext->indexRecord)++;
  int indexLine,indexColumn,pageData,pageGraph;
  UCHAR *fileName;                                                              // the name of the current file
  plot_data_t spectrumData;

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
  pageData=pageGraph=0;

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

    EngineReadFile(pEngineContext,rec,0,0);

    sprintf(tmpString,"Spectrum (%d/%d)",pEngineContext->indexRecord-1,pEngineContext->recordNumber);

    mediateAllocateAndSetPlotData(&spectrumData, pBuffers->lembda, pBuffers->spectrum, NDET, PlotDataType_Spectrum, "legend string");
    mediateResponsePlotData(pageGraph, &spectrumData, 1, "Spectrum", "Lambda (nm)", "Counts", responseHandle);
    mediateReleasePlotData(&spectrumData);
    mediateResponseLabelPage(pageGraph, fileName, tmpString, responseHandle);
    pageGraph++;
// QDOAS ???     fprintf(fp,"Project\t\t\t%s\n",pProject->name);

    if (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_ACTON) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE)) &&
         (pBuffers->darkCurrent!=NULL))
     {
      mediateAllocateAndSetPlotData(&spectrumData, pBuffers->lembda, pBuffers->darkCurrent,NDET, PlotDataType_Spectrum, "legend string");
      mediateResponsePlotData(pageGraph, &spectrumData, 1, "Dark current", "Lambda (nm)", "Counts", responseHandle);
      mediateReleasePlotData(&spectrumData);
      mediateResponseLabelPage(pageGraph, fileName, "Dark current", responseHandle);
      pageGraph++;
     }

    if (pBuffers->sigmaSpec!=NULL)
     {
     	sprintf(tmpString,"Error (%d/%d)",pEngineContext->indexRecord-1,pEngineContext->recordNumber);

      mediateAllocateAndSetPlotData(&spectrumData, pBuffers->lembda, pBuffers->sigmaSpec,NDET, PlotDataType_Spectrum, "legend string");
      mediateResponsePlotData(pageGraph, &spectrumData, 1, "Error", "Lambda (nm)", "Counts", responseHandle);
      mediateReleasePlotData(&spectrumData);
      mediateResponseLabelPage(pageGraph, fileName, tmpString, responseHandle);
      pageGraph++;
     }

    if (pBuffers->irrad!=NULL)
     {
      mediateAllocateAndSetPlotData(&spectrumData, pBuffers->lembda, pBuffers->irrad,NDET, PlotDataType_Spectrum, "legend string");
      mediateResponsePlotData(pageGraph, &spectrumData, 1, "Irradiance spectrum", "Lambda (nm)", "Counts", responseHandle);
      mediateReleasePlotData(&spectrumData);
      mediateResponseLabelPage(pageGraph, fileName, "Irradiance", responseHandle);
      pageGraph++;
     }

    if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) &&
        (pBuffers->specMax!=NULL) &&
        (pRecord->NSomme>1))
     {
      mediateAllocateAndSetPlotData(&spectrumData, pBuffers->specMaxx, pBuffers->specMax,pRecord->rejected+pRecord->NSomme, PlotDataType_Spectrum, "legend string");
      mediateResponsePlotData(pageGraph, &spectrumData, 1, "SpecMax", "Scans number", "Signal Maximum", responseHandle);
      mediateReleasePlotData(&spectrumData);
      mediateResponseLabelPage(pageGraph, fileName, "SpecMax", responseHandle);
      pageGraph++;
     }

    mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"File","%s",

// QDOAS ???
// QDOAS ???     fprintf(fp,"Project\t\t\t%s\n",pProject->name);
// QDOAS ???
// QDOAS ???     fprintf(fp,"File\t\t\t%s\n",
// QDOAS ???           ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)||
// QDOAS ???            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)||
// QDOAS ???            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OPUS))?PATH_fileSpectra:
                  fileName);

    if (strlen(pInstrumental->instrFunction))
     {
      if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_ULB)
       mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Dark Current","%s",pInstrumental->instrFunction);
      else if (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC)
       mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Instr. function","%s",pInstrumental->instrFunction);
      else if (((pInstrumental->mfcMaskSpec!=0) && ((UINT)MFC_header.ty==pInstrumental->mfcMaskSpec)) ||
               ((pInstrumental->mfcMaskSpec==0) &&
               ((MFC_header.wavelength1==pInstrumental->mfcMaskInstr) ||
                (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.))))
       mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Instr. function","%s",pInstrumental->instrFunction);
     }

// QDOAS ???
// QDOAS ???     if (strlen(pInstrumental->vipFile))
// QDOAS ???      {
// QDOAS ???       if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)
// QDOAS ???        fprintf(fp,"Dark current\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileDark,1));
// QDOAS ???       else if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) &&
// QDOAS ???             (((pInstrumental->mfcMaskSpec!=0) && (((UINT)MFC_header.ty==pInstrumental->mfcMaskSpec) || ((UINT)MFC_header.ty==pInstrumental->mfcMaskInstr))) ||
// QDOAS ???               ((pInstrumental->mfcMaskSpec==0) &&
// QDOAS ???               ((MFC_header.wavelength1==pInstrumental->mfcMaskInstr) ||
// QDOAS ???                (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.)))))
// QDOAS ???        fprintf(fp,"Dark current\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileDark,1));
// QDOAS ???       else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)
// QDOAS ???        fprintf(fp,"Straylight correction\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->vipFile,1));
// QDOAS ???       else
// QDOAS ???        fprintf(fp,"Interpixel\t\t\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->vipFile,1));
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (strlen(pInstrumental->dnlFile))
// QDOAS ???      {
// QDOAS ???       if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)
// QDOAS ???        fprintf(fp,"Offset\t\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileOffset,1));
// QDOAS ???       else if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) &&
// QDOAS ???              (((MFC_header.ty!=0) &&
// QDOAS ???              (((UINT)MFC_header.ty==pInstrumental->mfcMaskSpec) ||
// QDOAS ???               ((UINT)MFC_header.ty==pInstrumental->mfcMaskDark) ||
// QDOAS ???               ((UINT)MFC_header.ty==pInstrumental->mfcMaskInstr))) ||
// QDOAS ???               ((pInstrumental->mfcMaskSpec==0) &&
// QDOAS ???               ((MFC_header.wavelength1==pInstrumental->mfcMaskDark) ||
// QDOAS ???                (MFC_header.wavelength1==pInstrumental->mfcMaskInstr)||
// QDOAS ???                (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.)))))
// QDOAS ???        fprintf(fp,"Offset\t\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileOffset,1));
// QDOAS ???       else
// QDOAS ???        fprintf(fp,"Non linearity\t\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->dnlFile,1));
// QDOAS ???      }
// QDOAS ???
     if (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GOME2)
      mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Date and Time","%02d/%02d/%d %02d:%02d:%02d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
// QDOAS ???     else
// QDOAS ???      mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Date and Time","%02d/%02d/%d %02d:%02d:%02d.%06d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec,GOME2_ms);

 //    sprintf(tmpString,"%.3f -> %.3f \n",pRecord->TimeDec,pRecord->localTimeDec);

     if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD))
      {
       pTime=&pRecord->startTime;
       mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Start time","%02d:%02d:%02d",pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
       pTime=&pRecord->endTime;
       mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"End time","%02d:%02d:%02d",pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
      }

     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
      mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Record","%d/%d",pEngineContext->indexRecord-1,pEngineContext->recordNumber);
     else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI)
      {
      	if (pInstrumental->averageFlag)
        mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Record","%d/%d (%d spectra averaged)",
                pEngineContext->indexRecord-1,pEngineContext->recordNumber,pRecord->omi.omiNumberOfSpectraPerTrack);
      	else
        mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Record","%d/%d (track %d/%d, spectrum %d/%d)",
                pEngineContext->indexRecord-1,pEngineContext->recordNumber,
                pRecord->omi.omiTrackIndex,pRecord->omi.omiNumberOfTracks,
                pRecord->omi.omiTrackIndex,pRecord->omi.omiNumberOfSpectraPerTrack);
      }
     else
      mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Record","%d/%d",pEngineContext->indexRecord-1,pEngineContext->recordNumber);

     if (strlen(pRecord->Nom))
      mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Record name","%s",pRecord->Nom);

// QDOAS ???
// QDOAS ???     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
// QDOAS ???      {
// QDOAS ???       if (strlen(MFC_header.FileName))
// QDOAS ???        sprintf(tmpString,"Spectrum\t\t\t%s\n",MFC_header.FileName);
// QDOAS ???       if (strlen(MFC_header.specname))
// QDOAS ???        sprintf(tmpString,"Record name\t\t%s\n",MFC_header.specname);
// QDOAS ???       if (strlen(MFC_header.site))
// QDOAS ???        sprintf(tmpString,"Site\t\t\t%s\n",MFC_header.site);
// QDOAS ???       if (strlen(MFC_header.spectroname))
// QDOAS ???        sprintf(tmpString,"Spectro name\t\t%s\n",MFC_header.spectroname);
// QDOAS ???       if (strlen(MFC_header.scan_dev))
// QDOAS ???        sprintf(tmpString,"Scan device\t\t%s\n",MFC_header.scan_dev);
// QDOAS ???       if (strlen(MFC_header.first_line))
// QDOAS ???        sprintf(tmpString,"%s\n",MFC_header.first_line);
// QDOAS ???       if (strlen(MFC_header.spaeter))
// QDOAS ???        sprintf(tmpString,"%s\n",MFC_header.spaeter);
// QDOAS ???
// QDOAS ???       if (strlen(MFC_header.backgrnd))
// QDOAS ???        sprintf(tmpString,"Background\t\t%s\n",MFC_header.backgrnd);
// QDOAS ???
// QDOAS ???       sprintf(tmpString,"ty mask\t\t\t%d\n",MFC_header.ty);
// QDOAS ???      }

     if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))
      {
      	mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Orbit number","%d",pRecord->gome.orbitNumber+1);
      	mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Pixel number","%d",pRecord->gome.pixelNumber);
      	mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Pixel type","%d",pRecord->gome.pixelType);
      }
     else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
      mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Orbit number","%d",pRecord->scia.orbitNumber);

// QDOAS ???     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
// QDOAS ???      sprintf(tmpString,"Calibration parameters\t%.2f %.3e %.3e %.3e\n",pRecord->wavelength1,pRecord->dispersion[0],
// QDOAS ???                  pRecord->dispersion[1],pRecord->dispersion[2]);
// QDOAS ???
     mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Solar Zenith angle","%-.3f °",pRecord->Zm);
// QDOAS ???
     if ((pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GDP_ASCII) &&
         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GDP_BIN) &&
         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS) &&
         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_OMI) &&
         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GOME2))
      {
       mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Exposure time","%.3f sec",pRecord->Tint);
       mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Scans taken into account","%d",pRecord->NSomme);
       mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Elevation viewing angle","%.3f °",pRecord->elevationViewAngle);
       mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Azimuth viewing angle","%.3f °",pRecord->azimuthViewAngle);
      }
     else
      {
      	mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Solar Azimuth angle","%.3f °",pRecord->Azimuth);
      	mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Viewing Zenith angle","%.3f °",pRecord->zenithViewAngle);
      	mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Viewing Azimuth angle","%.3f °",pRecord->azimuthViewAngle);
      }

     if (pRecord->rejected>0)
      mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Rejected scans","%d",pRecord->rejected);
     if (pRecord->TDet!=(double)0.)
      mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Detector temperature","%.3f °",pRecord->TDet);

    if (pRecord->longitude!=(double)0.)
     mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Longitude","%.3f",pRecord->longitude);
    if (pRecord->latitude!=(double)0.)
     mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Latitude","%.3f",pRecord->latitude);
    if (pRecord->altitude!=(double)0.)
     mediateResponseCellInfo(pageData,indexLine++,indexColumn,responseHandle,"Altitude","%.3f",pRecord->altitude);

// QDOAS ???
// QDOAS ???     if (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
// QDOAS ???          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)) &&
// QDOAS ???         ((pRecord->aMoon!=(double)0.) || (pRecord->hMoon!=(double)0.) || (pRecord->fracMoon!=(double)0.)))
// QDOAS ???      {
// QDOAS ???       sprintf(tmpString,"Moon azimuthal angle\t%.3f °\n",pRecord->aMoon);
// QDOAS ???       sprintf(tmpString,"Moon elevation\t\t%.3f °\n",pRecord->hMoon);
// QDOAS ???       sprintf(tmpString,"Moon illuminated fraction\t%.3f °\n",pRecord->fracMoon);
// QDOAS ???      }
// QDOAS ???     else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
// QDOAS ???      {
// QDOAS ???       GOME_ORBIT_FILE *pOrbitFile;
// QDOAS ???       pOrbitFile=&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];
// QDOAS ???
// QDOAS ???       sprintf(tmpString,"Cloud fraction\t\t%.2f\n",(float)pOrbitFile->gdpBinSpectrum.cloudFraction*0.01);
// QDOAS ???       sprintf(tmpString,"Cloud top pressure\t\t%.2f\n",(float)pOrbitFile->gdpBinSpectrum.cloudTopPressure*0.01);
// QDOAS ???       sprintf(tmpString,"[O3 VCD]\t\t\t%.2f DU\n",(float)pOrbitFile->gdpBinSpectrum.o3*0.01);
// QDOAS ???       sprintf(tmpString,"[No2 VCD]\t\t%.2e mol/cm2\n",(float)pOrbitFile->gdpBinSpectrum.no2*1.e13);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     sprintf(tmpString,"\n\n");
// QDOAS ???     fclose(fp);
// QDOAS ???    }

    return rec;
  }
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

  EngineResetContext((ENGINE_CONTEXT *)engineContext);

  // Return

  return 0;
 }

int mediateRequestBeginAnalyseSpectra(void *engineContext,
				      const char *spectraFileName,
				      void *responseHandle)
 {
 	// Buffer allocation

  if (EngineRequestBeginBrowseSpectra((ENGINE_CONTEXT *)engineContext,spectraFileName,THREAD_TYPE_ANALYSIS)!=ERROR_ID_NO)
 	 mediateDisplayErrorMessage(responseHandle);

  return ((ENGINE_CONTEXT *)engineContext)->recordNumber;
 }

int mediateRequestNextMatchingAnalyseSpectrum(void *engineContext,
					      void *responseHandle)
 {
  return mediateRequestNextMatchingSpectrum((ENGINE_CONTEXT *)engineContext,responseHandle);
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



  // Close open files and release allocated buffers to reset the engine context

  EngineResetContext((ENGINE_CONTEXT *)engineContext);

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

  EngineResetContext((ENGINE_CONTEXT *)engineContext);

  // Return

  return 0;
 }


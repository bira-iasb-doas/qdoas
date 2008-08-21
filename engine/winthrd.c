
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  THREADS PROCESSING
//  Name of module    :  WINTHRD.C
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
//  ==================
//  INDEXES PROCESSING
//  ==================
//
//  THRD_Context - filter operations on project tree items according to thread type;
//  THRD_SetIndexes - set indexes to browse in tree;
//  ThrdNextRecord - set indexes in order to access to the next record to process;
//
//  ================
//  EVENT PROCESSING
//  ================
//
//  THRD_Error - temporary and fatal errors handling;
//  THRD_WaitEvent - wait and process an event;
//
//  ===============
//  DATA PROCESSING
//  ===============
//
//  THRD_ResetSpecInfo - release or reset data hold by the last thread;
//  ThrdInitSpecInfo - allocate buffers for a new project;
//  THRD_CopySpecInfo - make a copy of data on the current spectra file into another structure;
//  ThrdWriteSpecInfo - write data on current spectrum in a temporary file;
//  ThrdLoadInstrumental - load instrumental functions;
//  ThrdLoadProject - load a project
//
//  ===============
//  FILE PROCESSING
//  ===============
//
//  ThrdFileSetPointers - initialize file pointers;
//  ThrdReadFile - spectra file read out;
//
//  ======================================
//  AUTOMATIC REFERENCE SPECTRUM SELECTION
//  ======================================
//
//  ThrdSetRefIndexes - set indexes of spectra selected as reference for analysis;
//  ThrdNewRef - Load new reference spectra;
//
//  ===================
//  COMMANDS PROCESSING
//  ===================
//
//  THRD_BrowseSpectra - thread used for browsing spectra in a file;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  THRD_Alloc - allocate events objects for threads synchronization;
//  THRD_Free - release events objects;
//
//  ============
//  USE EXTERNAL
//  ============
//
//  ANALYSE_LoadData,ANALYSE_ResetData,VECTOR_NormalizeVector for analysis data processing;
//  ANALYSE_Kurucz,ANALYSE_AlignReference,ANALYSE_Spectrum for reference processing and spectra analysis;
//  ----------------------------------------------------------------------------

#include "doas.h"

// ================
// GLOBAL VARIABLES
// ================

DoasCh     THRD_asciiFile[MAX_ITEM_TEXT_LEN+1],*THRD_asciiPtr;        // ascii file for exporting spectra
HANDLE    THRD_hEvents[THREAD_EVENT_MAX];      // list of events
ENGINE_CONTEXT THRD_specInfo;          // data on current spectra and reference
UINT      THRD_id=THREAD_TYPE_NONE;            // thread identification number
double    THRD_localNoon;                      // local noon
INT       THRD_localShift;
DWORD     THRD_delay;
INT       THRD_correction;
INT       THRD_browseType;
INT       THRD_treeCallFlag;
INT       THRD_isFolder;
INT       THRD_recordLast;

// ================
// STATIC VARIABLES
// ================

INDEX  THRD_indexCaller,
       THRD_indexProjectOld,THRD_indexProjectCurrent,
       THRD_indexFileOld,THRD_indexFileCurrent,
       THRD_indexRecordOld,THRD_indexRecordCurrent,THRD_indexPathFileCurrent,THRD_indexPathFileOld;
INT    THRD_lastEvent=ITEM_NONE,THRD_increment,THRD_levelMax,
       THRD_resetFlag,THRD_setOriginalFlag,
       THRD_recordFirst;
INDEX *THRD_dataIndexes;
INT    THRD_dataIndexesNumber;
FILE  *THRD_asciiFp;
INT    THRD_pathOK;
INT    THRD_endProgram;
INT    thrdRefFlag;

THRD_GOTO THRD_goto={ 1, ITEM_NONE, 1, 0, ITEM_NONE, ITEM_NONE };

// ===============
// DATA PROCESSING
// ===============

// ------------------------------------------------------------------
// THRD_ResetSpecInfo : Release or reset data hold by the last thread
// ------------------------------------------------------------------

void THRD_ResetSpecInfo(ENGINE_CONTEXT *pEngineContext)
 {
 	BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("THRD_ResetSpecInfo",DEBUG_FCTTYPE_FILE);
  #endif

  // Initialization

  pBuffers=&pEngineContext->buffers;

  // Release buffers

  if (pBuffers->lambda!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","lambda",pBuffers->lambda,0);
  if (pBuffers->instrFunction!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","instrFunction",pBuffers->instrFunction,0);
  if (pBuffers->spectrum!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","spectrum",pBuffers->spectrum,0);
  if (pBuffers->sigmaSpec!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","sigmaSpec",pBuffers->sigmaSpec,0);
  if (pBuffers->darkCurrent!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","darkCurrent",pBuffers->darkCurrent,0);
  if (pBuffers->specMax!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","specMax",pBuffers->specMax,0);
  if (pBuffers->varPix!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","varPix",pBuffers->varPix,0);
  if (pBuffers->dnl!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","dnl",pBuffers->dnl,0);
  if (pBuffers->recordIndexes!=NULL)
   MEMORY_ReleaseBuffer("THRD_ResetSpecInfo ","recordIndexes",pBuffers->recordIndexes);

  CCD_ResetInstrumental(&pEngineContext->recordInfo.ccd);

  // Reset structure

  memset(pEngineContext,0,sizeof(ENGINE_CONTEXT));

  pEngineContext->indexRecord=
  pEngineContext->indexFile=ITEM_NONE;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("THRD_ResetSpecInfo",0);
  #endif
 }

// -----------------------------------------------------
// ThrdInitSpecInfo : Allocate buffers for a new project
// -----------------------------------------------------

RC ThrdInitSpecInfo(ENGINE_CONTEXT *pEngineContext,PROJECT *pProject)
 {
  // Declarations

  PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  INT detectorSize;
  RC rc=ERROR_ID_NO;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ThrdInitSpecInfo",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  pInstrumental=&pEngineContext->project.instrumental;
  pBuffers=&pEngineContext->buffers;

  // Initialize structure

  THRD_ResetSpecInfo(pEngineContext);

  memcpy(&pEngineContext->project,pProject,sizeof(PROJECT));

  // Spectra buffers allocation

  if (!(detectorSize=pInstrumental->detectorSize))
   rc=ERROR_SetLast("ThrdInitSpecInfo",ERROR_TYPE_WARNING,ERROR_ID_OUT_OF_RANGE,"Detector size",1,65535);

  else if (((pBuffers->lambda=MEMORY_AllocDVector("ThrdInitSpecInfo ","lambda",0,detectorSize-1))==NULL) ||
           ((pBuffers->spectrum=MEMORY_AllocDVector("ThrdInitSpecInfo ","spectrum",0,detectorSize-1))==NULL) ||

          (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV) ||
            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_OHP_96) ||
            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_HA_94) ||
            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_ULB) ||
            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE)) &&

           ((pBuffers->recordIndexes=(DoasU32 *)MEMORY_AllocBuffer("ThrdInitSpecInfo ","recordIndexes",2001,sizeof(DoasU32),0,MEMORY_TYPE_ULONG))==NULL)) ||

          (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI) ||
            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&

           ((pBuffers->sigmaSpec=MEMORY_AllocDVector("ThrdInitSpecInfo ","sigmaSpec",0,detectorSize-1))==NULL)) ||
           ((pProject->spectra.darkFlag!=0) &&
           ((pBuffers->darkCurrent=MEMORY_AllocDVector("ThrdInitSpecInfo ","darkCurrent",0,detectorSize-1))==NULL)))

   rc=ERROR_ID_ALLOC;

  else

   if (pBuffers->darkCurrent!=NULL)
    VECTOR_Init(pBuffers->darkCurrent,(double)0.,detectorSize);

  // Return

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ThrdInitSpecInfo",rc);
  #endif

  return rc;
 }

// ===============
// FILE PROCESSING
// ===============

// ------------------------------------------------------------------
// THRD_SpectrumCorrection : Apply instrumental correction to spectra
// ------------------------------------------------------------------

RC THRD_SpectrumCorrection(ENGINE_CONTEXT *pEngineContext,double *spectrum)
 {
  // Declaration

  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Odd even pixel correction

  if (pEngineContext->project.lfilter.type==PRJCT_FILTER_TYPE_ODDEVEN)
   rc=FILTER_OddEvenCorrection(pEngineContext->buffers.lambda,spectrum,spectrum,NDET);

  // Return

  return rc;
 }

double THRD_GetDist(double longit, double latit, double longitRef, double latitRef)
 {
  double degToRad,
         argCos,
         dist;

  degToRad=(double)PI2/360.;

  argCos=(double)cos(latit*degToRad)*cos(latitRef*degToRad)*
                (cos(longit*degToRad)*cos(longitRef*degToRad)+sin(longit*degToRad)*sin(longitRef*degToRad)) +
                 sin(latit*degToRad)*sin(latitRef*degToRad);

  if ((double)1.-fabs(argCos)>(double)1.e-12)
   dist=(double)6370.*acos(argCos);
  else
   dist=(argCos>(double)0.)?(double)0.:(double)6370.*PI;

  return dist;
 }


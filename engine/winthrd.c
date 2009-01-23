
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


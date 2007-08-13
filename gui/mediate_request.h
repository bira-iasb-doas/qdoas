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


#ifndef _MEDIATE_REQUEST_H_GUARD
#define _MEDIATE_REQUEST_H_GUARD

//----------------------------------------------------------
// Structures for mediateRequest*
//----------------------------------------------------------

#include "mediate_project.h"
#include "mediate_analysis_window.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// mediate_symbol_t
//
// Contains all information about a single symbol. It allows the GUI to provide
// information to the engine.

typedef struct mediate_symbol
{
  // To Be Determined.
} mediate_symbol_t;


//----------------------------------------------------------
// mediateRequest interface
//----------------------------------------------------------

// mediateRequestCreateEngineContext
//
// asks the engine to create a single context for safely accessing its features
// through the mediator layer. The GUI must treat the handle blindly. It's
// content and purpose is entirely the domain of the engine. The GUI must
// provide a valid engineContext to all mediateRequest* functions. The GUI is
// responsible for releasing the engineContext, and MUST do this with the
// mediateRequestDestroyEngineHandle function.
//
// On success 0 is returned and the value of handleEngine is set,
// otherwise -1 is retured and the value of handleEngine is undefined.

int mediateRequestCreateEngineContext(void **engineContext);


// mediateRequestDestroyEngineHandle
//
// asks the engine to release any resourse associated with an engineContext.
// Zero is returned on success, -1 otherwise.

int mediateRequestDestroyEngineContext(void *engineContext);


//----------------------------------------------------------

// mediateRequestSetProject
//
// project will be defined by the GUI. The engine is responsible for copying
// any required data from project. The project details will remain valid
// for the engine until the next call to mediateRequestSetProject.
// project may be the null pointer, in which case the engine may free any
// resources DIRECTLY associated with the project.
//
// Zero is returned if the operation succeeded, -1 otherwise.
//
// On success, project becomes the 'current project'.

int mediateRequestSetProject(void *engineContext, const mediate_project_t *project);


// mediateRequestSetAnalysisWindows
//
// analysisWindows is an array with numberOfWindows elements (indexed from 0). Each element
// in the array contains all user specified information about a single analysis window. The
// order of elements is important, in that the analysis of window k may depend on the results
// of the analysis of window k-1.
// The engine is responsible for copying any data from the analysisWindows array elements.
// This information remains valid until another call to mediateRequestSetAnalysisWindows is
// made, at which point the entire sent of analysis windows are to be replaced. If numberOfWindows
// is 0 the engine may release any resources associated DIRECTLY with the current set of analysis
// windows. The value of analysisWindows is undefined if numberOfWindows is 0.
//
// Zero is returned if the operation succeeded, -1 otherwise.

int mediateRequestSetAnalysisWindows(void *engineContext, int numberOfWindows, const mediate_analysis_window_t *analysisWindows);


// TODO

int mediateRequestSetSymbols(void *engineContext, int numberOfSymbols, const mediate_symbol_t *symbols);


//----------------------------------------------------------
// Browsing Spectra Interface
//----------------------------------------------------------

// mediateRequestBeginBrowseSpectra
//
// prepare the engine for controlled stepping through the spectra file in order to return
// spectral data. spectraFileName is the file that contains the spectral data. This file is
// implicitly associated with the current project. On exit (and success), it is expected
// that a subsequent call to mediateRequestGetNextMatchingSpectrum will retrieve the first
// record (if it matched the filter conditions in the current project). 
//
// The number of spectral records in the spectraFileName is returned on success, -1 otherwise.
// An error message should be posted with
//    mediateResponseErrorMessage(messageString, errorLevel, responseHandle);
//
// On success, spectraFileName becomes the 'current spectra file'.

int mediateRequestBeginBrowseSpectra(void *engineContext, const char *spectraFileName, void *responseHandle);


// mediateRequestGotoSpectrum
//
// attempt to set the current position in the current spectra file to recordNumber
// (indexed from 1 ??). A subsequent call to mediateRequestGetNextMatchingSpectrum will
// retrieve the data for record recordNumber (if it matched the filter conditions in the
// current project). 
//
// On success, recordNumber is returned. If recordNumber is greater than the number of
// record in the file (or <= 0), 0 is returned. -1 is returned for all other errors
// and an error message should be posted with
//    mediateResponseErrorMessage(messageString, errorLevel, responseHandle);

int mediateRequestGotoSpectrum(void *engineContext, int recordNumber, void *responseHandle);


// mediateRequestGetNextMatchingSpectrum
//
// attempt to locate and extract the data for the next spectral record in the current
// spectra file that matches the filter conditions of the current project. The search
// begins with the current spectral record. On success the data is extracted and
// pre-processed based on the settings of the current project. The spectrum data is
// returned with a call to
//    mediateResponseSpectrumData(dataArray, arrayLength, actualRecordNumber, responseHandle);
//
// On success, the actual record number of the matching spectrum is returned. Zero is returned
// if a matching spectrum is not found. -1 is returned for all other errors and an error message
// should be posted with 
//    mediateResponseErrorMessage(messageString, errorLevel, responseHandle);

int mediateRequestGetNextMatchingSpectrum(void *engineContext, void *responseHandle);


// mediateRequestEndBrowseSpectra
//
// notifies the engine that any resources DIRECTLY associated with the current spectra file
// may be released.
//
// On success, 0 is returned, -1 otherwise and an error message should be posted with
//    mediateResponseErrorMessage(messageString, errorLevel, responseHandle);

int mediateRequestEndBrowseSpectra(void *engineContext, void *responseHandle);

//----------------------------------------------------------


//----------------------------------------------------------
// Analysis Interface
//----------------------------------------------------------

// mediateRequestBeginAnalysis
//
// prepare the engine for controlled stepping through the spectra file in order to analysis
// spectra. spectraFileName is the file that contains the spectral data. This file is
// implicitly associated with the current project, the set of analysis windows and set of
// symbols for the engineContext. On exit (and success), it is expected that a subsequent
// call to mediateRequestAnalyseNextMatchingSpectrum will retrieve the first record (if
// it matches the filter conditions in the current project).
// If so configured in the current project, a wavelength calibration may be performed for each
// analysis window. Sets of data related to the calibration process can be returned with
// mediateResponseGraphicalData
//
// The number of spectral records in the spectraFileName is returned on success, -1 otherwise.
// An error message should be posted with
//    mediateResponseErrorMessage(messageString, errorLevel, responseHandle);
//
// On success, spectraFileName becomes the 'current spectra file' for the engineContext.

int mediateRequestBeginAnalysis(void *engineContext, const char *spectraFileName, void *responseHandle);


// mediateRequestAnalyseNextMatchingSpectrum
//
// attempt to locate and analyse the next spectral record in the current spectra file that
// matches the filter conditions of the current project. The search begins with the current
// spectral record. On success the matching spectrum is analysed. This involves pre-processing
// based on the settings of the current project, followed by analysis of each spectral window.
// The spectral windows are processed in the order they were defined. Graphical result data is
// passed to the GUI with calls to mediateResponseGraphicalData(...). Numerical and String result
// data can be passed to the GUI in a free-format tabular-based maner with calls to
// mediateResponseCellDataDouble(page, row, column, doubleValue, responseHandle), and 
// mediateResponseCellDataInteger(page, row, column, integerValue, responseHandle), and
// mediateResponseCellDataString(page, row, column, stringValue, responseHandle). Note that
// page, row and column index from 0 and have no formal upper limit (be sensible). Repeat writes to
// the same cell will overwrite any existing data. The table is initially empty and cells/pages may
// be left blank (it is the expectation that a separate page be used for each analysis window.
// 
// On success, the actual record number of the matching spectrum is returned. Zero is returned
// if a matching spectrum is not found. -1 is returned for all other errors and an error message
// should be posted with 
//    mediateResponseErrorMessage(messageString, errorLevel, responseHandle);

int mediateRequestAnalyseNextMatchingSpectrum(void *engineContext, void *responseHandle);


// mediateRequestEndAnalysis
//
// notifies the engine that any resources DIRECTLY associated with the current spectra file
// may be released.
//
// On success, 0 is returned, -1 otherwise and an error message should be posted with
//    mediateResponseErrorMessage(messageString, errorLevel, responseHandle);

int mediateRequestEndAnalysis(void *engineContext, void *responseHandle);



#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

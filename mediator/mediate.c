

//----------------------------------------------------------
// Overview
//----------------------------------------------------------
//
// From the perspective of the interface it does not matter whether the user interface
// is a GUI or a command-line application. We use 'GUI' as a general term to refer to
// the user interface. To refer to the core DOAS functionality we use the term 'engine'.
//
// The engine is responsible for all file-level operations on data files (spectral
// records, references, slit functions, instrument response functions, cross-sections,
// etc.)
// The GUI is responsible for all file level operations on configuration files. Any
// file accessed by the engine will be specified with the full filesystem path.
//
// A software layer will manage all interacton between the GUI and the engine. This
// will be a C language function, struct and native data type level interface. We
// will refer to this later as 'mediator'.
//
// Since the GUI/engine will typically be C++/C respectively, and different dynamic
// memory management schemes are stadnard in these languages, the mediator layer
// will have a 'data-copy' policy. Responsibility for freeing dynamically allocated
// data will NEVER pass through the mediator from GUI to engine or visa versa.
//
// The data structures in the interface will NOT include any data types specific to
// either the GUI or the engine. It must be possible to compile the GUI code without
// any reference to the engine header files. Likewise, it must be possible to compile
// the engine code without any reference to the GUI header files.
//
// The functions in the mediator will be separated into two groups. The first group
// will provide the GUI with an interface to control the activities of the engine.
// The second group will provide the engine with an interface to return data to the
// GUI. The first group will have names that begin with 'mediateRequest'. The second
// group will have names that begin with 'mediateResponse'.
// The implementation of the 'mediateResponse*' functions will be associated with the
// GUI. The implementation is the 'mediateRequest*' functions will be associated with
// the engine. Since the control of the engine is driven by the GUI, the implementation
// of the 'mediateRequest*' functions will typically involve calls to 'mediateResponse*'.
// The mediateRequest* and mediateResponse* functions will always include the argument
// responseHandle. This provides a context for the GUI and the implementation of the
// mediateRequest functions must use this argument blindly. The mediateRequest* functions
// always include the argument engineHandle. This is to provide context for the engine
// and the called (GUI) of the mediateRequest* functions must use this blindly.
//
// (NOTE: While only one engine instance is being used at a time, and since the engine
// incluementation is heavlily dependent on global variables, the engine handle is
// no likely to be used by the current DOAS engine. However, making it part of the
// interface now provides some level of future-proofing for the interface.

//----------------------------------------------------------
// Structures for mediateRequest*
//----------------------------------------------------------

// mediate_project_t
//
// Contains all user-specified information about a project. It allows the GUI to
// provide information to the engine.

typedef struct mediate_project
{

} mediate_project_t;


// mediate_analysis_window_t
//
// Contains all user-specified information about a single spectral analysis window.
// It allows the GUI to provide information to the engine.

typedef struct mediate_analysis_window
{

} mediate_analysis_window_t;


// mediate_symbol_t
//
// Contains all information about a single symbol. It allows the GUI to provide
// information to the engine.

typedef struct mediate_symbol
{

} mediate_symbol_t;


//----------------------------------------------------------
// mediateRequest interface
//----------------------------------------------------------

// mediateRequestCreateEngineHandle
//
// asks the engine to create a single context for safely accessing its features
// through the mediator layer. The GUI must treat the handle blindly. It's
// content and purpose is entirely the domain of the engine. The GUI must
// provide a valid engineHandle to all mediateRequest* functions. The GUI is
// responsible for releasing the engineHandle, and MUST do this with the
// mediateRequestDestroyEngineHandle function.
//
// On success 0 is returned and the value of handleEngine is set,
// otherwise -1 is retured and the value of handleEngine is undefined.

int mediateRequestCreateEngineHandle(void **engineHandle);


// mediateRequestDestroyEngineHandle
//
// asks the engine to release any resourse associated with an engineHandle.
// Zero is returned on success, -1 otherwise.

int mediateRequestDestroyEngineHandle(void *engineHandle);


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

int mediateRequestSetProject(void *engineHandle, const mediate_project_t *project);


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

int mediateRequestSetAnalysisWindows(void *engineHandle, int numberOfWindows, const mediate_analysis_window_t *analysisWindows);


// TODO

int mediateRequestSetSymbols(void *engineHandle, int numberOfSymbols, const mediate_symbol_t *symbols);


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

int mediateRequestBeginBrowseSpectra(void *engineHandle, const char *spectraFileName, void *responseHandle);


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

int mediateRequestGotoSpectrum(void *engineHandle, int recordNumber, void *responseHandle);


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

int mediateRequestGetNextMatchingSpectrum(void *engineHandle, void *responseHandle);


// mediateRequestEndBrowseSpectra
//
// notifies the engine that any resources DIRECTLY associated with the current spectra file
// may be released.
//
// On success, 0 is returned, -1 otherwise and an error message should be posted with
//    mediateResponseErrorMessage(messageString, errorLevel, responseHandle);

int mediateRequestEndBrowseSpectra(void *engineHandle, void *responseHandle);

//----------------------------------------------------------





// mediateResponseSpectrumData
//
// 
// mediateResponseSpectrumData(dataArray, arrayLength, actualRecordNumber, responseHandle);


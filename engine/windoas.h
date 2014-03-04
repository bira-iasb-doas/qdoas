
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
//  Name of module    :  WINDOAS.H
//  Program Language  :  Borland C++ 5.0 for Windows 95/NT
//
//  Software Engineer :  Caroline FAYT and Dr. Michel VAN ROOZENDAEL
//
//        Copyright  (C) Institut d'A‚ronomie Spatiale de Belgique
//                       Avenue Circulaire, 3
//                       1180     UCCLE
//                       BELGIUM
//
//  All rights are reserved. Reproduction in whole or in part is
//  prohibited without the written consent of the copyright owner.
//
//  ----------------------------------------------------------------------------
//  Main constants and global definitions
//  ----------------------------------------------------------------------------

#if !defined(__WDOAS_)
#define __WDOAS_

#include "../mediator/mediate_common.h"
#include "../engine/output_formats.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// =====================
// CONSTANTS DEFINITIONS
// =====================

// Indexes

#define NO_SELECTED_FILE    (INDEX) -2
#define ORTHOGONAL_BASE     (INDEX) -3

// Standard arrays dimensions

#define BUFFER_SIZE   4096                                                      // any buffer allocation
#define STRING_LENGTH 1023                                                      // maximum size for strings

// Files names

#define WINDOAS_HLP                 "QDOAS.HLP"       // help file
#define WINDOAS_LOG                 "QDOAS.LOG"       // log file
#define WINDOAS_DBG                 "QDOAS.DBG"       // debug file
#define WINDOAS_TMP                 "QDOAS.TMP"       // temporary file
#define WINDOAS_SYS                 "QDOAS.SYS"       // system file

#define BRO_AMF_FILE                "bro_coef352-369nm.dat"

#define ERROR_TYPE_TIME                        0x10
#define ERROR_TYPE_OWNERDRAWN                  0x20

// ======================
// STRUCTURES DEFINITIONS
// ======================

// for ACVD
// ====================
// RESOURCES MANAGEMENT
// ====================

RC      RESOURCE_Alloc(void);
void    RESOURCE_Free(void);

// ================
// FILES PROCESSING
// ================

// ------------------------------------
// CONSTANTS AND STRUCTURES DEFINITIONS
// ------------------------------------

#define FILES_PATH_MAX 100

// Base for building file filter
// -----------------------------

typedef struct _fileType
 {
  char fileType[MAX_ITEM_DESC_LEN+1];          // type of files
  char fileExt[12];                            // extension associated to this type of files
  char defaultPath[MAX_PATH_LEN+1];            // default path
 }
FILE_TYPE;

// Paths for files
// ---------------

typedef struct _filePath
 {
  char path[MAX_PATH_LEN+1];
  int   count;
 }
FILES_PATH;

// ----------------
// GLOBAL VARIABLES
// ----------------

extern char FILES_configuration[];            // configuration file
extern FILE_TYPE FILES_types[];                     // types of files supported by application
extern int FILES_version;                           // program version
extern FILES_PATH *FILES_paths;                     // all paths implied in configuration file
extern int FILES_nPaths;                            // the size of the previous buffer

// ----------
// PROTOTYPES
// ----------

// Load data from files
// --------------------

void   FILES_CompactPath(char *newPath,char *path,int useFileName,int addFlag);
char *FILES_RebuildFileName(char *newPath,char *path,int useFileName);
void   FILES_ChangePath(char *oldPath,char *newPath,int useFileName);
void   FILES_RemoveOnePath(char *path);
void   FILES_RetrievePath(char *pathString,SZ_LEN pathStringLength,char *fullFileName,SZ_LEN fullFileNameLength,int indexFileType,int changeDefaultPath);

RC     FILES_GetMatrixDimensions(FILE *fp,const char *fileName,int *pNl,int *pNc,const char *callingFunction,int errorType);
RC     FILES_LoadMatrix(FILE *fp,const char *fileName,double **matrix,int base,int nl,int nc,const char *callingFunction,int errorType);

// Select a file
// -------------

char  *FILES_BuildFileName(char *fileName,MASK fileType);

// =================
// OBSERVATION SITES
// =================

   #define MAX_SITES                    60
   #define SITE_NAME_BUFFER_LENGTH     128
   #define SITE_ABBREV_BUFFER_LENGTH     8

   // Structures definitions
   // ----------------------

   // Observation site properties

   typedef struct _observationSites
    {
     char name[SITE_NAME_BUFFER_LENGTH];
     char abbrev[SITE_ABBREV_BUFFER_LENGTH];
     double longitude;
     double latitude;
     double altitude;
     // QDOAS ??? int gmtShift;
    }
   OBSERVATION_SITE;

   // Global variables
   // ----------------

   extern OBSERVATION_SITE  *SITES_itemList;                                    // pointer to the list of sites objects
   extern int SITES_itemN;                                                      // the number of items in the previous list

   // Prototypes
   // ----------

   RC      SITES_Add(OBSERVATION_SITE *pNewSite);
   RC      SITES_Alloc(void);
   void    SITES_Free(void);

   INDEX   SITES_GetIndex(const char *siteName);

// =======
// SYMBOLS
// =======

   // Definitions
   // -----------

   #define MAX_SYMBOL_CROSS         60                                          // default maximum number of cross sections symbols

   #define SYMBOL_ALREADY_USED    0x01                                          // mask set if a symbol is used or referenced

   // Symbol description
   // ------------------

   typedef struct _symbol
    {
     char name[MAX_ITEM_NAME_LEN+1];
     char description[MAX_ITEM_DESC_LEN+1];
    }
   SYMBOL;

   typedef SYMBOL SYMBOL_CROSS;

   // Global variables
   // ----------------

   extern SYMBOL_CROSS *SYMB_itemCrossList;                                     // pointer to list of cross sections symbols
   extern int SYMB_itemCrossN;

   INDEX SYMB_GetListIndex(SYMBOL *symbolList,int symbolNumber,char *symbolName);
   RC SYMB_Add(char *symbolName,char *symbolDescription);

   RC    SYMB_Alloc(void);
   void  SYMB_Free(void);

// =================
// MATRIX PROCESSING
// =================

// Structures definitions
// ----------------------

typedef struct _matrix
 {
  int      nl,nc;           // resp. numbers of lines and columns of matrix
  INDEX    basel,basec;     // resp. base indexes for lines and columns in matrix
  double **matrix,          // pointer to columns in the matrix
         **deriv2;          // pointer to second derivatives
 }
MATRIX_OBJECT;

// Prototypes
// ----------

RC   MATRIX_Allocate(MATRIX_OBJECT *pMatrix,int nl,int nc,int basel,int basec,int allocateDeriv2, const char *callingFunction);
void MATRIX_Free(MATRIX_OBJECT *pMatrix, const char *callingFunctionShort);
RC   MATRIX_Copy(MATRIX_OBJECT *pTarget,MATRIX_OBJECT *pSource, const char *callingFunction);
RC   MATRIX_Load(char *fileName,MATRIX_OBJECT *pMatrix,int basel,int basec,int nl,int nc,double xmin,double xmax,int allocateDeriv2,int reverseFlag, const char *callingFunction);

// ===========================
// ANALYSIS WINDOWS PROPERTIES
// ===========================

// Analysis windows
// ----------------

#define MAX_ANLYS 50

typedef struct _analysis
 {
  char  windowName[MAX_ITEM_NAME_LEN+1];                   // name of window
  int    refSpectrumSelectionMode;                          // reference spectrum selection mode
  int    useKurucz;                                         // apply Kurucz
  int    useSref;                                           // obsolete
  int    applyGrid;                                         // if calibration is performed, apply new grid
  int    applySFP;                                          // if calibration is performed with SFP fitting, use fitted SFP for convoluting Real time cross sections
  int    displaySpectrum;                                   // force display spectrum
  int    displayResidue;                                    // force display residue
  int    displayTrend;                                      // force display trend
  int    displayRefEtalon;                                  // force display alignment of reference on etalon
  int    displayPredefined;                                 // force display of predefined parameters
  int    displayFits;                                       // force display fits
  int    bandType;
  char  refSpectrumFile[MAX_ITEM_TEXT_LEN+1];              // reference spectrum file in file mode
  char  refEtalon[MAX_ITEM_TEXT_LEN+1];                    // reference etalon file
  char  residualsFile[MAX_ITEM_TEXT_LEN+1];                // residuals safe keeping
  char  lambdaMin[13],lambdaMax[13];                       // wavelengths or pixels range
  char  lambdaMinK[13],lambdaMaxK[13];                     // wavelengths or pixels range
  INDEX  listEntryPoint[TAB_TYPE_ANLYS_MAX];                // entry points to list for all tab pages
  int    hidden;                                            // flag set if window is hidden
  double refSZA,refSZADelta;
  double refLatMin,refLatMax,refLonMin,refLonMax;
  int    pixelType;
  int    nspectra;
  char  gomePixelType[4];
 }
ANALYSIS_WINDOWS;

// ----------
// PROTOTYPES
// ----------

#if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
    LRESULT CALLBACK ANLYS_WndProc(HWND hwndAnlys,unsigned int msg,WPARAM mp1,LPARAM mp2);
    void  ANLYS_ViewCrossSections(HWND hwndTree);
    void  ANLYS_DeleteOneListItem(char *textItem,INDEX indexWindow,INDEX indexTab);
#endif

void  ANLYS_CopyItems(ANALYSIS_WINDOWS *pAnlysToPaste,ANALYSIS_WINDOWS *pAnlysToCopy);
void  ANLYS_PasteItems(ANALYSIS_WINDOWS *pAnlysToPaste,ANALYSIS_WINDOWS *pNewAnlys,INDEX indexParent);
void  ANLYS_ReleaseListItems(INDEX indexWindow);
void  ANLYS_ResetConfiguration(void);
void  ANLYS_LoadConfigurationOld(char *fileName);
void  ANLYS_OutputConfiguration(FILE *fp,INDEX indexProject,INDEX indexWindow);
void  ANLYS_LoadConfiguration(char *fileLine);
void  ANLYS_SaveConfiguration(FILE *fp,char *sectionName);

RC ANLYS_Alloc(void);
void ANLYS_Free(void);

// ===================
// PROJECTS PROPERTIES
// ===================

// ----------------
// SPECTRA TAB PAGE
// ----------------

// Spectra tab page description
// ----------------------------

typedef struct _prjctSpectra
 {
  int    noMin,noMax;                                                           // spectra numbers range
  float  SZAMin,SZAMax,SZADelta;                                                // SZA range
  float  cloudMin,cloudMax;                                                     // cloud fraction range (for satellite measurements)
  float  longMin,longMax,latMin,latMax,                                         // resp. longitude and latitude ranges
         radius;                                                                // radius if circle mode is used
  int    namesFlag,                                                             // use names
         darkFlag,                                                              // use dark current
         displaySpectraFlag,                                                    // display complete spectra
         displayDataFlag,                                                       // display data on spectra
         displayCalibFlag,
         displayFitFlag,                                                        // display fits
         displayPause,                                                          // QDOAS obsolete field !!! : force pause between two graph display
         displayDelay,                                                          // QDOAS obsolete field !!! : calculated delay for display pause
         maxGraphV,                                                             // QDOAS obsolete field !!! : maximum number of graphs in height a graphic page can hold
         maxGraphH,                                                             // QDOAS obsolete field !!! : maximum number of graphs in width a graphic page can hold
         mode;
  int    fieldsNumber;                                                          // number of ascii flags set in the next list
  char fieldsFlag[PRJCT_RESULTS_MAX];                                         // fields used in ascii format
 }
PRJCT_SPECTRA;

// -----------------
// ANALYSIS TAB PAGE
// -----------------

// Analysis tab page description
// -----------------------------

  typedef struct _prjctAnlys
  {
    int method;                                        // analysis method
    int fitWeighting;                                  // least-squares fit weighting
    int units;                                         // units for shift and stretch
    int interpol;                                      // interpolation
    double convergence;                                // convergence criterion
    double spike_tolerance;                            // max ratio of (pixel residual)/(average residual)

    int securityGap;
    int maxIterations;                                 // maximum number of iterations
  }
    PRJCT_ANLYS;

// ---------------
// FILTER TAB PAGE
// ---------------

// Filter tab page description
// ---------------------------

// --------------------
// CALIBRATION TAB PAGE
// --------------------

// Calibration tab page description
// --------------------------------

typedef struct _prjctKurucz
 {
  int              windowsNumber;                      // number of windows
  int              fwhmPolynomial;                     // security gap in pixels numbers
  int              shiftPolynomial;                    // degree of polynomial to use
  char           file[MAX_ITEM_TEXT_LEN+1];          // kurucz file
  char           slfFile[MAX_ITEM_TEXT_LEN+1];       // slit function file
  int              displayFit;                         // display fit flag
  int              displayResidual;                    // display new calibration flag
  int              displayShift;                       // display shift/Fwhm in each pixel
  int              displaySpectra;                     // display fwhm in each pixel
  int              fwhmFit;                            // force fit of fwhm while applying Kurucz
  int              fwhmType;                           // type of slit function to fit
  double           lambdaLeft;
  double           lambdaRight;
  int              invPolyDegree;
  int              analysisMethod;
 }
PRJCT_KURUCZ;

// ----------------------
// UNDERSAMPLING TAB PAGE
// ----------------------

typedef struct _prjctUsamp
 {
  char  kuruczFile[MAX_STR_LEN+1];
  int    method;
  double phase;
 }
PRJCT_USAMP;

// ---------------------
// INSTRUMENTAL TAB PAGE
// ---------------------

// Types of irradiances for SCIAMACHY

// D0 ESM diffuser -  neutral density filter in (regularly updated)
//
//        this is the only calibrated irradiance
//
// D1 ESM diffuser -  neutral density filter out
//
//        In principle as good as D0, but taken only once per month.  At the beginning,
//        some occasions for updating were missed.  So it starts with 999 values like
//        the other placeholders.
//
// D6 ASM diffuser (regularly updated)
//
//        All D6 data from 2002 cannot be used due to wront measurement geometry !
//
// D9 ASM diffuser spectrum in B.U. from 09.04.2003; not updated
//
//        Not in operational products but only validation masterset !  Can be used for DOAS.


#define SCIA_REFERENCE_ESM   0
#define SCIA_REFERENCE_ASM   6
#define SCIA_REFERENCE_ASMBU 9

extern int SCIA_clusters[PRJCT_INSTR_SCIA_CHANNEL_MAX][2];
extern int SCIA_ms; // number of milliseconds

extern int GOME2_mus; //number of microseconds

// Instrumental tab page description
// ---------------------------------

typedef struct _prjctAsciiFormat
 {
  int   format,szaSaveFlag,azimSaveFlag,elevSaveFlag,timeSaveFlag,dateSaveFlag,lambdaSaveFlag;
 }
PRJCT_ASCII;

typedef struct _prjctSaozFormat
 {
 	int spectralRegion;
 	int spectralType;
 }
PRJCT_SAOZ;

typedef struct _prjctSciaFormat
 {
  int         sciaChannel;
  int         sciaCluster[6];
  char       sciaReference[4];
 }
PRJCT_SCIA;

typedef struct _prjctGomeFormat
 {
 	int bandType;
 	int pixelType;
 }
PRJCT_GOME;

typedef struct _prjctOmiFormat
 {
   char refPath[MAX_STR_LEN+1];
   int spectralType;
   int averageFlag;
   int   omiTracks[MAX_SWATHSIZE];
   int   pixelQFRejectionFlag,pixelQFMaxGaps,pixelQFMask;                       // pixel quality flags rejection
   enum omi_xtrack_mode xtrack_mode;                                            // how to use XTrackQualityFlags
 }
PRJCT_OMI;

typedef struct _prjctInstrumental
 {
  char       observationSite[MAX_ITEM_NAME_LEN+1];                            // index of observation site in list
  char       readOutFormat;                                                   // spectra read out format
  int         user;                                                             // user defined
  char       calibrationFile[MAX_ITEM_TEXT_LEN+1];                            // calibration file
  char       instrFunction[MAX_ITEM_TEXT_LEN+1];                              // instrumental function
  char       vipFile[MAX_ITEM_TEXT_LEN+1];                                    // interpixel variability correction
  char       dnlFile[MAX_ITEM_TEXT_LEN+1];                                    // detector not linearity correction
  char       offsetFile[MAX_ITEM_TEXT_LEN+1];                                 // offset file
  char       imagePath[MAX_ITEM_TEXT_LEN+1];                                  // root path for camera pictures
  int         detectorSize;                                                     // size of detector in pixels
  int         azimuthFlag;
  int         averageFlag;
  PRJCT_ASCII ascii;
  PRJCT_SAOZ  saoz;
  PRJCT_GOME  gome;
  PRJCT_SCIA  scia;
  PRJCT_OMI   omi;
  int         wavelength;
  unsigned int        mfcMaskOffset;
  unsigned int        mfcMaskDark;
  unsigned int        mfcMaskInstr;
  unsigned int        mfcMaskSpec;
  int         mfcMaskUse;
  int         mfcMaxSpectra;
  int         mfcRevert;
  int         offsetFlag;
  double      lambdaMin,lambdaMax;
  char       mfcStdDate[24];
  float       opusTimeShift;
  char       fileExt[50];
 }
PRJCT_INSTRUMENTAL;

// ----------------------
// SLIT FUNCTION TAB PAGE
// ----------------------

typedef struct _prjctSlit
 {
  SLIT  slitFunction;                                  // slit function
  int   fwhmCorrectionFlag;                            // flag set if fwhm correction is to be applied
  char kuruczFile[MAX_STR_LEN+1];
 }
PRJCT_SLIT;

// ----------------
// RESULTS TAB PAGE
// ----------------

// Description of available fields to output
// -----------------------------------------

typedef struct _prjctResultsFields
 {
  char   fieldName[2*(MAX_ITEM_NAME_LEN+1)];
  int     fieldType;
  int     fieldSize;
  int     fieldDim1,fieldDim2;
  char   fieldFormat[MAX_ITEM_NAME_LEN+1];
 }
PRJCT_RESULTS_FIELDS;

// ASCII results tab page description
// ----------------------------------

typedef struct _prjctAsciiResults
{
  char path[MAX_ITEM_TEXT_LEN+1];                                               // path for results and fits files
  int   analysisFlag,calibFlag,referenceFlag,dirFlag,fileNameFlag,successFlag;  // store results in ascii format
  char fluxes[MAX_ITEM_TEXT_LEN+1];                                             // fluxes
  double bandWidth;                                                             // averaging bandwidth for fluxes
  // char cic[MAX_ITEM_TEXT_LEN+1];                                                // color indexes
  int fieldsNumber;                                                             // number of ascii flags set in the next list
  char fieldsFlag[PRJCT_RESULTS_MAX];                                           // fields used in output
  enum output_format file_format;
  char swath_name[HDFEOS_OBJ_LEN_MAX];
 }
PRJCT_RESULTS;

// Results tab page description
// ----------------------------

typedef struct _prjctNasaResults
 {
  char path[MAX_ITEM_TEXT_LEN+1];                                          // path for results and fits files
  char nasaFlag;                                                           // use NASA-AMES format
  char no2RejectionFlag;                                                   // force NO2 rejection test to be applied
  char instrument[MAX_ITEM_TEXT_LEN+1];                                    // instrument specification
  char experiment[MAX_ITEM_TEXT_LEN+1];                                    // experiment specification
  char fields[PRJCT_RESULTS_NASA_MAX][MAX_ITEM_NAME_LEN+1];                // fields used in NASA-AMES format
 }
PRJCT_RESULTS_NASA;

// --------------------------
// PROPERTY SHEET DESCRIPTION
// --------------------------

#define MAX_PROJECT 10

typedef struct _project
 {
  char name[MAX_ITEM_NAME_LEN+1];                     // name of window
  PRJCT_SPECTRA spectra;                               // spectra selection tab page
  PRJCT_ANLYS analysis;                                // analysis tab page
  PRJCT_FILTER lfilter;                                // filter (low pass options) tab page
  PRJCT_FILTER hfilter;                                // filter (high pass options) tab page
  PRJCT_KURUCZ kurucz;                                 // Kurucz tab page
  PRJCT_USAMP usamp;                                   // undersampling tab page
  PRJCT_INSTRUMENTAL instrumental;                     // instrumental tab page
  PRJCT_RESULTS asciiResults;                    // ASCII results tab page
  PRJCT_RESULTS_NASA nasaResults;                      // NASA results tab page
  PRJCT_SLIT slit;                                     // slit function tab page
 }
PROJECT;

// ----------------
// GLOBAL VARIABLES
// ----------------

extern PROJECT *PRJCT_itemList,PRJCT_panelProject;     // list of projects
extern char   *PRJCT_AnlysInterpol[],                 // interpolation methods
               *PRJCT_AnlysMethods[],                  // analysis methods
               *PRJCT_filterTypes[],
               *PRJCT_filterOutput[];

// ----------
// PROTOTYPES
// ----------

void  PRJCT_ReleaseSite(INDEX indexProject);
void  PRJCT_New(void);
void  PRJCT_DeleteAll(void);
void  PRJCT_ExpandAll(void);

RC   PRJCT_Alloc(void);
void PRJCT_Free(void);

void PRJCT_OutputConfiguration(FILE *fp,INDEX indexProject);
void PRJCT_LoadFile(char *fileName);
void PRJCT_ResetConfiguration(void);
void PRJCT_LoadConfiguration(char *fileLine);
void PRJCT_SaveConfiguration(FILE *fp,char *sectionName);

// ===============================
// CROSS SECTIONS CONVOLUTION TOOL
// ===============================

   // ----------------------
   // STRUCTURES DEFINITIONS
   // ----------------------

   // Cross section to convolute

   typedef struct _FFT
    {
     double *fftIn;
     double *fftOut;
     double *invFftIn;
     double *invFftOut;
     int     fftSize;
     int     oldSize;
    }
   FFT;

   // ----------------
   // GLOBAL VARIABLES
   // ----------------

   extern const char *XSCONV_slitTypes[SLIT_TYPE_MAX];

   // ----------
   // PROTOTYPES
   // ----------

   // Files processing

   RC   XSCONV_LoadCalibrationFile(MATRIX_OBJECT *pLambda,char *lambdaFile,int nextraPixels);
   RC   XSCONV_LoadSlitFunction(MATRIX_OBJECT *pSlitXs,MATRIX_OBJECT *pSlitXs2,SLIT *pSlit,double *pGaussWidth,int *pSlitType);
   RC XSCONV_ConvertCrossSectionFile(MATRIX_OBJECT *pCross, double lambdaMin,double lambdaMax,double shift,int conversionMode);
   RC   XSCONV_LoadCrossSectionFile(MATRIX_OBJECT *pCross,char *crossFile,double lambdaMin,double lambdaMax,double shift,int conversionMode);

   RC XSCONV_NewSlitFunction(SLIT *pSlitOptions,MATRIX_OBJECT *pSlit,double slitParam,SLIT *pSlit2Options,MATRIX_OBJECT *pSlit2,double slitParam2);

   // Convolution functions

   RC   XSCONV_GetFwhm(double *lambda,double *slit,double *deriv2,int nl,int slitType,double *slitParam);
   RC   XSCONV_TypeNone(MATRIX_OBJECT *pXsnew,MATRIX_OBJECT *pXshr);
   RC   XSCONV_FctGauss(double *pValue,double fwhm,double step,double delta);
   RC   XSCONV_TypeGauss(const double *lambda, const double *Spec, const double *SDeriv2,double lambdaj,double dldj,double *SpecConv,double fwhm,double n,int slitType, int ndet);
   RC   XSCONV_TypeStandardFFT(FFT *pFFT,int fwhmType,double slitParam,double slitParam2,double *lambda,double *target,int size);
   RC   XSCONV_TypeStandard(MATRIX_OBJECT *pXsnew,INDEX indexLambdaMin,INDEX indexLambdaMax,const MATRIX_OBJECT *pXshr, const MATRIX_OBJECT *pSlit, const MATRIX_OBJECT *pSlit2, const MATRIX_OBJECT *pI, double *Ic,int slitType,double slitParam,double slitParam2,int wveDptFlag);
   RC   XSCONV_TypeI0Correction(MATRIX_OBJECT *pXsnew,MATRIX_OBJECT *pXshr,MATRIX_OBJECT *pI0,MATRIX_OBJECT *pSlit,MATRIX_OBJECT *pSlit2,double conc,int slitType,double slitParam,double slitParam2,int wveDptFlag);

   // Main types of child windows
   // ---------------------------

extern PRJCT_ASCII ASCII_options;
extern ANALYSIS_WINDOWS  *ANLYS_windowsList;       // analysis windows list

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

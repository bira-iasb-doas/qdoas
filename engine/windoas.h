
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

#include "../mediator/mediate_common.h"

#if !defined(__WDOAS_)
#define __WDOAS_

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// ==================
// HEADERS TO INCLUDE
// ==================

// Resources IDs

// QDOAS ??? #include "windoas.rch"
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ???
// QDOAS ??? #include "..\help\windoas.h"
// QDOAS ??? #include "..\help\windoas-anlys.h"
// QDOAS ??? #include "..\help\windoas-basis.h"
// QDOAS ???
// QDOAS ??? // DIB processing
// QDOAS ???
// QDOAS ??? #include "dibutil.h"
// QDOAS ??? #include "dibapi.h"
// QDOAS ??? #include "dibdll.h"
// QDOAS ???
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // HDF processing
// QDOAS ???
// QDOAS ??? #if defined(__INCLUDE_HDF_) && __INCLUDE_HDF_
// QDOAS ??? #include "mfhdf.h"
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // #include "ximage.h"
// QDOAS ???
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

#pragma pack(1)

// ======================
// STRUCTURES DEFINITIONS
// ======================

// for ACVD

// Date in 16 bits format

typedef struct tagNEWDATE
 {
  short	year;
  char	day;
  char	month;
 }
NEWDATE;

// Time in 16 bits format

typedef struct tagNEWTIME
 {
  char	min;
  char	hour;
  char	hwnd;
  char	sec;
 }
NEWTIME;
// QDOAS ???
// QDOAS ??? // Tab page
// QDOAS ??? // --------
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ???
// QDOAS ??? typedef struct _tabPages
// QDOAS ???  {
// QDOAS ???   DoasCh        tabTitle[MAX_ITEM_NAME_LEN+1];          // title of tab page
// QDOAS ???   INT          dlgBox;                                 // dialog box to load from resources
// QDOAS ???   DLGPROC      dlgProc;                                // procedure for processing messages from the previous dialog box
// QDOAS ???   DLGTEMPLATE *dlgTemp;                                // load template from resources
// QDOAS ???   INT          hlprc;                                  // help resource
// QDOAS ???  }
// QDOAS ??? TAB_PAGE;
// QDOAS ???
// QDOAS ??? #endif

// ====================
// RESOURCES MANAGEMENT
// ====================

RC      RESOURCE_Alloc(void);
void    RESOURCE_Free(void);

// QDOAS ???
// QDOAS ??? // ===============
// QDOAS ??? // HDF file format
// QDOAS ??? // ===============
// QDOAS ???
// QDOAS ??? #if defined(__INCLUDE_HDF_) && __INCLUDE_HDF_
// QDOAS ???
// QDOAS ??? // The structure below describes the fields to retrieve from HDF files
// QDOAS ???
// QDOAS ??? typedef struct _hdfField
// QDOAS ???  {
// QDOAS ???   char  fieldName[VSNAMELENMAX];                                                // the name of the current field
// QDOAS ???   int32 fieldOrder,                                                             // the order if the specified field
// QDOAS ???         fieldType,                                                              // the type of the field
// QDOAS ???         dataSize;                                                               // the size in bytes of data (useful for buffers allocation)
// QDOAS ???   char *data;                                                                   // pointer to the data
// QDOAS ???  }
// QDOAS ??? HDF_FIELD;
// QDOAS ???
// QDOAS ??? // Information of a SDS
// QDOAS ???
// QDOAS ??? typedef struct _hdfSDS
// QDOAS ???  {
// QDOAS ???   char  sdsName[MAX_NC_NAME];                                                   // the name of the current SDS
// QDOAS ???   int32 sdsRank,                                                                // the rank of the current SDS
// QDOAS ???         sdsDim[MAX_VAR_DIMS],                                                   // the dimensions of the current SDS
// QDOAS ???         sdsDataType,                                                            // the data type of the current SDS
// QDOAS ???         sdsDataSize,                                                            // the size in bytes of the data
// QDOAS ???         sdsIndex;                                                               // index number of the current SDS
// QDOAS ???  }
// QDOAS ??? HDF_SDS;
// QDOAS ???
// QDOAS ??? RC HDF_GetSDSInfo(DoasCh *fileName,int32 hdfSDSId,int32 sdsRef,HDF_SDS *sdsList,INT nSDS);
// QDOAS ???
// QDOAS ??? #endif
// QDOAS ???
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
  DoasCh fileType[MAX_ITEM_DESC_LEN+1];          // type of files
  DoasCh fileExt[12];                            // extension associated to this type of files
  DoasCh defaultPath[MAX_PATH_LEN+1];            // default path
 }
FILE_TYPE;

// Paths for files
// ---------------

typedef struct _filePath
 {
  DoasCh path[MAX_PATH_LEN+1];
  INT   count;
 }
FILES_PATH;

// ----------------
// GLOBAL VARIABLES
// ----------------

EXTERN DoasCh FILES_configuration[];            // configuration file
EXTERN FILE_TYPE FILES_types[];                     // types of files supported by application
EXTERN INT FILES_version;                           // program version
EXTERN FILES_PATH *FILES_paths;                     // all paths implied in configuration file
EXTERN INT FILES_nPaths;                            // the size of the previous buffer

// ----------
// PROTOTYPES
// ----------

// Load data from files
// --------------------

void   FILES_CompactPath(DoasCh *newPath,DoasCh *path,INT useFileName,INT addFlag);
DoasCh *FILES_RebuildFileName(DoasCh *newPath,DoasCh *path,INT useFileName);
void   FILES_ChangePath(DoasCh *oldPath,DoasCh *newPath,INT useFileName);
void   FILES_RemoveOnePath(DoasCh *path);
void   FILES_RetrievePath(DoasCh *pathString,SZ_LEN pathStringLength,DoasCh *fullFileName,SZ_LEN fullFileNameLength,INT indexFileType,INT changeDefaultPath);

RC     FILES_GetMatrixDimensions(FILE *fp,DoasCh *fileName,INT *pNl,INT *pNc,DoasCh *callingFunction,INT errorType);
RC     FILES_LoadMatrix(FILE *fp,DoasCh *fileName,double **matrix,INT base,INT nl,INT nc,DoasCh *callingFunction,INT errorType);

// QDOAS ??? RC     FILES_Alloc(void);
// QDOAS ??? void   FILES_Free(void);

// Select a file
// -------------

DoasCh  *FILES_BuildFileName(DoasCh *fileName,MASK fileType);

// QDOAS ??? #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ??? RC      FILES_Open(HWND hwndParent,MASK fileType,DoasCh *buffer,INT bufferSize,DoasI32 openStyles,DoasCh openMode,INT *symbolReferenceNumber,INDEX indexSymbol,INT *pFileType,INT helpID);
// QDOAS ??? RC      FILES_Select(HWND hwndParent,DoasCh *buffer,INT bufferSize,MASK fileType,INT style,DoasCh openMode,INT *symbolReferenceNumber,INDEX indexSymbol,INT helpID);
// QDOAS ??? INDEX   FILES_Insert(HWND hwndTree,INDEX indexParent,INT dataType,MASK fileType);
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // "Files" menu commands processing
// QDOAS ??? // --------------------------------
// QDOAS ???
// QDOAS ??? void    FILES_LoadConfiguration(HWND hwndParent,DoasCh *fileName);
// QDOAS ??? void    FILES_SaveConfiguration(FILE *fp);
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ??? void    FILES_EnableSaveOption(HWND hwndParent,BOOL enableFlag);
// QDOAS ??? void    FILES_MenuNew(HWND hwndParent);
// QDOAS ??? void    FILES_MenuOpen(HWND hwndParent);
// QDOAS ??? RC      FILES_MenuSave(HWND hwndParent);
// QDOAS ??? RC      FILES_MenuSaveAs(HWND hwndParent);
// QDOAS ??? #endif
// QDOAS ???
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
     DoasCh name[SITE_NAME_BUFFER_LENGTH];
     DoasCh abbrev[SITE_ABBREV_BUFFER_LENGTH];
     double longitude;
     double latitude;
     double altitude;
     // QDOAS ??? INT gmtShift;
     // QDOAS ??? INT hidden;
    }
   OBSERVATION_SITE;

   // Global variables
   // ----------------

   EXTERN OBSERVATION_SITE  *SITES_itemList;                                    // pointer to the list of sites objects
   EXTERN INT SITES_itemN;                                                      // the number of items in the previous list

   // Prototypes
   // ----------

   RC      SITES_Add(OBSERVATION_SITE *pNewSite);
   RC      SITES_Alloc(void);
   void    SITES_Free(void);

   INDEX   SITES_GetIndex(DoasCh *siteName);

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
     DoasCh name[MAX_ITEM_NAME_LEN+1];
     DoasCh description[MAX_ITEM_DESC_LEN+1];
    }
   SYMBOL;

   typedef SYMBOL SYMBOL_CROSS;

   // Global variables
   // ----------------

   EXTERN SYMBOL_CROSS *SYMB_itemCrossList;                                     // pointer to list of cross sections symbols
   EXTERN INT SYMB_itemCrossN;

   INDEX SYMB_GetListIndex(SYMBOL *symbolList,INT symbolNumber,DoasCh *symbolName);
   RC SYMB_Add(DoasCh *symbolName,DoasCh *symbolDescription);

   RC    SYMB_Alloc(void);
   void  SYMB_Free(void);

// QDOAS ??? // ============================
// QDOAS ??? // RAW SPECTRA FILES PROPERTIES
// QDOAS ??? // ============================
// QDOAS ???
// QDOAS ???    // ------------------------------------
// QDOAS ???    // CONSTANTS AND STRUCTURES DEFINITIONS
// QDOAS ???    // ------------------------------------
// QDOAS ???
// QDOAS ???    #define MAX_RAW_SPECTRA 1000
// QDOAS ???
// QDOAS ???    enum _folderFlagType
// QDOAS ???     {
// QDOAS ???      RAW_FOLDER_TYPE_NONE,           // file type
// QDOAS ???      RAW_FOLDER_TYPE_USER,           // user folder type
// QDOAS ???      RAW_FOLDER_TYPE_PATH,           // directory folder type
// QDOAS ???      RAW_FOLDER_TYPE_PATHSUB         // directory folder type, include subfolders
// QDOAS ???     };
// QDOAS ???
// QDOAS ???    // GLOBAL VARIABLES
// QDOAS ???    // ----------------
// QDOAS ???
// QDOAS ???    typedef struct _rawSpectra
// QDOAS ???     {
// QDOAS ???      DoasCh fileName[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???      DoasCh windoasPath[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???      DoasCh names[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???      DoasCh dark[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???      INDEX indexProject;
// QDOAS ???      INT   folderFlag;
// QDOAS ???      INT   notAutomatic;
// QDOAS ???      INT   hidden;
// QDOAS ???     }
// QDOAS ???    RAW_FILES;
// QDOAS ???
// QDOAS ???    EXTERN RAW_FILES *RAW_spectraFiles;     // raw spectra files names
// QDOAS ???
// QDOAS ???    // Prototypes
// QDOAS ???    // ----------
// QDOAS ???
// QDOAS ???    void RAW_UpdateWindoasPath(INDEX indexItem,INDEX indexParent,INT folderFlag);
// QDOAS ???    INT  RAW_GetFolderFlag(INDEX indexTree);
// QDOAS ???
// QDOAS ???    #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???        void RAW_BrowseSpectra(HWND hwndTree,INT displayFlag,INT analysisFlag,INT flag,INT treeCallFlag);
// QDOAS ???        LRESULT CALLBACK RAW_WndProc(HWND hwndRaw,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ???    #endif
// QDOAS ???
// QDOAS ???    RC   RAW_Alloc(void);
// QDOAS ???    void RAW_Free(void);
// QDOAS ???
// QDOAS ???    void RAW_LoadConfiguration(DoasCh *fileLine);
// QDOAS ???    void RAW_SaveConfiguration(FILE *fp,DoasCh *sectionName);

// =================
// MATRIX PROCESSING
// =================

// Structures definitions
// ----------------------

typedef struct _matrix
 {
  INT      nl,nc;           // resp. numbers of lines and columns of matrix
  INDEX    basel,basec;     // resp. base indexes for lines and columns in matrix
  double **matrix,          // pointer to columns in the matrix
         **deriv2;          // pointer to second derivatives
 }
MATRIX_OBJECT;

// Prototypes
// ----------

RC   MATRIX_Allocate(MATRIX_OBJECT *pMatrix,INT nl,INT nc,INT basel,INT basec,INT allocateDeriv2,DoasCh *callingFunction);
void MATRIX_Free(MATRIX_OBJECT *pMatrix,DoasCh *callingFunctionShort);
RC   MATRIX_Copy(MATRIX_OBJECT *pTarget,MATRIX_OBJECT *pSource,DoasCh *callingFunction);
RC   MATRIX_Load(DoasCh *fileName,MATRIX_OBJECT *pMatrix,INT basel,INT basec,INT nl,INT nc,double xmin,double xmax,INT allocateDeriv2,INT reverseFlag,DoasCh *callingFunction);

// ===========================
// ANALYSIS WINDOWS PROPERTIES
// ===========================

// -----------
// DEFINITIONS
// -----------

// Description of columns in ListView control
// ------------------------------------------

#define MAX_LIST_COLUMNS 30

#define COMBOBOX_ITEM_TYPE_NONE           0x0000           // not a combobox control
#define COMBOBOX_ITEM_TYPE_EXCLUSIVE      0x0001           // combobox to fill with symbols that are not used yet
#define COMBOBOX_ITEM_TYPE_ORTHOGONAL     0x0002           // combobox to fill with all available symbols except the one selected
#define COMBOBOX_ITEM_TYPE_POLYNOME       0x0008           // polynomial modes
#define COMBOBOX_ITEM_TYPE_AMF            0x0010           // types of AMF files
#define COMBOBOX_ITEM_TYPE_XS             0x0020           // types of AMF files

typedef struct _listColumn
 {
  DoasCh columnTitle[MAX_ITEM_NAME_LEN+1];                  // title of column in list view
  INT columnWidth;                                         // initial width of the column
  INT columnFormat;                                        // format of column
  DoasCh controlName[20];                                   // name of associated control
  UINT controlStyle;                                       // style of associated control
  DoasCh controlPermanent;                                  // flag set if control is permanent
  INT comboboxItemType;                                    // type of combobox, cfr above
  char displayNumber;                                      // number that specify set of selected columns in list to display in ListView control
  DoasCh defaultValue[MAX_ITEM_NAME_LEN+1];                 // default value
 }
LIST_COLUMN;

// ListView items
// --------------

#define MAX_LIST_ITEMS 500

typedef struct _listItem
 {
  DoasCh  crossFileName[MAX_ITEM_TEXT_LEN+1];               // cross section file associated to the symbol
  DoasCh  amfFileName[MAX_ITEM_TEXT_LEN+1];                 // air mass factors file associated to the symbol
  DoasCh  itemText[MAX_LIST_COLUMNS][MAX_ITEM_TEXT_LEN+1];  // text for items to insert in different columns of ListView control
  INT    hidden;                                           // flag that indicates if the line is hidden in the user interface
  INDEX  indexParent,indexPrevious,indexNext;              // indexes of respectively, parent, previous and next item in list
 }
LIST_ITEM;

typedef struct _anlysTabPages
 {
  DoasCh tabTitle[MAX_ITEM_NAME_LEN+1];                     // title of tab page
  #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
      LIST_COLUMN *columnList;                             // list of columns to create in ListView control
  #endif
  INT columnNumber;
  DoasCh symbolType;                                        // type of symbol to use
  SYMBOL *symbolList;                                      // list of symbols to use
  INT symbolNumber;                                        // number of symbols in previous list
  INT *symbolReferenceNumber;                              // number of times a symbol is referenced to
  INT availableSymbolNumber;                               // number of available symbols
  INT listEntryPoint;                                      // entry point in ListView items list
  INT oldListEntryPoint;                                   // entry point in list before modifications
  INDEX *pTreeEntryPoint;                                  // entry point of analysis window in tree control
  char displayNumber;                                      // number that specify set of selected columns in list to display in ListView control
  DoasCh minDisplayNumber;                                  // minimum value for previous field
  INT maxDisplayNumber;                                    // maximum value for previous field

  #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
      INT anlysHelpId;
      INT calibHelpId;
  #endif
 }
ANLYS_TAB_PAGE;

// Analysis windows
// ----------------

#define MAX_ANLYS 50

typedef struct _analysis
 {
  DoasCh  windowName[MAX_ITEM_NAME_LEN+1];                   // name of window
  INT    refSpectrumSelectionMode;                          // reference spectrum selection mode
  INT    useKurucz;                                         // apply Kurucz
  INT    useSref;                                           // obsolete
  INT    applyGrid;                                         // if calibration is performed, apply new grid
  INT    applySFP;                                          // if calibration is performed with SFP fitting, use fitted SFP for convoluting Real time cross sections
  INT    displaySpectrum;                                   // force display spectrum
  INT    displayResidue;                                    // force display residue
  INT    displayTrend;                                      // force display trend
  INT    displayRefEtalon;                                  // force display alignment of reference on etalon
  INT    displayPredefined;                                 // force display of predefined parameters
  INT    displayFits;                                       // force display fits
  INT    bandType;
  DoasCh  refSpectrumFile[MAX_ITEM_TEXT_LEN+1];              // reference spectrum file in file mode
  DoasCh  refEtalon[MAX_ITEM_TEXT_LEN+1];                    // reference etalon file
  DoasCh  residualsFile[MAX_ITEM_TEXT_LEN+1];                // residuals safe keeping
  DoasCh  lambdaMin[13],lambdaMax[13];                       // wavelengths or pixels range
  DoasCh  lambdaMinK[13],lambdaMaxK[13];                     // wavelengths or pixels range
  INDEX  listEntryPoint[TAB_TYPE_ANLYS_MAX];                // entry points to list for all tab pages
  INT    hidden;                                            // flag set if window is hidden
  double refSZA,refSZADelta;
  double refLatMin,refLatMax,refLonMin,refLonMax;
  INT    pixelType;
  INT    nspectra;
  DoasCh  gomePixelType[4];
 }
ANALYSIS_WINDOWS;

// ----------
// PROTOTYPES
// ----------

#if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
    LRESULT CALLBACK ANLYS_WndProc(HWND hwndAnlys,UINT msg,WPARAM mp1,LPARAM mp2);
    void  ANLYS_ViewCrossSections(HWND hwndTree);
    void  ANLYS_DeleteOneListItem(DoasCh *textItem,INDEX indexWindow,INDEX indexTab);
#endif

void  ANLYS_CopyItems(ANALYSIS_WINDOWS *pAnlysToPaste,ANALYSIS_WINDOWS *pAnlysToCopy);
void  ANLYS_PasteItems(ANALYSIS_WINDOWS *pAnlysToPaste,ANALYSIS_WINDOWS *pNewAnlys,INDEX indexParent);
void  ANLYS_ReleaseListItems(INDEX indexWindow);
void  ANLYS_ResetConfiguration(void);
void  ANLYS_LoadConfigurationOld(DoasCh *fileName);
void  ANLYS_OutputConfiguration(FILE *fp,INDEX indexProject,INDEX indexWindow);
void  ANLYS_LoadConfiguration(DoasCh *fileLine);
void  ANLYS_SaveConfiguration(FILE *fp,DoasCh *sectionName);

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
  INT    noMin,noMax;                                                           // spectra numbers range
  float  SZAMin,SZAMax,SZADelta;                                                // SZA range
  float  cloudMin,cloudMax;                                                     // cloud fraction range (for satellite measurements)
  float  longMin,longMax,latMin,latMax,                                         // resp. longitude and latitude ranges
         radius;                                                                // radius if circle mode is used
  INT    namesFlag,                                                             // use names
         darkFlag,                                                              // use dark current
         displaySpectraFlag,                                                    // display complete spectra
         displayDataFlag,                                                       // display data on spectra
         displayFitFlag,                                                        // display fits
         displayPause,                                                          // QDOAS obsolete field !!! : force pause between two graph display
         displayDelay,                                                          // QDOAS obsolete field !!! : calculated delay for display pause
         maxGraphV,                                                             // QDOAS obsolete field !!! : maximum number of graphs in height a graphic page can hold
         maxGraphH,                                                             // QDOAS obsolete field !!! : maximum number of graphs in width a graphic page can hold
         mode;
  INT    fieldsNumber;                                                          // number of ascii flags set in the next list
  DoasCh fieldsFlag[PRJCT_RESULTS_ASCII_MAX];                                   // fields used in ascii format
 }
PRJCT_SPECTRA;

// -----------------
// ANALYSIS TAB PAGE
// -----------------

// Analysis tab page description
// -----------------------------

typedef struct _prjctAnlys
 {
  INT method;                                        // analysis method
  INT fitWeighting;                                  // least-squares fit weighting
  INT units;                                         // units for shift and stretch
  INT interpol;                                      // interpolation
  double convergence;                                                           // convergence criterion
  int securityGap;
  int maxIterations;                                                            // maximum number of iterations
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
  INT              windowsNumber;                      // number of windows
  INT              fwhmPolynomial;                     // security gap in pixels numbers
  INT              shiftPolynomial;                    // degree of polynomial to use
  DoasCh            file[MAX_ITEM_TEXT_LEN+1];          // kurucz file
  INT              displayFit;                         // display fit flag
  INT              displayResidual;                    // display new calibration flag
  INT              displayShift;                       // display shift/Fwhm in each pixel
  INT              displaySpectra;                     // display fwhm in each pixel
  INT              fwhmFit;                            // force fit of fwhm while applying Kurucz
  INT              fwhmType;                           // type of slit function to fit
  double           lambdaLeft;
  double           lambdaRight;
  INT              invPolyDegree;
  INT              analysisMethod;
 }
PRJCT_KURUCZ;

// ----------------------
// UNDERSAMPLING TAB PAGE
// ----------------------

typedef struct _prjctUsamp
 {
  DoasCh  kuruczFile[MAX_STR_LEN+1];
  INT    method;
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

EXTERN int SCIA_clusters[PRJCT_INSTR_SCIA_CHANNEL_MAX][2];
EXTERN int SCIA_ms;

EXTERN int GOME2_ms;

// Instrumental tab page description
// ---------------------------------

typedef struct _prjctAsciiFormat
 {
  INT   format,szaSaveFlag,azimSaveFlag,elevSaveFlag,timeSaveFlag,dateSaveFlag,lambdaSaveFlag;
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
  INT         sciaChannel;
  INT         sciaCluster[6];
  DoasCh       sciaReference[4];
 }
PRJCT_SCIA;

typedef struct _prjctGomeFormat
 {
 	INT bandType;
 	INT pixelType;
 }
PRJCT_GOME;

typedef struct _prjctInstrumental
 {
  DoasCh       observationSite[MAX_ITEM_NAME_LEN+1];    // index of observation site in list
  DoasCh       readOutFormat;                           // spectra read out format
  INT         user;                                    // user defined
  DoasCh       calibrationFile[MAX_ITEM_TEXT_LEN+1];    // calibration file
  DoasCh       instrFunction[MAX_ITEM_TEXT_LEN+1];      // instrumental function
  DoasCh       vipFile[MAX_ITEM_TEXT_LEN+1];            // interpixel variability correction
  DoasCh       dnlFile[MAX_ITEM_TEXT_LEN+1];            // detector not linearity correction
  INT         detectorSize;                            // size of detector in pixels
  INT         azimuthFlag;
  INT         averageFlag;
  PRJCT_ASCII ascii;
  PRJCT_SAOZ  saoz;
  PRJCT_GOME  gome;
  PRJCT_SCIA  scia;
  INT         wavelength;
  UINT        mfcMaskOffset;
  UINT        mfcMaskDark;
  UINT        mfcMaskInstr;
  UINT        mfcMaskSpec;
  INT         mfcMaskUse;
  INT         mfcMaxSpectra;
  INT         mfcRevert;
  INT         mfcStdOffset;
  DoasCh       mfcStdDate[24];
  float       opusTimeShift;
  DoasCh       fileExt[50];
  float       omiWavelength1,omiWavelength2;
 }
PRJCT_INSTRUMENTAL;

// ----------------------
// SLIT FUNCTION TAB PAGE
// ----------------------

typedef struct _prjctSlit
 {
  SLIT  slitFunction;                                  // slit function
  INT   fwhmCorrectionFlag;                            // flag set if fwhm correction is to be applied
  DoasCh kuruczFile[MAX_STR_LEN+1];
 }
PRJCT_SLIT;

// ----------------
// RESULTS TAB PAGE
// ----------------

// Description of available fields to output
// -----------------------------------------

typedef struct _prjctResultsFields
 {
  DoasCh   fieldName[2*(MAX_ITEM_NAME_LEN+1)];
  INT     fieldType;
  INT     fieldSize;
  INT     fieldDim1,fieldDim2;
  DoasCh   fieldFormat[MAX_ITEM_NAME_LEN+1];
 }
PRJCT_RESULTS_FIELDS;

// ASCII results tab page description
// ----------------------------------

typedef struct _prjctAsciiResults
 {
  DoasCh path[MAX_ITEM_TEXT_LEN+1];                                          // path for results and fits files
  INT   analysisFlag,calibFlag,dirFlag,configFlag,binaryFlag,fileNameFlag;   // store results in ascii format
  DoasCh fluxes[MAX_ITEM_TEXT_LEN+1];                                        // fluxes
  DoasCh cic[MAX_ITEM_TEXT_LEN+1];                                           // color indexes
  INT fieldsNumber;                                                         // number of ascii flags set in the next list
  DoasCh fieldsFlag[PRJCT_RESULTS_ASCII_MAX];                                // fields used in ascii format
 }
PRJCT_RESULTS_ASCII;

// Results tab page description
// ----------------------------

typedef struct _prjctNasaResults
 {
  DoasCh path[MAX_ITEM_TEXT_LEN+1];                                          // path for results and fits files
  DoasCh nasaFlag;                                                           // use NASA-AMES format
  DoasCh no2RejectionFlag;                                                   // force NO2 rejection test to be applied
  DoasCh instrument[MAX_ITEM_TEXT_LEN+1];                                    // instrument specification
  DoasCh experiment[MAX_ITEM_TEXT_LEN+1];                                    // experiment specification
  DoasCh fields[PRJCT_RESULTS_NASA_MAX][MAX_ITEM_NAME_LEN+1];                // fields used in NASA-AMES format
 }
PRJCT_RESULTS_NASA;

// --------------------------
// PROPERTY SHEET DESCRIPTION
// --------------------------

#define MAX_PROJECT 10

typedef struct _project
 {
  DoasCh name[MAX_ITEM_NAME_LEN+1];                     // name of window
  PRJCT_SPECTRA spectra;                               // spectra selection tab page
  PRJCT_ANLYS analysis;                                // analysis tab page
  PRJCT_FILTER lfilter;                                // filter (low pass options) tab page
  PRJCT_FILTER hfilter;                                // filter (high pass options) tab page
  PRJCT_KURUCZ kurucz;                                 // Kurucz tab page
  PRJCT_USAMP usamp;                                   // undersampling tab page
  PRJCT_INSTRUMENTAL instrumental;                     // instrumental tab page
  PRJCT_RESULTS_ASCII asciiResults;                    // ASCII results tab page
  PRJCT_RESULTS_NASA nasaResults;                      // NASA results tab page
  PRJCT_SLIT slit;                                     // slit function tab page
 }
PROJECT;

// ----------------
// GLOBAL VARIABLES
// ----------------

EXTERN PROJECT *PRJCT_itemList,PRJCT_panelProject;     // list of projects
EXTERN PRJCT_RESULTS_FIELDS PRJCT_resultsAscii[];      // list of printable fields in a record
EXTERN DoasCh   *PRJCT_AnlysInterpol[],                 // interpolation methods
               *PRJCT_AnlysMethods[],                  // analysis methods
               *PRJCT_filterTypes[],
               *PRJCT_filterOutput[];
// QDOAS ??? EXTERN HWND     PRJCT_hwndPage;

// ----------
// PROTOTYPES
// ----------

void  PRJCT_ReleaseSite(INDEX indexProject);
void  PRJCT_New(void);
void  PRJCT_DeleteAll(void);
void  PRJCT_ExpandAll(void);

// QDOAS ??? #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ??? LRESULT CALLBACK PRJCT_SpectraWndProc(HWND hwndSpectra,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? LRESULT CALLBACK PRJCT_AnlysWndProc(HWND hwndPrjct,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? LRESULT CALLBACK PRJCT_FilterWndProc(HWND hwndFilterTab,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? LRESULT CALLBACK PRJCT_CalibrationWndProc(HWND hwndCalibrationTab,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? LRESULT CALLBACK PRJCT_UsampWndProc(HWND hwndUsamp,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? LRESULT CALLBACK PRJCT_InstrumentalWndProc(HWND hwndInstrumental,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? LRESULT CALLBACK PRJCT_SlitWndProc(HWND hwndInstrumental,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? LRESULT CALLBACK PRJCT_ResultsAsciiWndProc(HWND hwndResults,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? LRESULT CALLBACK PRJCT_ResultsNasaWndProc(HWND hwndResults,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? LRESULT CALLBACK PRJCT_WndProc(HWND hwndPrjct,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? #endif
// QDOAS ???
RC   PRJCT_Alloc(void);
void PRJCT_Free(void);

void PRJCT_OutputConfiguration(FILE *fp,INDEX indexProject);
void PRJCT_LoadFile(DoasCh *fileName);
void PRJCT_ResetConfiguration(void);
void PRJCT_LoadConfiguration(DoasCh *fileLine);
void PRJCT_SaveConfiguration(FILE *fp,DoasCh *sectionName);

// ===============================
// CROSS SECTIONS CONVOLUTION TOOL
// ===============================

   // ----------------------
   // STRUCTURES DEFINITIONS
   // ----------------------

   // Cross section to convolute

   typedef struct _xs
    {
     INT     NDET;                                                              // size of vector
     double *lambda,                                                            // wavelength calibration
            *vector,                                                            // cross section
            *deriv2;                                                            // second derivative for interpolation
    }
   XS;

   typedef struct _FFT
    {
     double *fftIn;
     double *fftOut;
     double *invFftIn;
     double *invFftOut;
     INT     fftSize;
     INT     oldSize;
    }
   FFT;

   // ----------------
   // GLOBAL VARIABLES
   // ----------------

   EXTERN DoasCh *XSCONV_slitTypes[SLIT_TYPE_MAX];

   // ----------
   // PROTOTYPES
   // ----------

   // Files processing

   RC   XSCONV_LoadCalibrationFile(XS *pLambda,DoasCh *lambdaFile,INT nextraPixels);
   RC   XSCONV_LoadSlitFunction(XS *pSlitXs,SLIT *pSlit,double *pGaussWidth,INT *pSlitType);
   RC   XSCONV_LoadCrossSectionFile(XS *pCross,DoasCh *crossFile,double lambdaMin,double lambdaMax,double shift,INT conversionMode);

   RC XSCONV_NewSlitFunction(SLIT *pSlitOptions,XS *pSlit,double slitParam,SLIT *pSlit2Options,XS *pSlit2,double slitParam2);

   // Convolution functions

   RC   XSCONV_TypeNone(XS *pXsnew,XS *pXshr);
   RC   XSCONV_FctGauss(double *pValue,double fwhm,double step,double delta);
   RC   XSCONV_TypeGauss(double *lambda,double *Spec,double *SDeriv2,double lambdaj,double dldj,double *SpecConv,double fwhm,double n,INT slitType);
   RC   XSCONV_TypeStandardFFT(FFT *pFFT,INT fwhmType,double slitParam,double slitParam2,double *lambda,double *target,INT size);
   RC   XSCONV_TypeStandard(XS *pXsnew,INDEX indexLambdaMin,INDEX indexLambdaMax,XS *pXshr,XS *pSlit,XS *pI,double *Ic,INT slitType,double slitWidth,double slitParam,double slitParam2,double slitParam3,double slitParam4);
   RC   XSCONV_TypeI0Correction(XS *pXsnew,XS *pXshr,XS *pI0,XS *pSlit,double conc,INT slitType,double slitWidth,double slitParam,double slitParam2,double slitParam3,double slitParam4);
   RC   XSCONV_RealTimeXs(XS *pXshr,XS *pXsI0,XS *pSlit,double *IcVector,double *lambda,INT NDET,INDEX indexLambdaMin,INDEX indexLambdaMax,double *newXs,INT slitType,double slitParam,double slitParam2,double slitParam3,double slitParam4);

   // Buffers allocation

   void XSCONV_Reset(XS *pXsconv);
   RC   XSCONV_Alloc(XS *pXsconv,INT npts,INT deriv2Flag);

// QDOAS ???
// QDOAS ???    // Options in the WinDOAS configuration file
// QDOAS ???
// QDOAS ???    void XSCONV_ResetConfiguration(void);
// QDOAS ???    RC   XSCONV_LoadConfiguration(DoasCh *fileLine);
// QDOAS ???    void XSCONV_SaveConfiguration(FILE *fp);
// QDOAS ???
// QDOAS ???    // Dialog box processing
// QDOAS ???
// QDOAS ???    #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ???        void XSCONV_FileSelection(HWND hwndXsconv,DoasCh *file,MASK fileType,INT fileMode,INT fileCommand,INT ringFlag);
// QDOAS ???
// QDOAS ???        void XSCONV_SlitType(HWND hwndSlit,INT slitBase,SLIT *pSlit,SLIT *pSlit2);
// QDOAS ???
// QDOAS ???        LRESULT CALLBACK XSCONV_GeneralWndProc(HWND hwndGeneral,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ???        LRESULT CALLBACK XSCONV_SlitWndProc(HWND hwndSlit,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ???        LRESULT CALLBACK XSCONV_WndProc(HWND hwndXsconv,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ???
// QDOAS ???    #endif
// QDOAS ???

// ============================
// TREEVIEW CONTROLS PROCESSING
// ============================

// ----------------------
// TREE ITEMS DESCRIPTION
// ----------------------

// Maximum number of items supported by structure used for TreeView controls processing
// ------------------------------------------------------------------------------------

#define MAX_TREE_ITEMS 1500

// Node types for tree items properties
// ------------------------------------

enum treeItemTypes
 {
  TREE_ITEM_TYPE_NONE,                      // item has no type
  TREE_ITEM_TYPE_SITE_PARENT,               // observation sites parent node
  TREE_ITEM_TYPE_SITE_CHILDREN,             // observation sites children nodes
  TREE_ITEM_TYPE_CROSS_PARENT,              // cross sections symbols parent node
  TREE_ITEM_TYPE_CROSS_CHILDREN,            // cross sections symbols children nodes
  TREE_ITEM_TYPE_PROJECT_PARENT,            // project parent node
  TREE_ITEM_TYPE_PROJECT,                   // project nodes
  TREE_ITEM_TYPE_FILE_PARENT,               // files parent node
  TREE_ITEM_TYPE_FILE_CHILDREN,             // files children nodes
  TREE_ITEM_TYPE_ANALYSIS_PARENT,           // analysis parent nodes
  TREE_ITEM_TYPE_ANALYSIS_CHILDREN,         // analysis windows children nodes
  TREE_ITEM_TYPE_MAX
 };

// Data to collect per type of tree node
// -------------------------------------

typedef struct _treeItemType
 {
  INT     contextMenu;                      // context (or shortcut) menu to load from resources

  #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
  INT     dlgBox;                           // dialog box to load from resources
  DLGPROC dlgProc;                          // procedure for processing messages from the previous dialog box
  #endif

  INT     childDataType;                    // type of data for children nodes
  void   *dataList;                         // pointer to buffer used for storing data associated to children nodes
  void   *data2Paste;                       // pointer to the structure storing the data to paste
  INDEX   dataNumber;                       // number of data element in previous list
  INDEX   dataMaxNumber;                    // maximum number of data the list can support
  INT     dataSize;                         // size of data in bytes
  MASK    childFileType;                    // type of file for children
 }
TREE_ITEM_TYPE;

// Tree items description
// ----------------------

typedef struct _treeItem
 {
  DoasCh     textItem[MAX_ITEM_TEXT_LEN+1];  // complete item text
  DoasCh     newItem;                        // flag set for a new item
  INDEX     parentItem;                     // index of parent item
  INDEX     firstChildItem;                 // index of the first child in list
  INDEX     lastChildItem;                  // index of the first child in list
  INDEX     prevItem;                       // index of previous item in list
  INDEX     nextItem;                       // index of the next item in list
  INT       dataType;                       // type of data
  INDEX     dataIndex;                      // index of data in data list referenced by data type
  INT       useCount;                       // the number of times the item is referenced
  INT       childNumber;                    // total number of children
  INT       childHidden;                    // total number of hidden children
  DoasCh     hidden;                         // flag set if item is hidden
 }
TREE_ITEM;
// QDOAS ???
// QDOAS ??? // ----------------
// QDOAS ??? // GLOBAL VARIABLES
// QDOAS ??? // ----------------
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? EXTERN HIMAGELIST      TREE_hImageList;     // Handle to ImageList
// QDOAS ??? #endif
// QDOAS ???
EXTERN TREE_ITEM      *TREE_itemList;       // structure used for all tree items safe keeping
EXTERN TREE_ITEM_TYPE  TREE_itemType[];     // all tree types description
// QDOAS ??? EXTERN DoasCh           TREE_editFlag;       // flag set when editing a tree item label
// QDOAS ??? EXTERN INDEX           TREE_blankImageIndex,
// QDOAS ???                        TREE_openImageIndex,
// QDOAS ???                        TREE_closeImageIndex;
// QDOAS ??? EXTERN INT             TREE_typeOfItem2Copy;
// QDOAS ???
// QDOAS ??? // ----------
// QDOAS ??? // PROTOTYPES
// QDOAS ??? // ----------
// QDOAS ???
// QDOAS ??? // Resources management
// QDOAS ??? // --------------------
// QDOAS ???
// QDOAS ??? RC      TREE_Alloc(void);
// QDOAS ??? void    TREE_Free(void);
// QDOAS ??? void    TREE_Init(void);
// QDOAS ???
// QDOAS ??? // Tree control processing
// QDOAS ??? // -----------------------
// QDOAS ???
// QDOAS ??? INDEX   TREE_GetProjectParent(INDEX indexItem);
// QDOAS ??? void    TREE_CollapseChildNodes(HWND hwndTree,INDEX indexParent);
// QDOAS ??? INDEX   TREE_GetIndexByDataName(DoasCh *dataName,DoasCh dataType,INDEX entryPoint);
INDEX   TREE_GetIndexByDataIndex(INDEX dataIndex,DoasCh dataType,INDEX entryPoint);
// QDOAS ??? INT     TREE_GetIndexesByDataType(DoasCh dataType,INDEX entryPoint,INDEX *dataIndexes,INT *pDataNumber);
// QDOAS ??? void    TREE_UpdateItem(HWND hwndTree,INDEX indexItem);
// QDOAS ??? void    TREE_Reset(INDEX indexParent);
// QDOAS ??? INDEX   TREE_GetSelectedItem(HWND hwndTree);
// QDOAS ??? INDEX   TREE_DeleteOneItem(INDEX indexItemToDelete);
// QDOAS ??? INDEX   TREE_InsertOneItem(HWND hwndTree,DoasCh *textItem,INDEX parentItem,INT dataType,DoasCh newItem,DoasCh folderFlag,DoasCh hidden);
// QDOAS ??? void    TREE_DeleteChildList(INDEX indexParent);
// QDOAS ??? void    TREE_ExpandOneNode(HWND hwndTree,INDEX indexParent,HTREEITEM hParent);
// QDOAS ??? void    TREE_SortChildNodes(HWND hwndTree,INDEX indexParent);
// QDOAS ???
// QDOAS ??? // MDI child windows messages processing
// QDOAS ??? // -------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? HWND    TREE_Create(HWND hwndParent,INDEX indexTree);
// QDOAS ??? void    TREE_ReSize(HWND hwndParent,HWND hwndTree);
// QDOAS ??? void    TREE_Context(INDEX indexWindow,LPARAM mp2);
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // MDI child windows notification messages processing
// QDOAS ??? // --------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? void    TREE_GetDispInfo(HWND hwndTree,TV_DISPINFO FAR *pTree);
// QDOAS ??? void    TREE_BeginLabelEdit(HWND hwndTree,TV_DISPINFO FAR *pTree);
// QDOAS ??? void    TREE_EndLabelEdit(HWND hwndTree,TV_DISPINFO FAR *pTree,INDEX menuID);
// QDOAS ??? void    TREE_ExpandNode(HWND hwndTree,NM_TREEVIEW *pTree);
// QDOAS ??? void    TREE_CollapseNode(HWND hwndTree,NM_TREEVIEW *pTree);
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // MDI child windows context menu commands processing
// QDOAS ??? // --------------------------------------------------
// QDOAS ???
// QDOAS ??? void    TREE_InsertItem(HWND hwndTree,INT menuID,INT folderFlag);
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? void    TREE_Show(HWND hwndTree);
// QDOAS ??? void    TREE_Hide(HWND hwndTree);
// QDOAS ??? void    TREE_RenameItem(HWND hwndTree);
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? void    TREE_Sort(HWND hwndTree);
// QDOAS ??? void    TREE_DeleteItem(HWND hwndTree,INT menuID);
// QDOAS ??? void    TREE_DeleteAllItems(HWND hwndTree,INT menuID);
// QDOAS ??? void    TREE_ExpandAll(HWND hwndTree,INDEX indexParent);
// QDOAS ??? void    TREE_CollapseAll(HWND hwndTree,INDEX indexParent);
// QDOAS ??? void    TREE_Properties(HWND hwndTree);
// QDOAS ??? void    TREE_Copy(HWND hwndTree,INT cutFlag);
// QDOAS ??? void    TREE_Paste(HWND hwndTree);
// QDOAS ??? void    TREE_OutputConfig(HWND hwndTree);
// QDOAS ???
// QDOAS ??? // ===================================
// QDOAS ??? // DRAWING SPECTRA IN MDI CHILD WINDOW
// QDOAS ??? // ===================================
// QDOAS ???
// QDOAS ???    #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ???        // ---------------------
// QDOAS ???        // CONSTANTS DEFINITIONS
// QDOAS ???        // ---------------------
// QDOAS ???
// QDOAS ???        #define DRAW_CLASS "ClientWndClass"                                      // registration class
// QDOAS ???        #define MAX_GRAPH 20                                                     // the maximum number of graphs to draw in the MDI window
// QDOAS ???
// QDOAS ???        // Colours in RGB system colour
// QDOAS ???        // ----------------------------
// QDOAS ???
// QDOAS ???        #define CLR_WHITE                0x00FFFFFFL                             // 00RRGGBB (RGB system colour)
// QDOAS ???        #define CLR_BLACK                0x00000000L
// QDOAS ???        #define CLR_DARKGRAY             0x00101010L
// QDOAS ???        #define CLR_PALEGRAY             0x00C0C0C0L
// QDOAS ???        #define CLR_GREEN                0x0000FF00L
// QDOAS ???        #define CLR_RED                  0x000000FFL
// QDOAS ???        #define CLR_BLUE                 0x00FF0000L
// QDOAS ???
// QDOAS ???        #define DRAW_COLOR1              CLR_RED                                 // For Jos, use CLR_BLUE
// QDOAS ???        #define DRAW_COLOR2              CLR_BLACK
// QDOAS ???
// QDOAS ???        enum _drawStatus { DRAW_NO_SPECTRA, DRAW_READY, DRAW_DISPLAY, DRAW_SETTINGS };
// QDOAS ???
// QDOAS ???        // ----------------------
// QDOAS ???        // STRUCTURES DEFINITIONS
// QDOAS ???        // ----------------------
// QDOAS ???
// QDOAS ???        // Frame coordinates
// QDOAS ???
// QDOAS ???        typedef struct _drawFrame
// QDOAS ???         {
// QDOAS ???          double left,right,top,bottom;          // margins around a frame
// QDOAS ???         }
// QDOAS ???        DRAW_FRAME;
// QDOAS ???
// QDOAS ???        // Spectra specifications
// QDOAS ???
// QDOAS ???        typedef struct _drawSpectrum
// QDOAS ???         {
// QDOAS ???          DoasCh legend[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???          double *x,*y;                          // couple of XY vectors
// QDOAS ???          INT usex,usey;                         // flag set to tell which vectors are used
// QDOAS ???          INT vectorSize;                        // current size of previous vectors
// QDOAS ???          INT vectorSizeMax;                     // effective size of previous vectors
// QDOAS ???          INDEX indexPixMin,indexPixMax;         // limits of spectra window
// QDOAS ???          COLORREF colour;                       // colour for pen to use
// QDOAS ???          INT lineType;                          // type of line for pen to use
// QDOAS ???         }
// QDOAS ???        DRAW_SPECTRUM;
// QDOAS ???
// QDOAS ???        // Graph description
// QDOAS ???
// QDOAS ???        typedef struct _graph
// QDOAS ???         {
// QDOAS ???          DoasCh title[MAX_ITEM_TEXT_LEN+1],      // title of graph
// QDOAS ???                xTitle[MAX_ITEM_TEXT_LEN+1],     // title of x axis
// QDOAS ???                yTitle[MAX_ITEM_TEXT_LEN+1];     // title of y axis
// QDOAS ???          DRAW_FRAME specFrame;                  // frame in which spectra are to be hold to
// QDOAS ???          DRAW_SPECTRUM spectrum,dotSpectrum;    // spectra to display (max 2.)
// QDOAS ???          double  pixMin,pixMax;                 // extrema of graphics in pixels units
// QDOAS ???          double  specMin,specMax;               // extrema of graphics in spectrum units
// QDOAS ???          double *vGrid;                         // force vertical grid
// QDOAS ???          INT     nvGrid;                        // current size of previous vector
// QDOAS ???          INT     nvGridMax;                     // effective size of previous vector
// QDOAS ???          INDEX   indexChild;                    // index of the MDI child window that contain a zoom of the graph
// QDOAS ???          INT     autoScaleX,autoScaleY;
// QDOAS ???          double  pixMinOld,pixMaxOld,
// QDOAS ???                  specMinOld,specMaxOld;
// QDOAS ???         }
// QDOAS ???        DRAW_GRAPH;
// QDOAS ???
// QDOAS ???        // ----------
// QDOAS ???        // Prototypes
// QDOAS ???        // ----------
// QDOAS ???
// QDOAS ???        void  DRAW_Reset(INDEX indexChild);
// QDOAS ???        void  DRAW_Init(void);
// QDOAS ???        void  DRAW_Close(void);
// QDOAS ???        INDEX DRAW_OpenWindow(void);
// QDOAS ???        void  DRAW_CloseWindow(INDEX indexChild);
// QDOAS ???        BOOL  DRAW_ClassRegistration(HINSTANCE hinstParent);
// QDOAS ???        void  DRAW_GetGraphDistribution(INT graphNumber,INT *pGraphMaxV,INT *pGraphMaxH);
// QDOAS ???
// QDOAS ???        void  DRAW_Spectra(INT indexChild,DoasCh *windowTitle,DoasCh *graphTitle,DoasCh *xTitle,DoasCh *yTitle,double *vGrid,INT nvGrid,
// QDOAS ???                           double xMin,double xMax,double yMin,double yMax,
// QDOAS ???                           double *x,double *y,INT size,COLORREF colour,INDEX indexMin,INDEX indexMax,INT lineType,DoasCh *legend,
// QDOAS ???                           double *dotx,double *doty,INT dotSize,COLORREF dotColour,INDEX dotIndexMin,INDEX dotIndexMax,INT dotLineType,DoasCh *dotLegend,
// QDOAS ???                           INDEX graphNumber,INT graphMaxV,INT graphMaxH,INT paintFlag);
// QDOAS ???
// QDOAS ???        void  DRAW_SpecMax(double *specMax,INT totalScansNumber,COLORREF specMaxColour);
// QDOAS ???        void  DRAW_ZoomOut(void);
// QDOAS ???        void  DRAW_ClearWindow(INDEX indexWindow);
// QDOAS ???        void  DRAW_ReSize(HWND hwndParent,HWND hwndDraw);
// QDOAS ???        void  DRAW_Context(INDEX indexWindow,LPARAM mp2);
// QDOAS ???        void  DRAW_Destroy(HWND hwndDraw,UINT message,WPARAM wparam,LPARAM lparam);
// QDOAS ???
// QDOAS ???    #endif
// QDOAS ???
// QDOAS ??? // =================
// QDOAS ??? // MDI CHILD WINDOWS
// QDOAS ??? // =================
// QDOAS ???
// QDOAS ???    // Constants definitions
// QDOAS ???    // ---------------------
// QDOAS ???
// QDOAS ???    #define MAX_MDI_WINDOWS        50
// QDOAS ???    #define CHILD_DEFAULT_WIDTH   500
// QDOAS ???    #define CHILD_DEFAULT_HEIGHT  300
// QDOAS ???
// QDOAS ???    #if !defined (CW_USEDEFAULT)
// QDOAS ???    #define CW_USEDEFAULT           0
// QDOAS ???    #endif
// QDOAS ???
   // Main types of child windows
   // ---------------------------

// QDOAS ???   enum childWindowsTypes
// QDOAS ???    {
// QDOAS ???     CHILD_WINDOW_ENVIRONMENT,
// QDOAS ???     CHILD_WINDOW_PROJECT,
// QDOAS ???     CHILD_WINDOW_SPECTRA,
// QDOAS ???     CHILD_WINDOW_SPECINFO,
// QDOAS ???     CHILD_WINDOW_SPECMAX,
// QDOAS ???     CHILD_WINDOW_MAX
// QDOAS ???    };
// QDOAS ???
// QDOAS ???    #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ???        // MDI child windows description
// QDOAS ???        // -----------------------------
// QDOAS ???
// QDOAS ???        // Draw environment to attribute to a MDI child window
// QDOAS ???
// QDOAS ???        typedef struct _drawMDI
// QDOAS ???         {
// QDOAS ???          HWND hwndDraw;                                                            // handle of draw client area
// QDOAS ???          HBITMAP hbmBitmap,                                                        // handle of the bitmap compatible with the device in use
// QDOAS ???                  hdcBitmap,                                                        // handle of the memory used to store an image of the previous bitmap
// QDOAS ???                  hbmStock;                                                         // handle of the previous selected bitmap
// QDOAS ???          INT cxBitmap,cyBitmap;                                                    // resp. width and height of bitmap
// QDOAS ???          DRAW_GRAPH drawGraph[MAX_GRAPH];                                          // settings of graphs to hold in draw client area
// QDOAS ???          DoasCh title[MAX_ITEM_TEXT_LEN+1];                                         // title of mdi child window
// QDOAS ???          DoasCh graphNumber;                                                        // number of graph to draw in the draw client area
// QDOAS ???          DoasCh graphMax;                                                           // total number of graphs (graphMaxV*graphMaxH)
// QDOAS ???          DoasCh graphMaxV;                                                          // total number of graphs to hold vertically in the draw client area
// QDOAS ???          DoasCh graphMaxH;                                                          // total number of graphs to hold horizontally in the draw client area
// QDOAS ???         }
// QDOAS ???        DRAW_MDI;
// QDOAS ???
// QDOAS ???    #endif
// QDOAS ???
// QDOAS ???    #if !defined(__WINDOAS_WIN_)  || !(__WINDOAS_WIN_)
// QDOAS ???    typedef struct tagRECT
// QDOAS ???    {
// QDOAS ???        int left;
// QDOAS ???        int top;
// QDOAS ???        int right;
// QDOAS ???        int bottom;
// QDOAS ???    } RECT;
// QDOAS ???    #endif
// QDOAS ???
// QDOAS ???    typedef struct _childWindows
// QDOAS ???     {
// QDOAS ???     	#if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???          HWND hwndChild;                                                           // handle of the child window
// QDOAS ???          INDEX indexMenuItem;                                                      // index of the corresponding item in Windows menu
// QDOAS ???          HWND hwndEdit;                                                            // handle of edit control owned by the MDI child window
// QDOAS ???      #endif
// QDOAS ???          HWND hwndTree;                                                            // handle of tree owned by the MDI child window
// QDOAS ???          INT openFlag;                                                             // authorization for opening window
// QDOAS ???          INT graphFlag;                                                            // flag set if window is owned by a graph
// QDOAS ???          RECT rect;                                                                // size and position
// QDOAS ???      #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???          DRAW_MDI drawEnvironment;                                                 // draw client area if any
// QDOAS ???      #endif
// QDOAS ???          INT minimizedFlag;
// QDOAS ???      INDEX itemTree;                                                               // entry point in the list with tree items
// QDOAS ???     }
// QDOAS ???    CHILD_WINDOWS;
// QDOAS ???
// QDOAS ???    // Global variables
// QDOAS ???    // ----------------
// QDOAS ???
// QDOAS ???    EXTERN CHILD_WINDOWS CHILD_list[];                                              // list of child windows
// QDOAS ???
// QDOAS ???    #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???        EXTERN HWND          CHILD_hwndFrame;                                       // child area handle
// QDOAS ???    #endif
// QDOAS ???
// QDOAS ???    // Prototypes
// QDOAS ???    // ----------
// QDOAS ???
// QDOAS ???    #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???        void    CHILD_Create(HWND hwndParent);
// QDOAS ???        HWND    CHILD_Open(INT menuID,INT checkOpenFlag);
// QDOAS ???        void    CHILD_View(INDEX menuItem);
// QDOAS ???        INDEX   CHILD_GetActive(void);
// QDOAS ???        void    CHILD_SetActive(INDEX menuItem);
// QDOAS ???        BOOL    CHILD_ClassRegistration(HINSTANCE hinstParent);
// QDOAS ???        LRESULT CHILD_ReSize(HWND hwndParent,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ???    #endif
// QDOAS ???
// QDOAS ???    void    CHILD_ResetConfiguration(void);
// QDOAS ???    void    CHILD_LoadConfiguration(DoasCh *fileLine);
// QDOAS ???    void    CHILD_SaveConfiguration(FILE *fp,DoasCh *sectionName);
// QDOAS ???
// QDOAS ???
// QDOAS ??? // ======================
// QDOAS ??? // ASCII FILES MANAGEMENT
// QDOAS ??? // ======================
// QDOAS ???
EXTERN PRJCT_ASCII ASCII_options;
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? LRESULT CALLBACK ASCII_WndProc(HWND hwndAscii,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? void ASCII_SaveSpectra(INDEX indexWindow);
// QDOAS ???

// QDOAS ??? // ========================
// QDOAS ??? // UNDERSAMPLING CORRECTION
// QDOAS ??? // ========================
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? LRESULT CALLBACK USAMP_WndProc(HWND hwndUsamp,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? void USAMP_ResetConfiguration(void);
// QDOAS ??? RC   USAMP_LoadConfiguration(DoasCh *fileLine);
// QDOAS ??? void USAMP_SaveConfiguration(FILE *fp);
// QDOAS ???
// QDOAS ??? // ==================
// QDOAS ??? // MESSAGE PROCESSING
// QDOAS ??? // ==================
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? LRESULT CALLBACK MSG_WndProc(HWND hwndMsg,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? RC MSG_MessageBox(HWND hwnd,INT controlID,INT titleID,INT msgID,INT style,...);
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // ========
// QDOAS ??? // PRINTING
// QDOAS ??? // ========
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? void PRINT_Enable(INT flag);
// QDOAS ??? void PRINT_Window(INT cmd);
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // ====
// QDOAS ??? // HELP
// QDOAS ??? // ====
// QDOAS ???
// QDOAS ??? // Constants definitions
// QDOAS ??? // ---------------------
// QDOAS ???
// QDOAS ??? enum _programVersions
// QDOAS ???  {
// QDOAS ???   HELP_VERSION_1_00,  // WINDOAS version 1.0
// QDOAS ???   HELP_VERSION_1_01,  // WINDOAS version 1.1
// QDOAS ???   HELP_VERSION_1_02,  // WINDOAS version 1.2
// QDOAS ???   HELP_VERSION_1_03,  // WINDOAS version 1.3 (January 1999)
// QDOAS ???   HELP_VERSION_1_04,  // WINDOAS version 1.4 (February 1999)
// QDOAS ???   HELP_VERSION_1_05,  // WINDOAS version 1.5 (June 1999)
// QDOAS ???   HELP_VERSION_2_00,  // WINDOAS version 2.0 (Dec 2000)
// QDOAS ???   HELP_VERSION_2_01,  // WINDOAS version 2.01 (Aug. 2000)
// QDOAS ???   HELP_VERSION_2_02,  // WINDOAS version 2.02 (Aug. 2000)
// QDOAS ???   HELP_VERSION_2_03,
// QDOAS ???   HELP_VERSION_2_10,  // WinDOAS version 2.1 (February 2001)
// QDOAS ???   HELP_VERSION_MAX
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? // Global variables
// QDOAS ??? // ----------------
// QDOAS ???
// QDOAS ??? // The following variable holds a trace of WinDOAS versions in order to ensure a
// QDOAS ??? // maximum of compatibility when loading older configuration files
// QDOAS ???
// QDOAS ??? EXTERN DoasCh *HELP_programVersions[HELP_VERSION_MAX];
// QDOAS ???
// QDOAS ??? // Prototypes
// QDOAS ??? // ----------
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? LRESULT CALLBACK HELP_About(HWND hwndAbout,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? #endif
// QDOAS ???
// ===============================
// PATHS INSERTION IN PROJECT TREE
// ===============================

// Constant definition
// -------------------

#define PATH_MAX_FILES   2500

// Structure definition
// --------------------

typedef struct _pathFilesFilter
 {
  DoasCh outputPathShort[MAX_STR_SHORT_LEN+1];
  DoasCh folderNameShort[MAX_STR_SHORT_LEN+1];
  DoasCh filterShort[MAX_STR_SHORT_LEN+1];
  INT   useSubfolders;
 }
PATH_FILES_FILTER;

// Global variables
// ----------------

EXTERN DoasCh (*PATH_fileNamesShort)[MAX_STR_SHORT_LEN+1];
EXTERN DoasCh PATH_fileMax[MAX_STR_SHORT_LEN+1];
EXTERN DoasCh PATH_fileSpectra[MAX_STR_SHORT_LEN+1];        // current spectra file name

EXTERN INT PATH_fileNumber;
EXTERN INT PATH_dirNumber;
EXTERN INT PATH_mfcFlag,PATH_UofTFlag;
// QDOAS ???
// QDOAS ??? // Prototypes
// QDOAS ??? // ----------
// QDOAS ???
// QDOAS ??? void  PATH_GetFilesList(DoasCh *path,DoasCh *filter,DoasCh dirFlag);
// QDOAS ??? INDEX PATH_InsertFolder(DoasCh *path,DoasCh *filter,DoasCh folderFlag,INDEX indexParent);
// QDOAS ???
// QDOAS ??? void  PATH_Init(void);
// QDOAS ??? RC    PATH_Alloc(void);
// QDOAS ??? void  PATH_Free(void);
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? LRESULT CALLBACK PATH_WndProc(HWND hwndPath,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // ======================
// QDOAS ??? // SYS.C : SYSTEM CONTROL
// QDOAS ??? // ======================
// QDOAS ???
// QDOAS ??? // Global variables
// QDOAS ??? // ----------------
// QDOAS ???
// QDOAS ??? EXTERN DoasU32 SYS_memory,
// QDOAS ???              SYS_files,
// QDOAS ???              SYS_raw,
// QDOAS ???              SYS_tree,
// QDOAS ???              SYS_project,
// QDOAS ???              SYS_anlys,
// QDOAS ???              SYS_anlysItems,
// QDOAS ???              SYS_path;
// QDOAS ???
// QDOAS ??? // Prototypes
// QDOAS ??? // ----------
// QDOAS ???
// QDOAS ??? void SYS_Load(void);
// QDOAS ??? void SYS_Save(void);

EXTERN ANALYSIS_WINDOWS  *ANLYS_windowsList;       // analysis windows list
EXTERN LIST_ITEM         *ANLYS_itemList;          // list of items in ListView control owned by tab pages

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

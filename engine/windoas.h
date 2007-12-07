
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

// ==================
// HEADERS TO INCLUDE
// ==================

// Resources IDs

#include "comdefs.h"
#include "constants.h"
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
  SHORT	year;
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
// QDOAS ???   UCHAR        tabTitle[MAX_ITEM_NAME_LEN+1];          // title of tab page
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
// QDOAS ??? RC HDF_GetSDSInfo(UCHAR *fileName,int32 hdfSDSId,int32 sdsRef,HDF_SDS *sdsList,INT nSDS);
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
  UCHAR fileType[MAX_ITEM_DESC_LEN+1];          // type of files
  UCHAR fileExt[12];                            // extension associated to this type of files
  UCHAR defaultPath[MAX_PATH_LEN+1];            // default path
 }
FILE_TYPE;

// Paths for files
// ---------------

typedef struct _filePath
 {
  UCHAR path[MAX_PATH_LEN+1];
  INT   count;
 }
FILES_PATH;

// ----------------
// GLOBAL VARIABLES
// ----------------

EXTERN UCHAR FILES_configuration[];            // configuration file
EXTERN FILE_TYPE FILES_types[];                     // types of files supported by application
EXTERN INT FILES_version;                           // program version
EXTERN FILES_PATH *FILES_paths;                     // all paths implied in configuration file
EXTERN INT FILES_nPaths;                            // the size of the previous buffer

// ----------
// PROTOTYPES
// ----------

// Load data from files
// --------------------

void   FILES_CompactPath(UCHAR *newPath,UCHAR *path,INT useFileName,INT addFlag);
UCHAR *FILES_RebuildFileName(UCHAR *newPath,UCHAR *path,INT useFileName);
void   FILES_ChangePath(UCHAR *oldPath,UCHAR *newPath,INT useFileName);
void   FILES_RemoveOnePath(UCHAR *path);
void   FILES_RetrievePath(UCHAR *pathString,SZ_LEN pathStringLength,UCHAR *fullFileName,SZ_LEN fullFileNameLength,INT indexFileType,INT changeDefaultPath);

RC     FILES_GetMatrixDimensions(FILE *fp,UCHAR *fileName,INT *pNl,INT *pNc,UCHAR *callingFunction,INT errorType);
RC     FILES_LoadMatrix(FILE *fp,UCHAR *fileName,double **matrix,INT base,INT nl,INT nc,UCHAR *callingFunction,INT errorType);

// QDOAS ??? RC     FILES_Alloc(void);
// QDOAS ??? void   FILES_Free(void);

// Select a file
// -------------

UCHAR  *FILES_BuildFileName(UCHAR *fileName,MASK fileType);

// QDOAS ??? #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ??? RC      FILES_Open(HWND hwndParent,MASK fileType,UCHAR *buffer,INT bufferSize,LONG openStyles,UCHAR openMode,INT *symbolReferenceNumber,INDEX indexSymbol,INT *pFileType,INT helpID);
// QDOAS ??? RC      FILES_Select(HWND hwndParent,UCHAR *buffer,INT bufferSize,MASK fileType,INT style,UCHAR openMode,INT *symbolReferenceNumber,INDEX indexSymbol,INT helpID);
// QDOAS ??? INDEX   FILES_Insert(HWND hwndTree,INDEX indexParent,INT dataType,MASK fileType);
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // "Files" menu commands processing
// QDOAS ??? // --------------------------------
// QDOAS ???
// QDOAS ??? void    FILES_LoadConfiguration(HWND hwndParent,UCHAR *fileName);
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

   #define MAX_SITES              60
   #define MAX_SITES_ABBREVIATION  2

   // Structures definitions
   // ----------------------

   // Observation site properties

   typedef struct _observationSites
    {
     UCHAR name[MAX_ITEM_NAME_LEN+1];
     UCHAR abbrev[MAX_SITES_ABBREVIATION+1];
     double longitude;
     double latitude;
     double altitude;
     INT gmtShift;
     INT hidden;
    }
   OBSERVATION_SITE;

   // Global variables
   // ----------------

   EXTERN OBSERVATION_SITE  *SITES_itemList,                                    // pointer to the list of sites objects
                            *SITES_toPaste;                                     // the list of observation sites to paste
   EXTERN INDEX              SITES_treeEntryPoint;                              // entry point in the 'environment space' tree for sites objects

   // Prototypes
   // ----------

// QDOAS ???   #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???       LRESULT CALLBACK SITES_WndProc(HWND hwndSites,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   RC      SITES_Alloc(void);
// QDOAS ???   void    SITES_Free(void);

   INDEX   SITES_GetIndex(UCHAR *siteName);
// QDOAS ???   void    SITES_ResetConfiguration(void);
// QDOAS ???   void    SITES_LoadConfiguration(UCHAR *fileLine);
// QDOAS ???   void    SITES_SaveConfiguration(FILE *fp,UCHAR *sectionName);

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
     UCHAR name[MAX_ITEM_NAME_LEN+1];
     UCHAR description[MAX_ITEM_DESC_LEN+1];
    }
   SYMBOL;

   typedef SYMBOL SYMBOL_CROSS;

   // Global variables
   // ----------------

   EXTERN SYMBOL_CROSS *SYMB_itemCrossList;                                     // pointer to list of cross sections symbols
   EXTERN INT SYMB_itemCrossN;

   INDEX SYMB_GetListIndex(SYMBOL *symbolList,INT symbolNumber,UCHAR *symbolName);
   RC SYMB_Add(UCHAR *symbolName,UCHAR *symbolDescription);

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
// QDOAS ???      UCHAR fileName[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???      UCHAR windoasPath[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???      UCHAR names[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???      UCHAR dark[MAX_ITEM_TEXT_LEN+1];
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
// QDOAS ???    void RAW_LoadConfiguration(UCHAR *fileLine);
// QDOAS ???    void RAW_SaveConfiguration(FILE *fp,UCHAR *sectionName);

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

RC   MATRIX_Allocate(MATRIX_OBJECT *pMatrix,INT nl,INT nc,INT basel,INT basec,INT allocateDeriv2,UCHAR *callingFunction);
void MATRIX_Free(MATRIX_OBJECT *pMatrix,UCHAR *callingFunctionShort);
RC   MATRIX_Copy(MATRIX_OBJECT *pTarget,MATRIX_OBJECT *pSource,UCHAR *callingFunction);
RC   MATRIX_Load(UCHAR *fileName,MATRIX_OBJECT *pMatrix,INT basel,INT basec,INT nl,INT nc,double xmin,double xmax,INT allocateDeriv2,INT reverseFlag,UCHAR *callingFunction);

// ===========================
// ANALYSIS WINDOWS PROPERTIES
// ===========================

// -----------
// DEFINITIONS
// -----------

// Image of the visible lines in ListView
// --------------------------------------

typedef struct _visibleLinesImage
 {
  HWND  *hwnd;                                             // list of controls associated to items in the specified line of ListView control
  INT    indexItem;                                        // index of item in list
 }
VISIBLE_LINES_IMAGE;

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
  UCHAR columnTitle[MAX_ITEM_NAME_LEN+1];                  // title of column in list view
  INT columnWidth;                                         // initial width of the column
  INT columnFormat;                                        // format of column
  UCHAR controlName[20];                                   // name of associated control
  UINT controlStyle;                                       // style of associated control
  UCHAR controlPermanent;                                  // flag set if control is permanent
  INT comboboxItemType;                                    // type of combobox, cfr above
  CHAR displayNumber;                                      // number that specify set of selected columns in list to display in ListView control
  UCHAR defaultValue[MAX_ITEM_NAME_LEN+1];                 // default value
 }
LIST_COLUMN;

// ListView items
// --------------

#define MAX_LIST_ITEMS 500

typedef struct _listItem
 {
  UCHAR  crossFileName[MAX_ITEM_TEXT_LEN+1];               // cross section file associated to the symbol
  UCHAR  amfFileName[MAX_ITEM_TEXT_LEN+1];                 // air mass factors file associated to the symbol
  UCHAR  itemText[MAX_LIST_COLUMNS][MAX_ITEM_TEXT_LEN+1];  // text for items to insert in different columns of ListView control
  INT    hidden;                                           // flag that indicates if the line is hidden in the user interface
  INDEX  indexParent,indexPrevious,indexNext;              // indexes of respectively, parent, previous and next item in list
 }
LIST_ITEM;

typedef struct _anlysTabPages
 {
  UCHAR tabTitle[MAX_ITEM_NAME_LEN+1];                     // title of tab page
  #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
      LIST_COLUMN *columnList;                             // list of columns to create in ListView control
  #endif
  INT columnNumber;                                        // number of columns in previous list
  VISIBLE_LINES_IMAGE *visibleLinesImage;                  // description of lines in ListView
  UCHAR symbolType;                                        // type of symbol to use
  SYMBOL *symbolList;                                      // list of symbols to use
  INT symbolNumber;                                        // number of symbols in previous list
  INT *symbolReferenceNumber;                              // number of times a symbol is referenced to
  INT availableSymbolNumber;                               // number of available symbols
  INT listEntryPoint;                                      // entry point in ListView items list
  INT oldListEntryPoint;                                   // entry point in list before modifications
  INDEX *pTreeEntryPoint;                                  // entry point of analysis window in tree control
  CHAR displayNumber;                                      // number that specify set of selected columns in list to display in ListView control
  UCHAR minDisplayNumber;                                  // minimum value for previous field
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
  UCHAR  windowName[MAX_ITEM_NAME_LEN+1];                   // name of window
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
  UCHAR  refSpectrumFile[MAX_ITEM_TEXT_LEN+1];              // reference spectrum file in file mode
  UCHAR  refEtalon[MAX_ITEM_TEXT_LEN+1];                    // reference etalon file
  UCHAR  residualsFile[MAX_ITEM_TEXT_LEN+1];                // residuals safe keeping
  UCHAR  lambdaMin[13],lambdaMax[13];                       // wavelengths or pixels range
  UCHAR  lambdaMinK[13],lambdaMaxK[13];                     // wavelengths or pixels range
  INDEX  listEntryPoint[TAB_TYPE_ANLYS_MAX];                // entry points to list for all tab pages
  INT    hidden;                                            // flag set if window is hidden
  double refSZA,refSZADelta;
  double refLatMin,refLatMax,refLonMin,refLonMax;
  INT    pixelType;
  INT    nspectra;
  UCHAR  gomePixelType[4];
 }
ANALYSIS_WINDOWS;

// ----------
// PROTOTYPES
// ----------

#if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
    LRESULT CALLBACK ANLYS_WndProc(HWND hwndAnlys,UINT msg,WPARAM mp1,LPARAM mp2);
    void  ANLYS_ViewCrossSections(HWND hwndTree);
    void  ANLYS_DeleteOneListItem(UCHAR *textItem,INDEX indexWindow,INDEX indexTab);
#endif

void  ANLYS_CopyItems(ANALYSIS_WINDOWS *pAnlysToPaste,ANALYSIS_WINDOWS *pAnlysToCopy);
void  ANLYS_PasteItems(ANALYSIS_WINDOWS *pAnlysToPaste,ANALYSIS_WINDOWS *pNewAnlys,INDEX indexParent);
void  ANLYS_ReleaseListItems(INDEX indexWindow);
void  ANLYS_ResetConfiguration(void);
void  ANLYS_LoadConfigurationOld(UCHAR *fileName);
void  ANLYS_OutputConfiguration(FILE *fp,INDEX indexProject,INDEX indexWindow);
void  ANLYS_LoadConfiguration(UCHAR *fileLine);
void  ANLYS_SaveConfiguration(FILE *fp,UCHAR *sectionName);

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
  INT noMin,noMax;                                     // spectra numbers range
  float SZAMin,SZAMax,SZADelta;                        // SZA range
  float longMin,longMax,latMin,latMax,                 // resp. longitude and latitude ranges
        radius;                                        // radius if circle mode is used
  INT   namesFlag,                                     // use names
        darkFlag,                                      // use dark current
        displaySpectraFlag,                            // display complete spectra
        displayDataFlag,                               // display data on spectra
        displayFitFlag,                                // display fits
        displayPause,                                  // QDOAS obsolete field !!! : force pause between two graph display
        displayDelay,                                  // QDOAS obsolete field !!! : calculated delay for display pause
        maxGraphV,                                     // QDOAS obsolete field !!! : maximum number of graphs in height a graphic page can hold
        maxGraphH,                                     // QDOAS obsolete field !!! : maximum number of graphs in width a graphic page can hold
        mode;
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
 }
PRJCT_ANLYS;

// ---------------
// FILTER TAB PAGE
// ---------------

// Filter tab page description
// ---------------------------

typedef struct _prjctFilter
 {
  INT     type;                                          // type of filter
  FLOAT   fwhmWidth;                                     // fwhm width for gaussian
  FLOAT   kaiserCutoff;                                  // cutoff frequency for kaiser filter type
  FLOAT   kaiserPassBand;                                // pass band for kaiser filter type
  FLOAT   kaiserTolerance;                               // tolerance for kaiser filter type
  INT     filterOrder;                                   // filter order
  INT     filterWidth;                                   // filter width for boxcar, triangle or Savitsky-Golay filters
  INT     filterNTimes;                                  // the number of times to apply the filter
  INT     filterAction;
  double *filterFunction;
  INT     filterSize;
  double  filterEffWidth;
  INT     hpFilterCalib;
  INT     hpFilterAnalysis;
 }
PRJCT_FILTER;

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
  UCHAR            file[MAX_ITEM_TEXT_LEN+1];          // kurucz file
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
  UCHAR  kuruczFile[MAX_STR_LEN+1];
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
  UCHAR       sciaReference[4];
 }
PRJCT_SCIA;

typedef struct _prjctInstrumental
 {
  UCHAR       observationSite[MAX_ITEM_NAME_LEN+1];    // index of observation site in list
  UCHAR       readOutFormat;                           // spectra read out format
  INT         user;                                    // user defined
  UCHAR       calibrationFile[MAX_ITEM_TEXT_LEN+1];    // calibration file
  UCHAR       instrFunction[MAX_ITEM_TEXT_LEN+1];      // instrumental function
  UCHAR       vipFile[MAX_ITEM_TEXT_LEN+1];            // interpixel variability correction
  UCHAR       dnlFile[MAX_ITEM_TEXT_LEN+1];            // detector not linearity correction
  INT         detectorSize;                            // size of detector in pixels
  INT         azimuthFlag;
  INT         averageFlag;
  PRJCT_ASCII ascii;
  PRJCT_SAOZ  saoz;
  PRJCT_SCIA  scia;
  INT         wavelength;
  UINT        mfcMaskOffset;
  UINT        mfcMaskDark;
  UINT        mfcMaskInstr;
  UINT        mfcMaskSpec;
  INT         mfcMaskUse;
  INT         mfcMaxSpectra;
  INT         mfcStdOffset;
  INT         mfcRevert;
  float       opusTimeShift;
  UCHAR       fileExt[50];
  float       omiWavelength1,omiWavelength2;
 }
PRJCT_INSTRUMENTAL;

// ----------------------
// SLIT FUNCTION TAB PAGE
// ----------------------

// Description of the slit function

typedef struct _slit
 {
  INT    slitType;                                                              // type of line shape (see above)
  UCHAR  slitFile[MAX_STR_LEN+1];                                                // for line shapes provided in file, name of the file
  double slitParam;                                                             // up to 4 parameters can be provided for the line shape
  double slitParam2;                                                            //       usually, the first one is the FWHM
  double slitParam3;                                                            //       the Voigt profile function uses the 4 parameters
  double slitParam4;
 }
SLIT;

typedef struct _prjctSlit
 {
  SLIT  slitFunction;                                  // slit function
  INT   fwhmCorrectionFlag;                            // flag set if fwhm correction is to be applied
  UCHAR kuruczFile[MAX_STR_LEN+1];
 }
PRJCT_SLIT;

// ----------------
// RESULTS TAB PAGE
// ----------------

// Description of available fields to output
// -----------------------------------------

typedef struct _prjctResultsFields
 {
  UCHAR   fieldName[2*(MAX_ITEM_NAME_LEN+1)];
  INT     fieldType;
  INT     fieldSize;
  INT     fieldDim1,fieldDim2;
  UCHAR   fieldFormat[MAX_ITEM_NAME_LEN+1];
 }
PRJCT_RESULTS_FIELDS;

// ASCII results tab page description
// ----------------------------------

typedef struct _prjctAsciiResults
 {
  UCHAR path[MAX_ITEM_TEXT_LEN+1];                                          // path for results and fits files
  INT   analysisFlag,calibFlag,dirFlag,configFlag,binaryFlag;                             // store results in ascii format
  UCHAR fluxes[MAX_ITEM_TEXT_LEN+1];                                        // fluxes
  UCHAR cic[MAX_ITEM_TEXT_LEN+1];                                           // color indexes
  INT fieldsNumber;                                                         // number of ascii flags set in the next list
  UCHAR fieldsFlag[PRJCT_RESULTS_ASCII_MAX];                                // fields used in ascii format
 }
PRJCT_RESULTS_ASCII;

// Results tab page description
// ----------------------------

typedef struct _prjctNasaResults
 {
  UCHAR path[MAX_ITEM_TEXT_LEN+1];                                          // path for results and fits files
  UCHAR nasaFlag;                                                           // use NASA-AMES format
  UCHAR no2RejectionFlag;                                                   // force NO2 rejection test to be applied
  UCHAR instrument[MAX_ITEM_TEXT_LEN+1];                                    // instrument specification
  UCHAR experiment[MAX_ITEM_TEXT_LEN+1];                                    // experiment specification
  UCHAR fields[PRJCT_RESULTS_NASA_MAX][MAX_ITEM_NAME_LEN+1];                // fields used in NASA-AMES format
 }
PRJCT_RESULTS_NASA;

// --------------------------
// PROPERTY SHEET DESCRIPTION
// --------------------------

#define MAX_PROJECT 10

typedef struct _project
 {
  UCHAR name[MAX_ITEM_NAME_LEN+1];                     // name of window
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
EXTERN UCHAR   *PRJCT_AnlysInterpol[],                 // interpolation methods
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
void  PRJCT_Copy(INDEX indexParent,PROJECT *pProjectToPaste,PROJECT *pProjectToCopy);
void  PRJCT_Paste(HWND hwndTree,INDEX indexParent,PROJECT *pProjectNew,PROJECT *pProjectOld);

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
VOID PRJCT_Free(void);

void PRJCT_OutputConfiguration(FILE *fp,INDEX indexProject);
void PRJCT_LoadFile(UCHAR *fileName);
void PRJCT_ResetConfiguration(void);
void PRJCT_LoadConfiguration(UCHAR *fileLine);
void PRJCT_SaveConfiguration(FILE *fp,UCHAR *sectionName);

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

   // Options of the convolution tool dialog box
   // ------------------------------------------

   typedef struct _xsconv
    {                                                                           // GENERAL OPTIONS
     INT    convolutionType;                                                    // type of convolution
     INT    conversionMode;                                                     // conversion mode
     UCHAR  shift[MAX_ITEM_TEXT_LEN+1];                                         // shift to apply to the original high resolution cross section
     UCHAR  crossFile[MAX_PATH_LEN+1];                                          // high resolution cross section file
     UCHAR  path[MAX_PATH_LEN+1];                                               // output path
     UCHAR  calibrationFile[MAX_PATH_LEN+1];                                    // calibration file
     INT    noComment;                                                          // flag, 1 to save the convoluted cross section without comment

                                                                                // I0 CORRECTION
     UCHAR  kuruczFile[MAX_PATH_LEN+1];                                         // Kurucz file used when I0 correction is applied
     double conc;                                                               // concentration to use when applying I0 correction

                                                                                // SLIT FUNCTION
     SLIT   slitConv;                                                           // convolution slit function
     SLIT   slitDConv;                                                          // deconvolution slit function

                                                                                // FILTERING
     PRJCT_FILTER lfilter;                                                      // low filtering options
     PRJCT_FILTER hfilter;                                                      // high filtering options

                                                                                // UNDERSAMPLING
     INT    analysisMethod;                                                     // analysis method
     UCHAR  path2[MAX_PATH_LEN+1];                                              // output path for the second phase
     double fraction;                                                           // tunes the phase

                                                                                // RING
     INT    temperature;                                                        // temperature
    }
   XSCONV;

   // ----------------
   // GLOBAL VARIABLES
   // ----------------

   EXTERN UCHAR *XSCONV_slitTypes[SLIT_TYPE_MAX];
   EXTERN XSCONV XSCONV_buffer;

   // ----------
   // PROTOTYPES
   // ----------

   // Files processing

   RC   XSCONV_LoadCalibrationFile(XS *pLambda,UCHAR *lambdaFile,INT nextraPixels);
   RC   XSCONV_LoadSlitFunction(XS *pSlitXs,SLIT *pSlit,double *pGaussWidth,INT *pSlitType);
   RC   XSCONV_LoadCrossSectionFile(XS *pCross,UCHAR *crossFile,double lambdaMin,double lambdaMax,double shift,INT conversionMode);

   // Convolution functions

   RC   XSCONV_TypeNone(XS *pXsnew,XS *pXshr);
   RC   XSCONV_TypeGauss(double *lambda,double *Spec,double *SDeriv2,double lambdaj,double dldj,double *SpecConv,double fwhm,double n,INT slitType);
   RC   XSCONV_TypeStandardFFT(FFT *pFFT,INT fwhmType,double slitParam,double slitParam2,double *lambda,double *target,INT size);
   RC   XSCONV_TypeStandard(XS *pXsnew,INDEX indexLambdaMin,INDEX indexLambdaMax,XS *pXshr,XS *pSlit,XS *pI,double *Ic,INT slitType,double slitWidth,double slitParam,double slitParam2,double slitParam3,double slitParam4);
   RC   XSCONV_RealTimeXs(XS *pXshr,XS *pXsI0,XS *pSlit,double *IcVector,double *lambda,INDEX indexLambdaMin,INDEX indexLambdaMax,double *newXs,INT slitType,double slitParam,double slitParam2,double slitParam3,double slitParam4);

   // Buffers allocation

   void XSCONV_Reset(XS *pXsconv);
   RC   XSCONV_Alloc(XS *pXsconv,INT npts,INT deriv2Flag);
// QDOAS ???
// QDOAS ???    // Options in the WinDOAS configuration file
// QDOAS ???
// QDOAS ???    void XSCONV_ResetConfiguration(void);
// QDOAS ???    RC   XSCONV_LoadConfiguration(UCHAR *fileLine);
// QDOAS ???    void XSCONV_SaveConfiguration(FILE *fp);
// QDOAS ???
// QDOAS ???    // Dialog box processing
// QDOAS ???
// QDOAS ???    #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ???        void XSCONV_FileSelection(HWND hwndXsconv,UCHAR *file,MASK fileType,INT fileMode,INT fileCommand,INT ringFlag);
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

#if !defined(__WINDOAS_GUI_) || !(__WINDOAS_GUI_)
typedef LONG HTREEITEM;
#endif

typedef struct _treeItemType
 {
  INT     contextMenu;                      // context (or shortcut) menu to load from resources

  #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
  INT     dlgBox;                           // dialog box to load from resources
  DLGPROC dlgProc;                          // procedure for processing messages from the previous dialog box
  #endif

  INT     childDataType;                    // type of data for children nodes
  VOID   *dataList;                         // pointer to buffer used for storing data associated to children nodes
  VOID   *data2Paste;                       // pointer to the structure storing the data to paste
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
  UCHAR     textItem[MAX_ITEM_TEXT_LEN+1];  // complete item text
  UCHAR     newItem;                        // flag set for a new item
  INDEX     parentItem;                     // index of parent item
  INDEX     firstChildItem;                 // index of the first child in list
  INDEX     lastChildItem;                  // index of the first child in list
  INDEX     prevItem;                       // index of previous item in list
  INDEX     nextItem;                       // index of the next item in list
  HTREEITEM hti;                            // handle of the item in tree
  INT       dataType;                       // type of data
  INDEX     dataIndex;                      // index of data in data list referenced by data type
  INT       useCount;                       // the number of times the item is referenced
  INT       childNumber;                    // total number of children
  INT       childHidden;                    // total number of hidden children
  UCHAR     hidden;                         // flag set if item is hidden
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
// QDOAS ??? EXTERN UCHAR           TREE_editFlag;       // flag set when editing a tree item label
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
// QDOAS ??? INDEX   TREE_GetIndexByDataName(UCHAR *dataName,UCHAR dataType,INDEX entryPoint);
INDEX   TREE_GetIndexByDataIndex(INDEX dataIndex,UCHAR dataType,INDEX entryPoint);
// QDOAS ??? INT     TREE_GetIndexesByDataType(UCHAR dataType,INDEX entryPoint,INDEX *dataIndexes,INT *pDataNumber);
// QDOAS ??? void    TREE_UpdateItem(HWND hwndTree,INDEX indexItem);
// QDOAS ??? void    TREE_Reset(INDEX indexParent);
// QDOAS ??? INDEX   TREE_GetSelectedItem(HWND hwndTree);
// QDOAS ??? INDEX   TREE_DeleteOneItem(INDEX indexItemToDelete);
// QDOAS ??? INDEX   TREE_InsertOneItem(HWND hwndTree,UCHAR *textItem,INDEX parentItem,INT dataType,UCHAR newItem,UCHAR folderFlag,UCHAR hidden);
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
// QDOAS ???          UCHAR legend[MAX_ITEM_TEXT_LEN+1];
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
// QDOAS ???          UCHAR title[MAX_ITEM_TEXT_LEN+1],      // title of graph
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
// QDOAS ???        void  DRAW_Spectra(INT indexChild,UCHAR *windowTitle,UCHAR *graphTitle,UCHAR *xTitle,UCHAR *yTitle,double *vGrid,INT nvGrid,
// QDOAS ???                           double xMin,double xMax,double yMin,double yMax,
// QDOAS ???                           double *x,double *y,INT size,COLORREF colour,INDEX indexMin,INDEX indexMax,INT lineType,UCHAR *legend,
// QDOAS ???                           double *dotx,double *doty,INT dotSize,COLORREF dotColour,INDEX dotIndexMin,INDEX dotIndexMax,INT dotLineType,UCHAR *dotLegend,
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
// QDOAS ???          UCHAR title[MAX_ITEM_TEXT_LEN+1];                                         // title of mdi child window
// QDOAS ???          UCHAR graphNumber;                                                        // number of graph to draw in the draw client area
// QDOAS ???          UCHAR graphMax;                                                           // total number of graphs (graphMaxV*graphMaxH)
// QDOAS ???          UCHAR graphMaxV;                                                          // total number of graphs to hold vertically in the draw client area
// QDOAS ???          UCHAR graphMaxH;                                                          // total number of graphs to hold horizontally in the draw client area
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
// QDOAS ???    void    CHILD_LoadConfiguration(UCHAR *fileLine);
// QDOAS ???    void    CHILD_SaveConfiguration(FILE *fp,UCHAR *sectionName);
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
// QDOAS ??? // =================================
// QDOAS ??? // BITMAPS SAVE AND PRINT PROCESSING
// QDOAS ??? // =================================
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? void DIB_SaveSpectra(INDEX indexWindow);
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // ================
// QDOAS ??? // RING EFFECT TOOL
// QDOAS ??? // ================
// QDOAS ???
// QDOAS ??? enum _ringTypes
// QDOAS ???  {
// QDOAS ???   RING_TYPE_SOLAR,
// QDOAS ???   RING_TYPE_MOLECULAR,
// QDOAS ???   RING_TYPE_MAX
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? LRESULT CALLBACK RING_WndProc(HWND hwndRing,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? LRESULT CALLBACK RING_ProgressWndProc(HWND hwndProgress,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? void RING_ResetConfiguration(void);
// QDOAS ??? RC   RING_LoadConfiguration(UCHAR *fileLine);
// QDOAS ??? void RING_SaveConfiguration(FILE *fp);
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
// QDOAS ??? RC   USAMP_LoadConfiguration(UCHAR *fileLine);
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
// QDOAS ??? EXTERN UCHAR *HELP_programVersions[HELP_VERSION_MAX];
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
  UCHAR outputPathShort[MAX_STR_SHORT_LEN+1];
  UCHAR folderNameShort[MAX_STR_SHORT_LEN+1];
  UCHAR filterShort[MAX_STR_SHORT_LEN+1];
  INT   useSubfolders;
 }
PATH_FILES_FILTER;

// Global variables
// ----------------

EXTERN UCHAR (*PATH_fileNamesShort)[MAX_STR_SHORT_LEN+1];
EXTERN UCHAR PATH_fileMax[MAX_STR_SHORT_LEN+1];
EXTERN UCHAR PATH_fileSpectra[MAX_STR_SHORT_LEN+1];        // current spectra file name

EXTERN INT PATH_fileNumber;
EXTERN INT PATH_dirNumber;
EXTERN INT PATH_mfcFlag,PATH_UofTFlag;
// QDOAS ???
// QDOAS ??? // Prototypes
// QDOAS ??? // ----------
// QDOAS ???
// QDOAS ??? void  PATH_GetFilesList(UCHAR *path,UCHAR *filter,UCHAR dirFlag);
// QDOAS ??? INDEX PATH_InsertFolder(UCHAR *path,UCHAR *filter,UCHAR folderFlag,INDEX indexParent);
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
// QDOAS ??? EXTERN ULONG SYS_memory,
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

#endif


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

#define WINDOAS_HLP                 "WINDOAS.HLP"       // help file
#define WINDOAS_LOG                 "WINDOAS.LOG"       // log file
#define WINDOAS_DBG                 "WINDOAS.DBG"       // debug file
#define WINDOAS_TMP                 "WINDOAS.TMP"       // temporary file
#define WINDOAS_SYS                 "WINDOAS.SYS"       // system file

#define BRO_AMF_FILE                "bro_coef352-369nm.dat"

#define ERROR_TYPE_TIME                        0x10
#define ERROR_TYPE_OWNERDRAWN                  0x20

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
// QDOAS ???
// QDOAS ??? // ====================
// QDOAS ??? // RESOURCES MANAGEMENT
// QDOAS ??? // ====================
// QDOAS ???
// QDOAS ??? RC      RESOURCE_Alloc(void);
// QDOAS ??? void    RESOURCE_Free(void);
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ???
// QDOAS ??? // ===================
// QDOAS ??? // TOOL BAR PROCESSING
// QDOAS ??? // ===================
// QDOAS ???
// QDOAS ??? // Constant
// QDOAS ??? // --------
// QDOAS ???
// QDOAS ??? #define TLBAR_ID               90001L             // toolbar id
// QDOAS ??? #define TLBAR_TIMER_ID         90002L             // toolbar timer id
// QDOAS ??? #define TLBAR_TIMER_VALUE        100L             // accuracy in ms of the toolbar timer
// QDOAS ???
// QDOAS ??? // Global variables
// QDOAS ??? // ----------------
// QDOAS ???
// QDOAS ??? EXTERN HWND TLBAR_hwnd;             // toolbar handle for windows messages processing
// QDOAS ??? EXTERN HWND TLBAR_hwndGoto;
// QDOAS ??? EXTERN BOOL TLBAR_bSaveFlag;        // "change status", indicates at any time if the current configuration should be saved
// QDOAS ??? EXTERN BOOL TLBAR_bThreadFlag;      // "change status", indicates at any time if the program is in thread
// QDOAS ???
// QDOAS ??? // Prototypes
// QDOAS ??? // ----------
// QDOAS ???
// QDOAS ??? void    TLBAR_Init(void);
// QDOAS ??? void    TLBAR_Close(void);
// QDOAS ??? BOOL    TLBAR_Create(HWND hwndParent);
// QDOAS ??? LRESULT TLBAR_ReSize(HWND hwnd,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? LRESULT TLBAR_Notify(HWND hwnd,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? void    TLBAR_EnableThreadButtons(BOOL bFlag);
// QDOAS ??? void    TLBAR_Enable(BOOL bFlag);
// QDOAS ??? RC      TLBAR_Check(HWND hwndParent);
// QDOAS ??? void    TLBAR_Message(UINT msg);
// QDOAS ???
// QDOAS ??? // =====================
// QDOAS ??? // STATUS BAR PROCESSING
// QDOAS ??? // =====================
// QDOAS ???
// QDOAS ??? // Constants definitions
// QDOAS ??? // ---------------------
// QDOAS ???
// QDOAS ??? #define STBAR_TIMER_VALUE        60000L           // accuracy in ms for updating time in status bar
// QDOAS ??? #define STBAR_MSG_ID             90011L           // status bar id
// QDOAS ??? #define STBAR_TIMER_ID           90012L           // id of the timer called for updating time in status bar
// QDOAS ???
// QDOAS ??? #define STBAR_SECTION_TEXT       0                // section of the status bar for contextual help
// QDOAS ??? #define STBAR_SECTION_MOUSEPOS   1                // section of the status bar for graph coordinates
// QDOAS ??? #define STBAR_SECTION_TIME       2                // section of the status bar for time
// QDOAS ???
// QDOAS ??? // Global variable
// QDOAS ??? // ---------------
// QDOAS ???
// QDOAS ??? EXTERN  HWND STBAR_hwnd;            // status bar handle for Windows messages processing
// QDOAS ???
// QDOAS ??? // Prototypes
// QDOAS ??? // ----------
// QDOAS ???
// QDOAS ??? BOOL    STBAR_Create(HWND hwndParent);
// QDOAS ??? LRESULT STBAR_Timer (HWND hwnd,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? LRESULT STBAR_ReSize(HWND hwnd,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? LRESULT STBAR_Update(HWND hwnd,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ???
// QDOAS ??? #endif
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

// Common dialog open mode
// -----------------------

enum _filesOpenMode
 {
  FILE_MODE_OPEN,
  FILE_MODE_SAVE
 };

// All supported types of files
// ----------------------------

enum _filesTypes
 {
  FILE_TYPE_ALL,                                  // all files (*.*)
  FILE_TYPE_ASCII,                                // ASCII files (*.asc)
  FILE_TYPE_ASCII_SPECTRA,                        // ASCII spectra files (*.asc)
  FILE_TYPE_SPECTRA,                              // spectra files (*.spe)
  FILE_TYPE_CROSS,                                // cross sections files (*.xs)
  FILE_TYPE_REF,                                  // reference spectra files (*.ref)
  FILE_TYPE_AMF_SZA,                              // SZA dependent AMF files (*.amf_sza)
  FILE_TYPE_AMF_CLI,                              // Climatology dependent AMF files (*.amf_cli)
  FILE_TYPE_AMF_WVE,                              // Wavelength dependent AMF files (*.amf_wve)
  FILE_TYPE_NAMES,                                // spectra names files (*.nms)
  FILE_TYPE_DARK,                                 // dark current files (*.drk)
  FILE_TYPE_INTERPIXEL,                           // interpixel variability files (*.vip)
  FILE_TYPE_NOT_LINEARITY,                        // detector not linearity files (*.dnl)
  FILE_TYPE_CALIB,                                // calibration files (*.clb)
  FILE_TYPE_CALIB_KURUCZ,                         // Kurucz files (*.ktz)
  FILE_TYPE_SLIT,                                 // slit function (*.slf)
  FILE_TYPE_INSTR,                                // instrumental function (*.ins)
  FILE_TYPE_FILTER,                               // filter files (*.flt)
  FILE_TYPE_FIT,                                  // fits files (*.fit*)
  FILE_TYPE_INI,                                  // WINDOAS settings (*.wds)
  FILE_TYPE_BMP,                                  // bitmap files (*.bmp)
  FILE_TYPE_RES,                                  // residuals (*.res)
  FILE_TYPE_PATH,                                 // paths
  FILE_TYPE_CFG,                                  // config
  FILE_TYPE_MAX
 };

// Individual spectra safe keeping
// -------------------------------

enum _filesTypesSpectra
 {
  FILE_TYPE_SPECTRA_COMMENT,
  FILE_TYPE_SPECTRA_NOCOMMENT,
  FILE_TYPE_SPECTRA_MAX
 };

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

   enum _predefinedSymbols
    {
     SYMBOL_PREDEFINED_SPECTRUM,                                                // spectrum
     SYMBOL_PREDEFINED_REF,                                                     // reference
     SYMBOL_PREDEFINED_COM,                                                     // common residual
     SYMBOL_PREDEFINED_USAMP1,                                                  // undersampling phase 1
     SYMBOL_PREDEFINED_USAMP2,                                                  // undersampling phase 2
     SYMBOL_PREDEFINED_RING1,                                                   // different cross sections for ring
     SYMBOL_PREDEFINED_MAX
    };

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

   EXTERN SYMBOL_CROSS *SYMB_itemCrossList;                    // pointer to list of cross sections symbols
   EXTERN INDEX SYMB_crossTreeEntryPoint;                      // entry point in tree for cross sections symbols objects

// QDOAS ???
// QDOAS ???    // Prototypes
// QDOAS ???    // ----------
// QDOAS ???
// QDOAS ???    #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???        LRESULT CALLBACK SYMB_WndProc(HWND hwndSites,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ???    #endif
// QDOAS ???
   INDEX SYMB_GetListIndex(SYMBOL *symbolList,INT symbolNumber,UCHAR *symbolName);
// QDOAS ???
// QDOAS ???    RC    SYMB_Alloc(void);
// QDOAS ???    void  SYMB_Free(void);
// QDOAS ???
// QDOAS ???    void  SYMB_ResetConfiguration(void);
// QDOAS ???    void  SYMB_LoadConfiguration(UCHAR *fileName);
// QDOAS ???    void  SYMB_SaveConfiguration(FILE *fp,UCHAR *sectionName);
// QDOAS ???
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

enum _orthogonalTypes
 {
  ANLYS_ORTHOGONAL_TYPE_NONE,
  ANLYS_ORTHOGONAL_TYPE_BASE,
  ANLYS_ORTHOGONAL_TYPE_MAX
 };

enum _polynomeTypes
 {
 	ANLYS_POLY_TYPE_NONE,
 	ANLYS_POLY_TYPE_0,
 	ANLYS_POLY_TYPE_1,
 	ANLYS_POLY_TYPE_2,
 	ANLYS_POLY_TYPE_3,
 	ANLYS_POLY_TYPE_4,
 	ANLYS_POLY_TYPE_5,
 	ANLYS_POLY_TYPE_MAX
 };

enum _stretchTypes
 {
  ANLYS_STRETCH_TYPE_NONE,
  ANLYS_STRETCH_TYPE_FIRST_ORDER,
  ANLYS_STRETCH_TYPE_SECOND_ORDER,
  ANLYS_STRETCH_TYPE_MAX
 };

enum _crossTypes
 {
  ANLYS_CROSS_ACTION_NOTHING,
  ANLYS_CROSS_ACTION_INTERPOLATE,
  ANLYS_CROSS_ACTION_CONVOLUTE,
  ANLYS_CROSS_ACTION_CONVOLUTE_I0,
  ANLYS_CROSS_ACTION_CONVOLUTE_RING,
  ANLYS_CROSS_ACTION_MAX
 };

enum _amfTypes
 {
  ANLYS_AMF_TYPE_NONE,
  ANLYS_AMF_TYPE_SZA,
  ANLYS_AMF_TYPE_CLIMATOLOGY,
  ANLYS_AMF_TYPE_WAVELENGTH1,
  ANLYS_AMF_TYPE_WAVELENGTH2,
  ANLYS_AMF_TYPE_WAVELENGTH3,
  ANLYS_AMF_TYPE_MAX
 };

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

// Columns enumeration per tab page
// --------------------------------

// Columns description of the ListView in "cross sections" tab page

enum _crossSectionsColumns
 {
  // Cross Sections

  COLUMN_CROSS_FILE,              // Cross Sections
  COLUMN_CROSS_ORTHOGONAL,        // Orthogonalization
  COLUMN_CROSS_XS_TYPE,           // Cross section type
  COLUMN_CROSS_AMF_TYPE,          // AMF dependence to use
  COLUMN_CROSS_DISPLAY,           // Fit display
  COLUMN_CROSS_FILTER,            // Filter flag
  COLUMN_CROSS_CCFIT,             // Cc fit
  COLUMN_CROSS_CCINIT,            // Cc init
  COLUMN_CROSS_CCDELTA,           // Cc delta

  // Output

  COLUMN_CROSS_AMF_OUTPUT,        // Store AMF
  COLUMN_CROSS_RESIDUAL,          // Res Col
  COLUMN_CROSS_SLNTCOL,           // Slnt Col
  COLUMN_CROSS_SLNTERR,           // Slnt Err
  COLUMN_CROSS_SLNTFACT,          // Slnt Fact
  COLUMN_CROSS_VRTCOL,            // Vrt Col
  COLUMN_CROSS_VRTERR,            // Vrt Err
  COLUMN_CROSS_VRTFACT,           // Vrt Fact

  // Kurucz SVD matrix composition for reference alignment

  COLUMN_CROSS_SVD_ORTHOGONAL,    // Orthogonalization
  COLUMN_CROSS_SVD_DISPLAY,       // Fit display
  COLUMN_CROSS_SVD_XS_TYPE,       // Cross section type
  COLUMN_CROSS_SVD_CCFIT,         // Cc fit
  COLUMN_CROSS_SVD_CCINIT,        // Cc init
  COLUMN_CROSS_SVD_CCDELTA,       // Cc delta

  // Kurucz output

  COLUMN_CROSS_SVD_SLNTCOL,       // Slnt Col
  COLUMN_CROSS_SVD_SLNTERR,       // Slnt Err
  COLUMN_CROSS_SVD_SLNTFACT,      // Slnt Fact

  // I0 convolution

  COLUMN_CROSS_CCI0,              // Cc I0 convolution
  COLUMN_CROSS_SVD_CCI0,          // Cc I0 convolution (calibration)

  COLUMN_CROSS_MAX
 };

// Columns description of the ListView in "continuous functions" tab page

enum _continuousFunctionsColumns
 {
  COLUMN_CONTINUOUS_X0,           // terms in x0 and 1/x0
  COLUMN_CONTINUOUS_X1,           // terms in x1 and 1/x1
  COLUMN_CONTINUOUS_X2,           // terms in x2 and 1/x2
  COLUMN_CONTINUOUS_X3,           // terms in x3 and 1/x3
  COLUMN_CONTINUOUS_X4,           // terms in x4 and 1/x4
  COLUMN_CONTINUOUS_X5,           // terms in x5 and 1/x5
  COLUMN_CONTINUOUS_MAX,          // terms in x0 and 1/x0
 };

// Columns description of the ListView in "linear parameters" tab page

enum _linearColumns
 {
  COLUMN_POLY_PARAMS,        // Linear params
  COLUMN_POLY_POLYORDER,     // Polynomial order
  COLUMN_POLY_BASEORDER,     // Orthogonal base order
  COLUMN_POLY_STORE_FIT,     // Store fit
  COLUMN_POLY_STORE_ERROR,   // Store error
  COLUMN_POLY_MAX
 };

// Columns description of the ListView in "non linear parameters" tab page

enum _notLinearColumns
 {
  COLUMN_OTHERS_PARAMS,        // Non linear params
  COLUMN_OTHERS_FIT,           // Fit flag
  COLUMN_OTHERS_VALINIT,       // Val init
  COLUMN_OTHERS_VALDELTA,      // Val del
  COLUMN_OTHERS_STORE_FIT,     // Store fit
  COLUMN_OTHERS_STORE_ERROR,   // Store error
  COLUMN_OTHERS_VALMIN,        // Val min
  COLUMN_OTHERS_VALMAX,        // Val max
  COLUMN_OTHERS_MAX
 };

// Columns description of the ListView in "Shift and Stretch" tab page

enum _shiftStretchColumns
 {
  COLUMN_CROSS_TO_FIT,            // Cross Sections and spectrum
  COLUMN_SH_FIT,                  // Sh fit
  COLUMN_ST_FIT,                  // St fit
  COLUMN_SC_FIT,                  // Sc fit
  COLUMN_SH_STORE,                // Sh store
  COLUMN_ST_STORE,                // St store
  COLUMN_SC_STORE,                // Sc store
  COLUMN_ERR_STORE,               // Err store
  COLUMN_SH_INIT,                 // Sh init
  COLUMN_ST_INIT,                 // St init
  COLUMN_ST_INIT2,                // St init
  COLUMN_SC_INIT,                 // St init
  COLUMN_SC_INIT2,                // St init
  COLUMN_SH_DELTA,                // Sh delta
  COLUMN_ST_DELTA,                // St delta
  COLUMN_ST_DELTA2,               // St delta
  COLUMN_SC_DELTA,                // St delta
  COLUMN_SC_DELTA2,               // St delta
  COLUMN_SH_MIN,                  // Sh min
  COLUMN_SH_MAX,                  // Sh max
  COLUMN_SHST_MAX
 };

// Columns description of the ListView in "gaps" tab page

enum _gapsColumns
 {
  COLUMN_GAPS_TEXT,
  COLUMN_GAPS_MINVAL,             // Min Value
  COLUMN_GAPS_MAXVAL,             // Max Value
  COLUMN_GAPS_MAX
 };

// Analysis tab pages description
// ------------------------------

enum anlysTabPagesTypes
 {
  TAB_TYPE_ANLYS_CROSS,
  TAB_TYPE_ANLYS_LINEAR,
  TAB_TYPE_ANLYS_NOTLINEAR,
  TAB_TYPE_ANLYS_SHIFT_AND_STRETCH,
  TAB_TYPE_ANLYS_GAPS,
  TAB_TYPE_ANLYS_OUTPUT,
  TAB_TYPE_ANLYS_MAX
 };

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

enum _refSpectrumSelectionMode
 {
  ANLYS_REF_SELECTION_MODE_AUTOMATIC,                      // automatic selection from spectra files
  ANLYS_REF_SELECTION_MODE_FILE                            // reference spectrum in a specific file
 };

enum _kuruczMode
 {
  ANLYS_KURUCZ_NONE,
  ANLYS_KURUCZ_REF,
  ANLYS_KURUCZ_SPEC,
  ANLYS_KURUCZ_REF_AND_SPEC,
  ANLYS_KURUCZ_MAX
 };

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
  UCHAR  lembdaMin[13],lembdaMax[13];                       // wavelengths or pixels range
  UCHAR  lembdaMinK[13],lembdaMaxK[13];                     // wavelengths or pixels range
  INDEX  listEntryPoint[TAB_TYPE_ANLYS_MAX];                // entry points to list for all tab pages
  INT    hidden;                                            // flag set if window is hidden
  double refSZA,refSZADelta;
  double refLatMin,refLatMax,refLonMin,refLonMax;
  INT    pixelType;
  INT    nspectra;
  UCHAR  gomePixelType[4];
 }
ANALYSIS_WINDOWS;

// ------------------
// GLOBAL DEFINITIONS
// ------------------

EXTERN ANLYS_TAB_PAGE     ANLYS_tabPages[];        // tab pages description
EXTERN ANALYSIS_WINDOWS  *ANLYS_windowsList,       // analysis windows list
                         *ANLYS_toPaste;
EXTERN LIST_ITEM         *ANLYS_itemList;          // list of items in ListView control owned by tab pages
EXTERN UCHAR             *ANLYS_amf[];             // types of air mass factors files used for output
EXTERN UCHAR             *ANLYS_crossAction[];     // types of cross sections files
EXTERN INT                ANLYS_nItemsToPaste;

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

enum _prjctSpectraModes
 {
  PRJCT_SPECTRA_MODES_CIRCLE,
  PRJCT_SPECTRA_MODES_RECTANGLE,
  PRJCT_SPECTRA_MODES_OBSLIST,
  PRJCT_SPECTRA_MODES_MAX
 };

typedef struct _prjctSpectra
 {
  INT noMin,noMax,                                     // spectra numbers range
      pixMin,pixMax;                                   // pixels numbers range
  float SZAMin,SZAMax,SZADelta;                        // SZA range
  float longMin,longMax,latMin,latMax,                 // resp. longitude and latitude ranges
        radius;                                        // radius if circle mode is used
  INT   namesFlag,                                     // use names
        darkFlag,                                      // use dark current
        displaySpectraFlag,                            // display complete spectra
        displayDataFlag,                               // display data on spectra
        displayFitFlag,                                // display fits
        displayPause,                                  // force pause between two graph display
        displayDelay,                                  // calculated delay for display pause
        maxGraphV,                                     // maximum number of graphs in height a graphic page can hold
        maxGraphH,                                     // maximum number of graphs in width a graphic page can hold
        mode,
        indexBand;
 }
PRJCT_SPECTRA;

// -----------------
// ANALYSIS TAB PAGE
// -----------------

// Analysis methods
// ----------------

enum prjctAnlysMethod
 {
  PRJCT_ANLYS_METHOD_SVD,                              // Optical thickness fitting (SVD)
  PRJCT_ANLYS_METHOD_SVDMARQUARDT,                     // Intensity fitting (Marquardt-Levenberg+SVD)
  PRJCT_ANLYS_METHOD_MAX
 };

// Least-squares fit weighting
// ---------------------------

enum prjctAnlysFitWeighting
 {
  PRJCT_ANLYS_FIT_WEIGHTING_NONE,                      // no weighting
  PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL,              // instrumental weighting
  PRJCT_ANLYS_FIT_WEIGHTING_MAX
 };

// Units for shift and stretch
// ---------------------------

enum prjctAnlysUnits
 {
  PRJCT_ANLYS_UNITS_PIXELS,                            // pixels
  PRJCT_ANLYS_UNITS_NANOMETERS,                        // nanometers
  PRJCT_ANLYS_UNITS_MAX
 };

// Interpolation
// -------------

enum prjctAnlysInterpol
 {
  PRJCT_ANLYS_INTERPOL_LINEAR,                         // linear interpolation
  PRJCT_ANLYS_INTERPOL_SPLINE,                         // spline interpolation
  PRJCT_ANLYS_INTERPOL_MAX
 };

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

// Filter types
// ------------

enum prjctFilterTypes
 {
  PRJCT_FILTER_TYPE_NONE,                              // use no filter
  PRJCT_FILTER_TYPE_KAISER,                            // kaiser filter
  PRJCT_FILTER_TYPE_BOXCAR,                            // box car filter
  PRJCT_FILTER_TYPE_GAUSSIAN,                          // gaussian filter
  PRJCT_FILTER_TYPE_TRIANGLE,                          // triangular filter
  PRJCT_FILTER_TYPE_SG,                                // savitzky-Golay filter
  PRJCT_FILTER_TYPE_ODDEVEN,                           // odd-even pixel correction
  PRJCT_FILTER_TYPE_BINOMIAL,                          // binomial filter
  PRJCT_FILTER_TYPE_MAX
 };

// Filter action
// -------------

enum prjctFilterOutput
 {
  PRJCT_FILTER_OUTPUT_LOW,
  PRJCT_FILTER_OUTPUT_HIGH_SUB,
  PRJCT_FILTER_OUTPUT_HIGH_DIV,
  PRJCT_FILTER_OUTPUT_MAX
 };

enum prjctFilterFields
 {
  PRJCT_FILTER_FIELD_TYPE,
  PRJCT_FILTER_FIELD_CUTOFF_TEXT,
  PRJCT_FILTER_FIELD_CUTOFF,
  PRJCT_FILTER_FIELD_PASS_TEXT,
  PRJCT_FILTER_FIELD_PASS,
  PRJCT_FILTER_FIELD_TOLERANCE_TEXT,
  PRJCT_FILTER_FIELD_TOLERANCE,
  PRJCT_FILTER_FIELD_FWHM_TEXT,
  PRJCT_FILTER_FIELD_FWHM,
  PRJCT_FILTER_FIELD_WIDTH,
  PRJCT_FILTER_FIELD_PIXELS,
  PRJCT_FILTER_FIELD_ORDER_TEXT,
  PRJCT_FILTER_FIELD_ORDER,
  PRJCT_FILTER_FIELD_ORDER_EVEN,
  PRJCT_FILTER_FIELD_NITER_TEXT,
  PRJCT_FILTER_FIELD_NITER
 };

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

enum _prjctFwhmTypes
 {
  PRJCT_CALIB_FWHM_TYPE_GAUSS,
  PRJCT_CALIB_FWHM_TYPE_ERF,
  PRJCT_CALIB_FWHM_TYPE_INVPOLY,
  PRJCT_CALIB_FWHM_TYPE_VOIGT,
  PRJCT_CALIB_FWHM_TYPE_MAX
 };

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
  double           lembdaLeft;
  double           lembdaRight;
  INT              invPolyDegree;
  INT              analysisMethod;
 }
PRJCT_KURUCZ;

// ----------------------
// UNDERSAMPLING TAB PAGE
// ----------------------

enum _prjctUsampMethod
 {
  PRJCT_USAMP_FILE,                                    // no undersampling fitting
  PRJCT_USAMP_FIXED,                                   // undersampling fitting, fixed phase
  PRJCT_USAMP_AUTOMATIC,                               // undersampling fitting, automatic phase
  PRJCT_USAMP_MAX
 };

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

// type of read out format
// -----------------------

enum _prjctInstrFormat
 {
  PRJCT_INSTR_FORMAT_ASCII,                            // ASCII
  PRJCT_INSTR_FORMAT_LOGGER,                           // Logger (PDA,CCD or HAMAMATSU)
  PRJCT_INSTR_FORMAT_ACTON,                            // Acton (NILU)
  PRJCT_INSTR_FORMAT_PDAEGG,                           // PDA EG&G (sept. 94 until now)
  PRJCT_INSTR_FORMAT_PDAEGG_OLD,                       // PDA EG&G (spring 94)
  PRJCT_INSTR_FORMAT_PDAEGG_ULB,                       // PDA EG&G (ULB)
  PRJCT_INSTR_FORMAT_CCD_OHP_96,                       // CCD (OHP 96)
  PRJCT_INSTR_FORMAT_CCD_HA_94,                        // CCD (HARESTUA 94)
  PRJCT_INSTR_FORMAT_CCD_ULB,                          // CCD (ULB)
  PRJCT_INSTR_FORMAT_SAOZ_VIS,                         // SAOZ visible
  PRJCT_INSTR_FORMAT_SAOZ_UV,                          // SAOZ UV
  PRJCT_INSTR_FORMAT_SAOZ_EFM,                         // SAOZ EFM (1024)
  PRJCT_INSTR_FORMAT_MFC,                              // MFC Heidelberg
  PRJCT_INSTR_FORMAT_MFC_STD,                          // MFC Heidelberg
  PRJCT_INSTR_FORMAT_RASAS,                            // RASAS (INTA)
  PRJCT_INSTR_FORMAT_PDASI_EASOE,                       // EASOE
  PRJCT_INSTR_FORMAT_PDASI_OSMA,                       // PDA SI (OSMA)
  PRJCT_INSTR_FORMAT_CCD_EEV,                          // CCD EEV
  PRJCT_INSTR_FORMAT_OPUS,                             // FOURIER,OPUS format
  PRJCT_INSTR_FORMAT_GDP_ASCII,                        // GOME GDP ASCII format
  PRJCT_INSTR_FORMAT_GDP_BIN,                          // GOME GDP BINARY format
  PRJCT_INSTR_FORMAT_SCIA_HDF,                         // SCIAMACHY Calibrated Level 1 data in HDF format
  PRJCT_INSTR_FORMAT_SCIA_PDS,                         // SCIAMACHY Calibrated Level 1 data in PDS format
  PRJCT_INSTR_FORMAT_UOFT,                             // University of Toronto
  PRJCT_INSTR_FORMAT_NOAA,                             // NOAA
  PRJCT_INSTR_FORMAT_OMI,                              // OMI
  PRJCT_INSTR_FORMAT_GOME2,                            // GOME2
  PRJCT_INSTR_FORMAT_MAX
 };

enum _ulbCurveTypes
 {
  PRJCT_INSTR_ULB_TYPE_MANUAL,
  PRJCT_INSTR_ULB_TYPE_HIGH,
  PRJCT_INSTR_ULB_TYPE_LOW,
  PRJCT_INSTR_ULB_TYPE_MAX
 };

enum _saozSpectrumTypes
 {
  PRJCT_INSTR_SAOZ_TYPE_ZENITHAL,
  PRJCT_INSTR_SAOZ_TYPE_POINTED,
  PRJCT_INSTR_SAOZ_TYPE_MAX
 };

enum _iasbSpectrumTypes
 {
  PRJCT_INSTR_IASB_TYPE_ALL,
  PRJCT_INSTR_IASB_TYPE_ZENITHAL,
  PRJCT_INSTR_IASB_TYPE_OFFAXIS,
  PRJCT_INSTR_IASB_TYPE_MAX
 };

enum _niluFormatTypes
 {
  PRJCT_INSTR_NILU_FORMAT_OLD,
  PRJCT_INSTR_NILU_FORMAT_NEW,
  PRJCT_INSTR_NILU_FORMAT_MAX
 };

enum _asciiFormat
 {
  PRJCT_INSTR_ASCII_FORMAT_LINE,
  PRJCT_INSTR_ASCII_FORMAT_COLUMN
 };

enum _gdpBandTypes
 {
  PRJCT_INSTR_GDP_BAND_1A,
  PRJCT_INSTR_GDP_BAND_1B,
  PRJCT_INSTR_GDP_BAND_2A,
  PRJCT_INSTR_GDP_BAND_2B,
  PRJCT_INSTR_GDP_BAND_3,
  PRJCT_INSTR_GDP_BAND_4,
  PRJCT_INSTR_GDP_BAND_MAX
 };

enum _sciaChannels
 {
  PRJCT_INSTR_SCIA_CHANNEL_1,
  PRJCT_INSTR_SCIA_CHANNEL_2,
  PRJCT_INSTR_SCIA_CHANNEL_3,
  PRJCT_INSTR_SCIA_CHANNEL_4,
  PRJCT_INSTR_SCIA_CHANNEL_MAX
 };

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
  INT   format,szaSaveFlag,azimSaveFlag,elevSaveFlag,timeSaveFlag,dateSaveFlag,lembdaSaveFlag;
 }
PRJCT_ASCII;

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
  INT         wavelength;
  INT         sciaChannel;
  INT         sciaCluster[6];
  UCHAR       sciaReference[4];
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

// Possible fields to store in ascii format
// ----------------------------------------

enum _prjctResultsAscii
 {
  PRJCT_RESULTS_ASCII_SPECNO,
  PRJCT_RESULTS_ASCII_NAME,
  PRJCT_RESULTS_ASCII_DATE_TIME,
  PRJCT_RESULTS_ASCII_DATE,
  PRJCT_RESULTS_ASCII_TIME,
  PRJCT_RESULTS_ASCII_YEAR,
  PRJCT_RESULTS_ASCII_JULIAN,
  PRJCT_RESULTS_ASCII_JDFRAC,
  PRJCT_RESULTS_ASCII_TIFRAC,
  PRJCT_RESULTS_ASCII_SCANS,
  PRJCT_RESULTS_ASCII_TINT,
  PRJCT_RESULTS_ASCII_SZA,
  PRJCT_RESULTS_ASCII_CHI,
  PRJCT_RESULTS_ASCII_RMS,
  PRJCT_RESULTS_ASCII_AZIM,
  PRJCT_RESULTS_ASCII_TDET,
  PRJCT_RESULTS_ASCII_SKY,
  PRJCT_RESULTS_ASCII_BESTSHIFT,
  PRJCT_RESULTS_ASCII_REFZM,
  PRJCT_RESULTS_ASCII_REFSHIFT,
  PRJCT_RESULTS_ASCII_PIXEL,
  PRJCT_RESULTS_ASCII_PIXEL_TYPE,
  PRJCT_RESULTS_ASCII_ORBIT,
  PRJCT_RESULTS_ASCII_LONGIT,
  PRJCT_RESULTS_ASCII_LATIT,
  PRJCT_RESULTS_ASCII_ALTIT,
  PRJCT_RESULTS_ASCII_COVAR,
  PRJCT_RESULTS_ASCII_CORR,
  PRJCT_RESULTS_ASCII_CLOUD,
  PRJCT_RESULTS_ASCII_COEFF,
  PRJCT_RESULTS_ASCII_O3,
  PRJCT_RESULTS_ASCII_NO2,
  PRJCT_RESULTS_ASCII_CLOUDTOPP,
  PRJCT_RESULTS_ASCII_LOS_ZA,
  PRJCT_RESULTS_ASCII_LOS_AZIMUTH,
  PRJCT_RESULTS_ASCII_SAT_HEIGHT,
  PRJCT_RESULTS_ASCII_EARTH_RADIUS,
  PRJCT_RESULTS_ASCII_VIEW_ELEVATION,
  PRJCT_RESULTS_ASCII_VIEW_AZIMUTH,
  PRJCT_RESULTS_ASCII_SCIA_QUALITY,
  PRJCT_RESULTS_ASCII_SCIA_STATE_INDEX,
  PRJCT_RESULTS_ASCII_SCIA_STATE_ID,
  PRJCT_RESULTS_ASCII_MFC_STARTTIME,
  PRJCT_RESULTS_ASCII_MFC_ENDTIME,
  PRJCT_RESULTS_ASCII_MAX
 };

// Fields to store when using NASA-AMES format
// -------------------------------------------

enum _prjctResultsNasaFields
 {
  PRJCT_RESULTS_NASA_NO2,
  PRJCT_RESULTS_NASA_O3,
  PRJCT_RESULTS_NASA_OCLO,
  PRJCT_RESULTS_NASA_BRO,
  PRJCT_RESULTS_NASA_MAX
 };

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

enum prjctTabPagesTypes
 {
  TAB_TYPE_PRJCT_SPECTRA,                              // Spectra selection tab page
  TAB_TYPE_PRJCT_ANLYS,                                // Analysis tab page
  TAB_TYPE_PRJCT_FILTER,                               // Filter tab page
  TAB_TYPE_PRJCT_CALIBRATION,                          // Calibration tab page
  TAB_TYPE_PRJCT_USAMP,                                // Undersampling tab page
  TAB_TYPE_PRJCT_INSTRUMENTAL,                         // Instrumental tab page
  TAB_TYPE_PRJCT_SLIT,                                 // Slit function tab page
  TAB_TYPE_PRJCT_ASCII_RESULTS,                        // ASCII Results tab page
  TAB_TYPE_PRJCT_NASA_RESULTS,                         // NASA-AMES Results tab page
  TAB_TYPE_PRJCT_MAX
 };

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

   // ---------------------
   // CONSTANTS DEFINITIONS
   // ---------------------

   // Supported line shapes for convolution
   // -------------------------------------

   enum _slitTypes
    {
  //  	SLIT_TYPE_NONE,
     SLIT_TYPE_FILE,                                                            // user-defined line shape provided in a file
     SLIT_TYPE_GAUSS,                                                           // Gaussian line shape
     SLIT_TYPE_INVPOLY,                                                         // 2n-Lorentz (generalisation of the Lorentzian function
     SLIT_TYPE_VOIGT,                                                           // Voigt profile function
     SLIT_TYPE_ERF,                                                             // error function (convolution of a Gaussian and a boxcar)
     SLIT_TYPE_APOD,                                                            // apodisation function (used with FTS)
     SLIT_TYPE_APODNBS,                                                         // apodisation function (Norton Beer Strong function)
     SLIT_TYPE_GAUSS_FILE,                                                      // Gaussian line shape, wavelength dependent (provided in a file)
     SLIT_TYPE_INVPOLY_FILE,                                                    // 2n-Lorentz line shape, wavelength dependent (file)
   //  SLIT_TYPE_VOIGT_FILE,                                                    // Voigt profile function, wavelength dependent (file)
     SLIT_TYPE_ERF_FILE,                                                        // error function, wavelength dependent (file)
     SLIT_TYPE_GAUSS_T_FILE,                                                    // Gaussian line shape, wavelength and temperature dependent (file)
     SLIT_TYPE_ERF_T_FILE,                                                      // error function, wavelength and temperature dependent (file)
     SLIT_TYPE_MAX
    };

   // Convolution type
   // ----------------

   enum _convolutionTypes
    {
     CONVOLUTION_TYPE_NONE,                                                     // no convolution, interpolation only
     CONVOLUTION_TYPE_STANDARD,                                                 // standard convolution
     CONVOLUTION_TYPE_I0_CORRECTION,                                            // convolution using I0 correction
   // CONVOLUTION_TYPE_RING,                                                    // creation of a ring xs using high-resoluted solar and raman spectra
     CONVOLUTION_TYPE_MAX
    };

   // Conversion modes
   // ----------------

   enum _conversionModes
    {
     CONVOLUTION_CONVERSION_NONE,                                               // no conversion
     CONVOLUTION_CONVERSION_AIR2VAC,                                            // air to vacuum
     CONVOLUTION_CONVERSION_VAC2AIR,                                            // vacuum to air
     CONVOLUTION_CONVERSION_MAX
    };

  // Pages of the dialog box
  // -----------------------

   enum _convolutionTabPages
    {
     TAB_TYPE_XSCONV_GENERAL,                                                   // general information (files names, convolution type...)
     TAB_TYPE_XSCONV_SLIT,                                                      // information on the slit function
     TAB_TYPE_XSCONV_FILTER,                                                    // information on the filter to apply
     TAB_TYPE_XSCONV_MAX
    };

   // ----------------------
   // STRUCTURES DEFINITIONS
   // ----------------------

   // Cross section to convolute

   typedef struct _xs
    {
     INT     NDET;                                                              // size of vector
     double *lembda,                                                            // wavelength calibration
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

   RC   XSCONV_LoadCalibrationFile(XS *pLembda,UCHAR *lembdaFile,INT nextraPixels);
   RC   XSCONV_LoadSlitFunction(XS *pSlitXs,SLIT *pSlit,double *pGaussWidth,INT *pSlitType);
   RC   XSCONV_LoadCrossSectionFile(XS *pCross,UCHAR *crossFile,double lembdaMin,double lembdaMax,double shift,INT conversionMode);

   // Convolution functions

   RC   XSCONV_TypeNone(XS *pXsnew,XS *pXshr);
   RC   XSCONV_TypeGauss(double *lambda,double *Spec,double *SDeriv2,double lambdaj,double dldj,double *SpecConv,double fwhm,double n,INT slitType);
   RC   XSCONV_TypeStandardFFT(FFT *pFFT,INT fwhmType,double slitParam,double slitParam2,double *lembda,double *target,INT size);
   RC   XSCONV_TypeStandard(XS *pXsnew,INDEX indexLembdaMin,INDEX indexLembdaMax,XS *pXshr,XS *pSlit,XS *pI,double *Ic,INT slitType,double slitWidth,double slitParam,double slitParam2,double slitParam3,double slitParam4);
   RC   XSCONV_RealTimeXs(XS *pXshr,XS *pXsI0,XS *pSlit,double *IcVector,double *lembda,INDEX indexLembdaMin,INDEX indexLembdaMax,double *newXs,INT slitType,double slitParam,double slitParam2,double slitParam3,double slitParam4);

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
// =================
// THREAD PROCESSING
// =================

// ---------------------
// CONSTANTS DEFINITIONS
// ---------------------

enum _thrdId
 {
  THREAD_TYPE_NONE,
  THREAD_TYPE_SPECTRA,
  THREAD_TYPE_ANALYSIS,
  THREAD_TYPE_KURUCZ
 };

enum _thrdBrowse
 {
  THREAD_BROWSE_SPECTRA,
  THREAD_BROWSE_DARK,
  THREAD_BROWSE_ERROR,
  THREAD_BROWSE_EXPORT,
  THREAD_BROWSE_MFC_OFFSET,
  THREAD_BROWSE_MFC_DARK,
  THREAD_BROWSE_MFC_INSTR,
  THREAD_BROWSE_MFC_LAMP
 };

enum _thrdLevels
 {
  THREAD_LEVEL_RECORD,
  THREAD_LEVEL_FILE,
  THREAD_LEVEL_PROJECT,
  THREAD_LEVEL_MAX
 };

enum _threadEvents
 {
  THREAD_EVENT_FIRST,
  THREAD_EVENT_PREVIOUS,
  THREAD_EVENT_NEXT,
  THREAD_EVENT_LAST,
  THREAD_EVENT_GOTO,
  THREAD_EVENT_PLAY,
  THREAD_EVENT_PAUSE,
  THREAD_EVENT_STOP,
  THREAD_EVENT_STOP_PROGRAM,
  THREAD_EVENT_MAX
 };

enum _thrdGoto
 {
  THREAD_GOTO_RECORD,
  THREAD_GOTO_PIXEL
 };

// ----------------------
// STRUCTURES DEFINITIONS
// ----------------------

// Data on current spectrum
// ------------------------

// Geolocations and angles for satellite measurements

typedef struct _satelliteGeoloc
 {
  // Geolocations

  double lonCorners[4],
         latCorners[4],
         lonCenter,
         latCenter;

  // Angles

  float  solZen[3],
         solAzi[3],
         losZen[3],
         losAzi[3];

  // Miscellaneous

  float earthRadius,satHeight;
  float cloudTopPressure,cloudFraction;                                         // information on clouds
 }
SATELLITE_GEOLOC;

//
// OPUS format : these data have been provided by Ann-Carine VANDAELE, ULB
//

typedef struct _opusData
 {
  UCHAR  Apodization[5],SampleName[64],SampleForm[64],ChemistName[64],Date[15],Time[10];
  BOOL   Ascending;
  ULONG  NumberPoints;
  double WaveLow,WaveHigh,Resolution,Dispersion,ScaleFactor,Scantime;
  float  IrisDiameter,Maximum;
 }
OPUS_DATA;

//
// GOME format
//

typedef struct _gomeData                                                        // data on the current GOME pixel
 {
  INT   orbitNumber;                                                            // orbit number
  INT   pixelNumber;                                                            // pixel number
  INT   pixelType;                                                              // pixel type

  SHORT_DATE irradDate;                                                         // date of measurement for the irradiance spectrum
  struct time irradTime;                                                        // time of measurement for the irradiance spectrum

  int     nRef;                                                                 // size of irradiance vectors

  float longit[5];                                                              // longitudes (four corners of the GOME pixel + pixel centre)
  float latit[5];                                                               // latitudes (four corners of the GOME pixel + pixel centre)
  float sza[3];                                                                 // solar zenith angles (East, center and west points of the GOME pixel)
  float azim[3];                                                                // solar azimuth angles (East, center and west points of the GOME pixel)
 }
GOME_DATA;

typedef struct _ccd
 {
  MATRIX_OBJECT drk;
  MATRIX_OBJECT vip;
  MATRIX_OBJECT dnl;
 }
CCD;

//

typedef struct _sciamachy
 {
  INT    orbitNumber;                                                           // orbit number
  double longitudes[4],latitudes[4];                                            // geolocations at the 4 corners of the pixels
  float  solZen[3],solAzi[3],losZen[3],losAzi[3];                               // resp. solar and line of sight zenith and azimuth angles
  float  earthRadius,satHeight;                                                 // for satellite to TOA angles correction
  INDEX  stateIndex,stateId;                                                    // information on the state
  INT    qualityFlag;
 }
SCIA_DATA;

typedef struct _gome2
 {
  INT    orbitNumber;                                                           // orbit number
  double longitudes[4],latitudes[4];                                            // geolocations at the 4 corners of the pixels
  float  solZen[3],solAzi[3],losZen[3],losAzi[3];                               // resp. solar and line of sight zenith and azimuth angles
  float  earthRadius,satHeight;                                                 // for satellite to TOA angles correction
  float  cloudTopPressure,cloudFraction;                                        // information on clouds
 }
GOME2_DATA;

typedef struct _omi
 {
 	INDEX omiTrackIndex;                                                          // index of the track record
 	INDEX omiSpecIndex;                                                           // index of the current spectrum in the current track
 	int   omiNumberOfTracks,                                                      // total number of tracks
 	      omiNumberOfSpectraPerTrack;                                             // total number of spectra in tracks
 }
OMI_DATA;

typedef struct _specInfo
 {
  // Data on project and file

  PROJECT project;
  UCHAR fileName[MAX_STR_LEN+1];

  // spectra buffers

  double *lembda,         // wavelengths
         *spectrum,       // raw spectrum
         *sigmaSpec,      // error on raw spectrum if any
         *darkCurrent,    // dark current
         *specMax,        // maxima of signal over scans
         *instrFunction,
         *varPix,         // variability interpixel
         *dnl;            // non linearity of detector

  // record information

  ULONG  *recordIndexes;                                       // indexes of records for direct access

  CCD     ccd;

  INT     recordNumber;                                        // total number of record in file
  INT     recordIndexesSize;                                   // size of 'recordIndexes' buffer
  INT     recordSize;                                          // size of record if length fixed
  INDEX   indexRecord,indexFile,indexProject;
  INT     lastSavedRecord;

  // experiment data

  char   Nom[21];                                              // Nom du spectre
  int    NSomme;                                               // Nombre de sommations
  double Tint,                                                 // Temps d'int‚gration
         Zm,                                                   // Angle z‚nithal
         Azimuth,                                              // angle azimutal
         Tm;                                                   // Conv date & heure en sec
  char   SkyObs;                                               // Etat du ciel
  float  ReguTemp;                                             // R‚gulation de temp‚rature
  double TotalExpTime;                                         // Dur‚e totale pour une exp
                                                               // SAOZ

  double TDet,                                                 // Temp‚rature du D‚tecteur
         BestShift;                                            // Meilleur D‚calage

                                                               // CCD

  int rejected;                                                // Nbre de spectres rejet‚s
  int NTracks;                                                 // Nbre de tracks retenus
  SHORT_DATE present_day;                                      // Date de la mesure
  struct time present_time;                                    // Heure de la mesure

  double TimeDec;                                              // decimal time
  double localTimeDec;                                         // local decimal time
  INT    localCalDay;                                          // local calendar day
  double Cic;                                                  // color index
  int    useErrors;

  // only used for balloons measurements and GOME

  double longitude;
  double latitude;
  double altitude;

  double aMoon,hMoon,fracMoon;

  float  elevationViewAngle;                                                    // elevation viewing angle
  float  azimuthViewAngle;                                                      // azimuth viewing angle
  float  zenithViewAngle;

  // only used for MFC format

  struct time startTime;
  struct time endTime;
  float wavelength1;
  float dispersion[3];

  INDEX indexBand;

  // CCD ULB

  short NGrating;
  float Nanometers;

  // information specific to a format

  OPUS_DATA opus;                                                               // OPUS format
  GOME_DATA gome;                                                               // GOME format
  SCIA_DATA scia;                                                               // SCIAMACHY format
  GOME2_DATA gome2;                                                             // GOME2 format
  OMI_DATA omi;

  // satellite data

  UCHAR  refFileName[MAX_PATH_LEN+1];
  INT    refRecord;
 }
SPEC_INFO;

typedef struct _goto
 {
  INDEX indexOld;
  INDEX indexRecord;
  INDEX indexPixel;
  INT   indexType;
  INDEX indexMin;
  INDEX indexMax;
  INDEX increment;
  INDEX flag;
 }
THRD_GOTO;

typedef struct _thrdRef
 {
  INDEX  indexRecord;
  double dist;
  double sza;
 }
THRD_REF;

// ----------------
// GLOBAL VARIABLES
// ----------------

EXTERN UCHAR     THRD_asciiFile[];             // ASCII file for exporting spectra
EXTERN HANDLE    THRD_hEvents[];               // list of events
EXTERN SPEC_INFO THRD_specInfo,THRD_refInfo;   // data on current spectra and reference
EXTERN UINT      THRD_id;                      // thread identification number
EXTERN INT       THRD_levelMax;                // level of thread
EXTERN INT       THRD_lastEvent;               // last event
EXTERN DWORD     THRD_delay;                   // wait for next event
EXTERN double    THRD_localNoon;               // local noon
EXTERN INT       THRD_localShift;
EXTERN INT       THRD_correction;
EXTERN INT       THRD_browseType;
EXTERN THRD_GOTO THRD_goto;
EXTERN INT       THRD_treeCallFlag;
EXTERN INT       THRD_lastRefRecord;
EXTERN INT       THRD_increment;
EXTERN INT       THRD_isFolder;
EXTERN INT       THRD_recordLast;

// QDOAS ??? // ----------
// QDOAS ??? // PROTOTYPES
// QDOAS ??? // ----------
// QDOAS ???
// QDOAS ??? RC               THRD_OddEvenCorrection(double *lembdaData,double *specData,double *output,INT vectorSize);
double           THRD_GetDist(double longit, double latit, double longitRef, double latitRef);
// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? LRESULT CALLBACK THRD_GotoWndProc(HWND hwndThrdGoto,UINT msg,WPARAM mp1,LPARAM mp2);
// QDOAS ??? #endif
// QDOAS ??? BOOL             THRD_Context(INDEX indexItem,INT menuId);
// QDOAS ??? BOOL             THRD_SetIndexes(INDEX indexItem,INT treeCallFlag);
// QDOAS ??? RC               THRD_ProcessLastError(void);
// QDOAS ??? RC               THRD_Error(INT errorType,INT errorId,UCHAR *function,...);
// QDOAS ??? INDEX            THRD_WaitEvent(DWORD delay,INT moveFlag,INT incrementFlag);
// QDOAS ??? void             THRD_ResetSpecInfo(SPEC_INFO *pSpecInfo);
// QDOAS ??? void             THRD_LoadData(void);
RC               THRD_SpectrumCorrection(SPEC_INFO *pSpecInfo,double *spectrum);
// QDOAS ??? void             THRD_BrowseSpectra(void *);
// QDOAS ??? RC               THRD_Alloc(void);
// QDOAS ??? void             THRD_Free(void);
RC               THRD_CopySpecInfo(SPEC_INFO *pSpecInfoTarget,SPEC_INFO *pSpecInfoSource);
// QDOAS ???
// =======================
// GOME SPECTRA PROCESSING
// =======================

// --------------------
// CONSTANTS DEFINITION
// --------------------

#define BAND_NUMBER                    6                                        // number of bands to display
#define SCIENCE_DATA_DEFINED          10                                        // number of bands
#define SPECTRAL_FITT_ORDER            5                                        // degree of polynomial used for building wavelength scale
#define	MAX_FITT_ORDER	                6
#define GDP_BIN_ERROR_ID_MASK       0x01

enum _gomeGetCommand
 {
  GOME_GET_LEMBDA,
  GOME_GET_SPECTRUM,
  GOME_GET_REFERENCE
 };

// --------------------
// STRUCTURE DEFINITION
// --------------------

// Date in short format

typedef struct _shortDateTime
 {
  char  da_year;        /* Year - 1980      */
  char  da_day;         /* Day of the month */
  char  da_mon;         /* Month (1 = Jan)  */
  char  ti_hour;
  char  ti_min;
  char  ti_sec;
 }
SHORT_DATETIME;

// File header

typedef struct _gomeFileHeader
 {
  unsigned short nspectra;                    // total number of spectra in file
  char           version;
  char           mask;
  int            headerSize;                  // number of bytes before first record
  int            recordSize;                  // size of a record
  int            orbitNumber;                 // orbit number
  short          nbands;                      // number of available bands
  char           nSpectralParam;              // number of set of spectral parameters
  char           indexSpectralParam;          // index of set of spectral parameters for irradiance spectra
  SHORT_DATETIME dateAndTime;                 // measurement date and time in UT
 }
GDP_BIN_FILE_HEADER;

// For each available band, provide header with band and reference general info

typedef struct _gomeBandHeader
 {
  // Band info

  char       bandType;                        // band type
  short      bandSize;                        // band size

  // Reference info

  short      startDetector;                   // index of first pixel used on detector for this ban
  float      scalingFactor;
  float      scalingError;
 }
GDP_BIN_BAND_HEADER;

// Spectrum record structure

typedef struct                            // geolocation coordinates version 1
 {
  float           lonArray[5];            // longitude array
  float           latArray[5];            // latitude array
  float           szaArray[3];            // zenithal array
 }
GEO_1;

typedef struct                            // geolocation coordinates version 2
 {
  USHORT          lonArray[5];            // longitude array
  SHORT           latArray[5];            // latitude array
  float           szaArray[3];            // zenithal array
  float           azim;                   // azimuth
  float           losZa;                  // line of sight zenith angle
  float           losAzim;                // line of sight azimuth angle
  USHORT          unused[4];              // unused bytes
 }
GEO_2;

typedef struct                            // geolocation coordinates version 3
 {
  USHORT          lonArray[5];            // longitude array
  SHORT           latArray[5];            // latitude array
  float           szaArray[3];            // zenithal array
  USHORT          losZa[3];               // line of sight zenithal array
  USHORT          losAzim[3];             // line of sight azimuthal array
  float           satHeight;              // satellite geodetic height at point B
  float           radiusCurve;            // Earth radius curvatur at point B
 }
GEO_3;

typedef struct
 {
  // From Level 1 data

  SHORT_DATETIME  dateAndTime;                // measurement date and time in UT
  short           groundPixelID;              // ground pixel order
  char            groundPixelType;            // ground pixel type
  char            indexSpectralParam;         // index of set of spectral parameters in reference record to use for building calibration
  union _geo
   {
    GEO_1 geo1;
    GEO_2 geo2;
    GEO_3 geo3;
   }
  geo;

  // From Level 2 data

  unsigned short  o3;                     // O3 VCD
  unsigned short  no2;                    // NO2 VCD
  unsigned short  cloudFraction;          // Cloud fraction
  unsigned short  cloudTopPressure;       // Cloud top pressure
  float           aziArray[3];
  unsigned short  unused[4];             // for later new data ?
 }
SPECTRUM_RECORD;

// Keep useful information for fast access to a record

typedef struct _gome_recordInfo
 {
  INDEX  pixelNumber;                                                           // pixel number
  INDEX  pixelType;                                                             // pixel type
  double lat;                                                                   // latitude
  double lon;                                                                   // longitude
  double sza;                                                                   // solar zenith angle
 }
GDP_BIN_INFO;

typedef struct _GOMEOrbitFiles                                                  // description of an orbit
 {
 	UCHAR gdpBinFileName[MAX_STR_LEN+1];                                            // the name of the file with a part of the orbit
 	UCHAR gdpBinFileNumber[9];
  GDP_BIN_INFO *gdpBinInfo;                                                     // useful information on records for fast access
  INDEX gdpBinBandIndex;                                                        // indexes of bands present in the current file
  INDEX *gdpBinLatIndex,*gdpBinLonIndex,*gdpBinSzaIndex,*gdpBinPixIndex;        // indexes of records sorted resp. by latitudes, by SZA or by pixel number
  GDP_BIN_FILE_HEADER gdpBinHeader;
  GDP_BIN_BAND_HEADER gdpBinBandInfo[SCIENCE_DATA_DEFINED];
  SPECTRUM_RECORD     gdpBinSpectrum;
  int                 gdpBinSpectraSize,                                        // total size of spectra vector GDP_BIN_coeff
                      gdpBinCoeffSize,                                          // number of polynomial coefficients in vector
                      gdpBinStartPixel[SCIENCE_DATA_DEFINED];                   // starting pixels for bands present in the file
  double             *gdpBinCoeff;                                              // coefficients for reconstructing wavelength calibrations
  float               gdpBinScalingFactor[SCIENCE_DATA_DEFINED],                // scaling factors for spectra band per band
                      gdpBinScalingError[SCIENCE_DATA_DEFINED];                 // scaling factors for errors band per band
  USHORT             *gdpBinReference,                                          // buffer for irradiance spectra
                     *gdpBinRefError;                                           // errors on irradiance spectra
  INT                 specNumber;
  RC rc;
 }
GOME_ORBIT_FILE;

// ---------------------
// VARIABLES DECLARATION
// ---------------------

#define MAX_GOME_FILES 50 // maximum number of files per orbit

EXTERN GOME_ORBIT_FILE GDP_BIN_orbitFiles[MAX_GOME_FILES];                      // list of files for an orbit
EXTERN INDEX GDP_BIN_currentFileIndex;                                          // index of the current file in the list
EXTERN UCHAR *GDP_BIN_BandStrTab[];

// ----------
// PROTOTYPES
// ----------

INDEX            GDP_BIN_GetRecordNumber(INT pixelNumber);
#if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
LRESULT CALLBACK GDP_BIN_WndProc(HWND hwndGome,UINT msg,WPARAM mp1,LPARAM mp2);
#endif
RC               GDP_BIN_GetBand(SPEC_INFO *pSpecInfo,INT bandNo);
void             GDP_BIN_GetReferenceInfo(SPEC_INFO *pSpecInfo);
RC               GDP_BIN_LoadAnalysis(SPEC_INFO *pSpecInfo,FILE *specFp);

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

#endif

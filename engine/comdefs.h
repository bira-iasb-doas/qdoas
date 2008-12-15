
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  THE BIRA-IASB DOAS SOFTWARE FOR WINDOWS AND LINUX
//  Module purpose    :  LOW-LEVEL DEFINITIONS
//  Name of module    :  COMDEFS.H
//  Creation date     :  29 September 2004
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
//
//  MODULE DESCRIPTION
//
//  The present module contains the compiler directives and the definitions that
//  couldn't be supported by all operating systems or compilers.  It also
//  includes some definitions related to the low-level functions of the package
//  (errors handling, debugging, memory allocation...).
//
//  ----------------------------------------------------------------------------

#if !defined(__COMDEFS_)
#define __COMDEFS_

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// ===================
// COMPILATION CONTROL
// ===================

// QDOAS ??? #define __WINDOAS_WIN_ 1                                                        // 1 to compile for Windows
// QDOAS ??? #define __WINDOAS_GUI_ 1                                                        // 1 to use the Windows graphics user interface
// QDOAS ??? #define __BC32_        0                                                        // 1 to compile under Borland C/C++ 5.0

#define __INCLUDE_HDF_ 0                                                        // 1 to include HDF

// ===============
// INCLUDE HEADERS
// ===============

// QDOAS ??? // Operating system
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ???     #include <windows.h>
// QDOAS ???     #include <windowsx.h>
// QDOAS ???     #include <commctrl.h>
// QDOAS ???     #include <shlobj.h>
// QDOAS ???     #include <io.h>
// QDOAS ???     #include <sys\stat.h>
// QDOAS ???     #include <share.h>
// QDOAS ???     #include <process.h>
// QDOAS ???
// QDOAS ???     #if defined(__BC32_) && __BC32_
// QDOAS ???     #include <dir.h>
// QDOAS ???     #include <dirent.h>
// QDOAS ???     #else
// QDOAS ???
// QDOAS ???     #pragma comment(lib, "comctl32.lib")
// QDOAS ???     #pragma comment(lib, "ws2_32.lib")
// QDOAS ???
// QDOAS ???     #include <direct.h>
// QDOAS ???     #include <time.h>
// QDOAS ???     #endif
// QDOAS ??? #else
// QDOAS ???     #include <sys/io.h>
// QDOAS ???     #include <sys/dir.h>
// QDOAS ???     #include <dirent.h>
// QDOAS ??? #endif
// QDOAS ???

#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <stdint.h>

#ifndef WIN32
#define strnicmp strncasecmp
#endif


#pragma pack(1)

// QDOAS ??? // ===========
// QDOAS ??? // DEFINITIONS
// QDOAS ??? // ===========
// QDOAS ???
// QDOAS ??? #if !defined(INFINITE)
// QDOAS ??? #define INFINITE            0xFFFFFFFF                                          // Infinite timeout
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // Types [re]definition
// QDOAS ???

typedef int            INT;
typedef char           DoasCh;
typedef unsigned short DoasUS,WORD;
typedef int            BOOL;
typedef uint32_t       DoasU32;
typedef int32_t        DoasI32;
typedef unsigned char  BYTE;
typedef unsigned int   UINT;
//typedef unsigned long  HWND;
typedef unsigned long  DWORD;
typedef void *         HANDLE;

#if !defined(TRUE)
#define TRUE  1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#define EXTERN      extern
typedef int         INDEX,RC;                                            // RC holds for return code
typedef UINT        MASK,SZ_LEN;

// Macros

#ifndef max
#define max(a,b) ( ( (a) >= (b) ) ? (a) : (b) )                                 // returns the maximum between two numbers
#endif
#ifndef min
#define min(a,b) ( ( (a) <= (b) ) ? (a) : (b) )                                 // returns the minimum between two numbers
#endif

// Standard arrays dimensions

#define BUFFER_SIZE                            4096                             // any buffer allocation

// Strings lengths

#define MAX_STR_LEN                            1023                             // usual length for string buffers
#define MAX_STR_SHORT_LEN                      1023     // Before 511           // usual length for short strings
#define MAX_MSG_LEN                            1023                             // maximum length for messages
#define MAX_PATH_LEN                           1023                             // maximum length for path and file names
#define MAX_FCT_LEN                              63                             // maximum length for the names of functions
#define MAX_VAR_LEN                              63                             // maximum length for the names of variables
#define MAX_ITEM_TEXT_LEN                      1023     // Before 511           // item text length
#define MAX_ITEM_NAME_LEN                       127                             // name of a symbol
#define MAX_ITEM_DESC_LEN                       255                             // description of a symbol

// Other constants definitions

#define ITEM_NONE                              (int)-1                          // default value for an index or a return code out of range

#if defined(WIN32) && WIN32
    #define PATH_SEP '/'                                                        // path separator is different according to the OS (Windows or Unix/Linux)
    #define COMMENT_CHAR ';'                                                    // characters to use for commented lines
#else
    #define PATH_SEP '/'
    #define COMMENT_CHAR '#'
#endif

// =====================
// STRUCTURES DEFINITION
// =====================

// The struct date and struct time structures come from the dos.h file provided
// with the Turbo/Borland compilers. As it seems that these are not supported
// by all compilers and OS, we define them below but do not include the dos.h

struct time
 {
  unsigned char ti_min;                                                         /* Minutes */
  unsigned char ti_hour;                                                        /* Hours */
  unsigned char ti_hund;                                                        /* Hundredths of seconds */
  unsigned char ti_sec;                                                         /* Seconds */
 };

struct date
 {
  int  da_year;                                                                 /* Year - 1980      */
  char da_day;                                                                  /* Day of the month */
  char da_mon;                                                                  /* Month (1 = Jan)  */
 };

// The structure below supports the year in short format instead of integer.
// This is useful to support files with records including this struct date but
// written under DOS or Windows 16 bits.

typedef struct _shortDate
 {
  short da_year;                                                                /* Year - 1980      */
  char  da_day;                                                                 /* Day of the month */
  char  da_mon;                                                                 /* Month (1 = Jan)  */
 }
SHORT_DATE;

// ===============
// TABLE OF ERRORS
// ===============

// Standard errors id

#define ERROR_ID_NO                               0                             // no error
#define ERROR_ID_ALLOC                          100                             // buffer allocation error
#define ERROR_ID_ALLOCVECTOR                    101                             // double type vector allocation error
#define ERROR_ID_ALLOCMATRIX                    102                             // double type matrix allocation error
#define ERROR_ID_BUFFER_FULL                    103                             // a buffer is full and can not receive objects anymore
#define ERROR_ID_COMMANDLINE                    105                             // syntax error in command line
#define ERROR_ID_MEDIATE                        106

// Files

#define ERROR_ID_FILE_NOT_FOUND                 121                             // can not open a file in reading mode (file not found)
#define ERROR_ID_FILE_EMPTY                     122                             // the file is empty or not large enough
#define ERROR_ID_FILE_OPEN                      123                             // can not open a file in writing mode
#define ERROR_ID_FILE_END                       124                             // end of the file is reached
#define ERROR_ID_FILE_RECORD                    125                             // the current file record doesn't match the spectra selection criteria
#define ERROR_ID_FILE_BAD_FORMAT                126                             // unknown file format
#define ERROR_ID_FILE_BAD_LENGTH                127
#define ERROR_ID_WAVELENGTH                     128                             // bad wavelength calibration in the input file
#define ERROR_ID_FILE_OVERWRITE                 129                             // data not saved; change the output file name


// Debug

#define ERROR_ID_DEBUG_START                    201                             // debug mode already running
#define ERROR_ID_DEBUG_STOP                     202                             // debug mode not open
#define ERROR_ID_DEBUG_FCTTYPE                  203                             // unknown function type
#define ERROR_ID_DEBUG_LEVELS                   204                             // number of levels of function calls out of range
#define ERROR_ID_DEBUG_FCTBLOCK                 205                             // incorrect function block

// Memory

#define ERROR_ID_MEMORY_STACKNOTEMPTY           301                             // stack not empty at the end of the memory control
#define ERROR_ID_MEMORY_STACKNOTALLOCATED       302                             // stack not allocated
#define ERROR_ID_MEMORY_STACKALLOCATED          303                             // stack already allocated
#define ERROR_ID_MEMORY_OBJECTNOTFOUND          304                             // object not found in the stack
#define ERROR_ID_MEMORY_DEFMATRIX               305                             // the definition of a matrix is uncompleted
#define ERROR_ID_MEMORY_RELEASE                 306                             // try to release an object not in the stack

// Low-level math operations

#define ERROR_ID_DIVISION_BY_0                  501                             // division by zero
#define ERROR_ID_OVERFLOW                       502                             // exponential overflow
#define ERROR_ID_LOG                            503                             // log error
#define ERROR_ID_BAD_ARGUMENTS                  504                             // the input arguments may be wrong
#define ERROR_ID_MATRIX_DIMENSION               505                             // matrix dimensions must agree
#define ERROR_ID_SQRT_ARG                       506                             // sqrt argument error
#define ERROR_ID_POW                            507                             // pow overflow

// Output

#define ERROR_ID_AMF                            801                             // AMF not calculated
#define ERROR_ID_NOTHING_TO_SAVE                802                             // nothing to save
#define ERROR_ID_OUTPUT                         803

// High-level math functions

#define ERROR_ID_SVD_ILLCONDITIONNED           1101                             // ill-conditionned matrix
#define ERROR_ID_SVD_ARG                       1102                             // bad arguments
#define ERROR_ID_SPLINE                        1110                             // spline interpolation requests increasing absissae
#define ERROR_ID_VOIGT                         1111                             // Voigt function failed
#define ERROR_ID_ERF                           1112                             // error with the calculation of the erf function
#define ERROR_ID_JULIAN_DAY                    1120                             // error in the calculation of the Julian day
#define ERROR_ID_MATINV                        1130                             // matrix inversion failed
#define ERROR_ID_CONVERGENCE                   1140                             // the algorithm doesn't converge
#define ERROR_ID_USAMP                         1150                             // undersampling tool failed
#define ERROR_ID_OPTIONS                       1160                             // incompatible options in the analysis
#define ERROR_ID_GAUSSIAN                      1170                             // calculation of an effective slit function failed
#define ERROR_ID_SLIT                          1180                             // bad input slit function types for resolution adjustment
#define ERROR_ID_SLIT_T                        1190                             // bad input slit function types for resolution adjustment with temperature dependency
#define ERROR_ID_GAPS                          1200                             // invalid window for calibration
#define ERROR_ID_ANALYSIS                      1210                             // analysis aborted
#define ERROR_ID_MSGBOX_FIELDEMPTY             1220                             // field empty or invalid
#define ERROR_ID_NFREE                         1230                             // problem with the number of degrees of freedom
#define ERROR_ID_ORTHOGONAL_BASE               1240                             // problem with the orthogonalisation of cross sections
#define ERROR_ID_ORTHOGONAL_CASCADE            1250                             // problem with the orthogonalisation of cross sections
#define ERROR_ID_FWHM                          1260                             // Can't apply fitting of slit function parameters with calibration and resolution correction
#define ERROR_ID_OUT_OF_RANGE                  1270                             // field is out of range
#define ERROR_ID_FILE_AUTOMATIC                1280                             // no automatic reference selection can be perform on this type of file
#define ERROR_ID_NO_REF                        1290                             // no reference file found in the specified file

// Specific file format

#define ERROR_ID_GDP_BANDINDEX                 1300                             // band is not present in the GDP file

#define ERROR_ID_PDS                           1400                             // error in the SCIA PDS file
#define ERROR_ID_BEAT                          1401                             // error in the GOME2 format
#define ERROR_ID_FILE_FORMAT                   2000                             // bad file format

// =========
// DEBUGGING
// =========

// The following flag allows to easily enable/disable the debugging and to avoid
// the overload due to the presence of the debug functions in the code by removing
// them from the compilation as far as debug calls are included in a
// #if defined(__DEBUG_) / #endif block.

#define __DEBUG_                   0                                            // 1 to enable the debug mode, 0 to disable the debug mode

#define __DEBUG_DOAS_SVD_          0                                            // SVD decomposition
#define __DEBUG_DOAS_SHIFT_        0                                            // interpolation of vectors
#define __DEBUG_DOAS_DATA_         0                                            // load data
#define __DEBUG_DOAS_OUTPUT_       0                                            // output
#define __DEBUG_DOAS_FILE_         0                                            // file
#define __DEBUG_DOAS_CONFIG_       0                                            // config

// Types of functions to debug

#define  DEBUG_FCTTYPE_ALL    0xFF
#define  DEBUG_FCTTYPE_MEM    0x01                                              // memory allocation
#define  DEBUG_FCTTYPE_GUI    0x02                                              // user interface
#define  DEBUG_FCTTYPE_MATH   0x04                                              // math function
#define  DEBUG_FCTTYPE_APPL   0x08                                              // application related
#define  DEBUG_FCTTYPE_UTIL   0x10                                              // utility function
#define  DEBUG_FCTTYPE_FILE   0x20                                              // file management
#define  DEBUG_FCTTYPE_CONFIG 0x40                                              // check the config

// Authorize the debugging double type variables allocated by MEMORY_AllocDVector or MEMORY_AllocDMatrix

enum { DEBUG_DVAR_NO, DEBUG_DVAR_YES };

// Structures definition

typedef union _debugVarPtr                                                      // use a different definition of the pointer according to the type of the variable to debug
 {
  DoasCh   **ucharArray;                                                         // pointer to a string array
  DoasCh    *ucharVector;                                                        // pointer to a string
  short   **shortArray;                                                         // pointer to a short type array
  short    *shortVector;                                                        // pointer to a short type vector
  DoasUS  **ushortArray;                                                        // pointer to a unsigned short type array
  DoasUS   *ushortVector;                                                       // pointer to a unsigned short type vector
  int     **intArray;                                                           // pointer to a integer type array
  int      *intVector;                                                          // pointer to a integer type vector
  long    **longArray;                                                          // pointer to a long type array
  long     *longVector;                                                         // pointer to a long type vector
  float   **floatArray;                                                         // pointer to a float type array
  float    *floatVector;                                                        // pointer to a float type vector
  double   *doubleVector;                                                       // pointer to a double type vector
  double  **doubleArray;                                                        // pointer to a double type array
 }
DEBUG_VARPTR;

typedef struct _debugVariables                                                  // information on variables to debug
 {
  DoasCh         varName[MAX_VAR_LEN+1];                                         // the name of the variable to debug
  DEBUG_VARPTR  varData;                                                        // pointer to the buffer to print out
  INT           varNl,varNc;                                                    // the size of the variable to debug
  INT           varNlOff,varNcOff;                                              // the offset to apply to resp. index of lines and columns
  INDEX         varNlMin,varNlMax,varNcMin,varNcMax;                            // these indexes define the area of the vector or the matrix to print out
  INT           varType;                                                        // the type of the variable to debug
  INT           varMatrixFlag;                                                  // 1 if the object to debug is a matrix, 0 for a vector
 }
DEBUG_VARIABLE;

// Prototypes

void DEBUG_Print(DoasCh *formatString,...);
void DEBUG_PrintVar(DoasCh *message,...);
RC   DEBUG_FunctionBegin(DoasCh *fctName,MASK fctType);
RC   DEBUG_FunctionStop(DoasCh *fctName,RC rcFct);
RC   DEBUG_Start(DoasCh *fileName,DoasCh *fctName,MASK fctMask,int nLevels,int varFlag,int resetFlag);
RC   DEBUG_Stop(DoasCh *callingFct);

// ===============
// ERRORS HANDLING
// ===============

// Error types (November 2007 : modify the values for QDoas compatibility, see mediate_types.h)

#define ERROR_TYPE_UNKNOWN    (int)-1
#define ERROR_TYPE_WARNING          2                                           // WarningEngineError
#define ERROR_TYPE_FATAL            3                                           // FatalEngineError
#define ERROR_TYPE_DEBUG            1                                           // InformationEngineError
#define ERROR_TYPE_FILE             1                                           // InformationEngineError

// Description of an error

typedef struct _errorDescription
 {
  int   errorType;                                                              // type of error (warning, fatal error, ...)
  int   errorId;                                                                // id number of the error
  DoasCh errorFunction[MAX_FCT_LEN+1];                                           // name of the calling function that produced the error
  DoasCh errorString[MAX_MSG_LEN+1];                                             // error message
 }
ERROR_DESCRIPTION;

// Prototypes

RC ERROR_DisplayMessage(void *responseHandle);
RC ERROR_SetLast(DoasCh *callingFunction,int errorType,RC errorId,...);
RC ERROR_GetLast(ERROR_DESCRIPTION *pError);

// ===============
// MEMORY HANDLING
// ===============

// Constants definition

#define MEMORY_STACK_SIZE        5000                                           // maximum objets allowed in the stack

// type of objects

enum _memoryTypes
 {
 	MEMORY_TYPE_UNKNOWN,                                                          // unknown
 	MEMORY_TYPE_PTR,                                                              // pointer
 	MEMORY_TYPE_STRING,                                                           // character/string
 	MEMORY_TYPE_SHORT,                                                            // short
 	MEMORY_TYPE_USHORT,                                                           // unsigned short
 	MEMORY_TYPE_INT,                                                              // integer
 	MEMORY_TYPE_LONG,                                                             // long
 	MEMORY_TYPE_FLOAT,                                                            // float
 	MEMORY_TYPE_DOUBLE,                                                           // double
 	MEMORY_TYPE_STRUCT,                                                           // structure
 	MEMORY_TYPE_MAX
 };

#define MEMORY_TYPE_INDEX MEMORY_TYPE_INT
#define MEMORY_TYPE_ULONG MEMORY_TYPE_LONG
#define MEMORY_TYPE_UINT  MEMORY_TYPE_INT

// Information on allocated objects

typedef struct _memory
 {
  DoasCh  callingFunctionName[MAX_FCT_LEN+1];                                    // name of the calling function
  DoasCh  bufferName[MAX_VAR_LEN+1];                                             // name of the buffer
  DoasCh *pBuffer;                                                               // pointer to the allocated buffer
  INT    itemNumber;                                                            // number of items in buffer
  INT    itemSize;                                                              // size of item
  INT    offset;                                                                // index of the first item
  INT    type;                                                                  // type of object
 }
MEMORY;

// Global variables

EXTERN INT MEMORY_stackSize;                                                    // the size of the stack of allocated objects
EXTERN DoasCh *MEMORY_types[MEMORY_TYPE_MAX];                                    // available types for allocated objects

// Prototypes

void    *MEMORY_AllocBuffer(DoasCh *callingFunctionName,DoasCh *bufferName,INT itemNumber,INT itemSize,INT offset,INT type);
void     MEMORY_ReleaseBuffer(DoasCh *callingFunctionName,DoasCh *bufferName,void *pBuffer);
double  *MEMORY_AllocDVector(DoasCh *callingFunctionName,DoasCh *bufferName,int nl,int nh);
void     MEMORY_ReleaseDVector(DoasCh *callingFunctionName,DoasCh *bufferName,double *v,int nl);
double **MEMORY_AllocDMatrix(DoasCh *callingFunctionName,DoasCh *bufferName,int nrl,int nrh,int ncl,int nch);
void     MEMORY_ReleaseDMatrix(DoasCh *callingFunctionName,DoasCh *bufferName,double **m,int ncl,int nch,int nrl);

RC       MEMORY_Alloc(void);
RC       MEMORY_End(void);

RC       MEMORY_GetInfo(DEBUG_VARIABLE *pVariable,DoasCh *pBuffer);

// ======================================
// STDFUNC.C : STANDARD UTILITY FUNCTIONS
// ======================================

// Prototypes

double      STD_Pow10(int p);
DoasCh      *STD_StrTrim(DoasCh *str);
int         STD_Sscanf(DoasCh *line,DoasCh *formatString,...);
long        STD_FileLength(FILE *fp);

char       *STD_Strupr(char *n);
char       *STD_Strlwr(char *n);
int         STD_Stricmp(char *str1,char *str2);

int         STD_IsDir(char *filename);

// QDOAS ??? // ============
// QDOAS ??? // MAIN PROGRAM
// QDOAS ??? // ============
// QDOAS ???
// QDOAS ??? typedef struct _doasArg
// QDOAS ???  {
// QDOAS ???   INT   analysisFlag;
// QDOAS ???   DoasCh wdsFile[MAX_STR_LEN+1];
// QDOAS ???   DoasCh projectName[MAX_STR_LEN+1];
// QDOAS ???   DoasCh fileName[MAX_STR_LEN+1];
// QDOAS ???  }
// QDOAS ??? DOAS_ARG;
// QDOAS ???
// QDOAS ??? // Global variables
// QDOAS ??? // ----------------
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? EXTERN HINSTANCE DOAS_hInst;                                                    // current instance of the program
// QDOAS ??? EXTERN HWND      DOAS_hwndMain;                                                 // handle of the main window
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? EXTERN DoasCh     DOAS_szTitle[40];                                              // the text of title bar
// QDOAS ??? EXTERN DoasCh     DOAS_HelpPath[MAX_PATH_LEN+1];                                 // path for help file
// QDOAS ??? EXTERN DoasCh     DOAS_logFile[MAX_PATH_LEN+1];                                  // file for log errors
// QDOAS ??? EXTERN DoasCh     DOAS_dbgFile[MAX_PATH_LEN+1];                                  // file for debug output
// QDOAS ??? EXTERN DoasCh     DOAS_tmpFile[MAX_PATH_LEN+1];                                  // temporary file for spectra and analysis data
// QDOAS ??? EXTERN DoasCh     DOAS_sysFile[MAX_PATH_LEN+1];                                  // system file
// QDOAS ??? EXTERN DoasCh     DOAS_broAmfFile[MAX_PATH_LEN+1];                               // specific BrO processing
// QDOAS ???
// QDOAS ??? EXTERN DOAS_ARG  DOAS_arg;                                                      // arguments of the program when used from the command line prompt
// QDOAS ???
// QDOAS ??? // Prototypes
// QDOAS ??? // ----------
// QDOAS ???
// QDOAS ??? void    DOAS_CenterWindow(HWND hwndChild,HWND hwndParent);
// QDOAS ??? void    DOAS_ListMoveSelectedItems(HWND hwndParent,DoasU32 listFrom,DoasU32 listTo);
// QDOAS ???
// QDOAS ??? RC      MSG_MessageBox(HWND hwnd,INT controlID,INT titleID,INT msgID,INT style,...);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif


//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  ERROR MESSAGES PROCESSING
//  Name of module    :  ERROR.C
//  Creation date     :  20 March 2001
//  Modified          :  1 october 2004 (the error message is retrieved from a list)
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
//  ---------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  ERROR_SetLast - save the information on the last error in a stack;
//  ERROR_GetLast - retrieve the information about the last error in order to process it;
//
//  ----------------------------------------------------------------------------
//
//  MODULE DESCRIPTION
//
//  When a function fails, the error is rarely processed in the code of this
//  function but later by another one dedicated by the application to display
//  the error message on the screen or to output it in a log file.
//
//  The functions in this module allow to save temporarily the information
//  about the errors in a stack and to transmit them later and easily to the
//  application error handling function.
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"
#include "mediate_response.h"

// ====================
// CONSTANTS DEFINITION
// ====================

#define ERROR_MAX_ERRORS  10                                                    // maximum number of errors in the stack

// ====================
// STRUCTURE DEFINITION
// ====================

// Table of errors (association id number and message)

typedef struct _errorTable
 {
 	int    errorId;
 	DoasCh *errorMessage;
 }
ERROR_TABLE;

// ================
// STATIC VARIABLES
// ================

// Table of errors

ERROR_TABLE errorTable[]=
 {
 	// Standard errors

  { ERROR_ID_NO                        , "no error"                                                                                                           },
  { ERROR_ID_ALLOC                     , "buffer allocation error (%s)"                                                                                       },
  { ERROR_ID_ALLOCVECTOR               , "vector allocation error (%s, %d:%d)"                                                                                },
  { ERROR_ID_ALLOCMATRIX               , "matrix allocation error (%s, %d:%d x %d:%d)"                                                                        },
  { ERROR_ID_BUFFER_FULL               , "the buffer of %s is full"                                                                                           },
  { ERROR_ID_COMMANDLINE               , "syntax error in command line"                                                                                       },
  { ERROR_ID_MEDIATE                   , "Error with field \"%s\" - %s"                                                                                           },

  // File

  { ERROR_ID_FILE_NOT_FOUND            , "file %s not found"                                                                                                  },
  { ERROR_ID_FILE_EMPTY                , "file %s is empty or not large enough"                                                                               },
  { ERROR_ID_FILE_OPEN                 , "can not open file %s in writing mode"                                                                               },
  { ERROR_ID_FILE_END                  , "the end of the file %s is reached"                                                                                  },
  { ERROR_ID_FILE_RECORD               , "the current file record doesn't match the spectra selection criteria"                                               },
  { ERROR_ID_FILE_BAD_FORMAT           , "the format of the file %s is unknown"                                                                               },
  { ERROR_ID_FILE_BAD_LENGTH           , "records of file %s do not have the expected size"                                                                   },
  { ERROR_ID_FILE_OVERWRITE            , "data not saved; change the output file name"                                                                        },
  { ERROR_ID_WAVELENGTH                , "bad wavelength calibration in file %s"                                                                              },
  { ERROR_ID_FILE_NOT_SPECIFIED        , "missing file name in configuration: %s"},

  // Debug

  { ERROR_ID_DEBUG_START               , "debug mode already running"                                                                                         },
  { ERROR_ID_DEBUG_STOP                , "debug mode not open"                                                                                                },
  { ERROR_ID_DEBUG_FCTTYPE             , "unknown function type (0x%04X)"                                                                                     },
  { ERROR_ID_DEBUG_LEVELS              , "number of levels of function calls out of range [0..%d]"                                                            },
  { ERROR_ID_DEBUG_FCTBLOCK            , "incorrect function block (%s/%s)"                                                                                   },

  // Memory

  { ERROR_ID_MEMORY_STACKNOTEMPTY      , "stack not empty at the end of the memory control (%d objects remaining)"                                            },
  { ERROR_ID_MEMORY_STACKNOTALLOCATED  , "stack not allocated"                                                                                                },
  { ERROR_ID_MEMORY_STACKALLOCATED     , "stack already allocated"                                                                                            },
  { ERROR_ID_MEMORY_OBJECTNOTFOUND     , "object not found in the stack (0x%08X)"                                                                             },
  { ERROR_ID_MEMORY_DEFMATRIX          , "the definition of the matrix %s is uncompleted"                                                                     },
  { ERROR_ID_MEMORY_RELEASE            , "try to release an object not in the stack (%s,0x%08X)"                                                              },

  // Output

  { ERROR_ID_AMF                       , "Zenith angle %.2f not found in file %s; AMF not calculated"                                                         },
  { ERROR_ID_NOTHING_TO_SAVE           , "%s : nothing to save "                                                                                              },
  { ERROR_ID_OUTPUT                    , "Output file %s already exists but doesn't have the same configuration"                                              },

  // Low-level math operations

  { ERROR_ID_DIVISION_BY_0             , "division by 0 (%s)"                                                                                                 },
  { ERROR_ID_OVERFLOW                  , "exp overflow"                                                                                                       },
  { ERROR_ID_LOG                       , "log error"                                                                                                          },
  { ERROR_ID_BAD_ARGUMENTS             , "bad argument (%s)"                                                                                                  },
  { ERROR_ID_MATRIX_DIMENSION          , "matrix dimensions must agree"                                                                                       },
  { ERROR_ID_SQRT_ARG                  , "sqrt argument error"                                                                                                },
  { ERROR_ID_POW                       , "Pow Overflow"                                                                                                       },

  // High-level math functions

  { ERROR_ID_SVD_ILLCONDITIONED       , "ill-conditioned matrix"                                                                                            },
  { ERROR_ID_SVD_ARG                   , "the number of lines of the matrix to decompose is expected to be higher than the number of columns (%d x %d)"       },
  { ERROR_ID_SPLINE                    , "spline interpolation requests increasing absissae (indexes : %d - %d, values : %g - %g)"                            },
  { ERROR_ID_VOIGT                     , "Voigt function failed (x=%g,y=%g)"                                                                                  },
  { ERROR_ID_ERF                       , "error with the calculation of the erf function (%s)"                                                                },
  { ERROR_ID_JULIAN_DAY                , "error in the calculation of the Julian day : Julian = %d, Year = %d"                                                },
  { ERROR_ID_MATINV                    , "matrix inversion failed"                                                                                            },
  { ERROR_ID_CONVERGENCE               , "no convergence after %d iterations"                                                                                 },
  { ERROR_ID_USAMP                     , "undersampling tool failed %d"                                                                                       },
  { ERROR_ID_OPTIONS                   , "incompatible options for %s (%s analysis window)"                                                                   },
  { ERROR_ID_GAUSSIAN                  , "calculation of the effective slit function failed ( slit1 %g <= slit2 %g )"                                         },
  { ERROR_ID_SLIT                      , "Slit function should be gaussian or integrated gaussians type for resolution adjustment"                            },
  { ERROR_ID_SLIT_T                    , "Slit function should be gaussian or integrated gaussians type and temperature dependent for resolution adjustment"  },
  { ERROR_ID_GAPS                      , "Invalid window for calibration (%g,%g)"                                                                             },
  { ERROR_ID_ANALYSIS                  , "Analysis aborted (rec=%d,%s)"                                                                                       },
  { ERROR_ID_MSGBOX_FIELDEMPTY         , "The field '%s' is empty or invalid"                                                                                 },
  { ERROR_ID_NFREE                     , "Number of degree of freedom <= 0 : reduce the number of parameters or increase the width of the analysis window or reduce the width of filter" },
  { ERROR_ID_ORTHOGONAL_BASE           , "%s cross section forced to be orthogonalized to base because of %s cross section"                                   },
  { ERROR_ID_ORTHOGONAL_CASCADE        , "Orthogonalization in cascade not authorized. Orthogonalization of %s cross section ignored"                         },
  { ERROR_ID_FWHM                      , "Can't apply fitting of slit function parameters with calibration and resolution correction"                         },
  { ERROR_ID_OUT_OF_RANGE              , "%s is out of %d..%d range"                                                                                          },
  { ERROR_ID_FILE_AUTOMATIC            , "No automatic reference selection can be perform on this type of file"                                               },
  { ERROR_ID_NO_REF                    , "No reference spectrum found for %s in file %s"                                                                      },
  { ERROR_ID_CONVOLUTION               , "Convolved solar spectrum and cross sections are expected"                                                           },
  
  
  
  { ERROR_ID_GDP_BANDINDEX             , "The specified band is not present in the file %s"                                                                   },
  { ERROR_ID_PDS                       , "[%s] failed for %s PDS file"                                                                                        },
  { ERROR_ID_BEAT                      , "[%s] failed (file %s, error %s)"                                                                                    },
  { ERROR_ID_OMI_SWATH                 , "[%s] swath not present in %s"                                                                                       },
  { ERROR_ID_OMI_REF                   , "Incompatibility of reference between analysis windows"                                                              },
  { ERROR_ID_OMI_REFSIZE               , "%s and spectra swaths do not have the same size"                                                            },
  { ERROR_ID_FILE_FORMAT               , "File format is unknown"                                                                                             },
  { ERROR_ID_OMI_PIXELQF               , "Spectrum rejected based on pixels quality flags"                                                                    },
  { ERROR_ID_HDFEOS                    , "[%s] failed (file %s, error %s)"
  },
  { ERROR_ID_HDFEOS5_SWATH             , "Could not create swath %s in file %s"
  },
  { ERROR_ID_HDFEOS5_FILE_EXISTS       , "Can not write to existing HDF-EOS5 file %s"
  },
  { ERROR_ID_HDFEOS5_DEFFIELD          , "HDF-EOS5: error creating field %s"
  },
  { ERROR_ID_HDFEOS5_DEFDIM            , "HDF-EOS5: error creating dimension %s of size %d"
  },
  { ERROR_ID_HDFEOS5_WRITEFIELD        , "HDF-EOS5: error writing to field %s"
  },
  { ERROR_ID_HDFEOS5_WRITEATTR         , "HDF-EOS5: error writing to attribute %s"
  },
  // End of the table

  { ITEM_NONE                          , "Unknown error (%d)"                                                                                           }
 };

// Definition of a structure for holding the last error

static ERROR_DESCRIPTION errorStack[ERROR_MAX_ERRORS+1];                        // the error stack
static int errorStackN=0;                                                       // the number of errors in the stack

RC ERROR_DisplayMessage(void *responseHandle)
 {
 	// Declarations

 	ERROR_DESCRIPTION errorDescription;
 	RC rc;

 	rc=0;

 	// Get the last error message

 	while (ERROR_GetLast(&errorDescription)!=0)
 	 {
 	  mediateResponseErrorMessage(errorDescription.errorFunction,errorDescription.errorString,errorDescription.errorType, responseHandle);
 	  if (errorDescription.errorType==ERROR_TYPE_FATAL)
 	   rc=-1;
 	 }

 	return rc;    // 0 on success, -1 on fatal error
 }

// -----------------------------------------------------------------------------
// FUNCTION      ERROR_SetLast
// -----------------------------------------------------------------------------
// PURPOSE       Save the information on the last error in a stack;
//
// INPUT         callingFunction : the name of the calling function;
//               errorType       : type of error (fatal, warning,...)
//               errorId         : the error index in the list below
//               ...             : the list of arguments is variable according
//                                 to the selected message
//
// RETURN        the errorId
// -----------------------------------------------------------------------------

RC ERROR_SetLast(const DoasCh *callingFunction,int errorType,RC errorId,...)
 {
  // Declarations

  ERROR_DESCRIPTION *pError;                                                    // pointer to the current error
  va_list argList;                                                              // pointer to the variable argument list
  INDEX i;

  if (errorId!=ERROR_ID_NO)
   {
   	// Browse already registered errors

    for (i=0;i<errorStackN;i++)
     if (errorStack[i].errorId==errorId)
      break;

    if ((i==errorStackN) &&                                                     // if the error code is already in the stack, do not repeat it
        (errorStackN<=ERROR_MAX_ERRORS))                                        // the stack is not full
     {
      pError=&errorStack[errorStackN];

      // Initializations

      memset(pError,0,sizeof(ERROR_DESCRIPTION));
      strncpy(pError->errorFunction,callingFunction,MAX_FCT_LEN);

      // Save the information on the last error

      if (errorStackN<ERROR_MAX_ERRORS)
       {
        // Browse the table of errors to retrieve the message

        for (i=0;(errorTable[i].errorId!=ITEM_NONE) && (errorTable[i].errorId!=errorId);i++);

        // Error type and id number

        pError->errorType=errorType;
        pError->errorId=errorTable[i].errorId;

        // Build the error message

        if (pError->errorId==ITEM_NONE)
         sprintf(pError->errorString,"unknown error or error id out of range (%d)",errorId);
        else
         {
          va_start(argList,errorId);
          vsprintf(pError->errorString,errorTable[i].errorMessage,argList);
          va_end(argList);
         }
       }

      // Keep the last item in the stack for full stack warning

      else
       {
        pError->errorType=ERROR_TYPE_WARNING;                                   // type of error (warning, fatal error, ...)
        pError->errorId=ERROR_ID_BUFFER_FULL;                                   // id number of the error

        strcpy(pError->errorString,"the stack of errors is full - can not register errors anymore");
       }

      errorStackN++;
     }
   }
  // Return

  return errorId;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ERROR_GetLast
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve the information about the last error in order to process it;
//
// INPUT/OUTPUT  pError : pointer to the structure to receive the description of
//                        the last error.
//
// RETURN        the last return code or 0 if the input pointer is null
// -----------------------------------------------------------------------------

RC ERROR_GetLast(ERROR_DESCRIPTION *pError)
 {
 	// Transmit the information about the last error

  if (pError!=NULL)
   {
    if (errorStackN<=0)
     memset(pError,0,sizeof(ERROR_DESCRIPTION));
    else
     memcpy(pError,&errorStack[--errorStackN],sizeof(ERROR_DESCRIPTION));
   }

  // Return the last error id

  return (pError!=NULL)?pError->errorId:0;
 }


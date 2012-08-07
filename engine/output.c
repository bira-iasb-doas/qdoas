
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  OUTPUT
//  Name of module    :  OUTPUT.C
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
//  ===============
//  DATA PROCESSING
//  ===============
//
//  OUTPUT_GetWveAmf - correct a cross section using wavelength dependent AMF vector;
//
//  OutputGetAmf - return the AMF from table at a specified zenith angle;
//  OUTPUT_ReadAmf - load Air Mass Factors from file;
//  OutputFlux - return the flux at a specified wavelength;
//
//  OUTPUT_ResetData - release and reset all data used for output;
//
//  =======================
//  REGISTER DATA TO OUTPUT
//  =======================
//
//  OutputRegister - register a field to output;
//  OutputRegisterFluxes - register fluxes;
//  OutputRegisterFields - register all the fields that are not parameters of the fit
//  OutputRegisterCalib - register all the calibration fields for each window of the wavelength calibration interval;
//  OutputRegisterParam - register all the parameters of the fit;
//
//  OUTPUT_RegisterData - register all the data to output;
//
//  ===================
//  KEEP DATA TO OUTPUT
//  ===================
//
//  OutputCalib - save data related to the wavelength calibration;
//  OutputSaveRecord - save all the data on the current record (including fitted parameters);
//
//  ================
//  OUTPUT FUNCTIONS
//  ================
//
//  OutputBuildSiteFileName - build the output file name using the selected observation site;
//  OutputBuildFileName - for satellites measurements, build automatically a file name
//                        for output and create a directory structure;
//
//  OutputAscPrintTitles - print titles of columns in the output ASCII file;
//  OutputAscPrintDataSet - flush the data set to the output file (ASC format);
//  OutputBinWriteFields - output information on fields in the binary output file;
//  OutputBinVerifyFields - verify the consistency of the outputFile with the current data set to save;
//  OutputBinWriteDataSet - flush the data set to the output file (binary format);
//  OutputFileOpen - open the outputFile and save the preliminary information;
//
//  OUTPUT_FlushBuffers - flusth the buffers in a one shot;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  OUTPUT_LocalAlloc - allocate and initialize buffers for the records to output
//  OUTPUT_Alloc - allocate general use buffers for output;
//  OUTPUT_Free - release buffers allocated for output;
//
//  ----------------------------------------------------------------------------

#include "engine.h"

PRJCT_RESULTS_FIELDS PRJCT_resultsAscii[PRJCT_RESULTS_ASCII_MAX]=
 {
  { "Spec No"                         , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#4d"      },       // PRJCT_RESULTS_ASCII_SPECNO
  { "Name"                            , MEMORY_TYPE_STRING,             24, ITEM_NONE, ITEM_NONE, "%s"        },       // PRJCT_RESULTS_ASCII_NAME
  { "Date & time (YYYYMMDDhhmmss)"    , MEMORY_TYPE_STRING,             24, ITEM_NONE, ITEM_NONE, "%s"        },       // PRJCT_RESULTS_ASCII_DATE_TIME
  { "Date (DD/MM/YYYY)"               , MEMORY_TYPE_STRING,             24, ITEM_NONE, ITEM_NONE, "%s"        },       // PRJCT_RESULTS_ASCII_DATE
  { "Time (hh:mm:ss)"                 , MEMORY_TYPE_STRING,             24, ITEM_NONE, ITEM_NONE, "%s"        },       // PRJCT_RESULTS_ASCII_TIME
  { "Year"                            , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#4d"      },       // PRJCT_RESULTS_ASCII_YEAR
  { "Day number"                      , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#5d"      },       // PRJCT_RESULTS_ASCII_JULIAN
  { "Fractional day"                  , MEMORY_TYPE_DOUBLE, sizeof(double), ITEM_NONE, ITEM_NONE, "%#10.6lf"  },       // PRJCT_RESULTS_ASCII_JDFRAC
  { "Fractional time"                 , MEMORY_TYPE_DOUBLE, sizeof(double), ITEM_NONE, ITEM_NONE, "%#20.15lf" },       // PRJCT_RESULTS_ASCII_TIFRAC
  { "Scans"                           , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#5d"      },       // PRJCT_RESULTS_ASCII_SCANS
  { "Rejected"                        , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#5d"      },       // PRJCT_RESULTS_ASCII_NREJ
  { "Tint"                            , MEMORY_TYPE_DOUBLE, sizeof(double), ITEM_NONE, ITEM_NONE, "%#12.6lf"  },       // PRJCT_RESULTS_ASCII_TINT
  { "SZA"                             , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_SZA
  { "Chi Square"                      , MEMORY_TYPE_DOUBLE, sizeof(double), ITEM_NONE, ITEM_NONE, "%#12.4le"  },       // PRJCT_RESULTS_ASCII_CHI
  { "RMS"                             , MEMORY_TYPE_DOUBLE, sizeof(double), ITEM_NONE, ITEM_NONE, "%#12.4le"  },       // PRJCT_RESULTS_ASCII_RMS
  { "Solar Azimuth angle"             , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_AZIM
  { "Tdet"                            , MEMORY_TYPE_FLOAT , sizeof(double), ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_TDET
  { "Sky Obs"                         , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#2d"      },       // PRJCT_RESULTS_ASCII_SKY
  { "Best shift"                      , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_BESTSHIFT
  { "Ref SZA"                         , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_REFZM
  { "Ref2/Ref1 shift"                 , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_REFSHIFT
  { "Pixel number"                    , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#5d"      },       // PRJCT_RESULTS_ASCII_PIXEL
  { "Pixel type"                      , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#5d"      },       // PRJCT_RESULTS_ASCII_PIXEL_TYPE
  { "Orbit number"                    , MEMORY_TYPE_INT   , sizeof(INT)   , ITEM_NONE, ITEM_NONE, "%#8d"      },       // PRJCT_RESULTS_ASCII_ORBIT
  { "Longitude"                       , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_LONGIT
  { "Latitude"                        , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_LATIT
  { "Altitude"                        , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_ALTIT
  { "Covariances"                     , MEMORY_TYPE_DOUBLE, sizeof(double), ITEM_NONE, ITEM_NONE, "%#12.4le"  },       // PRJCT_RESULTS_ASCII_COVAR
  { "Correlations"                    , MEMORY_TYPE_DOUBLE, sizeof(double), ITEM_NONE, ITEM_NONE, "%#12.4le"  },       // PRJCT_RESULTS_ASCII_CORR
  { "Cloud fraction"                  , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_CLOUD
  { "Index coeff"                     , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#5d"      },       // PRJCT_RESULTS_ASCII_COEFF
  { "GDP O3 VCD"                      , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_O3
  { "GDP NO2 VCD"                     , MEMORY_TYPE_DOUBLE, sizeof(double), ITEM_NONE, ITEM_NONE, "%#12.4le"  },       // PRJCT_RESULTS_ASCII_NO2
  { "Cloud Top Pressure"              , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_CLOUDTOPP
  { "LoS ZA"                          , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_LOS_ZA
  { "LoS Azimuth"                     , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_LOS_AZIMUTH
  { "Satellite height"                , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_SAT_HEIGHT
  { "Earth radius"                    , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_EARTH_RADIUS
  { "Elev. viewing angle"             , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_VIEW_ELEVATION
  { "Azim. viewing angle"             , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_VIEW_AZIMUTH
  { "Zenith viewing angle"            , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_VIEW_ZENITH     -> the same as LOS ZA
  { "SCIAMACHY Quality Flag"          , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#5d"      },       // PRJCT_RESULTS_ASCII_SCIA_QUALITY
  { "SCIAMACHY State Index"           , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#5d"      },       // PRJCT_RESULTS_ASCII_SCIA_STATE_INDEX
  { "SCIAMACHY State Id"              , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#5d"      },       // PRJCT_RESULTS_ASCII_SCIA_STATE_ID
  { "Start Date (DDMMYYYY)"           , MEMORY_TYPE_STRING,             24, ITEM_NONE, ITEM_NONE, "%s"        },       // PRJCT_RESULTS_ASCII_STARTDATE
  { "Stop Date (DDMMYYYY)"            , MEMORY_TYPE_STRING,             24, ITEM_NONE, ITEM_NONE, "%s"        },       // PRJCT_RESULTS_ASCII_ENDDATE
  { "Start Time (hhmmss)"             , MEMORY_TYPE_STRING,             24, ITEM_NONE, ITEM_NONE, "%s"        },       // PRJCT_RESULTS_ASCII_STARTTIME
  { "Stop Time (hhmmss)"              , MEMORY_TYPE_STRING,             24, ITEM_NONE, ITEM_NONE, "%s"        },       // PRJCT_RESULTS_ASCII_ENDTIME
  { "Scanning angle"                  , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_SCANNING
  { "Filter number"                   , MEMORY_TYPE_INT   , sizeof(INT)   , ITEM_NONE, ITEM_NONE, "%#3d"      },       // PRJCT_RESULTS_ASCII_FILTERNUMBER
  { "Measurement type"                , MEMORY_TYPE_INT   , sizeof(INT)   , ITEM_NONE, ITEM_NONE, "%#3d"      },       // PRJCT_RESULTS_ASCII_MEASTYPE
  { "Head temperature"                , MEMORY_TYPE_DOUBLE, sizeof(double), ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_CCD_HEADTEMPERATURE
  { "Cooler status"                   , MEMORY_TYPE_INT   , sizeof(INT)   , ITEM_NONE, ITEM_NONE, "%#5d"      },       // PRJCT_RESULTS_ASCII_COOLING_STATUS
  { "Mirror status"                   , MEMORY_TYPE_INT   , sizeof(INT)   , ITEM_NONE, ITEM_NONE, "%#5d"      },       // PRJCT_RESULTS_ASCII_MIRROR_ERROR
  { "Compass angle"                   , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_COMPASS
  { "Pitch angle"                     , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_PITCH
  { "Roll angle"                      , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_ROLL
  { "Iterations number"               , MEMORY_TYPE_INT   , sizeof(INT)   , ITEM_NONE, ITEM_NONE, "%#6d"      },       // PRJCT_RESULTS_ASCII_ITER
  { "GOME2 scan direction"            , MEMORY_TYPE_INT   , sizeof(INT)   , ITEM_NONE, ITEM_NONE, "%#6d"      },       // PRJCT_RESULTS_ASCII_GOME2_SCANDIRECTION
  { "GOME2 SAA flag"                  , MEMORY_TYPE_INT   , sizeof(INT)   , ITEM_NONE, ITEM_NONE, "%#6d"      },       // PRJCT_RESULTS_ASCII_GOME2_SAA
  { "GOME2 sunglint risk flag"        , MEMORY_TYPE_INT   , sizeof(INT)   , ITEM_NONE, ITEM_NONE, "%#6d"      },       // PRJCT_RESULTS_ASCII_GOME2_SUNGLINT_RISK
  { "GOME2 sunglint high risk flag"   , MEMORY_TYPE_INT   , sizeof(INT)   , ITEM_NONE, ITEM_NONE, "%#6d"      },       // PRJCT_RESULTS_ASCII_GOME2_SUNGLINT_HIGHRISK
  { "GOME2 rainbow flag"              , MEMORY_TYPE_INT   , sizeof(INT)   , ITEM_NONE, ITEM_NONE, "%#6d"      },       // PRJCT_RESULTS_ASCII_GOME2_RAINBOW
  { "Diodes"                          , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_CCD_DIODES,
  { "Target Azimuth"                  , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_CCD_TARGETAZIMUTH,
  { "Target Elevation"                , MEMORY_TYPE_FLOAT , sizeof(float) , ITEM_NONE, ITEM_NONE, "%#12.6f"   },       // PRJCT_RESULTS_ASCII_CCD_TARGETELEVATION,
  { "Saturated"                       , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#5d"      },       // PRJCT_RESULTS_ASCII_SATURATED
  { "OMI index swath"                 , MEMORY_TYPE_INT   , sizeof(INT)   , ITEM_NONE, ITEM_NONE, "%#6d"      },       // PRJCT_RESULTS_ASCII_OMI_INDEX_SWATH,
  { "OMI index row"                   , MEMORY_TYPE_INT   , sizeof(int)   , ITEM_NONE, ITEM_NONE, "%#3d"      },       // PRJCT_RESULTS_ASCII_OMI_INDEX_ROW
  { "UAV servo sent position byte"    , MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#3d"      },       // PRJCT_RESULTS_ASCII_UAV_SERVO_BYTE_SENT
  { "UAV servo received position byte", MEMORY_TYPE_USHORT, sizeof(DoasUS), ITEM_NONE, ITEM_NONE, "%#3d"      }        // PRJCT_RESULTS_ASCII_UAV_SERVO_BYTE_RECEIVED
 };

typedef struct _NDSC_header
 {
  DoasCh investigator[20],
        instrument[12],
        station[12],
        species[12],
        startTime[20],
        endTime[20],
        qualityFlag[3];
 }
NDSC_HEADER;

PRJCT_RESULTS_FIELDS *outputFields;
DoasCh **outputColumns;
DoasCh OUTPUT_refFile[MAX_PATH_LEN+1];
INT   OUTPUT_nRec;

typedef struct _outputInfo
 {
  INT nbColumns;
  INT year,month,day;
  float longit,latit;
 }
OUTPUT_INFO;

OUTPUT_INFO *outputRecords;
INT outputNbDataSet,outputNbFields,outputNbRecords,outputMaxRecords;


// ===================
// GLOBAL DECLARATIONS
// ===================

DoasCh  OUTPUT_currentSpeFile[MAX_ITEM_TEXT_LEN+1],                              // complete results file name
       OUTPUT_currentAscFile[MAX_ITEM_TEXT_LEN+1];

// ===================
// STATIC DECLARATIONS
// ===================

double         OUTPUT_fluxes[MAX_FLUXES],OUTPUT_cic[MAX_CIC][2];                // fluxes and color indexes for ASCII results
INT            OUTPUT_NFluxes,OUTPUT_NCic;                                      // resp. number of fluxes and color indexes in previous buffers
AMF_SYMBOL    *OUTPUT_AmfSpace;                                                 // list of cross sections with associated AMF file
INT            OUTPUT_NAmfSpace,                                                // number of elements in previous buffer
               OUTPUT_chiSquareFlag,                                            // 1 to save the chi square in the output file
               OUTPUT_rmsFlag,                                                  // 1 to save the RMS in the output file
               OUTPUT_iterFlag,                                                 // 1 to save the number of iterations in the output file
               OUTPUT_refZmFlag,                                                // 1 to save the SZA of the reference spectrum in the output file
               OUTPUT_refShift,                                                 // 1 to save the shift of the reference spectrum on etalon
               OUTPUT_covarFlag,                                                // 1 to save the covariances in the output file
               OUTPUT_corrFlag,                                                 // 1 to save the correlations in the output file
               outputRunCalib,                                                  // 1 in run calibration mode
               outputCalibFlag;                                                 // <> 0 to save wavelength calibration parameters before analysis results

// ===============
// DATA PROCESSING
// ===============

// -----------------------------------------------------------------------------
// FUNCTION      OUTPUT_GetWveAmf
// -----------------------------------------------------------------------------
// PURPOSE       Correct a cross section using wavelength dependent AMF vector
//
// INPUT         pResults     output options for the selected cross section
//               Zm           the current solar zenith angle
//               lambda       the current wavelength calibration
//
// INPUT/OUTPUT  xs           the cross section to correct by wavelength dependent AMF
// OUTPUT        deriv2       second derivatives of the new cross section
// -----------------------------------------------------------------------------

RC OUTPUT_GetWveAmf(CROSS_RESULTS *pResults,double Zm,double *lambda,double *xs,double *deriv2)
 {
  // Declarations

  AMF_SYMBOL *pAmfSymbol;
  INDEX i;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  // This procedure applies only on wavelength dependent AMF

  if ((pResults->indexAmf!=ITEM_NONE) && (OUTPUT_AmfSpace!=NULL))
   {
   	pAmfSymbol=&OUTPUT_AmfSpace[pResults->indexAmf];

    for (i=0;i<NDET;i++)
     xs[i]*=(double)VECTOR_Table2_Index1(pAmfSymbol->Phi,pAmfSymbol->PhiLines,pAmfSymbol->PhiColumns,(double)lambda[i],(double)Zm);
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputGetAmf
// -----------------------------------------------------------------------------
// PURPOSE       Return the AMF from table at a specified zenith angle
//
// INPUT         pResults     output options for the selected cross section
//               Zm           the current solar zenith angle
//               Tm           the current number of seconds in order to retrieve
//                            AMF from a climatology table
//
// OUTPUT        pAmf         the calculated AMF
//
// RETURN        0 in case of success, any other value in case of error
// -----------------------------------------------------------------------------

RC OutputGetAmf(CROSS_RESULTS *pResults,double Zm,double Tm,double *pAmf)
 {
  // Declarations

  AMF_SYMBOL *pAmfSymbol;
  double Dte;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;
  *pAmf=(double)1.;

  if ((pResults->indexAmf!=ITEM_NONE) && (OUTPUT_AmfSpace!=NULL))
   {
    pAmfSymbol=&OUTPUT_AmfSpace[pResults->indexAmf];

    if (pAmfSymbol->Phi!=NULL)

     switch(pAmfSymbol->type)
      {
    // ------------------------------------------------------------------------
       case ANLYS_AMF_TYPE_CLIMATOLOGY :
        {
         Dte=(double) (ZEN_FNCaljda(&Tm)-1.)/365.;

         if ((Zm>=(double)pAmfSymbol->Phi[1][2]) && (Zm<=(double)pAmfSymbol->Phi[1][pAmfSymbol->PhiLines]) &&
             (Dte>=(double)pAmfSymbol->Phi[2][1]) && (Dte<=(double)pAmfSymbol->Phi[pAmfSymbol->PhiColumns][1]))

          *pAmf=(double)VECTOR_Table2_Index1(pAmfSymbol->Phi,pAmfSymbol->PhiLines,pAmfSymbol->PhiColumns,(double)Zm,(double)Dte);
        }
       break;
    // ------------------------------------------------------------------------
       case ANLYS_AMF_TYPE_SZA :

        if ((pAmfSymbol->deriv2!=NULL) &&
           ((rc=SPLINE_Vector(pAmfSymbol->Phi[1]+1,pAmfSymbol->Phi[2]+1,pAmfSymbol->deriv2[2]+1,pAmfSymbol->PhiLines,&Zm,pAmf,1,
                                 SPLINE_CUBIC,"OutputGetAmf"))!=ERROR_ID_NO))

         rc=ERROR_ID_AMF;

       break;
    // ------------------------------------------------------------------------
      }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      OUTPUT_ReadAmf
// -----------------------------------------------------------------------------
// PURPOSE       Load Air Mass Factors from file
//
// INPUT         symbolName   the name of the selected symbol
//               amfFileName  the name of the AMF file to load
//               amfType      the type of AMF to load
//
// OUTPUT        pIndexAmf    the index of AMF data in the AMF table
//
// RETURN        0 in case of success, any other value in case of error
// -----------------------------------------------------------------------------

RC OUTPUT_ReadAmf(DoasCh *symbolName,DoasCh *amfFileName,DoasCh amfType,INDEX *pIndexAmf)
 {
  // Declarations

  DoasCh  fileType,                                                             // file extension and type
        *oldColumn,*nextColumn;                                                 // go to the next column in record or the next record
  SZ_LEN symbolLength,fileLength;                                               // length of symbol and lines strings
  AMF_SYMBOL *pAmfSymbol;                                                       // pointer to an AMF symbol
  INDEX indexSymbol,indexColumn;                                                // indexes for loops and arrays
  INT PhiLines,PhiColumns,xsLines,xsColumns;                                    // dimensions of the AMF matrix
  FILE *amfFp;                                                                  // pointer to AMF file
  double **Phi,**deriv2,**xs,**xsDeriv2;                                        // pointers to resp. AMF buffer and second derivatives
  RC rc;                                                                        // return code

  // Initializations

  rc=ERROR_ID_NO;
  nextColumn=oldColumn=NULL;
  xsLines=xsColumns=0;
  indexSymbol=ITEM_NONE;
  deriv2=xs=xsDeriv2=NULL;
  amfFp=NULL;

  if (!(symbolLength=strlen(symbolName)) || !(fileLength=strlen(amfFileName)) || (amfType==ANLYS_AMF_TYPE_NONE))
   indexSymbol=ITEM_NONE;
  else if (OUTPUT_AmfSpace!=NULL) // ELSE LEVEL 0
   {
    // Retrieve the type of file

    fileType=amfType;

    // Search for symbol in list

    for (indexSymbol=0;indexSymbol<OUTPUT_NAmfSpace;indexSymbol++)
     {
      pAmfSymbol=&OUTPUT_AmfSpace[indexSymbol];

      if ((pAmfSymbol->type==fileType) &&
          (strlen(pAmfSymbol->symbolName)==symbolLength) &&
          (strlen(pAmfSymbol->amfFileName)==fileLength) &&
          !strcasecmp(pAmfSymbol->symbolName,symbolName) &&
          !strcasecmp(pAmfSymbol->amfFileName,amfFileName))

       break;
     }

    // A new symbol is found

    if (indexSymbol==OUTPUT_NAmfSpace)
     {
      // Symbol list is limited to MAX_SYMB symbols

      if (OUTPUT_NAmfSpace>=MAX_SYMB)
       rc=ERROR_SetLast("OUTPUT_ReadAmf",ERROR_TYPE_FATAL,ERROR_ID_OUT_OF_RANGE,OUTPUT_AmfSpace,0,MAX_SYMB-1);

      // File read out

      else    // ELSE LEVEL 1
       {
        if ((amfFp=fopen(amfFileName,"rt"))==NULL)
         rc=ERROR_SetLast("OUTPUT_ReadAmf",ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,amfFileName);
        else if (!(fileLength=STD_FileLength(amfFp)))
         rc=ERROR_SetLast("OUTPUT_ReadAmf",ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,amfFileName);
        else if (((nextColumn=(DoasCh *)MEMORY_AllocBuffer("OUTPUT_ReadAmf ","nextColumn",fileLength+1,1,0,MEMORY_TYPE_STRING))==NULL) ||
                 ((oldColumn=(DoasCh *)MEMORY_AllocBuffer("OUTPUT_ReadAmf ","oldColumn",fileLength+1,1,0,MEMORY_TYPE_STRING))==NULL))
         rc=ERROR_ID_ALLOC;
        else
         {
          if ((fileType==ANLYS_AMF_TYPE_CLIMATOLOGY) || (fileType==ANLYS_AMF_TYPE_SZA) || (fileType==ANLYS_AMF_TYPE_WAVELENGTH))
           rc=FILES_GetMatrixDimensions(amfFp,amfFileName,&PhiLines,&PhiColumns,"OUTPUT_ReadAmf",ERROR_TYPE_FATAL);
         }

        if (rc)
         goto EndOUTPUT_ReadAmf;

        // Allocate AMF matrix

        pAmfSymbol=&OUTPUT_AmfSpace[indexSymbol];
        fseek(amfFp,0L,SEEK_SET);

        if (((Phi=pAmfSymbol->Phi=(double **)MEMORY_AllocDMatrix("OUTPUT_ReadAmf ","Phi",0,PhiLines,1,PhiColumns))==NULL) ||
            ((fileType!=ANLYS_AMF_TYPE_CLIMATOLOGY) && (fileType!=ANLYS_AMF_TYPE_WAVELENGTH) &&
            ((deriv2=pAmfSymbol->deriv2=(double **)MEMORY_AllocDMatrix("OUTPUT_ReadAmf ","deriv2",1,PhiLines,2,PhiColumns))==NULL)))

         rc=ERROR_ID_ALLOC;

        else if (fileType==ANLYS_AMF_TYPE_CLIMATOLOGY)
         rc=FILES_LoadMatrix(amfFp,amfFileName,Phi,1,PhiLines,PhiColumns,"OUTPUT_ReadAmf",ERROR_TYPE_FATAL);
        else if (fileType==ANLYS_AMF_TYPE_SZA)
         rc=FILES_LoadMatrix(amfFp,amfFileName,Phi,1,PhiLines,PhiColumns,"OUTPUT_ReadAmf",ERROR_TYPE_FATAL);
        else if (fileType==ANLYS_AMF_TYPE_WAVELENGTH)
         rc=FILES_LoadMatrix(amfFp,amfFileName,Phi,1,PhiLines,PhiColumns,"OUTPUT_ReadAmf",ERROR_TYPE_FATAL);

        if (rc)
         goto EndOUTPUT_ReadAmf;

        // Second derivatives computations

        if ((fileType!=ANLYS_AMF_TYPE_CLIMATOLOGY) && (fileType!=ANLYS_AMF_TYPE_WAVELENGTH))
         for (indexColumn=2;(indexColumn<=PhiColumns) && !rc;indexColumn++)
          rc=SPLINE_Deriv2(Phi[1]+1,Phi[indexColumn]+1,deriv2[indexColumn]+1,PhiLines,"OUTPUT_ReadAmf ");

        // Add new symbol

        if (rc==ERROR_ID_NO)
         {
          pAmfSymbol=&OUTPUT_AmfSpace[indexSymbol];

          strcpy(pAmfSymbol->symbolName,symbolName);
          strcpy(pAmfSymbol->amfFileName,amfFileName);

          pAmfSymbol->type=fileType;
          pAmfSymbol->PhiLines=PhiLines;
          pAmfSymbol->PhiColumns=PhiColumns;
          pAmfSymbol->xsLines=xsLines;
          pAmfSymbol->xsColumns=xsColumns;

          OUTPUT_NAmfSpace++;
         }
       }     // END ELSE LEVEL 1
     }    // END if (indexSymbol==OUTPUT_NAmfSpace)

    if (indexSymbol>=OUTPUT_NAmfSpace)
     indexSymbol=ITEM_NONE;
   }   // END ELSE LEVEL 0

  // Return

  EndOUTPUT_ReadAmf :

  if (nextColumn!=NULL)
   MEMORY_ReleaseBuffer("OUTPUT_ReadAmf ","nextColumn",nextColumn);
  if (oldColumn!=NULL)
   MEMORY_ReleaseBuffer("OUTPUT_ReadAmf ","oldColumn",oldColumn);
  if (amfFp!=NULL)
   fclose(amfFp);

  *pIndexAmf=indexSymbol;

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputFlux
// -----------------------------------------------------------------------------
// PURPOSE       return the flux at a specified wavelength
//
// INPUT         pEngineContext    information on the current spectrum
//               waveLength   the wavelength
//
// RETURN        the flux calculated at the input wavelength
// -----------------------------------------------------------------------------

double OutputFlux(ENGINE_CONTEXT *pEngineContext,double waveLength)
 {
  // Declarations

  double flux;
  INDEX i,imin,imax;
  INT pixel;

  if ((waveLength<pEngineContext->buffers.lambda[0]) || (waveLength>pEngineContext->buffers.lambda[NDET-1]))
   flux=(double)0.;
  else
   {
    // Initialization

    pixel=FNPixel(pEngineContext->buffers.lambda,waveLength,NDET,PIXEL_CLOSEST);

    imin=max(pixel-3,0);
    imax=min(pixel+3,NDET-1);

    // Flux calculation

    for (i=imin,flux=(double)0.;i<=imax;i++)
     flux+=pEngineContext->buffers.spectrum[i];

    flux/=(double)(imax-imin+1);
   }

  // Return

  return flux;
 }

// -----------------------------------------------------------------------------
// FUNCTION      OUTPUT_ResetData
// -----------------------------------------------------------------------------
// PURPOSE       release and reset all data used for output
// -----------------------------------------------------------------------------

void OUTPUT_ResetData(void)
 {
  // Declarations

  FENO *pTabFeno;
  INDEX indexFeno,indexFenoColumn,indexTabCross,indexSymbol,indexField;
  CROSS_RESULTS *pResults;
  AMF_SYMBOL *pAmf;

  // Reset output part of data in analysis windows

  for (indexFenoColumn=0;indexFenoColumn<MAX_SWATHSIZE;indexFenoColumn++)
   {
    for (indexFeno=0;indexFeno<MAX_FENO;indexFeno++)
     {
      pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

      // Browse symbols

      for (indexTabCross=0;indexTabCross<MAX_FIT;indexTabCross++)
       {
        pResults=&pTabFeno->TabCrossResults[indexTabCross];

        pResults->indexAmf=ITEM_NONE;
     // -------------------------------------------
        pResults->StoreAmf=
        pResults->StoreShift=
        pResults->StoreStretch=
        pResults->StoreScale=
        pResults->StoreError=
        pResults->StoreSlntCol=
        pResults->StoreSlntErr=
        pResults->StoreVrtCol=
        pResults->StoreVrtErr=(DoasCh)0;
     // -------------------------------------------
        pResults->ResCol=(double)0.;
     // -------------------------------------------
        pResults->Amf=
        pResults->SlntCol=
        pResults->SlntErr=
        pResults->VrtCol=
        pResults->VrtErr=(double)9999.;
     // -------------------------------------------
        pResults->SlntFact=
        pResults->VrtFact=(double)1.;
       }
     }
   }

  // Release AMF matrices

  if (OUTPUT_AmfSpace!=NULL)
   {
    for (indexSymbol=0;indexSymbol<MAX_SYMB;indexSymbol++)
     {
      pAmf=&OUTPUT_AmfSpace[indexSymbol];

      if (pAmf->Phi!=NULL)
       MEMORY_ReleaseDMatrix("OUTPUT_ResetData ","Phi",pAmf->Phi,1,pAmf->PhiColumns,0);
      if (pAmf->deriv2!=NULL)
       MEMORY_ReleaseDMatrix("OUTPUT_ResetData ","deriv2",pAmf->deriv2,2,pAmf->PhiColumns,1);
      if (pAmf->xs!=NULL)
       MEMORY_ReleaseDMatrix("OUTPUT_ResetData ","xs",pAmf->xs,0,pAmf->xsColumns-1,0);
      if (pAmf->xsDeriv2!=NULL)
       MEMORY_ReleaseDMatrix("OUTPUT_ResetData ","xsDeriv2",pAmf->xsDeriv2,1,pAmf->xsColumns-1,1);
     }

    memset(OUTPUT_AmfSpace,0,sizeof(AMF_SYMBOL)*MAX_SYMB);
   }

  // Reset Amf

  OUTPUT_NAmfSpace=0;

  memset(OUTPUT_currentSpeFile,0,MAX_ITEM_TEXT_LEN+1);
  memset(OUTPUT_currentAscFile,0,MAX_ITEM_TEXT_LEN+1);

  for (indexField=0;indexField<outputNbFields;indexField++)
   {
    if (outputColumns[indexField]!=NULL)
     MEMORY_ReleaseBuffer("OUTPUT_ResetData",outputFields[indexField].fieldName,outputColumns[indexField]);
   }

  memset(outputFields,0,sizeof(PRJCT_RESULTS_FIELDS)*MAX_FIELDS);
  memset(outputColumns,0,sizeof(DoasCh *)*MAX_FIELDS);

  if (outputRecords!=NULL)
   MEMORY_ReleaseBuffer("OUTPUT_ResetData","outputRecords",outputRecords);

  outputRecords=NULL;

  OUTPUT_chiSquareFlag=
  OUTPUT_rmsFlag=
  OUTPUT_iterFlag=
  OUTPUT_refZmFlag=
  OUTPUT_refShift=
  OUTPUT_covarFlag=
  OUTPUT_corrFlag=
  outputRunCalib=
  outputCalibFlag=
  outputNbFields=
  outputNbDataSet=
  outputNbRecords=
  outputMaxRecords=0;
 }

//  =======================
//  REGISTER DATA TO OUTPUT
//  =======================

// -----------------------------------------------------------------------------
// FUNCTION      OutputRegister
// -----------------------------------------------------------------------------
// PURPOSE       Register a field to output
//
// INPUT         titlePart1   1st part of the field name
//               titlePart2   2nd part of the field name (optional)
//               titlePart3   3rd part of the field name (optional)
//               fieldType    the type of field to register (data type or format)
//               fieldSize    the size of one item
//               fieldDim1    1st dimension of the field
//               fieldDim2    2nd dimension of the field
//               format       the format string to use to output the data
// -----------------------------------------------------------------------------

void OutputRegister(DoasCh *titlePart1,DoasCh *titlePart2,DoasCh *titlePart3,INT fieldType,INT fieldSize,
                    INT fieldDim1,INT fieldDim2,DoasCh *format)
 {
  // Declarations

  DoasCh title[MAX_STR_LEN+1];
  PRJCT_RESULTS_FIELDS *pField;

  if (outputNbFields<MAX_FIELDS)
   {
    pField=&outputFields[outputNbFields++];

    sprintf(title,"%s%s%s",titlePart1,titlePart2,titlePart3);

    strncpy(pField->fieldName,title,2*MAX_ITEM_NAME_LEN);
    strncpy(pField->fieldFormat,format,MAX_ITEM_NAME_LEN);
    pField->fieldType=fieldType;
    pField->fieldSize=fieldSize;
    pField->fieldDim1=fieldDim1;
    pField->fieldDim2=fieldDim2;
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputRegisterFluxes
// -----------------------------------------------------------------------------
// PURPOSE       Register fluxes
//
// INPUT         pEngineContext   structure including information on project options
// -----------------------------------------------------------------------------

void OutputRegisterFluxes(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  DoasCh  columnTitle[MAX_ITEM_NAME_LEN+1],
        *ptrOld,*ptrNew;

  // Initializations

  OUTPUT_NFluxes=OUTPUT_NCic=0;

  // Fluxes

  for (ptrOld=pEngineContext->project.asciiResults.fluxes;(ptrOld!=NULL) && (strlen(ptrOld)!=0);ptrOld=ptrNew)
   {
    if (OUTPUT_NFluxes>=MAX_FLUXES)
     break;

    if (sscanf(ptrOld,"%lf",&OUTPUT_fluxes[OUTPUT_NFluxes])==1)
     {
      sprintf(columnTitle,"Fluxes %g",OUTPUT_fluxes[OUTPUT_NFluxes]);
      OutputRegister(columnTitle,"","",MEMORY_TYPE_DOUBLE,sizeof(double),ITEM_NONE,ITEM_NONE,"%#15.6le");
      OUTPUT_NFluxes++;
     }

    if ((ptrNew=strchr(ptrOld,';'))!=NULL)
     ptrNew++;
   }

  // Color indexes

  for (ptrOld=pEngineContext->project.asciiResults.cic;(ptrOld!=NULL) && (strlen(ptrOld)!=0);ptrOld=ptrNew)
   {
    if (OUTPUT_NCic>=MAX_CIC)
     break;

    if (sscanf(ptrOld,"%lf/%lf",&OUTPUT_cic[OUTPUT_NCic][0],&OUTPUT_cic[OUTPUT_NCic][1])>=2)
     {
      sprintf(columnTitle,"%g/%g",OUTPUT_cic[OUTPUT_NCic][0],OUTPUT_cic[OUTPUT_NCic][1]);
      OutputRegister(columnTitle,"","",MEMORY_TYPE_DOUBLE,sizeof(double),ITEM_NONE,ITEM_NONE,"%#15.6le");
      OUTPUT_NCic++;
     }

    if ((ptrNew=strchr(ptrOld,';'))!=NULL)
     ptrNew++;
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputRegisterFields
// -----------------------------------------------------------------------------
// PURPOSE       Register all the fields that are not parameters of the fit
//
// INPUT         pEngineContext   structure including information on project options
// -----------------------------------------------------------------------------

void OutputRegisterFields(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  PRJCT_RESULTS_FIELDS *pField;
  PRJCT_RESULTS_ASCII *pResults;
  PROJECT             *pProject;                                                // pointer to project data
  INDEX                indexField,
                       j;
  int satelliteFlag;

  // Initializations

  pProject=(PROJECT *)&pEngineContext->project;
  pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;

  satelliteFlag=((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
                 (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
                 (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
                 (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
                 (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2))?1:0;

  // Browse fields

  for (j=0;j<pResults->fieldsNumber;j++)
   {
    indexField=pResults->fieldsFlag[j];
    pField=&PRJCT_resultsAscii[indexField];

    // Fields to define for each spectral window

    if (indexField==PRJCT_RESULTS_ASCII_CHI)
     OUTPUT_chiSquareFlag=1;
    else if (indexField==PRJCT_RESULTS_ASCII_RMS)
     OUTPUT_rmsFlag=1;
    else if (indexField==PRJCT_RESULTS_ASCII_REFZM)
     OUTPUT_refZmFlag=(outputRunCalib)?0:1;
    else if (indexField==PRJCT_RESULTS_ASCII_REFSHIFT)
     OUTPUT_refShift=(outputRunCalib)?0:1;
    else if (indexField==PRJCT_RESULTS_ASCII_COVAR)
     OUTPUT_covarFlag=(outputRunCalib)?0:1;
    else if (indexField==PRJCT_RESULTS_ASCII_CORR)
     OUTPUT_chiSquareFlag=(outputRunCalib)?0:1;
    else if (indexField==PRJCT_RESULTS_ASCII_ITER)
     OUTPUT_iterFlag=1;
    else if (indexField==PRJCT_RESULTS_ASCII_CCD_DIODES)
     {
     	OutputRegister(pField->fieldName,"(1)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
     	OutputRegister(pField->fieldName,"(2)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
     	OutputRegister(pField->fieldName,"(3)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
     	OutputRegister(pField->fieldName,"(4)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
     }

    // Geolocation for satellite data

    else if (satelliteFlag &&
           ((indexField==PRJCT_RESULTS_ASCII_LONGIT) ||
            (indexField==PRJCT_RESULTS_ASCII_LATIT)))
     {
      OutputRegister(pField->fieldName,"(1)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
      OutputRegister(pField->fieldName,"(2)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
      OutputRegister(pField->fieldName,"(3)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
      OutputRegister(pField->fieldName,"(4)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);

      OutputRegister(pField->fieldName,"(pixel center)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
     }
    else if (satelliteFlag &&
           ((indexField==PRJCT_RESULTS_ASCII_SZA) ||
            (indexField==PRJCT_RESULTS_ASCII_AZIM) ||
            (indexField==PRJCT_RESULTS_ASCII_LOS_ZA) ||
            (indexField==PRJCT_RESULTS_ASCII_LOS_AZIMUTH)))
     {
      OutputRegister(pField->fieldName,"(A)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
      OutputRegister(pField->fieldName,"(B)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
      OutputRegister(pField->fieldName,"(C)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
     }

    // Other data

    else
     OutputRegister(pField->fieldName,"","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputRegisterCalib
// -----------------------------------------------------------------------------
// PURPOSE       Register all the calibration fields for each window of the
//               wavelength calibration interval
//
// INPUT         indexFenoK   the current window of the wavelength calibration interval
// -----------------------------------------------------------------------------

void OutputRegisterCalib(INDEX indexFenoK,INDEX indexFenoColumn)
 {
  // Declarations

  DoasCh                windowName[MAX_ITEM_NAME_LEN+1],                         // the name of the current spectral window
                       symbolName[MAX_ITEM_NAME_LEN+1];                         // the name of a symbol
  FENO                *pTabFeno;
  CROSS_REFERENCE     *TabCross;
  CROSS_RESULTS       *pTabCrossResults;
  INDEX                indexTabCross;
  INT                  nbWin,dim1,dim2;

  nbWin=KURUCZ_buffers[indexFenoColumn].Nb_Win;

  dim1=nbWin;
  dim2=ITEM_NONE;

  pTabFeno=&TabFeno[indexFenoColumn][indexFenoK];
  TabCross=pTabFeno->TabCross;

  // Register columns

  sprintf(windowName,"Calib(%d/%d).",indexFenoColumn+1,ANALYSE_swathSize);

  OutputRegister(windowName,"RMS","",MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");

  // Fitted parameters

  for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
   {
    pTabCrossResults=&pTabFeno->TabCrossResults[indexTabCross];
    sprintf(symbolName,"(%s)",WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName);

    if (pTabCrossResults->StoreSlntCol)                         // Slant column
     OutputRegister(windowName,"SlCol",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
    if (pTabCrossResults->StoreSlntErr)                         // Error on slant column
     OutputRegister(windowName,"SlErr",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");

    if (pTabCrossResults->StoreShift)              // Shift
     {
      OutputRegister(windowName,"Shift",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
      if (pTabCrossResults->StoreError)
       OutputRegister(windowName,"Err Shift",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
     }

    if (pTabCrossResults->StoreStretch)            // Stretch
     {
      OutputRegister(windowName,"Stretch",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
      if (pTabCrossResults->StoreError)
       OutputRegister(windowName,"Err Stretch",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
      OutputRegister(windowName,"Stretch2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
      if (pTabCrossResults->StoreError)
       OutputRegister(windowName,"Err Stretch2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
     }

    if (pTabCrossResults->StoreScale)              // Scale
     {
      OutputRegister(windowName,"Scale",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
      if (pTabCrossResults->StoreError)
       OutputRegister(windowName,"Err Scale",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
      OutputRegister(windowName,"Scale2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
      if (pTabCrossResults->StoreError)
       OutputRegister(windowName,"Err Scale2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
     }

    if (pTabCrossResults->StoreParam)              // Param
     {
      OutputRegister(windowName,WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName,"",MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
      if (pTabCrossResults->StoreParamError)
       OutputRegister(windowName,"Err ",WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
     }
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputRegisterParam
// -----------------------------------------------------------------------------
// PURPOSE       Register all the parameters of the fit
//
// INPUT         pEngineContext   structure including information on project options
//               hiddenFlag  indicates if the calling windows is hidden in the projects tree
// -----------------------------------------------------------------------------

void OutputRegisterParam(ENGINE_CONTEXT *pEngineContext,INT hiddenFlag)
 {
  // Declarations

  DoasCh               windowName[MAX_ITEM_NAME_LEN+1],                         // the name of the current spectral window
                       symbolName[MAX_ITEM_NAME_LEN+1];                         // the name of a symbol
  FENO                *pTabFeno;
  CROSS_REFERENCE     *TabCross;
  CROSS_RESULTS       *pTabCrossResults;
  INDEX                indexFeno,indexWin,indexFenoColumn,
                       indexTabCross,indexTabCross2;
  INT                  nbWin,dim1,dim2;

  dim1=ITEM_NONE;
  dim2=ITEM_NONE;


  for (indexFenoColumn=0;indexFenoColumn<ANALYSE_swathSize;indexFenoColumn++)
   {
    nbWin=(hiddenFlag)?KURUCZ_buffers[indexFenoColumn].Nb_Win:1;

   	if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) ||
   	     pEngineContext->project.instrumental.omi.omiTracks[indexFenoColumn])
   	 {
      // Browse analysis windows

      for (indexWin=0;indexWin<nbWin;indexWin++)

       for (indexFeno=0;indexFeno<NFeno;indexFeno++)
        {
         pTabFeno=&TabFeno[indexFenoColumn][indexFeno];
         TabCross=pTabFeno->TabCross;

         if (pTabFeno->hidden==hiddenFlag)
          {
          	if (!pTabFeno->hidden)
          	 {
             // Not fitted parameters

             sprintf(windowName,"%s.",pTabFeno->windowName);

             if (OUTPUT_refZmFlag)
              OutputRegister(windowName,"RefZm","",MEMORY_TYPE_FLOAT,sizeof(float),dim1,dim2,"%#8.3f");
             if (OUTPUT_refShift)
              OutputRegister(windowName,"Ref2/Ref1 Shift","",MEMORY_TYPE_FLOAT,sizeof(float),dim1,dim2,"%#8.3f");

             if (OUTPUT_covarFlag && (pTabFeno->svd.covar!=NULL))
              for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
               if (TabCross[indexTabCross].IndSvdA>0)
                for (indexTabCross2=0;indexTabCross2<indexTabCross;indexTabCross2++)
                 if (TabCross[indexTabCross2].IndSvdA>0)
                  {
                   sprintf(symbolName,"(%s,%s)",
                          WorkSpace[TabCross[indexTabCross2].Comp].symbolName,
                          WorkSpace[TabCross[indexTabCross].Comp].symbolName);
                   OutputRegister(windowName,"Covar",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
                  }

             if (OUTPUT_corrFlag && (pTabFeno->svd.covar!=NULL))
              for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
               if (TabCross[indexTabCross].IndSvdA>0)
                for (indexTabCross2=0;indexTabCross2<indexTabCross;indexTabCross2++)
                 if (TabCross[indexTabCross2].IndSvdA>0)
                  {
                   sprintf(symbolName,"(%s,%s)",
                          WorkSpace[TabCross[indexTabCross2].Comp].symbolName,
                          WorkSpace[TabCross[indexTabCross].Comp].symbolName);
                   OutputRegister(windowName,"Corr",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
                  }
            }
           else if (outputRunCalib)
            sprintf(windowName,"RunCalib(%d).",indexWin+1);

           if (OUTPUT_chiSquareFlag)
            OutputRegister(windowName,"Chi","",MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
           if (OUTPUT_rmsFlag)
            OutputRegister(windowName,"RMS","",MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
           if (OUTPUT_iterFlag)
            OutputRegister(windowName,"iter","",MEMORY_TYPE_INT,sizeof(int),dim1,dim2,"%#6d");

           // Fitted parameters

           for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
            {
             pTabCrossResults=&pTabFeno->TabCrossResults[indexTabCross];
             sprintf(symbolName,"(%s)",WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName);

             if (pTabCrossResults->indexAmf!=ITEM_NONE)
              {
               if (pTabCrossResults->StoreAmf)              // AMF
                OutputRegister(windowName,"AMF",symbolName,MEMORY_TYPE_FLOAT,sizeof(double),dim1,dim2,"%#8.3lf");
               if (pTabCrossResults->StoreVrtCol)           // Vertical column
                OutputRegister(windowName,"VCol",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
               if (pTabCrossResults->StoreVrtErr)           // Error on vertical column
                OutputRegister(windowName,"VErr",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
              }

             if (pTabCrossResults->StoreSlntCol)                         // Slant column
              OutputRegister(windowName,"SlCol",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
             if (pTabCrossResults->StoreSlntErr)                         // Error on slant column
              OutputRegister(windowName,"SlErr",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");

             if (pTabCrossResults->StoreShift)              // Shift
              {
               OutputRegister(windowName,"Shift",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
               if (pTabCrossResults->StoreError)
                OutputRegister(windowName,"Err Shift",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
              }

             if (pTabCrossResults->StoreStretch)            // Stretch
              {
               OutputRegister(windowName,"Stretch",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
               if (pTabCrossResults->StoreError)
                OutputRegister(windowName,"Err Stretch",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
               OutputRegister(windowName,"Stretch2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
               if (pTabCrossResults->StoreError)
                OutputRegister(windowName,"Err Stretch2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
              }

             if (pTabCrossResults->StoreScale)              // Scale
              {
               OutputRegister(windowName,"Scale",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
               if (pTabCrossResults->StoreError)
                OutputRegister(windowName,"Err Scale",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
               OutputRegister(windowName,"Scale2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
               if (pTabCrossResults->StoreError)
                OutputRegister(windowName,"Err Scale2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
              }

             if (pTabCrossResults->StoreParam)              // Param
              {
               OutputRegister(windowName,WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName,"",MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
               if (pTabCrossResults->StoreParamError)
                OutputRegister(windowName,"Err ",WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
              }
            }  // for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
          }  // if (pTabFeno->hidden==hiddenFlag)
        }  // for (indexFeno=0;indexFeno<NFeno;indexFeno++)

       break; // even for OMI, register param only one time
     }   // if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) ||
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      OUTPUT_RegisterData
// -----------------------------------------------------------------------------
// PURPOSE       Register all the data to output
//
// INPUT         pEngineContext   structure including information on project options
// -----------------------------------------------------------------------------

RC OUTPUT_RegisterData(ENGINE_CONTEXT *pEngineContext)
 {
 	// Declarations

  INDEX indexFeno,indexFenoK,indexFeno1;
  PROJECT *pProject;
  PRJCT_RESULTS_ASCII *pResults;
  INDEX indexFenoColumn;
 	RC rc;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("OUTPUT_RegisterData",DEBUG_FCTTYPE_FILE);
  #endif

 	// Initializations

  pProject=&pEngineContext->project;
  pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;
  outputCalibFlag=outputRunCalib=0;
  indexFenoK=indexFeno1=ITEM_NONE;
 	rc=ERROR_ID_NO;

  if (pProject->asciiResults.analysisFlag || pProject->asciiResults.calibFlag)
   {
    if (THRD_id==THREAD_TYPE_ANALYSIS)
     {
      // Satellites measurements and automatic reference selection : save information on the selected reference

   	  if (((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&
            pEngineContext->analysisRef.refAuto)                                           // automatic reference is requested for at least one analysis window
       {
   	    OutputRegister("Reference file","","",MEMORY_TYPE_STRING,MAX_PATH_LEN+1,1,1,"%s");
   	    OutputRegister("Number of records selected for the reference","","",MEMORY_TYPE_INT,sizeof(int),1,1,"%d");
   	   }

     	// Save information on the calibration

     	if (pResults->calibFlag)
     	 {
     	 	for (indexFenoColumn=0;indexFenoColumn<ANALYSE_swathSize;indexFenoColumn++)
     	 	 {
   	      if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) ||
   	           pEngineContext->project.instrumental.omi.omiTracks[indexFenoColumn])
   	       {
            for (indexFeno=0;indexFeno<NFeno;indexFeno++)
             if (TabFeno[indexFenoColumn][indexFeno].hidden)
              indexFenoK=indexFeno;
             else if (!TabFeno[indexFenoColumn][indexFeno].hidden &&
                     ((TabFeno[indexFenoColumn][indexFeno].useKurucz==ANLYS_KURUCZ_REF) ||
                      (TabFeno[indexFenoColumn][indexFeno].useKurucz==ANLYS_KURUCZ_REF_AND_SPEC)) &&
                      !TabFeno[indexFenoColumn][indexFeno].rcKurucz)
              {
              	if (indexFeno1==ITEM_NONE)
              	 indexFeno1=indexFeno;
               outputCalibFlag++;
              }

            if (indexFenoK==ITEM_NONE)
             outputCalibFlag=0;

            if (outputCalibFlag)
             OutputRegisterCalib(indexFenoK,indexFenoColumn);
           }
         }
       }
   	 }

    // Run calibration on measurement spectra

    else if (THRD_id==THREAD_TYPE_KURUCZ)
     outputRunCalib++;

    OutputRegisterFields(pEngineContext);                                       // do not depend on swath size
    OutputRegisterParam(pEngineContext,(THRD_id==THREAD_TYPE_ANALYSIS)?0:1);
    OutputRegisterFluxes(pEngineContext);                                       // do not depend on swath size
   }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("OUTPUT_RegisterData",rc);
  #endif

  return rc;
 }

// ===================
// KEEP DATA TO OUTPUT
// ===================

// -----------------------------------------------------------------------------
// FUNCTION      OutputCalib
// -----------------------------------------------------------------------------
// PURPOSE       Save data related to the wavelength calibration
//
// INPUT         pEngineContext   structure including information on project options
// -----------------------------------------------------------------------------

void OutputCalib(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  FENO                *pTabFeno;
  CROSS_REFERENCE     *TabCross;
  CROSS_RESULTS       *pTabCrossResults;
  INDEX                indexFeno,indexFenoColumn,indexFenoK,indexColumnOld,indexColumn,indexTabCross,indexWin;
  INT                  nbWin;
  double               defaultValue;

  PRJCT_RESULTS_ASCII *pResults;
  PROJECT             *pProject;                                                // pointer to project data

  // Initializations

  defaultValue=(double)9999.;

  pProject=(PROJECT *)&pEngineContext->project;
  pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;

  indexColumnOld=indexColumn=0;

  for (indexFenoColumn=0;indexFenoColumn<ANALYSE_swathSize;indexFenoColumn++)
   {
   	if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) ||
   	     pEngineContext->project.instrumental.omi.omiTracks[indexFenoColumn])
   	 {
      nbWin=KURUCZ_buffers[indexFenoColumn].Nb_Win;
      indexFenoK=ITEM_NONE;

      for (indexFeno=0;(indexFeno<NFeno) && (indexFenoK==ITEM_NONE);indexFeno++)
       if (TabFeno[indexFenoColumn][indexFeno].hidden==1)
        indexFenoK=indexFeno;

      if (outputCalibFlag && (indexFeno<NFeno))
       {
        pTabFeno=&TabFeno[indexFenoColumn][indexFenoK];
        TabCross=pTabFeno->TabCross;

        for (indexWin=0;indexWin<nbWin;indexWin++)
         {
         	// Bypass predefined data (e.g. reference spectrum record)

          for (indexColumn=indexColumnOld;(outputFields[indexColumn].fieldDim2!=ITEM_NONE);indexColumn++);

          // RMS

          ((double *)outputColumns[indexColumn++])[indexWin]=KURUCZ_buffers[indexFenoColumn].KuruczFeno[indexFeno].rms[indexWin];

          // Fitted parameters

          for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
           {
            pTabCrossResults=&pTabFeno->TabCrossResults[indexTabCross];

            pTabCrossResults=&KURUCZ_buffers[indexFenoColumn].KuruczFeno[indexFeno].results[indexWin][indexTabCross];

            if (pTabCrossResults->StoreSlntCol)            // Slant column
             	((double *)outputColumns[indexColumn++])[indexWin]=
             	(!pTabFeno->rc && (pTabCrossResults->SlntFact!=(double)0.))?
             	 (double)pTabCrossResults->SlntCol/pTabCrossResults->SlntFact:(double)defaultValue;

            if (pTabCrossResults->StoreSlntErr)            // Error on slant column
             	((double *)outputColumns[indexColumn++])[indexWin]=
             	(!pTabFeno->rc && (pTabCrossResults->SlntFact!=(double)0.))?
             	 (double)pTabCrossResults->SlntErr/pTabCrossResults->SlntFact:(double)defaultValue;

            if (pTabCrossResults->StoreShift)              // Shift
             {
              ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->Shift:(double)defaultValue;
              if (pTabCrossResults->StoreError)
               ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaShift:(double)defaultValue;
             }

            if (pTabCrossResults->StoreStretch)            // Stretch
             {
              ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->Stretch:(double)defaultValue;
              if (pTabCrossResults->StoreError)
               ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaStretch:(double)defaultValue;
              ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->Stretch2:(double)defaultValue;
              if (pTabCrossResults->StoreError)
               ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaStretch2:(double)defaultValue;
             }

            if (pTabCrossResults->StoreScale)              // Scale
             {
              ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->Scale:(double)defaultValue;
              if (pTabCrossResults->StoreError)
               ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaScale:(double)defaultValue;
              ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->Scale2:(double)defaultValue;
              if (pTabCrossResults->StoreError)
               ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaScale2:(double)defaultValue;
             }

            if (pTabCrossResults->StoreParam)              // Param
             {
              ((double *)outputColumns[indexColumn++])[indexWin]=
               (double)((!pTabFeno->rc)?
                       ((TabCross[indexTabCross].IndSvdA)?pTabCrossResults->SlntCol:pTabCrossResults->Param):defaultValue);
              if (pTabCrossResults->StoreParamError)
              ((double *)outputColumns[indexColumn++])[indexWin]=
               (double)((!pTabFeno->rc)?
                       ((TabCross[indexTabCross].IndSvdA)?pTabCrossResults->SlntErr:pTabCrossResults->SigmaParam):(double)defaultValue);
             }
           }
         }

        indexColumnOld=indexColumn;
       }
     }
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputSaveRecord
// -----------------------------------------------------------------------------
// PURPOSE       Save all the data on the current record (including fitted parameters)
//
// INPUT         pEngineContext   structure including information on project options
//               hiddenFlag  0 to save analysis results, 1 to save calibration results
// -----------------------------------------------------------------------------

void OutputSaveRecord(ENGINE_CONTEXT *pEngineContext,INT hiddenFlag,INDEX indexFenoColumn)
 {
  // Declarations

  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  PRJCT_RESULTS_FIELDS *pField;
  RECORD_INFO *pRecordInfo;

  FENO                *pTabFeno;
  CROSS_REFERENCE     *TabCross;
  CROSS_RESULTS       *pTabCrossResults;
  PROJECT             *pProject;                                                // pointer to project data
  PRJCT_RESULTS_ASCII *pResults;                                                // pointer to results part of project
  INDEX                indexRecord,
                       indexColumn,
                       indexFeno,
                       indexTabCross,indexTabCross2,
                       indexField,                                              // browse fields of record
                       indexFluxes,                                             // browse selected wavelengths for fluxes
                       indexCic,                                                // browse selected wavelengths for color indexes
                       i,k;
  double               flux;                                                    // temporary variable
  double               defaultValue;
  INT                  nbWin,indexWin;

  nbWin=(hiddenFlag)?KURUCZ_buffers[indexFenoColumn].Nb_Win:1;

  // Initializations

  pProject=(PROJECT *)&pEngineContext->project;
  pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;
  pRecordInfo=(RECORD_INFO *)&pEngineContext->recordInfo;

  pOrbitFile=(pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)?&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex]:NULL;

  if (pResults->analysisFlag)
   {
   	defaultValue=(double)9999.;

    for (indexColumn=0;(outputFields[indexColumn].fieldDim1!=ITEM_NONE);indexColumn++);

    indexRecord=outputNbRecords++;

    // ---------------------------------
    // INFORMATION ON THE CURRENT RECORD
    // ---------------------------------

    for (indexField=0,k=indexColumn;indexField<pResults->fieldsNumber;indexField++,k++)
     {
      pField=&outputFields[k];

      switch(pResults->fieldsFlag[indexField])
       {
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_SPECNO :
         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=(DoasUS)pEngineContext->indexRecord;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_NAME :
         strncpy(&outputColumns[indexColumn++][indexRecord*pField->fieldSize],pRecordInfo->Nom,pField->fieldSize);
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_DATE :

         sprintf(&outputColumns[indexColumn++][indexRecord*pField->fieldSize],"%02d/%02d/%d",
                (INT) pRecordInfo->present_day.da_day,
                (INT) pRecordInfo->present_day.da_mon,
                (INT) pRecordInfo->present_day.da_year);

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_TIME :

         sprintf(&outputColumns[indexColumn++][indexRecord*pField->fieldSize],"%02d:%02d:%02d",
                (INT) pRecordInfo->present_time.ti_hour,
                (INT) pRecordInfo->present_time.ti_min,
                (INT) pRecordInfo->present_time.ti_sec);

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_YEAR :
         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=(DoasUS)pRecordInfo->present_day.da_year;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_DATE_TIME :

         memset(&outputColumns[indexColumn][indexRecord*pField->fieldSize],0,pField->fieldSize);

         sprintf(&outputColumns[indexColumn][indexRecord*pField->fieldSize],"%4d%02d%02d%02d%02d%02d",
                (INT) pRecordInfo->present_day.da_year,
                (INT) pRecordInfo->present_day.da_mon,
                (INT) pRecordInfo->present_day.da_day,
                (INT) pRecordInfo->present_time.ti_hour,
                (INT) pRecordInfo->present_time.ti_min,
                (INT) pRecordInfo->present_time.ti_sec);

         if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
          sprintf(&outputColumns[indexColumn][indexRecord*pField->fieldSize+14],".%03d",(INT)SCIA_ms);
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
          sprintf(&outputColumns[indexColumn][indexRecord*pField->fieldSize+14],".%06d",(INT)GOME2_ms);

         indexColumn++;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_STARTTIME :

         memset(&outputColumns[indexColumn][indexRecord*pField->fieldSize],0,pField->fieldSize);

         sprintf(&outputColumns[indexColumn][indexRecord*pField->fieldSize],"%02d%02d%02d",
                (INT) pRecordInfo->startTime.ti_hour,
                (INT) pRecordInfo->startTime.ti_min,
                (INT) pRecordInfo->startTime.ti_sec);

         indexColumn++;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_ENDTIME :

         memset(&outputColumns[indexColumn][indexRecord*pField->fieldSize],0,pField->fieldSize);

         sprintf(&outputColumns[indexColumn][indexRecord*pField->fieldSize],"%02d%02d%02d",
                (INT) pRecordInfo->endTime.ti_hour,
                (INT) pRecordInfo->endTime.ti_min,
                (INT) pRecordInfo->endTime.ti_sec);

         indexColumn++;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_JULIAN :
         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=(DoasUS)ZEN_FNCaljda(&pRecordInfo->Tm);
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_JDFRAC :
         ((double *)outputColumns[indexColumn++])[indexRecord]=(double)ZEN_FNCaljda(&pRecordInfo->Tm)+ZEN_FNCaldti(&pRecordInfo->Tm)/24.;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_TIFRAC :

         if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
             (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2))
          ((double *)outputColumns[indexColumn++])[indexRecord]=(double)pRecordInfo->TimeDec;
         else
          ((double *)outputColumns[indexColumn++])[indexRecord]=(double)ZEN_FNCaldti(&pRecordInfo->Tm);

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_SCANS :
         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=(DoasUS)pRecordInfo->NSomme;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_NREJ :
         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=(DoasUS)pRecordInfo->rejected;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_TINT :
         ((double *)outputColumns[indexColumn++])[indexRecord]=(double)pRecordInfo->Tint;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_SZA :

         if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) || (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS))
          {
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.solZen[0];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.solZen[1];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.solZen[2];
          }
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
          {
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.solZen[0];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.solZen[1];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.solZen[2];
          }
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
          {
           if (pOrbitFile->gdpBinHeader.version<40)
            {
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo3.szaArray[0];k++;
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo3.szaArray[1];k++;
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo3.szaArray[2];
            }
           else
            {
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.szaArrayBOA[0];k++;
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.szaArrayBOA[1];k++;
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.szaArrayBOA[2];
            }
          }
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII)
          {
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome.sza[0];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome.sza[1];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome.sza[2];
          }
         else
          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->Zm;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_AZIM :

         if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
             (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS))
          {
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.solAzi[0];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.solAzi[1];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.solAzi[2];
          }
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
          {
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.solAzi[0];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.solAzi[1];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.solAzi[2];
          }
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII)
          {
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome.azim[0];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome.azim[1];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome.azim[2];
          }
         else if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) && (pOrbitFile->gdpBinHeader.version<40))
          {
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo3.aziArray[0];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo3.aziArray[1];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo3.aziArray[2];
          }
         else if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) && (pOrbitFile->gdpBinHeader.version>=40))
          {
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.aziArrayBOA[0];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.aziArrayBOA[1];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.aziArrayBOA[2];
          }
         else
          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->Azimuth;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_TDET :
         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->TDet;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_SKY :
         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=(DoasUS)pRecordInfo->SkyObs;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_BESTSHIFT :
         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->BestShift;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_PIXEL :
         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=
         ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) || (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))?(DoasUS)pRecordInfo->gome.pixelNumber:0;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_PIXEL_TYPE :
         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=
         ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) || (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))?(DoasUS)pRecordInfo->gome.pixelType:0;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_ORBIT :

         if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) || (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))
          ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pRecordInfo->gome.orbitNumber+1;
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
          ((INT *)outputColumns[indexColumn++])[indexRecord]=pRecordInfo->scia.orbitNumber;
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
          ((INT *)outputColumns[indexColumn++])[indexRecord]=pRecordInfo->gome2.orbitNumber;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_CLOUD :
         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->cloudFraction;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_CLOUDTOPP :
         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->cloudTopPressure;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_COEFF :

         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=
          (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)?
          (DoasUS)pOrbitFile->gdpBinSpectrum.indexSpectralParam:(DoasUS)defaultValue;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_SCIA_STATE_INDEX :

         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=
          (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)?
          (DoasUS)pRecordInfo->scia.stateIndex:(DoasUS)defaultValue;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_SCIA_STATE_ID :

         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=
          (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)?
          (DoasUS)pRecordInfo->scia.stateId:(DoasUS)defaultValue;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_SCIA_QUALITY :

         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=
          (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)?
          (DoasUS)pRecordInfo->scia.qualityFlag:(DoasUS)defaultValue;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_SAT_HEIGHT :

         if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) && (pOrbitFile->gdpBinHeader.version<40))
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo3.satHeight;
         else if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) && (pOrbitFile->gdpBinHeader.version>=40))
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.satHeight;
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.satHeight;
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.satHeight;
         else
          indexColumn++;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_EARTH_RADIUS :

         if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) && (pOrbitFile->gdpBinHeader.version<40))
          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo3.radiusCurve;
         else if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) && (pOrbitFile->gdpBinHeader.version>=40))
          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.radiusCurve;
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.earthRadius;
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.earthRadius;
         else
          indexColumn++;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_VIEW_ELEVATION :
         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->elevationViewAngle;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_VIEW_ZENITH :
         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->zenithViewAngle;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_VIEW_AZIMUTH :
         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->azimuthViewAngle;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_LOS_ZA :

         if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) || (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS))
          {
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.losZen[0];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.losZen[1];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.losZen[2];
          }
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
          {
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.losZen[0];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.losZen[1];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.losZen[2];
          }
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
          {
           if (pOrbitFile->gdpBinHeader.version<40)
            {
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)(pOrbitFile->gdpBinSpectrum.geo.geo3.losZa[0]*0.01);k++;
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)(pOrbitFile->gdpBinSpectrum.geo.geo3.losZa[1]*0.01);k++;
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)(pOrbitFile->gdpBinSpectrum.geo.geo3.losZa[2]*0.01);
            }
           else if (pOrbitFile->gdpBinHeader.version>=40)
            {
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.losZaBOA[0];k++;
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.losZaBOA[1];k++;
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.losZaBOA[2];
            }
          }
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII)
          indexColumn+=3;
         else
          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->zenithViewAngle;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_LOS_AZIMUTH :

         if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
          {
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.losAzi[0];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.losAzi[1];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.losAzi[2];
          }
         else if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
                  (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS))
          {
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.losAzi[0];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.losAzi[1];k++;
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.losAzi[2];
          }
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
          {
           if (pOrbitFile->gdpBinHeader.version<40)
            {
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)(pOrbitFile->gdpBinSpectrum.geo.geo3.losAzim[0]*0.01);k++;
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)(pOrbitFile->gdpBinSpectrum.geo.geo3.losAzim[1]*0.01);k++;
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)(pOrbitFile->gdpBinSpectrum.geo.geo3.losAzim[2]*0.01);
            }
           else
            {
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.losAzimBOA[0];k++;
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.losAzimBOA[1];k++;
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo4.losAzimBOA[2];
            }
          }
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII)
          indexColumn+=3;
         else
          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->azimuthViewAngle;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_LONGIT :

         if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
             (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS))
          for (i=0;i<4;i++,k++)
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.longitudes[i];
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
          for (i=0;i<4;i++,k++)
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.longitudes[i];
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
          {
           if (pOrbitFile->gdpBinHeader.version<40)
            for (i=0;i<4;i++,k++)
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.lonArray[i];
           else
            for (i=0;i<4;i++,k++)
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)0.01*pOrbitFile->gdpBinSpectrum.geo.geo4.lonArray[i];
          }
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII)
          for (i=0;i<4;i++,k++)
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome.longit[i];

         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->longitude;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_LATIT :

         if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
             (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS))
          for (i=0;i<4;i++,k++)
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->scia.latitudes[i];
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
          for (i=0;i<4;i++,k++)
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome2.latitudes[i];
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
          {
           if (pOrbitFile->gdpBinHeader.version<40)
            for (i=0;i<4;i++,k++)
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.latArray[i];
           else
            for (i=0;i<4;i++,k++)
             ((float *)outputColumns[indexColumn++])[indexRecord]=(float)0.01*pOrbitFile->gdpBinSpectrum.geo.geo4.latArray[i];
          }
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII)
          for (i=0;i<4;i++,k++)
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->gome.latit[i];

         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->latitude;

        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_ALTIT :
         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->altitude;
        break;
     // ----------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_SCANNING :
         if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MKZY)
          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->mkzy.scanningAngle;
         else
          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->als.scanningAngle;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_COMPASS :
         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->als.compassAngle;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_PITCH :
         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->als.pitchAngle;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_ROLL :
         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->als.rollAngle;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_FILTERNUMBER :
         ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pRecordInfo->ccd.filterNumber;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_MEASTYPE :
         if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)
          ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pRecordInfo->ccd.measureType;
         else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_BIRA)
          ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pRecordInfo->mfcBira.measurementType;
         else
          indexColumn++;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_CCD_HEADTEMPERATURE :
         ((double *)outputColumns[indexColumn++])[indexRecord]=(double)pRecordInfo->ccd.headTemperature;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_COOLING_STATUS :
         ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pRecordInfo->coolingStatus;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_MIRROR_ERROR :
         ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pRecordInfo->mirrorError;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_GOME2_SCANDIRECTION :
         ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pRecordInfo->gome2.scanDirection;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_GOME2_SAA :
         ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pRecordInfo->gome2.saaFlag;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_GOME2_SUNGLINT_RISK :
         ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pRecordInfo->gome2.sunglintDangerFlag;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_GOME2_SUNGLINT_HIGHRISK :
         ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pRecordInfo->gome2.sunglintHighDangerFlag;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_GOME2_RAINBOW :
         ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pRecordInfo->gome2.rainbowFlag;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_CCD_DIODES :

         if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)
          for (i=0;i<4;i++,k++)
           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->ccd.diodes[i];

        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_CCD_TARGETAZIMUTH :
         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->ccd.targetAzimuth;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_CCD_TARGETELEVATION :
         ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pRecordInfo->ccd.targetElevation;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_SATURATED :
         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=(DoasUS)pRecordInfo->ccd.saturatedFlag;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_OMI_INDEX_SWATH :
         ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pRecordInfo->omi.omiSwathIndex;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_OMI_INDEX_ROW :
         ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pRecordInfo->omi.omiRowIndex;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_UAV_SERVO_BYTE_SENT :
         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=(DoasUS)pRecordInfo->uavBira.servoSentPosition;
        break;
     // ---------------------------------------------------------------------
        case PRJCT_RESULTS_ASCII_UAV_SERVO_BYTE_RECEIVED :
         ((DoasUS *)outputColumns[indexColumn++])[indexRecord]=(DoasUS)pRecordInfo->uavBira.servoReceivedPosition;
        break;
     // ----------------------------------------------------------------------
        default :
         k--;
        break;
     // ----------------------------------------------------------------------
       }
     }

    // ----------------
    // ANALYSIS RESULTS
    // ----------------

    for (indexWin=0;indexWin<nbWin;indexWin++)

     for (indexFeno=0;indexFeno<NFeno;indexFeno++)
      {
      	pTabFeno=(!hiddenFlag)?&TabFeno[indexFenoColumn][indexFeno]:&TabFeno[indexFenoColumn][KURUCZ_buffers[indexFenoColumn].indexKurucz];
      	TabCross=pTabFeno->TabCross;

      	if (pTabFeno->hidden==hiddenFlag)
      	 {
      	  if (!pTabFeno->hidden)
      	   {
      	   	if (OUTPUT_refZmFlag)
      	   	 ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pTabFeno->Zm;
      	   	if (OUTPUT_refShift)
      	   	 ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pTabFeno->Shift;

           if (OUTPUT_covarFlag && (pTabFeno->svd.covar!=NULL))
            for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
             if (TabCross[indexTabCross].IndSvdA>0)
              for (indexTabCross2=0;indexTabCross2<indexTabCross;indexTabCross2++)
               if (TabCross[indexTabCross2].IndSvdA>0)
                ((double *)outputColumns[indexColumn++])[indexRecord]=
                (!pTabFeno->rc && (TabCross[indexTabCross].Fact!=(double)0.))?
                 (double)pTabFeno->svd.covar[TabCross[indexTabCross2].IndSvdA][TabCross[indexTabCross].IndSvdA]*pTabFeno->chiSquare/
                 (TabCross[indexTabCross].Fact*TabCross[indexTabCross2].Fact):(double)defaultValue;

           if (OUTPUT_corrFlag && (pTabFeno->svd.covar!=NULL))
            for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
             if (TabCross[indexTabCross].IndSvdA>0)
              for (indexTabCross2=0;indexTabCross2<indexTabCross;indexTabCross2++)
               if (TabCross[indexTabCross2].IndSvdA>0)
                ((double *)outputColumns[indexColumn++])[indexRecord]=
                (!pTabFeno->rc && (TabCross[indexTabCross].Fact!=(double)0.))?
                 (double)pTabFeno->svd.covar[TabCross[indexTabCross2].IndSvdA][TabCross[indexTabCross].IndSvdA]*pTabFeno->chiSquare/
                 (TabCross[indexTabCross].Fact*TabCross[indexTabCross2].Fact*pTabFeno->TabCrossResults[indexTabCross].SlntErr*pTabFeno->TabCrossResults[indexTabCross2].SlntErr):
                 (double)defaultValue;
          }

         if (OUTPUT_chiSquareFlag)
          ((double *)outputColumns[indexColumn++])[indexRecord]=(!hiddenFlag)?(double)pTabFeno->chiSquare:KURUCZ_buffers[indexFenoColumn].KuruczFeno[indexFeno].chiSquare[indexWin];
         if (OUTPUT_rmsFlag)
          ((double *)outputColumns[indexColumn++])[indexRecord]=(!hiddenFlag)?(double)pTabFeno->RMS:KURUCZ_buffers[indexFenoColumn].KuruczFeno[indexFeno].rms[indexWin];
         if (OUTPUT_iterFlag)
          ((int *)outputColumns[indexColumn++])[indexRecord]=(!hiddenFlag)?(double)pTabFeno->nIter:KURUCZ_buffers[indexFenoColumn].KuruczFeno[indexFeno].nIter[indexWin];

         // Cross sections results

         for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
          {
           pTabCrossResults=(!hiddenFlag)?&pTabFeno->TabCrossResults[indexTabCross]:&KURUCZ_buffers[indexFenoColumn].KuruczFeno[indexFeno].results[indexWin][indexTabCross];

           if (pTabCrossResults->indexAmf!=ITEM_NONE)
            {
             if (pTabCrossResults->StoreAmf)              // AMF
               ((float *)outputColumns[indexColumn++])[indexRecord]=
               (!pTabFeno->rc)?(float)pTabCrossResults->Amf:(float)defaultValue;
             if (pTabCrossResults->StoreVrtCol)           // Vertical column
               ((double *)outputColumns[indexColumn++])[indexRecord]=
               (!pTabFeno->rc && (pTabCrossResults->VrtFact!=(double)0.))?
                (double)pTabCrossResults->VrtCol/pTabCrossResults->VrtFact:(double)defaultValue;
             if (pTabCrossResults->StoreVrtErr)           // Error on vertical column
               ((double *)outputColumns[indexColumn++])[indexRecord]=
               (!pTabFeno->rc && (pTabCrossResults->VrtFact!=(double)0.))?
                (double)pTabCrossResults->VrtErr/pTabCrossResults->VrtFact:(double)defaultValue;
            }

           if (pTabCrossResults->StoreSlntCol)            // Slant column
          	 	((double *)outputColumns[indexColumn++])[indexRecord]=
          	 	(!pTabFeno->rc && (pTabCrossResults->SlntFact!=(double)0.))?
          	 	 (double)pTabCrossResults->SlntCol/pTabCrossResults->SlntFact:(double)defaultValue;

           if (pTabCrossResults->StoreSlntErr)            // Error on slant column
            	((double *)outputColumns[indexColumn++])[indexRecord]=
            	(!pTabFeno->rc && (pTabCrossResults->SlntFact!=(double)0.))?
            	 (double)pTabCrossResults->SlntErr/pTabCrossResults->SlntFact:(double)defaultValue;

           if (pTabCrossResults->StoreShift)              // Shift
            {
             ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->Shift:(double)defaultValue;
             if (pTabCrossResults->StoreError)
              ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaShift:(double)defaultValue;
            }

           if (pTabCrossResults->StoreStretch)            // Stretch
            {
             ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->Stretch:(double)defaultValue;
             if (pTabCrossResults->StoreError)
              ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaStretch:(double)defaultValue;
             ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->Stretch2:(double)defaultValue;
             if (pTabCrossResults->StoreError)
              ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaStretch2:(double)defaultValue;
            }

           if (pTabCrossResults->StoreScale)              // Scale
            {
             ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->Scale:(double)defaultValue;
             if (pTabCrossResults->StoreError)
              ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaScale:(double)defaultValue;
             ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->Scale2:(double)defaultValue;
             if (pTabCrossResults->StoreError)
              ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaScale2:(double)defaultValue;
            }

           if (pTabCrossResults->StoreParam)              // Param
            {
             ((double *)outputColumns[indexColumn++])[indexRecord]=
              (double)((!pTabFeno->rc)?
                      ((TabCross[indexTabCross].IndSvdA)?pTabCrossResults->SlntCol:pTabCrossResults->Param):defaultValue);
             if (pTabCrossResults->StoreParamError)
             ((double *)outputColumns[indexColumn++])[indexRecord]=
              (double)((!pTabFeno->rc)?
                      ((TabCross[indexTabCross].IndSvdA)?pTabCrossResults->SlntErr:pTabCrossResults->SigmaParam):(double)defaultValue);
            }
          }
      	 }
      }

    // Color indexes and fluxes

    for (indexFluxes=0;indexFluxes<OUTPUT_NFluxes;indexFluxes++)
     ((double *)outputColumns[indexColumn++])[indexRecord]=(double)OutputFlux(pEngineContext,OUTPUT_fluxes[indexFluxes]);
    for (indexCic=0;indexCic<OUTPUT_NCic;indexCic++)
     ((double *)outputColumns[indexColumn++])[indexRecord]=((flux=OutputFlux(pEngineContext,OUTPUT_cic[indexCic][1]))!=(double)0.)?
      (double)OutputFlux(pEngineContext,OUTPUT_cic[indexCic][0])/flux:(double)defaultValue;

    outputRecords[indexRecord].nbColumns=indexColumn;
    outputRecords[indexRecord].year=(int)pRecordInfo->present_day.da_year;
    outputRecords[indexRecord].month=(int)pRecordInfo->present_day.da_mon;
    outputRecords[indexRecord].day=(int)pRecordInfo->present_day.da_day;
    outputRecords[indexRecord].longit=(float)pRecordInfo->longitude;
    outputRecords[indexRecord].latit=(float)pRecordInfo->latitude;
   }
 }

// ================
// OUTPUT FUNCTIONS
// ================

// -----------------------------------------------------------------------------
// FUNCTION      OutputBuildSiteFileName
// -----------------------------------------------------------------------------
// PURPOSE       Build the output file name using the selected observation site
//
// INPUT         pEngineContext   structure including information on project options
//               year,month  current date to process (monthly files are created)
//               indexSite   index of the observation site
//               ascFlag     0 to add BIN extension, 1 to add ASC extension
//
// OUTPUT        outputFileName, the name of the output file
// -----------------------------------------------------------------------------

void OutputBuildSiteFileName(ENGINE_CONTEXT *pEngineContext,DoasCh *outputFileName,INT year,INT month,INDEX indexSite,INT ascFlag)
 {
  // Declarations

  PROJECT             *pProject;                                                // pointer to project data
  PRJCT_RESULTS_ASCII *pResults;                                                // pointer to results part of project
  DoasCh               *fileNamePtr;                                             // character pointers used for building output file name

  // Initializations

  pProject=(PROJECT *)&pEngineContext->project;
  pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;

  // Build the complete output path

  FILES_RebuildFileName(outputFileName,pResults->path,1);

  if ((fileNamePtr=strrchr(outputFileName,PATH_SEP))==NULL)                     // extract output file name without path
   fileNamePtr=outputFileName;
  else
   fileNamePtr++;

  sprintf(fileNamePtr,"%s_%04d%02d.%s",
         (indexSite!=ITEM_NONE)?(DoasCh *)SITES_itemList[indexSite].abbrev:(DoasCh *)"XX",year,month,
	 (DoasCh *)((ascFlag)?"ASC":"BIN"));
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputBuildFileName
// -----------------------------------------------------------------------------
// PURPOSE       For satellites measurements, build automatically a file name
//               for output and create a directory structure
//
// INPUT         pEngineContext       structure including information on project options
// INPUT/OUTPUT  outputFileName  the original output file name to complete
//               ascFlag         0 to add BIN extension, 1 to add ASC extension
//
// RETURN        ERROR_ID_NOTHING_TO_SAVE if there is nothing to save,
//               ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC OutputBuildFileName(ENGINE_CONTEXT *pEngineContext,DoasCh *outputFileName,INT ascFlag)
 {
  // Declarations

  PROJECT             *pProject;                                                // pointer to project data
  PRJCT_RESULTS_ASCII *pResults;                                                // pointer to results part of project
  OUTPUT_INFO         *pOutput;
  DoasCh               *fileNamePtr,                                             // character pointers used for building output file name
                       tmpBuffer[MAX_ITEM_TEXT_LEN+1],
                      *ptr,*ptr2;
  int                  satelliteFlag;
  RC                   rc;

  // Initializations

  pProject=(PROJECT *)&pEngineContext->project;
  pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;

  rc=ERROR_ID_NO;

  if (!outputNbRecords)
   rc=ERROR_SetLast("OutputBuildFileName",ERROR_TYPE_WARNING,ERROR_ID_NOTHING_TO_SAVE,pEngineContext->fileInfo.fileName);
  else
   {
    pOutput=&outputRecords[0];

    // Build the complete output path

    strcpy(outputFileName,pResults->path);

 //   FILES_RebuildFileName(outputFileName,pResults->path,1);

    if ((fileNamePtr=strrchr(outputFileName,PATH_SEP))==NULL)                   // extract output file name without path
     fileNamePtr=outputFileName;
    else
     fileNamePtr++;

    satelliteFlag=((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
                   (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
                   (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
                   (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
                   (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI) ||
                   (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2));

    if ((!strlen(fileNamePtr) || !strcasecmp(fileNamePtr,"automatic")) &&
       ((satelliteFlag && ((pProject->spectra.mode!=PRJCT_SPECTRA_MODES_OBSLIST) || (pProject->spectra.radius<=1.))) ||
       (!satelliteFlag && (pResults->fileNameFlag || (SITES_GetIndex(pProject->instrumental.observationSite)==ITEM_NONE)))))
     {
      if ((ptr=strrchr(pEngineContext->fileInfo.fileName,PATH_SEP))==NULL)
       ptr=pEngineContext->fileInfo.fileName;
      else
       ptr++;

      // Remove the separator character in order that outputFileName is only the output directory

      fileNamePtr--;
      *fileNamePtr=0;

      if (satelliteFlag && pResults->dirFlag)
       {
        // Create 'year' directory

        strcpy(tmpBuffer,outputFileName);
        sprintf(outputFileName,"%s%c%d",tmpBuffer,PATH_SEP,(int)pOutput->year);
        #if defined WIN32
        mkdir(outputFileName);
        #else
        mkdir(outputFileName,0755);
        #endif

        // Create 'month' directory

        strcpy(tmpBuffer,outputFileName);
        sprintf(outputFileName,"%s%c%02d",tmpBuffer,PATH_SEP,(int)pOutput->month);
        #if defined WIN32
        mkdir(outputFileName);
        #else
        mkdir(outputFileName,0755);
        #endif

        // Create 'day' directory

        strcpy(tmpBuffer,outputFileName);
        sprintf(outputFileName,"%s%c%02d",tmpBuffer,PATH_SEP,(int)pOutput->day);
        #if defined WIN32
        mkdir(outputFileName);
        #else
        mkdir(outputFileName,0755);
        #endif
       }

      // Build output file name

       strcpy(tmpBuffer,outputFileName);

      if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
          (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS))

       sprintf(outputFileName,"%s%cSCIA_%d%02d%02d_%05d",tmpBuffer,PATH_SEP,pOutput->year,pOutput->month,pOutput->day,pEngineContext->recordInfo.scia.orbitNumber);

//      else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
//       sprintf(outputFileName,"%s%cGOME_xxx_1B_%d%02d%02d_%05d",tmpBuffer,PATH_SEP,pOutput->year,pOutput->month,pOutput->day,pEngineContext->recordInfo.gome2.orbitNumber);
      else
       sprintf(outputFileName,"%s%c%s",tmpBuffer,PATH_SEP,ptr);

      ptr2=strrchr(outputFileName,PATH_SEP);

      if ((pProject->instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_MFC) &&
          (pProject->instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_MFC_STD) &&
         ((ptr=strrchr(ptr2+1,'.'))!=NULL))
       strcpy(ptr,(ascFlag)?".ASC":".BIN");
      else
       strcat(outputFileName,(ascFlag)?".ASC":".BIN");
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputAscPrintTitles
// -----------------------------------------------------------------------------
// PURPOSE       Print titles of columns in the output ASCII file
//
// INPUT         pEngineContext       structure including information on project options
//               fp              pointer to the output file
// -----------------------------------------------------------------------------

void OutputAscPrintTitles(ENGINE_CONTEXT *pEngineContext,FILE *fp)
 {
  // Declarations

  INDEX indexField,indexLine,indexColumn,indexField2;
  PRJCT_RESULTS_FIELDS *pField,*pField2;
  INT nbColumns,nbLines;

  // Initializations

  indexColumn=0;

  for (indexField=0;
      (indexField<outputNbFields) && (outputFields[indexField].fieldDim1!=ITEM_NONE);)
   {
   	pField=&outputFields[indexField];

   	// Case 1 : the number of columns is known

   	if (outputFields[indexField].fieldDim2!=ITEM_NONE)
   	 {
   	  fprintf(fp,"%c %s\t\n",COMMENT_CHAR,pField->fieldName);
   	  for (indexLine=0;indexLine<pField->fieldDim1;indexLine++)
   	   {
   	    for (indexColumn=0;indexColumn<pField->fieldDim2;indexColumn++)
   	     {
   	     	fprintf(fp,"%c ",COMMENT_CHAR);

          switch(pField->fieldType)
           {
         // -------------------------------------------------------------------------
            case MEMORY_TYPE_STRING :
             fprintf(fp,pField->fieldFormat,&outputColumns[indexField][(indexColumn*pField->fieldDim1+indexLine)*pField->fieldSize]);
            break;
         // -------------------------------------------------------------------------
            case MEMORY_TYPE_USHORT :
             fprintf(fp,pField->fieldFormat,(DoasUS)((DoasUS *)outputColumns[indexField])[indexColumn*pField->fieldDim1+indexLine]);
            break;
         // -------------------------------------------------------------------------
            case MEMORY_TYPE_INT :
             fprintf(fp,pField->fieldFormat,(int)((int *)outputColumns[indexField])[indexColumn*pField->fieldDim1+indexLine]);
            break;
         // -------------------------------------------------------------------------
            case MEMORY_TYPE_FLOAT :
             fprintf(fp,pField->fieldFormat,(float)((float *)outputColumns[indexField])[indexColumn*pField->fieldDim1+indexLine]);
            break;
         // -------------------------------------------------------------------------
            case MEMORY_TYPE_DOUBLE :
             fprintf(fp,pField->fieldFormat,(double)((double *)outputColumns[indexField])[indexColumn*pField->fieldDim1+indexLine]);
            break;
         // -------------------------------------------------------------------------
           }

          fprintf(fp," ");
         }
   	    fprintf(fp,"\n");
   	   }

   	  indexField++;
   	 }

   	// Case 2 : the number of columns is unknown

   	else
   	 {
   	 	fprintf(fp,"%c ",COMMENT_CHAR);
   	 	nbLines=outputFields[indexField].fieldDim1;

   	 	// titles

   	 	for (nbColumns=0,indexField2=indexField;
   	 	    (indexField2<outputNbFields) && (outputFields[indexField2].fieldDim1==nbLines);indexField2++,nbColumns++)
   	 	 fprintf(fp,"%s\t",outputFields[indexField2].fieldName);

   	 	 fprintf(fp,"\n");

   	 	// data

   	 	for (indexLine=0;indexLine<pField->fieldDim1;indexLine++)
   	 	 {
   	    fprintf(fp,"%c ",COMMENT_CHAR);

   	    for (indexField2=indexField;indexField2<indexField+nbColumns;indexField2++)
   	     {
   	     	pField2=&outputFields[indexField2];

          switch(pField2->fieldType)
           {
         // -------------------------------------------------------------------------
            case MEMORY_TYPE_STRING :
             fprintf(fp,pField2->fieldFormat,&outputColumns[indexField2][indexLine]);
            break;
         // -------------------------------------------------------------------------
            case MEMORY_TYPE_USHORT :
             fprintf(fp,pField2->fieldFormat,(DoasUS)((DoasUS *)outputColumns[indexField2])[indexLine]);
            break;
         // -------------------------------------------------------------------------
            case MEMORY_TYPE_INT :
             fprintf(fp,pField2->fieldFormat,(int)((int *)outputColumns[indexField2])[indexLine]);
            break;
         // -------------------------------------------------------------------------
            case MEMORY_TYPE_FLOAT :
             fprintf(fp,pField2->fieldFormat,(float)((float *)outputColumns[indexField2])[indexLine]);
            break;
         // -------------------------------------------------------------------------
            case MEMORY_TYPE_DOUBLE :
             fprintf(fp,pField2->fieldFormat,(double)((double *)outputColumns[indexField2])[indexLine]);
            break;
         // -------------------------------------------------------------------------
           }

          fprintf(fp," ");
         }
   	    fprintf(fp,"\n");
   	   }

   	  indexField+=nbColumns;
   	 }
   }

  fprintf(fp,"# ");
  for (;indexField<outputNbFields;indexField++)
   fprintf(fp,"%s\t",outputFields[indexField].fieldName);
  fprintf(fp,"\n");
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputAscPrintDataSet
// -----------------------------------------------------------------------------
// PURPOSE       Flush the data set to the output file (ASC format)
//
// INPUT         fp              pointer to the output file;
//               outputData      all the data to save;
//               nbRecords       the number of records to save.
// -----------------------------------------------------------------------------

void OutputAscPrintDataSet(FILE *fp,DoasCh **outputData,INT nbRecords)
 {
  // Declarations

  PRJCT_RESULTS_FIELDS *pField;
  INDEX indexField,firstRecordField,indexRecord;

  for (firstRecordField=0;(firstRecordField<outputNbFields) && (outputFields[firstRecordField].fieldDim1!=ITEM_NONE);firstRecordField++);

  for (indexRecord=0;indexRecord<nbRecords;indexRecord++)
   {
    for (indexField=firstRecordField;indexField<outputNbFields;indexField++)
     {
      pField=&outputFields[indexField];

      switch(pField->fieldType)
       {
     // -------------------------------------------------------------------------
        case MEMORY_TYPE_STRING :
         fprintf(fp,pField->fieldFormat,&outputData[indexField][indexRecord*pField->fieldSize]);
        break;
     // -------------------------------------------------------------------------
        case MEMORY_TYPE_USHORT :
         fprintf(fp,pField->fieldFormat,(DoasUS)((DoasUS *)outputData[indexField])[indexRecord]);
        break;
     // -------------------------------------------------------------------------
        case MEMORY_TYPE_INT :
         fprintf(fp,pField->fieldFormat,(int)((int *)outputData[indexField])[indexRecord]);
        break;
     // -------------------------------------------------------------------------
        case MEMORY_TYPE_FLOAT :
         fprintf(fp,pField->fieldFormat,(float)((float *)outputData[indexField])[indexRecord]);
        break;
     // -------------------------------------------------------------------------
        case MEMORY_TYPE_DOUBLE :
         fprintf(fp,pField->fieldFormat,(double)((double *)outputData[indexField])[indexRecord]);
        break;
     // -------------------------------------------------------------------------
       }

      fprintf(fp,"\t");
     }

    fprintf(fp,"\n");
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputBinWriteFields
// -----------------------------------------------------------------------------
// PURPOSE       Output information on fields in the binary output file
//
// INPUT         fp              pointer to the output file;
//               outputData      all the data to save;
// -----------------------------------------------------------------------------

void OutputBinWriteFields(FILE *fp,DoasCh **outputData)
 {
 	// Declarations

 	INDEX indexField;
 	PRJCT_RESULTS_FIELDS *pField;

 	fwrite(&outputNbDataSet,sizeof(int),1,fp);
 	fwrite(&outputNbFields,sizeof(int),1,fp);
  fwrite(outputFields,sizeof(PRJCT_RESULTS_FIELDS),outputNbFields,fp);

  for (indexField=0;indexField<outputNbFields;indexField++)
   {
   	pField=&outputFields[indexField];
    fwrite(outputData[indexField],pField->fieldDim1*((pField->fieldDim2==ITEM_NONE)?1:pField->fieldDim2)*pField->fieldSize,1,fp);
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputBinVerifyFields
// -----------------------------------------------------------------------------
// PURPOSE       Verify the consistency of the outputFile with the current data set to save
//
// INPUT         outputFileName  the name of the output file
//               fp              pointer to the output file;
//
// RETURN        ERROR_ID_ALLOC if the allocation of a vector failed
//               ERROR_ID_OUTPUT if file and current data set are unconsistent
//               ERROR_ID_NO if both are consisten
// -----------------------------------------------------------------------------

RC OutputBinVerifyFields(DoasCh *outputFileName,FILE *fp)
 {
 	// Declarations

  PRJCT_RESULTS_FIELDS *fields;
 	int nbFields;
  RC rc;

 	// Initialization

 	fields=NULL;
 	rc=ERROR_ID_NO;

  fread(&outputNbDataSet,sizeof(int),1,fp);
 	fread(&nbFields,sizeof(int),1,fp);

 	if (nbFields!=outputNbFields)
 	 rc=ERROR_SetLast("OutputBinVerifyFields",ERROR_TYPE_FATAL,ERROR_ID_OUTPUT,outputFileName);
 	else if ((fields=(PRJCT_RESULTS_FIELDS *)MEMORY_AllocBuffer("OutputBinVerifyFields","fields",nbFields,sizeof(PRJCT_RESULTS_FIELDS),0,MEMORY_TYPE_STRUCT))==NULL)
 	 rc=ERROR_ID_ALLOC;
 	else
 	 {
 	 	fread(fields,sizeof(PRJCT_RESULTS_FIELDS)*nbFields,1,fp);
 	 	if (memcmp(fields,outputFields,sizeof(PRJCT_RESULTS_FIELDS)*nbFields))
 	 	 rc=ERROR_SetLast("OutputBinVerifyFields",ERROR_TYPE_FATAL,ERROR_ID_OUTPUT,outputFileName);
 	 }

 	// Release the allocated buffer

  if (fields!=NULL)
   MEMORY_ReleaseBuffer("OutputBinVerifyFields","fields",fields);

 	// Return

 	return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputBinWriteDataSet
// -----------------------------------------------------------------------------
// PURPOSE       Flush the data set to the output file (binary format)
//
// INPUT         fp              pointer to the output file;
//               outputData      all the data to save;
//               nbRecords       the number of records to save.
// -----------------------------------------------------------------------------

RC OutputBinWriteDataSet(FILE *fp,DoasCh **outputData,INT nbRecords)
 {
  // Declarations

  double *scalingFactors,*dataMax,*dataMin,defaultValue;
  short *data;
  PRJCT_RESULTS_FIELDS *pField;
  INDEX indexField,indexRecord,firstRecordField;
  RC rc;

  // Initializations

  indexRecord=0;
  dataMax=dataMin=NULL;
  defaultValue=(double)9999.;
  rc=ERROR_ID_NO;
  data=NULL;

  if (((scalingFactors=(double *)MEMORY_AllocDVector("OutputBinWriteDataSet","scalingFactors",0,outputNbFields-1))==NULL) ||
      ((dataMax=(double *)MEMORY_AllocDVector("OutputBinWriteDataSet","dataMax",0,outputNbFields-1))==NULL) ||
      ((dataMin=(double *)MEMORY_AllocDVector("OutputBinWriteDataSet","dataMin",0,outputNbFields-1))==NULL) ||
      ((data=(short *)MEMORY_AllocBuffer("OutputBinWriteDataSet","data",nbRecords,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL))
   rc=ERROR_ID_ALLOC;
  else
   {
    // Initializations

    for (firstRecordField=0;(firstRecordField<outputNbFields) && (outputFields[firstRecordField].fieldDim1!=ITEM_NONE);firstRecordField++)
     {
      scalingFactors[firstRecordField]=(double)-1.;
      dataMax[firstRecordField]=dataMin[firstRecordField]=(double)0.;
     }

    for (indexField=firstRecordField;indexField<outputNbFields;indexField++)
     {
     	pField=&outputFields[indexField];
     	scalingFactors[indexField]=(double)-1.;
     	dataMax[indexField]=dataMin[indexField]=(double)0.;

     	// Search for the first valid value

     	if ((pField->fieldType==MEMORY_TYPE_INT) || (pField->fieldType==MEMORY_TYPE_FLOAT) || (pField->fieldType==MEMORY_TYPE_DOUBLE))
     	 {
       	for (indexRecord=0;(indexRecord<nbRecords) &&
       	  (((pField->fieldType==MEMORY_TYPE_INT) && (fabs((double)((int *)outputData[indexField])[indexRecord]-defaultValue)<(double)1.e-6)) ||
       	   ((pField->fieldType==MEMORY_TYPE_FLOAT) && (fabs((double)((float *)outputData[indexField])[indexRecord]-defaultValue)<(double)1.e-6)) ||
       	   ((pField->fieldType==MEMORY_TYPE_DOUBLE) && (fabs((double)((double *)outputData[indexField])[indexRecord]-defaultValue)<(double)1.e-6)));
       	     indexRecord++);

       	if (indexRecord<nbRecords)
       	 switch(pField->fieldType)
       	  {
       	// ---------------------------------------------------------------------
       	   case MEMORY_TYPE_INT :
       	    dataMax[indexField]=dataMin[indexField]=(double)((int *)outputData[indexField])[indexRecord++];
       	   break;
       	// ---------------------------------------------------------------------
       	   case MEMORY_TYPE_FLOAT :
       	    dataMax[indexField]=dataMin[indexField]=(double)((float *)outputData[indexField])[indexRecord++];
       	   break;
       	// ---------------------------------------------------------------------
       	   case MEMORY_TYPE_DOUBLE :
       	    dataMax[indexField]=dataMin[indexField]=(double)((double *)outputData[indexField])[indexRecord++];
       	   break;
       	// ---------------------------------------------------------------------
       	  }
     	 }

      if (pField->fieldType==MEMORY_TYPE_INT)
       {
       	for (;indexRecord<nbRecords;indexRecord++)
         if (fabs((double)(((int *)outputData[indexField])[indexRecord])-defaultValue)>(double)1.e-6)
        	 {
        	 	if (((double)((int *)outputData[indexField])[indexRecord])>dataMax[indexField])
        	 	 dataMax[indexField]=(double)((int *)outputData[indexField])[indexRecord];
        	 	if (((double)((int *)outputData[indexField])[indexRecord])<dataMin[indexField])
        	 	 dataMin[indexField]=(double)((int *)outputData[indexField])[indexRecord];
        	 }
       }
      else if (pField->fieldType==MEMORY_TYPE_FLOAT)
       {
       	for (;indexRecord<nbRecords;indexRecord++)
       	 if (fabs((double)(((float *)outputData[indexField])[indexRecord])-defaultValue)>(double)1.e-6)
        	 {
        	 	if (((double)((float *)outputData[indexField])[indexRecord])>dataMax[indexField])
        	 	 dataMax[indexField]=(double)((float *)outputData[indexField])[indexRecord];
        	 	if (((double)((float *)outputData[indexField])[indexRecord])<dataMin[indexField])
        	 	 dataMin[indexField]=(double)((float *)outputData[indexField])[indexRecord];
        	 }
       }
      else if (pField->fieldType==MEMORY_TYPE_DOUBLE)
       {
       	for (;indexRecord<nbRecords;indexRecord++)
       	 if (fabs((double)(((double *)outputData[indexField])[indexRecord])-defaultValue)>(double)1.e-6)
       	  {
       	  	if (((double)((double *)outputData[indexField])[indexRecord])>dataMax[indexField])
       	  	 dataMax[indexField]=(double)((double *)outputData[indexField])[indexRecord];
       	  	if (((double)((double *)outputData[indexField])[indexRecord])<dataMin[indexField])
       	  	 dataMin[indexField]=(double)((double *)outputData[indexField])[indexRecord];
       	  }
       }
     }

    // Calculate the scaling factors and output them

    for (indexField=firstRecordField;indexField<outputNbFields;indexField++)
     {
     	pField=&outputFields[indexField];

     	if ((pField->fieldType==MEMORY_TYPE_INT) ||
     	    (pField->fieldType==MEMORY_TYPE_FLOAT) ||
     	    (pField->fieldType==MEMORY_TYPE_DOUBLE))
     	 {
        if (dataMin[indexField]<dataMax[indexField])
         scalingFactors[indexField]=(double)64000./(dataMax[indexField]-dataMin[indexField]);
        else
         scalingFactors[indexField]=(double)1.;
       }
     }

    fseek(fp,0L,SEEK_END);
    fwrite(&nbRecords,sizeof(int),1,fp);
    fwrite(scalingFactors,sizeof(double)*outputNbFields,1,fp);
    fwrite(dataMin,sizeof(double)*outputNbFields,1,fp);

    // Adjust the maxima

    for (indexField=firstRecordField;indexField<outputNbFields;indexField++)
     {
      pField=&outputFields[indexField];

      if (scalingFactors[indexField]>=(double)0.)
       {
        switch(pField->fieldType)
         {
       // -------------------------------------------------------------------------
          case MEMORY_TYPE_INT :
           for (indexRecord=0;indexRecord<nbRecords;indexRecord++)
            data[indexRecord]=
             (fabs((double)(((int *)outputData[indexField])[indexRecord])-defaultValue)>(double)1.e-6)?
             (short)(((double)(((int *)outputData[indexField])[indexRecord])-dataMin[indexField])*scalingFactors[indexField]-32000.):(short)defaultValue;
          break;
       // -------------------------------------------------------------------------
          case MEMORY_TYPE_FLOAT :
           for (indexRecord=0;indexRecord<nbRecords;indexRecord++)
            data[indexRecord]=
             (fabs((double)(((float *)outputData[indexField])[indexRecord])-defaultValue)>(double)1.e-6)?
             (short)(((double)(((float *)outputData[indexField])[indexRecord])-dataMin[indexField])*scalingFactors[indexField]-32000.):(short)defaultValue;
          break;
       // -------------------------------------------------------------------------
          case MEMORY_TYPE_DOUBLE :
           for (indexRecord=0;indexRecord<nbRecords;indexRecord++)
            data[indexRecord]=
             (fabs((double)(((double *)outputData[indexField])[indexRecord])-defaultValue)>(double)1.e-6)?
             (short)((((double)((double *)outputData[indexField])[indexRecord])-dataMin[indexField])*scalingFactors[indexField]-32000.):(short)defaultValue;
          break;
       // -------------------------------------------------------------------------
         }

        fwrite(data,sizeof(short)*nbRecords,1,fp);
       }
      else
       fwrite(&outputData[indexField][0],pField->fieldSize*nbRecords,1,fp);
     }

    // Update the number of data set

    outputNbDataSet++;
    fseek(fp,0L,SEEK_SET);
    fwrite(&outputNbDataSet,sizeof(int),1,fp);
   }

  // Release the allocated buffers

  if (scalingFactors!=NULL)
   MEMORY_ReleaseDVector("OutputBinWriteDataSet","scalingFactors",scalingFactors,0);
  if (dataMax!=NULL)
   MEMORY_ReleaseDVector("OutputBinWriteDataSet","dataMax",dataMax,0);
  if (dataMin!=NULL)
   MEMORY_ReleaseDVector("OutputBinWriteDataSet","dataMin",dataMin,0);
  if (data!=NULL)
   MEMORY_ReleaseBuffer("OutputBinWriteDataSet","data",data);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      OutputFileOpen
// -----------------------------------------------------------------------------
// PURPOSE       Open the outputFile and save the preliminary information
//
// INPUT         pEngineContext       structure including information on project options
//               outputFileName  the name of the outputFile
//               ascFlag         0 to output to a binary file,
//                               1 to output data to an ASCII file
//
// RETURN        pointer to the output file
// -----------------------------------------------------------------------------

FILE *OutputFileOpen(ENGINE_CONTEXT *pEngineContext,DoasCh *outputFileName,INT ascFlag)
 {
  // Declarations

  FILE *fp;
  DoasCh r[4],w[4],a[4];
  INT newFile;
  INT corrupted;

  PROJECT             *pProject;                                                // pointer to project data
  PRJCT_RESULTS_ASCII *pResults;                                                // pointer to results part of project

  // Initializations

  pProject=(PROJECT *)&pEngineContext->project;
  pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;

  // Initializations

  strcpy(r,(ascFlag)?"rt":"rb");                                                // open the file in read mode
  strcpy(w,(ascFlag)?"w+t":"w+b");                                              // open the file in write mode
  strcpy(a,(ascFlag)?"a+t":"r+b");                                              // open the file in append mode

  outputNbDataSet=0;

  newFile=((fp=fopen(outputFileName,r))==NULL)?1:0;
  corrupted=(!newFile && !ascFlag && OutputBinVerifyFields(outputFileName,fp))?1:0;

  // Close the file

  if (fp!=NULL)
   fclose(fp);

  // Open the file in append mode

  if (corrupted)
   fp=NULL;
  else if (((fp=fopen(outputFileName,(newFile)?w:a))!=NULL) && newFile)
   {
    if (THRD_id==THREAD_TYPE_ANALYSIS)
     {
      // Satellites measurements and automatic reference selection : save information on the selected reference

      if (((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&
            pEngineContext->analysisRef.refAuto)                                           // automatic reference is requested for at least one analysis window
       {
       	strcpy(outputColumns[0],OUTPUT_refFile);
       	((int *)outputColumns[1])[0]=OUTPUT_nRec;
       }
     }

   	// Save information on the calibration

   	if (pResults->calibFlag)
     OutputCalib(pEngineContext);

    if (ascFlag)
     OutputAscPrintTitles(pEngineContext,fp);
    else
     OutputBinWriteFields(fp,outputColumns);
   }

  // Return

  return fp;
 }

// -----------------------------------------------------------------------------
// FUNCTION      OUTPUT_FlushBuffers
// -----------------------------------------------------------------------------
// PURPOSE       Flusth the buffers in a one shot
//
// INPUT         pEngineContext       structure including information on project options
//
// RETURN        Non zero value return code if the output failed
// -----------------------------------------------------------------------------

RC OUTPUT_FlushBuffers(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  DoasCh outputFileName[MAX_ITEM_TEXT_LEN+1],
        outputAutomaticFileName[MAX_ITEM_TEXT_LEN+1];
  FILE *outputFp;
  DoasCh **outputData;

  OBSERVATION_SITE *pSite;
  PROJECT             *pProject;                     // pointer to project data
  PRJCT_RESULTS_FIELDS *pField;
  PRJCT_RESULTS_ASCII *pResults;                     // pointer to results part of project
  DoasCh               *ptr;
  INDEX indexSite,indexField,firstRecordField,indexRecord,oldYear,oldMonth,oldRecord;
  INT fieldDim2;
  INT automatic,nbRecords;
  RC rc;

  // Initializations

  pProject=(PROJECT *)&pEngineContext->project;
  pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;

  memset(outputFileName,0,MAX_ITEM_TEXT_LEN+1);
  memset(outputAutomaticFileName,0,MAX_ITEM_TEXT_LEN+1);

  outputFp=NULL;
  outputData=NULL;
  firstRecordField=0;

  rc=ERROR_ID_NO;

  if ((pResults->analysisFlag || pResults->calibFlag) && outputNbRecords && !(rc=OutputBuildFileName(pEngineContext,outputFileName,!pResults->binaryFlag)))
   {
    if ((ptr=strrchr(outputFileName,PATH_SEP))==NULL)
     ptr=outputFileName;
    else
     ptr++;

    if ((strlen(ptr)==9) && !strcasecmp(ptr,"automatic"))
     *ptr='\0';

    automatic=!strlen(ptr);

    // ------------------
    // NOT AUTOMATIC MODE
    // ------------------

    if (!automatic)
     {
      if ((outputFp=OutputFileOpen(pEngineContext,outputFileName,!pResults->binaryFlag))==NULL)
       rc=ERROR_SetLast("OUTPUT_FlushBuffers",ERROR_TYPE_FATAL,ERROR_ID_FILE_OPEN,"outputFileName");
      else if (pResults->binaryFlag)
       rc=OutputBinWriteDataSet(outputFp,outputColumns,outputNbRecords);
      else
       OutputAscPrintDataSet(outputFp,outputColumns,outputNbRecords);
     }
    else if ((outputData=(DoasCh **)MEMORY_AllocBuffer("OUTPUT_FlushBuffers","outputData",MAX_FIELDS,sizeof(DoasCh *),0,MEMORY_TYPE_PTR))==NULL)
     rc=ERROR_SetLast("OUTPUT_FlushBuffers",ERROR_TYPE_FATAL,ERROR_ID_ALLOC,"outputData");
    else
     {
    // -------------------
    // FULL AUTOMATIC MODE
    // -------------------

     	memset(outputData,0,sizeof(DoasCh *)*MAX_FIELDS);

     	for (firstRecordField=0;(firstRecordField<outputNbFields) && (outputFields[firstRecordField].fieldDim1!=ITEM_NONE) && !rc;firstRecordField++)
     	 {
     	 	pField=&outputFields[firstRecordField];
     	 	fieldDim2=(pField->fieldDim2!=ITEM_NONE)?pField->fieldDim2:1;

     	  if ((outputData[firstRecordField]=(DoasCh *)MEMORY_AllocBuffer("OUTPUT_FlushBuffers",pField->fieldName,
     	                                             pField->fieldDim1*fieldDim2,pField->fieldSize,0,pField->fieldType))==NULL)
     	   rc=ERROR_SetLast("OUTPUT_FlushBuffers",ERROR_TYPE_FATAL,ERROR_ID_ALLOC,"outputData[firstRecordField]");
     	 }

     	for (indexField=firstRecordField;(indexField<outputNbFields) && !rc;indexField++)
     	 {
     	 	pField=&outputFields[indexField];
     	  if ((outputData[indexField]=(DoasCh *)MEMORY_AllocBuffer("OUTPUT_FlushBuffers",pField->fieldName,outputNbRecords,pField->fieldSize,0,pField->fieldType))==NULL)
     	   rc=ERROR_SetLast("OUTPUT_FlushBuffers",ERROR_TYPE_FATAL,ERROR_ID_ALLOC,"outputData[indexField]");
     	 }

     	if (!rc)
     	 {
     	 	// Overpasses : records are distributed using information on the measurement date and the geolocation

     	 	if ((pProject->spectra.mode==PRJCT_SPECTRA_MODES_OBSLIST) && (pProject->spectra.radius>1.))

         for (indexSite=0;indexSite<SITES_itemN;indexSite++)
          {
     	 	 	 indexRecord=0;
     	 	 	 pSite=&SITES_itemList[indexSite];

     	 	 	 while ((indexRecord<outputNbRecords) && !rc)
     	 	 	  {
       	 	 	 oldYear=outputRecords[indexRecord].year;
       	 	 	 oldMonth=outputRecords[indexRecord].month;

     	 	 	  	for (indexField=0;indexField<firstRecordField;indexField++)
     	 	 	  	 {
     	 	 	  	 	pField=&outputFields[indexField];                                // this line was not existing in WinDOAS --- BUG ???
               fieldDim2=(pField->fieldDim2!=ITEM_NONE)?pField->fieldDim2:1;
     	 	 	  	  memcpy(outputData[indexField],outputColumns[indexField],outputFields[indexField].fieldDim1*fieldDim2*outputFields[indexField].fieldSize);
     	 	 	  	 }

     	 	 	  	for (nbRecords=0;(indexRecord<outputNbRecords) && ((outputRecords[indexRecord].year==oldYear) || (outputRecords[indexRecord].month==oldMonth));indexRecord++)
     	 	 	  	 if (THRD_GetDist((double)outputRecords[indexRecord].longit,(double)outputRecords[indexRecord].latit,
     	 	 	  	                  (double)pSite->longitude,(double)pSite->latitude)<=(double)pProject->spectra.radius)
     	 	 	  	  {
       	 	  	 	 for (indexField=firstRecordField;(indexField<outputNbFields);indexField++)
       	 	  	 	  {
       	 	  	 	  	pField=&outputFields[indexField];
    	   	  	 	    memcpy(&outputData[indexField][nbRecords*pField->fieldSize],&outputColumns[indexField][indexRecord*pField->fieldSize],pField->fieldSize);
     	 	    	 	  }
     	 	    	 	 nbRecords++;
     	 	    	 	}

     	 	 	  	if (nbRecords)
     	 	 	  	 {
     	 	 	  	 	OutputBuildSiteFileName(pEngineContext,outputAutomaticFileName,oldYear,oldMonth,indexSite,!pResults->binaryFlag);

               if ((outputFp=OutputFileOpen(pEngineContext,outputAutomaticFileName,!pResults->binaryFlag))==NULL)
                ERROR_SetLast("OutputFlushBuffers",ERROR_TYPE_FATAL,ERROR_ID_FILE_OPEN,"outputFileName");
               else if (pResults->binaryFlag)
                rc=OutputBinWriteDataSet(outputFp,outputData,nbRecords);
               else
                OutputAscPrintDataSet(outputFp,outputData,nbRecords);

               if (outputFp!=NULL)
                {
                	fclose(outputFp);
                	outputFp=NULL;
                }
     	 	 	  	 }
     	 	 	  }
          }

     	 	// Records are saved using the information on the date only

     	 	else
     	 	 {
     	 	 	nbRecords=indexRecord=0;
     	 	 	oldRecord=0;
     	 	 	indexSite=SITES_GetIndex(pProject->instrumental.observationSite);

     	 	 	while (indexRecord<outputNbRecords)
     	 	 	 {
       	 	 	oldYear=outputRecords[indexRecord].year;
       	 	 	oldMonth=outputRecords[indexRecord].month;

    	 	 	  	for (indexField=0;indexField<firstRecordField;indexField++)
    	 	 	  	 {
    	 	 	  	 	pField=&outputFields[indexField];                                 // this line was not existing in WinDOAS --- BUG ???
    	 	 	  	 	fieldDim2=(pField->fieldDim2!=ITEM_NONE)?pField->fieldDim2:1;
    	 	 	  	  memcpy(outputData[indexField],outputColumns[indexField],outputFields[indexField].fieldDim1*fieldDim2*outputFields[indexField].fieldSize);
    	 	 	  	 }

     	 	 	 	for (;(indexRecord<outputNbRecords) && ((outputRecords[indexRecord].year==oldYear) || (outputRecords[indexRecord].month==oldMonth));indexRecord++);

     	 	 	 	if ((nbRecords=indexRecord-oldRecord)>0)
     	 	 	 	 {
       	 	 	 	for (indexField=firstRecordField;(indexField<outputNbFields) && !rc;indexField++)
       	 	 	 	 {
       	 	 	 	 	pField=&outputFields[indexField];
    	   	 	 	   memcpy(&outputData[indexField][0],&outputColumns[indexField][oldRecord*pField->fieldSize],nbRecords*pField->fieldSize);
     	 	   	 	 }

     	 	 	 	 	OutputBuildSiteFileName(pEngineContext,outputAutomaticFileName,oldYear,oldMonth,indexSite,!pResults->binaryFlag);

              if ((outputFp=OutputFileOpen(pEngineContext,outputAutomaticFileName,!pResults->binaryFlag))==NULL)
               ERROR_SetLast("OutputFlushBuffers",ERROR_TYPE_FATAL,ERROR_ID_FILE_OPEN,"outputFileName");
              else if (pResults->binaryFlag)
               rc=OutputBinWriteDataSet(outputFp,outputData,nbRecords);
              else
               OutputAscPrintDataSet(outputFp,outputData,nbRecords);

              if (outputFp!=NULL)
               {
               	fclose(outputFp);
               	outputFp=NULL;
               }
     	 	 	 	 }

     	 	 	 	oldRecord=indexRecord;
     	 	 	 }
     	 	 }
     	 }
     }

    // Close file

    if (outputFp!=NULL)
     fclose(outputFp);
   }

//  {
//  	FILE *fp;
//  	fp=fopen("qdoas.dbg","a+t");
//  	fprintf(fp,"   OUTPUT_FlushBuffers (%d records,rc %d)\n",outputNbRecords,rc);
//  	fclose(fp);
//  }

  outputNbRecords=0;
  pEngineContext->lastSavedRecord=0;

  // Release the allocated buffers

  if (outputData!=NULL)
   {
    for (indexField=0;indexField<outputNbFields;indexField++)
     if (outputData[indexField]!=NULL)
      MEMORY_ReleaseBuffer("OUTPUT_FlushBuffers",outputFields[indexField].fieldName,outputData[indexField]);

    MEMORY_ReleaseBuffer("OUTPUT_FlushBuffers","outputData",outputData);
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      OUTPUT_SaveResults
// -----------------------------------------------------------------------------
// PURPOSE       save all results (ASC/BIN)
//
// INPUT         pEngineContext   structure including information on the current record
//
// RETURN        Non zero value return code if the function failed
//               ERROR_ID_NO on success
// -----------------------------------------------------------------------------

RC OUTPUT_SaveResults(ENGINE_CONTEXT *pEngineContext,INDEX indexFenoColumn)
 {
  // Declarations

  CROSS_RESULTS *pTabCrossResults;
  RECORD_INFO *pRecordInfo;
  INDEX indexFeno,indexTabCross,i;
  double *Spectrum;
  FENO *pTabFeno;
  RC rc;

  // Initializations

  pRecordInfo=&pEngineContext->recordInfo;
  rc=ERROR_ID_NO;

  // AMF computation

  if (OUTPUT_AmfSpace!=NULL)

   for (indexFeno=0;indexFeno<NFeno;indexFeno++)
    {
     pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

     if ((THRD_id!=THREAD_TYPE_KURUCZ) && !pTabFeno->hidden && !pTabFeno->rcKurucz)
      {
       for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
        {
         pTabCrossResults=&pTabFeno->TabCrossResults[indexTabCross];

         if (pTabCrossResults->indexAmf!=ITEM_NONE)
          {
           if (OutputGetAmf(pTabCrossResults,pRecordInfo->Zm,pRecordInfo->Tm,&pTabCrossResults->Amf))
            rc=ERROR_SetLast("OutputSaveResults",ERROR_TYPE_WARNING,ERROR_ID_AMF,pRecordInfo->Zm,OUTPUT_AmfSpace[pTabCrossResults->indexAmf].amfFileName);
           else if (pTabCrossResults->Amf!=(double)0.)
            {
             pTabCrossResults->VrtCol=(pTabCrossResults->SlntCol+pTabCrossResults->ResCol)/pTabCrossResults->Amf;
             pTabCrossResults->VrtErr=pTabCrossResults->SlntErr/pTabCrossResults->Amf;
            }
          }
        }
      }
    }

  // Rebuild spectrum for fluxes and color indexes computation

  if ((pRecordInfo->NSomme!=0) && (pRecordInfo->TotalExpTime!=(double)0.))
   {
    Spectrum=(double *)pEngineContext->buffers.spectrum;

    for (i=0;i<NDET;i++)
     Spectrum[i]*=(double)pRecordInfo->NSomme/pRecordInfo->TotalExpTime;
   }

  if (outputNbRecords<pEngineContext->recordNumber)
   OutputSaveRecord(pEngineContext,(THRD_id==THREAD_TYPE_ANALYSIS)?0:1,indexFenoColumn);

  // Results safe keeping

  pEngineContext->lastSavedRecord=pEngineContext->indexRecord;

  // Return

  return rc;
 }

// ====================
// RESOURCES MANAGEMENT
// ====================

// -----------------------------------------------------------------------------
// FUNCTION        OUTPUT_LocalAlloc
// -----------------------------------------------------------------------------
// PURPOSE         Allocate and initialize buffers for the records to output
//
// INPUT           pEngineContext   structure including information on the current project
//
// RETURN          ERROR_ID_ALLOC if one of the buffer allocation failed
//                 ERROR_ID_NO in case of success
// -----------------------------------------------------------------------------

RC OUTPUT_LocalAlloc(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  PROJECT             *pProject;                     // pointer to project data
  PRJCT_RESULTS_ASCII *pResults;                     // pointer to results part of project
  PRJCT_RESULTS_FIELDS *pField;
  INT newRecordNumber,n;
  INDEX indexField,i;
  RC rc;                                                                        // return code

  // Initializations

  rc=ERROR_ID_NO;

  newRecordNumber=pEngineContext->recordNumber;

  pProject=(PROJECT *)&pEngineContext->project;
  pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;

  if (pResults->analysisFlag || pResults->calibFlag)
   {
    if (!newRecordNumber || (outputMaxRecords<newRecordNumber))
     {
      if (outputRecords!=NULL)
       MEMORY_ReleaseBuffer("OUTPUT_LocalAlloc","outputRecords",outputRecords);

      for (indexField=0;indexField<outputNbFields;indexField++)
       if (outputColumns[indexField]!=NULL)
        MEMORY_ReleaseBuffer("OUTPUT_LocalAlloc",outputFields[indexField].fieldName,outputColumns[indexField]);

      memset(outputColumns,0,sizeof(DoasCh *)*MAX_FIELDS);
      outputRecords=NULL;
      outputMaxRecords=0;
     }

    // Allocate new buffers

    if (newRecordNumber>0)
     {
      if ((outputMaxRecords<newRecordNumber) && ((outputRecords=(OUTPUT_INFO *)MEMORY_AllocBuffer("OUTPUT_LocalAlloc","outputRecords",newRecordNumber,sizeof(OUTPUT_INFO),0,MEMORY_TYPE_STRUCT))==NULL))
       rc=ERROR_ID_ALLOC;
      else
       {
        memset(outputRecords,0,sizeof(OUTPUT_INFO)*newRecordNumber);

        for (indexField=0;(indexField<outputNbFields) && !rc;indexField++)
         {
          pField=&outputFields[indexField];

          if (pField->fieldDim1==ITEM_NONE)
           n=newRecordNumber;
          else if (pField->fieldDim2==ITEM_NONE)
           n=pField->fieldDim1;
          else
           n=pField->fieldDim1*pField->fieldDim2;

          if ((outputMaxRecords<newRecordNumber) &&
             ((outputColumns[indexField]=(DoasCh *)MEMORY_AllocBuffer("OUTPUT_LocalAlloc",outputFields[indexField].fieldName,n,pField->fieldSize,0,pField->fieldType))==NULL))

           rc=ERROR_ID_ALLOC;

          else

           switch(pField->fieldType)
            {
          // -------------------------------------------------------------------------
             case MEMORY_TYPE_STRING :
              memset(outputColumns[indexField],0,n*pField->fieldSize);
             break;
          // -------------------------------------------------------------------------
             case MEMORY_TYPE_USHORT :
              for (i=0;i<n;i++)
               ((DoasUS *)outputColumns[indexField])[i]=9999;
             break;
          // -------------------------------------------------------------------------
             case MEMORY_TYPE_INT :
              for (i=0;i<n;i++)
               ((int *)outputColumns[indexField])[i]=9999;
             break;
          // -------------------------------------------------------------------------
             case MEMORY_TYPE_FLOAT :
              for (i=0;i<n;i++)
               ((float *)outputColumns[indexField])[i]=9999.;
             break;
          // -------------------------------------------------------------------------
             case MEMORY_TYPE_DOUBLE :
              for (i=0;i<n;i++)
               ((double *)outputColumns[indexField])[i]=(double)9999.;
             break;
          // -------------------------------------------------------------------------
            }
         }
       }

      if ((outputMaxRecords<newRecordNumber) && !rc)
       outputMaxRecords=newRecordNumber;
     }

    outputNbRecords=0;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION        OUTPUT_Alloc
// -----------------------------------------------------------------------------
// PURPOSE         Allocate and initialize general use buffers for output
//
// RETURN          ERROR_ID_ALLOC if one of the buffer allocation failed
//                 ERROR_ID_NO in case of success
// -----------------------------------------------------------------------------

RC OUTPUT_Alloc(void)
 {
  // Declarations

  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  // Allocate buffers resp. for data to output

  if (((outputFields=(PRJCT_RESULTS_FIELDS *)MEMORY_AllocBuffer("OUTPUT_Alloc","outputFields",MAX_FIELDS,sizeof(PRJCT_RESULTS_FIELDS),0,MEMORY_TYPE_STRUCT))==NULL) ||
      ((outputColumns=(DoasCh **)MEMORY_AllocBuffer("OUTPUT_Alloc","outputColumns",MAX_FIELDS,sizeof(DoasCh *),0,MEMORY_TYPE_PTR))==NULL) ||
      ((OUTPUT_AmfSpace=(AMF_SYMBOL *)MEMORY_AllocBuffer("OUTPUT_Alloc ","OUTPUT_AmfSpace",MAX_SYMB,sizeof(AMF_SYMBOL),0,MEMORY_TYPE_STRUCT))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    memset(outputFields,0,sizeof(PRJCT_RESULTS_FIELDS)*MAX_FIELDS);
    memset(outputColumns,0,sizeof(DoasCh *)*MAX_FIELDS);

    outputNbDataSet=0;
    outputNbFields=0;
   }

  // Return

  if (OUTPUT_AmfSpace!=NULL)
   memset(OUTPUT_AmfSpace,0,sizeof(AMF_SYMBOL)*MAX_SYMB);

  return rc;
 }


// -----------------------------------------------------------------------------
// FUNCTION        OUTPUT_Free
// -----------------------------------------------------------------------------
// PURPOSE         Release buffer allocated for output
// -----------------------------------------------------------------------------

void OUTPUT_Free(void)
 {
  if (OUTPUT_AmfSpace!=NULL)
   MEMORY_ReleaseBuffer("OUTPUT_Free ","OUTPUT_AmfSpace",OUTPUT_AmfSpace);

  if (outputFields!=NULL)
   MEMORY_ReleaseBuffer("OUTPUT_Free","outputFields",outputFields);
  if (outputColumns!=NULL)
   MEMORY_ReleaseBuffer("OUTPUT_Free","outputColumns",outputColumns);
  if (outputRecords!=NULL)
   MEMORY_ReleaseBuffer("OUTPUT_Free","outputRecords",outputRecords);

  OUTPUT_AmfSpace=NULL;
  outputFields=NULL;
  outputColumns=NULL;
  outputRecords=NULL;
 }

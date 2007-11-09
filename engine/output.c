
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
//  Module purpose    :  OUTPUT
//  Name of module    :  OUTPUT.C
//  Program Language  :  Borland C++ 5.0 for Windows 95/NT
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
//  OutputReadAmf - load Air Mass Factors from file;
//  OutputFlux - return the flux at a specified wavelength;
//
//  OUTPUT_ResetData - release and reset all data used for output;
//  OUTPUT_LoadCross - load output part relative to a cross section from analysis windows properties;
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
//  OutputRegisterNasaAmes - register all the NASA-AMES fields;
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
//  =========
//  NASA-AMES
//  =========
//
//  OutputNasaAmesHeader - build the NASA-AMES file header;
//  OutputStoreNasaAmes - store results for further calculation of NASA-AMES AM/PM means;
//  OutputResidualColumns - calculate the residual columns in the selected reference spectrum;
//  OutputSlope - calculate the slope for NO2 rejection;
//  OUTPUT_SaveNasaAmes - save results in NASA-AMES format;
//  OUTPUT_SaveResults - save all results (ASC/BIN and NASA-AMES formats);
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

#include "doas.h"

typedef struct _NDSC_header
 {
  UCHAR investigator[20],
        instrument[12],
        station[12],
        species[12],
        startTime[20],
        endTime[20],
        qualityFlag[3];
 }
NDSC_HEADER;

PRJCT_RESULTS_FIELDS *outputFields;
UCHAR **outputColumns;
UCHAR OUTPUT_refFile[MAX_PATH_LEN+1];
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

UCHAR  OUTPUT_nasaFile[MAX_STR_LEN+1];                                            // name of file with NASA-AMES header description
INT    OUTPUT_nasaNResults;                                                          // number of results stored in NASA-AMES structure
INT    outputNasaFlag;
UCHAR  OUTPUT_currentSpeFile[MAX_ITEM_TEXT_LEN+1],                                      // complete results file name
       OUTPUT_currentAscFile[MAX_ITEM_TEXT_LEN+1];

RC OUTPUT_RegisterData(ENGINE_CONTEXT *pEngineContext)
 {
 	return 0;
 }
void OUTPUT_ResetData(void)
 {
 }
RC OUTPUT_SaveResults(ENGINE_CONTEXT *pEngineContext)
 {
 	return 0;
 }
RC OUTPUT_GetWveAmf(CROSS_RESULTS *pResults,double Zm,double *lembda,double *xs,double *deriv2)
 {
 	return 0;
 }
RC OUTPUT_LoadCross(LIST_ITEM *pList,CROSS_RESULTS *pResults,INT *pAmfFlag,INT hidden)
 {
 	return 0;
 }

// QDOAS ??? // ===================
// QDOAS ??? // STATIC DECLARATIONS
// QDOAS ??? // ===================
// QDOAS ???
// QDOAS ??? // TAKE CARE !!! TAKE CARE !!! TAKE CARE !!!  TAKE CARE !!! TAKE CARE !!! TAKE CARE !!! TAKE CARE !!! TAKE CARE !!!
// QDOAS ???
// QDOAS ??? // There is a close connection between the following structure and the NASA-AMES header file !!!
// QDOAS ??? //
// QDOAS ??? // * respect the order of constituents : NO2, O3, OClO, BrO
// QDOAS ??? // * the scaling factors to load from file are :
// QDOAS ??? //
// QDOAS ??? //       . vertical column, slant column, error bar and AMF for NO2 and O3;
// QDOAS ??? //       . slant column and error bar for OClO and BrO;
// QDOAS ??? //
// QDOAS ??? // * the names of constituents should be retaken in their respective lines of header;
// QDOAS ??? // * color indexes are the fluxes computed at 550 nm and 350 nm computed at 90° and 93° of zenith angle, and normalized
// QDOAS ??? //   by color indexes computed of the same way at 90°;
// QDOAS ??? // * other fields to output in NASA-AMES format are fixed;
// QDOAS ??? //
// QDOAS ??? // In conclusion, the only action done on original header file is the deletion of factors and description lines relative to
// QDOAS ??? // undesirable constituents according to the selection made in project properties panel, NASA-AMES results tab-page.
// QDOAS ???
// QDOAS ???
// QDOAS ??? NASA_RESULTS   OUTPUT_nasaResults;                                              // results in NASA-AMES format safe keeping
// QDOAS ??? UCHAR         *OUTPUT_nasaBuf=NULL,                                             // buffer for Nasa-Ames header file
// QDOAS ???                OUTPUT_nasaBufTemp[MAX_ITEM_TEXT_LEN+1];                         // temporary buffer used for cic scaling factors safe keeping
// QDOAS ??? double         OUTPUT_fluxes[MAX_FLUXES],OUTPUT_cic[MAX_CIC][2];                // fluxes and color indexes for ASCII results
// QDOAS ??? INT            OUTPUT_NFluxes,OUTPUT_NCic;                                      // resp. number of fluxes and color indexes in previous buffers
// QDOAS ??? AMF_SYMBOL    *OUTPUT_AmfSpace;                                                 // list of cross sections with associated AMF file
// QDOAS ??? INT            OUTPUT_NAmfSpace,                                                // number of elements in previous buffer
// QDOAS ???                OUTPUT_nasaBufSize,                                              // size of NASA-AMES header in terms of bytes
// QDOAS ???                OUTPUT_nasaBufNLines,                                            // size of NASA-AMES header in terms of lines
// QDOAS ???                OUTPUT_chiSquareFlag,                                            // 1 to save the chi square in the output file
// QDOAS ???                OUTPUT_rmsFlag,                                                  // 1 to save the RMS in the output file
// QDOAS ???                OUTPUT_refZmFlag,                                                // 1 to save the SZA of the reference spectrum in the output file
// QDOAS ???                OUTPUT_refShift,                                                 // 1 to save the shift of the reference spectrum on etalon
// QDOAS ???                OUTPUT_covarFlag,                                                // 1 to save the covariances in the output file
// QDOAS ???                OUTPUT_corrFlag,                                                 // 1 to save the correlations in the output file
// QDOAS ???                outputRunCalib,                                                  // 1 in run calibration mode
// QDOAS ???                outputCalibFlag;                                                 // <> 0 to save wavelength calibration parameters before analysis results
// QDOAS ???
// QDOAS ??? // ===============
// QDOAS ??? // DATA PROCESSING
// QDOAS ??? // ===============
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OUTPUT_GetWveAmf
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Correct a cross section using wavelength dependent AMF vector
// QDOAS ??? //
// QDOAS ??? // INPUT         pResults     output options for the selected cross section
// QDOAS ??? //               Zm           the current solar zenith angle
// QDOAS ??? //               lembda       the current wavelength calibration
// QDOAS ??? //
// QDOAS ??? // INPUT/OUTPUT  xs           the cross section to correct by wavelength dependent AMF
// QDOAS ??? // OUTPUT        deriv2       second derivatives of the new cross section
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OUTPUT_GetWveAmf(CROSS_RESULTS *pResults,double Zm,double *lembda,double *xs,double *deriv2)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   AMF_SYMBOL *pAmfSymbol;
// QDOAS ???   double *amfVector,*amfDeriv2,*phi,*phi80,scale,amf;
// QDOAS ???   INDEX indexLine,indexColumn,i;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   amfVector=amfDeriv2=phi=phi80=NULL;
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // This procedure applies only on wavelength dependent AMF
// QDOAS ???
// QDOAS ???   if ((pResults->indexAmf!=ITEM_NONE) && (OUTPUT_AmfSpace!=NULL))
// QDOAS ???    {
// QDOAS ???     if ((((pAmfSymbol=&OUTPUT_AmfSpace[pResults->indexAmf])->type==ANLYS_AMF_TYPE_WAVELENGTH1) ||
// QDOAS ???           (pAmfSymbol->type==ANLYS_AMF_TYPE_WAVELENGTH2)) &&
// QDOAS ???           (pAmfSymbol->xsLines!=0))
// QDOAS ???      {
// QDOAS ???       // Allocate buffers
// QDOAS ???
// QDOAS ???       if (((amfVector=(double *)MEMORY_AllocDVector("OUTPUT_GetWveAmf ","amfVector",0,pAmfSymbol->xsLines-1))==NULL) ||
// QDOAS ???           ((amfDeriv2=(double *)MEMORY_AllocDVector("OUTPUT_GetWveAmf ","amfDeriv2",0,pAmfSymbol->xsLines-1))==NULL) ||
// QDOAS ???           ((phi=(double *)MEMORY_AllocDVector("OUTPUT_GetWveAmf ","phi",2,pAmfSymbol->PhiColumns))==NULL) ||
// QDOAS ???           ((phi80=(double *)MEMORY_AllocDVector("OUTPUT_GetWveAmf ","phi80",2,pAmfSymbol->PhiColumns))==NULL))
// QDOAS ???
// QDOAS ???        rc=ERROR_ID_ALLOC;
// QDOAS ???
// QDOAS ???       else
// QDOAS ???        {
// QDOAS ???         // Interpolate parameters at zenith angles of Zm and 80°
// QDOAS ???
// QDOAS ???         for (indexColumn=2;indexColumn<=pAmfSymbol->PhiColumns;indexColumn++)
// QDOAS ???
// QDOAS ???          if (((rc=SPLINE_Vector(pAmfSymbol->Phi[1]+1,pAmfSymbol->Phi[indexColumn]+1,pAmfSymbol->deriv2[indexColumn]+1,pAmfSymbol->PhiLines,&Zm,&phi[indexColumn],1,SPLINE_CUBIC,"OUTPUT_GetWveAmf "))!=ERROR_ID_NO) ||
// QDOAS ???              ((rc=SPLINE_Vector(pAmfSymbol->Phi[1]+1,pAmfSymbol->Phi[indexColumn]+1,pAmfSymbol->deriv2[indexColumn]+1,pAmfSymbol->PhiLines,&Feno->Zm,&phi80[indexColumn],1,SPLINE_CUBIC,"OUTPUT_GetWveAmf "))!=ERROR_ID_NO))
// QDOAS ???
// QDOAS ???           goto EndOUTPUT_GetWveAmf;
// QDOAS ???
// QDOAS ???         // Compute amf vector dependent on wavelength in amf file
// QDOAS ???
// QDOAS ???         if (pAmfSymbol->type==ANLYS_AMF_TYPE_WAVELENGTH1)
// QDOAS ???
// QDOAS ???          for (indexLine=0;indexLine<pAmfSymbol->xsLines;indexLine++)
// QDOAS ???           {
// QDOAS ???            amfVector[indexLine]=(double)0.;
// QDOAS ???            scale=(double)1.;
// QDOAS ???
// QDOAS ???            for (indexColumn=2;indexColumn<=pAmfSymbol->PhiColumns-pAmfSymbol->xsColumns;indexColumn++)
// QDOAS ???             {
// QDOAS ???              amfVector[indexLine]+=scale*(phi[indexColumn]-phi80[indexColumn]);
// QDOAS ???              scale*=pAmfSymbol->xs[0][indexLine];
// QDOAS ???             }
// QDOAS ???
// QDOAS ???            for (;indexColumn<pAmfSymbol->PhiColumns;indexColumn++)
// QDOAS ???             amfVector[indexLine]+=(phi[indexColumn]-phi80[indexColumn])*pAmfSymbol->xs[indexColumn-(pAmfSymbol->PhiColumns-pAmfSymbol->xsColumns)][indexLine];
// QDOAS ???
// QDOAS ???            if (phi[indexColumn]==(double)0.)
// QDOAS ???             {
// QDOAS ???              rc=ERROR_SetLast("OUTPUT_GetWveAmf",ERROR_TYPE_FATAL,ERROR_ID_DIVISION_BY_0,"calculation of the air mass factor");
// QDOAS ???              goto EndOUTPUT_GetWveAmf;
// QDOAS ???             }
// QDOAS ???
// QDOAS ???            amfVector[indexLine]*=phi80[indexColumn]/phi[indexColumn];
// QDOAS ???           }
// QDOAS ???
// QDOAS ???         else // Case 2
// QDOAS ???
// QDOAS ???          for (indexLine=0;indexLine<pAmfSymbol->xsLines;indexLine++)
// QDOAS ???           {
// QDOAS ???            amfVector[indexLine]=(double)0.;
// QDOAS ???            scale=(double)1.;
// QDOAS ???
// QDOAS ???            for (indexColumn=2;indexColumn<=pAmfSymbol->PhiColumns-pAmfSymbol->xsColumns;indexColumn++)
// QDOAS ???             {
// QDOAS ???              amfVector[indexLine]+=scale*phi[indexColumn];
// QDOAS ???              scale*=pAmfSymbol->xs[0][indexLine];
// QDOAS ???             }
// QDOAS ???
// QDOAS ???            for (;indexColumn<pAmfSymbol->PhiColumns;indexColumn++)
// QDOAS ???             amfVector[indexLine]+=phi[indexColumn]*pAmfSymbol->xs[indexColumn-(pAmfSymbol->PhiColumns-pAmfSymbol->xsColumns)][indexLine];
// QDOAS ???           }
// QDOAS ???
// QDOAS ???         rc=SPLINE_Deriv2((double *)pAmfSymbol->xs[0],amfVector,amfDeriv2,pAmfSymbol->xsLines,"OUTPUT_GetWveAmf ");
// QDOAS ???
// QDOAS ???         for (i=0;(i<NDET) && !rc;i++)
// QDOAS ???          if (!(rc=SPLINE_Vector((double *)pAmfSymbol->xs[0],amfVector,amfDeriv2,pAmfSymbol->xsLines,&lembda[i],&amf,1,SPLINE_CUBIC,"OUTPUT_GetWveAmf ")))
// QDOAS ???           xs[i]*=amf;
// QDOAS ???
// QDOAS ???         if (!rc)
// QDOAS ???          rc=SPLINE_Deriv2((double *)ANALYSE_splineX,xs,deriv2+1,NDET,"OUTPUT_GetWveAmf (2) ");
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???     else if (pAmfSymbol->type==ANLYS_AMF_TYPE_WAVELENGTH3)
// QDOAS ???      for (i=0;i<NDET;i++)
// QDOAS ???       xs[i]*=(double)VECTOR_Table2(pAmfSymbol->Phi,pAmfSymbol->PhiLines,pAmfSymbol->PhiColumns,(double)lembda[i],(double)Zm);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   EndOUTPUT_GetWveAmf :
// QDOAS ???
// QDOAS ???   // Release allocated buffers
// QDOAS ???
// QDOAS ???   if (amfVector!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("OUTPUT_GetWveAmf","amfVector",amfVector,0);
// QDOAS ???   if (amfDeriv2!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("OUTPUT_GetWveAmf","amfDeriv2",amfDeriv2,0);
// QDOAS ???   if (phi!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("OUTPUT_GetWveAmf","phi",phi,2);
// QDOAS ???   if (phi80)
// QDOAS ???    MEMORY_ReleaseDVector("OUTPUT_GetWveAmf","phi80",phi80,2);
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputGetAmf
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Return the AMF from table at a specified zenith angle
// QDOAS ??? //
// QDOAS ??? // INPUT         pResults     output options for the selected cross section
// QDOAS ??? //               Zm           the current solar zenith angle
// QDOAS ??? //               Tm           the current number of seconds in order to retrieve
// QDOAS ??? //                            AMF from a climatology table
// QDOAS ??? //
// QDOAS ??? // OUTPUT        pAmf         the calculated AMF
// QDOAS ??? //
// QDOAS ??? // RETURN        0 in case of success, any other value in case of error
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OutputGetAmf(CROSS_RESULTS *pResults,double Zm,double Tm,double *pAmf)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   AMF_SYMBOL *pAmfSymbol;
// QDOAS ???   double Dte;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???   *pAmf=(double)1.;
// QDOAS ???
// QDOAS ???   if ((pResults->indexAmf!=ITEM_NONE) && (OUTPUT_AmfSpace!=NULL))
// QDOAS ???    {
// QDOAS ???     pAmfSymbol=&OUTPUT_AmfSpace[pResults->indexAmf];
// QDOAS ???
// QDOAS ???     if (pAmfSymbol->Phi!=NULL)
// QDOAS ???
// QDOAS ???      switch(pAmfSymbol->type)
// QDOAS ???       {
// QDOAS ???     // ------------------------------------------------------------------------
// QDOAS ???        case ANLYS_AMF_TYPE_CLIMATOLOGY :
// QDOAS ???         {
// QDOAS ???          Dte=(double) (ZEN_FNCaljda(&Tm)-1.)/365.;
// QDOAS ???
// QDOAS ???          if ((Zm>=(double)pAmfSymbol->Phi[1][2]) && (Zm<=(double)pAmfSymbol->Phi[1][pAmfSymbol->PhiLines]) &&
// QDOAS ???              (Dte>=(double)pAmfSymbol->Phi[2][1]) && (Dte<=(double)pAmfSymbol->Phi[pAmfSymbol->PhiColumns][1]))
// QDOAS ???
// QDOAS ???           *pAmf=(double)VECTOR_Table2(pAmfSymbol->Phi,pAmfSymbol->PhiLines,pAmfSymbol->PhiColumns,(double)Zm,(double)Dte);
// QDOAS ???         }
// QDOAS ???        break;
// QDOAS ???     // ------------------------------------------------------------------------
// QDOAS ???        case ANLYS_AMF_TYPE_SZA :
// QDOAS ???
// QDOAS ???         if ((pAmfSymbol->deriv2!=NULL) &&
// QDOAS ???            ((rc=SPLINE_Vector(pAmfSymbol->Phi[1]+1,pAmfSymbol->Phi[2]+1,pAmfSymbol->deriv2[2]+1,pAmfSymbol->PhiLines,&Zm,pAmf,1,
// QDOAS ???                                  SPLINE_CUBIC,"OutputGetAmf "))!=ERROR_ID_NO))
// QDOAS ???
// QDOAS ???          rc=ERROR_ID_AMF;
// QDOAS ???
// QDOAS ???        break;
// QDOAS ???     // ------------------------------------------------------------------------
// QDOAS ???       }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputReadAmf
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Load Air Mass Factors from file
// QDOAS ??? //
// QDOAS ??? // INPUT         symbolName   the name of the selected symbol
// QDOAS ??? //               amfFileName  the name of the AMF file to load
// QDOAS ??? //               amfType      the type of AMF to load
// QDOAS ??? //
// QDOAS ??? // OUTPUT        pIndexAmf    the index of AMF data in the AMF table
// QDOAS ??? //
// QDOAS ??? // RETURN        0 in case of success, any other value in case of error
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OutputReadAmf(UCHAR *symbolName,UCHAR *amfFileName,UCHAR *amfType,INDEX *pIndexAmf)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR  fileType,                                                              // file extension and type
// QDOAS ???         *oldColumn,*nextColumn,                                                 // go to the next column in record or the next record
// QDOAS ???          fileName[MAX_ITEM_TEXT_LEN+1];                                         // the complete AMF file name
// QDOAS ???   SZ_LEN symbolLength,fileLength,lineLength;                                    // length of symbol and lines strings
// QDOAS ???   AMF_SYMBOL *pAmfSymbol;                                                       // pointer to an AMF symbol
// QDOAS ???   INDEX indexSymbol,indexLine,indexColumn;                                      // indexes for loops and arrays
// QDOAS ???   INT PhiLines,PhiColumns,xsLines,xsColumns,oldPhiColumns;                      // dimensions of the AMF matrix
// QDOAS ???   FILE *amfFp;                                                                  // pointer to AMF file
// QDOAS ???   double **Phi,**deriv2,**xs,**xsDeriv2,                                        // pointers to resp. AMF buffer and second derivatives
// QDOAS ???            tempPhi;                                                             // temporary variable
// QDOAS ???   RC rc;                                                                        // return code
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???   nextColumn=oldColumn=NULL;
// QDOAS ???   xsLines=xsColumns=0;
// QDOAS ???   amfFp=NULL;
// QDOAS ???
// QDOAS ???   FILES_RebuildFileName(fileName,amfFileName,1);
// QDOAS ???
// QDOAS ???   if (!(symbolLength=strlen(symbolName)) || !(fileLength=strlen(fileName)) || !STD_Stricmp(amfType,ANLYS_amf[ANLYS_AMF_TYPE_NONE]))
// QDOAS ???    indexSymbol=ITEM_NONE;
// QDOAS ???   else if (OUTPUT_AmfSpace!=NULL) // ELSE LEVEL 0
// QDOAS ???    {
// QDOAS ???     // Retrieve the type of file
// QDOAS ???
// QDOAS ???     if (!STD_Stricmp(amfType,ANLYS_amf[ANLYS_AMF_TYPE_SZA]))
// QDOAS ???      fileType=ANLYS_AMF_TYPE_SZA;
// QDOAS ???     else if (!STD_Stricmp(amfType,ANLYS_amf[ANLYS_AMF_TYPE_CLIMATOLOGY]))
// QDOAS ???      fileType=ANLYS_AMF_TYPE_CLIMATOLOGY;
// QDOAS ???     else if (!STD_Stricmp(amfType,ANLYS_amf[ANLYS_AMF_TYPE_WAVELENGTH1]))
// QDOAS ???      fileType=ANLYS_AMF_TYPE_WAVELENGTH1;
// QDOAS ???     else if (!STD_Stricmp(amfType,ANLYS_amf[ANLYS_AMF_TYPE_WAVELENGTH2]))
// QDOAS ???      fileType=ANLYS_AMF_TYPE_WAVELENGTH2;
// QDOAS ???     else
// QDOAS ???      fileType=ANLYS_AMF_TYPE_WAVELENGTH3;
// QDOAS ???
// QDOAS ???     // Search for symbol in list
// QDOAS ???
// QDOAS ???     for (indexSymbol=0;indexSymbol<OUTPUT_NAmfSpace;indexSymbol++)
// QDOAS ???      {
// QDOAS ???       pAmfSymbol=&OUTPUT_AmfSpace[indexSymbol];
// QDOAS ???
// QDOAS ???       if ((pAmfSymbol->type==fileType) &&
// QDOAS ???           (strlen(pAmfSymbol->symbolName)==symbolLength) &&
// QDOAS ???           (strlen(pAmfSymbol->amfFileName)==fileLength) &&
// QDOAS ???           !STD_Stricmp(pAmfSymbol->symbolName,symbolName) &&
// QDOAS ???           !STD_Stricmp(pAmfSymbol->amfFileName,fileName))
// QDOAS ???
// QDOAS ???        break;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // A new symbol is found
// QDOAS ???
// QDOAS ???     if (indexSymbol==OUTPUT_NAmfSpace)
// QDOAS ???      {
// QDOAS ???       // Symbol list is limited to MAX_SYMB symbols
// QDOAS ???
// QDOAS ???       if (OUTPUT_NAmfSpace>=MAX_SYMB)
// QDOAS ???
// QDOAS ???        THRD_Error(ERROR_TYPE_FATAL,(rc=ERROR_ID_MEMORY),"OutputReadAmf (OUTPUT_AmfSpace) ");
// QDOAS ???
// QDOAS ???       // File read out
// QDOAS ???
// QDOAS ???       else    // ELSE LEVEL 1
// QDOAS ???        {
// QDOAS ???         if ((amfFp=fopen(fileName,"rt"))==NULL)
// QDOAS ???          THRD_Error(ERROR_TYPE_FATAL,(rc=ERROR_ID_FILE_NOT_FOUND),"OutputReadAmf ",fileName);
// QDOAS ???         else if (!(fileLength=STD_FileLength(amfFp)))
// QDOAS ???          rc=ERROR_SetLast("OutputReadAmf",ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,fileName);
// QDOAS ???         else if (((nextColumn=(UCHAR *)MEMORY_AllocBuffer("OutputReadAmf ","nextColumn",fileLength+1,1,0,MEMORY_TYPE_STRING))==NULL) ||
// QDOAS ???                  ((oldColumn=(UCHAR *)MEMORY_AllocBuffer("OutputReadAmf ","oldColumn",fileLength+1,1,0,MEMORY_TYPE_STRING))==NULL))
// QDOAS ???          rc=ERROR_ID_ALLOC;
// QDOAS ???         else
// QDOAS ???          {
// QDOAS ???           if ((fileType==ANLYS_AMF_TYPE_CLIMATOLOGY) || (fileType==ANLYS_AMF_TYPE_SZA) || (fileType==ANLYS_AMF_TYPE_WAVELENGTH3))
// QDOAS ???            rc=FILES_GetMatrixDimensions(amfFp,fileName,&PhiLines,&PhiColumns,"OutputReadAmf",ERROR_TYPE_FATAL);
// QDOAS ???           else // polynomial wavelength dependence
// QDOAS ???            {
// QDOAS ???             while (fgets(oldColumn,fileLength,amfFp) && ((strchr(oldColumn,';')!=NULL) || (strchr(oldColumn,'*')!=NULL)));
// QDOAS ???
// QDOAS ???             // Parameters matrix
// QDOAS ???
// QDOAS ???             for (xsLines=oldPhiColumns=PhiColumns=0;PhiColumns==oldPhiColumns;xsLines++ /* NB : xsLines and PhiLines are invert in this case */)
// QDOAS ???              {
// QDOAS ???               for (PhiColumns=0;strlen(oldColumn);PhiColumns++)
// QDOAS ???                {
// QDOAS ???                 lineLength=strlen(oldColumn);
// QDOAS ???
// QDOAS ???                 oldColumn[lineLength++]='\n';
// QDOAS ???                 oldColumn[lineLength]=0;
// QDOAS ???
// QDOAS ???                 memset(nextColumn,0,fileLength);
// QDOAS ???                 sscanf(oldColumn,"%lf %[^'\n']",(double *)&tempPhi,nextColumn);
// QDOAS ???                 strcpy(oldColumn,nextColumn);
// QDOAS ???                }
// QDOAS ???
// QDOAS ???               if (!xsLines)
// QDOAS ???                oldPhiColumns=PhiColumns;
// QDOAS ???
// QDOAS ???               fgets(oldColumn,fileLength,amfFp);
// QDOAS ???              }
// QDOAS ???
// QDOAS ???             xsColumns=PhiColumns;
// QDOAS ???             PhiColumns=oldPhiColumns;
// QDOAS ???             PhiLines=2;
// QDOAS ???             xsLines--;
// QDOAS ???
// QDOAS ???             for (;!feof(amfFp) && fgets(oldColumn,fileLength,amfFp);PhiLines++);
// QDOAS ???
// QDOAS ???             indexLine=PhiLines;
// QDOAS ???             PhiLines=xsLines;
// QDOAS ???             xsLines=indexLine;
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         if (rc)
// QDOAS ???          goto EndOutputReadAmf;
// QDOAS ???
// QDOAS ???         // Allocate AMF matrix
// QDOAS ???
// QDOAS ???         pAmfSymbol=&OUTPUT_AmfSpace[indexSymbol];
// QDOAS ???         fseek(amfFp,0L,SEEK_SET);
// QDOAS ???
// QDOAS ???         if ((((fileType==ANLYS_AMF_TYPE_WAVELENGTH1) || (fileType==ANLYS_AMF_TYPE_WAVELENGTH2)) && ((xsLines==0) || (xsColumns==0) ||
// QDOAS ???              ((xs=pAmfSymbol->xs=(double **)MEMORY_AllocDMatrix("OutputReadAmf ","xs",0,xsLines-1,0,xsColumns-1))==NULL) ||
// QDOAS ???              ((xsDeriv2=pAmfSymbol->xsDeriv2=(double **)MEMORY_AllocDMatrix("OutputReadAmf ","xsDeriv2",1,xsLines,1,xsColumns-1))==NULL))) ||
// QDOAS ???              ((Phi=pAmfSymbol->Phi=(double **)MEMORY_AllocDMatrix("OutputReadAmf ","Phi",0,PhiLines,1,PhiColumns))==NULL) ||
// QDOAS ???              ((fileType!=ANLYS_AMF_TYPE_CLIMATOLOGY) && (fileType!=ANLYS_AMF_TYPE_WAVELENGTH3) &&
// QDOAS ???              ((deriv2=pAmfSymbol->deriv2=(double **)MEMORY_AllocDMatrix("OutputReadAmf ","deriv2",1,PhiLines,2,PhiColumns))==NULL)))
// QDOAS ???
// QDOAS ???          rc=ERROR_ID_ALLOC;
// QDOAS ???
// QDOAS ???         else if (fileType==ANLYS_AMF_TYPE_CLIMATOLOGY)
// QDOAS ???          rc=FILES_LoadMatrix(amfFp,fileName,Phi,1,PhiLines,PhiColumns,"OutputReadAmf",ERROR_TYPE_FATAL);
// QDOAS ???         else if (fileType==ANLYS_AMF_TYPE_SZA)
// QDOAS ???          rc=FILES_LoadMatrix(amfFp,fileName,Phi,1,PhiLines,PhiColumns,"OutputReadAmf",ERROR_TYPE_FATAL);
// QDOAS ???         else if (fileType==ANLYS_AMF_TYPE_WAVELENGTH3)
// QDOAS ???          rc=FILES_LoadMatrix(amfFp,fileName,Phi,1,PhiLines,PhiColumns,"OutputReadAmf",ERROR_TYPE_FATAL);
// QDOAS ???         else     // wavelength dependence
// QDOAS ???          {
// QDOAS ???           while (fgets(oldColumn,fileLength,amfFp) && ((strchr(oldColumn,';')!=NULL) || (strchr(oldColumn,'*')!=NULL)));
// QDOAS ???
// QDOAS ???           // Fill AMF matrix
// QDOAS ???
// QDOAS ???           for (indexLine=1;indexLine<=PhiLines;indexLine++)
// QDOAS ???            {
// QDOAS ???             for (indexColumn=1;indexColumn<=PhiColumns;indexColumn++)
// QDOAS ???              {
// QDOAS ???               lineLength=strlen(oldColumn);
// QDOAS ???
// QDOAS ???               oldColumn[lineLength++]='\n';
// QDOAS ???               oldColumn[lineLength]=0;
// QDOAS ???
// QDOAS ???               memset(nextColumn,0,fileLength);
// QDOAS ???               sscanf(oldColumn,"%lf %[^'\n']",(double *)&Phi[indexColumn][indexLine],nextColumn);
// QDOAS ???               strcpy(oldColumn,nextColumn);
// QDOAS ???              }
// QDOAS ???
// QDOAS ???             fgets(oldColumn,fileLength,amfFp);
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           // For wavelength dependent AMF, read out cross sections
// QDOAS ???
// QDOAS ???           for (indexLine=0;indexLine<xsLines;indexLine++)
// QDOAS ???            {
// QDOAS ???             for (indexColumn=0;indexColumn<xsColumns;indexColumn++)
// QDOAS ???              {
// QDOAS ???               lineLength=strlen(oldColumn);
// QDOAS ???
// QDOAS ???               oldColumn[lineLength++]='\n';
// QDOAS ???               oldColumn[lineLength]=0;
// QDOAS ???
// QDOAS ???               memset(nextColumn,0,fileLength);
// QDOAS ???               sscanf(oldColumn,"%lf %[^'\n']",(double *)&xs[indexColumn][indexLine],nextColumn);
// QDOAS ???               strcpy(oldColumn,nextColumn);
// QDOAS ???              }
// QDOAS ???
// QDOAS ???             fgets(oldColumn,fileLength,amfFp);
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         if (rc)
// QDOAS ???          goto EndOutputReadAmf;
// QDOAS ???
// QDOAS ???         // Second derivatives computations
// QDOAS ???
// QDOAS ???         if ((fileType!=ANLYS_AMF_TYPE_CLIMATOLOGY) && (fileType!=ANLYS_AMF_TYPE_WAVELENGTH3))
// QDOAS ???          for (indexColumn=2;(indexColumn<=PhiColumns) && !rc;indexColumn++)
// QDOAS ???           rc=SPLINE_Deriv2(Phi[1]+1,Phi[indexColumn]+1,deriv2[indexColumn]+1,PhiLines,"OutputReadAmf ");
// QDOAS ???
// QDOAS ???         if ((fileType==ANLYS_AMF_TYPE_WAVELENGTH1) || (fileType==ANLYS_AMF_TYPE_WAVELENGTH2))
// QDOAS ???          for (indexColumn=1;indexColumn<xsColumns;indexColumn++)
// QDOAS ???           rc=SPLINE_Deriv2((double *)xs[0],(double *)xs[indexColumn],xsDeriv2[indexColumn]+1,xsLines,"OutputReadAmf (2) ");
// QDOAS ???
// QDOAS ???         // Add new symbol
// QDOAS ???
// QDOAS ???         if (rc==ERROR_ID_NO)
// QDOAS ???          {
// QDOAS ???           pAmfSymbol=&OUTPUT_AmfSpace[indexSymbol];
// QDOAS ???
// QDOAS ???           strcpy(pAmfSymbol->symbolName,symbolName);
// QDOAS ???           strcpy(pAmfSymbol->amfFileName,fileName);
// QDOAS ???
// QDOAS ???           pAmfSymbol->type=fileType;
// QDOAS ???           pAmfSymbol->PhiLines=PhiLines;
// QDOAS ???           pAmfSymbol->PhiColumns=PhiColumns;
// QDOAS ???           pAmfSymbol->xsLines=xsLines;
// QDOAS ???           pAmfSymbol->xsColumns=xsColumns;
// QDOAS ???
// QDOAS ???           OUTPUT_NAmfSpace++;
// QDOAS ???          }
// QDOAS ???        }     // END ELSE LEVEL 1
// QDOAS ???      }    // END if (indexSymbol==OUTPUT_NAmfSpace)
// QDOAS ???
// QDOAS ???     if (indexSymbol>=OUTPUT_NAmfSpace)
// QDOAS ???      indexSymbol=ITEM_NONE;
// QDOAS ???    }   // END ELSE LEVEL 0
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   EndOutputReadAmf :
// QDOAS ???
// QDOAS ???   if (nextColumn!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("OutputReadAmf ","nextColumn",nextColumn);
// QDOAS ???   if (oldColumn!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("OutputReadAmf ","oldColumn",oldColumn);
// QDOAS ???   if (amfFp!=NULL)
// QDOAS ???    fclose(amfFp);
// QDOAS ???
// QDOAS ???   *pIndexAmf=indexSymbol;
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputFlux
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       return the flux at a specified wavelength
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext    information on the current spectrum
// QDOAS ??? //               waveLength   the wavelength
// QDOAS ??? //
// QDOAS ??? // RETURN        the flux calculated at the input wavelength
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? double OutputFlux(ENGINE_CONTEXT *pEngineContext,double waveLength)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   double flux;
// QDOAS ???   INDEX i,imin,imax;
// QDOAS ???   INT pixel;
// QDOAS ???
// QDOAS ???   if ((waveLength<pEngineContext->lembda[0]) || (waveLength>pEngineContext->lembda[NDET-1]))
// QDOAS ???    flux=(double)0.;
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     // Initialization
// QDOAS ???
// QDOAS ???     pixel=FNPixel(pEngineContext->lembda,waveLength,NDET,PIXEL_CLOSEST);
// QDOAS ???
// QDOAS ???     imin=max(pixel-3,0);
// QDOAS ???     imax=min(pixel+3,NDET-1);
// QDOAS ???
// QDOAS ???     // Flux calculation
// QDOAS ???
// QDOAS ???     for (i=imin,flux=(double)0.;i<=imax;i++)
// QDOAS ???      flux+=pEngineContext->spectrum[i];
// QDOAS ???
// QDOAS ???     flux/=(double)(imax-imin+1);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return flux;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OUTPUT_ResetData
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       release and reset all data used for output
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void OUTPUT_ResetData(void)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   FENO *pTabFeno;
// QDOAS ???   INDEX indexFeno,indexTabCross,indexSymbol,indexField;
// QDOAS ???   NASA_COMPONENTS *pComponent;
// QDOAS ???   CROSS_RESULTS *pResults;
// QDOAS ???   AMF_SYMBOL *pAmf;
// QDOAS ???
// QDOAS ???   // Reset output part of data in analysis windows
// QDOAS ???
// QDOAS ???   for (indexFeno=0;indexFeno<MAX_FENO;indexFeno++)
// QDOAS ???    {
// QDOAS ???     pTabFeno=&TabFeno[indexFeno];
// QDOAS ???
// QDOAS ???     // Browse symbols
// QDOAS ???
// QDOAS ???     for (indexTabCross=0;indexTabCross<MAX_FIT;indexTabCross++)
// QDOAS ???      {
// QDOAS ???       pResults=&pTabFeno->TabCrossResults[indexTabCross];
// QDOAS ???
// QDOAS ???       pResults->indexAmf=ITEM_NONE;
// QDOAS ???    // -------------------------------------------
// QDOAS ???       pResults->StoreAmf=
// QDOAS ???       pResults->StoreShift=
// QDOAS ???       pResults->StoreStretch=
// QDOAS ???       pResults->StoreScale=
// QDOAS ???       pResults->StoreError=
// QDOAS ???       pResults->StoreSlntCol=
// QDOAS ???       pResults->StoreSlntErr=
// QDOAS ???       pResults->StoreVrtCol=
// QDOAS ???       pResults->StoreVrtErr=(UCHAR)0;
// QDOAS ???    // -------------------------------------------
// QDOAS ???       pResults->ResCol=(double)0.;
// QDOAS ???    // -------------------------------------------
// QDOAS ???       pResults->Amf=
// QDOAS ???       pResults->SlntCol=
// QDOAS ???       pResults->SlntErr=
// QDOAS ???       pResults->VrtCol=
// QDOAS ???       pResults->VrtErr=(double)9999.;
// QDOAS ???    // -------------------------------------------
// QDOAS ???       pResults->SlntFact=
// QDOAS ???       pResults->VrtFact=(double)1.;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Release AMF matrices
// QDOAS ???
// QDOAS ???   if (OUTPUT_AmfSpace!=NULL)
// QDOAS ???    {
// QDOAS ???     for (indexSymbol=0;indexSymbol<MAX_SYMB;indexSymbol++)
// QDOAS ???      {
// QDOAS ???       pAmf=&OUTPUT_AmfSpace[indexSymbol];
// QDOAS ???
// QDOAS ???       if (pAmf->Phi!=NULL)
// QDOAS ???        MEMORY_ReleaseDMatrix("OUTPUT_ResetData ","Phi",pAmf->Phi,1,pAmf->PhiColumns,0);
// QDOAS ???       if (pAmf->deriv2!=NULL)
// QDOAS ???        MEMORY_ReleaseDMatrix("OUTPUT_ResetData ","deriv2",pAmf->deriv2,2,pAmf->PhiColumns,1);
// QDOAS ???       if (pAmf->xs!=NULL)
// QDOAS ???        MEMORY_ReleaseDMatrix("OUTPUT_ResetData ","xs",pAmf->xs,0,pAmf->xsColumns-1,0);
// QDOAS ???       if (pAmf->xsDeriv2!=NULL)
// QDOAS ???        MEMORY_ReleaseDMatrix("OUTPUT_ResetData ","xsDeriv2",pAmf->xsDeriv2,1,pAmf->xsColumns-1,1);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     memset(OUTPUT_AmfSpace,0,sizeof(AMF_SYMBOL)*MAX_SYMB);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Reset Amf
// QDOAS ???
// QDOAS ???   OUTPUT_NAmfSpace=0;
// QDOAS ???
// QDOAS ???   // Release NASA-AMES vectors used for results safe keeping and reset data
// QDOAS ???
// QDOAS ???   if (OUTPUT_nasaResults.Zm!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("OUTPUT_ResetData ","Zm",OUTPUT_nasaResults.Zm,0);
// QDOAS ???   if (OUTPUT_nasaResults.Tm!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("OUTPUT_ResetData ","Tm",OUTPUT_nasaResults.Tm,0);
// QDOAS ???   if (OUTPUT_nasaResults.TDet!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("OUTPUT_ResetData ","TDet",OUTPUT_nasaResults.TDet,0);
// QDOAS ???   if (OUTPUT_nasaResults.TimeDec!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("OUTPUT_ResetData ","TimeDec",OUTPUT_nasaResults.TimeDec,0);
// QDOAS ???   if (OUTPUT_nasaResults.Cic!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("OUTPUT_ResetData","Cic",OUTPUT_nasaResults.Cic,0);
// QDOAS ???
// QDOAS ???   OUTPUT_nasaResults.Zm=
// QDOAS ???   OUTPUT_nasaResults.Tm=
// QDOAS ???   OUTPUT_nasaResults.TDet=
// QDOAS ???   OUTPUT_nasaResults.TimeDec=
// QDOAS ???   OUTPUT_nasaResults.Cic=NULL;
// QDOAS ???
// QDOAS ???   OUTPUT_nasaResults.julianDay=
// QDOAS ???   OUTPUT_nasaResults.oldJulianDay=0;
// QDOAS ???   OUTPUT_nasaResults.refZm=(double)0.;
// QDOAS ???   OUTPUT_nasaNResults=0;
// QDOAS ???
// QDOAS ???   for (indexField=0;indexField<PRJCT_RESULTS_NASA_MAX;indexField++)
// QDOAS ???    {
// QDOAS ???     pComponent=&OUTPUT_nasaResults.components[indexField];
// QDOAS ???
// QDOAS ???     if (pComponent->SlntCol!=NULL)
// QDOAS ???      MEMORY_ReleaseDVector("OUTPUT_ResetData ","SlntCol",pComponent->SlntCol,0);
// QDOAS ???     if (pComponent->Error!=NULL)
// QDOAS ???      MEMORY_ReleaseDVector("OUTPUT_ResetData ","Error",pComponent->Error,0);
// QDOAS ???     if (pComponent->Amf!=NULL)
// QDOAS ???      MEMORY_ReleaseDVector("OUTPUT_ResetData ","Amf",pComponent->Amf,0);
// QDOAS ???
// QDOAS ???     pComponent->SlntCol=
// QDOAS ???     pComponent->Error=
// QDOAS ???     pComponent->Amf=NULL;
// QDOAS ???
// QDOAS ???     pComponent->indexWindow=
// QDOAS ???     pComponent->indexTabCross=ITEM_NONE;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   memset(OUTPUT_currentSpeFile,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   memset(OUTPUT_currentAscFile,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???   for (indexField=0;indexField<outputNbFields;indexField++)
// QDOAS ???    if (outputColumns[indexField]!=NULL)
// QDOAS ???     MEMORY_ReleaseBuffer("OUTPUT_ResetData",outputFields[indexField].fieldName,outputColumns[indexField]);
// QDOAS ???
// QDOAS ???   memset(outputFields,0,sizeof(PRJCT_RESULTS_FIELDS)*MAX_FIELDS);
// QDOAS ???   memset(outputColumns,0,sizeof(UCHAR *)*MAX_FIELDS);
// QDOAS ???
// QDOAS ???   if (outputRecords!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("OUTPUT_ResetData","outputRecords",outputRecords);
// QDOAS ???
// QDOAS ???   outputRecords=NULL;
// QDOAS ???
// QDOAS ???   OUTPUT_chiSquareFlag=
// QDOAS ???   OUTPUT_rmsFlag=
// QDOAS ???   OUTPUT_refZmFlag=
// QDOAS ???   OUTPUT_refShift=
// QDOAS ???   OUTPUT_covarFlag=
// QDOAS ???   OUTPUT_corrFlag=
// QDOAS ???   outputRunCalib=
// QDOAS ???   outputCalibFlag=
// QDOAS ???   outputNbFields=
// QDOAS ???   outputNbDataSet=
// QDOAS ???   outputNbRecords=
// QDOAS ???   outputMaxRecords=0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OUTPUT_LoadCross
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Load output part relative to a cross section from analysis windows properties
// QDOAS ??? //
// QDOAS ??? // INPUT         pList      options from the output page of analysis windows properties
// QDOAS ??? //               pResults   output options
// QDOAS ??? //               hidden     1 for the calibration options, 0 for analysis options
// QDOAS ??? //
// QDOAS ??? // OUTPUT        pAmfFlag   non zero if wavelength dependent AMF have to be accounted for
// QDOAS ??? //
// QDOAS ??? // RETURN        return code
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OUTPUT_LoadCross(LIST_ITEM *pList,CROSS_RESULTS *pResults,INT *pAmfFlag,INT hidden)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Read Amf from files
// QDOAS ???
// QDOAS ???   if (!(rc=OutputReadAmf(pList->itemText[COLUMN_CROSS_FILE],pList->amfFileName,pList->itemText[COLUMN_CROSS_AMF_TYPE],&pResults->indexAmf)))
// QDOAS ???    {
// QDOAS ???     // Load fields dependent on AMF
// QDOAS ???
// QDOAS ???     if ((pResults->indexAmf!=ITEM_NONE) && (OUTPUT_AmfSpace!=NULL))
// QDOAS ???      {
// QDOAS ???       if ((OUTPUT_AmfSpace[pResults->indexAmf].type==ANLYS_AMF_TYPE_WAVELENGTH1) ||
// QDOAS ???           (OUTPUT_AmfSpace[pResults->indexAmf].type==ANLYS_AMF_TYPE_WAVELENGTH2) ||
// QDOAS ???           (OUTPUT_AmfSpace[pResults->indexAmf].type==ANLYS_AMF_TYPE_WAVELENGTH3))
// QDOAS ???
// QDOAS ???        (*pAmfFlag)++;
// QDOAS ???
// QDOAS ???       pResults->StoreAmf=(pList->itemText[COLUMN_CROSS_AMF_OUTPUT][0]=='1')?(UCHAR)1:(UCHAR)0;          // flag set if AMF is to be written into output file
// QDOAS ???       pResults->StoreVrtCol=(pList->itemText[COLUMN_CROSS_VRTCOL][0]=='1')?(UCHAR)1:(UCHAR)0;           // flag set if vertical column is to be written into output file
// QDOAS ???       pResults->StoreVrtErr=(pList->itemText[COLUMN_CROSS_VRTERR][0]=='1')?(UCHAR)1:(UCHAR)0;           // flag set if error on vertical column is to be written into output file
// QDOAS ???       pResults->VrtFact=atof(pList->itemText[COLUMN_CROSS_VRTFACT]);                                    // vertical column factor
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Load fields independent from AMF
// QDOAS ???
// QDOAS ???     if (hidden==1)
// QDOAS ???      {
// QDOAS ???       pResults->StoreSlntCol=(pList->itemText[COLUMN_CROSS_SVD_SLNTCOL][0]=='1')?(UCHAR)1:(UCHAR)0;       // flag set if slant column is to be written into output file
// QDOAS ???       pResults->StoreSlntErr=(pList->itemText[COLUMN_CROSS_SVD_SLNTERR][0]=='1')?(UCHAR)1:(UCHAR)0;       // flag set if error on slant column is to be written into output file
// QDOAS ???       pResults->SlntFact=atof(pList->itemText[COLUMN_CROSS_SVD_SLNTFACT]);                                // slant column factor
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???       pResults->StoreSlntCol=(pList->itemText[COLUMN_CROSS_SLNTCOL][0]=='1')?(UCHAR)1:(UCHAR)0;           // flag set if slant column is to be written into output file
// QDOAS ???       pResults->StoreSlntErr=(pList->itemText[COLUMN_CROSS_SLNTERR][0]=='1')?(UCHAR)1:(UCHAR)0;           // flag set if error on slant column is to be written into output file
// QDOAS ???       pResults->SlntFact=atof(pList->itemText[COLUMN_CROSS_SLNTFACT]);                                    // slant column factor
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     pResults->ResCol=atof(pList->itemText[COLUMN_CROSS_RESIDUAL]);                                      // residual column
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? //  =======================
// QDOAS ??? //  REGISTER DATA TO OUTPUT
// QDOAS ??? //  =======================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputRegister
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Register a field to output
// QDOAS ??? //
// QDOAS ??? // INPUT         titlePart1   1st part of the field name
// QDOAS ??? //               titlePart2   2nd part of the field name (optional)
// QDOAS ??? //               titlePart3   3rd part of the field name (optional)
// QDOAS ??? //               fieldType    the type of field to register (data type or format)
// QDOAS ??? //               fieldSize    the size of one item
// QDOAS ??? //               fieldDim1    1st dimension of the field
// QDOAS ??? //               fieldDim2    2nd dimension of the field
// QDOAS ??? //               format       the format string to use to output the data
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void OutputRegister(UCHAR *titlePart1,UCHAR *titlePart2,UCHAR *titlePart3,INT fieldType,INT fieldSize,
// QDOAS ???                     INT fieldDim1,INT fieldDim2,UCHAR *format)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR title[MAX_STR_LEN+1];
// QDOAS ???   PRJCT_RESULTS_FIELDS *pField;
// QDOAS ???
// QDOAS ???   if (outputNbFields<MAX_FIELDS)
// QDOAS ???    {
// QDOAS ???     pField=&outputFields[outputNbFields++];
// QDOAS ???
// QDOAS ???     sprintf(title,"%s%s%s",titlePart1,titlePart2,titlePart3);
// QDOAS ???
// QDOAS ???     strncpy(pField->fieldName,title,2*MAX_ITEM_NAME_LEN);
// QDOAS ???     strncpy(pField->fieldFormat,format,MAX_ITEM_NAME_LEN);
// QDOAS ???     pField->fieldType=fieldType;
// QDOAS ???     pField->fieldSize=fieldSize;
// QDOAS ???     pField->fieldDim1=fieldDim1;
// QDOAS ???     pField->fieldDim2=fieldDim2;
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputRegisterFluxes
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Register fluxes
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext   structure including information on project options
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void OutputRegisterFluxes(ENGINE_CONTEXT *pEngineContext)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR  columnTitle[MAX_ITEM_NAME_LEN+1],
// QDOAS ???         *ptrOld,*ptrNew;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   OUTPUT_NFluxes=OUTPUT_NCic=0;
// QDOAS ???
// QDOAS ???   // Fluxes
// QDOAS ???
// QDOAS ???   for (ptrOld=pEngineContext->project.asciiResults.fluxes;(ptrOld!=NULL) && (strlen(ptrOld)!=0);ptrOld=ptrNew)
// QDOAS ???    {
// QDOAS ???     if (OUTPUT_NFluxes>=MAX_FLUXES)
// QDOAS ???      break;
// QDOAS ???
// QDOAS ???     if (sscanf(ptrOld,"%lf",&OUTPUT_fluxes[OUTPUT_NFluxes])==1)
// QDOAS ???      {
// QDOAS ???       sprintf(columnTitle,"Fluxes %g",OUTPUT_fluxes[OUTPUT_NFluxes]);
// QDOAS ???       OutputRegister(columnTitle,"","",MEMORY_TYPE_DOUBLE,sizeof(double),ITEM_NONE,ITEM_NONE,"%#15.6le");
// QDOAS ???       OUTPUT_NFluxes++;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if ((ptrNew=strchr(ptrOld,';'))!=NULL)
// QDOAS ???      ptrNew++;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Color indexes
// QDOAS ???
// QDOAS ???   for (ptrOld=pEngineContext->project.asciiResults.cic;(ptrOld!=NULL) && (strlen(ptrOld)!=0);ptrOld=ptrNew)
// QDOAS ???    {
// QDOAS ???     if (OUTPUT_NCic>=MAX_CIC)
// QDOAS ???      break;
// QDOAS ???
// QDOAS ???     if (sscanf(ptrOld,"%lf/%lf",&OUTPUT_cic[OUTPUT_NCic][0],&OUTPUT_cic[OUTPUT_NCic][1])>=2)
// QDOAS ???      {
// QDOAS ???       sprintf(columnTitle,"%g/%g",OUTPUT_cic[OUTPUT_NCic][0],OUTPUT_cic[OUTPUT_NCic][1]);
// QDOAS ???       OutputRegister(columnTitle,"","",MEMORY_TYPE_DOUBLE,sizeof(double),ITEM_NONE,ITEM_NONE,"%#15.6le");
// QDOAS ???       OUTPUT_NCic++;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if ((ptrNew=strchr(ptrOld,';'))!=NULL)
// QDOAS ???      ptrNew++;
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputRegisterFields
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Register all the fields that are not parameters of the fit
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext   structure including information on project options
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void OutputRegisterFields(ENGINE_CONTEXT *pEngineContext)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   PRJCT_RESULTS_FIELDS *pField;
// QDOAS ???   PRJCT_RESULTS_ASCII *pResults;
// QDOAS ???   PROJECT             *pProject;                                                // pointer to project data
// QDOAS ???   INDEX                indexField,
// QDOAS ???                        j;
// QDOAS ???   int satelliteFlag;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pProject=(PROJECT *)&pEngineContext->project;
// QDOAS ???   pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;
// QDOAS ???
// QDOAS ???   satelliteFlag=((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
// QDOAS ???                  (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
// QDOAS ???                  (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
// QDOAS ???                  (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
// QDOAS ???                  (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2))?1:0;
// QDOAS ???
// QDOAS ???   // Browse fields
// QDOAS ???
// QDOAS ???   for (j=0;j<pResults->fieldsNumber;j++)
// QDOAS ???    {
// QDOAS ???     indexField=pResults->fieldsFlag[j]-1;
// QDOAS ???     pField=&PRJCT_resultsAscii[indexField];
// QDOAS ???
// QDOAS ???     // Fields to define for each spectral window
// QDOAS ???
// QDOAS ???     if (indexField==PRJCT_RESULTS_ASCII_CHI)
// QDOAS ???      OUTPUT_chiSquareFlag=1;
// QDOAS ???     else if (indexField==PRJCT_RESULTS_ASCII_RMS)
// QDOAS ???      OUTPUT_rmsFlag=1;
// QDOAS ???     else if (indexField==PRJCT_RESULTS_ASCII_REFZM)
// QDOAS ???      OUTPUT_refZmFlag=(outputRunCalib)?0:1;
// QDOAS ???     else if (indexField==PRJCT_RESULTS_ASCII_REFSHIFT)
// QDOAS ???      OUTPUT_refShift=(outputRunCalib)?0:1;
// QDOAS ???     else if (indexField==PRJCT_RESULTS_ASCII_COVAR)
// QDOAS ???      OUTPUT_covarFlag=(outputRunCalib)?0:1;
// QDOAS ???     else if (indexField==PRJCT_RESULTS_ASCII_CORR)
// QDOAS ???      OUTPUT_chiSquareFlag=(outputRunCalib)?0:1;
// QDOAS ???
// QDOAS ???     // Geolocation for satellite data
// QDOAS ???
// QDOAS ???     else if (satelliteFlag &&
// QDOAS ???            ((indexField==PRJCT_RESULTS_ASCII_LONGIT) ||
// QDOAS ???             (indexField==PRJCT_RESULTS_ASCII_LATIT)))
// QDOAS ???      {
// QDOAS ???       OutputRegister(pField->fieldName,"(1)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
// QDOAS ???       OutputRegister(pField->fieldName,"(2)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
// QDOAS ???       OutputRegister(pField->fieldName,"(3)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
// QDOAS ???       OutputRegister(pField->fieldName,"(4)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
// QDOAS ???
// QDOAS ???       OutputRegister(pField->fieldName,"(pixel center)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
// QDOAS ???      }
// QDOAS ???     else if (satelliteFlag &&
// QDOAS ???            ((indexField==PRJCT_RESULTS_ASCII_SZA) ||
// QDOAS ???             (indexField==PRJCT_RESULTS_ASCII_AZIM) ||
// QDOAS ???             (indexField==PRJCT_RESULTS_ASCII_LOS_ZA) ||
// QDOAS ???             (indexField==PRJCT_RESULTS_ASCII_LOS_AZIMUTH)))
// QDOAS ???      {
// QDOAS ???       OutputRegister(pField->fieldName,"(A)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
// QDOAS ???       OutputRegister(pField->fieldName,"(B)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
// QDOAS ???       OutputRegister(pField->fieldName,"(C)","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Other data
// QDOAS ???
// QDOAS ???     else
// QDOAS ???      OutputRegister(pField->fieldName,"","",pField->fieldType,pField->fieldSize,pField->fieldDim1,pField->fieldDim2,pField->fieldFormat);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputRegisterCalib
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Register all the calibration fields for each window of the
// QDOAS ??? //               wavelength calibration interval
// QDOAS ??? //
// QDOAS ??? // INPUT         indexFenoK   the current window of the wavelength calibration interval
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void OutputRegisterCalib(INDEX indexFenoK)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR                windowName[MAX_ITEM_NAME_LEN+1],                         // the name of the current spectral window
// QDOAS ???                        symbolName[MAX_ITEM_NAME_LEN+1];                         // the name of a symbol
// QDOAS ???   FENO                *pTabFeno;
// QDOAS ???   CROSS_REFERENCE     *TabCross;
// QDOAS ???   CROSS_RESULTS       *pTabCrossResults;
// QDOAS ???   INDEX                indexTabCross;
// QDOAS ???   INT                  nbWin,dim1,dim2;
// QDOAS ???
// QDOAS ???   nbWin=KURUCZ_buffers.Nb_Win;
// QDOAS ???
// QDOAS ???   dim1=nbWin;
// QDOAS ???   dim2=ITEM_NONE;
// QDOAS ???
// QDOAS ???   pTabFeno=&TabFeno[indexFenoK];
// QDOAS ???   TabCross=pTabFeno->TabCross;
// QDOAS ???
// QDOAS ???   // Register columns
// QDOAS ???
// QDOAS ???   sprintf(windowName,"Calib.");
// QDOAS ???
// QDOAS ???   OutputRegister(windowName,"RMS","",MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???
// QDOAS ???   // Fitted parameters
// QDOAS ???
// QDOAS ???   for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
// QDOAS ???    {
// QDOAS ???     pTabCrossResults=&pTabFeno->TabCrossResults[indexTabCross];
// QDOAS ???     sprintf(symbolName,"(%s)",WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName);
// QDOAS ???
// QDOAS ???     if (pTabCrossResults->StoreSlntCol)                         // Slant column
// QDOAS ???      OutputRegister(windowName,"SlCol",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???     if (pTabCrossResults->StoreSlntErr)                         // Error on slant column
// QDOAS ???      OutputRegister(windowName,"SlErr",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???
// QDOAS ???     if (pTabCrossResults->StoreShift)              // Shift
// QDOAS ???      {
// QDOAS ???       OutputRegister(windowName,"Shift",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???       if (pTabCrossResults->StoreError)
// QDOAS ???        OutputRegister(windowName,"Err Shift",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (pTabCrossResults->StoreStretch)            // Stretch
// QDOAS ???      {
// QDOAS ???       OutputRegister(windowName,"Stretch",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???       if (pTabCrossResults->StoreError)
// QDOAS ???        OutputRegister(windowName,"Err Stretch",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???       OutputRegister(windowName,"Stretch2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???       if (pTabCrossResults->StoreError)
// QDOAS ???        OutputRegister(windowName,"Err Stretch2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (pTabCrossResults->StoreScale)              // Scale
// QDOAS ???      {
// QDOAS ???       OutputRegister(windowName,"Scale",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???       if (pTabCrossResults->StoreError)
// QDOAS ???        OutputRegister(windowName,"Err Scale",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???       OutputRegister(windowName,"Scale2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???       if (pTabCrossResults->StoreError)
// QDOAS ???        OutputRegister(windowName,"Err Scale2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (pTabCrossResults->StoreParam)              // Param
// QDOAS ???      {
// QDOAS ???       OutputRegister(windowName,WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName,"",MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???       if (pTabCrossResults->StoreParamError)
// QDOAS ???        OutputRegister(windowName,"Err ",WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputRegisterParam
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Register all the parameters of the fit
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext   structure including information on project options
// QDOAS ??? //               hiddenFlag  indicates if the calling windows is hidden in the projects tree
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void OutputRegisterParam(ENGINE_CONTEXT *pEngineContext,INT hiddenFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR                windowName[MAX_ITEM_NAME_LEN+1],                         // the name of the current spectral window
// QDOAS ???                        symbolName[MAX_ITEM_NAME_LEN+1];                         // the name of a symbol
// QDOAS ???   FENO                *pTabFeno;
// QDOAS ???   CROSS_REFERENCE     *TabCross;
// QDOAS ???   CROSS_RESULTS       *pTabCrossResults;
// QDOAS ???   INDEX                indexFeno,indexWin,
// QDOAS ???                        indexTabCross,indexTabCross2;
// QDOAS ???   INT                  nbWin,dim1,dim2;
// QDOAS ???
// QDOAS ???   dim1=ITEM_NONE;
// QDOAS ???   dim2=ITEM_NONE;
// QDOAS ???
// QDOAS ???   nbWin=(hiddenFlag)?KURUCZ_buffers.Nb_Win:1;
// QDOAS ???
// QDOAS ???   // Browse analysis windows
// QDOAS ???
// QDOAS ???   for (indexWin=0;indexWin<nbWin;indexWin++)
// QDOAS ???
// QDOAS ???    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
// QDOAS ???     {
// QDOAS ???      pTabFeno=&TabFeno[indexFeno];
// QDOAS ???      TabCross=pTabFeno->TabCross;
// QDOAS ???
// QDOAS ???      if (pTabFeno->hidden==hiddenFlag)
// QDOAS ???       {
// QDOAS ???       	if (!pTabFeno->hidden)
// QDOAS ???       	 {
// QDOAS ???          // Not fitted parameters
// QDOAS ???
// QDOAS ???          sprintf(windowName,"%s.",pTabFeno->windowName);
// QDOAS ???
// QDOAS ???          if (OUTPUT_refZmFlag)
// QDOAS ???           OutputRegister(windowName,"RefZm","",MEMORY_TYPE_FLOAT,sizeof(float),dim1,dim2,"%#8.3f");
// QDOAS ???          if (OUTPUT_refShift)
// QDOAS ???           OutputRegister(windowName,"Ref2/Ref1 Shift","",MEMORY_TYPE_FLOAT,sizeof(float),dim1,dim2,"%#8.3f");
// QDOAS ???
// QDOAS ???          if (OUTPUT_covarFlag && (pTabFeno->svd.covar!=NULL))
// QDOAS ???           for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
// QDOAS ???            if (TabCross[indexTabCross].IndSvdA>0)
// QDOAS ???             for (indexTabCross2=0;indexTabCross2<indexTabCross;indexTabCross2++)
// QDOAS ???              if (TabCross[indexTabCross2].IndSvdA>0)
// QDOAS ???               {
// QDOAS ???                sprintf(symbolName,"(%s,%s)",
// QDOAS ???                       WorkSpace[TabCross[indexTabCross2].Comp].symbolName,
// QDOAS ???                       WorkSpace[TabCross[indexTabCross].Comp].symbolName);
// QDOAS ???                OutputRegister(windowName,"Covar",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???               }
// QDOAS ???
// QDOAS ???          if (OUTPUT_corrFlag && (pTabFeno->svd.covar!=NULL))
// QDOAS ???           for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
// QDOAS ???            if (TabCross[indexTabCross].IndSvdA>0)
// QDOAS ???             for (indexTabCross2=0;indexTabCross2<indexTabCross;indexTabCross2++)
// QDOAS ???              if (TabCross[indexTabCross2].IndSvdA>0)
// QDOAS ???               {
// QDOAS ???                sprintf(symbolName,"(%s,%s)",
// QDOAS ???                       WorkSpace[TabCross[indexTabCross2].Comp].symbolName,
// QDOAS ???                       WorkSpace[TabCross[indexTabCross].Comp].symbolName);
// QDOAS ???                OutputRegister(windowName,"Corr",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???               }
// QDOAS ???         }
// QDOAS ???        else if (outputRunCalib)
// QDOAS ???         sprintf(windowName,"RunCalib(%d).",indexWin+1);
// QDOAS ???
// QDOAS ???        if (OUTPUT_chiSquareFlag)
// QDOAS ???         OutputRegister(windowName,"Chi","",MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???        if (OUTPUT_rmsFlag)
// QDOAS ???         OutputRegister(windowName,"RMS","",MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???
// QDOAS ???        // Fitted parameters
// QDOAS ???
// QDOAS ???        for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
// QDOAS ???         {
// QDOAS ???          pTabCrossResults=&pTabFeno->TabCrossResults[indexTabCross];
// QDOAS ???          sprintf(symbolName,"(%s)",WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName);
// QDOAS ???
// QDOAS ???          if (pTabCrossResults->indexAmf!=ITEM_NONE)
// QDOAS ???           {
// QDOAS ???            if (pTabCrossResults->StoreAmf)              // AMF
// QDOAS ???             OutputRegister(windowName,"AMF",symbolName,MEMORY_TYPE_FLOAT,sizeof(double),dim1,dim2,"%#8.3lf");
// QDOAS ???            if (pTabCrossResults->StoreVrtCol)           // Vertical column
// QDOAS ???             OutputRegister(windowName,"VCol",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???            if (pTabCrossResults->StoreVrtErr)           // Error on vertical column
// QDOAS ???             OutputRegister(windowName,"VErr",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???           }
// QDOAS ???
// QDOAS ???          if (pTabCrossResults->StoreSlntCol)                         // Slant column
// QDOAS ???           OutputRegister(windowName,"SlCol",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???          if (pTabCrossResults->StoreSlntErr)                         // Error on slant column
// QDOAS ???           OutputRegister(windowName,"SlErr",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???
// QDOAS ???          if (pTabCrossResults->StoreShift)              // Shift
// QDOAS ???           {
// QDOAS ???            OutputRegister(windowName,"Shift",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???            if (pTabCrossResults->StoreError)
// QDOAS ???             OutputRegister(windowName,"Err Shift",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???           }
// QDOAS ???
// QDOAS ???          if (pTabCrossResults->StoreStretch)            // Stretch
// QDOAS ???           {
// QDOAS ???            OutputRegister(windowName,"Stretch",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???            if (pTabCrossResults->StoreError)
// QDOAS ???             OutputRegister(windowName,"Err Stretch",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???            OutputRegister(windowName,"Stretch2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???            if (pTabCrossResults->StoreError)
// QDOAS ???             OutputRegister(windowName,"Err Stretch2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???           }
// QDOAS ???
// QDOAS ???          if (pTabCrossResults->StoreScale)              // Scale
// QDOAS ???           {
// QDOAS ???            OutputRegister(windowName,"Scale",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???            if (pTabCrossResults->StoreError)
// QDOAS ???             OutputRegister(windowName,"Err Scale",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???            OutputRegister(windowName,"Scale2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???            if (pTabCrossResults->StoreError)
// QDOAS ???             OutputRegister(windowName,"Err Scale2",symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???           }
// QDOAS ???
// QDOAS ???          if (pTabCrossResults->StoreParam)              // Param
// QDOAS ???           {
// QDOAS ???            OutputRegister(windowName,WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName,"",MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???            if (pTabCrossResults->StoreParamError)
// QDOAS ???             OutputRegister(windowName,"Err ",WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName,MEMORY_TYPE_DOUBLE,sizeof(double),dim1,dim2,"%#12.4le");
// QDOAS ???           }
// QDOAS ???         }
// QDOAS ???       }
// QDOAS ???     }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputRegisterNasaAmes
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Register all the NASA-AMES fields
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext   structure including information on project options
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OutputRegisterNasaAmes(ENGINE_CONTEXT *pEngineContext)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   PRJCT_RESULTS_NASA *pResults;                                                 // NASA-AMES results part of project
// QDOAS ???   NASA_COMPONENTS *pNasaResults;                                                // data relative to components to take into account for NASA-AMES results
// QDOAS ???   INDEX indexField,indexTabCross;                                               // indexes for loops and arrays
// QDOAS ???   CROSS_REFERENCE *TabCross;                                                    // cross sections reference table
// QDOAS ???   UCHAR *windowName;                                                            // name of the current analysis window
// QDOAS ???   FENO *pTabFeno;                                                               // pointer to the current analysis window
// QDOAS ???   INDEX indexFeno;
// QDOAS ???   RC rc;                                                                        // return code
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pResults=&pEngineContext->project.nasaResults;
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Allocate buffers for data on analyzed records
// QDOAS ???
// QDOAS ???   if (((OUTPUT_nasaResults.Zm==NULL) && ((OUTPUT_nasaResults.Zm=(double *)MEMORY_AllocDVector("OutputRegisterNasaAmes","Zm",0,MAX_RESULTS))==NULL)) ||
// QDOAS ???       ((OUTPUT_nasaResults.Tm==NULL) && ((OUTPUT_nasaResults.Tm=(double *)MEMORY_AllocDVector("OutputRegisterNasaAmes","Tm",0,MAX_RESULTS))==NULL)) ||
// QDOAS ???       ((OUTPUT_nasaResults.TDet==NULL) && ((OUTPUT_nasaResults.TDet=(double *)MEMORY_AllocDVector("OutputRegisterNasaAmes","TDet",0,MAX_RESULTS))==NULL)) ||
// QDOAS ???       ((OUTPUT_nasaResults.TimeDec==NULL) && ((OUTPUT_nasaResults.TimeDec=(double *)MEMORY_AllocDVector("OutputRegisterNasaAmes","TimeDec",0,MAX_RESULTS))==NULL)) ||
// QDOAS ???       ((OUTPUT_nasaResults.Cic==NULL) && ((OUTPUT_nasaResults.Cic=(double *)MEMORY_AllocDVector("OutputRegisterNasaAmes","Cic",0,MAX_RESULTS))==NULL)))
// QDOAS ???
// QDOAS ???    rc=ERROR_ID_ALLOC;
// QDOAS ???
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     for (indexField=0;(indexField<PRJCT_RESULTS_NASA_MAX) && !rc;indexField++)
// QDOAS ???      {
// QDOAS ???       pNasaResults=&OUTPUT_nasaResults.components[indexField];
// QDOAS ???
// QDOAS ???       for (indexFeno=0;indexFeno<NFeno;indexFeno++)
// QDOAS ???        {
// QDOAS ???         pTabFeno=&TabFeno[indexFeno];
// QDOAS ???         TabCross=pTabFeno->TabCross;
// QDOAS ???
// QDOAS ???         windowName=pTabFeno->windowName;
// QDOAS ???
// QDOAS ???         if (!pTabFeno->hidden && !STD_Stricmp(pResults->fields[indexField],windowName))
// QDOAS ???
// QDOAS ???          for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
// QDOAS ???           if (!STD_Stricmp(WorkSpace[TabCross[indexTabCross].Comp].symbolName,pNasaResults->symbolName))
// QDOAS ???            {
// QDOAS ???             // Allocate buffers for results
// QDOAS ???
// QDOAS ???             if (((pNasaResults->SlntCol=(double *)MEMORY_AllocDVector("OutputRegisterNasaAmes","SlntCol",0,MAX_RESULTS))==NULL) ||
// QDOAS ???                 ((pNasaResults->Error=(double *)MEMORY_AllocDVector("OutputRegisterNasaAmes","Error",0,MAX_RESULTS))==NULL) ||
// QDOAS ???                (((indexField==PRJCT_RESULTS_NASA_NO2) || (indexField==PRJCT_RESULTS_NASA_O3)) &&
// QDOAS ???                  (pTabFeno->TabCrossResults[indexTabCross].indexAmf!=ITEM_NONE) &&
// QDOAS ???                 ((pNasaResults->Amf=(double *)MEMORY_AllocDVector("OutputRegisterNasaAmes","Amf",0,MAX_RESULTS))==NULL)))
// QDOAS ???
// QDOAS ???              rc=ERROR_ID_ALLOC;
// QDOAS ???
// QDOAS ???             // Set indexes of selected analysis window and cross section
// QDOAS ???
// QDOAS ???             else
// QDOAS ???              {
// QDOAS ???               pNasaResults->indexWindow=indexFeno;
// QDOAS ???               pNasaResults->indexTabCross=indexTabCross;
// QDOAS ???               pNasaResults->ResCol=
// QDOAS ???
// QDOAS ???                ((TabFeno[indexFeno].refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) ||
// QDOAS ???                 (indexField==PRJCT_RESULTS_NASA_OCLO) || (indexField==PRJCT_RESULTS_NASA_BRO)) ?
// QDOAS ???                 (double)0.:TabFeno[indexFeno].TabCrossResults[indexTabCross].ResCol;
// QDOAS ???
// QDOAS ???               break;
// QDOAS ???              }
// QDOAS ???            }
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OUTPUT_RegisterData
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Register all the data to output
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext   structure including information on project options
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OUTPUT_RegisterData(ENGINE_CONTEXT *pEngineContext)
// QDOAS ???  {
// QDOAS ???  	// Declarations
// QDOAS ???
// QDOAS ???   INDEX indexFeno,indexFenoK,indexFeno1;
// QDOAS ???   PROJECT *pProject;
// QDOAS ???   PRJCT_RESULTS_ASCII *pResults;
// QDOAS ???  	RC rc;
// QDOAS ???
// QDOAS ???   #if defined(__DEBUG_) && __DEBUG_
// QDOAS ???   DEBUG_FunctionBegin("OUTPUT_RegisterData",DEBUG_FCTTYPE_FILE);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???  	// Initializations
// QDOAS ???
// QDOAS ???   pProject=&pEngineContext->project;
// QDOAS ???   pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;
// QDOAS ???   outputCalibFlag=outputRunCalib=0;
// QDOAS ???   indexFenoK=indexFeno1=ITEM_NONE;
// QDOAS ???  	rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   if (pProject->asciiResults.analysisFlag || pProject->asciiResults.calibFlag)
// QDOAS ???    {
// QDOAS ???     if (THRD_id==THREAD_TYPE_ANALYSIS)
// QDOAS ???      {
// QDOAS ???       // Satellites measurements and automatic reference selection : save information on the selected reference
// QDOAS ???
// QDOAS ???    	  if (((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
// QDOAS ???            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
// QDOAS ???            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
// QDOAS ???            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
// QDOAS ???            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&
// QDOAS ???             ANALYSE_refSelectionFlag)                                           // automatic reference is requested for at least one analysis window
// QDOAS ???        {
// QDOAS ???    	    OutputRegister("Reference file","","",MEMORY_TYPE_STRING,MAX_PATH_LEN+1,1,1,"%s");
// QDOAS ???    	    OutputRegister("Number of records selected for the reference","","",MEMORY_TYPE_INT,sizeof(int),1,1,"%d");
// QDOAS ???    	   }
// QDOAS ???
// QDOAS ???      	// Save information on the calibration
// QDOAS ???
// QDOAS ???      	if (pResults->calibFlag)
// QDOAS ???      	 {
// QDOAS ???         for (indexFeno=0;indexFeno<NFeno;indexFeno++)
// QDOAS ???          if (TabFeno[indexFeno].hidden)
// QDOAS ???           indexFenoK=indexFeno;
// QDOAS ???          else if (!TabFeno[indexFeno].hidden &&
// QDOAS ???                  ((TabFeno[indexFeno].useKurucz==ANLYS_KURUCZ_REF) ||
// QDOAS ???                   (TabFeno[indexFeno].useKurucz==ANLYS_KURUCZ_REF_AND_SPEC)) &&
// QDOAS ???                   !TabFeno[indexFeno].rcKurucz)
// QDOAS ???           {
// QDOAS ???           	if (indexFeno1==ITEM_NONE)
// QDOAS ???           	 indexFeno1=indexFeno;
// QDOAS ???            outputCalibFlag++;
// QDOAS ???           }
// QDOAS ???
// QDOAS ???         if (indexFenoK==ITEM_NONE)
// QDOAS ???          outputCalibFlag=0;
// QDOAS ???
// QDOAS ???         if (outputCalibFlag)
// QDOAS ???          OutputRegisterCalib(indexFenoK);
// QDOAS ???        }
// QDOAS ???    	 }
// QDOAS ???
// QDOAS ???     // Run calibration on measurement spectra
// QDOAS ???
// QDOAS ???     else if (THRD_id==THREAD_TYPE_KURUCZ)
// QDOAS ???      outputRunCalib++;
// QDOAS ???
// QDOAS ???     OutputRegisterFields(pEngineContext);
// QDOAS ???     OutputRegisterParam(pEngineContext,(THRD_id==THREAD_TYPE_ANALYSIS)?0:1);
// QDOAS ???     OutputRegisterFluxes(pEngineContext);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if (pProject->nasaResults.nasaFlag)
// QDOAS ???    rc=OutputRegisterNasaAmes(pEngineContext);
// QDOAS ???
// QDOAS ???   #if defined(__DEBUG_) && __DEBUG_
// QDOAS ???   DEBUG_FunctionStop("OUTPUT_RegisterData",rc);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ===================
// QDOAS ??? // KEEP DATA TO OUTPUT
// QDOAS ??? // ===================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputCalib
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Save data related to the wavelength calibration
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext   structure including information on project options
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void OutputCalib(ENGINE_CONTEXT *pEngineContext)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   FENO                *pTabFeno;
// QDOAS ???   CROSS_REFERENCE     *TabCross;
// QDOAS ???   CROSS_RESULTS       *pTabCrossResults;
// QDOAS ???   INDEX                indexFeno,indexFenoK,indexColumn,indexTabCross,indexWin;
// QDOAS ???   INT                  nbWin;
// QDOAS ???   double               defaultValue;
// QDOAS ???
// QDOAS ???   PRJCT_RESULTS_ASCII *pResults;
// QDOAS ???   PROJECT             *pProject;                                                // pointer to project data
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   defaultValue=(double)9999.;
// QDOAS ???
// QDOAS ???   pProject=(PROJECT *)&pEngineContext->project;
// QDOAS ???   pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;
// QDOAS ???
// QDOAS ???   nbWin=KURUCZ_buffers.Nb_Win;
// QDOAS ???   indexFenoK=ITEM_NONE;
// QDOAS ???
// QDOAS ???   for (indexFeno=0;(indexFeno<NFeno) && (indexFenoK==ITEM_NONE);indexFeno++)
// QDOAS ???    if (TabFeno[indexFeno].hidden==1)
// QDOAS ???     indexFenoK=indexFeno;
// QDOAS ???
// QDOAS ???   if (outputCalibFlag && (indexFeno<NFeno))
// QDOAS ???    {
// QDOAS ???     pTabFeno=&TabFeno[indexFenoK];
// QDOAS ???     TabCross=pTabFeno->TabCross;
// QDOAS ???
// QDOAS ???     for (indexWin=0;indexWin<nbWin;indexWin++)
// QDOAS ???      {
// QDOAS ???      	// Bypass predefined data (e.g. reference spectrum record)
// QDOAS ???
// QDOAS ???       for (indexColumn=0;(outputFields[indexColumn].fieldDim2!=ITEM_NONE);indexColumn++);
// QDOAS ???
// QDOAS ???       // RMS
// QDOAS ???
// QDOAS ???       ((double *)outputColumns[indexColumn++])[indexWin]=KURUCZ_buffers.KuruczFeno[indexFeno].rms[indexWin];
// QDOAS ???
// QDOAS ???       // Fitted parameters
// QDOAS ???
// QDOAS ???       for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
// QDOAS ???        {
// QDOAS ???         pTabCrossResults=&pTabFeno->TabCrossResults[indexTabCross];
// QDOAS ???
// QDOAS ???         pTabCrossResults=&KURUCZ_buffers.KuruczFeno[indexFeno].results[indexWin][indexTabCross];
// QDOAS ???
// QDOAS ???         if (pTabCrossResults->StoreSlntCol)            // Slant column
// QDOAS ???          	((double *)outputColumns[indexColumn++])[indexWin]=
// QDOAS ???          	(!pTabFeno->rc && (pTabCrossResults->SlntFact!=(double)0.))?
// QDOAS ???          	 (double)pTabCrossResults->SlntCol/pTabCrossResults->SlntFact:(double)defaultValue;
// QDOAS ???
// QDOAS ???         if (pTabCrossResults->StoreSlntErr)            // Error on slant column
// QDOAS ???          	((double *)outputColumns[indexColumn++])[indexWin]=
// QDOAS ???          	(!pTabFeno->rc && (pTabCrossResults->SlntFact!=(double)0.))?
// QDOAS ???          	 (double)pTabCrossResults->SlntErr/pTabCrossResults->SlntFact:(double)defaultValue;
// QDOAS ???
// QDOAS ???         if (pTabCrossResults->StoreShift)              // Shift
// QDOAS ???          {
// QDOAS ???           ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->Shift:(double)defaultValue;
// QDOAS ???           if (pTabCrossResults->StoreError)
// QDOAS ???            ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaShift:(double)defaultValue;
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         if (pTabCrossResults->StoreStretch)            // Stretch
// QDOAS ???          {
// QDOAS ???           ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->Stretch:(double)defaultValue;
// QDOAS ???           if (pTabCrossResults->StoreError)
// QDOAS ???            ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaStretch:(double)defaultValue;
// QDOAS ???           ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->Stretch2:(double)defaultValue;
// QDOAS ???           if (pTabCrossResults->StoreError)
// QDOAS ???            ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaStretch2:(double)defaultValue;
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         if (pTabCrossResults->StoreScale)              // Scale
// QDOAS ???          {
// QDOAS ???           ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->Scale:(double)defaultValue;
// QDOAS ???           if (pTabCrossResults->StoreError)
// QDOAS ???            ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaScale:(double)defaultValue;
// QDOAS ???           ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->Scale2:(double)defaultValue;
// QDOAS ???           if (pTabCrossResults->StoreError)
// QDOAS ???            ((double *)outputColumns[indexColumn++])[indexWin]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaScale2:(double)defaultValue;
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         if (pTabCrossResults->StoreParam)              // Param
// QDOAS ???          {
// QDOAS ???           ((double *)outputColumns[indexColumn++])[indexWin]=
// QDOAS ???            (double)((!pTabFeno->rc)?
// QDOAS ???                    ((TabCross[indexTabCross].IndSvdA)?pTabCrossResults->SlntCol:pTabCrossResults->Param):defaultValue);
// QDOAS ???           if (pTabCrossResults->StoreParamError)
// QDOAS ???           ((double *)outputColumns[indexColumn++])[indexWin]=
// QDOAS ???            (double)((!pTabFeno->rc)?
// QDOAS ???                    ((TabCross[indexTabCross].IndSvdA)?pTabCrossResults->SlntErr:pTabCrossResults->SigmaParam):(double)defaultValue);
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputSaveRecord
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Save all the data on the current record (including fitted parameters)
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext   structure including information on project options
// QDOAS ??? //               hiddenFlag  0 to save analysis results, 1 to save calibration results
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void OutputSaveRecord(ENGINE_CONTEXT *pEngineContext,INT hiddenFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
// QDOAS ???   PRJCT_RESULTS_FIELDS *pField;
// QDOAS ???
// QDOAS ???   FENO                *pTabFeno;
// QDOAS ???   CROSS_REFERENCE     *TabCross;
// QDOAS ???   CROSS_RESULTS       *pTabCrossResults;
// QDOAS ???   PROJECT             *pProject;                                                // pointer to project data
// QDOAS ???   PRJCT_RESULTS_ASCII *pResults;                                                // pointer to results part of project
// QDOAS ???   INDEX                indexRecord,
// QDOAS ???                        indexColumn,
// QDOAS ???                        indexFeno,
// QDOAS ???                        indexTabCross,indexTabCross2,
// QDOAS ???                        indexField,                                              // browse fields of record
// QDOAS ???                        indexFluxes,                                             // browse selected wavelengths for fluxes
// QDOAS ???                        indexCic,                                                // browse selected wavelengths for color indexes
// QDOAS ???                        i,k;
// QDOAS ???   double               flux;                                                    // temporary variable
// QDOAS ???   double               defaultValue;
// QDOAS ???   INT                  nbWin,indexWin;
// QDOAS ???
// QDOAS ???   nbWin=(hiddenFlag)?KURUCZ_buffers.Nb_Win:1;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pProject=(PROJECT *)&pEngineContext->project;
// QDOAS ???   pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;
// QDOAS ???
// QDOAS ???   if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
// QDOAS ???    pOrbitFile=&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];
// QDOAS ???
// QDOAS ???   if (pResults->analysisFlag)
// QDOAS ???    {
// QDOAS ???    	defaultValue=(double)9999.;
// QDOAS ???
// QDOAS ???     for (indexColumn=0;(outputFields[indexColumn].fieldDim1!=ITEM_NONE);indexColumn++);
// QDOAS ???
// QDOAS ???     indexRecord=outputNbRecords++;
// QDOAS ???
// QDOAS ???     // ---------------------------------
// QDOAS ???     // INFORMATION ON THE CURRENT RECORD
// QDOAS ???     // ---------------------------------
// QDOAS ???
// QDOAS ???     for (indexField=0,k=indexColumn;indexField<pResults->fieldsNumber;indexField++,k++)
// QDOAS ???      {
// QDOAS ???       pField=&outputFields[k];
// QDOAS ???
// QDOAS ???       switch(pResults->fieldsFlag[indexField]-1)
// QDOAS ???        {
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_SPECNO :
// QDOAS ???          ((USHORT *)outputColumns[indexColumn++])[indexRecord]=(USHORT)pEngineContext->indexRecord;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_NAME :
// QDOAS ???          strncpy(&outputColumns[indexColumn++][indexRecord*pField->fieldSize],pEngineContext->Nom,pField->fieldSize);
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_DATE :
// QDOAS ???
// QDOAS ???          sprintf(&outputColumns[indexColumn++][indexRecord*pField->fieldSize],"%02d/%02d/%d",
// QDOAS ???                 (INT) pEngineContext->present_day.da_day,
// QDOAS ???                 (INT) pEngineContext->present_day.da_mon,
// QDOAS ???                 (INT) pEngineContext->present_day.da_year);
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_TIME :
// QDOAS ???
// QDOAS ???          sprintf(&outputColumns[indexColumn++][indexRecord*pField->fieldSize],"%02d:%02d:%02d",
// QDOAS ???                 (INT) pEngineContext->present_time.ti_hour,
// QDOAS ???                 (INT) pEngineContext->present_time.ti_min,
// QDOAS ???                 (INT) pEngineContext->present_time.ti_sec);
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_YEAR :
// QDOAS ???          ((USHORT *)outputColumns[indexColumn++])[indexRecord]=(USHORT)pEngineContext->present_day.da_year;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_DATE_TIME :
// QDOAS ???
// QDOAS ???          memset(&outputColumns[indexColumn][indexRecord*pField->fieldSize],0,pField->fieldSize);
// QDOAS ???
// QDOAS ???          sprintf(&outputColumns[indexColumn][indexRecord*pField->fieldSize],"%4d%02d%02d%02d%02d%02d",
// QDOAS ???                 (INT) pEngineContext->present_day.da_year,
// QDOAS ???                 (INT) pEngineContext->present_day.da_mon,
// QDOAS ???                 (INT) pEngineContext->present_day.da_day,
// QDOAS ???                 (INT) pEngineContext->present_time.ti_hour,
// QDOAS ???                 (INT) pEngineContext->present_time.ti_min,
// QDOAS ???                 (INT) pEngineContext->present_time.ti_sec);
// QDOAS ???
// QDOAS ???          if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
// QDOAS ???           sprintf(&outputColumns[indexColumn][indexRecord*pField->fieldSize+14],".%03d",(INT)SCIA_ms);
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
// QDOAS ???           sprintf(&outputColumns[indexColumn][indexRecord*pField->fieldSize+14],".%06d",(INT)GOME2_ms);
// QDOAS ???
// QDOAS ???          indexColumn++;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_MFC_STARTTIME :
// QDOAS ???
// QDOAS ???          memset(&outputColumns[indexColumn][indexRecord*pField->fieldSize],0,pField->fieldSize);
// QDOAS ???
// QDOAS ???          sprintf(&outputColumns[indexColumn][indexRecord*pField->fieldSize],"%02d%02d%02d",
// QDOAS ???                 (INT) pEngineContext->startTime.ti_hour,
// QDOAS ???                 (INT) pEngineContext->startTime.ti_min,
// QDOAS ???                 (INT) pEngineContext->startTime.ti_sec);
// QDOAS ???
// QDOAS ???          indexColumn++;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_MFC_ENDTIME :
// QDOAS ???
// QDOAS ???          memset(&outputColumns[indexColumn][indexRecord*pField->fieldSize],0,pField->fieldSize);
// QDOAS ???
// QDOAS ???          sprintf(&outputColumns[indexColumn][indexRecord*pField->fieldSize],"%02d%02d%02d",
// QDOAS ???                 (INT) pEngineContext->endTime.ti_hour,
// QDOAS ???                 (INT) pEngineContext->endTime.ti_min,
// QDOAS ???                 (INT) pEngineContext->endTime.ti_sec);
// QDOAS ???
// QDOAS ???          indexColumn++;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_JULIAN :
// QDOAS ???          ((USHORT *)outputColumns[indexColumn++])[indexRecord]=(USHORT)ZEN_FNCaljda(&pEngineContext->Tm);
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_JDFRAC :
// QDOAS ???          ((double *)outputColumns[indexColumn++])[indexRecord]=(double)ZEN_FNCaljda(&pEngineContext->Tm)+ZEN_FNCaldti(&pEngineContext->Tm)/24.;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_TIFRAC :
// QDOAS ???
// QDOAS ???          if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
// QDOAS ???              (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2))
// QDOAS ???           ((double *)outputColumns[indexColumn++])[indexRecord]=(double)pEngineContext->TimeDec;
// QDOAS ???          else
// QDOAS ???           ((double *)outputColumns[indexColumn++])[indexRecord]=(double)ZEN_FNCaldti(&pEngineContext->Tm);
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_SCANS :
// QDOAS ???          ((USHORT *)outputColumns[indexColumn++])[indexRecord]=(USHORT)pEngineContext->NSomme;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_TINT :
// QDOAS ???          ((double *)outputColumns[indexColumn++])[indexRecord]=(double)pEngineContext->Tint;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_SZA :
// QDOAS ???
// QDOAS ???          if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) || (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS))
// QDOAS ???           {
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.solZen[0];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.solZen[1];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.solZen[2];
// QDOAS ???           }
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
// QDOAS ???           {
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.solZen[0];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.solZen[1];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.solZen[2];
// QDOAS ???           }
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
// QDOAS ???           {
// QDOAS ???            if (pOrbitFile->gdpBinHeader.version<2)
// QDOAS ???             {
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo1.szaArray[0];k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo1.szaArray[1];k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo1.szaArray[2];
// QDOAS ???             }
// QDOAS ???            else if (pOrbitFile->gdpBinHeader.version==2)
// QDOAS ???             {
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo2.szaArray[0];k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo2.szaArray[1];k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo2.szaArray[2];
// QDOAS ???             }
// QDOAS ???            else if (pOrbitFile->gdpBinHeader.version>=3)
// QDOAS ???             {
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo3.szaArray[0];k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo3.szaArray[1];k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo3.szaArray[2];
// QDOAS ???             }
// QDOAS ???           }
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII)
// QDOAS ???           {
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome.sza[0];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome.sza[1];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome.sza[2];
// QDOAS ???           }
// QDOAS ???          else
// QDOAS ???           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->Zm;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_AZIM :
// QDOAS ???
// QDOAS ???          if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
// QDOAS ???              (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS))
// QDOAS ???           {
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.solAzi[0];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.solAzi[1];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.solAzi[2];
// QDOAS ???           }
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
// QDOAS ???           {
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.solAzi[0];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.solAzi[1];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.solAzi[2];
// QDOAS ???           }
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII)
// QDOAS ???           {
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome.azim[0];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome.azim[1];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome.azim[2];
// QDOAS ???           }
// QDOAS ???          else if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) && (pOrbitFile->gdpBinHeader.version<5))
// QDOAS ???           {
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->Azimuth;k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->Azimuth;k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->Azimuth;
// QDOAS ???           }
// QDOAS ???          else if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) && (pOrbitFile->gdpBinHeader.version>=5))
// QDOAS ???           {
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.aziArray[0];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.aziArray[1];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.aziArray[2];
// QDOAS ???           }
// QDOAS ???          else
// QDOAS ???           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->Azimuth;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_TDET :
// QDOAS ???          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->TDet;
// QDOAS ???         break;
// QDOAS ???      // ---------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_SKY :
// QDOAS ???          ((USHORT *)outputColumns[indexColumn++])[indexRecord]=(USHORT)pEngineContext->SkyObs;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_BESTSHIFT :
// QDOAS ???          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->BestShift;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_PIXEL :
// QDOAS ???          ((USHORT *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???          ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) || (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))?(USHORT)pEngineContext->gome.pixelNumber:0;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_PIXEL_TYPE :
// QDOAS ???          ((USHORT *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???          ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) || (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))?(USHORT)pEngineContext->gome.pixelType:0;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_ORBIT :
// QDOAS ???
// QDOAS ???          if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) || (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))
// QDOAS ???           ((INT *)outputColumns[indexColumn++])[indexRecord]=(INT)pEngineContext->gome.orbitNumber+1;
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
// QDOAS ???           ((INT *)outputColumns[indexColumn++])[indexRecord]=pEngineContext->scia.orbitNumber;
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
// QDOAS ???           ((INT *)outputColumns[indexColumn++])[indexRecord]=pEngineContext->gome2.orbitNumber;
// QDOAS ???
// QDOAS ???
// QDOAS ???          ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) || (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))?
// QDOAS ???           (INT)pEngineContext->gome.orbitNumber+1:pEngineContext->scia.orbitNumber;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_CLOUD :
// QDOAS ???
// QDOAS ???          if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
// QDOAS ???           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.cloudFraction*0.01;
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
// QDOAS ???           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.cloudFraction;
// QDOAS ???          else
// QDOAS ???           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)defaultValue;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_O3 :
// QDOAS ???
// QDOAS ???          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)
// QDOAS ???          ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)?
// QDOAS ???            pOrbitFile->gdpBinSpectrum.o3*0.01:defaultValue);
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_NO2 :
// QDOAS ???
// QDOAS ???          ((double *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)?
// QDOAS ???           (double)pOrbitFile->gdpBinSpectrum.no2*1e13:(double)defaultValue;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_CLOUDTOPP :
// QDOAS ???
// QDOAS ???          if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
// QDOAS ???           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.cloudTopPressure;
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
// QDOAS ???           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.cloudTopPressure;
// QDOAS ???          else
// QDOAS ???           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)defaultValue;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_COEFF :
// QDOAS ???
// QDOAS ???          ((USHORT *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)?
// QDOAS ???           (USHORT)pOrbitFile->gdpBinSpectrum.indexSpectralParam:(USHORT)defaultValue;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_SCIA_STATE_INDEX :
// QDOAS ???
// QDOAS ???          ((USHORT *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)?
// QDOAS ???           (USHORT)pEngineContext->scia.stateIndex:(USHORT)defaultValue;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_SCIA_STATE_ID :
// QDOAS ???
// QDOAS ???          ((USHORT *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)?
// QDOAS ???           (USHORT)pEngineContext->scia.stateId:(USHORT)defaultValue;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_SCIA_QUALITY :
// QDOAS ???
// QDOAS ???          ((USHORT *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)?
// QDOAS ???           (USHORT)pEngineContext->scia.qualityFlag:(USHORT)defaultValue;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_SAT_HEIGHT :
// QDOAS ???
// QDOAS ???          if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) && (pOrbitFile->gdpBinHeader.version>=3))
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo3.satHeight;
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
// QDOAS ???           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.satHeight;
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
// QDOAS ???           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.satHeight;
// QDOAS ???          else
// QDOAS ???           indexColumn++;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_EARTH_RADIUS :
// QDOAS ???
// QDOAS ???          if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) && (pOrbitFile->gdpBinHeader.version>=3))
// QDOAS ???           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo3.radiusCurve;
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
// QDOAS ???           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.earthRadius;
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
// QDOAS ???           ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.earthRadius;
// QDOAS ???          else
// QDOAS ???           indexColumn++;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_VIEW_ELEVATION :
// QDOAS ???          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->elevationViewAngle;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_VIEW_AZIMUTH :
// QDOAS ???          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->azimuthViewAngle;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_LOS_ZA :
// QDOAS ???
// QDOAS ???          if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) || (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS))
// QDOAS ???           {
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.losZen[0];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.losZen[1];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.losZen[2];
// QDOAS ???           }
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
// QDOAS ???           {
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.losZen[0];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.losZen[1];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.losZen[2];
// QDOAS ???           }
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
// QDOAS ???           {
// QDOAS ???            if (pOrbitFile->gdpBinHeader.version==2)
// QDOAS ???             {
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)180.-pOrbitFile->gdpBinSpectrum.geo.geo2.losZa;k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)180.-pOrbitFile->gdpBinSpectrum.geo.geo2.losZa;k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)180.-pOrbitFile->gdpBinSpectrum.geo.geo2.losZa;
// QDOAS ???             }
// QDOAS ???            else if (pOrbitFile->gdpBinHeader.version>=3)
// QDOAS ???             {
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)(pOrbitFile->gdpBinSpectrum.geo.geo3.losZa[0]*0.01);k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)(pOrbitFile->gdpBinSpectrum.geo.geo3.losZa[1]*0.01);k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)(pOrbitFile->gdpBinSpectrum.geo.geo3.losZa[2]*0.01);
// QDOAS ???             }
// QDOAS ???            else
// QDOAS ???             indexColumn+=3;
// QDOAS ???           }
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII)
// QDOAS ???           indexColumn+=3;
// QDOAS ???          else
// QDOAS ???           indexColumn++;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_LOS_AZIMUTH :
// QDOAS ???
// QDOAS ???          if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
// QDOAS ???           {
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.losAzi[0];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.losAzi[1];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.losAzi[2];
// QDOAS ???           }
// QDOAS ???          else if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
// QDOAS ???                   (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS))
// QDOAS ???           {
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.losAzi[0];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.losAzi[1];k++;
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.losAzi[2];
// QDOAS ???           }
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
// QDOAS ???           {
// QDOAS ???            if (pOrbitFile->gdpBinHeader.version==2)
// QDOAS ???             {
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo2.losAzim;k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo2.losAzim;k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo2.losAzim;
// QDOAS ???             }
// QDOAS ???            else if (pOrbitFile->gdpBinHeader.version>=3)
// QDOAS ???             {
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)(pOrbitFile->gdpBinSpectrum.geo.geo3.losAzim[0]*0.01);k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)(pOrbitFile->gdpBinSpectrum.geo.geo3.losAzim[1]*0.01);k++;
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)(pOrbitFile->gdpBinSpectrum.geo.geo3.losAzim[2]*0.01);
// QDOAS ???             }
// QDOAS ???            else
// QDOAS ???             indexColumn+=3;
// QDOAS ???           }
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII)
// QDOAS ???           indexColumn+=3;
// QDOAS ???          else
// QDOAS ???           indexColumn++;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_LONGIT :
// QDOAS ???
// QDOAS ???          if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
// QDOAS ???              (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS))
// QDOAS ???           for (i=0;i<4;i++,k++)
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.longitudes[i];
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
// QDOAS ???           for (i=0;i<4;i++,k++)
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.longitudes[i];
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
// QDOAS ???           {
// QDOAS ???            if (pOrbitFile->gdpBinHeader.version<2)
// QDOAS ???             for (i=0;i<4;i++,k++)
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo1.lonArray[i];
// QDOAS ???            else if (pOrbitFile->gdpBinHeader.version==2)
// QDOAS ???             for (i=0;i<4;i++,k++)
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)0.01*pOrbitFile->gdpBinSpectrum.geo.geo2.lonArray[i];
// QDOAS ???            else if (pOrbitFile->gdpBinHeader.version>=3)
// QDOAS ???             for (i=0;i<4;i++,k++)
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.lonArray[i];
// QDOAS ???           }
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII)
// QDOAS ???           for (i=0;i<4;i++,k++)
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome.longit[i];
// QDOAS ???
// QDOAS ???          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->longitude;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_LATIT :
// QDOAS ???
// QDOAS ???          if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
// QDOAS ???              (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS))
// QDOAS ???           for (i=0;i<4;i++,k++)
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->scia.latitudes[i];
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
// QDOAS ???           for (i=0;i<4;i++,k++)
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome2.latitudes[i];
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
// QDOAS ???           {
// QDOAS ???            if (pOrbitFile->gdpBinHeader.version<2)
// QDOAS ???             for (i=0;i<4;i++,k++)
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pOrbitFile->gdpBinSpectrum.geo.geo1.latArray[i];
// QDOAS ???            else if (pOrbitFile->gdpBinHeader.version==2)
// QDOAS ???             for (i=0;i<4;i++,k++)
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)0.01*pOrbitFile->gdpBinSpectrum.geo.geo2.latArray[i];
// QDOAS ???            else if (pOrbitFile->gdpBinHeader.version>=3)
// QDOAS ???             for (i=0;i<4;i++,k++)
// QDOAS ???              ((float *)outputColumns[indexColumn++])[indexRecord]=(float)0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.latArray[i];
// QDOAS ???           }
// QDOAS ???          else if (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII)
// QDOAS ???           for (i=0;i<4;i++,k++)
// QDOAS ???            ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->gome.latit[i];
// QDOAS ???
// QDOAS ???          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->latitude;
// QDOAS ???
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case PRJCT_RESULTS_ASCII_ALTIT :
// QDOAS ???          ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pEngineContext->altitude;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         default :
// QDOAS ???          k--;
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // ----------------
// QDOAS ???     // ANALYSIS RESULTS
// QDOAS ???     // ----------------
// QDOAS ???
// QDOAS ???     for (indexWin=0;indexWin<nbWin;indexWin++)
// QDOAS ???
// QDOAS ???      for (indexFeno=0;indexFeno<NFeno;indexFeno++)
// QDOAS ???       {
// QDOAS ???       	pTabFeno=(!hiddenFlag)?&TabFeno[indexFeno]:&TabFeno[KURUCZ_buffers.indexKurucz];
// QDOAS ???       	TabCross=pTabFeno->TabCross;
// QDOAS ???
// QDOAS ???       	if (pTabFeno->hidden==hiddenFlag)
// QDOAS ???       	 {
// QDOAS ???       	  if (!pTabFeno->hidden)
// QDOAS ???       	   {
// QDOAS ???       	   	if (OUTPUT_refZmFlag)
// QDOAS ???       	   	 ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pTabFeno->Zm;
// QDOAS ???       	   	if (OUTPUT_refShift)
// QDOAS ???       	   	 ((float *)outputColumns[indexColumn++])[indexRecord]=(float)pTabFeno->Shift;
// QDOAS ???
// QDOAS ???            if (OUTPUT_covarFlag && (pTabFeno->svd.covar!=NULL))
// QDOAS ???             for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
// QDOAS ???              if (TabCross[indexTabCross].IndSvdA>0)
// QDOAS ???               for (indexTabCross2=0;indexTabCross2<indexTabCross;indexTabCross2++)
// QDOAS ???                if (TabCross[indexTabCross2].IndSvdA>0)
// QDOAS ???                 ((double *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???                 (!pTabFeno->rc && (TabCross[indexTabCross].Fact!=(double)0.))?
// QDOAS ???                  (double)pTabFeno->svd.covar[TabCross[indexTabCross2].IndSvdA][TabCross[indexTabCross].IndSvdA]*pTabFeno->chiSquare/
// QDOAS ???                  (TabCross[indexTabCross].Fact*TabCross[indexTabCross2].Fact):(double)defaultValue;
// QDOAS ???
// QDOAS ???            if (OUTPUT_corrFlag && (pTabFeno->svd.covar!=NULL))
// QDOAS ???             for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
// QDOAS ???              if (TabCross[indexTabCross].IndSvdA>0)
// QDOAS ???               for (indexTabCross2=0;indexTabCross2<indexTabCross;indexTabCross2++)
// QDOAS ???                if (TabCross[indexTabCross2].IndSvdA>0)
// QDOAS ???                 ((double *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???                 (!pTabFeno->rc && (TabCross[indexTabCross].Fact!=(double)0.))?
// QDOAS ???                  (double)pTabFeno->svd.covar[TabCross[indexTabCross2].IndSvdA][TabCross[indexTabCross].IndSvdA]*pTabFeno->chiSquare/
// QDOAS ???                  (TabCross[indexTabCross].Fact*TabCross[indexTabCross2].Fact*pTabFeno->TabCrossResults[indexTabCross].SlntErr*pTabFeno->TabCrossResults[indexTabCross2].SlntErr):
// QDOAS ???                  (double)defaultValue;
// QDOAS ???           }
// QDOAS ???
// QDOAS ???          if (OUTPUT_chiSquareFlag)
// QDOAS ???           ((double *)outputColumns[indexColumn++])[indexRecord]=(!hiddenFlag)?(double)pTabFeno->chiSquare:KURUCZ_buffers.KuruczFeno[indexFeno].chiSquare[indexWin];
// QDOAS ???          if (OUTPUT_rmsFlag)
// QDOAS ???           ((double *)outputColumns[indexColumn++])[indexRecord]=(!hiddenFlag)?(double)pTabFeno->RMS:KURUCZ_buffers.KuruczFeno[indexFeno].rms[indexWin];
// QDOAS ???
// QDOAS ???          // Cross sections results
// QDOAS ???
// QDOAS ???          for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
// QDOAS ???           {
// QDOAS ???            pTabCrossResults=(!hiddenFlag)?&pTabFeno->TabCrossResults[indexTabCross]:&KURUCZ_buffers.KuruczFeno[indexFeno].results[indexWin][indexTabCross];
// QDOAS ???
// QDOAS ???            if (pTabCrossResults->indexAmf!=ITEM_NONE)
// QDOAS ???             {
// QDOAS ???              if (pTabCrossResults->StoreAmf)              // AMF
// QDOAS ???                ((float *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???                (!pTabFeno->rc)?(float)pTabCrossResults->Amf:(float)defaultValue;
// QDOAS ???              if (pTabCrossResults->StoreVrtCol)           // Vertical column
// QDOAS ???                ((double *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???                (!pTabFeno->rc && (pTabCrossResults->VrtFact!=(double)0.))?
// QDOAS ???                 (double)pTabCrossResults->VrtCol/pTabCrossResults->VrtFact:(double)defaultValue;
// QDOAS ???              if (pTabCrossResults->StoreVrtErr)           // Error on vertical column
// QDOAS ???                ((double *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???                (!pTabFeno->rc && (pTabCrossResults->VrtFact!=(double)0.))?
// QDOAS ???                 (double)pTabCrossResults->VrtErr/pTabCrossResults->VrtFact:(double)defaultValue;
// QDOAS ???             }
// QDOAS ???
// QDOAS ???            if (pTabCrossResults->StoreSlntCol)            // Slant column
// QDOAS ???           	 	((double *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???           	 	(!pTabFeno->rc && (pTabCrossResults->SlntFact!=(double)0.))?
// QDOAS ???           	 	 (double)pTabCrossResults->SlntCol/pTabCrossResults->SlntFact:(double)defaultValue;
// QDOAS ???
// QDOAS ???            if (pTabCrossResults->StoreSlntErr)            // Error on slant column
// QDOAS ???             	((double *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???             	(!pTabFeno->rc && (pTabCrossResults->SlntFact!=(double)0.))?
// QDOAS ???             	 (double)pTabCrossResults->SlntErr/pTabCrossResults->SlntFact:(double)defaultValue;
// QDOAS ???
// QDOAS ???            if (pTabCrossResults->StoreShift)              // Shift
// QDOAS ???             {
// QDOAS ???              ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->Shift:(double)defaultValue;
// QDOAS ???              if (pTabCrossResults->StoreError)
// QDOAS ???               ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaShift:(double)defaultValue;
// QDOAS ???             }
// QDOAS ???
// QDOAS ???            if (pTabCrossResults->StoreStretch)            // Stretch
// QDOAS ???             {
// QDOAS ???              ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->Stretch:(double)defaultValue;
// QDOAS ???              if (pTabCrossResults->StoreError)
// QDOAS ???               ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaStretch:(double)defaultValue;
// QDOAS ???              ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->Stretch2:(double)defaultValue;
// QDOAS ???              if (pTabCrossResults->StoreError)
// QDOAS ???               ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaStretch2:(double)defaultValue;
// QDOAS ???             }
// QDOAS ???
// QDOAS ???            if (pTabCrossResults->StoreScale)              // Scale
// QDOAS ???             {
// QDOAS ???              ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->Scale:(double)defaultValue;
// QDOAS ???              if (pTabCrossResults->StoreError)
// QDOAS ???               ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaScale:(double)defaultValue;
// QDOAS ???              ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->Scale2:(double)defaultValue;
// QDOAS ???              if (pTabCrossResults->StoreError)
// QDOAS ???               ((double *)outputColumns[indexColumn++])[indexRecord]=(!pTabFeno->rc)?(double)pTabCrossResults->SigmaScale2:(double)defaultValue;
// QDOAS ???             }
// QDOAS ???
// QDOAS ???            if (pTabCrossResults->StoreParam)              // Param
// QDOAS ???             {
// QDOAS ???              ((double *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???               (double)((!pTabFeno->rc)?
// QDOAS ???                       ((TabCross[indexTabCross].IndSvdA)?pTabCrossResults->SlntCol:pTabCrossResults->Param):defaultValue);
// QDOAS ???              if (pTabCrossResults->StoreParamError)
// QDOAS ???              ((double *)outputColumns[indexColumn++])[indexRecord]=
// QDOAS ???               (double)((!pTabFeno->rc)?
// QDOAS ???                       ((TabCross[indexTabCross].IndSvdA)?pTabCrossResults->SlntErr:pTabCrossResults->SigmaParam):(double)defaultValue);
// QDOAS ???             }
// QDOAS ???           }
// QDOAS ???       	 }
// QDOAS ???       }
// QDOAS ???
// QDOAS ???     // Color indexes and fluxes
// QDOAS ???
// QDOAS ???     for (indexFluxes=0;indexFluxes<OUTPUT_NFluxes;indexFluxes++)
// QDOAS ???      ((double *)outputColumns[indexColumn++])[indexRecord]=(double)OutputFlux(pEngineContext,OUTPUT_fluxes[indexFluxes]);
// QDOAS ???     for (indexCic=0;indexCic<OUTPUT_NCic;indexCic++)
// QDOAS ???      ((double *)outputColumns[indexColumn++])[indexRecord]=((flux=OutputFlux(pEngineContext,OUTPUT_cic[indexCic][1]))!=(double)0.)?
// QDOAS ???       (double)OutputFlux(pEngineContext,OUTPUT_cic[indexCic][0])/flux:(double)defaultValue;
// QDOAS ???
// QDOAS ???     outputRecords[indexRecord].nbColumns=indexColumn;
// QDOAS ???     outputRecords[indexRecord].year=(int)pEngineContext->present_day.da_year;
// QDOAS ???     outputRecords[indexRecord].month=(int)pEngineContext->present_day.da_mon;
// QDOAS ???     outputRecords[indexRecord].day=(int)pEngineContext->present_day.da_day;
// QDOAS ???     outputRecords[indexRecord].longit=(float)pEngineContext->longitude;
// QDOAS ???     outputRecords[indexRecord].latit=(float)pEngineContext->latitude;
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ================
// QDOAS ??? // OUTPUT FUNCTIONS
// QDOAS ??? // ================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputBuildSiteFileName
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Build the output file name using the selected observation site
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext   structure including information on project options
// QDOAS ??? //               year,month  current date to process (monthly files are created)
// QDOAS ??? //               indexSite   index of the observation site
// QDOAS ??? //               ascFlag     0 to add BIN extension, 1 to add ASC extension
// QDOAS ??? //
// QDOAS ??? // OUTPUT        outputFileName, the name of the output file
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void OutputBuildSiteFileName(ENGINE_CONTEXT *pEngineContext,UCHAR *outputFileName,INT year,INT month,INDEX indexSite,INT ascFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   PROJECT             *pProject;                                                // pointer to project data
// QDOAS ???   PRJCT_RESULTS_ASCII *pResults;                                                // pointer to results part of project
// QDOAS ???   UCHAR               *fileNamePtr;                                             // character pointers used for building output file name
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pProject=(PROJECT *)&pEngineContext->project;
// QDOAS ???   pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;
// QDOAS ???
// QDOAS ???   // Build the complete output path
// QDOAS ???
// QDOAS ???   FILES_RebuildFileName(outputFileName,pResults->path,1);
// QDOAS ???
// QDOAS ???   if ((fileNamePtr=strrchr(outputFileName,PATH_SEP))==NULL)                     // extract output file name without path
// QDOAS ???    fileNamePtr=outputFileName;
// QDOAS ???   else
// QDOAS ???    fileNamePtr++;
// QDOAS ???
// QDOAS ???   sprintf(fileNamePtr,"%s%04d%02d.%s",
// QDOAS ???          (indexSite!=ITEM_NONE)?(UCHAR *)SITES_itemList[indexSite].abbrev:(UCHAR *)"XX",year,month,
// QDOAS ??? 	 (UCHAR *)((ascFlag)?"ASC":"BIN"));
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputBuildFileName
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       For satellites measurements, build automatically a file name
// QDOAS ??? //               for output and create a directory structure
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext       structure including information on project options
// QDOAS ??? // INPUT/OUTPUT  outputFileName  the original output file name to complete
// QDOAS ??? //               ascFlag         0 to add BIN extension, 1 to add ASC extension
// QDOAS ??? //
// QDOAS ??? // RETURN        ERROR_ID_NOTHING_TO_SAVE if there is nothing to save,
// QDOAS ??? //               ERROR_ID_NO otherwise
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OutputBuildFileName(ENGINE_CONTEXT *pEngineContext,UCHAR *outputFileName,INT ascFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   PROJECT             *pProject;                                                // pointer to project data
// QDOAS ???   PRJCT_RESULTS_ASCII *pResults;                                                // pointer to results part of project
// QDOAS ???   OUTPUT_INFO         *pOutput;
// QDOAS ???   UCHAR               *fileNamePtr,                                             // character pointers used for building output file name
// QDOAS ???                        tmpBuffer[MAX_ITEM_TEXT_LEN+1],
// QDOAS ???                       *ptr;
// QDOAS ???   RC                   rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pProject=(PROJECT *)&pEngineContext->project;
// QDOAS ???   pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   if (!outputNbRecords)
// QDOAS ???    THRD_Error(ERROR_TYPE_WARNING,(rc=ERROR_ID_NOTHING_TO_SAVE),"OutputBuildFileName",pEngineContext->fileName);
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     pOutput=&outputRecords[0];
// QDOAS ???
// QDOAS ???     // Build the complete output path
// QDOAS ???
// QDOAS ???     FILES_RebuildFileName(outputFileName,pResults->path,1);
// QDOAS ???
// QDOAS ???     if ((fileNamePtr=strrchr(outputFileName,PATH_SEP))==NULL)                   // extract output file name without path
// QDOAS ???      fileNamePtr=outputFileName;
// QDOAS ???     else
// QDOAS ???      fileNamePtr++;
// QDOAS ???
// QDOAS ???     if (!strlen(fileNamePtr) &&
// QDOAS ???        ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
// QDOAS ???         (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
// QDOAS ???         (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
// QDOAS ???         (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
// QDOAS ???         (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&
// QDOAS ???        ((pProject->spectra.mode!=PRJCT_SPECTRA_MODES_OBSLIST) || (pProject->spectra.radius<=1.)))
// QDOAS ???      {
// QDOAS ???       if ((ptr=strrchr(pEngineContext->fileName,PATH_SEP))==NULL)
// QDOAS ???        ptr=pEngineContext->fileName;
// QDOAS ???       else
// QDOAS ???        ptr++;
// QDOAS ???
// QDOAS ???       fileNamePtr--;                                                            // Remove the separator character in order to build the directory structure
// QDOAS ???       *fileNamePtr=0;
// QDOAS ???
// QDOAS ???       if (pResults->dirFlag)
// QDOAS ???        {
// QDOAS ???         // Create 'year' directory
// QDOAS ???
// QDOAS ???         strcpy(tmpBuffer,outputFileName);
// QDOAS ???         sprintf(outputFileName,"%s%c%d",tmpBuffer,PATH_SEP,(int)pOutput->year);
// QDOAS ???         #if defined (__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ???         mkdir(outputFileName);
// QDOAS ???         #else
// QDOAS ???         mkdir(outputFileName,0755);
// QDOAS ???         #endif
// QDOAS ???
// QDOAS ???         // Create 'month' directory
// QDOAS ???
// QDOAS ???         strcpy(tmpBuffer,outputFileName);
// QDOAS ???         sprintf(outputFileName,"%s%c%02d",tmpBuffer,PATH_SEP,(int)pOutput->month);
// QDOAS ???         #if defined (__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ???         mkdir(outputFileName);
// QDOAS ???         #else
// QDOAS ???         mkdir(outputFileName,0755);
// QDOAS ???         #endif
// QDOAS ???
// QDOAS ???         // Create 'day' directory
// QDOAS ???
// QDOAS ???         strcpy(tmpBuffer,outputFileName);
// QDOAS ???         sprintf(outputFileName,"%s%c%02d",tmpBuffer,PATH_SEP,(int)pOutput->day);
// QDOAS ???         #if defined (__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ???         mkdir(outputFileName);
// QDOAS ???         #else
// QDOAS ???         mkdir(outputFileName,0755);
// QDOAS ???         #endif
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Build output file name
// QDOAS ???
// QDOAS ???       strcpy(tmpBuffer,outputFileName);
// QDOAS ???       sprintf(outputFileName,"%s%c%s",tmpBuffer,PATH_SEP,ptr);
// QDOAS ???
// QDOAS ???       if ((ptr=strrchr(outputFileName,'.'))!=NULL)
// QDOAS ???        strcpy(ptr,(ascFlag)?".ASC":".BIN");
// QDOAS ???       else
// QDOAS ???        strcat(outputFileName,(ascFlag)?".ASC":".BIN");
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputAscPrintTitles
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Print titles of columns in the output ASCII file
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext       structure including information on project options
// QDOAS ??? //               fp              pointer to the output file
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void OutputAscPrintTitles(ENGINE_CONTEXT *pEngineContext,FILE *fp)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexField,indexLine,indexColumn,indexField2;
// QDOAS ???   PRJCT_RESULTS_FIELDS *pField,*pField2;
// QDOAS ???   INT nbColumns,nbLines;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   indexColumn=0;
// QDOAS ???
// QDOAS ???   for (indexField=0;
// QDOAS ???       (indexField<outputNbFields) && (outputFields[indexField].fieldDim1!=ITEM_NONE);)
// QDOAS ???    {
// QDOAS ???    	pField=&outputFields[indexField];
// QDOAS ???
// QDOAS ???    	// Case 1 : the number of columns is known
// QDOAS ???
// QDOAS ???    	if (outputFields[indexField].fieldDim2!=ITEM_NONE)
// QDOAS ???    	 {
// QDOAS ???    	  fprintf(fp,"%c %s\t\n",COMMENT_CHAR,pField->fieldName);
// QDOAS ???    	  for (indexLine=0;indexLine<pField->fieldDim1;indexLine++)
// QDOAS ???    	   {
// QDOAS ???    	    for (indexColumn=0;indexColumn<pField->fieldDim2;indexColumn++)
// QDOAS ???    	     {
// QDOAS ???    	     	fprintf(fp,"%c ",COMMENT_CHAR);
// QDOAS ???
// QDOAS ???           switch(pField->fieldType)
// QDOAS ???            {
// QDOAS ???          // -------------------------------------------------------------------------
// QDOAS ???             case MEMORY_TYPE_STRING :
// QDOAS ???              fprintf(fp,pField->fieldFormat,&outputColumns[indexField][(indexColumn*pField->fieldDim1+indexLine)*pField->fieldSize]);
// QDOAS ???             break;
// QDOAS ???          // -------------------------------------------------------------------------
// QDOAS ???             case MEMORY_TYPE_USHORT :
// QDOAS ???              fprintf(fp,pField->fieldFormat,(USHORT)((USHORT *)outputColumns[indexField])[indexColumn*pField->fieldDim1+indexLine]);
// QDOAS ???             break;
// QDOAS ???          // -------------------------------------------------------------------------
// QDOAS ???             case MEMORY_TYPE_INT :
// QDOAS ???              fprintf(fp,pField->fieldFormat,(int)((int *)outputColumns[indexField])[indexColumn*pField->fieldDim1+indexLine]);
// QDOAS ???             break;
// QDOAS ???          // -------------------------------------------------------------------------
// QDOAS ???             case MEMORY_TYPE_FLOAT :
// QDOAS ???              fprintf(fp,pField->fieldFormat,(float)((float *)outputColumns[indexField])[indexColumn*pField->fieldDim1+indexLine]);
// QDOAS ???             break;
// QDOAS ???          // -------------------------------------------------------------------------
// QDOAS ???             case MEMORY_TYPE_DOUBLE :
// QDOAS ???              fprintf(fp,pField->fieldFormat,(double)((double *)outputColumns[indexField])[indexColumn*pField->fieldDim1+indexLine]);
// QDOAS ???             break;
// QDOAS ???          // -------------------------------------------------------------------------
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           fprintf(fp," ");
// QDOAS ???          }
// QDOAS ???    	    fprintf(fp,"\n");
// QDOAS ???    	   }
// QDOAS ???
// QDOAS ???    	  indexField++;
// QDOAS ???    	 }
// QDOAS ???
// QDOAS ???    	// Case 2 : the number of columns is unknown
// QDOAS ???
// QDOAS ???    	else
// QDOAS ???    	 {
// QDOAS ???    	 	fprintf(fp,"%c ",COMMENT_CHAR);
// QDOAS ???    	 	nbLines=outputFields[indexField].fieldDim1;
// QDOAS ???
// QDOAS ???    	 	// titles
// QDOAS ???
// QDOAS ???    	 	for (nbColumns=0,indexField2=indexField;
// QDOAS ???    	 	    (indexField2<outputNbFields) && (outputFields[indexField2].fieldDim1==nbLines);indexField2++,nbColumns++)
// QDOAS ???    	 	 fprintf(fp,"%s\t",outputFields[indexField2].fieldName);
// QDOAS ???
// QDOAS ???    	 	 fprintf(fp,"\n");
// QDOAS ???
// QDOAS ???    	 	// data
// QDOAS ???
// QDOAS ???    	 	for (indexLine=0;indexLine<pField->fieldDim1;indexLine++)
// QDOAS ???    	 	 {
// QDOAS ???    	    fprintf(fp,"%c ",COMMENT_CHAR);
// QDOAS ???
// QDOAS ???    	    for (indexField2=indexField;indexField2<indexField+nbColumns;indexField2++)
// QDOAS ???    	     {
// QDOAS ???    	     	pField2=&outputFields[indexField2];
// QDOAS ???
// QDOAS ???           switch(pField2->fieldType)
// QDOAS ???            {
// QDOAS ???          // -------------------------------------------------------------------------
// QDOAS ???             case MEMORY_TYPE_STRING :
// QDOAS ???              fprintf(fp,pField2->fieldFormat,&outputColumns[indexField2][indexLine]);
// QDOAS ???             break;
// QDOAS ???          // -------------------------------------------------------------------------
// QDOAS ???             case MEMORY_TYPE_USHORT :
// QDOAS ???              fprintf(fp,pField2->fieldFormat,(USHORT)((USHORT *)outputColumns[indexField2])[indexLine]);
// QDOAS ???             break;
// QDOAS ???          // -------------------------------------------------------------------------
// QDOAS ???             case MEMORY_TYPE_INT :
// QDOAS ???              fprintf(fp,pField2->fieldFormat,(int)((int *)outputColumns[indexField2])[indexLine]);
// QDOAS ???             break;
// QDOAS ???          // -------------------------------------------------------------------------
// QDOAS ???             case MEMORY_TYPE_FLOAT :
// QDOAS ???              fprintf(fp,pField2->fieldFormat,(float)((float *)outputColumns[indexField2])[indexLine]);
// QDOAS ???             break;
// QDOAS ???          // -------------------------------------------------------------------------
// QDOAS ???             case MEMORY_TYPE_DOUBLE :
// QDOAS ???              fprintf(fp,pField2->fieldFormat,(double)((double *)outputColumns[indexField2])[indexLine]);
// QDOAS ???             break;
// QDOAS ???          // -------------------------------------------------------------------------
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           fprintf(fp," ");
// QDOAS ???          }
// QDOAS ???    	    fprintf(fp,"\n");
// QDOAS ???    	   }
// QDOAS ???
// QDOAS ???    	  indexField+=nbColumns;
// QDOAS ???    	 }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   fprintf(fp,"# ");
// QDOAS ???   for (;indexField<outputNbFields;indexField++)
// QDOAS ???    fprintf(fp,"%s\t",outputFields[indexField].fieldName);
// QDOAS ???   fprintf(fp,"\n");
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputAscPrintDataSet
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Flush the data set to the output file (ASC format)
// QDOAS ??? //
// QDOAS ??? // INPUT         fp              pointer to the output file;
// QDOAS ??? //               outputData      all the data to save;
// QDOAS ??? //               nbRecords       the number of records to save.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void OutputAscPrintDataSet(FILE *fp,UCHAR **outputData,INT nbRecords)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   PRJCT_RESULTS_FIELDS *pField;
// QDOAS ???   INDEX indexField,firstRecordField,indexRecord;
// QDOAS ???
// QDOAS ???   for (firstRecordField=0;(firstRecordField<outputNbFields) && (outputFields[firstRecordField].fieldDim1!=ITEM_NONE);firstRecordField++);
// QDOAS ???
// QDOAS ???   for (indexRecord=0;indexRecord<nbRecords;indexRecord++)
// QDOAS ???    {
// QDOAS ???     for (indexField=firstRecordField;indexField<outputNbFields;indexField++)
// QDOAS ???      {
// QDOAS ???       pField=&outputFields[indexField];
// QDOAS ???
// QDOAS ???       switch(pField->fieldType)
// QDOAS ???        {
// QDOAS ???      // -------------------------------------------------------------------------
// QDOAS ???         case MEMORY_TYPE_STRING :
// QDOAS ???          fprintf(fp,pField->fieldFormat,&outputData[indexField][indexRecord*pField->fieldSize]);
// QDOAS ???         break;
// QDOAS ???      // -------------------------------------------------------------------------
// QDOAS ???         case MEMORY_TYPE_USHORT :
// QDOAS ???          fprintf(fp,pField->fieldFormat,(USHORT)((USHORT *)outputData[indexField])[indexRecord]);
// QDOAS ???         break;
// QDOAS ???      // -------------------------------------------------------------------------
// QDOAS ???         case MEMORY_TYPE_INT :
// QDOAS ???          fprintf(fp,pField->fieldFormat,(int)((int *)outputData[indexField])[indexRecord]);
// QDOAS ???         break;
// QDOAS ???      // -------------------------------------------------------------------------
// QDOAS ???         case MEMORY_TYPE_FLOAT :
// QDOAS ???          fprintf(fp,pField->fieldFormat,(float)((float *)outputData[indexField])[indexRecord]);
// QDOAS ???         break;
// QDOAS ???      // -------------------------------------------------------------------------
// QDOAS ???         case MEMORY_TYPE_DOUBLE :
// QDOAS ???          fprintf(fp,pField->fieldFormat,(double)((double *)outputData[indexField])[indexRecord]);
// QDOAS ???         break;
// QDOAS ???      // -------------------------------------------------------------------------
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       fprintf(fp,"\t");
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     fprintf(fp,"\n");
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputBinWriteFields
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Output information on fields in the binary output file
// QDOAS ??? //
// QDOAS ??? // INPUT         fp              pointer to the output file;
// QDOAS ??? //               outputData      all the data to save;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void OutputBinWriteFields(FILE *fp,UCHAR **outputData)
// QDOAS ???  {
// QDOAS ???  	// Declarations
// QDOAS ???
// QDOAS ???  	INDEX indexField;
// QDOAS ???  	PRJCT_RESULTS_FIELDS *pField;
// QDOAS ???
// QDOAS ???  	fwrite(&outputNbDataSet,sizeof(int),1,fp);
// QDOAS ???  	fwrite(&outputNbFields,sizeof(int),1,fp);
// QDOAS ???   fwrite(outputFields,sizeof(PRJCT_RESULTS_FIELDS),outputNbFields,fp);
// QDOAS ???
// QDOAS ???   for (indexField=0;indexField<outputNbFields;indexField++)
// QDOAS ???    {
// QDOAS ???    	pField=&outputFields[indexField];
// QDOAS ???     fwrite(outputData[indexField],pField->fieldDim1*((pField->fieldDim2==ITEM_NONE)?1:pField->fieldDim2)*pField->fieldSize,1,fp);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputBinVerifyFields
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Verify the consistency of the outputFile with the current data set to save
// QDOAS ??? //
// QDOAS ??? // INPUT         outputFileName  the name of the output file
// QDOAS ??? //               fp              pointer to the output file;
// QDOAS ??? //
// QDOAS ??? // RETURN        ERROR_ID_ALLOC if the allocation of a vector failed
// QDOAS ??? //               ERROR_ID_OUTPUT if file and current data set are unconsistent
// QDOAS ??? //               ERROR_ID_NO if both are consisten
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OutputBinVerifyFields(UCHAR *outputFileName,FILE *fp)
// QDOAS ???  {
// QDOAS ???  	// Declarations
// QDOAS ???
// QDOAS ???   PRJCT_RESULTS_FIELDS *fields;
// QDOAS ???  	int nbFields;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???  	// Initialization
// QDOAS ???
// QDOAS ???  	fields=NULL;
// QDOAS ???  	rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   fread(&outputNbDataSet,sizeof(int),1,fp);
// QDOAS ???  	fread(&nbFields,sizeof(int),1,fp);
// QDOAS ???
// QDOAS ???  	if (nbFields!=outputNbFields)
// QDOAS ???  	 THRD_Error(ERROR_TYPE_FATAL,(rc=ERROR_ID_OUTPUT),"OutputBinVerifyFields",outputFileName);
// QDOAS ???  	else if ((fields=(PRJCT_RESULTS_FIELDS *)MEMORY_AllocBuffer("OutputBinVerifyFields","fields",nbFields,sizeof(PRJCT_RESULTS_FIELDS),0,MEMORY_TYPE_STRUCT))==NULL)
// QDOAS ???  	 THRD_Error(ERROR_TYPE_FATAL,(rc=ERROR_ID_ALLOC),"OutputBinVerifyFields",outputFileName);
// QDOAS ???  	else
// QDOAS ???  	 {
// QDOAS ???  	 	fread(fields,sizeof(PRJCT_RESULTS_FIELDS)*nbFields,1,fp);
// QDOAS ???  	 	if (memcmp(fields,outputFields,sizeof(PRJCT_RESULTS_FIELDS)*nbFields))
// QDOAS ???  	 	 THRD_Error(ERROR_TYPE_FATAL,(rc=ERROR_ID_OUTPUT),"OutputBinVerifyFields",outputFileName);
// QDOAS ???  	 }
// QDOAS ???
// QDOAS ???  	// Release the allocated buffer
// QDOAS ???
// QDOAS ???   if (fields!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("OutputBinVerifyFields","fields",fields);
// QDOAS ???
// QDOAS ???  	// Return
// QDOAS ???
// QDOAS ???  	return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputBinWriteDataSet
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Flush the data set to the output file (binary format)
// QDOAS ??? //
// QDOAS ??? // INPUT         fp              pointer to the output file;
// QDOAS ??? //               outputData      all the data to save;
// QDOAS ??? //               nbRecords       the number of records to save.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OutputBinWriteDataSet(FILE *fp,UCHAR **outputData,INT nbRecords)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   double *scalingFactors,*dataMax,*dataMin,defaultValue;
// QDOAS ???   SHORT *data;
// QDOAS ???   PRJCT_RESULTS_FIELDS *pField;
// QDOAS ???   INDEX indexField,indexRecord,firstRecordField;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   dataMax=dataMin=NULL;
// QDOAS ???   defaultValue=(double)9999.;
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???   data=NULL;
// QDOAS ???
// QDOAS ???   if (((scalingFactors=(double *)MEMORY_AllocDVector("OutputBinWriteDataSet","scalingFactors",0,outputNbFields-1))==NULL) ||
// QDOAS ???       ((dataMax=(double *)MEMORY_AllocDVector("OutputBinWriteDataSet","dataMax",0,outputNbFields-1))==NULL) ||
// QDOAS ???       ((dataMin=(double *)MEMORY_AllocDVector("OutputBinWriteDataSet","dataMin",0,outputNbFields-1))==NULL) ||
// QDOAS ???       ((data=(SHORT *)MEMORY_AllocBuffer("OutputBinWriteDataSet","data",nbRecords,sizeof(SHORT),0,MEMORY_TYPE_SHORT))==NULL))
// QDOAS ???    rc=ERROR_ID_ALLOC;
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     // Initializations
// QDOAS ???
// QDOAS ???     for (firstRecordField=0;(firstRecordField<outputNbFields) && (outputFields[firstRecordField].fieldDim1!=ITEM_NONE);firstRecordField++)
// QDOAS ???      {
// QDOAS ???       scalingFactors[firstRecordField]=(double)-1.;
// QDOAS ???       dataMax[firstRecordField]=dataMin[firstRecordField]=(double)0.;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     for (indexField=firstRecordField;indexField<outputNbFields;indexField++)
// QDOAS ???      {
// QDOAS ???      	pField=&outputFields[indexField];
// QDOAS ???      	scalingFactors[indexField]=(double)-1.;
// QDOAS ???      	dataMax[indexField]=dataMin[indexField]=(double)0.;
// QDOAS ???
// QDOAS ???      	// Search for the first valid value
// QDOAS ???
// QDOAS ???      	if ((pField->fieldType==MEMORY_TYPE_INT) || (pField->fieldType==MEMORY_TYPE_FLOAT) || (pField->fieldType==MEMORY_TYPE_DOUBLE))
// QDOAS ???      	 {
// QDOAS ???        	for (indexRecord=0;(indexRecord<nbRecords) &&
// QDOAS ???        	  (((pField->fieldType==MEMORY_TYPE_INT) && (fabs((double)((int *)outputData[indexField])[indexRecord]-defaultValue)<(double)1.e-6)) ||
// QDOAS ???        	   ((pField->fieldType==MEMORY_TYPE_FLOAT) && (fabs((double)((float *)outputData[indexField])[indexRecord]-defaultValue)<(double)1.e-6)) ||
// QDOAS ???        	   ((pField->fieldType==MEMORY_TYPE_DOUBLE) && (fabs((double)((double *)outputData[indexField])[indexRecord]-defaultValue)<(double)1.e-6)));
// QDOAS ???        	     indexRecord++);
// QDOAS ???
// QDOAS ???        	if (indexRecord<nbRecords)
// QDOAS ???        	 switch(pField->fieldType)
// QDOAS ???        	  {
// QDOAS ???        	// ---------------------------------------------------------------------
// QDOAS ???        	   case MEMORY_TYPE_INT :
// QDOAS ???        	    dataMax[indexField]=dataMin[indexField]=(double)((int *)outputData[indexField])[indexRecord++];
// QDOAS ???        	   break;
// QDOAS ???        	// ---------------------------------------------------------------------
// QDOAS ???        	   case MEMORY_TYPE_FLOAT :
// QDOAS ???        	    dataMax[indexField]=dataMin[indexField]=(double)((float *)outputData[indexField])[indexRecord++];
// QDOAS ???        	   break;
// QDOAS ???        	// ---------------------------------------------------------------------
// QDOAS ???        	   case MEMORY_TYPE_DOUBLE :
// QDOAS ???        	    dataMax[indexField]=dataMin[indexField]=(double)((double *)outputData[indexField])[indexRecord++];
// QDOAS ???        	   break;
// QDOAS ???        	// ---------------------------------------------------------------------
// QDOAS ???        	  }
// QDOAS ???      	 }
// QDOAS ???
// QDOAS ???       if (pField->fieldType==MEMORY_TYPE_INT)
// QDOAS ???        {
// QDOAS ???        	for (;indexRecord<nbRecords;indexRecord++)
// QDOAS ???          if (fabs((double)(((int *)outputData[indexField])[indexRecord])-defaultValue)>(double)1.e-6)
// QDOAS ???         	 {
// QDOAS ???         	 	if (((double)((int *)outputData[indexField])[indexRecord])>dataMax[indexField])
// QDOAS ???         	 	 dataMax[indexField]=(double)((int *)outputData[indexField])[indexRecord];
// QDOAS ???         	 	if (((double)((int *)outputData[indexField])[indexRecord])<dataMin[indexField])
// QDOAS ???         	 	 dataMin[indexField]=(double)((int *)outputData[indexField])[indexRecord];
// QDOAS ???         	 }
// QDOAS ???        }
// QDOAS ???       else if (pField->fieldType==MEMORY_TYPE_FLOAT)
// QDOAS ???        {
// QDOAS ???        	for (;indexRecord<nbRecords;indexRecord++)
// QDOAS ???        	 if (fabs((double)(((float *)outputData[indexField])[indexRecord])-defaultValue)>(double)1.e-6)
// QDOAS ???         	 {
// QDOAS ???         	 	if (((double)((float *)outputData[indexField])[indexRecord])>dataMax[indexField])
// QDOAS ???         	 	 dataMax[indexField]=(double)((float *)outputData[indexField])[indexRecord];
// QDOAS ???         	 	if (((double)((float *)outputData[indexField])[indexRecord])<dataMin[indexField])
// QDOAS ???         	 	 dataMin[indexField]=(double)((float *)outputData[indexField])[indexRecord];
// QDOAS ???         	 }
// QDOAS ???        }
// QDOAS ???       else if (pField->fieldType==MEMORY_TYPE_DOUBLE)
// QDOAS ???        {
// QDOAS ???        	for (;indexRecord<nbRecords;indexRecord++)
// QDOAS ???        	 if (fabs((double)(((double *)outputData[indexField])[indexRecord])-defaultValue)>(double)1.e-6)
// QDOAS ???        	  {
// QDOAS ???        	  	if (((double)((double *)outputData[indexField])[indexRecord])>dataMax[indexField])
// QDOAS ???        	  	 dataMax[indexField]=(double)((double *)outputData[indexField])[indexRecord];
// QDOAS ???        	  	if (((double)((double *)outputData[indexField])[indexRecord])<dataMin[indexField])
// QDOAS ???        	  	 dataMin[indexField]=(double)((double *)outputData[indexField])[indexRecord];
// QDOAS ???        	  }
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Calculate the scaling factors and output them
// QDOAS ???
// QDOAS ???     for (indexField=firstRecordField;indexField<outputNbFields;indexField++)
// QDOAS ???      {
// QDOAS ???      	pField=&outputFields[indexField];
// QDOAS ???
// QDOAS ???      	if ((pField->fieldType==MEMORY_TYPE_INT) ||
// QDOAS ???      	    (pField->fieldType==MEMORY_TYPE_FLOAT) ||
// QDOAS ???      	    (pField->fieldType==MEMORY_TYPE_DOUBLE))
// QDOAS ???      	 {
// QDOAS ???         if (dataMin[indexField]<dataMax[indexField])
// QDOAS ???          scalingFactors[indexField]=(double)64000./(dataMax[indexField]-dataMin[indexField]);
// QDOAS ???         else
// QDOAS ???          scalingFactors[indexField]=(double)1.;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     fseek(fp,0L,SEEK_END);
// QDOAS ???     fwrite(&nbRecords,sizeof(int),1,fp);
// QDOAS ???     fwrite(scalingFactors,sizeof(double)*outputNbFields,1,fp);
// QDOAS ???     fwrite(dataMin,sizeof(double)*outputNbFields,1,fp);
// QDOAS ???
// QDOAS ???     // Adjust the maxima
// QDOAS ???
// QDOAS ???     for (indexField=firstRecordField;indexField<outputNbFields;indexField++)
// QDOAS ???      {
// QDOAS ???       pField=&outputFields[indexField];
// QDOAS ???
// QDOAS ???       if (scalingFactors[indexField]>=(double)0.)
// QDOAS ???        {
// QDOAS ???         switch(pField->fieldType)
// QDOAS ???          {
// QDOAS ???        // -------------------------------------------------------------------------
// QDOAS ???           case MEMORY_TYPE_INT :
// QDOAS ???            for (indexRecord=0;indexRecord<nbRecords;indexRecord++)
// QDOAS ???             data[indexRecord]=
// QDOAS ???              (fabs((double)(((int *)outputData[indexField])[indexRecord])-defaultValue)>(double)1.e-6)?
// QDOAS ???              (SHORT)(((double)(((int *)outputData[indexField])[indexRecord])-dataMin[indexField])*scalingFactors[indexField]-32000.):(SHORT)defaultValue;
// QDOAS ???           break;
// QDOAS ???        // -------------------------------------------------------------------------
// QDOAS ???           case MEMORY_TYPE_FLOAT :
// QDOAS ???            for (indexRecord=0;indexRecord<nbRecords;indexRecord++)
// QDOAS ???             data[indexRecord]=
// QDOAS ???              (fabs((double)(((float *)outputData[indexField])[indexRecord])-defaultValue)>(double)1.e-6)?
// QDOAS ???              (SHORT)(((double)(((float *)outputData[indexField])[indexRecord])-dataMin[indexField])*scalingFactors[indexField]-32000.):(SHORT)defaultValue;
// QDOAS ???           break;
// QDOAS ???        // -------------------------------------------------------------------------
// QDOAS ???           case MEMORY_TYPE_DOUBLE :
// QDOAS ???            for (indexRecord=0;indexRecord<nbRecords;indexRecord++)
// QDOAS ???             data[indexRecord]=
// QDOAS ???              (fabs((double)(((double *)outputData[indexField])[indexRecord])-defaultValue)>(double)1.e-6)?
// QDOAS ???              (SHORT)((((double)((double *)outputData[indexField])[indexRecord])-dataMin[indexField])*scalingFactors[indexField]-32000.):(SHORT)defaultValue;
// QDOAS ???           break;
// QDOAS ???        // -------------------------------------------------------------------------
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         fwrite(data,sizeof(SHORT)*nbRecords,1,fp);
// QDOAS ???        }
// QDOAS ???       else
// QDOAS ???        fwrite(&outputData[indexField][0],pField->fieldSize*nbRecords,1,fp);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Update the number of data set
// QDOAS ???
// QDOAS ???     outputNbDataSet++;
// QDOAS ???     fseek(fp,0L,SEEK_SET);
// QDOAS ???     fwrite(&outputNbDataSet,sizeof(int),1,fp);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Release the allocated buffers
// QDOAS ???
// QDOAS ???   if (scalingFactors!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("OutputBinWriteDataSet","scalingFactors",scalingFactors,0);
// QDOAS ???   if (dataMax!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("OutputBinWriteDataSet","dataMax",dataMax,0);
// QDOAS ???   if (dataMin!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("OutputBinWriteDataSet","dataMin",dataMin,0);
// QDOAS ???   if (data!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("OutputBinWriteDataSet","data",data);
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputFileOpen
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Open the outputFile and save the preliminary information
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext       structure including information on project options
// QDOAS ??? //               outputFileName  the name of the outputFile
// QDOAS ??? //               ascFlag         0 to output to a binary file,
// QDOAS ??? //                               1 to output data to an ASCII file
// QDOAS ??? //
// QDOAS ??? // RETURN        pointer to the output file
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? FILE *OutputFileOpen(ENGINE_CONTEXT *pEngineContext,UCHAR *outputFileName,INT ascFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   FILE *fp;
// QDOAS ???   UCHAR r[4],w[4],a[4];
// QDOAS ???   INT newFile;
// QDOAS ???   INT corrupted;
// QDOAS ???
// QDOAS ???   PROJECT             *pProject;                                                // pointer to project data
// QDOAS ???   PRJCT_RESULTS_ASCII *pResults;                                                // pointer to results part of project
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pProject=(PROJECT *)&pEngineContext->project;
// QDOAS ???   pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   strcpy(r,(ascFlag)?"rt":"rb");                                                // open the file in read mode
// QDOAS ???   strcpy(w,(ascFlag)?"w+t":"w+b");                                              // open the file in write mode
// QDOAS ???   strcpy(a,(ascFlag)?"a+t":"r+b");                                              // open the file in append mode
// QDOAS ???
// QDOAS ???   outputNbDataSet=0;
// QDOAS ???
// QDOAS ???   newFile=((fp=fopen(outputFileName,r))==NULL)?1:0;
// QDOAS ???   corrupted=(!newFile && !ascFlag && OutputBinVerifyFields(outputFileName,fp))?1:0;
// QDOAS ???
// QDOAS ???   // Close the file
// QDOAS ???
// QDOAS ???   if (fp!=NULL)
// QDOAS ???    fclose(fp);
// QDOAS ???
// QDOAS ???   // Open the file in append mode
// QDOAS ???
// QDOAS ???   if (corrupted)
// QDOAS ???    fp=NULL;
// QDOAS ???   else if (((fp=fopen(outputFileName,(newFile)?w:a))!=NULL) && newFile)
// QDOAS ???    {
// QDOAS ???     if (THRD_id==THREAD_TYPE_ANALYSIS)
// QDOAS ???      {
// QDOAS ???       // Satellites measurements and automatic reference selection : save information on the selected reference
// QDOAS ???
// QDOAS ???       if (((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
// QDOAS ???            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
// QDOAS ???            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
// QDOAS ???            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
// QDOAS ???            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&
// QDOAS ???             ANALYSE_refSelectionFlag)                                           // automatic reference is requested for at least one analysis window
// QDOAS ???        {
// QDOAS ???        	strcpy(outputColumns[0],OUTPUT_refFile);
// QDOAS ???        	((int *)outputColumns[1])[0]=OUTPUT_nRec;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???    	// Save information on the calibration
// QDOAS ???
// QDOAS ???    	if (pResults->calibFlag)
// QDOAS ???      OutputCalib(pEngineContext);
// QDOAS ???
// QDOAS ???     if (ascFlag)
// QDOAS ???      OutputAscPrintTitles(pEngineContext,fp);
// QDOAS ???     else
// QDOAS ???      OutputBinWriteFields(fp,outputColumns);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return fp;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OUTPUT_FlushBuffers
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Flusth the buffers in a one shot
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext       structure including information on project options
// QDOAS ??? //
// QDOAS ??? // RETURN        Non zero value return code if the output failed
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OUTPUT_FlushBuffers(ENGINE_CONTEXT *pEngineContext)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR outputFileName[MAX_ITEM_TEXT_LEN+1],
// QDOAS ???         outputAutomaticFileName[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   FILE *outputFp;
// QDOAS ???   UCHAR **outputData;
// QDOAS ???
// QDOAS ???   OBSERVATION_SITE *pSite;
// QDOAS ???   PROJECT             *pProject;                     // pointer to project data
// QDOAS ???   PRJCT_RESULTS_FIELDS *pField;
// QDOAS ???   PRJCT_RESULTS_ASCII *pResults;                     // pointer to results part of project
// QDOAS ???   UCHAR               *ptr;
// QDOAS ???   INDEX indexSite,indexField,firstRecordField,indexRecord,oldYear,oldMonth,oldRecord;
// QDOAS ???   INT sitesNumber,fieldDim2;
// QDOAS ???   INT automatic,nbRecords;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pProject=(PROJECT *)&pEngineContext->project;
// QDOAS ???   pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;
// QDOAS ???
// QDOAS ???   memset(outputFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   memset(outputAutomaticFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???   sitesNumber=TREE_itemType[TREE_ITEM_TYPE_SITE_CHILDREN].dataNumber;
// QDOAS ???   outputFp=NULL;
// QDOAS ???   outputData=NULL;
// QDOAS ???   firstRecordField=0;
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   if ((pResults->analysisFlag || pResults->calibFlag) && outputNbRecords && !(rc=OutputBuildFileName(pEngineContext,outputFileName,!pResults->binaryFlag)))
// QDOAS ???    {
// QDOAS ???     if ((ptr=strrchr(outputFileName,PATH_SEP))==NULL)
// QDOAS ???      ptr=outputFileName;
// QDOAS ???     else
// QDOAS ???      ptr++;
// QDOAS ???
// QDOAS ???     automatic=!strlen(ptr);
// QDOAS ???
// QDOAS ???     // ------------------
// QDOAS ???     // NOT AUTOMATIC MODE
// QDOAS ???     // ------------------
// QDOAS ???
// QDOAS ???     if (!automatic)
// QDOAS ???      {
// QDOAS ???       if ((outputFp=OutputFileOpen(pEngineContext,outputFileName,!pResults->binaryFlag))==NULL)
// QDOAS ???        rc=ERROR_SetLast("OUTPUT_FlushBuffers",ERROR_TYPE_FATAL,ERROR_ID_FILE_OPEN,"outputFileName");
// QDOAS ???       else if (pResults->binaryFlag)
// QDOAS ???        rc=OutputBinWriteDataSet(outputFp,outputColumns,outputNbRecords);
// QDOAS ???       else
// QDOAS ???        OutputAscPrintDataSet(outputFp,outputColumns,outputNbRecords);
// QDOAS ???      }
// QDOAS ???     else if ((outputData=(UCHAR **)MEMORY_AllocBuffer("OUTPUT_FlushBuffers","outputData",MAX_FIELDS,sizeof(UCHAR *),0,MEMORY_TYPE_PTR))==NULL)
// QDOAS ???      rc=ERROR_SetLast("OUTPUT_FlushBuffers",ERROR_TYPE_FATAL,ERROR_ID_ALLOC,"outputData");
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???     // -------------------
// QDOAS ???     // FULL AUTOMATIC MODE
// QDOAS ???     // -------------------
// QDOAS ???
// QDOAS ???      	memset(outputData,0,sizeof(UCHAR *)*MAX_FIELDS);
// QDOAS ???
// QDOAS ???      	for (firstRecordField=0;(firstRecordField<outputNbFields) && (outputFields[firstRecordField].fieldDim1!=ITEM_NONE) && !rc;firstRecordField++)
// QDOAS ???      	 {
// QDOAS ???      	 	pField=&outputFields[firstRecordField];
// QDOAS ???      	 	fieldDim2=(pField->fieldDim2!=ITEM_NONE)?pField->fieldDim2:1;
// QDOAS ???
// QDOAS ???      	  if ((outputData[firstRecordField]=(UCHAR *)MEMORY_AllocBuffer("OUTPUT_FlushBuffers",pField->fieldName,
// QDOAS ???      	                                             pField->fieldDim1*fieldDim2,pField->fieldSize,0,pField->fieldType))==NULL)
// QDOAS ???      	   rc=ERROR_SetLast("OUTPUT_FlushBuffers",ERROR_TYPE_FATAL,ERROR_ID_ALLOC,"outputData[firstRecordField]");
// QDOAS ???      	 }
// QDOAS ???
// QDOAS ???      	for (indexField=firstRecordField;(indexField<outputNbFields) && !rc;indexField++)
// QDOAS ???      	 {
// QDOAS ???      	 	pField=&outputFields[indexField];
// QDOAS ???      	  if ((outputData[indexField]=(UCHAR *)MEMORY_AllocBuffer("OUTPUT_FlushBuffers",pField->fieldName,outputNbRecords,pField->fieldSize,0,pField->fieldType))==NULL)
// QDOAS ???      	   rc=ERROR_SetLast("OUTPUT_FlushBuffers",ERROR_TYPE_FATAL,ERROR_ID_ALLOC,"outputData[indexField]");
// QDOAS ???      	 }
// QDOAS ???
// QDOAS ???      	if (!rc)
// QDOAS ???      	 {
// QDOAS ???      	 	// Overpasses : records are distributed using information on the measurement date and the geolocation
// QDOAS ???
// QDOAS ???      	 	if ((pProject->spectra.mode==PRJCT_SPECTRA_MODES_OBSLIST) && (pProject->spectra.radius>1.))
// QDOAS ???
// QDOAS ???          for (indexSite=0;indexSite<sitesNumber;indexSite++)
// QDOAS ???           {
// QDOAS ???      	 	 	 indexRecord=0;
// QDOAS ???      	 	 	 pSite=&SITES_itemList[indexSite];
// QDOAS ???
// QDOAS ???      	 	 	 while ((indexRecord<outputNbRecords) && !rc)
// QDOAS ???      	 	 	  {
// QDOAS ???        	 	 	 oldYear=outputRecords[indexRecord].year;
// QDOAS ???        	 	 	 oldMonth=outputRecords[indexRecord].month;
// QDOAS ???
// QDOAS ???      	 	 	  	for (indexField=0;indexField<firstRecordField;indexField++)
// QDOAS ???      	 	 	  	 {
// QDOAS ???                fieldDim2=(pField->fieldDim2!=ITEM_NONE)?pField->fieldDim2:1;
// QDOAS ???      	 	 	  	  memcpy(outputData[indexField],outputColumns[indexField],outputFields[indexField].fieldDim1*fieldDim2*outputFields[indexField].fieldSize);
// QDOAS ???      	 	 	  	 }
// QDOAS ???
// QDOAS ???      	 	 	  	for (nbRecords=0;(indexRecord<outputNbRecords) && ((outputRecords[indexRecord].year==oldYear) || (outputRecords[indexRecord].month==oldMonth));indexRecord++)
// QDOAS ???      	 	 	  	 if (THRD_GetDist((double)outputRecords[indexRecord].longit,(double)outputRecords[indexRecord].latit,
// QDOAS ???      	 	 	  	                  (double)pSite->longitude,(double)pSite->latitude)<=(double)pProject->spectra.radius)
// QDOAS ???      	 	 	  	  {
// QDOAS ???        	 	  	 	 for (indexField=firstRecordField;(indexField<outputNbFields);indexField++)
// QDOAS ???        	 	  	 	  {
// QDOAS ???        	 	  	 	  	pField=&outputFields[indexField];
// QDOAS ???     	   	  	 	    memcpy(&outputData[indexField][nbRecords*pField->fieldSize],&outputColumns[indexField][indexRecord*pField->fieldSize],pField->fieldSize);
// QDOAS ???      	 	    	 	  }
// QDOAS ???      	 	    	 	 nbRecords++;
// QDOAS ???      	 	    	 	}
// QDOAS ???
// QDOAS ???      	 	 	  	if (nbRecords)
// QDOAS ???      	 	 	  	 {
// QDOAS ???      	 	 	  	 	OutputBuildSiteFileName(pEngineContext,outputAutomaticFileName,oldYear,oldMonth,indexSite,!pResults->binaryFlag);
// QDOAS ???
// QDOAS ???                if ((outputFp=OutputFileOpen(pEngineContext,outputAutomaticFileName,!pResults->binaryFlag))==NULL)
// QDOAS ???                 ERROR_SetLast("OutputFlushBuffers",ERROR_TYPE_FATAL,ERROR_ID_FILE_OPEN,"outputFileName");
// QDOAS ???              //   THRD_Error(ERROR_TYPE_FATAL,(rc=ERROR_ID_FILE_OPEN),"OutputFlushBuffers",outputFileName);
// QDOAS ???                else if (pResults->binaryFlag)
// QDOAS ???                 rc=OutputBinWriteDataSet(outputFp,outputData,nbRecords);
// QDOAS ???                else
// QDOAS ???                 OutputAscPrintDataSet(outputFp,outputData,nbRecords);
// QDOAS ???
// QDOAS ???                if (outputFp!=NULL)
// QDOAS ???                 {
// QDOAS ???                 	fclose(outputFp);
// QDOAS ???                 	outputFp=NULL;
// QDOAS ???                 }
// QDOAS ???      	 	 	  	 }
// QDOAS ???      	 	 	  }
// QDOAS ???           }
// QDOAS ???
// QDOAS ???      	 	// Records are saved using the information on the date only
// QDOAS ???
// QDOAS ???      	 	else
// QDOAS ???      	 	 {
// QDOAS ???      	 	 	nbRecords=indexRecord=0;
// QDOAS ???      	 	 	oldRecord=0;
// QDOAS ???      	 	 	indexSite=SITES_GetIndex(pProject->instrumental.observationSite);
// QDOAS ???
// QDOAS ???      	 	 	while (indexRecord<outputNbRecords)
// QDOAS ???      	 	 	 {
// QDOAS ???        	 	 	oldYear=outputRecords[indexRecord].year;
// QDOAS ???        	 	 	oldMonth=outputRecords[indexRecord].month;
// QDOAS ???
// QDOAS ???     	 	 	  	for (indexField=0;indexField<firstRecordField;indexField++)
// QDOAS ???     	 	 	  	 {
// QDOAS ???     	 	 	  	 	fieldDim2=(pField->fieldDim2!=ITEM_NONE)?pField->fieldDim2:1;
// QDOAS ???     	 	 	  	  memcpy(outputData[indexField],outputColumns[indexField],outputFields[indexField].fieldDim1*fieldDim2*outputFields[indexField].fieldSize);
// QDOAS ???     	 	 	  	 }
// QDOAS ???
// QDOAS ???      	 	 	 	for (;(indexRecord<outputNbRecords) && ((outputRecords[indexRecord].year==oldYear) || (outputRecords[indexRecord].month==oldMonth));indexRecord++);
// QDOAS ???
// QDOAS ???      	 	 	 	if ((nbRecords=indexRecord-oldRecord)>0)
// QDOAS ???      	 	 	 	 {
// QDOAS ???        	 	 	 	for (indexField=firstRecordField;(indexField<outputNbFields) && !rc;indexField++)
// QDOAS ???        	 	 	 	 {
// QDOAS ???        	 	 	 	 	pField=&outputFields[indexField];
// QDOAS ???     	   	 	 	   memcpy(&outputData[indexField][0],&outputColumns[indexField][oldRecord*pField->fieldSize],nbRecords*pField->fieldSize);
// QDOAS ???      	 	   	 	 }
// QDOAS ???
// QDOAS ???      	 	 	 	 	OutputBuildSiteFileName(pEngineContext,outputAutomaticFileName,oldYear,oldMonth,indexSite,!pResults->binaryFlag);
// QDOAS ???
// QDOAS ???               if ((outputFp=OutputFileOpen(pEngineContext,outputAutomaticFileName,!pResults->binaryFlag))==NULL)
// QDOAS ???                ERROR_SetLast("OutputFlushBuffers",ERROR_TYPE_FATAL,ERROR_ID_FILE_OPEN,"outputFileName");
// QDOAS ???               else if (pResults->binaryFlag)
// QDOAS ???                rc=OutputBinWriteDataSet(outputFp,outputData,nbRecords);
// QDOAS ???               else
// QDOAS ???                OutputAscPrintDataSet(outputFp,outputData,nbRecords);
// QDOAS ???
// QDOAS ???               if (outputFp!=NULL)
// QDOAS ???                {
// QDOAS ???                	fclose(outputFp);
// QDOAS ???                	outputFp=NULL;
// QDOAS ???                }
// QDOAS ???      	 	 	 	 }
// QDOAS ???
// QDOAS ???      	 	 	 	oldRecord=indexRecord;
// QDOAS ???      	 	 	 }
// QDOAS ???      	 	 }
// QDOAS ???      	 }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Close file
// QDOAS ???
// QDOAS ???     if (outputFp!=NULL)
// QDOAS ???      fclose(outputFp);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Release the allocated buffers
// QDOAS ???
// QDOAS ???   if (outputData!=NULL)
// QDOAS ???    {
// QDOAS ???     for (indexField=0;indexField<outputNbFields;indexField++)
// QDOAS ???      if (outputData[indexField]!=NULL)
// QDOAS ???       MEMORY_ReleaseBuffer("OUTPUT_FlushBuffers",outputFields[indexField].fieldName,outputData[indexField]);
// QDOAS ???
// QDOAS ???     MEMORY_ReleaseBuffer("OUTPUT_FlushBuffers","outputData",outputData);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   if (rc)
// QDOAS ???    THRD_ProcessLastError();
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // =========
// QDOAS ??? // NASA-AMES
// QDOAS ??? // =========
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputNasaAmesHeader
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Build the NASA-AMES file header
// QDOAS ??? //
// QDOAS ??? // INPUT         Tm        number of seconds to build the current date and time
// QDOAS ??? //               resFile   the name of the nasa-ames file
// QDOAS ??? //
// QDOAS ??? // RETURN        ERROR_ID_FILE_OPEN if the output file can not be open
// QDOAS ??? //               ERROR_ID_ALLOC if the allocation of a buffer failed
// QDOAS ??? //               ERROR_ID_NO on success
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OutputNasaAmesHeader(double Tm,UCHAR *resFile)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   PROJECT             *pProject;                                                // pointer to project part in data on spectrum record
// QDOAS ???   PRJCT_INSTRUMENTAL  *pInstrumental;                                           // pointer to instrumental part of project
// QDOAS ???   PRJCT_RESULTS_NASA  *pResults;                                                // pointer to results part of project
// QDOAS ???   NASA_COMPONENTS     *pNasaResults;                                            // pointer to scaling factors
// QDOAS ???   UCHAR               *nasaBuf,                                                 // copy of NASA-AMES header
// QDOAS ???                       *nasaBufKey,                                              // pointer to key words in lines in header
// QDOAS ???                       *nasaBufPtr,*ptrOld,*ptrNew,                              // pointers to characters
// QDOAS ???                        nasaBufLine[MAX_ITEM_TEXT_LEN+1],                        // copy of a line from header
// QDOAS ???                        nasaBufTemp[MAX_ITEM_TEXT_LEN+1];                        // temporary buffer
// QDOAS ???   INDEX                indexField,                                              // browse fields in record
// QDOAS ???                        indexLine,                                               // browse lines of header
// QDOAS ???                        i;
// QDOAS ???   INT                  nbAmPm,                                                  // number of total AM/PM results
// QDOAS ???                        nbRejected;                                              // number of rejected AM/PM results
// QDOAS ???   FILE                *resuFp;                                                  // file pointer
// QDOAS ???   RC                   rc;                                                      // return code
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pProject=(PROJECT *)&OUTPUT_nasaResults.project;
// QDOAS ???   pInstrumental=(PRJCT_INSTRUMENTAL *)&pProject->instrumental;
// QDOAS ???   pResults=(PRJCT_RESULTS_NASA *)&pProject->nasaResults;
// QDOAS ???   nasaBuf=NULL;
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Open file and allocate buffer for header processing
// QDOAS ???
// QDOAS ???   if ((resuFp=fopen(resFile,"w+t"))==NULL)
// QDOAS ???    THRD_Error(ERROR_TYPE_FATAL,(rc=ERROR_ID_FILE_OPEN),"OutputNasaAmesHeader ",resFile);
// QDOAS ???   else if (OUTPUT_nasaBufSize && ((nasaBuf=(UCHAR *)MEMORY_AllocBuffer("OutputNasaAmesHeader ","nasaBuf",OUTPUT_nasaBufSize+1,1,0,MEMORY_TYPE_STRING))==NULL))
// QDOAS ???    rc=ERROR_ID_ALLOC;
// QDOAS ???   else if (OUTPUT_nasaBufSize && (OUTPUT_nasaBuf!=NULL) )
// QDOAS ???    {
// QDOAS ???     memset(nasaBuf,0,OUTPUT_nasaBufSize+1);                                     // force end of string
// QDOAS ???     memcpy(nasaBuf,OUTPUT_nasaBuf,OUTPUT_nasaBufSize);                          // make a copy of file header
// QDOAS ???
// QDOAS ???     // Set number of rejected AM/PM results
// QDOAS ???
// QDOAS ???     nbRejected=0;
// QDOAS ???
// QDOAS ???     for (indexField=0;indexField<PRJCT_RESULTS_NASA_MAX;indexField++)
// QDOAS ???      {
// QDOAS ???       pNasaResults=&OUTPUT_nasaResults.components[indexField];
// QDOAS ???
// QDOAS ???       if ((pNasaResults->indexWindow==ITEM_NONE) || (pNasaResults->indexTabCross==ITEM_NONE))
// QDOAS ???        nbRejected+=((indexField==PRJCT_RESULTS_NASA_NO2)||(indexField==PRJCT_RESULTS_NASA_O3))?8:4;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     for (indexLine=0,ptrOld=nasaBuf;(indexLine<OUTPUT_nasaBufNLines)&&(ptrOld!=NULL);indexLine++,ptrOld=ptrNew)
// QDOAS ???      {
// QDOAS ???       nbAmPm=0;
// QDOAS ???
// QDOAS ???       // Set the end of the line
// QDOAS ???
// QDOAS ???       if ((ptrNew=strchr(ptrOld,0x0A))!=NULL)
// QDOAS ???        *ptrNew++=0;
// QDOAS ???
// QDOAS ???       strcpy(nasaBufLine,ptrOld);
// QDOAS ???
// QDOAS ???       // Search for key word
// QDOAS ???
// QDOAS ???       if ((nasaBufKey=strchr(nasaBufLine,'$'))!=NULL)
// QDOAS ???        {
// QDOAS ???         if ((nasaBufPtr=strchr(nasaBufKey,' '))!=NULL)
// QDOAS ???          {
// QDOAS ???           *nasaBufPtr++=0;                                                      // set the end of key word
// QDOAS ???           strcpy(nasaBufTemp,nasaBufPtr);                                       // nasaBufTemp receive the remaining characters
// QDOAS ???          }
// QDOAS ???         else
// QDOAS ???          nasaBufTemp[0]=0;
// QDOAS ???
// QDOAS ???         // Replace key word
// QDOAS ???
// QDOAS ???         if (!STD_Stricmp(nasaBufKey,"$LINE"))
// QDOAS ???          sprintf(nasaBufKey,"%d %s",OUTPUT_nasaBufNLines-nbRejected,nasaBufTemp);
// QDOAS ???         else if (!STD_Stricmp(nasaBufKey,"$DATE"))
// QDOAS ???          {
// QDOAS ???          	#if defined (__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ???           SYSTEMTIME sysTime;
// QDOAS ???           GetLocalTime(&sysTime);
// QDOAS ???           sprintf(nasaBufKey,"%02d %02d %02d %02d %02d %02d",
// QDOAS ???                          (INT)ZEN_FNCaljye(&Tm),
// QDOAS ???                          (INT)ZEN_FNCaljmon(ZEN_FNCaljye(&Tm),ZEN_FNCaljda(&Tm)),
// QDOAS ???                          (INT)ZEN_FNCaljday(ZEN_FNCaljye(&Tm),ZEN_FNCaljda(&Tm)),
// QDOAS ???                          (INT)sysTime.wYear,(INT)sysTime.wMonth,(INT)sysTime.wDay);
// QDOAS ???           #else
// QDOAS ???           time_t today;
// QDOAS ???           char shortdate[9];
// QDOAS ???           today=time(NULL);
// QDOAS ???           strftime(shortdate,9,"%y %m %d",localtime(&today));
// QDOAS ???           sprintf(nasaBufKey,"%02d %02d %02d %s",
// QDOAS ???                          (INT)ZEN_FNCaljye(&Tm),
// QDOAS ???                          (INT)ZEN_FNCaljmon(ZEN_FNCaljye(&Tm),ZEN_FNCaljda(&Tm)),
// QDOAS ???                          (INT)ZEN_FNCaljday(ZEN_FNCaljye(&Tm),ZEN_FNCaljda(&Tm)),
// QDOAS ???                          shortdate);
// QDOAS ???           #endif
// QDOAS ???          }
// QDOAS ???         else if (!STD_Stricmp(nasaBufKey,"$INSTR"))
// QDOAS ???          sprintf(nasaBufKey,"%s",pResults->instrument);
// QDOAS ???         else if (!STD_Stricmp(nasaBufKey,"$EXPERIMENT"))
// QDOAS ???          sprintf(nasaBufKey,"%s",pResults->experiment);
// QDOAS ???         else if (!STD_Stricmp(nasaBufKey,"$AM/PM-ENTRIES"))
// QDOAS ???          sprintf(nasaBufKey,"%d",(nbAmPm=atoi(nasaBufTemp))-nbRejected);
// QDOAS ???         else if (!STD_Stricmp(nasaBufKey,"$SITE"))
// QDOAS ???          sprintf(nasaBufKey,"%s %s",pInstrumental->observationSite,nasaBufTemp);
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Copy line into results file
// QDOAS ???
// QDOAS ???       fprintf(resuFp,"%s\n",nasaBufLine);
// QDOAS ???
// QDOAS ???       // AM/PM results description lines processing
// QDOAS ???
// QDOAS ???       if (nbAmPm)
// QDOAS ???        {
// QDOAS ???         // Output scaling factors
// QDOAS ???
// QDOAS ???         memset(nasaBufLine,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???         for (indexField=0;indexField<PRJCT_RESULTS_NASA_MAX;indexField++)
// QDOAS ???          {
// QDOAS ???           pNasaResults=&OUTPUT_nasaResults.components[indexField];
// QDOAS ???
// QDOAS ???           if (pNasaResults->indexWindow!=ITEM_NONE)
// QDOAS ???            {
// QDOAS ???             switch(indexField)
// QDOAS ???              {
// QDOAS ???            // -------------------------------------------------------------
// QDOAS ???               case PRJCT_RESULTS_NASA_NO2 :
// QDOAS ???               case PRJCT_RESULTS_NASA_O3 :
// QDOAS ???                sprintf(nasaBufTemp,"%g %g %g %g ",pNasaResults->VrtColFact,pNasaResults->SlntColFact,pNasaResults->SlntErrFact,pNasaResults->AmfFact);
// QDOAS ???               break;
// QDOAS ???            // -------------------------------------------------------------
// QDOAS ???               case PRJCT_RESULTS_NASA_OCLO :
// QDOAS ???               case PRJCT_RESULTS_NASA_BRO :
// QDOAS ???                sprintf(nasaBufTemp,"%g %g ",pNasaResults->SlntColFact,pNasaResults->SlntErrFact);
// QDOAS ???               break;
// QDOAS ???            // -------------------------------------------------------------
// QDOAS ???              }
// QDOAS ???
// QDOAS ???             strcat(nasaBufLine,nasaBufTemp);
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         strcat(nasaBufLine,OUTPUT_nasaBufTemp);
// QDOAS ???
// QDOAS ???         fprintf(resuFp,"%s\n",nasaBufLine);                                     // AM scaling factors
// QDOAS ???         fprintf(resuFp,"%s\n",nasaBufLine);                                     // PM scaling factors (assumes it's the same as AM one)
// QDOAS ???
// QDOAS ???         // Output results format
// QDOAS ???
// QDOAS ???         memset(nasaBufLine,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???         sprintf(nasaBufTemp,"9999 ");
// QDOAS ???
// QDOAS ???         for (indexField=0;indexField<(nbAmPm-nbRejected)/2;indexField++)
// QDOAS ???          strcat(nasaBufLine,nasaBufTemp);
// QDOAS ???
// QDOAS ???         fprintf(resuFp,"%s\n",nasaBufLine);                                     // AM format
// QDOAS ???         fprintf(resuFp,"%s\n",nasaBufLine);                                     // PM format (assumes it's the same as AM one)
// QDOAS ???
// QDOAS ???         // Skip equivalent formatted lines in original header
// QDOAS ???
// QDOAS ???         for (i=0;i<4;i++)
// QDOAS ???          {
// QDOAS ???           if ((ptrNew!=NULL) && ((ptrNew=strchr(ptrNew,0x0A))!=NULL))
// QDOAS ???            ptrNew++;
// QDOAS ???
// QDOAS ???           indexLine++;
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         // AM/PM results description lines
// QDOAS ???
// QDOAS ???         for (i=0,ptrOld=ptrNew;(i<nbAmPm) && (ptrOld!=NULL);i++,ptrOld=ptrNew)
// QDOAS ???          {
// QDOAS ???           if ((ptrNew=strchr(ptrOld,0x0A))!=NULL)
// QDOAS ???            *ptrNew++=0;
// QDOAS ???
// QDOAS ???           strcpy(nasaBufTemp,ptrOld);
// QDOAS ???           STD_Strupr(nasaBufTemp);
// QDOAS ???
// QDOAS ???           // Copy into results file, only lines relative to selected constituents
// QDOAS ???
// QDOAS ???           for (indexField=0;indexField<PRJCT_RESULTS_NASA_MAX;indexField++)
// QDOAS ???            {
// QDOAS ???             pNasaResults=&OUTPUT_nasaResults.components[indexField];
// QDOAS ???             if ((pNasaResults->indexWindow==ITEM_NONE) && (strstr(nasaBufTemp,pNasaResults->symbolName)!=NULL))
// QDOAS ???              break;
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           if (indexField==PRJCT_RESULTS_NASA_MAX)
// QDOAS ???            fprintf(resuFp,"%s\n",ptrOld);
// QDOAS ???
// QDOAS ???           indexLine++;
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   if (nasaBuf!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("OutputNasaAmesHeader ","nasaBuf",nasaBuf);
// QDOAS ???   if (resuFp!=NULL)
// QDOAS ???    fclose(resuFp);
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputStoreNasaAmes
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Store results for further calculation of NASA-AMES AM/PM means
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext       structure including information on project options
// QDOAS ??? //
// QDOAS ??? // RETURN        ERROR_ID_MEMORY if the buffer is full
// QDOAS ??? //               ERROR_ID_NO on success
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OutputStoreNasaAmes(ENGINE_CONTEXT *pEngineContext)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   NASA_COMPONENTS *pComponent;
// QDOAS ???   CROSS_RESULTS *pResults;
// QDOAS ???   INDEX indexField;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   if (pEngineContext->project.nasaResults.nasaFlag && (OUTPUT_AmfSpace!=NULL))
// QDOAS ???    {
// QDOAS ???     if ((OUTPUT_nasaResults.julianDay=ZEN_FNCaljda(&pEngineContext->Tm))!=OUTPUT_nasaResults.oldJulianDay)
// QDOAS ???      {
// QDOAS ???       // Save previous results in NASA-AMES format
// QDOAS ???
// QDOAS ???       OUTPUT_SaveNasaAmes();
// QDOAS ???
// QDOAS ???       memcpy(&OUTPUT_nasaResults.project,&pEngineContext->project,sizeof(PROJECT));
// QDOAS ???       OUTPUT_nasaResults.oldJulianDay=OUTPUT_nasaResults.julianDay;
// QDOAS ???       OUTPUT_nasaNResults=0;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if ((pEngineContext->Zm>=(double)77.) && (pEngineContext->Zm<=(double)95.))
// QDOAS ???      {
// QDOAS ???       // Not enough memory for storing all results
// QDOAS ???
// QDOAS ???       if (OUTPUT_nasaNResults==MAX_RESULTS)
// QDOAS ???        THRD_Error(ERROR_TYPE_FATAL,(rc=ERROR_ID_MEMORY),"OutputStoreNasaAmes ");
// QDOAS ???       else
// QDOAS ???        {
// QDOAS ???         // Auxiliary results safe keeping
// QDOAS ???
// QDOAS ???         OUTPUT_nasaResults.Zm[OUTPUT_nasaNResults]=pEngineContext->Zm;           // zenith angle
// QDOAS ???         OUTPUT_nasaResults.Tm[OUTPUT_nasaNResults]=pEngineContext->Tm;           // measurement time in sec
// QDOAS ???         OUTPUT_nasaResults.TDet[OUTPUT_nasaNResults]=pEngineContext->TDet;       // detector temperature
// QDOAS ???         OUTPUT_nasaResults.TimeDec[OUTPUT_nasaNResults]=pEngineContext->localTimeDec; // decimal measurement time
// QDOAS ???         OUTPUT_nasaResults.Cic[OUTPUT_nasaNResults]=pEngineContext->Cic;         // color index
// QDOAS ???         OUTPUT_nasaResults.SkyObs=pEngineContext->SkyObs;                        // the last sky state indication
// QDOAS ???
// QDOAS ???         // Primary results safe keeping
// QDOAS ???
// QDOAS ???         for (indexField=0;indexField<PRJCT_RESULTS_NASA_MAX;indexField++)
// QDOAS ???          {
// QDOAS ???           pComponent=&OUTPUT_nasaResults.components[indexField];
// QDOAS ???
// QDOAS ???           if (pComponent->indexWindow!=ITEM_NONE)
// QDOAS ???            {
// QDOAS ???             pResults=&TabFeno[pComponent->indexWindow].TabCrossResults[pComponent->indexTabCross];
// QDOAS ???
// QDOAS ???             pComponent->SlntCol[OUTPUT_nasaNResults]=pResults->SlntCol;
// QDOAS ???
// QDOAS ???             if (pComponent->Amf!=NULL)
// QDOAS ???              {
// QDOAS ???               pComponent->Error[OUTPUT_nasaNResults]=pResults->Amf/pResults->SlntErr;
// QDOAS ???               pComponent->Amf[OUTPUT_nasaNResults]=pResults->Amf;
// QDOAS ???              }
// QDOAS ???             else
// QDOAS ???              pComponent->Error[OUTPUT_nasaNResults]=(double)1./pResults->SlntErr;
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         OUTPUT_nasaNResults++;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputResidualColumns
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Calculate the residual columns in the selected reference spectrum
// QDOAS ??? //
// QDOAS ??? // RETURN        Non zero value return code if the function failed
// QDOAS ??? //               0 on success
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OutputResidualColumns(void)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   NASA_COMPONENTS *pComponent;
// QDOAS ???   CROSS_RESULTS *pResults;
// QDOAS ???   INDEX indexField,indexResults,indexAmPm;
// QDOAS ???   INT NResults[2],firstResults[2],NAmPm;
// QDOAS ???   SVD svd;
// QDOAS ???   double *Zm,*Error,x[3],VertR,FactR,AmfR;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   memset(&svd,0,sizeof(SVD));
// QDOAS ???   svd.DimC=2;
// QDOAS ???
// QDOAS ???   Zm=OUTPUT_nasaResults.Zm;
// QDOAS ???   Error=NULL;
// QDOAS ???
// QDOAS ???   // Search for records in the specified zenith range
// QDOAS ???
// QDOAS ???   for (indexResults=NResults[0]=NResults[1]=0;indexResults<OUTPUT_nasaNResults;indexResults++)
// QDOAS ???
// QDOAS ???    if ((Zm[indexResults]>=(double)84.) && (Zm[indexResults]<=(double)91.))
// QDOAS ???     {
// QDOAS ???      indexAmPm=(OUTPUT_nasaResults.TimeDec[indexResults]<=THRD_localNoon)?0:1;
// QDOAS ???
// QDOAS ???      if (!NResults[indexAmPm])
// QDOAS ???       firstResults[indexAmPm]=indexResults;
// QDOAS ???
// QDOAS ???      NResults[indexAmPm]++;
// QDOAS ???     }
// QDOAS ???
// QDOAS ???   // Buffers allocation
// QDOAS ???
// QDOAS ???   if (!(svd.DimL=max(NResults[0],NResults[1])) || ((rc=ANALYSE_SvdLocalAlloc("OutputResidualColumns",&svd))!=0) ||
// QDOAS ???       ((Error=(double *)MEMORY_AllocDVector("OutputResidualColumns ","Slant",1,svd.DimL))==NULL))
// QDOAS ???
// QDOAS ???    rc=ERROR_ID_ALLOC;
// QDOAS ???
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     // Browse NASA-AMES results
// QDOAS ???
// QDOAS ???     for (indexField=0;indexField<PRJCT_RESULTS_NASA_MAX;indexField++)
// QDOAS ???      {
// QDOAS ???       pComponent=&OUTPUT_nasaResults.components[indexField];
// QDOAS ???
// QDOAS ???       if ((pComponent->indexWindow!=ITEM_NONE) && (pComponent->Amf!=NULL) &&
// QDOAS ???           (TabFeno[pComponent->indexWindow].refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC))
// QDOAS ??? //           || (pComponent->ResCol==(double)0.)))
// QDOAS ???        {
// QDOAS ???         // Reinitialize temporary variables
// QDOAS ???
// QDOAS ???         pResults=&TabFeno[pComponent->indexWindow].TabCrossResults[pComponent->indexTabCross];
// QDOAS ???         VertR=(double)0.;
// QDOAS ???         FactR=(double)1.;
// QDOAS ???         NAmPm=0;
// QDOAS ???
// QDOAS ???         OutputGetAmf(pResults,TabFeno[pComponent->indexWindow].Zm,TabFeno[pComponent->indexWindow].Tm,&AmfR);
// QDOAS ???
// QDOAS ???         // AM data
// QDOAS ???
// QDOAS ???         if (NResults[0])
// QDOAS ???          {
// QDOAS ???           for (indexResults=firstResults[0];indexResults<firstResults[0]+NResults[0];indexResults++)
// QDOAS ???            Error[indexResults-firstResults[0]+1]=pComponent->Amf[indexResults]/pComponent->Error[indexResults];
// QDOAS ???
// QDOAS ???           // The slope of the straight line passing through slant(AMF) points give the AM mean vertical column
// QDOAS ???
// QDOAS ???           if ((rc=ANALYSE_LinFit(&svd,NResults[0],1,pComponent->Amf+firstResults[0]-1,Error,pComponent->SlntCol+firstResults[0]-1,x))!=ERROR_ID_NO)
// QDOAS ???            break;
// QDOAS ???
// QDOAS ???           // NO2 possible rejection
// QDOAS ???
// QDOAS ???           if ((indexField==PRJCT_RESULTS_NASA_NO2) && ((x[2]<(double)0.) || (x[2]>(double)1.e16)))
// QDOAS ???            FactR=(double)0.75;
// QDOAS ???           else
// QDOAS ???            {
// QDOAS ???             NAmPm++;
// QDOAS ???             VertR+=x[2];
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         // PM data
// QDOAS ???
// QDOAS ???         if (NResults[1])
// QDOAS ???          {
// QDOAS ???           // Rebuild errors on slant columns
// QDOAS ???
// QDOAS ???           for (indexResults=firstResults[1];indexResults<firstResults[1]+NResults[1];indexResults++)
// QDOAS ???            Error[indexResults-firstResults[1]+1]=pComponent->Amf[indexResults]/pComponent->Error[indexResults];
// QDOAS ???
// QDOAS ???           // The slope of the straight line passing through slant(AMF) points give the PM mean vertical column
// QDOAS ???
// QDOAS ???           if ((rc=ANALYSE_LinFit(&svd,NResults[1],1,pComponent->Amf+firstResults[1]-1,Error,pComponent->SlntCol+firstResults[1]-1,x))!=ERROR_ID_NO)
// QDOAS ???            break;
// QDOAS ???
// QDOAS ???           // NO2 possible rejection
// QDOAS ???
// QDOAS ???           if ((indexField==PRJCT_RESULTS_NASA_NO2) && ((x[2]<(double)0.) || (x[2]>(double)1.e16)))
// QDOAS ???            FactR=(double)1.25;
// QDOAS ???           else
// QDOAS ???            {
// QDOAS ???             NAmPm++;
// QDOAS ???             VertR+=x[2];
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         pComponent->ResCol=(NAmPm)?(VertR*FactR*AmfR)/NAmPm:(double)0.;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   if (Error!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("OutputResidualColumns ","Error",Error,1);
// QDOAS ???
// QDOAS ???   ANALYSE_SvdFree("OutputResidualColumns",&svd);
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OutputSlope
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Calculate the slope for NO2 rejection
// QDOAS ??? //
// QDOAS ??? // INPUT         indexAmPm          flag to separate AM and PM data
// QDOAS ??? //               Phi_min, Phi_Max   range of AMF
// QDOAS ??? //
// QDOAS ??? // RETURN        the slope coefficient
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? double OutputSlope(INDEX indexAmPm,double Phi_min,double Phi_max)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   double SumX,SumX2,SumXY,SumY,SumW,
// QDOAS ???         *TimeDec,tmpError,tmpAmf;
// QDOAS ???   NASA_COMPONENTS *pComponent;
// QDOAS ???   INT Cptr;
// QDOAS ???   INDEX i;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pComponent=&OUTPUT_nasaResults.components[PRJCT_RESULTS_NASA_NO2];
// QDOAS ???   SumX=SumX2=SumXY=SumY=SumW=(double)0.;
// QDOAS ???   TimeDec=OUTPUT_nasaResults.TimeDec;
// QDOAS ???   Cptr=0;
// QDOAS ???
// QDOAS ???   if ((pComponent->indexWindow!=ITEM_NONE) && (pComponent->Amf!=NULL) && OUTPUT_nasaNResults)
// QDOAS ???    {
// QDOAS ???     // Browse results
// QDOAS ???
// QDOAS ???     for (i=0,Cptr=0;i<OUTPUT_nasaNResults;i++)
// QDOAS ???      {
// QDOAS ???       tmpAmf=pComponent->Amf[i];
// QDOAS ???       tmpError=pComponent->Error[i]*pComponent->Error[i];
// QDOAS ???
// QDOAS ???       // Accumulations
// QDOAS ???
// QDOAS ???       if (((!indexAmPm && (TimeDec[i]<=THRD_localNoon)) ||
// QDOAS ???            (indexAmPm && (TimeDec[i]>THRD_localNoon))) &&
// QDOAS ???            (tmpAmf>=Phi_min) && (tmpAmf<=Phi_max))
// QDOAS ???        {
// QDOAS ???         SumW +=(double) tmpError;
// QDOAS ???         SumX +=(double) tmpError/tmpAmf;
// QDOAS ???         SumY +=(double) tmpError*(pComponent->SlntCol[i]+pComponent->ResCol)/tmpAmf;
// QDOAS ???         SumX2+=(double) tmpError/(tmpAmf*tmpAmf);
// QDOAS ???         SumXY+=(double) tmpError*(pComponent->SlntCol[i]+pComponent->ResCol)/(tmpAmf*tmpAmf);
// QDOAS ???
// QDOAS ???         Cptr++;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return (Cptr<3)?(double)0.:(double)fabs((SumW*SumXY-SumX*SumY)/(SumW*SumX2-SumX*SumX));
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OUTPUT_SaveNasaAmes
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       save results in NASA-AMES format
// QDOAS ??? //
// QDOAS ??? // RETURN        Non zero value return code if the function failed
// QDOAS ??? //               ERROR_ID_NO on success
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OUTPUT_SaveNasaAmes(void)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   PROJECT             *pProject;                                                // pointer to project data
// QDOAS ???   PRJCT_INSTRUMENTAL  *pInstrumental;                                           // pointer to instrumental part of project
// QDOAS ???   PRJCT_RESULTS_NASA  *pResults;                                                // pointer to results part of project
// QDOAS ???   NASA_COMPONENTS     *pComponent;                                              // pointer to one of components to take into account for NASA-AMES results
// QDOAS ???   OBSERVATION_SITE    *pSite;                                                   // pointer to observation site
// QDOAS ???   UCHAR               *fileNamePtr,                                             // character pointers used for building output file name
// QDOAS ???                        resFile[MAX_ITEM_TEXT_LEN+1];                            // complete results file name
// QDOAS ???   FILE                *resuFp;                                                  // file pointer
// QDOAS ???   INDEX                indexResults,                                            // browse results
// QDOAS ???                        indexField,                                              // browse interesting components
// QDOAS ???                        indexSite,
// QDOAS ???                        indexAmPm,                                               // browse AM/PM results
// QDOAS ???                        ind90[2],ind93[2];                                       // index for color indexes calculation
// QDOAS ???   double              *Zm,*Tm,*TDet,*TimeDec,*Cic,                              // substitution vectors
// QDOAS ???                        ZmMin[2],ZmMax[2],Ci90[2],Ci93[2],Ci,                    // color indexes calculation
// QDOAS ???                        ZmMean[2],TmMean[2],TDetMean[2],                         // AM/PM auxiliary results mean
// QDOAS ???                        Vert[PRJCT_RESULTS_NASA_MAX][2],                         // AM/PM means computed on vertical columns
// QDOAS ???                        Error[PRJCT_RESULTS_NASA_MAX][2],                        // AM/PM means computed on errors bars
// QDOAS ???                        Error2[PRJCT_RESULTS_NASA_MAX][2],                       // AM/PM sums of squares of errors bars
// QDOAS ???                        Amf[PRJCT_RESULTS_NASA_MAX][2],                          // AM/PM means computed on AMF
// QDOAS ???                        tmpError,tmpAmf,tmpTm;                                   // temporary variables
// QDOAS ???   INT                  NResults[2],NAmPm;                                       // AM/PM number of results taken into account
// QDOAS ???   RC                   rc;                                                      // return code
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pProject=(PROJECT *)&OUTPUT_nasaResults.project;
// QDOAS ???   pInstrumental=(PRJCT_INSTRUMENTAL *)&pProject->instrumental;
// QDOAS ???   pResults=(PRJCT_RESULTS_NASA *)&pProject->nasaResults;
// QDOAS ???   indexSite=SITES_GetIndex(pInstrumental->observationSite);
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   pSite=(indexSite!=ITEM_NONE)?&SITES_itemList[indexSite]:NULL;
// QDOAS ???
// QDOAS ???   if (outputNasaFlag && pResults->nasaFlag && OUTPUT_nasaResults.oldJulianDay && OUTPUT_nasaNResults && !OutputResidualColumns())
// QDOAS ???    {
// QDOAS ???     // All variables initializations
// QDOAS ???
// QDOAS ???     Zm=OUTPUT_nasaResults.Zm;
// QDOAS ???     Tm=OUTPUT_nasaResults.Tm;
// QDOAS ???     TDet=OUTPUT_nasaResults.TDet;
// QDOAS ???     TimeDec=OUTPUT_nasaResults.TimeDec;
// QDOAS ???     Cic=OUTPUT_nasaResults.Cic;
// QDOAS ???
// QDOAS ???     for (indexAmPm=0;indexAmPm<2;indexAmPm++)
// QDOAS ???      {
// QDOAS ???       // Auxiliary AM/PM variables initialization
// QDOAS ???
// QDOAS ???       NResults[indexAmPm]=0;
// QDOAS ???       ZmMean[indexAmPm]=TmMean[indexAmPm]=TDetMean[indexAmPm]=(double)0.;
// QDOAS ???
// QDOAS ???       // Primary AM/PM variables initialization
// QDOAS ???
// QDOAS ???       for (indexField=0;indexField<PRJCT_RESULTS_NASA_MAX;indexField++)
// QDOAS ???        Vert[indexField][indexAmPm]=Error[indexField][indexAmPm]=
// QDOAS ???        Error2[indexField][indexAmPm]=Amf[indexField][indexAmPm]=(double)0.;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // NO2 rejection test
// QDOAS ???
// QDOAS ???     if (pResults->no2RejectionFlag)
// QDOAS ???      {
// QDOAS ???       if ((OutputSlope(0,(double)4.,(double)22.)>=15.e15) && (OutputSlope(0,(double)10.,(double)22.)>=15.e15))  // AM
// QDOAS ???        Vert[PRJCT_RESULTS_NASA_NO2][0]=(double)9999.;
// QDOAS ???       if ((OutputSlope(1,(double)4.,(double)22.)>=15.e15) && (OutputSlope(1,(double)10.,(double)22.)>=15.e15))  // PM
// QDOAS ???        Vert[PRJCT_RESULTS_NASA_NO2][1]=(double)9999.;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Browse results
// QDOAS ???
// QDOAS ???     ZmMin[0]=ZmMin[1]=(double)99.;
// QDOAS ???     ZmMax[0]=ZmMax[1]=(double)0.;
// QDOAS ???     Ci90[0]=Ci90[1]=Ci93[0]=Ci93[1]=(double)0.;
// QDOAS ???     ind90[0]=ind90[1]=ind93[0]=ind93[1]=0;
// QDOAS ???
// QDOAS ???     for (indexResults=0;indexResults<OUTPUT_nasaNResults;indexResults++)
// QDOAS ???      {
// QDOAS ???       indexAmPm=(TimeDec[indexResults]<=THRD_localNoon)?0:1;
// QDOAS ???
// QDOAS ???       // Search for indexes of results to use for color indexes calculation
// QDOAS ???
// QDOAS ???       if (Zm[indexResults]<ZmMin[indexAmPm])
// QDOAS ???        ZmMin[indexAmPm]=(double)Zm[indexResults];
// QDOAS ???       else if (Zm[indexResults]>ZmMax[indexAmPm])
// QDOAS ???        ZmMax[indexAmPm]=(double)Zm[indexResults];
// QDOAS ???
// QDOAS ???       if ((Zm[indexResults]<90.) && (Zm[indexResults]>Zm[ind90[indexAmPm]]))
// QDOAS ???        ind90[indexAmPm]=indexResults;
// QDOAS ???       if ((Zm[indexResults]<93.) && (Zm[indexResults]>Zm[ind93[indexAmPm]]))
// QDOAS ???        ind93[indexAmPm]=indexResults;
// QDOAS ???
// QDOAS ???       if ((Zm[indexResults]>=(double)87.) && (Zm[indexResults]<=(double)91.))
// QDOAS ???        {
// QDOAS ???         NResults[indexAmPm]++;
// QDOAS ???
// QDOAS ???         // Auxiliary results accumulation
// QDOAS ???
// QDOAS ???         ZmMean[indexAmPm]+=Zm[indexResults];
// QDOAS ???         TmMean[indexAmPm]+=Tm[indexResults];
// QDOAS ???         TDetMean[indexAmPm]+=TDet[indexResults];
// QDOAS ???
// QDOAS ???         // Primary results accumulation
// QDOAS ???
// QDOAS ???         for (indexField=0;indexField<PRJCT_RESULTS_NASA_MAX;indexField++)
// QDOAS ???          {
// QDOAS ???           pComponent=&OUTPUT_nasaResults.components[indexField];
// QDOAS ???
// QDOAS ???           if ((pComponent->indexWindow!=ITEM_NONE) && (Vert[indexField][indexAmPm]!=(double)9999.))
// QDOAS ???            {
// QDOAS ???             tmpError=pComponent->Error[indexResults];
// QDOAS ???             tmpAmf=(pComponent->Amf!=NULL)?pComponent->Amf[indexResults]:(double)1.;
// QDOAS ???
// QDOAS ???             Error[indexField][indexAmPm]+=tmpError;
// QDOAS ???             Error2[indexField][indexAmPm]+=tmpError*tmpError;
// QDOAS ???             Vert[indexField][indexAmPm]+=(pComponent->SlntCol[indexResults]+pComponent->ResCol)*tmpError*tmpError/tmpAmf;
// QDOAS ???             Amf[indexField][indexAmPm]+=(double)tmpAmf;
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Means computations
// QDOAS ???
// QDOAS ???     if (NResults[0] || NResults[1])
// QDOAS ???      {
// QDOAS ???       for (indexAmPm=NAmPm=0;indexAmPm<2;indexAmPm++)
// QDOAS ???        {
// QDOAS ???         // Color indexes
// QDOAS ???
// QDOAS ???         if ((ZmMin[indexAmPm]<=(double)90.) && (ZmMax[indexAmPm]>=(double)93.))
// QDOAS ???          {
// QDOAS ???           Ci=(90.-Zm[ind90[indexAmPm]])/(Zm[ind90[indexAmPm]+1]-Zm[ind90[indexAmPm]]);
// QDOAS ???           Ci90[indexAmPm]=Cic[ind90[indexAmPm]]+(Cic[ind90[indexAmPm]+1]-Cic[ind90[indexAmPm]])*Ci;
// QDOAS ???           Ci=(93.-Zm[ind93[indexAmPm]])/(Zm[ind93[indexAmPm]+1]-Zm[ind93[indexAmPm]]);
// QDOAS ???           Ci93[indexAmPm]=Cic[ind93[indexAmPm]]+(Cic[ind93[indexAmPm]+1]-Cic[ind93[indexAmPm]])*Ci;
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         if (NResults[indexAmPm])
// QDOAS ???          {
// QDOAS ???           NAmPm++;
// QDOAS ???
// QDOAS ???           // Auxiliary results
// QDOAS ???
// QDOAS ???           ZmMean[indexAmPm]/=NResults[indexAmPm];
// QDOAS ???           TmMean[indexAmPm]/=NResults[indexAmPm];
// QDOAS ???           TDetMean[indexAmPm]/=NResults[indexAmPm];
// QDOAS ???
// QDOAS ???           // Primary results
// QDOAS ???
// QDOAS ???           for (indexField=0;indexField<PRJCT_RESULTS_NASA_MAX;indexField++)
// QDOAS ???            {
// QDOAS ???             pComponent=&OUTPUT_nasaResults.components[indexField];
// QDOAS ???
// QDOAS ???             if ((pComponent->indexWindow!=ITEM_NONE) && (Vert[indexField][indexAmPm]!=(double)9999.))
// QDOAS ???              {
// QDOAS ???               Vert[indexField][indexAmPm]/=Error2[indexField][indexAmPm];
// QDOAS ???               Error[indexField][indexAmPm]/=Error2[indexField][indexAmPm];
// QDOAS ???               Amf[indexField][indexAmPm]/=NResults[indexAmPm];                  // NB : AmfMean==1 if (pComponent->Amf==NULL)
// QDOAS ???              }
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Build output file name as following : <output path><site abbrev><YY><MM>.<NASA-AMES extension>
// QDOAS ???
// QDOAS ???       tmpTm=(TmMean[0]+TmMean[1])/NAmPm;
// QDOAS ???
// QDOAS ???       FILES_RebuildFileName(resFile,pResults->path,1);                          // get complete output path and file name
// QDOAS ???
// QDOAS ???       if ((fileNamePtr=strrchr(resFile,PATH_SEP))==NULL)                        // extract output file name without path
// QDOAS ???        fileNamePtr=resFile;
// QDOAS ???       else
// QDOAS ???        fileNamePtr++;
// QDOAS ???
// QDOAS ???       if (!strlen(fileNamePtr))
// QDOAS ???
// QDOAS ???        sprintf(fileNamePtr,"%s%02d%02d.UVV",
// QDOAS ???              ((pSite!=NULL)?(UCHAR *)pSite->abbrev:(UCHAR *)"XX"),
// QDOAS ???                ZEN_FNCaljye(&tmpTm)%100,
// QDOAS ???                ZEN_FNCaljmon(ZEN_FNCaljye(&tmpTm),ZEN_FNCaljda(&tmpTm)));
// QDOAS ???
// QDOAS ???       // Write header
// QDOAS ???
// QDOAS ???       if ((OUTPUT_nasaBuf!=NULL) && ((resuFp=fopen(resFile,"rt"))==NULL))
// QDOAS ???        rc=OutputNasaAmesHeader(tmpTm,resFile);
// QDOAS ???       if (resuFp!=NULL)
// QDOAS ???        fclose(resuFp);
// QDOAS ???
// QDOAS ???       // Write results
// QDOAS ???
// QDOAS ???       if (!rc && ((resuFp=fopen(resFile,"a+t"))!=NULL))
// QDOAS ???        {
// QDOAS ???         // Auxiliary results
// QDOAS ???
// QDOAS ???         fprintf(resuFp, "%-5d %#4d %#2d %#2d %#4d %#4d %#4d %#3d %#2d %#6d %#6d\n",
// QDOAS ???
// QDOAS ???                    // Julian day
// QDOAS ???
// QDOAS ???                    (int) ZEN_FNCaljda(&tmpTm),
// QDOAS ???
// QDOAS ???                    // Measurement date UT
// QDOAS ???
// QDOAS ???                    (int) ZEN_FNCaljye  ( &tmpTm ),
// QDOAS ???                    (int) ZEN_FNCaljmon ( ZEN_FNCaljye(&tmpTm), ZEN_FNCaljda(&tmpTm) ),
// QDOAS ???                    (int) ZEN_FNCaljday ( ZEN_FNCaljye(&tmpTm), ZEN_FNCaljda(&tmpTm) ),
// QDOAS ???
// QDOAS ???                    // Latitude and Longitude for Observation Site in decimal degrees ( North and East positive )
// QDOAS ???
// QDOAS ???                    (int)((pSite!=NULL)?(double)(pSite->latitude*100.+0.5):9999),
// QDOAS ???                    (int)((pSite!=NULL)?(double)(pSite->longitude*100.+0.5):9999),
// QDOAS ???
// QDOAS ???                    // Auxiliary variables
// QDOAS ???
// QDOAS ???                    (int)((double)100.*(ZmMean[0]+ZmMean[1])/NAmPm + 0.5),
// QDOAS ???                    (int)((double)(TDetMean[0]+TDetMean[1])/NAmPm + 273.5),
// QDOAS ???
// QDOAS ???                    (int)  OUTPUT_nasaResults.SkyObs,
// QDOAS ???                    (int)((double)OUTPUT_nasaResults.components[PRJCT_RESULTS_NASA_NO2].ResCol/OUTPUT_nasaResults.components[PRJCT_RESULTS_NASA_NO2].VrtColFact+0.5),
// QDOAS ???                    (int)((double)OUTPUT_nasaResults.components[PRJCT_RESULTS_NASA_O3].ResCol/OUTPUT_nasaResults.components[PRJCT_RESULTS_NASA_O3].VrtColFact+0.5));
// QDOAS ???
// QDOAS ???         // Primary results
// QDOAS ???
// QDOAS ???         for (indexAmPm=0;indexAmPm<2;indexAmPm++)
// QDOAS ???          {
// QDOAS ???           for (indexField=0;indexField<PRJCT_RESULTS_NASA_MAX;indexField++)
// QDOAS ???            {
// QDOAS ???             pComponent=&OUTPUT_nasaResults.components[indexField];
// QDOAS ???
// QDOAS ???             if (pComponent->indexWindow!=ITEM_NONE)
// QDOAS ???              {
// QDOAS ???               switch(indexField)
// QDOAS ???                {
// QDOAS ???              // ---------------------------------------------------------------
// QDOAS ???                 case PRJCT_RESULTS_NASA_NO2 :
// QDOAS ???                 case PRJCT_RESULTS_NASA_O3 :
// QDOAS ???
// QDOAS ???                  if ((NResults[indexAmPm]) && (Vert[indexField][indexAmPm]!=(double)9999.))
// QDOAS ???
// QDOAS ???                   fprintf(resuFp,"%#6ld %#6ld %#6ld %#6ld ",
// QDOAS ???                      (LONG)(Vert[indexField][indexAmPm]/pComponent->VrtColFact+0.5),
// QDOAS ???                      (LONG)(Vert[indexField][indexAmPm]*Amf[indexField][indexAmPm]/pComponent->SlntColFact+0.5),
// QDOAS ???                      (LONG)(Error[indexField][indexAmPm]/pComponent->SlntErrFact+0.5),
// QDOAS ???                      (LONG)(Amf[indexField][indexAmPm]/pComponent->AmfFact+0.5));
// QDOAS ???
// QDOAS ???                  else
// QDOAS ???                   fprintf(resuFp,"9999 9999 9999 9999 ");
// QDOAS ???
// QDOAS ???                 break;
// QDOAS ???              // ---------------------------------------------------------------
// QDOAS ???                 case PRJCT_RESULTS_NASA_OCLO :
// QDOAS ???                 case PRJCT_RESULTS_NASA_BRO :
// QDOAS ???
// QDOAS ???                  if (NResults[indexAmPm])
// QDOAS ???
// QDOAS ???                   fprintf(resuFp,"%#6ld %#6ld ",
// QDOAS ???
// QDOAS ???                     (LONG)(Vert[indexField][indexAmPm]*Amf[indexField][indexAmPm]/pComponent->SlntColFact+0.5),
// QDOAS ???                     (LONG)(Error[indexField][indexAmPm]/pComponent->SlntErrFact+0.5));
// QDOAS ???
// QDOAS ???                  else
// QDOAS ???
// QDOAS ???                   fprintf(resuFp,"9999 9999 ");
// QDOAS ???
// QDOAS ???                 break;
// QDOAS ???              // ---------------------------------------------------------------
// QDOAS ???                }
// QDOAS ???              }
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           fprintf(resuFp,"%#6ld %#6ld\n",
// QDOAS ???                  (Ci90[indexAmPm]!=(double)0.)?(LONG)(Ci90[indexAmPm]+0.5):9999,
// QDOAS ???                  (Ci93[indexAmPm]!=(double)0.)?(LONG)(Ci93[indexAmPm]+0.5):9999);
// QDOAS ???          }
// QDOAS ???         fclose(resuFp);
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      OUTPUT_SaveResults
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       save all results (ASC/BIN and NASA-AMES formats)
// QDOAS ??? //
// QDOAS ??? // INPUT         pEngineContext   structure including information on the current record
// QDOAS ??? //
// QDOAS ??? // RETURN        Non zero value return code if the function failed
// QDOAS ??? //               ERROR_ID_NO on success
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OUTPUT_SaveResults(ENGINE_CONTEXT *pEngineContext)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   CROSS_RESULTS *pTabCrossResults;
// QDOAS ???   INDEX indexFeno,indexTabCross,i;
// QDOAS ???   double *Spectrum;
// QDOAS ???   FENO *pTabFeno;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // AMF computation
// QDOAS ???
// QDOAS ???   if (OUTPUT_AmfSpace!=NULL)
// QDOAS ???
// QDOAS ???    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
// QDOAS ???     {
// QDOAS ???      pTabFeno=&TabFeno[indexFeno];
// QDOAS ???
// QDOAS ???      if ((THRD_id!=THREAD_TYPE_KURUCZ) && !pTabFeno->hidden && !pTabFeno->rcKurucz)
// QDOAS ???       {
// QDOAS ???        for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
// QDOAS ???         {
// QDOAS ???          pTabCrossResults=&pTabFeno->TabCrossResults[indexTabCross];
// QDOAS ???
// QDOAS ???          if (pTabCrossResults->indexAmf!=ITEM_NONE)
// QDOAS ???           {
// QDOAS ???            if (OutputGetAmf(pTabCrossResults,pEngineContext->Zm,pEngineContext->Tm,&pTabCrossResults->Amf))
// QDOAS ???             THRD_Error(ERROR_TYPE_WARNING,ERROR_ID_AMF,"OutputSaveResults ",pEngineContext->Zm,OUTPUT_AmfSpace[pTabCrossResults->indexAmf].amfFileName);
// QDOAS ???            else if (pTabCrossResults->Amf!=(double)0.)
// QDOAS ???             {
// QDOAS ???              pTabCrossResults->VrtCol=(pTabCrossResults->SlntCol+pTabCrossResults->ResCol)/pTabCrossResults->Amf;
// QDOAS ???              pTabCrossResults->VrtErr=pTabCrossResults->SlntErr/pTabCrossResults->Amf;
// QDOAS ???             }
// QDOAS ???           }
// QDOAS ???         }
// QDOAS ???       }
// QDOAS ???     }
// QDOAS ???
// QDOAS ???   // Rebuild spectrum for fluxes and color indexes computation
// QDOAS ???
// QDOAS ???   if ((pEngineContext->NSomme!=0) && (pEngineContext->TotalExpTime!=(double)0.))
// QDOAS ???    {
// QDOAS ???     Spectrum=(double *)pEngineContext->spectrum;
// QDOAS ???
// QDOAS ???     for (i=0;i<NDET;i++)
// QDOAS ???      Spectrum[i]*=(double)pEngineContext->NSomme/pEngineContext->TotalExpTime;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if (outputNbRecords<pEngineContext->recordNumber)
// QDOAS ???    OutputSaveRecord(pEngineContext,(THRD_id==THREAD_TYPE_ANALYSIS)?0:1);
// QDOAS ???
// QDOAS ???   // Results safe keeping
// QDOAS ???
// QDOAS ???   if (outputNasaFlag)                                                           // SAVE results in ASCII format
// QDOAS ???    rc=OutputStoreNasaAmes(pEngineContext);                                           // STORE results for future NASA-AMES means computation
// QDOAS ???
// QDOAS ???   pEngineContext->lastSavedRecord=pEngineContext->indexRecord;
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ====================
// QDOAS ??? // RESOURCES MANAGEMENT
// QDOAS ??? // ====================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        OUTPUT_LocalAlloc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Allocate and initialize buffers for the records to output
// QDOAS ??? //
// QDOAS ??? // INPUT           pEngineContext   structure including information on the current project
// QDOAS ??? //
// QDOAS ??? // RETURN          ERROR_ID_ALLOC if one of the buffer allocation failed
// QDOAS ??? //                 ERROR_ID_NO in case of success
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OUTPUT_LocalAlloc(ENGINE_CONTEXT *pEngineContext)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   PROJECT             *pProject;                     // pointer to project data
// QDOAS ???   PRJCT_RESULTS_ASCII *pResults;                     // pointer to results part of project
// QDOAS ???   PRJCT_RESULTS_FIELDS *pField;
// QDOAS ???   INT newRecordNumber,n;
// QDOAS ???   INDEX indexField,i;
// QDOAS ???   RC rc;                                                                        // return code
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   newRecordNumber=pEngineContext->recordNumber;
// QDOAS ???
// QDOAS ???   pProject=(PROJECT *)&pEngineContext->project;
// QDOAS ???   pResults=(PRJCT_RESULTS_ASCII *)&pProject->asciiResults;
// QDOAS ???
// QDOAS ???   if (pResults->analysisFlag || pResults->calibFlag)
// QDOAS ???    {
// QDOAS ???     if (!newRecordNumber || (outputMaxRecords<newRecordNumber))
// QDOAS ???      {
// QDOAS ???       if (outputRecords!=NULL)
// QDOAS ???        MEMORY_ReleaseBuffer("OUTPUT_LocalAlloc","outputRecords",outputRecords);
// QDOAS ???
// QDOAS ???       for (indexField=0;indexField<outputNbFields;indexField++)
// QDOAS ???        if (outputColumns[indexField]!=NULL)
// QDOAS ???         MEMORY_ReleaseBuffer("OUTPUT_LocalAlloc",outputFields[indexField].fieldName,outputColumns[indexField]);
// QDOAS ???
// QDOAS ???       memset(outputColumns,0,sizeof(UCHAR *)*MAX_FIELDS);
// QDOAS ???       outputRecords=NULL;
// QDOAS ???       outputMaxRecords=0;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Allocate new buffers
// QDOAS ???
// QDOAS ???     if (newRecordNumber)
// QDOAS ???      {
// QDOAS ???       if ((outputMaxRecords<newRecordNumber) && ((outputRecords=(OUTPUT_INFO *)MEMORY_AllocBuffer("OUTPUT_LocalAlloc","outputRecords",newRecordNumber,sizeof(OUTPUT_INFO),0,MEMORY_TYPE_STRUCT))==NULL))
// QDOAS ???        THRD_Error(ERROR_TYPE_FATAL,(rc=ERR_MEMORY_ALLOC_OUTPUT),"OUTPUT_LocalAlloc (outputRecords) ");
// QDOAS ???       else
// QDOAS ???        {
// QDOAS ???         memset(outputRecords,0,sizeof(OUTPUT_INFO)*newRecordNumber);
// QDOAS ???
// QDOAS ???         for (indexField=0;(indexField<outputNbFields) && !rc;indexField++)
// QDOAS ???          {
// QDOAS ???           pField=&outputFields[indexField];
// QDOAS ???
// QDOAS ???           if (pField->fieldDim1==ITEM_NONE)
// QDOAS ???            n=newRecordNumber;
// QDOAS ???           else if (pField->fieldDim2==ITEM_NONE)
// QDOAS ???            n=pField->fieldDim1;
// QDOAS ???           else
// QDOAS ???            n=pField->fieldDim1*pField->fieldDim2;
// QDOAS ???
// QDOAS ???           if ((outputMaxRecords<newRecordNumber) &&
// QDOAS ???              ((outputColumns[indexField]=(UCHAR *)MEMORY_AllocBuffer("OUTPUT_LocalAlloc","outputColumns",n,pField->fieldSize,0,pField->fieldType))==NULL))
// QDOAS ???            THRD_Error(ERROR_TYPE_FATAL,(rc=ERR_MEMORY_ALLOC_OUTPUT),"OUTPUT_LocalAlloc (outputColumns) ");
// QDOAS ???           else
// QDOAS ???
// QDOAS ???            switch(pField->fieldType)
// QDOAS ???             {
// QDOAS ???           // -------------------------------------------------------------------------
// QDOAS ???              case MEMORY_TYPE_STRING :
// QDOAS ???               memset(outputColumns[indexField],0,n*pField->fieldSize);
// QDOAS ???              break;
// QDOAS ???           // -------------------------------------------------------------------------
// QDOAS ???              case MEMORY_TYPE_USHORT :
// QDOAS ???               for (i=0;i<n;i++)
// QDOAS ???                ((USHORT *)outputColumns[indexField])[i]=9999;
// QDOAS ???              break;
// QDOAS ???           // -------------------------------------------------------------------------
// QDOAS ???              case MEMORY_TYPE_INT :
// QDOAS ???               for (i=0;i<n;i++)
// QDOAS ???                ((int *)outputColumns[indexField])[i]=9999;
// QDOAS ???              break;
// QDOAS ???           // -------------------------------------------------------------------------
// QDOAS ???              case MEMORY_TYPE_FLOAT :
// QDOAS ???               for (i=0;i<n;i++)
// QDOAS ???                ((float *)outputColumns[indexField])[i]=9999.;
// QDOAS ???              break;
// QDOAS ???           // -------------------------------------------------------------------------
// QDOAS ???              case MEMORY_TYPE_DOUBLE :
// QDOAS ???               for (i=0;i<n;i++)
// QDOAS ???                ((double *)outputColumns[indexField])[i]=(double)9999.;
// QDOAS ???              break;
// QDOAS ???           // -------------------------------------------------------------------------
// QDOAS ???             }
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       if ((outputMaxRecords<newRecordNumber) && !rc)
// QDOAS ???        outputMaxRecords=newRecordNumber;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     outputNbRecords=0;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        OUTPUT_Alloc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Allocate and initialize general use buffers for output
// QDOAS ??? //
// QDOAS ??? // RETURN          ERROR_ID_ALLOC if one of the buffer allocation failed
// QDOAS ??? //                 ERROR_ID_FILE_EMPTY if the file in NASA-AMES format is empty
// QDOAS ??? //                 ERROR_ID_NO in case of success
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC OUTPUT_Alloc(void)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   NASA_COMPONENTS *components;
// QDOAS ???   UCHAR *nasaBufLine;
// QDOAS ???   INDEX indexChar;
// QDOAS ???   FILE *nasaFp;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   components=OUTPUT_nasaResults.components;
// QDOAS ???   memset(OUTPUT_nasaBufTemp,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???   strcpy(components[PRJCT_RESULTS_NASA_NO2].symbolName,"NO2");
// QDOAS ???   strcpy(components[PRJCT_RESULTS_NASA_O3].symbolName,"O3");
// QDOAS ???   strcpy(components[PRJCT_RESULTS_NASA_OCLO].symbolName,"OCLO");
// QDOAS ???   strcpy(components[PRJCT_RESULTS_NASA_BRO].symbolName,"BRO");
// QDOAS ???
// QDOAS ???   outputNasaFlag=0;
// QDOAS ???   nasaFp=NULL;
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Allocate buffers resp. for NASA-AMES header file and for AMF
// QDOAS ???
// QDOAS ???   if ((outputFields=(PRJCT_RESULTS_FIELDS *)MEMORY_AllocBuffer("OUTPUT_Alloc ","outputFields",MAX_FIELDS,sizeof(PRJCT_RESULTS_FIELDS),0,MEMORY_TYPE_STRUCT))==NULL)
// QDOAS ???    THRD_Error(ERROR_TYPE_FATAL,(rc=ERR_MEMORY_ALLOC_OUTPUT),"OUTPUT_Alloc (outputFields) ");
// QDOAS ???   else if ((outputColumns=(UCHAR **)MEMORY_AllocBuffer("OUTPUT_Alloc","outputColumns",MAX_FIELDS,sizeof(UCHAR *),0,MEMORY_TYPE_PTR))==NULL)
// QDOAS ???    THRD_Error(ERROR_TYPE_FATAL,(rc=ERR_MEMORY_ALLOC_OUTPUT),"OUTPUT_Alloc (outputColumns) ");
// QDOAS ???   else if ((OUTPUT_AmfSpace=(AMF_SYMBOL *)MEMORY_AllocBuffer("OUTPUT_Alloc ","OUTPUT_AmfSpace",MAX_SYMB,sizeof(AMF_SYMBOL),0,MEMORY_TYPE_STRUCT))==NULL)
// QDOAS ???    THRD_Error(ERROR_TYPE_FATAL,(rc=ERR_MEMORY_ALLOC_OUTPUT),"OUTPUT_Alloc (1) ");
// QDOAS ???   else if ((nasaFp=fopen(OUTPUT_nasaFile,"rt"))!=NULL)
// QDOAS ???    {
// QDOAS ???     memset(outputFields,0,sizeof(PRJCT_RESULTS_FIELDS)*MAX_FIELDS);
// QDOAS ???     memset(outputColumns,0,sizeof(UCHAR *)*MAX_FIELDS);
// QDOAS ???
// QDOAS ???     outputNbDataSet=0;
// QDOAS ???     outputNbFields=0;
// QDOAS ???     outputNasaFlag=1;
// QDOAS ???
// QDOAS ???     if (!(OUTPUT_nasaBufSize=(INT)STD_FileLength(nasaFp)))
// QDOAS ???      rc=ERROR_SetLast("OUTPUT_Alloc",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,OUTPUT_nasaFile);
// QDOAS ???     else if ((OUTPUT_nasaBuf=(UCHAR *)MEMORY_AllocBuffer("OUTPUT_Alloc ","OUTPUT_nasaBuf",OUTPUT_nasaBufSize,1,0,MEMORY_TYPE_STRING))==NULL)
// QDOAS ???      THRD_Error(ERROR_TYPE_FATAL,(rc=ERR_MEMORY_ALLOC_OUTPUT),"OUTPUT_Alloc (2) ");
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???       // NASA-AMES header file read out
// QDOAS ???
// QDOAS ???       fread(OUTPUT_nasaBuf,OUTPUT_nasaBufSize,1,nasaFp);
// QDOAS ???
// QDOAS ???       // Determine size of header in terms of bytes and lines
// QDOAS ???
// QDOAS ???       for (indexChar=OUTPUT_nasaBufNLines=0;indexChar<OUTPUT_nasaBufSize;indexChar++)
// QDOAS ???        if (OUTPUT_nasaBuf[indexChar]==(UCHAR)0x0A)
// QDOAS ???         {
// QDOAS ???          OUTPUT_nasaBufNLines++;     // find a new line
// QDOAS ???          OUTPUT_nasaBufSize--;       // because of CR (0x0D)
// QDOAS ???         }
// QDOAS ???
// QDOAS ???       // Scaling factors read out
// QDOAS ???
// QDOAS ???       if ((nasaBufLine=strstr(OUTPUT_nasaBuf,"$AM/PM-ENTRIES"))!=NULL)
// QDOAS ???        {
// QDOAS ???         nasaBufLine=strchr(nasaBufLine,0x0A);  // go to the end of line
// QDOAS ???         nasaBufLine++;                         // go to the next line
// QDOAS ???
// QDOAS ???         // NB : assume that AM/PM factors are the same
// QDOAS ???
// QDOAS ???         sscanf(nasaBufLine,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %[^'\n']",
// QDOAS ???                            &components[PRJCT_RESULTS_NASA_NO2].VrtColFact,      // NO2 vertical column
// QDOAS ???                            &components[PRJCT_RESULTS_NASA_NO2].SlntColFact,     // NO2 slant column
// QDOAS ???                            &components[PRJCT_RESULTS_NASA_NO2].SlntErrFact,     // NO2 error bar
// QDOAS ???                            &components[PRJCT_RESULTS_NASA_NO2].AmfFact,         // NO2 air mass factor
// QDOAS ???                            &components[PRJCT_RESULTS_NASA_O3].VrtColFact,       // O3 vertical column
// QDOAS ???                            &components[PRJCT_RESULTS_NASA_O3].SlntColFact,      // O3 slant column
// QDOAS ???                            &components[PRJCT_RESULTS_NASA_O3].SlntErrFact,      // O3 error bar
// QDOAS ???                            &components[PRJCT_RESULTS_NASA_O3].AmfFact,          // O3 air mass factor
// QDOAS ???                            &components[PRJCT_RESULTS_NASA_OCLO].SlntColFact,    // OClO slant column
// QDOAS ???                            &components[PRJCT_RESULTS_NASA_OCLO].SlntErrFact,    // OClO error bar
// QDOAS ???                            &components[PRJCT_RESULTS_NASA_BRO].SlntColFact,     // BrO slant column
// QDOAS ???                            &components[PRJCT_RESULTS_NASA_BRO].SlntErrFact,     // BrO error bar
// QDOAS ???                             OUTPUT_nasaBufTemp);                                // color indexes scaling factors
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   if (OUTPUT_AmfSpace!=NULL)
// QDOAS ???    memset(OUTPUT_AmfSpace,0,sizeof(AMF_SYMBOL)*MAX_SYMB);
// QDOAS ???
// QDOAS ???   if (nasaFp!=NULL)
// QDOAS ???    fclose(nasaFp);
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        OUTPUT_Free
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Release buffer allocated for output
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void OUTPUT_Free(void)
// QDOAS ???  {
// QDOAS ???   if (OUTPUT_nasaBuf!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("OUTPUT_Free ","OUTPUT_nasaBuf",OUTPUT_nasaBuf);
// QDOAS ???   if (OUTPUT_AmfSpace!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("OUTPUT_Free ","OUTPUT_AmfSpace",OUTPUT_AmfSpace);
// QDOAS ???
// QDOAS ???   if (outputFields!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("OUTPUT_Free","outputFields",outputFields);
// QDOAS ???   if (outputColumns!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("OUTPUT_Free","outputColumns",outputColumns);
// QDOAS ???   if (outputRecords!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("OUTPUT_Free","outputRecords",outputRecords);
// QDOAS ???
// QDOAS ???   OUTPUT_nasaBuf=NULL;
// QDOAS ???   OUTPUT_AmfSpace=NULL;
// QDOAS ???   outputFields=NULL;
// QDOAS ???   outputColumns=NULL;
// QDOAS ???   outputRecords=NULL;
// QDOAS ???  }

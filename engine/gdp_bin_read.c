
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  GOME interface (calibrated level 1 data files in the modified IASB-BIRA format)
//  Name of module    :  GDP_BIN_Read.C
//  Creation date     :  First versions exist since 1998 (GWinDOAS)
//  Modified          :  5 november 2002 (possibility to read GOME binary format with WinDOAS)
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
//  ===================
//  ALLOCATION ROUTINES
//  ===================
//
//  GDP_BIN_ReleaseBuffers - release buffers allocated by GOME readout routines;
//
//  =================
//  READ OUT ROUTINES
//  =================
//
//  GDP_BIN_GetRecordNumber - get the record number of a GOME record from the pixel number;
//
//  GdpBinLambda - build a wavelength calibration (irradiance or
//                 earthshine spectra) using a set of coefficients of polynomial;
//
//  GDP_BIN_Set - retrieve information on data saved in the file from the header;
//  GDP_BIN_Read - GOME calibrated level 1 data read out (binary format);
//
//  =============================
//  AUTOMATIC REFERENCE SELECTION
//  =============================
//
//  GdpBinSortGetIndex - get the position in sorted list of a new element (pixel,latitude or SZA);
//  GdpBinSort - sort NADIR records on latitudes or SZA;
//  GdpBinRefLat - search for spectra in the orbit file matching latitudes and SZA conditions;
//  GdpBinRefSza - search for spectra in the orbit file matching SZA conditions only;
//
//  GdpBinBuildRef - build a reference spectrum by averaging a set of spectra
//                   matching latitudes and SZA conditions;
//
//  GdpBinRefSelection - selection of a reference spectrum in the current orbit;
//  GdpBinNewRef - in automatic reference selection, search for reference spectra;
//
//  ========
//  ANALYSIS
//  ========
//
//  GDP_BIN_LoadAnalysis - load analysis parameters depending on the irradiance spectrum;
//
//  ----------------------------------------------------------------------------

// =====================
// INCLUDE USUAL HEADERS
// =====================

#include <math.h>
#include <string.h>
#include <dirent.h>

#include "gdp_bin_read.h"

#include "engine_context.h"
#include "kurucz.h"
#include "stdfunc.h"
#include "mediate.h"
#include "analyse.h"
#include "engine.h"
#include "vector.h"
#include "winthrd.h"
#include "zenithal.h"
#include "output.h"

// ====================
// CONSTANTS DEFINITION
// ====================

#define	OFFSET	                    (double) 500.0
#define	PARAMETER	                 (double)5000.0

// =====================
// STRUCTURES DEFINITION
// =====================

typedef struct _gome_ref_
 {
  INDEX  indexFile;
  INDEX  indexRecord;
  INDEX  pixelNumber;
  INDEX  pixelType;
  double sza;
  double latitude;
  double longitude;
  double szaDist;
  double latDist;
 }
GDP_BIN_REF;

// ================
// GLOBAL VARIABLES
// ================

GOME_ORBIT_FILE GDP_BIN_orbitFiles[MAX_GOME_FILES];                             // list of files for an orbit
INDEX GDP_BIN_currentFileIndex=ITEM_NONE;                                       // index of the current file in the list
static int gdpBinOrbitFilesN=0;                                                 // the total number of files for the current orbit
static int gdpBinTotalRecordNumber;                                             // total number of records for an orbit
static int gdpBinLoadReferenceFlag=0;

// ==========
// PROTOTYPES
// ==========

INDEX GdpBinSortGetIndex(double value,int flag,int listSize,INDEX fileIndex);
void GdpBinSort(INDEX indexRecord,int flag,int listSize,INDEX fileIndex);

/*----------------------------------------------------------------------------*\
**                                EvalPolynom_f
**  Input parameters:
**    X: The point to evaluate the polynom
**    Coefficient: Describe the polynom
**    Grad: grad of the polynom
**  Output parameters:
**  Other interfaces:
**  Description:
**    This function evaluates a polynom of grad Grad described by Coefficient
**    in the value x
**  References:
**  Libraries:
**  Created:    22.3.94
**  Author: 	Diego Loyola, DLR/WT-DA-BS
\*----------------------------------------------------------------------------*/
float EvalPolynom_f(float X, const float *Coefficient, short Grad)
{
    float Result = 0.0, Mult = 1.0;
	short i;

    for (i=0; i<Grad; i++)
	{
        Result += Coefficient[i]*Mult;
        Mult *= X;
    }
    return Result;
}

/*----------------------------------------------------------------------------*\
**                                EvalPolynom_d
**  Input parameters:
**    X: The point to evaluate the polynom
**    Coefficient: Describe the polynom
**    Grad: grad of the polynom
**  Output parameters:
**  Other interfaces:
**  Description:
**    This function evaluates a polynom of grad Grad described by Coefficient
**    in the value x
**  References:
**  Libraries:
**  Created:    22.3.94
**  Author: 	Diego Loyola, DLR/WT-DA-BS
\*----------------------------------------------------------------------------*/
double EvalPolynom_d(double X, const double *Coefficient, short Grad)
{
    double Result = 0.0, Mult = 1.0;
	short i;

    for (i=0; i<Grad; i++)
	{
        Result += Coefficient[i]*Mult;
        Mult *= X;
    }
    return Result;
}

// ===================
// ALLOCATION ROUTINES
// ===================

// -----------------------------------------------------------------------------
// FUNCTION      GDP_BIN_ReleaseBuffers
// -----------------------------------------------------------------------------
// PURPOSE       Release buffers allocated by GOME readout routines
// -----------------------------------------------------------------------------

void GDP_BIN_ReleaseBuffers(void)
 {
  // Declarations

  GOME_ORBIT_FILE *pOrbitFile;
  INDEX gomeOrbitFileIndex;

  for (gomeOrbitFileIndex=0;gomeOrbitFileIndex<MAX_GOME_FILES;gomeOrbitFileIndex++)
   {
    pOrbitFile=&GDP_BIN_orbitFiles[gomeOrbitFileIndex];

    if (pOrbitFile->gdpBinReference!=NULL)
     MEMORY_ReleaseBuffer("gdpBinReleaseBuffers ","gdpBinreference",pOrbitFile->gdpBinReference);
    if (pOrbitFile->gdpBinRefError!=NULL)
     MEMORY_ReleaseBuffer("gdpBinReleaseBuffers ","gdpBinrefError",pOrbitFile->gdpBinRefError);
    if (pOrbitFile->gdpBinCoeff!=NULL)
     MEMORY_ReleaseDVector("gdpBinReleaseBuffers ","gdpBincoeff",pOrbitFile->gdpBinCoeff,0);

    if (pOrbitFile->gdpBinInfo!=NULL)
     MEMORY_ReleaseBuffer("gdpBinReleaseBuffers ","gdpBinInfo",pOrbitFile->gdpBinInfo);
    if (pOrbitFile->gdpBinLatIndex!=NULL)
     MEMORY_ReleaseBuffer("gdpBinReleaseBuffers ","gdpBinLatIndex",pOrbitFile->gdpBinLatIndex);
    if (pOrbitFile->gdpBinLonIndex!=NULL)
     MEMORY_ReleaseBuffer("gdpBinReleaseBuffers ","gdpBinLonIndex",pOrbitFile->gdpBinLonIndex);
    if (pOrbitFile->gdpBinSzaIndex!=NULL)
     MEMORY_ReleaseBuffer("gdpBinReleaseBuffers ","gdpBinSzaIndex",pOrbitFile->gdpBinSzaIndex);
    if (pOrbitFile->gdpBinPixIndex!=NULL)
     MEMORY_ReleaseBuffer("gdpBinReleaseBuffers ","gdpBinPixIndex",pOrbitFile->gdpBinPixIndex);

    memset(pOrbitFile,0,sizeof(GOME_ORBIT_FILE));
    pOrbitFile->gdpBinBandIndex=ITEM_NONE;
   }

  GDP_BIN_currentFileIndex=ITEM_NONE;                                           // index of the current file in the list
  gdpBinOrbitFilesN=0;                                                          // the total number of files for the current orbit
  gdpBinTotalRecordNumber=0;                                                    // total number of records for an orbit
  gdpBinLoadReferenceFlag=0;
 }

// =================
// READ OUT ROUTINES
// =================

// get real year from a SHORT_DATE encoded year
static inline int get_year(int year_in) {
  if (year_in<30) {
    year_in+=(short)2000;
  } else if (year_in<130) {
    year_in+=(short)1900;
  } else if (year_in<1930) {
    year_in+=(short)100;
  }
  return year_in;
}

// -----------------------------------------------------------------------------
// FUNCTION      GDP_BIN_GetRecordNumber
// -----------------------------------------------------------------------------
// PURPOSE       Get the record number of a GOME record from the pixel number
//
// INPUT         pixelNumber the pixel number of the GOME record to search for;
//               fileIndex   index of the file for the current orbit
//
// RETURN        the record number of the GOME record
// -----------------------------------------------------------------------------

INDEX GDP_BIN_GetRecordNumber(int pixelNumber)
 {
  return GdpBinSortGetIndex(pixelNumber,2,GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinHeader.nspectra,GDP_BIN_currentFileIndex)+1;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GdpBinLambda
// -----------------------------------------------------------------------------
// PURPOSE       Build a wavelength calibration (irradiance or
//               earthshine spectra) using a set of coefficients of polynomial
//
// INPUT         indexParam    index of the set of parameters to use
//               fileIndex   index of the file for the current orbit
//
// OUTPUT        lambda        the wavelength calibration
// -----------------------------------------------------------------------------

void GdpBinLambda(double *lambda,int indexParam,INDEX fileIndex)
 {
  // Declarations

  int offset;                                                                   // offset in bytes
  double lambdax;                                                               // wavelength evaluated by polynomial for a given pixel
  INDEX i,j;
  GOME_ORBIT_FILE *pOrbitFile;

  pOrbitFile=&GDP_BIN_orbitFiles[fileIndex];

  switch(pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandType)
	 {
 // ---------------------------------------------------------------------
 	 	case PRJCT_INSTR_GDP_BAND_1B :
 	 	 offset=400;
 	 	break;
 // ---------------------------------------------------------------------
    case PRJCT_INSTR_GDP_BAND_2B :
     offset=9;
    break;
 // ---------------------------------------------------------------------
    default :
     offset=0;
    break;
 // ---------------------------------------------------------------------
 	 }

  for (i=0,j=pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex];
       j<pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex]+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;i++,j++)
   {
    lambda[i]=(double)EvalPolynom_d((double)i+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].startDetector,
                     &pOrbitFile->gdpBinCoeff[pOrbitFile->gdpBinHeader.nSpectralParam*SPECTRAL_FITT_ORDER*pOrbitFile->gdpBinBandIndex+
                      indexParam*SPECTRAL_FITT_ORDER],SPECTRAL_FITT_ORDER);

   	lambdax=(double)EvalPolynom_d((double)i+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].startDetector-offset,
                           &pOrbitFile->gdpBinCoeff[pOrbitFile->gdpBinHeader.nSpectralParam*SPECTRAL_FITT_ORDER*pOrbitFile->gdpBinBandIndex+
                            indexParam*SPECTRAL_FITT_ORDER],SPECTRAL_FITT_ORDER);

    lambda[i]-=(double)EvalPolynom_d((double)(lambdax-OFFSET)/PARAMETER,&pOrbitFile->gdpBinCoeff[pOrbitFile->gdpBinHeader.nSpectralParam*SPECTRAL_FITT_ORDER*pOrbitFile->gdpBinHeader.nbands+MAX_FITT_ORDER*pOrbitFile->gdpBinBandIndex],MAX_FITT_ORDER);
	  }
	}

RC GDP_BIN_get_orbit_date(int *year, int *month, int *day) {

  FILE *fp = NULL;
  RC rc = ERROR_ID_NO;

  const GOME_ORBIT_FILE *pOrbitFile = &GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];

  if ((fp=fopen(pOrbitFile->gdpBinFileName,"rb"))==NULL) {
    return ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pOrbitFile->gdpBinFileName);
  }

  GDP_BIN_FILE_HEADER tempHeader;

  // perhaps can use header from pOrbitFile
  if (!fread(&tempHeader,sizeof(GDP_BIN_FILE_HEADER),1,fp)) {
    rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pOrbitFile->gdpBinFileName);
  } else {
    // goto first spectrum, right after header
    fseek(fp,(int32_t)pOrbitFile->gdpBinHeader.headerSize,SEEK_SET);

    SPECTRUM_RECORD tempSpectrum;

    if (!fread(&tempSpectrum,sizeof(SPECTRUM_RECORD),1,fp)) {
      rc=ERROR_ID_FILE_END;
    } else {
      *year = get_year((int) tempSpectrum.dateAndTime.da_year);
      *month = (int) tempSpectrum.dateAndTime.da_mon;
      *day = (int) tempSpectrum.dateAndTime.da_day;
    }
  }

  if(fp != NULL) {
    fclose(fp);
  }

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      GDP_BIN_Set
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve information on data saved in the file from the header
//
// INPUT         specFp    pointer to the current GOME orbit file
//
// INPUT/OUTPUT  pEngineContext interface for file operations
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  if the file is not found;
//               ERROR_ID_FILE_EMPTY      if the file is empty;
//               ERROR_ID_ALLOC           if allocation of a buffer failed;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC GDP_BIN_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  char filePath[MAX_STR_SHORT_LEN+1];
  char fileFilter[MAX_STR_SHORT_LEN+1];
  char filePrefix[MAX_STR_SHORT_LEN+1];
  char fileExt[MAX_STR_SHORT_LEN+1];
  struct dirent *fileInfo;
  DIR *hDir;
  INDEX indexFile;
  char *ptr;
  char fileName[MAX_ITEM_TEXT_LEN];                                          // file name
  INDEX i,j,indexRecord;                                                        // indexes for loops and arrays
  int useErrors,errorFlag;                                                      // 0 if errors are saved with spectra, 1 otherwise
  FILE *fp;
  RC rc;                                                                        // return code

  // Initializations

  if (specFp!=NULL)
   {
    fclose(specFp);
    // IAPIAP
    pEngineContext->fileInfo.specFp = specFp=NULL;
   }

  pEngineContext->recordNumber=0;
  gdpBinLoadReferenceFlag=0;

  GDP_BIN_currentFileIndex=ITEM_NONE;
  ANALYSE_oldLatitude=(double)99999.;                                           // in automatic reference selection, force the selection of a
  strcpy(fileName,pEngineContext->fileInfo.fileName);                                         // new reference spectrum
  NDET[0]=1024;
  rc=ERROR_ID_NO;

  // In automatic reference selection, the file has maybe already loaded

  if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto)
   {
    for (indexFile=0;indexFile<gdpBinOrbitFilesN;indexFile++)
     if ((strlen(pEngineContext->fileInfo.fileName)==strlen(GDP_BIN_orbitFiles[indexFile].gdpBinFileName)) &&
         !strcasecmp(pEngineContext->fileInfo.fileName,GDP_BIN_orbitFiles[indexFile].gdpBinFileName))
      break;

    if (indexFile<gdpBinOrbitFilesN)
     GDP_BIN_currentFileIndex=indexFile;
   }

  if (GDP_BIN_currentFileIndex==ITEM_NONE)
   {
   	// Release old buffers

   	GDP_BIN_ReleaseBuffers();

   	// Get the number of files to load

   	if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto)
    	{
    		gdpBinLoadReferenceFlag=1;

    		if (pEngineContext->analysisRef.refLon)
       {
    		  // Get file path

     	  strcpy(filePath,pEngineContext->fileInfo.fileName);

     	  if ((ptr=strrchr(filePath,PATH_SEP))==NULL)
     	   {
    	   	 strcpy(filePath,".");
    	   	 strcpy(fileFilter,pEngineContext->fileInfo.fileName);
    	   	}
    	   else
    	    {
    	     *ptr++=0;
    	     strcpy(fileFilter,ptr);
    	    }

    	   fileFilter[6]='\0';

       	// Get the file extension of the original file name

        memset(fileExt,0,MAX_STR_SHORT_LEN);

        if ((ptr=strrchr(pEngineContext->fileInfo.fileName,'.'))!=NULL)
         strcpy(fileExt,ptr+1);
        else if (strlen(pEngineContext->project.instrumental.fileExt))
         strcpy(fileExt,pEngineContext->project.instrumental.fileExt);
        else
         strcpy(fileExt,"spe");

        for (hDir=opendir(filePath);(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);)
         {
          sprintf(GDP_BIN_orbitFiles[gdpBinOrbitFilesN].gdpBinFileName,"%s/%s",filePath,fileInfo->d_name);
          if (!STD_IsDir(GDP_BIN_orbitFiles[gdpBinOrbitFilesN].gdpBinFileName))
           {
           	strcpy(filePrefix,fileInfo->d_name);
           	filePrefix[6]='\0';

            if (((ptr=strrchr(fileInfo->d_name,'.'))!=NULL) && (strlen(ptr+1)==strlen(fileExt)) && !strcasecmp(ptr+1,fileExt) &&
                 (strlen(filePrefix)==strlen(fileFilter)) && !strcasecmp(filePrefix,fileFilter))
             gdpBinOrbitFilesN++;
           }
         }

        if ( hDir != NULL ) closedir(hDir);

        if (!gdpBinOrbitFilesN)
         {
     	    gdpBinOrbitFilesN=1;
     	    strcpy(GDP_BIN_orbitFiles[0].gdpBinFileName,pEngineContext->fileInfo.fileName);
     	   }
       }
      else
       {
       	gdpBinOrbitFilesN=1;
       	strcpy(GDP_BIN_orbitFiles[0].gdpBinFileName,pEngineContext->fileInfo.fileName);
       }
   	 }
   	else
     {
     	gdpBinOrbitFilesN=1;
     	strcpy(GDP_BIN_orbitFiles[0].gdpBinFileName,pEngineContext->fileInfo.fileName);
     }

    GDP_BIN_currentFileIndex=ITEM_NONE;

    // Load files

    for (gdpBinTotalRecordNumber=indexFile=0;indexFile<gdpBinOrbitFilesN;indexFile++)
     {
     	pOrbitFile=&GDP_BIN_orbitFiles[indexFile];
     	pOrbitFile->specNumber=0;

     	if ((ptr=strrchr(pOrbitFile->gdpBinFileName,PATH_SEP))==NULL)
     	 ptr=pOrbitFile->gdpBinFileName;
     	else
     	 ptr++;

     	memset(pOrbitFile->gdpBinFileNumber,0,9);
     	strncpy(pOrbitFile->gdpBinFileNumber,ptr,8);

      // Open the current GOME orbit file

      if ((fp=fopen(pOrbitFile->gdpBinFileName,"rb"))==NULL)
       rc=ERROR_SetLast("GDP_Bin_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pOrbitFile->gdpBinFileName);

      // Read GOME file header

      else if (!fread(&pOrbitFile->gdpBinHeader,sizeof(GDP_BIN_FILE_HEADER),1,fp))
       rc=ERROR_SetLast("GDP_Bin_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pOrbitFile->gdpBinFileName);
      else
       {
        // In the GDP binary format, errors are saved on user request

        useErrors=((pOrbitFile->gdpBinHeader.mask&GDP_BIN_ERROR_ID_MASK)==GDP_BIN_ERROR_ID_MASK)?1:0;
        errorFlag=(useErrors)?0:1;

        // Retrieve information on bands present in the file

        for (i=0;i<pOrbitFile->gdpBinHeader.nbands;i++)
         fread(&pOrbitFile->gdpBinBandInfo[i],(sizeof(GDP_BIN_BAND_HEADER)-errorFlag*sizeof(float)),1,fp);

        // Coefficients of the polynomial for re-calculating the wavelength calibration

        pOrbitFile->gdpBinCoeffSize=pOrbitFile->gdpBinHeader.nSpectralParam*SPECTRAL_FITT_ORDER*pOrbitFile->gdpBinHeader.nbands;
        pOrbitFile->gdpBinCoeffSize+=MAX_FITT_ORDER*pOrbitFile->gdpBinHeader.nbands;

        // Get the size of spectra band per band

        for (i=1,pOrbitFile->gdpBinStartPixel[0]=0,pOrbitFile->gdpBinSpectraSize=pOrbitFile->gdpBinBandInfo[0].bandSize;
             i<pOrbitFile->gdpBinHeader.nbands;i++)
         {
          pOrbitFile->gdpBinStartPixel[i]=pOrbitFile->gdpBinStartPixel[i-1]+pOrbitFile->gdpBinBandInfo[i-1].bandSize;
          pOrbitFile->gdpBinSpectraSize+=pOrbitFile->gdpBinBandInfo[i].bandSize;
         }

        for (i=0;(i<pOrbitFile->gdpBinHeader.nbands);i++)
         if (pOrbitFile->gdpBinBandInfo[i].bandType==pEngineContext->project.instrumental.gome.bandType)
           pOrbitFile->gdpBinBandIndex=i;

        // Buffers allocation

        if (pOrbitFile->gdpBinBandIndex==ITEM_NONE)
         rc=ERROR_SetLast("GDP_Bin_Set",ERROR_TYPE_WARNING,ERROR_ID_GDP_BANDINDEX,fileName);
        else if (!pOrbitFile->gdpBinCoeffSize || !pOrbitFile->gdpBinSpectraSize || !pOrbitFile->gdpBinHeader.nspectra)
         rc=ERROR_SetLast("GDP_Bin_Set (1)",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);

        else if (((pOrbitFile->gdpBinCoeff=(double *)MEMORY_AllocDVector("GDP_BIN_Set ","pOrbitFile->gdpBinCoeff",0,pOrbitFile->gdpBinCoeffSize-1))==NULL) ||
                 ((pOrbitFile->gdpBinReference=(unsigned short *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinReference",pOrbitFile->gdpBinSpectraSize,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
                 ((pOrbitFile->gdpBinRefError=(unsigned short *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinRefError",pOrbitFile->gdpBinSpectraSize,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
                 ((pOrbitFile->gdpBinInfo=(GDP_BIN_INFO *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinInfo",pOrbitFile->gdpBinHeader.nspectra,sizeof(GDP_BIN_INFO),0,MEMORY_TYPE_STRUCT))==NULL) ||
                 ((pOrbitFile->gdpBinSzaIndex=(INDEX *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinSzaIndex",pOrbitFile->gdpBinHeader.nspectra,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
                 ((pOrbitFile->gdpBinLatIndex=(INDEX *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinLatIndex",pOrbitFile->gdpBinHeader.nspectra,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
                 ((pOrbitFile->gdpBinLonIndex=(INDEX *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinLonIndex",pOrbitFile->gdpBinHeader.nspectra,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
                 ((pOrbitFile->gdpBinPixIndex=(INDEX *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinPixIndex",pOrbitFile->gdpBinHeader.nspectra,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL))

         rc=ERROR_ID_ALLOC;

        else
         {
          NDET[0]=pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;

          // Read the irradiance spectrum

          fread(pOrbitFile->gdpBinCoeff,sizeof(double)*pOrbitFile->gdpBinCoeffSize,1,fp);
          fread(pOrbitFile->gdpBinReference,sizeof(unsigned short)*pOrbitFile->gdpBinSpectraSize,1,fp);

          if (useErrors)
           fread(pOrbitFile->gdpBinRefError,sizeof(unsigned short)*pOrbitFile->gdpBinSpectraSize,1,fp);

          pOrbitFile->specNumber=pOrbitFile->gdpBinHeader.nspectra;

          for (indexRecord=0;indexRecord<pOrbitFile->specNumber;indexRecord++)
           {
            fseek(fp,(int32_t)pOrbitFile->gdpBinHeader.headerSize+indexRecord*pOrbitFile->gdpBinHeader.recordSize,SEEK_SET);

            if (!fread(&pOrbitFile->gdpBinSpectrum,sizeof(SPECTRUM_RECORD),1,fp))
             rc=ERROR_SetLast("GDP_Bin_Set (2)",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);
            else if (((pOrbitFile->gdpBinHeader.version<40) && !fread(&pOrbitFile->gdpBinGeo3,sizeof(GEO_3),1,fp)) ||
                     ((pOrbitFile->gdpBinHeader.version>=40) && !fread(&pOrbitFile->gdpBinGeo4,sizeof(GEO_4),1,fp)))
             {
              pOrbitFile->gdpBinInfo[indexRecord].pixelNumber=pOrbitFile->gdpBinSpectrum.groundPixelID;
              pOrbitFile->gdpBinInfo[indexRecord].pixelType=pOrbitFile->gdpBinSpectrum.groundPixelType;

              if (pOrbitFile->gdpBinHeader.version<40)
               {
                pOrbitFile->gdpBinInfo[indexRecord].sza=(double)pOrbitFile->gdpBinGeo3.szaArray[1];
                pOrbitFile->gdpBinInfo[indexRecord].lat=(double)0.01*pOrbitFile->gdpBinGeo3.latArray[4];
                pOrbitFile->gdpBinInfo[indexRecord].lon=(double)0.01*pOrbitFile->gdpBinGeo3.lonArray[4];
               }
              else
               {
                pOrbitFile->gdpBinInfo[indexRecord].sza=(double)pOrbitFile->gdpBinGeo4.szaArrayBOA[1];
                pOrbitFile->gdpBinInfo[indexRecord].lat=(double)0.01*pOrbitFile->gdpBinGeo4.latArray[4];
                pOrbitFile->gdpBinInfo[indexRecord].lon=(double)0.01*pOrbitFile->gdpBinGeo4.lonArray[4];
               }

              GdpBinSort(indexRecord,0,indexRecord,indexFile);                 // sort latitudes
              GdpBinSort(indexRecord,1,indexRecord,indexFile);                 // sort SZA
              GdpBinSort(indexRecord,2,indexRecord,indexFile);                 // sort pixel numbers
              GdpBinSort(indexRecord,3,indexRecord,indexFile);                 // sort longitudes
             }
           }
         }

        fclose(fp);
       }

      if ((strlen(pEngineContext->fileInfo.fileName)==strlen(pOrbitFile->gdpBinFileName)) &&
          !strcasecmp(pEngineContext->fileInfo.fileName,pOrbitFile->gdpBinFileName))
       GDP_BIN_currentFileIndex=indexFile;

      gdpBinTotalRecordNumber+=pOrbitFile->specNumber;

      if (rc!=ERROR_ID_NO)
       pOrbitFile->rc=rc;

      rc=ERROR_ID_NO;
     }
   }

  if ((GDP_BIN_currentFileIndex!=ITEM_NONE) &&
     ((pEngineContext->recordNumber=GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].specNumber)>0) &&
     !rc) {

    pOrbitFile=&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];

    // Irradiance spectrum

    for (i=0,j=pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex];
         j<pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex]+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;i++,j++) {

      pEngineContext->buffers.irrad[i]=(double)pOrbitFile->gdpBinReference[j]/pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].scalingFactor;
    }

    pEngineContext->recordInfo.satellite.orbit_number= 1+pOrbitFile->gdpBinHeader.orbitNumber;
    pEngineContext->recordInfo.useErrors=((pOrbitFile->gdpBinHeader.mask&GDP_BIN_ERROR_ID_MASK)==GDP_BIN_ERROR_ID_MASK)?1:0;
    NDET[0]=pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;

    GdpBinLambda(pEngineContext->buffers.lambda,GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinHeader.indexSpectralParam,GDP_BIN_currentFileIndex);
    for (i=0; i<NDET[0]; ++i) {
      pEngineContext->buffers.lambda_irrad[i]=pEngineContext->buffers.lambda[i];
    }

    rc=GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].rc;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GDP_BIN_Read
// -----------------------------------------------------------------------------
// PURPOSE       GOME calibrated level 1 data read out (binary format)
//
// INPUT         recordNo     index of the record to read
//               specFp       pointer to the current GOME orbit file
//
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  if the file is not found;
//               ERROR_ID_FILE_END        the end of the file is reached;
//               ERROR_ID_FILE_RECORD     the record doesn't satisfy user constraints
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif

RC GDP_BIN_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,FILE *specFp,INDEX indexFile)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  unsigned short *spectrum,*specError;                                          // spectra are in short integers instead of double in the binary format
  SHORT_DATE today;                                                             // date of measurements
  double Max;                                                                   // maximum of the spectrum
  INDEX i,j;                                                                    // indexes for loops and arrays
  FILE *fp;
  RC rc;                                                                        // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;

  pOrbitFile=&GDP_BIN_orbitFiles[indexFile];
  spectrum=specError=NULL;
  rc=ERROR_ID_NO;
  fp=NULL;

  if (!pOrbitFile->specNumber)
   rc=ERROR_ID_FILE_EMPTY;
  else if ((recordNo<=0) || (recordNo>pOrbitFile->specNumber))
   rc=ERROR_ID_FILE_RECORD;
  else if ((fp=fopen(pOrbitFile->gdpBinFileName,"rb"))==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;

  // Buffers allocation

  else if (((spectrum=(unsigned short *)MEMORY_AllocBuffer("GDP_BIN_Read ","spectrum",pOrbitFile->gdpBinSpectraSize,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
            (pRecord->useErrors &&
           ((specError=(unsigned short *)MEMORY_AllocBuffer("GDP_BIN_Read ","specError",pOrbitFile->gdpBinSpectraSize,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL)))

  rc=ERROR_ID_ALLOC;

  else
   {
    // Complete record read out

    fseek(fp,(int32_t)pOrbitFile->gdpBinHeader.headerSize+(recordNo-1)*pOrbitFile->gdpBinHeader.recordSize,SEEK_SET);

    if (!fread(&pOrbitFile->gdpBinSpectrum,sizeof(SPECTRUM_RECORD),1,fp) ||
       ((pOrbitFile->gdpBinHeader.version<40) && !fread(&pOrbitFile->gdpBinGeo3,sizeof(GEO_3),1,fp)) ||
       ((pOrbitFile->gdpBinHeader.version>=40) && !fread(&pOrbitFile->gdpBinGeo4,sizeof(GEO_4),1,fp)) ||
        !fread(pOrbitFile->gdpBinScalingFactor,sizeof(float)*pOrbitFile->gdpBinHeader.nbands,1,fp) ||
        !fread(spectrum,sizeof(unsigned short)*pOrbitFile->gdpBinSpectraSize,1,fp) ||
       (pRecord->useErrors &&
       (!fread(pOrbitFile->gdpBinScalingError,sizeof(float)*pOrbitFile->gdpBinHeader.nbands,1,fp) ||
        !fread(specError,sizeof(unsigned short)*pOrbitFile->gdpBinSpectraSize,1,fp))))

     rc=ERROR_ID_FILE_END;

    else if ((pEngineContext->project.instrumental.gome.pixelType!=-1) &&
             (pOrbitFile->gdpBinSpectrum.groundPixelType!=pEngineContext->project.instrumental.gome.pixelType))
     rc=ERROR_ID_FILE_RECORD;
    else
     {
      today.da_year=pOrbitFile->gdpBinSpectrum.dateAndTime.da_year;
      today.da_mon=pOrbitFile->gdpBinSpectrum.dateAndTime.da_mon;
      today.da_day=pOrbitFile->gdpBinSpectrum.dateAndTime.da_day;

      today.da_year = (short) get_year((int)today.da_year);

      // Fill fields of structure

      pRecord->Tint     = (double)0.;
      pRecord->NSomme   = 0;

      if (pOrbitFile->gdpBinHeader.version<40)
       {
        pRecord->Zm       = (double)pOrbitFile->gdpBinGeo3.szaArray[1];
        pRecord->Azimuth  = (double)pOrbitFile->gdpBinGeo3.aziArray[1];
        pRecord->longitude=0.01*pOrbitFile->gdpBinGeo3.lonArray[4];
        pRecord->latitude =0.01*pOrbitFile->gdpBinGeo3.latArray[4];
        pRecord->zenithViewAngle=(float)0.01*pOrbitFile->gdpBinGeo3.losZa[1];
        pRecord->azimuthViewAngle=(float)0.01*pOrbitFile->gdpBinGeo3.losAzim[1];
        pRecord->cloudFraction=(float)pOrbitFile->gdpBinGeo3.cloudFraction;
        pRecord->cloudTopPressure=(float)pOrbitFile->gdpBinGeo3.cloudTopPressure;
        pRecord->satellite.altitude = pOrbitFile->gdpBinGeo3.satHeight;
        pRecord->satellite.earth_radius = pOrbitFile->gdpBinGeo3.radiusCurve;
       }
      else
       {
        pRecord->Zm       = (double)pOrbitFile->gdpBinGeo4.szaArrayBOA[1];
        pRecord->Azimuth  = (double)pOrbitFile->gdpBinGeo4.aziArrayBOA[1];
        pRecord->longitude=0.01*pOrbitFile->gdpBinGeo4.lonArray[4];
        pRecord->latitude =0.01*pOrbitFile->gdpBinGeo4.latArray[4];
        pRecord->zenithViewAngle=(float)pOrbitFile->gdpBinGeo4.losZaBOA[1];
        pRecord->azimuthViewAngle=(float)pOrbitFile->gdpBinGeo4.losAzimBOA[1];
        pRecord->cloudFraction=(float)pOrbitFile->gdpBinGeo4.cloudInfo.CloudFraction[0];
        pRecord->cloudTopPressure=(float)pOrbitFile->gdpBinGeo4.cloudInfo.CTP[0];
        pRecord->satellite.altitude = pOrbitFile->gdpBinGeo4.satHeight;
        pRecord->satellite.earth_radius = pOrbitFile->gdpBinGeo4.radiusCurve;
       }

      pRecord->present_datetime.thetime.ti_hour=pOrbitFile->gdpBinSpectrum.dateAndTime.ti_hour;
      pRecord->present_datetime.thetime.ti_min=pOrbitFile->gdpBinSpectrum.dateAndTime.ti_min;
      pRecord->present_datetime.thetime.ti_sec=pOrbitFile->gdpBinSpectrum.dateAndTime.ti_sec;

      memset(pRecord->Nom,0,20);
      pRecord->present_datetime.thedate.da_day = today.da_day;
      pRecord->present_datetime.thedate.da_mon = today.da_mon;
      pRecord->present_datetime.thedate.da_year = today.da_year;

      pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
      pRecord->localCalDay=ZEN_FNCaljda(&pRecord->Tm);
      pRecord->TotalExpTime=pRecord->TotalAcqTime=(double)0.;
      pRecord->TimeDec=(double)pOrbitFile->gdpBinSpectrum.dateAndTime.ti_hour+
                                 pOrbitFile->gdpBinSpectrum.dateAndTime.ti_min/60.+
                                 pOrbitFile->gdpBinSpectrum.dateAndTime.ti_sec/3600.;

      pRecord->gome.pixelNumber=pOrbitFile->gdpBinSpectrum.groundPixelID;
      pRecord->gome.pixelType=pOrbitFile->gdpBinSpectrum.groundPixelType;

      pRecord->altitude=(double)0.;

      // Convert spectrum from short integers to double

      GdpBinLambda(pBuffers->lambda,pOrbitFile->gdpBinSpectrum.indexSpectralParam,GDP_BIN_currentFileIndex);

      for (i=0,j=pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex],Max=(double)0.;
           j<pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex]+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;i++,j++)
       {
        pBuffers->spectrum[i]=(double)spectrum[j]/pOrbitFile->gdpBinScalingFactor[pOrbitFile->gdpBinBandIndex];
        pBuffers->sigmaSpec[i]=(pRecord->useErrors)?(double)specError[i]/pOrbitFile->gdpBinScalingError[pOrbitFile->gdpBinBandIndex]:pBuffers->spectrum[i];

        if (pBuffers->spectrum[i]>Max)
         Max=pBuffers->spectrum[i];
       }

      if (Max==(double)0.)
       rc=ERROR_ID_FILE_RECORD;
     }
   }

  // Release allocated buffers

  if (spectrum!=NULL)
   MEMORY_ReleaseBuffer("GDP_BIN_Read","spectrum",spectrum);
  if (specError!=NULL)
   MEMORY_ReleaseBuffer("GDP_BIN_Read","specError",specError);

  // Close file

  if (fp!=NULL)
   fclose(fp);

  return rc;
 }

// =============================
// AUTOMATIC REFERENCE SELECTION
// =============================

// -----------------------------------------------------------------------------
// FUNCTION      GdpBinSortGetIndex
// -----------------------------------------------------------------------------
// PURPOSE       Get the position in sorted list of a new element (pixel,latitude or SZA)
//
// INPUT         value        the value to sort out
//               flag         0 for latitudes, 1 for SZA, 2 for pixel number
//               listSize     the current size of the sorted list
//               fileIndex    index of the current file
//
// RETURN        the index of the new element in the sorted list;
// -----------------------------------------------------------------------------

INDEX GdpBinSortGetIndex(double value,int flag,int listSize,INDEX fileIndex)
 {
  // Declarations

  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX  imin,imax,icur;                                                        // indexes for dichotomic search
  double curValue;                                                              // value of element pointed by icur in the sorted list

  // Initializations

  pOrbitFile=&GDP_BIN_orbitFiles[fileIndex];
  imin=icur=0;
  imax=listSize;
  curValue=(double)0.;

  // Browse latitudes

  while (imax-imin>1)
   {
    // Dichotomic sort

    icur=(imin+imax)>>1;

    switch(flag)
     {
      case 0 : curValue=(double)pOrbitFile->gdpBinInfo[pOrbitFile->gdpBinLatIndex[icur]].lat; break;
      case 1 : curValue=(double)pOrbitFile->gdpBinInfo[pOrbitFile->gdpBinSzaIndex[icur]].sza; break;
      case 2 : curValue=(double)pOrbitFile->gdpBinInfo[pOrbitFile->gdpBinPixIndex[icur]].pixelNumber; break;
      case 3 : curValue=(double)pOrbitFile->gdpBinInfo[pOrbitFile->gdpBinPixIndex[icur]].lon; break;
     }

    // Move bounds

    if (curValue==value)
     imin=imax=icur;
    else if (curValue<value)
     imin=icur;
    else
     imax=icur;
   }

  if (listSize>0)
   {
    switch(flag)
     {
      case 0 : curValue=(double)pOrbitFile->gdpBinInfo[pOrbitFile->gdpBinLatIndex[imin]].lat; break;
      case 1 : curValue=(double)pOrbitFile->gdpBinInfo[pOrbitFile->gdpBinSzaIndex[imin]].sza; break;
      case 2 : curValue=(double)pOrbitFile->gdpBinInfo[pOrbitFile->gdpBinPixIndex[imin]].pixelNumber; break;
      case 3 : curValue=(double)pOrbitFile->gdpBinInfo[pOrbitFile->gdpBinPixIndex[imin]].lon; break;
     }

    icur=(curValue>value)?imin:imax;
   }

  // Return

  return icur;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GdpBinSort
// -----------------------------------------------------------------------------
// PURPOSE       Sort NADIR records on latitudes or SZA
//
// INPUT         indexRecord  the index of the record to sort out
//               flag         0 for latitudes, 1 for SZA, 2 for pixel number
//               listSize     the current size of the sorted list
//               fileIndex    index of the current file
//
// RETURN        the new index of the record in the sorted list;
// -----------------------------------------------------------------------------

void GdpBinSort(INDEX indexRecord,int flag,int listSize,INDEX fileIndex)
 {
  // Declaration

  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX  newIndex,                                                              // the position of the new record in the sorted list
        *sortedList,                                                            // the sorted list
         i;                                                                     // browse the sorted list in reverse way

  double value;                                                                 // the value to sort out

  // Initializations

  pOrbitFile=&GDP_BIN_orbitFiles[fileIndex];
  sortedList=NULL;
  value=(double)0.;

  switch(flag)
   {
 // -------------------------------------------
    case 0 :
     sortedList=pOrbitFile->gdpBinLatIndex;
     value=(double)pOrbitFile->gdpBinInfo[indexRecord].lat;
    break;
 // -------------------------------------------
    case 1 :
     sortedList=pOrbitFile->gdpBinSzaIndex;
     value=(double)pOrbitFile->gdpBinInfo[indexRecord].sza;
    break;
 // -------------------------------------------
    case 2 :
     sortedList=pOrbitFile->gdpBinPixIndex;
     value=(double)pOrbitFile->gdpBinInfo[indexRecord].pixelNumber;
    break;
 // -------------------------------------------
    case 3 :
     sortedList=pOrbitFile->gdpBinLonIndex;
     value=(double)pOrbitFile->gdpBinInfo[indexRecord].lon;
    break;
 // -------------------------------------------
  }

  newIndex=(flag<2)?GdpBinSortGetIndex(value,flag,listSize,fileIndex):listSize;

  // Shift values higher than the one to sort out

  if (newIndex<listSize)
   for (i=listSize;i>newIndex;i--)
    sortedList[i]=sortedList[i-1];

  // Insert new record in the sorted list

  sortedList[newIndex]=indexRecord;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GdpBinRefLat
// -----------------------------------------------------------------------------
// PURPOSE       Search for spectra in the orbit file matching latitudes and SZA
//               conditions
//
// INPUT         latMin,latMax determine the range of latitudes;
//               sza,szaDelta  determine the range of SZA;
//
// OUTPUT        refList      the list of potential reference spectra
//
// RETURN        the number of elements in the refList reference list
// -----------------------------------------------------------------------------

int GdpBinRefLat(GDP_BIN_REF *refList,double latMin,double latMax,double lonMin,double lonMax,double sza,double szaDelta,char *gomePixelType)
 {
  // Declarations

  INDEX fileIndex;
  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX ilatMin,ilatMax,ilatTmp,                                                // range of indexes of latitudes matching conditions in sorted list
        ilatIndex,                                                              // browse records with latitudes in the specified range
        indexRef;                                                               // browse reference already registered in order to keep the list sorted

  int nRef;                                                                     // the number of spectra matching latitudes and SZA conditions
  double szaDist,latDist;                                                       // distance with latitude and sza centers
  double lon;                                                                   // converts the longitude in the -180:180 range
  GDP_BIN_INFO *pRecord;

  // Initialization

  nRef=0;

  for (fileIndex=0;fileIndex<gdpBinOrbitFilesN;fileIndex++)
   {
   	pOrbitFile=&GDP_BIN_orbitFiles[fileIndex];

   	if (pOrbitFile->specNumber)
   	 {
      // Determine the set of records in the orbit file matching the latitudes conditions

      ilatMin=GdpBinSortGetIndex(latMin,0,pOrbitFile->gdpBinHeader.nspectra,fileIndex);
      ilatMax=GdpBinSortGetIndex(latMax,0,pOrbitFile->gdpBinHeader.nspectra,fileIndex);

      if (ilatMin>ilatMax)
       {
        ilatTmp=ilatMin;
        ilatMin=ilatMax;
        ilatMax=ilatTmp;
       }

      // Browse spectra matching latitudes conditions

      for (ilatIndex=ilatMin;ilatIndex<ilatMax;ilatIndex++)
       {
        pRecord=&pOrbitFile->gdpBinInfo[pOrbitFile->gdpBinLatIndex[ilatIndex]];

        if ((fabs(lonMax-lonMin)>EPSILON) && (fabs(lonMax-lonMin)<(double)359.))
         latDist=THRD_GetDist(pRecord->lon,pRecord->lat,(lonMax+lonMin)*0.5,(latMax+latMin)*0.5);
        else
         latDist=fabs(pRecord->lat-(latMax+latMin)*0.5);

        szaDist=fabs(pRecord->sza-sza);

        lon=((lonMin<0) && (pRecord->lon>180))?pRecord->lon-360:pRecord->lon;   // Longitudes for GOME are in the range 0..360
                                                                                // If lonMin<0, this means that the given longitudes are in -180..180 range
        // Limit the latitudes conditions to SZA conditions

        if ((gomePixelType[pRecord->pixelType]==1) &&
           ((szaDelta<EPSILON) || (szaDist<=szaDelta)) &&
           ((fabs(lonMax-lonMin)<EPSILON) || (fabs(lonMax-lonMin)>359.) ||
           ((lon>=lonMin) && (lon<=lonMax))))
         {
          // Keep the list of records sorted

          for (indexRef=nRef;indexRef>0;indexRef--)

           if (latDist>=refList[indexRef-1].latDist)
            break;
           else
            memcpy(&refList[indexRef],&refList[indexRef-1],sizeof(GDP_BIN_REF));

          refList[indexRef].indexFile=fileIndex;
          refList[indexRef].indexRecord=pOrbitFile->gdpBinLatIndex[ilatIndex];
          refList[indexRef].pixelNumber=pRecord->pixelNumber;
          refList[indexRef].pixelType=pRecord->pixelType;
          refList[indexRef].latitude=pRecord->lat;
          refList[indexRef].longitude=pRecord->lon;
          refList[indexRef].sza=pRecord->sza;
          refList[indexRef].szaDist=szaDist;
          refList[indexRef].latDist=latDist;

          nRef++;
         }
       }
     }
   }

  // Return

  return nRef;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GdpBinRefSza
// -----------------------------------------------------------------------------
// PURPOSE       Search for spectra in the orbit file matching SZA
//               conditions only
//
// INPUT         sza,szaDelta determine the range of SZA;
//
// OUTPUT        refList      the list of potential reference spectra
//
// RETURN        the number of elements in the refList reference list
// -----------------------------------------------------------------------------

int GdpBinRefSza(GDP_BIN_REF *refList,double sza,double szaDelta,char *gomePixelType)
 {
  // Declarations

  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX fileIndex;
  INDEX iszaMin,iszaMax,iszaTmp,                                                // range of indexes of SZA matching conditions in sorted list
        iszaIndex,                                                              // browse records with SZA in the specified SZA range
        indexRef;                                                               // browse reference already registered in order to keep the list sorted

  int nRef;                                                                     // the number of spectra matching latitudes and SZA conditions
  double szaDist;                                                               // distance with sza center
  GDP_BIN_INFO *pRecord;

  // Initialization

  nRef=0;

  for (fileIndex=0;fileIndex<gdpBinOrbitFilesN;fileIndex++)
   {
   	pOrbitFile=&GDP_BIN_orbitFiles[fileIndex];
   	if (pOrbitFile->specNumber)
   	 {
      // Determine the set of records in the orbit file matching SZA conditions

      if (szaDelta>EPSILON)
       {
        iszaMin=GdpBinSortGetIndex(sza-szaDelta,1,pOrbitFile->gdpBinHeader.nspectra,fileIndex);
        iszaMax=GdpBinSortGetIndex(sza+szaDelta,1,pOrbitFile->gdpBinHeader.nspectra,fileIndex);
       }
      else  // No SZA conditions, search for the minimum
       {
        iszaMin=0;
        iszaMax=pOrbitFile->gdpBinHeader.nspectra-1;
       }

      if (iszaMin>iszaMax)
       {
        iszaTmp=iszaMin;
        iszaMin=iszaMax;
        iszaMax=iszaTmp;
       }

      // Browse spectra matching SZA conditions

      for (iszaIndex=iszaMin;iszaIndex<iszaMax;iszaIndex++)
       {
        pRecord=&pOrbitFile->gdpBinInfo[pOrbitFile->gdpBinSzaIndex[iszaIndex]];

        if (gomePixelType[pRecord->pixelType]==1)
         {
          szaDist=fabs(pRecord->sza-sza);

          // Keep the list of records sorted

          for (indexRef=nRef;indexRef>0;indexRef--)

           if (szaDist>=refList[indexRef-1].szaDist)
            break;
           else
            memcpy(&refList[indexRef],&refList[indexRef-1],sizeof(GDP_BIN_REF));

          refList[indexRef].indexFile=fileIndex;
          refList[indexRef].indexRecord=pOrbitFile->gdpBinSzaIndex[iszaIndex];
          refList[indexRef].pixelNumber=pRecord->pixelNumber;
          refList[indexRef].pixelType=pRecord->pixelType;
          refList[indexRef].latitude=pRecord->lat;
          refList[indexRef].longitude=pRecord->lon;
          refList[indexRef].sza=pRecord->sza;
          refList[indexRef].szaDist=szaDist;
          refList[indexRef].latDist=(double)0.;

          nRef++;
         }
       }
     }
   }

  // Return

  return nRef;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GdpBinBuildRef
// -----------------------------------------------------------------------------
// PURPOSE       Build a reference spectrum by averaging a set of spectra
//               matching latitudes and SZA conditions
//
// INPUT         refList      the list of potential reference spectra
//               nRef         the number of elements in the previous list
//               nSpectra     the maximum number of spectra to average to build the reference spectrum;
//               lambda       the grid of the irradiance spectrum
//               pEngineContext    interface for file operations
//               specFp       pointer to the current file;
//
// OUTPUT        ref          the new reference spectrum
//
// RETURN        code returned by GDP_BIN_Read if any;
//               ERROR_ID_NO_REF if no reference spectrum is found;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC GdpBinBuildRef(GDP_BIN_REF *refList,int nRef,int nSpectra,double *lambda,double *ref,ENGINE_CONTEXT *pEngineContext,FILE *specFp,INDEX *pIndexLine,void *responseHandle)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  GDP_BIN_REF *pRef;                                                            // pointer to the current reference spectrum
  INDEX     indexRef,                                                           // browse reference in the list
            indexFile,                                                          // browse files
            indexColumn,                                                        // index of the current column in the cell page associated to the ref plot page
            i;                                                                  // index for loop and arrays

  int       nRec;                                                               // number of records used for the average
  RC        rc;                                                                 // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  rc=ERROR_ID_NO;
  indexColumn=2;

  const int n_wavel = NDET[0];

  for (i=0;i<n_wavel;i++)
   lambda[i]=ref[i]=(double)0.;

  // Search for spectra matching latitudes and SZA conditions

  for (nRec=0,indexRef=0,indexFile=ITEM_NONE;
      (indexRef<nRef) && (nRec<nSpectra) && !rc;indexRef++)
   {
    pRef=&refList[indexRef];

    if (((indexFile==ITEM_NONE) || (pRef->indexFile==indexFile)) &&
       !(rc=GDP_BIN_Read(pEngineContext,pRef->indexRecord+1,specFp,pRef->indexFile)))    // Read and accumulate selected radiances
     {
      if (indexFile==ITEM_NONE)
       {
        mediateResponseCellDataString(plotPageRef,(*pIndexLine)++,indexColumn,"Ref Selection",responseHandle);
        mediateResponseCellInfo(plotPageRef,(*pIndexLine)++,indexColumn,responseHandle,"Ref File","%s",GDP_BIN_orbitFiles[pRef->indexFile].gdpBinFileName);
        mediateResponseCellDataString(plotPageRef,(*pIndexLine),indexColumn,"Record",responseHandle);
        mediateResponseCellDataString(plotPageRef,(*pIndexLine),indexColumn+1,"Pixel number",responseHandle);
        mediateResponseCellDataString(plotPageRef,(*pIndexLine),indexColumn+2,"Pixel type",responseHandle);
        mediateResponseCellDataString(plotPageRef,(*pIndexLine),indexColumn+3,"SZA",responseHandle);
        mediateResponseCellDataString(plotPageRef,(*pIndexLine),indexColumn+4,"Lat",responseHandle);
        mediateResponseCellDataString(plotPageRef,(*pIndexLine),indexColumn+5,"Lon",responseHandle);

        (*pIndexLine)++;

        strcpy(pRecord->refFileName,GDP_BIN_orbitFiles[pRef->indexFile].gdpBinFileName);
        pRecord->refRecord=pRef->indexRecord+1;
       }

      mediateResponseCellDataInteger(plotPageRef,(*pIndexLine),indexColumn,pRef->indexRecord+1,responseHandle);
      mediateResponseCellDataInteger(plotPageRef,(*pIndexLine),indexColumn+1,pRef->pixelNumber,responseHandle);
      mediateResponseCellDataInteger(plotPageRef,(*pIndexLine),indexColumn+2,pRef->pixelType,responseHandle);
      mediateResponseCellDataDouble(plotPageRef,(*pIndexLine),indexColumn+3,pRef->sza,responseHandle);
      mediateResponseCellDataDouble(plotPageRef,(*pIndexLine),indexColumn+4,pRef->latitude,responseHandle);
      mediateResponseCellDataDouble(plotPageRef,(*pIndexLine),indexColumn+5,pRef->longitude,responseHandle);

      (*pIndexLine)++;

      for (i=0;i<n_wavel;i++)
       {
        lambda[i]+=(double)pEngineContext->buffers.lambda[i];
        ref[i]+=(double)pEngineContext->buffers.spectrum[i];
       }

      nRec++;
      indexFile=pRef->indexFile;
     }
   }

  if (nRec==0)
   rc=ERROR_ID_NO_REF;
  else if (!rc)
   {
   	strcpy(OUTPUT_refFile,GDP_BIN_orbitFiles[indexFile].gdpBinFileName);
   	OUTPUT_nRec=nRec;

    for (i=0;i<n_wavel;i++)
     {
      lambda[i]/=nRec;
      ref[i]/=nRec;
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GdpBinRefSelection
// -----------------------------------------------------------------------------
// PURPOSE       Selection of a reference spectrum in the current orbit
//
// INPUT         pEngineContext      collect information on the current spectrum;
//               specFp         pointer to the current file;
//               latMin,latMax  determine the range of latitudes;
//               lonMin,lonMax  determine the range of longitudes;
//               sza,szaDelta   determine the range of SZA;
//
//               nSpectra       the number of spectra to average to build the reference spectrum;
//               lambdaK,ref    reference spectrum to use if no spectrum in the orbit matches the sza and latitudes conditions;
//
// OUTPUT        lambdaN,refN   reference spectrum for northern hemisphere;
//               lambdaS,refS   reference spectrum for southern hemisphere.
//
// RETURN        ERROR_ID_ALLOC if the allocation of buffers failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC GdpBinRefSelection(ENGINE_CONTEXT *pEngineContext,
                      FILE *specFp,
                      double latMin,double latMax,
                      double lonMin,double lonMax,
                      double sza,double szaDelta,
                      int nSpectra,
                      double *lambdaK,double *ref,
                      double *lambdaN,double *refN,double *pRefNormN,
                      double *lambdaS,double *refS,double *pRefNormS,
                      char *gomePixelType,
                      void *responseHandle)
 {
  // Declarations

  GDP_BIN_REF *refList;                                                         // list of potential reference spectra
  int nRefN,nRefS;                                                              // number of reference spectra in the previous list resp. for Northern and Southern hemisphere
  INDEX indexLine,indexColumn;
  double latDelta,tmp;
  RC rc;                                                                        // return code

  // Initializations

  indexLine=1;
  indexColumn=2;

  if (latMin>latMax)
   {
   	tmp=latMin;
   	latMin=latMax;
   	latMax=tmp;
   }

  if (lonMin>lonMax)
   {
   	tmp=lonMin;
   	lonMin=lonMax;
   	lonMax=tmp;
   }

  latDelta=(double)fabs(latMax-latMin);
  szaDelta=(double)fabs(szaDelta);
  sza=(double)fabs(sza);

  rc=ERROR_ID_NO;

  const int n_wavel = NDET[0];

  memcpy(lambdaN,lambdaK,sizeof(double)*n_wavel);
  memcpy(lambdaS,lambdaK,sizeof(double)*n_wavel);

  memcpy(refN,ref,sizeof(double)*n_wavel);
  memcpy(refS,ref,sizeof(double)*n_wavel);

  nRefS=0;
  *pRefNormN=*pRefNormS=(double)1.;

  // Buffer allocation

  if ((refList=(GDP_BIN_REF *)MEMORY_AllocBuffer("GdpBinRefSelection ","refList",gdpBinTotalRecordNumber,sizeof(GDP_BIN_REF),0,MEMORY_TYPE_STRUCT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // Search for records matching latitudes and SZA conditions

    if (latDelta>EPSILON)                                                       // a latitude range is specified
     {
      // search for potential reference spectra in northern hemisphere

      if ((nRefN=nRefS=GdpBinRefLat(refList,latMin,latMax,lonMin,lonMax,sza,szaDelta,gomePixelType))>0)
       rc=GdpBinBuildRef(refList,nRefN,nSpectra,lambdaN,refN,pEngineContext,specFp,&indexLine,responseHandle);

      if (!rc)
       memcpy(refS,refN,sizeof(double)*n_wavel);
     }

    // Search for records matching SZA conditions only

    else
     {
      if ((nRefN=nRefS=GdpBinRefSza(refList,sza,szaDelta,gomePixelType))>0)
       rc=GdpBinBuildRef(refList,nRefN,nSpectra,lambdaN,refN,pEngineContext,specFp,&indexLine,responseHandle);

      if (!rc)
       memcpy(refS,refN,sizeof(double)*n_wavel);
     }

    if (!rc)
     {
      // No reference spectrum is found for both hemispheres -> error message

      if (!nRefN && !nRefS)
       rc=ERROR_SetLast("GdpBinRefSelection",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"the orbit",pEngineContext->fileInfo.fileName);

      // No reference spectrum found for Northern hemisphere -> use the reference found for Southern hemisphere

      else if (!nRefN)
       {
       	mediateResponseCellDataString(plotPageRef,indexLine++,indexColumn,"No record selected for the northern hemisphere, use reference of the southern hemisphere",responseHandle);
        memcpy(refN,refS,sizeof(double)*n_wavel);
       }

      // No reference spectrum found for Southern hemisphere -> use the reference found for Northern hemisphere

      else if (!nRefS)
       {
        mediateResponseCellDataString(plotPageRef,indexLine++,indexColumn,"No record selected for the southern hemisphere, use reference of the northern hemisphere",responseHandle);
        memcpy(refS,refN,sizeof(double)*n_wavel);
       }

      if (nRefN || nRefS)   // if nor record selected, use ref (normalized as loaded)
       {
        VECTOR_NormalizeVector(refN-1,n_wavel,pRefNormN,"GdpBinRefSelection (refN) ");
        VECTOR_NormalizeVector(refS-1,n_wavel,pRefNormS,"GdpBinRefSelection (refS) ");
       }
     }
   }

  ANALYSE_indexLine=indexLine+1;

  // Release allocated buffer

  if (refList!=NULL)
   MEMORY_ReleaseBuffer("GdpBinRefSelection ","refList",refList);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GdpBinNewRef
// -----------------------------------------------------------------------------
// PURPOSE       In automatic reference selection, search for reference spectra
//
// INPUT         pEngineContext    hold the configuration of the current project
//               specFp       pointer to the current file
//
// RETURN        ERROR_ID_ALLOC if something failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC GdpBinNewRef(ENGINE_CONTEXT *pEngineContext,FILE *specFp,void *responseHandle)
 {
  // Declarations

  INDEX indexFeno;                                                              // browse analysis windows
  FENO *pTabFeno;                                                               // current analysis window
  RC rc;                                                                        // return code

  // Initializations

  rc=EngineCopyContext(&ENGINE_contextRef,pEngineContext);                     // perform a backup of the pEngineContext structure

  memset(OUTPUT_refFile,0,DOAS_MAX_PATH_LEN+1);
  OUTPUT_nRec=0;

  if (ENGINE_contextRef.recordNumber==0)
   rc=ERROR_ID_ALLOC;
  else

   // Browse analysis windows

   for (indexFeno=0;(indexFeno<NFeno) && (rc<THREAD_EVENT_STOP);indexFeno++)
    {
     pTabFeno=&TabFeno[0][indexFeno];

     if ((pTabFeno->hidden!=1) &&
         (pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC) &&
         (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC))

      // Build reference spectra according to latitudes and SZA conditions

      rc=GdpBinRefSelection(&ENGINE_contextRef,
                             specFp,
                             pTabFeno->refLatMin,pTabFeno->refLatMax,
                             pTabFeno->refLonMin,pTabFeno->refLonMax,
                             pTabFeno->refSZA,pTabFeno->refSZADelta,
                             pTabFeno->nspectra,
                             pTabFeno->LambdaK,pTabFeno->Sref,
                             pTabFeno->LambdaN,pTabFeno->SrefN,&pTabFeno->refNormFactN,
                             pTabFeno->LambdaS,pTabFeno->SrefS,&pTabFeno->refNormFactS,
                             pTabFeno->gomePixelType,
                             responseHandle);
    }

  // Return

  return rc;
 }

// ========
// ANALYSIS
// ========

// -----------------------------------------------------------------------------
// FUNCTION      GDP_BIN_LoadAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Load analysis parameters depending on the irradiance spectrum
//
// INPUT         pEngineContext    data on the current file
//               specFp       pointer to the current file
//
// RETURN        0 for success
// -----------------------------------------------------------------------------

RC GDP_BIN_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,FILE *specFp,void *responseHandle)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX indexFeno,indexTabCross,indexWindow,i,j;                                // indexes for loops and array
  CROSS_REFERENCE *pTabCross;                                                   // pointer to the current cross section
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a symbol
  FENO *pTabFeno;                                                               // pointer to the current spectral analysis window
  double factTemp,lambdaMin,lambdaMax;                                          // working variables
  int DimL,useUsamp,useKurucz,saveFlag;                                         // working variables
  RC rc;                                                                        // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;

  saveFlag=(int)pEngineContext->project.spectra.displayDataFlag;
  pOrbitFile=&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];

  const int n_wavel = NDET[0];

  if (!(rc=pOrbitFile->rc) && (gdpBinLoadReferenceFlag || !pEngineContext->analysisRef.refAuto))
   {
    lambdaMin=(double)9999.;
    lambdaMax=(double)-9999.;

    rc=ERROR_ID_NO;
    useKurucz=useUsamp=0;

    // Browse analysis windows and load missing data

    for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
      {
       pTabFeno=&TabFeno[0][indexFeno];
       pTabFeno->NDET=n_wavel;

       // Load calibration and reference spectra

       if (!pTabFeno->gomeRefFlag)
        {
         memcpy(pTabFeno->LambdaRef,pEngineContext->buffers.lambda,sizeof(double)*n_wavel);

         for (i=0,j=pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex];
              j<pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex]+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;i++,j++)
          {
           pTabFeno->Sref[i]=(double)pOrbitFile->gdpBinReference[j]/pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].scalingFactor;
           if (pRecord->useErrors && pTabFeno->SrefSigma!=NULL)
            pTabFeno->SrefSigma[i]=(double)pOrbitFile->gdpBinRefError[j]/pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].scalingError;
          }

         if (!TabFeno[0][indexFeno].hidden)
          {
           if (!(rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&pTabFeno->refNormFact,"GDP_BIN_LoadAnalysis (Reference) ")) &&
              (!pRecord->useErrors || !(rc=VECTOR_NormalizeVector(pTabFeno->SrefSigma-1,pTabFeno->NDET,&factTemp,"GDP_BIN_LoadAnalysis (RefError) "))))
            {
             memcpy(pTabFeno->SrefEtalon,pTabFeno->Sref,sizeof(double)*pTabFeno->NDET);
             pTabFeno->useEtalon=pTabFeno->displayRef=1;

             // Browse symbols

             for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
              {
               pTabCross=&pTabFeno->TabCross[indexTabCross];
               pWrkSymbol=&WorkSpace[pTabCross->Comp];

               // Cross sections and predefined vectors

               if ((((pWrkSymbol->type==WRK_SYMBOL_CROSS) && (pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING)) ||
                    ((pWrkSymbol->type==WRK_SYMBOL_PREDEFINED) &&
                    ((indexTabCross==pTabFeno->indexCommonResidual) ||
                   (((indexTabCross==pTabFeno->indexUsamp1) || (indexTabCross==pTabFeno->indexUsamp2)) && (pUsamp->method==PRJCT_USAMP_FILE))))) &&
                    ((rc=ANALYSE_CheckLambda(pWrkSymbol,pTabFeno->LambdaRef,pTabFeno->NDET))!=ERROR_ID_NO))

                goto EndGOME_LoadAnalysis;
              }

             // Gaps : rebuild subwindows on new wavelength scale

             doas_spectrum *new_range = spectrum_new();
             for (indexWindow = 0, DimL=0; indexWindow < pTabFeno->fit_properties.Z; indexWindow++)
              {
               int pixel_start = FNPixel(pTabFeno->LambdaRef,pTabFeno->fit_properties.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
               int pixel_end = FNPixel(pTabFeno->LambdaRef,pTabFeno->fit_properties.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

               spectrum_append(new_range, pixel_start, pixel_end);

               DimL += pixel_end - pixel_start +1;
              }

             // Buffers allocation
             FIT_PROPERTIES_free("GDP_BIN_LoadAnalysis",&pTabFeno->fit_properties);
             pTabFeno->fit_properties.DimL=DimL;
             FIT_PROPERTIES_alloc("GDP_BIN_LoadAnalysis",&pTabFeno->fit_properties);
             // new spectral windows
             pTabFeno->fit_properties.specrange = new_range;

             if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,0))!=ERROR_ID_NO) ||
                 ((!pKuruczOptions->fwhmFit || !pTabFeno->useKurucz) && pTabFeno->xsToConvolute &&
                 ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,ANALYSIS_slitMatrix,ANALYSIS_slitParam,pSlitOptions->slitFunction.slitType,0,pSlitOptions->slitFunction.slitWveDptFlag))!=ERROR_ID_NO)))
              goto EndGOME_LoadAnalysis;

             pTabFeno->Decomp=1;
            }
          }

         memcpy(pTabFeno->LambdaK,pTabFeno->LambdaRef,sizeof(double)*n_wavel);
         memcpy(pTabFeno->Lambda,pTabFeno->LambdaRef,sizeof(double)*n_wavel);

         useUsamp+=pTabFeno->useUsamp;
         useKurucz+=pTabFeno->useKurucz;

         if (pTabFeno->useUsamp)
          {
           if (pTabFeno->LambdaRef[0]<lambdaMin)
            lambdaMin=pTabFeno->LambdaRef[0];
           if (pTabFeno->LambdaRef[n_wavel-1]>lambdaMax)
            lambdaMax=pTabFeno->LambdaRef[n_wavel-1];
          }
        }
      }

    // Kurucz

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
     {
      KURUCZ_Init(0,0);

      if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,responseHandle,0))!=ERROR_ID_NO))
       goto EndGOME_LoadAnalysis;
     }

    // Build undersampling cross sections

    if (useUsamp && (THRD_id!=THREAD_TYPE_KURUCZ))
     {
       // ANALYSE_UsampLocalFree();

       if ( (rc=ANALYSE_UsampLocalAlloc(0) )!=ERROR_ID_NO ||
            (rc=ANALYSE_UsampBuild(0,0))!=ERROR_ID_NO ||
            (rc=ANALYSE_UsampBuild(1,ITEM_NONE))!=ERROR_ID_NO)

       goto EndGOME_LoadAnalysis;
     }

    // Reference

    if ((THRD_id==THREAD_TYPE_ANALYSIS) && gdpBinLoadReferenceFlag && !(rc=GdpBinNewRef(pEngineContext,specFp,responseHandle)) &&
       !(rc=ANALYSE_AlignReference(pEngineContext,2,responseHandle,0)))  // automatic ref selection for Northern hemisphere
     rc=ANALYSE_AlignReference(pEngineContext,3,responseHandle,0);       // automatic ref selection for Southern hemisphere

    if (rc==ERROR_ID_NO_REF)
     for (i=GDP_BIN_currentFileIndex+1;i<gdpBinOrbitFilesN;i++)
      GDP_BIN_orbitFiles[i].rc=rc;
   }

  // Return

  EndGOME_LoadAnalysis :

  return rc;
 }

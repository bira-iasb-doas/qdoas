
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
//  Module purpose    :  GOME interface (calibrated level 1 data files in the modified IASB-BIRA format)
//  Name of module    :  GDP_BIN_Read.C
//  Program Language  :  Borland C++ 5.0 for Windows 95/NT
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
//  GdpBinLembda - build a wavelength calibration (irradiance or
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

#include "doas.h"

// ====================
// CONSTANTS DEFINITION
// ====================

#define	OFFSET	                    (double) 500.0
#define	PARAMETER	                 (double)5000.0
#define EPSILON                    (double)   1.e-5

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

UCHAR *GDP_BIN_BandStrTab[SCIENCE_DATA_DEFINED+1] =
 {
  "All bands",
  "Band 1a", "Band 1b", "Band 2a", "Band 2b", "Band 3",
  "Band 4","Blind 1a", "Straylight 1a",
  "Straylight 1b", "Straylight 2a"
 };

GOME_ORBIT_FILE GDP_BIN_orbitFiles[MAX_GOME_FILES];                             // list of files for an orbit
INDEX GDP_BIN_currentFileIndex=ITEM_NONE;                                       // index of the current file in the list
static int gdpBinOrbitFilesN=0;                                                 // the total number of files for the current orbit
static INT gdpBinTotalRecordNumber;                                             // total number of records for an orbit
static INT gdpBinLoadReferenceFlag=0;

// ==========
// PROTOTYPES
// ==========

INDEX GdpBinSortGetIndex(double value,int flag,int listSize,INDEX fileIndex);
void GdpBinSort(INDEX indexRecord,int flag,int listSize,INDEX fileIndex);

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

  for (gomeOrbitFileIndex=0;gomeOrbitFileIndex<gdpBinOrbitFilesN;gomeOrbitFileIndex++)
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

INDEX GDP_BIN_GetRecordNumber(INT pixelNumber)
 {
  return GdpBinSortGetIndex(pixelNumber,2,GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinHeader.nspectra,GDP_BIN_currentFileIndex)+1;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GdpBinLembda
// -----------------------------------------------------------------------------
// PURPOSE       Build a wavelength calibration (irradiance or
//               earthshine spectra) using a set of coefficients of polynomial
//
// INPUT         indexParam    index of the set of parameters to use
//               fileIndex   index of the file for the current orbit
//
// OUTPUT        lembda        the wavelength calibration
// -----------------------------------------------------------------------------

void GdpBinLembda(double *lembda,INT indexParam,INDEX fileIndex)
 {
  // Declarations

  INT offset;                                                                   // offset in bytes
  double lembdax;                                                               // wavelength evaluated by polynomial for a given pixel
  INDEX i,j;
  GOME_ORBIT_FILE *pOrbitFile;

  pOrbitFile=&GDP_BIN_orbitFiles[fileIndex];

  for (i=0,j=pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex];
       j<pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex]+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;i++,j++)

   lembda[i]=(double)EvalPolynom_d((double)i+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].startDetector,
                    &pOrbitFile->gdpBinCoeff[pOrbitFile->gdpBinHeader.nSpectralParam*SPECTRAL_FITT_ORDER*pOrbitFile->gdpBinBandIndex+
                     indexParam*SPECTRAL_FITT_ORDER],SPECTRAL_FITT_ORDER);

  if (pOrbitFile->gdpBinHeader.version>=4)
   {
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
     	lembdax=(double)EvalPolynom_d((double)i+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].startDetector-offset,
                             &pOrbitFile->gdpBinCoeff[pOrbitFile->gdpBinHeader.nSpectralParam*SPECTRAL_FITT_ORDER*pOrbitFile->gdpBinBandIndex+
                              indexParam*SPECTRAL_FITT_ORDER],SPECTRAL_FITT_ORDER);

      lembda[i]-=(double)EvalPolynom_d((double)(lembdax-OFFSET)/PARAMETER,&pOrbitFile->gdpBinCoeff[pOrbitFile->gdpBinHeader.nSpectralParam*SPECTRAL_FITT_ORDER*pOrbitFile->gdpBinHeader.nbands+MAX_FITT_ORDER*pOrbitFile->gdpBinBandIndex],MAX_FITT_ORDER);
			  }
			}
	}

// -----------------------------------------------------------------------------
// FUNCTION      GDP_BIN_Set
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve information on data saved in the file from the header
//
// INPUT         specFp    pointer to the current GOME orbit file
//
// INPUT/OUTPUT  pSpecInfo interface for file operations
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  if the file is not found;
//               ERROR_ID_FILE_EMPTY      if the file is empty;
//               ERROR_ID_ALLOC           if allocation of a buffer failed;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC GDP_BIN_Set(SPEC_INFO *pSpecInfo,FILE *specFp)
 {
  // Declarations

  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  UCHAR filePath[MAX_STR_SHORT_LEN+1];
  UCHAR fileFilter[MAX_STR_SHORT_LEN+1];
  #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
  WIN32_FIND_DATA fileInfo;                                                     // structure returned by FindFirstFile and FindNextFile APIs
  HANDLE hDir;                                                                  // handle to use with by FindFirstFile and FindNextFile APIs
  #else
  struct dirent *fileInfo;
  DIR *hDir;
  #endif
  INDEX indexFile;
  UCHAR *ptr;
  UCHAR fileName[MAX_ITEM_TEXT_LEN+1];                                          // file name
  INDEX i,indexRecord;                                                          // indexes for loops and arrays
  int useErrors,errorFlag;                                                      // 0 if errors are saved with spectra, 1 otherwise
  FILE *fp;
  RC rc;                                                                        // return code

  // Initializations

  if (specFp!=NULL)
   {
    fclose(specFp);
    specFp=NULL;
   }

  pSpecInfo->recordNumber=0;
  gdpBinLoadReferenceFlag=0;

  GDP_BIN_currentFileIndex=ITEM_NONE;
  ANALYSE_oldLatitude=(double)99999.;                                           // in automatic reference selection, force the selection of a
  strcpy(fileName,pSpecInfo->fileName);                                         // new reference spectrum
  NDET=1024;
  rc=ERROR_ID_NO;

  // In automatic reference selection, the file has maybe already loaded

  if ((THRD_id==THREAD_TYPE_ANALYSIS) && ANALYSE_refSelectionFlag)
   {
    for (indexFile=0;indexFile<gdpBinOrbitFilesN;indexFile++)
     if ((strlen(pSpecInfo->fileName)==strlen(GDP_BIN_orbitFiles[indexFile].gdpBinFileName)) &&
         !STD_Stricmp(pSpecInfo->fileName,GDP_BIN_orbitFiles[indexFile].gdpBinFileName))
      break;

    if (indexFile<gdpBinOrbitFilesN)
     GDP_BIN_currentFileIndex=indexFile;
   }

  if (GDP_BIN_currentFileIndex==ITEM_NONE)
   {
   	// Release old buffers

   	GDP_BIN_ReleaseBuffers();

   	// Get the number of files to load

   	if ((THRD_id==THREAD_TYPE_ANALYSIS) && ANALYSE_refSelectionFlag)
    	{
    		gdpBinLoadReferenceFlag=1;

    		if (ANALYSE_lonSelectionFlag)
       {
    		  // Get file path

     	  strcpy(filePath,pSpecInfo->fileName);

     	  if ((ptr=strrchr(filePath,PATH_SEP))==NULL)
    	   	strcpy(filePath,".");
    	   else
    	    *ptr=0;

   	 	  // Build file filter

   	 	  strcpy(fileFilter,pSpecInfo->fileName);
   	 	  if ((ptr=strrchr(fileFilter,PATH_SEP))==NULL)
   	 	   ptr=fileFilter;
   	 	  else
   	 	   ptr=ptr+1;

        strcpy(ptr,"*.*");

        // Search for files of the same orbit

        #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_

        for (hDir=FindFirstFile(fileFilter,&fileInfo),rc=1;
            (hDir!=INVALID_HANDLE_VALUE) && (rc!=0) && (gdpBinOrbitFilesN<MAX_GOME_FILES);rc=FindNextFile(hDir,&fileInfo))

         if ((fileInfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==0)
          {
          	sprintf(GDP_BIN_orbitFiles[gdpBinOrbitFilesN].gdpBinFileName,"%s%c%s",filePath,PATH_SEP,fileInfo.cFileName);
           gdpBinOrbitFilesN++;
          }

        // Close handle

        if (hDir!=NULL)
         FindClose(hDir);

        #else

        for (hDir=opendir(fileFilter);(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);)
          {
            sprintf(GDP_BIN_orbitFiles[gdpBinOrbitFilesN].gdpBinFileName,"%s/%s",filePath,fileInfo->d_name);
            if ( STD_IsDir(GDP_BIN_orbitFiles[gdpBinOrbitFilesN].gdpBinFileName) == 1 )
               gdpBinOrbitFilesN++;
          }

        if ( hDir != NULL ) closedir(hDir);

        #endif

        rc=ERROR_ID_NO;
       }
      else
       {
       	gdpBinOrbitFilesN=1;
       	strcpy(GDP_BIN_orbitFiles[0].gdpBinFileName,pSpecInfo->fileName);
       }
   	 }
   	else
     {
     	gdpBinOrbitFilesN=1;
     	strcpy(GDP_BIN_orbitFiles[0].gdpBinFileName,pSpecInfo->fileName);
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

        if (pOrbitFile->gdpBinHeader.version>=4)
         pOrbitFile->gdpBinCoeffSize+=MAX_FITT_ORDER*pOrbitFile->gdpBinHeader.nbands;

        // Get the size of spectra band per band

        for (i=1,pOrbitFile->gdpBinStartPixel[0]=0,pOrbitFile->gdpBinSpectraSize=pOrbitFile->gdpBinBandInfo[0].bandSize;
             i<pOrbitFile->gdpBinHeader.nbands;i++)
         {
          pOrbitFile->gdpBinStartPixel[i]=pOrbitFile->gdpBinStartPixel[i-1]+pOrbitFile->gdpBinBandInfo[i-1].bandSize;
          pOrbitFile->gdpBinSpectraSize+=pOrbitFile->gdpBinBandInfo[i].bandSize;
         }

        for (i=0;(i<pOrbitFile->gdpBinHeader.nbands);i++)
         {
         if (pOrbitFile->gdpBinBandInfo[i].bandType==pSpecInfo->project.instrumental.user)
           pOrbitFile->gdpBinBandIndex=i;
         }

        // Buffers allocation

        if ((pOrbitFile->gdpBinBandIndex==ITEM_NONE) || !pOrbitFile->gdpBinCoeffSize || !pOrbitFile->gdpBinSpectraSize || !pOrbitFile->gdpBinHeader.nspectra)
         rc=ERROR_SetLast("GDP_Bin_Set (1)",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);

        else if (((pOrbitFile->gdpBinCoeff=(double *)MEMORY_AllocDVector("GDP_BIN_Set ","pOrbitFile->gdpBinCoeff",0,pOrbitFile->gdpBinCoeffSize-1))==NULL) ||
                 ((pOrbitFile->gdpBinReference=(USHORT *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinReference",pOrbitFile->gdpBinSpectraSize,sizeof(USHORT),0,MEMORY_TYPE_USHORT))==NULL) ||
                 ((pOrbitFile->gdpBinRefError=(USHORT *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinRefError",pOrbitFile->gdpBinSpectraSize,sizeof(USHORT),0,MEMORY_TYPE_USHORT))==NULL) ||
                 ((pOrbitFile->gdpBinInfo=(GDP_BIN_INFO *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinInfo",pOrbitFile->gdpBinHeader.nspectra,sizeof(GDP_BIN_INFO),0,MEMORY_TYPE_STRUCT))==NULL) ||
                 ((pOrbitFile->gdpBinSzaIndex=(INDEX *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinSzaIndex",pOrbitFile->gdpBinHeader.nspectra,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
                 ((pOrbitFile->gdpBinLatIndex=(INDEX *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinLatIndex",pOrbitFile->gdpBinHeader.nspectra,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
                 ((pOrbitFile->gdpBinLonIndex=(INDEX *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinLonIndex",pOrbitFile->gdpBinHeader.nspectra,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
                 ((pOrbitFile->gdpBinPixIndex=(INDEX *)MEMORY_AllocBuffer("GDP_BIN_Set ","pOrbitFile->gdpBinPixIndex",pOrbitFile->gdpBinHeader.nspectra,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL))

         rc=ERROR_ID_ALLOC;

        else
         {
          NDET=pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;

          // Read the irradiance spectrum

          fread(pOrbitFile->gdpBinCoeff,sizeof(double)*pOrbitFile->gdpBinCoeffSize,1,fp);
          fread(pOrbitFile->gdpBinReference,sizeof(unsigned short)*pOrbitFile->gdpBinSpectraSize,1,fp);

          if (useErrors)
           fread(pOrbitFile->gdpBinRefError,sizeof(unsigned short)*pOrbitFile->gdpBinSpectraSize,1,fp);

          pOrbitFile->specNumber=((THRD_id==THREAD_TYPE_SPECTRA) && (THRD_browseType==THREAD_BROWSE_DARK))?1:pOrbitFile->gdpBinHeader.nspectra;

          if (THRD_browseType!=THREAD_BROWSE_DARK)
           for (indexRecord=0;indexRecord<pOrbitFile->specNumber;indexRecord++)
            {
             fseek(fp,(LONG)pOrbitFile->gdpBinHeader.headerSize+indexRecord*pOrbitFile->gdpBinHeader.recordSize,SEEK_SET);

             if (!fread(&pOrbitFile->gdpBinSpectrum,sizeof(SPECTRUM_RECORD),1,fp))
              rc=ERROR_SetLast("GDP_Bin_Set (2)",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);
             else
              {
               pOrbitFile->gdpBinInfo[indexRecord].pixelNumber=pOrbitFile->gdpBinSpectrum.groundPixelID;
               pOrbitFile->gdpBinInfo[indexRecord].pixelType=pOrbitFile->gdpBinSpectrum.groundPixelType;

               if (pOrbitFile->gdpBinHeader.version<2)
                {
                 pOrbitFile->gdpBinInfo[indexRecord].sza=(double)pOrbitFile->gdpBinSpectrum.geo.geo1.szaArray[1];
                 pOrbitFile->gdpBinInfo[indexRecord].lat=(double)pOrbitFile->gdpBinSpectrum.geo.geo1.latArray[4];
                 pOrbitFile->gdpBinInfo[indexRecord].lon=(double)pOrbitFile->gdpBinSpectrum.geo.geo1.lonArray[4];
                }
               else if (pOrbitFile->gdpBinHeader.version==2)
                {
                 pOrbitFile->gdpBinInfo[indexRecord].sza=(double)pOrbitFile->gdpBinSpectrum.geo.geo2.szaArray[1];
                 pOrbitFile->gdpBinInfo[indexRecord].lat=(double)0.01*pOrbitFile->gdpBinSpectrum.geo.geo2.latArray[4];
                 pOrbitFile->gdpBinInfo[indexRecord].lon=(double)0.01*pOrbitFile->gdpBinSpectrum.geo.geo2.lonArray[4];
                }
               else if (pOrbitFile->gdpBinHeader.version==3)
                {
                 pOrbitFile->gdpBinInfo[indexRecord].sza=(double)pOrbitFile->gdpBinSpectrum.geo.geo3.szaArray[1];
                 pOrbitFile->gdpBinInfo[indexRecord].lat=(double)0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.latArray[4];
                 pOrbitFile->gdpBinInfo[indexRecord].lon=(double)0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.lonArray[4];
                }
               else
                {
                 pOrbitFile->gdpBinInfo[indexRecord].sza=(double)pOrbitFile->gdpBinSpectrum.geo.geo3.szaArray[1];
                 pOrbitFile->gdpBinInfo[indexRecord].lat=(double)0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.latArray[4];
                 pOrbitFile->gdpBinInfo[indexRecord].lon=(double)0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.lonArray[4];
                }

    /*             {
                    FILE *fp;
                    UCHAR *ptr;
                    UCHAR newFile[256];

                    ptr=strrchr(fileName,PATH_SEP);
                    sprintf(newFile,"F:\\GOME_Applications\\GEO Tests\\MAT2\\%s",ptr+1);
                    ptr=strrchr(newFile,'.');
                    strcpy(ptr,".asc");

                    if ((fp=fopen(newFile,"rt"))==NULL)
                     {
                      fp=fopen(newFile,"w+t");
                      fprintf(fp,"PixelN\tPixelT\tSZA(A)\tSZA(B)\tSZA(C)\tLZA(A)\tLZA(B)\tLZA(C)\tLAZ(A)\tLAZ(B)\tLAZ(C)\tLAT(0)\tLAT(1)\tLAT(2)\tLAT(3)\tLAT(4)\tLON(0)\tLON(1)\tLON(2)\tLON(3)\tLON(4)\t\n");
                     }

                    fclose(fp);

                    fp=fopen(newFile,"a+t");
                    fprintf(fp,"%#5d %#5d %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f %#8.3f \n",
                                pOrbitFile->gdpBinSpectrum.groundPixelID,pOrbitFile->gdpBinSpectrum.groundPixelType,
                                pOrbitFile->gdpBinSpectrum.geo.geo3.szaArray[0],pOrbitFile->gdpBinSpectrum.geo.geo3.szaArray[1],pOrbitFile->gdpBinSpectrum.geo.geo3.szaArray[2],
                                0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.losZa[0],0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.losZa[1],0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.losZa[2],
                                0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.losAzim[0],0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.losAzim[1],0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.losAzim[2],
                                0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.latArray[0],0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.latArray[1],0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.latArray[2],0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.latArray[3],0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.latArray[4],
                                0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.lonArray[0],0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.lonArray[1],0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.lonArray[2],0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.lonArray[3],0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.lonArray[4]);
                    fclose(fp);
                   } */

               GdpBinSort(indexRecord,0,indexRecord,indexFile);                 // sort latitudes
               GdpBinSort(indexRecord,1,indexRecord,indexFile);                 // sort SZA
               GdpBinSort(indexRecord,2,indexRecord,indexFile);                 // sort pixel numbers
               GdpBinSort(indexRecord,3,indexRecord,indexFile);                 // sort longitudes
              }
            }
         }

        fclose(fp);
       }

      if ((strlen(pSpecInfo->fileName)==strlen(pOrbitFile->gdpBinFileName)) &&
          !STD_Stricmp(pSpecInfo->fileName,pOrbitFile->gdpBinFileName))
       GDP_BIN_currentFileIndex=indexFile;

      gdpBinTotalRecordNumber+=pOrbitFile->specNumber;

      if (rc!=ERROR_ID_NO)
       pOrbitFile->rc=rc;

      rc=ERROR_ID_NO;
     }
   }

  if ((GDP_BIN_currentFileIndex!=ITEM_NONE) &&
     ((pSpecInfo->recordNumber=GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].specNumber)>0) &&
     !rc)
   {
    pOrbitFile=&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];

    pSpecInfo->gome.orbitNumber=pOrbitFile->gdpBinHeader.orbitNumber;
    pSpecInfo->useErrors=((pOrbitFile->gdpBinHeader.mask&GDP_BIN_ERROR_ID_MASK)==GDP_BIN_ERROR_ID_MASK)?1:0;
    NDET=pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;

    GdpBinLembda(pSpecInfo->lembda,GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinHeader.indexSpectralParam,GDP_BIN_currentFileIndex);

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
// INPUT/OUTPUT  pSpecInfo    interface for file operations
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  if the file is not found;
//               ERROR_ID_FILE_END        the end of the file is reached;
//               ERROR_ID_FILE_RECORD     the record doesn't satisfy user constraints
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif

RC GDP_BIN_Read(SPEC_INFO *pSpecInfo,int recordNo,FILE *specFp,INDEX indexFile)
 {
  // Declarations

  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  unsigned short *spectrum,*specError;                                          // spectra are in short integers instead of double in the binary format
  SHORT_DATE today;                                                             // date of measurements
  double Max;                                                                   // maximum of the spectrum
  INDEX i,j;                                                                    // indexes for loops and arrays
  FILE *fp;
  RC rc;                                                                        // return code

  // Initializations

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

  // Irradiance spectrum

  else if ((THRD_id==THREAD_TYPE_SPECTRA) && (THRD_browseType==THREAD_BROWSE_DARK))

   for (i=0,j=pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex];
        j<pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex]+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;i++,j++)
    {
     pSpecInfo->spectrum[i]=(double)pOrbitFile->gdpBinReference[j]/pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].scalingFactor;
     pSpecInfo->sigmaSpec[i]=(pSpecInfo->useErrors)?(double)pOrbitFile->gdpBinRefError[j]/pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].scalingError:pSpecInfo->spectrum[i];
    }

  // Buffers allocation

  else if (((spectrum=(unsigned short *)MEMORY_AllocBuffer("GDP_BIN_Read ","spectrum",pOrbitFile->gdpBinSpectraSize,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
            (pSpecInfo->useErrors &&
           ((specError=(unsigned short *)MEMORY_AllocBuffer("GDP_BIN_Read ","specError",pOrbitFile->gdpBinSpectraSize,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL)))

  rc=ERROR_ID_ALLOC;

  else
   {
    // Complete record read out

    fseek(fp,(LONG)pOrbitFile->gdpBinHeader.headerSize+(recordNo-1)*pOrbitFile->gdpBinHeader.recordSize,SEEK_SET);

    if (!fread(&pOrbitFile->gdpBinSpectrum,sizeof(SPECTRUM_RECORD),1,fp) ||
        !fread(pOrbitFile->gdpBinScalingFactor,sizeof(float)*pOrbitFile->gdpBinHeader.nbands,1,fp) ||
        !fread(spectrum,sizeof(unsigned short)*pOrbitFile->gdpBinSpectraSize,1,fp) ||
       (pSpecInfo->useErrors &&
       (!fread(pOrbitFile->gdpBinScalingError,sizeof(float)*pOrbitFile->gdpBinHeader.nbands,1,fp) ||
        !fread(specError,sizeof(unsigned short)*pOrbitFile->gdpBinSpectraSize,1,fp))))

     rc=ERROR_ID_FILE_END;

    else
     {
      today.da_year=pOrbitFile->gdpBinSpectrum.dateAndTime.da_year;
      today.da_mon=pOrbitFile->gdpBinSpectrum.dateAndTime.da_mon;
      today.da_day=pOrbitFile->gdpBinSpectrum.dateAndTime.da_day;

      if (today.da_year<30)
       today.da_year+=(short)2000;
      else if (today.da_year<130)
       today.da_year+=(short)1900;
      else if (today.da_year<1930)
       today.da_year+=(short)100;

      // Fill fields of structure

      pSpecInfo->Tint     = (double)0.;
      pSpecInfo->NSomme   = 0;

      if (pOrbitFile->gdpBinHeader.version<2)
       {
        pSpecInfo->Zm       = (double)pOrbitFile->gdpBinSpectrum.geo.geo1.szaArray[1];
        pSpecInfo->longitude=pOrbitFile->gdpBinSpectrum.geo.geo1.lonArray[4];
        pSpecInfo->latitude =pOrbitFile->gdpBinSpectrum.geo.geo1.latArray[4];
       }
      else if (pOrbitFile->gdpBinHeader.version==2)
       {
        pSpecInfo->Zm       = (double)pOrbitFile->gdpBinSpectrum.geo.geo2.szaArray[1];
        pSpecInfo->Azimuth  = (double)pOrbitFile->gdpBinSpectrum.geo.geo2.azim;
        pSpecInfo->zenithViewAngle=pOrbitFile->gdpBinSpectrum.geo.geo2.losZa;
        pSpecInfo->azimuthViewAngle=pOrbitFile->gdpBinSpectrum.geo.geo2.losAzim;

        pSpecInfo->longitude=0.01*pOrbitFile->gdpBinSpectrum.geo.geo2.lonArray[4];
        pSpecInfo->latitude =0.01*pOrbitFile->gdpBinSpectrum.geo.geo2.latArray[4];
       }
      else // if (pOrbitFile->gdpBinHeader.version==3)
       {
        pSpecInfo->Zm       = (double)pOrbitFile->gdpBinSpectrum.geo.geo3.szaArray[1];

        if (pOrbitFile->gdpBinHeader.version>=5)
         pSpecInfo->Azimuth  = (double)pOrbitFile->gdpBinSpectrum.aziArray[1];

        pSpecInfo->longitude=0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.lonArray[4];
        pSpecInfo->latitude =0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.latArray[4];

        pSpecInfo->zenithViewAngle=(float)(0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.losZa[1]);
        pSpecInfo->azimuthViewAngle=(float)(0.01*pOrbitFile->gdpBinSpectrum.geo.geo3.losAzim[1]);
       }

      pSpecInfo->present_time.ti_hour=pOrbitFile->gdpBinSpectrum.dateAndTime.ti_hour;
      pSpecInfo->present_time.ti_min=pOrbitFile->gdpBinSpectrum.dateAndTime.ti_min;
      pSpecInfo->present_time.ti_sec=pOrbitFile->gdpBinSpectrum.dateAndTime.ti_sec;

      memset(pSpecInfo->Nom,0,20);
      memcpy((char *)&pSpecInfo->present_day,(char *)&today,sizeof(SHORT_DATE));

      pSpecInfo->Tm=(double)ZEN_NbSec(&pSpecInfo->present_day,&pSpecInfo->present_time,0);
      pSpecInfo->localCalDay=ZEN_FNCaljda(&pSpecInfo->Tm);
      pSpecInfo->TotalExpTime=(double)0.;
      pSpecInfo->TimeDec=(double)pOrbitFile->gdpBinSpectrum.dateAndTime.ti_hour+
                                 pOrbitFile->gdpBinSpectrum.dateAndTime.ti_min/60.+
                                 pOrbitFile->gdpBinSpectrum.dateAndTime.ti_sec/3600.;

      pSpecInfo->gome.pixelNumber=pOrbitFile->gdpBinSpectrum.groundPixelID;
      pSpecInfo->gome.pixelType=pOrbitFile->gdpBinSpectrum.groundPixelType;

      pSpecInfo->altitude=(double)0.;

      // Convert spectrum from short integers to double

      GdpBinLembda(pSpecInfo->lembda,pOrbitFile->gdpBinSpectrum.indexSpectralParam,GDP_BIN_currentFileIndex);

      for (i=0,j=pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex],Max=(double)0.;
           j<pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex]+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;i++,j++)
       {
        pSpecInfo->spectrum[i]=(double)spectrum[j]/pOrbitFile->gdpBinScalingFactor[pOrbitFile->gdpBinBandIndex];
        pSpecInfo->sigmaSpec[i]=(pSpecInfo->useErrors)?(double)specError[i]/pOrbitFile->gdpBinScalingError[pOrbitFile->gdpBinBandIndex]:pSpecInfo->spectrum[i];

        if (pSpecInfo->spectrum[i]>Max)
         Max=pSpecInfo->spectrum[i];
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

INT GdpBinRefLat(GDP_BIN_REF *refList,double latMin,double latMax,double lonMin,double lonMax,double sza,double szaDelta,UCHAR *gomePixelType)
 {
  // Declarations

  INDEX fileIndex;
  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX ilatMin,ilatMax,ilatTmp,                                                // range of indexes of latitudes matching conditions in sorted list
        ilatIndex,                                                              // browse records with latitudes in the specified range
        indexRef;                                                               // browse reference already registered in order to keep the list sorted

  INT nRef;                                                                     // the number of spectra matching latitudes and SZA conditions
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

INT GdpBinRefSza(GDP_BIN_REF *refList,double sza,double szaDelta,UCHAR *gomePixelType)
 {
  // Declarations

  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX fileIndex;
  INDEX iszaMin,iszaMax,iszaTmp,                                                // range of indexes of SZA matching conditions in sorted list
        iszaIndex,                                                              // browse records with SZA in the specified SZA range
        indexRef;                                                               // browse reference already registered in order to keep the list sorted

  INT nRef;                                                                     // the number of spectra matching latitudes and SZA conditions
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
//               lembda       the grid of the irradiance spectrum
//               pSpecInfo    interface for file operations
//               specFp       pointer to the current file;
//               fp           pointer to the file dedicated to the display of information on selected spectra
//
// OUTPUT        ref          the new reference spectrum
//
// RETURN        code returned by GDP_BIN_Read if any;
//               ERROR_ID_NO_REF if no reference spectrum is found;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC GdpBinBuildRef(GDP_BIN_REF *refList,INT nRef,INT nSpectra,double *lembda,double *ref,SPEC_INFO *pSpecInfo,FILE *specFp,FILE *fp)
 {
  // Declarations

  GDP_BIN_REF *pRef;                                                            // pointer to the current reference spectrum
  INDEX     indexRef,                                                           // browse reference in the list
            indexFile,                                                          // browse files
            i;                                                                  // index for loop and arrays
  INT       nRec;                                                               // number of records used for the average
  RC        rc;                                                                 // return code

  // Initializations

  rc=ERROR_ID_NO;

  for (i=0;i<NDET;i++)
   lembda[i]=ref[i]=(double)0.;

  // Search for spectra matching latitudes and SZA conditions

  for (nRec=0,indexRef=0,indexFile=ITEM_NONE;
      (indexRef<nRef) && (nRec<nSpectra) && !rc;indexRef++)
   {
    pRef=&refList[indexRef];

    if (((indexFile==ITEM_NONE) || (pRef->indexFile==indexFile)) &&
       !(rc=GDP_BIN_Read(pSpecInfo,pRef->indexRecord+1,specFp,pRef->indexFile)))    // Read and accumulate selected radiances
     {
      if (fp!=NULL)
       fprintf(fp,"%s %#5d\t%#5d\t%#5d\t%#6.2lf\t%#6.2lf\t%#6.2lf\n",GDP_BIN_orbitFiles[pRef->indexFile].gdpBinFileNumber,pRef->indexRecord+1,pRef->pixelNumber,pRef->pixelType,pRef->sza,pRef->latitude,pRef->longitude);

      for (i=0;i<NDET;i++)
       {
        lembda[i]+=(double)pSpecInfo->lembda[i];
        ref[i]+=(double)pSpecInfo->spectrum[i];
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

    for (i=0;i<NDET;i++)
     {
      lembda[i]/=nRec;
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
// INPUT         pSpecInfo      collect information on the current spectrum;
//               specFp         pointer to the current file;
//               latMin,latMax  determine the range of latitudes;
//               lonMin,lonMax  determine the range of longitudes;
//               sza,szaDelta   determine the range of SZA;
//
//               nSpectra       the number of spectra to average to build the reference spectrum;
//               lembdaK,ref    reference spectrum to use if no spectrum in the orbit matches the sza and latitudes conditions;
//
// OUTPUT        lembdaN,refN   reference spectrum for northern hemisphere;
//               lembdaS,refS   reference spectrum for southern hemisphere.
//
// RETURN        ERROR_ID_ALLOC if the allocation of buffers failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC GdpBinRefSelection(SPEC_INFO *pSpecInfo,
                      FILE *specFp,
                      double latMin,double latMax,
                      double lonMin,double lonMax,
                      double sza,double szaDelta,
                      int nSpectra,
                      double *lembdaK,double *ref,
                      double *lembdaN,double *refN,
                      double *lembdaS,double *refS,UCHAR *gomePixelType)
 {
  // Declarations

  GDP_BIN_REF *refList;                                                         // list of potential reference spectra
  INT nRefN,nRefS;                                                              // number of reference spectra in the previous list resp. for Northern and Southern hemisphere
  double normFact;                                                              // normalisation factor
  double latDelta,tmp;
  FILE *fp;                                                                     // pointer to the temporary file with information to display
  RC rc;                                                                        // return code

  // Initializations

// QDOAS ???  fp=(pSpecInfo->project.spectra.displayDataFlag)?fopen(DOAS_tmpFile,"w+t"):NULL;
  fp=NULL;   // QDOAS ???

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

  memcpy(lembdaN,lembdaK,sizeof(double)*NDET);
  memcpy(lembdaS,lembdaK,sizeof(double)*NDET);

  memcpy(refN,ref,sizeof(double)*NDET);
  memcpy(refS,ref,sizeof(double)*NDET);

  nRefS=0;

  // Buffer allocation

  if ((refList=(GDP_BIN_REF *)MEMORY_AllocBuffer("GdpBinRefSelection ","refList",gdpBinTotalRecordNumber,sizeof(GDP_BIN_REF),0,MEMORY_TYPE_STRUCT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // Search for records matching latitudes and SZA conditions

    if (latDelta>EPSILON)                                                       // a latitude range is specified
     {
      // search for potential reference spectra in northern hemisphere

      if (fp!=NULL)
       {
        fprintf(fp,"Ref Selection :\n");
        fprintf(fp,"File       Rec\t  PixN\t  PixT\t  SZA\t  Lat\t  Lon\n");
       }

      if ((nRefN=nRefS=GdpBinRefLat(refList,latMin,latMax,lonMin,lonMax,sza,szaDelta,gomePixelType))>0)
       rc=GdpBinBuildRef(refList,nRefN,nSpectra,lembdaN,refN,pSpecInfo,specFp,fp);

      if (!rc)
       memcpy(refS,refN,sizeof(double)*NDET);
     }

    // Search for records matching SZA conditions only

    else
     {
      if (fp!=NULL)
       {
        fprintf(fp,"Ref Selection :\n");
        fprintf(fp,"File       Rec\t  PixN\t  PixT\t  SZA\t  Lat\t  Lon\n");
       }

      if ((nRefN=nRefS=GdpBinRefSza(refList,sza,szaDelta,gomePixelType))>0)
       rc=GdpBinBuildRef(refList,nRefN,nSpectra,lembdaN,refN,pSpecInfo,specFp,fp);

      if (!rc)
       memcpy(refS,refN,sizeof(double)*NDET);
     }

    if (!rc)
     {
      // No reference spectrum is found for both hemispheres -> error message

      if (!nRefN && !nRefS)
       rc=ERROR_SetLast("GdpBinRefSelection",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"the orbit",pSpecInfo->fileName);

      // No reference spectrum found for Northern hemisphere -> use the reference found for Southern hemisphere

      else if (!nRefN)
       {
        if (fp!=NULL)
         fprintf(fp,"No record selected for the northern hemisphere, use reference of the southern hemisphere\n");

        memcpy(refN,refS,sizeof(double)*NDET);
       }

      // No reference spectrum found for Southern hemisphere -> use the reference found for Northern hemisphere

      else if (!nRefS)
       {
        if (fp!=NULL)
         fprintf(fp,"No record selected for the southern hemisphere, use reference of the northern hemisphere\n");

        memcpy(refS,refN,sizeof(double)*NDET);
       }

      if (nRefN || nRefS)   // if nor record selected, use ref (normalized as loaded)
       {
        ANALYSE_NormalizeVector(refN-1,NDET,&normFact,"GdpBinRefSelection (refN) ");
        ANALYSE_NormalizeVector(refS-1,NDET,&normFact,"GdpBinRefSelection (refS) ");
       }
     }
   }

  // Close file

  if (fp!=NULL)
   {
    fprintf(fp,"\n");
    fclose(fp);
   }

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
// INPUT         pSpecInfo    hold the configuration of the current project
//               specFp       pointer to the current file
//
// RETURN        ERROR_ID_ALLOC if something failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC GdpBinNewRef(SPEC_INFO *pSpecInfo,FILE *specFp)
 {
  // Declarations

  INDEX indexFeno;                                                              // browse analysis windows
  FENO *pTabFeno;                                                               // current analysis window
  RC rc;                                                                        // return code

  // Initializations

  rc=THRD_CopySpecInfo(&THRD_refInfo,pSpecInfo);                                // perform a backup of the pSpecInfo structure

  memset(OUTPUT_refFile,0,MAX_PATH_LEN+1);
  OUTPUT_nRec=0;

  if (THRD_refInfo.recordNumber==0)
   rc=ERROR_ID_ALLOC;
  else

   // Browse analysis windows

   for (indexFeno=0;(indexFeno<NFeno) && (rc<THREAD_EVENT_STOP);indexFeno++)
    {
     pTabFeno=&TabFeno[indexFeno];

     if ((pTabFeno->hidden!=1) &&
         (pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC) &&
         (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC))

      // Build reference spectra according to latitudes and SZA conditions

      rc=GdpBinRefSelection(&THRD_refInfo,
                             specFp,
                             pTabFeno->refLatMin,pTabFeno->refLatMax,
                             pTabFeno->refLonMin,pTabFeno->refLonMax,
                             pTabFeno->refSZA,pTabFeno->refSZADelta,
                             pTabFeno->nspectra,
                             pTabFeno->LembdaK,pTabFeno->Sref,
                             pTabFeno->LembdaN,pTabFeno->SrefN,
                             pTabFeno->LembdaS,pTabFeno->SrefS,
                             pTabFeno->gomePixelType);
    }

  THRD_goto.indexMin=THRD_goto.indexMax=ITEM_NONE;

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
// INPUT         pSpecInfo    data on the current file
//               specFp       pointer to the current file
//
// RETURN        0 for success
// -----------------------------------------------------------------------------

RC GDP_BIN_LoadAnalysis(SPEC_INFO *pSpecInfo,FILE *specFp)
 {
  // Declarations

  GOME_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX indexFeno,indexTabCross,indexWindow,i,j;                                // indexes for loops and array
  CROSS_REFERENCE *pTabCross;                                                   // pointer to the current cross section
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a symbol
  FENO *pTabFeno;                                                               // pointer to the current spectral analysis window
  double factTemp,lembdaMin,lembdaMax;                                          // working variables
  INT DimL,useUsamp,useKurucz,saveFlag;                                         // working variables
  RC rc;                                                                        // return code

  // Initializations

  saveFlag=(INT)pSpecInfo->project.spectra.displayDataFlag;
  pOrbitFile=&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];

  if (!(rc=pOrbitFile->rc) && (THRD_id==THREAD_TYPE_ANALYSIS) && (gdpBinLoadReferenceFlag || !ANALYSE_refSelectionFlag))
   {
    lembdaMin=(double)9999.;
    lembdaMax=(double)-9999.;

    rc=ERROR_ID_NO;
    useKurucz=useUsamp=0;

    // Browse analysis windows and load missing data

    for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
     if (!TabFeno[indexFeno].hidden)
      {
       pTabFeno=&TabFeno[indexFeno];
       pTabFeno->NDET=NDET;

       // Load calibration and reference spectra

       if (!pTabFeno->gomeRefFlag)
        {
         memcpy(pTabFeno->LembdaRef,pSpecInfo->lembda,sizeof(double)*NDET);

         for (i=0,j=pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex];
              j<pOrbitFile->gdpBinStartPixel[pOrbitFile->gdpBinBandIndex]+pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].bandSize;i++,j++)
          {
           pTabFeno->Sref[i]=(double)pOrbitFile->gdpBinReference[j]/pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].scalingFactor;
           if (pSpecInfo->useErrors && pTabFeno->SrefSigma!=NULL)
            pTabFeno->SrefSigma[i]=(double)pOrbitFile->gdpBinRefError[j]/pOrbitFile->gdpBinBandInfo[pOrbitFile->gdpBinBandIndex].scalingError;
          }

         if (!(rc=ANALYSE_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&factTemp,"GDP_BIN_LoadAnalysis (Reference) ")) &&
            (!pSpecInfo->useErrors || !(rc=ANALYSE_NormalizeVector(pTabFeno->SrefSigma-1,pTabFeno->NDET,&factTemp,"GDP_BIN_LoadAnalysis (RefError) "))))
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
                  ((rc=ANALYSE_CheckLembda(pWrkSymbol,pTabFeno->LembdaRef,"GDP_BIN_LoadAnalysis "))!=ERROR_ID_NO))

              goto EndGOME_LoadAnalysis;
            }

           // Gaps : rebuild subwindows on new wavelength scale

           for (indexWindow=0,DimL=0;indexWindow<pTabFeno->svd.Z;indexWindow++)
            {
             pTabFeno->svd.Fenetre[indexWindow][0]=FNPixel(pTabFeno->LembdaRef,pTabFeno->svd.LFenetre[indexWindow][0],NDET,PIXEL_AFTER);
             pTabFeno->svd.Fenetre[indexWindow][1]=FNPixel(pTabFeno->LembdaRef,pTabFeno->svd.LFenetre[indexWindow][1],NDET,PIXEL_BEFORE);

             DimL+=(pTabFeno->svd.Fenetre[indexWindow][1]-pTabFeno->svd.Fenetre[indexWindow][0]+1);
            }

           pTabFeno->svd.DimL=DimL;

           // Buffers allocation

           ANALYSE_SvdFree("GDP_BIN_LoadAnalysis",&pTabFeno->svd);
           ANALYSE_SvdLocalAlloc("GDP_BIN_LoadAnalysis",&pTabFeno->svd);

           if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LembdaRef))!=ERROR_ID_NO) ||
               (!pKuruczOptions->fwhmFit && pTabFeno->xsToConvolute &&
               ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LembdaRef,&ANALYSIS_slit,pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2,&pSlitOptions->slitFunction.slitParam3,&pSlitOptions->slitFunction.slitParam4))!=ERROR_ID_NO)))
            goto EndGOME_LoadAnalysis;

           pTabFeno->Decomp=1;
          }
        }

       memcpy(pTabFeno->LembdaK,pTabFeno->LembdaRef,sizeof(double)*NDET);
       memcpy(pTabFeno->Lembda,pTabFeno->LembdaRef,sizeof(double)*NDET);

       useUsamp+=pTabFeno->useUsamp;
       useKurucz+=pTabFeno->useKurucz;

       if (pTabFeno->useUsamp)
        {
         if (pTabFeno->LembdaRef[0]<lembdaMin)
          lembdaMin=pTabFeno->LembdaRef[0];
         if (pTabFeno->LembdaRef[NDET-1]>lembdaMax)
          lembdaMax=pTabFeno->LembdaRef[NDET-1];
        }
      }

    // Kurucz

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
     {
      KURUCZ_Init(0);

      if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0))!=ERROR_ID_NO))
       goto EndGOME_LoadAnalysis;
     }

    // Build undersampling cross sections

    if (useUsamp && (THRD_id!=THREAD_TYPE_KURUCZ))
     {
      USAMP_LocalFree();

      if (((rc=USAMP_LocalAlloc(0 /* lembdaMin,lembdaMax,oldNDET */))!=ERROR_ID_NO) ||
          ((rc=USAMP_BuildFromAnalysis(0,0))!=ERROR_ID_NO) ||                     // ((analysisFlag==0) && (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE) && (pUsamp->method==PRJCT_USAMP_FIXED))
          ((rc=USAMP_BuildFromAnalysis(1,ITEM_NONE))!=ERROR_ID_NO))               // ((analysisFlag==1) && (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) && (pUsamp->method==PRJCT_USAMP_FIXED))

       goto EndGOME_LoadAnalysis;
     }

    // Reference

    if ((THRD_id==THREAD_TYPE_ANALYSIS) && gdpBinLoadReferenceFlag && !(rc=GdpBinNewRef(pSpecInfo,specFp)) &&
       !(rc=ANALYSE_AlignReference(2,pSpecInfo->project.spectra.displayDataFlag)))  // automatic ref selection for Northern hemisphere
     rc=ANALYSE_AlignReference(3,pSpecInfo->project.spectra.displayDataFlag);       // automatic ref selection for Southern hemisphere

    if (rc==ERROR_ID_NO_REF)
     for (i=GDP_BIN_currentFileIndex+1;i<gdpBinOrbitFilesN;i++)
      GDP_BIN_orbitFiles[i].rc=rc;
   }

  // Return

  EndGOME_LoadAnalysis :

  return rc;
 }

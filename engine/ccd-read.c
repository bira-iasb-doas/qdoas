
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  CCD read out routines
//  Name of module    :  CCD-READ.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in the WinDOAS package in 97
//
//  Author            :  Ann-Carine VANDAELE (a-c.vandaele@oma.be)
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
//  This module contains the routines needed to read the data from the CCD detectors
//  operated by the BIRA-IASB institute andthe Universite Libre of Brussels.
//  The following formats are supported :
//
//  - PRJCT_INSTR_FORMAT_CCD_HA_94 for a 1024x512 detector used at Harestua in 94;
//  - PRJCT_INSTR_FORMAT_CCD_OHP_96 for a 1024x512 detector used at OHP in summer 96;
//  - PRJCT_INSTR_FORMAT_CCD_EEV for a 1340x400 CCD detector built in 99 for the
//                               total solar eclipse (11 Augustus) and still in use;
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetCCD_EEV - set file pointers and get the number of records (1340x400/1340x100);
//  ReliCCD_EEV - 1340x400/1340x100 format read out;
//
//  SetCCD - set file pointers and get the number of records (1024x512,94-96);
//  ReliCCD - 1024x512 format read out (1024x512 detectors, Harestua 94);
//  ReliCCDTrack - 1024x512 format read out (1024x512 detectors, OHP 96);
//
//  CCD_LoadInstrumental - load instrumental functions needed to correct measurements;
//  CCD_ResetInstrumental - release the buffers allocated by CCD_LoadInstrumental;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"

#define NCURVE      20
#define MAX_CAMERA_PICTURES 2000                                                // a picture should be captured every minute.  So 60x24 = 1440 pictures max per day

// ===================================
// 1340x400 DETECTORS (1999 until now)
// ===================================

// -------------------------------------
// Description of the header of a record
// -------------------------------------

typedef struct _ccdData
 {
  double      exposureTime;                                                     // exposure time
  INT         totalTime;                                                        // the total measurement time
  INT         lockedTemperature;                                                // 1 if temperature is locked, 0 otherwise
  double      targetTemperature;                                                // target temperature
  double      currentTemperature;                                               // current temperature

  int         roiWveStart,                                                      // region of interest
              roiWveEnd,
              roiWveGroup,
              roiSlitStart,
              roiSlitEnd,
              roiSlitGroup;


  SHORT_DATE  today;                                                            // measurement date and time
  struct time now;
  double      Tm;                                                               // measurement date and time in seconds
  double      Zm;                                                               // solar zenith angle
  double      Azimuth;                                                          // solar azimuth angle
  int         nrej,nacc;                                                        // resp. number of rejected spectra and number of accumulation
  int         saveTracks;                                                       // 1 if all tracks are saved, 0 if only the mean is saved
  int         vipCorrection;                                                    // 1 if a interpixel variability correction has been applied; 0 otherwise
  int         nlCorrection;                                                     // 1 if a nonlinearity correction has been applied; 0 otherwise
  short       nTint;                                                            // number of integration times
  double      brusagElevation;                                                  // elevation angle for off axis measurements
  INT         filterUnused;
  INT         filterNumber;
  double      headTemperature;
  short       alsFlag;                                                          // 1 for asl measurents (cfr Alexis)
  short       scanIndex;                                                        // scan index for asl measurements
  double      scanningAngle;
  DoasCh      doubleFlag;                                                       // for Alexis
  DoasCh      brusagVersion;
  float       diodes[4];
  float       targetElevation,targetAzimuth;
  DoasCh      saturatedFlag,ignoredFlag;
  DoasCh      ignored[926];                                                     // if completed with new data in the future, authorizes compatibility with previous versions
  float       compassAngle;
  float       pitchAngle;
  float       rollAngle;
  struct time startTime;
  struct time endTime;
  double      mirrorAzimuth;
  int         measureType;                                                      // if completed with new data in the future, authorizes compatibility with previous versions
 }
CCD_DATA;

typedef struct cameraPicture
 {
 	int timestamp;
 	DoasCh fileName[MAX_STR_LEN];
 }
CAMERA_PICTURE;

DoasCh *CCD_measureTypes[PRJCT_INSTR_EEV_TYPE_MAX]=
     	                            { "None","Off axis","Direct sun","Zenith","Dark","Lamp","Bentham","Almucantar","Offset","Azimuth", "Principal plane" };


// ------------------------------------------------------------------
// Different predefined integration times for dark current correction
// ------------------------------------------------------------------

#define MAXTPS 41

static double predTint[MAXTPS] =
 { 0.10,   0.20,   0.40,   0.50,   0.80,   1.00,   1.20,   1.50,   1.75,   2.00,   2.50,   3.00,   3.60,   4.30,   5.20,
   6.20,   7.50,   8.90,  11.00,  13.00,  16.00,  19.00,  22.00,  27.00,  32.00,
  38.00,  46.00,  55.00,  66.00,  80.00,  95.00, 115.00, 140.00, 160.00, 200.00,
  240.00, 280.00, 340.00, 410.00, 490.00, 590.00 };

static DoasU32 ccdDarkCurrentOffset[MAXTPS];                                      // offset of measured dark currents for each integration time

CAMERA_PICTURE ccdImageFilesList[MAX_CAMERA_PICTURES];
DoasCh ccdCurrentImagePath[MAX_ITEM_TEXT_LEN+1];
int ccdImageFilesN=0;

void CCD_GetImageFilesList(SHORT_DATE *pFileDate,DoasCh *rootPath)
 {
 	// Declarations

 	DoasCh imageFileName[MAX_ITEM_TEXT_LEN+1];
 	DoasCh dateStr[9],*ptr;
  struct dirent *fileInfo;
  int hour,minute,sec,newtimestamp;
  INDEX i;
  DIR *hDir;

  // Initializations

  sprintf(dateStr,"%d%02d%02d",pFileDate->da_year,pFileDate->da_mon,pFileDate->da_day);
  ccdImageFilesN=0;

  // Build the full path

  sprintf(ccdCurrentImagePath,"%s/%d/%02d/%02d",rootPath,pFileDate->da_year,pFileDate->da_mon,pFileDate->da_day);

  if (strlen(rootPath) && STD_IsDir(ccdCurrentImagePath))

   // Browse the directory

   for (hDir=opendir(ccdCurrentImagePath);(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);)
    {
    	sprintf(imageFileName,"%s/%s",ccdCurrentImagePath,fileInfo->d_name);

     if (!STD_IsDir(imageFileName) && ((ptr=strstr(fileInfo->d_name,dateStr))!=NULL))
      {
      	sscanf(ptr+8,"%02d%02d%02d",&hour,&minute,&sec);
      	newtimestamp=hour*3600+minute*60+sec;

      	if (!ccdImageFilesN || (newtimestamp>ccdImageFilesList[ccdImageFilesN-1].timestamp))
      	 {
         for (i=ccdImageFilesN;(i>0) && (newtimestamp<ccdImageFilesList[i-1].timestamp);i--)
          memcpy(&ccdImageFilesList[i],&ccdImageFilesList[i-1],sizeof(CAMERA_PICTURE));

      	  ccdImageFilesList[i].timestamp=newtimestamp;
         strcpy(ccdImageFilesList[i].fileName,imageFileName);
        }

       ccdImageFilesN++;
      }
    }
 }

INDEX CCD_SearchForImage(int timestamp)
 {
  // Declarations

  INDEX i,imin,imax;

  // Initializations

  i=imin=0;
  imax=ccdImageFilesN-1;

  // Dichotomic search

  while (imin+1<imax)
   {
   	i=(imax+imin)>>1;

   	if (timestamp<=ccdImageFilesList[imin].timestamp)
   	 i=imax=imin;
   	else if (timestamp>=ccdImageFilesList[imax].timestamp)
   	 i=imin=imax;
   	else if (timestamp<ccdImageFilesList[i].timestamp)
   	 imax=i;
   	else if (timestamp>=ccdImageFilesList[i].timestamp)
   	 imin=i;
   }

  if ((imax>=imin) && (i>=imin) && (i<=imax))
   {
    i=((timestamp-ccdImageFilesList[imin].timestamp)<=(ccdImageFilesList[imax].timestamp-timestamp))?imin:imax;
    if (abs(timestamp-ccdImageFilesList[i].timestamp)>60)
     i=ITEM_NONE;
   }
  else
   i=ITEM_NONE;

  // Return

  return i;
 }

// ------------------
// Read out functions
// ------------------

// -----------------------------------------------------------------------------
// FUNCTION      SetCCD_EEV
// -----------------------------------------------------------------------------
// PURPOSE       set file pointers and get the number of records (1340x400/1340x100)
//
// INPUT         pEngineContext : information on the file to read
//               specFp    : pointer to the current spectra file
//
// OUTPUT        pEngineContext->recordNumber, the number of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//               ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC SetCCD_EEV(ENGINE_CONTEXT *pEngineContext,FILE *specFp,FILE *darkFp)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  ANALYSIS_REF *pRef;

  CCD_DATA header;                                                              // header of a record
  DoasU32   *recordIndexes;                                                       // indexes of records for direct access
  INT      ccdX,ccdY,dataSize;                                                  // size of the detector
  int      specMaxFlag;                                                         // 1 to use specMax, the vector of maxima of the scans
  int      maxScans;
  INDEX    indexTps;                                                            // browse the predefined integration time
  DoasU32    offset;                                                              // offset to remove from the spectrum
  RC       rc;                                                                  // return code

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("SetCCD_EEV",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  pBuffers=&pEngineContext->buffers;
  pRef=&pEngineContext->analysisRef;

  recordIndexes=pEngineContext->buffers.recordIndexes;

  memset(ccdDarkCurrentOffset,-1L,sizeof(DoasU32)*MAXTPS);
  pEngineContext->recordIndexesSize=2001;
  pEngineContext->recordNumber=0;
  specMaxFlag=maxScans=0;

  rc=ERROR_ID_NO;

  // Release specMax buffer

  if (pBuffers->specMax!=NULL)
   MEMORY_ReleaseDVector("SetCCD_EEV","specMax",pBuffers->specMax,0);
  pBuffers->specMax=NULL;

  // Release scanref buffer

  if (pRef->scanRefIndexes!=NULL)
   MEMORY_ReleaseBuffer("SetCCD_EEV","scanRefIndexes",pRef->scanRefIndexes);
  pRef->scanRefIndexes=NULL;

  // Check the input file pointer

  if (specFp==NULL)
   rc=ERROR_SetLast("SetCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else
   {
    // Read the header of the first record

    fseek(specFp,0L,SEEK_SET);
    memset(recordIndexes,0L,sizeof(DoasU32)*pEngineContext->recordIndexesSize);

    while (!feof(specFp) && fread(&header,sizeof(CCD_DATA),1,specFp))
     {
     	if (!pEngineContext->recordNumber)
     	 CCD_GetImageFilesList(&header.today,pEngineContext->project.instrumental.imagePath);

      ccdX=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
      ccdY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;
      dataSize=(header.doubleFlag==(DoasCh)1)?sizeof(double):sizeof(DoasUS);

      pEngineContext->recordNumber++;

      recordIndexes[pEngineContext->recordNumber]=(DoasU32)
        recordIndexes[pEngineContext->recordNumber-1]+
        sizeof(CCD_DATA)+((header.saveTracks)?ccdX*ccdY*dataSize:ccdX*dataSize);

      // 15/02/2003 : dark current acquisition

      if ((header.nTint>0) &&
         ((header.today.da_year>2003) ||
         ((header.today.da_year==2003) && (header.today.da_mon>2)) ||
         ((header.today.da_year==2003) && (header.today.da_mon==2) && (header.today.da_day>14))))
       {
        recordIndexes[pEngineContext->recordNumber]+=
         (sizeof(double)+sizeof(short))*header.nTint+sizeof(double)*(header.nrej+header.nacc);

        if (header.nrej+header.nacc>maxScans)
         maxScans=header.nrej+header.nacc;

        specMaxFlag++;
       }

      fseek(specFp,recordIndexes[pEngineContext->recordNumber],SEEK_SET);
     }

    // Dark currents

    if ((darkFp!=NULL) && (pEngineContext->buffers.darkCurrent!=NULL))
     {
      fseek(darkFp,0L,SEEK_SET);
      offset=0L;

      while (!feof(darkFp) && fread(&header,sizeof(CCD_DATA),1,darkFp))
       {
        ccdX=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
        ccdY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;
        dataSize=(header.doubleFlag==(DoasCh)1)?sizeof(double):sizeof(DoasUS);

        for (indexTps=0;indexTps<MAXTPS;indexTps++)
         if (header.exposureTime==predTint[indexTps])
          break;

        if (indexTps<MAXTPS)
         ccdDarkCurrentOffset[indexTps]=offset;

        offset+=sizeof(CCD_DATA)+((header.saveTracks)?ccdX*ccdY*dataSize:ccdX*dataSize);
        fseek(darkFp,offset,SEEK_SET);
       }
     }

    // Allocate a buffer to display the variation of the signal along the acquisitions

    if ((specMaxFlag && ((pBuffers->specMax=MEMORY_AllocDVector("SetCCD_EEV","specMax",0,maxScans-1))==NULL)) ||
        (pEngineContext->analysisRef.refScan && pEngineContext->recordNumber &&
       ((pRef->scanRefIndexes=(INT *)MEMORY_AllocBuffer("EngineSetFile","scanRefIndexes",pEngineContext->recordNumber,sizeof(INT),0,MEMORY_TYPE_INT))==NULL)))
     rc=ERROR_ID_ALLOC;

    // Eclipse 99

/*  if (!(fileLength=filelength(fileno(specFp))) || !fread(&header,sizeof(CCD_DATA),1,specFp))
     THRD_Error(ERROR_TYPE_WARNING,(rc=ERROR_ID_FILE_EMPTY),"SetCCD_EVV1340x400 ",pEngineContext->fileInfo.fileName);
    else
     {
      // Retrieve the really used size of the detector

      NDET=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
      ccdSizeY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;
      pEngineContext->recordSize=sizeof(CCD_DATA)+sizeof(DoasUS)*NDET*ccdSizeY;
      pEngineContext->recordNumber=fileLength/pEngineContext->recordSize;
     }  */
   }

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("SetCCD_EEV",rc);
  #endif

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliCCD_EEV
// -----------------------------------------------------------------------------
// PURPOSE       1340x400/1340x100 format read out
//
// INPUT         pEngineContext : information on the current file
//               recordNo  : the index of the record to read
//               dateFlag  : 1 to search for a reference spectrum
//               localDay  : if dateFlag is 1, the calendar day for the
//                           reference spectrum to search for
//               specFp    : pointer to the spectra file
//               darkFp    : pointer to the dark currents file if any
//
// OUTPUT        information on the read out record
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_NOT_FOUND : if the input file pointer is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_EMPTY     : if the file is not large enough;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

RC ReliCCD_EEV(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *darkFp)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  CCD_DATA header;                                                              // header of the current record
  double *dspectrum,*tmpSpectrum;                                               // pointer to the vector in 'pEngineContext' to fill with the current spectrum
  DoasUS *spectrum;                                                             // spectrum to retrieve from the current record in the original format
  INT ccdX,ccdY,spSize,dataSize;                                                // dimensions of the CCD detector
  DoasUS *ccdTabNTint;                                                          // number of scans per different integration times
  double *ccdTabTint;                                                           // the different integration times used for the current measurement
  DoasUS *darkCurrent;                                                          // the dark current
  int nTint;                                                                    // the number of different integration times
//  double offset;                                                                // offset correction
  double tmLocal;
  INDEX i,j,k;                                                                  // indexes to browse vectors
  int measurementType;
  RC rc,rcFread;                                                                // return code

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ReliCCD_EEV",DEBUG_FCTTYPE_FILE);
  #endif
  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;

  dspectrum=pBuffers->spectrum;
  tmpSpectrum=NULL;
  ccdTabNTint=NULL;
  ccdTabTint=NULL;
  spectrum=NULL;
  darkCurrent=NULL;
  rc=ERROR_ID_NO;

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;
  else
   {
    for(i=0;i<NDET;i++)
     dspectrum[i]=(double)0.;

    // Set file pointers

    fseek(specFp,(DoasI32)pBuffers->recordIndexes[recordNo-1],SEEK_SET);

    // Complete record read out

    if (!fread(&header,sizeof(CCD_DATA),1,specFp))
     rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
    else
     {
      // get the size of the CCD

      ccdX=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
      ccdY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;
      dataSize=(header.doubleFlag==(DoasCh)1)?sizeof(double):sizeof(DoasUS);

      spSize=(header.saveTracks)?ccdX*ccdY:ccdX;
      nTint=header.nTint;

      // Buffers allocation

      if (((spectrum=(DoasUS *)MEMORY_AllocBuffer("ReliCCD_EEV","spectrum",spSize,sizeof(DoasUS),0,MEMORY_TYPE_USHORT))==NULL) ||
          ((tmpSpectrum=(double *)MEMORY_AllocBuffer("ReliCCD_EEV","tmpSpectrum",spSize,sizeof(double),0,MEMORY_TYPE_DOUBLE))==NULL) ||
          ((darkCurrent=(DoasUS *)MEMORY_AllocBuffer("ReliCCD_EEV","darkCurrent",spSize,sizeof(DoasUS),0,MEMORY_TYPE_USHORT))==NULL) ||
          ((ccdTabNTint=(DoasUS *)MEMORY_AllocBuffer("ReliCCD_EEV","ccdTabNTint",300,sizeof(DoasUS),0,MEMORY_TYPE_USHORT))==NULL) ||
          ((ccdTabTint=(double *)MEMORY_AllocBuffer("ReliCCD_EEV","ccdTabTint",300,sizeof(double),0,MEMORY_TYPE_DOUBLE))==NULL))

       rc=ERROR_ID_ALLOC;

       else
        {
         memset(spectrum,0,sizeof(DoasUS)*spSize);
         memset(darkCurrent,0,sizeof(DoasUS)*spSize);

         for(i=0;i<NDET;i++)
          tmpSpectrum[i]=(double)0.;

         rcFread=(header.doubleFlag==(DoasCh)1)?
                  fread(tmpSpectrum,sizeof(double)*spSize,1,specFp):
                  fread(spectrum,sizeof(DoasUS)*spSize,1,specFp);

         if (!rcFread)
          rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
         else if ((header.nTint<=0) ||
                  !fread(ccdTabTint,sizeof(double)*header.nTint,1,specFp) ||
                  !fread(ccdTabNTint,sizeof(short)*header.nTint,1,specFp) ||
                  !fread(pBuffers->specMax,sizeof(double)*(header.nacc+header.nrej),1,specFp))
          rc=ERROR_ID_FILE_RECORD;
         else if (!header.saveTracks)
          {
          	if (header.doubleFlag==(DoasCh)1)
          	 memcpy(dspectrum,tmpSpectrum,sizeof(double)*spSize);
          	else
            for (i=0;i<spSize;i++)
             dspectrum[i]=(double)spectrum[i];
          }
         else
          {
          	if (header.doubleFlag==(DoasCh)1)
          	 {
             // Accumulate spectra

             for (i=0;i<NDET;i++)
              {
               dspectrum[i]=(double)0.;
               for (j=0;j<ccdY;j++)
                dspectrum[i]+=tmpSpectrum[NDET*j+i];
               dspectrum[i]/=ccdY;
              }
          	 }
          	else
          	 {
             // Accumulate spectra

             for (i=0;i<NDET;i++)
              {
               dspectrum[i]=(double)0.;
               for (j=0;j<ccdY;j++)
                dspectrum[i]+=spectrum[NDET*j+i];
               dspectrum[i]/=ccdY;
              }
            }
          }

         if (!rc)
          {
           // Data on the current spectrum

           pRecord->Tint      = (double)header.exposureTime;
           pRecord->NSomme    = header.nacc;
           pRecord->Zm        = header.Zm;
           pRecord->Tm        = header.Tm;
           pRecord->NTracks   = (header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;
           pRecord->rejected  = header.nrej;
           pRecord->SkyObs    = 8;
           pRecord->TDet      = (double)header.currentTemperature;
           pRecord->ReguTemp  = (double)0.;
           pRecord->TotalExpTime = (double)0.;

           pRecord->als.alsFlag=0;
           pRecord->als.scanIndex=-1;
           pRecord->als.scanningAngle=
           pRecord->als.compassAngle=
           pRecord->als.pitchAngle=
           pRecord->als.rollAngle=(float)0.;

           pRecord->ccd.diodes[0]=pRecord->ccd.diodes[1]=pRecord->ccd.diodes[2]=pRecord->ccd.diodes[3]=(float)0.;
           pRecord->ccd.targetElevation=header.brusagElevation;
           pRecord->ccd.targetAzimuth=header.Azimuth;
           pRecord->ccd.saturatedFlag=0;
           pRecord->ccd.indexImage=CCD_SearchForImage((int)header.now.ti_hour*3600+header.now.ti_min*60+header.now.ti_sec);

           if (header.alsFlag)
            {
            	pRecord->als.alsFlag=header.alsFlag;
             pRecord->als.scanIndex=header.scanIndex;
             pRecord->als.scanningAngle=header.scanningAngle;                                          // total number of spectra in tracks
             pRecord->als.compassAngle=header.compassAngle;
             pRecord->als.pitchAngle=header.pitchAngle;
             pRecord->als.rollAngle=header.rollAngle;

             strcpy(pRecord->als.atrString,header.ignored);

             pRecord->ccd.filterNumber=pRecord->ccd.measureType=0;
             pRecord->ccd.headTemperature=(double)0.;
            }
           else
            {
              pRecord->ccd.filterNumber=header.filterNumber;
              pRecord->ccd.measureType=header.measureType;
              pRecord->ccd.headTemperature=header.headTemperature;

              if (header.brusagVersion==2)
               {
                memcpy(pRecord->ccd.diodes,header.diodes,sizeof(float)*4);
                pRecord->ccd.targetElevation=header.targetElevation;
                pRecord->ccd.targetAzimuth=header.targetAzimuth;
                pRecord->ccd.saturatedFlag=header.saturatedFlag;
               }
            }

           memcpy(&pRecord->present_day,&header.today,sizeof(SHORT_DATE));
           memcpy(&pRecord->present_time,&header.now,sizeof(struct time));

           if (header.Tm<EPSILON)
            pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);


           memcpy(&pRecord->startTime,&header.startTime,sizeof(struct time));
           memcpy(&pRecord->endTime,&header.endTime,sizeof(struct time));

           pRecord->elevationViewAngle=
                ((pRecord->present_day.da_year>2003) ||
                ((pRecord->present_day.da_year==2003) && (pRecord->present_day.da_mon>2)) ||
                ((pRecord->present_day.da_year==2003) && (pRecord->present_day.da_mon==2) && (pRecord->present_day.da_day>20)))?
                 (float)header.brusagElevation:(float)-1.;

           pRecord->Azimuth=header.Azimuth;

           if (!header.alsFlag)
            pRecord->azimuthViewAngle=
                 ((pRecord->present_day.da_year>2008) ||
                 ((pRecord->present_day.da_year==2008) && (pRecord->present_day.da_mon>4)))?
                  (float)header.mirrorAzimuth:(float)-1.;

           pRecord->TimeDec=(double)header.now.ti_hour+header.now.ti_min/60.+header.now.ti_sec/3600.;
           pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);
           memset(pRecord->Nom,0,20);

           // Build dark current

           if (pRecord->NSomme==0)
            rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"number of accumulations");
           else if ((pBuffers->darkCurrent!=NULL) && (darkFp!=NULL))
            {
             for (i=0;i<NDET;i++)
              pBuffers->darkCurrent[i]=(double)0.;

             for (k=0;k<nTint;k++)
              {
               pRecord->TotalExpTime+=(double)ccdTabNTint[k]*ccdTabTint[k];

               for (j=0;j<MAXTPS;j++)
                if (ccdTabTint[k]<=predTint[j])
                 break;

               if (j<MAXTPS)
                {
                 if ((j>0) && (predTint[j]!=ccdTabTint[k]) && (predTint[j]-ccdTabTint[k]>ccdTabTint[k]-predTint[j-1]))
                  j--;

                 fseek(darkFp,ccdDarkCurrentOffset[j],SEEK_SET);

                 if (!fread(&header,sizeof(CCD_DATA),1,darkFp))
                  rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
                 else if (!header.roiWveGroup || !header.roiSlitGroup)
                  rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"header is corrupted");
                 else
                  {
                   ccdX=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
                   ccdY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;

                   spSize=(header.saveTracks)?ccdX*(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup:ccdX;

                  	fread(darkCurrent,sizeof(DoasUS)*spSize,1,darkFp);

                   for (i=0;i<NDET;i++)
                    {
                     if (!header.saveTracks)
                      pBuffers->darkCurrent[i]+=darkCurrent[i]*ccdTabNTint[k];
                     else
                      {
                       for (j=0;j<ccdY;j++)
                        pBuffers->darkCurrent[i]+=(double)darkCurrent[NDET*j+i]*ccdTabNTint[k];
                       pBuffers->darkCurrent[i]/=ccdY;
                      }
                    }
                  }
                }
              }

             for (i=0;i<NDET;i++)
              {
               pBuffers->darkCurrent[i]=pBuffers->darkCurrent[i]/pRecord->NSomme;
               pBuffers->spectrum[i]-=pBuffers->darkCurrent[i];
              }
            }

           tmLocal=pRecord->Tm+THRD_localShift*3600.;
           pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
           measurementType=pEngineContext->project.instrumental.user;

           if (rc ||
              (dateFlag && (pRecord->elevationViewAngle>0.) && (pRecord->elevationViewAngle<80.)) ||                                                                              // reference spectra are zenith only
              (!dateFlag && pEngineContext->analysisRef.refScan && !pEngineContext->analysisRef.refSza && (pRecord->elevationViewAngle>80.)))    // zenith sky spectra are not analyzed in scan reference selection mode

            rc=ERROR_ID_FILE_RECORD;

           else if (measurementType!=PRJCT_INSTR_EEV_TYPE_NONE)
            {
            	if (((measurementType==PRJCT_INSTR_EEV_TYPE_OFFAXIS) && (pRecord->ccd.measureType!=PRJCT_INSTR_EEV_TYPE_OFFAXIS) && (pRecord->ccd.measureType!=PRJCT_INSTR_EEV_TYPE_ZENITH)) ||
            	    ((measurementType!=PRJCT_INSTR_EEV_TYPE_OFFAXIS) && (pRecord->ccd.measureType!=measurementType)))

            	 rc=ERROR_ID_FILE_RECORD;
            }
          }
       }
     }
   }

  // Straylight correction

  if (!rc && (pRecord->ccd.vip.matrix!=NULL) && (THRD_browseType==THREAD_BROWSE_SPECTRA))
   {
   	memcpy(tmpSpectrum,dspectrum,sizeof(double)*NDET);

   	for (i=0;i<NDET;i++)
   	 {
   	 	dspectrum[i]=(double)0.;

   	 	for (j=0;j<NDET;j++)
  	 	  dspectrum[i]+=pRecord->ccd.vip.matrix[j%NCURVE][(j/NCURVE)*NDET+i]*tmpSpectrum[j];
   	 }
   }

  // Release the allocated buffers

  if (spectrum!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD_EEV","spectrum",spectrum);
  if (tmpSpectrum!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD_EEV","tmpSpectrum",tmpSpectrum);
  if (darkCurrent!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD_EEV","darkCurrent",darkCurrent);
  if (ccdTabNTint!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD_EEV","ccdTabNTint",ccdTabNTint);
  if (ccdTabTint!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD_EEV","ccdTabTint",ccdTabTint);

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ReliCCD_EEV",rc);
  #endif

  // Return

  return rc;
 }

// ==========================
// 1024x512 DETECTORS (94-96)
// ==========================

// -----------------------------
// Description of records header
// -----------------------------

typedef struct _ccd_1024
 {
  short        Scans;                                                           //  equivalent to the value for I in the custom scan programs
  short        Memories;                                                        //  equivalent to the value for J in the custom scan programs
  short        Ignored_Scans;                                                   //  equivalent to the value for K in the custom scan programs
  short        Prep_Frames;                                                     //  equivalent to the value for H in the custom scan programs
  short        Detector_Temp;                                                   //  detector temperature in degrees Celsius
  short        Cooler_Locked;                                                   //  cooler locked flag
  short        Shutter_Open_Sync_Index;                                         //  TRUE if wait for sync pulse to open shutter
  short        Shutter_Close_Sync_Index;                                        //  TRUE if wait for sync pulse to close shutter
  short        Shutter_Forced_Mode;                                             //  0 = normal.  1 = closed.  2 = open.
  float        Exposure_Time;                                                   //  current exposure time
  short        Tracks;                                                          //  number of tracks per memory
  short        Number_of_Points;                                                //  number of points per tracks
  short        X0;                                                              //  start column of X axis
  short        Y0;                                                              //  start row of Y axis
  short        ActiveX;                                                         //  array size, number of channels in the X axis
  short        ActiveY;                                                         //  array size, number of channels in the Y axis
  float        ReguTemp;                                                        //  bin size for uniform columns
  short        Slicemode;                                                       //  0 = uniform   1 = non uniform.
  short        Trackmode;                                                       //  0 = contiguous   1 = Random.
  short        Shiftmode;                                                       //  0 = CCD   1 = diode array   2 = streak camera
  short        rejected;                                                        //  number of rejected spectra ( too low or too high signal )
  SHORT_DATE   Day;                                                             //  date of the measurement
  struct time  Hour;                                                            //  hour of the measurement
  double       Zm;                                                              //  zenith angle
  char         SkyObs;                                                          //  sky observation
 }
CCD_1024;

// -----------------------------------------------------------------------------
// FUNCTION      SetCCD
// -----------------------------------------------------------------------------
// PURPOSE       set file pointers and get the number of records (1024x512 detectors,94-96)
//
// INPUT         pEngineContext : information on the file to read
//               specFp    : pointer to the current spectra file
//               flag      : 0 for OHP 96, 1 for Harestua 94
//
// OUTPUT        pEngineContext->recordNumber, the number of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//               ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC SetCCD (ENGINE_CONTEXT *pEngineContext,FILE *specFp,INT flag)
 {
  // Declarations

  DoasU32  recordSize,                                                            // the size of one record in bytes (without SpecMax)
        *recordIndexes;                                                         // the position of spectra in bytes from the beginning of the file
  short *indexes,                                                               // the number of accumulations per spectrum (give by the same way,
                                                                                // the size of the SpecMax vectors)
         curvenum;                                                              // the number of spectra in the file
  INDEX  i;                                                                     // index for browsing spectra
  RC     rc;                                                                    // return code

  // Initializations

  pEngineContext->recordIndexesSize=2001;
  pEngineContext->recordNumber=0;
  recordSize=0L;
  rc=ERROR_ID_NO;

  // Buffers allocation

  recordIndexes=pEngineContext->buffers.recordIndexes;

  if (((indexes=(short *)MEMORY_AllocBuffer("SetCCD","indexes",pEngineContext->recordIndexesSize,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL) ||
      ((pEngineContext->buffers.specMax=MEMORY_AllocDVector("SetCCD","specMax",0,NDET-1))==NULL))

   rc=ERROR_ID_ALLOC;

  // Check the input file pointer

  else if (specFp==NULL)
   rc=ERROR_SetLast("SetCCD",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else
   {
    // Headers read out

    fseek(specFp,0L,SEEK_SET);

    if (fread(&curvenum,sizeof(short),1,specFp) &&                              // number of spectra in the file
        fread(indexes,pEngineContext->recordIndexesSize*sizeof(short),1,specFp) &&   // number of accumulations (size of SpecMax vectors) per measurement
        (curvenum>0))
     {
      pEngineContext->recordNumber=curvenum;

      // Calculate the size of a record (without SpecMax)

      if (flag==0)
       recordSize=(DoasI32)sizeof(CCD_1024)+44L*(sizeof(float)+sizeof(DoasUS)*NDET) + (DoasI32)(sizeof(float)+sizeof(short))*300;
      else if (flag==1)
       recordSize=(DoasI32)sizeof(CCD_1024)+(DoasI32)sizeof(short)*NDET;

      // Calculate the position in bytes from the beginning of the file for each spectra

      recordIndexes[0]=(DoasI32)(pEngineContext->recordIndexesSize+1)*sizeof(short);    // file header : size of indexes table + curvenum

      for (i=1;i<curvenum;i++)
       recordIndexes[i]=recordIndexes[i-1]+                                     // position of the previous record
                        recordSize+                                             // size of the record (without SpecMax)
                        indexes[i]*sizeof(short);                               // take SpecMax of the previous record into account
     }
   }

  // Release the allocated local buffers

  if (indexes!=NULL)
   MEMORY_ReleaseBuffer("SetCCD","indexes",indexes);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliCCD
// -----------------------------------------------------------------------------
// PURPOSE       1024x512 format read out (Harestua 94)
//
// INPUT         pEngineContext : information on the file to read
//               recordNo  : the index of the record to read
//               dateFlag  : 1 to search for a reference spectrum
//               localDay  : if dateFlag is 1, the calendar day for the
//                           reference spectrum to search for
//               specFp    : pointer to the spectra file
//               namesFp   : pointer to the names file if any
//               darkFp    : pointer to the dark currents file if any
//
// OUTPUT        information on the read out record
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_NOT_FOUND : if the input file pointer is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy input criteria
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

RC ReliCCD(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  CCD_1024 DetInfo;                                                             // data on the current record retrieved from the spectra file
  DoasCh names[20];                                                              // name of the current spectrum
  DoasUS *ISpectre,*ISpecMax;                                                   // resp. spectrum and SpecMax retrieved from the file
  double Max,tmLocal;                                                           // the maximum value measured for the current spectrum
  INDEX i;                                                                      // index for browsing pixels in the spectrum
  RC rc;                                                                        // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;
  ISpectre=ISpecMax=NULL;
  rc=ERROR_ID_NO;

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliCCD",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Buffers allocation

  else if (((ISpectre=(DoasUS *)MEMORY_AllocBuffer("ReliCCD","ISpectre",NDET,sizeof(DoasUS),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((ISpecMax=(DoasUS *)MEMORY_AllocBuffer("ReliCCD","ISpecMax",2000,sizeof(DoasUS),0,MEMORY_TYPE_USHORT))==NULL))

     rc=ERROR_ID_ALLOC;
  else
   {
    // Set file pointers

    if (namesFp!=NULL)
     fseek(namesFp,(DoasI32)20L*(recordNo-1),SEEK_SET);

    fseek(specFp,(DoasI32)pBuffers->recordIndexes[recordNo-1],SEEK_SET);

    // Complete record read out

    if (((namesFp!=NULL) && !fread(names,20,1,namesFp)) ||
         !fread(&DetInfo,sizeof(CCD_1024),1,specFp) ||
         !fread(ISpectre,sizeof(DoasUS)*NDET,1,specFp) ||
         !fread(ISpecMax,sizeof(DoasUS)*(DetInfo.Scans+DetInfo.rejected),1,specFp))

     rc=ERROR_ID_FILE_END;

    else if (DetInfo.Scans<=0)
     rc=ERROR_ID_FILE_RECORD;
    else
     {
      // Data on the current spectrum

      pRecord->Tint      = (double)DetInfo.Exposure_Time;
      pRecord->NSomme    = DetInfo.Scans;
      pRecord->Zm        = DetInfo.Zm;
      pRecord->NTracks   = DetInfo.Tracks;
      pRecord->rejected  = DetInfo.rejected;
      pRecord->SkyObs    = DetInfo.SkyObs;
      pRecord->TDet      = (double)-40.;
      pRecord->ReguTemp  = (float)DetInfo.ReguTemp;

      pRecord->TotalExpTime=(double)0.;
      pRecord->TimeDec=(double)DetInfo.Hour.ti_hour+DetInfo.Hour.ti_min/60.;

      pRecord->present_day.da_year  = DetInfo.Day.da_year;
      pRecord->present_day.da_mon   = DetInfo.Day.da_mon;
      pRecord->present_day.da_day   = DetInfo.Day.da_day;

      pRecord->present_time.ti_hour = DetInfo.Hour.ti_hour;
      pRecord->present_time.ti_min  = DetInfo.Hour.ti_min;
      pRecord->present_time.ti_sec  = DetInfo.Hour.ti_sec;

      pRecord->Tm = (double) ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);

      tmLocal=pRecord->Tm+THRD_localShift*3600.;

      pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
      pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

      memcpy(pRecord->Nom,names,20);

      // SpecMax (maxima per accumulation)

      for (i=0,Max=(double)ISpecMax[0];i<(DetInfo.Scans+DetInfo.rejected);i++)
       if ((pBuffers->specMax[i]=(double)ISpecMax[i])>Max)
        Max=(double)pBuffers->specMax[i];

      // Rebuild spectrum

      for (i=0;i<NDET;i++)
       pBuffers->spectrum[NDET-i-1]=(double)ISpectre[i]*Max/65000.*176.-908.25*44.;

      if (dateFlag && (pRecord->localCalDay!=localDay))
       rc=ERROR_ID_FILE_RECORD;
     }
   }

  // Release the allocated buffers

  if (ISpectre!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD","ISpectre",ISpectre);
  if (ISpecMax!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD","ISpecMax",ISpecMax);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliCCDTrack
// -----------------------------------------------------------------------------
// PURPOSE       1024x512 format read out (OHP 96)
//
// INPUT         pEngineContext : information on the file to read
//               recordNo  : the index of the record to read
//               dateFlag  : 1 to search for a reference spectrum
//               localDay  : if dateFlag is 1, the calendar day for the
//                           reference spectrum to search for
//               specFp    : pointer to the spectra file
//               namesFp   : pointer to the names file if any
//               darkFp    : pointer to the dark currents file if any
//
// OUTPUT        information on the read out record
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_NOT_FOUND : if the input file pointer is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

RC ReliCCDTrack(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  CCD_1024 DetInfo;                                                             // data on the current spectra retrieved from the spectra file
  DoasCh names[20],                                                              // name of the current spectrum
        fileName[MAX_PATH_LEN+1];                                               // the complete file name (including path)
  DoasUS *ISpectre,*ISpecMax;                                                   // resp. spectrum and SpecMax retrieved from the file
  float *TabTint,*MaxTrk;                                                       // resp. integration time and track data
  INT *TabNSomme;                                                               // scans data
  double *varPix,tmLocal;                                                       // instrumental function for interpixel variability correction
  INDEX i,j;                                                                    // indexes for browsing vectors
  FILE *varFp;                                                                  // pointer to the file for interpixel variability correction
  RC rc;                                                                        // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;

  ISpectre=ISpecMax=NULL;
  TabTint=MaxTrk=NULL;
  varPix=NULL;
  TabNSomme=NULL;
  varFp=NULL;
  rc=ERROR_ID_NO;

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Buffers allocation

  else if (((ISpectre=(DoasUS *)MEMORY_AllocBuffer("ReliCCDTrack","ISpectre",NDET,sizeof(DoasUS),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((ISpecMax=(DoasUS *)MEMORY_AllocBuffer("ReliCCDTrack","ISpecMax",2000,sizeof(DoasUS),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((MaxTrk=(float *)MEMORY_AllocBuffer("ReliCCDTrack","MaxTrk",44,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
           ((varPix=(double *)MEMORY_AllocBuffer("ReliCCDTrack","varPix",NDET,sizeof(double),0,MEMORY_TYPE_DOUBLE))==NULL) ||
           ((TabTint=(float *)MEMORY_AllocBuffer("ReliCCDTrack","TabTint",300,sizeof(float),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((TabNSomme=(int *)MEMORY_AllocBuffer("ReliCCDTrack","TabNSomme",300,sizeof(int),0,MEMORY_TYPE_INT))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Set file pointers

    if (namesFp!=NULL)
     fseek(namesFp,(DoasI32)20L*(recordNo-1),SEEK_SET);

    fseek(specFp,(DoasI32)pBuffers->recordIndexes[recordNo-1],SEEK_SET);

    // Complete record read out

    if (((namesFp!=NULL) && !fread(names,20,1,namesFp)) ||                      // name of the current record
         !fread(&DetInfo,sizeof(CCD_1024),1,specFp) ||                          // data on the current measurement
         !fread(MaxTrk,sizeof(float)*DetInfo.Tracks,1,specFp))                  // maximum value per track

     rc=ERROR_ID_FILE_END;

    else
     {
      // Read out the instrumental function for the interpixel variability correction

      if (strlen(pEngineContext->project.instrumental.vipFile)>0)
       {
        FILES_RebuildFileName(fileName,pEngineContext->project.instrumental.vipFile,1);
        varFp=fopen(fileName,"rb");
       }

      for (j=0;j<NDET;j++)
       {
        pBuffers->spectrum[j]=(double)0.;
        varPix[j]=(double)1.;
       }

      // Tracks accumulation and interpixel variability correction

      for (i=0;(i<DetInfo.Tracks) && !rc;i++)
       {
        if (!fread(ISpectre,sizeof(DoasUS)*NDET,1,specFp))
         rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
        else
         {
          if (varFp!=NULL)
           fread(varPix,sizeof(double)*NDET,1,varFp);

          for (j=0;j<NDET;j++)
           pBuffers->spectrum[j]+=(double)ISpectre[j]*MaxTrk[i]/varPix[j];
         }
       }

      if (!rc)
       {
        if (!fread(TabTint,sizeof(float)*300,1,specFp) || !fread(TabNSomme,sizeof(short)*300,1,specFp) ||
            !fread(ISpecMax,sizeof(DoasUS)*(DetInfo.Scans+DetInfo.rejected),1,specFp))

         rc=ERROR_ID_FILE_END;

        else if (DetInfo.Scans<=0)
         rc=ERROR_ID_FILE_RECORD;
        else
         {
          // Data on the current spectrum

          pRecord->TotalExpTime=(double)0.;

          for (i=0;TabNSomme[i]>0;i++)
           pRecord->TotalExpTime+=(double)TabNSomme[i]*TabTint[i];

          for (i=0;i<(DetInfo.Scans+DetInfo.rejected);i++)
           pBuffers->specMax[i]=(double)ISpecMax[i]*4.;

          pRecord->Tint      = (double)DetInfo.Exposure_Time;
          pRecord->NSomme    = DetInfo.Scans;
          pRecord->Zm        = DetInfo.Zm;
          pRecord->NTracks   = DetInfo.Tracks;
          pRecord->rejected  = DetInfo.rejected;
          pRecord->SkyObs    = DetInfo.SkyObs;
          pRecord->TDet      = (double)-40.;
          pRecord->ReguTemp  = (float)DetInfo.ReguTemp;

          pRecord->TotalExpTime=(double)0.;
          pRecord->TimeDec=(double)DetInfo.Hour.ti_hour+DetInfo.Hour.ti_min/60.;

          pRecord->present_day.da_year  = DetInfo.Day.da_year;
          pRecord->present_day.da_mon   = DetInfo.Day.da_mon;
          pRecord->present_day.da_day   = DetInfo.Day.da_day;

          pRecord->present_time.ti_hour = DetInfo.Hour.ti_hour;
          pRecord->present_time.ti_min  = DetInfo.Hour.ti_min;
          pRecord->present_time.ti_sec  = DetInfo.Hour.ti_sec;

          pRecord->Tm = (double) ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);

          tmLocal=pRecord->Tm+THRD_localShift*3600.;

          pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
          pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

          memcpy(pRecord->Nom,names,20);

          VECTOR_Invert(pBuffers->spectrum,NDET);

          // spectra mean calculation

          if (pRecord->NSomme)
           for (i=0;i<NDET;i++)
            pBuffers->spectrum[i]/=(double)(44.*pRecord->NSomme);

          if (dateFlag && (pRecord->localCalDay!=localDay))
           rc=ERROR_ID_FILE_RECORD;
         }
       }
     }
   }

  // Release the allocated buffers

  if (ISpectre!=NULL)
   MEMORY_ReleaseBuffer("ReliCCDTrack","ISpectre",ISpectre);
  if (ISpecMax!=NULL)
   MEMORY_ReleaseBuffer("ReliCCDTrack","ISpecMax",ISpecMax);
  if (TabTint!=NULL)
   MEMORY_ReleaseBuffer("ReliCCDTrack","TabTint",TabTint);
  if (MaxTrk!=NULL)
   MEMORY_ReleaseBuffer("ReliCCDTrack","MaxTrk",MaxTrk);
  if (varPix!=NULL)
   MEMORY_ReleaseBuffer("ReliCCDTrack","varPix",varPix);
  if (TabNSomme!=NULL)
   MEMORY_ReleaseBuffer("ReliCCDTrack","TabNSomme",TabNSomme);

  // Close file

  if (varFp!=NULL)
   fclose(varFp);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      CCD_LoadInstrumental
// -----------------------------------------------------------------------------
// PURPOSE       Load instrumental functions needed to correct measurements
//
// INPUT         pEngineContext   information on the current file
//
// RETURN        the code returned by MATRIX_Load if the load of a file failed
//               ERROR_ID_NO, otherwise.
// -----------------------------------------------------------------------------

RC CCD_LoadInstrumental(ENGINE_CONTEXT *pEngineContext)
 {
 	// Declarations

  PROJECT *pProject;                                                            // pointer to the project part of the engine context
  PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  DoasCh fileName[MAX_STR_LEN+1];                                                // the complete name (including path) of the file to load
  RC rc;                                                                        // the return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;
  pProject=&pEngineContext->project;
  pInstrumental=&pProject->instrumental;

  rc=ERROR_ID_NO;

  // Offset

  if ((strlen(pInstrumental->instrFunction)>0) && (pRecord->ccd.drk.matrix==NULL) &&
     ((rc=MATRIX_Load(FILES_RebuildFileName(fileName,pInstrumental->instrFunction,1),
                     &pRecord->ccd.drk,
                      0 /* line base */,0 /* column base */,NDET,NCURVE,
                      0.,0.,
                      0,0,"CCD_LoadInstrumental (offset)"))!=ERROR_ID_NO))

   goto End_CCD_LoadInstrumental;

  // Interpixel variability

  if ((strlen(pInstrumental->vipFile)>0) && (pRecord->ccd.vip.matrix==NULL) &&
     ((rc=MATRIX_Load(FILES_RebuildFileName(fileName,pInstrumental->vipFile,1),
                     &pRecord->ccd.vip,
                      0 /* line base */,0 /* column base */,0,NCURVE,
                      0.,0.,
                      0,0,"CCD_LoadInstrumental (vip)"))!=ERROR_ID_NO))

   goto End_CCD_LoadInstrumental;

  // Non linearity of the detector

  if ((strlen(pInstrumental->dnlFile)>0) && (pRecord->ccd.dnl.matrix==NULL) &&
     ((rc=MATRIX_Load(FILES_RebuildFileName(fileName,pInstrumental->dnlFile,1),
                     &pRecord->ccd.dnl,
                      0 /* line base */,0 /* column base */,0,0,
                      0.,0.,
                      1,0,"CCD_LoadInstrumental (dnl)"))!=ERROR_ID_NO))

   goto End_CCD_LoadInstrumental;

  // Return

  End_CCD_LoadInstrumental :

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      CCD_ResetInstrumental
// -----------------------------------------------------------------------------
// PURPOSE       Release the buffers allocated by CCD_LoadInstrumental
//
// INPUT         pUlb : pointer to the instrumental correction matrices
// -----------------------------------------------------------------------------

void CCD_ResetInstrumental(CCD *pCCD)
 {
  if (pCCD->vip.matrix!=NULL)
   MATRIX_Free(&pCCD->vip,"CCD_ResetInstrumental (vip)");
  if (pCCD->dnl.matrix!=NULL)
   MATRIX_Free(&pCCD->dnl,"CCD_ResetInstrumental (dnl)");
  if (pCCD->drk.matrix!=NULL)
   MATRIX_Free(&pCCD->drk,"CCD_ResetInstrumental (offset)");
 }

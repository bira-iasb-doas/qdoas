
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  THE BIRA-IASB DOAS SOFTWARE FOR WINDOWS AND LINUX
//  Module purpose    :  CCD read out routines
//  Name of module    :  CCD-READ.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in the WinDOAS package in 97
//
//  Authors           :  Caroline FAYT (caroline.fayt@oma.be)
//                       Ann-Carine VANDAELE (a-c.vandaele@oma.be)
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
//  - PRJCT_INSTR_FORMAT_CCD_ULB for ULB
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
//  SetCCD_Ulb - set file pointers and get the number of records (ULB format);
//  ReliCCD_Ulb - read the spectra measured by the CCD detector operated by ULB;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"

#define NCURVE      20

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
  UCHAR       filterName[16];                                                   // the name of the filter used if any
  UCHAR       ignored[998];                                                     // if completed with new data in the future, authorizes compatibility with previous versions
 }
CCD_DATA;

// ------------------------------------------------------------------
// Different predefined integration times for dark current correction
// ------------------------------------------------------------------

#define MAXTPS 41

static double predTint[MAXTPS] =
 { 0.10,   0.20,   0.40,   0.50,   0.80,   1.00,   1.20,   1.50,   1.75,   2.00,   2.50,   3.00,   3.60,   4.30,   5.20,
   6.20,   7.50,   8.90,  11.00,  13.00,  16.00,  19.00,  22.00,  27.00,  32.00,
  38.00,  46.00,  55.00,  66.00,  80.00,  95.00, 115.00, 140.00, 160.00, 200.00,
  240.00, 280.00, 340.00, 410.00, 490.00, 590.00 };

static ULONG ccdDarkCurrentOffset[MAXTPS];                                      // offset of measured dark currents for each integration time

// ------------------
// Read out functions
// ------------------

// -----------------------------------------------------------------------------
// FUNCTION      SetCCD_EEV
// -----------------------------------------------------------------------------
// PURPOSE       set file pointers and get the number of records (1340x400/1340x100)
//
// INPUT         pSpecInfo : information on the file to read
//               specFp    : pointer to the current spectra file
//
// OUTPUT        pSpecInfo->recordNumber, the number of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//               ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC SetCCD_EEV(SPEC_INFO *pSpecInfo,FILE *specFp,FILE *darkFp)
 {
  // Declarations

  CCD_DATA header;                                                              // header of a record
  INT      ccdX,ccdY;                                                           // size of the detector
  int      specMaxFlag;                                                         // 1 to use specMax, the vector of maxima of the scans
  INDEX    indexTps;                                                            // browse the predefined integration time
  ULONG    offset;                                                              // offset to remove from the spectrum
  RC       rc;                                                                  // return code

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("SetCCD_EEV",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  memset(ccdDarkCurrentOffset,-1L,sizeof(ULONG)*MAXTPS);
  pSpecInfo->recordIndexesSize=2001;
  pSpecInfo->recordNumber=0;
  specMaxFlag=0;
//  NDET=1340;
  rc=ERROR_ID_NO;

  // Release specMax buffer

  if (pSpecInfo->specMax!=NULL)
   MEMORY_ReleaseDVector("SetCCD_EEV","specMax",pSpecInfo->specMax,0);
  pSpecInfo->specMax=NULL;

  // Check the input file pointer

  if (specFp==NULL)
   rc=ERROR_SetLast("SetCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else
   {
    // Read the header of the first record

    fseek(specFp,0L,SEEK_SET);
    memset(pSpecInfo->recordIndexes,0L,sizeof(ULONG)*pSpecInfo->recordIndexesSize);

    while (!feof(specFp) && fread(&header,sizeof(CCD_DATA),1,specFp))
     {
      ccdX=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
      ccdY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;

      pSpecInfo->recordNumber++;

      pSpecInfo->recordIndexes[pSpecInfo->recordNumber]=(ULONG)
        pSpecInfo->recordIndexes[pSpecInfo->recordNumber-1]+
        sizeof(CCD_DATA)+((header.saveTracks)?ccdX*ccdY*sizeof(USHORT):ccdX*sizeof(USHORT));

      // 15/02/2003 : dark current acquisition

      if ((header.nTint>0) &&
         ((header.today.da_year>2003) ||
         ((header.today.da_year==2003) && (header.today.da_mon>2)) ||
         ((header.today.da_year==2003) && (header.today.da_mon==2) && (header.today.da_day>14))))
       {
        pSpecInfo->recordIndexes[pSpecInfo->recordNumber]+=
         (sizeof(double)+sizeof(SHORT))*header.nTint+sizeof(double)*(header.nrej+header.nacc);

        specMaxFlag++;
       }

      fseek(specFp,pSpecInfo->recordIndexes[pSpecInfo->recordNumber],SEEK_SET);
     }

    // Dark currents

    if ((darkFp!=NULL) && (pSpecInfo->darkCurrent!=NULL))
     {
      fseek(darkFp,0L,SEEK_SET);
      offset=0L;

      while (!feof(darkFp) && fread(&header,sizeof(CCD_DATA),1,darkFp))
       {
        ccdX=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
        ccdY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;

        for (indexTps=0;indexTps<MAXTPS;indexTps++)
         if (header.exposureTime==predTint[indexTps])
          break;

        if (indexTps<MAXTPS)
         ccdDarkCurrentOffset[indexTps]=offset;

        offset+=sizeof(CCD_DATA)+((header.saveTracks)?ccdX*ccdY*sizeof(USHORT):ccdX*sizeof(USHORT));
        fseek(darkFp,offset,SEEK_SET);
       }
     }

    // Allocate a buffer to display the variation of the signal along the acquisitions

    if (specMaxFlag && ((pSpecInfo->specMax=MEMORY_AllocDVector("SetCCD_EEV","specMax",0,NDET-1))==NULL))
     rc=ERROR_ID_ALLOC;

    // Eclipse 99

/*  if (!(fileLength=filelength(fileno(specFp))) || !fread(&header,sizeof(CCD_DATA),1,specFp))
     THRD_Error(ERROR_TYPE_WARNING,(rc=ERROR_ID_FILE_EMPTY),"SetCCD_EVV1340x400 ",pSpecInfo->fileName);
    else
     {
      // Retrieve the really used size of the detector

      NDET=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
      ccdSizeY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;
      pSpecInfo->recordSize=sizeof(CCD_DATA)+sizeof(USHORT)*NDET*ccdSizeY;
      pSpecInfo->recordNumber=fileLength/pSpecInfo->recordSize;
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
// INPUT         pSpecInfo : information on the current file
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

RC ReliCCD_EEV(SPEC_INFO *pSpecInfo,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *darkFp)
 {
  // Declarations

  CCD_DATA header;                                                              // header of the current record
  double *dspectrum,*tmpSpectrum;                                               // pointer to the vector in 'pSpecInfo' to fill with the current spectrum
  USHORT *spectrum;                                                             // spectrum to retrieve from the current record in the original format
  INT ccdX,ccdY,spSize;                                                         // dimensions of the CCD detector
  USHORT *ccdTabNTint;                                                          // number of scans per different integration times
  double *ccdTabTint;                                                           // the different integration times used for the current measurement
  USHORT *darkCurrent;                                                          // the dark current
  int nTint;                                                                    // the number of different integration times
  double offset;                                                                // offset correction
  INDEX i,j,k;                                                                  // indexes to browse vectors
  RC rc;                                                                        // return code

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ReliCCD_EEV",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  dspectrum=pSpecInfo->spectrum;
  tmpSpectrum=NULL;
  ccdTabNTint=NULL;
  ccdTabTint=NULL;
  spectrum=NULL;
  darkCurrent=NULL;
  rc=ERROR_ID_NO;

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else if ((recordNo<=0) || (recordNo>pSpecInfo->recordNumber))
   rc=ERROR_ID_FILE_END;
  else
   {
    // Set file pointers

    fseek(specFp,(LONG)pSpecInfo->recordIndexes[recordNo-1],SEEK_SET);

    // Complete record read out

    if (!fread(&header,sizeof(CCD_DATA),1,specFp))
     rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pSpecInfo->fileName);
    else
     {
      // get the size of the CCD

      ccdX=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
      ccdY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;

      spSize=(header.saveTracks)?ccdX*ccdY:ccdX;
      nTint=header.nTint;

      // Buffers allocation

      if (((spectrum=(USHORT *)MEMORY_AllocBuffer("ReliCCD_EEV","spectrum",spSize,sizeof(USHORT),0,MEMORY_TYPE_USHORT))==NULL) ||
          ((tmpSpectrum=(double *)MEMORY_AllocBuffer("ReliCCD_EEV","tmpSpectrum",NDET,sizeof(double),0,MEMORY_TYPE_DOUBLE))==NULL) ||
          ((darkCurrent=(USHORT *)MEMORY_AllocBuffer("ReliCCD_EEV","darkCurrent",NDET*400,sizeof(USHORT),0,MEMORY_TYPE_USHORT))==NULL) ||
          ((ccdTabNTint=(USHORT *)MEMORY_AllocBuffer("ReliCCD_EEV","ccdTabNTint",300,sizeof(USHORT),0,MEMORY_TYPE_USHORT))==NULL) ||
          ((ccdTabTint=(double *)MEMORY_AllocBuffer("ReliCCD_EEV","ccdTabTint",300,sizeof(double),0,MEMORY_TYPE_DOUBLE))==NULL))

       rc=ERROR_ID_ALLOC;

      else
       {
        memset(spectrum,0,sizeof(USHORT)*spSize);
        memset(darkCurrent,0,sizeof(USHORT)*spSize);

        if (!fread(spectrum,sizeof(USHORT)*spSize,1,specFp) ||
           ((header.nTint>0) &&
           (!fread(ccdTabTint,sizeof(double)*header.nTint,1,specFp) ||
            !fread(ccdTabNTint,sizeof(SHORT)*header.nTint,1,specFp) ||
            !fread(pSpecInfo->specMax,sizeof(double)*(header.nacc+header.nrej),1,specFp))))

         rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pSpecInfo->fileName);
        else if (!header.saveTracks)
         for (i=0;i<spSize;i++)
          dspectrum[i]=(double)spectrum[i];
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

        if (!rc)
         {
          // Data on the current spectrum

          pSpecInfo->Tint      = (double)header.exposureTime;
          pSpecInfo->NSomme    = header.nacc;
          pSpecInfo->Zm        = header.Zm;
          pSpecInfo->Tm        = header.Tm;
          pSpecInfo->NTracks   = (header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;
          pSpecInfo->rejected  = header.nrej;
          pSpecInfo->SkyObs    = 8;
          pSpecInfo->TDet      = (double)header.currentTemperature;
          pSpecInfo->ReguTemp  = (double)0.;

          memcpy(&pSpecInfo->present_day,&header.today,sizeof(SHORT_DATE));
          memcpy(&pSpecInfo->present_time,&header.now,sizeof(struct time));

          pSpecInfo->elevationViewAngle=
               ((pSpecInfo->present_day.da_year>2003) ||
               ((pSpecInfo->present_day.da_year==2003) && (pSpecInfo->present_day.da_mon>2)) ||
               ((pSpecInfo->present_day.da_year==2003) && (pSpecInfo->present_day.da_mon==2) && (pSpecInfo->present_day.da_day>20)))?
                (float)header.brusagElevation:(float)-1.;

          pSpecInfo->TimeDec=(double)header.now.ti_hour+header.now.ti_min/60.+header.now.ti_sec/3600.;
          if (strlen(header.filterName)>0)
           {
           	memset(pSpecInfo->Nom,' ',20);
           	pSpecInfo->Nom[20]='\0';
            strcpy(pSpecInfo->Nom,header.filterName);
            pSpecInfo->Nom[strlen(header.filterName)]=' ';
           }
          else
           memset(pSpecInfo->Nom,0,20);

          // Build dark current

          if (pSpecInfo->NSomme==0)
           rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"number of accumulations");
          else if ((pSpecInfo->darkCurrent!=NULL) && (darkFp!=NULL))
           {
            for (i=0;i<NDET;i++)
             pSpecInfo->darkCurrent[i]=(double)0.;

            for (k=0;k<nTint;k++)
             {
              pSpecInfo->TotalExpTime+=(double)ccdTabNTint[k]*ccdTabTint[k];

              for (j=0;j<MAXTPS;j++)
               if (ccdTabTint[k]<=predTint[j])
                break;

              if (j<MAXTPS)
               {
                if ((j>0) && (predTint[j]!=ccdTabTint[k]) && (predTint[j]-ccdTabTint[k]>ccdTabTint[k]-predTint[j-1]))
                 j--;

                fseek(darkFp,ccdDarkCurrentOffset[j],SEEK_SET);

                if (!fread(&header,sizeof(CCD_DATA),1,darkFp))
                 rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pSpecInfo->fileName);
                else if (!header.roiWveGroup || !header.roiSlitGroup)
                 rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"header is corrupted");
                else
                 {
                  ccdX=(header.roiWveEnd-header.roiWveStart+1)/header.roiWveGroup;
                  ccdY=(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup;

                  spSize=(header.saveTracks)?ccdX*(header.roiSlitEnd-header.roiSlitStart+1)/header.roiSlitGroup:ccdX;

                 	fread(darkCurrent,sizeof(USHORT)*spSize,1,darkFp);

                  for (i=0;i<NDET;i++)
                   {
                    if (!header.saveTracks)
                     pSpecInfo->darkCurrent[i]+=darkCurrent[i]*ccdTabNTint[k];
                    else
                     {
                      for (j=0;j<ccdY;j++)
                       pSpecInfo->darkCurrent[i]+=(double)darkCurrent[NDET*j+i]*ccdTabNTint[k];
                      pSpecInfo->darkCurrent[i]/=ccdY;
                     }
                   }
                 }
               }
             }

            for (i=0;i<NDET;i++)
             pSpecInfo->darkCurrent[i]=pSpecInfo->darkCurrent[i]/pSpecInfo->NSomme;
           }

          if (rc || (dateFlag && (pSpecInfo->elevationViewAngle>(double)-0.5) && (pSpecInfo->elevationViewAngle<88.)))                  // reference spectra are zenith only
           rc=ERROR_ID_FILE_RECORD;
         }
       }
     }
   }

  // Straylight correction

  if (!rc && (pSpecInfo->ccd.vip.matrix!=NULL) && (THRD_browseType==THREAD_BROWSE_SPECTRA))
   {
   	memcpy(tmpSpectrum,dspectrum,sizeof(double)*NDET);

   	// Dark current correction

   	for (i=0;i<NDET;i++)
   	 tmpSpectrum[i]-=pSpecInfo->darkCurrent[i];

   	for (i=0;i<NDET;i++)
   	 {
   	 	dspectrum[i]=(double)0.;

   	 	for (j=0;j<NDET;j++)
  	 	  dspectrum[i]+=pSpecInfo->ccd.vip.matrix[j%NCURVE][(j/NCURVE)*NDET+i]*tmpSpectrum[j];

  	 	 dspectrum[i]+=pSpecInfo->darkCurrent[i];    // the dark current is still subtracted in winthrd.c
   	 }
   }

  // Get an approximation of the offset

/*  if (!rc)
   {
    if ((pSpecInfo->elevationViewAngle<80.) && (pSpecInfo->lembda[0]>=290) && (pSpecInfo->lembda[0]<=300))
     {
     	int npoints;

     	npoints=50;

      for (offset=(double)0.,i=1;i<=npoints;i++)
       offset+=(dspectrum[i]-((pSpecInfo->darkCurrent!=NULL)?pSpecInfo->darkCurrent[i]:(double)0.));
      offset=(double)offset/npoints;

      for (i=0;i<NDET;i++)
       dspectrum[i]-=offset;
     }
   }  */

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
  SHORT        Scans;                                                           //  equivalent to the value for I in the custom scan programs
  SHORT        Memories;                                                        //  equivalent to the value for J in the custom scan programs
  SHORT        Ignored_Scans;                                                   //  equivalent to the value for K in the custom scan programs
  SHORT        Prep_Frames;                                                     //  equivalent to the value for H in the custom scan programs
  SHORT        Detector_Temp;                                                   //  detector temperature in degrees Celsius
  SHORT        Cooler_Locked;                                                   //  cooler locked flag
  SHORT        Shutter_Open_Sync_Index;                                         //  TRUE if wait for sync pulse to open shutter
  SHORT        Shutter_Close_Sync_Index;                                        //  TRUE if wait for sync pulse to close shutter
  SHORT        Shutter_Forced_Mode;                                             //  0 = normal.  1 = closed.  2 = open.
  float        Exposure_Time;                                                   //  current exposure time
  SHORT        Tracks;                                                          //  number of tracks per memory
  SHORT        Number_of_Points;                                                //  number of points per tracks
  SHORT        X0;                                                              //  start column of X axis
  SHORT        Y0;                                                              //  start row of Y axis
  SHORT        ActiveX;                                                         //  array size, number of channels in the X axis
  SHORT        ActiveY;                                                         //  array size, number of channels in the Y axis
  float        ReguTemp;                                                        //  bin size for uniform columns
  SHORT        Slicemode;                                                       //  0 = uniform   1 = non uniform.
  SHORT        Trackmode;                                                       //  0 = contiguous   1 = Random.
  SHORT        Shiftmode;                                                       //  0 = CCD   1 = diode array   2 = streak camera
  SHORT        rejected;                                                        //  number of rejected spectra ( too low or too high signal )
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
// INPUT         pSpecInfo : information on the file to read
//               specFp    : pointer to the current spectra file
//               flag      : 0 for OHP 96, 1 for Harestua 94
//
// OUTPUT        pSpecInfo->recordNumber, the number of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//               ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC SetCCD (SPEC_INFO *pSpecInfo,FILE *specFp,INT flag)
 {
  // Declarations

  ULONG  recordSize,                                                            // the size of one record in bytes (without SpecMax)
        *recordIndexes;                                                         // the position of spectra in bytes from the beginning of the file
  SHORT *indexes,                                                               // the number of accumulations per spectrum (give by the same way,
                                                                                // the size of the SpecMax vectors)
         curvenum;                                                              // the number of spectra in the file
  INDEX  i;                                                                     // index for browsing spectra
  RC     rc;                                                                    // return code

  // Initializations

  pSpecInfo->recordIndexesSize=2001;
  pSpecInfo->recordNumber=0;
  recordSize=0L;
  rc=ERROR_ID_NO;

  // Buffers allocation

  recordIndexes=pSpecInfo->recordIndexes;

  if (((indexes=(SHORT *)MEMORY_AllocBuffer("SetCCD","indexes",pSpecInfo->recordIndexesSize,sizeof(SHORT),0,MEMORY_TYPE_SHORT))==NULL) ||
      ((pSpecInfo->specMax=MEMORY_AllocDVector("SetCCD","specMax",0,NDET-1))==NULL))

   rc=ERROR_ID_ALLOC;

  // Check the input file pointer

  else if (specFp==NULL)
   rc=ERROR_SetLast("SetCCD",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else
   {
    // Headers read out

    fseek(specFp,0L,SEEK_SET);

    if (fread(&curvenum,sizeof(SHORT),1,specFp) &&                              // number of spectra in the file
        fread(indexes,pSpecInfo->recordIndexesSize*sizeof(SHORT),1,specFp) &&   // number of accumulations (size of SpecMax vectors) per measurement
        (curvenum>0))
     {
      pSpecInfo->recordNumber=curvenum;

      // Calculate the size of a record (without SpecMax)

      if (flag==0)
       recordSize=(LONG)sizeof(CCD_1024)+44L*(sizeof(float)+sizeof(USHORT)*NDET) + (LONG)(sizeof(float)+sizeof(SHORT))*300;
      else if (flag==1)
       recordSize=(LONG)sizeof(CCD_1024)+(LONG)sizeof(SHORT)*NDET;

      // Calculate the position in bytes from the beginning of the file for each spectra

      recordIndexes[0]=(LONG)(pSpecInfo->recordIndexesSize+1)*sizeof(SHORT);    // file header : size of indexes table + curvenum

      for (i=1;i<curvenum;i++)
       recordIndexes[i]=recordIndexes[i-1]+                                     // position of the previous record
                        recordSize+                                             // size of the record (without SpecMax)
                        indexes[i]*sizeof(SHORT);                               // take SpecMax of the previous record into account
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
// INPUT         pSpecInfo : information on the file to read
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

RC ReliCCD(SPEC_INFO *pSpecInfo,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp)
 {
  // Declarations

  CCD_1024 DetInfo;                                                             // data on the current record retrieved from the spectra file
  UCHAR names[20];                                                              // name of the current spectrum
  USHORT *ISpectre,*ISpecMax;                                                   // resp. spectrum and SpecMax retrieved from the file
  double Max,tmLocal;                                                           // the maximum value measured for the current spectrum
  INDEX i;                                                                      // index for browsing pixels in the spectrum
  RC rc;                                                                        // return code

  // Initializations

  ISpectre=ISpecMax=NULL;
  rc=ERROR_ID_NO;

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliCCD",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else if ((recordNo<=0) || (recordNo>pSpecInfo->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Buffers allocation

  else if (((ISpectre=(USHORT *)MEMORY_AllocBuffer("ReliCCD","ISpectre",NDET,sizeof(USHORT),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((ISpecMax=(USHORT *)MEMORY_AllocBuffer("ReliCCD","ISpecMax",2000,sizeof(USHORT),0,MEMORY_TYPE_USHORT))==NULL))

     rc=ERROR_ID_ALLOC;
  else
   {
    // Set file pointers

    if (namesFp!=NULL)
     fseek(namesFp,(LONG)20L*(recordNo-1),SEEK_SET);

    fseek(specFp,(LONG)pSpecInfo->recordIndexes[recordNo-1],SEEK_SET);

    // Complete record read out

    if (((namesFp!=NULL) && !fread(names,20,1,namesFp)) ||
         !fread(&DetInfo,sizeof(CCD_1024),1,specFp) ||
         !fread(ISpectre,sizeof(USHORT)*NDET,1,specFp) ||
         !fread(ISpecMax,sizeof(USHORT)*(DetInfo.Scans+DetInfo.rejected),1,specFp))

     rc=ERROR_ID_FILE_END;

    else if (DetInfo.Scans<=0)
     rc=ERROR_ID_FILE_RECORD;
    else
     {
      // Data on the current spectrum

      pSpecInfo->Tint      = (double)DetInfo.Exposure_Time;
      pSpecInfo->NSomme    = DetInfo.Scans;
      pSpecInfo->Zm        = DetInfo.Zm;
      pSpecInfo->NTracks   = DetInfo.Tracks;
      pSpecInfo->rejected  = DetInfo.rejected;
      pSpecInfo->SkyObs    = DetInfo.SkyObs;
      pSpecInfo->TDet      = (double)-40.;
      pSpecInfo->ReguTemp  = (float)DetInfo.ReguTemp;

      pSpecInfo->TotalExpTime=(double)0.;
      pSpecInfo->TimeDec=(double)DetInfo.Hour.ti_hour+DetInfo.Hour.ti_min/60.;

      pSpecInfo->present_day.da_year  = DetInfo.Day.da_year;
      pSpecInfo->present_day.da_mon   = DetInfo.Day.da_mon;
      pSpecInfo->present_day.da_day   = DetInfo.Day.da_day;

      pSpecInfo->present_time.ti_hour = DetInfo.Hour.ti_hour;
      pSpecInfo->present_time.ti_min  = DetInfo.Hour.ti_min;
      pSpecInfo->present_time.ti_sec  = DetInfo.Hour.ti_sec;

      pSpecInfo->Tm = (double) ZEN_NbSec(&pSpecInfo->present_day,&pSpecInfo->present_time,0);

      tmLocal=pSpecInfo->Tm+THRD_localShift*3600.;

      pSpecInfo->localCalDay=ZEN_FNCaljda(&tmLocal);
      pSpecInfo->localTimeDec=fmod(pSpecInfo->TimeDec+24.+THRD_localShift,(double)24.);

      memcpy(pSpecInfo->Nom,names,20);

      // SpecMax (maxima per accumulation)

      for (i=0,Max=(double)ISpecMax[0];i<(DetInfo.Scans+DetInfo.rejected);i++)
       if ((pSpecInfo->specMax[i]=(double)ISpecMax[i])>Max)
        Max=(double)pSpecInfo->specMax[i];

      // Rebuild spectrum

      for (i=0;i<NDET;i++)
       pSpecInfo->spectrum[NDET-i-1]=(double)ISpectre[i]*Max/65000.*176.-908.25*44.;

      if (dateFlag && (pSpecInfo->localCalDay!=localDay))
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
// INPUT         pSpecInfo : information on the file to read
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

RC ReliCCDTrack(SPEC_INFO *pSpecInfo,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp)
 {
  // Declarations

  CCD_1024 DetInfo;                                                             // data on the current spectra retrieved from the spectra file
  UCHAR names[20],                                                              // name of the current spectrum
        fileName[MAX_PATH_LEN+1];                                               // the complete file name (including path)
  USHORT *ISpectre,*ISpecMax;                                                   // resp. spectrum and SpecMax retrieved from the file
  float *TabTint,*MaxTrk;                                                       // resp. integration time and track data
  INT *TabNSomme;                                                               // scans data
  double *varPix,tmLocal;                                                       // instrumental function for interpixel variability correction
  INDEX i,j;                                                                    // indexes for browsing vectors
  FILE *varFp;                                                                  // pointer to the file for interpixel variability correction
  RC rc;                                                                        // return code

  // Initializations

  ISpectre=ISpecMax=NULL;
  TabTint=MaxTrk=NULL;
  varPix=NULL;
  TabNSomme=NULL;
  varFp=NULL;
  rc=ERROR_ID_NO;

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else if ((recordNo<=0) || (recordNo>pSpecInfo->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Buffers allocation

  else if (((ISpectre=(USHORT *)MEMORY_AllocBuffer("ReliCCDTrack","ISpectre",NDET,sizeof(USHORT),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((ISpecMax=(USHORT *)MEMORY_AllocBuffer("ReliCCDTrack","ISpecMax",2000,sizeof(USHORT),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((MaxTrk=(float *)MEMORY_AllocBuffer("ReliCCDTrack","MaxTrk",44,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
           ((varPix=(double *)MEMORY_AllocBuffer("ReliCCDTrack","varPix",NDET,sizeof(double),0,MEMORY_TYPE_DOUBLE))==NULL) ||
           ((TabTint=(float *)MEMORY_AllocBuffer("ReliCCDTrack","TabTint",300,sizeof(float),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((TabNSomme=(int *)MEMORY_AllocBuffer("ReliCCDTrack","TabNSomme",300,sizeof(int),0,MEMORY_TYPE_INT))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Set file pointers

    if (namesFp!=NULL)
     fseek(namesFp,(LONG)20L*(recordNo-1),SEEK_SET);

    fseek(specFp,(LONG)pSpecInfo->recordIndexes[recordNo-1],SEEK_SET);

    // Complete record read out

    if (((namesFp!=NULL) && !fread(names,20,1,namesFp)) ||                      // name of the current record
         !fread(&DetInfo,sizeof(CCD_1024),1,specFp) ||                          // data on the current measurement
         !fread(MaxTrk,sizeof(float)*DetInfo.Tracks,1,specFp))                  // maximum value per track

     rc=ERROR_ID_FILE_END;

    else
     {
      // Read out the instrumental function for the interpixel variability correction

      if (strlen(pSpecInfo->project.instrumental.vipFile)>0)
       {
        FILES_RebuildFileName(fileName,pSpecInfo->project.instrumental.vipFile,1);
        varFp=fopen(fileName,"rb");
       }

      for (j=0;j<NDET;j++)
       {
        pSpecInfo->spectrum[j]=(double)0.;
        varPix[j]=(double)1.;
       }

      // Tracks accumulation and interpixel variability correction

      for (i=0;(i<DetInfo.Tracks) && !rc;i++)
       {
        if (!fread(ISpectre,sizeof(USHORT)*NDET,1,specFp))
         rc=ERROR_SetLast("ReliCCD_EEV",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
        else
         {
          if (varFp!=NULL)
           fread(varPix,sizeof(double)*NDET,1,varFp);

          for (j=0;j<NDET;j++)
           pSpecInfo->spectrum[j]+=(double)ISpectre[j]*MaxTrk[i]/varPix[j];
         }
       }

      if (!rc)
       {
        if (!fread(TabTint,sizeof(float)*300,1,specFp) || !fread(TabNSomme,sizeof(SHORT)*300,1,specFp) ||
            !fread(ISpecMax,sizeof(USHORT)*(DetInfo.Scans+DetInfo.rejected),1,specFp))

         rc=ERROR_ID_FILE_END;

        else if (DetInfo.Scans<=0)
         rc=ERROR_ID_FILE_RECORD;
        else
         {
          // Data on the current spectrum

          pSpecInfo->TotalExpTime=(double)0.;

          for (i=0;TabNSomme[i]>0;i++)
           pSpecInfo->TotalExpTime+=(double)TabNSomme[i]*TabTint[i];

          for (i=0;i<(DetInfo.Scans+DetInfo.rejected);i++)
           pSpecInfo->specMax[i]=(double)ISpecMax[i]*4.;

          pSpecInfo->Tint      = (double)DetInfo.Exposure_Time;
          pSpecInfo->NSomme    = DetInfo.Scans;
          pSpecInfo->Zm        = DetInfo.Zm;
          pSpecInfo->NTracks   = DetInfo.Tracks;
          pSpecInfo->rejected  = DetInfo.rejected;
          pSpecInfo->SkyObs    = DetInfo.SkyObs;
          pSpecInfo->TDet      = (double)-40.;
          pSpecInfo->ReguTemp  = (float)DetInfo.ReguTemp;

          pSpecInfo->TotalExpTime=(double)0.;
          pSpecInfo->TimeDec=(double)DetInfo.Hour.ti_hour+DetInfo.Hour.ti_min/60.;

          pSpecInfo->present_day.da_year  = DetInfo.Day.da_year;
          pSpecInfo->present_day.da_mon   = DetInfo.Day.da_mon;
          pSpecInfo->present_day.da_day   = DetInfo.Day.da_day;

          pSpecInfo->present_time.ti_hour = DetInfo.Hour.ti_hour;
          pSpecInfo->present_time.ti_min  = DetInfo.Hour.ti_min;
          pSpecInfo->present_time.ti_sec  = DetInfo.Hour.ti_sec;

          pSpecInfo->Tm = (double) ZEN_NbSec(&pSpecInfo->present_day,&pSpecInfo->present_time,0);

          tmLocal=pSpecInfo->Tm+THRD_localShift*3600.;

          pSpecInfo->localCalDay=ZEN_FNCaljda(&tmLocal);
          pSpecInfo->localTimeDec=fmod(pSpecInfo->TimeDec+24.+THRD_localShift,(double)24.);

          memcpy(pSpecInfo->Nom,names,20);

          VECTOR_Invert(pSpecInfo->spectrum,NDET);

          // spectra mean calculation

          if (pSpecInfo->NSomme)
           for (i=0;i<NDET;i++)
            pSpecInfo->spectrum[i]/=(double)(44.*pSpecInfo->NSomme);

          if (dateFlag && (pSpecInfo->localCalDay!=localDay))
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

// =============================
// CCD DETECTORS OPERATED BY ULB
// =============================

// --------------------------------------
// Definition of constants and structures
// --------------------------------------

// see ACVD for further information on the format

#define INVNCURVE 0.05 // 1/NCURVE

// information on the fiber, how it lightens the CCD

typedef struct tagFIBER
 {
  short NPoint;
  short PointType;
  short FirstCol;
  short NTrack;
  short NCol;
  short TrackType;
  short FirstRow;
  short NRow;
  short Used;
 }
FIBER;

// information on the used grating

typedef struct tagGRATINGS
 {
  short Used;
  short NGrating;
  float Nanometers;
 }
GRATINGS;

// Header of records

typedef struct tagCCDDATA
 {
  short    Temperature;
  short    ScansNumber;
  short    Memories;
  short    PrepFrames;
  short    IgnoredScans;
  short    TempCtrl;
  float    ExposureTime;
  LONG     MeasurementTime;
  LONG     DelayTime;
  LONG     TotalExpTime;
  FIBER    Fiber1;
  FIBER    Fiber2;
  short    CoolerLocked;
  short    Shutter;
  GRATINGS Gratings[5];
  char     LastFile[80];
 }
CCDDATA;

// Header of the file

typedef struct tagFILE_HEADER_CCD
 {
  char         Reference[30];            /*  Campaign information             */
  char         Spectrometer[10];         /*  Spectrometer type                */
  char         Detector[10];             /*  Detector used with Spectrometer  */
  short        Temperature;              /*  Detector Temperature             */
  FIBER        Fiber1;                   /*  Optical Fiber 1                  */
  FIBER        Fiber2;                   /*  Optical Fiber 2;                 */
  LONG         MeasurementTime;          /*  Time for one Measurement         */
  LONG         DelayTime;                /*  Delay between two Measurements   */
  LONG         TotalExpTime;             /*  Total Experiment Time            */
  NEWDATE		Day;                          /*  Date of the measurement          */
 }
FILE_HEADER_CCD;

// Header of curves

typedef struct tagCURVE_HEADER_CCD
 {
  NEWTIME	HDeb;                /*  Time at the beginning  of the measurement  */
  NEWTIME	HFin;                /*  Time at the end of the measurement         */
  float    Tint;               /*  Exposure Time                              */
  short    NTracks;            /*  Number of Tracks                           */
  short    NScans;             /*  Number of Scans                            */
  short    Rejected;           /*  Number of rejected scans                   */
  short    ReguTemp;           /*  Temperature indicated by regulator         */
  short    Shutter;            /*  Shutter                                    */
  short    TempCtrl;           /*  Temperature Control on or off              */
  float    CoolerLocked;       /*  Cooler Locked                              */
  GRATINGS Grating;            /*  Information on the used grating            */
  char     Type;               /*  Type of measurement                        */
 }
CURVE_HEADER_CCD;

// ----------------
// Static variables
// ----------------

static FILE_HEADER_CCD ccdUlbFileHeader;

// -----------------------------------------------------------------------------
// FUNCTION      CCD_LoadInstrumental
// -----------------------------------------------------------------------------
// PURPOSE       Load instrumental functions needed to correct measurements
//
// INPUT         pSpecInfo   information on the current file
//
// RETURN        the code returned by MATRIX_Load if the load of a file failed
//               ERROR_ID_NO, otherwise.
// -----------------------------------------------------------------------------

RC CCD_LoadInstrumental(SPEC_INFO *pSpecInfo)
 {
 	// Declarations

  UCHAR fileName[MAX_STR_LEN+1];                                                // the complete name (including path) of the file to load
  RC rc;                                                                        // the return code

  rc=ERROR_ID_NO;

  // Offset

  if ((strlen(pSpecInfo->project.instrumental.instrFunction)>0) && (pSpecInfo->ccd.drk.matrix==NULL) &&
     ((rc=MATRIX_Load(FILES_RebuildFileName(fileName,pSpecInfo->project.instrumental.instrFunction,1),
                     &pSpecInfo->ccd.drk,
                      0 /* line base */,0 /* column base */,NDET,NCURVE,
                      0.,0.,
                      0,0,"CCD_LoadInstrumental (offset)"))!=ERROR_ID_NO))

   goto End_CCD_LoadInstrumental;

  // Interpixel variability

  if ((strlen(pSpecInfo->project.instrumental.vipFile)>0) && (pSpecInfo->ccd.vip.matrix==NULL) &&
     ((rc=MATRIX_Load(FILES_RebuildFileName(fileName,pSpecInfo->project.instrumental.vipFile,1),
                     &pSpecInfo->ccd.vip,
                      0 /* line base */,0 /* column base */,0,NCURVE,
                      0.,0.,
                      0,0,"CCD_LoadInstrumental (vip)"))!=ERROR_ID_NO))

   goto End_CCD_LoadInstrumental;

  // Non linearity of the detector

  if ((strlen(pSpecInfo->project.instrumental.dnlFile)>0) && (pSpecInfo->ccd.dnl.matrix==NULL) &&
     ((rc=MATRIX_Load(FILES_RebuildFileName(fileName,pSpecInfo->project.instrumental.dnlFile,1),
                     &pSpecInfo->ccd.dnl,
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

// -----------------------------------------------------------------------------
// FUNCTION      SetCCD_Ulb
// -----------------------------------------------------------------------------
// PURPOSE       set file pointers and get the number of records (ULB format)
//
// INPUT         pSpecInfo : information on the file to read
//               specFp    : pointer to the current spectra file
//
// OUTPUT        pSpecInfo->recordNumber, the number of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//               ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC SetCCD_Ulb(SPEC_INFO *pSpecInfo,FILE *specFp)
 {
  // Declarations

  CHAR  *indexes;                                                               // record indexes read out from file
  SHORT  curvenum;                                                              // number of records
  ULONG *recordIndexes;                                                         // save the position of each record in the file
  INDEX  i;                                                                     // browse spectra in the file
  RC     rc;                                                                    // return code

  // Initializations

  pSpecInfo->recordIndexesSize=2001;
  pSpecInfo->recordNumber=0;
  rc=ERROR_ID_NO;

  // Buffers allocation

  if (((indexes=(CHAR *)MEMORY_AllocBuffer("SetCCD_Ulb","indexes",pSpecInfo->recordIndexesSize,sizeof(CHAR),0,MEMORY_TYPE_STRING))==NULL) ||
      ((recordIndexes=pSpecInfo->recordIndexes=(ULONG *)MEMORY_AllocBuffer("SetCCD_Ulb","recordIndexes",pSpecInfo->recordIndexesSize,sizeof(ULONG),0,MEMORY_TYPE_LONG))==NULL))

   rc=ERROR_ID_ALLOC;

  // Check the input file pointer

  else if (specFp==NULL)
   rc=ERROR_SetLast("SetCCD_Ulb",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);

  else
   {
    // Headers read out

    fseek(specFp,0L,SEEK_SET);

    if (fread(&curvenum,sizeof(SHORT),1,specFp) &&
        fread(indexes,pSpecInfo->recordIndexesSize*sizeof(CHAR),1,specFp) &&
        fread(&ccdUlbFileHeader,sizeof(FILE_HEADER_CCD),1,specFp) &&
       (curvenum>0))
     {
      // Save the position of each record in the file

      pSpecInfo->recordNumber=curvenum;

      recordIndexes[0]=(LONG)pSpecInfo->recordIndexesSize*sizeof(CHAR)+sizeof(SHORT)+sizeof(FILE_HEADER_CCD);

      for (i=1;i<=curvenum;i++)
       recordIndexes[i]=recordIndexes[i-1]+sizeof(CURVE_HEADER_CCD)+(LONG)sizeof(float)*NDET*indexes[i];
     }
   }

  // Release the allocated local buffers

  if (indexes!=NULL)
   MEMORY_ReleaseBuffer("SetCCD_Ulb","indexes",indexes);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliCCD_Ulb
// -----------------------------------------------------------------------------
// PURPOSE       read the spectra measured by the CCD detector operated by ULB
//
// INPUT         pSpecInfo : information on the file to read
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
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_DIVISION_BY_0  : in case of division by 0
//               ERROR_ID_SPLINE         : if non increasing absissae are provided to the spline function;
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

RC ReliCCD_Ulb(SPEC_INFO *pSpecInfo,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *darkFp)
 {
 	// Declarations

  PRJCT_INSTRUMENTAL *pInput;                                                   // retrieve options on the current format
  CURVE_HEADER_CCD  CurveHeader;                                                // header of the current record
  double *spectrum,**vipMatrix,**dnlMatrix,dnl,**dnlDeriv,**offset;             // substitution pointers for matrices
  double tmLocal;                                                               // local time in seconds
  double dtrack;                                                                // a value of the track
  float *track;                                                                 // read a track
  INT dnlNl;                                                                    // the size of the vector for correcting the non linearity of the detector
  INDEX i,j;                                                                    // indexes for loops and arrays
  RC rc;                                                                        // return code

  // Initializations

  pInput=&pSpecInfo->project.instrumental;
  spectrum=pSpecInfo->spectrum;
  offset=(double **)pSpecInfo->ccd.drk.matrix;
  vipMatrix=(double **)pSpecInfo->ccd.vip.matrix;
  dnlMatrix=(double **)pSpecInfo->ccd.dnl.matrix;
  dnlDeriv=(double **)pSpecInfo->ccd.dnl.deriv2;
  dnlNl=pSpecInfo->ccd.dnl.nl;
  rc=ERROR_ID_NO;

  if ((track=(float *)MEMORY_AllocBuffer("ReliCCD_Ulb","track",NDET,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL)
   rc=ERROR_ID_ALLOC;
  else if (specFp==NULL)
   rc=ERROR_SetLast("ReliCCD_Ulb",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else if ((recordNo<=0) || (recordNo>pSpecInfo->recordNumber))
   rc=ERROR_ID_FILE_END;
  else
   {
    // Spectrum initialization

    VECTOR_Init(spectrum,(double)0.,NDET);

    // Set file pointer

    fseek(specFp,(LONG)pSpecInfo->recordIndexes[recordNo-1],SEEK_SET);

    // Read the header of the record

    if (!fread(&CurveHeader,sizeof(CURVE_HEADER_CCD),1,specFp))
     rc=ERROR_ID_FILE_END;

    else if ((CurveHeader.NTracks!=NCURVE) ||
            ((pInput->user>0) && (CurveHeader.Grating.NGrating!=(short)pInput->user)) ||
            ((pInput->wavelength>0) && ((CurveHeader.Grating.Nanometers<(float)pInput->wavelength-5.) ||
             (CurveHeader.Grating.Nanometers>(float)pInput->wavelength+5.))))

     rc=ERROR_ID_FILE_RECORD;

    else
     {
     	// Read track per track

      for (i=0;(i<NCURVE) && !rc;i++)
       {
        fread(track,sizeof(float)*NDET,1,specFp);

        for (j=0;(j<NDET) && !rc;j++)
         {
          // offset correction

          if (offset!=NULL)
           track[j]-=(float)offset[i][j]*CurveHeader.NScans;

          // Interpixel variability

          if (vipMatrix!=NULL)
           {
            if (vipMatrix[i][j]==(double)0.)
             rc=ERROR_SetLast("ReliCCD_Ulb",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"interpixel variability");
            else
             track[j]/=(float)(vipMatrix[i][j]*CurveHeader.NScans);
           }

          // Non linearity

          dtrack=(double)track[j];

          if (!rc && (dnlMatrix!=NULL) && (dnlDeriv!=NULL) &&
             (track[j]>=dnlMatrix[0][0]) && (track[j]<=dnlMatrix[0][dnlNl-1]) &&
            !(rc=SPLINE_Vector(dnlMatrix[0],dnlMatrix[1],dnlDeriv[1],dnlNl,&dtrack,&dnl,1,SPLINE_CUBIC,"ReliCCD_Ulb")))
           {
            if (dnl==(double)0.)
             rc=ERROR_SetLast("ReliCCD_Ulb",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"non linearity of the detector");
            else
             track[j]/=(float)dnl;
           }

          if (!rc)
           spectrum[j]+=(double)track[j];
         }
       }

      if (!rc)
       {
        // Get the information on the current record

        pSpecInfo->Tint      = (double)CurveHeader.Tint;
        pSpecInfo->NSomme    = CurveHeader.NScans;
        pSpecInfo->Zm        = -1;
        pSpecInfo->NTracks   = CurveHeader.NTracks;
        pSpecInfo->rejected  = CurveHeader.Rejected;
        pSpecInfo->SkyObs    = 0;
        pSpecInfo->TDet      = (double)ccdUlbFileHeader.Temperature;
        pSpecInfo->ReguTemp  = (short)CurveHeader.ReguTemp;

        pSpecInfo->present_day.da_year  = ccdUlbFileHeader.Day.year;
        pSpecInfo->present_day.da_mon   = ccdUlbFileHeader.Day.month;
        pSpecInfo->present_day.da_day   = ccdUlbFileHeader.Day.day;

        pSpecInfo->present_time.ti_sec=CurveHeader.HDeb.sec;
        pSpecInfo->present_time.ti_min=CurveHeader.HDeb.min;
        pSpecInfo->present_time.ti_hour=CurveHeader.HDeb.hour;

        pSpecInfo->Tm=(double)ZEN_NbSec(&pSpecInfo->present_day,&pSpecInfo->present_time,0);
        pSpecInfo->TotalExpTime=ccdUlbFileHeader.TotalExpTime;
        pSpecInfo->TimeDec=(double)CurveHeader.HDeb.hour+CurveHeader.HDeb.min/60.;

        tmLocal=pSpecInfo->Tm+THRD_localShift*3600.;

        pSpecInfo->localCalDay=ZEN_FNCaljda(&tmLocal);
        pSpecInfo->localTimeDec=fmod(pSpecInfo->TimeDec+24.+THRD_localShift,(double)24.);

        pSpecInfo->NGrating=CurveHeader.Grating.NGrating;
        pSpecInfo->Nanometers=CurveHeader.Grating.Nanometers;

        for (j=0;j<NDET;j++)
         spectrum[j]*=INVNCURVE;
       }
     }
   }

  // Release the allocated local buffer

  if (track!=NULL)
   MEMORY_ReleaseBuffer("ReliCCD_Ulb","track",track);

  // Return

  return rc;
 }



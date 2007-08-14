
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
//  Module purpose    :  PDA detectors read out routines
//  Name of module    :  PDA-READ.C
//  Program Language  :  Borland C++ 5.0 for Windows 95/NT
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in WinDOAS package in 97
//
//  Authors           :  Caroline FAYT and Ann Carine VANDAELE (ULB)
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
//  This module contains routines needed for reading out data from PDA detectors.
//  Three formats are described in this package :
//
//        PRJCT_INSTR_FORMAT_LOGGER        ASCII  format used for logger data by IASB;
//        PRJCT_INSTR_FORMAT_PDAEGG[_OLD]  binary format used for PDA EG&G by IASB;
//        PRJCT_INSTR_FORMAT_PDAEGG_ULB    binary format used for PDA EG&G by ULB.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetPDA_EGG_Logger - calculate the size and the number of records for a new file
//                      in logger format;
//
//  GotoPDA_EGG_Logger - goto the requested record number;
//
//  ReliPDA_EGG_Logger - logger format read out;
//
//  SetPDA_EGG - calculate the size and the number of records for a new file
//               in PDA EG&G binary format format;
//
//  ReliPDA_EGG - IASB PDA EG&G binary format read out;
//
//  SetPDA_EGG_Ulb - calculate the size and the number of records for a new file
//                   in ULB format;
//
//  ReliPDA_EGG_Ulb - ULB spectra read out and dark current correction;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "engine.h"

// ====================
// CONSTANTS DEFINITION
// ====================

#define LOG_LENGTH 6307

// =============
// LOGGER FORMAT
// =============

INDEX pdaLastRecord=ITEM_NONE;   // Record number of the last record read out.
                                 // The logger format is an ASCII one, so in order to speed up the
                                 // spectra read out, the file pointer is not moved for successive
                                 // spectra.

// -----------------------------------------------------------------------------
// FUNCTION      SetPDA_EGG_Logger
// -----------------------------------------------------------------------------
// PURPOSE       calculate the size and the number of records for a new file
//               in logger format
//
// INPUT         specFp      pointer to the spectra file
//
// OUTPUT        pSpecInfo   pointer to a structure whose some fields are filled
//                           with general data on the file
//
// RETURN        ERROR_ID_ALLOC           buffers allocation failed
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC SetPDA_EGG_Logger(SPEC_INFO *pSpecInfo,FILE *specFp)
 {
  // Declarations

  CHAR *record;                         // string buffer
  RC rc;                                // return code

  // Initializations

  pdaLastRecord=ITEM_NONE;
  pSpecInfo->recordNumber=0;
  rc=ERROR_ID_NO;

  // Buffer allocation

  if ((record=(char *)MEMORY_AllocBuffer("SetPDA_EGG_Logger ","record",8001,sizeof(CHAR),0,MEMORY_TYPE_STRING))==NULL)
   rc=ERROR_ID_ALLOC;
  else if (specFp==NULL)
   rc=ERROR_SetLast("SetPDA_EGG_Logger",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else
   {
    // Count the number of records in the file

//    fseek(specFp,0L,SEEK_SET);

//    while ((fgets(record,8000,specFp)!=0))
//     pSpecInfo->recordNumber++;

    pSpecInfo->recordNumber=STD_FileLength(specFp)/LOG_LENGTH;
   }

  // Release the allocated buffer

  if (record!=NULL)
   MEMORY_ReleaseBuffer("SetPDA_EGG_Logger ","record",record);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GotoPDA_EGG_Logger
// -----------------------------------------------------------------------------
// PURPOSE       goto the requested record number
//
// INPUT         specFp      pointer to the spectra file
//               recordNo    the record number (0 base indexed)
//
// RETURN        ERROR_ID_ALLOC           buffers allocation failed
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC GotoPDA_EGG_Logger(FILE *specFp,int recordNo)
 {
  // Declarations

  char *record;
  int recordSize;
  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Buffer allocation

  if ((record=(char *)MEMORY_AllocBuffer("GotoPDA_EGG_Logger ","record",8001,sizeof(char),0,MEMORY_TYPE_STRING))==NULL)
   rc=ERROR_ID_ALLOC;
  else if (specFp==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;
  else
   {
    // Goto back to the beginning of the file

    fseek(specFp,0L,SEEK_SET);

    // Skip the requested number of record

    fgets(record,8000,specFp);                             // read out the first record for calculating its size
    recordSize=strlen(record);                             // each record has the same size in despite this is an ASCII format
    fseek(specFp,(LONG)recordSize*recordNo,SEEK_SET);      // move the file pointer to the requested record

//    fseek(specFp,(LONG)LOG_LENGTH*recordNo,SEEK_SET);      // move the file pointer to the requested record
   }

  // Release the allocated buffer

  if (record!=NULL)
   MEMORY_ReleaseBuffer("GotoPDA_EGG_Logger ","record",record);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliPDA_EGG_Logger
// -----------------------------------------------------------------------------
// PURPOSE       logger format read out
//
// INPUT         recordNo     index of record in file
//               dateFlag     0 no date constraint; 1 a date selection is applied
//               specFp       pointer to the spectra file
//
// OUTPUT        pSpecInfo  : pointer to a structure whose some fields are filled
//                            with data on the current spectrum
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC ReliPDA_EGG_Logger(SPEC_INFO *pSpecInfo,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  char       *record,                                   // string buffer for record read out
             *p;                                        // pointer to characters in the previous string buffer
  double     *spectrum,tmLocal,                         // current spectrum
              Max;                                      // maximum value used for normalizing spectra during safe keeping
  SHORT_DATE  day;                                      // date of the current spectrum
  INT         iday,imonth,iyear,ihour,imin,isec,        // substitution variable for current measurement date and time read out
              ccdFlag;                                  // 0 for RETICON detector, 1 for CCD detector
  INDEX       i;                                        // index for browsing pixels in spectrum
  SZ_LEN      nameLen;
  RC          rc;                                       // return code

  // Initializations

  spectrum=pSpecInfo->spectrum;
  rc=ERROR_ID_NO;

  // Buffer allocation

  if ((record=(char *)MEMORY_AllocBuffer("ReliPDA_EGG_Logger ","record",8001,sizeof(char),0,MEMORY_TYPE_STRING))==NULL)
   rc=ERROR_ID_ALLOC;
  else if ((recordNo<=0) || (recordNo>pSpecInfo->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Set file pointer

  else if ((recordNo-pdaLastRecord==1) ||               // for successive spectra, don't move file pointer
          !(rc=GotoPDA_EGG_Logger(specFp,recordNo-1)))
   {
    pdaLastRecord=recordNo;

    // Record read out

    if (!fgets(record,8001,specFp) ||
//    if (!fread(record,LOG_LENGTH,1,specFp) ||
      (((p=strstr(record,"R "))==NULL) &&      // R holds for RETICON detectors
       ((p=strstr(record,"H "))==NULL) &&      // H holds for HAMAMATSU detectors
       ((p=strstr(record,"C "))==NULL)))       // C holds for CCD detectors

     rc=ERROR_ID_FILE_END;

    else
     {
      ccdFlag=(p[0]=='C')?1:0;

      // record read out

      p+=2;

      sscanf(p," %[^' '] %d/%d/%d %d:%d:%d %d %d %lf %lf %lf %f %f",

                 pSpecInfo->Nom,                              // record name
                &iday,&imonth,&iyear,&ihour,&imin,&isec,      // date and time
                &pSpecInfo->NSomme,                           // number of saved scans
                &pSpecInfo->rejected,                         // number of rejected scans
                &pSpecInfo->Tint,                             // integration time
                &pSpecInfo->Zm,                               // zenith angle
                &Max,                                         // scaling factor
                &pSpecInfo->azimuthViewAngle,
                &pSpecInfo->elevationViewAngle);

      if (!pSpecInfo->project.instrumental.azimuthFlag || (pSpecInfo->Azimuth==99999.))
       pSpecInfo->Azimuth=(double)-1.;
      if (pSpecInfo->elevationViewAngle==99999.)
       pSpecInfo->elevationViewAngle=(double)-1.;
      if (pSpecInfo->azimuthViewAngle==99999.)
       pSpecInfo->azimuthViewAngle=(double)-1.;
      else
       pSpecInfo->azimuthViewAngle-=90.;                                        // mirror is perpendicular to the direction of the sun

      // Build date and time of the current measurement

/*      { // !!!!!!!!!!!!
       INT inc,dayMonth[13]={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365}, maxDay;

       ihour+=13;
       maxDay=dayMonth[imonth];
       if ((iyear%4==0) & (imonth>=2))
        maxDay++;

       if (ihour>=24)
        {
         ihour%=24;
         iday++;

         if (iday>maxDay)
          {
           iday=1;
           imonth++;

           if (imonth>12)
            {
             imonth=1;
             iyear++;
            }
          }
        }
      } // !!!!!!!!!!!!  */

      day.da_day=(UCHAR)iday;
      day.da_mon=(UCHAR)imonth;
      day.da_year=(SHORT)iyear;

      if (day.da_year<30)
       day.da_year+=(short)2000;
      else if (day.da_year<130)
       day.da_year+=(short)1900;
      else if (day.da_year<1930)
       day.da_year+=(short)100;

      pSpecInfo->present_time.ti_hour=(UCHAR)ihour;
      pSpecInfo->present_time.ti_min=(UCHAR)imin;
      pSpecInfo->present_time.ti_sec=(UCHAR)isec;
//      pSpecInfo->Azimuth=(double)-1.;

      memcpy(&pSpecInfo->present_day,&day,sizeof(SHORT_DATE));

      // Available data on the current spectrum

      pSpecInfo->Tm=(double)ZEN_NbSec(&pSpecInfo->present_day,&pSpecInfo->present_time,0);
      pSpecInfo->TotalExpTime=(double)0.;
      pSpecInfo->TimeDec=(double)pSpecInfo->present_time.ti_hour+pSpecInfo->present_time.ti_min/60.+pSpecInfo->present_time.ti_sec/3600.;

      tmLocal=pSpecInfo->Tm+THRD_localShift*3600.;

      pSpecInfo->localCalDay=ZEN_FNCaljda(&tmLocal);
      pSpecInfo->localTimeDec=fmod(pSpecInfo->TimeDec+24.+THRD_localShift,(double)24.);

      nameLen=strlen(pSpecInfo->Nom);

      if (!pSpecInfo->NSomme || // (pSpecInfo->rejected>=pSpecInfo->NSomme) ||
         ((pSpecInfo->project.instrumental.user==PRJCT_INSTR_IASB_TYPE_ZENITHAL) && (pSpecInfo->Nom[nameLen-4]!='Z')) ||
         ((pSpecInfo->project.instrumental.user==PRJCT_INSTR_IASB_TYPE_OFFAXIS) && (pSpecInfo->Nom[nameLen-4]!='o')) ||
         (dateFlag && (pSpecInfo->localCalDay!=localDay)))

       rc=ERROR_ID_FILE_RECORD;

      else
       {
        // Spectrum read out

        p += 142;
        Max*=(double)pSpecInfo->NSomme/65000.;

        for (i=0;i<NDET;i++)
         {
          sscanf(p,"%lf",&spectrum[i]);
          spectrum[i]*=(double)Max/pSpecInfo->NSomme;  // test pSpecInfo->NSomme==0 is made before
          p+=6;
         }

        if (ccdFlag)
         VECTOR_Invert(spectrum,NDET);
       }
     }
   }

  // Release allocated buffer

  if (record!=NULL)
   MEMORY_ReleaseBuffer("ReliPDA_EGG_Logger ","record",record);

  // Return

  return rc;
 }

// =======================================
// BINARY FORMAT USED FOR PDA EG&G BY IASB
// =======================================

// ------------------
// Record description
// ------------------

#define PDA1453A struct PDA1453A
PDA1453A
 {
  float       ReguTemp;
  SHORT       Detector_Temp;
  SHORT       ScansNumber;
  SHORT       Rejected;
  double      Exposure_time;
  double      Zm;
  char        SkyObs;
  SHORT_DATE  today;
  struct time now;
  double      azimuth;
  float       mirrorElv;
 };

// ---------------------------------------------------------
// Arrays of integration times used by the real time program
// ---------------------------------------------------------

#define MAXTPS1 30 // Harestua
#define MAXTPS2 36 // OHP

float Tps1[MAXTPS1] = { (float)   0.1, (float)   0.2, (float)   0.3, (float)   0.4, (float)   0.55, (float)   0.75, (float)   1.,  (float)  1.5, (float)   2.,
                        (float)   3. , (float)   4. , (float)   5.5, (float)   7.5, (float)  10.  , (float)  15.  , (float)  20.,  (float) 30. , (float)  40.,
                        (float)  55. , (float)  75. , (float) 100. , (float) 125. , (float) 150.  , (float) 175.  , (float) 200.,  (float) 225., (float) 250.,
                        (float) 275. , (float) 300.,  (float) 600. };
float Tps2[MAXTPS2] =
 { (float)  1.00, (float)  1.20, (float)  1.50, (float)  1.75, (float)  2.00, (float)  2.50, (float)  3.00, (float)  3.60, (float)   4.30, (float)   5.20, (float)   6.20, (float)   7.50, (float)   8.90, (float)  11.00, (float)  13.00, (float)  16.00, (float)  19.00, (float)  22.00,
   (float) 27.00, (float) 32.00, (float) 38.00, (float) 46.00, (float) 55.00, (float) 66.00, (float) 80.00, (float) 95.00, (float) 115.00, (float) 140.00, (float) 160.00, (float) 200.00, (float) 240.00, (float) 280.00, (float) 340.00, (float) 410.00, (float) 490.00, (float) 590.00 };

// -----------------------------------------------------------------------------
// FUNCTION      SetPDA_EGG
// -----------------------------------------------------------------------------
// PURPOSE       calculate the size and the number of records for a new file
//               in PDA EG&G binary format
//
// INPUT         specFp      pointer to the spectra file
//               newFlag     0 format used during intercomparison campaign at Camborne, ENGLAND, 94
//                           1 format used from spring 94 until now
//
// OUTPUT        pSpecInfo   pointer to a structure whose some fields are filled
//                           with general data on the file
//
// RETURN        ERROR_ID_ALLOC           buffers allocation failed
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_EMPTY      the file is empty;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC SetPDA_EGG(SPEC_INFO *pSpecInfo,FILE *specFp,INT newFlag)
 {
  // Declarations

  PDA1453A header;                         // record header
  SHORT *indexes,                          // size of SpecMax arrays
         curvenum;                         // number of spectra in the file
  ULONG  recordSize,                       // size of a record without SpecMax
        *recordIndexes;                    // save the position of each record in the file
  INDEX i;                                 // browse spectra in the file
  RC rc;                                   // return code

  // Initializations

  pSpecInfo->recordIndexesSize=2001;
  recordIndexes=pSpecInfo->recordIndexes;
  rc=ERROR_ID_NO;

  // Buffers allocation

  if (((indexes=(SHORT *)MEMORY_AllocBuffer("SetPDA_EGG ","indexes",pSpecInfo->recordIndexesSize,sizeof(SHORT),0,MEMORY_TYPE_SHORT))==NULL) ||
      ((pSpecInfo->specMax=MEMORY_AllocDVector("SetPDA_EGG ","specMax",0,NDET-1))==NULL))

   rc=ERROR_ID_ALLOC;

  // Open spectra file

  else if (specFp==NULL)
   rc=ERROR_SetLast("SetPDA_EGG",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else
   {
// !!! */    UCHAR *ptr,fileout[MAX_ITEM_TEXT_LEN+1];
// !!! */    FILE *fp;

// !!! */    ptr=strrchr(fileName,PATH_SEP);
// !!! */    sprintf(fileout,"e:%s",ptr);
// !!! */    fp=fopen(fileout,"a+b");

    // Headers read out

    fseek(specFp,0L,SEEK_SET);

    if (!fread(&curvenum,sizeof(SHORT),1,specFp) ||
        !fread(indexes,pSpecInfo->recordIndexesSize*sizeof(SHORT),1,specFp) ||
        (curvenum<=0))

     rc=ERROR_SetLast("SetPDA_EGG",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pSpecInfo->fileName);

    else
     {
      // Save the position of each record in the file

// !!! */      fwrite(&curvenum,sizeof(SHORT),1,fp);
// !!! */      fwrite(indexes,pSpecInfo->recordIndexesSize*sizeof(SHORT),1,fp);
// !!! */      fclose(fp);

      pSpecInfo->recordNumber=curvenum;

      pSpecInfo->recordSize=recordSize=(LONG)sizeof(PDA1453A)-
                                             sizeof(double)-                    // azimuth (only since dec 2000)
                                             sizeof(float);                     // elevation (only since 2003)

      fread(&header,recordSize,1,specFp);

      if (pSpecInfo->project.instrumental.azimuthFlag)
       {
        pSpecInfo->recordSize+=sizeof(double);
        if ((header.today.da_year>=2003) || (header.today.da_year==1993))
         pSpecInfo->recordSize+=sizeof(float);
       }

      fseek(specFp,-((LONG)recordSize),SEEK_CUR);

      recordSize=pSpecInfo->recordSize+(LONG)sizeof(USHORT)*NDET+(300L*(sizeof(SHORT)+sizeof(float))+8L)*newFlag;

//      recordSize=(LONG)sizeof(PDA1453A)-((!pSpecInfo->project.instrumental.azimuthFlag)?
//                       sizeof(double):0)+(LONG)sizeof(USHORT)*NDET+(300L*(sizeof(SHORT)+sizeof(float))+8L)*newFlag;

      recordIndexes[0]=(LONG)(pSpecInfo->recordIndexesSize+1)*sizeof(SHORT);      // file header : size of indexes table + curvenum

      for (i=1;i<curvenum;i++)
       recordIndexes[i]=recordIndexes[i-1]+recordSize+indexes[i]*sizeof(USHORT);  // take size of SpecMax arrays into account
     }
   }

  // Release local buffers

  if (indexes!=NULL)
   MEMORY_ReleaseBuffer("SetPDA_EGG ","indexes",indexes);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliPDA_EGG
// -----------------------------------------------------------------------------
// PURPOSE       IASB PDA EG&G binary format read out
//
// INPUT         recordNo     index of record in file;
//               dateFlag     0 no date constraint; 1 a date selection is applied;
//               specFp       pointer to the spectra file;
//               namesFp      pointer to the names file if any;
//               darkFp       pointer to the dark current files if any;
//               newFlag      0 format used during intercomparison campaign at Camborne, ENGLAND, 94
//                            1 format used from spring 94 until now;
//
// OUTPUT        pSpecInfo  : pointer to a structure whose some fields are filled
//                            with data on the current spectrum
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC ReliPDA_EGG(SPEC_INFO *pSpecInfo,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp,INT newFlag)
 {
  // Declarations

  PDA1453A          header;                             // file header
  OBSERVATION_SITE *pSite;                              // pointer to the observation sites data
  UCHAR             fileNameShort[MAX_STR_SHORT_LEN+1], // temporary file name
                   *ptr,                                // pointer to part of the previous string
                    names[20];                          // name of the current spectrum
  USHORT           *ISpectre,                           // spectrum in the original format
                   *ISpecMax,                           // maximum values for each scan
                   *TabNSomme;                          // scans number for each integration time
  double            SMax,                               // maximum value of the current not normalized spectrum
                    Max,                                // scaling factor used for normalizing spectrum before safe keeping in the file
                   *ObsScan,                            // dark current spectrum
                    longit,latit,azimuth,Tm,tmLocal;    // temporary data
  INT               azimuthFlag,                        // 1 if format with azimuth angle; 0 otherwise
                    MAXTPS,                             // size of the integration time array to use
                    indexSite;                          // index of the observation site in the sites list
  float            *TabTint,*Tps;                       // integration time arrays
  INDEX             i,j,k;                              // indexes for loops and arrays
  FILE             *fp;                                 // file pointer
  SZ_LEN            nameLen;
  RC                rc;                                 // return code

// !!! */    FILE *gp;
// !!! */    UCHAR fileout[MAX_ITEM_TEXT_LEN+1];

  // Initializations

  memset(fileNameShort,0,MAX_STR_SHORT_LEN+1);
  strncpy(fileNameShort,pSpecInfo->fileName,MAX_STR_SHORT_LEN);

  ISpectre=ISpecMax=TabNSomme=NULL;
  azimuthFlag=pSpecInfo->project.instrumental.azimuthFlag;
  memset(names,0,20);
  TabTint=NULL;
  ObsScan=NULL;
  fp=NULL;

  Tps=(azimuthFlag)?Tps2:Tps1;
  MAXTPS=(azimuthFlag)?MAXTPS2:MAXTPS1;

// !!! */    ptr=strrchr(fileName,PATH_SEP);
// !!! */    sprintf(fileout,"e:%s",ptr);
// !!! */    gp=fopen(fileout,"a+b");

  rc=ERROR_ID_NO;

  if (specFp==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;
  else if ((recordNo<=0) || (recordNo>pSpecInfo->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Buffers allocation

  else if (((ISpectre=(USHORT *)MEMORY_AllocBuffer("ReliPDA_EGG ","ISpectre",NDET,sizeof(USHORT),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((ObsScan=(double *)MEMORY_AllocBuffer("ReliPDA_EGG ","ObsScan",NDET,sizeof(double),0,MEMORY_TYPE_DOUBLE))==NULL) ||
           ((ISpecMax=(USHORT *)MEMORY_AllocBuffer("ReliPDA_EGG ","ISpecMax",2000,sizeof(USHORT),0,MEMORY_TYPE_USHORT))==NULL) ||
           ((TabTint=(float *)MEMORY_AllocBuffer("ReliPDA_EGG ","TabTint",300,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
           ((TabNSomme=(USHORT *)MEMORY_AllocBuffer("ReliPDA_EGG ","TabNSomme",300,sizeof(USHORT),0,MEMORY_TYPE_USHORT))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Set file pointers

    if (namesFp!=NULL)
     fseek(namesFp,(LONG)20L*((recordNo-1)*(1+!newFlag)+!newFlag),SEEK_SET);

    fseek(specFp,(LONG)pSpecInfo->recordIndexes[recordNo-1],SEEK_SET);

    // Complete record read out

    if (((namesFp!=NULL) && !fread(names,20,1,namesFp)) ||
         !fread(&header,pSpecInfo->recordSize,1,specFp) ||
         !fread(ISpectre,sizeof(USHORT)*NDET,1,specFp) ||
         (newFlag &&
        (!fread(TabTint,sizeof(float)*300,1,specFp) ||
         !fread(TabNSomme,sizeof(USHORT)*300,1,specFp) ||
         !fread(&Max,sizeof(double),1,specFp))) ||
         !fread(ISpecMax,sizeof(USHORT)*(header.Rejected+header.ScansNumber),1,specFp))

     rc=ERROR_ID_FILE_END;

    else
     {
      nameLen=strlen(names);

      if (((pSpecInfo->project.instrumental.user==PRJCT_INSTR_IASB_TYPE_ZENITHAL) && (names[nameLen-4]!='Z')) ||
          ((pSpecInfo->project.instrumental.user==PRJCT_INSTR_IASB_TYPE_OFFAXIS) && (names[nameLen-4]!='o')))

       rc=ERROR_ID_FILE_RECORD;

      else
       {
        // Get the maximum value of the read out spectrum

        for (i=1,SMax=(double)ISpectre[0];i<NDET;i++)
         if ((double)ISpectre[i]>SMax)
          SMax=(double)ISpectre[i];

        // Time deviation correction

        {
         int Heure, Min, Sec, Np, Jday;
         double  DHour, DMin, DifUT;
         FILE *fp;

         if ((ptr=strrchr(fileNameShort,PATH_SEP))!=NULL)
          ptr++;
         else
          ptr=fileNameShort;

         *ptr=0;
         if ((int)(&fileNameShort[MAX_STR_SHORT_LEN]-ptr)>=(int)strlen("Drift.ret"))
          strcpy(ptr,"Drift.ret");

         DHour = (double) header.now.ti_hour+header.now.ti_min/60.+header.now.ti_sec/3600.;
         Tm=ZEN_NbSec(&header.today,&header.now,0);

         fp = fopen ( fileNameShort, "rt" );

         if ( fp != NULL )
           {
             fscanf ( fp, "%d", &Np );
             for ( i=0; i<Np; i++ )
               {
                 fscanf ( fp, "%d %lf", &Jday, &DifUT );
                 if ( Jday < 1 )  Jday += 365;
                 if ( Jday == (int) ZEN_FNCaljda(&Tm) ) break;
               }

             if ( Jday == ZEN_FNCaljda(&Tm) )
               {
                 DHour -= (DifUT / 60.);  /* DifUT donne en minutes  */
                 Heure = (int) DHour;
                 DMin  = (double) (DHour-Heure) * 60.;
                 Min   = (int) DMin;
                 Sec   = (int)((double)(DMin-Min) * 60.);

                 header.now.ti_hour = (UCHAR)Heure;
                 header.now.ti_min  = (UCHAR)Min;
                 header.now.ti_sec  = (UCHAR)Sec;

                 Tm = (double) ZEN_NbSec ( &header.today, &header.now, 0 );

                 if ((indexSite=SITES_GetIndex(pSpecInfo->project.instrumental.observationSite))!=ITEM_NONE)
                  {
                   pSite=&SITES_itemList[indexSite];

                   longit=-pSite->longitude;  // !!! sign is inverted
                   latit=pSite->latitude;

                   header.Zm = ZEN_FNTdiz(ZEN_FNCrtjul(&Tm),&longit,&latit,&azimuth);
                  }
               }

             fclose ( fp );
           }
        }

// ! ***/           fwrite(&header,sizeof(PDA1453A),1,gp);
// ! ***/           fwrite(ISpectre,sizeof(USHORT)*NDET,1,gp);
// ! ***/           fwrite(TabTint,sizeof(float)*300,1,gp);
// ! ***/           fwrite(TabNSomme,sizeof(USHORT)*300,1,gp);
// ! ***/           fwrite(&Max,sizeof(double),1,gp);
// ! ***/           fwrite(ISpecMax,sizeof(USHORT)*(header.Rejected+header.ScansNumber),1,gp);
// ! ***/           fclose(gp);

        memcpy((char *)&pSpecInfo->present_day,(char *)&header.today,sizeof(SHORT_DATE));
        memcpy((char *)&pSpecInfo->present_time,(char *)&header.now,sizeof(struct time));

        pSpecInfo->Tm=(double)ZEN_NbSec(&pSpecInfo->present_day,&pSpecInfo->present_time,0);
        pSpecInfo->TotalExpTime=(double)0.;
        pSpecInfo->TimeDec=(double)header.now.ti_hour+header.now.ti_min/60.+header.now.ti_sec/3600.;

        tmLocal=pSpecInfo->Tm+THRD_localShift*3600.;

        pSpecInfo->localCalDay=ZEN_FNCaljda(&tmLocal);
        pSpecInfo->localTimeDec=fmod(pSpecInfo->TimeDec+24.+THRD_localShift,(double)24.);
        pSpecInfo->elevationViewAngle=(pSpecInfo->present_day.da_year>=2003)?header.mirrorElv:(float)-1.;

        // User constraints

        if ((SMax==(double)0.) || (header.ScansNumber<=0) ||
            (dateFlag && (pSpecInfo->localCalDay!=localDay)))

         rc=ERROR_ID_FILE_RECORD;

        else
         {
          // Build the original spectrum

          if (newFlag)
           {
            if (pSpecInfo->specMax!=NULL)
             for (i=0;i<(header.Rejected+header.ScansNumber);i++)
              pSpecInfo->specMax[i]=(double)ISpecMax[i]; // *0.5;

            for (i=0;i<NDET;i++)
             pSpecInfo->spectrum[i]=(double)ISpectre[i]*Max/65000.; //SMax;
           }
          else
           {
            if (pSpecInfo->specMax!=NULL)
             for (i=0,Max=(double)0.;i<(header.Rejected+header.ScansNumber);i++)
              {
               Max+=(double)ISpecMax[i];
               pSpecInfo->specMax[i]=(double)ISpecMax[i];
              }

            for (i=0,Max/=(double)header.ScansNumber;i<NDET;i++ )
             pSpecInfo->spectrum[i]=(double)ISpectre[i]*Max/(SMax*2);
           }

          // Data on the current record

          pSpecInfo->TDet     = header.Detector_Temp;
          pSpecInfo->Tint     = header.Exposure_time;
          pSpecInfo->NSomme   = header.ScansNumber;
          pSpecInfo->Zm       = header.Zm;
          pSpecInfo->SkyObs   = header.SkyObs;
          pSpecInfo->rejected = header.Rejected;
          pSpecInfo->ReguTemp = header.ReguTemp;

          pSpecInfo->azimuthViewAngle  = (azimuthFlag)?(float)header.azimuth:(float)-1.;

          memcpy(pSpecInfo->Nom,names,20);

          // Dark current correction

          if (pSpecInfo->darkCurrent!=NULL)
           for (i=0;i<NDET;i++)
            pSpecInfo->darkCurrent[i]=(double)0.;

          if (newFlag)
           {
            k=0;

            while (TabNSomme[k]!=0)
             {
              pSpecInfo->TotalExpTime+=(double)TabNSomme[k]*TabTint[k];

              for (j=0;j<MAXTPS;j++)
               if (TabTint[k]==Tps[j])
                break;

              if ((j!=MAXTPS) && (pSpecInfo->darkCurrent!=NULL) && (darkFp!=NULL) &&
                  ((int)STD_FileLength(darkFp)>=(int)((j+1)*sizeof(double)*NDET)))
               {
                fseek(darkFp,(LONG)j*NDET*sizeof(double),SEEK_SET);
                fread(ObsScan,sizeof(double)*NDET,1,darkFp);

                for (i=0;i<NDET;i++)
                 pSpecInfo->darkCurrent[i]+=(double)ObsScan[i]*TabNSomme[k];
               }

              k++;
             }
           }
          else if (pSpecInfo->darkCurrent!=NULL)
           {
            fseek(darkFp,(LONG)(pSpecInfo->recordIndexesSize+1)*sizeof(SHORT)+(pSpecInfo->recordSize+(LONG)sizeof(USHORT)*NDET)*(recordNo-1),SEEK_SET);

            fread(&header,pSpecInfo->recordSize,1,darkFp);
            fread(ISpectre,sizeof(USHORT)*NDET,1,darkFp);

            for (i=0;i<NDET;i++)
             pSpecInfo->darkCurrent[i]=(double)ISpectre[i];
           }

          if (pSpecInfo->NSomme!=0)
           for (i=0;i<NDET;i++)
            {
             pSpecInfo->spectrum[i]/=(double)pSpecInfo->NSomme;
             if (pSpecInfo->darkCurrent!=NULL)
              pSpecInfo->darkCurrent[i]/=(double)pSpecInfo->NSomme;
            }
         }
       }
     }
   }

//  **********************************
//  CONVERSION INTO DATA LOGGER FORMAT
//  **********************************

//  if (!rc)
//   {
//    FILE *logFp;
//    char fileName[40];

//    sprintf(fileName,"zard%03d.%02du",ZEN_FNCaljda(&pSpecInfo->Tm),pSpecInfo->present_day.da_year%100);

//    if ((logFp=fopen(fileName,"a+t"))!=NULL)
//     {
//      Max=(double)pSpecInfo->spectrum[0]-pSpecInfo->darkCurrent[0];

//      for (i=0;i<NDET;i++)
//       if ((double)pSpecInfo->spectrum[i]-pSpecInfo->darkCurrent[i]>Max)
//        Max=(double)pSpecInfo->spectrum[i]-pSpecInfo->darkCurrent[i];

//      sprintf(pSpecInfo->Nom,"%#3d%02dHA%02d%02dZ%03d    ",ZEN_FNCaljda(&pSpecInfo->Tm),pSpecInfo->present_day.da_year%100,
//                      pSpecInfo->present_time.ti_hour,
//                      pSpecInfo->present_time.ti_min,
//                (int)(pSpecInfo->Zm*10.));

//      fprintf(logFp,"%04d/%02d/%02d %02d:%02d H %-20s %02d/%02d/%02d %02d:%02d:%02d %05d %05d %07.3f %4.1f %05d ",
//                      pSpecInfo->present_day.da_year,
//                      pSpecInfo->present_day.da_mon,
//                      pSpecInfo->present_day.da_day,
//                      pSpecInfo->present_time.ti_hour,
//                      pSpecInfo->present_time.ti_min,
//                      pSpecInfo->Nom,
//                      pSpecInfo->present_day.da_day,
//                      pSpecInfo->present_day.da_mon,
//                      pSpecInfo->present_day.da_year%100,
//                      pSpecInfo->present_time.ti_hour,
//                      pSpecInfo->present_time.ti_min,
//                      pSpecInfo->present_time.ti_sec,
//                      pSpecInfo->NSomme,                           // number of saved scans
//                      pSpecInfo->rejected,                         // number of rejected scans
//                      pSpecInfo->Tint,                             // integration time
//                      pSpecInfo->Zm,                               // zenith angle
//                      (int)(Max+0.5));                             // scaling factor

//      fprintf(logFp,"99999 99999 99999 99999 99999 99999 99999 99999 99999 99999 99999 99999 " );

//      for (i=0;i<NDET;i++)
//       fprintf(logFp,"%05d ",(pSpecInfo->spectrum[i]-pSpecInfo->darkCurrent[i])<=(double)0.?0:
//                       (int)((pSpecInfo->spectrum[i]-pSpecInfo->darkCurrent[i])*65000./Max));

//      fprintf(logFp,"\n");
//     }

//    // Release the allocated buffer

//    if (logFp!=NULL)
//     fclose(logFp);
//   }

  // Close file

  if (fp!=NULL)
   fclose(fp);

/*        {
         FILE *fp;
         fp=fopen("C:\\My Data\\Stations\\OHP\\Analysis\\ohp_Tint.dat","a+t");
         fprintf(fp,"%#5d %#3d %.3f %.2f %.2f\n",pSpecInfo->present_day.da_year,ZEN_FNCaljda(&pSpecInfo->Tm),pSpecInfo->TimeDec,pSpecInfo->Zm,pSpecInfo->Tint);
         fclose(fp);
        }    */

  // Release allocated buffers

  if (ISpectre!=NULL)
   MEMORY_ReleaseBuffer("ReliPDA_EGG ","ISpectre",ISpectre);
  if (ObsScan!=NULL)
   MEMORY_ReleaseBuffer("ReliPDA_EGG ","ObsScan",ObsScan);
  if (ISpecMax!=NULL)
   MEMORY_ReleaseBuffer("ReliPDA_EGG ","ISpecMax",ISpecMax);
  if (TabNSomme!=NULL)
   MEMORY_ReleaseBuffer("ReliPDA_EGG ","TabNSomme",TabNSomme);
  if (TabTint!=NULL)
   MEMORY_ReleaseBuffer("ReliPDA_EGG ","TabTint",TabTint);

  // Return

  return rc;
 }

// =====================================
// ULB FORMAT (ref. Ann Carine VANDAELE)
// =====================================

// --------------------
// Constants definition
// --------------------

#define RETBADPIXEL 3

// ----------------------
// Description of records
// ----------------------

// File header

typedef struct tagFILE_HEADER_RET
 {
  char         Reference[30];          // campaign information
  char         Spectrometer[10];       // spectrometer type
  char         Detector[10];           // detector used with the spectrometer
  SHORT        Temperature;            // detector temperature
  SHORT        BI1, BS1, BI2, BS2;     // low and high bounds
  LONG         MeasurementTime;        // time for one Measurement
  LONG         DelayTime;              // delay between two measurements
  LONG         TotalExpTime;           // total experiment time
  NEWDATE	Day;                   // date of the measurement
 }
FILE_HEADER_RET;

// Spectrum header

typedef struct tagCURVE_HEADER_RET
 {
  NEWTIME	HDeb;                  // time at the beginning of the measurement
  NEWTIME	HFin;          	       // time at the end of the measurement
  float  	Tint;                  // exposure Time
  SHORT    	NScans;                // number of Scans
  SHORT    	Rejected;              // number of rejected scans
  float  	ReguTemp;              // temperature indicated by regulator
  SHORT    	Shutter;               // shutter
  SHORT    	CoolerLocked;          // cooler locked
  char   	Type;                  // type of measurement
 }
CURVE_HEADER_RET;

FILE_HEADER_RET pdaUlbFileHeader;

// -----------------------------------------------------------------------------
// FUNCTION      SetPDA_EGG_Ulb
// -----------------------------------------------------------------------------
// PURPOSE       calculate the size and the number of records for a new file
//               in ULB format
//
// INPUT         specFp      pointer to the spectra file
//
// OUTPUT        pSpecInfo   pointer to a structure whose some fields are filled
//                           with general data on the file
//
// RETURN        ERROR_ID_ALLOC           buffers allocation failed
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_EMPTY      the file is empty;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC SetPDA_EGG_Ulb(SPEC_INFO *pSpecInfo,FILE *specFp)
 {
  // Declarations

  CHAR  *indexes;                      // record indexes read out from file
  SHORT  curvenum;                     // number of records
  ULONG  recordSize,                   // size in bytes of a record
        *recordIndexes;                // save the position of each record in the file
  INDEX  i;                            // browse spectra in the file
  RC     rc;                           // return code

  // Initializations

  pSpecInfo->recordIndexesSize=2001;
  recordIndexes=pSpecInfo->recordIndexes;
  rc=ERROR_ID_NO;

  // Buffers allocation

  if ((indexes=(CHAR *)MEMORY_AllocBuffer("SetPDA_EGG_Ulb ","indexes",pSpecInfo->recordIndexesSize,sizeof(CHAR),0,MEMORY_TYPE_STRING))==NULL)

   rc=ERROR_ID_ALLOC;

  // Open spectra file

  else if (specFp==NULL)
   rc=ERROR_SetLast("SetPDA_EGG_Ulb",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else
   {
    // Headers read out

    fseek(specFp,0L,SEEK_SET);

    if (!fread(&curvenum,sizeof(SHORT),1,specFp) ||
        !fread(indexes,pSpecInfo->recordIndexesSize*sizeof(CHAR),1,specFp) ||
        !fread(&pdaUlbFileHeader,sizeof(FILE_HEADER_RET),1,specFp) ||
        (curvenum<=0))

     rc=ERROR_SetLast("SetPDA_EGG_Ulb",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pSpecInfo->fileName);

    else
     {
      // Save the position of each record in the file

      pSpecInfo->recordNumber=curvenum;

      recordSize=(LONG)sizeof(CURVE_HEADER_RET)+(LONG)sizeof(double)*NDET;
      recordIndexes[0]=(LONG)pSpecInfo->recordIndexesSize*sizeof(CHAR)+sizeof(SHORT)+sizeof(FILE_HEADER_RET);

      for (i=1;i<=curvenum;i++)
       recordIndexes[i]=recordIndexes[i-1]+recordSize*indexes[i];
     }
   }

  // Release local buffers

  if (indexes!=NULL)
   MEMORY_ReleaseBuffer("SetPDA_EGG_Ulb ","indexes",indexes);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliPDA_EGG_Ulb
// -----------------------------------------------------------------------------
// PURPOSE       ULB spectra read out and dark current correction
//
// INPUT         recordNo     index of record in file
//               dateFlag     0 no date constraint; 1 a date selection is applied
//               specFp       pointer to the spectra file
//               namesFp      pointer to the names file if any
//               darkFp       pointer to the dark currents file if any
//
// OUTPUT        pSpecInfo  : pointer to a structure whose some fields are filled
//                            with data on the current spectrum
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_NOT_FOUND : the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : record not found
//               ERROR_ID_DIVISION_BY_0  : division by 0;
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC ReliPDA_EGG_Ulb(SPEC_INFO *pSpecInfo,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp)
 {
  // Declarations

  CURVE_HEADER_RET  CurveHeader;      // header of the current record
  ULONG             recordSize,       // size in bytes of a record
                    nCurve;           // the number of spectra in the current record
  double           *spectrum,         // spectrum satisfying user constraints
                   *tmpVector,
                  **tmpMatrix,
                    tmLocal;

  INDEX             i,j,k;            // indexes for loops and arrays
  INT               found;            // 0 record not found; 1 record found
  INT               limSup,nb;        // verify bounds
  RC                rc;               // return code

  // Initializations

  recordSize=(LONG)sizeof(CURVE_HEADER_RET)+(LONG)sizeof(double)*NDET;
  spectrum=pSpecInfo->spectrum;
  rc=ERROR_ID_NO;
  found=0;

  if (specFp==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;
  else if ((recordNo<=0) || (recordNo>pSpecInfo->recordNumber))
   rc=ERROR_ID_FILE_END;
  else
   {
    // Set file pointer

    fseek(specFp,(LONG)pSpecInfo->recordIndexes[recordNo-1],SEEK_SET);
    nCurve=(ULONG)(pSpecInfo->recordIndexes[recordNo]-pSpecInfo->recordIndexes[recordNo-1])/recordSize;

    // Browse spectra in the current record

    for (j=0;(j<(int)nCurve) && !rc && !found;j++)
     {
      if (!fread(&CurveHeader,sizeof(CURVE_HEADER_RET),1,specFp) ||
          !fread(spectrum,sizeof(double)*NDET,1,specFp))

       rc=ERROR_ID_FILE_END;

      else if ((CurveHeader.NScans>0) && (pSpecInfo->project.instrumental.user==(INT)CurveHeader.Type))
       {
        found=1;

        // Data on the current spectrum

        pSpecInfo->TDet     = pdaUlbFileHeader.Temperature;
        pSpecInfo->Tint     = CurveHeader.Tint;
        pSpecInfo->NSomme   = CurveHeader.NScans;
        pSpecInfo->Zm       = (double)-1.;
        pSpecInfo->Azimuth  = (double)-1;
        pSpecInfo->SkyObs   = CurveHeader.Type;       // Use sky observation field for type of measurement safe keeping
        pSpecInfo->rejected = CurveHeader.Rejected;
        pSpecInfo->ReguTemp = CurveHeader.ReguTemp;

        pSpecInfo->present_day.da_day=pdaUlbFileHeader.Day.day;
        pSpecInfo->present_day.da_mon=pdaUlbFileHeader.Day.month;
        pSpecInfo->present_day.da_year=pdaUlbFileHeader.Day.year;

        pSpecInfo->present_time.ti_sec=CurveHeader.HDeb.sec;
        pSpecInfo->present_time.ti_min=CurveHeader.HDeb.min;
        pSpecInfo->present_time.ti_hour=CurveHeader.HDeb.hour;

        pSpecInfo->Tm=(double)ZEN_NbSec(&pSpecInfo->present_day,&pSpecInfo->present_time,0);
        pSpecInfo->TotalExpTime=pdaUlbFileHeader.TotalExpTime;
        pSpecInfo->TimeDec=(double)CurveHeader.HDeb.hour+CurveHeader.HDeb.min/60.;

        tmLocal=pSpecInfo->Tm+THRD_localShift*3600.;

        pSpecInfo->localCalDay=ZEN_FNCaljda(&tmLocal);
        pSpecInfo->localTimeDec=fmod(pSpecInfo->TimeDec+24.+THRD_localShift,(double)24.);

        // Bad pixels elimination

        for (i=0;i<RETBADPIXEL;i++)
         spectrum[i]=spectrum[RETBADPIXEL];

        // Verify bounds

        limSup=(INT)((CurveHeader.Type==PRJCT_INSTR_ULB_TYPE_HIGH)?pdaUlbFileHeader.BS2:pdaUlbFileHeader.BS1);

        for (k=2,nb=(INT)(NDET/limSup+0.5);k<=nb;k++)
         for (i=j=(k-1)*limSup;i<k*limSup;i++,j--)
          spectrum[i]=spectrum[j-1];

        for (i=j=nb*limSup;i<NDET;i++,j--)
         spectrum[i]=spectrum[j-1];

        // Dark current correction

        if (pSpecInfo->ccd.drk.matrix!=NULL)

          for (i=0,tmpMatrix=(double **)pSpecInfo->ccd.drk.matrix;i<NDET;i++)
            spectrum[i]-=(tmpMatrix[1][i]+tmpMatrix[0][i]*pSpecInfo->Tint)*pSpecInfo->NSomme;

        // Interpixel Variability

        if (pSpecInfo->ccd.vip.matrix!=NULL)
         {
          for (i=0,tmpVector=(double *)pSpecInfo->ccd.vip.matrix[0];i<NDET;i++)
           if (tmpVector[i]==(double)0.)
           	rc=ERROR_SetLast("ReliPDA_EGG_Ulb",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"Interpixel Variability");
           else
            spectrum[i]/=tmpVector[i]*pSpecInfo->NSomme;
         }

        // Detector not linearity correction

        if (pSpecInfo->ccd.dnl.matrix!=NULL)
         {
          for (i=0;(i<NDET) && !rc;i++)

           if (pSpecInfo->ccd.dnl.matrix[0][i]==(double)0.)
            rc=ERROR_SetLast("ReliPDA_EGG_Ulb",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"non linearity of the detector");
           else
            spectrum[i]/=pSpecInfo->ccd.dnl.matrix[0][i];
         }

        if (!rc)
         VECTOR_Invert(spectrum,NDET);
       }
     }
   }

  if (!found)
   rc=ERROR_ID_FILE_RECORD;

  // Return

  return rc;
 }

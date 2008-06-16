
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  Read spectra from measurements performed by the University of Toronto
//  Name of module    :  UofT-read.c
//  Creation date     :  12 November 2003
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
//  This module has been written according to the format description provided by
//  Elham FARAHANI, Department of Physics, University of Toronto.  Two formats
//  are available : CSV with several spectra in the same file and another one
//  with the individual spectra distributed in several files.  In the latter
//  case, the following rules are applied :
//
//  - spectra numbers for one day go from 00 01 02 ... 09 0A 0B ... 0Z  to
//    Z0 Z1 ... Z9 ZA ZB ... ZZ e.g. 36x36 records maximum for one day;
//
//  - all individual spectra files of a same day are considered as a one large
//    file (in order to make a daily reference search easier);
//
//  - so, to insert files in the project tree, only one file per day is enough;
//  - for the use of the option "insert path" from the raw spectrum node of the
//    project tree, only the first number of daily records is listed;
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetUofT - calculate the number of spectra measured the current day;
//  UofTReadRecord - read a record in the case of individual spectra in several files;
//  UofTSkipCsvRecord - skip records in a sequential CSV ASCII file;
//  UofTReadRecordCsv - read out a record in the case of CSV format (several spectra in the same file);
//  ReliUofT - read a record in the UofT format.
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"

#define MAX_UOFT_RECORD (UOFT_BASE+(UOFT_BASE-1))                               // the maximum number of records for one day of measurement

// ========================
// DEFINITION OF STRUCTURES
// ========================

// Description of the format (as provided by Elham Faharani, University of Toronto)

typedef struct uoft_format
 {
  SHORT_DATE startDate;                                                         // start Date
  struct time startTime;                                                        // start Time
  SHORT_DATE finishDate;                                                        // finish Date
  struct time finishTime;                                                       // finish Time
  SHORT_DATE meanDate;                                                          // mean date derived from both start and finish dates
  struct time meanTime;                                                         // mean time derived from both start and finish times
  float startElev;                                                              // starting solar elevation angle
  float finishElev;                                                             // finish solar elevation angle
  float meanElev;                                                               // mean solar elevation angle derived from both starting and finish solar elevation angles
  float shutter;                                                                // shutter
  float numCounts;                                                              // ideal Num of Counts
  float slitWidth;                                                              // slit Width
  float groove;                                                                 // groove Density
  float turret;                                                                 // turret Position
  float blazeWve;                                                               // blaze wavelength
  float centerWve;                                                              // centre wavelength
  float intTime;                                                                // integration Time
  float numAcc;                                                                 // num Accumulations
  float minCCDT;                                                                // min CCD temperature
  float maxCCDT;                                                                // max TCCD temperature
  float meanCCDT;                                                               // mean CCD temperature
  float meanBoxT;                                                               // mean box temperature
 }
UOFT_FORMAT;

// ===================
// GLOBAL DECLARATIONS
// ===================

unsigned char UOFT_figures[UOFT_BASE+1]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// ===================
// STATIC DECLARATIONS
// ===================

static UOFT_FORMAT UofT_data;                                                   // data related to the current record
static INT UofT_csvFlag;                                                        // 1 for raw spectra in the CSV format, 0 for corrected spectra
static INDEX UofT_csvLastRecord=ITEM_NONE;                                      // index of last record in the CSV format
static INT UofT_recordList[MAX_UOFT_RECORD];                                    // the list of records

// =========
// FUNCTIONS
// =========

// -----------------------------------------------------------------------------
// FUNCTION      SetUofT
// -----------------------------------------------------------------------------
// PURPOSE       calculate the number of spectra measured the current day
//               according to the file format : several spectra in one file or
//               individual spectra in several files
//
// INPUT         pEngineContext : information on the file to read
//               specFp    : pointer to the current spectra file
//
// OUTPUT        pEngineContext->recordNumber, the number of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//               ERROR_ID_FILE_BAD_FORMAT if the format of the file is unknown;
//               ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC SetUofT(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  unsigned char  fileFilter[MAX_PATH_LEN+1],                                            // file filter
         fileLine[MAX_STR_LEN+1],                                               // file line
        *ptr,*ptr2,*ptr3,                                                       // pointers to parts of strings
         tmp1[3],tmp2[5];                                                       // temporary part of the file name
  int    uofTrecordNo,uofTdayNumber,uofTYear;                                   // resp. the record number, the day number and the year of the spectrum

  #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
  WIN32_FIND_DATA fileInfo;                                                     // structure returned by FindFirstFile and FindNextFile APIs
  HANDLE hDir;                                                                  // handle to use with by FindFirstFile and FindNextFile APIs
  #else
  struct dirent *fileInfo;
  DIR *hDir;
  unsigned char tmpFile[MAX_STR_LEN+1];
  #endif
  int recordNumber;                                                             // record number retrieved from the file name
  RC rc;                                                                        // return code

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("SetUofT",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  hDir=NULL;
  memset(&UofT_data,0,sizeof(UOFT_FORMAT));                                     // information on the current record
  memset(fileFilter,0,MAX_PATH_LEN+1);                                          // file filter
  pEngineContext->recordNumber=0;                                                    // number of records for the whole day
  pEngineContext->lastRefRecord=0;                                                         // last reference record
  UofT_csvLastRecord=ITEM_NONE;                                                 // in CSV format (several records in ASCII) : index of the last record
  UofT_csvFlag=0;                                                               // flag : 1 if CSV format is used
  rc=ERROR_ID_NO;

  strncpy(fileFilter,pEngineContext->fileInfo.fileName,MAX_PATH_LEN);                         // copy the file name into the file filter
  ptr2=strrchr(fileFilter,'.');                                                 // get the file extension (day number)
  ptr=strrchr(fileFilter,PATH_SEP);                                             // search for the path separator

  if ((ptr2==NULL) || (strlen(ptr2)!=4))
   rc=ERROR_SetLast("SetUofT",ERROR_TYPE_WARNING,ERROR_ID_FILE_BAD_FORMAT,pEngineContext->fileInfo.fileName);

  // CSV FORMAT

  else if (!STD_Stricmp(ptr2,".csv"))                                               // the extension of the file is CSV
   {
    // Set flag

    UofT_csvFlag=1;

    // Can't open file

    if (specFp==NULL)
     rc=ERROR_SetLast("SetUofT",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
    else
     {
      // Don't use the first line with the description of records headers

      fseek(specFp,0L,SEEK_SET);
      fgets(fileLine,MAX_STR_LEN,specFp);

      // Browse lines in the file

      do
       {
        // Header of the record

        if (!fgets(fileLine,MAX_STR_LEN,specFp)) break;

        // Spectrum data

        for (recordNumber=0;recordNumber<NDET;recordNumber++)
         if (!fgets(fileLine,MAX_STR_LEN,specFp))
          break;

        // Increment the number of records

        if (recordNumber==NDET)
         pEngineContext->recordNumber++;
       }
      while (!feof(specFp));
     }
   }

  // CORRECTED FORMAT

  else if (ptr!=NULL)
   {
   	*ptr++='\0';                                                                // end filter to the file path

   	#if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_

   	// Search for individual files with spectrum measured the same day

    for (hDir=FindFirstFile(fileFilter,&fileInfo),rc=1;
        (hDir!=INVALID_HANDLE_VALUE) && (rc!=0) && (pEngineContext->recordNumber<MAX_UOFT_RECORD);rc=FindNextFile(hDir,&fileInfo))
     {
      if (((fileInfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==0) &&          // this is not a directory
          ((ptr3=strrchr(fileInfo.cFileName,'.'))!=NULL) &&                     // there is a file extension
           !strnicmp((unsigned char *)fileInfo.cFileName,ptr,6) &&                      // the 6 first characters should be the same
           (strlen(ptr3)==strlen(ptr2)) && !STD_Stricmp(ptr3,ptr2))             // the file extension is the same as the filter one
       {
       	// Get the date of measurement from the file name

        sscanf(fileInfo.cFileName,"%2c%02d%4c.%03d",tmp1,&uofTYear,tmp2,&uofTdayNumber);
        uofTrecordNo=((unsigned char *)strchr(UOFT_figures,tmp2[2])-UOFT_figures)*UOFT_BASE+((unsigned char *)strchr(UOFT_figures,tmp2[3])-UOFT_figures);

        // Attribute a record number and calculate the total number of records

        for (recordNumber=pEngineContext->recordNumber;(recordNumber>0) && (UofT_recordList[recordNumber-1]>uofTrecordNo);recordNumber--)
         UofT_recordList[recordNumber]=UofT_recordList[recordNumber-1];

        UofT_recordList[recordNumber]=uofTrecordNo;
        pEngineContext->recordNumber++;
       }
     }

    // Close handle

    if (hDir!=NULL)
     FindClose(hDir);

   	#else

    // Search for individual files with spectrum measured the same day

    for (hDir=opendir(fileFilter);                                              // get the list of files in the directory
        (hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL) &&                     // get the next file in the directory
        (pEngineContext->recordNumber<MAX_UOFT_RECORD);)
     {
     	// Build the complete file name

     	sprintf(tmpFile,"%s%c%s",fileFilter,PATH_SEP,fileInfo->d_name);

     	// Verify the name of the file

     	if (!STD_IsDir(tmpFile) &&                                                // this is not a directory
         ((ptr3=strrchr(fileInfo->d_name,'.'))!=NULL) &&                        // there is a file extension
          !strnicmp(fileInfo->d_name,ptr,6) &&                                  // the 6 first characters should be the same
          (strlen(ptr3)==strlen(ptr2)) && !STD_Stricmp(ptr3,ptr2))              // the file extension is the same as the filter one
       {
       	// Get the date of measurement from the file name

        sscanf(fileInfo->d_name,"%2c%02d%4c.%03d",tmp1,&uofTYear,tmp2,&uofTdayNumber);
        uofTrecordNo=((unsigned char *)strchr(UOFT_figures,tmp2[2])-UOFT_figures)*UOFT_BASE+((unsigned char *)strchr(UOFT_figures,tmp2[3])-UOFT_figures);

        // Attribute a record number and calculate the total number of records

        for (recordNumber=pEngineContext->recordNumber;(recordNumber>0) && (UofT_recordList[recordNumber-1]>uofTrecordNo);recordNumber--)
         UofT_recordList[recordNumber]=UofT_recordList[recordNumber-1];

        UofT_recordList[recordNumber]=uofTrecordNo;
        pEngineContext->recordNumber++;
       }
     }

    // Close handle

    if (hDir!=NULL)
     closedir(hDir);

    #endif
   }

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("SetUofT",rc);
  #endif

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      UofTReadRecord
// -----------------------------------------------------------------------------
// PURPOSE       read a record in the case of individual spectra in several files
//
// INPUT         originalFileName  original file name
//               recordNo          the record number to read
//
// OUTPUT        spe               the spectrum
//               recordName        the record name
//
// RETURN        ERROR_ID_FILE_RECORD     problem while reading the record
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC UofTReadRecord(unsigned char *originalFileName,INT recordNo,double *spe,unsigned char *recordName)
 {
  // Declarations

  unsigned char  fileName[MAX_PATH_LEN+1],                                              // name of the current file (the current record)
        *ptr,*ptr2;                                                             // pointers to parts in the previous string
  unsigned char  fileLine[MAX_STR_LEN+1],                                               // line of file
         strBuffer[MAX_STR_LEN+1];                                              // string buffer

  FILE  *fp;                                                                    // pointer to the current file
  int    Dm,Mm,Ym,Ds,Ms,Ys,Df,Mf,Yf,                                            // fields of resp. mean, starting and ending dates
         hm,mm,sm,hs,ms,ss,hf,mf,sf;                                            // fields of resp. mean, starting and ending times
  INDEX  i;                                                                     // browse pixels in the spectrum
  RC     rc;                                                                    // return code

  // Initializations

  memset(fileName,0,MAX_PATH_LEN+1);
  memset(fileLine,0,MAX_STR_LEN+1);
  memset(strBuffer,0,MAX_STR_LEN+1);

  rc=ERROR_ID_NO;

  // Build the file name using the record number

  strncpy(fileName,originalFileName,MAX_PATH_LEN);

  if ((ptr=strrchr(fileName,PATH_SEP))==NULL)
   ptr=fileName+6;
  else
   ptr+=7;

  ptr2=strrchr(originalFileName,'.');                                           // get the file extension

  sprintf(ptr,"%c%c%s",
          UOFT_figures[(int)(UofT_recordList[(recordNo-1)]/UOFT_BASE)],
          UOFT_figures[(int)(UofT_recordList[(recordNo-1)]%UOFT_BASE)],ptr2);

  ptr=strrchr(fileName,PATH_SEP);
  if (ptr==NULL)
   strncpy(recordName,fileName,20);
  else
   strncpy(recordName,ptr+1,20);

  // Open file

  if ((fp=fopen(fileName,"rt"))==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;

  // Read all information on the current measurement

  else if (!STD_FileLength(fp) ||                                               // file length is zero
           !fgets(fileLine,MAX_STR_LEN,fp) ||
           !fgets(fileLine,MAX_STR_LEN,fp) ||
          (sscanf(fileLine,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                            &Dm,&Mm,&Ym,&hm,&mm,&sm,&Ds,&Ms,&Ys,&hs,&ms,&ss,&Df,&Mf,&Yf,&hf,&mf,&sf)!=18) ||
           !fgets(fileLine,MAX_STR_LEN,fp) || (sscanf(fileLine,"%f %f %f",&UofT_data.meanElev,&UofT_data.startElev,&UofT_data.finishElev)!=3) ||
           !fgets(fileLine,MAX_STR_LEN,fp) ||
          (sscanf(fileLine,"%f %f %f %f %f %f %f %f %f %f %f %f %f ",
                 &UofT_data.shutter,                                            // Shutter
                 &UofT_data.numCounts,                                          // Ideal Num of Counts
                 &UofT_data.slitWidth,                                          // Slit Width
                 &UofT_data.groove,                                             // Groove Density
                 &UofT_data.turret,                                             // Turret Position
                 &UofT_data.blazeWve,                                           // Blaze wavelength
                 &UofT_data.centerWve,                                          // Centre wavelength
                 &UofT_data.intTime,                                            // Integration Time
                 &UofT_data.numAcc,                                             // Num Accumulations
                 &UofT_data.meanCCDT,                                           // Mean CCD temperature
                 &UofT_data.minCCDT,                                            // Min CCD temperature
                 &UofT_data.maxCCDT,                                            // Max TCCD temperature
                 &UofT_data.meanBoxT)!=13))                                     // Mean box temperature

   rc=ERROR_SetLast("UofTReadRecord",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,originalFileName);
  else
   {
    // Spectrum read out

    for (i=0;(i<NDET) && fgets(fileLine,MAX_STR_LEN,fp) && sscanf(fileLine,"%lf",&spe[i]);i++);

    if (i<NDET)
     rc=ERROR_SetLast("UofTReadRecord",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,originalFileName);
    else
     {
      // Get date and time information (avoid problem of type conversion)

      UofT_data.startDate.da_day=(char)Ds;                                      // start Date (day)
      UofT_data.startDate.da_mon=(char)Ms;                                      // start Date (month)
      UofT_data.startDate.da_year=(short)Ys;                                    // start Date (year)

      UofT_data.startTime.ti_hour=(char)hs;                                     // start Time (hour)
      UofT_data.startTime.ti_min=(char)ms;                                      // start Time (min)
      UofT_data.startTime.ti_sec=(char)ss;                                      // start Time (sec)

      UofT_data.finishDate.da_day=(char)Df;                                     // finish Date (day)
      UofT_data.finishDate.da_mon=(char)Mf;                                     // finish Date (month)
      UofT_data.finishDate.da_year=(short)Yf;                                   // finish Date (year)

      UofT_data.finishTime.ti_hour=(char)hf;                                    // finish Time (hour)
      UofT_data.finishTime.ti_min=(char)mf;                                     // finish Time (min)
      UofT_data.finishTime.ti_sec=(char)sf;                                     // finish Time (sec)

      UofT_data.meanDate.da_day=(char)Dm;                                       // mean Date (day)
      UofT_data.meanDate.da_mon=(char)Mm;                                       // mean Date (month)
      UofT_data.meanDate.da_year=(short)Ym;                                     // mean Date (year)

      UofT_data.meanTime.ti_hour=(char)hm;                                      // mean Time (hour)
      UofT_data.meanTime.ti_min=(char)mm;                                       // mean Time (min)
      UofT_data.meanTime.ti_sec=(char)sm;                                       // mean Time (sec)
     }
   }

  // Close file

  if (fp!=NULL)
   fclose(fp);

  // Return

  if (rc)
   rc=ERROR_ID_FILE_RECORD;

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      UofTSkipCsvRecord
// -----------------------------------------------------------------------------
// PURPOSE       skip records in a sequential CSV ASCII file
//
// INPUT         specFp pointer to the current CSV file
//               nSkip  number of records to skip
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//               ERROR_ID_FILE_END if the end of the file is reached
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC UofTSkipCsvRecord(FILE *specFp,int nSkip)
 {
  // Declarations

  unsigned char fileLine[MAX_STR_LEN+1];                                                // line of the input file
  int recordCount,recordNumber;                                                 // resp. browse records to skip and lines in the file
  RC rc;                                                                        // record number

  // Initializations

  rc=ERROR_ID_NO;

  // Check file pointer

  if (specFp==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;
  else
   {
    // Go to the beginning of the file

    fseek(specFp,0L,SEEK_SET);
    fgets(fileLine,MAX_STR_LEN,specFp);

    // Browse records to skip

    for (recordCount=0;(recordCount<nSkip) && !feof(specFp);recordCount++)
     {
      // Header of the record

      if (!fgets(fileLine,MAX_STR_LEN,specFp)) break;

      // Spectrum data

      for (recordNumber=0;recordNumber<NDET;recordNumber++)
       if (!fgets(fileLine,MAX_STR_LEN,specFp))
        break;
     }

    if (recordCount<nSkip)
     rc=ERROR_ID_FILE_END;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      UofTReadRecordCsv
// -----------------------------------------------------------------------------
// PURPOSE       read out a record in the case of CSV format (several spectra in the same file)
//
// INPUT         pEngineContext   information on the current file
//               specFp      pointer to the current CSV file
//               recordNo    the index of the record to read
//
// OUTPUT        spe         the spectrum;
//
// RETURN        ERROR_ID_FILE_END     the end of the file is reached
//               ERROR_ID_NO           otherwise.
// -----------------------------------------------------------------------------

RC UofTReadRecordCsv(ENGINE_CONTEXT *pEngineContext,FILE *specFp,INT recordNo,double *spe)
 {
  // Declarations

  unsigned char  fileLine[MAX_STR_LEN+1],                                               // line of file
         strBuffer[MAX_STR_LEN+1];                                              // string buffer

  int    Dm,Mm,Ym,Ds,Ms,Ys,Df,Mf,Yf,                                            // fields of resp. mean, starting and ending dates
         hm,mm,sm,hs,ms,ss,hf,mf,sf,                                            // fields of resp. mean, starting and ending times
         cday;                                                                  // calendar day

  double tm1,tm2,tmm,decHours;                                                  // working variables to calculate the mean time
  INDEX  i;                                                                     // browse pixels in the spectrum
  RC     rc;                                                                    // return code

  // Initializations

  memset(fileLine,0,MAX_STR_LEN+1);
  memset(strBuffer,0,MAX_STR_LEN+1);

  rc=ERROR_ID_NO;

  // Can't open file

  if (specFp==NULL)
   rc=ERROR_SetLast("UofTReadRecordCsv",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);

  // Goto the specified record

  else if ((recordNo-UofT_csvLastRecord==1) || !(rc=UofTSkipCsvRecord(specFp,recordNo-1)))
   {
    UofT_csvLastRecord=recordNo;

    // Read all information about the current measurement

    if (!fgets(fileLine,MAX_STR_LEN,specFp) ||
        (sscanf(fileLine,"\"%d/%d/%d\",\"%d:%d:%d\",\"%d/%d/%d\",\"%d:%d:%d\",%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g",
                         &Ds,&Ms,&Ys,&hs,&ms,&ss,&Df,&Mf,&Yf,&hf,&mf,&sf,
                         &UofT_data.shutter,                                    // shutter
                         &UofT_data.numCounts,                                  // ideal Num of Counts
                         &UofT_data.slitWidth,                                  // slit Width
                         &UofT_data.groove,                                     // groove Density
                         &UofT_data.turret,                                     // turret Position
                         &UofT_data.blazeWve,                                   // blaze wavelength
                         &UofT_data.centerWve,                                  // centre wavelength
                         &UofT_data.intTime,                                    // integration Time
                         &UofT_data.numAcc,                                     // num Accumulations
                         &UofT_data.meanCCDT,                                   // mean CCD temperature
                         &UofT_data.minCCDT,                                    // min CCD temperature
                         &UofT_data.maxCCDT,                                    // max TCCD temperature
                         &UofT_data.meanBoxT)!=25))                             // mean box temperature

     rc=ERROR_ID_FILE_END;

    else
     {
      // Spectrum read out

      for (i=0;(i<NDET) && fgets(fileLine,MAX_STR_LEN,specFp) && sscanf(fileLine,"%lf",&spe[i]);i++);
      VECTOR_Invert(spe,NDET);

      if (i<NDET)
       rc=ERROR_ID_FILE_END;
      else
       {
        // Get date and time information

        UofT_data.startDate.da_day=(char)Ds;                                    // Start Date (day)
        UofT_data.startDate.da_mon=(char)Ms;                                    // Start Date (month)
        UofT_data.startDate.da_year=(Ys<50)?(short)(Ys+2000):(short)(Ys+1900);  // Start Date (year)

        UofT_data.startTime.ti_hour=(char)hs;                                   // Start Time (hour)
        UofT_data.startTime.ti_min=(char)ms;                                    // Start Time (min)
        UofT_data.startTime.ti_sec=(char)ss;                                    // Start Time (sec)

        UofT_data.finishDate.da_day=(char)Df;                                   // Finish Date (day)
        UofT_data.finishDate.da_mon=(char)Mf;                                   // Finish Date (month)
        UofT_data.finishDate.da_year=(Yf<50)?(short)(Yf+2000):(short)(Yf+1900); // Finish Date (year)

        UofT_data.finishTime.ti_hour=(char)hf;                                  // Finish Time (hour)
        UofT_data.finishTime.ti_min=(char)mf;                                   // Finish Time (min)
        UofT_data.finishTime.ti_sec=(char)sf;                                   // Finish Time (sec)

        // Calculate the measurement mean time

        tm1=(double)ZEN_NbSec(&UofT_data.startDate,&UofT_data.startTime,0);     // get the number of seconds at the beginning of the measurement
        tm2=(double)ZEN_NbSec(&UofT_data.finishDate,&UofT_data.finishTime,0);   // get the number of seconds at the end of the measurement

        tmm=(tm1+tm2)*0.5;                                                      // calculate the number of seconds at the middle of the measurement

        Ym=ZEN_FNCaljye(&tmm);                                                  // year
        cday=ZEN_FNCaljda(&tmm);                                                // calendar day
        Mm=ZEN_FNCaljmon(Ym,cday);                                              // month
        Dm=ZEN_FNCaljday(Ym,cday);                                              // day in month

        decHours=ZEN_FNCaldti(&tmm);                                            // decimal hours

        hm=(int)floor(decHours);                                                // hours
        mm=(int)floor((decHours-(double)hm)*60.);                               // min
        sm=(int)floor(((decHours-(double)hm)*60.-(double)mm)*60.+0.5);               // sec

        UofT_data.meanDate.da_day=(char)Dm;                                     // Mean Date (day)
        UofT_data.meanDate.da_mon=(char)Mm;                                     // Mean Date (month)
        UofT_data.meanDate.da_year=(short)Ym;                                   // Mean Date (year)

        UofT_data.meanTime.ti_hour=(char)hm;                                    // Mean Time (hour)
        UofT_data.meanTime.ti_min=(char)mm;                                     // Mean Time (min)
        UofT_data.meanTime.ti_sec=(char)sm;                                     // Mean Time (sec)
       }
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliUofT
// -----------------------------------------------------------------------------
// PURPOSE       Read a record in the UofT format
//
// INPUT         pEngineContext : information on the file to read
//               recordNo  : the index of the record to read
//               dateFlag  : 1 to search for a reference spectrum
//               localDay  : if dateFlag is 1, the calendar day for the
//                           reference spectrum to search for
//               specFp    : pointer to the spectra file
//
// OUTPUT        information on the read out record
//
// RETURN        ERROR_ID_FILE_END if the end of the file is reached;
//               ERROR_ID_FILE_RECORD if the record doesn't satisfy criteria
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC ReliUofT(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,INT localDay,FILE *specFp)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  RC      rc;                                                                   // return code
  double  tmLocal;                                                              // local time

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  rc=ERROR_ID_NO;

  // Read the record

  if ((recordNo>0) && (recordNo<=pEngineContext->recordNumber) &&
     !(rc=(UofT_csvFlag)?UofTReadRecordCsv(pEngineContext,specFp,recordNo,pEngineContext->buffers.spectrum):
                         UofTReadRecord(pEngineContext->fileInfo.fileName,recordNo,pEngineContext->buffers.spectrum,pRecord->Nom)))
   {
    memcpy(&pRecord->present_day,&UofT_data.meanDate,sizeof(SHORT_DATE));
    memcpy(&pRecord->present_time,&UofT_data.meanTime,sizeof(struct time));

    // Get information on the current record

    pRecord->NSomme=(int)UofT_data.numAcc;                                    // number of accumulations
    pRecord->Tint=(double)UofT_data.intTime*0.001;                            // integration time
    pRecord->Zm=(UofT_csvFlag)?-1.:(double)90.-UofT_data.meanElev;            // solar zenith angle
    pRecord->ReguTemp=UofT_data.meanBoxT;                                     // box temperature
    pRecord->TDet=(double)UofT_data.meanCCDT;                                 // detector temperature

    pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);
    pRecord->TotalExpTime=(double)0.;
    pRecord->TimeDec=(double)pRecord->present_time.ti_hour+pRecord->present_time.ti_min/60.+pRecord->present_time.ti_sec/3600.;

    // Determine the local time

    tmLocal=pRecord->Tm+THRD_localShift*3600.;

    pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
    pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

    // Search for a reference spectrum

    if (dateFlag && (pRecord->localCalDay>localDay))
     rc=ERROR_ID_FILE_END;

    else if ((pRecord->NSomme<=0) ||
             (dateFlag && (pRecord->localCalDay!=localDay)))

     rc=ERROR_ID_FILE_RECORD;

    else if (dateFlag)
     pEngineContext->lastRefRecord=recordNo;
   }

  // Return

  return rc;
 }


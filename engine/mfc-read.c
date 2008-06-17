
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  MFC read out routines
//  Name of module    :  MFC-READ.C
//  Creation date     :  24 April 99
//  Ref               :  MFC program (see IUP Heidelberg)
//
//  Authors           :  Heidelberg
//  Adaptations       :  Caroline FAYT
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
//  This module contains routines needed for reading out data in the MFC format
//  used at IUP Heidelberg.  Two formats are described in this package :
//
//        PRJCT_INSTR_FORMAT_MFC, the binary format;
//        PRJCT_INSTR_FORMAT_MFC_STD, the ASCII format;
//
//  See MFC references for further description of the format.
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetMFC - calculate the number of files in a directory in MFC format;
//
//  MFC_ReadRecord - record read out and processing in binary format;
//
//  ReliMFC - MFC binary format read out;
//
//  ReliMFCStd - MFC ASCII format read out;
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"

// ================
// GLOBAL VARIABLES
// ================

UCHAR MFC_fileInstr[MAX_STR_SHORT_LEN+1],      // instrumental function file name
      MFC_fileDark[MAX_STR_SHORT_LEN+1],       // dark current file name
      MFC_fileOffset[MAX_STR_SHORT_LEN+1];     // offset file name

INT   MFC_refFlag=0;

INT mfcLastSpectrum=0;

// -----------------------------------------------------------------------------
// FUNCTION      SetMFC
// -----------------------------------------------------------------------------
// PURPOSE       calculate the number of files in a directory in MFC format
//
// INPUT         specFp      pointer to the spectra file
//
// OUTPUT        pEngineContext   pointer to a structure whose some fields are filled
//                           with general data on the file
//
// RETURN        ERROR_ID_NO  no error;
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC SetMFC(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  PRJCT_INSTRUMENTAL *pInstrumental;
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  UCHAR  fileName[MAX_STR_SHORT_LEN+1],                                         // name of the current file
         format[20],                                                            // format string
        *ptr,*ptr2,*ptr3;                                                       // pointers to parts in the previous string
//  INDEX  indexFile;
    INDEX firstFile,lastFile;                                                   // indexes for browsing files (see file names format)
//  FILE  *fp;                                                                    // pointer to the current file
  RC rc;

  // Initializations

  pBuffers=&pEngineContext->buffers;

  memset(fileName,0,MAX_STR_SHORT_LEN+1);
  strncpy(fileName,pEngineContext->fileInfo.fileName,MAX_STR_SHORT_LEN);
  pInstrumental=&pEngineContext->project.instrumental;
  pEngineContext->lastRefRecord=0;
  mfcLastSpectrum=0;
  rc=ERROR_ID_NO;

  if ((THRD_isFolder || !THRD_treeCallFlag) && ((ptr=strrchr(fileName,PATH_SEP))!=NULL))
   {
   	if (strlen(PATH_fileMax))
   	 {
//    ptr+=2;         // get the number of the first file

      if ((ptr2=strrchr(ptr,'.'))==NULL)
       ptr2=strrchr(ptr,'\0');

      for (ptr3=ptr2;(ptr3>ptr+1) && isdigit(*(ptr3-1));ptr3--);

      sprintf(format,"%%0%dd%%s",ptr2-ptr3);

      sscanf(ptr3,"%d",&firstFile);
      sscanf(PATH_fileMax+(ptr3-ptr-1),"%d",&lastFile);

      pEngineContext->recordNumber=(lastFile-firstFile+1);

      // Refresh dark current and offset

      if (strlen(pInstrumental->dnlFile) &&                                       // dnl file specified
         !strrchr(pInstrumental->dnlFile,PATH_SEP) &&                             // no path separator
         (pBuffers->dnl!=NULL))                                                  // allocated vector
       {
       	strcpy(fileName,pEngineContext->fileInfo.fileName);
       	if ((ptr=strrchr(fileName,PATH_SEP))!=NULL)
       	 {
       	 	FILES_RebuildFileName(ptr+1,pInstrumental->dnlFile,0);

       	 	if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
           MFC_ReadRecord(fileName,
                         &MFC_headerOff,pBuffers->dnl,
                         &MFC_headerDrk,NULL,
                         &MFC_headerOff,NULL,
                          pInstrumental->mfcMaskOffset,pInstrumental->mfcMaskSpec,pInstrumental->mfcRevert);
       	 	else
           MFC_ReadRecordStd(pEngineContext,fileName,&MFC_headerOff,pBuffers->dnl,
                            &MFC_headerDrk,NULL,
                            &MFC_headerOff,NULL);
         }
       }

      if (strlen(pInstrumental->vipFile) &&                      // vip file specified
         !strrchr(pInstrumental->vipFile,PATH_SEP) &&            // no path separator
         (pBuffers->varPix!=NULL))                                               // allocated vector
       {
       	strcpy(fileName,pEngineContext->fileInfo.fileName);

       	if ((ptr=strrchr(fileName,PATH_SEP))!=NULL)
       	 {
       	  FILES_RebuildFileName(ptr+1,pInstrumental->vipFile,0);

       	 	if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
           MFC_ReadRecord(fileName,
                         &MFC_headerOff,pBuffers->dnl,
                         &MFC_headerDrk,NULL,
                         &MFC_headerOff,NULL,
                          pInstrumental->mfcMaskDark,pInstrumental->mfcMaskSpec,pInstrumental->mfcRevert);
       	 	else
           MFC_ReadRecordStd(pEngineContext,fileName,&MFC_headerDrk,pBuffers->varPix,
                            &MFC_headerDrk,NULL,
                            &MFC_headerOff,pBuffers->dnl);
         }
       }

      // Search for the last file

/*      for (indexFile=pInstrumental->mfcMaxSpectra+firstFile-1;indexFile>firstFile;indexFile--)
       {
        // Build the right file name

        sprintf(ptr,format,indexFile,ptr2);

        if ((fp=fopen(fileName,"rb"))!=NULL)
         {
          fclose(fp);
          break;
         }
       }   */

      // Calculate the number of files in the current directory;
      // in this format, there's one record per file and a directory
      // is seen as a set of records.

//      pEngineContext->recordNumber=indexFile-firstFile+1;
//      pEngineContext->recordNumber=pInstrumental->mfcMaxSpectra;
     }
    else
     rc=ERROR_SetLast("SetMFC",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
   }
  else
   pEngineContext->recordNumber=1;

  // Return

  return rc;
 }

// =================
// MFC BINARY FORMAT
// =================

TBinaryMFC MFC_headerDrk,                                                       // header of the dark current file
           MFC_headerOff,                                                       // header of the offset file
           MFC_header,                                                          // header of the spectra file
           MFC_headerInstr;                                                     // header of the instrumental function file

// -----------------------------------------------------------------------------
// FUNCTION      MFC_ReadRecord
// -----------------------------------------------------------------------------
// PURPOSE       record read out and processing in MFC binary format
//
// INPUT         fileName          the name of the current file;
//               pHeaderDrk, drk   dark current data if any;
//               pHeaderOff, off   offset data if any;
//               mask              mask used for spectra selection;
//
// OUTPUT        pHeaderSpe, spe   resp. data on the current record and the spectrum
//                                 to process;
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  the input file can't be found;
//               ERROR_ID_FILE_EMPTY      the file is empty;
//               ERROR_ID_FILE_RECORD     the record doesn't satisfy user constraints;
//               ERROR_ID_ALLOC           buffer allocation error;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC MFC_ReadRecord(UCHAR *fileName,
                  TBinaryMFC *pHeaderSpe,double *spe,
                  TBinaryMFC *pHeaderDrk,double *drk,
                  TBinaryMFC *pHeaderOff,double *off,
                  UINT mask,UINT maskSpec,UINT revertFlag)
 {
  // Declarations

  float *specTmp;   // the original spectrum
  FILE *fp;         // pointer to the current file
  INDEX i;          // browse pixels in the spectrum
  RC rc;            // return code

  // Initializations

  rc=ERROR_ID_NO;
  specTmp=NULL;

  // Open file

  if ((fp=fopen(fileName,"rb"))==NULL)
//   rc=ERROR_ID_FILE_RECORD;
   rc=ERROR_ID_FILE_NOT_FOUND;
  else if (!STD_FileLength(fp))
   rc=ERROR_SetLast("ReadMFCRecord",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);

  // Allocate a buffer for the spectrum

  else if ((specTmp=(float *)MEMORY_AllocBuffer("MFC_ReadRecord ","specTmp",NDET,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    for (i=0;i<NDET;i++)
     specTmp[i]=(float)0.;

    // Complete record read out

    if (!fread(pHeaderSpe,sizeof(TBinaryMFC),1,fp) ||                  // header
       ((mask!=maskSpec) && ((pHeaderSpe->ty&mask)==0) && ((UINT)pHeaderSpe->wavelength1!=mask)) ||                    // spectrum selection
        (pHeaderSpe->no_chan==0) || (pHeaderSpe->no_chan>NDET) ||      // verify the size of the spectrum
        !fread(specTmp,sizeof(float)*pHeaderSpe->no_chan,1,fp))        // spectrum read out
     {
      memset(pHeaderSpe,0,sizeof(TBinaryMFC));
      pHeaderSpe->int_time=(float)0.;
      rc=ERROR_ID_FILE_RECORD;
     }
    else
     {
      // Copy original spectrum to the output buffer

      for (i=0;i<NDET;i++)
       spe[i]=(double)specTmp[i];

      // Offset correction if any

      if ((off!=NULL) && (pHeaderOff->noscans>0) && (THRD_browseType!=THREAD_BROWSE_MFC_OFFSET))
       {
        for (i=0;i<NDET;i++)
         spe[i]-=(double)off[i]*pHeaderSpe->noscans/pHeaderOff->noscans;
       }

      // Dark current correction if any

      if ((drk!=NULL) && (pHeaderDrk->int_time!=(float)0.) && (THRD_browseType!=THREAD_BROWSE_MFC_OFFSET) && (THRD_browseType!=THREAD_BROWSE_MFC_DARK))
       {
        for (i=0;i<NDET;i++)
         spe[i]-=(double)drk[i]*pHeaderSpe->int_time/(pHeaderDrk->int_time*pHeaderDrk->noscans);
       }

      if (revertFlag && (THRD_browseType!=THREAD_BROWSE_MFC_OFFSET) && (THRD_browseType!=THREAD_BROWSE_MFC_DARK))
       VECTOR_Invert(spe,NDET);
     }
   }


  // Close file

  if (fp!=NULL)
   fclose(fp);

  // Release the allocated buffer

  if (specTmp!=NULL)
   MEMORY_ReleaseBuffer("MFC_ReadRecord ","specTmp",specTmp);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliMFC
// -----------------------------------------------------------------------------
// PURPOSE       MFC binary format read out
//
// INPUT         recordNo     index of record in file;
//               dateFlag     0 no date constraint; 1 a date selection is applied;
//               specFp       pointer to the spectra file;
//               mfcMask      mask for spectra selection;
//
// OUTPUT        pEngineContext  : pointer to a structure whose some fields are filled
//                            with data on the current spectrum
//
// RETURN        ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC ReliMFC(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,UINT mfcMask)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  INT                  day,mon,year,hour,min,sec,nsec,                          // date and time fields
                       firstFile;                                               // number of the first file in the current directory
  UCHAR                fileName[MAX_STR_SHORT_LEN+1],                           // name of the current file (the current record)
                       format[20],
                      *ptr,*ptr2;                                               // pointers to parts in the previous string
  SHORT_DATE           today;                                                   // date of the current record
  PRJCT_INSTRUMENTAL  *pInstrumental;                                           // pointer to the instrumental part of the project
  RC                   rc;                                                      // return code
  double tmLocal;

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;

  memset(fileName,0,MAX_STR_SHORT_LEN+1);
  strncpy(fileName,pEngineContext->fileInfo.fileName,MAX_STR_SHORT_LEN);
  pInstrumental=&pEngineContext->project.instrumental;
  rc=ERROR_ID_NO;

  if ((ptr=strrchr(fileName,PATH_SEP))==NULL)
   rc=ERROR_ID_FILE_RECORD;
  else if ((recordNo>0) && (recordNo<=pEngineContext->recordNumber))
   {
    // Build the right file name

//    if (THRD_treeCallFlag)
     {
//      ptr+=2;
//      sscanf(ptr,"%d",&firstFile);
      if ((ptr2=strrchr(ptr,'.'))==NULL)
       ptr2=strrchr(ptr,'\0');

      for (ptr=ptr2;isdigit(*(ptr-1));ptr--);

      sscanf(ptr,"%d",&firstFile);
      sprintf(format,"%%0%dd%%s",ptr2-ptr);
      sprintf(ptr,format,firstFile+recordNo-1,ptr2);
     }

    // Record read out

    if (!(rc=MFC_ReadRecord(fileName,&MFC_header,pBuffers->spectrum,&MFC_headerDrk,pBuffers->varPix,&MFC_headerOff,pBuffers->dnl,mfcMask,pInstrumental->mfcMaskSpec,pInstrumental->mfcRevert)))
     {
      if ((mfcMask==pInstrumental->mfcMaskSpec) &&
         (((pInstrumental->mfcMaskSpec!=(UINT)0) && ((UINT)MFC_header.ty==mfcMask)) ||
		  ((pInstrumental->mfcMaskSpec==(UINT)0) && ((UINT)MFC_header.ty==pInstrumental->mfcMaskSpec))) &&
        (((double)pInstrumental->wavelength>(double)100.) && ((MFC_header.wavelength1<(double)pInstrumental->wavelength-5.) || (MFC_header.wavelength1>(double)pInstrumental->wavelength+5.))))

       rc=ERROR_ID_FILE_RECORD;

      // In automatic file selection, replace instrumental functions with new ones if found

      if ((mfcMask==pInstrumental->mfcMaskSpec) &&
        (((mfcMask!=(UINT)0) && ((UINT)MFC_header.ty!=mfcMask)) || ((mfcMask==(UINT)0) && (rc==ERROR_ID_FILE_RECORD) && ((UINT)MFC_header.wavelength1!=mfcMask))))
       {
        if (pInstrumental->mfcMaskUse)
         {
          if ((((MFC_header.ty&pInstrumental->mfcMaskInstr)!=0) || (MFC_header.wavelength1==pInstrumental->mfcMaskInstr)) && (pBuffers->instrFunction!=NULL))
           {
            MFC_ReadRecord(fileName,
                          &MFC_headerInstr,pBuffers->instrFunction,
                          &MFC_headerDrk,pBuffers->varPix,               // instrument function should be corrected for dark current
                          &MFC_headerOff,pBuffers->dnl,                  // instrument function should be corrected for offset
                          pInstrumental->mfcMaskInstr,pInstrumental->mfcMaskSpec,pInstrumental->mfcRevert);

            FILES_CompactPath(MFC_fileInstr,fileName,1,1);
           }
          else if ((((MFC_header.ty&pInstrumental->mfcMaskDark)!=0) || (MFC_header.wavelength1==pInstrumental->mfcMaskDark)) && (pBuffers->varPix!=NULL))
           {
            MFC_ReadRecord(fileName,
                          &MFC_headerInstr,pBuffers->varPix,
                          &MFC_headerDrk,NULL,                            // no correction for dark current
                          &MFC_headerOff,pBuffers->dnl,                  // dark current should be corrected for offset
                          pInstrumental->mfcMaskDark,pInstrumental->mfcMaskSpec,0);

            FILES_CompactPath(MFC_fileDark,fileName,1,1);
           }
          else if ((((MFC_header.ty&pInstrumental->mfcMaskOffset)!=0) || (MFC_header.wavelength1==pInstrumental->mfcMaskOffset)) &&  (pBuffers->dnl!=NULL))
           {
            MFC_ReadRecord(fileName,
                          &MFC_headerOff,pBuffers->dnl,
                          &MFC_headerDrk,NULL,                            // no correction for dark current
                          &MFC_headerOff,NULL,                            // no correction for offset
                          pInstrumental->mfcMaskOffset,pInstrumental->mfcMaskSpec,0);

            FILES_CompactPath(MFC_fileOffset,fileName,1,1);
           }
         }
        rc=ERROR_ID_FILE_RECORD;
       }
//      else if (MFC_header.noscans<=0)
//       rc=ERROR_ID_FILE_RECORD;
     	// Wavelength selection

      if (!rc)
       {
        memset(PATH_fileSpectra,0,MAX_STR_SHORT_LEN+1);
        strncpy(PATH_fileSpectra,fileName,MAX_STR_SHORT_LEN);

        // Date and time read out

        sscanf(MFC_header.dateAndTime,"%d.%d.%d",&day,&mon,&year);
        sscanf(&MFC_header.dateAndTime[9],"%d:%d:%d",&hour,&min,&sec);

        today.da_day=(UCHAR)day;
        today.da_mon=(UCHAR)mon;
        today.da_year=(SHORT)year;

        if (today.da_year<30)
         today.da_year+=(short)2000;
        else if (today.da_year<130)
         today.da_year+=(short)1900;
        else if (today.da_year<1930)
         today.da_year+=(short)100;

        pRecord->startTime.ti_hour=(UCHAR)hour;
        pRecord->startTime.ti_min=(UCHAR)min;
        pRecord->startTime.ti_sec=(UCHAR)sec;

        sscanf(&MFC_header.dateAndTime[18],"%d:%d:%d",&hour,&min,&sec);

        pRecord->endTime.ti_hour=(UCHAR)hour;
        pRecord->endTime.ti_min=(UCHAR)min;
        pRecord->endTime.ti_sec=(UCHAR)sec;

        // Data on the current spectrum

        nsec=(pRecord->startTime.ti_hour*3600+pRecord->startTime.ti_min*60+pRecord->startTime.ti_sec+
              pRecord->endTime.ti_hour*3600+pRecord->endTime.ti_min*60+pRecord->endTime.ti_sec)/2;

        pRecord->present_day.da_day=(UCHAR)day;
        pRecord->present_day.da_mon=(UCHAR)mon;
        pRecord->present_day.da_year=(SHORT)year;

        if (pRecord->present_day.da_year<30)
         pRecord->present_day.da_year+=(short)2000;
        else if (pRecord->present_day.da_year<130)
         pRecord->present_day.da_year+=(short)1900;
        else if (pRecord->present_day.da_year<1930)
         pRecord->present_day.da_year+=(short)100;

        pRecord->present_time.ti_hour=(UCHAR)(nsec/3600);
        pRecord->present_time.ti_min=(UCHAR)((nsec%3600)/60);
        pRecord->present_time.ti_sec=(UCHAR)((nsec%3600)%60);

        pRecord->TDet     = 0;
        pRecord->Tint     = MFC_header.int_time*0.001;
        pRecord->NSomme   = MFC_header.noscans;

        pRecord->wavelength1=MFC_header.wavelength1;
        memcpy(pRecord->dispersion,MFC_header.dispersion,sizeof(float)*3);
        memcpy(pRecord->Nom,MFC_header.specname,20);

        pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);
        pRecord->TotalExpTime=(double)0.;
        pRecord->TimeDec=(double)pRecord->present_time.ti_hour+pRecord->present_time.ti_min/60.+pRecord->present_time.ti_sec/3600.;

        pRecord->longitude=-MFC_header.longitude;  // !!!
        pRecord->latitude=MFC_header.latitude;
        pRecord->altitude=(double)0.;
        pRecord->elevationViewAngle=(float)MFC_header.elevation;

        MFC_header.longitude=-MFC_header.longitude;

        pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);
        pRecord->Zm=ZEN_FNTdiz(ZEN_FNCrtjul(&pRecord->Tm),&pRecord->longitude,&pRecord->latitude,&pRecord->Azimuth);
        tmLocal=pRecord->Tm+THRD_localShift*3600.;

        pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
        pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

        pRecord->longitude=-MFC_header.longitude;  // !!!

        // User constraints

//        if (dateFlag && (pRecord->localCalDay>localDay))
//         rc=ERROR_ID_FILE_END;

        if (rc || (dateFlag && ((pRecord->localCalDay!=localDay) || (pRecord->elevationViewAngle<88.))))                  // reference spectra are zenith only
           rc=ERROR_ID_FILE_RECORD;
//        else if (pInstrumental->mfcRevert)
//         VECTOR_Invert(pBuffers->spectrum,NDET);
//        else if (dateFlag)
//         pEngineContext->lastRefRecord=recordNo;
       }
     }

    if (rc)
     rc=ERROR_ID_FILE_RECORD;
   }

  // Return

  return rc;
 }

// ================
// MFC ASCII FORMAT
// ================

// -----------------------------------------------------------------------------
// FUNCTION      MFC_ReadRecordStd
// -----------------------------------------------------------------------------
// PURPOSE       record read out and processing in MFC binary format
//
// INPUT         fileName          the name of the current file;
//               pHeaderDrk, drk   dark current data if any;
//               pHeaderOff, off   offset data if any;
//               mask              mask used for spectra selection;
//
// OUTPUT        pHeaderSpe, spe   resp. data on the current record and the spectrum
//                                 to process;
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  the input file can't be found;
//               ERROR_ID_FILE_EMPTY      the file is empty;
//               ERROR_ID_FILE_RECORD     the record doesn't satisfy user constraints;
//               ERROR_ID_ALLOC           buffer allocation error;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC MFC_ReadRecordStd(ENGINE_CONTEXT *pEngineContext,UCHAR *fileName,
                     TBinaryMFC *pHeaderSpe,double *spe,
                     TBinaryMFC *pHeaderDrk,double *drk,
                     TBinaryMFC *pHeaderOff,double *off)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  FILE *fp;         // pointer to the current file
  INT  pixFin,day,mon,year,hour,min,sec,nsec;        // date and time fields
  float                tmp;                                   // temporary variable
  UCHAR line[MAX_STR_SHORT_LEN+1],             // line of the current file
        keyWord[MAX_STR_SHORT_LEN+1],keyValue[MAX_STR_SHORT_LEN+1],ctmp;
  struct date          today;                                 // date of the current record

  INDEX i;          // browse pixels in the spectrum
  RC rc;            // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;

  memset(line,0,MAX_STR_SHORT_LEN+1);
  rc=ERROR_ID_NO;

  // Open file

  if ((fp=fopen(fileName,"rt"))==NULL)
//   rc=ERROR_ID_FILE_RECORD;
   rc=ERROR_ID_FILE_NOT_FOUND;
  else if (!STD_FileLength(fp))
   rc=ERROR_SetLast("ReadMFCRecordStd",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fileName);
  else
   {
    for (i=0;i<NDET;i++)
     spe[i]=(double)0.;

    if (fgets(line,MAX_STR_SHORT_LEN,fp) &&                                       // first line
        fgets(line,MAX_STR_SHORT_LEN,fp) && // (sscanf(line,"%d",&pixDeb)>=1) &&  // get the first pixel
        fgets(line,MAX_STR_SHORT_LEN,fp) && (sscanf(line,"%d",&pixFin)>=1))       // get the last pixel

     for (i=0;i<pixFin;i++)
      {
      	fgets(line,MAX_STR_SHORT_LEN,fp);
       sscanf(line,"%lf",&spe[i]);
      }

//    fgets(line,MAX_STR_SHORT_LEN,fp);

    fgets(line,MAX_STR_SHORT_LEN,fp);
    sscanf(line,"%[^'\n']",pHeaderSpe->specname);                               // Name of the spectrum

    fgets(line,MAX_STR_SHORT_LEN,fp);
    sscanf(line,"%[^'\n']",pHeaderSpe->spectroname);                            // Name of the spectrometer
    fgets(line,MAX_STR_SHORT_LEN,fp);
    sscanf(line,"%[^'\n']",pHeaderSpe->scan_dev);                               // Name of the scanning device

    memcpy(pRecord->Nom,pHeaderSpe->specname,20);

    if (fgets(line,MAX_STR_SHORT_LEN,fp))
     sscanf(line,"%2d%c%2d%c%d",&mon,&ctmp,&day,&ctmp,&year);

    fscanf(fp,"%d:%d:%d\n",&hour,&min,&sec);

    today.da_day=(UCHAR)day;
    today.da_mon=(UCHAR)mon;
    today.da_year=(USHORT)year;

    if (today.da_year<30)
     today.da_year+=(short)2000;
    else if (today.da_year<130)
     today.da_year+=(short)1900;
    else if (today.da_year<1930)
     today.da_year+=(short)100;

    pRecord->startTime.ti_hour=(UCHAR)hour;
    pRecord->startTime.ti_min=(UCHAR)min;
    pRecord->startTime.ti_sec=(UCHAR)sec;

    fscanf(fp,"%d:%d:%d\n",&hour,&min,&sec);

    pRecord->endTime.ti_hour=(UCHAR)hour;
    pRecord->endTime.ti_min=(UCHAR)min;
    pRecord->endTime.ti_sec=(UCHAR)sec;

    nsec=(pRecord->startTime.ti_hour*3600+pRecord->startTime.ti_min*60+pRecord->startTime.ti_sec+
          pRecord->endTime.ti_hour*3600+pRecord->endTime.ti_min*60+pRecord->endTime.ti_sec)/2;

    pRecord->present_day.da_day=(UCHAR)day;
    pRecord->present_day.da_mon=(UCHAR)mon;
    pRecord->present_day.da_year=(USHORT)year;

    if (pRecord->present_day.da_year<30)
     pRecord->present_day.da_year+=(short)2000;
    else if (pRecord->present_day.da_year<130)
     pRecord->present_day.da_year+=(short)1900;
    else if (pRecord->present_day.da_year<1930)
     pRecord->present_day.da_year+=(short)100;

    pRecord->present_time.ti_hour=(UCHAR)(nsec/3600);
    pRecord->present_time.ti_min=(UCHAR)((nsec%3600)/60);
    pRecord->present_time.ti_sec=(UCHAR)((nsec%3600)%60);

    pRecord->TDet     = 0;

    fscanf(fp,"%f\n",&tmp);
    fscanf(fp,"%f\n",&tmp);
    fscanf(fp,"SCANS %d\n",&pRecord->NSomme);
    fscanf(fp,"INT_TIME %lf\n",&pRecord->TotalExpTime);
    fgets(line,MAX_STR_SHORT_LEN,fp);
    fscanf(fp,"LONGITUDE %lf\n",&pRecord->longitude);
    fscanf(fp,"LATITUDE %lf\n",&pRecord->latitude);

    pRecord->TotalExpTime*=0.001;

    while (fgets(line,MAX_STR_SHORT_LEN,fp))
     {
     	if (strchr(line,'=')!=NULL)
     	 {
     	 	sscanf(line,"%s = %s",keyWord,keyValue);
     	 	if (!STD_Stricmp(keyWord,"AzimuthAngle"))
     	 	 pRecord->azimuthViewAngle=(float)atof(keyValue);
     	 	else if (!STD_Stricmp(keyWord,"ElevationAngle"))
     	 	 pRecord->elevationViewAngle=(float)atof(keyValue);
     	 	else if (!STD_Stricmp(keyWord,"ExposureTime"))
     	 	 pRecord->Tint=(double)atof(keyValue)*0.001;
     	 	else if (!STD_Stricmp(keyWord,"Latitude"))
     	 	 pRecord->latitude=(double)atof(keyValue);
     	 	else if (!STD_Stricmp(keyWord,"Longitude"))
     	 	 pRecord->longitude=(double)atof(keyValue);
     	 	else if (!STD_Stricmp(keyWord,"NumScans"))
     	 	 pRecord->NSomme=(int)atoi(keyValue);
     	 }
     }

    if ((pRecord->Tint<(double)1.e-3) && (pRecord->TotalExpTime>(double)1.e-3))
     pRecord->Tint=pRecord->TotalExpTime;

    pHeaderSpe->int_time=(float)pRecord->Tint;
    pHeaderSpe->noscans=pRecord->NSomme;

    // Offset correction if any

    if ((off!=NULL) && (pHeaderOff->noscans>0) && (THRD_browseType!=THREAD_BROWSE_MFC_OFFSET))
     {
      for (i=0;i<NDET;i++)
       spe[i]-=(double)off[i]*pHeaderSpe->noscans/pHeaderOff->noscans;
     }

    // Dark current correction if any

    if ((drk!=NULL) && (pHeaderDrk->int_time!=(float)0.) && (THRD_browseType!=THREAD_BROWSE_MFC_OFFSET) && (THRD_browseType!=THREAD_BROWSE_MFC_DARK))
     {
      for (i=0;i<NDET;i++)
       spe[i]-=(double)drk[i]*pHeaderSpe->int_time/(pHeaderDrk->int_time*pHeaderDrk->noscans);
     }
   }


  // Close file

  if (fp!=NULL)
   fclose(fp);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliMFCStd
// -----------------------------------------------------------------------------
// PURPOSE       MFC ASCII format read out
//
// INPUT         recordNo     index of record in file
//               dateFlag     0 no date constraint; 1 a date selection is applied
//               specFp       pointer to the spectra file
//
// OUTPUT        pEngineContext  : pointer to a structure whose some fields are filled
//                            with data on the current spectrum
//
// RETURN        ERROR_ID_FILE_NOT_FOUND : the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC ReliMFCStd(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  INT                  firstFile;                                               // number of the first file in the current directory
  UCHAR                fileName[MAX_STR_SHORT_LEN+1],                           // name of the current file (the current record)
                       format[20],
                      *ptr,*ptr2;                                               // pointers to parts in the previous string

  double               longit;                                                  // longitude of the current record
  FILE                *fp;                                                      // pointer to the current file
  RC                   rc;                                                      // return code
  double               tmLocal;

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;

  memset(fileName,0,MAX_STR_SHORT_LEN+1);
  strncpy(fileName,pEngineContext->fileInfo.fileName,MAX_STR_SHORT_LEN);
  fp=NULL;

  rc=ERROR_ID_NO;

  if ((ptr=strrchr(fileName,PATH_SEP))==NULL)
   rc=ERROR_ID_FILE_RECORD;
  else if ((recordNo>0) && (recordNo<=pEngineContext->recordNumber))
   {
    // Build the right file name

//    if (THRD_treeCallFlag)
     {
//      ptr+=2;
//      sscanf(ptr,"%d",&firstFile);
      if ((ptr2=strrchr(ptr,'.'))==NULL)
       ptr2=strrchr(ptr,'\0');
      for (ptr=ptr2;isdigit(*(ptr-1));ptr--);
//      ptr=ptr2-5;

      sscanf(ptr,"%d",&firstFile);
      sprintf(format,"%%0%dd%%s",ptr2-ptr);
      sprintf(ptr,format,firstFile+recordNo-1,ptr2);
     }

    // open the file

    if (!(rc=MFC_ReadRecordStd(pEngineContext,fileName,&MFC_header,pBuffers->spectrum,&MFC_headerDrk,pBuffers->varPix,&MFC_headerOff,pBuffers->dnl)))
     {
      memset(PATH_fileSpectra,0,MAX_STR_SHORT_LEN+1);
      strncpy(PATH_fileSpectra,fileName,MAX_STR_SHORT_LEN);

      pRecord->SkyObs   = 0;
      pRecord->rejected = 0;
      pRecord->ReguTemp = 0;

      pRecord->Azimuth  = 0;
      pRecord->BestShift=(double)0.;
      pRecord->NTracks=0;

      pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);
      pRecord->Zm=ZEN_FNTdiz(ZEN_FNCrtjul(&pRecord->Tm),&longit,&pRecord->latitude,NULL);
      pRecord->TimeDec=(double)pRecord->present_time.ti_hour+pRecord->present_time.ti_min/60.+pRecord->present_time.ti_sec/3600.;

      pRecord->altitude=(double)0.;
      longit=-pRecord->longitude;

      tmLocal=pRecord->Tm+THRD_localShift*3600.;

      pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
      pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

      // User constraints

      if (rc || (dateFlag && ((pRecord->localCalDay!=localDay) || (pRecord->elevationViewAngle<88.))))                  // reference spectra are zenith only
       rc=ERROR_ID_FILE_RECORD;
      else if (pEngineContext->project.instrumental.mfcRevert)
       VECTOR_Invert(pBuffers->spectrum,NDET);

      // for NOVAC tests, straylight correction

    /*  {
      	int i;
      	double offset;

      	offset=(double)0.;

      	for (i=50;i<200;i++)
        offset+=pBuffers->spectrum[i];

       offset/=(double)150.;

       for (i=0;i<NDET;i++)
        pBuffers->spectrum[i]-=offset;
      }  */
     }
   }
  else
   rc=ERROR_ID_FILE_RECORD;

  // Close file

  if (fp!=NULL)
   fclose(fp);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      MFC_LoadAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Load analysis parameters depending on the reference spectrum
//
// INPUT         pEngineContext    data on the current file
//
// RETURN        0 for success
// -----------------------------------------------------------------------------

RC MFC_LoadAnalysis(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  PRJCT_INSTRUMENTAL *pInstrumental;
  UCHAR fileName[MAX_STR_SHORT_LEN+1],*ptr;
  TBinaryMFC tbinaryRef;
  INDEX indexWindow,indexFeno,indexTabCross;                                    // indexes for loops and array
  CROSS_REFERENCE *pTabCross;                                                   // pointer to the current cross section
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a symbol
  FENO *pTabFeno;                                                               // pointer to the current spectral analysis window
  double factTemp;                                                              // working variables
  INT DimL,useKurucz,saveFlag;                                                       // working variables
  RC rc;                                                                        // return code

  // Initializations

  pBuffers=&pEngineContext->buffers;

  saveFlag=(INT)pEngineContext->project.spectra.displayDataFlag;
  pInstrumental=&pEngineContext->project.instrumental;
  memset(fileName,0,MAX_STR_SHORT_LEN+1);
  strncpy(fileName,pEngineContext->fileInfo.fileName,MAX_STR_SHORT_LEN);
  rc=ERROR_ID_NO;

  if ((THRD_id==THREAD_TYPE_ANALYSIS) && MFC_refFlag && ((ptr=strrchr(fileName,PATH_SEP))!=NULL))
   {
    useKurucz=0;

    // Browse analysis windows and load missing data

    for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
     if (!TabFeno[indexFeno].hidden && !TabFeno[indexFeno].gomeRefFlag)
      {
       pTabFeno=&TabFeno[indexFeno];
       pTabFeno->NDET=NDET;
       FILES_RebuildFileName(ptr+1,pTabFeno->refFile,0);

       if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
        rc=MFC_ReadRecord(fileName,
                         &tbinaryRef,pTabFeno->Sref,
                         &MFC_headerDrk,pBuffers->varPix,
                         &MFC_headerOff,pBuffers->dnl,
                         pInstrumental->mfcMaskSpec,pInstrumental->mfcMaskSpec,pInstrumental->mfcRevert);
    	 	else
        rc=MFC_ReadRecordStd(pEngineContext,fileName,
                            &tbinaryRef,pTabFeno->Sref,
                            &MFC_headerDrk,pBuffers->varPix,
                            &MFC_headerOff,pBuffers->dnl);

       if (!rc && !(rc=ANALYSE_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&factTemp,"MFC_LoadAnalysis (Reference) ")))
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
                 (indexTabCross==pTabFeno->indexCommonResidual))) &&
                ((rc=ANALYSE_CheckLambda(pWrkSymbol,pTabFeno->LambdaRef,"MFC_LoadAnalysis "))!=ERROR_ID_NO))

            goto EndMFC_LoadAnalysis;
          }

         // Gaps : rebuild subwindows on new wavelength scale

         for (indexWindow=0,DimL=0;indexWindow<pTabFeno->svd.Z;indexWindow++)
          {
           pTabFeno->svd.Fenetre[indexWindow][0]=FNPixel(pTabFeno->LambdaRef,pTabFeno->svd.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
           pTabFeno->svd.Fenetre[indexWindow][1]=FNPixel(pTabFeno->LambdaRef,pTabFeno->svd.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

           DimL+=(pTabFeno->svd.Fenetre[indexWindow][1]-pTabFeno->svd.Fenetre[indexWindow][0]+1);
          }

         pTabFeno->svd.DimL=DimL;

         // Buffers allocation

         SVD_Free("MFC_LoadAnalysis",&pTabFeno->svd);
         SVD_LocalAlloc("MFC_LoadAnalysis",&pTabFeno->svd);

         pTabFeno->Decomp=1;

         if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef))!=ERROR_ID_NO) ||
             (!pKuruczOptions->fwhmFit && pTabFeno->xsToConvolute &&
             ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,&ANALYSIS_slit,pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2,&pSlitOptions->slitFunction.slitParam3,&pSlitOptions->slitFunction.slitParam4))!=ERROR_ID_NO)))

          goto EndMFC_LoadAnalysis;
        }

       useKurucz+=pTabFeno->useKurucz;
      }

    // Wavelength calibration alignment

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
     {
      KURUCZ_Init(0);

      if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,NULL /* QDOAS !!! responseHandle */))!=ERROR_ID_NO))
       goto EndMFC_LoadAnalysis;
     }
   }

  // Return

  EndMFC_LoadAnalysis :

  return rc;
 }

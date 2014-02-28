
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

char MFC_fileInstr[MAX_STR_SHORT_LEN+1],      // instrumental function file name
      MFC_fileDark[MAX_STR_SHORT_LEN+1],       // dark current file name
      MFC_fileOffset[MAX_STR_SHORT_LEN+1];     // offset file name

int mfcLastSpectrum=0;

RC MFC_LoadOffset(ENGINE_CONTEXT *pEngineContext)
 {
 	// Declarations

 	PROJECT *pProject;                                                            // pointer to the current project
  PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RC rc;

  // Initializations

  pProject=&pEngineContext->project;
  pInstrumental=&pProject->instrumental;
  pBuffers=&pEngineContext->buffers;

  rc=ERROR_ID_NO;

  strcpy(MFC_fileOffset,pInstrumental->offsetFile);

  // Read offset

  if (strlen(pInstrumental->offsetFile) && (pBuffers->offset!=NULL))                  // offset
   {
   	VECTOR_Init(pBuffers->offset,0.,NDET);

    rc=(pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)?
        MFC_ReadRecord(pInstrumental->offsetFile,&MFC_headerOff,pBuffers->offset,
       &MFC_headerDrk,NULL,&MFC_headerOff,NULL,pInstrumental->mfcMaskOffset,pInstrumental->mfcMaskSpec,pInstrumental->mfcRevert): // remove offset from dark current
        MFC_ReadRecordStd(pEngineContext,pInstrumental->offsetFile,&MFC_headerOff,pBuffers->offset,
       &MFC_headerDrk,NULL,&MFC_headerOff,NULL);

    if (rc==ERROR_ID_FILE_END)
     rc=0;
   }

  // Return

  return rc;
 }

RC MFC_LoadDark(ENGINE_CONTEXT *pEngineContext)
 {
 	// Declarations

 	PROJECT *pProject;                                                            // pointer to the current project
  PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RC rc;

  // Initializations

  pProject=&pEngineContext->project;
  pInstrumental=&pProject->instrumental;
  pBuffers=&pEngineContext->buffers;

  rc=ERROR_ID_NO;

  strcpy(MFC_fileDark,pInstrumental->vipFile);

  // Read dark current

  if (strlen(pInstrumental->vipFile) && (pBuffers->varPix!=NULL))                  // dark current
   {
   	VECTOR_Init(pBuffers->varPix,0.,NDET);

    rc=(pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)?
        MFC_ReadRecord(pInstrumental->vipFile,&MFC_headerDrk,pBuffers->varPix,
       &MFC_headerDrk,NULL,&MFC_headerOff,pBuffers->offset,pInstrumental->mfcMaskOffset,pInstrumental->mfcMaskSpec,pInstrumental->mfcRevert): // remove offset from dark current
        MFC_ReadRecordStd(pEngineContext,pInstrumental->vipFile,&MFC_headerDrk,pBuffers->varPix,
       &MFC_headerDrk,NULL,&MFC_headerOff,pBuffers->offset);

    if (rc==ERROR_ID_FILE_END)
     rc=0;
   }

  // Return

  return rc;
 }

INDEX MFC_SearchForCurrentFileIndex(ENGINE_CONTEXT *pEngineContext)
 {
 	// Declarations

 	char  fileName[MAX_ITEM_TEXT_LEN+1];                                        // name of the current file
 	char *ptr,*scanRefFiles;
 	INDEX   indexRecord,indexFile;
 	int     nscanRefFiles;

 	// Initializations

 	indexRecord=ITEM_NONE;
 	scanRefFiles=pEngineContext->analysisRef.scanRefFiles;
 	nscanRefFiles=pEngineContext->analysisRef.nscanRefFiles;

 	//  Browse files

 	if (pEngineContext->analysisRef.refScan && (scanRefFiles!=NULL) && ((ptr=strrchr(pEngineContext->fileInfo.fileName,'/'))!=NULL) && (strlen(ptr+1)>0))
 	 {
 	  strcpy(fileName,ptr+1);

 	 	for (indexFile=0;indexFile<nscanRefFiles;indexFile++)
 	 	 if (!strcasecmp(fileName,&scanRefFiles[indexFile*(MAX_ITEM_TEXT_LEN+1)]))
 	 	  break;

 	 	if (indexFile<nscanRefFiles)
 	 	 indexRecord=indexFile;
 	 }

 	// Return

 	return indexRecord;
 }

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

RC SetMFC(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  PRJCT_INSTRUMENTAL *pInstrumental;
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  char  fileName[MAX_STR_SHORT_LEN+1];                                        // name of the current file
  RC rc;

  // Initializations

  pBuffers=&pEngineContext->buffers;

  memset(fileName,0,MAX_STR_SHORT_LEN+1);
  strncpy(fileName,pEngineContext->fileInfo.fileName,MAX_STR_SHORT_LEN);
  pInstrumental=&pEngineContext->project.instrumental;
  pEngineContext->lastRefRecord=0;
  mfcLastSpectrum=0;
  rc=ERROR_ID_NO;

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

RC MFC_ReadRecord(char *fileName,
                  TBinaryMFC *pHeaderSpe,double *spe,
                  TBinaryMFC *pHeaderDrk,double *drk,
                  TBinaryMFC *pHeaderOff,double *off,
                  unsigned int mask,unsigned int maskSpec,unsigned int revertFlag)
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
       ((mask!=maskSpec) && ((pHeaderSpe->ty&mask)==0) && ((unsigned int)pHeaderSpe->wavelength1!=mask)) ||                    // spectrum selection
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
         spe[i]-=(double)pHeaderSpe->noscans*drk[i]*pHeaderSpe->int_time/(pHeaderDrk->int_time*pHeaderDrk->noscans);
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
RC ReliMFC(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,unsigned int mfcMask)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  int                  day,mon,year,hour,min,sec,nsec,nsec1,nsec2,              // date and time fields
                       firstFile;                                               // number of the first file in the current directory
  char                fileName[MAX_STR_SHORT_LEN+1],                           // name of the current file (the current record)
                       format[20],
                      *ptr,*ptr2;                                               // pointers to parts in the previous string
  SHORT_DATE           today;                                                   // date of the current record
  PRJCT_INSTRUMENTAL  *pInstrumental;                                           // pointer to the instrumental part of the project
  RC                   rc;                                                      // return code
  double tmLocal,Tm1,Tm2;

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
      sprintf(format,"%%0%dd%%s",(int)(ptr2-ptr));
      sprintf(ptr,format,firstFile+recordNo-1,ptr2);
     }

    // Record read out

    if (!(rc=MFC_ReadRecord(fileName,&MFC_header,pBuffers->spectrum,&MFC_headerDrk,pBuffers->varPix,&MFC_headerOff,pBuffers->offset,mfcMask,pInstrumental->mfcMaskSpec,pInstrumental->mfcRevert)))
     {
      if ((mfcMask==pInstrumental->mfcMaskSpec) &&
         (((pInstrumental->mfcMaskSpec!=(unsigned int)0) && ((unsigned int)MFC_header.ty==mfcMask)) ||
		  ((pInstrumental->mfcMaskSpec==(unsigned int)0) && ((unsigned int)MFC_header.ty==pInstrumental->mfcMaskSpec))) &&
        (((double)pInstrumental->wavelength>(double)100.) && ((MFC_header.wavelength1<(double)pInstrumental->wavelength-5.) || (MFC_header.wavelength1>(double)pInstrumental->wavelength+5.))))

       rc=ERROR_ID_FILE_RECORD;

      // In automatic file selection, replace instrumental functions with new ones if found

      if ((mfcMask==pInstrumental->mfcMaskSpec) &&
        (((mfcMask!=(unsigned int)0) && ((unsigned int)MFC_header.ty!=mfcMask)) || ((mfcMask==(unsigned int)0) && (rc==ERROR_ID_FILE_RECORD) && ((unsigned int)MFC_header.wavelength1!=mfcMask))))
       {
        if (pInstrumental->mfcMaskUse)
         {
          if ((((MFC_header.ty&pInstrumental->mfcMaskInstr)!=0) || (MFC_header.wavelength1==pInstrumental->mfcMaskInstr)) && (pBuffers->instrFunction!=NULL))
           {
            MFC_ReadRecord(fileName,
                          &MFC_headerInstr,pBuffers->instrFunction,
                          &MFC_headerDrk,pBuffers->varPix,               // instrument function should be corrected for dark current
                          &MFC_headerOff,pBuffers->offset,                  // instrument function should be corrected for offset
                          pInstrumental->mfcMaskInstr,pInstrumental->mfcMaskSpec,pInstrumental->mfcRevert);

            FILES_CompactPath(MFC_fileInstr,fileName,1,1);
           }
          else if ((((MFC_header.ty&pInstrumental->mfcMaskDark)!=0) || (MFC_header.wavelength1==pInstrumental->mfcMaskDark)) && (pBuffers->varPix!=NULL))
           {
            MFC_ReadRecord(fileName,
                          &MFC_headerInstr,pBuffers->varPix,
                          &MFC_headerDrk,NULL,                            // no correction for dark current
                          &MFC_headerOff,pBuffers->offset,                  // dark current should be corrected for offset
                          pInstrumental->mfcMaskDark,pInstrumental->mfcMaskSpec,0);

            FILES_CompactPath(MFC_fileDark,fileName,1,1);
           }
          else if ((((MFC_header.ty&pInstrumental->mfcMaskOffset)!=0) || (MFC_header.wavelength1==pInstrumental->mfcMaskOffset)) &&  (pBuffers->offset!=NULL))
           {
            MFC_ReadRecord(fileName,
                          &MFC_headerOff,pBuffers->offset,
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
        // Date and time read out

        sscanf(MFC_header.dateAndTime,"%d.%d.%d",&day,&mon,&year);
        sscanf(&MFC_header.dateAndTime[9],"%d:%d:%d",&hour,&min,&sec);

        today.da_day=(char)day;
        today.da_mon=(char)mon;
        today.da_year=(short)year;

        if (today.da_year<30)
         today.da_year+=(short)2000;
        else if (today.da_year<130)
         today.da_year+=(short)1900;
        else if (today.da_year<1930)
         today.da_year+=(short)100;

        pRecord->startTime.ti_hour=(unsigned char)hour;
        pRecord->startTime.ti_min=(unsigned char)min;
        pRecord->startTime.ti_sec=(unsigned char)sec;

        sscanf(&MFC_header.dateAndTime[18],"%d:%d:%d",&hour,&min,&sec);

        pRecord->endTime.ti_hour=(unsigned char)hour;
        pRecord->endTime.ti_min=(unsigned char)min;
        pRecord->endTime.ti_sec=(unsigned char)sec;

        Tm1=(double)ZEN_NbSec(&today,&pRecord->startTime,0);
        Tm2=(double)ZEN_NbSec(&today,&pRecord->endTime,0);

        Tm1=(Tm1+Tm2)*0.5;

        pRecord->present_day.da_year  = (short) ZEN_FNCaljye (&Tm1);
        pRecord->present_day.da_mon   = (char) ZEN_FNCaljmon (ZEN_FNCaljye(&Tm1),ZEN_FNCaljda(&Tm1));
        pRecord->present_day.da_day   = (char) ZEN_FNCaljday (ZEN_FNCaljye(&Tm1),ZEN_FNCaljda(&Tm1));

        // Data on the current spectrum

        nsec1=pRecord->startTime.ti_hour*3600+pRecord->startTime.ti_min*60+pRecord->startTime.ti_sec;
        nsec2=pRecord->endTime.ti_hour*3600+pRecord->endTime.ti_min*60+pRecord->endTime.ti_sec;

        if (nsec2<nsec1)
         nsec2+=86400;

        nsec=(nsec1+nsec2)/2;

        pRecord->present_time.ti_hour=(unsigned char)(nsec/3600);
        pRecord->present_time.ti_min=(unsigned char)((nsec%3600)/60);
        pRecord->present_time.ti_sec=(unsigned char)((nsec%3600)%60);

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

RC MFC_ReadRecordStd(ENGINE_CONTEXT *pEngineContext,char *fileName,
                     TBinaryMFC *pHeaderSpe,double *spe,
                     TBinaryMFC *pHeaderDrk,double *drk,
                     TBinaryMFC *pHeaderOff,double *off)
 {
  // Declarations

  PRJCT_INSTRUMENTAL *pInstrumental;
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  FILE *fp;         // pointer to the current file
  int  pixFin,day,mon,year,hour,min,sec,nsec,mfcDate[3],yearN,dateSize,sepN;        // date and time fields
  float                tmp;                                   // temporary variable
  char line[MAX_STR_SHORT_LEN+1],             // line of the current file
        keyWord[MAX_STR_SHORT_LEN+1],keyValue[MAX_STR_SHORT_LEN+1],ctmp;
  SHORT_DATE         today;                                 // date of the current record

  INDEX i,iDay,iMon,iYear;          // browse pixels in the spectrum
  double Tm1,Tm2;
  int nsec1,nsec2;
  RC rc;            // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pInstrumental=&pEngineContext->project.instrumental;
  memset(mfcDate,0,sizeof(int)*3);
  iDay=iMon=iYear=-1;
  yearN=sepN=0;
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

    dateSize=strlen(pInstrumental->mfcStdDate);

    for (i=0;i<dateSize;i++)
     {
      if ((pInstrumental->mfcStdDate[i]=='Y') || (pInstrumental->mfcStdDate[i]=='y'))
       {
        iYear=sepN;
        yearN++;
       }
      else if ((pInstrumental->mfcStdDate[i]=='M') ||(pInstrumental->mfcStdDate[i]=='m'))
       iMon=sepN;
      else if ((pInstrumental->mfcStdDate[i]=='D') || (pInstrumental->mfcStdDate[i]=='d'))
       iDay=sepN;
      else
       sepN++;
     }

    if (fgets(line,MAX_STR_SHORT_LEN,fp))
     sscanf(line,"%d%c%d%c%d",&mfcDate[0],&ctmp,&mfcDate[1],&ctmp,&mfcDate[2]);

    day=mfcDate[iDay];
    mon=mfcDate[iMon];
    year=mfcDate[iYear];

    fscanf(fp,"%d:%d:%d\n",&hour,&min,&sec);

    today.da_day=(char)day;
    today.da_mon=(char)mon;
    today.da_year=(int)year;

    if (today.da_year<30)
     today.da_year+=(short)2000;
    else if (today.da_year<130)
     today.da_year+=(short)1900;
    else if (today.da_year<1930)
     today.da_year+=(short)100;

    pRecord->startTime.ti_hour=(unsigned char)hour;
    pRecord->startTime.ti_min=(unsigned char)min;
    pRecord->startTime.ti_sec=(unsigned char)sec;

    fscanf(fp,"%d:%d:%d\n",&hour,&min,&sec);

    pRecord->endTime.ti_hour=(unsigned char)hour;
    pRecord->endTime.ti_min=(unsigned char)min;
    pRecord->endTime.ti_sec=(unsigned char)sec;

    Tm1=(double)ZEN_NbSec(&today,&pRecord->startTime,0);
    Tm2=(double)ZEN_NbSec(&today,&pRecord->endTime,0);

    Tm1=(Tm1+Tm2)*0.5;

    pRecord->present_day.da_year  = (short) ZEN_FNCaljye (&Tm1);
    pRecord->present_day.da_mon   = (char) ZEN_FNCaljmon (ZEN_FNCaljye(&Tm1),ZEN_FNCaljda(&Tm1));
    pRecord->present_day.da_day   = (char) ZEN_FNCaljday (ZEN_FNCaljye(&Tm1),ZEN_FNCaljda(&Tm1));

    // Data on the current spectrum

    nsec1=pRecord->startTime.ti_hour*3600+pRecord->startTime.ti_min*60+pRecord->startTime.ti_sec;
    nsec2=pRecord->endTime.ti_hour*3600+pRecord->endTime.ti_min*60+pRecord->endTime.ti_sec;

    if (nsec2<nsec1)
     nsec2+=86400;

    nsec=(nsec1+nsec2)/2;

    pRecord->present_time.ti_hour=(unsigned char)(nsec/3600);
    pRecord->present_time.ti_min=(unsigned char)((nsec%3600)/60);
    pRecord->present_time.ti_sec=(unsigned char)((nsec%3600)%60);

    pRecord->TDet     = 0;

    fscanf(fp,"%f\n",&tmp);
    fscanf(fp,"%f\n",&tmp);
    fscanf(fp,"SCANS %d\n",&pRecord->NSomme);
    fscanf(fp,"int_TIME %lf\n",&pRecord->TotalExpTime);
    fgets(line,MAX_STR_SHORT_LEN,fp);
    fscanf(fp,"LONGITUDE %lf\n",&pRecord->longitude);
    fscanf(fp,"LATITUDE %lf\n",&pRecord->latitude);

    pRecord->TotalExpTime*=0.001;

    while (fgets(line,MAX_STR_SHORT_LEN,fp))
     {
     	if (strchr(line,'=')!=NULL)
     	 {
     	 	sscanf(line,"%s = %s",keyWord,keyValue);
     	 	if (!strcasecmp(keyWord,"AzimuthAngle"))
     	 	 pRecord->azimuthViewAngle=(float)atof(keyValue);
     	 	else if (!strcasecmp(keyWord,"ElevationAngle"))
     	 	 pRecord->elevationViewAngle=(float)atof(keyValue);
     	 	else if (!strcasecmp(keyWord,"ExposureTime"))
     	 	 pRecord->Tint=(double)atof(keyValue)*0.001;
     	 	else if (!strcasecmp(keyWord,"Latitude"))
     	 	 pRecord->latitude=(double)atof(keyValue);
     	 	else if (!strcasecmp(keyWord,"Longitude"))
     	 	 pRecord->longitude=(double)atof(keyValue);
     	 	else if (!strcasecmp(keyWord,"NumScans"))
     	 	 pRecord->NSomme=(int)atoi(keyValue);
     	 	else if (!strcasecmp(keyWord,"Temperature"))
     	 	 pRecord->TDet=(double)atof(keyValue);
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
       spe[i]-=(double)pHeaderSpe->noscans*drk[i]*pHeaderSpe->int_time/(pHeaderDrk->int_time*pHeaderDrk->noscans);
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

  char                fileName[MAX_STR_SHORT_LEN+1],                           // name of the current file (the current record)
                      *ptr;                                                     // pointers to parts in the previous string

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

// QDOAS ??? //    if (THRD_treeCallFlag)
// QDOAS ???      {
// QDOAS ??? //      ptr+=2;
// QDOAS ??? //      sscanf(ptr,"%d",&firstFile);
// QDOAS ???       if ((ptr2=strrchr(ptr,'.'))==NULL)
// QDOAS ???        ptr2=strrchr(ptr,'\0');
// QDOAS ???       for (ptr=ptr2;isdigit(*(ptr-1));ptr--);
// QDOAS ??? //      ptr=ptr2-5;
// QDOAS ???
// QDOAS ???       sscanf(ptr,"%d",&firstFile);
// QDOAS ???       sprintf(format,"%%0%dd%%s",ptr2-ptr);
// QDOAS ???       sprintf(ptr,format,firstFile+recordNo-1,ptr2);
// QDOAS ???      }

    // open the file

    if (!(rc=MFC_ReadRecordStd(pEngineContext,fileName,&MFC_header,pBuffers->spectrum,&MFC_headerDrk,pBuffers->varPix,&MFC_headerOff,pBuffers->offset)))
     {
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

      if (rc || (dateFlag && ((pRecord->localCalDay!=localDay) || (pRecord->elevationViewAngle<80.))) ||                     // reference spectra are zenith only
                (!dateFlag && pEngineContext->analysisRef.refScan && !pEngineContext->analysisRef.refSza && (pRecord->elevationViewAngle>80.)))
       rc=ERROR_ID_FILE_RECORD;
      else if (pEngineContext->project.instrumental.mfcRevert)
       VECTOR_Invert(pBuffers->spectrum,NDET);
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

// ===========================
// MFC BIRA-IASB BINARY FORMAT
// ===========================

//! \struct MFC_BIRA
//! \brief Description of a record in the new MFC binary file format developed
//!        at BIRA-IASB to make the processing of MFC files by QDOAS easier

typedef struct
 {
  //! \details the measurement type
  int          measurementType;
  //! \details the measurement date
  SHORT_DATE   measurementDate;
  //! \details the starting measurement time
  struct time  startTime;
  //! \details the ending measurement time
  struct time  endTime;
  //! \details the name of the spectrometer
  char         spectroName[64];
  //! \details the name of the device
  char         deviceName[64];
  //! \details the number of scans
  int          scansNumber;
  //! \details the exposure time
  float        exposureTime;
  //! \details the total exposure time (= \ref scansNumber x \ref exposureTime)
  float        totalExpTime;
  //! \details the name of the observation site
  char         siteName[64];
  //! \details the longitude of the observation site
  float        longitude;
  //! \details the latitude of the observation site
  float        latitude;
  //! \details the viewing azimuth angle
  float        azimuthAngle;
  //! \details the viewing elevation angle
  float        elevationAngle;
  //! \details the original file name
  char         fileName[MAX_STR_LEN+1];
  //! \details the temperature
  float        temperature;
 }
MFCBIRA_HEADER;

// -----------------------------------------------------------------------------
// FUNCTION MFCBIRA_Set
// -----------------------------------------------------------------------------
/*!
   \fn      RC MFCBIRA_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
   \details Set the number of records in a file in the MFC BIRA binary format.
            This number is the first integer read from the file.  Check also
            the size of the detector\n
   \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
   \param   [in]  specFp pointer to the spectra file to read
   \return  ERROR_ID_FILE_NOT_FOUND if the input file pointer \a specFp is NULL \n
            ERROR_ID_FILE_EMPTY if the file is empty\n
            ERROR_ID_NO on success
*/
// -----------------------------------------------------------------------------

RC MFCBIRA_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  MFCBIRA_HEADER header;
  MFC_BIRA *pMfcInfo;
  ANALYSIS_REF *pRef;
  double *offset,*darkCurrent;
  float *spectrum,drkTint;
  int detectorSize;
  int i,j,nOff,nDrk;
  RC rc;                                                                        // return code

  // Initializations

  pEngineContext->recordNumber=0;
  offset=pEngineContext->buffers.offset;
  darkCurrent=pEngineContext->buffers.varPix;
  pMfcInfo=&pEngineContext->recordInfo.mfcBira;
  pRef=&pEngineContext->analysisRef;
  spectrum=NULL;

  // Release scanref buffer

  if (pRef->scanRefIndexes!=NULL)
   MEMORY_ReleaseBuffer("SetCCD_EEV","scanRefIndexes",pRef->scanRefIndexes);
  pRef->scanRefIndexes=NULL;

  rc=ERROR_ID_NO;

  // Get the number of spectra in the file

  if (specFp==NULL)
   rc=ERROR_SetLast("MFCBIRA_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if (!STD_FileLength(specFp))
   rc=ERROR_SetLast("MFCBIRA_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
  else
   {
   	fread(&pEngineContext->recordNumber,sizeof(int),1,specFp);
   	fread(&detectorSize,sizeof(int),1,specFp);

   	if (pEngineContext->recordNumber<=0)
   	 rc=ERROR_SetLast("MFCBIRA_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
   	else if (detectorSize!=NDET)
   	 rc=ERROR_SetLast("MFCBIRA_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_BAD_LENGTH,pEngineContext->fileInfo.fileName);
   	else if ((spectrum=MEMORY_AllocBuffer("MFCBIRA_Reli","spectrum",sizeof(float)*NDET,1,0,MEMORY_TYPE_FLOAT))==NULL)
   	 rc=ERROR_ID_ALLOC;

   	else
   	 {
      // Allocate a buffer for the indexes of selected reference spectra (scan mode)

      if (pEngineContext->analysisRef.refScan && pEngineContext->recordNumber &&
        ((pRef->scanRefIndexes=(int *)MEMORY_AllocBuffer("SetCCD_EEV","scanRefIndexes",pEngineContext->recordNumber,sizeof(int),0,MEMORY_TYPE_INT))==NULL))

       rc=ERROR_ID_ALLOC;

      else
       pEngineContext->fileInfo.nScanRef=0;

       // Load offset and dark current

      if (!rc && (offset!=NULL) && (darkCurrent!=NULL))
       {
   	 	  // Initialize vectors

        VECTOR_Init(offset,(double)0.,NDET);
        VECTOR_Init(darkCurrent,(double)0.,NDET);

        drkTint=0.;

        // Browse records

        for (i=nOff=nDrk=0;i<pEngineContext->recordNumber;i++)
         {
         	fseek(specFp,2L*sizeof(int)+i*(sizeof(MFCBIRA_HEADER)+NDET*sizeof(float)),SEEK_SET);
   	      fread(&header,sizeof(MFCBIRA_HEADER),1,specFp);

   	      // Load offset

   	      if (header.measurementType==PRJCT_INSTR_MAXDOAS_TYPE_OFFSET)
   	       {
   	        fread(spectrum,sizeof(float)*NDET,1,specFp);
   	        for (j=0;j<NDET;j++)
   	         offset[j]+=(double)spectrum[j];

   	        nOff+=header.scansNumber;                                             // all offset should have the same exposure time
   	       }

   	      // Load dark current

   	      else if (header.measurementType==PRJCT_INSTR_MAXDOAS_TYPE_DARK)
   	       {
   	        fread(spectrum,sizeof(float)*NDET,1,specFp);
   	        for (j=0;j<NDET;j++)
   	         darkCurrent[j]+=(double)spectrum[j];

   	        drkTint=header.exposureTime;                                          // all dark current should have the same exposure time
   	        nDrk++;
   	       }
         }

        // Average offset (account for the number of spectra and the number of scans)

        if (nOff)
        	for (j=0;j<NDET;j++)
        	 offset[j]/=nOff;

        // Average dark current and correct by the offset

        if (nDrk)
        	for (j=0;j<NDET;j++)                                                     // drk=drk-offset*drkScans/offScans
        	 darkCurrent[j]=(darkCurrent[j]-offset[j])/((double)nDrk*drkTint);       // number of scans for the dark current should be 1
       }
   	 }
  }

  // Release allocated buffer

  if (spectrum!=NULL)
   MEMORY_ReleaseBuffer("MFCBIRA_Reli","spectrum",spectrum);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION MFCBIRA_Reli
// -----------------------------------------------------------------------------
/*!
   \fn      RC MFCBIRA_Reli(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
   \details
   \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
   \param   [in]  recordNo        the index of the record to read
   \param   [in]  dateFlag        1 to search for a reference spectrum; 0 otherwise
   \param   [in]  localDay        if \a dateFlag is 1, the calendar day for the reference spectrum to search for
   \param   [in]  specFp          pointer to the spectra file to read
   \return  ERROR_ID_ALLOC if the allocation of the buffer for the spectrum failed \n
            ERROR_ID_FILE_RECORD if the record is the spectrum is not a spectrum to analyze (sky or dark spectrum)\n
            ERROR_ID_NO on success
*/
// -----------------------------------------------------------------------------

RC MFCBIRA_Reli(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  MFCBIRA_HEADER header;                                                        // header of records
  RECORD_INFO *pRecordInfo;                                                     // pointer to the data part of the engine context
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  float *spectrum;                                                              // pointer to spectrum and offset
  double Tm1,Tm2,tmLocal,longit;
  int nsec,nsec1,nsec2;
  INDEX   i;                                                                    // browse pixels of the detector
  RC      rc;                                                                   // return code

  // Initializations

  pRecordInfo=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;
  rc=ERROR_ID_NO;

  if ((spectrum=MEMORY_AllocBuffer("MFCBIRA_Reli","spectrum",sizeof(float)*NDET,1,0,MEMORY_TYPE_FLOAT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
   	// Go to the requested record

   	fseek(specFp,2L*sizeof(int)+(recordNo-1)*(sizeof(MFCBIRA_HEADER)+NDET*sizeof(float)),SEEK_SET);
   	fread(&header,sizeof(MFCBIRA_HEADER),1,specFp);
   	fread(spectrum,sizeof(float)*NDET,1,specFp);

   	// Retrieve the main information from the header

   	pRecordInfo->NSomme=header.scansNumber;
   	pRecordInfo->Tint=header.exposureTime;
   	pRecordInfo->latitude=header.latitude;
   	pRecordInfo->longitude=header.longitude;
   	pRecordInfo->TotalExpTime=header.totalExpTime;
    pRecordInfo->elevationViewAngle=header.elevationAngle;
    pRecordInfo->azimuthViewAngle=header.azimuthAngle;
    pRecordInfo->TDet=header.temperature;

    strcpy(pRecordInfo->mfcBira.originalFileName,header.fileName);
    pRecordInfo->mfcBira.measurementType=header.measurementType;

    // Calculate the date and time at half of the measurement

    memcpy(&pRecordInfo->startTime,&header.startTime,sizeof(struct time));
    memcpy(&pRecordInfo->endTime,&header.endTime,sizeof(struct time));

    Tm1=(double)ZEN_NbSec(&header.measurementDate,&pRecordInfo->startTime,0);
    Tm2=(double)ZEN_NbSec(&header.measurementDate,&pRecordInfo->endTime,0);

    Tm1=(Tm1+Tm2)*0.5;

    pRecordInfo->present_day.da_year  = (short) ZEN_FNCaljye (&Tm1);
    pRecordInfo->present_day.da_mon   = (char) ZEN_FNCaljmon (ZEN_FNCaljye(&Tm1),ZEN_FNCaljda(&Tm1));
    pRecordInfo->present_day.da_day   = (char) ZEN_FNCaljday (ZEN_FNCaljye(&Tm1),ZEN_FNCaljda(&Tm1));

    // Data on the current spectrum

    nsec1=pRecordInfo->startTime.ti_hour*3600+pRecordInfo->startTime.ti_min*60+pRecordInfo->startTime.ti_sec;
    nsec2=pRecordInfo->endTime.ti_hour*3600+pRecordInfo->endTime.ti_min*60+pRecordInfo->endTime.ti_sec;

    if (nsec2<nsec1)
     nsec2+=86400;

    nsec=(nsec1+nsec2)/2;

    pRecordInfo->present_time.ti_hour=(unsigned char)(nsec/3600);
    pRecordInfo->present_time.ti_min=(unsigned char)((nsec%3600)/60);
    pRecordInfo->present_time.ti_sec=(unsigned char)((nsec%3600)%60);

    longit=-header.longitude;

    pRecordInfo->Tm=(double)ZEN_NbSec(&pRecordInfo->present_day,&pRecordInfo->present_time,0);
    pRecordInfo->Zm=ZEN_FNTdiz(ZEN_FNCrtjul(&pRecordInfo->Tm),&longit,&pRecordInfo->latitude,&pRecordInfo->Azimuth);

    pRecordInfo->TimeDec=(double)pRecordInfo->present_time.ti_hour+pRecordInfo->present_time.ti_min/60.+pRecordInfo->present_time.ti_sec/3600.;

    tmLocal=pRecordInfo->Tm+THRD_localShift*3600.;
    pRecordInfo->localCalDay=ZEN_FNCaljda(&tmLocal);
    pRecordInfo->localTimeDec=fmod(pRecordInfo->TimeDec+24.+THRD_localShift,(double)24.);

   	for (i=0;i<NDET;i++)
   	 pBuffers->spectrum[i]=(double)spectrum[i];

   	if ((header.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_DARK) && (header.measurementType!=PRJCT_INSTR_MAXDOAS_TYPE_OFFSET))
   	 {
     	// Offset correction

   	  if (pBuffers->offset!=NULL)
   	   for (i=0;i<NDET;i++)
   	    pBuffers->spectrum[i]-=pBuffers->offset[i]*header.scansNumber;          // offset is already divided by its number of scans

   	  // Dark current correction                                                // dark current is already divided by it integration time

   	  if (pBuffers->varPix!=NULL)
   	   for (i=0;i<NDET;i++)
   	    pBuffers->spectrum[i]-=pBuffers->varPix[i]*header.scansNumber*header.exposureTime;

   	  // Average

  	   for (i=0;i<NDET;i++)
  	    pBuffers->spectrum[i]/=header.scansNumber;

      if ((dateFlag && ((pRecordInfo->elevationViewAngle>0.) && (pRecordInfo->elevationViewAngle<80.))) ||                    // reference spectra are zenith only
         (!dateFlag && pEngineContext->analysisRef.refScan && !pEngineContext->analysisRef.refSza && (pRecordInfo->elevationViewAngle>80.)))    // zenith sky spectra are not analyzed in scan reference selection mode

       rc=ERROR_ID_FILE_RECORD;
     }
    else
     rc=ERROR_ID_FILE_RECORD;
   }

  // Release allocated buffer

  if (spectrum!=NULL)
   MEMORY_ReleaseBuffer("MFCBIRA_Reli","spectrum",spectrum);



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

RC MFC_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  PRJCT_INSTRUMENTAL *pInstrumental;
  char fileName[MAX_STR_SHORT_LEN+1],*ptr;
  TBinaryMFC tbinaryRef;
  INDEX indexWindow,indexFeno,indexTabCross;                                    // indexes for loops and array
  CROSS_REFERENCE *pTabCross;                                                   // pointer to the current cross section
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a symbol
  FENO *pTabFeno;                                                               // pointer to the current spectral analysis window
  int DimL,useKurucz,saveFlag;                                                       // working variables
  RC rc;                                                                        // return code

  // Initializations

  pBuffers=&pEngineContext->buffers;

  saveFlag=(int)pEngineContext->project.spectra.displayDataFlag;
  pInstrumental=&pEngineContext->project.instrumental;
  memset(fileName,0,MAX_STR_SHORT_LEN+1);
  strncpy(fileName,pEngineContext->fileInfo.fileName,MAX_STR_SHORT_LEN);
  rc=ERROR_ID_NO;

  if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->refFlag && ((ptr=strrchr(fileName,PATH_SEP))!=NULL))
   {
    useKurucz=0;

    // Browse analysis windows and load missing data

    for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
     if (!TabFeno[0][indexFeno].hidden && !TabFeno[0][indexFeno].gomeRefFlag)
      {
       pTabFeno=&TabFeno[0][indexFeno];
       pTabFeno->NDET=NDET;
       FILES_RebuildFileName(ptr+1,pTabFeno->refFile,0);

       if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
        rc=MFC_ReadRecord(fileName,
                         &tbinaryRef,pTabFeno->Sref,
                         &MFC_headerDrk,pBuffers->varPix,
                         &MFC_headerOff,pBuffers->offset,
                         pInstrumental->mfcMaskSpec,pInstrumental->mfcMaskSpec,pInstrumental->mfcRevert);
    	 	else
        rc=MFC_ReadRecordStd(pEngineContext,fileName,
                            &tbinaryRef,pTabFeno->Sref,
                            &MFC_headerDrk,pBuffers->varPix,
                            &MFC_headerOff,pBuffers->offset);

       if (!rc && !(rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&pTabFeno->refNormFact,"MFC_LoadAnalysis (Reference) ")))
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

         doas_spectrum *new_range = spectrum_new();
         for (indexWindow = 0, DimL=0; indexWindow < pTabFeno->svd.Z; indexWindow++)
          {
           int pixel_start = FNPixel(pTabFeno->LambdaRef,pTabFeno->svd.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
           int pixel_end = FNPixel(pTabFeno->LambdaRef,pTabFeno->svd.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

           spectrum_append(new_range, pixel_start, pixel_end);

           DimL += pixel_end - pixel_start +1;
          }

         // Buffers allocation
         SVD_Free("MFC_LoadAnalysis",&pTabFeno->svd);
         pTabFeno->svd.DimL=DimL;
         SVD_LocalAlloc("MFC_LoadAnalysis",&pTabFeno->svd);
         // new spectral windows
         pTabFeno->svd.specrange = new_range;

         pTabFeno->Decomp=1;

         if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,0))!=ERROR_ID_NO) ||
             (!pKuruczOptions->fwhmFit && pTabFeno->xsToConvolute &&
             ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,&ANALYSIS_slit,&ANALYSIS_slit2,pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2,0,pSlitOptions->slitFunction.slitWveDptFlag))!=ERROR_ID_NO)))

          goto EndMFC_LoadAnalysis;
        }

       useKurucz+=pTabFeno->useKurucz;
      }

    // Wavelength calibration alignment

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
     {
      KURUCZ_Init(0,0);

      if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,responseHandle,0))!=ERROR_ID_NO))
       goto EndMFC_LoadAnalysis;
     }
   }

  // Return

  EndMFC_LoadAnalysis :

  return rc;
 }

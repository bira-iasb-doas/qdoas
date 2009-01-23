
//  ----------------------------------------------------------------------------
/*!
    \file    MKZY-READ.C
    \brief   MKZY (Manne Kihlman and Zhang Yan) read out routines.
    \details This module contains the routines needed to read data from PAK files written
             in a compressed file format created by MANNE Kihlman and ZHANG Yan, Chalmers,
             Goteborg, Sweden.  This file format is used by the NOVAC network.
    \authors Kihlman MANNE and Yan ZHANG, Chalmers, Goteborg, Sweden\n
             Adapted for QDOAS by Caroline FAYT (caroline.fayt@aeronomie.be)
    \date    14 January 2009
*/
//  ----------------------------------------------------------------------------
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
//  =========
//  FUNCTIONS
//  =========
//
//  MKZY_UnPack - decompresses a spectrum record in the MKZY file format;
//  MKZY_ParseDate - decompose a date in the MKZY file format;
//  MKZY_ParseTime - decompose a time in the MKZY file format;
//  MKZY_Set - calculate the number of records in a file in MKZY format;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"

// ====================
// CONSTANTS DEFINITION
// ====================

#define MAX_SPECTRUM_LENGTH 4096
#define HEADSIZ 12

// ====================
// STRUCTURE DEFINITION
// ====================

typedef struct MKZYhdr
 {
  char           ident[4];                                                      // "MKZY"
  unsigned short hdrsize;                                                       // this is the size in bytes of the header
  unsigned short hdrversion;                                                    // version of the header
  unsigned short size;                                                          // the number of bytes with compressed data
 }
MKZY_HEADER;

//! \struct MKZY_RECORDINFO
//! \brief Description of a record in the MKZY file format
//! \details Further details about fields can be found in a document written by Kihlman.

typedef struct MKZY_RECORDINFO
 {
 	        //! \details
 	        //! The checksum is calculated on the uncompressed data; it is used to check that the compression worked properly. \n
 	        //! The checksum can be calculated with the following routine in C :\n\n
 	        //! \code
 	        //!      MKZY_RECORDINFO mkzy;
 	        //!      long spec[];              // containing the spectral data points
 	        //!      int speclen;              // containing the number of datapoints stored in the previous vector
 	        //!      int i;
 	        //!      unsigned short checksum;
 	        //!      unsigned short *p;
 	        //!
 	        //!      checksum=0L;
 	        //!      for (i=0;i<speclen;i++)
 	        //!       checksum += spec[i];
 	        //!
 	        //!      p=(unsigned short *)&checksum;
 	        //!      mkzy.checksum=p[0]+p[1];
 	        //! \endcode

  unsigned short checksum;

          //! \details the name of this specific measurement
  char           name[12];
          //! \details the name of the instrument
  char           instrumentname[16];
          //! \details the startchannel for the first data-point
  unsigned short startc;
          //! \details number of pixels saved in the data-field
  unsigned short pixels;
          //! \details the viewing angle of the instrument
  short          viewangle;
          //! \details total number of scans added
  unsigned short scans;
          //! \details exposure time, negative if set automatic
  short          exptime;
          //! \details channel of the spectrometer, typically 0
  unsigned char  channel;
          //! \details for further use, currently contains the status of the solenoid(s) in bit 0 and 1
  unsigned char  flag;
          //! \details Date in \a DDMMYY format. Dates and times are given as GMT (Greenwitch Mean Time).\n
          //! Use \ref MKZY_ParseDate to decompose the date in separate year, month and day fields.
  unsigned long  date;
          //! \details time when the scanning was started
  unsigned long  starttime;
          //! \details time when the scanning was finished
  unsigned long  stoptime;
          //! \details GPS latitude in degrees
  double         lat;
          //! \details GPS longitude in degrees
  double         lon;
          //! \details new in version 2
  short          altitude;
          //! \details new in version 2, nr between 0 and measurecnt-1
  char           measureidx;
          //! \details new in version 2, number of MEAS= lines in cfg.txt
  char           measurecnt;
          //! \details new in version 3, direction of 2nd motor
  short          viewangle2;
          //! \details new in version 3, given in cfg.txt
  short          compassdir;
          //! \details new in version 3, given in cfg.txt
  short          tiltX;
          //! \details new in version 3, given in cfg.txt
  short          tiltY;
          //! \details new in version 3, given in cfg.txt
  float          temperature;
          //! \details new in version 4, given in cfg.txt
  char           coneangle;
          //! \details The voltages read by the ADC.  New in version 5.
  unsigned short ADC[8];
 }
MKZY_RECORDINFO;

// -----------------------------------------------------------------------------
// FUNCTION MKZY_UnPack
// -----------------------------------------------------------------------------
/*!
   \fn      long MKZY_UnPack(unsigned char *inpek,long kvar,long *ut)
   \details This function decompresses a spectrum record in the MKZY file format
   \param   [in]  inpek buffer with the compressed spectrum
   \param   [in]  kvar  the length of the uncompressed spectrum
   \param   [out] ut    buffer to which the data will be uncompressed
   \return  the number of bytes in the output buffer \a ut
*/
// -----------------------------------------------------------------------------

long MKZY_UnPack(unsigned char *inpek,long kvar,long *ut)
 {
 	// Declarations

  long *utpek=NULL;
  short len,curr;
  short j,jj;
  long a;
  unsigned short lentofile=0;
  long bit=0;

  // validate the input data - Added 2006.02.13 by MJ

  if (kvar>MAX_SPECTRUM_LENGTH)
   return -1;
  if((ut==NULL) || (inpek==NULL))
   return -1;

  utpek=ut;
  lentofile=0;

  while (kvar>0)
   {
    len=0;
    for(j=0;j<7;j++)
     {
      len+=len;
      len|=inpek[(bit>>3)]>>(7-(bit&0x7))&1;
      bit++;
     }
    curr=0;
    for(j=0;j<5;j++)
     {
      curr+=curr;
      curr|=inpek[(bit>>3)]>>(7-(bit&0x7))&1;
      bit++;
     }

    if(curr)
     {
      for(jj=0;jj<len;jj++)
       {
        a=inpek[(bit>>3)]>>(7-(bit&0x7))&1;
        if(a)
         a=-1;
        bit++;
        for(j=1;j<curr;j++)
         {
          a+=a;
          a|=inpek[(bit>>3)]>>(7-(bit&0x7))&1;
          bit++;
         }
        *utpek++=a;
       }
     }
    else
     {
      for(jj=0;jj<len;jj++)
       *utpek++=0;
     }

    kvar-=len;
    lentofile+=len;
   }

  for(jj=1;jj<lentofile;jj++)
   ut[jj]+=ut[jj-1];

  // Return

  return(lentofile);
 }

// -----------------------------------------------------------------------------
// FUNCTION MKZY_ParseDate
// -----------------------------------------------------------------------------
/*!
   \fn      void MKZY_ParseDate(unsigned long d,SHORT_DATE *pDate)
   \details Decompose a date in the MKZY file format.\n
            The date field in the header \ref MKZY_RECORDINFO::date is an unsigned long.\n
            Dates are represented on 6 digits in the \a DDMMYY file format.\n\n
   \par     Example:
            To represent 29 November 2005, the date field will have the decimal value 291105.
   \param   [in]    d  the date in DDMMYY format
   \param   [out]   pDate pointer to the SHORT_DATE structure with separate year, month and day fields
*/
// -----------------------------------------------------------------------------

void MKZY_ParseDate(unsigned long d,SHORT_DATE *pDate)
 {
  pDate->da_day=(char)(d/10000L);                                               // the day
  pDate->da_mon=(char)((d-(unsigned long)pDate->da_day*10000L)/100L);           // the month
  pDate->da_year=(short)(d%100);                                                // the year

  if(pDate->da_year<100)
   pDate->da_year+=2000;                                                        //assume the 21:st century (should be ok for another 95 years)
 }

// -----------------------------------------------------------------------------
// FUNCTION MKZY_ParseTime
// -----------------------------------------------------------------------------
/*!
   \fn      void MKZY_ParseTime(unsigned long t,struct time *pTime)
   \details Decompose a time in the MKZY file format.\n
            Time fields in the header are unsigned long numbers.\n
            They are represented on 8 digits in the \a hhmmssdd file format where \a dd are the decimal milliseconds.\n\n
   \par     Example:
            The decimal value 09350067 represents the time 09:35:00, 670 milliseconds in the morning.
   \param   [in]    t  the time in \a hhmmssdd (where \a dd are the decimal milliseconds)
   \param   [out]   pTime pointer to a \a struct \a time structure with separate hour, min and sec fields
*/
// -----------------------------------------------------------------------------

void MKZY_ParseTime(unsigned long t,struct time *pTime)
 {
  pTime->ti_hour=(unsigned char)(t/1000000L);
  pTime->ti_min=(unsigned char)((t-(unsigned long)pTime->ti_hour*1000000L)/10000L);
  pTime->ti_sec=(unsigned char)((t-(unsigned long)pTime->ti_hour*1000000L-(unsigned long)pTime->ti_min*10000L)/100L);
 }

// -----------------------------------------------------------------------------
// FUNCTION MKZY_ReadRecord
// -----------------------------------------------------------------------------
/*!
   \fn      RC MKZY_ReadRecord(ENGINE_CONTEXT *pEngineContext,int recordNo,FILE *specFp)
   \details read a specified record from a file in MKZY format.\n
   \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
   \param   [in]  recordNo        the index of the record to read
   \param   [in]  specFp          pointer to the spectra file to read
   \return  ERROR_ID_FILE_NOT_FOUND if the input file pointer \a specFp is NULL \n
            ERROR_ID_FILE_END if the end of the file is reached\n
            ERROR_ID_ALLOC if the allocation of a buffer failed\n
            ERROR_ID_BUFFER_FULL if the retrieved data are larger than the allocated buffers\n
            ERROR_ID_FILE_RECORD if the record doesn't satisfy user criteria\n
            ERROR_ID_NO on success
*/
// -----------------------------------------------------------------------------

RC MKZY_ReadRecord(ENGINE_CONTEXT *pEngineContext,int recordNo,FILE *specFp)
 {
  // Declarations

  BUFFERS        *pBuffers;                                                     // pointer to the buffers part of the engine context
  RECORD_INFO    *pRecord;                                                      // pointer to the record part of the engine context
  MKZY_HEADER     header;                                                       // record header
  MKZY_RECORDINFO recordInfo;                                                   // information on the record
  unsigned char  *buffer;                                                       // buffer for the spectrum before unpacking
  unsigned long  *lbuffer;                                                      // buffer for the spectrum after unpacking
  unsigned short  checksum,*p;                                                  // check sum
  unsigned long   chk;                                                          // for the calculation of the check sum
  int             npixels;                                                      // number of pixels returned
  double         *spectrum;                                                     // the current spectrum and its maximum value
  double          tmLocal;                                                      // measurement local time
  int             nsec;                                                         // the total number of seconds
  INDEX           i;                                                            // browse pixels in the spectrum
  RC              rc;                                                           // return code

  // Initializations

  pBuffers=&pEngineContext->buffers;
  pRecord=&pEngineContext->recordInfo;
  spectrum=(double *)pBuffers->spectrum;
  buffer=NULL;
  lbuffer=NULL;
  rc=ERROR_ID_NO;

  // Initialize the spectrum

 	for (i=0;i<NDET;i++)
 	 spectrum[i]=(double)0.;

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("MKZY_ReadRecord",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;
  else if (((buffer=MEMORY_AllocBuffer("MKZY_ReadRecord","buffer",pBuffers->recordIndexes[recordNo]-pBuffers->recordIndexes[recordNo-1],1,0,MEMORY_TYPE_STRING))==NULL) ||
           ((lbuffer=(unsigned long *)MEMORY_AllocBuffer("MKZY_ReadRecord","lbuffer",NDET,sizeof(DoasU32),0,MEMORY_TYPE_ULONG))==NULL))
   rc=ERROR_ID_ALLOC;
  else
   {
   	// Goto the requested record

    fseek(specFp,(DoasI32)pBuffers->recordIndexes[recordNo-1],SEEK_SET);

    // Read the first bytes (including MKZY sequence and the size of the header)

    fread(&header,sizeof(MKZY_HEADER),1,specFp);
    fread(&recordInfo,min(sizeof(MKZY_RECORDINFO),header.hdrsize-sizeof(MKZY_HEADER)),1,specFp);

    strncpy(pRecord->Nom,recordInfo.name,12);                                   // the name of this specific measurement

    pRecord->NSomme=recordInfo.scans;                                           // total number of scans added
    pRecord->Tint=(double)fabs((double)recordInfo.exptime);                     // exposure time, negative if set automatic
    pRecord->longitude=(double)recordInfo.lon;                                  // GPS longitude in degrees
    pRecord->latitude=(double)recordInfo.lat;                                   // GPS latitude in degrees

    strcpy(pRecord->mkzy.instrumentname,recordInfo.instrumentname);             // the name of the instrument

    pRecord->mkzy.startc=recordInfo.startc;                                     // the startchannel for the first data-point
    pRecord->mkzy.pixels=recordInfo.pixels;                                     // number of pixels saved in the data-field
    pRecord->mkzy.channel=recordInfo.channel;                                   // channel of the spectrometer, typically 0
    pRecord->mkzy.coneangle=recordInfo.coneangle;                               // new in version 4, given in cfg.txt

    pRecord->mkzy.scanningAngle=(double)recordInfo.viewangle;                   // the viewing angle of the instrument
    pRecord->mkzy.scanningAngle2=(double)recordInfo.viewangle2;                 // the viewing angle of the 2nd instrument

    if (pRecord->mkzy.scanningAngle>180.)
     pRecord->mkzy.scanningAngle-=360.;
    if (pRecord->mkzy.scanningAngle2>180.)
     pRecord->mkzy.scanningAngle-=360.;

    MKZY_ParseDate(recordInfo.date,&pRecord->present_day);

    MKZY_ParseTime(recordInfo.starttime,&pRecord->startTime);
    MKZY_ParseTime(recordInfo.stoptime,&pRecord->endTime);

    nsec=(pRecord->startTime.ti_hour*3600+pRecord->startTime.ti_min*60+pRecord->startTime.ti_sec+
          pRecord->endTime.ti_hour*3600+pRecord->endTime.ti_min*60+pRecord->endTime.ti_sec)/2;

    pRecord->present_time.ti_hour=(unsigned char)(nsec/3600);
    pRecord->present_time.ti_min=(unsigned char)((nsec%3600)/60);
    pRecord->present_time.ti_sec=(unsigned char)((nsec%3600)%60);

    pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);
    pRecord->TotalExpTime=(double)0.;
    pRecord->TimeDec=(double)pRecord->present_time.ti_hour+pRecord->present_time.ti_min/60.+pRecord->present_time.ti_sec/3600.;
    pRecord->Zm=ZEN_FNTdiz(ZEN_FNCrtjul(&pRecord->Tm),&pRecord->longitude,&pRecord->latitude,&pRecord->Azimuth);

    tmLocal=pRecord->Tm+THRD_localShift*3600.;

    pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
    pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

    if (header.hdrversion>=2)
     pRecord->altitude=(double)recordInfo.altitude;                             // new in version 2

    // number of MEAS= lines in cfg.txt

    if (header.hdrversion>=3)
     pRecord->TDet=(double)recordInfo.temperature;                              // new in version 3, given in cfg.txt

    // Be sure to be at the beginning of the data

    fseek(specFp,(DoasI32)pBuffers->recordIndexes[recordNo-1]+header.hdrsize,SEEK_SET);

    // Read compressed data and uncompress them

    fread(buffer,pBuffers->recordIndexes[recordNo]-pBuffers->recordIndexes[recordNo-1]-header.hdrsize,1,specFp);

    if ((recordInfo.pixels>NDET) || ((npixels=MKZY_UnPack(buffer,recordInfo.pixels,(long *)lbuffer))<0))
     rc=ERROR_SetLast("MKZY_ReadRecord",ERROR_TYPE_WARNING,ERROR_ID_BUFFER_FULL,"spectra");
    else
     {
      // calculate the checksum

      for (chk=0L,i=0;i<npixels;i++)
       chk+=lbuffer[i];

      p=(unsigned short *)&chk;
      checksum=(unsigned short)p[0]+p[1];

      if (checksum!=recordInfo.checksum)
       rc=ERROR_ID_FILE_RECORD;
      else
       for (i=0;i<npixels;i++)
        spectrum[i]=(double)lbuffer[i];
     }
   }

  // Release allocated buffers

  if (buffer!=NULL)
   MEMORY_ReleaseBuffer("MKZY_ReadRecord","buffer",buffer);
  if (lbuffer!=NULL)
   MEMORY_ReleaseBuffer("MKZY_ReadRecord","lbuffer",lbuffer);

  // Return

  return rc;
 }

RC MKZY_SearchForOffset(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
 	// Declarations

 	INDEX indexRecord;
 	RC rc;

 	// Initializations

 	pEngineContext->recordInfo.mkzy.darkFlag=0;
 	rc=ERROR_ID_NO;

 	// Search for the spectrum

  for (indexRecord=1;indexRecord<=pEngineContext->recordNumber;indexRecord++)
   if (!(rc=MKZY_ReadRecord(pEngineContext,indexRecord,specFp)) && !stricmp(pEngineContext->recordInfo.Nom,"dark"))
  	 {
  	 	memcpy(pEngineContext->buffers.darkCurrent,pEngineContext->buffers.spectrum,sizeof(double)*NDET);
  	 	pEngineContext->recordInfo.mkzy.darkFlag=1;
  	 	pEngineContext->recordInfo.mkzy.darkScans=pEngineContext->recordInfo.NSomme;
  	 	break;
  	 }

 	// Return

 	return rc;
 }

RC MKZY_SearchForSky(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
 	// Declarations

 	INDEX indexRecord;
 	INDEX i;
 	RC rc;

 	// Initializations

 	pEngineContext->recordInfo.mkzy.skyFlag=0;
 	rc=ERROR_ID_NO;

 	// Search for the spectrum

  for (indexRecord=1;indexRecord<=pEngineContext->recordNumber;indexRecord++)
   if (!(rc=MKZY_ReadRecord(pEngineContext,indexRecord,specFp)) && !stricmp(pEngineContext->recordInfo.Nom,"sky"))
  	 {
  	 	memcpy(pEngineContext->buffers.scanRef,pEngineContext->buffers.spectrum,sizeof(double)*NDET);
  	 	pEngineContext->recordInfo.mkzy.skyFlag=1;

  	 	// Correct by the dark current

  	  if (pEngineContext->recordInfo.mkzy.darkFlag)
      for (i=0;i<NDET;i++)
       pEngineContext->buffers.scanRef[i]-=(double)pEngineContext->buffers.darkCurrent[i]*pEngineContext->recordInfo.NSomme/pEngineContext->recordInfo.mkzy.darkScans;

  	 	break;
  	 }

 	// Return

 	return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION MKZY_Set
// -----------------------------------------------------------------------------
/*!
   \fn      RC MKZY_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
   \details calculate the number of records in a file in MKZY format.\n
   \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
   \param   [in]  specFp pointer to the spectra file to read
   \return  ERROR_ID_FILE_NOT_FOUND if the input file pointer \a specFp is NULL \n
            ERROR_ID_FILE_EMPTY if the file is empty\n
            ERROR_ID_ALLOC if the allocation of a buffer failed\n
            ERROR_ID_NO on success
*/
// -----------------------------------------------------------------------------

RC MKZY_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  DoasU32 *recordIndexes;                                                       // save the position of each record in the file
  unsigned char *buffer,*ptr;                                                   // buffer to load the file
  SZ_LEN fileLength;                                                            // the length of the file to load
  INDEX indexRecord;                                                            // browse records
  INDEX i;                                                                      // browse pixels of the detector
  RC rc;                                                                        // return code

  // Initializations

  buffer=NULL;

  pEngineContext->recordNumber=0;
  pBuffers=&pEngineContext->buffers;
  pEngineContext->recordIndexesSize=2001;
  recordIndexes=pBuffers->recordIndexes;

  pEngineContext->recordInfo.mkzy.darkFlag=pEngineContext->recordInfo.mkzy.skyFlag=0;

  rc=ERROR_ID_NO;

  // Get the number of spectra in the file

  if (specFp==NULL)
   rc=ERROR_SetLast("MKZY_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if (!(fileLength=STD_FileLength(specFp)))
   rc=ERROR_SetLast("MKZY_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
  else if ((buffer=MEMORY_AllocBuffer("MKZY_Set","buffer",fileLength,1,0,MEMORY_TYPE_STRING))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
   	// load the buffer in one operation

   	fread(buffer,fileLength,1,specFp);

   	// search for the "magic" sequence of characters : MKZY

   	for (ptr=buffer;(ptr-buffer<fileLength-4) && (pEngineContext->recordNumber<pEngineContext->recordIndexesSize);ptr++)
   	 if ((ptr[0]=='M') && (ptr[1]=='K') && (ptr[2]=='Z') && (ptr[3]=='Y'))
  	  	recordIndexes[pEngineContext->recordNumber++]=ptr-buffer;

  	 recordIndexes[pEngineContext->recordNumber]=fileLength;

    if (!(rc=MKZY_SearchForOffset(pEngineContext,specFp)))
  	  rc=MKZY_SearchForSky(pEngineContext,specFp);
   }

  // Release allocated buffers

  if (buffer!=NULL)
   MEMORY_ReleaseBuffer("MKZY_Set","buffer",buffer);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION MKZY_Reli
// -----------------------------------------------------------------------------
/*!
   \fn      RC MKZY_Reli(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,INT localDay,FILE *specFp)
   \details call \ref MKZY_ReadRecord to read a specified record from a file in MKZY format and check that it is a spectrum to analyze (spectrum name should be 'other').\n
   \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
   \param   [in]  recordNo        the index of the record to read
   \param   [in]  dateFlag        1 to search for a reference spectrum; 0 otherwise
   \param   [in]  localDay        if \a dateFlag is 1, the calendar day for the reference spectrum to search for
   \param   [in]  specFp          pointer to the spectra file to read
   \return  the code returned by \ref MKZY_ReadRecord \n
            ERROR_ID_FILE_RECORD if the record is the spectrum is not a spectrum to analyze (sky or dark spectrum)\n
            ERROR_ID_NO on success
*/
// -----------------------------------------------------------------------------

RC MKZY_Reli(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,INT localDay,FILE *specFp)
 {
  // Declarations

  double *spectrum,*offset;                                                     // pointer to spectrum and offset
  INDEX   i;                                                                    // browse pixels of the detector
  RC      rc;                                                                   // return code

  // Initializations

  spectrum=pEngineContext->buffers.spectrum;
  offset=pEngineContext->buffers.darkCurrent;

  if (!(rc=MKZY_ReadRecord(pEngineContext,recordNo,specFp)))
   {
   	if (stricmp(pEngineContext->recordInfo.Nom,"other"))
     rc=ERROR_ID_FILE_RECORD;

    // Correction by offset

    else if ((offset!=NULL) && (pEngineContext->recordInfo.mkzy.darkScans>0))
     {
      for (i=0;i<NDET;i++)
       spectrum[i]-=(double)offset[i]*pEngineContext->recordInfo.NSomme/pEngineContext->recordInfo.mkzy.darkScans;
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION MKZY_LoadAnalysis
// -----------------------------------------------------------------------------
/*!
   \fn      RC MKZY_LoadAnalysis(ENGINE_CONTEXT *pEngineContext)
   \details as the reference spectrum is retrieved from spectra files, calibration
            has to be applied on each file\n
   \param   [in]  pEngineContext  pointer to the engine context
   \return  error code returned by one of the child function on error\n
            ERROR_ID_NO on success
*/
// -----------------------------------------------------------------------------

RC MKZY_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  PRJCT_INSTRUMENTAL *pInstrumental;
  INDEX indexWindow,indexFeno,indexTabCross;                                    // indexes for loops and array
  CROSS_REFERENCE *pTabCross;                                                   // pointer to the current cross section
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a symbol
  FENO *pTabFeno;                                                               // pointer to the current spectral analysis window
  double factTemp;                                                              // working variables
  INT DimL,useKurucz,saveFlag;                                                  // working variables
  RC rc;                                                                        // return code

  // Initializations

  pBuffers=&pEngineContext->buffers;

  saveFlag=(INT)pEngineContext->project.spectra.displayDataFlag;
  pInstrumental=&pEngineContext->project.instrumental;
  rc=ERROR_ID_NO;

  if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->recordInfo.mkzy.skyFlag && pEngineContext->refFlag)
   {
    useKurucz=0;

    // Browse analysis windows and load missing data

    for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
     {
      if (!TabFeno[indexFeno].hidden && !TabFeno[indexFeno].gomeRefFlag)
       {
        pTabFeno=&TabFeno[indexFeno];
        pTabFeno->NDET=NDET;

        memcpy(pTabFeno->Sref,pBuffers->scanRef,sizeof(double)*NDET);

        if (!rc && !(rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&factTemp,"MKZY_LoadAnalysis (Reference) ")))
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
                 ((rc=ANALYSE_CheckLambda(pWrkSymbol,pTabFeno->LambdaRef,"MKZY_LoadAnalysis "))!=ERROR_ID_NO))

             goto EndMKZY_LoadAnalysis;
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

          SVD_Free("MKZY_LoadAnalysis",&pTabFeno->svd);
          SVD_LocalAlloc("MKZY_LoadAnalysis",&pTabFeno->svd);

          pTabFeno->Decomp=1;

          if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef))!=ERROR_ID_NO) ||
              (!pKuruczOptions->fwhmFit && pTabFeno->xsToConvolute &&
              ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,&ANALYSIS_slit,pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2,&pSlitOptions->slitFunction.slitParam3,&pSlitOptions->slitFunction.slitParam4))!=ERROR_ID_NO)))

           goto EndMKZY_LoadAnalysis;
         }

        useKurucz+=pTabFeno->useKurucz;
       }
     }

    // Wavelength calibration alignment

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
     {
      KURUCZ_Init(0);

      if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,responseHandle))!=ERROR_ID_NO))
       goto EndMKZY_LoadAnalysis;
     }
   }

  // Return

  EndMKZY_LoadAnalysis :

  return rc;
 }

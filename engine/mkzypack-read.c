
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  MKZY PACK read out routines
//  Name of module    :  MKZYPACK-READ.C
//  Creation date     :  14 January 2009
//
//  Authors           :  Kihlman MANNE and Yan ZHANG, Chalmers, Goteborg, Sweden
//  Adaptations       :  Caroline FAYT (caroline.fayt@aeronomie.be)
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
//  This module contains the routines needed to read data from PAK files written
//  in a compressed file format created by MANNE Kihlman and ZHANG Yan, Chalmers,
//  Goteborg, Sweden.  This file format is used by the NOVAC network.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
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

typedef struct MKZYRecordInfo
 {
  unsigned short checksum;                                                      // checksum for the uncompressed data
  char           name[12];                                                      // the name of this specific measurement
  char           instrumentname[16];                                            // the name of the instrument
  unsigned short startc;                                                        // the startchannel for the first data-point
  unsigned short pixels;                                                        // number of pixels saved in the data-field
  short          viewangle;                                                     // the viewing angle of the instrument
  unsigned short scans;                                                         // total number of scans added
  short          exptime;                                                       // exposure time, negative if set automatic
  unsigned char  channel;                                                       // channel of the spectrometer, typically 0
  unsigned char  flag;                                                          // for further use, currently contains the status of the solenoid(s) in bit 0 and 1
  unsigned long  date;                                                          // date
  unsigned long  starttime;                                                     // time when the scanning was started
  unsigned long  stoptime;                                                      // time when the scanning was finished
  double         lat;                                                           // GPS latitude in degrees
  double         lon;                                                           // GPS longitude in degrees
  short          altitude;                                                      // new in version 2
  char           measureidx;                                                    // new in version 2, nr between 0 and measurecnt-1
  char           measurecnt;                                                    // new in version 2, number of MEAS= lines in cfg.txt
  short          viewangle2;                                                    //new in version 3, direction of 2nd motor
  short          compassdir;                                                    //new in version 3, given in cfg.txt
  short          tiltX;                                                         //new in version 3, given in cfg.txt
  short          tiltY;                                                         //new in version 3, given in cfg.txt
  float          temperature;                                                   //new in version 3, given in cfg.txt
  char           coneangle;                                                     //new in version 4, given in cfg.txt
  unsigned short ADC[8];                                                        //new in version 5
 }
MKZY_RECORDINFO;

// -----------------------------------------------------------------------------
// FUNCTION      MKZY_UnPack
// -----------------------------------------------------------------------------
// PURPOSE       Decompresses the spectrum
//
// INPUT         inpek - buffer with the compressed spectrum
//               kvar - the length of the uncompressed spectrum
//
// OUTPUT        ut - buffer to which the data will be uncompressed
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

//
// RETURN_CODE CSpectrumIO::ReadSpectrum(const CString &fileName, const int
// spectrumNumber, CSpectrum &spec, char *headerBuffer /*=NULL*/, int
// headerBufferSize /*=0*/, int *headerSize /*=NULL*/){
//          CString errorMessage;// a string used for error messages
//          MKPack mkPack;
//
//          long i,j;
//          long outlen;
//          unsigned long chk;
//          unsigned short checksum;
//          int hdrSize;
//
//          unsigned short *p=NULL;
//
//          i=0;
//          FILE *f=fopen(fileName, "rb");
//
//          if(f == NULL){
//                  errorMessage.Format("Could not open spectrum file: %s",
//  fileName);
//                  ShowMessage(errorMessage);
//                  m_lastError=ERROR_COULD_NOT_OPEN_FILE;
//                  return FAIL;
//          }
//
//          while(1)
//          {
//                  int ret;
//                  if(headerBuffer != NULL)
//                          ret=ReadNextSpectrumHeader(f, *headerSize,
//  &spec, headerBuffer, headerBufferSize);
//                  else
//                          ret=ReadNextSpectrumHeader(f, hdrSize, &spec);
//                  if(ret == 1)
//                          break;
//                  if(ret == 2)
//                          continue;
//
//                  if(i != spectrumNumber){
//                          char textBuffer[4];
//
//                          // Seek our way into the next spectrum...
//                          if(0 != fseek(f, min(MKZY.size,
//  4*MAX_SPECTRUM_LENGTH), SEEK_CUR))
//                                  break;
//
//                          // Make sure we're at the right place, if not
//  rewind again and search for the next
//                          // occurence of the "MKZY" string, which signals
//  the start of a 'new' spectrum.
//                          if(fread(textBuffer, 1, 4, f)<4)
//                                  break;
//                          if(NULL == strstr(textBuffer, "MKZY")){
//                                  // rewind
//                                  if(0 != fseek(f, -min(MKZY.size,
//  4*MAX_SPECTRUM_LENGTH), SEEK_CUR))
//                                          break;
//                          }else{
//                                  if(0 != fseek(f, -4, SEEK_CUR))
//                                          break;
//                          }
//
//                          ++i;
//                          continue;
//                  }
//                  else
//                  {
//                          // read the spectrum from the file
//
//                          if(MKZY.size > sizeof(buffer)){
//                                  // compressed data is too long. We cannot
//  read the full spectrum.
//                                  m_lastError=ERROR_SPECTRUM_TOO_LARGE;
//                                  fclose(f);
//                                  return FAIL;
//                          }
//
//                          if(fread(buffer,1,MKZY.size,f)<MKZY.size)
//  //read compressed info
//                          {
//                                  errorMessage.Format("Error EOF! in %s",
//  fileName);
//                                  ShowMessage(errorMessage);
//                                  fclose(f);
//                                  m_lastError=ERROR_EOF;
//                                  return FAIL;
//                          }
//
//                          if(MKZY.pixels > sizeof(outbuf)*sizeof(long)){
//                                  // The spectrum is longer than what the
//  buffer can handle. Trying to
//                                  // uncompress the whole spectrum will
//  result in a buffer overflow.
//                                  // this spectrum cannot be read - return.
//                                  m_lastError=ERROR_SPECTRUM_TOO_LARGE;
//                                  fclose(f);
//                                  return FAIL;
//                          }
//
//                          outlen=mkPack.UnPack(buffer,MKZY.pixels,
//  outbuf);//uncompress info(compressed buffer,num of sampling points,
//  uncompressedinfo)
//
//                          // validate that the decompression was ok - Added
//  2006.02.13 by MJ
//                          if(outlen<0){
//                                  m_lastError =ERROR_DECOMPRESS;
//                                  fclose(f);
//                                  return FAIL;
//                          }
//
//                          // validate that the spectrum is not too large -
//  Added 2006.02.13 by MJ
//                          if(outlen > MAX_SPECTRUM_LENGTH){
//                                  m_lastError=ERROR_SPECTRUM_TOO_LARGE;
//                                  fclose(f);
//                                  return FAIL;
//                          }
//
//                          // calculate the checksum
//                  chk=0;
//                  for(j=0;j<outlen && j<MAX_SPECTRUM_LENGTH;j++)
//                  {
//                          chk+=outbuf[j];
//                          spec.m_data[j]=outbuf[j];
//                  }
//                  p=(unsigned short *)&chk;
//                  checksum=p[0] + p[1];
//                  if(checksum != MKZY.checksum){
//                          errorMessage.Format("Checksum mismatch
//  %04x!=x%04x\n",checksum,MKZY.checksum);
//                          ShowMessage(errorMessage);
//
//                          m_lastError=ERROR_CHECKSUM_MISMATCH;
//                          fclose(f);
//                          return FAIL;
//                  }else{
//                          printf("Checksum is correct
//  0x%04x=0x%04x\n",checksum,MKZY.checksum);
//                  }
//
//                          // Get the maximum intensity
//                          if(MKZY.pixels > 0){
//                                  spec.m_info.m_peakIntensity =
//  (float)spec.MaxValue();
//                                  spec.m_info.m_offset        =
//  (float)spec.GetOffset();
//                          }
//
//                  fclose(f);
//
//                  return SUCCESS;
//                  }
//          }
//          multisize=i;
//          fclose(f);
//
//          this->m_lastError=ERROR_SPECTRUM_NOT_FOUND;
//          return FAIL;// spectrum not found
// }
//
// /** Rewinds the gien file to the beginning and forwards the current position
//                  in the file to the beginning of spectrum number
//  'spectrumNumber' (zero-based index).
//                  Return SUCCESS if all is ok, return FAIL if the file is
//  corrupt in some
//                          way or the spectrum number 'spectrumNumber' does
//  not exist in this file. */
//
// RETURN_CODE CSpectrumIO::ReadNextSpectrum(FILE *f, CSpectrum &spec){
//          int tmp;
//          return ReadNextSpectrum(f, spec, tmp);
// }
//
// /** Reads the next spectrum in the provided spectrum file.
//                  The spectrum file (which must be in the .pak format) must
//  be opened for reading
//                  in binary mode
//      @param f - The opened spectrum file.
//      @param spec - Will on successful return contain the desired spectrum.
//      @return SUCCESS if all is ok. */
// RETURN_CODE CSpectrumIO::ReadNextSpectrum(FILE *f, CSpectrum &spec, int
// &headerSize, char *headerBuffer, int headerBufferSize){
//          CString errorMessage;// a string used for error messages
//          long outlen;
//          long j;
//          unsigned long chk;
//          unsigned short checksum;
//          MKPack mkPack;
//
//          unsigned short *p=NULL;
//
//
//
//          // Decompress the spectrum itself
//          outlen=mkPack.UnPack(buffer,MKZY.pixels, outbuf);//uncompress
//  info(compressed buffer,num of sampling points, uncompressedinfo)
//
//          // validate that the decompression was ok - Added 2006.02.13 by MJ
//          if(outlen<0){
//                  this->m_lastError =ERROR_DECOMPRESS;
//                  return FAIL;
//          }
//          // validate that the spectrum is not too large - Added 2006.02.13
//  by MJ
//          if(outlen > MAX_SPECTRUM_LENGTH){
//                  this->m_lastError=ERROR_SPECTRUM_TOO_LARGE;
//                  return FAIL;
//          }
//
//          // calculate the checksum
//          chk=0;
//          for(j=0;j<outlen && j<MAX_SPECTRUM_LENGTH;j++)
//          {
//                  chk+=outbuf[j];
//          }
//          p=(unsigned short *)&chk;
//          checksum=p[0] + p[1];
//          if(checksum != MKZY.checksum){
//                  errorMessage.Format("Checksum mismatch
//  %04x!=x%04x\n",checksum,MKZY.checksum);
//                  ShowMessage(errorMessage);
//
//                  this->m_lastError=ERROR_CHECKSUM_MISMATCH;
//                  return FAIL;
//          }else{
//                  printf("Checksum is correct
//  0x%04x=0x%04x\n",checksum,MKZY.checksum);
//          }
//
//          // copy the spectrum
//          for(j=0;j<outlen && j<MAX_SPECTRUM_LENGTH;j++)
//                  spec.m_data[j]=outbuf[j];
//
//
//          // Get the maximum intensity
//          if(MKZY.pixels > 0){
//                  spec.m_info.m_peakIntensity=(float)spec.MaxValue();
//                  spec.m_info.m_offset       =(float)spec.GetOffset();
//          }
//
//          return SUCCESS;
// }
//
//

// -----------------------------------------------------------------------------
// FUNCTION      MKZY_ParseDate
// -----------------------------------------------------------------------------
// PURPOSE       Decompose a date
//
// INPUT         d : the date in DDMMYY
//
// OUTPUT        pDate : pointer to the date with year, month and day fields
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
// FUNCTION      MKZY_ParseTime
// -----------------------------------------------------------------------------
// PURPOSE       Decompose a time
//
// INPUT         t : the time in hhmmssxx (where xx holds for ms)
//
// OUTPUT        pTime : pointer to the time with hour, min, sec
// -----------------------------------------------------------------------------

void MKZY_ParseTime(unsigned long t,struct time *pTime)
 {
  pTime->ti_hour=(unsigned char)(t/1000000L);
  pTime->ti_min=(unsigned char)((t-(unsigned long)pTime->ti_hour*1000000L)/10000L);
  pTime->ti_sec=(unsigned char)((t-(unsigned long)pTime->ti_hour*1000000L-(unsigned long)pTime->ti_min*10000L)/100L);
 }

// -----------------------------------------------------------------------------
// FUNCTION      SetMKZYPack
// -----------------------------------------------------------------------------
// PURPOSE       calculate the number of records for a new file
//               in MKZY Pack format
//
// INPUT         pEngineContext : information on the file to read
//               specFp    : pointer to the spectra file to read;
//
// OUTPUT        pEngineContext->recordNumber, the number of records
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC SetMKZYPack(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  BUFFERS *pBuffers;                                                           // pointer to the buffers part of the engine context
  DoasU32 *recordIndexes;                                                      // save the position of each record in the file
  unsigned char *buffer,*ptr;                                                  // buffer to load the file
  SZ_LEN fileLength;                                                           // the length of the file to load
  RC rc;                                                                       // return code

  // Initializations

  buffer=NULL;

  pEngineContext->recordNumber=0;
  pBuffers=&pEngineContext->buffers;
  pEngineContext->recordIndexesSize=2001;
  recordIndexes=pBuffers->recordIndexes;
  rc=ERROR_ID_NO;

  // Get the number of spectra in the file

  if (specFp==NULL)
   rc=ERROR_SetLast("SetMKZYPack",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if (!(fileLength=STD_FileLength(specFp)))
   rc=ERROR_SetLast("SetMKZYPack",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
  else if ((buffer=MEMORY_AllocBuffer("SetMKZYPack","buffer",fileLength,1,0,MEMORY_TYPE_STRING))==NULL)
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
   }

  // Release allocated buffers

  if (buffer!=NULL)
   MEMORY_ReleaseBuffer("SetMKZYPack","buffer",buffer);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliMKZYPack
// -----------------------------------------------------------------------------
// PURPOSE       Read spectra in MKZY PACK format
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
// RETURN        ERROR_ID_FILE_NOT_FOUND : the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

RC ReliMKZYPack(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,INT localDay,FILE *specFp)
 {
  // Declarations

  BUFFERS        *pBuffers;                                                    // pointer to the buffers part of the engine context
  RECORD_INFO    *pRecord;                                                     // pointer to the record part of the engine context
  MKZY_HEADER     header;                                                      // record header
  MKZY_RECORDINFO recordInfo;                                                  // information on the record
  int             headerSize;                                                  // real size of the header
  unsigned char  *buffer;                                                      // buffer for the spectrum
  double         *spectrum,SMax;                                               // the current spectrum and its maximum value
  double          tmLocal;                                                     // measurement local time
  SHORT_DATE      today;                                                       // date of the current measurement
  int             nsec;                                                        // the total number of seconds
  INDEX           i;                                                           // browse pixels in the spectrum
  RC              rc;                                                          // return code

  // Initializations

  pBuffers=&pEngineContext->buffers;
  pRecord=&pEngineContext->recordInfo;
  spectrum=(double *)pBuffers->spectrum;
  buffer=NULL;
  SMax=(double)0.;
  rc=ERROR_ID_NO;

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliMKZYPack",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;
  else if ((buffer=MEMORY_AllocBuffer("ReliMKZYPack","buffer",pBuffers->recordIndexes[recordNo]-pBuffers->recordIndexes[recordNo-1],1,0,MEMORY_TYPE_STRING))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
   	// Goto the requested record

    fseek(specFp,(DoasI32)pBuffers->recordIndexes[recordNo-1],SEEK_SET);

    // Read the first bytes (including MKZY sequence and the size of the header)

    fread(&header,sizeof(MKZY_HEADER),1,specFp);
    fread(&recordInfo,min(sizeof(MKZY_RECORDINFO),header.hdrsize-sizeof(MKZY_HEADER)),1,specFp);

    // Be sure to be at the beginning of the data

    fseek(specFp,(DoasI32)pBuffers->recordIndexes[recordNo-1]+header.hdrsize,SEEK_SET);

    // Read compressed data

    fread(buffer,pBuffers->recordIndexes[recordNo]-pBuffers->recordIndexes[recordNo-1]-header.hdrsize,1,specFp);

    strncpy(pRecord->Nom,recordInfo.name,12);                                  // the name of this specific measurement

    pRecord->NSomme=recordInfo.scans;                                          // total number of scans added
    pRecord->Tint=(double)fabs((double)recordInfo.exptime);                    // exposure time, negative if set automatic
    pRecord->longitude=(double)recordInfo.lon;                                 // GPS longitude in degrees
    pRecord->latitude=(double)recordInfo.lat;                                  // GPS latitude in degrees

    strcpy(pRecord->mkzy.instrumentname,recordInfo.instrumentname);            // the name of the instrument

    pRecord->mkzy.startc=recordInfo.startc;                                    // the startchannel for the first data-point
    pRecord->mkzy.pixels=recordInfo.pixels;                                    // number of pixels saved in the data-field
    pRecord->mkzy.channel=recordInfo.channel;                                  // channel of the spectrometer, typically 0
    pRecord->mkzy.coneangle=recordInfo.coneangle;                              // new in version 4, given in cfg.txt

    pRecord->mkzy.scanningAngle=(double)recordInfo.viewangle;                  // the viewing angle of the instrument
    pRecord->mkzy.scanningAngle2=(double)recordInfo.viewangle2;                // the viewing angle of the 2nd instrument

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
     pRecord->altitude=(double)recordInfo.altitude;                            // new in version 2

    // number of MEAS= lines in cfg.txt

    if (header.hdrversion>=3)
     pRecord->TDet=(double)recordInfo.temperature;                             // new in version 3, given in cfg.txt

 	  for (i=0;i<NDET;i++)
 	   spectrum[i]=(double)1.;
   }

  // Release allocated buffer

  if (buffer!=NULL)
   MEMORY_ReleaseBuffer("ReliMKZYPack","buffer",buffer);

  // Return

  return rc;
 }

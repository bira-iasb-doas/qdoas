
//  ----------------------------------------------------------------------------
/*!
    \file    bira-airborne-read.C
    \brief   Format developed at BIRA-IASB for airborne measurements
    \details This module contains the routines needed to read data measured
             from aircraft and ULM
    \authors Caroline FAYT (caroline.fayt@aeronomie.be)
    \date    13 March 2009
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
//
//  AIRBORNE_Set - calculate the number of records in a file in the format
//                 developed at BIRA-IASB for airborne measurements;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"
#include "stdfunc.h"
#include <string.h>
#include <math.h>

// ====================
// CONSTANTS DEFINITION
// ====================

#define IGNORED_BYTES 144

// ====================
// STRUCTURE DEFINITION
// ====================

#pragma pack(1)

typedef struct _airborneData
 {
  SHORT_DATE  today;                                                            // measurement date and time
  struct time now;
  int         averagedSpectra;                                                  // number of averaged spectra
  int         totalTime;                                                        // the total measurement time
  int         nrejMeas,naccMeas;                                                // resp. number of rejected spectra and number of accumulation
  float       longitude,latitude,altitude;                                      // GPS data
  float       exposureTime;                                                     // exposure time (in milliseconds)
  struct time gpsTime;
  unsigned char floatflag;
  unsigned char sentPosition;
  unsigned char receivedPosition;
  unsigned char ignoredChar;
  float         temperature;
  float         pressure;
  unsigned char ignoredBytes[IGNORED_BYTES];
 }
AIRBORNE_DATA;

// -----------------------------------------------------------------------------
// FUNCTION AIRBORNE_Set
// -----------------------------------------------------------------------------
/*!
   \fn      RC AIRBORNE_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
   \details calculate the number of records in a file in the format developed at BIRA-IASB for airborne measurements.\n
   \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
   \param   [in]  specFp pointer to the spectra file to read
   \return  ERROR_ID_FILE_NOT_FOUND if the input file pointer \a specFp is NULL \n
            ERROR_ID_FILE_EMPTY if the file is empty\n
            ERROR_ID_ALLOC if the allocation of a buffer failed\n
            ERROR_ID_NO on success
*/
// -----------------------------------------------------------------------------

RC AIRBORNE_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  SZ_LEN fileLength;                                                            // the length of the file to load
  RC rc;                                                                        // return code

  // Initializations

  pEngineContext->recordNumber=0;
  rc=ERROR_ID_NO;

  // Get the number of spectra in the file

  if (specFp==NULL)
   rc=ERROR_SetLast("AIRBORNE_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if (!(fileLength=STD_FileLength(specFp)))
   rc=ERROR_SetLast("AIRBORNE_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
  else
   pEngineContext->recordNumber=fileLength/(sizeof(AIRBORNE_DATA)+sizeof(double)*NDET);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION AIRBORNE_Reli
// -----------------------------------------------------------------------------
/*!
   \fn      RC AIRBORNE_Reli(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
   \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.
   \param   [in]  recordNo        the index of the record to read
   \param   [in]  dateFlag        1 to search for a reference spectrum; 0 otherwise
   \param   [in]  localDay        if \a dateFlag is 1, the calendar day for the reference spectrum to search for
   \param   [in]  specFp          pointer to the spectra file to read
   \return  the code returned by \ref AIRBORNE_ReadRecord \n
            ERROR_ID_FILE_RECORD if the record is the spectrum is not a spectrum to analyze (sky or dark spectrum)\n
            ERROR_ID_NO on success
*/
// -----------------------------------------------------------------------------

RC AIRBORNE_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  RECORD_INFO   *pRecord;                                                       // pointer to the record part of the engine context
  AIRBORNE_DATA  header;                                                        // record header
  double        *spectrum;                                                      // the current spectrum
  double         tmLocal;                                                       // measurement local time
  double         offset;
  RC             rc;                                                            // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  spectrum=(double *)pEngineContext->buffers.spectrum;
  rc=ERROR_ID_NO;

  // Verify input

  if (specFp==NULL)
   rc=ERROR_SetLast("AIRBORNE_Read",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;
  else
   {
    // Complete the reading of the record

    fseek(specFp,(recordNo-1)*(sizeof(AIRBORNE_DATA)+sizeof(double)*NDET),SEEK_SET);
    fread(&header,sizeof(AIRBORNE_DATA),1,specFp);
    fread(spectrum,sizeof(double)*NDET,1,specFp);

    memcpy(&pRecord->present_day,&header.today,sizeof(SHORT_DATE));
    memcpy(&pRecord->present_time,&header.now,sizeof(SHORT_DATE));

    pRecord->TDet=(double)-1.;
    pRecord->rejected=header.nrejMeas;
    pRecord->Tint = (double)header.exposureTime*0.001;
    pRecord->NSomme = header.naccMeas;

    pRecord->longitude=(double)header.longitude;
    pRecord->latitude=(double)header.latitude;
    pRecord->altitude=(double)header.altitude;

    pRecord->uavBira.servoSentPosition=(unsigned char)header.sentPosition;                     // UAV servo control : position byte sent to the PIC
    pRecord->uavBira.servoReceivedPosition=(unsigned char)header.receivedPosition;             // UAV servo control : position byte received by the PIC
    pRecord->uavBira.temperature=(float)header.temperature;
    pRecord->uavBira.pressure=(float)header.pressure;

    pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);
    pRecord->Zm=(double)ZEN_FNTdiz(ZEN_FNCrtjul(&pRecord->Tm),&pRecord->longitude,&pRecord->latitude,&pRecord->Azimuth);
    pRecord->TotalExpTime=(double)header.totalTime;
    pRecord->TimeDec=(double)header.now.ti_hour+header.now.ti_min/60.+header.now.ti_sec/3600.;
    tmLocal=pRecord->Tm+THRD_localShift*3600.;

    pRecord->localCalDay=ZEN_FNCaljda(&tmLocal);
    pRecord->localTimeDec=fmod(pRecord->TimeDec+24.+THRD_localShift,(double)24.);

    if (dateFlag && (pRecord->localCalDay!=localDay))
     rc=ERROR_ID_FILE_RECORD;
   }

  // Return

  return rc;
 }

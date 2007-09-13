
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  THE BIRA-IASB DOAS SOFTWARE FOR WINDOWS AND LINUX
//  Module purpose    :  Read out spectra from the EASOE campaign
//  Name of module    :  EASOE-READ.C
//  Creation date     :  2 April 2004
//
//  Author            :  Caroline FAYT (caroline.fayt@oma.be)
//
//        Copyright  (C) Belgian Institute for Space Aeronomy (BIRA-IASB)
//                       Avenue Circulaire, 3
//                       1180     UCCLE
//                       BELGIUM
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
//  This module contains routines to read data measured during the EASOE
//  campaign (Keflavik, ICELAND, 1992)
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetEASOE - Get the size and the number of records of the current file
//  ReliEASOE - EASOE record read out;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"

// ==========
// DEFINITION
// ==========

#pragma pack(1)

typedef struct easoe
 {
  LONG t_int;                                                                   // exposure time
  short n_somm;                                                                 // number of accumulations
  SHORT_DATE present_day;                                                       // measurement date
  struct time present_time;                                                     // measurement time
  float  spectre[1025];                                                         // the measured spectrum
  char   SkyObs;                                                                // information on the sky
 }
EASOE;

// =========
// FUNCTIONS
// =========

// -----------------------------------------------------------------------------
// FUNCTION      SetEASOE
// -----------------------------------------------------------------------------
// PURPOSE       Get the size and the number of records of the current file
//
// INPUT         specFp    pointer to the spectra file
//
// OUTPUT        pSpecInfo->recordNumber, the number of records
//
// RETURN        ERROR_ID_ALLOC           buffers allocation failed
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_EMPTY      the file is empty;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC SetEASOE(SPEC_INFO *pSpecInfo,FILE *specFp,FILE *namesFp)
 {
  // Declarations

  UCHAR  names[20];                                                             // name of the current spectrum
  SHORT *indexes,                                                               // size of SpecMax arrays
         curvenum;                                                              // number of spectra in the file
  ULONG *recordIndexes;                                                         // save the position of each record in the file
  INDEX i;                                                                      // browse spectra in the file
  RC rc;                                                                        // return code

  // Initializations

  pSpecInfo->recordIndexesSize=2001;
  recordIndexes=pSpecInfo->recordIndexes;
  rc=ERROR_ID_NO;

  // Buffers allocation

  if (((indexes=(SHORT *)MEMORY_AllocBuffer("SetEASOE","indexes",pSpecInfo->recordIndexesSize,sizeof(SHORT),0,MEMORY_TYPE_SHORT))==NULL) ||
      ((pSpecInfo->specMax=MEMORY_AllocDVector("SetEASOE","specMax",0,NDET-1))==NULL))

   rc=ERROR_ID_ALLOC;

  // Open spectra file

  else if ((specFp==NULL) || (namesFp==NULL))
   rc=ERROR_SetLast("SetEASOE",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else
   {
    // Headers read out

    fseek(specFp,0L,SEEK_SET);
    fseek(namesFp,0L,SEEK_SET);

    if (!fread(&curvenum,sizeof(SHORT),1,specFp) ||
        !fread(indexes,pSpecInfo->recordIndexesSize*sizeof(SHORT),1,specFp) ||
        (curvenum<=0))

     rc=ERROR_SetLast("SetEASOE",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pSpecInfo->fileName);

    else
     {
      i=0;
      recordIndexes[0]=(LONG)(pSpecInfo->recordIndexesSize+1)*sizeof(SHORT);    // file header : size of indexes table + curvenum

      if (namesFp!=NULL )
       {
        fseek(namesFp,0L,SEEK_SET);

        while (!feof(namesFp) && fread(names,16,1,namesFp))
         {
          recordIndexes[i]+=indexes[i];

          if (names[11]=='Z')                                                   // name of a zenith spectrum
           {
            i++;
            recordIndexes[i]=recordIndexes[i-1]+pSpecInfo->recordSize;
           }
         }
       }

      pSpecInfo->recordNumber=curvenum;
      pSpecInfo->recordSize=(LONG)sizeof(EASOE);

      for (i=1;i<curvenum;i++)
       recordIndexes[i]=recordIndexes[i-1]+pSpecInfo->recordSize+indexes[i];    // take size of SpecMax arrays into account
     }
   }

  // Release local buffers

  if (indexes!=NULL)
   MEMORY_ReleaseBuffer("SetEASOE","indexes",indexes);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliEASOE
// -----------------------------------------------------------------------------
// PURPOSE       EASOE record read out
//
// INPUT         recordNo   index of record in file;
//               dateFlag   0 no date constraint; 1 a date selection is applied;
//               localDay   if dateFlag is 1, the calendar day for the
//                          reference spectrum to search for
//               specFp     pointer to the spectra file;
//               namesFp    pointer to the names file if any;
//               darkFp     pointer to the dark current files if any;
//
// OUTPUT        information on the read out record
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

RC ReliEASOE(SPEC_INFO *pSpecInfo,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp)
 {
  // Declarations

  EASOE             speRecord,drkRecord;                                        // resp. spectrum and dark current records
  UCHAR             names[20];                                                  // name of the current spectrum
  USHORT           *ISpecMax;                                                   // scans number for each integration time
  double            tmLocal;                                                    // temporary data
  INDEX             i,j;                                                        // indexes for loops and arrays
  RC                rc;                                                         // return code

  // Initializations

  ISpecMax=NULL;
  memset(names,0,20);

  rc=ERROR_ID_NO;

  memset(&speRecord,0,sizeof(EASOE));
  memset(&drkRecord,0,sizeof(EASOE));

  for (i=0;i<1025;i++)
   speRecord.spectre[i]=drkRecord.spectre[i]=(float)0.;

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliEASOE",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else if ((recordNo<=0) || (recordNo>pSpecInfo->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Buffers allocation

  else if ((ISpecMax=(USHORT *)MEMORY_AllocBuffer("ReliEASOE","ISpecMax",2000,sizeof(USHORT),0,MEMORY_TYPE_USHORT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // Set file pointers

    i=j=0;

    if (namesFp!=NULL)
     {
      fseek(namesFp,0L,SEEK_SET);

      while (i<recordNo)
       {
        if (!fread(names,16,1,namesFp))
         break;

        if (names[11]=='O')                                                     // name of a dark current spectrum
         j++;
        if (names[11]=='Z')                                                     // name of a zenith spectrum
         i++;
       }
     }

    fseek(specFp,(LONG)pSpecInfo->recordIndexes[recordNo-1],SEEK_SET);

    // Complete record read out

    fread(&speRecord,pSpecInfo->recordSize,1,specFp);                           // read out the zenith spectrum

    if ((darkFp!=NULL) && (j>0))
     {
      fseek(darkFp,(LONG)sizeof(EASOE)*(j-1),SEEK_SET);
      fread(&drkRecord,pSpecInfo->recordSize,1,darkFp);                         // read out the dark current
     }

    if (speRecord.n_somm>0)
     fread(ISpecMax,(speRecord.n_somm)<<1,1,specFp);

    // Invert spectra

    for (i=0;i<NDET;i++)
     {
      pSpecInfo->spectrum[i]=(double)speRecord.spectre[NDET-i-1];
      pSpecInfo->darkCurrent[i]=(double)drkRecord.spectre[NDET-i-1];
     }

    if (speRecord.n_somm>0)
     for (i=0;i<speRecord.n_somm;i++)
      pSpecInfo->specMax[i]=(double)ISpecMax[i];

    // Data on the current record

    memcpy(pSpecInfo->Nom,names,20);
    memcpy((char *)&pSpecInfo->present_day,(char *)&speRecord.present_day,sizeof(SHORT_DATE));
    memcpy((char *)&pSpecInfo->present_time,(char *)&drkRecord.present_time,sizeof(struct time));

    pSpecInfo->Tm=(double)ZEN_NbSec(&pSpecInfo->present_day,&pSpecInfo->present_time,0);
    pSpecInfo->TotalExpTime=(double)0.;
    pSpecInfo->TimeDec=(double)speRecord.present_time.ti_hour+speRecord.present_time.ti_min/60.+speRecord.present_time.ti_sec/3600.;

    tmLocal=pSpecInfo->Tm+THRD_localShift*3600.;

    pSpecInfo->localCalDay=ZEN_FNCaljda(&tmLocal);
    pSpecInfo->localTimeDec=fmod(pSpecInfo->TimeDec+24.+THRD_localShift,(double)24.);

    pSpecInfo->Tint     = (double)0.001*speRecord.t_int;
    pSpecInfo->NSomme   = speRecord.n_somm;
    pSpecInfo->SkyObs   = speRecord.SkyObs;

    if (dateFlag && (pSpecInfo->localCalDay!=localDay))
     rc=ERROR_ID_FILE_RECORD;
   }

  // Release allocated buffers

  if (ISpecMax!=NULL)
   MEMORY_ReleaseBuffer("ReliEASOE","ISpecMax",ISpecMax);

  // Return

  return rc;
 }

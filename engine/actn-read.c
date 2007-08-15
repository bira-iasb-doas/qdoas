 
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  THE BIRA-IASB DOAS SOFTWARE FOR WINDOWS AND LINUX
//  Module purpose    :  ACTON read out routines
//  Name of module    :  ACTN-READ.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in the WinDOAS package in 97
//
//  Author            :  Kjersti KARLSEN (1) (kjk@nilu.no)
//  Adaptations       :  Caroline FAYT (2) (caroline.fayt@oma.be)
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
//  This module contains the routines needed for reading the data from the ACTON
//  instrument operated by the NILU, Norway.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetActon_Logger - set file pointers for ACTON files and get the number of records
//  GotoActon_Logger - goto the requested record number;
//  ReliActon_Logger - ACTON spectra read out and dark current correction;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"

// ================
// STATIC VARIABLES
// ================

static INDEX actonLastRecord=ITEM_NONE;                                         // Record number of the last record read out.
                                                                                // The ACTON format is an ASCII one, so in order to speed up the
                                                                                // spectra read out, the file pointer is not moved for successive
                                                                                // spectra.

// -----------------------------------------------------------------------------
// FUNCTION      SetActon_Logger
// -----------------------------------------------------------------------------
// PURPOSE       set file pointers and get the number of records of the current file
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

RC SetActon_Logger(SPEC_INFO *pSpecInfo,FILE *specFp)
 {
  // Declarations

  CHAR *Record;                                                                 // string buffer for records
  RC rc;                                                                        // return code

  // Initializations

  actonLastRecord=ITEM_NONE;

  pSpecInfo->recordNumber=0;
  pSpecInfo->longitude=
  pSpecInfo->latitude=
  pSpecInfo->altitude=(double)0.;

  rc=ERROR_ID_NO;

  // Allocate a buffer for file lines read out

  if ((Record=(char *)MEMORY_AllocBuffer("SetActon_Logger","Record",1,16001,0,MEMORY_TYPE_STRING))==NULL)
   rc=ERROR_ID_ALLOC;

  // Calculate the number of spectra in the file

  else if (specFp==NULL)
   rc=ERROR_SetLast("SetActon_Logger",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else
   {
    fseek(specFp,0L,SEEK_SET);

    while ((fgets(Record,16000,specFp)!=0) &&                                   // a record is composed of three lines of max 16000 characters long
           (fgets(Record,16000,specFp)!=0) &&
           (fgets(Record,16000,specFp)!=0))

     pSpecInfo->recordNumber++;
   }

  // Release the allocated buffer

  if (Record!=NULL)
   MEMORY_ReleaseBuffer("SetActon_Logger","Record",Record);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GotoActon_Logger
// -----------------------------------------------------------------------------
// PURPOSE       goto the requested record number
//
// INPUT         specFp      pointer to the current spectra file
//               recordNo    the record number (0 base indexed)
//
// RETURN        ERROR_ID_ALLOC           buffers allocation failed
//               ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_FILE_END        the end of file is reached
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC GotoActon_Logger(FILE *specFp,int recordNo)
 {
  // Declarations

  char *Record;                                                                 // string buffer for records
  int i;                                                                        // index for browsing spectra in the file
  RC rc;                                                                        // return code

  // Initialization

  rc=ERROR_ID_NO;

  // Buffer allocation

  if ((Record=(char *)MEMORY_AllocBuffer("GotoActon_Logger ","Record",1,16001,0,MEMORY_TYPE_STRING))==NULL)
   rc=ERROR_ID_ALLOC;
  else if (specFp==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;
  else
   {
    // Goto back to the beginning of the file

    fseek(specFp,0L,SEEK_SET);

    // Skip the requested number of records

    for (i=0;(i<recordNo) && !rc;i++)

     if (!fgets(Record,16000,specFp) ||
         !fgets(Record,16000,specFp) ||
         !fgets(Record,16000,specFp))

      rc=ERROR_ID_FILE_END;
   }

  // Release the allocated buffer buffer

  if (Record!=NULL)
   MEMORY_ReleaseBuffer("GotoActon_Logger ","Record",Record);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReliActon_Logger
// -----------------------------------------------------------------------------
// PURPOSE       ACTON spectra read out and dark current correction
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

RC ReliActon_Logger(SPEC_INFO *pSpecInfo,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp)
 {
  // Declarations

  char       *Record1,*Record2,*Record3,*Temp,                                  // string buffers for file lines read out
             *pRecord,*pTemp;                                                   // pointers to part of previous strings

  int         i,                                                                // browse pixels in the current spectrum
              length;                                                           // string length
  UINT        help;                                                             // kjk oktober 1995
  double      dark_Tint;                                                        // kjk 251095
  int         found;                                                            // kjk 251095
  double     *spectrum;                                                         // kjk 251095
  SHORT_DATE  day;                                                              // date of the current measurement
  double      tmLocal;                                                          // the measurement time in seconds
  RC          rc;                                                               // return code

  // Initializations

  Record1=Record2=Record3=Temp=NULL;
  spectrum=pSpecInfo->spectrum;
  rc=ERROR_ID_NO;

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliActon_Logger",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else if ((recordNo<=0) || (recordNo>pSpecInfo->recordNumber))
   rc=ERROR_ID_FILE_END;

  // Buffers allocation

  else if (((Record1=(char *)MEMORY_AllocBuffer("ReliActon_Logger","Record1",1,16001,0,MEMORY_TYPE_STRING))==NULL) ||
           ((Record2=(char *)MEMORY_AllocBuffer("ReliActon_Logger","Record2",1,16001,0,MEMORY_TYPE_STRING))==NULL) ||
           ((Record3=(char *)MEMORY_AllocBuffer("ReliActon_Logger","Record3",1,16001,0,MEMORY_TYPE_STRING))==NULL) ||
           ((Temp=(char *)MEMORY_AllocBuffer("ReliActon_Logger","Temp",1,200,0,MEMORY_TYPE_STRING))==NULL))

   rc=ERROR_ID_ALLOC;

  else if ((recordNo-actonLastRecord==1) ||                                     // for successive spectra, don't move file pointer
          !(rc=GotoActon_Logger(specFp,recordNo-1)))
   {
    actonLastRecord=recordNo;

    // Read the complete record (in 3 lines)

    if (!fgets(Record1,16000,specFp) || !fgets(Record2,16000,specFp) || !fgets(Record3,16000,specFp))
     rc=ERROR_ID_FILE_END;
    else
     {
      // Header read out

      pRecord=Record1;
      pTemp=Temp;

      pTemp=strcpy(pTemp,"      \0");
      strncpy(pTemp,pRecord,2);
      day.da_year=(SHORT)atoi(pTemp);

      if (day.da_year<30)
       day.da_year+=(short)2000;
      else if (day.da_year<130)
       day.da_year+=(short)1900;
      else if (day.da_year<1930)
       day.da_year+=(short)100;

      pRecord=pRecord+2;
      pTemp=strcpy(pTemp,"      \0");
      strncpy(pTemp,pRecord,2);
      day.da_mon=(CHAR)atoi(pTemp);

      pRecord=pRecord+2;
      pTemp=strcpy(pTemp,"                    \0");
      strncpy(pTemp,pRecord,2);
      day.da_day=(CHAR)atoi(pTemp);

      pRecord=pRecord+3;
      strncpy(pSpecInfo->Nom,pRecord,19);
      pSpecInfo->Nom[(pSpecInfo->project.instrumental.user==PRJCT_INSTR_NILU_FORMAT_OLD)?18:19]='\0';
      pTemp=strcpy(pTemp,"                          \0");
      pTemp=strcpy(pTemp,pSpecInfo->Nom);

      pTemp=(pSpecInfo->project.instrumental.user==PRJCT_INSTR_NILU_FORMAT_OLD)?pTemp+12:pTemp+15;
      pSpecInfo->Zm=(double)atof(pTemp)/100.;
//      pSpecInfo->Zm=(double)pSpecInfo->Zm/((pSpecInfo->project.instrumental.user==PRJCT_INSTR_NILU_FORMAT_OLD)?10.:100.); /* changed from 10 to 100 when two decimals in the SZA, 14.08.98 kjk*/
      pSpecInfo->Azimuth=(double)-1;

      pRecord=pRecord+20;
      pTemp=strcpy(pTemp,"                    \0");
      strncpy(pTemp,pRecord,2);
      pSpecInfo->present_time.ti_hour=(UCHAR)atoi(pTemp);
      pRecord=pRecord+3;
      pTemp=strcpy(pTemp,"                    \0");
      strncpy(pTemp,pRecord,2);
      pSpecInfo->present_time.ti_min=(UCHAR)atoi(pTemp);
      pRecord=pRecord+3;
      pTemp=strcpy(pTemp,"                    \0");
      strncpy(pTemp,pRecord,2);
      pSpecInfo->present_time.ti_sec=(UCHAR)atoi(pTemp);
      pRecord=pRecord+3;
      pTemp=strcpy(pTemp,"                    \0");
      pSpecInfo->ReguTemp=(float)atof(pRecord);
      pRecord=pRecord+6;

      pSpecInfo->Tint=atof(pRecord);
      pRecord=pRecord+5;
      pSpecInfo->NSomme=atoi(pRecord);
      pRecord=pRecord+4;
      pSpecInfo->rejected=atoi(pRecord);

      memcpy(&pSpecInfo->present_day,&day,sizeof(SHORT_DATE));

      pSpecInfo->Tm=(double)ZEN_NbSec(&pSpecInfo->present_day,&pSpecInfo->present_time,0);

      pSpecInfo->TotalExpTime         = (double)pSpecInfo->NSomme*pSpecInfo->Tint;
      pSpecInfo->TimeDec              = (double)pSpecInfo->present_time.ti_hour+pSpecInfo->present_time.ti_min/60.;

      // Spectrum read out

      pRecord=Record2;

      for (i=0;i<NDET;i++)
       {
        sscanf(pRecord,"%lf",&spectrum[i]);
        help=(unsigned int)spectrum[i];
        Temp=strcpy(Temp,"                    \0");  /*ny*/
        sprintf(Temp,"%u",help);
        length=strlen(Temp);                       /*printf("length %d",length);*/
        pRecord=pRecord+length+1;
       }

      // Dark current read out (the dark current subtraction is made in the calling function)

      if ((darkFp!=NULL) && (pSpecInfo->NSomme>0))
       {
        fseek(darkFp,0L,SEEK_SET);
        found=0;

        while (!found && fgets(Record1,16000,darkFp) && fgets(Record2,16000,darkFp) && fgets(Record3,16000,darkFp))
         {
          pRecord=Record1+42;

          dark_Tint=(double) atof(pRecord);

          if (dark_Tint==pSpecInfo->Tint)
           {
            found=1;
            pRecord=Record2;

            for (i=0;i<NDET;i++)
             {
              sscanf(pRecord,"%lf",&pSpecInfo->darkCurrent[i]);
              help=(unsigned int)pSpecInfo->darkCurrent[i];
              pTemp=strcpy(pTemp,"                    \0"); /*ny*/
              sprintf(pTemp,"%u",help);
              length=strlen(pTemp);
              pRecord=pRecord+length+1;
             }
           }
         }
       }

      // Daily automatic reference spectrum

      tmLocal=pSpecInfo->Tm+THRD_localShift*3600.;

      pSpecInfo->localCalDay=ZEN_FNCaljda(&tmLocal);
      pSpecInfo->localTimeDec=fmod(pSpecInfo->TimeDec+24.+THRD_localShift,(double)24.);

      if (!pSpecInfo->NSomme || (pSpecInfo->rejected>=pSpecInfo->NSomme) ||
          (dateFlag && (pSpecInfo->localCalDay!=localDay)))

       rc=ERROR_ID_FILE_RECORD;
     }
   }

  // Release the allocated buffers

  if (Record1!=NULL)
   MEMORY_ReleaseBuffer("ReliActon_Logger","Record1",Record1);
  if (Record2!=NULL)
   MEMORY_ReleaseBuffer("ReliActon_Logger","Record2",Record2);
  if (Record3!=NULL)
   MEMORY_ReleaseBuffer("ReliActon_Logger","Record3",Record3);
  if (Temp!=NULL)
   MEMORY_ReleaseBuffer("ReliActon_Logger","Temp",Temp);     /* kjk oktober 1995 */

  // Return

  return rc;
 }

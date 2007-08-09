
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
//  Module purpose    :  FOURIER read out routines
//  Name of module    :  OPUS-READ.C
//  Program Language  :  Borland C++ 5.0 for Windows 95/NT
//  Creation date     :  Routines have been implemented on 21 november 2000
//                       from source code provided by Ann-Carine VANDAELE
//                       (IASB/BIRA and ULB)
//
//  Author            :  routines provided by Ann Carine VANDAELE (ULB)
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
//  This module contains routines needed for reading out spectra in OPUS format.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  OPUS_Set - this function initializes variables for OPUS format read out
//  OPUS_Read - OPUS format read out
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"

// ======================
// STRUCTURES DEFINITIONS
// ======================

typedef struct tagOpusHeader
 {
  double PgmVer;          //  Program Version Number
  int    DirPtr;          //  Pointer to first directory in bytes
  int    MaxBlk;          //  Max size of directory in terms of blocks
  int    CurBlk;          //  Current size of directory in terms of blocks
 }
OPUS_HEADER;

typedef struct tagDirectoryBlock
 {
  int    Type;             // Directory block type
  int    Length;           // Length in 32 bits words
  int    Pointer;          // Offset File Pointer
 }
DIR_BLOCK;

typedef struct tagParameterBlock
 {
  char   Name[4];
  short  Type;
  short  Rs;
 }
PAR_BLOCK;

static int dayMonth[]={0,31,28,31,30,31,30,31,31,30,31,30,31};

// -----------------------------------------------------------------------------
// FUNCTION      OPUS_Set
// -----------------------------------------------------------------------------
// PURPOSE       This function initializes variables for OPUS format read out
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
RC OPUS_Set(SPEC_INFO *pSpecInfo,FILE *specFp)
 {
  // Declarations

  OPUS_HEADER Header;                                  // File Header
  DIR_BLOCK DirBlock;                                  // Directory Block
  PAR_BLOCK ParBlock;                                  // Parameter Block
  PRJCT_INSTRUMENTAL *pInstr;

  UCHAR  DateTemp[32],                                 // temporary variable for atof conversion
         fileName[MAX_STR_SHORT_LEN+1],                // current file name
        *ptr1,*ptr2;                                   // pointers to the extension of the first and the last files in the current directory

  ULONG  MagicNumber;                                  // OPUS Signature
  double Dtemp;                                        // temporary variable for swapping
  double pas,fact;                                     // temporary variables for building the wavelength calibration

  INT    firstFile,lastFile,found;                     // first and last file number
  INDEX  i;                                            // index for loops and arrays
  RC     rc;                                           // return code

  // Initializations

  pInstr=&pSpecInfo->project.instrumental;
  found=0;

  pSpecInfo->opus.NumberPoints = (ULONG) 0;            // Number of Data Points
  pSpecInfo->opus.WaveLow = (double) 0.;               // First X Value
  pSpecInfo->opus.WaveHigh = (double) 0.;              // Last X Value
  pSpecInfo->opus.Dispersion = (double) 0.;            // Dispersion
  pSpecInfo->opus.Resolution = (double) 0.;            // Resolution
  pSpecInfo->opus.ScaleFactor = (double) 1.;           // Common Y Scaling Factor
  pSpecInfo->opus.IrisDiameter = (float) 0.;           // Aperture Setting
  pSpecInfo->opus.Scantime = (float) 0.;               // Scan Time

  pSpecInfo->opus.Apodization[0]=
  pSpecInfo->opus.ChemistName[0]=
  pSpecInfo->opus.SampleName[0]=
  pSpecInfo->opus.SampleForm[0]=
  pSpecInfo->opus.Date[0]=
  pSpecInfo->opus.Time[0]=0;

  rc=ERROR_ID_NO;

  // Header read out

  if (specFp==NULL)
   rc=ERROR_SetLast("OPUS_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pSpecInfo->fileName);
  else if (!fread(&MagicNumber,sizeof(ULONG),1,specFp))
   rc=ERROR_SetLast("OPUS_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pSpecInfo->fileName);
  else if (MagicNumber!=0XFEFE0A0AL)                                            // OPUS signature ?
   rc=ERROR_SetLast("OPUS_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_BAD_FORMAT,pSpecInfo->fileName);
  else if (!fread(&Header,sizeof(OPUS_HEADER),1,specFp))                        // Header Read out
   rc=ERROR_SetLast("OPUS_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pSpecInfo->fileName);
  else
   {
    // Directory Blocks Read Out

    for (i=0;(i<Header.CurBlk) && !rc;i++)
     {
      fseek(specFp,(LONG)Header.DirPtr+sizeof(DIR_BLOCK)*i,SEEK_SET);           // Set Pointer to Dir Block

      if (!fread(&DirBlock,sizeof(DIR_BLOCK),1,specFp))                         // Directory Block Read out
       rc=ERROR_ID_FILE_RECORD;
      else
       {
        fseek(specFp,DirBlock.Pointer,SEEK_SET);                                // Set Pointer to Dir Data

        switch ( (LONG) DirBlock.Type & 131063L )
         {
       // --------------------------------------------------------------
          case  1047L :   //  DATA STATUS PARAMETERS
          case  5143L :   //  transmittance PARAMETERS
       // --------------------------------------------------------------

          if ((!pInstr->user && (((LONG)DirBlock.Type&131063L)==1047L)) ||
              ((pInstr->user==1) && (((LONG)DirBlock.Type&131063L)==5143L)))
           {
            do
            {
             found=1;
             fread(&ParBlock,sizeof(PAR_BLOCK),1,specFp);

             if (!strcmp(ParBlock.Name,"NPT"))
              fread(&pSpecInfo->opus.NumberPoints,4,1,specFp);
             else if (!strcmp(ParBlock.Name,"FXV"))
              fread(&pSpecInfo->opus.WaveLow,8,1,specFp);
             else if (!strcmp(ParBlock.Name,"LXV"))
              fread(&pSpecInfo->opus.WaveHigh,8,1,specFp);
             else if (!strcmp(ParBlock.Name,"CSF"))
              fread(&pSpecInfo->opus.ScaleFactor,8,1,specFp);
             else if (!strcmp(ParBlock.Name,"DAT"))
              fread(&pSpecInfo->opus.Date,ParBlock.Rs<<1,1,specFp);
             else if (!strcmp(ParBlock.Name,"TIM"))
              fread(&pSpecInfo->opus.Time,ParBlock.Rs<<1,1,specFp);
             else if (!strcmp(ParBlock.Name,"END"))
              break;
             else
              fseek(specFp,(LONG)ParBlock.Rs<<1,SEEK_CUR);
            }
            while (1);

            //  WaveLength Range

            if (pSpecInfo->opus.WaveHigh>pSpecInfo->opus.WaveLow)
             pSpecInfo->opus.Ascending=TRUE;
            else
             {
              pSpecInfo->opus.Ascending=FALSE;

              Dtemp=pSpecInfo->opus.WaveHigh;
              pSpecInfo->opus.WaveHigh=pSpecInfo->opus.WaveLow;
              pSpecInfo->opus.WaveLow=Dtemp;
             }

            if (pSpecInfo->opus.NumberPoints>(ULONG)NDET)
             rc=ERROR_SetLast("OPUS_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_BAD_LENGTH,pSpecInfo->fileName,pSpecInfo->opus.NumberPoints);
            else if (pSpecInfo->opus.NumberPoints>1L)
             pSpecInfo->opus.Dispersion=(pSpecInfo->opus.WaveHigh-pSpecInfo->opus.WaveLow)/(double)(pSpecInfo->opus.NumberPoints-1L);

            //  Date and Time

            pSpecInfo->opus.Date[11]='\0';
           }

          break;

       // --------------------------------------------------------------
          case  96L :   //  PROCESSING PARAMETERS
       // --------------------------------------------------------------

          do
          {
           fread(&ParBlock,sizeof(PAR_BLOCK),1,specFp);

           if (!strcmp(ParBlock.Name,"APT"))
            fread(&DateTemp,ParBlock.Rs<<1,1,specFp);
           else if (!strcmp(ParBlock.Name,"END"))
            break;

           else
            fseek(specFp,(LONG)ParBlock.Rs<<1,SEEK_CUR);
          }
          while (1);

          pSpecInfo->opus.IrisDiameter=(float)atof(DateTemp);

          break;

       // ------------------------------------------------------------------
          default   :   //  NOT CONSIDERED
       // ------------------------------------------------------------------

          break;
         } //  END OF SWITCH DIR TYPE
       }
     }


    if (!found)
     rc=ERROR_SetLast("OPUS_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pSpecInfo->fileName);
    else if (!rc)
     {
      NDET=(INT)pSpecInfo->opus.NumberPoints;

      // Build the wavelength calibration

      pas=(pSpecInfo->opus.WaveHigh-pSpecInfo->opus.WaveLow)/(pSpecInfo->opus.NumberPoints-1);
      fact=(double)6329.915/6328.173*(1+pSpecInfo->opus.IrisDiameter*pSpecInfo->opus.IrisDiameter/16/220/220);

      if (pSpecInfo->opus.Ascending)
       for (i=0;i<(INT)pSpecInfo->opus.NumberPoints;i++)
        pSpecInfo->lembda[i]=(pSpecInfo->opus.WaveLow+pas*i)*fact;
      else
       for (i=0;i<(INT)pSpecInfo->opus.NumberPoints;i++)
        pSpecInfo->lembda[pSpecInfo->opus.NumberPoints-i-1]=(pSpecInfo->opus.WaveLow+pas*(pSpecInfo->opus.NumberPoints-i-1))*fact;

      // Get the number of records (one record per file but a directory can be considered as only one file with several records)

      memset(fileName,0,MAX_STR_SHORT_LEN+1);
      strncpy(fileName,pSpecInfo->fileName,MAX_STR_SHORT_LEN);

      if (THRD_treeCallFlag && ((ptr1=strrchr(fileName,'.'))!=NULL) && ((ptr2=strrchr(PATH_fileMax+1,'.'))!=NULL))
       {
        sscanf(ptr1+1,"%d",&firstFile);
        sscanf(ptr2+1,"%d",&lastFile);

        pSpecInfo->recordNumber=(lastFile-firstFile+1);
       }
      else
       pSpecInfo->recordNumber=1;
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      OPUS_Read
// -----------------------------------------------------------------------------
// PURPOSE       OPUS format read out
//
// INPUT         recordNo     index of record in file
//               dateFlag     flag for automatic reference selection
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
RC OPUS_Read(SPEC_INFO *pSpecInfo,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  PRJCT_INSTRUMENTAL *pInstr;
  UCHAR  fileName[MAX_STR_SHORT_LEN+1],*ptr1,*ptr2;                             // current file name

  OPUS_HEADER Header;                                                           // File Header
  DIR_BLOCK DirBlock;                                                           // Directory Block
  PAR_BLOCK ParBlock;                                                           // Parameter Block

  ULONG  MagicNumber,NumberPoints;                                              // OPUS Signature
  float *fSpectrum;                                                             // data in float precision
  UCHAR  DateTemp[32];                                                          // temporary variable for atof conversion
  double Dtemp,tmLocal,                                                         // temporary variable for swapping
         WaveHigh,WaveLow;                                                      // temporary variables for building the wavelength calibration
  float  newHour;

  INDEX  i;                                                                     // Indexes for loops and arrays
  INT    day,mon,year,hour,min,sec,                                             // date and time fields
         firstFile,found,                                                       // first file number
         nday;


  FILE  *fp;                                                                    // pointer to file
  RC rc;                                                                        // Return code

  // Initializations

  pInstr=&pSpecInfo->project.instrumental;
  fp=specFp;
  fSpectrum=NULL;
  memcpy(fileName,pSpecInfo->fileName,MAX_STR_SHORT_LEN);
  rc=ERROR_ID_NO;
  found=0;

  if ((recordNo>0) && (recordNo<=pSpecInfo->recordNumber))
   {
    if (THRD_treeCallFlag && ((ptr1=strrchr(pSpecInfo->fileName,'.'))!=NULL))
     {
      sscanf(ptr1+1,"%d",&firstFile);
      ptr2=fileName+(ptr1-pSpecInfo->fileName);
      sprintf(ptr2+1,"%03d",firstFile+recordNo-1);

      fp=fopen(fileName,"rb");
     }

    // Check the presence of the file

    if (fp==NULL)
     rc=ERROR_ID_FILE_RECORD;
    else if ((recordNo<=0) || (recordNo>pSpecInfo->recordNumber))
     rc=ERROR_ID_FILE_END;
    else
     {
      fseek(fp,0L,SEEK_SET);           // Set Pointer to the beginning of the file

      // Header read out

      if (!fread(&MagicNumber,sizeof(ULONG),1,fp))
       rc=ERROR_SetLast("OPUS_Read",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pSpecInfo->fileName);
      else if (MagicNumber!=0XFEFE0A0AL)                                        // OPUS signature ?
       rc=ERROR_SetLast("OPUS_Read",ERROR_TYPE_WARNING,ERROR_ID_FILE_BAD_FORMAT,pSpecInfo->fileName);
      else if (!fread(&Header,sizeof(OPUS_HEADER),1,fp))                        // Header Read out
       rc=ERROR_SetLast("OPUS_Read",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pSpecInfo->fileName);
      else
       {
        // Directory Blocks Read Out

        for (i=0;(i<Header.CurBlk) && !rc;i++)
         {
          fseek(fp,(LONG)Header.DirPtr+sizeof(DIR_BLOCK)*i,SEEK_SET);               // Set Pointer to Dir Block

          if (!fread(&DirBlock,sizeof(DIR_BLOCK),1,fp))                         // Directory Block Read out
           rc=ERROR_ID_FILE_RECORD;
          else
           {
            fseek(fp,DirBlock.Pointer,SEEK_SET);                                // Set Pointer to Dir Data

            switch ( (LONG) DirBlock.Type & 131063L )
             {
           // --------------------------------------------------------------
              case   1031L :   //  SAMPLE DATA
              case   5127L :   //  TRANSMITTANCE
           // --------------------------------------------------------------

              if ((!pInstr->user && (((LONG)DirBlock.Type&131063L)==1031L)) ||
                  ((pInstr->user==1) && (((LONG)DirBlock.Type&131063L)==5127L)))
               {
                if ((fSpectrum=(float *)MEMORY_AllocBuffer("OPUS_Read ","fSpectrum",NDET,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL)
                 rc=ERROR_ID_ALLOC;
                else if (!fread(fSpectrum,sizeof(float)*NDET,1,fp))
                 rc=ERROR_ID_FILE_RECORD;
                else
                 found=1;
               }

              break;

           // --------------------------------------------------------------
              case  1047L :   //  DATA STATUS PARAMETERS
              case  5143L :   //  TRANSMITTANCE PARAMETERS
           // --------------------------------------------------------------

              if ((!pInstr->user && (((LONG)DirBlock.Type&131063L)==1047L)) ||
                 ((pInstr->user==1) && (((LONG)DirBlock.Type&131063L)==5143L)))
               {
                do
                {
                 fread(&ParBlock,sizeof(PAR_BLOCK),1,fp);

                 if (!strcmp(ParBlock.Name,"NPT"))
                  fread(&NumberPoints,4,1,fp);
                 else if (!strcmp(ParBlock.Name,"FXV"))
                  fread(&WaveLow,8,1,fp);
                 else if (!strcmp(ParBlock.Name,"LXV"))
                  fread(&WaveHigh,8,1,fp);
                 else if (!strcmp(ParBlock.Name,"CSF"))
                  fread(&pSpecInfo->opus.ScaleFactor,8,1,fp);
                 else if (!strcmp(ParBlock.Name,"DAT"))
                  fread(&pSpecInfo->opus.Date,ParBlock.Rs<<1,1,fp);
                 else if (!strcmp(ParBlock.Name,"TIM"))
                  fread(&pSpecInfo->opus.Time,ParBlock.Rs<<1,1,fp);
                 else if (!strcmp(ParBlock.Name,"END"))
                  break;
                 else
                  fseek(fp,(LONG)ParBlock.Rs<<1,SEEK_CUR);
                }
                while (1);

                //  Date and Time

//                if ((pSpecInfo->opus.Ascending && ((WaveLow!=pSpecInfo->opus.WaveLow) || (WaveHigh!=pSpecInfo->opus.WaveHigh))) ||
//                   (!pSpecInfo->opus.Ascending && ((WaveLow!=pSpecInfo->opus.WaveHigh) || (WaveHigh!=pSpecInfo->opus.WaveLow))))

//                 rc=ERROR_ID_FILE_RECORD;

//                else
                 pSpecInfo->opus.Date[11]='\0';
               }

              break;

           // --------------------------------------------------------------
              case  32L :   //  INSTRUMENT PARAMETERS
           // --------------------------------------------------------------

              do
              {
               fread(&ParBlock,sizeof(PAR_BLOCK),1,fp);

               if (!strcmp(ParBlock.Name,"DUR"))
                fread(&pSpecInfo->opus.Scantime,8,1,fp);
               else if (!strcmp(ParBlock.Name,"END"))
                break;
               else
                fseek(fp,(LONG)ParBlock.Rs<<1,SEEK_CUR);
              }
              while (1);

              break;

           // --------------------------------------------------------------
              case  48L :   //  STANDARD ACQUISITION PARAMETERS
           // --------------------------------------------------------------

              do
              {
               fread(&ParBlock,sizeof(PAR_BLOCK),1,fp);

               if (!strcmp(ParBlock.Name,"RES"))
                fread(&pSpecInfo->opus.Resolution,8,1,fp);
               else if (!strcmp(ParBlock.Name,"END"))
                break;

               else
                fseek(fp,(LONG)ParBlock.Rs<<1,SEEK_CUR);
              }
              while (1);

              break;

           // --------------------------------------------------------------
              case  64L :   //  FT PARAMETERS
           // --------------------------------------------------------------

              do
              {
               fread(&ParBlock,sizeof(PAR_BLOCK),1,fp);

               if (!strcmp(ParBlock.Name,"APF"))
                fread(&pSpecInfo->opus.Apodization,ParBlock.Rs<<1,1,fp);
               else if (!strcmp(ParBlock.Name,"END"))
                break;

               else
                fseek(fp,(LONG)ParBlock.Rs<<1,SEEK_CUR);
              }
              while (1);

              break;

           // --------------------------------------------------------------
              case  96L :   //  PROCESSING PARAMETERS
           // --------------------------------------------------------------

              do
              {
               fread(&ParBlock,sizeof(PAR_BLOCK),1,fp);

               if (!strcmp(ParBlock.Name,"APT"))
                fread(&DateTemp,ParBlock.Rs<<1,1,fp);
               else if (!strcmp(ParBlock.Name,"END"))
                break;

               else
                fseek(fp,(LONG)ParBlock.Rs<<1,SEEK_CUR);
              }
              while (1);

              pSpecInfo->opus.IrisDiameter=(float)atof(DateTemp);

              break;

           // --------------------------------------------------------------
              case 160L :   //  SAMPLE ORIGIN PARAMETERS
           // --------------------------------------------------------------

              do
              {
               fread(&ParBlock,sizeof(PAR_BLOCK),1,fp);

               if (!strcmp(ParBlock.Name,"SNM"))
                fread(&pSpecInfo->opus.SampleName,ParBlock.Rs<<1,1,fp);
               else if (!strcmp(ParBlock.Name,"SFM"))
                fread(&pSpecInfo->opus.SampleForm,ParBlock.Rs<<1,1,fp);
               else if (!strcmp(ParBlock.Name,"CNM"))
                fread(&pSpecInfo->opus.ChemistName,ParBlock.Rs<<1,1,fp);

               else if (!strcmp(ParBlock.Name,"END"))
                break;

               else
                fseek(fp,(LONG)ParBlock.Rs<<1,SEEK_CUR);
              }
              while (1);

              break;

           // ------------------------------------------------------------------
              default   :   //  NOT CONSIDERED
           // ------------------------------------------------------------------

              break;
             } //  END OF SWITCH DIR TYPE
           }
         }
       }

      // Spectrum safe keeping

      if (!found || (fSpectrum==NULL))
       rc=ERROR_ID_FILE_RECORD;
      else if (!rc)
       {
        strcpy(PATH_fileSpectra,fileName);

        Dtemp=(pSpecInfo->opus.ScaleFactor==0.)?(double)1.:(double)pSpecInfo->opus.ScaleFactor;

        for (i=0;i<(INT)pSpecInfo->opus.NumberPoints;i++)
         pSpecInfo->spectrum[i]=(double)fSpectrum[i]*Dtemp;

        // Revert spectrum

        if (WaveLow>WaveHigh)

         for (i=0;i<(INT)pSpecInfo->opus.NumberPoints/2;i++)
          {
           Dtemp=pSpecInfo->spectrum[i];
           pSpecInfo->spectrum[i]=pSpecInfo->spectrum[(INT)pSpecInfo->opus.NumberPoints-i-1];
           pSpecInfo->spectrum[(INT)pSpecInfo->opus.NumberPoints-i-1]=Dtemp;
          }

        // Fill in SpecInfo fields

        pSpecInfo->NSomme=1;
        pSpecInfo->Tint=
        pSpecInfo->TotalExpTime=pSpecInfo->opus.Scantime;

        sscanf(pSpecInfo->opus.Date,"%02d/%02d/%d",&day,&mon,&year);
        sscanf(pSpecInfo->opus.Time,"%02d:%02d:%02d",&hour,&min,&sec);

        nday=dayMonth[mon];
        if ((mon==2) && ((year%4)==0))
         nday++;

        newHour=(float)hour+min/60.+sec/3600.+pSpecInfo->project.instrumental.opusTimeShift;

        if (newHour>24.)
         {
          newHour=(float)fmod(newHour,24.);
          day++;

          if (day>nday)
           {
            day-=nday;
            mon++;

            if (mon>12)
             {
              mon=1;
              year++;
             }
           }
         }
        else if (newHour<0.)
         {
          newHour=(float)fmod((newHour+24.),24.);
          day--;

          if (day<1)
           {
            day-=nday;
            mon--;

            if (mon<1)
             {
              mon=12;
              year--;
             }
           }
         }

        hour=(INT)newHour;
        min=(INT)((newHour-hour)*60.);
        sec=(INT)((newHour-hour-min/60.)*3600.);

        pSpecInfo->present_day.da_year=(SHORT)year;
        pSpecInfo->present_day.da_mon=(UCHAR)mon;
        pSpecInfo->present_day.da_day=(UCHAR)day;

        pSpecInfo->present_time.ti_hour=(UCHAR)hour;
        pSpecInfo->present_time.ti_min=(UCHAR)min;
        pSpecInfo->present_time.ti_sec=(UCHAR)sec;

        pSpecInfo->Tm=(double)ZEN_NbSec(&pSpecInfo->present_day,&pSpecInfo->present_time,0);
        pSpecInfo->TimeDec=(double)pSpecInfo->present_time.ti_hour+pSpecInfo->present_time.ti_min/60.+pSpecInfo->present_time.ti_sec/3600.;

        tmLocal=pSpecInfo->Tm+THRD_localShift*3600.;

        pSpecInfo->localCalDay=ZEN_FNCaljda(&tmLocal);
        pSpecInfo->localTimeDec=fmod(pSpecInfo->TimeDec+24.+THRD_localShift,(double)24.);

        if ((ptr1=strrchr(fileName,PATH_SEP))==NULL)
         ptr1=fileName;
        strncpy(pSpecInfo->Nom,ptr1+1,19);

        if (dateFlag && (pSpecInfo->localCalDay!=localDay))
         rc=ERROR_ID_FILE_RECORD;
       }
     }

    // Close current file

    if (fp!=specFp)
     fclose(fp);
   }

  // Release allocated buffer

  if (fSpectrum!=NULL)
   MEMORY_ReleaseBuffer("OPUS_Read ","fSpectrum",fSpectrum);

  // Return

  return rc;
 }

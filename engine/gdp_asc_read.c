
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  GOME interface (calibrated level 1 data files in the original ASCII format)
//  Name of module    :  GDP_Asc_Read.c
//  Creation date     :  10 December 2001 (original program : Gome-Readlvl1.exe)
//  Modified          :  11 December 2O01 (adapted for WinDOAS)
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
//  FUNCTIONS
//
//  =========================
//  GLOBAL VECTORS ALLOCATION
//  =========================
//
//  GDP_ASC_ReleaseBuffers : release allocated vectors;
//
//  ====================
//  CONVERSION FUNCTIONS
//  ====================
//
//  Band2Channel : this function returns the channel value corresponding to the
//                 given science data enumerator (the so-called band);
//
//  ==================
//  READ OUT FUNCTIONS
//  ==================
//
//  ReadERS            ERS information read out;
//
//  ReadSolarChannel   retrieve the solar irradiance spectrum measured at a specific channel;
//
//  ReadSolar          read out the irradiance section and return the irradiance spectrum
//                     measured at a specific channel;
//
//  GotoSpectra        goto earthshine spectra section;
//
//  ReadPixelInfo      return information on the current pixel;
//
//  ReadSpectrum       return the next earthshine spectra measured at a specific band;
//
//  ================
//  SPECTRA BROWSING
//  ================
//
//  GDP_ASC_Set            load the irradiance spectrum measured at the specified
//                     band and determine the number of records;
//
//  GDP_ASC_Read           GDP spectra read out (in the ASCII format);
//
//  ========
//  ANALYSIS
//  ========
//
//  GDP_ASC_LoadAnalysis   load analysis parameters depending on the irradiance spectrum
//  ----------------------------------------------------------------------------

// =====================
// INCLUDE USUAL HEADERS
// =====================

#include <time.h>
#include <string.h>
#include <math.h>

#include "doas.h"
#include "engine_context.h"
#include "analyse.h"
#include "kurucz.h"
#include "vector.h"
#include "zenithal.h"
#include "spline.h"
#include "stdfunc.h"
#include "winthrd.h"

// ====================
// CONSTANTS DEFINITION
// ====================

#define MAX_PIXELS 1024                                                         // maximum size for vectors
#define MAX_BANDS 6                                                             // maximum number of bands

enum _bands { BAND_1A, BAND_1B, BAND_2A, BAND_2B, BAND_3, BAND_4 };
enum _channels { CHANNEL_1, CHANNEL_2, CHANNEL_3, CHANNEL_4 };

// ================
// STATIC VARIABLES
// ================

const char *months[12]={"jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec"};
const char *bands[MAX_BANDS]={"1a","1b","2a","2b","3","4"};

INDEX gdpLastRecord=ITEM_NONE;                                                  // Record number of the last record read out.
double *GDP_ASC_refL,*GDP_ASC_ref,*GDP_ASC_refE;                                            // irradiance vectors
static int curr_utc_day; // #days from 01/01/1950 00:00:00 UTC until start of current L1B file

// =========================
// GLOBAL VECTORS ALLOCATION
// =========================

// -----------------------------------------------------------------------------
// FUNCTION      GDP_ASC_ReleaseBuffers
// -----------------------------------------------------------------------------
// PURPOSE       release allocated vectors
// -----------------------------------------------------------------------------

void GDP_ASC_ReleaseBuffers(void)
 {
  if (GDP_ASC_refL!=NULL)
   MEMORY_ReleaseDVector("GDP_ASC_ReleaseBuffers ","GDP_ASC_refL",GDP_ASC_refL,0);
  if (GDP_ASC_ref!=NULL)
   MEMORY_ReleaseDVector("GDP_ASC_ReleaseBuffers ","GDP_ASC_ref",GDP_ASC_ref,0);
  if (GDP_ASC_refE!=NULL)
   MEMORY_ReleaseDVector("GDP_ASC_ReleaseBuffers ","GDP_ASC_refE",GDP_ASC_refE,0);

  GDP_ASC_refL=GDP_ASC_ref=GDP_ASC_refE=NULL;
 }

// ====================
// CONVERSION FUNCTIONS
// ====================

// -----------------------------------------------------------------------------
// FUNCTION      Band2Channel
// -----------------------------------------------------------------------------
// PURPOSE       this function returns the channel value corresponding to the
//               given science data enumerator (the so-called band)
//
// INPUT         band : the science data enumerator
//
// RETURN        the equivalent channel
// -----------------------------------------------------------------------------

int Band2Channel(int band)
 {
  // Declaration

  int channel;

  // Select a channel according to the specified band

  switch (band)
   {
 // ----------------------------------------------------------------------------
    case BAND_1A :
    case BAND_1B :
     channel=CHANNEL_1;
    break;
 // ----------------------------------------------------------------------------
    case BAND_2A :
    case BAND_2B :
     channel=CHANNEL_2;
    break;
 // ----------------------------------------------------------------------------
    case BAND_3:
     channel=CHANNEL_3;
    break;
 // ----------------------------------------------------------------------------
    case BAND_4:
     channel=CHANNEL_4;
    break;
 // ----------------------------------------------------------------------------
    default :
     channel=-1;
    break;
 // ----------------------------------------------------------------------------
   }

  // Return

  return channel;
 }

// ==================
// READ OUT FUNCTIONS
// ==================

// -----------------------------------------------------------------------------
// FUNCTION      ReadERS
// -----------------------------------------------------------------------------
// PURPOSE       ERS information read out
//
// INPUT         fp : pointer to the GOME level-1 file to read out
//
// OUTPUT        pOrbitNumber : pointer to the orbit number;
//
// RETURN        1 if read out operation failed; 0 if success.
// -----------------------------------------------------------------------------
// DESCRIPTION
//
// ERS information consists in the starting date and time of the current orbit,
// the orbit number, the position and the velocity of the satellite, and other
// information about the current state of the satellite.  Only the orbit number
// is used.
// -----------------------------------------------------------------------------

RC ReadERS(FILE *fp,int *pOrbitNumber)
{
  // Declarations

  char  fileLine[STRING_LENGTH+1]; // file line
  int utcMSec;                     // UTC time
  RC rc;                           // return code

  // Initializations

  memset(fileLine,0,STRING_LENGTH+1);
 *pOrbitNumber=0;
  rc=1;

  // Search for ERS information

  while (!feof(fp) && fgets(fileLine,STRING_LENGTH,fp) && strnicmp(fileLine,"ERS Information",15));

  // Line read out

  if (fgets(fileLine,STRING_LENGTH,fp))
   {
    sscanf(fileLine,"%d %d %d",&curr_utc_day,&utcMSec,pOrbitNumber);
    rc=0;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReadSolarChannel
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve the solar irradiance spectrum measured at a specific channel
//
// INPUT         fp      : pointer to the GOME level-1 file to read out
//               channel : the channel number
//
// OUTPUT        lambda  : the wavelength calibration of the irradiance spectrum;
//               solar   : the irradiance spectrum;
//               solarE  : absolute errors on the irradiance spectrum
//
// RETURN        the size of returned vectors
// -----------------------------------------------------------------------------

int ReadSolarChannel(FILE *fp,int channel,double *lambda,double *solar,double *solarE)
 {
  // Declarations

  char  fileLine[STRING_LENGTH+1],                                             // file line
         channelString[STRING_LENGTH+1];                                        // the channel string to search for
  int    channelStringLength,                                                   // the length of the channel string
         channelNumber,                                                         // the current channel number
         npts;                                                                  // the number of points of vectors to return
  float  wavelStart,wavelEnd;                                                   // starting and ending wavelengths
  INDEX  i;                                                                     // browse pixels

  // Initializations

  memset(fileLine,0,STRING_LENGTH+1);
  npts=0;

  // Build the channel string

  sprintf(channelString,"Channel %d",channel+1);
  channelStringLength=strlen(channelString);

  // Search for the requested channel

  while (!feof(fp) && fgets(fileLine,STRING_LENGTH,fp) && strnicmp(fileLine,channelString,channelStringLength));

  // Read the solar irradiance measured at the requested channel

  if (!strnicmp(fileLine,channelString,channelStringLength))
   {
    // Retrieve the number of points

    sscanf(fileLine,"Channel %d %f %f %d",&channelNumber,&wavelStart,&wavelEnd,&npts);

    if (npts>MAX_PIXELS)
     npts=MAX_PIXELS;

    // Read out the wavelength calibration, the solar irradiance and the absolute error

    for (i=0;(i<npts) && fgets(fileLine,STRING_LENGTH,fp);i++)
     sscanf(fileLine,"%lf %lf %lf",&lambda[i],&solar[i],&solarE[i]);
   }

  // Return

  return npts;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReadSolar
// -----------------------------------------------------------------------------
// PURPOSE       Read out the irradiance section and return the irradiance spectrum
//               measured at a specific channel
//
// INPUT         fp      : pointer to the GOME level-1 file to read out
//               channel : channel number of the irradiance spectrum to return
//
// OUTPUT        pDate   : measurement date of the irradiance spectrum;
//               pTime   : measurement time of the irradiance spectrum;
//               lambda  : wavelength calibration of the returned irradiance spectrum;
//               solar   : the irradiance spectrum measured at the requested channel;
//               solarE  : the absolute errors on the returned irradiance spectrum;
//               pNpts   : size of returned vectors;
//
// RETURN        1 if read out operation failed; 0 if success.
// -----------------------------------------------------------------------------

RC ReadSolar(FILE *fp,int channel,SHORT_DATE *pDate,struct time *pTime,double *lambda,double *solar,double *solarE,int *pNpts)
 {
  // Declarations

  char  fileLine[STRING_LENGTH+1],                                             // file line
         month[16];                                                             // measurement month
  int    day,year,hour,min;                                                     // integer fields of the measurement date and time
  INDEX  indexMonth;                                                            // browse months
  float  sec;                                                                   // number of seconds
  RC     rc;                                                                    // return code

  // Initializations

  memset(fileLine,0,STRING_LENGTH+1);
  memset(month,0,16);
  memset(pDate,0,sizeof(SHORT_DATE));
  memset(pTime,0,sizeof(struct time));
 *pNpts=0;
  rc=1;

  // Search for ERS information

  while (!feof(fp) && fgets(fileLine,STRING_LENGTH,fp) && strnicmp(fileLine,"Solar Spectrum",14));

  if (!strnicmp(fileLine,"Solar Spectrum",14))
   {
    // Date and time read out

    sscanf(fileLine,"Solar Spectrum %02d-%3s-%4d %02d:%02d:%f",&day,month,&year,&hour,&min,&sec);

    // Convert month string to month number

    STD_Strlwr(month);

    for (indexMonth=0;indexMonth<12;indexMonth++)
     if (!strncmp(month,months[indexMonth],3))
      break;

    // Fill measurement date and time fields

    pDate->da_day=(char)day;
    pDate->da_mon=(char)(indexMonth+1);
    pDate->da_year=(short)year;

    pTime->ti_hour=(unsigned char)hour;
    pTime->ti_min=(unsigned char)min;
    pTime->ti_sec=(unsigned char)(int)(sec+0.5);

    // Read the irradiance spectrum measured at the requested channel

    if ((channel>=CHANNEL_1) && (channel<=CHANNEL_4))
     *pNpts=ReadSolarChannel(fp,channel,lambda,solar,solarE);

    if (*pNpts>0)
     rc=0;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GotoSpectra
// -----------------------------------------------------------------------------
// PURPOSE       Goto earthshine spectra section
//
// INPUT         fp      : pointer to the GOME level-1 file to read out
//
// RETURN        1 if read out operation failed; 0 if success.
// -----------------------------------------------------------------------------

RC GotoSpectra(FILE *fp)
 {
  // Declarations

  char  fileLine[STRING_LENGTH+1];                                             // file line
  RC     rc;                                                                    // return code

  // Initializations

  memset(fileLine,0,STRING_LENGTH+1);
  rc=1;

  // Search for earthshine spectra

  while (!feof(fp) && fgets(fileLine,STRING_LENGTH,fp) && strnicmp(fileLine,"Earthshine Spectrum",19));

  if (!strnicmp(fileLine,"Earthshine Spectrum",19))
   rc=0;

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GotoSpectraNumber
// -----------------------------------------------------------------------------
// PURPOSE       Goto a specified spectrum
//
// INPUT         fp       : pointer to the GOME level-1 file to read out
//               band     : requested band;
//               recordNo : requested record
//
// RETURN        1 if read out operation failed; 0 if success.
// -----------------------------------------------------------------------------

RC GotoSpectraNumber(FILE *fp,int band,int recordNo)
 {
  // Declarations

  char  fileLine[STRING_LENGTH+1];                                             // file line
  char  bandStr[10];                                                           // band string to search for
  int    bandStrLen;                                                            // size of the band string
  INDEX  nSpec;                                                                 // number of spectra
  RC     rc;                                                                    // return code

  // Initializations

  memset(fileLine,0,STRING_LENGTH+1);
  sprintf(bandStr,"band %s",bands[band]);
  bandStrLen=strlen(bandStr);
  nSpec=0;
  rc=1;

  // Goto the beginning of earthshine spectra section

  fseek(fp,0L,SEEK_SET);
  GotoSpectra(fp);

  // Search for earthshine spectra

  while ((nSpec<recordNo) && !feof(fp) && fgets(fileLine,STRING_LENGTH,fp))
   if (!strnicmp(fileLine,bandStr,bandStrLen))
    nSpec++;

  if (nSpec==recordNo)
   rc=0;

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReadPixelInfo
// -----------------------------------------------------------------------------
// PURPOSE       Return information on the current pixel
//
// INPUT         fp        : pointer to the GOME level-1 file to read out
//
// OUTPUT        pEngineContext : data on the current GOME pixel;
//
// RETURN        1 if read out operation failed; 0 if success.
// -----------------------------------------------------------------------------

RC ReadPixelInfo(FILE *fp,ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  char  fileLine[STRING_LENGTH+1],                                             // file line
         month[16];                                                             // measurement month
  int    day,year,hour,min,                                                     // integer fields of the measurement date and time
         nLines;                                                                // number of lines with information about the current pixel
  INDEX  indexMonth;                                                            // browse months
  float  sec;                                                                   // number of seconds
  RC     rc;                                                                    // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;

  memset(fileLine,0,STRING_LENGTH+1);
  memset(month,0,16);
  nLines=0;
  rc=1;

  // Read the measurement date and time

  if (fgets(fileLine,STRING_LENGTH,fp))
   {
    sscanf(fileLine,"%02d-%3s-%4d %02d:%02d:%f",&day,month,&year,&hour,&min,&sec);

    // Convert month string to month number

    STD_Strlwr(month);

    for (indexMonth=0;indexMonth<12;indexMonth++)
     if (!strncmp(month,months[indexMonth],3))
      break;

    // Fill measurement date and time fields

    pRecord->present_datetime.thedate.da_day=(char)day;
    pRecord->present_datetime.thedate.da_mon=(char)(indexMonth+1);
    pRecord->present_datetime.thedate.da_year=(short)year;

    pRecord->present_datetime.thetime.ti_hour=(unsigned char)hour;
    pRecord->present_datetime.thetime.ti_min=(unsigned char)min;
    pRecord->present_datetime.thetime.ti_sec=(unsigned char)(int)(sec+0.5);

    nLines++;
   }

  // Read out solar zenith and azimuth angles

  if (fgets(fileLine,STRING_LENGTH,fp))
   {
    sscanf(fileLine,"%f %f %f %f %f %f",
          &pRecord->gome.sza[0],&pRecord->gome.azim[0],
          &pRecord->gome.sza[1],&pRecord->gome.azim[1],
          &pRecord->gome.sza[2],&pRecord->gome.azim[2]);

    nLines++;
   }

  // Bypass some information

  if (fgets(fileLine,STRING_LENGTH,fp))                                         // line of sight solar angles
   nLines++;
  if (fgets(fileLine,STRING_LENGTH,fp))                                         // spacecraft angles
   nLines++;
  if (fgets(fileLine,STRING_LENGTH,fp))                                         // line of sight spacecraft angles
   nLines++;
  if (fgets(fileLine,STRING_LENGTH,fp))                                         // satellite height
   nLines++;

  // Geolocation data

  if (fgets(fileLine,STRING_LENGTH,fp))
   {
    sscanf(fileLine,"%f %f %f %f %f %f %f %f %f %f",
          &pRecord->gome.latit[0],&pRecord->gome.longit[0],
          &pRecord->gome.latit[1],&pRecord->gome.longit[1],
          &pRecord->gome.latit[2],&pRecord->gome.longit[2],
          &pRecord->gome.latit[3],&pRecord->gome.longit[3],
          &pRecord->gome.latit[4],&pRecord->gome.longit[4]);

    nLines++;
   }

  if (nLines==7)
   rc=0;

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ReadSpectrum
// -----------------------------------------------------------------------------
// PURPOSE       Return the next earthshine spectra measured at a specific band
//
// INPUT         fp        : pointer to the GOME level-1 file to read out
//               band      : band of the earthshine spectrum to return
//
// OUTPUT        pEngineContext : data on the current GOME pixel;
//               lambda    : wavelength calibration of the returned irradiance spectrum;
//               earth     : the earthshine spectrum measured at the requested band;
//               earthE    : the absolute errors on the returned earthshine spectrum;
//               pNpts     : size of returned vectors;
//
// RETURN        1 if read out operation failed; 0 if success.
// -----------------------------------------------------------------------------

RC ReadSpectrum(FILE *fp,int band,ENGINE_CONTEXT *pEngineContext,double *lambda,double *earth,double *earthE,int *pNpts)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  char  fileLine[STRING_LENGTH+1],                                             // file line
         bandStr[3];                                                            // current band type
  int    nBands,npts;                                                           // resp. the number of bands in the current record and the number of points in the current band
  float  wavelStart,wavelEnd;                                                   // the wavelength range
  INDEX  i;                                                                     // browse elements of vectors
  RC     rc;                                                                    // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;

  memset(fileLine,0,STRING_LENGTH+1);
 *pNpts=0;
  rc=1;

  // Browse lines of the orbit file

  while (rc && !feof(fp) && fgets(fileLine,STRING_LENGTH,fp))
   {
    // Retrieve information on the current ground pixel

    if (!strnicmp(fileLine,"Ground Pixel",12))
     {
      sscanf(fileLine,"Ground Pixel %d %d %d",&pRecord->gome.pixelNumber,&nBands,&pRecord->gome.pixelType);

      if (ReadPixelInfo(fp,pEngineContext))
       break;
     }

    // Read a band

    if (!strnicmp(fileLine,"Band",4))
     {
      sscanf(fileLine,"Band %[^' '] %lf %f %f %d",bandStr,&pRecord->Tint,&wavelStart,&wavelEnd,&npts);

      if ((pRecord->gome.pixelNumber>0) && !strnicmp(bandStr,bands[band],strlen(bands[band])))
       {
        if ((lambda==NULL) || (earth==NULL) || (earthE==NULL))
         rc=0;
        else
         {
          if (npts>MAX_PIXELS)
           npts=MAX_PIXELS;

          for (i=0;(i<npts) && fgets(fileLine,STRING_LENGTH,fp);i++)
           sscanf(fileLine,"%lf %lf %lf",&lambda[i],&earth[i],&earthE[i]);

          if (i==npts)
           {
            *pNpts=npts;
            rc=0;
           }
         }

        break;
       }

      // if the current band is not the requested one, by pass file lines

      else
       for (i=0;i<npts;i++)
        if (feof(fp) || !fgets(fileLine,STRING_LENGTH,fp))
         break;
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GDP_ASC_Set
// -----------------------------------------------------------------------------
// PURPOSE       Load the irradiance spectrum measured at the specified band and
//               determine the number of records
//
// INPUT         specFp      pointer to the GOME orbit file
//
// OUTPUT        pEngineContext   pointer to a structure whose some fields are filled
//                           with general data on the file
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  the input file pointer 'specFp' is NULL;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif

RC GDP_ASC_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  char bandStr[10];                                                            // band string
  GOME_DATA *pGome;                                                             // data specific to GOME
  int band,                                                                     // the user-requested band for spectra
      npts;                                                                     // the real number of points for GOME spectra

  RC rc;                                                                        // return code

  // Initializations

  gdpLastRecord=ITEM_NONE;
  pRecord=&pEngineContext->recordInfo;
  sprintf(bandStr,"Band %s",bands[pEngineContext->project.instrumental.user]);
  pGome=&pRecord->gome;

  pEngineContext->recordNumber=0;
  pRecord->longitude=
  pRecord->latitude=
  pRecord->altitude=(double)0.;
  pRecord->useErrors=1;                                                       // this format includes errors

  band=pEngineContext->project.instrumental.user;

  GDP_ASC_ReleaseBuffers();

  rc=ERROR_ID_NO;

  if (specFp==NULL)
   rc=ERROR_SetLast("SetGDP",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else
   {
    fseek(specFp,0L,SEEK_SET);

    // Irradiance vectors allocation

    if (((GDP_ASC_refL=(double *)MEMORY_AllocDVector("GDP_ASC_Set ","GDP_ASC_refL",0,MAX_PIXELS-1))==NULL) ||
        ((GDP_ASC_ref=(double *)MEMORY_AllocDVector("GDP_ASC_Set ","GDP_ASC_ref",0,MAX_PIXELS-1))==NULL) ||
        ((GDP_ASC_refE=(double *)MEMORY_AllocDVector("GDP_ASC_Set ","GDP_ASC_refE",0,MAX_PIXELS-1))==NULL))

     rc=ERROR_ID_ALLOC;

    // Load the irradiance spectrum

    else if (!ReadERS(specFp,&pRecord->satellite.orbit_number) &&
             !ReadSolar(specFp,Band2Channel(band),&pGome->irradDate,&pGome->irradTime,
                        GDP_ASC_refL,GDP_ASC_ref,GDP_ASC_refE,&pGome->nRef) &&
             !GotoSpectra(specFp))
     {
       pRecord->satellite.orbit_number += 1; // 0-based?
      if ((THRD_id==THREAD_TYPE_SPECTRA) && (THRD_browseType==THREAD_BROWSE_DARK))
       pEngineContext->recordNumber=1;
      else
       {
        // Get the number of records

        while (!ReadSpectrum(specFp,band,pEngineContext,NULL,NULL,NULL,&npts))
         pEngineContext->recordNumber++;
       }

      if (!pEngineContext->recordNumber)
       rc=ERROR_SetLast("SetGDP",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);

      NDET[0]=pGome->nRef;
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GDP_ASC_Read
// -----------------------------------------------------------------------------
// PURPOSE       GDP spectra read out (in the ASCII format)
//
// INPUT         recordNo     index of record in file
//               dateFlag     0 no date constraint; 1 a date selection is applied
//               specFp       pointer to the spectra file
//
// OUTPUT        pEngineContext  : data on the current record
//
// RETURN        ERROR_ID_ALLOC          : a buffer allocation failed;
//               ERROR_ID_FILE_END       : the end of the file is reached;
//               ERROR_ID_FILE_RECORD    : the record doesn't satisfy user constraints
//               ERROR_ID_NO             : otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif

RC GDP_ASC_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,FILE *specFp)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  double *lambda,*spectrum,*errors,                                             // use substitution variables
         *spectrum2,*specInt;                                                   // temporary buffers
  GOME_DATA *pGome;                                                             // pointer to the GOME part of the pEngineContext structure
  int band;                                                                     // the band to read out
  int npts;                                                                     // the size of returned vectors
  RC rc;                                                                        // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;

  lambda=pEngineContext->buffers.lambda;
  spectrum=pEngineContext->buffers.spectrum;
  errors=pEngineContext->buffers.sigmaSpec;
  spectrum2=specInt=NULL;
  band=pEngineContext->project.instrumental.user;
  pGome=&pRecord->gome;
  rc=ERROR_ID_NO;

  const int n_wavel = NDET[0];

  // Goto the requested record

  if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber)) {
    rc=ERROR_ID_FILE_END;
  } else if ((THRD_id==THREAD_TYPE_SPECTRA) && (THRD_browseType==THREAD_BROWSE_DARK)) {
    memcpy(lambda,GDP_ASC_refL,sizeof(double)*n_wavel);
    memcpy(spectrum,GDP_ASC_ref,sizeof(double)*n_wavel);
    memcpy(errors,GDP_ASC_refE,sizeof(double)*n_wavel);
   }
  else if ((recordNo-gdpLastRecord==1) ||                                       // for successive spectra, don't move file pointer
          !(rc=GotoSpectraNumber(specFp,band,recordNo-1)))
   {
    gdpLastRecord=recordNo;

    // Read out the spectrum

    if (ReadSpectrum(specFp,band,pEngineContext,lambda,spectrum,errors,&npts))
     rc=ERROR_ID_FILE_END;

    // Interpolate the earthshine spectrum on the irradiance grid

    else if (((spectrum2=(double *)MEMORY_AllocDVector("GDP_ASC_Read ","spectrum2",0,MAX_PIXELS-1))==NULL) ||
             ((specInt=(double *)MEMORY_AllocDVector("GDP_ASC_Read ","specInt",0,MAX_PIXELS-1))==NULL))
     rc=ERROR_ID_ALLOC;
    else if (!(rc=SPLINE_Deriv2(lambda,spectrum,spectrum2,npts,"GDP_ASC_Read ")) &&
             !(rc=SPLINE_Vector(lambda,spectrum,spectrum2,npts,GDP_ASC_refL,specInt,n_wavel,SPLINE_CUBIC,"GDP_ASC_Read ")))

    // Get information on the current GOME pixel

     {
      pRecord->TDet         = (double)0.;
      pRecord->NSomme       = 1;
      pRecord->Zm           = (double)pGome->sza[1];
      pRecord->Azimuth      = (double)pGome->azim[1];
      pRecord->SkyObs       = 0;
      pRecord->rejected     = 0;
      pRecord->ReguTemp     = 0.;
      pRecord->TotalExpTime = pRecord->TotalAcqTime = pRecord->Tint*pRecord->NSomme;
      pRecord->Tm           =(double)ZEN_NbSec(&pRecord->present_datetime.thedate,&pRecord->present_datetime.thetime,0);
      pRecord->TimeDec      =(double)pRecord->present_datetime.thetime.ti_hour+pRecord->present_datetime.thetime.ti_min/60.;
      pRecord->longitude    = pGome->longit[4];
      pRecord->latitude     = pGome->latit[4];
      pRecord->altitude     = (double) 0.;

      memcpy(lambda,GDP_ASC_refL,sizeof(double)*n_wavel);
      memcpy(spectrum,specInt,sizeof(double)*n_wavel);
     }
   }

  // Release allocated buffers

  if (spectrum2!=NULL)
   MEMORY_ReleaseDVector("GDP_ASC_Read ","spectrum2",spectrum2,0);
  if (specInt!=NULL)
   MEMORY_ReleaseDVector("GDP_ASC_Read ","specInt",specInt,0);

  // Return

  return rc;
 }

// ========
// ANALYSIS
// ========

// -----------------------------------------------------------------------------
// FUNCTION      GDP_ASC_LoadAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Load analysis parameters depending on the irradiance spectrum
//
// INPUT         pEngineContext    data on the current file
//               specFp       pointer to the current file
//
// RETURN        0 for success
// -----------------------------------------------------------------------------

RC GDP_ASC_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  INDEX indexFeno,indexTabCross,indexWindow,i;
  CROSS_REFERENCE *pTabCross;
  WRK_SYMBOL *pWrkSymbol;
  FENO *pTabFeno;
  double factTemp,lambdaMin,lambdaMax;
  int DimL,degree,invFlag,useUsamp,useKurucz,saveFlag,refSelectionFlag;
  RC rc;

  // Initializations

  saveFlag=(int)pEngineContext->project.spectra.displayDataFlag;
  refSelectionFlag=0;

  lambdaMin=(double)9999.;
  lambdaMax=(double)-9999.;

  rc=ERROR_ID_NO;
  useKurucz=useUsamp=0;

  const int n_wavel = NDET[0];

  // Browse analysis windows and load missing data

  for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
   if (!TabFeno[0][indexFeno].hidden)
    {
     pTabFeno=&TabFeno[0][indexFeno];
     pTabFeno->NDET=n_wavel;

     // Load calibration and reference spectra

     if (!pTabFeno->gomeRefFlag)
      {
       memcpy(pTabFeno->LambdaRef,GDP_ASC_refL,sizeof(double)*n_wavel);
       memcpy(pTabFeno->Sref,GDP_ASC_ref,sizeof(double)*n_wavel);
       memcpy(pTabFeno->SrefSigma,GDP_ASC_refE,sizeof(double)*n_wavel);

       if (!(rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,n_wavel,&pTabFeno->refNormFact,"GDP_ASC_LoadAnalysis (Reference) ")) &&
           !(rc=VECTOR_NormalizeVector(pTabFeno->SrefSigma-1,n_wavel,&factTemp,"GDP_ASC_LoadAnalysis (RefError) ")))
        {
         memcpy(pTabFeno->SrefEtalon,pTabFeno->Sref,sizeof(double)*n_wavel);
         pTabFeno->useEtalon=pTabFeno->displayRef=1;

         // Browse symbols

         for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
          {
           pTabCross=&pTabFeno->TabCross[indexTabCross];
           pWrkSymbol=&WorkSpace[pTabCross->Comp];

           // Cross sections and predefined vectors

           if ((((pWrkSymbol->type==WRK_SYMBOL_CROSS) && (pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING)) ||
                ((pWrkSymbol->type==WRK_SYMBOL_PREDEFINED) &&
                ((indexTabCross==pTabFeno->indexCommonResidual) ||
               (((indexTabCross==pTabFeno->indexUsamp1) || (indexTabCross==pTabFeno->indexUsamp2)) && (pUsamp->method==PRJCT_USAMP_FILE))))) &&
                ((rc=ANALYSE_CheckLambda(pWrkSymbol,pTabFeno->LambdaRef,pTabFeno->NDET))!=ERROR_ID_NO))

            goto EndGDP_ASC_LoadAnalysis;

           // Continuous functions

           if (pWrkSymbol->type==WRK_SYMBOL_CONTINUOUS)
            {
             invFlag=0;

             if (pWrkSymbol->symbolName[0]=='x')
              sscanf(pWrkSymbol->symbolName,"x%d",&degree);

             if (!invFlag)
              for (i=0;i<n_wavel;i++)
               pTabCross->vector[i]=(double)pow(pTabFeno->LambdaRef[i],degree);
            }
          }

         if ((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,0))!=ERROR_ID_NO)
          goto EndGDP_ASC_LoadAnalysis;

         // Gaps : rebuild subwindows on new wavelength scale

         doas_spectrum *new_range = spectrum_new();
         for (indexWindow = 0, DimL=0; indexWindow < pTabFeno->fit_properties.Z; indexWindow++)
          {
           int pixel_start = FNPixel(pTabFeno->LambdaRef,pTabFeno->fit_properties.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
           int pixel_end = FNPixel(pTabFeno->LambdaRef,pTabFeno->fit_properties.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

           spectrum_append(new_range, pixel_start, pixel_end);

           DimL += pixel_end - pixel_start +1;
          }

         // Buffers allocation
         FIT_PROPERTIES_free(__func__,&pTabFeno->fit_properties);
         pTabFeno->fit_properties.DimL=DimL;
         FIT_PROPERTIES_alloc(__func__,&pTabFeno->fit_properties);
         // new spectral windows
         pTabFeno->fit_properties.specrange = new_range;

         pTabFeno->Decomp=1;
        }

       memcpy(pTabFeno->LambdaK,pTabFeno->LambdaRef,sizeof(double)*n_wavel);
       memcpy(pTabFeno->Lambda,pTabFeno->LambdaRef,sizeof(double)*n_wavel);

       useUsamp+=pTabFeno->useUsamp;
       useKurucz+=pTabFeno->useKurucz;

       refSelectionFlag+=(pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC)?1:0;

       if (pTabFeno->useUsamp)
        {
         if (pTabFeno->LambdaRef[0]<lambdaMin)
          lambdaMin=pTabFeno->LambdaRef[0];
         if (pTabFeno->LambdaRef[n_wavel-1]>lambdaMax)
          lambdaMax=pTabFeno->LambdaRef[n_wavel-1];
        }
      }
    }

  // Kurucz

  if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
   {
    KURUCZ_Init(0,0);

    if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,NULL /* QDOAS !!! responseHandle */,0))!=ERROR_ID_NO))
     goto EndGDP_ASC_LoadAnalysis;
   }

  // Build undersampling cross sections

  if (useUsamp && (THRD_id!=THREAD_TYPE_KURUCZ))
   {
     // ANALYSE_UsampLocalFree();

    if (((rc=ANALYSE_UsampLocalAlloc(0))!=ERROR_ID_NO) ||
        ((rc=ANALYSE_UsampBuild(0,0))!=ERROR_ID_NO) ||
        ((rc=ANALYSE_UsampBuild(1,ITEM_NONE))!=ERROR_ID_NO))

     goto EndGDP_ASC_LoadAnalysis;
   }

  // Return

  EndGDP_ASC_LoadAnalysis :

  return rc;
 }

// use curr_utc_day, taken from the ERS header of the current orbit
// file, to get the date
void GDP_ASC_get_orbit_date(int *year, int *month, int *day) {
  struct tm start_utc = { .tm_sec = 0,
                          .tm_min = 0,
                          .tm_hour = 0,
                          .tm_mday = 1,
                          .tm_mon = 0, // month since jan (-> 0-11)
                          .tm_year = 50, // year since 1900
                          .tm_isdst = 0 };

  // get seconds since epoch of 1/1/1950 00:00:00 (GMT)
#ifndef _WIN32
  time_t time_epoch = timegm(&start_utc);
#else
  // get UTC time on MinGW32:

  // get current timezone setting
  char *timezone_old = getenv("TZ");
  // set timezone to UTC for mktime
  putenv("TZ=UTC");
  time_t time_epoch = mktime(&start_utc);
  if (timezone_old != NULL) {
    // restore environment
    putenv(timezone_old);
  } else {
    // no timezone was set, just remove our setting
    putenv("TZ="); // for MinGW32, we use this instead of unsetenv("TZ")
  }
#endif

  time_epoch += curr_utc_day * 24 * 3600;
  struct tm result;

#ifndef _WIN32
  gmtime_r(&time_epoch, &result);
#else
  struct tm *time = gmtime(&time_epoch);
  result = *time;
#endif

  *year = result.tm_year + 1900;
  *month = result.tm_mon + 1;
  *day = result.tm_mday;
}

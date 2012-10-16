
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  OMI interface
//  Name of module    :  OMIRead.C
//  Program Language  :  C/C++
//  Creation date     :  4 April 2007
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
//      thomasd@aeronomie.be                        info@stcorp.nl
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
//
//
//  LIBRARIES
//
//  This module uses HDF-EOS (Hierarchical Data Format - Earth Observing System)
//  libraries based on HDF-4
//
//  ----------------------------------------------------------------------------

// ========
// INCLUDES
// ========

#include "mediate.h"
#include "engine.h"
#include "hdf.h"
#include "HdfEosDef.h"

#define MAX_OMI_FILES 500
#define REFERENCE_COLUMN "WavelengthReferenceColumn"
#define WAVELENGTH_COEFFICIENT "WavelengthCoefficient"
#define OMI_NUM_COEFFICIENTS 5 // 5 coefficients in wavelenght polynomial
#define RADIANCE_MANTISSA "RadianceMantissa"
#define RADIANCE_PRECISION_MANTISSA "RadiancePrecisionMantissa"
#define RADIANCE_EXPONENT "RadianceExponent"
#define IRRADIANCE_MANTISSA "IrradianceMantissa"
#define IRRADIANCE_PRECISION_MANTISSA "IrradiancePrecisionMantissa"
#define IRRADIANCE_EXPONENT "IrradianceExponent"
#define PIXEL_QUALITY_FLAGS "PixelQualityFlags"

void omi_calculate_wavelengths(float32 wavelength_coeff[], int16 refcol, int32 n_wavel, double* lambda);
void omi_make_double(int16 mantissa[], int8 exponent[], int32 n_wavel, double* result);
void omi_interpolate_errors(int16 mantissa[], int32 n_wavel, double wavelengths[], double y[] );
RC omi_load_spectrum(int spec_type, int32 sw_id, int32 measurement, int32 track, int32 n_wavel, double *lambda, double *spectrum, double *sigma, DoasUS *pixelQualityFlags);

// ========================
// DEFINITION OF STRUCTURES
// ========================

enum _omiSwathType { OMI_SWATH_UV1, OMI_SWATH_UV2, OMI_SWATH_VIS, OMI_SWATH_MAX };
enum _omiVdataType { OMI_VDATA_GEO, OMI_VDATA_DATA, OMI_VDATA_ATTR, OMI_VDATA_MAX };
enum _omiSpecType  { OMI_SPEC_IRRAD, OMI_SPEC_RAD };

struct omi_field {
char *fieldname;
void *target;
};

// Geolocation fields

typedef struct _omi_geo
{
  double         *time;
  float          *secondsInDay;
  float          *spacecraftLatitude;
  float          *spacecraftLongitude;
  float          *spacecraftAltitude;
  float          *latitude;
  float          *longitude;
  float          *solarZenithAngle;
  float          *solarAzimuthAngle;
  float          *viewingZenithAngle;
  float          *viewingAzimuthAngle;
  short          *terrainHeight;
  unsigned short *groundPixelQualityFlags;
  uint8_t        *xtrackQualityFlags;
}
  OMI_GEO;

// Data fields

typedef struct _omi_spectrum
{
  DoasUS *pixelQualityFlags;
  float   wavelengthCoefficient[5];
  float   wavelengthCoefficientPrecision[5];
}
  OMI_SPECTRUM;

typedef struct _omi_ref
{
  double       **omiRefLambda;
  double       **omiRefSpectrum;
  double       **omiRefLambdaK;
  double       **omiRefSigma;
  DoasCh         omiRefFileName[MAX_STR_LEN+1];
  OMI_SPECTRUM   spectrum;
  long           omiNumberOfSpectraPerSwath,omiNumDataPoints;
  double        *omiRefFact;
  int            year,cday;
}
  OMI_REF;

typedef struct _omi_data_fields
{
  short   *wavelengthReferenceColumn;
  DoasCh   *measurementClass;
  DoasCh   *instrumentConfigurationId;
  DoasCh   *instrumentConfigurationVersion;
  DoasUS  *measurementQualityFlags;
  char    *numberSmallPixelColumns;
  char    *exposureType;
  float   *masterClockPeriod;
  DoasUS  *calibrationSettings;
  float   *exposureTime;
  float   *readoutTime;
  short   *smallPixelColumn;
  short   *gainSwitchingColumn1;
  short   *gainSwitchingColumn2;
  short   *gainSwitchingColumn3;
  char    *gainCode1;
  char    *gainCode2;
  char    *gainCode3;
  char    *gainCode4;
  char    *dSGainCode;
  char    *lowerStrayLightAreaBinningFactor;
  char    *upperStrayLightAreaBinningFactor;
  char    *lowerDarkAreaBinningFactor;
  char    *upperDarkAreaBinningFactor;
  short   *skipRows1;
  short   *skipRows2;
  short   *skipRows3;
  short   *skipRows4;
  float   *detectorTemperature;
  float   *opticalBenchTemperature;
  char    *imageBinningFactor;
  short   *binnedImageRows;
  short   *stopColumn;
}
  OMI_DATA_FIELDS;

// Swath attributes                                                             // Can not be retrieved with BEAT

typedef struct _omi_swath_attr
{
  int32_t  numTimes;
  int32_t  numTimesSmallPixel;
  float    earthSunDistance;
}
  OMI_SWATH_ATTR;

// Definition of a swath
//     - UV-1 Swath
//     - UV-2 Swath
//     - VIS Swath

typedef struct _omi_swath_earth
{
  OMI_GEO         geolocationFields;
  OMI_SPECTRUM    spectrum;
  OMI_DATA_FIELDS dataFields;
  // OMI_SWATH_ATTR  swathAttributes;                                           // unuseful with BEAT
}
  OMI_SWATH;

typedef struct _OMIOrbitFiles                                                   // description of an orbit
{
  DoasCh              omiFileName[MAX_STR_LEN+1];                               // the name of the file with a part of the orbit
  DoasCh              omiSwathName[MAX_STR_LEN+1];                              // the name of the selected swath (useful to select fields)
  OMI_SWATH           omiSwath;                                                 // all information about the swath
  INDEX              *omiLatIndex,*omiLonIndex,*omiSzaIndex;                    // indexes of records sorted resp. by latitude, by longitude and by SZA
  SATELLITE_GEOLOC   *omiGeolocations;                                          // geolocations
  INT                 specNumber;
  int32 swf_id, // hdfeos swath file id
    sw_id;      // hdfeos swath id
  long                omiNumberOfSwaths,omiNumberOfSpectraPerSwath,omiNumDataPoints;
  int                 version;
  INT                 rc;
}
  OMI_ORBIT_FILE;

// ================
// STATIC VARIABLES
// ================

DoasCh *OMI_EarthSwaths[OMI_SWATH_MAX]={"Earth UV-1 Swath","Earth UV-2 Swath","Earth VIS Swath"};
DoasCh *OMI_SunSwaths[OMI_SWATH_MAX]={"Sun Volume UV-1 _Swath","Sun Volume UV-2 Swath","Sun Volume VIS Swath"};

static OMI_ORBIT_FILE omiOrbitFiles[MAX_OMI_FILES];                             // list of files per day
static int omiOrbitFilesN=0,omiRefFilesN=0;                                     // the total number of files to browse in one shot
static INDEX omiCurrentFileIndex=ITEM_NONE;                                     // index of the current file in the list
static INT omiTotalRecordNumber=0;
static OMI_REF OMI_ref[MAX_FENO];                                               // the number of reference spectra is limited to the maximum number of analysis windows in a project

int OMI_beatLoaded=0;
int OMI_ms=0;
int omiSwathOld=ITEM_NONE;

void omi_calculate_wavelengths(float32 wavelength_coeff[], int16 refcol, int32 n_wavel, double* lambda);

// ===================
// ALLOCATION ROUTINES
// ===================

void OMI_ReleaseReference(void)
{
  // Declarations

  OMI_REF *pRef;
  INDEX i;

  // Initialization

  for (i=0;i<omiRefFilesN;i++)
    {
      pRef=&OMI_ref[i];

      if (pRef->omiRefLambda!=NULL)
	MEMORY_ReleaseDMatrix("OMI_ReleaseReference","omiRefLambda",pRef->omiRefLambda,0,pRef->omiNumberOfSpectraPerSwath-1,0);
      if (pRef->omiRefSpectrum!=NULL)
	MEMORY_ReleaseDMatrix("OMI_ReleaseReference","omiRefSpectrum",pRef->omiRefSpectrum,0,pRef->omiNumberOfSpectraPerSwath-1,0);
      if (pRef->omiRefLambdaK!=NULL)
	MEMORY_ReleaseDMatrix("OMI_ReleaseReference","omiRefLambdaK",pRef->omiRefLambdaK,0,pRef->omiNumberOfSpectraPerSwath-1,0);
      if (pRef->omiRefFact!=NULL)
	MEMORY_ReleaseDVector("OMI_ReleaseReference","omiRefSpectrumK",pRef->omiRefFact,0);
      if (pRef->omiRefSigma!=NULL)
	MEMORY_ReleaseDMatrix("OMI_ReleaseReference","omiRefSigma",pRef->omiRefSigma,0,pRef->omiNumberOfSpectraPerSwath-1,0);

      if (pRef->spectrum.pixelQualityFlags!=NULL)                                          // pixelquality
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","pRef->spectrum.pixelQualityFlags",pRef->spectrum.pixelQualityFlags);

      memset(pRef,0,sizeof(OMI_ref[i]));
    }

  omiRefFilesN=0;
}

RC OMI_AllocateReference(INDEX indexRef,int nSpectra,int nPoints)
{
  // Declarations

  OMI_REF *pRef;
  RC rc;

  // Initializations

  pRef=&OMI_ref[indexRef];
  rc=ERROR_ID_NO;

  if (((pRef->omiRefLambda=(double **)MEMORY_AllocDMatrix("OMI_AllocateReference","omiRefLambda",0,nPoints-1,0,nSpectra-1))==NULL) ||
      ((pRef->omiRefSpectrum=(double **)MEMORY_AllocDMatrix("OMI_AllocateReference","omiRefSpectrum",0,nPoints-1,0,nSpectra-1))==NULL) ||
      ((pRef->omiRefLambdaK=(double **)MEMORY_AllocDMatrix("OMI_AllocateReference","omiRefLambdaK",0,nPoints-1,0,nSpectra-1))==NULL) ||
      ((pRef->omiRefFact=(double *)MEMORY_AllocDVector("OMI_AllocateReference","omiRefFact",0,nSpectra-1))==NULL) ||
      ((pRef->omiRefSigma=(double **)MEMORY_AllocDMatrix("OMI_AllocateReference","omiRefSigma",0,nPoints-1,0,nSpectra-1))==NULL) ||

      ((pRef->spectrum.pixelQualityFlags=(unsigned short *)MEMORY_AllocBuffer("OMI_AllocateReference","pixelQualityFlags",nPoints,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL))

    rc=ERROR_ID_ALLOC;

  pRef->omiNumberOfSpectraPerSwath=nSpectra;
  pRef->omiNumDataPoints=nPoints;

  // Return

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      OMI_ReleaseBuffers
// -----------------------------------------------------------------------------
// PURPOSE       Release buffers allocated by OMI readout routines
// -----------------------------------------------------------------------------

void OMI_ReleaseBuffers(void)
{
  // Declarations

  OMI_ORBIT_FILE *pOrbitFile;
  INDEX omiOrbitFileIndex;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("OMI_ReleaseBuffers",DEBUG_FCTTYPE_FILE|DEBUG_FCTTYPE_MEM);
#endif

  for (omiOrbitFileIndex=0;omiOrbitFileIndex<omiOrbitFilesN;omiOrbitFileIndex++)
    {
      pOrbitFile=&omiOrbitFiles[omiOrbitFileIndex];

      if (pOrbitFile->omiGeolocations!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","omiGeolocations",pOrbitFile->omiGeolocations);
      if (pOrbitFile->omiLatIndex!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","omiLatIndex",pOrbitFile->omiLatIndex);
      if (pOrbitFile->omiLonIndex!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","omiLonIndex",pOrbitFile->omiLonIndex);
      if (pOrbitFile->omiSzaIndex!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","omiSzaIndex",pOrbitFile->omiSzaIndex);

      // Buffers related to spectra and calibration

      if (pOrbitFile->omiSwath.spectrum.pixelQualityFlags!=NULL)                                    //pixel quality
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","pOrbitFile->omiSwath.spectrum.pixelQualityFlags",pOrbitFile->omiSwath.spectrum.pixelQualityFlags);


      // Earth swath data fields

      if (pOrbitFile->omiSwath.dataFields.measurementQualityFlags!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","measurementQualityFlags",pOrbitFile->omiSwath.dataFields.measurementQualityFlags);
      if (pOrbitFile->omiSwath.dataFields.exposureTime!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","exposureTime",pOrbitFile->omiSwath.dataFields.exposureTime);
      if (pOrbitFile->omiSwath.dataFields.detectorTemperature!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","detectorTemperature",pOrbitFile->omiSwath.dataFields.detectorTemperature);
      if (pOrbitFile->omiSwath.dataFields.wavelengthReferenceColumn!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","wavelengthReferenceColumn",pOrbitFile->omiSwath.dataFields.wavelengthReferenceColumn);

      // Geolocation fields

      if (pOrbitFile->omiSwath.geolocationFields.time!=NULL)
	MEMORY_ReleaseDVector("OMI_ReleaseBuffers","time",pOrbitFile->omiSwath.geolocationFields.time,0);
      if (pOrbitFile->omiSwath.geolocationFields.secondsInDay!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","secondsInDay",pOrbitFile->omiSwath.geolocationFields.secondsInDay);

      if (pOrbitFile->omiSwath.geolocationFields.spacecraftLatitude!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","spacecraftLatitude",pOrbitFile->omiSwath.geolocationFields.spacecraftLatitude);
      if (pOrbitFile->omiSwath.geolocationFields.spacecraftLongitude!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","spacecraftLongitude",pOrbitFile->omiSwath.geolocationFields.spacecraftLongitude);
      if (pOrbitFile->omiSwath.geolocationFields.spacecraftAltitude!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","spacecraftAltitude",pOrbitFile->omiSwath.geolocationFields.spacecraftAltitude);
      if (pOrbitFile->omiSwath.geolocationFields.latitude!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","latitude",pOrbitFile->omiSwath.geolocationFields.latitude);
      if (pOrbitFile->omiSwath.geolocationFields.longitude!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","longitude",pOrbitFile->omiSwath.geolocationFields.longitude);
      if (pOrbitFile->omiSwath.geolocationFields.solarZenithAngle!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","solarZenithAngle",pOrbitFile->omiSwath.geolocationFields.solarZenithAngle);
      if (pOrbitFile->omiSwath.geolocationFields.solarAzimuthAngle!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","solarAzimuthAngle",pOrbitFile->omiSwath.geolocationFields.solarAzimuthAngle);
      if (pOrbitFile->omiSwath.geolocationFields.viewingZenithAngle!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","viewingZenithAngle",pOrbitFile->omiSwath.geolocationFields.viewingZenithAngle);
      if (pOrbitFile->omiSwath.geolocationFields.viewingAzimuthAngle!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","viewingAzimuthAngle",pOrbitFile->omiSwath.geolocationFields.viewingAzimuthAngle);
      if (pOrbitFile->omiSwath.geolocationFields.terrainHeight!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","terrainHeight",pOrbitFile->omiSwath.geolocationFields.terrainHeight);
      if (pOrbitFile->omiSwath.geolocationFields.groundPixelQualityFlags!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","groundPixelQualityFlags",pOrbitFile->omiSwath.geolocationFields.groundPixelQualityFlags);
      if (pOrbitFile->omiSwath.geolocationFields.xtrackQualityFlags!=NULL)
	MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","xtrackQualityFlags",pOrbitFile->omiSwath.geolocationFields.xtrackQualityFlags);

      // Close the current file
      if(pOrbitFile->sw_id != 0) {
	SWdetach(pOrbitFile->sw_id);
	pOrbitFile->sw_id = 0;
      }
      if(pOrbitFile->swf_id != 0) {
	SWclose(pOrbitFile->swf_id);
	pOrbitFile->swf_id = 0;
      }
    }

  for (omiOrbitFileIndex=0;omiOrbitFileIndex<MAX_OMI_FILES;omiOrbitFileIndex++)
    memset(&omiOrbitFiles[omiOrbitFileIndex],0,sizeof(OMI_ORBIT_FILE));

  omiOrbitFilesN=0;
  omiCurrentFileIndex=ITEM_NONE;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("OMI_ReleaseBuffers",0);
#endif
}

void OMI_TrackSelection(DoasCh *omiTrackSelection,int *omiTracks)
{
  // Declarations

  DoasCh str[256],*ptr;
  int number1,number2,i,n,resetFlag,rangeFlag;

  // Initializations

  resetFlag=1;
  rangeFlag=0;
  n=0;

  number1=number2=-1;

  if (!strlen(omiTrackSelection))
    for (i=0;i<MAX_SWATHSIZE;i++)
      omiTracks[i]=1;
  else
    {
      for (ptr=omiTrackSelection;(int)(ptr-omiTrackSelection)<=256;ptr++)
	{
	  if (resetFlag)
	    {
	      memset(str,0,256);
	      n=0;
	      resetFlag=0;
	    }

	  if ((*ptr>='0') && (*ptr<='9'))
	    str[n++]=*ptr;
	  else if ((*ptr==':') || (*ptr=='-'))
	    {
	      number1=atoi(str);
	      rangeFlag=1;
	      resetFlag=1;
	    }
	  else if ((*ptr==',') || (*ptr==';') || (*ptr=='\0'))
	    {
	      number2=atoi(str);
	      resetFlag=1;

	      if (!rangeFlag)
		number1=number2;

	      if ((number1>0) && (number1<MAX_SWATHSIZE) && (number2>0) && (number2<MAX_SWATHSIZE))
		for (i=number1-1;i<number2;i++)
		  omiTracks[i]=1;

	      number1=number2=-1;
	      rangeFlag=0;
	      resetFlag=1;

	      if (*ptr=='\0')
		break;
	    }
	}
    }
}

// -----------------------------------------------------------------------------
// FUNCTION      OMI_AllocateSwath
// -----------------------------------------------------------------------------
// PURPOSE       Allocated buffers to load OMI data
//
// INPUT         pSwath     the structures with the buffers to allocate
//               nSwaths    the number of swaths
//               nSpectra   the number of spectra per swath
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer fails
//               ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC OMI_AllocateSwath(OMI_SWATH *pSwath,int nSwaths,int nSpectra)
{
  // Declarations

  OMI_SPECTRUM *pSpectrum = &pSwath->spectrum;  // spectrum in earth swath
  OMI_DATA_FIELDS *pData = &pSwath->dataFields; // data on earth swath
  OMI_GEO *pGeo = &pSwath->geolocationFields;   // geolocations
  int nRecords = nSwaths*nSpectra;              // total number of spectra
  RC rc = ERROR_ID_NO;                          // Return code

  if (
      ((pSpectrum->pixelQualityFlags=(unsigned short *)MEMORY_AllocBuffer("OMI_AllocateSwath","pixelQualityFlags",NDET,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||

      // Earth swath

      ((pData->measurementQualityFlags=(unsigned short *)MEMORY_AllocBuffer("OMI_AllocateSwath","measurementQualityFlags",nSwaths,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
      ((pData->exposureTime=(float *)MEMORY_AllocBuffer("OMI_AllocateSwath","exposureTime",nSwaths,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pData->detectorTemperature=(float *)MEMORY_AllocBuffer("OMI_AllocateSwath","detectorTemperature",nSwaths,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pData->wavelengthReferenceColumn=(short *)MEMORY_AllocBuffer("OMI_AllocateSwath","wavelengthReferenceColumn",nSwaths,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL) ||

      // Geolocation

      ((pGeo->time=(double *)MEMORY_AllocDVector("OMI_AllocateSwath","time",0,nSwaths))==NULL) ||
      ((pGeo->secondsInDay=(float *)MEMORY_AllocBuffer("OMI_AllocateSwath","secondsInDay",nSwaths,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->spacecraftLatitude=(float *)MEMORY_AllocBuffer("OMI_AllocateSwath","spacecraftLatitude",nSwaths,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->spacecraftLongitude=(float *)MEMORY_AllocBuffer("OMI_AllocateSwath","spacecraftLongitude",nSwaths,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->spacecraftAltitude=(float *)MEMORY_AllocBuffer("OMI_AllocateSwath","spacecraftAltitude",nSwaths,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->latitude=(float *)MEMORY_AllocBuffer("OMI_AllocateSwath","latitude",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->longitude=(float *)MEMORY_AllocBuffer("OMI_AllocateSwath","longitude",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->solarZenithAngle=(float *)MEMORY_AllocBuffer("OMI_AllocateSwath","solarZenithAngle",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->solarAzimuthAngle=(float *)MEMORY_AllocBuffer("OMI_AllocateSwath","solarAzimuthAngle",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->viewingZenithAngle=(float *)MEMORY_AllocBuffer("OMI_AllocateSwath","viewingZenithAngle",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->viewingAzimuthAngle=(float *)MEMORY_AllocBuffer("OMI_AllocateSwath","viewingAzimuthAngle",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->terrainHeight=(short *)MEMORY_AllocBuffer("OMI_AllocateSwath","terrainHeight",nRecords,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL) ||
      ((pGeo->groundPixelQualityFlags=(unsigned short *)MEMORY_AllocBuffer("OMI_AllocateSwath","groundPixelQualityFlags",nRecords,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
      ((pGeo->xtrackQualityFlags=(uint8_t *)MEMORY_AllocBuffer("OMI_AllocateSwath","xtrackQualityFlags",nRecords,sizeof(unsigned short),0,MEMORY_TYPE_STRING))==NULL))

    rc=ERROR_ID_ALLOC;

  // Return

  return rc;
}

// =====================
// OMI READ OUT ROUTINES
// =====================

// -----------------------------------------------------------------------------
// FUNCTION      OMI_FromTAI1993ToYMD
// -----------------------------------------------------------------------------
// PURPOSE       Convert a International Atomic Time (TAI 1993 date) in the
//               equivalent YYYY/MM/DD hh:mm:ss format
//
// INPUT         tai          the number of seconds since 1993/01/01
// OUTPUT        pDate,pTime  pointers to resp. date and time in usual format
//               pms          pointer to the number of milliseconds
// -----------------------------------------------------------------------------

void OMI_FromTAI1993ToYMD(double tai,SHORT_DATE *pDate,struct time *pTime,int *pms)
{
  // Declarations

  int year;
  int sumDays,nDaysInYear;
  int mjd;

  // Initializations

  memset(pDate,0,sizeof(SHORT_DATE));
  memset(pTime,0,sizeof(struct time));

  mjd=(int)floor(tai/86400.);

  // get the number of years since 2000

  for (year=1993,sumDays=0,nDaysInYear=365;
       sumDays+nDaysInYear<mjd;)
    {
      year++;
      sumDays+=nDaysInYear;
      nDaysInYear=((year%4)==0)?366:365;
    }

  // Get date from the year and the calendar day

  pDate->da_year=(short)year;
  pDate->da_mon=(char)ZEN_FNCaljmon(year,mjd-sumDays+1);
  pDate->da_day=(char)ZEN_FNCaljday(year,mjd-sumDays+1);

  // Get time

  tai-=(double)mjd*86400.;
  pTime->ti_hour=(char)(floor(tai/3600.));
  tai-=(double)3600.*pTime->ti_hour;
  pTime->ti_min=(char)(floor(tai/60.));
  tai-=(double)60.*pTime->ti_min;
  pTime->ti_sec=(char)(floor(tai));

  *pms=(int)((double)(tai-floor(tai))*1000.);
}

// -----------------------------------------------------------------------------
// FUNCTION      OmiGetSwathData
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve data of swath
//
// INPUT         pOrbitFile   pointer to the current orbit file
// OUTPUT        rc           return code
//
// RETURN        ERROR_ID_NO if the function succeeds
//               ERROR_ID_BEAT otherwise
// -----------------------------------------------------------------------------

RC OmiGetSwathData(OMI_ORBIT_FILE *pOrbitFile)
{
  // Initializations
  OMI_DATA_FIELDS *pData = &pOrbitFile->omiSwath.dataFields;
  OMI_GEO *pGeo = &pOrbitFile->omiSwath.geolocationFields;
  RC rc=ERROR_ID_NO;

  struct omi_field swathdata[] =
    {
      {"MeasurementQualityFlags", pData->measurementQualityFlags},
      {"ExposureTime", pData->exposureTime},
      {"DetectorTemperature", pData->detectorTemperature},
      {"WavelengthReferenceColumn", pData->wavelengthReferenceColumn},
      {"Time",pGeo->time},
      {"SecondsInDay",pGeo->secondsInDay},
      {"SpacecraftLatitude", pGeo->spacecraftLatitude},
      {"SpacecraftLongitude", pGeo->spacecraftLongitude},
      {"SpacecraftAltitude", pGeo->spacecraftAltitude},
      {"Latitude", pGeo->latitude},
      {"Longitude", pGeo->longitude},
      {"SolarZenithAngle", pGeo->solarZenithAngle},
      {"SolarAzimuthAngle", pGeo->solarAzimuthAngle},
      {"ViewingZenithAngle", pGeo->viewingZenithAngle},
      {"ViewingAzimuthAngle", pGeo->viewingAzimuthAngle},
      {"TerrainHeight", pGeo->terrainHeight},
      {"GroundPixelQualityFlags", pGeo->groundPixelQualityFlags},
      {"XTrackQualityFlags",pGeo->xtrackQualityFlags} // TODO: check for presence of xtrackQualityFlags (not present in older files)
    };

  int32 start[] = {0,0};
  int32 edge[] =  {pOrbitFile->omiNumberOfSwaths,pOrbitFile->omiNumberOfSpectraPerSwath };
  intn swrc;
  unsigned int i;
  for (i=0; i<sizeof(swathdata)/sizeof(swathdata[0]); i++) {
    swrc = SWreadfield(pOrbitFile->sw_id, swathdata[i].fieldname, start, NULL, edge, swathdata[i].target);
    if (swrc == FAIL) {
      rc = ERROR_SetLast("OmiGetSwathData",ERROR_TYPE_WARNING,ERROR_ID_HDFEOS,swathdata[i].fieldname,pOrbitFile->omiFileName,"Can not read ", swathdata[i].fieldname);
      break;
    }
  }

  // Return

  return rc;
}

RC OmiOpen(OMI_ORBIT_FILE *pOrbitFile,char *swathName)
{
  RC    rc = ERROR_ID_NO;

  // Open the file
  int32 swf_id = SWopen(pOrbitFile->omiFileName, DFACC_READ);
  char *swathlist = NULL;
  if (swf_id == FAIL) {
    rc = ERROR_SetLast("OmiOpen",ERROR_TYPE_WARNING,ERROR_ID_HDFEOS,"OmiOpen",pOrbitFile->omiFileName,"SWopen");
    goto end_OmiOpen;
  }
  pOrbitFile->swf_id = swf_id;

  // Get a list of all swaths:
  int32 strbufsize;
  int nswath = SWinqswath(pOrbitFile->omiFileName, NULL, &strbufsize);
  if(nswath == FAIL) {
    rc = ERROR_SetLast("OmiOpen", ERROR_TYPE_WARNING, ERROR_ID_HDFEOS, "SWinqswath", pOrbitFile->omiFileName);
    goto end_OmiOpen;
  }
  swathlist = malloc((strbufsize+1)*sizeof(char));
  nswath = SWinqswath(pOrbitFile->omiFileName, swathlist, &strbufsize);

  // Look for requested swath in the list, and extract the complete name
  // e.g. "Earth UV-1 Swath" -> "Earth UV-1 Swath (60x159x4)"
  // this is needed to open the swath with SWattach

  char *swath_full_name = strstr(swathlist,swathName);
  if (swath_full_name == NULL) {
    rc = ERROR_SetLast("OmiOpen",ERROR_TYPE_WARNING,ERROR_ID_HDFEOS,"OmiOpen",pOrbitFile->omiFileName,"find swath");
    goto end_OmiOpen;
  }
  char *end_name = strpbrk(swath_full_name,",");

  if (end_name != NULL)
    *(end_name) = '\0';

  int32 sw_id = SWattach(swf_id, swath_full_name); // attach the swath
  if (sw_id == FAIL) {
    rc = ERROR_SetLast("OmiOpen", ERROR_TYPE_WARNING,ERROR_ID_HDFEOS,"OmiOpen",pOrbitFile->omiFileName,"SWattach");
    goto end_OmiOpen;
  }
  pOrbitFile->sw_id = sw_id;

  int32 dims[3];
  int32 rank;
  int32 numbertype;
  char dimlist[520];
  intn swrc = SWfieldinfo(sw_id, "RadianceMantissa",&rank,dims,&numbertype , dimlist);
  if(swrc == FAIL) {
    rc=ERROR_SetLast("OmiOpen", ERROR_TYPE_WARNING, ERROR_ID_FILE_EMPTY,pOrbitFile->omiFileName);
    goto end_OmiOpen;
  }

  pOrbitFile->omiNumberOfSwaths=(long)dims[0];
  pOrbitFile->omiNumberOfSpectraPerSwath=(long)dims[1];
  pOrbitFile->omiNumDataPoints=(long)dims[2];

  pOrbitFile->specNumber=pOrbitFile->omiNumberOfSwaths*pOrbitFile->omiNumberOfSpectraPerSwath;
  if (!pOrbitFile->specNumber)
    rc=ERROR_SetLast("OmiOpen",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pOrbitFile->omiFileName);


  // Allocate data

  if ((rc=OMI_AllocateSwath(&pOrbitFile->omiSwath,pOrbitFile->omiNumberOfSwaths,pOrbitFile->omiNumberOfSpectraPerSwath)))
    rc=ERROR_ID_ALLOC;
  // Retrieve information on records from Data fields and Geolocation fields
  else
    rc=OmiGetSwathData(pOrbitFile);

 end_OmiOpen:
  if(swathlist != NULL)
    free(swathlist);

  return rc;
}

RC OMI_LoadReference(ENGINE_CONTEXT *pEngineContext,DoasCh *refFile)
{
  OMI_REF * pRef=&OMI_ref[omiRefFilesN];
  int spectralType=pEngineContext->project.instrumental.omi.spectralType;
  RC rc=ERROR_ID_NO;

  int32 swf_id = 0;
  int32 sw_id = 0;

  swf_id = SWopen(refFile, DFACC_READ);
  if (swf_id == FAIL) {
    rc=ERROR_SetLast("OMI_LoadReference",ERROR_TYPE_WARNING,ERROR_ID_HDFEOS,"OMI_LoadReference",refFile,"SWopen");
    goto end_loadreference;
  }
  sw_id = SWattach(swf_id, OMI_SunSwaths[spectralType]);
  if (sw_id  == FAIL) {
    rc=ERROR_SetLast("OMI_LoadReference",ERROR_TYPE_WARNING,ERROR_ID_HDFEOS,"OMI_LoadReference",refFile,"SWattach");
    goto end_loadreference;
  }

  int32 n_xtrack = SWdiminfo(sw_id, "nXtrack");
  if (n_xtrack == FAIL) {
    rc = ERROR_SetLast("OMI_LoadReference", ERROR_TYPE_WARNING, ERROR_ID_HDFEOS, "nXtrack", refFile, "SWdiminfo");
    goto end_loadreference;
  }
  int32 n_wavel = SWdiminfo(sw_id, "nWavel");
  if (n_wavel == FAIL) {
    rc = ERROR_SetLast("OMI_LoadReference", ERROR_TYPE_WARNING, ERROR_ID_HDFEOS, "nWavel", refFile, "SWdiminfo");
    goto end_loadreference;
  }

  OMI_AllocateReference(omiRefFilesN,n_xtrack,n_wavel);

  strcpy(pRef->omiRefFileName,refFile);
  pRef->omiNumberOfSpectraPerSwath=n_xtrack;
  pRef->omiNumDataPoints=n_wavel;

  int indexSpectrum;
  for (indexSpectrum=0; indexSpectrum < pRef->omiNumberOfSpectraPerSwath; indexSpectrum++) {
    rc = omi_load_spectrum(OMI_SPEC_IRRAD, sw_id, 0, indexSpectrum, n_wavel,
			   pRef->omiRefLambda[indexSpectrum],
			   pRef->omiRefSpectrum[indexSpectrum],
			   pRef->omiRefSigma[indexSpectrum],
			   pRef->spectrum.pixelQualityFlags);
    if (rc)
      goto end_loadreference;
  }

  if (!rc)
    {
      omiRefFilesN++;
      NDET=pRef->omiNumDataPoints;
      ANALYSE_swathSize=pRef->omiNumberOfSpectraPerSwath;
    }

 end_loadreference:

  if(sw_id !=0)
    SWdetach(sw_id);
  if(swf_id !=0)
    SWclose(swf_id);

  return rc;
}

RC omi_load_spectrum(int spec_type, int32 sw_id, int32 measurement, int32 track, int32 n_wavel, double *lambda, double *spectrum, double *sigma, DoasUS *pixelQualityFlags) {
  RC rc = ERROR_ID_NO;

  int16 *mantissa = malloc(n_wavel*2);
  int16 *precisionmantissa = malloc(n_wavel*2);
  int8 *exponent = malloc(n_wavel);

  // names of the fields in omi hdf files.
  char *s_mantissa = IRRADIANCE_MANTISSA;
  char *s_precision_mantissa = IRRADIANCE_PRECISION_MANTISSA;
  char *s_exponent = IRRADIANCE_EXPONENT;

  if (spec_type == OMI_SPEC_RAD) {
    s_mantissa = RADIANCE_MANTISSA;
    s_precision_mantissa = RADIANCE_PRECISION_MANTISSA;
    s_exponent = RADIANCE_EXPONENT;
  }

  int32 start[] = {measurement, track, 0};

  // read reference column
  int16 refcol;
  intn swrc = SWreadfield(sw_id, REFERENCE_COLUMN, start, NULL, (int32[]) {1}, &refcol);
  // read wavelength coefficients
  int32 edge[] = {1,1,OMI_NUM_COEFFICIENTS}; // 1 measurement, 1 track, 5 coefficients
  float32 wavelength_coeff[5];
  swrc |= SWreadfield(sw_id, WAVELENGTH_COEFFICIENT, start, NULL, edge, &wavelength_coeff);

  if (swrc == FAIL) { // reading refcol or wavelengthcoeffs failed
    rc = ERROR_SetLast("omi_load_spectrum", ERROR_TYPE_WARNING, ERROR_ID_HDFEOS, "SWreadfield");
    goto end_load_spectrum;
  }
  // store wavelength in lambda
  omi_calculate_wavelengths(wavelength_coeff, refcol, n_wavel, lambda);

  // read (ir-)radiance mantissae, exponents & pixel quality
  edge[2] = n_wavel;

  swrc = SWreadfield(sw_id, s_mantissa, start, NULL, edge, mantissa);
  swrc |= SWreadfield(sw_id, s_precision_mantissa, start, NULL, edge, precisionmantissa);
  swrc |= SWreadfield(sw_id, s_exponent, start, NULL, edge, exponent);
  swrc |= SWreadfield(sw_id, PIXEL_QUALITY_FLAGS, start, NULL, edge, pixelQualityFlags);

  if(swrc) { // error reading either mantissa/precision_mantissa/exponent/qualityflags:
    rc = ERROR_SetLast("omi_load_spectrum", ERROR_TYPE_WARNING, ERROR_ID_HDFEOS, "SWreadfield");
    goto end_load_spectrum;
  }

  omi_make_double(mantissa, exponent, n_wavel, spectrum);
  omi_make_double(precisionmantissa, exponent, n_wavel, sigma);

  // check for suspicious numbers (mantissa = -32767)
  omi_interpolate_errors(mantissa,n_wavel,lambda,spectrum);
  omi_interpolate_errors(precisionmantissa,n_wavel,lambda,sigma);

 end_load_spectrum:
  free(mantissa);
  free(precisionmantissa);
  free(exponent);

  return rc;
}

void omi_calculate_wavelengths(float32 wavelength_coeff[], int16 refcol, int32 n_wavel, double* lambda) {
  int i;
  for (i=0; i<n_wavel; i++) {
    double x = (double) i-refcol;
    lambda[i] = 0.;
    int j;
    for (j=OMI_NUM_COEFFICIENTS-1; j>=0 ; j--) {
      lambda[i] = lambda[i]*x + (double)wavelength_coeff[j];
    }
  }
}

void omi_make_double(int16 mantissa[], int8 exponent[], int32 n_wavel, double* result) {
  int i;
  for (i=0; i<n_wavel; i++) {
    result[i] = (double)mantissa[i] * STD_Pow10((int)exponent[i]);
  }
}

void omi_interpolate_errors(int16 mantissa[], int32 n_wavel, double wavelengths[], double y[] ){

  int i;
  for (i=1; i<n_wavel -1; i++) {
    if(mantissa[i] == -32767) {
      double lambda = wavelengths[i];
      double lambda1 = wavelengths[i-1];
      double lambda2 = wavelengths[i+1];
      double y1 = y[i-1];
      double y2 = y[i+1];
      y[i] = (y2*(lambda-lambda1) + y1*(lambda2-lambda))/(lambda2-lambda1);
    }
  }
}

RC OMI_GetReference(DoasCh *refFile,INDEX indexColumn,double *lambda,double *ref,double *refSigma)
{
  // Declarations

  OMI_REF *pRef;
  INDEX indexRef;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  // Browse existing references

  for (indexRef=0;indexRef<omiRefFilesN;indexRef++)
    if (!strcasecmp(OMI_ref[indexRef].omiRefFileName,refFile))
      break;

  // Get

  if ((indexRef<omiRefFilesN) && (indexColumn>=0) && (indexColumn<OMI_ref[indexRef].omiNumberOfSpectraPerSwath))
    {
      pRef=&OMI_ref[indexRef];

      memcpy(lambda,pRef->omiRefLambda[indexColumn],sizeof(double)*pRef->omiNumDataPoints);
      memcpy(ref,pRef->omiRefSpectrum[indexColumn],sizeof(double)*pRef->omiNumDataPoints);
      memcpy(refSigma,pRef->omiRefSigma[indexColumn],sizeof(double)*pRef->omiNumDataPoints);
    }
  else
    rc=ERROR_SetLast("OMI_GetReference",ERROR_TYPE_WARNING,ERROR_ID_OMI_REF,"OMI_GetReference");

  // Return

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      OMI_Set
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve information on useful data sets from the HDF file and
//               load the irradiance spectrum measured at the specified channel
//
// INPUT/OUTPUT  pSpecInfo interface for file operations
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  if the file is not found;
//               ERROR_ID_FILE_EMPTY      if the file is empty;
//               ERROR_ID_HDF             if one or the access HDF functions failed;
//               ERROR_ID_ALLOC           if allocation of a buffer failed;
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC OMI_Set(ENGINE_CONTEXT *pEngineContext)
{
  // Declarations

  OMI_ORBIT_FILE *pOrbitFile;                                                 // pointer to the current orbit
  INDEX indexFile;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("OMI_Set",DEBUG_FCTTYPE_FILE);
#endif

  // Initializations

  pEngineContext->recordNumber=0;
  omiCurrentFileIndex=ITEM_NONE;
  omiSwathOld=ITEM_NONE;
  RC rc=ERROR_ID_NO;

  if (omiCurrentFileIndex==ITEM_NONE)
    {
      // Release old buffers

      OMI_ReleaseBuffers();

      omiOrbitFilesN=1;
      omiCurrentFileIndex=0;
      strcpy(omiOrbitFiles[0].omiFileName,pEngineContext->fileInfo.fileName);

      // Load files

      for (omiTotalRecordNumber=indexFile=0;indexFile<omiOrbitFilesN;indexFile++)
	{
	  pOrbitFile=&omiOrbitFiles[indexFile];

	  //pOrbitFile->omiPf=NULL;
	  pOrbitFile->specNumber=0;

	  // Open the file

	  if (!(rc=OmiOpen(pOrbitFile,OMI_EarthSwaths[pEngineContext->project.instrumental.omi.spectralType])))
	    {/*
	      if (pOrbitFile->omiPf!=NULL)
		{
		  coda_close(pOrbitFile->omiPf);
		  pOrbitFile->omiPf=NULL;
		}
	     */
	      if ((strlen(pEngineContext->fileInfo.fileName)==strlen(pOrbitFile->omiFileName)) &&
		  !strcasecmp(pEngineContext->fileInfo.fileName,pOrbitFile->omiFileName)) {
		omiCurrentFileIndex=indexFile;
	      }

	      pEngineContext->recordNumber=pOrbitFile->specNumber;

	      omiTotalRecordNumber+=pOrbitFile->omiNumberOfSwaths;
	      NDET=pOrbitFile->omiNumDataPoints;

	      if (rc!=ERROR_ID_NO) {
		pOrbitFile->rc=rc;
	      }

	      rc=ERROR_ID_NO;
	    }
	}
    }

  if (omiCurrentFileIndex==ITEM_NONE)
    rc=ERROR_SetLast("omi_Set",ERROR_TYPE_FATAL,ERROR_ID_ALLOC,"omiOrbitFiles");
  else if (!(pEngineContext->recordNumber=(pOrbitFile=&omiOrbitFiles[omiCurrentFileIndex])->specNumber))
    rc=ERROR_SetLast("omi_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pOrbitFile->omiFileName);
  else
    {
      /*
      if (!(rc=pOrbitFile->rc) && (pOrbitFile->omiPf==NULL) &&
	  !(rc=coda_open(pOrbitFile->omiFileName,&pOrbitFile->omiPf)) &&
	  !(rc=coda_cursor_set_product(&pOrbitFile->omiCursor,pOrbitFile->omiPf)))
	{
      */
	  pEngineContext->recordInfo.omi.omiNumberOfSwaths=pOrbitFile->omiNumberOfSwaths;
	  pEngineContext->recordInfo.omi.omiNumberOfRows=pOrbitFile->omiNumberOfSpectraPerSwath;
	  //	}
    }

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("OMI_Set",rc);
#endif

  // Return
  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      OMI_Read
// -----------------------------------------------------------------------------
// PURPOSE       OMI level 1 data read out
//
// INPUT         recordNo     index of the record to read
//
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_FILE_END        the end of the file is reached;
//               ERROR_ID_FILE_RECORD     the record doesn't satisfy user constraints
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC OMI_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,INDEX fileIndex)
{
  // Declarations

  OMI_ORBIT_FILE *pOrbitFile;                                                   // pointer to the current orbit
  OMI_SWATH *pSwath;
  OMI_SPECTRUM *pSpectrum;                                                      // spectrum in earth swath
  OMI_DATA_FIELDS *pData;                                                       // data on earth swath
  OMI_GEO *pGeo;                                                                // geolocations

  double *lambda,*spectrum,*sigma;
  RECORD_INFO *pRecord;
  INDEX indexSwath,indexSpectrum;
  INDEX i;                                                                    // index for loops and arrays
  RC rc;                                                                        // return code

  // Initializations

  pOrbitFile=&omiOrbitFiles[(fileIndex==ITEM_NONE)?omiCurrentFileIndex:fileIndex];
  pSwath=&pOrbitFile->omiSwath;
  pData=&pSwath->dataFields;
  pSpectrum=&pSwath->spectrum;
  pGeo=&pSwath->geolocationFields;

  spectrum=pEngineContext->buffers.spectrum;
  sigma=pEngineContext->buffers.sigmaSpec;
  lambda=pEngineContext->buffers.lambda;
  pRecord=&pEngineContext->recordInfo;
  rc=ERROR_ID_NO;

  // Goto the requested record

  if (!pOrbitFile->specNumber)
    rc=ERROR_ID_FILE_EMPTY;
  else if ((recordNo<=0) || (recordNo>pOrbitFile->specNumber))
    rc=ERROR_ID_FILE_END;
  else
    {
      for (i=0;i<NDET;i++)
	spectrum[i]=sigma[i]=(double)0.;

      indexSwath=floor((recordNo-1)/pOrbitFile->omiNumberOfSpectraPerSwath);      // index of the swath
      indexSpectrum=(recordNo-1)%pOrbitFile->omiNumberOfSpectraPerSwath;          // index of the spectrum in the swath

      if (!pEngineContext->project.instrumental.omi.omiTracks[indexSpectrum]) {
	rc=ERROR_ID_FILE_RECORD;
      } else if (!(rc=
		 omi_load_spectrum(OMI_SPEC_RAD,
				   pOrbitFile->sw_id,
				   indexSwath,
				   indexSpectrum,
				   pOrbitFile->omiNumDataPoints,
				   lambda,spectrum,sigma,pSpectrum->pixelQualityFlags)))
	{
	  if ((THRD_id==THREAD_TYPE_ANALYSIS) && omiRefFilesN)
	    {
	      if (omiSwathOld!=indexSwath)
		{
		  KURUCZ_indexLine=1;
		  omiSwathOld=indexSwath;
		}

	      memcpy(pEngineContext->buffers.irrad,OMI_ref[0].omiRefSpectrum[indexSpectrum],sizeof(double)*NDET);
	    }

	  pRecord->latitude=pGeo->latitude[recordNo-1];
	  pRecord->longitude=pGeo->longitude[recordNo-1];
	  pRecord->Zm=pGeo->solarZenithAngle[recordNo-1];
	  pRecord->Azimuth=pGeo->solarAzimuthAngle[recordNo-1];
	  pRecord->zenithViewAngle=pGeo->viewingZenithAngle[recordNo-1];
	  pRecord->azimuthViewAngle=pGeo->viewingAzimuthAngle[recordNo-1];
	  pRecord->useErrors=1;                                                     // Errors are available for OMI

	  // Complete information on the current spectrum

	  pRecord->omi.omiSwathIndex=indexSwath+1;                                  // index of the current swath
	  pRecord->omi.omiRowIndex=indexSpectrum+1;                                 // index of the current spectrum in the current swath
	  memcpy(pRecord->omi.omiPixelQF,pSpectrum->pixelQualityFlags,sizeof(unsigned short)*NDET);

	  OMI_FromTAI1993ToYMD((double)pGeo->time[indexSwath],&pRecord->present_day,&pRecord->present_time,&OMI_ms);

	  pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);
	}
    }

  // Return

  return rc;
}

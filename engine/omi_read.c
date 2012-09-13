
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  OMI interface
//  Name of module    :  OMIRead.C
//  Program Language  :  C/C++
//  Creation date     :  4 April 2007
//
//  Author            :  Caroline FAYT
//
//        Copyright  (C) Belgian Institute for Space Aeronomy (BIRA-IASB)
//                       Avenue Circulaire, 3
//                       1180     UCCLE
//                       BELGIUM
//
//  As the WinDOAS software is distributed freely within the DOAS community, it
//  would be nice if BIRA-IASB Institute and the authors were mentioned at least
//  in acknowledgements of papers presenting results obtained with this program.
//
//  The source code is also available on request for use, modification and free
//  distribution but authors are not responsible of unexpected behaviour of the
//  program if changes have been made on the original code.
//
//  ----------------------------------------------------------------------------
//  FUNCTIONS
//
//
//
//  LIBRARIES
//
//  This module uses read out routines written by Stefan Noel (Stefan.Noel@iup.physik.uni.bremen.de)
//  and Andreas Richter (richter@iup.physik.uni-bremen.de) from IFE/IUP Uni Bremen.  These routines
//  are based on based on BEAT library.
//
//  ----------------------------------------------------------------------------

// ========
// INCLUDES
// ========

#include "mediate.h"
#include "engine.h"
#include "coda.h"

#define MAX_OMI_FILES 500

// ========================
// DEFINITION OF STRUCTURES
// ========================

enum _omiSwathType { OMI_SWATH_UV1, OMI_SWATH_UV2, OMI_SWATH_VIS, OMI_SWATH_MAX };
enum _omiVdataType { OMI_VDATA_GEO, OMI_VDATA_DATA, OMI_VDATA_ATTR, OMI_VDATA_MAX };
enum _omiSpecType  { OMI_SPEC_IRRAD, OMI_SPEC_RAD };

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
  short  *mantissa;
  short  *precisionMantissa;
  int8_t *exponent;
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
  coda_ProductFile   *omiPf;                                                    // OMI product file pointer
  coda_Cursor         omiCursor;                                                // OMI file cursor
  coda_type          *omiRootType,                                              // it is important to keep types of main record classes
                     *omiSwathType,
                     *omiGeolocationType,
                     *omiDataType;
  long                omiNumberOfSwaths,omiNumberOfSpectraPerSwath,omiNumDataPoints;

  int                 version;
  INT                 rc;
 }
OMI_ORBIT_FILE;

// ================
// STATIC VARIABLES
// ================

DoasCh *OMI_EarthSwaths[OMI_SWATH_MAX]={"Earth_UV_1_Swath","Earth_UV_2_Swath","Earth_VIS_Swath"};
DoasCh *OMI_SunSwaths[OMI_SWATH_MAX]={"Sun_Volume_UV_1_Swath","Sun_Volume_UV_2_Swath","Sun_Volume_VIS_Swath"};

static OMI_ORBIT_FILE omiOrbitFiles[MAX_OMI_FILES];                             // list of files per day
static int omiOrbitFilesN=0,omiRefFilesN=0;                                     // the total number of files to browse in one shot
static INDEX omiCurrentFileIndex=ITEM_NONE;                                     // index of the current file in the list
static INT omiLoadReferenceFlag=0;
static INT omiTotalRecordNumber=0;
static OMI_REF OMI_ref[MAX_FENO];                                               // the number of reference spectra is limited to the maximum number of analysis windows in a project

int OMI_beatLoaded=0;
int OMI_ms=0;
int omiSwathOld=ITEM_NONE;

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

    if (pRef->spectrum.mantissa!=NULL)                                          // mantissa
     MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","pRef->mantissa",pRef->spectrum.mantissa);
    if (pRef->spectrum.precisionMantissa!=NULL)                                 // precision mantissa
     MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","pRef->spectrum.precisionMantissa",pRef->spectrum.precisionMantissa);
    if (pRef->spectrum.exponent!=NULL)                                          // exponent
     MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","pRef->spectrum.exponent",pRef->spectrum.exponent);

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

 	    ((pRef->spectrum.mantissa=(short *)MEMORY_AllocBuffer("OMI_AllocateReference","mantissa",nPoints,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL) ||
 	    ((pRef->spectrum.precisionMantissa=(short *)MEMORY_AllocBuffer("OMI_AllocateReference","precisionMantissa",nPoints,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL) ||
 	    ((pRef->spectrum.exponent=(int8_t *)MEMORY_AllocBuffer("OMI_AllocateReference","exponent",nPoints,sizeof(int8_t),0,MEMORY_TYPE_STRING))==NULL))

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

    if (pOrbitFile->omiSwath.spectrum.mantissa!=NULL)                                    // mantissa
     MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","pOrbitFile->omiSwath.spectrum.mantissa",pOrbitFile->omiSwath.spectrum.mantissa);
    if (pOrbitFile->omiSwath.spectrum.precisionMantissa!=NULL)                           // precision mantissa
     MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","pOrbitFile->omiSwath.spectrum.precisionMantissa",pOrbitFile->omiSwath.spectrum.precisionMantissa);
    if (pOrbitFile->omiSwath.spectrum.exponent!=NULL)                                    // exponent
     MEMORY_ReleaseBuffer("OMI_ReleaseBuffers","pOrbitFile->omiSwath.spectrum.exponent",pOrbitFile->omiSwath.spectrum.exponent);

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

    if (pOrbitFile->omiPf!=NULL)
     coda_close(pOrbitFile->omiPf);
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

 	OMI_SPECTRUM *pSpectrum;                                                      // spectrum in earth swath
 	OMI_DATA_FIELDS *pData;                                                       // data on earth swath
 	OMI_GEO *pGeo;                                                                // geolocations
 	int nRecords;                                                                 // total number of spectra
 	RC rc;                                                                        // Return code

 	// Initializations

 	pSpectrum=&pSwath->spectrum;
 	pData=&pSwath->dataFields;
 	pGeo=&pSwath->geolocationFields;
 	nRecords=nSwaths*nSpectra;
 	rc=ERROR_ID_NO;

 		// Buffers related to spectra and calibration

 	if (((pSpectrum->mantissa=(short *)MEMORY_AllocBuffer("OMI_AllocateSwath","mantissa",NDET,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL) ||
 	    ((pSpectrum->precisionMantissa=(short *)MEMORY_AllocBuffer("OMI_AllocateSwath","precisionMantissa",NDET,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL) ||
 	    ((pSpectrum->exponent=(int8_t *)MEMORY_AllocBuffer("OMI_AllocateSwath","exponent",NDET,sizeof(int8_t),0,MEMORY_TYPE_STRING))==NULL) ||

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
 	// Declarations

  OMI_DATA_FIELDS *pData;
  DoasCh omiPathMeasurementQualityFlags[MAX_STR_LEN+1],
         omiPathExposureTime[MAX_STR_LEN+1],
         omiPathDetectorTemperature[MAX_STR_LEN+1],
         omiPathWavelengthReferenceColumn[MAX_STR_LEN+1];
  RC    rc;                                                                     // return code

  // Initializations

  pData=&pOrbitFile->omiSwath.dataFields;
  rc=ERROR_ID_NO;

  // Paths initializations

  sprintf(omiPathMeasurementQualityFlags,"/%s/Data_Fields/MeasurementQualityFlags/MeasurementQualityFlags",pOrbitFile->omiSwathName);
  sprintf(omiPathExposureTime,"/%s/Data_Fields/ExposureTime/ExposureTime",pOrbitFile->omiSwathName);
  sprintf(omiPathDetectorTemperature,"/%s/Data_Fields/DetectorTemperature/DetectorTemperature",pOrbitFile->omiSwathName);
  sprintf(omiPathWavelengthReferenceColumn,"/%s/Data_Fields/WavelengthReferenceColumn/WavelengthReferenceColumn",pOrbitFile->omiSwathName);

  if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathMeasurementQualityFlags)!=0)
   rc=ERROR_SetLast("OmiGetSwathData",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathData",pOrbitFile->omiFileName,"Can not access Data_Fields\\MeasurementQualityFlags");
  else if (coda_cursor_read_uint16_array(&pOrbitFile->omiCursor,pData->measurementQualityFlags,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathData",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathData",pOrbitFile->omiFileName,"Can not read Data_Fields\\MeasurementQualityFlags");

  // Exposure time

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathExposureTime)!=0)
   rc=ERROR_SetLast("OmiGetSwathData",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathData",pOrbitFile->omiFileName,"Can not access Data_Fields\\ExposureTime");
  else if (coda_cursor_read_float_array(&pOrbitFile->omiCursor,pData->exposureTime,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathData",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathData",pOrbitFile->omiFileName,"Can not read Data_Fields\\ExposureTime");

  // Detector temperature

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathDetectorTemperature)!=0)
   rc=ERROR_SetLast("OmiGetSwathData",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathData",pOrbitFile->omiFileName,"Can not access Data_Fields\\DetectorTemperature");
  else if (coda_cursor_read_float_array(&pOrbitFile->omiCursor,pData->detectorTemperature,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathData",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathData",pOrbitFile->omiFileName,"Can not read Data_Fields\\DetectorTemperature");

  // Wavelength reference

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathWavelengthReferenceColumn)!=0)
   rc=ERROR_SetLast("OmiGetSwathData",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathData",pOrbitFile->omiFileName,"Can not access Data_Fields\\WavelengthReferenceColumn");
  else if (coda_cursor_read_int16_array(&pOrbitFile->omiCursor,pData->wavelengthReferenceColumn,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathData",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathData",pOrbitFile->omiFileName,"Can not read Data_Fields\\WavelengthReferenceColumn");

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      OmiGetSwathGeolocation
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve geolocation of swath
//
// INPUT         pOrbitFile   pointer to the current orbit file
// OUTPUT        rc           return code
//
// RETURN        ERROR_ID_NO if the function succeeds
//               ERROR_ID_BEAT otherwise
// -----------------------------------------------------------------------------

RC OmiGetSwathGeolocation(OMI_ORBIT_FILE *pOrbitFile) // ,INDEX indexTrack,INDEX indexSpe)
 {
 	// Declarations

  OMI_GEO *pGeo;
  DoasCh omiPathTime[MAX_STR_LEN+1],
         omiPathSecondsInDay[MAX_STR_LEN+1],
         omiPathSpacecraftLatitude[MAX_STR_LEN+1],
         omiPathSpacecraftLongitude[MAX_STR_LEN+1],
         omiPathSpacecraftAltitude[MAX_STR_LEN+1],
         omiPathLatitude[MAX_STR_LEN+1],
         omiPathLongitude[MAX_STR_LEN+1],
         omiPathSolarZenithAngle[MAX_STR_LEN+1],
         omiPathSolarAzimuthAngle[MAX_STR_LEN+1],
         omiPathViewingZenithAngle[MAX_STR_LEN+1],
         omiPathViewingAzimuthAngle[MAX_STR_LEN+1],
         omiPathTerrainHeight[MAX_STR_LEN+1],
         omiPathGroundPixelQualityFlags[MAX_STR_LEN+1],
         omiPathXtrackQualityFlags[MAX_STR_LEN+1];
  RC    rc;                                                                     // return code
  int   i,nRecords;

  // Initializations

  pGeo=&pOrbitFile->omiSwath.geolocationFields;
  rc=ERROR_ID_NO;

  // Paths initializations

  sprintf(omiPathTime,"/%s/Geolocation_Fields/Time/Time",pOrbitFile->omiSwathName);
  sprintf(omiPathSecondsInDay,"/%s/Geolocation_Fields/SecondsInDay/SecondsInDay",pOrbitFile->omiSwathName);
  sprintf(omiPathSpacecraftLatitude,"/%s/Geolocation_Fields/SpacecraftLatitude/SpacecraftLatitude",pOrbitFile->omiSwathName);
  sprintf(omiPathSpacecraftLongitude,"/%s/Geolocation_Fields/SpacecraftLongitude/SpacecraftLongitude",pOrbitFile->omiSwathName);
  sprintf(omiPathSpacecraftAltitude,"/%s/Geolocation_Fields/SpacecraftAltitude/SpacecraftAltitude",pOrbitFile->omiSwathName);
  sprintf(omiPathLatitude,"/%s/Geolocation_Fields/Latitude",pOrbitFile->omiSwathName);
  sprintf(omiPathLongitude,"/%s/Geolocation_Fields/Longitude",pOrbitFile->omiSwathName);
  sprintf(omiPathSolarZenithAngle,"/%s/Geolocation_Fields/SolarZenithAngle",pOrbitFile->omiSwathName);
  sprintf(omiPathSolarAzimuthAngle,"/%s/Geolocation_Fields/SolarAzimuthAngle",pOrbitFile->omiSwathName);
  sprintf(omiPathViewingZenithAngle,"/%s/Geolocation_Fields/ViewingZenithAngle",pOrbitFile->omiSwathName);
  sprintf(omiPathViewingAzimuthAngle,"/%s/Geolocation_Fields/ViewingAzimuthAngle",pOrbitFile->omiSwathName);
  sprintf(omiPathTerrainHeight,"/%s/Geolocation_Fields/TerrainHeight",pOrbitFile->omiSwathName);
  sprintf(omiPathGroundPixelQualityFlags,"/%s/Geolocation_Fields/GroundPixelQualityFlags",pOrbitFile->omiSwathName);
  sprintf(omiPathXtrackQualityFlags,"/%s/Geolocation_Fields/xtrackQualityFlags",pOrbitFile->omiSwathName);

  // Time

  if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathTime)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not access Geolocation_Fields\\Time");
  else if (coda_cursor_read_double_array(&pOrbitFile->omiCursor,pGeo->time,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\Time");

  // Seconds in day

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathSecondsInDay)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not access Geolocation_Fields\\SecondsInDay");
  else if (coda_cursor_read_float_array(&pOrbitFile->omiCursor,pGeo->secondsInDay,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\SecondsInDay");

  // Spacecraft latitude

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathSpacecraftLatitude)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not access Geolocation_Fields\\SpacecraftLatitude");
  else if (coda_cursor_read_float_array(&pOrbitFile->omiCursor,pGeo->spacecraftLatitude,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\SpacecraftLatitude");

  // Spacecraft longitude

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathSpacecraftLongitude)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not access Geolocation_Fields\\SpacecraftLongitude");
  else if (coda_cursor_read_float_array(&pOrbitFile->omiCursor,pGeo->spacecraftLongitude,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\SpacecraftLongitude");

  // Spacecraft altitude

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathSpacecraftAltitude)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not access Geolocation_Fields\\SpacecraftAltitude");
  else if (coda_cursor_read_float_array(&pOrbitFile->omiCursor,pGeo->spacecraftAltitude,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\SpacecraftAltitude");

  // Latitude

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathLatitude)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not access Geolocation_Fields\\Latitude");
  else if (coda_cursor_read_float_array(&pOrbitFile->omiCursor,pGeo->latitude,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\Latitude");

  // Longitude

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathLongitude)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not access Geolocation_Fields\\Longitude");
  else if (coda_cursor_read_float_array(&pOrbitFile->omiCursor,pGeo->longitude,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\Longitude");

  // Solar zenith angle

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathSolarZenithAngle)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not access Geolocation_Fields\\SolarZenithAngle");
  else if (coda_cursor_read_float_array(&pOrbitFile->omiCursor,pGeo->solarZenithAngle,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\SolarZenithAngle");

  // Solar azimuth angle

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathSolarAzimuthAngle)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not access Geolocation_Fields\\SolarAzimuthAngle");
  else if (coda_cursor_read_float_array(&pOrbitFile->omiCursor,pGeo->solarAzimuthAngle,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\SolarAzimuthAngle");

  // Viewing zenith angle

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathViewingZenithAngle)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not access Geolocation_Fields\\ViewingZenithAngle");
  else if (coda_cursor_read_float_array(&pOrbitFile->omiCursor,pGeo->viewingZenithAngle,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\ViewingZenithAngle");

  // Viewing azimuth angle

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathViewingAzimuthAngle)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not access Geolocation_Fields\\ViewingAzimuthAngle");
  else if (coda_cursor_read_float_array(&pOrbitFile->omiCursor,pGeo->viewingAzimuthAngle,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\ViewingAzimuthAngle");

  // Terrain Height

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathTerrainHeight)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not access Geolocation_Fields\\TerrainHeight");
  else if (coda_cursor_read_int16_array(&pOrbitFile->omiCursor,pGeo->terrainHeight,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\TerrainHeight");

  // Ground Pixel Quality flags

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathGroundPixelQualityFlags)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not access Geolocation_Fields\\GroundPixelQualityFlags");
  else if (coda_cursor_read_uint16_array(&pOrbitFile->omiCursor,pGeo->groundPixelQualityFlags,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\GroundPixelQualityFlags");

  // xtrack Quality flags

  else if (coda_cursor_goto(&pOrbitFile->omiCursor,omiPathXtrackQualityFlags)!=0)                         // xtrack quality flags not present in old versions of files
   for (i=0,nRecords=pOrbitFile->omiNumberOfSwaths*pOrbitFile->omiNumberOfSpectraPerSwath;i<nRecords;i++)
    pGeo->xtrackQualityFlags[i]=(uint8_t)0;
  else if (coda_cursor_read_uint8_array(&pOrbitFile->omiCursor,pGeo->xtrackQualityFlags,coda_array_ordering_c)!=0)
   rc=ERROR_SetLast("OmiGetSwathGeolocation",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSwathGeolocation",pOrbitFile->omiFileName,"Can not read Geolocation_Fields\\xtrackQualityFlags");

  // Return

  return rc;
 }

RC OmiGetSpectrum(coda_Cursor *pCursor,DoasCh *omiFileName,DoasCh *omiSwathName,INDEX indexSwath,INDEX indexSpectrum,OMI_SPECTRUM *pSpectrum,int ndata,int16_t refCol,double *lambda,double *spectrum,double *sigma)
 {
 	// Declarations

  DoasCh omiPathMantissa[MAX_STR_LEN+1],
         omiPathPrecisionMantissa[MAX_STR_LEN+1],
         omiPathWavelengthCoefficient[MAX_STR_LEN+1],
         omiPathExponent[MAX_STR_LEN+1],
         omiSpectraType[MAX_STR_LEN+1];

  double *tempLambda,*tempMantissa;
  double *tempPrecision;
  double a,b;
  INDEX  *tempIndexLow,*tempIndexHigh,ilow,ihigh;
  int nMantissa;
  double pixDelta;

 	long subset[3];
 	INDEX i,j,iold;
 	RC rc;

 	// Initializations

  subset[0]=(long)indexSwath;
  subset[1]=(long)indexSpectrum;
  subset[2]=(long)0;
  tempIndexLow=tempIndexHigh=NULL;
  tempLambda=NULL;
  tempMantissa=NULL;
  tempPrecision=NULL;
  nMantissa=0;

  strcpy(omiSpectraType,(strstr(omiSwathName,"Earth")!=NULL)?"Radiance":"Irradiance");

 	rc=ERROR_ID_NO;

  // Paths initializations

  sprintf(omiPathMantissa,"/%s/Data_Fields/%sMantissa",omiSwathName,omiSpectraType);
  sprintf(omiPathPrecisionMantissa,"/%s/Data_Fields/%sPrecisionMantissa",omiSwathName,omiSpectraType);
  sprintf(omiPathWavelengthCoefficient,"/%s/Data_Fields/WavelengthCoefficient",omiSwathName);
  sprintf(omiPathExponent,"/%s/Data_Fields/%sExponent",omiSwathName,omiSpectraType);

  if (((tempIndexLow=(INDEX *)MEMORY_AllocBuffer("OmiGetSpectrum","tempIndexLow",NDET,sizeof(INDEX),0,MEMORY_TYPE_INT))==NULL) ||
      ((tempIndexHigh=(INDEX *)MEMORY_AllocBuffer("OmiGetSpectrum","tempIndexHigh",NDET,sizeof(INDEX),0,MEMORY_TYPE_INT))==NULL) ||
      ((tempLambda=(double *)MEMORY_AllocDVector("OmiGetSpectrum","tempLambda",0,NDET-1))==NULL) ||
      ((tempMantissa=(double *)MEMORY_AllocDVector("OmiGetSpectrum","tempMantissa",0,NDET-1))==NULL) ||
      ((tempPrecision=(double *)MEMORY_AllocDVector("OmiGetSpectrum","tempPrecision",0,NDET-1))==NULL))

   rc=ERROR_ID_ALLOC;

  // Read wavelength coefficient

  else if (coda_cursor_goto(pCursor,omiPathWavelengthCoefficient)!=0)
   rc=ERROR_SetLast("OmiGetSpectrum",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSpectrum",omiFileName,"Can not access Data_Fields\\WavelengthCoefficient");
  else if (coda_cursor_goto_array_element(pCursor,3,subset)!=0)
   rc=ERROR_SetLast("OmiGetSpectrum",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSpectrum",omiFileName,"Can not goto first element of Data_Fields\\WavelengthCoefficient");
  else
   {
   	for (i=0;i<5;i++)
   	 {
   	 	coda_cursor_read_float(pCursor,&pSpectrum->wavelengthCoefficient[i]);
   	 	coda_cursor_goto_next_array_element(pCursor);
   	 }
   }

  // Read exponent

  if (!rc)
   {
   	if (coda_cursor_goto(pCursor,omiPathExponent)!=0)
     rc=ERROR_SetLast("OmiGetSpectrum",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSpectrum",omiFileName,"Can not access Data_Fields\\Exponent");
    else if (coda_cursor_goto_array_element(pCursor,3,subset)!=0)
     rc=ERROR_SetLast("OmiGetSpectrum",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSpectrum",omiFileName,"Can not goto first element of Data_Fields\\Exponent");
    else
     {
     	for (i=0;i<ndata;i++)
     	 {
     	 	coda_cursor_read_int8(pCursor,(int8_t *)&pSpectrum->exponent[i]);
     	 	coda_cursor_goto_next_array_element(pCursor);
     	 }
     }
   }

  // Read precision mantissa

  if (!rc)
   {
   	if (coda_cursor_goto(pCursor,omiPathPrecisionMantissa)!=0)
     rc=ERROR_SetLast("OmiGetSpectrum",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSpectrum",omiFileName,"Can not access Data_Fields\\PrecisionMantissa");
    else if (coda_cursor_goto_array_element(pCursor,3,subset)!=0)
     rc=ERROR_SetLast("OmiGetSpectrum",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSpectrum",omiFileName,"Can not goto first element of Data_Fields\\PrecisionMantissa");
    else
     {
     	for (i=0;i<ndata;i++)
     	 {
     	 	coda_cursor_read_int16(pCursor,&pSpectrum->precisionMantissa[i]);
     	 	coda_cursor_goto_next_array_element(pCursor);
     	 }
     }
   }

  // Read mantissa

  if (!rc)
   {
    if (coda_cursor_goto(pCursor,omiPathMantissa)!=0)
     rc=ERROR_SetLast("OmiGetSpectrum",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSpectrum",omiFileName,"Can not access Data_Fields\\Mantissa");
    else if (coda_cursor_goto_array_element(pCursor,3,subset)!=0)
     rc=ERROR_SetLast("OmiGetSpectrum",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetSpectrum",omiFileName,"Can not goto element of Data_Fields\\Mantissa");
    else
     {
     	for (i=0,ilow=ITEM_NONE,iold=-1;i<ndata;i++)
     	 {
     	 	pixDelta=(double)(i-refCol);

     	 	coda_cursor_read_int16(pCursor,&pSpectrum->mantissa[i]);
     	 	coda_cursor_goto_next_array_element(pCursor);

     	 	lambda[i]=(double)pSpectrum->wavelengthCoefficient[0]+
         	               (pSpectrum->wavelengthCoefficient[1]+
         	               (pSpectrum->wavelengthCoefficient[2]+
         	               (pSpectrum->wavelengthCoefficient[3]+
         	                pSpectrum->wavelengthCoefficient[4]*pixDelta)*pixDelta)*pixDelta)*pixDelta;

     	 	if (pSpectrum->mantissa[i]!=-32767)
     	 	 {
       	 	for (j=iold+1;j<i;j++)
       	 	 {
       	 	  if (tempIndexLow[j]==ITEM_NONE)
       	 	   tempIndexLow[j]=nMantissa;
       	 	  tempIndexHigh[j]=nMantissa;
       	 	 }

       	 	iold=i;
       	 	ilow=nMantissa;

          tempIndexLow[i]=tempIndexHigh[i]=nMantissa;
          tempLambda[nMantissa]=lambda[i];
     	 	  tempMantissa[nMantissa]=(double)pSpectrum->mantissa[i]*STD_Pow10((int)pSpectrum->exponent[i]);
     	 	  tempPrecision[nMantissa]=(double)pSpectrum->precisionMantissa[i]*STD_Pow10((int)pSpectrum->exponent[i]);

     	 	  nMantissa++;
     	 	 }
     	 	else
    	 	  tempIndexLow[i]=ilow;
     	 }

     	for (j=iold+1;j<ndata;j++)
     	 tempIndexHigh[j]=nMantissa;
     }
   }

  if (nMantissa)
   for (i=0;i<ndata;i++)
    {
    	if (tempIndexLow[i]==tempIndexHigh[i])
    	 {
    	 	spectrum[i]=tempMantissa[tempIndexLow[i]];
    	 	sigma[i]=tempPrecision[tempIndexLow[i]];
    	 }
    	else
    	 {
    	 	// linear interpolation

       a = (tempLambda[tempIndexHigh[i]]-lambda[i])/(tempLambda[tempIndexHigh[i]]-tempLambda[tempIndexLow[i]]);
       b = (lambda[i]-tempLambda[tempIndexLow[i]])/(tempLambda[tempIndexHigh[i]]-tempLambda[tempIndexLow[i]]);

       spectrum[i]=a*tempMantissa[tempIndexLow[i]]+b*tempMantissa[tempIndexHigh[i]];
       sigma[i]=a*tempPrecision[tempIndexLow[i]]+b*tempPrecision[tempIndexHigh[i]];
    	 }
    }
  else
   {
    for (i=0;i<ndata;i++)
     spectrum[i]=sigma[i]=(double)0.;
    rc=ERROR_ID_FILE_RECORD;
   }


// {
// 	FILE *fp;
// 	fp=fopen("toto.dat","a+t");
// 	for (i=0;i<ndata;i++)
// 	 fprintf(fp,"%-3d %-3d %d %d %g %g %g %g %g\n",indexSpectrum,i,tempIndexLow[i],tempIndexHigh[i],lambda[i],spectrum[i],(double)pSpectrum->mantissa[i]*STD_Pow10((int)pSpectrum->exponent[i]),tempMantissa[tempIndexLow[i]],tempMantissa[tempIndexHigh[i]]);
// 	fclose(fp);
// }

  // Release allocated vectors

  if (tempIndexLow!=NULL)
   MEMORY_ReleaseBuffer("OmiGetSpectrum","tempIndexLow",tempIndexLow);
  if (tempIndexHigh!=NULL)
   MEMORY_ReleaseBuffer("OmiGetSpectrum","tempIndexHigh",tempIndexHigh);
  if (tempLambda!=NULL)
   MEMORY_ReleaseDVector("OmiGetSpectrum","tempLambda",tempLambda,0);
  if (tempMantissa!=NULL)
   MEMORY_ReleaseDVector("OmiGetSpectrum","tempMantissa",tempMantissa,0);
  if (tempPrecision!=NULL)
   MEMORY_ReleaseDVector("OmiGetSpectrum","tempPrecision",tempPrecision,0);

 	// Return

 	return rc;
 }

RC OmiGetRecordsNumber(coda_Cursor *pCursor,DoasCh *omiFileName,DoasCh *omiSwathName,long *dims)
 {
 	// Declarations

  DoasCh omiPathMantissa[MAX_STR_LEN+1],omiSpectraType[MAX_STR_LEN+1];
  coda_type *typeRecord;
  int numDims;
 	RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  strcpy(omiSpectraType,(strstr(omiSwathName,"Earth")!=NULL)?"Radiance":"Irradiance");

  sprintf(omiPathMantissa,"/%s/Data_Fields/%sMantissa",omiSwathName,omiSpectraType);

  if (coda_cursor_goto(pCursor,omiPathMantissa))
   rc=ERROR_SetLast("OmiGetRecordsNumber",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetRecordsNumber",omiFileName,"Can not access Data_Fields\\Mantissa");
  else if (coda_cursor_get_type(pCursor,&typeRecord)!=0)
   rc=ERROR_SetLast("OmiGetRecordsNumber",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetRecordsNumber",omiFileName,"Can not get type of Data_Fields\\Mantissa");
  else if (coda_type_get_array_dim(typeRecord,&numDims,dims)!=0)
   rc=ERROR_SetLast("OmiGetRecordsNumber",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiGetRecordsNumber",omiFileName,"Can not get dims of Data_Fields\\Mantissa");

  // Return

  return rc;
 }

RC OmiOpen(OMI_ORBIT_FILE *pOrbitFile,char *swathName)
 {
  // Declarations

  long numFields;
  long dims[CODA_MAX_NUM_DIMS];
  const char *fieldName;
  int i;
  RC    rc;                                                                     // return code

  // Initialization

  memset(dims,0,sizeof(long)*CODA_MAX_NUM_DIMS);

  // Open the file

  if ((rc=coda_open(pOrbitFile->omiFileName,&pOrbitFile->omiPf))!=0)
   {
    /* maybe not enough memory space to map the file in memory =>
     * temporarily disable memory mapping of files and try again
     */
    coda_set_option_use_mmap(0);
    rc=coda_open(pOrbitFile->omiFileName,&pOrbitFile->omiPf);        // &*productFile
    coda_set_option_use_mmap(1);
   }

  if (rc!=0)
   rc=ERROR_SetLast("OmiOpen",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiOpen",pOrbitFile->omiFileName,"coda_open");
  else if (coda_cursor_set_product(&pOrbitFile->omiCursor,pOrbitFile->omiPf)!=0)
   rc=ERROR_SetLast("OmiOpen",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiOpen",pOrbitFile->omiFileName,"coda_cursor_set_product");
  else if (coda_get_product_root_type(pOrbitFile->omiPf,&pOrbitFile->omiRootType)!=0)
   rc=ERROR_SetLast("OmiOpen",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiOpen",pOrbitFile->omiFileName,"coda_get_product_root_type");
  else if (coda_type_get_num_record_fields(pOrbitFile->omiRootType,&numFields)!=0)
   rc=ERROR_SetLast("OmiOpen",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiOpen",pOrbitFile->omiFileName,"coda_type_get_num_record_fields");
  else
   {
   	// Check that requested swath is present in the file

   	for (i=0;(i<numFields) && !rc;i++)
   	 if (coda_type_get_record_field_name(pOrbitFile->omiRootType,i,&fieldName)!=0)
   	  rc=ERROR_SetLast("OmiOpen",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiOpen",pOrbitFile->omiFileName,"coda_type_get_num_record_fields");
   	 else if (!strncmp(fieldName,swathName,strlen(swathName)))
   	 	break;

   	if (i<numFields)
   	 {
   	 	// Determine the type of fields for futher use of CODA functions

   	  strcpy(pOrbitFile->omiSwathName,fieldName);
      if (coda_type_get_record_field_type(pOrbitFile->omiRootType,i,&pOrbitFile->omiSwathType)!= 0)
       rc=ERROR_SetLast("OmiOpen",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiOpen",pOrbitFile->omiFileName,"coda_type_get_record_field_type (Swath)");
      else if (coda_type_get_record_field_type(pOrbitFile->omiSwathType,0,&pOrbitFile->omiGeolocationType)!= 0)
       rc=ERROR_SetLast("OmiOpen",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiOpen",pOrbitFile->omiFileName,"coda_type_get_record_field_type (Geolocation_Fields)");
      else if (coda_type_get_record_field_type(pOrbitFile->omiSwathType,1,&pOrbitFile->omiDataType)!= 0)
       rc=ERROR_SetLast("OmiOpen",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OmiOpen",pOrbitFile->omiFileName,"coda_type_get_record_field_type (Data_Fields)");

      // Get the number of swaths

      else if ((rc=OmiGetRecordsNumber(&pOrbitFile->omiCursor,pOrbitFile->omiFileName,pOrbitFile->omiSwathName,dims))!=0)
       rc=ERROR_ID_ALLOC;
      else
       {
       	pOrbitFile->omiNumberOfSwaths=dims[0];
       	pOrbitFile->omiNumberOfSpectraPerSwath=dims[1];
       	pOrbitFile->omiNumDataPoints=dims[2];

       	pOrbitFile->specNumber=pOrbitFile->omiNumberOfSwaths*pOrbitFile->omiNumberOfSpectraPerSwath;
       }

      if (!pOrbitFile->specNumber)
       rc=ERROR_SetLast("OmiOpen",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pOrbitFile->omiFileName);

      // Allocate data

      else if ((rc=OMI_AllocateSwath(&pOrbitFile->omiSwath,pOrbitFile->omiNumberOfSwaths,pOrbitFile->omiNumberOfSpectraPerSwath)))
       rc=ERROR_ID_ALLOC;

      // Retrieve information on records from Data fields and Geolocation fields

      else if (!(rc=OmiGetSwathData(pOrbitFile)))
       rc=OmiGetSwathGeolocation(pOrbitFile);
   	 }
   	else
   	 rc=ERROR_SetLast("OmiOpen",ERROR_TYPE_WARNING,ERROR_ID_OMI_SWATH,swathName,pOrbitFile->omiFileName);
   }

  // Return

  return rc;
 }

RC OMI_LoadReference(ENGINE_CONTEXT *pEngineContext,DoasCh *refFile)
 {
 	// Declarations

  coda_ProductFile   *omiPf;                                                    // OMI product file pointer
  coda_Cursor         omiCursor;                                                // OMI file cursor
  DoasCh omiPathWavelengthReferenceColumn[MAX_STR_LEN+1];
// DoasCh refFile[MAX_STR_LEN+1];
//  struct dirent *fileInfo;
  long dims[CODA_MAX_NUM_DIMS];
//  DIR *hDir;
//  int fileFound,
  int indexSpectrum,spectralType;
  int16_t refCol;
 	RC rc;
 	OMI_REF *pRef;

 	// Initializations

  pRef=&OMI_ref[omiRefFilesN];
  spectralType=pEngineContext->project.instrumental.omi.spectralType;
 	rc=ERROR_ID_NO;
  memset(dims,0,sizeof(long)*CODA_MAX_NUM_DIMS);
 	// sprintf(newRefPath,"%s/%d/%03d",refPath,year,cday);
 	sprintf(omiPathWavelengthReferenceColumn,"/%s/Data_Fields/WavelengthReferenceColumn/WavelengthReferenceColumn",OMI_SunSwaths[spectralType]);

//  // Search for the reference file
//
//  for (hDir=opendir(newRefPath),fileFound=0;(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL) && !fileFound;)
//   {
//    sprintf(refFile,"%s/%s",newRefPath,fileInfo->d_name);
//    if ((STD_IsDir(refFile)==0) &&                                              // it is not a directory
//       ((ptr=strrchr(refFile,'.'))!=NULL) &&                                    // search for file extension
//         strcasecmp(ptr,".he4"))                                                // extension for HDF 4
//     fileFound=1;
//   }
//
//  if (hDir!=NULL)
//   closedir(hDir);
//
//  if (!fileFound)
//   rc=ERROR_SetLast("OMI_ReadRef",ERROR_TYPE_WARNING,ERROR_ID_OMI_REF,"OMI_ReadRef",newRefPath);
//
//  // Open the file
//
//  else



  if (!OMI_beatLoaded)
   {
    coda_init();
    coda_set_option_bypass_special_types(0);                                    // do not bypass special types
    coda_set_option_perform_boundary_checks(0);
    coda_set_option_perform_conversions(1);                                     // conversions authorized

    OMI_beatLoaded=1;
   }


  if ((rc=coda_open(refFile,&omiPf))!=0)
   {
    /* maybe not enough memory space to map the file in memory =>
     * temporarily disable memory mapping of files and try again
     */

    coda_set_option_use_mmap(0);
    rc=coda_open(refFile,&omiPf);        // &*productFile
    coda_set_option_use_mmap(1);
   }

  if (rc!=0)
   rc=ERROR_SetLast("OMI_LoadReference",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OMI_LoadReference",refFile,"coda_open");
  else if (coda_cursor_set_product(&omiCursor,omiPf)!=0)
   rc=ERROR_SetLast("OMI_LoadReference",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OMI_LoadReference",refFile,"coda_cursor_set_product");
  else if (((rc=OmiGetRecordsNumber(&omiCursor,refFile,OMI_SunSwaths[spectralType],dims))!=0) ||
           ((rc=OMI_AllocateReference(omiRefFilesN,dims[1],dims[2]))!=0))
   rc=ERROR_ID_ALLOC;
  else if (coda_cursor_goto(&omiCursor,omiPathWavelengthReferenceColumn)!=0)
   rc=ERROR_SetLast("OMI_LoadReference",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OMI_LoadReference",refFile,"Can not access Data_Fields\\WavelengthReferenceColumn");
  else if (coda_cursor_goto_first_array_element(&omiCursor))
   rc=ERROR_SetLast("OMI_LoadReference",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OMI_LoadReference",refFile,"Can not access first element of Data_Fields\\WavelengthReferenceColumn");
  else if (coda_cursor_read_int16(&omiCursor,&refCol)!=0)
   rc=ERROR_SetLast("OMI_LoadReference",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"OMI_LoadReference",refFile,"Can not read Data_Fields\\WavelengthReferenceColumn");
  else
   {
   	strcpy(pRef->omiRefFileName,refFile);

    pRef->omiNumberOfSpectraPerSwath=dims[1];
    pRef->omiNumDataPoints=dims[2];

    NDET=pRef->omiNumDataPoints;
//     pRef->year=year;
//     pRef->cday=cday;

   	// Load spectra

   	for (indexSpectrum=0;(indexSpectrum<pRef->omiNumberOfSpectraPerSwath) && !rc;indexSpectrum++)
     if (!(rc=OmiGetSpectrum(&omiCursor,refFile,OMI_SunSwaths[spectralType],0,indexSpectrum,&pRef->spectrum,pRef->omiNumDataPoints,refCol,pRef->omiRefLambda[indexSpectrum],pRef->omiRefSpectrum[indexSpectrum],pRef->omiRefSigma[indexSpectrum])))
    	 memcpy(pRef->omiRefLambdaK[indexSpectrum],pRef->omiRefLambda[indexSpectrum],sizeof(double)*pRef->omiNumDataPoints);
   }

  if (!rc)
   {
    omiRefFilesN++;
    NDET=pRef->omiNumDataPoints;
    ANALYSE_swathSize=pRef->omiNumberOfSpectraPerSwath;
   }

// {
// 	FILE *fp;
// 	int i,j;
// 	fp=fopen("OMI_Ref.dat","w+t");
// 	for (i=0;i<NDET;i++)
// 	 {
// 	  for (j=0;j<pRef->omiNumberOfSpectraPerSwath;j++)
// 	   fprintf(fp,"%g ",pRef->omiRefSpectrum[j][i]);
// 	  fprintf(fp,"\n");
// 	 }
// 	fclose(fp);
// }
//
// {
// 	FILE *fp;
// 	int i,j;
// 	fp=fopen("OMI_Lambda.dat","w+t");
// 	for (i=0;i<NDET;i++)
// 	 {
// 	  for (j=0;j<pRef->omiNumberOfSpectraPerSwath;j++)
// 	   fprintf(fp,"%g ",pRef->omiRefLambda[j][i]);
// 	  fprintf(fp,"\n");
// 	 }
// 	fclose(fp);
// }


 	// Return

 	return rc;
 }

RC OMI_GetReference(ENGINE_CONTEXT *pEngineContext,DoasCh *refFile,INDEX indexColumn,double *lambda,double *ref,double *refSigma)
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
  // DoasCh filePath[MAX_STR_SHORT_LEN+1];
  // DoasCh fileFilter[MAX_STR_SHORT_LEN+1];
  // #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
  // WIN32_FIND_DATA fileInfo,fileInfoSub;                                         // structure returned by FindFirstFile and FindNextFile APIs
  // HANDLE hDir,hDirSub;                                                          // handle to use with by FindFirstFile and FindNextFile APIs
  // #else
  // struct dirent *fileInfo;
  // DIR *hDir;
  // #endif
  INDEX indexFile;
  // INT searchAllOrbits;
  // DoasCh *ptr,*fileExt;
  INT oldCurrentIndex,cday;
  SHORT_DATE currentDate;
  struct time currentTime;
  int currentMs;
  double Tm;

  RC rc;                                                                  // return code

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("OMI_Set",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  pEngineContext->recordNumber=0;
  oldCurrentIndex=omiCurrentFileIndex;
  omiCurrentFileIndex=ITEM_NONE;
  omiSwathOld=ITEM_NONE;
  rc=ERROR_ID_NO;

  // CODA initialization

  if (!OMI_beatLoaded)
   {
    coda_init();
    coda_set_option_bypass_special_types(0);                                    // do not bypass special types
    coda_set_option_perform_boundary_checks(0);
    coda_set_option_perform_conversions(1);                                     // conversions authorized

    OMI_beatLoaded=1;
   }

  // // In automatic reference selection, the file has maybe already loaded
  //
  // if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto)
  //  {
  //   // Close the previous files
  //
  //   if (omiOrbitFilesN && (oldCurrentIndex!=ITEM_NONE) && (oldCurrentIndex<omiOrbitFilesN) &&
  //      (omiOrbitFiles[oldCurrentIndex].omiPf!=NULL))
  //    {
  //    	coda_close(omiOrbitFiles[oldCurrentIndex].omiPf);
  //     omiOrbitFiles[oldCurrentIndex].omiPf=NULL;
  //    }
  //
  //   for (indexFile=0;indexFile<omiOrbitFilesN;indexFile++)
  //    if ((strlen(pEngineContext->fileInfo.fileName)==strlen(omiOrbitFiles[indexFile].omiFileName)) &&
  //        !strcasecmp(pEngineContext->fileInfo.fileName,omiOrbitFiles[indexFile].omiFileName))
  //     break;
  //
  //   if (indexFile<omiOrbitFilesN)
  //    omiCurrentFileIndex=indexFile;
  //  }
  //
  if (omiCurrentFileIndex==ITEM_NONE)
   {
  //  	searchAllOrbits=0;
  //
   	// Release old buffers

    OMI_ReleaseBuffers();

   	// Get the number of files to load
  //
  //  	if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto)
  //   	{
  //   		omiLoadReferenceFlag=1;
  //
  // 		  // Get file path
  //
  //  	  strcpy(filePath,pEngineContext->fileInfo.fileName);
  //
  //  	  if ((ptr=strrchr(filePath,PATH_SEP))==NULL)
  //  	   {
  // 	   	 strcpy(filePath,".");
  // 	   	 ptr=pEngineContext->fileInfo.fileName;
  // 	   	}
  // 	   else
  // 	    *ptr++=0;
  //
  // 	   fileExt=strrchr(ptr,'.');
  //
 	//  	  // Build file filter
  //
 	//  	  strcpy(fileFilter,pEngineContext->fileInfo.fileName);
 	//  	  if ((ptr=strrchr(fileFilter,PATH_SEP))==NULL)
 	//  	   {
 	//  	    strcpy(fileFilter,".");
 	//  	    ptr=fileFilter+1;
 	//  	   }
  //
 	//  	  if (fileExt==NULL)
 	//  	   sprintf(ptr,"%c*.*",PATH_SEP);
 	//  	  else if (!pEngineContext->analysisRef.refLon || strcasecmp(fileExt,".nadir"))
 	//  	   sprintf(ptr,"%c*.%s",PATH_SEP,fileExt);
 	//  	  else
 	//  	   {
 	//  	   	*ptr='\0';
 	//  	   	if ((ptr=strrchr(fileFilter,PATH_SEP))==NULL)                           // goto the parent directory
	 // 	      sprintf(fileFilter,"*.nadir");
	 // 	     else
	 // 	      {
	 // 	      	searchAllOrbits=1;
	 // 	      	*ptr='\0';
	 // 	      	strcpy(filePath,fileFilter);
 	//  	      sprintf(ptr,"%c*.*",PATH_SEP);
 	//  	     }
 	//  	   }
  //
  //     // Search for files of the same orbit
  //
  //     for (hDir=opendir(filePath);(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);)
  //       {
  //         sprintf(omiOrbitFiles[omiOrbitFilesN].omiFileName,"%s/%s",filePath,fileInfo->d_name);
  //         if ( STD_IsDir(omiOrbitFiles[omiOrbitFilesN].omiFileName) == 0 )
  //            omiOrbitFilesN++;
  //       }
  //
  //     if ( hDir != NULL ) closedir(hDir);
  //
  //    //   for (hDir=opendir(filePath);(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);)
  //    //    {
  //    //     sprintf(GDP_BIN_orbitFiles[gdpBinOrbitFilesN].gdpBinFileName,"%s/%s",filePath,fileInfo->d_name);
  //    //     if (!STD_IsDir(GDP_BIN_orbitFiles[gdpBinOrbitFilesN].gdpBinFileName))
  //    //      {
  //    //      	strcpy(filePrefix,fileInfo->d_name);
  //    //      	filePrefix[6]='\0';
  //    //
  //    //       if (((ptr=strrchr(fileInfo->d_name,'.'))!=NULL) && (strlen(ptr+1)==strlen(fileExt)) && !strcasecmp(ptr+1,fileExt) &&
  //    //            (strlen(filePrefix)==strlen(fileFilter)) && !strcasecmp(filePrefix,fileFilter))
  //    //        gdpBinOrbitFilesN++;
  //    //      }
  //    //    }
  //
  //     rc=ERROR_ID_NO;
  //  	 }
  //  	else
  //    {
     	omiOrbitFilesN=1;
     	omiCurrentFileIndex=0;
     	strcpy(omiOrbitFiles[0].omiFileName,pEngineContext->fileInfo.fileName);
  // }

    // Load files

    for (omiTotalRecordNumber=indexFile=0;indexFile<omiOrbitFilesN;indexFile++)
     {
    	 pOrbitFile=&omiOrbitFiles[indexFile];

     	pOrbitFile->omiPf=NULL;
     	pOrbitFile->specNumber=0;

      // Open the file

      if (!(rc=OmiOpen(pOrbitFile,OMI_EarthSwaths[pEngineContext->project.instrumental.omi.spectralType])))
       {



  //     	Gome2ReadOrbitInfo(pOrbitFile,(int)pEngineContext->project.instrumental.user);
  //     	Gome2BrowseMDR(pOrbitFile,(int)pEngineContext->project.instrumental.user);
  //
  //     	if ((pOrbitFile->specNumber=(THRD_browseType==THREAD_BROWSE_DARK)?1:pOrbitFile->omiInfo.total_nadir_obs)>0)
  //     	 {
  //     	  if (((pOrbitFile->omiGeolocations=(SATELLITE_GEOLOC *)MEMORY_AllocBuffer("OMI_Set","geoloc",pOrbitFile->specNumber,sizeof(SATELLITE_GEOLOC),0,MEMORY_TYPE_STRUCT))==NULL) ||
  //            ((pOrbitFile->omiLatIndex=(INDEX *)MEMORY_AllocBuffer("OMI_Set","omiLatIndex",pOrbitFile->specNumber,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
  //            ((pOrbitFile->omiLonIndex=(INDEX *)MEMORY_AllocBuffer("OMI_Set","omiLonIndex",pOrbitFile->specNumber,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
  //            ((pOrbitFile->omiSzaIndex=(INDEX *)MEMORY_AllocBuffer("OMI_Set","omiSzaIndex",pOrbitFile->specNumber,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL))
  //
  //     	   rc=ERROR_ID_ALLOC;
  //     	  else if (!(rc=Gome2ReadSunRef(pOrbitFile,(int)pEngineContext->project.instrumental.user)))
  //   	   	 Gome2ReadGeoloc(pOrbitFile,(int)pEngineContext->project.instrumental.user);
  //       }
  //
        if (pOrbitFile->omiPf!=NULL)
         {
     	    coda_close(pOrbitFile->omiPf);
          pOrbitFile->omiPf=NULL;
         }

        if ((strlen(pEngineContext->fileInfo.fileName)==strlen(pOrbitFile->omiFileName)) &&
            !strcasecmp(pEngineContext->fileInfo.fileName,pOrbitFile->omiFileName))
         omiCurrentFileIndex=indexFile;

        pEngineContext->recordNumber=pOrbitFile->specNumber;

        omiTotalRecordNumber+=pOrbitFile->omiNumberOfSwaths;
        NDET=pOrbitFile->omiNumDataPoints;

        if (rc!=ERROR_ID_NO)
         pOrbitFile->rc=rc;

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
    if (!(rc=pOrbitFile->rc) && (pOrbitFile->omiPf==NULL) &&
        !(rc=coda_open(pOrbitFile->omiFileName,&pOrbitFile->omiPf)) &&
        !(rc=coda_cursor_set_product(&pOrbitFile->omiCursor,pOrbitFile->omiPf)))
     {
     	pEngineContext->recordInfo.omi.omiNumberOfSwaths=pOrbitFile->omiNumberOfSwaths;
     	pEngineContext->recordInfo.omi.omiNumberOfRows=pOrbitFile->omiNumberOfSpectraPerSwath;
     }
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
  INDEX   refCol,                                                               // the wavelength reference column
          i;                                                                    // index for loops and arrays
  double pixDelta;
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

    if (!pEngineContext->project.instrumental.omi.omiTracks[indexSpectrum])
     rc=ERROR_ID_FILE_RECORD;
    else if (!(rc=OmiGetSpectrum(&pOrbitFile->omiCursor,pOrbitFile->omiFileName,pOrbitFile->omiSwathName,indexSwath,indexSpectrum,&pOrbitFile->omiSwath.spectrum,pOrbitFile->omiNumDataPoints,pData->wavelengthReferenceColumn[indexSwath],lambda,spectrum,sigma)))
     {
//  {
//  	FILE *fp;
//  	int i;
//  	fp=fopen("OMI_Spec.dat","w+t");
//  	for (i=0;i<NDET;i++)
//    fprintf(fp,"%g %g %g %g\n",OMI_ref[0].omiRefLambda[indexSpectrum][i],OMI_ref[0].omiRefSpectrum[indexSpectrum][i],lambda[i],spectrum[i]);
//  	fclose(fp);
//  }
     	if ((THRD_id==THREAD_TYPE_ANALYSIS) && omiRefFilesN)
     	 {
     	 	if (omiSwathOld!=indexSwath)
     	 	 {
     	 	 	KURUCZ_indexLine=1;
     	 	 	omiSwathOld==indexSwath;
     	 	 }

     	 	memcpy(pEngineContext->buffers.irrad,OMI_ref[0].omiRefSpectrum[indexSpectrum],sizeof(double)*NDET);
     	 }

      // refCol=pData->wavelengthReferenceColumn[indexSwath];
      //
     	// // Build the wavelength calibration and the spectrum
      //
     	// for (i=0;i<NDET;i++)
     	//  {
     	//  	pixDelta=(double)(i-refCol);
      //
     	//   lambda[i]=(double)pSpectrum->wavelengthCoefficient[0]+
     	//                    (pSpectrum->wavelengthCoefficient[1]+
     	//                    (pSpectrum->wavelengthCoefficient[2]+
     	//                    (pSpectrum->wavelengthCoefficient[3]+
     	//                     pSpectrum->wavelengthCoefficient[4]*pixDelta)*pixDelta)*pixDelta)*pixDelta;
      //
    	 //   spectrum[i]=(double)pSpectrum->mantissa[i]*STD_Pow10((int)pSpectrum->exponent[i]);
    	 //   sigma[i]=(double)pSpectrum->precisionMantissa[i]*STD_Pow10((int)pSpectrum->exponent[i]);
     	//  }

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
     	pRecord->omi.omiGroundPQF=(DoasUS)pGeo->groundPixelQualityFlags[recordNo-1];          // ground pixel quality flag
     	pRecord->omi.omiXtrackQF=(DoasUS)pGeo->xtrackQualityFlags[recordNo-1];                // xtrack quality flag

     	OMI_FromTAI1993ToYMD((double)pGeo->time[indexSwath],&pRecord->present_day,&pRecord->present_time,&OMI_ms);

     	pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);
     }
   }

  // Return

 	return rc;
 }

// !!! double OMI_ms;                                                                  // number of milliseconds
// !!!
// !!! // ================
// !!! // STATIC VARIABLES
// !!! // ================
// !!!
// !!! static UCHAR *omiVdata[OMI_VDATA_MAX]= { "Geolocation Fields", "Data Fields", "Swath Attributes" };
// !!!
// !!! static OMI_SWATH       omiSwathIrrad;                                           // information on the irradiance spectra
// !!! static OMI_SWATH       omiEarthSwath;                                           // information on the current earth swath
// !!! static int             omiNumberOfSwaths;                                       // the total number of tracks
// !!! static int             omiNumberOfSpectraPerSwath;                              // the number of spectra per track
// !!!
// !!! static int32  omiFileId=FAIL;                                                   // id of the OMI HDF file
// !!! static int32  omiSdsId=FAIL;                                                    // id of the SDS interface of the HDF file
// !!!
// !!! // =========
// !!! // FUNCTIONS
// !!! // =========
// !!!
// !!! // =========
// !!! // UTILITIES
// !!! // =========
// !!!


// ========
// ANALYSIS
// ========

// -----------------------------------------------------------------------------
// FUNCTION      OMI_LoadAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Load analysis parameters depending on the irradiance spectrum
//
// INPUT         pEngineContext    data on the current file
//
// RETURN        0 for success
// -----------------------------------------------------------------------------

RC OMI_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,INDEX indexFile,INDEX indexSpectrum,void *responseHandle)
 {
  // Declarations

  OMI_ORBIT_FILE *pOrbitFile;                                                 // pointer to the current orbit
  INDEX indexFeno,indexTabCross,indexWindow;                                    // indexes for loops and array
  CROSS_REFERENCE *pTabCross;                                                   // pointer to the current cross section
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a symbol
  FENO *pTabFeno;                                                               // pointer to the current spectral analysis window
  double lambdaMin,lambdaMax;                                                   // working variables
  INT DimL,useUsamp,useKurucz,saveFlag;                                         // working variables
  RC rc;                                                                        // return code

  // Initializations

//  DEBUG_Print(DOAS_logFile,"Enter OMI_LoadAnalysis\n");

  rc=ERROR_ID_NO;
  pOrbitFile=&omiOrbitFiles[indexFile];
  saveFlag=(INT)pEngineContext->project.spectra.displayDataFlag;

//   if (!(rc=pOrbitFile->rc) && (OMILoadReferenceFlag || !pEngineContext->analysisRef.refAuto))
//    {
//     lambdaMin=(double)9999.;
//     lambdaMax=(double)-9999.;
//
//     rc=ERROR_ID_NO;
//     useKurucz=useUsamp=0;
//
//     // Browse analysis windows and load missing data
//
//     for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
//      {
//       pTabFeno=&TabFeno[indexFeno];
//       pTabFeno->NDET=NDET;
//
//       // Load calibration and reference spectra
//
//       if (!pTabFeno->gomeRefFlag)
//        {
//         memcpy(pTabFeno->LambdaRef,pOrbitFile->OMISunWve,sizeof(double)*NDET);
//         memcpy(pTabFeno->Sref,pOrbitFile->OMISunRef,sizeof(double)*NDET);
//
//         if (!TabFeno[indexFeno].hidden)
//          {
//           if (!(rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&pTabFeno->refNormFact,"OMI_LoadAnalysis (Reference) ")))
//            {
//             memcpy(pTabFeno->SrefEtalon,pTabFeno->Sref,sizeof(double)*pTabFeno->NDET);
//             pTabFeno->useEtalon=pTabFeno->displayRef=1;
//
//             // Browse symbols
//
//             for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
//              {
//               pTabCross=&pTabFeno->TabCross[indexTabCross];
//               pWrkSymbol=&WorkSpace[pTabCross->Comp];
//
//               // Cross sections and predefined vectors
//
//               if ((((pWrkSymbol->type==WRK_SYMBOL_CROSS) && (pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING)) ||
//                    ((pWrkSymbol->type==WRK_SYMBOL_PREDEFINED) &&
//                    ((indexTabCross==pTabFeno->indexCommonResidual) ||
//                   (((indexTabCross==pTabFeno->indexUsamp1) || (indexTabCross==pTabFeno->indexUsamp2)) && (pUsamp->method==PRJCT_USAMP_FILE))))) &&
//                    ((rc=ANALYSE_CheckLambda(pWrkSymbol,pTabFeno->LambdaRef,"OMI_LoadAnalysis "))!=ERROR_ID_NO))
//
//                goto EndOMI_LoadAnalysis;
//              }
//
//             // Gaps : rebuild subwindows on new wavelength scale
//
//             for (indexWindow=0,DimL=0;indexWindow<pTabFeno->svd.Z;indexWindow++)
//              {
//               pTabFeno->svd.Fenetre[indexWindow][0]=FNPixel(pTabFeno->LambdaRef,pTabFeno->svd.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
//               pTabFeno->svd.Fenetre[indexWindow][1]=FNPixel(pTabFeno->LambdaRef,pTabFeno->svd.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);
//
//               DimL+=(pTabFeno->svd.Fenetre[indexWindow][1]-pTabFeno->svd.Fenetre[indexWindow][0]+1);
//              }
//
//             pTabFeno->svd.DimL=DimL;
//
//             // Buffers allocation
//
//             SVD_Free("OMI_LoadAnalysis",&pTabFeno->svd);
//             SVD_LocalAlloc("OMI_LoadAnalysis",&pTabFeno->svd);
//
//             pTabFeno->Decomp=1;
//
//             if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef))!=ERROR_ID_NO) ||
//                 ((!pKuruczOptions->fwhmFit || !pTabFeno->useKurucz) && pTabFeno->xsToConvolute &&
//                 ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,&ANALYSIS_slit,&ANALYSIS_slit2,pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2))!=ERROR_ID_NO)))
//
//              goto EndOMI_LoadAnalysis;
//            }
//          }
//
//         memcpy(pTabFeno->LambdaK,pTabFeno->LambdaRef,sizeof(double)*pTabFeno->NDET);
//         memcpy(pTabFeno->Lambda,pTabFeno->LambdaRef,sizeof(double)*pTabFeno->NDET);
//
//         useUsamp+=pTabFeno->useUsamp;
//         useKurucz+=pTabFeno->useKurucz;
//
//         if (pTabFeno->useUsamp)
//          {
//           if (pTabFeno->LambdaRef[0]<lambdaMin)
//            lambdaMin=pTabFeno->LambdaRef[0];
//           if (pTabFeno->LambdaRef[pTabFeno->NDET-1]>lambdaMax)
//            lambdaMax=pTabFeno->LambdaRef[pTabFeno->NDET-1];
//          }
//        }
//      }
//
//     // Wavelength calibration alignment
//
//     if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
//      {
//       KURUCZ_Init(0);
//
//       if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,responseHandle))!=ERROR_ID_NO))
//        goto EndOMI_LoadAnalysis;
//      }
//
//     // Build undersampling cross sections
//
//     if (useUsamp && (THRD_id!=THREAD_TYPE_KURUCZ))
//      {
//       ANALYSE_UsampLocalFree();
//
//       if (((rc=ANALYSE_UsampLocalAlloc(0))!=ERROR_ID_NO) ||
//           ((rc=ANALYSE_UsampBuild(0,0))!=ERROR_ID_NO) ||
//           ((rc=ANALYSE_UsampBuild(1,ITEM_NONE))!=ERROR_ID_NO))
//
//        goto EndOMI_LoadAnalysis;
//      }
//
//     // Automatic reference selection
//
//     if (OMILoadReferenceFlag && !(rc=OMINewRef(pEngineContext,responseHandle)) &&
//        !(rc=ANALYSE_AlignReference(pEngineContext,2,pEngineContext->project.spectra.displayDataFlag,responseHandle))) // automatic ref selection for Northern hemisphere
//          rc=ANALYSE_AlignReference(pEngineContext,3,pEngineContext->project.spectra.displayDataFlag,responseHandle);     // automatic ref selection for Southern hemisphere
//    }

  // Return

  EndOMI_LoadAnalysis :

//  DEBUG_Print(DOAS_logFile,"End OMI_LoadAnalysis %d\n",rc);

  return rc;
 }

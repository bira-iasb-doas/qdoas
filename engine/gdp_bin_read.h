#ifndef GDP_BIN_READ_H
#define GDP_BIN_READ_H

#include <stdio.h>

#include "doas.h"

#define SCIENCE_DATA_DEFINED          10                                        // number of bands
#define SPECTRAL_FITT_ORDER            5                                        // degree of polynomial used for building wavelength scale
#define	MAX_FITT_ORDER	                6
#define GDP_BIN_ERROR_ID_MASK       0x01

enum _gomeGetCommand
 {
  GOME_GET_LEMBDA,
  GOME_GET_SPECTRUM,
  GOME_GET_REFERENCE
 };

#pragma pack(push,1)

// Date in short format
typedef struct _SHORTDateTime
 {
  char  da_year;        /* Year - 1900      */
  char  da_day;         /* Day of the month */
  char  da_mon;         /* Month (1 = Jan)  */
  char  ti_hour;
  char  ti_min;
  char  ti_sec;
 }
SHORT_DATETIME;

// File header
typedef struct _gomeFileHeader
 {
  unsigned short nspectra;                    // total number of spectra in file
  char           version;
  char           mask;
  int            headerSize;                  // number of bytes before first record
  int            recordSize;                  // size of a record
  int            orbitNumber;                 // orbit number
  short          nbands;                      // number of available bands
  char           nSpectralParam;              // number of set of spectral parameters
  char           indexSpectralParam;          // index of set of spectral parameters for irradiance spectra
  SHORT_DATETIME dateAndTime;                 // measurement date and time in UT
 }
GDP_BIN_FILE_HEADER;

// For each available band, provide header with band and reference general info

typedef struct _gomeBandHeader
 {
  // Band info

  char       bandType;                        // band type
  short      bandSize;                        // band size

  // Reference info

  short      startDetector;                   // index of first pixel used on detector for this ban
  float      scalingFactor;
  float      scalingError;
 }
GDP_BIN_BAND_HEADER;

// Spectrum record structure

typedef struct                            // geolocation coordinates version 3
 {
  unsigned short          lonArray[5];            // longitude array
  short           latArray[5];            // latitude array
  float           szaArray[3];            // zenithal array
  unsigned short          losZa[3];               // line of sight zenithal array
  unsigned short          losAzim[3];             // line of sight azimuthal array
  float           satHeight;              // satellite geodetic height at point B
  float           radiusCurve;            // Earth radius curvatur at point B

  // From Level 2 data

  unsigned short  o3;                     // O3 VCD
  unsigned short  no2;                    // NO2 VCD
  unsigned short  cloudFraction;          // Cloud fraction
  unsigned short  cloudTopPressure;       // Cloud top pressure
  float           aziArray[3];
  unsigned short  unused[4];             // for later new data ?
 }
GEO_3;

// Cloud information : new in version 4.00, may 2009

typedef struct
 {
	 float SurfaceHeight;
	 float SurfaceAlbedo;
	 float UV_Albedo;
	 int Elevation;
	 float CloudFraction[2]; /* Cloud Fraction and error */
	 float CloudTopAlbedo[2]; /* Cloud Top Albedo and error */
	 float CloudTopHeight[2]; /* Cloud Top Height and error */
	 float TAU[2]; /* Cloud Optical Thickness and error */
	 float CTP[2]; /* Cloud Top Pressure and error */
	 short Mode; /* 0=normal, 1=snow/ice */
	 short Type; /* 1=Cirrus, 2=Cirrostratus, 3=Deep convection, 4=Altocumulus, 5=Altostratus, etc */
 }
GDP_BIN_CLOUD_HEADER;

typedef struct                            // geolocation coordinates version 4.00 from May 2009
 {
  unsigned short       lonArray[5];       // longitude array
  short                latArray[5];       // latitude array
  float                szaArrayTOA[3];    // solar zenithal angles, top of atmosphere
  float                aziArrayTOA[3];    // solar azimuth angles, top of atmosphere
  float                losZaTOA[3];       // line of sight zenithal angles, top of atmosphere
  float                losAzimTOA[3];     // line of sight azimuth angles, top of atmosphere
  float                szaArrayBOA[3];    // solar zenithal angles, bottom of atmosphere
  float                aziArrayBOA[3];    // solar azimuth angles, bottom of atmosphere
  float                losZaBOA[3];       // line of sight zenithal angles, bottom of atmosphere
  float                losAzimBOA[3];     // line of sight azimuth angles, bottom of atmosphere
  float                satHeight;         // satellite geodetic height at point B
  float                radiusCurve;       // Earth radius curvature at point B
  GDP_BIN_CLOUD_HEADER cloudInfo;
 }
GEO_4;

typedef struct
 {
  // From Level 1 data

  SHORT_DATETIME  dateAndTime;                // measurement date and time in UT
  short           groundPixelID;              // ground pixel order
  char            groundPixelType;            // ground pixel type
  char            indexSpectralParam;         // index of set of spectral parameters in reference record to use for building calibration
 }
SPECTRUM_RECORD;

// Keep useful information for fast access to a record

typedef struct _gome_recordInfo
 {
  INDEX  pixelNumber;                                                           // pixel number
  INDEX  pixelType;                                                             // pixel type
  double lat;                                                                   // latitude
  double lon;                                                                   // longitude
  double sza;                                                                   // solar zenith angle
 }
GDP_BIN_INFO;

typedef struct _GOMEOrbitFiles                                                  // description of an orbit
 {
 	char gdpBinFileName[MAX_STR_LEN+1];                                            // the name of the file with a part of the orbit
 	char gdpBinFileNumber[9];
  GDP_BIN_INFO *gdpBinInfo;                                                     // useful information on records for fast access
  INDEX gdpBinBandIndex;                                                        // indexes of bands present in the current file
  INDEX *gdpBinLatIndex,*gdpBinLonIndex,*gdpBinSzaIndex,*gdpBinPixIndex;        // indexes of records sorted resp. by latitudes, by SZA or by pixel number
  GDP_BIN_FILE_HEADER gdpBinHeader;
  GDP_BIN_BAND_HEADER gdpBinBandInfo[SCIENCE_DATA_DEFINED];
  SPECTRUM_RECORD     gdpBinSpectrum;
  GEO_3               gdpBinGeo3;
  GEO_4               gdpBinGeo4;
  int                 gdpBinSpectraSize,                                        // total size of spectra vector GDP_BIN_coeff
                      gdpBinCoeffSize,                                          // number of polynomial coefficients in vector
                      gdpBinStartPixel[SCIENCE_DATA_DEFINED];                   // starting pixels for bands present in the file
  double             *gdpBinCoeff;                                              // coefficients for reconstructing wavelength calibrations
  float               gdpBinScalingFactor[SCIENCE_DATA_DEFINED],                // scaling factors for spectra band per band
                      gdpBinScalingError[SCIENCE_DATA_DEFINED];                 // scaling factors for errors band per band
  unsigned short             *gdpBinReference,                                          // buffer for irradiance spectra
                     *gdpBinRefError;                                           // errors on irradiance spectra
  int                 specNumber;
  RC rc;
 }
GOME_ORBIT_FILE;

#pragma pack(pop)

// ---------------------
// VARIABLES DECLARATION
// ---------------------

#define MAX_GOME_FILES 50 // maximum number of files per orbit

extern GOME_ORBIT_FILE GDP_BIN_orbitFiles[MAX_GOME_FILES];                      // list of files for an orbit
extern INDEX GDP_BIN_currentFileIndex;                                          // index of the current file in the list
extern char *GDP_BIN_BandStrTab[];

// ----------
// PROTOTYPES
// ----------

INDEX            GDP_BIN_GetRecordNumber(int pixelNumber);
RC               GDP_BIN_GetBand(ENGINE_CONTEXT *pEngineContext,int bandNo);
void             GDP_BIN_GetReferenceInfo(ENGINE_CONTEXT *pEngineContext);
RC               GDP_BIN_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,FILE *specFp,void *responseHandle);
RC               GDP_BIN_get_orbit_date(int *year, int *month, int *day);


#endif

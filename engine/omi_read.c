
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  OMI interface
//  Name of module    :  omi_read.c
//  Program Language  :  C
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
//  LIBRARIES
//
//  This module uses HDF-EOS (Hierarchical Data Format - Earth Observing System)
//  libraries based on HDF-4
//
//  ----------------------------------------------------------------------------

#include "mediate.h"
#include "engine.h"
#include "hdf.h"
#include "HdfEosDef.h"

#include <time.h>

#define MAX_OMI_FILES 500

// Omi field names for readout routines:
#define REFERENCE_COLUMN "WavelengthReferenceColumn"
#define WAVELENGTH_COEFFICIENT "WavelengthCoefficient"
#define OMI_NUM_COEFFICIENTS 5 // 5 coefficients in wavelenght polynomial
#define OMI_NUM_ROWS 60
#define OMI_XTRACK_NOTUSED 255 // XTrackQualityFlags = 255 is used to label unused rows in the detector during special zoom mode.
#define RADIANCE_MANTISSA "RadianceMantissa"
#define RADIANCE_PRECISION_MANTISSA "RadiancePrecisionMantissa"
#define RADIANCE_EXPONENT "RadianceExponent"
#define IRRADIANCE_MANTISSA "IrradianceMantissa"
#define IRRADIANCE_PRECISION_MANTISSA "IrradiancePrecisionMantissa"
#define IRRADIANCE_EXPONENT "IrradianceExponent"
#define PIXEL_QUALITY_FLAGS "PixelQualityFlags"
#define NXTRACK "nXtrack"
#define NWAVEL "nWavel"

// ========================
// DEFINITION OF STRUCTURES
// ========================

enum _omiSwathType { OMI_SWATH_UV1, OMI_SWATH_UV2, OMI_SWATH_VIS, OMI_SWATH_MAX };
enum _omiVdataType { OMI_VDATA_GEO, OMI_VDATA_DATA, OMI_VDATA_ATTR, OMI_VDATA_MAX };
enum _omiSpecType  { OMI_SPEC_IRRAD, OMI_SPEC_RAD };

struct omi_buffer {
  const char *buffername;
  void *bufferptr;
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
  unsigned short *pixelQualityFlags;
  float   wavelengthCoefficient[5];
  float   wavelengthCoefficientPrecision[5];
}
  OMI_SPECTRUM;

typedef struct _omi_ref
{
  double       **omiRefLambda;
  double       **omiRefSpectrum;
  double       **omiRefSigma;
  char         omiRefFileName[MAX_STR_LEN+1];
  OMI_SPECTRUM   spectrum;
  long           nXtrack,nWavel;
  double        *omiRefFact;
  int            year,cday;
}
  OMI_REF;

typedef struct _omi_data_fields
{
  short   *wavelengthReferenceColumn;
  unsigned short  *measurementQualityFlags;
}
  OMI_DATA_FIELDS;

typedef struct _omi_swath_earth
{
  OMI_GEO         geolocationFields;
  OMI_SPECTRUM    spectrum;
  OMI_DATA_FIELDS dataFields;
}
  OMI_SWATH;

typedef struct _OMIOrbitFiles // description of an orbit
{
  char      *omiFileName;       // the name of the file with a part of the orbit
  OMI_SWATH *omiSwath;          // all information about the swath
  int        specNumber;
  int32      swf_id,            // hdfeos swath file id
    sw_id;                      // hdfeos swath id
  long       nMeasurements,
    nXtrack,                    // number of detector tracks (normally 60)
    nWavel;
  int        rc;
}
  OMI_ORBIT_FILE;

/* Before calculating the automatic reference spectrum, we build a
 * list of all spectra matching the search criteria for one of the
 * analysis windows and one of the detector rows.  This is to avoid
 * reading the same spectrum twice when it is used in the reference
 * calculation for multiple analysis windows.
 */
struct omi_ref_spectrum
{
  float                    solarZenithAngle;
  float                    latitude;
  float                    longitude;
  int                      measurement_number;
  int                      detector_row;
  double                  *wavelengths;
  double                  *spectrum;
  double                  *errors;
  OMI_ORBIT_FILE          *orbit_file; // orbit file containing this spectrum
  struct omi_ref_spectrum *next; // next in the list;
};

/* List of spectra to be used in the automatic reference calculation
 * for a single pair (analysiswindow, detector row).
 */
struct omi_ref_list
{
  struct omi_ref_spectrum *reference;
  struct omi_ref_list *next;
};

const char *OMI_EarthSwaths[OMI_SWATH_MAX]={"Earth UV-1 Swath","Earth UV-2 Swath","Earth VIS Swath"};
const char *OMI_SunSwaths[OMI_SWATH_MAX]={"Sun Volume UV-1 Swath","Sun Volume UV-2 Swath","Sun Volume VIS Swath"};

// ================
// STATIC VARIABLES
// ================

static OMI_ORBIT_FILE current_orbit_file;
static int omiRefFilesN=0; // the total number of files to browse in one shot
static int omiTotalRecordNumber=0;
static OMI_REF OMI_ref[MAX_FENO]; // the number of reference spectra is limited to the maximum number of analysis windows in a project

static OMI_ORBIT_FILE* reference_orbit_files[MAX_OMI_FILES]; // List of filenames for which the current automatic reference spectrum is valid. -> all spectra from the same day/same directory.
static int num_reference_orbit_files = 0;
static bool automatic_reference_ok[OMI_NUM_ROWS]; // array to keep track if automatic reference creation spectrum failed for one of the detector rows

int OMI_ms=0;
int omiSwathOld=ITEM_NONE;

static RC OmiOpen(OMI_ORBIT_FILE *pOrbitFile,const char *swathName);
static void omi_free_swath_data(OMI_SWATH *pSwath);
static void omi_calculate_wavelengths(float32 wavelength_coeff[], int16 refcol, int32 n_wavel, double* lambda);
static void omi_make_double(int16 mantissa[], int8 exponent[], int32 n_wavel, double* result);
static void omi_interpolate_errors(int16 mantissa[], int32 n_wavel, double wavelengths[], double y[] );
static RC omi_load_spectrum(int spec_type, int32 sw_id, int32 measurement, int32 track, int32 n_wavel, double *lambda, double *spectrum, double *sigma, unsigned short *pixelQualityFlags);
static void average_spectrum(double *average, double *errors, struct omi_ref_list *spectra, double *wavelength_grid);

// ===================
// ALLOCATION ROUTINES
// ===================

void OMI_ReleaseReference(void)
{
  // Declarations

  OMI_REF *pRef;

  // Initialization

  for (int i=0;i<omiRefFilesN;i++)
    {
      pRef=&OMI_ref[i];

      if (pRef->omiRefLambda!=NULL)
	MEMORY_ReleaseDMatrix(__func__,"omiRefLambda",pRef->omiRefLambda,0,pRef->nXtrack-1,0);
      if (pRef->omiRefSpectrum!=NULL)
	MEMORY_ReleaseDMatrix(__func__,"omiRefSpectrum",pRef->omiRefSpectrum,0,pRef->nXtrack-1,0);
      if (pRef->omiRefFact!=NULL)
	MEMORY_ReleaseDVector(__func__,"omiRefSpectrumK",pRef->omiRefFact,0);
      if (pRef->omiRefSigma!=NULL)
	MEMORY_ReleaseDMatrix(__func__,"omiRefSigma",pRef->omiRefSigma,0,pRef->nXtrack-1,0);

      if (pRef->spectrum.pixelQualityFlags!=NULL)                                          // pixelquality
	MEMORY_ReleaseBuffer(__func__,"pRef->spectrum.pixelQualityFlags",pRef->spectrum.pixelQualityFlags);

      memset(pRef,0,sizeof(OMI_ref[i]));
    }

  omiRefFilesN=0;
}

static RC OMI_AllocateReference(INDEX indexRef,int nSpectra,int nPoints)
{
  // Declarations

  OMI_REF *pRef;
  RC rc;

  // Initializations

  pRef=&OMI_ref[indexRef];
  rc=ERROR_ID_NO;

  if (((pRef->omiRefLambda=(double **)MEMORY_AllocDMatrix(__func__,"omiRefLambda",0,nPoints-1,0,nSpectra-1))==NULL) ||
      ((pRef->omiRefSpectrum=(double **)MEMORY_AllocDMatrix(__func__,"omiRefSpectrum",0,nPoints-1,0,nSpectra-1))==NULL) ||
      ((pRef->omiRefFact=(double *)MEMORY_AllocDVector(__func__,"omiRefFact",0,nSpectra-1))==NULL) ||
      ((pRef->omiRefSigma=(double **)MEMORY_AllocDMatrix(__func__,"omiRefSigma",0,nPoints-1,0,nSpectra-1))==NULL) ||

      ((pRef->spectrum.pixelQualityFlags=(unsigned short *)MEMORY_AllocBuffer(__func__,"pixelQualityFlags",nPoints,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL))

    rc=ERROR_ID_ALLOC;

  pRef->nXtrack=nSpectra;
  pRef->nWavel=nPoints;

  // Return

  return rc;
}

/*! Use a detector row or not, based on XTrackQualityFlags and the
 *  chosen settings in the analysis configuration.
 *
 * OMI XTrackQualityFlags description from OMI IODS_Vol_2_issue8:
 *
 *  Bit 0-2:
 *    0 = ok
 *    1 = affected by anomaly, don't use pixel
 *    2 = slightly affected, use with caution
 *    3 = affected, not optimally corrected, use with caution
 *    4 = affected, optimally corrected, use with caution
 *    5 = "not used"
 *    6 = "not used"
 *    7 = error during correction, don't use
 *  Bit 3: reserved for future use
 *  Bit 4 = possibly affected by wavelength shift
 *  Bit 5 = possibly affected by blockage
 *  Bit 6 = possibly affected by stray sunlight
 *  Bit 7 = possibly affected by stray earthshine
 *
 *  We provide three possible uses of XTrackQualityFlags in the analysis:
 *
 *  IGNORE: don't use xtrackqualityflags
 *  STRICT: only use pixels with flag 0 (unaffected)
 *  NONSTRICT: also use pixels which are slightly affected or corrected (flag 2,3 or 4)
 */
bool omi_use_track(int quality_flag, enum omi_xtrack_mode mode)
 {
   bool result;
   if (quality_flag == OMI_XTRACK_NOTUSED) {
     result = false;
   } else {
     quality_flag &= 7; // reduce to bits 0-2
     switch(mode)
       {
       case XTRACKQF_IGNORE:
         result = true;
         break;
       case XTRACKQF_STRICT:
         result = (quality_flag == 0);
         break;
       case XTRACKQF_NONSTRICT:
         result = (quality_flag == 0 || quality_flag == 2 || quality_flag == 3 || quality_flag == 4);
         break;
       default:
         result = true;
       }
   }
   return result;
 }

/*! \brief check if automatic reference creation was successful for this row */
bool omi_has_automatic_reference(int row)
{
  return automatic_reference_ok[row];
}

/*! \brief release the allocated buffers with swath attributes. */
static void omi_free_swath_data(OMI_SWATH *pSwath)
{
  if(pSwath != NULL) {
    struct omi_buffer omi_swath_buffers[] = {
      {"pixelQualityFlags",pSwath->spectrum.pixelQualityFlags},
      {"measurementQualityFlags",pSwath->dataFields.measurementQualityFlags},
      {"wavelengthReferenceColumn",pSwath->dataFields.wavelengthReferenceColumn},
      {"secondsInDay",pSwath->geolocationFields.secondsInDay},
      {"spacecraftLatitude",pSwath->geolocationFields.spacecraftLatitude},
      {"spacecraftLongitude",pSwath->geolocationFields.spacecraftLongitude},
      {"spacecraftAltitude",pSwath->geolocationFields.spacecraftAltitude},
      {"latitude",pSwath->geolocationFields.latitude},
      {"longitude",pSwath->geolocationFields.longitude},
      {"solarZenithAngle",pSwath->geolocationFields.solarZenithAngle},
      {"solarAzimuthAngle",pSwath->geolocationFields.solarAzimuthAngle},
      {"viewingZenithAngle",pSwath->geolocationFields.viewingZenithAngle},
      {"viewingAzimuthAngle",pSwath->geolocationFields.viewingAzimuthAngle},
      {"terrainHeight",pSwath->geolocationFields.terrainHeight},
      {"groundPixelQualityFlags",pSwath->geolocationFields.groundPixelQualityFlags},
      {"xtrackQualityFlags",pSwath->geolocationFields.xtrackQualityFlags},
    };
    
    for(unsigned int i=0; i<sizeof(omi_swath_buffers)/sizeof(omi_swath_buffers[0]); i++) {
      void *ptr = omi_swath_buffers[i].bufferptr;
      if (ptr != NULL)
        MEMORY_ReleaseBuffer(__func__, omi_swath_buffers[i].buffername, ptr);
    }
    
    if (pSwath->geolocationFields.time!=NULL)
      MEMORY_ReleaseDVector(__func__, "time",pSwath->geolocationFields.time,0);

    free(pSwath);
  }

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,0);
#endif
}

static void omi_close_orbit_file(OMI_ORBIT_FILE *pOrbitFile)
{
  if(pOrbitFile->sw_id != 0) {
    SWdetach(pOrbitFile->sw_id);
    pOrbitFile->sw_id = 0;
  }
  if(pOrbitFile->swf_id != 0) {
    SWclose(pOrbitFile->swf_id);
    pOrbitFile->swf_id = 0;
  }
}

static void omi_destroy_orbit_file(OMI_ORBIT_FILE *pOrbitFile) {
  omi_close_orbit_file(pOrbitFile);
  
  free(pOrbitFile->omiFileName);
  pOrbitFile->omiFileName = NULL;

  if(pOrbitFile->omiSwath != NULL) {
    omi_free_swath_data(pOrbitFile->omiSwath);
    pOrbitFile->omiSwath = NULL;
  }
  free(pOrbitFile);
}

void OMI_TrackSelection(const char *omiTrackSelection,int *omiTracks)
{
  // Declarations

  char str[256];
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
      for (const char *ptr=omiTrackSelection;(int)(ptr-omiTrackSelection)<=256;ptr++)
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

static RC OMI_AllocateSwath(OMI_SWATH **swath,int nSwaths,int nSpectra)
{
  // Declarations

  OMI_SWATH *pSwath = malloc(sizeof(OMI_SWATH));
  *swath = pSwath;

  OMI_SPECTRUM *pSpectrum = &pSwath->spectrum;  // spectrum in earth swath
  OMI_DATA_FIELDS *pData = &pSwath->dataFields; // data on earth swath
  OMI_GEO *pGeo = &pSwath->geolocationFields;   // geolocations
  int nRecords = nSwaths*nSpectra;              // total number of spectra
  RC rc = ERROR_ID_NO;                          // Return code

  if (
      ((pSpectrum->pixelQualityFlags=(unsigned short *)MEMORY_AllocBuffer(__func__,"pixelQualityFlags",NDET,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||

      // Earth swath

      ((pData->measurementQualityFlags=(unsigned short *)MEMORY_AllocBuffer(__func__,"measurementQualityFlags",nSwaths,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
      ((pData->wavelengthReferenceColumn=(short *)MEMORY_AllocBuffer(__func__,"wavelengthReferenceColumn",nSwaths,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL) ||

      // Geolocation

      ((pGeo->time=(double *)MEMORY_AllocDVector(__func__,"time",0,nSwaths))==NULL) ||
      ((pGeo->secondsInDay=(float *)MEMORY_AllocBuffer(__func__,"secondsInDay",nSwaths,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->spacecraftLatitude=(float *)MEMORY_AllocBuffer(__func__,"spacecraftLatitude",nSwaths,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->spacecraftLongitude=(float *)MEMORY_AllocBuffer(__func__,"spacecraftLongitude",nSwaths,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->spacecraftAltitude=(float *)MEMORY_AllocBuffer(__func__,"spacecraftAltitude",nSwaths,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->latitude=(float *)MEMORY_AllocBuffer(__func__,"latitude",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->longitude=(float *)MEMORY_AllocBuffer(__func__,"longitude",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->solarZenithAngle=(float *)MEMORY_AllocBuffer(__func__,"solarZenithAngle",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->solarAzimuthAngle=(float *)MEMORY_AllocBuffer(__func__,"solarAzimuthAngle",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->viewingZenithAngle=(float *)MEMORY_AllocBuffer(__func__,"viewingZenithAngle",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->viewingAzimuthAngle=(float *)MEMORY_AllocBuffer(__func__,"viewingAzimuthAngle",nRecords,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pGeo->terrainHeight=(short *)MEMORY_AllocBuffer(__func__,"terrainHeight",nRecords,sizeof(short),0,MEMORY_TYPE_SHORT))==NULL) ||
      ((pGeo->groundPixelQualityFlags=(unsigned short *)MEMORY_AllocBuffer(__func__,"groundPixelQualityFlags",nRecords,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL) ||
      ((pGeo->xtrackQualityFlags=(uint8_t *)MEMORY_AllocBuffer(__func__,"xtrackQualityFlags",nRecords,sizeof(unsigned short),0,MEMORY_TYPE_STRING))==NULL))

    rc=ERROR_ID_ALLOC;

  // Return

  return rc;
}

// Check if the current automatic reference spectrum is still valid for the requested file name
static bool valid_reference_file(char *spectrum_file)
{
  for (int i=0; i < num_reference_orbit_files; i++ ) {
    if(!strcasecmp(spectrum_file, reference_orbit_files[i]->omiFileName))
      return true;
  }
  return false;
}

// Create the  of all orbit files in the same directory as the given file.
static RC read_reference_orbit_files(const char *spectrum_file) {
  RC rc = ERROR_ID_NO;

  // clear old reference_orbit_files array
  for(int i=0; i< num_reference_orbit_files; i++) {
    omi_destroy_orbit_file(reference_orbit_files[i]);
    reference_orbit_files[i] = NULL;
  }
  num_reference_orbit_files = 0;

  // get list of orbit files in same directory as requested file
  char current_dir[MAX_STR_SHORT_LEN+1];
  strcpy(current_dir, spectrum_file);
  
  char *directory_end = strrchr(current_dir,PATH_SEP);

  if (directory_end == NULL) { // relative path without leading './' is used
    sprintf(current_dir,".%c",PATH_SEP);
  } else {
    *(directory_end) = '\0'; // terminate current_dir string after directory separator
  }
    
  struct dirent *fileInfo;
  DIR *hDir = opendir(current_dir);
  if (hDir != NULL)
    {
      while( (fileInfo=readdir(hDir)) && num_reference_orbit_files < MAX_OMI_FILES ) {
        if(fileInfo->d_name[0] !='.') // better to use 'if (fileInfo->d_type == DT_REG)' ?
          {
            reference_orbit_files[num_reference_orbit_files] = malloc(sizeof(OMI_ORBIT_FILE));
            char *file_name = malloc(strlen(current_dir)+strlen(fileInfo->d_name) +2); //directory + path_sep + filename + trailing \0
            sprintf(file_name,"%s%c%s",current_dir,PATH_SEP,fileInfo->d_name);
            reference_orbit_files[num_reference_orbit_files]->omiFileName = file_name;
            reference_orbit_files[num_reference_orbit_files]->omiSwath = NULL;
            num_reference_orbit_files++;
          }
      }
      closedir(hDir);
    }
  else
    rc = ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,"Omi automatic reference selection");

  return rc;
}

// check if a given spectrum matches the criteria to use it in the automatic reference spectrum
static bool use_as_reference(OMI_ORBIT_FILE *orbit_file, int recordnumber, FENO *pTabFeno, enum omi_xtrack_mode xtrack_mode) {
  float lon_min = pTabFeno->refLonMin;
  float lon_max = pTabFeno->refLonMax;
  float lat_min = pTabFeno->refLatMin;
  float lat_max = pTabFeno->refLatMax;
  float sza_min = pTabFeno->refSZA - pTabFeno->refSZADelta;
  float sza_max = pTabFeno->refSZA + pTabFeno->refSZADelta;

  float lon = orbit_file->omiSwath->geolocationFields.longitude[recordnumber];
  float lat = orbit_file->omiSwath->geolocationFields.latitude[recordnumber];
  float sza = orbit_file->omiSwath->geolocationFields.solarZenithAngle[recordnumber];

  int xTrackQF = orbit_file->omiSwath->geolocationFields.xtrackQualityFlags[recordnumber];

  bool use_row = omi_use_track(xTrackQF, xtrack_mode);
  
  // if a range (0.0,0.0) is chosen ( < EPSILON), we don't select based on this parameter
  bool use_lon = lon_max - lon_min > EPSILON;
  bool use_lat = lat_max - lat_min > EPSILON;
  bool use_sza = sza_max - sza_min > EPSILON;

  return use_row
    && ((lon_min <= lon && lon_max >= lon) || !use_lon)
    && ((lat_min <= lat && lat_max >= lat) || !use_lat)
    && ((sza_min <= sza && sza_max >= sza) || !use_sza);
}

static char *automatic_reference_info(struct omi_ref_list *spectra) {
  char *filename = spectra->reference->orbit_file->omiFileName;
  struct omi_ref_list *current = spectra;
  int length = strlen(filename);
  while(current != NULL) {
    // spectra in the list are ordered per file.  We want to list each filename only once.
    if (current->reference->orbit_file->omiFileName != filename) {
      filename = current->reference->orbit_file->omiFileName;
      length += strlen(filename + 5); // allocate memory for "\n# <filename>: "
    }
    length+= 6; // " current->reference->measurement_number"
    current = current->next;
  }
  char *result = malloc(length);
  
  current = spectra;
  filename = spectra->reference->orbit_file->omiFileName;
  strcpy(result,"# ");
  strcat(result,filename);
  strcat(result,":");
  while(current != NULL) {
    if (current->reference->orbit_file->omiFileName != filename) {
      filename = current->reference->orbit_file->omiFileName;
      strcat(result,"\n# ");
      strcat(result,filename);
      strcat(result,":");
    }
    char tempstring[10];
    sprintf(tempstring, " %d",current->reference->measurement_number);
    strcat(result,tempstring);
    current = current->next;
  }

  return result;
}

static void free_ref_candidates( struct omi_ref_spectrum *reflist) {
  while(reflist != NULL) {
    struct omi_ref_spectrum *temp = reflist->next;
    free(reflist->spectrum);
    free(reflist->errors);
    free(reflist->wavelengths);
    free(reflist);
    reflist = temp;
  }
}

static void free_row_references(struct omi_ref_list *(*row_references)[NFeno][OMI_TOTAL_ROWS]) 
{
  for(int i=0; i<NFeno; i++) {
    for(int j=0; j<OMI_TOTAL_ROWS; j++) {
      struct omi_ref_list *tempref,*omi_ref = (*row_references)[i][j];
      while(omi_ref != NULL) {
        tempref = omi_ref->next;
        free(omi_ref);
        omi_ref = tempref;
      }
    }
  }
  free(row_references);
}

/* Read the geolocation data of all spectra in an orbit file and store
 * spectra matching the search criteria for the automatic reference
 * spectrum for one or more analysis windows in a list.
 */
static RC find_matching_spectra(ENGINE_CONTEXT *pEngineContext, OMI_ORBIT_FILE *orbit_file, struct omi_ref_list *(*row_references)[NFeno][OMI_TOTAL_ROWS], struct omi_ref_spectrum **first)
{
  RC rc = 0;
  int allocs = 0;

  enum omi_xtrack_mode xtrack_mode = pEngineContext->project.instrumental.omi.xtrack_mode;

  for (int measurement=0; measurement < orbit_file->nMeasurements; measurement++) {
    for(int row = 0; row < orbit_file->nXtrack; row++) {
      if (pEngineContext->project.instrumental.omi.omiTracks[row]) {

        int recordnumber = measurement*orbit_file->nXtrack + row;
        struct omi_ref_spectrum *newref = NULL; // will be allocated and used only if the spectrum is used in the automatic reference calculation for one of the analysis windows.
        
        // loop over all analysis windows and look if the current
        // spectrum can be used in the automatic reference for any of
        // them.
        for(int analysis_window = 0; analysis_window<NFeno; analysis_window++) {

          FENO *pTabFeno = &TabFeno[row][analysis_window];
          if (!pTabFeno->hidden 
              && pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC
              && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC
              && use_as_reference(orbit_file,recordnumber,pTabFeno, xtrack_mode)) {

            // create new spectrum structure if needed.  If the spectrum is already used for another analysis window, we can reuse the existing spectrum.
            if(newref == NULL) {
              newref = malloc(sizeof (struct omi_ref_spectrum));
              allocs++;
              newref->solarZenithAngle = orbit_file->omiSwath->geolocationFields.solarZenithAngle[recordnumber];
              newref->longitude = orbit_file->omiSwath->geolocationFields.longitude[recordnumber];
              newref->latitude = orbit_file->omiSwath->geolocationFields.latitude[recordnumber];
              newref->measurement_number = measurement;
              newref->detector_row = row;
              newref->next = *first;
              *first = newref; // add spectrum to the list of all spectra
              newref->orbit_file = orbit_file;
              newref->spectrum = malloc(orbit_file->nWavel * sizeof(*newref->spectrum));
              newref->errors = malloc(orbit_file->nWavel * sizeof(*newref->errors));
              newref->wavelengths = malloc(orbit_file->nWavel * sizeof(*newref->wavelengths));

              rc = omi_load_spectrum(OMI_SPEC_RAD, orbit_file->sw_id, measurement, row, orbit_file->nWavel, newref->wavelengths, newref->spectrum, newref->errors, NULL);

              if(rc)
                goto end_find_matching_spectra;
            }
            
            struct omi_ref_list *list_item = malloc(sizeof(struct omi_ref_list));
            list_item->reference = newref;
            list_item->next = (*row_references)[analysis_window][row];
            (*row_references)[analysis_window][row] = list_item; // add reference to the list of spectra for this analysis window/row
          }
        }
      }
    }
  }

 end_find_matching_spectra:
  return rc;
}

/* Calculate the automatic reference spectrum by averaging all spectra
   in the list.  The error on the automatic reference (for each pixel)
   is calculated as 1/n_spectra * sqrt(sum (sigma_i)^2)
   */
static void average_spectrum( double *average, double *errors, struct omi_ref_list *spectra, double *wavelength_grid) {
  int nWavel = spectra->reference->orbit_file->nWavel;

  double tempspectrum[nWavel];
  double temperrors[nWavel];
  double derivs[nWavel];

  for(int i=0; i<nWavel; i++) {
    average[i] = 0.;
    errors[i] = 0.;
  }

  int n_spectra = 0;
  for(struct omi_ref_list *cur_spectrum = spectra; cur_spectrum != NULL; cur_spectrum = cur_spectrum->next, n_spectra++) {
    struct omi_ref_spectrum* reference = cur_spectrum->reference;

    // interpolate reference on wavelength_grid
    int rc = SPLINE_Deriv2(reference->wavelengths,reference->spectrum,derivs,nWavel,__func__);
    rc |= SPLINE_Vector(reference->wavelengths,reference->spectrum,derivs,nWavel,wavelength_grid,tempspectrum,nWavel,SPLINE_CUBIC,__func__);
    if (rc) {
      break;
    }

    // interpolate instrumental errors on wavelength_grid
    rc = SPLINE_Vector(reference->wavelengths,reference->errors,NULL,nWavel,wavelength_grid,temperrors,nWavel,SPLINE_LINEAR,__func__);
    if(rc)
      break;

    for(int i=0; i<nWavel; i++) {
      average[i] += tempspectrum[i];
      errors[i] += temperrors[i] * temperrors[i];
    }
  }

  for(int i=0; i<nWavel; i++) {
    average[i] /= n_spectra;
    errors[i] = sqrt(errors[i])/n_spectra; // * sqrt_n_spectra; // random error on the average of n_spectra gaussian variables
  }
}

static RC setup_automatic_reference(ENGINE_CONTEXT *pEngineContext, void *responseHandle)
{
  // keep a NFeno*OMI_TOTAL_ROWS array of matching spectra for every detector row & analysis window
  struct omi_ref_list *(*row_references)[NFeno][OMI_TOTAL_ROWS] = malloc(NFeno * OMI_TOTAL_ROWS * sizeof(struct omi_ref_list*));
  for(int analysis_window = 0; analysis_window<NFeno; analysis_window++) {
    for(int row = 0; row < OMI_TOTAL_ROWS; row++) {
      (*row_references)[analysis_window][row] = NULL;
    }
  }

  // list containing the actual data of the selected spectra
  struct omi_ref_spectrum *ref_candidates = NULL;
  // strings describing the selected spectra for each row/analysis window
  for(int row = 0; row < OMI_TOTAL_ROWS; row++)
    for(int analysis_window = 0; analysis_window < NFeno; analysis_window++) {
      free(TabFeno[row][analysis_window].ref_description);
      TabFeno[row][analysis_window].ref_description = NULL;
    }

  // create the list of all orbit files used for this automatic reference 
  RC rc = read_reference_orbit_files(pEngineContext->fileInfo.fileName);
  if(rc)
    goto end_setup_automatic_reference;
  
  // open each reference orbit file; find & read matching spectra in the file
  for(int i = 0; i < num_reference_orbit_files; i++) {
    OMI_ORBIT_FILE *orbit_file = reference_orbit_files[i];
    rc = OmiOpen(orbit_file,OMI_EarthSwaths[pEngineContext->project.instrumental.omi.spectralType]);
    if (rc)
      goto end_setup_automatic_reference;
    
    // add matching spectra in this orbit file to the lists row_references & ref_candidates
    find_matching_spectra(pEngineContext, orbit_file, row_references, &ref_candidates);

    // relevant data has been copied to ref_candidates, so we can free the swath data, and close the orbit file
    omi_free_swath_data(orbit_file->omiSwath);
    orbit_file->omiSwath = NULL;
    omi_close_orbit_file(reference_orbit_files[i]);
  }

  // take the average of the matching spectra for each detector row & analysis window:
  for(int row = 0; row < OMI_TOTAL_ROWS; row++) {
    if (pEngineContext->project.instrumental.omi.omiTracks[row]) {
      automatic_reference_ok[row] = true; // initialize to true, set it to false if automatic reference fails for one or more analysis windows
      for(int analysis_window = 0; analysis_window < NFeno; analysis_window++) {
        FENO *pTabFeno = &TabFeno[row][analysis_window];
        if(pTabFeno->hidden || !pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC ) {
          continue;
        }

        struct omi_ref_list *reflist = (*row_references)[analysis_window][row];
        
        if(reflist != NULL) {
          average_spectrum(pTabFeno->SrefN, pTabFeno->SrefSigma, reflist, pTabFeno->LambdaK);
          VECTOR_NormalizeVector(pTabFeno->SrefN-1,NDET,&pTabFeno->refNormFactN, __func__);
          // copy SrefN to SrefS...
          memcpy(pTabFeno->SrefS,pTabFeno->SrefN,sizeof(double)* NDET);
          pTabFeno->refNormFactS = pTabFeno->refNormFactN;
          memcpy(pTabFeno->LambdaN,pTabFeno->LambdaK, sizeof(double) * NDET);
          memcpy(pTabFeno->LambdaS,pTabFeno->LambdaK, sizeof(double) * NDET);
	  pTabFeno->ref_description = automatic_reference_info(reflist);
        } else{
          char errormessage[250];
          sprintf( errormessage, "Can not find reference spectra for row %d and analysis window_%s", row, pTabFeno->windowName);
          mediateResponseErrorMessage(__func__, errormessage, WarningEngineError, responseHandle);
          automatic_reference_ok[row] = false;
        }
      }
    }
  }

 end_setup_automatic_reference:
  // free row_references & ref_candidates
  free_row_references(row_references);
  free_ref_candidates(ref_candidates);

  return rc;
}

// Convert number of seconds since 01/01/1993 00:00:00 to date
static void tai_to_ymd(double tai, struct tm *result, int *ms) {
  static struct tm start_tai = { .tm_sec = 0,
                                 .tm_min = 0,
                                 .tm_hour = 0,
                                 .tm_mday = 1,
                                 .tm_mon = 0, // month since jan (-> 0-11)
                                 .tm_year = 93, // year since 1900
                                 .tm_isdst = 0 };

  // get seconds since epoch of 1/1/1993 00:00:00 (GMT)
#ifndef _WIN32
  time_t time_epoch = timegm(&start_tai);
#else
  // get UTC time on MinGW32:
  putenv("TZ=UTC");
  time_t time_epoch = mktime(&start_tai);
#endif

  int seconds = floor(tai); // seconds since 1/1/1993 (GMT)
  time_epoch += seconds;

#ifndef _WIN32
  gmtime_r(&time_epoch, result);
#else
  struct tm *time = gmtime(&time_epoch);
  *result = *time;
#endif

  if (ms != NULL) {
    *ms = (int)(1000*(tai - seconds));
  }
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

static RC OmiGetSwathData(OMI_ORBIT_FILE *pOrbitFile)
{
  // Initializations
  OMI_DATA_FIELDS *pData = &pOrbitFile->omiSwath->dataFields;
  OMI_GEO *pGeo = &pOrbitFile->omiSwath->geolocationFields;
  RC rc=ERROR_ID_NO;

  struct omi_buffer swathdata[] =
    {
      {"MeasurementQualityFlags", pData->measurementQualityFlags},
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
      {"XTrackQualityFlags",pGeo->xtrackQualityFlags}
    };

  int32 start[] = {0,0};
  int32 edge[] =  {pOrbitFile->nMeasurements,pOrbitFile->nXtrack };
  intn swrc;
  for (unsigned int i=0; i<sizeof(swathdata)/sizeof(swathdata[0]); i++) {
    swrc = SWreadfield(pOrbitFile->sw_id, (char *) swathdata[i].buffername, start, NULL, edge, swathdata[i].bufferptr);
    if (swrc == FAIL) {
      rc = ERROR_SetLast("OmiGetSwathData",ERROR_TYPE_FATAL,ERROR_ID_HDFEOS,swathdata[i].buffername,pOrbitFile->omiFileName,"Can not read ", swathdata[i].buffername);
      break;
    }
  }

  // normalize longitudes: should be in the range 0-360
  for (int i=0; i< (pOrbitFile->nMeasurements * pOrbitFile->nXtrack); i++) {
    if(pGeo->longitude[i] < 0.)
      pGeo->longitude[i] += 360.;
  }

  // Return

  return rc;
}

static RC OmiOpen(OMI_ORBIT_FILE *pOrbitFile,const char *swathName)
{
  RC rc = ERROR_ID_NO;

  // Open the file
  int32 swf_id = SWopen(pOrbitFile->omiFileName, DFACC_READ);
  if (swf_id == FAIL) {
    rc = ERROR_SetLast("OmiOpen",ERROR_TYPE_FATAL,ERROR_ID_HDFEOS,"OmiOpen",pOrbitFile->omiFileName,"SWopen");
    goto end_OmiOpen;
  }
  pOrbitFile->swf_id = swf_id;

  // Get a list of all swaths in the file:
  int32 strbufsize = 0;
  int nswath = SWinqswath(pOrbitFile->omiFileName, NULL, &strbufsize);
  if(nswath == FAIL) {
    rc = ERROR_SetLast("OmiOpen", ERROR_TYPE_FATAL, ERROR_ID_HDFEOS, "SWinqswath", pOrbitFile->omiFileName);
    goto end_OmiOpen;
  } else {
    char swathlist[strbufsize+1];
    nswath = SWinqswath(pOrbitFile->omiFileName, swathlist, &strbufsize);

    // Look for requested swath in the list, and extract the complete
    // name e.g. look for "Earth UV-1 Swath" and extract "Earth UV-1
    // Swath (60x159x4)"
    //
    // (the complete name is needed to open the swath with SWattach)
    char *swath_full_name = strstr(swathlist,swathName);
    if (swath_full_name == NULL) {
    rc = ERROR_SetLast("OmiOpen",ERROR_TYPE_FATAL,ERROR_ID_HDFEOS,"OmiOpen",pOrbitFile->omiFileName,"find swath");
    goto end_OmiOpen;
    }
    char *end_name = strpbrk(swath_full_name,",");

    if (end_name != NULL)
      *(end_name) = '\0';
    
    int32 sw_id = SWattach(swf_id, swath_full_name); // attach the swath
    if (sw_id == FAIL) {
      rc = ERROR_SetLast("OmiOpen", ERROR_TYPE_FATAL,ERROR_ID_HDFEOS,"OmiOpen",pOrbitFile->omiFileName,"SWattach");
      goto end_OmiOpen;
    }
    pOrbitFile->sw_id = sw_id;
  
    int32 dims[3];
    int32 rank;
    int32 numbertype;
    char dimlist[520]; // 520 is a safe maximum length, see HDF-EOS ref for SWfieldinfo()
    intn swrc = SWfieldinfo(sw_id, (char *) "RadianceMantissa",&rank,dims,&numbertype , dimlist);
    if(swrc == FAIL) {
      rc=ERROR_SetLast("OmiOpen", ERROR_TYPE_FATAL, ERROR_ID_FILE_EMPTY,pOrbitFile->omiFileName);
      goto end_OmiOpen;
    }

    pOrbitFile->nMeasurements=(long)dims[0];
    pOrbitFile->nXtrack=(long)dims[1];
    pOrbitFile->nWavel=(long)dims[2];
    
    pOrbitFile->specNumber=pOrbitFile->nMeasurements*pOrbitFile->nXtrack;
    if (!pOrbitFile->specNumber)
      rc=ERROR_SetLast("OmiOpen",ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,pOrbitFile->omiFileName);

    // Allocate data
    rc=OMI_AllocateSwath(&pOrbitFile->omiSwath,pOrbitFile->nMeasurements,pOrbitFile->nXtrack);
    if(!rc)
      // Retrieve information on records from Data fields and Geolocation fields
      rc=OmiGetSwathData(pOrbitFile);
  }

 end_OmiOpen:
  return rc;
}

static RC OMI_LoadReference(int spectralType, const char *refFile, OMI_REF **return_ref)
{
  OMI_REF *pRef=&OMI_ref[omiRefFilesN];
  RC rc=ERROR_ID_NO;

  int32 swf_id = 0;
  int32 sw_id = 0;

  swf_id = SWopen((char *)refFile, DFACC_READ); // library header doesn't contain const modifier
  if (swf_id == FAIL) {
    rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_HDFEOS,refFile,"can't open file ","");
    goto end_loadreference;
  }
  sw_id = SWattach(swf_id, (char *) OMI_SunSwaths[spectralType]); // library header doesn't contain const modifier
  if (sw_id  == FAIL) {
    rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_HDFEOS,OMI_SunSwaths[spectralType],"swath not found in file ", refFile);
    goto end_loadreference;
  }

  int32 n_xtrack = SWdiminfo(sw_id, (char *) NXTRACK);
  if (n_xtrack == FAIL) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS, NXTRACK, "can't access dimension in file ", refFile);
    goto end_loadreference;
  }
  int32 n_wavel = SWdiminfo(sw_id, (char *) NWAVEL);
  if (n_wavel == FAIL) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS, NWAVEL, "can't access dimension in file ", refFile);
    goto end_loadreference;
  }

  OMI_AllocateReference(omiRefFilesN,n_xtrack,n_wavel);

  strcpy(pRef->omiRefFileName,refFile);
  pRef->nXtrack=n_xtrack;
  pRef->nWavel=n_wavel;
  
  for (int indexSpectrum=0; indexSpectrum < pRef->nXtrack; indexSpectrum++) {
    rc = omi_load_spectrum(OMI_SPEC_IRRAD, sw_id, 0, indexSpectrum, n_wavel,
			   pRef->omiRefLambda[indexSpectrum],
			   pRef->omiRefSpectrum[indexSpectrum],
			   pRef->omiRefSigma[indexSpectrum],
			   pRef->spectrum.pixelQualityFlags);
    if (rc)
      goto end_loadreference;
  }

  if (!rc) {
    ++omiRefFilesN;
    NDET=pRef->nWavel;
    ANALYSE_swathSize=pRef->nXtrack;
    *return_ref = pRef;
  }

 end_loadreference:

  if(sw_id !=0)
    SWdetach(sw_id);
  if(swf_id !=0)
    SWclose(swf_id);
  
  return rc;
}

/*! read wavelengths, spectrum, and errors into the buffers lambda,
 * spectrum and sigma.  If any of these pointers is NULL, the
 * corresponding data is not read.
 */
static RC omi_load_spectrum(int spec_type, int32 sw_id, int32 measurement, int32 track, int32 n_wavel, double *lambda, double *spectrum, double *sigma, unsigned short *pixelQualityFlags) {
  RC rc = ERROR_ID_NO;

  int16 *mantissa = malloc(n_wavel*sizeof(*mantissa));
  int16 *precisionmantissa = malloc(n_wavel*sizeof(*precisionmantissa));
  int8 *exponent = malloc(n_wavel*sizeof(*exponent));

  // names of the fields in omi hdf files.
  const char *s_mantissa = IRRADIANCE_MANTISSA;
  const char *s_precision_mantissa = IRRADIANCE_PRECISION_MANTISSA;
  const char *s_exponent = IRRADIANCE_EXPONENT;

  if (spec_type == OMI_SPEC_RAD) {
    s_mantissa = RADIANCE_MANTISSA;
    s_precision_mantissa = RADIANCE_PRECISION_MANTISSA;
    s_exponent = RADIANCE_EXPONENT;
  }

  int32 start[] = {measurement, track, 0};
  int32 edge[] = {1,1,0}; // read 1 measurement, 1 detector row
  intn swrc = 0;

  // read wavelengths:
  if(lambda != NULL) 
    {
      // read reference column
      int16 refcol;
      swrc = SWreadfield(sw_id, (char *) REFERENCE_COLUMN, start, NULL, (int32[]) {1}, &refcol);
      // read 5 wavelength coefficients
      edge[2] = OMI_NUM_COEFFICIENTS;
      float32 wavelength_coeff[OMI_NUM_COEFFICIENTS];
      swrc |= SWreadfield(sw_id,(char *) WAVELENGTH_COEFFICIENT, start, NULL, edge, wavelength_coeff);
      
      if (swrc == FAIL) {
        rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS, "SWreadfield");
        goto end_load_spectrum;
      }
      // store wavelength in lambda
      omi_calculate_wavelengths(wavelength_coeff, refcol, n_wavel, lambda);
    }

  // (ir-)radiance mantissae, exponents & pixel quality have dimension (nMeasurement x nXtrack x nWavel)
  edge[2] = n_wavel;

  if(spectrum != NULL || sigma != NULL) {
    swrc |= SWreadfield(sw_id, (char *) s_exponent, start, NULL, edge, exponent);

    if(spectrum != NULL) {
      swrc |= SWreadfield(sw_id, (char *) s_mantissa, start, NULL, edge, mantissa);
      if(!swrc) {
        omi_make_double(mantissa, exponent, n_wavel, spectrum);
        omi_interpolate_errors(mantissa,n_wavel,lambda,spectrum);
      }
    }

    if(sigma != NULL) {
      swrc |= SWreadfield(sw_id, (char *) s_precision_mantissa, start, NULL, edge, precisionmantissa);
      if(!swrc) {
        omi_make_double(precisionmantissa, exponent, n_wavel, sigma);
        omi_interpolate_errors(precisionmantissa,n_wavel,lambda,sigma);
      }
    }
  }

  if(pixelQualityFlags != NULL)
    swrc |= SWreadfield(sw_id, (char *) PIXEL_QUALITY_FLAGS, start, NULL, edge, pixelQualityFlags);

  if(swrc) // error reading either mantissa/precision_mantissa/exponent/qualityflags:
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_HDFEOS, "SWreadfield");

 end_load_spectrum:
  free(mantissa);
  free(precisionmantissa);
  free(exponent);

  return rc;
}

static void omi_calculate_wavelengths(float32 wavelength_coeff[], int16 refcol, int32 n_wavel, double* lambda) {
  int i;
  // OMI wavelengths provided as a degree 4 polynomial
  // evaluate lambda = c_4*x^4 +c_3*x^3 ... + c_0,
  //                 = (((c_4*x + c_3)*x + c_2)*x + c_1)*x + c_0.
  for (i=0; i<n_wavel; i++) {
    double x = (double) i-refcol;
    lambda[i] = 0.;
    int j;
    for (j=OMI_NUM_COEFFICIENTS-1; j>=0 ; j--) {
      lambda[i] = lambda[i]*x + (double)wavelength_coeff[j];
    }
  }
}

static void omi_make_double(int16 mantissa[], int8 exponent[], int32 n_wavel, double* result) {
  int i;
  for (i=0; i<n_wavel; i++) {
    result[i] = (double)mantissa[i] * STD_Pow10((int)exponent[i]);
  }
}

static void omi_interpolate_errors(int16 mantissa[], int32 n_wavel, double wavelengths[], double y[] ){

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

RC OMI_GetReference(int spectralType, const char *refFile, INDEX indexColumn, double *lambda, double *ref, double *refSigma)
{
  
  RC rc=ERROR_ID_NO; 

  OMI_REF *pRef= NULL;

  // Browse existing references
  for (int indexRef=0; indexRef<omiRefFilesN && (pRef == NULL); ++indexRef) {
    if (!strcasecmp(OMI_ref[indexRef].omiRefFileName,refFile))
      pRef = &OMI_ref[indexRef];
  }
  if (pRef == NULL) {
    // if not found, load the reference now
    rc = OMI_LoadReference(spectralType, refFile, &pRef);
    if (rc != ERROR_ID_NO)
      pRef = NULL;
  }

  if ((pRef != NULL) && (indexColumn>=0) && (indexColumn< pRef->nXtrack)) {
    memcpy(lambda,pRef->omiRefLambda[indexColumn],sizeof(double)*pRef->nWavel);
    memcpy(ref,pRef->omiRefSpectrum[indexColumn],sizeof(double)*pRef->nWavel);
    memcpy(refSigma,pRef->omiRefSigma[indexColumn],sizeof(double)*pRef->nWavel);
  } else {
    rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_OMI_REF,__func__);
  }

  return rc;
}

RC OMI_load_analysis(ENGINE_CONTEXT *pEngineContext, void *responseHandle) {
  RC rc = ERROR_ID_NO;
   
  // if we need a new automatic reference, generate it
  if(pEngineContext->analysisRef.refAuto && !valid_reference_file(current_orbit_file.omiFileName)) {
    rc = setup_automatic_reference(pEngineContext, responseHandle);
    if(rc) {
      goto end_omi_load_analysis;
    }

    for(int i=0; i<ANALYSE_swathSize; i++) {
      if (pEngineContext->project.instrumental.omi.omiTracks[i]) {
        // fit wavelength shift between calibrated solar irradiance
        // and automatic reference spectrum and apply this shift to
        // absorption crosssections
        rc = ANALYSE_AlignReference(pEngineContext,2,pEngineContext->project.spectra.displayDataFlag,responseHandle,i);
      }
    }

  }

 end_omi_load_analysis:
  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      OMI_Set
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve information on useful data sets from the HDF file.
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
#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_FILE);
#endif

  // Initializations
  pEngineContext->recordNumber=0;
  omiSwathOld=ITEM_NONE;
  RC rc=ERROR_ID_NO;

  // Release old buffers and close file (if open)
  omi_free_swath_data(current_orbit_file.omiSwath);
  omi_close_orbit_file(&current_orbit_file);

  current_orbit_file.omiFileName = malloc(strlen(pEngineContext->fileInfo.fileName)+1);
  strcpy(current_orbit_file.omiFileName,pEngineContext->fileInfo.fileName);
  current_orbit_file.specNumber=0;

  // Open the file
  if (!(rc=OmiOpen(&current_orbit_file,OMI_EarthSwaths[pEngineContext->project.instrumental.omi.spectralType])))
    {
      pEngineContext->recordNumber=current_orbit_file.specNumber;
      pEngineContext->recordInfo.omi.nMeasurements=current_orbit_file.nMeasurements;
      pEngineContext->recordInfo.omi.nXtrack=current_orbit_file.nXtrack;

      omiTotalRecordNumber+=current_orbit_file.nMeasurements;

      NDET=current_orbit_file.nWavel;
    }
  else
    {
      omi_free_swath_data(current_orbit_file.omiSwath);
      omi_close_orbit_file(&current_orbit_file);
    }

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

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

RC OMI_Read(ENGINE_CONTEXT *pEngineContext,int recordNo)
{
  // Initializations
  
  OMI_ORBIT_FILE *pOrbitFile = &current_orbit_file; // pointer to the current orbit
  OMI_GEO *pGeo= &pOrbitFile->omiSwath->geolocationFields;

  double *spectrum=pEngineContext->buffers.spectrum;
  double *sigma=pEngineContext->buffers.sigmaSpec;
  double *lambda=pEngineContext->buffers.lambda;
  RECORD_INFO *pRecord=&pEngineContext->recordInfo;
  RC rc=ERROR_ID_NO;

  // Goto the requested record

  if (!pOrbitFile->specNumber)
    rc=ERROR_ID_FILE_EMPTY;
  else if ((recordNo<=0) || (recordNo>pOrbitFile->specNumber))
    rc=ERROR_ID_FILE_END;
  else
    {
      for (int i=0;i<NDET;i++)
	spectrum[i]=sigma[i]=(double)0.;
      
      int indexMeasurement=floor((recordNo-1)/pOrbitFile->nXtrack);      // index of the swath
      int indexSpectrum=(recordNo-1)%pOrbitFile->nXtrack;          // index of the spectrum in the swath

      if (!pEngineContext->project.instrumental.omi.omiTracks[indexSpectrum]) {
	rc=ERROR_ID_FILE_RECORD;
      } else if (!(rc=
                   omi_load_spectrum(OMI_SPEC_RAD,
                                     pOrbitFile->sw_id,
                                     indexMeasurement,
                                     indexSpectrum,
                                     pOrbitFile->nWavel,
                                     lambda,spectrum,sigma,
                                     pRecord->omi.omiPixelQF)))
	{
	  if ((THRD_id==THREAD_TYPE_ANALYSIS) && omiRefFilesN)
	    {
	      if (omiSwathOld!=indexMeasurement)
		{
		  KURUCZ_indexLine=1;
		  omiSwathOld=indexMeasurement;
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
          
	  pRecord->omi.omiMeasurementIndex=indexMeasurement+1;                                  // index of the current measurement
	  pRecord->omi.omiRowIndex=indexSpectrum+1;                                 // row of the current spectrum in the current measurement
          pRecord->omi.omiXtrackQF = pGeo->xtrackQualityFlags[recordNo-1];
          
          struct tm time_record;
	  tai_to_ymd((double)pGeo->time[indexMeasurement],&time_record, &OMI_ms);

          SHORT_DATE* pDate = &pRecord->present_day;
          struct time *pTime = &pRecord->present_time;

          pTime->ti_hour = (char)(time_record.tm_hour);
          pTime->ti_min = (char)(time_record.tm_min);
          pTime->ti_sec = (char)(time_record.tm_sec);
          
          pDate->da_year = (short)(time_record.tm_year + 1900);
          pDate->da_mon = (char)(time_record.tm_mon + 1);
          pDate->da_day = (char)(time_record.tm_mday);
          
	  pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);
	}
    }
  
  // Return

  return rc;
}

void OMI_get_orbit_date(int *year, int *month, int *day) {
  struct tm start_time;
  tai_to_ymd(current_orbit_file.omiSwath->geolocationFields.time[0], &start_time, NULL);
  *year = start_time.tm_year + 1900;
  *month = start_time.tm_mon + 1;
  *day = start_time.tm_mday;
}

void OMI_ReleaseBuffers(void) {

  for(int i=0; i< num_reference_orbit_files; i++) {

    omi_destroy_orbit_file(reference_orbit_files[i]);
    reference_orbit_files[i] = NULL;
  }

  omi_close_orbit_file(&current_orbit_file);

  num_reference_orbit_files = 0;

  omiRefFilesN=0; // the total number of files to browse in one shot
  omiTotalRecordNumber=0;
  omiSwathOld=ITEM_NONE;

}

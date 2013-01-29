
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
//  Module purpose    :  GOME2 interface
//  Name of module    :  Gome2Read.C
//  Program Language  :  Borland C++ 5.0 for Windows 95/NT
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

// ====================
// CONSTANTS DEFINITION
// ====================                                                         // these definitions come from the routines written by Stefan Noel and Andreas Richter

#define MAX_GOME2_FILES 500                                                     // usually, maximum of files per day is 480

#define NCHANNEL 6                                                              // the number of channels
#define NPIXEL_MAX 1024                                                         // the size of the detector
#define NBAND 10                                                                // number of virtual bands
#define EARTHSHINE 6                                                            // MDR subclass for earthshine measurements
#define N_TINT_MAX 6                                                            // max. no. of unique int.times
#define N_SCAN_MAX 32                                                           // maximum number of scans

#define SCAN_FORWARD 4.5                                                        // duration of a forward scan
#define SCAN_BACKWARD 1.5                                                       // duration of a backward scan
#define SCAN_DURATION (SCAN_FORWARD+SCAN_BACKWARD)

// =====================
// STRUCTURES DEFINITION
// =====================

#pragma pack(1)

typedef struct _gome2MdrInfo
 {
  uint16_t num_recs[NBAND];                                                     // number of records for the requested band
  uint16_t rec_length[NBAND];                                                   // record length for the requested band
 	INDEX    indexMDR;                                                            // index of the MDR in the file
 	double   startTime;                                                           // starting time of the MDR

 	// !!! version < 12 : NCHANNEL

  uint8_t  int_index[NCHANNEL];                                                 // index of the integration time (version <= 12
  double   unique_int[NBAND];                                                   // integration time
  double   scanner_angle[N_TINT_MAX][N_SCAN_MAX];                               // scanner angles
  double   corner_lat[N_TINT_MAX][4][N_SCAN_MAX];                               // latitudes at the four corners of the pixels
  double   corner_lon[N_TINT_MAX][4][N_SCAN_MAX];                               // longitudes at the four corners of the pixels
  double   centre_lat[N_TINT_MAX][N_SCAN_MAX];                                  // latitude at the centre of the pixels
  double   centre_lon[N_TINT_MAX][N_SCAN_MAX];                                  // longitude at the centre of the pixels
  double   sza[N_TINT_MAX][3][N_SCAN_MAX];                                      // solar zenith angles
  double   saa[N_TINT_MAX][3][N_SCAN_MAX]; 	                                    // solar azimuth angles
  double   lza[N_TINT_MAX][3][N_SCAN_MAX];                                      // line of sight zenith angles
  double   laa[N_TINT_MAX][3][N_SCAN_MAX];                                      // line of sight azimuth angles

 	//     version == 12 : NBAND

  double   integration_times[NBAND];

  double   scanner_angle12[N_SCAN_MAX];                                         // scanner angles
  double   corner_lat12[N_SCAN_MAX][4];                                         // latitudes at the four corners of the pixels
  double   corner_lon12[N_SCAN_MAX][4];                                         // longitudes at the four corners of the pixels
  double   centre_lat12[N_SCAN_MAX];                                            // latitude at the centre of the pixels
  double   centre_lon12[N_SCAN_MAX];                                            // longitude at the centre of the pixels
  double   sza12[N_SCAN_MAX][3];                                                // solar zenith angles
  double   saa12[N_SCAN_MAX][3]; 	                                              // solar azimuth angles
  double   lza12[N_SCAN_MAX][3];                                                // line of sight zenith angles
  double   laa12[N_SCAN_MAX][3];                                                // line of sight azimuth angles

  uint16_t geo_rec_length[NBAND];
  uint8_t  cloudFitMode[N_SCAN_MAX];
  double   cloudTopPressure[N_SCAN_MAX];
  double   cloudFraction[N_SCAN_MAX];
  uint8_t  scanDirection;
  uint8_t  saaFlag[N_SCAN_MAX];
  uint8_t  sunglintDangerFlag[N_SCAN_MAX];
  uint8_t  sunglintHighDangerFlag[N_SCAN_MAX];
  uint8_t  rainbowFlag[N_SCAN_MAX];
 }
GOME2_MDR;

typedef struct _gome2Info
 {
 	uint8_t  channelIndex;
  uint16_t startPixel;
  uint16_t no_of_pixels;
  uint16_t orbitStart;
  uint16_t orbitEnd;
  uint32_t total_viadr;
  uint32_t total_mdr;

  int      total_nadir_mdr;
  int      total_nadir_obs;

  double start_lambda;
  double end_lambda;
  GOME2_MDR *mdr;
 }
GOME2_INFO;

typedef struct _GOME2OrbitFiles                                                 // description of an orbit
 {
 	DoasCh               gome2FileName[MAX_STR_LEN+1];                             // the name of the file with a part of the orbit
 	GOME2_INFO          gome2Info;                                                // all internal information about the PDS file like data offsets etc.
  double             *gome2SunRef,*gome2SunWve;                                 // the sun reference spectrum and calibration
  INDEX              *gome2LatIndex,*gome2LonIndex,*gome2SzaIndex;              // indexes of records sorted resp. by latitude, by longitude and by SZA
  SATELLITE_GEOLOC   *gome2Geolocations;                                        // geolocations
  INT                 specNumber;
  coda_ProductFile   *gome2Pf;                                                  // GOME2 product file pointer
  coda_Cursor         gome2Cursor;                                              // GOME2 file cursor
  coda_Cursor         gome2CursorMDR;                                           // GOME2 file cursor on MDR
  int                 version;
  INT                 rc;
 }
GOME2_ORBIT_FILE;

typedef struct _gome2RefSelection
 {
 	INDEX  indexFile;
  INDEX  indexRecord;
  double sza;
  double latitude;
  double longitude;
  double cloudFraction;
  double szaDist;
  double latDist;
  double lonDist;
 }
GOME2_REF;

// ================
// GLOBAL VARIABLES
// ================

INT GOME2_ms=0;

// ================
// STATIC VARIABLES
// ================

char *const gome2BandName[NBAND] =
 {
  "BAND_1A",
  "BAND_1B",
  "BAND_2A",
  "BAND_2B",
  "BAND_3",
  "BAND_4",
  "BAND_PP",
  "BAND_PS",
  "BAND_SWPP",
  "BAND_SWPS"
 };

static GOME2_ORBIT_FILE gome2OrbitFiles[MAX_GOME2_FILES];                       // list of files per day
static int gome2OrbitFilesN=0;                                                  // the total number of files to browse in one shot
static INDEX gome2CurrentFileIndex=ITEM_NONE;                                   // index of the current file in the list
int GOME2_beatLoaded=0;
static INT gome2LoadReferenceFlag=0;
static INT gome2TotalRecordNumber=0;

// =========
// FUNCTIONS
// =========

void Gome2Sort(GOME2_ORBIT_FILE *pOrbitFile,INDEX indexRecord,int flag,int listSize);

// =====================================
// COMPATIBILITY WITH OLD BEAT FUNCTIONS
// =====================================

int beat_get_utc_string_from_time(double time, char *utc_string)
{
    int DAY, MONTH, YEAR, HOUR, MINUTE, SECOND, MUSEC;
    const char *monthname[12] = {
        "JAN",
        "FEB",
        "MAR",
        "APR",
        "MAY",
        "JUN",
        "JUL",
        "AUG",
        "SEP",
        "OCT",
        "NOV",
        "DEC"
    };

    if (utc_string == NULL)
    {
        coda_set_error(CODA_ERROR_INVALID_ARGUMENT, "utc_string argument is NULL (%s:%u)", __FILE__, __LINE__);
        return -1;
    }

    if (coda_double_to_datetime(time, &YEAR, &MONTH, &DAY, &HOUR, &MINUTE, &SECOND, &MUSEC) != 0)
    {
        return -1;
    }
    if (YEAR < 0 || YEAR > 9999)
    {
        coda_set_error(CODA_ERROR_INVALID_DATETIME, "the year can not be represented using a positive four digit "
                       "number");
        return -1;
    }
    sprintf(utc_string, "%02d-%3s-%04d %02d:%02d:%02d.%06u", DAY, monthname[MONTH - 1], YEAR, HOUR, MINUTE, SECOND,
            MUSEC);

    return 0;
}

int beat_cursor_read_geolocation_double_split(const coda_Cursor *cursor, double *dst_latitude,
                                                          double *dst_longitude)
{
    coda_Cursor pair_cursor;

    pair_cursor = *(coda_Cursor *)cursor;
    if (coda_cursor_goto_record_field_by_index(&pair_cursor, 0) != 0)
    {
        // beat_errno = coda_errno_to_beat_errno(coda_errno);
        return -1;
    }
    if (coda_cursor_read_double(&pair_cursor, dst_latitude) != 0)
    {
        // beat_errno = coda_errno_to_beat_errno(coda_errno);
        return -1;
    }
    if (coda_cursor_goto_next_record_field(&pair_cursor) != 0)
    {
        // beat_errno = coda_errno_to_beat_errno(coda_errno);
        return -1;
    }
    if (coda_cursor_read_double(&pair_cursor, dst_longitude) != 0)
    {
        // beat_errno = coda_errno_to_beat_errno(coda_errno);
        return -1;
    }

    return 0;
}

int beat_cursor_read_geolocation_double_split_array(const coda_Cursor *cursor, double *dst_latitude,
                                                                double *dst_longitude,
                                                                coda_array_ordering array_ordering)
{
    long dim[CODA_MAX_NUM_DIMS];
    long num_elements;
    int num_dims;
    long i;

    if (coda_cursor_get_num_elements((coda_Cursor *)cursor, &num_elements) != 0)
    {
        // beat_errno = = coda_errno_to_beat_errno(coda_errno);
        return -1;
    }
    if (coda_cursor_get_array_dim((coda_Cursor *)cursor, &num_dims, dim) != 0)
    {
        // beat_errno = = coda_errno_to_beat_errno(coda_errno);
        return -1;
    }

    if (num_elements > 0)
    {
        if (coda_cursor_goto_first_array_element((coda_Cursor *)cursor) != 0)
        {
            // beat_errno = = coda_errno_to_beat_errno(coda_errno);
            return -1;
        }

        for (i = 0; i < num_elements; i++)
        {
            long index = i;

            if (array_ordering == coda_array_ordering_fortran)
            {
                index = coda_c_index_to_fortran_index(num_dims, dim, i);
            }

            if (beat_cursor_read_geolocation_double_split(cursor, &dst_latitude[index], &dst_longitude[index]) != 0)
            {
                return -1;
            }
            if (i < num_elements - 1)
            {
                if (coda_cursor_goto_next_array_element((coda_Cursor *)cursor) != 0)
                {
                    // beat_errno = = coda_errno_to_beat_errno(coda_errno);
                    return -1;
                }
            }
        }
        coda_cursor_goto_parent((coda_Cursor *)cursor);
    }
    return 0;
}

// =========
// UTILITIES
// =========

/* read UTC string from product; returns both double value (= sec since 1 Jan 2000) and string */
int read_utc_string(coda_Cursor* cursor, char * utc_string, double * data)
{
    int status;

    status = coda_cursor_read_double(cursor, data);
    if (status == 0) {
	if (coda_isNaN(*data))
	{
	    strcpy(utc_string,"                           ");
	}
	else
	{
	    status = beat_get_utc_string_from_time(*data, utc_string);
	}
    }
    return status;
}

// -----------------------------------------------------------------------------
// FUNCTION      Gome2GotoOBS
// -----------------------------------------------------------------------------
// PURPOSE       Move cursor to the requested radiance record
//
// INPUT         indexBand      index of the selected band
//               indexMDR       index of the requested MDR
//               indexObs       index of the radiance record in the MDR
// -----------------------------------------------------------------------------

void Gome2GotoOBS(GOME2_ORBIT_FILE *pOrbitFile,INDEX indexBand,INDEX indexMDR,INDEX indexObs)
 {
 	// Declarations

 	INT obsToBypass;

 	// Initializations

  obsToBypass=indexObs*pOrbitFile->gome2Info.mdr[indexMDR].rec_length[indexBand];

  // Goto the current MDR

  coda_cursor_goto_root(&pOrbitFile->gome2Cursor);
  coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"MDR");
  coda_cursor_goto_array_element_by_index(&pOrbitFile->gome2Cursor,pOrbitFile->gome2Info.mdr[indexMDR].indexMDR);

 	coda_cursor_goto_available_union_field(&pOrbitFile->gome2Cursor);                         // MDR.GOME2_MDR_L1B_EARTHSHINE_V1
 	coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,gome2BandName[indexBand]); // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.band(indexBand)

 	coda_cursor_goto_array_element_by_index(&pOrbitFile->gome2Cursor,obsToBypass);
 }

// -----------------------------------------------------------------------------
// FUNCTION      Gome2GetMDRIndex
// -----------------------------------------------------------------------------
// PURPOSE       Given a record number, return the MDR index.
//
// INPUT         indexBand      index of the selected band
//               recordNo       the requested record number;
//
// OUTPUT        pObs           the total number of observations covered by
//                              previous MDR
//
// RETURN        the index of the MDR
// -----------------------------------------------------------------------------

INDEX Gome2GetMDRIndex(GOME2_ORBIT_FILE *pOrbitFile,INDEX indexBand,int recordNo,INT *pObs)
 {
  // Declarations

  INDEX indexMDR;                                                               // browse MDR
  INT sumObs;                                                                   // accumulate the number of observations in the different states

  // Search for the state

  for (indexMDR=sumObs=0;indexMDR<pOrbitFile->gome2Info.total_nadir_mdr;sumObs+=pOrbitFile->gome2Info.mdr[indexMDR].num_recs[indexBand],indexMDR++)
   if (sumObs+pOrbitFile->gome2Info.mdr[indexMDR].num_recs[indexBand]>recordNo)
    break;

  // Return

  *pObs=sumObs;

  return indexMDR; // pOrbitFile->gome2Info.mdr[indexMDR].indexMDR;
 }

// ===============
// GOME2 FUNCTIONS
// ===============

// -----------------------------------------------------------------------------
// FUNCTION      Gome2Open
// -----------------------------------------------------------------------------
// PURPOSE       Open the file name and check if it is really a GOME2 one
//
// INPUT         fileName     the name of the current GOME2 orbit file
//
// OUTPUT        productFile  pointer to the product file
//
// RETURN        ERROR_ID_BEAT if the open failed
//               ERROR_ID_NO  otherwise.
// -----------------------------------------------------------------------------

int Gome2Open(coda_ProductFile **productFile, char *fileName,int *version)
 {
  // Declarations

  char *productClass;                                                           // product class (EPS is expected)
  char *productType;                                                            // product type (GOME_xxx_1B is expected)
  RC    rc;                                                                     // return code

  // Open the file

  rc=coda_open(fileName,productFile);          // &*productFile

  if (rc!=0) // && (coda_errno==CODA_ERROR_FILE_OPEN))
   {
    /* maybe not enough memory space to map the file in memory =>
     * temporarily disable memory mapping of files and try again
     */
    coda_set_option_use_mmap(0);
    rc=coda_open(fileName,productFile);        // &*productFile
    coda_set_option_use_mmap(1);
   }

  if (rc!=0)
   rc=ERROR_SetLast("Gome2Init",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"coda_open",fileName,""); //coda_errno_to_string(coda_errno));
  else
   {
   	// Retrieve the product class and type

    coda_get_product_class((const coda_ProductFile *)*productFile,(const char **)&productClass);
    coda_get_product_type((const coda_ProductFile *)*productFile,(const char **)&productType);
    coda_get_product_version((const coda_ProductFile *)*productFile, version);

    if (strcmp(productClass,"EPS") || strcmp(productType,"GOME_xxx_1B"))
     rc=ERROR_SetLast("Gome2Init",ERROR_TYPE_WARNING,ERROR_ID_BEAT,"coda_get_product_class or coda_get_product_type",fileName,"Not a GOME2 Level-1B file");
   }

  // Return

  return rc;
 }

RC Gome2ReadSunRef(GOME2_ORBIT_FILE *pOrbitFile,INDEX bandIndex)
 {
 	// Declarations

  double gome2SunWve[NCHANNEL][NPIXEL_MAX];                                     // wavelength
  double gome2SunRef[NCHANNEL][NPIXEL_MAX];                                     // irradiance
  INDEX  channel;                                                               // channel index
  INDEX  i;                                                                     // browse pixels
  RC     rc;                                                                    // return code

  // Initializations

  channel=(INDEX)pOrbitFile->gome2Info.channelIndex;

  rc=ERROR_ID_NO;

  // Allocate buffers

  if (((pOrbitFile->gome2SunWve=(double *)MEMORY_AllocDVector("Gome2ReadSunRef","gome2SunWve",0,NDET))==NULL) ||
      ((pOrbitFile->gome2SunRef=(double *)MEMORY_AllocDVector("Gome2ReadSunRef","gome2SunRef",0,NDET))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    coda_cursor_goto_root(&pOrbitFile->gome2Cursor);
    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"VIADR_SMR");            // VIADR_SMR (Variable Internal Auxiliary Data Record - Sun Mean Reference)
    coda_cursor_goto_first_array_element(&pOrbitFile->gome2Cursor);
    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"LAMBDA_SMR");           // VIADR_SMR.LAMBDA_SMR

    coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&gome2SunWve[0][0],coda_array_ordering_c);

    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // VIADR_SMR
    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"SMR");                  // VIADR_SMR.LAMBDA_SMR

    coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&gome2SunRef[0][0],coda_array_ordering_c);

    for (i=0;i<NDET;i++)
     {
      pOrbitFile->gome2SunWve[i]=gome2SunWve[channel][i];
      pOrbitFile->gome2SunRef[i]=gome2SunRef[channel][i];
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      Gome2ReadOrbitInfo
// -----------------------------------------------------------------------------
// PURPOSE       Read general information on the current orbit
//
// INPUT         productFile  pointer to the current product file
//               bandIndex    the user selected band
//
// OUTPUT        pGome2Info   information on the current orbit
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO    otherwise.
// -----------------------------------------------------------------------------

RC Gome2ReadOrbitInfo(GOME2_ORBIT_FILE *pOrbitFile,int bandIndex)
 {
 	// Declarations

  int status;

  uint8_t  channel_index[NBAND];
  uint16_t startPixel[NBAND];
  uint16_t no_of_pixels[NBAND];

  double start_lambda[NBAND];
  double end_lambda[NBAND];
  GOME2_INFO *pGome2Info;

  RC rc;

  // Initializations

  pGome2Info=&pOrbitFile->gome2Info;
  coda_cursor_goto_root(&pOrbitFile->gome2Cursor);
  rc=ERROR_ID_NO;

  // Retrieve the MPHR

  status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"MPHR");          // MPHR

  status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"ORBIT_START");   // MPHR.orbitStart
  coda_cursor_read_uint16 (&pOrbitFile->gome2Cursor, &pGome2Info->orbitStart);
  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"ORBIT_END");     // MPHR.ORBIT_END
  coda_cursor_read_uint16 (&pOrbitFile->gome2Cursor, &pGome2Info->orbitEnd);
  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"TOTAL_VIADR");   // MPHR.TOTAL_VIADR (Variable Internal Auxiliary Data Record)
  status = coda_cursor_read_uint32 (&pOrbitFile->gome2Cursor, &pGome2Info->total_viadr);
  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"TOTAL_MDR");     // MPHR.TOTAL_MDR (Measurement Data Record)
  status = coda_cursor_read_uint32 (&pOrbitFile->gome2Cursor, &pGome2Info->total_mdr);
  coda_cursor_goto_root(&pOrbitFile->gome2Cursor);

  // Retrieve GIADR_Bands

  status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"GIADR_Bands");

  coda_cursor_goto_array_element_by_index(&pOrbitFile->gome2Cursor,0);          // !!! beat 6.7.0

  status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"CHANNEL_NUMBER");
  status = coda_cursor_read_uint8_array (&pOrbitFile->gome2Cursor, channel_index, coda_array_ordering_c);
  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"START_PIXEL");
  status = coda_cursor_read_uint16_array (&pOrbitFile->gome2Cursor, startPixel, coda_array_ordering_c);
  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"NUMBER_OF_PIXELS");
  status = coda_cursor_read_uint16_array (&pOrbitFile->gome2Cursor, no_of_pixels, coda_array_ordering_c);
  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"START_LAMBDA");
  status = coda_cursor_read_double_array (&pOrbitFile->gome2Cursor, start_lambda, coda_array_ordering_c);
  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  status = coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"END_LAMBDA");
  status = coda_cursor_read_double_array (&pOrbitFile->gome2Cursor, end_lambda, coda_array_ordering_c);
  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);  // !!! beat 6.7.0

  // Output

  pGome2Info->channelIndex=channel_index[bandIndex]-1;
  pGome2Info->startPixel = startPixel[bandIndex];
  pGome2Info->no_of_pixels = no_of_pixels[bandIndex];
  pGome2Info->start_lambda = start_lambda[bandIndex];
  pGome2Info->end_lambda = end_lambda[bandIndex];

  coda_cursor_goto_root(&pOrbitFile->gome2Cursor);

  // Buffer allocation error

  if ((pGome2Info->total_mdr<=0) ||
     ((pGome2Info->mdr=(GOME2_MDR *)MEMORY_AllocBuffer("Gome2ReadOrbitInfo","MDR",pGome2Info->total_mdr,sizeof(GOME2_MDR),0,MEMORY_TYPE_STRUCT))==NULL))

   rc=ERROR_ID_ALLOC;

  // Return

  return rc;
 }

int Gome2ReadMDRInfo(GOME2_ORBIT_FILE *pOrbitFile,GOME2_MDR *pMdr,int indexBand)
 {
 	// Declarations

  uint8_t subclass,observationMode;
  char start_time[40];
  double utc_start_double;
  int indexActual,i;
  char geoEarthActualString[25];

  // Generic record header

  coda_cursor_goto_available_union_field(&pOrbitFile->gome2Cursor);
  coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"RECORD_HEADER");          // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.RECORD_HEADER

  // Subclass (determined by the instrument group - earthshine is expected)

  coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"RECORD_SUBCLASS");        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.RECORD_HEADER.RECORD_SUBCLASS
  coda_cursor_read_uint8(&pOrbitFile->gome2Cursor,&subclass);
  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.RECORD_HEADER

  // Start time of the record

  coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"RECORD_START_TIME");      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.RECORD_HEADER.RECORD_START_TIME
  read_utc_string(&pOrbitFile->gome2Cursor,start_time,&utc_start_double);
  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.RECORD_HEADER

  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1

  if (subclass==EARTHSHINE)
   {
    // Observation mode (NADIR is expected)

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"OBSERVATION_MODE");       // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.OBSERVATION_MODE
    coda_cursor_read_uint8(&pOrbitFile->gome2Cursor,&observationMode);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1

    // number of records in each band

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"NUM_RECS");               // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.NUM_RECS
    coda_cursor_read_uint16_array(&pOrbitFile->gome2Cursor,pMdr->num_recs,coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1

    // length of records in each band

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"REC_LENGTH");             // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.REC_LENGTH
    coda_cursor_read_uint16_array(&pOrbitFile->gome2Cursor,pMdr->rec_length,coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1

    // flags

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"PCD_BASIC");              // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC

    if (pOrbitFile->version<=11)
     {
      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_SAA");                  // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_SAA
      coda_cursor_read_uint8(&pOrbitFile->gome2Cursor,&pMdr->saaFlag[0]);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_SUNGLINT");          // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_SUNGLINT
      coda_cursor_read_uint8(&pOrbitFile->gome2Cursor,&pMdr->sunglintDangerFlag[0]);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_RAINBOW");          // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_RAINBOW
      coda_cursor_read_uint8(&pOrbitFile->gome2Cursor,&pMdr->rainbowFlag[0]);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

      pMdr->sunglintHighDangerFlag[0]=0;

      memset(&pMdr->saaFlag[1],pMdr->saaFlag[0],N_SCAN_MAX-1);
      memset(&pMdr->sunglintDangerFlag[1],pMdr->sunglintDangerFlag[0],N_SCAN_MAX-1);
      memset(&pMdr->sunglintHighDangerFlag[1],pMdr->sunglintHighDangerFlag[0],N_SCAN_MAX-1);
      memset(&pMdr->rainbowFlag[1],pMdr->rainbowFlag[0],N_SCAN_MAX-1);
     }
    else
     {
      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_SAA");                  // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_SAA
      coda_cursor_read_uint8_array(&pOrbitFile->gome2Cursor,pMdr->saaFlag,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_SUNGLINT_RISK");          // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_SUNGLINT_RISK
      coda_cursor_read_uint8_array(&pOrbitFile->gome2Cursor,pMdr->sunglintDangerFlag,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_SUNGLINT_HIGH_RISK");   // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_SUNGLINT_HIGH_RISK
      coda_cursor_read_uint8_array(&pOrbitFile->gome2Cursor,pMdr->sunglintHighDangerFlag,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"F_RAINBOW");              // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC.F_RAINBOW
      coda_cursor_read_uint8_array(&pOrbitFile->gome2Cursor,pMdr->rainbowFlag,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.PCD_BASIC
     }

    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1

    // Unique integration times in the scan

    if (pOrbitFile->version<=11)
     {
     	pMdr->scanDirection=255;                                                                  // Not defined

      // Additional geolocation record for the actual integration time of the earthshine measurements

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "GEO_EARTH_ACTUAL");      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "UNIQUE_INT");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.UNIQUE_INT
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,pMdr->unique_int,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // Unique int. time index for each channel

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "INT_INDEX");             // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.INT_INDEX
      coda_cursor_read_uint8_array(&pOrbitFile->gome2Cursor,pMdr->int_index,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // actual scanner angles

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SCANNER_ANGLE_ACTUAL");  // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SCANNER_ANGLE_ACTUAL
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->scanner_angle[0][0],coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // 4 corner coordinates @ points ABCD

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"CORNER_ACTUAL");          // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.CORNER_ACTUAL

      beat_cursor_read_geolocation_double_split_array(&pOrbitFile->gome2Cursor,
    	 	                                              &pMdr->corner_lat[0][0][0],
    	 	                                              &pMdr->corner_lon[0][0][0],
    	 	                                               coda_array_ordering_c);

      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // centre coordinate (point F)

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "CENTRE_ACTUAL");         // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.CENTRE_ACTUAL

      beat_cursor_read_geolocation_double_split_array(&pOrbitFile->gome2Cursor,
                                                		    &pMdr->centre_lat[0][0],
                                                		    &pMdr->centre_lon[0][0],
                                                		     coda_array_ordering_c);

      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // 3 SZAs @ points EFG

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"SOLAR_ZENITH_ACTUAL");    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_ZENITH_ACTUAL
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->sza[0][0][0],coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // 3 Solar azimuth angles @ points EFG

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SOLAR_AZIMUTH_ACTUAL");  // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_AZIMUTH_ACTUAL
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->saa[0][0][0],coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // 3 SZAs @ points EFG

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"SAT_ZENITH_ACTUAL");      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_ZENITH_ACTUAL
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->lza[0][0][0],coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      // 3 Solar azimuth angles @ points EFG

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SAT_AZIMUTH_ACTUAL");  // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_AZIMUTH_ACTUAL
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->laa[0][0][0],coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1
     }
    else
     {
      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "UNIQUE_INT");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V5.UNIQUE_INT
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,pMdr->unique_int,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V5

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "INTEGRATION_TIMES");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V5.INTEGRATION_TIMES
      coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,pMdr->integration_times,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "GEO_REC_LENGTH");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V5.GEO_REC_LENGTH
      coda_cursor_read_uint16_array(&pOrbitFile->gome2Cursor,pMdr->geo_rec_length,coda_array_ordering_c);
      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);

      for (indexActual=0;indexActual<NBAND;indexActual++)
       if (fabs(pMdr->unique_int[indexActual]-pMdr->integration_times[indexBand])<(double)1.e-6)
        break;

      sprintf(geoEarthActualString,"GEO_EARTH_ACTUAL_%d",indexActual+1);
      coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,geoEarthActualString);            // MDR.GOME2_MDR_L1B_EARTHSHINE_V5.GEO_EARTH_ACTUAL

      for (i=0;i<pMdr->geo_rec_length[indexActual];i++)
       {
       	coda_cursor_goto_array_element_by_index(&pOrbitFile->gome2Cursor,i);

        // actual scanner angles

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SCAN_DIRECTION");        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SCAN_DIRECTION
        coda_cursor_read_uint8(&pOrbitFile->gome2Cursor,&pMdr->scanDirection);
        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // actual scanner angles

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SCANNER_ANGLE_ACTUAL");  // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SCANNER_ANGLE_ACTUAL
        coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->scanner_angle12[i],coda_array_ordering_c);
        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // 4 corner coordinates @ points ABCD

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"CORNER_ACTUAL");          // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.CORNER_ACTUAL

        beat_cursor_read_geolocation_double_split_array(&pOrbitFile->gome2Cursor,
    	   	                                              &pMdr->corner_lat12[i][0],
    	   	                                              &pMdr->corner_lon12[i][0],
    	   	                                               coda_array_ordering_c);

        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // centre coordinate (point F)

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "CENTRE_ACTUAL");         // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.CENTRE_ACTUAL

        beat_cursor_read_geolocation_double_split(&pOrbitFile->gome2Cursor,
                                                  &pMdr->centre_lat12[i],
                                                  &pMdr->centre_lon12[i]);

        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // 3 SZAs @ points EFG

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"SOLAR_ZENITH_ACTUAL");    // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_ZENITH_ACTUAL
        coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->sza12[i][0],coda_array_ordering_c);
        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // 3 Solar azimuth angles @ points EFG

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SOLAR_AZIMUTH_ACTUAL");  // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_AZIMUTH_ACTUAL
        coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->saa12[i][0],coda_array_ordering_c);
        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // 3 SZAs @ points EFG

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"SAT_ZENITH_ACTUAL");      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_ZENITH_ACTUAL
        coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->lza12[i][0],coda_array_ordering_c);
        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        // 3 Solar azimuth angles @ points EFG

        coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "SAT_AZIMUTH_ACTUAL");  // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL.SOLAR_AZIMUTH_ACTUAL
        coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->laa12[i][0],coda_array_ordering_c);
        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.GEO_EARTH_ACTUAL

        coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);
       }

      coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);
     }

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "CLOUD");               // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "FIT_MODE");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD.FIT_MODE
    coda_cursor_read_uint8_array(&pOrbitFile->gome2Cursor,&pMdr->cloudFitMode[0],coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "FIT_1");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD.FIT_1
    coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->cloudTopPressure[0],coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD

    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor, "FIT_2");            // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD.FIT_2
    coda_cursor_read_double_array(&pOrbitFile->gome2Cursor,&pMdr->cloudFraction[0],coda_array_ordering_c);
    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.CLOUD

    coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1

    // Output

    pMdr->startTime=utc_start_double;
   }

  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                        // MDR

  // Return

  return (subclass==EARTHSHINE)?0:1;
 }

RC Gome2BrowseMDR(GOME2_ORBIT_FILE *pOrbitFile,INDEX indexBand)
 {
  // Declarations

  GOME2_INFO *pGome2Info;
  DoasCh *ptr,*ptrOld;
  DoasCh geoFileName[MAX_STR_SHORT_LEN+1];
  GOME2_MDR *pMdr;
  FILE *geoFp;
  INDEX i;
  RC  rc;

  // Initializations

  pGome2Info=&pOrbitFile->gome2Info;
  rc=ERROR_ID_NO;

  // Search for the equivalent geolocation file

  strcpy(geoFileName,pOrbitFile->gome2FileName);
  if ((ptrOld=strrchr(geoFileName,PATH_SEP))==NULL)
   ptrOld=geoFileName;
  else
   ptrOld++;

  if ((ptr=strrchr(ptrOld,'.'))==NULL)
   strcat(geoFileName,".geo");
  else
   strcpy(ptr,".geo");

  if ((geoFp=fopen(geoFileName,"rb"))!=NULL)
   {
   	pGome2Info->total_nadir_mdr=STD_FileLength(geoFp)/sizeof(GOME2_MDR);
   	fread(pGome2Info->mdr,sizeof(GOME2_MDR),pGome2Info->total_nadir_mdr,geoFp);
   	fclose(geoFp);

   	for (i=0;i<pGome2Info->total_nadir_mdr;i++)
   	 pGome2Info->total_nadir_obs+=pGome2Info->mdr[i].num_recs[indexBand];
   }
  else
   {
    coda_cursor_goto_root(&pOrbitFile->gome2Cursor);
    coda_cursor_goto_record_field_by_name(&pOrbitFile->gome2Cursor,"MDR");

    for (i=0;(uint32_t)i<pGome2Info->total_mdr;i++)
     {
      coda_cursor_goto_array_element_by_index(&pOrbitFile->gome2Cursor,i);
      pMdr=&pGome2Info->mdr[pGome2Info->total_nadir_mdr];

     	if (!(rc=Gome2ReadMDRInfo(pOrbitFile,pMdr,indexBand)))
     	 {
     	 	pGome2Info->total_nadir_obs+=pMdr->num_recs[indexBand];
     	 	pMdr->indexMDR=i;

     	// 	{
     	// 		FILE *fp;
      //
     	// 		fp=fopen("toto.dat","a+t");
      //
     	// 		fprintf(fp,"version %d MDR %-2d  Obs %-2d  Int %-12.6f\n",pOrbitFile->version,i,pGome2Info->mdr[pGome2Info->total_nadir_mdr].num_recs[indexBand],
      //
     	// 		 	 	(pOrbitFile->version<=11)?pMdr->unique_int[pMdr->int_index[indexBand]]:pMdr->integration_times[indexBand]);
      //
     	// 		fclose(fp);
     	// 	}

     	  pGome2Info->total_nadir_mdr++;
     	 }

     	coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);
     }
   }

  // Return

  return rc;
 }

void Gome2ReadGeoloc(GOME2_ORBIT_FILE *pOrbitFile,INDEX indexBand)
 {
 	// Declarations

 	INDEX indexRecord,indexMDR,indexObs,indexTint;
 	INT mdrObs;
 	GOME2_MDR *pMdr;
 	GOME2_INFO *pGome2Info;

 	// Initialization

 	pGome2Info=&pOrbitFile->gome2Info;

 	for (indexRecord=0;indexRecord<pGome2Info->total_nadir_obs;indexRecord++)
 	 {
 	 	indexMDR=Gome2GetMDRIndex(pOrbitFile,indexBand,indexRecord,&mdrObs);
 	 	indexObs=indexRecord-mdrObs;

 	 	pMdr=&pGome2Info->mdr[indexMDR];

 	 	if (pOrbitFile->version<=11)
 	 	 {
 	 	  indexTint=(INDEX)pMdr->int_index[indexBand];

 	 	  // Solar zenith angles

 	 	  pOrbitFile->gome2Geolocations[indexRecord].solZen[0]=(float)pMdr->sza[indexTint][0][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].solZen[1]=(float)pMdr->sza[indexTint][1][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].solZen[2]=(float)pMdr->sza[indexTint][2][indexObs];

 	 	  // Solar azimuth angles

 	 	  pOrbitFile->gome2Geolocations[indexRecord].solAzi[0]=(float)pMdr->saa[indexTint][0][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].solAzi[1]=(float)pMdr->saa[indexTint][1][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].solAzi[2]=(float)pMdr->saa[indexTint][2][indexObs];

 	 	  // Line of sight viewing angles

 	 	  pOrbitFile->gome2Geolocations[indexRecord].losZen[0]=(float)pMdr->lza[indexTint][0][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].losZen[1]=(float)pMdr->lza[indexTint][1][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].losZen[2]=(float)pMdr->lza[indexTint][2][indexObs];

 	 	  // Line of sight azimuth angles

 	 	  pOrbitFile->gome2Geolocations[indexRecord].losAzi[0]=(float)pMdr->laa[indexTint][0][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].losAzi[1]=(float)pMdr->laa[indexTint][1][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].losAzi[2]=(float)pMdr->laa[indexTint][2][indexObs];

 	 	  // Longitudes

 	 	  pOrbitFile->gome2Geolocations[indexRecord].lonCorners[0]=pMdr->corner_lon[indexTint][0][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].lonCorners[1]=pMdr->corner_lon[indexTint][1][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].lonCorners[2]=pMdr->corner_lon[indexTint][2][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].lonCorners[3]=pMdr->corner_lon[indexTint][3][indexObs];

 	 	  pOrbitFile->gome2Geolocations[indexRecord].lonCenter=pMdr->centre_lon[indexTint][indexObs];

 	 	  // Latitudes

 	 	  pOrbitFile->gome2Geolocations[indexRecord].latCorners[0]=pMdr->corner_lat[indexTint][0][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].latCorners[1]=pMdr->corner_lat[indexTint][1][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].latCorners[2]=pMdr->corner_lat[indexTint][2][indexObs];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].latCorners[3]=pMdr->corner_lat[indexTint][3][indexObs];

 	 	  pOrbitFile->gome2Geolocations[indexRecord].latCenter=pMdr->centre_lat[indexTint][indexObs];
 	 	 }
 	 	else
 	 	 {
 	 	  // Solar zenith angles

 	 	  pOrbitFile->gome2Geolocations[indexRecord].solZen[0]=(float)pMdr->sza12[indexObs][0];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].solZen[1]=(float)pMdr->sza12[indexObs][1];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].solZen[2]=(float)pMdr->sza12[indexObs][2];

 	 	  // Solar azimuth angles

 	 	  pOrbitFile->gome2Geolocations[indexRecord].solAzi[0]=(float)pMdr->saa12[indexObs][0];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].solAzi[1]=(float)pMdr->saa12[indexObs][1];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].solAzi[2]=(float)pMdr->saa12[indexObs][2];

 	 	  // Line of sight viewing angles

 	 	  pOrbitFile->gome2Geolocations[indexRecord].losZen[0]=(float)pMdr->lza12[indexObs][0];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].losZen[1]=(float)pMdr->lza12[indexObs][1];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].losZen[2]=(float)pMdr->lza12[indexObs][2];

 	 	  // Line of sight azimuth angles

 	 	  pOrbitFile->gome2Geolocations[indexRecord].losAzi[0]=(float)pMdr->laa12[indexObs][0];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].losAzi[1]=(float)pMdr->laa12[indexObs][1];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].losAzi[2]=(float)pMdr->laa12[indexObs][2];

 	 	  // Longitudes

 	 	  pOrbitFile->gome2Geolocations[indexRecord].lonCorners[0]=pMdr->corner_lon12[indexObs][0];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].lonCorners[1]=pMdr->corner_lon12[indexObs][1];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].lonCorners[2]=pMdr->corner_lon12[indexObs][2];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].lonCorners[3]=pMdr->corner_lon12[indexObs][3];

 	 	  pOrbitFile->gome2Geolocations[indexRecord].lonCenter=pMdr->centre_lon12[indexObs];

 	 	  // Latitudes

 	 	  pOrbitFile->gome2Geolocations[indexRecord].latCorners[0]=pMdr->corner_lat12[indexObs][0];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].latCorners[1]=pMdr->corner_lat12[indexObs][1];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].latCorners[2]=pMdr->corner_lat12[indexObs][2];
 	 	  pOrbitFile->gome2Geolocations[indexRecord].latCorners[3]=pMdr->corner_lat12[indexObs][3];

 	 	  pOrbitFile->gome2Geolocations[indexRecord].latCenter=pMdr->centre_lat12[indexObs];
 	 	 }

 	 	pOrbitFile->gome2Geolocations[indexRecord].cloudTopPressure=(float)((pMdr->cloudFitMode[indexObs]==0)?pMdr->cloudTopPressure[indexObs]:-1.);
 	 	pOrbitFile->gome2Geolocations[indexRecord].cloudFraction=(float)((pMdr->cloudFitMode[indexObs]==0)?pMdr->cloudFraction[indexObs]:-1.);

    pOrbitFile->gome2Geolocations[indexRecord].scanDirection=(int)pMdr->scanDirection;
    pOrbitFile->gome2Geolocations[indexRecord].saaFlag=(int)pMdr->saaFlag[indexObs];
    pOrbitFile->gome2Geolocations[indexRecord].sunglintDangerFlag=(int)pMdr->sunglintDangerFlag[indexObs];
    pOrbitFile->gome2Geolocations[indexRecord].sunglintHighDangerFlag=(int)pMdr->sunglintHighDangerFlag[indexObs];
    pOrbitFile->gome2Geolocations[indexRecord].rainbowFlag=(int)pMdr->rainbowFlag[indexObs];

    if (pOrbitFile->gome2Geolocations[indexRecord].cloudFraction<(float)-1.)
     pOrbitFile->gome2Geolocations[indexRecord].cloudFraction=
     pOrbitFile->gome2Geolocations[indexRecord].cloudTopPressure=(float)-1.;

    Gome2Sort(pOrbitFile,indexRecord,0,indexRecord);                            // sort latitudes
    Gome2Sort(pOrbitFile,indexRecord,1,indexRecord);                            // sort longitudes
    Gome2Sort(pOrbitFile,indexRecord,2,indexRecord);                            // sort SZA
 	 }
 }

// ===================
// ALLOCATION ROUTINES
// ===================

// -----------------------------------------------------------------------------
// FUNCTION      GOME2_ReleaseBuffers
// -----------------------------------------------------------------------------
// PURPOSE       Release buffers allocated by GOME2 readout routines
// -----------------------------------------------------------------------------

void GOME2_ReleaseBuffers(void)
 {
 	// Declarations

 	GOME2_ORBIT_FILE *pOrbitFile;
 	INDEX gome2OrbitFileIndex;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("GOME2_ReleaseBuffers",DEBUG_FCTTYPE_FILE|DEBUG_FCTTYPE_MEM);
  #endif

  for (gome2OrbitFileIndex=0;gome2OrbitFileIndex<gome2OrbitFilesN;gome2OrbitFileIndex++)
   {
   	pOrbitFile=&gome2OrbitFiles[gome2OrbitFileIndex];

    if (pOrbitFile->gome2Info.mdr!=NULL)
     MEMORY_ReleaseBuffer("GOME2_ReleaseBuffers","MDR",pOrbitFile->gome2Info.mdr);
    if (pOrbitFile->gome2Geolocations!=NULL)
     MEMORY_ReleaseBuffer("GOME2_ReleaseBuffers","gome2Geolocations",pOrbitFile->gome2Geolocations);
    if (pOrbitFile->gome2LatIndex!=NULL)
     MEMORY_ReleaseBuffer("GOME2_ReleaseBuffers","gome2LatIndex",pOrbitFile->gome2LatIndex);
    if (pOrbitFile->gome2LonIndex!=NULL)
     MEMORY_ReleaseBuffer("GOME2_ReleaseBuffers","gome2LonIndex",pOrbitFile->gome2LonIndex);
    if (pOrbitFile->gome2SzaIndex!=NULL)
     MEMORY_ReleaseBuffer("GOME2_ReleaseBuffers","gome2SzaIndex",pOrbitFile->gome2SzaIndex);

    if (pOrbitFile->gome2SunRef!=NULL)
     MEMORY_ReleaseBuffer("GOME2_ReleaseBuffers","gome2SunRef",pOrbitFile->gome2SunRef);
    if (pOrbitFile->gome2SunWve!=NULL)
     MEMORY_ReleaseBuffer("GOME2_ReleaseBuffers","gome2SunWve",pOrbitFile->gome2SunWve);

 	  // Close the current file

    if (pOrbitFile->gome2Pf!=NULL)
     coda_close(pOrbitFile->gome2Pf);
   }

  for (gome2OrbitFileIndex=0;gome2OrbitFileIndex<MAX_GOME2_FILES;gome2OrbitFileIndex++)
  	memset(&gome2OrbitFiles[gome2OrbitFileIndex],0,sizeof(GOME2_ORBIT_FILE));

  gome2OrbitFilesN=0;
  gome2CurrentFileIndex=ITEM_NONE;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("GOME2_ReleaseBuffers",0);
  #endif
 }

// =======================
// GOME2 READ OUT ROUTINES
// =======================

// -----------------------------------------------------------------------------
// FUNCTION      GOME2_Set
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

RC GOME2_Set(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  GOME2_ORBIT_FILE *pOrbitFile;                                                 // pointer to the current orbit
  DoasCh filePath[MAX_STR_SHORT_LEN+1];
  DoasCh fileFilter[MAX_STR_SHORT_LEN+1];
  #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
  WIN32_FIND_DATA fileInfo,fileInfoSub;                                         // structure returned by FindFirstFile and FindNextFile APIs
  HANDLE hDir,hDirSub;                                                          // handle to use with by FindFirstFile and FindNextFile APIs
  #else
  struct dirent *fileInfo;
  DIR *hDir;
  #endif
  INDEX indexFile;
  INT searchAllOrbits;
  DoasCh *ptr,*fileExt;
  INT oldCurrentIndex;
  RC rc;                                                                  // return code

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("GOME2_Set",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  // gome2LoadReferenceFlag=0;
  ANALYSE_oldLatitude=(double)99999.;
  pEngineContext->recordNumber=0;
  oldCurrentIndex=gome2CurrentFileIndex;
  gome2CurrentFileIndex=ITEM_NONE;
  rc=ERROR_ID_NO;

  if (!GOME2_beatLoaded)
   {
    coda_init();
    coda_set_option_perform_boundary_checks(1);
    GOME2_beatLoaded=1;
   }

  // In automatic reference selection, the file has maybe already loaded

  if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto)
   {
    // Close the previous files

    if (gome2OrbitFilesN && (oldCurrentIndex!=ITEM_NONE) && (oldCurrentIndex<gome2OrbitFilesN) &&
       (gome2OrbitFiles[oldCurrentIndex].gome2Pf!=NULL))
     {
     	coda_close(gome2OrbitFiles[oldCurrentIndex].gome2Pf);
      gome2OrbitFiles[oldCurrentIndex].gome2Pf=NULL;
     }

    for (indexFile=0;indexFile<gome2OrbitFilesN;indexFile++)
     {
      if ((strlen(pEngineContext->fileInfo.fileName)==strlen(gome2OrbitFiles[indexFile].gome2FileName)) &&
          !strcasecmp(pEngineContext->fileInfo.fileName,gome2OrbitFiles[indexFile].gome2FileName))
       break;
     }

    if (indexFile<gome2OrbitFilesN)
     gome2CurrentFileIndex=indexFile;
   }

  if (gome2CurrentFileIndex==ITEM_NONE)
   {
   	searchAllOrbits=0;

   	// Release old buffers

   	GOME2_ReleaseBuffers();

   	// Get the number of files to load

   	if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto)
    	{
    		gome2LoadReferenceFlag=1;

  		  // Get file path

   	  strcpy(filePath,pEngineContext->fileInfo.fileName);

   	  if ((ptr=strrchr(filePath,PATH_SEP))==NULL)
   	   {
  	   	 strcpy(filePath,".");
  	   	 ptr=pEngineContext->fileInfo.fileName;
  	   	}
  	   else
  	    *ptr++=0;

  	   fileExt=strrchr(ptr,'.');

 	 	  // Build file filter

 	 	  strcpy(fileFilter,pEngineContext->fileInfo.fileName);
 	 	  if ((ptr=strrchr(fileFilter,PATH_SEP))==NULL)
 	 	   {
 	 	    strcpy(fileFilter,".");
 	 	    ptr=fileFilter+1;
 	 	   }

 	 	  if (fileExt==NULL)
 	 	   sprintf(ptr,"%c*.*",PATH_SEP);
 	 	  else if (!pEngineContext->analysisRef.refLon || strcasecmp(fileExt,".nadir"))
 	 	   sprintf(ptr,"%c*.%s",PATH_SEP,fileExt);
 	 	  else
 	 	   {
 	 	   	*ptr='\0';
 	 	   	if ((ptr=strrchr(fileFilter,PATH_SEP))==NULL)                           // goto the parent directory
	 	      sprintf(fileFilter,"*.nadir");
	 	     else
	 	      {
	 	      	searchAllOrbits=1;
	 	      	*ptr='\0';
	 	      	strcpy(filePath,fileFilter);
 	 	      sprintf(ptr,"%c*.*",PATH_SEP);
 	 	     }
 	 	   }

      // Search for files of the same orbit

      for (hDir=opendir(filePath);(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);)
        {
          sprintf(gome2OrbitFiles[gome2OrbitFilesN].gome2FileName,"%s/%s",filePath,fileInfo->d_name);
          if ( STD_IsDir(gome2OrbitFiles[gome2OrbitFilesN].gome2FileName) == 0 )
           gome2OrbitFilesN++;
        }

      if ( hDir != NULL ) closedir(hDir);

     //   for (hDir=opendir(filePath);(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);)
     //    {
     //     sprintf(GDP_BIN_orbitFiles[gdpBinOrbitFilesN].gdpBinFileName,"%s/%s",filePath,fileInfo->d_name);
     //     if (!STD_IsDir(GDP_BIN_orbitFiles[gdpBinOrbitFilesN].gdpBinFileName))
     //      {
     //      	strcpy(filePrefix,fileInfo->d_name);
     //      	filePrefix[6]='\0';
     //
     //       if (((ptr=strrchr(fileInfo->d_name,'.'))!=NULL) && (strlen(ptr+1)==strlen(fileExt)) && !strcasecmp(ptr+1,fileExt) &&
     //            (strlen(filePrefix)==strlen(fileFilter)) && !strcasecmp(filePrefix,fileFilter))
     //        gdpBinOrbitFilesN++;
     //      }
     //    }

      rc=ERROR_ID_NO;
   	 }
   	else
     {
     	gome2OrbitFilesN=1;
     	gome2CurrentFileIndex=0;
     	strcpy(gome2OrbitFiles[0].gome2FileName,pEngineContext->fileInfo.fileName);
     }

    // Load files

    for (gome2TotalRecordNumber=indexFile=0;indexFile<gome2OrbitFilesN;indexFile++)
     {
     	pOrbitFile=&gome2OrbitFiles[indexFile];

     	pOrbitFile->gome2Pf=NULL;
     	pOrbitFile->specNumber=0;

      // Open the file

      if (!(rc=Gome2Open(&pOrbitFile->gome2Pf,pOrbitFile->gome2FileName,&pOrbitFile->version)))
       {
       	coda_cursor_set_product(&pOrbitFile->gome2Cursor,pOrbitFile->gome2Pf);

       	Gome2ReadOrbitInfo(pOrbitFile,(int)pEngineContext->project.instrumental.user);
       	Gome2BrowseMDR(pOrbitFile,(int)pEngineContext->project.instrumental.user);

       	if ((pOrbitFile->specNumber=(THRD_browseType==THREAD_BROWSE_DARK)?1:pOrbitFile->gome2Info.total_nadir_obs)>0)
       	 {
       	  if (((pOrbitFile->gome2Geolocations=(SATELLITE_GEOLOC *)MEMORY_AllocBuffer("GOME2_Set","geoloc",pOrbitFile->specNumber,sizeof(SATELLITE_GEOLOC),0,MEMORY_TYPE_STRUCT))==NULL) ||
              ((pOrbitFile->gome2LatIndex=(INDEX *)MEMORY_AllocBuffer("GOME2_Set","gome2LatIndex",pOrbitFile->specNumber,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
              ((pOrbitFile->gome2LonIndex=(INDEX *)MEMORY_AllocBuffer("GOME2_Set","gome2LonIndex",pOrbitFile->specNumber,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
              ((pOrbitFile->gome2SzaIndex=(INDEX *)MEMORY_AllocBuffer("GOME2_Set","gome2SzaIndex",pOrbitFile->specNumber,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL))

       	   rc=ERROR_ID_ALLOC;
       	  else if (!(rc=Gome2ReadSunRef(pOrbitFile,(int)pEngineContext->project.instrumental.user)))
     	   	 Gome2ReadGeoloc(pOrbitFile,(int)pEngineContext->project.instrumental.user);
         }

        if (pOrbitFile->gome2Pf!=NULL)
         {
     	    coda_close(pOrbitFile->gome2Pf);
          pOrbitFile->gome2Pf=NULL;
         }

        if ((strlen(pEngineContext->fileInfo.fileName)==strlen(pOrbitFile->gome2FileName)) &&
            !strcasecmp(pEngineContext->fileInfo.fileName,pOrbitFile->gome2FileName))
         gome2CurrentFileIndex=indexFile;

        gome2TotalRecordNumber+=pOrbitFile->specNumber;

        pOrbitFile->rc=rc;
        rc=ERROR_ID_NO;
       }
     }
   }

  if (gome2CurrentFileIndex==ITEM_NONE)
   rc=ERROR_SetLast("GOME2_Set",ERROR_TYPE_FATAL,ERROR_ID_ALLOC,"gome2OrbitFiles");
  else if (!(pEngineContext->recordNumber=(pOrbitFile=&gome2OrbitFiles[gome2CurrentFileIndex])->specNumber))
   rc=ERROR_SetLast("GOME2_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pOrbitFile->gome2FileName);
  else
   {
    if (!(rc=pOrbitFile->rc) && (pOrbitFile->gome2Pf==NULL) &&
        !(rc=Gome2Open(&pOrbitFile->gome2Pf,pEngineContext->fileInfo.fileName,&pOrbitFile->version)))
     {
     	coda_cursor_set_product(&pOrbitFile->gome2Cursor,pOrbitFile->gome2Pf);
      memcpy(pEngineContext->buffers.lambda,pOrbitFile->gome2SunWve,sizeof(double)*NDET);

     	memcpy(pEngineContext->buffers.irrad,pOrbitFile->gome2SunRef,sizeof(double)*NDET);
     }
   }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("GOME2_Set",rc);
  #endif

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      GOME2_Read
// -----------------------------------------------------------------------------
// PURPOSE       GOME2 level 1 data read out
//
// INPUT         recordNo     index of the record to read
//
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_FILE_END        the end of the file is reached;
//               ERROR_ID_FILE_RECORD     the record doesn't satisfy user constraints
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

RC GOME2_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,INDEX fileIndex)
 {
 	// Declarations

  double   *unique_int;                                                         // integration time
  uint8_t  *int_index;                                                          // index of the integration time
  uint32_t  spe32;
  int i;
  double tint;
  double *spectrum,*sigma;                                                      // radiances and errors
  INDEX indexMDR;
  INT mdrObs;
  INDEX indexBand;
  INT year,month,day,hour,min,sec,msec;
  double utcTime;
  GOME2_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  SATELLITE_GEOLOC *pGeoloc;
  GOME2_INFO *pGome2Info;
  RECORD_INFO *pRecord;
 	RC rc;                                                                        // return code

 	// Debug

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("GOME2_Read",DEBUG_FCTTYPE_FILE);
  #endif

 	// Initializations

 	pOrbitFile=&gome2OrbitFiles[(fileIndex==ITEM_NONE)?gome2CurrentFileIndex:fileIndex];
 	pGome2Info=&pOrbitFile->gome2Info;
 	spectrum=pEngineContext->buffers.spectrum;
 	sigma=pEngineContext->buffers.sigmaSpec;
 	indexBand=(INDEX)pEngineContext->project.instrumental.user;
 	pRecord=&pEngineContext->recordInfo;

 	rc=ERROR_ID_NO;

  // Goto the requested record

  if (!pOrbitFile->specNumber)
   rc=ERROR_ID_FILE_EMPTY;
  else if ((recordNo<=0) || (recordNo>pOrbitFile->specNumber))
   rc=ERROR_ID_FILE_END;
  else if ((THRD_id!=THREAD_TYPE_SPECTRA) || (THRD_browseType!=THREAD_BROWSE_DARK))
   {
    for (i=0;i<NDET;i++)
     spectrum[i]=sigma[i]=(double)0.;

    if ((indexMDR=Gome2GetMDRIndex(pOrbitFile,indexBand,recordNo-1,&mdrObs))==ITEM_NONE)
     rc=ERROR_ID_FILE_RECORD;
    else
     {
     	unique_int=pGome2Info->mdr[indexMDR].unique_int;
     	int_index=pGome2Info->mdr[indexMDR].int_index;

     	tint=(pOrbitFile->version<=11)?unique_int[int_index[indexBand]]:pGome2Info->mdr[indexMDR].integration_times[indexBand];

     	Gome2GotoOBS(pOrbitFile,(INDEX)indexBand,indexMDR,recordNo-mdrObs-1);

     	for (i=pGome2Info->startPixel;(i<pGome2Info->startPixel+pGome2Info->no_of_pixels) && !rc;i++)
     	 {
     	 	coda_cursor_goto_first_record_field(&pOrbitFile->gome2Cursor);                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i].RAD


     	 	coda_cursor_read_uint32(&pOrbitFile->gome2Cursor,&spe32);
     	  coda_cursor_read_double(&pOrbitFile->gome2Cursor,&spectrum[i]);


     	  coda_cursor_goto_next_record_field(&pOrbitFile->gome2Cursor);                       // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i].ERR_RAD
     	  coda_cursor_read_double(&pOrbitFile->gome2Cursor,&sigma[i]);
     	  coda_cursor_goto_parent(&pOrbitFile->gome2Cursor);                                  // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i]
     	  coda_cursor_goto_next_array_element(&pOrbitFile->gome2Cursor);                      // MDR.GOME2_MDR_L1B_EARTHSHINE_V1.BAND[i+1]

     	  if (fabs(spectrum[i])>(double)1.e20)
     	   rc=ERROR_ID_FILE_RECORD;
     	 }

      if (!rc)
       {
     	  utcTime=pGome2Info->mdr[indexMDR].startTime+tint*(recordNo-mdrObs-2);     // NOV 2011 : problem with integration time (FRESCO comparison)
     	  coda_double_to_datetime(utcTime,&year,&month,&day,&hour,&min,&sec,&msec);

     	  // Output information on the current record

     	  pRecord->present_day.da_day=(char)day;
        pRecord->present_day.da_mon=(char)month;
        pRecord->present_day.da_year=(short)year;

        pRecord->present_time.ti_hour=(unsigned char)hour;
        pRecord->present_time.ti_min=(unsigned char)min;
        pRecord->present_time.ti_sec=(unsigned char)sec;

        GOME2_ms=msec;

        // Geolocation

        pGeoloc=&pOrbitFile->gome2Geolocations[recordNo-1];

        memcpy(pRecord->gome2.latitudes,pGeoloc->latCorners,sizeof(double)*4);
        memcpy(pRecord->gome2.longitudes,pGeoloc->lonCorners,sizeof(double)*4);
        memcpy(pRecord->gome2.solZen,pGeoloc->solZen,sizeof(float)*3);
        memcpy(pRecord->gome2.solAzi,pGeoloc->solAzi,sizeof(float)*3);
        memcpy(pRecord->gome2.losZen,pGeoloc->losZen,sizeof(float)*3);
        memcpy(pRecord->gome2.losAzi,pGeoloc->losAzi,sizeof(float)*3);

        pRecord->cloudTopPressure=pGeoloc->cloudTopPressure;
        pRecord->cloudFraction=pGeoloc->cloudFraction;

        pRecord->gome2.scanDirection=pGeoloc->scanDirection;
        pRecord->gome2.saaFlag=pGeoloc->saaFlag;
        pRecord->gome2.sunglintDangerFlag=pGeoloc->sunglintDangerFlag;
        pRecord->gome2.sunglintHighDangerFlag=pGeoloc->sunglintHighDangerFlag;
        pRecord->gome2.rainbowFlag=pGeoloc->rainbowFlag;

        // Miscellaneous data (for TEMIS)

        pRecord->latitude=pGeoloc->latCenter;
        pRecord->longitude=pGeoloc->lonCenter;

        pRecord->Zm=pGeoloc->solZen[1];
        pRecord->Azimuth=pGeoloc->solAzi[1];
        pRecord->zenithViewAngle=pGeoloc->losZen[1];
        pRecord->azimuthViewAngle=pGeoloc->losAzi[1];

        pRecord->Tint=tint;

        pRecord->TimeDec=(double)hour+min/60.+(sec+msec*1.e-3)/(60.*60.);
        pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);

        pRecord->gome2.orbitNumber=(int)pOrbitFile->gome2Info.orbitStart;

 	 	  // {
 	 	  // 	FILE *fp;
 	 	  // 	fp=fopen("toto.dat","a+t");
 	 	  // 	fprintf(fp,"%-2d %-2d %02d%02d%02d.%03d %-10.4f %-10.4f %-10.4f %-10.4f %-10.4f %-10.4f %-10.4f %-10.4f %-10.4f %-10.4f \n",indexMDR,recordNo-mdrObs,
 	 	  // 	            hour,min,sec,(int)floor(msec/1000+0.5),
      //
 	 	  // 	            pGeoloc->latCorners[0],
 	 	  // 	            pGeoloc->latCorners[1],
 	 	  // 	            pGeoloc->latCorners[2],
 	 	  // 	            pGeoloc->latCorners[3],
 	 	  // 	            pGeoloc->latCenter,
 	 	  // 	            pGeoloc->lonCorners[0],
 	 	  // 	            pGeoloc->lonCorners[1],
 	 	  // 	            pGeoloc->lonCorners[2],
 	 	  // 	            pGeoloc->lonCorners[3],
 	 	  // 	            pGeoloc->lonCenter);
      //
 	 	  // 	fclose(fp);
 	 	  // }

        if ((pEngineContext->project.spectra.cloudMin>=0.) &&
            (pEngineContext->project.spectra.cloudMax<=1.) &&
            (fabs((double)(pEngineContext->project.spectra.cloudMax-pEngineContext->project.spectra.cloudMin))>EPSILON) &&
            (fabs((double)(pEngineContext->project.spectra.cloudMax-pEngineContext->project.spectra.cloudMin))<1.-EPSILON) &&
           ((pRecord->cloudFraction<pEngineContext->project.spectra.cloudMin-EPSILON) || (pRecord->cloudFraction>pEngineContext->project.spectra.cloudMax+EPSILON)))

         rc=ERROR_ID_FILE_RECORD;
       }
     }
   }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("GOME2_Read",rc);
  #endif

 	// Return

 	return rc;
 }

// =============================
// AUTOMATIC REFERENCE SELECTION
// =============================

// -----------------------------------------------------------------------------
// FUNCTION      Gome2SortGetIndex
// -----------------------------------------------------------------------------
// PURPOSE       Get the position in sorted list of a new element (latitude, longitude or SZA)
//
// INPUT         fileIndex    index of the current file
//               value        the value to sort out
//               flag         0 for latitudes, 1 for longitudes, 2 for SZA
//               listSize     the current size of the sorted list
//
// RETURN        the index of the new element in the sorted list;
// -----------------------------------------------------------------------------

INDEX Gome2SortGetIndex(GOME2_ORBIT_FILE *pOrbitFile,double value,int flag,int listSize)
 {
  // Declarations

  INDEX  imin,imax,icur;                                                        // indexes for dichotomic search
  double curValue;                                                              // value of element pointed by icur in the sorted list
  double curMinValue,curMaxValue;                                               // range of values

  // Initializations

  imin=icur=0;
  imax=listSize;
  curValue=curMinValue=curMaxValue=(double)0.;

  // Browse latitudes

  while (imax-imin>1)
   {
    // Dichotomic search

    icur=(imin+imax)>>1;

    switch(flag)
     {
   // ----------------------------------------------------------------------------
      case 0 :
       curValue=(double)pOrbitFile->gome2Geolocations[pOrbitFile->gome2LatIndex[icur]].latCenter;
      break;
   // ----------------------------------------------------------------------------
      case 1 :
       curValue=(double)pOrbitFile->gome2Geolocations[pOrbitFile->gome2LonIndex[icur]].lonCenter;
      break;
   // ----------------------------------------------------------------------------
      case 2 :
       curValue=(double)pOrbitFile->gome2Geolocations[pOrbitFile->gome2SzaIndex[icur]].solZen[1];
      break;
   // ----------------------------------------------------------------------------
     }

    // Move bounds

    if (curValue==value)
     imin=imax=icur;
    else if (curValue<value)
     imin=icur;
    else
     imax=icur;
   }

  if ((listSize>0) && (imax<listSize))
   {
    switch(flag)
     {
   // ----------------------------------------------------------------------------
      case 0 :
       curMinValue=(double)pOrbitFile->gome2Geolocations[pOrbitFile->gome2LatIndex[imin]].latCenter;
       curMaxValue=(double)pOrbitFile->gome2Geolocations[pOrbitFile->gome2LatIndex[imax]].latCenter;
      break;
   // ----------------------------------------------------------------------------
      case 1 :
       curMinValue=(double)pOrbitFile->gome2Geolocations[pOrbitFile->gome2LonIndex[imin]].lonCenter;
       curMaxValue=(double)pOrbitFile->gome2Geolocations[pOrbitFile->gome2LatIndex[imax]].lonCenter;
      break;
   // ----------------------------------------------------------------------------
      case 2 :
       curMinValue=(double)pOrbitFile->gome2Geolocations[pOrbitFile->gome2SzaIndex[imin]].solZen[1];
       curMaxValue=(double)pOrbitFile->gome2Geolocations[pOrbitFile->gome2LatIndex[imax]].solZen[1];
      break;
   // ----------------------------------------------------------------------------
    }

    icur=(value-curMinValue<curMaxValue-value)?imin:imax;
   }

  // Return

  return icur;
 }

// -----------------------------------------------------------------------------
// FUNCTION      Gome2Sort
// -----------------------------------------------------------------------------
// PURPOSE       Sort NADIR records on latitudes, longitudes or SZA
//
// INPUT         fileIndex    index of the current file
//               indexRecord  the index of the record to sort out
//               flag         0 for latitudes, 1 for longitudes, 2 for SZA
//               listSize     the current size of the sorted list
//
// RETURN        the new index of the record in the sorted list;
// -----------------------------------------------------------------------------

void Gome2Sort(GOME2_ORBIT_FILE *pOrbitFile,INDEX indexRecord,int flag,int listSize)
 {
  // Declaration

  INDEX  newIndex,                                                              // the position of the new record in the sorted list
        *sortedList,                                                            // the sorted list
         i;                                                                     // browse the sorted list in reverse way

  double value;                                                                 // the value to sort out

  // Initializations

  value=(double)0.;

  switch(flag)
   {
 // ----------------------------------------------------------------------------
    case 0 :
     sortedList=pOrbitFile->gome2LatIndex;
     value=pOrbitFile->gome2Geolocations[indexRecord].latCenter;
    break;
 // ----------------------------------------------------------------------------
    case 1 :
     sortedList=pOrbitFile->gome2LonIndex;
     value=pOrbitFile->gome2Geolocations[indexRecord].lonCenter;
    break;
 // ----------------------------------------------------------------------------
    case 2 :
    default :
     sortedList=pOrbitFile->gome2SzaIndex;
     value=pOrbitFile->gome2Geolocations[indexRecord].solZen[1];
    break;
 // ----------------------------------------------------------------------------
   }

  newIndex=Gome2SortGetIndex(pOrbitFile,value,flag,listSize);

  // Shift values higher than the one to sort out

  if (newIndex<listSize)
   for (i=listSize;i>newIndex;i--)
    sortedList[i]=sortedList[i-1];

  // Insert new record in the sorted list

  sortedList[newIndex]=indexRecord;
 }

// -----------------------------------------------------------------------------
// FUNCTION      Gome2RefLat
// -----------------------------------------------------------------------------
// PURPOSE       Search for spectra in the orbit file matching latitudes and SZA
//               conditions
//
// INPUT         maxRefSize         the maximum size of vectors
//               latMin,latMax      determine the range of latitudes;
//               sza,szaDelta       determine the range of SZA;
//               cloudMin,cloudMax  determine the range of cloud fraction
//
// OUTPUT        refList       the list of potential reference spectra
//
// RETURN        the number of elements in the refList reference list
// -----------------------------------------------------------------------------

INT Gome2RefLat(GOME2_REF *refList,INT maxRefSize,double latMin,double latMax,double lonMin,double lonMax,double sza,double szaDelta,double cloudMin,double cloudMax)
 {
  // Declarations

  INDEX fileIndex;
  GOME2_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX ilatMin,ilatMax,ilatTmp,                                                // range of indexes of latitudes matching conditions in sorted list
        ilatIndex,                                                              // browse records with latitudes in the specified range
        indexRef;                                                               // browse reference already registered in order to keep the list sorted

  INT nRef;                                                                     // the number of spectra matching latitudes and SZA conditions
  double szaDist,latDist;                                                       // distance with latitude and sza centers
  double lon;                                                                   // converts the longitude in the -180:180 range
  SATELLITE_GEOLOC *pRecord;

  // Initialization

  nRef=0;

  for (fileIndex=0;(fileIndex<gome2OrbitFilesN) && !nRef;fileIndex++)
   {
   	pOrbitFile=&gome2OrbitFiles[fileIndex];

   	if (!pOrbitFile->rc && pOrbitFile->specNumber)
   	 {
      // Determine the set of records in the orbit file matching the latitudes conditions

      ilatMin=Gome2SortGetIndex(pOrbitFile,latMin,0,pOrbitFile->specNumber);
      ilatMax=Gome2SortGetIndex(pOrbitFile,latMax,0,pOrbitFile->specNumber);

      if (ilatMin>ilatMax)
       {
        ilatTmp=ilatMin;
        ilatMin=ilatMax;
        ilatMax=ilatTmp;
       }

      // Browse spectra matching latitudes conditions

      for (ilatIndex=ilatMin;(ilatIndex<ilatMax) && (nRef<maxRefSize);ilatIndex++)

       if (ilatIndex<pOrbitFile->specNumber)
        {
         pRecord=&pOrbitFile->gome2Geolocations[pOrbitFile->gome2LatIndex[ilatIndex]];

         if ((fabs(lonMax-lonMin)>EPSILON) && (fabs(lonMax-lonMin)<(double)359.))
          latDist=THRD_GetDist(pRecord->lonCenter,pRecord->latCenter,(lonMax+lonMin)*0.5,(latMax+latMin)*0.5);
         else
          latDist=fabs(pRecord->latCenter-(latMax+latMin)*0.5);

         szaDist=fabs(pRecord->solZen[1]-sza);

         lon=((lonMax>(double)180.) &&
              (pRecord->lonCenter<(double)0.))?pRecord->lonCenter+360:pRecord->lonCenter;         // Longitudes for GOME2 are in the range -180..180 */

         // Limit the latitudes conditions to SZA conditions

         if ((pRecord->latCenter>=latMin) && (pRecord->latCenter<=latMax) &&
            ((szaDelta<EPSILON) || (szaDist<=szaDelta)) &&
            ((fabs(lonMax-lonMin)<EPSILON) || (fabs(lonMax-lonMin)>359.) ||
            ((lon>=lonMin) && (lon<=lonMax))) &&
            ((fabs(cloudMax-cloudMin)<EPSILON) || (fabs(cloudMax-cloudMin)>(double)1.-EPSILON) ||
            ((pRecord->cloudFraction>=cloudMin) && (pRecord->cloudFraction<=cloudMax))) )
          {
           // Keep the list of records sorted

           for (indexRef=nRef;indexRef>0;indexRef--)

            if (latDist>=refList[indexRef-1].latDist)
             break;
            else
             memcpy(&refList[indexRef],&refList[indexRef-1],sizeof(GOME2_REF));

           refList[indexRef].indexFile=fileIndex;
           refList[indexRef].indexRecord=pOrbitFile->gome2LatIndex[ilatIndex];
           refList[indexRef].latitude=pRecord->latCenter;
           refList[indexRef].longitude=pRecord->lonCenter;
           refList[indexRef].cloudFraction=pRecord->cloudFraction;
           refList[indexRef].sza=pRecord->solZen[1];
           refList[indexRef].szaDist=szaDist;
           refList[indexRef].latDist=latDist;

           nRef++;
          }
        }
     }
   }

  // Return

  return nRef;
 }

// -----------------------------------------------------------------------------
// FUNCTION      Gome2RefSza
// -----------------------------------------------------------------------------
// PURPOSE       Search for spectra in the orbit file matching SZA
//               conditions only
//
// INPUT         maxRefSize   the maximum size of vectors
//               sza,szaDelta determine the range of SZA;
//
// OUTPUT        refList      the list of potential reference spectra
//
// RETURN        the number of elements in the refList reference list
// -----------------------------------------------------------------------------

INT Gome2RefSza(GOME2_REF *refList,INT maxRefSize,double sza,double szaDelta,double cloudMin,double cloudMax)
 {
  // Declarations

  INDEX fileIndex;
  GOME2_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX iszaMin,iszaMax,iszaTmp,                                                // range of indexes of SZA matching conditions in sorted list
        iszaIndex,                                                              // browse records with SZA in the specified SZA range
        indexRef;                                                               // browse reference already registered in order to keep the list sorted

  INT nRef;                                                                     // the number of spectra matching latitudes and SZA conditions
  double szaDist;                                                               // distance with sza center
  SATELLITE_GEOLOC *pRecord;

  // Initialization

  nRef=0;

  for (fileIndex=0;(fileIndex<gome2OrbitFilesN) && !nRef;fileIndex++)
   {
    pOrbitFile=&gome2OrbitFiles[fileIndex];

    if (!pOrbitFile->rc && pOrbitFile->specNumber)
     {
      // Determine the set of records in the orbit file matching SZA conditions

      if (szaDelta>EPSILON)
       {
        iszaMin=Gome2SortGetIndex(pOrbitFile,sza-szaDelta,2,pOrbitFile->specNumber);
        iszaMax=Gome2SortGetIndex(pOrbitFile,sza+szaDelta,2,pOrbitFile->specNumber);
       }
      else  // No SZA conditions, search for the minimum
       {
        iszaMin=0;
        iszaMax=pOrbitFile->specNumber-1;
       }

      if (iszaMin>iszaMax)
       {
        iszaTmp=iszaMin;
        iszaMin=iszaMax;
        iszaMax=iszaTmp;
       }

      // Browse spectra matching SZA conditions

      for (iszaIndex=iszaMin;(iszaIndex<iszaMax) && (nRef<maxRefSize);iszaIndex++)

       if (iszaIndex<pOrbitFile->specNumber)
        {
         pRecord=&pOrbitFile->gome2Geolocations[pOrbitFile->gome2SzaIndex[iszaIndex]];
         szaDist=fabs(pRecord->solZen[1]-sza);

         if (((szaDelta<EPSILON) || (szaDist<=szaDelta)) &&
            ((fabs(cloudMax-cloudMin)<EPSILON) || (fabs(cloudMax-cloudMin)>(double)1.-EPSILON) ||
            ((pRecord->cloudFraction>=cloudMin) && (pRecord->cloudFraction<=cloudMax))))
          {
           // Keep the list of records sorted

           for (indexRef=nRef;indexRef>0;indexRef--)

            if (szaDist>=refList[indexRef-1].szaDist)
             break;
            else
             memcpy(&refList[indexRef],&refList[indexRef-1],sizeof(GOME2_REF));

           refList[indexRef].indexFile=fileIndex;
           refList[indexRef].indexRecord=pOrbitFile->gome2SzaIndex[iszaIndex];
           refList[indexRef].latitude=pRecord->latCenter;
           refList[indexRef].longitude=pRecord->lonCenter;
           refList[indexRef].cloudFraction=pRecord->cloudFraction;
           refList[indexRef].sza=pRecord->solZen[1];
           refList[indexRef].szaDist=szaDist;
           refList[indexRef].latDist=(double)0.;

           nRef++;
          }
        }
     }
   }

  // Return

  return nRef;
 }

// -----------------------------------------------------------------------------
// FUNCTION      Gome2BuildRef
// -----------------------------------------------------------------------------
// PURPOSE       Build a reference spectrum by averaging a set of spectra
//               matching latitudes and SZA conditions
//
// INPUT         refList      the list of potential reference spectra
//               nRef         the number of elements in the previous list
//               nSpectra     the maximum number of spectra to average to build the reference spectrum;
//               lambda       the grid of the irradiance spectrum
//               pEngineContext    interface for file operations
//               fp           pointer to the file dedicated to the display of information on selected spectra
//
// OUTPUT        ref          the new reference spectrum
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_HDF in one of HDF file operation failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC Gome2BuildRef(GOME2_REF *refList,INT nRef,INT nSpectra,double *lambda,double *ref,ENGINE_CONTEXT *pEngineContext,INDEX *pIndexLine,void *responseHandle)
 {
  // Declarations

  RECORD_INFO *pRecord;
  GOME2_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  GOME2_REF *pRef;                                                               // pointer to the current reference spectrum
  INDEX     indexRef,                                                           // browse reference in the list
            indexFile,                                                          // browse files
            i;                                                                  // index for loop and arrays
  INT       nRec;                                                               // number of records use for the average
  INT       alreadyOpen;
  INDEX     indexColumn;
  RC        rc;                                                                 // return code

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  for (i=0;i<NDET;i++)
   ref[i]=(double)0.;

  indexColumn=2;
  rc=ERROR_ID_NO;

  // Search for spectra matching latitudes and SZA conditions

  for (nRec=0,indexRef=0,indexFile=ITEM_NONE;
      (indexRef<nRef) && (nRec<nSpectra) && !rc;indexRef++)
   {
    pRef=&refList[indexRef];

    if ((indexFile==ITEM_NONE) || (pRef->indexFile==indexFile))
     {
      pOrbitFile=&gome2OrbitFiles[pRef->indexFile];

      if (pOrbitFile->rc==ERROR_ID_NO)
       {
        alreadyOpen=(pOrbitFile->gome2Pf!=NULL)?1:0;

        if (!alreadyOpen && !(rc=Gome2Open(&pOrbitFile->gome2Pf,pOrbitFile->gome2FileName,&pOrbitFile->version)))
        	coda_cursor_set_product(&pOrbitFile->gome2Cursor,pOrbitFile->gome2Pf);

        if (!(rc=GOME2_Read(pEngineContext,pRef->indexRecord,pRef->indexFile)))
         {
          if (indexFile==ITEM_NONE)
           {
            mediateResponseCellDataString(plotPageRef,(*pIndexLine)++,indexColumn,"Ref Selection",responseHandle);
            mediateResponseCellInfo(plotPageRef,(*pIndexLine)++,indexColumn,responseHandle,"Ref File","%s",gome2OrbitFiles[refList[0].indexFile].gome2FileName);
            mediateResponseCellDataString(plotPageRef,(*pIndexLine),indexColumn,"Record",responseHandle);
            mediateResponseCellDataString(plotPageRef,(*pIndexLine),indexColumn+1,"SZA",responseHandle);
            mediateResponseCellDataString(plotPageRef,(*pIndexLine),indexColumn+2,"Lat",responseHandle);
            mediateResponseCellDataString(plotPageRef,(*pIndexLine),indexColumn+3,"Lon",responseHandle);
            mediateResponseCellDataString(plotPageRef,(*pIndexLine),indexColumn+4,"CF",responseHandle);

            (*pIndexLine)++;

            strcpy(pRecord->refFileName,gome2OrbitFiles[pRef->indexFile].gome2FileName);
            pRecord->refRecord=pRef->indexRecord+1;
           }

         	mediateResponseCellDataInteger(plotPageRef,(*pIndexLine),indexColumn,pRef->indexRecord+1,responseHandle);
         	mediateResponseCellDataDouble(plotPageRef,(*pIndexLine),indexColumn+1,pRef->sza,responseHandle);
         	mediateResponseCellDataDouble(plotPageRef,(*pIndexLine),indexColumn+2,pRef->latitude,responseHandle);
         	mediateResponseCellDataDouble(plotPageRef,(*pIndexLine),indexColumn+3,pRef->longitude,responseHandle);
         	mediateResponseCellDataDouble(plotPageRef,(*pIndexLine),indexColumn+4,pRef->cloudFraction,responseHandle);

         	(*pIndexLine)++;

          for (i=0;i<NDET;i++)
           ref[i]+=(double)pEngineContext->buffers.spectrum[i];

          nRec++;
          indexFile=pRef->indexFile;
         }

        if (!alreadyOpen)
         {
          if (pOrbitFile->gome2Pf!=NULL)
           coda_close(pOrbitFile->gome2Pf);

          pOrbitFile->gome2Pf=NULL;
         }
       }
     }
   }

  if (nRec==0)
   rc=ERROR_ID_NO_REF;
  else if (!rc || (rc==ERROR_ID_FILE_RECORD))
   {
   	strcpy(OUTPUT_refFile,gome2OrbitFiles[indexFile].gome2FileName);
   	OUTPUT_nRec=nRec;

    for (i=0;i<NDET;i++)
     ref[i]/=nRec;

    rc=ERROR_ID_NO;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      Gome2RefSelection
// -----------------------------------------------------------------------------
// PURPOSE       Selection of a reference spectrum in the current orbit
//
// INPUT         pEngineContext    collect information on the current spectrum;
//               latMin,latMax determine the range of latitudes;
//               lonMin,lonMax determine the range of longitudes;
//               sza,szaDelta determine the range of SZA;
//               cloudMin,cloudMax determine the range of cloud fraction
//
//               nSpectra     the number of spectra to average to build the reference spectrum;
//               lambdaK,ref  reference spectrum to use if no spectrum in the orbit matches the sza and latitudes conditions;
//
// OUTPUT        lambdaN,refN reference spectrum for northern hemisphere;
//               lambdaS,refS reference spectrum for southern hemisphere.
//
// RETURN        ERROR_ID_ALLOC if the allocation of buffers failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC Gome2RefSelection(ENGINE_CONTEXT *pEngineContext,
                    double latMin,double latMax,
                    double lonMin,double lonMax,
                    double sza,double szaDelta,
                    double cloudMin,double cloudMax,
                    int nSpectra,
                    double *lambdaK,double *ref,
                    double *lambdaN,double *refN,double *pRefNormN,
                    double *lambdaS,double *refS,double *pRefNormS,
                    void *responseHandle)
 {
  // Declarations

  GOME2_REF *refList;                                                            // list of potential reference spectra
  double latDelta,tmp;
  INT nRefN,nRefS;                                                              // number of reference spectra in the previous list resp. for Northern and Southern hemisphere
  INDEX indexLine,indexColumn;
  RC rc;                                                                        // return code

  // Initializations

  mediateResponseRetainPage(plotPageRef,responseHandle);
  *pRefNormN=*pRefNormS=(double)1.;
  indexLine=1;
  indexColumn=2;

  if (latMin>latMax)
   {
   	tmp=latMin;
   	latMin=latMax;
   	latMax=tmp;
   }

  if (lonMin>lonMax)
   {
   	tmp=lonMin;
   	lonMin=lonMax;
   	lonMax=tmp;
   }

  if (cloudMin>cloudMax)
   {
   	tmp=cloudMin;
   	cloudMin=cloudMax;
   	cloudMax=tmp;
   }

  latDelta=(double)fabs(latMax-latMin);
  szaDelta=(double)fabs(szaDelta);
  sza=(double)fabs(sza);

  rc=ERROR_ID_NO;

  memcpy(lambdaN,lambdaK,sizeof(double)*NDET);
  memcpy(lambdaS,lambdaK,sizeof(double)*NDET);

  memcpy(refN,ref,sizeof(double)*NDET);
  memcpy(refS,ref,sizeof(double)*NDET);

  nRefN=nRefS=0;

  // Buffers allocation

  if ((refList=(GOME2_REF *)MEMORY_AllocBuffer("Gome2RefSelection ","refList",gome2TotalRecordNumber,sizeof(GOME2_REF),0,MEMORY_TYPE_STRUCT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // Search for records matching latitudes and SZA conditions

    if (latDelta>EPSILON)                                                       // a latitude range is specified
     {
      // search for potential reference spectra in northern hemisphere

      if ((nRefN=nRefS=Gome2RefLat(refList,gome2TotalRecordNumber,latMin,latMax,lonMin,lonMax,sza,szaDelta,cloudMin,cloudMax))>0)
       rc=Gome2BuildRef(refList,nRefN,nSpectra,lambdaN,refN,pEngineContext,&indexLine,responseHandle);

      if (!rc)
       memcpy(refS,refN,sizeof(double)*NDET);
     }

    // Search for records matching SZA conditions only

    else
     {
      if ((nRefN=nRefS=Gome2RefSza(refList,gome2TotalRecordNumber,sza,szaDelta,cloudMin,cloudMax))>0)
       rc=Gome2BuildRef(refList,nRefN,nSpectra,lambdaN,refN,pEngineContext,&indexLine,responseHandle);

      if (!rc)
       memcpy(refS,refN,sizeof(double)*NDET);
     }

    if (!rc)
     {
      // No reference spectrum is found for both hemispheres -> error message

      if (!nRefN && !nRefS)
       rc=ERROR_SetLast("SciaRefSelection",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"the orbit",pEngineContext->fileInfo.fileName);

      // No reference spectrum found for Northern hemisphere -> use the reference found for Southern hemisphere

      else if (!nRefN)
       {
        mediateResponseCellDataString(plotPageRef,indexLine++,indexColumn,"No record selected for the northern hemisphere, use reference of the southern hemisphere",responseHandle);
        memcpy(refN,refS,sizeof(double)*NDET);
       }

      // No reference spectrum found for Southern hemisphere -> use the reference found for Northern hemisphere

      else if (!nRefS)
       {
       	mediateResponseCellDataString(plotPageRef,indexLine++,indexColumn,"No record selected for the southern hemisphere, use reference of the northern hemisphere",responseHandle);
        memcpy(refS,refN,sizeof(double)*NDET);
       }

      if (nRefN || nRefS)   // if no record selected, use ref (normalized as loaded)
       {
        VECTOR_NormalizeVector(refN-1,NDET,pRefNormN,"Gome2RefSelection (refN) ");
        VECTOR_NormalizeVector(refS-1,NDET,pRefNormS,"Gome2RefSelection (refS) ");
       }
     }
   }

  // Release allocated buffers

  ANALYSE_indexLine=indexLine+1;

  if (refList!=NULL)
   MEMORY_ReleaseBuffer("Gome2RefSelection ","refList",refList);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      Gome2NewRef
// -----------------------------------------------------------------------------
// PURPOSE       In automatic reference selection, search for reference spectra
//
// INPUT         pEngineContext    hold the configuration of the current project
//
// RETURN        ERROR_ID_ALLOC if something failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC Gome2NewRef(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
 {
  // Declarations

  INDEX indexFeno;                                                              // browse analysis windows
  FENO *pTabFeno;                                                               // current analysis window
  RC rc;                                                                        // return code

  // Initializations

//  DEBUG_Print(DOAS_logFile,"Enter Gome2NewRef\n");

  memset(OUTPUT_refFile,0,MAX_PATH_LEN+1);
  OUTPUT_nRec=0;

  memset(pEngineContext->recordInfo.refFileName,0,MAX_PATH_LEN+1);
  pEngineContext->recordInfo.refRecord=ITEM_NONE;

  rc=EngineCopyContext(&ENGINE_contextRef,pEngineContext);

  if (ENGINE_contextRef.recordNumber==0)
   rc=ERROR_ID_ALLOC;
  else

   // Browse analysis windows

   for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
    {
     pTabFeno=&TabFeno[0][indexFeno];

     if ((pTabFeno->hidden!=1) &&
         (pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC) &&
         (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC))

      // Build reference spectra according to latitudes and SZA conditions

      rc=Gome2RefSelection(&ENGINE_contextRef,
                          pTabFeno->refLatMin,pTabFeno->refLatMax,
                          pTabFeno->refLonMin,pTabFeno->refLonMax,
                          pTabFeno->refSZA,pTabFeno->refSZADelta,
                          pTabFeno->cloudFractionMin,pTabFeno->cloudFractionMax,
                          pTabFeno->nspectra,
                          pTabFeno->LambdaK,pTabFeno->Sref,
                          pTabFeno->LambdaN,pTabFeno->SrefN,&pTabFeno->refNormFactN,
                          pTabFeno->LambdaS,pTabFeno->SrefS,&pTabFeno->refNormFactS,
                          responseHandle);
    }

  // Return

//  DEBUG_Print(DOAS_logFile,"End Gome2NewRef %d\n",rc);

  strcpy(pEngineContext->recordInfo.refFileName,ENGINE_contextRef.recordInfo.refFileName);
  pEngineContext->recordInfo.refRecord=ENGINE_contextRef.recordInfo.refRecord;

  return rc;
 }

// ========
// ANALYSIS
// ========

// -----------------------------------------------------------------------------
// FUNCTION      GOME2_LoadAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Load analysis parameters depending on the irradiance spectrum
//
// INPUT         pEngineContext    data on the current file
//
// RETURN        0 for success
// -----------------------------------------------------------------------------

RC GOME2_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
 {
  // Declarations

  GOME2_ORBIT_FILE *pOrbitFile;                                                 // pointer to the current orbit
  INDEX indexFeno,indexTabCross,indexWindow;                                    // indexes for loops and array
  CROSS_REFERENCE *pTabCross;                                                   // pointer to the current cross section
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a symbol
  FENO *pTabFeno;                                                               // pointer to the current spectral analysis window
  double lambdaMin,lambdaMax;                                                   // working variables
  INT DimL,useUsamp,useKurucz,saveFlag;                                         // working variables
  RC rc;                                                                        // return code

  // Initializations

//  DEBUG_Print(DOAS_logFile,"Enter GOME2_LoadAnalysis\n");

  pOrbitFile=&gome2OrbitFiles[gome2CurrentFileIndex];
  saveFlag=(INT)pEngineContext->project.spectra.displayDataFlag;

  if (!(rc=pOrbitFile->rc) && (gome2LoadReferenceFlag || !pEngineContext->analysisRef.refAuto))
   {
    lambdaMin=(double)9999.;
    lambdaMax=(double)-9999.;

    rc=ERROR_ID_NO;
    useKurucz=useUsamp=0;

    // Browse analysis windows and load missing data

    for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
     {
      pTabFeno=&TabFeno[0][indexFeno];
      pTabFeno->NDET=NDET;

      // Load calibration and reference spectra

      if (!pTabFeno->gomeRefFlag)
       {
        memcpy(pTabFeno->LambdaRef,pOrbitFile->gome2SunWve,sizeof(double)*NDET);
        memcpy(pTabFeno->Sref,pOrbitFile->gome2SunRef,sizeof(double)*NDET);

        if (!TabFeno[0][indexFeno].hidden)
         {
          if (!(rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&pTabFeno->refNormFact,"GOME2_LoadAnalysis (Reference) ")))
           {
            memcpy(pTabFeno->SrefEtalon,pTabFeno->Sref,sizeof(double)*pTabFeno->NDET);
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
                   ((rc=ANALYSE_CheckLambda(pWrkSymbol,pTabFeno->LambdaRef,"GOME2_LoadAnalysis "))!=ERROR_ID_NO))

               goto EndGOME2_LoadAnalysis;
             }

            // Gaps : rebuild subwindows on new wavelength scale

            doas_spectrum *new_range = spectrum_new();
            for (indexWindow = 0, DimL=0; indexWindow < pTabFeno->svd.Z; indexWindow++)
             {
              int pixel_start = FNPixel(pTabFeno->LambdaRef,pTabFeno->svd.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
              int pixel_end = FNPixel(pTabFeno->LambdaRef,pTabFeno->svd.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

              spectrum_append(new_range, pixel_start, pixel_end);

              DimL += pixel_end - pixel_start +1;
             }

            // Buffers allocation
            SVD_Free("GOME2_LoadAnalysis",&pTabFeno->svd);
            pTabFeno->svd.DimL=DimL;
            SVD_LocalAlloc("GOME2_LoadAnalysis",&pTabFeno->svd);
            // new spectral windows
            pTabFeno->svd.specrange = new_range;

            pTabFeno->Decomp=1;

            if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,0))!=ERROR_ID_NO) ||
                ((!pKuruczOptions->fwhmFit || !pTabFeno->useKurucz) && pTabFeno->xsToConvolute &&
                ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,&ANALYSIS_slit,&ANALYSIS_slit2,pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2,0))!=ERROR_ID_NO)))

             goto EndGOME2_LoadAnalysis;
           }
         }

        memcpy(pTabFeno->LambdaK,pTabFeno->LambdaRef,sizeof(double)*pTabFeno->NDET);
        memcpy(pTabFeno->Lambda,pTabFeno->LambdaRef,sizeof(double)*pTabFeno->NDET);

        useUsamp+=pTabFeno->useUsamp;
        useKurucz+=pTabFeno->useKurucz;

        if (pTabFeno->useUsamp)
         {
          if (pTabFeno->LambdaRef[0]<lambdaMin)
           lambdaMin=pTabFeno->LambdaRef[0];
          if (pTabFeno->LambdaRef[pTabFeno->NDET-1]>lambdaMax)
           lambdaMax=pTabFeno->LambdaRef[pTabFeno->NDET-1];
         }
       }
     }

    // Wavelength calibration alignment

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
     {
      KURUCZ_Init(0,0);

      if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,responseHandle,0))!=ERROR_ID_NO))
       goto EndGOME2_LoadAnalysis;
     }

    // Build undersampling cross sections

    if (useUsamp && (THRD_id!=THREAD_TYPE_KURUCZ))
     {
      ANALYSE_UsampLocalFree();

      if (((rc=ANALYSE_UsampLocalAlloc(0))!=ERROR_ID_NO) ||
          ((rc=ANALYSE_UsampBuild(0,0))!=ERROR_ID_NO) ||
          ((rc=ANALYSE_UsampBuild(1,ITEM_NONE))!=ERROR_ID_NO))

       goto EndGOME2_LoadAnalysis;
     }

    // Automatic reference selection

    if (gome2LoadReferenceFlag && !(rc=Gome2NewRef(pEngineContext,responseHandle)) &&
       !(rc=ANALYSE_AlignReference(pEngineContext,2,pEngineContext->project.spectra.displayDataFlag,responseHandle,0))) // automatic ref selection for Northern hemisphere
         rc=ANALYSE_AlignReference(pEngineContext,3,pEngineContext->project.spectra.displayDataFlag,responseHandle,0);     // automatic ref selection for Southern hemisphere

    if (!rc)
     gome2LoadReferenceFlag=0;
   }

  // Return

  EndGOME2_LoadAnalysis :

//  DEBUG_Print(DOAS_logFile,"End GOME2_LoadAnalysis %d\n",rc);

  return rc;
 }

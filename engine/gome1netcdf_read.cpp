
//  ----------------------------------------------------------------------------
//! \addtogroup Format
//!
//! @{
//!
//! \file      gome1netcdf_read.cpp
//! \brief     Routines to read GOME (ERS2) data in netCDF format.
//! \details   GOME1 on ERS2 was before delivered in ASCII.  With a converter developed at BIRA-IASB,
//!            we converted them in a binary format (cfr gdp_bin_read.c).\n
//!            Now, the data are also distributed in netCDF format by the DLR. \n
//!            Observations are separated in two groups :
//!            \li MODE_NADIR for ground pixels (in the past, East, Center and West)
//!            \li MODE_NADIR_BACKSCAN for backscan pixels.\n
//! \details   The irradiance can be found in the IRRADIANCE group.  The memory for
//!            type of pixels (East,Center,West and backscan) is planned but only the first
//!            one should be filled.
//! \details   These groups are subdivided in 6 bands with a number of pixels that could be different from one band to the other :   \n
//!                 BAND_1A, BAND_1B, BAND_2A, BAND_2B, BAND_3, BAND_4 \n
//! \details   In each band, the following groups can be found :
//!            \li  GEODATA : %Geolocation coordinates and angles
//!            \li  CLOUDDATA : information on clouds
//!            \li  OBSERVATIONS : with the radiance information
//! \details
//! \authors   Caroline FAYT (qdoas@aeronomie.be)
//! \date      22/01/2018 (creation date)
//! \todo      Date and time functions should be common to satellite formats\n
//!            Complete with general ground-based fields
//! \copyright QDOAS is distributed under GNU General Public License
//!
//! @}
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  GOME1NETCDF_Read_Geodata   load geolocation and angles from the GEODATA group
//  GOME1NETCDF_Read_Clouddata load cloud information from the CLOUDDATA group
//  GOME1NETCDF_Read_Irrad     load the irradiance spectrum from the IRRADIANCE group
//  GOME1NETCDF_Read_Calib     load  the wavelength grid for available detector temperatures
//
//  GOME1NETCDF_Set            open the netCDF file, get the number of records and load metadata variables
//  GOME1NETCDF_Read           read a specified record from a file in netCDF format
//  GOME1NETCDF_Cleanup        close the current file and release allocated buffers
//
//  GOME1NETCDF_get_orbit_date return the date of the current orbit file to create the output directory
//  GOME1NETCDF_LoadAnalysis   load analysis parameters depending on the reference spectrum
//
//  ----------------------------------------------------------------------------
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//        BIRA-IASB
//        Belgian Institute for Space Aeronomy
//        Ringlaan 3 Avenue Circulaire
//        1180     UCCLE
//        BELGIUM
//        qdoas@aeronomie.be
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or (at
//  your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string>
#include <vector>
#include <array>
#include <set>
#include <tuple>
#include <stdexcept>
#include <algorithm>
#include <sstream>

#include <cassert>
#include <cmath>

#include "gome1netcdf_read.h"
#include "netcdfwrapper.h"
#include "dir_iter.h"

extern "C" {
#include "winthrd.h"
#include "comdefs.h"
#include "stdfunc.h"
#include "engine_context.h"
#include "engine.h"
#include "mediate.h"
#include "analyse.h"
#include "spline.h"
#include "vector.h"
#include "zenithal.h"
#include "kurucz.h"
#include "ref_list.h"
}

using std::string;
using std::vector;
using std::set;

#define MAX_GOME_FILES 50                                                       //!< \details Maximum number of files per orbit (in principle, 14/15 should be enough)
#define GOME1NETCDF_NBAND                6                                      //!< \details The number of spectral bands (1A, 1B, 2A, 2B, 3, 4)
#define GOME1NETCDF_DETECTOR_SIZE     1024                                      //!< \details The size of the detector

#define NUM_VZA_REFS                     4                                      //!< \details The number of pixel types (considered here as "rows")

const char *gome1netcdf_bandName[GOME1NETCDF_NBAND]=                            //!< \details Available bands in GOME1 files
 {
  "BAND_1A",
  "BAND_1B",
  "BAND_2A",
  "BAND_2B",
  "BAND_3",
  "BAND_4"
 };

// ======================
// STRUCTURES DEFINITIONS
// ======================

namespace
 {
  //! \struct calib
  //!< \details wavelength grid per detector temperature

  struct calib
   {
    int channel_number;                                                         //!< \details the number of channels
    int channel_size;                                                           //!< \details should be the size of the detector
    int temp_number;                                                            //!< \details the number of considered temperatures

    vector<float> wavelength;                                                   //!< \details wavelength grid
   };

  //! \struct refspec
  //!< \details irradiance spectrum

  struct refspec
   {
    vector<float> ref_spec;                                                     //!< \details the reference spectrum
    vector<float> ref_sigma;                                                    //!< \details error on the reference spectrum
    vector<short> spectral_index;                                               //!< \details index of the spectral wavelength grid (see CALIBRATION/wavelength)
   };

  //! \struct geodata
  //! \brief %Geolocation coordinates and angles present in the GEODATA group of the GOME1 netCDF file.

  struct geodata
   {
    vector<float> sza;                                                          //!< \details Solar zenith angle
    vector<float> sza_sat;                                                      //!< \details Solar zenith angle at satellite
    vector<float> saa;                                                          //!< \details Solar azimuth angle
    vector<float> saa_sat;                                                      //!< \details Solar azimuth angle_sat at satellite
    vector<float> vza;                                                          //!< \details Viewing zenith angle
    vector<float> vza_sat;                                                      //!< \details Viewing zenith angle at satellite
    vector<float> vaa;                                                          //!< \details Viewing azimuth angle
    vector<float> vaa_sat;                                                      //!< \details Viewing azimuth angle at satellite
    vector<float> lon;                                                          //!< \details pixel center longitude
    vector<float> lat;                                                          //!< \details pixel center latitude
    vector<float> alt_sat;                                                      //!< \details Satellite altitude
    vector<float> lat_bounds;                                                   //!< \details The corner coordinate latitudes of each observation
    vector<float> lon_bounds;                                                   //!< \details The corner coordinate longitudes of each observation
    vector<float> earth_rad;                                                    //!< \details The earth radius
   };

  //! \struct clouddata
  //! \brief Information on clouds contained in the CLOUDDATA group of the GOME1 netCDF file.

  struct clouddata
   {
    vector<float> cloud_alb;                                                    //!< \details Cloud albedo
    vector<float> cloud_alb_prec;                                               //!< \details Cloud albedo precision
    vector<float> cloud_frac;                                                   //!< \details Cloud fraction
    vector<float> cloud_frac_prec;                                              //!< \details Cloud fraction precision
    vector<float> cloud_hgt;                                                    //!< \details Cloud height
    vector<float> cloud_hgt_prec;                                               //!< \details Cloud height precision
    vector<float> cloud_pres;                                                   //!< \details Cloud pressure
    vector<float> cloud_pres_prec;                                              //!< \details Cloud pressure precision
    vector<float> surf_hgt;                                                     //!< \details Surface height (kms)

    vector<unsigned char> snow_ice_flag;                                        //!< \details Snow/ice flag (0 : normal)
    vector<unsigned char> sun_glint;                                            //!< \details Possible Sun-glint derived by a geometrical calculation using viewing angles (0 = no, 1 = yes)
   };

  //! \struct GOME1NETCDF_REF
  //! \brief information for automatic reference selection

  typedef struct _gome1netCDF_refSelection {
    double sza;                                                                 //!< \details solar zenith angle
    INDEX  pixelNumber;                                                         //!< \details pixel number
    INDEX  pixelType;                                                           //!< \details pixel type
    double latitude;                                                            //!< \details latitude
    double longitude;                                                           //!< \details longitude
    double cloudFraction;                                                       //!< \details cloud fraction
  }
  GOME1NETCDF_REF;

  //! \struct GOME1NETCDF_ORBIT_FILE
  //! \brief description of an orbit in GOME1 netCDF format

  typedef struct _gome1netCDF_orbitFiles
  {
    char fileName[MAX_STR_LEN+1];                                               //!< \details the name of the file with a part of the orbit
    NetCDFFile current_file;                                                    //!< \details Pointer to the current netCDF file
    GOME1NETCDF_REF *refInfo[4];                                                //!< \details the minimum information useful for automatic reference selection
    string root_name;                                                           //!< \details The name of the root (should be the basename of the file)
    int refNum[4];                                                              //!< \details number of reference spectra

    geodata ground_geodata;                                                     //!< \details Keep the geolocation data and angles content from the MODE_NADIR group as far as the netCDF file is open
    geodata backscan_geodata;                                                   //!< \details Keep the geolocation data and angles content from the MODE_NADIR_BACKSCAN group as far as the netCDF file is open
    clouddata ground_clouddata;                                                 //!< \details Keep the cloud information content from the MODE_NADIR group as far as the netCDF file is open
    clouddata backscan_clouddata;                                               //!< \details Keep the cloud information content from the MODE_NADIR_BACKSCAN group as far as the netCDF file is open
    calib calibration;                                                          //!< \details Keep information on the wavelength grids as far as the netCDF file is open
    refspec irradiance;                                                         //!< \details Keep information on the irradiance spectrum as far as the netCDF file is open

    size_t det_size;                                                            //!< \details The current detector size
    size_t scan_size;                                                           //!< \details The number of lines in the MODE_NADIR group
    size_t scan_size_bs;                                                        //!< \details The number of lines in the MODE_NADIR_BACKSCAN group
    size_t pixel_size;                                                          //!< \details The number of ground pixels in one scan line (3 for the MODE_NADIR group)
    size_t pixel_size_bs;                                                       //!< \details The number of backscan pixels in one scan line (1 for the MODE_NADIR_BACKSCAN group)
    int start_pixel;                                                            //!< \details start pixel in the channel (depend on the selected band)

    vector<int>scanline_indexes;
    vector<int>scanline_pixtype;
    vector<int>scanline_pixnum;
    vector<int>alongtrack_indexes;
    vector<double> delta_time; // number of milliseconds after reference_time
    time_t reference_time;

    string mode;                                                                  //!< \details mode MODE_NADIR or MODE_NARROW_SWATH
    string mode_bs;                                                               //!< \details mode MODE_NADIR_BACKSCAN or MODE_NARROW_SWATH_BACKSCAN

    // GDP_BIN_INFO *gdpBinInfo;                                                     //!< \details useful information on records for fast access
    // INDEX gdpBinBandIndex;                                                        //!< \details indexes of bands present in the current file
    // unsigned short             *gdpBinReference,                                  //!< \details      // buffer for irradiance spectra
    //                    *gdpBinRefError;                                           //!< \details errors on irradiance spectra
    int specNumber;                                                               //!< \details total number of spectra in the file
    int n_alongtrack;                                                             //!< \details total number of spectra per row (pixel type)
    RC rc;
  }
  GOME1NETCDF_ORBIT_FILE;
 }    // end namespace

// ================
// STATIC VARIABLES
// ================

// references for each pixel type and for each analysis window;
static struct reference (*vza_refs)[NUM_VZA_REFS];

static GOME1NETCDF_ORBIT_FILE gome1netCDF_orbitFiles[MAX_GOME_FILES];           //!< \details list of files for an orbit

static int gome1netCDF_currentFileIndex=ITEM_NONE;                              //!< \details index of the current file in the list
static int gome1netCDF_loadReferenceFlag=0;                                     //!< \details 1 for analysis with automatic reference selection
static int gome1netCDF_orbitFilesN=0;                                           //!< \details the total number of files for the current day
static int gome1netCDF_totalRecordNumber=0;                                       //!< \details total number of records for an orbit

static int channel_index;                                                       //!< \details For the irradiance, the channel number (depend on the selected band)


// replace by functions using QDateTime?
static void getDate(GOME1NETCDF_ORBIT_FILE *pOrbitFile,double delta_t, struct datetime *date_time) {
  // TODO: handle UTC leap seconds?  Is this possible? (no UTC time info in file?)
  time_t thetime = pOrbitFile->reference_time + (int)floor(delta_t); //1000;

  struct date *pDate = &date_time->thedate;
  struct time *pTime = &date_time->thetime;

  struct tm thedate;
#ifndef _WIN32
  gmtime_r(&thetime, &thedate);
#else
  struct tm *time = gmtime(&thetime);
  thedate = *time;
#endif

  pDate->da_year = thedate.tm_year + 1900;
  pDate->da_mon = thedate.tm_mon + 1; // month, from 1 to 12
  pDate->da_day = thedate.tm_mday;

  pTime->ti_hour = thedate.tm_hour;
  pTime->ti_min = thedate.tm_min;
  pTime->ti_sec = thedate.tm_sec;

  date_time->millis = static_cast<int>((delta_t-floor(delta_t))*1000.+0.1); // % 1000;
}

static void set_reference_time(GOME1NETCDF_ORBIT_FILE *pOrbitFile,const string& utc_date) {
  int year,month,day;
  std::istringstream utc(utc_date);

  utc >> year;
  utc.ignore(1,'-');
  utc >> month;
  utc.ignore(1,'-');
  utc >> day;

  struct tm t = {
    0,  // seconds of minutes from 0 to 61
    0,  // minutes of hour from 0 to 59
    0,  // hours of day from 0 to 24
    day,  // day of month from 1 to 31
    month - 1,  // month of year from 0 to 11
    year - 1900, // year since 1900
    0,  // days since sunday
    0,  // days since January 1st
    0, // have daylight savings time?
#if defined(__GNUC__) && !defined(__MINGW32__) // initialize extra fields available in GCC but not in MinGW32
    0, // Seconds east of UTC
    0  // Timezone abbreviation
#endif
  };

   // get number of seconds since 1/1/1970, UTC
  pOrbitFile->reference_time = STD_timegm(&t);
}

// -----------------------------------------------------------------------------
// FUNCTION GOME1NETCDF_Read_Geodata
// -----------------------------------------------------------------------------
//!
//! \fn      static geodata GOME1NETCDF_Read_Geodata(NetCDFGroup geodata_group,size_t scan_size,size_t pixel_size)
//! \details Load geolocation and angles from the GEODATA group \n
//! \param   [in]  geodata_group the netCDF group to read (MODE_NADIR or MODE_NADIR_BACKSCAN
//! \param   [in]  scan_size      the number of scanlines
//! \return  [in]  pixel_size  the size of vectors in the pixel direction (3 for ground pixels, 1 for backscans)
//! \return  : a \a geodata structure with all the GEODATA variables
//!
// -----------------------------------------------------------------------------

static geodata GOME1NETCDF_Read_Geodata(NetCDFGroup geodata_group,size_t scan_size,size_t pixel_size)
 {
  // Declarations

  geodata result;                                                               // geodata
  const size_t start[] = {0,0,0,0};                                             // there is no reason not to start from 0, the presence of the fourth dimension depends on variables
  size_t count[] = {1,scan_size,pixel_size,1};                                  // the presence of the fourth dimension depends on variables

  // Get geodata variables

  geodata_group.getVar("earth_radius",start,count,3,(float)-1.,result.earth_rad);
  geodata_group.getVar("latitude",start,count,3,(float)-1.,result.lat);
  geodata_group.getVar("longitude",start,count,3,(float)-1.,result.lon);
  geodata_group.getVar("satellite_altitude",start,count,3,(float)-1.,result.alt_sat);

  // Corner coordinates

  count[3]=4;

  geodata_group.getVar("latitude_bounds",start,count,4,(float)-1.,result.lat_bounds);
  geodata_group.getVar("longitude_bounds",start,count,4,(float)-1.,result.lon_bounds);

  // Angles

  count[3]=3;

  geodata_group.getVar("solar_zenith_angle",start,count,4,(float)-1.,result.sza);
  geodata_group.getVar("solar_zenith_angle_sat",start,count,4,(float)-1.,result.sza_sat);
  geodata_group.getVar("solar_azimuth_angle",start,count,4,(float)-1.,result.saa);
  geodata_group.getVar("solar_azimuth_angle_sat",start,count,4,(float)-1.,result.saa_sat);
  geodata_group.getVar("viewing_zenith_angle",start,count,4,(float)-1.,result.vza);
  geodata_group.getVar("viewing_zenith_angle_sat",start,count,4,(float)-1.,result.vza_sat);
  geodata_group.getVar("viewing_azimuth_angle",start,count,4,(float)-1.,result.vaa);
  geodata_group.getVar("viewing_azimuth_angle_sat",start,count,4,(float)-1.,result.vaa_sat);

  // Return

  return result;
 }

// -----------------------------------------------------------------------------
// FUNCTION GOME1NETCDF_Read_Clouddata
// -----------------------------------------------------------------------------
//!
//! \fn      static geodata GOME1NETCDF_Read_Clouddata(NetCDFGroup geodata_group,size_t scan_size,size_t pixel_size)
//! \details Load cloud information from the CLOUDDATA group
//! \param   [in]  clouddata_group the netCDF group to read (MODE_NADIR or MODE_NADIR_BACKSCAN
//! \param   [in]  scan_size        the number of scanlines
//! \return  [in]  pixel_size    the size of vectors in the pixel direction (3 for ground pixels, 1 for backscans)
//! \return  : a \a clouddata structure with all the CLOUDDATA variables
//!
// -----------------------------------------------------------------------------

static clouddata GOME1NETCDF_Read_Clouddata(NetCDFGroup clouddata_group,size_t scan_size,size_t pixel_size)
 {
  // Declarations

  clouddata result;                                                             // clouddata
  const size_t start[] = {0,0,0};                                               // there is no reason not to start from 0, the presence of the fourth dimension depends on variables
  const size_t count[] = {1,scan_size,pixel_size};                              // the presence of the fourth dimension depends on variables

  // Get geodata variables

  clouddata_group.getVar("cloud_albedo",start,count,3,(float)-1.,result.cloud_alb);
  clouddata_group.getVar("cloud_albedo_precision",start,count,3,(float)-1.,result.cloud_alb_prec);
  clouddata_group.getVar("cloud_fraction",start,count,3,(float)-1.,result.cloud_frac);
  clouddata_group.getVar("cloud_fraction_precision",start,count,3,(float)-1.,result.cloud_frac_prec);
  clouddata_group.getVar("cloud_height",start,count,3,(float)-1.,result.cloud_hgt);
  clouddata_group.getVar("cloud_height_precision",start,count,3,(float)-1.,result.cloud_hgt_prec);
  clouddata_group.getVar("cloud_pressure",start,count,3,(float)-1.,result.cloud_pres);
  clouddata_group.getVar("cloud_pressure_precision",start,count,3,(float)-1.,result.cloud_pres_prec);
  clouddata_group.getVar("snow_ice_flag",start,count,3,(unsigned char)0,result.snow_ice_flag);
  clouddata_group.getVar("sun_glint",start,count,3,(unsigned char)0,result.sun_glint);
  clouddata_group.getVar("surface_height",start,count,3,(float)-1.,result.surf_hgt);

  // Return

  return result;
 }

// -----------------------------------------------------------------------------
// FUNCTION GOME1NETCDF_Read_Calib
// -----------------------------------------------------------------------------
//!
//! \fn      static geodata GOME1NETCDF_Read_Calib(NetCDFGroup calib_group)
//! \details Load the wavelength grid for available detector temperatures
//! \param   [in]  calib_group      the netCDF group from which to retrieve the calibration grid
//! \return  : a calib structure with the grid of wavelengths
//!
// -----------------------------------------------------------------------------

static calib GOME1NETCDF_Read_Calib(NetCDFGroup calib_group)
 {
  // Declarations

  calib   result;                                                               // wavelength grids

  // Get dimensions of variables in the CALIBRATION group

  result.channel_size=calib_group.dimLen("channel_pixels");                     // should be the size of the detector
  result.temp_number=calib_group.dimLen("detector_temperature");                // the number of considered temperatures

  result.channel_number=calib_group.dimLen("total_detector_pixels")/result.channel_size;            // the number of channels

  const   size_t start[] = {0,0,0};
  const   size_t count[] = {(size_t)result.temp_number,(size_t)result.channel_number,(size_t)result.channel_size};

  // Get the wavelength grid per detector temperature

  calib_group.getVar("wavelength",start,count,3,(float)0.,result.wavelength);

  // Return

  return result;
 }

// -----------------------------------------------------------------------------
// FUNCTION GOME1NETCDF_Get_Wavelength
// -----------------------------------------------------------------------------
//!
//! \fn      void  GOME1NETCDF_Get_Wavelength(ENGINE_CONTEXT *pEngineContext,int channel_index,int temp_index,double *wavelength)
//! \details Load  get information on the irradiance from the IRRADIANCE group
//! \param   [in]  channel_index  the index of the channel
//! \param   [in]  temp_index     the index of the temperature (called spectral_index in the netCDF file)
//! \param   [out] wavelength     the wavelength grid for the specified temperature
//!
// -----------------------------------------------------------------------------

void GOME1NETCDF_Get_Wavelength(GOME1NETCDF_ORBIT_FILE *pOrbitFile,int channel_index,int temp_index,double *wavelength)
 {
  auto wve = reinterpret_cast<const float(*)[pOrbitFile->calibration.channel_number][pOrbitFile->calibration.channel_size]>(pOrbitFile->calibration.wavelength.data());

  if ((temp_index>=0) && (temp_index<pOrbitFile->calibration.temp_number) && (channel_index>=0) && (channel_index<pOrbitFile->calibration.channel_size))
   for (int i=0;i<(int)pOrbitFile->calibration.channel_size;i++)
    wavelength[i]=wve[temp_index][channel_index][i];
 }

// -----------------------------------------------------------------------------
// FUNCTION GOME1NETCDF_Read_Irrad
// -----------------------------------------------------------------------------
//!
//! \fn      static refspec GOME1NETCDF_Read_Irrad(NetCDFGroup irrad_group)
//! \details Get information on the irradiance from the IRRADIANCE group
//! \param   [in]  pEngineContext   pointer to the engine context
//! \param   [in]  irrad_group      the netCDF group from which to retrieve the irradiance
//! \param   [in]  channel_index    index of the current channel
//! \return  : a refspec structure with all the irradiance variables
//!
// -----------------------------------------------------------------------------

static refspec GOME1NETCDF_Read_Irrad(NetCDFGroup irrad_group,int channelIndex)
 {
  // Declarations

  refspec result;                                                               // irradiance information
  int     irrad_size=irrad_group.dimLen("spectral_channel");                    // should be the size of the detector
  const   size_t start[] = {(size_t)channelIndex,0};
  const   size_t count[] = {1,(size_t)irrad_size};

  // Get information on the irradiance

  irrad_group.getVar("irradiance",start,count,2,(float)0.,result.ref_spec);
  irrad_group.getVar("irradiance_precision",start,count,2,(float)0.,result.ref_sigma);
  irrad_group.getVar("spectral_index",start,count,1,(short)0,result.spectral_index);

  // Return

  return result;
 }

// -----------------------------------------------------------------------------
// FUNCTION GOME1NETCDF_Get_Irradiance
// -----------------------------------------------------------------------------
//!
//! \fn      void GOME1NETCDF_Get_Irradiance (ENGINE_CONTEXT *pEngineContext,int channel_index,double *wavelength,double *irrad)
//! \details get irradiance from the IRRADIANCE group and its calibration from the CALIBRATION group
//! \channel [in]  channel_index the index of the requested channel
//! \param   [out] wavelength    the wavelength grid of the irradiance spectrum
//! \param   [out] irrad         the irradiance spectrum
//!
// -----------------------------------------------------------------------------

void GOME1NETCDF_Get_Irradiance(GOME1NETCDF_ORBIT_FILE *pOrbitFile,int channel_index,double *wavelength,double *irrad)
 {
  GOME1NETCDF_Get_Wavelength(pOrbitFile,channel_index,(int)pOrbitFile->irradiance.spectral_index[0],wavelength);

  for (int i=0;i<(int)pOrbitFile->calibration.channel_size;i++)
   irrad[i]=(double)pOrbitFile->irradiance.ref_spec[i];
 }

// -----------------------------------------------------------------------------
// FUNCTION get_ref_info
// -----------------------------------------------------------------------------
//!
//! \fn      RC get_ref_info(GOME1NETCDF_ORBIT_FILE *pOrbitFile,GOME1NETCDF_REF *ref_list)
//! \details Browse spectra of the current orbit file and collect information for the automatic reference selection
//! \param   [out] ref_list  the information on spectra useful for the automatic reference selection\n
//! \param   [in]  pOrbitFile : pointer to the current orbit file\n
//!
// -----------------------------------------------------------------------------

static void get_ref_info(GOME1NETCDF_ORBIT_FILE *pOrbitFile)
 {
  // Global declarations

  GOME1NETCDF_REF *refList,*pRef;
  geodata geo;
  int pixelSize;

  // Declare substition variables for ground pixels

  geo=pOrbitFile->ground_geodata;                                               // do not work ???
  pixelSize=3;

  auto sza_gr = reinterpret_cast<const float(*)[pixelSize][3]>(pOrbitFile->ground_geodata.sza.data());
  auto lat_gr =  reinterpret_cast<const float(*)[pixelSize]>(pOrbitFile->ground_geodata.lat.data());
  auto lon_gr =  reinterpret_cast<const float(*)[pixelSize]>(pOrbitFile->ground_geodata.lon.data());
  auto cloud_gr =  reinterpret_cast<const float(*)[pixelSize]>(pOrbitFile->ground_clouddata.cloud_frac.data());


  // Declare substition variables for backscan pixels

  geo=pOrbitFile->backscan_geodata;
  pixelSize=1;

  auto sza_bs = reinterpret_cast<const float(*)[pixelSize][3]>(pOrbitFile->backscan_geodata.sza.data());
  auto lat_bs =  reinterpret_cast<const float(*)[pixelSize]>(pOrbitFile->backscan_geodata.lat.data());
  auto lon_bs =  reinterpret_cast<const float(*)[pixelSize]>(pOrbitFile->backscan_geodata.lon.data());

  auto cloud_bs =  reinterpret_cast<const float(*)[pixelSize]>(pOrbitFile->backscan_clouddata.cloud_frac.data());

//   {
//     FILE *fp;
//     fp=fopen("toto.dat","a+t");

  for (int i=0;i<pOrbitFile->specNumber;i++)
   {
    // Declarations

    size_t scanIndex=pOrbitFile->scanline_indexes[i];                           // index in the ground pixel scanlines or backscan scanlines
    int    pixelType=pOrbitFile->scanline_pixtype[i];                           // pixel type
    size_t pixelIndex=(pixelType==3)?0:pixelType;                               // index of the pixel in the scan : should be 0,1,2 for ground pixels and 0 for backscans

    // Get useful information for automatic reference selection

    if (pOrbitFile->refInfo[pixelType]!=NULL)
     {
      refList=pOrbitFile->refInfo[pixelType];
      pRef=&refList[pOrbitFile->refNum[pixelType]];

      pRef->pixelType=pixelType;
      pRef->sza=(pixelType==3)?sza_bs[scanIndex][pixelIndex][1]:sza_gr[scanIndex][pixelIndex][1];
      pRef->latitude=(pixelType==3)?lat_bs[scanIndex][pixelIndex]:lat_gr[scanIndex][pixelIndex];
      pRef->longitude=(pixelType==3)?lon_bs[scanIndex][pixelIndex]:lon_gr[scanIndex][pixelIndex];
      pRef->cloudFraction=(pixelType==3)?cloud_bs[scanIndex][pixelIndex]:cloud_gr[scanIndex][pixelIndex];

      pOrbitFile->refNum[pixelType]=pOrbitFile->refNum[pixelType]+1;

  //    if (i>=970 && i<=990)
  //     fprintf(fp,"+++ %d %d %d %g %g %g\n",(i+1),pixelType,pixelIndex,pRef->latitude,pRef->longitude,pRef->sza);
     }
   }


  //   fclose(fp);
  //  }
 }

// -----------------------------------------------------------------------------
// FUNCTION GOME1NETCDF_Set
// -----------------------------------------------------------------------------
//!
//! \fn      RC GOME1NETCDF_Set(ENGINE_CONTEXT *pEngineContext)
//! \details Open the netCDF file, get the number of records, load metadata variables and irradiance.\n
//! \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.\n
//! \return  ERROR_ID_NETCDF on run time error (opening of the file didn't succeed, missing variable...)\n
//!          ERROR_ID_NO on success
//!
// -----------------------------------------------------------------------------

RC GOME1NETCDF_Set(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  GOME1NETCDF_ORBIT_FILE *pOrbitFile;
  PRJCT_INSTRUMENTAL *pInstrumental;
  NetCDFGroup root_group;
  NetCDFGroup band_group;
  NetCDFGroup geodata_group;
  NetCDFGroup clouddata_group;
  NetCDFGroup obs_group;
  NetCDFGroup irrad_group;
  NetCDFGroup calib_group;
  RC rc = ERROR_ID_NO;
  int selected_band;
  vector<int> scanline;
  vector<int> scanline_bs;
  vector <double> deltatime;
  vector <double> deltatime_bs;
  vector <short> startpixel;
  int *iscan,*iscan_bs,maxscan;
  int i,j,k,n;

  // Initializations

  pInstrumental=&pEngineContext->project.instrumental;
  selected_band=pInstrumental->gomenetcdf.bandType;

  // Define the channel index from the requested type of band

  if ((selected_band==PRJCT_INSTR_GDP_BAND_1A) || (selected_band==PRJCT_INSTR_GDP_BAND_1B))
   channel_index=0;
  else if ((selected_band==PRJCT_INSTR_GDP_BAND_2A) || (selected_band==PRJCT_INSTR_GDP_BAND_2B))
   channel_index=1;
  else if (selected_band==PRJCT_INSTR_GDP_BAND_3)
   channel_index=2;
  else if (selected_band==PRJCT_INSTR_GDP_BAND_4)
   channel_index=3;
  else
   channel_index=2;

  pEngineContext->recordNumber=0;
  gome1netCDF_loadReferenceFlag=0;
  gome1netCDF_currentFileIndex=ITEM_NONE;

  // In automatic reference selection, the file has maybe already loaded

  if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto)
   {
    int indexFile = 0;
    for (;indexFile<gome1netCDF_orbitFilesN;indexFile++)
      if (!strcasecmp(pEngineContext->fileInfo.fileName,gome1netCDF_orbitFiles[indexFile].fileName) )
        break;

    if (indexFile<gome1netCDF_orbitFilesN)
      gome1netCDF_currentFileIndex=indexFile;
   }

  // File has not been loaded already -> load a new file.

  if (gome1netCDF_currentFileIndex==ITEM_NONE)
   {
    // Release old buffers
    GOME1NETCDF_Cleanup();

    if ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto)
     {
      gome1netCDF_loadReferenceFlag=1;

      char filePath[MAX_STR_SHORT_LEN+1];
      strcpy(filePath,pEngineContext->fileInfo.fileName);

      char *ptr = strrchr(filePath, PATH_SEP);
      if (ptr == NULL)
       strcpy(filePath,".");
      else
       *ptr = '\0';

      DIR *hDir=opendir(filePath);
      struct dirent *fileInfo = NULL;
      while (hDir!=NULL && ((fileInfo=readdir(hDir))!=NULL) )
       {
        sprintf(gome1netCDF_orbitFiles[gome1netCDF_orbitFilesN].fileName,"%s/%s",filePath,fileInfo->d_name);
        if (!STD_IsDir(gome1netCDF_orbitFiles[gome1netCDF_orbitFilesN].fileName))
          ++gome1netCDF_orbitFilesN;
       }

      if (hDir != NULL)
        closedir(hDir);
     }

    if (!gome1netCDF_orbitFilesN)
     {
      gome1netCDF_orbitFilesN=1;
      strcpy(gome1netCDF_orbitFiles[0].fileName,pEngineContext->fileInfo.fileName);
     }

    // Load files

    gome1netCDF_totalRecordNumber=0;
    for (int indexFile=0;(indexFile<gome1netCDF_orbitFilesN) && !rc;indexFile++)
     {
      pOrbitFile=&gome1netCDF_orbitFiles[indexFile];
      pOrbitFile->specNumber=0;
      pOrbitFile->rc=0;
      iscan=iscan_bs=NULL;

      // Try to open the file and load metadata

      try
       {
        pOrbitFile->current_file = NetCDFFile(pOrbitFile->fileName,NC_NOWRITE);                 // open file
        pOrbitFile->root_name = pOrbitFile->current_file.getName();                             // get the root name (should be the file name)
        root_group = pOrbitFile->current_file.getGroup(pOrbitFile->root_name);                  // go to the root
        set_reference_time(pOrbitFile,pOrbitFile->current_file.getAttText("time_reference"));   // get the reference time

        pOrbitFile->scan_size=
        pOrbitFile->scan_size_bs=
        pOrbitFile->pixel_size=
        pOrbitFile->pixel_size_bs=(size_t)0;

        // Load the wavelengths grids

        calib_group=pOrbitFile->current_file.getGroup(pOrbitFile->root_name+"/CALIBRATION");
        pOrbitFile->calibration=GOME1NETCDF_Read_Calib(calib_group);

        // Load the irradiance spectrum

        irrad_group=pOrbitFile->current_file.getGroup(pOrbitFile->root_name+"/IRRADIANCE");
        pOrbitFile->irradiance=GOME1NETCDF_Read_Irrad(irrad_group,channel_index);

        // MODE_NADIR or MODE_NARROW_SWATH ?

        if (pOrbitFile->current_file.groupID(pOrbitFile->root_name+"/MODE_NADIR/"+gome1netcdf_bandName[selected_band])!=-1)
         pOrbitFile->mode="/MODE_NADIR/";
        else if (pOrbitFile->current_file.groupID(pOrbitFile->root_name+"/MODE_NARROW_SWATH/"+gome1netcdf_bandName[selected_band])!=-1)
         pOrbitFile->mode="/MODE_NARROW_SWATH/";
        else
         pOrbitFile->mode="";

        if (pOrbitFile->current_file.groupID(pOrbitFile->root_name+"/MODE_NADIR_BACKSCAN/"+gome1netcdf_bandName[selected_band])!=-1)
         pOrbitFile->mode_bs="/MODE_NADIR_BACKSCAN/";
        else if (pOrbitFile->current_file.groupID(pOrbitFile->root_name+"/MODE_NARROW_SWATH_BACKSCAN/"+gome1netcdf_bandName[selected_band])!=-1)
         pOrbitFile->mode_bs="/MODE_NARROW_SWATH_BACKSCAN/";
        else
         pOrbitFile->mode_bs="";

        if (!pOrbitFile->mode.length() && pOrbitFile->mode_bs.length())
         continue;

             // Dimensions of spectra are 'time' x 'scan_size' x 'pixel_size ' x 'spectral_channel'
             // For example : 1 x 552 x 3 x 832

        // Read ground pixels geodata and clouddata

        if (pInstrumental->gomenetcdf.pixelType!=PRJCT_INSTR_GOME1_PIXEL_BACKSCAN)   // if not backscan pixels only
         {
          band_group = pOrbitFile->current_file.getGroup(pOrbitFile->root_name+pOrbitFile->mode+gome1netcdf_bandName[selected_band]);

          pEngineContext->project.instrumental.use_row[0]=
          pEngineContext->project.instrumental.use_row[1]=
          pEngineContext->project.instrumental.use_row[2]=true;

          if ((band_group.dimLen("time")!=1) || (band_group.dimLen("ground_pixel")!=3))
           pOrbitFile->rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_FORMAT, "Dimensions of ground pixels in the GOME1 netCDF file are not the expected ones");  // in case of error, capture the message
          else
           {
            // Get the different groups

            geodata_group = pOrbitFile->current_file.getGroup(pOrbitFile->root_name+pOrbitFile->mode+gome1netcdf_bandName[selected_band]+"/GEODATA");
            clouddata_group = pOrbitFile->current_file.getGroup(pOrbitFile->root_name+pOrbitFile->mode+gome1netcdf_bandName[selected_band]+"/CLOUDDATA");
            obs_group = pOrbitFile->current_file.getGroup(pOrbitFile->root_name+pOrbitFile->mode+gome1netcdf_bandName[selected_band]+"/OBSERVATIONS");

            // Get the scanline and pixel size for ground pixel

            pOrbitFile->scan_size=band_group.dimLen("scanline");
            pOrbitFile->pixel_size=band_group.dimLen("ground_pixel");
            pOrbitFile->det_size=band_group.dimLen("spectral_channel");

            // Read the metadata

            pOrbitFile->ground_geodata=GOME1NETCDF_Read_Geodata(geodata_group,pOrbitFile->scan_size,pOrbitFile->pixel_size);
            pOrbitFile->ground_clouddata=GOME1NETCDF_Read_Clouddata(clouddata_group,pOrbitFile->scan_size,pOrbitFile->pixel_size);

            // Get the scanline indexes

            const size_t start[] = {0,0,0};
            const size_t count[] = {1,pOrbitFile->scan_size,pOrbitFile->pixel_size};

            obs_group.getVar("scanline",start,count,2,(int)-1,scanline);
            obs_group.getVar("delta_time",start,count,3,(double)0.,deltatime);
            band_group.getVar("start_pixel",start,count,1,(short)0,startpixel);
           }
         }

        // Read backscans geodata and clouddata

        if (pInstrumental->gomenetcdf.pixelType!=PRJCT_INSTR_GOME1_PIXEL_GROUND)     // if not ground pixels only
         {
          band_group = pOrbitFile->current_file.getGroup(pOrbitFile->root_name+pOrbitFile->mode_bs+gome1netcdf_bandName[selected_band]);
          pEngineContext->project.instrumental.use_row[3]=true;

          if ((band_group.dimLen("time")!=1) || (band_group.dimLen("ground_pixel")!=1))
           pOrbitFile->rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_FORMAT, "Dimensions of backscan pixels in the GOME1 netCDF file are not the expected ones");  // in case of error, capture the message
          else
           {
            // Get the different groups

            geodata_group = pOrbitFile->current_file.getGroup(pOrbitFile->root_name+pOrbitFile->mode_bs+gome1netcdf_bandName[selected_band]+"/GEODATA");
            clouddata_group = pOrbitFile->current_file.getGroup(pOrbitFile->root_name+pOrbitFile->mode_bs+gome1netcdf_bandName[selected_band]+"/CLOUDDATA");
            obs_group = pOrbitFile->current_file.getGroup(pOrbitFile->root_name+pOrbitFile->mode_bs+gome1netcdf_bandName[selected_band]+"/OBSERVATIONS");

            // Get the scanline and pixel size for backscans

            pOrbitFile->scan_size_bs=band_group.dimLen("scanline");
            pOrbitFile->pixel_size_bs=band_group.dimLen("ground_pixel");
            pOrbitFile->det_size=band_group.dimLen("spectral_channel");

            // Read the metadata

            pOrbitFile->backscan_geodata=GOME1NETCDF_Read_Geodata(geodata_group,pOrbitFile->scan_size_bs,pOrbitFile->pixel_size_bs);
            pOrbitFile->backscan_clouddata=GOME1NETCDF_Read_Clouddata(clouddata_group,pOrbitFile->scan_size_bs,pOrbitFile->pixel_size_bs);

            // Get the scanline indexes

            const size_t start[] = {0,0,0};
            const size_t count[] = {1,pOrbitFile->scan_size_bs,pOrbitFile->pixel_size_bs};

            obs_group.getVar("scanline",start,count,2,(int)-1,scanline_bs);
            obs_group.getVar("delta_time",start,count,3,(double)0.,deltatime_bs);
            band_group.getVar("start_pixel",start,count,1,(short)0,startpixel);
           }
         }

        // Assign size and allocate buffers to keep information as long as the file is open

        pOrbitFile->specNumber=pOrbitFile->scan_size*pOrbitFile->pixel_size+pOrbitFile->scan_size_bs*pOrbitFile->pixel_size_bs;  // get the total number of records (ground pixels + backscans)

        if  ((THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto &&

           (((pInstrumental->gomenetcdf.pixelType!=PRJCT_INSTR_GOME1_PIXEL_BACKSCAN) &&
           (((pOrbitFile->refInfo[0]=(GOME1NETCDF_REF *)MEMORY_AllocBuffer(__func__,"refInfo[0]",pOrbitFile->specNumber,sizeof(GOME1NETCDF_REF),0,MEMORY_TYPE_STRUCT))==NULL) ||
            ((pOrbitFile->refInfo[1]=(GOME1NETCDF_REF *)MEMORY_AllocBuffer(__func__,"refInfo[1]",pOrbitFile->specNumber,sizeof(GOME1NETCDF_REF),0,MEMORY_TYPE_STRUCT))==NULL) ||
            ((pOrbitFile->refInfo[2]=(GOME1NETCDF_REF *)MEMORY_AllocBuffer(__func__,"refInfo[2]",pOrbitFile->specNumber,sizeof(GOME1NETCDF_REF),0,MEMORY_TYPE_STRUCT))==NULL)))  ||

            ((pInstrumental->gomenetcdf.pixelType!=PRJCT_INSTR_GOME1_PIXEL_GROUND) &&
            ((pOrbitFile->refInfo[3]=(GOME1NETCDF_REF *)MEMORY_AllocBuffer(__func__,"refInfo[3]",pOrbitFile->specNumber,sizeof(GOME1NETCDF_REF),0,MEMORY_TYPE_STRUCT))==NULL))))

         rc= pOrbitFile->rc = ERROR_ID_ALLOC;  // in case of error, capture the message
        else
         {
          pOrbitFile->start_pixel=startpixel[0];

          pOrbitFile->scanline_indexes.resize(pOrbitFile->specNumber);
          pOrbitFile->scanline_pixtype.resize(pOrbitFile->specNumber);
          pOrbitFile->scanline_pixnum.resize(pOrbitFile->specNumber);
          pOrbitFile->alongtrack_indexes.resize(pOrbitFile->specNumber);
          pOrbitFile->delta_time.resize(pOrbitFile->specNumber);

          auto delta_time_scan=reinterpret_cast<const double(*)[pOrbitFile->pixel_size]>(deltatime.data());
          auto delta_time_scan_bs = reinterpret_cast<const double(*)[pOrbitFile->pixel_size_bs]>(deltatime_bs.data());

          pOrbitFile->n_alongtrack=0;

          // Get the maximum scanline number

          maxscan=0;
          if (pOrbitFile->scan_size && (scanline[(int)pOrbitFile->scan_size-1]>maxscan))
           maxscan=scanline[(int)pOrbitFile->scan_size-1];
          if (pOrbitFile->scan_size_bs && (scanline_bs[(int)pOrbitFile->scan_size_bs-1]>maxscan))
           maxscan=scanline_bs[(int)pOrbitFile->scan_size_bs-1];
          maxscan++;

          // Allocate scanline buffers to sort ground pixels and backscans on the scanline number

          if (((iscan=(int *)malloc(sizeof(int)*maxscan))==NULL) ||
              ((iscan_bs=(int *)malloc(sizeof(int)*maxscan))==NULL))

           rc=ERROR_ID_ALLOC;
          else
           {
            // Initialize scanline buffers

            for (i=0;i<maxscan;i++)
             iscan[i]=iscan_bs[i]=ITEM_NONE;

            // Consider ground pixels and fill scanline buffer with the indexes of ground pixels

            if (pOrbitFile->scan_size)
             for (i=0;i<(int)pOrbitFile->scan_size;i++)
              iscan[scanline[i]]=i;

            // Consider backscan pixels and fill scanline buffer with the indexes of backscan pixels

            if (pOrbitFile->scan_size_bs)
             for (i=0;i<(int)pOrbitFile->scan_size_bs;i++)
              iscan_bs[scanline_bs[i]]=i;

            // The following loop browse scanline and sort them

            for (i=k=0;i<maxscan;i++)
             {
              // Consider ground pixels

              if ((j=iscan[i])!=ITEM_NONE)
               {
                for (n=0;n<(int)pOrbitFile->pixel_size;n++)
                 {
                  pOrbitFile->scanline_indexes[k+n]=j;
                  pOrbitFile->scanline_pixtype[k+n]=n;
                  pOrbitFile->scanline_pixnum[k+n]=i;
                  pOrbitFile->alongtrack_indexes[k+n]=pOrbitFile->n_alongtrack;
                  pOrbitFile->delta_time[k+n]=delta_time_scan[j][n];
                 }
                k+=pOrbitFile->pixel_size;
               }

              // Consider backscan pixels

              if ((j=iscan_bs[i])!=ITEM_NONE)
               {
                pOrbitFile->scanline_indexes[k]=j;
                pOrbitFile->scanline_pixtype[k]=3;
                pOrbitFile->scanline_pixnum[k]=i;
                pOrbitFile->alongtrack_indexes[k]=pOrbitFile->n_alongtrack;
                pOrbitFile->delta_time[k]=delta_time_scan_bs[j][0];
                k++;
               }

              if ((iscan[i]!=ITEM_NONE) || (iscan_bs[i]!=ITEM_NONE))
               pOrbitFile->n_alongtrack++;
             }
           }

          // Sort ground pixels and backscans using scanline

          pOrbitFile->start_pixel=startpixel[0];
         }

        // get_ref_info(pOrbitFile);
       }
      catch (std::runtime_error& e)
       {
        pOrbitFile->rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());  // in case of error, capture the message
       }

      // Return

      if (iscan!=NULL)
       free(iscan);
      if (iscan_bs!=NULL)
       free(iscan_bs);

      if (!strcasecmp(pEngineContext->fileInfo.fileName,pOrbitFile->fileName))
       gome1netCDF_currentFileIndex=indexFile;

      gome1netCDF_totalRecordNumber+=pOrbitFile->specNumber;
     }
   } // end if (currentfileindex == ITEM_NONE)

  if (!rc
      && gome1netCDF_currentFileIndex!=ITEM_NONE
      && !(rc=gome1netCDF_orbitFiles[gome1netCDF_currentFileIndex].rc)
      && (pEngineContext->recordNumber=gome1netCDF_orbitFiles[gome1netCDF_currentFileIndex].specNumber)>0)

   {
    pOrbitFile=&gome1netCDF_orbitFiles[gome1netCDF_currentFileIndex];

    pEngineContext->n_crosstrack=NUM_VZA_REFS;
    pEngineContext->n_alongtrack=pOrbitFile->n_alongtrack;

    // Irradiance spectrum

    GOME1NETCDF_Get_Irradiance(pOrbitFile,channel_index,pEngineContext->buffers.lambda_irrad,pEngineContext->buffers.irrad);

    for(i=0; i<MAX_SWATHSIZE; ++i)
     NDET[i]=(int)pOrbitFile->calibration.channel_size;
   }
  else
   rc=1;

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION GOME1NETCDF_Read
// -----------------------------------------------------------------------------
//!
//! \fn      RC GOME1NETCDF_Read(ENGINE_CONTEXT *pEngineContext,int recordNo)
//! \details Read a specified record from a file in netCDF format
//! \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.\n
//! \param   [in]  recordNo        the index of the record to read\n
//! \param   [in]  fileIndex       the index of the orbit file (in automatic reference selection, all files of the current folder could be loaded)
//! \return  ERROR_ID_FILE_END if the requested record number is not found\n
//!          ERROR_ID_FILE_RECORD if the requested record doesn't satisfy current criteria (for example for the selection of the reference)\n
//!          ERROR_ID_NO on success
//!
// -----------------------------------------------------------------------------

RC GOME1NETCDF_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,INDEX fileIndex)
 {
  // Declarations

  GOME1NETCDF_ORBIT_FILE *pOrbitFile;
  PRJCT_INSTRUMENTAL *pInstrumental;
  NetCDFGroup obs_group;                                                        // measurement group in the netCDF file
  RECORD_INFO *pRecordInfo;                                                     // pointer to the record structure in the engine context
  geodata geo;                                                                  // geolocation and angles
  clouddata cloud;                                                              // information on clouds
  int selected_band;                                                            // index of the selected band (0..6)
  vector<float> wve;                                                            // wavelength calibration
  vector<float> spe;                                                            // spectrum
  vector<float> err;                                                            // instrumental errors
  vector<short> clb;                                                            // spectral index to retrieve the calibration
  vector<int>   scanline;                                                       // index of pixels in the original orbit

//  vector<short> qf;                                                             // quality flag
  int i,j;                                                                      // index for loops and arrays
  RC rc;                                                                        // return code

  // Initializations

  pOrbitFile=&gome1netCDF_orbitFiles[(fileIndex==ITEM_NONE)?gome1netCDF_currentFileIndex:fileIndex];
  pRecordInfo=&pEngineContext->recordInfo;
  pRecordInfo->i_alongtrack=pOrbitFile->alongtrack_indexes[recordNo-1];                  // because in mediate, use +1
  pInstrumental=&pEngineContext->project.instrumental;
  selected_band=pInstrumental->gomenetcdf.bandType;
  rc = ERROR_ID_NO;

  // The requested record is out of range

  if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;
  else if (pOrbitFile->delta_time[recordNo-1]>(double)1.e36)
   rc=ERROR_ID_FILE_RECORD;
  else
   {
    size_t scanIndex=pOrbitFile->scanline_indexes[recordNo-1];                  // index in the ground pixel scanlines or backscan scanlines
    int    pixelType=pOrbitFile->scanline_pixtype[recordNo-1];                  // pixel type
    size_t pixelIndex=(pixelType==3)?0:pixelType;                               // index of the pixel in the scan : should be 0,1,2 for ground pixels and 0 for backscans
    int    pixelSize=(pixelType==3)?1:3;                                        // pixel size : should be 3 for ground pixels and 1 for backscans
    size_t start[] = {0,scanIndex,((int)pixelType==3)?0:(size_t)pixelType,0};
    size_t count[] = {1,1,1,pOrbitFile->det_size};                              // only one record to load

    getDate(pOrbitFile,pOrbitFile->delta_time[recordNo-1], &pRecordInfo->present_datetime);

    for (i=0;i<(int)pOrbitFile->calibration.channel_size;i++)
     {
      pEngineContext->buffers.spectrum[i]=
      pEngineContext->buffers.sigmaSpec[i]=(double)0.;
     }


  // TODO SHORT_DATE irradDate;                                                         // date of measurement for the irradiance spectrum
  // TODO struct time irradTime;                                                        // time of measurement for the irradiance spectrum

  // TODO int     nRef;                                                                 // size of irradiance vectors

    geo=(pixelType==3)?pOrbitFile->backscan_geodata:pOrbitFile->ground_geodata;
    cloud=(pixelType==3)?pOrbitFile->backscan_clouddata:pOrbitFile->ground_clouddata;

    // Solar zenith angles

    auto sza = reinterpret_cast<const float(*)[pixelSize][3]>(geo.sza.data());
    auto saa = reinterpret_cast<const float(*)[pixelSize][3]>(geo.saa.data());
    auto vza = reinterpret_cast<const float(*)[pixelSize][3]>(geo.vza.data());
    auto vaa = reinterpret_cast<const float(*)[pixelSize][3]>(geo.vaa.data());
    auto sza_sat = reinterpret_cast<const float(*)[pixelSize][3]>(geo.sza_sat.data());
    auto saa_sat = reinterpret_cast<const float(*)[pixelSize][3]>(geo.saa_sat.data());
    auto vza_sat = reinterpret_cast<const float(*)[pixelSize][3]>(geo.vza_sat.data());
    auto vaa_sat = reinterpret_cast<const float(*)[pixelSize][3]>(geo.vaa_sat.data());

    for (i=0;i<3;i++)
     {
      pRecordInfo->gome.sza[i]=sza[scanIndex][pixelIndex][i];
      pRecordInfo->gome.azim[i]=saa[scanIndex][pixelIndex][i];
      pRecordInfo->gome.vza[i]=vza[scanIndex][pixelIndex][i];
      pRecordInfo->gome.vaa[i]=vaa[scanIndex][pixelIndex][i];
     }

    pRecordInfo->Zm=pRecordInfo->gome.sza[1];
    pRecordInfo->Azimuth=pRecordInfo->gome.azim[1];
    pRecordInfo->zenithViewAngle=pRecordInfo->gome.vza[1];
    pRecordInfo->azimuthViewAngle=pRecordInfo->gome.vaa[1];

    pRecordInfo->satellite.sza=sza_sat[scanIndex][pixelIndex][1];
    pRecordInfo->satellite.saa=saa_sat[scanIndex][pixelIndex][1];
    pRecordInfo->satellite.vza=vza_sat[scanIndex][pixelIndex][1];
    pRecordInfo->satellite.vaa=vaa_sat[scanIndex][pixelIndex][1];

    // Geolocations

    auto latitude = reinterpret_cast<const float(*)[pixelSize]>(geo.lat.data());
    auto longitude = reinterpret_cast<const float(*)[pixelSize]>(geo.lon.data());

    pRecordInfo->latitude=latitude[scanIndex][pixelIndex];
    pRecordInfo->longitude=longitude[scanIndex][pixelIndex];

    auto latitude_bounds = reinterpret_cast<const float(*)[pixelSize][4]>(geo.lat_bounds.data());
    auto longitude_bounds = reinterpret_cast<const float(*)[pixelSize][4]>(geo.lon_bounds.data());

    for (i=0;i<4;i++)
     {
      pRecordInfo->satellite.cornerlats[i]=latitude_bounds[scanIndex][pixelIndex][i];
      pRecordInfo->satellite.cornerlons[i]=longitude_bounds[scanIndex][pixelIndex][i];
     }

    // Satellite height and earth radius

    auto sat_height = reinterpret_cast<const float(*)[pixelSize]>(geo.alt_sat.data());
    auto earth_radius = reinterpret_cast<const float(*)[pixelSize]>(geo.earth_rad.data());

    pRecordInfo->satellite.altitude = sat_height[scanIndex][pixelIndex];
    pRecordInfo->satellite.earth_radius = earth_radius[scanIndex][pixelIndex];

    // Information on clouds

    auto cloud_fraction = reinterpret_cast<const float(*)[pixelSize]>(cloud.cloud_frac.data());
    auto cloud_top_pressure = reinterpret_cast<const float(*)[pixelSize]>(cloud.cloud_pres.data());

    pRecordInfo->satellite.cloud_fraction = cloud_fraction[scanIndex][pixelIndex];
    pRecordInfo->satellite.cloud_top_pressure = cloud_top_pressure[scanIndex][pixelIndex];

    // Get spectra

    obs_group = pOrbitFile->current_file.getGroup(pOrbitFile->root_name+((pixelType==3)?pOrbitFile->mode_bs:pOrbitFile->mode)+gome1netcdf_bandName[selected_band]+"/OBSERVATIONS");

    // auto spec = reinterpret_cast<const float(*)[det_size]>(spe.data());

    obs_group.getVar("radiance",start,count,4,(float)0.,spe);
    obs_group.getVar("radiance_precision",start,count,4,(float)0.,err);
    obs_group.getVar("spectral_index",start,count,3,(short)0,clb);

    GOME1NETCDF_Get_Wavelength(pOrbitFile,channel_index,(int)clb[0],pEngineContext->buffers.lambda);

    for (i=0,j=pOrbitFile->start_pixel;i<(int)pOrbitFile->det_size;i++,j++)
     {
      pEngineContext->buffers.spectrum[j]=spe[i];
      pEngineContext->buffers.sigmaSpec[j]=err[i];
     }

    // obs_group.getVar("scanline",start,count,2,(int)0,scanline);

    pRecordInfo->gome.pixelNumber=pOrbitFile->scanline_pixnum[recordNo-1];      // pixel number
    pRecordInfo->gome.pixelType=pixelType;                                      // pixel type
    pRecordInfo->i_crosstrack=pixelType;
   }

  // Release buffers

  scanline.clear();

  wve.clear();
  spe.clear();
  err.clear();
  clb.clear();

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION GOME1NETCDF_Cleanup
// -----------------------------------------------------------------------------
//!
//! \fn      void GOME1NETCDF_Cleanup(void)
//! \details Close the current file and release allocated buffers\n
//!
// -----------------------------------------------------------------------------

void GOME1NETCDF_Cleanup(void)
 {
  GOME1NETCDF_ORBIT_FILE *pOrbitFile;

  for (int indexFile = 0;indexFile<gome1netCDF_orbitFilesN;indexFile++)
   {
    pOrbitFile=&gome1netCDF_orbitFiles[indexFile];

    for (int j=0;j<NUM_VZA_REFS;j++)
     {
      if (pOrbitFile->refInfo[j]!=NULL)
       free(pOrbitFile->refInfo[j]);

      pOrbitFile->refNum[j]=0;
      pOrbitFile->refInfo[j]=NULL;
     }

    pOrbitFile->calibration = calib();
    pOrbitFile->irradiance = refspec();
    pOrbitFile->ground_geodata = geodata();
    pOrbitFile->ground_clouddata = clouddata();
    pOrbitFile->backscan_geodata = geodata();
    pOrbitFile->backscan_clouddata = clouddata();

    pOrbitFile->scanline_indexes.clear();
    pOrbitFile->scanline_pixtype.clear();
    pOrbitFile->scanline_pixnum.clear();
    pOrbitFile->alongtrack_indexes.clear();
    pOrbitFile->delta_time.clear();
    pOrbitFile->current_file.close();
   }

  gome1netCDF_orbitFilesN=0;
  gome1netCDF_currentFileIndex=ITEM_NONE;
 }

// -----------------------------------------------------------------------------
// FUNCTION GOME1NETCDF_get_orbit_date
// -----------------------------------------------------------------------------
//!
//! \fn      RC GOME1NETCDF_get_orbit_date(int *orbit_year, int *orbit_month, int *orbit_day)
//! \details Return the date of the current orbit file to create the output directory
//! \param   [out]  year  pointer to the engine context; some fields are affected by this function.\n
//! \param   [in]  responseHandle  address where to transmit error to the user interface\n
//! \return  0 on success, 1 otherwise
//!
// -----------------------------------------------------------------------------

// ACCOUNT FOR ORBIT !!!!!!!!!!!!!!!!!!!!!

int GOME1NETCDF_get_orbit_date(int *orbit_year, int *orbit_month, int *orbit_day) {
  if (gome1netCDF_currentFileIndex==ITEM_NONE)
   return 0;
  else
   {
    GOME1NETCDF_ORBIT_FILE *pOrbitFile;
    pOrbitFile=&gome1netCDF_orbitFiles[gome1netCDF_currentFileIndex];
    // current_file = NetCDFFile(pOrbitFile->fileName,NC_NOWRITE);                 // open current file

    std::istringstream orbit_start(pOrbitFile->current_file.getAttText("time_reference"));
    // time_coverage_start is formatted as "YYYY-MM-DD"
    char tmp; // to skip "-" chars
    orbit_start >> *orbit_year >> tmp >> *orbit_month >> tmp >> *orbit_day;
    // current_file.close();
    return  orbit_start.good() ? 0 : 1;
   }
}


static void free_vza_refs(void) {
  if (vza_refs != NULL) {
    for (int i=0; i<NFeno; ++i) {
      for(size_t j=0; j<NUM_VZA_REFS; ++j) {
        free(vza_refs[i][j].spectrum);
      }
    }
  }
  free(vza_refs);
  vza_refs=NULL;
}

// -----------------------------------------------------------------------------
// FUNCTION get_ref_info
// -----------------------------------------------------------------------------
//!
//! \fn      RC get_ref_info(GOME1NETCDF_ORBIT_FILE *pOrbitFile,GOME1NETCDF_REF *ref_list)
//! \details Browse spectra of the current orbit file and collect information for the automatic reference selection
//! \param   [out] ref_list  the information on spectra useful for the automatic reference selection\n
//! \param   [in]  pOrbitFile : pointer to the current orbit file\n
//!
// -----------------------------------------------------------------------------

static void get_ref_info2(GOME1NETCDF_ORBIT_FILE *pOrbitFile,GOME1NETCDF_REF *ref_list)
 {
  // Global declarations

  GOME1NETCDF_REF *pRef;
  geodata geo;
  int pixelSize;

  // Declare substition variables for ground pixels

  geo=pOrbitFile->ground_geodata;                                               // do not work ???
  pixelSize=3;

  auto sza_gr = reinterpret_cast<const float(*)[pixelSize][3]>(geo.sza.data());
  auto lat_gr =  reinterpret_cast<const float(*)[pixelSize]>(geo.lat.data());
  auto lon_gr =  reinterpret_cast<const float(*)[pixelSize]>(geo.lon.data());
  auto cloud_gr =  reinterpret_cast<const float(*)[pixelSize]>(pOrbitFile->ground_clouddata.cloud_frac.data());

  // Declare substition variables for backscan pixels

  geo=pOrbitFile->backscan_geodata;
  pixelSize=1;

  auto sza_bs = reinterpret_cast<const float(*)[pixelSize][3]>(geo.sza.data());
  auto lat_bs =  reinterpret_cast<const float(*)[pixelSize]>(geo.lat.data());
  auto lon_bs =  reinterpret_cast<const float(*)[pixelSize]>(geo.lon.data());

  auto cloud_bs =  reinterpret_cast<const float(*)[pixelSize]>(pOrbitFile->backscan_clouddata.cloud_frac.data());

  //  {
  //   FILE *fp;
  //   fp=fopen("toto.dat","a+t");

  for (int i=0;i<pOrbitFile->specNumber;i++)
   {
    // Declarations

    size_t scanIndex=pOrbitFile->scanline_indexes[i];                           // index in the ground pixel scanlines or backscan scanlines
    int    pixelType=pOrbitFile->scanline_pixtype[i];                           // pixel type
    size_t pixelIndex=(pixelType==3)?0:pixelType;                               // index of the pixel in the scan : should be 0,1,2 for ground pixels and 0 for backscans

    // Get useful information for automatic reference selection

    pRef=&ref_list[i];

    pRef->pixelType=pixelType;
    pRef->sza=(pixelType==3)?sza_bs[scanIndex][pixelIndex][1]:sza_gr[scanIndex][pixelIndex][1];
    pRef->latitude=(pixelType==3)?lat_bs[scanIndex][pixelIndex]:lat_gr[scanIndex][pixelIndex];
    pRef->longitude=(pixelType==3)?lon_bs[scanIndex][pixelIndex]:lon_gr[scanIndex][pixelIndex];
    pRef->cloudFraction=(pixelType==3)?cloud_bs[scanIndex][pixelIndex]:cloud_gr[scanIndex][pixelIndex];

  //  if (i>=970 && i<=990)
  //   fprintf(fp,"--- %d %d %d %g %g %g\n",(i+1),pixelType,pixelIndex,pRef->latitude,pRef->longitude,pRef->sza);

   }


  //   fclose(fp);
  //  }
 }

static bool use_as_reference(GOME1NETCDF_REF *pRef,const FENO *feno)
 {
  const double latDelta = fabs(feno->refLatMin - feno->refLatMax);
  const double lonDelta = fabs(feno->refLonMin - feno->refLonMax);
  const double cloudDelta = fabs(feno->cloudFractionMin - feno->cloudFractionMax);

  // Check if the current record satisfies reference selection

  const bool match_lat = latDelta <= EPSILON || (pRef->latitude >= feno->refLatMin && pRef->latitude <= feno->refLatMax);
  const bool match_lon = lonDelta <= EPSILON
        || ( (feno->refLonMin < feno->refLonMax && pRef->longitude >=feno->refLonMin && pRef->longitude <= feno->refLonMax)
          || (feno->refLonMin >= feno->refLonMax && (pRef->longitude >= feno->refLonMin || pRef->longitude <= feno->refLonMax))
           ); // if refLonMin > refLonMax, we have either lonMin < lon < 360, or 0 < lon < refLonMax

  const bool match_sza = feno->refSZADelta <= EPSILON || (fabs(pRef->sza-feno->refSZA) <= feno->refSZADelta);
  const bool match_cloud = cloudDelta <= EPSILON || (pRef->cloudFraction>9.e36) || (pRef->cloudFraction >= feno->cloudFractionMin && pRef->cloudFraction <= feno->cloudFractionMax);

  /* if (match_lat && match_lon && match_sza)
   {
    FILE *fp;
    fp=fopen("toto.dat","a+t");
    fprintf(fp,"%d %g %g %g\n",pRef->pixelType,pRef->latitude,pRef->longitude,pRef->sza);
    fclose(fp);
   } */


  return (match_lat && match_lon && match_sza && match_cloud);
 }

// create a list of all spectra that match reference selection criteria for one or more analysis windows.
static int find_ref_spectra(struct ref_list *(*selected_spectra)[NUM_VZA_REFS], struct ref_list **list_handle)
 {
  // zero-initialize
  for (int i=0; i<NFeno; ++i)
   {
    for (size_t j=0; j<NUM_VZA_REFS; ++j)
     selected_spectra[i][j] = (struct ref_list *)NULL;
   }
  *list_handle = NULL;

  // iterate over all orbit files in same directory
  for (int i=0; i<gome1netCDF_orbitFilesN; ++i) {
    GOME1NETCDF_ORBIT_FILE *pOrbitFile=&gome1netCDF_orbitFiles[i];
    size_t n_wavel = pOrbitFile->calibration.channel_size;
    GOME1NETCDF_REF *refList;

   /* {
      FILE *fp;
      fp=fopen("toto.dat","a+t");
      fprintf(fp,"%s\n",pOrbitFile->fileName);
      fclose(fp);
     } */


    if ((refList=(GOME1NETCDF_REF *)malloc(pOrbitFile->specNumber*sizeof(GOME1NETCDF_REF)))!=NULL)
     {
      get_ref_info2(pOrbitFile,refList);

      for (int j=0; j<pOrbitFile->specNumber; ++j) {
        // each spectrum can be used for multiple analysis windows, so
        // we use one copy, and share the pointer between the different
        // analysis windows. We initialize as NULL, it becomes not-null
        // as soon as it is used in one or more analysis windows:
        struct ref_spectrum *ref = NULL;
        RC rc;

        // check if this spectrum satisfies constraints for one of the analysis windows:
        for(int analysis_window = 0; analysis_window<NFeno; analysis_window++) {
          const int pixel_type = pOrbitFile->scanline_pixtype[j];
          const FENO *pTabFeno = &TabFeno[pixel_type][analysis_window];

          if (!pTabFeno->hidden
              && pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC
              && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC
              && use_as_reference(&refList[j],pTabFeno) ) {

              /* {
                 FILE *fp;
                 fp=fopen("toto.dat","a+t");
                 fprintf(fp,"Found %d %d %d %d\n",i,j,analysis_window,pixel_type);
                 fclose(fp);
                } */

            if (ref == NULL) {
              // ref hasn't been initialized yet for another analysis window, so do that now:
              ref = (struct ref_spectrum *)malloc(sizeof(struct ref_spectrum));
              ref->lambda = (double *)malloc(n_wavel*sizeof(*ref->lambda));
              ref->spectrum = (double *)malloc(n_wavel*sizeof(*ref->spectrum));

              // store the new reference at the front of the linked list:
              struct ref_list *newRef = (struct ref_list *)malloc(sizeof(*newRef));
              newRef->ref = ref;
              newRef->next = *list_handle;
              *list_handle = newRef;

              if ((rc = GOME1NETCDF_Read(&ENGINE_contextRef, j, i))!= ERROR_ID_NO)
               return rc;

              for (int k=0; k<(int)n_wavel; ++k)
               {
                ref->lambda[k] = ENGINE_contextRef.buffers.lambda[k];
                ref->spectrum[k] = ENGINE_contextRef.buffers.spectrum[k];
               }
            }

            // store ref at the front of the list of selected references for this analysis window and vza bin.
            struct ref_list *list_item = (struct ref_list *)malloc(sizeof(struct ref_list));
            list_item->ref = ref;
            list_item->next = selected_spectra[analysis_window][pixel_type];
            selected_spectra[analysis_window][pixel_type] = list_item;
          }
        }
      }

      free(refList);
     }
  }

  return ERROR_ID_NO;
}

static void initialize_vza_refs(void) {
  free_vza_refs(); // will free previous allocated structures, if any.

  vza_refs = (struct reference (*)[4])malloc(NFeno * sizeof(*vza_refs));

  // Build array of pointers to the collection of VZA references:
  for (int i=0; i<NFeno; ++i) {
    const int n_wavel = TabFeno[0][i].NDET;
    for(size_t j=0; j<NUM_VZA_REFS; ++j) {
      struct reference *ref = &vza_refs[i][j];
      ref->spectrum = (double *)malloc(n_wavel*sizeof(*ref->spectrum));
      for (int i=0; i<n_wavel; ++i)
        ref->spectrum[i]=0.;
      ref->n_wavel = n_wavel;
      ref->n_spectra = 0;
      ref->norm = ref->shift = ref->stretch = ref->stretch2 = 0.0;
    }
  }
}

static int show_ref_info(int i_row, const FENO *pTabFeno, const struct reference *refs, void *responseHandle) {
  int i_column=2;

  mediateResponseLabelPage(plotPageRef, "Reference", "Reference", responseHandle);
  mediateResponseCellDataString(plotPageRef, i_row, i_column, "Analysis window", responseHandle);
  mediateResponseCellDataString(plotPageRef, i_row, 1+i_column, pTabFeno->windowName, responseHandle);
  i_row+=2;
  mediateResponseCellDataString(plotPageRef, i_row, i_column, "Earthshine reference", responseHandle);
  mediateResponseCellDataString(plotPageRef, i_row, 1+i_column, "# spectra", responseHandle);
  mediateResponseCellDataString(plotPageRef, i_row, 2+i_column, "shift", responseHandle);
  mediateResponseCellDataString(plotPageRef, i_row, 3+i_column, "stretch", responseHandle);
  ++i_row;
  const char *pixeltype[] = {"EAST", "CENTER", "WEST", "BACKSCAN"};
  for (int i=0; i != sizeof(pixeltype)/sizeof(pixeltype[0]); ++i) {
    mediateResponseCellDataString(plotPageRef, i_row, i_column, pixeltype[i], responseHandle);
    mediateResponseCellDataInteger(plotPageRef, i_row, 1+ i_column, refs[i].n_spectra, responseHandle);
    mediateResponseCellDataDouble(plotPageRef, i_row, 2 + i_column,refs[i].shift, responseHandle);
    mediateResponseCellDataDouble(plotPageRef, i_row, 3 + i_column,refs[i].stretch, responseHandle);
    ++i_row;
    const char* labelfmt = "Reference %s";
    char plot_label[strlen(labelfmt) +strlen(pixeltype[i])];
    sprintf(plot_label, labelfmt, pixeltype[i]);
    plot_data_t spectrum_data;
    spectrum_data.x = pTabFeno->LambdaRef;
    spectrum_data.y = refs[i].spectrum;
    spectrum_data.length = refs[i].n_wavel;
    spectrum_data.curveName[0] = '\0';

    mediateResponsePlotData(plotPageRef,&spectrum_data,1,Spectrum,forceAutoScale,plot_label,"Wavelength (nm)","Intensity", responseHandle);
  }
  return ++i_row;
}

// -----------------------------------------------------------------------------
// FUNCTION      GOME1NETCDF_NewRef
// -----------------------------------------------------------------------------
// PURPOSE       In automatic reference selection, search for reference spectra
//
// INPUT         pEngineContext    hold the configuration of the current project
//
// RETURN        ERROR_ID_ALLOC if something failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC GOME1NETCDF_NewRef(ENGINE_CONTEXT *pEngineContext,void *responseHandle) {

  // make a copy of the EngineContext structure to read reference data
  RC rc=EngineCopyContext(&ENGINE_contextRef,pEngineContext);

  if (ENGINE_contextRef.recordNumber==0)
    return ERROR_ID_ALLOC;

  // Allocate reference structures:
  initialize_vza_refs();

  // 1. look in all candidate orbit files (i.e. orbit files in same
  //    dir)

  // for each analysis window: selected spectra per VZA bin
  // the same spectrum can be used in multiple analysis windows.
  struct ref_list *selected_spectra[NFeno][NUM_VZA_REFS];

  // list_handle: list of references to same set of spectra, used for
  // memory management.  In this list, each spectrum appears only once.
  struct ref_list *list_handle;

  rc = find_ref_spectra(selected_spectra, &list_handle);

  if (rc != ERROR_ID_NO)
    goto cleanup;

  for (int indexFenoColumn=0;(indexFenoColumn<ANALYSE_swathSize) && !rc;indexFenoColumn++)
   {
    // 2. average spectra per analysis window and per VZA bin
    for (int i=0;(i<NFeno) && (rc<THREAD_EVENT_STOP);i++)
     {
      FENO *pTabFeno=&TabFeno[indexFenoColumn][i];

      if ((pTabFeno->hidden!=1) &&
          (pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC) &&
          (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC))
       {

        //for (size_t j=0; j<NUM_VZA_REFS; ++j) {
          if (selected_spectra[i][indexFenoColumn] == NULL)
           {
            // We may not find references for every VZA bin/analysis
            // window.  At this point we just emit a warning (it's not a
            // problem until we *need* that during retrieval for that bin).

          /*  {
             FILE *fp;
             fp=fopen("toto.dat","a+t");
             fprintf(fp,"Not found %d %d\n",i,indexFenoColumn);
             fclose(fp);
            } */

  #define MESSAGE " for analysis window %s and VZA bin %d"
            const int length = strlen(MESSAGE) + strlen(pTabFeno->windowName) + strlen(TOSTRING(MAX_FENO));
            char tmp[length];
            sprintf(tmp, MESSAGE, pTabFeno->windowName, indexFenoColumn); // TODO convert ref number back to bin for error message
  #undef MESSAGE
            ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_REFERENCE_SELECTION, tmp);
            continue;
           }
          struct reference *ref = &vza_refs[i][indexFenoColumn];

        /*  {
           FILE *fp;
           fp=fopen("toto.dat","a+t");
           fprintf(fp,"Average_ref_spectra %d %s\n",i,pTabFeno->windowName);
           fclose(fp);
          } */

          rc = average_ref_spectra(selected_spectra[i][indexFenoColumn], pTabFeno->LambdaRef, pTabFeno->NDET, ref);

        /*  {
           FILE *fp;
           fp=fopen("toto.dat","a+t");
           fprintf(fp,"Average_ref_spectra %d\n",rc);
           fclose(fp);
          } */

          if (rc != ERROR_ID_NO)
            goto cleanup;

          // align ref w.r.t irradiance reference:
          double sigma_shift, sigma_stretch, sigma_stretch2; // not used here...

        /*  {
           FILE *fp;
           fp=fopen("toto.dat","a+t");
           fprintf(fp," ANALYSE_fit_shift_stretch %d %s\n",i,pTabFeno->windowName);
           fclose(fp);
          } */

          rc = ANALYSE_fit_shift_stretch(1, 0, pTabFeno->SrefEtalon, ref->spectrum,
                                         &ref->shift, &ref->stretch, &ref->stretch2,
                                         &sigma_shift, &sigma_stretch, &sigma_stretch2);

  /* {
   FILE *fp;
   fp=fopen("toto.dat","a+t");
   fprintf(fp,"ANALYSE_fit_shift_stretch %d\n",rc);
   fclose(fp);
  } */

       }
     }
   }

 cleanup:
  // 3. free lists created in step 1

  // for 'selected_spectra', we only free the 'gome_ref_list'
  // structures, the other components are pointers to spetra owned by
  // the list_handle structure:
  for (int i=0; i<NFeno; ++i) {
    for (size_t j=0; j<NUM_VZA_REFS; ++j) {
      free_ref_list(selected_spectra[i][j], FREE_LIST_ONLY);
    }
  }

  // for 'list_handle', we also free the gome_ref_spectrum* pointers,
  // and the double* pointers 'lambda' & 'spectrum':
  free_ref_list(list_handle, FREE_DATA);

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION GOME1NETCDF_LoadAnalysis
// -----------------------------------------------------------------------------
//!
//! \fn      RC GOME1NETCDF_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
//! \details Load analysis parameters depending on the reference spectrum
//! \param   [in]  pEngineContext  pointer to the engine context; some fields are affected by this function.\n
//! \param   [in]  responseHandle  address where to transmit error to the user interface\n
//! \return  ERROR_ID_FILE_END if the requested record number is not found\n
//!          ERROR_ID_FILE_RECORD if the requested record doesn't satisfy current criteria (for example for the selection of the reference)\n
//!          ERROR_ID_NO on success
//!
// -----------------------------------------------------------------------------

RC GOME1NETCDF_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle) {

  GOME1NETCDF_ORBIT_FILE *pOrbitFile=&gome1netCDF_orbitFiles[gome1netCDF_currentFileIndex];
  const int n_wavel=pOrbitFile->calibration.channel_size;
  int saveFlag= pEngineContext->project.spectra.displayDataFlag;

  RC rc=pOrbitFile->rc;

  // don't continue when current file has an error, or if we are
  // working with automatic references and don't need to create a new
  // reference:

  if (rc || (pEngineContext->analysisRef.refAuto && ! gome1netCDF_loadReferenceFlag) )
    return rc;

  int useUsamp=0,useKurucz=0;

  // Browse analysis windows and load missing data

  for (int indexFenoColumn=0;(indexFenoColumn<ANALYSE_swathSize) && !rc;indexFenoColumn++) {

   if (!pEngineContext->project.instrumental.use_row[indexFenoColumn]) continue;

   for (int indexFeno=0; indexFeno<NFeno && !rc; indexFeno++) {
     FENO *pTabFeno=&TabFeno[indexFenoColumn][indexFeno];
     pTabFeno->NDET=n_wavel;

     // Load calibration and reference spectra

     if (!pTabFeno->gomeRefFlag) { // use irradiance from L1B file

       GOME1NETCDF_Get_Irradiance(pOrbitFile,channel_index,pTabFeno->LambdaRef,pTabFeno->Sref);

       if (!TabFeno[indexFenoColumn][indexFeno].hidden) {
         rc = VECTOR_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&pTabFeno->refNormFact,"GOME1NETCDF_LoadAnalysis (Reference) ");

         if (rc)
           goto EndGOME1NETCDF_LoadAnalysis;

         memcpy(pTabFeno->SrefEtalon,pTabFeno->Sref,sizeof(double) *pTabFeno->NDET);
         pTabFeno->useEtalon=pTabFeno->displayRef=1;

         // Browse symbols

         for (int indexTabCross=0; indexTabCross<pTabFeno->NTabCross; indexTabCross++) {
           CROSS_REFERENCE *pTabCross=&pTabFeno->TabCross[indexTabCross];
           WRK_SYMBOL *pWrkSymbol=&WorkSpace[pTabCross->Comp];

           // Cross sections and predefined vectors

           if ((((pWrkSymbol->type==WRK_SYMBOL_CROSS) && (pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING)) ||
                ((pWrkSymbol->type==WRK_SYMBOL_PREDEFINED) &&
                 ((indexTabCross==pTabFeno->indexCommonResidual) ||
                  (((indexTabCross==pTabFeno->indexUsamp1) || (indexTabCross==pTabFeno->indexUsamp2)) && (pUsamp->method==PRJCT_USAMP_FILE))))) &&
               ((rc=ANALYSE_CheckLambda(pWrkSymbol,pTabFeno->LambdaRef,pTabFeno->NDET)) !=ERROR_ID_NO))
               {
             goto EndGOME1NETCDF_LoadAnalysis;
            }
         }

         // Gaps : rebuild subwindows on new wavelength scale

         doas_spectrum *new_range = spectrum_new();
         int DimL=0;
         for (int indexWindow = 0; indexWindow < pTabFeno->fit_properties.Z; indexWindow++) {
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

         rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,indexFenoColumn);

         if (rc) goto EndGOME1NETCDF_LoadAnalysis;

         if ( (!pKuruczOptions->fwhmFit || !pTabFeno->useKurucz) && pTabFeno->xsToConvolute) {
           rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,ANALYSIS_slitMatrix,ANALYSIS_slitParam,pSlitOptions->slitFunction.slitType,indexFenoColumn,pSlitOptions->slitFunction.slitWveDptFlag);
         }

         if (rc) goto EndGOME1NETCDF_LoadAnalysis;
       }
     }

     memcpy(pTabFeno->LambdaK,pTabFeno->LambdaRef,sizeof(double) *pTabFeno->NDET);
     memcpy(pTabFeno->Lambda,pTabFeno->LambdaRef,sizeof(double) *pTabFeno->NDET);

     useUsamp+=pTabFeno->useUsamp;
     useKurucz+=pTabFeno->useKurucz;
   }

  // Wavelength calibration alignment

  if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ)) {
    KURUCZ_Init(0,indexFenoColumn);

    if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0,responseHandle,indexFenoColumn)) !=ERROR_ID_NO))
      goto EndGOME1NETCDF_LoadAnalysis;
  }
 }

  // Build undersampling cross sections

  if (useUsamp && (THRD_id!=THREAD_TYPE_KURUCZ) && !(rc=ANALYSE_UsampLocalAlloc(0))) {
    // ANALYSE_UsampLocalFree();

   for (int indexFenoColumn=0;indexFenoColumn<ANALYSE_swathSize;indexFenoColumn++)

    if (((rc=ANALYSE_UsampLocalAlloc(0)) !=ERROR_ID_NO) ||
        ((rc=ANALYSE_UsampBuild(0,0,indexFenoColumn)) !=ERROR_ID_NO) ||
        ((rc=ANALYSE_UsampBuild(1,ITEM_NONE,indexFenoColumn)) !=ERROR_ID_NO))

      goto EndGOME1NETCDF_LoadAnalysis;
  }

  // Automatic reference selection

  if (gome1netCDF_loadReferenceFlag)
   rc=GOME1NETCDF_NewRef(pEngineContext,responseHandle);
  if (!rc) gome1netCDF_loadReferenceFlag=0;

EndGOME1NETCDF_LoadAnalysis:

  return rc;
}


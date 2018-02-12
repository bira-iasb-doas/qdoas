
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
#include "mediate.h"
#include "analyse.h"
#include "spline.h"
#include "vector.h"
#include "zenithal.h"
#include "kurucz.h"
}

using std::string;
using std::vector;
using std::set;

#define GOME1NETCDF_NBAND                6
#define GOME1NETCDF_DETECTOR_SIZE     1024

const char *gome1netcdf_bandName[GOME1NETCDF_NBAND]=                             //!< \details Available bands in GOME1 files
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

//! \struct calib
//!< \details wavelength grid per detector temperature

struct calib
 {
  int channel_number;                                                           //!< \details the number of channels
  int channel_size;                                                             //!< \details should be the size of the detector
  int temp_number;                                                              //!< \details the number of considered temperatures

  vector<float> wavelength;                                                     //!< \details wavelength grid
 };

//! \struct refspec
//!< \details irradiance spectrum

struct refspec
 {
  vector<float> ref_spec;                                                       //!< \details the reference spectrum
  vector<float> ref_sigma;                                                      //!< \details error on the reference spectrum
  vector<short> spectral_index;                                                 //!< \details index of the spectral wavelength grid (see CALIBRATION/wavelength)
 };

//! \struct geodata
//! \brief %Geolocation coordinates and angles present in the GEODATA group of the GOME1 netCDF file.

struct geodata
 {
  vector<float> sza;                                                            //!< \details Solar zenith angle
  vector<float> sza_sat;                                                        //!< \details Solar zenith angle at satellite
  vector<float> saa;                                                            //!< \details Solar azimuth angle
  vector<float> saa_sat;                                                        //!< \details Solar azimuth angle_sat at satellite
  vector<float> vza;                                                            //!< \details Viewing zenith angle
  vector<float> vza_sat;                                                        //!< \details Viewing zenith angle at satellite
  vector<float> vaa;                                                            //!< \details Viewing azimuth angle
  vector<float> vaa_sat;                                                        //!< \details Viewing azimuth angle at satellite
  vector<float> lon;                                                            //!< \details pixel center longitude
  vector<float> lat;                                                            //!< \details pixel center latitude
  vector<float> alt_sat;                                                        //!< \details Satellite altitude
  vector<float> lat_bounds;                                                     //!< \details The corner coordinate latitudes of each observation
  vector<float> lon_bounds;                                                     //!< \details The corner coordinate longitudes of each observation
  vector<float> earth_rad;                                                      //!< \details The earth radius
 };

//! \struct clouddata
//! \brief Information on clouds contained in the CLOUDDATA group of the GOME1 netCDF file.

struct clouddata
 {
  vector<float> cloud_alb;                                                      //!< \details Cloud albedo
  vector<float> cloud_alb_prec;                                                 //!< \details Cloud albedo precision
  vector<float> cloud_frac;                                                     //!< \details Cloud fraction
  vector<float> cloud_frac_prec;                                                //!< \details Cloud fraction precision
  vector<float> cloud_hgt;                                                      //!< \details Cloud height
  vector<float> cloud_hgt_prec;                                                 //!< \details Cloud height precision
  vector<float> cloud_pres;                                                     //!< \details Cloud pressure
  vector<float> cloud_pres_prec;                                                //!< \details Cloud pressure precision
  vector<float> surf_hgt;                                                       //!< \details Surface height (kms)

  vector<unsigned char> snow_ice_flag;                                          //!< \details Snow/ice flag (0 : normal)
  vector<unsigned char> sun_glint;                                              //!< \details Possible Sun-glint derived by a geometrical calculation using viewing angles (0 = no, 1 = yes)
 };

// ================
// STATIC VARIABLES
// ================

static NetCDFFile current_file;                                                 //!< \details Pointer to the current netCDF file
static string root_name;                                                        //!< \details The name of the root (should be the basename of the file)
static geodata ground_geodata;                                                  //!< \details Keep the geolocation data and angles content from the MODE_NADIR group as far as the netCDF file is open
static geodata backscan_geodata;                                                //!< \details Keep the geolocation data and angles content from the MODE_NADIR_BACKSCAN group as far as the netCDF file is open
static clouddata ground_clouddata;                                              //!< \details Keep the cloud information content from the MODE_NADIR group as far as the netCDF file is open
static clouddata backscan_clouddata;                                            //!< \details Keep the cloud information content from the MODE_NADIR_BACKSCAN group as far as the netCDF file is open
static calib calibration;                                                       //!< \details Keep information on the wavelength grids as far as the netCDF file is open
static refspec irradiance;                                                      //!< \details Keep information on the irradiance spectrum as far as the netCDF file is open

static size_t det_size;                                                         //!< \details The current detector size
static size_t scan_size;                                                        //!< \details The number of lines in the MODE_NADIR group
static size_t scan_size_bs;                                                     //!< \details The number of lines in the MODE_NADIR_BACKSCAN group
static size_t pixel_size;                                                       //!< \details The number of ground pixels in one scan line (3 for the MODE_NADIR group)
static size_t pixel_size_bs;                                                    //!< \details The number of backscan pixels in one scan line (1 for the MODE_NADIR_BACKSCAN group)
static int channel_index;                                                       //!< \details For the irradiance, the channel number (depend on the selected band)
static int start_pixel;                                                         //!< \details start pixel in the channel (depend on the selected band)

static int gome1netCDF_loadReferenceFlag=0;

static vector<int>scanline_indexes;
static vector<char>scanline_pixtype;
static vector<int>alongtrack_indexes;
static vector<double> delta_time; // number of milliseconds after reference_time
static time_t reference_time;

// replace by functions using QDateTime?
static void getDate(double delta_t, struct datetime *date_time) {
  // TODO: handle UTC leap seconds?  Is this possible? (no UTC time info in file?)
  time_t thetime = reference_time + (int)floor(delta_t); //1000;

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

static void set_reference_time(const string& utc_date) {
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
  reference_time = STD_timegm(&t);
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
//! \param   [in]  pEngineContext pointer to the engine context (to get the default irradiance)
//! \param   [in]  channel_index  the index of the channel
//! \param   [in]  temp_index     the index of the temperature (called spectral_index in the netCDF file)
//! \param   [out] wavelength     the wavelength grid for the specified temperature
//!
// -----------------------------------------------------------------------------

void GOME1NETCDF_Get_Wavelength(ENGINE_CONTEXT *pEngineContext,int channel_index,int temp_index,double *wavelength)
 {
  auto wve = reinterpret_cast<const float(*)[calibration.channel_number][calibration.channel_size]>(calibration.wavelength.data());

  if ((temp_index>=0) && (temp_index<calibration.temp_number) && (channel_index>=0) && (channel_index<calibration.channel_size))
   for (int i=0;i<(int)calibration.channel_size;i++)
    wavelength[i]=wve[temp_index][channel_index][i];
  else
   memcpy(wavelength,pEngineContext->buffers.lambda_irrad,sizeof(double)*calibration.channel_size);
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
//! \param   [in]  pEngineContext pointer to the engine context (to get the default irradiance)
//! \channel [in]  channel_index the index of the requested channel
//! \param   [out] wavelength    the wavelength grid of the irradiance spectrum
//! \param   [out] irrad         the irradiance spectrum
//!
// -----------------------------------------------------------------------------

void GOME1NETCDF_Get_Irradiance (ENGINE_CONTEXT *pEngineContext,int channel_index,double *wavelength,double *irrad)
 {
  // Declarations

  NetCDFGroup calib_group;
  NetCDFGroup irrad_group;

  // Load the wavelengths grids

  calib_group=current_file.getGroup(root_name+"/CALIBRATION");
  calibration=GOME1NETCDF_Read_Calib(calib_group);

  // Load the irradiance spectrum

  irrad_group=current_file.getGroup(root_name+"/IRRADIANCE");
  irradiance=GOME1NETCDF_Read_Irrad(irrad_group,channel_index);

  GOME1NETCDF_Get_Wavelength(pEngineContext,channel_index,(int)irradiance.spectral_index[0],wavelength);

  for (int i=0;i<(int)calibration.channel_size;i++)
   irrad[i]=(double)irradiance.ref_spec[i];
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

  PRJCT_INSTRUMENTAL *pInstrumental;
  NetCDFGroup root_group;
  NetCDFGroup band_group;
  NetCDFGroup geodata_group;
  NetCDFGroup clouddata_group;
  NetCDFGroup obs_group;
  NetCDFGroup irrad_group;
  RC rc = ERROR_ID_NO;
  int selected_band;
  vector<int> scanline,scanline_bs;
  vector <double> deltatime,deltatime_bs;
  vector <short> startpixel;
  int i,j,k,n;
  int currentScanIndex;

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

  // Try to open the file and load metadata

  try
   {
    current_file = NetCDFFile(pEngineContext->fileInfo.fileName,NC_NOWRITE);    // open file
    root_name = current_file.getName();                                         // get the root name (should be the file name)
    root_group = current_file.getGroup(root_name);                              // go to the root
    set_reference_time(current_file.getAttText("time_reference"));              // get the reference time

    scan_size=scan_size_bs=pixel_size=pixel_size_bs=(size_t)0;

         // Dimensions of spectra are 'time' x 'scan_size' x 'pixel_size ' x 'spectral_channel'
         // For example : 1 x 552 x 3 x 832

    // Read ground pixels geodata and clouddata

    if (pInstrumental->gomenetcdf.pixelType!=PRJCT_INSTR_GOME1_PIXEL_BACKSCAN)   // if not backscan pixels only
     {
      band_group = current_file.getGroup(root_name+"/MODE_NADIR/"+gome1netcdf_bandName[selected_band]);

      pEngineContext->project.instrumental.use_row[0]=
      pEngineContext->project.instrumental.use_row[1]=
      pEngineContext->project.instrumental.use_row[2]=true;

      if ((band_group.dimLen("time")!=1) || (band_group.dimLen("ground_pixel")!=3))
       rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_FORMAT, "Dimensions of ground pixels in the GOME1 netCDF file are not the expected ones");  // in case of error, capture the message
      else
       {
        // Get the different groups

        geodata_group = current_file.getGroup(root_name+"/MODE_NADIR/"+gome1netcdf_bandName[selected_band]+"/GEODATA");
        clouddata_group = current_file.getGroup(root_name+"/MODE_NADIR/"+gome1netcdf_bandName[selected_band]+"/CLOUDDATA");
        obs_group = current_file.getGroup(root_name+"/MODE_NADIR/"+gome1netcdf_bandName[selected_band]+"/OBSERVATIONS");

        // Get the scanline and pixel size for ground pixel

        scan_size=band_group.dimLen("scanline");
        pixel_size=band_group.dimLen("ground_pixel");
        det_size=band_group.dimLen("spectral_channel");

        // Read the metadata

        ground_geodata=GOME1NETCDF_Read_Geodata(geodata_group,scan_size,pixel_size);
        ground_clouddata=GOME1NETCDF_Read_Clouddata(clouddata_group,scan_size,pixel_size);

        // Get the scanline indexes

        const size_t start[] = {0,0,0};
        const size_t count[] = {1,scan_size,pixel_size};

        obs_group.getVar("scanline",start,count,2,(int)-1,scanline);
        obs_group.getVar("delta_time",start,count,3,(double)0.,deltatime);
        band_group.getVar("start_pixel",start,count,1,(short)0,startpixel);
       }
     }

    // Read backscans geodata and clouddata

    if (pInstrumental->gomenetcdf.pixelType!=PRJCT_INSTR_GOME1_PIXEL_GROUND)     // if not ground pixels only
     {
      band_group = current_file.getGroup(root_name+"/MODE_NADIR_BACKSCAN/"+gome1netcdf_bandName[selected_band]);
      pEngineContext->project.instrumental.use_row[3]=true;

      if ((band_group.dimLen("time")!=1) || (band_group.dimLen("ground_pixel")!=1))
       rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_FORMAT, "Dimensions of backscan pixels in the GOME1 netCDF file are not the expected ones");  // in case of error, capture the message
      else
       {
        // Get the different groups

        geodata_group = current_file.getGroup(root_name+"/MODE_NADIR_BACKSCAN/"+gome1netcdf_bandName[selected_band]+"/GEODATA");
        clouddata_group = current_file.getGroup(root_name+"/MODE_NADIR_BACKSCAN/"+gome1netcdf_bandName[selected_band]+"/CLOUDDATA");
        obs_group = current_file.getGroup(root_name+"/MODE_NADIR_BACKSCAN/"+gome1netcdf_bandName[selected_band]+"/OBSERVATIONS");

        // Get the scanline and pixel size for backscans

        scan_size_bs=band_group.dimLen("scanline");
        pixel_size_bs=band_group.dimLen("ground_pixel");
        det_size=band_group.dimLen("spectral_channel");

        // Read the metadata

        backscan_geodata=GOME1NETCDF_Read_Geodata(geodata_group,scan_size_bs,pixel_size_bs);
        backscan_clouddata=GOME1NETCDF_Read_Clouddata(clouddata_group,scan_size_bs,pixel_size_bs);

        // Get the scanline indexes

        const size_t start[] = {0,0,0};
        const size_t count[] = {1,scan_size_bs,pixel_size_bs};

        obs_group.getVar("scanline",start,count,2,(int)-1,scanline_bs);
        obs_group.getVar("delta_time",start,count,3,(double)0.,deltatime_bs);
        band_group.getVar("start_pixel",start,count,1,(short)0,startpixel);
       }
     }

    // Assign size

    pEngineContext->recordNumber=scan_size*pixel_size+scan_size_bs*pixel_size_bs;  // get the total number of records (ground pixels + backscans)

    // Sort ground pixels and backscans using scanline

    start_pixel=startpixel[0];

    scanline_indexes.resize(pEngineContext->recordNumber);
    scanline_pixtype.resize(pEngineContext->recordNumber);
    alongtrack_indexes.resize(pEngineContext->recordNumber);
    delta_time.resize(pEngineContext->recordNumber);

    auto delta_time_scan = reinterpret_cast<const double(*)[pixel_size]>(deltatime.data());
    auto delta_time_scan_bs = reinterpret_cast<const double(*)[pixel_size_bs]>(deltatime_bs.data());
    pEngineContext->n_alongtrack=0;

    for (currentScanIndex=-1,                  // original scan line index
         i=0,                                  // index to browse elements in scanline
         j=0,                                  // index to browse elements in scanline_bs
         k=0;
       ((i<(int)scan_size) || (j<(int)scan_size_bs)) && (k<(int)pEngineContext->recordNumber);)
     {
      if ((i<(int)scan_size) && pixel_size ) // && (scanline[i]>(int)currentScanIndex))
       {
        for (n=0;n<(int)pixel_size;n++)
         {
          scanline_indexes[k+n]=i;
          scanline_pixtype[k+n]=n;
          alongtrack_indexes[k+n]=pEngineContext->n_alongtrack;
          delta_time[k+n]=delta_time_scan[i][n];
         }

        currentScanIndex=scanline[i];
        k+=n;
        i++;
       }

      if ((j<(int)scan_size_bs) && pixel_size_bs && (!pixel_size || (scanline_bs[j]==currentScanIndex)))
       {
        scanline_indexes[k]=j;
        scanline_pixtype[k]=3;
        alongtrack_indexes[k]=pEngineContext->n_alongtrack;
        delta_time[k]=delta_time_scan_bs[j][0];

        k++;
        j++;
       }

      pEngineContext->n_alongtrack++;
     }
    
    GOME1NETCDF_Get_Irradiance(pEngineContext,channel_index,pEngineContext->buffers.lambda_irrad,pEngineContext->buffers.irrad);

    for(i=0; i<MAX_SWATHSIZE; ++i)
     NDET[i]=(int)calibration.channel_size;

    pEngineContext->n_crosstrack=4;
   }
  catch (std::runtime_error& e)
   {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());  // in case of error, capture the message
   }

  // Return
  
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
//! \return  ERROR_ID_FILE_END if the requested record number is not found\n
//!          ERROR_ID_FILE_RECORD if the requested record doesn't satisfy current criteria (for example for the selection of the reference)\n
//!          ERROR_ID_NO on success
//!
// -----------------------------------------------------------------------------

RC GOME1NETCDF_Read(ENGINE_CONTEXT *pEngineContext,int recordNo)
 {
  // Declarations

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

  pRecordInfo=&pEngineContext->recordInfo;
  pRecordInfo->i_alongtrack=alongtrack_indexes[recordNo-1];                  // because in mediate, use +1
  pInstrumental=&pEngineContext->project.instrumental;
  selected_band=pInstrumental->gomenetcdf.bandType;
  rc = ERROR_ID_NO;

  // The requested record is out of range

  if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;
  else if (delta_time[recordNo-1]>(double)1.e36)
   rc=ERROR_ID_FILE_RECORD;
  else
   {
    size_t scanIndex=scanline_indexes[recordNo-1];                              // index in the ground pixel scanlines or backscan scanlines
    int    pixelType=scanline_pixtype[recordNo-1];                              // pixel type
    size_t pixelIndex=(pixelType==3)?0:pixelType;                               // index of the pixel in the scan : should be 0,1,2 for ground pixels and 0 for backscans
    int    pixelSize=(pixelType==3)?1:3;                                        // pixel size : should be 3 for ground pixels and 1 for backscans
    size_t start[] = {0,scanIndex,(pixelType==3)?0:pixelType,0};
    size_t count[] = {1,1,1,det_size};                                          // only one record to load

    getDate(delta_time[recordNo-1], &pRecordInfo->present_datetime);

    for (i=0;i<calibration.channel_size;i++)
     {
      pEngineContext->buffers.spectrum[i]=
      pEngineContext->buffers.sigmaSpec[i]=(double)0.;
     }


  // TODO SHORT_DATE irradDate;                                                         // date of measurement for the irradiance spectrum
  // TODO struct time irradTime;                                                        // time of measurement for the irradiance spectrum

  // TODO int     nRef;                                                                 // size of irradiance vectors

    geo=(pixelType==3)?backscan_geodata:ground_geodata;
    cloud=(pixelType==3)?backscan_clouddata:ground_clouddata;

    // Solar zenith angles

    auto sza = reinterpret_cast<const float(*)[pixelSize][3]>(geo.sza.data());
    auto saa = reinterpret_cast<const float(*)[pixelSize][3]>(geo.saa.data());
    auto vza = reinterpret_cast<const float(*)[pixelSize][3]>(geo.vza.data());
    auto vaa = reinterpret_cast<const float(*)[pixelSize][3]>(geo.vaa.data());

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

    obs_group = current_file.getGroup(root_name+((pixelType==3)?"/MODE_NADIR_BACKSCAN/":"/MODE_NADIR/")+gome1netcdf_bandName[selected_band]+"/OBSERVATIONS");

    // auto spec = reinterpret_cast<const float(*)[det_size]>(spe.data());

    obs_group.getVar("radiance",start,count,4,(float)0.,spe);
    obs_group.getVar("radiance_precision",start,count,4,(float)0.,err);
    obs_group.getVar("spectral_index",start,count,3,(short)0,clb);

    GOME1NETCDF_Get_Wavelength(pEngineContext,channel_index,(int)clb[0],pEngineContext->buffers.lambda);

    for (i=0,j=start_pixel;i<(int)det_size;i++,j++)
     {
      pEngineContext->buffers.spectrum[j]=spe[i];
      pEngineContext->buffers.sigmaSpec[j]=err[i];
     }

    obs_group.getVar("scanline",start,count,2,(int)0,scanline);

    pRecordInfo->gome.pixelNumber=scanline[0];                                  // pixel number
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
  current_file.close();

  calibration = calib();
  irradiance = refspec();
  ground_geodata = geodata();
  ground_clouddata = clouddata();
  backscan_geodata = geodata();
  backscan_clouddata = clouddata();

  scanline_indexes.clear();
  scanline_pixtype.clear();
  alongtrack_indexes.clear();
  delta_time.clear();
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
  std::istringstream orbit_start(current_file.getAttText("time_reference"));
  // time_coverage_start is formatted as "YYYY-MM-DD"
  char tmp; // to skip "-" chars
  orbit_start >> *orbit_year >> tmp >> *orbit_month >> tmp >> *orbit_day;
  return  orbit_start.good() ? 0 : 1;
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

  // TO CHANGE LATER GOME1NETCDF_ORBIT_FILE *pOrbitFile=&GOME1NETCDFOrbitFiles[GOME1NETCDFCurrentFileIndex];
  // const int n_wavel = pOrbitFile->GOME1NETCDFInfo.no_of_pixels;

  const int n_wavel=calibration.channel_size;
  int saveFlag= pEngineContext->project.spectra.displayDataFlag;

  RC rc=0; // TO CHANGE LATER pOrbitFile->rc;

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
   // TO CHANGE LATER    memcpy(pTabFeno->LambdaRef,pOrbitFile->GOME1NETCDFSunWve,sizeof(double) *n_wavel);
   // TO CHANGE LATER    memcpy(pTabFeno->Sref,pOrbitFile->GOME1NETCDFSunRef,sizeof(double) *n_wavel);


       memcpy(pTabFeno->LambdaRef,pEngineContext->buffers.lambda_irrad,sizeof(double) *n_wavel);
       memcpy(pTabFeno->Sref,pEngineContext->buffers.irrad,sizeof(double) *n_wavel);

       if (!TabFeno[0][indexFeno].hidden) {
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

    if (// ((rc=ANALYSE_UsampLocalAlloc(0)) !=ERROR_ID_NO) ||
        ((rc=ANALYSE_UsampBuild(0,0,indexFenoColumn)) !=ERROR_ID_NO) ||
        ((rc=ANALYSE_UsampBuild(1,ITEM_NONE,indexFenoColumn)) !=ERROR_ID_NO))

      goto EndGOME1NETCDF_LoadAnalysis;
  }
// Automatic reference selection
// TODO LATER if (gome1netCDF_loadReferenceFlag)
// TODO LATER   rc=GOME1NETCDFNewRef(pEngineContext,responseHandle);
// TODO LATER if (!rc) gome1netCDF_loadReferenceFlag=0;

EndGOME1NETCDF_LoadAnalysis:

  return rc;
}


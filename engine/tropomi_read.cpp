#include <string>
#include <vector>
#include <cassert>
#include <array>
#include <tuple>
#include <stdexcept>
#include <algorithm>
#include <sstream>

#include <cmath>

#include "tropomi.h"
#include "tropomi_read.h"
#include "netcdfwrapper.h"

#include "winthrd.h"
#include "comdefs.h"
#include "stdfunc.h"
#include "engine_context.h"
#include "analyse.h"

using std::string;
using std::vector;

// create an array {"UV1", "UV2", "UVIS1", ... } in same order as the
// tropomiSpectralBand enum:
const char* band_names[] = {
#define EXPAND(BAND, LABEL ) LABEL,
  TROPOMI_BANDS
#undef EXPAND
};

struct refspec {
  refspec() : lambda(), irradiance(), sigma() {};
  vector<double> lambda;
  vector<double> irradiance;
  vector<double> sigma;
};

// map of reference (irradiance) spectra: for each filename, keep a
// list of spectra for each detector row.
static vector<refspec> reference_spectra;

static NetCDFFile current_file;
static string current_band;
static vector< vector<double> > nominal_wavelengths; // L1B radiance wavelength calibration
static double fill_nominal_wavelengths; // fill value for L1B radiance wavelength calibration

struct geodata {
  vector<double> sza, vza, saa,  vaa,
    lon, lat,
    lon_bounds, lat_bounds,
    sat_lon, sat_lat;
};
static geodata current_geodata;

static size_t size_spectral; // number of wavelengths per spectrum
static size_t size_scanline; // number of measurements (i.e. along track)
static size_t size_groundpixel; // number of detector rows (i.e. cross-track)

static time_t reference_time; // since orbit start date
static vector<float> delta_time; // number of milliseconds after reference_time

// replace by functions using QDateTime?
static void getDate(int delta_t, struct datetime *date_time, int *pMs) {
  // TODO: handle UTC leap seconds?  Is this possible? (no UTC time info in file?)
  time_t thetime = reference_time + delta_t/1000;

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

  *pMs = static_cast<int>(delta_t) % 1000;
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

static geodata read_geodata(const NetCDFGroup& geo_group, size_t n_scanline, size_t n_groundpixel) {

  geodata result;

  // each element of this array contains the name of the netCDF
  // variable, the vector in which we want to store the data, and the
  // size of each element
  using std::ref;
  std::array<std::tuple<const string, vector<double>&, size_t>, 10> geovars {
    make_tuple("solar_zenith_angle", ref(result.sza), 1),
      make_tuple("viewing_zenith_angle", ref(result.vza), 1),
      make_tuple("solar_azimuth_angle", ref(result.saa), 1),
      make_tuple("viewing_azimuth_angle", ref(result.vaa), 1),
      make_tuple("latitude", ref(result.lat), 1),
      make_tuple("longitude", ref(result.lon), 1),
      make_tuple("longitude_bounds", ref(result.lon_bounds), 4),
      make_tuple("latitude_bounds", ref(result.lat_bounds), 4),
      make_tuple("satellite_longitude", ref(result.sat_lon), 1),
      make_tuple("satellite_latitude", ref(result.sat_lat), 1)};
  
  for (auto& var : geovars) {
    const string& name =std::get<0>(var);
    auto& target = std::get<1>(var);
    const size_t elem_size = std::get<2>(var);

    target.resize(n_scanline * n_groundpixel * elem_size);
    const size_t start[] = {0, 0, 0, 0};
    const size_t count[] = {1, n_scanline, n_groundpixel, elem_size};
    geo_group.getVar(name, start, count, target.data() );
  }

  return result;
}

int tropomi_set(ENGINE_CONTEXT *pEngineContext) {

  int rc = 0;
  try {
    current_file = NetCDFFile(pEngineContext->fileInfo.fileName);

    current_band = band_names[pEngineContext->project.instrumental.tropomi.spectralBand];

    set_reference_time(current_file.getAttText("time_reference"));

    NetCDFGroup obsGroup = current_file.getGroup(current_band + "_RADIANCE/STANDARD_MODE/OBSERVATIONS");

    size_scanline = obsGroup.dimLen("scanline");
    size_spectral = obsGroup.dimLen("spectral_channel");
    size_groundpixel = obsGroup.dimLen("ground_pixel");

    pEngineContext->recordNumber = size_groundpixel * size_scanline;
    pEngineContext->recordInfo.n_alongtrack= size_scanline;
    pEngineContext->recordInfo.n_crosstrack= size_groundpixel;

    size_t start[] = {0, 0};
    size_t count[] = {1, size_scanline};
    delta_time.resize(size_scanline);
    obsGroup.getVar("delta_time", start, count, &delta_time[0]);

    NetCDFGroup instrGroup = current_file.getGroup(current_band + "_RADIANCE/STANDARD_MODE/INSTRUMENT");
    size_t start_wl[] = {0, 0, 0};
    size_t count_wl[] = {1, 1, size_spectral};
    nominal_wavelengths.resize(size_groundpixel);

    for(size_t i=0; i<size_groundpixel; ++i) {
      nominal_wavelengths[i].resize(size_spectral);
      start_wl[1] = i;
      instrGroup.getVar("nominal_wavelength", start_wl, count_wl, &nominal_wavelengths[i][0]);
    }
    fill_nominal_wavelengths = instrGroup.getFillValue<double>("nominal_wavelength");

    const auto geo_group = current_file.getGroup(current_band + "_RADIANCE/STANDARD_MODE/GEODATA");
    current_geodata = read_geodata(geo_group, size_scanline, size_groundpixel);

  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return rc;
}

static void get_geodata(RECORD_INFO *pRecord, const geodata& geo, int record) {
  pRecord->latitude= geo.lat[record-1];
  pRecord->longitude= geo.lon[record-1];
  pRecord->Zm= geo.sza[record-1];
  pRecord->Azimuth= geo.saa[record-1];
  pRecord->zenithViewAngle= geo.vza[record-1];
  pRecord->azimuthViewAngle= geo.vaa[record-1];
}

int tropomi_read(ENGINE_CONTEXT *pEngineContext,int record) {

  assert(record > 0); // record is the requested record number, starting from 1
  int rc = 0;

  NetCDFGroup obsGroup = current_file.getGroup(current_band + "_RADIANCE/STANDARD_MODE/OBSERVATIONS");

  const size_t indexScanline = (record - 1) / size_groundpixel;
  const size_t indexPixel = (record - 1) % size_groundpixel;
  size_t n_wavel = 0;

  if (THRD_id==THREAD_TYPE_ANALYSIS) {
     // in analysis mode, variables must have been initialized by tropomi_init()
    assert(reference_spectra.size() == ANALYSE_swathSize);
    n_wavel = NDET[indexPixel];

    const refspec& ref = reference_spectra.at(indexPixel);
    for (size_t i=0; i<ref.lambda.size(); ++i) {
      pEngineContext->buffers.lambda_irrad[i] = ref.lambda[i];
      pEngineContext->buffers.irrad[i] = ref.irradiance[i];
    }
  } else {
    n_wavel = size_spectral;
  }

  // dimensions of radiance & error are
  // ('time','scanline','ground_pixel','spectral_channel')
  const size_t start[] = {0, indexScanline, indexPixel, 0};
  const size_t count[] = {1, 1, 1, size_spectral};

  vector<double> rad(size_spectral);
  vector<double> rad_noise(size_spectral);
  try {
    obsGroup.getVar("radiance", start, count, rad.data() );
    obsGroup.getVar("radiance_noise", start, count, rad_noise.data() );

    const double fill_rad = obsGroup.getFillValue<double>("radiance");
    const double fill_noise = obsGroup.getFillValue<double>("radiance_noise");
    const vector<double>& lambda = nominal_wavelengths.at(indexPixel);
    
    // copy non-fill values to buffers:
    size_t j=0;
    for (size_t i=0; i<rad.size() && j<n_wavel; ++i) {
      double li = lambda[i];
      double ri = rad[i];
      double ni = rad_noise[i];
      if (li != fill_nominal_wavelengths && ri != fill_rad && ni != fill_noise) {
        pEngineContext->buffers.lambda[j]=li;
        pEngineContext->buffers.spectrum[j]=ri;
        pEngineContext->buffers.sigmaSpec[j]=ri/(std::pow(10.0, ni/10.0));
        ++j;
      }
    }
    // check if the earthshine spectrum is shorter than the reference
    // spectrum (e.g.due to different number of fill values).
    if (j<n_wavel) {
      // This is not a very clean solution, but we assume that
      // reducing NDET[i] is always safe:
      NDET[indexPixel] = j;
    }
  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }

  RECORD_INFO *pRecord = &pEngineContext->recordInfo;

  pRecord->i_alongtrack = indexScanline;
  pRecord->i_crosstrack = indexPixel;
  pRecord->useErrors = 1;
  get_geodata(pRecord, current_geodata, record);

  int ms;
  getDate(delta_time[indexScanline], &pRecord->present_datetime, &ms);

  return rc;
}

vector<refspec> loadReference(const string& filename, const string& band) {
  NetCDFFile refFile(filename);
  auto irrObsGroup = refFile.getGroup(band + "_IRRADIANCE/STANDARD_MODE/OBSERVATIONS");
  auto instrGroup = refFile.getGroup(band + "_IRRADIANCE/STANDARD_MODE/INSTRUMENT");
  // get dimensions:
  size_t size_pixel = irrObsGroup.dimLen("pixel");
  size_t nlambda = irrObsGroup.dimLen("spectral_channel");

  vector<refspec> result(size_pixel);
  for(size_t i=0; i<size_pixel; ++i) {
    refspec& ref_pixel = result[i];

    // irradiance & irradiance_noise have dimensions
    // (time, scanline, pixel, spectral_channel)
    size_t start_irr[] = {0, 0, i, 0};
    size_t count_irr[] = {1, 1, 1, nlambda};

    // calibrated_wavelength has dimensions
    // (time, pixel, spectral_channel)
    size_t start_lambda[] = {0, i, 0};
    size_t count_lambda[] = {1, 1, nlambda};

    vector<double> lambda(nlambda);
    vector<double> irra(nlambda);
    vector<double> noise(nlambda);

    instrGroup.getVar("calibrated_wavelength", start_lambda, count_lambda, lambda.data());
    irrObsGroup.getVar("irradiance", start_irr, count_irr, irra.data());
    irrObsGroup.getVar("irradiance_noise", start_irr, count_irr, noise.data());

    const double fill_lambda = instrGroup.getFillValue<double>("calibrated_wavelength");
    const double fill_irra = irrObsGroup.getFillValue<double>("irradiance");
    const double fill_noise = irrObsGroup.getFillValue<double>("irradiance_noise");

    for (size_t j=0; j<nlambda; ++j) {
      double lj = lambda[j];
      double ij = irra[j];
      double nj = noise[j];
      if (lj != fill_lambda && ij != fill_irra && nj != fill_noise) {
        ref_pixel.lambda.push_back(lj);
        ref_pixel.irradiance.push_back(ij);
        ref_pixel.sigma.push_back(ij/(std::pow(10.0, nj/10.0)));
      }
    }
  }

  return result;
}

int tropomi_init(const char *ref_filename, ENGINE_CONTEXT *pEngineContext) {

  RC rc = ERROR_ID_NO;
  try {
    reference_spectra = loadReference(ref_filename, 
                                      band_names[pEngineContext->project.instrumental.tropomi.spectralBand]);
  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }

  ANALYSE_swathSize = reference_spectra.size();

  for(int i=0; i<ANALYSE_swathSize; ++i) {
    NDET[i] = reference_spectra[i].lambda.size();
    if (NDET[i] != 0) {
      pEngineContext->project.instrumental.use_row[i] = true;
    }
  }

  return rc;
}

int tropomi_get_reference(const char *filename, int pixel,
                          double *lambda, double *spectrum, double *sigma, int *n_wavel) {
  int rc = ERROR_ID_NO;

  try {
    const refspec& r = reference_spectra.at(pixel);

    if (!r.irradiance.size() )
      return ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_REF_DATA, pixel);

    *n_wavel = r.irradiance.size();

    for (size_t i = 0; i < r.irradiance.size(); ++i) {
      lambda[i] = r.lambda[i];
      spectrum[i] = r.irradiance[i];
      sigma[i] = r.sigma[i];
    }
  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_TROPOMI_REF, filename, pixel, e.what());
  }

  return rc;
}

int tropomi_get_orbit_date(int *orbit_year, int *orbit_month, int *orbit_day) {
  std::istringstream orbit_start(current_file.getAttText("time_coverage_start"));
  // time_coverage_start is formatted as "YYYY-MM-DD"
  char tmp; // to skip "-" chars
  orbit_start >> *orbit_year >> tmp >> *orbit_month >> tmp >> *orbit_day;
  return  orbit_start.good() ? 0 : 1;
}

void tropomi_cleanup(void) {
  current_file.close();
  
  current_geodata = geodata();
  current_band = "";
  
  size_spectral = size_scanline = size_groundpixel = 0;
  reference_time = 0;

  nominal_wavelengths.clear();
  delta_time.clear();
  reference_spectra.clear();
}

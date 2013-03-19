/*! \file output_private.h \brief Contains functions that can be used
  to extract output data and store it in the appropriate output_field
  buffers.  For a registered output field, output_field::get_data will
  normally contain a pointer to one ofthese functions.*/

#ifdef output_c //only include this file in output_c
#include "engine.h"

#define DEFAULT_VALUE 9999.

RC write_spikes(char *spikestring, unsigned int length, BOOL *spikes,int ndet);
double output_flux(const ENGINE_CONTEXT *pEngineContext, double wavelength);

static inline FENO *get_tabfeno_calib(struct output_field *this_field __attribute__ ((unused)), int indexFenoColumn) {
  return &TabFeno[indexFenoColumn][KURUCZ_buffers[indexFenoColumn].indexKurucz];
}

static inline FENO *get_tabfeno_analysis(struct output_field *this_field, int indexFenoColumn) {
  return &TabFeno[indexFenoColumn][this_field->index_feno];
}

static inline CROSS_RESULTS *get_cross_results_calib(struct output_field *this_field, int indexFenoColumn, int index_calib) {
  FENO *pTabFeno = &TabFeno[indexFenoColumn][KURUCZ_buffers[indexFenoColumn].indexKurucz];
  return (pTabFeno->rc)
    ? NULL
    : &KURUCZ_buffers[indexFenoColumn].KuruczFeno[this_field->index_feno].results[index_calib][this_field->index_cross];
}

static inline CROSS_RESULTS *get_cross_results(struct output_field *this_field, int indexFenoColumn, int index_calib __attribute__ ((unused)) ) {
  FENO * pTabFeno = &TabFeno[indexFenoColumn][this_field->index_feno];
  return (pTabFeno->rc) 
    ? NULL
    : &pTabFeno->TabCrossResults[this_field->index_cross];
}

static inline void get_specno(struct output_field *this_field __attribute__ ((unused)), int *specno, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *specno = pEngineContext->indexRecord;
}

static inline void get_name(struct output_field *this_field __attribute__ ((unused)), char **name, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *name = strdup(pEngineContext->recordInfo.Nom);
}

static inline void get_date(struct output_field *this_field __attribute__ ((unused)), struct date *date, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  SHORT_DATE datein = pEngineContext->recordInfo.present_day;
  date->da_day = datein.da_day;
  date->da_mon = datein.da_mon;
  date->da_year = datein.da_year;
}

static inline void get_time(struct output_field *this_field __attribute__ ((unused)), struct time *time, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *time = pEngineContext->recordInfo.present_time;
}

static inline void get_datetime(struct output_field *this_field __attribute__ ((unused)), struct datetime *datetime, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  get_date(this_field, &datetime->thedate, pEngineContext, indexFenoColumn, index_calib);
  datetime->thetime = pEngineContext->recordInfo.present_time;
  datetime->millis = -1;
}

static inline void scia_get_datetime(struct output_field *this_field __attribute__ ((unused)), struct datetime *datetime, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  get_datetime(this_field, datetime, pEngineContext, indexFenoColumn, index_calib);
  datetime->millis =  SCIA_ms;
  datetime->microseconds = -1;
}

static inline void gome2_get_datetime(struct output_field *this_field, struct datetime *datetime, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib __attribute__ ((unused))) {
  get_datetime(this_field, datetime, pEngineContext, indexFenoColumn, index_calib);
  datetime->microseconds = GOME2_mus;
  datetime->millis = -1;
}

static inline void get_year(struct output_field *this_field __attribute__ ((unused)), unsigned short *year, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *year = pEngineContext->recordInfo.present_day.da_year;
}

static inline void get_start_time(struct output_field *this_field __attribute__ ((unused)), struct time *time, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *time = pEngineContext->recordInfo.startTime;
}

static inline void get_end_time(struct output_field *this_field __attribute__ ((unused)), struct time *time, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *time = pEngineContext->recordInfo.endTime;
}

static inline void get_julian(struct output_field *this_field __attribute__ ((unused)), unsigned short *julian, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *julian = ZEN_FNCaljda(&pEngineContext->recordInfo.Tm);
}

static inline void get_frac_julian(struct output_field *this_field __attribute__ ((unused)), double *julian, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *julian = ZEN_FNCaljda(&pEngineContext->recordInfo.Tm) + ZEN_FNCaldti(&pEngineContext->recordInfo.Tm)/24.;
}

static inline void get_frac_time(struct output_field *this_field __attribute__ ((unused)), double *frac_time, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *frac_time = ZEN_FNCaldti(&pEngineContext->recordInfo.Tm);
}

static inline void get_frac_time_recordinfo(struct output_field *this_field __attribute__ ((unused)), double *frac_time, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *frac_time = pEngineContext->recordInfo.TimeDec;
}

static inline void get_scans(struct output_field *this_field __attribute__ ((unused)), unsigned short *scans, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *scans = pEngineContext->recordInfo.NSomme;
}

static inline void get_n_rejected(struct output_field *this_field __attribute__ ((unused)), unsigned short *n_rejected, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *n_rejected = pEngineContext->recordInfo.rejected;
}

static inline void get_t_int(struct output_field *this_field __attribute__ ((unused)), double *t_int, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *t_int = pEngineContext->recordInfo.Tint;
}

static inline void scia_get_sza(struct output_field *this_field __attribute__ ((unused)), float *sza, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(sza, pEngineContext->recordInfo.scia.solZen, 3 * sizeof(*sza));
}

static inline void gome2_get_sza(struct output_field *this_field __attribute__ ((unused)), float *sza, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(sza, pEngineContext->recordInfo.gome2.solZen, 3 * sizeof(*sza));
}

static inline void gdp3_get_sza(struct output_field *this_field __attribute__ ((unused)), float *sza, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  float *sza_gdp = GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo3.szaArray;
  memcpy(sza, sza_gdp, 3 * sizeof(*sza));
}

static inline void gdp4_get_sza(struct output_field *this_field __attribute__ ((unused)), float *sza, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  float *sza_gdp = GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo4.szaArrayBOA;
  memcpy(sza, sza_gdp, 3 * sizeof(*sza));
}

static inline void gdpasc_get_sza(struct output_field *this_field __attribute__ ((unused)), float *sza, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(sza, pEngineContext->recordInfo.gome.sza, 3 * sizeof(*sza));
}

static inline void get_sza(struct output_field *this_field __attribute__ ((unused)), float *sza, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *sza = pEngineContext->recordInfo.Zm;
}

static inline void scia_get_azim(struct output_field *this_field __attribute__ ((unused)), float *azim, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(azim, pEngineContext->recordInfo.scia.solAzi, 3 * sizeof(*azim));
}

static inline void gome2_get_azim(struct output_field *this_field __attribute__ ((unused)), float *azim, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(azim, pEngineContext->recordInfo.gome2.solAzi, 3 * sizeof(*azim));
}

static inline void gdp3_get_azim(struct output_field *this_field __attribute__ ((unused)), float *azim, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  float *azim_gdp = GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo3.aziArray;
  memcpy(azim, azim_gdp, 3 * sizeof(*azim));
}

static inline void gdp4_get_azim(struct output_field *this_field __attribute__ ((unused)), float *azim, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  float *azim_gdp = GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo4.aziArrayBOA;
  memcpy(azim, azim_gdp, 3 * sizeof(*azim));
}

static inline void gdpasc_get_azim(struct output_field *this_field __attribute__ ((unused)), float *azim, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(azim, pEngineContext->recordInfo.gome.azim, 3 * sizeof(*azim));
}

static inline void get_azim(struct output_field *this_field __attribute__ ((unused)), float *azim, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *azim = pEngineContext->recordInfo.Azimuth;
}

static inline void get_tdet(struct output_field *this_field __attribute__ ((unused)), float *tdet, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *tdet = pEngineContext->recordInfo.TDet;
}

static inline void get_sky(struct output_field *this_field __attribute__ ((unused)), unsigned short *skyobs, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *skyobs = pEngineContext->recordInfo.SkyObs;
}

static inline void get_bestshift(struct output_field *this_field __attribute__ ((unused)), float *bestshift, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *bestshift = pEngineContext->recordInfo.BestShift;
}

static inline void get_pixel_number(struct output_field *this_field __attribute__ ((unused)), unsigned short *pixel_number, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *pixel_number = pEngineContext->recordInfo.gome.pixelNumber;
}

static inline void get_pixel_type(struct output_field *this_field __attribute__ ((unused)), unsigned short *pixel_type, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *pixel_type = pEngineContext->recordInfo.gome.pixelType;
}

static inline void gdp_get_orbit_number(struct output_field *this_field __attribute__ ((unused)), int *orbit_number, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *orbit_number = pEngineContext->recordInfo.gome.orbitNumber+1;
}

static inline void scia_get_orbit_number(struct output_field *this_field __attribute__ ((unused)), int *orbit_number, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *orbit_number = pEngineContext->recordInfo.scia.orbitNumber;
}

static inline void gome2_get_orbit_number(struct output_field *this_field __attribute__ ((unused)), int *orbit_number, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *orbit_number = pEngineContext->recordInfo.gome2.orbitNumber;
}

static inline void get_cloud_fraction(struct output_field *this_field __attribute__ ((unused)), float *cloud_fraction, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *cloud_fraction = pEngineContext->recordInfo.cloudFraction;
}

static inline void get_cloud_top_pressure(struct output_field *this_field __attribute__ ((unused)), float *cloud_top_pressure, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *cloud_top_pressure = pEngineContext->recordInfo.cloudTopPressure;
}

static inline void get_scia_state_index(struct output_field *this_field __attribute__ ((unused)), unsigned short *scia_state_index, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *scia_state_index = pEngineContext->recordInfo.scia.stateIndex;
}

static inline void get_scia_state_id(struct output_field *this_field __attribute__ ((unused)), unsigned short *scia_state_id, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *scia_state_id = pEngineContext->recordInfo.scia.stateId;
}

static inline void get_scia_quality(struct output_field *this_field __attribute__ ((unused)), unsigned short *scia_quality, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *scia_quality = pEngineContext->recordInfo.scia.qualityFlag;
}

static inline void gdp3_get_sat_height(struct output_field *this_field __attribute__ ((unused)), float *sat_height, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *sat_height = GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo3.satHeight;
}

static inline void gdp4_get_sat_height(struct output_field *this_field __attribute__ ((unused)), float *sat_height, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *sat_height = GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo4.satHeight;
}

static inline void scia_get_sat_height(struct output_field *this_field __attribute__ ((unused)), float *sat_height, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *sat_height = pEngineContext->recordInfo.scia.satHeight;
}

static inline void gome2_get_sat_height(struct output_field *this_field __attribute__ ((unused)), float *sat_height, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *sat_height = pEngineContext->recordInfo.gome2.satHeight;
}

static inline void gdp3_get_earth_radius(struct output_field *this_field __attribute__ ((unused)), float *earth_radius, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *earth_radius = GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo3.radiusCurve;
}

static inline void gdp4_get_earth_radius(struct output_field *this_field __attribute__ ((unused)), float *earth_radius, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *earth_radius = GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo4.radiusCurve;
}

static inline void scia_get_earth_radius(struct output_field *this_field __attribute__ ((unused)), float *earth_radius, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *earth_radius = pEngineContext->recordInfo.scia.earthRadius;
}

static inline void gome2_get_earth_radius(struct output_field *this_field __attribute__ ((unused)), float *earth_radius, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *earth_radius = pEngineContext->recordInfo.gome2.earthRadius;
}

static inline void get_view_elevation(struct output_field *this_field __attribute__ ((unused)), float *view_elevation, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *view_elevation = pEngineContext->recordInfo.elevationViewAngle;
}

static inline void get_view_zenith(struct output_field *this_field __attribute__ ((unused)), float *view_zenith, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *view_zenith = pEngineContext->recordInfo.zenithViewAngle;
}

static inline void get_view_azimuth(struct output_field *this_field __attribute__ ((unused)), float *view_azimuth, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *view_azimuth = pEngineContext->recordInfo.azimuthViewAngle;
}

static inline void scia_get_los_zenith(struct output_field *this_field __attribute__ ((unused)), float *los_zenith, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(los_zenith, pEngineContext->recordInfo.scia.losZen, 3 * sizeof(*los_zenith));
}

static inline void gome2_get_los_zenith(struct output_field *this_field __attribute__ ((unused)), float *los_zenith, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(los_zenith, pEngineContext->recordInfo.gome2.losZen, 3 * sizeof(*los_zenith));
}

static inline void gdp3_get_los_zenith(struct output_field *this_field __attribute__ ((unused)), float *los_zenith, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  for(int i=0; i<3; i++) {
    los_zenith[i] = GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo3.losZa[i] * 0.01;
  }
}

static inline void gdp4_get_los_zenith(struct output_field *this_field __attribute__ ((unused)), float *los_zenith, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(los_zenith, GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo4.losZaBOA, 3 * sizeof(*los_zenith));
}

static inline void get_los_zenith (struct output_field *this_field __attribute__ ((unused)), float *los_zenith, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *los_zenith = pEngineContext->recordInfo.zenithViewAngle;
}

static inline void scia_get_los_azimuth(struct output_field *this_field __attribute__ ((unused)), float *los_azimuth, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(los_azimuth, pEngineContext->recordInfo.scia.losAzi, 3 * sizeof(*los_azimuth));
}

static inline void gome2_get_los_azimuth(struct output_field *this_field __attribute__ ((unused)), float *los_azimuth, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(los_azimuth, pEngineContext->recordInfo.gome2.losAzi, 3 * sizeof(*los_azimuth));
}

static inline void gdp3_get_los_azimuth(struct output_field *this_field __attribute__ ((unused)), float *los_azimuth, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  for(int i=0; i<3; i++) {
    los_azimuth[i] = GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo3.losAzim[i] * 0.01;
  }
}

static inline void gdp4_get_los_azimuth(struct output_field *this_field __attribute__ ((unused)), float *los_azimuth, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(los_azimuth, GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo4.losZaBOA, 3 * sizeof(*los_azimuth));
}

static inline void get_los_azimuth (struct output_field *this_field __attribute__ ((unused)), float *los_azimuth, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *los_azimuth = pEngineContext->recordInfo.azimuthViewAngle;
}

static inline void get_longitude (struct output_field *this_field __attribute__ ((unused)), float *longitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *longitude = pEngineContext->recordInfo.longitude;
}

static inline void scia_get_corner_longitudes(struct output_field *this_field __attribute__ ((unused)), float *longitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  for(int i=0; i<4; i++)
    longitude[i] = (float)pEngineContext->recordInfo.scia.longitudes[i];
}

static inline void gome2_get_corner_longitudes(struct output_field *this_field __attribute__ ((unused)), float *longitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  for(int i=0; i<4; i++)
    longitude[i] = (float)pEngineContext->recordInfo.gome2.longitudes[i];
}

static inline void gdp3_get_corner_longitudes(struct output_field *this_field __attribute__ ((unused)), float *longitude, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  for (int i=0; i<4; i++) {
    longitude[i] = 0.01 * GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo3.lonArray[i];
  }
}

static inline void gdp4_get_corner_longitudes(struct output_field *this_field __attribute__ ((unused)), float *longitude, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  for (int i=0; i<4; i++) {
    longitude[i] = 0.01 * GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo4.lonArray[i];
  }
}

static inline void gdpasc_get_corner_longitudes(struct output_field *this_field __attribute__ ((unused)), float *longitude, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(longitude, pEngineContext->recordInfo.gome.longit, 4 * sizeof(*longitude));
}

static inline void get_latitude (struct output_field *this_field __attribute__ ((unused)), float *latitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *latitude = pEngineContext->recordInfo.latitude;
}

static inline void scia_get_corner_latitudes(struct output_field *this_field __attribute__ ((unused)), float *latitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  for(int i=0; i<4; i++)
    latitude[i] = (float)pEngineContext->recordInfo.scia.latitudes[i];
}

static inline void gome2_get_corner_latitudes(struct output_field *this_field __attribute__ ((unused)), float *latitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  for(int i=0; i<4; i++)
    latitude[i] = (float)pEngineContext->recordInfo.gome2.latitudes[i];
}

static inline void gdp3_get_corner_latitudes(struct output_field *this_field __attribute__ ((unused)), float *latitude, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  for (int i=0; i<4; i++) {
    latitude[i] = 0.01 * GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo3.latArray[i];
  }
}

static inline void gdp4_get_corner_latitudes(struct output_field *this_field __attribute__ ((unused)), float *latitude, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  for (int i=0; i<4; i++) {
    latitude[i] = 0.01 * GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinSpectrum.geo.geo4.latArray[i];
  }
}

static inline void gdpasc_get_corner_latitudes(struct output_field *this_field __attribute__ ((unused)), float *latitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(latitude, pEngineContext->recordInfo.gome.latit, 4 * sizeof(*latitude));
}

static inline void get_altitude(struct output_field *this_field __attribute__ ((unused)), float *altitude, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *altitude = pEngineContext->recordInfo.altitude;
}

static inline void mkzy_get_scanning_angle(struct output_field *this_field __attribute__ ((unused)), float *scanning_angle, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *scanning_angle = pEngineContext->recordInfo.mkzy.scanningAngle;
}

static inline void get_scanning_angle(struct output_field *this_field __attribute__ ((unused)), float *scanning_angle, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *scanning_angle = pEngineContext->recordInfo.als.scanningAngle;
}

static inline void get_compass_angle(struct output_field *this_field __attribute__ ((unused)), float *compass_angle, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *compass_angle = pEngineContext->recordInfo.als.compassAngle;
}

static inline void get_pitch_angle(struct output_field *this_field __attribute__ ((unused)), float *pitch_angle, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *pitch_angle = pEngineContext->recordInfo.als.pitchAngle;
}

static inline void get_roll_angle(struct output_field *this_field __attribute__ ((unused)), float *roll_angle, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *roll_angle = pEngineContext->recordInfo.als.rollAngle;
}

static inline void get_filter_number(struct output_field *this_field __attribute__ ((unused)), int *filter_number, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *filter_number = pEngineContext->recordInfo.ccd.filterNumber;
}

static inline void ccd_get_meastype(struct output_field *this_field __attribute__ ((unused)), int *meastype, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *meastype = pEngineContext->recordInfo.ccd.measureType;
}

static inline void mfc_get_meastype(struct output_field *this_field __attribute__ ((unused)), int *meastype, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *meastype = pEngineContext->recordInfo.mfcBira.measurementType;
}

static inline void ccd_get_head_temperature(struct output_field *this_field __attribute__ ((unused)), double *head_temperature, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *head_temperature = pEngineContext->recordInfo.ccd.headTemperature;
}

static inline void get_cooling_status(struct output_field *this_field __attribute__ ((unused)), int *cooling_status, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *cooling_status = pEngineContext->recordInfo.coolingStatus;
}

static inline void get_mirror_error(struct output_field *this_field __attribute__ ((unused)), int *mirror_error, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *mirror_error = pEngineContext->recordInfo.mirrorError;
}

static inline void gome2_get_scan_direction(struct output_field *this_field __attribute__ ((unused)), int *scan_direction, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *scan_direction = pEngineContext->recordInfo.gome2.scanDirection;
}

static inline void gome2_get_saa(struct output_field *this_field __attribute__ ((unused)), int *saa, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *saa = pEngineContext->recordInfo.gome2.saaFlag;
}

static inline void gome2_get_sunglint_risk(struct output_field *this_field __attribute__ ((unused)), int *sunglint_risk, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *sunglint_risk = pEngineContext->recordInfo.gome2.sunglintDangerFlag;
}

static inline void gome2_get_sunglint_high_risk(struct output_field *this_field __attribute__ ((unused)), int *sunglint_high_risk, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *sunglint_high_risk = pEngineContext->recordInfo.gome2.sunglintHighDangerFlag;
}

static inline void gome2_get_rainbow(struct output_field *this_field __attribute__ ((unused)), int *rainbow, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *rainbow = pEngineContext->recordInfo.gome2.rainbowFlag;
}

static inline void get_diodes(struct output_field *this_field __attribute__ ((unused)), float *diodes, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  memcpy(diodes, pEngineContext->recordInfo.ccd.diodes, 4 * sizeof(*diodes));
}

static inline void get_target_azimuth(struct output_field *this_field __attribute__ ((unused)), float *azimuth, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *azimuth = pEngineContext->recordInfo.ccd.targetAzimuth;
}

static inline void get_target_elevation(struct output_field *this_field __attribute__ ((unused)), float *elevation, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *elevation = pEngineContext->recordInfo.ccd.targetElevation;
}

static inline void get_saturated_flag(struct output_field *this_field __attribute__ ((unused)), unsigned short *saturated_flag, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *saturated_flag = pEngineContext->recordInfo.ccd.saturatedFlag;
}

static inline void get_omi_measurement_number(struct output_field *this_field __attribute__ ((unused)), int *row, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *row = pEngineContext->recordInfo.omi.omiMeasurementIndex;
}

static inline void get_omi_row(struct output_field *this_field __attribute__ ((unused)), int *row, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *row = pEngineContext->recordInfo.omi.omiRowIndex;
}

static inline void get_omi_groundpixelqf(struct output_field *this_field __attribute__ ((unused)), unsigned short *groundpixelqf, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *groundpixelqf = pEngineContext->recordInfo.omi.omiGroundPQF;
}

static inline void get_omi_xtrackqf(struct output_field *this_field __attribute__ ((unused)), unsigned short *xtrackqf, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *xtrackqf = pEngineContext->recordInfo.omi.omiXtrackQF;
}

static inline void get_uav_servo_byte_sent(struct output_field *this_field __attribute__ ((unused)), unsigned short *servo_sent_position, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *servo_sent_position = pEngineContext->recordInfo.uavBira.servoSentPosition;
}

static inline void get_uav_servo_byte_received(struct output_field *this_field __attribute__ ((unused)), unsigned short *servo_received_position, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *servo_received_position = pEngineContext->recordInfo.uavBira.servoReceivedPosition;
}

static inline void get_slant_column(struct output_field *this_field, double *slant_column, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *slant_column = ( pTabCrossResults && pTabCrossResults->SlntFact!=(double)0.) 
    ? pTabCrossResults->SlntCol/pTabCrossResults->SlntFact
    : DEFAULT_VALUE;
}

static inline void get_slant_err(struct output_field *this_field, double *slant_err, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *slant_err = ( pTabCrossResults && pTabCrossResults->SlntFact!=(double)0.) 
    ? pTabCrossResults->SlntErr/pTabCrossResults->SlntFact
    : DEFAULT_VALUE;
}

static inline void get_shift(struct output_field *this_field, double *shift, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *shift = ( pTabCrossResults ) ? pTabCrossResults->Shift : DEFAULT_VALUE;
}

static inline void get_shift_err(struct output_field *this_field, double *shift_err, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *shift_err = ( pTabCrossResults ) ? pTabCrossResults->SigmaShift : DEFAULT_VALUE;
}

static inline void get_stretch(struct output_field *this_field, double *stretch, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *stretch = ( pTabCrossResults ) ? pTabCrossResults->Stretch : DEFAULT_VALUE;
}

static inline void get_stretch2(struct output_field *this_field, double *stretch, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *stretch = ( pTabCrossResults ) ? pTabCrossResults->Stretch2 : DEFAULT_VALUE;
}

static inline void get_stretches(struct output_field *this_field, double *stretch, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  if (pTabCrossResults) {
    stretch[0] = pTabCrossResults->Stretch;
    stretch[1] = pTabCrossResults->Stretch2;
  } else {
    stretch[0] = stretch[1] = DEFAULT_VALUE;
  }
}

static inline void get_stretch_err(struct output_field *this_field, double *stretch_error, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *stretch_error = (pTabCrossResults) ? pTabCrossResults->SigmaStretch : DEFAULT_VALUE;
}

static inline void get_stretch2_err(struct output_field *this_field, double *stretch_error, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *stretch_error = (pTabCrossResults) ? pTabCrossResults->SigmaStretch2 : DEFAULT_VALUE;
}

static inline void get_stretch_errors(struct output_field *this_field, double *stretch_errors, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  if (pTabCrossResults) {
    stretch_errors[0] = pTabCrossResults->SigmaStretch;
    stretch_errors[1] = pTabCrossResults->SigmaStretch2;
  } else {
    stretch_errors[0] = stretch_errors[1] = DEFAULT_VALUE;
  }
}

static inline void get_scale(struct output_field *this_field, double *scale, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *scale = ( pTabCrossResults ) ? pTabCrossResults->Scale : DEFAULT_VALUE;
}

static inline void get_scale2(struct output_field *this_field, double *scale, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *scale = ( pTabCrossResults ) ? pTabCrossResults->Scale2 : DEFAULT_VALUE;
}

static inline void get_scales(struct output_field *this_field, double *scales, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  if (pTabCrossResults) {
    scales[0] = pTabCrossResults->Scale;
    scales[1] = pTabCrossResults->Scale2;
  } else {
    scales[0] = scales[1] = DEFAULT_VALUE;
  }
}

static inline void get_scale_err(struct output_field *this_field, double *scale_error, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *scale_error = (pTabCrossResults)
    ? pTabCrossResults->SigmaScale
    : DEFAULT_VALUE;
}

static inline void get_scale2_err(struct output_field *this_field, double *scale_error, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *scale_error = (pTabCrossResults)
    ? pTabCrossResults->SigmaScale2
    : DEFAULT_VALUE;
}

static inline void get_scale_errors(struct output_field *this_field, double *scale_errors, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  if (pTabCrossResults) {
    scale_errors[0] = pTabCrossResults->SigmaScale;
    scale_errors[1] = pTabCrossResults->SigmaScale2;
  } else {
    scale_errors[0] = scale_errors[1] = DEFAULT_VALUE;
  }
}

static inline void get_param(struct output_field *this_field, double *param, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  if (pTabCrossResults) {
    *param = (this_field->get_tabfeno(this_field, indexFenoColumn)->TabCross[this_field->index_cross].IndSvdA)
      ? pTabCrossResults->SlntCol 
      : pTabCrossResults->Param;
  } else {
    *param = DEFAULT_VALUE;
  }
}

static inline void get_param_err(struct output_field *this_field, double *param_err, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  if (pTabCrossResults) {
    *param_err = (this_field->get_tabfeno(this_field, indexFenoColumn)->TabCross[this_field->index_cross].IndSvdA)
      ? pTabCrossResults->SlntErr
      : pTabCrossResults->SigmaParam;
  } else {
    *param_err = DEFAULT_VALUE;
  }
}

static inline void get_amf(struct output_field *this_field, float *amf, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *amf = pTabCrossResults ? pTabCrossResults->Amf : DEFAULT_VALUE;
}

static inline void get_vrt_col(struct output_field *this_field, double *vrt_col, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *vrt_col = ( pTabCrossResults && (pTabCrossResults->VrtFact != (double)0.) ) 
    ? pTabCrossResults->VrtCol/pTabCrossResults->VrtFact
    : DEFAULT_VALUE;
}

static inline void get_vrt_err(struct output_field *this_field, double *vrt_err, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  CROSS_RESULTS *pTabCrossResults = this_field->get_cross_results(this_field, indexFenoColumn, index_calib);
  *vrt_err = ( pTabCrossResults && (pTabCrossResults->VrtFact!=(double)0.) )
    ? pTabCrossResults->VrtErr/pTabCrossResults->VrtFact
    : DEFAULT_VALUE;
}

static inline void get_refzm(struct output_field *this_field, float *refzm, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib __attribute__ ((unused))) {
  *refzm = (float) this_field->get_tabfeno(this_field, indexFenoColumn)->Zm;
}

static inline void get_ref_shift(struct output_field *this_field, float *ref_shift, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib __attribute__ ((unused))) {
  *ref_shift = (float) this_field->get_tabfeno(this_field, indexFenoColumn)->Shift;
}

static inline void get_corr(struct output_field *this_field, double *corr, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib __attribute__ ((unused))) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  if (pTabFeno && pTabFeno->TabCross[this_field->index_cross].Fact != (double) 0.) {
    CROSS_REFERENCE *TabCross = pTabFeno->TabCross;
    *corr = pTabFeno->svd.covar[TabCross[this_field->index_cross2].IndSvdA][TabCross[this_field->index_cross].IndSvdA]*pTabFeno->chiSquare/
      (TabCross[this_field->index_cross].Fact*TabCross[this_field->index_cross2].Fact);
  } else
    *corr = DEFAULT_VALUE;
}

static inline void get_covar(struct output_field *this_field, double *covar, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib __attribute__ ((unused))) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  if (pTabFeno && pTabFeno->TabCross[this_field->index_cross].Fact != (double) 0.) {
    CROSS_REFERENCE *TabCross = pTabFeno->TabCross;
    *covar = pTabFeno->svd.covar[TabCross[this_field->index_cross2].IndSvdA][TabCross[this_field->index_cross].IndSvdA]*pTabFeno->chiSquare/
      (TabCross[this_field->index_cross].Fact*TabCross[this_field->index_cross2].Fact 
       * pTabFeno->TabCrossResults[this_field->index_cross].SlntErr *  pTabFeno->TabCrossResults[this_field->index_cross2].SlntErr);
  }
}

static inline void get_spikes(struct output_field *this_field, char **spike_list, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib __attribute__ ((unused))) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  *spike_list = malloc(50);
  write_spikes(*spike_list, 50, pTabFeno->spikes, pTabFeno->NDET);
}

static inline void omi_get_rejected_pixels(struct output_field *this_field, char **spike_list, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib __attribute__ ((unused))) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  *spike_list = malloc(50);
  write_spikes(*spike_list, 50, pTabFeno->omiRejPixelsQF, pTabFeno->NDET);
}

static inline void get_omi_rejected_pixels(struct output_field *this_field, char **pixel_list, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib __attribute__ ((unused))) {
  FENO *pTabFeno = this_field->get_tabfeno(this_field, indexFenoColumn);
  *pixel_list = malloc(50);
  write_spikes(*pixel_list, 50, pTabFeno->omiRejPixelsQF, pTabFeno->NDET);
}

static inline void get_rms(struct output_field *this_field, double *rms, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib __attribute__ ((unused))) {
  *rms = this_field->get_tabfeno(this_field, indexFenoColumn)->RMS;
}

static inline void get_rms_calib(struct output_field *this_field, double *rms, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  *rms = KURUCZ_buffers[indexFenoColumn].KuruczFeno[this_field->index_feno].rms[index_calib];
}

static inline void get_wavelength_calib(struct output_field *this_field, double *wavelength, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  *wavelength = KURUCZ_buffers[indexFenoColumn].KuruczFeno[this_field->index_feno].wve[index_calib];
}

static inline void get_n_iter(struct output_field *this_field, int *n_iter, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib __attribute__ ((unused))) {
  *n_iter = this_field->get_tabfeno(this_field, indexFenoColumn)->nIter; 
}

static inline void get_n_iter_calib(struct output_field *this_field, int *n_iter, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  *n_iter = KURUCZ_buffers[indexFenoColumn].KuruczFeno[this_field->index_feno].nIter[index_calib];
}

static inline void get_chisquare(struct output_field *this_field, double *chisquare, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib __attribute__ ((unused))) {
  *chisquare = this_field->get_tabfeno(this_field, indexFenoColumn)->chiSquare;
}

static inline void get_chisquare_calib(struct output_field *this_field, double *chisquare, const ENGINE_CONTEXT *pEngineContext __attribute__ ((unused)), int indexFenoColumn, int index_calib) {
  *chisquare = KURUCZ_buffers[indexFenoColumn].KuruczFeno[this_field->index_feno].chiSquare[index_calib];
}

static inline void get_flux(struct output_field *this_field, double *flux, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  *flux = output_flux(pEngineContext, OUTPUT_fluxes[this_field->index_flux]);
}

static inline void get_cic(struct output_field *this_field, double *cic, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn __attribute__ ((unused)), int index_calib __attribute__ ((unused))) {
  double flux1 = output_flux(pEngineContext, OUTPUT_cic[this_field->index_cic][1]);
  *cic = (flux1 != (double) 0.)
    ? output_flux(pEngineContext, OUTPUT_cic[this_field->index_cic][0])/flux1
    : DEFAULT_VALUE;
}

// write_spikes:
// concatenate all pixels containing spikes into a single string for output.

RC write_spikes(char *spikestring, unsigned int length, BOOL *spikes,int ndet) {
  strcpy(spikestring,"");
  char num[10];
  RC rc = 0;
  int nspikes = 0;

  int i;

  if (spikes!=NULL)
   for (i=0; i< ndet; i++)
    {
     if (spikes[i])
      {
       (nspikes++ > 0 ) ? sprintf(num,",%d",i): sprintf(num,"%d",i);
       if(strlen(num) + strlen(spikestring) < length) {
        strcat(spikestring,num);
       } else {
        rc = 1;
        break;
       }
      }
    }

  if (!nspikes)
   strcpy(spikestring,"-1");

  return rc;
}
#endif
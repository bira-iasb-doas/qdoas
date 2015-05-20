#include <vector>
#include <string>

#include "apex_read.h"

#include "netcdfwrapper.h"
#include "engine_context.h"
#include "analyse.h"

using std::vector;
using std::string;

static NetCDFFile radiance_file;
static vector<double> wavelengths;

static string reference_filename;
static vector<double> reference_radiance;
static vector<double> reference_wavelengths;
static size_t reference_col_dim;
static size_t reference_spectral_dim;

static size_t spectral_dim; // number of wavelengths
static size_t row_dim; // along track
static size_t col_dim; // cross-track

int apex_set(ENGINE_CONTEXT *pEngineContext) {
  int rc = 0;

  try {
    radiance_file = NetCDFFile(pEngineContext->fileInfo.fileName);

    row_dim = radiance_file.dimLen("row_dim");
    spectral_dim = radiance_file.dimLen("spectral_dim");
    col_dim = radiance_file.dimLen("col_dim");

    if (reference_spectral_dim && reference_spectral_dim != spectral_dim) {
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, "Reference file spectral dimension different from measurement file dimension.");
    }
    if (reference_col_dim && col_dim != col_dim) {
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, "Reference file cross-track dimension different from measurement file dimension.");
    }

    pEngineContext->recordNumber = row_dim * col_dim;
    pEngineContext->recordInfo.n_alongtrack= row_dim;
    pEngineContext->recordInfo.n_crosstrack= col_dim;
    
    NDET = spectral_dim;

    wavelengths.resize(spectral_dim);
    size_t start[] = {0, 0};
    size_t count[] = {spectral_dim, 1};
    radiance_file.getVar("radiance_wavelength", start, count, &wavelengths[0]);

  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return rc;
}

int apex_read(ENGINE_CONTEXT *pEngineContext, int record) {
  int rc = 0;

  int i_alongtrack = (record - 1) / col_dim;
  int i_crosstrack = (record - 1) % col_dim;

  size_t start[] = {i_alongtrack, i_crosstrack, 0};
  size_t count[] = {1, 1, spectral_dim};

  try {
    radiance_file.getVar("radiance", start, count, pEngineContext->buffers.spectrum);
  } catch(std::runtime_error& e) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }

  RECORD_INFO *pRecord = &pEngineContext->recordInfo;
  pRecord->i_alongtrack=i_alongtrack;
  pRecord->i_crosstrack=i_crosstrack;

  return rc;
}

int apex_get_reference(const char *filename, int i_crosstrack, 
                       double *lambda, double *spectrum) {
  int rc = ERROR_ID_NO;

  if (reference_filename != filename) {
    // read all reference spectra in one go and keep them in memory
    try {
      NetCDFFile reference_file(filename);
      reference_col_dim = reference_file.dimLen("col_dim");
      reference_spectral_dim = reference_file.dimLen("spectral_dim");
      NDET = reference_spectral_dim;
      ANALYSE_swathSize = reference_col_dim;

      size_t start[] = {0, 0};
      size_t count[] = {reference_col_dim, reference_spectral_dim};

      vector<double> radiance(reference_col_dim * reference_spectral_dim);
      reference_file.getVar("reference_radiance", start, count, radiance.data());
      vector<double> wavelengths(reference_spectral_dim);
      reference_file.getVar("reference_wavelength", start, &reference_spectral_dim, wavelengths.data());

      reference_radiance = radiance;
      reference_wavelengths = wavelengths;
      reference_filename = filename;
    } catch(std::runtime_error& e) {
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
    }
  }

  memcpy(lambda, reference_wavelengths.data(), reference_spectral_dim*sizeof(lambda[0]));
  memcpy(spectrum, &reference_radiance[i_crosstrack*reference_spectral_dim], reference_spectral_dim*sizeof(spectrum[0]));

  return rc;
}

void apex_clean() {
  radiance_file.close();

  wavelengths.clear();
  reference_wavelengths.clear();
  reference_radiance.clear();
  reference_filename = "";
  reference_col_dim = reference_spectral_dim 
    = spectral_dim = col_dim = row_dim = 0;
}

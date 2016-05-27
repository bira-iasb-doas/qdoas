#include <vector>
#include <string>
#include <map>
#include <cassert>

#include "apex_read.h"

#include "netcdfwrapper.h"
#include "engine_context.h"
#include "analyse.h"

using std::vector;
using std::string;
using std::map;

static NetCDFFile radiance_file;

static map<string,vector<vector<double>>> reference_radiances;
static map<string,vector<double> > reference_wavelengths;

static string init_filename;
static size_t spectral_dim; // number of wavelengths
static size_t row_dim; // along track
static size_t col_dim; // cross-track

static vector<vector<double> > load_reference_radiances(const NetCDFFile& reference_file) {
  vector<vector<double> > radiances(reference_file.dimLen("col_dim"));
  const size_t reference_spectral_dim = reference_file.dimLen("spectral_dim");

  for (size_t i=0; i<radiances.size(); ++i) {
    vector<double>& rad = radiances[i];
    rad.resize(spectral_dim);
    const size_t start[] = {i, 0};
    const size_t count[] = {1, reference_spectral_dim};
    reference_file.getVar("reference_radiance", start, count, rad.data());
  }
  return radiances;
}

int apex_init(const char *reference_filename, ENGINE_CONTEXT *pEngineContext) {
  try {
    NetCDFFile reference_file(reference_filename);
    col_dim = reference_file.dimLen("col_dim");
    spectral_dim = reference_file.dimLen("spectral_dim");
    ANALYSE_swathSize = col_dim;
    for (size_t i=0; i< col_dim; ++i) {
      pEngineContext->project.instrumental.use_row[i] = true;
      NDET[i] = spectral_dim;
    }
    init_filename = reference_filename;
  } catch(std::runtime_error& e) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return ERROR_ID_NO;
}

int apex_set(ENGINE_CONTEXT *pEngineContext) {
  int rc = 0;

  try {
    radiance_file = NetCDFFile(pEngineContext->fileInfo.fileName);

    row_dim = radiance_file.dimLen("row_dim");
    size_t file_spectral_dim = radiance_file.dimLen("spectral_dim");
    size_t file_col_dim = radiance_file.dimLen("col_dim");

    if (file_spectral_dim != spectral_dim) {
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, "Reference file spectral dimension different from measurement file dimension");
    }
    if (file_col_dim != col_dim) {
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, "Reference file cross-track dimension different from measurement file dimension");
    }

    pEngineContext->recordNumber = row_dim * col_dim;
    pEngineContext->recordInfo.n_alongtrack= row_dim;
    pEngineContext->recordInfo.n_crosstrack= col_dim;

    size_t start[] = {0, 0};
    size_t count[] = {spectral_dim, 1};
    radiance_file.getVar("radiance_wavelength", start, count, pEngineContext->buffers.lambda);

  } catch(std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return rc;
}

int apex_read(ENGINE_CONTEXT *pEngineContext, int record) {
  int rc = 0;

  assert(record > 1);
  const size_t i_alongtrack = (record - 1) / col_dim;
  const size_t i_crosstrack = (record - 1) % col_dim;

  const size_t start[] = {i_alongtrack, i_crosstrack, 0};
  const size_t count[] = {1, 1, spectral_dim};

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

int apex_get_reference(const char *filename, int i_crosstrack, double *lambda, double *spectrum, int *n_wavel) {
  auto& radiances = reference_radiances[filename];
  auto& lambda_ref = reference_wavelengths[filename];

  if (!radiances.size() ){
    try {
      NetCDFFile reference_file(filename);
      radiances = load_reference_radiances(reference_file);
      lambda_ref.resize(reference_file.dimLen("spectral_dim"));
      const size_t start[] = {0};
      const size_t count[] = {lambda_ref.size()};
      reference_file.getVar("reference_wavelength", start, count, lambda_ref.data() );
    } catch(std::runtime_error& e) {
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF,
                           (string {"Error reading reference spectra from file '"} + filename + "'").c_str() );
    }
  }

  if (radiances.size() != col_dim || radiances.at(0).size() != spectral_dim) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF,
                         (string { "reference file '" } + filename + "' spectral or column dimension don't match dimensions from '" + init_filename + "'").c_str());
  }
  *n_wavel = spectral_dim;
  for (size_t i=0; i<spectral_dim; ++i) {
    lambda[i] = lambda_ref[i];
    spectrum[i] = radiances.at(i_crosstrack)[i];
  }
  return ERROR_ID_NO;
}

void apex_clean() {
  radiance_file.close();

  init_filename = "";
  reference_wavelengths.clear();
  reference_radiances.clear();
  spectral_dim = col_dim = row_dim = 0;
}

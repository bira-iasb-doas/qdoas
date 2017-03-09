#include <array>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <ctime>

#include "netcdfwrapper.h"
#include "output_netcdf.h"
#include "engine_context.h"
#include "omi_read.h"

using std::string;
using std::vector;
using std::array;

static NetCDFFile output_file;
static NetCDFGroup output_group;

static size_t n_alongtrack, n_crosstrack, n_calib;

static int dim_crosstrack, dim_alongtrack, dim_calib, dim_date, dim_time, dim_datetime, 
  dim_2, dim_3, dim_4, dim_5, dim_6, dim_7, dim_8, dim_9; // dimids

const static string calib_subgroup_name = "Calib";

enum vartype { Analysis, Calibration};

static void create_dimensions(NetCDFGroup &group) {
  struct dim {
    string name;
    size_t size;
    int &id;
  };

  const array<struct dim,14> swathdims  {
    { { "n_crosstrack", n_crosstrack, dim_crosstrack},
        { "n_alongtrack", n_alongtrack, dim_alongtrack },
          { "n_calib", n_calib, dim_calib},
            { "date", 3, dim_date}, // year, month, day
              { "time", 3, dim_time}, // hour, min, secs
                { "datetime", 7, dim_datetime}, // hour, min, secs, milliseconds
        // allow for fields with different number of columns
        // example: fields such as
        // azimuth/longitude/... can contain 3-4-5 values
                  { "2", 2, dim_2}, 
                    { "3", 3, dim_3}, 
                      { "4", 4, dim_4},
                        { "5", 5, dim_5},
                          { "6", 6, dim_6},
                            { "7", 7, dim_7},
                              { "8", 8, dim_8},
                                { "9", 9, dim_9} } };
 
  for (auto& dim : swathdims) {
    dim.id = group.defDim(dim.name, dim.size);
  }
}

static void getDims(const struct output_field& thefield, vector<int>& dimids, vector<size_t>& chunksizes) {
  // for dimensions simply numbered "2, 3, ... 9"
  const array<int, 8> dimnumbers { { dim_2, dim_3, dim_4, dim_5, dim_6, dim_7, dim_8, dim_9 } };

  if (thefield.data_cols > 1) {
    assert(thefield.data_cols < 10);
    dimids.push_back(dimnumbers[thefield.data_cols -2]);
    chunksizes.push_back(thefield.data_cols);
  }
  switch (thefield.memory_type) {
  case OUTPUT_DATE:
    dimids.push_back(dim_date);
    chunksizes.push_back(3);
    break;
  case OUTPUT_TIME:
    dimids.push_back(dim_time);
    chunksizes.push_back(3);
    break;
  case OUTPUT_DATETIME:
    dimids.push_back(dim_datetime);
    chunksizes.push_back(7);
    break;
  default:
    break;
  }
}

static nc_type getNCType(enum output_datatype xtype) {
  switch(xtype) {
  case OUTPUT_STRING:
    return NC_STRING;
    break;
  case OUTPUT_SHORT:
    return NC_SHORT;
    break;
  case OUTPUT_USHORT:
    return NC_USHORT;
    break;
  case OUTPUT_FLOAT:
    return NC_FLOAT;
    break;
  case OUTPUT_INT:
    return NC_INT;
    break;
  case OUTPUT_DOUBLE:
    return NC_DOUBLE;
    break;
  case OUTPUT_TIME:
  case OUTPUT_DATE:
  case OUTPUT_DATETIME:
    return NC_INT; // date/time/datetime are stored as a set of integers in NetCDF
    break;
  default:
  assert(false);
  return 0;
  }
}

static string get_netcdf_varname(const char *varname_in) {
  string result {varname_in};
  std::replace(result.begin(), result.end(), '/', '-');
  return result;
}

static void define_variable(NetCDFGroup &group, const struct output_field& thefield, const string& varname, enum vartype vtype) {

  vector<int> dimids;
  vector<size_t>chunksizes;

  if (vtype == Calibration) {
    if (n_crosstrack > 1) {
      dimids.push_back(dim_crosstrack);
      chunksizes.push_back(std::min<size_t>(100, n_crosstrack));
    }
  
    dimids.push_back(dim_calib);
    chunksizes.push_back(std::min<size_t>(100, n_calib));
  } else {
    dimids.push_back(dim_alongtrack);
    chunksizes.push_back(std::min<size_t>(100, n_alongtrack));

    if (n_crosstrack > 1) {
      dimids.push_back(dim_crosstrack);
      chunksizes.push_back(std::min<size_t>(100, n_crosstrack));
    }
  }
  getDims(thefield, dimids, chunksizes);

  const int varid = group.defVar(varname, dimids, getNCType(thefield.memory_type));
  group.defVarChunking(varid, NC_CHUNKED, chunksizes.data());
  group.defVarDeflate(varid);
  group.defVarFletcher32(varid, NC_FLETCHER32);
  
  switch (thefield.memory_type) {
  case OUTPUT_STRING:
    group.putAttr("_FillValue", QDOAS_FILL_STRING, varid);
    break;
  case OUTPUT_SHORT:
    group.putAttr("_FillValue", QDOAS_FILL_SHORT, varid);
    break;
  case OUTPUT_USHORT:
    group.putAttr("_FillValue", QDOAS_FILL_USHORT, varid);
    break;
  case OUTPUT_FLOAT:
    group.putAttr("_FillValue", QDOAS_FILL_FLOAT, varid);
    break;
  case OUTPUT_DOUBLE:
    group.putAttr("_FillValue", QDOAS_FILL_DOUBLE, varid);
    break;
  case OUTPUT_INT:
  case OUTPUT_TIME:     // date/time/datetime are stored as a set of integers in our NetCDF files
  case OUTPUT_DATE:
  case OUTPUT_DATETIME:
    group.putAttr("_FillValue", QDOAS_FILL_INT, varid);
    break;
  default:
    assert(false);
  }
}

static void write_global_attrs(const ENGINE_CONTEXT*pEngineContext, NetCDFGroup &group) {
  static const string qdoas_attr = string("Results obtained using Qdoas (")
    + cQdoasVersionString + "), \n"
    "Belgian Institute for Space Aeronomy (BIRA-IASB)\n"
    "http://uv-vis.bira.be/software/QDOAS";

  group.putAttr("Qdoas", qdoas_attr);

  time_t curtime = time(NULL);
  group.putAttr("CreationTime",string(ctime(&curtime) ) );

  const char *input_filename = strrchr(pEngineContext->fileInfo.fileName,PATH_SEP);
  if (input_filename) {
    ++input_filename; // if we have found PATH_SEP, file name starts at character behind PATH_SEP 
  } else { // no PATH_SEP found -> just use fileinfo.fileName
    input_filename = pEngineContext->fileInfo.fileName;
  }
  group.putAttr("InputFile", input_filename);
  group.putAttr("QDOASConfig", pEngineContext->project.config_file);
  group.putAttr("QDOASConfigProject", pEngineContext->project.project_name);
}

static void write_calibration_field(const struct output_field& calibfield, NetCDFGroup &group, const string& varname,
                                    const vector<size_t> start, const vector<size_t> count) {
  switch (calibfield.memory_type) {
  case OUTPUT_INT:
    group.putVar(varname, start.data(), count.data(), static_cast<const int *>(calibfield.data));
    break;
  case OUTPUT_SHORT:
    group.putVar(varname, start.data(), count.data(), static_cast<const short *>(calibfield.data));
    break;
  case OUTPUT_USHORT:
    group.putVar(varname, start.data(), count.data(), static_cast<const unsigned short *>(calibfield.data));
    break;
  case OUTPUT_STRING:
    group.putVar(varname, start.data(), count.data(), static_cast<const char **>(calibfield.data));
    break;
  case OUTPUT_FLOAT:
    group.putVar(varname, start.data(), count.data(), static_cast<const float *>(calibfield.data));
    break;
  case OUTPUT_DOUBLE:
    group.putVar(varname, start.data(), count.data(), static_cast<const double *>(calibfield.data));
    break;
  case OUTPUT_DATE:
  case OUTPUT_TIME:
  case OUTPUT_DATETIME:
    assert(false && "date, time or datetime output for calibration not supported");
    break;
  }
}

static void write_calibration_data(NetCDFGroup& group) {

  for (unsigned int i=0; i<calib_num_fields; ++i) {
    const struct output_field& calibfield = output_data_calib[i];

    NetCDFGroup calib_group = group.getGroup(calibfield.windowname).getGroup(calib_subgroup_name);

    string varname = get_netcdf_varname(calibfield.fieldname);

    if ( !calib_group.hasVar(varname)) {
      /* when crosstrack dimension is > 1, output_data_calib contains
       * a copy of each calibration field for each detector row.  We
       * want to define one variable with n_crosstrack columns for
       * each calibration field.  -> check if variable already exists
       * and create it if needed. */
      define_variable(calib_group, calibfield, varname, Calibration);
    }

    vector<int> dimids(calib_group.dimIDs(varname));
    vector<size_t> start(dimids.size());
    start[0] = calibfield.index_row;
    vector<size_t> count;
    for (auto dim : dimids) {
      if (dim == dim_crosstrack) {
        count.push_back(1);
      } else { // we want to write across the full extent of each dimension, except crosstrack
        count.push_back(group.dimLen(dim));
      }
    }
    write_calibration_field(calibfield, calib_group, varname, start, count);
  }
  
}

void write_automatic_reference_info(const ENGINE_CONTEXT *pEngineContext, NetCDFGroup& group) {
  if ( pEngineContext->project.asciiResults.referenceFlag
       && pEngineContext->analysisRef.refAuto ) {
    switch(pEngineContext->project.instrumental.readOutFormat) {
    case PRJCT_INSTR_FORMAT_GDP_ASCII:
    case PRJCT_INSTR_FORMAT_GDP_BIN:
    case PRJCT_INSTR_FORMAT_SCIA_PDS:
    case PRJCT_INSTR_FORMAT_GOME2: {
      const char *reffile = OUTPUT_refFile;
      group.putAttr("Automatic reference file", reffile);
      group.putAttr("Automatic reference: number of records", OUTPUT_nRec);
    }
      break;
    case PRJCT_INSTR_FORMAT_OMI:
      for(int analysiswindow=0; analysiswindow < NFeno; ++analysiswindow) {
        for(int row=0; row< OMI_TOTAL_ROWS; row++ ) {
          const FENO *pTabFeno = &TabFeno[row][analysiswindow];
          if (!pTabFeno->hidden
              && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC
              && pTabFeno->ref_description != NULL) {
            std::stringstream attrname;
            attrname << pTabFeno->windowName << " - row " << row << " automatic reference";
            group.putAttr(attrname.str(), pTabFeno->ref_description);
          }
        }
      }
      break;
    default:
      break;
    }
  }
}

// create a subgroup for each analysis window, and for the calibration
// data belonging to that window
void create_subgroups(NetCDFGroup &group) {

  for (unsigned int i=0; i<calib_num_fields; ++i) {
    if (group.groupID(output_data_calib[i].windowname) < 0) {
      // group not yet created
      auto subgroup = group.defGroup(output_data_calib[i].windowname);
      subgroup.defGroup(calib_subgroup_name);
    }
  }
  for (unsigned int i=0; i<output_num_fields; ++i) {
    if (output_data_analysis[i].windowname &&
        group.groupID(output_data_analysis[i].windowname) < 0) {
      group.defGroup(output_data_analysis[i].windowname);
    }
  }
}

RC netcdf_open(const ENGINE_CONTEXT *pEngineContext, const char *filename) {
  try {
    NetCDFFile output(filename + string(output_file_extensions[NETCDF]), NC_WRITE );
    output_group = output.defGroup(pEngineContext->project.asciiResults.swath_name);

    n_crosstrack = ANALYSE_swathSize;
    n_alongtrack = pEngineContext->recordNumber / n_crosstrack;
    n_calib = 0;
    for (int firstrow = 0; firstrow<ANALYSE_swathSize; firstrow++) {
      if (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI ||
          pEngineContext->project.instrumental.use_row[firstrow] ) {
        n_calib = KURUCZ_buffers[firstrow].Nb_Win;
        break;
      }
    }
    create_dimensions(output_group);
    create_subgroups(output_group);
    write_global_attrs(pEngineContext, output_group);
    write_automatic_reference_info(pEngineContext, output_group);
    write_calibration_data(output_group);

    for (unsigned int i=0; i<output_num_fields; ++i) {
      NetCDFGroup group = output_data_analysis[i].windowname ? 
        output_group.getGroup(output_data_analysis[i].windowname) : output_group;
      define_variable(group, output_data_analysis[i], get_netcdf_varname(output_data_analysis[i].fieldname), Analysis);
    }

    output_file = std::move(output);
  } catch (std::runtime_error& e) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what() );
  }
  return 0;
}

void netcdf_close_file() {
  output_file.close();
}

template<typename T, typename U>
inline static void assign_buffer(T buffer, const U& source) {
  *buffer = source;
}

inline static void assign_buffer(int *buffer, const struct time& t) {
  buffer[0] = t.ti_hour;
  buffer[1] = t.ti_min;
  buffer[2] = t.ti_sec;
}

inline static void assign_buffer(int *buffer, const struct date& d) {
  buffer[0] = d.da_year;
  buffer[1] = d.da_mon;
  buffer[2] = d.da_day;
}

inline static void assign_buffer(int *buffer, const struct datetime& d) {
  buffer[0] = d.thedate.da_year;
  buffer[1] = d.thedate.da_mon;
  buffer[2] = d.thedate.da_day;
  buffer[3] = d.thetime.ti_hour;
  buffer[4] = d.thetime.ti_min;
  buffer[5] = d.thetime.ti_sec;
  buffer[6] = (d.millis != -1)
    ? d.millis * 1000 // SCIA: milliseconds->convert to micro
    : d.microseconds; // GOME-2: microseconds
}

template<typename T>
inline static size_t vardimension() {
  return 1;
}

template<>
size_t vardimension<struct date>() {
  return 3;
}

template<>
size_t vardimension<struct time>() {
  return 3;
}

template<>
size_t vardimension<struct datetime>() {
  return 7;
}

template<typename T, typename U = T>
static void write_buffer(const struct output_field *thefield, const bool selected[], int num_records, const OUTPUT_INFO *recordinfo) {

  size_t ncols = thefield->data_cols;
  size_t dimension = vardimension<U>();

  // variables that depend on the analysis window go the the
  // appropriate subgroup for their analysis window:
  NetCDFGroup group = thefield->windowname ? output_group.getGroup(thefield->windowname) : output_group;
  string varname { get_netcdf_varname(thefield->fieldname) };
  T fill = group.getFillValue<T>(group.varID(varname));
  
  // buffer will hold all output data for this variable
  vector<T> buffer(n_alongtrack * n_crosstrack * ncols * dimension, fill);
  for (int record=0; record < num_records; ++record) {
    if (selected[record]) {

      int i_crosstrack = (recordinfo[record].specno-1) % n_crosstrack; //specno is 1-based
      int i_alongtrack = (recordinfo[record].specno-1) / n_crosstrack;

      for (size_t i=0; i< ncols; ++i) {
        // write into the buffer at the correct index position, using
        // the correct layout for the type of data stored:
        int index = i_alongtrack*n_crosstrack*ncols + i_crosstrack*ncols + i;
        assign_buffer(&buffer[dimension*index], static_cast<U*>(thefield->data)[record*ncols+i]);
      }
    }
  }
  group.putVar(varname, buffer.data() );
}

// specialization to deal with string variable types...
template<>
void write_buffer<const char*>(const struct output_field *thefield, const bool selected[], int num_records, const OUTPUT_INFO *recordinfo) {

  size_t ncols = thefield->data_cols;

  // variables that depend on the analysis window go the the
  // appropriate subgroup for their analysis window:
  NetCDFGroup group = thefield->windowname ? output_group.getGroup(thefield->windowname) : output_group;
  string varname { get_netcdf_varname(thefield->fieldname) };
  string fill = group.getFillValue<string>(group.varID(varname));
  
  // buffer will hold all output data for this variable
  vector<const char*> buffer(n_alongtrack * n_crosstrack * ncols, fill.c_str());
  for (int record=0; record < num_records; ++record) {
    if (selected[record]) {

      int i_crosstrack = (recordinfo[record].specno-1) % n_crosstrack; //specno is 1-based
      int i_alongtrack = (recordinfo[record].specno-1) / n_crosstrack;

      for (size_t i=0; i< ncols; ++i) {
        // write into the buffer at the correct index position, using
        // the correct layout for the type of data stored:
        int index = i_alongtrack*n_crosstrack*ncols + i_crosstrack*ncols + i;
        assign_buffer(&buffer[index], static_cast<const char**>(thefield->data)[record*ncols+i]);
      }
    }
  }
  group.putVar(varname, buffer.data() );
}

RC netcdf_write_analysis_data(const bool selected_records[], int num_records, const OUTPUT_INFO *recordinfo) {
  int rc = ERROR_ID_NO;
  try {
    for (unsigned int i=0; i<output_num_fields; ++i) {
      struct output_field *thefield = &output_data_analysis[i];
      
      switch(thefield->memory_type) {
      case OUTPUT_INT:
        write_buffer<int>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_SHORT:
        write_buffer<short>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_USHORT:
        write_buffer<unsigned short>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_STRING:
        write_buffer<const char*>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_FLOAT:
        write_buffer<float>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_DOUBLE:
        write_buffer<double>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_DATE:
        write_buffer<int, struct date>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_TIME:
        write_buffer<int, struct time>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_DATETIME:
        write_buffer<int, struct datetime>(thefield, selected_records, num_records, recordinfo);
        break;
      }
    }
  } catch (std::runtime_error& e) {
    rc =  ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return rc;
}

RC netcdf_allow_file(const char *filename, const PRJCT_RESULTS *results) {
  int rc = ERROR_ID_NO;
  try {
    NetCDFFile test(filename, NC_WRITE);
    if (test.groupID(results->swath_name) >= 0 ) {
      rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_OUTPUT_NETCDF, filename, results->swath_name);
    }
  } catch (std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return rc;
}

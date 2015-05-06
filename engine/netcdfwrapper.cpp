#include "netcdfwrapper.h"

#include <stdexcept>
#include <cassert>
#include <iostream>

using std::string;
using std::vector;

int NetCDFGroup::groupID(const string& groupName) const {
  int grpid;
  int rc = nc_inq_grp_full_ncid(groupid, groupName.c_str(), &grpid);
  if(rc == NC_NOERR) {
    return grpid;
  } else {
    return 0;
  }
}

bool NetCDFGroup::hasVar(const string& varName) const {
  int id;
  return (nc_inq_varid(groupid, varName.c_str(), &id) == NC_NOERR);
}

int NetCDFGroup::varID(const string& varName) const {
  int id;
  int rc = nc_inq_varid(groupid, varName.c_str(), &id);
  if(rc == NC_NOERR) {
    return id;
  } else {
    throw std::runtime_error("Cannot find NetCDF variable '"+name+"/"+varName+"'");
  }
}

int NetCDFGroup::numDims(int varid) const {
  int ndims;
  if (nc_inq_varndims(groupid, varid, &ndims) == NC_NOERR) {
    return ndims;
  } else {
    throw std::runtime_error("Cannot get number of dimensions for variable '" + varName(varid) + "'");
  }
}

int NetCDFGroup::numDims(const string& varName) const {
  return numDims(varID(varName));
}

vector<int> NetCDFGroup::dimIDs(const std::string &varName) const {
  return dimIDs(varID(varName));
}
vector<int> NetCDFGroup::dimIDs(int varid) const {
  vector<int> dimids(numDims(varid));
  if (nc_inq_vardimid(groupid, varid, dimids.data()) == NC_NOERR) {
    return dimids;
  } else {
    throw std::runtime_error("Cannot dimension ids for variable '" + varName(varid) + "'");
  }
}

string NetCDFGroup::varName(int varid) const {
  char varname[NC_MAX_NAME+1];
  if (nc_inq_varname(groupid, varid, varname) == NC_NOERR) {
    return string(varname);
  } else {
    throw std::runtime_error("Cannot find NetCDF variable with id '"+std::to_string(varid)+"' in group '"+name+"'");
  }
}

int NetCDFGroup::dimID(const string& dimName) const {
  int id;
  int rc = nc_inq_dimid(groupid, dimName.c_str(), &id);
  if(rc == NC_NOERR) {
    return id;
  } else {
    throw std::runtime_error("Cannot find NetCDF dimension '"+dimName+"' in group '"+name+"'");
  }
}


size_t NetCDFGroup::dimLen(const string& dimName) const {
  return dimLen(dimID(dimName));
}

size_t NetCDFGroup::dimLen(int dimid) const {
  size_t len;
  if (nc_inq_dimlen(groupid, dimid, &len) == NC_NOERR) {
    return len;
  } else {
    throw std::runtime_error("Cannot get length of NetCDF dimension '"+dimName(dimid)+"' in group '"+name+"'");
  }
}

string NetCDFGroup::dimName(int dimid) const {
  char dimname[NC_MAX_NAME + 1];
  if (nc_inq_dimname(groupid, dimid, dimname) == NC_NOERR) {
    return(dimname);
  } else {
    throw std::runtime_error("Cannot get name of NetCDF dimension '"+ std::to_string(dimid)+"' in group '"+name+"'");
  }
}

void NetCDFFile::close() {
  if (groupid) {
    int rc = nc_close(groupid);
    if(rc != NC_NOERR) {
      throw std::runtime_error("Error closing NetCDF file '" + filename + "'");
    }
    groupid = 0;
    filename = "";
  }
}

NetCDFFile::NetCDFFile(NetCDFFile &&other) : NetCDFGroup(other), filename(other.filename) {
  other.groupid=0;
  other.filename="";
}

NetCDFFile& NetCDFFile::operator=(NetCDFFile &&other) {
  this->close();
  groupid = other.groupid;
  filename = other.filename;
  other.groupid=0;
  other.filename="";

  return *this;
}

static int openNetCDF(const string &filename, int mode) {
  int groupid;
  int rc = nc_open(filename.c_str(), mode, &groupid);

  if (rc != NC_NOERR && mode != NC_NOWRITE) {
    // file doesn't exist or is not a valid NetCDF file and we are in write mode
    rc = nc_create(filename.c_str(), NC_NETCDF4, &groupid);
  }
  if (rc == NC_NOERR) {
    return groupid;
  } else {
    throw std::runtime_error("Error opening NetCDF file '" + filename + "'");
  }
}

NetCDFFile::NetCDFFile(const string& filename, int mode) : NetCDFGroup(openNetCDF(filename, mode)),
                                                           filename(filename) { }

NetCDFFile::~NetCDFFile() {
  // destructor should not throw exceptions
  try {
    this->close();
  } catch(std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
  }
}

NetCDFGroup NetCDFGroup::getGroup(const string& groupname) const {
  int id = groupID(groupname);

  if(id)
    return NetCDFGroup(groupID(groupname), groupname);
  else
    throw std::runtime_error("Cannot open NetCDF group '" + groupname + "'");
}

NetCDFGroup NetCDFGroup::defGroup(const string& groupname) {
  int newid;
  int rc = nc_def_grp(groupid, groupname.c_str(), &newid);

  if (rc == NC_NOERR) {
    return NetCDFGroup(newid, name + "/" +groupname);
  } else {
    const char *err;
    switch (rc) {
    case NC_ENAMEINUSE:
      err = ", group already exists";
      break;
    case NC_EBADNAME:
      err = ", name contains illegal characters";
      break;
    default:
      err = "";
    }
    throw std::runtime_error("Error creating group '" + groupname + "'" + err);
  }
}

int NetCDFGroup::defDim(const string& dimname, size_t len) {
  int dimid;
  int rc = nc_def_dim(groupid, dimname.c_str(), len, &dimid);

  if (rc == NC_NOERR) {
    return dimid;
  } else {
    throw std::runtime_error("Error creating dimension '" + dimname + "'");
  }    
}

int NetCDFGroup::defVar(const string& varname, const vector<int>& dimids, nc_type xtype) {
  int varid;
  int rc = nc_def_var(groupid, varname.c_str(), xtype, dimids.size(), dimids.data(), &varid);
  if (rc == NC_NOERR) {
    return varid;
  } else {
    throw std::runtime_error("Error creating variable '" + varname + "' in group '" + name + "'");
  }
}

int NetCDFGroup::defVar(const string& name, const vector<string>& dimnames, nc_type xtype) {
  std::vector<int> dimids;
  dimids.reserve(dimnames.size());
  for (auto &name : dimnames) {
    dimids.push_back(dimID(name));
  }
  return defVar(name, dimids, xtype);
}

void NetCDFGroup::defVarChunking(int varid, int storage, size_t *chunksizes) {
  if (nc_def_var_chunking(groupid, varid, storage, chunksizes) != NC_NOERR) {
    throw std::runtime_error("Error setting variable chunking for '" + varName(varid) + "' in group '" + name + "'");
  }
}

void NetCDFGroup::defVarChunking(const string& name, int storage, size_t *chunksizes) {
  defVarChunking(varID(name), storage, chunksizes);
}

void NetCDFGroup::defVarDeflate(int varid, int shuffle, int deflate, int deflate_level) {
  assert(deflate_level && deflate_level <= 9);
  if (nc_def_var_deflate(groupid, varid, shuffle, deflate, deflate_level) != NC_NOERR) {
    throw std::runtime_error("Error setting variable compression for '" + varName(varid) + "' in group '" + name + "'");
  }
}

void NetCDFGroup::defVarDeflate(const string& name, int shuffle, int deflate, int deflate_level) {
  defVarDeflate(varID(name), shuffle, deflate, deflate_level);
}

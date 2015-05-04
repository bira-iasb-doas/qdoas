#ifndef NETCDFWRAPPER_H
#define NETCDFWRAPPER_H

#include <netcdf.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>

// simple wrapper class around NetCDF C API calls.
class NetCDFGroup {
  
public:
  NetCDFGroup(int id=0, const std::string& groupName ="") :  groupid(id), name(groupName) {};

  template<typename T>
  void getFillValue(const std::string& varName, T *fill) const;

  bool hasVar(const std::string& varName) const;
  int varID(const std::string& varName) const;
  int numDims(const std::string& varName) const;
  int numDims(int varid) const;
  std::vector<int> dimIDs(const std::string& varName) const;
  std::vector<int> dimIDs(int varid) const;
  std::string varName(int varid) const;
  int defVar(const std::string& name, const std::vector<int>& dimids, nc_type xtype);
  int defVar(const std::string& name, const std::vector<std::string>& dimnames, nc_type xtype);
  void defVarChunking(int varid, int storage, size_t *chunksizes);
  void defVarChunking(const std::string& name, int storage, size_t *chunksizes);
  void defVarDeflate(int varid, int shuffle=1, int deflate=1, int deflate_level=7);
  void defVarDeflate(const std::string& name, int shuffle=1, int deflate=1, int deflate_level=7);

  template<typename T>
  inline void getVar(int varid, const size_t start[], const size_t count[], T *out) const {
    if (ncGetVar(varid, start, count, out) != NC_NOERR) {
      throw std::runtime_error("Cannot read NetCDF variable '"+name+"/"+varName(varid)+"'");
    } };
  template<typename T>
  inline void getVar(const std::string& name, const size_t start[], const size_t count[], T *out) const {
    getVar(varID(name), start, count, out);  };

  template<typename T>
  inline void putVar(int varid, const size_t start[], const size_t count[], T *in) {
    if (ncPutVar(varid, start, count, in) != NC_NOERR) {
      throw std::runtime_error("Cannot write NetCDF variable '"+name+"/"+varName(varid)+"'");
    } };
  template<typename T>
  inline void putVar(const std::string& name, const size_t start[], const size_t count[], T *in) { 
    putVar(varID(name), start, count, in); };
  template<typename T>
  inline void putVar(int varid, T *in) {
    if (ncPutVar(varid,in) != NC_NOERR) {
      throw std::runtime_error("Cannot write NetCDF variable '"+name+"/"+varName(varid)+"'");
    } };
  template<typename T>
  inline void putVar(const std::string& name, T *in) { 
    putVar(varID(name), in); };

  template<typename T>
  inline void putAttr(const std::string& name, size_t len, T*in, int varid=NC_GLOBAL) {
    if (ncPutAttr(varid, name.c_str(), len, in) != NC_NOERR) {
      throw std::runtime_error("Cannot write NetCDF attribute '" + name + "'" 
                               + (varid == NC_GLOBAL ? "" : " for variable '"+ varName(varid) + "'") );
    }
  }
  template<typename T>
  inline void putAttr(const std::string& name, const std::vector<T>& in, int varid=NC_GLOBAL) {
    putAttr(name, in.size(), in.data(), varid);
  }
  template<typename T>
  inline void putAttr(const std::string& name, T in, int varid=NC_GLOBAL) {
    putAttr(name, 1, &in, varid);
  }
  inline void putAttr(const std::string& name, const std::string& value, int varid=NC_GLOBAL) {
    putAttr(name, value.length(), value.c_str(), varid);
  }
  inline void putAttr(const std::string&name, const std::vector<std::string>& strings, int varid=NC_GLOBAL) {
    std::vector<const char*> charvec;
    for (const auto &s : strings) {
      charvec.push_back(s.c_str() );
    }
    putAttr(name, charvec.size(), charvec.data(), varid);
  }
  inline void putAttr(const std::string&name, char *in, int varid=NC_GLOBAL) {
    ncPutAttr(varid, name.c_str(), strlen(in), in);
  }

  int groupID(const std::string& groupName) const;
  int groupID() const {return groupid;} ;
  NetCDFGroup getGroup(const std::string& groupName) const;
  NetCDFGroup defGroup(const std::string& groupName);

  int defDim(const std::string& dimName, size_t len);
  int dimID(const std::string& dimName) const;
  size_t dimLen(const std::string& dimName) const;
  size_t dimLen(int dimid) const;
  std::string dimName(int dimid) const;
  const std::string& getName() const { return name; };

protected:
  int groupid;

private:
  std::string name;

  inline int ncGetVar(int varid, const size_t start[], const size_t count[], float *out) const {
    return nc_get_vara_float(groupid, varid, start, count, out); };
  inline int ncGetVar(int varid, const size_t start[], const size_t count[], double *out) const {
    return nc_get_vara_double(groupid, varid, start, count, out); };

  inline int ncPutVar(int varid, const size_t start[], const size_t count[], float const *in) {
    return nc_put_vara_float(groupid, varid, start, count, in); };
  inline int ncPutVar(int varid, const size_t start[], const size_t count[], double const *in) {
    return nc_put_vara_double(groupid, varid, start, count, in); };
  inline int ncPutVar(int varid, const size_t start[], const size_t count[], short const *in) {
    return nc_put_vara_short(groupid, varid, start, count, in); };
  inline int ncPutVar(int varid, const size_t start[], const size_t count[], unsigned short const *in) {
    return nc_put_vara_ushort(groupid, varid, start, count, in); }
  inline int ncPutVar(int varid, const size_t start[], const size_t count[], int const *in) {
    return nc_put_vara_int(groupid, varid, start, count, in); };
  inline int ncPutVar(int varid, const size_t start[], const size_t count[], char const **in ) {
    return nc_put_vara_string(groupid, varid, start, count, in); };

  inline int ncPutVar(int varid, float const *in) {
    return nc_put_var_float(groupid, varid, in); };
  inline int ncPutVar(int varid, double const *in) {
    return nc_put_var_double(groupid, varid, in); };
  inline int ncPutVar(int varid, short const *in) {
    return nc_put_var_short(groupid, varid, in); };
  inline int ncPutVar(int varid, unsigned short const *in) {
    return nc_put_var_ushort(groupid, varid, in); }
  inline int ncPutVar(int varid, int const *in) {
    return nc_put_var_int(groupid, varid, in); };
  inline int ncPutVar(int varid, char const **in ) {
    return nc_put_var_string(groupid, varid, in); };

  inline int ncPutAttr(int varid, char const *name, size_t len, const char *text) {
    return nc_put_att_text(groupid, varid, name, len, text);
  }
  inline int ncPutAttr(int varid, char const *name, size_t len, const double *d) {
    return nc_put_att_double(groupid, varid, name, NC_DOUBLE, len, d);
  }
  inline int ncPutAttr(int varid, char const *name, size_t len, const int *i) {
    return nc_put_att_int(groupid, varid, name, NC_DOUBLE, len, i);
  }
  inline int ncPutAttr(int varid, char const *name, size_t len, const char **s) {
    return nc_put_att_string(groupid, varid, name, len, s);
  }
};

// NetCDF file with root group.
class NetCDFFile : public NetCDFGroup {
public:
  NetCDFFile(const std::string& fileName, int mode=NC_NOWRITE);
  NetCDFFile() : filename() {};
  ~NetCDFFile();

  NetCDFFile(NetCDFFile&& other);
  NetCDFFile& operator=(NetCDFFile&& other);

  const std::string& getFile() const { return filename; };

  NetCDFFile(const NetCDFFile& that) = delete; // prevent copying
  NetCDFFile& operator=(NetCDFFile& other) = delete; // prevent assignment

  void close();

private:
  std::string filename;
};

#endif

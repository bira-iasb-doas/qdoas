/*
 * Copyright (C) 2002-2007 Science & Technology BV, The Netherlands.
 *
 * This file is part of BEAT.
 *
 * BEAT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * BEAT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BEAT; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef BEAT_H
#define BEAT_H

/** \file */

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C"
{
#endif
/* *INDENT-ON* */

#include <stdio.h>

#ifdef WIN32
// #include <windows.h>

#ifndef int8_t
#define int8_t  signed char
#endif
#ifndef int16_t
#define int16_t signed short
#endif
#ifndef int32_t
#define int32_t int
#endif
#ifndef int64_t
#define int64_t __int64
#endif

#ifndef uint8_t
#define uint8_t  unsigned char
#endif
#ifndef uint16_t
#define uint16_t unsigned short
#endif
#ifndef uint32_t
#define uint32_t unsigned int
#endif
#ifndef uint64_t
#define uint64_t unsigned __int64
#endif
#endif

#if defined(WIN32) && defined(LIBBEATDLL)
# ifdef LIBBEATDLL_EXPORTS
#  define LIBBEAT_API __declspec(dllexport)
# else
#  define LIBBEAT_API __declspec(dllimport)
# endif
#else
# define LIBBEAT_API
#endif

#ifndef BEAT_CONFIG_H

/* These are the 'configure' defines that we need to export.
 * However, we need to assume that the user of beat.h may also be defining
 * some of these values himself. In order to prevent double definitions each
 * define below is encapsulated by a #ifndef - #endif block.
 */

#ifndef HAVE_INTTYPES_H
#undef HAVE_INTTYPES_H
#endif
#ifndef HAVE_STDINT_H
#undef HAVE_STDINT_H
#endif
#ifndef HAVE_SYS_TYPES_H
#undef HAVE_SYS_TYPES_H
#endif

#ifndef int8_t
#undef int8_t
#endif
#ifndef int16_t
#undef int16_t
#endif
#ifndef int32_t
#undef int32_t
#endif
#ifndef int64_t
#undef int64_t
#endif

#ifndef uint8_t
#undef uint8_t
#endif
#ifndef uint16_t
#undef uint16_t
#endif
#ifndef uint32_t
#undef uint32_t
#endif
#ifndef uint64_t
#undef uint64_t
#endif

#if HAVE_INTTYPES_H
#include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# else
#  if HAVE_SYS_TYPES_H
#   include <sys/types.h>
#  endif
# endif
#endif

#endif /* BEAT_CONFIG_H */

#ifndef PRINTF_ATTR
#undef PRINTF_ATTR
#ifndef PRINTF_ATTR
#define PRINTF_ATTR
#endif
#endif

extern LIBBEAT_API const char *libbeat_version;

/** Maximum number of dimensions of a multidimensional array */
#define BEAT_MAX_NUM_DIMS         8
/** Length of a UTC string (without terminating 0) */
#define BEAT_UTC_STRING_LENGTH   27

extern LIBBEAT_API int beat_errno;
extern LIBBEAT_API FILE *beat_error_log;

#define BEAT_SUCCESS                                          (0)
#define BEAT_ERROR_OUT_OF_MEMORY                             (-1)
#define BEAT_ERROR_HDF4                                     (-10)
#define BEAT_ERROR_NO_HDF4_SUPPORT                          (-11)
#define BEAT_ERROR_HDF5                                     (-12)
#define BEAT_ERROR_NO_HDF5_SUPPORT                          (-13)
#define BEAT_ERROR_XML                                      (-14)

#define BEAT_ERROR_FILE_NOT_FOUND                           (-20)
#define BEAT_ERROR_FILE_OPEN                                (-21)
#define BEAT_ERROR_FILE_READ                                (-22)
#define BEAT_ERROR_FILE_WRITE                               (-23)

#define BEAT_ERROR_INVALID_ARGUMENT                        (-100)
#define BEAT_ERROR_INVALID_INDEX                           (-101)
#define BEAT_ERROR_INVALID_NAME                            (-102)
#define BEAT_ERROR_INVALID_FORMAT                          (-103)
#define BEAT_ERROR_INVALID_DATETIME                        (-104)
#define BEAT_ERROR_INVALID_CURSOR                          (-105)
#define BEAT_ERROR_INVALID_CURSOR_TYPE                     (-106)
#define BEAT_ERROR_INVALID_PRODUCTFILE                     (-107)
#define BEAT_ERROR_INVALID_PRODUCT_CLASS                   (-108)
#define BEAT_ERROR_INVALID_TYPE                            (-109)
#define BEAT_ERROR_ARRAY_NUM_DIMS_MISMATCH                 (-110)
#define BEAT_ERROR_ARRAY_OUT_OF_BOUNDS                     (-111)
#define BEAT_ERROR_NO_PARENT                               (-112)

#define BEAT_ERROR_UNKNOWN_PRODUCT_CLASS                   (-200)
#define BEAT_ERROR_UNSUPPORTED_PRODUCT_TYPE                (-201)
#define BEAT_ERROR_UNSUPPORTED_PRODUCT_VERSION             (-202)

#define BEAT_ERROR_PRODUCT                                 (-300)
#define BEAT_ERROR_OUT_OF_BOUNDS_READ                      (-301)

#define BEAT_ERROR_DATA_DICTIONARY                         (-400)

enum beat_array_ordering
{
    array_ordering_c,                       /**< C array ordering */
    array_ordering_fortran                  /**< Fortran array ordering */
};

enum beat_filefilter_status
{
    beat_ffs_error,                         /**< error: General error (usually file I/O related) */
    beat_ffs_filter_error,                  /**< error: Incorrect filter string */
    beat_ffs_could_not_open_file,           /**< error: Could not open file */
    beat_ffs_could_not_access_directory,    /**< error: Could not recurse into directory */
    beat_ffs_unsupported_file,              /**< File can not be read by BEAT */
    beat_ffs_match,                         /**< File matches filter */
    beat_ffs_no_match,                      /**< File does not match filter */
    beat_ffs_undetermined_match             /**< Could not unambiguously determine whether file matches filter */
};

enum beat_backend
{
    beat_backend_dd,                        /**< Data Dictionary backend (can include XML) */
    beat_backend_xml,                       /**< XML backend (can include Data Dictionary definitions) */
    beat_backend_hdf4,                      /**< HDF4 backend */
    beat_backend_hdf5                       /**< HDF5 backend */
};

enum beat_type_class
{
    beat_record_class,                      /**< Class of all record types */
    beat_array_class,                       /**< Class of all array types */
    beat_integer_class,                     /**< Class of all integer types */
    beat_real_class,                        /**< Class of all real (floating point) types */
    beat_text_class,                        /**< Class of all text types */
    beat_raw_class,                         /**< Class of all unformatted types (data that is not interpreted) */
    beat_special_class                      /**< Class of all special data types (such as time, complex, geolocation) */
};

enum beat_special_type
{
    beat_special_no_data,                   /**< No data (data object is not available) */
    beat_special_vsf_integer,               /**< A compound containing a variable scale factor and an integer.
                                                 The returned double value equals: integer_value x 10^(-scale_factor) */
    beat_special_time,                      /**< Data specifying a date/time value */
    beat_special_complex,                   /**< Data consisting of a real and imaginary value */
    beat_special_geolocation,               /**< Data consisting of a latitude and longitude */
    beat_special_hidden_union               /**< A special union that automatically moves to its only available record
                                                 field if a cursor is moved to it */
};

enum beat_native_type
{
    native_type_not_available = -1,        /**< native type value for compound entities that can not be read directly */
    native_type_int8,                      /**< 8 bit signed integer (#beat_integer_class) */
    native_type_uint8,                     /**< 8 bit unsigned integer (#beat_integer_class) */
    native_type_int16,                     /**< 16 bit signed integer (#beat_integer_class) */
    native_type_uint16,                    /**< 16 bit unsigned integer (#beat_integer_class) */
    native_type_int32,                     /**< 32 bit signed integer (#beat_integer_class) */
    native_type_uint32,                    /**< 32 bit unsigned integer (#beat_integer_class) */
    native_type_int64,                     /**< 64 bit signed integer (#beat_integer_class) */
    native_type_uint64,                    /**< 64 bit unsigned integer (#beat_integer_class) */
    native_type_float,                     /**< 32 bit IEEE floating point type (#beat_real_class) */
    native_type_double,                    /**< 64 bit IEEE floating point type (#beat_real_class) */
    native_type_char,                      /**< single character (#beat_text_class) */
    native_type_string,                    /**< string (#beat_text_class) */
    native_type_bytes                      /**< series of uninterpreted bytes (#beat_raw_class) */
};

typedef enum beat_array_ordering beat_array_ordering;
typedef enum beat_filefilter_status beat_filefilter_status;
typedef enum beat_backend beat_backend;
typedef enum beat_type_class beat_type_class;
typedef enum beat_special_type beat_special_type;
typedef enum beat_native_type beat_native_type;

typedef struct beat_ProductFile beat_ProductFile;
typedef struct beat_Cursor beat_Cursor;
typedef struct beat_Type beat_Type;

/* BEAT General */

LIBBEAT_API int beat_init(void);
LIBBEAT_API void beat_done(void);

LIBBEAT_API int beat_set_option_bypass_special_types(int enable);
LIBBEAT_API int beat_get_option_bypass_special_types(void);
LIBBEAT_API int beat_set_option_perform_boundary_checks(int enable);
LIBBEAT_API int beat_get_option_perform_boundary_checks(void);
LIBBEAT_API int beat_set_option_perform_conversions(int enable);
LIBBEAT_API int beat_get_option_perform_conversions(void);
LIBBEAT_API int beat_set_option_use_fast_size_expressions(int enable);
LIBBEAT_API int beat_get_option_use_fast_size_expressions(void);
LIBBEAT_API int beat_set_option_use_mmap(int enable);
LIBBEAT_API int beat_get_option_use_mmap(void);

LIBBEAT_API double beat_NaN(void);
LIBBEAT_API int beat_isNaN(double x);
LIBBEAT_API double beat_PlusInf(void);
LIBBEAT_API double beat_MinInf(void);
LIBBEAT_API int beat_isInf(double x);
LIBBEAT_API int beat_isPlusInf(double x);
LIBBEAT_API int beat_isMinInf(double x);

LIBBEAT_API int beat_c_index_to_fortran_index(int num_dims, const int dim[], int index);

LIBBEAT_API int beat_datetime_to_double(int YEAR, int MONTH, int DAY, int HOUR, int MINUTE, int SECOND, int MUSEC,
                                        double *datetime);
LIBBEAT_API int beat_double_to_datetime(double datetime, int *YEAR, int *MONTH, int *DAY, int *HOUR, int *MINUTE,
                                        int *SECOND, int *MUSEC);
LIBBEAT_API int beat_month_to_integer(const char month[3]);
LIBBEAT_API int beat_get_utc_string_from_time(double time, char *utc_string);
LIBBEAT_API int beat_get_time_from_string(const char *str, double *time);

LIBBEAT_API int beat_match_filefilter(const char *filefilter, int num_filepaths, const char **filepaths,
                                      int (*callbackfunc) (const char *, beat_filefilter_status, const char *, void *),
                                      void *userdata);

/* BEAT Error */

/* *INDENT-OFF* */
LIBBEAT_API void beat_set_error(int err, const char *message, ...) PRINTF_ATTR;
LIBBEAT_API void beat_set_product_error_information(const char *filename, const char *information, ...) PRINTF_ATTR;
/* *INDENT-ON* */
LIBBEAT_API const char *beat_errno_to_string(int err);

/* BEAT Product */

LIBBEAT_API int beat_recognize_file(const char *filename, const char **product_class, const char **product_type,
                                    int *product_version, int64_t *file_size);

LIBBEAT_API int beat_open(const char *filename, beat_ProductFile **pf);
LIBBEAT_API int beat_close(beat_ProductFile *pf);

LIBBEAT_API int beat_get_product_filename(const beat_ProductFile *pf, const char **filename);
LIBBEAT_API int beat_get_product_file_size(const beat_ProductFile *pf, int64_t *file_size);
LIBBEAT_API int beat_get_product_class(const beat_ProductFile *pf, const char **product_class);
LIBBEAT_API int beat_get_product_type(const beat_ProductFile *pf, const char **product_type);
LIBBEAT_API int beat_get_product_version(const beat_ProductFile *pf, int *version);
LIBBEAT_API int beat_get_product_root_type(const beat_ProductFile *pf, beat_Type **type);
LIBBEAT_API int beat_get_product_backend(const beat_ProductFile *pf, beat_backend *backend);

LIBBEAT_API int beat_get_product_variable_value(beat_ProductFile *pf, const char *variable, int index, int64_t *value);

/* BEAT Types */

LIBBEAT_API const char *beat_type_get_class_name(beat_type_class type_class);
LIBBEAT_API const char *beat_type_get_native_type_name(beat_native_type native_type);
LIBBEAT_API const char *beat_type_get_special_type_name(beat_special_type special_type);

LIBBEAT_API int beat_type_has_ascii_content(const beat_Type *type, int *has_ascii_content);
LIBBEAT_API int beat_type_has_xml_content(const beat_Type *type, int *has_xml_content);

LIBBEAT_API int beat_type_get_class(const beat_Type *type, beat_type_class *type_class);
LIBBEAT_API int beat_type_get_read_type(const beat_Type *type, beat_native_type *read_type);
LIBBEAT_API int beat_type_get_string_length(const beat_Type *type, long *length);
LIBBEAT_API int beat_type_get_bit_size(const beat_Type *type, int64_t *bit_size);
LIBBEAT_API int beat_type_get_name(const beat_Type *type, const char **name);
LIBBEAT_API int beat_type_get_description(const beat_Type *type, const char **description);
LIBBEAT_API int beat_type_get_unit(const beat_Type *type, const char **unit);
LIBBEAT_API int beat_type_get_fixed_value(const beat_Type *type, const char **fixed_value, long *length);

LIBBEAT_API int beat_type_get_num_record_fields(const beat_Type *type, int *num_fields);
LIBBEAT_API int beat_type_get_record_field_index_from_name(const beat_Type *type, const char *name, int *index);
LIBBEAT_API int beat_type_get_record_field_type(const beat_Type *type, int index, beat_Type **field_type);
LIBBEAT_API int beat_type_get_record_field_name(const beat_Type *type, int index, const char **name);
LIBBEAT_API int beat_type_get_record_field_hidden_status(const beat_Type *type, int index, int *hidden);
LIBBEAT_API int beat_type_get_record_field_available_status(const beat_Type *type, int index, int *available);
LIBBEAT_API int beat_type_get_record_union_status(const beat_Type *type, int *is_union);

LIBBEAT_API int beat_type_get_array_num_dims(const beat_Type *type, int *num_dims);
LIBBEAT_API int beat_type_get_array_dim(const beat_Type *type, int *num_dims, int dim[]);
LIBBEAT_API int beat_type_get_array_base_type(const beat_Type *type, beat_Type **base_type);

LIBBEAT_API int beat_type_get_special_type(const beat_Type *type, beat_special_type *special_type);
LIBBEAT_API int beat_type_get_special_base_type(const beat_Type *type, beat_Type **base_type);

/* BEAT Cursor */

LIBBEAT_API int beat_cursor_set_product(beat_Cursor *cursor, beat_ProductFile *pf);

LIBBEAT_API int beat_cursor_goto_first_record_field(beat_Cursor *cursor);
LIBBEAT_API int beat_cursor_goto_next_record_field(beat_Cursor *cursor);
LIBBEAT_API int beat_cursor_goto_record_field_by_index(beat_Cursor *cursor, int index);
LIBBEAT_API int beat_cursor_goto_record_field_by_name(beat_Cursor *cursor, const char *name);
LIBBEAT_API int beat_cursor_goto_available_union_field(beat_Cursor *cursor);

LIBBEAT_API int beat_cursor_goto_first_array_element(beat_Cursor *cursor);
LIBBEAT_API int beat_cursor_goto_next_array_element(beat_Cursor *cursor);
LIBBEAT_API int beat_cursor_goto_array_element(beat_Cursor *cursor, int num_subs, const int subs[]);
LIBBEAT_API int beat_cursor_goto_array_element_by_index(beat_Cursor *cursor, int index);

LIBBEAT_API int beat_cursor_goto_attributes(beat_Cursor *cursor);

LIBBEAT_API int beat_cursor_goto_root(beat_Cursor *cursor);
LIBBEAT_API int beat_cursor_goto_parent(beat_Cursor *cursor);

LIBBEAT_API int beat_cursor_use_base_type_of_special_type(beat_Cursor *cursor);

LIBBEAT_API int beat_cursor_has_ascii_content(const beat_Cursor *cursor, int *has_ascii_content);
LIBBEAT_API int beat_cursor_has_xml_content(const beat_Cursor *cursor, int *has_xml_content);

LIBBEAT_API int beat_cursor_get_string_length(const beat_Cursor *cursor, long *length);
LIBBEAT_API int beat_cursor_get_bit_size(const beat_Cursor *cursor, int64_t *bit_size);
LIBBEAT_API int beat_cursor_get_byte_size(const beat_Cursor *cursor, int64_t *byte_size);
LIBBEAT_API int beat_cursor_get_num_elements(const beat_Cursor *cursor, int *num_elements);

LIBBEAT_API int beat_cursor_get_product_file(const beat_Cursor *cursor, beat_ProductFile **pf);

LIBBEAT_API int beat_cursor_get_depth(const beat_Cursor *cursor, int *depth);
LIBBEAT_API int beat_cursor_get_index(const beat_Cursor *cursor, int *index);

LIBBEAT_API int beat_cursor_get_file_bit_offset(const beat_Cursor *cursor, int64_t *bit_offset);
LIBBEAT_API int beat_cursor_get_file_byte_offset(const beat_Cursor *cursor, int64_t *byte_offset);

LIBBEAT_API int beat_cursor_get_type_class(const beat_Cursor *cursor, beat_type_class *type_class);
LIBBEAT_API int beat_cursor_get_read_type(const beat_Cursor *cursor, beat_native_type *read_type);
LIBBEAT_API int beat_cursor_get_special_type(const beat_Cursor *cursor, beat_special_type *special_type);
LIBBEAT_API int beat_cursor_get_type(const beat_Cursor *cursor, beat_Type **type);

LIBBEAT_API int beat_cursor_get_record_field_index_from_name(const beat_Cursor *cursor, const char *name, int *index);
LIBBEAT_API int beat_cursor_get_record_field_available_status(const beat_Cursor *cursor, int index, int *available);
LIBBEAT_API int beat_cursor_get_available_union_field_index(beat_Cursor *cursor, int *index);

LIBBEAT_API int beat_cursor_get_array_dim(const beat_Cursor *cursor, int *num_dims, int dim[]);

/* read simple-type scalars */

LIBBEAT_API int beat_cursor_read_int8(const beat_Cursor *cursor, int8_t *dst);
LIBBEAT_API int beat_cursor_read_uint8(const beat_Cursor *cursor, uint8_t *dst);
LIBBEAT_API int beat_cursor_read_int16(const beat_Cursor *cursor, int16_t *dst);
LIBBEAT_API int beat_cursor_read_uint16(const beat_Cursor *cursor, uint16_t *dst);
LIBBEAT_API int beat_cursor_read_int32(const beat_Cursor *cursor, int32_t *dst);
LIBBEAT_API int beat_cursor_read_uint32(const beat_Cursor *cursor, uint32_t *dst);
LIBBEAT_API int beat_cursor_read_int64(const beat_Cursor *cursor, int64_t *dst);
LIBBEAT_API int beat_cursor_read_uint64(const beat_Cursor *cursor, uint64_t *dst);

LIBBEAT_API int beat_cursor_read_float(const beat_Cursor *cursor, float *dst);
LIBBEAT_API int beat_cursor_read_double(const beat_Cursor *cursor, double *dst);

LIBBEAT_API int beat_cursor_read_char(const beat_Cursor *cursor, char *dst);
LIBBEAT_API int beat_cursor_read_string(const beat_Cursor *cursor, char *dst, long dst_size);

LIBBEAT_API int beat_cursor_read_bits(const beat_Cursor *cursor, uint8_t *dst, int64_t bit_offset, int64_t bit_length);
LIBBEAT_API int beat_cursor_read_bytes(const beat_Cursor *cursor, uint8_t *dst, int64_t offset, int64_t length);

/* read simple-type arrays */

LIBBEAT_API int beat_cursor_read_int8_array(const beat_Cursor *cursor, int8_t *dst, beat_array_ordering array_ordering);
LIBBEAT_API int beat_cursor_read_uint8_array(const beat_Cursor *cursor, uint8_t *dst,
                                             beat_array_ordering array_ordering);
LIBBEAT_API int beat_cursor_read_int16_array(const beat_Cursor *cursor, int16_t *dst,
                                             beat_array_ordering array_ordering);
LIBBEAT_API int beat_cursor_read_uint16_array(const beat_Cursor *cursor, uint16_t *dst,
                                              beat_array_ordering array_ordering);
LIBBEAT_API int beat_cursor_read_int32_array(const beat_Cursor *cursor, int32_t *dst,
                                             beat_array_ordering array_ordering);
LIBBEAT_API int beat_cursor_read_uint32_array(const beat_Cursor *cursor, uint32_t *dst,
                                              beat_array_ordering array_ordering);
LIBBEAT_API int beat_cursor_read_int64_array(const beat_Cursor *cursor, int64_t *dst,
                                             beat_array_ordering array_ordering);
LIBBEAT_API int beat_cursor_read_uint64_array(const beat_Cursor *cursor, uint64_t *dst,
                                              beat_array_ordering array_ordering);
LIBBEAT_API int beat_cursor_read_float_array(const beat_Cursor *cursor, float *dst, beat_array_ordering array_ordering);
LIBBEAT_API int beat_cursor_read_double_array(const beat_Cursor *cursor, double *dst,
                                              beat_array_ordering array_ordering);
LIBBEAT_API int beat_cursor_read_char_array(const beat_Cursor *cursor, char *dst, beat_array_ordering array_ordering);

/* read two-component-type scalars to tuple */

LIBBEAT_API int beat_cursor_read_complex_double_pair(const beat_Cursor *cursor, double *dst);
LIBBEAT_API int beat_cursor_read_geolocation_double_pair(const beat_Cursor *cursor, double *dst);

/* read two-component-type arrays to tuples */

LIBBEAT_API int beat_cursor_read_complex_double_pairs_array(const beat_Cursor *cursor, double *dst,
                                                            beat_array_ordering array_ordering);
LIBBEAT_API int beat_cursor_read_geolocation_double_pairs_array(const beat_Cursor *cursor, double *dst,
                                                                beat_array_ordering array_ordering);

/* read two-component-type scalars to 'split' representation */

LIBBEAT_API int beat_cursor_read_complex_double_split(const beat_Cursor *cursor, double *dst_re, double *dst_im);
LIBBEAT_API int beat_cursor_read_geolocation_double_split(const beat_Cursor *cursor, double *dst_latitude,
                                                          double *dst_longitude);

/* read two-component-type arrays to 'split' representation */

LIBBEAT_API int beat_cursor_read_complex_double_split_array(const beat_Cursor *cursor, double *dst_re,
                                                            double *dst_im, beat_array_ordering array_ordering);
LIBBEAT_API int beat_cursor_read_geolocation_double_split_array(const beat_Cursor *cursor, double *dst_latitude,
                                                                double *dst_longitude,
                                                                beat_array_ordering array_ordering);

/* DO NOT USE ANY OF THE FIELDS CONTAINED IN THE RECORDS BELOW! */

#ifndef SWIG

#ifdef BEAT_INTERNAL
#define BEAT_PRIVATE_FIELD(name) name
#else
#define BEAT_PRIVATE_FIELD(name) beat_private_field_ ## name
#endif

#define BEAT_CURSOR_MAXDEPTH 16

struct beat_Cursor
{
    struct beat_ProductFile *BEAT_PRIVATE_FIELD(pf);
    int BEAT_PRIVATE_FIELD(n);
    struct
    {
        struct beat_Type *type;
        int index;
        int64_t bit_offset;
    } BEAT_PRIVATE_FIELD(stack)[BEAT_CURSOR_MAXDEPTH];
};

#endif /* SWIG */

/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */

#endif

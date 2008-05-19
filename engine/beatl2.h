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

#ifndef BEATL2_H
#define BEATL2_H

/** \file */

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C"
{
#endif
/* *INDENT-ON* */

#ifdef WIN32
#ifndef int32_t
#define int32_t int
#endif
#endif

#if defined(WIN32) && defined(LIBBEATL2DLL)
# ifdef LIBBEATL2DLL_EXPORTS
#  define LIBBEATL2_API __declspec(dllexport)
# else
#  define LIBBEATL2_API __declspec(dllimport)
# endif
#else
# define LIBBEATL2_API
#endif

#ifndef BEAT_CONFIG_H

/* These are the 'configure' defines that we need to export.   
 * However, we need to assume that the user of beatl2.h may also be defining
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

#ifndef int32_t
#undef int32_t
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

extern LIBBEATL2_API const char *libbeatl2_version;

/** Maximum number of dimensions of a multidimensional array in a BEATL2 Record Type */
#define BEATL2_MAX_NUM_DIMS	8

extern LIBBEATL2_API int beatl2_errno;

#define BEATL2_SUCCESS                                      (0)
#define BEATL2_ERROR_LIBBEAT                            (-2000)
#define BEATL2_ERROR_OUT_OF_MEMORY                      (-2001)
#define BEATL2_ERROR_HDF4                               (-2010)
#define BEATL2_ERROR_NO_HDF4_SUPPORT                    (-2011)
#define BEATL2_ERROR_HDF5                               (-2012)
#define BEATL2_ERROR_NO_HDF5_SUPPORT                    (-2013)

#define BEATL2_ERROR_FILE_OPEN                          (-2100)
#define BEATL2_ERROR_FILE_READ                          (-2101)
#define BEATL2_ERROR_FILE_WRITE                         (-2102)

#define BEATL2_ERROR_PRODUCTFILE_INCONSISTENT           (-2105)

#define BEATL2_ERROR_UNSUPPORTED_PRODUCTFILE            (-2106)
#define BEATL2_ERROR_MULTIPLE_PRODUCTFILE_TYPES         (-2107)
#define BEATL2_ERROR_MULTIPLE_RECORD_TYPES              (-2108)

#define BEATL2_ERROR_INVALID_ARGUMENT                   (-2110)
#define BEATL2_ERROR_INVALID_RECORD_TYPE                (-2111)
#define BEATL2_ERROR_INVALID_FIELDNAME                  (-2112)
#define BEATL2_ERROR_INVALID_FIELD_INDEX                (-2113)
#define BEATL2_ERROR_INVALID_FIELDNAME_FORMAT           (-2114)
#define BEATL2_ERROR_FIELD_ALREADY_EXISTS               (-2115)

#define BEATL2_ERROR_INCOMPATIBLE_RECORDS               (-2120)
#define BEATL2_ERROR_NO_TYPE_FIELD                      (-2121)
#define BEATL2_ERROR_DIMENSION_SIZE_MISMATCH            (-2122)
#define BEATL2_ERROR_INCOMPATIBLE_FIELDS                (-2123)
#define BEATL2_ERROR_CANNOT_SORT_ON_FIELD               (-2124)

#define BEATL2_ERROR_NO_DATA                            (-2130)

#define BEATL2_ERROR_FILTER_OPTIONS_SYNTAX              (-2200)
#define BEATL2_ERROR_INVALID_FILTER_OPTION              (-2201)
#define BEATL2_ERROR_INVALID_FILTER_OPTION_VALUE        (-2202)
#define BEATL2_ERROR_MANDATORY_FILTER_OPTION_MISSING    (-2203)
#define BEATL2_ERROR_PRODUCT_TYPE_FILTER_OPTION_MISSING (-2204)

enum beatl2_BasicType
{
    beatl2_int32,
    beatl2_double,
    beatl2_string_ptr
};

typedef enum beatl2_BasicType beatl2_BasicType;

typedef struct beatl2_Record beatl2_Record;

LIBBEATL2_API int beatl2_init(void);
LIBBEATL2_API void beatl2_done(void);

/* *INDENT-OFF* */ 
LIBBEATL2_API void beatl2_set_error(const int err, const char *message, ...) PRINTF_ATTR;
/* *INDENT-ON* */ 
LIBBEATL2_API const char *beatl2_errno_to_string(const int err);

LIBBEATL2_API int beatl2_ingest(int num_files, const char **filenames, const char *filter, beatl2_Record **record);

LIBBEATL2_API beatl2_Record *beatl2_record_create(void);
LIBBEATL2_API void beatl2_record_remove(beatl2_Record *record);

LIBBEATL2_API int beatl2_record_get_num_fields(const beatl2_Record *record);
LIBBEATL2_API int beatl2_record_get_field_index_from_name(const beatl2_Record *record, const char *field_name);

LIBBEATL2_API int beatl2_record_add_field(beatl2_Record *record, const char *field_name,
                                          beatl2_BasicType type, int num_dims, int dim[]);
LIBBEATL2_API int beatl2_record_add_string_field(beatl2_Record *record, const char *field_name, const char *str);
LIBBEATL2_API int beatl2_record_remove_field(beatl2_Record *record, int index);

LIBBEATL2_API int beatl2_record_attach_field(beatl2_Record *record, const char *field_name,
                                             beatl2_BasicType type, int num_dims, int dim[], void *field_data);
LIBBEATL2_API int beatl2_record_detach_field(beatl2_Record *record, int index, void **field_data);

LIBBEATL2_API int beatl2_record_get_field_name(const beatl2_Record *record, int index, const char **field_name);
LIBBEATL2_API int beatl2_record_get_field_type(const beatl2_Record *record, int index, beatl2_BasicType *type);
LIBBEATL2_API int beatl2_record_get_field_dim(const beatl2_Record *record, int index, int *num_dims, int dim[]);
LIBBEATL2_API int beatl2_record_get_field_num_elements(const beatl2_Record *record, int index, int *num_elements);
LIBBEATL2_API int beatl2_record_get_field_data(const beatl2_Record *record, int index, void **field_data);

LIBBEATL2_API int beatl2_record_set_string_data_element(void *field_data, int array_index, const char *str);

LIBBEATL2_API int beatl2_record_change_field_name(beatl2_Record *record, int index, const char *field_name);

LIBBEATL2_API int beatl2_record_append_data_to_field(beatl2_Record *record, int index, int appendable_dim,
                                                     int num_dims, int dim[], void *data);
LIBBEATL2_API int beatl2_record_append_field_to_field_by_index(int appendable_dim,
                                                               beatl2_Record *record_1, int index_1,
                                                               const beatl2_Record *record_2, int index_2);
LIBBEATL2_API int beatl2_record_append_field_to_field_by_name(int appendable_dim,
                                                              beatl2_Record *record_1, const char *field_name_1,
                                                              const beatl2_Record *record_2, const char *field_name_2);
LIBBEATL2_API int beatl2_record_reshape_field_data(beatl2_Record *record, int index, int dim_id,
                                                   int num_dim_elements, const int *dim_element_id);

LIBBEATL2_API int beatl2_record_get_record_type(const beatl2_Record *record, const char **record_type);

LIBBEATL2_API int beatl2_append(beatl2_Record *record_1, const beatl2_Record *record_2, const char *dimension);
LIBBEATL2_API int beatl2_find_colocated_data(beatl2_Record *record_1, beatl2_Record *record_2,
                                             double time_distance, double radial_distance, double altitude_distance);
LIBBEATL2_API int beatl2_slice(beatl2_Record *record, const char *dimension, int num_ids, const int *id);
LIBBEATL2_API int beatl2_slice_range(beatl2_Record *record, const char *dimension, int from_id, int to_id);
LIBBEATL2_API int beatl2_sort(beatl2_Record *record, const char *dimension, const char *field_name);


LIBBEATL2_API int beatl2_export(const char *format, const char *filename, const beatl2_Record *record);
LIBBEATL2_API int beatl2_import(const char *format, const char *filename, beatl2_Record **record);

LIBBEATL2_API void beatl2_c_array_to_fortran_array_int32(int32_t *c_array, int32_t *fortran_array, int ndims,
                                                         const int dim[]);
LIBBEATL2_API void beatl2_fortran_array_to_c_array_int32(int32_t *fortran_array, int32_t *c_array, int ndims,
                                                         const int dim[]);
LIBBEATL2_API void beatl2_c_array_to_fortran_array_double(double *c_array, double *fortran_array, int ndims,
                                                          const int dim[]);
LIBBEATL2_API void beatl2_fortran_array_to_c_array_double(double *fortran_array, double *c_array, int ndims,
                                                          const int dim[]);

LIBBEATL2_API int beatl2_hitran_code_to_molecule_name(int hitran_code, int html_flag, const char **name);
LIBBEATL2_API int beatl2_molecule_name_to_hitran_code(const char *name, int *hitran_code);


/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */

#endif /* !defined(BEATL2_H) */

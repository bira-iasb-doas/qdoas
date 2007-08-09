#ifndef  __BIN_READ_                          /* Avoid multiple includes */
#define  __BIN_READ_

#include "lv1_struct.h"
#include "read1c_defs.h"

void swap_bytes_float(unsigned char *var);
void swap_bytes_int(unsigned char *var);
void swap_bytes_short(unsigned char *var);

void GeoN_array_getbin (FILE* unit, GeoN *var, int nr);
void GeoL_array_getbin (FILE* unit, GeoL *var, int nr);
void ads_states_getbin (FILE* unit, ADS_STATES *var);
void gads_sun_ref_getbin (FILE* unit, gads_sun_ref *var);
void mds_1c_constant_getbin (FILE* unit, mds_1c_constant *var);
void cal_options_GADS_getbin (FILE* unit, CAL_OPTIONS_GADS *var);

#endif /* __BIN_READ */

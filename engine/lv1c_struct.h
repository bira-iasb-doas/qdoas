/**** level 1c MDS structures ****/
/** requires lv1_struct.h **/

#ifndef  __LV1C_STRUCT                        /* Avoid multiple includes */
#define  __LV1C_STRUCT


#ifndef __LV1_STRUCT
#include "lv1_struct.h"
#endif

#pragma pack(1)

/* constant part of 1c MDS */
typedef struct mds_1c_constant
{
  MJD StartTime;
  unsigned int length;
  char quality;
  float orbit_phase;
  unsigned short category;
  unsigned short state_id;
  unsigned short cluster_id;
  unsigned short nobs;
  unsigned short npixels;
  unsigned char unit_flag;
} mds_1c_constant;

/* constant part of 1c integrated PMD MDS */
typedef struct mds_1c_pmd_constant
{
  MJD StartTime;
  unsigned int length;
  char quality;
  float orbit_phase;
  unsigned short category;
  unsigned short state_id;
  unsigned short duration;
  unsigned short npmd;
  unsigned short ngeo;
}
mds_1c_pmd_constant;

/* constant part of 1c fract. polarisation MDS */
typedef struct mds_1c_pol_constant
{
  MJD StartTime;
  unsigned int length;
  char quality;
  float orbit_phase;
  unsigned short category;
  unsigned short state_id;
  unsigned short duration;
  unsigned short ngeo;
  unsigned short npol;
  unsigned short nint;
  unsigned short int_times[64];
  unsigned short repetition[64];
}
mds_1c_pol_constant;

#endif /* __LV1C_STRUCT */

/* definitions for SCIA Level 1b software */

#ifndef  __LV1_DEFS                        /* Avoid multiple includes */
#define  __LV1_DEFS

#define MAX_CHANNELS		8
#define MAX_CLUSTERS		16
#define MAX_DETECTOR_PIXELS	1024
#define MAX_LV1PATH		200

#define MAX_FILLING 65535.
#define FILL_LIMIT 95. /* in %; higher values will result in a warning message */

/* unit of int. time, etc. (in s) */
#define TIMEUNIT 0.0625

/* MDS types */

typedef enum L1C_MDS_type {
NADIR       ,
LIMB        ,
OCCULTATION ,
MONITORING  ,
MAX_MDS_TYPES
} L1C_MDS_type;


/* Cluster data types */
#define RSIG  1
#define RSIGC 2
#define ESIG  3
#define ESIGC 4

#define PMD_NUMBER 7

#endif /* __LV1_DEFS */

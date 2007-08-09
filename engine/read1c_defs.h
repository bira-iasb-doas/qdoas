/* definitions */

#ifndef  __READ1C_DEFS                        /* Avoid multiple includes */
#define  __READ1C_DEFS

#include<stdlib.h>
#include<stdio.h>
#include "lv1_defs.h"
#include "lv1_struct.h"
#include "lv1c_struct.h"

#define SCAN_LIMB_DURATION 24
#define SCAN_LIMB_STEP_DURATION 3

				/* Error list (to be extended ...) */
typedef enum L1C_error {OK,		/* No error */
		FILE_NOT_FOUND,	/* File cannot be opened */
		FILE_NOT_CLOSED,	/* File nannot be closed
					   (not opened?) */
		END_OF_DATA, /* Behind last data */
		END_OF_NADIR,	/* Behind last nadir data */
		END_OF_LIMB, 	/* Behind last limb data */
		END_OF_OCCULTATION,/* Behind last occultation data */
		CLUSTER_WAVELENGTH_MISMATCH, /* Available cluster not in
					      wavelength window */
		NO_CLUSTER_MATCH_IN_STATE,
		LV1CNO_DATA,
		NO_NADIR,
		NO_LIMB,
		NO_OCC,
		NO_MON,
		NO_SUN_REF,
		NO_NADIR_DATA,	/*  */
		NO_PMD_DATA,	/*  */
		NO_LIMB_DATA,	/*  */
		NO_OCC_DATA,	/*  */
		NO_MON_DATA,	/*  */
		NO_SUN_REF_DATA,	/*  */
			NO_ADS_STATES, /* ADS STATES not found */
			NO_FIXED_WAVELENGTH, /* */
			NO_GADS_CAL_OPTIONS, /* Error in open
						GADS_CAL_OPTIONS */
			END_OF_NADIR_STATES, /* No more Nadir states */
			END_OF_STATES, /* No more Nadir states */
			INCONSISTENT_STATE,  /* Inconsistency in state */
			WL_OUT_OF_RANGE     /* bad user wavelength */
} L1C_error;

/* Reading Type list */
typedef enum L1C_read {
  READ_NADIR=1,
  READ_LIMB,
  READ_OCCULTATION,
  READ_SUN,
  READ_MONITORING
} L1C_read;

/* Solar Measurement types */
/* Fill in all known types - to be extended */
typedef enum solar_type {
  STANDARD=0,
  ESM_DIFF_SUN_REF=0,
  OCC_SUN_REF,
  SUBSOLAR_SUN_REF,
  SUN_D1=0,
  SUN_O =1,
  SUN_S =2,
  SUN_D2=3,
  SUN_D3=4,
  SUN_D4=5,
  SUN_D5=6,
  SUN_D6=7,
  SUN_D7=8,
  SUN_D8=9,
  SUN_D9=10,
} solar_type;

typedef struct user_file_info
{
    /* MPH */
    char *product ; /* Product File name ascii 1 AsciiString (9 bytes) + 1 * 62 bytes + (2 bytes) 2  */
    char *software_ver ; /* Software Version number of processing softwareFormat: Name of processor (up to 10 characters)/ version number (4 characters) -- */
    int abs_orbit; /* Start absolute orbit number               */
    /* SPH */
    char *key_data_version ; /* Key Data version (pattern XX              */
    char *m_factor_version ; /* Key Data version (pattern XX              */
    /* GADS calibration options */
    char *l1b_product_name; 	/* L1b product name */
    char cal_applied[17];	/* Numbers [0-7] of applied calibrations as string */
} USER_FILE_INFO; // was 'user_file_info'


typedef struct user_data_pmd
{
    float pmd[7];
} user_data_pmd;


/* structure for data in user format */

typedef struct user_data
{
    unsigned int n_wl;		/* number of spectral points */
    unsigned short *pixel_nr;	/* pixel numbers */
    unsigned short *pixel_ch;	/* channel of pixels */
    unsigned short *pixel_cls;	/* cluster */
    unsigned short *pixel_coadd; /* co-adding, applied by reading routine */
    float *wl;			/* wavelength */
    float *wl_err;		/* wavelength error */
    float *signal;		/* signal science channel */
    float *signal_err;		/* signal error */
    float int_time;		/* integration time (seconds) */
    MJD mjd;			/* Date (Julian days after 1.1.2000 */
    char date[29];		/* Date string */
    GeoN  geo_nadir;		/* Structure for Geolocation */
    GeoL  geo_limb;
    GeoCal geo_cal;
    unsigned int n_pmd;		/*  */
    user_data_pmd *pmd;		/* integrated PMD readouts (if available)*/
				/* orbit*/
    int n_readout;              /* Number of readout */
    int n_state;                /* Number of state */
/* State information (same for each readout in state) */
    int state_id;
    char state_date[29];
/* information about the product
   (same for each readout)*/
    /* MPH */
    char *product ;             /* Product File name ascii 1 AsciiString (9 bytes) + 1 * 62 bytes + (2 bytes) 2  */
    char *software_ver ; /* Software Version number of processing softwareFormat: Name of processor (up to 10 characters)/ version number (4 characters) -- */
    int abs_orbit; /* Start absolute orbit number               */
    /* SPH */
    char *key_data_version ; /* Key Data version (pattern XX              */
    char *m_factor_version ; /* Key Data version (pattern XX              */
    /* GADS calibration options */
    char *l1b_product_name; 	/* L1b product name */
    char *cal_applied;	/* Numbers [0-7] of applied calibrations as string */
} user_data;

/* structure for solar data in user format */

typedef struct user_data_solar
{
    char sun_spect_id[3];	/* String with id of solar spectrum */
    unsigned int n_wl;		/* number of spectral points */
    unsigned short *pixel_nr;	/* pixel numbers */
    unsigned short *pixel_ch;	/* channel of pixels */
    float *wl;			/* wavelength */
    float *signal;		/* signal science channel */
    float *signal_err;		/* signal error */
    MJD mjd;                    /* Date (Julian days after 1.1.2000 */
    char date[29];		/* Date string */
/* information about the product
   (same for each readout)*/
    /* MPH */
    char *product ;             /* Product File name ascii 1 AsciiString (9 bytes) + 1 * 62 bytes + (2 bytes) 2  */
    char *software_ver ; /* Software Version number of processing softwareFormat: Name of processor (up to 10 characters)/ version number (4 characters) -- */
    int abs_orbit; /* Start absolute orbit number               */
    /* SPH */
    char *key_data_version ; /* Key Data version (pattern XX              */
    char *m_factor_version ; /* Key Data version (pattern XX              */
    /* GADS calibration options */
    char *l1b_product_name; 	/* L1b product name */
    char *cal_applied;	/* Numbers [0-7] of applied calibrations as string */
} user_data_solar;



/* GADS Calibration Options */

/*  DS_NAME : CAL_OPTIONS */
// Changed all 'signed char' into 'char'
typedef struct cal_options_GADS {
    char l1b_product_name[62];	/*ascii  1AsciiString  1 * 62 bytesLevel 1b Product name  */
    char geo_filter_flag ;	/*  flag1BooleanFlag  1 * 1 byte Geolocation Filter Flag (-1 filter applied, 0 not used)*/
    float start_lat ;		/*  (1e-6) degrees  1GeoCoordinate1 * 4 bytes Start Latitude  */
    float start_lon ;		/*  (1e-6) degrees  1GeoCoordinate1 * 4 bytes Start Longitude*/
    float end_lat;			/*  (1e-6) degrees  1GeoCoordinate1 * 4 bytesEnd Latitude*/
    float end_lon;			/*  (1e-6) degrees  1GeoCoordinate1 * 4 bytes End Longitude*/
    char  time_filter_flag ; /* flag1BooleanFlag  1 * 1 byte Time Filter Flag (-1 filter applied, 0 not used)  */
    MJD  start_time;		/*  MJD 1UtcTransport 1 * 12 bytes Filter Start time  */
    MJD  stop_time ;		/*  MJD 1UtcTransport 1 * 12 bytes Filter Stop time*/
    char  category_filter_flag  ; /*  flag1BooleanFlag  1 * 1 byte Measurement Category Filter Flag (-1 filter applied, 0 not used)*/
    unsigned short  category[5]  ;	/*  -5IntegerU2 5 * 2 bytes Selected Measurment Category */
    char nadir_mds_flag  ;	/*  flag1BooleanFlag  1 * 1 byte Process Nadir MDS Flag (-1 MDS created, 0 not written) */
    char limb_mds_flag;	/*  flag1BooleanFlag  1 * 1 byte Process Limb MDS Flag (-1 MDS created, 0 not written)  */
    char occ_mds_flag ;	/*  flag1BooleanFlag  1 * 1 byte Process Occultation MDS Flag (-1 MDS created, 0 not written)  */
    char mon_mds_flag ;	/*  flag1BooleanFlag  1 * 1 byte Process Monitoring MDS Flag (-1 MDS created, 0 not written)*/
    char pmd_mds_flag ;	/*  flag1BooleanFlag  1 * 1 byte PMD MDS Flag (-1 MDS created, 0 not written)  */
    char frac_pol_mds_flag  ; /*  flag1BooleanFlag  1 * 1 byte Fractional Polarisation MDS Flag (-1 MDS created, 0 not written)*/
    char slit_function_gads_flag  ; /*  flag1BooleanFlag  1 * 1 byte Slit Function GADS Flag (-1 GADS copied, 0 not copied)*/
    char sun_mean_ref_gads_flag; /*  flag1BooleanFlag  1 * 1 byte Sun Mean Reference GADS Flag (-1 GADS copied, 0 not copied)  */
    char leakage_current_gads_flag; /*  flag1booleanflag  1 * 1 byte Leakage Current GADS Flag (-1 GADS copied, 0 not copied) */
    char spectral_cal_gads_flag; /*  flag1BooleanFlag  1 * 1 byte Spectral Calibration GADS Flag (-1 GADS copied, 0 not copied)*/
    char pol_sens_gads_flag ; /*  flag1BooleanFlag  1 * 1 byte Polarisation Sensitivity GADS Flag (-1 GADS copied, 0 not copied)  */
    char rad_sens_gads_flag ; /*  flag1BooleanFlag  1 * 1 byte Radiance Sensitivity GADS Flag (-1 GADS copied, 0 not copied)*/
    char ppg_etalon_gads_flag  ; /*  flag1BooleanFlag  1 * 1 byte PPG/Etalon GADS Flag (-1 GADS copied, 0 not copied)*/
    unsigned short  num_nadir_clusters ; /*  -1IntegerU2 1 * 2 bytes Number of Nadir Clusters selected */
    unsigned short  num_limb_clusters  ; /*  -1IntegerU2 1 * 2 bytes Number of Limb Clusters selected  */
    unsigned short  num_occ_clusters; /*  -1IntegerU2 1 * 2 bytes Number of Occultation Clusters selected  */
    unsigned short  num_mon_clusters; /*  -1IntegerU2 1 * 2 bytes Number of Monitoring Clusters selected*/
    char  nadir_cluster_flag  [64]  ; /*  flag  64BooleanFlag  64 * 1 byte Nadir Cluster Flags (-1 used, 0 not used)  */
    char  limb_cluster_flag[64]; /*  flag  64BooleanFlag  64 * 1 byte Limb Cluster Flags (-1 used, 0 not used)*/
    char  occ_cluster_flag [64]; /*  flag  64BooleanFlag  64 * 1 byte Occultation Cluster Flags (-1 used, 0 not used)*/
    char  mon_cluster_flag [64]; /*  flag  64BooleanFlag  64 * 1 byte Monitoring Cluster Flags (-1 used, 0 not used) */
    char  mem_effect_cal_flag; /*  flag1BooleanFlag  1 * 1 byte Memory Effect Correction Flag (-1 applied, 0 not applied) */
    char  leakage_current_cal_flag ; /*  flag1BooleanFlag  1 * 1 byte Leakage Current Calibration Flag (-1 applied, 0 not applied) */
    char  straylight_cal_flag; /*  flag1BooleanFlag  1 * 1 byte Straylight Calibration Flag (-1 applied, 0 not applied) */
    char  ppg_cal_flag ;	/*  flag1BooleanFlag  1 * 1 byte PPG Calibration Flag (-1 applied, 0 not applied)  */
    char  etalon_cal_flag ; /*  flag1BooleanFlag  1 * 1 byte Etalon Calibration Flag (-1 applied, 0 not applied) */
    char  spectral_cal_flag  ; /*  flag1BooleanFlag  1 * 1 byte Spectral Calibration Flag (-1 applied, 0 not applied)  */
    char  polarisation_cal_flag ; /*  flag1BooleanFlag  1 * 1 byte Polarisation Calibration Flag (-1 applied, 0 not applied) */
    char  radiance_cal_flag  ; /*  flag1BooleanFlag  1 * 1 byte Radiance Calibration Flag (-1 applied, 0 not applied)*/
} CAL_OPTIONS_GADS;
// was: cal_options_GADS;		/*Record Length : 400 bytes   */

/* Structure holding data of one state  */

typedef struct state_cluster_data
{
    mds_1c_constant mds_const;
    int type;
    short quality;
    int cluster_id;
    unsigned int n_geo;
    unsigned int n_wl;
    unsigned short *pixel; /* pixel_nr[n_wl] */
    float *wl;     /* wl[n_wl] */
    float *wl_err; /* wl_err[n_wl] */
    float *signal;    /* rad[n_geo*n_wl] */
    float *signal_err; /* rad_err[n_geo*n_wl] */
    GeoN  *geo_nadir; /* geo_nadir[n_geo] */
    GeoL  *geo_limb;
    GeoCal *geo_cal;
} state_cluster_data;


/* ------------------------ integrated PMDs -------------------- */
/* PMD MDS for Nadir measurments PMD MDS for Nadir measurments # Description Units Count Type Size Data Record */

typedef struct nadir_pmd {
  MJD dsr_time ; /* Start time of scan phase MJD 1 UtcTransport 1 * 12 bytes 2      */
  unsigned int dsr_length ; /* Length of this DSR in bytes bytes 1 IntegerU4 1 * 4 bytes 3     */
  unsigned char quality_flag ; /* Quality Indicator (-1 for blank MDSR, 0 otherwise) flag 1 BooleanFlag 1 * 1 byte 4   */
  float orb_phase ; /* Orbit phase after eclipse of the state (range: 0-1) - 1 FloatFl 1 * 4 bytes 5  */
  unsigned short meas_cat ; /* Measurement Category - 1 IntegerU2 1 * 2 bytes 6         */
  unsigned short state_id ; /* State ID - 1 IntegerU2 1 * 2 bytes 7         */
  unsigned short dur_scan_phase ; /* Duration of scan phase of the state 1/16 s 1 IntegerU2 1 * 2 bytes 8   */
  unsigned short num_pmd ; /* Number of Integrated PMD Values - 1 IntegerU2 1 * 2 bytes 9      */
  unsigned short num_geo ; /* Number of Geolocation Values - 1 IntegerU2 1 * 2 bytes 10       */
  float* int_pmd ; /* Integrated PMD values BUs ? FloatFl N * 4 bytes 11        */
  GeoN * geo_nadir ; /* Geolocation - ? - N * 108 bytes a          */
  GeoL * geo_limb ; /* Geolocation - ? - N * 108 bytes a          */
} NADIR_PMD; // was: nadir_pmd;



/* MPH ;  Main Product Header Main Product Header # Description Units Count Type Size Data Record 1    */
typedef struct MPH {
char product[63] ; /* Product File name ascii 1 AsciiString (9 bytes) + 1 * 62 bytes + (2 bytes) 2  */
char proc_stage [ 2]; /* Processing Stage FlagN = Near Real Time, T = test product, V= fully validated (fully consolidated) product, S special product. Letters  between N and V (with the exception of T and S) indicate steps in the consolidation process, with */
char ref_doc [24]; /* Reference Document Describing Product AA-BB-CCC-DD-EEEE_V/I__ */
char spare_1 [41]; /* Spare - 1 SpareField 1 * 40 bytes + (1 byte) 5       */
char acquisition_station [21]; /* Acquisition Station ID (up to 3 codes) If not used, set to ____________________ ascii 1 AsciiString (21 bytes) */
char proc_center [ 7]; /* Processing Center ID which generated current product If not used, set to ______ ascii 1 AsciiString (13 bytes) */
char proc_time [28]; /* UTC Time of Processing (product generation time)UTC Time format          */
char software_ver [15]; /* Software Version number of processing softwareFormat: Name of processor (up to 10 characters)/ version number (4 characters) -- */
char spare_2 [41]; /* Spare - 1 SpareField 1 * 40 bytes + (1 byte) 10       */
char sensing_start [28]; /* UTC start time of data sensing (first measurement in first data record) UTC Time format    */
char sensing_stop [28]; /* UTC stop time of data sensing (last measurements last data record) UTC Time format     */
char spare_3 [41]; /* Spare - 1 SpareField 1 * 40 bytes + (1 byte) 13       */
char phase [ 2]; /* Phasephase letter                 */
char cycle [ 5]; /* CycleCycle number                 */
char rel_orbit [ 7]; /* Start relative orbit number If not used, set to +00000 - 1 AsciiIntegerAs (10 bytes) + 1 * */
char abs_orbit [ 7]; /* Start absolute orbit number               */
char state_vector_time [28]; /* UTC of ENVISAT state vector              */
char delta_ut1 [ 9]; /* DUT1=UT1-UTC                  */
char x_position [13]; /* X Position in Earth-Fixed reference              */
char y_position [13]; /* Y Position in Earth-Fixed reference              */
char z_position [13]; /* Z Position in Earth-Fixed reference              */
char x_velocity [13]; /* X velocity in Earth fixed reference             */
char y_velocity [13]; /* Y velocity in Earth fixed reference             */
char z_velocity [13]; /* Z velocity in Earth fixed reference             */
char vector_source [ 3]; /* Source of Orbit Vectors ascii 1 AsciiString (15 bytes) + 1 * 2 bytes + (2 bytes) 26 */
char spare_4 [41]; /* Spare - 1 SpareField 1 * 40 bytes + (1 byte) 27       */
char utc_sbt_time [28]; /* UTC time corresponding to SBT below(currently defined to be given at the time of the ascending node state */
char sat_binary_time [12]; /* Satellite Binary Time (SBT) 32bit integer time of satellite clock         */
char clock_step [12]; /* Clock Step Sizeclock step in picoseconds             */
char spare_5 [33]; /* Spare - 1 SpareField 1 * 32 bytes + (1 byte) 31       */
char leap_utc [28]; /* UTC time of the occurrence of the Leap SecondSet to ___________________________ if not used     */
char leap_sign [ 5]; /* Leap second sign(+001 if positive Leap Second, -001 if negative)Set to +000 if not used    */
char leap_err [ 2]; /* Leap second errorif leap second occurs within processing segment = 1, otherwise = 0If not used, set to */
char spare_6 [40]; /* Spare - 1 SpareField 1 * 40 bytes + (1 byte) 35       */
char product_err [ 2]; /* 1 or 0                */
char tot_size [22]; /* Total Size Of Product (# bytes DSR + SPH+ MPH) bytes 1 AsciiIntegerAd (9 bytes) + 1 * */
char sph_size [12]; /* Length Of SPH(# bytes in SPH) bytes 1 AsciiIntegerAl (9 bytes) + 1 * 11 bytes + (8 */
char num_dsd [12]; /* Number of DSDs(# DSDs) - 1 AsciiIntegerAl (8 bytes) + 1 * 11 bytes + (1 byte) 39 */
char dsd_size [12]; /* Length of Each DSD(# bytes for each DSD, all DSDs shall have the same length) - 1 AsciiIntegerAl */
char num_data_sets [12]; /* Number of DSs attached(not all DSDs have a DS attached) - 1 AsciiIntegerAl (14 bytes) + 1 * */
char spare_7 [41]; /* Spare - 1 SpareField 1 * 40 bytes + (1 byte) Record Length : 1247 bytes DS_NAME : */
} MPH;



/* Level 1B/1C SPH Level 1B SPH # Description Units Count Type Size Data Record 1    */

typedef struct SPH {
char sph_descriptor [ 29]; /* SPH descriptor ascii 1 AsciiString (16 bytes) + 1 * 28 bytes + (2 bytes) 2   */
char stripline_continuity_indicator [ 5]; /* Value: +000 = No stripline continuity, the product is a complete segment Other: Stripline Counter - 1 AsciiIntegerAc */
char slice_position [ 5]; /* Value: +001 to NUM_SLICES Default value if no stripline continuity = +001 - 1 AsciiIntegerAc (15 bytes) + */
char num_slices [ 5]; /* Number of slices in this stripline Default value if no continuity = +001 - 1 AsciiIntegerAc (11 bytes) */
char start_time [28]; /* Start time of the measurement data in this product          */
char stop_time [28]; /* Time of the end of the measurement data in this product (last MDSR time)     */
char start_lat [12]; /* Latitude of the satellite nadir at the START_TIME           */
char start_long [12]; /* Longitude of the satellite nadir at the START_TIME           */
char stop_lat [12]; /* Latitude of the satellite nadir at the STOP_TIME           */
char stop_long [12]; /* Longitude of the satellite nadir at the STOP_TIME           */
char spare_1 [51]; /* Spare - 1 SpareField 1 * 50 bytes + (1 byte) 12       */
char key_data_version [ 6]; /* Key Data version (pattern XX              */
char m_factor_version [ 6]; /* Key Data version (pattern XX              */
char spectral_cal_check_sum [ 5]; /* Range of spectral calibration error ??(summary): GOOD if ? &lt[  ];= 0        */
char saturated_pixel [ 5]; /* Number of saturated detector pixels n (summary): GOOD if n = 0 FAIR if 0 &lt; n &lt;= */
char dead_pixel [ 5]; /* Number of dead detector pixels n (summary): GOOD if n = 0 FAIR if 0 &lt; n &lt;= */
char dark_check_sum [ 5]; /* Difference between measurement and calibrated dark signal (summary): &quot;GOOD&quot; &quot;FAIR&quot; &quot;BAD0&quot; boundaries still tbd     */
char no_of_nadir_states [ 5]; /* Number of Nadir states where MDSRs have been stored, example: +025 - 1 AsciiIntegerAc (19 bytes) + 1 */
char no_of_limb_states [ 5]; /* Number of Limb states where MDSRs have been stored, example: +025 - 1 AsciiIntegerAc (18 bytes) + 1 */
char no_of_occultation_states [ 5]; /* Number of Occultation states where MDSRs have been stored, example: +025 - 1 AsciiIntegerAc (25 bytes) + 1 */
char no_of_moni_states [ 5]; /* Number of WLS, SLS, and Sun diffuser states where MDSRs have been stored and the corresponding processing has */
char no_of_noproc_states [ 5]; /* Number of states present in the processing time window applied to the level-0 product but not counted in */
char comp_dark_states [ 5]; /* Number of processed complete dark states - 1 AsciiIntegerAc (17 bytes) + 1 * 4 bytes + (1 */
char incomp_dark_states [ 5]; /* Number of incomplete dark states - 1 AsciiIntegerAc (19 bytes) + 1 * 4 bytes + (1 byte) */
char spare_2 [ 5]; /* Spare - 1 SpareField 1 * 4 bytes + (1 byte) Record Length : 697 bytes DS_NAME : */
} SPH;


/* (DSD) info structure */
typedef struct info_l1c
{
    FILE *FILE_l1c;		/* File handler for Lv1C - file */
    MPH mph;
    SPH sph;
    // user_file_info user_file_info; /* Store user important information of product */
    USER_FILE_INFO user_file_info; /* Store user important information of product */
  int num_dsd;		/* No. of DSDs */
  DSD states;		/* state summary DSD */
  DSD nadir;		/* nadir DSD */
  DSD limb;		/* limb DSD */
  DSD occ;	        /* occultation  DSD */
  DSD mon;	        /* monitoring DSD */
  DSD nadir_pmd;	/* nadir PMD DSD */
  DSD limb_pmd;		/* limb PMD DSD */
  DSD occ_pmd;	        /* occultation PMD DSD */
  DSD nadir_pol;	/* nadir fract. polarisaton DSD */
  DSD limb_pol;		/* limb fract. polarisaton DSD */
  DSD occ_pol;	        /* occultation fract. polarisaton DSD */
  DSD sun_ref;	        /* sun reference DSD */
  DSD specbas;	        /* spectral calibration basis DSD */
  DSD specpar;	        /* spectral calibration paramters DSD */
  DSD cal_options;	/* calibration options DSD */
  /* other DSDs may be added */
    int n_mds[MAX_MDS_TYPES]; /* No. of Measurem. data sets
				 (previous 4 numbers); */
    int n_pmd_mds[MAX_MDS_TYPES]; /* No of PMD datasets for each type; */

    // ads_states *ads_states;    /* Complete State info (Clusterdefs!)*/
    ADS_STATES *ads_states;    /* Complete State info (Clusterdefs!)*/

    int *idx_states[MAX_MDS_TYPES];
    int n_states[MAX_MDS_TYPES];

    int mds_offset[MAX_MDS_TYPES];
    int mds_pmd_offset[MAX_MDS_TYPES];

    MJD *mjd_mds;
    int *idx_mds;
    int *meas_cat;



    MJD *mjd_nadir_mds; /* Collect starttimes of MDS nadir */
    int *idx_nadir_mds; /* Index to states of MDS nadir */
    MJD *mjd_limb_mds; /* Collect starttimes of MDS limb */
    int *idx_limb_mds; /* Index to states of MDS limb */

  float wl[NPIXEL];  /* Fixed wawelength grid */
  // cal_options_GADS cal_options_GADS;   /*Calibration Options GADS*/
  CAL_OPTIONS_GADS cal_options_GADS;   /*Calibration Options GADS*/

    int cluster_ids[64];	/* List of clusterids in File */
    int max_cluster_ids;	/* Number of clusterids */

    int cur_cluster_ids[64];	/* List of clusterids in this state */
    int cur_max_cluster_ids;	/* Number of current clusterids */

    int cur_state_nr[MAX_MDS_TYPES];

    int cur_readout_in_state;	/* Actual readout for window in state */
    int cur_max_readout_in_state; /* Max readouts in state */
    int cur_pix_start;		/* Actual pix_window */
    int cur_pix_end;		/* Actual pix_window */
    short cur_used_clusters[64];/* List of really used clusters in current state */
				/* actual cluster data */
				/* coadding factor to reach integration time */
				/* of window */
    int coadd_cur_used_clusters[64];
    int pix_start_cur_used_clusters[64];
    int pix_end_cur_used_clusters[64];
    int sum_pix_cur_used_clusters;
    int geo_cur_used_clusters;	/* geolocation to use for output */
    int max_cur_used_clusters;  /* number of used clusters */
    int cur_it;			/* Integration time currently used */
    int cur_num_pmd;
				/* Data in one state */
    state_cluster_data st_cl_data[64];
    // nadir_pmd nadir_pmd_data;
    NADIR_PMD nadir_pmd_data;
} info_l1c;



#endif /*  __READ1C_DEFS */



















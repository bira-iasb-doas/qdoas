#ifndef  __LV1_STRUCT                        /* Avoid multiple includes */
#define  __LV1_STRUCT

#define __LITTLE_ENDIAN__ 1


/**** some definitions adapted from NADC library ****/

#define MAX_UTC_STRING    ((size_t) 28)

#define PDS_MPH_LENGTH        1247
#define PDS_DSD_LENGTH         280

#define NUM_COORDS              4

#define MAX_CLUSTER            64

#define NPARAM 5
#define NCHANNEL 8
#define NPIXEL 8192
#define NPMD 7
#define MAX_DETECTOR_PIXELS  1024
#define MAX_CHANNELS         8
/*
 * compound data types; adapted from NADC lib
 */

typedef struct MJD
{
  int days;
  unsigned int secnd;
  unsigned int musec;
}
MJD;

typedef struct Coord
{
  int lat;
  int lon;
}
Coord;

typedef struct Clcon
{
  unsigned char id;
  unsigned char channel;
  unsigned short pixel_nr;
  unsigned short length;
  float pet;
  unsigned short int_time;
  unsigned short coadd;
  unsigned short n_read;
  unsigned char type;
}
CLCON; // changed from 'Clcon'

typedef union det_signal
{
  struct signal_breakout
  {
    unsigned int  mem :8;
    unsigned int  det :24;
  } field;
  unsigned int signal;
}
det_signal;

typedef struct Rsig
{
  signed char    mem;
  unsigned short signal;
  unsigned char  stray;
}
Rsig;

typedef struct Rsigc
{
  det_signal    signal;
  unsigned char stray;
}
Rsigc;

typedef struct Esig
{
  signed char    spare;
  unsigned char  stray;
  unsigned short signal;
}
Esig;

typedef struct Esigc
{
  det_signal    signal;
  unsigned char stray;
}
Esigc;

typedef struct PolV
{
  float q[12];
  float q_error[12];
  float u[12];
  float u_error[12];
  float wavelength[13];
  float gdf_par[3];
}
PolV;

typedef struct GeoL
{
  float esm_pos;
  float asm_pos;
  float sza_toa[3];
  float saa_toa[3];
  float los_zen[3];
  float los_azi[3];
  float sat_height;
  float earth_radius;
  Coord sub_sat;
  Coord tangent_ground_point[3];
  float tangent_height[3];
  float doppler_shift;
}
GeoL;

typedef struct GeoN
{
  float esm_pos;
  float sza_toa[3];
  float saa_toa[3];
  float los_zen[3];
  float los_azi[3];
  float sat_height;
  float earth_radius;
  Coord sub_sat;
  Coord corner_coord[4];
  Coord centre_coord;
}
GeoN;

typedef struct GeoCal
{
  float esm_pos;
  float asm_pos;
  float sza;
  Coord sub_sat;
}
GeoCal;

typedef struct L0Hdr
{
  unsigned char packet_header[6];
  unsigned char data_field_header[66];
}
L0Hdr;

/* Note: This is really:

   typedef struct L0Hdr
   {
   PacketHeader_s packet_header;
   DataFieldHeader_s data_field_header;
   DetFieldHeader_s det_field_header;
   }
   L0Hdr;

*/



/*****************************************/

typedef struct DSD
{
  char name[29];
  char type;
  char filename[63];
  unsigned int offset;
  unsigned int size;
  unsigned int num_dsr;
  int dsr_size;
}
DSD;

/*--------------------------------------------------------------------*\
** DSD data structures
\*--------------------------------------------------------------------*/

/* Leakage current (constant part) */
typedef struct gads_leak_c
{
  float fpn[NPIXEL];
  float fpn_err[NPIXEL];
  float lc[NPIXEL];
  float lc_err[NPIXEL];
  float pmd_off_c[2*NPMD];
  float pmd_off_c_err[2*NPMD];
  float noise[NPIXEL];
}
gads_leak_c;

/* Leakage current (variable part) */
typedef struct gads_leak_v
{
  float orbit_phase;
  float temp[10]; /* OBM , 8x Det, PMD */
  float lv[3072]; /* only channels 6-8 */
  float lv_err[3072]; /* only channels 6-8 */
  float strayl[NPIXEL];
  float strayl_err[NPIXEL];
  float strayl_pmd[NPMD];
  float strayl_pmd_err[NPMD];
  float pmd_off_v[2]; /* only PMD 5 and 6 */
  float pmd_off_v_err[2]; /* only PMD 5 and 6 */
}
gads_leak_v;

/* ppg/etalon parameters */
typedef struct gads_ppg {
  float ppg_corr_factor[MAX_CHANNELS][MAX_DETECTOR_PIXELS];
  float etal_corr_factor[MAX_CHANNELS][MAX_DETECTOR_PIXELS];
  float etalon_residual[MAX_CHANNELS][MAX_DETECTOR_PIXELS];
  float wls_degradation[MAX_CHANNELS][MAX_DETECTOR_PIXELS];
  unsigned char q_bad[MAX_CHANNELS][MAX_DETECTOR_PIXELS];
}
gads_ppg;

/* spectral basis */
typedef struct gads_spec_basis
{
  float wavelength[NPIXEL];
}
gads_spec_basis;

/* spectral calibration parameters */
typedef struct gads_spec
{
  float orbit_phase;
  double coeff[NCHANNEL][NPARAM];  /* = [ncoeff][nchannels], sequence tbc */
  unsigned short nr_lines[NCHANNEL];
  float error[NCHANNEL];
}
gads_spec;

/* sun reference */
typedef struct gads_sun_ref
{
  char id[2];
  float wavel[NPIXEL];
  float spectrum[NPIXEL];
  float precision[NPIXEL];
  float accuracy[NPIXEL];
  float etalon[NPIXEL];
  float asm_pos;
  float esm_pos;
  float sun_elev;
  float pmd_mean[NPMD];
  float out_of_band_nd_out[NPMD];
  float out_of_band_nd_in[NPMD];
  float doppler;
}
gads_sun_ref;

/* slit function parameters */
typedef struct gads_slit
{
  unsigned short pixel;
  unsigned char type;
  float fwhm;
  float lorenz;
}
gads_slit;

/* small aperture slit function parameters */
typedef struct gads_slit_small
{
  unsigned short pixel;
  unsigned char type;
  float fwhm;
  float lorenz;
}
gads_slit_small;

/* states */
typedef struct ads_states
{
  MJD StartTime;
  unsigned char  mds_attached;
  unsigned char  flag_reason;
  float orbit_phase;
  unsigned short category;
  unsigned short state_id;
  unsigned short duration;
  unsigned short longest_int_time;
  unsigned short num_clusters;
  CLCON Clcon[MAX_CLUSTER];  // changed from 'Clcon'
  unsigned char  flag_mds;
  unsigned short num_aux;
  unsigned short num_pmd;
  unsigned short num_int;
  unsigned short int_times[MAX_CLUSTER];
  unsigned short num_polar[MAX_CLUSTER];
  unsigned short total_polar;
  unsigned short num_dsr;
  unsigned int  length_dsr;
}
ADS_STATES;  // Changed from 'ads_states'

/* Leakage current (new) */
typedef struct ads_leak_new
{
  MJD StartTimeFirst;
  unsigned char  mds_attached;
  MJD StartTimeLast;
  float orbit_phase;
  float temp[10]; /* OBM , 8x Det, PMD */
  float fpn[NPIXEL];
  float fpn_err[NPIXEL];
  float lc[NPIXEL];
  float lc_err[NPIXEL];
  float noise[NPIXEL];
  float pmd_off[2*NPMD];
  float pmd_off_err[2*NPMD];
}
ads_leak_new;

/* avg. dark measurements */
typedef struct ads_avg_dark
{
  MJD StartTime;
  unsigned char  mds_attached;
  float spectrum[NPIXEL];
  float stddev[NPIXEL];
  float pmd_off[2*NPMD];
  float pmd_off_err[2*NPMD];
  float strayl[NPIXEL];
  float strayl_err[NPIXEL];
  float strayl_pmd[NPMD];
  float strayl_pmd_err[NPMD];
}
ads_avg_dark;


/* new ppg/etalon parameters */
typedef struct ads_ppg_new {
  MJD StartTime;
  unsigned char  mds_attached;
  float ppg_corr_factor[MAX_CHANNELS][MAX_DETECTOR_PIXELS];
  float etal_corr_factor[MAX_CHANNELS][MAX_DETECTOR_PIXELS];
  float etalon_residual[MAX_CHANNELS][MAX_DETECTOR_PIXELS];
  float average_wls[MAX_CHANNELS][MAX_DETECTOR_PIXELS];
  float sigma_average_wls[MAX_CHANNELS][MAX_DETECTOR_PIXELS];
  unsigned char q_bad[MAX_CHANNELS][MAX_DETECTOR_PIXELS];
}
ads_ppg_new;

/* spectral calibration parameters */
typedef struct ads_spec_new
{
  MJD StartTime;
  unsigned char mds_attached;
  float orbit_phase;
  double coeff[NCHANNEL][NPARAM];  /* sequence tbc */
  unsigned char source[NCHANNEL];
  unsigned short nr_lines[NCHANNEL];
  float error[NCHANNEL];
  float spectrum[NPIXEL];
  float line_pos[NCHANNEL][3];  /* sequence tbc */
}
ads_spec_new;

/* new sun reference */
typedef struct ads_sun_ref_new
{
  MJD StartTime;
  unsigned char mds_attached;
  char id[2];
  unsigned char nd_flag;
  float wavel[NPIXEL];
  float spectrum[NPIXEL];
  float precision[NPIXEL];
  float accuracy[NPIXEL];
  float etalon[NPIXEL];
  float asm_pos;
  float esm_pos;
  float sun_elev;
  float pmd_mean[NPMD];
  float out_of_band[NPMD];
  float doppler;
}
ads_sun_ref_new;

#endif /* __LV1_STRUCT */



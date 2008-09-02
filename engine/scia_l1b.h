#ifndef  __SCIA_L1B                       /* Avoid multiple includes */
#define  __SCIA_L1B

#include "scia_common.h"


/* DSD name enum */
    extern char DS_NAME_SCI_NL__1P [40][29];

extern int SIGpccount;
extern int SIGpcount;
extern int SIGgccount;
extern int SIGgcount;

typedef enum DSD_SCI_NL__1P
{
    SUMMARY_QUALITY,
    GEOLOCATION,
    INSTRUMENT_PARAMS,
    LEAKAGE_CONSTANT,
    LEAKAGE_VARIABLE,
    PPG_ETALON,
    SPECTRAL_BASE,
    SPECTRAL_CALIBRATION,
    SUN_REFERENCE,
    POL_SENS_NADIR,
    POL_SENS_LIMB,
    POL_SENS_OCC,
    RAD_SENS_NADIR,
    RAD_SENS_LIMB,
    RAD_SENS_OCC,
    ERRORS_ON_KEY_DATA,
    SLIT_FUNCTION,
    SMALL_AP_SLIT_FUNCTION,
    STATES,
    PMD_PACKETS,
    AUXILIARY_PACKETS,
    NEW_LEAKAGE,
    DARK_AVERAGE,
    NEW_PPG_ETALON,
    NEW_SPECTRAL_CALIBRATION,
    NEW_SUN_REFERENCE,
    NADIR,
    LIMB,
    OCCULTATION,
    MONITORING,
    LEVEL_0_PRODUCT,
    LEAKAGE_FILE,
    PPG_ETALON_FILE,
    SPECTRAL_FILE,
    SUN_REF_FILE,
    KEY_DATA_FILE,
    M_FACTOR_FILE,
    INIT_FILE,
    ORBIT_FILE,
    ATTITUDE_FILE,
    MAX_DS_NAME_SCI_NL__1P
} DSD_SCI_NL__1P ;


/* the numbers are according to the field  'clus_data_type' in 'Clus_Config'
   as specified in the IODD */
typedef int Cluster_Data_Type;
enum Cluster_Data_Type_enum {
    RSIG = 1,
    RSIGC = 2,
    ESIG = 3,
    ESIGC = 4
};

/* MDS type  */

#pragma pack(1)


typedef struct SPH_SCI_NL__1P {
        /*  SPH descriptor */
    char sph_descriptor[29];
        /*  Value: +000 = No stripline continuity, the product is a complete
            segment Other: Stripline Counter */
    char stripline_continuity_indicator[5];
        /*  Value: +001 to NUM_SLICES Default value if no stripline continuity
            = +001 */
    char slice_position[5];
        /*  Number of slices in this stripline Default value if no continuity
            = +001 */
    char num_slices[5];
        /*  Start time of the measurement data in this product. UTC time of
            first MDSR */
    char start_time[28];
        /*  Time of the end of the measurement data in this product (last MDSR
            time). UTC time of last MDSR */
    char stop_time[28];
        /*  Latitude of the satellite nadir at the START_TIME. WGS84 latitude,
            positive = North */
    char start_lat[12];
        /*  Longitude of the satellite nadir at the START_TIME. WGS84 longitude,
            positive = East, 0 = Greenwich */
    char start_long[12];
        /*  Latitude of the satellite nadir at the STOP_TIME. WGS84 latitude,
            positive = North */
    char stop_lat[12];
        /*  Longitude of the satellite nadir at the STOP_TIME. WGS84 longitude,
            positive = East, 0 = Greenwich */
    char stop_long[12];
        /*  Spare */
    char spare_1[50];
        /*  Key Data version (pattern XX.XX, e.g. 01.25) */
    char key_data_version[6];
        /*  Key Data version (pattern XX.XX, e.g. 01.25) */
    char m_factor_version[6];
        /*  Range of spectral calibration error ??(summary): GOOD if ? &lt;=
            0.02FAIR if 0.02 &lt; ? &lt;= 0.05 BAD&Oslash; if ? &gt; 0.05 */
    char spectral_cal_check_sum[5];
        /*  Number of saturated detector pixels n (summary): GOOD if n = 0
            FAIR if 0 &lt; n &lt;= 100 BAD&Oslash; if n &gt; 100 */
    char saturated_pixel[5];
        /*  Number of dead detector pixels n (summary): GOOD if n = 0 FAIR if
            0 &lt; n &lt;= 10 BAD&Oslash; if n &gt; 10 */
    char dead_pixel[5];
        /*  Difference between measurement and calibrated dark signal
            (summary): */
    char dark_check_sum[5];
        /*  Number of Nadir states where MDSRs have been stored, example:
            +025 */
    char no_of_nadir_states[5];
        /*  Number of Limb states where MDSRs have been stored, example:
            +025 */
    char no_of_limb_states[5];
        /*  Number of Occultation states where MDSRs have been stored, example:
            +025 */
    char no_of_occultation_states[5];
        /*  Number of WLS, SLS, and Sun diffuser states where MDSRs have been
            stored and the corresponding processing has been completed
            (i.e. 'newly calculated' ADSRs stored), plus the Number of Monitoring
            states where MDSRs have been stored, example: +025 */
    char no_of_moni_states[5];
        /*  Number of states present in the processing time window applied
            to the level-0 product but not counted in other fields counting
            different types of states. Exemple: +025 */
    char no_of_noproc_states[5];
        /*  Number of processed complete dark states */
    char comp_dark_states[5];
        /*  Number of incomplete dark states */
    char incomp_dark_states[5];
        /*  Spare */
    char spare_2[4];
} SPH_SCI_NL__1P ;

SCIA_err Read_SPH_SCI_NL__1P (FILE* unit, SPH_SCI_NL__1P *sph);
SCIA_err Write_SPH_SCI_NL__1P (FILE* unit, SPH_SCI_NL__1P *sph);


/* Definitions of science channel structs */
/*  Science data contain three parts
       mem : memory effect       (8 bit)
       signal : detector signal  (16 bit for small , 24bit for large(co-added) science data)
       stray : straylight corr   (8 bit)
*/


typedef struct Rsig
{
    struct
    {
	signed char mem;	/*  8 bit*/
	unsigned short signal;	/* 16 bit */
    } det;
    unsigned char  stray;
}
Rsig;

typedef struct Rsigc
{
   struct
    {
	signed int  mem :8;	/* 8 bit */
	unsigned int signal :24; /* 24 bit */
    } det;
   unsigned char stray;
}
Rsigc;


typedef struct Esig
{
    struct
    {
	signed char mem;	/*  8 bit*/
	unsigned short signal;	/* 16 bit */
    } det;
  unsigned char  stray;
}
Esig;

typedef struct Esigc
{
   struct
    {
	signed int  mem :8;	/* 8 bit */
	unsigned int signal :24; /* 24 bit */
    } det;
  unsigned char stray;
}
Esigc;


/* general signal structure for easy internal handling */
typedef struct Signal
{
    unsigned int signal;
    signed char mem;
    unsigned char stray;
    double cal_sig; 		/* for storing "calibrated" signal */
} Signal;

/* -----------------------  */


typedef struct Auxiliary_Packets {
        /*  Start time of the auxiliary data packet */
    MJD dsr_time;
        /*  Attachment flag (Always set to zero for this ADS) */
    char attach_flag;
        /*  Auxiliary data packet of the level 0 data */
    unsigned char dp_level_0[1666];
} Auxiliary_Packets ;


typedef struct Clus_Config {
        /*  Cluster ID (1-64 are valid entries for an existing cluster, the
            first cluster ID being <<0>> notifies the end of the
            cluster ID list) */
    unsigned char cluster_id;
        /*  Channel Number (1-8) */
    unsigned char chan_num;
        /*  Start pixel number (inclusive, 0-1023) */
    unsigned short start_pix;
        /*  Cluster length (1-1024) */
    unsigned short clus_len;
        /*  Pixel Exposure time (PET) */
    float pet;
        /*  Integration time */
    unsigned short intgr_time;
        /*  Co-adding factor */
    unsigned short coadd_factor;
        /*  Number of cluster readouts per DSR */
    unsigned short num_readouts;
        /*  Cluster Data Type: Value defining the cluster data structure: */
    unsigned char clus_data_type;
} Clus_Config ;


typedef struct Dark_Average {
        /*  Start time of the dark measurement state which was used to calculate
            this ADSR */
    MJD dsr_time;
        /*  Attachment Flag */
    char attach_flag;
        /*  Average dark measurement spectrum */
    float avg_dark_meas_spec[8192];
        /*  Standard deviation of the dark measurement spectrum */
    float stan_dev_dark_meas_spec[8192];
        /*  PMD dark offset for all PMDs for the amplifier A and B (1A, 1B, 2A,
            etc.) */
    float pmd_dark_offset[14];
        /*  Error on the PMD offset */
    float error_pmd_off[14];
        /*  Solar straylight scattered from the azimuth mirror */
    float sol_stray_azimuth_mir[8192];
        /*  Error on the solar straylight */
    float error_sol_stray[8192];
        /*  Straylight offset for PMDs */
    float stray_off_pmd[7];
        /*  Error on the PMD straylight offset */
    float error_pmd_stray_off[7];
} Dark_Average ;


typedef struct Errors_On_Key_Data {
        /*  Error on u2 nadir */
    float err_u2_nad[8192];
        /*  Error on u3 nadir */
    float err_u3_nad[8192];
        /*  Error on u2 limb */
    float err_u2_limb[8192];
        /*  Error on u3 limb */
    float err_u3_limb[8192];
        /*  Error on the radiance sensitivity for the optical bench only */
    float err_rad_opt[8192];
        /*  Additional error on radiance sensitivity for elevation mirror
            only (nadir viewing) */
    float err_rad_mirror_nad[8192];
        /*  Additional error on radiance sensitivity for elevation and
            azimuth mirror (limb viewing) */
    float err_rad_mirror_limb[8192];
        /*  Additional error on radiance sensitivity for diffuser and azimuth
            mirror (sun diffuser measurements). */
    float err_rad_diff_mirror[8192];
        /*  Error on BSDF */
    float err_bsdf[8192];
} Errors_On_Key_Data ;


typedef struct Frac_Pol {
        /*  Fractional polarization values Q (6 values derived from PMDs,
            5 values derived from the overlapping regions, and one model
            value below 300 nm) */
    float frac_pol_vals_q[12];
        /*  Errors on Q values */
    float err_q[12];
        /*  Fractional polarization values U (6 values derived from the
            PMDs and one model value below 300 nm) */
    float frac_pol_vals_u[12];
        /*  Errors on the U values */
    float err_u[12];
        /*  Representing wavelength for the fractional polarisation values
            and the 45 degree PMD */
    float rep_wavlen[13];
        /*  GDF parameters */
    float gdf_para[3];
} Frac_Pol ;


typedef struct Geo {
        /*  Position of ESM compared to zero position */
    float pos_esm;
        /*  Solar zenith angles of start, middle, and end of the integration
            time */
    float sol_zen_ang[3];
        /*  Solar azimuth angles of start, middle, and end of the integration
            time */
    float sol_azi_ang[3];
        /*  Line of sight zenith angles of start, middle, and end of the integration
            time */
    float los_zen_ang[3];
        /*  Line of sight azimuth angles of start, middle, and end of the integration
            time */
    float los_azi_ang[3];
        /*  Satellite height at the middle of the integration time */
    float sat_h;
        /*  Earth radius at the middle of the integration time */
    float earth_rad;
        /*  Sub-satellite point at the middle of the integration time */
    Coord sub_sat_point;
        /*  4 corner coordinates of the ground scene which is covered by the
            state (the first co-ordinate is the one which is the first in time
            and flight direction, the second the first in time and last in
            flight direction, the third the last in time and first in flight
            direction and the fourth the last in time and flight direction) */
    Coord corner_coord[4];
        /*  Center co-ordinate of the nadir ground pixel */
    Coord center_coord;
} Geo ;


typedef struct Geolocation {
        /*  Start time of the scan phase of the state */
    MJD dsr_time;
        /*  Attachment Flag (set to 1 if all MDSRs corresponding to this ADSR
            are blank, set to zero otherwise) */
    char attach_flag;
        /*  4 geolocation co-ordinates */
    Coord coord_grd[4];
} Geolocation ;


typedef struct Instrument_Params {
        /*  n_lc_min */
    unsigned char n_lc_min;
        /*  ds_n_phases (~12) */
    unsigned char ds_n_phases;
        /*  ds_phase_boundaries (# = ds_n_phas + 1) */
    float ds_phase_boundaries[13];
        /*  lc_stray_index */
    float lc_stray_index[2];
        /*  lc_harm_order */
    unsigned char lc_harm_order;
        /*  ds_poly_order */
    unsigned char ds_poly_order;
        /*  do_var_lc_cha (3 times 4 characters per EPI-TAXX channel) */
    char do_var_lc_cha[12];
        /*  do_stray_lc_cha (8 times 4 characters per channel) */
    char do_stray_lc_cha[32];
        /*  do_var_lc_pmd (2 times 4 characters per IR PMDs) */
    char do_var_lc_pmd[8];
        /*  do_stray_lc_pmd (7 times 4 characters per PMD) */
    char do_stray_lc_pmd[28];
        /*  electrons_bu (per channel) */
    float electrons_bu[8];
        /*  ppg_error */
    float ppg_error;
        /*  stray_error */
    float stray_error;
        /*  sp_n_phases (~12) */
    unsigned char sp_n_phases;
        /*  sp_phase_boundaries (# = sp_n_phase + 1) */
    float sp_phase_boundaries[13];
        /*  startpix_6+ */
    unsigned short startpix_6_plus;
        /*  startpix_8+ */
    unsigned short startpix_8_plus;
        /*  h_toa */
    float h_toa;
        /*  lambda_end_gdf */
    float lambda_end_gdf;
        /*  do_pol_point (&quot;t&quot; for true and &quot;f&quot; for
            false) */
    char do_pol_point[12];
        /*  sat_level */
    unsigned short sat_level[8];
        /*  pmd_saturation_limit */
    unsigned short pmd_saturation_limit;
        /*  do_use_limb_dark (&quot;t&quot; for true and &quot;f&quot;
            for false) */
    char do_use_limb_dark;
        /*  do_pixelwise (&quot;t&quot; for true and &quot;f&quot; for
            false) */
    char do_pixelwise[8];
        /*  alpha0_asm */
    float alpha0_asm;
        /*  alpha0_esm */
    float alpha0_esm;
        /*  do_fraunhofer (8 times 5 characters per channel) */
    char do_fraunhofer[40];
        /*  do_etalon (8 times 3 characters per channel) */
    char do_etalon[24];
        /*  do_ib_sd_etn (&quot;t&quot; for true and &quot;f&quot; for
            false) */
    char do_ib_sd_etn[7];
        /*  do_ib_sd_etn (&quot;t&quot; for true and &quot;f&quot; for
            false) */
    char do_ib_oc_etn[7];
        /*  Level 2 SMR */
    unsigned char level_2_SMR[8];
} Instrument_Params ;



typedef struct L0_Header {
        /*  Level 0 Product Header (consists of the Packet header (6 bytes)
            and the Data field Header (66 bytes) as defined in A-1. */
    unsigned char header_info[72];
} L0_Header ;


typedef struct Leakage_Constant {
        /*  Constant fraction of the fixed pattern noise (FPN) for each detector
            element of all eight channels (intersect of leakage current
            straight line) */
    float fpn_const[8192];
        /*  Error on constant fraction of FPN */
    float err_fpn_const[8192];
        /*  Constant fraction of the leakage current for each detector element
            of all eight channels (slope of leakage current straight line) */
    float leak_const[8192];
        /*  Error on constant fraction of leakage current */
    float err_leak_const[8192];
        /*  Constant fraction of the PMD dark offset of all 7 PMDs  (1A 1B 2A 2B ...*/
    float pmd_off_const[14];
        /*  Error on constant fraction of PMD offset */
    float err_pmd_off_const[14];
        /*  Mean noise (mean value of standard deviations per detector element)
            per detector array and noise on PMDs) */
    float mean_noise[8192];
} Leakage_Constant ;


typedef struct Leakage_Variable {
        /*  Orbit phase after eclipse (range: 0-1) */
    float orb_phase;
        /*  OBM (near radiator), detector (channels 6-8) and PMD temperatures */
    float obm_det_pmd[10];
        /*  Variable fraction of the leakage current on top of the constant
            fraction for channels 6 to 8 */
    float leak_var[3072];
        /*  Error of variable fraction of LC */
    float err_leak_var[3072];
        /*  Solar straylight scattered from the azimuth mirror */
    float sol_stray[8192];
        /*  Error on the solar straylight */
    float err_sol_stray[8192];
        /*  Straylight offset for PMDs */
    float pmd_stray[7];
        /*  Error on straylight offset for PMDs */
    float err_pmd_stray[7];
        /*  Variable fraction of the PMD dark offset on top of the constant
            fraction for PMD 5 and 6 */
    float pmd_off_var[2];
        /*  Error on the variable fraction of PMD offset */
    float err_pmd_off_var[2];
} Leakage_Variable ;


/* typedef struct Limb { */
/*         /\*  Start time of DSR *\/ */
/*     MJD dsr_time; */
/*         /\*  Length of this DSR in bytes *\/ */
/*     unsigned int dsr_length; */
/*         /\*  Quality Indicator (-1 for blank MDSR, 0 otherwise) *\/ */
/*     char quality_flag; */
/*         /\*  Scale factor for the straylight values in the signal compound */
/*             type per channel *\/ */
/*     unsigned char scale_factor[8]; */
/*         /\*  Saturation Flags *\/ */
/*     char sat_flag[?]; */
/*         /\*  Red Grass flags *\/ */
/*     char red_grass_flag[?]; */
/*         /\*  Sun Glint / Rainbow flags (all flags are set to zero if not used) *\/ */
/*     char sun_glint_flag[?]; */
/*         /\*  Geolocation *\/ */
/*     Geo geo[?]; */
/*         /\*  Level 0 detector module data packet headers *\/ */
/*     L0_Header l0_header[?]; */
/*         /\*  Integrated PMD values *\/ */
/*     float int_pmd[?]; */
/*         /\*  Fractional polarisation values *\/ */
/*     Frac_Pol frac_pol[?]; */
/*         /\*  Cluster data *\/ */
/*     unsigned char clus_dat[?]; */
/* } Limb ; */


/* typedef struct Monitoring { */
/*         /\*  Start time of DSR *\/ */
/*     MJD dsr_time; */
/*         /\*  Length of this DSR in bytes *\/ */
/*     unsigned int dsr_length; */
/*         /\*  Quality Indicator (-1 for blank MDSR, 0 otherwise) *\/ */
/*     char quality_flag; */
/*         /\*  Geolocation *\/ */
/*     Geo geo[?]; */
/*         /\*  Level 0 detector module data packet headers *\/ */
/*     L0_Header l0_header[?]; */
/*         /\*  Cluster data *\/ */
/*     unsigned char clus_dat[?]; */
/* } Monitoring ; */


/* typedef struct Nadir { */
/*         /\*  Start time of DSR *\/ */
/*     MJD dsr_time; */
/*         /\*  Length of this DSR in bytes *\/ */
/*     unsigned int dsr_length; */
/*         /\*  Quality Indicator (-1 for blank MDSR, 0 otherwise) *\/ */
/*     char quality_flag; */
/*         /\*  Scale factor for the straylight values in the signal compound */
/*             type per channel *\/ */
/*     unsigned char scale_factor[8]; */
/*         /\*  Saturation Flags *\/ */
/*     char sat_flag[?]; */
/*         /\*  Red Grass flags *\/ */
/*     char red_grass_flag[?]; */
/*         /\*  Sun Glint / Rainbow flags (all flags are set to zero if not used) *\/ */
/*     char sun_glint_flag[?]; */
/*         /\*  Geolocation *\/ */
/*     Geo geo[?]; */
/*         /\*  Level 0 detector module data packet headers *\/ */
/*     L0_Header l0_header[?]; */
/*         /\*  Integrated PMD values *\/ */
/*     float int_pmd[?]; */
/*         /\*  Fractional polarisation values *\/ */
/*     Frac_Pol frac_pol[?]; */
/*         /\*  Cluster data *\/ */
/*     unsigned char clus_dat[?]; */
/* } Nadir ; */


typedef struct New_Leakage {
        /*  Start time of the first dark measurement state (out of three)
            which was used to calculate this ADSR */
    MJD dsr_time;
        /*  Attachment Flag */
    char attach_flag;
        /*  Start time of the last dark measurement state which was used to
            calculate this ADSR */
    MJD start_time_last;
        /*  Orbit phase after eclipse (range: 0-1) */
    float orb_phase;
        /*  OBM (near radiator), detector (8x) and PMD temperatures */
    float obm_det_pmd[10];
        /*  Fixed pattern noise for channels 1 to 8 */
    float fpn[8192];
        /*  Error on the FPN */
    float err_fpn[8192];
        /*  Leakage current for channels 1 to 8 */
    float leak_cur[8192];
        /*  Error on the leakage current */
    float err_leak_cur[8192];
        /*  Mean noise (mean value of standard deviations per detector element) */
    float mean_noise[8192];
        /*  PMD dark offset for all PMDs */
    float pmd_off[14];
        /*  Error on the PMD offset */
    float err_pmd_off[14];
} New_Leakage ;


typedef struct New_Ppg_Etalon {
        /*  Start time of first WLS measurement */
    MJD dsr_time;
        /*  Attachment Flag */
    char attach_flag;
        /*  Pixel-to-pixel gain factor */
    float pix_gain_fact[8192];
        /*  Etalon Correction Factor */
    float etal_corr_fac[8192];
        /*  Etalon Residual */
    float etal_resid[8192];
        /*  Average WLS spectrum which has been used for the determination
            of PPG and Etalon */
    float avg_wls_spec_ppg_eta[8192];
        /*  Standard deviation of the WLS spectrum */
    float sd_wls_spec[8192];
        /*  Bad pixel mask */
    unsigned char bad_pix_mask[8192];
} New_Ppg_Etalon ;


typedef struct New_Spectral_Calibration {
        /*  Start time of the SLS measurement state which was used to calculate
            this ADSR */
    MJD dsr_time;
        /*  Attachment flag (Always set to zero for this ADS) */
    char attach_flag;
        /*  Orbit phase after eclipse (range: the absolute value is between
            0 and 1) */
    float orb_phase;
        /*  Coefficients in the 4th order polynomial for each detector array
            (channel) */
    double coeff[40];
        /*  Source of spectral calibration parameters (0=SLS, 1=sun diffuser) */
    unsigned char src_spec_cal_param[8];
        /*  Number of used lines per channel */
    unsigned short num_lines[8];
        /*  Wavelength calibration error per channel */
    float cal_err[8];
        /*  Average SLS or solar spectrum which has been used for the determination
            of spectral calibration parameters */
    float avg_sls_sol_spec[8192];
        /*  Selected line positions for 3 lines per channel */
    float sel_line_pos[24];
} New_Spectral_Calibration ;


typedef struct New_Sun_Reference {
        /*  Start time of the first dark measurement state (out of three)
            which was used to calculate this ADSR */
    MJD dsr_time;
        /*  Attachment Flag */
    char attach_flag;
        /*  Sun spectrum identifier (&quot;Dn&quot; fo sun diffuser, &quot;O0&quot;
            for occultation, &quot;S0&quot; for sub-solar) */
    char sun_spect_id[2];
        /*  Neutral density filter flag */
    char neu_den_filt_flag;
        /*  Wavelength of the sun spectrum */
    float wvlen_sun_spec[8192];
        /*  Mean sun reference spectrum */
    float mean_ref_spec[8192];
        /*  Relative radiometric precision of the mean sun reference spectrum */
    float rel_rad_prec[8192];
        /*  Relative radiometric accuracy of the mean sun reference spectrum */
    float rel_rad_acc[8192];
        /*  Diffuser / Small Aperture Etalon */
    float diff_aper_etalon[8192];
        /*  Average azimuth mirror position */
    float ave_azi_pos;
        /*  Average elevation mirror position (diffuser) */
    float avg_ele_pos;
        /*  Average solar elevation angle */
    float avg_solar_ele_ang;
        /*  Mean value of the corresponding PMD measurements */
    float mean_pmd[7];
        /*  PMD out-of-band signal */
    float pmd_out[7];
        /*  Doppler shift at 500 nm */
    float dopp_shift_500nm;
} New_Sun_Reference ;


/* separate between L0 part expanded and L1 only part
   not all software based on this library can handle L0 part */
#ifndef  __LO_PART_EXPANDED__


typedef struct Pmd_Packets {
        /*  Start time of the PMD data packet */
    MJD dsr_time;
        /*  Attachment flag (Always set to zero for this ADS) */
    char attach_flag;
        /*  PMD data packet of the level 0 data */
    unsigned char dp_level_0[6820];
} Pmd_Packets ;


#else
/* START : Pmd_Packets with expanded Level 0 part */
#define NR_OF_PMD_BLOCKS	200
#define NR_OF_PMDS		7
#define PMD_READOUTS		2

typedef struct Pmd_DataBlock_s
{
	unsigned short PMD_Sync;
	unsigned short Data[PMD_READOUTS * NR_OF_PMDS];
	unsigned short BroadCastCounter;
        union
	{
	  struct is_dt_breakout
	  {
	    unsigned int IS:1;
	    unsigned int DeltaTime:15;
	  }field;
	  unsigned short IS_DT_field;
	} IS_DT;
}
Pmd_DataBlock_s;


typedef struct Pmd_Packets {
        /*  Start time of the PMD data packet */
    MJD dsr_time;
        /*  Attachment flag (Always set to zero for this ADS) */
    char attach_flag;
        /*  PMD data packet of the level 0 data */
        /* PacketHeader_s */
    unsigned short PacketId;
    unsigned short SequenceControl;
    unsigned short PacketLength;
	/*DataFieldHeader_s */
    unsigned short Length;
    unsigned short StateCode;
    unsigned short ICU_Time[2];
    unsigned short Redundancy;
    unsigned short PacketID_Overflow;
        /* PMD_DataBlockHeader_s */
    unsigned short PMD_Temperature;
        /* PMD_DataBlock_s */
    Pmd_DataBlock_s pmd_datablock_s[NR_OF_PMD_BLOCKS];
} Pmd_Packets ;

/* END : Pmd_Packets with expanded Level 0 part */
#endif

typedef struct Pol_Sens_Limb {
        /*  Elevation mirror position */
    float elev_mirr_pos;
        /*  Azimuth mirror position */
    float azi_mirr_pos;
        /*  u2 limb for the elevation and azimuth mirror position of field
            1 and 2 */
    float u2_limb[8192];
        /*  u3 limb for the elevation and azimuth mirror position of field
            1 and 2 */
    float u3_limb[8192];
} Pol_Sens_Limb ;

typedef struct Pol_Sens_Occ {
        /*  Elevation mirror position */
    float elev_mirr_pos;
        /*  Azimuth mirror position */
    float azi_mirr_pos;
        /*  u2 limb for the elevation and azimuth mirror position of field
            1 and 2 */
    float u2_limb[8192];
        /*  u3 limb for the elevation and azimuth mirror position of field
            1 and 2 */
    float u3_limb[8192];
} Pol_Sens_Occ ;


typedef struct Pol_Sens_Nadir {
        /*  Elevation mirror position */
    float elev_mirr_pos;
        /*  u2 nadir for the elevation mirror position of field 1 */
    float u2_nad[8192];
        /*  u3 nadir for the elevation mirror position of field 1 */
    float u3_nad[8192];
} Pol_Sens_Nadir ;


typedef struct Ppg_Etalon {
        /*  Pixel-to-pixel gain factor */
    float pix_gain_fac[8192];
        /*  Etalon Correction Factor */
    float etal_corr_fac[8192];
        /*  Etalon Residual */
    float etal_resid[8192];
        /*  WLS degradation factor */
    float wls_deg_fact[8192];
        /*  Bad Pixel Mask */
    unsigned char bad_pix_mask[8192];
} Ppg_Etalon ;


typedef struct Rad_Sens_Limb {
        /*  Elevation mirror position */
    float elev_mirr_pos;
        /*  Azimuth mirror position */
    float azi_mirr_pos;
        /*  Radiance sensitivity for the elevation and azimuth mirror position
            of field 1 and 2 */
    float rad_sen[8192];
} Rad_Sens_Limb ;


typedef struct Rad_Sens_Occ {
        /*  Elevation mirror position */
    float elev_mirr_pos;
        /*  Azimuth mirror position */
    float azi_mirr_pos;
        /*  Radiance sensitivity for the elevation and azimuth mirror position
            of field 1 and 2 */
    float rad_sen[8192];
} Rad_Sens_Occ ;


typedef struct Rad_Sens_Nadir {
        /*  Elevation mirror position */
    float elev_mirr_pos;
        /*  Radiance sensitivity for the elevation and azimuth mirror position
            of field 1 and 2 */
    float rad_sen[8192];
} Rad_Sens_Nadir ;


typedef struct SPH {
        /*  SPH descriptor */
    char sph_descriptor[29];
        /*  Value: +000 = No stripline continuity, the product is a complete
            segment Other: Stripline Counter */
    char stripline_continuity_indicator[5];
        /*  Value: +001 to NUM_SLICES Default value if no stripline continuity
            = +001 */
    char slice_position[5];
        /*  Number of slices in this stripline Default value if no continuity
            = +001 */
    char num_slices[5];
        /*  Start time of the measurement data in this product. UTC time of
            first MDSR */
    char start_time[28];
        /*  Time of the end of the measurement data in this product (last MDSR
            time). UTC time of last MDSR */
    char stop_time[28];
        /*  Latitude of the satellite nadir at the START_TIME. WGS84 latitude,
            positive = North */
    char start_lat[12];
        /*  Longitude of the satellite nadir at the START_TIME. WGS84 longitude,
            positive = East, 0 = Greenwich */
    char start_long[12];
        /*  Latitude of the satellite nadir at the STOP_TIME. WGS84 latitude,
            positive = North */
    char stop_lat[12];
        /*  Longitude of the satellite nadir at the STOP_TIME. WGS84 longitude,
            positive = East, 0 = Greenwich */
    char stop_long[12];
        /*  Spare */
    char spare_1[51];
        /*  Key Data version (pattern XX.XX, e.g. 01.25) */
    char key_data_version[6];
        /*  Key Data version (pattern XX.XX, e.g. 01.25) */
    char m_factor_version[6];
        /*  Range of spectral calibration error ??(summary): GOOD if ? &lt;=
            0.02FAIR if 0.02 &lt; ? &lt;= 0.05 BAD&Oslash; if ? &gt; 0.05 */
    char spectral_cal_check_sum[5];
        /*  Number of saturated detector pixels n (summary): GOOD if n = 0
            FAIR if 0 &lt; n &lt;= 100 BAD&Oslash; if n &gt; 100 */
    char saturated_pixel[5];
        /*  Number of dead detector pixels n (summary): GOOD if n = 0 FAIR if
            0 &lt; n &lt;= 10 BAD&Oslash; if n &gt; 10 */
    char dead_pixel[5];
        /*  Difference between measurement and calibrated dark signal
            (summary): */
    char dark_check_sum[5];
        /*  Number of Nadir states where MDSRs have been stored, example:
            +025 */
    char no_of_nadir_states[5];
        /*  Number of Limb states where MDSRs have been stored, example:
            +025 */
    char no_of_limb_states[5];
        /*  Number of Occultation states where MDSRs have been stored, example:
            +025 */
    char no_of_occultation_states[5];
        /*  Number of WLS, SLS, and Sun diffuser states where MDSRs have been
            stored and the corresponding processing has been completed
            (i.e. 'newly calculated' ADSRs stored), plus the Number of Monitoring
            states where MDSRs have been stored, example: +025 */
    char no_of_moni_states[5];
        /*  Number of states present in the processing time window applied
            to the level-0 product but not counted in other fields counting
            different types of states. Exemple: +025 */
    char no_of_noproc_states[5];
        /*  Number of processed complete dark states */
    char comp_dark_states[5];
        /*  Number of incomplete dark states */
    char incomp_dark_states[5];
        /*  Spare */
    char spare_2[5];
} SPH ;


typedef struct Slit_Function {
        /*  Pixel position for which the slit function is given (0-8191) */
    unsigned short pix_pos_slit_fun;
        /*  Type of slit function (1 = gauss, 2 = single hyperbolic, 3 = voigt) */
    unsigned char type_slit_fun;
        /*  FWHM of slit function [pixel] */
    float fwhm_slit_fun;
        /*  For voigt: FWHM of Lorenzian part [pixel] */
    float f_voi_fwhm_loren;
} Slit_Function ;


typedef struct Small_Ap_Slit_Function {
        /*  Pixel position for which the slit function is given (0-8191) */
    unsigned short pix_pos_slit_fun;
        /*  Type of slit function (1 = gauss, 2 = single hyperbolic, 3 = voigt) */
    unsigned char type_slit_fun;
        /*  FWHM of slit function [pixel], for voigt: Lorenzian part */
    float fwhm_slit_fun;
        /*  For voigt only: FWHM of Gaussian part [pixel] */
    float f_voi_fwhm_gauss;
} Small_Ap_Slit_Function ;


typedef struct Spectral_Base {
        /*  Wavelength of detector pixel */
    float wvlen_det_pix[8192];
} Spectral_Base ;


typedef struct Spectral_Calibration {
        /*  Orbit phase after eclipse (range: the absolute value is between
            0 and 1) */
    float orb_phase;
        /*  Coefficients in the 4th order polynomial for each detector array
            (channel) */
    double coeff[40];
        /*  Number of used lines per channel */
    unsigned short num_lines[8];
        /*  Wavelength calibration error per channel */
    float cal_err[8];
} Spectral_Calibration ;


typedef struct States {
        /*  Start time of the scan phase of the state */
    MJD dsr_time;
        /*  Attachment Flag (set to 1 if all MDSRs corresponding to this ADSR
            are blank, set to zero otherwise) */
    char attach_flag;
        /*  Reason code if the attachment flag is set to '1'0: MDS DSRs are
            not attached, because this type measurement is not intended
            to be in the level 1b product (dark measurements)1: the measurement
            state was corrupted */
    unsigned char reason_code;
        /*  Orbit phase after eclipse of the state (range: 0-1) */
    float orb_phase;
        /*  Measurement Category */
    unsigned short meas_cat;
        /*  State ID */
    unsigned short state_id;
        /*  Duration of scan phase of the state */
    unsigned short dur_scan_phase;
        /*  Longest Integration time */
    unsigned short longest_intg_time;
        /*  Number of clusters */
    unsigned short num_clus;
        /*  Cluster Configuration */
    Clus_Config clus_config[64];
        /*  MDS for this state (1 = nadir, 2 = limb, 3 = occultation, 4 = monitoring) */
    unsigned char mds_type;
        /*  Number of repeated geolocation and Level-0 headers */
    unsigned short num_rep_geo;
        /*  Number of Integrated PMD Values */
    unsigned short num_pmd;
        /*  Number of different integration times */
    unsigned short num_diff_intg_times;
        /*  Integration times: Ordering from longest to shortest integration
            time */
    unsigned short intg_times[64];
        /*  Number of fractional Polarisation Values per Integration time:Ordering
            from longest to shortest integration time */
    unsigned short num_pol_per_intg[64];
        /*  Number of Fractional Polarisation Values */
    unsigned short num_pol;
        /*  Number of DSRs */
    unsigned short num_dsr;
        /*  Length of this DSR in bytes */
    unsigned int len_dsr;
} States ;


typedef struct Summary_Quality {
        /*  Start time of the scan phase of the state */
    MJD dsr_time;
        /*  Attachment Flag (set to 1 if all MDSRs corresponding to this ADSR
            are blank, set to zero otherwise) */
    char attach_flag;
        /*  Mean value of the wavelength differences of Fraunhofer lines
            compared to the wavelength calibration parameters (per channel).
            Set to zero in the case of a corrupted state or an unprocessed state. */
    float mean_wavlen_diff[8];
        /*  Standard deviation of the wavelength differences from field
            3. Set to zero in the case of a corrupted state or an unprocessed
            state. */
    float std_dev_wavlen_diff[8];
        /*  Number of missing readouts in state. Set to zero in the case of
            an unprocessed state. */
    unsigned short num_miss_readouts;
        /*  Mean difference of leakage current or offset per channel and
            PMD (this field is only valid for limb states). The ordering is
            Channels 1 to 8, followed by PMDs 1 to 6, followed by the 45 degree
            PMD. Set to zero in the case of a corrupted state or an unprocessed
            state. */
    float mean_diff_leak[15];
        /*  Sun glint region flag, 0 = no sun glint or corrupted state or unprocessed
            state, 1 = sun glint */
    char sun_glint_flag;
        /*  Rainbow region flag, 0 = no rainbow or corrupted state or unprocessed
            state, 1 = rainbow */
    char rainbow_flag;
        /*  SAA region flag */
    char saa_region_flag;
        /*  Number of hot pixel per channel and PMD (order: 1 to 8 and A to F and
            45 degree) */
    unsigned short num_hotpixels_perchannel[15];
        /*  Spare */
    char spare_1[ 10 ];
} Summary_Quality ;


typedef struct Sun_Reference {
        /*  Sun spectrum identifier (&quot;Dn&quot; for sun difuser, &quot;O0&quot;
            for occultation, &quot;S0&quot; for sub-solar; &quot;n&quot;
            may indicate various diffuser angles) */
    char sun_spect_id[2];
        /*  Wavelength of the sun measurement */
    float wvlen_sun_meas[8192];
        /*  Mean sun reference spectrum */
    float mean_ref_spec[8192];
        /*  Radiometric precision of the mean sun reference spectrum */
    float rad_pre_mean_sun_ref_spec[8192];
        /*  Radiometric accuracy of the mean sun reference spectrum */
    float rad_acc_mean_sun_ref_spec[8192];
        /*  Diffuser / Small Aperture Etalon */
    float diff_aper_etalon[8192];
        /*  Average azimuth mirror position */
    float avg_azi_pos;
        /*  Average elevation mirror position (diffuser) */
    float avg_ele_pos;
        /*  Average solar elevation angle */
    float avg_solar_ele_ang;
        /*  Mean value of the corresponding PMD measurements */
    float mean_pmd[7];
        /*  PMD out-of-band signal with ND out */
    float pmd_out_nd_out[7];
        /*  PMD out-of-band signal with ND in */
    float pmd_out_nd_in[7];
        /*  Doppler shift at 500 nm */
    float dopp_shift_500nm;
} Sun_Reference ;


/* Reading routines */
void Auxiliary_Packets_getbin (FILE* unit, Auxiliary_Packets *var);
void Clus_Config_getbin (FILE* unit, Clus_Config *var);
void Clus_Config_array_getbin (FILE* unit, Clus_Config *var, int nr);
void Dark_Average_getbin (FILE* unit, Dark_Average *var);
void Errors_On_Key_Data_getbin (FILE* unit, Errors_On_Key_Data *var);
void Frac_Pol_getbin (FILE* unit, Frac_Pol *var);
void Frac_Pol_array_getbin (FILE* unit, Frac_Pol *var, int nr);
void Geo_getbin (FILE* unit, Geo *var);
void Geolocation_getbin (FILE* unit, Geolocation *var);
void Instrument_Params_getbin (FILE* unit, Instrument_Params *var);
void L0_Header_getbin (FILE* unit, L0_Header *var);
void L0_Header_array_getbin (FILE* unit, L0_Header *var, int nr);
void Leakage_Constant_getbin (FILE* unit, Leakage_Constant *var);
void Leakage_Variable_getbin (FILE* unit, Leakage_Variable *var);
void New_Leakage_getbin (FILE* unit, New_Leakage *var);
void New_Ppg_Etalon_getbin (FILE* unit, New_Ppg_Etalon *var);
void New_Spectral_Calibration_getbin (FILE* unit, New_Spectral_Calibration *var);
void New_Sun_Reference_getbin (FILE* unit, New_Sun_Reference *var);
void Pmd_Packets_getbin (FILE* unit, Pmd_Packets *var);
void Pol_Sens_Limb_getbin (FILE* unit, Pol_Sens_Limb *var);
void Pol_Sens_Occ_getbin (FILE* unit, Pol_Sens_Occ *var);
void Pol_Sens_Nadir_getbin (FILE* unit, Pol_Sens_Nadir *var);
void Ppg_Etalon_getbin (FILE* unit, Ppg_Etalon *var);
void Rad_Sens_Limb_getbin (FILE* unit, Rad_Sens_Limb *var);
void Rad_Sens_Occ_getbin (FILE* unit, Rad_Sens_Occ *var);
void Rad_Sens_Nadir_getbin (FILE* unit, Rad_Sens_Nadir *var);
void SPH_getbin (FILE* unit, SPH *var);
void Slit_Function_getbin (FILE* unit, Slit_Function *var);
void Small_Ap_Slit_Function_getbin (FILE* unit, Small_Ap_Slit_Function *var);
void Spectral_Base_getbin (FILE* unit, Spectral_Base *var);
void Spectral_Calibration_getbin (FILE* unit, Spectral_Calibration *var);
void States_getbin (FILE* unit, States *var);
void Summary_Quality_getbin (FILE* unit, Summary_Quality *var);
void Sun_Reference_getbin (FILE* unit, Sun_Reference *var);

void Signal_array_getbin (FILE* unit, Signal *var, int nr, Cluster_Data_Type type);
/*
void Limb_getbin (FILE* unit, Limb *var);
void Monitoring_getbin (FILE* unit, Monitoring *var);
void Nadir_getbin (FILE* unit, Nadir *var);
*/

/* Writing routines */

void Auxiliary_Packets_putbin (FILE* unit, Auxiliary_Packets *var);
void Clus_Config_putbin (FILE* unit, Clus_Config *var);
void Clus_Config_array_putbin (FILE* unit, Clus_Config *var, int nr);
void Dark_Average_putbin (FILE* unit, Dark_Average *var);
void Errors_On_Key_Data_putbin (FILE* unit, Errors_On_Key_Data *var);
void Frac_Pol_putbin (FILE* unit, Frac_Pol *var);
void Frac_Pol_array_putbin (FILE* unit, Frac_Pol *var, int nr);
void Geo_putbin (FILE* unit, Geo *var);
void Geolocation_putbin (FILE* unit, Geolocation *var);
void Instrument_Params_putbin (FILE* unit, Instrument_Params *var);
void L0_Header_putbin (FILE* unit, L0_Header *var);
void L0_Header_array_putbin (FILE* unit, L0_Header *var, int nr);
void Leakage_Constant_putbin (FILE* unit, Leakage_Constant *var);
void Leakage_Variable_putbin (FILE* unit, Leakage_Variable *var);
void New_Leakage_putbin (FILE* unit, New_Leakage *var);
void New_Ppg_Etalon_putbin (FILE* unit, New_Ppg_Etalon *var);
void New_Spectral_Calibration_putbin (FILE* unit, New_Spectral_Calibration *var);
void New_Sun_Reference_putbin (FILE* unit, New_Sun_Reference *var);
void Pmd_Packets_putbin (FILE* unit, Pmd_Packets *var);
void Pol_Sens_Limb_putbin (FILE* unit, Pol_Sens_Limb *var);
void Pol_Sens_Occ_putbin (FILE* unit, Pol_Sens_Occ *var);
void Pol_Sens_Nadir_putbin (FILE* unit, Pol_Sens_Nadir *var);
void Ppg_Etalon_putbin (FILE* unit, Ppg_Etalon *var);
void Rad_Sens_Limb_putbin (FILE* unit, Rad_Sens_Limb *var);
void Rad_Sens_Nadir_putbin (FILE* unit, Rad_Sens_Nadir *var);
void Rad_Sens_Occ_putbin (FILE* unit, Rad_Sens_Occ *var);
void Slit_Function_putbin (FILE* unit, Slit_Function *var);
void Small_Ap_Slit_Function_putbin (FILE* unit, Small_Ap_Slit_Function *var);
void Spectral_Base_putbin (FILE* unit, Spectral_Base *var);
void Spectral_Calibration_putbin (FILE* unit, Spectral_Calibration *var);
void States_putbin (FILE* unit, States *var);
void Summary_Quality_putbin (FILE* unit, Summary_Quality *var);
void Sun_Reference_putbin (FILE* unit, Sun_Reference *var);

void Signal_array_putbin (FILE* unit, Signal *var, int nr, Cluster_Data_Type type);

/*
void Nadir_putbin (FILE* unit, Nadir *var);
void Limb_putbin (FILE* unit, Limb *var);
void Monitoring_putbin (FILE* unit, Monitoring *var);
*/

#endif

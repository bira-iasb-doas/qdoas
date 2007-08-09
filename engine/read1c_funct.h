#include<stdlib.h>
#include<stdio.h>
#include "lv1_defs.h"
#include "lv1_struct.h"

/*************************************************************
 * User routines
 *************************************************************/


				/* Open L1c, specify what to read */
L1C_error openL1c (char* FILE_name, info_l1c *info);

				/* Close L1c */
L1C_error closeL1c (info_l1c *info);


				/* Read Solar spectrum */
L1C_error read_solar (info_l1c *info,
		      char *sun_spec_id,
		      user_data_solar *ud);
				/* Read nadir measurements one by one */
L1C_error read_next_mds (info_l1c *info,
			 user_data *ud, L1C_MDS_type type);

				/* Select wavelength window */
L1C_error set_wl_window (info_l1c *info, float wl_start, float wl_end,
			 int channel);
			  /* Select wavelength window (with pixels) */
L1C_error set_pix_window (info_l1c *info, int pix_start, int pix_end);


/* free allocated memory */
L1C_error free_user_data (user_data *ud);
L1C_error free_user_data_solar (user_data_solar *ud);


/*************************************************************
 * Internal routines
 ************************************************************/

/* rea solar spectrum */
L1C_error read_solar_complete (info_l1c *info,
			       solar_type sol,
			       float *wl[],
			       float *irr[],
			       float *irr_err[]);




L1C_error ADS_state (info_l1c *info);

void ReadClusterInfo(ADS_STATES * state);

L1C_error Fixed_Wavelength_grid (info_l1c *info);


				/* only for internal use, will vanish
				 next release */
L1C_error read_solar_complete (info_l1c *info,
			       solar_type sol,
			       float *wl[],
			       float *irr[],
			       float *irr_err[]);

L1C_error Read_cal_options_GADS (info_l1c *info);

int MJDcompare (MJD *mjd1, MJD *mjd2);

L1C_error calc_cluster_window (info_l1c *info, L1C_MDS_type type);

L1C_error coadd_signal (unsigned int n_wl, unsigned int n_coadd,
			float *signal,           /* Input: rad[n_coadd*n_wl] */
			float *signal_err,       /* Input: rad_err[n_coadd*n_wl] */
			float *add_signal,       /* Output: rad[n_wl] */
			float *add_signal_err);   /* Output: rad_err[n_wl] */


/* Read MPH. SPH, DSD */

L1C_error Read_MPH (FILE* unit, MPH *mph);
L1C_error Read_SPH (FILE* unit, SPH *sph, MPH *mph);
L1C_error Read_DSD (FILE* unit, DSD *dsd);

/*--------------------------------------------------------------------*\
**
** Routines to read SCIAMACHY Level 1c product
**
** Author: S. Noel, IFE/IUP Uni Bremen
**        (Stefan.Noel@iup.physik.uni.bremen.de)
**
** Version: 0.1, 17 Apr 2002 (first beta release)
**
\*--------------------------------------------------------------------*/

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>

#include "lv1_defs.h"
#include "lv1_struct.h"
#include "lv1c_struct.h" /* requires "lv1_struct.h" ! */

#include "read1c_defs.h"
#include "read1c_funct.h"
#include "bin_read.h"

#include "utc_string.h"

// #define __DEBUG_L1C__

L1C_error read_next_pmd_state (info_l1c *info, L1C_MDS_type type);

/* codes for spectra; these need to be synchronised with solar_type definitions */
/* const char *const sun_ref_code[] = {"D1", "O ", "S ", "D2", "D3",
                                    "D4", "D5", "D6","D7", "D8", "D9",
                                    "D0"};
*/

/*********************************************************************\
* Open L1c, specify what to read
* reads all DSDs and stores read values in info structure
\*********************************************************************/

L1C_error openL1c (char* FILE_name, info_l1c *info)
{
    int num_dsd = 0;
    int i, cat;
    DSD dsd;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("openL1c",DEBUG_FCTTYPE_FILE);
  #endif

    info->FILE_l1c = fopen(FILE_name, "rb");

    if (info->FILE_l1c == NULL) {
	printf("Input file %s could not be opened! Abort!\n", FILE_name);
	return FILE_NOT_FOUND;
    }

    Read_MPH (info->FILE_l1c, &info->mph);
    Read_SPH (info->FILE_l1c, &info->sph, &info->mph);
#if defined(__DEBUG_L1C__)
    printf ("info->mph.product: %s\n",info->mph.product);
    printf ("info->mph.spare_1: %s\n\n",info->mph.spare_1);
#endif
    /*--------------------------------------------------------------------*\
    ** read MPH
    ** get number of DSDs
    \*--------------------------------------------------------------------*/

    sscanf(info->mph.num_dsd, "%d", &num_dsd);
    info->num_dsd = num_dsd;

    /*--------------------------------------------------------------------*\
    ** read DSDs
    note: num_dsd seems to be too high by 1 (spare entry)!
    \*--------------------------------------------------------------------*/

    for (i=0; i<info->num_dsd-1; i++)
    {
	Read_DSD ( info->FILE_l1c, &dsd );

#if defined(__DEBUG_L1C__)
{FILE *fp;
fp=fopen("toto.dat","a+t");
	fprintf (fp,"%s\t%c\t%s\t%i\t%i\t%i\t%i\n",
		dsd.name,
		dsd.type,
		dsd.filename,
		dsd.offset,
		dsd.size,
		dsd.num_dsr,
		dsd.dsr_size);
		fclose(fp);
	}
#endif
	/* check for dsd names */
	if (strcmp(dsd.name, "STATES") == 0) {
	    info->states = dsd;
	}
	else if (strcmp(dsd.name, "SUN_REFERENCE") == 0) {
	    info->sun_ref = dsd;
	}
	else if (strcmp(dsd.name, "NADIR_PMD") == 0) {
	    info->nadir_pmd = dsd;
	}
	else if (strcmp(dsd.name, "LIMB_PMD") == 0) {
	    info->limb_pmd = dsd;
	}
	else if (strcmp(dsd.name, "OCCULTATION_PMD") == 0) {
	    info->occ_pmd = dsd;
	}
	else if (strcmp(dsd.name, "NADIR_FRAC_POL") == 0) {
	    info->nadir_pol = dsd;
	}
	else if (strcmp(dsd.name, "LIMB_FRAC_POL") == 0) {
	    info->limb_pol = dsd;
	}
	else if (strcmp(dsd.name, "OCCULTATION_FRAC_POL") == 0) {
	    info->occ_pol = dsd;
	}
	else if (strcmp(dsd.name, "NADIR") == 0) {
	    info->nadir = dsd;
	}
	else if (strcmp(dsd.name, "LIMB") == 0) {
	    info->limb = dsd;
	}
	else if (strcmp(dsd.name, "OCCULTATION") == 0) {
	    info->occ = dsd;
	}
	else if (strcmp(dsd.name, "MONITORING") == 0) {
	    info->mon = dsd;
	}
	else if (strcmp(dsd.name, "CAL_OPTIONS") == 0) {
	    info->cal_options = dsd;
	}
	else {
	    /* do nothing */
	}
    }
#if defined(__DEBUG_L1C__)
    printf ("open_lv1c");
    printf ("NADIR DSD %i\n",info->nadir.num_dsr);
#endif

    /* Put MDS offsets in array */
    info->mds_offset[NADIR] = info->nadir.offset;
    info->mds_offset[LIMB] = info->limb.offset;
    info->mds_offset[OCCULTATION] = info->occ.offset;
    info->mds_offset[MONITORING] = info->mon.offset;
    /* Put MDS num_dsr in array */
    info->n_mds[NADIR] = info->nadir.num_dsr;
    info->n_mds[LIMB] = info->limb.num_dsr;
    info->n_mds[OCCULTATION] = info->occ.num_dsr;
    info->n_mds[MONITORING] = info->mon.num_dsr;
    /* Put MDS num_dsr in array */
    info->n_pmd_mds[NADIR]       = info->nadir_pmd.num_dsr;
    info->n_pmd_mds[LIMB]        = info->limb_pmd.num_dsr;
    info->n_pmd_mds[OCCULTATION] = info->occ_pmd.num_dsr;
    info->n_pmd_mds[MONITORING]  = 0;
    /* Put MDS num_dsr in array */
    info->mds_pmd_offset[NADIR]       = info->nadir_pmd.offset;
    info->mds_pmd_offset[LIMB]        = info->limb_pmd.offset;
    info->mds_pmd_offset[OCCULTATION] = info->occ_pmd.offset;
    info->mds_pmd_offset[MONITORING]  = 0;

    /*  Read Calibr. Options */
    Read_cal_options_GADS (info);

    /*  fill in general user_file_info */
    info->user_file_info.product = info->mph.product;
    info->user_file_info.software_ver = info->mph.software_ver;
    info->user_file_info.abs_orbit = atoi (info->mph.abs_orbit);
    info->user_file_info.key_data_version = info->sph.key_data_version;
    info->user_file_info.m_factor_version = info->sph.m_factor_version;
    info->user_file_info.l1b_product_name = info->cal_options_GADS.l1b_product_name;
    i = 0;
    *info->user_file_info.cal_applied = '\0';
    if ( info->cal_options_GADS.mem_effect_cal_flag== -1 )
    {
	sprintf ( info->user_file_info.cal_applied + 2*i , " 0");
	i++;
    }
    if ( info->cal_options_GADS.leakage_current_cal_flag == -1 )
    {
	sprintf ( info->user_file_info.cal_applied + 2*i , " 1");
	i++;
    }
    if ( info->cal_options_GADS.straylight_cal_flag== -1 )
    {
	sprintf ( info->user_file_info.cal_applied + 2*i , " 2");
	i++;
    }
    if ( info->cal_options_GADS.ppg_cal_flag == -1 )
    {
	sprintf ( info->user_file_info.cal_applied + 2*i , " 3");
	i++;
    }
    if ( info->cal_options_GADS.etalon_cal_flag == -1 )
    {
	sprintf ( info->user_file_info.cal_applied + 2*i , " 4");
	i++;
    }
    if ( info->cal_options_GADS.spectral_cal_flag  == -1 )
    {
	sprintf ( info->user_file_info.cal_applied + 2*i , " 5");
	i++;
    }
    if ( info->cal_options_GADS.polarisation_cal_flag == -1 )
    {
	sprintf ( info->user_file_info.cal_applied + 2*i , " 6");
	i++;
    }
    if ( info->cal_options_GADS.radiance_cal_flag  == -1 )
    {
	sprintf ( info->user_file_info.cal_applied + 2*i , " 7");
	i++;
    }

/* Read ADS states  */
    if ( ADS_state (info) != OK )
	return NO_ADS_STATES;
/* Count really available states */
#if defined(__DEBUG_L1C__)
    printf ("open_lv1c\n"
	    "Num nadir states: %i"
	    "\n Num limb states: %i\n",
	    info->mds_offset[NADIR], info->mds_offset[LIMB]);
#endif
/* Build index lists for MDS <> states */
/* Trust that both are in consistent order */
/*  initialize arrays */
    for (cat = 0 ; cat < MAX_MDS_TYPES ; cat++)
    {
	info->idx_states[cat] = (int*) malloc (sizeof(int) *
					info->states.num_dsr);
	info->n_states[cat] = 0;
    }
/*  go through all states*/
    for (i=0; i < (int)info->states.num_dsr; i++)
    {
	switch (info->ads_states[i].category)
	{
	    /*  count up categories and store index */
	    case 1:
	    case 3:
	    case 17:
	    case 18:
		info->idx_states[NADIR][info->n_states[NADIR]++] = i;
		break;
	    case 2:
		info->idx_states[LIMB][info->n_states[LIMB]++] = i;
		break;
	    case 4:
	    case 5:
	    case 6:
		info->idx_states[OCCULTATION][info->n_states[OCCULTATION]++] = i;
		break;
	    case 7:
	    case 8:
	    case 9:
	    case 10:
	    case 11:
	    case 12:
	    case 13:
	    case 14:
	    case 15:
	    case 16:
		info->idx_states[MONITORING][info->n_states[MONITORING]++] = i;
		break;
	    default:
#if defined(__DEBUG_L1C__)
		printf ("open_lv1c\n"
			" Measurement cat  %i not implemented yet ", cat);
#endif
		;
	}
    }
/*  check if corresponding MDS are available */
/*  if not, simply set n_state[type] to zero */

    for (cat = 0 ; cat < MAX_MDS_TYPES ; cat++)
    {
	if ( info->n_mds[cat] == 0 )
	    info->n_states[cat] = 0;
    }

/* Read solar spektrum for wl-grid */
    Fixed_Wavelength_grid (info);
/* set various counters to zero */
    info->cur_readout_in_state = 0;
    info->cur_max_readout_in_state = 0;
    info->cur_pix_start = 0;
    info->cur_pix_end = 0;

    return OK;
}


L1C_error Fixed_Wavelength_grid (info_l1c *info)
{
    int n;
    /* use the pre-launch wavelength grid, stored in wvlen_det_pix.c
       in one large array */
#include "wvlen_det_pix.c"
    for (n=0; n<NPIXEL; n++)
	info->wl[n] = (float)wvlen_det_pix[n];

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("openL1c",DEBUG_FCTTYPE_FILE);
  #endif

    return OK;
}

/*********************************************************************\
 * Define pixel window from wavelength window
 * kb 23.04.01
 * channel : specify you want to read (1 - 8) to avoid problems in
 *           overlap regions (data from other channel)
 *           channel=0  : Do not restrict channel
\*********************************************************************/

L1C_error set_wl_window (info_l1c *info, float wl_start, float wl_end,
			 int channel)
{
    int n;
    int search_start=0;
    int search_end=NPIXEL;
#if defined(__DEBUG_L1C__)
    printf ("%8.2f %8.2f\n",wl_start,wl_end);
#endif
    if (channel > 0 && channel < 7 )
    {
	search_start = (channel-1) * 1024;
	search_end = channel * 1024;
    }


    for (n=search_start ; n<search_end; n++)
    {
	if (info->wl[n] > wl_start)
	    break;
    }
    if ( n == search_end )
	return WL_OUT_OF_RANGE;
    if ( n > search_start)
	info->cur_pix_start = n-1;
    else
	info->cur_pix_start = n;

    for (n = search_end; n>search_start; n--)
    {
	if ( info->wl[n-1] < wl_end )
	    break;
    }
    if (n == 0)
	return WL_OUT_OF_RANGE;
    info->cur_pix_end = n;

    if (info->cur_pix_start >= info->cur_pix_end)
	return WL_OUT_OF_RANGE;
    return OK;
}


/*********************************************************************\
 * Define pixel window from pixel_nr
 * kb 23.04.01
\*********************************************************************/

L1C_error set_pix_window (info_l1c *info, int pix_start, int pix_end)
{
    if (pix_start < 0 || pix_end > 8192 || pix_start >= pix_end)
	return  WL_OUT_OF_RANGE;
    info->cur_pix_end = pix_end;
    info->cur_pix_start = pix_start;
    return OK;
}




/*********************************************************************\
 * Close L1c
\*********************************************************************/

L1C_error closeL1c (info_l1c *info)
{
    free (info->ads_states);
    free (info->mjd_nadir_mds);
    free (info->mjd_limb_mds);
    free (info->idx_nadir_mds);
    free (info->idx_limb_mds);

    if ((info->FILE_l1c!=NULL) && (fclose(info->FILE_l1c)!= 0)) {
	return FILE_NOT_CLOSED;
  }
  return OK;
}


/*********************************************************************\
 * Free memory in user_data, allocated in read_next....
\*********************************************************************/

L1C_error free_user_data (user_data *ud)

{
    free (ud->pixel_nr);
    free (ud->pixel_cls);
    free (ud->pixel_ch);
    free (ud->pixel_coadd);
    free (ud->wl);
    free (ud->wl_err);
    free (ud->signal);
    free (ud->signal_err);
    if (ud->n_pmd > 0)
	free (ud->pmd);
    return OK;
}

L1C_error free_user_data_solar (user_data_solar *ud)
{
    free (ud->pixel_nr);
    free (ud->pixel_ch);
    free (ud->wl);
    free (ud->signal);
    free (ud->signal_err);
    return OK;
}

/*********************************************************************\
 * Read cal_options_GADS
 * kb, 23.04.02
\*********************************************************************/

L1C_error Read_cal_options_GADS (info_l1c *info)
{
    int cluster;

				/* go to GADS cal_options */
    if (fseek(info->FILE_l1c, info->cal_options.offset, SEEK_SET) != -1)
    {
				/* read into memory (include swapping) */
	cal_options_GADS_getbin (info->FILE_l1c, &info->cal_options_GADS);
				/* Set list of included clusters */
	for (cluster=0, info->max_cluster_ids=0; cluster<MAX_CLUSTER; cluster++)
	{
	    if (info->cal_options_GADS.nadir_cluster_flag[cluster] == 0)
		continue;		/* Cluster not available */
	    info->cluster_ids [info->max_cluster_ids++] = cluster;
	}
	return OK;
    }
    return NO_GADS_CAL_OPTIONS;
}



/*********************************************************************\
 * Compare two dates MJD
 * MJD compare : 0 equal  -1 earlier  1 later
 * kb 23.04.01
\*********************************************************************/

int MJDcompare (MJD *mjd1, MJD *mjd2)
{
    if (mjd1->days < mjd2->days)
	return -1;
    if (mjd1->days > mjd2->days)
	return 1;
				/* Days are equal !! */
    if (mjd1->secnd < mjd2->secnd)
	return -1;
    if (mjd1->secnd > mjd2->secnd)
	return 1;
				/* secnd are equal !! */
    if (mjd1->musec < mjd2->musec)
	return -1;
    if (mjd1->musec > mjd2->musec)
	return 1;
				/* musec are equal !! */
    return 0;
}

/**********************************************************************
 * add 1/16 seconds to MJD
 **********************************************************************/

MJD MJDadd (MJD mjd, int sec_16)
{
    unsigned sec_16_factor = 62500;
    unsigned secnd;
    unsigned musec = mjd.musec;
    mjd.musec = (musec + sec_16 * sec_16_factor) % 1000000;
    secnd = mjd.secnd +
	(musec + sec_16 * sec_16_factor) / 1000000;
    mjd.secnd = secnd % 86400;
    mjd.days = mjd.days + secnd / 86400;
    return mjd;
}

/*********************************************************************\
 * Read complete state info
\*********************************************************************/

L1C_error ADS_state (info_l1c *info)
{
    unsigned num_states, istate;
    ADS_STATES *state;
    num_states = info->states.num_dsr; /* No. of states */
    if (num_states == 0)
	return NO_ADS_STATES;
    if (fseek(info->FILE_l1c, info->states.offset, SEEK_SET) != -1)
    {
				/* reserve memory for states */
	info->ads_states = (ADS_STATES*) malloc (num_states * sizeof(ADS_STATES));
				/* easy access */
	state = info->ads_states;
				/* read and swap states */
	for (istate=0; istate<num_states;istate++)
	{
	    ads_states_getbin (info->FILE_l1c, &state[istate]);
				/* check the attach flag */
	    if ( state[istate].mds_attached != 0 )
	    {			/* ignore states without MDS,  */
		istate--;	/* set counters one back */
		num_states--;
		info->states.num_dsr--;
	    }

#if defined(__DEBUG_L1C__)
	    printf("State %2i %2i %5i %4i %2i\n",
		   istate,
		   state[istate].state_id,
		   state[istate].duration,
		   state[istate].num_clusters,
		   state[istate].num_dsr);
#endif
	}
	return OK;
    }
    return NO_ADS_STATES;
}





/*********************************************************************\
 * read solar
\*********************************************************************/

L1C_error read_solar (info_l1c *info,
		      char *sun_spec_id,
		      user_data_solar *ud)
{
  gads_sun_ref sun_ref;
  int pix_start, pix_end, pix_delta;
  int iset;
  int i,n;

				/* are there sun reference data sets? */
  if (info->sun_ref.num_dsr <= 0) {
    return NO_SUN_REF;
  }
				/* position file */
  if (fseek(info->FILE_l1c, info->sun_ref.offset, SEEK_SET) != 0) {
    return NO_SUN_REF;
  }
				/* choose needed pixels */
  pix_start = info->cur_pix_start;
  pix_end = info->cur_pix_end;
  pix_delta = pix_end - pix_start;

  ud->n_wl = pix_delta;

				/* allocate memory for output */
  ud->pixel_nr = (unsigned short*) malloc(pix_delta*sizeof(unsigned short));
  ud->pixel_ch = (unsigned short*) malloc(pix_delta*sizeof(unsigned short));
  ud->wl = (float*)malloc(pix_delta*sizeof(float));
  ud->signal = (float*)malloc(pix_delta*sizeof(float));
  ud->signal_err = (float*)malloc(pix_delta*sizeof(float));



  for (iset=0; iset<(int)info->sun_ref.num_dsr;iset++) {

      gads_sun_ref_getbin (info->FILE_l1c, &sun_ref);
/*     sun_ref = ReadSunReference(info->FILE_l1c); */

      if (strncmp(sun_ref.id, sun_spec_id, 2) == 0)
      {
	  for (i=pix_start,n=0; i<pix_end; i++,n++)
	  {
	      ud->pixel_nr[n] = i%1024;
	      ud->pixel_ch[n] = i/1024+1;
	      ud->wl[n] = sun_ref.wavel[i];
	      ud->signal[n] = sun_ref.spectrum[i];
	      ud->signal_err[n] = sun_ref.accuracy[i];
	/* also available: sun_ref.etalon[i], sun_ref.precision[i] */
	  }
	  break;			/* no need to read further */
      }
  }
  if (iset == (int)info->sun_ref.num_dsr) {
      printf("no solar data available\n");
      return NO_SUN_REF_DATA;
  }
  /* Copy Sun_spec_id */
  strncpy (ud->sun_spect_id,  sun_ref.id, 3);
  /* Copy orbit info */
  ud->product            = info->user_file_info.product ;
  ud->software_ver       = info->user_file_info.software_ver ;
  ud->abs_orbit          = info->user_file_info.abs_orbit;
  ud->key_data_version   = info->user_file_info.key_data_version ;
  ud->m_factor_version   = info->user_file_info.m_factor_version ;
  ud->l1b_product_name   = info->user_file_info.l1b_product_name;
  ud->cal_applied        = info->user_file_info.cal_applied;

#if defined(__DEBUG_L1C__)
  printf("solar spectrum successfully read\n");
#endif
  return OK;
}



/*********************************************************************\
 * Read MDS (1c) complete
\*********************************************************************/

/*********************************************************************\
 * Clean up memory from previous read nadir state
 * kb 21.07.02
\*********************************************************************/


L1C_error clear_state (info_l1c *info, L1C_MDS_type type)
{
    int cluster,id;
    if (info->cur_state_nr[type] > 0)
    {
	for (cluster=0; cluster<info->cur_max_cluster_ids; cluster++)
	{
	    id = info->cur_cluster_ids[cluster];
	    free (info->st_cl_data[id].pixel);
	    free (info->st_cl_data[id].wl);
	    free (info->st_cl_data[id].wl_err);
	    free (info->st_cl_data[id].signal);
	    free (info->st_cl_data[id].signal_err);
	    switch (info->st_cl_data[id].type)
	    {
		case NADIR:
		    free (info->st_cl_data[id].geo_nadir);
		    break;
		case LIMB:
		case OCCULTATION:
		    free (info->st_cl_data[id].geo_limb);
		    break;
		case MONITORING:
		    free (info->st_cl_data[id].geo_cal);
		    break;
		default:
		    fprintf(stderr, "L1C_error clear_nadir_state("
			    "info_l1c *info) : Invalid measurement type!\n");
		    exit (1);
		    break;
	    }
	}
    }
    return OK;
}

/*********************************************************************\
 * Coadding data and errors
 * kb 23.04.01
\*********************************************************************/

L1C_error coadd_signal (unsigned int n_wl, unsigned int n_coadd,
			float *signal,           /* Input: rad[n_coadd*n_wl] */
			float *signal_err,       /* Input: rad_err[n_coadd*n_wl] */
			float *add_signal,       /* Output: rad[n_wl] */
			float *add_signal_err)   /* Output: rad_err[n_wl] */
{
    int nw, nc;
    for (nw = 0; nw < (int)n_wl; nw++)
    {
	add_signal[nw] = 0.0;
	add_signal_err[nw] = 0.0;
	for (nc = 0; nc < (int)n_coadd; nc++)
	{
	    add_signal[nw] += signal[ nc*n_wl + nw ];
	    add_signal_err[nw] += signal_err[ nc*n_wl + nw ] * signal_err[ nc*n_wl + nw ];
	}
	add_signal[nw] = add_signal[nw] / (float)n_coadd;
				/* !!!!!!!!!!! Check coadding errors : OK*/
	add_signal_err[nw] = (float)(sqrt (add_signal_err[nw]) / (float)n_coadd);
    }
    return OK;
}

L1C_error Read_MPH (FILE* unit, MPH *mph)

{
    size_t n;
    char *tmp = (char*) mph;
    for (n=0; n<sizeof(MPH); n++)
	tmp[n] = 0;

    fscanf(unit,
	   "PRODUCT=\"%62c\"\n"
	   "PROC_STAGE=%1c\n"
	   "REF_DOC=\"%23c\"\n"
/*	   "%40c\n" */
	   "ACQUISITION_STATION=\"%20c\"\n"
	   "PROC_CENTER=\"%6c\"\n"
	   "PROC_TIME=\"%27c\"\n"
	   "SOFTWARE_VER=\"%14c\"\n"
/*	   "%40c\n" */
	   "SENSING_START=\"%27c\"\n"
	   "SENSING_STOP=\"%27c\"\n"
/*	   "%40c\n" */
	   "PHASE=%1c\n"
	   "CYCLE=%4c\n"
	   "REL_ORBIT=%6c\n"
	   "ABS_ORBIT=%6c\n"
	   "STATE_VECTOR_TIME=\"%27c\"\n"
	   "DELTA_UT1=%8c<s>\n"
	   "X_POSITION=%12c<m>\n"
	   "Y_POSITION=%12c<m>\n"
	   "Z_POSITION=%12c<m>\n"
	   "X_VELOCITY=%12c<m/s>\n"
	   "Y_VELOCITY=%12c<m/s>\n"
	   "Z_VELOCITY=%12c<m/s>\n"
	   "VECTOR_SOURCE=\"%2c\"\n"
/*	   "%40c\n" */
	   "UTC_SBT_TIME=\"%27c\"\n"
	   "SAT_BINARY_TIME=%11c\n"
	   "CLOCK_STEP=%11c<ps>\n"
/*	   "%32c\n" */
	   "LEAP_UTC=\"%27c\"\n"
	   "LEAP_SIGN=%4c\n"
	   "LEAP_ERR=%1c\n"
/*	   "%40c\n" */
	   "PRODUCT_ERR=%1c\n"
	   "TOT_SIZE=%21c<bytes>\n"
	   "SPH_SIZE=%11c<bytes>\n"
	   "NUM_DSD=%11c\n"
	   "DSD_SIZE=%11c<bytes>\n"
	   "NUM_DATA_SETS=%11c\n",
	   mph->product,
	   mph->proc_stage,
	   mph->ref_doc ,
/*	   mph->spare_1 , */
	   mph->acquisition_station ,
	   mph->proc_center ,
	   mph->proc_time ,
	   mph->software_ver ,
/*	   mph->spare_2 ,*/
	   mph->sensing_start ,
	   mph->sensing_stop ,
/*	   mph->spare_3 , */
	   mph->phase ,
	   mph->cycle ,
	   mph->rel_orbit ,
	   mph->abs_orbit ,
	   mph->state_vector_time ,
	   mph->delta_ut1 ,
	   mph->x_position ,
	   mph->y_position ,
	   mph->z_position ,
	   mph->x_velocity ,
	   mph->y_velocity ,
	   mph->z_velocity ,
	   mph->vector_source ,
/*	   mph->spare_4 , */
	   mph->utc_sbt_time ,
	   mph->sat_binary_time ,
	   mph->clock_step ,
/*	   mph->spare_5 , */
	   mph->leap_utc ,
	   mph->leap_sign ,
	   mph->leap_err ,
/*	   mph->spare_6 , */
	   mph->product_err ,
	   mph->tot_size ,
	   mph->sph_size ,
	   mph->num_dsd ,
	   mph->dsd_size ,
	   mph->num_data_sets
/*	   mph->spare_7 */ );

    return OK;
}


L1C_error Read_SPH (FILE* unit, SPH *sph, MPH *mph)

{
    size_t n;
    float swver;
    char *tmp = (char*) sph;
    char iv[100];

    for (n=0; n<sizeof(SPH); n++)
	tmp[n] = 0;

    fscanf(unit,
	   "SPH_DESCRIPTOR=\"%28c\"\n"
	   "STRIPLINE_CONTINUITY_INDICATOR=%4c\n"
	   "SLICE_POSITION=%4c\n"
	   "NUM_SLICES=%4c\n"
	   "START_TIME=\"%27c\"\n"
	   "STOP_TIME=\"%27c\"\n"
	   "START_LAT=%11c<10-6degN>\n"
	   "START_LONG=%11c<10-6degE>\n"
	   "STOP_LAT=%11c<10-6degN>\n"
	   "STOP_LONG=%11c<10-6degE>\n",
	   sph->sph_descriptor ,
	   sph->stripline_continuity_indicator ,
	   sph->slice_position ,
	   sph->num_slices ,
	   sph->start_time ,
	   sph->stop_time ,
	   sph->start_lat ,
	   sph->start_long ,
	   sph->stop_lat ,
	   sph->stop_long );

/*
  For SCIA versions up to 5.04 the next line in the SPH is
  empty but for 50 spaces. This line is automatically skipped
  when reading the next line which has KEY_DATA_VERSION.

  For SCIA versions as of 6.00 the next line in the SPH is a
  string starting with INIT_VERSION. This line has to be read
  properly in order for the next read of KEY_DATA_VERSION to
  work well.

  Hence, determine from the software version -- which is read
  in the MPH -- whether to read an extra line.
  [Jos van Geffen, 1 August 2006]
*/

    sscanf(mph->software_ver,"SCIA/%5f     ",&swver);
    if ( swver >= 6.0 )
       fscanf(unit, "%50c\n", iv);

    fscanf(unit,
	   "KEY_DATA_VERSION=\"%5c\"\n"
	   "M_FACTOR_VERSION=\"%5c\"\n"
	   "SPECTRAL_CAL_CHECK_SUM=\"%4c\"\n"
	   "SATURATED_PIXEL=\"%4c\"\n"
	   "DEAD_PIXEL=\"%4c\"\n"
	   "DARK_CHECK_SUM=\"%4c\"\n"
	   "NO_OF_NADIR_STATES=%4c\n"
	   "NO_OF_LIMB_STATES=%4c\n"
	   "NO_OF_OCCULTATION_STATES=%4c\n"
	   "NO_OF_MONI_STATES=%4c\n"
	   "NO_OF_NOPROC_STATES=%4c\n"
	   "COMP_DARK_STATES=%4c\n"
	   "INCOMP_DARK_STATES=%4c\n",
	   sph->key_data_version ,
	   sph->m_factor_version ,
	   sph->spectral_cal_check_sum ,
	   sph->saturated_pixel ,
	   sph->dead_pixel ,
	   sph->dark_check_sum ,
	   sph->no_of_nadir_states ,
	   sph->no_of_limb_states ,
	   sph->no_of_occultation_states ,
	   sph->no_of_moni_states ,
	   sph->no_of_noproc_states ,
	   sph->comp_dark_states ,
	   sph->incomp_dark_states );

    return OK;
};


L1C_error Read_DSD (FILE* unit, DSD *dsd)
{
    int err;
    char *str_ptr;
    err=fscanf(unit,
	   "DS_NAME=\"%28c\"\n"
	   "DS_TYPE=%1c\n"
	   "FILENAME=\"%62c\"\n"
	   "DS_OFFSET=+%20u<bytes>\n"
	   "DS_SIZE=%21u<bytes>\n"
	   "NUM_DSR=%11u\n"
	   "DSR_SIZE=%11d<bytes>\n",
	   dsd->name,
	   &dsd->type,
	   dsd->filename,
	   &dsd->offset,
	   &dsd->size,
	   &dsd->num_dsr,
	   &dsd->dsr_size);

    dsd->name[28]='\0';
    if ( (str_ptr = strpbrk(dsd->name, " ") ) != NULL)
	*str_ptr='\0';

    dsd->filename[62]='\0';
    if ( (str_ptr = strpbrk(dsd->filename, " ") ) != NULL)
	*str_ptr='\0';

    return OK;
}

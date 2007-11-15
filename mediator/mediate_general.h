/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef _MEDIATE_GENERAL_H_GUARD
#define _MEDIATE_GENERAL_H_GUARD

#include "mediate_limits.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

  
  /*************************************************************************/
  /* Symbol */

  typedef struct mediate_symbol
  {
    char name[SYMBOL_NAME_BUFFER_LENGTH];
    char description[SYMBOL_DESCR_BUFFER_LENGTH];
  } mediate_symbol_t;
  
  
  /*************************************************************************/
  /* Site */
  
  typedef struct mediate_site
  {
    char name[SITE_NAME_BUFFER_LENGTH];
    char abbreviation[SITE_ABBREV_BUFFER_LENGTH];
    double longitude;
    double latitude;
    double altitude;
  } mediate_site_t;
  
  
  /*************************************************************************/
  /* Slits */

  struct slit_file {
    char filename[FILENAME_BUFFER_LENGTH];
  };

  struct slit_gaussian {
    double fwhm;
  };

  struct slit_lorentz {
    double width;
    int degree;
  };

  struct slit_voigt {
    double fwhmL, fwhmR;
    double glRatioL, glRatioR;
  };

  struct slit_error {
    double fwhm;
    double width;
  };

  struct slit_apod {
    double resolution;
    double phase;
  };

  struct slit_lorentz_file {
    char filename[FILENAME_BUFFER_LENGTH];
    int degree;
  };

  struct slit_error_file {
    char filename[FILENAME_BUFFER_LENGTH];
    double width;
  };


  /*************************************************************************/
  /* Filters */

  struct filter_kaiser
  {
    double cutoffFrequency;
    double tolerance;
    double passband;
    int iterations;
  };

  struct filter_boxcar
  {
    int width;            /* odd number of pixels */
    int iterations;
  };

  struct filter_gaussian
  {
    double fwhm;          /* pixels */
    int iterations;
  };

  struct filter_triangular
  {
    int width;            /* odd number of pixels */
    int iterations;
  };

  struct filter_savitzky_golay
  {
    int width;            /* odd number of pixels */
    int order;            /* even number */
    int iterations;
  };

  struct filter_binomial
  {
    int width;            /* odd number of pixels */
    int iterations;
  };

  typedef struct mediate_filter
  {
    int mode;
    struct filter_kaiser kaiser;
    struct filter_boxcar boxcar;
    struct filter_gaussian gaussian;
    struct filter_triangular triangular;
    struct filter_savitzky_golay savitzky;
    struct filter_binomial binomial;
  } mediate_filter_t;


  /*************************************************************************/
  /* Components shared by project and analysis window */

  struct anlyswin_cross_section
  {
    char symbol[SYMBOL_NAME_BUFFER_LENGTH];
    char crossSectionFile[FILENAME_BUFFER_LENGTH];    /* the cross section filename */
    char orthogonal[SYMBOL_NAME_BUFFER_LENGTH];       /* a symbol or predefined constant */
    int crossType;
    int amfType;
    char amfFile[FILENAME_BUFFER_LENGTH];
    int requireFit;
    int requireFilter;
    int requireCcFit;
    double initialCc;
    double deltaCc;
    double ccIo;
  };

  struct anlyswin_linear
  {
    int xPolyOrder;
    int xBaseOrder;
    int xFlagFitStore;
    int xFlagErrStore;

    int xinvPolyOrder;
    int xinvBaseOrder;
    int xinvFlagFitStore;
    int xinvFlagErrStore;

    int offsetPolyOrder;
    int offsetBaseOrder;
    int offsetFlagFitStore;
    int offsetFlagErrStore;
  };

  struct anlyswin_nonlinear
  {
    int solFlagFit;
    double solInitial;
    double solDelta;
    int solFlagFitStore;
    int solFlagErrStore;

    int off0FlagFit;
    double off0Initial;
    double off0Delta;
    int off0FlagFitStore;
    int off0FlagErrStore;

    int off1FlagFit;
    double off1Initial;
    double off1Delta;
    int off1FlagFitStore;
    int off1FlagErrStore;

    int off2FlagFit;
    double off2Initial;
    double off2Delta;
    int off2FlagFitStore;
    int off2FlagErrStore;

    int comFlagFit;
    double comInitial;
    double comDelta;
    int comFlagFitStore;
    int comFlagErrStore;

    int usamp1FlagFit;
    double usamp1Initial;
    double usamp1Delta;
    int usamp1FlagFitStore;
    int usamp1FlagErrStore;

    int usamp2FlagFit;
    double usamp2Initial;
    double usamp2Delta;
    int usamp2FlagFitStore;
    int usamp2FlagErrStore;

    int ramanFlagFit;
    double ramanInitial;
    double ramanDelta;
    int ramanFlagFitStore;
    int ramanFlagErrStore;
  };

  struct anlyswin_shift_stretch
  {
    int nSymbol;
    char symbol[MAX_AW_SHIFT_STRETCH][SYMBOL_NAME_BUFFER_LENGTH];
    int shFit;
    int stFit;
    int scFit;
    int shStore;
    int stStore;
    int scStore;
    int errStore;
    double shInit;
    double stInit;
    double stInit2;
    double scInit;
    double scInit2;
    double shDelta;
    double stDelta;
    double stDelta2;
    double scDelta;
    double scDelta2;
    double shMin;
    double shMax;
  };

  struct anlyswin_gap
  {
    double minimum;
    double maximum;
  };

  struct anlyswin_output
  {
    char symbol[SYMBOL_NAME_BUFFER_LENGTH];
    int amf;
    // residual
    int slantCol;
    int slantErr;
    double slantFactor;
    int vertCol;
    int vertErr;
    double vertFactor;
  };

  struct calibration_sfp
  {
    int fitFlag;
    double initialValue;
    double deltaValue;
    int fitStore;
    int errStore;
  };


  /*************************************************************************/
  /* struct array wrappers ... */

  typedef struct cross_section_list
  {
    int nCrossSection;
    struct anlyswin_cross_section crossSection[MAX_AW_CROSS_SECTION];
  } cross_section_list_t;

  typedef struct shift_stretch_list
  {
    int nShiftStretch;
    struct anlyswin_shift_stretch shiftStretch[MAX_AW_SHIFT_STRETCH];
  } shift_stretch_list_t;

  typedef struct gap_list
  {
    int nGap;
    struct anlyswin_gap gap[MAX_AW_GAP];
  } gap_list_t;

  typedef struct output_list
  {
    int nOutput;
    struct anlyswin_output output[MAX_AW_CROSS_SECTION];
  } output_list_t;

  typedef struct data_select_list
  {
    int nSelected;
    unsigned char selected[256];
  } data_select_list_t;

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

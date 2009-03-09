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


#ifndef _MEDIATE_PROJECT_H_GUARD
#define _MEDIATE_PROJECT_H_GUARD

#include "mediate_limits.h"
#include "mediate_general.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif



  /****************************************************/
  /* Geolocation */

  /* cGeolocationModeCircle */
  struct geolocation_circle
  {
    double radius;
    double centerLongitude;
    double centerLatitude;
  };

  /* cGeolocationModeRectangle */
  struct geolocation_rectangle
  {
    double westernLongitude;
    double easternLongitude;
    double southernLatitude;
    double northernLatitude;
  };

  /* cGeolocationModeSites */
  struct geolocation_sites
  {
    double radius;
  };

  /* all gelocations and a selection (mode) */
  struct geolocation
  {
    int mode;
    struct geolocation_circle circle;
    struct geolocation_rectangle rectangle;
    struct geolocation_sites sites;
  };

  /****************************************************/
  /* Project Spectra */

  typedef struct mediate_project_spectra
  {
    /* Data required from engine for Display purposes */
    int requireSpectra;
    int requireData;
    int requireFits;

    /* SZA (Solar Zenith Angle) range of interest */
    double szaMinimum;
    double szaMaximum;
    double szaDelta;

    /* Spectral record range */
    int recordNumberMinimum;
    int recordNumberMaximum;

    /* boolean flags for separate dark and name files */
    int useDarkFile;
    int useNameFile;

    /* geolocation limits */
    struct geolocation geo;

  } mediate_project_spectra_t;


  /****************************************************/
  /* Project Display */

  typedef struct mediate_project_display
  {
    /* Data required from engine for Display purposes */
    int requireSpectra;
    int requireData;
    int requireFits;
    data_select_list_t selection;
  } mediate_project_display_t;


  /****************************************************/
  /* Project Selection */

  typedef struct mediate_project_selection
  {
    /* SZA (Solar Zenith Angle) range of interest */
    double szaMinimum;
    double szaMaximum;
    double szaDelta;

    /* Spectral record range */
    int recordNumberMinimum;
    int recordNumberMaximum;

    /* boolean flags for separate dark and name files */
    int useDarkFile;
    int useNameFile;

    /* geolocation limits */
    struct geolocation geo;

  } mediate_project_selection_t;


  /****************************************************/
  /* Project Analysis */

  typedef struct mediate_project_analysis
  {
    int methodType;
    int fitType;
    int unitType;
    int interpolationType;
    int interpolationSecurityGap;
    double convergenceCriterion;
  } mediate_project_analysis_t;


  /****************************************************/
  /* Project Filtering - mediate_filter_t */


  /****************************************************/
  /* Project Calibration */

  typedef struct mediate_project_calibration
  {
    char solarRefFile[FILENAME_BUFFER_LENGTH];
    int methodType;
    int subWindows;
    int lineShape;
    int lorentzDegree;
    int shiftDegree;
    int sfpDegree;
    double wavelengthMin;
    double wavelengthMax;
    int requireSpectra;
    int requireFits;
    int requireResidual;
    int requireShiftSfp;
    /* table data ... */
    cross_section_list_t crossSectionList;
    struct anlyswin_linear linear;
    struct calibration_sfp sfp[4]; // SFP1 .. SFP4
    shift_stretch_list_t shiftStretchList;
    gap_list_t gapList;
    output_list_t outputList;

  } mediate_project_calibration_t;


  /****************************************************/
  /* Project Undersampling */

  typedef struct mediate_project_undersampling
  {
    char solarRefFile[FILENAME_BUFFER_LENGTH];
    int method;
    double shift;
  } mediate_project_undersampling_t;


  /****************************************************/
  /* Project Instrumental */

  struct instrumental_ascii
  {
    int detectorSize;
    int format;
    int flagZenithAngle;
    int flagAzimuthAngle;
    int flagElevationAngle;
    int flagDate;
    int flagTime;
    int flagWavelength;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_logger {
    int spectralType;
    int flagAzimuthAngle;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_acton {
    int niluType;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_saoz {
    int spectralRegion;
    int spectralType;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_mfc {
    int detectorSize;
    int revert;
    int autoFileSelect;
    int firstWavelength;
    unsigned int offsetMask;
    unsigned int instrFctnMask;
    unsigned int spectraMask;
    unsigned int darkCurrentMask;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
    char darkCurrentFile[FILENAME_BUFFER_LENGTH];
    char offsetFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_mfcstd {
    int  detectorSize;
    int  revert;
    int  straylight;
    char dateFormat[24];
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
    char darkCurrentFile[FILENAME_BUFFER_LENGTH];
    char offsetFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_minimum {
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_ccd {
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
    char interPixelVariabilityFile[FILENAME_BUFFER_LENGTH];
    char detectorNonLinearityFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_ccdulb {
    int grating;
    int centralWavelength;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char offsetFile[FILENAME_BUFFER_LENGTH];
    char interPixelVariabilityFile[FILENAME_BUFFER_LENGTH];
    char detectorNonLinearityFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_pdaeggulb {
    int curveType;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
    char interPixelVariabilityFile[FILENAME_BUFFER_LENGTH];
    char detectorNonLinearityFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_ccdeev {
    int  detectorSize;
    int  spectralType;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
    char straylightCorrectionFile[FILENAME_BUFFER_LENGTH];
    char detectorNonLinearityFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_opus {
    int detectorSize;
    double timeShift;
    int flagTransmittance;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_gdp {
    int bandType;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_scia {
    int channel;
    char clusters[32];                      // flags with cluster number as index
    char sunReference[4];                            // 2 characters plus terminator
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  struct instrumental_omi {
    int spectralType;
    double minimumWavelength;
    double maximumWavelength;
    int flagAverage;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];
  };

  typedef struct mediate_project_instrumental
   {
    int format;
    char siteName[SITE_NAME_BUFFER_LENGTH];
    struct instrumental_ascii ascii;
    struct instrumental_logger logger;
    struct instrumental_acton acton;
    struct instrumental_logger pdaegg;
    struct instrumental_logger pdaeggold;
    struct instrumental_ccd ccdohp96;
    struct instrumental_ccd ccdha94;
    struct instrumental_saoz saozvis;
    struct instrumental_minimum saozefm;
    struct instrumental_mfc mfc;
    struct instrumental_mfcstd mfcstd;
    struct instrumental_minimum rasas;
    struct instrumental_minimum pdasieasoe;
    struct instrumental_ccdeev ccdeev;
    struct instrumental_gdp gdpascii;
    struct instrumental_gdp gdpbin;
    struct instrumental_scia sciahdf;
    struct instrumental_scia sciapds;
    struct instrumental_minimum uoft;
    struct instrumental_minimum noaa;
    struct instrumental_omi omi;
    struct instrumental_gdp gome2;
    struct instrumental_minimum mkzy;
   }
  mediate_project_instrumental_t;


  /****************************************************/
  /* Project Slit */

  typedef struct mediate_project_slit
  {
    char solarRefFile[FILENAME_BUFFER_LENGTH];
    int applyFwhmCorrection;
    mediate_slit_function_t function;      /* the selected slit and properties for all slit types */
  } mediate_project_slit_t;


  /****************************************************/
  /* Project Output */

  typedef struct mediate_project_output
  {
    int analysisFlag;
    int calibrationFlag;
    int configurationFlag;
    int binaryFormatFlag;
    int directoryFlag;
    int filenameFlag;
    char flux[FLUX_BUFFER_LENGTH];
    char colourIndex[COLOUR_INDEX_BUFFER_LENGTH];
    char path[FILENAME_BUFFER_LENGTH];
    /* result field flags. A list of PRJCT_RESULTS_ASCII_*** ... */
    data_select_list_t selection;
  } mediate_project_output_t;

  /****************************************************/
  /* Project NASA AMES */
  /* TODO JUST REMOVE */

  typedef struct mediate_project_nasa_ames
  {
    int saveFlag;
    int rejectTestFlag;
    char anlysWinNO2[ANLYSWIN_NAME_BUFFER_LENGTH];
    char anlysWinO3[ANLYSWIN_NAME_BUFFER_LENGTH];
    char anlysWinBrO[ANLYSWIN_NAME_BUFFER_LENGTH];
    char anlysWinOClO[ANLYSWIN_NAME_BUFFER_LENGTH];
    char path[FILENAME_BUFFER_LENGTH];
    char instrument[NASA_AMES_VAR_BUFFER_LENGTH];
    char experiment[NASA_AMES_VAR_BUFFER_LENGTH];
  } mediate_project_nasa_ames_t;


  /* mediate_project_t
   *
   * Contains all user-specified information about a project. It allows the GUI to
   * provide information to the engine.
   */

  typedef struct mediate_project
  {
    /* Coupled to the control offered by the GUI Project Tabs. */
    mediate_project_spectra_t spectra;
    mediate_project_display_t display;
    mediate_project_selection_t selection;
    mediate_project_analysis_t analysis;
    mediate_filter_t lowpass;
    mediate_filter_t highpass;
    mediate_project_calibration_t calibration;
    mediate_project_undersampling_t undersampling;
    mediate_project_instrumental_t instrumental;
    mediate_project_slit_t slit;
    mediate_project_output_t output;
  } mediate_project_t;


  /****************************************************/
  /* Helper functions */

  void initializeMediateProject(mediate_project_t *d);
  void initializeMediateProjectDisplay(mediate_project_display_t *d);
  void initializeMediateProjectSelection(mediate_project_selection_t *d);
  void initializeMediateProjectAnalysis(mediate_project_analysis_t *d);
  void initializeMediateProjectFiltering(mediate_filter_t *d);
  void initializeMediateProjectCalibration(mediate_project_calibration_t *d);
  void initializeMediateProjectUndersampling(mediate_project_undersampling_t *d);
  void initializeMediateProjectInstrumental(mediate_project_instrumental_t *d);
  void initializeMediateProjectSlit(mediate_project_slit_t *d);
  void initializeMediateProjectOutput(mediate_project_output_t *d);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

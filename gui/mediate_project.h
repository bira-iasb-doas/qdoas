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

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif


#define FILENAME_BUFFER_LENGTH  256
#define SITE_NAME_BUFFER_LENGTH 128

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
  /* Project Analysis */
  
  static const int cProjAnalysisMethodModeOptDens     = 1;
  static const int cProjAnalysisMethodModeMarqLevSvd  = 2;
  static const int cProjAnalysisMethodModeMarqLevFull = 3;

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
  /* Project Filtering */
  
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

  typedef struct mediate_project_filtering
  {
    int mode;
    struct filter_kaiser kaiser;
    struct filter_boxcar boxcar;
    struct filter_gaussian gaussian;
    struct filter_triangular triangular;
    struct filter_savitzky_golay savitzky;
    struct filter_binomial binomial;
  } mediate_project_filtering_t;


  /****************************************************/
  /* Project Calibration */

  typedef struct mediate_project_calibration
  {
    char solarRefFile[FILENAME_BUFFER_LENGTH];
    int method;
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
    int spectralType;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];    
  };

  struct instrumental_rasas {
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];    
  };

  struct instrumental_ccd {
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];    
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
    int detectorSize;
    char calibrationFile[FILENAME_BUFFER_LENGTH];
    char instrFunctionFile[FILENAME_BUFFER_LENGTH];    
    char straylightCorrectionFile[FILENAME_BUFFER_LENGTH];
    char detectorNonLinearityFile[FILENAME_BUFFER_LENGTH];    
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
    struct instrumental_pdaeggulb pdaeggulb;
    struct instrumental_ccd ccdohp96;
    struct instrumental_ccd ccdha94;
    struct instrumental_ccd ccdulb;
    struct instrumental_saoz saozvis;
    struct instrumental_saoz saozuv;
    struct instrumental_rasas saozefm;
    struct instrumental_rasas rasas;
    struct instrumental_rasas pdasieasoe;
    struct instrumental_logger pdasiosma;
    struct instrumental_ccdeev ccdeev;
    struct instrumental_rasas uoft;
    struct instrumental_rasas noaa;
    
  } mediate_project_instrumental_t;



    
  // mediate_project_t
  //
  // Contains all user-specified information about a project. It allows the GUI to
  // provide information to the engine.
  
  typedef struct mediate_project
  {
    // TODO - Coupled to the control offered by the GUI Project Tabs.
    mediate_project_spectra_t spectra;
    mediate_project_analysis_t analysis;
    mediate_project_filtering_t lowpass;
    mediate_project_filtering_t highpass;
    mediate_project_calibration_t calibration;
    mediate_project_undersampling_t undersampling;
    mediate_project_instrumental_t instrumental;
    
  } mediate_project_t;
  

  /****************************************************/
  /* Helper functions */

  void initializeMediateProject(mediate_project_t *d);
  void initializeMediateProjectSpectra(mediate_project_spectra_t *d);
  void initializeMediateProjectAnalysis(mediate_project_analysis_t *d);
  void initializeMediateProjectFiltering(mediate_project_filtering_t *d);
  void initializeMediateProjectCalibration(mediate_project_calibration_t *d);
  void initializeMediateProjectUndersampling(mediate_project_undersampling_t *d);
  void initializeMediateProjectInstrumental(mediate_project_instrumental_t *d);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

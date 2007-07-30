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

  /****************************************************/
  /* Geolocation */

  static const int cGeolocationModeNone      = 0;
  static const int cGeolocationModeCircle    = 1;
  static const int cGeolocationModeRectangle = 2;

  struct geolocation_circle
  {
    int mode;        /* eGeolocationModeCircle */
    double radius;
    double centreLongitude;
    double centreLatitude;
  };
  
  struct geolocation_rectangle
  {
    int mode;        /* eGeolocationModeRectangle */
    double westernLongitude;
    double easternLongitude;
    double southernLatitude;
    double northernLatitude;
  };
  
  union geolocation
  {
    int mode;
    struct geolocation_circle circle;
    struct geolocation_rectangle rectangle;
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
    union geolocation geo;
    
  } mediate_project_spectra_t;


  /****************************************************/
  /* Project Analysis */
  
  static const int cProjAnalysisMethodModeOptDens     = 1;
  static const int cProjAnalysisMethodModeMarqLevSvd  = 2;
  static const int cProjAnalysisMethodModeMarqLevFull = 3;

  static const int cProjAnalysisFitModeNone         = 0;
  static const int cProjAnalysisFitModeInstrumental = 1;
  static const int cProjAnalysisFitModeStatistical  = 2;

  static const int cProjAnalysisUnitModePixel     = 1;
  static const int cProjAnalysisUnitModeNanometer = 2;

  static const int cProjAnalysisInterpolationModeLinear = 1;
  static const int cProjAnalysisInterpolationModeSpline = 2;


  typedef struct mediate_project_analysis
  {
    int methodType;
    int fitType;
    int unitType;
    int interpolationType;
  } mediate_project_analysis_t;
  
  
  
  // mediate_project_t
  //
  // Contains all user-specified information about a project. It allows the GUI to
  // provide information to the engine.
  
  typedef struct mediate_project
  {
    // To Be Defined - Coupled to the control offered by the GUI Project Tabs.
    mediate_project_spectra_t spectra;
    mediate_project_analysis_t analysis;
    
  } mediate_project_t;
  

  /****************************************************/
  /* Helper functions */

  void initializeMediateProject(mediate_project_t *d);
  void initializeMediateProjectSpectra(mediate_project_spectra_t *d);
  void initializeMediateProjectAnalysis(mediate_project_analysis_t *d);
  

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

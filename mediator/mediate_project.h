#ifndef _MEDIATE_PROJECT_H_GUARD
#define _MEDIATE_PROJECT_H_GUARD

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

  /****************************************************/
  /* Geolocation */

  const int cGeolocationModeNone      = 0;
  const int cGeolocationModeCircle    = 1;
  const int cGeolocationModeRectangle = 2;

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
  
  const int cProjAnalysisMethodModeOptDens     = 1;
  const int cProjAnalysisMethodModeMarqLevSvd  = 2;
  const int cProjAnalysisMethodModeMarqLevFull = 3;

  const int cProjAnalysisFitModeNone         = 0;
  const int cProjAnalysisFitModeInstrumental = 1;
  const int cProjAnalysisFitModeStatistical  = 2;

  const int cProjAnalysisUnitModePixel     = 1;
  const int cProjAnalysisUnitModeNanometer = 2;

  const int cProjAnalysisInterpolationModeLinear = 1;
  const int cProjAnalysisInterpolationModeSpline = 2;


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
  
#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
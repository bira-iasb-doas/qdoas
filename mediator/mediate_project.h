#ifndef _MEDIATE_PROJECT_H_GUARD
#define _MEDIATE_PROJECT_H_GUARD

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

struct geolocation_circle
{
  int type;        /* 1 */
  double radius;
  double centrelLongitude;
  double centreLatitude;
};

struct geolocation_rectangle
{
  int type;        /* 2 */
  double westernLongitude;
  double easternLongitude;
  double southernLatitude;
  double northernLatitude;
};

union geolocation
{
  int type;
  struct geolocation_circle circle;
  struct geolocation_rectangle rectangle;
};

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

// mediate_project_t
//
// Contains all user-specified information about a project. It allows the GUI to
// provide information to the engine.

typedef struct mediate_project
{
  // To Be Defined - Coupled to the control offered by the GUI Project Tabs.
  mediate_project_spectra_t spectra;

} mediate_project_t;

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

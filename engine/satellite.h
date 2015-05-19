#ifndef SATELLITE_H
#define SATELLITE_H

// Geolocations and angles for satellite measurements
typedef struct _satelliteGeoloc
 {
  // Geolocations

  double lonCorners[4],
         latCorners[4],
         lonCenter,
         latCenter;

  // Angles

  float  solZen[3],
         solAzi[3],
         losZen[3],
         losAzi[3];

  // Miscellaneous

  float earthRadius,satHeight;
  float cloudTopPressure,cloudFraction;                                         // information on clouds
  int   saaFlag;
  int   sunglintDangerFlag;
  int   sunglintHighDangerFlag;
  int   rainbowFlag;
  int   scanDirection;
} SATELLITE_GEOLOC;

#endif

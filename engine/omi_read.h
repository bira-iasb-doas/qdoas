#ifndef OMI_READ_H
#define OMI_READ_H

#include "doas.h"

void OMI_TrackSelection(const char *omiTrackSelection,int *omiTracks);
void OMI_ReleaseReference(void);
void OMI_ReleaseBuffers(void);
RC   OMI_GetReference(int spectralType, const char *refFile,INDEX indexColumn,double *lambda,double *ref,double *refSigma);
RC   OMI_Set(ENGINE_CONTEXT *pEngineContext);
RC   OMI_read_earth(ENGINE_CONTEXT *pEngineContext,int recordNo);
RC   OMI_load_analysis(ENGINE_CONTEXT *pEngineContext, void *responseHandle);
RC   OMI_get_orbit_date(int *year, int *month, int *day);
bool omi_use_track(int quality_flag, enum omi_xtrack_mode mode);
bool omi_has_automatic_reference(int row);

#endif

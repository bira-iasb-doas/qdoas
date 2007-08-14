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


#include "mediate_project.h"
#include "constants.h"

void initializeMediateProject(mediate_project_t *d)
{
  /* delegate to sub component initialization functions */

  initializeMediateProjectSpectra(&(d->spectra));
  initializeMediateProjectAnalysis(&(d->analysis));
  initializeMediateProjectFiltering(&(d->lowpass));
  initializeMediateProjectFiltering(&(d->highpass));
  initializeMediateProjectCalibration(&(d->calibration));
  initializeMediateProjectUndersampling(&(d->undersampling));
}

void initializeMediateProjectSpectra(mediate_project_spectra_t *d)
{
  /* no display */
  d->requireSpectra = 0;
  d->requireData    = 0;
  d->requireFits    = 0;
  
  /* full range SZA */ 
  d->szaMinimum =   0.0;
  d->szaMaximum = 180.0;
  d->szaDelta   =   0.0;
  
  /* zeros for no restrictions */
  d->recordNumberMinimum = 0;
  d->recordNumberMaximum = 0;
  
  /* false */
  d->useDarkFile = 0;
  d->useNameFile = 0;
  
  /* unrestricted geolocation */
  d->geo.mode = PRJCT_SPECTRA_MODES_NONE;

  /* zero the restricted modes */
  d->geo.circle.radius = 0.0;
  d->geo.circle.centerLongitude = 0.0;
  d->geo.circle.centerLatitude = 0.0;

  d->geo.rectangle.easternLongitude = 0.0;
  d->geo.rectangle.westernLongitude = 0.0;
  d->geo.rectangle.northernLatitude = 0.0;
  d->geo.rectangle.southernLatitude = 0.0;

  d->geo.sites.radius = 0.0;
}

void initializeMediateProjectAnalysis(mediate_project_analysis_t *d)
{
  d->methodType        = cProjAnalysisMethodModeOptDens;
  d->fitType           = PRJCT_ANLYS_FIT_WEIGHTING_NONE;
  d->unitType          = PRJCT_ANLYS_UNITS_PIXELS;
  d->interpolationType = PRJCT_ANLYS_INTERPOL_LINEAR;

  d->interpolationSecurityGap = 10;
  d->convergenceCriterion = 1.0e-4;
}

void initializeMediateProjectFiltering(mediate_project_filtering_t *d)
{
  d->mode = PRJCT_FILTER_TYPE_NONE;

  /* kaiser */
  d->kaiser.cutoffFrequency = 0.0;
  d->kaiser.tolerance       = 0.0;
  d->kaiser.passband        = 0.0;
  d->kaiser.iterations      = 0;

  /* boxcar */
  d->boxcar.width      = 0;
  d->boxcar.iterations = 0;

  /* gaussian */
  d->gaussian.fwhm       = 0.0;
  d->gaussian.iterations = 0;

  /* triangular*/
  d->triangular.width      = 0;
  d->triangular.iterations = 0;

  /* savitzky-golay */
  d->savitzky.width      = 0;
  d->savitzky.order      = 0;
  d->savitzky.iterations = 0;

  /* binomial */
  d->binomial.width      = 0;
  d->binomial.iterations = 0;

}

void initializeMediateProjectCalibration(mediate_project_calibration_t *d)
{
  *(d->solarRefFile) = '\0'; // empty filename
  d->method = 0;
  d->subWindows = 1;
  d->lineShape = 0;
  d->lorentzDegree = 0;
  d->shiftDegree = 0;
  d->sfpDegree = 0;
  d->wavelengthMin = 0.0;
  d->wavelengthMax = 0.0;
  d->requireSpectra = 0;
  d->requireFits = 0;
  d->requireResidual = 0;
  d->requireShiftSfp = 0;
}

void initializeMediateProjectUndersampling(mediate_project_undersampling_t *d)
{
  *(d->solarRefFile) = '\0'; // empty filename
  d->method = 0;
  d->shift = 0.0;
}

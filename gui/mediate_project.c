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

void initializeMediateProject(mediate_project_t *d)
{
  /* delegate to sub component initialization functions */

  initializeMediateProjectSpectra(&(d->spectra));
  initializeMediateProjectAnalysis(&(d->analysis));
  initializeMediateProjectFiltering(&(d->lowpass));
  initializeMediateProjectFiltering(&(d->highpass));
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
  d->geo.mode = cGeolocationModeNone;

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
  d->fitType           = cProjAnalysisFitModeNone;
  d->unitType          = cProjAnalysisUnitModePixel;
  d->interpolationType = cProjAnalysisInterpolationModeLinear;

  d->interpolationSecurityGap = 10;
  d->convergenceCriterion = 1.0e-4;
}

void initializeMediateProjectFiltering(mediate_project_filtering_t *d)
{
  d->mode = cProjFilteringModeNone;

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

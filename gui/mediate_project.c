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

#include <string.h>

#include "mediate_project.h"
#include "constants.h"

// ==========================================================
// INITIALIZATIONS OF NON-ZERO VALUES AND FLOAT/DOUBLE FIELDS
// ==========================================================

void initializeMediateProjectSpectra(mediate_project_spectra_t *d)
{
  memset(d, 0, sizeof(mediate_project_spectra_t));

  // any non-zero defaults...

  d->requireSpectra = 1;
  d->requireData = 1;
  d->requireFits = 1;
  d->szaMaximum = 96.0;
}

void initializeMediateProjectDisplay(mediate_project_display_t *d)
{
  memset(d, 0, sizeof(mediate_project_display_t));

  // any non-zero defaults...

  d->requireSpectra = 1;
  d->requireData = 1;
  d->requireFits = 1;
}

void initializeMediateProjectSelection(mediate_project_selection_t *d)
{
  memset(d, 0, sizeof(mediate_project_selection_t));

  // any non-zero defaults...

  d->szaMaximum = 96.0;
}

void initializeMediateProjectAnalysis(mediate_project_analysis_t *d)
{
  memset(d, 0, sizeof(mediate_project_analysis_t));

  // any non-zero defaults...

  d->interpolationSecurityGap = 10;
  d->convergenceCriterion = 1.0e-4;
}

void initializeMediateProjectFiltering(mediate_project_filtering_t *d)
{
  memset(d, 0, sizeof(mediate_project_filtering_t));

  // any non-zero defaults...
}

void initializeMediateProjectCalibration(mediate_project_calibration_t *d)
{
  memset(d, 0, sizeof(mediate_project_calibration_t));

  // any non-zero defaults...
}

void initializeMediateProjectUndersampling(mediate_project_undersampling_t *d)
{
  memset(d, 0, sizeof(mediate_project_undersampling_t));

  // any non-zero defaults...
}

void initializeMediateProjectInstrumental(mediate_project_instrumental_t *d)
{
  memset(d, 0, sizeof(mediate_project_instrumental_t));

  // any non-zero defaults...

  d->saozvis.spectralRegion=PRJCT_INSTR_SAOZ_REGION_VIS;
}

void initializeMediateProjectSlit(mediate_project_slit_t *d)
{
  memset(d, 0, sizeof(mediate_project_slit_t));

  // any non-zero defaults...
}

void initializeMediateProjectOutput(mediate_project_output_t *d)
{
  memset(d, 0, sizeof(mediate_project_output_t));

  // any non-zero defaults...
}

void initializeMediateProjectNasaAmes(mediate_project_nasa_ames_t *d)
{
  memset(d, 0, sizeof(mediate_project_nasa_ames_t));

  // any non-zero defaults...
}

void initializeMediateProject(mediate_project_t *d)
{
  /* delegate to sub component initialization functions */
  initializeMediateProjectSpectra(&(d->spectra)); // TODO - remove
  initializeMediateProjectDisplay(&(d->display));
  initializeMediateProjectSelection(&(d->selection));
  initializeMediateProjectAnalysis(&(d->analysis));
  initializeMediateProjectFiltering(&(d->lowpass));
  initializeMediateProjectFiltering(&(d->highpass));
  initializeMediateProjectCalibration(&(d->calibration));
  initializeMediateProjectUndersampling(&(d->undersampling));
  initializeMediateProjectInstrumental(&(d->instrumental));
  initializeMediateProjectSlit(&(d->slit));
  initializeMediateProjectOutput(&(d->output));
  initializeMediateProjectNasaAmes(&(d->nasaames));
}

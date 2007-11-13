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

#include "mediate_analysis_window.h"
#include "constants.h"

void initializeMediateAnalysisWindow(mediate_analysis_window_t *d)
 {
	 // Declarations

	 int i;

	 // Initializations

  memset(d, 0, sizeof(mediate_analysis_window_t));

  d->requireSpectrum=
  d->requirePolynomial=
  d->requireFit=
  d->requireResidual=
  d->requirePredefined=
  d->requireRefRatio=1;

  // Initialize non integer fields

  d->refSzaCenter=
  d->refSzaDelta=(double)0.;

  // Cross sections

  for (i=0;i<MAX_AW_CROSS_SECTION;i++)
   {
   	d->crossSectionList.crossSection[i].requireFit=1;
   	d->crossSectionList.crossSection[i].requireCcFit=1;
    d->crossSectionList.crossSection[i].initialCc=(double)0.;
    d->crossSectionList.crossSection[i].deltaCc=(double)1.e-3;
   }

  // Non-linear parameters

  d->nonlinear.solInitial=
  d->nonlinear.off0Initial=
  d->nonlinear.off1Initial=
  d->nonlinear.off2Initial=
  d->nonlinear.comInitial=
  d->nonlinear.usamp1Initial=
  d->nonlinear.usamp2Initial=
  d->nonlinear.ramanInitial=(double)0.;

  d->nonlinear.solDelta=
  d->nonlinear.off0Delta=
  d->nonlinear.off1Delta=
  d->nonlinear.off2Delta=
  d->nonlinear.comDelta=
  d->nonlinear.usamp1Delta=
  d->nonlinear.usamp2Delta=
  d->nonlinear.ramanDelta=(double)1.e-3;

  // Shift and stretch

  for (i=0;i<MAX_AW_SHIFT_STRETCH;i++)
   {
    d->shiftStretchList.shiftStretch[i].shInit=
    d->shiftStretchList.shiftStretch[i].stInit=
    d->shiftStretchList.shiftStretch[i].stInit2=
    d->shiftStretchList.shiftStretch[i].scInit=
    d->shiftStretchList.shiftStretch[i].scInit2=(double)0.;

    d->shiftStretchList.shiftStretch[i].shDelta=
    d->shiftStretchList.shiftStretch[i].stDelta=
    d->shiftStretchList.shiftStretch[i].stDelta2=
    d->shiftStretchList.shiftStretch[i].scDelta=
    d->shiftStretchList.shiftStretch[i].scDelta2=(double)1.e-3;

    d->shiftStretchList.shiftStretch[i].shMin=
    d->shiftStretchList.shiftStretch[i].shMax=(double)0.;
   }

  // Gaps

  for (i=0;i<MAX_AW_GAP;i++)
   d->gapList.gap[i].minimum=d->gapList.gap[i].maximum=(double)0.;

    gap_list_t gapList;

  // Output

  for (i=0;i<MAX_AW_CROSS_SECTION;i++)
   d->outputList.output[i].slantFactor=d->outputList.output[i].vertFactor=(double)1.;
 }

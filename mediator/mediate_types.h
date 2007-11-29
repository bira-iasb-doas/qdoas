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


#ifndef _MEDIATE_TYPES_H_GUARD
#define _MEDIATE_TYPES_H_GUARD

enum eEngineErrorType {
  NoEngineError,
  InformationEngineError,
  WarningEngineError,
  FatalEngineError
};

//* TODO - remove ePlotDataType */
enum ePlotDataType {
  PlotDataType_Spectrum,
  PlotDataType_Fit,
  PlotDataType_Shift,
  PlotDataType_Fwhm,
  PlotDataType_Points
};

enum eCurveStyleType {
  Line,
  Point
};

enum ePlotScaleType {
  Spectrum,
  SpecMax,
  Residual
};

// Plot scaling control

enum _spectraAutoscale
 {
 	allowFixedScale,                                                              // allow the scaling if checked in the plot properties
 	forceAutoScale                                                                // do not account for the scaling
 };

// Indexes for the plot pages

#define plotPageSpectrum        0                                               // spectra to browse or analyze
#define plotPageDarkCurrent     1                                               // dark currents
#define plotPageSpecMax         2                                               // specmax (variation of the signal with the scans)
#define plotPageErrors          3                                               // errors on measurements
#define plotPageIrrad           4                                               // irradiance spectra
#define plotPageRef             5                                               // ref1/ref2
#define plotPageCross           6                                               // cross sections
#define plotPageCalib          10                                               // kurucz
#define plotPageAnalysis       20                                               // analysis

#endif


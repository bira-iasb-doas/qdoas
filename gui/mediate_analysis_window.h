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


#ifndef _MEDIATE_ANALYSIS_WINDOW_H_GUARD
#define _MEDIATE_ANALYSIS_WINDOW_H_GUARD

#include "mediate_limits.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

  struct anlyswin_cross_section
  {
    char symbol[SYMBOL_NAME_BUFFER_LENGTH];
    char crossSectionFile[FILENAME_BUFFER_LENGTH];    // the cross section filename
    char orthogonal[SYMBOL_NAME_BUFFER_LENGTH];       // a symbol or predefined constant
    int crossType;
    int amfType;
    char amfFile[FILENAME_BUFFER_LENGTH];
    int requireFit;
    int requireFilter;
    int requireCcFit;
    double initialCc;
    double deltaCc;
    int requireCcIo;
  };

  struct anlys_linear
  {
    int xPolyOrder;
    int xBaseOrder;
    int xFlagFitStore;
    int xFlagErrStore;

    int xinvPolyOrder;
    int xinvBaseOrder;
    int xinvFlagFitStore;
    int xinvFlagErrStore;

    int offsetPolyOrder;
    int offsetBaseOrder;
    int offsetFlagFitStore;
    int offsetFlagErrStore;
  };

  struct anlyswin_shift_stretch
  {
    int nSymbol;
    char symbol[MAX_AW_SHIFT_STRETCH][SYMBOL_NAME_BUFFER_LENGTH];
    int shFit;
    int stFit;
    int scFit;
    int shStore;
    int stStore;
    int scStore;
    int errStore;
    int shInit;
    double stInit;
    double stInit2;
    double scInit;
    double scInit2;
    int shDelta;
    double stDelta;
    double stDelta2;
    double scDelta;
    double scDelta2;
    int shMin;
    int shMax;
  };

  // mediate_analysis_window_t
  //
  // Contains all user-specified information about a single spectral analysis window.
  // It allows the GUI to provide information to the engine.
  
  typedef struct mediate_analysis_window
  {
    int kuruczMode;
    int refSpectrumSelection;
    char refOneFile[FILENAME_BUFFER_LENGTH];
    char refTwoFile[FILENAME_BUFFER_LENGTH];
    char residualFile[FILENAME_BUFFER_LENGTH];
    double refSzaCenter;
    double refSzaDelta;
    int fitMinWavelength;
    int fitMaxWavelength;
    int requireSpectrum;
    int requirePolynomial;
    int requireFit;
    int requireResidual;
    int requirePredefined;
    int requireRefRatio;
    /* table data ... */
    int nCrossSection;      /* number of valid elements in the following array */
    struct anlyswin_cross_section crossSection[MAX_AW_CROSS_SECTION];

    struct anlys_linear linear;
    
    int nShiftStretch;
    struct anlyswin_shift_stretch shiftStretch[MAX_AW_SHIFT_STRETCH];

  } mediate_analysis_window_t;
  
  
  /****************************************************/
  /* Helper functions */
  
  void initializeMediateAnalysisWindow(mediate_analysis_window_t *d);
  
#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

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


#ifndef _MEDIATE_GENERAL_H_GUARD
#define _MEDIATE_GENERAL_H_GUARD

#include "mediate_limits.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

  // mediate_symbol_t
  //
  // Contains all information about a single symbol. It allows the GUI to provide
  // information to the engine.
  
  typedef struct mediate_symbol
  {
    char name[SYMBOL_NAME_BUFFER_LENGTH];
    char description[SYMBOL_DESCR_BUFFER_LENGTH];
  } mediate_symbol_t;
  
  
  // mediate_site_t
  //
  // Contains all information about a single site. It allows the GUI to provide
  // information to the engine.
  
  typedef struct mediate_site
  {
    char name[SITE_NAME_BUFFER_LENGTH];
    char abbreviation[SITE_ABBREV_BUFFER_LENGTH];
    double longitude;
    double latitude;
    double altitude;
  } mediate_site_t;
  
  
  // components of both mediate_analysis_window and mediate_project structs
  
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

  struct anlyswin_linear
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
    double shInit;
    double stInit;
    double stInit2;
    double scInit;
    double scInit2;
    double shDelta;
    double stDelta;
    double stDelta2;
    double scDelta;
    double scDelta2;
    double shMin;
    double shMax;
  };

  struct anlyswin_gap
  {
    double minimum;
    double maximum;
  };

  struct anlyswin_output
  {
    char symbol[SYMBOL_NAME_BUFFER_LENGTH];
    int amf;
    // residual
    int slantCol;
    int slantErr;
    double slantFactor;
    int vertCol;
    int vertErr;
    double vertFactor;
  };

  // struct array wrappers ...

  typedef struct cross_section_list
  {
    int nCrossSection;
    struct anlyswin_cross_section crossSection[MAX_AW_CROSS_SECTION];
  } cross_section_list_t;

  typedef struct shift_stretch_list
  {
    int nShiftStretch;
    struct anlyswin_shift_stretch shiftStretch[MAX_AW_SHIFT_STRETCH];
  } shift_stretch_list_t;

  typedef struct gap_list
  {
    int nGap;
    struct anlyswin_gap gap[MAX_AW_GAP];
  } gap_list_t;

  typedef struct output_list
  {
    int nOutput;
    struct anlyswin_output output[MAX_AW_CROSS_SECTION];
  } output_list_t;


#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

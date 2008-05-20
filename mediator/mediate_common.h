
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  Functions of the mediator that are common to the
//                       QDOAS application and tools
//  Name of module    :  mediate_common.h
//  Creation date     :  07/05/2008
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//  The QT version of the program has been developed jointly by the Belgian
//  Institute for Space Aeronomy (BIRA-IASB) and the Science and Technology
//  company (S[&]T) - Copyright (C) 2007
//
//      BIRA-IASB                                   S[&]T
//      Belgian Institute for Space Aeronomy        Science [&] Technology
//      Avenue Circulaire, 3                        Postbus 608
//      1180     UCCLE                              2600 AP Delft
//      BELGIUM                                     THE NETHERLANDS
//      caroline.fayt@aeronomie.be                  info@stcorp.nl
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//  ----------------------------------------------------------------------------

#include "mediate_limits.h"
#include "mediate_general.h"
#include "../engine/comdefs.h"
#include "../engine/constants.h"

#ifndef _MEDIATE_COMMON_
#define _MEDIATE_COMMON_

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// Description of the slit function

typedef struct _slit
 {
  INT    slitType;                                                              // type of line shape (see above)
  UCHAR  slitFile[MAX_STR_LEN+1];                                                // for line shapes provided in file, name of the file
  double slitParam;                                                             // up to 4 parameters can be provided for the line shape
  double slitParam2;                                                            //       usually, the first one is the FWHM
  double slitParam3;                                                            //       the Voigt profile function uses the 4 parameters
  double slitParam4;
 }
SLIT;

typedef struct _filter
 {
  INT     type;                                          // type of filter
  FLOAT   fwhmWidth;                                     // fwhm width for gaussian
  FLOAT   kaiserCutoff;                                  // cutoff frequency for kaiser filter type
  FLOAT   kaiserPassBand;                                // pass band for kaiser filter type
  FLOAT   kaiserTolerance;                               // tolerance for kaiser filter type
  INT     filterOrder;                                   // filter order
  INT     filterWidth;                                   // filter width for boxcar, triangle or Savitsky-Golay filters
  INT     filterNTimes;                                  // the number of times to apply the filter
  INT     filterAction;
  double *filterFunction;
  INT     filterSize;
  double  filterEffWidth;
  INT     hpFilterCalib;
  INT     hpFilterAnalysis;
 }
PRJCT_FILTER;

void setMediateSlit(SLIT *pEngineSlit,const mediate_slit_function_t *pMediateSlit);
void setMediateFilter(PRJCT_FILTER *pEngineFilter,const mediate_filter_t *pMediateFilter);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

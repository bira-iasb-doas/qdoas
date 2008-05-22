
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  Functions of the engine that are common to the
//                       QDOAS application and tools
//  Name of module    :  engine_common.h
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

#ifndef _ENGINE_COMMON_
#define _ENGINE_COMMON_

#include "comdefs.h"

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

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

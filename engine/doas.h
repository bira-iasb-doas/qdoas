
//
//  Product/Project   :  QDOAS
//  Name of module    :  DOAS.H
//  Creation date     :  1997
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
//
//  MODULE DESCRIPTION
//
//  The present module contains definitions used by most of the DOAS routines
//
//  ----------------------------------------------------------------------------

/*! \file doas.h main header file */

#ifndef DOAS_H
#define DOAS_H

#include "output_formats.h"
#include "comdefs.h"
#include "constants.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// =====================
// CONSTANTS DEFINITIONS
// =====================

#define MAX_FEN                20    // maximum number of gaps in a analysis window
#define MAX_FIT                50    // maximum number of parameters to fit
#define MAX_FENO               25    // maximum number of analysis windows in a project
#define MAX_SYMB              150    // maximum number of different symbols in a project
#define MAX_SPECMAX          5000    // maximum number of items in SpecMax

#define DIM                    10    // default number of security pixels for border effects

#define MAX_KURUCZ_FWHM_PARAM   4    // maximum number of non linear parameters for fitting fwhm with Kurucz

#define STOP      ITEM_NONE

#define DOAS_PI     (double) 3.14159265358979323846
#define PI2         (double) 6.28318530717958647692
#define PIDEMI      (double) 1.57079632679489661923

typedef struct _prjctAnlys PRJCT_ANLYS;
typedef struct _prjctKurucz PRJCT_KURUCZ;
typedef struct _prjctUsamp PRJCT_USAMP;
typedef struct _prjctSlit PRJCT_SLIT;

typedef struct _prjctAsciiResults PRJCT_RESULTS;

typedef struct _matrix MATRIX_OBJECT;

typedef struct _FFT FFT;

typedef struct _feno FENO;
typedef struct _KuruczFeno KURUCZ_FENO;
typedef struct _Kurucz KURUCZ;

typedef struct _wrkSymbol WRK_SYMBOL;
typedef struct _crossResults CROSS_RESULTS;

typedef struct _ccd CCD;
typedef struct _engineContext ENGINE_CONTEXT;

// ----------------
// GLOBAL VARIABLES
// ----------------
extern int NWorkSpace,NDET;
extern int           DimC,DimL,DimP,Z,NFeno,SvdPDeb,SvdPFin;
extern WRK_SYMBOL   *WorkSpace;

extern PRJCT_ANLYS  *pAnalysisOptions;             // analysis options
extern PRJCT_KURUCZ *pKuruczOptions;               // Kurucz options
extern PRJCT_SLIT   *pSlitOptions;                 // slit function options
extern PRJCT_USAMP  *pUsamp;

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

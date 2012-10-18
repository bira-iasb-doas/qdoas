

//  ----------------------------------------------------------------------------
//
//  Product/Project   :  THE BIRA-IASB DOAS SOFTWARE FOR WINDOWS AND LINUX
//  Module purpose    :  CONSTANTS DEFINITIONS COMMON TO BOTH ENGINE AND GUI
//  Name of module    :  CONSTANTS.H
//  Creation date     :  14 August 2007
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
//  This module contains all the definitions that are common to both the user
//  interface and the engine
//
//  ----------------------------------------------------------------------------

#if !defined(__CONSTANTS_)
#define __CONSTANTS_

// =====================
// CONSTANTS DEFINITIONS
// =====================

// Indexes

#define NO_SELECTED_FILE    (INDEX) -2
#define ORTHOGONAL_BASE     (INDEX) -3

// Standard arrays dimensions

#define BUFFER_SIZE   4096                                                      // any buffer allocation
#define STRING_LENGTH 1023                                                      // maximum size for strings


// ================
// OPERATING MODES
// ================

enum _thrdId
  {
    THREAD_TYPE_NONE,
    THREAD_TYPE_SPECTRA,     // for browse spectra
    THREAD_TYPE_ANALYSIS,    // for run analysis
    THREAD_TYPE_KURUCZ       // for run calibration
  };


// ================
// FILES PROCESSING
// ================

// ------------------------------------
// CONSTANTS AND STRUCTURES DEFINITIONS
// ------------------------------------

#define FILES_PATH_MAX 100

// Common dialog open mode
// -----------------------

enum _filesOpenMode
 {
  FILE_MODE_OPEN,
  FILE_MODE_SAVE
 };

// All supported types of files
// ----------------------------

enum _filesTypes
 {
  FILE_TYPE_ALL,                                  // all files (*.*)
  FILE_TYPE_ASCII,                                // ASCII files (*.asc)
  FILE_TYPE_ASCII_SPECTRA,                        // ASCII spectra files (*.asc)
  FILE_TYPE_SPECTRA,                              // spectra files (*.spe)
  FILE_TYPE_CROSS,                                // cross sections files (*.xs)
  FILE_TYPE_REF,                                  // reference spectra files (*.ref)
  FILE_TYPE_AMF_SZA,                              // SZA dependent AMF files (*.amf_sza)
  FILE_TYPE_AMF_CLI,                              // Climatology dependent AMF files (*.amf_cli)
  FILE_TYPE_AMF_WVE,                              // Wavelength dependent AMF files (*.amf_wve)
  FILE_TYPE_NAMES,                                // spectra names files (*.nms)
  FILE_TYPE_DARK,                                 // dark current files (*.drk)
  FILE_TYPE_INTERPIXEL,                           // interpixel variability files (*.vip)
  FILE_TYPE_NOT_LINEARITY,                        // detector not linearity files (*.dnl)
  FILE_TYPE_CALIB,                                // calibration files (*.clb)
  FILE_TYPE_CALIB_KURUCZ,                         // Kurucz files (*.ktz)
  FILE_TYPE_SLIT,                                 // slit function (*.slf)
  FILE_TYPE_INSTR,                                // instrumental function (*.ins)
  FILE_TYPE_FILTER,                               // filter files (*.flt)
  FILE_TYPE_FIT,                                  // fits files (*.fit*)
  FILE_TYPE_INI,                                  // WINDOAS settings (*.wds)
  FILE_TYPE_BMP,                                  // bitmap files (*.bmp)
  FILE_TYPE_RES,                                  // residuals (*.res)
  FILE_TYPE_PATH,                                 // paths
  FILE_TYPE_CFG,                                  // config
  FILE_TYPE_MAX
 };

// Individual spectra safe keeping
// -------------------------------

enum _filesTypesSpectra
 {
  FILE_TYPE_SPECTRA_COMMENT,
  FILE_TYPE_SPECTRA_NOCOMMENT,
  FILE_TYPE_SPECTRA_MAX
 };

// =================
// OBSERVATION SITES
// =================

   #define MAX_SITES              60
   #define MAX_SITES_ABBREVIATION  2

// =======
// SYMBOLS
// =======

   // Definitions
   // -----------

   #define MAX_SYMBOL_CROSS         60                                          // default maximum number of cross sections symbols

   #define SYMBOL_ALREADY_USED    0x01                                          // mask set if a symbol is used or referenced

   enum _predefinedSymbols
    {
     SYMBOL_PREDEFINED_SPECTRUM,                                                // spectrum
     SYMBOL_PREDEFINED_REF,                                                     // reference
     SYMBOL_PREDEFINED_COM,                                                     // common residual
     SYMBOL_PREDEFINED_USAMP1,                                                  // undersampling phase 1
     SYMBOL_PREDEFINED_USAMP2,                                                  // undersampling phase 2
     SYMBOL_PREDEFINED_RING1,                                                   // different cross sections for ring
     SYMBOL_PREDEFINED_MAX
    };

// ============================
// RAW SPECTRA FILES PROPERTIES
// ============================

   // ------------------------------------
   // CONSTANTS AND STRUCTURES DEFINITIONS
   // ------------------------------------

   #define MAX_RAW_SPECTRA 1000

   enum _folderFlagType
    {
     RAW_FOLDER_TYPE_NONE,           // file type
     RAW_FOLDER_TYPE_USER,           // user folder type
     RAW_FOLDER_TYPE_PATH,           // directory folder type
     RAW_FOLDER_TYPE_PATHSUB         // directory folder type, include subfolders
    };


// ===========================
// ANALYSIS WINDOWS PROPERTIES
// ===========================

// Analysis windows
// ----------------

// -----------
// DEFINITIONS
// -----------

// Description of columns in ListView control
// ------------------------------------------

#define MAX_LIST_COLUMNS 30

#define COMBOBOX_ITEM_TYPE_NONE           0x0000           // not a combobox control
#define COMBOBOX_ITEM_TYPE_EXCLUSIVE      0x0001           // combobox to fill with symbols that are not used yet
#define COMBOBOX_ITEM_TYPE_ORTHOGONAL     0x0002           // combobox to fill with all available symbols except the one selected
#define COMBOBOX_ITEM_TYPE_POLYNOME       0x0008           // polynomial modes
#define COMBOBOX_ITEM_TYPE_AMF            0x0010           // types of AMF files
#define COMBOBOX_ITEM_TYPE_XS             0x0020           // types of AMF files

enum _orthogonalTypes
 {
  ANLYS_ORTHOGONAL_TYPE_NONE,
  ANLYS_ORTHOGONAL_TYPE_BASE,
  ANLYS_ORTHOGONAL_TYPE_MAX
 };

enum _polynomeTypes
 {
 	ANLYS_POLY_TYPE_NONE,
 	ANLYS_POLY_TYPE_0,
 	ANLYS_POLY_TYPE_1,
 	ANLYS_POLY_TYPE_2,
 	ANLYS_POLY_TYPE_3,
 	ANLYS_POLY_TYPE_4,
 	ANLYS_POLY_TYPE_5,
 	ANLYS_POLY_TYPE_MAX
 };

enum _stretchTypes
 {
  ANLYS_STRETCH_TYPE_NONE,
  ANLYS_STRETCH_TYPE_FIRST_ORDER,
  ANLYS_STRETCH_TYPE_SECOND_ORDER,
  ANLYS_STRETCH_TYPE_MAX
 };

enum _crossTypes
 {
  ANLYS_CROSS_ACTION_NOTHING,
  ANLYS_CROSS_ACTION_INTERPOLATE,
  ANLYS_CROSS_ACTION_CONVOLUTE,
  ANLYS_CROSS_ACTION_CONVOLUTE_I0,
  ANLYS_CROSS_ACTION_CONVOLUTE_RING,
  ANLYS_CROSS_ACTION_MAX
 };

enum _amfTypes
 {
  ANLYS_AMF_TYPE_NONE,
  ANLYS_AMF_TYPE_SZA,
  ANLYS_AMF_TYPE_CLIMATOLOGY,
  ANLYS_AMF_TYPE_WAVELENGTH,
  ANLYS_AMF_TYPE_MAX
 };

// Columns enumeration per tab page
// --------------------------------

// Columns description of the ListView in "cross sections" tab page

enum _crossSectionsColumns
 {
  // Cross Sections

  COLUMN_CROSS_FILE,              // Cross Sections
  COLUMN_CROSS_ORTHOGONAL,        // Orthogonalization
  COLUMN_CROSS_XS_TYPE,           // Cross section type
  COLUMN_CROSS_AMF_TYPE,          // AMF dependence to use
  COLUMN_CROSS_DISPLAY,           // Fit display
  COLUMN_CROSS_FILTER,            // Filter flag
  COLUMN_CROSS_CCFIT,             // Cc fit
  COLUMN_CROSS_CCINIT,            // Cc init
  COLUMN_CROSS_CCDELTA,           // Cc delta

  // Output

  COLUMN_CROSS_AMF_OUTPUT,        // Store AMF
  COLUMN_CROSS_RESIDUAL,          // Res Col
  COLUMN_CROSS_SLNTCOL,           // Slnt Col
  COLUMN_CROSS_SLNTERR,           // Slnt Err
  COLUMN_CROSS_SLNTFACT,          // Slnt Fact
  COLUMN_CROSS_VRTCOL,            // Vrt Col
  COLUMN_CROSS_VRTERR,            // Vrt Err
  COLUMN_CROSS_VRTFACT,           // Vrt Fact

  // Kurucz SVD matrix composition for reference alignment

  COLUMN_CROSS_SVD_ORTHOGONAL,    // Orthogonalization
  COLUMN_CROSS_SVD_DISPLAY,       // Fit display
  COLUMN_CROSS_SVD_XS_TYPE,       // Cross section type
  COLUMN_CROSS_SVD_CCFIT,         // Cc fit
  COLUMN_CROSS_SVD_CCINIT,        // Cc init
  COLUMN_CROSS_SVD_CCDELTA,       // Cc delta

  // Kurucz output

  COLUMN_CROSS_SVD_SLNTCOL,       // Slnt Col
  COLUMN_CROSS_SVD_SLNTERR,       // Slnt Err
  COLUMN_CROSS_SVD_SLNTFACT,      // Slnt Fact

  // I0 convolution

  COLUMN_CROSS_CCI0,              // Cc I0 convolution
  COLUMN_CROSS_SVD_CCI0,          // Cc I0 convolution (calibration)

  COLUMN_CROSS_MAX
 };

// Columns description of the ListView in "continuous functions" tab page

enum _continuousFunctionsColumns
 {
  COLUMN_CONTINUOUS_X0,           // terms in x0 and 1/x0
  COLUMN_CONTINUOUS_X1,           // terms in x1 and 1/x1
  COLUMN_CONTINUOUS_X2,           // terms in x2 and 1/x2
  COLUMN_CONTINUOUS_X3,           // terms in x3 and 1/x3
  COLUMN_CONTINUOUS_X4,           // terms in x4 and 1/x4
  COLUMN_CONTINUOUS_X5,           // terms in x5 and 1/x5
  COLUMN_CONTINUOUS_MAX,          // terms in x0 and 1/x0
 };

// Columns description of the ListView in "linear parameters" tab page

enum _linearColumns
 {
  COLUMN_POLY_PARAMS,        // Linear params
  COLUMN_POLY_POLYORDER,     // Polynomial order
  COLUMN_POLY_BASEORDER,     // Orthogonal base order
  COLUMN_POLY_STORE_FIT,     // Store fit
  COLUMN_POLY_STORE_ERROR,   // Store error
  COLUMN_POLY_MAX
 };

// Columns description of the ListView in "non linear parameters" tab page

enum _notLinearColumns
 {
  COLUMN_OTHERS_PARAMS,        // Non linear params
  COLUMN_OTHERS_FIT,           // Fit flag
  COLUMN_OTHERS_VALINIT,       // Val init
  COLUMN_OTHERS_VALDELTA,      // Val del
  COLUMN_OTHERS_STORE_FIT,     // Store fit
  COLUMN_OTHERS_STORE_ERROR,   // Store error
  COLUMN_OTHERS_VALMIN,        // Val min
  COLUMN_OTHERS_VALMAX,        // Val max
  COLUMN_OTHERS_MAX
 };

// Columns description of the ListView in "Shift and Stretch" tab page

enum _shiftStretchColumns
 {
  COLUMN_CROSS_TO_FIT,            // Cross Sections and spectrum
  COLUMN_SH_FIT,                  // Sh fit
  COLUMN_ST_FIT,                  // St fit
  COLUMN_SC_FIT,                  // Sc fit
  COLUMN_SH_STORE,                // Sh store
  COLUMN_ST_STORE,                // St store
  COLUMN_SC_STORE,                // Sc store
  COLUMN_ERR_STORE,               // Err store
  COLUMN_SH_INIT,                 // Sh init
  COLUMN_ST_INIT,                 // St init
  COLUMN_ST_INIT2,                // St init
  COLUMN_SC_INIT,                 // St init
  COLUMN_SC_INIT2,                // St init
  COLUMN_SH_DELTA,                // Sh delta
  COLUMN_ST_DELTA,                // St delta
  COLUMN_ST_DELTA2,               // St delta
  COLUMN_SC_DELTA,                // St delta
  COLUMN_SC_DELTA2,               // St delta
  COLUMN_SH_MIN,                  // Sh min
  COLUMN_SH_MAX,                  // Sh max
  COLUMN_SHST_MAX
 };

// Columns description of the ListView in "gaps" tab page

enum _gapsColumns
 {
  COLUMN_GAPS_TEXT,
  COLUMN_GAPS_MINVAL,             // Min Value
  COLUMN_GAPS_MAXVAL,             // Max Value
  COLUMN_GAPS_MAX
 };

// Analysis tab pages description
// ------------------------------

enum anlysTabPagesTypes
 {
  TAB_TYPE_ANLYS_CROSS,
  TAB_TYPE_ANLYS_LINEAR,
  TAB_TYPE_ANLYS_NOTLINEAR,
  TAB_TYPE_ANLYS_SHIFT_AND_STRETCH,
  TAB_TYPE_ANLYS_GAPS,
  TAB_TYPE_ANLYS_OUTPUT,
  TAB_TYPE_ANLYS_MAX
 };

#define MAX_ANLYS 50

enum _nonLinearParameters
 {
 	ANLYS_NL_SOL,
 	ANLYS_NL_OFF0,
 	ANLYS_NL_OFF1,
 	ANLYS_NL_OFF2,
 	ANLYS_NL_COM,
 	ANLYS_NL_USAMP1,
 	ANLYS_NL_USAMP2,
 	ANLYS_NL_RAMAN,
 	ANLYS_NL_MAX
 };

enum _refSpectrumSelectionMode
 {
  ANLYS_REF_SELECTION_MODE_AUTOMATIC,                      // automatic selection from spectra files
  ANLYS_REF_SELECTION_MODE_FILE                            // reference spectrum in a specific file
 };

enum _maxdoasRefSelectionMode
 {
  ANLYS_MAXDOAS_REF_SZA,
  ANLYS_MAXDOAS_REF_SCAN
 };

enum _kuruczMode
 {
  ANLYS_KURUCZ_NONE,
  ANLYS_KURUCZ_REF,
  ANLYS_KURUCZ_SPEC,
  ANLYS_KURUCZ_REF_AND_SPEC,
  ANLYS_KURUCZ_MAX
 };

// ===================
// PROJECTS PROPERTIES
// ===================

// ----------------
// SPECTRA TAB PAGE
// ----------------

// Spectra tab page description
// ----------------------------

enum _prjctSpectraModes
 {
  PRJCT_SPECTRA_MODES_NONE,
  PRJCT_SPECTRA_MODES_CIRCLE,
  PRJCT_SPECTRA_MODES_RECTANGLE,
  PRJCT_SPECTRA_MODES_OBSLIST,
  PRJCT_SPECTRA_MODES_MAX
 };

// -----------------
// ANALYSIS TAB PAGE
// -----------------

// Analysis methods
// ----------------

enum prjctAnlysMethod
 {
  PRJCT_ANLYS_METHOD_SVD,                              // Optical thickness fitting (SVD)
  PRJCT_ANLYS_METHOD_SVDMARQUARDT,                     // Intensity fitting (Marquardt-Levenberg+SVD)
  PRJCT_ANLYS_METHOD_MAX
 };

// Least-squares fit weighting
// ---------------------------

enum prjctAnlysFitWeighting
 {
  PRJCT_ANLYS_FIT_WEIGHTING_NONE,                      // no weighting
  PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL,              // instrumental weighting
  PRJCT_ANLYS_FIT_WEIGHTING_MAX
 };

// Units for shift and stretch
// ---------------------------

enum prjctAnlysUnits
 {
  PRJCT_ANLYS_UNITS_PIXELS,                            // pixels
  PRJCT_ANLYS_UNITS_NANOMETERS,                        // nanometers
  PRJCT_ANLYS_UNITS_MAX
 };

// Interpolation
// -------------

enum prjctAnlysInterpol
 {
  PRJCT_ANLYS_INTERPOL_LINEAR,                         // linear interpolation
  PRJCT_ANLYS_INTERPOL_SPLINE,                         // spline interpolation
  PRJCT_ANLYS_INTERPOL_MAX
 };

// ---------------
// FILTER TAB PAGE
// ---------------

// Filter types
// ------------

enum prjctFilterTypes
 {
  PRJCT_FILTER_TYPE_NONE,                              // use no filter
  PRJCT_FILTER_TYPE_KAISER,                            // kaiser filter
  PRJCT_FILTER_TYPE_BOXCAR,                            // box car filter
  PRJCT_FILTER_TYPE_GAUSSIAN,                          // gaussian filter
  PRJCT_FILTER_TYPE_TRIANGLE,                          // triangular filter
  PRJCT_FILTER_TYPE_SG,                                // savitzky-Golay filter
  PRJCT_FILTER_TYPE_ODDEVEN,                           // odd-even pixel correction
  PRJCT_FILTER_TYPE_BINOMIAL,                          // binomial filter
  PRJCT_FILTER_TYPE_MAX
 };

// Filter action
// -------------

enum prjctFilterOutput
 {
  PRJCT_FILTER_OUTPUT_LOW,
  PRJCT_FILTER_OUTPUT_HIGH_SUB,
  PRJCT_FILTER_OUTPUT_HIGH_DIV,
  PRJCT_FILTER_OUTPUT_MAX
 };

enum prjctFilterFields
 {
  PRJCT_FILTER_FIELD_TYPE,
  PRJCT_FILTER_FIELD_CUTOFF_TEXT,
  PRJCT_FILTER_FIELD_CUTOFF,
  PRJCT_FILTER_FIELD_PASS_TEXT,
  PRJCT_FILTER_FIELD_PASS,
  PRJCT_FILTER_FIELD_TOLERANCE_TEXT,
  PRJCT_FILTER_FIELD_TOLERANCE,
  PRJCT_FILTER_FIELD_FWHM_TEXT,
  PRJCT_FILTER_FIELD_FWHM,
  PRJCT_FILTER_FIELD_WIDTH,
  PRJCT_FILTER_FIELD_PIXELS,
  PRJCT_FILTER_FIELD_ORDER_TEXT,
  PRJCT_FILTER_FIELD_ORDER,
  PRJCT_FILTER_FIELD_ORDER_EVEN,
  PRJCT_FILTER_FIELD_NITER_TEXT,
  PRJCT_FILTER_FIELD_NITER
 };

// --------------------
// CALIBRATION TAB PAGE
// --------------------

// Calibration tab page description
// --------------------------------

#define NSFP 2

enum _prjctFwhmTypes
 {
  PRJCT_CALIB_FWHM_TYPE_NONE,
  PRJCT_CALIB_FWHM_TYPE_FILE,
  PRJCT_CALIB_FWHM_TYPE_GAUSS,
  PRJCT_CALIB_FWHM_TYPE_ERF,
  PRJCT_CALIB_FWHM_TYPE_INVPOLY,
  PRJCT_CALIB_FWHM_TYPE_VOIGT,
  PRJCT_CALIB_FWHM_TYPE_AGAUSS,
  PRJCT_CALIB_FWHM_TYPE_MAX
 };

// ----------------------
// UNDERSAMPLING TAB PAGE
// ----------------------

enum _prjctUsampMethod
 {
  PRJCT_USAMP_FILE,                                    // no undersampling fitting
  PRJCT_USAMP_FIXED,                                   // undersampling fitting, fixed phase
  PRJCT_USAMP_AUTOMATIC,                               // undersampling fitting, automatic phase
  PRJCT_USAMP_MAX
 };

// ---------------------
// INSTRUMENTAL TAB PAGE
// ---------------------

// type of read out format
// -----------------------

enum _prjctInstrFormat
 {
  PRJCT_INSTR_FORMAT_ASCII,                                                     // ASCII
  PRJCT_INSTR_FORMAT_LOGGER,                                                    // Logger (PDA,CCD or HAMAMATSU)
  PRJCT_INSTR_FORMAT_ACTON,                                                     // Acton (NILU)
  PRJCT_INSTR_FORMAT_PDAEGG,                                                    // PDA EG&G (sept. 94 until now)
  PRJCT_INSTR_FORMAT_PDAEGG_OLD,                                                // PDA EG&G (spring 94)
  PRJCT_INSTR_FORMAT_PDAEGG_ULB,                                                // PDA EG&G (ULB)
  PRJCT_INSTR_FORMAT_CCD_OHP_96,                                                // CCD (OHP 96)
  PRJCT_INSTR_FORMAT_CCD_HA_94,                                                 // CCD (HARESTUA 94)
  PRJCT_INSTR_FORMAT_CCD_ULB,                                                   // CCD (ULB)
  PRJCT_INSTR_FORMAT_SAOZ_VIS,                                                  // SAOZ visible
  PRJCT_INSTR_FORMAT_SAOZ_UV,                                                   // SAOZ UV
  PRJCT_INSTR_FORMAT_SAOZ_EFM,                                                  // SAOZ EFM (1024)
  PRJCT_INSTR_FORMAT_MFC,                                                       // MFC Heidelberg
  PRJCT_INSTR_FORMAT_MFC_STD,                                                   // MFC Heidelberg
  PRJCT_INSTR_FORMAT_MFC_BIRA,                                                  // MFC BIRA-IASB
  PRJCT_INSTR_FORMAT_RASAS,                                                     // RASAS (INTA)
  PRJCT_INSTR_FORMAT_PDASI_EASOE,                                               // EASOE
  PRJCT_INSTR_FORMAT_PDASI_OSMA,                                                // PDA SI (OSMA)
  PRJCT_INSTR_FORMAT_CCD_EEV,                                                   // CCD EEV
  PRJCT_INSTR_FORMAT_OPUS,                                                      // FOURIER,OPUS format
  PRJCT_INSTR_FORMAT_GDP_ASCII,                                                 // GOME GDP ASCII format
  PRJCT_INSTR_FORMAT_GDP_BIN,                                                   // GOME GDP BINARY format
  PRJCT_INSTR_FORMAT_SCIA_HDF,                                                  // SCIAMACHY Calibrated Level 1 data in HDF format
  PRJCT_INSTR_FORMAT_SCIA_PDS,                                                  // SCIAMACHY Calibrated Level 1 data in PDS format
  PRJCT_INSTR_FORMAT_UOFT,                                                      // University of Toronto
  PRJCT_INSTR_FORMAT_NOAA,                                                      // NOAA
  PRJCT_INSTR_FORMAT_OMI,                                                       // OMI
  PRJCT_INSTR_FORMAT_GOME2,                                                     // GOME2
  PRJCT_INSTR_FORMAT_MKZY,                                                      // MANNE Kihlman and ZHANG Yan pak format
  PRJCT_INSTR_FORMAT_BIRA_AIRBORNE,                                             // MOBILEDOAS and ULMDOAS (BIRA-IASB)
  PRJCT_INSTR_FORMAT_OCEAN_OPTICS,                                              // Ocean optics
  PRJCT_INSTR_FORMAT_MAX
 };

enum _prjctInstrType
  {
    PRJCT_INSTR_TYPE_GROUND_BASED,
    PRJCT_INSTR_TYPE_SATELLITE,
    PRJCT_INSTR_TYPE_MAX
  };

enum _eevSpectrumTypes
 {
 	PRJCT_INSTR_EEV_TYPE_NONE,
 	PRJCT_INSTR_EEV_TYPE_OFFAXIS,
 	PRJCT_INSTR_EEV_TYPE_DIRECTSUN,
 	PRJCT_INSTR_EEV_TYPE_ZENITH,
 	PRJCT_INSTR_EEV_TYPE_DARK,
 	PRJCT_INSTR_EEV_TYPE_LAMP,
 	PRJCT_INSTR_EEV_TYPE_BENTHAM,
 	PRJCT_INSTR_EEV_TYPE_ALMUCANTAR,
 	PRJCT_INSTR_EEV_TYPE_OFFSET,
 	PRJCT_INSTR_EEV_TYPE_AZIMUTH,
 	PRJCT_INSTR_EEV_TYPE_PRINCIPALPLANE,
 	PRJCT_INSTR_EEV_TYPE_MAX
 };

 enum _mfcbiraSpectrumTypes
  {
  	PRJCT_INSTR_MFC_TYPE_UNKNOWN,
  	PRJCT_INSTR_MFC_TYPE_MEASUREMENT,
  	PRJCT_INSTR_MFC_TYPE_OFFSET,
  	PRJCT_INSTR_MFC_TYPE_DARK,
  	PRJCT_INSTR_MFC_TYPE_MAX
  };

enum _ulbCurveTypes
 {
  PRJCT_INSTR_ULB_TYPE_MANUAL,
  PRJCT_INSTR_ULB_TYPE_HIGH,
  PRJCT_INSTR_ULB_TYPE_LOW,
  PRJCT_INSTR_ULB_TYPE_MAX
 };

enum _saozSpectrumRegion
 {
 	PRJCT_INSTR_SAOZ_REGION_UV,
 	PRJCT_INSTR_SAOZ_REGION_VIS
 };

enum _saozSpectrumTypes
 {
  PRJCT_INSTR_SAOZ_TYPE_ZENITHAL,
  PRJCT_INSTR_SAOZ_TYPE_POINTED,
  PRJCT_INSTR_SAOZ_TYPE_MAX
 };

enum _iasbSpectrumTypes
 {
  PRJCT_INSTR_IASB_TYPE_ALL,
  PRJCT_INSTR_IASB_TYPE_ZENITHAL,
  PRJCT_INSTR_IASB_TYPE_OFFAXIS,
  PRJCT_INSTR_IASB_TYPE_MAX
 };

enum _niluFormatTypes
 {
  PRJCT_INSTR_NILU_FORMAT_OLD,
  PRJCT_INSTR_NILU_FORMAT_NEW,
  PRJCT_INSTR_NILU_FORMAT_MAX
 };

enum _asciiFormat
 {
  PRJCT_INSTR_ASCII_FORMAT_LINE,
  PRJCT_INSTR_ASCII_FORMAT_COLUMN
 };

enum _gdpBandTypes
 {
  PRJCT_INSTR_GDP_BAND_1A,
  PRJCT_INSTR_GDP_BAND_1B,
  PRJCT_INSTR_GDP_BAND_2A,
  PRJCT_INSTR_GDP_BAND_2B,
  PRJCT_INSTR_GDP_BAND_3,
  PRJCT_INSTR_GDP_BAND_4,
  PRJCT_INSTR_GDP_BAND_MAX
 };

enum _gdpPixelTypes
 {
 	PRJCT_INSTR_GDP_PIXEL_ALL,
 	PRJCT_INSTR_GDP_PIXEL_EAST,
 	PRJCT_INSTR_GDP_PIXEL_CENTER,
 	PRJCT_INSTR_GDP_PIXEL_WEST,
 	PRJCT_INSTR_GDP_PIXEL_BACKSCAN,
 	PRJCT_INSTR_GDP_PIXEL_MAX

 };

enum _sciaChannels
 {
  PRJCT_INSTR_SCIA_CHANNEL_1,
  PRJCT_INSTR_SCIA_CHANNEL_2,
  PRJCT_INSTR_SCIA_CHANNEL_3,
  PRJCT_INSTR_SCIA_CHANNEL_4,
  PRJCT_INSTR_SCIA_CHANNEL_MAX
 };

enum _omiSpectralTypes
 {
  PRJCT_INSTR_OMI_TYPE_UV1,
  PRJCT_INSTR_OMI_TYPE_UV2,
  PRJCT_INSTR_OMI_TYPE_VIS,
  PRJCT_INSTR_OMI_TYPE_MAX
 };

#define MAX_SWATHSIZE         200    // maximum number of tracks per swath (for satellites)


// Types of irradiances for SCIAMACHY

// D0 ESM diffuser -  neutral density filter in (regularly updated)
//
//        this is the only calibrated irradiance
//
// D1 ESM diffuser -  neutral density filter out
//
//        In principle as good as D0, but taken only once per month.  At the beginning,
//        some occasions for updating were missed.  So it starts with 999 values like
//        the other placeholders.
//
// D6 ASM diffuser (regularly updated)
//
//        All D6 data from 2002 cannot be used due to wront measurement geometry !
//
// D9 ASM diffuser spectrum in B.U. from 09.04.2003; not updated
//
//        Not in operational products but only validation masterset !  Can be used for DOAS.


#define SCIA_REFERENCE_ESM   0
#define SCIA_REFERENCE_ASM   6
#define SCIA_REFERENCE_ASMBU 9

// ----------------
// RESULTS TAB PAGE
// ----------------

// Possible fields to store in ascii format
// ----------------------------------------

enum _prjctResultsAscii
 {
  PRJCT_RESULTS_ASCII_SPECNO,
  PRJCT_RESULTS_ASCII_NAME,
  PRJCT_RESULTS_ASCII_DATE_TIME,
  PRJCT_RESULTS_ASCII_DATE,
  PRJCT_RESULTS_ASCII_TIME,
  PRJCT_RESULTS_ASCII_YEAR,
  PRJCT_RESULTS_ASCII_JULIAN,
  PRJCT_RESULTS_ASCII_JDFRAC,
  PRJCT_RESULTS_ASCII_TIFRAC,
  PRJCT_RESULTS_ASCII_SCANS,
  PRJCT_RESULTS_ASCII_NREJ,
  PRJCT_RESULTS_ASCII_TINT,
  PRJCT_RESULTS_ASCII_SZA,
  PRJCT_RESULTS_ASCII_CHI,
  PRJCT_RESULTS_ASCII_RMS,
  PRJCT_RESULTS_ASCII_AZIM,
  PRJCT_RESULTS_ASCII_TDET,
  PRJCT_RESULTS_ASCII_SKY,
  PRJCT_RESULTS_ASCII_BESTSHIFT,
  PRJCT_RESULTS_ASCII_REFZM,
  PRJCT_RESULTS_ASCII_REFSHIFT,
  PRJCT_RESULTS_ASCII_PIXEL,
  PRJCT_RESULTS_ASCII_PIXEL_TYPE,
  PRJCT_RESULTS_ASCII_ORBIT,
  PRJCT_RESULTS_ASCII_LONGIT,
  PRJCT_RESULTS_ASCII_LATIT,
  PRJCT_RESULTS_ASCII_ALTIT,
  PRJCT_RESULTS_ASCII_COVAR,
  PRJCT_RESULTS_ASCII_CORR,
  PRJCT_RESULTS_ASCII_CLOUD,
  PRJCT_RESULTS_ASCII_COEFF,
  PRJCT_RESULTS_ASCII_O3,
  PRJCT_RESULTS_ASCII_NO2,
  PRJCT_RESULTS_ASCII_CLOUDTOPP,
  PRJCT_RESULTS_ASCII_LOS_ZA,
  PRJCT_RESULTS_ASCII_LOS_AZIMUTH,
  PRJCT_RESULTS_ASCII_SAT_HEIGHT,
  PRJCT_RESULTS_ASCII_EARTH_RADIUS,
  PRJCT_RESULTS_ASCII_VIEW_ELEVATION,
  PRJCT_RESULTS_ASCII_VIEW_AZIMUTH,
  PRJCT_RESULTS_ASCII_VIEW_ZENITH,
  PRJCT_RESULTS_ASCII_SCIA_QUALITY,
  PRJCT_RESULTS_ASCII_SCIA_STATE_INDEX,
  PRJCT_RESULTS_ASCII_SCIA_STATE_ID,
  PRJCT_RESULTS_ASCII_STARTDATE,
  PRJCT_RESULTS_ASCII_ENDDATE,
  PRJCT_RESULTS_ASCII_STARTTIME,
  PRJCT_RESULTS_ASCII_ENDTIME,
  PRJCT_RESULTS_ASCII_SCANNING,
  PRJCT_RESULTS_ASCII_FILTERNUMBER,
  PRJCT_RESULTS_ASCII_MEASTYPE,
  PRJCT_RESULTS_ASCII_CCD_HEADTEMPERATURE,
  PRJCT_RESULTS_ASCII_COOLING_STATUS,
  PRJCT_RESULTS_ASCII_MIRROR_ERROR,
  PRJCT_RESULTS_ASCII_COMPASS,
  PRJCT_RESULTS_ASCII_PITCH,
  PRJCT_RESULTS_ASCII_ROLL,
  PRJCT_RESULTS_ASCII_ITER,
  PRJCT_RESULTS_ASCII_GOME2_SCANDIRECTION,
  PRJCT_RESULTS_ASCII_GOME2_SAA,
  PRJCT_RESULTS_ASCII_GOME2_SUNGLINT_RISK,
  PRJCT_RESULTS_ASCII_GOME2_SUNGLINT_HIGHRISK,
  PRJCT_RESULTS_ASCII_GOME2_RAINBOW,
  PRJCT_RESULTS_ASCII_CCD_DIODES,
  PRJCT_RESULTS_ASCII_CCD_TARGETAZIMUTH,
  PRJCT_RESULTS_ASCII_CCD_TARGETELEVATION,
  PRJCT_RESULTS_ASCII_SATURATED,
  PRJCT_RESULTS_ASCII_OMI_INDEX_SWATH,
  PRJCT_RESULTS_ASCII_OMI_INDEX_ROW,
  PRJCT_RESULTS_ASCII_OMI_GROUNDP_QF,
  PRJCT_RESULTS_ASCII_OMI_XTRACK_QF,
  PRJCT_RESULTS_ASCII_OMI_PIXELS_QF,
  PRJCT_RESULTS_ASCII_SPIKES,
  PRJCT_RESULTS_ASCII_UAV_SERVO_BYTE_SENT,
  PRJCT_RESULTS_ASCII_UAV_SERVO_BYTE_RECEIVED,
  PRJCT_RESULTS_ASCII_MAX
 };

// Fields to store when using NASA-AMES format
// -------------------------------------------

enum _prjctResultsNasaFields
 {
  PRJCT_RESULTS_NASA_NO2,
  PRJCT_RESULTS_NASA_O3,
  PRJCT_RESULTS_NASA_OCLO,
  PRJCT_RESULTS_NASA_BRO,
  PRJCT_RESULTS_NASA_MAX
 };

// --------------------------
// PROPERTY SHEET DESCRIPTION
// --------------------------

#define MAX_PROJECT 10

enum prjctTabPagesTypes
 {
  TAB_TYPE_PRJCT_SPECTRA,                              // Spectra selection tab page
  TAB_TYPE_PRJCT_ANLYS,                                // Analysis tab page
  TAB_TYPE_PRJCT_FILTER,                               // Filter tab page
  TAB_TYPE_PRJCT_CALIBRATION,                          // Calibration tab page
  TAB_TYPE_PRJCT_USAMP,                                // Undersampling tab page
  TAB_TYPE_PRJCT_INSTRUMENTAL,                         // Instrumental tab page
  TAB_TYPE_PRJCT_SLIT,                                 // Slit function tab page
  TAB_TYPE_PRJCT_ASCII_RESULTS,                        // ASCII Results tab page
  TAB_TYPE_PRJCT_NASA_RESULTS,                         // NASA-AMES Results tab page
  TAB_TYPE_PRJCT_MAX
 };

// ===============================
// CROSS SECTIONS CONVOLUTION TOOL
// ===============================

   // ---------------------
   // CONSTANTS DEFINITIONS
   // ---------------------

   // Supported line shapes for convolution
   // -------------------------------------

   enum _slitTypes
    {
  //  	SLIT_TYPE_NONE,
     SLIT_TYPE_FILE,                                                            // user-defined line shape provided in a file
     SLIT_TYPE_GAUSS,                                                           // Gaussian line shape
     SLIT_TYPE_INVPOLY,                                                         // 2n-Lorentz (generalisation of the Lorentzian function
     SLIT_TYPE_VOIGT,                                                           // Voigt profile function
     SLIT_TYPE_ERF,                                                             // error function (convolution of a Gaussian and a boxcar)
     SLIT_TYPE_AGAUSS,                                                          // asymmetric gaussian line shape
     SLIT_TYPE_APOD,                                                            // apodisation function (used with FTS)
     SLIT_TYPE_APODNBS,                                                         // apodisation function (Norton Beer Strong function)
   //  SLIT_TYPE_GAUSS_FILE,                                                      // Gaussian line shape, wavelength dependent (provided in a file)
   //  SLIT_TYPE_INVPOLY_FILE,                                                    // 2n-Lorentz line shape, wavelength dependent (file)
   //  SLIT_TYPE_VOIGT_FILE,                                                    // Voigt profile function, wavelength dependent (file)
   //  SLIT_TYPE_ERF_FILE,                                                        // error function, wavelength dependent (file)
   //  SLIT_TYPE_GAUSS_T_FILE,                                                    // Gaussian line shape, wavelength and temperature dependent (file)
   //  SLIT_TYPE_ERF_T_FILE,                                                      // error function, wavelength and temperature dependent (file)
     SLIT_TYPE_MAX
    };

   // Convolution type
   // ----------------

   enum _convolutionTypes
    {
     CONVOLUTION_TYPE_NONE,                                                     // no convolution, interpolation only
     CONVOLUTION_TYPE_STANDARD,                                                 // standard convolution
     CONVOLUTION_TYPE_I0_CORRECTION,                                            // convolution using I0 correction
   // CONVOLUTION_TYPE_RING,                                                    // creation of a ring xs using high-resoluted solar and raman spectra
     CONVOLUTION_TYPE_MAX
    };

   // Conversion modes
   // ----------------

   enum _conversionModes
    {
     CONVOLUTION_CONVERSION_NONE,                                               // no conversion
     CONVOLUTION_CONVERSION_AIR2VAC,                                            // air to vacuum
     CONVOLUTION_CONVERSION_VAC2AIR,                                            // vacuum to air
     CONVOLUTION_CONVERSION_MAX
    };

  // Pages of the dialog box
  // -----------------------

   enum _convolutionTabPages
    {
     TAB_TYPE_XSCONV_GENERAL,                                                   // general information (files names, convolution type...)
     TAB_TYPE_XSCONV_SLIT,                                                      // information on the slit function
     TAB_TYPE_XSCONV_FILTER,                                                    // information on the filter to apply
     TAB_TYPE_XSCONV_MAX
    };


enum omi_xtrack_mode { 
  IGNORE = 0,
  STRICT = 1,
  NONSTRICT = 2
};

extern const char *STR_IGNORE, *STR_STRICT, *STR_NONSTRICT;
#endif

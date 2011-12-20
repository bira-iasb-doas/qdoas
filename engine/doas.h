

//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  GEOCENTRIC MOON POSITIONS COMPUTATION
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

#include "windoas.h"

#if !defined(__DOAS_)
#define __DOAS_

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// ==================
// HEADERS TO INCLUDE
// ==================

// =====================
// CONSTANTS DEFINITIONS
// =====================

#define MAX_FEN                20    // maximum number of gaps in a analysis window
#define MAX_FIT                50    // maximum number of parameters to fit
#define MAX_FENO               25    // maximum number of analysis windows in a project
#define MAX_SYMB              150    // maximum number of different symbols in a project

#define DIM                    10    // default number of security pixels for border effects

#define MAX_KURUCZ_FWHM_PARAM   4    // maximum number of non linear parameters for fitting fwhm with Kurucz

#define STOP      ITEM_NONE

#define PI          (double) 3.14159265358979323846
#define PI2         (double) 6.28318530717958647692
#define PIDEMI      (double) 1.57079632679489661923

#define EPSILON     (double)   1.e-6

// ===================================================
// VECTOR.C : UTILITY FUNCTIONS FOR VECTORS AND MATRIX
// ===================================================

void   VECTOR_Init(double *vector,double value,int dim);
int    VECTOR_Equal(double *vector1,double *vector2,int dim,double error);
double VECTOR_Max(double *vector,int dim);
double VECTOR_Min(double *vector,int dim);
RC     VECTOR_Log(double *out,double *in,int dim,DoasCh *callingFunction);
int    VECTOR_LocGt(double *vector,double value,int dim);
void   VECTOR_Invert(double *vector,int dim);
double VECTOR_Table1(double *X0,int Nx,double *Y0,int Ny,double **Table,double X,double Y);
double VECTOR_Table2(double **Table,INT Nx,INT Ny,double X,double Y);
double VECTOR_Norm(double *v,INT dim);
RC     VECTOR_NormalizeVector(double *v,INT dim,double *fact,DoasCh *function);

// =================================
// FVOIGT.C : Voigt profile function
// =================================

double Voigtx(double x,double y);

// ======================
// ERF.C : error function
// ======================

double erf ( double x );

double ERF_GetValue(double newX);
RC     ERF_Alloc(void);
void   ERF_Free(void);

// ===========================
// FILTER.C : Filter functions
// ===========================

// ----------
// PROTOTYPES
// ----------

void realft(double *source,double *buffer,int nn,int sens);

RC   FILTER_OddEvenCorrection(double *lambdaData,double *specData,double *output,INT vectorSize);
RC   FILTER_Vector(PRJCT_FILTER *pFilter,double *Input,double *Output,int Size,INT outputType);
RC   FILTER_Build(PRJCT_FILTER *pFilter,double param1,double param2,double param3);
RC   FILTER_LoadFilter(PRJCT_FILTER *pFilter);

// =========================================
// SPLINE.C : linear and cubic interpolation
// =========================================

enum _spline
 {
  SPLINE_LINEAR,                                                             // linear interpolation
  SPLINE_CUBIC,                                                             // spline interpolation
  SPLINE_MAX
 };

RC SPLINE_Deriv2(double *X,double *Y,double *Y2,int n,DoasCh *callingFunction);
RC SPLINE_Vector(double *xa,double *ya,double *y2a,int na,double *xb,double *yb,int nb,int type,DoasCh *callingFunction);

// ====================================
// SVD.C : Singular value decomposition
// ====================================

// SVD decomposition on an analysis window
// ---------------------------------------

typedef struct _svd
 {
  INT DimC,DimL,DimP,NF,NP,Z,nFit,                     // gaps and analysis window limits in pixels units
      Fenetre[MAX_FEN][2];

  double   LFenetre[MAX_FEN][2],                       // gaps and analysis window limits in wavelength units (nm)
         **A,**U,**V,*W,**P,                           // SVD matrices
         **covar,
          *SigmaSqr;
 }
SVD;

RC SVD_Bksb(double **u,double *w,double **v,int m,int n,double *b,double *x);
RC SVD_Dcmp(double **a,int m,int n,double *w,double **v,double *SigmaSqr,double **covar);

void SVD_Free(DoasCh *callingFunctionShort,SVD *pSvd);
RC   SVD_LocalAlloc(DoasCh *callingFunctionShort,SVD *pSvd);

// ============================================================
// CURFIT.C : Least-square fit applied to a non linear function
// ============================================================

double Fchisq(int mode,int nFree,double *Y,double *Yfit,double *sigmay,int nY);

RC Curfit(int     mode,                                                         // I   method of weighting least-squares fit
          int     nFree,                                                        // I   the number of degrees of freedom
          double *X,                                                            // I   data points for independent variable
          double *specX,                                                        // I   the spectrum to evaluate
          double *srefX,                                                        // I   the control spectrum (also called reference spectrum)
          int     nX,                                                           // I   the size of previous vectors (depending on the size of the detector)
          double *Y,                                                            // I   the data to fit (vector of zeros to fit the residual)
          double *sigmaY,                                                       // I   standard deviations for Y data points
          int     nY,                                                           // I   number of data points in Y
          double *P,                                                            // I/O parameters fitted linearly in the fitting function (ANALYSE_Function)
          double *A,                                                            // I/O parameters to fit non linearly in this function
          double *deltaA,                                                       // I   increments for non linear parameters in A
          double *sigmaA,                                                       // O   standard deviations for non linear parameters in A
          double *minA,                                                         // I   lower limit for fitted parameters in A
          double *maxA,                                                         // I   upper limit for fitted parameters in A
          int     nA,                                                           // I   number of parameters in A
          double *Yfit,                                                         // O   vector of calculated values of Y
          double *pLambda,                                                      // O   proportion of gradient search included
          double *pChisqr,                                                      // O   reduced Chi square for fit ( output )
          int    *pNiter);                                                      // O   number of iterations

// ---------------------------------------------------------------------
// MOON.C : Get moon positions in terms of azimuthal angle and elevation
// ---------------------------------------------------------------------

void MOON_GetPosition(DoasCh  *inputDate,   // input date and time for moon positions calculation
                      double  longitude,   // longitude of the observation site
                      double  latitude,    // latitude of the observation site
                      double  altitude,    // altitude of the observation site
                      double *pA,          // azimuth, measured westward from the south
                      double *ph,          // altitude above the horizon
                      double *pFrac);       // illuminated fraction of the moon

// ==========================================================
// ZENITHAL.C : CALCULATION OF ZENITHAL ANGLES AND JULIAN DAY
// ==========================================================

// Prototypes
// ----------

double  ZEN_NbSec     ( SHORT_DATE *today, struct time *now, int flag);
double  ZEN_FNTdiz    ( double NbreJours, double *ObsLong, double *ObsLat,double *pAzimuth );
double  ZEN_FNCrtjul  ( double *NbreSec );
double  ZEN_FNCaldti  ( double *Tm );
char   *ZEN_FNCaljti  ( double *Tm, char *str );
int     ZEN_FNCaljda  ( double *Tm );
int     ZEN_FNCaljday ( int Year, int Julian );
int     ZEN_FNCaljye  ( double *Tm );
int     ZEN_FNCaljmon ( int Year, int Julian );

// ==================================
// ANALYSE.C : main analysis function
// ==================================

// ---------------------
// STRUCTURE DEFINITIONS
// ---------------------

// Symbols used in a project
// -------------------------

enum _wrkSymbolType
 {
  WRK_SYMBOL_CROSS,
  WRK_SYMBOL_CONTINUOUS,
  WRK_SYMBOL_PREDEFINED,
  WRK_SYMBOL_SPECTRUM
 };

typedef struct _wrkSymbol
 {
  DoasCh        type,                           // type of symbol
                symbolName[MAX_STR_LEN+1],      // name of symbol
                crossFileName[MAX_STR_LEN+1],   // name of cross section file
                amfFileName[MAX_STR_LEN+1];     // name of AMF file name
  MATRIX_OBJECT xs;                             // cross sections (wavelength+cross section(s))
 }
WRK_SYMBOL;

// Symbol cross reference
// ----------------------

typedef struct _crossReference
 {
  int    Comp,                                                                  // index of component in WrkSpace list
         crossAction,                                                           // action to process on cross section before analysis
         IndSvdA,                                                               // index of column in SVD matrix
         IndSvdP,                                                               // index of column in SVD matrix
         IndOrthog,                                                             // order in orthogonal base
         FitConc,                                                               // flag set if concentration is to be fit (non linear method) or modified (linear method)
         FitFromPrevious,                                                       // flag set if the value of the concentration has to be retrieved from a previous window
         FitParam,                                                              // flag set if non linear parameter (other than shift or stretch) is to be fit
         FitShift,                                                              // flag set if shift is to be fit
         FitStretch,                                                            // flag set if stretch order 1 is to be fit
         FitStretch2,                                                           // flag set if stretch order 2 is to be fit
         FitScale,                                                              // flag set if a scaling factor order 1 is to be fit
         FitScale2,                                                             // flag set if a scaling factor order 2 is to be fit
         TypeStretch,                                                           // order of stretch to fit
         TypeScale;                                                             // order of scaling factor to fit

  DoasCh display,                                                               // flag set if fit is to be displayed
         amfType,                                                               // type of AMF
         filterFlag;                                                            // flag set if symbol is to be filteres

  double Fact,                                                                  // normalization factors
         I0Conc,
         InitConc,                                                              // initial concentration
         InitParam,                                                             // initial non linear parameter
         InitShift,                                                             // initial shift
         InitStretch,                                                           // initial stretch order 1
         InitStretch2,                                                          // initial stretch order 2
         InitScale,                                                             // initial scaling factor order 1
         InitScale2,                                                            // initial scaling factor order 2
         DeltaConc,                                                             // step for concentration
         DeltaParam,                                                            // step for non linear parameter
         DeltaShift,                                                            // step for shift
         DeltaStretch,                                                          // step for stretch order 1
         DeltaStretch2,                                                         // step for stretch order 2
         DeltaScale,                                                            // step for scaling factor order 1
         DeltaScale2,                                                           // step for scaling factor order 2
         MinShift,                                                              // maximum value for shift
         MaxShift,                                                              // maximum value for shift
         MinParam,                                                              // minimum value for parameter
         MaxParam,                                                              // maximum value for parameter
        *vector,                                                                // copy of vector
        *Deriv2;                                                                // second derivative
 }
CROSS_REFERENCE;

// Results
// -------

typedef struct _crossResults
 {
  DoasCh  StoreParam,                // flag set if non linear parameter is to be written into output file
         StoreShift,                // flag set if shift is to be written into output file
         StoreStretch,              // flag set if stretch order 1 is to be written into output file
         StoreScale,                // flag set if scaling factor order 1 is to be written into output file
         StoreParamError,           // flag set if error on non linear parameter is to be written into output file
         StoreError,                // flag set if error on previous parameters is to be written into output file
         StoreAmf,                  // flag set if air mass factor is to be written into output file
         StoreSlntCol,              // flag set if slant column is to be written into output file
         StoreSlntErr,              // flag set if error on slant column is to be written into output file
         StoreVrtCol,               // flag set if vertical column is to be written into output file
         StoreVrtErr;               // flag set if error on vertical column is to be written into output file

  double Param,                     // non linear parameter returned by CurFitMethod
         Shift,                     // shift returned by CurFitMethod
         Stretch,                   // stretch order 1 returned by CurFitMethod
         Stretch2,                  // stretch order 2 returned by CurFitMethod
         Scale,                     // scaling factor order 1 returned by CurFitMethod
         Scale2,                    // scaling factor order 2 returned by CurFitMethod
         SigmaParam,                // error on param
         SigmaShift,                // error on shift
         SigmaStretch,              // error on stretch order 1
         SigmaStretch2,             // error on stretch order 2
         SigmaScale,                // error on scaling factor order 1
         SigmaScale2,               // error on scaling factor order 2
         SlntCol,                   // slant column
         SlntErr,                   // error on slant column
         SlntFact,                  // slant column factor
         VrtCol,                    // vertical column
         VrtErr,                    // error on vertical column
         VrtFact,                   // vertical column factor
         ResCol,                    // residual column
         Amf;                       // air mass factor

  INDEX  indexAmf;                  // index of AMF data in AMF table cross reference
 }
CROSS_RESULTS;

// Analysis window description
// ---------------------------

enum _analysisType
 {
  ANALYSIS_TYPE_FWHM_NONE,                                                      //  no fwhm fit
  ANALYSIS_TYPE_FWHM_CORRECTION,                                                //  fwhm correction between spectrum and reference based on their temperature
  ANALYSIS_TYPE_FWHM_KURUCZ,                                                    //  fwhm fit in Kurucz procedure
  ANALYSIS_TYPE_FWHM_NLFIT                                                      //  fit the difference of resolution between spectrum and reference
 };

typedef struct _satellite_ref_
 {
  INDEX  indexFile;
  INDEX  indexRecord;
  INDEX  pixelNumber;
  INDEX  pixelType;
  double sza;
  double latitude;
  double longitude;
  double szaDist;
  double latDist;
 }
SATELLITE_REF;

typedef struct _feno
 {
                                                                                //  copy of data from analysis window panel

  DoasCh          windowName[MAX_ITEM_NAME_LEN+1];                              //  name of analysis window
  DoasCh          refFile[MAX_ITEM_TEXT_LEN+1],                                 //  reference file in reference file selection mode
                  ref1[MAX_ITEM_TEXT_LEN+1],                                    //  first reference spectrum (in order to replace the SrefEtalon in the old ANALYSIS_WINDOWS structure)
                  ref2[MAX_ITEM_TEXT_LEN+1],                                    //  second reference spectrum (in order to replace the SrefEtalon in the old ANALYSIS_WINDOWS structure)
                  residualsFile[MAX_ITEM_TEXT_LEN+1];
  double          refSZA,refSZADelta,refMaxdoasSZA,refMaxdoasSZADelta;          //  in automatic reference selection mode, SZA constraints
  INT             refSpectrumSelectionMode;                                     //  reference spectrum selection mode
  INT             refMaxdoasSelectionMode;                                      //  for MAXDOAS measurements, selection of the reference spectrum based on the scan or the SZA
  double          cloudFractionMin,cloudFractionMax;
  DoasCh          refAM[MAX_ITEM_TEXT_LEN+1],refPM[MAX_ITEM_TEXT_LEN+1];        //  in automatic reference selection mode, names of the spectra files selected for the reference spectra (specific file format : MFC)
  INDEX           indexRefMorning,indexRefAfternoon,                            //  in automatic reference selection mode, index of selected records
                  indexRef;                                                     //  in automatic reference selection mode, index of current selected record
  double          ZmRefMorning,ZmRefAfternoon,Zm,                               //  in automatic reference selection mode, zenithal angles of selected records
                  oldZmRefMorning,oldZmRefAfternoon,                            //  make a copy of previous zenithal angles
                  TimeDec,Tm,                                                   //  in automatic reference selection mode, measurement time of selected record
                  TDet;                                                         //  temperature of reference

  SHORT_DATE      refDate;                                                      //  in automatic reference selection mode, date of selected record
  INT             displaySpectrum;                                              //  force display spectrum
  INT             displayResidue;                                               //  force display residue
  INT             displayTrend;                                                 //  force display trend
  INT             displayRefEtalon;                                             //  force display alignment of reference on etalon
  INT             displayFits;                                                  //  force display fits
  INT             displayPredefined;                                            //  force display predefined parameters
  INT             displayRef;

  INT             displayFlag;                                                  //  summary of the previous flag
  INT             displayLineIndex;                                             //  index of the current line
  INT             hidden;                                                       //  flag set if window is hidden e.g. for Kurucz calibration
  INT             useKurucz;                                                    //  flag set if Kurucz calibration is to be used for a new wavelength scale
  INT             useUsamp;                                                     //  flag set if undersampling correction is requested
  INT             amfFlag;                                                      //  flag set if there is a wavelength dependence of AMF for one or several cross sections
  INT             useEtalon;                                                    //  flag set if etalon reference is used
  INT             xsToConvolute;                                                //  flag set if high resolution cross sections to convolute real time
  INT             xsToConvoluteI0;

  SATELLITE_REF  *satelliteRef;

  double         *LambdaRef,                                                    //  absolute reference wavelength scale
                 *LambdaK,                                                      //  new wavelength scale after Kurucz
                 *Lambda,                                                       //  wavelength scale to use for analysis
                 *Sref,                                                         //  reference spectrum
                 *SrefSigma,                                                    //  error on reference spectrum
                 *SrefEtalon,                                                   //  etalon reference spectrum
                 *LambdaN,*LambdaS,
                 *SrefN,*SrefS,
                  Shift,                                                        //  shift found when aligning etalon on reference
                  Stretch,                                                      //  stretch order 1 found when aligning etalon on reference
                  Stretch2,                                                     //  stretch order 2 found when aligning etalon on reference
                  refNormFact,
                  ShiftN,                                                       //  shift found when aligning etalon on reference
                  StretchN,                                                     //  stretch order 1 found when aligning etalon on reference
                  Stretch2N,                                                    //  stretch order 2 found when aligning etalon on reference
                  refNormFactN,
                  ShiftS,                                                       //  shift found when aligning etalon on reference
                  StretchS,                                                     //  stretch order 1 found when aligning etalon on reference
                  Stretch2S,                                                    //  stretch order 2 found when aligning etalon on reference
                  refNormFactS,
                  chiSquare,                                                    //  chi square
                  RMS;

  INT             nIter;                                                        //  number of iterations
  INT             Decomp;                                                       //  force SVD decomposition
  SVD             svd;                                                          //  SVD decomposition data
  CROSS_REFERENCE TabCross[MAX_FIT];                                            //  symbol cross reference
  CROSS_RESULTS   TabCrossResults[MAX_FIT];                                     //  results stored per symbol in previous list
  INT             NTabCross;                                                    //  number of elements in the two previous lists
  INDEX           indexSpectrum,                                                //  index of raw spectrum in symbol cross reference
                  indexReference,                                               //  index of reference spectrum in symbol cross reference
                  indexFwhmParam[MAX_KURUCZ_FWHM_PARAM],                        //  index of 1st predefined parameter when fitting fwhm with Kurucz
                  indexFwhmConst,                                               //  index of 'fwhm (constant)' predefined parameter in symbol cross reference
                  indexFwhmOrder1,                                              //  index of 'fwhm (order 1)' predefined parameter in symbol cross reference
                  indexFwhmOrder2,                                              //  index of 'fwhm (order 2)' predefined parameter in symbol cross reference
                  indexSol,                                                     //  index of 'Sol' predefined parameter in symbol cross reference
                  indexOffsetConst,                                             //  index of 'offset (constant)' predefined parameter in symbol cross reference
                  indexOffsetOrder1,                                            //  index of 'offset (order 1)' predefined parameter in symbol cross reference
                  indexOffsetOrder2,                                            //  index of 'offset (order 2)' predefined parameter in symbol cross reference
                  indexCommonResidual,                                          //  index of 'Common residual' predefined parameter in symbol cross reference
                  indexUsamp1,                                                  //  index of 'Undersampling (phase 1)' predefined parameter in symbol cross reference
                  indexUsamp2,                                                  //  index of 'Undersampling (phase 2)' predefined parameter in symbol cross reference
                  indexRing1;

  int             OrthoSet[MAX_FIT];                                            //  Vecteurs candidats à une orthogonalisation
  int             NOrtho;
  int             DifRLis;                                                      //  Nombre de points pour le lissage du log du sp. de ref
  double         *fwhmPolyRef[MAX_KURUCZ_FWHM_PARAM],                           //  polynomial coefficients for building wavelength dependence of fwhm for reference
                 *fwhmVector[MAX_KURUCZ_FWHM_PARAM],
                 *fwhmDeriv2[MAX_KURUCZ_FWHM_PARAM],
                  xmean,ymean;                                                  // resp. the spectrum and reference averaged on the fitting window
  int             analysisType;
  int             analysisMethod;
  int             bandType;
  double          refLatMin,refLatMax;
  double          refLonMin,refLonMax;
  int             nspectra;
  INT             NDET;
  double          lambdaMinK,lambdaMaxK;
  INT             gomeRefFlag;
  INT             mfcRefFlag;
  RC              rcKurucz;
  INT             SvdPDeb,SvdPFin,Dim,LimMin,LimMax,LimN;
  INT             rc;
  DoasCh           gomePixelType[4];
  INT             offlFlag;                            // non zero if linear offset is fitted
  int             longPathFlag;                                                 // for Anoop
 }
FENO;

// =================
// THREAD PROCESSING
// =================

// ---------------------
// CONSTANTS DEFINITIONS
// ---------------------

enum _thrdBrowse
 {
  THREAD_BROWSE_SPECTRA,
  THREAD_BROWSE_DARK,
  THREAD_BROWSE_ERROR,
  THREAD_BROWSE_EXPORT,
  THREAD_BROWSE_MFC_OFFSET,
  THREAD_BROWSE_MFC_DARK,
  THREAD_BROWSE_MFC_INSTR,
  THREAD_BROWSE_MFC_LAMP
 };

enum _thrdLevels
 {
  THREAD_LEVEL_RECORD,
  THREAD_LEVEL_FILE,
  THREAD_LEVEL_PROJECT,
  THREAD_LEVEL_MAX
 };

enum _threadEvents
 {
  THREAD_EVENT_FIRST,
  THREAD_EVENT_PREVIOUS,
  THREAD_EVENT_NEXT,
  THREAD_EVENT_LAST,
  THREAD_EVENT_GOTO,
  THREAD_EVENT_PLAY,
  THREAD_EVENT_PAUSE,
  THREAD_EVENT_STOP,
  THREAD_EVENT_STOP_PROGRAM,
  THREAD_EVENT_MAX
 };

enum _thrdGoto
 {
  THREAD_GOTO_RECORD,
  THREAD_GOTO_PIXEL
 };

// ----------------------
// STRUCTURES DEFINITIONS
// ----------------------

// Data on current spectrum
// ------------------------

// Geolocations and angles for satellite measurements

typedef struct _satelliteGeoloc
 {
  // Geolocations

  double lonCorners[4],
         latCorners[4],
         lonCenter,
         latCenter;

  // Angles

  float  solZen[3],
         solAzi[3],
         losZen[3],
         losAzi[3];

  // Miscellaneous

  float earthRadius,satHeight;
  float cloudTopPressure,cloudFraction;                                         // information on clouds
  int   saaFlag;
  int   sunglintDangerFlag;
  int   sunglintHighDangerFlag;
  int   rainbowFlag;
  int   scanDirection;
 }
SATELLITE_GEOLOC;

// ==============
// ENGINE CONTEXT
// ==============

// Buffers specific to CCD

typedef struct _ccd
 {
  MATRIX_OBJECT drk;
  MATRIX_OBJECT vip;
  MATRIX_OBJECT dnl;
  INT           filterNumber;
  double        headTemperature;
  INT           measureType;
  float         diodes[4];
  float         targetElevation,targetAzimuth;
  int           saturatedFlag;
  INDEX         indexImage;
 }
CCD;

// Record information specific to the GOME format

typedef struct _gomeData                                                        // data on the current GOME pixel
 {
  INT   orbitNumber;                                                            // orbit number
  INT   pixelNumber;                                                            // pixel number
  INT   pixelType;                                                              // pixel type

  SHORT_DATE irradDate;                                                         // date of measurement for the irradiance spectrum
  struct time irradTime;                                                        // time of measurement for the irradiance spectrum

  int     nRef;                                                                 // size of irradiance vectors

  float longit[5];                                                              // longitudes (four corners of the GOME pixel + pixel centre)
  float latit[5];                                                               // latitudes (four corners of the GOME pixel + pixel centre)
  float sza[3];                                                                 // solar zenith angles (East, center and west points of the GOME pixel)
  float azim[3];                                                                // solar azimuth angles (East, center and west points of the GOME pixel)
 }
GOME_DATA;

// Record information specific to SCIAMACHY

typedef struct _sciamachy
 {
  INT    orbitNumber;                                                           // orbit number
  double longitudes[4],latitudes[4];                                            // geolocations at the 4 corners of the pixels
  float  solZen[3],solAzi[3],losZen[3],losAzi[3];                               // resp. solar and line of sight zenith and azimuth angles
  float  earthRadius,satHeight;                                                 // for satellite to TOA angles correction
  INDEX  stateIndex,stateId;                                                    // information on the state
  INT    qualityFlag;
 }
SCIA_DATA;

// Record information specific to GOME2

typedef struct _gome2
 {
  INT    orbitNumber;                                                           // orbit number
  double longitudes[4],latitudes[4];                                            // geolocations at the 4 corners of the pixels
  float  solZen[3],solAzi[3],losZen[3],losAzi[3];                               // resp. solar and line of sight zenith and azimuth angles
  float  earthRadius,satHeight;                                                 // for satellite to TOA angles correction
  int    saaFlag;
  int    sunglintDangerFlag;
  int    sunglintHighDangerFlag;
  int    rainbowFlag;
  int    scanDirection;
 }
GOME2_DATA;

// Record information specific to OMI

typedef struct _omi
 {
 	INDEX omiTrackIndex;                                                          // index of the track record
 	INDEX omiSpecIndex;                                                           // index of the current spectrum in the current track
 	int   omiNumberOfTracks,                                                      // total number of tracks
 	      omiNumberOfSpectraPerTrack;                                             // total number of spectra in tracks
 }
OMI_DATA;

typedef struct _als
 {
  short       alsFlag;
  short       scanIndex;
  double      scanningAngle;
  float       compassAngle;
  float       pitchAngle;
  float       rollAngle;
  DoasCh      atrString[1024];
 }
ALS_DATA;

typedef struct _mkzy
 {
  double         scanningAngle;
  double         scanningAngle2;
  char           instrumentname[16];                                            // the name of the instrument
  unsigned short startc;                                                        // the startchannel for the first data-point
  unsigned short pixels;                                                        // number of pixels saved in the data-field
  unsigned char  channel;                                                       // channel of the spectrometer, typically 0
  char           coneangle;                                                     // new in version 4, given in cfg.txt
  int            darkFlag,offsetFlag,skyFlag;                                   // flags indicating the presence resp. of dark current, offset and sky spectra in the file
  int            darkScans,offsetScans;                                         // number of scans of the dark current and the offset
  double         darkTint;
  int            recordNumber;
 }
MKZY_DATA;

typedef struct _mfcBira
 {
 	char originalFileName[1024];
 	int  measurementType;
 }
MFC_BIRA;

typedef struct uoft_format
 {
 	SHORT_DATE meanDate;                                                          // averaged date
  SHORT_DATE startDate;                                                         // start date
  SHORT_DATE endDate;                                                           // end date
  struct time meanTime;                                                         // averaged time
  struct time startTime;                                                        // start time
  struct time endTime;                                                          // end time
  float solarElevAngle;                                                         // averaged solar elevation angle
  float startSolarElevAngle;                                                    // solar elevation angle at the beginning of the measurement
  float endSolarElevAngle;                                                      // solar elevation angle at the end of the measurement
  float shutter;                                                                // shutter
  float numCounts;                                                              // ideal Num of Counts
  float slitWidth;                                                              // slit Width
  float groove;                                                                 // groove Density
  float turret;                                                                 // turret Position
  float blazeWve;                                                               // blaze wavelength
  float centerWve;                                                              // centre wavelength
  float intTime;                                                                // integration Time
  float numAcc;                                                                 // num Accumulations
  float meanCCDT;                                                               // mean CCD temperature
  float minCCDT;                                                                // min CCD temperature
  float maxCCDT;                                                                // max TCCD temperature
  float meanBoxT;                                                               // mean box temperature
  float measType;                                                               // measurement type
  float viewElev;
  float viewAzim;
  float filterId;
  float longitude;
  float latitude;
 }
UOFT_DATA;

// Buffers needed to load spectra

typedef struct _engineBuffers
 {
  // spectra buffers

  double *lambda,                                                               // wavelengths
         *spectrum,                                                             // raw spectrum
         *sigmaSpec,                                                            // error on raw spectrum if any
         *irrad,                                                                // irradiance spectrum (for satellites measurements)
         *darkCurrent,                                                          // dark current
         *offset,                                                               // offset
         *specMaxx,                                                             // scans number for SpecMax
         *specMax,                                                              // maxima of signal over scans
         *instrFunction,                                                        // instrumental function
         *varPix,                                                               // variability interpixel
         *scanRef,                                                              // reference spectrum for the scan (MAXDOAS measurements)
         *dnl;                                                                  // non linearity of detector

  DoasU32  *recordIndexes;                                                      // indexes of records for direct access (specific to BIRA-IASB spectra file format)
 }
BUFFERS;

// Information related to the file

typedef struct _engineFileInfo
 {
 	DoasCh   fileName[MAX_STR_LEN+1];                                             // the name of the file
 	FILE   *specFp,*darkFp,*namesFp;                                              // file pointers for the engine
 	INT nScanRef;                                                                 // number of reference spectra in the scanRefIndexes buffer
 }
FILE_INFO;

// Information on the record

typedef struct _engineRecordInfo
 {
  // Measurement data common to any format

  char   Nom[21];                                                               // name of the spectrum
  int    NSomme;                                                                // total number of scans
  int    rejected;                                                              // number of rejected scans
  double Tint,                                                                  // integration time
         Zm,                                                                    // solar zenith angle
         oldZm,
         Azimuth,                                                               // solar azimut angle
         Tm;                                                                    // date and time in seconds since ...
  char   SkyObs;                                                                // observation of the sky (obsolete)
  float  ReguTemp;                                                              // temperature regulation
  double TotalExpTime;                                                          // total experiment time

  SHORT_DATE present_day;                                                       // measurement date
  struct time present_time;                                                     // measurement time

  double TimeDec;                                                               // decimal time
  double localTimeDec;                                                          // local decimal time
  INT    localCalDay;                                                           // local calendar day
  double Cic;                                                                   // color index

  double aMoon,hMoon,fracMoon;                                                  // moon information

  float  zenithViewAngle;                                                       // zenith viewing angle
  float  elevationViewAngle;                                                    // elevation viewing angle
  float  azimuthViewAngle;                                                      // azimuth viewing angle
  float  scanningAngle;                                                         // scanning angle

  // Data related to specific formats

                                                                                // SATELLITES MEASUREMENTS

  int    useErrors;                                                             // 1 if errors are present in the files (GOME)
  int    coolingStatus,mirrorError;                                             // only for OHP measurements (September 2008)
  INDEX  indexBand;                                                             // index of the band

  GOME_DATA gome;                                                               // GOME format
  SCIA_DATA scia;                                                               // SCIAMACHY format
  GOME2_DATA gome2;                                                             // GOME2 format
  OMI_DATA omi;
  ALS_DATA als;
  MKZY_DATA mkzy;
  MFC_BIRA mfcBira;
  UOFT_DATA uoft;

  double longitude;                                                             // longitude
  double latitude;                                                              // latitude
  double altitude;                                                              // altitude
  double cloudFraction;
  double cloudTopPressure;

                                                                                // SAOZ

  double TDet,                                                                  // temperature of the detector
         BestShift;                                                             // best shift

  int NTracks;                                                                  // Nbre de tracks retenus

                                                                                // MFC format

  SHORT_DATE startDate;
  SHORT_DATE endDate;
  struct time startTime;                                                        // starting time
  struct time endTime;                                                          // ending time
  float wavelength1;                                                            // first wavelength
  float dispersion[3];                                                          // dispersion parameters

  // Reference data

  DoasCh  refFileName[MAX_PATH_LEN+1];
  INT    refRecord;

  // CCD

  CCD    ccd;    // !!! This field should always be the last one -> cfr. ENGINE_CopyContext
 }
RECORD_INFO;

typedef struct _engineCalibFeno
 {
  cross_section_list_t crossSectionList;
  struct anlyswin_linear linear;
  struct calibration_sfp sfp[4]; // SFP1 .. SFP4
  shift_stretch_list_t shiftStretchList;
  output_list_t outputList;
 }
CALIB_FENO;

typedef struct _analysisRef
 {
 	DoasCh   *scanRefFiles;                                                       // in automatic selection of the reference spectrum, maxdoas measurements, scan mode, it is important to save the name of the reference file
 	INT      *scanRefIndexes;                                                     // in automatic selection of the reference spectrum, maxdoas measurements, scan mode, indexes of zenith spectra of the scan

 	int nscanRefFiles;

 	int refAuto;
 	int refScan;
 	int refSza;
 	int refLon;
 }
ANALYSIS_REF;

// Analysis part of the engine

// QDOAS engine

typedef struct _engineContext
 {
  BUFFERS           buffers;                                                    // buffers needed to load spectra
  FILE_INFO         fileInfo;                                                   // the name of the file to load and file pointers
  RECORD_INFO       recordInfo;                                                 // data on the current record
  PROJECT           project;                                                    // data from the current project
  ANALYSIS_REF      analysisRef;

  // record information

  INT     recordNumber;                                                         // total number of record in file
  INT     recordIndexesSize;                                                    // size of 'recordIndexes' buffer
  INT     recordSize;                                                           // size of record if length fixed
  INDEX   indexRecord,indexFile;
  INDEX   currentRecord;
  INDEX   lastRefRecord;
  INT     lastSavedRecord;
  INT     satelliteFlag;

  INT     refFlag;                                                              // this flag is set when the reference spectrum is retrieved from spectra files

  CALIB_FENO        calibFeno;                                                  // transfer of wavelength calibration options from the project mediator to the analysis mediator
 }
ENGINE_CONTEXT;

typedef struct _thrdRef
 {
  INDEX  indexRecord;
  double dist;
  double sza;
 }
THRD_REF;

// ----------------
// GLOBAL VARIABLES
// ----------------

EXTERN DoasCh     THRD_asciiFile[];             // ASCII file for exporting spectra
EXTERN HANDLE    THRD_hEvents[];               // list of events
EXTERN ENGINE_CONTEXT THRD_specInfo;           // data on current spectra and reference
EXTERN UINT      THRD_id;                      // thread identification number
EXTERN INT       THRD_levelMax;                // level of thread
EXTERN INT       THRD_lastEvent;               // last event
EXTERN DWORD     THRD_delay;                   // wait for next event
EXTERN INT       THRD_localShift;
EXTERN INT       THRD_correction;
EXTERN INT       THRD_browseType;
EXTERN INT       THRD_treeCallFlag;
EXTERN INT       THRD_increment;
EXTERN INT       THRD_isFolder;
EXTERN INT       THRD_recordLast;

// ----------
// PROTOTYPES
// ----------

double           THRD_GetDist(double longit, double latit, double longitRef, double latitRef);
RC               THRD_SpectrumCorrection(ENGINE_CONTEXT *pEngineContext,double *spectrum);
RC               THRD_CopySpecInfo(ENGINE_CONTEXT *pSpecInfoTarget,ENGINE_CONTEXT *pSpecInfoSource);
RC               THRD_NewRef(ENGINE_CONTEXT *pEngineContext);

// =====================
// STRUCTURES DEFINITION
// =====================

typedef struct _AnalyseLinearParameters
 {
 	DoasCh symbolName[MAX_ITEM_TEXT_LEN+1];
 	int polyOrder;
 	int baseOrder;
 	int storeFit;
 	int storeError;
 }
ANALYSE_LINEAR_PARAMETERS;

typedef struct _AnalyseNonLinearParameters
 {
 	DoasCh symbolName[MAX_ITEM_TEXT_LEN+1];
 	DoasCh crossFileName[MAX_ITEM_TEXT_LEN+1];
 	int fitFlag;
 	double initialValue;
 	double deltaValue;
 	double minValue;
 	double maxValue;
 	int storeFit;
 	int storeError;
 }
ANALYSE_NON_LINEAR_PARAMETERS;

typedef struct anlyswin_cross_section ANALYSIS_CROSS;
typedef struct anlyswin_shift_stretch ANALYSIS_SHIFT_STRETCH;
typedef struct anlyswin_gap ANALYSIS_GAP;
typedef struct anlyswin_output ANALYSIS_OUTPUT;

// -------------------
// GLOBAL DECLARATIONS
// -------------------

EXTERN INT    ANALYSE_plotKurucz,ANALYSE_plotRef,ANALYSE_indexLine;

EXTERN DoasCh *ANLYS_crossAction[ANLYS_CROSS_ACTION_MAX];
EXTERN DoasCh *ANLYS_amf[ANLYS_AMF_TYPE_MAX];

EXTERN PRJCT_FILTER *ANALYSE_plFilter,*ANALYSE_phFilter;
EXTERN WRK_SYMBOL   *WorkSpace;
EXTERN INT NWorkSpace,NDET;
EXTERN INT           DimC,DimL,DimP,Z,NFeno,(*Fenetre)[2],
                     SvdPDeb,SvdPFin;
EXTERN PRJCT_ANLYS  *pAnalysisOptions;             // analysis options
EXTERN PRJCT_KURUCZ *pKuruczOptions;               // Kurucz options
EXTERN PRJCT_SLIT   *pSlitOptions;                 // slit function options
EXTERN PRJCT_USAMP  *pUsamp;
EXTERN FENO         *TabFeno,*Feno;
EXTERN MATRIX_OBJECT ANALYSIS_slit,O3TD;
EXTERN double      **U,*x,*Lambda,
                    *ANALYSE_pixels,
                    *ANALYSE_splineX,              // abscissa used for spectra, in the units selected by user
                    *ANALYSE_splineX2,             // in pixels units, second derivatives of corresponding wavelengths
                    *ANALYSE_absolu,               // residual spectrum
                    *ANALYSE_secX,                 // residual spectrum + the contribution of a cross section for fit display
                    *ANALYSE_t,                    // residual transmission in Marquadt-Levenberg not linear method
                    *ANALYSE_tc,                   // residual transmission in Marquadt-Levenberg not linear method
                    *ANALYSE_xsTrav,
                    *ANALYSE_shift,
                    *ANALYSE_zeros,
                    *ANALYSE_ones,
                     ANALYSE_nFree,
                     ANALYSE_oldLatitude;

// ----------
// PROTOTYPES
// ----------

RC ANALYSE_Function ( double *lambda,double *X, double *Y, INT ndet, double *Y0, double *SigmaY, double *Yfit, int Npts,
              double *fitParamsC, double *fitParamsF );

enum _pixelSelection
 {
 	PIXEL_BEFORE,
 	PIXEL_AFTER,
 	PIXEL_CLOSEST
 };

RC   FNPixel   ( double *lambdaVector, double lambdaValue, INT npts,INT pixelSelection );

RC   ANALYSE_CheckLambda(WRK_SYMBOL *pWrkSymbol,double *lambda,DoasCh *callingFunction);
RC   ANALYSE_XsInterpolation(FENO *pTabFeno,double *newLambda);
RC   ANALYSE_XsConvolution(FENO *pTabFeno,double *newLambda,MATRIX_OBJECT *pSlit,INT slitType,double *slitParam1,double *slitParam2);
RC   ANALYSE_LinFit(SVD *pSvd,INT Npts,INT Degree,double *a,double *sigma,double *b,double *x);
void ANALYSE_SvdFree(DoasCh *callingFunctionShort,SVD *pSvd);
RC   ANALYSE_SvdLocalAlloc(DoasCh *callingFunctionShort,SVD *pSvd);
RC   ANALYSE_SvdInit(SVD *pSvd);
RC   ANALYSE_CurFitMethod(double *Spectre,double *SigmaSpec,double *Sref,double *Chisqr,INT *pNiter,double speNormFact,double refNormFact);
void ANALYSE_ResetData(void);
RC   ANALYSE_SetInit(ENGINE_CONTEXT *pEngineContext);
RC   ANALYSE_AlignReference(ENGINE_CONTEXT *pEngineContext,INT refFlag,INT saveFlag,void *responseHandle);
RC   ANALYSE_Spectrum(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

void ANALYSE_SetAnalysisType(void);
RC   ANALYSE_LoadRef(ENGINE_CONTEXT *pEngineContext);
RC   ANALYSE_LoadCross(ENGINE_CONTEXT *pEngineContext,ANALYSIS_CROSS *crossSectionList,INT nCross,INT hidden,double *lambda);
RC   ANALYSE_LoadLinear(ANALYSE_LINEAR_PARAMETERS *linearList,INT nLinear);
RC   ANALYSE_LoadNonLinear(ENGINE_CONTEXT *pEngineContext,ANALYSE_NON_LINEAR_PARAMETERS *nonLinearList,INT nNonLinear,double *lambda);
RC   ANALYSE_LoadShiftStretch(ANALYSIS_SHIFT_STRETCH *shiftStretchList,INT nShiftStretch);
RC   ANALYSE_LoadGaps(ENGINE_CONTEXT *pEngineContext,ANALYSIS_GAP *gapList,INT nGaps,double *lambda,double lambdaMin,double lambdaMax);
RC   ANALYSE_LoadOutput(ANALYSIS_OUTPUT *outputList,INT nOutput);
RC   ANALYSE_LoadSlit(PRJCT_SLIT *pSlit);

RC   ANALYSE_Alloc(void);
void ANALYSE_Free(void);

RC   ANALYSE_UsampBuild(INT analysisFlag,INT gomeFlag);
void ANALYSE_UsampGlobalFree(void);
RC   ANALYSE_UsampGlobalAlloc(double lambdaMin,double lambdaMax,INT size);
RC   ANALYSE_UsampLocalAlloc(INT gomeFlag);
void ANALYSE_UsampLocalFree(void);

// ============================
// KURUCZ.C : Kurucz procedures
// ============================

// ---------------------
// STRUCTURES DEFINITION
// ---------------------

typedef struct _KuruczFeno
 {
  SVD            *svdFeno;                              // svd environments associated to list of little windows
  double         *Grid;
  FFT            *fft;                                  // fourier transform of high resolution kurucz spectrum
  CROSS_RESULTS **results;
  double         *chiSquare;
  double         *rms;
  int            *nIter;
 }
KURUCZ_FENO;

typedef struct _Kurucz
 {
  KURUCZ_FENO *KuruczFeno;
  XS      hrSolar;                              // high resolution kurucz spectrum for convolution
  SVD     svdFwhm;                              // svd matrix used for computing coefficients of polynomial fitting fwhm
  double *solar,                                // convoluted kurucz spectrum
         *lambdaF,
         *solarF,                               // filtered solar spectrum (high pass filtering)
         *solarF2,                              // second derivatives for the previous vector
         *offset,
         *fwhmVector[MAX_KURUCZ_FWHM_PARAM],    // wavelength dependence of fwhm
         *fwhmDeriv2[MAX_KURUCZ_FWHM_PARAM],    // wavelength dependence of fwhm
         *VPix,*VSig,*Pcalib,                   // polynomial coefficients computation
         *pixMid,*VLambda,*VShift,              // display
         *fwhm[MAX_KURUCZ_FWHM_PARAM],          // fwhm found for each little window
         *fwhmSigma[MAX_KURUCZ_FWHM_PARAM],     // errors on fwhm
         *fwhmPolySpec[MAX_KURUCZ_FWHM_PARAM];  // polynomial coefficients for building wavelength dependence of fwhm for spectra
  INT    *NIter;                                // number of iterations

  MATRIX_OBJECT crossFits;                      // cross sections fits to display

  INT     Nb_Win,                               // number of little windows
          shiftDegree,                          // degree of the shift polynomial
          fwhmDegree,                           // degree of the fwhm polynomial
          solarFGap;

  INDEX   indexKurucz;                          // index of analysis window with Kurucz description

  DoasCh   displayFit;                           // display fit flag
  DoasCh   displayResidual;                      // display new calibration flag
  DoasCh   displayShift;                         // display shift in each pixel flag
  DoasCh   displaySpectra;                       // display complete spectra
  DoasCh   method;                               // analysis method (Marquadt,SVD)
  DoasCh   units;                                // units selected for analysis (pixel/nm)
 }
KURUCZ;

// -------------------
// GLOBAL DECLARATIONS
// -------------------

EXTERN KURUCZ KURUCZ_buffers;
EXTERN FFT *pKURUCZ_fft;

// ----------
// PROTOTYPES
// ----------

RC   KURUCZ_Spectrum(double *oldLambda,double *newLambda,double *spectrum,double *reference,double *instrFunction,
                     DoasCh displayFlag,DoasCh *windowTitle,double **coeff,double **fwhmVector,double **fwhmDeriv2,INT saveFlag,INDEX indexFeno,void *responseHandle);
RC   KURUCZ_ApplyCalibration(FENO *pTabFeno,double *newLambda);
RC   KURUCZ_Reference(double *instrFunction,INDEX refFlag,INT saveFlag,INT gomeFlag,void *responseHandle);
void KURUCZ_Init(INT gomeFlag);
RC   KURUCZ_Alloc(PROJECT *pProject,double *lambda,INDEX indexKurucz,double lambdaMin,double lambdaMax);
void KURUCZ_Free(void);

// ==================================
// USAMP.C : undersampling correction
// ==================================

typedef struct _usamp
 {
  XS       hrSolar;
  INT     *lambdaRange[4];                       // for each analysis window, give the lambda range
  double **kuruczConvoluted,                     // high resolution kurucz convoluted on its own calibration
         **kuruczConvoluted2,                    // second derivatives of previous vector
         **kuruczInterpolated,                   // high resolution and convoluted kurucz on analysis windows calibrations
         **kuruczInterpolated2;                  // second derivatives of previous vectors
 }
USAMP;

RC USAMP_BuildCrossSections(double *phase1,                                     // OUTPUT : phase 1 calculation
                            double *phase2,                                     // OUTPUT : phase 2 calculation
                            double *gomeLambda,                                 // GOME calibration
                            double *gomeLambda2,                                // shifted GOME calibration
                            double *kuruczInterpolated,                         // preconvoluted Kurucz spectrum interpolated on gome calibration
                            double *kuruczInterpolatedDeriv2,                   // interpolated Kurucz spectrum second derivatives
                            INT     nGome,                                      // size of GOME calibration
                            double *kuruczLambda,                               // Kurucz high resolution wavelength scale
                            double *kuruczConvolved,                            // preconvoluted Kurucz spectrum on high resolution wavelength scale
                            double *kuruczConvolvedDeriv2,                      // preconvoluted Kurucz second derivatives
                            INT     nKurucz,                                    // size of Kurucz vectors
                            INT     analysisMethod);                            // analysis method

RC USAMP_Build(double *phase1,                                                  // OUTPUT : phase 1 calculation
               double *phase2,                                                  // OUTPUT : phase 2 calculation
               double *gomeLambda,                                              // GOME calibration
               INT     nGome,                                                   // size of GOME calibration
               double *kuruczLambda,                                            // Kurucz calibration
               double *kuruczSpectrum,                                          // Kurucz spectrum
               double *kuruczDeriv2,                                            // Kurucz second derivatives
               INT     nKurucz,                                                 // size of Kurucz vectors
               SLIT   *pSlit,                                                   // slit function
               double  fraction,                                                // tunes the phase
               INT     analysisMethod);                                         // analysis method

// ======
// OUTPUT
// ======

// ---------------------
// CONSTANTS DEFINITIONS
// ---------------------

#define     ASC_EXT  "ASC"
#define     NASA_EXT "NSA"

#define     NASA_AMES_HEADER_FILE "NASAAMES.HDR"

#define     MAX_FLUXES    20
#define     MAX_CIC       20
#define     MAX_RESULTS  500   // 250 measurements the morning; 250 measurements the afternoon.
#define     MAX_FIELDS   1600

// ----------------------
// STRUCTURES DEFINITIONS
// ----------------------

// Air Mass Factors (AMF) table cross reference
// --------------------------------------------

typedef struct _amfReference
 {
  DoasCh    type,                          // type of symbol
           symbolName[MAX_STR_LEN+1],      // name of symbol
           amfFileName[MAX_STR_LEN+1];     // name of AMF file
  double **Phi;                           // AMF data
  double **deriv2;                        // AMF second derivatives for spline calculations
  double **xs;                            // cross sections
  double **xsDeriv2;                      // cross sections second derivatives
  INT      PhiLines,                      // number of lines in Phi matrix
           PhiColumns,                    // number of columns in Phi matrix
           xsLines,                       // number of lines in Param matrix
           xsColumns;                     // number of columns in Param matrix
 }
AMF_SYMBOL;

// Data relative to components to take into account for NASA-AMES results
// ----------------------------------------------------------------------

typedef struct _nasaComponents

 {
  DoasCh   symbolName[MAX_ITEM_NAME_LEN+1];                                           // name of component
  double  SlntColFact,SlntErrFact,VrtColFact,AmfFact,                                // scaling factors
          ResCol;                                                                    // residual column in reference spectrum
  INDEX   indexWindow,                                                               // index of analysis window if used
          indexTabCross;                                                             // index of symbol in cross reference
  double *SlntCol,*Error,*Amf;                                                       // primary results : resp. slant column, error and AMF
 }
NASA_COMPONENTS;

// Results in NASA-AMES format safe keeping
// ----------------------------------------

typedef struct _nasaResults

 {
  PROJECT project;
  NASA_COMPONENTS components[PRJCT_RESULTS_NASA_MAX];                                // components to take into account for NASA-AMES results
  INT             julianDay,                                                         // current julian day
                  oldJulianDay,                                                      // previous julian day
                  amNResults,pmNResults;                                             // number of AM/PM results
  double          refZm,                                                             // reference zenithal angle
                 *Zm,*Tm,*TDet,*TimeDec,*Cic;                                        // auxiliary results
  char            SkyObs;                                                            // sky state indication
 }
NASA_RESULTS;

// -------------------
// GLOBAL DECLARATIONS
// -------------------

EXTERN DoasCh       OUTPUT_refFile[MAX_PATH_LEN+1];
EXTERN INT         OUTPUT_nRec;
EXTERN DoasCh       OUTPUT_nasaFile[MAX_STR_LEN+1];          // name of file with NASA-AMES header description
EXTERN INT         OUTPUT_nasaNResults;                     // number of results stored in NASA-AMES structure
EXTERN AMF_SYMBOL *OUTPUT_AmfSpace;                         // list of cross sections with associated AMF file

// ----------
// PROTOTYPES
// ----------

RC   OUTPUT_GetWveAmf(CROSS_RESULTS *pResults,double Zm,double *lambda,double *xs,double *deriv2);
RC OUTPUT_ReadAmf(DoasCh *symbolName,DoasCh *amfFileName,DoasCh amfType,INDEX *pIndexAmf);

void OUTPUT_ResetData(void);
RC   OUTPUT_RegisterData(ENGINE_CONTEXT *pEngineContext);
RC   OUTPUT_FlushBuffers(ENGINE_CONTEXT *pEngineContext);

RC   OUTPUT_SaveNasaAmes(void);
RC   OUTPUT_SaveResults(ENGINE_CONTEXT *pEngineContext);

RC   OUTPUT_LocalAlloc(ENGINE_CONTEXT *pEngineContext);
RC   OUTPUT_Alloc(void);
void OUTPUT_Free(void);

// =======================
// GOME SPECTRA PROCESSING
// =======================

// --------------------
// CONSTANTS DEFINITION
// --------------------

#define BAND_NUMBER                    6                                        // number of bands to display
#define SCIENCE_DATA_DEFINED          10                                        // number of bands
#define SPECTRAL_FITT_ORDER            5                                        // degree of polynomial used for building wavelength scale
#define	MAX_FITT_ORDER	                6
#define GDP_BIN_ERROR_ID_MASK       0x01

enum _gomeGetCommand
 {
  GOME_GET_LEMBDA,
  GOME_GET_SPECTRUM,
  GOME_GET_REFERENCE
 };

// --------------------
// STRUCTURE DEFINITION
// --------------------

// Date in short format

typedef struct _SHORTDateTime
 {
  char  da_year;        /* Year - 1980      */
  char  da_day;         /* Day of the month */
  char  da_mon;         /* Month (1 = Jan)  */
  char  ti_hour;
  char  ti_min;
  char  ti_sec;
 }
SHORT_DATETIME;

// File header

typedef struct _gomeFileHeader
 {
  unsigned short nspectra;                    // total number of spectra in file
  char           version;
  char           mask;
  int            headerSize;                  // number of bytes before first record
  int            recordSize;                  // size of a record
  int            orbitNumber;                 // orbit number
  short          nbands;                      // number of available bands
  char           nSpectralParam;              // number of set of spectral parameters
  char           indexSpectralParam;          // index of set of spectral parameters for irradiance spectra
  SHORT_DATETIME dateAndTime;                 // measurement date and time in UT
 }
GDP_BIN_FILE_HEADER;

// For each available band, provide header with band and reference general info

typedef struct _gomeBandHeader
 {
  // Band info

  char       bandType;                        // band type
  short      bandSize;                        // band size

  // Reference info

  short      startDetector;                   // index of first pixel used on detector for this ban
  float      scalingFactor;
  float      scalingError;
 }
GDP_BIN_BAND_HEADER;

// Spectrum record structure

typedef struct                            // geolocation coordinates version 3
 {
  DoasUS          lonArray[5];            // longitude array
  short           latArray[5];            // latitude array
  float           szaArray[3];            // zenithal array
  DoasUS          losZa[3];               // line of sight zenithal array
  DoasUS          losAzim[3];             // line of sight azimuthal array
  float           satHeight;              // satellite geodetic height at point B
  float           radiusCurve;            // Earth radius curvatur at point B

  // From Level 2 data

  unsigned short  o3;                     // O3 VCD
  unsigned short  no2;                    // NO2 VCD
  unsigned short  cloudFraction;          // Cloud fraction
  unsigned short  cloudTopPressure;       // Cloud top pressure
  float           aziArray[3];
  unsigned short  unused[4];             // for later new data ?
 }
GEO_3;

// Cloud information : new in version 4.00, may 2009

typedef struct
 {
	 float SurfaceHeight;
	 float SurfaceAlbedo;
	 float UV_Albedo;
	 int Elevation;
	 float CloudFraction[2]; /* Cloud Fraction and error */
	 float CloudTopAlbedo[2]; /* Cloud Top Albedo and error */
	 float CloudTopHeight[2]; /* Cloud Top Height and error */
	 float TAU[2]; /* Cloud Optical Thickness and error */
	 float CTP[2]; /* Cloud Top Pressure and error */
	 short Mode; /* 0=normal, 1=snow/ice */
	 short Type; /* 1=Cirrus, 2=Cirrostratus, 3=Deep convection, 4=Altocumulus, 5=Altostratus, etc */
 }
GDP_BIN_CLOUD_HEADER;

typedef struct                            // geolocation coordinates version 4.00 from May 2009
 {
  unsigned short       lonArray[5];       // longitude array
  short                latArray[5];       // latitude array
  float                szaArrayTOA[3];    // solar zenithal angles, top of atmosphere
  float                aziArrayTOA[3];    // solar azimuth angles, top of atmosphere
  float                losZaTOA[3];       // line of sight zenithal angles, top of atmosphere
  float                losAzimTOA[3];     // line of sight azimuth angles, top of atmosphere
  float                szaArrayBOA[3];    // solar zenithal angles, bottom of atmosphere
  float                aziArrayBOA[3];    // solar azimuth angles, bottom of atmosphere
  float                losZaBOA[3];       // line of sight zenithal angles, bottom of atmosphere
  float                losAzimBOA[3];     // line of sight azimuth angles, bottom of atmosphere
  float                satHeight;         // satellite geodetic height at point B
  float                radiusCurve;       // Earth radius curvature at point B
  GDP_BIN_CLOUD_HEADER cloudInfo;
 }
GEO_4;

typedef struct
 {
  // From Level 1 data

  SHORT_DATETIME  dateAndTime;                // measurement date and time in UT
  short           groundPixelID;              // ground pixel order
  char            groundPixelType;            // ground pixel type
  char            indexSpectralParam;         // index of set of spectral parameters in reference record to use for building calibration
  union _geo
   {
    GEO_3 geo3;
    GEO_4 geo4;
   }
  geo;
 }
SPECTRUM_RECORD;

// Keep useful information for fast access to a record

typedef struct _gome_recordInfo
 {
  INDEX  pixelNumber;                                                           // pixel number
  INDEX  pixelType;                                                             // pixel type
  double lat;                                                                   // latitude
  double lon;                                                                   // longitude
  double sza;                                                                   // solar zenith angle
 }
GDP_BIN_INFO;

typedef struct _GOMEOrbitFiles                                                  // description of an orbit
 {
 	DoasCh gdpBinFileName[MAX_STR_LEN+1];                                            // the name of the file with a part of the orbit
 	DoasCh gdpBinFileNumber[9];
  GDP_BIN_INFO *gdpBinInfo;                                                     // useful information on records for fast access
  INDEX gdpBinBandIndex;                                                        // indexes of bands present in the current file
  INDEX *gdpBinLatIndex,*gdpBinLonIndex,*gdpBinSzaIndex,*gdpBinPixIndex;        // indexes of records sorted resp. by latitudes, by SZA or by pixel number
  GDP_BIN_FILE_HEADER gdpBinHeader;
  GDP_BIN_BAND_HEADER gdpBinBandInfo[SCIENCE_DATA_DEFINED];
  SPECTRUM_RECORD     gdpBinSpectrum;
  int                 gdpBinSpectraSize,                                        // total size of spectra vector GDP_BIN_coeff
                      gdpBinCoeffSize,                                          // number of polynomial coefficients in vector
                      gdpBinStartPixel[SCIENCE_DATA_DEFINED];                   // starting pixels for bands present in the file
  double             *gdpBinCoeff;                                              // coefficients for reconstructing wavelength calibrations
  float               gdpBinScalingFactor[SCIENCE_DATA_DEFINED],                // scaling factors for spectra band per band
                      gdpBinScalingError[SCIENCE_DATA_DEFINED];                 // scaling factors for errors band per band
  DoasUS             *gdpBinReference,                                          // buffer for irradiance spectra
                     *gdpBinRefError;                                           // errors on irradiance spectra
  INT                 specNumber;
  RC rc;
 }
GOME_ORBIT_FILE;

// ---------------------
// VARIABLES DECLARATION
// ---------------------

#define MAX_GOME_FILES 50 // maximum number of files per orbit

EXTERN GOME_ORBIT_FILE GDP_BIN_orbitFiles[MAX_GOME_FILES];                      // list of files for an orbit
EXTERN INDEX GDP_BIN_currentFileIndex;                                          // index of the current file in the list
EXTERN DoasCh *GDP_BIN_BandStrTab[];

// ----------
// PROTOTYPES
// ----------

INDEX            GDP_BIN_GetRecordNumber(INT pixelNumber);
RC               GDP_BIN_GetBand(ENGINE_CONTEXT *pEngineContext,INT bandNo);
void             GDP_BIN_GetReferenceInfo(ENGINE_CONTEXT *pEngineContext);
RC               GDP_BIN_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,FILE *specFp,void *responseHandle);

// ==============
// FILES READ OUT
// ==============

EXTERN DoasCh *CCD_measureTypes[];
EXTERN DoasCh *MFCBIRA_measureTypes[];

RC   SetUofT(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliUofT(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,INT localDay,FILE *specFp);
RC   SetNOAA(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliNOAA(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,INT localDay,FILE *specFp);
RC   SetCCD_EEV(ENGINE_CONTEXT *pEngineContext,FILE *specFp,FILE *darkFp);
RC   ReliCCD_EEV(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *darkFp);
RC   SetCCD (ENGINE_CONTEXT *pEngineContext,FILE *specFp,INT flag);
RC   ReliCCD(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp);
RC   ReliCCDTrack(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp);
RC   SetPDA_EGG(ENGINE_CONTEXT *pEngineContext,FILE *specFp,int newFlag);
RC   ReliPDA_EGG(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp,int newFlag);
RC   SetPDA_EGG_Logger(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliPDA_EGG_Logger(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);

RC   SetEASOE(ENGINE_CONTEXT *pEngineContext,FILE *specFp,FILE *namesFp);
RC   ReliEASOE(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp);

// SAOZ (CNRS, France)

RC   SetSAOZ(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliSAOZ(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp);
RC   MKZY_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   MKZY_Reli(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,INT localDay,FILE *specFp);
RC   AIRBORNE_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   AIRBORNE_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,INT localDay,FILE *specFp);
RC   SetSAOZEfm(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliSAOZEfm(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
RC   SetActon_Logger(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliActon_Logger(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp);
RC   SetOceanOptics(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliOceanOptics(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
RC   ASCII_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ASCII_Read(ENGINE_CONTEXT *pEngineContext,INT recordNo,INT dateFlag,int localDay,FILE *specFp);
RC   SetRAS(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliRAS(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);

void SCIA_ReleaseBuffers(DoasCh format);
RC   SCIA_SetPDS(ENGINE_CONTEXT *pEngineContext);
RC   SCIA_ReadPDS(ENGINE_CONTEXT *pEngineContext,int recordNo);
INDEX SCIA_GetRecordNumber(INT hdfRecord,INT obsNumber);
RC SCIA_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

enum _omiSwathType { OMI_SWATH_UV1, OMI_SWATH_UV2, OMI_SWATH_VIS, OMI_SWATH_MAX };
enum _omiVdataType { OMI_VDATA_GEO, OMI_VDATA_DATA, OMI_VDATA_ATTR, OMI_VDATA_MAX };
enum _omiSpecType  { OMI_SPEC_IRRAD, OMI_SPEC_RAD };

// Geolocation fields

typedef struct _omi_geo
 {
  double         *time;
  float          *secondsInDay;
  float          *spacecraftLatitude;
  float          *spacecraftLongitude;
  float          *spacecraftAltitude;
  float          *latitude;
  float          *longitude;
  float          *solarZenithAngle;
  float          *solarAzimuthAngle;
  float          *viewingZenithAngle;
  float          *viewingAzimuthAngle;
  short          *terrainHeight;
  unsigned short *groundPixelQualityFlags;
 }
OMI_GEO;

// Data fields

typedef struct _omi_spectrum
 {
  short  *mantissa;
  short  *precisionMantissa;
  char   *exponent;
  DoasUS *pixelQualityFlags;
  float  *wavelengthCoefficient;
  float  *wavelengthCoefficientPrecision;
 }
OMI_SPECTRUM;

typedef struct _omi_data_fields
 {
 	short   *wavelengthReferenceColumn;
  DoasCh   *measurementClass;
  DoasCh   *instrumentConfigurationId;
  DoasCh   *instrumentConfigurationVersion;
  DoasUS  *measurementQualityFlags;
  char    *numberSmallPixelColumns;
  char    *exposureType;
  float   *masterClockPeriod;
  DoasUS  *calibrationSettings;
  float   *exposureTime;
  float   *readoutTime;
  short   *smallPixelColumn;
  short   *gainSwitchingColumn1;
  short   *gainSwitchingColumn2;
  short   *gainSwitchingColumn3;
  char    *gainCode1;
  char    *gainCode2;
  char    *gainCode3;
  char    *gainCode4;
  char    *dSGainCode;
  char    *lowerStrayLightAreaBinningFactor;
  char    *upperStrayLightAreaBinningFactor;
  char    *lowerDarkAreaBinningFactor;
  char    *upperDarkAreaBinningFactor;
  short   *skipRows1;
  short   *skipRows2;
  short   *skipRows3;
  short   *skipRows4;
  float   *detectorTemperature;
  float   *opticalBenchTemperature;
  char    *imageBinningFactor;
  short   *binnedImageRows;
  short   *stopColumn;
 }
OMI_DATA_FIELDS;

// Swath attributes

typedef struct _omi_swath_attr
 {
  int32_t  numTimes;
  int32_t  numTimesSmallPixel;
  float    earthSunDistance;
 }
OMI_SWATH_ATTR;

// Definition of a swath
//     - UV-1 Swath
//     - UV-2 Swath
//     - VIS Swath

typedef struct _omi_swath_earth
 {
  OMI_GEO         geolocationFields;
  OMI_SPECTRUM    spectrum;
  OMI_DATA_FIELDS dataFields;
  OMI_SWATH_ATTR  swathAttributes;
 }
OMI_SWATH;

extern DoasCh *OMI_swaths[OMI_SWATH_MAX];

RC   OMI_GetEarthSwath(OMI_SWATH *pSwath,DoasCh *omiFileName,int omiFileId,int earthSwath,int swathType);
void OMI_ReleaseBuffers(void);
RC   OMI_SetHDF(ENGINE_CONTEXT *pEngineContext);
RC   OMI_ReadHDF(ENGINE_CONTEXT *pEngineContext,int recordNo);

RC   CCD_LoadInstrumental(ENGINE_CONTEXT *pEngineContext);
void CCD_ResetInstrumental(CCD *pCCD);

typedef struct _TOldFlags
 {
  INT   mode;
  INT   smooth;
  INT   deg_reg;
  DoasCh Null[8];
  DoasCh Ref[8];
 }
TOldFlags;

typedef struct _TBinaryMFC
 {
  DoasCh     version[20];           //     version number (not of interest)
  INT       no_chan;               // !!! number of channels - 1 (usually 1023)
  void     *Spectrum;              //     pointer to the spectrum, only used at runtime
  DoasCh     specname[20];          //     optional name of the spectrum
  DoasCh     site[20];              //     name of measurement site
  DoasCh     spectroname[20];       //     name of spectrograph
  DoasCh     scan_dev[20];          //     name of scan device, e.g. PDA
  DoasCh     first_line[80];
  float     elevation;             //     elevation viewing angle
  DoasCh     spaeter[72];
  INT       ty;                    //     spectrum flags, can be used to distinguish between
                                   //     different types of spectrum (e.g. straylight,
                                   //     offset, dark current...
  DoasCh     dateAndTime[28];
//  DoasCh     date[9];               // !!! date of measurement
//  DoasCh     start_time[9];         // !!! start time of measurement
//  DoasCh     stop_time[9];          // !!! stop time of measurement
//  DoasCh     dummy;
  INT       low_lim;
  INT       up_lim;
  INT       plot_low_lim;
  INT       plot_up_lim;
  INT       act_chno;
  INT       noscans;               // !!! number of scans added in this spectrum
  float     int_time;              // !!! integration time in seconds
  float     latitude;              //     latitude of measurement site
  float     longitude;             //     longitude of measurement site
  INT       no_peaks;
  INT       no_bands;
  float     min_y;                 //     minmum of spectrum
  float     max_y;                 //     maximum of spectrum
  float     y_scale;
  float     offset_Scale;
  float     wavelength1;           // !!! wavelength of channel 0
  float     average;               //     average signal of spectrum
  float     dispersion[3];         // !!! dispersion given as a polynome:
                                   //     wavelength=wavelength1 + dispersion[0]*C + dispersion[1]*C^2
                                   //                            + dispersion[2]*C^3;   C: channel number
                                   //                              (0..1023)
  float     opt_dens;
  TOldFlags OldFlags;
  DoasCh     FileName[8];           //     filename of spectrum
  DoasCh     backgrnd[8];
  INT       gap_list[40];
  DoasCh    *comment;
  INT       reg_no;
  void     *Prev, *Next;
 }
TBinaryMFC;

extern TBinaryMFC MFC_headerDrk,MFC_headerOff,MFC_header,MFC_headerInstr;
extern int MFC_format;
extern DoasCh MFC_fileInstr[MAX_STR_SHORT_LEN+1],
             MFC_fileDark[MAX_STR_SHORT_LEN+1],
             MFC_fileOffset[MAX_STR_SHORT_LEN+1],
             MFC_fileSpectra[MAX_STR_SHORT_LEN+1],
             MFC_fileMin[MAX_STR_SHORT_LEN+1];

RC MFC_LoadOffset(ENGINE_CONTEXT *pEngineContext);
RC MFC_LoadDark(ENGINE_CONTEXT *pEngineContext);
RC MFC_ReadRecord(DoasCh *fileName,TBinaryMFC *pHeaderSpe,double *spe,TBinaryMFC *pHeaderDrk,double *drk,TBinaryMFC *pHeaderOff,double *off,UINT mask,UINT maskSpec,UINT revertFlag);
RC MFC_ReadRecordStd(ENGINE_CONTEXT *pEngineContext,DoasCh *fileName,
                     TBinaryMFC *pHeaderSpe,double *spe,
                     TBinaryMFC *pHeaderDrk,double *drk,
                     TBinaryMFC *pHeaderOff,double *off);
INDEX MFC_SearchForCurrentFileIndex(ENGINE_CONTEXT *pEngineContext)  ;
RC   SetMFC(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliMFC(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,UINT mfcMask);
RC   ReliMFCStd(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
RC   MFCBIRA_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   MFCBIRA_Reli(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);

RC MFC_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);


RC MKZY_SearchForSky(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC MKZY_SearchForOffset(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC MKZY_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

double EvalPolynom_d(double X, const double *Coefficient, short Grad);

extern double *GDP_refL,*GDP_ref,*GDP_refE;

void GDP_ASC_ReleaseBuffers(void);
RC   GDP_ASC_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   GDP_ASC_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,FILE *specFp);
RC   GDP_ASC_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,FILE *specFp);

void GDP_BIN_ReleaseBuffers(void);
RC   GDP_BIN_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   GDP_BIN_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,FILE *specFp,INDEX indexFile);

// GOME2

extern int GOME2_beatLoaded;

void GOME2_ReleaseBuffers(void);

RC GOME2_Set(ENGINE_CONTEXT *pEngineContext);
RC GOME2_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,INDEX fileIndex);
RC GOME2_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

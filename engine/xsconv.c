
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  THE BIRA-IASB DOAS SOFTWARE FOR WINDOWS AND LINUX
//  Module purpose    :  CROSS SECTIONS CONVOLUTION TOOLS
//  Name of module    :  XSCONV.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in WinDOAS package in June 97
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
//  FUNCTIONS
//
//  =====================
//  SUPPORTED LINE SHAPES
//  =====================
//
//  XsconvFctGauss - gaussian function;
//  XsconvFctApod - apodisation function (FTS);
//  XsconvFctApodNBS - apodisation function Norton Beer Strong (FTS);
//
//  XsconvFctBuild - build a line shape in a vector;
//
//  ======================
//  TAB CONTROL PROCESSING
//  ======================
//
//  XsconvTabChange - TCN_SELCHANGE notification message processing;
//  XsconvTabReSize - resize pages in tab control;
//
//  ===============
//  DATA PROCESSING
//  ===============
//
//  XSCONV_Reset - release buffers;
//  XSCONV_Alloc - allocate buffers involved in convolution;
//  XsconvGetFwhm - get slit function full width half way up;
//
//  =====================
//  CONVOLUTION FUNCTIONS
//  =====================
//
//  XSCONV_TypeNone - apply no convolution, interpolation only;
//  XSCONV_TypeGauss - gaussian convolution with variable half way up width;
//  XSCONV_TypeStandard - standard convolution of cross section with a slit function;
//  XSCONV_RealTimeXs - real time cross sections convolution;
//
//  XsconvTypeI0Correction - convolution of cross sections with I0 correction;
//  XsconvRebuildSlitFunction - rebuild slit function onto a regular wavelength scale;
//  XsconvPowFFTMin - return index of the first minimum found in the power spectrum obtained by FFT;
//  XsconvFreqFilter - frequencies filtering;
//  XsconvNewSlitFunction - replace slit function by a new one when a deconvolution slit function is given;
//
//  XSCONV_Convolution - main convolution function;
//
//  ================
//  FILES PROCESSING
//  ================
//
//  XSCONV_FileSelection - file selection;
//  XSCONV_LoadCalibrationFile - final wavelength scale read out;
//  XSCONV_LoadSlitFunction - slit function read out;
//  XSCONV_LoadCrossSectionFile - load a cross section file;
//
//  XsconvSaveNew - save the convoluted cross section;
//
//  ==========================================
//  CONVOLUTION DIALOG BOX MESSAGES PROCESSING
//  ==========================================
//
//  GENERAL TAB PAGE MESSAGES PROCESSING
//  ------------------------------------
//
//  XsconvGeneralKurucz - enable/Disable Kurucz fields on convolution type;
//  XsconvGeneralInit - WM_INIT message processing;
//  XsconvGeneralCommand - WM_COMMAND message processing;
//  XsconvGeneralDestroy - WM_DESTROY message processing;
//
//  XSCONV_GeneralWndProc - dispatch messages from the page related to general options;
//
//  SLIT FUNCTION TAB PAGE MESSAGES PROCESSING
//  ------------------------------------------
//
//  XsconvSlitInit - WM_INIT message processing;
//  XsconvSlitCommand - WM_COMMAND message processing;
//  XsconvSlitDestroy - WM_DESTROY message processing;
//
//  XSCONV_SlitWndProc - dispatch messages from the page related to the slit function;
//  XSCONV_SlitType - according to the selected slit function, show/hide slit options;
//
//  CONVOLUTION DIALOG BOX MESSAGES PROCESSING
//  ------------------------------------------
//
//  XsconvInit - initialization of the convolution tools dialog box (WM_INITDIALOG);
//  XsconvNotify - WM_NOTIFY message processing;
//  XsconvOK- validate the convolution options on OK command;
//  XsConvHelp - dispatch help messages from the convolution tools dialog box;
//  XsconvCommand - dispatch command messages from the convolution tools dialog box;
//
//  XSCONV_WndProc - dispatch messages from the convolution tools dialog box;
//
//  ==================
//  CONFIGURATION FILE
//  ==================
//
//  XSCONV_ResetConfiguration- initialize the convolution options to default values;
//  XSCONV_LoadConfiguration - load last used information in the convolution tool box from the wds configuration file;
//  XSCONV_SaveConfiguration - save the last information from the convolution tool box in the wds configuration file.
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"

// =====================
// CONSTANTS DEFINITIONS
// =====================

#define XSCONV_SECTION "Convolution"

// Slit types

UCHAR *XSCONV_slitTypes[SLIT_TYPE_MAX]=
 {
// !!!  	"None",
  "File",
  "Gaussian",
  "2n-Lorentz",
  "Voigt",
  "Error function",
  "Boxcar (FTS)",
  "Norton Beer Strong (FTS)",
  "Gaussian, wavelength dependent",
  "2n-Lorentz, wavelength dependent",
//  "Voigt, wavelength dependent",
  "Error function, wavelength dependent",
  "Gaussian, wavelength+t� dependent",
  "Error function, wavelength+t� dependent"
 };

XSCONV XSCONV_options,XSCONV_buffer;

// ================
// STATIC VARIABLES
// ================

// Tab pages description

// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? static TAB_PAGE xsconvTabPages[TAB_TYPE_XSCONV_MAX]=
// QDOAS ???  {
// QDOAS ???   { "General",       DLG_TOOL_CONVOLUTION_GENERAL, (DLGPROC) XSCONV_GeneralWndProc, NULL},   // General data
// QDOAS ???   { "Slit function", DLG_TOOL_CONVOLUTION_SLIT,    (DLGPROC) XSCONV_SlitWndProc,    NULL},   // Slit function
// QDOAS ???   { "Filtering"    , DLG_PRJCT_FILTER,             (DLGPROC) PRJCT_FilterWndProc,   NULL}    // Filter function
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? static HWND xsconvHwndPage=NULL;                                                // handle to the current page
// QDOAS ???
// QDOAS ??? static HCURSOR xsconvHCurHourGlass,                                             // handle to the hour glass cursor (when waiting for an action comleted)
// QDOAS ???                xsconvHCurSave;                                                  // handle to the old cursor before the hour glass take place
// QDOAS ??? #endif

// QDOAS ??? // Convolution method
// QDOAS ???
// QDOAS ??? static UCHAR *xsconvConvolutionTypesStr[CONVOLUTION_TYPE_MAX]=
// QDOAS ???  {
// QDOAS ???   "Interpolation only",
// QDOAS ???   "Standard convolution",
// QDOAS ???   "Convolution with I0 correction"
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? static UCHAR *xsconvConvolutionTypes[CONVOLUTION_TYPE_MAX]=
// QDOAS ???  {
// QDOAS ???   "None",
// QDOAS ???   "Standard",
// QDOAS ???   "I0 correction"
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? // Conversion modes
// QDOAS ???
// QDOAS ??? static UCHAR *xsconvConversionModes[CONVOLUTION_CONVERSION_MAX]=
// QDOAS ???  {
// QDOAS ???   "None",
// QDOAS ???   "Air to Vac",
// QDOAS ???   "Vac to Air"
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? // Files extensions
// QDOAS ???
// QDOAS ??? static UCHAR *xsconvFileExt[CONVOLUTION_TYPE_MAX]=
// QDOAS ???  {
// QDOAS ???   "_none",                                                                      // CONVOLUTION_TYPE_NONE
// QDOAS ???   "_std",                                                                       // CONVOLUTION_TYPE_STANDARD
// QDOAS ???   "_i0",                                                                        // CONVOLUTION_TYPE_I0_CORRECTION
// QDOAS ??? //  "_ring"                         // CONVOLUTION_TYPE_RING
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? static XS  XSCONV_slitFunction,                                                 // slit function used for convolution
// QDOAS ???            XSCONV_slitDFunction,                                                // slit function used for deconvolution
// QDOAS ???            XSCONV_xshr,                                                         // high resolution cross section
// QDOAS ???            XSCONV_xsnew,                                                        // convoluted cross section
// QDOAS ???            XSCONV_kurucz;                                                       // kurucz

// QDOAS ??? static INDEX xsconvIndexSelected;

// =====================
// SUPPORTED LINE SHAPES
// =====================

// -----------------------------------------------------------------------------
// FUNCTION      XsconvFctGauss
// -----------------------------------------------------------------------------
// PURPOSE       Gaussian function
//
// INPUT         fwhm   : full-width at half maximum of the function;
//               step   : resolution of the line shape (usefule for normalisation);
//               delta  : the distance to the centre wavelength (or
//                        wavenumber);
//
// OUTPUT        pValue : the value of the function calculated at delta;
//
// RETURN        rc     : ERROR_ID_BAD_ARGUMENTS if one of the argument is not correct
//                        ERROR_ID_NO if the function is successful.
// -----------------------------------------------------------------------------

RC XsconvFctGauss(double *pValue,double fwhm,double step,double delta)
 {
  // Declarations

  double sigma,a,ia;                                                            // temporary variables
  RC rc;                                                                        // return code

  // Initializations

  *pValue=(double)0.;
  rc=ERROR_ID_NO;

  // Gaussian function

  if (fwhm<=(double)0.)
   rc=ERROR_SetLast("XsconvFctGauss",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"fwhm<=0");
  else if (step<=(double)0.)
   rc=ERROR_SetLast("XsconvFctGauss",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"step<=0");
  else
   {
    sigma=fwhm*0.5;
    a=sigma/sqrt(log(2.));
    ia=(double)step/(a*sqrt(PI));

    *pValue=ia*exp(-(delta*delta)/(a*a));
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      XsconvFctApod
// -----------------------------------------------------------------------------
// PURPOSE       Apodisation function (FTS)
//
// INPUT         resolution  : the resolution;
//               dispersion  : the dispersion;
//               phase       : the phase;
//               delta       : the distance to the centre wavelength (or
//                             wavenumber);
//
// OUTPUT        pValue      : the value of the function calculated in delta;
//
// RETURN        rc  ERROR_ID_BAD_ARGUMENTS if one of the argument is not correct
//                   ERROR_ID_NO if the function is successful.
// -----------------------------------------------------------------------------
// PROCESSING
//
// This function is implemented as provided in apod.m MATLAB function by
// Ann-Carine VANDAELE (IASB/BIRA, ULB)
// -----------------------------------------------------------------------------

RC XsconvFctApod(double *pValue,double resolution,double phase,double dispersion,double delta)
 {
  // Declarations

  double phi,d,a1,sinphi,cosphi,b,eps;                                          // temporary variables
  RC rc;                                                                        // return code

  // Initializations

  *pValue=(double)0.;
  eps=(double)1.e-6;
  rc=ERROR_ID_NO;

  // Apodisation function

  if (resolution<=(double)0.)
   rc=ERROR_SetLast("XsconvFctApod",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"resolution<=0");
  else
   {
    phi=(double)phase*PI/180.0;                                                 // degree -> radians
    d=(double)1.8*dispersion/resolution;
    a1=(double)PI*1.8/resolution;
    sinphi=sin(phi);
    cosphi=cos(phi);
    b=a1*delta;

    *pValue=(fabs(b)<eps)?d*cosphi:d*(sinphi-sin(phi-b))/b;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      XsconvFctApodNBS
// -----------------------------------------------------------------------------
// PURPOSE       Apodisation function Norton Beer Strong (FTS)
//
// INPUT         resolution  : the resolution;
//               dispersion  : the dispersion;
//               phase       : the phase;
//               delta       : the distance to the centre wavelength (or
//                             wavenumber);
//
// OUTPUT        pValue      : the value of the function calculated in delta;
//
// RETURN        rc     : ERROR_ID_BAD_ARGUMENTS if one of the argument is not correct
//                        ERROR_ID_NO if the function is successful.
// -----------------------------------------------------------------------------
// PROCESSING
//
// This function is implemented as provided in apodNBS.m MATLAB function by
// Ann-Carine VANDAELE (IASB/BIRA, ULB)
// -----------------------------------------------------------------------------

RC XsconvFctApodNBS(double *pValue,double resolution,double phase,double dispersion,double delta)
 {
  // Declarations

  double phi,d,a1,sinphi,cosphi,b,                                              // temporary variables
         phib,c0,c1,c2,c4,
         btwo,bfour,beight,dtempm,eps;

  RC rc;                                                                        // return code

  // Initializations

  *pValue=(double)0.;
  eps=(double)1.e-6;
  rc=ERROR_ID_NO;

  // Apodisation function (NBS)

  if (resolution<=(double)0.)
   rc=ERROR_SetLast("XsconvFctApodNBS",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"resolution<=0");
  else
   {
    phi=(double)phase*PI/180.0;                                                 // degree -> radians
    d=(double)1.8*dispersion/resolution;
    a1=(double)PI*1.8/resolution;
    sinphi=sin(phi);
    cosphi=cos(phi);
    b=a1*delta;
    phib=phi-b;

    c0=0.09;
    c1=0.0;
    c2=0.5875*d*8.0;
    c4=0.3225*d*128.0;

    btwo=b*b;
    bfour=btwo*btwo;
    beight=bfour*bfour;

    dtempm=c0*(sinphi-sin(phib))/b+
           c2*((3.0+btwo/2.0+bfour/8.0)*sinphi-(3.0-btwo)*sin(phib)-3.0*cos(phib)*b)/(bfour*b)+
           c4*((315.0+22.5*btwo+9.0*bfour/8.0+bfour*btwo/16.0+beight/128.0)*sinphi-(315.0-135.0*btwo+3.0*bfour)*sin(phib)-(315.0-30.0*btwo)*cos(phib)*b)/(beight*b);

    *pValue=(fabs(b)<eps)?(c0+c1/3.0+c2/15.0+c4/315.0)*cosphi:dtempm;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      XsconvFctBuild
// -----------------------------------------------------------------------------
// PURPOSE       Build a line shape in a vector
//
// INPUT         slitLembda  : the calibration (wavelength, wavenumber or pixel);
//               slitSize    : the size of previous vector;
//               slitType    : the type of line shape to build;
//               slitParam   : the list of slit function parameters
//               nSlitParam  : the number of elements in the previous list
//
// OUTPUT        slitVector  : the calculated line shape
//
// RETURN        rc          : return code
// -----------------------------------------------------------------------------

RC XsconvFctBuild(double *slitLembda,double *slitVector,INT slitSize,INT slitType,double *slitParam,INT nSlitParam)
 {
  // Declarations

  double sum,                                                                   // calculate the integral of the line shape for normalisation
         invSum,                                                                // inverse of the sum (in order to optimize the calculation time)
         slitStep;                                                              // the slit function dispersion
  INDEX  i;                                                                     // browse wavelength
  RC     rc;                                                                    // return code

  // Initializations

  sum=(double)0.;
  rc=ERROR_ID_NO;

  // Get the dispersion

  if (slitSize<=1)
   rc=ERROR_SetLast("XsconvFctBuild",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"slitSize<=1");
 else
   {
    // Calculate the resolution of the slit function

    slitStep=(double)(slitLembda[slitSize-1]-slitLembda[0])/(slitSize-1);

    // Build the slit function; the message is dispatched according to the selected type of line shape

    switch(slitType)
     {
   // --------------------------------------------------------------------------
      case SLIT_TYPE_GAUSS :

       if (nSlitParam<1)
        rc=ERROR_SetLast("XsconvFctBuild",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"Gauss function needs 1 parameter");
       else
        for (i=0;(i<slitSize) && !rc;i++)
         rc=XsconvFctGauss(&slitVector[i],
                             slitParam[0],                                      // FWHM
                             slitStep,                                          // resolution of the line shape
                             slitLembda[i]);                                    // distance to the centre wavelength

      break;
   // --------------------------------------------------------------------------
      case SLIT_TYPE_APOD :
      case SLIT_TYPE_APODNBS :

       if (nSlitParam<3)
        rc=ERROR_SetLast("XsconvFctBuild",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"Apod function needs 3 parameters");

       else if (slitType==SLIT_TYPE_APOD)
        for (i=0;(i<slitSize) && !rc;sum+=slitVector[i],i++)
         rc=XsconvFctApod(&slitVector[i],
                               slitParam[0],                                    // resolution
                               slitParam[1],                                    // phase
                               slitParam[2],                                    // dispersion
                               slitLembda[i]);                                  // distance to the centre wavelength

       else if (slitType==SLIT_TYPE_APODNBS)
        for (i=0;(i<slitSize) && !rc;sum+=slitVector[i],i++)
         rc=XsconvFctApodNBS(&slitVector[i],
                                  slitParam[0],                                 // resolution
                                  slitParam[1],                                 // phase
                                  slitParam[2],                                 // dispersion
                                  slitLembda[i]);                               // distance to the centre wavelength

      break;
   // --------------------------------------------------------------------------
      default :
       rc=ERROR_SetLast("XsconvFctBuild",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"the type of line shape is not supported");
      break;
   // --------------------------------------------------------------------------
     }

    // Normalisation of the slit function

    if (!rc && (slitType==SLIT_TYPE_APOD))
     {
      if (sum==(double)0.)
       rc=ERROR_SetLast("XsconvFctBuild",ERROR_TYPE_FATAL,ERROR_ID_DIVISION_BY_0,"the type of line shape is not supported");
      else if ((slitType==SLIT_TYPE_APOD))
       for (invSum=(double)1./sum,i=0;i<slitSize;i++)
        slitVector[i]*=invSum;
     }
   }

  return rc;
 }

// QDOAS ??? // ======================
// QDOAS ??? // TAB CONTROL PROCESSING
// QDOAS ??? // ======================
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? // ---------------------------------------------------------------
// QDOAS ??? // XsconvTabChange : TCN_SELCHANGE notification message processing
// QDOAS ??? // ---------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void XsconvTabChange(HWND hwndTab)
// QDOAS ???  {
// QDOAS ???   // Declarations and initializations
// QDOAS ???
// QDOAS ???   TAB_PAGE *pTab;
// QDOAS ???   xsconvIndexSelected=TabCtrl_GetCurSel(hwndTab);
// QDOAS ???   pTab=&xsconvTabPages[xsconvIndexSelected];
// QDOAS ???
// QDOAS ???   // Destroys the current child dialog box, if any.
// QDOAS ???
// QDOAS ???   if (xsconvHwndPage!=NULL)
// QDOAS ???    DestroyWindow(xsconvHwndPage);
// QDOAS ???
// QDOAS ???   // Load the selected child dialog box
// QDOAS ???
// QDOAS ???   xsconvHwndPage=CreateDialogIndirect(DOAS_hInst,pTab->dlgTemp,hwndTab,pTab->dlgProc);
// QDOAS ???   ShowWindow(hwndTab,SW_SHOW);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ---------------------------------------------
// QDOAS ??? // XsconvTabReSize : Resize pages in tab control
// QDOAS ??? // ---------------------------------------------
// QDOAS ???
// QDOAS ??? void XsconvTabReSize(HWND hwndParent,HWND hwndChild)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   RECT rcParent,rcChild;                 // windows rectangle
// QDOAS ???
// QDOAS ???   // Window resizing
// QDOAS ???
// QDOAS ???   GetWindowRect(hwndParent,&rcParent);   // parent window rectangle
// QDOAS ???   GetWindowRect(hwndChild,&rcChild);     // child window rectangle
// QDOAS ???
// QDOAS ???   MoveWindow(hwndChild,1,
// QDOAS ???             (rcParent.bottom-rcParent.top)-(rcChild.bottom-rcChild.top)-10,
// QDOAS ???              rcChild.right-rcChild.left,
// QDOAS ???              rcChild.bottom-rcChild.top,
// QDOAS ???              TRUE);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif

// ===============
// DATA PROCESSING
// ===============

// ------------------------------
// XSCONV_Reset : Release buffers
// ------------------------------

void XSCONV_Reset(XS *pXsconv)
 {
  if (pXsconv->lembda!=NULL)
   MEMORY_ReleaseDVector("XSCONV_Reset","lembda",pXsconv->lembda,0);
  if (pXsconv->vector!=NULL)
   MEMORY_ReleaseDVector("XSCONV_Reset","vector",pXsconv->vector,0);
  if (pXsconv->deriv2!=NULL)
   MEMORY_ReleaseDVector("XSCONV_Reset","deriv2",pXsconv->deriv2,0);

  memset(pXsconv,0,sizeof(XS));
 }

// ------------------------------------------------------
// XSCONV_Alloc : Allocate buffers involved in convolution
// ------------------------------------------------------

RC XSCONV_Alloc(XS *pXsconv,INT npts,INT deriv2Flag)
 {
  // Declaration

  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Buffers allocation

  if ((npts<=0) ||
     ((pXsconv->lembda=MEMORY_AllocDVector("XSCONV_Alloc ","lembda",0,npts-1))==NULL) ||
     ((pXsconv->vector=MEMORY_AllocDVector("XSCONV_Alloc ","vector",0,npts-1))==NULL) ||
      (deriv2Flag && ((pXsconv->deriv2=MEMORY_AllocDVector("XSCONV_Alloc ","deriv2",0,npts-1))==NULL)))

   rc=ERROR_ID_ALLOC;

  else
   pXsconv->NDET=npts;

  // Return

  return rc;
 }

// --------------------------------------------------------
// XsconvGetFwhm : Get slit function full width half way up
// --------------------------------------------------------

RC XsconvGetFwhm(XS *pSlit,INT slitType,double *slitParam)
 {
  // Declarations

  double max,lmin,lmax,l1,l2,x,x0;                         // search for wavelengths giving function at half way up
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;
  x0=(double)0.;

  if (slitType==SLIT_TYPE_FILE)
   {
    // Get the value of function half way up

    if ((rc=SPLINE_Vector(pSlit->lembda,pSlit->vector,pSlit->deriv2,pSlit->NDET,&x0,&max,1,SPLINE_CUBIC,"XsconvGetFwhm "))!=0)
     goto EndFwhm;

    max*=(double)0.5;

    // Search for the wavelength in the left part giving the value of function half way up

    for (lmin=pSlit->lembda[0],lmax=(double)0.,l1=lmin*0.5,x=(double)0.;
        (lmin<lmax) && fabs(x-max)>1.e-5;l1=(lmin+lmax)*0.5)
     {
      if ((rc=SPLINE_Vector(pSlit->lembda,pSlit->vector,pSlit->deriv2,pSlit->NDET,&l1,&x,1,SPLINE_CUBIC,"XsconvGetFwhm "))!=0)
       goto EndFwhm;

      if (x<max)
       lmin=l1;
      else if (x>max)
       lmax=l1;
     }

    // Search for the wavelength in the right part giving the value of function half way up

    for (lmin=(double)0.,lmax=pSlit->lembda[pSlit->NDET-1],l2=lmax*0.5,x=(double)0.;
        (lmin<lmax) && fabs(x-max)>1.e-5;l2=(lmin+lmax)*0.5)
     {
      if ((rc=SPLINE_Vector(pSlit->lembda,pSlit->vector,pSlit->deriv2,pSlit->NDET,&l2,&x,1,SPLINE_CUBIC,"XsconvGetFwhm "))!=0)
       goto EndFwhm;

      if (x<max)
       lmax=l2;
      else if (x>max)
       lmin=l2;
     }

    // Width of slit function half way up

    *slitParam=(double)(l2-l1);
   }

  EndFwhm :

  // Return

  return rc;
 }

// ================
// FILES PROCESSING
// ================

// -------------------------------------
// XSCONV_FileSelection : File selection
// -------------------------------------

// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? void XSCONV_FileSelection(HWND hwndXsconv,UCHAR *file,MASK fileType,INT fileMode,INT fileCommand,INT ringFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR *ptr;
// QDOAS ???   SZ_LEN pathLength;
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   pathLength=strlen(file);
// QDOAS ???
// QDOAS ???   // Set automatic path in output mode
// QDOAS ???
// QDOAS ???   if ((fileMode==FILE_MODE_SAVE) && !ringFlag)
// QDOAS ???    {
// QDOAS ???     if ((pathLength!=0) && (file[pathLength-1]==PATH_SEP))
// QDOAS ???      strcat(file,"automatic");
// QDOAS ???     else if (pathLength==0)
// QDOAS ???      {
// QDOAS ???       UCHAR pathSep[2];
// QDOAS ???
// QDOAS ???       pathSep[0]=PATH_SEP;
// QDOAS ???       pathSep[1]='\0';
// QDOAS ???
// QDOAS ???       getcwd(file,MAX_ITEM_TEXT_LEN);
// QDOAS ???       strcat(file,pathSep);
// QDOAS ???       strcat(file,"automatic");
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Select path and file
// QDOAS ???
// QDOAS ???   FILES_Select(hwndXsconv,file,MAX_ITEM_TEXT_LEN+1,fileType,0,(UCHAR)fileMode,NULL,ITEM_NONE,ITEM_NONE);
// QDOAS ???
// QDOAS ???   // Returned path processing in output mode
// QDOAS ???
// QDOAS ???   if ((fileMode==FILE_MODE_SAVE) && !ringFlag &&
// QDOAS ???       (!STD_Stricmp((ptr=file),"automatic") ||
// QDOAS ???      (((ptr=strrchr(file,PATH_SEP))!=NULL) &&
// QDOAS ???        (strlen(++ptr)!=0) &&
// QDOAS ???        !STD_Stricmp(ptr,"automatic"))))
// QDOAS ???
// QDOAS ???    *ptr=0;
// QDOAS ???
// QDOAS ???   SetWindowText(GetDlgItem(hwndXsconv,fileCommand),file);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif

// ------------------------------------------------------------
// XSCONV_LoadCalibrationFile : Final wavelength scale read out
// ------------------------------------------------------------

RC XSCONV_LoadCalibrationFile(XS *pLembda,UCHAR *lembdaFile,INT nextraPixels)
 {
  // Declarations

  UCHAR  lembdaBuffer[MAX_ITEM_TEXT_LEN+1];
  double step;
  FILE *lembdaFp;
  INT npts;
  INDEX i;
  RC rc;

  // Initializations

  XSCONV_Reset(pLembda);
  rc=ERROR_ID_NO;
  npts=0;

  // Open BINARY file

  FILES_RebuildFileName(lembdaBuffer,lembdaFile,1);

  if ((lembdaFp=fopen(FILES_RebuildFileName(lembdaBuffer,lembdaFile,1),"rt"))==NULL)
   rc=ERROR_SetLast("XSCONV_LoadCalibrationFile",ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,lembdaBuffer);
  else
   {
    // Get the number of lines

    while (!feof(lembdaFp) && fgets(lembdaBuffer,MAX_ITEM_TEXT_LEN,lembdaFp))
     if ((strchr(lembdaBuffer,';')==NULL) && (strchr(lembdaBuffer,'*')==NULL))
      npts++;

    if (!npts)
     rc=ERROR_SetLast("XSCONV_LoadCalibrationFile",ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,lembdaFile);

    // Buffers allocation

    else if (XSCONV_Alloc(pLembda,npts+2*nextraPixels,0))
     rc=ERROR_ID_ALLOC;

    // File read out

    else
     {
      // File read out

      VECTOR_Init(pLembda->lembda,(double)0.,npts+2*nextraPixels);
      fseek(lembdaFp,0L,SEEK_SET);

      for (i=0;(i<npts) && !feof(lembdaFp);)
       if (fgets(lembdaBuffer,MAX_ITEM_TEXT_LEN,lembdaFp) && (strchr(lembdaBuffer,';')==NULL) && (strchr(lembdaBuffer,'*')==NULL))
        {
         sscanf(lembdaBuffer,"%lf ",&pLembda->lembda[i+nextraPixels]);
         i++;
        }

      step=(pLembda->lembda[nextraPixels+npts-1]-pLembda->lembda[nextraPixels])/npts;

      for (i=0;i<nextraPixels;i++)
       {
        pLembda->lembda[i]=pLembda->lembda[nextraPixels]-step*(nextraPixels-i);
        pLembda->lembda[nextraPixels+npts+i]=pLembda->lembda[nextraPixels+npts-1]+step*i;
       }
     }
   }

  // Close file

  if (lembdaFp!=NULL)
   fclose(lembdaFp);

  // Return

  return rc;
 }

// ------------------------------------------------
// XSCONV_LoadSlitFunction : Slit function read out
// ------------------------------------------------

RC XSCONV_LoadSlitFunction(XS *pSlitXs,SLIT *pSlit,double *pGaussWidth,INT *pSlitType)
 {
  // Declarations

  UCHAR  slitBuffer[MAX_ITEM_TEXT_LEN+1];
  FILE  *slitFp;
  RC     rc;
  INDEX  i;
  double x,a,a2,delta,sigma,invapi,sumPoly,slitStep,norm1,norm2,slitParam[4];
  INT    nSlit,slitType,slitSize,nFwhm;

  // Initializations

  XSCONV_Reset(pSlitXs);

  invapi=norm1=norm2=(double)0.;
  sumPoly=(double)0.;
  slitFp=NULL;
  nSlit=0;

  slitParam[0]=pSlit->slitParam;
  slitParam[1]=pSlit->slitParam2;
  slitParam[2]=pSlit->slitParam3;
  slitParam[3]=pSlit->slitParam4;

  slitType=pSlit->slitType;
  nFwhm=6;         // number of pixels by FWHM
  rc=ERROR_ID_NO;

  // Input read out

  if ((slitType!=SLIT_TYPE_GAUSS) &&    // File or wavelength dependent function
      (slitType!=SLIT_TYPE_INVPOLY) &&
      (slitType!=SLIT_TYPE_ERF) &&
      (slitType!=SLIT_TYPE_VOIGT) &&
      (slitType!=SLIT_TYPE_APOD) &&
      (slitType!=SLIT_TYPE_APODNBS))
   {
    // Open ASCII file

    if ((slitFp=fopen(FILES_RebuildFileName(slitBuffer,pSlit->slitFile,1),"rt"))==NULL)
     rc=ERROR_SetLast("XSCONV_LoadSlitFunction",ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,pSlit->slitFile);
    else
     {
      // Get the number of lines

      while (!feof(slitFp) && fgets(slitBuffer,MAX_ITEM_TEXT_LEN,slitFp))
       if ((strchr(slitBuffer,';')==NULL) && (strchr(slitBuffer,'*')==NULL))
        nSlit++;

      if (!nSlit)
       rc=ERROR_SetLast("XSCONV_LoadSlitFunction",ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,pSlit->slitFile);

      // Buffers allocation

      else if (XSCONV_Alloc(pSlitXs,nSlit,1))
       rc=ERROR_ID_ALLOC;

      else
       {
        // File read out

        fseek(slitFp,0L,SEEK_SET);

        for (i=0,sumPoly=(double)0.;(i<nSlit) && !feof(slitFp);)
         if (fgets(slitBuffer,MAX_ITEM_TEXT_LEN,slitFp) && (strchr(slitBuffer,';')==NULL) && (strchr(slitBuffer,'*')==NULL))
          {
           sscanf(slitBuffer,"%lf %lf",&pSlitXs->lembda[i],&pSlitXs->vector[i]);
           if (i)
            sumPoly+=(pSlitXs->vector[i]+pSlitXs->vector[i-1])*(pSlitXs->lembda[i]-pSlitXs->lembda[i-1])*0.5;

           i++;
          }

        // Function normalization

        if (slitType==SLIT_TYPE_FILE)
         for (i=0;i<nSlit;i++)
          pSlitXs->vector[i]/=sumPoly;
       }
     }

    // Second derivatives calculation for future interpolations

    if (!rc &&
       !(rc=SPLINE_Deriv2(pSlitXs->lembda,pSlitXs->vector,pSlitXs->deriv2,nSlit,"XSCONV_LoadSlitFunction ")))

     rc=XsconvGetFwhm(pSlitXs,slitType,pGaussWidth);
   }
  else if (pSlit->slitParam==(double)0.)
   rc=ERROR_SetLast("XSCONV_LoadSlitFunction",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"SlitParam");

  // Precalculate the slit function

  else if ((slitType!=SLIT_TYPE_APOD) && (slitType!=SLIT_TYPE_APODNBS))
   {
    slitStep=pSlit->slitParam/nFwhm;                               // 6 points/FWHM

 //   slitSize=(int)(ceil((nFwhm*pSlit->slitParam)/slitStep)+1.);

    slitSize=nFwhm*nFwhm+1;

//    slitSize=1001;
//    slitStep=(6*pSlit->slitParam)/(slitSize-1);

    if ((pSlitType!=NULL) && !XSCONV_Alloc(pSlitXs,slitSize,1))
     {
      sigma=pSlit->slitParam*0.5;
      a=sigma/sqrt(log(2.));
      a2=pSlit->slitParam3*0.5/sqrt(log(2.));
      delta=pSlit->slitParam2*0.5;

      if (slitType==SLIT_TYPE_GAUSS)
       invapi=(double)1./(a*sqrt(PI))*slitStep;
      else if ((slitType==SLIT_TYPE_VOIGT) && (pSlit->slitParam3!=(double)0.))
       {
        norm1=(double)Voigtx((double)0.,pSlit->slitParam2);
        norm2=(double)Voigtx((double)0.,pSlit->slitParam4);

        if ((norm1==(double)0.) || (norm2==(double)0.))
         rc=ERROR_SetLast("XSCONV_LoadSlitFunction",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"norm1 or norm2");
        else
         {
          norm1=(double)1./norm1;
          norm2=(double)1./norm2;
         }
       }
      else
       norm1=norm2=(double)1.;

      for (i=0,x=(double)-0.5*nFwhm*pSlit->slitParam;(i<slitSize) && !rc;i++,x+=(double)slitStep)
       {
        pSlitXs->lembda[i]=(double)x;

        if (slitType==SLIT_TYPE_INVPOLY)
         sumPoly+=pSlitXs->vector[i]=(double)pow(sigma,pSlit->slitParam2)/(pow(x,pSlit->slitParam2)+pow(sigma,pSlit->slitParam2));
        else if (slitType==SLIT_TYPE_ERF)
         pSlitXs->vector[i]=(double)(ERF_GetValue((x+delta)/a)-ERF_GetValue((x-delta)/a))/(4.*delta)*slitStep;
        else if (slitType==SLIT_TYPE_GAUSS)
         pSlitXs->vector[i]=(double)invapi*exp(-(x*x)/(a*a));
        else if (slitType==SLIT_TYPE_VOIGT)
         {
          if ((x>(double)0.) && (pSlit->slitParam3!=(double)0.))
           pSlitXs->vector[i]=(double)Voigtx(x/a2,pSlit->slitParam4)*norm2;
          else
           pSlitXs->vector[i]=(double)Voigtx(x/a,pSlit->slitParam2)*norm1;

          sumPoly+=pSlitXs->vector[i];
         }
       }
   //   exit(1);
      if (slitType==SLIT_TYPE_GAUSS)
       rc=XsconvFctBuild(pSlitXs->lembda,pSlitXs->vector,slitSize,pSlit->slitType,slitParam,4);

      if (!rc)
       {
        if ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_VOIGT))
         for (i=0;i<slitSize;i++)
          pSlitXs->vector[i]/=sumPoly;

        rc=SPLINE_Deriv2(pSlitXs->lembda,pSlitXs->vector,pSlitXs->deriv2,slitSize,"XSCONV_LoadSlitFunction ");

        if (pSlitType!=NULL)
         *pSlitType=SLIT_TYPE_FILE;
       }
     }

    if (pGaussWidth!=NULL)
     *pGaussWidth=pSlit->slitParam;
   }

  // for apodisation function, no pre-calculation

  else if (pGaussWidth!=NULL)
   *pGaussWidth=pSlit->slitParam;

  // Close file

  if (slitFp!=NULL)
   fclose(slitFp);

  // Return

  return rc;
 }

// -------------------------------------------------------
// XSCONV_LoadCrossSectionFile : Load a cross section file
// -------------------------------------------------------

RC XSCONV_LoadCrossSectionFile(XS *pCross,UCHAR *crossFile,double lembdaMin,double lembdaMax,double shift,INT conversionMode)
 {
  // Declarations

  double lembda,crossValue,firstLembda;
  UCHAR  crossBuffer[MAX_ITEM_TEXT_LEN+1];
  FILE  *crossFp;
  INT    found,npts;
  INDEX  i,i0,iN,istep;
  RC     rc;

  // Initializations

  firstLembda=lembda=(double)0.;
  XSCONV_Reset(pCross);
  npts=found=0;
  rc=ERROR_ID_NO;

  // Open ASCII file

  if ((crossFp=fopen(FILES_RebuildFileName(crossBuffer,crossFile,1),"rt"))==NULL)
   rc=ERROR_SetLast("XSCONV_LoadCrossSectionFile",ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,crossBuffer);
  else
   {
    // First, search for data in the specified wavelengths range and determine the correct vector size

    while (!feof(crossFp) && fgets(crossBuffer,MAX_ITEM_TEXT_LEN,crossFp) && (!found || ((lembda>=lembdaMin) && (lembda<=lembdaMax))))

     if ((strchr(crossBuffer,';')==NULL) && (strchr(crossBuffer,'*')==NULL) && (sscanf(crossBuffer,"%lf %lf",&lembda,&crossValue)==2))
      {
       if (!found)
        firstLembda=lembda;

       if ((lembda>=lembdaMin) && (lembda<=lembdaMax))
        {
         npts++;
         found=1;
        }
     }

    if (!npts)
     rc=ERROR_SetLast("XSCONV_LoadCrossSectionFile",ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,crossBuffer);

    // Buffers allocation

    else if (XSCONV_Alloc(pCross,npts,1))
     rc=ERROR_ID_ALLOC;

    else
     {
      // Set indexes according to wavelength order

      if (firstLembda<=lembda)
       {
        i0=0;
        iN=npts;
        istep=1;
       }
      else
       {
        i0=npts-1;
        iN=-1;
        istep=-1;
       }

      // Fill vectors

      fseek(crossFp,0L,SEEK_SET);

      for (i=i0;(i!=iN) && !feof(crossFp) && fgets(crossBuffer,MAX_ITEM_TEXT_LEN,crossFp);)

       if ((strchr(crossBuffer,';')==NULL) && (strchr(crossBuffer,'*')==NULL) &&
           (sscanf(crossBuffer,"%lf %lf",&lembda,&crossValue)==2) &&
           (lembda>=lembdaMin) && (lembda<=lembdaMax))
        {
         if (conversionMode==CONVOLUTION_CONVERSION_VAC2AIR)
          lembda=(double)0.1*(9.9972683*lembda+0.0107-(19.625/lembda));
         else if (conversionMode==CONVOLUTION_CONVERSION_AIR2VAC)
          lembda=(double)0.1*(10.0027325*lembda-0.0107+(19.625/lembda));

         pCross->lembda[i]=lembda+shift;
         pCross->vector[i]=crossValue;

         i+=istep;
        }

      // Second derivatives computation

      rc=SPLINE_Deriv2(pCross->lembda,pCross->vector,pCross->deriv2,npts,"XSCONV_LoadCrossSectionFile ");
     }
   }

  if (crossFp!=NULL)
   fclose(crossFp);

  // Return

  return rc;
 }

// -------------------------------------------------
// XsconvSaveNew : Save the convoluted cross section
// -------------------------------------------------

// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? RC XsconvSaveNew(HWND hwndXsconv,XS *pXs,double *filterVector,XSCONV *pXsOptions,INT nextraPixels)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR fileName[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   PRJCT_FILTER *pLFilter,*pHFilter;
// QDOAS ???   SZ_LEN fileNameLength;
// QDOAS ???   HWND hwndTab;
// QDOAS ???   UCHAR *ptr,*ptr2;
// QDOAS ???   FILE *fp;
// QDOAS ???   INDEX i,slitType;
// QDOAS ???   INT nsize;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Change cursor appearance
// QDOAS ???
// QDOAS ???   SetCursor(xsconvHCurHourGlass);
// QDOAS ???   pLFilter=&pXsOptions->lfilter;
// QDOAS ???   pHFilter=&pXsOptions->hfilter;
// QDOAS ???   nsize=pXs->NDET-nextraPixels;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   hwndTab=GetDlgItem(hwndXsconv,TOOL_CONVOLUTION_TAB);
// QDOAS ???   strcpy(fileName,pXsOptions->path);
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   if (((ptr=strrchr(fileName,PATH_SEP))!=NULL) && !strlen(ptr+1))
// QDOAS ???    {
// QDOAS ???     // Concatenate file name to output path
// QDOAS ???
// QDOAS ???     if ((ptr2=strrchr(pXsOptions->crossFile,PATH_SEP))!=NULL)
// QDOAS ???      ptr2++;
// QDOAS ???     else
// QDOAS ???      ptr2=pXsOptions->crossFile;
// QDOAS ???
// QDOAS ???     sprintf(ptr,"%c%s",PATH_SEP,ptr2);
// QDOAS ???
// QDOAS ???     // Replace file extension by the correct one
// QDOAS ???
// QDOAS ???     if ((fileNameLength=strlen(fileName))!=0)
// QDOAS ???      {
// QDOAS ???       if ((ptr=strrchr(fileName,'.'))==NULL)
// QDOAS ???        {
// QDOAS ???         fileName[fileNameLength++]='.';
// QDOAS ???         fileName[fileNameLength]=0;
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       strcat(fileName,xsconvFileExt[pXsOptions->convolutionType]);
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Open file
// QDOAS ???
// QDOAS ???   if ((fp=fopen(FILES_RebuildFileName(fileName,fileName,1),"rt"))!=NULL)
// QDOAS ???    {
// QDOAS ???     TabCtrl_SetCurSel(hwndTab,TAB_TYPE_XSCONV_GENERAL);
// QDOAS ???     XsconvTabChange(hwndTab);
// QDOAS ???     if (MSG_MessageBox(hwndXsconv,TOOL_CONVOLUTION_PATH_BUTTON,MENU_WINDOWS_TOOL_CONVOLUTION,IDS_MSGBOX_FILE,
// QDOAS ???                         MB_YESNO|MB_ICONQUESTION,fileName)==IDNO)
// QDOAS ???      rc=IDS_MSGBOX_FILE;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if (fp!=NULL)
// QDOAS ???    fclose(fp);
// QDOAS ???
// QDOAS ???   // Save file
// QDOAS ???
// QDOAS ???   if (!rc)
// QDOAS ???    {
// QDOAS ???     if ((fp=fopen(fileName,"w+t"))!=NULL)
// QDOAS ???      {
// QDOAS ???       if (!pXsOptions->noComment)
// QDOAS ???        {
// QDOAS ???         // Header
// QDOAS ???
// QDOAS ???         fprintf(fp,";\n");
// QDOAS ???         fprintf(fp,"; High resolution cross section file : %s\n",FILES_RebuildFileName(fileName,pXsOptions->crossFile,1));
// QDOAS ???         fprintf(fp,"; Calibration file : %s\n",FILES_RebuildFileName(fileName,pXsOptions->calibrationFile,1));
// QDOAS ???         fprintf(fp,"; Shift applied : %s nm\n",pXsOptions->shift);
// QDOAS ???         fprintf(fp,"; Convolution type : %s\n",xsconvConvolutionTypesStr[pXsOptions->convolutionType]);
// QDOAS ???
// QDOAS ???         if (pXsOptions->convolutionType!=CONVOLUTION_TYPE_NONE)
// QDOAS ???          {
// QDOAS ???           if (pXsOptions->convolutionType==CONVOLUTION_TYPE_I0_CORRECTION)
// QDOAS ???            {
// QDOAS ???             fprintf(fp,"; Kurucz file : %s\n",FILES_RebuildFileName(fileName,pXsOptions->kuruczFile,1));
// QDOAS ???             fprintf(fp,"; Concentration in I0 (mol/cm**2) : %g\n",pXsOptions->conc);
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           fprintf(fp,"; Slit function type : %s\n",XSCONV_slitTypes[(slitType=pXsOptions->slitConv.slitType)]);
// QDOAS ???
// QDOAS ???           if (((slitType=pXsOptions->slitConv.slitType)==SLIT_TYPE_FILE) ||
// QDOAS ???                (slitType==SLIT_TYPE_GAUSS_FILE) ||
// QDOAS ???                (slitType==SLIT_TYPE_INVPOLY_FILE) ||
// QDOAS ??? //               (slitType==SLIT_TYPE_VOIGT_FILE) ||
// QDOAS ???                (slitType==SLIT_TYPE_ERF_FILE))
// QDOAS ???            fprintf(fp,"; Slit function file : %s\n",FILES_RebuildFileName(fileName,pXsOptions->slitConv.slitFile,1));
// QDOAS ???           if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_ERF))
// QDOAS ???            fprintf(fp,"; Gaussian FWHM : %.3f\n",pXsOptions->slitConv.slitParam);
// QDOAS ???           if ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_INVPOLY_FILE))
// QDOAS ???            fprintf(fp,"; Polynomial degree : %d\n",(int)pXsOptions->slitConv.slitParam2);
// QDOAS ???           if ((slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_ERF_FILE))
// QDOAS ???            fprintf(fp,"; Boxcar width : %.3f\n",pXsOptions->slitConv.slitParam2);
// QDOAS ???
// QDOAS ???           if ((slitType== SLIT_TYPE_APOD) || (slitType== SLIT_TYPE_APODNBS))
// QDOAS ???            {
// QDOAS ???             fprintf(fp,"; Resolution : %.3lf\n",pXsOptions->slitConv.slitParam);
// QDOAS ???             fprintf(fp,"; Phase      : %.3lf\n",pXsOptions->slitConv.slitParam2);
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           if (slitType==SLIT_TYPE_VOIGT)
// QDOAS ???            {
// QDOAS ???             fprintf(fp,"; Gaussian FWHM (L) : %.3f\n",pXsOptions->slitConv.slitParam);
// QDOAS ???             fprintf(fp,"; Lorentz/Gauss ratio (L) : %.3f\n",pXsOptions->slitConv.slitParam2);
// QDOAS ???             fprintf(fp,"; Gaussian FWHM (R) : %.3f\n",pXsOptions->slitConv.slitParam3);
// QDOAS ???             fprintf(fp,"; Lorentz/Gauss ratio (R) : %.3f\n",pXsOptions->slitConv.slitParam4);
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           if (((slitType=pXsOptions->slitDConv.slitType)!=SLIT_TYPE_FILE) || (strlen(pXsOptions->slitDConv.slitFile)!=0))
// QDOAS ???            fprintf(fp,"; Deconvolution slit function type : %s",XSCONV_slitTypes[slitType]);
// QDOAS ???           if (((slitType==SLIT_TYPE_FILE) && strlen(pXsOptions->slitDConv.slitFile)) ||
// QDOAS ???                (slitType==SLIT_TYPE_GAUSS_FILE) ||
// QDOAS ???                (slitType==SLIT_TYPE_INVPOLY_FILE) ||
// QDOAS ??? //               (slitType==SLIT_TYPE_VOIGT_FILE) ||
// QDOAS ???                (slitType==SLIT_TYPE_ERF_FILE))
// QDOAS ???            fprintf(fp,"; Deconvolution slit function file : %s\n",FILES_RebuildFileName(fileName,pXsOptions->slitDConv.slitFile,1));
// QDOAS ???           if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_ERF))
// QDOAS ???            fprintf(fp,"; Deconvolution slit function gaussian FWHM : %lf\n",pXsOptions->slitDConv.slitParam);
// QDOAS ???           if ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_INVPOLY_FILE))
// QDOAS ???            fprintf(fp,"; Deconvolution slit function polynomial degree : %d\n",(int)pXsOptions->slitDConv.slitParam2);
// QDOAS ???           if ((slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_ERF_FILE))
// QDOAS ???            fprintf(fp,"; Deconvolution slit function boxcar width : %lf\n",pXsOptions->slitDConv.slitParam2);
// QDOAS ???
// QDOAS ???           if (slitType==SLIT_TYPE_VOIGT)
// QDOAS ???            {
// QDOAS ???             fprintf(fp,"; Gaussian FWHM (L) : %.3f\n",pXsOptions->slitDConv.slitParam);
// QDOAS ???             fprintf(fp,"; Lorentz/Gauss ratio (L) : %.3f\n",pXsOptions->slitDConv.slitParam2);
// QDOAS ???             fprintf(fp,"; Gaussian FWHM (R) : %.3f\n",pXsOptions->slitDConv.slitParam3);
// QDOAS ???             fprintf(fp,"; Lorentz/Gauss ratio (R) : %.3f\n",pXsOptions->slitDConv.slitParam4);
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         // Low pass filtering
// QDOAS ???
// QDOAS ???         if (pLFilter->type!=PRJCT_FILTER_TYPE_NONE)
// QDOAS ???          {
// QDOAS ???           fprintf(fp,"; Low-pass filtering\n");
// QDOAS ???           fprintf(fp,"; Filter applied : %s\n",PRJCT_filterTypes[pLFilter->type]);
// QDOAS ???           fprintf(fp,"; Filter number of iterations : %d\n",pLFilter->filterNTimes);
// QDOAS ???
// QDOAS ???           if (pLFilter->type==PRJCT_FILTER_TYPE_KAISER)
// QDOAS ???            {
// QDOAS ???             fprintf(fp,"; Cutoff frequency : %g\n",pLFilter->kaiserCutoff);
// QDOAS ???             fprintf(fp,"; Pass band : %g\n",pLFilter->kaiserPassBand);
// QDOAS ???             fprintf(fp,"; Tolerance : %g\n",pLFilter->kaiserTolerance);
// QDOAS ???            }
// QDOAS ???           else if (pLFilter->type==PRJCT_FILTER_TYPE_GAUSSIAN)
// QDOAS ???            fprintf(fp,"; Gaussian FWHM : %g\n",pLFilter->fwhmWidth);
// QDOAS ???           else if ((pLFilter->type==PRJCT_FILTER_TYPE_BOXCAR) || (pLFilter->type==PRJCT_FILTER_TYPE_TRIANGLE))
// QDOAS ???            fprintf(fp,"; Filter width : %d\n",pLFilter->filterWidth);
// QDOAS ???           else if (pLFilter->type==PRJCT_FILTER_TYPE_SG)
// QDOAS ???            {
// QDOAS ???             fprintf(fp,"; Filter width : %d\n",pLFilter->filterWidth);
// QDOAS ???             fprintf(fp,"; Filter order : %d\n",pLFilter->filterOrder);
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         // High pass filtering
// QDOAS ???
// QDOAS ???         if (pHFilter->type!=PRJCT_FILTER_TYPE_NONE)
// QDOAS ???          {
// QDOAS ???           fprintf(fp,"; High-pass filtering\n");
// QDOAS ???           fprintf(fp,"; Filter applied : %s\n",PRJCT_filterTypes[pHFilter->type]);
// QDOAS ???           fprintf(fp,"; Filter number of iterations : %d\n",pHFilter->filterNTimes);
// QDOAS ???
// QDOAS ???           if (pHFilter->type==PRJCT_FILTER_TYPE_KAISER)
// QDOAS ???            {
// QDOAS ???             fprintf(fp,"; Cutoff frequency : %g\n",pHFilter->kaiserCutoff);
// QDOAS ???             fprintf(fp,"; Pass band : %g\n",pHFilter->kaiserPassBand);
// QDOAS ???             fprintf(fp,"; Tolerance : %g\n",pHFilter->kaiserTolerance);
// QDOAS ???            }
// QDOAS ???           else if (pHFilter->type==PRJCT_FILTER_TYPE_GAUSSIAN)
// QDOAS ???            fprintf(fp,"; Gaussian FWHM : %g\n",pHFilter->fwhmWidth);
// QDOAS ???           else if ((pHFilter->type==PRJCT_FILTER_TYPE_BOXCAR) || (pHFilter->type==PRJCT_FILTER_TYPE_TRIANGLE))
// QDOAS ???            fprintf(fp,"; Filter width : %d\n",pHFilter->filterWidth);
// QDOAS ???           else if (pHFilter->type==PRJCT_FILTER_TYPE_SG)
// QDOAS ???            {
// QDOAS ???             fprintf(fp,"; Filter width : %d\n",pHFilter->filterWidth);
// QDOAS ???             fprintf(fp,"; Filter order : %d\n",pHFilter->filterOrder);
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         fprintf(fp,";\n; Columns description :\n");
// QDOAS ???         fprintf(fp,"; Column 1 : calibration;\n");
// QDOAS ???         fprintf(fp,"; Column 2 : %s;\n",((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))?
// QDOAS ???                    "convoluted and filtered cross section" : "convoluted cross section");
// QDOAS ???
// QDOAS ???         if ((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))
// QDOAS ???          fprintf(fp,"; Column 3 : convoluted only cross section.\n");
// QDOAS ???         fprintf(fp,";\n");
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Cross section
// QDOAS ???
// QDOAS ???       if ((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))
// QDOAS ???        for (i=nextraPixels;i<nsize;i++)
// QDOAS ???         fprintf(fp,"%.14le %.14le %.14le\n",pXs->lembda[i],filterVector[i],pXs->vector[i]);
// QDOAS ???       else
// QDOAS ???        for (i=nextraPixels;i<nsize;i++)
// QDOAS ???         fprintf(fp,"%.14le %.14le\n",pXs->lembda[i],pXs->vector[i]);
// QDOAS ???
// QDOAS ???       fclose(fp);
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???       TabCtrl_SetCurSel(hwndTab,TAB_TYPE_XSCONV_GENERAL);
// QDOAS ???       XsconvTabChange(hwndTab);
// QDOAS ???       MSG_MessageBox(hwndXsconv,TOOL_CONVOLUTION_PATH_BUTTON,MENU_WINDOWS_TOOL_CONVOLUTION,(rc=ERROR_ID_FILE_OPEN),
// QDOAS ???                       MB_OK|MB_ICONHAND,fileName);
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif // __WINDOAS_GUI_

// =====================
// CONVOLUTION FUNCTIONS
// =====================

// ----------------------------------------------------------
// XSCONV_TypeNone : Apply no convolution, interpolation only
// ----------------------------------------------------------

RC XSCONV_TypeNone(XS *pXsnew,XS *pXshr)
 {
  return SPLINE_Vector(pXshr->lembda,pXshr->vector,pXshr->deriv2,pXshr->NDET,pXsnew->lembda,pXsnew->vector,pXsnew->NDET,SPLINE_CUBIC,"XSCONV_TypeNone ");
 }

// -----------------------------------------------------------------------
// XSCONV_TypeGauss : Gaussian convolution with variable half way up width
// -----------------------------------------------------------------------

RC XSCONV_TypeGauss(double *lambda,double *Spec,double *SDeriv2,double lambdaj,
                    double dldj,double *SpecConv,double fwhm,double slitParam2,INT slitType)
 {
  // Declarations

  double h,oldF,newF,Lim,ld_inc,ldi,dld,a,delta,
         lambdaMax,SpecOld, SpecNew,sigma,
         crossFIntegral, FIntegral;

  RC rc;

  // Initializations

  fwhm=fabs(fwhm);
  crossFIntegral=FIntegral=(double)0.;
  sigma=fwhm*0.5;
  a=sigma/sqrt(log(2.));
  delta=slitParam2*0.5;

  Lim=(double)2.*fwhm;

  if ((ld_inc=(double)fwhm/3.)>dldj)
   ld_inc=dldj;

  h=(double)ld_inc*0.5;

  // Browse wavelengths in the final calibration vector

  ldi=lambdaj-Lim;
  lambdaMax=lambdaj+Lim;

  // Search for first pixel in high resolution cross section in the wavelength range delimited by slit function

  dld = -(ldi-lambdaj);

  if (slitType==SLIT_TYPE_GAUSS)
//   oldF=(double)exp(-4.*log(2.)*(dld*dld)/(fwhm*fwhm));
   rc=XsconvFctGauss(&oldF,fwhm,ld_inc,dld);
  else if (slitType==SLIT_TYPE_INVPOLY)
   oldF=(double)pow(sigma,(double)slitParam2)/(pow(dld,(double)slitParam2)+pow(sigma,(double)slitParam2));
  else if (slitType==SLIT_TYPE_ERF)
   oldF=(double)(ERF_GetValue((dld+delta)/a)-ERF_GetValue((dld-delta)/a))/(4.*delta);

  rc=SPLINE_Vector(lambda,Spec,SDeriv2,NDET,&ldi,&SpecOld,1,SPLINE_CUBIC,"XSCONV_TypeGauss ");

  while (!rc && (ldi<=lambdaMax))
   {
    ldi += (double) ld_inc;
    dld = -(ldi-lambdaj);

    if (slitType==SLIT_TYPE_GAUSS)
//     newF=(double)exp(-4.*log(2.)*(dld*dld)/(fwhm*fwhm));
     rc=XsconvFctGauss(&newF,fwhm,ld_inc,dld);
    else if (slitType==SLIT_TYPE_INVPOLY)
     newF=(double)pow(sigma,(double)slitParam2)/(pow(dld,(double)slitParam2)+pow(sigma,(double)slitParam2));
    else if (slitType==SLIT_TYPE_ERF)
     newF=(double)(ERF_GetValue((dld+delta)/a)-ERF_GetValue((dld-delta)/a))/(4.*delta);

    if ((rc=SPLINE_Vector(lambda,Spec,SDeriv2,NDET,&ldi,&SpecNew,1,SPLINE_CUBIC,"XSCONV_TypeGauss "))!=0)
     break;

    crossFIntegral += (SpecOld*oldF+SpecNew*newF)*h;
    FIntegral      += (oldF+newF)*h;

    oldF=newF;
    SpecOld=SpecNew;
   }

  *SpecConv=(FIntegral!=(double)0.)?(double)crossFIntegral/FIntegral:(double)1.;

  // Return

  return rc;
 }

RC XSCONV_TypeStandardFFT(FFT *pFFT,INT fwhmType,double slitParam,double slitParam2,double *lembda,double *target,INT size)
 {
  // Declarations

  double F,G,w,a,sigma,delta,step;
  INDEX i;
  INT ndemi;
  RC rc;

  // Initializations

  if ((slitParam>(double)0.) &&
     ((fwhmType==SLIT_TYPE_GAUSS) || ((fwhmType==SLIT_TYPE_ERF) && (slitParam2>(double)0.))))
   {
    ndemi=pFFT->fftSize>>1;
    step=(pFFT->fftIn[pFFT->oldSize]-pFFT->fftIn[1])/(pFFT->oldSize-1.);

    sigma=slitParam*0.5;
    a=sigma/sqrt(log(2.));
    delta=slitParam2*0.5;

    w=(double)PI/step;
    F=exp(-a*a*w*w*0.25);
    G=(fwhmType==SLIT_TYPE_GAUSS)?(double)1.:sin(w*delta)/(w*delta);

    pFFT->invFftIn[1]=pFFT->fftOut[1];
    pFFT->invFftIn[2]=pFFT->fftOut[2]*F*G;

    for (i=2;i<=ndemi;i++)
     {
      w=(double)PI*(i-1.)/(ndemi*step);

      F=(double)exp(-a*a*w*w*0.25);
      G=(double)(fwhmType==SLIT_TYPE_GAUSS)?(double)1.:(double)sin(w*delta)/(w*delta);

      pFFT->invFftIn[(i<<1) /* i*2 */-1]=pFFT->fftOut[(i<<1) /* i*2 */-1]*F*G;      // Real part
      pFFT->invFftIn[(i<<1) /* i*2 */]=pFFT->fftOut[(i<<1) /* i*2 */]*F*G;          // Imaginary part
     }

    realft(pFFT->invFftIn,pFFT->invFftOut,pFFT->fftSize,-1);

//    for (i=1;i<=pFFT->fftSize;i++)
//     pFFT->invFftOut[i]/=step;

    if (!(rc=SPLINE_Deriv2(pFFT->fftIn+1,pFFT->invFftOut+1,pFFT->invFftIn+1,pFFT->oldSize,"XSCONV_TypeStandardFFT ")))
     rc=SPLINE_Vector(pFFT->fftIn+1,pFFT->invFftOut+1,pFFT->invFftIn+1,pFFT->oldSize,lembda,target,size,SPLINE_CUBIC,"XSCONV_TypeStandardFFT ");
   }
  else
   rc=ERROR_SetLast("XSCONV_TypeStandardFFT",ERROR_TYPE_WARNING,ERROR_ID_BAD_ARGUMENTS);

  // Return

  return rc;
 }

// --------------------------------------------------------------------------------
// XSCONV_TypeStandard : Standard convolution of cross section with a slit function
// --------------------------------------------------------------------------------

//
// RC XSCONV_TypeStandard(XS *pXsnew,XS *pXshr,XS *pSlit,XS *pI,double *Ic,
//                        INT slitType,double slitWidth,double slitParam)
//
// with :
//
//  - pXsnew->lembda : final wavelength scale (input);
//  - pXsnew->vector : pXshr->vector after convolution (output);
//
//  - pXshr : cross section high resolution (wavelength scale,slit vector and second derivatives);
//  - pSlit : slit function (wavelength scale, slit vector and second derivatives);
//
//  - pI,Ic : these extra parameters are mainly used when I0 correction is applied in order to speed up total convolution
//            work because integrals of I and I0 can be computed simultaneously;
//
//    if I0 correction is applied, Ic is the I convoluted vector;
//    if no I0 correction is applied, Ic is set to NULL but pI is set to pXshr in order to avoid extra tests in loops;
//
//    the computed integral is then the same as pXshr's one and is not used;
//
//  - slitType : type of slit function;
//  - slitWidth : slit half base width;
//  - slitParam : for gaussian slit functions, half way up slit width.
//
// NB : pI->lembda==pXshr->lembda.
//

RC GetNewF(double *pNewF,
           INT     slitType,
           double *slitLembda,
           double *slitVector,
           double *slitDeriv2,
           INT     slitNDET,
           double  dist,
           double  slitParam,
           double  slitParam2,
           double  slitParam3,
           double  slitParam4,
           double  step)
 {
  double sigma,a,a2,newF,norm1,norm2,delta;
  INT rc;

  newF=(double)0.;
  sigma=slitParam*0.5;
  a=sigma/sqrt(log(2.));
  a2=slitParam3*0.5/sqrt(log(2.));
  delta=slitParam2*0.5;

  rc=ERROR_ID_NO;

  if ((slitType==SLIT_TYPE_VOIGT) && (slitParam3!=(double)0.))
   {
    norm1=(double)Voigtx((double)0.,slitParam2);
    norm2=(double)Voigtx((double)0.,slitParam4);

    if ((norm1==(double)0.) || (norm2==(double)0.))
     rc=ERROR_SetLast("GetNewF",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"calculation of the voigt function");
    else
     {
      norm1=(double)1./norm1;
      norm2=(double)1./norm2;
     }
   }
  else
   norm1=norm2=(double)1.;

  if ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_INVPOLY_FILE))
   newF=(double)pow(sigma,slitParam2)/(pow(dist,slitParam2)+pow(sigma,slitParam2));
  else if (((slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_ERF_FILE)) && (slitParam2!=(double)0.))
   newF=(double)(ERF_GetValue((dist+delta)/a)-ERF_GetValue((dist-delta)/a))/(4.*delta);
  else if ((slitType==SLIT_TYPE_VOIGT) ) // || (slitType===SLIT_TYPE_VOIGT_FILE))
   {
    if ((dist>(double)0.) && (slitParam3!=(double)0.))
     newF=(double)Voigtx(dist/a2,slitParam4)*norm2;
    else
     newF=(double)Voigtx(dist/a,slitParam2)*norm1;
   }
  else if (slitType==SLIT_TYPE_FILE)
   rc=SPLINE_Vector(slitLembda,slitVector,slitDeriv2,slitNDET,&dist,&newF,1,SPLINE_CUBIC,"GetNewF ");
  else if (slitType==SLIT_TYPE_APOD)
   rc=XsconvFctApod(&newF,slitParam,slitParam2,0.01,dist);
  else if (slitType==SLIT_TYPE_APODNBS)
   rc=XsconvFctApodNBS(&newF,slitParam,slitParam2,0.01,dist);
  else // if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_GAUSS_FILE))
   newF=(double)exp(-4.*log(2.)*(dist*dist)/(slitParam*slitParam));

//   rc=XsconvFctGauss(&newF,slitParam,step,dist);

  *pNewF=newF;

  return rc;
 }

RC XSCONV_TypeStandard(XS *pXsnew,INDEX indexLembdaMin,INDEX indexLembdaMax,XS *pXshr,XS *pSlit,XS *pI,double *Ic,INT slitType,double slitWidth,double slitParam,double slitParam2,double slitParam3,double slitParam4)
 {
  // Declarations

  double *xsnewLembda,*xsnewVector,
         *xshrLembda,*xshrVector,*xshrDeriv2,
         *slitLembda,*slitVector,*slitDeriv2,dist,
         *IVector,*IDeriv2,
          crossFIntegral,IFIntegral,FIntegral,
          oldF,newF,oldIF,newIF,stepF,h,
          slitCenter,c,si,
          lembda,lembdaMin,lembdaMax,oldXshr,newXshr;
  INDEX   xshrPixMin,
          xsnewIndex,indexOld,indexNew,
          klo,khi,i;
  INT     xshrNDET,xsnewNDET,slitNDET,msgCount;
  RC      rc;

  // Use substitution variables

  xsnewLembda=pXsnew->lembda;
  xsnewVector=pXsnew->vector;

  xshrLembda=pXshr->lembda;
  xshrVector=pXshr->vector;
  xshrDeriv2=pXshr->deriv2;

  if (pSlit!=NULL)
   {
    slitLembda=pSlit->lembda;
    slitVector=pSlit->vector;
    slitDeriv2=pSlit->deriv2;
    slitNDET=pSlit->NDET;
   }
  else
   {
    slitLembda=slitVector=slitDeriv2=NULL;
    slitNDET=0;
   }

  xsnewNDET=pXsnew->NDET;
  xshrNDET=pXshr->NDET;

  IVector=pI->vector;
  IDeriv2=pI->deriv2;

  // Initializations

  if (slitType==SLIT_TYPE_FILE)
   stepF=slitLembda[1]-slitLembda[0];
  else if ((slitType==SLIT_TYPE_APOD) ||(slitType==SLIT_TYPE_APODNBS))
   stepF=slitParam/50.;
  else
   stepF=slitParam/6.;
  msgCount=0;

  rc=ERROR_ID_NO;

  if ((slitType==SLIT_TYPE_FILE) || (slitType==SLIT_TYPE_VOIGT))
   {
    // Calculation of the center of the slit function

    for (i=0,c=si=(double)0.;i<slitNDET;i++)
     {
      c+=(double)slitLembda[i]*slitVector[i];
      si+=(double)slitVector[i];
     }

    slitCenter=(double)c/si;
   }
  else
   {
    slitCenter=(double)0.;
    slitWidth=(double)3.*slitParam;
   }

  // Browse wavelengths in the final calibration vector

  for (xsnewIndex=max(0,indexLembdaMin);(xsnewIndex<xsnewNDET) && (xsnewIndex<indexLembdaMax) && !rc;xsnewIndex++)
   {
    lembda=xsnewLembda[xsnewIndex];

    if ((slitType==SLIT_TYPE_GAUSS_FILE) ||
        (slitType==SLIT_TYPE_INVPOLY_FILE) ||
//        (slitType==SLIT_TYPE_VOIGT_FILE) ||
        (slitType==SLIT_TYPE_ERF_FILE))
     {
      SPLINE_Vector(slitLembda,slitVector,slitDeriv2,slitNDET,&lembda,&slitParam,1,SPLINE_CUBIC,"XSCONV_TypeStandard ");

      stepF=slitParam/6.;            // 6 points/FWHM
      slitWidth=(double)3.*slitParam;
     }

    if (slitType!=SLIT_TYPE_FILE)
     {
      lembdaMin=lembda-slitWidth;
      lembdaMax=lembda+slitWidth;
     }
    else
     {
      lembdaMin=lembda+slitLembda[0];                     // add 1 nm
      lembdaMax=lembda+slitLembda[slitNDET-1];
     }

    // Search for first pixel in high resolution cross section in the wavelength range delimited by slit function

    for (klo=0,khi=xshrNDET-1;khi-klo>1;)
     {
      xshrPixMin=(khi+klo)>>1;

      if (xshrLembda[xshrPixMin]>lembdaMin)
       khi=xshrPixMin;
      else
       klo=xshrPixMin;
     }

    xshrPixMin=(xshrLembda[klo]<lembdaMin)?khi:klo;
    crossFIntegral=IFIntegral=FIntegral=(double)0.;

    // Case 1 : the resolution of cross section is better than the resolution of slit function => slit function interpolation only

    if (xshrLembda[xshrPixMin+1]-xshrLembda[xshrPixMin]<=stepF)
     {
      // set indexes to browse wavelengths in the grid of the high resolution cross section

      indexOld=xshrPixMin;
      indexNew=indexOld+1;

      // distance to the central wavelength

      dist=(double)slitCenter-(xshrLembda[indexOld]-lembda); // !!! slit function is inversed for convolution

      rc=GetNewF(&newF,slitType,slitLembda,slitVector,slitDeriv2,slitNDET,
                  dist,slitParam,slitParam2,slitParam3,slitParam4,xshrLembda[xshrPixMin+1]-xshrLembda[xshrPixMin]);

      // browse the grid of the high resolution cross section

      while ((indexNew<xshrNDET) && (xshrLembda[indexNew]<=lembdaMax) && !rc)
       {
        oldF=newF;

        dist=(double)slitCenter-(xshrLembda[indexNew]-lembda);

        // Convolution

        if (!(rc=GetNewF(&newF,slitType,slitLembda,slitVector,slitDeriv2,slitNDET,
                          dist,slitParam,slitParam2,slitParam3,slitParam4,xshrLembda[indexNew]-xshrLembda[indexOld])))
         {
          h=(xshrLembda[indexNew]-xshrLembda[indexOld])*0.5;  // use trapezium formula for surface computation (B+b)*H/2
          crossFIntegral+=(xshrVector[indexOld]*oldF+xshrVector[indexNew]*newF)*h;
          IFIntegral+=(IVector[indexOld]*oldF+IVector[indexNew]*newF)*h;
          FIntegral+=(oldF+newF)*h;
         }

        indexOld=indexNew++;
       }
     }

    // Case 2 : the resolution of slit function is better than the resolution of cross section => cross section interpolation

    else

// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???       if (msgCount || (THRD_id!=THREAD_TYPE_NONE) ||
// QDOAS ???             ((rc=MSG_MessageBox(DOAS_hwndMain,ITEM_NONE,MENU_TOOLS_CONVOLUTION,
// QDOAS ???              IDS_MSGBOX_XSCONV_GRID,MB_YESNO|MB_ICONHAND))==IDYES))
// QDOAS ??? #endif

     {
      msgCount=1;

      // set indexes to browse wavelengths in the grid of the slit function if pre-calculated

      indexOld=0;
      indexNew=1;
      rc=0;

      // Calculate first value for the slit function

      if (slitType==SLIT_TYPE_FILE)
       {
        dist=lembda+slitLembda[indexOld];
        oldF=slitVector[indexOld];
       }
      else
       {
        dist=lembda-slitWidth;

        if ((rc=GetNewF(&oldF,slitType,slitLembda,slitVector,slitDeriv2,slitNDET,
                        dist-lembda,slitParam,slitParam2,slitParam3,slitParam4,stepF))!=ERROR_ID_NO)

         goto EndTypeStandard;
       }

      // Calculate first value for the high resolution cross section

      oldXshr=(double)0.;

      if ((dist<xshrLembda[0]) || (dist>xshrLembda[xshrNDET-1]))
       oldF=oldIF=oldXshr=(double)0.;
      else if (!(rc=SPLINE_Vector(xshrLembda,xshrVector,xshrDeriv2,xshrNDET,&dist,&oldXshr,1,SPLINE_CUBIC,"XSCONV_TypeStandard ")) && (Ic!=NULL))
       rc=SPLINE_Vector(xshrLembda,IVector,IDeriv2,xshrNDET,&dist,&oldIF,1,SPLINE_CUBIC,"XSCONV_TypeStandard ");

      // browse the grid of the slit function

      while ((((slitType==SLIT_TYPE_FILE) && (indexNew<slitNDET)) ||
              ((slitType!=SLIT_TYPE_FILE) && (dist+stepF<=lembda+slitWidth))) && !rc)
       {
        // the slit function is pre-calculated

        if (slitType==SLIT_TYPE_FILE)
         {
          dist=lembda+slitLembda[indexNew];
          newF=slitVector[indexNew];

          h=fabs((slitLembda[indexNew]-slitLembda[indexOld])*0.5);
          indexOld=indexNew++;
         }

        // the slit function is calculated now

        else
         {
          dist+=stepF;
          h=stepF*0.5;

          if ((rc=GetNewF(&newF,slitType,slitLembda,slitVector,slitDeriv2,slitNDET,
                           dist-lembda,slitParam,slitParam2,slitParam3,slitParam4,stepF))!=ERROR_ID_NO)

           goto EndTypeStandard;
         }

        if ((dist>=xshrLembda[0]) && (dist<=xshrLembda[xshrNDET-1]))
         {
          // interpolation of the high resolution cross section

          if ((rc=SPLINE_Vector(xshrLembda,xshrVector,xshrDeriv2,xshrNDET,&dist,&newXshr,1,SPLINE_CUBIC,"XSCONV_TypeStandard "))!=0)
           goto EndTypeStandard;

          // Convolution

          crossFIntegral+=(oldF*oldXshr+newF*newXshr)*h;
          FIntegral+=(oldF+newF)*h;

          // I0 correction

          if ((Ic!=NULL) && !(rc=SPLINE_Vector(xshrLembda,IVector,IDeriv2,xshrNDET,&dist,&newIF,1,SPLINE_CUBIC,"XSCONV_TypeStandard ")))
           {
            IFIntegral+=(oldF*oldIF+newF*newIF)*h;
            oldIF=newIF;
           }
         }
        else
         newF=newXshr=(double)0.;

        oldXshr=newXshr;
        oldF=newF;
       }
     }

    xsnewVector[xsnewIndex]=(FIntegral!=(double)0.)?crossFIntegral/FIntegral:(double)0.;

    if (Ic!=NULL)
     Ic[xsnewIndex]=(FIntegral!=(double)0.)?IFIntegral/FIntegral:(double)0.;
   }

  EndTypeStandard :

  // Return

  return rc;
 }

// -------------------------------------------------------------------------
// XsconvTypeI0Correction : Convolution of cross sections with I0 correction
// -------------------------------------------------------------------------

RC XsconvTypeI0Correction(XS *pXsnew,XS *pXshr,XS *pI0,XS *pSlit,double conc,INT slitType,double slitWidth,double slitParam,double slitParam2,double slitParam3,double slitParam4)
 {
  // Declarations

  double  sigma,
         *xshrLembda,*xshrVector,*xshrDeriv2,
         *xsnewLembda,*xsnewVector,
         *ILembda,*IVector,*IDeriv2,
         *I0Lembda,*I0Vector,*I0cVector,*IcVector;

  INT INDET,xshrNDET,xsnewNDET;
  INDEX i;
  XS I,I0c;
  RC rc;

  // Change cursor appearance

// QDOAS ???  #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???  SetCursor(xsconvHCurHourGlass);
// QDOAS ???  #endif

  // Use substitution variables

  I0Lembda=pI0->lembda;
  I0Vector=pI0->vector;

  xshrLembda=pXshr->lembda;
  xshrVector=pXshr->vector;
  xshrDeriv2=pXshr->deriv2;

  xsnewLembda=pXsnew->lembda;
  xsnewVector=pXsnew->vector;

  xshrNDET=pXshr->NDET;
  xsnewNDET=pXsnew->NDET;

  // Initializations

  memset(&I,0,sizeof(XS));
  memset(&I0c,0,sizeof(XS));

  INDET=pI0->NDET;
  IcVector=NULL;
  rc=ERROR_ID_NO;

  if (conc<=(double)0.)

   rc=ERROR_SetLast("XsconvTypeI0Correction",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS);

  // Buffers allocation

  else if (XSCONV_Alloc(&I,INDET,1) ||
           XSCONV_Alloc(&I0c,xsnewNDET,0) ||
         ((IcVector=(double *)MEMORY_AllocDVector("XsconvTypeI0Correction ","IcVector",0,xsnewNDET-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    memcpy(I0c.lembda,xsnewLembda,sizeof(double)*xsnewNDET);

    // Use substitution variables

    ILembda=I.lembda;
    IVector=I.vector;
    IDeriv2=I.deriv2;

    I0cVector=I0c.vector;

    // Build I from I0 (solar spectrum) with the specified cross section absorption

    memcpy(ILembda,I0Lembda,sizeof(double)*INDET);

    VECTOR_Init(I0cVector,(double)0.,sizeof(double));
    VECTOR_Init(IcVector,(double)0.,sizeof(double));

    for (i=0;(i<INDET) && !rc;i++)
     if (!(rc=SPLINE_Vector(xshrLembda,xshrVector,xshrDeriv2,xshrNDET,&I0Lembda[i],&sigma,1,SPLINE_CUBIC,"XsconvTypeI0Correction ")))
      {
       if (-sigma*conc>(double)700.)
        rc=ERROR_SetLast("XsconvTypeI0Correction",ERROR_TYPE_FATAL,ERROR_ID_OVERFLOW);
       else
        IVector[i]=(double)I0Vector[i]*exp(-sigma*conc);
      }

    // I and I0 convolution

    if (!rc &&
        !(rc=SPLINE_Deriv2(ILembda,IVector,IDeriv2,INDET,"XsconvTypeI0Correction ")) &&               // I second derivatives calculation
        !(rc=XSCONV_TypeStandard(&I0c,0,xsnewNDET,pI0,pSlit,&I,IcVector,slitType,slitWidth,slitParam,slitParam2,slitParam3,slitParam4)))    // I0 and I convolution
     {
      // Cross section convolution

      for (i=0;(i<xsnewNDET) && !rc;i++)
       {
        if ((I0cVector[i]<=(double)0.) || (IcVector[i]<=(double)0.) || (conc<=(double)0.))
         xsnewVector[i]=(double)0.;
//         THRD_Error(ERROR_TYPE_FATAL,(rc=ERROR_ID_LOG),"XsconvTypeI0Correction ");
        else
         xsnewVector[i]=(double)log(I0cVector[i]/IcVector[i])/conc;
       }
     }
   }

  // Release allocated buffers

  XSCONV_Reset(&I);
  XSCONV_Reset(&I0c);

  if (IcVector!=NULL)
   MEMORY_ReleaseDVector("XsconvTypeI0Correction ","IcVector",IcVector,0);

  // Return

  return rc;
 }

// --------------------------------------------------------
// XSCONV_RealTimeXs : real time cross sections convolution
// --------------------------------------------------------

RC XSCONV_RealTimeXs(XS *pXshr,XS *pXsI0,XS *pSlit,double *IcVector,                         // high resolution cross section
                     double *lembda,INDEX indexLembdaMin,INDEX indexLembdaMax,               // final calibration wavelength scale
                     double *newXs,                                                          // convoluted cross section
                     INT slitType,double slitParam,double slitParam2,double slitParam3,double slitParam4)      // slit options
 {
  // Declarations

  XS xsnew;
  RC rc;

  // Use substitution variables

  xsnew.lembda=lembda;
  xsnew.vector=newXs;
  xsnew.NDET=NDET;

  // process convolution

  if ((pSlit==NULL) || !(rc=XsconvGetFwhm(pSlit,slitType,&slitParam)))
   rc=XSCONV_TypeStandard(&xsnew,indexLembdaMin,indexLembdaMax,pXshr,pSlit,(pXsI0==NULL)?pXshr:pXsI0,IcVector,slitType,(double)3.*slitParam,slitParam,slitParam2,slitParam3,slitParam4);

  // Return

  return rc;
 }

// ---------------------------------------------------------------------------------
// XsconvRebuildSlitFunction : Rebuild slit function onto a regular wavelength scale
// ---------------------------------------------------------------------------------

RC XsconvRebuildSlitFunction(double *lembda,double *slit,INT nslit,SLIT *pSlit,XS *pSlitXs)
 {
  // Declarations

  double slitParam,n,sigma,delta,a;
  INDEX i;
  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Build deconvolution slit function

  if (pSlit->slitType==SLIT_TYPE_GAUSS)
   for (i=0,slitParam=pSlit->slitParam*pSlit->slitParam;i<nslit;i++)
    slit[i]=(double)exp(-4.*log(2.)*(lembda[i]*lembda[i])/slitParam);

  else if (pSlit->slitType==SLIT_TYPE_INVPOLY)
   for (i=0,sigma=pSlit->slitParam*0.5,n=(double)pSlit->slitParam2;i<nslit;i++)
    slit[i]=(double)pow(sigma,n)/(pow(lembda[i],n)+pow(sigma,n));

  else if (pSlit->slitType==SLIT_TYPE_ERF)
   for (i=0,sigma=pSlit->slitParam*0.5,a=(double)sigma/sqrt(log(2.)),delta=(double)pSlit->slitParam2*0.5;i<nslit;i++)
    slit[i]=(double)(ERF_GetValue((lembda[i]+delta)/a)-ERF_GetValue((lembda[i]-delta)/a))/(4.*delta);

  else // slit type == SLIT_TYPE_FILE
   rc=SPLINE_Vector(pSlitXs->lembda,pSlitXs->vector,pSlitXs->deriv2,pSlitXs->NDET,lembda,slit,nslit,SPLINE_CUBIC,"XsconvRebuildSlitFunction ");

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------------------------
// XsconvPowFFTMin : return index of the first minimum found in the power spectrum obtained by FFT
// -----------------------------------------------------------------------------------------------

INDEX XsconvPowFFTMin(double *fft,INT n2)
 {
  // Declarations

  double minold,minnew;
  INDEX i,ndemi;

  // Initialization

  minold=(double)fft[1]*fft[1];  // the lowest frequency (no imaginary part)

  // Search for the minimum of the power spectrum

  for (i=2,ndemi=n2/2;i<=ndemi;i++,minold=minnew)

   if ((minnew=fft[(i<<1) /* i*2 */-1]*fft[(i<<1) /* i*2 */-1]+        // real part
               fft[(i<<1) /* i*2 */]*fft[(i<<1) /* i*2 */])>=minold)   // imaginary part

    break;

  if ((--i==ndemi) && (fft[2]*fft[2]<minold))
   i++;

  // Return

  return i;
 }

// ----------------------------------------
// XsconvFreqFilter : Frequencies filtering
// ----------------------------------------

double XsconvFreqFilter(double freq,double fc,double bp)
 {
  double f;

  if (freq<fc-bp)
   f=(double)1.;
  else if (freq<=fc+bp)
   f=(double)0.5*(1.+cos(PIDEMI*(1+(freq-fc)/bp)));
  else
   f=(double)0.;

  return f;
 }

// ------------------------------------------------------------------------------------------------------
// XsconvNewSlitFunction : Replace slit function by a new one when a deconvolution slit function is given
// ------------------------------------------------------------------------------------------------------

RC XsconvNewSlitFunction(HWND hwndXsconv,SLIT *pSlitOptions,XS *pSlit,double slitParam,SLIT *pSlit2Options,XS *pSlit2,double slitParam2)
 {
  // Declarations

  double *slit,*slit2,*newSlit,*lembda,              // substitution vectors
         *slitFFTin,*slit2FFTin,                     // FFT input vectors
         *slitFFTout,*slit2FFTout,                   // FFT output vectors
          slitStep,fc,bp,
          norm,norm2,                                // normalization
          a,b,c,d,div;                               // substitution variables for complex calculation
  INDEX i,i0;
  INT n2,ndemi,nslit,nslitOld;
  RC rc;

  // Initializations

  slit=slit2=newSlit=slitFFTin=slit2FFTin=slitFFTout=slit2FFTout=NULL;
  rc=ERROR_ID_NO;

  // With the slit function width half way up, give a preliminary sampling

  slitStep=slitParam2*0.2;                                // force a sampling about five pixels at half way up
  nslit=(int)floor(slitParam*4./slitStep);                // convert the step from wavelength to a number of pixels
  nslit=(nslit%2!=0)?nslit:nslit+1;                        // force the number of points to be odd

  // Buffers allocation

  if (((lembda=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction ","lembda",0,nslit))==NULL) ||
      ((slit=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction ","slit",0,nslit))==NULL) ||
      ((slit2=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction ","slit2",0,nslit))==NULL) ||
      ((newSlit=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction ","newSlit",0,nslit))==NULL) ||
      ((slitFFTin=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction ","slitFFTin",1,
              (n2=(int)pow((double)2.,ceil(log((double)nslit)/log((double)2.))))))==NULL) ||

      ((slit2FFTin=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction ","slit2FFTin",1,n2))==NULL) ||
      ((slitFFTout=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction ","slitFFTout",1,n2))==NULL) ||
      ((slit2FFTout=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction ","slit2FFTout",1,n2))==NULL))

   rc=ERROR_ID_ALLOC;

  // Both convolution and deconvolution slit functions are gaussian functions

  else if (((pSlit2Options->slitType==SLIT_TYPE_GAUSS) && (pSlitOptions->slitType==SLIT_TYPE_GAUSS)) ||
           ((pSlit2Options->slitType==SLIT_TYPE_ERF) && (pSlitOptions->slitType==SLIT_TYPE_ERF) && (pSlit2Options->slitParam2==pSlitOptions->slitParam2)))
   {
    if (slitParam2<slitParam)

     for (i=0,i0=nslit>>1,slitParam=slitParam*slitParam-slitParam2*slitParam2;i<nslit;i++)
      {
       lembda[i]=(double)slitStep*(i-i0);
       newSlit[i]=(double)exp(-4.*log(2.)*(lembda[i]*lembda[i])/slitParam);
      }
    else
     {
     	rc=ERROR_SetLast("XsconvNewSlitFunction",ERROR_TYPE_FATAL,ERROR_ID_GAUSSIAN,slitParam,slitParam2);
      goto EndNewSlit;
     }
   }
  else
   {
    // ==========
    // First pass
    // ==========

    // Resample deconvolution slit function onto the regular stepped wavelength scale

    for (i=0,i0=nslit>>1;i<nslit;i++)
     lembda[i]=(double)slitStep*(i-i0);

    if ((rc=XsconvRebuildSlitFunction(lembda,slit2,nslit,pSlit2Options,pSlit2))!=0)
     goto EndNewSlit;

    // Vector normalization

    VECTOR_Init(slit2FFTin+1,(double)0.,n2);

    for (i=1,norm2=(double)0.;i<nslit;i++)
     norm2+=(slit2[i]+slit2[i-1]);

    if (norm2==(double)0.)
     {
     	rc=ERROR_SetLast("XsconvNewSlitFunction",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"calculation of a norm");
      goto EndNewSlit;
     }

    norm2=(double)1./(norm2*0.5*slitStep);
    ndemi=nslit/2;

    // Split deconvolution function for FFT calculation

    slit2FFTin[1]=slit2[ndemi]*norm2;

    for (i=ndemi+1;i<nslit;i++)
     {
      slit2FFTin[i-ndemi+1]=slit2[i]*norm2;
      slit2FFTin[n2-(i-ndemi)+1]=slit2[nslit-i-1]*norm2;
     }

    // FFT calculation of deconvolution slit function

    realft(slit2FFTin,slit2FFTout,n2,1);

    // Search for the first minimum of power spectrum

    if (((i=XsconvPowFFTMin(slit2FFTout,n2))<=ndemi) && (i>1))
     {
      nslitOld=nslit;
      slitStep=(double)ceil(100*(ndemi*slitStep)/(i-1))*0.01;

      nslit=(int)floor(slitParam*4./slitStep);                 // convert the step from wavelength to a number of pixels
      nslit=(nslit%2!=0)?nslit:nslit+1;                        // force the number of points to be odd

      if (nslit>nslitOld)
       {
        // Release allocated buffers

        if (lembda!=NULL)
         MEMORY_ReleaseDVector("XsconvNewSlitFunction ","lembda",lembda,0);
        if (slit!=NULL)
         MEMORY_ReleaseDVector("XsconvNewSlitFunction ","slit",slit,0);
        if (slit2!=NULL)
         MEMORY_ReleaseDVector("XsconvNewSlitFunction ","slit2",slit2,0);
        if (newSlit!=NULL)
         MEMORY_ReleaseDVector("XsconvNewSlitFunction ","newSlit",newSlit,0);
        if (slitFFTin!=NULL)
         MEMORY_ReleaseDVector("XsconvNewSlitFunction ","slitFFTin",slitFFTin,1);
        if (slit2FFTin!=NULL)
         MEMORY_ReleaseDVector("XsconvNewSlitFunction ","slit2FFTin",slit2FFTin,1);
        if (slitFFTout!=NULL)
         MEMORY_ReleaseDVector("XsconvNewSlitFunction ","slitFFTout",slitFFTout,1);
        if (slit2FFTout!=NULL)
         MEMORY_ReleaseDVector("XsconvNewSlitFunction ","slit2FFTout",slit2FFTout,1);

        slit=slit2=newSlit=slitFFTin=slit2FFTin=slitFFTout=slit2FFTout=NULL;

        // Allocate buffers with the new size

        if (((lembda=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction (2) ","lembda",0,nslit))==NULL) ||
            ((slit=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction (2) ","slit",0,nslit))==NULL) ||
            ((slit2=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction (2) ","slit2",0,nslit))==NULL) ||
            ((newSlit=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction (2) ","newSlit",0,nslit))==NULL) ||
            ((slitFFTin=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction (2) ","slitFFTin",1,
                    (n2=(int)pow((double)2.,ceil(log((double)nslit)/log((double)2.))))))==NULL) ||

            ((slit2FFTin=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction (2) ","slit2FFTin",1,n2))==NULL) ||
            ((slitFFTout=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction (2) ","slitFFTout",1,n2))==NULL) ||
            ((slit2FFTout=(double *)MEMORY_AllocDVector("XsconvNewSlitFunction (2) ","slit2FFTout",1,n2))==NULL))
         {
          rc=ERROR_ID_ALLOC;
          goto EndNewSlit;
         }
       }
     }

    // ===========
    // Second pass
    // ===========

    // Resample convolution and deconvolution slit functions onto the regular stepped wavelength scale

    for (i=0,i0=nslit>>1;i<nslit;i++)
     lembda[i]=(double)slitStep*(i-i0);

    // Build deconvolution slit function

    if (((rc=XsconvRebuildSlitFunction(lembda,slit,nslit,pSlitOptions,pSlit))!=0) ||
        ((rc=XsconvRebuildSlitFunction(lembda,slit2,nslit,pSlit2Options,pSlit2))!=0))

     goto EndNewSlit;

    // Vectors initializations

    VECTOR_Init(slitFFTin+1,(double)0.,n2);
    VECTOR_Init(slit2FFTin+1,(double)0.,n2);

    // Normalization factors calculation

    for (i=1,norm=norm2=(double)0.;i<nslit;i++)
     {
      norm+=slit[i]+slit[i-1];
      norm2+=slit2[i]+slit2[i-1];
     }

    if ((norm==(double)0.) || (norm2==(double)0.))
     {
     	rc=ERROR_SetLast("XsconvNewSlitFunction",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"calculation of a norm");
      goto EndNewSlit;
     }

    norm=(double)1./(norm*0.5*slitStep);
    norm2=(double)1./(norm2*0.5*slitStep);

    ndemi=nslit/2;

    slitFFTin[1]=slit[ndemi]*norm;
    slit2FFTin[1]=slit2[ndemi]*norm2;

    for (i=ndemi+1;i<nslit;i++)
     {
      slitFFTin[i-ndemi+1]=slit[i]*norm;
      slitFFTin[n2-(i-ndemi)+1]=slit[nslit-i-1]*norm;
      slit2FFTin[i-ndemi+1]=slit2[i]*norm2;
      slit2FFTin[n2-(i-ndemi)+1]=slit2[nslit-i-1]*norm2;
     }

    // FFT calculations of slit functions

    realft(slitFFTin,slitFFTout,n2,1);
    realft(slit2FFTin,slit2FFTout,n2,1);

    // Convolution slit function filtering

    if (((i=XsconvPowFFTMin(slitFFTout,n2))<=(ndemi=n2/2)) && (i>1))
     {
      fc=bp=(double)(i-1)/(n2*slitStep);

      fc*=(double)0.8;
      bp*=(double)0.2;

      slitFFTout[1]*=XsconvFreqFilter((double)1./(n2*slitStep),fc,bp);
      slitFFTout[2]*=XsconvFreqFilter((double)(ndemi+1.)/(n2*slitStep),fc,bp);

      for (i=2;i<=ndemi;i++)
       {
        slitFFTout[(i<<1)-1]*=XsconvFreqFilter((double)(i-1)/(n2*slitStep),fc,bp);   // real part
        slitFFTout[(i<<1)]*=XsconvFreqFilter((double)(i-1)/(n2*slitStep),fc,bp);     // imaginary part
       }
     }

    // Complex division slitFFTout/slit2FFTout

    slitFFTout[1]/=(double)slit2FFTout[1];   // the lowest frequency (no imaginary part)
    slitFFTout[2]/=(double)slit2FFTout[2];   // the highest frequency (no imaginary part)

    for (i=2;i<=ndemi;i++)
     {
      // Use substitution variables

      a=slitFFTout[(i<<1) /* i*2 */-1];   // real part
      b=slitFFTout[(i<<1) /* i*2 */];     // imaginary part
      c=slit2FFTout[(i<<1) /* i*2 */-1];  // real part
      d=slit2FFTout[(i<<1) /* i*2 */];    // imaginary part

      // Perform (a+bi)/(c+di)=(a+bi)*(c-di)/(c2+d2)=(ac+bd)/(c2+d2)+i*(bc-ad)/(c2+d2)

      if ((div=c*c+d*d)!=(double)0.)
       {
        slitFFTout[(i<<1) /* i*2 */-1]=(a*c+b*d)/div;
        slitFFTout[(i<<1) /* i*2 */]=(b*c-a*d)/div;
       }
     }

    // FFT inverse calculation for the effective slit function

    realft(slitFFTout,slitFFTin,n2,-1);

    // Rebuild the new slit function

    newSlit[(ndemi=nslit/2)]=slitFFTin[1];

    for (i=ndemi+1;i<nslit;i++)
     {
      newSlit[i]=slitFFTin[i-ndemi+1];
      newSlit[nslit-i-1]=slitFFTin[n2-(i-ndemi-1)];
     }
   }

// QDOAS ???  #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ???  if (!rc && (CHILD_list[CHILD_WINDOW_SPECTRA].hwndChild!=NULL))
// QDOAS ???   {
// QDOAS ???    UCHAR str[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???
// QDOAS ???    sprintf(str,"Convolution tool : effective slit function (%d x %.4f)",nslit,slitStep);
// QDOAS ???
// QDOAS ???    SendMessage(CHILD_hwndFrame,WM_MDIACTIVATE,(WPARAM)CHILD_list[CHILD_WINDOW_SPECTRA].hwndChild,(LPARAM)0);
// QDOAS ???
// QDOAS ???    DRAW_Spectra(CHILD_WINDOW_SPECTRA,str,"","Wavelength (nm)","",NULL,0,
// QDOAS ???                 (double)0.,(double)0.,(double)0.,(double)0.,
// QDOAS ???                 lembda,newSlit,nslit,DRAW_COLOR1,0,nslit-1,PS_SOLID,NULL,
// QDOAS ???                 NULL,NULL,0,0,0,0,0,NULL,
// QDOAS ???                 0,1,1,1);
// QDOAS ???
// QDOAS ???    if (MessageBox(hwndXsconv,"Accept effective slit function ? ","Convolution tool",MB_YESNO)==IDNO)
// QDOAS ???     {
// QDOAS ???      rc=THREAD_EVENT_STOP;
// QDOAS ???      goto EndNewSlit;
// QDOAS ???     }
// QDOAS ???   }
// QDOAS ???
// QDOAS ???   #endif

  // Release previous buffers

  XSCONV_Reset(pSlit);

  // Allocate

  if (XSCONV_Alloc(pSlit,nslit,1))
   rc=ERROR_ID_ALLOC;
  else
   {
    memcpy(pSlit->lembda,lembda,sizeof(double)*nslit);
    memcpy(pSlit->vector,newSlit,sizeof(double)*nslit);

    rc=SPLINE_Deriv2(pSlit->lembda,pSlit->vector,pSlit->deriv2,nslit,"XsconvNewSlitFunction ");
   }

  EndNewSlit :

  // Release allocated buffers

  if (lembda!=NULL)
   MEMORY_ReleaseDVector("XsconvNewSlitFunction ","lembda",lembda,0);
  if (slit!=NULL)
   MEMORY_ReleaseDVector("XsconvNewSlitFunction ","slit",slit,0);
  if (slit2!=NULL)
   MEMORY_ReleaseDVector("XsconvNewSlitFunction ","slit2",slit2,0);
  if (newSlit!=NULL)
   MEMORY_ReleaseDVector("XsconvNewSlitFunction ","newSlit",newSlit,0);
  if (slitFFTin!=NULL)
   MEMORY_ReleaseDVector("XsconvNewSlitFunction ","slitFFTin",slitFFTin,1);
  if (slit2FFTin!=NULL)
   MEMORY_ReleaseDVector("XsconvNewSlitFunction ","slit2FFTin",slit2FFTin,1);
  if (slitFFTout!=NULL)
   MEMORY_ReleaseDVector("XsconvNewSlitFunction ","slitFFTout",slitFFTout,1);
  if (slit2FFTout!=NULL)
   MEMORY_ReleaseDVector("XsconvNewSlitFunction ","slit2FFTout",slit2FFTout,1);

  // Return

  return rc;
 }

// QDOAS ??? // ----------------------------------------------
// QDOAS ??? // XSCONV_Convolution : Main convolution function
// QDOAS ??? // ----------------------------------------------
// QDOAS ???
// QDOAS ??? RC XSCONV_Convolution(HWND hwndXsconv)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR windowTitle[MAX_ITEM_TEXT_LEN+1],graphTitle[MAX_ITEM_TEXT_LEN+1],fileName[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   double lembdaMin,lembdaMax,slitParam,slitParam2,slitWidth,*filterVector;
// QDOAS ???   PRJCT_FILTER *plFilter,*phFilter;
// QDOAS ???   SLIT *pSlitConv,*pSlitDConv;
// QDOAS ???   INT slitType,slitType2,deconvFlag,dispConv,dispLFilter,dispHFilter,nGraph,iGraph;
// QDOAS ???   INT lowFilterType,highFilterType,nFilter;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pSlitDConv=&XSCONV_options.slitDConv;
// QDOAS ???   pSlitConv=&XSCONV_options.slitConv;
// QDOAS ???   slitType=pSlitConv->slitType;
// QDOAS ???   slitType2=pSlitDConv->slitType;
// QDOAS ???   slitParam=slitParam2=(double)0.;
// QDOAS ???
// QDOAS ???   // Filtering
// QDOAS ???
// QDOAS ???   plFilter=&XSCONV_options.lfilter;
// QDOAS ???   phFilter=&XSCONV_options.hfilter;
// QDOAS ???
// QDOAS ???   lowFilterType=plFilter->type;            // low pass filtering
// QDOAS ???   highFilterType=phFilter->type;           // high pass filtering
// QDOAS ???
// QDOAS ???   plFilter->filterFunction=phFilter->filterFunction=NULL;
// QDOAS ???
// QDOAS ???   if ((((lowFilterType=plFilter->type)!=PRJCT_FILTER_TYPE_NONE) &&
// QDOAS ???         (lowFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) &&
// QDOAS ???        ((rc=ANALYSE_LoadFilter(plFilter))!=0)) ||
// QDOAS ???
// QDOAS ???       (((highFilterType=phFilter->type)!=PRJCT_FILTER_TYPE_NONE) &&
// QDOAS ???         (highFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) &&
// QDOAS ???        ((rc=ANALYSE_LoadFilter(phFilter))!=0)))
// QDOAS ???
// QDOAS ???    goto EndConvolution;
// QDOAS ???
// QDOAS ???   nFilter=0;
// QDOAS ???
// QDOAS ???   if ((lowFilterType!=PRJCT_FILTER_TYPE_NONE) && (lowFilterType!=PRJCT_FILTER_TYPE_ODDEVEN))
// QDOAS ???    nFilter+=(int)(plFilter->filterWidth*sqrt(plFilter->filterNTimes)+0.5);
// QDOAS ???   if ((highFilterType!=PRJCT_FILTER_TYPE_NONE) && (highFilterType!=PRJCT_FILTER_TYPE_ODDEVEN))
// QDOAS ???    nFilter+=(int)(phFilter->filterWidth*sqrt(phFilter->filterNTimes)+0.5);
// QDOAS ???
// QDOAS ???   filterVector=NULL;
// QDOAS ???
// QDOAS ???   // Display control
// QDOAS ???
// QDOAS ???   deconvFlag=((XSCONV_options.convolutionType!=CONVOLUTION_TYPE_NONE) && (slitType!=SLIT_TYPE_GAUSS_FILE) && (slitType!=SLIT_TYPE_INVPOLY_FILE) && (slitType!=SLIT_TYPE_ERF_FILE) &&
// QDOAS ???             (((pSlitDConv->slitType==SLIT_TYPE_FILE) && (strlen(pSlitDConv->slitFile)!=0)) ||
// QDOAS ???             (((pSlitDConv->slitType==SLIT_TYPE_GAUSS) || (pSlitDConv->slitType==SLIT_TYPE_INVPOLY) || (pSlitDConv->slitType==SLIT_TYPE_ERF)) && (pSlitDConv->slitParam>0.))))?1:0;
// QDOAS ???
// QDOAS ???   // Change current cursor into hour glass
// QDOAS ???
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   xsconvHCurHourGlass=LoadCursor(NULL,IDC_WAIT);
// QDOAS ???   xsconvHCurSave = SetCursor(xsconvHCurHourGlass);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   THRD_delay=INFINITE;
// QDOAS ???
// QDOAS ???   // Load calibration file and slit function
// QDOAS ???
// QDOAS ???   if (!(rc=XSCONV_LoadCalibrationFile(&XSCONV_xsnew,XSCONV_options.calibrationFile,nFilter)) &&
// QDOAS ???      (((lowFilterType==PRJCT_FILTER_TYPE_NONE) && (highFilterType==PRJCT_FILTER_TYPE_NONE)) ||
// QDOAS ???       ((filterVector=(double *)MEMORY_AllocDVector("XSCONV_Convolution ","filterVector",0,XSCONV_xsnew.NDET-1))!=NULL)) &&
// QDOAS ???       ((XSCONV_options.convolutionType==CONVOLUTION_TYPE_NONE) ||
// QDOAS ???      (!(rc=XSCONV_LoadSlitFunction(&XSCONV_slitFunction,&XSCONV_options.slitConv,&slitParam,&slitType)) &&
// QDOAS ???       (!deconvFlag || !(rc=XSCONV_LoadSlitFunction(&XSCONV_slitDFunction,pSlitDConv,&slitParam2,&slitType2))))))
// QDOAS ???    {
// QDOAS ???     slitWidth=(double)2.*slitParam;
// QDOAS ???
// QDOAS ???     // Window in wavelength
// QDOAS ???
// QDOAS ???     if ((slitType!=SLIT_TYPE_FILE) || (XSCONV_options.convolutionType==CONVOLUTION_TYPE_NONE))
// QDOAS ???      {
// QDOAS ???       lembdaMin=XSCONV_xsnew.lembda[0]-slitWidth-1.;                     // add 1 nm
// QDOAS ???       lembdaMax=XSCONV_xsnew.lembda[XSCONV_xsnew.NDET-1]+slitWidth+1.;
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???       lembdaMin=XSCONV_xsnew.lembda[0]+XSCONV_slitFunction.lembda[0]-1.;                     // add 1 nm
// QDOAS ???       lembdaMax=XSCONV_xsnew.lembda[XSCONV_xsnew.NDET-1]+XSCONV_slitFunction.lembda[XSCONV_slitFunction.NDET-1]+1.;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (deconvFlag)
// QDOAS ???      slitType=SLIT_TYPE_FILE;  // the resulting effective slit function works as a slit file type one
// QDOAS ???
// QDOAS ???     // Determine effective slit function when a deconvolution slit function is given
// QDOAS ???
// QDOAS ???     if ((!deconvFlag || !(rc=XsconvNewSlitFunction(hwndXsconv,pSlitConv,&XSCONV_slitFunction,slitParam,pSlitDConv,&XSCONV_slitDFunction,slitParam2))) &&
// QDOAS ???
// QDOAS ???     // Load high resolution Kurucz file in convolution with I0 correction method
// QDOAS ???
// QDOAS ???         ((XSCONV_options.convolutionType!=CONVOLUTION_TYPE_I0_CORRECTION) ||
// QDOAS ???         !(rc=XSCONV_LoadCrossSectionFile(&XSCONV_kurucz,XSCONV_options.kuruczFile,lembdaMin,lembdaMax,(double)0.,CONVOLUTION_CONVERSION_NONE))) &&
// QDOAS ???         !(rc=XSCONV_LoadCrossSectionFile(&XSCONV_xshr,XSCONV_options.crossFile,lembdaMin,lembdaMax,(double)atof(XSCONV_options.shift),XSCONV_options.conversionMode)))
// QDOAS ???      {
// QDOAS ???       dispConv=((XSCONV_options.convolutionType!=CONVOLUTION_TYPE_NONE) ||
// QDOAS ???             (XSCONV_xshr.NDET!=XSCONV_xsnew.NDET) ||
// QDOAS ???             !VECTOR_Equal(XSCONV_xshr.lembda,XSCONV_xsnew.lembda,XSCONV_xsnew.NDET,(double)1.e-7))?1:0;
// QDOAS ???
// QDOAS ???       dispLFilter=(lowFilterType!=PRJCT_FILTER_TYPE_NONE)?1:0;
// QDOAS ???       dispHFilter=(highFilterType!=PRJCT_FILTER_TYPE_NONE)?1:0;
// QDOAS ???
// QDOAS ???       nGraph=dispConv+dispLFilter+dispHFilter;
// QDOAS ???       iGraph=0;
// QDOAS ???
// QDOAS ???       // -----------
// QDOAS ???       // Convolution
// QDOAS ???       // -----------
// QDOAS ???
// QDOAS ???       switch(XSCONV_options.convolutionType)
// QDOAS ???        {
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case CONVOLUTION_TYPE_NONE :
// QDOAS ???          rc=XSCONV_TypeNone(&XSCONV_xsnew,&XSCONV_xshr);
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case CONVOLUTION_TYPE_STANDARD :
// QDOAS ???          rc=XSCONV_TypeStandard(&XSCONV_xsnew,0,XSCONV_xsnew.NDET,&XSCONV_xshr,&XSCONV_slitFunction,&XSCONV_xshr,NULL,slitType,slitWidth,slitParam,
// QDOAS ???                                  XSCONV_options.slitConv.slitParam2,XSCONV_options.slitConv.slitParam3,XSCONV_options.slitConv.slitParam4);
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???         case CONVOLUTION_TYPE_I0_CORRECTION :
// QDOAS ???           rc=XsconvTypeI0Correction(&XSCONV_xsnew,&XSCONV_xshr,&XSCONV_kurucz,&XSCONV_slitFunction,XSCONV_options.conc,slitType,slitWidth,slitParam,XSCONV_options.slitConv.slitParam2,XSCONV_options.slitConv.slitParam3,XSCONV_options.slitConv.slitParam4);
// QDOAS ???         break;
// QDOAS ???      // ----------------------------------------------------------------------
// QDOAS ???      }
// QDOAS ???
// QDOAS ???       if (rc!=0)
// QDOAS ???        goto EndConvolution;
// QDOAS ???
// QDOAS ???       #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ???       if ((CHILD_list[CHILD_WINDOW_SPECTRA].hwndChild!=NULL) && dispConv)
// QDOAS ???        {
// QDOAS ???         SendMessage(CHILD_hwndFrame,WM_MDIACTIVATE,(WPARAM)CHILD_list[CHILD_WINDOW_SPECTRA].hwndChild,(LPARAM)0);
// QDOAS ???         sprintf(windowTitle,"Original cross section : %s",FILES_RebuildFileName(fileName,XSCONV_options.crossFile,1));
// QDOAS ???
// QDOAS ???         DRAW_Spectra(CHILD_WINDOW_SPECTRA,windowTitle,
// QDOAS ???                     (XSCONV_options.convolutionType!=CONVOLUTION_TYPE_NONE)?"After convolution":"After interpolation","Wavelength (nm)","",NULL,0,
// QDOAS ???                     (double)0.,(double)0.,(double)0.,(double)0.,
// QDOAS ???                      XSCONV_xshr.lembda,XSCONV_xshr.vector,XSCONV_xshr.NDET,DRAW_COLOR2,0,XSCONV_xshr.NDET-1,PS_SOLID,"Original",
// QDOAS ???                      XSCONV_xsnew.lembda+nFilter,XSCONV_xsnew.vector+nFilter,XSCONV_xsnew.NDET-2*nFilter,DRAW_COLOR1,0,XSCONV_xsnew.NDET-2*nFilter-1,PS_SOLID,
// QDOAS ???                     (XSCONV_options.convolutionType!=CONVOLUTION_TYPE_NONE)?"Convoluted":"Interpolated",
// QDOAS ???                      iGraph,1,nGraph,(iGraph==nGraph-1)?1:1);
// QDOAS ???
// QDOAS ???         iGraph++;
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       #endif
// QDOAS ???
// QDOAS ???       if (filterVector!=NULL)
// QDOAS ???        memcpy(filterVector,XSCONV_xsnew.vector,XSCONV_xsnew.NDET*sizeof(double));
// QDOAS ???
// QDOAS ???       // ------------------
// QDOAS ???       // Low-Pass filtering
// QDOAS ???       // ------------------
// QDOAS ???
// QDOAS ???       if ((lowFilterType!=PRJCT_FILTER_TYPE_NONE) && (filterVector!=NULL) &&
// QDOAS ???         (((lowFilterType==PRJCT_FILTER_TYPE_ODDEVEN) && !(rc=THRD_OddEvenCorrection(XSCONV_xsnew.lembda,XSCONV_xsnew.vector,filterVector,XSCONV_xsnew.NDET))) ||
// QDOAS ???          ((lowFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) && !(rc=FILTER_Vector(plFilter,filterVector,filterVector,XSCONV_xsnew.NDET,PRJCT_FILTER_OUTPUT_LOW)))))
// QDOAS ???        {
// QDOAS ???        	#if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ???         SendMessage(CHILD_hwndFrame,WM_MDIACTIVATE,(WPARAM)CHILD_list[CHILD_WINDOW_SPECTRA].hwndChild,(LPARAM)0);
// QDOAS ???         sprintf(windowTitle,"Original cross section : %s",FILES_RebuildFileName(fileName,XSCONV_options.crossFile,1));
// QDOAS ???         strcpy(graphTitle,"After low-pass filtering");
// QDOAS ???
// QDOAS ???         DRAW_Spectra(CHILD_WINDOW_SPECTRA,windowTitle,graphTitle,"Wavelength (nm)","",NULL,0,
// QDOAS ???                     (double)0.,(double)0.,(double)0.,(double)0.,
// QDOAS ???                      XSCONV_xsnew.lembda+nFilter,XSCONV_xsnew.vector+nFilter,XSCONV_xsnew.NDET-2*nFilter,DRAW_COLOR2,0,XSCONV_xsnew.NDET-2*nFilter-1,PS_SOLID,"Not filtered",
// QDOAS ???                      XSCONV_xsnew.lembda+nFilter,filterVector+nFilter,XSCONV_xsnew.NDET-2*nFilter,DRAW_COLOR1,0,XSCONV_xsnew.NDET-2*nFilter-1,PS_SOLID,"Filtered",
// QDOAS ???                      iGraph,1,nGraph,(iGraph==nGraph-1)?1:0);
// QDOAS ???
// QDOAS ???         iGraph++;
// QDOAS ???
// QDOAS ???         #endif
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // -------------------
// QDOAS ???       // High-Pass filtering
// QDOAS ???       // -------------------
// QDOAS ???
// QDOAS ???       if ((highFilterType!=PRJCT_FILTER_TYPE_NONE) && (highFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) && (filterVector!=NULL) &&
// QDOAS ???          !(rc=FILTER_Vector(phFilter,filterVector,filterVector,XSCONV_xsnew.NDET,PRJCT_FILTER_OUTPUT_HIGH_SUB+phFilter->filterAction)))
// QDOAS ???        {
// QDOAS ???        	#if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ???         SendMessage(CHILD_hwndFrame,WM_MDIACTIVATE,(WPARAM)CHILD_list[CHILD_WINDOW_SPECTRA].hwndChild,(LPARAM)0);
// QDOAS ???         sprintf(windowTitle,"Original cross section : %s",FILES_RebuildFileName(fileName,XSCONV_options.crossFile,1));
// QDOAS ???         strcpy(graphTitle,"After high-pass filtering");
// QDOAS ???
// QDOAS ???         DRAW_Spectra(CHILD_WINDOW_SPECTRA,windowTitle,graphTitle,"Wavelength (nm)","",NULL,0,
// QDOAS ???                     (double)0.,(double)0.,(double)0.,(double)0.,
// QDOAS ???                      XSCONV_xsnew.lembda+nFilter,filterVector+nFilter,XSCONV_xsnew.NDET-2*nFilter,DRAW_COLOR2,0,XSCONV_xsnew.NDET-2*nFilter-1,PS_SOLID,"",
// QDOAS ???                      NULL,NULL,0,DRAW_COLOR1,0,0,PS_SOLID,"",
// QDOAS ???                      iGraph,1,nGraph,(iGraph==nGraph-1)?1:0);
// QDOAS ???
// QDOAS ???         iGraph++;
// QDOAS ???
// QDOAS ???         #endif
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // ----------------------------
// QDOAS ???       // Save resulting cross section
// QDOAS ???       // ----------------------------
// QDOAS ???
// QDOAS ???       #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???       XsconvSaveNew(hwndXsconv,&XSCONV_xsnew,filterVector,&XSCONV_options,nFilter);
// QDOAS ???       #endif
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   EndConvolution :
// QDOAS ???
// QDOAS ???   // Release allocated buffers
// QDOAS ???
// QDOAS ???   XSCONV_Reset(&XSCONV_xsnew);
// QDOAS ???   XSCONV_Reset(&XSCONV_slitFunction);
// QDOAS ???   XSCONV_Reset(&XSCONV_slitDFunction);
// QDOAS ???   XSCONV_Reset(&XSCONV_xshr);
// QDOAS ???   XSCONV_Reset(&XSCONV_kurucz);
// QDOAS ???
// QDOAS ???   if (filterVector!=NULL)
// QDOAS ???    MEMORY_ReleaseDVector("XSCONV_Convolution ","filterVector",filterVector,0);
// QDOAS ???
// QDOAS ???   if (plFilter->filterFunction!=NULL)
// QDOAS ???    {
// QDOAS ???     MEMORY_ReleaseDVector("XSCONV_Convolution ","FILTER_function",plFilter->filterFunction,1);
// QDOAS ???     plFilter->filterFunction=NULL;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if (phFilter->filterFunction!=NULL)
// QDOAS ???    {
// QDOAS ???     MEMORY_ReleaseDVector("XSCONV_Convolution ","FILTER_function",phFilter->filterFunction,1);
// QDOAS ???     phFilter->filterFunction=NULL;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   SetCursor(xsconvHCurSave);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? // ==========================================
// QDOAS ??? // CONVOLUTION DIALOG BOX MESSAGES PROCESSING
// QDOAS ??? // ==========================================
// QDOAS ???
// QDOAS ??? // GENERAL TAB PAGE MESSAGES PROCESSING
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XsconvGeneralKurucz
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Enable/Disable Kurucz fields on convolution type
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndGeneral : the handle to the current tab page
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void XsconvGeneralKurucz(HWND hwndGeneral)
// QDOAS ???  {
// QDOAS ???  	// Declaration
// QDOAS ???
// QDOAS ???   INT action;
// QDOAS ???
// QDOAS ???   // Get the convolution type
// QDOAS ???
// QDOAS ???   action=((XSCONV_buffer.convolutionType=SendMessage(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_SELECTION),CB_GETCURSEL,0,0))==CONVOLUTION_TYPE_I0_CORRECTION)?
// QDOAS ???            SW_SHOW:SW_HIDE;
// QDOAS ???
// QDOAS ???   // Hide/show the fields (Kurucz and XS concentration) related to the I0 correction according to the selected convolution type
// QDOAS ???
// QDOAS ???   ShowWindow(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_KURUCZ_BUTTON),action);
// QDOAS ???   ShowWindow(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_KURUCZ_FILE),action);
// QDOAS ???   ShowWindow(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_CONC_TEXT),action);
// QDOAS ???   ShowWindow(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_CONC),action);
// QDOAS ???   ShowWindow(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_CONC_UNITS),action);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XsconvGeneralInit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         WM_INIT message processing
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndGeneral : the handle to the current tab page
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void XsconvGeneralInit(HWND hwndGeneral)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR string[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   INDEX indexItem;
// QDOAS ???
// QDOAS ???   // Resize tab page
// QDOAS ???
// QDOAS ???   XsconvTabReSize(GetParent(hwndGeneral),hwndGeneral);
// QDOAS ???
// QDOAS ???   // Fill comboboxes fields
// QDOAS ???
// QDOAS ???   for (indexItem=0;indexItem<CONVOLUTION_TYPE_MAX;indexItem++)
// QDOAS ???    SendMessage(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_SELECTION),CB_ADDSTRING,0,(LPARAM)xsconvConvolutionTypes[indexItem]);
// QDOAS ???   for (indexItem=0;indexItem<CONVOLUTION_CONVERSION_MAX;indexItem++)
// QDOAS ???    SendMessage(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_CONVERSION),CB_ADDSTRING,0,(LPARAM)xsconvConversionModes[indexItem]);
// QDOAS ???
// QDOAS ???   // Set convolution type
// QDOAS ???
// QDOAS ???   SendMessage(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_SELECTION),CB_SETCURSEL,(WPARAM)XSCONV_buffer.convolutionType,0);
// QDOAS ???   SendMessage(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_CONVERSION),CB_SETCURSEL,(WPARAM)XSCONV_buffer.conversionMode,0);
// QDOAS ???
// QDOAS ???   XsconvGeneralKurucz(hwndGeneral);
// QDOAS ???
// QDOAS ???   // Fill edit controls
// QDOAS ???
// QDOAS ???   SetWindowText(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_SHIFT),XSCONV_buffer.shift);
// QDOAS ???   SetWindowText(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_CROSS_FILE),XSCONV_buffer.crossFile);
// QDOAS ???   SetWindowText(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_PATH),XSCONV_buffer.path);
// QDOAS ???   SetWindowText(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_CALIBRATION_FILE),XSCONV_buffer.calibrationFile);
// QDOAS ???   SetWindowText(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_KURUCZ_FILE),XSCONV_buffer.kuruczFile);
// QDOAS ???
// QDOAS ???   sprintf(string,"%g",XSCONV_buffer.conc);
// QDOAS ???   SetWindowText(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_CONC),string);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XsconvGeneralCommand
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         WM_COMMAND message processing
// QDOAS ??? //
// QDOAS ??? // INPUT           usual syntax for windows messages routines
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? LRESULT CALLBACK XsconvGeneralCommand(HWND hwndGeneral,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch((ULONG)GET_WM_COMMAND_ID(mp1,mp2))
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_CONVOLUTION_SELECTION :
// QDOAS ???      XsconvGeneralKurucz(hwndGeneral);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_CONVOLUTION_CROSS_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndGeneral,XSCONV_buffer.crossFile,FILE_TYPE_CROSS,FILE_MODE_OPEN,TOOL_CONVOLUTION_CROSS_FILE,0);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_CONVOLUTION_PATH_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndGeneral,XSCONV_buffer.path,FILE_TYPE_ALL,FILE_MODE_SAVE,TOOL_CONVOLUTION_PATH,0);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_CONVOLUTION_CALIBRATION_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndGeneral,XSCONV_buffer.calibrationFile,FILE_TYPE_CALIB,FILE_MODE_OPEN,TOOL_CONVOLUTION_CALIBRATION_FILE,0);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_CONVOLUTION_KURUCZ_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndGeneral,XSCONV_buffer.kuruczFile,FILE_TYPE_CALIB_KURUCZ,FILE_MODE_OPEN,TOOL_CONVOLUTION_KURUCZ_FILE,0);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XsconvGeneralDestroy
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         WM_DESTROY message processing
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndGeneral : the handle to the current tab page
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void XsconvGeneralDestroy(HWND hwndGeneral)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   UCHAR string[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???
// QDOAS ???   // Get the conversion mode
// QDOAS ???
// QDOAS ???   XSCONV_buffer.conversionMode=SendMessage(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_CONVERSION),CB_GETCURSEL,0,0);
// QDOAS ???
// QDOAS ???   // Get the shift to apply to the wavelength calibration and the names of the input files
// QDOAS ???
// QDOAS ???   GetWindowText(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_SHIFT),XSCONV_buffer.shift,MAX_PATH_LEN);
// QDOAS ???   GetWindowText(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_CROSS_FILE),XSCONV_buffer.crossFile,MAX_PATH_LEN);
// QDOAS ???   GetWindowText(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_PATH),XSCONV_buffer.path,MAX_PATH_LEN);
// QDOAS ???   GetWindowText(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_CALIBRATION_FILE),XSCONV_buffer.calibrationFile,MAX_PATH_LEN);
// QDOAS ???   GetWindowText(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_KURUCZ_FILE),XSCONV_buffer.kuruczFile,MAX_PATH_LEN);
// QDOAS ???
// QDOAS ???   // Get the concentration (I0 correction)
// QDOAS ???
// QDOAS ???   memset(string,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   GetWindowText(GetDlgItem(hwndGeneral,TOOL_CONVOLUTION_CONC),string,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   XSCONV_buffer.conc=fabs(atof(string));
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XSCONV_GeneralWndProc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         dispatch messages from the page related to general options
// QDOAS ??? //
// QDOAS ??? // INPUT           usual syntax for windows messages routines
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? LRESULT CALLBACK XSCONV_GeneralWndProc(HWND hwndGeneral,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch (msg)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_INITDIALOG :
// QDOAS ???      XsconvGeneralInit(hwndGeneral);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_COMMAND :
// QDOAS ???      XsconvGeneralCommand(hwndGeneral,msg,mp1,mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_DESTROY :
// QDOAS ???      XsconvGeneralDestroy(hwndGeneral);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // SLIT FUNCTION TAB PAGE MESSAGES PROCESSING
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XSCONV_SlitType
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         According to the selected slit function, show/hide slit options
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndSlit : the handle to the current tab page
// QDOAS ??? //                 slitBase : the id of the "reference" field
// QDOAS ??? //
// QDOAS ??? // INPUT/OUTPUT    pSlit    : pointer to the current convolution options
// QDOAS ??? //                 pSlit2   : pointer to the current deconvolution options
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ???
// QDOAS ??? void XSCONV_SlitType(HWND hwndSlit,INT slitBase,SLIT *pSlit,SLIT *pSlit2)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INT action,fileAction,gaussAction,voigtAction,slitType,show_9_10;
// QDOAS ???   UCHAR string[MAX_ITEM_TEXT_LEN+1],slitFile[MAX_PATH_LEN+1];
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   slitType=(UCHAR)SendMessage(GetDlgItem(hwndSlit,slitBase),CB_GETCURSEL,0,0);
// QDOAS ???   voigtAction=((slitType!=SLIT_TYPE_VOIGT))?SW_HIDE:SW_SHOW;
// QDOAS ???   show_9_10=(slitType!=SLIT_TYPE_VOIGT)?SW_HIDE:SW_SHOW;
// QDOAS ???
// QDOAS ???   // Get the slit function file
// QDOAS ???
// QDOAS ???   GetWindowText(GetDlgItem(hwndSlit,slitBase+5 /* file */),slitFile,MAX_PATH_LEN+1);
// QDOAS ???
// QDOAS ???   // Modify the text of some fields according to the selected slit function file type
// QDOAS ???
// QDOAS ???   if (slitType==SLIT_TYPE_GAUSS)
// QDOAS ???    {
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+1),"Gaussian FWHM");
// QDOAS ???    }
// QDOAS ???   else if ((slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_ERF_FILE) || (slitType==SLIT_TYPE_ERF_T_FILE))
// QDOAS ???    {
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+1),"Gaussian FWHM");
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+6),"Boxcar width");
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+8),"nm");
// QDOAS ???    }
// QDOAS ???   else if ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_INVPOLY_FILE))
// QDOAS ???    {
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+1),"Width");
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+6),"Degree");
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+8),"(multiple of 2)");
// QDOAS ???    }
// QDOAS ???   else if (slitType==SLIT_TYPE_VOIGT)
// QDOAS ???    {
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+1),"Gaussian FWHM (L)");
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+6),"Lorentz/Gauss (L)");
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+9),"Gaussian FWHM (R)");
// QDOAS ???    }
// QDOAS ???   else if ((slitType==SLIT_TYPE_APOD) || (slitType==SLIT_TYPE_APODNBS))
// QDOAS ???    {
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+1),"Resolution");
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+6),"Phase");
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Keep a trace of the previous options
// QDOAS ???
// QDOAS ???   if ((slitType!=SLIT_TYPE_GAUSS) &&
// QDOAS ???       (slitType!=SLIT_TYPE_INVPOLY) &&
// QDOAS ???       (slitType!=SLIT_TYPE_ERF) &&
// QDOAS ???       (slitType!=SLIT_TYPE_VOIGT) &&
// QDOAS ???       (slitType!=SLIT_TYPE_APOD) &&
// QDOAS ???       (slitType!=SLIT_TYPE_APODNBS))
// QDOAS ???    {
// QDOAS ???     GetWindowText(GetDlgItem(hwndSlit,slitBase+2 /* gauss width */),string,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???     pSlit->slitParam=fabs(atof(string));
// QDOAS ???     GetWindowText(GetDlgItem(hwndSlit,slitBase+7 /* inv polynomial degree */),string,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???     pSlit->slitParam2=fabs(atof(string));
// QDOAS ???     GetWindowText(GetDlgItem(hwndSlit,slitBase+10 /* Voigt Gauss FWHM */),string,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???     pSlit->slitParam3=fabs(atof(string));
// QDOAS ???     GetWindowText(GetDlgItem(hwndSlit,slitBase+12 /* Voigt Lorentz/Gauss ratio */),string,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???     pSlit->slitParam4=fabs(atof(string));
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+5 /* file */),slitFile);
// QDOAS ???
// QDOAS ???     fileAction=SW_SHOW;
// QDOAS ???     gaussAction=SW_HIDE;
// QDOAS ???    }
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     GetWindowText(GetDlgItem(hwndSlit,slitBase+5 /* file */),slitFile,MAX_PATH_LEN+1);
// QDOAS ???     sprintf(string,"%.3lf",pSlit->slitParam);
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+2 /* gauss width */),string);
// QDOAS ???     sprintf(string,"%.3lf",pSlit->slitParam2);
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+7 /* inv polynomial degree */),string);
// QDOAS ???     sprintf(string,"%.3lf",pSlit->slitParam3);
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+10 /* Voigt Gauss FWHM */),string);
// QDOAS ???     sprintf(string,"%.3lf",pSlit->slitParam4);
// QDOAS ???     SetWindowText(GetDlgItem(hwndSlit,slitBase+12 /* Voigt Lorentz/Gauss ratio */),string);
// QDOAS ???
// QDOAS ???     fileAction=SW_HIDE;
// QDOAS ???     gaussAction=SW_SHOW;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   pSlit->slitType=slitType;
// QDOAS ???
// QDOAS ???   // Show/hide file fields
// QDOAS ???
// QDOAS ???   ShowWindow(GetDlgItem(hwndSlit,slitBase+4 /* file button */),fileAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndSlit,slitBase+5 /* file control edit */),fileAction);
// QDOAS ???
// QDOAS ???   // Show/hide gauss fields
// QDOAS ???
// QDOAS ???   ShowWindow(GetDlgItem(hwndSlit,slitBase+1 /* text */),gaussAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndSlit,slitBase+2 /* control edit */),gaussAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndSlit,slitBase+3 /* nm */),gaussAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndSlit,slitBase+6 /* n */),(gaussAction && ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_VOIGT) || (slitType==SLIT_TYPE_APOD) || (slitType==SLIT_TYPE_APODNBS) || (slitType==SLIT_TYPE_ERF))) || (slitType==SLIT_TYPE_INVPOLY_FILE) || (slitType==SLIT_TYPE_ERF_FILE) || (slitType==SLIT_TYPE_ERF_T_FILE));
// QDOAS ???   ShowWindow(GetDlgItem(hwndSlit,slitBase+7 /* control edit */),(gaussAction && ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_VOIGT) || (slitType==SLIT_TYPE_APOD) || (slitType==SLIT_TYPE_APODNBS) || (slitType==SLIT_TYPE_ERF))) || ((slitType==SLIT_TYPE_INVPOLY_FILE) || (slitType==SLIT_TYPE_ERF_FILE) || (slitType==SLIT_TYPE_ERF_T_FILE)));
// QDOAS ???   ShowWindow(GetDlgItem(hwndSlit,slitBase+8 /* text */),((gaussAction==SW_SHOW) && (voigtAction==SW_HIDE) && ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_VOIGT) || (slitType==SLIT_TYPE_ERF))) || ((slitType==SLIT_TYPE_INVPOLY_FILE) || (slitType==SLIT_TYPE_ERF_FILE) || (slitType==SLIT_TYPE_ERF_T_FILE)));
// QDOAS ???
// QDOAS ???   // Show/hide Voigt fields
// QDOAS ???
// QDOAS ???   ShowWindow(GetDlgItem(hwndSlit,slitBase+9),show_9_10);
// QDOAS ???   ShowWindow(GetDlgItem(hwndSlit,slitBase+10),show_9_10);
// QDOAS ???   ShowWindow(GetDlgItem(hwndSlit,slitBase+11),voigtAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndSlit,slitBase+12),voigtAction);
// QDOAS ???
// QDOAS ???   // Hide deconvolution fields if the convolution slit function type is wavelength dependent
// QDOAS ???
// QDOAS ???   if ((slitBase==TOOL_CONVOLUTION_TYPE) || (slitBase==TOOL_DECONVOLUTION_TYPE))
// QDOAS ???    {
// QDOAS ???    	action=((XSCONV_buffer.slitConv.slitType==SLIT_TYPE_GAUSS_FILE) ||
// QDOAS ???         	   (XSCONV_buffer.slitConv.slitType==SLIT_TYPE_INVPOLY_FILE) ||
// QDOAS ???    	        (XSCONV_buffer.slitConv.slitType==SLIT_TYPE_ERF_FILE))?SW_HIDE:SW_SHOW;
// QDOAS ???
// QDOAS ???     ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_GROUP),action);
// QDOAS ???     ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_TYPE),action);
// QDOAS ???     ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_SLIT_TYPE),action);
// QDOAS ???
// QDOAS ???     if (action==SW_HIDE)
// QDOAS ???      {
// QDOAS ???       ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_GAUSS_WIDTH_TEXT),SW_HIDE);
// QDOAS ???       ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_GAUSS_WIDTH),SW_HIDE);
// QDOAS ???       ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_GAUSS_WIDTH_NM),SW_HIDE);
// QDOAS ???
// QDOAS ???       ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_INVPOLY_TEXT),SW_HIDE);
// QDOAS ???       ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_INVPOLY),SW_HIDE);
// QDOAS ???       ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_INVPOLY_2),SW_HIDE);
// QDOAS ???
// QDOAS ???       ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_SLIT_BUTTON),SW_HIDE);
// QDOAS ???       ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_SLIT_FILE),SW_HIDE);
// QDOAS ???
// QDOAS ???       ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_SLIT_FILE),SW_HIDE);
// QDOAS ???       ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_VOIGT_RGAUSS_TEXT),SW_HIDE);
// QDOAS ???       ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_VOIGT_RGAUSS),SW_HIDE);
// QDOAS ???       ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_VOIGT_RRATIO_TEXT),SW_HIDE);
// QDOAS ???       ShowWindow(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_VOIGT_RRATIO),SW_HIDE);
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XsconvSlitInit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         WM_INIT message processing
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndSlit : the handle to the current tab page
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void XsconvSlitInit(HWND hwndSlit)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR string[MAX_ITEM_TEXT_LEN+1];                                            // string for float fields
// QDOAS ???   HWND  hwndConv,hwndDConv;                                                     // handles to convolution and deconvolution fields
// QDOAS ???   SLIT *pSlitConv,*pSlitDConv;                                                  // resp. convolution and deconvolution options
// QDOAS ???   INDEX indexItem;                                                              // index to browse items to fill comboboxes
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pSlitConv=&XSCONV_buffer.slitConv;
// QDOAS ???   pSlitDConv=&XSCONV_buffer.slitDConv;
// QDOAS ???
// QDOAS ???   hwndConv=GetDlgItem(hwndSlit,TOOL_CONVOLUTION_TYPE);
// QDOAS ???   hwndDConv=GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_TYPE);
// QDOAS ???
// QDOAS ???   // Resize the tab page
// QDOAS ???
// QDOAS ???   XsconvTabReSize(GetParent(hwndSlit),hwndSlit);
// QDOAS ???
// QDOAS ???   // Set the slit function type
// QDOAS ???
// QDOAS ???   for (indexItem=0;indexItem<SLIT_TYPE_GAUSS_T_FILE;indexItem++)
// QDOAS ???    {
// QDOAS ???     SendMessage(hwndConv,CB_ADDSTRING,0,(LPARAM)XSCONV_slitTypes[indexItem]);
// QDOAS ???     if (indexItem<SLIT_TYPE_GAUSS_FILE)
// QDOAS ???      SendMessage(hwndDConv,CB_ADDSTRING,0,(LPARAM)XSCONV_slitTypes[indexItem]);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   SendMessage(hwndConv,CB_SETCURSEL,(WPARAM)pSlitConv->slitType,0);
// QDOAS ???   SendMessage(hwndDConv,CB_SETCURSEL,(WPARAM)pSlitDConv->slitType,0);
// QDOAS ???
// QDOAS ???   // Fill edit controls
// QDOAS ???
// QDOAS ???   SetWindowText(GetDlgItem(hwndSlit,TOOL_CONVOLUTION_SLIT_FILE),pSlitConv->slitFile);
// QDOAS ???   sprintf(string,"%g",pSlitConv->slitParam);
// QDOAS ???   SetWindowText(GetDlgItem(hwndSlit,TOOL_CONVOLUTION_GAUSS_WIDTH),string);
// QDOAS ???   sprintf(string,"%g",pSlitConv->slitParam2);
// QDOAS ???   SetWindowText(GetDlgItem(hwndSlit,TOOL_CONVOLUTION_INVPOLY),string);
// QDOAS ???   sprintf(string,"%g",pSlitConv->slitParam3);
// QDOAS ???   SetWindowText(GetDlgItem(hwndSlit,TOOL_CONVOLUTION_VOIGT_RGAUSS),string);
// QDOAS ???   sprintf(string,"%g",pSlitConv->slitParam4);
// QDOAS ???   SetWindowText(GetDlgItem(hwndSlit,TOOL_CONVOLUTION_VOIGT_RRATIO),string);
// QDOAS ???   SetWindowText(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_SLIT_FILE),pSlitDConv->slitFile);
// QDOAS ???   sprintf(string,"%g",pSlitDConv->slitParam);
// QDOAS ???   SetWindowText(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_GAUSS_WIDTH),string);
// QDOAS ???   sprintf(string,"%g",pSlitDConv->slitParam2);
// QDOAS ???   SetWindowText(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_INVPOLY),string);
// QDOAS ???   sprintf(string,"%g",pSlitDConv->slitParam3);
// QDOAS ???   SetWindowText(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_VOIGT_RGAUSS),string);
// QDOAS ???   sprintf(string,"%g",pSlitDConv->slitParam4);
// QDOAS ???   SetWindowText(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_VOIGT_RRATIO),string);
// QDOAS ???
// QDOAS ???   // Show/Hide File/Gauss fields
// QDOAS ???
// QDOAS ???   XSCONV_SlitType(hwndSlit,TOOL_CONVOLUTION_TYPE,pSlitConv,pSlitDConv);
// QDOAS ???   XSCONV_SlitType(hwndSlit,TOOL_DECONVOLUTION_TYPE,pSlitDConv,pSlitConv);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XsconvSlitCommand
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         WM_COMMAND message processing
// QDOAS ??? //
// QDOAS ??? // INPUT           usual syntax for windows messages routines
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? LRESULT CALLBACK XsconvSlitCommand(HWND hwndSlit,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch((ULONG)GET_WM_COMMAND_ID(mp1,mp2))
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_CONVOLUTION_SLIT_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndSlit,XSCONV_buffer.slitConv.slitFile,FILE_TYPE_SLIT,FILE_MODE_OPEN,TOOL_CONVOLUTION_SLIT_FILE,0);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_DECONVOLUTION_SLIT_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndSlit,XSCONV_buffer.slitDConv.slitFile,FILE_TYPE_SLIT,FILE_MODE_OPEN,TOOL_DECONVOLUTION_SLIT_FILE,0);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_CONVOLUTION_TYPE :
// QDOAS ???      XSCONV_SlitType(hwndSlit,TOOL_CONVOLUTION_TYPE,&XSCONV_buffer.slitConv,&XSCONV_buffer.slitDConv);
// QDOAS ???      XSCONV_SlitType(hwndSlit,TOOL_DECONVOLUTION_TYPE,&XSCONV_buffer.slitDConv,&XSCONV_buffer.slitConv);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_DECONVOLUTION_TYPE :
// QDOAS ???      XSCONV_SlitType(hwndSlit,TOOL_CONVOLUTION_TYPE,&XSCONV_buffer.slitConv,&XSCONV_buffer.slitDConv);
// QDOAS ???      XSCONV_SlitType(hwndSlit,TOOL_DECONVOLUTION_TYPE,&XSCONV_buffer.slitDConv,&XSCONV_buffer.slitConv);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XsconvSlitDestroy
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         WM_DESTROY message processing
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndSlit : the handle to the current tab page
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void XsconvSlitDestroy(HWND hwndSlit)
// QDOAS ???  {
// QDOAS ???  	// Declaration
// QDOAS ???
// QDOAS ???   UCHAR string[MAX_ITEM_TEXT_LEN+1];                                            // string for float fields
// QDOAS ???
// QDOAS ???   // Get the type of slit function
// QDOAS ???
// QDOAS ???   XSCONV_buffer.slitConv.slitType=(UCHAR)SendMessage(GetDlgItem(hwndSlit,TOOL_CONVOLUTION_TYPE),CB_GETCURSEL,0,0);
// QDOAS ???   XSCONV_buffer.slitDConv.slitType=(UCHAR)SendMessage(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_TYPE),CB_GETCURSEL,0,0);
// QDOAS ???
// QDOAS ???   // Convolution options
// QDOAS ???
// QDOAS ???   memset(string,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   GetWindowText(GetDlgItem(hwndSlit,TOOL_CONVOLUTION_GAUSS_WIDTH),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   XSCONV_buffer.slitConv.slitParam=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndSlit,TOOL_CONVOLUTION_INVPOLY),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   XSCONV_buffer.slitConv.slitParam2=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndSlit,TOOL_CONVOLUTION_VOIGT_RGAUSS),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   XSCONV_buffer.slitConv.slitParam3=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndSlit,TOOL_CONVOLUTION_VOIGT_RRATIO),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   XSCONV_buffer.slitConv.slitParam4=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndSlit,TOOL_CONVOLUTION_SLIT_FILE),XSCONV_buffer.slitConv.slitFile,MAX_PATH_LEN);
// QDOAS ???
// QDOAS ???   // Deconvolution options
// QDOAS ???
// QDOAS ???   memset(string,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   GetWindowText(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_GAUSS_WIDTH),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   XSCONV_buffer.slitDConv.slitParam=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_INVPOLY),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   XSCONV_buffer.slitDConv.slitParam2=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_VOIGT_RGAUSS),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   XSCONV_buffer.slitDConv.slitParam3=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_VOIGT_RRATIO),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   XSCONV_buffer.slitDConv.slitParam4=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndSlit,TOOL_DECONVOLUTION_SLIT_FILE),XSCONV_buffer.slitDConv.slitFile,MAX_PATH_LEN);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XSCONV_SlitWndProc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         dispatch messages from the page related to the slit function
// QDOAS ??? //
// QDOAS ??? // INPUT           usual syntax for windows messages routines
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? LRESULT CALLBACK XSCONV_SlitWndProc(HWND hwndSlit,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch (msg)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_INITDIALOG :
// QDOAS ???      XsconvSlitInit(hwndSlit);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_COMMAND :
// QDOAS ???      XsconvSlitCommand(hwndSlit,msg,mp1,mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_DESTROY :
// QDOAS ???      XsconvSlitDestroy(hwndSlit);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // CONVOLUTION DIALOG BOX MESSAGES PROCESSING
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XsconvInit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         initialization of the convolution tools dialog box (WM_INITDIALOG)
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndXsconv : the handle to the convolution tools dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void XsconvInit(HWND hwndXsconv)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   TAB_PAGE *pTab;                                                               // pointer to the current tab control
// QDOAS ???   TC_ITEM tie;                                                                  // attributes of pages in tab control
// QDOAS ???   HRSRC hrsrc;                                                                  // handle of resource
// QDOAS ???   HGLOBAL hglb;                                                                 // global handle
// QDOAS ???   INDEX indexTab;                                                               // tab page number (base 0)
// QDOAS ???   HWND hwndTab;                                                                 // handle to the current tab page
// QDOAS ???
// QDOAS ???   // Center dialog box on parent window
// QDOAS ???
// QDOAS ???   DOAS_CenterWindow(hwndXsconv,GetWindow(hwndXsconv,GW_OWNER));
// QDOAS ???   memcpy(&XSCONV_buffer,&XSCONV_options,sizeof(XSCONV));
// QDOAS ???
// QDOAS ???   // Check box
// QDOAS ???
// QDOAS ???   CheckDlgButton(hwndXsconv,TOOL_CONVOLUTION_NOCOMMENT,(XSCONV_buffer.noComment!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   hwndTab=GetDlgItem(hwndXsconv,TOOL_CONVOLUTION_TAB);
// QDOAS ???
// QDOAS ???   tie.mask=TCIF_TEXT|TCIF_IMAGE;                                                // specify wich members are valid
// QDOAS ???   tie.iImage=-1;                                                                // no icon is associated to tabs
// QDOAS ???
// QDOAS ???   // Browse tab control pages
// QDOAS ???
// QDOAS ???   for (indexTab=0;indexTab<TAB_TYPE_XSCONV_MAX;indexTab++)
// QDOAS ???    {
// QDOAS ???     pTab=&xsconvTabPages[indexTab];
// QDOAS ???
// QDOAS ???     // Insert tab pages
// QDOAS ???
// QDOAS ???     tie.pszText=pTab->tabTitle;
// QDOAS ???     TabCtrl_InsertItem(hwndTab,indexTab,&tie);
// QDOAS ???
// QDOAS ???     // Lock resources
// QDOAS ???
// QDOAS ???     hrsrc=FindResource(NULL,(LPCTSTR)MAKEINTRESOURCE(pTab->dlgBox),(LPCTSTR)RT_DIALOG);
// QDOAS ???     hglb=LoadResource(DOAS_hInst,hrsrc);
// QDOAS ???     pTab->dlgTemp=(DLGTEMPLATE *)LockResource(hglb);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Rebuild files names
// QDOAS ???
// QDOAS ???   FILES_RebuildFileName(XSCONV_buffer.calibrationFile,XSCONV_options.calibrationFile,1);
// QDOAS ???   FILES_RebuildFileName(XSCONV_buffer.crossFile,XSCONV_options.crossFile,1);
// QDOAS ???   FILES_RebuildFileName(XSCONV_buffer.path,XSCONV_options.path,1);
// QDOAS ???   FILES_RebuildFileName(XSCONV_buffer.kuruczFile,XSCONV_options.kuruczFile,1);
// QDOAS ???   FILES_RebuildFileName(XSCONV_buffer.slitConv.slitFile,XSCONV_options.slitConv.slitFile,1);
// QDOAS ???   FILES_RebuildFileName(XSCONV_buffer.slitDConv.slitFile,XSCONV_options.slitDConv.slitFile,1);
// QDOAS ???
// QDOAS ???   // Set first tab page
// QDOAS ???
// QDOAS ???   XsconvTabChange(hwndTab);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XsconvNotify
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         WM_NOTIFY message processing
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndXsconv : the handle to the convolution tools dialog box
// QDOAS ??? //                 pHdr       : pointer to the object that sends the notification
// QDOAS ??? //                              message
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void XsconvNotify(HWND hwndXsconv,NMHDR *pHdr)
// QDOAS ???  {
// QDOAS ???   // Notification message processing
// QDOAS ???
// QDOAS ???   switch (pHdr->code)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TCN_SELCHANGE :         // the currently selected tab page has changed
// QDOAS ???      XsconvTabChange(GetDlgItem(hwndXsconv,TOOL_CONVOLUTION_TAB));
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XsconvOK
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Validate the convolution options on OK command
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndXsconv : the handle to the convolution tools dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void XsconvOK(HWND hwndXsconv)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR controlName[MAX_ITEM_NAME_LEN+1];                                       // the name of a control
// QDOAS ???   SLIT *pSlitConv,*pSlitDConv;                                                  // pointers to the convolution and deconvolution slit function options
// QDOAS ???   PRJCT_FILTER *plFilter,*phFilter;                                             // pointers to the low-pass and high-pass filtering options
// QDOAS ???   HWND hwndTab;                                                                 // handle to the tab control with the different pages of options
// QDOAS ???   INDEX indexTab;                                                               // index to the tab page to select in case of bad entry
// QDOAS ???   RC rc,                                                                        // error code
// QDOAS ???      idControl;                                                                 // id number of the control to point in case of bad entry
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   hwndTab=GetDlgItem(hwndXsconv,TOOL_CONVOLUTION_TAB);
// QDOAS ???   pSlitConv=&XSCONV_buffer.slitConv;
// QDOAS ???   pSlitDConv=&XSCONV_buffer.slitDConv;
// QDOAS ???   plFilter=&XSCONV_buffer.lfilter;
// QDOAS ???   phFilter=&XSCONV_buffer.hfilter;
// QDOAS ???   indexTab=ITEM_NONE;
// QDOAS ???   idControl=ITEM_NONE;
// QDOAS ???   memset(controlName,0,MAX_ITEM_NAME_LEN+1);
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Get the current page
// QDOAS ???
// QDOAS ???   XsconvTabChange(hwndTab);
// QDOAS ???
// QDOAS ???   // General options
// QDOAS ???
// QDOAS ???   indexTab=TAB_TYPE_XSCONV_GENERAL;
// QDOAS ???
// QDOAS ???   if (!strlen(XSCONV_buffer.crossFile))
// QDOAS ???    {
// QDOAS ???    	idControl=TOOL_CONVOLUTION_CROSS_FILE;
// QDOAS ???    	strcpy(controlName,"Input file");
// QDOAS ???    	rc=IDS_MSGBOX_FIELDEMPTY;
// QDOAS ???    }
// QDOAS ???   else if (!strlen(XSCONV_buffer.calibrationFile))
// QDOAS ???    {
// QDOAS ???    	idControl=TOOL_CONVOLUTION_CALIBRATION_FILE;
// QDOAS ???    	strcpy(controlName,"Calibration");
// QDOAS ???    	rc=IDS_MSGBOX_FIELDEMPTY;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Convolution page
// QDOAS ???
// QDOAS ???   if (!rc && (XSCONV_buffer.convolutionType!=CONVOLUTION_TYPE_NONE))
// QDOAS ???    {
// QDOAS ???    	indexTab=TAB_TYPE_XSCONV_SLIT;
// QDOAS ???
// QDOAS ??? /* !!!
// QDOAS ???
// QDOAS ???    	if ((pSlitConv->slitType==SLIT_TYPE_NONE) && (pSlitDConv->slitType==SLIT_TYPE_NONE))
// QDOAS ???      {
// QDOAS ???       idControl=TOOL_CONVOLUTION_TYPE;
// QDOAS ???       strcpy(controlName,"Slit Function type");
// QDOAS ???       rc=IDS_MSGBOX_FIELDEMPTY;
// QDOAS ???      }    */
// QDOAS ???
// QDOAS ???    	// Convolution
// QDOAS ???
// QDOAS ???     if (// !!! (pSlitConv->slitType!=SLIT_TYPE_NONE) &&
// QDOAS ???         (pSlitConv->slitType!=SLIT_TYPE_GAUSS) &&
// QDOAS ???         (pSlitConv->slitType!=SLIT_TYPE_INVPOLY) &&
// QDOAS ???         (pSlitConv->slitType!=SLIT_TYPE_ERF) &&
// QDOAS ???         (pSlitConv->slitType!=SLIT_TYPE_VOIGT) &&
// QDOAS ???         (pSlitConv->slitType!=SLIT_TYPE_APOD) &&
// QDOAS ???         (pSlitConv->slitType!=SLIT_TYPE_APODNBS) && !strlen(pSlitConv->slitFile))
// QDOAS ???      {
// QDOAS ???       idControl=TOOL_CONVOLUTION_SLIT_FILE;
// QDOAS ???       strcpy(controlName,"Convolution slit function file");
// QDOAS ???       rc=IDS_MSGBOX_FIELDEMPTY;
// QDOAS ???      }
// QDOAS ???     else if (((pSlitConv->slitType==SLIT_TYPE_INVPOLY) || (pSlitConv->slitType==SLIT_TYPE_INVPOLY_FILE)) &&
// QDOAS ???              ((pSlitConv->slitParam2<=(double)0.) ||
// QDOAS ???               (pSlitConv->slitParam2-floor(pSlitConv->slitParam2)!=(double)0.) ||
// QDOAS ???               (fmod(pSlitConv->slitParam2,(double)2.)!=(double)0.)))
// QDOAS ???      {
// QDOAS ???       idControl=TOOL_CONVOLUTION_INVPOLY;
// QDOAS ???       strcpy(controlName,"convolution function");
// QDOAS ???       rc=IDS_MSGBOX_DEGREE_2;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Deconvolution
// QDOAS ???
// QDOAS ???     if ((pSlitDConv->slitType!=SLIT_TYPE_FILE) &&
// QDOAS ???         (pSlitDConv->slitType!=SLIT_TYPE_GAUSS) &&
// QDOAS ???         (pSlitDConv->slitType!=SLIT_TYPE_INVPOLY) &&
// QDOAS ???         (pSlitDConv->slitType!=SLIT_TYPE_ERF) &&
// QDOAS ???         (pSlitDConv->slitType!=SLIT_TYPE_VOIGT) &&
// QDOAS ???         (pSlitDConv->slitType!=SLIT_TYPE_APOD) &&
// QDOAS ???         (pSlitDConv->slitType!=SLIT_TYPE_APODNBS) && !strlen(pSlitConv->slitFile))
// QDOAS ???      {
// QDOAS ???       idControl=TOOL_DECONVOLUTION_SLIT_FILE;
// QDOAS ???       strcpy(controlName,"Deconvolution slit function file");
// QDOAS ???       rc=IDS_MSGBOX_FIELDEMPTY;
// QDOAS ???      }
// QDOAS ???     else if (((pSlitDConv->slitType==SLIT_TYPE_INVPOLY) || (pSlitDConv->slitType==SLIT_TYPE_INVPOLY_FILE)) &&
// QDOAS ???              ((pSlitDConv->slitParam2<=(double)0.) ||
// QDOAS ???               (pSlitDConv->slitParam2-floor(pSlitDConv->slitParam2)!=(double)0.) ||
// QDOAS ???               (fmod(pSlitDConv->slitParam2,(double)2.)!=(double)0.)))
// QDOAS ???      {
// QDOAS ???       idControl=TOOL_DECONVOLUTION_INVPOLY;
// QDOAS ???       strcpy(controlName,"deconvolution function");
// QDOAS ???       rc=IDS_MSGBOX_DEGREE_2;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Low-pass filtering options
// QDOAS ???
// QDOAS ???   if (!rc && (plFilter->type!=PRJCT_FILTER_TYPE_NONE))
// QDOAS ???    {
// QDOAS ???    	indexTab=TAB_TYPE_XSCONV_FILTER;
// QDOAS ???
// QDOAS ???     if (((plFilter->type==PRJCT_FILTER_TYPE_BOXCAR) ||
// QDOAS ???          (plFilter->type==PRJCT_FILTER_TYPE_TRIANGLE) ||
// QDOAS ???          (plFilter->type==PRJCT_FILTER_TYPE_SG) ||
// QDOAS ???          (plFilter->type==PRJCT_FILTER_TYPE_BINOMIAL)) &&
// QDOAS ???          (plFilter->filterWidth%2!=1))
// QDOAS ???      {
// QDOAS ???       idControl=plFilter->type+PRJCT_FILTER_FIELD_FWHM;
// QDOAS ???       strcpy(controlName,"Filter width");
// QDOAS ???       rc=IDS_MSGBOX_FILTER_WIDTH;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     else if ((plFilter->type==PRJCT_FILTER_TYPE_SG) &&
// QDOAS ???             ((plFilter->filterOrder%2!=0) || (plFilter->filterOrder==0)))
// QDOAS ???      {
// QDOAS ???       idControl=plFilter->type+PRJCT_FILTER_FIELD_ORDER;
// QDOAS ???       strcpy(controlName,"Filter order");
// QDOAS ???       rc=IDS_MSGBOX_FILTER_ORDER;
// QDOAS ???      }
// QDOAS ???     else if ((plFilter->type==PRJCT_FILTER_TYPE_GAUSSIAN) && (plFilter->fwhmWidth<=(double)0.))
// QDOAS ???      {
// QDOAS ???       idControl=plFilter->type+PRJCT_FILTER_FIELD_FWHM;
// QDOAS ???       strcpy(controlName,"Filter width");
// QDOAS ???       rc=IDS_MSGBOX_POSITIVE;
// QDOAS ???      }
// QDOAS ???     else if (plFilter->filterNTimes<=0)
// QDOAS ???      {
// QDOAS ???       idControl=plFilter->type+PRJCT_FILTER_FIELD_NITER;
// QDOAS ???       strcpy(controlName,"Number of iterations");
// QDOAS ???       rc=IDS_MSGBOX_POSITIVE;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // High-pass filtering options
// QDOAS ???
// QDOAS ???   if (!rc && (phFilter->type!=PRJCT_FILTER_TYPE_NONE))
// QDOAS ???    {
// QDOAS ???    	indexTab=TAB_TYPE_XSCONV_FILTER;
// QDOAS ???
// QDOAS ???     if (((phFilter->type==PRJCT_FILTER_TYPE_BOXCAR) ||
// QDOAS ???          (phFilter->type==PRJCT_FILTER_TYPE_TRIANGLE) ||
// QDOAS ???          (phFilter->type==PRJCT_FILTER_TYPE_SG) ||
// QDOAS ???          (phFilter->type==PRJCT_FILTER_TYPE_BINOMIAL)) &&
// QDOAS ???          (phFilter->filterWidth%2!=1))
// QDOAS ???      {
// QDOAS ???       idControl=phFilter->type+PRJCT_FILTER_FIELD_FWHM;
// QDOAS ???       strcpy(controlName,"Filter width");
// QDOAS ???       rc=IDS_MSGBOX_FILTER_WIDTH;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     else if ((phFilter->type==PRJCT_FILTER_TYPE_SG) &&
// QDOAS ???             ((phFilter->filterOrder%2!=0) || (phFilter->filterOrder==0)))
// QDOAS ???      {
// QDOAS ???       idControl=phFilter->type+PRJCT_FILTER_FIELD_ORDER;
// QDOAS ???       strcpy(controlName,"Filter order");
// QDOAS ???       rc=IDS_MSGBOX_FILTER_ORDER;
// QDOAS ???      }
// QDOAS ???     else if ((phFilter->type==PRJCT_FILTER_TYPE_GAUSSIAN) && (phFilter->fwhmWidth<=(double)0.))
// QDOAS ???      {
// QDOAS ???       idControl=phFilter->type+PRJCT_FILTER_FIELD_FWHM;
// QDOAS ???       strcpy(controlName,"Filter width");
// QDOAS ???       rc=IDS_MSGBOX_POSITIVE;
// QDOAS ???      }
// QDOAS ???     else if (phFilter->filterNTimes<=0)
// QDOAS ???      {
// QDOAS ???       idControl=phFilter->type+PRJCT_FILTER_FIELD_NITER;
// QDOAS ???       strcpy(controlName,"Number of iterations");
// QDOAS ???       rc=IDS_MSGBOX_POSITIVE;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Send error message
// QDOAS ???
// QDOAS ???   if (rc)
// QDOAS ???    {
// QDOAS ???     TabCtrl_SetCurSel(hwndTab,indexTab);
// QDOAS ???     XsconvTabChange(hwndTab);
// QDOAS ???     MSG_MessageBox(hwndXsconv,idControl,MENU_WINDOWS_TOOL_CONVOLUTION,rc,MB_OK|MB_ICONHAND,controlName);
// QDOAS ???    }
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     XSCONV_buffer.noComment=(UCHAR)(IsDlgButtonChecked(hwndXsconv,TOOL_CONVOLUTION_NOCOMMENT)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???
// QDOAS ???     memcpy(&XSCONV_options,&XSCONV_buffer,sizeof(XSCONV));
// QDOAS ???
// QDOAS ???     FILES_ChangePath(XSCONV_options.calibrationFile,XSCONV_buffer.calibrationFile,1);
// QDOAS ???     FILES_ChangePath(XSCONV_options.crossFile,XSCONV_buffer.crossFile,1);
// QDOAS ???     FILES_ChangePath(XSCONV_options.path,XSCONV_buffer.path,1);
// QDOAS ???     FILES_ChangePath(XSCONV_options.kuruczFile,XSCONV_buffer.kuruczFile,1);
// QDOAS ???     FILES_ChangePath(XSCONV_options.slitConv.slitFile,XSCONV_buffer.slitConv.slitFile,1);
// QDOAS ???     FILES_ChangePath(XSCONV_options.slitDConv.slitFile,XSCONV_buffer.slitDConv.slitFile,1);
// QDOAS ???
// QDOAS ???     XSCONV_Convolution(hwndXsconv);
// QDOAS ???
// QDOAS ???     if (!TLBAR_bSaveFlag)
// QDOAS ???      TLBAR_Enable(TRUE);
// QDOAS ???
// QDOAS ???     EndDialog(hwndXsconv,TRUE);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XsConvHelp
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         dispatch help messages from the convolution tools dialog box
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndXsconv : handle to the convolution tools dialog box
// QDOAS ??? //                 ctrlId     : id of the field help is needed for
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void XsConvHelp(HWND hwndXsconv,INT ctrlId)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INT helpId;
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   helpId=ITEM_NONE;
// QDOAS ???
// QDOAS ???   // Dispatches help messages
// QDOAS ???
// QDOAS ???   if (xsconvIndexSelected==TAB_TYPE_XSCONV_FILTER)
// QDOAS ???    helpId=HLP_TOOLS_CONV_TAB3_DATA;
// QDOAS ???   else if (ctrlId==ITEM_NONE)
// QDOAS ???    helpId=(xsconvIndexSelected==TAB_TYPE_XSCONV_GENERAL)?HLP_TOOLS_CONV_TAB1_TYPE:HLP_TOOLS_CONV_TAB2_DATA;
// QDOAS ???   else
// QDOAS ???
// QDOAS ???    switch(ctrlId)
// QDOAS ???     {
// QDOAS ???   // ----------------------------------------------------------------------------
// QDOAS ???      case TOOL_CONVOLUTION_SELECTION :
// QDOAS ???       helpId=HLP_TOOLS_CONV_TAB1_TYPE;
// QDOAS ???      break;
// QDOAS ???   // ----------------------------------------------------------------------------
// QDOAS ???      case TOOL_CONVOLUTION_CONVERSION :
// QDOAS ???       helpId=HLP_TOOLS_CONV_TAB1_CONVERSION;
// QDOAS ???      break;
// QDOAS ???   // ----------------------------------------------------------------------------
// QDOAS ???      case TOOL_CONVOLUTION_SHIFT_GROUP :
// QDOAS ???      case TOOL_CONVOLUTION_SHIFT :
// QDOAS ???       helpId=HLP_TOOLS_CONV_TAB1_SHIFT;
// QDOAS ???      break;
// QDOAS ???   // ----------------------------------------------------------------------------
// QDOAS ???      case TOOL_CONVOLUTION_CROSS_BUTTON :
// QDOAS ???      case TOOL_CONVOLUTION_CROSS_FILE :
// QDOAS ???      case TOOL_CONVOLUTION_PATH_BUTTON :
// QDOAS ???      case TOOL_CONVOLUTION_PATH :
// QDOAS ???      case TOOL_CONVOLUTION_CALIBRATION_BUTTON :
// QDOAS ???      case TOOL_CONVOLUTION_CALIBRATION_FILE :
// QDOAS ???       helpId=HLP_TOOLS_CONV_TAB1_FILES;
// QDOAS ???      break;
// QDOAS ???   // ----------------------------------------------------------------------------
// QDOAS ???      case TOOL_CONVOLUTION_CONC_TEXT :
// QDOAS ???      case TOOL_CONVOLUTION_CONC :
// QDOAS ???      case TOOL_CONVOLUTION_CONC_UNITS :
// QDOAS ???      case TOOL_CONVOLUTION_KURUCZ_BUTTON :
// QDOAS ???      case TOOL_CONVOLUTION_KURUCZ_FILE :
// QDOAS ???       helpId=HLP_TOOLS_CONV_TAB1_I0;
// QDOAS ???      break;
// QDOAS ???   // ----------------------------------------------------------------------------
// QDOAS ???      case TOOL_CONVOLUTION_TYPE :
// QDOAS ???      case TOOL_CONVOLUTION_GAUSS_WIDTH_TEXT :
// QDOAS ???      case TOOL_CONVOLUTION_GAUSS_WIDTH :
// QDOAS ???      case TOOL_CONVOLUTION_GAUSS_WIDTH_NM :
// QDOAS ???      case TOOL_CONVOLUTION_SLIT_BUTTON :
// QDOAS ???      case TOOL_CONVOLUTION_SLIT_FILE :
// QDOAS ???      case TOOL_CONVOLUTION_INVPOLY_TEXT :
// QDOAS ???      case TOOL_CONVOLUTION_INVPOLY :
// QDOAS ???      case TOOL_CONVOLUTION_INVPOLY_2 :
// QDOAS ???       helpId=HLP_TOOLS_CONV_TAB2_DATA;
// QDOAS ???      break;
// QDOAS ???   // ----------------------------------------------------------------------------
// QDOAS ???      case TOOL_DECONVOLUTION_SLIT_TYPE :
// QDOAS ???      case TOOL_DECONVOLUTION_TYPE :
// QDOAS ???      case TOOL_DECONVOLUTION_GAUSS_WIDTH_TEXT :
// QDOAS ???      case TOOL_DECONVOLUTION_GAUSS_WIDTH :
// QDOAS ???      case TOOL_DECONVOLUTION_GAUSS_WIDTH_NM :
// QDOAS ???      case TOOL_DECONVOLUTION_SLIT_BUTTON :
// QDOAS ???      case TOOL_DECONVOLUTION_SLIT_FILE :
// QDOAS ???      case TOOL_DECONVOLUTION_INVPOLY_TEXT :
// QDOAS ???      case TOOL_DECONVOLUTION_INVPOLY :
// QDOAS ???      case TOOL_DECONVOLUTION_INVPOLY_2 :
// QDOAS ???       helpId=HLP_TOOLS_CONV_TAB2_DECONV;
// QDOAS ???      break;
// QDOAS ???   // ----------------------------------------------------------------------------
// QDOAS ???     }
// QDOAS ???
// QDOAS ???   // Call help processor
// QDOAS ???
// QDOAS ???   if (helpId!=ITEM_NONE)
// QDOAS ???    WinHelp(hwndXsconv,DOAS_HelpPath,HELP_CONTEXT,helpId);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XsconvCommand
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         dispatch command messages from the convolution tools dialog box
// QDOAS ??? //
// QDOAS ??? // INPUT           usual syntax for windows messages routines
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? LRESULT CALLBACK XsconvCommand(HWND hwndXsconv,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch((ULONG)GET_WM_COMMAND_ID(mp1,mp2))
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDOK :
// QDOAS ???      XsconvOK(hwndXsconv);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDCANCEL :
// QDOAS ???      EndDialog(hwndXsconv,FALSE);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDHELP :
// QDOAS ???      XsConvHelp(hwndXsconv,ITEM_NONE);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XSCONV_WndProc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         dispatch messages from the convolution tools dialog box
// QDOAS ??? //
// QDOAS ??? // INPUT           usual syntax for windows messages routines
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? LRESULT CALLBACK XSCONV_WndProc(HWND hwndXsconv,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch (msg)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_INITDIALOG :
// QDOAS ???      XsconvInit(hwndXsconv);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_NOTIFY :
// QDOAS ???      XsconvNotify(hwndXsconv,(NMHDR *)mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_COMMAND :
// QDOAS ???      XsconvCommand(hwndXsconv,msg,mp1,mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_HELP :
// QDOAS ???      XsConvHelp(hwndXsconv,((LPHELPINFO)mp2)->iCtrlId);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // ==================
// QDOAS ??? // CONFIGURATION FILE
// QDOAS ??? // ==================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XSCONV_ResetConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Initialize the convolution options to default values
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void XSCONV_ResetConfiguration(void)
// QDOAS ???  {
// QDOAS ???  	// Declarations
// QDOAS ???
// QDOAS ???   PRJCT_FILTER *plFilter,*phFilter;                                             // pointer to the filter options
// QDOAS ???
// QDOAS ???   // Reset all the options
// QDOAS ???
// QDOAS ???   memset(&XSCONV_options,0,sizeof(XSCONV));
// QDOAS ???
// QDOAS ???   // Low-pass filtering
// QDOAS ???
// QDOAS ???   plFilter=&XSCONV_options.lfilter;
// QDOAS ???
// QDOAS ???   plFilter->fwhmWidth=                                                          // fwhm width for gaussian
// QDOAS ???   plFilter->kaiserCutoff=                                                       // cutoff frequency for kaiser filter type
// QDOAS ???   plFilter->kaiserPassBand=                                                     // pass band for kaiser filter type
// QDOAS ???   plFilter->kaiserTolerance=(float)0.;                                          // tolerance for kaiser filter type
// QDOAS ???   plFilter->filterNTimes=1;                                                     // number of times the filter has to be applied
// QDOAS ???
// QDOAS ???   // High-pass filtering
// QDOAS ???
// QDOAS ???   phFilter=&XSCONV_options.hfilter;
// QDOAS ???
// QDOAS ???   phFilter->fwhmWidth=                                                          // fwhm width for gaussian
// QDOAS ???   phFilter->kaiserCutoff=                                                       // cutoff frequency for kaiser filter type
// QDOAS ???   phFilter->kaiserPassBand=                                                     // pass band for kaiser filter type
// QDOAS ???   phFilter->kaiserTolerance=(float)0.;                                          // tolerance for kaiser filter type
// QDOAS ???   phFilter->filterNTimes=1;                                                     // number of times the filter has to be applied
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XSCONV_LoadConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Load last used information in the convolution tool box from
// QDOAS ??? //                 the wds configuration file
// QDOAS ??? //
// QDOAS ??? // INPUT           fileLine : the convolution line in the configuration file
// QDOAS ??? //
// QDOAS ??? // RETURN          0 if the input line is not related to the convolution options
// QDOAS ??? //                 1 if the input line is related to the convolution options
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC XSCONV_LoadConfiguration(UCHAR *fileLine)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR keyName[MAX_ITEM_NAME_LEN+1],                                           // key name of the input line
// QDOAS ???         text[MAX_STR_LEN+1];                                                    // the remaining of the input line
// QDOAS ???   RC    rc;                                                                     // return code
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   memset(keyName,0,MAX_ITEM_NAME_LEN+1);
// QDOAS ???   memset(text,0,MAX_STR_LEN+1);
// QDOAS ???
// QDOAS ???   rc=0;
// QDOAS ???
// QDOAS ???   if ((sscanf(fileLine,"%[^'=']=%[^'\n']",keyName,text)>=2) && !STD_Stricmp(keyName,XSCONV_SECTION))
// QDOAS ???    {
// QDOAS ???    	rc=1;
// QDOAS ???
// QDOAS ???     // Load new options
// QDOAS ???
// QDOAS ???     memset(&XSCONV_options,0,sizeof(XSCONV));
// QDOAS ???
// QDOAS ???     if (sscanf(text,"%d,%[^','],%[^','],%[^','],%[^','],%[^','],%d,%d,%lf,%lf,%[^','],%d,%lf,%lf,%[^','],%lf,%d,%f,%f,%f,%f,%d,%d,%d,%d,%d,%f,%f,%f,%f,%d,%d,%d,%d,%lf,%lf,%lf,%lf",
// QDOAS ???
// QDOAS ???                                                                                 // GENERAL
// QDOAS ???               (INT *)&XSCONV_options.convolutionType,                           // type of convolution (none, standard, I0 correction)
// QDOAS ???             (UCHAR *) XSCONV_options.shift,                                     // shift to apply to the original cross section
// QDOAS ???             (UCHAR *) XSCONV_options.crossFile,                                 // the name of the high-resoluted cross section file
// QDOAS ???             (UCHAR *) XSCONV_options.path,                                      // the output path
// QDOAS ???             (UCHAR *) XSCONV_options.calibrationFile,                           // the name of the wavelength calibration file
// QDOAS ???             (UCHAR *) XSCONV_options.kuruczFile,                                // the name of the high resoluted solar spectrum file (I0 correction)
// QDOAS ???               (INT *)&XSCONV_options.noComment,                                 // flag, 1 to save no comment lines in the output file
// QDOAS ???
// QDOAS ???                                                                                 // SLIT FUNCTION
// QDOAS ???               (INT *)&XSCONV_options.slitConv.slitType,                         // the type of the slit function to convolute the cross section
// QDOAS ???            (double *)&XSCONV_options.slitConv.slitParam,                        // the first parameter
// QDOAS ???            (double *)&XSCONV_options.slitConv.slitParam2,                       // the second parameter
// QDOAS ???             (UCHAR *) XSCONV_options.slitConv.slitFile,                         // the name of the slit function file
// QDOAS ???               (INT *)&XSCONV_options.slitDConv.slitType,                        // the type of the slit function to deconvolute the cross section
// QDOAS ???            (double *)&XSCONV_options.slitDConv.slitParam,                       // the first parameter (deconvolution)
// QDOAS ???            (double *)&XSCONV_options.slitDConv.slitParam2,                      // the second parameter (deconvolution)
// QDOAS ???             (UCHAR *) XSCONV_options.slitDConv.slitFile,                        // the name of the slit function file (deconvolution)
// QDOAS ???            (double *)&XSCONV_options.conc,                                      // concentration to use (I0 correction)
// QDOAS ???
// QDOAS ???                                                                                 // LOW-PASS FILTERING
// QDOAS ???               (int *)&XSCONV_options.lfilter.type,                              // type of filter
// QDOAS ???             (float *)&XSCONV_options.lfilter.fwhmWidth,                         // fwhm width for gaussian filters
// QDOAS ???             (float *)&XSCONV_options.lfilter.kaiserCutoff,                      // cutoff frequency for kaiser filter type
// QDOAS ???             (float *)&XSCONV_options.lfilter.kaiserPassBand,                    // pass band for kaiser filter type
// QDOAS ???             (float *)&XSCONV_options.lfilter.kaiserTolerance,                   // tolerance for kaiser filter type
// QDOAS ???               (int *)&XSCONV_options.lfilter.filterWidth,                       // filter width for boxcar, triangle and Savitsky-Golay filters
// QDOAS ???               (int *)&XSCONV_options.lfilter.filterOrder,                       // filter order for Savitsky-Golay filters
// QDOAS ???               (int *)&XSCONV_options.conversionMode,                            // conversion mode (vacuum to air or air to vacuum)
// QDOAS ???               (int *)&XSCONV_options.lfilter.filterNTimes,                      // the number of times the filter has to be applied
// QDOAS ???
// QDOAS ???                                                                                 // HIGH-PASS FILTERING
// QDOAS ???               (int *)&XSCONV_options.hfilter.type,                              // type of filter
// QDOAS ???             (float *)&XSCONV_options.hfilter.fwhmWidth,                         // fwhm width for gaussian filters
// QDOAS ???             (float *)&XSCONV_options.hfilter.kaiserCutoff,                      // cutoff frequency for kaiser filter type
// QDOAS ???             (float *)&XSCONV_options.hfilter.kaiserPassBand,                    // pass band for kaiser filter type
// QDOAS ???             (float *)&XSCONV_options.hfilter.kaiserTolerance,                   // tolerance for kaiser filter type
// QDOAS ???               (int *)&XSCONV_options.hfilter.filterWidth,                       // filter width for boxcar, triangle and Savitsky-Golay filters
// QDOAS ???               (int *)&XSCONV_options.hfilter.filterOrder,                       // filter order for Savitsky-Golay filters
// QDOAS ???               (int *)&XSCONV_options.hfilter.filterNTimes,                      // the number of times the filter has to be applied
// QDOAS ???               (int *)&XSCONV_options.hfilter.filterAction,                      // action according to the analysis mode (substraction or division)
// QDOAS ???
// QDOAS ???            (double *)&XSCONV_options.slitConv.slitParam3,                       // third parameter (convolution with Voigt function)
// QDOAS ???            (double *)&XSCONV_options.slitConv.slitParam4,                       // fourth parameter (convolution with Voigt function)
// QDOAS ???            (double *)&XSCONV_options.slitDConv.slitParam3,                      // third parameter (deconvolution with Voigt function)
// QDOAS ???            (double *)&XSCONV_options.slitDConv.slitParam4)>=1)                  // fourth parameter (deconvolution with Voigt function)
// QDOAS ???      {
// QDOAS ???      	// Compact the files names
// QDOAS ???
// QDOAS ???       FILES_CompactPath(XSCONV_options.crossFile,XSCONV_options.crossFile,1,1);
// QDOAS ???       FILES_CompactPath(XSCONV_options.path,XSCONV_options.path,1,1);
// QDOAS ???       FILES_CompactPath(XSCONV_options.calibrationFile,XSCONV_options.calibrationFile,1,1);
// QDOAS ???       FILES_CompactPath(XSCONV_options.kuruczFile,XSCONV_options.kuruczFile,1,1);
// QDOAS ???       FILES_CompactPath(XSCONV_options.slitConv.slitFile,XSCONV_options.slitConv.slitFile,1,1);
// QDOAS ???       FILES_CompactPath(XSCONV_options.slitDConv.slitFile,XSCONV_options.slitDConv.slitFile,1,1);
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        XSCONV_SaveConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Save the last information from the convolution tool box in
// QDOAS ??? //                 the wds configuration file
// QDOAS ??? //
// QDOAS ??? // INPUT           fp          : pointer to the current wds configuration file
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void XSCONV_SaveConfiguration(FILE *fp)
// QDOAS ???  {
// QDOAS ???   fprintf(fp,"%s=%d,%s,%s,%s,%s,%s,%d,%d,%lf,%lf,%s,%d,%lf,%lf,%s,%lf,%d,%.3f,%.3f,%.3f,%.3f,%d,%d,%d,%d,%d,%.3f,%.3f,%.3f,%.3f,%d,%d,%d,%d,%.3f,%.3f,%.3f,%.3f\n",
// QDOAS ???
// QDOAS ???               XSCONV_SECTION,                                                   // convolution key name
// QDOAS ???
// QDOAS ???                                                                                 // GENERAL
// QDOAS ???               XSCONV_options.convolutionType,                                   // type of convolution (none, standard, I0 correction)
// QDOAS ???               XSCONV_options.shift,                                             // shift to apply to the original cross section
// QDOAS ???               XSCONV_options.crossFile,                                         // the name of the high-resoluted cross section file
// QDOAS ???               XSCONV_options.path,                                              // the output path
// QDOAS ???               XSCONV_options.calibrationFile,                                   // the name of the wavelength calibration file
// QDOAS ???               XSCONV_options.kuruczFile,                                        // the name of the high resoluted solar spectrum file (I0 correction)
// QDOAS ???               XSCONV_options.noComment,                                         // flag, 1 to save no comment lines in the output file
// QDOAS ???
// QDOAS ???                                                                                 // SLIT FUNCTION
// QDOAS ???               XSCONV_options.slitConv.slitType,                                 // the type of the slit function to convolute the cross section
// QDOAS ???               XSCONV_options.slitConv.slitParam,                                // the first parameter
// QDOAS ???               XSCONV_options.slitConv.slitParam2,                               // the second parameter
// QDOAS ???               XSCONV_options.slitConv.slitFile,                                 // the name of the slit function file
// QDOAS ???               XSCONV_options.slitDConv.slitType,                                // the type of the slit function to deconvolute the cross section
// QDOAS ???               XSCONV_options.slitDConv.slitParam,                               // the first parameter (deconvolution)
// QDOAS ???               XSCONV_options.slitDConv.slitParam2,                              // the second parameter (deconvolution)
// QDOAS ???               XSCONV_options.slitDConv.slitFile,                                // the name of the slit function file (deconvolution)
// QDOAS ???               XSCONV_options.conc,                                              // concentration to use (I0 correction)
// QDOAS ???
// QDOAS ???                                                                                 // LOW-PASS FILTERING
// QDOAS ???          (INT)XSCONV_options.lfilter.type,                                      // type of filter
// QDOAS ???               XSCONV_options.lfilter.fwhmWidth,                                 // fwhm width for gaussian filters
// QDOAS ???               XSCONV_options.lfilter.kaiserCutoff,                              // cutoff frequency for kaiser filter type
// QDOAS ???               XSCONV_options.lfilter.kaiserPassBand,                            // pass band for kaiser filter type
// QDOAS ???               XSCONV_options.lfilter.kaiserTolerance,                           // tolerance for kaiser filter type
// QDOAS ???               XSCONV_options.lfilter.filterWidth,                               // filter width for boxcar, triangle and Savitsky-Golay filters
// QDOAS ???               XSCONV_options.lfilter.filterOrder,                               // filter order for Savitsky-Golay filters
// QDOAS ???               XSCONV_options.conversionMode,                                    // conversion mode (vacuum to air or air to vacuum)
// QDOAS ???               XSCONV_options.lfilter.filterNTimes,                              // the number of times the filter has to be applied
// QDOAS ???
// QDOAS ???                                                                                 // HIGH-PASS FILTERING
// QDOAS ???          (INT)XSCONV_options.hfilter.type,                                      // type of filter
// QDOAS ???               XSCONV_options.hfilter.fwhmWidth,                                 // fwhm width for gaussian filters
// QDOAS ???               XSCONV_options.hfilter.kaiserCutoff,                              // cutoff frequency for kaiser filter type
// QDOAS ???               XSCONV_options.hfilter.kaiserPassBand,                            // pass band for kaiser filter type
// QDOAS ???               XSCONV_options.hfilter.kaiserTolerance,                           // tolerance for kaiser filter type
// QDOAS ???               XSCONV_options.hfilter.filterWidth,                               // filter width for boxcar, triangle and Savitsky-Golay filters
// QDOAS ???               XSCONV_options.hfilter.filterOrder,                               // filter order for Savitsky-Golay filters
// QDOAS ???               XSCONV_options.hfilter.filterNTimes,                              // the number of times the filter has to be applied
// QDOAS ???               XSCONV_options.hfilter.filterAction,                              // action according to the analysis mode (substraction or division)
// QDOAS ???
// QDOAS ???               XSCONV_options.slitConv.slitParam3,                               // third parameter (convolution with Voigt function)
// QDOAS ???               XSCONV_options.slitConv.slitParam4,                               // fourth parameter (convolution with Voigt function)
// QDOAS ???               XSCONV_options.slitDConv.slitParam3,                              // third parameter (deconvolution with Voigt function)
// QDOAS ???               XSCONV_options.slitDConv.slitParam4);                             // fourth parameter (deconvolution with Voigt function)
// QDOAS ???  }
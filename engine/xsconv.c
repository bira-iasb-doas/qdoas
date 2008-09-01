
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
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
//  XSCONV_FctGauss - gaussian function;
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
//  XSCONV_NewSlitFunction - replace slit function by a new one when a deconvolution slit function is given;
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

// #include "../mediator/mediate_response.h"
// #include "../mediator/mediate_types.h"
// #include "../mediator/mediate_xsconv.h"

#include "doas.h"

// =====================
// CONSTANTS DEFINITIONS
// =====================

#define XSCONV_SECTION "Convolution"

// Slit types

DoasCh *XSCONV_slitTypes[SLIT_TYPE_MAX]=
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
  "Gaussian, wavelength+t° dependent",
  "Error function, wavelength+t° dependent"
 };

// =====================
// SUPPORTED LINE SHAPES
// =====================

// -----------------------------------------------------------------------------
// FUNCTION      XSCONV_FctGauss
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

RC XSCONV_FctGauss(double *pValue,double fwhm,double step,double delta)
 {
  // Declarations

  double sigma,a,ia;                                                            // temporary variables
  RC rc;                                                                        // return code

  // Initializations

  *pValue=(double)0.;
  rc=ERROR_ID_NO;

  // Gaussian function

  if (fwhm<=(double)0.)
   rc=ERROR_SetLast("XSCONV_FctGauss",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"fwhm<=0");
  else if (step<=(double)0.)
   rc=ERROR_SetLast("XSCONV_FctGauss",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"step<=0");
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
// INPUT         slitLambda  : the calibration (wavelength, wavenumber or pixel);
//               slitSize    : the size of previous vector;
//               slitType    : the type of line shape to build;
//               slitParam   : the list of slit function parameters
//               nSlitParam  : the number of elements in the previous list
//
// OUTPUT        slitVector  : the calculated line shape
//
// RETURN        rc          : return code
// -----------------------------------------------------------------------------

RC XsconvFctBuild(double *slitLambda,double *slitVector,INT slitSize,INT slitType,double *slitParam,INT nSlitParam)
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

    slitStep=(double)(slitLambda[slitSize-1]-slitLambda[0])/(slitSize-1);

    // Build the slit function; the message is dispatched according to the selected type of line shape

    switch(slitType)
     {
   // --------------------------------------------------------------------------
      case SLIT_TYPE_GAUSS :

       if (nSlitParam<1)
        rc=ERROR_SetLast("XsconvFctBuild",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"Gauss function needs 1 parameter");
       else
        for (i=0;(i<slitSize) && !rc;i++)
         rc=XSCONV_FctGauss(&slitVector[i],
                             slitParam[0],                                      // FWHM
                             slitStep,                                          // resolution of the line shape
                             slitLambda[i]);                                    // distance to the centre wavelength

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
                               slitLambda[i]);                                  // distance to the centre wavelength

       else if (slitType==SLIT_TYPE_APODNBS)
        for (i=0;(i<slitSize) && !rc;sum+=slitVector[i],i++)
         rc=XsconvFctApodNBS(&slitVector[i],
                                  slitParam[0],                                 // resolution
                                  slitParam[1],                                 // phase
                                  slitParam[2],                                 // dispersion
                                  slitLambda[i]);                               // distance to the centre wavelength

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
  if (pXsconv->lambda!=NULL)
   MEMORY_ReleaseDVector("XSCONV_Reset","lambda",pXsconv->lambda,0);
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
     ((pXsconv->lambda=MEMORY_AllocDVector("XSCONV_Alloc ","lambda",0,npts-1))==NULL) ||
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

    if ((rc=SPLINE_Vector(pSlit->lambda,pSlit->vector,pSlit->deriv2,pSlit->NDET,&x0,&max,1,SPLINE_CUBIC,"XsconvGetFwhm "))!=0)
     goto EndFwhm;

    max*=(double)0.5;

    // Search for the wavelength in the left part giving the value of function half way up

    for (lmin=pSlit->lambda[0],lmax=(double)0.,l1=lmin*0.5,x=(double)0.;
        (lmin<lmax) && fabs(x-max)>1.e-5;l1=(lmin+lmax)*0.5)
     {
      if ((rc=SPLINE_Vector(pSlit->lambda,pSlit->vector,pSlit->deriv2,pSlit->NDET,&l1,&x,1,SPLINE_CUBIC,"XsconvGetFwhm "))!=0)
       goto EndFwhm;

      if (x<max)
       lmin=l1;
      else if (x>max)
       lmax=l1;
     }

    // Search for the wavelength in the right part giving the value of function half way up

    for (lmin=(double)0.,lmax=pSlit->lambda[pSlit->NDET-1],l2=lmax*0.5,x=(double)0.;
        (lmin<lmax) && fabs(x-max)>1.e-5;l2=(lmin+lmax)*0.5)
     {
      if ((rc=SPLINE_Vector(pSlit->lambda,pSlit->vector,pSlit->deriv2,pSlit->NDET,&l2,&x,1,SPLINE_CUBIC,"XsconvGetFwhm "))!=0)
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
// QDOAS ??? void XSCONV_FileSelection(HWND hwndXsconv,DoasCh *file,MASK fileType,INT fileMode,INT fileCommand,INT ringFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   DoasCh *ptr;
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
// QDOAS ???       DoasCh pathSep[2];
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
// QDOAS ???   FILES_Select(hwndXsconv,file,MAX_ITEM_TEXT_LEN+1,fileType,0,(DoasCh)fileMode,NULL,ITEM_NONE,ITEM_NONE);
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

RC XSCONV_LoadCalibrationFile(XS *pLambda,DoasCh *lambdaFile,INT nextraPixels)
 {
  // Declarations

  DoasCh  lambdaBuffer[MAX_ITEM_TEXT_LEN+1];
  double step;
  FILE *lambdaFp;
  INT npts;
  INDEX i;
  RC rc;

  // Initializations

  XSCONV_Reset(pLambda);
  rc=ERROR_ID_NO;
  npts=0;

  // Open BINARY file

  FILES_RebuildFileName(lambdaBuffer,lambdaFile,1);

  if ((lambdaFp=fopen(FILES_RebuildFileName(lambdaBuffer,lambdaFile,1),"rt"))==NULL)
   rc=ERROR_SetLast("XSCONV_LoadCalibrationFile",ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,lambdaBuffer);
  else
   {
    // Get the number of lines

    while (!feof(lambdaFp) && fgets(lambdaBuffer,MAX_ITEM_TEXT_LEN,lambdaFp))
     if ((strchr(lambdaBuffer,';')==NULL) && (strchr(lambdaBuffer,'*')==NULL))
      npts++;

    if (!npts)
     rc=ERROR_SetLast("XSCONV_LoadCalibrationFile",ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,lambdaFile);

    // Buffers allocation

    else if (XSCONV_Alloc(pLambda,npts+2*nextraPixels,0))
     rc=ERROR_ID_ALLOC;

    // File read out

    else
     {
      // File read out

      VECTOR_Init(pLambda->lambda,(double)0.,npts+2*nextraPixels);
      fseek(lambdaFp,0L,SEEK_SET);

      for (i=0;(i<npts) && !feof(lambdaFp);)
       if (fgets(lambdaBuffer,MAX_ITEM_TEXT_LEN,lambdaFp) && (strchr(lambdaBuffer,';')==NULL) && (strchr(lambdaBuffer,'*')==NULL))
        {
         sscanf(lambdaBuffer,"%lf ",&pLambda->lambda[i+nextraPixels]);
         i++;
        }

      step=(pLambda->lambda[nextraPixels+npts-1]-pLambda->lambda[nextraPixels])/npts;

      for (i=0;i<nextraPixels;i++)
       {
        pLambda->lambda[i]=pLambda->lambda[nextraPixels]-step*(nextraPixels-i);
        pLambda->lambda[nextraPixels+npts+i]=pLambda->lambda[nextraPixels+npts-1]+step*i;
       }
     }
   }

  // Close file

  if (lambdaFp!=NULL)
   fclose(lambdaFp);

  // Return

  return rc;
 }

// ------------------------------------------------
// XSCONV_LoadSlitFunction : Slit function read out
// ------------------------------------------------

RC XSCONV_LoadSlitFunction(XS *pSlitXs,SLIT *pSlit,double *pGaussWidth,INT *pSlitType)
 {
  // Declarations

  DoasCh  slitBuffer[MAX_ITEM_TEXT_LEN+1];
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
           sscanf(slitBuffer,"%lf %lf",&pSlitXs->lambda[i],&pSlitXs->vector[i]);
           if (i)
            sumPoly+=(pSlitXs->vector[i]+pSlitXs->vector[i-1])*(pSlitXs->lambda[i]-pSlitXs->lambda[i-1])*0.5;

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
       !(rc=SPLINE_Deriv2(pSlitXs->lambda,pSlitXs->vector,pSlitXs->deriv2,nSlit,"XSCONV_LoadSlitFunction ")))

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
        pSlitXs->lambda[i]=(double)x;

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
       rc=XsconvFctBuild(pSlitXs->lambda,pSlitXs->vector,slitSize,pSlit->slitType,slitParam,4);

      if (!rc)
       {
        if ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_VOIGT))
         for (i=0;i<slitSize;i++)
          pSlitXs->vector[i]/=sumPoly;

        rc=SPLINE_Deriv2(pSlitXs->lambda,pSlitXs->vector,pSlitXs->deriv2,slitSize,"XSCONV_LoadSlitFunction ");

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

RC XSCONV_LoadCrossSectionFile(XS *pCross,DoasCh *crossFile,double lambdaMin,double lambdaMax,double shift,INT conversionMode)
 {
  // Declarations

  double lambda,crossValue,firstLambda;
  DoasCh  crossBuffer[MAX_ITEM_TEXT_LEN+1];
  FILE  *crossFp;
  INT    found,npts;
  INDEX  i,i0,iN,istep;
  RC     rc;

  // Initializations

  firstLambda=lambda=(double)0.;
  XSCONV_Reset(pCross);
  npts=found=0;
  rc=ERROR_ID_NO;

  // Open ASCII file



  if ((crossFp=fopen(FILES_RebuildFileName(crossBuffer,crossFile,1),"rt"))==NULL)
   rc=ERROR_SetLast("XSCONV_LoadCrossSectionFile",ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,crossBuffer);
  else
   {
    // First, search for data in the specified wavelengths range and determine the correct vector size

    while (!feof(crossFp) && fgets(crossBuffer,MAX_ITEM_TEXT_LEN,crossFp) && (!found || ((lambda>=lambdaMin) && (lambda<=lambdaMax))))

     if ((strchr(crossBuffer,';')==NULL) && (strchr(crossBuffer,'*')==NULL) && (sscanf(crossBuffer,"%lf %lf",&lambda,&crossValue)==2))
      {
       if (!found)
        firstLambda=lambda;

       if ((lambda>=lambdaMin) && (lambda<=lambdaMax))
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

      if (firstLambda<=lambda)
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
           (sscanf(crossBuffer,"%lf %lf",&lambda,&crossValue)==2) &&
           (lambda>=lambdaMin) && (lambda<=lambdaMax))
        {
         if (conversionMode==CONVOLUTION_CONVERSION_VAC2AIR)
          lambda=(double)0.1*(9.9972683*lambda+0.0107-(19.625/lambda));
         else if (conversionMode==CONVOLUTION_CONVERSION_AIR2VAC)
          lambda=(double)0.1*(10.0027325*lambda-0.0107+(19.625/lambda));

         pCross->lambda[i]=lambda+shift;
         pCross->vector[i]=crossValue;

         i+=istep;
        }

      // Second derivatives computation

      rc=SPLINE_Deriv2(pCross->lambda,pCross->vector,pCross->deriv2,npts,"XSCONV_LoadCrossSectionFile ");
     }
   }

  if (crossFp!=NULL)
   fclose(crossFp);

  // Return

  return rc;
 }

// =====================
// CONVOLUTION FUNCTIONS
// =====================

// ----------------------------------------------------------
// XSCONV_TypeNone : Apply no convolution, interpolation only
// ----------------------------------------------------------

RC XSCONV_TypeNone(XS *pXsnew,XS *pXshr)
 {
  return SPLINE_Vector(pXshr->lambda,pXshr->vector,pXshr->deriv2,pXshr->NDET,pXsnew->lambda,pXsnew->vector,pXsnew->NDET,SPLINE_CUBIC,"XSCONV_TypeNone ");
 }


RC XSCONV_TypeStandardFFT(FFT *pFFT,INT fwhmType,double slitParam,double slitParam2,double *lambda,double *target,INT size)
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
     rc=SPLINE_Vector(pFFT->fftIn+1,pFFT->invFftOut+1,pFFT->invFftIn+1,pFFT->oldSize,lambda,target,size,SPLINE_CUBIC,"XSCONV_TypeStandardFFT ");
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
//  - pXsnew->lambda : final wavelength scale (input);
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
// NB : pI->lambda==pXshr->lambda.
//

RC GetNewF(double *pNewF,
           INT     slitType,
           double *slitLambda,
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
   rc=SPLINE_Vector(slitLambda,slitVector,slitDeriv2,slitNDET,&dist,&newF,1,SPLINE_CUBIC,"GetNewF ");
  else if (slitType==SLIT_TYPE_APOD)
   rc=XsconvFctApod(&newF,slitParam,slitParam2,0.01,dist);
  else if (slitType==SLIT_TYPE_APODNBS)
   rc=XsconvFctApodNBS(&newF,slitParam,slitParam2,0.01,dist);
  else // if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_GAUSS_FILE))
   newF=(double)exp(-4.*log(2.)*(dist*dist)/(slitParam*slitParam));

//   rc=XSCONV_FctGauss(&newF,slitParam,step,dist);

  *pNewF=newF;

  return rc;
 }

RC XSCONV_TypeStandard(XS *pXsnew,INDEX indexLambdaMin,INDEX indexLambdaMax,XS *pXshr,XS *pSlit,XS *pI,double *Ic,INT slitType,double slitWidth,double slitParam,double slitParam2,double slitParam3,double slitParam4)
 {
  // Declarations

  double *xsnewLambda,*xsnewVector,
         *xshrLambda,*xshrVector,*xshrDeriv2,
         *slitLambda,*slitVector,*slitDeriv2,dist,
         *IVector,*IDeriv2,
          crossFIntegral,IFIntegral,FIntegral,
          oldF,newF,oldIF,newIF,stepF,h,
          slitCenter,c,si,
          lambda,lambdaMin,lambdaMax,oldXshr,newXshr;
  INDEX   xshrPixMin,
          xsnewIndex,indexOld,indexNew,
          klo,khi,i;
  INT     xshrNDET,xsnewNDET,slitNDET,msgCount;
  RC      rc;

  // Use substitution variables

  xsnewLambda=pXsnew->lambda;
  xsnewVector=pXsnew->vector;

  xshrLambda=pXshr->lambda;
  xshrVector=pXshr->vector;
  xshrDeriv2=pXshr->deriv2;

  if (pSlit!=NULL)
   {
    slitLambda=pSlit->lambda;
    slitVector=pSlit->vector;
    slitDeriv2=pSlit->deriv2;
    slitNDET=pSlit->NDET;
   }
  else
   {
    slitLambda=slitVector=slitDeriv2=NULL;
    slitNDET=0;
   }

  xsnewNDET=pXsnew->NDET;
  xshrNDET=pXshr->NDET;

  IVector=pI->vector;
  IDeriv2=pI->deriv2;

  // Initializations

  if (slitType==SLIT_TYPE_FILE)
   stepF=slitLambda[1]-slitLambda[0];
  else if ((slitType==SLIT_TYPE_APOD) ||(slitType==SLIT_TYPE_APODNBS))
   stepF=slitParam/50.;
  else
   stepF=slitParam/6.;
  msgCount=0;

  rc=ERROR_ID_NO;

// ADD AN OPTION LATER  if ((slitType==SLIT_TYPE_FILE) || (slitType==SLIT_TYPE_VOIGT))
// ADD AN OPTION LATER   {
// ADD AN OPTION LATER    // Calculation of the center of the slit function
// ADD AN OPTION LATER
// ADD AN OPTION LATER    for (i=0,c=si=(double)0.;i<slitNDET;i++)
// ADD AN OPTION LATER     {
// ADD AN OPTION LATER      c+=(double)slitLambda[i]*slitVector[i];
// ADD AN OPTION LATER      si+=(double)slitVector[i];
// ADD AN OPTION LATER     }
// ADD AN OPTION LATER
// ADD AN OPTION LATER    slitCenter=(double)c/si;
// ADD AN OPTION LATER   }
// ADD AN OPTION LATER  else
   {
    slitCenter=(double)0.;
    slitWidth=(double)3.*slitParam;
   }

  // Browse wavelengths in the final calibration vector

  for (xsnewIndex=max(0,indexLambdaMin);(xsnewIndex<xsnewNDET) && (xsnewIndex<indexLambdaMax) && !rc;xsnewIndex++)
   {
    lambda=xsnewLambda[xsnewIndex];

    if ((slitType==SLIT_TYPE_GAUSS_FILE) ||
        (slitType==SLIT_TYPE_INVPOLY_FILE) ||
//        (slitType==SLIT_TYPE_VOIGT_FILE) ||
        (slitType==SLIT_TYPE_ERF_FILE))
     {
      SPLINE_Vector(slitLambda,slitVector,slitDeriv2,slitNDET,&lambda,&slitParam,1,SPLINE_CUBIC,"XSCONV_TypeStandard ");

      stepF=slitParam/6.;            // 6 points/FWHM
      slitWidth=(double)3.*slitParam;
     }

    if (slitType!=SLIT_TYPE_FILE)
     {
      lambdaMin=lambda-slitWidth;
      lambdaMax=lambda+slitWidth;
     }
    else
     {
      lambdaMin=lambda+slitLambda[0];                     // add 1 nm
      lambdaMax=lambda+slitLambda[slitNDET-1];
     }

    // Search for first pixel in high resolution cross section in the wavelength range delimited by slit function

    for (klo=0,khi=xshrNDET-1;khi-klo>1;)
     {
      xshrPixMin=(khi+klo)>>1;

      if (xshrLambda[xshrPixMin]>lambdaMin)
       khi=xshrPixMin;
      else
       klo=xshrPixMin;
     }

    xshrPixMin=(xshrLambda[klo]<lambdaMin)?khi:klo;
    crossFIntegral=IFIntegral=FIntegral=(double)0.;

    // Case 1 : the resolution of cross section is better than the resolution of slit function => slit function interpolation only

    if (xshrLambda[xshrPixMin+1]-xshrLambda[xshrPixMin]<=stepF)
     {
      // set indexes to browse wavelengths in the grid of the high resolution cross section

      indexOld=xshrPixMin;
      indexNew=indexOld+1;

      // distance to the central wavelength

      dist=(double)slitCenter-(xshrLambda[indexOld]-lambda); // !!! slit function is inversed for convolution

      rc=GetNewF(&newF,slitType,slitLambda,slitVector,slitDeriv2,slitNDET,
                  dist,slitParam,slitParam2,slitParam3,slitParam4,xshrLambda[xshrPixMin+1]-xshrLambda[xshrPixMin]);

      // browse the grid of the high resolution cross section

      while ((indexNew<xshrNDET) && (xshrLambda[indexNew]<=lambdaMax) && !rc)
       {
        oldF=newF;

        dist=(double)slitCenter-(xshrLambda[indexNew]-lambda);

        // Convolution

        if (!(rc=GetNewF(&newF,slitType,slitLambda,slitVector,slitDeriv2,slitNDET,
                          dist,slitParam,slitParam2,slitParam3,slitParam4,xshrLambda[indexNew]-xshrLambda[indexOld])))
         {
          h=(xshrLambda[indexNew]-xshrLambda[indexOld])*0.5;  // use trapezium formula for surface computation (B+b)*H/2
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
        dist=lambda+slitLambda[indexOld];
        oldF=slitVector[indexOld];
       }
      else
       {
        dist=lambda-slitWidth;

        if ((rc=GetNewF(&oldF,slitType,slitLambda,slitVector,slitDeriv2,slitNDET,
                        dist-lambda,slitParam,slitParam2,slitParam3,slitParam4,stepF))!=ERROR_ID_NO)

         goto EndTypeStandard;
       }

      // Calculate first value for the high resolution cross section

      oldXshr=(double)0.;

      if ((dist<xshrLambda[0]) || (dist>xshrLambda[xshrNDET-1]))
       oldF=oldIF=oldXshr=(double)0.;
      else if (!(rc=SPLINE_Vector(xshrLambda,xshrVector,xshrDeriv2,xshrNDET,&dist,&oldXshr,1,SPLINE_CUBIC,"XSCONV_TypeStandard ")) && (Ic!=NULL))
       rc=SPLINE_Vector(xshrLambda,IVector,IDeriv2,xshrNDET,&dist,&oldIF,1,SPLINE_CUBIC,"XSCONV_TypeStandard ");

      // browse the grid of the slit function

      while ((((slitType==SLIT_TYPE_FILE) && (indexNew<slitNDET)) ||
              ((slitType!=SLIT_TYPE_FILE) && (dist+stepF<=lambda+slitWidth))) && !rc)
       {
        // the slit function is pre-calculated

        if (slitType==SLIT_TYPE_FILE)
         {
          dist=lambda+slitLambda[indexNew];
          newF=slitVector[indexNew];

          h=fabs((slitLambda[indexNew]-slitLambda[indexOld])*0.5);
          indexOld=indexNew++;
         }

        // the slit function is calculated now

        else
         {
          dist+=stepF;
          h=stepF*0.5;

          if ((rc=GetNewF(&newF,slitType,slitLambda,slitVector,slitDeriv2,slitNDET,
                           dist-lambda,slitParam,slitParam2,slitParam3,slitParam4,stepF))!=ERROR_ID_NO)

           goto EndTypeStandard;
         }

        if ((dist>=xshrLambda[0]) && (dist<=xshrLambda[xshrNDET-1]))
         {
          // interpolation of the high resolution cross section

          if ((rc=SPLINE_Vector(xshrLambda,xshrVector,xshrDeriv2,xshrNDET,&dist,&newXshr,1,SPLINE_CUBIC,"XSCONV_TypeStandard "))!=0)
           goto EndTypeStandard;

          // Convolution

          crossFIntegral+=(oldF*oldXshr+newF*newXshr)*h;
          FIntegral+=(oldF+newF)*h;

          // I0 correction

          if ((Ic!=NULL) && !(rc=SPLINE_Vector(xshrLambda,IVector,IDeriv2,xshrNDET,&dist,&newIF,1,SPLINE_CUBIC,"XSCONV_TypeStandard ")))
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

RC XSCONV_TypeI0Correction(XS *pXsnew,XS *pXshr,XS *pI0,XS *pSlit,double conc,INT slitType,double slitWidth,double slitParam,double slitParam2,double slitParam3,double slitParam4)
 {
  // Declarations

  double  sigma,
         *xshrLambda,*xshrVector,*xshrDeriv2,
         *xsnewLambda,*xsnewVector,
         *ILambda,*IVector,*IDeriv2,
         *I0Lambda,*I0Vector,*I0cVector,*IcVector;

  INT INDET,xshrNDET,xsnewNDET;
  INDEX i;
  XS I,I0c;
  RC rc;

  // Use substitution variables

  I0Lambda=pI0->lambda;
  I0Vector=pI0->vector;

  xshrLambda=pXshr->lambda;
  xshrVector=pXshr->vector;
  xshrDeriv2=pXshr->deriv2;

  xsnewLambda=pXsnew->lambda;
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

   rc=ERROR_SetLast("XSCONV_TypeI0Correction",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"General tab page : Conc. <= 0.");

  // Buffers allocation

  else if (XSCONV_Alloc(&I,INDET,1) ||
           XSCONV_Alloc(&I0c,xsnewNDET,0) ||
         ((IcVector=(double *)MEMORY_AllocDVector("XSCONV_TypeI0Correction ","IcVector",0,xsnewNDET-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    memcpy(I0c.lambda,xsnewLambda,sizeof(double)*xsnewNDET);

    // Use substitution variables

    ILambda=I.lambda;
    IVector=I.vector;
    IDeriv2=I.deriv2;

    I0cVector=I0c.vector;

    // Build I from I0 (solar spectrum) with the specified cross section absorption

    memcpy(ILambda,I0Lambda,sizeof(double)*INDET);

    VECTOR_Init(I0cVector,(double)0.,sizeof(double));
    VECTOR_Init(IcVector,(double)0.,sizeof(double));

    for (i=0;(i<INDET) && !rc;i++)
     if (!(rc=SPLINE_Vector(xshrLambda,xshrVector,xshrDeriv2,xshrNDET,&I0Lambda[i],&sigma,1,SPLINE_CUBIC,"XSCONV_TypeI0Correction ")))
      {
       if (-sigma*conc>(double)700.)
        rc=ERROR_SetLast("XSCONV_TypeI0Correction",ERROR_TYPE_FATAL,ERROR_ID_OVERFLOW);
       else
        IVector[i]=(double)I0Vector[i]*exp(-sigma*conc);
      }

    // I and I0 convolution

    if (!rc &&
        !(rc=SPLINE_Deriv2(ILambda,IVector,IDeriv2,INDET,"XSCONV_TypeI0Correction ")) &&               // I second derivatives calculation
        !(rc=XSCONV_TypeStandard(&I0c,0,xsnewNDET,pI0,pSlit,&I,IcVector,slitType,slitWidth,slitParam,slitParam2,slitParam3,slitParam4)))    // I0 and I convolution
     {
      // Cross section convolution

      for (i=0;(i<xsnewNDET) && !rc;i++)
       {
        if ((I0cVector[i]<=(double)0.) || (IcVector[i]<=(double)0.) || (conc<=(double)0.))
         xsnewVector[i]=(double)0.;
//         THRD_Error(ERROR_TYPE_FATAL,(rc=ERROR_ID_LOG),"XSCONV_TypeI0Correction ");
        else
         xsnewVector[i]=(double)log(I0cVector[i]/IcVector[i])/conc;
       }
     }
   }

  // Release allocated buffers

  XSCONV_Reset(&I);
  XSCONV_Reset(&I0c);

  if (IcVector!=NULL)
   MEMORY_ReleaseDVector("XSCONV_TypeI0Correction ","IcVector",IcVector,0);

  // Return

  return rc;
 }

// --------------------------------------------------------
// XSCONV_RealTimeXs : real time cross sections convolution
// --------------------------------------------------------

RC XSCONV_RealTimeXs(XS *pXshr,XS *pXsI0,XS *pSlit,double *IcVector,                         // high resolution cross section
                     double *lambda,INT NDET,INDEX indexLambdaMin,INDEX indexLambdaMax,               // final calibration wavelength scale
                     double *newXs,                                                          // convoluted cross section
                     INT slitType,double slitParam,double slitParam2,double slitParam3,double slitParam4)      // slit options
 {
  // Declarations

  XS xsnew;
  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Use substitution variables

  xsnew.lambda=lambda;
  xsnew.vector=newXs;
  xsnew.NDET=NDET;

  // process convolution

  if ((pSlit==NULL) || !(rc=XsconvGetFwhm(pSlit,slitType,&slitParam)))
   rc=XSCONV_TypeStandard(&xsnew,indexLambdaMin,indexLambdaMax,pXshr,pSlit,(pXsI0==NULL)?pXshr:pXsI0,IcVector,slitType,(double)3.*slitParam,slitParam,slitParam2,slitParam3,slitParam4);

  // Return

  return rc;
 }

// ---------------------------------------------------------------------------------
// XsconvRebuildSlitFunction : Rebuild slit function onto a regular wavelength scale
// ---------------------------------------------------------------------------------

RC XsconvRebuildSlitFunction(double *lambda,double *slit,INT nslit,SLIT *pSlit,XS *pSlitXs)
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
    slit[i]=(double)exp(-4.*log(2.)*(lambda[i]*lambda[i])/slitParam);

  else if (pSlit->slitType==SLIT_TYPE_INVPOLY)
   for (i=0,sigma=pSlit->slitParam*0.5,n=(double)pSlit->slitParam2;i<nslit;i++)
    slit[i]=(double)pow(sigma,n)/(pow(lambda[i],n)+pow(sigma,n));

  else if (pSlit->slitType==SLIT_TYPE_ERF)
   for (i=0,sigma=pSlit->slitParam*0.5,a=(double)sigma/sqrt(log(2.)),delta=(double)pSlit->slitParam2*0.5;i<nslit;i++)
    slit[i]=(double)(ERF_GetValue((lambda[i]+delta)/a)-ERF_GetValue((lambda[i]-delta)/a))/(4.*delta);

  else // slit type == SLIT_TYPE_FILE
   rc=SPLINE_Vector(pSlitXs->lambda,pSlitXs->vector,pSlitXs->deriv2,pSlitXs->NDET,lambda,slit,nslit,SPLINE_CUBIC,"XsconvRebuildSlitFunction ");

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
// XSCONV_NewSlitFunction : Replace slit function by a new one when a deconvolution slit function is given
// ------------------------------------------------------------------------------------------------------

RC XSCONV_NewSlitFunction(SLIT *pSlitOptions,XS *pSlit,double slitParam,SLIT *pSlit2Options,XS *pSlit2,double slitParam2)
 {
  // Declarations

  double *slit,*slit2,*newSlit,*lambda,              // substitution vectors
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

  if (((lambda=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","lambda",0,nslit))==NULL) ||
      ((slit=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","slit",0,nslit))==NULL) ||
      ((slit2=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","slit2",0,nslit))==NULL) ||
      ((newSlit=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","newSlit",0,nslit))==NULL) ||
      ((slitFFTin=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","slitFFTin",1,
              (n2=(int)pow((double)2.,ceil(log((double)nslit)/log((double)2.))))))==NULL) ||

      ((slit2FFTin=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","slit2FFTin",1,n2))==NULL) ||
      ((slitFFTout=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","slitFFTout",1,n2))==NULL) ||
      ((slit2FFTout=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","slit2FFTout",1,n2))==NULL))

   rc=ERROR_ID_ALLOC;

  // Both convolution and deconvolution slit functions are gaussian functions

  else if (((pSlit2Options->slitType==SLIT_TYPE_GAUSS) && (pSlitOptions->slitType==SLIT_TYPE_GAUSS)) ||
           ((pSlit2Options->slitType==SLIT_TYPE_ERF) && (pSlitOptions->slitType==SLIT_TYPE_ERF) && (pSlit2Options->slitParam2==pSlitOptions->slitParam2)))
   {
    if (slitParam2<slitParam)

     for (i=0,i0=nslit>>1,slitParam=slitParam*slitParam-slitParam2*slitParam2;i<nslit;i++)
      {
       lambda[i]=(double)slitStep*(i-i0);
       newSlit[i]=(double)exp(-4.*log(2.)*(lambda[i]*lambda[i])/slitParam);
      }
    else
     {
     	rc=ERROR_SetLast("XSCONV_NewSlitFunction",ERROR_TYPE_FATAL,ERROR_ID_GAUSSIAN,slitParam,slitParam2);
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
     lambda[i]=(double)slitStep*(i-i0);

    if ((rc=XsconvRebuildSlitFunction(lambda,slit2,nslit,pSlit2Options,pSlit2))!=0)
     goto EndNewSlit;

    // Vector normalization

    VECTOR_Init(slit2FFTin+1,(double)0.,n2);

    for (i=1,norm2=(double)0.;i<nslit;i++)
     norm2+=(slit2[i]+slit2[i-1]);

    if (norm2==(double)0.)
     {
     	rc=ERROR_SetLast("XSCONV_NewSlitFunction",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"calculation of a norm");
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

        if (lambda!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","lambda",lambda,0);
        if (slit!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit",slit,0);
        if (slit2!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit2",slit2,0);
        if (newSlit!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","newSlit",newSlit,0);
        if (slitFFTin!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slitFFTin",slitFFTin,1);
        if (slit2FFTin!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit2FFTin",slit2FFTin,1);
        if (slitFFTout!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slitFFTout",slitFFTout,1);
        if (slit2FFTout!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit2FFTout",slit2FFTout,1);

        slit=slit2=newSlit=slitFFTin=slit2FFTin=slitFFTout=slit2FFTout=NULL;

        // Allocate buffers with the new size

        if (((lambda=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","lambda",0,nslit))==NULL) ||
            ((slit=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","slit",0,nslit))==NULL) ||
            ((slit2=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","slit2",0,nslit))==NULL) ||
            ((newSlit=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","newSlit",0,nslit))==NULL) ||
            ((slitFFTin=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","slitFFTin",1,
                    (n2=(int)pow((double)2.,ceil(log((double)nslit)/log((double)2.))))))==NULL) ||

            ((slit2FFTin=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","slit2FFTin",1,n2))==NULL) ||
            ((slitFFTout=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","slitFFTout",1,n2))==NULL) ||
            ((slit2FFTout=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","slit2FFTout",1,n2))==NULL))
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
     lambda[i]=(double)slitStep*(i-i0);

    // Build deconvolution slit function

    if (((rc=XsconvRebuildSlitFunction(lambda,slit,nslit,pSlitOptions,pSlit))!=0) ||
        ((rc=XsconvRebuildSlitFunction(lambda,slit2,nslit,pSlit2Options,pSlit2))!=0))

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
     	rc=ERROR_SetLast("XSCONV_NewSlitFunction",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"calculation of a norm");
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
// QDOAS ???    DoasCh str[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???
// QDOAS ???    sprintf(str,"Convolution tool : effective slit function (%d x %.4f)",nslit,slitStep);
// QDOAS ???
// QDOAS ???    SendMessage(CHILD_hwndFrame,WM_MDIACTIVATE,(WPARAM)CHILD_list[CHILD_WINDOW_SPECTRA].hwndChild,(LPARAM)0);
// QDOAS ???
// QDOAS ???    DRAW_Spectra(CHILD_WINDOW_SPECTRA,str,"","Wavelength (nm)","",NULL,0,
// QDOAS ???                 (double)0.,(double)0.,(double)0.,(double)0.,
// QDOAS ???                 lambda,newSlit,nslit,DRAW_COLOR1,0,nslit-1,PS_SOLID,NULL,
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
    memcpy(pSlit->lambda,lambda,sizeof(double)*nslit);
    memcpy(pSlit->vector,newSlit,sizeof(double)*nslit);

    rc=SPLINE_Deriv2(pSlit->lambda,pSlit->vector,pSlit->deriv2,nslit,"XSCONV_NewSlitFunction ");
   }

  EndNewSlit :

  // Release allocated buffers

  if (lambda!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","lambda",lambda,0);
  if (slit!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit",slit,0);
  if (slit2!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit2",slit2,0);
  if (newSlit!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","newSlit",newSlit,0);
  if (slitFFTin!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slitFFTin",slitFFTin,1);
  if (slit2FFTin!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit2FFTin",slit2FFTin,1);
  if (slitFFTout!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slitFFTout",slitFFTout,1);
  if (slit2FFTout!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit2FFTout",slit2FFTout,1);

  // Return

  return rc;
 }


//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  INTERPOLATION FUNCTIONS
//  Name of module    :  SPLINE.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in WinDOAS package in June 97
//
//  Reference         :  Numerical recipes in C
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
//  This module provides functions for interpolation of tabulated functions.
//  Linear and cubic spline are supported.  In the second case, the second
//  derivatives of the function must be pre-calculated.
//
//  See Numerical Recipes for further description or comment.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SPLINE_Deriv2  calculates the second derivatives needed for cubic spline interpolation
//  SPLINE_Vector  function for linear and cubic interpolation;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <math.h>
#include <stdlib.h>

#include "spline.h"
#include "doas.h"

// -----------------------------------------------------------------------------
// FUNCTION      SPLINE_Deriv2
// -----------------------------------------------------------------------------
// PURPOSE       This function calculates the second derivatives needed for
//               cubic spline interpolation
//
// INPUT         X,Y               vectors defining the tabulated function Y=f(X);
//               n                 the size of previous vectors;
//               callingFunction   the name of the calling function for error
//                                 message if any;
//
// OUTPUT        Y2  second derivatives needed for evaluating the spline function
//
// RETURN        ERROR_ID_ALLOC if the allocation of a temporary buffer failed;
//               ERROR_ID_SPLINE if non increasing absissae are provided;
//               ERROR_ID_NO for success.
// -----------------------------------------------------------------------------

RC SPLINE_Deriv2(const double *X, const double *Y,double *Y2,int n,const char *callingFunction)
{
  // Declarations

  int i, k;
  double p, qn, sig, un, *u, yp1,ypn;
  RC rc;

  // Debugging

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_SHIFT_) && __DEBUG_DOAS_SHIFT_
  DEBUG_FunctionBegin("SPLINE_Deriv2",DEBUG_FCTTYPE_MATH);
#endif

  // Initializations

  yp1=ypn=(double)1.e30;                                                        // derivatives at boundaries are assumed unknown
  rc=ERROR_ID_NO;

  // Temporary buffer allocation

  if ((u=(double *)MEMORY_AllocDVector("SPLINE_Deriv2","u",0,n-1))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // consider lower boundary :

    if (yp1>(double)0.99e30)                                                    // The lower boundary condition is
     Y2[0]=u[0]=(double)0.;                                                     // set to be "natural" else to have
    else if (X[1]-X[0]<=(double)0.)
     rc=ERROR_SetLast(callingFunction,ERROR_TYPE_FATAL,ERROR_ID_SPLINE);
    else
     {
      Y2[0] = (double) -0.5;
      u [0] = (double) (3.0/(X[1]-X[0])) * ((Y[1]-Y[0])/(X[1]-X[0])-yp1);
     }

    // decomposition loop of the tridiagonal algorithm

    for (i=1;(i<n-1)&&!rc;i++)

     if (X[i+1]-X[i]<=0)
      rc=ERROR_SetLast(callingFunction,ERROR_TYPE_FATAL,ERROR_ID_SPLINE,i,i+1,X[i],X[i+1]);
     else
      {
       sig=(double)(X[i]-X[i-1])/(X[i+1]-X[i-1]);
       p=(double)sig*Y2[i-1]+2.;
       Y2[i]=(double)(sig-1.)/p;
       u[i]=(double)(Y[i+1]-Y[i])/(X[i+1]-X[i])-(Y[i]-Y[i-1])/(X[i]-X[i-1]);
       u[i]=(double)(6.*u[i]/(X[i+1]-X[i-1])-sig*u[i-1])/p;

       if (fabs(u[i])<(double)1.e-300)
	u[i]=(double)0.;
      }

    // consider upper boundary :

    if (!rc)
     {
      if (ypn>0.99e30)
       qn=un=(double)0.;
      else
       {
	qn=(double)0.5;
	un=(double)(3./(X[i]-X[i-1]))*(ypn-(Y[i]-Y[i-1])/(X[i]-X[i-1]));
       }

      Y2[i]=(un-qn*u[i-1])/(qn*Y2[i-1]+1.);

      // backsubstitution loop of the tridiagonal algorithm :

      for (k=i-1;k>=0;k--)
       Y2[k]=Y2[k]*Y2[k+1]+u[k];
     }

    MEMORY_ReleaseDVector("SPLINE_Deriv2","u",u,0);
   }

  // Debugging

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_SHIFT_) && __DEBUG_DOAS_SHIFT_
  if (rc)
   DEBUG_PrintVar("Input vectors",X,0,n-1,Y,0,n-1,NULL);
  DEBUG_FunctionStop("SPLINE_Deriv2",rc);
#endif

  // Return

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      SPLINE_Vector
// -----------------------------------------------------------------------------
// PURPOSE       function for linear and cubic interpolation
//
// INPUT         xa,ya             original vectors defining the tabulated function ya=f(xa);
//               y2a               needed for cubic spline only; second derivatives of the function
//               na                size of original vectors
//               xb                new absissae
//               nb                the size of xb;
//               type              interpolation type (SPLINE_LINEAR,SPLINE_CUBIC)
//               callingFunction   the name of the calling function for error
//                                 message if any;
//
// OUTPUT        yb                the function evaluated in new absissae xb.
//
// RETURN        ERROR_ID_SPLINE if non increasing absissae are provided;
//               ERROR_ID_NO for success.
//
// REMARK        the original function interpolates only on one new absissae;
//               this function applies on vector of new absissae.
// -----------------------------------------------------------------------------

RC SPLINE_Vector(const double *xa, const double *ya, const double *y2a,int na, const double *xb,double *yb,int nb,int type,const char *callingFunction)
{
  RC rc=ERROR_ID_NO;

  double xlo = xa[0];
  double xhi = xa[na-1];

  int k=0;

  // Browse new absissae

  for (int indexb=0;indexb<nb;indexb++) {
    double x=xb[indexb];
    
    // new absissae is out of boundaries
    if (x<=xa[0])
      yb[indexb]=ya[0];
    else if (x>=xa[na-1])
      yb[indexb]=ya[na-1];
    else {

      // search for k such that xa[k-1] < x <= xa[k],
      //
      // because we often interpolate many successive xa in a row, we
      // keep the 'k' value from the previous iteration and start from
      // there.  This als makes linear search faster than binary
      // search (we usually only have to search a few steps)
      
      if (k && xa[k-1] >= x) {
        // if this x is smaller than the one from the previous
        // iteration, reset k=0 and look from there.
        k = 0;
      }

      // 8-fold hand-unrolled loop seems to work well here...
      while (k+7 < na) {
        if (xa[k] >= x) {
          goto foundk;
        }
        if (xa[k+1] >= x) {
          k+=1; goto foundk;
        }
        if (xa[k+2] >= x) {
          k+=2; goto foundk;
        }
        if (xa[k+3] >= x) {
          k+=3; goto foundk;
        }
        if (xa[k+4] >= x) {
          k+=4; goto foundk;
        }
        if (xa[k+5] >= x) {
          k+=5; goto foundk;
        }
        if (xa[k+6] >= x) {
          k+=6; goto foundk;
        }
        if (xa[k+7] >= x) {
          k+=7; goto foundk;
        }
        k+=8;
      }
      for (; k<na && xa[k] < x; ++k);
    foundk:
      xlo=xa[k-1];
      xhi=xa[k];
      double h=xhi-xlo;

      if (h<=(double)0.)
        rc=ERROR_SetLast(callingFunction,ERROR_TYPE_WARNING,ERROR_ID_SPLINE,k-1,k,xa[k-1],xa[k]);
      else {
        // get ratios        
        double a = (xhi-x)/h;
        double b = 1.-a; // (x-xlo)/h;
       
        // interpolation
       
       yb[indexb] = a*ya[k-1]+b*ya[k]; // assume type == SPLINE_LINEAR or SPLINE_CUBIC
       if (type==SPLINE_CUBIC)
         yb[indexb] += ((a*a*a-a)*y2a[k-1]+(b*b*b-b)*y2a[k])*(h*h)/6.;
      }
    }
  }

  return rc;
}

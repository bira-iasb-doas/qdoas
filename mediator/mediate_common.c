
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  Functions of the mediator that are common to the
//                       QDOAS application and tools
//  Name of module    :  mediate_common.c
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

#include "mediate_common.h"

// -----------------------------------------------------------------------------
// FUNCTION      setMediateSlit
// -----------------------------------------------------------------------------
// PURPOSE       Slit function parameterization
// -----------------------------------------------------------------------------

void setMediateSlit(SLIT *pEngineSlit,const mediate_slit_function_t *pMediateSlit)
 {
  // Fields

  pEngineSlit->slitType=pMediateSlit->type;

  pEngineSlit->slitParam=
  pEngineSlit->slitParam2=
  pEngineSlit->slitParam3=
  pEngineSlit->slitParam4=(double)0.;

  switch(pEngineSlit->slitType)
   {
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_GAUSS :                                                      // Gaussian line shape
     pEngineSlit->slitParam=pMediateSlit->gaussian.fwhm;
    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_INVPOLY :                                                    // 2n-Lorentz (generalisation of the Lorentzian function
     pEngineSlit->slitParam=pMediateSlit->lorentz.width;
     pEngineSlit->slitParam2=pMediateSlit->lorentz.degree;
    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_VOIGT :                                                      // Voigt profile function
     pEngineSlit->slitParam=pMediateSlit->voigt.fwhmL;
     pEngineSlit->slitParam2=pMediateSlit->voigt.glRatioL;
     pEngineSlit->slitParam3=pMediateSlit->voigt.fwhmR;
     pEngineSlit->slitParam4=pMediateSlit->voigt.glRatioR;
    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_ERF :                                                        // error function (convolution of a Gaussian and a boxcar)
     pEngineSlit->slitParam=pMediateSlit->error.fwhm;
     pEngineSlit->slitParam2=pMediateSlit->error.width;
    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_APOD :                                                       // apodisation function (used with FTS)
     pEngineSlit->slitParam=pMediateSlit->boxcarapod.resolution;
     pEngineSlit->slitParam2=pMediateSlit->boxcarapod.phase;
    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_APODNBS :                                                    // apodisation function (Norton Beer Strong function)
     pEngineSlit->slitParam=pMediateSlit->nbsapod.resolution;
     pEngineSlit->slitParam2=pMediateSlit->nbsapod.phase;
    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_GAUSS_FILE :
     strcpy(pEngineSlit->slitFile,pMediateSlit->gaussianfile.filename);
    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_INVPOLY_FILE :                                               // 2n-Lorentz line shape, wavelength dependent (file)
     strcpy(pEngineSlit->slitFile,pMediateSlit->lorentzfile.filename);
     pEngineSlit->slitParam2=pMediateSlit->lorentzfile.degree;
    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_ERF_FILE :                                                   // error function, wavelength dependent (file)
     strcpy(pEngineSlit->slitFile,pMediateSlit->errorfile.filename);
     pEngineSlit->slitParam2=pMediateSlit->errorfile.width;
    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_GAUSS_T_FILE :
     strcpy(pEngineSlit->slitFile,pMediateSlit->gaussiantempfile.filename);
    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_ERF_T_FILE :
     strcpy(pEngineSlit->slitFile,pMediateSlit->errortempfile.filename);
    break;
 // ----------------------------------------------------------------------------
    default :
     strcpy(pEngineSlit->slitFile,pMediateSlit->file.filename);
    break;
 // ----------------------------------------------------------------------------
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateFilter
// -----------------------------------------------------------------------------
// PURPOSE       Filtering configuration
// -----------------------------------------------------------------------------

void setMediateFilter(PRJCT_FILTER *pEngineFilter,const mediate_filter_t *pMediateFilter)
 {
 } 
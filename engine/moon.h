
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS               
//  Module purpose    :  GEOCENTRIC MOON POSITIONS COMPUTATION
//  Name of module    :  MOON.H
//  Creation date     :  1996
//
//  ----------------------------------------------------------------------------
//
//  REFERENCES :
//
//  [1] Astronomy on the Personal Computer, Montenbruck & Pfleger, 1989-1994
//  [2] Astronomical formulae for calculators, Meeus, 1979
//  Qdoas is a cross-platform application for spectral analysis with the DOAS
//  algorithm (Differential Optical Analysis software) 
//
//  ----------------------------------------------------------------------------
//
//  LICENSE :
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

// ============
// HEADER FILES
// ============

#include "doas.h"

void MOON_GetPosition(DoasCh  *inputDate,                                        // input date and time for moon positions calculation
                      double  longitude,                                        // longitude of the observation site
                      double  latitude,                                         // latitude of the observation site
                      double  altitude,                                         // altitude of the observation site
                      double *pA,                                               // azimuth, measured westward from the south
                      double *ph,                                               // altitude above the horizon
                      double *pFrac);                                           // illuminated fraction of the moon


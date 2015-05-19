
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  RESOURCES MANAGEMENT MODULE
//  Name of module    :  RESOURCE.C
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
//  This module allows to change the value of some system variables (for example,
//  to increase the size of buffers).
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  FOR externAL USE :
//
//  RESOURCE_Alloc - memory allocation;
//  RESOURCE_Free - free memory allocated for resources.
//
//  ----------------------------------------------------------------------------

#include "resource.h"

#include "engine.h"
#include "output.h"
#include "erf.h"
#include "analyse.h"
#include "winsites.h"
#include "winsymb.h"

// -------------------------------------
// RESOURCE_Alloc : resources allocation
// -------------------------------------

RC RESOURCE_Alloc(void)
 {
  // Declaration

  RC rc;

  // Memory allocation

// QDOAS ???   SYS_Load();

  #if defined(__DEBUG_) && __DEBUG_
  MEMORY_Alloc();
  #endif

  if (!(rc=ERF_Alloc()) &&         // Error function
      !(rc=SITES_Alloc()) &&       // Observation sites
      !(rc=SYMB_Alloc()) &&        // Symbols
      !(rc=ANALYSE_Alloc()) &&     // Analysis
      !(rc=OUTPUT_Alloc()))        // Output

  // Initializations
   {
   }

  // Return;

  return rc;
 }

// ---------------------------------------------------
// RESOURCE_Free : free memory allocated for resources
// ---------------------------------------------------

void RESOURCE_Free(void)
 {
  ERF_Free();                 // Error function
  SITES_Free();               // Observation sites
  SYMB_Free();                // Symbols
  ANALYSE_Free();             // Analysis
  OUTPUT_Free();              // Output

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Start(ENGINE_dbgFile,"RESOURCE_Free > MEMORY_End",DEBUG_FCTTYPE_MEM,5,DEBUG_DVAR_NO,0);
  MEMORY_End();
  DEBUG_Stop("RESOURCE_Free > MEMORY_End");
  #endif
 }

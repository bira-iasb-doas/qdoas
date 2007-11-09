
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
//  Module purpose    :  RESOURCES MANAGEMENT MODULE
//  Name of module    :  RESOURCE.C
//  Program Language  :  Borland C++ 5.0 for Windows 95/NT
//  Creation date     :  1997
//  Author            :  Caroline FAYT
//
//        Copyright  (C) Belgian Institute for Space Aeronomy (BIRA-IASB)
//                       Avenue Circulaire, 3
//                       1180     UCCLE
//                       BELGIUM
//
//  As the WinDOAS software is distributed freely within the DOAS community, it
//  would be nice if BIRA-IASB Institute and the authors were mentioned at least
//  in acknowledgements of papers presenting results obtained with this program.
//
//  The source code is also available on request for use, modification and free
//  distribution but authors are not responsible of unexpected behaviour of the
//  program if changes have been made on the original code.
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
//  FOR EXTERNAL USE :
//
//  RESOURCE_Alloc - memory allocation;
//  RESOURCE_Free - free memory allocated for resources.
//
//  ----------------------------------------------------------------------------

#include "engine.h"

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
// QDOAS ???       !(rc=SITES_Alloc()) &&       // Observation sites
      !(rc=SYMB_Alloc()) &&        // Symbols
// QDOAS ???       !(rc=ANLYS_Alloc()) &&       // Analysis windows
// QDOAS ???       !(rc=PRJCT_Alloc()) &&       // Projects
// QDOAS ???       !(rc=RAW_Alloc()) &&         // Raw spectra files
      !(rc=ANALYSE_Alloc()))      // Analysis
// QDOAS ???       !(rc=OUTPUT_Alloc()) &&        // Output
// QDOAS ???       !(rc=PATH_Alloc()))          // Path

  // Initializations
   {
// QDOAS ???    PATH_Init();
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
// QDOAS ???   FILES_Free();               // Paths
// QDOAS ???   SITES_Free();               // Observation sites
  SYMB_Free();                // Symbols
// QDOAS ???   ANLYS_Free();               // Analysis windows
// QDOAS ???   PRJCT_Free();               // Projects
// QDOAS ???   RAW_Free();                 // Raw spectra files
  ANALYSE_Free();             // Analysis
// QDOAS ???   OUTPUT_Free();              // Output
// QDOAS ???   PATH_Free();                // Path

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Start(DOAS_dbgFile,"RESOURCE_Free > MEMORY_End",DEBUG_FCTTYPE_MEM,5,DEBUG_DVAR_NO,0);
  MEMORY_End();
  DEBUG_Stop("RESOURCE_Free > MEMORY_End");
  #endif
 }

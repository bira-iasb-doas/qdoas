
/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "mediate_xsconv.h"

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestConvolution
// -----------------------------------------------------------------------------
// PURPOSE       Transfer the convolution options from the GUI to the engine
//
// RETURN        ERROR_ID_NO if no error found
// -----------------------------------------------------------------------------

int mediateRequestConvolution(void *engineContext,mediate_convolution_t *pMediateConvolution,void *responseHandle)
 {
 	// Declarations
   ENGINE_XSCONV_CONTEXT *pEngineContext = (ENGINE_XSCONV_CONTEXT*)engineContext;
   RC rc;
   
   // Initializations
   
   rc=ERROR_ID_NO;

  // General information

  pEngineContext->convolutionType=pMediateConvolution->general.convolutionType;
  pEngineContext->conversionMode=pMediateConvolution->general.conversionType;
  pEngineContext->shift=pMediateConvolution->general.shift;
  pEngineContext->conc=pMediateConvolution->general.conc;
  pEngineContext->noComment=pMediateConvolution->general.noheader;

  strcpy(pEngineContext->crossFile,pMediateConvolution->general.inputFile);                 // high resolution cross section file
  strcpy(pEngineContext->path,pMediateConvolution->general.outputFile);                     // output path
  strcpy(pEngineContext->calibrationFile,pMediateConvolution->general.calibrationFile);     // calibration file
  strcpy(pEngineContext->kuruczFile,pMediateConvolution->general.solarRefFile);             // Kurucz file used when I0 correction is applied

  // Description of the slit function

  setMediateSlit(&pEngineContext->slitConv,&pMediateConvolution->conslit);
  setMediateSlit(&pEngineContext->slitDConv,&pMediateConvolution->decslit);

  // Filtering configuration

  setMediateFilter(&pEngineContext->lfilter,&pMediateConvolution->lowpass);
  setMediateFilter(&pEngineContext->lfilter,&pMediateConvolution->highpass);

 	// Return

 	return rc;    // supposed that an error at the level of the load of projects stops the current session
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateXsconvCreateContext
// -----------------------------------------------------------------------------
// PURPOSE       This function is called when one of the convolution tool is
//               started.  It creates a single context for safely accessing its
//               features through the mediator layer.  The engine context is
//               never destroyed before the user exits the program.
//
// RETURN        On success 0 is returned and the value of handleEngine is set,
//               otherwise -1 is retured and the value of handleEngine is undefined.
// -----------------------------------------------------------------------------

int mediateXsconvCreateContext(void **engineContext, void *responseHandle)
 {
 	ENGINE_XSCONV_CONTEXT *pEngineContext;

  if ((pEngineContext=*engineContext=(void *)EngineXsconvCreateContext())==NULL)
   ERROR_DisplayMessage(responseHandle);

  return (pEngineContext!=NULL)?0:-1;
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateXsconvDestroyContext
// -----------------------------------------------------------------------------
// PURPOSE       Destroy the engine context when the user exits the program.
//
// RETURN        Zero is returned on success, -1 otherwise.
// -----------------------------------------------------------------------------

int mediateXsconvDestroyContext(void *engineContext, void *responseHandle)
 {
  return (!EngineXsconvDestroyContext((ENGINE_XSCONV_CONTEXT *)engineContext))?0:-1;
 }

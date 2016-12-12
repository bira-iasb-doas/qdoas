
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  ENGINE CONTEXT
//  Name of module    :  ENGINE.C
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
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//
//  ----------------------------------------------------------------------------

#include <string.h>
#include <math.h>
#include <dirent.h>

#include "engine.h"

#include "spline.h"
#include "engine_context.h"
#include "winsites.h"
#include "winsymb.h"
#include "winfiles.h"
#include "resource.h"
#include "svd.h"
#include "analyse.h"
#include "vector.h"
#include "winthrd.h"
#include "kurucz.h"
#include "mediate.h"
#include "stdfunc.h"
#include "zenithal.h"
#include "output.h"
#include "tropomi_read.h"

#include "omi_read.h"
#include "gdp_bin_read.h"
#include "apex_read.h"
#include "spectrum_files.h"

#include "coda.h"

int ENGINE_refStartDate=0;                                                      // automatic reference selection : 0 use localday of records, 1 use starting date and time of the first measurements

ENGINE_CONTEXT engineContext,                                                   // engine context used to make the interface between the mediator and the engine
  ENGINE_contextRef,ENGINE_contextRef2;                                               // engine context used for the automatic search of the reference spectrum
char ENGINE_dbgFile[DOAS_MAX_PATH_LEN+1];
double ENGINE_localNoon;

// -----------------------------------------------------------------------------
// FUNCTION      EngineResetContext
// -----------------------------------------------------------------------------
// PURPOSE       Destroy the context of the current engine
//
// INPUT         pEngineContext     pointer to the engine context
//               closeFiles         1 to close files, 0 otherwise (in order not to close files twice)
// -----------------------------------------------------------------------------

void EngineResetContext(ENGINE_CONTEXT *pEngineContext)
 {
   // Declarations

   BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
   RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

#if defined(__DEBUG_) && __DEBUG_
   DEBUG_FunctionBegin("EngineResetContext",DEBUG_FCTTYPE_FILE);
#endif

   // Initializations

   pRecord=&pEngineContext->recordInfo;
   pBuffers=&pEngineContext->buffers;

   // Release buffers

   if (pBuffers->lambda!=NULL)
    MEMORY_ReleaseDVector("EngineResetContext ","lambda",pBuffers->lambda,0);
   if (pBuffers->instrFunction!=NULL)
    MEMORY_ReleaseDVector("EngineResetContext ","instrFunction",pBuffers->instrFunction,0);
   if (pBuffers->spectrum!=NULL)
    MEMORY_ReleaseDVector("EngineResetContext ","spectrum",pBuffers->spectrum,0);
   if (pBuffers->sigmaSpec!=NULL)
    MEMORY_ReleaseDVector("EngineResetContext ","sigmaSpec",pBuffers->sigmaSpec,0);
   if (pBuffers->lambda_irrad!=NULL)
    MEMORY_ReleaseDVector("EngineResetContext ","lambda_irrad",pBuffers->lambda_irrad,0);
   if (pBuffers->irrad!=NULL)
    MEMORY_ReleaseDVector("EngineResetContext ","irrad",pBuffers->irrad,0);
   if (pBuffers->darkCurrent!=NULL)
    MEMORY_ReleaseDVector("EngineResetContext ","darkCurrent",pBuffers->darkCurrent,0);
   if (pBuffers->offset!=NULL)
    MEMORY_ReleaseDVector("EngineResetContext ","offset",pBuffers->offset,0);
   if (pBuffers->scanRef!=NULL)
    MEMORY_ReleaseDVector("EngineResetContext ","scanRef",pBuffers->scanRef,0);
   if (pBuffers->specMaxx!=NULL)
    MEMORY_ReleaseDVector("EngineResetContext ","specMaxx",pBuffers->specMaxx,0);
   if (pBuffers->specMax!=NULL)
    MEMORY_ReleaseDVector("EngineResetContext ","specMax",pBuffers->specMax,0);
   if (pBuffers->varPix!=NULL)
    MEMORY_ReleaseDVector("EngineResetContext ","varPix",pBuffers->varPix,0);
   if (pBuffers->recordIndexes!=NULL)
    MEMORY_ReleaseBuffer("EngineResetContext ","recordIndexes",pBuffers->recordIndexes);

   if (pEngineContext->analysisRef.refIndexes!=NULL)
    MEMORY_ReleaseBuffer("EngineResetContext ","refIndexes",pEngineContext->analysisRef.refIndexes);
   if (pEngineContext->analysisRef.zmList!=NULL)
    MEMORY_ReleaseBuffer("EngineResetContext ","zmList",pEngineContext->analysisRef.zmList);
   if (pEngineContext->analysisRef.timeDec!=NULL)
    MEMORY_ReleaseBuffer("EngineResetContext ","timeDec",pEngineContext->analysisRef.timeDec);

   if (pBuffers->dnl.matrix!=NULL)
    MATRIX_Free(&pBuffers->dnl,"EngineResetContext (dnl)");
   if (pRecord->omi.omiPixelQF!=NULL)
    MEMORY_ReleaseBuffer("EngineResetContext","omiPixelQF",pRecord->omi.omiPixelQF);

   MFC_ResetFiles(pEngineContext);
   CCD_ResetInstrumental(&pRecord->ccd);
   ASCII_Free("EngineResetContext");

   // Reset structure

   memset(pEngineContext,0,sizeof(ENGINE_CONTEXT));

#if defined(__DEBUG_) && __DEBUG_
   DEBUG_FunctionStop("EngineResetContext",0);
#endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineCopyContext
// -----------------------------------------------------------------------------
// PURPOSE       Duplicate the engine context
//
// INPUT         pEngineContextTarget     pointer to the target engine context
//               pEngineContextSource     pointer to the source engine context
// -----------------------------------------------------------------------------

RC EngineCopyContext(ENGINE_CONTEXT *pEngineContextTarget,ENGINE_CONTEXT *pEngineContextSource)
 {
   // Declarations

   BUFFERS *pBuffersTarget,*pBuffersSource;
   RC rc;

   // Initializations

   pBuffersTarget=&pEngineContextTarget->buffers;
   pBuffersSource=&pEngineContextSource->buffers;
   rc=ERROR_ID_NO;

   // take maximum detector size to make sure that buffers allocated
   // here are big enough
   int max_ndet = 0;
   for (int i=0; i<ANALYSE_swathSize; ++i) {
     if (NDET[i] > max_ndet)
       max_ndet = NDET[i];
   }

   // Buffers allocation

   if (((pBuffersSource->lambda!=NULL) && (pBuffersTarget->lambda==NULL) &&
        ((pBuffersTarget->lambda=(double *)MEMORY_AllocDVector("EngineCopyContext","lambda",0,max_ndet-1))==NULL)) ||
       ((pBuffersSource->lambda_irrad!=NULL) && (pBuffersTarget->lambda_irrad==NULL) &&
        ((pBuffersTarget->lambda_irrad=(double *)MEMORY_AllocDVector("EngineCopyContext","lambda_irrad",0,max_ndet-1))==NULL)) ||
       ((pBuffersSource->instrFunction!=NULL) && (pBuffersTarget->instrFunction==NULL) &&
        ((pBuffersTarget->instrFunction=(double *)MEMORY_AllocDVector("EngineCopyContext","instrFunction",0,max_ndet-1))==NULL)) ||
       ((pBuffersSource->spectrum!=NULL) && (pBuffersTarget->spectrum==NULL) &&
        ((pBuffersTarget->spectrum=(double *)MEMORY_AllocDVector("EngineCopyContext","spectrum",0,max_ndet-1))==NULL)) ||
       ((pBuffersSource->sigmaSpec!=NULL) && (pBuffersTarget->sigmaSpec==NULL) &&
        ((pBuffersTarget->sigmaSpec=(double *)MEMORY_AllocDVector("EngineCopyContext","sigmaSpec",0,max_ndet-1))==NULL)) ||
       ((pBuffersSource->irrad!=NULL) && (pBuffersTarget->irrad==NULL) &&
        ((pBuffersTarget->irrad=(double *)MEMORY_AllocDVector("EngineCopyContext","irrad",0,max_ndet-1))==NULL)) ||
       ((pBuffersSource->darkCurrent!=NULL) && (pBuffersTarget->darkCurrent==NULL) &&
        ((pBuffersTarget->darkCurrent=(double *)MEMORY_AllocDVector("EngineCopyContext","darkCurrent",0,max_ndet-1))==NULL)) ||
       ((pBuffersSource->offset!=NULL) && (pBuffersTarget->offset==NULL) &&
        ((pBuffersTarget->offset=(double *)MEMORY_AllocDVector("EngineCopyContext","offset",0,max_ndet-1))==NULL)) ||
       ((pBuffersSource->scanRef!=NULL) && (pBuffersTarget->scanRef==NULL) &&
        ((pBuffersTarget->scanRef=(double *)MEMORY_AllocDVector("EngineCopyContext","scanRef",0,max_ndet-1))==NULL)) ||
       ((pBuffersSource->varPix!=NULL) && (pBuffersTarget->varPix==NULL) &&
        ((pBuffersTarget->varPix=(double *)MEMORY_AllocDVector("EngineCopyContext","varPix",0,max_ndet-1))==NULL)) ||
       ((pBuffersSource->specMaxx!=NULL) && (pBuffersTarget->specMaxx==NULL) &&
        ((pBuffersTarget->specMaxx=(double *)MEMORY_AllocDVector("EngineCopyContext","specMaxx",0,MAX_SPECMAX-1))==NULL)) ||
       ((pBuffersSource->specMax!=NULL) && (pBuffersTarget->specMax==NULL) &&
        ((pBuffersTarget->specMax=(double *)MEMORY_AllocDVector("EngineCopyContext","specMax",0,MAX_SPECMAX-1))==NULL)) ||
       ((pEngineContextSource->recordInfo.omi.omiPixelQF!=NULL) && (pEngineContextTarget->recordInfo.omi.omiPixelQF==NULL) &&
        ((pEngineContextTarget->recordInfo.omi.omiPixelQF=(unsigned short *)MEMORY_AllocBuffer("EngineCopyContext","omiPixelQF",max_ndet,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL)) ||
       ((pBuffersSource->recordIndexes!=NULL) && (pBuffersTarget->recordIndexes==NULL) &&
        ((pBuffersTarget->recordIndexes=(uint32_t *)MEMORY_AllocBuffer("THRD_CopySpecInfo","recordIndexes",
         (pEngineContextTarget->recordIndexesSize=pEngineContextSource->recordIndexesSize),sizeof(uint32_t),0,MEMORY_TYPE_ULONG))==NULL)) ||
       ((pEngineContextSource->recordInfo.ccd.vip.matrix!=NULL) && (pEngineContextTarget->recordInfo.ccd.vip.matrix==NULL) &&
        ((rc=MATRIX_Copy(&pEngineContextTarget->recordInfo.ccd.vip,&pEngineContextSource->recordInfo.ccd.vip,"EngineCopyContext"))!=ERROR_ID_NO)) ||
       ((pEngineContextSource->recordInfo.ccd.dnl.matrix!=NULL) && (pEngineContextTarget->recordInfo.ccd.dnl.matrix==NULL) &&
        ((rc=MATRIX_Copy(&pEngineContextTarget->recordInfo.ccd.dnl,&pEngineContextSource->recordInfo.ccd.dnl,"EngineCopyContext"))!=ERROR_ID_NO)) ||
       ((pEngineContextSource->recordInfo.ccd.drk.matrix!=NULL) && (pEngineContextTarget->recordInfo.ccd.drk.matrix==NULL) &&
        ((rc=MATRIX_Copy(&pEngineContextTarget->recordInfo.ccd.drk,&pEngineContextSource->recordInfo.ccd.drk,"EngineCopyContext"))!=ERROR_ID_NO)) ||
       ((pBuffersSource->dnl.matrix!=NULL) && (pBuffersTarget->dnl.matrix==NULL) &&
        ((rc=MATRIX_Copy(&pBuffersTarget->dnl,&pBuffersSource->dnl,"EngineCopyContext"))!=ERROR_ID_NO)))

    rc=ERROR_ID_ALLOC;

   else
    {
     if ((pBuffersTarget->lambda!=NULL) && (pBuffersSource->lambda!=NULL))
      memcpy(pBuffersTarget->lambda,pBuffersSource->lambda,sizeof(double)*max_ndet);
     if ((pBuffersTarget->lambda_irrad!=NULL) && (pBuffersSource->lambda_irrad!=NULL))
      memcpy(pBuffersTarget->lambda_irrad,pBuffersSource->lambda_irrad,sizeof(double)*max_ndet);
     if ((pBuffersTarget->instrFunction!=NULL) && (pBuffersSource->instrFunction!=NULL))
      memcpy(pBuffersTarget->instrFunction,pBuffersSource->instrFunction,sizeof(double)*max_ndet);
     if ((pBuffersTarget->spectrum!=NULL) && (pBuffersSource->spectrum!=NULL))
      memcpy(pBuffersTarget->spectrum,pBuffersSource->spectrum,sizeof(double)*max_ndet);
     if ((pBuffersTarget->sigmaSpec!=NULL) && (pBuffersSource->sigmaSpec!=NULL))
      memcpy(pBuffersTarget->sigmaSpec,pBuffersSource->sigmaSpec,sizeof(double)*max_ndet);
     if ((pBuffersTarget->irrad!=NULL) && (pBuffersSource->irrad!=NULL))
      memcpy(pBuffersTarget->irrad,pBuffersSource->irrad,sizeof(double)*max_ndet);
     if ((pBuffersTarget->darkCurrent!=NULL) && (pBuffersSource->darkCurrent!=NULL))
      memcpy(pBuffersTarget->darkCurrent,pBuffersSource->darkCurrent,sizeof(double)*max_ndet);
     if ((pBuffersTarget->offset!=NULL) && (pBuffersSource->offset!=NULL))
      memcpy(pBuffersTarget->offset,pBuffersSource->offset,sizeof(double)*max_ndet);
     if ((pBuffersTarget->scanRef!=NULL) && (pBuffersSource->scanRef!=NULL))
      memcpy(pBuffersTarget->scanRef,pBuffersSource->scanRef,sizeof(double)*max_ndet);
     if ((pBuffersTarget->varPix!=NULL) && (pBuffersSource->varPix!=NULL))
      memcpy(pBuffersTarget->varPix,pBuffersSource->varPix,sizeof(double)*max_ndet);
     if ((pBuffersTarget->specMaxx!=NULL) && (pBuffersSource->specMaxx!=NULL))
      memcpy(pBuffersTarget->specMaxx,pBuffersSource->specMaxx,sizeof(double)*MAX_SPECMAX);
     if ((pBuffersTarget->specMax!=NULL) && (pBuffersSource->specMax!=NULL))
      memcpy(pBuffersTarget->specMax,pBuffersSource->specMax,sizeof(double)*MAX_SPECMAX);
     if ((pEngineContextTarget->recordInfo.omi.omiPixelQF!=NULL) && (pEngineContextSource->recordInfo.omi.omiPixelQF!=NULL))
      memcpy(pEngineContextTarget->recordInfo.omi.omiPixelQF,pEngineContextSource->recordInfo.omi.omiPixelQF,sizeof(unsigned short)*max_ndet);
     if ((pBuffersTarget->recordIndexes!=NULL) && (pBuffersSource->recordIndexes!=NULL))
      memcpy(pBuffersTarget->recordIndexes,pBuffersSource->recordIndexes,sizeof(uint32_t)*pEngineContextSource->recordIndexesSize);

     // Other structures

     memcpy(&pEngineContextTarget->project,&pEngineContextSource->project,sizeof(PROJECT));              // project options
     memcpy(&pEngineContextTarget->fileInfo,&pEngineContextSource->fileInfo,sizeof(FILE_INFO));          // the name of the file to load and file pointers
     memcpy(&pEngineContextTarget->recordInfo,&pEngineContextSource->recordInfo,sizeof(RECORD_INFO)-sizeof(CCD));
     memcpy(&pEngineContextTarget->calibFeno,&pEngineContextSource->calibFeno,sizeof(CALIB_FENO));
     memcpy(&pEngineContextTarget->analysisRef,&pEngineContextSource->analysisRef,sizeof(ANALYSIS_REF));

     // Do not make a copy of the following vectors; reset the address in order not to release vectors twice

     pEngineContextTarget->analysisRef.refIndexes=NULL;
 	   pEngineContextTarget->analysisRef.zmList=NULL;
 	   pEngineContextTarget->analysisRef.timeDec=NULL;

 	   pEngineContextTarget->recordInfo.mfcDoasis.fileNames=NULL;

     // Other fields

     pEngineContextTarget->recordNumber=pEngineContextSource->recordNumber;                              // total number of record in file
     pEngineContextTarget->recordIndexesSize=pEngineContextSource->recordIndexesSize;                    // size of 'recordIndexes' buffer
     pEngineContextTarget->recordSize=pEngineContextSource->recordSize;                                  // size of record if length fixed
     pEngineContextTarget->indexRecord=pEngineContextSource->indexRecord;
     pEngineContextTarget->indexFile=pEngineContextSource->indexFile;
     pEngineContextTarget->lastRefRecord=pEngineContextSource->lastRefRecord;
     pEngineContextTarget->lastSavedRecord=pEngineContextSource->lastSavedRecord;
    }

   // Return

   return rc;
 }

// =======
// PROJECT
// =======

// -----------------------------------------------------------------------------
// FUNCTION      EngineSetProject
// -----------------------------------------------------------------------------
// PURPOSE       Allocate buffers requested by the project
//
// INPUT         pEngineContext           pointer to the engine context
//
// RETURN        ERROR_ID_ALLOC           if the allocation of a buffer fails
//               ERROR_ID_FILE_EMPTY      if the calibration file is not large enough
//               ERROR_ID_FILE_NOT_FOUND  if a file (calibration or instrument) doesn't exist
// -----------------------------------------------------------------------------

RC EngineSetProject(ENGINE_CONTEXT *pEngineContext)
 {
   // Declarations

   PROJECT *pProject;                                                            // pointer to the current project
   PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
   BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
   RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

   double *lambdaInstr;                                                          // wavelength calibration of the instrument function
   double *instrFunction;                                                        // instrumental function
   double *instrDeriv2;                                                          // second derivative for the instrument function
   RC rc;                                                                        // return code
   FILE *fp;                                                                     // file pointer
   char str[MAX_ITEM_TEXT_LEN];                                              // buffer to read the lines of the file
   int i,indexSite;                                                              // index for loops and arrays

   // Initializations

   int max_ndet = 0;
   for (int i=0; i<ANALYSE_swathSize; ++i) {
     if (NDET[i] > max_ndet)
       max_ndet = NDET[i];
   }

   ANALYSE_plotKurucz=ANALYSE_plotRef=0;
   ANALYSE_indexLine=1;
   pBuffers=&pEngineContext->buffers;
   pProject=&pEngineContext->project;
   pInstrumental=&pProject->instrumental;
   pRecord=&pEngineContext->recordInfo;

   pEngineContext->lastRefRecord=0;
   pRecord->nSpecMax=0;
   pRecord->i_crosstrack=0;

   pEngineContext->mfcDoasisFlag=((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
                                  (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD))?1:0;

   pEngineContext->satelliteFlag=((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
                                  (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
                                  (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
                                  (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI) ||
                                  (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_TROPOMI) ||
                                  (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GOME2))?1:0;

   ENGINE_localNoon=(double)12.;

   THRD_localShift=(int)(((indexSite=SITES_GetIndex(pInstrumental->observationSite))!=ITEM_NONE)?
                         floor(SITES_itemList[indexSite].longitude/15.):0); // 24./360.

   rc=ERROR_ID_NO;

   // Allocate buffers

   if (((pBuffers->lambda=MEMORY_AllocDVector(__func__,"lambda",0,max_ndet-1))==NULL) ||
       ((pBuffers->spectrum=MEMORY_AllocDVector(__func__,"spectrum",0,max_ndet-1))==NULL) ||

       (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_OHP_96) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_HA_94) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MKZY)) &&

        ((pBuffers->recordIndexes=(uint32_t *)MEMORY_AllocBuffer(__func__,"recordIndexes",2001,sizeof(uint32_t),0,MEMORY_TYPE_ULONG))==NULL)) ||

       (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_OHP_96) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_HA_94) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE)) &&

        (((pBuffers->specMaxx=(double *)MEMORY_AllocDVector(__func__,"specMaxx",0,MAX_SPECMAX-1))==NULL) ||
        ((pBuffers->specMax=(double *)MEMORY_AllocDVector(__func__,"specMax",0,MAX_SPECMAX-1))==NULL))) ||


       ( ( pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII ||
           pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN ||
           pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS ||
           pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI ||
           pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_TROPOMI ||
           pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GOME2 ) &&

         ( (pBuffers->sigmaSpec=MEMORY_AllocDVector(__func__,"sigmaSpec",0,max_ndet-1))==NULL ||
           (pBuffers->lambda_irrad=MEMORY_AllocDVector(__func__,"lambda_irrad",0,max_ndet-1))==NULL ||
           (pBuffers->irrad=MEMORY_AllocDVector(__func__,"irrad",0,max_ndet-1))==NULL
           )
         ) ||

       ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI) &&
        ((pRecord->omi.omiPixelQF=(unsigned short *)MEMORY_AllocBuffer(__func__,"omiPixelQF",max_ndet,sizeof(unsigned short),0,MEMORY_TYPE_USHORT))==NULL)) ||

       ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MKZY) &&
        ((pBuffers->scanRef=MEMORY_AllocDVector(__func__,"scanRef",0,max_ndet-1))==NULL)) ||

       (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_ACTON) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MKZY) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)) &&
        ((pBuffers->darkCurrent=MEMORY_AllocDVector(__func__,"darkCurrent",0,max_ndet-1))==NULL)) ||

       (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MKZY) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_BIRA)) &&
        ((pBuffers->offset=MEMORY_AllocDVector(__func__,"offset",0,max_ndet-1))==NULL)) ||

       ( ((((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)) && strlen(pInstrumental->vipFile)) ||
            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_BIRA)) &&

         ((pBuffers->varPix=MEMORY_AllocDVector(__func__,"varPix",0,max_ndet-1))==NULL)) ||

       // Load the detector non linearity file

       ((strlen(pInstrumental->dnlFile)>0) && (pBuffers->dnl.matrix==NULL) &&
        ((rc=MATRIX_Load(pInstrumental->dnlFile,
                         &pBuffers->dnl,
                         0,0,
                         0.,0.,
                         1,0,"EngineSetProject (dnl)"))!=ERROR_ID_NO)))

    rc=ERROR_ID_ALLOC;

   else {
     // Buffers initialization

     if (pBuffers->specMaxx!=NULL)
       for (i=0;i<MAX_SPECMAX;i++)
         pBuffers->specMaxx[i]=(double)i+1;

     // Load the wavelength calibration

     if (!strlen(pInstrumental->calibrationFile) || (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI))
       for (i=0;i<max_ndet;i++)
         pBuffers->lambda[i] = 0.0;
     else if ((fp=fopen(pInstrumental->calibrationFile,"rt"))==NULL)
       rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pInstrumental->calibrationFile);
     else {
       for (i=0;i<max_ndet;) {
         if (!fgets(str,MAX_ITEM_TEXT_LEN,fp)) {
           break;
         } else if ((strchr(str,';')==NULL) && (strchr(str,'*')==NULL)) {
           sscanf(str,"%lf",&pBuffers->lambda[i]);
           i++;
         }
       }
       if (i!=max_ndet)
         rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,pInstrumental->calibrationFile);
       fclose(fp);
      }

     if (!rc)
      {
       // Initialize buffers

       if (pBuffers->darkCurrent!=NULL)
        VECTOR_Init(pBuffers->darkCurrent,(double)0.,max_ndet);                  // To check the initialization of the ANALYSE_zeros vector ...
       if (pBuffers->offset!=NULL)
        VECTOR_Init(pBuffers->offset,(double)0.,max_ndet);                       // To check the initialization of the ANALYSE_zeros vector ...
       if (pBuffers->scanRef!=NULL)
        VECTOR_Init(pBuffers->scanRef,(double)0.,max_ndet);                      // To check the initialization of the ANALYSE_zeros vector ...

       // Load the instrumental function                                           // QDOAS !!! LOAD vip + dnl

       if (strlen(pInstrumental->instrFunction))
        {
         lambdaInstr=instrFunction=instrDeriv2=NULL;

         if ((fp=fopen(pInstrumental->instrFunction,"rt"))==NULL)
          rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pInstrumental->instrFunction);
         else if (((pBuffers->instrFunction=MEMORY_AllocDVector(__func__,"instrFunction",0,max_ndet-1))==NULL) ||
                  ((lambdaInstr=MEMORY_AllocDVector(__func__,"lambdaInstr",0,max_ndet-1))==NULL) ||
                  ((instrFunction=MEMORY_AllocDVector(__func__,"instrFunction",0,max_ndet-1))==NULL) ||
                  ((instrDeriv2=MEMORY_AllocDVector(__func__,"instrDeriv2",0,max_ndet-1))==NULL))

          rc=ERROR_ID_ALLOC;

         else
          {
           for (i=0;(i<max_ndet) && fgets(str,MAX_ITEM_TEXT_LEN,fp);)
            if ((strchr(str,';')==NULL) && (strchr(str,'*')==NULL))
             {
              sscanf(str,"%lf %lf",&lambdaInstr[i],&instrFunction[i]);
              i++;
             }

           if (!SPLINE_Deriv2(lambdaInstr,instrFunction,instrDeriv2,max_ndet,__func__))
            rc=SPLINE_Vector(lambdaInstr,instrFunction,instrDeriv2,max_ndet,pBuffers->lambda,pBuffers->instrFunction,max_ndet,SPLINE_CUBIC,__func__);
          }

         if (fp!=NULL)
          fclose(fp);

         // Release the allocated buffers

         if (lambdaInstr!=NULL)
          MEMORY_ReleaseDVector(__func__,"lambdaInstr",lambdaInstr,0);
         if (instrFunction!=NULL)
          MEMORY_ReleaseDVector(__func__,"instrFunction",instrFunction,0);
         if (instrDeriv2!=NULL)
          MEMORY_ReleaseDVector(__func__,"instrDeriv2",instrDeriv2,0);
        }

       // MFC : load dark current and offset

       if (!rc && pEngineContext->mfcDoasisFlag && !(rc=MFC_LoadOffset(pEngineContext)))
        rc=MFC_LoadDark(pEngineContext);
      }
    }

   // Return

   return rc;
 }

// =====
// FILES
// =====

// -----------------------------------------------------------------------------
// FUNCTION      EngineCloseFile
// -----------------------------------------------------------------------------
// PURPOSE       Close the files on end browsing spectra or session exit
//
// INPUT         pFile     pointer to the file part of the engine context
// -----------------------------------------------------------------------------

void EngineCloseFile(FILE_INFO *pFile)
 {
   // Close previous files

   if (pFile->specFp!=NULL)
    fclose(pFile->specFp);
   if (pFile->darkFp!=NULL)
    fclose(pFile->darkFp);
   if (pFile->namesFp!=NULL)
    fclose(pFile->namesFp);

   pFile->specFp=pFile->darkFp=pFile->namesFp=NULL;
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineSetFile
// -----------------------------------------------------------------------------
// PURPOSE       Initialize file pointers
//
// INPUT         pEngineContext     pointer to the engine context
//               fileName           the name of the file to browse
//
// RETURN        0 in case of success; the code of the error otherwise
// -----------------------------------------------------------------------------

RC EngineSetFile(ENGINE_CONTEXT *pEngineContext,const char *fileName,void *responseHandle)
 {
   // Declarations

   FILE_INFO *pFile;
   char fileTmp[MAX_STR_LEN+1];
   RC rc;

   // Initializations

   ENGINE_refStartDate=0;

   pFile=&pEngineContext->fileInfo;
   pEngineContext->recordInfo.oldZm=(double)-1;

   rc=ERROR_ID_NO;

   strcpy(pFile->fileName,fileName);
   strcpy(fileTmp,fileName);

   // About names of record

   // SAOZ : The spectra names are used to select zenith sky or pointed measurements.
   //        In principle, names files should be in the same directory as the spectra files
   //        For the moment, I suppose that the file exists and if not, the selection of the
   //        measurement is ignored.  To improve ???

   if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SAOZ_VIS) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD))

    pFile->namesFp=fopen(FILES_BuildFileName(fileTmp,FILE_TYPE_NAMES),"rb");

   // Dark current files : the file name is automatically built from the spectra file name

   if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_ACTON) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE))

    pFile->darkFp=fopen(FILES_BuildFileName(fileTmp,FILE_TYPE_DARK),"rb");

   // Some satellite measurements have their own functions to open the file

   if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) &&
       (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_TROPOMI) &&
       (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS) &&
       (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GOME2) &&
       ((pFile->specFp=fopen(pEngineContext->fileInfo.fileName,"rb"))==NULL))

    rc=ERROR_SetLast("EngineSetFile",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pFile->fileName);

   else

    switch((int)pEngineContext->project.instrumental.readOutFormat)
     {
      // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_ASCII :
       rc=ASCII_Set(pEngineContext,pFile->specFp);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_ACTON :
       rc=SetActon_Logger(pEngineContext,pFile->specFp);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_PDASI_EASOE :
       rc=SetEASOE(pEngineContext,pFile->specFp,pFile->namesFp);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_OCEAN_OPTICS :
       rc=SetOceanOptics(pEngineContext,pFile->specFp);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_PDAEGG :
       rc=SetPDA_EGG(pEngineContext,pFile->specFp,1);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_PDAEGG_OLD :
       rc=SetPDA_EGG(pEngineContext,pFile->specFp,0);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_LOGGER :
       rc=SetPDA_EGG_Logger(pEngineContext,pFile->specFp);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SAOZ_VIS :
       rc=SetSAOZ(pEngineContext,pFile->specFp);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SAOZ_EFM :
       rc=SetSAOZEfm(pEngineContext,pFile->specFp);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_BIRA_MOBILE :
     case PRJCT_INSTR_FORMAT_BIRA_AIRBORNE :
       rc=AIRBORNE_Set(pEngineContext,pFile->specFp);
       break;
     case PRJCT_INSTR_FORMAT_APEX :
       rc=apex_set(pEngineContext);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_MFC :
     case PRJCT_INSTR_FORMAT_MFC_STD :
       if (!(rc=SetMFC(pEngineContext,pFile->specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_EXPORT) && (THRD_id!=THREAD_TYPE_NONE))
        rc=MFC_LoadAnalysis(pEngineContext,responseHandle);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_MFC_BIRA :
        rc=MFCBIRA_Set(pEngineContext,pFile->specFp);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_RASAS :
       rc=SetRAS(pEngineContext,pFile->specFp);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_UOFT :
       rc=SetUofT(pEngineContext,pFile->specFp);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_NOAA :
       rc=SetNOAA(pEngineContext,pFile->specFp);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_OMI :
       rc=OMI_Set(pEngineContext);
       if(!rc)
         rc = OMI_load_analysis(pEngineContext, responseHandle);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_TROPOMI:
       rc=tropomi_set(pEngineContext);
       // todo: OMI_load_analysis
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_CCD_EEV :
       rc=SetCCD_EEV(pEngineContext,pFile->specFp,pFile->darkFp);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_CCD_OHP_96 :
       rc=SetCCD(pEngineContext,pFile->specFp,0);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_CCD_HA_94 :
       rc=SetCCD(pEngineContext,pFile->specFp,1);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_GDP_ASCII :
       rc=GDP_ASC_Set(pEngineContext,pFile->specFp);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_GDP_BIN :
       if (!(rc=GDP_BIN_Set(pEngineContext,pFile->specFp)) &&  (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_EXPORT) && (THRD_id!=THREAD_TYPE_NONE))
        rc=GDP_BIN_LoadAnalysis(pEngineContext,pEngineContext->fileInfo.specFp,responseHandle);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SCIA_PDS :
       if (!(rc=SCIA_SetPDS(pEngineContext)) &&  (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_EXPORT) && (THRD_id!=THREAD_TYPE_NONE))
        rc=SCIA_LoadAnalysis(pEngineContext,responseHandle);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_GOME2 :
       if (!(rc=GOME2_Set(pEngineContext)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_EXPORT) && (THRD_id!=THREAD_TYPE_NONE))
        rc=GOME2_LoadAnalysis(pEngineContext,responseHandle);
       break;
       // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_MKZY :
       if (!(rc=MKZY_Set(pEngineContext,pFile->specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_EXPORT) && (THRD_id!=THREAD_TYPE_NONE))
        rc=MKZY_LoadAnalysis(pEngineContext,responseHandle);
       break;
       // ---------------------------------------------------------------------------
     default :
       rc=ERROR_ID_FILE_FORMAT;
       break;
       // ---------------------------------------------------------------------------
     }

   // Return

   return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineReadFile
// -----------------------------------------------------------------------------
// PURPOSE       Dispatch the reading command according to the file format
//
// INPUT         pEngineContext     pointer to the engine context
//               indexRecord        index of the record to read
//               dateFlag           1 to search for a reference spectrum (GB)
//               localDay           if dateFlag is 1, the calendar day for the
//                                  reference spectrum to search for
// -----------------------------------------------------------------------------

RC EngineReadFile(ENGINE_CONTEXT *pEngineContext,int indexRecord,int dateFlag,int localCalDay)
 {
   // Declarations

   RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
   FILE_INFO *pFile;                                                             // pointer to the file part of the engine context
   INDEX indexSite;
   OBSERVATION_SITE *pSite;
   double longit,latit;
   int rc;                                                                       // Return code

   // Initializations

   pFile=&pEngineContext->fileInfo;
   pRecord=&pEngineContext->recordInfo;

   memset(pRecord->Nom,0,20);

   pRecord->Zm=-1.;
   pRecord->Azimuth=-1.;
   pRecord->SkyObs=8;
   pRecord->ReguTemp=0.;
   pRecord->TDet=0.;
   pRecord->BestShift=0.;
   pRecord->rejected=0;
   pRecord->NTracks=0;
   pRecord->elevationViewAngle=-1.;
   pRecord->azimuthViewAngle=-1.;

   pRecord->longitude=0.;
   pRecord->latitude=0.;
   pRecord->altitude=0.;
   pRecord->cloudFraction=(double)0.;
   pRecord->cloudTopPressure=(double)0.;

   pRecord->aMoon=0.;
   pRecord->hMoon=0.;
   pRecord->fracMoon=0.;

   pEngineContext->indexRecord=indexRecord;  // !!! for the output

   switch((int)pEngineContext->project.instrumental.readOutFormat)
    {
     // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_ASCII :
      rc=ASCII_Read(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_ACTON :
      rc=ReliActon_Logger(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp,pFile->namesFp,pFile->darkFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDASI_EASOE :
      rc=ReliEASOE(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp,pFile->namesFp,pFile->darkFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_OCEAN_OPTICS :
      rc=ReliOceanOptics(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDAEGG :
      rc=ReliPDA_EGG(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp,pFile->namesFp,pFile->darkFp,1);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDAEGG_OLD :
      rc=ReliPDA_EGG(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp,pFile->namesFp,pFile->darkFp,0);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_LOGGER :
      rc=ReliPDA_EGG_Logger(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_VIS :
      rc=ReliSAOZ(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp,pFile->namesFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_EFM :
      rc=ReliSAOZEfm(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_BIRA_MOBILE :
    case PRJCT_INSTR_FORMAT_BIRA_AIRBORNE :
      rc=AIRBORNE_Read(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_APEX :
      rc=apex_read(pEngineContext,indexRecord);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC :
      rc=ReliMFC(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp,pEngineContext->project.instrumental.mfcMaskSpec);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC_STD :
      rc=ReliMFCStd(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC_BIRA :
      rc=MFCBIRA_Reli(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_RASAS :
      rc=ReliRAS(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_UOFT :
      rc=ReliUofT(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_NOAA :
      rc=ReliNOAA(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_OMI :
      rc=OMI_read_earth(pEngineContext,indexRecord);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_TROPOMI :
      rc=tropomi_read(pEngineContext,indexRecord);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_CCD_EEV :
      rc=ReliCCD_EEV(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp,pFile->darkFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_CCD_HA_94 :
      rc=ReliCCD(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp,pFile->namesFp,pFile->darkFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_CCD_OHP_96 :
      rc=ReliCCDTrack(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp,pFile->namesFp,pFile->darkFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GDP_ASCII :
      rc=GDP_ASC_Read(pEngineContext,indexRecord,dateFlag,pFile->specFp);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GDP_BIN :
      rc=GDP_BIN_Read(pEngineContext,indexRecord,pFile->specFp,GDP_BIN_currentFileIndex);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SCIA_PDS :
      rc=SCIA_ReadPDS(pEngineContext,indexRecord);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GOME2 :
      rc=GOME2_Read(pEngineContext,indexRecord,ITEM_NONE);
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MKZY :
      rc=MKZY_Reli(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
      break;
      // ---------------------------------------------------------------------------
    default :
      rc=ERROR_ID_FILE_BAD_FORMAT;
      break;
      // ---------------------------------------------------------------------------
    }

   if (rc)
     return rc;

   int i_crosstrack = (indexRecord - 1) % ANALYSE_swathSize;
   const int n_wavel = NDET[i_crosstrack];

   rc=THRD_SpectrumCorrection(pEngineContext,pEngineContext->buffers.spectrum,n_wavel);

   if (rc)
     return rc;

   pEngineContext->indexRecord=indexRecord;
   if (pRecord->oldZm<(double)0.)
     pRecord->oldZm=pRecord->Zm;

   // Correction of the solar zenith angle with the geolocation of the specified observation site

   if ((indexSite=SITES_GetIndex(pEngineContext->project.instrumental.observationSite))!=ITEM_NONE) {
     pSite=&SITES_itemList[indexSite];

     longit=-pSite->longitude;   // !!! sign is inverted

     pRecord->longitude=-longit;
     pRecord->latitude=latit=(double)pSite->latitude;

     if (pSite->altitude>(double)0.)
       pRecord->altitude=pSite->altitude*0.001;

     pRecord->Zm=(pRecord->Tm!=(double)0.)?ZEN_FNTdiz(ZEN_FNCrtjul(&pRecord->Tm),&longit,&latit,&pRecord->Azimuth):(double)-1.;
   }

   return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineRequestBeginBrowseSpectra
// -----------------------------------------------------------------------------
// PURPOSE       Dispatch the reading command according to the file format
//
// INPUT         pEngineContext     pointer to the engine context
//               spectraFileName    the name of the file to process
//               threadType         type of thread action
//
// RETURN        0 in case of success; the code of the error otherwise
// -----------------------------------------------------------------------------

RC EngineRequestBeginBrowseSpectra(ENGINE_CONTEXT *pEngineContext,const char *spectraFileName,void *responseHandle)
 {
   // Declarations

   ANALYSIS_REF *pRef;
   int recordNumber;
   int resetFlag;
   RC rc;

   // Initializations

   resetFlag=(!pEngineContext->mfcDoasisFlag || (THRD_id!=THREAD_TYPE_ANALYSIS) || !pEngineContext->recordInfo.mfcDoasis.nFiles || (MFC_SearchForCurrentFileIndex(pEngineContext)==ITEM_NONE))?1:0;
   pEngineContext->recordInfo.mfcDoasis.resetFlag=resetFlag;

   pRef=&pEngineContext->analysisRef;
   rc=ERROR_ID_NO;

   // Set file pointers

   if (!(rc=EngineRequestEndBrowseSpectra(pEngineContext)) &&
       !(rc=EngineSetFile(pEngineContext,spectraFileName,responseHandle)) &&
       pEngineContext->recordNumber &&
       ((THRD_id==THREAD_TYPE_SPECTRA) || !(rc=OUTPUT_LocalAlloc(pEngineContext))))
     {
       pEngineContext->indexRecord=0;
       pEngineContext->currentRecord=1;
     }

   // For ground-based measurements, allocate a buffer for the indexes of selected reference spectra (automatic reference selection mode)

   if (resetFlag && (THRD_id==THREAD_TYPE_ANALYSIS) && pEngineContext->analysisRef.refAuto && !pEngineContext->satelliteFlag && pEngineContext->recordNumber) {
     if (   (pEngineContext->mfcDoasisFlag && (MFC_AllocFiles(pEngineContext)!=ERROR_ID_NO)) ||
            !(recordNumber=(pEngineContext->mfcDoasisFlag)?pEngineContext->recordInfo.mfcDoasis.nFiles:pEngineContext->recordNumber) ||
            ((pRef->refIndexes=(int *)MEMORY_AllocBuffer("EngineRequestBeginBrowseSpectra","refIndexes",recordNumber,sizeof(int),0,MEMORY_TYPE_INT))==NULL) ||
            ((pRef->zmList=(double *)MEMORY_AllocDVector("EngineRequestBeginBrowseSpectra","zmList",0,recordNumber-1))==NULL) ||
            ((pRef->timeDec=(double *)MEMORY_AllocDVector("EngineRequestBeginBrowseSpectra","timeDec",0,recordNumber-1))==NULL)) {

       rc=ERROR_ID_ALLOC;

     } else {
       pRef->nRef=0;
       pRef->zmMinIndex=
         pRef->indexScanBefore=
         pRef->indexScanAfter=ITEM_NONE;
     }
   }

   // retain calibration plot in case it is already there (e.g. for OMI)
   if (ANALYSE_plotKurucz)
     mediateResponseRetainPage(plotPageCalib,responseHandle);

   // in browsing mode, do initialisation:
   // (otherwise this is done in mediateRequestSetAnalysisWindows
   if (THRD_id == THREAD_TYPE_SPECTRA || THRD_id == THREAD_TYPE_EXPORT) {
     switch(pEngineContext->project.instrumental.readOutFormat) {
     case PRJCT_INSTR_FORMAT_APEX:
       rc = apex_init(pEngineContext->fileInfo.fileName,pEngineContext);
       break;
     default:
       break;
     }
   }

   return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineRequestEndBrowseSpectra
// -----------------------------------------------------------------------------
// PURPOSE       End the spectra browsing thread
//
// INPUT         pEngineContext     pointer to the engine context
//
// RETURN        0 in case of success; the code of the error otherwise
// -----------------------------------------------------------------------------

RC EngineRequestEndBrowseSpectra(ENGINE_CONTEXT *pEngineContext)
 {
   // Declarations

   ANALYSIS_REF *pRef;
   RC rc;

   // Initializations

   pRef=&pEngineContext->analysisRef;
   rc=ERROR_ID_NO;

   if ((THRD_id!=THREAD_TYPE_NONE) && (THRD_id!=THREAD_TYPE_SPECTRA))
    {
     rc=OUTPUT_FlushBuffers(pEngineContext);

     if ((!pEngineContext->mfcDoasisFlag || pEngineContext->recordInfo.mfcDoasis.resetFlag) && pEngineContext->analysisRef.refAuto && !pEngineContext->satelliteFlag)
      {
       // Release buffers used for automatic reference

       if (pRef->refIndexes!=NULL)
        MEMORY_ReleaseBuffer("EngineRequestEndBrowseSpectra","refIndexes",pRef->refIndexes);
       pRef->refIndexes=NULL;

       if (pRef->zmList!=NULL)
        MEMORY_ReleaseDVector("EngineRequestEndBrowseSpectra","zmList",pRef->zmList,0);
       pRef->zmList=NULL;

       if (pRef->timeDec!=NULL)
        MEMORY_ReleaseDVector("EngineRequestEndBrowseSpectra","timeDec",pRef->timeDec,0);
       pRef->timeDec=NULL;
      }
    }

   // Close the files

   EngineCloseFile(&pEngineContext->fileInfo);

   // Retrun

   return rc;
 }

// =======
// SESSION
// =======

// -----------------------------------------------------------------------------
// FUNCTION      EngineEndCurrentSession
// -----------------------------------------------------------------------------
// PURPOSE       Release buffers allocated by the current session
//
// INPUT         pEngineContext     pointer to the engine context
// -----------------------------------------------------------------------------

RC EngineEndCurrentSession(ENGINE_CONTEXT *pEngineContext)
 {
   RC rc;

   rc=ERROR_ID_NO;

   if (THRD_id!=THREAD_TYPE_NONE)
    {
     // Flush buffers

     rc=EngineRequestEndBrowseSpectra(pEngineContext);

     // Release other allocated buffers

     GDP_ASC_ReleaseBuffers();
     GDP_BIN_ReleaseBuffers();
     GOME2_ReleaseBuffers();
     OMI_ReleaseBuffers();
     tropomi_cleanup();
     SCIA_ReleaseBuffers(pEngineContext->project.instrumental.readOutFormat);
     apex_clean();

     if ((THRD_id!=THREAD_TYPE_NONE) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_EXPORT))
      {
       if (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI)
        OMI_ReleaseReference();

       ANALYSE_ResetData();
      }

     if (THRD_id==THREAD_TYPE_EXPORT)
      OUTPUT_ResetData();

     // Reset the context of the engine

     EngineResetContext(pEngineContext);
     EngineResetContext(&ENGINE_contextRef);
     EngineResetContext(&ENGINE_contextRef2);

     THRD_id=THREAD_TYPE_NONE;
     SYMB_itemCrossN=SYMBOL_PREDEFINED_MAX;
    }

   // Return

   return rc;
 }

// ===================================================
// CREATE/DESTROY ENGINE CONTEXT ON PROGRAM START/EXIT
// ===================================================

// -----------------------------------------------------------------------------
// FUNCTION      EngineCreateContext
// -----------------------------------------------------------------------------
// PURPOSE       Create a context for the engine
// -----------------------------------------------------------------------------

ENGINE_CONTEXT *EngineCreateContext(void)
 {
   // Declaration

   ENGINE_CONTEXT *pEngineContext=&engineContext;                                // pointer to the engine context

   // Initializations

   strcpy(ENGINE_dbgFile,"QDOAS.dbg");

   memset(pEngineContext,0,sizeof(ENGINE_CONTEXT));                              // main engine context
   memset(&ENGINE_contextRef,0,sizeof(ENGINE_CONTEXT));                          // copy of the engine context for the automatic search of the reference spectrum
   memset(&ENGINE_contextRef2,0,sizeof(ENGINE_CONTEXT));                         // copy of the engine context for the automatic search of the reference spectrum (scan mode)

   THRD_id=THREAD_TYPE_NONE;

   if (RESOURCE_Alloc()!=ERROR_ID_NO)
    pEngineContext=NULL;

   // Return

   return pEngineContext;
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineDestroyContext
// -----------------------------------------------------------------------------
// PURPOSE       Destroy the context of the current engine on program exit
//
//                -> release buffers allocated by EngineCreateContext
//                -> close open files and release buffers allocated by the current session
//
// INPUT         pEngineContext     pointer to the engine context
// -----------------------------------------------------------------------------

RC EngineDestroyContext(ENGINE_CONTEXT *pEngineContext)
 {
   RC rc;

   rc=EngineEndCurrentSession(pEngineContext);

   RESOURCE_Free();

   if (GOME2_beatLoaded)
    {
     coda_done();
     GOME2_beatLoaded=0;
    }

   return rc;
 }

// ==========================================
// AUTOMATIC SEARCH OF THE REFERENCE SPECTRUM
// ==========================================

// -----------------------------------------------------------------------------
// FUNCTION      EngineBuildRefList
// -----------------------------------------------------------------------------
// PURPOSE       Browse spectra and search for zenith sky spectra that can be used
//               as reference.  Keep SZA, record number and time of measurements.
//
// INPUT         pEngineContext     pointer to the engine context
//
// RETURN        0 in case of success; the code of the error otherwise
// -----------------------------------------------------------------------------

RC EngineBuildRefList(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  int *indexList;
  double *ZmList,*TimeDec;
  INDEX               indexRecord,                                              // browse spectra records in file
                      indexZmMin,indexZmMax;                                    // index of records with SZA minimum and maximum

  PRJCT_INSTRUMENTAL *pInstr;                                                   // pointer to the instrumental part of the project
  MFC_DOASIS         *pMfc;                                                     // MFC DOASIS format (binary and STD format)
  double              ZmMin,ZmMax;                                              // extrema zenith angles found in file
  int                 NRecord;                                                  // number of hold record
  int                 localCalDay;                                              // local day number
  int                 recordNumber;                                             // the current number of records
  RC                  rc;                                                       // return code

  // Make a backup of the buffer part of the engine context

  EngineCopyContext(&ENGINE_contextRef,pEngineContext);
  EngineCopyContext(&ENGINE_contextRef2,pEngineContext);

  ENGINE_contextRef.analysisRef.refScan=0;                                      // in order not to have error on zenith sky spectra

  // Initializations

  indexList=pEngineContext->analysisRef.refIndexes;
  ZmList=pEngineContext->analysisRef.zmList;
  TimeDec=pEngineContext->analysisRef.timeDec;

  localCalDay=pEngineContext->recordInfo.localCalDay;

  pInstr=&ENGINE_contextRef.project.instrumental;
  pMfc=&pEngineContext->recordInfo.mfcDoasis;

  recordNumber=(!pEngineContext->mfcDoasisFlag)?pEngineContext->recordNumber:pMfc->nFiles;

  indexZmMin=indexZmMax=ITEM_NONE;
  NRecord=0;
  ZmMin=360.;
  ZmMax=0.;

  rc=ERROR_ID_NO;

  if ((pInstr->readOutFormat==PRJCT_INSTR_FORMAT_LOGGER) ||
      (pInstr->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
      (pInstr->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD))
   {
   	ENGINE_contextRef.project.instrumental.user=
   	ENGINE_contextRef2.project.instrumental.user=PRJCT_INSTR_IASB_TYPE_ZENITHAL;
   }

  if (ENGINE_contextRef.recordNumber>0)
   {
    // Browse records in file

    for (indexRecord=ENGINE_contextRef.lastRefRecord+1;indexRecord<=recordNumber;indexRecord++)
     {
     	if (pEngineContext->mfcDoasisFlag)
       sprintf(ENGINE_contextRef.fileInfo.fileName,"%s%c%s",pMfc->filePath,PATH_SEP,&pMfc->fileNames[(indexRecord-1)*(DOAS_MAX_PATH_LEN+1)]);

      if (!(rc=EngineReadFile(&ENGINE_contextRef,(!pEngineContext->mfcDoasisFlag)?indexRecord:1,1,localCalDay)) && (ENGINE_contextRef.recordInfo.Zm>(double)0.) && (ENGINE_contextRef.recordInfo.Zm<(double)96.))
       {
        // Data on record

        indexList[NRecord]=indexRecord;                                         // index of record
        ZmList[NRecord]=ENGINE_contextRef.recordInfo.Zm;                        // zenith angle
        TimeDec[NRecord]=ENGINE_contextRef.recordInfo.localTimeDec;             // decimal time for determining when the measurement has occured

        // Minimum and maximum zenith angle

        if (ENGINE_contextRef.recordInfo.Zm<ZmMin)
         {
          ZmMin=ENGINE_contextRef.recordInfo.Zm;
          indexZmMin=NRecord;
         }

        if (ENGINE_contextRef.recordInfo.Zm>ZmMax)
         {
          ZmMax=ENGINE_contextRef.recordInfo.Zm;
          indexZmMax=NRecord;
         }

        NRecord++;
       }
      else if (rc==ERROR_ID_FILE_END)
       {
        rc=ERROR_ID_NO;
        break;
       }
     }

    if (!NRecord)
     rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"all the day",ENGINE_contextRef.fileInfo.fileName);
    else if (rc==ERROR_ID_FILE_RECORD)
     rc=ERROR_ID_NO;
   }

  pEngineContext->analysisRef.nRef=NRecord;
  pEngineContext->analysisRef.zmMinIndex=indexZmMin;
  pEngineContext->analysisRef.zmMaxIndex=indexZmMax;

// TO CHECK WITH MPI PEOPLE   {
// TO CHECK WITH MPI PEOPLE    FILE *fp;
// TO CHECK WITH MPI PEOPLE    int i;
// TO CHECK WITH MPI PEOPLE    fp=fopen("RefList.dat","w+t");
// TO CHECK WITH MPI PEOPLE    fprintf(fp,"List of references : \n");
// TO CHECK WITH MPI PEOPLE    for (i=0;i<NRecord;i++)
// TO CHECK WITH MPI PEOPLE     fprintf(fp,"%d %g %s\n",indexList[i],ZmList[i],&pEngineContext->recordInfo.mfcDoasis.fileNames[(indexList[i]-1)*(DOAS_MAX_PATH_LEN+1)]);
// TO CHECK WITH MPI PEOPLE
// TO CHECK WITH MPI PEOPLE    fclose(fp);
// TO CHECK WITH MPI PEOPLE   }


  // Copy information from the ref context to the main context

  pEngineContext->lastRefRecord=ENGINE_contextRef.lastRefRecord;

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineSZASetRefIndexes
// -----------------------------------------------------------------------------
// PURPOSE       Set indexes of records to use as reference in SZA mode
//
// INPUT         pEngineContext     pointer to the engine context
//
// RETURN        0 in case of success; the code of the error otherwise
// -----------------------------------------------------------------------------

RC EngineSZASetRefIndexes(ENGINE_CONTEXT *pEngineContext,FENO *pTabFeno)
 {
 	// Declarations

  MFC_DOASIS *pMfc;
 	ANALYSIS_REF *pRef;
 	double ZmMin,ZmMax;
 	double deltaZmMorning,deltaZmAfternoon;
 	INDEX indexZmMin,indexRecord;
 	int recordNumber;
 	RC rc;

 	// Initialization

  pMfc=&pEngineContext->recordInfo.mfcDoasis;
 	pRef=&pEngineContext->analysisRef;
 	ZmMin=pRef->zmList[pRef->zmMinIndex];
 	ZmMax=pRef->zmList[pRef->zmMaxIndex];
 	indexZmMin=pRef->zmMinIndex;

 	recordNumber=(!pEngineContext->mfcDoasisFlag)?pEngineContext->recordNumber:pMfc->nFiles;

 	rc=ERROR_ID_NO;

  if (!pTabFeno->hidden &&
      (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) &&
      (pTabFeno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SZA))
   {
    // Indexes reinitialization

    pTabFeno->indexRefMorning=pTabFeno->indexRefAfternoon=pTabFeno->indexRef=ITEM_NONE;

    // No reference spectrum found in SZA range

    if (ZmMax<pTabFeno->refSZA-pTabFeno->refSZADelta)
     rc=ERROR_SetLast("EngineSetRefIndexesMFC",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"all the day",pEngineContext->fileInfo.fileName);

    // Select record with SZA minimum

    else if (ZmMin>=pTabFeno->refSZA+pTabFeno->refSZADelta)
     pTabFeno->indexRefMorning=pTabFeno->indexRefAfternoon=indexZmMin;

    // Select a record for the morning and the afternoon

    else
     {
      deltaZmMorning=deltaZmAfternoon=360.;

      for (indexRecord=0;indexRecord<recordNumber;indexRecord++)
       {
        if ((pRef->zmList[indexRecord]>=pTabFeno->refSZA-pTabFeno->refSZADelta) &&
            (pRef->zmList[indexRecord]<=pTabFeno->refSZA+pTabFeno->refSZADelta))
         {
          if ((pRef->timeDec[indexRecord]<=ENGINE_localNoon) && (fabs(pTabFeno->refSZA-pRef->zmList[indexRecord])<deltaZmMorning))
           {
            pTabFeno->indexRefMorning=indexRecord;
            deltaZmMorning=fabs(pTabFeno->refSZA-pRef->zmList[indexRecord]);
           }

          if ((pRef->timeDec[indexRecord]>ENGINE_localNoon) && (fabs(pTabFeno->refSZA-pRef->zmList[indexRecord])<deltaZmAfternoon))
           {
            pTabFeno->indexRefAfternoon=indexRecord;
            deltaZmAfternoon=fabs(pTabFeno->refSZA-pRef->zmList[indexRecord]);
           }
         }
       }

      // No record found for the morning OR the afternoon

      if ((pTabFeno->indexRefMorning==ITEM_NONE) && (pTabFeno->indexRefAfternoon==ITEM_NONE))
       rc=ERROR_SetLast("EngineSetRefIndexesMFC",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"all the day",pEngineContext->fileInfo.fileName);
      else
       {
        if (pTabFeno->indexRefMorning==ITEM_NONE)
         {
          rc=ERROR_SetLast("EngineSetRefIndexesMFC",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"the morning",pEngineContext->fileInfo.fileName);
          pTabFeno->indexRefMorning=pTabFeno->indexRefAfternoon;
         }
        else if (pTabFeno->indexRefAfternoon==ITEM_NONE)
         {
          rc=ERROR_SetLast("EngineSetRefIndexesMFC",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"the afternoon",ENGINE_contextRef.fileInfo.fileName);
          pTabFeno->indexRefAfternoon=pTabFeno->indexRefMorning;
         }
       }
     }

    pTabFeno->ZmRefMorning=(pTabFeno->indexRefMorning!=ITEM_NONE)?pRef->zmList[pTabFeno->indexRefMorning]:(double)-1.;
    pTabFeno->ZmRefAfternoon=(pTabFeno->indexRefAfternoon!=ITEM_NONE)?pRef->zmList[pTabFeno->indexRefAfternoon]:(double)-1.;

    if (pTabFeno->indexRefMorning!=ITEM_NONE)
     pTabFeno->indexRefMorning=pRef->refIndexes[pTabFeno->indexRefMorning];
    if (pTabFeno->indexRefAfternoon!=ITEM_NONE)
     pTabFeno->indexRefAfternoon=pRef->refIndexes[pTabFeno->indexRefAfternoon];
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineScanSetRefIndexes
// -----------------------------------------------------------------------------
// PURPOSE       Set indexes of records to use as reference in scan mode
//
// INPUT         pEngineContext     pointer to the engine context
//
// RETURN        0 in case of success; the code of the error otherwise
// -----------------------------------------------------------------------------

void EngineScanSetRefIndexes(ENGINE_CONTEXT *pEngineContext,INDEX indexRecord)
 {
 	// Declarations

 	ANALYSIS_REF *pRef;
 	int *refIndexes,nRef;
 	int indexScanBefore,indexScanAfter,indexScanMin,indexScanMax,indexScanRecord;

 	// Initializations

 	pRef=&pEngineContext->analysisRef;
 	refIndexes=pRef->refIndexes;
 	nRef=pRef->nRef;

 	indexScanBefore=indexScanAfter=ITEM_NONE;

  // Dichotomic search of the reference spectrum

  if (indexRecord<refIndexes[0])
   indexScanAfter=refIndexes[0];
  else if (indexRecord>refIndexes[nRef-1])
   indexScanBefore=refIndexes[nRef-1];
  else
   {
    indexScanMin=indexScanRecord=0;
    indexScanMax=nRef;

    // Dichotomic search

    while (indexScanMax-indexScanMin>1)
     {
      indexScanRecord=(indexScanMin+indexScanMax)>>1;

      if (refIndexes[indexScanRecord]==indexRecord)
       indexScanMin=indexScanMax=ITEM_NONE;                                  // the current record is also a zenith (-> in the list of possible reference spectra)
      else if (refIndexes[indexScanRecord]<indexRecord)
       indexScanMin=indexScanRecord;
      else
       indexScanMax=indexScanRecord;
     }

    if ((indexScanMin!=ITEM_NONE) && (indexScanMin<nRef) && (refIndexes[indexScanMin]<=indexRecord))
     indexScanBefore=refIndexes[indexScanMin];

    if ((indexScanMax!=ITEM_NONE) && (indexScanMax<nRef) && (refIndexes[indexScanMax]>=indexRecord))
     indexScanAfter=refIndexes[indexScanMax];
   }

  pRef->indexScanBefore=indexScanBefore;
  pRef->indexScanAfter=indexScanAfter;
 }

RC EngineLoadRefMFC(ENGINE_CONTEXT *pEngineContextRef,ENGINE_CONTEXT *pEngineContext,INDEX indexRefRecord)
 {
 	// Declarations

  RC rc;

 	// Initialization

 	rc=ERROR_ID_NO;

 	if ((indexRefRecord!=ITEM_NONE) && (indexRefRecord<=pEngineContext->recordInfo.mfcDoasis.nFiles))
 	 {
    // Build reference file name

    sprintf(pEngineContextRef->fileInfo.fileName,"%s%c%s",pEngineContext->recordInfo.mfcDoasis.filePath,PATH_SEP,&pEngineContext->recordInfo.mfcDoasis.fileNames[(indexRefRecord-1)*(DOAS_MAX_PATH_LEN+1)]);

    // Load file

    rc=EngineReadFile(pEngineContextRef,1,0,0);
   }
  else
   rc=ERROR_ID_NO_REF;

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineNewRef
// -----------------------------------------------------------------------------
// PURPOSE       Load a new reference spectrum
//
// INPUT         pEngineContext     pointer to the engine context
//
// RETURN        0 in case of success; the code of the error otherwise
// -----------------------------------------------------------------------------

RC EngineNewRef(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
 {
   // Declarations

   RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
   ANALYSIS_REF *pRef;

   INDEX indexRefRecord,                                                         // index of best record in file for reference selection
     indexTabFeno,                                                           // browse analysis windows
     indexWindow,                                                            // avoid gaps
     indexPage,
     indexColumn,
     indexRecord,
     indexScanRecord,
     indexScanBefore,indexScanAfter;

   FENO *pTabFeno;                                                               // pointer to the analysis window
   int useKurucz,alignRef,useUsamp,saveFlag,newDimL;
   int newref;
   RC rc;

   // Initializations

   pRecord=&pEngineContext->recordInfo;
   pRef=&pEngineContext->analysisRef;
   rc=ERROR_ID_NO;
   saveFlag=(int)pEngineContext->project.spectra.displayDataFlag;
   useKurucz=alignRef=useUsamp=0;
   indexRecord=indexRefRecord=indexScanRecord=indexScanBefore=indexScanAfter=ITEM_NONE;
   indexColumn=2;
   newref=0;

   // Build a list of reference spectra

   if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_ASCII) && !pEngineContext->project.instrumental.ascii.szaSaveFlag)
    rc=ERROR_SetLast("EngineNewRef",ERROR_TYPE_WARNING,ERROR_ID_FILE_AUTOMATIC);

   // ENGINE_refStartDate : useful when records in the file cover two days in local time (due to the time shift).  In this case, compare starting date and time

   else if ((( ENGINE_refStartDate && (memcmp(&pEngineContext->fileInfo.startDate,&ENGINE_contextRef.fileInfo.startDate,sizeof(SHORT_DATE)) || memcmp(&pEngineContext->fileInfo.startTime,&ENGINE_contextRef.fileInfo.startTime,sizeof(struct time)))) ||
             (!ENGINE_refStartDate && (pRecord->localCalDay!=ENGINE_contextRef.recordInfo.localCalDay)) ||
             (pEngineContext->mfcDoasisFlag && pEngineContext->recordInfo.mfcDoasis.resetFlag)) &&
            !(rc=EngineBuildRefList(pEngineContext)))

    newref=1;  // Load a new set of reference files

   else if (rc!=ERROR_ID_NO)
    goto EndNewRef;

   // Get the current record

   if (pEngineContext->mfcDoasisFlag)
    indexRecord=MFC_SearchForCurrentFileIndex(pEngineContext);
   else
   	indexRecord=pEngineContext->indexRecord;

   // For scan mode, determine the indexes of the zenith records before and after the current record

   if (pEngineContext->analysisRef.refScan)
    {
    	if (newref || ((pRef->indexScanBefore==ITEM_NONE) && (pRef->indexScanAfter==ITEM_NONE)) || (indexRecord<=pRef->indexScanBefore) || ((pRef->indexScanAfter!=ITEM_NONE) && (indexRecord>=pRef->indexScanAfter)))
    	 EngineScanSetRefIndexes(pEngineContext,indexRecord);

  	  indexScanBefore=pRef->indexScanBefore;
  	  indexScanAfter=pRef->indexScanAfter;
    }

   // Browse analysis windows

   for (indexTabFeno=0;(indexTabFeno<NFeno) && !rc;indexTabFeno++)
    {
     pTabFeno=&TabFeno[0][indexTabFeno];
     indexPage=indexTabFeno+plotPageAnalysis;
     pTabFeno->newrefFlag=0;

     if (!(pTabFeno->hidden) &&                                                     // not the definition of the window for the wavelength calibration
         (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC))  // automatic reference selection only
      {
       pTabFeno->displayRef=0;

       indexRefRecord=ITEM_NONE;

       // Reference spectrum selected on the SZA

       if (pTabFeno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SZA)
        {
        	if (newref && ((rc=EngineSZASetRefIndexes(pEngineContext,pTabFeno))!=ERROR_ID_NO))
        	 break;
        	else
          indexRefRecord=(pRecord->localTimeDec<=ENGINE_localNoon)?pTabFeno->indexRefMorning:pTabFeno->indexRefAfternoon;
        }
       else if (pTabFeno->refSpectrumSelectionScanMode==ANLYS_MAXDOAS_REF_SCAN_BEFORE)
        indexRefRecord=(indexScanBefore!=ITEM_NONE)?indexScanBefore:indexScanAfter;
       else if (pTabFeno->refSpectrumSelectionScanMode==ANLYS_MAXDOAS_REF_SCAN_AFTER)
        indexRefRecord=(indexScanAfter!=ITEM_NONE)?indexScanAfter:indexScanBefore;

       // ANLYS_MAXDOAS_REF_SCAN_AVERAGE && ANLYS_MAXDOAS_REF_SCAN_INTERPOLATE
       // Should load both reference spectra (zenith before and after the scan) but one is missing

       else if (indexScanBefore==ITEM_NONE)
        indexRefRecord=indexScanAfter;
       else if (indexScanAfter==ITEM_NONE)
        indexRefRecord=indexScanBefore;

       // There is no reference spectrum for the requested twilight or scan -> exit

       if ((indexRefRecord==ITEM_NONE) && (indexScanBefore==ITEM_NONE) && (indexScanAfter==ITEM_NONE))
        {
         memcpy(&ENGINE_contextRef.recordInfo.present_datetime.thedate,&pRecord->present_datetime.thedate,sizeof(pRecord->present_datetime.thedate));
         pTabFeno->rc=-1;
//         break;
        }

       // There is a reference spectrum for the requested twilight or scan -> read it

       else if  (newref ||
               ((indexRefRecord!=ITEM_NONE) && (indexRefRecord!=pTabFeno->indexRef)) ||
               ((indexRefRecord==ITEM_NONE) && ((pTabFeno->refSpectrumSelectionScanMode==ANLYS_MAXDOAS_REF_SCAN_INTERPOLATE) || (indexScanBefore!=pTabFeno->indexRefScanBefore) || (indexScanAfter!=pTabFeno->indexRefScanAfter))))
        {
        	pTabFeno->newrefFlag=1;

         if (!pEngineContext->mfcDoasisFlag)
          {
          	if (indexRefRecord!=ITEM_NONE)
            rc=EngineReadFile(&ENGINE_contextRef,indexRefRecord,0,0);
           else if (!(rc=EngineReadFile(&ENGINE_contextRef,indexScanBefore,0,0)))       // in average mode, it is better to reload reference spectra because ref contexts could have changed in previous analysis windows
            rc=EngineReadFile(&ENGINE_contextRef2,indexScanAfter,0,0);
          }
         else
          {
          	if (indexRefRecord!=ITEM_NONE)
          	 rc=EngineLoadRefMFC(&ENGINE_contextRef,pEngineContext,indexRefRecord);
          	else if (!(rc=EngineLoadRefMFC(&ENGINE_contextRef,pEngineContext,indexScanBefore)))
          	 rc=EngineLoadRefMFC(&ENGINE_contextRef2,pEngineContext,indexScanAfter);
          }

         if (!rc)
          {
           alignRef++;

           if (indexRefRecord!=ITEM_NONE)
            memcpy(pTabFeno->Sref,ENGINE_contextRef.buffers.spectrum,sizeof(double)*pTabFeno->NDET);
           else
            {
            	double *sref1,*sref2;
            	double tint1,tint2;

            	sref1=ENGINE_contextRef.buffers.spectrum;
            	sref2=ENGINE_contextRef2.buffers.spectrum;

            	tint1=(fabs(ENGINE_contextRef.recordInfo.Tint)<(double)EPSILON)?(double)1.:ENGINE_contextRef.recordInfo.Tint;     // to avoid division by zero if the integration time is not defined
            	tint2=(fabs(ENGINE_contextRef2.recordInfo.Tint)<(double)EPSILON)?(double)1.:ENGINE_contextRef2.recordInfo.Tint;   // to avoid division by zero if the integration time is not defined

            	// normalize by the integration time first in order to have the same absorption structures

            	if (pTabFeno->refSpectrumSelectionScanMode==ANLYS_MAXDOAS_REF_SCAN_AVERAGE)
           	 	for (int i=0;i<pTabFeno->NDET;i++)
           	 	 pTabFeno->Sref[i]=(sref1[i]/tint1+sref2[i]/tint2)*0.5;
           	 else if (pTabFeno->refSpectrumSelectionScanMode==ANLYS_MAXDOAS_REF_SCAN_INTERPOLATE)
           	  {
           	 	 for (int i=0;i<pTabFeno->NDET;i++)
           	 	  pTabFeno->Sref[i]=sref1[i]/tint1+(pEngineContext->recordInfo.Tm-ENGINE_contextRef.recordInfo.Tm)*(sref2[i]/tint2-sref1[i]/tint1)/(ENGINE_contextRef2.recordInfo.Tm-ENGINE_contextRef.recordInfo.Tm);
           	  }
            }

           if (!pTabFeno->useEtalon)
            {
             memcpy(pTabFeno->LambdaK,ENGINE_contextRef.buffers.lambda,sizeof(double)*pTabFeno->NDET);
             memcpy(pTabFeno->LambdaRef,ENGINE_contextRef.buffers.lambda,sizeof(double)*pTabFeno->NDET);

             doas_spectrum *new_range = spectrum_new();
             for (indexWindow = 0, newDimL=0; indexWindow < pTabFeno->svd.Z; indexWindow++)
              {
               int pixel_start = FNPixel(ENGINE_contextRef.buffers.lambda,pTabFeno->svd.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
               int pixel_end = FNPixel(ENGINE_contextRef.buffers.lambda,pTabFeno->svd.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);
               spectrum_append(new_range, pixel_start, pixel_end);

               newDimL += pixel_end - pixel_start +1;
              }

             if (newDimL != pTabFeno->svd.DimL)
              { // reallocate complete SVD structure.
               SVD_Free("EngineNewRef",&pTabFeno->svd);
               pTabFeno->svd.DimL=newDimL;
               SVD_LocalAlloc("EngineNewRef",&pTabFeno->svd);
              }
             else if(pTabFeno->svd.specrange != NULL) // only update specrange
              spectrum_destroy(pTabFeno->svd.specrange);

             pTabFeno->svd.specrange = new_range;

             if (pTabFeno->useKurucz)
              {
               KURUCZ_Init(0,0);  // Currently doesn't account for OMI swaths !!!
               useKurucz++;
              }
             else if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,0))!=ERROR_ID_NO) ||
                      ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,ANALYSIS_slitMatrix,ANALYSIS_slitParam,pSlitOptions->slitFunction.slitType,0,pSlitOptions->slitFunction.slitWveDptFlag))!=ERROR_ID_NO))
              break;

            }

           if (pTabFeno->useUsamp)
            useUsamp++;


           if ((rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&pTabFeno->refNormFact,"EngineNewRef"))!=ERROR_ID_NO)
            break;

           pTabFeno->indexRef=indexRefRecord;
           pTabFeno->Zm=ENGINE_contextRef.recordInfo.Zm;
           pTabFeno->Tm=ENGINE_contextRef.recordInfo.Tm;
           pTabFeno->TimeDec=ENGINE_contextRef.recordInfo.TimeDec;

           if (indexRefRecord==ITEM_NONE)
            {
             pTabFeno->indexRefScanBefore=indexScanBefore;
             pTabFeno->indexRefScanAfter=indexScanAfter;

             pTabFeno->Zm2=ENGINE_contextRef2.recordInfo.Zm;
             pTabFeno->Tm2=ENGINE_contextRef2.recordInfo.Tm;
             pTabFeno->TimeDec2=ENGINE_contextRef2.recordInfo.TimeDec;
            }
           else
            pTabFeno->indexRefScanBefore=pTabFeno->indexRefScanAfter=ITEM_NONE;

           pTabFeno->displayRef=1;

           pTabFeno->refDate = ENGINE_contextRef.recordInfo.present_datetime.thedate;
          }
         else if (((indexRefRecord!=ITEM_NONE) && (indexRefRecord==pTabFeno->indexRef)) ||
                  ((indexScanBefore!=ITEM_NONE) && (indexScanBefore==pTabFeno->indexRefScanBefore)) ||
                  ((indexScanAfter!=ITEM_NONE) && (indexScanAfter==pTabFeno->indexRefScanAfter)))

          pTabFeno->displayRef=1;
        }

       if (pEngineContext->project.spectra.displayFitFlag && ((indexRefRecord!=ITEM_NONE) || (indexScanBefore!=ITEM_NONE) || (indexScanAfter!=ITEM_NONE)))
        {
         struct date *pDay;                                                      // pointer to measurement date
         struct time *pTime;                                                     // pointer to measurement date
         char string[80],tabTitle[80];
         plot_data_t spectrumData;
         int SvdPDeb,SvdPFin;
         int indexLine;

         SvdPDeb=spectrum_start(pTabFeno->svd.specrange);
         SvdPFin=spectrum_end(pTabFeno->svd.specrange);
         pDay=&ENGINE_contextRef.recordInfo.present_datetime.thedate;
         pTime=&ENGINE_contextRef.recordInfo.present_datetime.thetime;

         if (pTabFeno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SZA)
          sprintf(string,"Selected ref (%d, SZA %.2f)",pTabFeno->indexRef,ENGINE_contextRef.recordInfo.Zm);
         else if (pTabFeno->indexRef!=ITEM_NONE)
          sprintf(string,"Selected ref (%d)",pTabFeno->indexRef);
         else if ((pTabFeno->indexRefScanBefore!=ITEM_NONE) && (pTabFeno->indexRefScanAfter!=ITEM_NONE))
          sprintf(string,"%s ref (%d & %d)",(pTabFeno->refSpectrumSelectionScanMode==ANLYS_MAXDOAS_REF_SCAN_INTERPOLATE)?"Interpolated":"Averaged",pTabFeno->indexRefScanBefore,pTabFeno->indexRefScanAfter);

         sprintf(tabTitle,"%s results (%d/%d)",pTabFeno->windowName,pEngineContext->indexRecord,pEngineContext->recordNumber);

         mediateAllocateAndSetPlotData(&spectrumData,"Measured",&pTabFeno->LambdaRef[SvdPDeb],&pTabFeno->Sref[SvdPDeb],SvdPFin-SvdPDeb+1,Line);
         mediateResponsePlotData(indexPage,&spectrumData,1,Spectrum,forceAutoScale,string,"Wavelength (nm)","Intensity", responseHandle);
         mediateResponseLabelPage(indexPage, pEngineContext->fileInfo.fileName, tabTitle, responseHandle);
         mediateReleasePlotData(&spectrumData);

         if (pEngineContext->project.spectra.displayDataFlag)
          {
           pTabFeno->displayLineIndex=mediateRequestDisplaySpecInfo(pEngineContext,indexPage,responseHandle);
           indexLine=pTabFeno->displayLineIndex+1;

           mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Selected reference for window","%s",pTabFeno->windowName);

           if (pEngineContext->mfcDoasisFlag)
            mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Reference file","%s",(pTabFeno->indexRef!=ITEM_NONE)?&pEngineContext->recordInfo.mfcDoasis.fileNames[(pTabFeno->indexRef-1)*(DOAS_MAX_PATH_LEN+1)]:&pEngineContext->recordInfo.mfcDoasis.fileNames[(pTabFeno->indexRefScanBefore-1)*(DOAS_MAX_PATH_LEN+1)]);
           else
            mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Record number","%d/%d",(pTabFeno->indexRef!=ITEM_NONE)?pTabFeno->indexRef:pTabFeno->indexRefScanBefore,ENGINE_contextRef.recordNumber);

           mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Date and Time","%02d/%02d/%d %02d:%02d:%02d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
           mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"SZA","%g",ENGINE_contextRef.recordInfo.Zm);

           if ((pTabFeno->indexRefScanBefore!=ITEM_NONE) && (pTabFeno->indexRefScanAfter!=ITEM_NONE))
            {
            	pDay=&ENGINE_contextRef2.recordInfo.present_datetime.thedate;

             if (pEngineContext->mfcDoasisFlag)
              mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Reference file (2)","%s",&pEngineContext->recordInfo.mfcDoasis.fileNames[(pTabFeno->indexRefScanAfter-1)*(DOAS_MAX_PATH_LEN+1)]);
             else
              mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Record number (2)","%d/%d",pTabFeno->indexRefScanAfter,ENGINE_contextRef2.recordNumber);

             mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Date and Time (2)","%02d/%02d/%d %02d:%02d:%02d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
             mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"SZA (2)","%g",ENGINE_contextRef2.recordInfo.Zm);
            }

           pTabFeno->displayLineIndex=indexLine+1;
          }

        }
      }
    }

   // Reference alignment

   if (!rc && useKurucz)
    rc=KURUCZ_Reference(ENGINE_contextRef.buffers.instrFunction,1,saveFlag,1,responseHandle,0);

   if (!rc && alignRef)
    rc=ANALYSE_AlignReference(pEngineContext,1,responseHandle,0);

   if (!rc && useUsamp)
    rc=ANALYSE_UsampBuild(1,ITEM_NONE);

   EndNewRef :

   // Return

   return rc;
 }


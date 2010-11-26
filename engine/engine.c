

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

#include "mediate.h"
#include "engine.h"

INT  NDET;                                                                      // detector size

ENGINE_CONTEXT engineContext,                                                   // engine context used to make the interface between the mediator and the engine
               ENGINE_contextRef;                                               // engine context used for the automatic search of the reference spectrum
DoasCh ENGINE_dbgFile[MAX_PATH_LEN+1];
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
  if (pBuffers->irrad!=NULL)
   MEMORY_ReleaseDVector("EngineResetContext ","irrad",pBuffers->irrad,0);
  if (pBuffers->darkCurrent!=NULL)
   MEMORY_ReleaseDVector("EngineResetContext ","darkCurrent",pBuffers->darkCurrent,0);
  if (pBuffers->scanRef!=NULL)
   MEMORY_ReleaseDVector("EngineResetContext ","scanRef",pBuffers->scanRef,0);
  if (pBuffers->specMaxx!=NULL)
   MEMORY_ReleaseDVector("EngineResetContext ","specMaxx",pBuffers->specMaxx,0);
  if (pBuffers->specMax!=NULL)
   MEMORY_ReleaseDVector("EngineResetContext ","specMax",pBuffers->specMax,0);
  if (pEngineContext->analysisRef.scanRefIndexes!=NULL)
   MEMORY_ReleaseBuffer("EngineResetContext ","scanRefIndexes",pEngineContext->analysisRef.scanRefIndexes);
  if (pEngineContext->analysisRef.scanRefFiles!=NULL)
   MEMORY_ReleaseBuffer("EngineResetContext ","scanRefFiles",pEngineContext->analysisRef.scanRefFiles);
  if (pBuffers->varPix!=NULL)
   MEMORY_ReleaseDVector("EngineResetContext ","varPix",pBuffers->varPix,0);
  if (pBuffers->dnl!=NULL)
   MEMORY_ReleaseDVector("EngineResetContext ","dnl",pBuffers->dnl,0);
  if (pBuffers->recordIndexes!=NULL)
   MEMORY_ReleaseBuffer("EngineResetContext ","recordIndexes",pBuffers->recordIndexes);

  CCD_ResetInstrumental(&pRecord->ccd);

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

  // Buffers allocation

  if (((pBuffersSource->lambda!=NULL) && (pBuffersTarget->lambda==NULL) &&
      ((pBuffersTarget->lambda=(double *)MEMORY_AllocDVector("EngineCopyContext","lambda",0,NDET-1))==NULL)) ||
      ((pBuffersSource->instrFunction!=NULL) && (pBuffersTarget->instrFunction==NULL) &&
      ((pBuffersTarget->instrFunction=(double *)MEMORY_AllocDVector("EngineCopyContext","instrFunction",0,NDET-1))==NULL)) ||
      ((pBuffersSource->spectrum!=NULL) && (pBuffersTarget->spectrum==NULL) &&
      ((pBuffersTarget->spectrum=(double *)MEMORY_AllocDVector("EngineCopyContext","spectrum",0,NDET-1))==NULL)) ||
      ((pBuffersSource->sigmaSpec!=NULL) && (pBuffersTarget->sigmaSpec==NULL) &&
      ((pBuffersTarget->sigmaSpec=(double *)MEMORY_AllocDVector("EngineCopyContext","sigmaSpec",0,NDET-1))==NULL)) ||
      ((pBuffersSource->irrad!=NULL) && (pBuffersTarget->irrad==NULL) &&
      ((pBuffersTarget->irrad=(double *)MEMORY_AllocDVector("EngineCopyContext","irrad",0,NDET-1))==NULL)) ||
      ((pBuffersSource->darkCurrent!=NULL) && (pBuffersTarget->darkCurrent==NULL) &&
      ((pBuffersTarget->darkCurrent=(double *)MEMORY_AllocDVector("EngineCopyContext","darkCurrent",0,NDET-1))==NULL)) ||
      ((pBuffersSource->scanRef!=NULL) && (pBuffersTarget->scanRef==NULL) &&
      ((pBuffersTarget->scanRef=(double *)MEMORY_AllocDVector("EngineCopyContext","scanRef",0,NDET-1))==NULL)) ||
      ((pBuffersSource->varPix!=NULL) && (pBuffersTarget->varPix==NULL) &&
      ((pBuffersTarget->varPix=(double *)MEMORY_AllocDVector("EngineCopyContext","varPix",0,NDET-1))==NULL)) ||
      ((pBuffersSource->dnl!=NULL) && (pBuffersTarget->dnl==NULL) &&
      ((pBuffersTarget->dnl=(double *)MEMORY_AllocDVector("EngineCopyContext","dnl",0,NDET-1))==NULL)) ||
      ((pBuffersSource->specMaxx!=NULL) && (pBuffersTarget->specMaxx==NULL) &&
      ((pBuffersTarget->specMaxx=(double *)MEMORY_AllocDVector("EngineCopyContext","specMaxx",0,NDET-1))==NULL)) ||
      ((pBuffersSource->specMax!=NULL) && (pBuffersTarget->specMax==NULL) &&
      ((pBuffersTarget->specMax=(double *)MEMORY_AllocDVector("EngineCopyContext","specMax",0,NDET-1))==NULL)) ||
      ((pEngineContextSource->analysisRef.scanRefIndexes!=NULL) && (pEngineContextTarget->analysisRef.scanRefIndexes==NULL) &&
      ((pEngineContextTarget->analysisRef.scanRefIndexes=(INT *)MEMORY_AllocBuffer("EngineCopyContext","scanRefIndexes",pEngineContextSource->fileInfo.nScanRef,sizeof(INT),0,MEMORY_TYPE_INT))==NULL)) ||
      ((pBuffersSource->recordIndexes!=NULL) && (pBuffersTarget->recordIndexes==NULL) &&
      ((pBuffersTarget->recordIndexes=(DoasU32 *)MEMORY_AllocBuffer("THRD_CopySpecInfo","recordIndexes",
       (pEngineContextTarget->recordIndexesSize=pEngineContextSource->recordIndexesSize),sizeof(DoasU32),0,MEMORY_TYPE_ULONG))==NULL)) ||
      ((pEngineContextTarget->recordInfo.ccd.vip.matrix!=NULL) && (pEngineContextTarget->recordInfo.ccd.vip.matrix==NULL) &&
      ((rc=MATRIX_Copy(&pEngineContextTarget->recordInfo.ccd.vip,&pEngineContextSource->recordInfo.ccd.vip,"EngineCopyContext"))!=ERROR_ID_NO)) ||
      ((pEngineContextTarget->recordInfo.ccd.dnl.matrix!=NULL) && (pEngineContextTarget->recordInfo.ccd.dnl.matrix==NULL) &&
      ((rc=MATRIX_Copy(&pEngineContextTarget->recordInfo.ccd.dnl,&pEngineContextSource->recordInfo.ccd.dnl,"EngineCopyContext"))!=ERROR_ID_NO)) ||
      ((pEngineContextTarget->recordInfo.ccd.drk.matrix!=NULL) && (pEngineContextTarget->recordInfo.ccd.drk.matrix==NULL) &&
      ((rc=MATRIX_Copy(&pEngineContextTarget->recordInfo.ccd.drk,&pEngineContextSource->recordInfo.ccd.drk,"EngineCopyContext"))!=ERROR_ID_NO)))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Buffers

    if ((pBuffersTarget->lambda!=NULL) && (pBuffersSource->lambda!=NULL))
     memcpy(pBuffersTarget->lambda,pBuffersSource->lambda,sizeof(double)*NDET);
    if ((pBuffersTarget->instrFunction!=NULL) && (pBuffersSource->instrFunction!=NULL))
     memcpy(pBuffersTarget->instrFunction,pBuffersSource->instrFunction,sizeof(double)*NDET);
    if ((pBuffersTarget->spectrum!=NULL) && (pBuffersSource->spectrum!=NULL))
     memcpy(pBuffersTarget->spectrum,pBuffersSource->spectrum,sizeof(double)*NDET);
    if ((pBuffersTarget->sigmaSpec!=NULL) && (pBuffersSource->sigmaSpec!=NULL))
     memcpy(pBuffersTarget->sigmaSpec,pBuffersSource->sigmaSpec,sizeof(double)*NDET);
    if ((pBuffersTarget->irrad!=NULL) && (pBuffersSource->irrad!=NULL))
     memcpy(pBuffersTarget->irrad,pBuffersSource->irrad,sizeof(double)*NDET);
    if ((pBuffersTarget->darkCurrent!=NULL) && (pBuffersSource->darkCurrent!=NULL))
     memcpy(pBuffersTarget->darkCurrent,pBuffersSource->darkCurrent,sizeof(double)*NDET);
    if ((pBuffersTarget->scanRef!=NULL) && (pBuffersSource->scanRef!=NULL))
     memcpy(pBuffersTarget->scanRef,pBuffersSource->scanRef,sizeof(double)*NDET);
    if ((pBuffersTarget->varPix!=NULL) && (pBuffersSource->varPix!=NULL))
     memcpy(pBuffersTarget->varPix,pBuffersSource->varPix,sizeof(double)*NDET);
    if ((pBuffersTarget->dnl!=NULL) && (pBuffersSource->dnl!=NULL))
     memcpy(pBuffersTarget->dnl,pBuffersSource->dnl,sizeof(double)*NDET);
    if ((pBuffersTarget->specMaxx!=NULL) && (pBuffersSource->specMaxx!=NULL))
     memcpy(pBuffersTarget->specMaxx,pBuffersSource->specMaxx,sizeof(double)*NDET);
    if ((pBuffersTarget->specMax!=NULL) && (pBuffersSource->specMax!=NULL))
     memcpy(pBuffersTarget->specMax,pBuffersSource->specMax,sizeof(double)*NDET);
    if ((pEngineContextTarget->analysisRef.scanRefIndexes!=NULL) && (pEngineContextSource->analysisRef.scanRefIndexes!=NULL))
     memcpy(pEngineContextTarget->analysisRef.scanRefIndexes,pEngineContextSource->analysisRef.scanRefIndexes,sizeof(INT)*pEngineContextSource->fileInfo.nScanRef);
    if ((pBuffersTarget->recordIndexes!=NULL) && (pBuffersSource->recordIndexes!=NULL))
     memcpy(pBuffersTarget->recordIndexes,pBuffersSource->recordIndexes,sizeof(DoasU32)*pEngineContextSource->recordIndexesSize);

    // Other structures

    memcpy(&pEngineContextTarget->project,&pEngineContextSource->project,sizeof(PROJECT));              // project options
    memcpy(&pEngineContextTarget->fileInfo,&pEngineContextSource->fileInfo,sizeof(FILE_INFO));          // the name of the file to load and file pointers
    memcpy(&pEngineContextTarget->recordInfo,&pEngineContextSource->recordInfo,sizeof(RECORD_INFO)-sizeof(CCD));
    memcpy(&pEngineContextTarget->calibFeno,&pEngineContextSource->calibFeno,sizeof(CALIB_FENO));
    memcpy(&pEngineContextTarget->analysisRef,&pEngineContextSource->analysisRef,sizeof(ANALYSIS_REF));

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

 	double *lambdaInstr;                                                          // wavelength calibration of the instrument function
 	double *instrFunction;                                                        // instrumental function
 	double *instrDeriv2;                                                          // second derivative for the instrument function
 	RC rc;                                                                        // return code
 	FILE *fp;                                                                     // file pointer
 	DoasCh str[MAX_ITEM_TEXT_LEN+1];                                              // buffer to read the lines of the file
 	int i,indexSite;                                                              // index for loops and arrays

 	// Initializations

 	ANALYSE_plotKurucz=ANALYSE_plotRef=0;
  ANALYSE_indexLine=1;
  pBuffers=&pEngineContext->buffers;
  pProject=&pEngineContext->project;
  pInstrumental=&pProject->instrumental;

  pEngineContext->lastRefRecord=0;

  pEngineContext->satelliteFlag=((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
                                 (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
                                 (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
                                 (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI) ||
                                 (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GOME2))?1:0;

 	ENGINE_localNoon=(double)12.;

  THRD_localShift=(int)(((indexSite=SITES_GetIndex(pInstrumental->observationSite))!=ITEM_NONE)?
                          floor(SITES_itemList[indexSite].longitude/15.):0); // 24./360.

 	rc=ERROR_ID_NO;

  // Allocate buffers

  if (((pBuffers->lambda=MEMORY_AllocDVector("EngineSetProject","lambda",0,NDET-1))==NULL) ||
      ((pBuffers->spectrum=MEMORY_AllocDVector("EngineSetProject","spectrum",0,NDET-1))==NULL) ||

     (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_OHP_96) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_HA_94) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MKZY)) &&

      ((pBuffers->recordIndexes=(DoasU32 *)MEMORY_AllocBuffer("EngineSetProject","recordIndexes",2001,sizeof(DoasU32),0,MEMORY_TYPE_ULONG))==NULL)) ||

     (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&

     (((pBuffers->sigmaSpec=MEMORY_AllocDVector("EngineSetProject","sigmaSpec",0,NDET-1))==NULL) ||
      ((pBuffers->irrad=MEMORY_AllocDVector("EngineSetProject","irrad",0,NDET-1))==NULL))) ||

      ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MKZY) &&
      ((pBuffers->scanRef=MEMORY_AllocDVector("EngineSetProject","scanRef",0,NDET-1))==NULL)) ||

     (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_ACTON) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MKZY) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)) &&
      ((pBuffers->darkCurrent=MEMORY_AllocDVector("EngineSetProject","darkCurrent",0,NDET-1))==NULL)) ||

     (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_BIRA)) &&

      ((strlen(pInstrumental->vipFile) && ((pBuffers->varPix=MEMORY_AllocDVector("EngineSetProject","varPix",0,NDET-1))==NULL)) ||
       (strlen(pInstrumental->dnlFile) && ((pBuffers->dnl=MEMORY_AllocDVector("EngineSetProject","dnl",0,NDET-1))==NULL)))))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Load the wavelength calibration

    if (!strlen(pInstrumental->calibrationFile))
     for (i=0;i<NDET;i++)
      pBuffers->lambda[i]=i+1;
    else if ((fp=fopen(pInstrumental->calibrationFile,"rt"))==NULL)
     rc=ERROR_SetLast("EngineSetProject",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pInstrumental->calibrationFile);
    else
     {
      for (i=0;i<NDET;)
       if (!fgets(str,MAX_ITEM_TEXT_LEN,fp))
        break;
       else if ((strchr(str,';')==NULL) && (strchr(str,'*')==NULL))
        {
         sscanf(str,"%lf",&pBuffers->lambda[i]);
         i++;
        }

      if (i!=NDET)
       rc=ERROR_SetLast("EngineSetProject",ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,pInstrumental->calibrationFile);

      fclose(fp);
     }

    // Load the instrumental function                                           // QDOAS !!! LOAD vip + dnl

    if (strlen(pInstrumental->instrFunction))
     {
     	lambdaInstr=instrFunction=instrDeriv2=NULL;

      if ((fp=fopen(pInstrumental->instrFunction,"rt"))==NULL)
       rc=ERROR_SetLast("EngineSetProject",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pInstrumental->instrFunction);
      else if (((pBuffers->instrFunction=MEMORY_AllocDVector("EngineSetProject","instrFunction",0,NDET-1))==NULL) ||
               ((lambdaInstr=MEMORY_AllocDVector("EngineSetProject","lambdaInstr",0,NDET-1))==NULL) ||
               ((instrFunction=MEMORY_AllocDVector("EngineSetProject","instrFunction",0,NDET-1))==NULL) ||
               ((instrDeriv2=MEMORY_AllocDVector("EngineSetProject","instrDeriv2",0,NDET-1))==NULL))

       rc=ERROR_ID_ALLOC;

      else
       {
        for (i=0;(i<NDET) && fgets(str,MAX_ITEM_TEXT_LEN,fp);)
         if ((strchr(str,';')==NULL) && (strchr(str,'*')==NULL))
          {
           sscanf(str,"%lf %lf",&lambdaInstr[i],&instrFunction[i]);
           i++;
          }

        if (!SPLINE_Deriv2(lambdaInstr,instrFunction,instrDeriv2,NDET,"EngineSetProject"))
         rc=SPLINE_Vector(lambdaInstr,instrFunction,instrDeriv2,NDET,pBuffers->lambda,pBuffers->instrFunction,NDET,SPLINE_CUBIC,"EngineSetProject");
       }

      if (fp!=NULL)
       fclose(fp);

      // Release the allocated buffers

      if (lambdaInstr!=NULL)
       MEMORY_ReleaseDVector("EngineSetProject","lambdaInstr",lambdaInstr,0);
      if (instrFunction!=NULL)
       MEMORY_ReleaseDVector("EngineSetProject","instrFunction",instrFunction,0);
      if (instrDeriv2!=NULL)
       MEMORY_ReleaseDVector("EngineSetProject","instrDeriv2",instrDeriv2,0);
     }

    // MFC : load dark current and offset

    if (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) || (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)) &&
        !(rc=MFC_LoadOffset(pEngineContext)))

     rc=MFC_LoadDark(pEngineContext);

    // Initialize buffers

    if (pBuffers->darkCurrent!=NULL)
     VECTOR_Init(pBuffers->darkCurrent,(double)0.,NDET);                  // To check the initialization of the ANALYSE_zeros vector ...
    if (pBuffers->scanRef!=NULL)
     VECTOR_Init(pBuffers->scanRef,(double)0.,NDET);                      // To check the initialization of the ANALYSE_zeros vector ...
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
  DoasCh fileTmp[MAX_STR_LEN+1];
 	RC rc;

 	// Initializations

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
      (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS) &&
      (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GOME2) &&
     ((pFile->specFp=fopen(pEngineContext->fileInfo.fileName,"rb"))==NULL))

   rc=ERROR_SetLast("EngineSetFile",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pFile->fileName);

  else

   switch((INT)pEngineContext->project.instrumental.readOutFormat)
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
     case PRJCT_INSTR_FORMAT_BIRA_AIRBORNE :
      rc=AIRBORNE_Set(pEngineContext,pFile->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_MFC :
     case PRJCT_INSTR_FORMAT_MFC_STD :
      if (!(rc=SetMFC(pEngineContext,pFile->specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
       rc=MFC_LoadAnalysis(pEngineContext,responseHandle);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_MFC_BIRA :
      if (!(rc=MFCBIRA_Set(pEngineContext,pFile->specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
       rc=MFC_LoadAnalysis(pEngineContext,responseHandle);
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
     #if defined (__INCLUDE_HDF_) && __INCLUDE_HDF_
     case PRJCT_INSTR_FORMAT_OMI :
      rc=OMI_SetHDF(pEngineContext);
     break;
     #endif
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
      if (!(rc=GDP_BIN_Set(pEngineContext,pFile->specFp)) &&  (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
       rc=GDP_BIN_LoadAnalysis(pEngineContext,pEngineContext->fileInfo.specFp,responseHandle);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SCIA_PDS :
      if (!(rc=SCIA_SetPDS(pEngineContext)) &&  (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
       rc=SCIA_LoadAnalysis(pEngineContext,responseHandle);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_GOME2 :
      if (!(rc=GOME2_Set(pEngineContext)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
       rc=GOME2_LoadAnalysis(pEngineContext,responseHandle);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_MKZY :
      if (!(rc=MKZY_Set(pEngineContext,pFile->specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
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

RC EngineReadFile(ENGINE_CONTEXT *pEngineContext,int indexRecord,INT dateFlag,INT localCalDay)
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
  pRecord->Cic=0.;
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

  switch((INT)pEngineContext->project.instrumental.readOutFormat)
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
    case PRJCT_INSTR_FORMAT_BIRA_AIRBORNE :
     rc=AIRBORNE_Read(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
    break;
 // ---------------------------------------------------------------------------
// QDOAS ???    case PRJCT_INSTR_FORMAT_MFC :
// QDOAS ???
// QDOAS ???     switch(THRD_browseType)
// QDOAS ???      {
// QDOAS ???    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// QDOAS ???       case THREAD_BROWSE_MFC_OFFSET :
// QDOAS ???        mfcMask=pInstrumental->mfcMaskOffset;
// QDOAS ???       break;
// QDOAS ???    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// QDOAS ???       case THREAD_BROWSE_MFC_DARK :
// QDOAS ???        mfcMask=pInstrumental->mfcMaskDark;
// QDOAS ???       break;
// QDOAS ???    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// QDOAS ???       case THREAD_BROWSE_MFC_INSTR :
// QDOAS ???        mfcMask=pInstrumental->mfcMaskInstr;
// QDOAS ???       break;
// QDOAS ???    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// QDOAS ???       default :
// QDOAS ???        mfcMask=pInstrumental->mfcMaskSpec;
// QDOAS ???       break;
// QDOAS ???    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// QDOAS ???     }
// QDOAS ???
// QDOAS ???     rc=ReliMFC(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp,mfcMask);
// QDOAS ???
// QDOAS ???    break;
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
    #if defined (__INCLUDE_HDF_) && __INCLUDE_HDF_
    case PRJCT_INSTR_FORMAT_OMI :
     rc=OMI_ReadHDF(pEngineContext,indexRecord);
    break;
    #endif
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

  if (!rc && !(rc=THRD_SpectrumCorrection(pEngineContext,pEngineContext->buffers.spectrum)))
   {
   	pEngineContext->indexRecord=indexRecord;
   	if (pRecord->oldZm<(double)0.)
   	 pRecord->oldZm=pRecord->Zm;

   	// Correction of the solar zenith angle with the geolocation of the specified observation site

    if ((indexSite=SITES_GetIndex(pEngineContext->project.instrumental.observationSite))!=ITEM_NONE)
     {
      pSite=&SITES_itemList[indexSite];

      longit=-pSite->longitude;   // !!! sign is inverted

      pRecord->longitude=-longit;
      pRecord->latitude=latit=(double)pSite->latitude;

      if (pSite->altitude>(double)0.)
       pRecord->altitude=pSite->altitude*0.001;

      pRecord->Zm=(pRecord->Tm!=(double)0.)?ZEN_FNTdiz(ZEN_FNCrtjul(&pRecord->Tm),&longit,&latit,&pRecord->Azimuth):(double)-1.;
     }
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
 	// Declaration

 	RC rc;

  // Set file pointers

  if (!(rc=EngineRequestEndBrowseSpectra(pEngineContext)) &&
      !(rc=EngineSetFile(pEngineContext,spectraFileName,responseHandle)) &&
        pEngineContext->recordNumber &&
      ((THRD_id==THREAD_TYPE_SPECTRA) || !(rc=OUTPUT_LocalAlloc(pEngineContext))))
   {
    pEngineContext->indexRecord=0;
    pEngineContext->currentRecord=1;
   }

//  {
//  	FILE *fp;
//  	char *ptr;
//  	fp=fopen("qdoas.dbg","a+t");
//  	if ((ptr=strrchr(spectraFileName,'/'))!=NULL)
//  	 ptr=ptr+1;
//  	else
//  	 ptr=(char *)spectraFileName;
//
//  	fprintf(fp,"EngineRequestBeginBrowseSpectra %s (%d records,rc %d)\n",ptr,pEngineContext->recordNumber,rc);
//  	fclose(fp);
//  }


  // Return

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
 	// Declaration

 	RC rc;

 	rc=ERROR_ID_NO;

 	if ((THRD_id!=THREAD_TYPE_NONE) && (THRD_id!=THREAD_TYPE_SPECTRA))
 	 rc=OUTPUT_FlushBuffers(pEngineContext);

//  {
//  	FILE *fp;
//  	char *ptr;
//  	fp=fopen("qdoas.dbg","a+t");
//  	if ((ptr=strrchr(pEngineContext->fileInfo.fileName,'/'))!=NULL)
//  	 ptr=ptr+1;
//  	else
//  	 ptr=pEngineContext->fileInfo.fileName;
//
//  	fprintf(fp,"EngineRequestEndBrowseSpectra %s (rc %d)\n",ptr,rc);
//  	fclose(fp);
//  }

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

   	// Reset the context of the engine

   	EngineResetContext(pEngineContext);
   	EngineResetContext(&ENGINE_contextRef);

   	// Release other allocated buffers

    GDP_ASC_ReleaseBuffers();
    GDP_BIN_ReleaseBuffers();

    GOME2_ReleaseBuffers(GOME2_BEAT_CLOSE);
  //  OMI_ReleaseBuffers();

    SCIA_ReleaseBuffers(pEngineContext->project.instrumental.readOutFormat);

    if ((THRD_id!=THREAD_TYPE_NONE) && (THRD_id!=THREAD_TYPE_SPECTRA))
     ANALYSE_ResetData();

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

 	return rc;
 }

// ==========================================
// AUTOMATIC SEARCH OF THE REFERENCE SPECTRUM
// ==========================================

// -----------------------------------------------------------------------------
// FUNCTION      EngineSetRefIndexes
// -----------------------------------------------------------------------------
// PURPOSE       Browse spectra and search for a reference spectrum
//
// INPUT         pEngineContext     pointer to the engine context
//
// RETURN        0 in case of success; the code of the error otherwise
// -----------------------------------------------------------------------------

RC EngineSetRefIndexes(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  INDEX              *indexList,                                                // indexes of records
                      indexTabFeno,                                             // browse analysis windows list
                      indexRecord,                                              // browse spectra records in file
                      indexZmMin;                                               // index of record with SZA minimum

  PROJECT            *pProject;                                                 // pointer to the project part of the engine
  PRJCT_SPECTRA      *pSpectra;                                                 // pointer to spectra part of the project
  PRJCT_INSTRUMENTAL *pInstr;                                                   // pointer to the instrumental part of the project
  FENO               *pTabFeno;                                                 // pointer to analysis windows
  double             *ZmList,*TimeDec,                                          // zenith angles and decimal time of records
                      ZmMin,ZmMax,                                              // extrema zenith angles found in file
                      deltaZmMorning,                                           // select record in the morning with SZA the closest by SZA base
                      deltaZmAfternoon;                                         // select record in the afternoon with SZA the closest by SZA base
  INT                 NRecord;                                                  // number of hold record
  INT                 localCalDay;                                              // local day number
  INT                 recordNumber;                                             // the current number of records
  RC                  rc;                                                       // return code

  // Initializations

  indexList=NULL;
  indexZmMin=ITEM_NONE;
  ZmList=TimeDec=NULL;
  NRecord=0;
  ZmMin=360.;
  ZmMax=0.;

  pEngineContext->fileInfo.nScanRef=0;

  rc=ERROR_ID_NO;

  // Make a backup of the spectra part of the engine context

  EngineCopyContext(&ENGINE_contextRef,pEngineContext);

  ENGINE_contextRef.analysisRef.refScan=0;  // in order not to have error on zenith sky spectra

  pProject=&ENGINE_contextRef.project;
  pSpectra=&ENGINE_contextRef.project.spectra;
  pInstr=&ENGINE_contextRef.project.instrumental;

  localCalDay=ENGINE_contextRef.recordInfo.localCalDay;
  recordNumber=ENGINE_contextRef.recordNumber;

  if ((pInstr->readOutFormat==PRJCT_INSTR_FORMAT_LOGGER) ||
      (pInstr->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
      (pInstr->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD))

   pInstr->user=PRJCT_INSTR_IASB_TYPE_ZENITHAL;

  if (ENGINE_contextRef.recordNumber>0)
   {
    // Memory allocation

    if (((indexList=(INDEX *)MEMORY_AllocBuffer("EngineSetRefIndexes","",recordNumber,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
        ((ZmList=(double *)MEMORY_AllocDVector("EngineSetRefIndexes","",0,recordNumber-1))==NULL) ||
        ((TimeDec=(double *)MEMORY_AllocDVector("EngineSetRefIndexes","",0,recordNumber-1))==NULL))

     rc=ERROR_ID_ALLOC;

    else
     {
      // Browse records in file

      for (indexRecord=ENGINE_contextRef.lastRefRecord+1;indexRecord<=recordNumber;indexRecord++)
       {
       if (!(rc=EngineReadFile(&ENGINE_contextRef,indexRecord,1,localCalDay)) &&
           (ENGINE_contextRef.recordInfo.Zm>(double)0.))
        {
         if (rc==ITEM_NONE)
          rc=0;

         // Data on record

         indexList[NRecord]=indexRecord;                                        // index of record
         ZmList[NRecord]=ENGINE_contextRef.recordInfo.Zm;                       // zenith angle
         TimeDec[NRecord]=ENGINE_contextRef.recordInfo.localTimeDec;            // decimal time for determining when the measurement has occured

         // Minimum and maximum zenith angle

         if (ENGINE_contextRef.recordInfo.Zm<ZmMin)
          {
           ZmMin=ENGINE_contextRef.recordInfo.Zm;
           indexZmMin=NRecord;
          }

         if (ENGINE_contextRef.recordInfo.Zm>ZmMax)
          ZmMax=ENGINE_contextRef.recordInfo.Zm;

         NRecord++;
        }
       else if (rc==ERROR_ID_FILE_END)
        {
         rc=ERROR_ID_NO;
         break;
        }
       }



      if (rc==ERROR_ID_FILE_RECORD)
       rc=ERROR_ID_NO;

      if (NRecord && pEngineContext->analysisRef.refScan && (pEngineContext->analysisRef.scanRefIndexes!=NULL))
       {
       	memcpy(pEngineContext->analysisRef.scanRefIndexes,indexList,sizeof(INT)*NRecord);
       	pEngineContext->fileInfo.nScanRef=NRecord;
       }

      // Browse analysis windows

      for (indexTabFeno=0;(indexTabFeno<NFeno) && !rc;indexTabFeno++)
       {
        pTabFeno=&TabFeno[indexTabFeno];

        if (!pTabFeno->hidden && (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) && (pTabFeno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SZA))
         {
          // Indexes reinitialization

          pTabFeno->indexRefMorning=pTabFeno->indexRefAfternoon=pTabFeno->indexRef=ITEM_NONE;

          // No reference spectrum found in SZA range

          if (ZmMax<pTabFeno->refSZA-pTabFeno->refSZADelta)
           rc=ERROR_SetLast("EngineSetRefIndexes",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"all the day",ENGINE_contextRef.fileInfo.fileName);

          // Select record with SZA minimum

          else if (ZmMin>=pTabFeno->refSZA+pTabFeno->refSZADelta)
           pTabFeno->indexRefMorning=pTabFeno->indexRefAfternoon=indexZmMin;

          // Select a record for the morning and the afternoon

          else
           {
            deltaZmMorning=deltaZmAfternoon=360.;

            for (indexRecord=0;indexRecord<NRecord;indexRecord++)
             {
              if ((ZmList[indexRecord]>=pTabFeno->refSZA-pTabFeno->refSZADelta) &&
                  (ZmList[indexRecord]<=pTabFeno->refSZA+pTabFeno->refSZADelta))
               {
                if ((TimeDec[indexRecord]<=ENGINE_localNoon) && (fabs(pTabFeno->refSZA-ZmList[indexRecord])<deltaZmMorning))
                 {
                  pTabFeno->indexRefMorning=indexRecord;
                  deltaZmMorning=fabs(pTabFeno->refSZA-ZmList[indexRecord]);
                 }

                if ((TimeDec[indexRecord]>ENGINE_localNoon) && (fabs(pTabFeno->refSZA-ZmList[indexRecord])<deltaZmAfternoon))
                 {
                  pTabFeno->indexRefAfternoon=indexRecord;
                  deltaZmAfternoon=fabs(pTabFeno->refSZA-ZmList[indexRecord]);
                 }
               }
             }

            // No record found for the morning OR the afternoon

            if ((pTabFeno->indexRefMorning==ITEM_NONE) && (pTabFeno->indexRefAfternoon==ITEM_NONE))
             rc=ERROR_SetLast("EngineSetRefIndexes",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"all the day",ENGINE_contextRef.fileInfo.fileName);
            else if (pInstr->readOutFormat!=PRJCT_INSTR_FORMAT_ASCII)
             {
              if (pTabFeno->indexRefMorning==ITEM_NONE)
               {
               	rc=ERROR_SetLast("EngineSetRefIndexes",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"the morning",ENGINE_contextRef.fileInfo.fileName);
                pTabFeno->indexRefMorning=pTabFeno->indexRefAfternoon;
               }
              else if (pTabFeno->indexRefAfternoon==ITEM_NONE)
               {
               	rc=ERROR_SetLast("EngineSetRefIndexes",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"the afternoon",ENGINE_contextRef.fileInfo.fileName);
                pTabFeno->indexRefAfternoon=pTabFeno->indexRefMorning;
               }
             }
           }

          pTabFeno->oldZmRefMorning=pTabFeno->ZmRefMorning;
          pTabFeno->oldZmRefAfternoon=pTabFeno->ZmRefAfternoon;
          pTabFeno->ZmRefMorning=(pTabFeno->indexRefMorning!=ITEM_NONE)?ZmList[pTabFeno->indexRefMorning]:(double)-1.;
          pTabFeno->ZmRefAfternoon=(pTabFeno->indexRefAfternoon!=ITEM_NONE)?ZmList[pTabFeno->indexRefAfternoon]:(double)-1.;
          pTabFeno->indexRefMorning=(pTabFeno->indexRefMorning!=ITEM_NONE)?indexList[pTabFeno->indexRefMorning]:ITEM_NONE;
          pTabFeno->indexRefAfternoon=(pTabFeno->indexRefAfternoon!=ITEM_NONE)?indexList[pTabFeno->indexRefAfternoon]:ITEM_NONE;
         }
       }
     }
   }

  // Copy information from the ref context to the main context

  pEngineContext->lastRefRecord=ENGINE_contextRef.lastRefRecord;

  // Release buffers

  if (indexList!=NULL)
   MEMORY_ReleaseBuffer("EngineSetRefIndexes","indexList",indexList);
  if (ZmList!=NULL)
   MEMORY_ReleaseDVector("EngineSetRefIndexes","ZmList",ZmList,0);
  if (TimeDec!=NULL)
   MEMORY_ReleaseDVector("EngineSetRefIndexes","TimeDec",TimeDec,0);

  // Return

  return rc;
 }


// -----------------------------------------------------------------------------
// FUNCTION      EngineSetRefIndexesMFC
// -----------------------------------------------------------------------------
// PURPOSE       Browse spectra and search for a reference spectrum
//
// INPUT         pEngineContext     pointer to the engine context
//
// RETURN        0 in case of success; the code of the error otherwise
// -----------------------------------------------------------------------------

RC EngineSetRefIndexesMFC(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  INDEX               indexTabFeno,                                             // browse analysis windows list
                      indexZmMin,                                               // index of record with SZA minimum
                      indexRecord,                                              // browse records
                     *indexList;
  DoasCh              filePath[MAX_ITEM_TEXT_LEN+1];
  DoasCh              fileName[MAX_ITEM_TEXT_LEN+1];
  DoasCh             *ptr,fileExt[MAX_ITEM_TEXT_LEN+1];
  DoasCh            **fileList;
  struct dirent *fileInfo;
  DIR *hDir;
  INDEX               indexFile;
  ANALYSIS_REF *pRef;
  PROJECT            *pProject;                                                 // pointer to the project part of the engine
  PRJCT_SPECTRA      *pSpectra;                                                 // pointer to spectra part of the project
  PRJCT_INSTRUMENTAL *pInstr;                                                   // pointer to the instrumental part of the project
  FENO               *pTabFeno;                                                 // pointer to analysis windows
  double             *ZmList,*TimeDec,                                          // zenith angles and decimal time of records
                      ZmMin,ZmMax,                                              // extrema zenith angles found in file
                      deltaZmMorning,                                           // select record in the morning with SZA the closest by SZA base
                      deltaZmAfternoon;                                         // select record in the afternoon with SZA the closest by SZA base
  INT                 NRecord;                                                  // number of hold record
  INT                 localCalDay;                                              // local day number
  INT                 fileNumber;                                               // the number of files in the current directory
  INT                 recordNumber;                                             // the current number of records
  RC                  rc;                                                       // return code

  // Initializations

  fileList=NULL;
  indexZmMin=ITEM_NONE;
  ZmList=TimeDec=NULL;
  indexList=NULL;
  NRecord=0;
  ZmMin=360.;
  ZmMax=0.;
  hDir=NULL;

  rc=ERROR_ID_NO;

  // Release scanref buffer

  pRef=&pEngineContext->analysisRef;

  if (pRef->scanRefIndexes!=NULL)
   MEMORY_ReleaseBuffer("EngineSetRefIndexesMFC","scanRefIndexes",pRef->scanRefIndexes);
  pRef->scanRefIndexes=NULL;

  // Release buffers

  if (pRef->scanRefFiles!=NULL)
   MEMORY_ReleaseBuffer("EngineSetRefIndexesMFC","scanRefFiles",pRef->scanRefFiles);
  pRef->scanRefFiles=NULL;

  pRef->nscanRefFiles=0;
  pEngineContext->fileInfo.nScanRef=0;

  // Make a backup of the spectra part of the engine context

  EngineCopyContext(&ENGINE_contextRef,pEngineContext);

  ENGINE_contextRef.analysisRef.refScan=0;                                      // in order not to have error on zenith sky spectra

  pProject=&ENGINE_contextRef.project;
  pSpectra=&ENGINE_contextRef.project.spectra;
  pInstr=&ENGINE_contextRef.project.instrumental;

  pRef=&pEngineContext->analysisRef;

  localCalDay=ENGINE_contextRef.recordInfo.localCalDay;
  recordNumber=ENGINE_contextRef.recordNumber;

  // Get file path

  strcpy(filePath,pEngineContext->fileInfo.fileName);

  if ((ptr=strrchr(filePath,PATH_SEP))==NULL)
   {
  	 strcpy(filePath,".");
  	 ptr=pEngineContext->fileInfo.fileName;
  	}
  else
   *ptr++=0;

 	// Retrieve file extension

  if ((ptr=strrchr(ptr,'.'))!=NULL)
   strcpy(fileExt,ptr+1);
  else if (strlen(pEngineContext->project.instrumental.fileExt))
   strcpy(fileExt,pEngineContext->project.instrumental.fileExt);
  else
   memset(fileExt,0,MAX_STR_SHORT_LEN+1);

  // Browse files to get the file number

  for (hDir=opendir(filePath),fileNumber=0;(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);)
   {
    sprintf(fileName,"%s%c%s",filePath,PATH_SEP,fileInfo->d_name);

    if (!STD_IsDir(fileName) &&
     ((((ptr=strrchr(fileInfo->d_name,'.'))==NULL) && !strlen(fileExt)) ||
       ((strlen(ptr+1)==strlen(fileExt)) && !STD_Stricmp(ptr+1,fileExt))))

     fileNumber++;
   }

  if (hDir!=NULL)
   {
    closedir(hDir);
    hDir=NULL;
   }

  if (fileNumber>0)
   {
    pRef->nscanRefFiles=pEngineContext->fileInfo.nScanRef=fileNumber;

    // Memory allocation

    if (((fileList=(DoasCh **)MEMORY_AllocBuffer("EngineSetRefIndexesMFC","fileList",fileNumber,sizeof(DoasCh *),0,MEMORY_TYPE_PTR))==NULL) ||
        ((indexList=(INDEX *)MEMORY_AllocBuffer("EngineSetRefIndexesMFC","",fileNumber,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
        ((ZmList=(double *)MEMORY_AllocDVector("EngineSetRefIndexesMFC","ZmList",0,fileNumber-1))==NULL) ||
        ((TimeDec=(double *)MEMORY_AllocDVector("EngineSetRefIndexesMFC","TimeDec",0,fileNumber-1))==NULL) ||
         (pRef->refScan &&
       (((pRef->scanRefIndexes=(INT *)MEMORY_AllocBuffer("EngineSetRefIndexesMFC","scanRefIndexes",fileNumber,sizeof(INT),0,MEMORY_TYPE_INT))==NULL) ||
        ((pRef->scanRefFiles=(DoasCh *)MEMORY_AllocBuffer("EngineSetRefIndexesMFC","scanRefFiles",fileNumber*(MAX_ITEM_TEXT_LEN+1),1,0,MEMORY_TYPE_STRING))==NULL))))

     rc=ERROR_ID_ALLOC;

    else
     {
     	memset(fileList,0,sizeof(DoasCh *)*fileNumber);

     	if (pRef->refScan)
     	 memset(pRef->scanRefFiles,0,fileNumber*(MAX_ITEM_TEXT_LEN+1));

     	for (indexFile=0;(indexFile<fileNumber) && !rc;indexFile++)
     	 if ((fileList[indexFile]=(DoasCh *)MEMORY_AllocBuffer("EngineSetRefIndexesMFC","fileList[indexFile]",MAX_ITEM_TEXT_LEN+1,sizeof(DoasCh),0,MEMORY_TYPE_STRING))==NULL)
     	  rc=ERROR_SetLast("EngineSetRefIndexesMFC",ERROR_TYPE_FATAL,ERROR_ID_ALLOC,"fileList[indexFile]");

      if (!rc)
       {
        // Browse records in file

        for (hDir=opendir(filePath),NRecord=0,indexFile=0;(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);)
         {
          sprintf(ENGINE_contextRef.fileInfo.fileName,"%s%c%s",filePath,PATH_SEP,fileInfo->d_name);

          if (!STD_IsDir(ENGINE_contextRef.fileInfo.fileName) &&
           ((((ptr=strrchr(fileInfo->d_name,'.'))==NULL) && !strlen(fileExt)) ||
             ((strlen(ptr+1)==strlen(fileExt)) && !STD_Stricmp(ptr+1,fileExt)))
             )
           {
            if (rc==ITEM_NONE)
             rc=0;

            // Data on record

            if (pRef->refScan)                                                                   // reference of the scan
             strcpy(&pRef->scanRefFiles[indexFile*(MAX_ITEM_TEXT_LEN+1)],fileInfo->d_name);

            strcpy(fileList[indexFile],ENGINE_contextRef.fileInfo.fileName);    // index of record

            if (!(rc=EngineReadFile(&ENGINE_contextRef,1,1,localCalDay)) && (ENGINE_contextRef.recordInfo.Zm>(double)0.))
             {
              indexList[NRecord]=indexFile;                                       // index of record
              ZmList[NRecord]=ENGINE_contextRef.recordInfo.Zm;                    // zenith angle
              TimeDec[NRecord]=ENGINE_contextRef.recordInfo.localTimeDec;         // decimal time for determining when the measurement has occured

              // Minimum and maximum zenith angle

              if (ENGINE_contextRef.recordInfo.Zm<ZmMin)
               {
                ZmMin=ENGINE_contextRef.recordInfo.Zm;
                indexZmMin=NRecord;
               }

              if (ENGINE_contextRef.recordInfo.Zm>ZmMax)
               ZmMax=ENGINE_contextRef.recordInfo.Zm;

              NRecord++;
             }
            else if (rc==ERROR_ID_FILE_END)
             {
              rc=ERROR_ID_NO;
              break;
             }

            indexFile++;
           }
         }

        if (hDir!=NULL)
         {
          closedir(hDir);
          hDir=NULL;
         }

        if (rc==ERROR_ID_FILE_RECORD)
         rc=ERROR_ID_NO;

        if (NRecord && pEngineContext->analysisRef.refScan)
         {
         	memcpy(pEngineContext->analysisRef.scanRefIndexes,indexList,sizeof(INT)*NRecord);
         	pEngineContext->fileInfo.nScanRef=NRecord;
         }

        // Browse analysis windows

        for (indexTabFeno=0;(indexTabFeno<NFeno) && !rc;indexTabFeno++)
         {
          pTabFeno=&TabFeno[indexTabFeno];

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

              for (indexRecord=0;indexRecord<NRecord;indexRecord++)
               {
                if ((ZmList[indexRecord]>=pTabFeno->refSZA-pTabFeno->refSZADelta) &&
                    (ZmList[indexRecord]<=pTabFeno->refSZA+pTabFeno->refSZADelta))
                 {
                  if ((TimeDec[indexRecord]<=ENGINE_localNoon) && (fabs(pTabFeno->refSZA-ZmList[indexRecord])<deltaZmMorning))
                   {
                    pTabFeno->indexRefMorning=indexRecord;
                    deltaZmMorning=fabs(pTabFeno->refSZA-ZmList[indexRecord]);
                   }

                  if ((TimeDec[indexRecord]>ENGINE_localNoon) && (fabs(pTabFeno->refSZA-ZmList[indexRecord])<deltaZmAfternoon))
                   {
                    pTabFeno->indexRefAfternoon=indexRecord;
                    deltaZmAfternoon=fabs(pTabFeno->refSZA-ZmList[indexRecord]);
                   }
                 }
               }

              // No record found for the morning OR the afternoon

              if ((pTabFeno->indexRefMorning==ITEM_NONE) && (pTabFeno->indexRefAfternoon==ITEM_NONE))
               rc=ERROR_SetLast("EngineSetRefIndexesMFC",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"all the day",pEngineContext->fileInfo.fileName);
              else if (pInstr->readOutFormat!=PRJCT_INSTR_FORMAT_ASCII)
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

            pTabFeno->oldZmRefMorning=pTabFeno->ZmRefMorning;
            pTabFeno->oldZmRefAfternoon=pTabFeno->ZmRefAfternoon;
            pTabFeno->ZmRefMorning=(pTabFeno->indexRefMorning!=ITEM_NONE)?ZmList[pTabFeno->indexRefMorning]:(double)-1.;
            pTabFeno->ZmRefAfternoon=(pTabFeno->indexRefAfternoon!=ITEM_NONE)?ZmList[pTabFeno->indexRefAfternoon]:(double)-1.;

            strcpy(pTabFeno->refAM,(pTabFeno->indexRefMorning!=ITEM_NONE)?fileList[indexList[pTabFeno->indexRefMorning]]:"");
            strcpy(pTabFeno->refPM,(pTabFeno->indexRefAfternoon!=ITEM_NONE)?fileList[indexList[pTabFeno->indexRefAfternoon]]:"");
           }
         }
       }
     }

    if (pRef->refScan)
     pRef->nscanRefFiles=fileNumber;

    // Release buffers

    if (fileList!=NULL)
     {
      for (indexFile=0;indexFile<fileNumber;indexFile++)
       if (fileList[indexFile]!=NULL)
        MEMORY_ReleaseBuffer("EngineSetRefIndexesMFC","fileList[indesFile]",fileList[indexFile]);

      MEMORY_ReleaseBuffer("EngineSetRefIndexesMFC","fileList",fileList);
     }

    if (indexList!=NULL)
     MEMORY_ReleaseBuffer("EngineSetRefIndexesMFC","indexList",indexList);
    if (ZmList!=NULL)
     MEMORY_ReleaseDVector("EngineSetRefIndexesMFC","ZmList",ZmList,0);
    if (TimeDec!=NULL)
     MEMORY_ReleaseDVector("EngineSetRefIndexesMFC","TimeDec",TimeDec,0);
   }

  // Copy information from the ref context to the main context

  pEngineContext->lastRefRecord=ENGINE_contextRef.lastRefRecord;

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

  INDEX indexRefRecord,                                                         // index of best record in file for reference selection
        indexTabFeno,                                                           // browse analysis windows
        indexWindow,                                                            // avoid gaps
        indexPage,
        indexColumn,
        indexRecord,
        indexScanRecord;

  FENO *pTabFeno;                                                               // pointer to the analysis window
  INT useKurucz,alignRef,useUsamp,saveFlag,newDimL;
  INT *scanRefIndexes;
  double factTemp;
  DoasCh *ptr;
  RC rc;

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  rc=ERROR_ID_NO;
  saveFlag=(INT)pEngineContext->project.spectra.displayDataFlag;
  useKurucz=alignRef=useUsamp=0;
  indexRefRecord=indexScanRecord=ITEM_NONE;
  indexColumn=2;

  // Select spectra records as reference

  if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_ASCII) && !pEngineContext->project.instrumental.ascii.szaSaveFlag)
   rc=ERROR_SetLast("EngineNewRef",ERROR_TYPE_WARNING,ERROR_ID_FILE_AUTOMATIC);

  else if (pRecord->localCalDay!=ENGINE_contextRef.recordInfo.localCalDay)
//  else if (memcmp((char *)&pRecord->present_day,(char *)&ENGINE_contextRef.present_day,sizeof(SHORT_DATE))!=0)  // ref and spectrum are not of the same day
   rc=((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
       (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_BIRA)) ?
         EngineSetRefIndexesMFC(pEngineContext):EngineSetRefIndexes(pEngineContext);

  if (pEngineContext->analysisRef.refScan && (pEngineContext->analysisRef.scanRefIndexes!=NULL))
   {
   	scanRefIndexes=pEngineContext->analysisRef.scanRefIndexes;

    indexRecord=((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD) ||
                 (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
                 (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_BIRA)) ?
                  MFC_SearchForCurrentFileIndex(pEngineContext):pEngineContext->indexRecord;

   	if (indexRecord!=ITEM_NONE)
   	 {
   	  for (indexScanRecord=0;indexScanRecord<pEngineContext->fileInfo.nScanRef;indexScanRecord++)
   	   if (scanRefIndexes[indexScanRecord]>=indexRecord)
   	    break;

      indexScanRecord=(indexScanRecord>=pEngineContext->fileInfo.nScanRef)?ITEM_NONE:scanRefIndexes[indexScanRecord];
   	 }
   	else
   	 indexScanRecord=ITEM_NONE;
   }

  for (indexTabFeno=0;(indexTabFeno<NFeno) && !rc;indexTabFeno++)
   {
    pTabFeno=&TabFeno[indexTabFeno];
    indexPage=indexTabFeno+plotPageAnalysis;

    if (!(pTabFeno->hidden) &&                                                      // not the definition of the window for the wavelength calibration
         (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC))  // automatic reference selection only
     {
      pTabFeno->displayRef=0;

      // Reference spectrum selected on the SZA

      if (pTabFeno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SZA)
       indexRefRecord=(pRecord->localTimeDec<=ENGINE_localNoon)?pTabFeno->indexRefMorning:pTabFeno->indexRefAfternoon;
      else
       indexRefRecord=indexScanRecord;

      // There is no reference spectrum for the requested twilight

      if (indexRefRecord==ITEM_NONE)
       {
        memcpy((char *)&ENGINE_contextRef.recordInfo.present_day,(char *)&pRecord->present_day,sizeof(SHORT_DATE));
        rc=ERROR_ID_FILE_RECORD;
       }

      // There is a reference spectrum for the requested twilight

      else if ((indexRefRecord!=pTabFeno->indexRef) || (indexRefRecord!=ENGINE_contextRef.indexRecord))
       {
       	if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_MFC) &&
            (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_MFC_STD) &&
            (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_MFC_BIRA))

         rc=EngineReadFile(&ENGINE_contextRef,indexRefRecord,0,0);

        else
         {
         	if (pEngineContext->analysisRef.refScan)
         	 {
           	strcpy(ENGINE_contextRef.fileInfo.fileName,pEngineContext->fileInfo.fileName);

           	if ((ptr=strrchr(ENGINE_contextRef.fileInfo.fileName,'/'))!=NULL)
           	 strcpy(ptr+1,&pEngineContext->analysisRef.scanRefFiles[indexRefRecord*(MAX_ITEM_TEXT_LEN+1)]);
           	else
           	 strcpy(ENGINE_contextRef.fileInfo.fileName,&pEngineContext->analysisRef.scanRefFiles[indexRefRecord*(MAX_ITEM_TEXT_LEN+1)]);
         	 }
         	else
         	 strcpy(ENGINE_contextRef.fileInfo.fileName,(pRecord->localTimeDec<=ENGINE_localNoon)?pTabFeno->refAM:pTabFeno->refPM);

         	rc=EngineReadFile(&ENGINE_contextRef,1,0,0);
         }

        if (!rc)
         {
          alignRef++;

          if (!pTabFeno->useEtalon)
           {
            memcpy(pTabFeno->LambdaK,ENGINE_contextRef.buffers.lambda,sizeof(double)*NDET);
            memcpy(pTabFeno->LambdaRef,ENGINE_contextRef.buffers.lambda,sizeof(double)*NDET);

            for (indexWindow=0,newDimL=0;indexWindow<pTabFeno->svd.Z;indexWindow++)
             {
              pTabFeno->svd.Fenetre[indexWindow][0]=FNPixel(ENGINE_contextRef.buffers.lambda,pTabFeno->svd.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
              pTabFeno->svd.Fenetre[indexWindow][1]=FNPixel(ENGINE_contextRef.buffers.lambda,pTabFeno->svd.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

              newDimL+=(pTabFeno->svd.Fenetre[indexWindow][1]-pTabFeno->svd.Fenetre[indexWindow][0]+1);
             }

            if (newDimL!=pTabFeno->svd.DimL)
             {
              ANALYSE_SvdFree("EngineNewRef",&pTabFeno->svd);
              pTabFeno->svd.DimL=newDimL;
              ANALYSE_SvdLocalAlloc("EngineNewRef",&pTabFeno->svd);
             }

            if (pTabFeno->useKurucz)
             {
              KURUCZ_Init(0);
              useKurucz++;
             }
            else if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef))!=ERROR_ID_NO) ||
                     ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,&ANALYSIS_slit,pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2,&pSlitOptions->slitFunction.slitParam3,&pSlitOptions->slitFunction.slitParam4))!=ERROR_ID_NO))
             break;
           }

          if (pTabFeno->useUsamp)
           useUsamp++;

          memcpy(pTabFeno->Sref,ENGINE_contextRef.buffers.spectrum,sizeof(double)*NDET);

          if ((rc=VECTOR_NormalizeVector(pTabFeno->Sref-1,NDET,&pTabFeno->refNormFact,"EngineNewRef"))!=ERROR_ID_NO)
           break;

          pTabFeno->indexRef=indexRefRecord;
          pTabFeno->Zm=ENGINE_contextRef.recordInfo.Zm;
          pTabFeno->Tm=ENGINE_contextRef.recordInfo.Tm;
          pTabFeno->TimeDec=ENGINE_contextRef.recordInfo.TimeDec;
          pTabFeno->displayRef=1;

          memcpy(&pTabFeno->refDate,&ENGINE_contextRef.recordInfo.present_day,sizeof(SHORT_DATE));
         }
        else if (indexRefRecord==pTabFeno->indexRef)
         pTabFeno->displayRef=1;
       }

      if (indexRefRecord!=ITEM_NONE && pEngineContext->project.spectra.displaySpectraFlag)
       {
        SHORT_DATE  *pDay;                                                      // pointer to measurement date
        struct time *pTime;                                                     // pointer to measurement date
       	char string[80],tabTitle[80];
       	plot_data_t spectrumData;
       	int SvdPDeb,SvdPFin;
       	int indexLine;

       	SvdPDeb=pTabFeno->svd.Fenetre[0][0];
       	SvdPFin=pTabFeno->svd.Fenetre[pTabFeno->svd.Z-1][1];
        pDay=&ENGINE_contextRef.recordInfo.present_day;
        pTime=&ENGINE_contextRef.recordInfo.present_time;

       	sprintf(string,"Selected reference (%d/%d, SZA %.2f)",ENGINE_contextRef.indexRecord,ENGINE_contextRef.recordNumber,ENGINE_contextRef.recordInfo.Zm);
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

          if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD) ||
              (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
              (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_BIRA))
           mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Reference file","%s",ENGINE_contextRef.fileInfo.fileName);
          else
           mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Record number","%d/%d",ENGINE_contextRef.indexRecord,ENGINE_contextRef.recordNumber);

          mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Date and Time","%02d/%02d/%d %02d:%02d:%02d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
          mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"SZA","%g",ENGINE_contextRef.recordInfo.Zm);

          pTabFeno->displayLineIndex=indexLine+1;
         }

       }
     }
   }

  // Reference alignment

  if (!rc && useKurucz)
   rc=KURUCZ_Reference(ENGINE_contextRef.buffers.instrFunction,1,saveFlag,1,responseHandle);

  if (!rc && alignRef)
   rc=ANALYSE_AlignReference(pEngineContext,1,saveFlag,responseHandle);

  if (!rc && useUsamp)
   rc=ANALYSE_UsampBuild(1,ITEM_NONE);

  // Return

  return rc;
 }


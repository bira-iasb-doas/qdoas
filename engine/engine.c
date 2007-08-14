

#include "engine.h"

ENGINE_CONTEXT engineContext;

// -----------------------------------------------------------------------------
// FUNCTION      EngineCreateContext
// -----------------------------------------------------------------------------
// PURPOSE       Create a context for the engine
// -----------------------------------------------------------------------------

ENGINE_CONTEXT *EngineCreateContext(void)
 {
 	memset(&engineContext,0,sizeof(ENGINE_CONTEXT));
 	return &engineContext;
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineDestroyContext
// -----------------------------------------------------------------------------
// PURPOSE       Destroy the context of the current engine
//
// INPUT         pEngineContext     pointer to the engine context
// -----------------------------------------------------------------------------

void EngineDestroyContext(ENGINE_CONTEXT *pEngineContext)
 {
  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("EngineDestroyContext",DEBUG_FCTTYPE_FILE);
  #endif

  // Release buffers

  if (pEngineContext->lembda!=NULL)
   MEMORY_ReleaseDVector("EngineDestroyContext ","lembda",pEngineContext->lembda,0);
  if (pEngineContext->instrFunction!=NULL)
   MEMORY_ReleaseDVector("EngineDestroyContext ","instrFunction",pEngineContext->instrFunction,0);
  if (pEngineContext->spectrum!=NULL)
   MEMORY_ReleaseDVector("EngineDestroyContext ","spectrum",pEngineContext->spectrum,0);
  if (pEngineContext->sigmaSpec!=NULL)
   MEMORY_ReleaseDVector("EngineDestroyContext ","sigmaSpec",pEngineContext->sigmaSpec,0);
  if (pEngineContext->darkCurrent!=NULL)
   MEMORY_ReleaseDVector("EngineDestroyContext ","darkCurrent",pEngineContext->darkCurrent,0);
  if (pEngineContext->specMax!=NULL)
   MEMORY_ReleaseDVector("EngineDestroyContext ","specMax",pEngineContext->specMax,0);
  if (pEngineContext->varPix!=NULL)
   MEMORY_ReleaseDVector("EngineDestroyContext ","varPix",pEngineContext->varPix,0);
  if (pEngineContext->dnl!=NULL)
   MEMORY_ReleaseDVector("EngineDestroyContext ","dnl",pEngineContext->dnl,0);
  if (pEngineContext->recordIndexes!=NULL)
   MEMORY_ReleaseBuffer("EngineDestroyContext ","recordIndexes",pEngineContext->recordIndexes);

  CCD_ResetInstrumental(&pEngineContext->ccd);

  // Reset structure

  memset(pEngineContext,0,sizeof(ENGINE_CONTEXT));

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("EngineDestroyContext",0);
  #endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineSetProject
// -----------------------------------------------------------------------------
// PURPOSE       Allocate buffers requested by the project
//
// INPUT         pEngineContext     pointer to the engine context
// -----------------------------------------------------------------------------

void EngineSetProject(ENGINE_CONTEXT *pEngineContext)
 {
 	// Declarations

 	PROJECT *pProject;
 	RC rc;
 	int i;

 	// Initializations

 	pProject=&pEngineContext->project;

  // Fill global variables

  NDET=pEngineContext->NDET;

  // Allocate buffers

  if (((pEngineContext->lembda=MEMORY_AllocDVector("EngineSetProject ","lembda",0,NDET-1))==NULL) ||
      ((pEngineContext->spectrum=MEMORY_AllocDVector("EngineSetProject ","spectrum",0,NDET-1))==NULL) ||

     (((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_CCD_OHP_96) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_CCD_HA_94) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_ULB) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE)) &&

      ((pEngineContext->recordIndexes=(ULONG *)MEMORY_AllocBuffer("EngineSetProject ","recordIndexes",2001,sizeof(ULONG),0,MEMORY_TYPE_LONG))==NULL)) ||

     (((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&

      ((pEngineContext->sigmaSpec=MEMORY_AllocDVector("EngineSetProject ","sigmaSpec",0,NDET-1))==NULL)) ||
      ((pProject->spectra.darkFlag!=0) &&
      ((pEngineContext->darkCurrent=MEMORY_AllocDVector("EngineSetProject ","darkCurrent",0,NDET-1))==NULL)))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Initialize buffers

    for (i=0;i<NDET;i++)                                                        // Load calibration file from the project
     pEngineContext->lembda[i]=i;

    if (pEngineContext->darkCurrent!=NULL)
     VECTOR_Init(pEngineContext->darkCurrent,(double)0.,NDET);                  // To check the initialization of the ANALYSE_zeros vector ...
   }
 }

int EngineSetFile(ENGINE_CONTEXT *pEngineContext,const char *fileName)
 {
 	// Declarations

 	RC rc;

 	// Initialization

 	rc=ERROR_ID_NO;

  strcpy(pEngineContext->fileName,fileName);

  // Some satellite measurements have their own functions to open the file

  if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_HDF) &&
      (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) &&
      (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS) &&
      (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GOME2) &&
     ((pEngineContext->specFp=fopen(engineContext.fileName,"rb"))==NULL))

   rc=ERROR_SetLast("EngineSetFile",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileName);

  else

   switch((INT)pEngineContext->project.instrumental.readOutFormat)
    {
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_ASCII :
      rc=ASCII_Set(pEngineContext,pEngineContext->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_ACTON :
      rc=SetActon_Logger(pEngineContext,pEngineContext->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_PDASI_EASOE :
      rc=SetEASOE(pEngineContext,pEngineContext->specFp,pEngineContext->namesFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_PDAEGG :
      rc=SetPDA_EGG(pEngineContext,pEngineContext->specFp,1);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_PDAEGG_OLD :
      rc=SetPDA_EGG(pEngineContext,pEngineContext->specFp,0);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_LOGGER :
      rc=SetPDA_EGG_Logger(pEngineContext,pEngineContext->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_PDAEGG_ULB :
      rc=SetPDA_EGG_Ulb(pEngineContext,pEngineContext->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SAOZ_VIS :
      rc=SetSAOZ(pEngineContext,pEngineContext->specFp,VIS);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SAOZ_UV :
      rc=SetSAOZ(pEngineContext,pEngineContext->specFp,UV);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SAOZ_EFM :
      rc=SetSAOZEfm(pEngineContext,pEngineContext->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_MFC :
     case PRJCT_INSTR_FORMAT_MFC_STD :
      if (!(rc=SetMFC(pEngineContext,pEngineContext->specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
       rc=MFC_LoadAnalysis(pEngineContext);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_RASAS :
      rc=SetRAS(pEngineContext,pEngineContext->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_UOFT :
      rc=SetUofT(pEngineContext,pEngineContext->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_NOAA :
      rc=SetNOAA(pEngineContext,pEngineContext->specFp);
     break;
  // ---------------------------------------------------------------------------
     #if defined (__INCLUDE_HDF_) && __INCLUDE_HDF_
     case PRJCT_INSTR_FORMAT_OMI :
      rc=OMI_SetHDF(pEngineContext);
     break;
     #endif
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_CCD_EEV :
      rc=SetCCD_EEV(pEngineContext,pEngineContext->specFp,pEngineContext->darkFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_CCD_OHP_96 :
      rc=SetCCD(pEngineContext,pEngineContext->specFp,0);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_CCD_HA_94 :
      rc=SetCCD(pEngineContext,pEngineContext->specFp,1);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_CCD_ULB :
      rc=SetCCD_Ulb(pEngineContext,pEngineContext->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_OPUS :
      rc=OPUS_Set(pEngineContext,pEngineContext->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_GDP_ASCII :
      if (!(rc=GDP_ASC_Set(pEngineContext,pEngineContext->specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
       rc=GDP_ASC_LoadAnalysis(pEngineContext,pEngineContext->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_GDP_BIN :
      if (!(rc=GDP_BIN_Set(pEngineContext,pEngineContext->specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
       rc=GDP_BIN_LoadAnalysis(pEngineContext,pEngineContext->specFp);

     break;
  // ---------------------------------------------------------------------------
     #if defined (__INCLUDE_HDF_) && __INCLUDE_HDF_
     case PRJCT_INSTR_FORMAT_SCIA_HDF :
      if (!(rc=SCIA_SetHDF(pEngineContext)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
       rc=SCIA_LoadAnalysis(pEngineContext);
     break;
     #endif
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SCIA_PDS :
      if (!(rc=SCIA_SetPDS(pEngineContext)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
       rc=SCIA_LoadAnalysis(pEngineContext);
     break;
  // ---------------------------------------------------------------------------
// GOME2     case PRJCT_INSTR_FORMAT_GOME2 :
// GOME2      if (!(rc=GOME2_Set(pEngineContext)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
// GOME2       rc=GOME2_LoadAnalysis(pEngineContext);
// GOME2     break;
  // ---------------------------------------------------------------------------
     default :
      rc=ERROR_ID_FILE_FORMAT;
     break;
  // ---------------------------------------------------------------------------
   }

  return pEngineContext->recordNumber;
 }

void EngineReadFile(ENGINE_CONTEXT *pEngineContext,int indexRecord,double *x,double *y)
 {
 	ReliPDA_EGG_Logger(pEngineContext,indexRecord,0,0,engineContext.specFp);

 	memcpy((double *)x,(double *)pEngineContext->lembda,sizeof(double)*pEngineContext->NDET);
  memcpy((double *)y,(double *)pEngineContext->spectrum,sizeof(double)*pEngineContext->NDET);
 }

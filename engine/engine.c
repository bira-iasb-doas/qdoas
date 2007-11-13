

#include "engine.h"

ENGINE_CONTEXT engineContext;
UCHAR DOAS_dbgFile[MAX_PATH_LEN+1];

// -----------------------------------------------------------------------------
// FUNCTION      EngineCreateContext
// -----------------------------------------------------------------------------
// PURPOSE       Create a context for the engine
// -----------------------------------------------------------------------------

ENGINE_CONTEXT *EngineCreateContext(void)
 {
 	// Declaration

 	ENGINE_CONTEXT *pEngineContext=&engineContext;

  // Initializations

 	strcpy(DOAS_dbgFile,"QDOAS.dbg");
 	memset(pEngineContext,0,sizeof(ENGINE_CONTEXT));

 	if (RESOURCE_Alloc()!=ERROR_ID_NO)
   pEngineContext=NULL;

  // Return

 	return pEngineContext;
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineResetContext
// -----------------------------------------------------------------------------
// PURPOSE       Destroy the context of the current engine
//
// INPUT         pEngineContext     pointer to the engine context
// -----------------------------------------------------------------------------

void EngineResetContext(ENGINE_CONTEXT *pEngineContext)
 {
 	// Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  FILE_INFO *pFile;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("EngineResetContext",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;
  pFile=&pEngineContext->fileInfo;

  // Close files

  if (pFile->specFp!=NULL)
   fclose(pFile->specFp);
  if (pFile->darkFp!=NULL)
   fclose(pFile->darkFp);
  if (pFile->namesFp!=NULL)
   fclose(pFile->namesFp);

  // Release buffers

  if (pBuffers->lembda!=NULL)
   MEMORY_ReleaseDVector("EngineResetContext ","lembda",pBuffers->lembda,0);
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
  if (pBuffers->specMaxx!=NULL)
   MEMORY_ReleaseDVector("EngineResetContext ","specMaxx",pBuffers->specMaxx,0);
  if (pBuffers->specMax!=NULL)
   MEMORY_ReleaseDVector("EngineResetContext ","specMax",pBuffers->specMax,0);
  if (pBuffers->varPix!=NULL)
   MEMORY_ReleaseDVector("EngineResetContext ","varPix",pBuffers->varPix,0);
  if (pBuffers->dnl!=NULL)
   MEMORY_ReleaseDVector("EngineResetContext ","dnl",pBuffers->dnl,0);
  if (pBuffers->recordIndexes!=NULL)
   MEMORY_ReleaseBuffer("EngineResetContext ","recordIndexes",pBuffers->recordIndexes);

  CCD_ResetInstrumental(&pRecord->ccd);

  GDP_ASC_ReleaseBuffers();
  GDP_BIN_ReleaseBuffers();

//  GOME2_ReleaseBuffers(GOME2_BEAT_CLOSE);
//  OMI_ReleaseBuffers();

  SCIA_ReleaseBuffers(pEngineContext->project.instrumental.readOutFormat);

  if ((THRD_id!=THREAD_TYPE_NONE) && (THRD_id!=THREAD_TYPE_SPECTRA))
   ANALYSE_ResetData();

  THRD_id=THREAD_TYPE_NONE;

  // Reset structure

  memset(pEngineContext,0,sizeof(ENGINE_CONTEXT));

  // Reset other buffers

  SYMB_itemCrossN=SYMBOL_PREDEFINED_MAX;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("EngineResetContext",0);
  #endif
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
 	// Reset the context of the engine

 	EngineResetContext(pEngineContext);

 	// Release other allocated buffers

 	RESOURCE_Free();
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

 	PROJECT *pProject;                                                            // pointer to the current project
  PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

 	double *lembdaInstr;                                                          // wavelength calibration of the instrument function
 	double *instrFunction;                                                        // instrumental function
 	double *instrDeriv2;                                                          // second derivative for the instrument function
 	RC rc;                                                                        // return code
 	FILE *fp;                                                                     // file pointer
 	UCHAR str[MAX_ITEM_TEXT_LEN+1];                                               // buffer to read the lines of the file
 	int i;                                                                        // index for loops and arrays

 	// Initializations

  pBuffers=&pEngineContext->buffers;
  pProject=&pEngineContext->project;
  pInstrumental=&pProject->instrumental;

 	lembdaInstr=instrFunction=instrDeriv2=NULL;
 	rc=ERROR_ID_NO;

  // Allocate buffers

  if (((pBuffers->lembda=MEMORY_AllocDVector("EngineSetProject ","lembda",0,NDET-1))==NULL) ||
      ((pBuffers->spectrum=MEMORY_AllocDVector("EngineSetProject ","spectrum",0,NDET-1))==NULL) ||

     (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_OHP_96) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_HA_94) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE)) &&

      ((pBuffers->recordIndexes=(ULONG *)MEMORY_AllocBuffer("EngineSetProject ","recordIndexes",2001,sizeof(ULONG),0,MEMORY_TYPE_LONG))==NULL)) ||

     (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&

     (((pBuffers->sigmaSpec=MEMORY_AllocDVector("EngineSetProject ","sigmaSpec",0,NDET-1))==NULL) ||
      ((pBuffers->irrad=MEMORY_AllocDVector("EngineSetProject ","irrad",0,NDET-1))==NULL))) ||

     (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_ACTON) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
       (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE)) &&
       ((pBuffers->darkCurrent=MEMORY_AllocDVector("EngineSetProject ","darkCurrent",0,NDET-1))==NULL)))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Load the wavelength calibration

    if ((fp=fopen(pInstrumental->calibrationFile,"rt"))!=NULL)
     {
      for (i=0;i<NDET;)
       if (!fgets(str,MAX_ITEM_TEXT_LEN,fp))
        break;
       else if ((strchr(str,';')==NULL) && (strchr(str,'*')==NULL))
        {
         sscanf(str,"%lf",&pBuffers->lembda[i]);
         i++;
        }

      if (i!=NDET)
       rc=ERROR_SetLast("EngineSetProject",ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,pInstrumental->calibrationFile);

      fclose(fp);
     }
    else
     for (i=0;i<NDET;i++)
      pBuffers->lembda[i]=i+1;

    // Load the instrumental function

    if (strlen(pInstrumental->instrFunction))
     {
      if ((fp=fopen(pInstrumental->instrFunction,"rt"))==NULL)
       rc=ERROR_SetLast("EngineSetProject",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pInstrumental->instrFunction);
      else if (((pBuffers->instrFunction=MEMORY_AllocDVector("EngineSetProject","instrFunction",0,NDET-1))==NULL) ||
               ((lembdaInstr=MEMORY_AllocDVector("EngineSetProject","lembdaInstr",0,NDET-1))==NULL) ||
               ((instrFunction=MEMORY_AllocDVector("EngineSetProject","instrFunction",0,NDET-1))==NULL) ||
               ((instrDeriv2=MEMORY_AllocDVector("EngineSetProject","instrDeriv2",0,NDET-1))==NULL))

       rc=ERROR_ID_ALLOC;

      else
       {
        for (i=0;(i<NDET) && fgets(str,MAX_ITEM_TEXT_LEN,fp);)
         if ((strchr(str,';')==NULL) && (strchr(str,'*')==NULL))
          {
           sscanf(str,"%lf %lf",&lembdaInstr[i],&instrFunction[i]);
           i++;
          }

        if (!SPLINE_Deriv2(lembdaInstr,instrFunction,instrDeriv2,NDET,"EngineSetProject"))
         rc=SPLINE_Vector(lembdaInstr,instrFunction,instrDeriv2,NDET,pBuffers->lembda,pBuffers->instrFunction,NDET,SPLINE_CUBIC,"EngineSetProject");
       }

      if (fp!=NULL)
       fclose(fp);
     }

    // Initialize buffers

    if (pBuffers->darkCurrent!=NULL)
     VECTOR_Init(pBuffers->darkCurrent,(double)0.,NDET);                  // To check the initialization of the ANALYSE_zeros vector ...
   }

  // Release the allocated buffers

  if (lembdaInstr!=NULL)
   MEMORY_ReleaseDVector("EngineSetProject","lembdaInstr",lembdaInstr,0);
  if (instrFunction!=NULL)
   MEMORY_ReleaseDVector("EngineSetProject","instrFunction",instrFunction,0);
  if (instrDeriv2!=NULL)
   MEMORY_ReleaseDVector("EngineSetProject","instrDeriv2",instrDeriv2,0);
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

RC EngineSetFile(ENGINE_CONTEXT *pEngineContext,const char *fileName)
 {
 	// Declarations

  FILE_INFO *pFile;
  UCHAR fileTmp[MAX_STR_LEN+1];
 	RC rc;

 	// Initializations

 	pFile=&pEngineContext->fileInfo;

 	rc=ERROR_ID_NO;

  strcpy(pFile->fileName,fileName);
  strcpy(fileTmp,fileName);

  // Close previous files

  if (pFile->specFp!=NULL)
   fclose(pFile->specFp);
  if (pFile->darkFp!=NULL)
   fclose(pFile->darkFp);
  if (pFile->namesFp!=NULL)
   fclose(pFile->namesFp);

  pFile->specFp=pFile->darkFp=pFile->namesFp=NULL;

  // About names of record

  // SAOZ : The spectra names are used to select zenith sky or pointed measurements.
  //        In principle, names files should be in the same directory as the spectra files
  //        For the moment, I suppose that the file exists and if not, the selection of the
  //        measurement is ignored.  To improve ???

  if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SAOZ_PCDNMOS) ||
      (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE))

   pFile->namesFp=fopen(FILES_BuildFileName(fileTmp,FILE_TYPE_NAMES),"rb");

  // Dark current files : the file name is automatically built from the spectra file name

  if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
      (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_ACTON) ||
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
      rc=SetPDA_EGG(pEngineContext,pFile->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_LOGGER :
      rc=SetPDA_EGG_Logger(pEngineContext,pFile->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SAOZ_PCDNMOS :
      rc=SetSAOZ(pEngineContext,pFile->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SAOZ_EFM :
      rc=SetSAOZEfm(pEngineContext,pFile->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_MFC :
     case PRJCT_INSTR_FORMAT_MFC_STD :
      if (!(rc=SetMFC(pEngineContext,pFile->specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
       rc=MFC_LoadAnalysis(pEngineContext);
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
      if (!(rc=GDP_ASC_Set(pEngineContext,pFile->specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
       rc=GDP_ASC_LoadAnalysis(pEngineContext,pFile->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_GDP_BIN :
      if (!(rc=GDP_BIN_Set(pEngineContext,pFile->specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
       rc=GDP_BIN_LoadAnalysis(pEngineContext,pFile->specFp);

     break;
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
 	INDEX i;
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
     rc=ReliPDA_EGG(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp,pFile->namesFp,pFile->darkFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_LOGGER :
     rc=ReliPDA_EGG_Logger(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_PCDNMOS :
     rc=ReliSAOZ(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp,pFile->namesFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_EFM :
     rc=ReliSAOZEfm(pEngineContext,indexRecord,dateFlag,localCalDay,pFile->specFp);
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
// QDOAS ???     case PRJCT_INSTR_FORMAT_GOME2 :
// QDOAS ???      rc=GOME2_Read(pEngineContext,indexRecord);
// QDOAS ???     break;
 // ---------------------------------------------------------------------------
    default :
     rc=ERROR_ID_FILE_BAD_FORMAT;
    break;
 // ---------------------------------------------------------------------------
   }

  if (pEngineContext->buffers.instrFunction!=NULL)
   {
    for (i=0;(i<NDET) && !rc;i++)
     if (pEngineContext->buffers.instrFunction[i]==(double)0.)
      rc=ERROR_SetLast("EngineReadFile",ERROR_TYPE_FATAL,ERROR_ID_DIVISION_BY_0,"Instrumental function");
     else
      pEngineContext->buffers.spectrum[i]/=pEngineContext->buffers.instrFunction[i];
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

RC EngineRequestBeginBrowseSpectra(ENGINE_CONTEXT *pEngineContext,const char *spectraFileName,unsigned int action)
 {
 	// Declaration

 	RC rc;

 	// Initialize

  THRD_id=action;
  THRD_browseType=THREAD_BROWSE_SPECTRA;

  // Set file pointers

  if (!(rc=EngineSetFile(pEngineContext,spectraFileName)) && !pEngineContext->recordNumber)
   rc=ERROR_SetLast("EngineRequestBeginBrowseSpectra",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,spectraFileName);
  else
   pEngineContext->indexRecord = 1;

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      EngineSetRefIndexes
// -----------------------------------------------------------------------------
// PURPOSE       Browse spectra and search for a reference spectrum
//
// INPUT         pEngineContext     pointer to the engine context
//
// RETURN        0 in case of success; the code of the error otherwise
// -----------------------------------------------------------------------------
/*
RC EngineSetRefIndexes(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  ENGINE_CONTEXT      engineCopy;                                               // backup of the engine context
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
  INT                 localCalDay;
  RC                  rc;                                                       // return code

  // Initializations

  indexList=NULL;
  indexZmMin=ITEM_NONE;
  ZmList=TimeDec=NULL;
  NRecord=0;
  ZmMin=360.;
  ZmMax=0.;

  // Make a backup of the spectra part of the engine context

  memcpy(&engineCopy,pEngineContext,sizeof(ENGINE_CONTEXT));

  pProject=&engineCopy.project;
  pSpectra=&engineCopy.project.spectra;
  pInstr=&engineCopy.project.instrumental;

  localCalDay=pEngineContext->recordInfo.localCalDay;

  if ((pInstr->readOutFormat==PRJCT_INSTR_FORMAT_LOGGER) ||
      (pInstr->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG))

   pInstr->user=PRJCT_INSTR_IASB_TYPE_ZENITHAL;

  if (pEngineContext->recordNumber>0)
   {
    // Memory allocation

    if (((indexList=(INDEX *)MEMORY_AllocBuffer("ThrdSetRefIndexes ","",pEngineContext->recordNumber,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
        ((ZmList=(double *)MEMORY_AllocDVector("ThrdSetRefIndexes ","",0,pEngineContext->recordNumber-1))==NULL) ||
        ((TimeDec=(double *)MEMORY_AllocDVector("ThrdSetRefIndexes ","",0,pEngineContext->recordNumber-1))==NULL))

     rc=ERROR_ID_ALLOC;

    else
     {
      // Browse records in file

      for (indexRecord=THRD_lastRefRecord+1;indexRecord<=pEngineContext->recordNumber;indexRecord++)
       if (!(rc=EngineReadFile(&THRD_refInfo,indexRecord,1,localCalDay)) &&
           (pEngineContext->recordInfo.Zm>(double)0.))
        {
         if (rc==ITEM_NONE)
          rc=0;

         // Data on record

         indexList[NRecord]=indexRecord;                             // index of record
         ZmList[NRecord]=pEngineContext->recordInfo.Zm;                 // zenith angle
         TimeDec[NRecord]=pEngineContext->recordInfo.localTimeDec;      // decimal time for determining when the measurement has occured

         // Minimum and maximum zenith angle

         if (pEngineContext->recordInfo.Zm<ZmMin)
          {
           ZmMin=pEngineContext->recordInfo.Zm;
           indexZmMin=NRecord;
          }

         if (pEngineContext->recordInfo.Zm>ZmMax)
          ZmMax=pEngineContext->recordInfo.Zm;

         NRecord++;
        }
       else if (rc==ERROR_ID_FILE_END)
        {
         rc=ERROR_ID_NO;
         break;
        }

      if (rc==ERROR_ID_FILE_RECORD)
       rc=ERROR_ID_NO;

      // Browse analysis windows

      for (indexTabFeno=0;(indexTabFeno<NFeno) && (rc<THREAD_EVENT_STOP);indexTabFeno++)
       {
        pTabFeno=&TabFeno[indexTabFeno];

        if (!pTabFeno->hidden && (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC))
         {
          // Indexes reinitialization

          pTabFeno->indexRefMorning=pTabFeno->indexRefAfternoon=pTabFeno->indexRef=ITEM_NONE;

          // No reference spectrum found in SZA range

          if (ZmMax<pTabFeno->refSZA-pTabFeno->refSZADelta)
           rc=ERROR_SetLast("ThrdSetRefIndexes",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"all the day",pEngineContext->fileInfo.fileName);

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
                if ((TimeDec[indexRecord]<=THRD_localNoon) && (fabs(pTabFeno->refSZA-ZmList[indexRecord])<deltaZmMorning))
                 {
                  pTabFeno->indexRefMorning=indexRecord;
                  deltaZmMorning=fabs(pTabFeno->refSZA-ZmList[indexRecord]);
                 }

                if ((TimeDec[indexRecord]>THRD_localNoon) && (fabs(pTabFeno->refSZA-ZmList[indexRecord])<deltaZmAfternoon))
                 {
                  pTabFeno->indexRefAfternoon=indexRecord;
                  deltaZmAfternoon=fabs(pTabFeno->refSZA-ZmList[indexRecord]);
                 }
               }
             }

            // No record found for the morning OR the afternoon

            if ((pTabFeno->indexRefMorning==ITEM_NONE) && (pTabFeno->indexRefAfternoon==ITEM_NONE))
             rc=ERROR_SetLast("ThrdSetRefIndexes",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"all the day",pEngineContext->fileInfo.fileName);
            else if (pInstr->readOutFormat!=PRJCT_INSTR_FORMAT_ASCII)
             {
              if (pTabFeno->indexRefMorning==ITEM_NONE)
               {
               	rc=ERROR_SetLast("ThrdSetRefIndexes",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"the morning",pEngineContext->fileInfo.fileName);
                pTabFeno->indexRefMorning=pTabFeno->indexRefAfternoon;
               }
              else if (pTabFeno->indexRefAfternoon==ITEM_NONE)
               {
               	rc=ERROR_SetLast("ThrdSetRefIndexes",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"the afternoon",pEngineContext->fileInfo.fileName);
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

  // Restore the backup of the spectra part of the engine context

  memcpy(pEngineContext,&engineCopy,sizeof(ENGINE_CONTEXT));

  // Release buffers

  if (indexList!=NULL)
   MEMORY_ReleaseBuffer("ThrdSetRefIndexes ","indexList",indexList);
  if (ZmList!=NULL)
   MEMORY_ReleaseDVector("ThrdSetRefIndexes ","ZmList",ZmList,0);
  if (TimeDec!=NULL)
   MEMORY_ReleaseDVector("ThrdSetRefIndexes ","TimeDec",TimeDec,0);

  // Return

  return rc;
 }
*/
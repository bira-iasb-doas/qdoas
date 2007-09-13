

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

 	// Initializations

 	engineContext.TDet=(double)0.;

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
  if (pEngineContext->irrad!=NULL)
   MEMORY_ReleaseDVector("EngineDestroyContext ","irrad",pEngineContext->irrad,0);
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

 	PROJECT *pProject;                                                            // pointer to the current project
 	double *lembdaInstr;                                                          // wavelength calibration of the instrument function
 	double *instrFunction;                                                        // instrumental function
 	double *instrDeriv2;                                                          // second derivative for the instrument function
 	RC rc;                                                                        // return code
 	FILE *fp;                                                                     // file pointer
 	UCHAR str[MAX_ITEM_TEXT_LEN+1];                                               // buffer to read the lines of the file
 	int i;                                                                        // index for loops and arrays

 	// Initializations

 	pProject=&pEngineContext->project;
 	lembdaInstr=instrFunction=instrDeriv2=NULL;
 	rc=ERROR_ID_NO;

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
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE)) &&

      ((pEngineContext->recordIndexes=(ULONG *)MEMORY_AllocBuffer("EngineSetProject ","recordIndexes",2001,sizeof(ULONG),0,MEMORY_TYPE_LONG))==NULL)) ||

     (((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&

     (((pEngineContext->sigmaSpec=MEMORY_AllocDVector("EngineSetProject ","sigmaSpec",0,NDET-1))==NULL) ||
      ((pEngineContext->irrad=MEMORY_AllocDVector("EngineSetProject ","irrad",0,NDET-1))==NULL))) ||

     (((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_ACTON) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
       (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE)) &&
       ((pEngineContext->darkCurrent=MEMORY_AllocDVector("EngineSetProject ","darkCurrent",0,NDET-1))==NULL)))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Load the wavelength calibration

    if ((fp=fopen(pProject->instrumental.calibrationFile,"rt"))!=NULL)
     {
      for (i=0;i<NDET;)
       if (!fgets(str,MAX_ITEM_TEXT_LEN,fp))
        break;
       else if ((strchr(str,';')==NULL) && (strchr(str,'*')==NULL))
        {
         sscanf(str,"%lf",&pEngineContext->lembda[i]);
         i++;
        }

      if (i!=NDET)
       rc=ERROR_SetLast("EngineSetProject",ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,pProject->instrumental.calibrationFile);

      fclose(fp);
     }
    else
     for (i=0;i<NDET;i++)
      pEngineContext->lembda[i]=i+1;

    // Load the instrumental function

    if ((fp=fopen(pProject->instrumental.instrFunction,"rt"))==NULL)
     rc=ERROR_SetLast("EngineSetProject",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pProject->instrumental.instrFunction);
    else if (((pEngineContext->instrFunction=MEMORY_AllocDVector("EngineSetProject","instrFunction",0,NDET-1))==NULL) ||
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
       rc=SPLINE_Vector(lembdaInstr,instrFunction,instrDeriv2,NDET,pEngineContext->lembda,pEngineContext->instrFunction,NDET,SPLINE_CUBIC,"EngineSetProject");
     }

    if (fp!=NULL)
     fclose(fp);

    // Initialize buffers

    if (pEngineContext->darkCurrent!=NULL)
     VECTOR_Init(pEngineContext->darkCurrent,(double)0.,NDET);                  // To check the initialization of the ANALYSE_zeros vector ...
   }

  // Release the allocated buffers

  if (lembdaInstr!=NULL)
   MEMORY_ReleaseDVector("EngineSetProject","lembdaInstr",lembdaInstr,0);
  if (instrFunction!=NULL)
   MEMORY_ReleaseDVector("EngineSetProject","instrFunction",instrFunction,0);
  if (instrDeriv2!=NULL)
   MEMORY_ReleaseDVector("EngineSetProject","instrDeriv2",instrDeriv2,0);
 }

int EngineSetFile(ENGINE_CONTEXT *pEngineContext,const char *fileName)
 {
 	// Declarations

  UCHAR fileTmp[MAX_STR_LEN+1];
 	RC rc;

 	// Initializations

 	rc=ERROR_ID_NO;

  strcpy(pEngineContext->fileName,fileName);
  strcpy(fileTmp,fileName);

  // About names of record

  // SAOZ : The spectra names are used to select zenith sky or pointed measurements.
  //        In principle, names files should be in the same directory as the spectra files
  //        For the moment, I suppose that the file exists and if not, the selection of the
  //        measurement is ignored.  To improve ???

  if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SAOZ_PCDNMOS) ||
      (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE))

   pEngineContext->namesFp=fopen(FILES_BuildFileName(fileTmp,FILE_TYPE_NAMES),"rb");

  // Dark current files : the file name is automatically built from the spectra file name

  if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
      (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_ACTON) ||
      (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE))

   pEngineContext->darkFp=fopen(FILES_BuildFileName(fileTmp,FILE_TYPE_DARK),"rb");

  // Some satellite measurements have their own functions to open the file

  if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) &&
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
      rc=SetPDA_EGG(pEngineContext,pEngineContext->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_LOGGER :
      rc=SetPDA_EGG_Logger(pEngineContext,pEngineContext->specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SAOZ_PCDNMOS :
      rc=SetSAOZ(pEngineContext,pEngineContext->specFp);
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

void EngineReadFile(ENGINE_CONTEXT *pEngineContext,int indexRecord,INT dateFlag,INT localCalDay)
 {
 	// Declarations

 	INDEX i;
 	int rc;                                                                       // Return code

 	// Initializations

  memset(pEngineContext->Nom,0,20);

  pEngineContext->Zm=-1.;
  pEngineContext->Azimuth=-1.;
  pEngineContext->SkyObs=8;
  pEngineContext->ReguTemp=0.;
  pEngineContext->TDet=0.;
  pEngineContext->BestShift=0.;
  pEngineContext->rejected=0;
  pEngineContext->NTracks=0;
  pEngineContext->Cic=0.;
  pEngineContext->elevationViewAngle=-1.;
  pEngineContext->azimuthViewAngle=-1.;

  pEngineContext->longitude=0.;
  pEngineContext->latitude=0.;
  pEngineContext->altitude=0.;

  pEngineContext->aMoon=0.;
  pEngineContext->hMoon=0.;
  pEngineContext->fracMoon=0.;

  switch((INT)pEngineContext->project.instrumental.readOutFormat)
   {
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_ASCII :
     rc=ASCII_Read(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_ACTON :
     rc=ReliActon_Logger(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp,engineContext.namesFp,engineContext.darkFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDASI_EASOE :
     rc=ReliEASOE(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp,engineContext.namesFp,engineContext.darkFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDAEGG :
     rc=ReliPDA_EGG(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp,engineContext.namesFp,engineContext.darkFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_LOGGER :
     rc=ReliPDA_EGG_Logger(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_PCDNMOS :
     rc=ReliSAOZ(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp,engineContext.namesFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_EFM :
     rc=ReliSAOZEfm(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp);
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
// QDOAS ???     rc=ReliMFC(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp,mfcMask);
// QDOAS ???
// QDOAS ???    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC_STD :
     rc=ReliMFCStd(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_RASAS :
     rc=ReliRAS(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_UOFT :
     rc=ReliUofT(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_NOAA :
     rc=ReliNOAA(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp);
    break;
 // ---------------------------------------------------------------------------
    #if defined (__INCLUDE_HDF_) && __INCLUDE_HDF_
    case PRJCT_INSTR_FORMAT_OMI :
     rc=OMI_ReadHDF(pEngineContext,indexRecord);
    break;
    #endif
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_CCD_EEV :
     rc=ReliCCD_EEV(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp,engineContext.darkFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_CCD_HA_94 :
     rc=ReliCCD(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp,engineContext.namesFp,engineContext.darkFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_CCD_OHP_96 :
     rc=ReliCCDTrack(pEngineContext,indexRecord,dateFlag,localCalDay,engineContext.specFp,engineContext.namesFp,engineContext.darkFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GDP_ASCII :
     rc=GDP_ASC_Read(pEngineContext,indexRecord,dateFlag,engineContext.specFp);
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GDP_BIN :
     rc=GDP_BIN_Read(pEngineContext,indexRecord,engineContext.specFp,GDP_BIN_currentFileIndex);
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

  if (pEngineContext->instrFunction!=NULL)
   {
    for (i=0;(i<NDET) && !rc;i++)
     if (pEngineContext->instrFunction[i]==(double)0.)
      rc=ERROR_SetLast("EngineReadFile",ERROR_TYPE_FATAL,ERROR_ID_DIVISION_BY_0,"Instrumental function");
     else
      pEngineContext->spectrum[i]/=pEngineContext->instrFunction[i];
   }
 }

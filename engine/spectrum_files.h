#ifndef SPECTRUM_FILES_H
#define SPECTRUM_FILES_H

#include <stdio.h>

#include "doas.h"

// ==============
// FILES READ OUT
// ==============

extern const char *MAXDOAS_measureTypes[];
extern const char *MFCBIRA_measureTypes[];

RC   SetUofT(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliUofT(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
RC   SetNOAA(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliNOAA(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
RC   SetCCD_EEV(ENGINE_CONTEXT *pEngineContext,FILE *specFp,FILE *darkFp);
RC   ReliCCD_EEV(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *darkFp);
RC   SetCCD (ENGINE_CONTEXT *pEngineContext,FILE *specFp,int flag);
RC   ReliCCD(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp);
RC   ReliCCDTrack(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp);
RC   SetPDA_EGG(ENGINE_CONTEXT *pEngineContext,FILE *specFp,int newFlag);
RC   ReliPDA_EGG(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp,int newFlag);
RC   SetPDA_EGG_Logger(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliPDA_EGG_Logger(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);

RC   SetEASOE(ENGINE_CONTEXT *pEngineContext,FILE *specFp,FILE *namesFp);
RC   ReliEASOE(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp);

// SAOZ (CNRS, France)

RC   SetSAOZ(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliSAOZ(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp);
RC   MKZY_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   MKZY_Reli(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
RC   AIRBORNE_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   AIRBORNE_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
RC   SetSAOZEfm(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliSAOZEfm(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
RC   SetActon_Logger(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliActon_Logger(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,FILE *namesFp,FILE *darkFp);
RC   SetOceanOptics(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliOceanOptics(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
RC   ASCII_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ASCII_QDOAS_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ASCII_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
RC   ASCII_QDOAS_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
void ASCII_Free(const char *functionStr);
void ASCII_QDOAS_Reset(void);
RC   SetRAS(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliRAS(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);

void SCIA_ReleaseBuffers(char format);
RC   SCIA_SetPDS(ENGINE_CONTEXT *pEngineContext);
RC   SCIA_ReadPDS(ENGINE_CONTEXT *pEngineContext,int recordNo);
INDEX SCIA_GetRecordNumber(int hdfRecord,int obsNumber);
void SCIA_get_orbit_date(int *year, int *month, int *day);
RC SCIA_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

extern int SCIA_clusters[PRJCT_INSTR_SCIA_CHANNEL_MAX][2];

RC   CCD_LoadInstrumental(ENGINE_CONTEXT *pEngineContext);
void CCD_ResetInstrumental(CCD *pCCD);
char *CCD_GetImageFile(INDEX indexImage);

RC MKZY_SearchForSky(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC MKZY_SearchForOffset(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC MKZY_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

void GDP_ASC_ReleaseBuffers(void);
RC   GDP_ASC_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   GDP_ASC_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,FILE *specFp);
void GDP_ASC_get_orbit_date(int *year, int *month, int *day);
RC   GDP_ASC_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,FILE *specFp);

void GDP_BIN_ReleaseBuffers(void);
RC   GDP_BIN_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   GDP_BIN_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,FILE *specFp,INDEX indexFile);

// GOME2

extern int GOME2_beatLoaded;

void GOME2_ReleaseBuffers(void);

RC GOME2_Set(ENGINE_CONTEXT *pEngineContext);
RC GOME2_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,INDEX fileIndex);
RC GOME2_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);
void GOME2_get_orbit_date(int *year, int *month, int *day);

#endif

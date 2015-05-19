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
RC   ASCII_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
void ASCII_Free(const char *functionStr);
RC   SetRAS(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliRAS(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);

void SCIA_ReleaseBuffers(char format);
RC   SCIA_SetPDS(ENGINE_CONTEXT *pEngineContext);
RC   SCIA_ReadPDS(ENGINE_CONTEXT *pEngineContext,int recordNo);
INDEX SCIA_GetRecordNumber(int hdfRecord,int obsNumber);
void SCIA_get_orbit_date(int *year, int *month, int *day);
RC SCIA_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

extern int SCIA_clusters[PRJCT_INSTR_SCIA_CHANNEL_MAX][2];
extern int SCIA_ms; // number of milliseconds

RC   CCD_LoadInstrumental(ENGINE_CONTEXT *pEngineContext);
void CCD_ResetInstrumental(CCD *pCCD);
char *CCD_GetImageFile(INDEX indexImage);

typedef struct _TOldFlags
 {
  int   mode;
  int   smooth;
  int   deg_reg;
  char Null[8];
  char Ref[8];
 }
TOldFlags;

typedef struct _TBinaryMFC
 {
  char     version[20];           //     version number (not of interest)
  int       no_chan;               // !!! number of channels - 1 (usually 1023)
  void     *Spectrum;              //     pointer to the spectrum, only used at runtime
  char     specname[20];          //     optional name of the spectrum
  char     site[20];              //     name of measurement site
  char     spectroname[20];       //     name of spectrograph
  char     scan_dev[20];          //     name of scan device, e.g. PDA
  char     first_line[80];
  float     elevation;             //     elevation viewing angle
  char     spaeter[72];
  int       ty;                    //     spectrum flags, can be used to distinguish between
                                   //     different types of spectrum (e.g. straylight,
                                   //     offset, dark current...
  char     dateAndTime[28];
//  char     date[9];               // !!! date of measurement
//  char     start_time[9];         // !!! start time of measurement
//  char     stop_time[9];          // !!! stop time of measurement
//  char     dummy;
  int       low_lim;
  int       up_lim;
  int       plot_low_lim;
  int       plot_up_lim;
  int       act_chno;
  int       noscans;               // !!! number of scans added in this spectrum
  float     int_time;              // !!! integration time in seconds
  float     latitude;              //     latitude of measurement site
  float     longitude;             //     longitude of measurement site
  int       no_peaks;
  int       no_bands;
  float     min_y;                 //     minmum of spectrum
  float     max_y;                 //     maximum of spectrum
  float     y_scale;
  float     offset_Scale;
  float     wavelength1;           // !!! wavelength of channel 0
  float     average;               //     average signal of spectrum
  float     dispersion[3];         // !!! dispersion given as a polynome:
                                   //     wavelength=wavelength1 + dispersion[0]*C + dispersion[1]*C^2
                                   //                            + dispersion[2]*C^3;   C: channel number
                                   //                              (0..1023)
  float     opt_dens;
  TOldFlags OldFlags;
  char     FileName[8];           //     filename of spectrum
  char     backgrnd[8];
  int       gap_list[40];
  char    *comment;
  int       reg_no;
  void     *Prev, *Next;
 }
TBinaryMFC;

extern TBinaryMFC MFC_headerDrk,MFC_headerOff,MFC_header,MFC_headerInstr;
extern int MFC_format;
extern char MFC_fileInstr[MAX_STR_SHORT_LEN+1],
             MFC_fileDark[MAX_STR_SHORT_LEN+1],
             MFC_fileOffset[MAX_STR_SHORT_LEN+1],
             MFC_fileSpectra[MAX_STR_SHORT_LEN+1],
             MFC_fileMin[MAX_STR_SHORT_LEN+1];

RC MFC_LoadOffset(ENGINE_CONTEXT *pEngineContext);
RC MFC_LoadDark(ENGINE_CONTEXT *pEngineContext);
RC MFC_ReadRecord(char *fileName,TBinaryMFC *pHeaderSpe,double *spe,TBinaryMFC *pHeaderDrk,double *drk,TBinaryMFC *pHeaderOff,double *off,unsigned int mask,unsigned int maskSpec,unsigned int revertFlag);
RC MFC_ReadRecordStd(ENGINE_CONTEXT *pEngineContext,char *fileName,
                     TBinaryMFC *pHeaderSpe,double *spe,
                     TBinaryMFC *pHeaderDrk,double *drk,
                     TBinaryMFC *pHeaderOff,double *off);
INDEX MFC_SearchForCurrentFileIndex(ENGINE_CONTEXT *pEngineContext)  ;
RC   SetMFC(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   ReliMFC(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp,unsigned int mfcMask);
RC   ReliMFCStd(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);
RC   MFCBIRA_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp);
RC   MFCBIRA_Reli(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp);

RC MFC_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);


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
extern int GOME2_mus; //number of microseconds

void GOME2_ReleaseBuffers(void);

RC GOME2_Set(ENGINE_CONTEXT *pEngineContext);
RC GOME2_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,INDEX fileIndex);
RC GOME2_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);
void GOME2_get_orbit_date(int *year, int *month, int *day);

#endif

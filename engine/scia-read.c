
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
//  Module purpose    :  SCIAMACHY interface
//  Name of module    :  SCIA_Read.C
//  Program Language  :  Borland C++ 5.0 for Windows 95/NT
//  Creation date     :  19 September 2002 (HDF version)
//  Modified          :  08 December 2002 (PDS version)
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
//  FUNCTIONS
//
//  ===================
//  ALLOCATION ROUTINES
//  ===================
//
//  SCIA_ReleaseBuffers - release buffers allocated by SCIAMACHY readout routines;
//  SciaAllocateClusters - allocate buffers for the definition of clusters;
//
//  =========
//  UTILITIES
//  =========
//
//  SCIA_FromMJD2000ToYMD - convert a MJD2000 date in the equivalent YYYY/MM/DD hh:mm:ss format;
//  SciaGetStateIndex - given a record number, return the state index;
//
//  SciaNadirStates - analyze information on NADIR states;
//  SciaNadirGeolocations - retrieve geolocation data;
//  SciaReadSunRefPDS - read the sun reference spectrum from the PDS file;
//  SciaReadNadirMDSInfo - read information on Nadir measurement data set;
//  SciaReadNadirMDS - rread a NADIR measurement data set;
//
//  SCIA_SetPDS  Retrieve information on useful data sets from the PDS file and
//               load the irradiance spectrum measured at the specified channel
//
//  SCIA_ReadPDS - SCIAMACHY calibrated level 1 data read out;
//
//  REFERENCE
//
//  PDS functions use SCIAMACHY Level 1c read out routines written by Stefan Noel
//  from IFE/IUP Uni Bremen (Stefan.Noel@iup.physik.uni.bremen.de), version 0.1,
//  17 Apr 2002 (first beta release).
//
//  These routines are distributed through the WEB.
//
//  =============================
//  AUTOMATIC REFERENCE SELECTION
//  =============================
//
//  SciaSortGetIndex - get the position in sorted list of a new element (latitude, longitude or SZA);
//  SCIA_Sort - sort NADIR records on latitudes, longitudes or SZA;
//  SciaRefLat - search for spectra in the orbit file matching latitudes and SZA conditions;
//  SciaRefSza - search for spectra in the orbit file matching SZA conditions only;
//  SciaBuildRef - build a reference spectrum by averaging a set of spectra matching latitudes and SZA conditions;
//  SciaRefSelection - selection of a reference spectrum in the current orbit;
//  SciaNewRef - in automatic reference selection, search for reference spectra;
//
//  ========
//  ANALYSIS
//  ========
//
//  SCIA_LoadAnalysis - load analysis parameters depending on the irradiance spectrum;
//
//  ----------------------------------------------------------------------------

// ===============
// INCLUDE HEADERS
// ===============

#include "doas.h"

#include "bin_read.h"
#include "lv1c_struct.h"
#include "read1c_defs.h"
#include "read1c_funct.h"

// ====================
// CONSTANTS DEFINITION
// ====================

#define EPSILON                    (double)1.e-5

// No specific constants for PDS format

// =====================
// STRUCTURES DEFINITION
// =====================

// Get the definition of clusters for a given state

typedef struct _sciaClusDef
 {
  INT mdsOffset;                                                                // offset of the current measurement data set from the beginning of the file
  INT nobs;                                                                     // total number of observations in the measurement data set
  INT startPix;                                                                 // starting pixels
  INT npixels;                                                                  // number of pixels in the cluster
  INT coadd;                                                                    // coadd factor
 }
SCIA_CLUSDEF;

// Definition of NADIR states

typedef struct _sciaNadirState                                                  // information on a NADIR state
 {
  INT stateId;                                                                  // id of the state
  INT int_time;                                                                 // highest integration time (accounting for clusters)
  INT clusId;                                                                   // id of the cluster with the highest integration time
  INT nobs;                                                                     // real number of observations accounting for coadd factors
  double dsrTime;                                                               // starting dsr time
 }
SCIA_NADIR_STATE;

// Information on selected clusters

typedef struct _sciaClusters
 {
  INT clusId;                                                                   // id of the selected cluster
  float *spe,*err;                                                              // buffers to read out
  SCIA_CLUSDEF *clusDef;                                                        // get the definition of this cluster in the different states
 }
SCIA_CLUSTER;

// Automatic reference selection

typedef struct _refSelection
 {
 	INDEX  indexFile;
  INDEX  indexRecord;
  double sza;
  double latitude;
  double longitude;
  double szaDist;
  double latDist;
  double lonDist;
 }
SCIA_REF;

int SCIA_clusters[PRJCT_INSTR_SCIA_CHANNEL_MAX][2]=
 {
  {  2,  5 },
  {  8, 10 },
  { 13, 18 },
  { 22, 27 }
 };

INT SCIA_ms=0;

// ================
// STATIC VARIABLES
// ================

#define MAX_SCIA_FILES 150 // maximum number of files per day

typedef struct _sciaOrbitFiles                                                  // description of an orbit
 {
 	UCHAR sciaFileName[MAX_STR_LEN+1];                                            // the name of the file with a part of the orbit
 	info_l1c        sciaPDSInfo;                                                  // all internal information about the PDS file like data offsets etc.
  float *sciaSunRef,*sciaSunWve;                                                // the sun reference spectrum and calibration
  INDEX *sciaLatIndex,*sciaLonIndex,*sciaSzaIndex;                              // indexes of records sorted resp. by latitude, by longitude and by SZA
  SATELLITE_GEOLOC *sciaGeolocations;                                           // geolocations
  SCIA_NADIR_STATE *sciaNadirStates;                                            // NADIR states
  SCIA_CLUSTER   *sciaNadirClusters;                                            // definition of NADIR clusters to select
  INDEX           sciaNadirClustersIdx[MAX_CLUSTER];                            // get the indexes of clusters in the previous structure
  INT             sciaNadirStatesN,                                             // number of NADIR states
                  sciaNadirClustersN;                                           // number of NADIR clusters to select
  INT specNumber;
  INT rc;
 }
SCIA_ORBIT_FILE;

static SCIA_ORBIT_FILE sciaOrbitFiles[MAX_SCIA_FILES];                          // list of files for an orbit
static int sciaOrbitFilesN=0;                                                   // the total number of files for the current orbit
static INDEX sciaCurrentFileIndex=ITEM_NONE;                                    // index of the current file in the list
static INT sciaTotalRecordNumber;                                               // total number of records for an orbit
static INT sciaLoadReferenceFlag=0;

// ==========
// PROTOTYPES
// ==========

void SciaSort(INDEX indexRecord,int flag,int listSize,INDEX fileIndex);
void mds_1c_constant_getbin(FILE* unit, mds_1c_constant *var);

// ===================
// ALLOCATION ROUTINES
// ===================

// -----------------------------------------------------------------------------
// FUNCTION      SCIA_ReleaseBuffers
// -----------------------------------------------------------------------------
// PURPOSE       Release buffers allocated by SCIAMACHY readout routines
// -----------------------------------------------------------------------------

void SCIA_ReleaseBuffers(UCHAR format)
 {
  // Declarations

  SCIA_CLUSTER *pCluster;
  SCIA_ORBIT_FILE *pOrbitFile;
  INDEX sciaOrbitFileIndex;
  INDEX indexCluster;

  for (sciaOrbitFileIndex=0;sciaOrbitFileIndex<sciaOrbitFilesN;sciaOrbitFileIndex++)
   {
   	pOrbitFile=&sciaOrbitFiles[sciaOrbitFileIndex];

    // Release buffers common to both formats

    for (indexCluster=0;indexCluster<pOrbitFile->sciaNadirClustersN;indexCluster++)
     {
      pCluster=&pOrbitFile->sciaNadirClusters[indexCluster];

      if (pCluster->clusDef!=NULL)
       MEMORY_ReleaseBuffer("SCIA_ReleaseBuffers ","clusDef",pCluster->clusDef);
      if (pCluster->spe!=NULL)
       MEMORY_ReleaseBuffer("SCIA_ReleaseBuffers ","spe",pCluster->spe);
      if (pCluster->err!=NULL)
       MEMORY_ReleaseBuffer("SCIA_ReleaseBuffers ","err",pCluster->err);
     }

    if (pOrbitFile->sciaNadirStates!=NULL)
     MEMORY_ReleaseBuffer("SCIA_ReleaseBuffers ","sciaNadirStates",pOrbitFile->sciaNadirStates);
    if (pOrbitFile->sciaNadirClusters!=NULL)
     MEMORY_ReleaseBuffer("SCIA_ReleaseBuffers ","sciaNadirClusters",pOrbitFile->sciaNadirClusters);

    if (pOrbitFile->sciaGeolocations!=NULL)
     MEMORY_ReleaseBuffer("SCIA_ReleaseBuffers ","sciaGeolocations",pOrbitFile->sciaGeolocations);
    if (pOrbitFile->sciaLatIndex!=NULL)
     MEMORY_ReleaseBuffer("SCIA_ReleaseBuffers ","sciaLatIndex",pOrbitFile->sciaLatIndex);
    if (pOrbitFile->sciaLonIndex!=NULL)
     MEMORY_ReleaseBuffer("SCIA_ReleaseBuffers ","sciaLonIndex",pOrbitFile->sciaLonIndex);
    if (pOrbitFile->sciaSzaIndex!=NULL)
     MEMORY_ReleaseBuffer("SCIA_ReleaseBuffers ","sciaSzaIndex",pOrbitFile->sciaSzaIndex);

    if (pOrbitFile->sciaSunRef!=NULL)
     MEMORY_ReleaseBuffer("SCIA_ReleaseBuffers ","sciaSunRef",pOrbitFile->sciaSunRef);
    if (pOrbitFile->sciaSunWve!=NULL)
     MEMORY_ReleaseBuffer("SCIA_ReleaseBuffers ","sciaSunWve",pOrbitFile->sciaSunWve);

    // PDS format

    if (format==PRJCT_INSTR_FORMAT_SCIA_PDS)
     closeL1c(&pOrbitFile->sciaPDSInfo);                                        // Close the current PDS file
   }

  for (sciaOrbitFileIndex=0;sciaOrbitFileIndex<MAX_SCIA_FILES;sciaOrbitFileIndex++)
  	memset(&sciaOrbitFiles[sciaOrbitFileIndex],0,sizeof(SCIA_ORBIT_FILE));

  sciaOrbitFilesN=0;
  sciaCurrentFileIndex=ITEM_NONE;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaAllocateClusters
// -----------------------------------------------------------------------------
// PURPOSE       Allocate buffers for the definition of clusters
//
// INPUT/OUTPUT  pEngineContext    interface for file operations
// INPUT         clustersList the list of clusters present in the file
//               nClusters    the number of clusters present in the file
//               nStates      the number of states
//               fileIndex    index of the file for the current orbit
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO    otherwise.
// -----------------------------------------------------------------------------

RC SciaAllocateClusters(ENGINE_CONTEXT *pEngineContext,int *clustersList,int nClusters,int nStates,INDEX fileIndex)
 {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit file
  PRJCT_INSTRUMENTAL *pInstr;                                                   // pointer to instrumental options of projects properties
  INDEX indexCluster;                                                           // browse used clusters
  SCIA_CLUSTER *pCluster;                                                       // pointer to the current cluster
  int userCluster[MAX_CLUSTER];                                                 // the clusters user selection
  RC rc;                                                                        // return code

  // Initializations

//  DEBUG_Print(DOAS_logFile,"Enter SciaAllocateClusters\n");

  pOrbitFile=&sciaOrbitFiles[fileIndex];
  pInstr=&pEngineContext->project.instrumental;
  memset(userCluster,0,sizeof(int)*MAX_CLUSTER);
  memset(pOrbitFile->sciaNadirClustersIdx,ITEM_NONE,sizeof(int)*MAX_CLUSTER);   // !!!
  rc=ERROR_ID_NO;

  for (indexCluster=SCIA_clusters[pInstr->scia.sciaChannel][0];indexCluster<=SCIA_clusters[pInstr->scia.sciaChannel][1];indexCluster++)
   if (pInstr->scia.sciaCluster[indexCluster-SCIA_clusters[pInstr->scia.sciaChannel][0]])
    userCluster[indexCluster-1]=1;

  // Allocate a buffer for the definition of clusters

  if ((pOrbitFile->sciaNadirClusters=(SCIA_CLUSTER *)MEMORY_AllocBuffer("SciaAllocateClusters ","sciaNadirClusters",MAX_CLUSTER,sizeof(SCIA_CLUSTER),0,MEMORY_TYPE_STRUCT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // Browse clusters present in the file; keep only those requested by user

    for (indexCluster=0;(indexCluster<nClusters) && !rc;indexCluster++)
     if (userCluster[clustersList[indexCluster]])
      {
       pCluster=&pOrbitFile->sciaNadirClusters[pOrbitFile->sciaNadirClustersN];

       pCluster->clusId=clustersList[indexCluster];
       pCluster->spe=pCluster->err=NULL;

       if ((pCluster->clusDef=(SCIA_CLUSDEF *)MEMORY_AllocBuffer("SciaAllocateClusters ","clusDef",nStates,sizeof(SCIA_CLUSDEF),0,MEMORY_TYPE_STRUCT))==NULL)
        rc=ERROR_ID_ALLOC;
       else
        {
         pOrbitFile->sciaNadirClustersIdx[clustersList[indexCluster]]=pOrbitFile->sciaNadirClustersN;
         pOrbitFile->sciaNadirClustersN++;
        }
      }

    // If requested clusters are not in the file, display error

    if (!rc && !pOrbitFile->sciaNadirClustersN)
     rc=ERROR_SetLast("SciaAllocateClusters",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
   }

  // Return

//  DEBUG_Print(DOAS_logFile,"End SciaAllocateClusters %d\n",rc);

  return rc;
 }

// =========
// UTILITIES
// =========

// -----------------------------------------------------------------------------
// FUNCTION      SCIA_FromMJD2000ToYMD
// -----------------------------------------------------------------------------
// PURPOSE       Convert a MJD2000 date in the equivalent YYYY/MM/DD hh:mm:ss format
//
// INPUT         mjd          the date in MJD2000 format
// INPUT/OUTPUT  pDate,pTime  pointers to resp. date and time in usual format
// -----------------------------------------------------------------------------

void SCIA_FromMJD2000ToYMD(double mjd,SHORT_DATE *pDate,struct time *pTime)
 {
  // Declarations

  int year;
  double sumDays,nDaysInYear,nDaysInMonth;
  int daysInMonth[]={31,28,31,30,31,30,31,31,30,31,30,31};

  // Initializations

  memset(pDate,0,sizeof(SHORT_DATE));
  memset(pTime,0,sizeof(struct time));

  // get the number of years since 2000

  for (year=2000,sumDays=(double)0.,nDaysInYear=(double)366.;
       sumDays+nDaysInYear<mjd;)
   {
    year++;
    sumDays+=nDaysInYear;
    nDaysInYear=((year%4)==0)?(double)366.:(double)365.;
   }

  // Get date from the year and the calendar day

  pDate->da_year=(short)year;
  pDate->da_mon=(char)ZEN_FNCaljmon(year,(int)floor(mjd-sumDays+1.));
  pDate->da_day=(char)ZEN_FNCaljday(year,(int)floor(mjd-sumDays+1.));

  // Get time

  mjd-=(double)floor(mjd);
  pTime->ti_hour=(char)(floor(mjd*24.));
  mjd=mjd*24.-pTime->ti_hour;
  pTime->ti_min=(char)(floor(mjd*60.));
  mjd=mjd*60.-pTime->ti_min;
  pTime->ti_sec=(char)(floor(mjd*60.));
  mjd=mjd*60.-pTime->ti_sec;

  // Round the number of milliseconds and correct date and time if the rounded number of milliseconds is exactly 1000

  SCIA_ms=(int)floor(mjd*1000+0.5);

  if (SCIA_ms>=1000)
   {
   	SCIA_ms%=1000;
   	if (++pTime->ti_sec>=(UCHAR)60)
   	 {
   	 	pTime->ti_sec%=(UCHAR)60;
   	 	if (++pTime->ti_min>=(UCHAR)60)
     	 {
     	 	pTime->ti_min%=(UCHAR)60;
     	 	if (++pTime->ti_hour>=(UCHAR)24)
     	 	 {
     	 	 	pTime->ti_hour%=(UCHAR)24;
     	 	 	nDaysInMonth=daysInMonth[pDate->da_mon-1];
     	 	 	if (((pDate->da_year%4)==0) && (pDate->da_mon==(UCHAR)2))
     	 	 	 nDaysInMonth++;
     	 	 	if (++pDate->da_day>(UCHAR)nDaysInMonth)
     	 	 	 {
     	 	 	 	pDate->da_day=1;
     	 	 	 	if (++pDate->da_mon>(UCHAR)12)
     	 	 	 	 {
     	 	 	 	 	pDate->da_mon=(UCHAR)1;
     	 	 	 	 	pDate->da_year++;
     	 	 	 	 }
     	 	 	 }
     	 	 }
   	 	 }
   	 }
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaGetStateIndex
// -----------------------------------------------------------------------------
// PURPOSE       Given a record number, return the state index.
//
// INPUT         recordNo       the requested record number;
//               fileIndex      index of the file for the current orbit
//
// OUTPUT        pObs           the total number of observations covered by
//                              previous states
//
// RETURN        the index of the state
// -----------------------------------------------------------------------------

INDEX SciaGetStateIndex(int recordNo,INT *pObs,INDEX fileIndex)
 {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit file
  INDEX indexState;                                                             // browse states
  INT sumObs;                                                                   // accumulate the number of observations in the different states

  // Initialization

  pOrbitFile=&sciaOrbitFiles[fileIndex];

  // Search for the state

  for (indexState=sumObs=0;indexState<pOrbitFile->sciaNadirStatesN;sumObs+=pOrbitFile->sciaNadirStates[indexState].nobs,indexState++)
   if (sumObs+pOrbitFile->sciaNadirStates[indexState].nobs>recordNo)
    break;

  // Return

  *pObs=sumObs;

  return indexState;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaNadirStates
// -----------------------------------------------------------------------------
// PURPOSE       Analyze information on NADIR states
//
// INPUT         fileIndex    index of the current file
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_FILE_EMPTY if no record is found in the file;
//               ERROR_ID_NO    otherwise.
// -----------------------------------------------------------------------------

RC SciaNadirStates(ENGINE_CONTEXT *pEngineContext,INDEX fileIndex)
 {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  ADS_STATES *pAdsState;                                                        // pointer to the current state (from ADS of PDS file)
  SCIA_NADIR_STATE *pStateInfo;                                                 // pointer to the current state in this program
  SCIA_CLUSTER *pCluster;                                                       // pointer to the definition of the current cluster
  SCIA_CLUSDEF *pClusDef;                                                       // pointer to the definition of the current cluster
  INDEX indexState,indexCluster;                                                // browse resp. NADIR states and clusters
  INT   maxPix[MAX_CLUSTER],                                                    // for the set of selected clusters to read, get the maximum number of pixels and
        maxCoadd[MAX_CLUSTER];                                                  // the maximum coadd factor in order to further determine the maximum size of vectors to allocate
  RC rc;                                                                        // return code

  // DEBUG

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("SciaNadirStates",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  pOrbitFile=&sciaOrbitFiles[fileIndex];
  pOrbitFile->sciaNadirStatesN=pOrbitFile->sciaPDSInfo.n_states[NADIR];         // number of NADIR states
  pOrbitFile->specNumber=0;

  memset(maxPix,0,sizeof(INT)*MAX_CLUSTER);
  memset(maxCoadd,0,sizeof(INT)*MAX_CLUSTER);

  rc=ERROR_ID_NO;

  DEBUG_Print("Number of states : %d\n",pOrbitFile->sciaNadirStatesN);
  DEBUG_Print("Number of selected clusters : %d\n",pOrbitFile->sciaNadirClustersN);
  for (indexCluster=0;indexCluster<pOrbitFile->sciaNadirClustersN;indexCluster++)
   DEBUG_Print("    Cluster %2d\n",pOrbitFile->sciaNadirClusters[indexCluster].clusId+1);

  // Allocate a buffer for NADIR states

  if ((pOrbitFile->sciaNadirStates=(SCIA_NADIR_STATE *)MEMORY_AllocBuffer("SciaNadirStates ","sciaNadirStates",pOrbitFile->sciaNadirStatesN,sizeof(SCIA_NADIR_STATE),0,MEMORY_TYPE_STRUCT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // Browse states

    for (indexState=0;indexState<pOrbitFile->sciaNadirStatesN;indexState++)
     {
      pAdsState=&pOrbitFile->sciaPDSInfo.ads_states[pOrbitFile->sciaPDSInfo.idx_states[NADIR][indexState]];
      pStateInfo=&pOrbitFile->sciaNadirStates[indexState];
      pStateInfo->stateId=pAdsState->state_id;
      pStateInfo->nobs=99999;

      // Search for the cluster with the highest integration time

      for (indexCluster=0;indexCluster<pOrbitFile->sciaNadirClustersN;indexCluster++)
       {
        pCluster=&pOrbitFile->sciaNadirClusters[indexCluster];
        pClusDef=pCluster->clusDef+indexState;

        if (pClusDef->npixels>maxPix[indexCluster])
         maxPix[indexCluster]=pClusDef->npixels;
        if (pClusDef->nobs<pStateInfo->nobs)
         {
         	// given a set of clusters, the total number of records is determined with the lowest spatial resolution

          pStateInfo->nobs=pClusDef->nobs;
          pStateInfo->clusId=pCluster->clusId;
         }
       }

      DEBUG_Print("State index %-2d id %d nobs %-3d duration %d clus id %d clus duration %d\n",indexState+1,pStateInfo->stateId,pStateInfo->nobs,pAdsState->duration,pStateInfo->clusId,pAdsState->Clcon[pStateInfo->clusId].int_time);

      pOrbitFile->specNumber+=pStateInfo->nobs;
      pStateInfo->dsrTime=(double)pAdsState->StartTime.days+1.*(pAdsState->StartTime.secnd+pAdsState->StartTime.musec/1000000.)/86400.;
      pStateInfo->int_time=pAdsState->Clcon[pStateInfo->clusId].int_time;

      for (indexCluster=0;indexCluster<pOrbitFile->sciaNadirClustersN;indexCluster++)
       {
        pCluster=&pOrbitFile->sciaNadirClusters[indexCluster];
        pClusDef=pCluster->clusDef+indexState;

        pClusDef->startPix=pAdsState->Clcon[pCluster->clusId].pixel_nr;
        pClusDef->coadd=pClusDef->nobs/pStateInfo->nobs;

        DEBUG_Print("State %d Nobs %d Npixels %d Coadd %d\n",indexState+1,pClusDef->nobs,pClusDef->npixels,pClusDef->coadd);

        // Calculate the maximum size of vectors to allocate

        if (pClusDef->coadd>maxCoadd[indexCluster])
         maxCoadd[indexCluster]=pClusDef->coadd;
       }
     }

    for (indexCluster=0;(indexCluster<pOrbitFile->sciaNadirClustersN) && !rc;indexCluster++)
     {
      pCluster=&pOrbitFile->sciaNadirClusters[indexCluster];

      // Buffers allocation for MDS

      if (((pCluster->spe=(float *)MEMORY_AllocBuffer("SciaNadirStates ","spe",maxPix[indexCluster]*maxCoadd[indexCluster],sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
          ((pCluster->err=(float *)MEMORY_AllocBuffer("SciaNadirStates ","err",maxPix[indexCluster]*maxCoadd[indexCluster],sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL))

       rc=ERROR_ID_ALLOC;
     }

    if (!pOrbitFile->specNumber)
     rc=ERROR_SetLast("SciaNadirStates",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
   }

  // DEBUG

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("SciaNadirStates",rc);
  #endif

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaNadirGeolocations
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve geolocation data
//
// INPUT         fileIndex    index of the current file
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO    otherwise.
// -----------------------------------------------------------------------------

RC SciaNadirGeolocations(ENGINE_CONTEXT *pEngineContext,INDEX fileIndex)
 {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  GeoN *sciaGeoloc;                                                             // geolocations in the PDS format
  SATELLITE_GEOLOC *pSciaGeoloc;                                                // geolocations in the WinDOAS format
  SCIA_NADIR_STATE *pState;                                                     // pointer to the current state
  SCIA_CLUSDEF *pClusDef;                                                       // pointer to the definition of the cluster with the highest integration time in the current state
  ULONG offset;                                                                 // offset of geolocation data from the beginning of file
  INDEX indexRecord;                                                            // browse records
  INDEX indexObs;                                                               // browse observations in the current NADIR MDS
  INDEX indexState;                                                             // browse states
  RC rc;                                                                        // return code

  // Initializations

//  DEBUG_Print(DOAS_logFile,"Enter SciaNadirGeolocations\n");

  pOrbitFile=&sciaOrbitFiles[fileIndex];
  indexRecord=0;
  rc=ERROR_ID_NO;

  // Buffers allocation

  if (((sciaGeoloc=(GeoN *)MEMORY_AllocBuffer("SciaNadirGeolocations ","sciaGeoloc",pOrbitFile->specNumber,sizeof(GeoN),0,MEMORY_TYPE_STRUCT))==NULL) ||
      ((pOrbitFile->sciaGeolocations=(SATELLITE_GEOLOC *)MEMORY_AllocBuffer("SciaNadirGeolocations ","pOrbitFile->sciaGeolocations",pOrbitFile->specNumber,sizeof(SATELLITE_GEOLOC),0,MEMORY_TYPE_STRUCT))==NULL) ||
      ((pOrbitFile->sciaLatIndex=(INDEX *)MEMORY_AllocBuffer("SciaNadirGeolocations ","sciaLatIndex",pOrbitFile->specNumber,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
      ((pOrbitFile->sciaLonIndex=(INDEX *)MEMORY_AllocBuffer("SciaNadirGeolocations ","sciaLonIndex",pOrbitFile->specNumber,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
      ((pOrbitFile->sciaSzaIndex=(INDEX *)MEMORY_AllocBuffer("SciaNadirGeolocations ","sciaSzaIndex",pOrbitFile->specNumber,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL))

   rc=ERROR_ID_ALLOC;

  else

   for (indexState=0;indexState<pOrbitFile->sciaNadirStatesN;indexState++)
    {
     // for the current state, get the cluster with the highest integration time

     pState=&pOrbitFile->sciaNadirStates[indexState];
     pClusDef=&pOrbitFile->sciaNadirClusters[pOrbitFile->sciaNadirClustersIdx[pState->clusId]].clusDef[indexState];

     // calculate offset

     offset=pClusDef->mdsOffset+                                                // beginning of the MDS
            4*sizeof(int)+                                                      // StartTime+length
            2*sizeof(char)+                                                     // quality+unit_flag
            1*sizeof(float)+                                                    // orbit_phase
            5*sizeof(short)+                                                    // category+state_id+cluster_id+nobs+npixels
           (sizeof(USHORT)+                                                     // pixels id
          2*sizeof(float))*pClusDef->npixels+                                   // wavelength+wavelength errors
          2*pClusDef->npixels*pClusDef->nobs*sizeof(float);                     // signal+error

     // Read geolocations

     fseek(pOrbitFile->sciaPDSInfo.FILE_l1c,offset,SEEK_SET);
  	  GeoN_array_getbin(pOrbitFile->sciaPDSInfo.FILE_l1c,sciaGeoloc,pClusDef->nobs);

  	  // Browse observations

  	  for (indexObs=0;indexObs<pClusDef->nobs;indexObs++,indexRecord++)
  	   {
  	    pSciaGeoloc=&pOrbitFile->sciaGeolocations[indexRecord];

  	    // longitudes at the 4 corners of the pixel

  	    pSciaGeoloc->lonCorners[0]=(double)sciaGeoloc[indexObs].corner_coord[0].lon*1e-6;
  	    pSciaGeoloc->lonCorners[1]=(double)sciaGeoloc[indexObs].corner_coord[1].lon*1e-6;
  	    pSciaGeoloc->lonCorners[2]=(double)sciaGeoloc[indexObs].corner_coord[2].lon*1e-6;
  	    pSciaGeoloc->lonCorners[3]=(double)sciaGeoloc[indexObs].corner_coord[3].lon*1e-6;

  	    // latitudes at the 4 corners of the pixel

  	    pSciaGeoloc->latCorners[0]=(double)sciaGeoloc[indexObs].corner_coord[0].lat*1e-6;
  	    pSciaGeoloc->latCorners[1]=(double)sciaGeoloc[indexObs].corner_coord[1].lat*1e-6;
  	    pSciaGeoloc->latCorners[2]=(double)sciaGeoloc[indexObs].corner_coord[2].lat*1e-6;
  	    pSciaGeoloc->latCorners[3]=(double)sciaGeoloc[indexObs].corner_coord[3].lat*1e-6;

  	    // longitude and latitude at pixel centre

  	    pSciaGeoloc->lonCenter=(double)sciaGeoloc[indexObs].centre_coord.lon*1e-6;
  	    pSciaGeoloc->latCenter=(double)sciaGeoloc[indexObs].centre_coord.lat*1e-6;

  	    // angles

  	    memcpy(pSciaGeoloc->solZen,sciaGeoloc[indexObs].sza_toa,sizeof(float)*3);// TOA solar zenith angles
  	    memcpy(pSciaGeoloc->solAzi,sciaGeoloc[indexObs].saa_toa,sizeof(float)*3);// TOA solar azimuth angles
  	    memcpy(pSciaGeoloc->losZen,sciaGeoloc[indexObs].los_zen,sizeof(float)*3);// LOS zenith angles
  	    memcpy(pSciaGeoloc->losAzi,sciaGeoloc[indexObs].los_azi,sizeof(float)*3);// LOS azimuth angles

       SciaSort(indexRecord,0,indexRecord,fileIndex);                           // sort latitudes
       SciaSort(indexRecord,1,indexRecord,fileIndex);                           // sort longitudes
       SciaSort(indexRecord,2,indexRecord,fileIndex);                           // sort SZA

       // Miscellaneous

       pSciaGeoloc->earthRadius=sciaGeoloc[indexObs].earth_radius;              // earth radius useful to convert satellite angles to TOA angles
       pSciaGeoloc->satHeight=sciaGeoloc[indexObs].sat_height;                  // satellite height useful to convert satellite angles to TOA angles
  	   }
    }

  // Release allocated buffers

  if (sciaGeoloc!=NULL)
   MEMORY_ReleaseBuffer("SciaNadirGeolocations ","sciaGeoloc",sciaGeoloc);

//      if (rc!=ERROR_ID_NO)
//       DEBUG_Print(DOAS_logFile,"SciaNadirGeolocations failed for file %s (rc=%d)",pOrbitFile->sciaFileName,rc);

  // Return

//  DEBUG_Print(DOAS_logFile,"End SciaNadirGeolocations\n");

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaReadSunRefPDS
// -----------------------------------------------------------------------------
// PURPOSE       Read the sun reference spectrum from the PDS file
//
// INPUT         fileIndex    index of the current file
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed
//               ERROR_ID_NO    otherwise.
// -----------------------------------------------------------------------------

RC SciaReadSunRefPDS(ENGINE_CONTEXT *pEngineContext,INDEX fileIndex)
 {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  ULONG offset;                                                                 // offset of reference spectra from the beginning of the PDS file
  INDEX indexRef;                                                               // browse reference spectra in the file
  CHAR refId[2];                                                                // id of the reference spectra
  FILE *fp;                                                                     // pointer to the current file
  INDEX i;                                                                      // browse positions in calibration and reference vectors
  double version;
  int dlrFlag;
  RC rc;                                                                        // return code

  // Initializations

//  DEBUG_Print(DOAS_logFile,"Begin SciaReadSunRefPDS\n");

  pOrbitFile=&sciaOrbitFiles[fileIndex];
  fp=pOrbitFile->sciaPDSInfo.FILE_l1c;
  dlrFlag=0;
  rc=ERROR_ID_NO;

  // Buffers allocation

  if (((pOrbitFile->sciaSunRef=(float *)MEMORY_AllocBuffer("SciaReadSunRefPDS ","sciaSunRef",NDET,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL) ||
      ((pOrbitFile->sciaSunWve=(float *)MEMORY_AllocBuffer("SciaReadSunRefPDS ","sciaSunWve",NDET,sizeof(float),0,MEMORY_TYPE_FLOAT))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Browse reference spectra

    for (indexRef=0,offset=pOrbitFile->sciaPDSInfo.sun_ref.offset;
         indexRef<(int)pOrbitFile->sciaPDSInfo.sun_ref.num_dsr;
         indexRef++,offset+=sizeof(gads_sun_ref))
     {
      // Get the reference id (should be D1)

      fseek(fp,offset,SEEK_SET);
      fread(&refId,2,1,fp);

      if ((strlen(pOrbitFile->sciaPDSInfo.user_file_info.software_ver)>4) &&
          !strnicmp(pOrbitFile->sciaPDSInfo.user_file_info.software_ver,"SCIA",4))
       sscanf(pOrbitFile->sciaPDSInfo.user_file_info.software_ver,"SCIA/%lf",&version);
      else
       {
        sscanf(pOrbitFile->sciaPDSInfo.user_file_info.software_ver,"%lf",&version);
        dlrFlag=1;
       }

      if ((refId[0]==toupper(pEngineContext->project.instrumental.scia.sciaReference[0])) && (refId[1]==pEngineContext->project.instrumental.scia.sciaReference[1]))
       {
        // Read the wavelength calibration

        fseek(fp,offset+2+(pEngineContext->project.instrumental.scia.sciaChannel)*NDET*sizeof(float),SEEK_SET);
        fread(pOrbitFile->sciaSunWve,sizeof(float)*NDET,1,fp);

        // Read the reference spectrum

        fseek(fp,offset+2+(pEngineContext->project.instrumental.scia.sciaChannel+8)*NDET*sizeof(float),SEEK_SET); /* 8*NDET + (channel-1)*NDET = (channel+7)*NDET */
        fread(pOrbitFile->sciaSunRef,sizeof(float)*NDET,1,fp);

        #if defined(__LITTLE_ENDIAN__)
        for (i=0;i<NDET;i++)
         {
          swap_bytes_float((unsigned char *)((float *)&pOrbitFile->sciaSunWve[i]));
          swap_bytes_float((unsigned char *)((float *)&pOrbitFile->sciaSunRef[i]));
         }
        #endif

        break;
       }
     }

    if ((indexRef>=(int)pOrbitFile->sciaPDSInfo.sun_ref.num_dsr) ||
       ((fabs(pOrbitFile->sciaSunWve[0]-999.)<(double)1.) && (fabs(pOrbitFile->sciaSunRef[0]-999.)<(double)1.)))
     rc=ERROR_SetLast("SciaReadSunRefPDS",ERROR_TYPE_WARNING,ERROR_ID_PDS,"NO_SUN_REF (2)",pEngineContext->fileInfo.fileName);
    else
     {
      for (i=0;i<NDET;i++)
       pEngineContext->buffers.lembda[i]=(double)(((float *)pOrbitFile->sciaSunWve)[i]);

      for (i=0;i<NDET;i++)
       pEngineContext->buffers.irrad[i]=(double)(((float *)pOrbitFile->sciaSunRef)[i]);
     }
   }

  // Return

//  DEBUG_Print(DOAS_logFile,"End SciaReadSunRefPDS %d\n",rc);

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaReadNadirMDSInfo
// -----------------------------------------------------------------------------
// PURPOSE       Read information on Nadir measurement data set
//
// INPUT         fileIndex    index of the current file
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed
//               ERROR_ID_NO    otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC SciaReadNadirMDSInfo(ENGINE_CONTEXT *pEngineContext,INDEX fileIndex)
 {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  mds_1c_constant mds;                                                          // information on measurement data set
  SCIA_CLUSTER *pCluster;                                                       // pointer to the current cluster
  SCIA_CLUSDEF *pClusDef;                                                       // pointer to the definition of the current cluster
  INDEX indexNadirMDS,                                                          // browse NADIR measurement data sets
        indexCluster,                                                           // browse clusters to account for
        indexState;                                                             // index of the current state
  ULONG offset;                                                                 // offset in file
  RC rc;

  // DEBUG                                                                        // return code

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("SciaReadNadirMDSInfo",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  pOrbitFile=&sciaOrbitFiles[fileIndex];
  rc=ERROR_ID_NO;

  if (!rc)
   {
   	DEBUG_Print("Number of MDS : %d %d\n",pOrbitFile->sciaPDSInfo.nadir.num_dsr,pOrbitFile->sciaPDSInfo.max_cluster_ids);

    // Browse NADIR MDS

    for (offset=pOrbitFile->sciaPDSInfo.mds_offset[NADIR],indexNadirMDS=0;
	     indexNadirMDS<(int)pOrbitFile->sciaPDSInfo.nadir.num_dsr;indexNadirMDS++)
     {
      // Read the MDS offset

      rc=fseek(pOrbitFile->sciaPDSInfo.FILE_l1c,offset,SEEK_SET);

      mds_1c_constant_getbin(pOrbitFile->sciaPDSInfo.FILE_l1c,&mds);

      if ((mds.category==1) && ((indexCluster=pOrbitFile->sciaNadirClustersIdx[mds.cluster_id-1])!=ITEM_NONE))
       {
        // get the index of the state and the cluster

        pCluster=&pOrbitFile->sciaNadirClusters[indexCluster];
        indexState=indexNadirMDS/min(pOrbitFile->sciaPDSInfo.max_cluster_ids,56);
        pClusDef=pCluster->clusDef+indexState;

        // Complete the definition of the cluster

        DEBUG_Print("indexState %d indexCluster %d nobs %d npixels %d offset %ld\n",indexState,indexCluster,mds.nobs,mds.npixels,offset);

        pClusDef->nobs=mds.nobs;
        pClusDef->npixels=mds.npixels;
        pClusDef->mdsOffset=offset;

        pEngineContext->recordInfo.scia.qualityFlag=mds.quality;
       }

      offset+=mds.length;
     }
   }

  // DEBUG                                                                        // return code

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("SciaReadNadirMDSInfo",rc);
  #endif

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaReadNadirMDS
// -----------------------------------------------------------------------------
// PURPOSE       Read a NADIR measurement data set
//
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// INPUT         indexState   index of state for the NADIR measurement data set to read out
//               indexRecord  index of record in the current state
//               fileIndex    index of the current file
// -----------------------------------------------------------------------------

RC SciaReadNadirMDS(ENGINE_CONTEXT *pEngineContext,INDEX indexState,INDEX indexRecord,INDEX fileIndex)
 {
  // Declarations

  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  ULONG offset;                                                                 // offset from the beginning of file for data to read
  SCIA_CLUSTER *pCluster;                                                       // pointer to the current cluster
  SCIA_CLUSDEF *pClusDef;                                                       // pointer to the definition of the cluster in the current state
  INDEX indexCluster;                                                           // browse cluster to read out
  INDEX i,j;                                                                    // browse position in spectra
  FILE *fp;                                                                     // pointer to the PDS file

  // Initializations

  pBuffers=&pEngineContext->buffers;
  pOrbitFile=&sciaOrbitFiles[fileIndex];
  fp=pOrbitFile->sciaPDSInfo.FILE_l1c;

  for (i=0;i<NDET;i++)
   pBuffers->spectrum[i]=pBuffers->sigmaSpec[i]=(double)0.;

  for (indexCluster=0;indexCluster<pOrbitFile->sciaNadirClustersN;indexCluster++)
   {
    // Get the definition of the current cluster

    pCluster=&pOrbitFile->sciaNadirClusters[indexCluster];
    pClusDef=&pCluster->clusDef[indexState];

    // bypass MDS header (see mds_1c_constant_getbin to understand the calcul of the offset)

    offset=pClusDef->mdsOffset+                                                 // beginning of the MDS
           4*sizeof(int)+                                                       // StartTime+length
           2*sizeof(char)+                                                      // quality+unit_flag
           1*sizeof(float)+                                                     // orbit_phase
           5*sizeof(short)+                                                     // category+state_id+cluster_id+nobs+npixels

    // don't need pixels id and wavelength+wavelength errors

          (sizeof(USHORT)+                                                      // pixels id
         2*sizeof(float))*pClusDef->npixels;                                    // wavelength+wavelength errors

    // Spectra read out

    fseek(fp,offset+(pClusDef->npixels*pClusDef->coadd*indexRecord)*sizeof(float),SEEK_SET);
    fread(pCluster->spe,sizeof(float)*pClusDef->npixels*pClusDef->coadd,1,fp);

    offset+=pClusDef->npixels*pClusDef->nobs*sizeof(float);

    // Spectra errors read out

    fseek(fp,offset+(pClusDef->npixels*pClusDef->coadd*indexRecord)*sizeof(float),SEEK_SET);
    fread(pCluster->err,sizeof(float)*pClusDef->npixels*pClusDef->coadd,1,fp);

    #if defined(__LITTLE_ENDIAN__)
    for (i=0;i<pClusDef->npixels*pClusDef->coadd;i++)
     {
      swap_bytes_float((unsigned char *)&pCluster->spe[i]);
      swap_bytes_float((unsigned char *)&pCluster->err[i]);
     }
    #endif

    // Coadd observations

    for (j=0;j<pClusDef->coadd;j++)
     for (i=0;i<pClusDef->npixels;i++)
      {
       pBuffers->spectrum[i+pClusDef->startPix]+=(double)pCluster->spe[pClusDef->npixels*j+i];
       pBuffers->sigmaSpec[i+pClusDef->startPix]+=(double)pCluster->err[pClusDef->npixels*j+i]*pCluster->err[pClusDef->npixels*j+i];
      }

    if (pClusDef->coadd!=0)
     for (i=0;i<pClusDef->npixels;i++)
      {
       pBuffers->spectrum[i+pClusDef->startPix]/=(double)pClusDef->coadd;
       if (fabs(pBuffers->sigmaSpec[i+pClusDef->startPix])>(double)1.e-15)
        pBuffers->sigmaSpec[i+pClusDef->startPix]=(double)sqrt(pBuffers->sigmaSpec[i+pClusDef->startPix])/(double)pClusDef->coadd;
      }
   }

  return 0;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SCIA_SetPDS
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve information on useful data sets from the PDS file and
//               load the irradiance spectrum measured at the specified channel
//
// INPUT/OUTPUT  pEngineContext interface for file operations
//
// RETURN        ERROR_ID_FILE_NOT_FOUND  if the file is not found;
//               ERROR_ID_FILE_EMPTY      if the file is empty;
//               ERROR_ID_ALLOC           if allocation of a buffer failed;
//               ERROR_ID_PDS             if one of the PDS functions failed
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC SCIA_SetPDS(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  UCHAR filePath[MAX_STR_SHORT_LEN+1];
  UCHAR fileFilter[MAX_STR_SHORT_LEN+1];
  #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
  WIN32_FIND_DATA fileInfo;                                                     // structure returned by FindFirstFile and FindNextFile APIs
  HANDLE hDir;                                                                  // handle to use with by FindFirstFile and FindNextFile APIs
  #else
  struct dirent *fileInfo;
  DIR *hDir;
  #endif
  INDEX indexFile;
  UCHAR *ptr,*ptrOld;
  INT oldCurrentIndex;
  UCHAR *_nList[10];
  INT _n;
  RC rc;                                                                        // return code

  // DEBUG

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("SCIA_SetPDS",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  _n=0;
  sciaLoadReferenceFlag=0;
  ANALYSE_oldLatitude=(double)99999.;
  pEngineContext->recordNumber=0;
  oldCurrentIndex=sciaCurrentFileIndex;
  sciaCurrentFileIndex=ITEM_NONE;
  pOrbitFile=NULL;
  rc=ERROR_ID_NO;

  // In automatic reference selection, the file has maybe already loaded

  if ((THRD_id==THREAD_TYPE_ANALYSIS) && ANALYSE_refSelectionFlag)
   {
    // Close the previous files

    if (sciaOrbitFilesN && (oldCurrentIndex!=ITEM_NONE) && (oldCurrentIndex<sciaOrbitFilesN) && (sciaOrbitFiles[oldCurrentIndex].sciaPDSInfo.FILE_l1c!=NULL))
     {
      fclose(sciaOrbitFiles[oldCurrentIndex].sciaPDSInfo.FILE_l1c);
      sciaOrbitFiles[oldCurrentIndex].sciaPDSInfo.FILE_l1c=NULL;
     }

    for (indexFile=0;indexFile<sciaOrbitFilesN;indexFile++)
     if ((strlen(pEngineContext->fileInfo.fileName)==strlen(sciaOrbitFiles[indexFile].sciaFileName)) &&
         !STD_Stricmp(pEngineContext->fileInfo.fileName,sciaOrbitFiles[indexFile].sciaFileName))
      break;

    if (indexFile<sciaOrbitFilesN)
     sciaCurrentFileIndex=indexFile;
   }

  if (sciaCurrentFileIndex==ITEM_NONE)
   {
   	// Release old buffers

   	SCIA_ReleaseBuffers(pEngineContext->project.instrumental.readOutFormat);

   	// Get the number of files to load

   	if ((THRD_id==THREAD_TYPE_ANALYSIS) && ANALYSE_refSelectionFlag)
    	{
    		sciaLoadReferenceFlag=1;

    		// Get file path

     	strcpy(filePath,pEngineContext->fileInfo.fileName);

     	if ((ptr=strrchr(filePath,PATH_SEP))==NULL)
    	 	strcpy(filePath,".");
   	 else
    	  *ptr=0;

   	 	// Build file filter

   	 	strcpy(fileFilter,pEngineContext->fileInfo.fileName);
   	 	if ((ptr=strrchr(fileFilter,PATH_SEP))==NULL)
   	 	 ptr=fileFilter;

   	 	for (ptrOld=ptr,_n=0;(((_nList[_n]=strchr(ptrOld+1,'_'))!=NULL) && (_n<10));ptrOld=_nList[_n],_n++);

   	 	if (_n<8) // it's a SCIAMACHY file
       {
     	  sciaOrbitFilesN=1;
     	  strcpy(sciaOrbitFiles[0].sciaFileName,pEngineContext->fileInfo.fileName);
       }
      else
       {
        if (ANALYSE_lonSelectionFlag)
         {
          if (!strlen(pEngineContext->project.instrumental.fileExt))
           strcpy(ptr+1,"*.N1.child");
          else
           sprintf(ptr+1,"*.%s",pEngineContext->project.instrumental.fileExt);
         }
        else
         {
          memset(ptr+1,'?',(_nList[5]-ptr)-1);
          if (!strlen(pEngineContext->project.instrumental.fileExt))
   	 	     sprintf(_nList[7]+1,"*.N1.child");
   	 	    else
   	 	     sprintf(_nList[7]+1,"*.%s",pEngineContext->project.instrumental.fileExt);
   	 	   }

        // Search for files of the same orbit

        #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_

        for (hDir=FindFirstFile(fileFilter,&fileInfo),rc=1;
            (hDir!=INVALID_HANDLE_VALUE) && (rc!=0) && (sciaOrbitFilesN<MAX_SCIA_FILES);rc=FindNextFile(hDir,&fileInfo))

         if ((fileInfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==0)
          {
          	sprintf(sciaOrbitFiles[sciaOrbitFilesN].sciaFileName,"%s%c%s",filePath,PATH_SEP,fileInfo.cFileName);
           sciaOrbitFilesN++;
          }

        // Close handle

        if (hDir!=NULL)
         FindClose(hDir);

        #else

        for (hDir=opendir(fileFilter);(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL);)
          {
            sprintf(sciaOrbitFiles[sciaOrbitFilesN].sciaFileName,"%s/%s",filePath,fileInfo->d_name);
            if ( STD_IsDir(sciaOrbitFiles[sciaOrbitFilesN].sciaFileName) == 1 )
               sciaOrbitFilesN++;
          }

        if ( hDir != NULL ) closedir(hDir);

        #endif

        rc=ERROR_ID_NO;
       }
   	 }
   	else
     {
     	sciaOrbitFilesN=1;
     	strcpy(sciaOrbitFiles[0].sciaFileName,pEngineContext->fileInfo.fileName);
     }

    // Load files

    for (sciaTotalRecordNumber=indexFile=0;indexFile<sciaOrbitFilesN;indexFile++)
     {
     	pOrbitFile=&sciaOrbitFiles[indexFile];

     	pOrbitFile->sciaPDSInfo.FILE_l1c=NULL;
     	pOrbitFile->specNumber=0;

      // Open file

      if (openL1c(pOrbitFile->sciaFileName,&pOrbitFile->sciaPDSInfo)!=ERROR_ID_NO)
       rc=ERROR_SetLast("SCIA_SetPDS",ERROR_TYPE_WARNING,ERROR_ID_PDS,"openL1c",pOrbitFile->sciaFileName);
  //     rc=ERROR_ID_PDS;

      // Read the irradiance data set to get the wavelength calibration

      else if (!(rc=SciaReadSunRefPDS(pEngineContext,indexFile)) &&
               ((THRD_id!=THREAD_TYPE_SPECTRA) || (THRD_browseType!=THREAD_BROWSE_DARK)))
       {
        // Read information on radiances spectra

        if (!pOrbitFile->sciaPDSInfo.n_states[NADIR])
         rc=ERROR_ID_FILE_EMPTY;
        else if (!(rc=SciaAllocateClusters(pEngineContext,pOrbitFile->sciaPDSInfo.cluster_ids,     // Allocate buffers for clusters
                 pOrbitFile->sciaPDSInfo.max_cluster_ids,pOrbitFile->sciaPDSInfo.n_states[NADIR],indexFile)) &&
                 !(rc=SciaReadNadirMDSInfo(pEngineContext,indexFile)) &&                           // get offset of NADIR measurement data set
                 !(rc=SciaNadirStates(pEngineContext,indexFile)) &&                                // determine the number of records from the cluster with highest integration time in the different states
                 !(rc=SciaNadirGeolocations(pEngineContext,indexFile)))                            // Read geolocations

         pEngineContext->recordInfo.scia.orbitNumber=atoi(pOrbitFile->sciaPDSInfo.mph.abs_orbit);
       }

      if (pOrbitFile->sciaPDSInfo.FILE_l1c!=NULL)
       {
        fclose(pOrbitFile->sciaPDSInfo.FILE_l1c);                               // Close the current PDS file
        pOrbitFile->sciaPDSInfo.FILE_l1c=NULL;
       }

      if ((strlen(pEngineContext->fileInfo.fileName)==strlen(pOrbitFile->sciaFileName)) &&
          !STD_Stricmp(pEngineContext->fileInfo.fileName,pOrbitFile->sciaFileName))
       sciaCurrentFileIndex=indexFile;

      sciaTotalRecordNumber+=pOrbitFile->specNumber;

      if (rc!=ERROR_ID_NO)
       pOrbitFile->rc=rc;

      rc=ERROR_ID_NO;
     }
   }

  if ((sciaCurrentFileIndex==ITEM_NONE) || !(pEngineContext->recordNumber=(pOrbitFile=&sciaOrbitFiles[sciaCurrentFileIndex])->specNumber))
   rc=ERROR_SetLast("SCIA_SetPDS",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pOrbitFile->sciaFileName);
  else
   {
    pEngineContext->recordInfo.scia.orbitNumber=atoi(pOrbitFile->sciaPDSInfo.mph.abs_orbit);
    if (!(rc=pOrbitFile->rc) && (pOrbitFile->sciaPDSInfo.FILE_l1c==NULL))
     pOrbitFile->sciaPDSInfo.FILE_l1c=fopen(pOrbitFile->sciaFileName,"rb");
   }

  // DEBUG

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("SetSCIAPDS",rc);
  #endif

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SCIA_ReadPDS
// -----------------------------------------------------------------------------
// PURPOSE       SCIAMACHY calibrated level 1 data read out
//
// INPUT         recordNo     index of the record to read
//
// INPUT/OUTPUT  pEngineContext    interface for file operations
//
// RETURN        ERROR_ID_FILE_END        the end of the file is reached;
//               ERROR_ID_FILE_RECORD     the record doesn't satisfy user constraints
//               ERROR_ID_NO              otherwise.
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC SCIA_ReadPDS(ENGINE_CONTEXT *pEngineContext,int recordNo)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  SATELLITE_GEOLOC *pSciaData;                                                  // data (geolocation+angles) of the current record
  INT stateObs;                                                                 // total number of observations covered by previous states
  INDEX indexState;                                                             // index of the current state
//   double tmp;
  RC rc;                                                                        // return code

  // Initializations

//  DEBUG_Print(DOAS_logFile,"Enter SCIA_ReadPDS %d\n",recordNo);

  pRecord=&pEngineContext->recordInfo;
  pOrbitFile=&sciaOrbitFiles[sciaCurrentFileIndex];
  rc=ERROR_ID_NO;

  // Goto the requested record

  if (!pOrbitFile->specNumber)
   rc=ERROR_ID_FILE_EMPTY;
  else if ((recordNo<=0) || (recordNo>pOrbitFile->specNumber))
   rc=ERROR_ID_FILE_END;
  else if ((THRD_id!=THREAD_TYPE_SPECTRA) || (THRD_browseType!=THREAD_BROWSE_DARK))
   {
    if ((indexState=SciaGetStateIndex(recordNo-1,&stateObs,sciaCurrentFileIndex))==ITEM_NONE)
     rc=ERROR_ID_FILE_RECORD;
    else
     {
      SciaReadNadirMDS(pEngineContext,indexState,recordNo-stateObs-1,sciaCurrentFileIndex);

      // Geolocation and angles data

      pSciaData=&pOrbitFile->sciaGeolocations[recordNo-1];

      memcpy(pRecord->scia.latitudes,pSciaData->latCorners,sizeof(double)*4);
      memcpy(pRecord->scia.longitudes,pSciaData->lonCorners,sizeof(double)*4);
      memcpy(pRecord->scia.solZen,pSciaData->solZen,sizeof(float)*3);
      memcpy(pRecord->scia.solAzi,pSciaData->solAzi,sizeof(float)*3);
      memcpy(pRecord->scia.losZen,pSciaData->losZen,sizeof(float)*3);
      memcpy(pRecord->scia.losAzi,pSciaData->losAzi,sizeof(float)*3);

      // Misecellaneous data (for TEMIS)

      pRecord->scia.earthRadius=pSciaData->earthRadius;
      pRecord->scia.satHeight=pSciaData->satHeight;
      pRecord->scia.stateIndex=indexState;
      pRecord->scia.stateId=pOrbitFile->sciaNadirStates[indexState].stateId;

      pRecord->latitude=pSciaData->latCenter;
      pRecord->longitude=pSciaData->lonCenter;

      pRecord->Zm=pSciaData->solZen[1];
      pRecord->Azimuth=pSciaData->solAzi[1];
      pRecord->zenithViewAngle=pSciaData->losZen[1];
      pRecord->azimuthViewAngle=pSciaData->losAzi[1];

      pRecord->Tint=pOrbitFile->sciaNadirStates[indexState].int_time/16.;

      pRecord->TimeDec=(double)pOrbitFile->sciaNadirStates[indexState].dsrTime+(recordNo-stateObs-1)*pOrbitFile->sciaNadirStates[indexState].int_time/16./86400.;
      pRecord->TimeDec=(double)(pRecord->TimeDec-floor(pRecord->TimeDec))*24.;

      // Get date and time

      SCIA_FromMJD2000ToYMD((double)pOrbitFile->sciaNadirStates[indexState].dsrTime+
                            (double)(recordNo-stateObs-1)*pOrbitFile->sciaNadirStates[indexState].int_time/16./86400.,&pRecord->present_day,&pRecord->present_time);

//      tmp=(pRecord->TimeDec-floor(pRecord->TimeDec))*60.;                   // fractional minutes
//      tmp=(tmp-floor(tmp))*60.;                                                 // fractional seconds
//      tmp=(tmp-floor(tmp))*1000.;                                               // fractional ms

//      SCIA_ms=(int)floor(tmp+0.5);

      pRecord->Tm=(double)ZEN_NbSec(&pRecord->present_day,&pRecord->present_time,0);  // !!!
     }
   }

//  DEBUG_Print(DOAS_logFile,"End  SCIA_ReadPDS %d - %d\n",recordNo,rc);

  // Return

  return rc;
 }

// =============================
// AUTOMATIC REFERENCE SELECTION
// =============================

// -----------------------------------------------------------------------------
// FUNCTION      SciaSortGetIndex
// -----------------------------------------------------------------------------
// PURPOSE       Get the position in sorted list of a new element (latitude, longitude or SZA)
//
// INPUT         fileIndex    index of the current file
//               value        the value to sort out
//               flag         0 for latitudes, 1 for longitudes, 2 for SZA
//               listSize     the current size of the sorted list
//
// RETURN        the index of the new element in the sorted list;
// -----------------------------------------------------------------------------

INDEX SciaSortGetIndex(double value,int flag,int listSize,INDEX fileIndex)
 {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX  imin,imax,icur;                                                        // indexes for dichotomic search
  double curValue;                                                              // value of element pointed by icur in the sorted list
  double curMinValue,curMaxValue;                                               // range of values

  // Initializations

  pOrbitFile=&sciaOrbitFiles[fileIndex];

  curValue=curMinValue=curMaxValue=(double)0.;

  imin=icur=0;
  imax=listSize;

  // Browse latitudes

  while (imax-imin>1)
   {
    // Dichotomic search

    icur=(imin+imax)>>1;

    switch(flag)
     {
   // ----------------------------------------------------------------------------
      case 0 :
       curValue=(double)pOrbitFile->sciaGeolocations[pOrbitFile->sciaLatIndex[icur]].latCenter;
      break;
   // ----------------------------------------------------------------------------
      case 1 :
       curValue=(double)pOrbitFile->sciaGeolocations[pOrbitFile->sciaLonIndex[icur]].lonCenter;
      break;
   // ----------------------------------------------------------------------------
      case 2 :
       curValue=(double)pOrbitFile->sciaGeolocations[pOrbitFile->sciaSzaIndex[icur]].solZen[1];
      break;
   // ----------------------------------------------------------------------------
     }

    // Move bounds

    if (curValue==value)
     imin=imax=icur;
    else if (curValue<value)
     imin=icur;
    else
     imax=icur;
   }

  if ((listSize>0) && (imax<listSize))
   {
    switch(flag)
     {
   // ----------------------------------------------------------------------------
      case 0 :
       curMinValue=(double)pOrbitFile->sciaGeolocations[pOrbitFile->sciaLatIndex[imin]].latCenter;
       curMaxValue=(double)pOrbitFile->sciaGeolocations[pOrbitFile->sciaLatIndex[imax]].latCenter;
      break;
   // ----------------------------------------------------------------------------
      case 1 :
       curMinValue=(double)pOrbitFile->sciaGeolocations[pOrbitFile->sciaLonIndex[imin]].lonCenter;
       curMaxValue=(double)pOrbitFile->sciaGeolocations[pOrbitFile->sciaLatIndex[imax]].lonCenter;
      break;
   // ----------------------------------------------------------------------------
      case 2 :
       curMinValue=(double)pOrbitFile->sciaGeolocations[pOrbitFile->sciaSzaIndex[imin]].solZen[1];
       curMaxValue=(double)pOrbitFile->sciaGeolocations[pOrbitFile->sciaLatIndex[imax]].solZen[1];
      break;
   // ----------------------------------------------------------------------------
    }

    icur=(value-curMinValue<curMaxValue-value)?imin:imax;
   }

  // Return

  return icur;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaSort
// -----------------------------------------------------------------------------
// PURPOSE       Sort NADIR records on latitudes, longitudes or SZA
//
// INPUT         fileIndex    index of the current file
//               indexRecord  the index of the record to sort out
//               flag         0 for latitudes, 1 for longitudes, 2 for SZA
//               listSize     the current size of the sorted list
//
// RETURN        the new index of the record in the sorted list;
// -----------------------------------------------------------------------------

void SciaSort(INDEX indexRecord,int flag,int listSize,INDEX fileIndex)
 {
  // Declaration

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX  newIndex,                                                              // the position of the new record in the sorted list
        *sortedList,                                                            // the sorted list
         i;                                                                     // browse the sorted list in reverse way

  double value;                                                                 // the value to sort out

  // Initializations

  pOrbitFile=&sciaOrbitFiles[fileIndex];
  value=(double)0.;
  sortedList=NULL;

  switch(flag)
   {
 // ----------------------------------------------------------------------------
    case 0 :
     sortedList=pOrbitFile->sciaLatIndex;
     value=pOrbitFile->sciaGeolocations[indexRecord].latCenter;
    break;
 // ----------------------------------------------------------------------------
    case 1 :
     sortedList=pOrbitFile->sciaLonIndex;
     value=pOrbitFile->sciaGeolocations[indexRecord].lonCenter;
    break;
 // ----------------------------------------------------------------------------
    case 2 :
     sortedList=pOrbitFile->sciaSzaIndex;
     value=pOrbitFile->sciaGeolocations[indexRecord].solZen[1];
    break;
 // ----------------------------------------------------------------------------
   }

  newIndex=SciaSortGetIndex(value,flag,listSize,fileIndex);

  // Shift values higher than the one to sort out

  if (newIndex<listSize)
   for (i=listSize;i>newIndex;i--)
    sortedList[i]=sortedList[i-1];

  // Insert new record in the sorted list

  sortedList[newIndex]=indexRecord;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaRefLat
// -----------------------------------------------------------------------------
// PURPOSE       Search for spectra in the orbit file matching latitudes and SZA
//               conditions
//
// INPUT         maxRefSize    the maximum size of vectors
//               latMin,latMax determine the range of latitudes;
//               sza,szaDelta  determine the range of SZA;
//
// OUTPUT        refList       the list of potential reference spectra
//
// RETURN        the number of elements in the refList reference list
// -----------------------------------------------------------------------------

INT SciaRefLat(SCIA_REF *refList,INT maxRefSize,double latMin,double latMax,double lonMin,double lonMax,double sza,double szaDelta)
 {
  // Declarations

  INDEX fileIndex;
  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX ilatMin,ilatMax,ilatTmp,                                                // range of indexes of latitudes matching conditions in sorted list
        ilatIndex,                                                              // browse records with latitudes in the specified range
        indexRef;                                                               // browse reference already registered in order to keep the list sorted

  INT nRef;                                                                     // the number of spectra matching latitudes and SZA conditions
  double szaDist,latDist;                                                       // distance with latitude and sza centers
  double lon;                                                                   // converts the longitude in the -180:180 range
  SATELLITE_GEOLOC *pRecord;

  // Initialization

  nRef=0;

  for (fileIndex=0;(fileIndex<sciaOrbitFilesN) && !nRef;fileIndex++)
   {
   	pOrbitFile=&sciaOrbitFiles[fileIndex];

   	if (!pOrbitFile->rc && pOrbitFile->specNumber)
   	 {
      // Determine the set of records in the orbit file matching the latitudes conditions

      ilatMin=SciaSortGetIndex(latMin,0,pOrbitFile->specNumber,fileIndex);
      ilatMax=SciaSortGetIndex(latMax,0,pOrbitFile->specNumber,fileIndex);

      if (ilatMin>ilatMax)
       {
        ilatTmp=ilatMin;
        ilatMin=ilatMax;
        ilatMax=ilatTmp;
       }

      // Browse spectra matching latitudes conditions

      for (ilatIndex=ilatMin;(ilatIndex<ilatMax) && (nRef<maxRefSize);ilatIndex++)

       if (ilatIndex<pOrbitFile->specNumber)
        {
         pRecord=&pOrbitFile->sciaGeolocations[pOrbitFile->sciaLatIndex[ilatIndex]];

         if ((fabs(lonMax-lonMin)>EPSILON) && (fabs(lonMax-lonMin)<(double)359.))
          latDist=THRD_GetDist(pRecord->lonCenter,pRecord->latCenter,(lonMax+lonMin)*0.5,(latMax+latMin)*0.5);
         else
          latDist=fabs(pRecord->latCenter-(latMax+latMin)*0.5);

         szaDist=fabs(pRecord->solZen[1]-sza);

         lon=((lonMax>(double)180.) &&
              (pRecord->lonCenter<(double)0.))?pRecord->lonCenter+360:pRecord->lonCenter;         // Longitudes for SCIA are in the range -180..180 */

         // Limit the latitudes conditions to SZA conditions

         if ((pRecord->latCenter>=latMin) && (pRecord->latCenter<=latMax) &&
            ((szaDelta<EPSILON) || (szaDist<=szaDelta)) &&
            ((fabs(lonMax-lonMin)<EPSILON) || (fabs(lonMax-lonMin)>359.) ||
            ((lon>=lonMin) && (lon<=lonMax))))
          {
           // Keep the list of records sorted

           for (indexRef=nRef;indexRef>0;indexRef--)

            if (latDist>=refList[indexRef-1].latDist)
             break;
            else
             memcpy(&refList[indexRef],&refList[indexRef-1],sizeof(SCIA_REF));

           refList[indexRef].indexFile=fileIndex;
           refList[indexRef].indexRecord=pOrbitFile->sciaLatIndex[ilatIndex];
           refList[indexRef].latitude=pRecord->latCenter;
           refList[indexRef].longitude=pRecord->lonCenter;
           refList[indexRef].sza=pRecord->solZen[1];
           refList[indexRef].szaDist=szaDist;
           refList[indexRef].latDist=latDist;

           nRef++;
          }
        }
     }
   }

  // Return

  return nRef;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaRefSza
// -----------------------------------------------------------------------------
// PURPOSE       Search for spectra in the orbit file matching SZA
//               conditions only
//
// INPUT         maxRefSize   the maximum size of vectors
//               sza,szaDelta determine the range of SZA;
//
// OUTPUT        refList      the list of potential reference spectra
//
// RETURN        the number of elements in the refList reference list
// -----------------------------------------------------------------------------

INT SciaRefSza(SCIA_REF *refList,INT maxRefSize,double sza,double szaDelta)
 {
  // Declarations

  INDEX fileIndex;
  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX iszaMin,iszaMax,iszaTmp,                                                // range of indexes of SZA matching conditions in sorted list
        iszaIndex,                                                              // browse records with SZA in the specified SZA range
        indexRef;                                                               // browse reference already registered in order to keep the list sorted

  INT nRef;                                                                     // the number of spectra matching latitudes and SZA conditions
  double szaDist;                                                               // distance with sza center
  SATELLITE_GEOLOC *pRecord;

  // Initialization

  nRef=0;

  for (fileIndex=0;(fileIndex<sciaOrbitFilesN) && !nRef;fileIndex++)
   {
    pOrbitFile=&sciaOrbitFiles[fileIndex];

    if (!pOrbitFile->rc && pOrbitFile->specNumber)
     {
      // Determine the set of records in the orbit file matching SZA conditions

      if (szaDelta>EPSILON)
       {
        iszaMin=SciaSortGetIndex(sza-szaDelta,2,pOrbitFile->specNumber,fileIndex);
        iszaMax=SciaSortGetIndex(sza+szaDelta,2,pOrbitFile->specNumber,fileIndex);
       }
      else  // No SZA conditions, search for the minimum
       {
        iszaMin=0;
        iszaMax=pOrbitFile->specNumber-1;
       }

      if (iszaMin>iszaMax)
       {
        iszaTmp=iszaMin;
        iszaMin=iszaMax;
        iszaMax=iszaTmp;
       }

      // Browse spectra matching SZA conditions

      for (iszaIndex=iszaMin;(iszaIndex<iszaMax) && (nRef<maxRefSize);iszaIndex++)

       if (iszaIndex<pOrbitFile->specNumber)
        {
         pRecord=&pOrbitFile->sciaGeolocations[pOrbitFile->sciaSzaIndex[iszaIndex]];
         szaDist=fabs(pRecord->solZen[1]-sza);

         if ((szaDelta<EPSILON) || (szaDist<=szaDelta))
          {
           // Keep the list of records sorted

           for (indexRef=nRef;indexRef>0;indexRef--)

            if (szaDist>=refList[indexRef-1].szaDist)
             break;
            else
             memcpy(&refList[indexRef],&refList[indexRef-1],sizeof(SCIA_REF));

           refList[indexRef].indexFile=fileIndex;
           refList[indexRef].indexRecord=pOrbitFile->sciaSzaIndex[iszaIndex];
           refList[indexRef].latitude=pRecord->latCenter;
           refList[indexRef].sza=pRecord->solZen[1];
           refList[indexRef].szaDist=szaDist;
           refList[indexRef].latDist=(double)0.;

           nRef++;
          }
        }
     }
   }

  // Return

  return nRef;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaBuildRef
// -----------------------------------------------------------------------------
// PURPOSE       Build a reference spectrum by averaging a set of spectra
//               matching latitudes and SZA conditions
//
// INPUT         refList      the list of potential reference spectra
//               nRef         the number of elements in the previous list
//               nSpectra     the maximum number of spectra to average to build the reference spectrum;
//               lembda       the grid of the irradiance spectrum
//               pEngineContext    interface for file operations
//               fp           pointer to the file dedicated to the display of information on selected spectra
//
// OUTPUT        ref          the new reference spectrum
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC SciaBuildRef(SCIA_REF *refList,INT nRef,INT nSpectra,double *lembda,double *ref,ENGINE_CONTEXT *pEngineContext,FILE *fp)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  SCIA_REF *pRef;                                                               // pointer to the current reference spectrum
  INDEX     indexRef,                                                           // browse reference in the list
            indexFile,                                                          // browse files
            indexState,indexObs,                                                // state index and observation number of the current record
            i;                                                                  // index for loop and arrays
  INT       nRec;                                                               // number of records use for the average
  INT       alreadyOpen;
  RC        rc;                                                                 // return code

  // Initialization

  pRecord=&pEngineContext->recordInfo;

  for (i=0;i<NDET;i++)
   ref[i]=(double)0.;

  rc=ERROR_ID_NO;

  if (nRef)
   {
    if (fp!=NULL)
     {
      fprintf(fp,"Ref Selection :\n");
      fprintf(fp,"%s\n",sciaOrbitFiles[refList[0].indexFile].sciaFileName);
      fprintf(fp,"Rec\t  SZA\t  Lat\t  Lon\n");
     }

    strcpy(pRecord->refFileName,sciaOrbitFiles[refList[0].indexFile].sciaFileName);
    pRecord->refRecord=refList[0].indexRecord+1;
   }

  // Search for spectra matching latitudes and SZA conditions in the selected record

  for (nRec=0,indexRef=0,indexFile=ITEM_NONE;
      (indexRef<nRef) && (nRec<nSpectra) && !rc;indexRef++)
   {
    pRef=&refList[indexRef];

    if ((indexFile==ITEM_NONE) || (pRef->indexFile==indexFile))
     {
      pOrbitFile=&sciaOrbitFiles[pRef->indexFile];

      alreadyOpen=(pOrbitFile->sciaPDSInfo.FILE_l1c!=NULL)?1:0;

      if (!alreadyOpen)
       pOrbitFile->sciaPDSInfo.FILE_l1c=fopen(pOrbitFile->sciaFileName,"rb");

      if ((indexState=SciaGetStateIndex(pRef->indexRecord,&indexObs,pRef->indexFile))==ITEM_NONE)
       rc=ERROR_ID_FILE_RECORD;

      // Read and accumulate selected radiances

      else if (((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) && !(rc=SciaReadNadirMDS(pEngineContext,indexState,pRef->indexRecord-indexObs,pRef->indexFile))))
       {
        if (fp!=NULL)
         fprintf(fp,"%-5d\t%-6.2lf\t%-6.2lf\t%-6.2lf\n",
                    pRef->indexRecord+1,pRef->sza,pRef->latitude,pRef->longitude);

        for (i=0;i<NDET;i++)
         ref[i]+=(double)pEngineContext->buffers.spectrum[i];

        nRec++;
        indexFile=pRef->indexFile;
       }

      if (!alreadyOpen)
       {
        fclose(pOrbitFile->sciaPDSInfo.FILE_l1c);
        pOrbitFile->sciaPDSInfo.FILE_l1c=NULL;
       }
     }
   }

  if (nRec==0)
   rc=ERROR_ID_NO_REF;
  else if (!rc)
   {
   	strcpy(OUTPUT_refFile,sciaOrbitFiles[indexFile].sciaFileName);
   	OUTPUT_nRec=nRec;

    for (i=0;i<NDET;i++)
     ref[i]/=nRec;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaRefSelection
// -----------------------------------------------------------------------------
// PURPOSE       Selection of a reference spectrum in the current orbit
//
// INPUT         pEngineContext    collect information on the current spectrum;
//               latMin,latMax determine the range of latitudes;
//               lonMin,lonMax determine the range of longitudes;
//               sza,szaDelta determine the range of SZA;
//
//               nSpectra     the number of spectra to average to build the reference spectrum;
//               lembdaK,ref  reference spectrum to use if no spectrum in the orbit matches the sza and latitudes conditions;
//
// OUTPUT        lembdaN,refN reference spectrum for northern hemisphere;
//               lembdaS,refS reference spectrum for southern hemisphere.
//
// RETURN        ERROR_ID_ALLOC if the allocation of buffers failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC SciaRefSelection(ENGINE_CONTEXT *pEngineContext,
                    double latMin,double latMax,
                    double lonMin,double lonMax,
                    double sza,double szaDelta,
                    int nSpectra,
                    double *lembdaK,double *ref,
                    double *lembdaN,double *refN,
                    double *lembdaS,double *refS)
 {
  // Declarations

  SCIA_REF *refList;                                                            // list of potential reference spectra
  double latDelta,tmp;
  INT nRefN,nRefS;                                                              // number of reference spectra in the previous list resp. for Northern and Southern hemisphere
  double normFact;                                                              // normalisation factor
  FILE *fp;                                                                     // pointer to the temporary file with information to display
  RC rc;                                                                        // return code

  // Initializations

// QDOAS ???  fp=(pEngineContext->project.spectra.displayDataFlag)?fopen(DOAS_tmpFile,"w+t"):NULL;
  fp=NULL;    // QDOAS ???

  if (latMin>latMax)
   {
   	tmp=latMin;
   	latMin=latMax;
   	latMax=tmp;
   }

  if (lonMin>lonMax)
   {
   	tmp=lonMin;
   	lonMin=lonMax;
   	lonMax=tmp;
   }

  latDelta=(double)fabs(latMax-latMin);
  szaDelta=(double)fabs(szaDelta);
  sza=(double)fabs(sza);

  rc=ERROR_ID_NO;

  memcpy(lembdaN,lembdaK,sizeof(double)*NDET);
  memcpy(lembdaS,lembdaK,sizeof(double)*NDET);

  memcpy(refN,ref,sizeof(double)*NDET);
  memcpy(refS,ref,sizeof(double)*NDET);

  nRefS=0;

  // Buffers allocation

  if ((refList=(SCIA_REF *)MEMORY_AllocBuffer("SciaRefSelection ","refList",sciaTotalRecordNumber,sizeof(SCIA_REF),0,MEMORY_TYPE_STRUCT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // Search for records matching latitudes and SZA conditions

    if (latDelta>EPSILON)                                                       // a latitude range is specified
     {
      // search for potential reference spectra in northern hemisphere

      if ((nRefN=nRefS=SciaRefLat(refList,sciaTotalRecordNumber,latMin,latMax,lonMin,lonMax,sza,szaDelta))>0)
       rc=SciaBuildRef(refList,nRefN,nSpectra,lembdaN,refN,pEngineContext,fp);

      if (!rc)
       memcpy(refS,refN,sizeof(double)*NDET);
     }

    // Search for records matching SZA conditions only

    else
     {
      if ((nRefN=nRefS=SciaRefSza(refList,sciaTotalRecordNumber,sza,szaDelta))>0)
       rc=SciaBuildRef(refList,nRefN,nSpectra,lembdaN,refN,pEngineContext,fp);

      if (!rc)
       memcpy(refS,refN,sizeof(double)*NDET);
     }

    if (!rc)
     {
      // No reference spectrum is found for both hemispheres -> error message

      if (!nRefN && !nRefS)
       rc=ERROR_SetLast("SciaRefSelection",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"the orbit",pEngineContext->fileInfo.fileName);

      // No reference spectrum found for Northern hemisphere -> use the reference found for Southern hemisphere

      else if (!nRefN)
       {
        if (fp!=NULL)
         fprintf(fp,"No record selected for the northern hemisphere, use reference of the southern hemisphere\n");

        memcpy(refN,refS,sizeof(double)*NDET);
       }

      // No reference spectrum found for Southern hemisphere -> use the reference found for Northern hemisphere

      else if (!nRefS)
       {
        if (fp!=NULL)
         fprintf(fp,"No record selected for the southern hemisphere, use reference of the northern hemisphere\n");

        memcpy(refS,refN,sizeof(double)*NDET);
       }

      if (nRefN || nRefS)   // if no record selected, use ref (normalized as loaded)
       {
        ANALYSE_NormalizeVector(refN-1,NDET,&normFact,"SciaRefSelection (refN) ");
        ANALYSE_NormalizeVector(refS-1,NDET,&normFact,"SciaRefSelection (refS) ");
       }
     }
   }

  // Close file

  if (fp!=NULL)
   {
    fprintf(fp,"\n");
    fclose(fp);
   }

  // Release allocated buffers

  if (refList!=NULL)
   MEMORY_ReleaseBuffer("SciaRefSelection ","refList",refList);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      SciaNewRef
// -----------------------------------------------------------------------------
// PURPOSE       In automatic reference selection, search for reference spectra
//
// INPUT         pEngineContext    hold the configuration of the current project
//
// RETURN        ERROR_ID_ALLOC if something failed;
//               ERROR_ID_NO otherwise.
// -----------------------------------------------------------------------------

RC SciaNewRef(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
  INDEX indexFeno;                                                              // browse analysis windows
  FENO *pTabFeno;                                                               // current analysis window
  RC rc;                                                                        // return code

  // Initializations

//  DEBUG_Print(DOAS_logFile,"Enter SciaNewRef\n");

  pRecord=&pEngineContext->recordInfo;
  memset(OUTPUT_refFile,0,MAX_PATH_LEN+1);
  OUTPUT_nRec=0;

  memset(pRecord->refFileName,0,MAX_PATH_LEN+1);
  pRecord->refRecord=ITEM_NONE;

  rc=THRD_CopySpecInfo(&THRD_refInfo,pEngineContext);                                // perform a backup of the pEngineContext structure

  if (THRD_refInfo.recordNumber==0)
   rc=ERROR_ID_ALLOC;
  else

   // Browse analysis windows

   for (indexFeno=0;(indexFeno<NFeno) && (rc<THREAD_EVENT_STOP);indexFeno++)
    {
     pTabFeno=&TabFeno[indexFeno];

     if ((pTabFeno->hidden!=1) &&
         (pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC) &&
         (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC))

      // Build reference spectra according to latitudes and SZA conditions

      rc=SciaRefSelection(&THRD_refInfo,
                          pTabFeno->refLatMin,pTabFeno->refLatMax,
                          pTabFeno->refLonMin,pTabFeno->refLonMax,
                          pTabFeno->refSZA,pTabFeno->refSZADelta,
                          pTabFeno->nspectra,
                          pTabFeno->LembdaK,pTabFeno->Sref,
                          pTabFeno->LembdaN,pTabFeno->SrefN,
                          pTabFeno->LembdaS,pTabFeno->SrefS);
    }

  THRD_goto.indexMin=THRD_goto.indexMax=ITEM_NONE;

  // Return

//  DEBUG_Print(DOAS_logFile,"End SciaNewRef %d\n",rc);

  strcpy(pRecord->refFileName,THRD_refInfo.recordInfo.refFileName);
  pRecord->refRecord=THRD_refInfo.recordInfo.refRecord;

  return rc;
 }

// ========
// ANALYSIS
// ========

// -----------------------------------------------------------------------------
// FUNCTION      SCIA_LoadAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Load analysis parameters depending on the irradiance spectrum
//
// INPUT         pEngineContext    data on the current file
//
// RETURN        0 for success
// -----------------------------------------------------------------------------

RC SCIA_LoadAnalysis(ENGINE_CONTEXT *pEngineContext)
 {
  // Declarations

  SCIA_ORBIT_FILE *pOrbitFile;                                                  // pointer to the current orbit
  INDEX indexFeno,indexTabCross,indexWindow,i;                                  // indexes for loops and array
  CROSS_REFERENCE *pTabCross;                                                   // pointer to the current cross section
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a symbol
  FENO *pTabFeno;                                                               // pointer to the current spectral analysis window
  double factTemp,lembdaMin,lembdaMax;                                          // working variables
  INT DimL,useUsamp,useKurucz,saveFlag;                                         // working variables
  RC rc;                                                                        // return code

  // Initializations

//  DEBUG_Print(DOAS_logFile,"Enter SCIA_LoadAnalysis\n");

  pOrbitFile=&sciaOrbitFiles[sciaCurrentFileIndex];
  saveFlag=(INT)pEngineContext->project.spectra.displayDataFlag;

  if (!(rc=pOrbitFile->rc) && (THRD_id==THREAD_TYPE_ANALYSIS) && (sciaLoadReferenceFlag || !ANALYSE_refSelectionFlag))
   {
    lembdaMin=(double)9999.;
    lembdaMax=(double)-9999.;

    rc=ERROR_ID_NO;
    useKurucz=useUsamp=0;

    // Browse analysis windows and load missing data

    for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
     if (!TabFeno[indexFeno].hidden)
      {
       pTabFeno=&TabFeno[indexFeno];
       pTabFeno->NDET=NDET;

       // Load calibration and reference spectra

       if (!pTabFeno->gomeRefFlag)
        {
         for (i=0;i<pTabFeno->NDET;i++)
          {
           pTabFeno->LembdaRef[i]=(double)(((float *)pOrbitFile->sciaSunWve)[i]);
           pTabFeno->Sref[i]=(double)(((float *)pOrbitFile->sciaSunRef)[i]);
          }

//         memcpy(pTabFeno->SrefSigma,SCIA_refE,sizeof(double)*pTabFeno->NDET);

         if (!(rc=ANALYSE_NormalizeVector(pTabFeno->Sref-1,pTabFeno->NDET,&factTemp,"SCIA_LoadAnalysis (Reference) "))) // &&
//             !(rc=ANALYSE_NormalizeVector(pTabFeno->SrefSigma-1,pTabFeno->NDET,&factTemp,"SCIA_LoadAnalysis (RefError) ")))
          {
           memcpy(pTabFeno->SrefEtalon,pTabFeno->Sref,sizeof(double)*pTabFeno->NDET);
           pTabFeno->useEtalon=pTabFeno->displayRef=1;

           // Browse symbols

           for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
            {
             pTabCross=&pTabFeno->TabCross[indexTabCross];
             pWrkSymbol=&WorkSpace[pTabCross->Comp];

             // Cross sections and predefined vectors

             if ((((pWrkSymbol->type==WRK_SYMBOL_CROSS) && (pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING)) ||
                  ((pWrkSymbol->type==WRK_SYMBOL_PREDEFINED) &&
                  ((indexTabCross==pTabFeno->indexCommonResidual) ||
                 (((indexTabCross==pTabFeno->indexUsamp1) || (indexTabCross==pTabFeno->indexUsamp2)) && (pUsamp->method==PRJCT_USAMP_FILE))))) &&
                  ((rc=ANALYSE_CheckLembda(pWrkSymbol,pTabFeno->LembdaRef,"SCIA_LoadAnalysis "))!=ERROR_ID_NO))

              goto EndSCIA_LoadAnalysis;
            }

           // Gaps : rebuild subwindows on new wavelength scale

           for (indexWindow=0,DimL=0;indexWindow<pTabFeno->svd.Z;indexWindow++)
            {
             pTabFeno->svd.Fenetre[indexWindow][0]=FNPixel(pTabFeno->LembdaRef,pTabFeno->svd.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
             pTabFeno->svd.Fenetre[indexWindow][1]=FNPixel(pTabFeno->LembdaRef,pTabFeno->svd.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

             DimL+=(pTabFeno->svd.Fenetre[indexWindow][1]-pTabFeno->svd.Fenetre[indexWindow][0]+1);
            }

           pTabFeno->svd.DimL=DimL;

           // Buffers allocation

           ANALYSE_SvdFree("SCIA_LoadAnalysis",&pTabFeno->svd);
           ANALYSE_SvdLocalAlloc("SCIA_LoadAnalysis",&pTabFeno->svd);

           pTabFeno->Decomp=1;

           if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LembdaRef))!=ERROR_ID_NO) ||
               (!pKuruczOptions->fwhmFit && pTabFeno->xsToConvolute &&
               ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LembdaRef,&ANALYSIS_slit,pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2,&pSlitOptions->slitFunction.slitParam3,&pSlitOptions->slitFunction.slitParam4))!=ERROR_ID_NO)))

            goto EndSCIA_LoadAnalysis;
          }

         memcpy(pTabFeno->LembdaK,pTabFeno->LembdaRef,sizeof(double)*pTabFeno->NDET);
         memcpy(pTabFeno->Lembda,pTabFeno->LembdaRef,sizeof(double)*pTabFeno->NDET);

         useUsamp+=pTabFeno->useUsamp;
         useKurucz+=pTabFeno->useKurucz;

         if (pTabFeno->useUsamp)
          {
           if (pTabFeno->LembdaRef[0]<lembdaMin)
            lembdaMin=pTabFeno->LembdaRef[0];
           if (pTabFeno->LembdaRef[pTabFeno->NDET-1]>lembdaMax)
            lembdaMax=pTabFeno->LembdaRef[pTabFeno->NDET-1];
          }
        }
      }

    // Wavelength calibration alignment

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
     {
      KURUCZ_Init(0);

      if ((THRD_id!=THREAD_TYPE_KURUCZ) && ((rc=KURUCZ_Reference(NULL,0,saveFlag,0))!=ERROR_ID_NO))
       goto EndSCIA_LoadAnalysis;
     }

    // Build undersampling cross sections

    if (useUsamp && (THRD_id!=THREAD_TYPE_KURUCZ))
     {
      USAMP_LocalFree();

      if (((rc=USAMP_LocalAlloc(0 /* lembdaMin,lembdaMax,oldNDET */))!=ERROR_ID_NO) ||
          ((rc=USAMP_BuildFromAnalysis(0,0))!=ERROR_ID_NO) ||
          ((rc=USAMP_BuildFromAnalysis(1,ITEM_NONE))!=ERROR_ID_NO))

       goto EndSCIA_LoadAnalysis;
     }

    // Automatic reference selection

    if (sciaLoadReferenceFlag && !(rc=SciaNewRef(pEngineContext)) &&
      !(rc=ANALYSE_AlignReference(2,pEngineContext->project.spectra.displayDataFlag))) // automatic ref selection for Northern hemisphere
     rc=ANALYSE_AlignReference(3,pEngineContext->project.spectra.displayDataFlag);     // automatic ref selection for Southern hemisphere
   }

  // Return

  EndSCIA_LoadAnalysis :

//  DEBUG_Print(DOAS_logFile,"End SCIA_LoadAnalysis %d\n",rc);

  return rc;
 }

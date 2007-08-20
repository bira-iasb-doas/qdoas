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

#include <stdlib.h>

#include "mediate.h"
#include "mediate_response.h"
#include "engine.h"

int mediateRequestCreateEngineContext(void **engineContext, void *responseHandle)
 {
  *engineContext = (void *)EngineCreateContext();
  return 0;
}

int mediateRequestDestroyEngineContext(void *engineContext, void *responseHandle)
{
  EngineDestroyContext(engineContext);
  return 0;
}

int mediateRequestSetProject(void *engineContext,
			     const mediate_project_t *project, void *responseHandle)
 {
 	// Declarations

	 PROJECT *pEngineProject;
	 ENGINE_CONTEXT *pEngineContext;

	 // Initialization

	 pEngineContext=(ENGINE_CONTEXT *)engineContext;

	 //
	 // Transfer data from mediate project to engine project -> still to do
	 //

	 pEngineProject=&pEngineContext->project;

	 pEngineProject->instrumental.readOutFormat=PRJCT_INSTR_FORMAT_LOGGER;
	 pEngineProject->instrumental.azimuthFlag=1;

	 // ------------------------

	 // Set the detector size (implicit to the selection of the file format)

  switch (pEngineProject->instrumental.readOutFormat)
   {
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_CCD_EEV :                          // CCD EEV 1340x400
     pEngineContext->NDET=(pEngineProject->instrumental.detectorSize)?pEngineProject->instrumental.detectorSize:1340;
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_LOGGER :                           // Logger (PDA,CCD or HAMAMATSU)
    case PRJCT_INSTR_FORMAT_ACTON :                            // Acton (NILU)
    case PRJCT_INSTR_FORMAT_PDAEGG :                           // PDA EG&G (sept. 94 until now)
    case PRJCT_INSTR_FORMAT_PDAEGG_OLD :                       // PDA EG&G (spring 94)
    case PRJCT_INSTR_FORMAT_PDAEGG_ULB :                       // PDA EG&G (ULB)
    case PRJCT_INSTR_FORMAT_CCD_OHP_96 :                       // CCD OHP 96
    case PRJCT_INSTR_FORMAT_CCD_HA_94 :                        // CCD Harestua 94
    case PRJCT_INSTR_FORMAT_PDASI_EASOE :                      // PDA SI (IASB)
    case PRJCT_INSTR_FORMAT_PDASI_OSMA :                       // PDA SI (OSMA)
    case PRJCT_INSTR_FORMAT_SAOZ_EFM :                         // SAOZ
    case PRJCT_INSTR_FORMAT_RASAS :                            // RASAS (INTA)
    case PRJCT_INSTR_FORMAT_GDP_ASCII :                        // GOME GDP ASCII format
    case PRJCT_INSTR_FORMAT_GDP_BIN :                          // GOME GDP ASCII format
    case PRJCT_INSTR_FORMAT_SCIA_HDF :                         // SCIAMACHY calibrated Level 1 data in HDF format
    case PRJCT_INSTR_FORMAT_SCIA_PDS :                         // SCIAMACHY calibrated Level 1 data in PDS format
    case PRJCT_INSTR_FORMAT_NOAA :                             // NOAA
    case PRJCT_INSTR_FORMAT_OMI :                              // OMI
	   case PRJCT_INSTR_FORMAT_GOME2 :                            // GOME2
     pEngineContext->NDET=1024;
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_VIS :                         // SAOZ visible
    case PRJCT_INSTR_FORMAT_SAOZ_UV :                          // SAOZ UV
    case PRJCT_INSTR_FORMAT_CCD_ULB :                          // CCD (ULB)
     pEngineContext->NDET=512;
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_UOFT :                             // University of Toronto
     pEngineContext->NDET=2000;
    break;
 // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC :                              // MFC Heidelberg
    case PRJCT_INSTR_FORMAT_MFC_STD :                          // MFC Heidelberg (ASCII)
    default :
     pEngineContext->NDET=pEngineProject->instrumental.detectorSize;
    break;
 // ---------------------------------------------------------------------------
   }

//  if (engineContext<=0)
//   mediateResponseErrorMessage("mediateRequestSetProject","The size of the detector is invalid", eFatalEngineError, responseHandle);
//  else
     EngineSetProject(pEngineContext);

  return 0;
 }

int mediateRequestSetAnalysisWindows(void *engineContext,
				     int numberOfWindows,
				     const mediate_analysis_window_t *analysisWindows,
				     void *responseHandle)
{
  return 0;
}

int mediateRequestSetSymbols(void *engineContext,
			     int numberOfSymbols,
			     const mediate_symbol_t *symbols,
			     void *responseHandle)
{
  return 0;
}

int mediateRequestSetSites(void *engineContext,
			   int numberOfSites,
			   const mediate_site_t *sites,
			   void *responseHandle)
{
  return 0;
}


int mediateRequestBeginBrowseSpectra(void *engineContext,
				     const char *spectraFileName,
				     void *responseHandle)
{
  ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;

  pEngineContext->recordNumber=EngineSetFile(pEngineContext,spectraFileName);

  pEngineContext->indexRecord = 1;

  // mediateResponseErrorMessage("fred", "Coucou message", eWarningEngineError, responseHandle);

  return pEngineContext->recordNumber;
}

int mediateRequestGotoSpectrum(void *engineContext,
			       int recordNumber,
			       void *responseHandle)
{
  ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;

  if (recordNumber > 0 && recordNumber <= pEngineContext->recordNumber) {
    pEngineContext->indexRecord = recordNumber;
    return recordNumber;
  }
  else {
    pEngineContext->indexRecord = pEngineContext->recordNumber + 1;
    return 0;
  }
}

int mediateRequestNextMatchingBrowseSpectrum(void *engineContext,
					     void *responseHandle)
{
	 // Declarations

  ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
  PROJECT *pProject;
  PRJCT_INSTRUMENTAL *pInstrumental;
  SHORT_DATE  *pDay;                                                            // pointer to measurement date
  struct time *pTime;                                                           // pointer to measurement date<<<<<<< .mine
  char tmpString[80];                                                           // buffer for formatted strings
  int rec = (pEngineContext->indexRecord)++;
  int indexLine,indexColumn,page;
  double x[1024], y[1024];
  plot_data_t spectrumData;
  int i;
  double xx, scale;

  // Initializations

  pProject=&pEngineContext->project;
  pInstrumental=&pProject->instrumental;
  pDay=&pEngineContext->present_day;
  pTime=&pEngineContext->present_time;

  indexLine=3;
  indexColumn=2;
  page=0;

  if (rec > pEngineContext->recordNumber) {
    return 0;
  }
  else {
    //-----------------
    // TEMPORARY COMMENT
    //-----------------
    // Caroline, I dont like the mediateAllocateAndSetPlotData stuff we added just before you left.
    // It works, but we are copying the data too many times. I think it would be better if you
    // adapt the system so that it justs alloocates the data arrays and your engine code can
    // write directly into the allocated double* arrays. Right now, you read data, it gets copied
    // into the plot_data_t by the mediateAllocateAndSetPlotData function, and the mediateResponsePlotData
    // copies that into the structures managed by the responseHandle... we should eliminate one
    // of the copies.

    EngineReadFile(pEngineContext,rec,x,y);

    mediateAllocateAndSetPlotData(&spectrumData, x, y, 1024, PlotDataType_Spectrum, "legend string");
    mediateResponsePlotData(0, &spectrumData, 1, "Spectrum", "Lambda (nm)", "Y Label", responseHandle);
    mediateReleasePlotData(&spectrumData);
// QDOAS ???     fprintf(fp,"Project\t\t\t%s\n",pProject->name);
    mediateAllocateAndSetPlotData(&spectrumData, x, y, 512, PlotDataType_Spectrum, "legend string");
    mediateResponsePlotData(5, &spectrumData, 1, "Fred", "on page 5", "blah", responseHandle);
    mediateReleasePlotData(&spectrumData);

    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"File","%s",

// QDOAS ???
// QDOAS ???     fprintf(fp,"Project\t\t\t%s\n",pProject->name);
// QDOAS ???
// QDOAS ???     fprintf(fp,"File\t\t\t%s\n",
// QDOAS ???           ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)||
// QDOAS ???            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)||
// QDOAS ???            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OPUS))?PATH_fileSpectra:
                  pEngineContext->fileName);


    if (strlen(pInstrumental->instrFunction))
     {
      if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_ULB)
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Dark Current","%s",pInstrumental->instrFunction);
      else if (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC)
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Instr. function","%s",pInstrumental->instrFunction);
      else if (((pInstrumental->mfcMaskSpec!=0) && ((UINT)MFC_header.ty==pInstrumental->mfcMaskSpec)) ||
               ((pInstrumental->mfcMaskSpec==0) &&
               ((MFC_header.wavelength1==pInstrumental->mfcMaskInstr) ||
                (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.))))
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Instr. function","%s",pInstrumental->instrFunction);
     }

// QDOAS ???
// QDOAS ???     if (strlen(pInstrumental->vipFile))
// QDOAS ???      {
// QDOAS ???       if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)
// QDOAS ???        fprintf(fp,"Dark current\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileDark,1));
// QDOAS ???       else if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) &&
// QDOAS ???             (((pInstrumental->mfcMaskSpec!=0) && (((UINT)MFC_header.ty==pInstrumental->mfcMaskSpec) || ((UINT)MFC_header.ty==pInstrumental->mfcMaskInstr))) ||
// QDOAS ???               ((pInstrumental->mfcMaskSpec==0) &&
// QDOAS ???               ((MFC_header.wavelength1==pInstrumental->mfcMaskInstr) ||
// QDOAS ???                (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.)))))
// QDOAS ???        fprintf(fp,"Dark current\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileDark,1));
// QDOAS ???       else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)
// QDOAS ???        fprintf(fp,"Straylight correction\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->vipFile,1));
// QDOAS ???       else
// QDOAS ???        fprintf(fp,"Interpixel\t\t\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->vipFile,1));
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (strlen(pInstrumental->dnlFile))
// QDOAS ???      {
// QDOAS ???       if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)
// QDOAS ???        fprintf(fp,"Offset\t\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileOffset,1));
// QDOAS ???       else if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) &&
// QDOAS ???              (((MFC_header.ty!=0) &&
// QDOAS ???              (((UINT)MFC_header.ty==pInstrumental->mfcMaskSpec) ||
// QDOAS ???               ((UINT)MFC_header.ty==pInstrumental->mfcMaskDark) ||
// QDOAS ???               ((UINT)MFC_header.ty==pInstrumental->mfcMaskInstr))) ||
// QDOAS ???               ((pInstrumental->mfcMaskSpec==0) &&
// QDOAS ???               ((MFC_header.wavelength1==pInstrumental->mfcMaskDark) ||
// QDOAS ???                (MFC_header.wavelength1==pInstrumental->mfcMaskInstr)||
// QDOAS ???                (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.)))))
// QDOAS ???        fprintf(fp,"Offset\t\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileOffset,1));
// QDOAS ???       else
// QDOAS ???        fprintf(fp,"Non linearity\t\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->dnlFile,1));
// QDOAS ???      }
// QDOAS ???
     if (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GOME2)
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Date and Time","%02d/%02d/%d %02d:%02d:%02d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
// QDOAS ???     else
// QDOAS ???      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Date and Time","%02d/%02d/%d %02d:%02d:%02d.%06d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec,GOME2_ms);

 //    sprintf(tmpString,"%.3f -> %.3f \n",pEngineContext->TimeDec,pEngineContext->localTimeDec);

     if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD))
      {
       pTime=&pEngineContext->startTime;
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Start time","%02d:%02d:%02d",pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
       pTime=&pEngineContext->endTime;
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"End time","%02d:%02d:%02d",pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
      }

     if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF))
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Record","%d/%d",pEngineContext->indexRecord,pEngineContext->recordNumber);
     else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI)
      {
      	if (pEngineContext->project.instrumental.averageFlag)
        mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Record","%d/%d (%d spectra averaged)",
                pEngineContext->indexRecord,pEngineContext->recordNumber,pEngineContext->omi.omiNumberOfSpectraPerTrack);
      	else
        mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Record","%d/%d (track %d/%d, spectrum %d/%d)",
                pEngineContext->indexRecord,pEngineContext->recordNumber,
                pEngineContext->omi.omiTrackIndex,pEngineContext->omi.omiNumberOfTracks,
                pEngineContext->omi.omiSpecIndex,pEngineContext->omi.omiNumberOfSpectraPerTrack);
      }
     else
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Record","%d/%d",pEngineContext->indexRecord,pEngineContext->recordNumber);

// QDOAS ???
// QDOAS ???     if (strlen(pEngineContext->Nom))
// QDOAS ???      sprintf(tmpString,"Record name\t\t%s\n",pEngineContext->Nom);
// QDOAS ???
// QDOAS ???     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
// QDOAS ???      {
// QDOAS ???       if (strlen(MFC_header.FileName))
// QDOAS ???        sprintf(tmpString,"Spectrum\t\t\t%s\n",MFC_header.FileName);
// QDOAS ???       if (strlen(MFC_header.specname))
// QDOAS ???        sprintf(tmpString,"Record name\t\t%s\n",MFC_header.specname);
// QDOAS ???       if (strlen(MFC_header.site))
// QDOAS ???        sprintf(tmpString,"Site\t\t\t%s\n",MFC_header.site);
// QDOAS ???       if (strlen(MFC_header.spectroname))
// QDOAS ???        sprintf(tmpString,"Spectro name\t\t%s\n",MFC_header.spectroname);
// QDOAS ???       if (strlen(MFC_header.scan_dev))
// QDOAS ???        sprintf(tmpString,"Scan device\t\t%s\n",MFC_header.scan_dev);
// QDOAS ???       if (strlen(MFC_header.first_line))
// QDOAS ???        sprintf(tmpString,"%s\n",MFC_header.first_line);
// QDOAS ???       if (strlen(MFC_header.spaeter))
// QDOAS ???        sprintf(tmpString,"%s\n",MFC_header.spaeter);
// QDOAS ???
// QDOAS ???       if (strlen(MFC_header.backgrnd))
// QDOAS ???        sprintf(tmpString,"Background\t\t%s\n",MFC_header.backgrnd);
// QDOAS ???
// QDOAS ???       sprintf(tmpString,"ty mask\t\t\t%d\n",MFC_header.ty);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
// QDOAS ???         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))
// QDOAS ???      {
// QDOAS ???       sprintf(tmpString,"Orbit Number\t\t%d\n",pEngineContext->gome.orbitNumber+1);
// QDOAS ???       sprintf(tmpString,"Pixel Number\t\t%d\n",pEngineContext->gome.pixelNumber);
// QDOAS ???       sprintf(tmpString,"Pixel Type\t\t%d\n",pEngineContext->gome.pixelType);
// QDOAS ???      }
// QDOAS ???     else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
// QDOAS ???      sprintf(tmpString,"Orbit Number\t\t%d\n",pEngineContext->scia.orbitNumber);
// QDOAS ???
// QDOAS ???     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
// QDOAS ???      sprintf(tmpString,"Calibration parameters\t%.2f %.3e %.3e %.3e\n",pEngineContext->wavelength1,pEngineContext->dispersion[0],
// QDOAS ???                  pEngineContext->dispersion[1],pEngineContext->dispersion[2]);
// QDOAS ???
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Solar Zenith angle","%-.3f °",pEngineContext->Zm);
// QDOAS ???
     if ((pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GDP_ASCII) &&
         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GDP_BIN) &&
         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_HDF) &&
         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS) &&
         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_OMI) &&
         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GOME2))
      {
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Exposure time","%.3f sec",pEngineContext->Tint);
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scans taken into account","%d",pEngineContext->NSomme);
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Elevation viewing angle","%.3f °",pEngineContext->elevationViewAngle);
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Azimuth viewing angle","%.3f °",pEngineContext->azimuthViewAngle);
      }
     else
      {
      	mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Solar Azimuth angle","%.3f °",pEngineContext->Azimuth);
      	mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Viewing Zenith angle","%.3f °",pEngineContext->zenithViewAngle);
      	mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Viewing Azimuth angle","%.3f °",pEngineContext->azimuthViewAngle);
      }

     if (pEngineContext->rejected>0)
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Rejected scans","%d",pEngineContext->rejected);
     if (pEngineContext->TDet!=(double)0.)
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Detector temperature","%.3f °",pEngineContext->TDet);
// QDOAS ???
// QDOAS ???     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_ULB)
// QDOAS ???      {
// QDOAS ???       sprintf(tmpString,"NTracks\t\t\t%d \n",pEngineContext->NTracks);
// QDOAS ???       sprintf(tmpString,"Grating\t\t\t%d \n",pEngineContext->NGrating);
// QDOAS ???       sprintf(tmpString,"Centre Wavelength\t\t%.1f nm\n",pEngineContext->Nanometers);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (pEngineContext->longitude!=(double)0.)
// QDOAS ???      sprintf(tmpString,"Longitude\t\t%.3f °\n",pEngineContext->longitude);
// QDOAS ???     if (pEngineContext->latitude!=(double)0.)
// QDOAS ???      sprintf(tmpString,"Latitude\t\t\t%.3f °\n",pEngineContext->latitude);
// QDOAS ???     if (pEngineContext->altitude!=(double)0.)
// QDOAS ???      sprintf(tmpString,"Altitude\t\t\t%.3f km\n",pEngineContext->altitude);
// QDOAS ???
// QDOAS ???     if (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
// QDOAS ???          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)) &&
// QDOAS ???         ((pEngineContext->aMoon!=(double)0.) || (pEngineContext->hMoon!=(double)0.) || (pEngineContext->fracMoon!=(double)0.)))
// QDOAS ???      {
// QDOAS ???       sprintf(tmpString,"Moon azimuthal angle\t%.3f °\n",pEngineContext->aMoon);
// QDOAS ???       sprintf(tmpString,"Moon elevation\t\t%.3f °\n",pEngineContext->hMoon);
// QDOAS ???       sprintf(tmpString,"Moon illuminated fraction\t%.3f °\n",pEngineContext->fracMoon);
// QDOAS ???      }
// QDOAS ???     else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
// QDOAS ???      {
// QDOAS ???       GOME_ORBIT_FILE *pOrbitFile;
// QDOAS ???       pOrbitFile=&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];
// QDOAS ???
// QDOAS ???       sprintf(tmpString,"Cloud fraction\t\t%.2f\n",(float)pOrbitFile->gdpBinSpectrum.cloudFraction*0.01);
// QDOAS ???       sprintf(tmpString,"Cloud top pressure\t\t%.2f\n",(float)pOrbitFile->gdpBinSpectrum.cloudTopPressure*0.01);
// QDOAS ???       sprintf(tmpString,"[O3 VCD]\t\t\t%.2f DU\n",(float)pOrbitFile->gdpBinSpectrum.o3*0.01);
// QDOAS ???       sprintf(tmpString,"[No2 VCD]\t\t%.2e mol/cm2\n",(float)pOrbitFile->gdpBinSpectrum.no2*1.e13);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     sprintf(tmpString,"\n\n");
// QDOAS ???     fclose(fp);
// QDOAS ???    }

//    mediateResponseCellDataString(0, 3, 2, "of", responseHandle);
//    mediateResponseCellDataInteger(0, 5, 1, 987654, responseHandle);
//    mediateResponseCellDataString(0, 0, 0, "Origin", responseHandle);
//    mediateResponseCellDataDouble(0, 5, 4, 1.23456e7, responseHandle);
//    mediateResponseCellDataInteger(5, 3, 3, pEngineContext->recordNumber, responseHandle);
//    mediateResponseCellDataInteger(5, 3, 1, pEngineContext->indexRecord, responseHandle);

    sprintf(tmpString,"Spectrum (%d/%d)",pEngineContext->indexRecord,pEngineContext->recordNumber);

    mediateResponseLabelPage(0, pEngineContext->fileName, tmpString, responseHandle);
    mediateResponseLabelPage(5, "Analysis ...", "BrO", responseHandle);


    return rec;
  }
}

int mediateRequestEndBrowseSpectra(void *engineContext,
				   void *responseHandle)
{
  ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;

  pEngineContext->indexRecord = -1;
  ++(pEngineContext->indexRecord);

  // close file pointers !

  return 0;
}

int mediateRequestBeginAnalyseSpectra(void *engineContext,
				      const char *spectraFileName,
				      void *responseHandle)
{
  return 0;
}

int mediateRequestNextMatchingAnalyseSpectrum(void *engineContext,
					      void *responseHandle)
{
  return 0;
}

int mediateRequestPrevMatchingAnalyseSpectrum(void *engineContext,
					      void *responseHandle)
{
  return 0;
}

int mediateRequestEndAnalyseSpectra(void *engineContext,
				    void *responseHandle)
{
  return 0;
}


int mediateRequestBeginCalibrateSpectra(void *engineContext,
					const char *spectraFileName,
					void *responseHandle)
{
  return 0;
}

int mediateRequestNextMatchingCalibrateSpectrum(void *engineContext,
						void *responseHandle)
{
  return 0;
}

int mediateRequestPrevMatchingCalibrateSpectrum(void *engineContext,
						void *responseHandle)
{
  return 0;
}

int mediateRequestEndCalibrateSpectra(void *engineContext,
				      void *responseHandle)
{
  return 0;
}


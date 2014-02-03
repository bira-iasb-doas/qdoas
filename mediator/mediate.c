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

#include "mediate.h"
#include "engine.h"
#include "output.h"

#include <stdio.h>
#include <stdbool.h>

int mediateRequestDisplaySpecInfo(void *engineContext,int page,void *responseHandle)
 {
   // Declarations

   ENGINE_CONTEXT *pEngineContext=(ENGINE_CONTEXT *)engineContext;
   PROJECT *pProject;                                                            // pointer to the project part of the engine context
   PRJCT_SPECTRA *pSpectra;                                                      // pointer to the spectra part of the project
   PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
   RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
   SHORT_DATE  *pDay;                                                            // pointer to measurement date
   struct time *pTime;                                                           // pointer to measurement date
   int indexLine,indexColumn;
   char blankString[256];

   // Initializations

   pRecord=&pEngineContext->recordInfo;
   pProject=&pEngineContext->project;
   pSpectra=&pProject->spectra;
   pInstrumental=&pProject->instrumental;
   pDay=&pRecord->present_day;
   pTime=&pRecord->present_time;

   memset(blankString,' ',256);
   blankString[255]='\0';

   indexLine=1;
   indexColumn=2;

   mediateResponseCellInfo(page,0,3,responseHandle,blankString,blankString);

   if (strlen(pInstrumental->instrFunction))
    {
     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_ULB)
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Dark Current","%s",pInstrumental->instrFunction);
     else if (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC)
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Transmission file","%s",pInstrumental->instrFunction);
     else if (((pInstrumental->mfcMaskSpec!=0) && ((unsigned int)MFC_header.ty==pInstrumental->mfcMaskSpec)) ||
              ((pInstrumental->mfcMaskSpec==0) &&
               ((MFC_header.wavelength1==pInstrumental->mfcMaskInstr) ||
                (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.))))
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Transmission file","%s",pInstrumental->instrFunction);
    }

   if (strlen(pInstrumental->vipFile))
    {
     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Dark current","%s",MFC_fileDark);
     else if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) &&
              (((pInstrumental->mfcMaskSpec!=0) && (((unsigned int)MFC_header.ty==pInstrumental->mfcMaskSpec) || ((unsigned int)MFC_header.ty==pInstrumental->mfcMaskInstr))) ||
               ((pInstrumental->mfcMaskSpec==0) &&
                ((MFC_header.wavelength1==pInstrumental->mfcMaskInstr) ||
                 (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.)))))
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Dark current","%s",MFC_fileDark);
     else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Straylight correction","%s",pInstrumental->vipFile);
     else
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Interpixel","%s",pInstrumental->vipFile);
    }

  if (strlen(pInstrumental->offsetFile))
    {
      if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)
        mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Offset","%s",MFC_fileOffset);
      else if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) &&
               (((MFC_header.ty!=0) &&
                 (((unsigned int)MFC_header.ty==pInstrumental->mfcMaskSpec) ||
                  ((unsigned int)MFC_header.ty==pInstrumental->mfcMaskDark) ||
                  ((unsigned int)MFC_header.ty==pInstrumental->mfcMaskInstr))) ||
                ((pInstrumental->mfcMaskSpec==0) &&
                 ((MFC_header.wavelength1==pInstrumental->mfcMaskDark) ||
                  (MFC_header.wavelength1==pInstrumental->mfcMaskInstr)||
                  (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.)))))
        mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Offset","%s",MFC_fileOffset);
    }

  if (strlen(pInstrumental->dnlFile))
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Non linearity","%s",pInstrumental->dnlFile);

  if (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GOME2)
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Date and Time","%02d/%02d/%d %02d:%02d:%02d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
  else
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Date and Time","%02d/%02d/%d %02d:%02d:%02d.%06d",pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec,GOME2_mus);

  //      sprintf(tmpString,"%.3f -> %.3f \n",pRecord->TimeDec,pRecord->localTimeDec);

  pDay=&pRecord->startDate;
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_STARTDATE])
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Start date","%02d/%02d/%d",pDay->da_day,pDay->da_mon,pDay->da_year);
  pDay=&pRecord->endDate;
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_ENDDATE])
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"End date","%02d/%02d/%d",pDay->da_day,pDay->da_mon,pDay->da_year);

  pTime=&pRecord->startTime;
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_STARTTIME])
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Start time","%02d:%02d:%02d",pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
  pTime=&pRecord->endTime;
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_ENDTIME])
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"End time","%02d:%02d:%02d",pTime->ti_hour,pTime->ti_min,pTime->ti_sec);

  if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
    mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Record","%d/%d",pEngineContext->indexRecord,pEngineContext->recordNumber);
  else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI)
    {
      if (pInstrumental->averageFlag)
        mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Record","%d/%d (%d spectra averaged)",
                                pEngineContext->indexRecord,pEngineContext->recordNumber,pRecord->omi.nMeasurements);
      else
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Record","%d/%d (measurement %d/%d, row %d/%d)",
                               pEngineContext->indexRecord,pEngineContext->recordNumber,
                               pRecord->omi.omiMeasurementIndex,pRecord->omi.nMeasurements,
                               pRecord->omi.omiRowIndex,pRecord->omi.nXtrack);

      if (pSpectra->fieldsFlag[PRJCT_RESULTS_OMI_INDEX_SWATH])
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Index of measurement","%d",pRecord->omi.omiMeasurementIndex);
      if (pSpectra->fieldsFlag[PRJCT_RESULTS_OMI_INDEX_ROW])
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Index of row","%d",pRecord->omi.omiRowIndex);
      if (pSpectra->fieldsFlag[PRJCT_RESULTS_OMI_GROUNDP_QF])
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"GroundPixel QF","%d",pRecord->omi.omiGroundPQF);
      if (pSpectra->fieldsFlag[PRJCT_RESULTS_OMI_XTRACK_QF])
       mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"XTrack QF","%d",pRecord->omi.omiXtrackQF);
    }
  else
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Record","%d/%d",pEngineContext->indexRecord,pEngineContext->recordNumber);

  if (strlen(pRecord->Nom) && (pSpectra->fieldsFlag[PRJCT_RESULTS_NAME]))
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Record name","%s",pRecord->Nom);

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

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_PIXEL])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Pixel number","%d",pRecord->gome.pixelNumber);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_PIXEL_TYPE])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Pixel type","%d",pRecord->gome.pixelType);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_ORBIT])
   {
    if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) || (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Orbit number","%d",pRecord->gome.orbitNumber+1);
    else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Orbit number","%d",pRecord->scia.orbitNumber);
   }

  // QDOAS ???     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
  // QDOAS ???      sprintf(tmpString,"Calibration parameters\t%.2f %.3e %.3e %.3e\n",pRecord->wavelength1,pRecord->dispersion[0],
  // QDOAS ???                  pRecord->dispersion[1],pRecord->dispersion[2]);
  // QDOAS ???

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_TINT])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Exposure time","%.3f sec",pRecord->Tint);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_SCANS])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scans taken into account","%d",pRecord->NSomme);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_NREJ])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Rejected scans","%d",pRecord->rejected);

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_SZA])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Solar Zenith angle","%-.3f",pRecord->Zm);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_AZIM])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Solar Azimuth angle","%.3f",pRecord->Azimuth);

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_VIEW_ELEVATION])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Elevation viewing angle","%.3f",pRecord->elevationViewAngle);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_VIEW_ZENITH])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Zenith viewing angle","%.3f",pRecord->zenithViewAngle);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_VIEW_AZIMUTH])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Azimuth viewing angle","%.3f",pRecord->azimuthViewAngle);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_LOS_ZA])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Viewing Zenith angle","%.3f",pRecord->zenithViewAngle);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_LOS_AZIMUTH])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Viewing Azimuth angle","%.3f",pRecord->azimuthViewAngle);

  if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MKZY)
   {
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_SCANNING])
     {
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scanning angle","%.3f",pRecord->mkzy.scanningAngle);
      mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scanning angle 2","%.3f",pRecord->mkzy.scanningAngle2);
     }
   }

  if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)
   {
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_SCANNING])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Telescope Scanning angle","%.3f",pRecord->als.scanningAngle);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_COMPASS])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scanning compass angle","%.3f",pRecord->als.compassAngle);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_PITCH])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scanning compass angle","%.3f",pRecord->als.pitchAngle);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_ROLL])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scanning compass angle","%.3f",pRecord->als.rollAngle);

    if (pSpectra->fieldsFlag[PRJCT_RESULTS_FILTERNUMBER])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Filter number","%d",pRecord->ccd.filterNumber);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_CCD_HEADTEMPERATURE])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Temperature in optic head","%.3f deg",pRecord->ccd.headTemperature);

    if (pSpectra->fieldsFlag[PRJCT_RESULTS_CCD_DIODES])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Diodes","%.3f %.3f %.3f %.3f",pRecord->ccd.diodes[0],pRecord->ccd.diodes[1],pRecord->ccd.diodes[2],pRecord->ccd.diodes[3]);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_CCD_TARGETAZIMUTH])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Target azimuth","%.3f deg",pRecord->ccd.targetAzimuth);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_CCD_TARGETELEVATION])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Target elevation","%.3f deg",pRecord->ccd.targetElevation);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_SATURATED])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Saturated","%s",(pRecord->ccd.saturatedFlag)?"yes":"no");
   }

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_MEASTYPE] && (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV))
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Measurement type","%s",CCD_measureTypes[pRecord->ccd.measureType]);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_MEASTYPE] && (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_BIRA))
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Measurement type","%s",MFCBIRA_measureTypes[pRecord->mfcBira.measurementType]);

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_TDET])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Detector temperature","%.3f",pRecord->TDet);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_MIRROR_ERROR])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Mirror status","%.3f",(pRecord->mirrorError==1)?"!!! PROBLEM !!!":"OK");
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_COOLING_STATUS])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Cooler status","%.3f",(pRecord->coolingStatus==0)?"!!! UNLOCKED !!!":"Locked");

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_LONGIT])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Longitude","%.3f",pRecord->longitude);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_LATIT])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Latitude","%.3f",pRecord->latitude);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_ALTIT])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Altitude","%.3f",pRecord->altitude);

  if (pSpectra->fieldsFlag[PRJCT_RESULTS_CLOUD])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Cloud fraction","%.3f",pRecord->cloudFraction);
  if (pSpectra->fieldsFlag[PRJCT_RESULTS_CLOUDTOPP])
   mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Cloud top pressure","%.3f",pRecord->cloudTopPressure);

  if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GOME2)
   {
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_GOME2_SCANDIRECTION])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Scan direction","%d",pRecord->gome2.scanDirection);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_GOME2_SAA])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"SAA flag","%d",pRecord->gome2.saaFlag);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_GOME2_SUNGLINT_RISK])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Sunglint risk flag","%d",pRecord->gome2.sunglintDangerFlag);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_GOME2_SUNGLINT_HIGHRISK])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Sunglint high risk flag","%d",pRecord->gome2.sunglintHighDangerFlag);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_GOME2_RAINBOW])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Rainbow flag","%d",pRecord->gome2.rainbowFlag);
   }

  if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_BIRA_AIRBORNE)
   {
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_UAV_SERVO_BYTE_SENT])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Servo position byte sent","%d",(int)pRecord->uavBira.servoSentPosition);
    if (pSpectra->fieldsFlag[PRJCT_RESULTS_UAV_SERVO_BYTE_RECEIVED])
     mediateResponseCellInfo(page,indexLine++,indexColumn,responseHandle,"Servo position byte received","%d",(int)pRecord->uavBira.servoReceivedPosition);
   }

  // Return

  return indexLine;
 }

// check if an output path is valid.
//
// for output file name: check if we can write to the file
//
// for automatic file names: just check if the root directory exists
RC check_output_path(const char* output_path) {

  RC rc = ERROR_ID_NO;

  const char *output_path_end = strrchr(output_path, PATH_SEP);

  // Just test that the output path exists (otherwise, "a" create new files and titles are not output

  // if ( strcmp("automatic", output_path_end + 1) != 0 ) {
  //   // if no automatic output filenames are chosen: check we can write to the file
  //   FILE *test = fopen(output_path, "a");
  //
  //   if (test == NULL) {
  //     rc = ERROR_SetLast("Output Path configuration error" , ERROR_TYPE_FATAL, ERROR_ID_FILE_OPEN, output_path);
  //   } else {
  //     fclose(test);
  //   }
  // } else // otherwise: automatic output files: just check if the directory exists

    if (output_path_end != NULL) {
      // if no path separator given, the output path is current working dir and we don't test
      char path[MAX_ITEM_TEXT_LEN + 1];
      size_t pathlen = output_path_end - output_path;
      strncpy(path, output_path, pathlen);
      path[pathlen]='\0';
      if ( STD_IsDir(path) != 1 ) {
        rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_DIR_NOT_FOUND, path, ", please create the directory of change the Output Path ");
      }
    }

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      MediateRequestPlotSpectra
// -----------------------------------------------------------------------------
// PURPOSE       Plot all the spectra related to a record
//
// INPUT         pEngineContext     pointer to the engine context
//
// RETURN        0 in case of success; the code of the error otherwise
// -----------------------------------------------------------------------------

void mediateRequestPlotSpectra(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
 {
   // Declarations

   PROJECT *pProject;                                                            // pointer to the project part of the engine context
   PRJCT_SPECTRA *pSpectra;                                                      // pointer to the spectra part of the project
   PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
   BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
   RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
   char tmpString[80];                                                           // buffer for formatted strings
   char *fileName;                                                              // the name of the current file
   plot_data_t spectrumData;

   // Initializations

   pRecord=&pEngineContext->recordInfo;
   pBuffers=&pEngineContext->buffers;
   pProject=&pEngineContext->project;
   pSpectra=&pProject->spectra;
   pInstrumental=&pProject->instrumental;

   fileName=pEngineContext->fileInfo.fileName;

   if (ANALYSE_plotRef)
    mediateResponseRetainPage(plotPageRef,responseHandle);
   if (ANALYSE_plotKurucz)
    mediateResponseRetainPage(plotPageCalib,responseHandle);

   sprintf(tmpString,"Spectrum (%d/%d)",pEngineContext->indexRecord,pEngineContext->recordNumber);

   if (pProject->spectra.displaySpectraFlag)
    {
     double *tempSpectrum;
     int i;

     if ((tempSpectrum=(double *)MEMORY_AllocDVector("mediateRequestPlotSpectra","tempSpectrum",0,NDET-1))!=NULL)
      {
       memcpy(tempSpectrum,pBuffers->spectrum,sizeof(double)*NDET);

       if (pEngineContext->buffers.instrFunction!=NULL)
        {
         for (i=0;i<NDET;i++)
          if (pBuffers->instrFunction[i]==(double)0.)
           tempSpectrum[i]=(double)0.;
          else
           tempSpectrum[i]/=pBuffers->instrFunction[i];
        }

       mediateAllocateAndSetPlotData(&spectrumData, "Spectrum",pBuffers->lambda, tempSpectrum, NDET, Line);
       mediateResponsePlotData(plotPageSpectrum, &spectrumData, 1, Spectrum, allowFixedScale, "Spectrum", "Wavelength (nm)", "Counts", responseHandle);
       mediateReleasePlotData(&spectrumData);
       mediateResponseLabelPage(plotPageSpectrum, fileName, tmpString, responseHandle);

       MEMORY_ReleaseDVector("mediateRequestPlotSpectra","spectrum",tempSpectrum,0);
      }

     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MKZY)
      {
       if ((pBuffers->offset!=NULL) && pEngineContext->recordInfo.mkzy.offsetFlag)
        {
         sprintf(tmpString,"Offset");

         mediateAllocateAndSetPlotData(&spectrumData, tmpString,pBuffers->lambda, pBuffers->offset, NDET, Line);
         mediateResponsePlotData(plotPageOffset, &spectrumData, 1, Spectrum, forceAutoScale, tmpString, "Wavelength (nm)", "Counts", responseHandle);
         mediateReleasePlotData(&spectrumData);
         mediateResponseLabelPage(plotPageOffset, fileName, tmpString, responseHandle);
        }

       if ((pBuffers->darkCurrent!=NULL) && pEngineContext->recordInfo.mkzy.darkFlag)
        {
         sprintf(tmpString,(pEngineContext->recordInfo.mkzy.offsetFlag)?"Dark current":"Offset");

         mediateAllocateAndSetPlotData(&spectrumData,tmpString,pBuffers->lambda, pBuffers->darkCurrent, NDET, Line);
         mediateResponsePlotData(plotPageDarkCurrent, &spectrumData, 1, Spectrum, forceAutoScale,tmpString, "Wavelength (nm)", "Counts", responseHandle);
         mediateReleasePlotData(&spectrumData);
         mediateResponseLabelPage(plotPageDarkCurrent, fileName, tmpString, responseHandle);
        }

       if ((pBuffers->scanRef!=NULL) && pEngineContext->recordInfo.mkzy.skyFlag)
        {
         sprintf(tmpString,"Sky spectrum");

         mediateAllocateAndSetPlotData(&spectrumData, "Sky spectrum",pBuffers->lambda, pBuffers->scanRef, NDET, Line);
         mediateResponsePlotData(plotPageIrrad, &spectrumData, 1, Spectrum, forceAutoScale, "Sky spectrum", "Wavelength (nm)", "Counts", responseHandle);
         mediateReleasePlotData(&spectrumData);
         mediateResponseLabelPage(plotPageIrrad, fileName, tmpString, responseHandle);
        }
      }

     if (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_ACTON) ||
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE) ||
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)) &&
         (pEngineContext->fileInfo.darkFp!=NULL) && (pBuffers->darkCurrent!=NULL))
      {
       sprintf(tmpString,"Dark current (%d/%d)",pEngineContext->indexRecord,pEngineContext->recordNumber);

       mediateAllocateAndSetPlotData(&spectrumData, "Dark current",pBuffers->lambda, pBuffers->darkCurrent, NDET, Line);
       mediateResponsePlotData(plotPageDarkCurrent, &spectrumData, 1, Spectrum, forceAutoScale, "Dark current", "Wavelength (nm)", "Counts", responseHandle);
       mediateReleasePlotData(&spectrumData);
       mediateResponseLabelPage(plotPageDarkCurrent, fileName, tmpString, responseHandle);
      }

     if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV) && (strlen(pInstrumental->imagePath)>0) && (pRecord->ccd.indexImage!=ITEM_NONE))
      {
       sprintf(tmpString,"Camera picture (%d/%d)",pEngineContext->indexRecord,pEngineContext->recordNumber);

       mediateAllocateAndSetPlotData(&spectrumData, "Camera picture",pBuffers->lambda, pBuffers->spectrum, NDET, Line);
       mediateResponsePlotData(plotPageImage, &spectrumData, 1, Spectrum, forceAutoScale, "Camera picture", "Wavelength (nm)", "Counts", responseHandle);
       mediateReleasePlotData(&spectrumData);
       mediateResponseLabelPage(plotPageImage, fileName, tmpString, responseHandle);
      }

     if (pBuffers->sigmaSpec!=NULL)
      {
       sprintf(tmpString,"Error (%d/%d)",pEngineContext->indexRecord,pEngineContext->recordNumber);

       mediateAllocateAndSetPlotData(&spectrumData, "Error",pBuffers->lambda, pBuffers->sigmaSpec, NDET, Line);
       mediateResponsePlotData(plotPageErrors, &spectrumData, 1, Spectrum, forceAutoScale, "Error", "Wavelength (nm)", "Counts", responseHandle);
       mediateReleasePlotData(&spectrumData);
       mediateResponseLabelPage(plotPageErrors, fileName, tmpString, responseHandle);
      }

     if ((pBuffers->irrad!=NULL) && ((pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_OMI) || (THRD_id==THREAD_TYPE_ANALYSIS)))
      {
       mediateAllocateAndSetPlotData(&spectrumData, "Irradiance spectrum",pBuffers->lambda, pBuffers->irrad, NDET, Line);
       mediateResponsePlotData(plotPageIrrad, &spectrumData, 1, Spectrum, forceAutoScale, "Irradiance spectrum", "Wavelength (nm)", "Counts", responseHandle);
       mediateReleasePlotData(&spectrumData);
       mediateResponseLabelPage(plotPageIrrad, fileName, "Irradiance", responseHandle);
      }

     if (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)) &&
         (pBuffers->specMax!=NULL) &&
         (pRecord->NSomme>1))
      {
      	sprintf(tmpString,"SpecMax (%d/%d)",pEngineContext->indexRecord,pEngineContext->recordNumber);

       mediateAllocateAndSetPlotData(&spectrumData, "SpecMax",pBuffers->specMaxx, pBuffers->specMax,pRecord->rejected+pRecord->NSomme, Line);
       mediateResponsePlotData(plotPageSpecMax, &spectrumData, 1, SpecMax, allowFixedScale, "SpecMax", "Scans number", "Signal Maximum", responseHandle);
       mediateReleasePlotData(&spectrumData);
       mediateResponseLabelPage(plotPageSpecMax, fileName, tmpString, responseHandle);
      }
    }

   if (pSpectra->displaySpectraFlag && pSpectra->displayDataFlag)
    mediateRequestDisplaySpecInfo(pEngineContext,plotPageSpectrum,responseHandle);

   //   QDOAS ???
   //   QDOAS ???     if (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
   //   QDOAS ???          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)) &&
   //   QDOAS ???         ((pRecord->aMoon!=(double)0.) || (pRecord->hMoon!=(double)0.) || (pRecord->fracMoon!=(double)0.)))
   //   QDOAS ???      {
   //   QDOAS ???       sprintf(tmpString,"Moon azimuthal angle\t%.3f\n",pRecord->aMoon);
   //   QDOAS ???       sprintf(tmpString,"Moon elevation\t\t%.3f\n",pRecord->hMoon);
   //   QDOAS ???       sprintf(tmpString,"Moon illuminated fraction\t%.3f\n",pRecord->fracMoon);
   //   QDOAS ???      }
   //   QDOAS ???     else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
   //   QDOAS ???      {
   //   QDOAS ???       GOME_ORBIT_FILE *pOrbitFile;
   //   QDOAS ???       pOrbitFile=&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];
   //   QDOAS ???
   //   QDOAS ???       sprintf(tmpString,"Cloud fraction\t\t%.2f\n",(float)pOrbitFile->gdpBinSpectrum.cloudFraction*0.01);
   //   QDOAS ???       sprintf(tmpString,"Cloud top pressure\t\t%.2f\n",(float)pOrbitFile->gdpBinSpectrum.cloudTopPressure*0.01);
   //   QDOAS ???       sprintf(tmpString,"[O3 VCD]\t\t\t%.2f DU\n",(float)pOrbitFile->gdpBinSpectrum.o3*0.01);
   //   QDOAS ???       sprintf(tmpString,"[No2 VCD]\t\t%.2e mol/cm2\n",(float)pOrbitFile->gdpBinSpectrum.no2*1.e13);
   //   QDOAS ???      }
   //   QDOAS ???
   //   QDOAS ???     sprintf(tmpString,"\n\n");
   //   QDOAS ???     fclose(fp);
   //   QDOAS ???    }
 }

// =================================
// CREATE/DESTROY THE ENGINE CONTEXT
// =================================

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestCreateEngineContext
// -----------------------------------------------------------------------------
// PURPOSE       This function is called on program start.  It creates a single
//               context for safely accessing its features through the mediator
//               layer.  The engine context is never destroyed before the user
//               exits the program.
//
// RETURN        On success 0 is returned and the value of handleEngine is set,
//               otherwise -1 is retured and the value of handleEngine is undefined.
// -----------------------------------------------------------------------------

int mediateRequestCreateEngineContext(void **engineContext, void *responseHandle)
 {
   ENGINE_CONTEXT *pEngineContext;

   if ((pEngineContext=*engineContext=(void *)EngineCreateContext())==NULL)
    ERROR_DisplayMessage(responseHandle);

   return (pEngineContext!=NULL)?0:-1;
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestDestroyEngineContext
// -----------------------------------------------------------------------------
// PURPOSE       Destroy the engine context when the user exits the program.
//
// RETURN        Zero is returned on success, -1 otherwise.
// -----------------------------------------------------------------------------

int mediateRequestDestroyEngineContext(void *engineContext, void *responseHandle)
 {
   return (!EngineDestroyContext(engineContext))?0:-1;
 }

// ==============================================================
// TRANSFER OF PROJECT PROPERTIES FROM THE MEDIATOR TO THE ENGINE
// ==============================================================

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectDisplay
// -----------------------------------------------------------------------------
// PURPOSE       Display part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectDisplay(PRJCT_SPECTRA *pEngineSpectra,const mediate_project_display_t *pMediateSpectra)
 {
   // Declaration

   INDEX i;

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionBegin("setMediateProjectDisplay",DEBUG_FCTTYPE_CONFIG);
#endif

   // Control of what to display

   pEngineSpectra->displaySpectraFlag=pMediateSpectra->requireSpectra;
   pEngineSpectra->displayDataFlag=pMediateSpectra->requireData;
   pEngineSpectra->displayCalibFlag=pMediateSpectra->requireCalib;
   pEngineSpectra->displayFitFlag=pMediateSpectra->requireFits;

   memset(pEngineSpectra->fieldsFlag,0,PRJCT_RESULTS_MAX);

   for (i=0;i<pMediateSpectra->selection.nSelected;i++)
    pEngineSpectra->fieldsFlag[(int)pMediateSpectra->selection.selected[i]]=(char)1;
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectSelection
// -----------------------------------------------------------------------------
// PURPOSE       Selection part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectSelection(PRJCT_SPECTRA *pEngineSpectra,const mediate_project_selection_t *pMediateSpectra)
 {
   // Declarations

   float tmp;

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionBegin("setMediateProjectSelection",DEBUG_FCTTYPE_CONFIG);
#endif

   // Spectral record range

   pEngineSpectra->noMin=pMediateSpectra->recordNumberMinimum;
   pEngineSpectra->noMax=pMediateSpectra->recordNumberMaximum;

   // SZA (Solar Zenith Angle) range of interest

   pEngineSpectra->SZAMin=(float)pMediateSpectra->szaMinimum;
   pEngineSpectra->SZAMax=(float)pMediateSpectra->szaMaximum;
   pEngineSpectra->SZADelta=(float)pMediateSpectra->szaDelta;

   // Cloud fraction

   pEngineSpectra->cloudMin=(float)pMediateSpectra->cloudFractionMinimum;
   pEngineSpectra->cloudMax=(float)pMediateSpectra->cloudFractionMaximum;

   if (pEngineSpectra->cloudMin>=pEngineSpectra->cloudMax+EPSILON)
    {
     tmp=pEngineSpectra->cloudMin;
     pEngineSpectra->cloudMin=pEngineSpectra->cloudMax;
     pEngineSpectra->cloudMax=tmp;
    }

   // QDOAS ??? to move to the instrumental page

   pEngineSpectra->namesFlag=pMediateSpectra->useNameFile;
   pEngineSpectra->darkFlag=pMediateSpectra->useDarkFile;

   // Geolocation

   pEngineSpectra->mode=pMediateSpectra->geo.mode;
   pEngineSpectra->radius=
     pEngineSpectra->longMin=
     pEngineSpectra->longMax=
     pEngineSpectra->latMin=
     pEngineSpectra->latMax=(double)0.;


   switch (pEngineSpectra->mode)
    {
     // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_CIRCLE :

      pEngineSpectra->radius=pMediateSpectra->geo.circle.radius;
      pEngineSpectra->longMin=(float)pMediateSpectra->geo.circle.centerLongitude;
      pEngineSpectra->latMin=(float)pMediateSpectra->geo.circle.centerLatitude;

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_RECTANGLE :

      pEngineSpectra->longMin=pMediateSpectra->geo.rectangle.westernLongitude;
      pEngineSpectra->longMax=pMediateSpectra->geo.rectangle.easternLongitude;
      pEngineSpectra->latMin=pMediateSpectra->geo.rectangle.southernLatitude;
      pEngineSpectra->latMax=pMediateSpectra->geo.rectangle.northernLatitude;

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_OBSLIST :
      pEngineSpectra->radius=pMediateSpectra->geo.sites.radius;
      break;
      // ----------------------------------------------------------------------------
    }

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_

   DEBUG_Print("noMin - noMax : %d - %d\n",pEngineSpectra->noMin,pEngineSpectra->noMax);
   DEBUG_Print("SZAMin - SZAMax - SZADelta : %.3f %.3f %.3f\n",pEngineSpectra->SZAMin,pEngineSpectra->SZAMax,pEngineSpectra->SZADelta);

   switch (pEngineSpectra->mode)
    {
     // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_CIRCLE :
      DEBUG_Print("Geo mode circle (radius,long,lat) : %.3f, %.3f, %.3f\n",pEngineSpectra->radius,pEngineSpectra->longMin,pEngineSpectra->longMax);
      break;
      // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_RECTANGLE :
      DEBUG_Print("Geo mode rectangle (long range, lat range) : %.3f - %.3f, %.3f - %.3f\n",
                  pEngineSpectra->longMin,pEngineSpectra->longMax,pEngineSpectra->latMin,pEngineSpectra->latMax);
      break;
      // ----------------------------------------------------------------------------
    case PRJCT_SPECTRA_MODES_OBSLIST :
      DEBUG_Print("Geo mode sites : %.3f\n",pEngineSpectra->radius);
      break;
      // ----------------------------------------------------------------------------
    }

   DEBUG_FunctionStop("setMediateProjectSelection",0);
#endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectAnalysis
// -----------------------------------------------------------------------------
// PURPOSE       Analysis part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectAnalysis(PRJCT_ANLYS *pEngineAnalysis,const mediate_project_analysis_t *pMediateAnalysis)
 {
#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_

   char *prjctAnlysMethods[PRJCT_ANLYS_METHOD_MAX]={"Optical density fitting","Intensity fitting (Marquardt-Levenberg+SVD)"};
   char *prjctAnlysFitWeighting[PRJCT_ANLYS_FIT_WEIGHTING_MAX]={"No weighting","Instrumental weighting"};
   char *prjctAnlysInterpol[PRJCT_ANLYS_INTERPOL_MAX]={"linear","spline"};

   DEBUG_FunctionBegin("setMediateProjectAnalysis",DEBUG_FCTTYPE_CONFIG);
#endif

   pEngineAnalysis->method=pMediateAnalysis->methodType;                         // analysis method
   pEngineAnalysis->fitWeighting=pMediateAnalysis->fitType;                      // least-squares fit weighting
   pEngineAnalysis->units=PRJCT_ANLYS_UNITS_NANOMETERS;                          // units for shift and stretch : force nm with QDOAS !!!
   pEngineAnalysis->interpol=pMediateAnalysis->interpolationType;                // interpolation
   pEngineAnalysis->convergence=pMediateAnalysis->convergenceCriterion;          // convergence criterion
   pEngineAnalysis->spike_tolerance=pMediateAnalysis->spike_tolerance;
   pEngineAnalysis->securityGap=pMediateAnalysis->interpolationSecurityGap;      // security pixels to take in order to avoid interpolation problems at the edge of the spectral window
   pEngineAnalysis->maxIterations=pMediateAnalysis->maxIterations;               // maximum number of iterations

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_Print("Analysis method : %s\n",prjctAnlysMethods[pEngineAnalysis->method]);
   DEBUG_Print("Least-squares fit weighting : %s\n",prjctAnlysFitWeighting[pEngineAnalysis->fitWeighting]);
   DEBUG_Print("Interpolation method : %s\n",prjctAnlysInterpol[pEngineAnalysis->interpol]);
   DEBUG_Print("Convergence criteria : %g\n",pEngineAnalysis->convergence);
   DEBUG_Print("Number of security pixels : %d\n",pEngineAnalysis->securityGap);

   DEBUG_FunctionStop("setMediateProjectAnalysis",0);
#endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectCalibration
// -----------------------------------------------------------------------------
// PURPOSE       Calibration part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectCalibration(PRJCT_KURUCZ *pEngineCalibration,CALIB_FENO *pEngineCalibFeno,const mediate_project_calibration_t *pMediateCalibration,int displayFitFlag)
 {
#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   char *prjctAnlysMethods[PRJCT_ANLYS_METHOD_MAX]={"Optical density fitting","Intensity fitting (Marquardt-Levenberg+SVD)"};
   DEBUG_FunctionBegin("setMediateProjectCalibration",DEBUG_FCTTYPE_CONFIG);
#endif

   strcpy(pEngineCalibration->file,pMediateCalibration->solarRefFile);           // kurucz file
   strcpy(pEngineCalibration->slfFile,pMediateCalibration->slfFile);             // slit function file

   pEngineCalibration->analysisMethod=pMediateCalibration->methodType;           // analysis method type
   pEngineCalibration->windowsNumber=pMediateCalibration->subWindows;            // number of windows
   pEngineCalibration->fwhmPolynomial=pMediateCalibration->sfpDegree;            // security gap in pixels numbers
   pEngineCalibration->shiftPolynomial=pMediateCalibration->shiftDegree;         // degree of polynomial to use

   if (displayFitFlag)
    {
     pEngineCalibration->displayFit=pMediateCalibration->requireFits;              // display fit flag
     pEngineCalibration->displayResidual=pMediateCalibration->requireResidual;     // display new calibration flag
     pEngineCalibration->displayShift=pMediateCalibration->requireShiftSfp;        // display shift/Fwhm in each pixel
     pEngineCalibration->displaySpectra=pMediateCalibration->requireSpectra;       // display fwhm in each pixel
    }
   else
    pEngineCalibration->displayFit=pEngineCalibration->displayResidual=pEngineCalibration->displayShift=pEngineCalibration->displaySpectra=0;

   pEngineCalibration->fwhmFit=(pMediateCalibration->lineShape>0)?1:0;           // force fit of fwhm while applying Kurucz
   pEngineCalibration->lambdaLeft=pMediateCalibration->wavelengthMin;            // minimum wavelength for the spectral interval
   pEngineCalibration->lambdaRight=pMediateCalibration->wavelengthMax;           // maximum wavelength for the spectral interval
   pEngineCalibration->invPolyDegree=pMediateCalibration->lorentzDegree;         // degree of the lorentzian  �

   switch(pMediateCalibration->lineShape)
    {
     // ---------------------------------------------------------------------------
    case PRJCT_CALIB_FWHM_TYPE_FILE :
      pEngineCalibration->fwhmType=SLIT_TYPE_FILE;
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_CALIB_FWHM_TYPE_ERF :
      pEngineCalibration->fwhmType=SLIT_TYPE_ERF;
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_CALIB_FWHM_TYPE_INVPOLY :
      pEngineCalibration->fwhmType=SLIT_TYPE_INVPOLY;
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_CALIB_FWHM_TYPE_VOIGT :
      pEngineCalibration->fwhmType=SLIT_TYPE_VOIGT;
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_CALIB_FWHM_TYPE_AGAUSS :
      pEngineCalibration->fwhmType=SLIT_TYPE_AGAUSS;
      break;
      // ---------------------------------------------------------------------------
    default :
      pEngineCalibration->fwhmType=SLIT_TYPE_GAUSS;
      break;
      // ---------------------------------------------------------------------------
    }

   // !!! tables will be loaded by mediateRequestSetAnalysisWindows !!!

   memcpy(&pEngineCalibFeno->crossSectionList,&pMediateCalibration->crossSectionList,sizeof(cross_section_list_t));
   memcpy(&pEngineCalibFeno->linear,&pMediateCalibration->linear,sizeof(struct anlyswin_linear));
   memcpy(pEngineCalibFeno->sfp,pMediateCalibration->sfp,sizeof(struct calibration_sfp)*4); // SFP1 .. SFP4
   memcpy(&pEngineCalibFeno->shiftStretchList,&pMediateCalibration->shiftStretchList,sizeof(shift_stretch_list_t));
   memcpy(&pEngineCalibFeno->outputList,&pMediateCalibration->outputList,sizeof(output_list_t));

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_Print("Analysis method : %s\n",prjctAnlysMethods[pMediateCalibration->methodType]);
   DEBUG_Print("Kurucz file : %s\n",pEngineCalibration->file);
   DEBUG_Print("Spectral range : %g - %g nm\n",pEngineCalibration->lambdaLeft,pEngineCalibration->lambdaRight);
   DEBUG_Print("Display %d %d %d %d (Spectra,Residual,Fit,Shift/SFP)\n",pEngineCalibration->displaySpectra,pEngineCalibration->displayResidual,pEngineCalibration->displayFit,pEngineCalibration->displayShift);
   DEBUG_Print("Number of windows: %d\n",pEngineCalibration->windowsNumber);
   DEBUG_Print("Degree of polynomials (shift/SFP) : %d - %d\n",pEngineCalibration->shiftPolynomial,pEngineCalibration->fwhmPolynomial);
   DEBUG_Print("Fit the slit function (slit function type) : %d (%d)\n",pEngineCalibration->fwhmFit,pEngineCalibration->fwhmType);
   DEBUG_FunctionStop("setMediateProjectCalibration",0);
#endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectUndersampling
// -----------------------------------------------------------------------------
// PURPOSE       Undersampling part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectUndersampling(PRJCT_USAMP *pEngineUsamp,const mediate_project_undersampling_t *pMediateUsamp)
 {
#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionBegin("setMediateProjectUndersampling",DEBUG_FCTTYPE_CONFIG);
#endif

   strcpy(pEngineUsamp->kuruczFile,pMediateUsamp->solarRefFile);

   pEngineUsamp->method=pMediateUsamp->method;
   pEngineUsamp->phase=pMediateUsamp->shift;

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionStop("setMediateProjectUndersampling",0);
#endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectInstrumental
// -----------------------------------------------------------------------------
// PURPOSE       Instrumental part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectInstrumental(PRJCT_INSTRUMENTAL *pEngineInstrumental,const mediate_project_instrumental_t *pMediateInstrumental)
 {
   INDEX indexCluster;

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionBegin("setMediateProjectInstrumental",DEBUG_FCTTYPE_CONFIG);
#endif

   pEngineInstrumental->readOutFormat=(char)pMediateInstrumental->format;                           // File format
   strcpy(pEngineInstrumental->observationSite,pMediateInstrumental->siteName); 		                   // Observation site
   NDET=0;

   switch (pEngineInstrumental->readOutFormat)
    {
     // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_ACTON :                                                                 // Acton (NILU)

      NDET=1024;                                                                     // size of the detector

      pEngineInstrumental->user=pMediateInstrumental->acton.niluType;                                // old or new format

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->acton.calibrationFile);      // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->acton.transmissionFunctionFile);      // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_LOGGER :                                                                // Logger (PDA,CCD or HAMAMATSU)

      NDET=1024;                                                                                     // size of the detector
      pEngineInstrumental->azimuthFlag=(int)pMediateInstrumental->logger.flagAzimuthAngle;           // format including or not the azimuth angle
      pEngineInstrumental->user=pMediateInstrumental->logger.spectralType;                           // spectrum type (offaxis or zenith)

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->logger.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->logger.transmissionFunctionFile);     // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_ASCII :                                                                 // Format ASCII

      NDET=pMediateInstrumental->ascii.detectorSize;                                                 // size of the detector

      pEngineInstrumental->ascii.format=pMediateInstrumental->ascii.format;                          // format line or column
      pEngineInstrumental->ascii.szaSaveFlag=pMediateInstrumental->ascii.flagZenithAngle;            // 1 if the solar zenith angle information is saved in the file
      pEngineInstrumental->ascii.azimSaveFlag=pMediateInstrumental->ascii.flagAzimuthAngle;          // 1 if the solar azimuth angle information is saved in the file
      pEngineInstrumental->ascii.elevSaveFlag=pMediateInstrumental->ascii.flagElevationAngle;        // 1 if the viewing elevation angle information is saved in the file
      pEngineInstrumental->ascii.timeSaveFlag=pMediateInstrumental->ascii.flagTime;                  // 1 if the time information is saved in the file
      pEngineInstrumental->ascii.dateSaveFlag=pMediateInstrumental->ascii.flagDate;                  // 1 if the date information is saved in the file
      pEngineInstrumental->ascii.lambdaSaveFlag=pMediateInstrumental->ascii.flagWavelength;          // 1 if the wavelength calibration is saved with spectra in the file

      pEngineInstrumental->offsetFlag=pMediateInstrumental->ascii.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->ascii.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->ascii.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->ascii.calibrationFile);      // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->ascii.transmissionFunctionFile);      // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDAEGG_OLD :                                                            // PDA EG&G (spring 94)

      NDET=1024;                                                                                     // size of the detector

      pEngineInstrumental->azimuthFlag=(int)0;                                                       // format including or not the azimuth angle
      pEngineInstrumental->user=PRJCT_INSTR_IASB_TYPE_ALL;                                           // spectrum type (offaxis or zenith)

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->pdaeggold.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->pdaeggold.transmissionFunctionFile);     // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDAEGG :                                                                // PDA EG&G (sept. 94 until now)

      NDET=1024;                                                                                     // size of the detector

      pEngineInstrumental->azimuthFlag=(int)pMediateInstrumental->pdaegg.flagAzimuthAngle;           // format including or not the azimuth angle
      pEngineInstrumental->user=pMediateInstrumental->pdaegg.spectralType;                           // spectrum type (offaxis or zenith)

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->pdaegg.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->pdaegg.transmissionFunctionFile);     // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDASI_EASOE :                                                           // PDA SI (IASB)

      NDET=1024;                                                                                     // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->pdasieasoe.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->pdasieasoe.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->pdasieasoe.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->pdasieasoe.calibrationFile); // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->pdasieasoe.transmissionFunctionFile); // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_OCEAN_OPTICS :                                                                 // Format OCEAN OPTICS

      NDET=pMediateInstrumental->oceanoptics.detectorSize;                                           // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->oceanoptics.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->oceanoptics.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->oceanoptics.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->oceanoptics.calibrationFile);      // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->oceanoptics.transmissionFunctionFile);      // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_VIS :                                                              // SAOZ PCD/NMOS 512

      NDET=512;                                                                                      // size of the detector

      pEngineInstrumental->saoz.spectralRegion=pMediateInstrumental->saozvis.spectralRegion;         // spectral region (UV or visible)
      pEngineInstrumental->saoz.spectralType=pMediateInstrumental->saozvis.spectralType;             // spectral type (zenith sky or pointed measuremets

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->saozvis.calibrationFile);    // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->saozvis.transmissionFunctionFile);    // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_EFM :                                                              // SAOZ EFM 1024

      NDET=1024;                                                                                     // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->saozefm.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->saozefm.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->saozefm.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->saozefm.calibrationFile);    // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->saozefm.transmissionFunctionFile);    // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_BIRA_AIRBORNE :                                                              // BIRA AIRBORNE

      NDET=2048;                                                                                     // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->biraairborne.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->biraairborne.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->biraairborne.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->biraairborne.calibrationFile);    // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->biraairborne.transmissionFunctionFile);    // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_RASAS :                                                                 // Format RASAS (INTA)

      NDET=1024;                                                                                     // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->rasas.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->rasas.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->rasas.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->rasas.calibrationFile);      // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->rasas.transmissionFunctionFile);      // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_NOAA :                                                                  // NOAA

      NDET=1024;                                                                                     // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->noaa.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->noaa.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->noaa.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->noaa.calibrationFile);       // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->noaa.transmissionFunctionFile);       // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SCIA_PDS :                                                              // SCIAMACHY calibrated Level 1 data in PDS format

      NDET=1024;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->sciapds.calibrationFile);    // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->sciapds.transmissionFunctionFile);    // instrumental function file
      strcpy(pEngineInstrumental->dnlFile,pMediateInstrumental->sciapds.detectorNonLinearityFile);   // correction for the non linearity of the detector

      pEngineInstrumental->scia.sciaChannel=pMediateInstrumental->sciapds.channel;

      for (indexCluster=SCIA_clusters[pEngineInstrumental->scia.sciaChannel][0];
           indexCluster<=SCIA_clusters[pEngineInstrumental->scia.sciaChannel][1];
           indexCluster++)

       pEngineInstrumental->scia.sciaCluster[indexCluster-SCIA_clusters[pEngineInstrumental->scia.sciaChannel][0]]=
         (pMediateInstrumental->sciapds.clusters[indexCluster])?1:0;

      memcpy(pEngineInstrumental->scia.sciaReference,pMediateInstrumental->sciapds.sunReference,4);

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GDP_ASCII :                                                               // GOME ASCII format

      NDET=1024;                                                                                     // Could be reduced by Set function

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->gdpascii.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->gdpascii.transmissionFunctionFile);     // instrumental function file

      pEngineInstrumental->gome.bandType=pMediateInstrumental->gdpascii.bandType;
      pEngineInstrumental->gome.pixelType=pMediateInstrumental->gdpascii.pixelType-1;

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GDP_BIN :                                                               // GOME WinDOAS BINARY format

      NDET=1024;                                                                                     // Could be reduced by Set function

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->gdpbin.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->gdpbin.transmissionFunctionFile);     // instrumental function file

      pEngineInstrumental->gome.bandType=pMediateInstrumental->gdpbin.bandType;
      pEngineInstrumental->gome.pixelType=pMediateInstrumental->gdpbin.pixelType-1;

      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_CCD_EEV :                                                               // CCD EEV 1340x400

      NDET=(pMediateInstrumental->ccdeev.detectorSize)?pMediateInstrumental->ccdeev.detectorSize:1340;
      pEngineInstrumental->user=pMediateInstrumental->ccdeev.spectralType;                           // spectrum type (offaxis or zenith)

      pEngineInstrumental->offsetFlag=pMediateInstrumental->ccdeev.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->ccdeev.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->ccdeev.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->ccdeev.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->ccdeev.transmissionFunctionFile);     // instrumental function file
      strcpy(pEngineInstrumental->imagePath,pMediateInstrumental->ccdeev.imagePath);     // instrumental function file

      // ---> not used for the moment : pMediateInstrumental->ccdeev.straylightCorrectionFile;
      // ---> not used for the moment : pMediateInstrumental->ccdeev.detectorNonLinearityFile;

      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GOME2 :                            // GOME2

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->gome2.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->gome2.transmissionFunctionFile);     // instrumental function file

      pEngineInstrumental->user=pMediateInstrumental->gome2.bandType;

      NDET=1024;
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_UOFT :                             // University of Toronto

      pEngineInstrumental->offsetFlag=pMediateInstrumental->uoft.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->uoft.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->uoft.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->uoft.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->uoft.transmissionFunctionFile);     // instrumental function file

      NDET=2048;
      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC :                              // MFC Heidelberg

      NDET=pEngineInstrumental->detectorSize=pMediateInstrumental->mfc.detectorSize;

      pEngineInstrumental->mfcRevert=pMediateInstrumental->mfc.revert;

      pEngineInstrumental->mfcMaskOffset=pMediateInstrumental->mfc.offsetMask;
      pEngineInstrumental->mfcMaskInstr=pMediateInstrumental->mfc.instrFctnMask;
      pEngineInstrumental->mfcMaskSpec=pMediateInstrumental->mfc.spectraMask;
      pEngineInstrumental->mfcMaskDark=pMediateInstrumental->mfc.darkCurrentMask;

      pEngineInstrumental->wavelength=pMediateInstrumental->mfc.firstWavelength;
      pEngineInstrumental->mfcMaskUse=pMediateInstrumental->mfc.autoFileSelect;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->mfc.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->mfc.transmissionFunctionFile);     // instrumental function file
      strcpy(pEngineInstrumental->vipFile,pMediateInstrumental->mfc.darkCurrentFile);             // dark current file
      strcpy(pEngineInstrumental->offsetFile,pMediateInstrumental->mfc.offsetFile);               // offset file

      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC_STD :                          // MFC Heidelberg (ASCII)

      NDET=pEngineInstrumental->detectorSize=pMediateInstrumental->mfcstd.detectorSize;

      pEngineInstrumental->mfcRevert=pMediateInstrumental->mfcstd.revert;
      pEngineInstrumental->offsetFlag=pMediateInstrumental->mfcstd.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->mfcstd.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->mfcstd.lambdaMax;

      strcpy(pEngineInstrumental->mfcStdDate,pMediateInstrumental->mfcstd.dateFormat);

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->mfcstd.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->mfcstd.transmissionFunctionFile);     // instrumental function file
      strcpy(pEngineInstrumental->vipFile,pMediateInstrumental->mfcstd.darkCurrentFile);             // dark current file
      strcpy(pEngineInstrumental->offsetFile,pMediateInstrumental->mfcstd.offsetFile);               // offset file

      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC_BIRA :                          // MFC BIRA (binary)

      pEngineInstrumental->offsetFlag=pMediateInstrumental->mfcbira.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->mfcbira.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->mfcbira.lambdaMax;

      NDET=pEngineInstrumental->detectorSize=pMediateInstrumental->mfcbira.detectorSize;
      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->mfcbira.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->mfcbira.transmissionFunctionFile);       // instrumental function file

      break;
      // ---------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MKZY :                                                                  // MKZY

      NDET=2048;                                                                                     // size of the detector

      pEngineInstrumental->offsetFlag=pMediateInstrumental->mkzy.straylight;
      pEngineInstrumental->lambdaMin=pMediateInstrumental->mkzy.lambdaMin;
      pEngineInstrumental->lambdaMax=pMediateInstrumental->mkzy.lambdaMax;

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->mkzy.calibrationFile);       // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->mkzy.transmissionFunctionFile);       // instrumental function file

      break;
      // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_OMI :

      NDET=1024;                                                                                  // Could be reduced by Set function

      memset(pEngineInstrumental->omi.omiTracks,0,sizeof(int)*MAX_SWATHSIZE);

      strcpy(pEngineInstrumental->calibrationFile,pMediateInstrumental->omi.calibrationFile);     // calibration file
      strcpy(pEngineInstrumental->instrFunction,pMediateInstrumental->omi.transmissionFunctionFile);     // instrumental function file

      pEngineInstrumental->omi.spectralType=pMediateInstrumental->omi.spectralType;
      pEngineInstrumental->omi.averageFlag=pMediateInstrumental->omi.flagAverage;
      pEngineInstrumental->omi.pixelQFRejectionFlag=pMediateInstrumental->omi.pixelQFRejectionFlag;
      pEngineInstrumental->omi.pixelQFMaxGaps=pMediateInstrumental->omi.pixelQFMaxGaps;
      pEngineInstrumental->omi.pixelQFMask=pMediateInstrumental->omi.pixelQFMask;
      pEngineInstrumental->omi.xtrack_mode=pMediateInstrumental->omi.xtrack_mode;

      OMI_TrackSelection((char *)pMediateInstrumental->omi.trackSelection,pEngineInstrumental->omi.omiTracks);

      break;
      // ----------------------------------------------------------------------------
    }

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionStop("setMediateProjectInstrumental",0);
#endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectSlit
// -----------------------------------------------------------------------------
// PURPOSE       Slit part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectSlit(PRJCT_SLIT *pEngineSlit,const mediate_project_slit_t *pMediateSlit)
 {
#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionBegin("setMediateProjectSlit",DEBUG_FCTTYPE_CONFIG);
#endif

   // Fields

   pEngineSlit->fwhmCorrectionFlag=pMediateSlit->applyFwhmCorrection;
   strcpy(pEngineSlit->kuruczFile,pMediateSlit->solarRefFile);

   setMediateSlit(&pEngineSlit->slitFunction,&pMediateSlit->function);

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionStop("setMediateProjectSlit",0);
#endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateProjectOutput
// -----------------------------------------------------------------------------
// PURPOSE       Output (binary/ascii format) part of the project properties
// -----------------------------------------------------------------------------

void setMediateProjectOutput(PRJCT_RESULTS *pEngineOutput,const mediate_project_output_t *pMediateOutput)
 {
   // Declarations

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   int i;
   DEBUG_FunctionBegin("setMediateProjectOutput",DEBUG_FCTTYPE_CONFIG);
#endif

   strcpy(pEngineOutput->path,pMediateOutput->path);                             // path for results and fits files
   strcpy(pEngineOutput->fluxes,pMediateOutput->flux);                           // fluxes
   strcpy(pEngineOutput->cic,pMediateOutput->colourIndex);                       // color indexes
   strcpy(pEngineOutput->swath_name,pMediateOutput->swath_name);

   pEngineOutput->file_format=pMediateOutput->file_format;

   pEngineOutput->analysisFlag=pMediateOutput->analysisFlag;
   pEngineOutput->calibFlag=pMediateOutput->calibrationFlag;
   pEngineOutput->referenceFlag=pMediateOutput->referenceFlag;
   pEngineOutput->dirFlag=pMediateOutput->directoryFlag;
   pEngineOutput->fileNameFlag=pMediateOutput->filenameFlag;

   if (!(pEngineOutput->fieldsNumber=pMediateOutput->selection.nSelected))
    memset(pEngineOutput->fieldsFlag,0,PRJCT_RESULTS_MAX);
   else
    memcpy(pEngineOutput->fieldsFlag,pMediateOutput->selection.selected,pEngineOutput->fieldsNumber);

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_Print("Path %s",pEngineOutput->path);
   DEBUG_Print("Fluxes %s",pEngineOutput->fluxes);
   DEBUG_Print("Cic %s",pEngineOutput->cic);
   DEBUG_FunctionStop("setMediateProjectOutput",0);
#endif
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestSetProject
// -----------------------------------------------------------------------------
// PURPOSE       Interface between the mediator and the engine for project properties
//
//               Project will be defined by the GUI. The engine is responsible for copying
//               any required data from project. The project details will remain valid
//               for the engine until the next call to mediateRequestSetProject.
//               project may be the null pointer, in which case the engine may free any
//               resources DIRECTLY associated with the project.
//
//               The operatingMode indicates the intended usage (Browsing, Analysis or Calibration).
//               It is a workaround that may ultimately be removed.
//
// RETURN        Zero is returned if the operation succeeded, -1 otherwise.
//               On success, project becomes the 'current project'.
// -----------------------------------------------------------------------------

int mediateRequestSetProject(void *engineContext,
			     const mediate_project_t *project,
			     int operatingMode,
			     void *responseHandle)
 {
   // Declarations

   ENGINE_CONTEXT *pEngineContext;                                               // engine context
   PROJECT *pEngineProject;                                                      // project part of the engine
   RC rc;                                                                        // return code

   // Initializations

   pEngineContext=(ENGINE_CONTEXT *)engineContext;
   pEngineProject=(PROJECT *)&pEngineContext->project;

   rc=ERROR_ID_NO;

   // Release buffers allocated at the previous session

   EngineEndCurrentSession(pEngineContext);

   THRD_id=operatingMode;

   // Transfer projects options from the mediator to the engine

   // TO DO : Initialize the pEngineProject->name

   setMediateProjectDisplay(&pEngineProject->spectra,&project->display);
   setMediateProjectSelection(&pEngineProject->spectra,&project->selection);
   setMediateProjectAnalysis(&pEngineProject->analysis,&project->analysis);
   setMediateFilter(&pEngineProject->lfilter,&project->lowpass,0,0);
   setMediateFilter(&pEngineProject->hfilter,&project->highpass,1,0);
   setMediateProjectCalibration(&pEngineProject->kurucz,&pEngineContext->calibFeno,&project->calibration,pEngineContext->project.spectra.displayCalibFlag);
   setMediateProjectInstrumental(&pEngineProject->instrumental,&project->instrumental);
   setMediateProjectUndersampling(&pEngineProject->usamp,&project->undersampling);
   setMediateProjectSlit(&pEngineProject->slit,&project->slit);
   setMediateProjectOutput(&pEngineProject->asciiResults,&project->output);

   // Allocate buffers requested by the project

   if (EngineSetProject(pEngineContext)!=ERROR_ID_NO)
    rc=ERROR_DisplayMessage(responseHandle);

   return (rc!=ERROR_ID_NO)?-1:0;    // supposed that an error at the level of the load of projects stops the current session
 }

// =======================================================================
// TRANSFER OF ANALYSIS WINDOWS PROPERTIES FROM THE MEDIATOR TO THE ENGINE
// =======================================================================

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestSetAnalysisLinear
// -----------------------------------------------------------------------------
// PURPOSE       Load linear parameters
// -----------------------------------------------------------------------------

RC mediateRequestSetAnalysisLinear(struct anlyswin_linear *pLinear,INDEX indexFenoColumn)
 {
   // Declarations

   ANALYSE_LINEAR_PARAMETERS linear[3];
   RC rc;

   // Debug

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionBegin("mediateRequestSetAnalysisLinear",DEBUG_FCTTYPE_CONFIG);
#endif

   // Polynomial (x)

   strcpy(linear[0].symbolName,"Polynomial (x)");

   linear[0].polyOrder=pLinear->xPolyOrder-1;
   linear[0].baseOrder=pLinear->xBaseOrder-1;
   linear[0].storeFit=pLinear->xFlagFitStore;
   linear[0].storeError=pLinear->xFlagErrStore;

   // Polynomial (1/x)

   // not used anymore strcpy(linear[1].symbolName,"Polynomial (1/x)");
   // not used anymore
   // not used anymore linear[1].polyOrder=pLinear->xinvPolyOrder-1;
   // not used anymore linear[1].baseOrder=pLinear->xinvBaseOrder-1;
   // not used anymore linear[1].storeFit=pLinear->xinvFlagFitStore;
   // not used anymore linear[1].storeError=pLinear->xinvFlagErrStore;

   // Linear offset

   strcpy(linear[1].symbolName,"Offset");

   linear[1].polyOrder=pLinear->offsetPolyOrder-1;
   linear[1].baseOrder=pLinear->offsetBaseOrder-1;
   linear[1].storeFit=pLinear->offsetFlagFitStore;
   linear[1].storeError=pLinear->offsetFlagErrStore;

   rc=ANALYSE_LoadLinear(linear,2,indexFenoColumn);

   // Debug

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionStop("mediateRequestSetAnalysisLinear",rc);
#endif

   // Return

   return rc;
 }

// Caro : in the future, replace structures anlyswin_nonlinear and calibration_sfp with the following one more flexible

// typedef struct _AnalyseNonLinearParameters
//  {
//  	char symbolName[MAX_ITEM_TEXT_LEN+1];
//  	char crossFileName[MAX_ITEM_TEXT_LEN+1];
//  	int fitFlag;
//  	double initialValue;
//  	double deltaValue;
//  	double minValue;
//  	double maxValue;
//  	int storeFit;
//  	int storeError;
//  }
// ANALYSE_NON_LINEAR_PARAMETERS;

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestSetAnalysisNonLinearCalib
// -----------------------------------------------------------------------------
// PURPOSE       Load non linear parameters for the calibration
// -----------------------------------------------------------------------------

#define NNONLINEAR_CALIB 4

RC mediateRequestSetAnalysisNonLinearCalib(ENGINE_CONTEXT *pEngineContext,struct calibration_sfp *nonLinearCalib,double *lambda,INDEX indexFenoColumn)
 {
   // Declarations

   ANALYSE_NON_LINEAR_PARAMETERS nonLinear[NNONLINEAR_CALIB];
   INDEX indexNonLinear;
   RC rc;

   // Debug

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionBegin("mediateRequestSetAnalysisNonLinearCalib",DEBUG_FCTTYPE_CONFIG);
#endif

   // Initialization

   memset(nonLinear,0,sizeof(ANALYSE_NON_LINEAR_PARAMETERS)*NNONLINEAR_CALIB);

   for (indexNonLinear=0;indexNonLinear<NNONLINEAR_CALIB;indexNonLinear++)
    {
     nonLinear[indexNonLinear].minValue=nonLinear[indexNonLinear].maxValue=(double)0.;
     sprintf(nonLinear[indexNonLinear].symbolName,"SFP %d",indexNonLinear+1);

     nonLinear[indexNonLinear].fitFlag=nonLinearCalib[indexNonLinear].fitFlag;
     nonLinear[indexNonLinear].initialValue=nonLinearCalib[indexNonLinear].initialValue;
     nonLinear[indexNonLinear].deltaValue=nonLinearCalib[indexNonLinear].deltaValue;
     nonLinear[indexNonLinear].storeFit=nonLinearCalib[indexNonLinear].fitStore;
     nonLinear[indexNonLinear].storeError=nonLinearCalib[indexNonLinear].errStore;
    }

   rc=ANALYSE_LoadNonLinear(pEngineContext,nonLinear,NNONLINEAR_CALIB,lambda,indexFenoColumn );

   // Debug

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionStop("mediateRequestSetAnalysisNonLinearCalib",rc);
#endif

   // Return

   return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestSetAnalysisNonLinearDoas
// -----------------------------------------------------------------------------
// PURPOSE       Load non linear parameters
// -----------------------------------------------------------------------------

#define NNONLINEAR_DOAS 8

RC mediateRequestSetAnalysisNonLinearDoas(ENGINE_CONTEXT *pEngineContext,struct anlyswin_nonlinear *pNonLinear,double *lambda, INDEX indexFenoColumn)
 {
   // Declarations

   ANALYSE_NON_LINEAR_PARAMETERS nonLinear[NNONLINEAR_DOAS];
   INDEX indexNonLinear;
   RC rc;

   // Debug

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionBegin("mediateRequestSetAnalysisNonLinearDoas",DEBUG_FCTTYPE_CONFIG);
#endif

   // Initialization

   memset(nonLinear,0,sizeof(ANALYSE_NON_LINEAR_PARAMETERS)*NNONLINEAR_DOAS);                 // this reset cross section file names
   // QDOAS MISSING FIELD !!!
   for (indexNonLinear=0;indexNonLinear<NNONLINEAR_DOAS;indexNonLinear++)
    nonLinear[indexNonLinear].minValue=nonLinear[indexNonLinear].maxValue=(double)0.;

   // Sol

   strcpy(nonLinear[0].symbolName,"Sol");

   nonLinear[0].fitFlag=pNonLinear->solFlagFit;
   nonLinear[0].initialValue=pNonLinear->solInitial;
   nonLinear[0].deltaValue=pNonLinear->solDelta;
   nonLinear[0].storeFit=pNonLinear->solFlagFitStore;
   nonLinear[0].storeError=pNonLinear->solFlagErrStore;

   // Offset (Constant)

   strcpy(nonLinear[1].symbolName,"Offset (Constant)");

   nonLinear[1].fitFlag=pNonLinear->off0FlagFit;
   nonLinear[1].initialValue=pNonLinear->off0Initial;
   nonLinear[1].deltaValue=pNonLinear->off0Delta;
   nonLinear[1].storeFit=pNonLinear->off0FlagFitStore;
   nonLinear[1].storeError=pNonLinear->off0FlagErrStore;

   // Offset (Order 1)

   strcpy(nonLinear[2].symbolName,"Offset (Order 1)");

   nonLinear[2].fitFlag=pNonLinear->off1FlagFit;
   nonLinear[2].initialValue=pNonLinear->off1Initial;
   nonLinear[2].deltaValue=pNonLinear->off1Delta;
   nonLinear[2].storeFit=pNonLinear->off1FlagFitStore;
   nonLinear[2].storeError=pNonLinear->off1FlagErrStore;

   // Offset (Order 2)

   strcpy(nonLinear[3].symbolName,"Offset (Order 2)");

   nonLinear[3].fitFlag=pNonLinear->off2FlagFit;
   nonLinear[3].initialValue=pNonLinear->off2Initial;
   nonLinear[3].deltaValue=pNonLinear->off2Delta;
   nonLinear[3].storeFit=pNonLinear->off2FlagFitStore;
   nonLinear[3].storeError=pNonLinear->off2FlagErrStore;

   // Com

   strcpy(nonLinear[4].symbolName,"Com");
   strcpy(nonLinear[4].crossFileName,pNonLinear->comFile);

   nonLinear[4].fitFlag=pNonLinear->comFlagFit;
   nonLinear[4].initialValue=pNonLinear->comInitial;
   nonLinear[4].deltaValue=pNonLinear->comDelta;
   nonLinear[4].storeFit=pNonLinear->comFlagFitStore;
   nonLinear[4].storeError=pNonLinear->comFlagErrStore;

   // Usamp1

   strcpy(nonLinear[5].symbolName,"Usamp1");
   strcpy(nonLinear[5].crossFileName,pNonLinear->usamp1File);

   nonLinear[5].fitFlag=pNonLinear->usamp1FlagFit;
   nonLinear[5].initialValue=pNonLinear->usamp1Initial;
   nonLinear[5].deltaValue=pNonLinear->usamp1Delta;
   nonLinear[5].storeFit=pNonLinear->usamp1FlagFitStore;
   nonLinear[5].storeError=pNonLinear->usamp1FlagErrStore;

   // Usamp2

   strcpy(nonLinear[6].symbolName,"Usamp2");
   strcpy(nonLinear[6].crossFileName,pNonLinear->usamp2File);

   nonLinear[6].fitFlag=pNonLinear->usamp2FlagFit;
   nonLinear[6].initialValue=pNonLinear->usamp2Initial;
   nonLinear[6].deltaValue=pNonLinear->usamp2Delta;
   nonLinear[6].storeFit=pNonLinear->usamp2FlagFitStore;
   nonLinear[6].storeError=pNonLinear->usamp2FlagErrStore;

   // Resol

   strcpy(nonLinear[7].symbolName,"Resol");

   nonLinear[7].fitFlag=pNonLinear->resolFlagFit;
   nonLinear[7].initialValue=pNonLinear->resolInitial;
   nonLinear[7].deltaValue=pNonLinear->resolDelta;
   nonLinear[7].storeFit=pNonLinear->resolFlagFitStore;
   nonLinear[7].storeError=pNonLinear->resolFlagErrStore;

   rc=ANALYSE_LoadNonLinear(pEngineContext,nonLinear,NNONLINEAR_DOAS,lambda,indexFenoColumn);

   // Debug

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
   DEBUG_FunctionStop("mediateRequestSetAnalysisNonLinearDoas",rc);
#endif

   // Return

   return rc;
 }

int mediateRequestSetAnalysisWindows(void *engineContext,
				     int numberOfWindows,
				     const mediate_analysis_window_t *analysisWindows,
				     int operatingMode,
				     void *responseHandle)
 {
   // Declarations

   double lambdaMin,lambdaMax;
   int useKurucz,                                                               // flag set if Kurucz is to be used in at least one analysis window
       useUsamp,                                                                // flag set if undersampling correction is requested in at least one analysis window
       xsToConvolute,                                                           // flag set if at least one cross section has to be convolved in at least one analysis window
       xsToConvoluteI0,                                                         // flag set if at least one cross section has to be I0-convolved in at least one analysis window
       saveFlag;
   INDEX indexKurucz,indexWindow;
   ENGINE_CONTEXT *pEngineContext;                                               // engine context
   PRJCT_INSTRUMENTAL *pInstrumental;
   mediate_analysis_window_t *pAnalysisWindows;                                  // pointer to the current analysis window from the user interface
   mediate_analysis_window_t calibWindows;                                       // pointer to the calibration parameters
   FENO *pTabFeno;                                                               // pointer to the description of an analysis window
   int indexFeno,indexFenoColumn,i;                                              // browse analysis windows
   MATRIX_OBJECT hr_solar_temp; // to preload high res solar spectrum
   RC rc;                                                                        // return code
   
   // Initializations

   lambdaMin=1000;
   lambdaMax=0;
   pEngineContext=(ENGINE_CONTEXT *)engineContext;
   pInstrumental=&pEngineContext->project.instrumental;
   saveFlag=(int)pEngineContext->project.spectra.displayDataFlag;
   useKurucz=useUsamp=xsToConvolute=xsToConvoluteI0=0;
   indexKurucz=ITEM_NONE;

   memset(&calibWindows,0,sizeof(mediate_analysis_window_t));
   memset(&hr_solar_temp, 0, sizeof(hr_solar_temp));

   memcpy(&calibWindows.crossSectionList,&pEngineContext->calibFeno.crossSectionList,sizeof(cross_section_list_t));
   memcpy(&calibWindows.linear,&pEngineContext->calibFeno.linear,sizeof(struct anlyswin_linear));
   memcpy(&calibWindows.shiftStretchList,&pEngineContext->calibFeno.shiftStretchList,sizeof(shift_stretch_list_t));
   memcpy(&calibWindows.outputList,&pEngineContext->calibFeno.outputList,sizeof(output_list_t));

   // Reinitialize all global variables used for the analysis, release old buffers and allocate new ones

   KURUCZ_indexLine=1;
   rc=ANALYSE_SetInit(pEngineContext);

   // if the user wants to write output to a file, check if the path is valid before starting analysis
   if ( (THRD_id==THREAD_TYPE_ANALYSIS && pEngineContext->project.asciiResults.analysisFlag) ||
        (THRD_id==THREAD_TYPE_KURUCZ && pEngineContext->project.asciiResults.calibFlag) ) {
     rc = check_output_path(pEngineContext->project.asciiResults.path);
     if (rc != ERROR_ID_NO)
       goto handle_errors;
   }

   if (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI)
    {
     if ((THRD_id==THREAD_TYPE_ANALYSIS) && numberOfWindows)
      {
       for (indexFeno=0;(indexFeno<numberOfWindows+1) && !rc;indexFeno++)
        if (strlen(analysisWindows[indexFeno].refOneFile)) // Analysis Window properties -> Reference 1.
         {
          rc=OMI_LoadReference(pEngineContext,(char *)analysisWindows[indexFeno].refOneFile);
          break;
         }
      }
     else if (THRD_id==THREAD_TYPE_KURUCZ)
      {
       if (!strlen(pEngineContext->project.instrumental.calibrationFile)) // Project properties ->Instrumental: Calibration File
         rc=ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_NOT_SPECIFIED,
                          "please specify an OMI irradiance file in Project properties>Instrumental>Calibration File");
       else
        rc=OMI_LoadReference(pEngineContext,(char *)pEngineContext->project.instrumental.calibrationFile);
      }
    }

   // Load analysis windows

   for (indexFenoColumn=0;(indexFenoColumn<ANALYSE_swathSize) && !rc;indexFenoColumn++)
    {
     if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) ||
         pEngineContext->project.instrumental.omi.omiTracks[indexFenoColumn])
      {
       NFeno=0;

       for (indexFeno=0;(indexFeno<numberOfWindows+1) && !rc;indexFeno++)
        {
         // Pointers initialization

         pTabFeno=(FENO *)&TabFeno[indexFenoColumn][NFeno];

         pTabFeno->hidden=!indexFeno;
         pAnalysisWindows=(!pTabFeno->hidden)?(mediate_analysis_window_t *)&analysisWindows[indexFeno-1]:(mediate_analysis_window_t *)&calibWindows;

         pTabFeno->NDET=NDET;

         if ((pTabFeno->hidden<2) && ((THRD_id==THREAD_TYPE_ANALYSIS) || (pTabFeno->hidden==1)))               // QDOAS : avoid the load of disabled analysis windows with hidden==2
          {
           if (pTabFeno->hidden)                                                     // if indexFeno==0, load calibration parameters
            {
             strcpy(pTabFeno->windowName,"Calibration description");                 // like WinDOAS
             pTabFeno->analysisMethod=pKuruczOptions->analysisMethod;
            }
           else                                                                       // otherwise, load analysis windows from analysisWindows[indexFeno-1]
            {
             // Load data from analysis windows panels

             strcpy(pTabFeno->windowName,pAnalysisWindows->name);
             strcpy(pTabFeno->residualsFile,pAnalysisWindows->residualFile);
             strcpy(pTabFeno->ref1,pAnalysisWindows->refOneFile);
             strcpy(pTabFeno->ref2,pAnalysisWindows->refTwoFile);

             if ((pTabFeno->refSpectrumSelectionMode=pAnalysisWindows->refSpectrumSelection)==ANLYS_REF_SELECTION_MODE_AUTOMATIC)
              {
               if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV) || (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD) ||
                   ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_ASCII) && pEngineContext->project.instrumental.ascii.elevSaveFlag))
                {
                 if ((pTabFeno->refMaxdoasSelectionMode=pAnalysisWindows->refMaxdoasSelection)==ANLYS_MAXDOAS_REF_SCAN)
                  pEngineContext->analysisRef.refScan++;
                 else if (pTabFeno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SZA)
                  pEngineContext->analysisRef.refSza++;
                }

               pTabFeno->refSZA=(double)pAnalysisWindows->refSzaCenter;
               pTabFeno->refSZADelta=(double)pAnalysisWindows->refSzaDelta;

               pTabFeno->refLatMin=pAnalysisWindows->refMinLatitude;
               pTabFeno->refLatMax=pAnalysisWindows->refMaxLatitude;
               pTabFeno->refLonMin=pAnalysisWindows->refMinLongitude;
               pTabFeno->refLonMax=pAnalysisWindows->refMaxLongitude;

               pTabFeno->cloudFractionMin=pAnalysisWindows->cloudFractionMin;
               pTabFeno->cloudFractionMax=pAnalysisWindows->cloudFractionMax;

               pTabFeno->gomePixelType[0]=pTabFeno->gomePixelType[1]=pTabFeno->gomePixelType[2]=pTabFeno->gomePixelType[3]=0;

               // GOME Pixel type : probably will be replaced later by a maximum value for LOS angle

               if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
                   (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))
                {
                 pTabFeno->gomePixelType[0]=pAnalysisWindows->pixelTypeEast;
                 pTabFeno->gomePixelType[1]=pAnalysisWindows->pixelTypeCenter;
                 pTabFeno->gomePixelType[2]=pAnalysisWindows->pixelTypeWest;
                 pTabFeno->gomePixelType[3]=pAnalysisWindows->pixelTypeBackscan;
                }

               pTabFeno->nspectra=pAnalysisWindows->refNs;

               pEngineContext->analysisRef.refAuto++;

               if ((fabs(pTabFeno->refLonMax-pTabFeno->refLonMin)>1.e-5) ) // && (fabs(pTabFeno->refLonMax-pTabFeno->refLonMin)<359.))
                pEngineContext->analysisRef.refLon++;
              }

             if (pEngineContext->project.spectra.displayFitFlag)
              {
               pTabFeno->displaySpectrum=pAnalysisWindows->requireSpectrum;
               pTabFeno->displayResidue=pAnalysisWindows->requireResidual;
               pTabFeno->displayTrend=pAnalysisWindows->requirePolynomial;
               pTabFeno->displayRefEtalon=pAnalysisWindows->requireRefRatio;
               pTabFeno->displayFits=pAnalysisWindows->requireFit;
               pTabFeno->displayPredefined=pAnalysisWindows->requirePredefined;

               pTabFeno->displayFlag=pTabFeno->displaySpectrum+
                 pTabFeno->displayResidue+
                 pTabFeno->displayTrend+
                 pTabFeno->displayRefEtalon+
                 pTabFeno->displayFits+
                 pTabFeno->displayPredefined;
              }

             pTabFeno->useKurucz=pAnalysisWindows->kuruczMode;

             pTabFeno->analysisMethod=pAnalysisOptions->method;
             useKurucz+=pAnalysisWindows->kuruczMode;
            }  // if (pTabFeno->hidden)

           pTabFeno->Decomp=1;

           // spikes buffer
           if ((pTabFeno->spikes == NULL) &&
               ((pTabFeno->spikes=(bool *)MEMORY_AllocBuffer("mediateRequestSetAnalysisWindows ","spikes",NDET,sizeof(int),0,MEMORY_TYPE_INT))==NULL))
            {
             rc = ERROR_ID_ALLOC;
             break;
            }

           // Wavelength scales read out

           if (((pTabFeno->Lambda==NULL) && ((pTabFeno->Lambda=MEMORY_AllocDVector("mediateRequestSetAnalysisWindows ","Lambda",0,NDET-1))==NULL)) ||
               ((pTabFeno->LambdaK==NULL) && ((pTabFeno->LambdaK=MEMORY_AllocDVector("mediateRequestSetAnalysisWindows ","LambdaK",0,NDET-1))==NULL)) ||
               ((pTabFeno->LambdaRef==NULL) && ((pTabFeno->LambdaRef=MEMORY_AllocDVector("mediateRequestSetAnalysisWindows ","LambdaRef",0,NDET-1))==NULL)) ||

               // omi rejected pixels

               ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI) && pEngineContext->project.instrumental.omi.pixelQFRejectionFlag &&
                (pTabFeno->omiRejPixelsQF == NULL) && ((pTabFeno->omiRejPixelsQF=(bool *)MEMORY_AllocBuffer("mediateRequestSetAnalysisWindows ","omiRejPixelsQF",NDET,sizeof(int),0,MEMORY_TYPE_INT))==NULL)))
            {
             rc=ERROR_ID_ALLOC;
             break;
            }

           for (i=0;i<NDET;i++)
            pTabFeno->LambdaRef[i]=i;  // NB : for satellites measurements, irradiance is retrieved later from spectra files

           //           if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GDP_ASCII) &&
           //               (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GDP_BIN) &&
           //               (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_HDF) &&
           //               (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS))

           if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI) && strlen(pInstrumental->calibrationFile) &&
               ((rc=OMI_GetReference(pInstrumental->calibrationFile,indexFenoColumn,pEngineContext->buffers.lambda,pEngineContext->buffers.spectrum,pEngineContext->buffers.sigmaSpec))!=ERROR_ID_NO))
            break;

           memcpy(pTabFeno->LambdaRef,pEngineContext->buffers.lambda,sizeof(double)*NDET);
           memcpy(pTabFeno->Lambda,pEngineContext->buffers.lambda,sizeof(double)*NDET);

           if (!(rc=ANALYSE_LoadRef(pEngineContext,indexFenoColumn)) &&   // eventually, modify LambdaRef for continuous functions
               !(rc=ANALYSE_LoadCross(pEngineContext,pAnalysisWindows->crossSectionList.crossSection,pAnalysisWindows->crossSectionList.nCrossSection,pTabFeno->hidden,pTabFeno->LambdaRef,indexFenoColumn)) &&
               !(rc=mediateRequestSetAnalysisLinear(&pAnalysisWindows->linear,indexFenoColumn)) &&

               // Caro : int the future, replace structures anlyswin_nonlinear and calibration_sfp with the following one more flexible
               //        mediateRequestSetAnalysisNonLinearDoas and mediateRequestSetAnalysisNonLinearCalib would be replaced by only one call to ANALYSE_LoadNonLinear

               ((!pTabFeno->hidden && !(rc=mediateRequestSetAnalysisNonLinearDoas(pEngineContext,&pAnalysisWindows->nonlinear,pTabFeno->LambdaRef,indexFenoColumn))) ||
                (pTabFeno->hidden && !(rc=mediateRequestSetAnalysisNonLinearCalib(pEngineContext,pEngineContext->calibFeno.sfp,pTabFeno->LambdaRef,indexFenoColumn)))) &&

               !(rc=ANALYSE_LoadShiftStretch(pAnalysisWindows->shiftStretchList.shiftStretch,pAnalysisWindows->shiftStretchList.nShiftStretch,indexFenoColumn)) &&
               !(rc=ANALYSE_LoadOutput(pAnalysisWindows->outputList.output,pAnalysisWindows->outputList.nOutput,indexFenoColumn)) &&
               (pTabFeno->hidden ||
                (!(rc=ANALYSE_LoadGaps(pEngineContext,pAnalysisWindows->gapList.gap,pAnalysisWindows->gapList.nGap,pTabFeno->LambdaRef,pAnalysisWindows->fitMinWavelength,pAnalysisWindows->fitMaxWavelength,indexFenoColumn)) &&

                 (!pTabFeno->gomeRefFlag || !(rc=SVD_LocalAlloc("ANALYSE_LoadData",&pTabFeno->svd)))
                 )))
            {
             if (pTabFeno->hidden==1)
              indexKurucz=NFeno;
             else
              {
               useUsamp+=pTabFeno->useUsamp;
               xsToConvolute+=pTabFeno->xsToConvolute;
               xsToConvoluteI0+=pTabFeno->xsToConvoluteI0;

               if (pTabFeno->gomeRefFlag || pEngineContext->refFlag)
                {
                 memcpy(pTabFeno->Lambda,pTabFeno->LambdaRef,sizeof(double)*NDET);
                 memcpy(pTabFeno->LambdaK,pTabFeno->LambdaRef,sizeof(double)*NDET);

                 if (pTabFeno->LambdaRef[NDET-1]-pTabFeno->Lambda[0]+1!=NDET)
                  rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LambdaRef,indexFenoColumn);
                }
              }

             ANALYSE_SetAnalysisType(indexFenoColumn);

             if (!pTabFeno->hidden)
              {
               lambdaMin=min(lambdaMin,pTabFeno->LambdaRef[0]);
               lambdaMax=max(lambdaMax,pTabFeno->LambdaRef[NDET-1]);
              }

             NFeno++;
            }
          } // if ((pTabFeno->hidden<2) && ((THRD_id==THREAD_TYPE_ANALYSIS) || (pTabFeno->hidden==1)))
        }  // for (indexFeno=0;(indexFeno<numberOfWindows+1) && !rc;indexFeno++)
      }  // if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) ||
    } // for (indexFenoColumn=0;(indexFenoColumn<ANALYSE_swathSize) && !rc;indexFenoColumn++)

   if (lambdaMin>=lambdaMax)
    {
     lambdaMin=pEngineContext->buffers.lambda[0];
     lambdaMax=pEngineContext->buffers.lambda[NDET-1];
    }

   if (rc)
     goto handle_errors;

   // load slit function from project properties -> slit page?
   // calibration procedure with FWHM fit -> Kurucz (and xs) are convolved with the fitted slit function
   // no calibration procedure and no xs to convolve -> nothing to do with the slit function in the slit page
   // other cases:
   if ( (useKurucz && !pKuruczOptions->fwhmFit) // calibration procedure but FWHM not fitted
        || (!useKurucz  && xsToConvolute) ) {   // no calibration procedure and xs to convolve
     // -> use the slit function in the slit page of project properties to convolve
     //    solar spectrum and xs
     rc=ANALYSE_LoadSlit(pSlitOptions,useKurucz||xsToConvoluteI0);
   }
   if (rc)
     goto handle_errors;

   if ((THRD_id==THREAD_TYPE_KURUCZ) || useKurucz) {
     // pre-load multi-row Kurucz reference spectrum one time, reuse it for each indexFenoColumn in KURUCZ_Alloc
     char kurucz_file[MAX_ITEM_TEXT_LEN+1];
     FILES_RebuildFileName(kurucz_file,(pKuruczOptions->fwhmFit)?pKuruczOptions->file:pSlitOptions->kuruczFile,1);

     if ( !strlen(kurucz_file) ) {
       rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_MSGBOX_FIELDEMPTY, "Solar Ref. File");
     } else {
       rc = MATRIX_Load(kurucz_file, &hr_solar_temp, 0,0,0,0, lambdaMin, lambdaMax, 1, 0, __func__);
     }
   }

   if (rc)
     goto handle_errors;

   for (indexFenoColumn=0;(indexFenoColumn<ANALYSE_swathSize) && !rc;indexFenoColumn++) {

     if ( (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) ||
          pEngineContext->project.instrumental.omi.omiTracks[indexFenoColumn]) {
       if ((xsToConvolute && !useKurucz) || !pKuruczOptions->fwhmFit)
         for (indexWindow=0;(indexWindow<NFeno) && !rc;indexWindow++) {
           pTabFeno=&TabFeno[indexFenoColumn][indexWindow];

           if ((pSlitOptions->slitFunction.slitType==SLIT_TYPE_NONE) && pTabFeno->xsToConvolute)
             rc = ERROR_SetLast("mediateRequestSetAnalysisWindows", ERROR_TYPE_FATAL, ERROR_ID_CONVOLUTION);
           //
           else if (pTabFeno->xsToConvolute && /* pTabFeno->useEtalon && */ (pTabFeno->gomeRefFlag || pEngineContext->refFlag) &&
                    ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LambdaRef,&ANALYSIS_slit,&ANALYSIS_slit2,pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2,indexFenoColumn,pSlitOptions->slitFunction.slitWveDptFlag))!=0))

             break;
         }

       if (!rc) {
         // Allocate Kurucz buffers on Run Calibration or
         //                            Run Analysis and wavelength calibration is different from None at least for one spectral window
         //
         // Apply the calibration procedure on the reference spectrum if the wavelength calibration is different from None at least for one spectral window
         if ((THRD_id==THREAD_TYPE_KURUCZ) || useKurucz) {
           rc=KURUCZ_Alloc(&pEngineContext->project,pEngineContext->buffers.lambda,indexKurucz,lambdaMin,lambdaMax,indexFenoColumn, &hr_solar_temp);
           if (!rc && useKurucz) {
             rc=KURUCZ_Reference(pEngineContext->buffers.instrFunction,0,saveFlag,1,responseHandle,indexFenoColumn);
           }
         }

         if (!rc && (THRD_id!=THREAD_TYPE_KURUCZ)) {
           rc=ANALYSE_AlignReference(pEngineContext,0,saveFlag,responseHandle,indexFenoColumn);
         }
       }

       if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI) && rc) {
         // Error on one irradiance spectrum shouldn't stop the analysis of other spectra
         for (indexWindow=0;indexWindow<NFeno;indexWindow++)
           TabFeno[indexFenoColumn][indexWindow].rcKurucz=rc;
         rc=ERROR_ID_NO;
       }
     }
   }

   // OMI SEE LATER

   if (!rc && !(rc=OUTPUT_RegisterData(pEngineContext)) &&
       (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) && useUsamp &&
       !(rc=ANALYSE_UsampGlobalAlloc(lambdaMin,lambdaMax,NDET)) &&
       !(rc=ANALYSE_UsampLocalAlloc(1)))
    rc=ANALYSE_UsampBuild(0,1);

 handle_errors:

   MATRIX_Free(&hr_solar_temp, __func__);

   if (rc!=ERROR_ID_NO)
    ERROR_DisplayMessage(responseHandle);

   return (rc!=ERROR_ID_NO)?-1:0;    // supposed that an error at the level of the load of projects stops the current session
 }

// ===============================================================
// TRANSFER OF THE LIST OF SYMBOLS FROM THE MEDIATOR TO THE ENGINE
// ===============================================================

int mediateRequestSetSymbols(void *engineContext,
			     int numberOfSymbols,
			     const mediate_symbol_t *symbols,
			     void *responseHandle)
 {
   // Declarations

   int indexSymbol;
   RC rc;

   // Initializations

   SYMB_itemCrossN=SYMBOL_PREDEFINED_MAX;
   rc=ERROR_ID_NO;

   // Add symbols in the list

   for (indexSymbol=0;(indexSymbol<numberOfSymbols) && !rc;indexSymbol++)
    rc=SYMB_Add((char *)symbols[indexSymbol].name,(char *)symbols[indexSymbol].description);

   // Check for error

   if (rc)
    ERROR_DisplayMessage(responseHandle);

   // Return

   return rc;
 }

// =========================================================================
// TRANSFER OF THE LIST OF OBSERVATION SITES FROM THE MEDIATOR TO THE ENGINE
// =========================================================================

int mediateRequestSetSites(void *engineContext,
			   int numberOfSites,
			   const mediate_site_t *sites,
			   void *responseHandle)
 {
   // Declarations

   int indexSite;
   RC rc;

   // Initializations

   SITES_itemN=0;
   rc=ERROR_ID_NO;

   // Add the observation site in the list

   for (indexSite=0;(indexSite<numberOfSites) && !rc;indexSite++)
    rc=SITES_Add((OBSERVATION_SITE *)&sites[indexSite]);

   // Check for error

   if (rc)
    ERROR_DisplayMessage(responseHandle);

   // Return

   return rc;
 }

// ==================
// BROWSING FUNCTIONS
// ==================

int mediateRequestBeginBrowseSpectra(void *engineContext,
				     const char *spectraFileName,
				     void *responseHandle)
 {
   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
   int rc;

   rc=ERROR_ID_NO;

   if (EngineRequestBeginBrowseSpectra(pEngineContext,spectraFileName,responseHandle)!=0)
    rc=ERROR_DisplayMessage(responseHandle);

   return ((rc==ERROR_ID_NO)?pEngineContext->recordNumber:-1);
 }

int mediateRequestGotoSpectrum(void *engineContext,
			       int recordNumber,
			       void *responseHandle)
 {
   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;

   if (recordNumber > 0 && recordNumber <= pEngineContext->recordNumber) {
    pEngineContext->currentRecord=recordNumber;

    return recordNumber;
   }
   else {
    pEngineContext->currentRecord = 0;
    return 0;
   }
 }

int mediateRequestNextMatchingSpectrum(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
 {
   // Declarations

   PROJECT *pProject;                                                            // pointer to the project part of the engine context
   RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context
   int orec=pEngineContext->indexRecord;
   int rec=pEngineContext->currentRecord;
   int upperLimit=pEngineContext->recordNumber;
   int inc,geoFlag;
   double longit,latit;
   INDEX indexSite;
   OBSERVATION_SITE *pSite;
   RC rc;

   // Initializations

   pProject=&pEngineContext->project;
   pRecord=&pEngineContext->recordInfo;
   inc=1;
   geoFlag=1;

   rc=ERROR_ID_NO;

   if (!pEngineContext->recordNumber)                                            // file is empty
    return 0;

   // IAP 200812 - set 'starting point' - next or goto
   if (pEngineContext->currentRecord) {
    // use this as the starting point rather than indexRecord
    rec = pEngineContext->currentRecord;
    // reset the currentRecord to 0 now that the GOTO request has been serviced.
    pEngineContext->currentRecord = 0;
   }
   else {
    rec = pEngineContext->indexRecord + 1; // start at the Next record
   }

   // consider increasing the starting point
   if (pProject->spectra.noMin && rec < pProject->spectra.noMin)
    rec = pProject->spectra.noMin;

   // consider reducing the upper boundary (inclusive)
   if (pProject->spectra.noMax && pProject->spectra.noMax < upperLimit)
    upperLimit = pProject->spectra.noMax;

   // Loop in search of a matching record - respect the min and max limits
   // in general 'break' to exit the loop

   while (rc == ERROR_ID_NO && rec <= upperLimit)
    {
     // read the 'next' record
     if ((rc=EngineReadFile(pEngineContext,rec,0,0))!=ERROR_ID_NO)
      {
       // reset the rc based on the severity of the failure - for non fatal errors keep searching
       rc = ERROR_DisplayMessage(responseHandle);
      }
     else if ( pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI &&
               !omi_use_track(pRecord->omi.omiXtrackQF, pProject->instrumental.omi.xtrack_mode) ) {
      // skip this spectrum
     } else
      {
       longit=pRecord->longitude;
       latit=pRecord->latitude;
       geoFlag=1;

       if ((pProject->spectra.mode==PRJCT_SPECTRA_MODES_CIRCLE) && (pProject->spectra.radius>1.) &&
           (THRD_GetDist(longit,latit,pProject->spectra.longMin,pProject->spectra.latMin)>(double)pProject->spectra.radius))
        geoFlag=0;
       else if ((pProject->spectra.mode==PRJCT_SPECTRA_MODES_OBSLIST) && (pProject->spectra.radius>1.))
        {
         for (indexSite=0;indexSite<SITES_itemN;indexSite++)
          {
           pSite=&SITES_itemList[indexSite];

           // QDOAS ???           if (!pSite->hidden)
           {
            if (THRD_GetDist(longit,latit,pSite->longitude,pSite->latitude)<=(double)pProject->spectra.radius)
             break;
           }
          }
         if (indexSite==SITES_itemN)
          geoFlag=0;
        }
       else if ((pProject->spectra.mode==PRJCT_SPECTRA_MODES_RECTANGLE) &&

                (((pProject->spectra.longMin!=pProject->spectra.longMax) &&
                  ((longit>max(pProject->spectra.longMin,pProject->spectra.longMax)) ||
                   (longit<min(pProject->spectra.longMin,pProject->spectra.longMax)))) ||

                 ((pProject->spectra.latMin!=pProject->spectra.latMax) &&
                  ((latit>max(pProject->spectra.latMin,pProject->spectra.latMax)) ||
                   (latit<min(pProject->spectra.latMin,pProject->spectra.latMax))))))

        geoFlag=0;

       if (geoFlag) {
        if (((fabs(pProject->spectra.SZAMin-pProject->spectra.SZAMax)<(double)1.e-4) ||
             ((pRecord->Zm>=pProject->spectra.SZAMin) && (pRecord->Zm<=pProject->spectra.SZAMax))) &&
            ((fabs(pProject->spectra.SZADelta)<(double)1.e-4) ||
             (fabs(pRecord->Zm-pRecord->oldZm)>pProject->spectra.SZADelta))) {
         // this record matches - exit the search loop
         break;
        }
       }

      }
      
     // try the next record
     rec+=inc;
    }

   if (rc != ERROR_ID_NO) {
    // search loop terminated due to fatal error - a message was already logged
    return -1;
   }
   else if (rec > upperLimit) {
    // did not find a matching record ... reread the last matching index (if there was one)
    // and return 0 to indicate the end of records.

    if (orec != 0) {
     if ((rc=EngineReadFile(pEngineContext,orec,0,0))!=ERROR_ID_NO)
      {
       ERROR_DisplayMessage(responseHandle);
       return -1; // error
      }
    }

    return 0; // No more matching records
   }

   return pEngineContext->indexRecord;
 }

// mediateRequestNextMatchingBrowseSpectrum
//
// attempt to locate and extract the data for the next spectral record in the current
// spectra file that matches the filter conditions of the current project. The search
// begins with the current spectral record. On success the data is extracted and
// pre-processed based on the settings of the current project. The spectrum data is
// returned with a call to
//    mediateResponsePlotData(page, plotDataArray, arrayLength, title, xLabel, yLabel, responseHandle);
//
// On success, the actual record number of the matching spectrum is returned. Zero is returned
// if a matching spectrum is not found. -1 is returned for all other errors and an error message
// should be posted with
//    mediateResponseErrorMessage(functionName, messageString, errorLevel, responseHandle);

int mediateRequestNextMatchingBrowseSpectrum(void *engineContext,
					     void *responseHandle)
 {
   // Declarations

   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;

   int rec = mediateRequestNextMatchingSpectrum(pEngineContext,responseHandle);

   if (rec > 0 && (pEngineContext->indexRecord<=pEngineContext->recordNumber)) {

    mediateRequestPlotSpectra(pEngineContext,responseHandle);
   }

   //  {
   //  	FILE *fp;
   //  	fp=fopen("qdoas.dbg","a+t");
   //  	fprintf(fp,"   mediateRequestNextMatchingBrowseSpectrum %d/%d\n",pEngineContext->indexRecord,pEngineContext->recordNumber);
   //  	fclose(fp);
   //  }


   return rec;
 }

int mediateRequestEndBrowseSpectra(void *engineContext,
				   void *responseHandle)
 {
   RC rc;

   if ((rc=EngineRequestEndBrowseSpectra((ENGINE_CONTEXT *)engineContext))!=0)
    ERROR_DisplayMessage(responseHandle);

   // Close open files and release allocated buffers to reset the engine context

   return 0;
 }

int mediateRequestBeginAnalyseSpectra(void *engineContext,
				      const char *spectraFileName,
				      void *responseHandle)
 {
   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
   RC rc = ERROR_ID_NO;

   rc=EngineRequestBeginBrowseSpectra(pEngineContext,spectraFileName,responseHandle);

   if (rc != ERROR_ID_NO) {
     ERROR_DisplayMessage(responseHandle);
     return -1;
   } else {
     return pEngineContext->recordNumber;
   }
 }

int mediateRequestNextMatchingAnalyseSpectrum(void *engineContext,
					      void *responseHandle)
 {
   // Declarations

   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
   RC rc = ERROR_ID_NO;

   int rec = mediateRequestNextMatchingSpectrum(pEngineContext,responseHandle);

   if (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI &&
       pEngineContext->analysisRef.refAuto)
    {
     // for omi, when using automatic reference selection for one or
     // more analysis windows, check if automatic reference spectrum is
     // ok for this detector row, if not: get next spectrum.
     while( !omi_has_automatic_reference(pEngineContext->recordInfo.omi.omiRowIndex-1)
            && rec > 0 )
      {
       rec = mediateRequestNextMatchingSpectrum(pEngineContext,responseHandle);
      }
    }

   if (rec > 0 && (pEngineContext->indexRecord<=pEngineContext->recordNumber))
    {
     mediateRequestPlotSpectra(pEngineContext,responseHandle);       
     
     if (!pEngineContext->analysisRef.refAuto || pEngineContext->satelliteFlag || ((rc=EngineNewRef(pEngineContext,responseHandle))==ERROR_ID_NO) )
      rc=ANALYSE_Spectrum(pEngineContext,responseHandle);

     if (rc!=ERROR_ID_NO)
      ERROR_DisplayMessage(responseHandle);
    }

   // NB if the function returns -1, the problem is that it is not possible to process
   // next records.

   return (rc != ERROR_ID_REF_ALIGNMENT) ? rec : -1;
 }

int mediateRequestPrevMatchingAnalyseSpectrum(void *engineContext,
					      void *responseHandle)
 {
   return 0;
 }

int mediateRequestEndAnalyseSpectra(void *engineContext,
				    void *responseHandle)
 {
   // Declarations

   RC rc;

   // Close open files and release allocated buffers to reset the engine context

   if ((rc=EngineRequestEndBrowseSpectra((ENGINE_CONTEXT *)engineContext))!=0)
    ERROR_DisplayMessage(responseHandle);

   // Return

   return 0;
 }


int mediateRequestBeginCalibrateSpectra(void *engineContext,
					const char *spectraFileName,
					void *responseHandle)
 {
   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
   RC rc;

   if ((rc=EngineRequestBeginBrowseSpectra(pEngineContext,spectraFileName,responseHandle))!=ERROR_ID_NO)
    ERROR_DisplayMessage(responseHandle);

   return (rc==ERROR_ID_NO)?((ENGINE_CONTEXT *)engineContext)->recordNumber:-1;
 }

int mediateRequestNextMatchingCalibrateSpectrum(void *engineContext,
						void *responseHandle)
 {
   // Declarations

   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
   RC rc = ERROR_ID_NO;

   int rec = mediateRequestNextMatchingSpectrum(pEngineContext,responseHandle);

   if (rec > 0 && (pEngineContext->indexRecord<=pEngineContext->recordNumber))
    {
     mediateRequestPlotSpectra(pEngineContext,responseHandle);

     if ((rc = ANALYSE_Spectrum(pEngineContext,responseHandle))!=ERROR_ID_NO)
      ERROR_DisplayMessage(responseHandle);
    }
   // NB if the function returns -1, the problem is that it is not possible to process
   // next records.
   return rec; // (rc == ERROR_ID_NO) ? rec : -1;
 }

int mediateRequestPrevMatchingCalibrateSpectrum(void *engineContext,
						void *responseHandle)
 {
   return 0;
 }

int mediateRequestEndCalibrateSpectra(void *engineContext,
				      void *responseHandle)
 {
   RC rc;

   // Close open files and release allocated buffers to reset the engine context

   if ((rc=EngineRequestEndBrowseSpectra((ENGINE_CONTEXT *)engineContext))!=0)
    ERROR_DisplayMessage(responseHandle);

   // Return

   return 0;
 }

int mediateRequestStop(void *engineContext,
		       void *responseHandle)
 {
   // Close open files and release allocated buffers to reset the engine context

   if (EngineRequestEndBrowseSpectra((ENGINE_CONTEXT *)engineContext)!=0)
    ERROR_DisplayMessage(responseHandle);

   return 0;
 }


int mediateRequestViewCrossSections(void *engineContext, char *awName,double minWavelength, double maxWavelength,
                                    int nFiles, char **filenames, void *responseHandle)
 {
   // Declarations

   ENGINE_CONTEXT *pEngineContext = (ENGINE_CONTEXT *)engineContext;
   char symbolName[MAX_ITEM_NAME_LEN+1],*ptr,                                   // the symbol name
     windowTitle[MAX_ITEM_TEXT_LEN+1],                                       // title to display at the top of the page
     tabTitle[MAX_ITEM_TEXT_LEN+1];                                          // title to display on the tab of the page
   MATRIX_OBJECT xs;                                                             // matrix to load the cross section
   INDEX indexFile;                                                              // browse files
   int   indexLine,indexColumn;                                                  // browse lines and column in the data page
   plot_data_t xs2plot;                                                          // cross  section to plot

   // Initializations

   sprintf(windowTitle,"Cross sections used in %s analysis window of project %s",awName,pEngineContext->project.name);   // !!! it would be nice to add also the project name
   sprintf(tabTitle,"%s.%s (XS)",pEngineContext->project.name,awName);
   indexLine=indexColumn=2;

   // Get index of selected analysis window in list

   for (indexFile=0;indexFile<nFiles;indexFile++,indexLine++)
    {
     // Reinitialize the matrix object

     memset(&xs,0,sizeof(MATRIX_OBJECT));

     // Retrieve the symbol name from the file

     if ((ptr=strrchr(filenames[indexFile],'/'))!=NULL)
      {
       strcpy(symbolName,ptr+1);
       if ((ptr=strchr(symbolName,'_'))!=NULL)
        *ptr=0;
      }
     else
      symbolName[0]='\0';

     // Load the file

     if (!MATRIX_Load(filenames[indexFile],&xs,0 /* line base */,0 /* column base */,0,0,
                      minWavelength,maxWavelength,
                      0,   // no derivatives
                      1,   // reverse vectors if needed
                      "mediateRequestViewCrossSections") && (xs.nl>1) && (xs.nc>1))
      {
       // Plot the cross section

       mediateAllocateAndSetPlotData(&xs2plot,symbolName,xs.matrix[0],xs.matrix[1],xs.nl,Line);
       mediateResponsePlotData(plotPageCross,&xs2plot,1,Spectrum,0,symbolName,"Wavelength","cm**2 / molec", responseHandle);
       mediateResponseLabelPage(plotPageCross,windowTitle,tabTitle, responseHandle);
       mediateReleasePlotData(&xs2plot);
       mediateResponseCellInfo(plotPageCross,indexLine,indexColumn,responseHandle,filenames[indexFile],"%s","Loaded");
      }
     else
      mediateResponseCellInfo(plotPageCross,indexLine,indexColumn,responseHandle,filenames[indexFile],"%s","Not found !!!");

     // Release the allocated buffers

     MATRIX_Free(&xs,"mediateRequestViewCrossSections");
    }

   // Return

   return 0;
 }

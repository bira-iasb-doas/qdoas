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


#include <QTextStream>

#include "CWorkSpace.h"
#include "CWProjectTree.h"
#include "CQdoasConfigWriter.h"

#include "constants.h"

#include "debugutil.h"

const char sTrue[] = "true";
const char sFalse[] = "false";

CQdoasConfigWriter::CQdoasConfigWriter(const CWProjectTree *projectTree) :
  m_projectTree(projectTree)
{
}

CQdoasConfigWriter::~CQdoasConfigWriter()
{
}

QString CQdoasConfigWriter::write(const QString &fileName)
{
  QString msg;
  FILE *fp = fopen(fileName.toAscii().constData(), "w");

  if (fp == NULL) {
    QTextStream stream(&msg);

    stream << "Failed to open file '" << fileName << "' for writing.";
    return msg;
  }

  // fp is open for writing ...
  int i, n;
  CWorkSpace *ws = CWorkSpace::instance();

  fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<qdoas>\n");

  // paths
  fprintf(fp, "  <paths>\n"
	  "    <!-- upto 10 paths can be specified (index 0 to 9). Any file or directory name in the  -->\n"
	  "    <!-- raw_spectra tree that begins with %%? (where ? is a single digit) is expanded with -->\n"
	  "    <!-- the correponding path.                                                            -->\n\n");

  for (i=0; i<10; ++i) {
    QString path = ws->path(i);
    if (!path.isNull()) {
      fprintf(fp, "    <path index=\"%d\">%s</path>\n", i, path.toAscii().constData());
    }
  }
  fprintf(fp, "  </paths>\n");

  // sites
  const mediate_site_t *siteList = ws->siteList(n);
  if (siteList != NULL) {
    fprintf(fp, "  <sites>\n");
    for (i=0; i<n; ++i) {
      fprintf(fp, "    <site name=\"%s\" abbrev=\"%s\" long=\"%.3f\" lat=\"%.3f\" alt=\"%.3f\" />\n",
	      siteList[i].name, siteList[i].abbreviation, siteList[i].longitude,
	      siteList[i].latitude, siteList[i].altitude);
    }
    fprintf(fp, "  </sites>\n");
    delete [] siteList;
  }

  // symbols
  const mediate_symbol_t *symbolList = ws->symbolList(n);
  if (symbolList != NULL) {
    fprintf(fp, "  <symbols>\n");
    for (i=0; i<n; ++i) {
      fprintf(fp, "    <symbol name=\"%s\" descr=\"%s\" />\n",
	      symbolList[i].name, symbolList[i].description);
    }
    fprintf(fp, "  </symbols>\n");
    delete [] symbolList;
  }

  
  writeProjects(fp);

  fprintf(fp, "</qdoas>\n");

  if (fclose(fp)) {
    QTextStream stream(&msg);

    stream << "Error writing to the project file '" << fileName << "'";
  }

  return msg;
}

void CQdoasConfigWriter::writeProjects(FILE *fp)
{
  const QTreeWidgetItem *item;
  const CProjectItem *projItem;
  const mediate_project_t *properties;
  QString projName;

  int n = m_projectTree->topLevelItemCount();
  int i = 0;

  while (i < n) {
    projItem = dynamic_cast<const CProjectItem*>(m_projectTree->topLevelItem(i));
    if (projItem != NULL && projItem->childCount() == 2) {
      // Should always be a project item ... with two children
      projName = projItem->text(0);

      properties = CWorkSpace::instance()->findProject(projName);
      if (properties != NULL) {
	// write the project data
	fprintf(fp, "  <project name=\"%s\">\n", projName.toAscii().constData());
	writeProperties(fp, properties);

	// Analysis Windows ...
	item = projItem->child(1); // Analysis Windows Branch
	writeAnalysisWindows(fp, projName, item);

	// Raw spectra ...
	item = projItem->child(0); // Raw Spectra Branch
	writeRawSpectraTree(fp, item);

	fprintf(fp, "  </project>\n");
      }
    }
    ++i;
  }
}

void CQdoasConfigWriter::writeProperties(FILE *fp, const mediate_project_t *d)
{
  writePropertiesDisplay(fp, &(d->display));
  writePropertiesSelection(fp, &(d->selection));
  writePropertiesAnalysis(fp, &(d->analysis));
  writePropertiesFilter(fp, "low", &(d->lowpass));
  writePropertiesFilter(fp, "high", &(d->highpass));
  writePropertiesCalibration(fp, &(d->calibration));
  writePropertiesUndersampling(fp, &(d->undersampling));
  writePropertiesInstrumental(fp, &(d->instrumental));
  writePropertiesSlit(fp, &(d->slit));
  writePropertiesOutput(fp, &(d->output));
  writePropertiesNasaAmes(fp, &(d->nasaames));
}

void CQdoasConfigWriter::writePropertiesDisplay(FILE *fp, const mediate_project_display_t *d)
{
  fprintf(fp, "    <display spectra=\"%s\" data=\"%s\" fits=\"%s\">\n",
	  (d->requireSpectra ? sTrue : sFalse), (d->requireData ? sTrue : sFalse), (d->requireFits ? sTrue : sFalse));

  writeDataSelectList(fp, &(d->selection));
  
  fprintf(fp, "    </display>\n");

}

void CQdoasConfigWriter::writePropertiesSelection(FILE *fp, const mediate_project_selection_t *d)
{
  fprintf(fp, "    <selection>\n");
  fprintf(fp, "      <sza min=\"%.3f\" max=\"%.3f\" delta=\"%.3f\" />\n", d->szaMinimum, d->szaMaximum, d->szaDelta);
  fprintf(fp, "      <record min=\"%d\" max=\"%d\" />\n", d->recordNumberMinimum, d->recordNumberMaximum);
  switch (d->geo.mode) {
  case PRJCT_SPECTRA_MODES_CIRCLE:
    fprintf(fp, "      <geolocation selected=\"circle\">\n");
    break;
  case PRJCT_SPECTRA_MODES_RECTANGLE:
    fprintf(fp, "      <geolocation selected=\"rectangle\">\n");
    break;
  case PRJCT_SPECTRA_MODES_OBSLIST:
    fprintf(fp, "      <geolocation selected=\"sites\">\n");
    break;
  default:
     fprintf(fp, "      <geolocation selected=\"none\">\n");
  }
  fprintf(fp, "        <circle radius=\"%.3f\" long=\"%.3f\" lat=\"%.3f\" />\n",
	  d->geo.circle.radius, d->geo.circle.centerLongitude, d->geo.circle.centerLatitude);
  fprintf(fp, "        <rectangle west=\"%.3f\" east=\"%.3f\" south=\"%.3f\" north=\"%.3f\" />\n",
	  d->geo.rectangle.westernLongitude, d->geo.rectangle.easternLongitude,
	  d->geo.rectangle.southernLatitude, d->geo.rectangle.northernLatitude);
  fprintf(fp, "        <sites radius=\"%.3f\" />\n", d->geo.sites.radius);
  fprintf(fp, "      </geolocation>\n    </selection>\n");

}

void CQdoasConfigWriter::writePropertiesAnalysis(FILE *fp, const mediate_project_analysis_t *d)
{
  fprintf(fp, "    <analysis method=");
  switch (d->methodType) {
  case PRJCT_ANLYS_METHOD_SVD:
    fprintf(fp, "\"ODF\"");
    break;
  case PRJCT_ANLYS_METHOD_SVDMARQUARDT:
    fprintf(fp, "\"ML+SVD\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " fit=");
  switch (d->fitType) {
  case PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL:
    fprintf(fp, "\"instr\"");
    break;
  default:
    fprintf(fp, "\"none\"");
  }
  fprintf(fp, " unit=");
  switch (d->unitType) {
  case PRJCT_ANLYS_UNITS_PIXELS:
    fprintf(fp, "\"pixel\"");
    break;
  case PRJCT_ANLYS_UNITS_NANOMETERS:
    fprintf(fp, "\"nm\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " interpolation=");
  switch (d->interpolationType) {
  case PRJCT_ANLYS_INTERPOL_LINEAR:
    fprintf(fp, "\"linear\"");
    break;
  case PRJCT_ANLYS_INTERPOL_SPLINE:
    fprintf(fp, "\"spline\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " gap=\"%d\" converge=\"%g\" >\n", d->interpolationSecurityGap, d->convergenceCriterion);
  fprintf(fp, "      <!-- method        : ODF ML+SVD -->\n"
	  "      <!-- fit           : none instr -->\n"
	  "      <!-- unit          : pixel nm -->\n"
	  "      <!-- interpolation : linear spline -->\n"
	  "    </analysis>\n");
}

void CQdoasConfigWriter::writePropertiesFilter(FILE *fp, const char *passband, const mediate_filter_t *d)
{
  fprintf(fp, "    <%spass_filter selected=", passband); // low or high
  switch (d->mode) {
  case PRJCT_FILTER_TYPE_KAISER:
    fprintf(fp, "\"kaiser\"");
    break;
  case PRJCT_FILTER_TYPE_BOXCAR:
    fprintf(fp, "\"boxcar\"");
    break;
  case PRJCT_FILTER_TYPE_GAUSSIAN:
    fprintf(fp, "\"gaussian\"");
    break;
  case PRJCT_FILTER_TYPE_TRIANGLE:
    fprintf(fp, "\"triangular\"");
    break;
  case PRJCT_FILTER_TYPE_SG:
    fprintf(fp, "\"savitzky\"");
    break;
  case PRJCT_FILTER_TYPE_ODDEVEN:
    fprintf(fp, "\"oddeven\"");
    break;
  case PRJCT_FILTER_TYPE_BINOMIAL:
    fprintf(fp, "\"binomial\"");
    break;
  default:
    fprintf(fp, "\"none\"");
  }
  fprintf(fp, ">\n");

  fprintf(fp, "      <kaiser cutoff=\"%f\" tolerance=\"%f\" passband=\"%f\" iterations=\"%d\" />\n",
	  d->kaiser.cutoffFrequency, d->kaiser.tolerance, d->kaiser.passband, d->kaiser.iterations);
  fprintf(fp, "      <boxcar width=\"%d\" iterations=\"%d\" />\n",
	  d->boxcar.width, d->boxcar.iterations);
  fprintf(fp, "      <gaussian fwhm=\"%f\" iterations=\"%d\" />\n",
	  d->gaussian.fwhm, d->gaussian.iterations);
  fprintf(fp, "      <triangular width=\"%d\" iterations=\"%d\" />\n",
	  d->triangular.width, d->triangular.iterations);
  fprintf(fp, "      <savitzky_golay width=\"%d\" order=\"%d\" iterations=\"%d\" />\n",
	  d->savitzky.width, d->savitzky.order, d->savitzky.iterations);
  fprintf(fp, "      <binomial width=\"%d\" iterations=\"%d\" />\n",
	  d->binomial.width, d->binomial.iterations);
  fprintf(fp, "    </%spass_filter>\n", passband);
}

void CQdoasConfigWriter::writePropertiesCalibration(FILE *fp, const mediate_project_calibration_t *d)
{
  fprintf(fp, "    <calibration ref=\"%s\" method=", d->solarRefFile);
  switch (d->methodType) {
  case PRJCT_ANLYS_METHOD_SVD:
    fprintf(fp, "\"ODF\"");
    break;
  case PRJCT_ANLYS_METHOD_SVDMARQUARDT:
    fprintf(fp, "\"ML+SVD\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, ">\n      <line shape=");
  switch (d->lineShape) {
  case PRJCT_CALIB_FWHM_TYPE_GAUSS:
    fprintf(fp, "\"gauss\"");
    break;
  case PRJCT_CALIB_FWHM_TYPE_ERF:
    fprintf(fp, "\"error\"");
    break;
  case PRJCT_CALIB_FWHM_TYPE_INVPOLY:
    fprintf(fp, "\"lorentz\"");
    break;
  case PRJCT_CALIB_FWHM_TYPE_VOIGT:
    fprintf(fp, "\"voigt\"");
    break;
  default:
    fprintf(fp, "\"none\"");
  }
  fprintf(fp, " lorentzdegree=\"%d\" />\n", d->lorentzDegree);
  fprintf(fp, "      <display shape=\"%s\" fits=\"%s\" residual=\"%s\" shiftsfp=\"%s\" />\n",
	  (d->requireSpectra ? sTrue : sFalse), (d->requireFits ? sTrue : sFalse),
	  (d->requireResidual ? sTrue : sFalse), (d->requireShiftSfp ? sTrue : sFalse));
  fprintf(fp, "      <polynomial shift=\"%d\" sfp=\"%d\" />\n", d->shiftDegree, d->sfpDegree);
  fprintf(fp, "      <window min=\"%.1f\" max=\"%.1f\" intervals=\"%d\" />\n",
	  d->wavelengthMin, d->wavelengthMax, d->subWindows);

  writeCrossSectionList(fp, &(d->crossSectionList));
  writeSfps(fp, &(d->sfp[0]));
  writeLinear(fp, &(d->linear));
  writeShiftStretchList(fp, &(d->shiftStretchList));
  writeGapList(fp, &(d->gapList));
  writeOutputList(fp, &(d->outputList));

  fprintf(fp, "    </calibration>\n");
}

void CQdoasConfigWriter::writePropertiesUndersampling(FILE *fp, const mediate_project_undersampling_t *d)
{
  fprintf(fp, "    <undersampling ref=\"%s\" method=", d->solarRefFile);
  switch (d->method) {
  case PRJCT_USAMP_FILE:
    fprintf(fp, "\"file\"");
    break;
  case PRJCT_USAMP_FIXED:
    fprintf(fp, "\"fixed\"");
    break;
  case PRJCT_USAMP_AUTOMATIC:
    fprintf(fp, "\"auto\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " shift=\"%f\" />\n", d->shift);
}

void CQdoasConfigWriter::writePropertiesInstrumental(FILE *fp, const mediate_project_instrumental_t *d)
{
  int i;
  QString tmpStr;
  CWorkSpace *ws = CWorkSpace::instance();

  fprintf(fp, "    <instrumental format=");
  switch (d->format) {
  case PRJCT_INSTR_FORMAT_ASCII:
    fprintf(fp, "\"ascii\"");
    break;
  case PRJCT_INSTR_FORMAT_LOGGER:
    fprintf(fp, "\"logger\"");
    break;
  case PRJCT_INSTR_FORMAT_ACTON:
    fprintf(fp, "\"acton\"");
    break;
  case PRJCT_INSTR_FORMAT_PDAEGG:
    fprintf(fp, "\"pdaegg\"");
    break;
  case PRJCT_INSTR_FORMAT_PDAEGG_OLD:
    fprintf(fp, "\"pdaeggold\"");
    break;
  case PRJCT_INSTR_FORMAT_PDAEGG_ULB:
    fprintf(fp, "\"pdaeggulb\"");
    break;
  case PRJCT_INSTR_FORMAT_CCD_OHP_96:
    fprintf(fp, "\"ccdohp96\"");
    break;
  case PRJCT_INSTR_FORMAT_CCD_HA_94:
    fprintf(fp, "\"ccdha94\"");
    break;
  case PRJCT_INSTR_FORMAT_CCD_ULB:
    fprintf(fp, "\"ccdulb\"");
    break;
  case PRJCT_INSTR_FORMAT_SAOZ_VIS:
    fprintf(fp, "\"saozvis\"");
    break;
  case PRJCT_INSTR_FORMAT_SAOZ_UV:
    fprintf(fp, "\"saozuv\"");
    break;
  case PRJCT_INSTR_FORMAT_SAOZ_EFM:
    fprintf(fp, "\"saozefm\"");
    break;
  case PRJCT_INSTR_FORMAT_MFC:
    fprintf(fp, "\"mfc\"");
    break;
  case PRJCT_INSTR_FORMAT_MFC_STD:
    fprintf(fp, "\"mfcstd\"");
    break;
  case PRJCT_INSTR_FORMAT_RASAS:
    fprintf(fp, "\"rasas\"");
    break;
  case PRJCT_INSTR_FORMAT_PDASI_EASOE:
    fprintf(fp, "\"pdasieasoe\"");
    break;
  case PRJCT_INSTR_FORMAT_PDASI_OSMA:
    fprintf(fp, "\"osma\"");
    break;
  case PRJCT_INSTR_FORMAT_CCD_EEV:
    fprintf(fp, "\"ccdeev\"");
    break;
  case PRJCT_INSTR_FORMAT_OPUS:
    fprintf(fp, "\"opus\"");
    break;
  case PRJCT_INSTR_FORMAT_GDP_ASCII:
    fprintf(fp, "\"gdpascii\"");
    break;
  case PRJCT_INSTR_FORMAT_GDP_BIN:
    fprintf(fp, "\"gdpbin\"");
    break;
  case PRJCT_INSTR_FORMAT_SCIA_HDF:
    fprintf(fp, "\"sciahdf\"");
    break;
  case PRJCT_INSTR_FORMAT_SCIA_PDS:
    fprintf(fp, "\"sciapds\"");
    break;
  case PRJCT_INSTR_FORMAT_UOFT:
    fprintf(fp, "\"uoft\"");
    break;
  case PRJCT_INSTR_FORMAT_NOAA:
    fprintf(fp, "\"noaa\"");
    break;
  case PRJCT_INSTR_FORMAT_OMI:
    fprintf(fp, "\"omi\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " site=\"%s\">\n", d->siteName);

  // ascii
  fprintf(fp, "      <ascii size=\"%d\" format=", d->ascii.detectorSize);
  switch (d->ascii.format) {
  case PRJCT_INSTR_ASCII_FORMAT_LINE:
    fprintf(fp, "\"line\"");
    break;
  case PRJCT_INSTR_ASCII_FORMAT_COLUMN:
    fprintf(fp, "\"column\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " zen=\"%s\" azi=\"%s\" ele=\"%s\" date=\"%s\" time=\"%s\" lambda=\"%s\"",
	  (d->ascii.flagZenithAngle ? sTrue : sFalse), (d->ascii.flagAzimuthAngle ? sTrue : sFalse),
	  (d->ascii.flagElevationAngle ? sTrue : sFalse), (d->ascii.flagDate ? sTrue : sFalse),
	  (d->ascii.flagTime ? sTrue : sFalse), (d->ascii.flagWavelength ? sTrue : sFalse));

  tmpStr = ws->simplifyPath(QString(d->ascii.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->ascii.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // logger
  fprintf(fp, "      <logger type=");
  switch (d->logger.spectralType) {
  case PRJCT_INSTR_IASB_TYPE_ALL:
    fprintf(fp, "\"all\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_ZENITHAL:
    fprintf(fp, "\"zenithal\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_OFFAXIS:
    fprintf(fp, "\"off-axis\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " azi=\"%s\"", (d->logger.flagAzimuthAngle ? sTrue : sFalse));

  tmpStr = ws->simplifyPath(QString(d->logger.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->logger.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // acton
  fprintf(fp, "      <acton type=");
  switch (d->acton.niluType) {
  case PRJCT_INSTR_NILU_FORMAT_OLD:
    fprintf(fp, "\"old\"");
    break;
  case PRJCT_INSTR_NILU_FORMAT_NEW:
    fprintf(fp, "\"new\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }

  tmpStr = ws->simplifyPath(QString(d->acton.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->acton.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // pdaegg
  fprintf(fp, "      <pdaegg type=");
  switch (d->pdaegg.spectralType) {
  case PRJCT_INSTR_IASB_TYPE_ALL:
    fprintf(fp, "\"all\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_ZENITHAL:
    fprintf(fp, "\"zenithal\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_OFFAXIS:
    fprintf(fp, "\"off-axis\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " azi=\"%s\"", (d->pdaegg.flagAzimuthAngle ? sTrue : sFalse));

  tmpStr = ws->simplifyPath(QString(d->pdaegg.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->pdaegg.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // pdaeggold
  fprintf(fp, "      <pdaeggold type=");
  switch (d->pdaeggold.spectralType) {
  case PRJCT_INSTR_IASB_TYPE_ALL:
    fprintf(fp, "\"all\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_ZENITHAL:
    fprintf(fp, "\"zenithal\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_OFFAXIS:
    fprintf(fp, "\"off-axis\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " azi=\"%s\"", (d->pdaeggold.flagAzimuthAngle ? sTrue : sFalse));

  tmpStr = ws->simplifyPath(QString(d->pdaeggold.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->pdaeggold.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // pdaeggold
  fprintf(fp, "      <pdaeggold type=");
  switch (d->pdaeggold.spectralType) {
  case PRJCT_INSTR_IASB_TYPE_ALL:
    fprintf(fp, "\"all\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_ZENITHAL:
    fprintf(fp, "\"zenithal\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_OFFAXIS:
    fprintf(fp, "\"off-axis\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " azi=\"%s\"", (d->pdaeggold.flagAzimuthAngle ? sTrue : sFalse));

  tmpStr = ws->simplifyPath(QString(d->pdaeggold.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->pdaeggold.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // pdaeggulb
  fprintf(fp, "      <pdaeggulb type=");
  switch (d->pdaeggulb.curveType) {
  case PRJCT_INSTR_ULB_TYPE_MANUAL:
    fprintf(fp, "\"manual\"");
    break;
  case PRJCT_INSTR_ULB_TYPE_HIGH:
    fprintf(fp, "\"high\"");
    break;
  case PRJCT_INSTR_ULB_TYPE_LOW:
    fprintf(fp, "\"low\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }

  tmpStr = ws->simplifyPath(QString(d->pdaeggulb.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->pdaeggulb.instrFunctionFile));
  fprintf(fp, " instr=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->pdaeggulb.interPixelVariabilityFile));
  fprintf(fp, " ipv=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->pdaeggulb.detectorNonLinearityFile));
  fprintf(fp, " dnl=\"%s\" />\n", tmpStr.toAscii().constData());

  // ccdohp96
  tmpStr = ws->simplifyPath(QString(d->ccdohp96.calibrationFile));
  fprintf(fp, "      <ccdohp96 calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->ccdohp96.instrFunctionFile));
  fprintf(fp, " instr=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->ccdohp96.interPixelVariabilityFile));
  fprintf(fp, " ipv=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->ccdohp96.detectorNonLinearityFile));
  fprintf(fp, " dnl=\"%s\" />\n", tmpStr.toAscii().constData());

  // ccdha94
  tmpStr = ws->simplifyPath(QString(d->ccdha94.calibrationFile));
  fprintf(fp, "      <ccdha94 calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->ccdha94.instrFunctionFile));
  fprintf(fp, " instr=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->ccdha94.interPixelVariabilityFile));
  fprintf(fp, " ipv=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->ccdha94.detectorNonLinearityFile));
  fprintf(fp, " dnl=\"%s\" />\n", tmpStr.toAscii().constData());

  // ccdulb
  fprintf(fp, "      <ccdulb grating=\"%d\" cen=\"%d\"", d->ccdulb.grating, d->ccdulb.centralWavelength);

  tmpStr = ws->simplifyPath(QString(d->ccdulb.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->ccdulb.offsetFile));
  fprintf(fp, " offset=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->ccdulb.interPixelVariabilityFile));
  fprintf(fp, " ipv=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->ccdulb.detectorNonLinearityFile));
  fprintf(fp, " dnl=\"%s\" />\n", tmpStr.toAscii().constData());

  // saoz vis
  fprintf(fp, "      <saozvis type=");
  switch (d->saozvis.spectralType) {
  case PRJCT_INSTR_SAOZ_TYPE_ZENITHAL:
    fprintf(fp, "\"zenithal\"");
    break;
  case PRJCT_INSTR_SAOZ_TYPE_POINTED:
    fprintf(fp, "\"pointed\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }

  tmpStr = ws->simplifyPath(QString(d->saozvis.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->saozvis.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // saoz uv
  fprintf(fp, "      <saozuv type=");
  switch (d->saozuv.spectralType) {
  case PRJCT_INSTR_SAOZ_TYPE_ZENITHAL:
    fprintf(fp, "\"zenithal\"");
    break;
  case PRJCT_INSTR_SAOZ_TYPE_POINTED:
    fprintf(fp, "\"pointed\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }

  tmpStr = ws->simplifyPath(QString(d->saozuv.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->saozuv.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // saozefm
  tmpStr = ws->simplifyPath(QString(d->saozefm.calibrationFile));
  fprintf(fp, "      <saozefm calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->saozefm.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // mfc
  fprintf(fp, "      <mfc size=\"%d\" first=\"%d\" revert=\"%s\" auto=\"%s\" omask=\"%d\" imask=\"%d\" dmask=\"%d\" smask=\"%d\"",
	  d->mfc.detectorSize, d->mfc.firstWavelength,
	  (d->mfc.revert ? sTrue: sFalse), (d->mfc.autoFileSelect ? sTrue : sFalse),
	  d->mfc.offsetMask, d->mfc.instrFctnMask, d->mfc.darkCurrentMask, d->mfc.spectraMask);

  tmpStr = ws->simplifyPath(QString(d->mfc.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->mfc.instrFunctionFile));
  fprintf(fp, " instr=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->mfc.darkCurrentFile));
  fprintf(fp, " dark=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->mfc.offsetFile));
  fprintf(fp, " offset=\"%s\" />\n", tmpStr.toAscii().constData());

  // mfcstd
  fprintf(fp, "      <mfcstd size=\"%d\" revert=\"%s\"", d->mfcstd.detectorSize, (d->mfcstd.revert ? sTrue : sFalse));

  tmpStr = ws->simplifyPath(QString(d->mfcstd.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->mfcstd.instrFunctionFile));
  fprintf(fp, " instr=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->mfcstd.darkCurrentFile));
  fprintf(fp, " dark=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->mfcstd.offsetFile));
  fprintf(fp, " offset=\"%s\" />\n", tmpStr.toAscii().constData());

  // rasas
  tmpStr = ws->simplifyPath(QString(d->rasas.calibrationFile));
  fprintf(fp, "      <rasas calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->rasas.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // pdasieasoe
  tmpStr = ws->simplifyPath(QString(d->pdasieasoe.calibrationFile));
  fprintf(fp, "      <pdasieasoe calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->pdasieasoe.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // pdasiosma
  fprintf(fp, "      <pdasiosma type=");
  switch (d->pdasiosma.spectralType) {
  case PRJCT_INSTR_IASB_TYPE_ALL:
    fprintf(fp, "\"all\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_ZENITHAL:
    fprintf(fp, "\"zenithal\"");
    break;
  case PRJCT_INSTR_IASB_TYPE_OFFAXIS:
    fprintf(fp, "\"off-axis\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " azi=\"%s\"", (d->pdasiosma.flagAzimuthAngle ? sTrue : sFalse));

  tmpStr = ws->simplifyPath(QString(d->pdasiosma.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->pdasiosma.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // ccdeev
  fprintf(fp, "      <ccdeev size=\"%d\"", d->ccdeev.detectorSize);

  tmpStr = ws->simplifyPath(QString(d->ccdeev.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->ccdeev.instrFunctionFile));
  fprintf(fp, " instr=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->ccdeev.straylightCorrectionFile));
  fprintf(fp, " stray=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->ccdeev.detectorNonLinearityFile));
  fprintf(fp, " dnl=\"%s\" />\n", tmpStr.toAscii().constData());

  // opus
  fprintf(fp, "      <opus size=\"%d\" time=\"%.2f\" trans=\"%s\"", d->opus.detectorSize,
	  d->opus.timeShift, (d->opus.flagTransmittance ? sTrue : sFalse));

  tmpStr = ws->simplifyPath(QString(d->opus.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->opus.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // gdpascii
  fprintf(fp, "      <gdpascii type=");
  switch (d->gdpascii.bandType) {
  case PRJCT_INSTR_GDP_BAND_1A:
    fprintf(fp, "\"1a\"");
    break;
  case PRJCT_INSTR_GDP_BAND_1B:
    fprintf(fp, "\"1b\"");
    break;
  case PRJCT_INSTR_GDP_BAND_2A:
    fprintf(fp, "\"2a\"");
    break;
  case PRJCT_INSTR_GDP_BAND_2B:
    fprintf(fp, "\"2B\"");
    break;
  case PRJCT_INSTR_GDP_BAND_3:
    fprintf(fp, "\"3\"");
    break;
  case PRJCT_INSTR_GDP_BAND_4:
    fprintf(fp, "\"4\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  tmpStr = ws->simplifyPath(QString(d->gdpascii.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->gdpascii.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // gdpbin
  fprintf(fp, "      <gdpbin type=");
  switch (d->gdpbin.bandType) {
  case PRJCT_INSTR_GDP_BAND_1A:
    fprintf(fp, "\"1a\"");
    break;
  case PRJCT_INSTR_GDP_BAND_1B:
    fprintf(fp, "\"1b\"");
    break;
  case PRJCT_INSTR_GDP_BAND_2A:
    fprintf(fp, "\"2a\"");
    break;
  case PRJCT_INSTR_GDP_BAND_2B:
    fprintf(fp, "\"2B\"");
    break;
  case PRJCT_INSTR_GDP_BAND_3:
    fprintf(fp, "\"3\"");
    break;
  case PRJCT_INSTR_GDP_BAND_4:
    fprintf(fp, "\"4\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }

  tmpStr = ws->simplifyPath(QString(d->gdpbin.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->gdpbin.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // sciahdf
  fprintf(fp, "      <sciahdf channel=");
  switch (d->sciahdf.channel) {
  case PRJCT_INSTR_SCIA_CHANNEL_1:
    fprintf(fp, "\"1\"");
    break;
  case PRJCT_INSTR_SCIA_CHANNEL_2:
    fprintf(fp, "\"2\"");
    break;
  case PRJCT_INSTR_SCIA_CHANNEL_3:
    fprintf(fp, "\"3\"");
    break;
  case PRJCT_INSTR_SCIA_CHANNEL_4:
    fprintf(fp, "\"4\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " sunref=\"%s\"", d->sciahdf.sunReference);
  for (i=2; i<=5; ++i) {
    if (d->sciahdf.clusters[i])
      fprintf(fp, " c%d=\"true\"", i);
  }
  for (i=8; i<=10; ++i) {
    if (d->sciahdf.clusters[i])
      fprintf(fp, " c%d=\"true\"", i);
  }
  for (i=13; i<=18; ++i) {
    if (d->sciahdf.clusters[i])
      fprintf(fp, " c%d=\"true\"", i);
  }
  for (i=22; i<=27; ++i) {
    if (d->sciahdf.clusters[i])
      fprintf(fp, " c%d=\"true\"", i);
  }

  tmpStr = ws->simplifyPath(QString(d->sciahdf.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->sciahdf.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // sciapds
  fprintf(fp, "      <sciapds channel=");
  switch (d->sciapds.channel) {
  case PRJCT_INSTR_SCIA_CHANNEL_1:
    fprintf(fp, "\"1\"");
    break;
  case PRJCT_INSTR_SCIA_CHANNEL_2:
    fprintf(fp, "\"2\"");
    break;
  case PRJCT_INSTR_SCIA_CHANNEL_3:
    fprintf(fp, "\"3\"");
    break;
  case PRJCT_INSTR_SCIA_CHANNEL_4:
    fprintf(fp, "\"4\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " sunref=\"%s\"", d->sciapds.sunReference);
  for (i=2; i<=5; ++i) {
    if (d->sciapds.clusters[i])
      fprintf(fp, " c%d=\"true\"", i);
  }
  for (i=8; i<=10; ++i) {
    if (d->sciapds.clusters[i])
      fprintf(fp, " c%d=\"true\"", i);
  }
  for (i=13; i<=18; ++i) {
    if (d->sciapds.clusters[i])
      fprintf(fp, " c%d=\"true\"", i);
  }
  for (i=22; i<=27; ++i) {
    if (d->sciapds.clusters[i])
      fprintf(fp, " c%d=\"true\"", i);
  }

  tmpStr = ws->simplifyPath(QString(d->sciapds.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->sciapds.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // uoft
  tmpStr = ws->simplifyPath(QString(d->uoft.calibrationFile));
  fprintf(fp, "      <uoft calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->uoft.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // noaa
  tmpStr = ws->simplifyPath(QString(d->noaa.calibrationFile));
  fprintf(fp, "      <noaa calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->noaa.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  // omi
  fprintf(fp, "      <omi type=");
  switch (d->omi.spectralType) {
  case PRJCT_INSTR_OMI_TYPE_UV1:
    fprintf(fp, "\"uv1\"");
    break;
  case PRJCT_INSTR_OMI_TYPE_UV2:
    fprintf(fp, "\"uv2\"");
    break;
  case PRJCT_INSTR_OMI_TYPE_VIS:
    fprintf(fp, "\"vis\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, " min=\"%.1f\" max=\"%.1f\" ave=\"%s\"",
	  d->omi.minimumWavelength, d->omi.maximumWavelength, (d->omi.flagAverage ? sTrue : sFalse));

  tmpStr = ws->simplifyPath(QString(d->omi.calibrationFile));
  fprintf(fp, " calib=\"%s\"", tmpStr.toAscii().constData());

  tmpStr = ws->simplifyPath(QString(d->omi.instrFunctionFile));
  fprintf(fp, " instr=\"%s\" />\n", tmpStr.toAscii().constData());

  fprintf(fp, "    </instrumental>\n");
}

void CQdoasConfigWriter::writePropertiesSlit(FILE *fp, const mediate_project_slit_t *d)
{
  QString tmpStr;
  CWorkSpace *ws = CWorkSpace::instance();

  tmpStr = ws->simplifyPath(QString(d->solarRefFile));
  fprintf(fp, "    <slit ref=\"%s\" fwhmcor=\"%s\">\n", tmpStr.toAscii().data(),
	  (d->applyFwhmCorrection ? sTrue : sFalse));

  writeSlitFunction(fp, &(d->function));
  
  fprintf(fp, "    </slit>\n");
}

void CQdoasConfigWriter::writeSlitFunction(FILE *fp, const mediate_slit_function_t *d)
{
  QString tmpStr;
  CWorkSpace *ws = CWorkSpace::instance();

  fprintf(fp, "      <slit_func type=");

  switch (d->type) {
  case SLIT_TYPE_FILE:
    fprintf(fp, "\"file\"");
    break;
  case SLIT_TYPE_GAUSS:
    fprintf(fp, "\"gaussian\"");
    break;
  case SLIT_TYPE_INVPOLY:
    fprintf(fp, "\"lorentz\"");
    break;
  case SLIT_TYPE_VOIGT:
    fprintf(fp, "\"voigt\"");
    break;
  case SLIT_TYPE_ERF:
    fprintf(fp, "\"error\"");
    break;
  case SLIT_TYPE_APOD:
    fprintf(fp, "\"boxcarapod\"");
    break;
  case SLIT_TYPE_APODNBS:
    fprintf(fp, "\"nbsapod\"");
    break;
  case SLIT_TYPE_GAUSS_FILE:
    fprintf(fp, "\"gaussianfile\"");
    break;
  case SLIT_TYPE_INVPOLY_FILE:
    fprintf(fp, "\"lorentzfile\"");
    break;
  case SLIT_TYPE_ERF_FILE:
    fprintf(fp, "\"errorfile\"");
    break;
  case SLIT_TYPE_GAUSS_T_FILE:
    fprintf(fp, "\"gaussiantempfile\"");
    break;
  case SLIT_TYPE_ERF_T_FILE:
    fprintf(fp, "\"errortempfile\"");
    break;
  default:
    fprintf(fp, "\"invalid\"");
  }
  fprintf(fp, ">\n");

  tmpStr = ws->simplifyPath(QString(d->file.filename));
  fprintf(fp, "        <file file=\"%s\" />\n", tmpStr.toAscii().data());

  fprintf(fp, "        <gaussian fwhm=\"%.3f\" />\n", d->gaussian.fwhm);
  fprintf(fp, "        <lorentz width=\"%.3f\" degree=\"%d\" />\n", d->lorentz.width, d->lorentz.degree);
  fprintf(fp, "        <voigt fwhmleft=\"%.3f\" fwhmright=\"%.3f\" glrleft=\"%.3f\" glrright=\"%.3f\" />\n",
	  d->voigt.fwhmL, d->voigt.fwhmR, d->voigt.glRatioL, d->voigt.glRatioR);
  fprintf(fp, "        <error fwhm=\"%.3f\" width=\"%.3f\" />\n", d->error.fwhm, d->error.width);
  fprintf(fp, "        <boxcarapod resolution=\"%.3f\" phase=\"%.3f\" />\n",
	  d->boxcarapod.resolution, d->boxcarapod.phase);
  fprintf(fp, "        <nbsapod resolution=\"%.3f\" phase=\"%.3f\" />\n",
	  d->nbsapod.resolution, d->nbsapod.phase);

  tmpStr = ws->simplifyPath(QString(d->gaussianfile.filename));
  fprintf(fp, "        <gaussianfile file=\"%s\" />\n", tmpStr.toAscii().data());

  tmpStr = ws->simplifyPath(QString(d->lorentzfile.filename));
  fprintf(fp, "        <lorentzfile file=\"%s\" degree=\"%d\" />\n", tmpStr.toAscii().data(), d->lorentzfile.degree);

  tmpStr = ws->simplifyPath(QString(d->errorfile.filename));
  fprintf(fp, "        <errorfile file=\"%s\" width=\"%.3f\" />\n", tmpStr.toAscii().data(), d->errorfile.width);

  tmpStr = ws->simplifyPath(QString(d->gaussiantempfile.filename));
  fprintf(fp, "        <gaussiantempfile file=\"%s\" />\n", tmpStr.toAscii().data());

  tmpStr = ws->simplifyPath(QString(d->errortempfile.filename));
  fprintf(fp, "        <errortempfile file=\"%s\" width=\"%.3f\" />\n", tmpStr.toAscii().data(), d->errortempfile.width);
  
  fprintf(fp, "      </slit_func>\n");
}


void CQdoasConfigWriter::writePropertiesOutput(FILE *fp, const mediate_project_output_t *d)
{
  QString tmpStr = CWorkSpace::instance()->simplifyPath(QString(d->path));

  fprintf(fp, "    <output path=\"%s\" anlys=\"%s\" calib=\"%s\" conf=\"%s\" bin=\"%s\" dirs=\"%s\" flux=\"%s\" cic=\"%s\" >\n",
	  tmpStr.toAscii().data(), (d->analysisFlag ? sTrue : sFalse),
	  (d->calibrationFlag ? sTrue : sFalse), (d->configurationFlag ? sTrue : sFalse),
	  (d->binaryFormatFlag ? sTrue : sFalse), (d->directoryFlag ? sTrue : sFalse),
	  d->flux, d->colourIndex);

  writeDataSelectList(fp, &(d->selection));

  fprintf(fp, "    </output>\n");
}

void CQdoasConfigWriter::writePropertiesNasaAmes(FILE *fp, const mediate_project_nasa_ames_t *d)
{
  QString tmpStr = CWorkSpace::instance()->simplifyPath(QString(d->path));

  fprintf(fp, "    <nasa_ames path=\"%s\" save=\"%s\" reject=\"%s\" instr=\"%s\" exp=\"%s\" no2=\"%s\" o3=\"%s\" bro=\"%s\" oclo=\"%s\" />\n",
	  tmpStr.toAscii().data(), (d->saveFlag ? sTrue : sFalse), (d->rejectTestFlag ? sTrue : sFalse),
	  d->instrument, d->experiment, d->anlysWinNO2, d->anlysWinO3, d->anlysWinBrO, d->anlysWinOClO);
}

void CQdoasConfigWriter::writeRawSpectraTree(FILE *fp, const QTreeWidgetItem *rawSpectraItem)
{
  fprintf(fp, "    <raw_spectra>\n"
	  "      <!-- Disable file, folder and directory items with the disable set equal to \"true\". -->\n"
	  "      <!--  The default is enabled.                                                       -->\n");

  QTreeWidgetItem *item;
  int nChildren = rawSpectraItem->childCount();
  int i = 0;

  while (i < nChildren) {
    item = rawSpectraItem->child(i);
    writeSpectraTreeNode(fp, item, 3);
    ++i;
  }

  fprintf(fp, "    </raw_spectra>\n");
}

void CQdoasConfigWriter::writeSpectraTreeNode(FILE *fp, const QTreeWidgetItem *item, int depth)
{
  int i;

  switch (item->type()) {
  case cSpectraFolderItemType:
    {
      const CSpectraFolderItem *folderItem = static_cast<const CSpectraFolderItem*>(item);
      int nChildren = item->childCount();

      for (i=0; i<depth; ++i) fprintf(fp, "  "); // indenting
      fprintf(fp, "<folder name=\"%s\"", item->text(0).toAscii().constData());
      if (folderItem->isEnabled())
	fprintf(fp, ">\n");
      else
	fprintf(fp, " disable=\"true\">\n");

      // now all children ...
      i = 0;
      while (i < nChildren) {
	writeSpectraTreeNode(fp, item->child(i), depth + 1);
	++i;
      }

      for (i=0; i<depth; ++i) fprintf(fp, "  "); // indenting
      fprintf(fp, "</folder>\n");
    }
    break;
  case cSpectraDirectoryItemType:
    {
      const CSpectraDirectoryItem *dirItem = static_cast<const CSpectraDirectoryItem*>(item);
      QString dirName = CWorkSpace::instance()->simplifyPath(dirItem->directoryName());

      for (i=0; i<depth; ++i) fprintf(fp, "  "); // indenting
      fprintf(fp, "<directory name=\"%s\" filters=\"%s\" recursive=\"%s\"",
	      dirName.toAscii().constData(),
	      dirItem->fileFilters().toAscii().constData(),
	      (dirItem->isRecursive() ? sTrue : sFalse));
      if (dirItem->isEnabled())
	fprintf(fp, " />\n");
      else
	fprintf(fp, " disable=\"true\" />\n");
    }
    break;
  case cSpectraFileItemType:
    {
      const CSpectraFileItem *fileItem = static_cast<const CSpectraFileItem*>(item);
      QString fileName = CWorkSpace::instance()->simplifyPath(fileItem->file().filePath());

      for (i=0; i<depth; ++i) fprintf(fp, "  "); // indenting
      fprintf(fp, "<file name=\"%s\"", fileName.toAscii().constData());
      if (fileItem->isEnabled())
	fprintf(fp, " />\n");
      else
	fprintf(fp, " disable=\"true\" />\n");
    }
    break;
  }
}

void CQdoasConfigWriter::writeAnalysisWindows(FILE *fp, const QString &projectName, const QTreeWidgetItem *item)
{
  const CAnalysisWindowItem *awItem;
  const mediate_analysis_window_t *properties;
  QString awName, tmpStr;

  CWorkSpace *ws = CWorkSpace::instance();

  int n = item->childCount();
  int i = 0;

  while (i < n) {
    awItem = dynamic_cast<const CAnalysisWindowItem*>(item->child(i));
    if (awItem != NULL) {
      awName = awItem->text(0);

      properties = CWorkSpace::instance()->findAnalysisWindow(projectName, awName);
      if (properties != NULL) {

	fprintf(fp, "    <analysis_window name=\"%s\" kurucz=", awName.toAscii().data());

	switch (properties->kuruczMode) {
	case ANLYS_KURUCZ_REF:
	  fprintf(fp, "\"ref\""); break;
	case ANLYS_KURUCZ_SPEC:
	  fprintf(fp, "\"spec\""); break;
	case ANLYS_KURUCZ_REF_AND_SPEC:
	  fprintf(fp, "\"ref+spec\""); break;
	default:
	  fprintf(fp, "\"none\"");
	}

	if (properties->refSpectrumSelection == ANLYS_REF_SELECTION_MODE_AUTOMATIC)
	  fprintf(fp, " refsel=\"auto\"");
	else
	  fprintf(fp, " refsel=\"file\"");

	fprintf(fp, " min=\"%d\" max=\"%d\" >\n", properties->fitMinWavelength, properties->fitMaxWavelength);

	fprintf(fp, "      <display spectrum=\"%s\" poly=\"%s\" fits=\"%s\" residual=\"%s\" predef=\"%s\" ratio=\"%s\" />\n",
		(properties->requireSpectrum ? sTrue : sFalse),
		(properties->requirePolynomial ? sTrue : sFalse),
		(properties->requireFit ? sTrue : sFalse),
		(properties->requireResidual ? sTrue : sFalse),
		(properties->requirePredefined ? sTrue : sFalse),
		(properties->requireRefRatio ? sTrue : sFalse));

	tmpStr = ws->simplifyPath(QString(properties->refOneFile));	
	fprintf(fp, "      <files refone=\"%s\"", tmpStr.toAscii().data());
	tmpStr = ws->simplifyPath(QString(properties->refTwoFile));		
	fprintf(fp, " reftwo=\"%s\"", tmpStr.toAscii().data());
	tmpStr = ws->simplifyPath(QString(properties->residualFile));	
	fprintf(fp, " residual=\"%s\" szacenter=\"%.3f\" szadelta=\"%.3f\" />\n", tmpStr.toAscii().data(),
		properties->refSzaCenter , properties->refSzaDelta);

	// cross sections ....
	writeCrossSectionList(fp, &(properties->crossSectionList));

	// linear
	writeLinear(fp, &(properties->linear));

	// nonlinear
	writeNonLinear(fp, &(properties->nonlinear));

	// shift and stretch
	writeShiftStretchList(fp, &(properties->shiftStretchList));

	// gaps...
	writeGapList(fp, &(properties->gapList));

	// output...
	writeOutputList(fp, &(properties->outputList));

	fprintf(fp, "    </analysis_window>\n");
      }
    }

    ++i;
  }
}

void CQdoasConfigWriter::writePolyType(FILE *fp, const char *attr, int type)
{
  switch (type) {
  case ANLYS_POLY_TYPE_0: fprintf(fp, " %s=\"0\"", attr); break;
  case ANLYS_POLY_TYPE_1: fprintf(fp, " %s=\"1\"", attr); break;
  case ANLYS_POLY_TYPE_2: fprintf(fp, " %s=\"2\"", attr); break;
  case ANLYS_POLY_TYPE_3: fprintf(fp, " %s=\"3\"", attr); break;
  case ANLYS_POLY_TYPE_4: fprintf(fp, " %s=\"4\"", attr); break;
  case ANLYS_POLY_TYPE_5: fprintf(fp, " %s=\"5\"", attr); break;
  default: fprintf(fp, " %s=\"none\"", attr);
  }
}

void CQdoasConfigWriter::writeCrossSectionList(FILE *fp, const cross_section_list_t *data)
{
  QString tmpStr;
  CWorkSpace *ws = CWorkSpace::instance();
  const struct anlyswin_cross_section *d = &(data->crossSection[0]);
  int j = 0;

  fprintf(fp, "      <cross_sections>\n");

  while (j < data->nCrossSection) {
    fprintf(fp, "        <cross_section sym=\"%s\" ortho=\"%s\" cstype=",
	    d->symbol,
	    d->orthogonal);
    
    switch (d->crossType) {
    case ANLYS_CROSS_ACTION_INTERPOLATE:
      fprintf(fp, "\"interp\""); break;
    case ANLYS_CROSS_ACTION_CONVOLUTE:
      fprintf(fp, "\"std\""); break;
    case ANLYS_CROSS_ACTION_CONVOLUTE_I0:
      fprintf(fp, "\"io\""); break;
    case ANLYS_CROSS_ACTION_CONVOLUTE_RING:
      fprintf(fp, "\"ring\""); break;
    default:
      fprintf(fp, "\"none\"");
    }
    fprintf(fp, " amftype=");
    switch (d->amfType) {
    case ANLYS_AMF_TYPE_SZA:
      fprintf(fp, "\"sza\""); break;
    case ANLYS_AMF_TYPE_CLIMATOLOGY:
      fprintf(fp, "\"climate\""); break;
    case ANLYS_AMF_TYPE_WAVELENGTH1:
      fprintf(fp, "\"wave1\""); break;
    case ANLYS_AMF_TYPE_WAVELENGTH2:
      fprintf(fp, "\"wave2\""); break;
    case ANLYS_AMF_TYPE_WAVELENGTH3:
      fprintf(fp, "\"wave3\""); break;
    default:
      fprintf(fp, "\"none\"");
    }
    
    fprintf(fp, " fit=\"%s\" filter=\"%s\" ccfit=\"%s\" icc=\"%.3f\" dcc=\"%.3f\" ccio=\"%.3f\"",
	    (d->requireFit ? sTrue : sFalse),
	    (d->requireFilter ? sTrue : sFalse),
	    (d->requireCcFit ? sTrue : sFalse),
	    d->initialCc, d->deltaCc, d->ccIo);
    tmpStr = ws->simplifyPath(QString(d->crossSectionFile));
    fprintf(fp, " csfile=\"%s\"", tmpStr.toAscii().data());
    tmpStr = ws->simplifyPath(QString(d->amfFile));
    fprintf(fp, " amffile=\"%s\" />\n", tmpStr.toAscii().data());
    
    ++d;
    ++j;
  }
  fprintf(fp, "      </cross_sections>\n");
}

void CQdoasConfigWriter::writeLinear(FILE *fp, const struct anlyswin_linear *d)
{
  fprintf(fp, "      <linear");
  writePolyType(fp, "xpoly", d->xPolyOrder);
  writePolyType(fp, "xbase", d->xBaseOrder);
  fprintf(fp, " xfit=\"%s\" xerr=\"%s\"",
	  (d->xFlagFitStore ? sTrue : sFalse),
	  (d->xFlagErrStore ? sTrue : sFalse));
  writePolyType(fp, "xinvpoly", d->xinvPolyOrder);
  writePolyType(fp, "xinvbase", d->xinvBaseOrder);
  fprintf(fp, " xinvfit=\"%s\" xinverr=\"%s\"",
	  (d->xinvFlagFitStore ? sTrue : sFalse),
	  (d->xinvFlagErrStore ? sTrue : sFalse));
  writePolyType(fp, "offpoly", d->offsetPolyOrder);
  writePolyType(fp, "offbase", d->offsetBaseOrder);
  fprintf(fp, " offfit=\"%s\" offerr=\"%s\"",
	  (d->offsetFlagFitStore ? sTrue : sFalse),
	  (d->offsetFlagErrStore ? sTrue : sFalse));
  fprintf(fp, " />\n");
}

void CQdoasConfigWriter::writeNonLinear(FILE *fp, const struct anlyswin_nonlinear *d)
{
  fprintf(fp, "      <nonlinear solfit=\"%s\" solinit=\"%.3f\" soldelt=\"%.3f\" solfstr=\"%s\" solestr=\"%s\"",
	  (d->solFlagFit ? sTrue : sFalse), d->solInitial, d->solDelta,
	  (d->solFlagErrStore ? sTrue : sFalse), (d->solFlagErrStore ? sTrue : sFalse));

  fprintf(fp, " o0fit=\"%s\" o0init=\"%.3f\" o0delt=\"%.3f\" o0str=\"%s\" o0estr=\"%s\"",
	  (d->off0FlagFit ? sTrue : sFalse), d->off0Initial, d->off0Delta,
	  (d->off0FlagErrStore ? sTrue : sFalse), (d->off0FlagErrStore ? sTrue : sFalse));

  fprintf(fp, " o1fit=\"%s\" o1init=\"%.3f\" o1delt=\"%.3f\" o1str=\"%s\" o1estr=\"%s\"",
	  (d->off1FlagFit ? sTrue : sFalse), d->off1Initial, d->off1Delta,
	  (d->off1FlagErrStore ? sTrue : sFalse), (d->off1FlagErrStore ? sTrue : sFalse));

  fprintf(fp, " o2fit=\"%s\" o2init=\"%.3f\" o2delt=\"%.3f\" o2str=\"%s\" o2estr=\"%s\"",
	  (d->off2FlagFit ? sTrue : sFalse), d->off2Initial, d->off2Delta,
	  (d->off2FlagErrStore ? sTrue : sFalse), (d->off2FlagErrStore ? sTrue : sFalse));

  fprintf(fp, " comfit=\"%s\" cominit=\"%.3f\" comdelt=\"%.3f\" comstr=\"%s\" comestr=\"%s\"",
	  (d->comFlagFit ? sTrue : sFalse), d->comInitial, d->comDelta,
	  (d->comFlagErrStore ? sTrue : sFalse), (d->comFlagErrStore ? sTrue : sFalse));

  fprintf(fp, " u1fit=\"%s\" u1init=\"%.3f\" u1delt=\"%.3f\" u1str=\"%s\" u1estr=\"%s\"",
	  (d->usamp1FlagFit ? sTrue : sFalse), d->usamp1Initial, d->usamp1Delta,
	  (d->usamp1FlagErrStore ? sTrue : sFalse), (d->usamp1FlagErrStore ? sTrue : sFalse));

  fprintf(fp, " u2fit=\"%s\" u2init=\"%.3f\" u2delt=\"%.3f\" u2str=\"%s\" u2estr=\"%s\"",
	  (d->usamp2FlagFit ? sTrue : sFalse), d->usamp2Initial, d->usamp2Delta,
	  (d->usamp2FlagErrStore ? sTrue : sFalse), (d->usamp2FlagErrStore ? sTrue : sFalse));

  fprintf(fp, " ramfit=\"%s\" raminit=\"%.3f\" ramdelt=\"%.3f\" ramstr=\"%s\" ramestr=\"%s\" />\n",
	  (d->ramanFlagFit ? sTrue : sFalse), d->ramanInitial, d->ramanDelta,
	  (d->ramanFlagErrStore ? sTrue : sFalse), (d->ramanFlagErrStore ? sTrue : sFalse));

}

void CQdoasConfigWriter::writeShiftStretchList(FILE *fp, const shift_stretch_list_t *data)
{
  int k, j;
  const struct anlyswin_shift_stretch *d = &(data->shiftStretch[0]);
 
  fprintf(fp, "      <shift_stretches>\n");

  j = 0;
  while (j < data->nShiftStretch) {
    fprintf(fp, "        <shift_stretch shfit=\"%s\" stfit=",
	    (d->shFit ? sTrue : sFalse));
    switch (d->stFit) {
    case ANLYS_STRETCH_TYPE_FIRST_ORDER: fprintf(fp, "\"1st\""); break;
    case ANLYS_STRETCH_TYPE_SECOND_ORDER: fprintf(fp, "\"2nd\""); break;
    default: fprintf(fp, "\"none\"");
    }
    fprintf(fp, " scfit=");
    switch (d->scFit) {
    case ANLYS_STRETCH_TYPE_FIRST_ORDER: fprintf(fp, "\"1st\""); break;
    case ANLYS_STRETCH_TYPE_SECOND_ORDER: fprintf(fp, "\"2nd\""); break;
    default: fprintf(fp, "\"none\"");
    }
    fprintf(fp, " shstr=\"%s\" ststr=\"%s\" scstr=\"%s\" errstr=\"%s\"",
	    (d->shStore ? sTrue : sFalse),
	    (d->stStore ? sTrue : sFalse),
	    (d->scStore ? sTrue : sFalse),
	    (d->errStore ? sTrue : sFalse));
    fprintf(fp, " shini=\"%.3f\" stini=\"%.3f\" stini2=\"%.3f\" scini=\"%.3f\" scini2=\"%.3f\"",
	    d->shInit,
	    d->stInit, d->stInit2,
	    d->scInit, d->scInit2);
    fprintf(fp, " shdel=\"%.4f\" stdel=\"%.4f\" stdel2=\"%.4f\" scdel=\"%.4f\" scdel2=\"%.4f\"",
	    d->shDelta,
	    d->stDelta, d->stDelta2,
	    d->scDelta, d->scDelta2);
    fprintf(fp, " shmin=\"%.3f\" shmax=\"%.3f\" >\n",
	    d->shMin, d->shMax);
    
    k = 0;
    while (k < d->nSymbol) {
      fprintf(fp, "          <symbol name=\"%s\" />\n", d->symbol[k]);
      ++k;
    }
  
    fprintf(fp, "        </shift_stretch>\n");

    ++d;
    ++j;
  }
  
  fprintf(fp, "      </shift_stretches>\n");
}

void CQdoasConfigWriter::writeGapList(FILE *fp, const gap_list_t *d)
{
  int j = 0;

  fprintf(fp, "      <gaps>\n");

  while (j < d->nGap) {
    fprintf(fp, "        <gap min=\"%.2f\" max=\"%.2f\" />\n",
	    d->gap[j].minimum, d->gap[j].maximum);
    ++j;
  }
  fprintf(fp, "      </gaps>\n");
}

void CQdoasConfigWriter::writeOutputList(FILE *fp, const output_list_t *d)
{
  int j = 0;

  fprintf(fp, "      <outputs>\n");
  
  while (j < d->nOutput) {
    fprintf(fp, "        <output sym=\"%s\" amf=\"%s\" scol=\"%s\" serr=\"%s\" sfact=\"%.3f\"",
	    d->output[j].symbol,
	    (d->output[j].amf ? sTrue : sFalse),
	    (d->output[j].slantCol ? sTrue : sFalse),
	    (d->output[j].slantErr ? sTrue : sFalse),
	    d->output[j].slantFactor);
    fprintf(fp, " vcol=\"%s\" verr=\"%s\" vfact=\"%.3f\" />\n",
	    (d->output[j].vertCol ? sTrue : sFalse),
	    (d->output[j].vertErr ? sTrue : sFalse),
	    d->output[j].vertFactor);
    
    ++j;
  }
  fprintf(fp, "      </outputs>\n");
}

void CQdoasConfigWriter::writeSfps(FILE *fp, const struct calibration_sfp *d)
{
  fprintf(fp, "      <sfps>\n");

  for (int i=0; i<4; ++i) {
    fprintf(fp, "        <sfp index=\"%d\" fit=\"%s\" init=\"%.3f\" delta=\"%.3f\" fstr=\"%s\" estr=\"%s\" />\n",
	    i+1, (d->fitFlag ? sTrue : sFalse),
	    d->initialValue, d->deltaValue,
	    (d->fitStore ? sTrue : sFalse), (d->errStore ? sTrue : sFalse));
    ++d;
  }

  fprintf(fp, "      </sfps>\n");
}


void CQdoasConfigWriter::writeDataSelectList(FILE *fp, const data_select_list_t *d)
{
  int i = 0;
  while (i < d->nSelected) {

    fprintf(fp, "      <field name=\"");
    switch (d->selected[i]) {
    case PRJCT_RESULTS_ASCII_SPECNO:           fprintf(fp, "specno"); break;        
    case PRJCT_RESULTS_ASCII_NAME:             fprintf(fp, "name"); break;        
    case PRJCT_RESULTS_ASCII_DATE_TIME:        fprintf(fp, "date_time"); break;
    case PRJCT_RESULTS_ASCII_DATE:             fprintf(fp, "date"); break;
    case PRJCT_RESULTS_ASCII_TIME:             fprintf(fp, "time"); break;
    case PRJCT_RESULTS_ASCII_YEAR:             fprintf(fp, "year"); break;
    case PRJCT_RESULTS_ASCII_JULIAN:           fprintf(fp, "julian"); break;
    case PRJCT_RESULTS_ASCII_JDFRAC:           fprintf(fp, "jdfrac"); break;
    case PRJCT_RESULTS_ASCII_TIFRAC:           fprintf(fp, "tifrac"); break;
    case PRJCT_RESULTS_ASCII_SCANS:            fprintf(fp, "scans"); break;
    case PRJCT_RESULTS_ASCII_TINT:             fprintf(fp, "tint"); break;
    case PRJCT_RESULTS_ASCII_SZA:              fprintf(fp, "sza"); break;
    case PRJCT_RESULTS_ASCII_CHI:              fprintf(fp, "chi"); break;
    case PRJCT_RESULTS_ASCII_RMS:              fprintf(fp, "rms"); break;
    case PRJCT_RESULTS_ASCII_AZIM:             fprintf(fp, "azim"); break;
    case PRJCT_RESULTS_ASCII_TDET:             fprintf(fp, "tdet"); break;
    case PRJCT_RESULTS_ASCII_SKY:              fprintf(fp, "sky"); break;
    case PRJCT_RESULTS_ASCII_BESTSHIFT:        fprintf(fp, "bestshift"); break;
    case PRJCT_RESULTS_ASCII_REFZM:            fprintf(fp, "refzm"); break;
    case PRJCT_RESULTS_ASCII_REFSHIFT:         fprintf(fp, "refshift"); break;
    case PRJCT_RESULTS_ASCII_PIXEL:            fprintf(fp, "pixel"); break;
    case PRJCT_RESULTS_ASCII_PIXEL_TYPE:       fprintf(fp, "pixel_type"); break;
    case PRJCT_RESULTS_ASCII_ORBIT:            fprintf(fp, "orbit"); break;
    case PRJCT_RESULTS_ASCII_LONGIT:           fprintf(fp, "longit"); break;
    case PRJCT_RESULTS_ASCII_LATIT:            fprintf(fp, "latit"); break;
    case PRJCT_RESULTS_ASCII_ALTIT:            fprintf(fp, "altit"); break;
    case PRJCT_RESULTS_ASCII_COVAR:            fprintf(fp, "covar"); break;
    case PRJCT_RESULTS_ASCII_CORR:             fprintf(fp, "corr"); break;
    case PRJCT_RESULTS_ASCII_CLOUD:            fprintf(fp, "cloud"); break;
    case PRJCT_RESULTS_ASCII_COEFF:            fprintf(fp, "coeff"); break;
    case PRJCT_RESULTS_ASCII_O3:               fprintf(fp, "o3"); break;
    case PRJCT_RESULTS_ASCII_NO2:              fprintf(fp, "no2"); break;
    case PRJCT_RESULTS_ASCII_CLOUDTOPP:        fprintf(fp, "cloudtopp"); break;
    case PRJCT_RESULTS_ASCII_LOS_ZA:           fprintf(fp, "los_za"); break;
    case PRJCT_RESULTS_ASCII_LOS_AZIMUTH:      fprintf(fp, "los_azimuth"); break;
    case PRJCT_RESULTS_ASCII_SAT_HEIGHT:       fprintf(fp, "sat_height"); break;
    case PRJCT_RESULTS_ASCII_EARTH_RADIUS:     fprintf(fp, "earth_radius"); break;
    case PRJCT_RESULTS_ASCII_VIEW_ELEVATION:   fprintf(fp, "view_elevation"); break;
    case PRJCT_RESULTS_ASCII_VIEW_AZIMUTH:     fprintf(fp, "view_azimuth"); break;
    case PRJCT_RESULTS_ASCII_SCIA_QUALITY:     fprintf(fp, "scia_quality"); break;
    case PRJCT_RESULTS_ASCII_SCIA_STATE_INDEX: fprintf(fp, "scia_state_index"); break;
    case PRJCT_RESULTS_ASCII_SCIA_STATE_ID:    fprintf(fp, "scia_state_id"); break;
    case PRJCT_RESULTS_ASCII_MFC_STARTTIME:    fprintf(fp, "mfc_starttime"); break;
    case PRJCT_RESULTS_ASCII_MFC_ENDTIME:      fprintf(fp, "mfc_endtime"); break;
    default: fprintf(fp, "Invalid");
    }
    fprintf(fp, "\" />\n");
    ++i;
  }
}

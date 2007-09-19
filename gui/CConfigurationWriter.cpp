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
#include "CConfigurationWriter.h"

#include "constants.h"

const char sTrue[] = "true";
const char sFalse[] = "false";

CConfigurationWriter::CConfigurationWriter(const CWProjectTree *projectTree) :
  m_projectTree(projectTree)
{
}

CConfigurationWriter::~CConfigurationWriter()
{
}

QString CConfigurationWriter::write(const QString &fileName)
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
  }

  writeProjects(fp);

  fprintf(fp, "</qdoas>\n");

  if (fclose(fp)) {
    QTextStream stream(&msg);

    stream << "Error writing to the project file '" << fileName << "'";
  }

  return msg;
}

void CConfigurationWriter::writeProjects(FILE *fp)
{
  const QTreeWidgetItem *item;
  const CProjectItem *projItem;
  const mediate_project_t *properties;

  int n = m_projectTree->topLevelItemCount();
  int i = 0;

  while (i < n) {
    projItem = dynamic_cast<const CProjectItem*>(m_projectTree->topLevelItem(i));
    if (projItem != NULL && projItem->childCount() == 2) {
      // Should always be a project item ... with two children
      properties = CWorkSpace::instance()->findProject(projItem->text(0));
      if (properties != NULL) {
	// write the project data
	fprintf(fp, "  <project name=\"%s\">\n", projItem->text(0).toAscii().constData());
	writeProperties(fp, properties);

	// analysis windows ...

	item = projItem->child(0); // Raw Spectra Node
	writeRawSpectraTree(fp, item);

	fprintf(fp, "  </project>\n");
      }
    }
    ++i;
  }
}

void CConfigurationWriter::writeProperties(FILE *fp, const mediate_project_t *d)
{
  writePropertiesSpectra(fp, &(d->spectra));
  writePropertiesAnalysis(fp, &(d->analysis));
  writePropertiesFilter(fp, "low", &(d->lowpass));
  writePropertiesFilter(fp, "high", &(d->highpass));
  writePropertiesCalibration(fp, &(d->calibration));
  writePropertiesUndersampling(fp, &(d->undersampling));
  writePropertiesInstrumental(fp, &(d->instrumental));
  writePropertiesSlit(fp, &(d->slit));
}

void CConfigurationWriter::writePropertiesSpectra(FILE *fp, const mediate_project_spectra_t *d)
{
  fprintf(fp, "    <spectra>\n");
  fprintf(fp, "      <display spectra=\"%s\" data=\"%s\" fits=\"%s\" />\n",
	  (d->requireSpectra ? sTrue : sFalse), (d->requireData ? sTrue : sFalse), (d->requireFits ? sTrue : sFalse));
  fprintf(fp, "      <sza min=\"%.3f\" max=\"%.3f\" delta=\"%.3f\" />\n", d->szaMinimum, d->szaMaximum, d->szaDelta);
  fprintf(fp, "      <record min=\"%d\" max=\"%d\" />\n", d->recordNumberMinimum, d->recordNumberMaximum);
  fprintf(fp, "      <files dark=\"%s\" names=\"%s\" />\n", (d->useDarkFile ? sTrue : sFalse),
	  (d->useNameFile ? sTrue : sFalse));
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
  fprintf(fp, "      </geolocation>\n    </spectra>\n");

}

void CConfigurationWriter::writePropertiesAnalysis(FILE *fp, const mediate_project_analysis_t *d)
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

void CConfigurationWriter::writePropertiesFilter(FILE *fp, const char *passband, const mediate_project_filtering_t *d)
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

void CConfigurationWriter::writePropertiesCalibration(FILE *fp, const mediate_project_calibration_t *d)
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
  fprintf(fp, "    </calibration>\n");
}

void CConfigurationWriter::writePropertiesUndersampling(FILE *fp, const mediate_project_undersampling_t *d)
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

void CConfigurationWriter::writePropertiesInstrumental(FILE *fp, const mediate_project_instrumental_t *d)
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

void CConfigurationWriter::writePropertiesSlit(FILE *fp, const mediate_project_slit_t *d)
{
  fprintf(fp, "    <slit ref=\"%s\" fwhmcor=\"%s\" type=", d->solarRefFile, (d->applyFwhmCorrection ? sTrue : sFalse));
  switch (d->slitType) {
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

  fprintf(fp, "      <file file=\"%s\" />\n", d->file.filename);
  fprintf(fp, "      <gaussian fwhm=\"%.3f\" />\n", d->gaussian.fwhm);
  fprintf(fp, "      <lorentz width=\"%.3f\" degree=\"%d\" />\n", d->lorentz.width, d->lorentz.degree);
  fprintf(fp, "      <voigt fwhmleft=\"%.3f\" fwhmright=\"%.3f\" glrleft=\"%.3f\" glrright=\"%.3f\" />\n",
	  d->voigt.fwhmL, d->voigt.fwhmR, d->voigt.glRatioL, d->voigt.glRatioR);
  fprintf(fp, "      <error fwhm=\"%.3f\" width=\"%.3f\" />\n", d->error.fwhm, d->error.width);
  fprintf(fp, "      <boxcarapod resolution=\"%.3f\" phase=\"%.3f\" />\n",
	  d->boxcarapod.resolution, d->boxcarapod.phase);
  fprintf(fp, "      <nbsapod resolution=\"%.3f\" phase=\"%.3f\" />\n",
	  d->nbsapod.resolution, d->nbsapod.phase);
  fprintf(fp, "      <gaussianfile file=\"%s\" />\n", d->gaussianfile.filename);
  fprintf(fp, "      <lorentzfile file=\"%s\" degree=\"%d\" />\n", d->lorentzfile.filename, d->lorentzfile.degree);
  fprintf(fp, "      <errorfile file=\"%s\" width=\"%.3f\" />\n", d->errorfile.filename, d->errorfile.width);
  fprintf(fp, "      <gaussiantempfile file=\"%s\" />\n", d->gaussiantempfile.filename);
  fprintf(fp, "      <errortempfile file=\"%s\" width=\"%.3f\" />\n", d->errortempfile.filename, d->errortempfile.width);
  
  fprintf(fp, "    </slit>\n");
}

void CConfigurationWriter::writeRawSpectraTree(FILE *fp, const QTreeWidgetItem *rawSpectraItem)
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

void CConfigurationWriter::writeSpectraTreeNode(FILE *fp, const QTreeWidgetItem *item, int depth)
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



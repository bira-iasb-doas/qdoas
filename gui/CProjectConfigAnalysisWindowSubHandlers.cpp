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


#include "CProjectConfigAnalysisWindowSubHandlers.h"

#include "constants.h"

CAnalysisWindowSubHandler::CAnalysisWindowSubHandler(CQdoasProjectConfigHandler *master,
						     CAnalysisWindowConfigItem *item) :
  CConfigSubHandler(master),
  m_item(item)
{
}

CAnalysisWindowSubHandler::~CAnalysisWindowSubHandler()
{

}

bool CAnalysisWindowSubHandler::start(const QXmlAttributes &atts)
{
  mediate_analysis_window_t *d = m_item->properties();

  m_item->setName(atts.value("name"));

  QString str = atts.value("kurucz");
  if (str == "ref")
    d->kuruczMode = ANLYS_KURUCZ_REF;
  else if (str == "spec")
    d->kuruczMode = ANLYS_KURUCZ_SPEC;
  else if (str == "ref+spec")
    d->kuruczMode = ANLYS_KURUCZ_REF_AND_SPEC;
  else
    d->kuruczMode = ANLYS_KURUCZ_NONE;

  d->refSpectrumSelection = (atts.value("refsel") == "auto") ? ANLYS_REF_SELECTION_MODE_AUTOMATIC :  ANLYS_REF_SELECTION_MODE_FILE;

  d->fitMinWavelength = atts.value("min").toInt();
  d->fitMaxWavelength = atts.value("max").toInt();

  // MUST have a valid name
  return !m_item->name().isEmpty();
}

bool CAnalysisWindowSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  mediate_analysis_window_t *d = m_item->properties();

  if (element == "display") {
    d->requireSpectrum = (atts.value("spectrum") == "true") ? 1 : 0;
    d->requirePolynomial = (atts.value("poly") == "true") ? 1 : 0;
    d->requireFit = (atts.value("fits") == "true") ? 1 : 0;
    d->requireResidual = (atts.value("residual") == "true") ? 1 : 0;
    d->requirePredefined = (atts.value("predef") == "true") ? 1 : 0;
    d->requireRefRatio = (atts.value("ratio") == "true") ? 1 : 0;

  }
  else if (element == "files") {
    QString str;

    str = atts.value("refone");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->refOneFile))
	strcpy(d->refOneFile, str.toAscii().data());
      else
	return postErrorMessage("Reference 1 Filename too long");
    }

    str = atts.value("reftwo");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->refTwoFile))
	strcpy(d->refTwoFile, str.toAscii().data());
      else
	return postErrorMessage("Reference 2 Filename too long");
    }

    str = atts.value("residual");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->residualFile))
	strcpy(d->residualFile, str.toAscii().data());
      else
	return postErrorMessage("Residual Filename too long");
    }

    d->refSzaCenter = atts.value("szacenter").toDouble();
    d->refSzaDelta = atts.value("szadelta").toDouble();

  }
  else if (element == "linear") {

    d->linear.xPolyOrder = mapToPolyType(atts.value("xpoly"));
    d->linear.xBaseOrder = mapToPolyType(atts.value("xbase"));
    d->linear.xFlagFitStore = (atts.value("xfit") == "true") ? 1 : 0;
    d->linear.xFlagErrStore = (atts.value("xerr") == "true") ? 1 : 0;

    d->linear.xinvPolyOrder = mapToPolyType(atts.value("xinvpoly"));
    d->linear.xinvBaseOrder = mapToPolyType(atts.value("xinvbase"));
    d->linear.xinvFlagFitStore = (atts.value("xinvfit") == "true") ? 1 : 0;
    d->linear.xinvFlagErrStore = (atts.value("xinverr") == "true") ? 1 : 0;
    
    d->linear.offsetPolyOrder = mapToPolyType(atts.value("offpoly"));
    d->linear.offsetBaseOrder = mapToPolyType(atts.value("offbase"));
    d->linear.offsetFlagFitStore = (atts.value("offfit") == "true") ? 1 : 0;
    d->linear.offsetFlagErrStore = (atts.value("offerr") == "true") ? 1 : 0;
    
  }
  else if (element == "cross_section") {
    return m_master->installSubHandler(new CAnalysisWindowCrossSectionSubHandler(m_master, d), atts);
  }
  else if (element == "shift_stretch") {
    return m_master->installSubHandler(new CAnalysisWindowShiftStretchSubHandler(m_master, d), atts);
  }

  return true;
}

int CAnalysisWindowSubHandler::mapToPolyType(const QString &str)
{
  int result = ANLYS_POLY_TYPE_NONE;

  if (str == "0") result = ANLYS_POLY_TYPE_0;
  else if (str == "1") result = ANLYS_POLY_TYPE_1;
  else if (str == "2") result = ANLYS_POLY_TYPE_2;
  else if (str == "3") result = ANLYS_POLY_TYPE_3;
  else if (str == "4") result = ANLYS_POLY_TYPE_4;
  else if (str == "5") result = ANLYS_POLY_TYPE_5;

  return result;
}

CAnalysisWindowCrossSectionSubHandler::CAnalysisWindowCrossSectionSubHandler(CQdoasProjectConfigHandler *master,
									     mediate_analysis_window_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

CAnalysisWindowCrossSectionSubHandler::~CAnalysisWindowCrossSectionSubHandler()
{
}

bool CAnalysisWindowCrossSectionSubHandler::start(const QXmlAttributes &atts)
{
  if (m_d->nCrossSection < MAX_AW_CROSS_SECTION) {

    QString str;
    struct anlyswin_cross_section *d = &(m_d->crossSection[m_d->nCrossSection]);
    
    str = atts.value("sym");
    if (!str.isEmpty() && str.length() < (int)sizeof(d->symbol))
      strcpy(d->symbol, str.toAscii().data());
    else
      return postErrorMessage("missing symbol (or name too long)");

    str = atts.value("ortho");
    if (!str.isEmpty() && str.length() < (int)sizeof(d->orthogonal))
      strcpy(d->orthogonal, str.toAscii().data());
    else
      return postErrorMessage("missing ortho (or name too long)");

    str = atts.value("cstype");
    if (str == "interp") d->crossType = ANLYS_CROSS_ACTION_INTERPOLATE;
    else if (str == "std") d->crossType = ANLYS_CROSS_ACTION_CONVOLUTE;
    else if (str == "io") d->crossType = ANLYS_CROSS_ACTION_CONVOLUTE_I0;
    else if (str == "ring") d->crossType = ANLYS_CROSS_ACTION_CONVOLUTE_RING;
    else d->crossType = ANLYS_CROSS_ACTION_NOTHING;

    str = atts.value("amftype");
    if (str == "sza") d->amfType = ANLYS_AMF_TYPE_SZA;
    else if (str == "climate") d->amfType = ANLYS_AMF_TYPE_CLIMATOLOGY;
    else if (str == "wave1") d->amfType = ANLYS_AMF_TYPE_WAVELENGTH1;
    else if (str == "wave2") d->amfType = ANLYS_AMF_TYPE_WAVELENGTH2;
    else if (str == "wave3") d->amfType = ANLYS_AMF_TYPE_WAVELENGTH3;
    else d->amfType = ANLYS_AMF_TYPE_NONE;

    d->requireFit = (atts.value("fit") == "true") ? 1 : 0;
    d->requireFilter = (atts.value("filter") == "true") ? 1 : 0;
    d->requireCcFit = (atts.value("ccfit") == "true") ? 1 : 0;
    d->initialCc = atts.value("icc").toDouble();
    d->deltaCc = atts.value("dcc").toDouble();
    d->requireCcIo = (atts.value("ccio") == "true") ? 1 : 0;

    str = atts.value("csfile");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->crossSectionFile))
	strcpy(d->crossSectionFile, str.toAscii().data());
      else
	return postErrorMessage("Cross Section filename too long");
    }
    else
      return postErrorMessage("Missing cross section file");

    str = atts.value("amffile");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->amfFile))
	strcpy(d->amfFile, str.toAscii().data());
      else
	return postErrorMessage("AMF filename too long");
    }

    // All OK
    ++(m_d->nCrossSection);
    
    return true;
  }

  return postErrorMessage("Too many cross sections in analysis window");
}


CAnalysisWindowShiftStretchSubHandler::CAnalysisWindowShiftStretchSubHandler(CQdoasProjectConfigHandler *master,
									     mediate_analysis_window_t *d) :
  CConfigSubHandler(master),
  m_d(d)
{
}

CAnalysisWindowShiftStretchSubHandler::~CAnalysisWindowShiftStretchSubHandler()
{
}

bool CAnalysisWindowShiftStretchSubHandler::start(const QXmlAttributes &atts)
{
  if (m_d->nShiftStretch < MAX_AW_SHIFT_STRETCH) {

    QString str;
    struct anlyswin_shift_stretch *d = &(m_d->shiftStretch[m_d->nShiftStretch]);
    
    d->shFit = (atts.value("shfit") == "true") ? 1 : 0;    

    str = atts.value("stfit");
    if (str == "1st") d->stFit = ANLYS_STRETCH_TYPE_FIRST_ORDER;
    else if (str == "2nd") d->stFit = ANLYS_STRETCH_TYPE_SECOND_ORDER;
    else d->stFit = ANLYS_STRETCH_TYPE_NONE;

    str = atts.value("scfit");
    if (str == "1st") d->scFit = ANLYS_STRETCH_TYPE_FIRST_ORDER;
    else if (str == "2nd") d->scFit = ANLYS_STRETCH_TYPE_SECOND_ORDER;
    else d->scFit = ANLYS_STRETCH_TYPE_NONE;

    d->shStore = (atts.value("shstr") == "true") ? 1 : 0;    
    d->stStore = (atts.value("ststr") == "true") ? 1 : 0;    
    d->scStore = (atts.value("scstr") == "true") ? 1 : 0;    
    d->errStore = (atts.value("errstr") == "true") ? 1 : 0;    
    
    d->shInit = atts.value("shini").toInt();
    d->stInit = atts.value("stini").toDouble();
    d->stInit2 = atts.value("stini2").toDouble();
    d->scInit = atts.value("scini").toDouble();
    d->scInit2 = atts.value("scini2").toDouble();
    
    d->shDelta = atts.value("shdel").toInt();
    d->stDelta = atts.value("stdel").toDouble();
    d->stDelta2 = atts.value("stdel2").toDouble();
    d->scDelta = atts.value("scdel").toDouble();
    d->scDelta2 = atts.value("scdel2").toDouble();
    
    d->shMin = atts.value("shmin").toInt();
    d->shMax = atts.value("shmax").toInt();
    
    return true;
  }

  return postErrorMessage("Too many cross sections in analysis window");
}

bool CAnalysisWindowShiftStretchSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "symbol" && m_d->nShiftStretch < MAX_AW_SHIFT_STRETCH) {

    struct anlyswin_shift_stretch *d = &(m_d->shiftStretch[m_d->nShiftStretch]);

    if (d->nSymbol < MAX_AW_SHIFT_STRETCH) {
      
      QString str = atts.value("name");
      if (!str.isEmpty() && str.length() < (int)SYMBOL_NAME_BUFFER_LENGTH) {
	strcpy(&(d->symbol[d->nSymbol][0]), str.toAscii().data());
	++(d->nSymbol);

	return true;
      }
    }
  }

  return false;
}

bool CAnalysisWindowShiftStretchSubHandler::end(void)
{
  if (m_d->nShiftStretch < MAX_AW_SHIFT_STRETCH)
    ++(m_d->nShiftStretch);

  return true;
}

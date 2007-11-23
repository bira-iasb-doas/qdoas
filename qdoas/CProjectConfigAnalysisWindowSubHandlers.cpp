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
#include "CProjectConfigItem.h"

#include "constants.h"

CAnalysisWindowSubHandler::CAnalysisWindowSubHandler(CConfigHandler *master,
						     CAnalysisWindowConfigItem *item) :
  CBasicConfigSubHandler(master),
  m_item(item)
{
}

CAnalysisWindowSubHandler::~CAnalysisWindowSubHandler()
{

}

bool CAnalysisWindowSubHandler::start(const QXmlAttributes &atts)
{
  mediate_analysis_window_t *d = m_item->properties();

  if (!m_item->setName(atts.value("name")))
    return postErrorMessage("Analysis window name too long.");

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

  d->fitMinWavelength = atts.value("min").toDouble();
  d->fitMaxWavelength = atts.value("max").toDouble();

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
    d->refMinLongitude = atts.value("minlon").toDouble();
    d->refMaxLongitude = atts.value("maxlon").toDouble();
    d->refMinLatitude = atts.value("minlat").toDouble();
    d->refMaxLatitude = atts.value("maxlat").toDouble();
    d->refNs = atts.value("refns").toInt();

    d->pixelTypeEast = (atts.value("east") == "true") ? 1 : 0;
    d->pixelTypeCenter = (atts.value("center") == "true") ? 1 : 0;
    d->pixelTypeWest = (atts.value("west") == "true") ? 1 : 0;
    d->pixelTypeBackscan = (atts.value("backscan") == "true") ? 1 : 0;

  }
  else if (element == "cross_section") {
    return m_master->installSubHandler(new CAnalysisWindowCrossSectionSubHandler(m_master, &(d->crossSectionList)), atts);
  }
  else if (element == "linear") {
    return m_master->installSubHandler(new CAnalysisWindowLinearSubHandler(m_master, &(d->linear)), atts);    
  }
  else if (element == "nonlinear") {
    return m_master->installSubHandler(new CAnalysisWindowNonLinearSubHandler(m_master, &(d->nonlinear)), atts);    
  }
  else if (element == "shift_stretch") {
    return m_master->installSubHandler(new CAnalysisWindowShiftStretchSubHandler(m_master, &(d->shiftStretchList)), atts);
  }
  else if (element == "gap") {
    return m_master->installSubHandler(new CAnalysisWindowGapSubHandler(m_master, &(d->gapList)), atts);
  }
  else if (element == "output") {
    return m_master->installSubHandler(new CAnalysisWindowOutputSubHandler(m_master, &(d->outputList)), atts);
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

CAnalysisWindowCrossSectionSubHandler::CAnalysisWindowCrossSectionSubHandler(CConfigHandler *master,
									     cross_section_list_t *d) :
  CBasicConfigSubHandler(master),
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
    d->constrainedCc = (atts.value("cstrncc") == "true") ? 1 : 0;
    d->requireCcFit = (atts.value("ccfit") == "true") ? 1 : 0;
    d->initialCc = atts.value("icc").toDouble();
    d->deltaCc = atts.value("dcc").toDouble();
    d->ccIo = atts.value("ccio").toDouble();

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

//------------------------------------------------------------

CAnalysisWindowLinearSubHandler::CAnalysisWindowLinearSubHandler(CConfigHandler *master,
								 struct anlyswin_linear *d) :
  CBasicConfigSubHandler(master),
  m_d(d)
{
}

CAnalysisWindowLinearSubHandler::~CAnalysisWindowLinearSubHandler()
{
}

bool CAnalysisWindowLinearSubHandler::start(const QXmlAttributes &atts)
{
  m_d->xPolyOrder = CAnalysisWindowSubHandler::mapToPolyType(atts.value("xpoly"));
  m_d->xBaseOrder = CAnalysisWindowSubHandler::mapToPolyType(atts.value("xbase"));
  m_d->xFlagFitStore = (atts.value("xfit") == "true") ? 1 : 0;
  m_d->xFlagErrStore = (atts.value("xerr") == "true") ? 1 : 0;
  
  m_d->xinvPolyOrder = CAnalysisWindowSubHandler::mapToPolyType(atts.value("xinvpoly"));
  m_d->xinvBaseOrder = CAnalysisWindowSubHandler::mapToPolyType(atts.value("xinvbase"));
  m_d->xinvFlagFitStore = (atts.value("xinvfit") == "true") ? 1 : 0;
  m_d->xinvFlagErrStore = (atts.value("xinverr") == "true") ? 1 : 0;
  
  m_d->offsetPolyOrder = CAnalysisWindowSubHandler::mapToPolyType(atts.value("offpoly"));
  m_d->offsetBaseOrder = CAnalysisWindowSubHandler::mapToPolyType(atts.value("offbase"));
  m_d->offsetFlagFitStore = (atts.value("offfit") == "true") ? 1 : 0;
  m_d->offsetFlagErrStore = (atts.value("offerr") == "true") ? 1 : 0;

  return true;
}

//------------------------------------------------------------

CAnalysisWindowNonLinearSubHandler::CAnalysisWindowNonLinearSubHandler(CConfigHandler *master,
								       struct anlyswin_nonlinear *d) :
  CBasicConfigSubHandler(master),
  m_d(d)
{
}

CAnalysisWindowNonLinearSubHandler::~CAnalysisWindowNonLinearSubHandler()
{
}

bool CAnalysisWindowNonLinearSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  m_d->solFlagFit = (atts.value("solfit") == "true") ? 1 : 0;
  m_d->solInitial = atts.value("solinit").toDouble();
  m_d->solDelta = atts.value("soldelt").toDouble();
  m_d->solFlagFitStore = (atts.value("solfstr") == "true") ? 1 : 0;
  m_d->solFlagErrStore = (atts.value("solestr") == "true") ? 1 : 0;

  m_d->off0FlagFit = (atts.value("o0fit") == "true") ? 1 : 0;
  m_d->off0Initial = atts.value("o0init").toDouble();
  m_d->off0Delta = atts.value("o0delt").toDouble();
  m_d->off0FlagFitStore = (atts.value("o0fstr") == "true") ? 1 : 0;
  m_d->off0FlagErrStore = (atts.value("o0estr") == "true") ? 1 : 0;

  m_d->off1FlagFit = (atts.value("o1fit") == "true") ? 1 : 0;
  m_d->off1Initial = atts.value("o1init").toDouble();
  m_d->off1Delta = atts.value("o1delt").toDouble();
  m_d->off1FlagFitStore = (atts.value("o1fstr") == "true") ? 1 : 0;
  m_d->off1FlagErrStore = (atts.value("o1estr") == "true") ? 1 : 0;

  m_d->off2FlagFit = (atts.value("o2fit") == "true") ? 1 : 0;
  m_d->off2Initial = atts.value("o2init").toDouble();
  m_d->off2Delta = atts.value("o2delt").toDouble();
  m_d->off2FlagFitStore = (atts.value("o2fstr") == "true") ? 1 : 0;
  m_d->off2FlagErrStore = (atts.value("o2estr") == "true") ? 1 : 0;

  m_d->comFlagFit = (atts.value("comfit") == "true") ? 1 : 0;
  m_d->comInitial = atts.value("cominit").toDouble();
  m_d->comDelta = atts.value("comdelt").toDouble();
  m_d->comFlagFitStore = (atts.value("comfstr") == "true") ? 1 : 0;
  m_d->comFlagErrStore = (atts.value("comestr") == "true") ? 1 : 0;

  m_d->usamp1FlagFit = (atts.value("u1fit") == "true") ? 1 : 0;
  m_d->usamp1Initial = atts.value("u1init").toDouble();
  m_d->usamp1Delta = atts.value("u1delt").toDouble();
  m_d->usamp1FlagFitStore = (atts.value("u1fstr") == "true") ? 1 : 0;
  m_d->usamp1FlagErrStore = (atts.value("u1estr") == "true") ? 1 : 0;

  m_d->usamp2FlagFit = (atts.value("u2fit") == "true") ? 1 : 0;
  m_d->usamp2Initial = atts.value("u2init").toDouble();
  m_d->usamp2Delta = atts.value("u2delt").toDouble();
  m_d->usamp2FlagFitStore = (atts.value("u2fstr") == "true") ? 1 : 0;
  m_d->usamp2FlagErrStore = (atts.value("u2estr") == "true") ? 1 : 0;

  m_d->ramanFlagFit = (atts.value("ramfit") == "true") ? 1 : 0;
  m_d->ramanInitial = atts.value("raminit").toDouble();
  m_d->ramanDelta = atts.value("ramdelt").toDouble();
  m_d->ramanFlagFitStore = (atts.value("ramfstr") == "true") ? 1 : 0;
  m_d->ramanFlagErrStore = (atts.value("ramestr") == "true") ? 1 : 0;

  str = atts.value("comfile");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->comFile))
      strcpy(m_d->comFile, str.toAscii().data());
    else
      return postErrorMessage("Com filename too long");
  }

  str = atts.value("u1file");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->usamp1File))
      strcpy(m_d->usamp1File, str.toAscii().data());
    else
      return postErrorMessage("Usamp1 filename too long");
  }

  str = atts.value("u2file");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->usamp2File))
      strcpy(m_d->usamp2File, str.toAscii().data());
    else
      return postErrorMessage("Usamp2 filename too long");
  }

  str = atts.value("ramfile");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_d->ramanFile))
      strcpy(m_d->ramanFile, str.toAscii().data());
    else
      return postErrorMessage("Raman filename too long");
  }

  return true;
}

//------------------------------------------------------------

CAnalysisWindowShiftStretchSubHandler::CAnalysisWindowShiftStretchSubHandler(CConfigHandler *master,
									     shift_stretch_list_t *d) :
  CBasicConfigSubHandler(master),
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
    
    d->shInit = atts.value("shini").toDouble();
    d->stInit = atts.value("stini").toDouble();
    d->stInit2 = atts.value("stini2").toDouble();
    d->scInit = atts.value("scini").toDouble();
    d->scInit2 = atts.value("scini2").toDouble();
    
    d->shDelta = atts.value("shdel").toDouble();
    d->stDelta = atts.value("stdel").toDouble();
    d->stDelta2 = atts.value("stdel2").toDouble();
    d->scDelta = atts.value("scdel").toDouble();
    d->scDelta2 = atts.value("scdel2").toDouble();
    
    d->shMin = atts.value("shmin").toDouble();
    d->shMax = atts.value("shmax").toDouble();
    
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

//------------------------------------------------------------

CAnalysisWindowGapSubHandler::CAnalysisWindowGapSubHandler(CConfigHandler *master,
							   gap_list_t *d) :
  CBasicConfigSubHandler(master),
  m_d(d)
{
}

CAnalysisWindowGapSubHandler::~CAnalysisWindowGapSubHandler()
{
}

bool CAnalysisWindowGapSubHandler::start(const QXmlAttributes &atts)
{
  if (m_d->nGap < MAX_AW_GAP) {
    m_d->gap[m_d->nGap].minimum = atts.value("min").toDouble();
    m_d->gap[m_d->nGap].maximum = atts.value("max").toDouble();
    if (m_d->gap[m_d->nGap].minimum < m_d->gap[m_d->nGap].maximum) {
      ++(m_d->nGap);
      return true;
    }
    else
      return postErrorMessage("Invalid gap definition");
  }

  return postErrorMessage("Too many gaps defined");
}

//------------------------------------------------------------

CAnalysisWindowOutputSubHandler::CAnalysisWindowOutputSubHandler(CConfigHandler *master,
								 output_list_t *d) :
  CBasicConfigSubHandler(master),
  m_d(d)
{
}

CAnalysisWindowOutputSubHandler::~CAnalysisWindowOutputSubHandler()
{
}

bool CAnalysisWindowOutputSubHandler::start(const QXmlAttributes &atts)
{
  if (m_d->nOutput < MAX_AW_CROSS_SECTION) {

    QString str;
    struct anlyswin_output *d = &(m_d->output[m_d->nOutput]);
    
    str = atts.value("sym");
    if (!str.isEmpty() && str.length() < (int)sizeof(d->symbol))
      strcpy(d->symbol, str.toAscii().data());
    else
      return postErrorMessage("missing symbol (or name too long)");

    d->amf = (atts.value("amf") == "true") ? 1 : 0;
    // residual TODO
    d->slantCol = (atts.value("scol") == "true") ? 1 : 0;
    d->slantErr = (atts.value("serr") == "true") ? 1 : 0;
    d->slantFactor = atts.value("sfact").toDouble();
    d->vertCol = (atts.value("vcol") == "true") ? 1 : 0;
    d->vertErr = (atts.value("verr") == "true") ? 1 : 0;
    d->vertFactor = atts.value("vfact").toDouble();

    // All OK
    ++(m_d->nOutput);
    
    return true;
  }

  return postErrorMessage("Too many outputs in analysis window");
}

//------------------------------------------------------------

CAnalysisWindowSfpSubHandler::CAnalysisWindowSfpSubHandler(CConfigHandler *master,
							   struct calibration_sfp *d) :
  CBasicConfigSubHandler(master),
  m_d(d)
{
}

CAnalysisWindowSfpSubHandler::~CAnalysisWindowSfpSubHandler()
{
}

bool CAnalysisWindowSfpSubHandler::start(const QXmlAttributes &atts)
{
  int index = atts.value("index").toInt();
  
  if (index > 0 && index <= 4) {
    struct calibration_sfp *p = (m_d + index - 1);

    p->fitFlag = (atts.value("fit") == "true") ? 1 : 0;
    p->initialValue = atts.value("init").toDouble();
    p->deltaValue = atts.value("delta").toDouble();
    p->fitStore = (atts.value("fstr") == "true") ? 1 : 0;
    p->errStore = (atts.value("estr") == "true") ? 1 : 0;
    
    return true;
  }

  return postErrorMessage("Invalid SFP index");
}



#include "CConfigSubHandlerUtils.h"

#include "constants.h"

#include "debugutil.h"

//------------------------------------------------------------------------
// handler for <lowpass_filter> and <highpass_filter>

CFilteringSubHandler::CFilteringSubHandler(CConfigHandler *master,
					   mediate_filter_t *filter) :
  CBasicConfigSubHandler(master),
  m_filter(filter)
{
}

CFilteringSubHandler::~CFilteringSubHandler()
{
}

bool CFilteringSubHandler::start(const QXmlAttributes &atts)
{
  // selected filter

  QString str = atts.value("selected");

  if (str == "none")
    m_filter->mode = PRJCT_FILTER_TYPE_NONE;
  else if (str == "kaiser")
    m_filter->mode = PRJCT_FILTER_TYPE_KAISER;
  else if (str == "boxcar")
    m_filter->mode = PRJCT_FILTER_TYPE_BOXCAR;
  else if (str == "gaussian")
    m_filter->mode = PRJCT_FILTER_TYPE_GAUSSIAN;
  else if (str == "triangular")
    m_filter->mode = PRJCT_FILTER_TYPE_TRIANGLE;
  else if (str == "savitzky")
    m_filter->mode = PRJCT_FILTER_TYPE_SG;
  else if (str == "oddeven")
    m_filter->mode = PRJCT_FILTER_TYPE_ODDEVEN;
  else if (str == "binomial")
    m_filter->mode = PRJCT_FILTER_TYPE_BINOMIAL;
  else
    return postErrorMessage("Invalid filter method");

  return true;
}

bool CFilteringSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // sub element of lowpass_filter or highpass_filter

  if (element == "kaiser") {

    m_filter->kaiser.cutoffFrequency = atts.value("cutoff").toDouble();
    m_filter->kaiser.tolerance = atts.value("tolerance").toDouble();
    m_filter->kaiser.passband = atts.value("passband").toDouble();
    m_filter->kaiser.iterations = atts.value("iterations").toInt();
    m_filter->kaiser.usage.calibrationFlag = (atts.value("cal") == "true") ? 1 : 0;
    m_filter->kaiser.usage.fittingFlag = (atts.value("fit") == "true") ? 1 : 0;
    m_filter->kaiser.usage.divide = (atts.value("div") == "true") ? 1 : 0;
  }
  else if (element == "boxcar") {

    m_filter->boxcar.width = atts.value("width").toInt();
    m_filter->boxcar.iterations = atts.value("iterations").toInt();
    m_filter->boxcar.usage.calibrationFlag = (atts.value("cal") == "true") ? 1 : 0;
    m_filter->boxcar.usage.fittingFlag = (atts.value("fit") == "true") ? 1 : 0;
    m_filter->boxcar.usage.divide = (atts.value("div") == "true") ? 1 : 0;
  }
  else if (element == "gaussian") {

    m_filter->gaussian.fwhm = atts.value("fwhm").toDouble();
    m_filter->gaussian.iterations = atts.value("iterations").toInt();
    m_filter->gaussian.usage.calibrationFlag = (atts.value("cal") == "true") ? 1 : 0;
    m_filter->gaussian.usage.fittingFlag = (atts.value("fit") == "true") ? 1 : 0;
    m_filter->gaussian.usage.divide = (atts.value("div") == "true") ? 1 : 0;
  }
  else if (element == "triangular") {

    m_filter->triangular.width = atts.value("width").toInt();
    m_filter->triangular.iterations = atts.value("iterations").toInt();
    m_filter->triangular.usage.calibrationFlag = (atts.value("cal") == "true") ? 1 : 0;
    m_filter->triangular.usage.fittingFlag = (atts.value("fit") == "true") ? 1 : 0;
    m_filter->triangular.usage.divide = (atts.value("div") == "true") ? 1 : 0;
  }
  else if (element == "savitzky") {

    m_filter->savitzky.width = atts.value("width").toInt();
    m_filter->savitzky.order = atts.value("order").toInt();
    m_filter->savitzky.iterations = atts.value("iterations").toInt();
    m_filter->savitzky.usage.calibrationFlag = (atts.value("cal") == "true") ? 1 : 0;
    m_filter->savitzky.usage.fittingFlag = (atts.value("fit") == "true") ? 1 : 0;
    m_filter->savitzky.usage.divide = (atts.value("div") == "true") ? 1 : 0;
  }
  else if (element == "binomial") {

    m_filter->binomial.width = atts.value("width").toInt();
    m_filter->binomial.iterations = atts.value("iterations").toInt();
    m_filter->binomial.usage.calibrationFlag = (atts.value("cal") == "true") ? 1 : 0;
    m_filter->binomial.usage.fittingFlag = (atts.value("fit") == "true") ? 1 : 0;
    m_filter->binomial.usage.divide = (atts.value("div") == "true") ? 1 : 0;
  }

  return true;
}

//------------------------------------------------------------------------
// handler for <slit_func>

CSlitFunctionSubHandler::CSlitFunctionSubHandler(CConfigHandler *master,
						 mediate_slit_function_t *function) :
  CBasicConfigSubHandler(master),
  m_function(function)
{
}

CSlitFunctionSubHandler::~CSlitFunctionSubHandler()
{
}

bool CSlitFunctionSubHandler::start(const QXmlAttributes &atts)
{
  QString str = atts.value("type");

  if (str == "file")
    m_function->type = SLIT_TYPE_FILE;
  else if (str == "gaussian")
    m_function->type = SLIT_TYPE_GAUSS;
  else if (str == "lorentz")
    m_function->type = SLIT_TYPE_INVPOLY;
  else if (str == "voigt")
    m_function->type = SLIT_TYPE_VOIGT;
  else if (str == "error")
    m_function->type = SLIT_TYPE_ERF;
  else if (str == "agauss")
    m_function->type = SLIT_TYPE_AGAUSS;
  else if (str == "boxcarapod")
    m_function->type = SLIT_TYPE_APOD;
  else if (str == "nbsapod")
    m_function->type = SLIT_TYPE_APODNBS;
  else if (str == "gaussianfile")
    m_function->type = SLIT_TYPE_GAUSS_FILE;
  else if (str == "lorentzfile")
    m_function->type = SLIT_TYPE_INVPOLY_FILE;
  else if (str == "errorfile")
    m_function->type = SLIT_TYPE_ERF_FILE;
  else if (str == "gaussiantempfile")
    m_function->type = SLIT_TYPE_GAUSS_T_FILE;
  else if (str == "errortempfile")
    m_function->type = SLIT_TYPE_ERF_T_FILE;
  else
    return postErrorMessage("Invalid slit type");

  return true;
}

bool CSlitFunctionSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "file") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->file.filename))
	strcpy(m_function->file.filename, str.toAscii().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }
  }
  else if (element == "gaussian") {

    m_function->gaussian.fwhm = atts.value("fwhm").toDouble();
  }
  else if (element == "lorentz") {

    m_function->lorentz.width = atts.value("width").toDouble();
    m_function->lorentz.degree = atts.value("degree").toInt();
  }
  else if (element == "voigt") {

    m_function->voigt.fwhmL = atts.value("fwhmleft").toDouble();
    m_function->voigt.fwhmR = atts.value("fwhmright").toDouble();
    m_function->voigt.glRatioL = atts.value("glrleft").toDouble();
    m_function->voigt.fwhmL = atts.value("glrright").toDouble();
  }
  else if (element == "error") {

    m_function->error.fwhm = atts.value("fwhm").toDouble();
    m_function->error.width = atts.value("width").toDouble();
  }
  else if (element == "agauss") {

    m_function->agauss.fwhm = atts.value("fwhm").toDouble();
    m_function->agauss.asym = atts.value("asym").toDouble();
  }
  else if (element == "boxcarapod") {

    m_function->boxcarapod.resolution = atts.value("resolution").toDouble();
    m_function->boxcarapod.phase = atts.value("phase").toDouble();
  }
  else if (element == "nbsapod") {

    m_function->nbsapod.resolution = atts.value("resolution").toDouble();
    m_function->nbsapod.phase = atts.value("phase").toDouble();
  }
  else if (element == "gaussianfile") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->gaussianfile.filename))
	strcpy(m_function->gaussianfile.filename, str.toAscii().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }
  }
  else if (element == "lorentzfile") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->lorentzfile.filename))
	strcpy(m_function->lorentzfile.filename, str.toAscii().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }

    m_function->lorentzfile.degree = atts.value("degree").toInt();
  }
  else if (element == "errorfile") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->errorfile.filename))
	strcpy(m_function->errorfile.filename, str.toAscii().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }

    m_function->errorfile.width = atts.value("width").toDouble();
  }
  else if (element == "gaussiantempfile") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->gaussiantempfile.filename))
	strcpy(m_function->gaussiantempfile.filename, str.toAscii().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }
  }
  else if (element == "errortempfile") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_function->errortempfile.filename))
	strcpy(m_function->errortempfile.filename, str.toAscii().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }

    m_function->errortempfile.width = atts.value("width").toDouble();
  }

  return true;
}


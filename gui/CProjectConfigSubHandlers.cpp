
#include "CProjectConfigSubHandlers.h"

#include "debugutil.h"

//------------------------------------------------------------------------
//
// Handler for <paths> element (and sub elements)

CPathSubHandler::CPathSubHandler(CQdoasProjectConfigHandler *master) :
  CConfigSubHandler(master),
  m_index(-1)
{
}

CPathSubHandler::~CPathSubHandler()
{
}

bool CPathSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // should be a path element <path index="?">/this/is/the/path</path>

  
  if (element == "path") {
    bool ok;
    m_index = atts.value("index").toInt(&ok);
    if (!ok || m_index < 0 || m_index > 9) {
      m_index = -1;
      return postErrorMessage("Invalid path index");
    }
  }
  else {
    m_index = -1;
    return postErrorMessage("Invalid child element of paths");
  }

  m_path.clear();

  return true;
}

bool CPathSubHandler::character(const QString &ch)
{
  // collect all path characters
  m_path += ch;

  return true;
}

bool CPathSubHandler::end(const QString &element)
{
  if (m_index != -1)
    m_master->setPath(m_index, m_path);

  return true;
}

//------------------------------------------------------------------------
//
// Handler for <project> element


CProjectSubHandler::CProjectSubHandler(CQdoasProjectConfigHandler *master) :
  CConfigSubHandler(master)
{
  m_project = new CProjectConfigItem;
}

CProjectSubHandler::~CProjectSubHandler()
{
  delete m_project;
}

bool CProjectSubHandler::start(const QXmlAttributes &atts)
{
  // the project element - must have a name

  m_project->setProjectName(atts.value("name"));

  return !m_project->projectName().isEmpty();
}

bool CProjectSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // a sub element of project ... create a specialized handler and delegate
  mediate_project_t *prop = m_project->properties();

  if (element == "spectra") {
    return m_master->installSubHandler(new CProjectSpectraSubHandler(m_master, &(prop->spectra)), atts);
  }
  else if (element == "analysis") {
    return m_master->installSubHandler(new CProjectAnalysisSubHandler(m_master, &(prop->analysis)), atts);
  }
  else if (element == "raw_spectra") {
    return m_master->installSubHandler(new CProjectRawSpectraSubHandler(m_master, m_project->rootNode()), atts);
  }
  else if (element == "lowpass_filter") {
    return m_master->installSubHandler(new CProjectFilteringSubHandler(m_master, &(prop->lowpass)), atts);
  }
  else if (element == "highpass_filter") {
    return m_master->installSubHandler(new CProjectFilteringSubHandler(m_master, &(prop->highpass)), atts);
  }
  else if (element == "calibration") {
    return m_master->installSubHandler(new CProjectCalibrationSubHandler(m_master, &(prop->calibration)), atts);
  }

  TRACE2("proj Handler : " << element.toStdString());

  return true;
}

bool CProjectSubHandler::end(const QString &element)
{
  // TODO - remove this ...
  // end of sub element ... all sub elements are managed by sub handlers.
  return true;
}

bool CProjectSubHandler::end()
{
  // end of project ... hand project data over to the master handler

  m_master->addProjectItem(m_project);
  m_project = NULL; // releases ownership responsibility

  return true;
}

//------------------------------------------------------------------------
// handler for <spectra> (child of project)

CProjectSpectraSubHandler::CProjectSpectraSubHandler(CQdoasProjectConfigHandler *master,
						     mediate_project_spectra_t *spectra) :
  CConfigSubHandler(master),
  m_spectra(spectra)
{
}

CProjectSpectraSubHandler::~CProjectSpectraSubHandler()
{
}

bool CProjectSpectraSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "display") {

    // default to false if attributes are not present

    m_spectra->requireSpectra = (atts.value("spectra") == "true") ? 1 : 0;
    m_spectra->requireData = (atts.value("data") == "true") ? 1 : 0;
    m_spectra->requireFits = (atts.value("fits") == "true") ? 1 : 0;
  }
  else if (element == "sza") {

    // defaults from mediateInitializeProject() are ok if attributes are not present

    QString str;
    bool ok;
    double tmp;

    str = atts.value("min");
    if (!str.isEmpty()) {
      tmp = str.toDouble(&ok);
      if (ok)
	m_spectra->szaMinimum = tmp;
      else
	return postErrorMessage("Invalid value for min");
    }

    str = atts.value("max");
    tmp = str.toDouble(&ok);
    if (ok)
      m_spectra->szaMaximum = tmp;

    str = atts.value("delta");
    tmp = str.toDouble(&ok);
    if (ok)
      m_spectra->szaDelta = tmp;
  }
  else if (element == "record") {

    // defaults from mediateInitializeProject() are ok if attributes are not present

    QString str;
    bool ok;
    int tmp;

    str = atts.value("min");
    tmp = str.toInt(&ok);
    if (ok)
      m_spectra->recordNumberMinimum = tmp;

    str = atts.value("max");
    tmp = str.toInt(&ok);
    if (ok)
      m_spectra->recordNumberMaximum = tmp;
  }
  else if (element == "files") {

    // defaults to false if attributes are no present

    m_spectra->useDarkFile = (atts.value("dark") == "true") ? 1 : 0;
    m_spectra->useNameFile = (atts.value("name") == "true") ? 1 : 0;
  }
  else if (element == "circle") {
    
    // defaults from mediateInitializeProject() are ok if attributes are not present

    QString str;
    bool ok;
    double tmp;

    str = atts.value("radius");
    tmp = str.toDouble(&ok);
    if (ok)
      m_spectra->geo.circle.radius = tmp;

    str = atts.value("long");
    tmp = str.toDouble(&ok);
    if (ok)
      m_spectra->geo.circle.centerLongitude = tmp;

    str = atts.value("lat");
    tmp = str.toDouble(&ok);
    if (ok)
      m_spectra->geo.circle.centerLatitude = tmp;
  }
  else if (element == "rectangle") {

    // defaults from mediateInitializeProject() are ok if attributes are not present

    QString str;
    bool ok;
    double tmp;

    str = atts.value("east");
    tmp = str.toDouble(&ok);
    if (ok)
      m_spectra->geo.rectangle.easternLongitude = tmp;

    str = atts.value("west");
    tmp = str.toDouble(&ok);
    if (ok)
      m_spectra->geo.rectangle.westernLongitude = tmp;

    str = atts.value("north");
    tmp = str.toDouble(&ok);
    if (ok)
      m_spectra->geo.rectangle.northernLatitude = tmp;

    str = atts.value("south");
    tmp = str.toDouble(&ok);
    if (ok)
      m_spectra->geo.rectangle.southernLatitude = tmp;
  }
  else if (element == "geolocation") {
    
    QString selected = atts.value("selected");

    if (selected == "circle")
      m_spectra->geo.mode = cGeolocationModeCircle;
    else if (selected == "rectangle")
      m_spectra->geo.mode = cGeolocationModeRectangle;
    else
      m_spectra->geo.mode = cGeolocationModeNone; // default and "none"
  }

  return true;
}

bool CProjectSpectraSubHandler::end(const QString &element)
{
  return true;
}


//------------------------------------------------------------------------
// handler for <analysis> (child of project)

CProjectAnalysisSubHandler::CProjectAnalysisSubHandler(CQdoasProjectConfigHandler *master,
						       mediate_project_analysis_t *analysis) :
  CConfigSubHandler(master),
  m_analysis(analysis)
{
}

CProjectAnalysisSubHandler::~CProjectAnalysisSubHandler()
{
}

bool CProjectAnalysisSubHandler::start(const QXmlAttributes &atts)
{
  // all options are in the attributes of the analysis element itself

  QString str;
  int tmpInt;
  double tmpDouble;
  bool ok;

  str = atts.value("method");
  if (str == "ODF")
    m_analysis->methodType = cProjAnalysisMethodModeOptDens;
  else if (str == "ML+SVD")
    m_analysis->methodType = cProjAnalysisMethodModeMarqLevSvd;
  else if (str == "FML")
    m_analysis->methodType = cProjAnalysisMethodModeMarqLevFull;
  else
    return postErrorMessage("Invalid analysis method");
  
  
  str = atts.value("fit");
  if (str == "none")
    m_analysis->methodType = cProjAnalysisFitModeNone;
  else if (str == "instr")
    m_analysis->methodType = cProjAnalysisFitModeInstrumental;
  else if (str == "stat")
    m_analysis->methodType = cProjAnalysisFitModeStatistical;
  else
    return postErrorMessage("Invalid analysis fit");
  
  
  str = atts.value("unit");
  if (str == "pixel")
    m_analysis->methodType = cProjAnalysisUnitModePixel;
  else if (str == "nm")
    m_analysis->methodType = cProjAnalysisUnitModeNanometer;
  else
    return postErrorMessage("Invalid analysis unit");
    

  str = atts.value("interpolation");
  if (str == "linear")
    m_analysis->methodType = cProjAnalysisInterpolationModeLinear;
  else if (str == "spline")
    m_analysis->methodType = cProjAnalysisInterpolationModeSpline;
  else
    return postErrorMessage("Invalid analysis interpolation");
    

  str = atts.value("gap");
  if (!str.isEmpty()) {
    tmpInt = str.toInt(&ok);
    if (ok)
      m_analysis->interpolationSecurityGap = tmpInt;
    else
      return postErrorMessage("Invalid analysis gap");
  }

  str = atts.value("converge");
  if (!str.isEmpty()) {
    tmpDouble = str.toDouble(&ok);
    if (ok)
      m_analysis->convergenceCriterion = tmpDouble;
    else
      return postErrorMessage("Invalid analysis gap");
  }
      
  return true;
}


//------------------------------------------------------------------------
// handler for <raw_spectra> (child of project)

CProjectRawSpectraSubHandler::CProjectRawSpectraSubHandler(CQdoasProjectConfigHandler *master,
							   CProjectConfigTreeNode *node) :
  CConfigSubHandler(master),
  m_node(node)
{
}

CProjectRawSpectraSubHandler::~CProjectRawSpectraSubHandler()
{
}

bool CProjectRawSpectraSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // <directory>, <file> or <folder>

  QString name = atts.value("name");
  bool enabled = (atts.value("disabled") != "true");

  // MUST have a name ...
  if (name.isEmpty())
    return false;

  if (element == "file") {
    // expand the name (the filename)
    name = m_master->pathExpand(name);
    m_node->addChild(new CProjectConfigFile(name, enabled));
  }
  else if (element == "directory") {

    // expand the name (the directory name)
    name = m_master->pathExpand(name);
    m_node->addChild(new CProjectConfigDirectory(name, atts.value("filter"),
						 (atts.value("recursive") == "true"), enabled));
  }
  else if (element == "folder") {
    // create an item for the folder now ...
    CProjectConfigFolder *item = new CProjectConfigFolder(name, enabled);
    m_node->addChild(item);

    // and a sub handler for child items
    return m_master->installSubHandler(new CProjectRawSpectraSubHandler(m_master, item), atts);
  }

  return true;
}


//------------------------------------------------------------------------
// handler for <lowpass_filter> and <highpass_filter> (children of project)

CProjectFilteringSubHandler::CProjectFilteringSubHandler(CQdoasProjectConfigHandler *master,
						       mediate_project_filtering_t *filter) :
  CConfigSubHandler(master),
  m_filter(filter)
{
}

CProjectFilteringSubHandler::~CProjectFilteringSubHandler()
{
}

bool CProjectFilteringSubHandler::start(const QXmlAttributes &atts)
{
  // selected filter

  QString str = atts.value("selected");

  if (str == "none")
    m_filter->mode = cProjFilteringModeNone;
  else if (str == "kaiser")
    m_filter->mode = cProjFilteringModeKaiser;
  else if (str == "boxcar")
    m_filter->mode = cProjFilteringModeBoxcar;
  else if (str == "gaussian")
    m_filter->mode = cProjFilteringModeGaussian;
  else if (str == "triangular")
    m_filter->mode = cProjFilteringModeTriangular;
  else if (str == "savitzky")
    m_filter->mode = cProjFilteringModeSavitzkyGolay;
  else if (str == "oddeven")
    m_filter->mode = cProjFilteringModeOddEvenCorrection;
  else if (str == "binomial")
    m_filter->mode = cProjFilteringModeBinomial;
  else
    return postErrorMessage("Invalid filter method");
  
  return true;
}

bool CProjectFilteringSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // sub element of lowpass_filter or highpass_filter
  
  if (element == "kaiser") {
    bool ok;
    double tmpDouble;
    int tmpInt;

    tmpDouble = atts.value("cutoff").toDouble(&ok);
    if (ok)
      m_filter->kaiser.cutoffFrequency = tmpDouble;
    else
      return postErrorMessage("Invalid cutoff frequency for kaiser filter");

    tmpDouble = atts.value("tolerance").toDouble(&ok);
    if (ok)
      m_filter->kaiser.tolerance = tmpDouble;
    else
      return postErrorMessage("Invalid tolerance for kaiser filter");

    tmpDouble = atts.value("passband").toDouble(&ok);
    if (ok)
      m_filter->kaiser.passband = tmpDouble;
    else
      return postErrorMessage("Invalid passband for kaiser filter");

    tmpInt = atts.value("iterations").toInt(&ok);
    if (ok)
      m_filter->kaiser.iterations = tmpInt;
    else
      return postErrorMessage("Invalid iterations for kaiser filter");

  }
  else if (element == "boxcar") {
    bool ok;
    int tmpInt;

    tmpInt = atts.value("width").toInt(&ok);
    if (ok)
      m_filter->boxcar.width = tmpInt;
    else
      return postErrorMessage("Invalid width for boxcar filter");

    tmpInt = atts.value("iterations").toInt(&ok);
    if (ok)
      m_filter->boxcar.iterations = tmpInt;
    else
      return postErrorMessage("Invalid iterations for boxcar filter");

  }
  else if (element == "gaussian") {
    bool ok;
    double tmpDouble;
    int tmpInt;

    tmpDouble = atts.value("fwhm").toDouble(&ok);
    if (ok)
      m_filter->gaussian.fwhm = tmpDouble;
    else
      return postErrorMessage("Invalid passband for gaussian filter");

    tmpInt = atts.value("iterations").toInt(&ok);
    if (ok)
      m_filter->gaussian.iterations = tmpInt;
    else
      return postErrorMessage("Invalid iterations for gaussian filter");

  }
  else if (element == "triangular") {
    bool ok;
    int tmpInt;

    tmpInt = atts.value("width").toInt(&ok);
    if (ok)
      m_filter->triangular.width = tmpInt;
    else
      return postErrorMessage("Invalid width for triangular filter");

    tmpInt = atts.value("iterations").toInt(&ok);
    if (ok)
      m_filter->triangular.iterations = tmpInt;
    else
      return postErrorMessage("Invalid iterations for triangular filter");

  }
  else if (element == "savitzky") {
    bool ok;
    int tmpInt;

    tmpInt = atts.value("width").toInt(&ok);
    if (ok)
      m_filter->savitzky.width = tmpInt;
    else
      return postErrorMessage("Invalid width for savitzky filter");

    tmpInt = atts.value("order").toInt(&ok);
    if (ok)
      m_filter->savitzky.order = tmpInt;
    else
      return postErrorMessage("Invalid order for savitzky filter");

    tmpInt = atts.value("iterations").toInt(&ok);
    if (ok)
      m_filter->savitzky.iterations = tmpInt;
    else
      return postErrorMessage("Invalid iterations for savitzky filter");

  }
  else if (element == "binomial") {
    bool ok;
    int tmpInt;

    tmpInt = atts.value("width").toInt(&ok);
    if (ok)
      m_filter->binomial.width = tmpInt;
    else
      return postErrorMessage("Invalid width for binomial filter");

    tmpInt = atts.value("iterations").toInt(&ok);
    if (ok)
      m_filter->binomial.iterations = tmpInt;
    else
      return postErrorMessage("Invalid iterations for binomial filter");
  }

  return true;
}

//------------------------------------------------------------------------
// handler for <calibration> (child of project)

CProjectCalibrationSubHandler::CProjectCalibrationSubHandler(CQdoasProjectConfigHandler *master,
						       mediate_project_calibration_t *calibration) :
  CConfigSubHandler(master),
  m_calibration(calibration)
{
}

CProjectCalibrationSubHandler::~CProjectCalibrationSubHandler()
{
}

bool CProjectCalibrationSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  str = atts.value("method");
  if (str == "ODF")
    m_calibration->method = cProjAnalysisMethodModeOptDens;
  else if (str == "ML+SVD")
    m_calibration->method = cProjAnalysisMethodModeMarqLevSvd;
  else if (str == "FML")
    m_calibration->method = cProjAnalysisMethodModeMarqLevFull;
  else
    return postErrorMessage("Invalid analysis method");
  
  str = atts.value("ref");
  if (!str.isEmpty()) {
    if (str.length() < (int)sizeof(m_calibration->solarRefFile))
      strcpy(m_calibration->solarRefFile, str.toAscii().data());
    else
      return postErrorMessage("Solar Reference Filename too long");
  }
  
  return true;
}

bool CProjectCalibrationSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // sub element of calibration
  
  if (element == "line") {
    QString str;
    bool ok;
    int tmpInt;

    str = atts.value("shape");
    if (str == "none")
      m_calibration->lineShape = cSpectralLineShapeNone;
    else if (str == "gaussian")
      m_calibration->lineShape = cSpectralLineShapeGaussian;
    else if (str == "error")
      m_calibration->lineShape = cSpectralLineShapeErrorFunction;
    else if (str == "lorentz")
      m_calibration->lineShape = cSpectralLineShapeLorentz;
    else if (str == "voigt")
      m_calibration->lineShape = cSpectralLineShapeVoigt;
    else
      postErrorMessage("Invalid line shape");

    tmpInt = atts.value("lorentzdegree").toInt(&ok);
    if (ok)
      m_calibration->lorentzDegree = tmpInt;

  }
  else if (element == "display") {

    m_calibration->requireSpectra = (atts.value("spectra") == "true") ? 1 : 0;
    m_calibration->requireFits = (atts.value("fits") == "true") ? 1 : 0;
    m_calibration->requireResidual = (atts.value("residual") == "true") ? 1 : 0;
    m_calibration->requireShiftSfp = (atts.value("shiftsfp") == "true") ? 1 : 0;
  }
  else if (element == "polynomial") {
    bool ok;
    int tmpInt;

    tmpInt = atts.value("shift").toInt(&ok);
    if (ok)
      m_calibration->shiftDegree = tmpInt;

    tmpInt = atts.value("sfp").toInt(&ok);
    if (ok)
      m_calibration->sfpDegree = tmpInt;

  }
  else if (element == "window") {
    bool ok;
    int tmpInt;
    double tmpDouble;

    tmpDouble = atts.value("min").toDouble(&ok);
    if (ok)
      m_calibration->wavelengthMin = tmpDouble;

    tmpDouble = atts.value("max").toDouble(&ok);
    if (ok)
      m_calibration->wavelengthMax = tmpDouble;

    tmpInt = atts.value("intervals").toInt(&ok);
    if (ok)
      m_calibration->subWindows = tmpInt;

  }

  return true;
}

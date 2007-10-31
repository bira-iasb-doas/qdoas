
#include "CProjectConfigSubHandlers.h"
#include "CProjectConfigAnalysisWindowSubHandlers.h"

#include "constants.h"

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
// Handler for <sites> element (and sub elements)

CSiteSubHandler::CSiteSubHandler(CQdoasProjectConfigHandler *master) :
  CConfigSubHandler(master)
{
}

CSiteSubHandler::~CSiteSubHandler()
{
}

bool CSiteSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "site") {
    QString str;
    bool ok;
    double tmpDouble;

    // create a new config item for the site
    CSiteConfigItem *item = new CSiteConfigItem;

    str = atts.value("name");
    if (str.isEmpty()) {
      delete item;
      return postErrorMessage("Missing site name");
    }
    else
      item->setSiteName(str);

    str = atts.value("abbrev");
    if (!str.isEmpty())
      item->setAbbreviation(str);

    tmpDouble = atts.value("long").toDouble(&ok);
    if (ok)
      item->setLongitude(tmpDouble);

    tmpDouble = atts.value("lat").toDouble(&ok);
    if (ok)
      item->setLatitude(tmpDouble);

    tmpDouble = atts.value("alt").toDouble(&ok);
    if (ok)
      item->setAltitude(tmpDouble);

    m_master->addSiteItem(item);

    return true;
  }

  return false;
}

//------------------------------------------------------------------------
//
// Handler for <symbol> element

CSymbolSubHandler::CSymbolSubHandler(CQdoasProjectConfigHandler *master) :
  CConfigSubHandler(master)
{
}

CSymbolSubHandler::~CSymbolSubHandler()
{
}

bool CSymbolSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "symbol") {
    QString name;

    name = atts.value("name");
    if (name.isEmpty()) {
      return postErrorMessage("Missing symbol name");
    }

    m_master->addSymbol(name, atts.value("descr"));

    return true;
  }

  return false;
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

  m_project->setName(atts.value("name"));

  return !m_project->name().isEmpty();
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
  else if (element == "undersampling") {
    return m_master->installSubHandler(new CProjectUndersamplingSubHandler(m_master, &(prop->undersampling)), atts);
  }
  else if (element == "instrumental") {
    return m_master->installSubHandler(new CProjectInstrumentalSubHandler(m_master, &(prop->instrumental)), atts);
  }
  else if (element == "slit") {
    return m_master->installSubHandler(new CProjectSlitSubHandler(m_master, &(prop->slit)), atts);
  }
  else if (element == "output") {
    return m_master->installSubHandler(new CProjectOutputSubHandler(m_master, &(prop->output)), atts);
  }
  else if (element == "nasa_ames") {
    return m_master->installSubHandler(new CProjectNasaAmesSubHandler(m_master, &(prop->nasaames)), atts);
  }
  else if (element == "analysis_window") {
    // allocate a new item in the project for this AW
    CAnalysisWindowConfigItem *awItem = m_project->issueNewAnalysisWindowItem();
    if (awItem)
      return m_master->installSubHandler(new CAnalysisWindowSubHandler(m_master, awItem), atts);

    return false; // fall through failure
  }

  TRACE2("proj Handler : " << element.toStdString());

  return true; // TODO - false unknown element ...
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
    m_spectra->szaMinimum = atts.value("min").toDouble();
    m_spectra->szaMaximum = atts.value("max").toDouble();
    m_spectra->szaDelta = atts.value("delta").toDouble();

  }
  else if (element == "record") {

    // defaults from mediateInitializeProject() are ok if attributes are not present
    m_spectra->recordNumberMinimum = atts.value("min").toInt();
    m_spectra->recordNumberMaximum = atts.value("max").toInt();
  }
  else if (element == "files") {

    // defaults to false if attributes are no present
    m_spectra->useDarkFile = (atts.value("dark") == "true") ? 1 : 0;
    m_spectra->useNameFile = (atts.value("name") == "true") ? 1 : 0;
  }
  else if (element == "circle") {

    m_spectra->geo.circle.radius = atts.value("radius").toDouble();
    m_spectra->geo.circle.centerLongitude = atts.value("long").toDouble();
    m_spectra->geo.circle.centerLatitude = atts.value("lat").toDouble();
  }
  else if (element == "rectangle") {

    m_spectra->geo.rectangle.easternLongitude = atts.value("east").toDouble();
    m_spectra->geo.rectangle.westernLongitude = atts.value("west").toDouble();
    m_spectra->geo.rectangle.northernLatitude = atts.value("north").toDouble();
    m_spectra->geo.rectangle.southernLatitude = atts.value("south").toDouble();
  }
  else if (element == "sites") {

    m_spectra->geo.sites.radius = atts.value("radius").toDouble();
  }
  else if (element == "geolocation") {

    QString selected = atts.value("selected");

    if (selected == "circle")
      m_spectra->geo.mode = PRJCT_SPECTRA_MODES_CIRCLE;
    else if (selected == "rectangle")
      m_spectra->geo.mode = PRJCT_SPECTRA_MODES_RECTANGLE;
    else if (selected == "sites")
      m_spectra->geo.mode = PRJCT_SPECTRA_MODES_OBSLIST;
    else
      m_spectra->geo.mode = PRJCT_SPECTRA_MODES_NONE; // default and "none"
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

  str = atts.value("method");
  if (str == "ODF")
    m_analysis->methodType = PRJCT_ANLYS_METHOD_SVD;
  else if (str == "ML+SVD")
    m_analysis->methodType = PRJCT_ANLYS_METHOD_SVDMARQUARDT;
  else
    return postErrorMessage("Invalid analysis method");

  str = atts.value("fit");
  if (str == "none")
    m_analysis->methodType = PRJCT_ANLYS_FIT_WEIGHTING_NONE;
  else if (str == "instr")
    m_analysis->methodType = PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL;
  else
    return postErrorMessage("Invalid analysis fit");

  str = atts.value("unit");
  if (str == "pixel")
    m_analysis->methodType = PRJCT_ANLYS_UNITS_PIXELS;
  else if (str == "nm")
    m_analysis->methodType = PRJCT_ANLYS_UNITS_NANOMETERS;
  else
    return postErrorMessage("Invalid analysis unit");

  str = atts.value("interpolation");
  if (str == "linear")
    m_analysis->methodType = PRJCT_ANLYS_INTERPOL_LINEAR;
  else if (str == "spline")
    m_analysis->methodType = PRJCT_ANLYS_INTERPOL_SPLINE;
  else
    return postErrorMessage("Invalid analysis interpolation");

  m_analysis->interpolationSecurityGap = atts.value("gap").toInt();
  m_analysis->convergenceCriterion = atts.value("converge").toDouble();

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
    m_node->addChild(new CProjectConfigDirectory(name, atts.value("filters"),
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
    m_filter->mode = PRJCT_FILTER_TYPE_BINOMIAL;
  else if (str == "binomial")
    m_filter->mode = PRJCT_FILTER_TYPE_BINOMIAL;
  else
    return postErrorMessage("Invalid filter method");

  return true;
}

bool CProjectFilteringSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // sub element of lowpass_filter or highpass_filter

  if (element == "kaiser") {

    m_filter->kaiser.cutoffFrequency = atts.value("cutoff").toDouble();
    m_filter->kaiser.tolerance = atts.value("tolerance").toDouble();
    m_filter->kaiser.passband = atts.value("passband").toDouble();
    m_filter->kaiser.iterations = atts.value("iterations").toInt();
  }
  else if (element == "boxcar") {

    m_filter->boxcar.width = atts.value("width").toInt();
    m_filter->boxcar.iterations = atts.value("iterations").toInt();
  }
  else if (element == "gaussian") {

    m_filter->gaussian.fwhm = atts.value("fwhm").toDouble();
    m_filter->gaussian.iterations = atts.value("iterations").toInt();
  }
  else if (element == "triangular") {

    m_filter->triangular.width = atts.value("width").toInt();
    m_filter->triangular.iterations = atts.value("iterations").toInt();
  }
  else if (element == "savitzky") {

    m_filter->savitzky.width = atts.value("width").toInt();
    m_filter->savitzky.order = atts.value("order").toInt();
    m_filter->savitzky.iterations = atts.value("iterations").toInt();
  }
  else if (element == "binomial") {

    m_filter->binomial.width = atts.value("width").toInt();
    m_filter->binomial.iterations = atts.value("iterations").toInt();
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
    m_calibration->methodType = PRJCT_ANLYS_METHOD_SVD;
  else if (str == "ML+SVD")
    m_calibration->methodType = PRJCT_ANLYS_METHOD_SVDMARQUARDT;
  else
    return postErrorMessage("Invalid analysis method");

  str = atts.value("ref");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
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

    str = atts.value("shape");
    if (str == "none")
      m_calibration->lineShape = PRJCT_CALIB_FWHM_TYPE_NONE;
    else if (str == "gaussian")
      m_calibration->lineShape = PRJCT_CALIB_FWHM_TYPE_GAUSS;
    else if (str == "error")
      m_calibration->lineShape = PRJCT_CALIB_FWHM_TYPE_ERF;
    else if (str == "lorentz")
      m_calibration->lineShape = PRJCT_CALIB_FWHM_TYPE_INVPOLY;
    else if (str == "voigt")
      m_calibration->lineShape = PRJCT_CALIB_FWHM_TYPE_VOIGT;
    else
      postErrorMessage("Invalid line shape");

    m_calibration->lorentzDegree = atts.value("lorentzdegree").toInt();
  }
  else if (element == "display") {

    m_calibration->requireSpectra = (atts.value("spectra") == "true") ? 1 : 0;
    m_calibration->requireFits = (atts.value("fits") == "true") ? 1 : 0;
    m_calibration->requireResidual = (atts.value("residual") == "true") ? 1 : 0;
    m_calibration->requireShiftSfp = (atts.value("shiftsfp") == "true") ? 1 : 0;
  }
  else if (element == "polynomial") {

    m_calibration->shiftDegree = atts.value("shift").toInt();
    m_calibration->sfpDegree = atts.value("sfp").toInt();
  }
  else if (element == "window") {

    m_calibration->wavelengthMin = atts.value("min").toDouble();
    m_calibration->wavelengthMax = atts.value("max").toDouble();
    m_calibration->subWindows = atts.value("intervals").toInt();
  }
  else if (element == "cross_section") {
    return m_master->installSubHandler(new CAnalysisWindowCrossSectionSubHandler(m_master, &(m_calibration->crossSectionList)), atts);
  }
  else if (element == "linear") {
    return m_master->installSubHandler(new CAnalysisWindowLinearSubHandler(m_master, &(m_calibration->linear)), atts);    
  }
  else if (element == "sfp") {
    return m_master->installSubHandler(new CAnalysisWindowSfpSubHandler(m_master, &(m_calibration->sfp[0])), atts); 
  }
  else if (element == "shift_stretch") {
    return m_master->installSubHandler(new CAnalysisWindowShiftStretchSubHandler(m_master, &(m_calibration->shiftStretchList)), atts);
  }
  else if (element == "gap") {
    return m_master->installSubHandler(new CAnalysisWindowGapSubHandler(m_master, &(m_calibration->gapList)), atts);
  }
  else if (element == "output") {
    return m_master->installSubHandler(new CAnalysisWindowOutputSubHandler(m_master, &(m_calibration->outputList)), atts);
  }

  return true;
}

//------------------------------------------------------------------------
// handler for <undersampling> (child of project)

CProjectUndersamplingSubHandler::CProjectUndersamplingSubHandler(CQdoasProjectConfigHandler *master,
								 mediate_project_undersampling_t *undersampling) :
  CConfigSubHandler(master),
  m_undersampling(undersampling)
{
}

CProjectUndersamplingSubHandler::~CProjectUndersamplingSubHandler()
{
}

bool CProjectUndersamplingSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  str = atts.value("method");
  if (str == "file")
    m_undersampling->method = PRJCT_USAMP_FILE;
  else if (str == "fixed")
    m_undersampling->method = PRJCT_USAMP_FIXED;
  else if (str == "auto")
    m_undersampling->method = PRJCT_USAMP_AUTOMATIC;
  else
    return postErrorMessage("Invalid analysis method");

  str = atts.value("ref");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_undersampling->solarRefFile))
      strcpy(m_undersampling->solarRefFile, str.toAscii().data());
    else
      return postErrorMessage("Solar Reference Filename too long");
  }

  m_undersampling->shift = atts.value("shift").toDouble();

  return true;
}

//------------------------------------------------------------------------
// handler for <instrumental> (child of project)

CProjectInstrumentalSubHandler::CProjectInstrumentalSubHandler(CQdoasProjectConfigHandler *master,
							       mediate_project_instrumental_t *instrumental) :
  CConfigSubHandler(master),
  m_instrumental(instrumental)
{
}

CProjectInstrumentalSubHandler::~CProjectInstrumentalSubHandler()
{
}

bool CProjectInstrumentalSubHandler::start(const QXmlAttributes &atts)
{
  // the <instrumental ...> element

  QString str;

  str = atts.value("format");
  if (str == "ascii")
    m_instrumental->format = PRJCT_INSTR_FORMAT_ASCII;
  else if (str == "logger")
    m_instrumental->format = PRJCT_INSTR_FORMAT_LOGGER;
  else if (str == "acton")
    m_instrumental->format = PRJCT_INSTR_FORMAT_ACTON;
  else if (str == "pdaegg")
    m_instrumental->format = PRJCT_INSTR_FORMAT_PDAEGG;
  else if (str == "pdaegg_old")
    m_instrumental->format = PRJCT_INSTR_FORMAT_PDAEGG_OLD;
  else if (str == "pdaegg_ulb")
    m_instrumental->format = PRJCT_INSTR_FORMAT_PDAEGG_ULB;
  else if (str == "ccdohp_96")
    m_instrumental->format = PRJCT_INSTR_FORMAT_CCD_OHP_96;
  else if (str == "ccdha_94")
    m_instrumental->format = PRJCT_INSTR_FORMAT_CCD_HA_94;
  else if (str == "ccdulb")
    m_instrumental->format = PRJCT_INSTR_FORMAT_CCD_ULB;
  else if (str == "saozvis")
    m_instrumental->format = PRJCT_INSTR_FORMAT_SAOZ_VIS;
  else if (str == "saozuv")
    m_instrumental->format = PRJCT_INSTR_FORMAT_SAOZ_UV;
  else if (str == "saozefm")
    m_instrumental->format = PRJCT_INSTR_FORMAT_SAOZ_EFM;
  else if (str == "mfc")
    m_instrumental->format = PRJCT_INSTR_FORMAT_MFC;
  else if (str == "mfcstd")
    m_instrumental->format = PRJCT_INSTR_FORMAT_MFC_STD;
  else if (str == "rasas")
    m_instrumental->format = PRJCT_INSTR_FORMAT_RASAS;
  else if (str == "pdasieasoe")
    m_instrumental->format = PRJCT_INSTR_FORMAT_PDASI_EASOE;
  else if (str == "pdasiosma")
    m_instrumental->format = PRJCT_INSTR_FORMAT_PDASI_OSMA;
  else if (str == "ccdeev")
    m_instrumental->format = PRJCT_INSTR_FORMAT_CCD_EEV;
  else if (str == "opus")
    m_instrumental->format = PRJCT_INSTR_FORMAT_OPUS;
  else if (str == "gdpascii")
    m_instrumental->format = PRJCT_INSTR_FORMAT_GDP_ASCII;
  else if (str == "gdpbin")
    m_instrumental->format = PRJCT_INSTR_FORMAT_GDP_BIN;
  else if (str == "sciahdf")
    m_instrumental->format = PRJCT_INSTR_FORMAT_SCIA_HDF;
  else if (str == "sciapds")
    m_instrumental->format = PRJCT_INSTR_FORMAT_SCIA_PDS;
  else if (str == "uoft")
    m_instrumental->format = PRJCT_INSTR_FORMAT_UOFT;
  else if (str == "noaa")
    m_instrumental->format = PRJCT_INSTR_FORMAT_NOAA;
  else if (str == "omi")
    m_instrumental->format = PRJCT_INSTR_FORMAT_OMI;
  else if (str == "gome2")
    m_instrumental->format = PRJCT_INSTR_FORMAT_GOME2;
  else
    return postErrorMessage("Invalid instrumental format");

  str = atts.value("site");
  if (!str.isEmpty()) {
    if (str.length() < (int)sizeof(m_instrumental->siteName))
      strcpy(m_instrumental->siteName, str.toAscii().data());
    else
      return postErrorMessage("Instrumental Site Name too long");
  }

  return true;
}

bool CProjectInstrumentalSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // format specific children of instrumental

  if (element == "ascii") { // ASCII
    QString str;

    m_instrumental->ascii.detectorSize = atts.value("size").toInt();

    str = atts.value("format");
    if (str == "line")
      m_instrumental->ascii.format = PRJCT_INSTR_ASCII_FORMAT_LINE;
    else if (str == "column")
      m_instrumental->ascii.format = PRJCT_INSTR_ASCII_FORMAT_COLUMN;
    else
      return postErrorMessage("Invalid ascii format");

    m_instrumental->ascii.flagZenithAngle = (atts.value("zen") == "true") ? 1 : 0;
    m_instrumental->ascii.flagAzimuthAngle = (atts.value("azi") == "true") ? 1 : 0;
    m_instrumental->ascii.flagElevationAngle = (atts.value("ele") == "true") ? 1 : 0;
    m_instrumental->ascii.flagDate = (atts.value("date") == "true") ? 1 : 0;
    m_instrumental->ascii.flagTime = (atts.value("time") == "true") ? 1 : 0;
    m_instrumental->ascii.flagWavelength = (atts.value("lambda") == "true") ? 1 : 0;

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ascii.calibrationFile))
	strcpy(m_instrumental->ascii.calibrationFile, str.toAscii().data());
      else
	return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ascii.instrFunctionFile))
	strcpy(m_instrumental->ascii.instrFunctionFile, str.toAscii().data());
      else
	return postErrorMessage("Instrument Function  Filename too long");
    }

  }
  else if (element == "logger") { // LOGGER
    return helperLoadLogger(atts, &(m_instrumental->logger));

  }
  else if (element == "acton") { // ACTON
    QString str;

    str = atts.value("type");
    if (str == "old")
      m_instrumental->acton.niluType = PRJCT_INSTR_NILU_FORMAT_OLD;
    else if (str == "new")
      m_instrumental->acton.niluType = PRJCT_INSTR_NILU_FORMAT_NEW;
    else
      return postErrorMessage("Invalid acton Type");

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->acton.calibrationFile))
	strcpy(m_instrumental->acton.calibrationFile, str.toAscii().data());
      else
	return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->acton.instrFunctionFile))
	strcpy(m_instrumental->acton.instrFunctionFile, str.toAscii().data());
      else
	return postErrorMessage("Instrument Function Filename too long");
    }

  }
  else if (element == "pdaegg") { // PDA EGG
    return helperLoadLogger(atts, &(m_instrumental->pdaegg));

  }
  else if (element == "pdaeggold") { // PDA EGG OLD
    return helperLoadLogger(atts, &(m_instrumental->pdaeggold));

  }
  else if (element == "pdaeggulb") { // PDA EGG ULB
    QString str;

    str = atts.value("type");
    if (str == "manual")
      m_instrumental->pdaeggulb.curveType = PRJCT_INSTR_ULB_TYPE_MANUAL;
    else if (str == "high")
      m_instrumental->pdaeggulb.curveType = PRJCT_INSTR_ULB_TYPE_HIGH;
    else if (str == "low")
      m_instrumental->pdaeggulb.curveType = PRJCT_INSTR_ULB_TYPE_LOW;
    else
      return postErrorMessage("Invalid pdaeggulb Type");

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->pdaeggulb.calibrationFile))
	strcpy(m_instrumental->pdaeggulb.calibrationFile, str.toAscii().data());
      else
	return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->pdaeggulb.instrFunctionFile))
	strcpy(m_instrumental->pdaeggulb.instrFunctionFile, str.toAscii().data());
      else
	return postErrorMessage("Instrument Function Filename too long");
    }

    str = atts.value("ipv");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->pdaeggulb.interPixelVariabilityFile))
	strcpy(m_instrumental->pdaeggulb.interPixelVariabilityFile, str.toAscii().data());
      else
	return postErrorMessage("Inter Pixel Variability Filename too long");
    }

    str = atts.value("dnl");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->pdaeggulb.detectorNonLinearityFile))
	strcpy(m_instrumental->pdaeggulb.detectorNonLinearityFile, str.toAscii().data());
      else
	return postErrorMessage("Detector Non-Linearity Filename too long");
    }

  }
  else if (element == "ccdohp96") { // CCD OHP 96
    return helperLoadCcd(atts, &(m_instrumental->ccdohp96));

  }
  else if (element == "ccdha94") { // CCD HA 94
    return helperLoadCcd(atts, &(m_instrumental->ccdha94));

  }
  else if (element == "ccdulb") { // CCD ULB
    QString str;

    m_instrumental->ccdulb.grating = atts.value("grating").toInt();
    m_instrumental->ccdulb.centralWavelength = atts.value("cen").toInt();

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ccdulb.calibrationFile))
	strcpy(m_instrumental->ccdulb.calibrationFile, str.toAscii().data());
      else
	return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("offset");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ccdulb.offsetFile))
	strcpy(m_instrumental->ccdulb.offsetFile, str.toAscii().data());
      else
	return postErrorMessage("Offset Filename too long");
    }

    str = atts.value("ipv");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ccdulb.interPixelVariabilityFile))
	strcpy(m_instrumental->ccdulb.interPixelVariabilityFile, str.toAscii().data());
      else
	return postErrorMessage("Inter Pixel Variability Filename too long");
    }

    str = atts.value("dnl");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ccdulb.detectorNonLinearityFile))
	strcpy(m_instrumental->ccdulb.detectorNonLinearityFile, str.toAscii().data());
      else
	return postErrorMessage("Detector Non-Linearity Filename too long");
    }

  }
  else if (element == "saozvis") { // SAOZ VIS
    return helperLoadSaoz(atts, &(m_instrumental->saozvis));

  }
  else if (element == "saozuv") { // SAOZ UV
    return helperLoadSaoz(atts, &(m_instrumental->saozuv));

  }
  else if (element == "saozefm") { // SAOZ EFM
    return helperLoadMinimum(atts, &(m_instrumental->saozefm));

  }
  else if (element == "mfc") { // MFC
    QString str;

    m_instrumental->mfc.detectorSize = atts.value("size").toInt();
    m_instrumental->mfc.firstWavelength = atts.value("first").toInt();

    m_instrumental->mfc.revert = (atts.value("revert") == "true") ? 1 : 0;
    m_instrumental->mfc.autoFileSelect = (atts.value("auto") == "true") ? 1 : 0;
    m_instrumental->mfc.offsetMask = atts.value("omask").toUInt();
    m_instrumental->mfc.instrFctnMask = atts.value("imask").toUInt();
    m_instrumental->mfc.darkCurrentMask = atts.value("dmask").toUInt();
    m_instrumental->mfc.spectraMask = atts.value("smask").toUInt();

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfc.calibrationFile))
	strcpy(m_instrumental->mfc.calibrationFile, str.toAscii().data());
      else
	return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfc.instrFunctionFile))
	strcpy(m_instrumental->mfc.instrFunctionFile, str.toAscii().data());
      else
	return postErrorMessage("Instrument Function Filename too long");
    }
    str = atts.value("dark");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfc.darkCurrentFile))
	strcpy(m_instrumental->mfc.darkCurrentFile, str.toAscii().data());
      else
	return postErrorMessage("Dark Current Filename too long");
    }

    str = atts.value("offset");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfc.offsetFile))
	strcpy(m_instrumental->mfc.offsetFile, str.toAscii().data());
      else
	return postErrorMessage("Offset Filename too long");
    }

  }
  else if (element == "mfcstd") { // MFC STD
    QString str;

    m_instrumental->mfcstd.detectorSize = atts.value("size").toInt();
    m_instrumental->mfcstd.revert = (atts.value("revert") == "true") ? 1 : 0;

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfcstd.calibrationFile))
	strcpy(m_instrumental->mfcstd.calibrationFile, str.toAscii().data());
      else
	return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfcstd.instrFunctionFile))
	strcpy(m_instrumental->mfcstd.instrFunctionFile, str.toAscii().data());
      else
	return postErrorMessage("Instrument Function Filename too long");
    }
    str = atts.value("dark");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfcstd.darkCurrentFile))
	strcpy(m_instrumental->mfcstd.darkCurrentFile, str.toAscii().data());
      else
	return postErrorMessage("Dark Current Filename too long");
    }

    str = atts.value("offset");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->mfcstd.offsetFile))
	strcpy(m_instrumental->mfcstd.offsetFile, str.toAscii().data());
      else
	return postErrorMessage("Offset Filename too long");
    }

  }
  else if (element == "rasas") { // RASAS
    return helperLoadMinimum(atts, &(m_instrumental->rasas));

  }
  else if (element == "pdasieasoe") { // PDASI EASOE
    return helperLoadMinimum(atts, &(m_instrumental->pdasieasoe));

  }
  else if (element == "pdasiosma") { // PDASI OSMA
    return helperLoadLogger(atts, &(m_instrumental->pdasiosma));

  }
  else if (element == "ccdeev") { // CCD EEV
    QString str;

    m_instrumental->ccdeev.detectorSize = atts.value("size").toInt();

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ccdeev.calibrationFile))
	strcpy(m_instrumental->ccdeev.calibrationFile, str.toAscii().data());
      else
	return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ccdeev.instrFunctionFile))
	strcpy(m_instrumental->ccdeev.instrFunctionFile, str.toAscii().data());
      else
	return postErrorMessage("Instrument Function Filename too long");
    }

    str = atts.value("stray");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ccdeev.straylightCorrectionFile))
	strcpy(m_instrumental->ccdeev.straylightCorrectionFile, str.toAscii().data());
      else
	return postErrorMessage("Stray Light Correction Filename too long");
    }

    str = atts.value("dnl");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->ccdeev.detectorNonLinearityFile))
	strcpy(m_instrumental->ccdeev.detectorNonLinearityFile, str.toAscii().data());
      else
	return postErrorMessage("Detector Non-Linearity Filename too long");
    }

  }
  else if (element == "opus") { // OPUS
    QString str;

    m_instrumental->opus.detectorSize = atts.value("size").toInt();
    m_instrumental->opus.timeShift = atts.value("time").toDouble();
    m_instrumental->opus.flagTransmittance = (atts.value("trans") == "true") ? 1 : 0;

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->opus.calibrationFile))
	strcpy(m_instrumental->opus.calibrationFile, str.toAscii().data());
      else
	return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->opus.instrFunctionFile))
	strcpy(m_instrumental->opus.instrFunctionFile, str.toAscii().data());
      else
	return postErrorMessage("Instrument Function Filename too long");
    }

  }
  else if (element == "gdpascii") { // GDP ASCII
    helperLoadGdp(atts, &(m_instrumental->gdpascii));

  }
  else if (element == "gdpbin") { // GDP BIN
    helperLoadGdp(atts, &(m_instrumental->gdpbin));

  }
  else if (element == "sciahdf") { // SCIA HDF
    helperLoadScia(atts, &(m_instrumental->sciahdf));

  }
  else if (element == "sciapds") { // SCIA PDS
    helperLoadScia(atts, &(m_instrumental->sciapds));

  }
  else if (element == "uoft") { // UOFT
    return helperLoadMinimum(atts, &(m_instrumental->uoft));

  }
  else if (element == "noaa") { // NOAA
    return helperLoadMinimum(atts, &(m_instrumental->noaa));

  }
  else if (element == "omi") { // OMI
    QString str;

    str = atts.value("type");
    if (str == "uv1")
      m_instrumental->omi.spectralType = PRJCT_INSTR_OMI_TYPE_UV1;
    else if (str == "uv2")
      m_instrumental->omi.spectralType = PRJCT_INSTR_OMI_TYPE_UV2;
    else if (str == "vis")
      m_instrumental->omi.spectralType = PRJCT_INSTR_OMI_TYPE_VIS;
    else
      return postErrorMessage("Invalid omi Spectral Type");

    m_instrumental->omi.minimumWavelength = atts.value("min").toDouble();
    m_instrumental->omi.maximumWavelength = atts.value("max").toDouble();
    m_instrumental->omi.flagAverage = (atts.value("ave") == "true") ? 1 : 0;

    str = atts.value("calib");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->omi.calibrationFile))
	strcpy(m_instrumental->omi.calibrationFile, str.toAscii().data());
      else
	return postErrorMessage("Calibration Filename too long");
    }

    str = atts.value("instr");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_instrumental->omi.instrFunctionFile))
	strcpy(m_instrumental->omi.instrFunctionFile, str.toAscii().data());
      else
	return postErrorMessage("Instrument Function Filename too long");
    }

  }

  // ... other formats ...

  return true;
}

bool CProjectInstrumentalSubHandler::helperLoadLogger(const QXmlAttributes &atts, struct instrumental_logger *d)
{
  QString str;

  // spectral type
  str = atts.value("type");
  if (str == "all")
    d->spectralType = PRJCT_INSTR_IASB_TYPE_ALL;
  else if (str == "zenithal")
    d->spectralType = PRJCT_INSTR_IASB_TYPE_ZENITHAL;
  else if (str == "off-axis")
    d->spectralType = PRJCT_INSTR_IASB_TYPE_OFFAXIS;
  else
    return postErrorMessage("Invalid spectral Type");

  d->flagAzimuthAngle = (atts.value("azi") == "true") ? 1 : 0;

  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.toAscii().data());
    else
      return postErrorMessage("Calibration Filename too long");
  }

  str = atts.value("instr");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->instrFunctionFile))
      strcpy(d->instrFunctionFile, str.toAscii().data());
    else
      return postErrorMessage("Instrument Function Filename too long");
  }

  return true;
}

bool CProjectInstrumentalSubHandler::helperLoadSaoz(const QXmlAttributes &atts, struct instrumental_saoz *d)
{
  QString str;

  // spectral type
  str = atts.value("type");
  if (str == "zenithal")
    d->spectralType = PRJCT_INSTR_SAOZ_TYPE_ZENITHAL;
  else if (str == "pointed")
    d->spectralType = PRJCT_INSTR_SAOZ_TYPE_POINTED;
  else
    return postErrorMessage("Invalid spectral Type");

  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.toAscii().data());
    else
      return postErrorMessage("Calibration Filename too long");
  }

  str = atts.value("instr");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->instrFunctionFile))
      strcpy(d->instrFunctionFile, str.toAscii().data());
    else
      return postErrorMessage("Instrument Function Filename too long");
  }

  return true;
}

bool CProjectInstrumentalSubHandler::helperLoadMinimum(const QXmlAttributes &atts, struct instrumental_minimum *d)
{
  QString str;

  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.toAscii().data());
    else
      return postErrorMessage("Calibration Filename too long");
  }

  str = atts.value("instr");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->instrFunctionFile))
      strcpy(d->instrFunctionFile, str.toAscii().data());
    else
      return postErrorMessage("Instrument Function Filename too long");
  }

  return true;
}

bool CProjectInstrumentalSubHandler::helperLoadCcd(const QXmlAttributes &atts, struct instrumental_ccd *d)
{
  QString str;

  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.toAscii().data());
    else
      return postErrorMessage("Calibration Filename too long");
  }

  str = atts.value("instr");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->instrFunctionFile))
      strcpy(d->instrFunctionFile, str.toAscii().data());
    else
      return postErrorMessage("Instrument Function Filename too long");
  }

  str = atts.value("ipv");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->interPixelVariabilityFile))
      strcpy(d->interPixelVariabilityFile, str.toAscii().data());
    else
      return postErrorMessage("Inter Pixel Variability Filename too long");
  }

  str = atts.value("dnl");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->detectorNonLinearityFile))
      strcpy(d->detectorNonLinearityFile, str.toAscii().data());
    else
      return postErrorMessage("Detector Non-Linearity Filename too long");
  }

  return true;
}

bool CProjectInstrumentalSubHandler::helperLoadGdp(const QXmlAttributes &atts, struct instrumental_gdp *d)
{
  QString str;

  str = atts.value("type");
  if (str == "1a")
    d->bandType = PRJCT_INSTR_GDP_BAND_1A;
  else if (str == "1b")
    d->bandType = PRJCT_INSTR_GDP_BAND_1B;
  else if (str == "2a")
    d->bandType = PRJCT_INSTR_GDP_BAND_2A;
  else if (str == "2b")
    d->bandType = PRJCT_INSTR_GDP_BAND_2B;
  else if (str == "3")
    d->bandType = PRJCT_INSTR_GDP_BAND_3;
  else if (str == "3")
    d->bandType = PRJCT_INSTR_GDP_BAND_4;
  else
    return postErrorMessage("Invalid gdp band");

  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.toAscii().data());
    else
      return postErrorMessage("Calibration Filename too long");
  }

  str = atts.value("instr");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->instrFunctionFile))
      strcpy(d->instrFunctionFile, str.toAscii().data());
    else
      return postErrorMessage("Instrument Function Filename too long");
  }

  return true;
}

bool CProjectInstrumentalSubHandler::helperLoadScia(const QXmlAttributes &atts, struct instrumental_scia *d)
{
  QString str;

  str = atts.value("channel");
  if (str == "1")
    d->channel = PRJCT_INSTR_SCIA_CHANNEL_1;
  else if (str == "2")
    d->channel = PRJCT_INSTR_SCIA_CHANNEL_2;
  else if (str == "3")
    d->channel = PRJCT_INSTR_SCIA_CHANNEL_3;
  else if (str == "4")
    d->channel = PRJCT_INSTR_SCIA_CHANNEL_4;
  else
    return postErrorMessage("Invalid scia channel");

  // clusters
  memset(d->clusters, 0, sizeof(d->clusters)); // zero
  d->clusters[2]  = (atts.value("c2")  == "true") ? 1 : 0;
  d->clusters[3]  = (atts.value("c3")  == "true") ? 1 : 0;
  d->clusters[4]  = (atts.value("c4")  == "true") ? 1 : 0;
  d->clusters[5]  = (atts.value("c5")  == "true") ? 1 : 0;

  d->clusters[8]  = (atts.value("c8")  == "true") ? 1 : 0;
  d->clusters[9]  = (atts.value("c9")  == "true") ? 1 : 0;
  d->clusters[10] = (atts.value("c10") == "true") ? 1 : 0;

  d->clusters[13] = (atts.value("c13") == "true") ? 1 : 0;
  d->clusters[14] = (atts.value("c14") == "true") ? 1 : 0;
  d->clusters[15] = (atts.value("c15") == "true") ? 1 : 0;
  d->clusters[16] = (atts.value("c16") == "true") ? 1 : 0;
  d->clusters[17] = (atts.value("c17") == "true") ? 1 : 0;
  d->clusters[18] = (atts.value("c18") == "true") ? 1 : 0;

  d->clusters[22] = (atts.value("c22") == "true") ? 1 : 0;
  d->clusters[23] = (atts.value("c23") == "true") ? 1 : 0;
  d->clusters[24] = (atts.value("c24") == "true") ? 1 : 0;
  d->clusters[25] = (atts.value("c25") == "true") ? 1 : 0;
  d->clusters[26] = (atts.value("c26") == "true") ? 1 : 0;
  d->clusters[27] = (atts.value("c27") == "true") ? 1 : 0;

  str = atts.value("sunref");
  if (!str.isEmpty()) {
    if (str.length() < (int)sizeof(d->sunReference))
      strcpy(d->sunReference, str.toAscii().data());
    else
      return postErrorMessage("Sun Reference too long");
  }

  str = atts.value("calib");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->calibrationFile))
      strcpy(d->calibrationFile, str.toAscii().data());
    else
      return postErrorMessage("Calibration Filename too long");
  }

  str = atts.value("instr");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(d->instrFunctionFile))
      strcpy(d->instrFunctionFile, str.toAscii().data());
    else
      return postErrorMessage("Instrument Function Filename too long");
  }

  return true;
}

//------------------------------------------------------------------------
// handler for <slit> (child of project)

CProjectSlitSubHandler::CProjectSlitSubHandler(CQdoasProjectConfigHandler *master,
					       mediate_project_slit_t *slit) :
  CConfigSubHandler(master),
  m_slit(slit)
{
}

CProjectSlitSubHandler::~CProjectSlitSubHandler()
{
}

bool CProjectSlitSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  str = atts.value("ref");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_slit->solarRefFile))
      strcpy(m_slit->solarRefFile, str.toAscii().data());
    else
      return postErrorMessage("Solar Reference Filename too long");
  }

  str = atts.value("type");
  if (str == "file")
    m_slit->slitType = SLIT_TYPE_FILE;
  else if (str == "gaussian")
    m_slit->slitType = SLIT_TYPE_GAUSS;
  else if (str == "lorentz")
    m_slit->slitType = SLIT_TYPE_INVPOLY;
  else if (str == "voigt")
    m_slit->slitType = SLIT_TYPE_VOIGT;
  else if (str == "error")
    m_slit->slitType = SLIT_TYPE_ERF;
  else if (str == "boxcarapod")
    m_slit->slitType = SLIT_TYPE_APOD;
  else if (str == "nbsapod")
    m_slit->slitType = SLIT_TYPE_APODNBS;
  else if (str == "gaussianfile")
    m_slit->slitType = SLIT_TYPE_GAUSS_FILE;
  else if (str == "lorentzfile")
    m_slit->slitType = SLIT_TYPE_INVPOLY_FILE;
  else if (str == "errorfile")
    m_slit->slitType = SLIT_TYPE_ERF_FILE;
  else if (str == "gaussiantempfile")
    m_slit->slitType = SLIT_TYPE_GAUSS_T_FILE;
  else if (str == "errortempfile")
    m_slit->slitType = SLIT_TYPE_ERF_T_FILE;
  else
    return postErrorMessage("Invalid slit type");

  m_slit->applyFwhmCorrection = (atts.value("fwhmcor") == "true") ? 1 : 0;

  return true;
}

bool CProjectSlitSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "file") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_slit->file.filename))
	strcpy(m_slit->file.filename, str.toAscii().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }
  }
  else if (element == "gaussian") {

    m_slit->gaussian.fwhm = atts.value("fwhm").toDouble();
  }
  else if (element == "lorentz") {

    m_slit->lorentz.width = atts.value("width").toDouble();
    m_slit->lorentz.degree = atts.value("degree").toInt();
  }
  else if (element == "voigt") {

    m_slit->voigt.fwhmL = atts.value("fwhmleft").toDouble();
    m_slit->voigt.fwhmR = atts.value("fwhmright").toDouble();
    m_slit->voigt.glRatioL = atts.value("glrleft").toDouble();
    m_slit->voigt.fwhmL = atts.value("glrright").toDouble();
  }
  else if (element == "error") {

    m_slit->error.fwhm = atts.value("fwhm").toDouble();
    m_slit->error.width = atts.value("width").toDouble();
  }
  else if (element == "boxcarapod") {

    m_slit->boxcarapod.resolution = atts.value("resolution").toDouble();
    m_slit->boxcarapod.phase = atts.value("phase").toDouble();
  }
  else if (element == "nbsapod") {

    m_slit->nbsapod.resolution = atts.value("resolution").toDouble();
    m_slit->nbsapod.phase = atts.value("phase").toDouble();
  }
  else if (element == "gaussianfile") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_slit->gaussianfile.filename))
	strcpy(m_slit->gaussianfile.filename, str.toAscii().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }
  }
  else if (element == "lorentzfile") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_slit->lorentzfile.filename))
	strcpy(m_slit->lorentzfile.filename, str.toAscii().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }

    m_slit->lorentzfile.degree = atts.value("degree").toInt();
  }
  else if (element == "errorfile") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_slit->errorfile.filename))
	strcpy(m_slit->errorfile.filename, str.toAscii().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }

    m_slit->errorfile.width = atts.value("width").toDouble();
  }
  else if (element == "gaussiantempfile") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_slit->gaussiantempfile.filename))
	strcpy(m_slit->gaussiantempfile.filename, str.toAscii().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }
  }
  else if (element == "errortempfile") {

    QString str = atts.value("file");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(m_slit->errortempfile.filename))
	strcpy(m_slit->errortempfile.filename, str.toAscii().data());
      else
	return postErrorMessage("Slit Function Filename too long");
    }

    m_slit->errortempfile.width = atts.value("width").toDouble();
  }

  return true;
}

//------------------------------------------------------------------------
// handler for <output> (child of project)

CProjectOutputSubHandler::CProjectOutputSubHandler(CQdoasProjectConfigHandler *master,
						   mediate_project_output_t *output) :
  CConfigSubHandler(master),
  m_output(output)
{
}

CProjectOutputSubHandler::~CProjectOutputSubHandler()
{
}

bool CProjectOutputSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  str = atts.value("path");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_output->path))
      strcpy(m_output->path, str.toAscii().data());
    else
      return postErrorMessage("Output path too long");
  }

  m_output->analysisFlag = (atts.value("anlys") == "true") ? 1 : 0;
  m_output->calibrationFlag = (atts.value("calib") == "true") ? 1 : 0;
  m_output->configurationFlag = (atts.value("conf") == "true") ? 1 : 0;
  m_output->binaryFormatFlag = (atts.value("bin") == "true") ? 1 : 0;
  m_output->directoryFlag = (atts.value("dirs") == "true") ? 1 : 0;

  str = atts.value("flux");
  if (!str.isEmpty()) {
    if (str.length() < (int)sizeof(m_output->flux))
      strcpy(m_output->flux, str.toAscii().data());
    else
      return postErrorMessage("Output flux too long");
  }

  str = atts.value("cic");
  if (!str.isEmpty()) {
    if (str.length() < (int)sizeof(m_output->colourIndex))
      strcpy(m_output->colourIndex, str.toAscii().data());
    else
      return postErrorMessage("Output colour index too long");
  }

  return true;
}

bool CProjectOutputSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element != "field")
    return postErrorMessage("Invalid XML element");

  if (m_output->nSelected >= PRJCT_RESULTS_ASCII_MAX)
    return postErrorMessage("Too many output fields");

  QString str = atts.value("name");
  if (str == "specno")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_SPECNO;
  else if (str == "name")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_NAME;           
  else if (str == "date_time")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_DATE_TIME;      
  else if (str == "date")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_DATE;           
  else if (str == "time")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_TIME;           
  else if (str == "year")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_YEAR;           
  else if (str == "julian")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_JULIAN;         
  else if (str == "jdfrac")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_JDFRAC;         
  else if (str == "tifrac")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_TIFRAC;         
  else if (str == "scans")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_SCANS;          
  else if (str == "tint")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_TINT;           
  else if (str == "sza")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_SZA;            
  else if (str == "chi")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_CHI;            
  else if (str == "rms")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_RMS;            
  else if (str == "azim")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_AZIM;           
  else if (str == "tdet")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_TDET;           
  else if (str == "sky")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_SKY;            
  else if (str == "bestshift")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_BESTSHIFT;      
  else if (str == "refzm")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_REFZM;          
  else if (str == "refshift")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_REFSHIFT;       
  else if (str == "pixel")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_PIXEL;          
  else if (str == "pixel_type")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_PIXEL_TYPE;     
  else if (str == "orbit")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_ORBIT;          
  else if (str == "longit")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_LONGIT;         
  else if (str == "latit")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_LATIT;          
  else if (str == "altit")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_ALTIT;          
  else if (str == "covar")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_COVAR;          
  else if (str == "corr")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_CORR;           
  else if (str == "cloud")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_CLOUD;          
  else if (str == "coeff")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_COEFF;          
  else if (str == "o3")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_O3;             
  else if (str == "no2")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_NO2;            
  else if (str == "cloudtopp")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_CLOUDTOPP;      
  else if (str == "los_za")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_LOS_ZA;         
  else if (str == "los_azimuth")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_LOS_AZIMUTH;    
  else if (str == "sat_height")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_SAT_HEIGHT;     
  else if (str == "earth_radius")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_EARTH_RADIUS;   
  else if (str == "view_elevation")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_VIEW_ELEVATION; 
  else if (str == "view_azimuth")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_VIEW_AZIMUTH;   
  else if (str == "scia_quality")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_SCIA_QUALITY;   
  else if (str == "scia_state_index")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_SCIA_STATE_INDEX;
  else if (str == "scia_state_id")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_SCIA_STATE_ID;  
  else if (str == "mfc_starttime")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_MFC_STARTTIME;  
  else if (str == "mfc_endtime")
    m_output->selected[m_output->nSelected] = PRJCT_RESULTS_ASCII_MFC_ENDTIME;    
  else
    return postErrorMessage("Invalid output field " + str);

  // MUST be ok ...
  ++(m_output->nSelected);

  return true;
}

//------------------------------------------------------------------------
// handler for <nasa_ames> (child of project)

CProjectNasaAmesSubHandler::CProjectNasaAmesSubHandler(CQdoasProjectConfigHandler *master,
						       mediate_project_nasa_ames_t *nasaames) :
  CConfigSubHandler(master),
  m_nasaames(nasaames)
{
}

CProjectNasaAmesSubHandler::~CProjectNasaAmesSubHandler()
{
}

bool CProjectNasaAmesSubHandler::start(const QXmlAttributes &atts)
{
  QString str;

  str = atts.value("path");
  if (!str.isEmpty()) {
    str = m_master->pathExpand(str);
    if (str.length() < (int)sizeof(m_nasaames->path))
      strcpy(m_nasaames->path, str.toAscii().data());
    else
      return postErrorMessage("NasaAmes path too long");
  }

  m_nasaames->saveFlag = (atts.value("save") == "true") ? 1 : 0;
  m_nasaames->rejectTestFlag = (atts.value("reject") == "true") ? 1 : 0;

  str = atts.value("instr");
  if (!str.isEmpty()) {
    if (str.length() < (int)sizeof(m_nasaames->instrument))
      strcpy(m_nasaames->instrument, str.toAscii().data());
    else
      return postErrorMessage("NASA-AMES instrument too long");
  }

  str = atts.value("exp");
  if (!str.isEmpty()) {
    if (str.length() < (int)sizeof(m_nasaames->experiment))
      strcpy(m_nasaames->experiment, str.toAscii().data());
    else
      return postErrorMessage("NASA-AMES experiment too long");
  }

  str = atts.value("no2");
  if (!str.isEmpty()) {
    if (str.length() < (int)sizeof(m_nasaames->anlysWinNO2))
      strcpy(m_nasaames->anlysWinNO2, str.toAscii().data());
    else
      return postErrorMessage("NASA-AMES NO2 window name too long");
  }

  str = atts.value("o3");
  if (!str.isEmpty()) {
    if (str.length() < (int)sizeof(m_nasaames->anlysWinO3))
      strcpy(m_nasaames->anlysWinO3, str.toAscii().data());
    else
      return postErrorMessage("NASA-AMES O3 window name too long");
  }

  str = atts.value("bro");
  if (!str.isEmpty()) {
    if (str.length() < (int)sizeof(m_nasaames->anlysWinBrO))
      strcpy(m_nasaames->anlysWinBrO, str.toAscii().data());
    else
      return postErrorMessage("NASA-AMES BrO window name too long");
  }

  str = atts.value("oclo");
  if (!str.isEmpty()) {
    if (str.length() < (int)sizeof(m_nasaames->anlysWinOClO))
      strcpy(m_nasaames->anlysWinOClO, str.toAscii().data());
    else
      return postErrorMessage("NASA-AMES OClO window name too long");
  }

  return true;
}

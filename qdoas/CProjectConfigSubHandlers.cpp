
#include "CProjectConfigSubHandlers.h"
#include "CProjectConfigAnalysisWindowSubHandlers.h"
#include "CConfigSubHandlerUtils.h"
#include "CProjectConfigTreeNode.h"

#include "constants.h"

#include "debugutil.h"

//------------------------------------------------------------------------

CSelectorSubHandler::CSelectorSubHandler(CConfigHandler *master, data_select_list_t *selectList) :
  CBasicConfigSubHandler(master),
  m_selectList(selectList)
{
}

CSelectorSubHandler::~CSelectorSubHandler()
{
}

bool CSelectorSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element != "field")
    return postErrorMessage("Invalid XML element");

  data_select_list_t *d = m_selectList;

  if (d->nSelected >= PRJCT_RESULTS_ASCII_MAX)
    return postErrorMessage("Too many output fields");

  QString str = atts.value("name");
  if (str == "specno")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_SPECNO;
  else if (str == "name")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_NAME;
  else if (str == "date_time")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_DATE_TIME;
  else if (str == "date")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_DATE;
  else if (str == "time")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_TIME;
  else if (str == "year")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_YEAR;
  else if (str == "julian")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_JULIAN;
  else if (str == "jdfrac")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_JDFRAC;
  else if (str == "tifrac")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_TIFRAC;
  else if (str == "scans")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_SCANS;
  else if (str == "rejected")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_NREJ;
  else if (str == "tint")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_TINT;
  else if (str == "sza")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_SZA;
  else if (str == "chi")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_CHI;
  else if (str == "rms")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_RMS;
  else if (str == "azim")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_AZIM;
  else if (str == "tdet")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_TDET;
  else if (str == "sky")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_SKY;
  else if (str == "bestshift")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_BESTSHIFT;
  else if (str == "refzm")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_REFZM;
  else if (str == "refshift")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_REFSHIFT;
  else if (str == "pixel")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_PIXEL;
  else if (str == "pixel_type")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_PIXEL_TYPE;
  else if (str == "orbit")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_ORBIT;
  else if (str == "longit")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_LONGIT;
  else if (str == "latit")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_LATIT;
  else if (str == "altit")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_ALTIT;
  else if (str == "covar")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_COVAR;
  else if (str == "corr")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_CORR;
  else if (str == "cloud")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_CLOUD;
  else if (str == "coeff")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_COEFF;
  else if (str == "o3")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_O3;
  else if (str == "no2")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_NO2;
  else if (str == "cloudtopp")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_CLOUDTOPP;
  else if (str == "los_za")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_LOS_ZA;
  else if (str == "los_azimuth")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_LOS_AZIMUTH;
  else if (str == "sat_height")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_SAT_HEIGHT;
  else if (str == "earth_radius")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_EARTH_RADIUS;
  else if (str == "view_elevation")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_VIEW_ELEVATION;
  else if (str == "view_azimuth")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_VIEW_AZIMUTH;
  else if (str == "scia_quality")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_SCIA_QUALITY;
  else if (str == "scia_state_index")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_SCIA_STATE_INDEX;
  else if (str == "scia_state_id")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_SCIA_STATE_ID;
  else if (str == "mfc_starttime")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_STARTTIME;
  else if (str == "mfc_endtime")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_ENDTIME;

  else if (str == "scanning_angle")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_SCANNING;
  else if (str == "ccd_filterNumber")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_CCD_FILTERNUMBER;
  else if (str == "ccd_measType")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_CCD_MEASTYPE;
  else if (str == "ccd_headTemp")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_CCD_HEADTEMPERATURE;
  else if (str == "cooler_status")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_COOLING_STATUS;
  else if (str == "mirror_status")
    d->selected[d->nSelected] = PRJCT_RESULTS_ASCII_MIRROR_ERROR;

  else
    return postErrorMessage("Invalid output field " + str);

  // MUST be ok ...
  ++(d->nSelected);

  return true;
}


//------------------------------------------------------------------------
// handler for <display> (child of project)

CProjectDisplaySubHandler::CProjectDisplaySubHandler(CConfigHandler *master, mediate_project_display_t *display) :
  CSelectorSubHandler(master, &(display->selection)),
  m_display(display)
{
}

CProjectDisplaySubHandler::~CProjectDisplaySubHandler()
{
}

bool CProjectDisplaySubHandler::start(const QXmlAttributes &atts)
{
  m_display->requireSpectra = (atts.value("spectra") == "true") ? 1 : 0;
  m_display->requireData = (atts.value("data") == "true") ? 1 : 0;
  m_display->requireFits = (atts.value("fits") == "true") ? 1 : 0;

  return true;
}

//------------------------------------------------------------------------
// handler for <selection> (child of project)

CProjectSelectionSubHandler::CProjectSelectionSubHandler(CConfigHandler *master, mediate_project_selection_t *selection) :
  CBasicConfigSubHandler(master),
  m_selection(selection)
{
}

CProjectSelectionSubHandler::~CProjectSelectionSubHandler()
{
}

bool CProjectSelectionSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "sza") {

    // defaults from mediateInitializeProject() are ok if attributes are not present
    m_selection->szaMinimum = atts.value("min").toDouble();
    m_selection->szaMaximum = atts.value("max").toDouble();
    m_selection->szaDelta = atts.value("delta").toDouble();

  }
  else if (element == "record") {

    // defaults from mediateInitializeProject() are ok if attributes are not present
    m_selection->recordNumberMinimum = atts.value("min").toInt();
    m_selection->recordNumberMaximum = atts.value("max").toInt();
  }
  else if (element == "circle") {

    m_selection->geo.circle.radius = atts.value("radius").toDouble();
    m_selection->geo.circle.centerLongitude = atts.value("long").toDouble();
    m_selection->geo.circle.centerLatitude = atts.value("lat").toDouble();
  }
  else if (element == "rectangle") {

    m_selection->geo.rectangle.easternLongitude = atts.value("east").toDouble();
    m_selection->geo.rectangle.westernLongitude = atts.value("west").toDouble();
    m_selection->geo.rectangle.northernLatitude = atts.value("north").toDouble();
    m_selection->geo.rectangle.southernLatitude = atts.value("south").toDouble();
  }
  else if (element == "sites") {

    m_selection->geo.sites.radius = atts.value("radius").toDouble();
  }
  else if (element == "geolocation") {

    QString selected = atts.value("selected");

    if (selected == "circle")
      m_selection->geo.mode = PRJCT_SPECTRA_MODES_CIRCLE;
    else if (selected == "rectangle")
      m_selection->geo.mode = PRJCT_SPECTRA_MODES_RECTANGLE;
    else if (selected == "sites")
      m_selection->geo.mode = PRJCT_SPECTRA_MODES_OBSLIST;
    else
      m_selection->geo.mode = PRJCT_SPECTRA_MODES_NONE; // default and "none"
  }

  return true;
}


//------------------------------------------------------------------------
// handler for <analysis> (child of project)

CProjectAnalysisSubHandler::CProjectAnalysisSubHandler(CConfigHandler *master,
						       mediate_project_analysis_t *analysis) :
  CBasicConfigSubHandler(master),
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
    m_analysis->fitType = PRJCT_ANLYS_FIT_WEIGHTING_NONE;
  else if (str == "instr")
    m_analysis->fitType = PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL;
  else
    return postErrorMessage("Invalid analysis fit");

  str = atts.value("unit");
  if (str == "pixel")
    m_analysis->unitType = PRJCT_ANLYS_UNITS_PIXELS;
  else if (str == "nm")
    m_analysis->unitType = PRJCT_ANLYS_UNITS_NANOMETERS;
  else
    return postErrorMessage("Invalid analysis unit");

  str = atts.value("interpolation");
  if (str == "linear")
    m_analysis->interpolationType = PRJCT_ANLYS_INTERPOL_LINEAR;
  else if (str == "spline")
    m_analysis->interpolationType = PRJCT_ANLYS_INTERPOL_SPLINE;
  else
    return postErrorMessage("Invalid analysis interpolation");

  m_analysis->interpolationSecurityGap = atts.value("gap").toInt();
  m_analysis->convergenceCriterion = atts.value("converge").toDouble();

  return true;
}


//------------------------------------------------------------------------
// handler for <raw_spectra> (child of project)

CProjectRawSpectraSubHandler::CProjectRawSpectraSubHandler(CConfigHandler *master,
							   CProjectConfigTreeNode *node) :
  CBasicConfigSubHandler(master),
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
// handler for <calibration> (child of project)

CProjectCalibrationSubHandler::CProjectCalibrationSubHandler(CConfigHandler *master,
							     mediate_project_calibration_t *calibration) :
  CBasicConfigSubHandler(master),
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
    else if (str == "gauss")
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

CProjectUndersamplingSubHandler::CProjectUndersamplingSubHandler(CConfigHandler *master,
								 mediate_project_undersampling_t *undersampling) :
  CBasicConfigSubHandler(master),
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

CProjectInstrumentalSubHandler::CProjectInstrumentalSubHandler(CConfigHandler *master,
							       mediate_project_instrumental_t *instrumental) :
  CBasicConfigSubHandler(master),
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
  else if (str == "ccdohp_96")
    m_instrumental->format = PRJCT_INSTR_FORMAT_CCD_OHP_96;
  else if (str == "ccdha_94")
    m_instrumental->format = PRJCT_INSTR_FORMAT_CCD_HA_94;
  else if (str == "saozvis")
    m_instrumental->format = PRJCT_INSTR_FORMAT_SAOZ_VIS;
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
  else if (str == "ccdeev")
    m_instrumental->format = PRJCT_INSTR_FORMAT_CCD_EEV;
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
  else if (str == "mkzypack")
    m_instrumental->format = PRJCT_INSTR_FORMAT_MKZYPACK;
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
  else if (element == "ccdohp96") { // CCD OHP 96
    return helperLoadCcd(atts, &(m_instrumental->ccdohp96));

  }
  else if (element == "ccdha94") { // CCD HA 94
    return helperLoadCcd(atts, &(m_instrumental->ccdha94));

  }
  else if (element == "saozvis") { // SAOZ VIS
    return helperLoadSaoz(atts, &(m_instrumental->saozvis));

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
    m_instrumental->mfcstd.straylight = (atts.value("straylight") == "true") ? 1 : 0;

    str = atts.value("date");
    if (!str.isEmpty()) {
      if (str.length() < (int)sizeof(m_instrumental->mfcstd.dateFormat))
        strcpy(m_instrumental->mfcstd.dateFormat, str.toAscii().data());
      else
        return postErrorMessage("Date format too long");
     }

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
  else if (element == "gome2") { // GOME2
    helperLoadGdp(atts, &(m_instrumental->gome2));
  }
  else if (element == "mkzypack") { // MKZY Pack
    return helperLoadMinimum(atts, &(m_instrumental->mkzypack));
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

CProjectSlitSubHandler::CProjectSlitSubHandler(CConfigHandler *master,
					       mediate_project_slit_t *slit) :
  CBasicConfigSubHandler(master),
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

  m_slit->applyFwhmCorrection = (atts.value("fwhmcor") == "true") ? 1 : 0;

  return true;
}

bool CProjectSlitSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  if (element == "slit_func") {

    return m_master->installSubHandler(new CSlitFunctionSubHandler(m_master, &(m_slit->function)), atts);
  }

  return true;
}

//------------------------------------------------------------------------
// handler for <output> (child of project)

CProjectOutputSubHandler::CProjectOutputSubHandler(CConfigHandler *master,
						   mediate_project_output_t *output) :
  CSelectorSubHandler(master, &(output->selection)),
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
  m_output->filenameFlag = (atts.value("file") == "true") ? 1 : 0;

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


//------------------------------------------------------------------------
// handler for <nasa_ames> (child of project)

CProjectNasaAmesSubHandler::CProjectNasaAmesSubHandler(CConfigHandler *master,
						       mediate_project_nasa_ames_t *nasaames) :
  CBasicConfigSubHandler(master),
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

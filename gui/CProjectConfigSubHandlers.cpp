
#include "CProjectConfigSubHandlers.h"

#include "debugutil.h"

//------------------------------------------------------------------------
//
// Handler for <project> element


CProjectSubHandler::CProjectSubHandler(CQdoasProjectConfigHandler *master) :
  CConfigSubHandler(master)
{
  m_project = new mediate_project_t;

  initializeMediateProject(m_project);
}

CProjectSubHandler::~CProjectSubHandler()
{
  delete m_project;
}

bool CProjectSubHandler::start(const QXmlAttributes &atts)
{
  // the project element

  m_name = atts. value("name");

  return !m_name.isEmpty();
}

bool CProjectSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  // a sub element of project ... create a specialized handler and delegate

  TRACE2("proj Handler : " << element.toStdString());

  if (element == "spectra") {
    return m_master->installSubHandler(new CProjectSpectraSubHandler(m_master, &(m_project->spectra)), atts);
  }
  
  return true;
}

bool CProjectSubHandler::end(const QString &element)
{
  // end of sub element ... all sub elements are managed by sub handlers.
  return true;
}

bool CProjectSubHandler::end()
{
  // end of project ... hand project data over to the master handler
  
  // ... collect all the bits and make a single object will ALL project content
  // that is properties, analysis windows, raw spectra trees...

  // TODO
  return true;
}

//------------------------------------------------------------------------

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

    m_spectra->requireSpectra = (atts.value("spectra") == "true") ? 1 : 0;
    m_spectra->requireData = (atts.value("data") == "true") ? 1 : 0;
    m_spectra->requireFits = (atts.value("fits") == "true") ? 1 : 0;
  }
  else if (element == "sza") {

    QString str;
    bool ok;
    double tmp;

    str = atts.value("min");
    tmp = str.toDouble(&ok);
    if (ok)
      m_spectra->szaMinimum = tmp;

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

    m_spectra->useDarkFile = (atts.value("dark") == "true") ? 1 : 0;
    m_spectra->useNameFile = (atts.value("name") == "true") ? 1 : 0;
  }
  else if (element == "circle") {
    
    QString str;
    bool ok;
    double tmp;

    m_spectra->geo.circle.mode = cGeolocationModeCircle;

    str = atts.value("radius");
    tmp = str.toDouble(&ok);
    if (ok)
      m_spectra->geo.circle.radius = tmp;

    str = atts.value("long");
    tmp = str.toDouble(&ok);
    if (ok)
      m_spectra->geo.circle.centreLongitude = tmp;

    str = atts.value("lat");
    tmp = str.toDouble(&ok);
    if (ok)
      m_spectra->geo.circle.centreLatitude = tmp;
  }
  else if (element == "rectangle") {

    QString str;
    bool ok;
    double tmp;

    m_spectra->geo.rectangle.mode = cGeolocationModeRectangle;

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
    // defaults to none ...
    m_spectra->geo.mode = cGeolocationModeNone;
  }

  return true;
}

bool CProjectSpectraSubHandler::end(const QString &element)
{
  return true;
}

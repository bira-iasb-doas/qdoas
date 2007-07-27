
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFontMetrics>
#include <QRegExpValidator>

#include "CWProjectTabSpectra.h"
#include "CValidator.h"

CWProjectTabSpectra::CWProjectTabSpectra(const mediate_project_spectra_t *properties, QWidget *parent) :
  QFrame(parent)
{
  // construct the GUI and use properties (not NULL) to set the state of the edit widgets.
  // Each of the GUI components maintains its bit of 'properties' state (until 'apply'ed).

  // use font metrics to size the line edits
  QFontMetrics fm(font());
  int pixels;
  QString tmpStr;

  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  
  QVBoxLayout *leftLayout = new QVBoxLayout;

  // left side
  
  // Display (require data) group
  QGroupBox *displayGroup = new QGroupBox("Display", this);
  QVBoxLayout *displayGroupLayout = new QVBoxLayout;

  m_reqSpectraCheck = new QCheckBox("Spectra");
  displayGroupLayout->addWidget(m_reqSpectraCheck);
  m_reqDataCheck = new QCheckBox("Data");
  displayGroupLayout->addWidget(m_reqDataCheck);
  m_reqFitsCheck = new QCheckBox("Fits");
  displayGroupLayout->addWidget(m_reqFitsCheck);
  displayGroup->setLayout(displayGroupLayout);

  m_reqSpectraCheck->setCheckState(properties->requireSpectra ? Qt::Checked : Qt::Unchecked);
  m_reqDataCheck->setCheckState(properties->requireData ? Qt::Checked : Qt::Unchecked);
  m_reqFitsCheck->setCheckState(properties->requireFits ? Qt::Checked : Qt::Unchecked);

  leftLayout->addWidget(displayGroup);

  // Gelocation selection - also in a group box
  QGroupBox *geoGroup = new QGroupBox("Geolocations", this);
  QVBoxLayout *geoGroupLayout = new QVBoxLayout;
  m_geolocationEdit = new CWGeolocation(&(properties->geo));
  geoGroupLayout->addWidget(m_geolocationEdit);
  geoGroup->setLayout(geoGroupLayout);

  leftLayout->addWidget(geoGroup);

  leftLayout->addStretch(1);

  mainLayout->addLayout(leftLayout, 1);

  // right side

  QVBoxLayout *rightLayout = new QVBoxLayout;

   // SZA group
  QGroupBox *szaGroup = new QGroupBox("SZA", this);
  QGridLayout *szaGroupLayout = new QGridLayout;

  pixels = fm.width("00000000");
  szaGroupLayout->addWidget(new QLabel("Min", this), 0, 0);
  m_szaMinEdit = new QLineEdit(this);
  m_szaMinEdit->setFixedWidth(pixels);
  m_szaMinEdit->setValidator(new CSzaValidator(m_szaMinEdit));
  szaGroupLayout->addWidget(m_szaMinEdit, 0, 1);
  szaGroupLayout->addWidget(new QLabel("Max", this), 1, 0);
  m_szaMaxEdit = new QLineEdit(this);
  m_szaMaxEdit->setFixedWidth(pixels);
  m_szaMaxEdit->setValidator(new CSzaValidator(m_szaMaxEdit));
  szaGroupLayout->addWidget(m_szaMaxEdit, 1, 1);
  szaGroupLayout->addWidget(new QLabel("Delta", this), 2, 0);
  m_szaDeltaEdit = new QLineEdit(this);
  m_szaDeltaEdit->setFixedWidth(pixels);
  m_szaDeltaEdit->setValidator(new CSzaValidator(m_szaDeltaEdit));
  szaGroupLayout->addWidget(m_szaDeltaEdit, 2, 1);
  szaGroup->setLayout(szaGroupLayout);
 
  // use the validators to input-check the initial values
  m_szaMinEdit->validator()->fixup(tmpStr.setNum(properties->szaMinimum));
  m_szaMinEdit->setText(tmpStr);
  m_szaMaxEdit->validator()->fixup(tmpStr.setNum(properties->szaMaximum));
  m_szaMaxEdit->setText(tmpStr);
  m_szaDeltaEdit->validator()->fixup(tmpStr.setNum(properties->szaDelta));
  m_szaDeltaEdit->setText(tmpStr);

  rightLayout->addWidget(szaGroup);

  // Record group
  QGroupBox *recordGroup = new QGroupBox("Spectra No. Range", this);
  QGridLayout *recordGroupLayout = new QGridLayout;

  pixels = fm.width("000000");
  recordGroupLayout->addWidget(new QLabel("Min", this), 0, 0);
  m_recordMinEdit = new QLineEdit(this);
  m_recordMinEdit->setFixedWidth(pixels);
  m_recordMinEdit->setValidator(new QIntValidator(0, 99999, m_recordMinEdit));
  recordGroupLayout->addWidget(m_recordMinEdit, 0, 1);
  recordGroupLayout->addWidget(new QLabel("Max", this), 1, 0);
  m_recordMaxEdit = new QLineEdit(this);
  m_recordMaxEdit->setFixedWidth(pixels);
  m_recordMaxEdit->setValidator(new QIntValidator(0, 99999, m_recordMaxEdit));
  recordGroupLayout->addWidget(m_recordMaxEdit, 1, 1);
  recordGroup->setLayout(recordGroupLayout);

  m_recordMinEdit->validator()->fixup(tmpStr.setNum(properties->recordNumberMinimum));
  m_recordMinEdit->setText(tmpStr);
  m_recordMaxEdit->validator()->fixup(tmpStr.setNum(properties->recordNumberMaximum));
  m_recordMaxEdit->setText(tmpStr);

  rightLayout->addWidget(recordGroup);

  // Dark + Name group
  QGroupBox *fileGroup = new QGroupBox("Files", this);
  QVBoxLayout *fileGroupLayout = new QVBoxLayout;

  m_useNameCheck = new QCheckBox("Names", this);
  fileGroupLayout->addWidget(m_useNameCheck);
  m_useDarkCheck = new QCheckBox("Dark current", this);
  fileGroupLayout->addWidget(m_useDarkCheck);
  fileGroup->setLayout(fileGroupLayout);

  m_useNameCheck->setCheckState(properties->useNameFile ? Qt::Checked : Qt::Unchecked);
  m_useDarkCheck->setCheckState(properties->useDarkFile ? Qt::Checked : Qt::Unchecked);

  rightLayout->addWidget(fileGroup);

  mainLayout->addLayout(rightLayout, 0);
}

CWProjectTabSpectra::~CWProjectTabSpectra()
{
}

void CWProjectTabSpectra::apply(mediate_project_spectra_t *properties) const
{
  // extract state from the GUI and set properties
  QString tmpStr;

  // Display
  properties->requireSpectra = (m_reqSpectraCheck->checkState() == Qt::Checked);
  properties->requireData = (m_reqDataCheck->checkState() == Qt::Checked);
  properties->requireFits = (m_reqFitsCheck->checkState() == Qt::Checked);
  
  // SZA
  tmpStr = m_szaMinEdit->text();
  properties->szaMinimum = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
  tmpStr = m_szaMaxEdit->text();
  properties->szaMaximum = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
  tmpStr = m_szaDeltaEdit->text();
  properties->szaDelta = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();

  // Record
  tmpStr = m_recordMinEdit->text();
  properties->recordNumberMinimum = tmpStr.isEmpty() ? 0 : tmpStr.toInt();
  tmpStr = m_recordMaxEdit->text();
  properties->recordNumberMaximum = tmpStr.isEmpty() ? 0 : tmpStr.toInt();
  
  // Files
  properties->useNameFile = (m_useNameCheck->checkState() == Qt::Checked);
  properties->useDarkFile = (m_useDarkCheck->checkState() == Qt::Checked);

  // Geolocation
  m_geolocationEdit->apply(&(properties->geo));
}

CWGeolocation::CWGeolocation(const union geolocation *geo, QWidget *parent) :
  QFrame(parent)
{
  // use font metrics to size the line edits
  QFontMetrics fm(font());
  int pixels = fm.width("00000000"); // same for all lineedits

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
   
  m_modeStack = new QStackedWidget(this);

  m_modeCombo = new QComboBox(this);
  mainLayout->addWidget(m_modeCombo);

  m_modeCombo->addItem("Unrestricted", QVariant(cGeolocationModeNone));
  m_modeStack->addWidget(new QFrame(this)); // blank placeholder

  // each mode has a frame widget that is placed in the stack. The widget shown is
  // coupled to the modeCombo

  // circle
  QFrame *circleFrame = new QFrame;
  QGridLayout *circleLayout = new QGridLayout;

  // row 0
  circleLayout->addWidget(new QLabel("Radius (degrees)", circleFrame), 0, 0);
  m_radiusEdit = new QLineEdit(this);
  m_radiusEdit->setFixedWidth(pixels);
  circleLayout->addWidget(m_radiusEdit, 0, 1);

  // row 1
  circleLayout->addWidget(new QLabel("Centre Longitude (degrees)", circleFrame), 1, 0);
  m_cenLongEdit = new QLineEdit(this);
  m_cenLongEdit->setFixedWidth(pixels);
  circleLayout->addWidget(m_cenLongEdit, 1, 1);

  circleLayout->addWidget(new QLabel("Centre Latitude (degrees)", circleFrame) , 2, 0);
  m_cenLatEdit = new QLineEdit(this);
  m_cenLatEdit->setFixedWidth(pixels);  
  circleLayout->addWidget(m_cenLatEdit, 2, 1);

  circleFrame->setLayout(circleLayout);
  m_modeStack->addWidget(circleFrame);
  m_modeCombo->addItem("Circle", QVariant(cGeolocationModeCircle));

  // rectangle
  QFrame *rectangleFrame = new QFrame;
  QGridLayout *rectangleLayout = new QGridLayout;

  // row 0
  rectangleLayout->addWidget(new QLabel("Western Limit (long. degrees)", rectangleFrame), 0, 0);
  m_westEdit = new QLineEdit(this);
  m_westEdit->setFixedWidth(pixels);
  rectangleLayout->addWidget(m_westEdit, 0, 1);

  // row 1
  rectangleLayout->addWidget(new QLabel("Eastern Limit (long. degrees)", rectangleFrame), 1, 0);
  m_eastEdit = new QLineEdit(this);
  m_eastEdit->setFixedWidth(pixels);
  rectangleLayout->addWidget(m_eastEdit, 1, 1);
  
  // row 2
  rectangleLayout->addWidget(new QLabel("Southern Limit (long. degrees)", rectangleFrame) , 2, 0);
  m_southEdit = new QLineEdit(this);
  m_southEdit->setFixedWidth(pixels);
  rectangleLayout->addWidget(m_southEdit, 2, 1);
  
  // row 3 
  rectangleLayout->addWidget( new QLabel("Northern Limit (long. degrees)", rectangleFrame), 3, 0);
  m_northEdit = new QLineEdit(this);
  m_northEdit->setFixedWidth(pixels);
  rectangleLayout->addWidget(m_northEdit, 3, 1);

  rectangleFrame->setLayout(rectangleLayout);
  m_modeStack->addWidget(rectangleFrame);
  m_modeCombo->addItem("Rectangle", QVariant(cGeolocationModeRectangle));

  mainLayout->addWidget(m_modeStack);

  // set the defaults

  // connections
  connect(m_modeCombo, SIGNAL(currentIndexChanged(int)), m_modeStack, SLOT(setCurrentIndex(int)));

  // set the mode based on the current selected item

  QString tmpStr;
  int index = m_modeCombo->findData(QVariant(geo->mode));
  
  switch (geo->mode) {
  case cGeolocationModeRectangle:
    {
      // TODO - ditch sprintf
      m_westEdit->setText(tmpStr.sprintf("%.3f", geo->rectangle.westernLongitude));
      m_eastEdit->setText(tmpStr.sprintf("%.3f", geo->rectangle.easternLongitude));
      m_southEdit->setText(tmpStr.sprintf("%.3f", geo->rectangle.southernLatitude));
      m_northEdit->setText(tmpStr.sprintf("%.3f", geo->rectangle.northernLatitude));
    }
    break;
  case cGeolocationModeCircle:
    {
      m_radiusEdit->setText(tmpStr.sprintf("%.3f", geo->circle.radius));
      m_cenLongEdit->setText(tmpStr.sprintf("%.3f", geo->circle.centreLongitude));
      m_cenLatEdit->setText(tmpStr.sprintf("%.3f", geo->circle.centreLatitude));
    }
    break;
  }
  
  if (index != -1)
    m_modeCombo->setCurrentIndex(index);
}

CWGeolocation::~CWGeolocation()
{
}

void CWGeolocation::apply(union geolocation *geo) const
{
  int index = m_modeCombo->currentIndex();
  int mode = (index == -1) ? cGeolocationModeNone : m_modeCombo->itemData(index).toInt();

  switch (mode) {
  case cGeolocationModeRectangle:
    {
      QString tmpStr;
      geo->rectangle.mode = mode;
      tmpStr = m_westEdit->text();
      geo->rectangle.westernLongitude = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
      tmpStr = m_eastEdit->text();
      geo->rectangle.easternLongitude = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
      tmpStr = m_southEdit->text();
      geo->rectangle.southernLatitude = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
      tmpStr = m_northEdit->text();
      geo->rectangle.northernLatitude = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
    }
    break;
  case cGeolocationModeCircle:
    {
      QString tmpStr;
      geo->circle.mode = mode;
      tmpStr = m_radiusEdit->text();
      geo->circle.radius = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
      tmpStr = m_cenLongEdit->text();
      geo->circle.centreLongitude = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
      tmpStr = m_cenLatEdit->text();
      geo->circle.centreLatitude = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
    }
    break;
  default:
    geo->mode = cGeolocationModeNone;
  }
}


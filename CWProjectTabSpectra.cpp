
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFontMetrics>
#include <QRegExpValidator>

#include "CWProjectTabSpectra.h"

class CSzaValidator : public QValidator
{
 public:
  CSzaValidator(QObject *obj);

  virtual QValidator::State validate(QString &input, int &pos) const;
};

CSzaValidator::CSzaValidator(QObject *obj) :
  QValidator(obj)
{
}

QValidator::State CSzaValidator::validate(QString &input, int &pos) const
{

  if (input.isEmpty()) return QValidator::Intermediate;

  bool ok;
  double v = input.toDouble(&ok);

  if (ok) {
    if (v < 0.0 || v > 180.0) return QValidator::Invalid;
    if (v < 10.0 && input.length() > 5) return QValidator::Invalid;
    if (v < 100.0 && input.length() > 6)  return QValidator::Invalid;
    return QValidator::Acceptable;
  }

  return QValidator::Invalid;
}


//-------------------------------


CWProjectTabSpectra::CWProjectTabSpectra(mediate_project_spectra_t *properties, QWidget *parent) :
  QFrame(parent),
  m_properties(properties)
{
  QFontMetrics fm(font());
  int pixels;

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

  leftLayout->addWidget(displayGroup);

  // Gelocation selection - also in a group box
  QGroupBox *geoGroup = new QGroupBox("Gelocations", this);
  QVBoxLayout *geoGroupLayout = new QVBoxLayout;
  geoGroupLayout->addWidget(new CWGeolocation(&(m_properties->geo)));
  geoGroup->setLayout(geoGroupLayout);

  leftLayout->addWidget(geoGroup);

  leftLayout->addStretch(1);

  mainLayout->addLayout(leftLayout);

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

  rightLayout->addWidget(recordGroup);

  // Dark + Name group
  QGroupBox *fileGroup = new QGroupBox("Files", this);
  QVBoxLayout *fileGroupLayout = new QVBoxLayout;

  m_useNameCheck = new QCheckBox("Names", this);
  fileGroupLayout->addWidget(m_useNameCheck);
  m_useDarkCheck = new QCheckBox("Dark current", this);
  fileGroupLayout->addWidget(m_useDarkCheck);
  fileGroup->setLayout(fileGroupLayout);

  rightLayout->addWidget(fileGroup);

  mainLayout->addLayout(rightLayout);
}

CWProjectTabSpectra::~CWProjectTabSpectra()
{
}


CWGeolocation::CWGeolocation(union geolocation *geo, QWidget *parent) :
  QFrame(parent),
  m_geo(geo)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
   
  m_modeCombo = new QComboBox(this);
  m_modeCombo->addItem("Unrestricted", QVariant(int(0)));
  m_modeCombo->addItem("Circle", QVariant(int(1)));
  m_modeCombo->addItem("Rectangle", QVariant(int(2)));
  mainLayout->addWidget(m_modeCombo);
  
  // rectangle
  m_rectangleFrame = new QFrame;
  QGridLayout *rectangleLayout = new QGridLayout;

  // row 0
  rectangleLayout->addWidget(new QLabel("Western Limit (long. degrees)", m_rectangleFrame), 0, 0);
  m_westEdit = new QLineEdit(this);
  rectangleLayout->addWidget(m_westEdit, 0, 1);

  // row 1
  rectangleLayout->addWidget(new QLabel("Eastern Limit (long. degrees)", m_rectangleFrame), 1, 0);
  m_eastEdit = new QLineEdit(this);
  rectangleLayout->addWidget(m_eastEdit, 1, 1);
  
  // row 2
  rectangleLayout->addWidget(new QLabel("Southern Limit (long. degrees)", m_rectangleFrame) , 2, 0);
  m_southEdit = new QLineEdit(this);
  rectangleLayout->addWidget(m_southEdit, 2, 1);
  
  // row 3 
  rectangleLayout->addWidget( new QLabel("Northern Limit (long. degrees)", m_rectangleFrame), 3, 0);
  m_northEdit = new QLineEdit(this);
  rectangleLayout->addWidget(m_northEdit, 3, 1);

  m_rectangleFrame->setLayout(rectangleLayout);

  m_rectangleFrame->hide();
  mainLayout->addWidget(m_rectangleFrame);

  // circle
  m_circleFrame = new QFrame;
  QGridLayout *circleLayout = new QGridLayout;

  // row 0
  circleLayout->addWidget(new QLabel("Radius (degrees)", m_circleFrame), 0, 0);
  m_radiusEdit = new QLineEdit(this);
  circleLayout->addWidget(m_radiusEdit, 0, 1);

  // row 1
  circleLayout->addWidget(new QLabel("Centre Longitude (degrees)", m_circleFrame), 1, 0);
  m_westEdit = new QLineEdit(this);
  circleLayout->addWidget(m_westEdit, 1, 1);

  circleLayout->addWidget(new QLabel("Centre Latitude (degrees)", m_circleFrame) , 2, 0);
  m_southEdit = new QLineEdit(this);
  circleLayout->addWidget(m_southEdit, 2, 1);

  m_circleFrame->setLayout(circleLayout);

  m_circleFrame->hide();
  mainLayout->addWidget(m_circleFrame);

  // set the defaults

  // connections
  connect(m_modeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotModeChanged(int)));

  // set the mode based on the current type
  int index =m_modeCombo->findData(QVariant(geo->type));
  if (index != -1)
    m_modeCombo->setCurrentIndex(index);
}

CWGeolocation::~CWGeolocation()
{
}

void CWGeolocation::slotModeChanged(int index)
{
  int type = m_modeCombo->itemData(index).toInt();

  switch (type) {
  case 2: // rectangle
    {
      m_circleFrame->hide();
      m_rectangleFrame->show();
    }
    break;
  case 1: // circle
    {
      m_rectangleFrame->hide();
      m_circleFrame->show();
    }
    break;
  case 0: // unrestricted
  default:
    {
      m_circleFrame->hide();
      m_rectangleFrame->hide();
    }
    break;
  }
}


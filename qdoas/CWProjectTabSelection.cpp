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


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFontMetrics>
#include <QRegExpValidator>

#include "CWProjectTabSelection.h"
#include "CValidator.h"

#include "constants.h"

CWProjectTabSelection::CWProjectTabSelection(const mediate_project_selection_t *properties, QWidget *parent) :
  QFrame(parent)
{
  // construct the GUI and use properties (not NULL) to set the state of the edit widgets.
  // Each of the GUI components maintains its bit of 'properties' state (until 'apply'ed).

  // use font metrics to size the line edits
  QFontMetrics fm(font());
  int pixels;
  QString tmpStr;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  QHBoxLayout *topLayout = new QHBoxLayout;
  topLayout->setMargin(0);

   // SZA group
  QGroupBox *szaGroup = new QGroupBox("SZA", this);
  QGridLayout *szaGroupLayout = new QGridLayout;

  pixels = fm.width("00000000");
  szaGroupLayout->addWidget(new QLabel("Min", this), 0, 0);
  m_szaMinEdit = new QLineEdit(this);
  m_szaMinEdit->setFixedWidth(pixels);
  m_szaMinEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 180.0, 3, m_szaMinEdit));
  szaGroupLayout->addWidget(m_szaMinEdit, 0, 1);
  szaGroupLayout->addWidget(new QLabel("Max", this), 1, 0);
  m_szaMaxEdit = new QLineEdit(this);
  m_szaMaxEdit->setFixedWidth(pixels);
  m_szaMaxEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 180.0, 3, m_szaMaxEdit));
  szaGroupLayout->addWidget(m_szaMaxEdit, 1, 1);
  szaGroupLayout->addWidget(new QLabel("Delta", this), 2, 0);
  m_szaDeltaEdit = new QLineEdit(this);
  m_szaDeltaEdit->setFixedWidth(pixels);
  m_szaDeltaEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 180.0, 3, m_szaDeltaEdit));
  szaGroupLayout->addWidget(m_szaDeltaEdit, 2, 1);
  szaGroupLayout->setColumnStretch(2, 1);
  szaGroup->setLayout(szaGroupLayout);

  // use the validators to input-check the initial values
  m_szaMinEdit->validator()->fixup(tmpStr.setNum(properties->szaMinimum));
  m_szaMinEdit->setText(tmpStr);
  m_szaMaxEdit->validator()->fixup(tmpStr.setNum(properties->szaMaximum));
  m_szaMaxEdit->setText(tmpStr);
  m_szaDeltaEdit->validator()->fixup(tmpStr.setNum(properties->szaDelta));
  m_szaDeltaEdit->setText(tmpStr);

  topLayout->addWidget(szaGroup);

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
  recordGroupLayout->setColumnStretch(2, 1);
  recordGroup->setLayout(recordGroupLayout);

  m_recordMinEdit->validator()->fixup(tmpStr.setNum(properties->recordNumberMinimum));
  m_recordMinEdit->setText(tmpStr);
  m_recordMaxEdit->validator()->fixup(tmpStr.setNum(properties->recordNumberMaximum));
  m_recordMaxEdit->setText(tmpStr);

  topLayout->addWidget(recordGroup);

  // Cloud fraction group

  m_cloudFractionGroup = new QGroupBox("Cloud fraction", this);
  QGridLayout *cloudFractionGroupLayout = new QGridLayout;
  cloudFractionGroupLayout->addWidget(new QLabel("Min", this), 0, 0);

  m_cloudFractionMinEdit = new QLineEdit(this);
  m_cloudFractionMinEdit->setFixedWidth(pixels);
  m_cloudFractionMinEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 1.0, 3, m_cloudFractionMinEdit));
  cloudFractionGroupLayout->addWidget(m_cloudFractionMinEdit, 0, 1);
  cloudFractionGroupLayout->addWidget(new QLabel("Max", this), 1, 0);
  m_cloudFractionMaxEdit = new QLineEdit(this);
  m_cloudFractionMaxEdit->setFixedWidth(pixels);
  m_cloudFractionMaxEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 1.0, 3, m_cloudFractionMaxEdit));
  cloudFractionGroupLayout->addWidget(m_cloudFractionMaxEdit, 1, 1);

  cloudFractionGroupLayout->setColumnStretch(2, 1);
  m_cloudFractionGroup->setLayout(cloudFractionGroupLayout);

  // use the validators to input-check the initial values
  m_cloudFractionMinEdit->validator()->fixup(tmpStr.setNum(properties->cloudFractionMinimum));
  m_cloudFractionMinEdit->setText(tmpStr);
  m_cloudFractionMaxEdit->validator()->fixup(tmpStr.setNum(properties->cloudFractionMaximum));
  m_cloudFractionMaxEdit->setText(tmpStr);

  topLayout->addWidget(m_cloudFractionGroup);

  mainLayout->addLayout(topLayout);

  // Gelocation selection - also in a group box
  QGroupBox *geoGroup = new QGroupBox("Geolocations", this);
  QVBoxLayout *geoGroupLayout = new QVBoxLayout;
  m_geolocationEdit = new CWGeolocation(&(properties->geo));
  geoGroupLayout->addWidget(m_geolocationEdit);
  geoGroup->setLayout(geoGroupLayout);

  mainLayout->addWidget(geoGroup);

  mainLayout->addStretch(1);
}

CWProjectTabSelection::~CWProjectTabSelection()
{
}

void CWProjectTabSelection::apply(mediate_project_selection_t *properties) const
{
  // extract state from the GUI and set properties
  QString tmpStr;

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

  // Cloud fraction

  tmpStr = m_cloudFractionMinEdit->text();
  properties->cloudFractionMinimum = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
  tmpStr = m_cloudFractionMaxEdit->text();
  properties->cloudFractionMaximum = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();

  // Geolocation
  m_geolocationEdit->apply(&(properties->geo));
}

void CWProjectTabSelection::slotInstrumentChanged(int instrument)
{
	if (instrument==PRJCT_INSTR_FORMAT_GOME2)
	 m_cloudFractionGroup->show();
	else
	 m_cloudFractionGroup->hide();
}


CWGeolocation::CWGeolocation(const struct geolocation *geo, QWidget *parent) :
  QFrame(parent)
{
  // use font metrics to size the line edits
  QFontMetrics fm(font());
  int pixels = fm.width("00000000"); // same for all lineedits

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  m_modeStack = new QStackedWidget(this);

  m_modeCombo = new QComboBox(this);
  mainLayout->addWidget(m_modeCombo);

  m_modeCombo->addItem("Unrestricted", QVariant(PRJCT_SPECTRA_MODES_NONE));
  m_modeStack->addWidget(new QFrame(this)); // blank placeholder

  // each mode has a frame widget that is placed in the stack. The widget shown is
  // coupled to the modeCombo

  // circle
  QFrame *circleFrame = new QFrame;
  QGridLayout *circleLayout = new QGridLayout;

  // row 0
  circleLayout->addWidget(new QLabel("Radius (km)", circleFrame), 0, 0);
  m_radiusEdit = new QLineEdit(this);
  m_radiusEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 6500.0, 3, m_radiusEdit));
  m_radiusEdit->setFixedWidth(pixels);
  circleLayout->addWidget(m_radiusEdit, 0, 1);

  // row 1
  circleLayout->addWidget(new QLabel("Center Longitude (degrees)", circleFrame), 1, 0);
  m_cenLongEdit = new QLineEdit(this);
  m_cenLongEdit->setValidator(new CDoubleFixedFmtValidator(-360, 360.0, 3, m_cenLongEdit));
  m_cenLongEdit->setFixedWidth(pixels);
  circleLayout->addWidget(m_cenLongEdit, 1, 1);

  // row 2
  circleLayout->addWidget(new QLabel("Center Latitude (degrees)", circleFrame) , 2, 0);
  m_cenLatEdit = new QLineEdit(this);
  m_cenLatEdit->setValidator(new CDoubleFixedFmtValidator(-90.0, 90.0, 3, m_cenLatEdit));
  m_cenLatEdit->setFixedWidth(pixels);
  circleLayout->addWidget(m_cenLatEdit, 2, 1);

  circleFrame->setLayout(circleLayout);
  m_modeStack->addWidget(circleFrame);
  m_modeCombo->addItem("Circle", QVariant(PRJCT_SPECTRA_MODES_CIRCLE));

  // rectangle
  QFrame *rectangleFrame = new QFrame;
  QGridLayout *rectangleLayout = new QGridLayout;

  // row 0
  rectangleLayout->addWidget(new QLabel("Western Limit (long. degrees)", rectangleFrame), 0, 0);
  m_westEdit = new QLineEdit(this);
  m_westEdit->setValidator(new CDoubleFixedFmtValidator(-180.0, 180.0, 3, m_westEdit));
  m_westEdit->setFixedWidth(pixels);
  rectangleLayout->addWidget(m_westEdit, 0, 1);

  // row 1
  rectangleLayout->addWidget(new QLabel("Eastern Limit (long. degrees)", rectangleFrame), 1, 0);
  m_eastEdit = new QLineEdit(this);
  m_eastEdit->setValidator(new CDoubleFixedFmtValidator(-180.0, 180.0, 3, m_eastEdit));
  m_eastEdit->setFixedWidth(pixels);
  rectangleLayout->addWidget(m_eastEdit, 1, 1);

  // row 2
  rectangleLayout->addWidget(new QLabel("Southern Limit (lat. degrees)", rectangleFrame) , 2, 0);
  m_southEdit = new QLineEdit(this);
  m_southEdit->setValidator(new CDoubleFixedFmtValidator(-90.0, 90.0, 3, m_southEdit));
  m_southEdit->setFixedWidth(pixels);
  rectangleLayout->addWidget(m_southEdit, 2, 1);

  // row 3
  rectangleLayout->addWidget( new QLabel("Northern Limit (lat. degrees)", rectangleFrame), 3, 0);
  m_northEdit = new QLineEdit(this);
  m_northEdit->setValidator(new CDoubleFixedFmtValidator(-90.0, 90.0, 3, m_northEdit));
  m_northEdit->setFixedWidth(pixels);
  rectangleLayout->addWidget(m_northEdit, 3, 1);

  rectangleFrame->setLayout(rectangleLayout);
  m_modeStack->addWidget(rectangleFrame);
  m_modeCombo->addItem("Rectangle", QVariant(PRJCT_SPECTRA_MODES_RECTANGLE));

  // sites
  QFrame *sitesFrame = new QFrame;
  QGridLayout *sitesLayout = new QGridLayout;

  sitesLayout->addWidget(new QLabel("Radius (km)", sitesFrame), 0, 0);
  m_sitesRadiusEdit = new QLineEdit(this);
  m_sitesRadiusEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 6500.0, 3, m_sitesRadiusEdit));
  m_sitesRadiusEdit->setFixedWidth(pixels);
  sitesLayout->addWidget(m_sitesRadiusEdit, 0, 1);

  sitesFrame->setLayout(sitesLayout);
  m_modeStack->addWidget(sitesFrame);
  m_modeCombo->addItem("Sites", QVariant(PRJCT_SPECTRA_MODES_OBSLIST));

  mainLayout->addWidget(m_modeStack);

  // set the defaults

  // connections
  connect(m_modeCombo, SIGNAL(currentIndexChanged(int)), m_modeStack, SLOT(setCurrentIndex(int)));

  // set the mode based on the current selected item

  QString tmpStr;
  int index = m_modeCombo->findData(QVariant(geo->mode));

  // rectangle
  m_westEdit->validator()->fixup(tmpStr.setNum(geo->rectangle.westernLongitude));
  m_westEdit->setText(tmpStr);
  m_eastEdit->validator()->fixup(tmpStr.setNum(geo->rectangle.easternLongitude));
  m_eastEdit->setText(tmpStr);
  m_southEdit->validator()->fixup(tmpStr.setNum(geo->rectangle.southernLatitude));
  m_southEdit->setText(tmpStr);
  m_northEdit->validator()->fixup(tmpStr.setNum(geo->rectangle.northernLatitude));
  m_northEdit->setText(tmpStr);

  // circle
  m_radiusEdit->validator()->fixup(tmpStr.setNum(geo->circle.radius));
  m_radiusEdit->setText(tmpStr);
  m_cenLongEdit->validator()->fixup(tmpStr.setNum(geo->circle.centerLongitude));
  m_cenLongEdit->setText(tmpStr);
  m_cenLatEdit->validator()->fixup(tmpStr.setNum(geo->circle.centerLatitude));
  m_cenLatEdit->setText (tmpStr);

  // sites
  m_sitesRadiusEdit->validator()->fixup(tmpStr.setNum(geo->sites.radius));
  m_sitesRadiusEdit->setText(tmpStr);

  if (index != -1)
    m_modeCombo->setCurrentIndex(index);
}

CWGeolocation::~CWGeolocation()
{
}

void CWGeolocation::apply(struct geolocation *geo) const
{
  int index = m_modeCombo->currentIndex();

  geo->mode = (index == -1) ? PRJCT_SPECTRA_MODES_NONE : m_modeCombo->itemData(index).toInt();

  // set data for all modes
  QString tmpStr;

  tmpStr = m_westEdit->text();
  geo->rectangle.westernLongitude = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
  tmpStr = m_eastEdit->text();
  geo->rectangle.easternLongitude = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
  tmpStr = m_southEdit->text();
  geo->rectangle.southernLatitude = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
  tmpStr = m_northEdit->text();
  geo->rectangle.northernLatitude = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();

  tmpStr = m_radiusEdit->text();
  geo->circle.radius = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
  tmpStr = m_cenLongEdit->text();
  geo->circle.centerLongitude = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();
  tmpStr = m_cenLatEdit->text();
  geo->circle.centerLatitude = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();

  tmpStr = m_sitesRadiusEdit->text();
  geo->sites.radius = tmpStr.isEmpty() ? 0.0 : tmpStr.toDouble();

}


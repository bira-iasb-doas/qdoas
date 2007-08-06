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

#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QFileDialog>

#include "CWProjectTabCalibration.h"
#include "CValidator.h"

#include "debugutil.h"

CWProjectTabCalibration::CWProjectTabCalibration(const mediate_project_calibration_t *properties, QWidget *parent) :
  QFrame(parent)
{
  int index;
  QString tmpStr;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  QGridLayout *topLayout = new QGridLayout;

  mainLayout->addSpacing(25);

  // ref file
  topLayout->addWidget(new QLabel("Solar Ref. File", this), 0, 0);
  m_refFileEdit  = new QLineEdit(this);
  m_refFileEdit->setMaxLength(sizeof(properties->solarRefFile) - 1); // limit test length to buffer size

  topLayout->addWidget(m_refFileEdit, 0, 1);
  QPushButton *browseBtn = new QPushButton("Browse...", this);
  topLayout->addWidget(browseBtn, 0, 2);

  // method
  topLayout->addWidget(new QLabel("Analysis Method", this), 1, 0);
  m_methodCombo = new QComboBox(this);
  m_methodCombo->addItem("Optical Density Fitting", QVariant(cProjAnalysisMethodModeOptDens));
  m_methodCombo->addItem("Intensity fitting (Marquardt-Levenberg+SVD)", QVariant(cProjAnalysisMethodModeMarqLevSvd));
  m_methodCombo->addItem("Intensity fitting (Full Marquardt-Levenberg)", QVariant(cProjAnalysisMethodModeMarqLevFull));
  topLayout->addWidget(m_methodCombo, 1, 1, 1, 2); // spans two columns

  // line shape
  topLayout->addWidget(new QLabel("Line Shape (SFP)", this), 2, 0);
  m_lineShapeCombo = new QComboBox(this);
  m_lineShapeCombo->addItem("Dont Fit", QVariant(cSpectralLineShapeNone));
  m_lineShapeCombo->addItem("Gaussian", QVariant(cSpectralLineShapeGaussian));
  m_lineShapeCombo->addItem("Error Function", QVariant(cSpectralLineShapeErrorFunction));
  m_lineShapeCombo->addItem("2n-Lorentz", QVariant(cSpectralLineShapeLorentz));
  m_lineShapeCombo->addItem("Voigt", QVariant(cSpectralLineShapeVoigt));
  topLayout->addWidget(m_lineShapeCombo, 2, 1);
 
  // degree of 2n-Lorentz
  m_degreeWidget = new QFrame(this);
  QHBoxLayout *degreeLayout = new QHBoxLayout(m_degreeWidget);
  degreeLayout->addWidget(new QLabel("Degree", this));
  m_degreeSpinBox = new QSpinBox(this);
  m_degreeSpinBox->setRange(0, 10);
  degreeLayout->addWidget(m_degreeSpinBox);
  topLayout->addWidget(m_degreeWidget, 2, 2);
  m_degreeWidget->hide(); // show when lineshape combo is cSpectralLineShapeLorentz

  mainLayout->addLayout(topLayout);

  mainLayout->addSpacing(25);

  // middle
  QHBoxLayout *groupLayout = new QHBoxLayout;

  // display
  QGroupBox *displayGroup = new QGroupBox("Display", this);
  QGridLayout *displayLayout = new QGridLayout(displayGroup);

  m_spectraCheck = new QCheckBox("Spectra", displayGroup);
  displayLayout->addWidget(m_spectraCheck, 0, 0);
  m_fitsCheck = new QCheckBox("Fits", displayGroup);
  displayLayout->addWidget(m_fitsCheck, 0, 1);
  m_residualCheck = new QCheckBox("Residual", displayGroup);
  displayLayout->addWidget(m_residualCheck, 1, 0);
  m_shiftSfpCheck = new QCheckBox("Shift/SFP", displayGroup);
  displayLayout->addWidget(m_shiftSfpCheck, 1, 1);
  
  groupLayout->addWidget(displayGroup);

  // Polynomial Degree
  QGroupBox *polyGroup = new QGroupBox("Polynomial Degree", this);
  QGridLayout *polyLayout = new QGridLayout(polyGroup);

  polyLayout->addWidget(new QLabel("Shift", polyGroup), 0, 0);
  m_shiftDegreeSpinBox = new QSpinBox(polyGroup);
  m_shiftDegreeSpinBox->setRange(1,5);
  polyLayout->addWidget(m_shiftDegreeSpinBox, 0, 1);

  polyLayout->addWidget(new QLabel("SFP", polyGroup), 1, 0);
  m_sfpDegreeSpinBox = new QSpinBox(polyGroup);
  m_sfpDegreeSpinBox->setRange(1,5);
  polyLayout->addWidget(m_sfpDegreeSpinBox, 1, 1);

  groupLayout->addWidget(polyGroup);

  // window limts
  QGroupBox *windowGroup = new QGroupBox("Window Limts (nm)", this);
  QGridLayout *windowLayout = new QGridLayout(windowGroup);

  windowLayout->addWidget(new QLabel("Min", windowGroup), 0, 0);
  m_lambdaMinEdit = new QLineEdit(windowGroup);
  m_lambdaMinEdit->setValidator(new CDoubleFixedFmtValidator(100.0, 900.0, 2, m_lambdaMinEdit));
  windowLayout->addWidget(m_lambdaMinEdit, 0, 1);

  windowLayout->addWidget(new QLabel("Max", windowGroup), 1, 0);
  m_lambdaMaxEdit = new QLineEdit(windowGroup);
  m_lambdaMaxEdit->setValidator(new CDoubleFixedFmtValidator(100.0, 900.0, 2, m_lambdaMaxEdit));
  windowLayout->addWidget(m_lambdaMaxEdit, 1, 1);
 
  groupLayout->addWidget(windowGroup);

  mainLayout->addLayout(groupLayout);
  
  mainLayout->addSpacing(25);

  // bottom
  QHBoxLayout *subWindowLayout = new QHBoxLayout;
  
  subWindowLayout->addWidget(new QLabel("Number of sub-windows", this));
  m_subWindowsSpinBox = new QSpinBox(this);
  m_subWindowsSpinBox->setRange(1, 50);
  subWindowLayout->addWidget(m_subWindowsSpinBox);
  subWindowLayout->addStretch(1);

  QPushButton *fitBtn = new QPushButton("Fit Parameters", this);
  subWindowLayout->addWidget(fitBtn);

  mainLayout->addLayout(subWindowLayout);
  
  mainLayout->addStretch(1);

  // set initial values

  m_refFileEdit->setText(QString(properties->solarRefFile));

  index = m_methodCombo->findData(QVariant(properties->method));
  if (index != -1)
    m_methodCombo->setCurrentIndex(index);

  m_degreeSpinBox->setValue(properties->lorentzDegree);

  index = m_lineShapeCombo->findData(QVariant(properties->lineShape));
  if (index != -1) {
    m_lineShapeCombo->setCurrentIndex(index); // no signals yet
    slotLineShapeSelectionChanged(index);
  }

  m_spectraCheck->setChecked(properties->requireSpectra != 0);
  m_fitsCheck->setChecked(properties->requireFits != 0);
  m_residualCheck->setChecked(properties->requireResidual != 0);
  m_shiftSfpCheck->setChecked(properties->requireShiftSfp != 0);
  
  m_shiftDegreeSpinBox->setValue(properties->shiftDegree);
  m_sfpDegreeSpinBox->setValue(properties->sfpDegree);
  
  m_lambdaMinEdit->validator()->fixup(tmpStr.setNum(properties->wavelengthMin));
  m_lambdaMinEdit->setText(tmpStr);
  m_lambdaMaxEdit->validator()->fixup(tmpStr.setNum(properties->wavelengthMax));
  m_lambdaMaxEdit->setText(tmpStr);

  m_subWindowsSpinBox->setValue(properties->subWindows);

  // connections
  connect(m_lineShapeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLineShapeSelectionChanged(int)));
  connect(browseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseSolarRefFile()));
  connect(fitBtn, SIGNAL(clicked()), this, SLOT(slotFitParameters()));
  
}

CWProjectTabCalibration::~CWProjectTabCalibration()
{
}

void CWProjectTabCalibration::apply(mediate_project_calibration_t *properties) const
{
  // a safe text length is assured.
  strcpy(properties->solarRefFile, m_refFileEdit->text().toAscii().data());

  properties->method = m_methodCombo->itemData(m_methodCombo->currentIndex()).toInt();
  
  properties->lineShape = m_lineShapeCombo->itemData(m_lineShapeCombo->currentIndex()).toInt();
  properties->lorentzDegree = m_degreeSpinBox->value();

  properties->requireSpectra = (m_spectraCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->requireFits = (m_fitsCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->requireResidual = (m_residualCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->requireShiftSfp = (m_shiftSfpCheck->checkState() == Qt::Checked) ? 1 : 0;
  
  properties->shiftDegree = m_shiftDegreeSpinBox->value();
  properties->sfpDegree = m_sfpDegreeSpinBox->value();
  
  properties->wavelengthMin = m_lambdaMinEdit->text().toDouble();
  properties->wavelengthMax = m_lambdaMaxEdit->text().toDouble();

  properties->subWindows = m_subWindowsSpinBox->value(); 
}

void CWProjectTabCalibration::slotLineShapeSelectionChanged(int index)
{
  if (m_lineShapeCombo->itemData(index).toInt() == cSpectralLineShapeLorentz)
    m_degreeWidget->show();
  else
    m_degreeWidget->hide();
}

void CWProjectTabCalibration::slotBrowseSolarRefFile()
{
  QString filename = QFileDialog::getOpenFileName(this, "Open Solar Reference File", "/home/ian",
                                                  "Kurucz File (*.ktz);;All Files (*)");
  
  if (!filename.isEmpty())
    m_refFileEdit->setText(filename);
}

void CWProjectTabCalibration::slotFitParameters()
{
  TRACE("CWProjectTabCalibration::slotFitParameters : TODO");
}

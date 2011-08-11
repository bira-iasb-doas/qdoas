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
#include "CPreferences.h"
#include "CValidator.h"

#include "constants.h"

#include "debugutil.h"

CWProjectTabCalibration::CWProjectTabCalibration(const mediate_project_calibration_t *properties, QWidget *parent) :
  QFrame(parent)
{
  int index;
  QString tmpStr;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  QGridLayout *topLayout = new QGridLayout;

  mainLayout->addSpacing(5);

  // ref file
  topLayout->addWidget(new QLabel("Solar Ref. File", this), 0, 0);
  m_refFileEdit  = new QLineEdit(this);
  m_refFileEdit->setMaxLength(sizeof(properties->solarRefFile) - 1); // limit test length to buffer size

  topLayout->addWidget(m_refFileEdit, 0, 1);
  QPushButton *browseBtn = new QPushButton("Browse", this);
  topLayout->addWidget(browseBtn, 0, 2);

  // methodType
  topLayout->addWidget(new QLabel("Analysis Method", this), 1, 0);
  m_methodCombo = new QComboBox(this);
  m_methodCombo->addItem("Optical Density Fitting", QVariant(PRJCT_ANLYS_METHOD_SVD));
  m_methodCombo->addItem("Intensity fitting (Marquardt-Levenberg+SVD)", QVariant(PRJCT_ANLYS_METHOD_SVDMARQUARDT));
  topLayout->addWidget(m_methodCombo, 1, 1, 1, 2); // spans two columns

  // line shape
  topLayout->addWidget(new QLabel("Line Shape (SFP)", this), 2, 0);
  m_lineShapeCombo = new QComboBox(this);
  m_lineShapeCombo->addItem("Dont Fit", QVariant(PRJCT_CALIB_FWHM_TYPE_NONE));
  m_lineShapeCombo->addItem("Gaussian", QVariant(PRJCT_CALIB_FWHM_TYPE_GAUSS));
  m_lineShapeCombo->addItem("Error Function", QVariant(PRJCT_CALIB_FWHM_TYPE_ERF));
  m_lineShapeCombo->addItem("2n-Lorentz", QVariant(PRJCT_CALIB_FWHM_TYPE_INVPOLY));
  m_lineShapeCombo->addItem("Voigt", QVariant(PRJCT_CALIB_FWHM_TYPE_VOIGT));
  topLayout->addWidget(m_lineShapeCombo, 2, 1);

  // degree of 2n-Lorentz
  m_degreeWidget = new QFrame(this);
  m_degreeWidget->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *degreeLayout = new QHBoxLayout(m_degreeWidget);
  degreeLayout->setMargin(0);
  degreeLayout->addWidget(new QLabel("Degree", this));
  m_degreeSpinBox = new QSpinBox(this);
  m_degreeSpinBox->setRange(0, 10);
  degreeLayout->addWidget(m_degreeSpinBox);
  topLayout->addWidget(m_degreeWidget, 2, 2);
  m_degreeWidget->hide(); // show when lineshape combo is cSpectralLineShapeLorentz

  // force some sizes to prevent 'jumpy' display.
  m_degreeSpinBox->setFixedHeight(m_lineShapeCombo->sizeHint().height());
  browseBtn->setFixedWidth(m_degreeWidget->sizeHint().width());

  mainLayout->addLayout(topLayout);

  mainLayout->addSpacing(5);

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
  m_shiftDegreeSpinBox->setRange(0,5);
  polyLayout->addWidget(m_shiftDegreeSpinBox, 0, 1);

  polyLayout->addWidget(new QLabel("SFP", polyGroup), 1, 0);
  m_sfpDegreeSpinBox = new QSpinBox(polyGroup);
  m_sfpDegreeSpinBox->setRange(0,5);
  polyLayout->addWidget(m_sfpDegreeSpinBox, 1, 1);

  groupLayout->addWidget(polyGroup);

  // window limts
  QGroupBox *windowGroup = new QGroupBox("Window Limits (nm)", this);
  QGridLayout *windowLayout = new QGridLayout(windowGroup);
  windowLayout->setMargin(5);
  windowLayout->setSpacing(3);

  windowLayout->addWidget(new QLabel("Min", windowGroup), 0, 0);
  m_lambdaMinEdit = new QLineEdit(windowGroup);
  m_lambdaMinEdit->setFixedWidth(50);
  m_lambdaMinEdit->setValidator(new CDoubleFixedFmtValidator(100.0, 900.0, 2, m_lambdaMinEdit));
  windowLayout->addWidget(m_lambdaMinEdit, 0, 1);

  windowLayout->addWidget(new QLabel("Max", windowGroup), 1, 0);
  m_lambdaMaxEdit = new QLineEdit(windowGroup);
  m_lambdaMaxEdit->setFixedWidth(50);
  m_lambdaMaxEdit->setValidator(new CDoubleFixedFmtValidator(100.0, 900.0, 2, m_lambdaMaxEdit));
  windowLayout->addWidget(m_lambdaMaxEdit, 1, 1);

  windowLayout->addWidget(new QLabel("Sub-windows", windowGroup), 2, 0);
  m_subWindowsSpinBox = new QSpinBox(this);
  m_subWindowsSpinBox->setFixedWidth(50);
  m_subWindowsSpinBox->setRange(1, 50);
  windowLayout->addWidget(m_subWindowsSpinBox, 2, 1);

  groupLayout->addWidget(windowGroup);

  mainLayout->addLayout(groupLayout);

  mainLayout->addSpacing(5);

  // fit paramters tables
  m_tabs = new QTabWidget(this);

  m_moleculesTab = new CWMoleculesDoasTable("Molecules", 120);
  m_tabs->addTab(m_moleculesTab, "Molecules");
  m_linearTab = new CWLinearParametersDoasTable("Linear Parameters", 120);
  m_tabs->addTab(m_linearTab, "Linear Parameters");
  m_sfpTab = new CWSfpParametersDoasTable("SFP Parameters", 120);
  m_tabs->addTab(m_sfpTab, "SFP Parameters");
  m_shiftAndStretchTab = new CWShiftAndStretchDoasTable("Cross sections and spectrum", 180);
  m_tabs->addTab(m_shiftAndStretchTab, "Shift and Stretch");
  m_gapTab = new CWGapDoasTable("Gaps", 240);
  m_tabs->addTab(m_gapTab, "Gaps");
  m_outputTab = new CWOutputDoasTable("Output", 120);
  m_tabs->addTab(m_outputTab, "Output");

  mainLayout->addWidget(m_tabs, 1);

  // tabel interconnections - MUST be connected before populating the tables ....
  connect(m_fitsCheck, SIGNAL(stateChanged(int)),  m_moleculesTab, SLOT(slotFitColumnCheckable(int)));
  connect(m_shiftAndStretchTab, SIGNAL(signalLockSymbol(const QString &, const QObject *)),
	  m_moleculesTab, SLOT(slotLockSymbol(const QString &, const QObject *)));
  connect(m_shiftAndStretchTab, SIGNAL(signalUnlockSymbol(const QString &, const QObject *)),
	  m_moleculesTab, SLOT(slotUnlockSymbol(const QString &, const QObject *)));
  connect(m_moleculesTab, SIGNAL(signalSymbolListChanged(const QStringList&)),
	  m_shiftAndStretchTab, SLOT(slotSymbolListChanged(const QStringList&)));
  connect(m_moleculesTab, SIGNAL(signalSymbolListChanged(const QStringList&)),
	  m_outputTab, SLOT(slotSymbolListChanged(const QStringList&)));

  // set the current values
  m_refFileEdit->setText(QString(properties->solarRefFile));

  index = m_methodCombo->findData(QVariant(properties->methodType));
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

  m_moleculesTab->populate(&(properties->crossSectionList));
  m_linearTab->populate(&(properties->linear));
  m_sfpTab->populate(&(properties->sfp[0]));
  m_shiftAndStretchTab->populate(&(properties->shiftStretchList));
  m_gapTab->populate(&(properties->gapList));
  m_outputTab->populate(&(properties->outputList));

  m_moleculesTab->slotFitColumnCheckable(m_fitsCheck->checkState());
  // disable the AMF tab
  m_moleculesTab->setColumnEnabled(2, false);

  // no AMF, residual or vertical output
  m_outputTab->setColumnEnabled(0, false);
  m_outputTab->setColumnEnabled(1, false);
  m_outputTab->setColumnEnabled(5, false);
  m_outputTab->setColumnEnabled(6, false);
  m_outputTab->setColumnEnabled(7, false);

  // connections
  connect(m_lineShapeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLineShapeSelectionChanged(int)));
  connect(browseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseSolarRefFile()));

}

CWProjectTabCalibration::~CWProjectTabCalibration()
{
}

void CWProjectTabCalibration::apply(mediate_project_calibration_t *properties) const
{
  // a safe text length is assured.
  strcpy(properties->solarRefFile, m_refFileEdit->text().toAscii().data());

  properties->methodType = m_methodCombo->itemData(m_methodCombo->currentIndex()).toInt();

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

  m_moleculesTab->apply(&(properties->crossSectionList));
  m_linearTab->apply(&(properties->linear));
  m_sfpTab->apply(&(properties->sfp[0]));
  m_shiftAndStretchTab->apply(&(properties->shiftStretchList));
  m_gapTab->apply(&(properties->gapList));
  m_outputTab->apply(&(properties->outputList));
}

void CWProjectTabCalibration::slotLineShapeSelectionChanged(int index)
{
  int tmp = m_lineShapeCombo->itemData(index).toInt();

  if (tmp == PRJCT_CALIB_FWHM_TYPE_INVPOLY)
    m_degreeWidget->show();
  else
    m_degreeWidget->hide();

  m_sfpDegreeSpinBox->setEnabled(tmp != PRJCT_CALIB_FWHM_TYPE_NONE);

}

void CWProjectTabCalibration::slotBrowseSolarRefFile()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Open Solar Reference File",
						  pref->directoryName("Ref"),
                                                  "Kurucz File (*.ktz);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Ref", filename);

    m_refFileEdit->setText(filename);
  }
}

void CWProjectTabCalibration::slotOutputCalibration(bool enabled)
{
  m_outputTab->setEnabled(enabled);
}


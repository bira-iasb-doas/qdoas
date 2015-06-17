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
#include "PolynomialTab.h"

#include "constants.h"

#include "debugutil.h"

CWProjectTabCalibration::CWProjectTabCalibration(const mediate_project_calibration_t *properties, QWidget *parent) :
  QFrame(parent)
{
  int index;
  QString tmpStr;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);   
  QLabel *methodLabel,*refLabel;

  QGridLayout *topLayout = new QGridLayout;

  mainLayout->addSpacing(5);     
  
  // ref file    
  topLayout->addWidget((refLabel=new QLabel("Solar Ref. File", this)), 0, 0);  
  m_refFileEdit  = new QLineEdit(this);
  m_refFileEdit->setMaxLength(sizeof(properties->solarRefFile) - 1); // limit test length to buffer size
  m_refBrowseBtn = new QPushButton("Browse", this);
  topLayout->addWidget(m_refFileEdit, 0, 1, 1, 2);
  topLayout->addWidget(m_refBrowseBtn, 0, 3);
  
  // methodType
  topLayout->addWidget((methodLabel=new QLabel("Analysis Method", this)), 1, 0);
  m_methodCombo = new QComboBox(this);
  m_methodCombo->addItem("Optical Density Fitting", QVariant(PRJCT_ANLYS_METHOD_SVD));
  m_methodCombo->addItem("Intensity fitting (Marquardt-Levenberg+SVD)", QVariant(PRJCT_ANLYS_METHOD_SVDMARQUARDT));
  topLayout->addWidget(m_methodCombo, 1, 1, 1, 3); // spans 3 columns

  // line shape
  topLayout->addWidget(new QLabel("Line Shape (SFP)", this), 2, 0);
  m_lineShapeCombo = new QComboBox(this);
  m_lineShapeCombo->addItem("Don't Fit", QVariant(PRJCT_CALIB_FWHM_TYPE_NONE));
  m_lineShapeCombo->addItem("File", QVariant(PRJCT_CALIB_FWHM_TYPE_FILE));
  m_lineShapeCombo->addItem("Gaussian", QVariant(PRJCT_CALIB_FWHM_TYPE_GAUSS));
  m_lineShapeCombo->addItem("Error Function", QVariant(PRJCT_CALIB_FWHM_TYPE_ERF));
  m_lineShapeCombo->addItem("2n-Lorentz", QVariant(PRJCT_CALIB_FWHM_TYPE_INVPOLY));
  m_lineShapeCombo->addItem("Voigt", QVariant(PRJCT_CALIB_FWHM_TYPE_VOIGT));
  m_lineShapeCombo->addItem("Asymmetric Gaussian", QVariant(PRJCT_CALIB_FWHM_TYPE_AGAUSS));
  topLayout->addWidget(m_lineShapeCombo, 2, 1);

  // File
  m_slfFileEdit  = new QLineEdit(this);
  QPushButton *fileBrowseBtn = new QPushButton("Browse", this);
  m_fileWidget = new QFrame(this);
  m_fileWidget->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *fileLayout = new QHBoxLayout(m_fileWidget);
  fileLayout->setMargin(0);
  fileLayout->addWidget(m_slfFileEdit);
  fileLayout->addWidget(fileBrowseBtn);

  topLayout->addWidget(m_fileWidget, 2, 2, 1, 2);
  m_fileWidget->hide(); // show when lineshape combo is File

  // order of 2n-Lorentz
  m_orderWidget = new QFrame(this);
  m_orderWidget->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *orderLayout = new QHBoxLayout(m_orderWidget);
  orderLayout->setMargin(0);
  orderLayout->setAlignment(Qt::AlignLeft);
  orderLayout->addWidget(new QLabel("Order", this));
  m_orderSpinBox = new QSpinBox(this);
  m_orderSpinBox->setRange(1, 10); 
  orderLayout->addWidget(m_orderSpinBox);
  topLayout->addWidget(m_orderWidget, 2, 2, 1, 2);
  m_orderWidget->hide(); // show when lineshape combo is cSpectralLineShapeLorentz

  topLayout->setColumnStretch(0, 0);
  topLayout->setColumnStretch(1, 0);
  topLayout->setColumnStretch(2, 1);
  topLayout->setColumnStretch(3, 0);

  // force some sizes to prevent 'jumpy' display.
  m_orderSpinBox->setFixedHeight(m_lineShapeCombo->sizeHint().height());
  m_slfFileEdit->setFixedHeight(m_lineShapeCombo->sizeHint().height());

  mainLayout->addLayout(topLayout);

  // middle
  QHBoxLayout *groupLayout = new QHBoxLayout;
  groupLayout->setMargin(0);
  groupLayout->setSpacing(5);

  // display
  QGroupBox *displayGroup = new QGroupBox("Display", this);
  QGridLayout *displayLayout = new QGridLayout(displayGroup);
  displayLayout->setMargin(3);
  displayLayout->setSpacing(0);

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
  polyLayout->setMargin(3);
  polyLayout->setAlignment(Qt::AlignLeft);

  polyLayout->addWidget(new QLabel("Shift", polyGroup), 0, 0, Qt::AlignRight);
  m_shiftDegreeSpinBox = new QSpinBox(polyGroup);
  m_shiftDegreeSpinBox->setRange(0,5);
  m_shiftDegreeSpinBox->setFixedWidth(50);
  polyLayout->addWidget(m_shiftDegreeSpinBox, 0, 1, Qt::AlignLeft);

  polyLayout->addWidget(new QLabel("SFP", polyGroup), 1, 0, Qt::AlignRight);
  m_sfpDegreeSpinBox = new QSpinBox(polyGroup);
  m_sfpDegreeSpinBox->setRange(0,5);
  m_sfpDegreeSpinBox->setFixedWidth(50);
  polyLayout->addWidget(m_sfpDegreeSpinBox, 1, 1, Qt::AlignLeft);

  groupLayout->addWidget(polyGroup);

  // window limits
  QGroupBox *windowGroup = new QGroupBox("Window Limits (nm)", this);
  QGridLayout *windowLayout = new QGridLayout(windowGroup);
  windowLayout->setMargin(5);
  windowLayout->setAlignment(Qt::AlignLeft);

  windowLayout->addWidget(new QLabel("Min", windowGroup), 0, 0, Qt::AlignRight);
  m_lambdaMinEdit = new QLineEdit(windowGroup);
  //  m_lambdaMinEdit->setFixedWidth(50);
  m_lambdaMinEdit->setValidator(new CDoubleFixedFmtValidator(100.0, 900.0, 2, m_lambdaMinEdit));
  windowLayout->addWidget(m_lambdaMinEdit, 0, 1, Qt::AlignLeft);

  windowLayout->addWidget(new QLabel("Max", windowGroup), 0, 2, Qt::AlignRight);
  m_lambdaMaxEdit = new QLineEdit(windowGroup);
  //  m_lambdaMaxEdit->setFixedWidth(50);
  m_lambdaMaxEdit->setValidator(new CDoubleFixedFmtValidator(100.0, 900.0, 2, m_lambdaMaxEdit));
  windowLayout->addWidget(m_lambdaMaxEdit, 0, 3, Qt::AlignLeft);

  windowLayout->addWidget(new QLabel("Sub-windows", windowGroup), 1, 0, 1, 3, Qt::AlignRight);
  m_subWindowsSpinBox = new QSpinBox(this);
  m_subWindowsSpinBox->setFixedWidth(50);
  m_subWindowsSpinBox->setRange(1, 50);
  windowLayout->addWidget(m_subWindowsSpinBox, 1, 3, Qt::AlignLeft);

  groupLayout->addWidget(windowGroup);

  mainLayout->addLayout(groupLayout);

  // fit paramters tables
  m_tabs = new QTabWidget(this);

  m_moleculesTab = new CWMoleculesDoasTable("Molecules", 120);
  m_tabs->addTab(m_moleculesTab, "Molecules");
  m_linearTab = new PolynomialTab;
  m_tabs->addTab(m_linearTab, "Linear Parameters");
  m_sfpTab = new CWSfpParametersDoasTable("SFP Parameters");
  m_tabs->addTab(m_sfpTab, "SFP Parameters");
  m_shiftAndStretchTab = new CWShiftAndStretchDoasTable("Cross sections and spectrum");
  m_tabs->addTab(m_shiftAndStretchTab, "Shift and Stretch");
  m_gapTab = new CWGapDoasTable("Gaps");
  m_tabs->addTab(m_gapTab, "Gaps");
  m_outputTab = new CWOutputDoasTable("Output");
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
  m_slfFileEdit->setText(QString(properties->slfFile));

  index = m_methodCombo->findData(QVariant(properties->methodType));
  if (index != -1)
    m_methodCombo->setCurrentIndex(index);

  m_orderSpinBox->setValue(properties->lorentzOrder);

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
  connect(m_refBrowseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseSolarRefFile()));
  connect(fileBrowseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseSlfFile()));

}

void CWProjectTabCalibration::apply(mediate_project_calibration_t *properties) const
{
  // a safe text length is assured.
  strcpy(properties->solarRefFile, m_refFileEdit->text().toAscii().data());
  strcpy(properties->slfFile, m_slfFileEdit->text().toAscii().data());

  properties->methodType = m_methodCombo->itemData(m_methodCombo->currentIndex()).toInt();

  properties->lineShape = m_lineShapeCombo->itemData(m_lineShapeCombo->currentIndex()).toInt();
  properties->lorentzOrder = m_orderSpinBox->value();

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

  if (tmp == PRJCT_CALIB_FWHM_TYPE_FILE)
    m_fileWidget->show();
  else
    m_fileWidget->hide();

  if (tmp == PRJCT_CALIB_FWHM_TYPE_INVPOLY)
    m_orderWidget->show();
  else
    m_orderWidget->hide();
    
  if (tmp==PRJCT_CALIB_FWHM_TYPE_NONE) {
    m_refFileEdit->setEnabled(false);
    m_refBrowseBtn->setEnabled(false);
  } else {
    m_refFileEdit->setEnabled(true);
    m_refBrowseBtn->setEnabled(true);
  }
   
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

void CWProjectTabCalibration::slotBrowseSlfFile()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Open Slit Function File",
						  pref->directoryName("Slf"),
                                                  "Slit function File (*.slf);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Slf", filename);

    m_slfFileEdit->setText(filename);
  }
}

void CWProjectTabCalibration::slotOutputCalibration(bool enabled)
{
  m_outputTab->setEnabled(enabled);
}


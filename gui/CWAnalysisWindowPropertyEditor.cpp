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
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QIntValidator>
#include <QFileDialog>

#include "CWorkSpace.h"
#include "CWAnalysisWindowPropertyEditor.h"
#include "CValidator.h"

#include "CDoasTable.h"

CWAnalysisWindowPropertyEditor::CWAnalysisWindowPropertyEditor(const QString &projectName,
							       const QString &analysisWindowName,
							       QWidget *parent) :
  CWEditor(parent),
  m_projectName(projectName),
  m_analysisWindowName(analysisWindowName)
{
  mediate_analysis_window_t *awData = CWorkSpace::instance()->findAnalysisWindow(m_projectName, m_analysisWindowName);
  
  if (!awData)
    return; // TODO - assert or throw

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(25);
  mainLayout->setSpacing(5);

  QHBoxLayout *topLayout = new QHBoxLayout;

  // calibration combo ... group
  QGroupBox *calibrationGroup = new QGroupBox("Wavelength Calibration", this);
  QVBoxLayout *calibrationLayout = new QVBoxLayout(calibrationGroup);
  calibrationLayout->setMargin(3);
  calibrationLayout->setSpacing(0);
  m_calibrationCombo = new QComboBox(calibrationGroup);
  m_calibrationCombo->addItem("None");
  m_calibrationCombo->addItem("Ref Only");
  m_calibrationCombo->addItem("Spectra Only");
  m_calibrationCombo->addItem("Ref + Spectra");
  calibrationLayout->addWidget(m_calibrationCombo);
  calibrationLayout->addStretch(1);

  topLayout->addWidget(calibrationGroup);

  // radio button - ref selection
  QGroupBox *refSelectGroup = new QGroupBox("Ref. Selection", this);
  QVBoxLayout *refSelectLayout = new QVBoxLayout(refSelectGroup);
  refSelectLayout->setMargin(3);
  refSelectLayout->setSpacing(0);
  QRadioButton *autoButton = new QRadioButton("Automatic",  refSelectGroup);
  refSelectLayout->addWidget(autoButton);
  QRadioButton *fileButton = new QRadioButton("File",  refSelectGroup);
  refSelectLayout->addWidget(fileButton);
  
  topLayout->addWidget(refSelectGroup);

  // fitting interval
  QGroupBox *fitIntervalGroup = new QGroupBox("Fitting Interval", this);
  QGridLayout *fitIntervalLayout = new QGridLayout(fitIntervalGroup);
  fitIntervalLayout->setMargin(3);
  fitIntervalLayout->setSpacing(0);
  fitIntervalLayout->addWidget(new QLabel("Min", fitIntervalGroup), 0, 0);
  m_fitMinEdit = new QLineEdit(fitIntervalGroup);
  m_fitMinEdit->setFixedWidth(50);
  m_fitMinEdit->setValidator(new QIntValidator(100, 999, m_fitMinEdit));
  fitIntervalLayout->addWidget(m_fitMinEdit, 0, 1);
  fitIntervalLayout->addWidget(new QLabel("Max", fitIntervalGroup), 1, 0);
  m_fitMaxEdit = new QLineEdit(fitIntervalGroup);
  m_fitMaxEdit->setFixedWidth(50);
  m_fitMaxEdit->setValidator(new QIntValidator(100, 999, m_fitMaxEdit));
  fitIntervalLayout->addWidget(m_fitMaxEdit, 1, 1);

  topLayout->addWidget(fitIntervalGroup);
  
  // display
  QGroupBox *displayGroup = new QGroupBox("Display", this);
  QGridLayout *displayLayout = new QGridLayout(displayGroup);
  displayLayout->setMargin(3);
  displayLayout->setSpacing(0);
  m_spectrumCheck = new QCheckBox("Spectrum", displayGroup);
  displayLayout->addWidget(m_spectrumCheck, 0, 0);
  m_polyCheck = new QCheckBox("Polyn.", displayGroup);
  displayLayout->addWidget(m_polyCheck, 0, 1);
  m_fitsCheck = new QCheckBox("Fits", displayGroup);
  displayLayout->addWidget(m_fitsCheck, 0, 2);
  m_residualCheck = new QCheckBox("Residual", displayGroup);
  displayLayout->addWidget(m_residualCheck, 1, 0);
  m_predefCheck = new QCheckBox("Predef.", displayGroup);
  displayLayout->addWidget(m_predefCheck, 1, 1);
  m_ratioCheck = new QCheckBox("Ref1/Ref2", displayGroup);
  displayLayout->addWidget(m_ratioCheck, 1, 2);
  
  topLayout->addWidget(displayGroup);
  topLayout->addStretch(1);

  mainLayout->addLayout(topLayout);

  // files
  QGroupBox *filesGroup = new QGroupBox("Files", this);
  QVBoxLayout *filesLayout = new QVBoxLayout(filesGroup);
  filesLayout->setMargin(3);
  filesLayout->setSpacing(0);

  // row 0 - Ref1
  m_refOneFrame = new QFrame(filesGroup);
  m_refOneFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *refOneFrameLayout = new QHBoxLayout(m_refOneFrame);
  refOneFrameLayout->setMargin(0);
  QLabel *labelOne = new QLabel(" Reference 1 ", m_refOneFrame);
  labelOne->setFixedWidth(85);
  m_refOneEdit = new QLineEdit(m_refOneFrame);
  QPushButton *refOneBrowseBtn = new QPushButton("Browse", m_refOneFrame);
  refOneBrowseBtn->setFixedWidth(70);
  refOneFrameLayout->addWidget(labelOne);
  refOneFrameLayout->addWidget(m_refOneEdit, 1);
  refOneFrameLayout->addWidget(refOneBrowseBtn);

  filesLayout->addWidget(m_refOneFrame);

  // row 1 - Ref2
  m_refTwoEditFrame = new QFrame(filesGroup);
  m_refTwoEditFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *refTwoEditFrameLayout = new QHBoxLayout(m_refTwoEditFrame);
  refTwoEditFrameLayout->setMargin(0);
  QLabel *labelTwoEdit = new QLabel(" Reference 2 ", m_refTwoEditFrame);
  labelTwoEdit->setFixedWidth(85);
  m_refTwoEdit = new QLineEdit(m_refTwoEditFrame);
  QPushButton *refTwoBrowseBtn = new QPushButton("Browse", m_refTwoEditFrame);
  refTwoBrowseBtn->setFixedWidth(70);
  refTwoEditFrameLayout->addWidget(labelTwoEdit);
  refTwoEditFrameLayout->addWidget(m_refTwoEdit, 1);
  refTwoEditFrameLayout->addWidget(refTwoBrowseBtn);

  m_refTwoSzaFrame = new QFrame(filesGroup);
  m_refTwoSzaFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *szaLayout = new QHBoxLayout(m_refTwoSzaFrame);
  szaLayout->setMargin(0);
  QLabel *labelTwoSza = new QLabel(" Reference 2    SZA ", m_refTwoSzaFrame);
  labelTwoSza->setFixedWidth(120);
  m_szaValueEdit = new QLineEdit(m_refTwoSzaFrame);
  m_szaValueEdit->setFixedWidth(50);
  m_szaValueEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 180.0, 3, m_szaValueEdit));
  m_szaMarginEdit = new QLineEdit(m_refTwoSzaFrame);
  m_szaMarginEdit->setFixedWidth(50);
  m_szaMarginEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 180.0, 3, m_szaMarginEdit));
  szaLayout->addWidget(labelTwoSza);
  szaLayout->addWidget(m_szaValueEdit);
  szaLayout->addWidget(new QLabel("+/-", m_refTwoSzaFrame));
  szaLayout->addWidget(m_szaMarginEdit);
  szaLayout->addStretch(1);

  // stack for ref 2 switching ...
  m_refTwoStack = new QStackedLayout;
  m_refTwoStack->setMargin(0);
  m_refTwoStack->addWidget(m_refTwoEditFrame); // takes index 0
  m_refTwoStack->addWidget(m_refTwoSzaFrame);  // takes index 1
  filesLayout->addLayout(m_refTwoStack);

  // row 2 - residual
  QFrame *residualFrame = new QFrame(filesGroup);
  residualFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *residualFrameLayout = new QHBoxLayout(residualFrame);
  residualFrameLayout->setMargin(0);
  QLabel *labelRes = new QLabel(" Residual ", residualFrame);
  labelRes->setFixedWidth(85);
  m_residualEdit = new QLineEdit(residualFrame);
  QPushButton *residualBrowseBtn = new QPushButton("Browse", residualFrame);
  residualBrowseBtn->setFixedWidth(70);
  residualFrameLayout->addWidget(labelRes);
  residualFrameLayout->addWidget(m_residualEdit, 1);
  residualFrameLayout->addWidget(residualBrowseBtn);

  filesLayout->addWidget(residualFrame);
  
  mainLayout->addWidget(filesGroup);
  
  // table tabs
  m_tabs = new QTabWidget(this);

  CDoasTable *dummy = new CDoasTable("Fred");
  dummy->createColumnCheck("test", 55);
  dummy->createColumnCheck("this", 85);
  dummy->createColumnCheck("thing", 120);
 
  m_tabs->addTab(dummy, "Molecules");

  mainLayout->addWidget(m_tabs, 1);

  // caption string and context tag
  m_captionStr = "Properties of Analysis Window : ";
  m_captionStr += m_projectName;
  m_captionStr += " / ";
  m_captionStr += m_analysisWindowName;
  
  m_contextTag = m_projectName;
  m_contextTag += ":";
  m_contextTag += m_analysisWindowName;
  m_contextTag += " Prop";

  // set the current values
  autoButton->setChecked(true);
  slotRefSelectionChanged(true);

  m_calibrationCombo->setCurrentIndex(0);
  slotWavelengthCalibrationChanged(0);

  // slot on automatic button is sufficient
  connect(autoButton, SIGNAL(toggled(bool)), this, SLOT(slotRefSelectionChanged(bool)));
  connect(m_calibrationCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotWavelengthCalibrationChanged(int)));
  connect(refOneBrowseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseRefOne()));
  connect(refTwoBrowseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseRefTwo()));
  connect(residualBrowseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseResidual()));

  notifyAcceptActionOk(true);
}

CWAnalysisWindowPropertyEditor::~CWAnalysisWindowPropertyEditor()
{
}

bool CWAnalysisWindowPropertyEditor::actionOk(void)
{
  // call apply for all tabs ...
  mediate_analysis_window_t *awData = CWorkSpace::instance()->findAnalysisWindow(m_projectName, m_analysisWindowName);
  
  if (awData) {

    return true;
  }

  // Project not found ... TODO

  return false;
}

void CWAnalysisWindowPropertyEditor::actionHelp(void)
{
}

void CWAnalysisWindowPropertyEditor::slotRefSelectionChanged(bool checked)
{
  m_refTwoStack->setCurrentIndex(checked ? 1 : 0);
}

void CWAnalysisWindowPropertyEditor::slotWavelengthCalibrationChanged(int index)
{
  switch (index) {
  case 0: // nothing
  case 1: // ref only
    {
      m_refOneFrame->setEnabled(true);
      m_refTwoEditFrame->setEnabled(true);
      m_refTwoSzaFrame->setEnabled(true);
    }
    break;
  case 2: // spectrum only
    {
      m_refOneFrame->setEnabled(false);
      m_refTwoEditFrame->setEnabled(false);
      m_refTwoSzaFrame->setEnabled(false);
    }
    break;
  case 3: // ref + spectrum
    {
      m_refOneFrame->setEnabled(false);      
      m_refTwoEditFrame->setEnabled(true);
      m_refTwoSzaFrame->setEnabled(true);
    }
    break;
  }
}

void CWAnalysisWindowPropertyEditor::slotBrowseRefOne()
{
  QString file = QFileDialog::getOpenFileName(this, "Select Reference 1", QString(),
					      "Kurucz File(*.ktz);;All Files (*)");

  if (!file.isEmpty())
    m_refOneEdit->setText(file);
}

void CWAnalysisWindowPropertyEditor::slotBrowseRefTwo()
{
  QString file = QFileDialog::getOpenFileName(this, "Select Reference 2", QString(),
					      "Kurucz File(*.ktz);;All Files (*)");

  if (!file.isEmpty())
    m_refTwoEdit->setText(file);
}

void CWAnalysisWindowPropertyEditor::slotBrowseResidual()
{
  QString file = QFileDialog::getOpenFileName(this, "Select Residual", QString(),
					      "All Files (*)");
  
  if (!file.isEmpty())
    m_residualEdit->setText(file);
}

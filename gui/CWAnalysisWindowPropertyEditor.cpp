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

#include "constants.h"

CWAnalysisWindowPropertyEditor::CWAnalysisWindowPropertyEditor(const QString &projectName,
							       const QString &analysisWindowName,
							       QWidget *parent) :
  CWEditor(parent),
  m_projectName(projectName),
  m_analysisWindowName(analysisWindowName)
{
  mediate_analysis_window_t *d = CWorkSpace::instance()->findAnalysisWindow(m_projectName, m_analysisWindowName);
  
  if (!d)
    return; // TODO - assert or throw

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(10);
  mainLayout->setSpacing(5);

  QHBoxLayout *topLayout = new QHBoxLayout;

  // calibration combo ... group
  QGroupBox *calibrationGroup = new QGroupBox("Wavelength Calibration", this);
  QVBoxLayout *calibrationLayout = new QVBoxLayout(calibrationGroup);
  calibrationLayout->setMargin(3);
  calibrationLayout->setSpacing(0);
  m_calibrationCombo = new QComboBox(calibrationGroup);
  m_calibrationCombo->addItem("None", QVariant(ANLYS_KURUCZ_NONE));
  m_calibrationCombo->addItem("Ref Only", QVariant(ANLYS_KURUCZ_REF));
  m_calibrationCombo->addItem("Spectra Only", QVariant(ANLYS_KURUCZ_SPEC));
  m_calibrationCombo->addItem("Ref + Spectra", QVariant(ANLYS_KURUCZ_REF_AND_SPEC));
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
  m_refOneEdit->setMaxLength(sizeof(d->refOneFile)-1);
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
  m_refTwoEdit->setMaxLength(sizeof(d->refTwoFile)-1);
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
  m_szaCenterEdit = new QLineEdit(m_refTwoSzaFrame);
  m_szaCenterEdit->setFixedWidth(50);
  m_szaCenterEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 180.0, 3, m_szaCenterEdit));
  m_szaDeltaEdit = new QLineEdit(m_refTwoSzaFrame);
  m_szaDeltaEdit->setFixedWidth(50);
  m_szaDeltaEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 180.0, 3, m_szaDeltaEdit));
  szaLayout->addWidget(labelTwoSza);
  szaLayout->addWidget(m_szaCenterEdit);
  szaLayout->addWidget(new QLabel("+/-", m_refTwoSzaFrame));
  szaLayout->addWidget(m_szaDeltaEdit);
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
  m_residualEdit->setMaxLength(sizeof(d->residualFile)-1);
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
  int index;
  QString tmpStr;

  autoButton->setChecked(d->refSpectrumSelection == ANLYS_REF_SELECTION_MODE_AUTOMATIC);
  slotRefSelectionChanged(d->refSpectrumSelection == ANLYS_REF_SELECTION_MODE_AUTOMATIC);

  index = m_calibrationCombo->findData(d->kuruczMode);
  if (index != -1) {
    m_calibrationCombo->setCurrentIndex(index);
    slotWavelengthCalibrationChanged(index);
  }

  m_fitMinEdit->setText(tmpStr.setNum(d->fitMinWavelength));
  m_fitMaxEdit->setText(tmpStr.setNum(d->fitMaxWavelength));

  m_spectrumCheck->setChecked(d->requireSpectrum ? Qt::Checked : Qt::Unchecked);
  m_polyCheck->setChecked(d->requirePolynomial ? Qt::Checked : Qt::Unchecked);
  m_fitsCheck->setChecked(d->requireFit ? Qt::Checked : Qt::Unchecked);
  m_residualCheck->setChecked(d->requireResidual ? Qt::Checked : Qt::Unchecked);
  m_predefCheck->setChecked(d->requirePredefined ? Qt::Checked : Qt::Unchecked);
  m_ratioCheck->setChecked(d->requireRefRatio ? Qt::Checked : Qt::Unchecked);

  m_refOneEdit->setText(d->refOneFile);
  m_refTwoEdit->setText(d->refTwoFile);
  m_residualEdit->setText(d->residualFile);

  m_szaCenterEdit->validator()->fixup(tmpStr.setNum(d->refSzaCenter));
  m_szaCenterEdit->setText(tmpStr);
  m_szaDeltaEdit->validator()->fixup(tmpStr.setNum(d->refSzaDelta));
  m_szaDeltaEdit->setText(tmpStr);

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
  mediate_analysis_window_t *d = CWorkSpace::instance()->findAnalysisWindow(m_projectName, m_analysisWindowName);
  
  if (d) {

    d->kuruczMode = m_calibrationCombo->itemData(m_calibrationCombo->currentIndex()).toInt();
    d->refSpectrumSelection = m_refTwoStack->currentIndex() ? ANLYS_REF_SELECTION_MODE_FILE : ANLYS_REF_SELECTION_MODE_AUTOMATIC;

    d->fitMinWavelength = m_fitMinEdit->text().toInt();
    d->fitMaxWavelength = m_fitMaxEdit->text().toInt();

    d->requireSpectrum = (m_spectrumCheck->checkState() == Qt::Checked) ? 1 : 0;
    d->requirePolynomial = (m_polyCheck->checkState() == Qt::Checked) ? 1 : 0;
    d->requireFit = (m_fitsCheck->checkState() == Qt::Checked) ? 1 : 0;
    d->requireResidual = (m_residualCheck->checkState() == Qt::Checked) ? 1 : 0;
    d->requirePredefined = (m_predefCheck->checkState() == Qt::Checked) ? 1 : 0;
    d->requireRefRatio = (m_ratioCheck->checkState() == Qt::Checked) ? 1 : 0;
    
    strcpy(d->refOneFile, m_refOneEdit->text().toAscii().data());
    strcpy(d->refTwoFile, m_refTwoEdit->text().toAscii().data());
    strcpy(d->residualFile, m_residualEdit->text().toAscii().data());

    d->refSzaCenter = m_szaCenterEdit->text().toDouble();
    d->refSzaDelta = m_szaDeltaEdit->text().toDouble();

    // call apply for all tabs ...

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
  int mode = m_calibrationCombo->itemData(index).toInt();

  switch (mode) {
  case ANLYS_KURUCZ_NONE:
  case ANLYS_KURUCZ_REF:
    {
      m_refOneFrame->setEnabled(true);
      m_refTwoEditFrame->setEnabled(true);
      m_refTwoSzaFrame->setEnabled(true);
    }
    break;
  case ANLYS_KURUCZ_SPEC:
    {
      m_refOneFrame->setEnabled(false);
      m_refTwoEditFrame->setEnabled(false);
      m_refTwoSzaFrame->setEnabled(false);
    }
    break;
  case ANLYS_KURUCZ_REF_AND_SPEC:
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

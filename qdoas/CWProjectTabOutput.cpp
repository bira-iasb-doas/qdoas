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


#include <cstring>

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>

#include "CWProjectTabOutput.h"
#include "CWOutputSelector.h"
#include "CPreferences.h"

#include "constants.h"

#include "debugutil.h"


CWProjectTabOutput::CWProjectTabOutput(const mediate_project_output_t *properties, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  mainLayout->addSpacing(25);
  
  // Output Path
  m_pathFrame = new QFrame(this);
  m_pathFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *pathLayout = new QHBoxLayout(m_pathFrame);

  pathLayout->addWidget(new QLabel("Output Path", m_pathFrame));

  m_pathEdit = new QLineEdit(m_pathFrame);
  m_pathEdit->setMaxLength(sizeof(properties->path)-1);
  pathLayout->addWidget(m_pathEdit, 1);

  QPushButton *browseBtn = new QPushButton("Browse", m_pathFrame);
  pathLayout->addWidget(browseBtn);

  mainLayout->addWidget(m_pathFrame);

  mainLayout->addSpacing(5);

  // checkboxes and edits
  QHBoxLayout *middleLayout = new QHBoxLayout;

  // check boxes
  QGroupBox *checkGroup = new QGroupBox("Options", this);
  QGridLayout *checkLayout = new QGridLayout(checkGroup);

  m_analysisCheck = new QCheckBox("Analysis");
  checkLayout->addWidget(m_analysisCheck, 0, 0);

  m_calibrationCheck = new QCheckBox("Calibration");
  checkLayout->addWidget(m_calibrationCheck, 1, 0);

  m_configurationCheck = new QCheckBox("Configuration");
  checkLayout->addWidget(m_configurationCheck, 0, 1);

  m_binaryCheck = new QCheckBox("Binary format");
  checkLayout->addWidget(m_binaryCheck, 1, 1);

  m_directoryCheck = new QCheckBox("Directories");
  checkLayout->addWidget(m_directoryCheck, 2, 1);

  middleLayout->addWidget(checkGroup);

  // edits
  m_editGroup = new QGroupBox("Flux / Colour Index", this);
  QGridLayout *editLayout = new QGridLayout(m_editGroup);

  editLayout->addWidget(new QLabel("Fluxes"), 0, 0);
  m_fluxEdit = new QLineEdit(this);
  m_fluxEdit->setMaxLength(sizeof(properties->flux)-1);
  editLayout->addWidget(m_fluxEdit, 0, 1);
  
  editLayout->addWidget(new QLabel("Cic"), 1, 0);
  m_colourIndexEdit = new QLineEdit(this);
  m_colourIndexEdit->setMaxLength(sizeof(properties->colourIndex)-1);
  editLayout->addWidget(m_colourIndexEdit, 1, 1);

  middleLayout->addWidget(m_editGroup);

  mainLayout->addLayout(middleLayout);

  mainLayout->addSpacing(5);

  m_selector = new CWOutputSelector(&(properties->selection), this);
  mainLayout->addWidget(m_selector);

  // initialize ...
  m_pathEdit->setText(QString(properties->path));

  m_analysisCheck->setCheckState(properties->analysisFlag ? Qt::Checked : Qt::Unchecked);
  m_calibrationCheck->setCheckState(properties->calibrationFlag ? Qt::Checked : Qt::Unchecked);
  m_configurationCheck->setCheckState(properties->configurationFlag ? Qt::Checked : Qt::Unchecked);
  m_binaryCheck->setCheckState(properties->binaryFormatFlag ? Qt::Checked : Qt::Unchecked);
  m_directoryCheck->setCheckState(properties->directoryFlag ? Qt::Checked : Qt::Unchecked);

  m_fluxEdit->setText(QString(properties->flux));
  m_colourIndexEdit->setText(QString(properties->colourIndex));

  setComponentsEnabled((m_analysisCheck->checkState() == Qt::Checked), (m_calibrationCheck->checkState() == Qt::Checked));

  // connections
  connect(m_analysisCheck, SIGNAL(stateChanged(int)), this, SLOT(slotAnalysisCheckChanged(int)));
  connect(m_calibrationCheck, SIGNAL(stateChanged(int)), this, SLOT(slotCalibrationCheckChanged(int)));
  connect(browseBtn, SIGNAL(clicked()), this, SLOT(slotBrowsePath()));
}

CWProjectTabOutput::~CWProjectTabOutput()
{
}

void CWProjectTabOutput::apply(mediate_project_output_t *properties) const
{
  properties->analysisFlag = (m_analysisCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->calibrationFlag = (m_calibrationCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->configurationFlag = (m_configurationCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->binaryFormatFlag = (m_binaryCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->directoryFlag = (m_directoryCheck->checkState() == Qt::Checked) ? 1 : 0;

  strcpy(properties->flux, m_fluxEdit->text().toAscii().data());
  strcpy(properties->colourIndex, m_colourIndexEdit->text().toAscii().data());
  strcpy(properties->path, m_pathEdit->text().toAscii().data());

  m_selector->apply(&(properties->selection));
}

void CWProjectTabOutput::slotBrowsePath()
{
  CPreferences *pref = CPreferences::instance();

  QString fileName = QFileDialog::getSaveFileName(this, "Select Output Path", pref->directoryName("Output"),
						  "All Files (*)");

  if (!fileName.isEmpty()) {
    // save it again
    pref->setDirectoryNameGivenFile("Output", fileName);
    
    m_pathEdit->setText(fileName);
  }
}

void CWProjectTabOutput::slotInstrumentChanged(int instrument)
{
  m_selector->setInstrument(instrument);
}

void CWProjectTabOutput::slotAnalysisCheckChanged(int state)
{
  setComponentsEnabled((state == Qt::Checked), (m_calibrationCheck->checkState() == Qt::Checked));
}

void CWProjectTabOutput::slotCalibrationCheckChanged(int state)
{
  setComponentsEnabled((m_analysisCheck->checkState() == Qt::Checked), (state == Qt::Checked));

  emit signalOutputCalibration((state == Qt::Checked));
}

void CWProjectTabOutput::setComponentsEnabled(bool analysisEnabled, bool calibrationEnabled)
{
  bool allEnabled = (analysisEnabled || calibrationEnabled);

  m_configurationCheck->setEnabled(allEnabled);
  m_binaryCheck->setEnabled(allEnabled);
  m_directoryCheck->setEnabled(allEnabled);

  m_pathFrame->setEnabled(allEnabled);

  m_editGroup->setEnabled(analysisEnabled);
  m_selector->setEnabled(analysisEnabled);
}




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
#include <iostream>

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
#include "output_formats.h"

#include "debugutil.h"


CWProjectTabOutput::CWProjectTabOutput(const mediate_project_output_t *properties, QWidget *parent) :
  QFrame(parent)
{
  // main layout: VBox
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addSpacing(25);

  // Output file/format widgets:
  m_pathFrame = new QFrame(this);
  m_pathFrame->setFrameStyle(QFrame::NoFrame);

  m_pathEdit = new QLineEdit(m_pathFrame);
  m_pathEdit->setMaxLength(sizeof(properties->path)-1);

  QPushButton *browseBtn = new QPushButton("Browse", m_pathFrame);

  m_selectFileFormat = new QComboBox(m_pathFrame);
  // fill combobox with available output formats.  The index of each
  // item will correspond to the enum value of the output format.
  for(int i=0; i<=LAST_OUTPUT_FORMAT; i++) {
    m_selectFileFormat->addItem(output_file_extensions[i]);
  }

  QFrame *swathFrame = new QFrame(m_pathFrame);
  swathFrame->setFrameStyle(QFrame::NoFrame);
  m_swathNameEdit = new QLineEdit("QDOAS_results", swathFrame);
  QRegExp validSwathName("[^/]{1,255}"); // Swath name may not contain "/" and can be 1 to 255 characters long.
  m_swathNameEdit->setValidator(new QRegExpValidator(validSwathName, m_swathNameEdit));

  // Layout for output file/format widgets:
  QHBoxLayout *pathLayout = new QHBoxLayout();
  pathLayout->addWidget(new QLabel("Output Path", m_pathFrame),0);
  pathLayout->addWidget(m_pathEdit, 1);
  pathLayout->addWidget(browseBtn);
  pathLayout->addWidget(new QLabel("File Format", m_pathFrame),0);
  pathLayout->addWidget(m_selectFileFormat, 0);

  QVBoxLayout *outputFileLayout = new QVBoxLayout(m_pathFrame);
  outputFileLayout->addLayout(pathLayout, 0);
  outputFileLayout->addWidget(swathFrame);

  QHBoxLayout *swathLayout = new QHBoxLayout(swathFrame);
  swathLayout->setContentsMargins(0, 0, 0, 0);
  swathLayout->addWidget(new QLabel("HDF-EOS Swath Name", swathFrame),0);
  swathLayout->addWidget(m_swathNameEdit, 1);

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

  m_referenceCheck = new QCheckBox("Automatic reference");
  checkLayout->addWidget(m_referenceCheck, 2, 0);

  m_directoryCheck = new QCheckBox("Directories");
  checkLayout->addWidget(m_directoryCheck, 0, 1);

  m_useFileName = new QCheckBox("Use file name");
  checkLayout->addWidget(m_useFileName, 1, 1);

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

  m_swathNameEdit->setText(properties->swath_name);

  m_selectFileFormat->setCurrentIndex(properties->file_format);
  swathFrame->setVisible(properties->file_format == HDFEOS5); // swath frame is hidden unless HDF-EOS5 output is selected

  m_analysisCheck->setCheckState(properties->analysisFlag ? Qt::Checked : Qt::Unchecked);
  m_calibrationCheck->setCheckState(properties->calibrationFlag ? Qt::Checked : Qt::Unchecked);
  m_referenceCheck->setCheckState(properties->referenceFlag ? Qt::Checked : Qt::Unchecked);
  m_directoryCheck->setCheckState(properties->directoryFlag ? Qt::Checked : Qt::Unchecked);
  m_useFileName->setCheckState(properties->filenameFlag ? Qt::Checked : Qt::Unchecked);

  m_fluxEdit->setText(QString(properties->flux));
  m_colourIndexEdit->setText(QString(properties->colourIndex));

  setComponentsEnabled((m_analysisCheck->checkState() == Qt::Checked),
                       (m_calibrationCheck->checkState() == Qt::Checked) || (m_referenceCheck->checkState() == Qt::Checked) );

  // connections
  connect(m_analysisCheck, SIGNAL(stateChanged(int)), this, SLOT(slotAnalysisCheckChanged(int)));
  connect(m_calibrationCheck, SIGNAL(stateChanged(int)), this, SLOT(slotCalibrationCheckChanged(int)));
  connect(m_referenceCheck, SIGNAL(stateChanged(int)), this, SLOT(slotReferenceCheckChanged(int)));
  connect(browseBtn, SIGNAL(clicked()), this, SLOT(slotBrowsePath()));
  connect(m_selectFileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSelectFileFormatChanged(int)));
}

CWProjectTabOutput::~CWProjectTabOutput()
{
}

void CWProjectTabOutput::apply(mediate_project_output_t *properties) const
{
  properties->analysisFlag = (m_analysisCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->calibrationFlag = (m_calibrationCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->referenceFlag = (m_referenceCheck->checkState() == Qt::Checked);
  properties->directoryFlag = (m_directoryCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->filenameFlag = (m_useFileName->checkState() == Qt::Checked) ? 1 : 0;

  properties->file_format = static_cast<enum output_format>(m_selectFileFormat->currentIndex());

  strcpy(properties->flux, m_fluxEdit->text().toAscii().data());
  strcpy(properties->colourIndex, m_colourIndexEdit->text().toAscii().data());
  strcpy(properties->path, m_pathEdit->text().toAscii().data());
  
  strcpy(properties->swath_name, m_swathNameEdit->hasAcceptableInput()
         ? m_swathNameEdit->text().toAscii().data()
         : OUTPUT_HDFEOS_DEFAULT_SWATH);

  m_selector->apply(&(properties->selection));
}

void CWProjectTabOutput::slotSelectFileFormatChanged(int index)
{
  // parent widget of m_swathNameEdit contains the label and the text
  // field -> set both to visible/invisible if HDF-EOS5 output is
  // selected/unselected
  m_swathNameEdit->parentWidget()->setVisible(index==HDFEOS5);
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
  setComponentsEnabled((state == Qt::Checked),
                       (m_calibrationCheck->checkState() == Qt::Checked) || (m_referenceCheck->checkState() == Qt::Checked));
}

void CWProjectTabOutput::slotCalibrationCheckChanged(int state)
{
  setComponentsEnabled((m_analysisCheck->checkState() == Qt::Checked),
                       (state == Qt::Checked) || (m_referenceCheck->checkState() == Qt::Checked));

  emit signalOutputCalibration((state == Qt::Checked));
}

void CWProjectTabOutput::slotReferenceCheckChanged(int state)
{
  setComponentsEnabled((m_analysisCheck->checkState() == Qt::Checked),
                       (m_calibrationCheck->checkState() == Qt::Checked) || (state == Qt::Checked));
}

void CWProjectTabOutput::setComponentsEnabled(bool analysisEnabled, bool calibrationEnabled)
{
  bool allEnabled = (analysisEnabled || calibrationEnabled);

  m_directoryCheck->setEnabled(allEnabled);
  m_useFileName->setEnabled(allEnabled);

  m_pathFrame->setEnabled(allEnabled);

  m_editGroup->setEnabled(analysisEnabled);
  m_selector->setEnabled(analysisEnabled);
}

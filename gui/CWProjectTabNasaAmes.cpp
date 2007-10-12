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

#include "CWProjectTabNasaAmes.h"
#include "CWorkSpace.h"
#include "CPreferences.h"

#include "constants.h"

#include "debugutil.h"

CWProjectTabNasaAmes::CWProjectTabNasaAmes(const QString &projectName, const mediate_project_nasa_ames_t *properties, QWidget *parent) :
  QFrame(parent)
{
  int index;

  QGridLayout *mainLayout = new QGridLayout(this);

  mainLayout->setMargin(25);
  
  // Results Path
  m_pathFrame = new QFrame(this);
  m_pathFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *pathLayout = new QHBoxLayout(m_pathFrame);

  pathLayout->addWidget(new QLabel("Results Path", m_pathFrame));

  m_pathEdit = new QLineEdit(m_pathFrame);
  m_pathEdit->setMaxLength(sizeof(properties->path)-1);
  pathLayout->addWidget(m_pathEdit, 1);

  QPushButton *browseBtn = new QPushButton("Browse", m_pathFrame);
  pathLayout->addWidget(browseBtn);

  mainLayout->addWidget(m_pathFrame, 0, 0, 1, 3); // span 3 columns

  // check boxes
  m_saveCheck = new QCheckBox("Save NASA-AMES Results", this);
  mainLayout->addWidget(m_saveCheck, 1, 0);

  m_rejectTestCheck = new QCheckBox("NO2 Rejection Test", this);
  mainLayout->addWidget(m_rejectTestCheck, 2, 0);

  // edits
  m_instrumentLabel = new QLabel("Instrument", this);
  mainLayout->addWidget(m_instrumentLabel, 1, 1, Qt::AlignRight);
  m_instrumentEdit = new QLineEdit(this);
  m_instrumentEdit->setMaxLength(sizeof(properties->instrument)-1);
  mainLayout->addWidget(m_instrumentEdit, 1, 2);
  
  m_experimentLabel = new QLabel("Experiment", this);
  mainLayout->addWidget(m_experimentLabel, 2, 1, Qt::AlignRight);
  m_experimentEdit = new QLineEdit(this);
  m_experimentEdit->setMaxLength(sizeof(properties->experiment)-1);
  mainLayout->addWidget(m_experimentEdit, 2, 2);

  m_comboGroup = new QGroupBox("Selected Analysis Window", this);
  QGridLayout *comboLayout = new QGridLayout(m_comboGroup);

  // combo boxes
  comboLayout->addWidget(new QLabel("NO2", m_comboGroup), 0, 0, Qt::AlignRight);
  m_awComboNO2 = new QComboBox(m_comboGroup);
  m_awComboNO2->addItem("Dont Save");
  m_awComboNO2->addItems(CWorkSpace::instance()->analysisWindowsWithSymbol(projectName, "NO2"));
  comboLayout->addWidget(m_awComboNO2, 0, 1);

  comboLayout->addWidget(new QLabel("O3", m_comboGroup), 1, 0, Qt::AlignRight);
  m_awComboO3 = new QComboBox(m_comboGroup);
  m_awComboO3->addItem("Dont Save");
  m_awComboO3->addItems(CWorkSpace::instance()->analysisWindowsWithSymbol(projectName, "O3"));
  comboLayout->addWidget(m_awComboO3, 1, 1);

  comboLayout->addWidget(new QLabel("BrO", m_comboGroup), 0, 2, Qt::AlignRight);
  m_awComboBrO = new QComboBox(m_comboGroup);
  m_awComboBrO->addItem("Dont Save");
  m_awComboBrO->addItems(CWorkSpace::instance()->analysisWindowsWithSymbol(projectName, "BrO"));
  comboLayout->addWidget(m_awComboBrO, 0, 3);

  comboLayout->addWidget(new QLabel("OClO", m_comboGroup), 1, 2, Qt::AlignRight);
  m_awComboOClO = new QComboBox(m_comboGroup);
  m_awComboOClO->addItem("Dont Save");
  m_awComboOClO->addItems(CWorkSpace::instance()->analysisWindowsWithSymbol(projectName, "OClO"));
  comboLayout->addWidget(m_awComboOClO, 1, 3);

  comboLayout->setColumnStretch(0, 1);
  comboLayout->setColumnStretch(1, 2);
  comboLayout->setColumnStretch(2, 1);
  comboLayout->setColumnStretch(3, 2);
  comboLayout->setColumnStretch(4, 3);


  mainLayout->addWidget(m_comboGroup, 3, 0, 1, 3);

  mainLayout->setRowStretch(4, 1);
  mainLayout->setColumnStretch(0, 1);
  mainLayout->setColumnStretch(1, 1);
  mainLayout->setColumnStretch(2, 1);

  // initialize ...
  m_pathEdit->setText(QString(properties->path));

  m_saveCheck->setCheckState(properties->saveFlag ? Qt::Checked : Qt::Unchecked);
  m_rejectTestCheck->setCheckState(properties->rejectTestFlag ? Qt::Checked : Qt::Unchecked);

  m_instrumentEdit->setText(QString(properties->instrument));
  m_experimentEdit->setText(QString(properties->experiment));

  index = m_awComboNO2->findText(QString(properties->anlysWinNO2));
  if (index != -1)
    m_awComboNO2->setCurrentIndex(index);

  index = m_awComboO3->findText(QString(properties->anlysWinO3));
  if (index != -1)
    m_awComboO3->setCurrentIndex(index);
  
  index = m_awComboBrO->findText(QString(properties->anlysWinBrO));
  if (index != -1)
    m_awComboBrO->setCurrentIndex(index);

  index = m_awComboOClO->findText(QString(properties->anlysWinOClO));
  if (index != -1)
    m_awComboOClO->setCurrentIndex(index);

  slotSaveCheckChanged(m_saveCheck->checkState());

  // connections
  connect(m_saveCheck, SIGNAL(stateChanged(int)), this, SLOT(slotSaveCheckChanged(int)));
  connect(browseBtn, SIGNAL(clicked()), this, SLOT(slotBrowsePath()));
}

CWProjectTabNasaAmes::~CWProjectTabNasaAmes()
{
}

void CWProjectTabNasaAmes::apply(mediate_project_nasa_ames_t *properties) const
{
  properties->saveFlag = (m_saveCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->rejectTestFlag = (m_rejectTestCheck->checkState() == Qt::Checked) ? 1 : 0;

  strcpy(properties->instrument, m_instrumentEdit->text().toAscii().data());
  strcpy(properties->experiment, m_experimentEdit->text().toAscii().data());
  strcpy(properties->path, m_pathEdit->text().toAscii().data());

  // combo boxes ....
  if (m_awComboNO2->currentIndex() != 0)
    strcpy(properties->anlysWinNO2, m_awComboNO2->currentText().toAscii().data());
  else
    *(properties->anlysWinNO2) = '\0'; // Dont Save => empty

  if (m_awComboO3->currentIndex() != 0)
    strcpy(properties->anlysWinO3, m_awComboO3->currentText().toAscii().data());
  else
    *(properties->anlysWinO3) = '\0'; // Dont Save => empty

  if (m_awComboBrO->currentIndex() != 0)
    strcpy(properties->anlysWinBrO, m_awComboBrO->currentText().toAscii().data());
  else
    *(properties->anlysWinBrO) = '\0'; // Dont Save => empty

  if (m_awComboOClO->currentIndex() != 0)
    strcpy(properties->anlysWinOClO, m_awComboOClO->currentText().toAscii().data());
  else
    *(properties->anlysWinOClO) = '\0'; // Dont Save => empty
}

void CWProjectTabNasaAmes::slotBrowsePath()
{
  CPreferences *pref = CPreferences::instance();

  QString dirName = QFileDialog::getExistingDirectory(this, "Select NASA-AMES Result Path", pref->directoryName("NasaAmes"));

  if (!dirName.isEmpty()) {
    // save it again
    pref->setDirectoryName("NasaAmes", dirName);

    m_pathEdit->setText(dirName);
  }
}

void CWProjectTabNasaAmes::slotSaveCheckChanged(int state)
{
  bool enable = (state == Qt::Checked);

  m_pathFrame->setEnabled(enable);
  m_rejectTestCheck->setEnabled(enable);

  m_instrumentLabel->setEnabled(enable);
  m_instrumentEdit->setEnabled(enable);
  m_experimentLabel->setEnabled(enable);
  m_experimentEdit->setEnabled(enable);
  
  m_comboGroup->setEnabled(enable);
}


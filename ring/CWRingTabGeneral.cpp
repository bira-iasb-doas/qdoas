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
#include <QPushButton>
#include <QFileDialog>

#include "CWRingTabGeneral.h"
#include "CValidator.h"

#include "constants.h"


CWRingTabGeneral::CWRingTabGeneral(const mediate_ring_t *properties, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(25);
  mainLayout->setSpacing(15);

  QGridLayout *fileLayout = new QGridLayout;
  int row = 0;

  // output
  fileLayout->addWidget(new QLabel("Output", this), row, 0);
  m_outputFileEdit = new QLineEdit(this);
  m_outputFileEdit->setMaxLength(sizeof(properties->outputFile)-1);
  fileLayout->addWidget(m_outputFileEdit, row, 1);
  QPushButton *outputBtn = new QPushButton("Browse", this);
  fileLayout->addWidget(outputBtn, row, 2);
  ++row;

  // calibration
  fileLayout->addWidget(new QLabel("Calibration", this), row, 0);
  m_calibFileEdit = new QLineEdit(this);
  m_calibFileEdit->setMaxLength(sizeof(properties->calibrationFile)-1);
  fileLayout->addWidget(m_calibFileEdit, row, 1);
  QPushButton *calibBtn = new QPushButton("Browse", this);
  fileLayout->addWidget(calibBtn, row, 2);
  ++row;

  // solar ref
  fileLayout->addWidget(new QLabel("Solar Ref.", this), row, 0);
  m_refFileEdit = new QLineEdit(this);
  m_refFileEdit->setMaxLength(sizeof(properties->solarRefFile)-1);
  fileLayout->addWidget(m_refFileEdit, row, 1);
  QPushButton *refBtn = new QPushButton("Browse", this);
  fileLayout->addWidget(refBtn, row, 2);
  ++row;

  mainLayout->addLayout(fileLayout);

  m_slitEdit = new CWSlitSelector(&(properties->slit), "Slit Function", this);
  mainLayout->addWidget(m_slitEdit);

  QHBoxLayout *tempLayout = new QHBoxLayout;
  tempLayout->setMargin(0);
  tempLayout->addWidget(new QLabel("Temperature (K)", this));
  m_tempEdit = new QLineEdit(this);
  m_tempEdit->setValidator(new CDoubleFixedFmtValidator(0, 999, 1, m_tempEdit));
  tempLayout->addWidget(m_tempEdit);
  tempLayout->addStretch(1);

  mainLayout->addLayout(tempLayout);

  mainLayout->addStretch(1);

  m_headerCheck = new QCheckBox("Remove Header", this);
  mainLayout->addWidget(m_headerCheck, 0, Qt::AlignLeft);

  // initialize
  reset(properties);

  // connections
  connect(outputBtn, SIGNAL(clicked()), this, SLOT(slotBrowseOutput()));
  connect(calibBtn, SIGNAL(clicked()), this, SLOT(slotBrowseCalibration()));
  connect(refBtn, SIGNAL(clicked()), this, SLOT(slotBrowseSolarReference()));

}

CWRingTabGeneral::~CWRingTabGeneral()
{
}

void CWRingTabGeneral::reset(const mediate_ring_t *properties)
{
  // set/reset gui state from properties

  // temp
  QString tmpStr;
  
  tmpStr.setNum(properties->temperature);
  m_tempEdit->validator()->fixup(tmpStr);
  m_tempEdit->setText(tmpStr);

  m_headerCheck->setCheckState(properties->noheader ? Qt::Checked : Qt::Unchecked);

  // files
  m_outputFileEdit->setText(properties->outputFile);
  m_calibFileEdit->setText(properties->calibrationFile);
  m_refFileEdit->setText(properties->solarRefFile);

  // slit function
  m_slitEdit->reset(&(properties->slit));
}

void CWRingTabGeneral::apply(mediate_ring_t *properties) const
{
  properties->noheader = (m_headerCheck->checkState() == Qt::Checked) ? 1 : 0;
  properties->temperature = m_tempEdit->text().toDouble();  

  strcpy(properties->outputFile, m_outputFileEdit->text().toAscii().constData());
  strcpy(properties->calibrationFile, m_calibFileEdit->text().toAscii().constData());
  strcpy(properties->solarRefFile, m_refFileEdit->text().toAscii().constData());

  m_slitEdit->apply(&(properties->slit));
}

void CWRingTabGeneral::slotBrowseOutput()
{
  QString fileName = QFileDialog::getSaveFileName(this, "Output File", ".", "*");
  
  if (!fileName.isEmpty())
    m_outputFileEdit->setText(fileName);
}

void CWRingTabGeneral::slotBrowseCalibration()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Calibration File", ".", "*.clb");
  
  if (!fileName.isEmpty())
    m_calibFileEdit->setText(fileName);
}

void CWRingTabGeneral::slotBrowseSolarReference()
{
  QString fileName = QFileDialog::getOpenFileName(this, "reference File", ".", "*.ktz");
  
  if (!fileName.isEmpty())
    m_refFileEdit->setText(fileName);
}


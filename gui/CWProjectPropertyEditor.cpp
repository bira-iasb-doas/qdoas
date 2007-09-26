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


#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

#include "CWorkSpace.h"

#include "CWProjectTree.h"
#include "CWProjectPropertyEditor.h"


CWProjectPropertyEditor::CWProjectPropertyEditor(const QString &projectName, QWidget *parent) :
  CWEditor(parent),
  m_projectName(projectName)
{
  mediate_project_t *projectData = CWorkSpace::instance()->findProject(m_projectName);
  
  if (!projectData)
    return; // TODO - assert or throw

  QGridLayout *mainLayout = new QGridLayout(this);

  //QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(25);
  mainLayout->setSpacing(5);
  
  m_tabs = new QTabWidget(this);
  
  // Spectra Tab
  m_spectraTab = new CWProjectTabSpectra(&(projectData->spectra));
  m_tabs->addTab(m_spectraTab, "Spectra");
  
  // Analysis Tab
  m_analysisTab = new CWProjectTabAnalysis(&(projectData->analysis));
  m_tabs->addTab(m_analysisTab, "Analysis");

  // Filtering Tab
  m_filteringTab = new CWProjectTabFiltering(&(projectData->lowpass), &(projectData->highpass));
  m_tabs->addTab(m_filteringTab, "Filtering");

  // Calibration Tab
  m_calibrationTab = new CWProjectTabCalibration(&(projectData->calibration));
  m_tabs->addTab(m_calibrationTab, "Calibration");

  // Undersampling Tab
  m_undersamplingTab = new CWProjectTabUndersampling(&(projectData->undersampling));
  m_tabs->addTab(m_undersamplingTab, "Undersampling");

  // Instrumental Tab
  m_instrumentalTab = new CWProjectTabInstrumental(&(projectData->instrumental));
  m_tabs->addTab(m_instrumentalTab, "Instrumental");

  // Slit Tab
  m_slitTab = new CWProjectTabSlit(&(projectData->slit));
  m_tabs->addTab(m_slitTab, "Slit");

  // try and keep the complete tab widget set at the smallest possible size.
  mainLayout->setColumnMinimumWidth(0, 0);
  mainLayout->addWidget(m_tabs, 0, 1);
  mainLayout->setColumnMinimumWidth(2, 0);
  mainLayout->setRowMinimumHeight(1, 0);
  mainLayout->setColumnStretch(0, 1);
  mainLayout->setColumnStretch(2, 1);
  mainLayout->setRowStretch(1, 1);

  // caption string and context tag
  m_captionStr = "Properties of Project : ";
  m_captionStr += m_projectName;

  m_contextTag = m_projectName;
  m_contextTag += " Prop";
  
  notifyAcceptActionOk(true);
}

CWProjectPropertyEditor::~CWProjectPropertyEditor()
{
}

bool CWProjectPropertyEditor::actionOk(void)
{
  // call apply for all tabs ...
  mediate_project_t *projectData = CWorkSpace::instance()->findProject(m_projectName);
  
  if (projectData) {
    m_spectraTab->apply(&(projectData->spectra));
    m_analysisTab->apply(&(projectData->analysis));
    m_filteringTab->apply(&(projectData->lowpass), &(projectData->highpass));
    m_calibrationTab->apply(&(projectData->calibration));
    m_undersamplingTab->apply(&(projectData->undersampling));
    m_instrumentalTab->apply(&(projectData->instrumental));
    m_slitTab->apply(&(projectData->slit));

    CWorkSpace::instance()->modifiedProjectProperties(m_projectName);

    return true;
  }

  // Project not found ... TODO

  return false;
}

void CWProjectPropertyEditor::actionHelp(void)
{
}


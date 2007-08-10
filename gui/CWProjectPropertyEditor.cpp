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
#include <QLabel>
#include <QMessageBox>

#include "CWorkSpace.h"

#include "CWProjectTree.h"
#include "CWProjectPropertyEditor.h"


CWProjectPropertyEditor::CWProjectPropertyEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                                         QWidget *parent) :
  CWEditor(parent),
  m_projectTree(projectTree)
{
  m_projectName = item->text(0);
  mediate_project_t *projectData = CWorkSpace::instance()->findProject(m_projectName);
  
  if (!projectData)
    return; // TODO - assert or throw

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
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

  mainLayout->addWidget(m_tabs);

  // caption string and context tag
  m_captionStr = "Properties of Project : ";
  m_captionStr += m_projectName;

  m_contextTag = m_projectName;
  m_contextTag += "-ProjPropEditor";
  
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

    return true;
  }

  // Project not found ... TODO

  return false;
}

void CWProjectPropertyEditor::actionHelp(void)
{
}


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
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

#include "CWorkSpace.h"
#include "CHelpSystem.h"

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

  // Instrument selection combos
  QHBoxLayout *instrLayout = new QHBoxLayout;
  instrLayout->setMargin(0);
  instrLayout->setSpacing(15);

  m_instrTypeCombo = new QComboBox(this);
  m_instrTypeCombo->addItem("Ground-Based", QVariant(PRJCT_INSTR_TYPE_GROUND_BASED));
  m_instrTypeCombo->addItem("Satellites", QVariant(PRJCT_INSTR_TYPE_SATELLITE));

  instrLayout->addWidget(m_instrTypeCombo);
  instrLayout->addSpacing(30);
  instrLayout->addWidget(new QLabel("Instr. Format", this));

  // create and populate ground based
  m_groundFormatCombo = new QComboBox(this);
  m_groundFormatCombo->addItem("Acton (NILU)", QVariant(PRJCT_INSTR_FORMAT_ACTON));
  m_groundFormatCombo->addItem("ASCII", QVariant(PRJCT_INSTR_FORMAT_ASCII));
  m_groundFormatCombo->addItem("CCD all tracks", QVariant(PRJCT_INSTR_FORMAT_CCD_OHP_96));
  m_groundFormatCombo->addItem("CCD EEV (BIRA-IASB, NILU)", QVariant(PRJCT_INSTR_FORMAT_CCD_EEV));
  m_groundFormatCombo->addItem("CCD Sesame I", QVariant(PRJCT_INSTR_FORMAT_CCD_HA_94));
  m_groundFormatCombo->addItem("CCD (University of Toronto)", QVariant(PRJCT_INSTR_FORMAT_UOFT));
  m_groundFormatCombo->addItem("EASOE", QVariant(PRJCT_INSTR_FORMAT_PDASI_EASOE));
  m_groundFormatCombo->addItem("Logger (PDA, CCD or HAMAMATSU)", QVariant(PRJCT_INSTR_FORMAT_LOGGER));
  m_groundFormatCombo->addItem("MFC (Heidelberg)", QVariant(PRJCT_INSTR_FORMAT_MFC));
  m_groundFormatCombo->addItem("MFC Std (Heidelberg)", QVariant(PRJCT_INSTR_FORMAT_MFC_STD));
  m_groundFormatCombo->addItem("NOAA", QVariant(PRJCT_INSTR_FORMAT_NOAA));
  m_groundFormatCombo->addItem("PDA EG&G (Sept. 94 until now)", QVariant(PRJCT_INSTR_FORMAT_PDAEGG));
  m_groundFormatCombo->addItem("PDA EG&G (Spring 94)", QVariant(PRJCT_INSTR_FORMAT_PDAEGG_OLD));
  m_groundFormatCombo->addItem("RASAS (INTA)", QVariant(PRJCT_INSTR_FORMAT_RASAS));
  m_groundFormatCombo->addItem("SAOZ EFM (1024)", QVariant(PRJCT_INSTR_FORMAT_SAOZ_EFM));
  m_groundFormatCombo->addItem("SAOZ PCD/NMOS (512)", QVariant(PRJCT_INSTR_FORMAT_SAOZ_VIS));
  m_groundFormatCombo->addItem("MKZY PAK (NOVAC, MANNE Kihlman and ZHANG Yan)", QVariant(PRJCT_INSTR_FORMAT_MKZY));
  m_groundFormatCombo->hide();
  // create and populate satellite
  m_satelliteFormatCombo = new QComboBox(this);
  m_satelliteFormatCombo->addItem("GDP (ASCII)", QVariant(PRJCT_INSTR_FORMAT_GDP_ASCII));
  m_satelliteFormatCombo->addItem("GDP (Binary)", QVariant(PRJCT_INSTR_FORMAT_GDP_BIN));
  m_satelliteFormatCombo->addItem("GOME2", QVariant(PRJCT_INSTR_FORMAT_GOME2));
  m_satelliteFormatCombo->addItem("OMI", QVariant(PRJCT_INSTR_FORMAT_OMI));
  m_satelliteFormatCombo->addItem("SCIAMACHY L1C (HDF format)", QVariant(PRJCT_INSTR_FORMAT_SCIA_HDF));
  m_satelliteFormatCombo->addItem("SCIAMACHY L1C (PDS format)", QVariant(PRJCT_INSTR_FORMAT_SCIA_PDS));
  m_satelliteFormatCombo->hide();

  // insert both instrument combos ... one will always be hidden ...
  instrLayout->addWidget(m_groundFormatCombo, 1);
  instrLayout->addWidget(m_satelliteFormatCombo, 1);

  mainLayout->addLayout(instrLayout, 0, 1);


  m_tabs = new QTabWidget(this);

  // Display Tab
  m_displayTab = new CWProjectTabDisplay(&(projectData->display));
  m_tabs->addTab(m_displayTab, "Display");

  // Selection Tab
  m_selectionTab = new CWProjectTabSelection(&(projectData->selection));
  m_tabs->addTab(m_selectionTab, "Selection");

  // Analysis Tab
  m_analysisTab = new CWProjectTabAnalysis(&(projectData->analysis));
  m_tabs->addTab(m_analysisTab, "Analysis");

  // Filtering Tab
  m_filteringTab = new CWFilteringEditor(&(projectData->lowpass), &(projectData->highpass), CWFilteringEditor::CalFitCheck);
  m_tabs->addTab(m_filteringTab, "Filtering");

  // Calibration Tab
  m_calibrationTab = new CWProjectTabCalibration(&(projectData->calibration));
  m_calibrationTab->slotOutputCalibration(projectData->output.calibrationFlag != 0);
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

  // Output Tab
  m_outputTab = new CWProjectTabOutput(&(projectData->output));
  m_outputTab->slotInstrumentChanged(projectData->instrumental.format);
  m_tabs->addTab(m_outputTab, "Output");

  /* TODO JUST REMOVE
  // NASA AMES Tab
  m_nasaAmesTab = new CWProjectTabNasaAmes(projectName, &(projectData->nasaames));
  m_tabs->addTab(m_nasaAmesTab, "NASA-AMES Results");
  */

  // try and keep the complete tab widget set at the smallest possible size.
  mainLayout->setColumnMinimumWidth(0, 0);
  mainLayout->addWidget(m_tabs, 1, 1);
  mainLayout->setColumnMinimumWidth(2, 0);
  mainLayout->setRowMinimumHeight(2, 0);
  mainLayout->setColumnStretch(0, 1);
  mainLayout->setColumnStretch(2, 1);
  mainLayout->setRowStretch(2, 1);

  // caption string and context tag
  m_captionStr = "Properties of Project : ";
  m_captionStr += m_projectName;

  m_contextTag = m_projectName;
  m_contextTag += " Prop";

  // initialize
  int index;

  m_selectedInstrument = projectData->instrumental.format;

  index = m_groundFormatCombo->findData(QVariant(m_selectedInstrument));
  if (index != -1) {
    // Is Ground-Based instrument
    m_groundFormatCombo->setCurrentIndex(index);
    m_instrTypeCombo->setCurrentIndex(0);
  }
  else {
    index = m_satelliteFormatCombo->findData(QVariant(m_selectedInstrument));
    if (index != -1) {
      // Is Satellite instrument
      m_satelliteFormatCombo->setCurrentIndex(index);
      m_instrTypeCombo->setCurrentIndex(1);
    }
  }
  // this makes everything consistent ... and resets m_slectedInstrument
  slotInstrumentTypeChanged(m_instrTypeCombo->currentIndex());

  m_displayTab->slotInstrumentChanged(m_selectedInstrument);
  m_instrumentalTab->slotInstrumentChanged(m_selectedInstrument);
  m_outputTab->slotInstrumentChanged(m_selectedInstrument);

  m_instrumentalTab->slotInstrumentTypeChanged(m_instrTypeCombo->itemData(m_instrTypeCombo->currentIndex()).toInt());

  // connections
  connect(m_instrTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotInstrumentTypeChanged(int)));
  connect(m_groundFormatCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotGroundInstrumentChanged(int)));
  connect(m_satelliteFormatCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSatelliteInstrumentChanged(int)));

  connect(this, SIGNAL(signalInstrumentChanged(int)), m_displayTab, SLOT(slotInstrumentChanged(int)));
  connect(this, SIGNAL(signalInstrumentChanged(int)), m_instrumentalTab, SLOT(slotInstrumentChanged(int)));
  connect(this, SIGNAL(signalInstrumentChanged(int)), m_outputTab, SLOT(slotInstrumentChanged(int)));
  connect(this, SIGNAL(signalInstrumentTypeChanged(int)), m_instrumentalTab, SLOT(slotInstrumentTypeChanged(int)));

  connect(m_outputTab, SIGNAL(signalOutputCalibration(bool)), m_calibrationTab, SLOT(slotOutputCalibration(bool)));

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

    m_displayTab->apply(&(projectData->display));
    m_selectionTab->apply(&(projectData->selection));
    m_analysisTab->apply(&(projectData->analysis));
    m_filteringTab->apply(&(projectData->lowpass), &(projectData->highpass));

    // consider changes to the symbols used by calibration
    CWorkSpace *ws = CWorkSpace::instance();

    for (int i=0; i < projectData->calibration.crossSectionList.nCrossSection; ++i)
      ws->decrementUseCount(projectData->calibration.crossSectionList.crossSection[i].symbol);

    m_calibrationTab->apply(&(projectData->calibration));

    for (int i=0; i < projectData->calibration.crossSectionList.nCrossSection; ++i)
      ws->incrementUseCount(projectData->calibration.crossSectionList.crossSection[i].symbol);

    m_undersamplingTab->apply(&(projectData->undersampling));
    m_instrumentalTab->apply(&(projectData->instrumental));
    m_slitTab->apply(&(projectData->slit));
    m_outputTab->apply(&(projectData->output));

    projectData->instrumental.format = m_selectedInstrument;

    // m_nasaAmesTab->apply(&(projectData->nasaames)); TODO JUST REMOVE

    CWorkSpace::instance()->modifiedProjectProperties(m_projectName);

    return true;
  }

  // Project not found ... TODO

  return false;
}

void CWProjectPropertyEditor::actionHelp(void)
{
  CHelpSystem::showHelpTopic("project", "Projects");
}

void CWProjectPropertyEditor::slotInstrumentTypeChanged(int index)
{
  // Ground = 0 : Satellite = 1

  if (index == 0) {
    // Ground-Based ...
    m_satelliteFormatCombo->hide();
    m_groundFormatCombo->show();

    // the instrument also changes to the selected instrument
    slotGroundInstrumentChanged(m_groundFormatCombo->currentIndex());
  }
  else {
    // Satellite ...
    m_groundFormatCombo->hide();
    m_satelliteFormatCombo->show();

    // the instrument also changes to the selected instrument
    slotSatelliteInstrumentChanged(m_satelliteFormatCombo->currentIndex());
  }

  emit signalInstrumentTypeChanged(m_instrTypeCombo->itemData(index).toInt());
}

void CWProjectPropertyEditor::slotGroundInstrumentChanged(int index)
{
  m_selectedInstrument = m_groundFormatCombo->itemData(index).toInt();

  emit signalInstrumentChanged(m_selectedInstrument);
}

void CWProjectPropertyEditor::slotSatelliteInstrumentChanged(int index)
{
  m_selectedInstrument = m_satelliteFormatCombo->itemData(index).toInt();

  emit signalInstrumentChanged(m_selectedInstrument);
}

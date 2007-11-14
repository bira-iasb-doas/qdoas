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
#include <QLabel>
#include <QPushButton>
#include <QFontMetrics>
#include <QFileDialog>

#include "CWProjectTabSlit.h"

#include "constants.h"

#include "debugutil.h"

//--------------------------------------------------------

static const int cSuggestedColumnZeroWidth = 120; // try and keep editor layout
static const int cSuggestedColumnTwoWidth  = 100; // consistent
static const int cStandardEditWidth         = 70;

//--------------------------------------------------------

CWProjectTabSlit::CWProjectTabSlit(const mediate_project_slit_t *slit, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  mainLayout->addSpacing(25);

  QGridLayout *topLayout = new QGridLayout;

  // solar reference
  topLayout->addWidget(new QLabel("Solar Ref. File", this), 0, 0);
  m_solarRefFileEdit = new QLineEdit(this);
  m_solarRefFileEdit->setMaxLength(sizeof(slit->solarRefFile)-1);
  topLayout->addWidget(m_solarRefFileEdit, 0, 1);
  QPushButton *refBrowseBtn = new QPushButton("Browse", this);
  topLayout->addWidget(refBrowseBtn, 0, 2);

  // fhwm correction checkbox
  m_fwhmCorrectionCheck = new QCheckBox("Apply FWHM correction", this);
  topLayout->addWidget(m_fwhmCorrectionCheck, 1, 1, 1, 2);

  // slit type
  m_slitCombo = new QComboBox(this);
  m_slitStack = new QStackedWidget(this);
  // insert widgets into the stack and items into the combo in lock-step.

  m_fileEdit = new CWSlitFileEdit(&(slit->file));
  m_slitStack->addWidget(m_fileEdit);
  m_slitCombo->addItem("File", QVariant(SLIT_TYPE_FILE));

  m_gaussianEdit = new CWSlitGaussianEdit(&(slit->gaussian));
  m_slitStack->addWidget(m_gaussianEdit);
  m_slitCombo->addItem("Gaussian", QVariant(SLIT_TYPE_GAUSS));

  m_lorentzEdit = new CWSlitLorentzEdit(&(slit->lorentz));
  m_slitStack->addWidget(m_lorentzEdit);
  m_slitCombo->addItem("2n-Lorentz", QVariant(SLIT_TYPE_INVPOLY));

  m_voigtEdit = new CWSlitVoigtEdit(&(slit->voigt));
  m_slitStack->addWidget(m_voigtEdit);
  m_slitCombo->addItem("Voigt", QVariant(SLIT_TYPE_VOIGT));

  m_errorEdit = new CWSlitErrorEdit(&(slit->error));
  m_slitStack->addWidget(m_errorEdit);
  m_slitCombo->addItem("Error Function", QVariant(SLIT_TYPE_ERF));

  m_boxcarApodEdit = new CWSlitApodEdit(&(slit->boxcarapod));
  m_slitStack->addWidget(m_boxcarApodEdit);
  m_slitCombo->addItem("Boxcar (FTS)", QVariant(SLIT_TYPE_APOD));

  m_nbsApodEdit = new CWSlitApodEdit(&(slit->nbsapod));
  m_slitStack->addWidget(m_nbsApodEdit);
  m_slitCombo->addItem("Norton Beer Strong (FTS)", QVariant(SLIT_TYPE_APODNBS));

  m_gaussianFileEdit = new CWSlitFileEdit(&(slit->gaussianfile));
  m_slitStack->addWidget(m_gaussianFileEdit);
  m_slitCombo->addItem("Gaussian, wavelength dependent", QVariant(SLIT_TYPE_GAUSS_FILE));

  m_lorentzFileEdit = new CWSlitLorentzFileEdit(&(slit->lorentzfile));
  m_slitStack->addWidget(m_lorentzFileEdit);
  m_slitCombo->addItem("2n-Lorentz, wavelength dependent", QVariant(SLIT_TYPE_INVPOLY_FILE));

  m_errorFileEdit = new CWSlitErrorFileEdit(&(slit->errorfile));
  m_slitStack->addWidget(m_errorFileEdit);
  m_slitCombo->addItem("Error Function, wavelength dependent", QVariant(SLIT_TYPE_ERF_FILE));

  m_gaussianTempFileEdit = new CWSlitFileEdit(&(slit->gaussiantempfile));
  m_slitStack->addWidget(m_gaussianTempFileEdit);
  m_slitCombo->addItem("Gaussian, wavelength + Temp. dependent", QVariant(SLIT_TYPE_GAUSS_T_FILE));

  m_errorTempFileEdit = new CWSlitErrorFileEdit(&(slit->errortempfile));
  m_slitStack->addWidget(m_errorTempFileEdit);
  m_slitCombo->addItem("Error Function, wavelength + Temp. dependent", QVariant(SLIT_TYPE_ERF_T_FILE));


  topLayout->addWidget(new QLabel("Slit Function Type", this), 2, 0);
  topLayout->addWidget(m_slitCombo, 2, 1, 1, 2);

  topLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  topLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);
  topLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(topLayout);
  mainLayout->addWidget(m_slitStack);
  mainLayout->addStretch(1);

  // connections
  connect(refBrowseBtn, SIGNAL(clicked()), this, SLOT(slotSolarRefFileBrowse()));
  connect(m_slitCombo, SIGNAL(currentIndexChanged(int)), m_slitStack, SLOT(setCurrentIndex(int)));

  // initialize
  m_solarRefFileEdit->setText(slit->solarRefFile);
  m_fwhmCorrectionCheck->setCheckState(slit->applyFwhmCorrection ? Qt::Checked : Qt::Unchecked);
  // set the current slit - stack will follow
  int index = m_slitCombo->findData(QVariant(slit->slitType));
  if (index != -1)
    m_slitCombo->setCurrentIndex(index);

 }

CWProjectTabSlit::~CWProjectTabSlit()
{
}

void CWProjectTabSlit::apply(mediate_project_slit_t *slit) const
{
  // set values for ALL slits ... and the selected slit type

  slit->slitType = m_slitCombo->itemData(m_slitCombo->currentIndex()).toInt();
  slit->applyFwhmCorrection = m_fwhmCorrectionCheck->isChecked() ? 1 : 0;
  strcpy(slit->solarRefFile, m_solarRefFileEdit->text().toAscii().data());
  
  m_fileEdit->apply(&(slit->file));
  m_gaussianEdit->apply(&(slit->gaussian));
  m_lorentzEdit->apply(&(slit->lorentz));
  m_voigtEdit->apply(&(slit->voigt));
  m_errorEdit->apply(&(slit->error));
  m_boxcarApodEdit->apply(&(slit->boxcarapod));
  m_nbsApodEdit->apply(&(slit->nbsapod));
  m_gaussianFileEdit->apply(&(slit->gaussianfile));
  m_lorentzFileEdit->apply(&(slit->lorentzfile));
  m_errorFileEdit->apply(&(slit->errorfile));
  m_gaussianTempFileEdit->apply(&(slit->gaussiantempfile));
  m_errorTempFileEdit->apply(&(slit->errortempfile));
}

void CWProjectTabSlit::slotSolarRefFileBrowse()
{
  QString file = QFileDialog::getOpenFileName(this, "Select Solar Reference File", QString(),
					      "Kurucz File(*.ktz);;All Files (*)");

  if (!file.isEmpty())
    m_solarRefFileEdit->setText(file);
}
   
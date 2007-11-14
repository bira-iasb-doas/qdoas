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

#include "CWConvTabSlit.h"
#include "CValidator.h"

#include "constants.h"


CWConvTabSlit::CWConvTabSlit(const mediate_conv_slit_t *conv, const mediate_conv_slit_t *deconv, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(25);
  mainLayout->setSpacing(15);

  m_convEdit = new CWSlitSelector(conv, "Convolution", this);
  mainLayout->addWidget(m_convEdit);

  m_deconvEdit = new CWSlitSelector(deconv, "Deconvolution", this);
  mainLayout->addWidget(m_deconvEdit);
}

CWConvTabSlit::~CWConvTabSlit()
{
}

void CWConvTabSlit::apply(mediate_conv_slit_t *conv, mediate_conv_slit_t *deconv) const
{
  m_convEdit->apply(conv);
  m_deconvEdit->apply(deconv);
}




CWSlitSelector::CWSlitSelector(const mediate_conv_slit_t *slit, const QString &title, QWidget *parent) :
  QGroupBox(title, parent)
{

  QGridLayout *mainLayout = new QGridLayout(this);

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


  mainLayout->addWidget(new QLabel("Slit Function Type", this), 0, 0);
  mainLayout->addWidget(m_slitCombo, 0, 1);

  mainLayout->addWidget(m_slitStack, 1, 0, 1, 2);
  mainLayout->setRowStretch(2, 1);

  // initialize set the current slit - stack will follow
  int index = m_slitCombo->findData(QVariant(slit->slitType));
  if (index != -1) {
    m_slitCombo->setCurrentIndex(index);
    m_slitStack->setCurrentIndex(index);
  }

  // connections
  connect(m_slitCombo, SIGNAL(currentIndexChanged(int)), m_slitStack, SLOT(setCurrentIndex(int)));
}

CWSlitSelector::~CWSlitSelector()
{
}

void CWSlitSelector::apply(mediate_conv_slit_t *slit) const
{
  // set values for ALL slits ... and the selected slit type

  slit->slitType = m_slitCombo->itemData(m_slitCombo->currentIndex()).toInt();
  
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

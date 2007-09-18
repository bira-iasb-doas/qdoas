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
#include "CValidator.h"

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
  connect(refBrowseBtn, SIGNAL(clicked()), this, SLOT(slotSolarRefFileBrowse()));
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


  topLayout->addWidget(new QLabel("Slit Function Type", this), 2, 0);
  topLayout->addWidget(m_slitCombo, 2, 1, 1, 2);

  topLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  topLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);
  topLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(topLayout);
  mainLayout->addWidget(m_slitStack);
  mainLayout->addStretch(1);

  // connections
  connect(m_slitCombo, SIGNAL(currentIndexChanged(int)), m_slitStack, SLOT(setCurrentIndex(int)));

  // set the current slit - stack will follow
  int index;

  index = m_slitCombo->findData(QVariant(slit->slitType));
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
}

//--------------------------------------------------------
// Specific Editors

CWSlitFileEdit::CWSlitFileEdit(const struct slit_file *d, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  gridLayout->addWidget(new QLabel("File"), 0, 0, Qt::AlignRight);
  m_filenameEdit = new QLineEdit(this);
  m_filenameEdit->setMaxLength(sizeof(d->filename)-1);
  gridLayout->addWidget(m_filenameEdit, 0, 1);
  QPushButton *browseBtn = new QPushButton("Browse", this);
  gridLayout->addWidget(browseBtn, 0, 2);

  // initialise
  m_filenameEdit->setText(d->filename);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

CWSlitFileEdit::~CWSlitFileEdit()
{
}

void CWSlitFileEdit::apply(struct slit_file *d) const
{
  strcpy(d->filename, m_filenameEdit->text().toAscii().data());
}

//--------------------------------------------------------

CWSlitGaussianEdit::CWSlitGaussianEdit(const struct slit_gaussian *d, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  gridLayout->addWidget(new QLabel("FWHM (nm)"), 0, 0, Qt::AlignRight);
  m_fwhmEdit = new QLineEdit(this);
  m_fwhmEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmEdit));  
  gridLayout->addWidget(m_fwhmEdit, 0, 1, Qt::AlignLeft);

  // initialise
  QString tmpStr;
  m_fwhmEdit->validator()->fixup(tmpStr.setNum(d->fwhm));
  m_fwhmEdit->setText(tmpStr);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

CWSlitGaussianEdit::~CWSlitGaussianEdit()
{
}

void CWSlitGaussianEdit::apply(struct slit_gaussian *d) const
{
  d->fwhm = m_fwhmEdit->text().toDouble();
}




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

#include "CWSlitEditors.h"
#include "CValidator.h"
#include "CPreferences.h"

#include "constants.h"

#include "debugutil.h"

//--------------------------------------------------------

static const int cSuggestedColumnZeroWidth = 120; // try and keep editor layout
static const int cSuggestedColumnTwoWidth  = 100; // consistent
static const int cStandardEditWidth         = 70;

//--------------------------------------------------------

CWSlitFileBase::CWSlitFileBase(QWidget *parent) :
  QFrame(parent)
{
}

CWSlitFileBase::~CWSlitFileBase()
{
}

void CWSlitFileBase::helperConstructFileEdit(QGridLayout *gridLayout, int &row, const char *str, int len)
{
  gridLayout->addWidget(new QLabel("Slit Function File"), row, 0, Qt::AlignRight);
  m_filenameEdit = new QLineEdit(this);
  m_filenameEdit->setMaxLength(len-1);
  gridLayout->addWidget(m_filenameEdit, row, 1);
  QPushButton *browseBtn = new QPushButton("Browse", this);
  gridLayout->addWidget(browseBtn, row, 2);
  ++row;

  // initialise
  m_filenameEdit->setText(str);

  connect(browseBtn, SIGNAL(clicked()), this, SLOT(slotBrowseFile()));
}

void CWSlitFileBase::slotBrowseFile()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Slit Function File",
						  pref->directoryName("Slit"),
						  "Slit Function File (*.slf);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Slit", filename);

    m_filenameEdit->setText(filename);
  }
}

//--------------------------------------------------------

CWSlitFileEdit::CWSlitFileEdit(const struct slit_file *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  helperConstructFileEdit(gridLayout, row, d->filename, sizeof(d->filename));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

CWSlitFileEdit::~CWSlitFileEdit()
{
}

void CWSlitFileEdit::reset(const struct slit_file *d)
{
  m_filenameEdit->setText(d->filename);
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
  reset(d);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

CWSlitGaussianEdit::~CWSlitGaussianEdit()
{
}

void CWSlitGaussianEdit::reset(const struct slit_gaussian *d)
{
  QString tmpStr;
  m_fwhmEdit->validator()->fixup(tmpStr.setNum(d->fwhm));
  m_fwhmEdit->setText(tmpStr);
}

void CWSlitGaussianEdit::apply(struct slit_gaussian *d) const
{
  d->fwhm = m_fwhmEdit->text().toDouble();
}

//--------------------------------------------------------

CWSlitLorentzEdit::CWSlitLorentzEdit(const struct slit_lorentz *d, QWidget *parent) :
  QFrame(parent)
{
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // width
  gridLayout->addWidget(new QLabel("Width (nm)"), row, 0, Qt::AlignRight);
  m_widthEdit = new QLineEdit(this);
  m_widthEdit->setFixedWidth(cStandardEditWidth);
  m_widthEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_widthEdit));
  gridLayout->addWidget(m_widthEdit, row, 1, Qt::AlignLeft);
  ++row;
  // degree
  gridLayout->addWidget(new QLabel("Degree"), row, 0, Qt::AlignRight);
  m_degreeSpin = new QSpinBox(this);
  m_degreeSpin->setRange(0,10);
  m_degreeSpin->setSingleStep(2);
  m_degreeSpin->setFixedWidth(cStandardEditWidth);
  gridLayout->addWidget(m_degreeSpin, row, 1, Qt::AlignLeft);
  ++row;

  // initialise
  reset(d);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

CWSlitLorentzEdit::~CWSlitLorentzEdit()
{
}

void CWSlitLorentzEdit::reset(const struct slit_lorentz *d)
{
  QString tmpStr;
  m_widthEdit->validator()->fixup(tmpStr.setNum(d->width));
  m_widthEdit->setText(tmpStr);
  m_degreeSpin->setValue(d->degree);
}

void CWSlitLorentzEdit::apply(struct slit_lorentz *d) const
{
  d->width = m_widthEdit->text().toDouble();
  d->degree = m_degreeSpin->value();
}

//--------------------------------------------------------

CWSlitVoigtEdit::CWSlitVoigtEdit(const struct slit_voigt *d, QWidget *parent) :
  QFrame(parent)
{
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // left FWHM
  gridLayout->addWidget(new QLabel("Gaussian FWHM (L)"), row, 0, Qt::AlignRight);
  m_fwhmLeftEdit = new QLineEdit(this);
  m_fwhmLeftEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmLeftEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmLeftEdit));
  gridLayout->addWidget(m_fwhmLeftEdit, row, 1, Qt::AlignLeft);
  // right FHWM
  gridLayout->addWidget(new QLabel("Gaussian FWHM (R)"), row, 3, Qt::AlignRight);
  m_fwhmRightEdit = new QLineEdit(this);
  m_fwhmRightEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmRightEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmRightEdit));
  gridLayout->addWidget(m_fwhmRightEdit, row, 4, Qt::AlignLeft);

  gridLayout->addWidget(new QLabel("(nm)"), row, 2, Qt::AlignLeft);
  ++row;

  // left Gauss/Lorentz ratio
  gridLayout->addWidget(new QLabel("Gaussian/Lorentz (L)"), row, 0, Qt::AlignRight);
  m_ratioLeftEdit = new QLineEdit(this);
  m_ratioLeftEdit->setFixedWidth(cStandardEditWidth);
  m_ratioLeftEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 10.0, 3, m_ratioLeftEdit));
  gridLayout->addWidget(m_ratioLeftEdit, row, 1, Qt::AlignLeft);
  // right Gauss/Lorentz ratio
  gridLayout->addWidget(new QLabel("Gaussian/Lorentz (R)"), row, 3, Qt::AlignRight);
  m_ratioRightEdit = new QLineEdit(this);
  m_ratioRightEdit->setFixedWidth(cStandardEditWidth);
  m_ratioRightEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 10.0, 3, m_ratioRightEdit));
  gridLayout->addWidget(m_ratioRightEdit, row, 4, Qt::AlignLeft);
  ++row;

  // initialise
  reset(d);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(2, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

CWSlitVoigtEdit::~CWSlitVoigtEdit()
{
}

void CWSlitVoigtEdit::reset(const struct slit_voigt *d)
{
  QString tmpStr;
  m_fwhmLeftEdit->validator()->fixup(tmpStr.setNum(d->fwhmL));
  m_fwhmLeftEdit->setText(tmpStr);
  m_fwhmRightEdit->validator()->fixup(tmpStr.setNum(d->fwhmR));
  m_fwhmRightEdit->setText(tmpStr);
  m_ratioLeftEdit->validator()->fixup(tmpStr.setNum(d->glRatioL));
  m_ratioLeftEdit->setText(tmpStr);
  m_ratioRightEdit->validator()->fixup(tmpStr.setNum(d->glRatioR));
  m_ratioRightEdit->setText(tmpStr);
}

void CWSlitVoigtEdit::apply(struct slit_voigt *d) const
{
  d->fwhmL = m_fwhmLeftEdit->text().toDouble();
  d->fwhmR = m_fwhmRightEdit->text().toDouble();
  d->glRatioL = m_ratioLeftEdit->text().toDouble();
  d->glRatioR = m_ratioRightEdit->text().toDouble();
}

//--------------------------------------------------------

CWSlitErrorEdit::CWSlitErrorEdit(const struct slit_error *d, QWidget *parent) :
  QFrame(parent)
{
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // fwhm
  gridLayout->addWidget(new QLabel("Gaussian FWHM (nm)"), row, 0, Qt::AlignRight);
  m_fwhmEdit = new QLineEdit(this);
  m_fwhmEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmEdit));
  gridLayout->addWidget(m_fwhmEdit, row, 1, Qt::AlignLeft);
  ++row;
  // width
  gridLayout->addWidget(new QLabel("Boxcar Width (nm)"), row, 0, Qt::AlignRight);
  m_widthEdit = new QLineEdit(this);
  m_widthEdit->setFixedWidth(cStandardEditWidth);
  m_widthEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_widthEdit));
  gridLayout->addWidget(m_widthEdit, row, 1, Qt::AlignLeft);
  ++row;

  // initialise
  reset(d);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

CWSlitErrorEdit::~CWSlitErrorEdit()
{
}

void CWSlitErrorEdit::reset(const struct slit_error *d)
{
  QString tmpStr;
  m_fwhmEdit->validator()->fixup(tmpStr.setNum(d->fwhm));
  m_fwhmEdit->setText(tmpStr);
  m_widthEdit->validator()->fixup(tmpStr.setNum(d->width));
  m_widthEdit->setText(tmpStr);
}

void CWSlitErrorEdit::apply(struct slit_error *d) const
{
  d->fwhm = m_fwhmEdit->text().toDouble();
  d->width = m_widthEdit->text().toDouble();
}

//--------------------------------------------------------

CWSlitApodEdit::CWSlitApodEdit(const struct slit_apod *d, QWidget *parent) :
  QFrame(parent)
{
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // resolution
  gridLayout->addWidget(new QLabel("Resolution (nm)"), row, 0, Qt::AlignRight);
  m_resolutionEdit = new QLineEdit(this);
  m_resolutionEdit->setFixedWidth(cStandardEditWidth);
  m_resolutionEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_resolutionEdit));
  gridLayout->addWidget(m_resolutionEdit, row, 1, Qt::AlignLeft);
  ++row;
  // phase
  gridLayout->addWidget(new QLabel("Phase"), row, 0, Qt::AlignRight);
  m_phaseEdit = new QLineEdit(this);
  m_phaseEdit->setFixedWidth(cStandardEditWidth);
  m_phaseEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 360.0, 3, m_phaseEdit));
  gridLayout->addWidget(m_phaseEdit, row, 1, Qt::AlignLeft);
  ++row;

  // initialise
  reset(d);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

CWSlitApodEdit::~CWSlitApodEdit()
{
}

void CWSlitApodEdit::reset(const struct slit_apod *d)
{
  QString tmpStr;
  m_resolutionEdit->validator()->fixup(tmpStr.setNum(d->resolution));
  m_resolutionEdit->setText(tmpStr);
  m_phaseEdit->validator()->fixup(tmpStr.setNum(d->phase));
  m_phaseEdit->setText(tmpStr);
}

void CWSlitApodEdit::apply(struct slit_apod *d) const
{
  d->resolution = m_resolutionEdit->text().toDouble();
  d->phase = m_phaseEdit->text().toDouble();
}

//--------------------------------------------------------

CWSlitLorentzFileEdit::CWSlitLorentzFileEdit(const struct slit_lorentz_file *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  helperConstructFileEdit(gridLayout, row, d->filename, sizeof(d->filename));

  // degree
  gridLayout->addWidget(new QLabel("Degree"), row, 0, Qt::AlignRight);
  m_degreeSpin = new QSpinBox(this);
  m_degreeSpin->setRange(0,10);
  m_degreeSpin->setSingleStep(2);
  m_degreeSpin->setFixedWidth(cStandardEditWidth);
  gridLayout->addWidget(m_degreeSpin, row, 1, Qt::AlignLeft);
  ++row;

  // initialize
  m_degreeSpin->setValue(d->degree);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

CWSlitLorentzFileEdit::~CWSlitLorentzFileEdit()
{
}

void CWSlitLorentzFileEdit::reset(const struct slit_lorentz_file *d)
{
  m_degreeSpin->setValue(d->degree);

  m_filenameEdit->setText(d->filename);
}

void CWSlitLorentzFileEdit::apply(struct slit_lorentz_file *d) const
{
  strcpy(d->filename, m_filenameEdit->text().toAscii().data());
  d->degree = m_degreeSpin->value();
}

//--------------------------------------------------------

CWSlitErrorFileEdit::CWSlitErrorFileEdit(const struct slit_error_file *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  helperConstructFileEdit(gridLayout, row, d->filename, sizeof(d->filename));

  // width
  gridLayout->addWidget(new QLabel("Boxcar Width (nm)"), row, 0, Qt::AlignRight);
  m_widthEdit = new QLineEdit(this);
  m_widthEdit->setFixedWidth(cStandardEditWidth);
  m_widthEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_widthEdit));
  gridLayout->addWidget(m_widthEdit, row, 1, Qt::AlignLeft);
  ++row;

  // initialise
  QString tmpStr;
  m_widthEdit->validator()->fixup(tmpStr.setNum(d->width));
  m_widthEdit->setText(tmpStr);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

CWSlitErrorFileEdit::~CWSlitErrorFileEdit()
{
}

void CWSlitErrorFileEdit::reset(const struct slit_error_file *d)
{
  QString tmpStr;
  m_widthEdit->validator()->fixup(tmpStr.setNum(d->width));
  m_widthEdit->setText(tmpStr);

  m_filenameEdit->setText(d->filename);
}

void CWSlitErrorFileEdit::apply(struct slit_error_file *d) const
{
  strcpy(d->filename, m_filenameEdit->text().toAscii().data());
  d->width = m_widthEdit->text().toDouble();
}

//--------------------------------------------------------

CWSlitAGaussEdit::CWSlitAGaussEdit(const struct slit_agauss *d, QWidget *parent) :
  QFrame(parent)
{
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // fwhm
  gridLayout->addWidget(new QLabel("Gaussian FWHM (nm)"), row, 0, Qt::AlignRight);
  m_fwhmEdit = new QLineEdit(this);
  m_fwhmEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmEdit));
  gridLayout->addWidget(m_fwhmEdit, row, 1, Qt::AlignLeft);
  ++row;

  // asymmetry factor
  gridLayout->addWidget(new QLabel("Asymmetry factor"), row, 0, Qt::AlignRight);
  m_asymEdit = new QLineEdit(this);
  m_asymEdit->setFixedWidth(cStandardEditWidth);
  m_asymEdit->setValidator(new CDoubleFixedFmtValidator(-50., 50.0, 3, m_asymEdit));
  gridLayout->addWidget(m_asymEdit, row, 1, Qt::AlignLeft);
  ++row;

  // initialise
  reset(d);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

CWSlitAGaussEdit::~CWSlitAGaussEdit()
{
}

void CWSlitAGaussEdit::reset(const struct slit_agauss *d)
{
  QString tmpStr;
  m_fwhmEdit->validator()->fixup(tmpStr.setNum(d->fwhm));
  m_fwhmEdit->setText(tmpStr);
  m_asymEdit->validator()->fixup(tmpStr.setNum(d->asym));
  m_asymEdit->setText(tmpStr);
}

void CWSlitAGaussEdit::apply(struct slit_agauss *d) const
{
  d->fwhm = m_fwhmEdit->text().toDouble();
  d->asym = m_asymEdit->text().toDouble();
}

//--------------------------------------------------------



CWSlitSelector::CWSlitSelector(const mediate_slit_function_t *slit, const QString &title, QWidget *parent) :
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

  m_agaussEdit = new CWSlitAGaussEdit(&(slit->agauss));
  m_slitStack->addWidget(m_agaussEdit);
  m_slitCombo->addItem("Asymmetric Gaussian", QVariant(SLIT_TYPE_AGAUSS));

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
  int index = m_slitCombo->findData(QVariant(slit->type));
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

void CWSlitSelector::reset(const mediate_slit_function_t *slit)
{
  int index = m_slitCombo->findData(QVariant(slit->type));
  if (index != -1) {
    m_slitCombo->setCurrentIndex(index);
    // stack will follow ...
  }

  m_fileEdit->reset(&(slit->file));
  m_gaussianEdit->reset(&(slit->gaussian));
  m_lorentzEdit->reset(&(slit->lorentz));
  m_voigtEdit->reset(&(slit->voigt));
  m_errorEdit->reset(&(slit->error));
  m_agaussEdit->reset(&(slit->agauss));
  m_boxcarApodEdit->reset(&(slit->boxcarapod));
  m_nbsApodEdit->reset(&(slit->nbsapod));
  m_gaussianFileEdit->reset(&(slit->gaussianfile));
  m_lorentzFileEdit->reset(&(slit->lorentzfile));
  m_errorFileEdit->reset(&(slit->errorfile));
  m_gaussianTempFileEdit->reset(&(slit->gaussiantempfile));
  m_errorTempFileEdit->reset(&(slit->errortempfile));
}

void CWSlitSelector::apply(mediate_slit_function_t *slit) const
{
  // set values for ALL slits ... and the selected slit type

  slit->type = m_slitCombo->itemData(m_slitCombo->currentIndex()).toInt();

  m_fileEdit->apply(&(slit->file));
  m_gaussianEdit->apply(&(slit->gaussian));
  m_lorentzEdit->apply(&(slit->lorentz));
  m_voigtEdit->apply(&(slit->voigt));
  m_errorEdit->apply(&(slit->error));
  m_agaussEdit->apply(&(slit->agauss));
  m_boxcarApodEdit->apply(&(slit->boxcarapod));
  m_nbsApodEdit->apply(&(slit->nbsapod));
  m_gaussianFileEdit->apply(&(slit->gaussianfile));
  m_lorentzFileEdit->apply(&(slit->lorentzfile));
  m_errorFileEdit->apply(&(slit->errorfile));
  m_gaussianTempFileEdit->apply(&(slit->gaussiantempfile));
  m_errorTempFileEdit->apply(&(slit->errortempfile));
}

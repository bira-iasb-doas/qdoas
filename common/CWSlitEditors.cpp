
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

#include <QStackedLayout>
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

void CWSlitFileBase::helperConstructFileEdit(QGridLayout *gridLayout, int &row, const char *label,const char *str, int len)
{
	if (row<2)
	 {
   QLabel *labelfwhm = new QLabel(label);
   labelfwhm->setFixedWidth(115);
   labelfwhm->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

   gridLayout->addWidget(labelfwhm, row, 0, Qt::AlignRight);
   m_filenameEdit[row] = new QLineEdit(this);
   m_filenameEdit[row]->setMaxLength(len-1);
   gridLayout->addWidget(m_filenameEdit[row], row, 1);
   m_browseBtn[row] = new QPushButton("Browse", this);
   gridLayout->addWidget(m_browseBtn[row], row, 2);

   // initialise
   m_filenameEdit[row]->setText(str);

   connect(m_browseBtn[row], SIGNAL(clicked()), this, SLOT(slotBrowseFile()));
   ++row;
  }
}

void CWSlitFileBase::slotBrowseFile()
{
 	QPushButton *button = (QPushButton *)sender();

  CPreferences *pref = CPreferences::instance();
  QString filename = QFileDialog::getOpenFileName(this, "Select Slit Function File",
						  pref->directoryName("Slit"),
						  "Slit Function File (*.slf);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Slit", filename);

  if (button==m_browseBtn[1])
   m_filenameEdit[1]->setText(filename);
  else
   m_filenameEdit[0]->setText(filename);
  }
}

void CWSlitFileBase::slotToggleWavelength(int state)
 {
  m_toggleWavelengthStack->setCurrentIndex((state)?1:0);
 }

//--------------------------------------------------------

CWSlitFileEdit::CWSlitFileEdit(const struct slit_file *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // Wavelength dependent

  m_wavelengthDependent = new QCheckBox("Wavelength dependent", this);
  gridLayout->addWidget(m_wavelengthDependent, 0, 1, Qt::AlignLeft);
  connect(m_wavelengthDependent, SIGNAL(stateChanged(int)), this, SLOT (slotToggleWavelength(int)));

   QLabel *labelfwhm = new QLabel("Slit Function File");
   labelfwhm->setFixedWidth(115);
   labelfwhm->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

   gridLayout->addWidget(labelfwhm, 1, 0, Qt::AlignRight);
   m_filenameEdit[0] = new QLineEdit(this);
   m_filenameEdit[0]->setMaxLength(sizeof(d->filename));
   gridLayout->addWidget(m_filenameEdit[0], 1, 1);
   m_browseBtn[0] = new QPushButton("Browse", this);
   // m_browseBtn[0]->setMaxLength(sizeof("Browse")-1);
    gridLayout->addWidget(m_browseBtn[0], 1, 2);

   // initialise
   m_filenameEdit[0]->setText(d->filename);

   connect(m_browseBtn[0], SIGNAL(clicked()), this, SLOT(slotBrowseFile()));

  QFrame *fileFrame = new QFrame(this);
  fileFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fileFrameLayout = new QGridLayout(fileFrame);
  fileFrameLayout->setMargin(0);

  QFrame *fileFrameWve = new QFrame(this);
  fileFrameWve->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fileFrameLayoutWve = new QGridLayout(fileFrameWve);
  fileFrameLayoutWve->setMargin(0);

  row=1;

  helperConstructFileEdit(fileFrameLayoutWve, row, "Stretch on wavelength",d->filename2, sizeof(d->filename2));

  m_toggleWavelengthStack = new QStackedLayout;
  m_toggleWavelengthStack->setMargin(0);
  m_toggleWavelengthStack->addWidget(fileFrame);
  m_toggleWavelengthStack->addWidget(fileFrameWve);

  mainLayout->addLayout(gridLayout);
  mainLayout->addLayout(m_toggleWavelengthStack);

  reset(d);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addStretch(1);
}

CWSlitFileEdit::~CWSlitFileEdit()
{
}

void CWSlitFileEdit::reset(const struct slit_file *d)
{
	 m_filenameEdit[0]->setText(d->filename);

	 if (d->wveDptFlag)
   m_filenameEdit[1]->setText(d->filename2);

  m_wavelengthDependent->setCheckState(d->wveDptFlag ? Qt::Checked : Qt::Unchecked);

}

void CWSlitFileEdit::apply(struct slit_file *d) const
{
	 d->wveDptFlag = m_wavelengthDependent->isChecked() ? 1 : 0;
	 strcpy(d->filename, m_filenameEdit[0]->text().toAscii().data());

  if (d->wveDptFlag)
   strcpy(d->filename2, m_filenameEdit[1]->text().toAscii().data());
}

//--------------------------------------------------------

CWSlitGaussianEdit::CWSlitGaussianEdit(const struct slit_gaussian *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
	 int row=0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // Wavelength dependent

  m_wavelengthDependent = new QCheckBox("Wavelength dependent", this);
  gridLayout->addWidget(m_wavelengthDependent, 0, 1, Qt::AlignLeft);
  connect(m_wavelengthDependent, SIGNAL(stateChanged(int)), this, SLOT (slotToggleWavelength(int)));

  QFrame *fwhmFrame = new QFrame(this);
  fwhmFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *fwhmFrameLayout = new QHBoxLayout(fwhmFrame);
  fwhmFrameLayout->setMargin(0);
  QLabel *labelfwhm = new QLabel("FWHM (nm)", fwhmFrame);
  labelfwhm->setFixedWidth(115);
  labelfwhm->setAlignment(Qt::AlignVCenter|Qt::AlignRight);


  m_fwhmEdit = new QLineEdit(this);
  m_fwhmEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmEdit));

  fwhmFrameLayout->addWidget(labelfwhm);
  fwhmFrameLayout->addWidget(m_fwhmEdit, 1);
  fwhmFrameLayout->addStretch(0);

  QFrame *fileFrame = new QFrame(this);
  fileFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fileFrameLayout = new QGridLayout(fileFrame);
  fileFrameLayout->setMargin(0);

  helperConstructFileEdit(fileFrameLayout, row, "Slit Function File",d->filename, sizeof(d->filename));

  m_toggleWavelengthStack = new QStackedLayout;
  m_toggleWavelengthStack->setMargin(0);
  m_toggleWavelengthStack->addWidget(fwhmFrame);
  m_toggleWavelengthStack->addWidget(fileFrame);

  mainLayout->addLayout(gridLayout);
  mainLayout->addLayout(m_toggleWavelengthStack);

  // initialise
  reset(d);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(1, 1);

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
  m_wavelengthDependent->setCheckState(d->wveDptFlag ? Qt::Checked : Qt::Unchecked);
  m_filenameEdit[0]->setText(d->filename);

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

void CWSlitGaussianEdit::apply(struct slit_gaussian *d) const
{
  d->fwhm = m_fwhmEdit->text().toDouble();
  d->wveDptFlag = m_wavelengthDependent->isChecked() ? 1 : 0;
  strcpy(d->filename, m_filenameEdit[0]->text().toAscii().data());

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

//--------------------------------------------------------

CWSlitLorentzEdit::CWSlitLorentzEdit(const struct slit_lorentz *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  QGridLayout *degreeLayout = new QGridLayout;

  m_wavelengthDependent = new QCheckBox("Wavelength dependent", this);
  gridLayout->addWidget(m_wavelengthDependent, 0, 1, Qt::AlignLeft);
  connect(m_wavelengthDependent, SIGNAL(stateChanged(int)), this, SLOT (slotToggleWavelength(int)));

  QFrame *fwhmFrame = new QFrame(this);
  fwhmFrame->setFrameStyle(QFrame::NoFrame);
  QHBoxLayout *fwhmFrameLayout = new QHBoxLayout(fwhmFrame);
  fwhmFrameLayout->setMargin(0);

  // width

  QLabel *labelWidth = new QLabel("Width (nm)", fwhmFrame);
  labelWidth->setFixedWidth(115);
  labelWidth->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

  m_widthEdit = new QLineEdit(this);
  m_widthEdit->setFixedWidth(cStandardEditWidth);
  m_widthEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_widthEdit));

  fwhmFrameLayout->addWidget(labelWidth);
  fwhmFrameLayout->addWidget(m_widthEdit, 1);
  fwhmFrameLayout->addStretch(0);

  QFrame *fileFrame = new QFrame(this);
  fileFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fileFrameLayout = new QGridLayout(fileFrame);
  fileFrameLayout->setMargin(0);

  helperConstructFileEdit(fileFrameLayout, row, "Slit Function File",d->filename, sizeof(d->filename));

  m_toggleWavelengthStack = new QStackedLayout;
  m_toggleWavelengthStack->setMargin(0);
  m_toggleWavelengthStack->addWidget(fwhmFrame);
  m_toggleWavelengthStack->addWidget(fileFrame);

  // degree

  QLabel *labelDegree = new QLabel("Degree", fwhmFrame);
  labelDegree->setFixedWidth(115);
  labelDegree->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

  degreeLayout->addWidget(labelDegree, 0, 0, Qt::AlignRight);
  m_degreeSpin = new QSpinBox(this);
  m_degreeSpin->setRange(0,10);
  m_degreeSpin->setSingleStep(2);
  m_degreeSpin->setFixedWidth(cStandardEditWidth);
  degreeLayout->addWidget(m_degreeSpin, 0, 1, Qt::AlignLeft);
  ++row;

  mainLayout->addLayout(gridLayout);
  mainLayout->addLayout(m_toggleWavelengthStack);
  mainLayout->addLayout(degreeLayout);

  // initialise
  reset(d);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(1, 1);
  degreeLayout->setColumnMinimumWidth(0, 115); // cSuggestedColumnZeroWidth);
  degreeLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);
  degreeLayout->setColumnStretch(1, 1);

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

  m_wavelengthDependent->setCheckState(d->wveDptFlag ? Qt::Checked : Qt::Unchecked);
  m_filenameEdit[0]->setText(d->filename);

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

void CWSlitLorentzEdit::apply(struct slit_lorentz *d) const
{
  d->width = m_widthEdit->text().toDouble();
  d->degree = m_degreeSpin->value();

  d->wveDptFlag = m_wavelengthDependent->isChecked() ? 1 : 0;
  strcpy(d->filename, m_filenameEdit[0]->text().toAscii().data());

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

//--------------------------------------------------------

CWSlitVoigtEdit::CWSlitVoigtEdit(const struct slit_voigt *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  m_wavelengthDependent = new QCheckBox("Wavelength dependent", this);
  gridLayout->addWidget(m_wavelengthDependent, 0, 1, Qt::AlignLeft);
  connect(m_wavelengthDependent, SIGNAL(stateChanged(int)), this, SLOT (slotToggleWavelength(int)));

  QFrame *fwhmFrame = new QFrame(this);
  fwhmFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fwhmFrameLayout = new QGridLayout(fwhmFrame);
  fwhmFrameLayout->setMargin(0);

  // left FWHM

  QLabel *labelFwhm = new QLabel("Gaussian FWHM (nm)", fwhmFrame);
  labelFwhm->setFixedWidth(115);
  labelFwhm->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

  m_fwhmLeftEdit = new QLineEdit(this);
  m_fwhmLeftEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmLeftEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmLeftEdit));

  QLabel *labelRatio = new QLabel("Lorentz/Gaussian ratio", fwhmFrame);
  labelRatio->setFixedWidth(115);
  labelRatio->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

  m_ratioLeftEdit = new QLineEdit(this);
  m_ratioLeftEdit->setFixedWidth(cStandardEditWidth);
  m_ratioLeftEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 10.0, 3, m_ratioLeftEdit));

  fwhmFrameLayout->addWidget(labelFwhm,0,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_fwhmLeftEdit,0,1,Qt::AlignLeft);
  fwhmFrameLayout->addWidget(labelRatio,1,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_ratioLeftEdit,1, 1,Qt::AlignLeft);
  // fwhmFrameLayout->addStretch(0);

  QFrame *fileFrame = new QFrame(this);
  fileFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fileFrameLayout = new QGridLayout(fileFrame);
  fileFrameLayout->setMargin(0);

  helperConstructFileEdit(fileFrameLayout, row, "Gaussian FWHM File",d->filename, sizeof(d->filename));
  helperConstructFileEdit(fileFrameLayout, row, "Lorentz/Gaussian ratio",d->filename2, sizeof(d->filename2));

  m_toggleWavelengthStack = new QStackedLayout;
  m_toggleWavelengthStack->setMargin(0);
  m_toggleWavelengthStack->addWidget(fwhmFrame);
  m_toggleWavelengthStack->addWidget(fileFrame);

  mainLayout->addLayout(gridLayout);
  mainLayout->addLayout(m_toggleWavelengthStack);

  // initialise
  reset(d);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(2, 1);

  fwhmFrameLayout->setColumnMinimumWidth(0, 115);
  fwhmFrameLayout->setColumnStretch(1, 1);

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
  m_ratioLeftEdit->validator()->fixup(tmpStr.setNum(d->glRatioL));
  m_ratioLeftEdit->setText(tmpStr);

  m_wavelengthDependent->setCheckState(d->wveDptFlag ? Qt::Checked : Qt::Unchecked);
  m_filenameEdit[0]->setText(d->filename);
  m_filenameEdit[1]->setText(d->filename2);

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

void CWSlitVoigtEdit::apply(struct slit_voigt *d) const
{
  d->fwhmL = m_fwhmLeftEdit->text().toDouble();
  d->glRatioL = m_ratioLeftEdit->text().toDouble();

  d->wveDptFlag = m_wavelengthDependent->isChecked() ? 1 : 0;
  strcpy(d->filename, m_filenameEdit[0]->text().toAscii().data());
  strcpy(d->filename2, m_filenameEdit[1]->text().toAscii().data());

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

//--------------------------------------------------------

CWSlitErrorEdit::CWSlitErrorEdit(const struct slit_error *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  m_wavelengthDependent = new QCheckBox("Wavelength dependent", this);
  gridLayout->addWidget(m_wavelengthDependent, 0, 1, Qt::AlignLeft);
  connect(m_wavelengthDependent, SIGNAL(stateChanged(int)), this, SLOT (slotToggleWavelength(int)));

  QFrame *fwhmFrame = new QFrame(this);
  fwhmFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fwhmFrameLayout = new QGridLayout(fwhmFrame);
  fwhmFrameLayout->setMargin(0);

  // fwhm

  QLabel *labelFwhm = new QLabel("Gaussian FWHM (nm)", fwhmFrame);
  labelFwhm->setFixedWidth(115);
  labelFwhm->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

  m_fwhmEdit = new QLineEdit(this);
  m_fwhmEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmEdit));

  // width

  QLabel *labelWidth = new QLabel("Boxcar width", fwhmFrame);
  labelWidth->setFixedWidth(115);
  labelWidth->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

  m_widthEdit = new QLineEdit(this);
  m_widthEdit->setFixedWidth(cStandardEditWidth);
  m_widthEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_widthEdit));

  fwhmFrameLayout->addWidget(labelFwhm,0,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_fwhmEdit,0,1,Qt::AlignLeft);
  fwhmFrameLayout->addWidget(labelWidth,1,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_widthEdit,1, 1,Qt::AlignLeft);

  QFrame *fileFrame = new QFrame(this);
  fileFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fileFrameLayout = new QGridLayout(fileFrame);
  fileFrameLayout->setMargin(0);

  helperConstructFileEdit(fileFrameLayout, row, "Gaussian FWHM File",d->filename, sizeof(d->filename));
  helperConstructFileEdit(fileFrameLayout, row, "Boxcar width File",d->filename2, sizeof(d->filename2));

  m_toggleWavelengthStack = new QStackedLayout;
  m_toggleWavelengthStack->setMargin(0);
  m_toggleWavelengthStack->addWidget(fwhmFrame);
  m_toggleWavelengthStack->addWidget(fileFrame);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addLayout(m_toggleWavelengthStack);
  mainLayout->addStretch(1);

  // initialise
  reset(d);

  fwhmFrameLayout->setColumnMinimumWidth(0, 115);
  fwhmFrameLayout->setColumnStretch(1, 1);
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

  m_wavelengthDependent->setCheckState(d->wveDptFlag ? Qt::Checked : Qt::Unchecked);
  m_filenameEdit[0]->setText(d->filename);
  m_filenameEdit[1]->setText(d->filename2);

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

void CWSlitErrorEdit::apply(struct slit_error *d) const
{
  d->fwhm = m_fwhmEdit->text().toDouble();
  d->width = m_widthEdit->text().toDouble();

  d->wveDptFlag = m_wavelengthDependent->isChecked() ? 1 : 0;
  strcpy(d->filename, m_filenameEdit[0]->text().toAscii().data());
  strcpy(d->filename2, m_filenameEdit[1]->text().toAscii().data());

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
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

  helperConstructFileEdit(gridLayout, row, "Slit Function File",d->filename, sizeof(d->filename));

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

  m_filenameEdit[0]->setText(d->filename);
}

void CWSlitLorentzFileEdit::apply(struct slit_lorentz_file *d) const
{
  strcpy(d->filename, m_filenameEdit[0]->text().toAscii().data());
  d->degree = m_degreeSpin->value();
}

//--------------------------------------------------------

CWSlitErrorFileEdit::CWSlitErrorFileEdit(const struct slit_error_file *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  helperConstructFileEdit(gridLayout, row, "Gaussian Width (nm)",d->filename, sizeof(d->filename));
  helperConstructFileEdit(gridLayout, row, "Boxcar Width (nm)",d->filename2, sizeof(d->filename2));

  // width
  // gridLayout->addWidget(new QLabel("Boxcar Width (nm)"), row, 0, Qt::AlignRight);
  // m_widthEdit = new QLineEdit(this);
  // m_widthEdit->setFixedWidth(cStandardEditWidth);
  // m_widthEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_widthEdit));
  // gridLayout->addWidget(m_widthEdit, row, 1, Qt::AlignLeft);
  // ++row;
  //
  // // initialise
  // QString tmpStr;
  // m_widthEdit->validator()->fixup(tmpStr.setNum(d->width));
  // m_widthEdit->setText(tmpStr);

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
  // QString tmpStr;
  // m_widthEdit->validator()->fixup(tmpStr.setNum(d->width));
  // m_widthEdit->setText(tmpStr);

  m_filenameEdit[0]->setText(d->filename);
  m_filenameEdit[1]->setText(d->filename2);
}

void CWSlitErrorFileEdit::apply(struct slit_error_file *d) const
{
  strcpy(d->filename, m_filenameEdit[0]->text().toAscii().data());
  strcpy(d->filename2, m_filenameEdit[1]->text().toAscii().data());
}

//--------------------------------------------------------

CWSlitAGaussEdit::CWSlitAGaussEdit(const struct slit_agauss *d, QWidget *parent) :
  CWSlitFileBase(parent)
{
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  m_wavelengthDependent = new QCheckBox("Wavelength dependent", this);
  gridLayout->addWidget(m_wavelengthDependent, 0, 1, Qt::AlignLeft);
  connect(m_wavelengthDependent, SIGNAL(stateChanged(int)), this, SLOT (slotToggleWavelength(int)));

  QFrame *fwhmFrame = new QFrame(this);
  fwhmFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fwhmFrameLayout = new QGridLayout(fwhmFrame);
  fwhmFrameLayout->setMargin(0);

  // fwhm

  QLabel *labelFwhm = new QLabel("Gaussian FWHM (nm)", fwhmFrame);
  labelFwhm->setFixedWidth(115);
  labelFwhm->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

  m_fwhmEdit = new QLineEdit(this);
  m_fwhmEdit->setFixedWidth(cStandardEditWidth);
  m_fwhmEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 50.0, 3, m_fwhmEdit));

  QLabel *labelAsym = new QLabel("Asymmetry factor", fwhmFrame);
  labelAsym->setFixedWidth(115);
  labelAsym->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

  // asymmetry factor

  m_asymEdit = new QLineEdit(this);
  m_asymEdit->setFixedWidth(cStandardEditWidth);
  m_asymEdit->setValidator(new CDoubleFixedFmtValidator(-50., 50.0, 3, m_asymEdit));

  fwhmFrameLayout->addWidget(labelFwhm,0,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_fwhmEdit,0,1,Qt::AlignLeft);
  fwhmFrameLayout->addWidget(labelAsym,1,0,Qt::AlignRight);
  fwhmFrameLayout->addWidget(m_asymEdit,1, 1,Qt::AlignLeft);

  QFrame *fileFrame = new QFrame(this);
  fileFrame->setFrameStyle(QFrame::NoFrame);
  QGridLayout *fileFrameLayout = new QGridLayout(fileFrame);
  fileFrameLayout->setMargin(0);

  helperConstructFileEdit(fileFrameLayout, row, "Gaussian FWHM File",d->filename, sizeof(d->filename));
  helperConstructFileEdit(fileFrameLayout, row, "Boxcar width File",d->filename2, sizeof(d->filename2));

  m_toggleWavelengthStack = new QStackedLayout;
  m_toggleWavelengthStack->setMargin(0);
  m_toggleWavelengthStack->addWidget(fwhmFrame);
  m_toggleWavelengthStack->addWidget(fileFrame);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addLayout(m_toggleWavelengthStack);
  mainLayout->addStretch(1);

  // initialise
  reset(d);

  fwhmFrameLayout->setColumnMinimumWidth(0, 115);
  fwhmFrameLayout->setColumnStretch(1, 1);
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

  m_wavelengthDependent->setCheckState(d->wveDptFlag ? Qt::Checked : Qt::Unchecked);
  m_filenameEdit[0]->setText(d->filename);
  m_filenameEdit[1]->setText(d->filename2);

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
}

void CWSlitAGaussEdit::apply(struct slit_agauss *d) const
{
  d->fwhm = m_fwhmEdit->text().toDouble();
  d->asym = m_asymEdit->text().toDouble();

  d->wveDptFlag = m_wavelengthDependent->isChecked() ? 1 : 0;
  strcpy(d->filename, m_filenameEdit[0]->text().toAscii().data());
  strcpy(d->filename2, m_filenameEdit[1]->text().toAscii().data());

  m_toggleWavelengthStack->setCurrentIndex(d->wveDptFlag);
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

  // not used anymore commented on 01/02/2012 m_gaussianFileEdit = new CWSlitFileEdit(&(slit->gaussianfile));
  // not used anymore commented on 01/02/2012 m_slitStack->addWidget(m_gaussianFileEdit);
  // not used anymore commented on 01/02/2012 m_slitCombo->addItem("Gaussian, wavelength dependent", QVariant(SLIT_TYPE_GAUSS_FILE));
  // not used anymore commented on 01/02/2012
  // not used anymore commented on 01/02/2012 m_lorentzFileEdit = new CWSlitLorentzFileEdit(&(slit->lorentzfile));
  // not used anymore commented on 01/02/2012 m_slitStack->addWidget(m_lorentzFileEdit);
  // not used anymore commented on 01/02/2012 m_slitCombo->addItem("2n-Lorentz, wavelength dependent", QVariant(SLIT_TYPE_INVPOLY_FILE));
  // not used anymore commented on 01/02/2012
  // not used anymore commented on 01/02/2012 m_errorFileEdit = new CWSlitErrorFileEdit(&(slit->errorfile));
  // not used anymore commented on 01/02/2012 m_slitStack->addWidget(m_errorFileEdit);
  // not used anymore commented on 01/02/2012 m_slitCombo->addItem("Error Function, wavelength dependent", QVariant(SLIT_TYPE_ERF_FILE));

  // not used anymore : commented on 12/01/2012 m_gaussianTempFileEdit = new CWSlitFileEdit(&(slit->gaussiantempfile));
  // not used anymore : commented on 12/01/2012 m_slitStack->addWidget(m_gaussianTempFileEdit);
  // not used anymore : commented on 12/01/2012 m_slitCombo->addItem("Gaussian, wavelength + Temp. dependent", QVariant(SLIT_TYPE_GAUSS_T_FILE));
  // not used anymore : commented on 12/01/2012
  // not used anymore : commented on 12/01/2012 m_errorTempFileEdit = new CWSlitErrorFileEdit(&(slit->errortempfile));
  // not used anymore : commented on 12/01/2012 m_slitStack->addWidget(m_errorTempFileEdit);
  // not used anymore : commented on 12/01/2012 m_slitCombo->addItem("Error Function, wavelength + Temp. dependent", QVariant(SLIT_TYPE_ERF_T_FILE));


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
  // not used anymore : commented on 01/02/2012 m_gaussianFileEdit->reset(&(slit->gaussianfile));
  // not used anymore : commented on 01/02/2012 m_lorentzFileEdit->reset(&(slit->lorentzfile));
  // not used anymore : commented on 01/02/2012 m_errorFileEdit->reset(&(slit->errorfile));
  // not used anymore : commented on 12/01/2012 m_gaussianTempFileEdit->reset(&(slit->gaussiantempfile));
  // not used anymore : commented on 12/01/2012 m_errorTempFileEdit->reset(&(slit->errortempfile));
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
  // not used anymore : commented on 01/02/2012 m_gaussianFileEdit->apply(&(slit->gaussianfile));
  // not used anymore : commented on 01/02/2012 m_lorentzFileEdit->apply(&(slit->lorentzfile));
  // not used anymore : commented on 01/02/2012 m_errorFileEdit->apply(&(slit->errorfile));
  // not used anymore : commented on 12/01/2012 m_gaussianTempFileEdit->apply(&(slit->gaussiantempfile));
  // not used anymore : commented on 12/01/2012 m_errorTempFileEdit->apply(&(slit->errortempfile));
}

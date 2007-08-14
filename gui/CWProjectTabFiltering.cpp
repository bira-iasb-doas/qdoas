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
#include <QFontMetrics>

#include "CWProjectTabFiltering.h"
#include "CValidator.h"

#include "debugutil.h"

CWProjectTabFiltering::CWProjectTabFiltering(const mediate_project_filtering_t *lowpass,
					     const mediate_project_filtering_t *highpass, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // Low Pass Filter
  QGroupBox *lowGroup = new QGroupBox("Low Pass Filter", this);
  QVBoxLayout *lowLayout = new QVBoxLayout(lowGroup);
  m_lowCombo = new QComboBox;
  m_lowStack = new QStackedWidget;
  // insert widgets into the stack and items into the combo in lock-step.

  // none
  m_lowStack->addWidget(new QFrame);
  m_lowCombo->addItem("No Filter", QVariant(cProjFilteringModeNone));

  // kaiser
  m_lowKaiser = new CWKaiserEdit(&(lowpass->kaiser));
  m_lowStack->addWidget(m_lowKaiser);
  m_lowCombo->addItem("Kaiser Filter", QVariant(cProjFilteringModeKaiser));
  
  // boxcar
  m_lowBoxcar = new CWBoxcarTriangularBinomialEdit(&(lowpass->boxcar));
  m_lowStack->addWidget(m_lowBoxcar);
  m_lowCombo->addItem("Boxcar Filter", QVariant(cProjFilteringModeBoxcar));
  
  // gaussian
  m_lowGaussian = new CWGaussianEdit(&(lowpass->gaussian));
  m_lowStack->addWidget(m_lowGaussian);
  m_lowCombo->addItem("Gaussian Filter", QVariant(cProjFilteringModeGaussian));
  
  // triangular
  m_lowTriangular = new CWBoxcarTriangularBinomialEdit(&(lowpass->triangular));
  m_lowStack->addWidget(m_lowTriangular);
  m_lowCombo->addItem("Triangular Filter", QVariant(cProjFilteringModeTriangular));
  
  // savitzky-golay
  m_lowSavitzky = new CWSavitzkyGolayEdit(&(lowpass->savitzky));
  m_lowStack->addWidget(m_lowSavitzky);
  m_lowCombo->addItem("Savitzky-Golay Filter", QVariant(cProjFilteringModeSavitzkyGolay));
  
  // none
  m_lowStack->addWidget(new QFrame);
  m_lowCombo->addItem("Odd-Even Correction", QVariant(cProjFilteringModeOddEvenCorrection));

  // binomial
  m_lowBinomial = new CWBoxcarTriangularBinomialEdit(&(lowpass->binomial));
  m_lowStack->addWidget(m_lowBinomial);
  m_lowCombo->addItem("Binomial Filter", QVariant(cProjFilteringModeBinomial));
  

  // low pass group organisation
  lowLayout->addWidget(m_lowCombo);
  lowLayout->addWidget(m_lowStack);
  lowLayout->addStretch(1);


  // High Pass Filter
  QGroupBox *highGroup = new QGroupBox("High Pass Filter", this);
  QVBoxLayout *highLayout = new QVBoxLayout(highGroup);
  m_highCombo = new QComboBox;
  m_highStack = new QStackedWidget;
  // insert widgets into the stack and items into the combo in lock-step.

  // none
  m_highStack->addWidget(new QFrame);
  m_highCombo->addItem("No Filter", QVariant(cProjFilteringModeNone));

  // kaiser
  m_highKaiser = new CWKaiserEdit(&(highpass->kaiser));
  m_highStack->addWidget(m_highKaiser);
  m_highCombo->addItem("Kaiser Filter", QVariant(cProjFilteringModeKaiser));
  
  // boxcar
  m_highBoxcar = new CWBoxcarTriangularBinomialEdit(&(highpass->boxcar));
  m_highStack->addWidget(m_highBoxcar);
  m_highCombo->addItem("Boxcar Filter", QVariant(cProjFilteringModeBoxcar));
  
  // gaussian
  m_highGaussian = new CWGaussianEdit(&(highpass->gaussian));
  m_highStack->addWidget(m_highGaussian);
  m_highCombo->addItem("Gaussian Filter", QVariant(cProjFilteringModeGaussian));
  
  // triangular
  m_highTriangular = new CWBoxcarTriangularBinomialEdit(&(highpass->triangular));
  m_highStack->addWidget(m_highTriangular);
  m_highCombo->addItem("Triangular Filter", QVariant(cProjFilteringModeTriangular));
  
  // savitzky-golay
  m_highSavitzky = new CWSavitzkyGolayEdit(&(highpass->savitzky));
  m_highStack->addWidget(m_highSavitzky);
  m_highCombo->addItem("Savitzky-Golay Filter", QVariant(cProjFilteringModeSavitzkyGolay));
  
  // none
  m_highStack->addWidget(new QFrame);
  m_highCombo->addItem("Odd-Even Correction", QVariant(cProjFilteringModeOddEvenCorrection));

  // binomial
  m_highBinomial = new CWBoxcarTriangularBinomialEdit(&(highpass->binomial));
  m_highStack->addWidget(m_highBinomial);
  m_highCombo->addItem("Binomial Filter", QVariant(cProjFilteringModeBinomial));
  
  // high pass group organisation
  highLayout->addWidget(m_highCombo);
  highLayout->addWidget(m_highStack);
  highLayout->addStretch(1);


  // equal space to both groups
  mainLayout->addWidget(lowGroup, 1);
  mainLayout->addWidget(highGroup, 1);

  // connections
  connect(m_lowCombo, SIGNAL(currentIndexChanged(int)), m_lowStack, SLOT(setCurrentIndex(int)));
  connect(m_highCombo, SIGNAL(currentIndexChanged(int)), m_highStack, SLOT(setCurrentIndex(int)));

  // set the current mode - stack will follow
  int index;
  
  index = m_lowCombo->findData(QVariant(lowpass->mode));
  if (index != -1)
    m_lowCombo->setCurrentIndex(index);

  index = m_highCombo->findData(QVariant(highpass->mode));
  if (index != -1)
    m_highCombo->setCurrentIndex(index);

 }

CWProjectTabFiltering::~CWProjectTabFiltering()
{
}

void CWProjectTabFiltering::apply(mediate_project_filtering_t *lowpass, mediate_project_filtering_t *highpass) const
{
  // set values for ALL filters ... and the selected mode

  lowpass->mode = m_lowCombo->itemData(m_lowCombo->currentIndex()).toInt();
  highpass->mode = m_highCombo->itemData(m_highCombo->currentIndex()).toInt();

  m_lowKaiser->apply(&(lowpass->kaiser));
  m_highKaiser->apply(&(highpass->kaiser));

  m_lowBoxcar->apply(&(lowpass->boxcar));
  m_highBoxcar->apply(&(highpass->boxcar));

  m_lowGaussian->apply(&(lowpass->gaussian));
  m_highGaussian->apply(&(highpass->gaussian));

  m_lowTriangular->apply(&(lowpass->triangular));
  m_highTriangular->apply(&(highpass->triangular));

  m_lowBinomial->apply(&(lowpass->binomial));
  m_highBinomial->apply(&(highpass->binomial));

}


//--------------------------------------------------------
// Specific Filter Editors...

static const int cSuggestedColumnZeroWidth = 120; // try and keep editor layout
static const int cMaxIterations            =  20; // and data consistent.
static const Qt::Alignment cLabelAlign     = Qt::AlignRight;

//--------------------------------------------------------

CWKaiserEdit::CWKaiserEdit(const struct filter_kaiser *d, QWidget *parent) :
  QFrame(parent)
{
  QString tmpStr;
  int row = 0;

  QGridLayout *mainLayout = new QGridLayout(this);

  // row 0
  mainLayout->addWidget(new QLabel("Cut-Off Frequency", this), row, 0, cLabelAlign);

  m_cutoffEdit = new QLineEdit(this);
  m_cutoffEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 100.0, 4, m_cutoffEdit));
  m_cutoffEdit->setFixedWidth(60);

  m_cutoffEdit->validator()->fixup(tmpStr.setNum(d->cutoffFrequency));
  m_cutoffEdit->setText(tmpStr);

  mainLayout->addWidget(m_cutoffEdit, row, 1, Qt::AlignLeft);
  ++row;

  // row 1
  mainLayout->addWidget(new QLabel("Tolerance (dB)", this), row, 0, cLabelAlign);

  m_toleranceEdit = new QLineEdit(this);
  m_toleranceEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 100.0, 4, m_toleranceEdit));
  m_toleranceEdit->setFixedWidth(60);

  m_toleranceEdit->validator()->fixup(tmpStr.setNum(d->tolerance));
  m_toleranceEdit->setText(tmpStr);

  mainLayout->addWidget(m_toleranceEdit, row, 1, Qt::AlignLeft);
  ++row;

  // row 0
  row = 0;
  mainLayout->addWidget(new QLabel("Passband", this), row, 2, cLabelAlign);

  m_passbandEdit = new QLineEdit(this);
  m_passbandEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 100.0, 4, m_passbandEdit));
  m_passbandEdit->setFixedWidth(60);

  m_passbandEdit->validator()->fixup(tmpStr.setNum(d->passband));
  m_passbandEdit->setText(tmpStr);

  mainLayout->addWidget(m_passbandEdit, row, 3, Qt::AlignLeft);
  ++row;

  // row 1
  mainLayout->addWidget(new QLabel("Iterations", this), row, 2, cLabelAlign);

  m_iterationsSpinBox = new QSpinBox(this);
  m_iterationsSpinBox->setRange(1, cMaxIterations);
  m_iterationsSpinBox->setFixedWidth(60);

  m_iterationsSpinBox->setValue(d->iterations);

  mainLayout->addWidget(m_iterationsSpinBox, row, 3, Qt::AlignLeft);
  ++row;

  // layout control
  mainLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  mainLayout->setColumnStretch(1, 1);
  mainLayout->setColumnMinimumWidth(2, cSuggestedColumnZeroWidth);
  mainLayout->setColumnStretch(3, 1);
}

CWKaiserEdit::~CWKaiserEdit()
{
}

void CWKaiserEdit::apply(struct filter_kaiser *d) const
{
  bool ok;
  double tmp;

  tmp = m_cutoffEdit->text().toDouble(&ok);
  d->cutoffFrequency = ok ? tmp : 0.0;
  tmp = m_toleranceEdit->text().toDouble(&ok);
  d->tolerance = ok ? tmp : 0.0;
  tmp = m_passbandEdit->text().toDouble(&ok);
  d->passband = ok ? tmp : 0.0;

  d->iterations = m_iterationsSpinBox->value();
}

//--------------------------------------------------------

CWBoxcarTriangularBinomialEdit::CWBoxcarTriangularBinomialEdit(const struct filter_boxcar *d, QWidget *parent) :
  QFrame(parent)
{
  init(d->width, d->iterations);
}

CWBoxcarTriangularBinomialEdit::CWBoxcarTriangularBinomialEdit(const struct filter_triangular *d, QWidget *parent) :
  QFrame(parent)
{
  init(d->width, d->iterations);
}

CWBoxcarTriangularBinomialEdit::CWBoxcarTriangularBinomialEdit(const struct filter_binomial *d, QWidget *parent) :
  QFrame(parent)
{
  init(d->width, d->iterations);
}

void CWBoxcarTriangularBinomialEdit::init(int filterWidth, int nIterations)
{
  int row = 0;
  QGridLayout *mainLayout = new QGridLayout(this);

  // row 0
  mainLayout->addWidget(new QLabel("Width (pixels)", this), row, 0, cLabelAlign);

  m_widthSpinBox = new QSpinBox(this);
  m_widthSpinBox->setRange(1, 99);
  m_widthSpinBox->setSingleStep(2);
  m_widthSpinBox->setFixedWidth(60);

  m_widthSpinBox->setValue(filterWidth);

  mainLayout->addWidget(m_widthSpinBox, row, 1, Qt::AlignLeft);
  ++row;

  // row 1
  mainLayout->addWidget(new QLabel("Iterations", this), row, 0, cLabelAlign);

  m_iterationsSpinBox = new QSpinBox(this);
  m_iterationsSpinBox->setRange(1, cMaxIterations);
  m_iterationsSpinBox->setFixedWidth(60);

  m_iterationsSpinBox->setValue(nIterations);

  mainLayout->addWidget(m_iterationsSpinBox, row, 1, Qt::AlignLeft);
  ++row;

  // layout control
  mainLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  mainLayout->setColumnStretch(1, 1);
}

CWBoxcarTriangularBinomialEdit::~CWBoxcarTriangularBinomialEdit()
{
}

void CWBoxcarTriangularBinomialEdit::apply(struct filter_boxcar *d) const
{
  d->width = m_widthSpinBox->value();
  d->iterations = m_iterationsSpinBox->value();
}

void CWBoxcarTriangularBinomialEdit::apply(struct filter_triangular *d) const
{
  d->width = m_widthSpinBox->value();
  d->iterations = m_iterationsSpinBox->value();
}

void CWBoxcarTriangularBinomialEdit::apply(struct filter_binomial *d) const
{
  d->width = m_widthSpinBox->value();
  d->iterations = m_iterationsSpinBox->value();
}

//--------------------------------------------------------

CWGaussianEdit::CWGaussianEdit(const struct filter_gaussian *d, QWidget *parent) :
  QFrame(parent)
{
  QString tmpStr;
  int row = 0;

  QGridLayout *mainLayout = new QGridLayout(this);

  // row 0
  mainLayout->addWidget(new QLabel("FWHM (pixels)", this), row, 0, cLabelAlign);

  m_fwhmEdit = new QLineEdit(this);
  m_fwhmEdit->setValidator(new CDoubleFixedFmtValidator(0.0001, 100.0, 4, m_fwhmEdit));
  m_fwhmEdit->setFixedWidth(60);

  m_fwhmEdit->validator()->fixup(tmpStr.setNum(d->fwhm));
  m_fwhmEdit->setText(tmpStr);

  mainLayout->addWidget(m_fwhmEdit, row, 1, Qt::AlignLeft);
  ++row;

  // row 1
  mainLayout->addWidget(new QLabel("Iterations", this), row, 0, cLabelAlign);

  m_iterationsSpinBox = new QSpinBox(this);
  m_iterationsSpinBox->setRange(1, cMaxIterations);
  m_iterationsSpinBox->setFixedWidth(60);

  m_iterationsSpinBox->setValue(d->iterations);

  mainLayout->addWidget(m_iterationsSpinBox, row, 1, Qt::AlignLeft);
  ++row;

  // layout control
  mainLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  mainLayout->setColumnStretch(1, 1);
}

CWGaussianEdit::~CWGaussianEdit()
{
}

void CWGaussianEdit::apply(struct filter_gaussian *d) const
{
  bool ok;
  double tmp;

  tmp = m_fwhmEdit->text().toDouble(&ok);
  d->fwhm = ok ? tmp : 0.0;

  d->iterations = m_iterationsSpinBox->value();
}

//--------------------------------------------------------

CWSavitzkyGolayEdit::CWSavitzkyGolayEdit(const struct filter_savitzky_golay *d, QWidget *parent) :
  QFrame(parent)
{
  int row = 0;
  QGridLayout *mainLayout = new QGridLayout(this);

  // row 0
  mainLayout->addWidget(new QLabel("Width (pixels)", this), row, 0, cLabelAlign);

  m_widthSpinBox = new QSpinBox(this);
  m_widthSpinBox->setRange(1, 99);
  m_widthSpinBox->setSingleStep(2);
  m_widthSpinBox->setFixedWidth(60);

  m_widthSpinBox->setValue(d->width);

  mainLayout->addWidget(m_widthSpinBox, row, 1, Qt::AlignLeft);
  ++row;

  // row 1
  mainLayout->addWidget(new QLabel("Order", this), row, 0, cLabelAlign);

  m_orderSpinBox = new QSpinBox(this);
  m_orderSpinBox->setRange(2, 10);
  m_orderSpinBox->setSingleStep(2);
  m_orderSpinBox->setFixedWidth(60);

  m_orderSpinBox->setValue(d->order);

  mainLayout->addWidget(m_orderSpinBox, row, 1, Qt::AlignLeft);
  ++row;

  // row 2
  mainLayout->addWidget(new QLabel("Iterations", this), row, 0, cLabelAlign);

  m_iterationsSpinBox = new QSpinBox(this);
  m_iterationsSpinBox->setRange(1, cMaxIterations);
  m_iterationsSpinBox->setFixedWidth(60);

  m_iterationsSpinBox->setValue(d->iterations);

  mainLayout->addWidget(m_iterationsSpinBox, row, 1, Qt::AlignLeft);
  ++row;

  // layout control
  mainLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  mainLayout->setColumnStretch(1, 1);
}

CWSavitzkyGolayEdit::~CWSavitzkyGolayEdit()
{
}

void CWSavitzkyGolayEdit::apply(struct filter_savitzky_golay *d) const
{
  d->width = m_widthSpinBox->value();
  d->order = m_orderSpinBox->value();
  d->iterations = m_iterationsSpinBox->value();
}


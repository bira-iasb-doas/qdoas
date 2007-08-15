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
#include <QFontMetrics>
#include <QFileDialog>

#include "CWProjectTabInstrumental.h"
#include "CValidator.h"

#include "constants.h"

#include "debugutil.h"

CWProjectTabInstrumental::CWProjectTabInstrumental(const mediate_project_instrumental_t *instr, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  mainLayout->addSpacing(25);

  QGridLayout *topLayout = new QGridLayout;

  // Format
  m_formatCombo = new QComboBox(this);
  m_formatStack = new QStackedWidget(this);
  // insert widgets into the stack and items into the combo in lock-step.

  // ascii
  m_asciiEdit = new CWInstrAsciiEdit(&(instr->ascii));
  m_formatStack->addWidget(m_asciiEdit);
  m_formatCombo->addItem("ASCII", QVariant(PRJCT_INSTR_FORMAT_ASCII));

  topLayout->addWidget(new QLabel("Format", this), 0, 0);
  topLayout->addWidget(m_formatCombo, 0, 1);
  
  // Site
  m_siteCombo = new QComboBox(this);
  m_siteCombo->addItem("No Site Specified");
  // TODO get the list from the workspace ...

  topLayout->addWidget(new QLabel("Site", this), 1, 0);
  topLayout->addWidget(m_siteCombo, 1, 1);

  topLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(topLayout);
  mainLayout->addWidget(m_formatStack);
  mainLayout->addStretch(1);


  // connections
  connect(m_formatCombo, SIGNAL(currentIndexChanged(int)), m_formatStack, SLOT(setCurrentIndex(int)));

  // set the current format - stack will follow
  int index;
  
  index = m_formatCombo->findData(QVariant(instr->format));
  if (index != -1)
    m_formatCombo->setCurrentIndex(index);

  index = m_siteCombo->findData(QVariant(QString(instr->siteName)));
  if (index != -1)
    m_siteCombo->setCurrentIndex(index);

 }

CWProjectTabInstrumental::~CWProjectTabInstrumental()
{
}

void CWProjectTabInstrumental::apply(mediate_project_instrumental_t *instr) const
{
  // set values for ALL instruments ... and the selected mode

  instr->format = m_formatCombo->itemData(m_formatCombo->currentIndex()).toInt();
  // site ... TODO

  m_asciiEdit->apply(&(instr->ascii));
}


//--------------------------------------------------------
// Specific Instrument Editors... TODO

static const int cSuggestedColumnZeroWidth = 120; // try and keep editor layout
static const int cSuggestedColumnTwoWidth  = 100; // consistent
static const int cDetSizeEditWidth         = 70;
static const Qt::Alignment cLabelAlign     = Qt::AlignRight;

//--------------------------------------------------------

CWInstrAsciiEdit::CWInstrAsciiEdit(const struct instrumental_ascii *d, QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // Format and Flags group
  QHBoxLayout *groupLayout = new QHBoxLayout;

  // Format
  QGroupBox *formatGroup = new QGroupBox("Format", this);
  QVBoxLayout *formatLayout = new QVBoxLayout(formatGroup);

  m_lineRadioButton = new QRadioButton("line", formatGroup);
  formatLayout->addWidget(m_lineRadioButton);
  m_columnRadioButton = new QRadioButton("column", formatGroup);
  formatLayout->addWidget(m_columnRadioButton);
  formatLayout->addStretch(1);

  groupLayout->addWidget(formatGroup);

  // Flags
  QGroupBox *flagsGroup = new QGroupBox("Read from file", this);
  QGridLayout *flagsLayout = new QGridLayout(flagsGroup);

  m_zenCheck = new QCheckBox("Zen. Angle", flagsGroup);
  flagsLayout->addWidget(m_zenCheck, 0, 0);
  m_aziCheck = new QCheckBox("Azi. Angle", flagsGroup);
  flagsLayout->addWidget(m_aziCheck, 0, 1);
  m_eleCheck = new QCheckBox("Elev. Angle", flagsGroup);
  flagsLayout->addWidget(m_eleCheck, 0, 2);

  m_dateCheck = new QCheckBox("DD/MM/YY", flagsGroup);
  flagsLayout->addWidget(m_dateCheck, 1, 0);
  m_timeCheck = new QCheckBox("Decimal Time", flagsGroup);
  flagsLayout->addWidget(m_timeCheck, 1, 1);
  m_lambdaCheck = new QCheckBox("Lambda", flagsGroup);
  flagsLayout->addWidget(m_lambdaCheck, 1, 2);

  groupLayout->addWidget(flagsGroup);

  mainLayout->addLayout(groupLayout);

  // bottom layout - det. size and files
  QGridLayout *bottomLayout = new QGridLayout;

  bottomLayout->addWidget(new QLabel("Detector Size", this), 0, 0);
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cDetSizeEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  bottomLayout->addWidget(m_detSizeEdit, 0, 1);
  
  bottomLayout->addWidget(new QLabel("Calibration File", this), 1, 0);
  m_calibEdit = new QLineEdit(this);
  m_calibEdit->setMaxLength(sizeof(d->calibrationFile)-1);
  bottomLayout->addWidget(m_calibEdit, 1, 1);
  QPushButton *calibBrowseBtn = new QPushButton("Browse...", this);
  bottomLayout->addWidget(calibBrowseBtn, 1, 2);

  bottomLayout->addWidget(new QLabel("Instr. Function File", this), 2, 0);
  m_instrEdit = new QLineEdit(this);
  m_instrEdit->setMaxLength(sizeof(d->instrFunctionFile)-1);
  bottomLayout->addWidget(m_instrEdit, 2, 1);
  QPushButton *instrBrowseBtn = new QPushButton("Browse...", this);
  bottomLayout->addWidget(instrBrowseBtn, 2, 2);

  mainLayout->addLayout(bottomLayout);

  // initial values
  QString tmpStr;

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  // format
  if (d->format == 0) m_lineRadioButton->setChecked(true);
  else m_columnRadioButton->setChecked(true);

  // flags
  m_zenCheck->setCheckState(d->flagZenithAngle ? Qt::Checked : Qt::Unchecked);
  m_aziCheck->setCheckState(d->flagAzimuthAngle ? Qt::Checked : Qt::Unchecked);
  m_eleCheck->setCheckState(d->flagElevationAngle ? Qt::Checked : Qt::Unchecked);
  m_dateCheck->setCheckState(d->flagDate ? Qt::Checked : Qt::Unchecked);
  m_timeCheck->setCheckState(d->flagTime ? Qt::Checked : Qt::Unchecked);
  m_lambdaCheck->setCheckState(d->flagWavelength ? Qt::Checked : Qt::Unchecked);

  // files
  m_calibEdit->setText(QString(d->calibrationFile));
  m_instrEdit->setText(QString(d->instrFunctionFile));

  // connections
  connect(calibBrowseBtn, SIGNAL(clicked()), this, SLOT(slotCalibBrowse()));
  connect(instrBrowseBtn, SIGNAL(clicked()), this, SLOT(slotInstrBrowse()));
}

CWInstrAsciiEdit::~CWInstrAsciiEdit()
{
}

void CWInstrAsciiEdit::apply(struct instrumental_ascii *d) const
{
  // detected size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // format
  if (m_lineRadioButton->isChecked()) d->format = 0;
  if (m_columnRadioButton->isChecked()) d->format = 1;

  // flags
  d->flagZenithAngle = m_zenCheck->isChecked() ? 1 : 0;
  d->flagAzimuthAngle = m_aziCheck->isChecked() ? 1 : 0;
  d->flagElevationAngle = m_eleCheck->isChecked() ? 1 : 0;
  d->flagDate = m_dateCheck->isChecked() ? 1 : 0;
  d->flagTime = m_timeCheck->isChecked() ? 1 : 0;
  d->flagWavelength = m_lambdaCheck->isChecked() ? 1 : 0;

  // files
  strcpy(d->calibrationFile, m_calibEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_instrEdit->text().toAscii().data());

}

void CWInstrAsciiEdit::slotCalibBrowse()
{
  QString file = QFileDialog::getOpenFileName(this, "Select Calibration File", QString(),
					      "Calibration File (*.clb);;All Files (*)");

  if (!file.isEmpty())
    m_calibEdit->setText(file);
}

void CWInstrAsciiEdit::slotInstrBrowse()
{
  QString file = QFileDialog::getOpenFileName(this, "Select Instrument Function File", QString(),
					      "Instrument Function File (*.ins);;All Files (*)");
  
  if (!file.isEmpty())
    m_instrEdit->setText(file);
}

//--------------------------------------------------------


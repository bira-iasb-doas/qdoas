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

  // logger
  m_loggerEdit = new CWInstrLoggerEdit(&(instr->logger));
  m_formatStack->addWidget(m_loggerEdit);
  m_formatCombo->addItem("Logger", QVariant(PRJCT_INSTR_FORMAT_LOGGER));

  // acton
  m_actonEdit = new CWInstrActonEdit(&(instr->acton));
  m_formatStack->addWidget(m_actonEdit);
  m_formatCombo->addItem("Acton", QVariant(PRJCT_INSTR_FORMAT_ACTON));

  // pdaegg
  m_pdaEggEdit = new CWInstrLoggerEdit(&(instr->pdaegg));
  m_formatStack->addWidget(m_pdaEggEdit);
  m_formatCombo->addItem("PDA EGG", QVariant(PRJCT_INSTR_FORMAT_PDAEGG));

  // saozvis
  m_saozVisEdit = new CWInstrSaozEdit(&(instr->saozvis));
  m_formatStack->addWidget(m_saozVisEdit);
  m_formatCombo->addItem("SAOZ Vis", QVariant(PRJCT_INSTR_FORMAT_SAOZ_VIS));

  // saozuv
  m_saozUvEdit = new CWInstrSaozEdit(&(instr->saozuv));
  m_formatStack->addWidget(m_saozUvEdit);
  m_formatCombo->addItem("SAOZ UV", QVariant(PRJCT_INSTR_FORMAT_SAOZ_UV));

  // saozefm
  m_saozEfmEdit = new CWInstrRasasEdit(&(instr->saozefm));
  m_formatStack->addWidget(m_saozEfmEdit);
  m_formatCombo->addItem("SAOZ EFM", QVariant(PRJCT_INSTR_FORMAT_SAOZ_EFM));

  // rasas
  m_rasasEdit = new CWInstrRasasEdit(&(instr->rasas));
  m_formatStack->addWidget(m_rasasEdit);
  m_formatCombo->addItem("RASAS", QVariant(PRJCT_INSTR_FORMAT_RASAS));

   // pdasieasoe
  m_pdasiEasoeEdit = new CWInstrRasasEdit(&(instr->pdasieasoe));
  m_formatStack->addWidget(m_pdasiEasoeEdit);
  m_formatCombo->addItem("EASOE", QVariant(PRJCT_INSTR_FORMAT_PDASI_EASOE));

  // pdasiosma
  m_pdasiOsmaEdit = new CWInstrLoggerEdit(&(instr->pdasiosma));
  m_formatStack->addWidget(m_pdasiOsmaEdit);
  m_formatCombo->addItem("PDASI OSMA", QVariant(PRJCT_INSTR_FORMAT_PDASI_OSMA));

  // uoft
  m_uoftEdit = new CWInstrRasasEdit(&(instr->uoft));
  m_formatStack->addWidget(m_uoftEdit);
  m_formatCombo->addItem("U. of T.", QVariant(PRJCT_INSTR_FORMAT_UOFT));

  // uoft
  m_noaaEdit = new CWInstrRasasEdit(&(instr->noaa));
  m_formatStack->addWidget(m_noaaEdit);
  m_formatCombo->addItem("NOAA", QVariant(PRJCT_INSTR_FORMAT_NOAA));

  topLayout->addWidget(new QLabel("Format", this), 0, 0);
  topLayout->addWidget(m_formatCombo, 0, 1);
  
  // Site
  m_siteCombo = new QComboBox(this);
  m_siteCombo->addItem("No Site Specified");
  // TODO get the list from the workspace ...

  topLayout->addWidget(new QLabel("Site", this), 1, 0);
  topLayout->addWidget(m_siteCombo, 1, 1);

  topLayout->setColumnMinimumWidth(0, 90);
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
  m_loggerEdit->apply(&(instr->logger));
  m_actonEdit->apply(&(instr->acton));
  m_pdaEggEdit->apply(&(instr->pdaegg));
  m_saozVisEdit->apply(&(instr->saozvis));
  m_saozUvEdit->apply(&(instr->saozuv));
  m_saozEfmEdit->apply(&(instr->saozefm));
  m_rasasEdit->apply(&(instr->rasas));
  m_pdasiEasoeEdit->apply(&(instr->pdasieasoe));
  m_pdasiOsmaEdit->apply(&(instr->pdasiosma));
  m_uoftEdit->apply(&(instr->uoft));
  m_noaaEdit->apply(&(instr->noaa));
}


//--------------------------------------------------------
// Specific Instrument Editors... TODO

static const int cSuggestedColumnZeroWidth = 120; // try and keep editor layout
static const int cSuggestedColumnTwoWidth  = 100; // consistent
static const int cDetSizeEditWidth         = 70;
static const Qt::Alignment cLabelAlign     = Qt::AlignRight;

//--------------------------------------------------------

CWCalibInstrEdit::CWCalibInstrEdit(QWidget *parent) :
  QFrame(parent)
{
}

CWCalibInstrEdit::~CWCalibInstrEdit()
{
}

void CWCalibInstrEdit::slotCalibBrowse()
{
  QString file = QFileDialog::getOpenFileName(this, "Select Calibration File", QString(),
					      "Calibration File (*.clb);;All Files (*)");

  if (!file.isEmpty())
    m_calibEdit->setText(file);
}

void CWCalibInstrEdit::slotInstrBrowse()
{
  QString file = QFileDialog::getOpenFileName(this, "Select Instrument Function File", QString(),
					      "Instrument Function File (*.ins);;All Files (*)");
  
  if (!file.isEmpty())
    m_instrEdit->setText(file);
}

//--------------------------------------------------------

CWInstrAsciiEdit::CWInstrAsciiEdit(const struct instrumental_ascii *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
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

  bottomLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  bottomLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(bottomLayout);

  // initial values
  QString tmpStr;

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  // format
  if (d->format == PRJCT_INSTR_ASCII_FORMAT_LINE)
    m_lineRadioButton->setChecked(true);
  else if (d->format == PRJCT_INSTR_ASCII_FORMAT_COLUMN)
    m_columnRadioButton->setChecked(true);

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
  if (m_lineRadioButton->isChecked()) d->format = PRJCT_INSTR_ASCII_FORMAT_LINE;
  if (m_columnRadioButton->isChecked()) d->format = PRJCT_INSTR_ASCII_FORMAT_COLUMN;

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

//--------------------------------------------------------

CWInstrLoggerEdit::CWInstrLoggerEdit(const struct instrumental_logger *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  QGridLayout *mainLayout = new QGridLayout(this);
  
  // Spectral Type
  mainLayout->addWidget(new QLabel("Spectral Type", this), 0, 0);
  m_spectralTypeCombo = new QComboBox(this);
  m_spectralTypeCombo->addItem("All", QVariant(PRJCT_INSTR_IASB_TYPE_ALL));
  m_spectralTypeCombo->addItem("Zenithal", QVariant(PRJCT_INSTR_IASB_TYPE_ZENITHAL));
  m_spectralTypeCombo->addItem("Off-Axis", QVariant(PRJCT_INSTR_IASB_TYPE_OFFAXIS));
  mainLayout->addWidget(m_spectralTypeCombo, 0, 1);

  // Azimuth
  mainLayout->addWidget(new QLabel("With Azimuth Angle", this), 1, 0);
  m_aziCheck = new QCheckBox(this);
  mainLayout->addWidget(m_aziCheck, 1, 1);
  
  // files
  mainLayout->addWidget(new QLabel("Calibration File", this), 2, 0);
  m_calibEdit = new QLineEdit(this);
  m_calibEdit->setMaxLength(sizeof(d->calibrationFile)-1);
  mainLayout->addWidget(m_calibEdit, 2, 1);
  QPushButton *calibBrowseBtn = new QPushButton("Browse...", this);
  mainLayout->addWidget(calibBrowseBtn, 2, 2);

  mainLayout->addWidget(new QLabel("Instr. Function File", this), 3, 0);
  m_instrEdit = new QLineEdit(this);
  m_instrEdit->setMaxLength(sizeof(d->instrFunctionFile)-1);
  mainLayout->addWidget(m_instrEdit, 3, 1);
  QPushButton *instrBrowseBtn = new QPushButton("Browse...", this);
  mainLayout->addWidget(instrBrowseBtn, 3, 2);

  mainLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  mainLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  // initial values

  // spectral type
  int index = m_spectralTypeCombo->findData(QVariant(d->spectralType));
  if (index != -1)
    m_spectralTypeCombo->setCurrentIndex(index);

  // flag
  m_aziCheck->setCheckState(d->flagAzimuthAngle ? Qt::Checked : Qt::Unchecked);

  // files
  m_calibEdit->setText(QString(d->calibrationFile));
  m_instrEdit->setText(QString(d->instrFunctionFile));

  // connections
  connect(calibBrowseBtn, SIGNAL(clicked()), this, SLOT(slotCalibBrowse()));
  connect(instrBrowseBtn, SIGNAL(clicked()), this, SLOT(slotInstrBrowse()));
  
}

CWInstrLoggerEdit::~CWInstrLoggerEdit()
{
}
  
void CWInstrLoggerEdit::apply(struct instrumental_logger *d) const
{
  // spectral type
  d->spectralType = m_spectralTypeCombo->itemData(m_spectralTypeCombo->currentIndex()).toInt();

  // flags
  d->flagAzimuthAngle = m_aziCheck->isChecked() ? 1 : 0;

  // files
  strcpy(d->calibrationFile, m_calibEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_instrEdit->text().toAscii().data());  
}

//--------------------------------------------------------

CWInstrActonEdit::CWInstrActonEdit(const struct instrumental_acton *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  QGridLayout *mainLayout = new QGridLayout(this);
  
  // NILU Type
  mainLayout->addWidget(new QLabel("NILU Format Type", this), 0, 0);
  m_niluTypeCombo = new QComboBox(this);
  m_niluTypeCombo->addItem("Old", QVariant(PRJCT_INSTR_NILU_FORMAT_OLD));
  m_niluTypeCombo->addItem("New", QVariant(PRJCT_INSTR_NILU_FORMAT_NEW));
  mainLayout->addWidget(m_niluTypeCombo, 0, 1);

  // files
  mainLayout->addWidget(new QLabel("Calibration File", this), 1, 0);
  m_calibEdit = new QLineEdit(this);
  m_calibEdit->setMaxLength(sizeof(d->calibrationFile)-1);
  mainLayout->addWidget(m_calibEdit, 1, 1);
  QPushButton *calibBrowseBtn = new QPushButton("Browse...", this);
  mainLayout->addWidget(calibBrowseBtn, 1, 2);

  mainLayout->addWidget(new QLabel("Instr. Function File", this), 2, 0);
  m_instrEdit = new QLineEdit(this);
  m_instrEdit->setMaxLength(sizeof(d->instrFunctionFile)-1);
  mainLayout->addWidget(m_instrEdit, 2, 1);
  QPushButton *instrBrowseBtn = new QPushButton("Browse...", this);
  mainLayout->addWidget(instrBrowseBtn, 2, 2);

  mainLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  mainLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  // initial values

  // nilu type
  int index = m_niluTypeCombo->findData(QVariant(d->niluType));
  if (index != -1)
    m_niluTypeCombo->setCurrentIndex(index);

  // files
  m_calibEdit->setText(QString(d->calibrationFile));
  m_instrEdit->setText(QString(d->instrFunctionFile));

  // connections
  connect(calibBrowseBtn, SIGNAL(clicked()), this, SLOT(slotCalibBrowse()));
  connect(instrBrowseBtn, SIGNAL(clicked()), this, SLOT(slotInstrBrowse()));
  
}

CWInstrActonEdit::~CWInstrActonEdit()
{
}
  
void CWInstrActonEdit::apply(struct instrumental_acton *d) const
{
  // nilu type
  d->niluType = m_niluTypeCombo->itemData(m_niluTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_calibEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_instrEdit->text().toAscii().data());  
}

//--------------------------------------------------------

CWInstrSaozEdit::CWInstrSaozEdit(const struct instrumental_saoz *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  // Spectral Type
  gridLayout->addWidget(new QLabel("Spectral Type", this), 0, 0);
  m_spectralTypeCombo = new QComboBox(this);
  m_spectralTypeCombo->addItem("Zenithal", QVariant(PRJCT_INSTR_SAOZ_TYPE_ZENITHAL));
  m_spectralTypeCombo->addItem("Pointed", QVariant(PRJCT_INSTR_SAOZ_TYPE_POINTED));
  gridLayout->addWidget(m_spectralTypeCombo, 0, 1);

  // files
  gridLayout->addWidget(new QLabel("Calibration File", this), 1, 0);
  m_calibEdit = new QLineEdit(this);
  m_calibEdit->setMaxLength(sizeof(d->calibrationFile)-1);
  gridLayout->addWidget(m_calibEdit, 1, 1);
  QPushButton *calibBrowseBtn = new QPushButton("Browse...", this);
  gridLayout->addWidget(calibBrowseBtn, 1, 2);

  gridLayout->addWidget(new QLabel("Instr. Function File", this), 2, 0);
  m_instrEdit = new QLineEdit(this);
  m_instrEdit->setMaxLength(sizeof(d->instrFunctionFile)-1);
  gridLayout->addWidget(m_instrEdit, 2, 1);
  QPushButton *instrBrowseBtn = new QPushButton("Browse...", this);
  gridLayout->addWidget(instrBrowseBtn, 2, 2);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // spectral type
  int index = m_spectralTypeCombo->findData(QVariant(d->spectralType));
  if (index != -1)
    m_spectralTypeCombo->setCurrentIndex(index);

  // files
  m_calibEdit->setText(QString(d->calibrationFile));
  m_instrEdit->setText(QString(d->instrFunctionFile));

  // connections
  connect(calibBrowseBtn, SIGNAL(clicked()), this, SLOT(slotCalibBrowse()));
  connect(instrBrowseBtn, SIGNAL(clicked()), this, SLOT(slotInstrBrowse()));
  
}

CWInstrSaozEdit::~CWInstrSaozEdit()
{
}
  
void CWInstrSaozEdit::apply(struct instrumental_saoz *d) const
{
  // spectral type
  d->spectralType = m_spectralTypeCombo->itemData(m_spectralTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_calibEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_instrEdit->text().toAscii().data());  
}

//--------------------------------------------------------

CWInstrRasasEdit::CWInstrRasasEdit(const struct instrumental_rasas *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  // files
  gridLayout->addWidget(new QLabel("Calibration File", this), 0, 0);
  m_calibEdit = new QLineEdit(this);
  m_calibEdit->setMaxLength(sizeof(d->calibrationFile)-1);
  gridLayout->addWidget(m_calibEdit, 0, 1);
  QPushButton *calibBrowseBtn = new QPushButton("Browse...", this);
  gridLayout->addWidget(calibBrowseBtn, 0, 2);

  gridLayout->addWidget(new QLabel("Instr. Function File", this), 1, 0);
  m_instrEdit = new QLineEdit(this);
  m_instrEdit->setMaxLength(sizeof(d->instrFunctionFile)-1);
  gridLayout->addWidget(m_instrEdit, 1, 1);
  QPushButton *instrBrowseBtn = new QPushButton("Browse...", this);
  gridLayout->addWidget(instrBrowseBtn, 1, 2);

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // files
  m_calibEdit->setText(QString(d->calibrationFile));
  m_instrEdit->setText(QString(d->instrFunctionFile));

  // connections
  connect(calibBrowseBtn, SIGNAL(clicked()), this, SLOT(slotCalibBrowse()));
  connect(instrBrowseBtn, SIGNAL(clicked()), this, SLOT(slotInstrBrowse()));
  
}

CWInstrRasasEdit::~CWInstrRasasEdit()
{
}
  
void CWInstrRasasEdit::apply(struct instrumental_rasas *d) const
{
  // files
  strcpy(d->calibrationFile, m_calibEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_instrEdit->text().toAscii().data());  
}

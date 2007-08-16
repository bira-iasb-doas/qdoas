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
  m_formatCombo->addItem("Logger (PDA, CCD or HAMAMATSU)", QVariant(PRJCT_INSTR_FORMAT_LOGGER));

  // acton
  m_actonEdit = new CWInstrActonEdit(&(instr->acton));
  m_formatStack->addWidget(m_actonEdit);
  m_formatCombo->addItem("Acton (NILU)", QVariant(PRJCT_INSTR_FORMAT_ACTON));

  // pdaegg
  m_pdaEggEdit = new CWInstrLoggerEdit(&(instr->pdaegg));
  m_formatStack->addWidget(m_pdaEggEdit);
  m_formatCombo->addItem("PDA EG&G (Sept. 94 until now)", QVariant(PRJCT_INSTR_FORMAT_PDAEGG));

  // pdaeggold
  m_pdaEggOldEdit = new CWInstrLoggerEdit(&(instr->pdaeggold));
  m_formatStack->addWidget(m_pdaEggOldEdit);
  m_formatCombo->addItem("PDA EG&G (Spring 94)", QVariant(PRJCT_INSTR_FORMAT_PDAEGG_OLD));

  // pdaeggulb
  m_pdaEggUlbEdit = new CWInstrPdaEggUlbEdit(&(instr->pdaeggulb));
  m_formatStack->addWidget(m_pdaEggUlbEdit);
  m_formatCombo->addItem("PDA EG&G (ULB)", QVariant(PRJCT_INSTR_FORMAT_PDAEGG_ULB));

  // ccdohp96
  m_ccdOhp96Edit = new CWInstrCcdEdit(&(instr->ccdohp96));
  m_formatStack->addWidget(m_ccdOhp96Edit);
  m_formatCombo->addItem("CCD all tracks", QVariant(PRJCT_INSTR_FORMAT_CCD_OHP_96));

  // ccdha94
  m_ccdHa94Edit = new CWInstrCcdEdit(&(instr->ccdha94));
  m_formatStack->addWidget(m_ccdHa94Edit);
  m_formatCombo->addItem("CCD Sesame I", QVariant(PRJCT_INSTR_FORMAT_CCD_HA_94));

  // ccdulb
  m_ccdUlbEdit = new CWInstrCcdUlbEdit(&(instr->ccdulb));
  m_formatStack->addWidget(m_ccdUlbEdit);
  m_formatCombo->addItem("CCD (ULB)", QVariant(PRJCT_INSTR_FORMAT_CCD_ULB));

  // saozvis
  m_saozVisEdit = new CWInstrSaozEdit(&(instr->saozvis));
  m_formatStack->addWidget(m_saozVisEdit);
  m_formatCombo->addItem("SAOZ Visible", QVariant(PRJCT_INSTR_FORMAT_SAOZ_VIS));

  // saozuv
  m_saozUvEdit = new CWInstrSaozEdit(&(instr->saozuv));
  m_formatStack->addWidget(m_saozUvEdit);
  m_formatCombo->addItem("SAOZ UV", QVariant(PRJCT_INSTR_FORMAT_SAOZ_UV));

  // saozefm
  m_saozEfmEdit = new CWInstrRasasEdit(&(instr->saozefm));
  m_formatStack->addWidget(m_saozEfmEdit);
  m_formatCombo->addItem("SAOZ EFM", QVariant(PRJCT_INSTR_FORMAT_SAOZ_EFM));

  // MFC.... TODO

  // rasas
  m_rasasEdit = new CWInstrRasasEdit(&(instr->rasas));
  m_formatStack->addWidget(m_rasasEdit);
  m_formatCombo->addItem("RASAS (INTA)", QVariant(PRJCT_INSTR_FORMAT_RASAS));

   // pdasieasoe
  m_pdasiEasoeEdit = new CWInstrRasasEdit(&(instr->pdasieasoe));
  m_formatStack->addWidget(m_pdasiEasoeEdit);
  m_formatCombo->addItem("EASOE", QVariant(PRJCT_INSTR_FORMAT_PDASI_EASOE));

  // pdasiosma
  m_pdasiOsmaEdit = new CWInstrLoggerEdit(&(instr->pdasiosma));
  m_formatStack->addWidget(m_pdasiOsmaEdit);
  m_formatCombo->addItem("PDA SI (OSMA)", QVariant(PRJCT_INSTR_FORMAT_PDASI_OSMA));

  // ccdeev
  m_ccdEevEdit = new CWInstrCcdEevEdit(&(instr->ccdeev));
  m_formatStack->addWidget(m_ccdEevEdit);
  m_formatCombo->addItem("CCD EEV (BIRA-IASB, NILU)", QVariant(PRJCT_INSTR_FORMAT_CCD_EEV));

  // opus
  m_opusEdit = new CWInstrOpusEdit(&(instr->opus));
  m_formatStack->addWidget(m_opusEdit);
  m_formatCombo->addItem("OPUS", QVariant(PRJCT_INSTR_FORMAT_OPUS));

  // gdpascii
  m_gdpAsciiEdit = new CWInstrGdpEdit(&(instr->gdpascii));
  m_formatStack->addWidget(m_gdpAsciiEdit);
  m_formatCombo->addItem("GDP (ASCII)", QVariant(PRJCT_INSTR_FORMAT_GDP_ASCII));

  // gdpbin
  m_gdpBinEdit = new CWInstrGdpEdit(&(instr->gdpbin));
  m_formatStack->addWidget(m_gdpBinEdit);
  m_formatCombo->addItem("GDP (Binary)", QVariant(PRJCT_INSTR_FORMAT_GDP_BIN));

  // sciahds
  m_sciaHdfEdit = new CWInstrSciaEdit(&(instr->sciahdf));
  m_formatStack->addWidget(m_sciaHdfEdit);
  m_formatCombo->addItem("SCIAMACHY L1C (HDF format)", QVariant(PRJCT_INSTR_FORMAT_SCIA_HDF));

  // sciapds
  m_sciaPdsEdit = new CWInstrSciaEdit(&(instr->sciapds));
  m_formatStack->addWidget(m_sciaPdsEdit);
  m_formatCombo->addItem("SCIAMACHY L1C (PDS format)", QVariant(PRJCT_INSTR_FORMAT_SCIA_PDS));

  // uoft
  m_uoftEdit = new CWInstrRasasEdit(&(instr->uoft));
  m_formatStack->addWidget(m_uoftEdit);
  m_formatCombo->addItem("CCD (University of Toronto)", QVariant(PRJCT_INSTR_FORMAT_UOFT));

  // noaa
  m_noaaEdit = new CWInstrRasasEdit(&(instr->noaa));
  m_formatStack->addWidget(m_noaaEdit);
  m_formatCombo->addItem("NOAA", QVariant(PRJCT_INSTR_FORMAT_NOAA));

  // omi
  m_omiEdit = new CWInstrOmiEdit(&(instr->omi));
  m_formatStack->addWidget(m_omiEdit);
  m_formatCombo->addItem("OMI", QVariant(PRJCT_INSTR_FORMAT_OMI));

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
  m_pdaEggOldEdit->apply(&(instr->pdaeggold));
  m_pdaEggUlbEdit->apply(&(instr->pdaeggulb));
  m_ccdOhp96Edit->apply(&(instr->ccdohp96));
  m_ccdHa94Edit->apply(&(instr->ccdha94));
  m_ccdUlbEdit->apply(&(instr->ccdulb));
  m_saozVisEdit->apply(&(instr->saozvis));
  m_saozUvEdit->apply(&(instr->saozuv));
  m_saozEfmEdit->apply(&(instr->saozefm));
  m_rasasEdit->apply(&(instr->rasas));
  m_pdasiEasoeEdit->apply(&(instr->pdasieasoe));
  m_pdasiOsmaEdit->apply(&(instr->pdasiosma));
  m_ccdEevEdit->apply(&(instr->ccdeev));
  m_gdpAsciiEdit->apply(&(instr->gdpascii));
  m_gdpBinEdit->apply(&(instr->gdpbin));
  m_sciaHdfEdit->apply(&(instr->sciahdf));
  m_sciaPdsEdit->apply(&(instr->sciapds));
  m_uoftEdit->apply(&(instr->uoft));
  m_noaaEdit->apply(&(instr->noaa));
  m_omiEdit->apply(&(instr->omi));
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

void CWCalibInstrEdit::helperConstructCalInsFileWidgets(QGridLayout *gridLayout, int &row,
							const char *calib, int lenCalib,
							const char *instr, int lenInstr)
{
  // Helper that constructs the file editing widgets and places them in a grid layout.
  // row is the first 'free row' in the grid on entry and is updated to be the next
  // 'free row' on exit.

  // files
  gridLayout->addWidget(new QLabel("Calibration File", this), row, 0);
  m_calibEdit = new QLineEdit(this);
  m_calibEdit->setMaxLength(lenCalib-1);
  gridLayout->addWidget(m_calibEdit, row, 1);
  QPushButton *calibBrowseBtn = new QPushButton("Browse...", this);
  gridLayout->addWidget(calibBrowseBtn, row, 2);
  ++row;

  gridLayout->addWidget(new QLabel("Instr. Function", this), row, 0);
  m_instrEdit = new QLineEdit(this);
  m_instrEdit->setMaxLength(lenInstr-1);
  gridLayout->addWidget(m_instrEdit, row, 1);
  QPushButton *instrBrowseBtn = new QPushButton("Browse...", this);
  gridLayout->addWidget(instrBrowseBtn, row, 2);
  ++row;

  // initialise the values
  m_calibEdit->setText(QString(calib));
  m_instrEdit->setText(QString(instr));

  // connections
  connect(calibBrowseBtn, SIGNAL(clicked()), this, SLOT(slotCalibBrowse()));
  connect(instrBrowseBtn, SIGNAL(clicked()), this, SLOT(slotInstrBrowse()));
}
  
//--------------------------------------------------------

CWAllFilesEdit::CWAllFilesEdit(QWidget *parent) :
  CWCalibInstrEdit(parent)
{
}

CWAllFilesEdit::~CWAllFilesEdit()
{
}

void CWAllFilesEdit::slotInterPixelVariabilityBrowse()
{
  QString file = QFileDialog::getOpenFileName(this, "Select Inter-Pixel Variability File", QString(),
					      "Inter-Pixel Variability File (*.ipv);;All Files (*)");
  
  if (!file.isEmpty())
    m_ipvEdit->setText(file);
}

void CWAllFilesEdit::slotDetectorNonLinearityBrowse()
{
  QString file = QFileDialog::getOpenFileName(this, "Select Detector Non-Linearity File", QString(),
					      "Detector Non-Linearity File (*.dnl);;All Files (*)");
  
  if (!file.isEmpty())
    m_dnlEdit->setText(file);
}

void CWAllFilesEdit::helperConstructIpvDnlFileWidgets(QGridLayout *gridLayout, int &row,
						      const char *ipv, int lenIpv,
						      const char *dnl, int lenDnl)
{
  // Helper that constructs the file editing widgets and places them in a grid layout.
  // row is the first 'free row' in the grid on entry and is updated to be the next
  // 'free row' on exit.

  gridLayout->addWidget(new QLabel("Inter-Pixel Var.", this), row, 0);
  m_ipvEdit = new QLineEdit(this);
  m_ipvEdit->setMaxLength(lenIpv-1);
  gridLayout->addWidget(m_ipvEdit, row, 1);
  QPushButton *ipvBrowseBtn = new QPushButton("Browse...", this);
  gridLayout->addWidget(ipvBrowseBtn, row, 2);
  ++row;

  gridLayout->addWidget(new QLabel("Det. Non-Linearity", this), row, 0);
  m_dnlEdit = new QLineEdit(this);
  m_dnlEdit->setMaxLength(lenDnl-1);
  gridLayout->addWidget(m_dnlEdit, row, 1);
  QPushButton *dnlBrowseBtn = new QPushButton("Browse...", this);
  gridLayout->addWidget(dnlBrowseBtn, row, 2);
  ++row;

  // initialise the values
  m_ipvEdit->setText(QString(ipv));
  m_dnlEdit->setText(QString(dnl));

  // connections
  connect(ipvBrowseBtn, SIGNAL(clicked()), this, SLOT(slotInterPixelVariabilityBrowse()));
  connect(dnlBrowseBtn, SIGNAL(clicked()), this, SLOT(slotDetectorNonLinearityBrowse()));
}

void CWAllFilesEdit::helperConstructFileWidgets(QGridLayout *gridLayout, int &row,
						const char *calib, int lenCalib,
						const char *instr, int lenInstr,
						const char *ipv, int lenIpv,
						const char *dnl, int lenDnl)
{
  // Helper that constructs the file editing widgets and places them in a grid layout.
  // row is the first 'free row' in the grid on entry and is updated to be the next
  // 'free row' on exit.

  helperConstructCalInsFileWidgets(gridLayout, row, calib, lenCalib, instr, lenInstr);
  helperConstructIpvDnlFileWidgets(gridLayout, row, ipv, lenIpv, dnl, lenDnl);
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

  m_zenCheck = new QCheckBox("Solar Zenith Angle", flagsGroup);
  flagsLayout->addWidget(m_zenCheck, 0, 0);
  m_dateCheck = new QCheckBox("DD/MM/YYYY", flagsGroup);
  flagsLayout->addWidget(m_dateCheck, 0, 1);

  m_aziCheck = new QCheckBox("Azimuth Viewing Angle", flagsGroup);
  flagsLayout->addWidget(m_aziCheck, 1, 0);
  m_timeCheck = new QCheckBox("Decimal Time", flagsGroup);
  flagsLayout->addWidget(m_timeCheck, 1, 1);

  m_eleCheck = new QCheckBox("Elevation Viewing Angle", flagsGroup);
  flagsLayout->addWidget(m_eleCheck, 2, 0);
  m_lambdaCheck = new QCheckBox("Lambda", flagsGroup);
  flagsLayout->addWidget(m_lambdaCheck, 2, 1);

  groupLayout->addWidget(flagsGroup);

  mainLayout->addLayout(groupLayout);

  // bottom layout - det. size and files
  int row = 0;
  QGridLayout *bottomLayout = new QGridLayout;

  bottomLayout->addWidget(new QLabel("Detector Size", this), row, 0);
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cDetSizeEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  bottomLayout->addWidget(m_detSizeEdit, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(bottomLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));
  
  bottomLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  bottomLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(bottomLayout);
  mainLayout->addStretch(1);

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
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  // Spectral Type
  gridLayout->addWidget(new QLabel("Spectral Type", this), row, 0);
  m_spectralTypeCombo = new QComboBox(this);
  m_spectralTypeCombo->addItem("All", QVariant(PRJCT_INSTR_IASB_TYPE_ALL));
  m_spectralTypeCombo->addItem("Zenithal", QVariant(PRJCT_INSTR_IASB_TYPE_ZENITHAL));
  m_spectralTypeCombo->addItem("Off-Axis", QVariant(PRJCT_INSTR_IASB_TYPE_OFFAXIS));
  gridLayout->addWidget(m_spectralTypeCombo, row, 1);
  ++row;

  // Azimuth
  m_aziCheck = new QCheckBox("Format with azimuth angle", this);
  gridLayout->addWidget(m_aziCheck, row, 1, 1, 2);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));
  
  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // spectral type
  int index = m_spectralTypeCombo->findData(QVariant(d->spectralType));
  if (index != -1)
    m_spectralTypeCombo->setCurrentIndex(index);

  // flag
  m_aziCheck->setCheckState(d->flagAzimuthAngle ? Qt::Checked : Qt::Unchecked);
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
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  // NILU Type
  gridLayout->addWidget(new QLabel("NILU Format Type", this), row, 0);
  m_niluTypeCombo = new QComboBox(this);
  m_niluTypeCombo->addItem("Old format", QVariant(PRJCT_INSTR_NILU_FORMAT_OLD));
  m_niluTypeCombo->addItem("New format", QVariant(PRJCT_INSTR_NILU_FORMAT_NEW));
  gridLayout->addWidget(m_niluTypeCombo, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));
  
  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // nilu type
  int index = m_niluTypeCombo->findData(QVariant(d->niluType));
  if (index != -1)
    m_niluTypeCombo->setCurrentIndex(index);

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
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  // Spectral Type
  gridLayout->addWidget(new QLabel("Spectral Type", this), row, 0);
  m_spectralTypeCombo = new QComboBox(this);
  m_spectralTypeCombo->addItem("Zenithal", QVariant(PRJCT_INSTR_SAOZ_TYPE_ZENITHAL));
  m_spectralTypeCombo->addItem("Pointed", QVariant(PRJCT_INSTR_SAOZ_TYPE_POINTED));
  gridLayout->addWidget(m_spectralTypeCombo, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));
  
  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // spectral type
  int index = m_spectralTypeCombo->findData(QVariant(d->spectralType));
  if (index != -1)
    m_spectralTypeCombo->setCurrentIndex(index);

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
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));
  
  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
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

//--------------------------------------------------------

CWInstrCcdEdit::CWInstrCcdEdit(const struct instrumental_ccd *d, QWidget *parent) :
  CWAllFilesEdit(parent)
{
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  // files
  helperConstructFileWidgets(gridLayout, row,
			     d->calibrationFile, sizeof(d->calibrationFile),
			     d->instrFunctionFile, sizeof(d->instrFunctionFile),
			     d->interPixelVariabilityFile, sizeof(d->interPixelVariabilityFile),
			     d->detectorNonLinearityFile, sizeof(d->detectorNonLinearityFile));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);
}

CWInstrCcdEdit::~CWInstrCcdEdit()
{
}
  
void CWInstrCcdEdit::apply(struct instrumental_ccd *d) const
{
  // files
  strcpy(d->calibrationFile, m_calibEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_instrEdit->text().toAscii().data());  
  strcpy(d->interPixelVariabilityFile, m_ipvEdit->text().toAscii().data());  
  strcpy(d->detectorNonLinearityFile, m_dnlEdit->text().toAscii().data());  
}

//--------------------------------------------------------

CWInstrCcdUlbEdit::CWInstrCcdUlbEdit(const struct instrumental_ccdulb *d, QWidget *parent) :
  CWAllFilesEdit(parent)
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  // grating
  gridLayout->addWidget(new QLabel("Grating number"), row, 0);
  m_gratingEdit = new QLineEdit(this);
  m_gratingEdit->setFixedWidth(cDetSizeEditWidth);
  m_gratingEdit->setValidator(new QIntValidator(0, 99, m_gratingEdit));
  gridLayout->addWidget(m_gratingEdit, row, 1);
  ++row;

  // central wavelength
  gridLayout->addWidget(new QLabel("Central Wavelength (nm)"), row, 0);
  m_cenLambdaEdit = new QLineEdit(this);
  m_cenLambdaEdit->setFixedWidth(cDetSizeEditWidth);
  m_cenLambdaEdit->setValidator(new QIntValidator(0, 999, m_cenLambdaEdit));
  gridLayout->addWidget(m_cenLambdaEdit, row, 1);
  ++row;

  // files - non standard files ..
  gridLayout->addWidget(new QLabel("Calibraton File", this), row, 0);
  m_calibEdit = new QLineEdit(this);
  m_calibEdit->setMaxLength(sizeof(d->calibrationFile)-1);
  gridLayout->addWidget(m_calibEdit, row, 1);
  QPushButton *calibBrowseBtn = new QPushButton("Browse...", this);
  gridLayout->addWidget(calibBrowseBtn, row, 2);
  ++row;

  gridLayout->addWidget(new QLabel("Offset", this), row, 0);
  m_instrEdit = new QLineEdit(this);
  m_instrEdit->setMaxLength(sizeof(d->offsetFile)-1);
  gridLayout->addWidget(m_instrEdit, row, 1);
  QPushButton *offsetBrowseBtn = new QPushButton("Browse...", this);
  gridLayout->addWidget(offsetBrowseBtn, row, 2);
  ++row;

  helperConstructIpvDnlFileWidgets(gridLayout, row,
				   d->interPixelVariabilityFile, sizeof(d->interPixelVariabilityFile),
				   d->detectorNonLinearityFile, sizeof(d->detectorNonLinearityFile));
  
  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initialise values
  m_gratingEdit->setText(tmpStr.setNum(d->grating));
  m_cenLambdaEdit->setText(tmpStr.setNum(d->centralWavelength));

  m_calibEdit->setText(QString(d->calibrationFile));
  m_instrEdit->setText(QString(d->offsetFile));
  
  // connections
  connect(calibBrowseBtn, SIGNAL(clicked()), this, SLOT(slotCalibBrowse()));
  connect(offsetBrowseBtn, SIGNAL(clicked()), this, SLOT(slotInstrBrowse()));
}

CWInstrCcdUlbEdit::~CWInstrCcdUlbEdit()
{
}
  
void CWInstrCcdUlbEdit::apply(struct instrumental_ccdulb *d) const
{
  // grating
  d->grating = m_gratingEdit->text().toInt();

  // central wavlength
  d->centralWavelength = m_cenLambdaEdit->text().toInt();

  // files
  strcpy(d->calibrationFile, m_calibEdit->text().toAscii().data());
  strcpy(d->offsetFile, m_instrEdit->text().toAscii().data());  
  strcpy(d->interPixelVariabilityFile, m_ipvEdit->text().toAscii().data());  
  strcpy(d->detectorNonLinearityFile, m_dnlEdit->text().toAscii().data());  
}

//--------------------------------------------------------

CWInstrPdaEggUlbEdit::CWInstrPdaEggUlbEdit(const struct instrumental_pdaeggulb *d, QWidget *parent) :
  CWAllFilesEdit(parent)
{
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  // Curve Type
  gridLayout->addWidget(new QLabel("Curve Type", this), row, 0);
  m_curveTypeCombo = new QComboBox(this);
  m_curveTypeCombo->addItem("Manual", QVariant(PRJCT_INSTR_ULB_TYPE_MANUAL));
  m_curveTypeCombo->addItem("High", QVariant(PRJCT_INSTR_ULB_TYPE_HIGH));
  m_curveTypeCombo->addItem("Low", QVariant(PRJCT_INSTR_ULB_TYPE_LOW));
  gridLayout->addWidget(m_curveTypeCombo, row, 1);
  ++row;

  // files
  helperConstructFileWidgets(gridLayout, row,
			     d->calibrationFile, sizeof(d->calibrationFile),
			     d->instrFunctionFile, sizeof(d->instrFunctionFile),
			     d->interPixelVariabilityFile, sizeof(d->interPixelVariabilityFile),
			     d->detectorNonLinearityFile, sizeof(d->detectorNonLinearityFile));
  
  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // curve type
  int index = m_curveTypeCombo->findData(QVariant(d->curveType));
  if (index != -1)
    m_curveTypeCombo->setCurrentIndex(index);

}

CWInstrPdaEggUlbEdit::~CWInstrPdaEggUlbEdit()
{
}
  
void CWInstrPdaEggUlbEdit::apply(struct instrumental_pdaeggulb *d) const
{
  // curve type
  d->curveType = m_curveTypeCombo->itemData(m_curveTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_calibEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_instrEdit->text().toAscii().data());  
  strcpy(d->interPixelVariabilityFile, m_ipvEdit->text().toAscii().data());  
  strcpy(d->detectorNonLinearityFile, m_dnlEdit->text().toAscii().data());  
}

//--------------------------------------------------------

CWInstrCcdEevEdit::CWInstrCcdEevEdit(const struct instrumental_ccdeev *d, QWidget *parent) :
  CWAllFilesEdit(parent)
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  // detector size
  gridLayout->addWidget(new QLabel("Detector Size", this), row, 0);
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cDetSizeEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  gridLayout->addWidget(m_detSizeEdit, row, 1);
  ++row;

  // files
  CWCalibInstrEdit::helperConstructCalInsFileWidgets(gridLayout, row,
						     d->calibrationFile, sizeof(d->calibrationFile),
						     d->instrFunctionFile, sizeof(d->instrFunctionFile));

  // non-standard files...
  gridLayout->addWidget(new QLabel("Straylight Correction", this), row, 0);
  m_ipvEdit = new QLineEdit(this);
  m_ipvEdit->setMaxLength(sizeof(d->straylightCorrectionFile)-1);
  gridLayout->addWidget(m_ipvEdit, row, 1);
  QPushButton *strayBrowseBtn = new QPushButton("Browse...", this);
  gridLayout->addWidget(strayBrowseBtn, row, 2);
  ++row;

  gridLayout->addWidget(new QLabel("Det. Non-Linearity", this), row, 0);
  m_dnlEdit = new QLineEdit(this);
  m_dnlEdit->setMaxLength(sizeof(d->detectorNonLinearityFile)-1);
  gridLayout->addWidget(m_dnlEdit, row, 1);
  QPushButton *dnlBrowseBtn = new QPushButton("Browse...", this);
  gridLayout->addWidget(dnlBrowseBtn, row, 2);
  ++row;

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initialise the values

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  m_ipvEdit->setText(QString(d->straylightCorrectionFile));
  m_dnlEdit->setText(QString(d->detectorNonLinearityFile));

  // connections
  connect(strayBrowseBtn, SIGNAL(clicked()), this, SLOT(slotInterPixelVariabilityBrowse()));
  connect(dnlBrowseBtn, SIGNAL(clicked()), this, SLOT(slotDetectorNonLinearityBrowse()));
  
}

CWInstrCcdEevEdit::~CWInstrCcdEevEdit()
{
}
  
void CWInstrCcdEevEdit::apply(struct instrumental_ccdeev *d) const
{
  // detector size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // files
  strcpy(d->calibrationFile, m_calibEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_instrEdit->text().toAscii().data());  
  strcpy(d->straylightCorrectionFile, m_ipvEdit->text().toAscii().data());  
  strcpy(d->detectorNonLinearityFile, m_dnlEdit->text().toAscii().data());  
}

//--------------------------------------------------------

CWInstrOpusEdit::CWInstrOpusEdit(const struct instrumental_opus *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  // detector size and flag transmittance
  gridLayout->addWidget(new QLabel("Detector Size", this), row, 0);
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cDetSizeEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  gridLayout->addWidget(m_detSizeEdit, row, 1);
  m_transmittanceCheck = new QCheckBox("Transmittance", this);
  gridLayout->addWidget(m_transmittanceCheck, row, 2);
  ++row;

  // time shift
  gridLayout->addWidget(new QLabel("Time Shift (hours)", this), row, 0);
  m_timeShiftEdit = new QLineEdit(this);
  m_timeShiftEdit->setFixedWidth(cDetSizeEditWidth);
  m_timeShiftEdit->setValidator(new CDoubleFixedFmtValidator(-24.0, 24.0, 2, m_timeShiftEdit));
  gridLayout->addWidget(m_timeShiftEdit, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initialise values

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);
  
  // transmittance
  m_transmittanceCheck->setCheckState(d->flagTransmittance ? Qt::Checked : Qt::Unchecked);

  // time shift
  tmpStr.setNum(d->timeShift);
  m_timeShiftEdit->validator()->fixup(tmpStr);
  m_timeShiftEdit->setText(tmpStr);

}

CWInstrOpusEdit::~CWInstrOpusEdit()
{
}
  
void CWInstrOpusEdit::apply(struct instrumental_opus *d) const
{
  // detector size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // time shift
  d->timeShift = m_timeShiftEdit->text().toDouble();

  // transmittance
  d->flagTransmittance = (m_transmittanceCheck->checkState() == Qt::Checked) ? 1 : 0;

  // files
  strcpy(d->calibrationFile, m_calibEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_instrEdit->text().toAscii().data());  
}

//--------------------------------------------------------

CWInstrGdpEdit::CWInstrGdpEdit(const struct instrumental_gdp *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // band
  gridLayout->addWidget(new QLabel("Band Type", this), row, 0);
  m_bandTypeCombo = new QComboBox(this);
  m_bandTypeCombo->addItem("Band 1a", QVariant(PRJCT_INSTR_GDP_BAND_1A));
  m_bandTypeCombo->addItem("Band 1b", QVariant(PRJCT_INSTR_GDP_BAND_1B));
  m_bandTypeCombo->addItem("Band 2a", QVariant(PRJCT_INSTR_GDP_BAND_2A));
  m_bandTypeCombo->addItem("Band 2b", QVariant(PRJCT_INSTR_GDP_BAND_2B));
  m_bandTypeCombo->addItem("Band 3", QVariant(PRJCT_INSTR_GDP_BAND_3));
  m_bandTypeCombo->addItem("Band 4", QVariant(PRJCT_INSTR_GDP_BAND_4));
  gridLayout->addWidget(m_bandTypeCombo, row, 1);
  ++row;
  
  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));
  
  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // band
  int index = m_bandTypeCombo->findData(QVariant(d->bandType));
  if (index != -1)
    m_bandTypeCombo->setCurrentIndex(index);
}

CWInstrGdpEdit::~CWInstrGdpEdit()
{
}
  
void CWInstrGdpEdit::apply(struct instrumental_gdp *d) const
{
  // band
  d->bandType = m_bandTypeCombo->itemData(m_bandTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_calibEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_instrEdit->text().toAscii().data());  
}

//--------------------------------------------------------------------------

CWInstrSciaEdit::CWInstrSciaEdit(const struct instrumental_scia *d, QWidget *parent) :
  CWCalibInstrEdit(parent),
  m_clusterOffset(0)
{
  QString tmpStr;
  int i;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  
  // channel
  gridLayout->addWidget(new QLabel("Channel", this), row, 0);
  m_channelCombo = new QComboBox(this);
  m_channelCombo->addItem("Channel 1", QVariant(PRJCT_INSTR_SCIA_CHANNEL_1));
  m_channelCombo->addItem("Channel 2", QVariant(PRJCT_INSTR_SCIA_CHANNEL_2));
  m_channelCombo->addItem("Channel 3", QVariant(PRJCT_INSTR_SCIA_CHANNEL_3));
  m_channelCombo->addItem("Channel 4", QVariant(PRJCT_INSTR_SCIA_CHANNEL_4));
  gridLayout->addWidget(m_channelCombo, row, 1);
  ++row;
  
  // clusters
  gridLayout->addWidget(new QLabel("Clusters", this), row, 0);
  QHBoxLayout *checkLayout = new QHBoxLayout;
  for (i=0; i<6; ++i) {
    m_clusterCheck[i] = new QCheckBox(this);
    //m_clusterCheck[i]->setFixedWidth(40);
    checkLayout->addWidget(m_clusterCheck[i]);
  }
  checkLayout->addStretch(1);
  gridLayout->addLayout(checkLayout, row, 1, 1, 2, Qt::AlignLeft);
  ++row;

  // sun reference
  gridLayout->addWidget(new QLabel("Reference", this), row, 0);
  m_referenceEdit = new QLineEdit(this);
  m_referenceEdit->setFixedWidth(50);
  m_referenceEdit->setMaxLength(2);
  gridLayout->addWidget(m_referenceEdit, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // cluster state
  memcpy(m_clusterState, d->clusters, sizeof(m_clusterState));

  // channel
  int index = m_channelCombo->findData(QVariant(d->channel));
  if (index != -1) {
    m_channelCombo->setCurrentIndex(index);
    slotChannelChanged(index);
  }

  m_referenceEdit->setText(QString(d->sunReference));
  
  // connections
  connect(m_channelCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChannelChanged(int)));

  connect(m_clusterCheck[0], SIGNAL(stateChanged(int)), this, SLOT(slotCluster0Changed(int)));
  connect(m_clusterCheck[1], SIGNAL(stateChanged(int)), this, SLOT(slotCluster1Changed(int)));
  connect(m_clusterCheck[2], SIGNAL(stateChanged(int)), this, SLOT(slotCluster2Changed(int)));
  connect(m_clusterCheck[3], SIGNAL(stateChanged(int)), this, SLOT(slotCluster3Changed(int)));
  connect(m_clusterCheck[4], SIGNAL(stateChanged(int)), this, SLOT(slotCluster4Changed(int)));
  connect(m_clusterCheck[5], SIGNAL(stateChanged(int)), this, SLOT(slotCluster5Changed(int)));
}

CWInstrSciaEdit::~CWInstrSciaEdit()
{
}

void CWInstrSciaEdit::apply(struct instrumental_scia *d) const
{
  // channel
  d->channel = m_channelCombo->itemData(m_channelCombo->currentIndex()).toInt();

  // cluster state
  memcpy(d->clusters, m_clusterState, sizeof(d->clusters));

  // sun reference
  strcpy(d->sunReference, m_referenceEdit->text().toAscii().data());

  // files
  strcpy(d->calibrationFile, m_calibEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_instrEdit->text().toAscii().data());  
}
  
void CWInstrSciaEdit::slotChannelChanged(int index)
{
  int i;
  QString str;

  // the index defines the clusters that should be displyed
  switch (index) {
  case 0:
    m_clusterOffset = 2;
    for (i=0; i<4; ++i) {
      m_clusterCheck[i]->setText(str.setNum(m_clusterOffset + i));
      m_clusterCheck[i]->setCheckState(m_clusterState[m_clusterOffset + i] ? Qt::Checked : Qt::Unchecked);
      m_clusterCheck[i]->show();
    }
    for (i=4; i<6; ++i)
      m_clusterCheck[i]->hide();

    break;
  case 1:
    m_clusterOffset = 8;
    for (i=0; i<3; ++i) {
      m_clusterCheck[i]->setText(str.setNum(m_clusterOffset + i));
      m_clusterCheck[i]->setCheckState(m_clusterState[m_clusterOffset + i] ? Qt::Checked : Qt::Unchecked);
      m_clusterCheck[i]->show();
    }
    for (i=3; i<6; ++i)
      m_clusterCheck[i]->hide();

    break;
  case 2:
    m_clusterOffset = 13;
    for (i=0; i<6; ++i) {
      m_clusterCheck[i]->setText(str.setNum(m_clusterOffset + i));
      m_clusterCheck[i]->setCheckState(m_clusterState[m_clusterOffset + i] ? Qt::Checked : Qt::Unchecked);
      m_clusterCheck[i]->show();
    }
    break;
  case 3:
    m_clusterOffset = 22;
    for (i=0; i<6; ++i) {
      m_clusterCheck[i]->setText(str.setNum(m_clusterOffset + i));
      m_clusterCheck[i]->setCheckState(m_clusterState[m_clusterOffset + i] ? Qt::Checked : Qt::Unchecked);
      m_clusterCheck[i]->show();
    }
    break;
  }
}

void CWInstrSciaEdit::slotCluster0Changed(int state)
{
  m_clusterState[m_clusterOffset] = (state == Qt::Checked) ? 1 : 0;
}

void CWInstrSciaEdit::slotCluster1Changed(int state)
{
  m_clusterState[m_clusterOffset+1] = (state == Qt::Checked) ? 1 : 0;
}

void CWInstrSciaEdit::slotCluster2Changed(int state)
{
  m_clusterState[m_clusterOffset+2] = (state == Qt::Checked) ? 1 : 0;
}

void CWInstrSciaEdit::slotCluster3Changed(int state)
{
  m_clusterState[m_clusterOffset+3] = (state == Qt::Checked) ? 1 : 0;
}

void CWInstrSciaEdit::slotCluster4Changed(int state)
{
  m_clusterState[m_clusterOffset+4] = (state == Qt::Checked) ? 1 : 0;
}

void CWInstrSciaEdit::slotCluster5Changed(int state)
{
  m_clusterState[m_clusterOffset+5] = (state == Qt::Checked) ? 1 : 0;
}

//--------------------------------------------------------

CWInstrOmiEdit::CWInstrOmiEdit(const struct instrumental_omi *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // spectral
  gridLayout->addWidget(new QLabel("Spectral Type", this), row, 0);
  m_spectralTypeCombo = new QComboBox(this);
  m_spectralTypeCombo->addItem("UV-1", QVariant(PRJCT_INSTR_OMI_TYPE_UV1));
  m_spectralTypeCombo->addItem("UV-2", QVariant(PRJCT_INSTR_OMI_TYPE_UV2));
  m_spectralTypeCombo->addItem("Visible", QVariant(PRJCT_INSTR_OMI_TYPE_VIS));
  gridLayout->addWidget(m_spectralTypeCombo, row, 1);
  ++row;
  
  // spectral range
  gridLayout->addWidget(new QLabel("Wavelength Range (nm)", this), row, 0);
  QHBoxLayout *rangeLayout = new QHBoxLayout;
  m_minLambdaEdit = new QLineEdit(this);
  m_minLambdaEdit->setFixedWidth(cDetSizeEditWidth);
  m_minLambdaEdit->setValidator(new CDoubleFixedFmtValidator(100.0, 999.9, 1, m_minLambdaEdit));
  rangeLayout->addWidget(m_minLambdaEdit);
  m_maxLambdaEdit = new QLineEdit(this);
  m_maxLambdaEdit->setFixedWidth(cDetSizeEditWidth);
  m_maxLambdaEdit->setValidator(new CDoubleFixedFmtValidator(100.0, 999.9, 1, m_maxLambdaEdit));
  rangeLayout->addWidget(m_maxLambdaEdit);
  rangeLayout->addStretch(1);
  gridLayout->addLayout(rangeLayout, row, 1, 1, 2, Qt::AlignLeft);
  ++row;

  // average
  m_averageCheck = new QCheckBox("Average spectra in tracks", this);
  gridLayout->addWidget(m_averageCheck, row, 1, 1, 2, Qt::AlignLeft);
  ++row;
  
  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));
  
  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initial values

  // spectral
  int index = m_spectralTypeCombo->findData(QVariant(d->spectralType));
  if (index != -1)
    m_spectralTypeCombo->setCurrentIndex(index);

  // wavelength range
  tmpStr.setNum(d->minimumWavelength);
  m_minLambdaEdit->validator()->fixup(tmpStr);
  m_minLambdaEdit->setText(tmpStr);
  tmpStr.setNum(d->maximumWavelength);
  m_maxLambdaEdit->validator()->fixup(tmpStr);
  m_maxLambdaEdit->setText(tmpStr);

  // average
  m_averageCheck->setCheckState(d->flagAverage ? Qt::Checked : Qt::Unchecked);

}

CWInstrOmiEdit::~CWInstrOmiEdit()
{
}
  
void CWInstrOmiEdit::apply(struct instrumental_omi *d) const
{
  // spectral
  d->spectralType = m_spectralTypeCombo->itemData(m_spectralTypeCombo->currentIndex()).toInt();

  // wavelength range
  d->minimumWavelength = m_minLambdaEdit->text().toDouble();
  d->maximumWavelength = m_maxLambdaEdit->text().toDouble();

  d->flagAverage = (m_averageCheck->checkState() == Qt::Checked) ? 1 : 0;

  // files
  strcpy(d->calibrationFile, m_calibEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_instrEdit->text().toAscii().data());  
}


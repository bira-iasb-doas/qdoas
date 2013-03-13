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
#include "CWSiteListCombo.h"
#include "CWorkSpace.h"
#include "CPreferences.h"

#include "constants.h"

#include "debugutil.h"

CWProjectTabInstrumental::CWProjectTabInstrumental(const mediate_project_instrumental_t *instr, QWidget *parent) :
  QFrame(parent)
{
  int index;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  mainLayout->addSpacing(25);

  QGridLayout *topLayout = new QGridLayout;

  m_formatStack = new QStackedWidget(this);
  // insert widgets into the stack, and store their index in the map - keyed by the instrument format

  // ascii
  m_asciiEdit = new CWInstrAsciiEdit(&(instr->ascii));
  index = m_formatStack->addWidget(m_asciiEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_ASCII, index));

  // logger
  m_loggerEdit = new CWInstrLoggerEdit(&(instr->logger));
  index = m_formatStack->addWidget(m_loggerEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_LOGGER, index));

  // acton
  m_actonEdit = new CWInstrActonEdit(&(instr->acton));
  index = m_formatStack->addWidget(m_actonEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_ACTON, index));

  // pdaegg
  m_pdaEggEdit = new CWInstrLoggerEdit(&(instr->pdaegg));
  index = m_formatStack->addWidget(m_pdaEggEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_PDAEGG, index));

  // pdaeggold
  m_pdaEggOldEdit = new CWInstrLoggerEdit(&(instr->pdaeggold));
  index = m_formatStack->addWidget(m_pdaEggOldEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_PDAEGG_OLD, index));

  // ccdohp96
  m_ccdOhp96Edit = new CWInstrCcdEdit(&(instr->ccdohp96));
  index = m_formatStack->addWidget(m_ccdOhp96Edit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_CCD_OHP_96, index));

  // ccdha94
  m_ccdHa94Edit = new CWInstrCcdEdit(&(instr->ccdha94));
  index = m_formatStack->addWidget(m_ccdHa94Edit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_CCD_HA_94, index));

  // saozvis
  m_saozVisEdit = new CWInstrSaozEdit(&(instr->saozvis));
  index = m_formatStack->addWidget(m_saozVisEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_SAOZ_VIS, index));

  // saozefm
  m_saozEfmEdit = new CWInstrMinimumEdit(&(instr->saozefm));
  index = m_formatStack->addWidget(m_saozEfmEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_SAOZ_EFM, index));

  // mfc
  m_mfcEdit = new CWInstrMfcEdit(&(instr->mfc));
  index = m_formatStack->addWidget(m_mfcEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_MFC, index));

  // mfcstd
  m_mfcStdEdit = new CWInstrMfcStdEdit(&(instr->mfcstd));
  index = m_formatStack->addWidget(m_mfcStdEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_MFC_STD, index));

  // mfcbira
  m_mfcbiraEdit = new CWInstrMfcbiraEdit(&(instr->mfcbira));
  index = m_formatStack->addWidget(m_mfcbiraEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_MFC_BIRA, index));

  // rasas
  m_rasasEdit = new CWInstrMinimumEdit(&(instr->rasas));
  index = m_formatStack->addWidget(m_rasasEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_RASAS, index));

   // pdasieasoe
  m_pdasiEasoeEdit = new CWInstrMinimumEdit(&(instr->pdasieasoe));
  index = m_formatStack->addWidget(m_pdasiEasoeEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_PDASI_EASOE, index));

  // ccdeev
  m_ccdEevEdit = new CWInstrCcdEevEdit(&(instr->ccdeev));
  index = m_formatStack->addWidget(m_ccdEevEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_CCD_EEV, index));

  // gdpascii
  m_gdpAsciiEdit = new CWInstrGdpEdit(&(instr->gdpascii));
  index = m_formatStack->addWidget(m_gdpAsciiEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_GDP_ASCII, index));

  // gdpbin
  m_gdpBinEdit = new CWInstrGdpEdit(&(instr->gdpbin));
  index = m_formatStack->addWidget(m_gdpBinEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_GDP_BIN, index));

  // sciapds
  m_sciaPdsEdit = new CWInstrSciaEdit(&(instr->sciapds));
  index = m_formatStack->addWidget(m_sciaPdsEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_SCIA_PDS, index));

  // uoft
  m_uoftEdit = new CWInstrMinimumEdit(&(instr->uoft));
  index = m_formatStack->addWidget(m_uoftEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_UOFT, index));

  // noaa
  m_noaaEdit = new CWInstrMinimumEdit(&(instr->noaa));
  index = m_formatStack->addWidget(m_noaaEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_NOAA, index));

  // omi
  m_omiEdit = new CWInstrOmiEdit(&(instr->omi));
  index = m_formatStack->addWidget(m_omiEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_OMI, index));

  // gome2
  m_gome2Edit = new CWInstrGome2Edit(&(instr->gome2));
  index = m_formatStack->addWidget(m_gome2Edit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_GOME2, index));

  // mkzy pack
  m_mkzyEdit = new CWInstrMinimumEdit(&(instr->mkzy));
  index = m_formatStack->addWidget(m_mkzyEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_MKZY, index));

  // bira airborne
  m_biraairborneEdit = new CWInstrMinimumEdit(&(instr->biraairborne));
  index = m_formatStack->addWidget(m_biraairborneEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_BIRA_AIRBORNE, index));

  // ocean optics
  m_oceanOpticsEdit = new CWInstrOceanOpticsEdit(&(instr->oceanoptics));
  index = m_formatStack->addWidget(m_oceanOpticsEdit);
  m_instrumentToStackIndexMap.insert(std::map<int,int>::value_type(PRJCT_INSTR_FORMAT_OCEAN_OPTICS	, index));

  // Site
  m_siteCombo = new CWSiteListCombo(this); // automatically populated

  topLayout->addWidget(new QLabel("Site", this), 1, 0);
  topLayout->addWidget(m_siteCombo, 1, 1);

  topLayout->setColumnMinimumWidth(0, 90);
  topLayout->setColumnStretch(1, 1);

  mainLayout->addLayout(topLayout);
  mainLayout->addWidget(m_formatStack);
  mainLayout->addStretch(1);

  // set the current format - stack will follow
  slotInstrumentChanged(instr->format);

  index = m_siteCombo->findText(QString(instr->siteName));
  if (index != -1)
    m_siteCombo->setCurrentIndex(index);
 }

CWProjectTabInstrumental::~CWProjectTabInstrumental()
{
}

void CWProjectTabInstrumental::apply(mediate_project_instrumental_t *instr) const
{
  // set values for ALL instruments ... the selected mode is handled elsewhere (by the parent).

  QString siteName = m_siteCombo->currentText();
//  if (siteName != "No Site Specified" && siteName.length() < (int)sizeof(instr->siteName))
    strcpy(instr->siteName, siteName.toAscii().data());

  m_asciiEdit->apply(&(instr->ascii));
  m_loggerEdit->apply(&(instr->logger));
  m_actonEdit->apply(&(instr->acton));
  m_pdaEggEdit->apply(&(instr->pdaegg));
  m_pdaEggOldEdit->apply(&(instr->pdaeggold));
  m_ccdOhp96Edit->apply(&(instr->ccdohp96));
  m_ccdHa94Edit->apply(&(instr->ccdha94));
  m_saozVisEdit->apply(&(instr->saozvis));
  m_saozEfmEdit->apply(&(instr->saozefm));
  m_mfcEdit->apply(&(instr->mfc));
  m_mfcStdEdit->apply(&(instr->mfcstd));
  m_mfcbiraEdit->apply(&(instr->mfcbira));
  m_rasasEdit->apply(&(instr->rasas));
  m_pdasiEasoeEdit->apply(&(instr->pdasieasoe));
  m_ccdEevEdit->apply(&(instr->ccdeev));
  m_gdpAsciiEdit->apply(&(instr->gdpascii));
  m_gdpBinEdit->apply(&(instr->gdpbin));
  m_sciaPdsEdit->apply(&(instr->sciapds));
  m_uoftEdit->apply(&(instr->uoft));
  m_noaaEdit->apply(&(instr->noaa));
  m_omiEdit->apply(&(instr->omi));
  m_gome2Edit->apply(&(instr->gome2));
  m_mkzyEdit->apply(&(instr->mkzy));
  m_biraairborneEdit->apply(&(instr->biraairborne));
  m_oceanOpticsEdit->apply(&(instr->oceanoptics));
}

void CWProjectTabInstrumental::slotInstrumentChanged(int instrument)
{
  std::map<int,int>::const_iterator it = m_instrumentToStackIndexMap.find(instrument);
  if (it != m_instrumentToStackIndexMap.end()) {

    m_formatStack->setCurrentIndex(it->second);
  }
}

void CWProjectTabInstrumental::slotInstrumentTypeChanged(int instrumentType)
{
 m_siteCombo->setEnabled((instrumentType==PRJCT_INSTR_TYPE_SATELLITE)?0:1);
}

//--------------------------------------------------------
// Specific Instrument Editors...

static const int cSuggestedColumnZeroWidth = 120; // try and keep editor layout
static const int cSuggestedColumnTwoWidth  = 100; // consistent
static const int cStandardEditWidth         = 70;

//--------------------------------------------------------

CWCalibInstrEdit::CWCalibInstrEdit(QWidget *parent) :
  QFrame(parent)
{
}

CWCalibInstrEdit::~CWCalibInstrEdit()
{
}

void CWCalibInstrEdit::slotCalibOneBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Calibration File",
						  pref->directoryName("Calib"),
						  "Calibration File (*.clb);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Calib", filename);

    m_fileOneEdit->setText(filename);
  }
}

void CWCalibInstrEdit::slotInstrTwoBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Instrument Function File",
						  pref->directoryName("Instr"),
						  "Instrument Function File (*.ins);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Instr", filename);

    m_fileTwoEdit->setText(filename);
  }
}

void CWCalibInstrEdit::slotOffsetTwoBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Offset File",
						  pref->directoryName("Offset"),
						  "Offset File (*.txt);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Offset", filename);

    m_fileTwoEdit->setText(filename);
  }
}

void CWCalibInstrEdit::helperConstructFileWidget(QLineEdit **fileEdit, QGridLayout *gridLayout, int &row,
						    const char *str, int len,
						    const char *label, const char *slot)
{
  // Helper that constructs the fileOneEdit editing widget and place it in a grid layout.
  // row is the first 'free row' in the grid on entry and is updated to be the next
  // 'free row' on exit.

  gridLayout->addWidget(new QLabel(label, this), row, 0);
  QLineEdit *edit = new QLineEdit(this);
  edit->setMaxLength(len-1);
  gridLayout->addWidget(edit, row, 1);
  QPushButton *browseBtn = new QPushButton("Browse", this);
  gridLayout->addWidget(browseBtn, row, 2);
  ++row;

  // initialise the value
  edit->setText(QString(str));

  // connections
  connect(browseBtn, SIGNAL(clicked()), this, slot);

  *fileEdit = edit;
}

void CWCalibInstrEdit::helperConstructCalInsFileWidgets(QGridLayout *gridLayout, int &row,
							const char *calib, int lenCalib,
							const char *instr, int lenInstr)
{
  // Helper that constructs the file editing widgets and places them in a grid layout.
  // row is the first 'free row' in the grid on entry and is updated to be the next
  // 'free row' on exit.

  helperConstructFileWidget(&m_fileOneEdit, gridLayout, row, calib, lenCalib,
			    "Calibration File", SLOT(slotCalibOneBrowse()));

  helperConstructFileWidget(&m_fileTwoEdit, gridLayout, row, instr, lenInstr,
			    "Transmission file", SLOT(slotInstrTwoBrowse()));
}

//--------------------------------------------------------

CWAllFilesEdit::CWAllFilesEdit(QWidget *parent) :
  CWCalibInstrEdit(parent)
{
}

CWAllFilesEdit::~CWAllFilesEdit()
{
}

void CWAllFilesEdit::slotInterPixelVariabilityThreeBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Inter-Pixel Variability File",
						  pref->directoryName("IntPixVar"),
						  "Inter-Pixel Variability File (*.ipv);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("IntPixVar", filename);

    m_fileThreeEdit->setText(filename);
  }
}

void CWAllFilesEdit::slotDarkCurrentThreeBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Dark Current File",
						  pref->directoryName("Dark"),
						  "Dark Current File (*.drk);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Dark", filename);

    m_fileThreeEdit->setText(filename);
  }
}

void CWAllFilesEdit::slotStraylightCorrectionThreeBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Stray-Light File",
						  pref->directoryName("Stray"),
						  "Stray-Light File (*.str);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Stray", filename);

    m_fileThreeEdit->setText(filename);
  }
}

void CWAllFilesEdit::slotDetectorNonLinearityFourBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Detector Non-Linearity File",
						  pref->directoryName("DetNonLin"),
						  "Detector Non-Linearity File (*.dnl);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("DetNonLin", filename);

    m_fileFourEdit->setText(filename);
  }
}

void CWAllFilesEdit::slotOffsetFourBrowse()
{
  CPreferences *pref = CPreferences::instance();

  QString filename = QFileDialog::getOpenFileName(this, "Select Offset File",
						  pref->directoryName("Offset"),
						  "Offset File (*.txt);;All Files (*)");

  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("Offset", filename);

    m_fileFourEdit->setText(filename);
  }
}

void CWAllFilesEdit::slotImagePathFiveBrowse()                                  // !!!!!
{
 QString dir = CPreferences::instance()->directoryName("ImageDir", ".");

 // modal dialog
 dir = QFileDialog::getExistingDirectory(0, "Select the root path for camera pictures if any", dir);

 if (!dir.isEmpty()) {
   CPreferences::instance()->setDirectoryName("ImageDir", dir);
   m_fileFiveEdit->setText(dir);
 }
}

void CWAllFilesEdit::helperConstructIpvDnlFileWidgets(QGridLayout *gridLayout, int &row,
						      const char *ipv, int lenIpv,
						      const char *dnl, int lenDnl)
{
  // Helper that constructs the file editing widgets and places them in a grid layout.
  // row is the first 'free row' in the grid on entry and is updated to be the next
  // 'free row' on exit.

  helperConstructFileWidget(&m_fileThreeEdit, gridLayout, row, ipv, lenIpv,
			    "Interpixel Variability", SLOT(slotInterPixelVariabilityThreeBrowse()));

  helperConstructFileWidget(&m_fileFourEdit, gridLayout, row, dnl, lenDnl,
			    "Det. Non-Linearity", SLOT(slotDetectorNonLinearityFourBrowse()));
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
	 int row;
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

  QGroupBox *straylightGroup = new QGroupBox("Straylight bias", this);
  QGridLayout *straylightLayout = new QGridLayout(straylightGroup);

  row=0;

  m_strayLightCheck = new QCheckBox("Correct straylight bias", straylightGroup);
  straylightLayout->addWidget(m_strayLightCheck, row, 0);

  ++row;

  straylightLayout->addWidget(new QLabel("Wavelength min", straylightGroup), row, 0);             // detector size label
  m_lambdaMinEdit = new QLineEdit(straylightGroup);
  m_lambdaMinEdit->setFixedWidth(50);
  m_lambdaMinEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 900.0, 2, m_lambdaMinEdit));
  straylightLayout->addWidget(m_lambdaMinEdit, row, 1);

  ++row;

  straylightLayout->addWidget(new QLabel("Wavelength max", straylightGroup), row, 0);             // detector size label
  m_lambdaMaxEdit = new QLineEdit(straylightGroup);
  m_lambdaMaxEdit->setFixedWidth(50);
  m_lambdaMaxEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 900.0, 2, m_lambdaMaxEdit));
  straylightLayout->addWidget(m_lambdaMaxEdit, row, 1);

  groupLayout->addWidget(straylightGroup);

  mainLayout->addLayout(groupLayout);

  // bottom layout - det. size and files
  row = 0;
  QGridLayout *bottomLayout = new QGridLayout;

  bottomLayout->addWidget(new QLabel("Detector Size", this), row, 0);
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
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

  m_strayLightCheck->setCheckState(d->straylight ? Qt::Checked : Qt::Unchecked);

  // straylight bias
  tmpStr.setNum(d->lambdaMin);
  m_lambdaMinEdit->validator()->fixup(tmpStr);
  m_lambdaMinEdit->setText(tmpStr);
  tmpStr.setNum(d->lambdaMax);
  m_lambdaMaxEdit->validator()->fixup(tmpStr);
  m_lambdaMaxEdit->setText(tmpStr);
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

  // straylight

  d->straylight = (m_strayLightCheck->checkState() == Qt::Checked) ? 1 : 0;
  d->lambdaMin = m_lambdaMinEdit->text().toDouble();
  d->lambdaMax = m_lambdaMaxEdit->text().toDouble();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());

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
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());
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
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());
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
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());
}

//--------------------------------------------------------------------------

CWInstrMfcEdit::CWInstrMfcEdit(const struct instrumental_mfc *d, QWidget *parent) :
  CWAllFilesEdit(parent)
{
  QString tmpStr;
  int row = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // detector size and first wavelength
  QGridLayout *topLayout = new QGridLayout;
  topLayout->addWidget(new QLabel("Detector Size", this), 0, 0);
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  topLayout->addWidget(m_detSizeEdit, 0, 1, Qt::AlignLeft);
  topLayout->addWidget(new QLabel("1st Wavelength (nm)", this), 0, 2);
  m_firstLambdaEdit = new QLineEdit(this);
  m_firstLambdaEdit->setFixedWidth(cStandardEditWidth);
  m_firstLambdaEdit->setValidator(new QIntValidator(100, 999, m_firstLambdaEdit));
  topLayout->addWidget(m_firstLambdaEdit, 0, 3, Qt::AlignLeft);

  mainLayout->addLayout(topLayout);

  // masks and check boxes
  QHBoxLayout *middleLayout = new QHBoxLayout;
  // check boxes
  QVBoxLayout *checkLayout = new QVBoxLayout;
  m_revertCheck = new QCheckBox("Revert", this);
  checkLayout->addWidget(m_revertCheck);
  m_autoCheck = new QCheckBox("Auto. File Selection", this);
  checkLayout->addWidget(m_autoCheck);
  middleLayout->addLayout(checkLayout);
  // mask group
  QGroupBox *maskGroup = new QGroupBox("Masks", this);
  QGridLayout *maskLayout = new QGridLayout(maskGroup);
  // offset
  maskLayout->addWidget(new QLabel("Offset", maskGroup), 0, 0);
  m_offsetMaskEdit = new QLineEdit(maskGroup);
  m_offsetMaskEdit->setFixedWidth(cStandardEditWidth);
  m_offsetMaskEdit->setValidator(new QIntValidator(0, 65535, m_offsetMaskEdit));
  maskLayout->addWidget(m_offsetMaskEdit, 0, 1);
  // instr fctn
  maskLayout->addWidget(new QLabel("Instr. ftcn", maskGroup), 0, 2);
  m_instrMaskEdit = new QLineEdit(maskGroup);
  m_instrMaskEdit->setFixedWidth(cStandardEditWidth);
  m_instrMaskEdit->setValidator(new QIntValidator(0, 65535, m_instrMaskEdit));
  maskLayout->addWidget(m_instrMaskEdit, 0, 3);
  // dark
  maskLayout->addWidget(new QLabel("Dark Current", maskGroup), 1, 0);
  m_darkMaskEdit = new QLineEdit(maskGroup);
  m_darkMaskEdit->setFixedWidth(cStandardEditWidth);
  m_darkMaskEdit->setValidator(new QIntValidator(0, 65535, m_darkMaskEdit));
  maskLayout->addWidget(m_darkMaskEdit, 1, 1);
  // spectra
  maskLayout->addWidget(new QLabel("Spectra", maskGroup), 1, 2);
  m_spectraMaskEdit = new QLineEdit(maskGroup);
  m_spectraMaskEdit->setFixedWidth(cStandardEditWidth);
  m_spectraMaskEdit->setValidator(new QIntValidator(0, 65535, m_spectraMaskEdit));
  maskLayout->addWidget(m_spectraMaskEdit, 1, 3);

  middleLayout->addWidget(maskGroup);
  mainLayout->addLayout(middleLayout);

  QGridLayout *gridLayout = new QGridLayout;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));

  // non-standard files...
  helperConstructFileWidget(&m_fileThreeEdit, gridLayout, row,
			    d->darkCurrentFile, sizeof(d->darkCurrentFile),
			    "Dark Current", SLOT(slotDarkCurrentThreeBrowse()));

  helperConstructFileWidget(&m_fileFourEdit, gridLayout, row,
			    d->offsetFile, sizeof(d->offsetFile),
			    "Offset", SLOT(slotOffsetFourBrowse()));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initialise the values

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  // first wavelength
  tmpStr.setNum(d->firstWavelength);
  m_firstLambdaEdit->validator()->fixup(tmpStr);
  m_firstLambdaEdit->setText(tmpStr);

  // check boxes
  m_revertCheck->setCheckState(d->revert ? Qt::Checked : Qt::Unchecked);
  m_autoCheck->setCheckState(d->autoFileSelect ? Qt::Checked : Qt::Unchecked);

  // masks
  tmpStr.setNum(d->offsetMask);
  m_offsetMaskEdit->validator()->fixup(tmpStr);
  m_offsetMaskEdit->setText(tmpStr);

  tmpStr.setNum(d->instrFctnMask);
  m_instrMaskEdit->validator()->fixup(tmpStr);
  m_instrMaskEdit->setText(tmpStr);

  tmpStr.setNum(d->darkCurrentMask);
  m_darkMaskEdit->validator()->fixup(tmpStr);
  m_darkMaskEdit->setText(tmpStr);

  tmpStr.setNum(d->spectraMask);
  m_spectraMaskEdit->validator()->fixup(tmpStr);
  m_spectraMaskEdit->setText(tmpStr);

}

CWInstrMfcEdit::~CWInstrMfcEdit()
{
}

void CWInstrMfcEdit::apply(struct instrumental_mfc *d) const
{
  // detector size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // first wavlength
  d->firstWavelength = m_firstLambdaEdit->text().toInt();

  // checkboxes
  d->revert = (m_revertCheck->checkState() == Qt::Checked) ? 1 : 0;
  d->autoFileSelect = (m_autoCheck->checkState() == Qt::Checked) ? 1 : 0;

  // masks
  d->offsetMask = m_offsetMaskEdit->text().toUInt();
  d->instrFctnMask = m_instrMaskEdit->text().toUInt();
  d->darkCurrentMask = m_darkMaskEdit->text().toUInt();
  d->spectraMask = m_spectraMaskEdit->text().toUInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());
  strcpy(d->darkCurrentFile, m_fileThreeEdit->text().toAscii().data());
  strcpy(d->offsetFile, m_fileFourEdit->text().toAscii().data());
}

//--------------------------------------------------------------------------

CWInstrMfcStdEdit::CWInstrMfcStdEdit(const struct instrumental_mfcstd *d, QWidget *parent) :
  CWAllFilesEdit(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QHBoxLayout *groupLayout = new QHBoxLayout;
  QString tmpStr;



  QGridLayout *gridLayout = new QGridLayout;

  QGroupBox *formatGroup = new QGroupBox("Format", this);
  QGridLayout *formatLayout = new QGridLayout(formatGroup);

  int row = 0;

  m_revertCheck = new QCheckBox("Revert spectra", formatGroup);                        // revert spectra check box
  formatLayout->addWidget(m_revertCheck, row, 0);

  ++row;

  formatLayout->addWidget(new QLabel("Detector Size", formatGroup), row, 0);             // detector size label
  m_detSizeEdit = new QLineEdit(formatGroup);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  formatLayout->addWidget(m_detSizeEdit, row, 1, Qt::AlignLeft);

  ++row;

  formatLayout->addWidget(new QLabel("Date Format", formatGroup), row, 0);               // date format label
  m_dateFormatEdit = new QLineEdit(formatGroup);
  m_dateFormatEdit->setMaxLength(sizeof(d->dateFormat)-1);
  formatLayout->addWidget(m_dateFormatEdit, row, 1, Qt::AlignLeft);

  groupLayout->addWidget(formatGroup);

  QGroupBox *straylightGroup = new QGroupBox("Straylight bias", this);
  QGridLayout *straylightLayout = new QGridLayout(straylightGroup);

  row=0;

  m_strayLightCheck = new QCheckBox("Correct straylight bias", straylightGroup);
  straylightLayout->addWidget(m_strayLightCheck, row, 0);

  ++row;

  straylightLayout->addWidget(new QLabel("Wavelength min", straylightGroup), row, 0);             // detector size label
  m_lambdaMinEdit = new QLineEdit(straylightGroup);
  m_lambdaMinEdit->setFixedWidth(50);
  m_lambdaMinEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 900.0, 2, m_lambdaMinEdit));
  straylightLayout->addWidget(m_lambdaMinEdit, row, 1);

  ++row;

  straylightLayout->addWidget(new QLabel("Wavelength max", straylightGroup), row, 0);             // detector size label
  m_lambdaMaxEdit = new QLineEdit(straylightGroup);
  m_lambdaMaxEdit->setFixedWidth(50);
  m_lambdaMaxEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 900.0, 2, m_lambdaMaxEdit));
  straylightLayout->addWidget(m_lambdaMaxEdit, row, 1);

  groupLayout->addWidget(straylightGroup);
  mainLayout->addLayout(groupLayout);

  row=0;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));

  // non-standard files...
  helperConstructFileWidget(&m_fileThreeEdit, gridLayout, row,
			    d->darkCurrentFile, sizeof(d->darkCurrentFile),
			    "Dark Current", SLOT(slotDarkCurrentThreeBrowse()));

  helperConstructFileWidget(&m_fileFourEdit, gridLayout, row,
			    d->offsetFile, sizeof(d->offsetFile),
			    "Offset", SLOT(slotOffsetFourBrowse()));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initialise the values

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  m_dateFormatEdit->setText(QString(d->dateFormat));

  // revert
  m_revertCheck->setCheckState(d->revert ? Qt::Checked : Qt::Unchecked);
  m_strayLightCheck->setCheckState(d->straylight ? Qt::Checked : Qt::Unchecked);

  // straylight bias
  tmpStr.setNum(d->lambdaMin);
  m_lambdaMinEdit->validator()->fixup(tmpStr);
  m_lambdaMinEdit->setText(tmpStr);
  tmpStr.setNum(d->lambdaMax);
  m_lambdaMaxEdit->validator()->fixup(tmpStr);
  m_lambdaMaxEdit->setText(tmpStr);
}

CWInstrMfcStdEdit::~CWInstrMfcStdEdit()
{
}

void CWInstrMfcStdEdit::apply(struct instrumental_mfcstd *d) const
{
  // detector size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // revert
  d->revert = (m_revertCheck->checkState() == Qt::Checked) ? 1 : 0;
  d->straylight = (m_strayLightCheck->checkState() == Qt::Checked) ? 1 : 0;
  d->lambdaMin = m_lambdaMinEdit->text().toDouble();
  d->lambdaMax = m_lambdaMaxEdit->text().toDouble();

  strcpy(d->dateFormat,m_dateFormatEdit->text().toAscii().data());

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());
  strcpy(d->darkCurrentFile, m_fileThreeEdit->text().toAscii().data());
  strcpy(d->offsetFile, m_fileFourEdit->text().toAscii().data());
}

//--------------------------------------------------------------------------

CWInstrMfcbiraEdit::CWInstrMfcbiraEdit(const struct instrumental_mfcbira *d, QWidget *parent) :
  CWAllFilesEdit(parent)
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QHBoxLayout *groupLayout = new QHBoxLayout;
  QGridLayout *gridLayout = new QGridLayout;

  QGroupBox *straylightGroup = new QGroupBox("Straylight bias", this);
  QGridLayout *straylightLayout = new QGridLayout(straylightGroup);

  m_strayLightCheck = new QCheckBox("Correct straylight bias", straylightGroup);
  straylightLayout->addWidget(m_strayLightCheck, row, 0);

  ++row;

  straylightLayout->addWidget(new QLabel("Wavelength min", straylightGroup), row, 0);             // detector size label
  m_lambdaMinEdit = new QLineEdit(straylightGroup);
  m_lambdaMinEdit->setFixedWidth(50);
  m_lambdaMinEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 900.0, 2, m_lambdaMinEdit));
  straylightLayout->addWidget(m_lambdaMinEdit, row, 1);

  ++row;

  straylightLayout->addWidget(new QLabel("Wavelength max", straylightGroup), row, 0);             // detector size label
  m_lambdaMaxEdit = new QLineEdit(straylightGroup);
  m_lambdaMaxEdit->setFixedWidth(50);
  m_lambdaMaxEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 900.0, 2, m_lambdaMaxEdit));
  straylightLayout->addWidget(m_lambdaMaxEdit, row, 1);

  groupLayout->addWidget(straylightGroup);
  groupLayout->addStretch(0);

  gridLayout->addWidget(new QLabel("Detector Size", this), row, 0);             // detector size label
  ++row;

  // Second line

  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  gridLayout->addWidget(m_detSizeEdit, row, 0, Qt::AlignLeft);

  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));

  mainLayout->addLayout(groupLayout,0);
  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initialise the values

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  m_strayLightCheck->setCheckState(d->straylight ? Qt::Checked : Qt::Unchecked);

  // straylight bias
  tmpStr.setNum(d->lambdaMin);
  m_lambdaMinEdit->validator()->fixup(tmpStr);
  m_lambdaMinEdit->setText(tmpStr);
  tmpStr.setNum(d->lambdaMax);
  m_lambdaMaxEdit->validator()->fixup(tmpStr);
  m_lambdaMaxEdit->setText(tmpStr);
}

CWInstrMfcbiraEdit::~CWInstrMfcbiraEdit()
{
}

void CWInstrMfcbiraEdit::apply(struct instrumental_mfcbira *d) const
{
  // detector size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());

  d->straylight = (m_strayLightCheck->checkState() == Qt::Checked) ? 1 : 0;
  d->lambdaMin = m_lambdaMinEdit->text().toDouble();
  d->lambdaMax = m_lambdaMaxEdit->text().toDouble();
}

//--------------------------------------------------------

CWInstrMinimumEdit::CWInstrMinimumEdit(const struct instrumental_minimum *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
	 QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  QHBoxLayout *groupLayout = new QHBoxLayout;

  QGroupBox *straylightGroup = new QGroupBox("Straylight bias", this);
  QGridLayout *straylightLayout = new QGridLayout(straylightGroup);

  m_strayLightCheck = new QCheckBox("Correct straylight bias", straylightGroup);
  straylightLayout->addWidget(m_strayLightCheck, row, 0);

  ++row;

  straylightLayout->addWidget(new QLabel("Wavelength min", straylightGroup), row, 0);             // detector size label
  m_lambdaMinEdit = new QLineEdit(straylightGroup);
  m_lambdaMinEdit->setFixedWidth(50);
  m_lambdaMinEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 900.0, 2, m_lambdaMinEdit));
  straylightLayout->addWidget(m_lambdaMinEdit, row, 1);

  ++row;

  straylightLayout->addWidget(new QLabel("Wavelength max", straylightGroup), row, 0);             // detector size label
  m_lambdaMaxEdit = new QLineEdit(straylightGroup);
  m_lambdaMaxEdit->setFixedWidth(50);
  m_lambdaMaxEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 900.0, 2, m_lambdaMaxEdit));
  straylightLayout->addWidget(m_lambdaMaxEdit, row, 1);

  groupLayout->addWidget(straylightGroup);
  groupLayout->addStretch(0);

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(groupLayout);
  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // straylight bias
  m_strayLightCheck->setCheckState(d->straylight ? Qt::Checked : Qt::Unchecked);
  tmpStr.setNum(d->lambdaMin);
  m_lambdaMinEdit->validator()->fixup(tmpStr);
  m_lambdaMinEdit->setText(tmpStr);
  tmpStr.setNum(d->lambdaMax);
  m_lambdaMaxEdit->validator()->fixup(tmpStr);
  m_lambdaMaxEdit->setText(tmpStr);
}

CWInstrMinimumEdit::~CWInstrMinimumEdit()
{
}

void CWInstrMinimumEdit::apply(struct instrumental_minimum *d) const
{
  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());

  // straylight bias

  d->straylight = (m_strayLightCheck->checkState() == Qt::Checked) ? 1 : 0;
  d->lambdaMin = m_lambdaMinEdit->text().toDouble();
  d->lambdaMax = m_lambdaMaxEdit->text().toDouble();
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
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());
  strcpy(d->interPixelVariabilityFile, m_fileThreeEdit->text().toAscii().data());
  strcpy(d->detectorNonLinearityFile, m_fileFourEdit->text().toAscii().data());
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
  m_gratingEdit->setFixedWidth(cStandardEditWidth);
  m_gratingEdit->setValidator(new QIntValidator(0, 99, m_gratingEdit));
  gridLayout->addWidget(m_gratingEdit, row, 1);
  ++row;

  // central wavelength
  gridLayout->addWidget(new QLabel("Central Wavelength (nm)"), row, 0);
  m_cenLambdaEdit = new QLineEdit(this);
  m_cenLambdaEdit->setFixedWidth(cStandardEditWidth);
  m_cenLambdaEdit->setValidator(new QIntValidator(0, 999, m_cenLambdaEdit));
  gridLayout->addWidget(m_cenLambdaEdit, row, 1);
  ++row;

  // files - non standard files ..
  helperConstructFileWidget(&m_fileOneEdit, gridLayout, row,
			    d->calibrationFile, sizeof(d->calibrationFile),
			    "Calibraton File", SLOT(slotCalibOneBrowse()));

  helperConstructFileWidget(&m_fileTwoEdit, gridLayout, row,
			    d->offsetFile, sizeof(d->offsetFile),
			    "Offset", SLOT(slotOffsetTwoBrowse()));

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
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->offsetFile, m_fileTwoEdit->text().toAscii().data());
  strcpy(d->interPixelVariabilityFile, m_fileThreeEdit->text().toAscii().data());
  strcpy(d->detectorNonLinearityFile, m_fileFourEdit->text().toAscii().data());
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
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());
  strcpy(d->interPixelVariabilityFile, m_fileThreeEdit->text().toAscii().data());
  strcpy(d->detectorNonLinearityFile, m_fileFourEdit->text().toAscii().data());
}

//--------------------------------------------------------

CWInstrCcdEevEdit::CWInstrCcdEevEdit(const struct instrumental_ccdeev *d, QWidget *parent) :
  CWAllFilesEdit(parent)
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  QHBoxLayout *groupLayout = new QHBoxLayout;

  QGroupBox *straylightGroup = new QGroupBox("Straylight bias", this);
  QGridLayout *straylightLayout = new QGridLayout(straylightGroup);

  m_strayLightCheck = new QCheckBox("Correct straylight bias", straylightGroup);
  straylightLayout->addWidget(m_strayLightCheck, row, 0);

  ++row;

  straylightLayout->addWidget(new QLabel("Wavelength min", straylightGroup), row, 0);             // detector size label
  m_lambdaMinEdit = new QLineEdit(straylightGroup);
  m_lambdaMinEdit->setFixedWidth(50);
  m_lambdaMinEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 900.0, 2, m_lambdaMinEdit));
  straylightLayout->addWidget(m_lambdaMinEdit, row, 1);

  ++row;

  straylightLayout->addWidget(new QLabel("Wavelength max", straylightGroup), row, 0);             // detector size label
  m_lambdaMaxEdit = new QLineEdit(straylightGroup);
  m_lambdaMaxEdit->setFixedWidth(50);
  m_lambdaMaxEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 900.0, 2, m_lambdaMaxEdit));
  straylightLayout->addWidget(m_lambdaMaxEdit, row, 1);

  groupLayout->addWidget(straylightGroup);
  groupLayout->addStretch(0);

  // detector size
  gridLayout->addWidget(new QLabel("Detector Size", this), row, 0);
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  gridLayout->addWidget(m_detSizeEdit, row, 1);
  ++row;

  // Spectral Type
  gridLayout->addWidget(new QLabel("Spectral Type", this), row, 0);
  m_spectralTypeCombo = new QComboBox(this);
  m_spectralTypeCombo->addItem("All", QVariant(PRJCT_INSTR_EEV_TYPE_NONE));
  m_spectralTypeCombo->addItem("Off-axis", QVariant(PRJCT_INSTR_EEV_TYPE_OFFAXIS));
  m_spectralTypeCombo->addItem("Direct sun", QVariant(PRJCT_INSTR_EEV_TYPE_DIRECTSUN));
  m_spectralTypeCombo->addItem("Almucantar", QVariant(PRJCT_INSTR_EEV_TYPE_ALMUCANTAR));
  gridLayout->addWidget(m_spectralTypeCombo, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(gridLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));

  // non-standard files...

  helperConstructFileWidget(&m_fileFiveEdit, gridLayout, row,                   // !!!!
			    d->imagePath, sizeof(d->imagePath),
			    "Camera pictures", SLOT(slotImagePathFiveBrowse()));

  helperConstructFileWidget(&m_fileThreeEdit, gridLayout, row,
			    d->straylightCorrectionFile, sizeof(d->straylightCorrectionFile),
			    "Stray-Light Correction", SLOT(slotStraylightCorrectionThreeBrowse()));

  helperConstructFileWidget(&m_fileFourEdit, gridLayout, row,
			    d->detectorNonLinearityFile, sizeof(d->detectorNonLinearityFile),
			    "Det. Non-Linearity", SLOT(slotDetectorNonLinearityFourBrowse()));

  gridLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  gridLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(groupLayout);
  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(1);

  // initialise the values

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  // spectral type

  int index = m_spectralTypeCombo->findData(QVariant(d->spectralType));
  if (index != -1)
    m_spectralTypeCombo->setCurrentIndex(index);

  // straylight bias
  m_strayLightCheck->setCheckState(d->straylight ? Qt::Checked : Qt::Unchecked);
  tmpStr.setNum(d->lambdaMin);
  m_lambdaMinEdit->validator()->fixup(tmpStr);
  m_lambdaMinEdit->setText(tmpStr);
  tmpStr.setNum(d->lambdaMax);
  m_lambdaMaxEdit->validator()->fixup(tmpStr);
  m_lambdaMaxEdit->setText(tmpStr);
}

CWInstrCcdEevEdit::~CWInstrCcdEevEdit()
{
}

void CWInstrCcdEevEdit::apply(struct instrumental_ccdeev *d) const
{
  // detector size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // spectral type
  d->spectralType = m_spectralTypeCombo->itemData(m_spectralTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());
  strcpy(d->straylightCorrectionFile, m_fileThreeEdit->text().toAscii().data());
  strcpy(d->detectorNonLinearityFile, m_fileFourEdit->text().toAscii().data());
  strcpy(d->imagePath, m_fileFiveEdit->text().toAscii().data());

  // straylight bias

  d->straylight = (m_strayLightCheck->checkState() == Qt::Checked) ? 1 : 0;
  d->lambdaMin = m_lambdaMinEdit->text().toDouble();
  d->lambdaMax = m_lambdaMaxEdit->text().toDouble();
}

//--------------------------------------------------------

CWInstrOpusEdit::CWInstrOpusEdit(const struct instrumental_opus *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  QString tmpStr;
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;

  // detector size
  gridLayout->addWidget(new QLabel("Detector Size", this), row, 0);
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  gridLayout->addWidget(m_detSizeEdit, row, 1);
  ++row;

  // time shift
  gridLayout->addWidget(new QLabel("Time Shift (hours)", this), row, 0);
  m_timeShiftEdit = new QLineEdit(this);
  m_timeShiftEdit->setFixedWidth(cStandardEditWidth);
  m_timeShiftEdit->setValidator(new CDoubleFixedFmtValidator(-24.0, 24.0, 2, m_timeShiftEdit));
  gridLayout->addWidget(m_timeShiftEdit, row, 1);
  ++row;

  //flag transmittance
  m_transmittanceCheck = new QCheckBox("Transmittance", this);
  gridLayout->addWidget(m_transmittanceCheck, row, 1);
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
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());
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

  // pixel type

  gridLayout->addWidget(new QLabel("Pixel Type", this), row, 0);
  m_pixelTypeCombo = new QComboBox(this);
  m_pixelTypeCombo->addItem("All", QVariant(PRJCT_INSTR_GDP_PIXEL_ALL));
  m_pixelTypeCombo->addItem("East", QVariant(PRJCT_INSTR_GDP_PIXEL_EAST));
  m_pixelTypeCombo->addItem("Center", QVariant(PRJCT_INSTR_GDP_PIXEL_CENTER));
  m_pixelTypeCombo->addItem("West", QVariant(PRJCT_INSTR_GDP_PIXEL_WEST));
  m_pixelTypeCombo->addItem("Backscan", QVariant(PRJCT_INSTR_GDP_PIXEL_BACKSCAN));
  gridLayout->addWidget(m_pixelTypeCombo, row, 1);
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

  // pixel type
  index = m_pixelTypeCombo->findData(QVariant(d->pixelType));
  if (index != -1)
    m_pixelTypeCombo->setCurrentIndex(index);
}

CWInstrGdpEdit::~CWInstrGdpEdit()
{
}

void CWInstrGdpEdit::apply(struct instrumental_gdp *d) const
{
  // band
  d->bandType = m_bandTypeCombo->itemData(m_bandTypeCombo->currentIndex()).toInt();
  // pixel type
  d->pixelType = m_pixelTypeCombo->itemData(m_pixelTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());
}

//--------------------------------------------------------------------------

CWInstrGome2Edit::CWInstrGome2Edit(const struct instrumental_gome2 *d, QWidget *parent) :
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

CWInstrGome2Edit::~CWInstrGome2Edit()
{
}

void CWInstrGome2Edit::apply(struct instrumental_gome2 *d) const
{
  // band
  d->bandType = m_bandTypeCombo->itemData(m_bandTypeCombo->currentIndex()).toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());
}

//--------------------------------------------------------------------------

CWInstrSciaEdit::CWInstrSciaEdit(const struct instrumental_scia *d, QWidget *parent) :
  CWAllFilesEdit(parent),
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

  helperConstructFileWidget(&m_fileFourEdit, gridLayout, row, d->detectorNonLinearityFile, sizeof(d->detectorNonLinearityFile),
			    "Det. Non-Linearity", SLOT(slotDetectorNonLinearityFourBrowse()));

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
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());
  strcpy(d->detectorNonLinearityFile, m_fileFourEdit->text().toAscii().data());
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
  char str[80];
  int row = 0;
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QGridLayout *gridLayout = new QGridLayout;
  // Format and Flags group
  QHBoxLayout *groupLayout = new QHBoxLayout;

  // spectral
  gridLayout->addWidget(new QLabel("Spectral Type", this), row, 0);
  m_spectralTypeCombo = new QComboBox(this);
  m_spectralTypeCombo->addItem("UV-1", QVariant(PRJCT_INSTR_OMI_TYPE_UV1));
  m_spectralTypeCombo->addItem("UV-2", QVariant(PRJCT_INSTR_OMI_TYPE_UV2));
  m_spectralTypeCombo->addItem("Visible", QVariant(PRJCT_INSTR_OMI_TYPE_VIS));
  gridLayout->addWidget(m_spectralTypeCombo, row, 1);
  ++row;

  // Track selection
  gridLayout->addWidget(new QLabel("Row selection (for example : 5-15,20-30)", this), row, 0);
  m_trackSelection = new QLineEdit(this);
  gridLayout->addWidget(m_trackSelection, row, 1);
  ++row;

  // Pixels quality flags

  QGroupBox *pixelQFGroup = new QGroupBox("Pixel Quality Flags", this);
  QGridLayout *pixelQFLayout = new QGridLayout(pixelQFGroup);

  m_pixelQFCheck = new QCheckBox("Pixel rejection based on quality flags", pixelQFGroup);
  pixelQFLayout->addWidget(m_pixelQFCheck, row, 0);

  ++row;

  pixelQFLayout->addWidget(new QLabel("Mask for pixel rejection", pixelQFGroup), row, 0);             // detector size label
  m_pixelQFMaskEdit = new QLineEdit(pixelQFGroup);
  m_pixelQFMaskEdit->setFixedWidth(50);
  pixelQFLayout->addWidget(m_pixelQFMaskEdit, row, 1);

  ++row;

  pixelQFLayout->addWidget(new QLabel("Maximum number of gaps", pixelQFGroup), row, 0);             // detector size label
  m_pixelQFMaxGapsEdit = new QLineEdit(pixelQFGroup);
  m_pixelQFMaxGapsEdit->setFixedWidth(50);
  m_pixelQFMaxGapsEdit->setValidator(new QIntValidator(0, 20, m_pixelQFMaxGapsEdit));
  pixelQFLayout->addWidget(m_pixelQFMaxGapsEdit, row, 1);

  groupLayout->addWidget(pixelQFGroup);
  mainLayout->addLayout(groupLayout);

  // XTrack Quality Flag handling:
  QGroupBox *xtrackQFBox = new QGroupBox("Cross-track Quality Flags", this);
  m_ignoreXTrackQF = new QRadioButton("Ignore");
  m_nonstrictXTrackQF = new QRadioButton("Exclude bad pixels");
  m_strictXTrackQF = new QRadioButton("Exclude all affected pixels");

  switch(d->xtrack_mode) {
  case XTRACKQF_IGNORE:
    m_ignoreXTrackQF->setChecked(true);
    break;
  case XTRACKQF_NONSTRICT:
    m_nonstrictXTrackQF->setChecked(true);
    break;
  case XTRACKQF_STRICT:
    m_strictXTrackQF->setChecked(true);
    break;
  }

  QVBoxLayout *xtrackQFBoxLayout = new QVBoxLayout;
  xtrackQFBoxLayout->setAlignment(Qt::AlignCenter);
  xtrackQFBoxLayout->addWidget(m_ignoreXTrackQF);
  xtrackQFBoxLayout->addWidget(m_nonstrictXTrackQF);
  xtrackQFBoxLayout->addWidget(m_strictXTrackQF);
  xtrackQFBoxLayout->addStretch(1);
  xtrackQFBox->setLayout(xtrackQFBoxLayout);
  
  mainLayout->addWidget(xtrackQFBox);

//  // spectral range
//  gridLayout->addWidget(new QLabel("Wavelength Range (nm)", this), row, 0);
//  QHBoxLayout *rangeLayout = new QHBoxLayout;
//  m_minLambdaEdit = new QLineEdit(this);
//  m_minLambdaEdit->setFixedWidth(cStandardEditWidth);
//  m_minLambdaEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 999.9, 1, m_minLambdaEdit));
//  rangeLayout->addWidget(m_minLambdaEdit);
//  m_maxLambdaEdit = new QLineEdit(this);
//  m_maxLambdaEdit->setFixedWidth(cStandardEditWidth);
//  m_maxLambdaEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 999.9, 1, m_maxLambdaEdit));
//  rangeLayout->addWidget(m_maxLambdaEdit);
//  rangeLayout->addStretch(1);
//  gridLayout->addLayout(rangeLayout, row, 1, 1, 2, Qt::AlignLeft);
//  ++row;
//
//  // average
//  m_averageCheck = new QCheckBox("Average spectra in tracks", this);
//  gridLayout->addWidget(m_averageCheck, row, 1, 1, 2, Qt::AlignLeft);
//  ++row;



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

  m_trackSelection->setText(QString(d->trackSelection));

  // pixel quality flag

  m_pixelQFCheck->setCheckState(d->pixelQFRejectionFlag ? Qt::Checked : Qt::Unchecked);

  tmpStr.setNum(d->pixelQFMaxGaps);
  m_pixelQFMaxGapsEdit->validator()->fixup(tmpStr);
  m_pixelQFMaxGapsEdit->setText(tmpStr);

  sprintf(str,"%02X",d->pixelQFMask);
  m_pixelQFMaskEdit->setText(QString(str));

  // wavelength range
//  tmpStr.setNum(d->minimumWavelength);
//  m_minLambdaEdit->validator()->fixup(tmpStr);
//  m_minLambdaEdit->setText(tmpStr);
//  tmpStr.setNum(d->maximumWavelength);
//  m_maxLambdaEdit->validator()->fixup(tmpStr);
//  m_maxLambdaEdit->setText(tmpStr);
//
//  // average
//  m_averageCheck->setCheckState(d->flagAverage ? Qt::Checked : Qt::Unchecked);

}

CWInstrOmiEdit::~CWInstrOmiEdit()
{
}

void CWInstrOmiEdit::apply(struct instrumental_omi *d) const
{
  // spectral
  d->spectralType = m_spectralTypeCombo->itemData(m_spectralTypeCombo->currentIndex()).toInt();

  // wavelength range
//  d->minimumWavelength = m_minLambdaEdit->text().toDouble();
//  d->maximumWavelength = m_maxLambdaEdit->text().toDouble();
//
//  d->flagAverage = (m_averageCheck->checkState() == Qt::Checked) ? 1 : 0;

   // track selection

  d->pixelQFRejectionFlag=(m_pixelQFCheck->checkState() == Qt::Checked) ? 1 : 0;
  d->pixelQFMaxGaps=m_pixelQFMaxGapsEdit->text().toInt();

  sscanf(m_pixelQFMaskEdit->text().toAscii().data(),"%02X",&d->pixelQFMask);

  strcpy(d->trackSelection, m_trackSelection->text().toAscii().data());

  // XTrack Quality Flags:
  if (m_strictXTrackQF->isChecked() )
    d->xtrack_mode = XTRACKQF_STRICT;
  else if (m_nonstrictXTrackQF->isChecked() )
    d->xtrack_mode = XTRACKQF_NONSTRICT;
  else
    d->xtrack_mode = XTRACKQF_IGNORE;

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());
}

//--------------------------------------------------------

CWInstrOceanOpticsEdit::CWInstrOceanOpticsEdit(const struct instrumental_oceanoptics *d, QWidget *parent) :
  CWCalibInstrEdit(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  int row = 0;
  QGridLayout *bottomLayout = new QGridLayout;

  QHBoxLayout *groupLayout = new QHBoxLayout;

  QGroupBox *straylightGroup = new QGroupBox("Straylight bias", this);
  QGridLayout *straylightLayout = new QGridLayout(straylightGroup);

  m_strayLightCheck = new QCheckBox("Correct straylight bias", straylightGroup);
  straylightLayout->addWidget(m_strayLightCheck, row, 0);

  ++row;

  straylightLayout->addWidget(new QLabel("Wavelength min", straylightGroup), row, 0);             // detector size label
  m_lambdaMinEdit = new QLineEdit(straylightGroup);
  m_lambdaMinEdit->setFixedWidth(50);
  m_lambdaMinEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 900.0, 2, m_lambdaMinEdit));
  straylightLayout->addWidget(m_lambdaMinEdit, row, 1);

  ++row;

  straylightLayout->addWidget(new QLabel("Wavelength max", straylightGroup), row, 0);             // detector size label
  m_lambdaMaxEdit = new QLineEdit(straylightGroup);
  m_lambdaMaxEdit->setFixedWidth(50);
  m_lambdaMaxEdit->setValidator(new CDoubleFixedFmtValidator(0.0, 900.0, 2, m_lambdaMaxEdit));
  straylightLayout->addWidget(m_lambdaMaxEdit, row, 1);

  groupLayout->addWidget(straylightGroup);
  groupLayout->addStretch(0);

  bottomLayout->addWidget(new QLabel("Detector Size", this), row, 0);
  m_detSizeEdit = new QLineEdit(this);
  m_detSizeEdit->setFixedWidth(cStandardEditWidth);
  m_detSizeEdit->setValidator(new QIntValidator(0, 8192, m_detSizeEdit));
  bottomLayout->addWidget(m_detSizeEdit, row, 1);
  ++row;

  // files
  helperConstructCalInsFileWidgets(bottomLayout, row,
				   d->calibrationFile, sizeof(d->calibrationFile),
				   d->instrFunctionFile, sizeof(d->instrFunctionFile));

  bottomLayout->setColumnMinimumWidth(0, cSuggestedColumnZeroWidth);
  bottomLayout->setColumnMinimumWidth(2, cSuggestedColumnTwoWidth);

  mainLayout->addLayout(groupLayout);
  mainLayout->addLayout(bottomLayout);
  mainLayout->addStretch(1);

  // initial values
  QString tmpStr;

  // detector size
  tmpStr.setNum(d->detectorSize);
  m_detSizeEdit->validator()->fixup(tmpStr);
  m_detSizeEdit->setText(tmpStr);

  // straylight bias
  m_strayLightCheck->setCheckState(d->straylight ? Qt::Checked : Qt::Unchecked);
  tmpStr.setNum(d->lambdaMin);
  m_lambdaMinEdit->validator()->fixup(tmpStr);
  m_lambdaMinEdit->setText(tmpStr);
  tmpStr.setNum(d->lambdaMax);
  m_lambdaMaxEdit->validator()->fixup(tmpStr);
  m_lambdaMaxEdit->setText(tmpStr);
}

CWInstrOceanOpticsEdit::~CWInstrOceanOpticsEdit()
{
}

void CWInstrOceanOpticsEdit::apply(struct instrumental_oceanoptics *d) const
{
  // straylight bias

  d->straylight = (m_strayLightCheck->checkState() == Qt::Checked) ? 1 : 0;
  d->lambdaMin = m_lambdaMinEdit->text().toDouble();
  d->lambdaMax = m_lambdaMaxEdit->text().toDouble();

  // detected size
  d->detectorSize = m_detSizeEdit->text().toInt();

  // files
  strcpy(d->calibrationFile, m_fileOneEdit->text().toAscii().data());
  strcpy(d->instrFunctionFile, m_fileTwoEdit->text().toAscii().data());

}

//--------------------------------------------------------

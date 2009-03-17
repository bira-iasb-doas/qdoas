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


#ifndef _CWPROJECTTABINSTRUMENTAL_H_GUARD
#define _CWPROJECTTABINSTRUMENTAL_H_GUARD

#include <map>

#include <QFrame>
#include <QComboBox>
#include <QStackedWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QGridLayout>

#include "mediate_project.h"

class CWInstrAsciiEdit;
class CWInstrLoggerEdit;
class CWInstrActonEdit;
class CWInstrSaozEdit;
class CWInstrMinimumEdit;
class CWInstrCcdEdit;
class CWInstrCcdUlbEdit;
class CWInstrPdaEggUlbEdit;
class CWInstrCcdEevEdit;
class CWInstrOpusEdit;
class CWInstrGdpEdit;
class CWInstrSciaEdit;
class CWInstrOmiEdit;
class CWInstrMfcEdit;
class CWInstrMfcStdEdit;

//--------------------------------------------------------------------------
class CWProjectTabInstrumental : public QFrame
{
Q_OBJECT
 public:
  CWProjectTabInstrumental(const mediate_project_instrumental_t *instr, QWidget *parent = 0);
  virtual ~CWProjectTabInstrumental();

  void apply(mediate_project_instrumental_t *instr) const;

 public slots:
  void slotInstrumentChanged(int instrument);
  void slotInstrumentTypeChanged(int instrumentType);

 private:
  QComboBox *m_siteCombo;
  QStackedWidget *m_formatStack;
  // widgets for the configuration of each instrument file format
  CWInstrAsciiEdit *m_asciiEdit;
  CWInstrLoggerEdit *m_loggerEdit;
  CWInstrActonEdit *m_actonEdit;
  CWInstrLoggerEdit *m_pdaEggEdit;
  CWInstrLoggerEdit *m_pdaEggOldEdit;
  CWInstrPdaEggUlbEdit *m_pdaEggUlbEdit;
  CWInstrCcdEdit *m_ccdOhp96Edit;
  CWInstrCcdEdit *m_ccdHa94Edit;
  CWInstrCcdUlbEdit *m_ccdUlbEdit;
  CWInstrSaozEdit *m_saozVisEdit;
  CWInstrSaozEdit *m_saozUvEdit;
  CWInstrMfcEdit *m_mfcEdit;
  CWInstrMfcStdEdit *m_mfcStdEdit;
  CWInstrMinimumEdit *m_saozEfmEdit;
  CWInstrMinimumEdit *m_rasasEdit;
  CWInstrMinimumEdit *m_pdasiEasoeEdit;
  CWInstrLoggerEdit *m_pdasiOsmaEdit;
  CWInstrCcdEevEdit *m_ccdEevEdit;
  CWInstrOpusEdit *m_opusEdit;
  CWInstrGdpEdit *m_gdpAsciiEdit;
  CWInstrGdpEdit *m_gdpBinEdit;
  CWInstrSciaEdit *m_sciaHdfEdit;
  CWInstrSciaEdit *m_sciaPdsEdit;
  CWInstrMinimumEdit *m_uoftEdit;
  CWInstrMinimumEdit *m_noaaEdit;
  CWInstrOmiEdit *m_omiEdit;
  CWInstrGdpEdit *m_gome2Edit;
  CWInstrMinimumEdit *m_mkzyEdit;
  CWInstrMinimumEdit *m_biraairborneEdit;
  std::map<int,int> m_instrumentToStackIndexMap;
};

//--------------------------------------------------------------------------

// base class for editors with calibration and instrument files. Only provides
// the implementation for the browse slots. Instanciation of the protected
// member widgets is the responsibility of concrete derived classes.

class CWCalibInstrEdit : public QFrame
{
Q_OBJECT
 public:
  CWCalibInstrEdit(QWidget *parent = 0);
  virtual ~CWCalibInstrEdit();

 protected:
  void helperConstructFileWidget(QLineEdit **fileEdit, QGridLayout *gridLayout, int &row,
				 const char *str, int len,
				 const char *label, const char *slot);

  void helperConstructCalInsFileWidgets(QGridLayout *gridLayout, int &row,
					const char *calib, int lenCalib,
					const char *instr, int lenInstr);

 public slots:
  void slotCalibOneBrowse();
  void slotInstrTwoBrowse();
  void slotOffsetTwoBrowse();

 protected:
  QLineEdit *m_fileOneEdit, *m_fileTwoEdit;
};

//--------------------------------------------------------------------------

// base class for editors with all instrument files. Only provides
// the implementation for the browse slots.Instanciation of the protected
// member widgets is the responsibility of concrete derived classes.
// A helper function can be used for the construction of these widgets.

class CWAllFilesEdit : public CWCalibInstrEdit
{
Q_OBJECT
 public:
  CWAllFilesEdit(QWidget *parent = 0);
  virtual ~CWAllFilesEdit();

 protected:
  void helperConstructIpvDnlFileWidgets(QGridLayout *gridLayout, int &row,
					const char *ipv, int lenIpv,
					const char *dnl, int lenDnl);

  void helperConstructFileWidgets(QGridLayout *gridLayout, int &row,
				  const char *calib, int lenCalib,
				  const char *instr, int lenInstr,
				  const char *ipv, int lenIpv,
				  const char *dnl, int lenDnl);

 public slots:
  void slotInterPixelVariabilityThreeBrowse();
  void slotDarkCurrentThreeBrowse();
  void slotStraylightCorrectionThreeBrowse();
  void slotDetectorNonLinearityFourBrowse();
  void slotOffsetFourBrowse();

 protected:
  QLineEdit *m_fileThreeEdit, *m_fileFourEdit;
};

//--------------------------------------------------------------------------
class CWInstrAsciiEdit : public CWCalibInstrEdit
{
 public:
  CWInstrAsciiEdit(const struct instrumental_ascii *d, QWidget *parent = 0);
  virtual ~CWInstrAsciiEdit();

  void apply(struct instrumental_ascii *d) const;

 private:
   QLineEdit *m_detSizeEdit;
   QRadioButton *m_lineRadioButton, *m_columnRadioButton;
   QCheckBox *m_zenCheck, *m_aziCheck, *m_eleCheck, *m_dateCheck, *m_timeCheck, *m_lambdaCheck;
};

//--------------------------------------------------------------------------

class CWInstrLoggerEdit : public CWCalibInstrEdit
{
 public:
  CWInstrLoggerEdit(const struct instrumental_logger *d, QWidget *parent = 0);
  virtual ~CWInstrLoggerEdit();

  void apply(struct instrumental_logger *d) const;

 private:
  QComboBox *m_spectralTypeCombo;
  QCheckBox *m_aziCheck;
};

//--------------------------------------------------------------------------

class CWInstrActonEdit : public CWCalibInstrEdit
{
 public:
  CWInstrActonEdit(const struct instrumental_acton *d, QWidget *parent = 0);
  virtual ~CWInstrActonEdit();

  void apply(struct instrumental_acton *d) const;

 private:
  QComboBox *m_niluTypeCombo;
};

//--------------------------------------------------------------------------

class CWInstrSaozEdit : public CWCalibInstrEdit
{
 public:
  CWInstrSaozEdit(const struct instrumental_saoz *d, QWidget *parent = 0);
  virtual ~CWInstrSaozEdit();

  void apply(struct instrumental_saoz *d) const;

 private:
  QComboBox *m_spectralTypeCombo;
};

//--------------------------------------------------------------------------

class CWInstrMfcEdit : public CWAllFilesEdit
{
 public:
  CWInstrMfcEdit(const struct instrumental_mfc *d, QWidget *parent = 0);
  virtual ~CWInstrMfcEdit();

  void apply(struct instrumental_mfc *d) const;

 private:
  QLineEdit *m_detSizeEdit, *m_firstLambdaEdit;
  QLineEdit *m_offsetMaskEdit, *m_instrMaskEdit, *m_darkMaskEdit, *m_spectraMaskEdit;
  QCheckBox *m_revertCheck, *m_autoCheck;
};

//--------------------------------------------------------------------------

class CWInstrMfcStdEdit : public CWAllFilesEdit
{
 public:
  CWInstrMfcStdEdit(const struct instrumental_mfcstd *d, QWidget *parent = 0);
  virtual ~CWInstrMfcStdEdit();

  void apply(struct instrumental_mfcstd *d) const;

 private:
  QLineEdit *m_detSizeEdit;
  QLineEdit *m_dateFormatEdit;
  QCheckBox *m_revertCheck;
  QCheckBox *m_strayLightCheck;
};

//--------------------------------------------------------------------------

class CWInstrMinimumEdit : public CWCalibInstrEdit
{
 public:
  CWInstrMinimumEdit(const struct instrumental_minimum *d, QWidget *parent = 0);
  virtual ~CWInstrMinimumEdit();

  void apply(struct instrumental_minimum *d) const;
};

//--------------------------------------------------------------------------

class CWInstrCcdEdit : public CWAllFilesEdit
{
 public:
  CWInstrCcdEdit(const struct instrumental_ccd *d, QWidget *parent = 0);
  virtual ~CWInstrCcdEdit();

  void apply(struct instrumental_ccd *d) const;
};

//--------------------------------------------------------------------------

class CWInstrCcdUlbEdit : public CWAllFilesEdit
{
 public:
  CWInstrCcdUlbEdit(const struct instrumental_ccdulb *d, QWidget *parent = 0);
  virtual ~CWInstrCcdUlbEdit();

  void apply(struct instrumental_ccdulb *d) const;

 private:
  QLineEdit *m_gratingEdit, *m_cenLambdaEdit;
};

//--------------------------------------------------------------------------

class CWInstrPdaEggUlbEdit : public CWAllFilesEdit
{
 public:
  CWInstrPdaEggUlbEdit(const struct instrumental_pdaeggulb *d, QWidget *parent = 0);
  virtual ~CWInstrPdaEggUlbEdit();

  void apply(struct instrumental_pdaeggulb *d) const;

 private:
  QComboBox *m_curveTypeCombo;
};

//--------------------------------------------------------------------------

class CWInstrCcdEevEdit : public CWAllFilesEdit
{
 public:
  CWInstrCcdEevEdit(const struct instrumental_ccdeev *d, QWidget *parent = 0);
  virtual ~CWInstrCcdEevEdit();

  void apply(struct instrumental_ccdeev *d) const;

 private:
  QLineEdit *m_detSizeEdit;
  QComboBox *m_spectralTypeCombo;
};

//--------------------------------------------------------------------------

class CWInstrOpusEdit : public CWCalibInstrEdit
{
 public:
  CWInstrOpusEdit(const struct instrumental_opus *d, QWidget *parent = 0);
  virtual ~CWInstrOpusEdit();

  void apply(struct instrumental_opus *d) const;

 private:
  QLineEdit *m_detSizeEdit, *m_timeShiftEdit;
  QCheckBox *m_transmittanceCheck;
};

//--------------------------------------------------------------------------

class CWInstrGdpEdit : public CWCalibInstrEdit
{
 public:
  CWInstrGdpEdit(const struct instrumental_gdp *d, QWidget *parent = 0);
  virtual ~CWInstrGdpEdit();

  void apply(struct instrumental_gdp *d) const;

 private:
  QComboBox *m_bandTypeCombo;
};

//--------------------------------------------------------------------------

class CWInstrSciaEdit : public CWCalibInstrEdit
{
Q_OBJECT
 public:
  CWInstrSciaEdit(const struct instrumental_scia *d, QWidget *parent = 0);
  virtual ~CWInstrSciaEdit();

  void apply(struct instrumental_scia *d) const;

 public slots:
  void slotChannelChanged(int index);
  void slotCluster0Changed(int state);
  void slotCluster1Changed(int state);
  void slotCluster2Changed(int state);
  void slotCluster3Changed(int state);
  void slotCluster4Changed(int state);
  void slotCluster5Changed(int state);

 private:
  QComboBox *m_channelCombo;
  QCheckBox *m_clusterCheck[6];
  QLineEdit *m_referenceEdit;
  unsigned char m_clusterState[32];
  int m_clusterOffset; // maps check box to cluster index
};


//--------------------------------------------------------------------------

class CWInstrOmiEdit : public CWCalibInstrEdit
{
 public:
  CWInstrOmiEdit(const struct instrumental_omi *d, QWidget *parent = 0);
  virtual ~CWInstrOmiEdit();

  void apply(struct instrumental_omi *d) const;

 private:
  QComboBox *m_spectralTypeCombo;
  QLineEdit *m_minLambdaEdit, *m_maxLambdaEdit;
  QCheckBox *m_averageCheck;
};


#endif


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
class CWInstrRasasEdit;
class CWInstrCcdEdit;
class CWInstrPdaEggUlbEdit;
class CWInstrCcdEevEdit;

//--------------------------------------------------------------------------
class CWProjectTabInstrumental : public QFrame
{
 public:
  CWProjectTabInstrumental(const mediate_project_instrumental_t *instr, QWidget *parent = 0);
  virtual ~CWProjectTabInstrumental();

  void apply(mediate_project_instrumental_t *instr) const;

 private:
  QComboBox *m_formatCombo, *m_siteCombo;
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
  CWInstrCcdEdit *m_ccdUlbEdit;
  CWInstrSaozEdit *m_saozVisEdit;
  CWInstrSaozEdit *m_saozUvEdit;
  CWInstrRasasEdit *m_saozEfmEdit;
  CWInstrRasasEdit *m_rasasEdit;
  CWInstrRasasEdit *m_pdasiEasoeEdit;
  CWInstrLoggerEdit *m_pdasiOsmaEdit;
  CWInstrCcdEevEdit *m_ccdEevEdit;
  CWInstrRasasEdit *m_uoftEdit;
  CWInstrRasasEdit *m_noaaEdit;

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
  void helperConstructFileWidgets(QGridLayout *gridLayout, int &row,
				  const char *calib, int lenCalib,
				  const char *instr, int lenInstr);

 public slots:
  void slotCalibBrowse();
  void slotInstrBrowse();

 protected:
  QLineEdit *m_calibEdit, *m_instrEdit;
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
  void helperConstructFileWidgets(QGridLayout *gridLayout, int &row,
				  const char *calib, int lenCalib,
				  const char *instr, int lenInstr,
				  const char *ipv, int lenIpv,
				  const char *dnl, int lenDnl);
  
 public slots:
  void slotInterPixelVariabilityBrowse();
  void slotDetectorNonLinearityBrowse();

 protected:
  QLineEdit *m_ipvEdit, *m_dnlEdit;
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

class CWInstrRasasEdit : public CWCalibInstrEdit
{
 public:
  CWInstrRasasEdit(const struct instrumental_rasas *d, QWidget *parent = 0);
  virtual ~CWInstrRasasEdit();
  
  void apply(struct instrumental_rasas *d) const;
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
};



#endif


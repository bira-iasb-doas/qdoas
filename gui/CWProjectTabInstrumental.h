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

#include "mediate_project.h"

class CWInstrAsciiEdit;

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
};

//--------------------------------------------------------------------------

class CWInstrAsciiEdit : public QFrame
{
Q_OBJECT
 public:
  CWInstrAsciiEdit(const struct instrumental_ascii *d, QWidget *parent = 0);
  virtual ~CWInstrAsciiEdit();

  void apply(struct instrumental_ascii *d) const;

 public slots:
   void slotCalibBrowse();
   void slotInstrBrowse();

 private:
   QLineEdit *m_detSizeEdit, *m_calibEdit, *m_instrEdit;
   QRadioButton *m_lineRadioButton, *m_columnRadioButton;
   QCheckBox *m_zenCheck, *m_aziCheck, *m_eleCheck, *m_dateCheck, *m_timeCheck, *m_lambdaCheck;
};

#endif


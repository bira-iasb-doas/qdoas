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


#ifndef _CWPROJECTTABSLIT_H_GUARD
#define _CWPROJECTTABSLIT_H_GUARD

#include <QFrame>
#include <QComboBox>
#include <QStackedWidget>
#include <QLineEdit>
#include <QCheckBox>

#include "mediate_project.h"

class CWSlitFileEdit;
class CWSlitGaussianEdit;

//--------------------------------------------------------------------------
class CWProjectTabSlit : public QFrame
{
 public:
  CWProjectTabSlit(const mediate_project_slit_t *instr, QWidget *parent = 0);
  virtual ~CWProjectTabSlit();

  void apply(mediate_project_slit_t *slit) const;

 private:
  QLineEdit *m_solarRefFileEdit;
  QCheckBox *m_fwhmCorrectionCheck;
  QComboBox *m_slitCombo;
  QStackedWidget *m_slitStack;
  // widgets for the configuration of each slit
  CWSlitFileEdit *m_fileEdit;
  CWSlitGaussianEdit *m_gaussianEdit;
};

//--------------------------------------------------------------------------

class CWSlitFileEdit : public QFrame
{
 public:
  CWSlitFileEdit(const struct slit_file *d, QWidget *parent = 0);
  virtual ~CWSlitFileEdit();

  void apply(struct slit_file *d) const;

 private:
  QLineEdit *m_filenameEdit;
};

class CWSlitGaussianEdit : public QFrame
{
 public:
  CWSlitGaussianEdit(const struct slit_gaussian *d, QWidget *parent = 0);
  virtual ~CWSlitGaussianEdit();

  void apply(struct slit_gaussian *d) const;

 private:
  QLineEdit *m_fwhmEdit;
};


#endif


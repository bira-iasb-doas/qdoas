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


#ifndef _CWSLITEDITORS_H_GUARD
#define _CWSLITEDITORS_H_GUARD

#include <QFrame>
#include <QComboBox>
#include <QStackedWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QGridLayout>

#include "mediate_general.h"

//--------------------------------------------------------------------------
// base for specifc editor that require a slit function file

class CWSlitFileBase : public QFrame
{
Q_OBJECT
 public:
  CWSlitFileBase(QWidget *parent = 0);
  virtual ~CWSlitFileBase();

 protected:
  void helperConstructFileEdit(QGridLayout *gridLayout, int &row, const char *str, int len);

 public slots:
  void slotBrowseFile();
   
 protected:
  QLineEdit *m_filenameEdit;
};

//--------------------------------------------------------------------------

class CWSlitFileEdit : public CWSlitFileBase
{
 public:
  CWSlitFileEdit(const struct slit_file *d, QWidget *parent = 0);
  virtual ~CWSlitFileEdit();

  void apply(struct slit_file *d) const;
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

class CWSlitLorentzEdit : public QFrame
{
 public:
  CWSlitLorentzEdit(const struct slit_lorentz *d, QWidget *parent = 0);
  virtual ~CWSlitLorentzEdit();

  void apply(struct slit_lorentz *d) const;

 private:
  QLineEdit *m_widthEdit;
  QSpinBox *m_degreeSpin;
};

class CWSlitVoigtEdit : public QFrame
{
 public:
  CWSlitVoigtEdit(const struct slit_voigt *d, QWidget *parent = 0);
  virtual ~CWSlitVoigtEdit();

  void apply(struct slit_voigt *d) const;

 private:
  QLineEdit *m_fwhmLeftEdit, *m_fwhmRightEdit;
  QLineEdit *m_ratioLeftEdit, *m_ratioRightEdit;
};

class CWSlitErrorEdit : public QFrame
{
 public:
  CWSlitErrorEdit(const struct slit_error *d, QWidget *parent = 0);
  virtual ~CWSlitErrorEdit();

  void apply(struct slit_error *d) const;

 private:
  QLineEdit *m_fwhmEdit;
  QLineEdit *m_widthEdit;
};

class CWSlitApodEdit : public QFrame
{
 public:
  CWSlitApodEdit(const struct slit_apod *d, QWidget *parent = 0);
  virtual ~CWSlitApodEdit();

  void apply(struct slit_apod *d) const;

 private:
  QLineEdit *m_resolutionEdit;
  QLineEdit *m_phaseEdit;
};

class CWSlitLorentzFileEdit : public CWSlitFileBase
{
 public:
  CWSlitLorentzFileEdit(const struct slit_lorentz_file *d, QWidget *parent = 0);
  virtual ~CWSlitLorentzFileEdit();

  void apply(struct slit_lorentz_file *d) const;

 private:
  QSpinBox *m_degreeSpin;
};

class CWSlitErrorFileEdit : public CWSlitFileBase
{
 public:
  CWSlitErrorFileEdit(const struct slit_error_file *d, QWidget *parent = 0);
  virtual ~CWSlitErrorFileEdit();

  void apply(struct slit_error_file *d) const;

 private:
  QLineEdit *m_widthEdit;
};

#endif

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

#ifndef _CWPROJECTTABFILTERING_H_GUARD
#define _CWPROJECTTABFILTERING_H_GUARD

#include <QFrame>
#include <QComboBox>
#include <QStackedWidget>
#include <QLineEdit>
#include <QSpinBox>

#include "mediate_project.h"

class CWKaiserEdit;
class CWBoxcarEdit;
class CWGaussianEdit;

//--------------------------------------------------------------------------
class CWProjectTabFiltering : public QFrame
{
 public:
  CWProjectTabFiltering(const mediate_project_filtering_t *lowpass,
			const mediate_project_filtering_t *highpass, QWidget *parent = 0);
  virtual ~CWProjectTabFiltering();

  void apply(mediate_project_filtering_t *lowpass, mediate_project_filtering_t *highpass) const;

 private:
  QComboBox *m_lowCombo, *m_highCombo;
  QStackedWidget *m_lowStack, *m_highStack;
  // widgets for the configuration of each filter (low and high). 

  CWBoxcarEdit *m_lowBoxcar, *m_highBoxcar;
  CWKaiserEdit *m_lowKaiser, *m_highKaiser;
  CWGaussianEdit *m_lowGaussian, *m_highGaussian;
};

//--------------------------------------------------------------------------

class CWKaiserEdit : public QFrame
{
 public:
  CWKaiserEdit(const struct filter_kaiser *d, QWidget *parent = 0);
  virtual ~CWKaiserEdit();

  void apply(struct filter_kaiser *d) const;

 private:
  QLineEdit *m_cutoffEdit, *m_toleranceEdit, *m_passbandEdit;
  QSpinBox *m_iterationsSpinBox;
};

//--------------------------------------------------------------------------

class CWBoxcarEdit : public QFrame
{
 public:
  CWBoxcarEdit(const struct filter_boxcar *d, QWidget *parent = 0);
  virtual ~CWBoxcarEdit();

  void apply(struct filter_boxcar *d) const;

 private:
  QSpinBox *m_widthSpinBox;
  QSpinBox *m_iterationsSpinBox;
};

//--------------------------------------------------------------------------

class CWGaussianEdit : public QFrame
{
 public:
  CWGaussianEdit(const struct filter_gaussian *d, QWidget *parent = 0);
  virtual ~CWGaussianEdit();

  void apply(struct filter_gaussian *d) const;

 private:
  QLineEdit *m_fwhmEdit;
  QSpinBox *m_iterationsSpinBox;
};

#endif


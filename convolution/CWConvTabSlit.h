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


#ifndef _CWCONVTABSLIT_H_GUARD
#define _CWCONVTABSLIT_H_GUARD

#include <QFrame>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>

#include "mediate_convolution.h"

#include "CWSlitEditors.h"

class CWSlitSelector;

class CWConvTabSlit : public QFrame
{
Q_OBJECT
 public:
  CWConvTabSlit(const mediate_conv_slit_t *conv, const mediate_conv_slit_t *deconv, QWidget *parent = 0);
  virtual ~CWConvTabSlit();

  void apply(mediate_conv_slit_t *conv, mediate_conv_slit_t *deconv) const;

 private:
  CWSlitSelector *m_convEdit, *m_deconvEdit;
};


class CWSlitSelector : public QGroupBox
{
 public:
  CWSlitSelector(const mediate_conv_slit_t *slit, const QString &title, QWidget *parent = 0);
  virtual ~CWSlitSelector();

  void apply(mediate_conv_slit_t *slit) const;

 private:
  QComboBox *m_slitCombo;
  QStackedWidget *m_slitStack;
  // widgets for the configuration of each slit
  CWSlitFileEdit *m_fileEdit;
  CWSlitGaussianEdit *m_gaussianEdit;
  CWSlitLorentzEdit *m_lorentzEdit;
  CWSlitVoigtEdit *m_voigtEdit;
  CWSlitErrorEdit *m_errorEdit;
  CWSlitApodEdit *m_boxcarApodEdit, *m_nbsApodEdit;
  CWSlitFileEdit *m_gaussianFileEdit;
  CWSlitLorentzFileEdit *m_lorentzFileEdit;
  CWSlitErrorFileEdit *m_errorFileEdit;
  CWSlitFileEdit *m_gaussianTempFileEdit;
  CWSlitErrorFileEdit *m_errorTempFileEdit;
};

#endif

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


#ifndef _CWPROJECTTABCALIBRATION_H_GUARD
#define _CWPROJECTTABCALIBRATION_H_GUARD

#include <QFrame>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

#include "mediate_project.h"

#include <QTabWidget>

#include "mediate_project.h"

#include "CWAnalysisWindowDoasTables.h"


class CWProjectTabCalibration : public QFrame
{
Q_OBJECT
 public:
  CWProjectTabCalibration(const mediate_project_calibration_t *properties, QWidget *parent = 0);
  virtual ~CWProjectTabCalibration();

  void apply(mediate_project_calibration_t *properties) const;

 public slots:
  void slotLineShapeSelectionChanged(int index);
  void slotBrowseSolarRefFile();

 private:
  QLineEdit *m_refFileEdit;
  QComboBox *m_methodCombo;
  QComboBox *m_lineShapeCombo;
  QFrame *m_degreeWidget;
  QCheckBox *m_spectraCheck, *m_fitsCheck, *m_residualCheck, *m_shiftSfpCheck;
  QSpinBox *m_degreeSpinBox, *m_shiftDegreeSpinBox, *m_sfpDegreeSpinBox;
  QLineEdit *m_lambdaMinEdit, *m_lambdaMaxEdit;
  QSpinBox *m_subWindowsSpinBox;

  QTabWidget *m_tabs;

  CWMoleculesDoasTable *m_moleculesTab;
  CWLinearParametersDoasTable *m_linearTab;
  CWShiftAndStretchDoasTable *m_shiftAndStretchTab;
  CWGapDoasTable *m_gapTab;
  CWOutputDoasTable *m_outputTab;
};

#endif

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


#ifndef _CWANALYSISWINDOWPROPERTYEDITOR_H_GUARD
#define _CWANALYSISWINDOWPROPERTYEDITOR_H_GUARD

#include <QString>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QStackedLayout>

#include "CWEditor.h"
#include "CWAnalysisWindowDoasTables.h"

class CWAnalysisWindowPropertyEditor : public CWEditor
{
Q_OBJECT
 public:
  CWAnalysisWindowPropertyEditor(const QString &projectName, const QString &analysisWindowName, QWidget *parent = 0);
  virtual ~CWAnalysisWindowPropertyEditor();

  virtual bool actionOk();
  virtual void actionHelp();

 public slots:
  void slotRefSelectionChanged(bool checked);
  void slotWavelengthCalibrationChanged(int index);
  void slotBrowseRefOne();
  void slotBrowseRefTwo();
  void slotBrowseResidual();

 private:
  QComboBox *m_calibrationCombo;
  QLineEdit *m_fitMinEdit, *m_fitMaxEdit;
  QCheckBox *m_spectrumCheck, *m_polyCheck, *m_fitsCheck;
  QCheckBox *m_residualCheck, *m_predefCheck, *m_ratioCheck;

  QFrame *m_refOneFrame, *m_refTwoEditFrame, *m_refTwoSzaFrame;
  QLineEdit *m_refOneEdit, *m_refTwoEdit, *m_residualEdit;
  QLineEdit *m_szaCenterEdit, *m_szaDeltaEdit;
  QStackedLayout *m_refTwoStack;
  

  QTabWidget *m_tabs;
  // specialized DoasTables for each tab ...
  CWMoleculesDoasTable *m_moleculesTab;
  CWLinearParametersDoasTable *m_linearTab;
  CWShiftAndStretchDoasTable *m_shiftAndStretchTab;

  QString m_projectName, m_analysisWindowName;
};

#endif

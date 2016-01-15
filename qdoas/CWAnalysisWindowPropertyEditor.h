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
#include <QSpinBox>
#include <QRadioButton>
#include <QPushButton>
#include <QStackedLayout>

#include "CWEditor.h"
#include "CWorkSpace.h"
#include "CWAnalysisWindowDoasTables.h"

class PolynomialTab;

class CWAnalysisWindowPropertyEditor : public CWEditor, public CProjectObserver
{
Q_OBJECT
 public:
  CWAnalysisWindowPropertyEditor(const QString &projectName, const QString &analysisWindowName, QWidget *parent = 0);

  virtual bool actionOk();
  virtual void actionHelp();

  virtual void updateModifyProject(const QString &projectName);

 private:
  void projectPropertiesChanged();

 public slots:
  void slotRefSelectionChanged(bool checked);
  void slotWavelengthCalibrationChanged(int index);
  void slotBrowseRefOne();
  void slotBrowseRefTwo();
  void slotBrowseResidual();
  void slotMaxdoasSelectionChanged(bool checked);
  void slotPageChanged(int index);

 private:
  QComboBox *m_calibrationCombo;
  QLineEdit *m_fitMinEdit, *m_fitMaxEdit,*m_resolEdit;
  QCheckBox *m_spectrumCheck, *m_polyCheck, *m_fitsCheck;
  QCheckBox *m_residualCheck, *m_predefCheck, *m_ratioCheck;

  QFrame *m_refOneFrame;
  QFrame *m_refTwoEditFrame, *m_refTwoSzaFrame, *m_satelliteFrame, *m_maxdoasFrame, *m_maxdoasSzaFrame, *m_maxdoasScanFrame;
  QLineEdit *m_refOneEdit, *m_refTwoEdit, *m_residualEdit;
  QLineEdit *m_szaCenterEdit, *m_szaDeltaEdit;
  QLineEdit *m_maxdoasSzaCenterEdit, *m_maxdoasSzaDeltaEdit;

  QComboBox *m_scanCombo;
  // satellite only ...
  QLineEdit *m_refTwoLonMinEdit, *m_refTwoLonMaxEdit;
  QLineEdit *m_refTwoLatMinEdit, *m_refTwoLatMaxEdit;
  QLineEdit *m_cloudFractionMinEdit,*m_cloudFractionMaxEdit;
  QSpinBox *m_refTwoNsSpin;
  // gome pixel type ...
  QCheckBox *m_eastCheck, *m_centerCheck, *m_westCheck, *m_backscanCheck;

  QStackedLayout *m_refTwoStack;
  QStackedLayout *m_residualStack;

  QTabWidget *m_tabs;
  // specialized DoasTables for each tab ...
  CWMoleculesDoasTable *m_moleculesTab;
  PolynomialTab *m_linearTab;
  CWNonLinearParametersDoasTable *m_nonLinearTab;
  CWShiftAndStretchDoasTable *m_shiftAndStretchTab;
  CWGapDoasTable *m_gapTab;
  CWOutputDoasTable *m_outputTab;


  QString m_projectName, m_analysisWindowName;
  bool m_autoSelection, m_activePixelType, m_activeCloudFraction,m_scanSelection;
  int m_selectedPage;
};

#endif

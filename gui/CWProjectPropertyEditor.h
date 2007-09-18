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

#ifndef _CWPROJECTPROPERTYEDITOR_H_GUARD
#define _CWPROJECTPROPERTYEDITOR_H_GUARD

#include <QLineEdit>
#include <QStringList>
#include <QTreeWidgetItem>

#include "CWEditor.h"
#include "CWProjectTabSpectra.h"
#include "CWProjectTabAnalysis.h"
#include "CWProjectTabFiltering.h"
#include "CWProjectTabCalibration.h"
#include "CWProjectTabUndersampling.h"
#include "CWProjectTabInstrumental.h"
#include "CWProjectTabSlit.h"

class CWProjectTree;

class CWProjectPropertyEditor : public CWEditor
{
Q_OBJECT
 public:
  CWProjectPropertyEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                          QWidget *parent = 0);
  virtual ~CWProjectPropertyEditor();

  virtual bool actionOk();
  virtual void actionHelp();

 private:
  QTabWidget *m_tabs;
  CWProjectTabSpectra *m_spectraTab;
  CWProjectTabAnalysis *m_analysisTab;
  CWProjectTabFiltering *m_filteringTab;
  CWProjectTabCalibration *m_calibrationTab;
  CWProjectTabUndersampling *m_undersamplingTab;
  CWProjectTabInstrumental *m_instrumentalTab;
  CWProjectTabSlit *m_slitTab;

  CWProjectTree *m_projectTree;
  QString m_projectName;
};

#endif

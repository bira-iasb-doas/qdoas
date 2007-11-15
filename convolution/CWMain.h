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


#ifndef _CWMAIN_H_GUARD
#define _CWMAIN_H_GUARD

#include <cstdio>

#include <QFrame>
#include <QMenuBar>
#include <QTabWidget>
#include <QAction>

#include "CWConvTabGeneral.h"
#include "CWConvTabSlit.h"
#include "CWFilteringEditor.h"

class CConvEngineController;
class CHelpSystem;

class CWMain : public QFrame
{
Q_OBJECT
 public:

  CWMain(QWidget *parent = 0);
  virtual ~CWMain();

  virtual void closeEvent(QCloseEvent *e);
 
 protected:
  void writeConfiguration(FILE *fp);
  bool checkStateAndConsiderSaveFile(void);
  void setConfigFileName(const QString &fileName);

 public slots:
  void slotOpenFile();
  void slotNewFile();
  void slotSaveFile();
  void slotSaveAsFile();
  void slotQdoasHelp();
  void slotHelpBrowserPreference(bool light);
  void slotAboutQdoas();
  void slotAboutQt();
  void slotErrorMessages(int highestLevel, const QString &messages);
  void slotOk();
  
 private:
  QMenuBar *m_menuBar;
  QTabWidget *m_pageTab;

  CWConvTabGeneral *m_generalTab;
  CWConvTabSlit *m_slitTab;
  CWFilteringEditor *m_filteringTab;

  QString m_configFile;
  
  CConvEngineController *m_controller;

  // actions ...
  QAction *m_saveAction;
  QAction *m_saveAsAction;

  bool m_stateValid;
  mediate_convolution_t m_properties;

  // help interface ...
  CHelpSystem *m_helpInterface;
};

#endif

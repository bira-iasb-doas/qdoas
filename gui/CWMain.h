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

#include <QFrame>
#include <QMenuBar>
#include <QToolBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableView>

class CWProjectTree;
class CWSiteTree;
class CWUserSymbolTree;
class CWActiveContext;
class CEngineController;
class CWTableRegion;

class CWMain : public QFrame
{
Q_OBJECT
 public:

  CWMain(QWidget *parent = 0);
  virtual ~CWMain();

  virtual void closeEvent(QCloseEvent *e);
 
 public slots:
  void slotOpenFile();
  void slotErrorMessages(int highestLevel, const QString &messages);

 private:
  QMenuBar *m_menuBar;
  QToolBar *m_toolBar;
  QTabWidget *m_projEnvTab;

  CWProjectTree *m_projTree;
  CWSiteTree *m_siteTree;
  CWUserSymbolTree *m_userSymbolTree;

  CWTableRegion *m_tableRegion;

  CWActiveContext *m_activeContext;

  QStatusBar *m_statusBar;

  CEngineController *m_controller;
  
};

#endif

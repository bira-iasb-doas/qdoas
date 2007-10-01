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

#ifndef _CNAVIGATIONPANEL_H_GUARD
#define _CNAVIGATIONPANEL_H_GUARD

#include <QObject>
#include <QAction>
#include <QToolBar>
#include <QLineEdit>
#include <QComboBox>
#include <QStringList>

// provide a set of tool-bar actions for navigation

class CNavigationPanel : public QObject
{
Q_OBJECT
 public:
  enum NavButton { eFirst, ePrevious, eNext, eLast };

  CNavigationPanel(QToolBar *toolBar);
  virtual ~CNavigationPanel();

 public slots:
  void slotSetMaxIndex(int maxIndex);
  void slotSetCurrentIndex(int index);
  void slotSetFileList(const QStringList &fileList);
  void slotSetCurrentFile(int index);
  void slotSetEnabled(bool enabled); 

 private slots:
  void slotFirstClicked();
  void slotPreviousClicked();
  void slotNextClicked();
  void slotLastClicked();
  void slotStopClicked();
  void slotIndexEditChanged();
  void slotFileSelected(int);

 signals:
  void signalFirstClicked();
  void signalPreviousClicked();
  void signalNextClicked();
  void signalLastClicked();
  void signalStopClicked();
  void signalIndexChanged(int);
  void signalSelectedFileChanged(int);

 private:
  QAction *m_firstBtn, *m_prevBtn, *m_nextBtn, *m_lastBtn, *m_stopBtn;
  QLineEdit *m_indexEdit;
  QComboBox *m_fileCombo;

  int m_maxIndex, m_currentIndex;
};

#endif

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


#ifndef _CHELPSYSTEMIMPL_H_GUARD
#define _CHELPSYSTEMIMPL_H_GUARD

#include <QFrame>
#include <QTextBrowser>

#include "CHelpSystem.h"


class CHelpImpl : public QObject
{
 public:
  enum eStatus { Uncertain, Open, Closed, Error };

  CHelpImpl();
  virtual ~CHelpImpl();

  virtual void display(const QString &url) = 0;
};

// Two possible clients .. QtAssistant or QBrowser

#if defined(HELP_QT_ASSISTANT)
// Has QT Assistant Support

#include <QAssistantClient>

class CHelpImplAssistant : public CHelpImpl
{
Q_OBJECT
 public:
  CHelpImplAssistant(const QString &profile, const QString &helpDir, QObject *parent = 0);
  virtual ~CHelpImplAssistant();
  
  void display(const QString &url);

  CHelpImpl::eStatus status(void) const;

 private slots:
  void slotAssistantOpened();
  void slotAssistantClosed();
  void slotAssistantError(const QString &msg);

 private:
  QAssistantClient *m_client;
  QString m_helpDir;
  CHelpImpl::eStatus m_status;
};

#endif

class CBasicHelpBrowser;

class CHelpImplTextBrowser : public CHelpImpl
{
 public:
  CHelpImplTextBrowser(const QString &helpDir, const QString &startPage);
  virtual ~CHelpImplTextBrowser();
  
  void display(const QString &url);

  void browserClosed();

 private:
  CBasicHelpBrowser *m_browser;
  QString m_helpDir, m_startPage;
};

class CBasicHelpBrowser : public QFrame
{
Q_OBJECT
 public:
  CBasicHelpBrowser(const QString &homeUrl, CHelpImplTextBrowser *owner, QWidget *parent = 0);
  virtual ~CBasicHelpBrowser();
 
  void displayDocument(const QString &url);
 
 protected:
  virtual void closeEvent(QCloseEvent *e);

 public slots:
  void slotHome();

 private:
  QString m_homeUrl;
  CHelpImplTextBrowser *m_owner;
  QTextBrowser *m_browser;
};

#endif

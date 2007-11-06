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


#include <QCloseEvent>
#include <QVBoxLayout>
#include <QToolBar>
#include <QIcon>
#include <QAction>
#include <QDir>

#include <QMessageBox> // temp

#include "CHelpSystemImpl.h"
#include "CPreferences.h"

#include "debugutil.h"

CHelpImpl::CHelpImpl()
{
}

CHelpImpl::~CHelpImpl()
{
}

// Two possible clients .. Qt-Assistant or QTextBrowser

//-----------------------------------------------------------------------

#if defined(HELP_QT_ASSISTANT)

CHelpImplAssistant::CHelpImplAssistant(const QString &profile, const QString &helpDir, QObject *parent) :
  CHelpImpl(),
  m_helpDir(helpDir),
  m_status(CHelpImpl::Uncertain)
{
  // need the ... location of the assistant binary
  QString path = CPreferences::instance()->directoryName("Assistant");
    
  QStringList arguments;
  arguments << "-profile" << profile;
  
  m_client = new QAssistantClient(path, parent);
  m_client->setArguments(arguments);
  
  // the openAssistant call is asynchronous ... which means that this object must
  // be constructed before we can determine if it is 'useful'...   So...
  // connect to the signals emitted and provide an alternative way for the
  // help system to 'switch implementations' ...
  
  connect(m_client, SIGNAL(assistantOpened()), this, SLOT(slotAssistantOpened()));
  connect(m_client, SIGNAL(assistantClosed()), this, SLOT(slotAssistantClosed()));
  connect(m_client, SIGNAL(error(const QString&)), this, SLOT(slotAssistantError(const QString&)));

  m_client->openAssistant();
}

CHelpImplAssistant::~CHelpImplAssistant()
{
  m_client->closeAssistant();
  delete m_client;
}
  
void CHelpImplAssistant::display(const QString &url)
{
  if (m_status == CHelpImpl::Open) {
    m_client->showPage(m_helpDir + QDir::separator() + url);
  }
  else if (m_status == CHelpImpl::Closed) {
    m_client->openAssistant();
    if (!url.isEmpty())    
      m_client->showPage(m_helpDir + QDir::separator() + url);
  }
}

CHelpImpl::eStatus CHelpImplAssistant::status(void) const
{
  return m_status;
}

void CHelpImplAssistant::slotAssistantOpened()
{
  TRACE("Assistant Opened");
  m_status = CHelpImpl::Open;
}

void CHelpImplAssistant::slotAssistantClosed()
{
  TRACE("Assistant Closed");
  m_status = CHelpImpl::Closed;
}

void CHelpImplAssistant::slotAssistantError(const QString &msg)
{
  TRACE("Assistant Error " << msg.toStdString());
  
  // parse the error to determine how serious this is ...
  if (msg.startsWith("Failed to start"))
    m_status = CHelpImpl::Error;
}

#endif

//-----------------------------------------------------------------------

CHelpImplTextBrowser::CHelpImplTextBrowser(const QString &helpDir, const QString &startPage) :
  CHelpImpl(),
  m_browser(NULL),
  m_helpDir(helpDir),
  m_startPage(startPage)
{
  m_helpDir.prepend("file:///");

  int len = m_helpDir.length();
  int i = 0;
  while (i < len) {
    if (m_helpDir[i] == '\\') m_helpDir[i] = '/';
    ++i;
  }
  if (m_helpDir[len-1] != '/')
    m_helpDir += '/';

  // lazy evaluate the browser ...
}

CHelpImplTextBrowser::~CHelpImplTextBrowser()
{
  if (m_browser) {
    delete m_browser;
  }
}

void CHelpImplTextBrowser::display(const QString &url)
{
  if (!m_browser) {
    m_browser = new CBasicHelpBrowser(m_helpDir + m_startPage, this); // no  parent
  }

  if (m_browser) {
    m_browser->displayDocument(m_helpDir + url);
    if (m_browser->isMinimized())
      m_browser->showNormal();
    else
      m_browser->show();
      
    m_browser->raise();
  }
}

void CHelpImplTextBrowser::browserClosed(void)
{
  // the browser is closing ... reset
  m_browser = NULL;
}

CBasicHelpBrowser::CBasicHelpBrowser(const QString &homeUrl, CHelpImplTextBrowser *owner, QWidget *parent) :
  QFrame(parent, Qt::Window|Qt::WindowMinMaxButtonsHint|Qt::WindowSystemMenuHint),
  m_homeUrl(homeUrl),
  m_owner(owner)
{
  setWindowTitle("Qdoas User Manual");
  setWindowIcon(QIcon(QPixmap(":/icons/logo.png")));

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(0);
  mainLayout->setSpacing(0);

  m_browser = new QTextBrowser(this);

  QToolBar *toolBar = new QToolBar(this);
  QAction *backAction = toolBar->addAction(QIcon(":/icons/help_back_32.png"), "Back", m_browser, SLOT(backward()));
  QAction *forwardAction = toolBar->addAction(QIcon(":/icons/help_forward_32.png"),"Forward", m_browser, SLOT(forward()));
  toolBar->addAction(QIcon(":/icons/help_home_32.png"), "Home", this, SLOT(slotHome()));
  toolBar->addAction(QIcon(":/icons/help_exit_32.png"), "Close", this, SLOT(close()));

  mainLayout->addWidget(toolBar);

  mainLayout->addWidget(m_browser, 1);

  // connections
  connect(m_browser, SIGNAL(backwardAvailable(bool)), backAction, SLOT(setEnabled(bool)));
  connect(m_browser, SIGNAL(forwardAvailable(bool)), forwardAction, SLOT(setEnabled(bool)));

  backAction->setEnabled(false);
  forwardAction->setEnabled(false);

  // resize to preferences
  resize(CPreferences::instance()->windowSize("Help", QSize(400,350)));
}

CBasicHelpBrowser::~CBasicHelpBrowser()
{
}

void CBasicHelpBrowser::displayDocument(const QString &url)
{
  m_browser->setSource(url);
}

void CBasicHelpBrowser::closeEvent(QCloseEvent *e)
{
  // about to close ...
  m_owner->browserClosed();

  // store the size
  CPreferences::instance()->setWindowSize("Help", size());

  e->accept();
}

void CBasicHelpBrowser::slotHome()
{
  m_browser->setSource(m_homeUrl);
}

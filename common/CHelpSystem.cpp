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

#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileDialog>
#include <QCoreApplication>
#include <QTime>
#include <QApplication>

#include "CHelpSystem.h"
#include "CHelpSystemImpl.h"
#include "CPreferences.h"

#include "debugutil.h"

// relative to the root directory for the help files
#define HELP_HOME_PAGE "index.html"

CHelpSystem* CHelpSystem::m_instance = NULL;

CHelpSystem* CHelpSystem::establishHelpSystem(QWidget *parent)
{
  if (m_instance == NULL && parent != NULL) {
    m_instance = new CHelpSystem(parent);
    return m_instance;
  }

  // only want one object to create this system and control it...
  // All others use the static showHelpTopic method and hope for the best.
  return NULL;
}

void CHelpSystem::showHelpTopic(const QString &chapter, const QString &key)
{
  if (m_instance) {
    m_instance->openBrowser(chapter, key);
  }
}

CHelpSystem::CHelpSystem(QWidget *parent) :
  m_parentWidget(parent),
  m_impl(NULL),
  m_block(false),
  m_preferLightBrowser(false)
{
  // the parent is needed for the impl... it MUST have a lifetime
  // at least as long as this.

  // populate the map ...

  m_chapterMap.insert(std::map<QString,int>::value_type("introduction", 1));
  m_chapterMap.insert(std::map<QString,int>::value_type("installation", 2));
  m_chapterMap.insert(std::map<QString,int>::value_type("gui", 3));
  m_chapterMap.insert(std::map<QString,int>::value_type("algorithms", 4));
  m_chapterMap.insert(std::map<QString,int>::value_type("quickstart", 5));
  m_chapterMap.insert(std::map<QString,int>::value_type("site", 6));
  m_chapterMap.insert(std::map<QString,int>::value_type("symbol", 6));
  m_chapterMap.insert(std::map<QString,int>::value_type("project", 7));
  m_chapterMap.insert(std::map<QString,int>::value_type("tool", 8));
  m_chapterMap.insert(std::map<QString,int>::value_type("annex", 9));
  m_chapterMap.insert(std::map<QString,int>::value_type("appendix", 9));
}

CHelpSystem::~CHelpSystem()
{
  m_instance = NULL;
  // shutdown the implementation
  delete m_impl;
}

void CHelpSystem::openBrowser(void)
{
  createImplementation(&m_impl);

  if (m_impl)
    m_impl->display(HELP_HOME_PAGE); // the table of contents
}

void CHelpSystem::openBrowser(const QString &chapter, const QString &key)
{
  createImplementation(&m_impl);

  if (m_impl) {
    QString url;
    // resolve chapter
    std::map<QString,int>::const_iterator it = m_chapterMap.find(chapter);
    if (it != m_chapterMap.end()) {
      // construct the URL (relative)
      QTextStream stream(&url);
      stream << "chapter" << (it->second) << "/chapter" << (it->second) << ".html";
      if (!key.isEmpty())
	stream << '#' << key;
    }
    else {
      url = HELP_HOME_PAGE;
    }

    m_impl->display(url);
  }
}

void CHelpSystem::closeBrowser(void)
{
  delete m_impl;
  m_impl = NULL;
}

void CHelpSystem::preferLightBrowser(bool light)
{
#if defined(HELP_QT_ASSISTANT)
  if (light != m_preferLightBrowser) {
    m_preferLightBrowser = light;
    closeBrowser();
  }
#endif
}

bool CHelpSystem::supportsQtAssistant(void) const
{
#if defined(HELP_QT_ASSISTANT)
  return true;
#else
  return false;
#endif
}

void CHelpSystem::createImplementation(CHelpImpl **impl)
{
  // try and create an concete instance of a CHelpImpl implementation class. This
  // does nothing if *impl is not null.

  // this routine safely blocks recursive calls (the recursision is obscure, and can results
  // from explicit eent processing ... see the comments in the 'Assistant' section below.

  if (*impl || m_block) return; // critically, this does NOT change *impl
  m_block = true;

  // need the profile file (to locate the help files/directory)

  bool storeHelpDir = false;
  QString helpDir = CPreferences::instance()->directoryName("Help", ".");

  QString profile = helpDir + QDir::separator() + "qdoas.hlp";

  while (!profile.isEmpty() && !QFile::exists(profile)) {
    profile = QFileDialog::getOpenFileName(m_parentWidget, "Help File", QString(), "qdoas.hlp");
    storeHelpDir = true;
  }
  if (!profile.isEmpty()) {
    if (storeHelpDir) {
      helpDir = CPreferences::dirName(profile); // just the directory name
      CPreferences::instance()->setDirectoryName("Help", helpDir);
    }

#if defined(HELP_QT_ASSISTANT)
    // Qt Assistant is the first choice... but this is a bit tricky.
    //   The CHelpImplAssistant constructor attempts to open Assistant (as a process),
    //   but construction completes before it is known if the assistant process succeeded
    //   to start. Signals are conneted to slots of the CHelpImplAssistant instance that
    //   allow the the status of the Assistant process to be determined. After construction
    //   we poll the CHelpImplAssistant instance until it's status is known. We explicitly
    //   request the Qt engine to process events, otherwise the signals that indicate the
    //   status of the Assistant process will never be delivered, resulting in a hung process.

    if (!m_preferLightBrowser) {

      CHelpImplAssistant *tmp = new CHelpImplAssistant(profile, helpDir, m_parentWidget);

      // wait until we know if this succeeded or failed ... need to process events for this to work.
      // allow at most 10 seconds...
      QTime monitor;
      monitor.start();
      CHelpImpl::Status status = tmp->status();

      QApplication::setOverrideCursor(Qt::WaitCursor);

      while (status == CHelpImpl::eUncertain && monitor.elapsed() < 10000) {
	// wait for 100ms, even if there are no events.
	QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	status = tmp->status();
      }

      QApplication::restoreOverrideCursor();

      if (status == CHelpImpl::eOpen) {
	// started -> set *impl, release the recursion block and return
	*impl = tmp;
	m_block = false;
	return;
      }
      else {
	delete tmp; // parhaps failed, perhaps still trying ...
      }
    }

#endif

    // browser based ... be it preference or otherwise

    *impl = new CHelpImplTextBrowser(helpDir, HELP_HOME_PAGE);
  }

  // makes no change to *impl
  m_block = false;
}


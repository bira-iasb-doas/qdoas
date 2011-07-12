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
#include <QDesktopServices>
#include <QUrl>
// #include <QWebView>

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

bool CHelpSystem::changeDir(const QString &chapter, const QString &key)
 {
 	QString helpDir=CPreferences::instance()->directoryName("Help", ".");
 	QString indexFile=helpDir+"/"+chapter+"/"+key+".html";
 	QString fileName=key+".html";
 	bool storeHelpDir=false;

 	if (m_instance)
 	 {
    while (!indexFile.isEmpty() && !QFile::exists(indexFile)) {
      indexFile = QFileDialog::getOpenFileName(m_parentWidget, "Help File", fileName, "*.html");
      storeHelpDir = true;
    }

    if (!indexFile.isEmpty() &&  storeHelpDir)
     {
     	char rootPath[1024],*ptr;

     	strcpy(rootPath,indexFile.toAscii().constData());
     	if ((ptr=strrchr(rootPath,'/'))!=NULL)
     	 *ptr='\0';
     	if (!chapter.isEmpty() && ((ptr=strrchr(rootPath,'/'))!=NULL))
     	 *ptr='\0';

      helpDir = QString(rootPath); // just the directory name

      CPreferences::instance()->setDirectoryName("Help", helpDir);

      fileName="file:///"+indexFile;
      QDesktopServices::openUrl(QUrl(fileName) );
     }
   }

  return storeHelpDir;
 }

void CHelpSystem::showHelpTopic(const QString &chapter, const QString &key)
{
 // PREVIOUS VERSION if (m_instance) {
 // PREVIOUS VERSION  m_instance->openBrowser(chapter, key);
 // PREVIOUS VERSION }

 #if defined(QDOAS_HELP_PATH)
  QString path=QDOAS_HELP_PATH;
 #else
  QString path=CPreferences::instance()->directoryName("Help", ".");
 #endif

 // Load the default browser

 char strPath[800],strUrl[800];
 bool pageFound=true;
 FILE *fp;

 strcpy(strPath,path.toAscii().constData());

 if (strlen(chapter.toAscii().constData()))
  sprintf(strUrl,"%s/%s/%s.html",strPath,chapter.toAscii().constData(),key.toAscii().constData());
 else
  sprintf(strUrl,"%s/%s.html",strPath,key.toAscii().constData());

 if ((fp=fopen(strUrl,"rt"))==NULL)
  pageFound=false;
 else
  fclose(fp);

 if (m_instance)
  m_instance->changeDir(chapter,key);

 if (pageFound)
  {
   QString fileName="file:///"+QString(strUrl);
   QDesktopServices::openUrl(QUrl(fileName) );
  }
}

CHelpSystem::CHelpSystem(QWidget *parent) :
  m_parentWidget(parent),
  m_impl(NULL),
  m_block(false),
  m_preferLightBrowser(false)
{
  // NOT NEEDED ANYMORE // the parent is needed for the impl... it MUST have a lifetime
  // NOT NEEDED ANYMORE // at least as long as this.
  // NOT NEEDED ANYMORE
  // NOT NEEDED ANYMORE // populate the map ...
  // NOT NEEDED ANYMORE
  // NOT NEEDED ANYMORE m_chapterMap.insert(std::map<QString,int>::value_type("Project", 1));
  // NOT NEEDED ANYMORE m_chapterMap.insert(std::map<QString,int>::value_type("Analysis", 2));
}

CHelpSystem::~CHelpSystem()
{
  m_instance = NULL;
  // shutdown the implementation
  delete m_impl;
}

void CHelpSystem::openBrowser(void)
{
 // NOT NEEDED ANYMORE createImplementation(&m_impl);
 // NOT NEEDED ANYMORE
 // NOT NEEDED ANYMORE if (m_impl)
 // NOT NEEDED ANYMORE   m_impl->display(HELP_HOME_PAGE); // the table of contents
}

void CHelpSystem::openBrowser(const QString &chapter, const QString &key)
{
// NOT NEEDED ANYMORE  createImplementation(&m_impl);
// NOT NEEDED ANYMORE
// NOT NEEDED ANYMORE  if (m_impl) {
// NOT NEEDED ANYMORE    QString url;
// NOT NEEDED ANYMORE    // resolve chapter
// NOT NEEDED ANYMORE    std::map<QString,int>::const_iterator it = m_chapterMap.find(chapter);
// NOT NEEDED ANYMORE
// NOT NEEDED ANYMORE    if (it != m_chapterMap.end()) {
// NOT NEEDED ANYMORE
// NOT NEEDED ANYMORE      QTextStream stream(&url);
// NOT NEEDED ANYMORE
// NOT NEEDED ANYMORE      stream << chapter << "/" << key << ".html";
// NOT NEEDED ANYMORE
// NOT NEEDED ANYMORE      //stream << "chapter" << (it->second) << "/chapter" << (it->second) << ".html";
// NOT NEEDED ANYMORE//      if (!key.isEmpty())
// NOT NEEDED ANYMORE//	stream << '#' << key;
// NOT NEEDED ANYMORE    }
// NOT NEEDED ANYMORE    else {
// NOT NEEDED ANYMORE      url = HELP_HOME_PAGE;
// NOT NEEDED ANYMORE    }
// NOT NEEDED ANYMORE
// NOT NEEDED ANYMORE//     QWebView *view = new QWebView(this);
// NOT NEEDED ANYMORE//     view->load(QUrl(url));
// NOT NEEDED ANYMORE//     view->show();
// NOT NEEDED ANYMORE
// NOT NEEDED ANYMORE    m_impl->display(url);
// NOT NEEDED ANYMORE  }
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


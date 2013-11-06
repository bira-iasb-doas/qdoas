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
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>

#include "CHelpSystem.h"
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
  
  return storeHelpDir;
}

void CHelpSystem::showHelpTopic(const QString &chapter, const QString &key)
{
  
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

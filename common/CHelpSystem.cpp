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
#include <QMessageBox>
#include <QUrl>
#include <QDesktopServices>

#include <stdlib.h>

#include "CHelpSystem.h"
#include "CPreferences.h"

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

// Ask the user to locate the Qdoas help files, and update the preferences if found.
QString CHelpSystem::changeDir(void)
{
  QString helpDir = CPreferences::instance()->directoryName("Help", ".");

  QMessageBox::information(m_parentWidget, "Help", "Qdoas can not find the Help directory."
                           "Please select the directory containing the main Qdoas help file"
                           "(index.html).");

  helpDir = QFileDialog::getExistingDirectory(m_parentWidget, "Qdoas Help directory", helpDir);

  if (!helpDir.isEmpty() && QFile::exists(helpDir + "/index.html") ) {
    CPreferences::instance()->setDirectoryName("Help", helpDir);
  } else {
    QMessageBox::warning(m_parentWidget, "Help", "Could not find Help files.");
  }
  return helpDir;
}

void CHelpSystem::showHelpTopic(const QString &chapter, const QString &key)
{
  QString relPath = "/" + chapter + "/" + key + ".html";

  // If we have the right path in the preferences, that is the first choice
  QString helpFile = CPreferences::instance()->directoryName("Help", ".")
    + relPath;

  // check for "QDOAS_HELP_DIR" environment variable, which is set if we are
  // running the linux package.
  if (!QFile::exists(helpFile)) {
  	 #if defined(WIN32)
    const char *helpDir = getenv("QDOAS_HELP_DIR");
    #else
    const char *helpDir = std::getenv("QDOAS_HELP_DIR");
    #endif
    if (helpDir != NULL) {
      helpFile = QDir(helpDir).absolutePath() + relPath;
    }
  }

#if defined(QDOAS_HELP_PATH)
  // If QDOAS_HELP_PATH was defined at compile time, try that:
  if (!QFile::exists(helpFile)) {
    helpFile = QString(QDOAS_HELP_PATH) + relPath;
  }
#endif

  // If nothing was found, ask the user to point to the Help directory
  if (!QFile::exists(helpFile)) {
    if(m_instance) {
      helpFile = m_instance->changeDir() + relPath;
    }
  }

  if (QFile::exists(helpFile)) {
    QDesktopServices::openUrl(QUrl("file:///" + helpFile)); // file: URI with three slashes for windows!
  }
}

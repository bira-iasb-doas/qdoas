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


#ifndef _CPREFERENCES_H_GUARD
#define _CPREFERENCES_H_GUARD

#include <QSettings>
#include <QSize>
#include <QString>

// WARNING - This class is NOT thread safe. ONLY use this in the main thread of the GUI.

class CPreferences
{
 public:
  static CPreferences* instance(void);
  
  ~CPreferences(); // flushes data to permanent storage

  // interface for saving/restoring preferences...

  QSize windowSize(const QString &key, const QSize &fallback = QSize()) const;
  void setWindowSize(const QString &key, const QSize &size);

  QString directoryName(const QString &key, const QString &fallback = QString()) const;
  void setDirectoryName(const QString &key, const QString &directory);
  void setDirectoryNameGivenFile(const QString &key, const QString &fileName);

  QString fileExtension(const QString &key, int index, const QString &fallback = QString()) const;
  void setFileExtension(const QString &key, int index, const QString &extension);
  void setFileExtensionGivenFile(const QString &key, int index, const QString &fileName);

  // for general stuff just get a handle to the settings
  QSettings& settings(void);

 private:
  // controlled creation and prevent NO copying
  CPreferences();
  CPreferences(const CPreferences &) {}
  CPreferences& operator=(const CPreferences &) { return *this; }

 private:
  static CPreferences *m_instance;
  QSettings *m_settings;
};

inline QSettings& CPreferences::settings(void) { return *m_settings; }

#endif

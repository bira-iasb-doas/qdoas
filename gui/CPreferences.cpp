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


#include <QCoreApplication>
#include <QTextStream>

#include "CPreferences.h"

// initialise static data

CPreferences *CPreferences::m_instance = NULL;
 
CPreferences* CPreferences::instance(void)
{
  if (m_instance == NULL)
    m_instance = new CPreferences;

  return m_instance;
}

CPreferences::~CPreferences()
{
  // flush data to permanent storage
  m_settings->sync();
  delete m_settings;

  m_instance = NULL;
}

CPreferences::CPreferences()
{
  QCoreApplication::setOrganizationDomain("www.oma.be");

  m_settings = new QSettings("BIRA-IASB", "Qdoas");
}

// interface for saving/restoring preferences...

QSize CPreferences::windowSize(const QString &key, const QSize &fallback) const
{
  m_settings->beginGroup("WindowSize");
  QSize tmp = m_settings->value(key, fallback).toSize();
  m_settings->endGroup();

  return tmp;
}

void CPreferences::setWindowSize(const QString &key, const QSize &size)
{
  m_settings->beginGroup("WindowSize");
  m_settings->setValue(key, size);
  m_settings->endGroup();
}

QString CPreferences::directoryName(const QString &key, const QString &fallback) const
{
  m_settings->beginGroup("Directory");
  QString tmp = m_settings->value(key, fallback).toString();
  m_settings->endGroup();

  return tmp;
}

void CPreferences::setDirectoryName(const QString &key, const QString &directory)
{
  m_settings->beginGroup("Directory");
  m_settings->setValue(key, directory);
  m_settings->endGroup();  
}

void CPreferences::setDirectoryNameGivenFile(const QString &key, const QString &fileName)
{
  // trim the file from the directory
  int index = fileName.lastIndexOf('/');
  if (index == -1) index = fileName.lastIndexOf('\\');
  if (index != -1) {
    setDirectoryName(key, fileName.left(index));
  }
}

QString CPreferences::fileExtension(const QString &key, int index, const QString &fallback) const
{
  QString tmp(key);
  QTextStream stream(&tmp);
  
  stream << index;

  m_settings->beginGroup("FileExtension");
  tmp = m_settings->value(tmp, fallback).toString();
  m_settings->endGroup();
  
  return tmp;
}

void CPreferences::setFileExtension(const QString &key, int index, const QString &extension)
{
  QString tmp(key);
  QTextStream stream(&tmp);
  
  stream << index;

  m_settings->beginGroup("FileExtension");
  m_settings->setValue(tmp, extension);
  m_settings->endGroup();
}
  
void CPreferences::setFileExtensionGivenFile(const QString &key, int index, const QString &fileName)
{
  int pos = fileName.lastIndexOf('.');
  if (pos == -1) {
    // default to all files ...
    setFileExtension(key, index, "*");
  }
  else {
    pos =  fileName.length() - pos - 1;
    if (pos > 0)
      setFileExtension(key, index, fileName.right(pos));
    else
      setFileExtension(key, index, "*");
  }
}

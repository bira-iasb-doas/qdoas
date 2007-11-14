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


#ifndef _CHELPSYSTEM_H_GUARD
#define _CHELPSYSTEM_H_GUARD

#include <map>

#include <QString>
#include <QWidget>

class CHelpImpl;

class CHelpSystem
{
 public:
  static CHelpSystem* establishHelpSystem(QWidget *parent);

  static void showHelpTopic(const QString &chapter, const QString &key = QString());

  ~CHelpSystem();

 public:
  void openBrowser(void);
  void openBrowser(const QString &chapter, const QString &key = QString());

  void closeBrowser(void);

  void preferLightBrowser(bool light);
  bool isLightBrowserPreferred(void) const;
  bool supportsQtAssistant() const;

 private:
  void createImplementation(CHelpImpl **impl);
  CHelpSystem(QWidget *parent);

 private:
  static CHelpSystem* m_instance;

  QWidget *m_parentWidget;
  CHelpImpl *m_impl;
  std::map<QString,int> m_chapterMap;
  bool m_block;
  bool m_preferLightBrowser;
};

inline bool CHelpSystem::isLightBrowserPreferred(void) const { return m_preferLightBrowser; }

#endif

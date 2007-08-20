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

#ifndef _CWORKSPACE_H_GUARD
#define _CWORKSPACE_H_GUARD

#include <map>
#include <list>
#include <set>
#include <QString>

#include "mediate.h"

class CSitesObserver;

struct SProjBucket
{
  mediate_project_t *project;
  std::map<QString,mediate_analysis_window_t*> window;

  // takes ownership of projectData
  SProjBucket(mediate_project_t *projectData) : project(projectData) {}
  // NOTE: shallow copy of dynamic memory. Use for std::map<>::insert operations ONLY.
  SProjBucket(const SProjBucket &c) : project(c.project), window(c.window) {}
};

struct SPathBucket
{
  int index;
  QString path;

  SPathBucket(int i, const QString &p) : index(i), path(p) {}
  // sort with longest path first ... if equal then string compare with operator<()
  bool operator<(const SPathBucket &rhs) const { return ((path.length() > rhs.path.length()) || (path.length() == rhs.path.length() && path < rhs.path)); }
};


class CWorkSpace
{
 public:
  static CWorkSpace* instance(void);

  ~CWorkSpace();

  mediate_project_t* findProject(const QString &projectName) const;
  mediate_analysis_window_t* findAnalysisWindow(const QString &projectName, const QString &windowName) const;

  const mediate_site_t* findSite(const QString &siteName);

  bool createProject(const QString &newProjectName);
  bool createAnalysisWindow(const QString &projectName, const QString &newWindowName);
  bool createSite(const QString &newSiteName, const QString &abbr, double longitude, double latitude, double altitude);

  bool renameProject(const QString &oldProjectName, const QString &newProjectName);
  bool renameAnalysisWindow(const QString &projectName, const QString &oldWindowName, const QString &newWindowName);

  bool modifySite(const QString &siteName, const QString &abbr, double longitude, double latitude, double altitude);
  mediate_site_t* siteList(int &listLength) const;

  bool destroyProject(const QString &projectName);
  bool destroyAnalysisWindow(const QString &projectName, const QString &newWindowName);
  bool destroySite(const QString &siteName);

  void removePath(int index);
  void addPath(int index, const QString &path);
  QString simplifyPath(const QString &name) const;

 private:
  // singleton => no copies permitted
  CWorkSpace() {}
  CWorkSpace(const CWorkSpace &) {}
  CWorkSpace& operator=(const CWorkSpace &) { return *this; }

  void attach(CSitesObserver *observer);
  void detach(CSitesObserver *observer);
  
  friend class CSitesObserver;

 private:
  static CWorkSpace *m_instance;

  std::map<QString,SProjBucket> m_projMap;
  std::set<SPathBucket> m_pathSet;
  std::map<QString,mediate_site_t*> m_siteMap;
  std::list<CSitesObserver*> m_sitesObserverList;
};

class CSitesObserver {
 public:
  CSitesObserver();   // attaches and detaches to the singleton during construction/destruction
  virtual ~CSitesObserver();

  virtual void updateNewSite(const QString &newSiteName);
  virtual void updateModifySite(const QString &siteName);
  virtual void updateDeleteSite(const QString &siteName);
};

#endif


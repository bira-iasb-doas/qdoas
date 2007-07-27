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
#include <QString>

#include "mediate.h"

struct SProjBucket
{
  mediate_project_t *project;
  std::map<QString,mediate_analysis_window_t*> window;

  // takes ownership of projectData
  SProjBucket(mediate_project_t *projectData) : project(projectData) {}
  // NOTE: shallow copy of dynamic memory. Use for std::map<>::insert operations ONLY.
  SProjBucket(const SProjBucket &c) : project(c.project), window(c.window) {}
};

class CWorkSpace
{
 public:
  static CWorkSpace* instance(void);

  ~CWorkSpace();

  mediate_project_t* findProject(const QString &projectName) const;
  mediate_analysis_window_t* findAnalysisWindow(const QString &projectName, const QString &windowName) const;

  bool createProject(const QString &newProjectName);
  bool createAnalysisWindow(const QString &projectName, const QString &newWindowName);

  bool renameProject(const QString &oldProjectName, const QString &newProjectName);
  bool renameAnalysisWindow(const QString &projectName, const QString &oldWindowName, const QString &newWindowName);

  bool destroyProject(const QString &newProjectName);
  bool destroyAnalysisWindow(const QString &projectName, const QString &newWindowName);

 private:
  // singleton => no copies permitted
  CWorkSpace() {}
  CWorkSpace(const CWorkSpace &) {}
  CWorkSpace& operator=(const CWorkSpace &) { return *this; }

 private:
  static CWorkSpace *m_instance;

  std::map<QString,SProjBucket> m_projMap;
};

#endif


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

#ifndef _CSESSION_H_GUARD
#define _CSESSION_H_GUARD

// A ata bucket for holding a snap-shot of the state
// requirede for a browse or analysis session
#include <map>

#include <QList>
#include <QStringList>
#include <QFileInfo>

#include "RefCountPtr.h"

#include "mediate_project.h"
#include "mediate_analysis_window.h"

class CSession;
class CSessionIterator;

class CSessionItem
{
 public:
  CSessionItem(const mediate_project_t *project);
  ~CSessionItem();

  void addFile(const QFileInfo &file);

 private:
  mediate_project_t m_project;
  QList<mediate_analysis_window_t> m_windows;
  QList<QFileInfo> m_files;

  friend class CSession;
  friend class CSessionIterator;
};

class CSession
{
 public:
  CSession(bool forAnalysis = false);
  ~CSession();

  void addFile(const QFileInfo &file, const QString &projectName);

  int size(void) const;
  QStringList fileList(void) const;

 private:
  CSession(const CSession &);
  CSession& operator=(const CSession&);

 private:
  typedef std::map<QString,CSessionItem*> sessionmap_t;

  sessionmap_t m_map;

  bool m_forAnalysis; // indicates if analysis windows are required ...

  friend class CSessionIterator;
};

class CSessionIterator
{
 public:
  CSessionIterator();
  CSessionIterator(const RefCountConstPtr<CSession> &session);
  CSessionIterator(const CSessionIterator &other);
  ~CSessionIterator();
  
  CSessionIterator& operator=(const CSessionIterator &rhs);

  CSessionIterator& operator++(void);
  CSessionIterator& operator--(void);

  CSessionIterator& operator()(int index);

  int index(void) const;

  bool atEnd(void) const;
  bool atBegin(void) const;

  const QFileInfo& file(void) const;
  const mediate_project_t* project(void) const;

  bool operator==(const CSessionIterator &rhs) const;
  bool operator!=(const CSessionIterator &rhs) const;

 private:
  RefCountConstPtr<CSession> m_session;
  CSession::sessionmap_t::const_iterator m_mapIt;
  int m_fileIndex;
  int m_offset;
};

#endif

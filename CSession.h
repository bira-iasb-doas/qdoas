#ifndef _CSESSION_H_GUARD
#define _CSESSION_H_GUARD

// A ata bucket for holding a snap-shot of the state
// requirede for a browse or analysis session
#include <map>

#include <QList>
#include <QFileInfo>

#include "RefCountPtr.h"

#include "mediate_project.h"
#include "mediate_analysis_window.h"

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

  friend class CSessionIterator;
};

class CSession
{
 public:
  CSession(bool forAnalysis = false);
  ~CSession();

  void addFile(const QFileInfo &file, const QString &projectName);

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
  CSessionIterator(RefCountPtr<CSession> session);
  CSessionIterator(const CSessionIterator &other);
  ~CSessionIterator();
  
  CSessionIterator& operator=(const CSessionIterator &rhs);

  CSessionIterator& operator++(void);

  bool atEnd(void) const;

  const QFileInfo& file(void) const;
  const mediate_project_t* project(void) const;

  bool operator==(const CSessionIterator &rhs) const;
  bool operator!=(const CSessionIterator &rhs) const;

 private:
  RefCountPtr<CSession> m_session;
  CSession::sessionmap_t::iterator m_mapIt;
  int m_fileIndex;
};

#endif

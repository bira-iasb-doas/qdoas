#ifndef _CSESSION_H_GUARD
#define _CSESSION_H_GUARD

// A ata bucket for holding a snap-shot of the state
// requirede for a browse or analysis session
#include <map>

#include <QList>
#include <QFileInfo>

#include "mediate_project.h"
#include "mediate_analysis_window.h"

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
};

#endif

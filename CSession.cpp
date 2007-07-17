#include <assert.h>

#include "CSession.h"

#include "CWorkSpace.h"

CSessionItem::CSessionItem(const mediate_project_t *project)
{
  // copy the project data
  m_project = *project;
}

CSessionItem::~CSessionItem()
{
}

void CSessionItem::addFile(const QFileInfo &file)
{
  m_files.push_back(file);
}

CSession::CSession(bool forAnalysis) :
  m_forAnalysis(forAnalysis)
{

}

CSession::~CSession()
{
  sessionmap_t::iterator it = m_map.begin();
  while (it != m_map.end()) {
    delete it->second;
    ++it;
  }
  m_map.clear();
}

void CSession::addFile(const QFileInfo &file, const QString &projectName)
{
  sessionmap_t::iterator it = m_map.find(projectName);
  
  if (it == m_map.end()) {
    // dont have this project yet ... grab it from the workspace
    mediate_project_t *proj = CWorkSpace::instance()->findProject(projectName);
    if (proj) {
      CSessionItem *item = new CSessionItem(proj);
      item->addFile(file);

      if (m_forAnalysis) {
	assert(false); // TODO also get the analysis windows ...
      }
 
      m_map.insert(sessionmap_t::value_type(projectName,item));
    }
    else {
      // this is very bad ...
      assert(false);
    }
  }
  else {
    // exists
    (it->second)->addFile(file);
  }
}


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

int CSession::size(void) const
{
  int count = 0;

  // get the number of files in the session
  sessionmap_t::const_iterator it = m_map.begin();
  while (it != m_map.end()) {
    count += (it->second)->m_files.size();
    ++it;
  }
      
  return count;
}


CSessionIterator::CSessionIterator() :
  m_fileIndex(0),
  m_offset(0)
{
}

CSessionIterator::CSessionIterator(RefCountPtr<CSession> session) :
  m_session(session)
{
  if (m_session != 0) {
    m_mapIt = m_session->m_map.begin();
    m_fileIndex = 0;
    m_offset = 0;
  }
}

CSessionIterator::CSessionIterator(const CSessionIterator &other) :
  m_session(other.m_session),
  m_mapIt(other.m_mapIt),
  m_fileIndex(other.m_fileIndex),
  m_offset(other.m_offset)
{
}

CSessionIterator::~CSessionIterator()
{
}

CSessionIterator& CSessionIterator::operator=(const CSessionIterator &rhs)
{
  m_session = rhs.m_session;
  m_mapIt = rhs.m_mapIt;
  m_fileIndex = rhs.m_fileIndex;
  m_offset = rhs.m_offset;

  return *this;
}

CSessionIterator& CSessionIterator::operator++(void)
{
  // no range checks here - use atEnd first ...

  if (++m_fileIndex == (m_mapIt->second)->m_files.size()) {
    m_offset += m_fileIndex;
    ++m_mapIt;
    m_fileIndex = 0;
  }

  return *this;
}

CSessionIterator& CSessionIterator::operator--(void)
{
  // no range checks here - use atBegin first ...

  if (--m_fileIndex < 0) {
    --m_mapIt;
    m_fileIndex = (m_mapIt->second)->m_files.size();
    m_offset -= m_fileIndex;
    --m_fileIndex;
  }

  return *this;
}

// specific positioning of the iterator

CSessionIterator& CSessionIterator::operator()(int index)
{
  // no range checks
  m_offset = 0;
  m_fileIndex = index;
  m_mapIt = m_session->m_map.begin();
  while (m_mapIt != m_session->m_map.end() &&
	 m_fileIndex >= (m_mapIt->second)->m_files.size()) {
    m_offset += (m_mapIt->second)->m_files.size();
    m_fileIndex = index - m_offset;
    ++m_mapIt;
  }

  return *this;
}

int CSessionIterator::index(void) const
{
  return m_offset + m_fileIndex;
}

bool CSessionIterator::atEnd(void) const
{
  return (m_session == 0 || m_mapIt == m_session->m_map.end() || m_fileIndex == (m_mapIt->second)->m_files.size());
}

bool CSessionIterator::atBegin(void) const
{
  // NOTE returns false if session is null

  return (m_session != 0 && m_mapIt == m_session->m_map.begin() && m_fileIndex == 0);
}

const QFileInfo& CSessionIterator::file(void) const
{
  return (m_mapIt->second)->m_files.at(m_fileIndex);
}

const mediate_project_t* CSessionIterator::project(void) const
{
  return &((m_mapIt->second)->m_project);
}

bool CSessionIterator::operator==(const CSessionIterator &rhs) const
{
  return (m_session == rhs.m_session && m_mapIt == rhs.m_mapIt && m_fileIndex == rhs.m_fileIndex);
}

bool CSessionIterator::operator!=(const CSessionIterator &rhs) const
{
  return !operator==(rhs);
}


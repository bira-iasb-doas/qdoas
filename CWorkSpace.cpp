
#include "CWorkSpace.h"

CWorkSpace *CWorkSpace::m_instance = NULL;

CWorkSpace* CWorkSpace::instance(void)
{
  if (m_instance == NULL)
    m_instance = new CWorkSpace;

  return m_instance;
}

CWorkSpace::~CWorkSpace()
{
  // free all dynamic memory
  
  // Project ... 
  std::map<QString,SProjBucket>::iterator pIt = m_projMap.begin();
  while (pIt != m_projMap.end()) {    
    std::map<QString,mediate_analysis_window_t*>::iterator wIt = (pIt->second).window.begin();
    while (wIt != (pIt->second).window.end()) {
      delete wIt->second; // delete the analysis window data
      ++wIt;
    }
    delete (pIt->second).project; // delete the project data
    ++pIt;
  }
  m_projMap.clear(); // happens anyway

  m_instance = NULL;
}

mediate_project_t* CWorkSpace::findProject(const QString &projectName) const
{
  std::map<QString,SProjBucket>::const_iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end())
    return (pIt->second).project;

  return NULL; // not found
}

mediate_analysis_window_t* CWorkSpace::findAnalysisWindow(const QString &projectName,
							  const QString &windowName) const
{
  std::map<QString,SProjBucket>::const_iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    // project exists
    std::map<QString,mediate_analysis_window_t*>::const_iterator wIt = (pIt->second).window.find(windowName);
    if (wIt != (pIt->second).window.end())
      return wIt->second;
  }
  
  return NULL; // not found
}

bool CWorkSpace::createProject(const QString &newProjectName)
{
  // does it already exist?
  std::map<QString,SProjBucket>::iterator pIt = m_projMap.find(newProjectName);
  if (pIt == m_projMap.end()) {
    // does not exist
    mediate_project_t *tmp = new mediate_project_t;
    // initialise the project data ... TODO
    
    // insert the project (with no windows)
    m_projMap.insert(std::map<QString,SProjBucket>::value_type(newProjectName,SProjBucket(tmp)));
    return true;
  }

  return false;
}

bool CWorkSpace::createAnalysisWindow(const QString &projectName, const QString &newWindowName)
{
  // project must exist
  std::map<QString,SProjBucket>::iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    std::map<QString,mediate_analysis_window_t*>::iterator wIt = (pIt->second).window.find(newWindowName);
    if (wIt == (pIt->second).window.end()) {
      // analysis window does not already exist
      mediate_analysis_window_t *tmp = new mediate_analysis_window_t;
      // initialise the analysis window data ... TODO

      // insert the window into the map
      (pIt->second).window.insert(std::map<QString,mediate_analysis_window_t*>::value_type(newWindowName,tmp));
      return true;
    }
  }
  return false;
}

bool CWorkSpace::renameProject(const QString &oldProjectName, const QString &newProjectName)
{
  // project must exist - locate by old name
  std::map<QString,SProjBucket>::iterator oldIt = m_projMap.find(oldProjectName);
  if (oldIt != m_projMap.end()) {
    // no change is OK
    if (oldProjectName == newProjectName)
      return true;

    // check that the new name is not in use
    std::map<QString,SProjBucket>::iterator newIt = m_projMap.find(newProjectName);
    if (newIt == m_projMap.end()) {
      // ok to rename - change of key so insert for the new key then remove the old entry
      m_projMap.insert(std::map<QString,SProjBucket>::value_type(newProjectName, oldIt->second));
      m_projMap.erase(oldIt);
      return true;
    }
  }

  return false;
}

bool CWorkSpace::renameAnalysisWindow(const QString &projectName, const QString &oldWindowName,
				      const QString &newWindowName)
{
  // project must exist
  std::map<QString,SProjBucket>::iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    // locate the window by the old name - must exist
    std::map<QString,mediate_analysis_window_t*>::iterator oldIt = (pIt->second).window.find(oldWindowName);
    if (oldIt != (pIt->second).window.end()) {
      // no change in the name is ok
      if (oldWindowName == newWindowName)
	return true;

      std::map<QString,mediate_analysis_window_t*>::iterator newIt = (pIt->second).window.find(newWindowName);
      if (newIt == (pIt->second).window.end()) {
	// ok to rename - change of key so insert for the new key then remove the old entry
	(pIt->second).window.insert(std::map<QString,mediate_analysis_window_t*>::value_type(newWindowName, oldIt->second));
	(pIt->second).window.erase(oldIt);
	return true;
      }
    }
  }
  return false;
}

bool CWorkSpace::destroyProject(const QString &projectName)
{
  // project must exist
  std::map<QString,SProjBucket>::iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    // delete all analysis windows ...
    std::map<QString,mediate_analysis_window_t*>::iterator wIt = (pIt->second).window.begin();
    while (wIt != (pIt->second).window.end()) {
      delete wIt->second;
      ++wIt;
    }
    (pIt->second).window.clear();
    delete (pIt->second).project;

    m_projMap.erase(pIt);
    return true;
  }
  return false;
}

bool CWorkSpace::destroyAnalysisWindow(const QString &projectName, const QString &windowName)
{
  // project must exist
  std::map<QString,SProjBucket>::iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    std::map<QString,mediate_analysis_window_t*>::iterator wIt = (pIt->second).window.find(windowName);
    if (wIt != (pIt->second).window.end()) {
      delete wIt->second;
      (pIt->second).window.erase(wIt);
      return true;
    }
  }
  return false;
}


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


#include <QTextStream>

#include "CWorkSpace.h"

#include "debugutil.h"

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

  // site
  std::map<QString,mediate_site_t*>::iterator sIt = m_siteMap.begin();
  while (sIt != m_siteMap.end()) {
    delete sIt->second;
    ++sIt;
  }
  m_siteMap.clear();

  m_instance = NULL;
}

void CWorkSpace::removeAllContent(void)
{
  // Projects ...  Dont worry about reference count handling because
  // the symbol list will be cleared as well.

  std::map<QString,SProjBucket>::iterator pIt = m_projMap.begin();
  while (pIt != m_projMap.end()) {    
    // notify the observers
    std::list<CProjectObserver*>::iterator obs = m_projectObserverList.begin();
    while (obs != m_projectObserverList.end()) {
      (*obs)->updateDeleteProject(pIt->first);
      ++obs;
    }

    std::map<QString,mediate_analysis_window_t*>::iterator wIt = (pIt->second).window.begin();
    while (wIt != (pIt->second).window.end()) {
      delete wIt->second; // delete the analysis window data
      ++wIt;
    }
    delete (pIt->second).project; // delete the project data
    ++pIt;
  }
  m_projMap.clear();

  // sites
  std::map<QString,mediate_site_t*>::iterator sIt = m_siteMap.begin();
  while (sIt != m_siteMap.end()) {
    // notify the observers
    std::list<CSitesObserver*>::iterator obs = m_sitesObserverList.begin();
    while (obs != m_sitesObserverList.end()) {
      (*obs)->updateDeleteSite(sIt->first);
      ++obs;
    }

    delete sIt->second;
    ++sIt;
  }
  m_siteMap.clear();

  // symbols
  std::map<QString,SSymbolBucket>::iterator mIt = m_symbolMap.begin();
  while (mIt != m_symbolMap.end()) {
    // notify the observers
    std::list<CSymbolObserver*>::iterator obs = m_symbolObserverList.begin();
    while (obs != m_symbolObserverList.end()) {
      (*obs)->updateDeleteSymbol(mIt->first);
      ++obs;
    }
    ++mIt;
  }
  m_symbolMap.clear();

  m_pathSet.clear();
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

const mediate_site_t* CWorkSpace::findSite(const QString &siteName) const
{
  std::map<QString,mediate_site_t*>::const_iterator it = m_siteMap.find(siteName);
  if (it != m_siteMap.end()) {
    return it->second;
  }

  return NULL; // not found
}

QString CWorkSpace::findSymbol(const QString &symbolName) const
{
  std::map<QString,SSymbolBucket>::const_iterator it = m_symbolMap.find(symbolName);
  if (it != m_symbolMap.end()) {
    // symbol exists
    return (it->second).description; // The string returned may be empty, but is not null.
  }

  return QString(); // a NULL string
}

void CWorkSpace::incrementUseCount(const QString &symbolName)
{
  std::map<QString,SSymbolBucket>::iterator it = m_symbolMap.find(symbolName);
  if (it != m_symbolMap.end())
    ++((it->second).useCount);
}

void CWorkSpace::decrementUseCount(const QString &symbolName)
{
  std::map<QString,SSymbolBucket>::iterator it = m_symbolMap.find(symbolName);
  if (it != m_symbolMap.end() && (it->second).useCount)
    --((it->second).useCount);
}

bool CWorkSpace::createProject(const QString &newProjectName)
{
  if (newProjectName.isEmpty())
    return false;

  // does it already exist?
  std::map<QString,SProjBucket>::iterator pIt = m_projMap.find(newProjectName);
  if (pIt == m_projMap.end()) {
    // does not exist
    mediate_project_t *tmp = new mediate_project_t;

    initializeMediateProject(tmp);
    
    // insert the project (with no windows)
    m_projMap.insert(std::map<QString,SProjBucket>::value_type(newProjectName,SProjBucket(tmp)));

    // notify the observers
    std::list<CProjectObserver*>::iterator obs = m_projectObserverList.begin();
    while (obs != m_projectObserverList.end()) {
      (*obs)->updateNewProject(newProjectName);
      ++obs;
    }

    return true;
  }

  return false;
}

bool CWorkSpace::createAnalysisWindow(const QString &projectName, const QString &newWindowName)
{
  if (newWindowName.isEmpty())
    return false;

  // project must exist
  std::map<QString,SProjBucket>::iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    std::map<QString,mediate_analysis_window_t*>::iterator wIt = (pIt->second).window.find(newWindowName);
    if (wIt == (pIt->second).window.end()) {
      // analysis window does not already exist
      mediate_analysis_window_t *tmp = new mediate_analysis_window_t;

      initializeMediateAnalysisWindow(tmp);

      // insert the window into the map
      (pIt->second).window.insert(std::map<QString,mediate_analysis_window_t*>::value_type(newWindowName,tmp));
      return true;
    }
  }
  return false;
}

bool CWorkSpace::createSite(const QString &newSiteName, const QString &abbr,
			    double longitude, double latitude, double altitude)
{
  mediate_site_t *tmp;

  // limit check ...
  if (newSiteName.isEmpty() || newSiteName.length() >= (int)sizeof(tmp->name) || abbr.length() >= (int)sizeof(tmp->abbreviation))
    return false;

  std::map<QString,mediate_site_t*>::iterator it = m_siteMap.find(newSiteName);
  if (it == m_siteMap.end()) {
    tmp = new mediate_site_t;

    strcpy(tmp->name, newSiteName.toAscii().data());
    strcpy(tmp->abbreviation, abbr.toAscii().data());
    tmp->longitude = longitude;
    tmp->latitude = latitude;
    tmp->altitude = altitude;
    
    m_siteMap.insert(std::map<QString,mediate_site_t*>::value_type(newSiteName, tmp));

    // notify the observers
    std::list<CSitesObserver*>::iterator obs = m_sitesObserverList.begin();
    while (obs != m_sitesObserverList.end()) {
      (*obs)->updateNewSite(newSiteName);
      ++obs;
    }
    return true;
  }
  return false;
}

bool CWorkSpace::createSymbol(const QString &newSymbolName, const QString &description)
{
  mediate_symbol_t *tmp; // just for size checking ....

  if (newSymbolName.isEmpty() || newSymbolName.length() >= (int)sizeof(tmp->name) || description.length() >= (int)sizeof(tmp->description))
    return false;

  std::map<QString,SSymbolBucket>::iterator it = m_symbolMap.find(newSymbolName);
  if (it == m_symbolMap.end()) {
    if (description.isNull()) {
      // make an empty description
      QString emptyStr = "";
      m_symbolMap.insert(std::map<QString,SSymbolBucket>::value_type(newSymbolName, SSymbolBucket(emptyStr)));
    }
    else
      m_symbolMap.insert(std::map<QString,SSymbolBucket>::value_type(newSymbolName, SSymbolBucket(description)));

    // notify the observers
    std::list<CSymbolObserver*>::iterator obs = m_symbolObserverList.begin();
    while (obs != m_symbolObserverList.end()) {
      (*obs)->updateNewSymbol(newSymbolName);
      ++obs;
    }    
    return true;
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
      // ok to rename - first notify, then touch the map, then notify again

      // notify the observers of a delete
      std::list<CProjectObserver*>::iterator obs = m_projectObserverList.begin();
      while (obs != m_projectObserverList.end()) {
	(*obs)->updateDeleteProject(oldProjectName);
	++obs;
      }

      // change of key so insert for the new key then remove the old entry
      m_projMap.insert(std::map<QString,SProjBucket>::value_type(newProjectName, oldIt->second));
      m_projMap.erase(oldIt);

      // notify the observers again
      obs = m_projectObserverList.begin();
      while (obs != m_projectObserverList.end()) {
	(*obs)->updateNewProject(newProjectName);
	++obs;
      }

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

bool CWorkSpace::modifySite(const QString &siteName, const QString &abbr,
			    double longitude, double latitude, double altitude)
{  
  mediate_site_t *tmp;

  // limit check ...
  if (siteName.length() >= (int)sizeof(tmp->name) || abbr.length() >= (int)sizeof(tmp->abbreviation))
    return false;

  std::map<QString,mediate_site_t*>::iterator it = m_siteMap.find(siteName);
  if (it != m_siteMap.end()) {

    tmp = it->second;

    // cant change the name
    strcpy(tmp->abbreviation, abbr.toAscii().data());
    tmp->longitude = longitude;
    tmp->latitude = latitude;
    tmp->altitude = altitude;
    
    // notify the observers
    std::list<CSitesObserver*>::iterator obs = m_sitesObserverList.begin();
    while (obs != m_sitesObserverList.end()) {
      (*obs)->updateModifySite(siteName);
      ++obs;
    }

    return true;
  }
  return false;
}

bool CWorkSpace::modifySymbol(const QString &symbolName, const QString &description)
{
  std::map<QString,SSymbolBucket>::iterator it = m_symbolMap.find(symbolName);
  if (it != m_symbolMap.end()) {
    // symbol exists - and should
    if (description.isNull())
      (it->second).description = ""; // empty
    else
      (it->second).description = description;

    // notify the observers
    std::list<CSymbolObserver*>::iterator obs = m_symbolObserverList.begin();
    while (obs != m_symbolObserverList.end()) {
      (*obs)->updateModifySymbol(symbolName);
      ++obs;
    }    

    return true;
  }

  return false;
}

void CWorkSpace::modifiedProjectProperties(const QString &projectName)
{
  // notify the observers of the modification
  std::list<CProjectObserver*>::iterator obs = m_projectObserverList.begin();
  while (obs != m_projectObserverList.end()) {
    (*obs)->updateModifyProject(projectName);
    ++obs;
  }
}

// data returned must be freed by the client with 'operator delete []'

mediate_site_t* CWorkSpace::siteList(int &listLength) const
{
  size_t n = m_siteMap.size();
  
  if (n > 0) {
    mediate_site_t *siteList = new mediate_site_t[n];
    mediate_site_t *tmp = siteList;
    
    // walk the list and copy
    std::map<QString,mediate_site_t*>::const_iterator it = m_siteMap.begin();
    while (it != m_siteMap.end()) {
      *tmp = *(it->second);
      ++tmp;
      ++it;
    }
    
    listLength = (int)n;
    
    return siteList;
  }
  
  listLength = 0;
  return NULL;
}

// data returned must be freed by the client with 'operator delete []'

mediate_symbol_t* CWorkSpace::symbolList(int &listLength) const
{
  size_t n = m_symbolMap.size();
  
  if (n > 0) {
    mediate_symbol_t *symbolList = new mediate_symbol_t[n];
    mediate_symbol_t *tmp = symbolList;
    
    // walk the list and copy
    std::map<QString,SSymbolBucket>::const_iterator it = m_symbolMap.begin();
    while (it != m_symbolMap.end()) {
      strcpy(tmp->name, (it->first).toAscii().data());
      strcpy(tmp->description, (it->second).description.toAscii().data());
      ++tmp;
      ++it;
    }
    
    listLength = (int)n;
    
    return symbolList;
  }
  
  listLength = 0;
  return NULL;
}

mediate_analysis_window_t* CWorkSpace::analysisWindowList(const QString &projectName, int &listLength) const
{ 
  std::map<QString,SProjBucket>::const_iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {
    // project exists
    size_t n = (pIt->second).window.size();
    if (n > 0) {
      mediate_analysis_window_t *data = new mediate_analysis_window_t[n];
      mediate_analysis_window_t *p = data;

      // walk the map and copy ... TODO - is the order important ???
      std::map<QString,mediate_analysis_window_t*>::const_iterator wIt = (pIt->second).window.begin();
      while (wIt != (pIt->second).window.end()) {
	*p = *(wIt->second); // blot copy
	++wIt;
      }
      listLength = (int)n;
      return data;
    }
  }
  
  listLength = 0;
  return NULL;
}

QStringList CWorkSpace::symbolList(void) const
{
  QStringList symbolList;

  std::map<QString,SSymbolBucket>::const_iterator it = m_symbolMap.begin();
  while (it != m_symbolMap.end()) {
    symbolList << (it->first);
    ++it;
  }

  return symbolList;
}

bool CWorkSpace::destroyProject(const QString &projectName)
{
  // project must exist
  std::map<QString,SProjBucket>::iterator pIt = m_projMap.find(projectName);
  if (pIt != m_projMap.end()) {

    // notify the observers
    std::list<CProjectObserver*>::iterator obs = m_projectObserverList.begin();
    while (obs != m_projectObserverList.end()) {
      (*obs)->updateDeleteProject(projectName);
      ++obs;
    }

    // delete all analysis windows ...
    std::map<QString,mediate_analysis_window_t*>::iterator wIt = (pIt->second).window.begin();
    while (wIt != (pIt->second).window.end()) {      
      // update the useCount for symbols
      for (int i=0; i < (wIt->second)->crossSectionList.nCrossSection; ++i)
	decrementUseCount((wIt->second)->crossSectionList.crossSection[i].symbol);

      delete wIt->second;
      ++wIt;
    }

    // update the useCount for symbols
    for (int i=0; i < (pIt->second).project->calibration.crossSectionList.nCrossSection; ++i)
      decrementUseCount((pIt->second).project->calibration.crossSectionList.crossSection[i].symbol);

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

      // update the useCount for symbols
      for (int i=0; i < (wIt->second)->crossSectionList.nCrossSection; ++i)
	decrementUseCount((wIt->second)->crossSectionList.crossSection[i].symbol);

      delete wIt->second;
      (pIt->second).window.erase(wIt);
      return true;
    }
  }
  return false;
}

bool CWorkSpace::destroySite(const QString &siteName)
{
  std::map<QString,mediate_site_t*>::iterator it = m_siteMap.find(siteName);
  if (it != m_siteMap.end()) {

    // notify the observers
    std::list<CSitesObserver*>::iterator obs = m_sitesObserverList.begin();
    while (obs != m_sitesObserverList.end()) {
      (*obs)->updateDeleteSite(siteName);
      ++obs;
    }

    delete it->second;
    m_siteMap.erase(it);
    return true;
  }
  return false;
}

bool CWorkSpace::destroySymbol(const QString &symbolName)
{
  std::map<QString,SSymbolBucket>::iterator it = m_symbolMap.find(symbolName);
  if (it != m_symbolMap.end()) {
    // Only permitted if the useCount is zero ...
    if ((it->second).useCount == 0) {
      // notify the observers
      std::list<CSymbolObserver*>::iterator obs = m_symbolObserverList.begin();
      while (obs != m_symbolObserverList.end()) {
	(*obs)->updateDeleteSymbol(symbolName);
	++obs;
      }
      m_symbolMap.erase(it);
      return true;
    }
  }
  return false;
}

void CWorkSpace::removePath(int index)
{
  // locate by index - walk the set

  std::set<SPathBucket>::iterator it = m_pathSet.begin();
  while (it != m_pathSet.end()) {
    if (it->index == index) {
      m_pathSet.erase(it);
      break;
    }
    ++it;
  }
}

void CWorkSpace::addPath(int index, const QString &path)
{
  // DO NOT allow empty paths to be stored.
  // DO NOT allow duplicate indexes

  // check for this index - just remove it if it exists
  std::set<SPathBucket>::iterator it = m_pathSet.begin();
  while (it != m_pathSet.end()) {
    if (it->index == index) {
      m_pathSet.erase(it);
      break;
    }
    ++it;
  }

  if (path.isEmpty())
    return;

  m_pathSet.insert(SPathBucket(index, path));
}

QString CWorkSpace::simplifyPath(const QString &name) const
{
  // walk in search of a matching path. set is sorted so that
  // a longer path will match first.

  std::set<SPathBucket>::const_iterator it = m_pathSet.begin();
  while (it != m_pathSet.end()) {
    
    if (name.startsWith(it->path)) {
      // matches ... but must match either perfectly or on a path separator boundary
      int len = it->path.length();  // certain that name.length() > path.length()
      if (name.length() == len) {
	// one-to-one match
	QString tmp;
	QTextStream stream(&tmp);
	
	stream << '%' << it->index;

	return tmp;
      }
      else if (name.at(len) == '/' || name.at(len) == '\\') {
	// matches on separator boundary
	QString tmp;
	QTextStream stream(&tmp);
	
	stream << '%' << it->index << name.right(name.length() - len);

	return tmp;
	
      }
    }
    ++it;
  }
  // no matches
  return name;
}

QString CWorkSpace::path(int index) const
{
  std::set<SPathBucket>::const_iterator it = m_pathSet.begin();
  while (it != m_pathSet.end() && it->index != index) ++it;
  if (it != m_pathSet.end())
    return it->path;

  return QString();
}

void CWorkSpace::attach(CSitesObserver *observer)
{
  if (std::find(m_sitesObserverList.begin(), m_sitesObserverList.end(), observer) == m_sitesObserverList.end())
    m_sitesObserverList.push_back(observer);
}

void CWorkSpace::detach(CSitesObserver *observer)
{
  m_sitesObserverList.remove(observer);
}

void CWorkSpace::attach(CSymbolObserver *observer)
{
  if (std::find(m_symbolObserverList.begin(), m_symbolObserverList.end(), observer) == m_symbolObserverList.end())
    m_symbolObserverList.push_back(observer);
}

void CWorkSpace::detach(CSymbolObserver *observer)
{
  m_symbolObserverList.remove(observer);
}

void CWorkSpace::attach(CProjectObserver *observer)
{
  if (std::find(m_projectObserverList.begin(), m_projectObserverList.end(), observer) == m_projectObserverList.end())
    m_projectObserverList.push_back(observer);
}
void CWorkSpace::detach(CProjectObserver *observer)
{
  m_projectObserverList.remove(observer);
}

//-----------------------------------------------------------------------

CSitesObserver::CSitesObserver()
{
  CWorkSpace::instance()->attach(this);
}

CSitesObserver::~CSitesObserver()
{
  CWorkSpace::instance()->detach(this);
}

void CSitesObserver::updateNewSite(const QString &newSiteName)
{
}

void CSitesObserver::updateModifySite(const QString &siteName)
{
}

void CSitesObserver::updateDeleteSite(const QString &siteName)
{
}

//-----------------------------------------------------------------------

CSymbolObserver::CSymbolObserver()
{
  CWorkSpace::instance()->attach(this);
}

CSymbolObserver::~CSymbolObserver()
{
  CWorkSpace::instance()->detach(this);
}

void CSymbolObserver::updateNewSymbol(const QString &newSymbolName)
{
}

void CSymbolObserver::updateModifySymbol(const QString &symbolName)
{
}

void CSymbolObserver::updateDeleteSymbol(const QString &symbolName)
{
}

//-----------------------------------------------------------------------

CProjectObserver::CProjectObserver()
{
  CWorkSpace::instance()->attach(this);
}

CProjectObserver::~CProjectObserver()
{
  CWorkSpace::instance()->detach(this);
}

void CProjectObserver::updateNewProject(const QString &newProjectName)
{
}

void CProjectObserver::updateModifyProject(const QString &projectName)
{
}

void CProjectObserver::updateDeleteProject(const QString &projectName)
{
}


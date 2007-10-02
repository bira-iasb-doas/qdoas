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

#include "CEngineController.h"
#include "CEngineRequest.h"
#include "CEngineResponse.h"

#include "debugutil.h"

static const int cBrowseMode   = 0x1;
static const int cAnalyseMode  = 0x2;
static const int cRequestBit = 0x8;


CEngineController::CEngineController(QObject *parent) :
  QObject(parent),
  m_mode(0),
  m_currentProject(NULL),
  m_currentRecord(-1),
  m_numberOfRecords(0),
  m_numberOfFiles(0)
{

  // create the engine thread
  m_thread = new CEngineThread(this);

  m_thread->setRunState(true);
}

CEngineController::~CEngineController()
{
}

void CEngineController::notifyNumberOfFiles(int nFiles)
{
  
}

void CEngineController::notifyCurrentFile(int fileNumber)
{
}

void CEngineController::notifyReadyToNavigateRecords(const QString &filename, int numberOfRecords)
{
  // successfully started browsing a file
  // TODO signals for buttons

  // sanity check - currentIt and filename must be consistent
  assert(m_currentIt.file().filePath() == filename);

  m_mode &= ~cRequestBit; // clear the request bit 

  m_numberOfRecords = numberOfRecords;
  m_currentRecord = 0;

  // signals for navigation control
  emit signalCurrentFileChanged(m_currentIt.index(), m_numberOfRecords);
  // files
  emit signalCurrentFileChanged(filename);

  // session is up and running
  emit signalSessionRunning(isSessionRunning());
}

void CEngineController::notifyCurrentRecord(int recordNumber)
{
  m_currentRecord = recordNumber;
  
  emit signalCurrentRecordChanged(m_currentRecord);
}

void CEngineController::notifyEndOfRecords(void)
{
  m_currentRecord = m_numberOfRecords + 1;
  
  emit signalCurrentRecordChanged(m_currentRecord);
}

void CEngineController::notifyPlotData(QList<SPlotData> &plotDataList, QList<STitleTag> &titleList)
{
  // the controller takes the data in plotDataList and titleList
  // and organises the data-sets into a set of pages. Each page is
  // then (safely) dispatched.

  std::map<int,CPlotPageData*> pageMap;
  std::map<int,CPlotPageData*>::iterator mIt;
  int pageNo;

  while (!plotDataList.isEmpty()) {
    // existing page?
    pageNo = plotDataList.front().page;
    mIt = pageMap.find(pageNo);
    if (mIt == pageMap.end()) {
      // need a new page
      CPlotPageData *newPage = new CPlotPageData(pageNo);
      newPage->addPlotDataSet(plotDataList.front().data);
      pageMap.insert(std::map<int,CPlotPageData*>::value_type(pageNo, newPage));
    }
    else {
      // exists
      (mIt->second)->addPlotDataSet(plotDataList.front().data);
    }
    plotDataList.pop_front();
  }

  // built a map of pages and emptied the plotDataList list (argument).

  // set page titles and tags if specified ... emptying the list as we go
  while (!titleList.isEmpty()) {
    mIt = pageMap.find(titleList.front().page);
    if (mIt != pageMap.end()) {
      (mIt->second)->setTitle(titleList.front().title);
      (mIt->second)->setTag(titleList.front().tag);
    }
    titleList.pop_front();
  }

  // shift the items in the pageMap to a QList for cheap and safe dispatch. 

  QList< RefCountConstPtr<CPlotPageData> > pageList;

  mIt = pageMap.begin();
  while (mIt != pageMap.end()) {
    pageList.push_back(RefCountConstPtr<CPlotPageData>(mIt->second));
    ++mIt;
  }
  pageMap.clear();

  // send the pages to any connected slots
  emit signalPlotPages(pageList);
}

void CEngineController::notifyTableData(QList<SCell> &cellList)
{
  // the controller takes the cells and organises the data into
  // pages. Each page is then (safely) dispatched.

  std::map<int,CTablePageData*> pageMap;
  std::map<int,CTablePageData*>::iterator mIt;
  int pageNo;

  while (!cellList.isEmpty()) {
    // existing page?
    pageNo = cellList.front().page;
    mIt = pageMap.find(pageNo);
    if (mIt == pageMap.end()) {
      // need a new page
      CTablePageData *newPage = new CTablePageData(pageNo);
      newPage->addCell(cellList.front().row, cellList.front().col, cellList.front().data);
      pageMap.insert(std::map<int,CTablePageData*>::value_type(pageNo, newPage));
    }
    else {
      // exists
      (mIt->second)->addCell(cellList.front().row, cellList.front().col, cellList.front().data);
    }
    cellList.pop_front();
  }

  // built a map of pages and emptied cellList (argument).
  // shift them to a QList for cheap and safe dispatch. 

  QList< RefCountConstPtr<CTablePageData> > pageList;

  mIt = pageMap.begin();
  while (mIt != pageMap.end()) {
    pageList.push_back(RefCountConstPtr<CTablePageData>(mIt->second));
    ++mIt;
  }
  pageMap.clear();

  // send the pages to any connected slots
  emit signalTablePages(pageList);
}

void CEngineController::notifyErrorMessages(int highestErrorLevel, const QList<CEngineError> &errorMessages)
{
  // format each into a message text and put in a single string for posting ...
  QString msg;
  QTextStream stream(&msg);

  QList<CEngineError>::const_iterator it = errorMessages.begin();
  while (it != errorMessages.end()) {
    // one message per line
    switch (it->errorLevel()) {
    case eInformationEngineError:
      stream << "INFO    (";
      break;
    case eWarningEngineError:
      stream << "WARNING (";
      break;
    case eFatalEngineError:
      stream << "FATAL   (";
      break;
    }

    stream << it->tag() << ") " << it->message() << ".\n";

    ++it;
  }

  emit signalErrorMessages(highestErrorLevel, msg);
}

void CEngineController::notifyEndBrowseFile(void)
{
  // nothing to do unless the request bit is set (a stop request).
  if (m_mode & cRequestBit) {
    m_mode = 0; // definitely stopped
    
    emit signalSessionRunning(isSessionRunning());
  }
}

bool CEngineController::event(QEvent *e)
{
  if (e->type() == cEngineResponseType) {

    // one or more responses are ready for processing ...
    QList<CEngineResponse*> responses;

    m_thread->takeResponses(responses);

    // work through the responses ...
    while (!responses.isEmpty()) {
      CEngineResponse *tmp = responses.takeFirst();

      tmp->process(this);

      delete tmp;
    }

    e->accept();
    return true;
  }

  // let the base class handle it
  return QObject::event(e);
}

void CEngineController::slotNextFile()
{
  CEngineRequestCompound *req = new CEngineRequestCompound;
  
  // done with the current file
  if (m_numberOfRecords >= 0) {
    switch (m_mode) {
    case cBrowseMode:
      req->addRequest(new CEngineRequestEndBrowseFile);
      break;
    case cAnalyseMode:
      break;
    }
  }

  if (!m_currentIt.atEnd()) {
    ++m_currentIt;
    if (!m_currentIt.atEnd()) {
      // check for a change in project
      if (m_currentProject != m_currentIt.project()) {
	m_currentProject = m_currentIt.project();
	req->addRequest(new CEngineRequestSetProject(m_currentProject));
      }

      switch (m_mode) {
      case cBrowseMode:
	req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().filePath()));
	break;
      case cAnalyseMode:
	break;
      }
    }

  }

  m_thread->request(req);
}

void CEngineController::slotGotoFile(int number)
{
  CEngineRequestCompound *req = new CEngineRequestCompound;
  
  // done with the current file
  if (m_numberOfRecords >= 0) {
    switch (m_mode) {
    case cBrowseMode:
      req->addRequest(new CEngineRequestEndBrowseFile);
      break;
    case cAnalyseMode:
      break;
    }
  }

  if (number >= 0 && number < m_numberOfFiles) {
    // implicitly checks that that m_numberOfFiles > 0
    m_currentIt(number);
    // check for a change in project
    if (m_currentProject != m_currentIt.project()) {
      m_currentProject = m_currentIt.project();
      req->addRequest(new CEngineRequestSetProject(m_currentProject));
    }
    
    switch (m_mode) {
    case cBrowseMode:
      req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().filePath()));
      break;
    case cAnalyseMode:
      break;
    }
  }

  m_thread->request(req);
}


void CEngineController::slotFirstRecord()
{
  slotGotoRecord(1);
}

void CEngineController::slotPreviousRecord()
{
  slotGotoRecord(m_currentRecord - 1);
}

void CEngineController::slotNextRecord()
{
  if (m_currentRecord != -1) {
    
    switch (m_mode) {
    case cBrowseMode:
      m_thread->request(new CEngineRequestBrowseNextRecord);
      break;
    case cAnalyseMode:
      break;
    }
  }
}

void CEngineController::slotLastRecord()
{
  slotGotoRecord(m_numberOfRecords);
}

void CEngineController::slotGotoRecord(int recordNumber)
{
  if (m_currentRecord != -1 && recordNumber > 0 && recordNumber <= m_numberOfRecords) {

    switch (m_mode) {
    case cBrowseMode:
      m_thread->request(new CEngineRequestBrowseSpecificRecord(recordNumber));
      break;
    case cAnalyseMode:
      break;
    }
  }
}

void CEngineController::slotStep()
{
  if (m_currentRecord >= 0 && m_currentRecord < m_numberOfRecords) {
    // step record
    switch (m_mode) {
    case cBrowseMode:
      m_thread->request(new CEngineRequestBrowseNextRecord);
      break;
    case cAnalyseMode:
      break;
    }
  }
  else if (m_currentRecord == m_numberOfRecords && !m_currentIt.atEnd()) {

    CEngineRequestCompound *req = new CEngineRequestCompound;
  
    // done with the current file
    if (m_numberOfRecords >= 0) {
      switch (m_mode) {
      case cBrowseMode:
	req->addRequest(new CEngineRequestEndBrowseFile);
	break;
      case cAnalyseMode:
	break;
      }
    }

    ++m_currentIt;
    if (!m_currentIt.atEnd()) {
      // check for a change in project
      if (m_currentProject != m_currentIt.project()) {
	m_currentProject = m_currentIt.project();
	req->addRequest(new CEngineRequestSetProject(m_currentProject));
      }

      switch (m_mode) {
      case cBrowseMode:
	req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().filePath()));
	break;
      case cAnalyseMode:
	break;
      }
    }

    m_thread->request(req);
  }
}

void CEngineController::slotStartBrowseSession(const RefCountPtr<CSession> &session)
{
  // make a compound request
  CEngineRequestCompound *req = new CEngineRequestCompound;

  // tidy up from the previous(current) session
  switch (m_mode) {
  case cBrowseMode:
    req->addRequest(new CEngineRequestEndBrowseFile); 
    break;
  case cAnalyseMode:
    break;
  }

  // change session and reset current markers
  m_session = session;
  m_currentIt = CSessionIterator(m_session);

  QStringList sessionFileList = m_session->fileList();

  m_numberOfFiles = sessionFileList.count();
  m_currentRecord = -1;
  m_currentProject = NULL;

  if (!m_currentIt.atEnd()) {

    m_mode = cBrowseMode | cRequestBit;

    m_currentProject = m_currentIt.project();

    req->addRequest(new CEngineRequestSetProject(m_currentProject));
    req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().filePath()));
  }

  emit signalFileListChanged(sessionFileList);

  m_thread->request(req);
}

// end a session
void CEngineController::slotStopSession()
{
  int oldMode = m_mode;

  m_mode |= cRequestBit;

  // tidy up and wait for the response
  switch (oldMode) {
  case cBrowseMode:
    m_thread->request(new CEngineRequestEndBrowseFile); 
    break;
  case cAnalyseMode:
    break;
  }
}

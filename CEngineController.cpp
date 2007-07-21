
#include <iostream> // TODO

#include "CEngineController.h"
#include "CEngineRequest.h"
#include "CEngineResponse.h"

static const int cBrowseMode  = 1;
static const int cAnalyseMode = 2;


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

void CEngineController::notifySetProject(void)
{
  std::cout << "CEngineController::notifySetProject OBSOLETE" << std::endl;
}

void CEngineController::notifyNumberOfFiles(int nFiles)
{
  
}

void CEngineController::notifyCurrentFile(int fileNumber)
{
}

void CEngineController::notifyReadyToNavigateRecords(int numberOfRecords)
{
  // successfully started browsing a file
  // TODO signals for buttons

  m_numberOfRecords = numberOfRecords;
  m_currentRecord = 0;

  std::cout << "CEngineController::notifyNumberOfRecords " << m_numberOfRecords << std::endl;

  // signals for navigation control
  // records
  emit signalNumberOfRecordsChanged(m_numberOfRecords);
  emit signalCurrentRecordChanged(m_currentRecord);

  // files
  emit signalCurrentFileChanged(m_currentIt.index() + 1);
}

void CEngineController::notifyCurrentRecord(int recordNumber)
{
  m_currentRecord = recordNumber;
  
  std::cout << "CEngineController::notifyCurrentRecord " << recordNumber << std::endl;

  emit signalCurrentRecordChanged(m_currentRecord);
}

void CEngineController::notifyEndOfRecords(void)
{
  m_currentRecord = m_numberOfRecords + 1;
  
  std::cout << "CEngineController::notifyEndOfRecords"<< std::endl;

  emit signalCurrentRecordChanged(m_currentRecord);
}

void CEngineController::notifyPlotData(QList<SPlotDataBucket> &buckets)
{
  // the controller takes the buckets and organises the data-sets
  // into a set of pages. Each page is then (safely) dispatched.

  std::map<int,CPlotPageData*> pageMap;
  std::map<int,CPlotPageData*>::iterator mIt;
  int pageNo;

  while (!buckets.isEmpty()) {
    // existing page?
    pageNo = buckets.front().page;
    mIt = pageMap.find(pageNo);
    if (mIt == pageMap.end()) {
      // need a new page
      CPlotPageData *newPage = new CPlotPageData(pageNo);
      newPage->addPlotDataSet(buckets.front().data);
      pageMap.insert(std::map<int,CPlotPageData*>::value_type(pageNo, newPage));
    }
    else {
      // exists
      (mIt->second)->addPlotDataSet(buckets.front().data);
    }
    buckets.pop_front();
  }

  // built a map of pages and emptied the buckets list (argument).
  // shift them to a QList for cheap and safe dispatch. 
  std::cout << "notify - " << pageMap.size() << " pages" << std::endl;

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

void CEngineController::slotFirstFile()
{
  slotGotoFile(1);
}

void CEngineController::slotPreviousFile()
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

  if (m_numberOfFiles && !m_currentIt.atBegin()) {
    --m_currentIt;
    // check for a change in project
    if (m_currentProject != m_currentIt.project()) {
      m_currentProject = m_currentIt.project();
      req->addRequest(new CEngineRequestSetProject(m_currentProject));
    }

    switch (m_mode) {
    case cBrowseMode:
      req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().fileName()));
      break;
    case cAnalyseMode:
      break;
    }
  }

  m_thread->request(req);
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
	req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().fileName()));
	break;
      case cAnalyseMode:
	break;
      }
    }

  }

  m_thread->request(req);
}

void CEngineController::slotLastFile()
{
  slotGotoFile(m_numberOfFiles);
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

  --number; // make it zero indexing

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
      req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().fileName()));
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

void CEngineController::slotStartBrowseSession(RefCountPtr<CSession> session)
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

  // change session and resent current markers
  m_session = session;
  m_currentIt = CSessionIterator(m_session);
  m_numberOfFiles = m_session->size();
  m_currentRecord = -1;
  m_currentProject = NULL;

  if (!m_currentIt.atEnd()) {

    m_mode = cBrowseMode;

    m_currentProject = m_currentIt.project();

    req->addRequest(new CEngineRequestSetProject(m_currentProject));
    req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().fileName()));
  }

  emit signalNumberOfFilesChanged(m_numberOfFiles);

  m_thread->request(req);
}

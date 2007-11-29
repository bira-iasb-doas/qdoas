/* Qdoas is a cross-platform application for spectral analysis with the DOAS
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

#include "CQdoasEngineController.h"
#include "CEngineRequest.h"
#include "CEngineResponse.h"
#include "constants.h"

#include "debugutil.h"

CQdoasEngineController::CQdoasEngineController(QObject *parent) :
  QObject(parent),
  CEngineController(),
  m_state(Idle),
  m_currentProject(NULL),
  m_currentRecord(-1),
  m_numberOfRecords(0),
  m_numberOfFiles(0)
{

  // create the engine thread
  m_thread = new CEngineThread(this);

  m_thread->setRunState(true);
}

CQdoasEngineController::~CQdoasEngineController()
{
}

void CQdoasEngineController::notifyNumberOfFiles(int nFiles)
{

}

void CQdoasEngineController::notifyCurrentFile(int fileNumber)
{
}

void CQdoasEngineController::notifyReadyToNavigateRecords(const QString &filename, int numberOfRecords)
{
  // successfully started accessing a file

  // sanity check - currentIt and filename must be consistent
  assert(m_currentIt.file().filePath() == filename);

  m_numberOfRecords = numberOfRecords;
  m_currentRecord = 0;

  // signals for navigation control
  emit signalCurrentFileChanged(m_currentIt.index(), m_numberOfRecords);
  // files
  emit signalCurrentFileChanged(filename);

  // session is up and running
  m_state = Running;
  emit signalSessionRunning(isSessionRunning());

  slotNextRecord(); // goto the first spectrum ...
}

void CQdoasEngineController::notifyCurrentRecord(int recordNumber)
{
  m_currentRecord = recordNumber;

  emit signalCurrentRecordChanged(m_currentRecord);
}

void CQdoasEngineController::notifyEndOfRecords(void)
{
  m_currentRecord = m_numberOfRecords + 1;

  emit signalCurrentRecordChanged(m_currentRecord);
}

void CQdoasEngineController::notifyPlotData(QList<SPlotData> &plotDataList, QList<STitleTag> &titleList)
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

void CQdoasEngineController::notifyTableData(QList<SCell> &cellList)
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

void CQdoasEngineController::notifyErrorMessages(int highestErrorLevel, const QList<CEngineError> &errorMessages)
{
  // format each into a message text and put in a single string for posting ...
  QString msg;
  QTextStream stream(&msg);

  QList<CEngineError>::const_iterator it = errorMessages.begin();
  while (it != errorMessages.end()) {
    // one message per line
    switch (it->errorLevel()) {
    case InformationEngineError:
      stream << "INFO    (";
      break;
    case WarningEngineError:
      stream << "WARNING (";
      break;
    case FatalEngineError:
      stream << "FATAL   (";
      break;
    }

    stream << it->tag() << ") " << it->message() << ".\n";

    ++it;
  }

  emit signalErrorMessages(highestErrorLevel, msg);
}

void CQdoasEngineController::notifyEndAccessFile(void)
{
  if (m_state == Stopping) {
    m_state = Idle;

    emit signalSessionRunning(isSessionRunning());
  }
}

bool CQdoasEngineController::event(QEvent *e)
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

void CQdoasEngineController::slotNextFile()
{
  if (m_state != Running) return;

  CEngineRequestCompound *req = new CEngineRequestCompound;

  // done with the current file
  if (m_numberOfRecords >= 0) {
    switch (m_session->mode()) {
    case CSession::Browse:
      req->addRequest(new CEngineRequestEndBrowseFile);
      break;
    case CSession::Calibrate:
      req->addRequest(new CEngineRequestEndCalibrateFile);
      break;
    case CSession::Analyse:
      req->addRequest(new CEngineRequestEndAnalyseFile);
      break;
    }
  }

  if (!m_currentIt.atEnd()) {
    ++m_currentIt;
    if (!m_currentIt.atEnd()) {
      // check for a change in project
      if (m_currentProject != m_currentIt.project()) {

	int opMode = THREAD_TYPE_NONE;
	switch (m_session->mode()) {
	case CSession::Browse: opMode = THREAD_TYPE_SPECTRA; break;
	case CSession::Calibrate: opMode = THREAD_TYPE_KURUCZ; break;
	case CSession::Analyse: opMode = THREAD_TYPE_ANALYSIS; break;
	}
	m_currentProject = m_currentIt.project();
	req->addRequest(new CEngineRequestSetProject(m_currentProject, opMode));
	// might also need to replace the analysis windows
	if (m_session->mode() == CSession::Calibrate || m_session->mode() == CSession::Analyse) {
	  int nWindows;
	  const mediate_analysis_window_t *anlysWinList = m_currentIt.analysisWindowList(nWindows);
	  req->addRequest(new CEngineRequestSetAnalysisWindows(anlysWinList, nWindows, opMode));
	}
      }

      switch (m_session->mode()) {
      case CSession::Browse:
	req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().filePath()));
	break;
      case CSession::Calibrate:
	req->addRequest(new CEngineRequestBeginCalibrateFile(m_currentIt.file().filePath()));
	break;
      case CSession::Analyse:
	req->addRequest(new CEngineRequestBeginAnalyseFile(m_currentIt.file().filePath()));
	break;
      }
    }

  }

  m_thread->request(req);
}

void CQdoasEngineController::slotGotoFile(int number)
{
  if (m_state != Running) return;

  CEngineRequestCompound *req = new CEngineRequestCompound;

  // done with the current file
  if (m_numberOfRecords >= 0) {
    switch (m_session->mode()) {
    case CSession::Browse:
      req->addRequest(new CEngineRequestEndBrowseFile);
      break;
    case CSession::Calibrate:
      req->addRequest(new CEngineRequestEndCalibrateFile);
      break;
    case CSession::Analyse:
      req->addRequest(new CEngineRequestEndAnalyseFile);
      break;
    }
  }

  if (number >= 0 && number < m_numberOfFiles) {
    // implicitly checks that that m_numberOfFiles > 0
    m_currentIt(number);
    // check for a change in project
    if (m_currentProject != m_currentIt.project()) {
      int opMode = THREAD_TYPE_NONE;
      switch (m_session->mode()) {
      case CSession::Browse: opMode = THREAD_TYPE_SPECTRA; break;
      case CSession::Calibrate: opMode = THREAD_TYPE_KURUCZ; break;
      case CSession::Analyse: opMode = THREAD_TYPE_ANALYSIS; break;
      }
      m_currentProject = m_currentIt.project();
      req->addRequest(new CEngineRequestSetProject(m_currentProject, opMode));
    }

    switch (m_session->mode()) {
    case CSession::Browse:
      req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().filePath()));
      break;
    case CSession::Calibrate:
      req->addRequest(new CEngineRequestBeginCalibrateFile(m_currentIt.file().filePath()));
      break;
    case CSession::Analyse:
      req->addRequest(new CEngineRequestBeginAnalyseFile(m_currentIt.file().filePath()));
      break;
    }
  }

  m_thread->request(req);
}


void CQdoasEngineController::slotFirstRecord()
{
  slotGotoRecord(1);
}

void CQdoasEngineController::slotPreviousRecord()
{
  slotGotoRecord(m_currentRecord - 1);
}

void CQdoasEngineController::slotNextRecord()
{
  if (m_state != Running) return;

  if (m_currentRecord != -1) {

    switch (m_session->mode()) {
    case CSession::Browse:
      m_thread->request(new CEngineRequestBrowseNextRecord);
      break;
    case CSession::Calibrate:
      m_thread->request(new CEngineRequestCalibrateNextRecord);
      break;
    case CSession::Analyse:
      m_thread->request(new CEngineRequestAnalyseNextRecord);
      break;
    }
  }
}

void CQdoasEngineController::slotLastRecord()
{
  slotGotoRecord(m_numberOfRecords);
}

void CQdoasEngineController::slotGotoRecord(int recordNumber)
{
  if (m_state != Running) return;

  if (m_currentRecord != -1 && recordNumber > 0 && recordNumber <= m_numberOfRecords) {

    switch (m_session->mode()) {
    case CSession::Browse:
      m_thread->request(new CEngineRequestBrowseSpecificRecord(recordNumber));
      break;
    case CSession::Calibrate:
      m_thread->request(new CEngineRequestCalibrateSpecificRecord(recordNumber));
      break;
    case CSession::Analyse:
      m_thread->request(new CEngineRequestAnalyseSpecificRecord(recordNumber));
      break;
    }
  }
}

void CQdoasEngineController::slotStep()
{
  if (m_state != Running) return;

  if (m_currentRecord >= 0 && m_currentRecord < m_numberOfRecords) {
    // step record
    switch (m_session->mode()) {
    case CSession::Browse:
      m_thread->request(new CEngineRequestBrowseNextRecord);
      break;
    case CSession::Calibrate:
      m_thread->request(new CEngineRequestCalibrateNextRecord);
      break;
    case CSession::Analyse:
      m_thread->request(new CEngineRequestAnalyseNextRecord);
      break;
    }
  }
  else if (m_currentRecord == m_numberOfRecords && !m_currentIt.atEnd()) {

    CSessionIterator tmpIt = m_currentIt;

    // only move if there IS a next file ... this leaves the last file 'active'.

    ++tmpIt;
    if (!tmpIt.atEnd()) {
      m_currentIt = tmpIt;

      // move to the next file
      CEngineRequestCompound *req = new CEngineRequestCompound;

      // done with the current file
      if (m_numberOfRecords >= 0) {
	switch (m_session->mode()) {
	case CSession::Browse:
	  req->addRequest(new CEngineRequestEndBrowseFile);
	  break;
	case CSession::Calibrate:
	  req->addRequest(new CEngineRequestEndCalibrateFile);
	  break;
	case CSession::Analyse:
	  req->addRequest(new CEngineRequestEndAnalyseFile);
	  break;
	}
      }

      // check for a change in project
      if (m_currentProject != m_currentIt.project()) {
	int opMode = THREAD_TYPE_NONE;
	switch (m_session->mode()) {
	case CSession::Browse: opMode = THREAD_TYPE_SPECTRA; break;
	case CSession::Calibrate: opMode = THREAD_TYPE_KURUCZ; break;
	case CSession::Analyse: opMode = THREAD_TYPE_ANALYSIS; break;
	}
	m_currentProject = m_currentIt.project();
	req->addRequest(new CEngineRequestSetProject(m_currentProject, opMode));
      }

      switch (m_session->mode()) {
      case CSession::Browse:
	req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().filePath()));
	break;
      case CSession::Calibrate:
	req->addRequest(new CEngineRequestBeginCalibrateFile(m_currentIt.file().filePath()));
	break;
      case CSession::Analyse:
	req->addRequest(new CEngineRequestBeginAnalyseFile(m_currentIt.file().filePath()));
	break;
      }

      m_thread->request(req);
    }
  }
}

void CQdoasEngineController::slotStartSession(const RefCountPtr<CSession> &session)
{
  // the controller must be idle to start a session ...
  if (m_state != Idle) return;

  // need a compound request
  CEngineRequestCompound *req = new CEngineRequestCompound;

  // change session and reset current markers
  m_session = session;
  m_currentIt = CSessionIterator(m_session);

  QStringList sessionFileList = m_session->fileList();

  m_numberOfFiles = sessionFileList.count();
  m_currentRecord = -1;
  m_currentProject = NULL;

  if (!m_currentIt.atEnd()) {
    m_currentProject = m_currentIt.project();

    // mode dependent parts of the request
    if (m_session->mode() == CSession::Browse) {
      req->addRequest(new CEngineRequestSetProject(m_currentProject, THREAD_TYPE_SPECTRA));
      req->addRequest(new CEngineRequestBeginBrowseFile(m_currentIt.file().filePath()));
    }
    else {
      // take the site and symbol lists from the session ... and hand responsibility over to request objects.
      int nSymbols, nSites, nWindows;
      mediate_symbol_t *symbols = m_session->takeSymbolList(nSymbols);
      mediate_site_t *sites = m_session->takeSiteList(nSites);
      const mediate_analysis_window_t *anlysWinList = m_currentIt.analysisWindowList(nWindows);

      if (symbols)
	req->addRequest(new CEngineRequestSetSymbols(symbols, nSymbols));
      if (sites)
	req->addRequest(new CEngineRequestSetSites(sites, nSites));


      if (m_session->mode() == CSession::Analyse) {
	req->addRequest(new CEngineRequestSetProject(m_currentProject, THREAD_TYPE_ANALYSIS));
	req->addRequest(new CEngineRequestSetAnalysisWindows(anlysWinList, nWindows, THREAD_TYPE_ANALYSIS));
	req->addRequest(new CEngineRequestBeginAnalyseFile(m_currentIt.file().filePath()));
      }
      else if (m_session->mode() == CSession::Calibrate) {
	req->addRequest(new CEngineRequestSetProject(m_currentProject, THREAD_TYPE_KURUCZ));
	req->addRequest(new CEngineRequestSetAnalysisWindows(anlysWinList, nWindows, THREAD_TYPE_KURUCZ));
	req->addRequest(new CEngineRequestBeginCalibrateFile(m_currentIt.file().filePath()));
      }
    }

    // change state
    m_state = Pending;
  }

  emit signalFileListChanged(sessionFileList);

  m_thread->request(req);
}

void CQdoasEngineController::slotStopSession()
{
  if (m_state != Running) return;

  m_state = Stopping;

  // tidy up and wait for the response
  switch (m_session->mode()) {
  case CSession::Browse:
    m_thread->request(new CEngineRequestEndBrowseFile);
    break;
  case CSession::Calibrate:
    m_thread->request(new CEngineRequestEndCalibrateFile);
    break;
  case CSession::Analyse:
    m_thread->request(new CEngineRequestEndAnalyseFile);
    break;
  }
}

void CQdoasEngineController::slotViewCrossSections(const RefCountPtr<CViewCrossSectionData> &awData)
{
  TRACE("CQdoasEngineController::slotViewCrossSections");

  if (m_state != Idle) return;

  const mediate_analysis_window_t *d = awData->analysisWindow();

  int nFiles = d->crossSectionList.nCrossSection;

  char **filenames = new char*[nFiles];
  char *awName=new char[strlen(d->name)];

  strcpy(awName,d->name);

  for (int i=0; i<nFiles; ++i) {
    int len = strlen(d->crossSectionList.crossSection[i].crossSectionFile);
    char *tmp = new char[len + 1];
    strcpy(tmp, d->crossSectionList.crossSection[i].crossSectionFile);
    filenames[i] = tmp;
  }

  // request takes responsibility for the char** and char* memory.

  CEngineRequestViewCrossSections *req = new CEngineRequestViewCrossSections(awName,
                                                                             d->fitMinWavelength,
                                                                             d->fitMaxWavelength,
                                                                             nFiles, filenames);


  // send the request
  m_thread->request(req);
}

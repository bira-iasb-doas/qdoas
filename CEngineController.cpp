
#include "CEngineController.h"
#include "CEngineRequest.h"
#include "CEngineResponse.h"

static const int cBrowseMode  = 1;
static const int cAnalyseMode = 2;


CEngineController::CEngineController(QObject *parent) :
  QObject(parent),
  m_mode(0),
  m_currentFile(-1),
  m_currentRecord(-1)
{

  // create the engine thread
  m_thread = new CEngineThread(this);

  m_thread->setRunState(true);
}

CEngineController::~CEngineController()
{
}

bool CEngineController::browseSpectra(const mediate_project_t *project,
				      QList<QFileInfo> &fileList)
{
  // first abort the current processing if any ... TODO
  /*
  switch (m_mode) {
  case cBrowseMode:
    {
      m_thread->request(new CEngineRequestEndBrowse);
    }
    break;
  case cAnalyseMode:
    {
      m_thread->request(new CEngineRequestEndAnalyse);
    }
  }
  */

  m_mode = 0;  

  // discard the current list of files and replace with the new list
  m_fileList = fileList;
  
  if (!m_fileList.isEmpty()) {
    m_mode = cBrowseMode;
    // configure the engine - response will trigger the next action
    m_thread->request(new CEngineRequestSetProject(project));
  }
}

void CEngineController::notifySetProject(void)
{
  // follow up based on the mode
  switch (m_mode) {
  case cBrowseMode:
    {
      // set the active file to the first in the list and make a request
      if (!m_fileList.isEmpty()) {
	m_currentFile = 0;
	m_thread->request(new CEngineRequestBeginBrowseFile(m_fileList.at(m_currentFile).fileName()));
      }
    }
    break;
  case cAnalyseMode:
    {
    }
    break;
  }
}

void CEngineController::notifyNumbeOfFiles(int nFiles)
{
}

void CEngineController::notifyCurrentFile(int fileNumber)
{
}

void CEngineController::notifyNumberOfRecords(int nRec)
{
}

void CEngineController::notifyCurrentRecord(int recNumber)
{
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
}

void CEngineController::slotNextFile()
{
}

void CEngineController::slotPreviousFile()
{
}

void CEngineController::slotGotoFile(int number)
{
}

void CEngineController::slotFirstRecord()
{
}

void CEngineController::slotNextRecord()
{
}

void CEngineController::slotGotoRecord(int recNumber)
{
}

void CEngineController::slotStartBrowseSession(RefCountedPtr<CSession> session)
{
  m_session = session;

  // commence ... TODO
}

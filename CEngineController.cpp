
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
  m_currentRecord(-1)
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
  // follow up based on the mode
  switch (m_mode) {
  case cBrowseMode:
    {
      if (!m_currentIt.atEnd()) {
        std::cout << "CEngineController::notifySetProject" << std::endl;
	m_thread->request(new CEngineRequestBeginBrowseFile(m_currentIt.file().fileName()));
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
  // successfully started browsing a file
  // TODO signals for buttons

  m_currentRecord = 0;

  std::cout << "CEngineController::notifyNumberOfRecords " << nRec << std::endl;
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

void CEngineController::slotStartBrowseSession(RefCountPtr<CSession> session)
{
  // shutdown current activity ... TODO

  m_session = session;

  m_currentIt = CSessionIterator(m_session);
  m_currentProject = NULL;

  if (!m_currentIt.atEnd()) {

    m_currentProject = m_currentIt.project();
    m_mode = cBrowseMode;
    // configure the engine - response will trigger the next action
    m_thread->request(new CEngineRequestSetProject(m_currentProject));    
  }
}

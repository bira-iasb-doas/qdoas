
#include <assert.h>

#include <QCoreApplication>

#include "CEngineThread.h"
#include "CEngineController.h"
#include "CEngineRequest.h"
#include "CEngineResponse.h"

#include "mediate.h"

CEngineThread::CEngineThread(CEngineController *parent) :
  QThread(),
  m_activeRequest(NULL),
  m_terminated(false)
{
  // get an engine context
  assert(mediateRequestCreateEngineContext(&m_engineContext) == 0);
}

CEngineThread::~CEngineThread()
{
  m_reqQueueMutex.lock();
  m_terminated = false;
  m_reqQueueMutex.unlock();

  // wait intil the run thread has stopped ...
  while (isRunning())
    QThread::msleep(50);

  assert(mediateRequestDestroyEngineContext(m_engineContext) == 0);
}

void CEngineThread::setRunState(bool setRunning)
{
  if (setRunning) {
    m_reqQueueMutex.lock();
    m_terminated = false;
    m_reqQueueMutex.unlock();
    
    start();
  }
  else {
    m_reqQueueMutex.lock();
    m_terminated = true;
    m_reqQueueMutex.unlock();
    // let the run loop exit cleanly
  }
}

void CEngineThread::request(CEngineRequest *req)
{
  m_reqQueueMutex.lock();
  
  m_requests.push_back(req); // add the request
  
  // wake the engine thread if it is sitting idle
  if (m_activeRequest == NULL)
    m_cv.wakeOne();
  
  m_reqQueueMutex.unlock();
}

void CEngineThread::takeResponses(QList<CEngineResponse*> &responses)
{
  m_respQueueMutex.lock();
  
  responses = m_responses;
  m_responses.clear();

  m_respQueueMutex.unlock();
}

void CEngineThread::respond(CEngineResponse *resp)
{
  // used by the engine thread to post data for collection by the GUI thread
  m_respQueueMutex.lock();

  m_responses.push_back(resp);
  // post the notify event to the parent
  QCoreApplication::postEvent(parent(), new QEvent(cEngineResponseType));

  m_respQueueMutex.unlock();
}

void CEngineThread::run()
{
  // engine thread - loop until terminated ...
  //
  // m_terminated, m_requests and m_activeRequest are all protected by 
  // m_reqQueueMutex

  m_reqQueueMutex.lock();
  while (!m_terminated) {
    
    while (m_requests.isEmpty() && !m_terminated) {
      m_cv.wait(&m_reqQueueMutex, 2000); // wake every two seconds to check for termination
    }
    if (!m_requests.isEmpty() && !m_terminated) {
      // queue is not empty - take the first item ...
      m_activeRequest = m_requests.takeFirst();
      
      m_reqQueueMutex.unlock();

      // process the active request
      m_activeRequest->process(this);

      // lock before checking m_terminated
      m_reqQueueMutex.lock();
    }
  }
  m_reqQueueMutex.unlock();
}

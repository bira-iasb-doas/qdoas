
#include <assert.h>

#include <QCoreApplication>

#include "CEngineThread.h"
#include "CEngineController.h"
#include "CEngineRequest.h"
#include "CEngineResponse.h"

#include "mediate.h"

CEngineThread::CEngineThread(CEngineController *parent) :
  QThread(parent),
  m_activeRequest(NULL),
  m_terminated(false)
{
  // get an engine context
  assert(mediateRequestCreateEngineContext(&m_engineContext) == 0);
}

CEngineThread::~CEngineThread()
{
  m_reqQueueMutex.lock();
  m_terminated = true;
  m_cv.wakeOne();
  m_reqQueueMutex.unlock();

  // wait until the run thread has stopped ...
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
    m_cv.wakeOne();
    m_reqQueueMutex.unlock();
    // let the run loop exit cleanly
  }
}

void CEngineThread::request(CEngineRequest *req)
{
  m_reqQueueMutex.lock();
  
  m_requests.push_back(req); // add the request
  // wake the engine thread if it is sitting idle (harmless if it is not idle)
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
  // m_terminated, m_requests and are protected by 
  // m_reqQueueMutex

  CEngineRequest *activeRequest;

  m_reqQueueMutex.lock();
  while (!m_terminated) {
    
    // will wake the thread if a request is lodged or terminated is set true.
    while (m_requests.isEmpty() && !m_terminated) {
      m_cv.wait(&m_reqQueueMutex);
    }

    if (!m_requests.isEmpty() && !m_terminated) {
      // queue is not empty - process the first item
      activeRequest = m_requests.takeFirst();
      
      m_reqQueueMutex.unlock();

      // process the request then discard it
      activeRequest->process(this);
      delete activeRequest;

      // lock before modifying the queue and checking m_terminated 
      m_reqQueueMutex.lock();
    }
  }
  m_reqQueueMutex.unlock();
}

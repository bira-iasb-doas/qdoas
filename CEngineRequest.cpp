
#include "CEngineRequest.h"
#include "CEngineResponse.h"
#include "CEngineThread.h"

#include "mediate.h"

//------------------------------------------------------------

CEngineRequest::CEngineRequest(int type) :
  m_type(type)
{
}

CEngineRequest::~CEngineRequest()
{
}

//------------------------------------------------------------

CEngineRequestSetProject::CEngineRequestSetProject(const mediate_project_t *project) :
  CEngineRequest(cEngineRequestSetProjectType)
{
  // deep copy the data from project.
  m_project = *project;
}

CEngineRequestSetProject::~CEngineRequestSetProject()
{
}

void CEngineRequestSetProject::process(CEngineThread *engineThread)
{
  // process is called from the thread and drives the engine through the
  // mediator interface.

  int rc = mediateRequestSetProject(engineThread->engineContext(),
				    &m_project);

  // post a response
  CEngineResponseSetProject *resp = new CEngineResponseSetProject;

  if (rc == -1)
    resp->addErrorMessage("Failed to copy project data to the engine.");

  engineThread->respond(resp);
}

//------------------------------------------------------------

CEngineRequestBeginBrowseFile::CEngineRequestBeginBrowseFile(const QString &fileName) :
  CEngineRequest(cEngineRequestBeginBrowseFileType),
  m_fileName(fileName)
{
}

CEngineRequestBeginBrowseFile::~CEngineRequestBeginBrowseFile()
{
}

void CEngineRequestBeginBrowseFile::process(CEngineThread *engineThread)
{
  // open the file and get back the number of records (and calibration data?)

  // create a response as the handle
  CEngineResponseBeginBrowseFile *resp = new CEngineResponseBeginBrowseFile(m_fileName);

  int rc = mediateRequestBeginBrowseSpectra(engineThread->engineContext(),
					    m_fileName.toAscii().constData(), resp);

  resp->setNumberOfRecords(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);
}

//------------------------------------------------------------


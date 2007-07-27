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

CEngineRequestCompound::CEngineRequestCompound() :
  CEngineRequest(cEngineRequestCompoundType)
{
}

CEngineRequestCompound::~CEngineRequestCompound()
{
  // delete all requests
  while (!m_requestList.isEmpty()) {
    delete m_requestList.takeFirst();
  }
}

bool CEngineRequestCompound::process(CEngineThread *engineThread)
{
  bool result = true; // empty is ok
  QList<CEngineRequest*>::iterator it = m_requestList.begin();

  // process sequentially until done or a failure occurred
  while (it != m_requestList.end() && (result = (*it)->process(engineThread)))
    ++it;

  return result;
}

void CEngineRequestCompound::addRequest(CEngineRequest *req)
{
  // takes owneship of req
  m_requestList.push_back(req);
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

bool CEngineRequestSetProject::process(CEngineThread *engineThread)
{
  // process is called from the thread and drives the engine through the
  // mediator interface.

  int rc = mediateRequestSetProject(engineThread->engineContext(),
				    &m_project);

  // TODO

  // no response unless there was an error
  //  if (rc == -1) {
    //    resp->addErrorMessage("Failed to copy project data to the engine.");
    //engineThread->respond(resp);

  return (rc == 0);
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

bool CEngineRequestBeginBrowseFile::process(CEngineThread *engineThread)
{
  // open the file and get back the number of records (and calibration data?)

  // create a response as the handle
  CEngineResponseBeginBrowseFile *resp = new CEngineResponseBeginBrowseFile(m_fileName);

  int rc = mediateRequestBeginBrowseSpectra(engineThread->engineContext(),
					    m_fileName.toAscii().constData(), resp);

  resp->setNumberOfRecords(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestBrowseNextRecord::CEngineRequestBrowseNextRecord() :
  CEngineRequest(cEngineRequestBrowseNextRecordType)
{
}

CEngineRequestBrowseNextRecord::~CEngineRequestBrowseNextRecord()
{
}

bool CEngineRequestBrowseNextRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseBrowseRecord *resp = new CEngineResponseBrowseRecord;

  int rc = mediateRequestGetNextMatchingSpectrum(engineThread->engineContext(),
						 resp);

  resp->setRecordNumber(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestBrowseSpecificRecord::CEngineRequestBrowseSpecificRecord(int recordNumber) :
  CEngineRequest(cEngineRequestBrowseSpecificRecordType),
  m_recordNumber(recordNumber)
{
}

CEngineRequestBrowseSpecificRecord::~CEngineRequestBrowseSpecificRecord()
{
}

bool CEngineRequestBrowseSpecificRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseBrowseRecord *resp = new CEngineResponseBrowseRecord;

  int rc = mediateRequestGotoSpectrum(engineThread->engineContext(),
				      m_recordNumber, resp);
  
  if (rc > 0) {
    // successfully positioned .. now browse
    rc = mediateRequestGetNextMatchingSpectrum(engineThread->engineContext(),
					       resp);

    resp->setRecordNumber(rc); // -1 if an error occurred
  }

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestEndBrowseFile::CEngineRequestEndBrowseFile() :
  CEngineRequest(cEngineRequestEndBrowseFileType)
{
}

CEngineRequestEndBrowseFile::~CEngineRequestEndBrowseFile()
{
}

bool CEngineRequestEndBrowseFile::process(CEngineThread *engineThread)
{
  // create a response as the handle - TODO
  void *resp = NULL;

  int rc = mediateRequestEndBrowseSpectra(engineThread->engineContext(), resp);

  return (rc != -1);
}


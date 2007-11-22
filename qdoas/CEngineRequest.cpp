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

#include "debugutil.h"

//------------------------------------------------------------

CEngineRequest::CEngineRequest(CEngineRequest::RequestType type) :
  m_type(type)
{
}

CEngineRequest::~CEngineRequest()
{
}

//------------------------------------------------------------

CEngineRequestCompound::CEngineRequestCompound() :
  CEngineRequest(eEngineRequestCompoundType)
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
  CEngineRequest(eEngineRequestSetProjectType)
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

  CEngineResponseMessage *resp = new CEngineResponseMessage;

  int rc = mediateRequestSetProject(engineThread->engineContext(),
				    &m_project, resp);

  // no response unless there was an error
  if (rc == -1)
    engineThread->respond(resp);
  else
    delete resp;

  return (rc == 0);
}

//------------------------------------------------------------

CEngineRequestSetAnalysisWindows::CEngineRequestSetAnalysisWindows(const mediate_analysis_window_t *windowList, int nWindows) :
  CEngineRequest(eEngineRequestSetAnalysisWindowType),
  m_windowList(NULL),
  m_nWindows(0)
{
  // deep copy the data from windowList
  if (windowList != NULL && nWindows > 0) {
    m_nWindows = nWindows;
    m_windowList = new mediate_analysis_window_t[m_nWindows];
    memcpy(m_windowList, windowList, m_nWindows * sizeof(mediate_analysis_window_t));
  }
}

CEngineRequestSetAnalysisWindows::~CEngineRequestSetAnalysisWindows()
{
  delete [] m_windowList;
}

bool CEngineRequestSetAnalysisWindows::process(CEngineThread *engineThread)
{
  // process is called from the thread and drives the engine through the
  // mediator interface.

  CEngineResponse *resp = new CEngineResponseMessage;

  int rc = mediateRequestSetAnalysisWindows(engineThread->engineContext(),
					    m_nWindows, m_windowList, resp);
  
  // no response unless there was an error
  if (rc == -1)
    engineThread->respond(resp);
  else
    delete resp;
  
  return (rc == 0);
}

//------------------------------------------------------------

CEngineRequestSetSymbols::CEngineRequestSetSymbols(mediate_symbol_t *symbolList, int nSymbols) :
  CEngineRequest(eEngineRequestSetSymbolsType),
  m_symbolList(symbolList),
  m_nSymbols(nSymbols)
{
  // takes ownership of the symbolList
}

CEngineRequestSetSymbols::~CEngineRequestSetSymbols()
{
  delete [] m_symbolList;
}

bool CEngineRequestSetSymbols::process(CEngineThread *engineThread)
{
  // process is called from the thread and drives the engine through the
  // mediator interface.

  CEngineResponse *resp = new CEngineResponseMessage;

  int rc = mediateRequestSetSymbols(engineThread->engineContext(),
				    m_nSymbols, m_symbolList, resp);
  
  // no response unless there was an error
  if (rc == -1)
    engineThread->respond(resp);
  else
    delete resp;
  
  return (rc == 0);
}

//------------------------------------------------------------

CEngineRequestSetSites::CEngineRequestSetSites(mediate_site_t *siteList, int nSites) :
  CEngineRequest(eEngineRequestSetSitesType),
  m_siteList(siteList),
  m_nSites(nSites)
{
  // takes ownership of the siteList
}

CEngineRequestSetSites::~CEngineRequestSetSites()
{
  delete [] m_siteList;
}

bool CEngineRequestSetSites::process(CEngineThread *engineThread)
{
  // process is called from the thread and drives the engine through the
  // mediator interface.

  CEngineResponse *resp = new CEngineResponseMessage;

  int rc = mediateRequestSetSites(engineThread->engineContext(),
				  m_nSites, m_siteList, resp);

  // no response unless there was an error
  if (rc == -1)
    engineThread->respond(resp);
  else
    delete resp;
  
  return (rc == 0);
}

//------------------------------------------------------------

CEngineRequestBeginBrowseFile::CEngineRequestBeginBrowseFile(const QString &fileName) :
  CEngineRequest(eEngineRequestBeginBrowseFileType),
  m_fileName(fileName)
{
}

CEngineRequestBeginBrowseFile::~CEngineRequestBeginBrowseFile()
{
}

bool CEngineRequestBeginBrowseFile::process(CEngineThread *engineThread)
{
  TRACE("CEngineRequestBeginBrowseFile" << m_fileName.toStdString());

  // open the file and get back the number of records (and calibration data?)

  // create a response as the handle
  CEngineResponseBeginAccessFile *resp = new CEngineResponseBeginAccessFile(m_fileName);

  int rc = mediateRequestBeginBrowseSpectra(engineThread->engineContext(),
					    m_fileName.toAscii().constData(), resp);

  resp->setNumberOfRecords(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestBrowseNextRecord::CEngineRequestBrowseNextRecord() :
  CEngineRequest(eEngineRequestBrowseNextRecordType)
{
}

CEngineRequestBrowseNextRecord::~CEngineRequestBrowseNextRecord()
{
}

bool CEngineRequestBrowseNextRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseAccessRecord *resp = new CEngineResponseAccessRecord;

  int rc = mediateRequestNextMatchingBrowseSpectrum(engineThread->engineContext(),
						    resp);

  resp->setRecordNumber(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestBrowseSpecificRecord::CEngineRequestBrowseSpecificRecord(int recordNumber) :
  CEngineRequest(eEngineRequestBrowseSpecificRecordType),
  m_recordNumber(recordNumber)
{
}

CEngineRequestBrowseSpecificRecord::~CEngineRequestBrowseSpecificRecord()
{
}

bool CEngineRequestBrowseSpecificRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseAccessRecord *resp = new CEngineResponseAccessRecord;

  int rc = mediateRequestGotoSpectrum(engineThread->engineContext(),
				      m_recordNumber, resp);
  
  if (rc > 0) {
    // successfully positioned .. now browse
    rc = mediateRequestNextMatchingBrowseSpectrum(engineThread->engineContext(),
						  resp);

    resp->setRecordNumber(rc); // -1 if an error occurred
  }

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestEndBrowseFile::CEngineRequestEndBrowseFile() :
  CEngineRequest(eEngineRequestEndBrowseFileType)
{
}

CEngineRequestEndBrowseFile::~CEngineRequestEndBrowseFile()
{
}

bool CEngineRequestEndBrowseFile::process(CEngineThread *engineThread)
{
  TRACE("CEngineRequestEndBrowseFile");

  CEngineResponseEndAccessFile *resp = new CEngineResponseEndAccessFile;

  int rc = mediateRequestEndBrowseSpectra(engineThread->engineContext(), resp);

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestBeginAnalyseFile::CEngineRequestBeginAnalyseFile(const QString &fileName) :
  CEngineRequest(eEngineRequestBeginAnalyseFileType),
  m_fileName(fileName)
{
}

CEngineRequestBeginAnalyseFile::~CEngineRequestBeginAnalyseFile()
{
}

bool CEngineRequestBeginAnalyseFile::process(CEngineThread *engineThread)
{
  // open the file and get back the number of records (and calibration data?)
  TRACE2("BAF " << m_fileName.toStdString());

  // create a response as the handle
  CEngineResponseBeginAccessFile *resp = new CEngineResponseBeginAccessFile(m_fileName);

  int rc = mediateRequestBeginAnalyseSpectra(engineThread->engineContext(),
					     m_fileName.toAscii().constData(), resp);

  resp->setNumberOfRecords(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestAnalyseNextRecord::CEngineRequestAnalyseNextRecord() :
  CEngineRequest(eEngineRequestAnalyseNextRecordType)
{
}

CEngineRequestAnalyseNextRecord::~CEngineRequestAnalyseNextRecord()
{
}

bool CEngineRequestAnalyseNextRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  TRACE2("ANR");

  CEngineResponseAccessRecord *resp = new CEngineResponseAccessRecord;

  int rc = mediateRequestNextMatchingAnalyseSpectrum(engineThread->engineContext(),
						     resp);

  resp->setRecordNumber(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestAnalyseSpecificRecord::CEngineRequestAnalyseSpecificRecord(int recordNumber) :
  CEngineRequest(eEngineRequestAnalyseSpecificRecordType),
  m_recordNumber(recordNumber)
{
}

CEngineRequestAnalyseSpecificRecord::~CEngineRequestAnalyseSpecificRecord()
{
}

bool CEngineRequestAnalyseSpecificRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseAccessRecord *resp = new CEngineResponseAccessRecord;

  int rc = mediateRequestGotoSpectrum(engineThread->engineContext(),
				      m_recordNumber, resp);
  
  if (rc > 0) {
    // successfully positioned .. now analyse
    rc = mediateRequestNextMatchingAnalyseSpectrum(engineThread->engineContext(),
						   resp);

    resp->setRecordNumber(rc); // -1 if an error occurred
  }

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestEndAnalyseFile::CEngineRequestEndAnalyseFile() :
  CEngineRequest(eEngineRequestEndAnalyseFileType)
{
}

CEngineRequestEndAnalyseFile::~CEngineRequestEndAnalyseFile()
{
}

bool CEngineRequestEndAnalyseFile::process(CEngineThread *engineThread)
{
  TRACE2("EAF");

  CEngineResponseEndAccessFile *resp = new CEngineResponseEndAccessFile;

  int rc = mediateRequestEndAnalyseSpectra(engineThread->engineContext(), resp);

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestBeginCalibrateFile::CEngineRequestBeginCalibrateFile(const QString &fileName) :
  CEngineRequest(eEngineRequestBeginCalibrateFileType),
  m_fileName(fileName)
{
}

CEngineRequestBeginCalibrateFile::~CEngineRequestBeginCalibrateFile()
{
}

bool CEngineRequestBeginCalibrateFile::process(CEngineThread *engineThread)
{
  // open the file and get back the number of records (and calibration data?)

  // create a response as the handle
  CEngineResponseBeginAccessFile *resp = new CEngineResponseBeginAccessFile(m_fileName);

  int rc = mediateRequestBeginCalibrateSpectra(engineThread->engineContext(),
					    m_fileName.toAscii().constData(), resp);

  resp->setNumberOfRecords(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestCalibrateNextRecord::CEngineRequestCalibrateNextRecord() :
  CEngineRequest(eEngineRequestCalibrateNextRecordType)
{
}

CEngineRequestCalibrateNextRecord::~CEngineRequestCalibrateNextRecord()
{
}

bool CEngineRequestCalibrateNextRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseAccessRecord *resp = new CEngineResponseAccessRecord;

  int rc = mediateRequestNextMatchingCalibrateSpectrum(engineThread->engineContext(),
						    resp);

  resp->setRecordNumber(rc); // -1 if an error occurred

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestCalibrateSpecificRecord::CEngineRequestCalibrateSpecificRecord(int recordNumber) :
  CEngineRequest(eEngineRequestCalibrateSpecificRecordType),
  m_recordNumber(recordNumber)
{
}

CEngineRequestCalibrateSpecificRecord::~CEngineRequestCalibrateSpecificRecord()
{
}

bool CEngineRequestCalibrateSpecificRecord::process(CEngineThread *engineThread)
{
  // create a response as the handle
  CEngineResponseAccessRecord *resp = new CEngineResponseAccessRecord;

  int rc = mediateRequestGotoSpectrum(engineThread->engineContext(),
				      m_recordNumber, resp);
  
  if (rc > 0) {
    // successfully positioned .. now analyse
    rc = mediateRequestNextMatchingCalibrateSpectrum(engineThread->engineContext(),
						   resp);

    resp->setRecordNumber(rc); // -1 if an error occurred
  }

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}

//------------------------------------------------------------

CEngineRequestEndCalibrateFile::CEngineRequestEndCalibrateFile() :
  CEngineRequest(eEngineRequestEndCalibrateFileType)
{
}

CEngineRequestEndCalibrateFile::~CEngineRequestEndCalibrateFile()
{
}

bool CEngineRequestEndCalibrateFile::process(CEngineThread *engineThread)
{
  CEngineResponseEndAccessFile *resp = new CEngineResponseEndAccessFile;

  int rc = mediateRequestEndCalibrateSpectra(engineThread->engineContext(), resp);

  // post the response
  engineThread->respond(resp);

  return (rc != -1);
}


CEngineRequestViewCrossSections::CEngineRequestViewCrossSections(double minWavelength, double maxWavelength,
                                                                 int nFiles, char **filenames) :
  CEngineRequest(eEngineRequestViewCrossSectionsType),
  m_minWavelength(minWavelength),
  m_maxWavelength(maxWavelength),
  m_nFiles(nFiles),
  m_filenames(filenames)
{
}

CEngineRequestViewCrossSections::~CEngineRequestViewCrossSections()
{
  for (int i=0; i<m_nFiles; ++i) {
    delete [] m_filenames[i];
  }
  delete [] m_filenames;
}

bool CEngineRequestViewCrossSections::process(CEngineThread *engineThread)
{
  CEngineResponseTool *resp = new CEngineResponseTool;
  
  int rc = mediateRequestViewCrossSections(engineThread->engineContext(),
                                           m_minWavelength, m_maxWavelength,
                                           m_nFiles, m_filenames, resp);
  
  // post the response
  engineThread->respond(resp);
  
  return (rc != -1);
}


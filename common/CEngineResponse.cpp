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


#include "CEngineResponse.h"
#include "CEngineController.h"

//------------------------------------------------------------

CEngineResponse::CEngineResponse(CEngineResponse::ResponseType type) :
  m_type(type),
  m_highestErrorLevel(0)
{
}

CEngineResponse::~CEngineResponse()
{
}

void CEngineResponse::addErrorMessage(const QString &tag, const QString &msg, int errorLevel)
{
  m_errorMessages.push_back(CEngineError(tag, msg, errorLevel));

  if (errorLevel > m_highestErrorLevel)
    m_highestErrorLevel = errorLevel;
}

bool CEngineResponse::processErrors(CEngineController *engineController)
{
  if (hasErrors()) {
    // send the set of messages to the controller 
    engineController->notifyErrorMessages(m_highestErrorLevel, m_errorMessages);

    return hasFatalError();
  }
  
  return false;
}

//------------------------------------------------------------

CEngineResponseMessage::CEngineResponseMessage() :
  CEngineResponse(eEngineResponseMessageType)
{
}

CEngineResponseMessage::~CEngineResponseMessage()
{
}

void CEngineResponseMessage::process(CEngineController *engineController)
{
  processErrors(engineController);
}

//------------------------------------------------------------

CEngineResponseVisual::CEngineResponseVisual(CEngineResponse::ResponseType type) :
  CEngineResponse(type)
{
}

CEngineResponseVisual::~CEngineResponseVisual()
{
  while (!m_plotDataList.isEmpty()) {
    delete m_plotDataList.front().data;
    m_plotDataList.pop_front();
  }
}

void CEngineResponseVisual::process(CEngineController *engineController)
{
  // consider the error messages first - if fatal stop here
  if (processErrors(engineController))
    return;

  if (!m_cellList.isEmpty() || !m_plotDataList.isEmpty()) {
    engineController->notifyTableData(m_cellList);
    engineController->notifyPlotData(m_plotDataList, m_titleList);
  }
}

void CEngineResponseVisual::addDataSet(int pageNumber, const CPlotDataSet *dataSet)
{
  m_plotDataList.push_back(SPlotData(pageNumber, dataSet));
}

void CEngineResponseVisual::addPageTitleAndTag(int pageNumber, const QString &title, const QString &tag)
{
  m_titleList.push_back(STitleTag(pageNumber, title, tag));
}

void CEngineResponseVisual::addCell(int pageNumber, int row, int col, const QVariant &data)
{
  m_cellList.push_back(SCell(pageNumber, row, col, data));
}

//------------------------------------------------------------

CEngineResponseBeginAccessFile::CEngineResponseBeginAccessFile(const QString &fileName) :
  CEngineResponseVisual(eEngineResponseBeginAccessFileType),
  m_fileName(fileName),
  m_numberOfRecords(-1)
{
}

CEngineResponseBeginAccessFile::~CEngineResponseBeginAccessFile()
{
}

void CEngineResponseBeginAccessFile::process(CEngineController *engineController)
{
  // consider the error messages first - if fatal stop here
  if (processErrors(engineController))
    return;

  if (!m_cellList.isEmpty() || !m_plotDataList.isEmpty()) {
    engineController->notifyTableData(m_cellList);
    engineController->notifyPlotData(m_plotDataList, m_titleList);
  }

  if (m_numberOfRecords > 0) {
    // calibration data ... TODO ...
    
    engineController->notifyReadyToNavigateRecords(m_fileName, m_numberOfRecords);
    // wait for the request to process a record ...
  }
}

void CEngineResponseBeginAccessFile::setNumberOfRecords(int numberOfRecords)
{
  m_numberOfRecords = numberOfRecords;
}

//------------------------------------------------------------

CEngineResponseSpecificRecord::CEngineResponseSpecificRecord(CEngineResponse::ResponseType type) :
  CEngineResponseVisual(type),
  m_recordNumber(-1)
{
}

CEngineResponseSpecificRecord::~CEngineResponseSpecificRecord()
{
}

void CEngineResponseSpecificRecord::process(CEngineController *engineController)
{
  // consider the error messages first - if fatal stop here
  if (processErrors(engineController))
    return;

  // TODO
  if (m_recordNumber == 0) {
    // EOF
    engineController->notifyEndOfRecords();
  }
  else if (m_recordNumber > 0) {
    // display ... table data MUST be before plot data
    engineController->notifyTableData(m_cellList);
    engineController->notifyPlotData(m_plotDataList, m_titleList);

    engineController->notifyCurrentRecord(m_recordNumber);
  }
  else {
    // some error condition ... TODO
  }
}

void CEngineResponseSpecificRecord::setRecordNumber(int recordNumber)
{
  m_recordNumber = recordNumber;
}

//------------------------------------------------------------

CEngineResponseAccessRecord::CEngineResponseAccessRecord() :
  CEngineResponseSpecificRecord(eEngineResponseAccessRecordType)
{
}

CEngineResponseAccessRecord::~CEngineResponseAccessRecord()
{
}

//------------------------------------------------------------

CEngineResponseSet::CEngineResponseSet() :
  CEngineResponseVisual(eEngineResponseSetType)
{
}

CEngineResponseSet::~CEngineResponseSet()
{
}

//------------------------------------------------------------

CEngineResponseTool::CEngineResponseTool() :
  CEngineResponseVisual(eEngineResponseToolType)
{
}

CEngineResponseTool::~CEngineResponseTool()
{
}

//------------------------------------------------------------


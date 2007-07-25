
#include "CEngineResponse.h"
#include "CEngineController.h"

//------------------------------------------------------------

CEngineResponse::CEngineResponse(int type) :
  m_type(type)
{
}

CEngineResponse::~CEngineResponse()
{
}

void CEngineResponse::addErrorMessage(const QString &msg)
{
  m_errorMessages.push_back(msg);
}

//------------------------------------------------------------

CEngineResponseSetProject::CEngineResponseSetProject() :
  CEngineResponse(cEngineResponseSetProjectType)
{
}

CEngineResponseSetProject::~CEngineResponseSetProject()
{
}

void CEngineResponseSetProject::process(CEngineController *engineController)
{
  // only called if there are no error messages ...
  engineController->notifySetProject();
}

//------------------------------------------------------------

CEngineResponseBeginBrowseFile::CEngineResponseBeginBrowseFile(const QString &fileName) :
  CEngineResponse(cEngineResponseBeginBrowseFileType),
  m_fileName(fileName),
  m_numberOfRecords(-1)
{
}

CEngineResponseBeginBrowseFile::~CEngineResponseBeginBrowseFile()
{
}

void CEngineResponseBeginBrowseFile::process(CEngineController *engineController)
{
  // TODO
  if (m_numberOfRecords > 0) {
    // calibration data ... TODO ...

    engineController->notifyReadyToNavigateRecords(m_numberOfRecords);
    // wait for the request to process a record ...
  }
}

void CEngineResponseBeginBrowseFile::setNumberOfRecords(int numberOfRecords)
{
  m_numberOfRecords = numberOfRecords;
}

//------------------------------------------------------------

CEngineResponseBrowseRecord::CEngineResponseBrowseRecord() :
  CEngineResponse(cEngineResponseBrowseRecordType),
  m_recordNumber(-1)
{
}

CEngineResponseBrowseRecord::~CEngineResponseBrowseRecord()
{
  // should have been emptied in the trasfer to the controller

  while (!m_plotDataBuckets.isEmpty()) {
    delete m_plotDataBuckets.front().data;
    m_plotDataBuckets.pop_front();
  }
}

void CEngineResponseBrowseRecord::process(CEngineController *engineController)
{
  // TODO
  if (m_recordNumber == 0) {
    // EOF
    engineController->notifyEndOfRecords();
  }
  else if (m_recordNumber > 0) {
    // display data ...
    engineController->notifyPlotData(m_plotDataBuckets);

    engineController->notifyCurrentRecord(m_recordNumber);
  }
  else {
    // some error condition ... TODO
  }
}

void CEngineResponseBrowseRecord::setRecordNumber(int recordNumber)
{
  m_recordNumber = recordNumber;
}

void CEngineResponseBrowseRecord::addDataSet(int pageNumber, const CPlotDataSet *dataSet)
{
  m_plotDataBuckets.push_back(SPlotDataBucket(pageNumber, dataSet));
}

void CEngineResponseBrowseRecord::addCell(int pageNumber, int row, int col,
					  const QVariant &data)
{
  m_cells.push_back(SCell(pageNumber, row, col, data));
}

//------------------------------------------------------------


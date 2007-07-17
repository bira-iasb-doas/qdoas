
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
  // TODO ...
}

void CEngineResponseBeginBrowseFile::setNumberOfRecords(int numberOfRecords)
{
  m_numberOfRecords = numberOfRecords;
}

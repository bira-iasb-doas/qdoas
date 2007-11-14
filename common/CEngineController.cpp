
#include "CEngineController.h"

CEngineController::CEngineController()
{
}

void CEngineController::notifyNumberOfFiles(int nFiles)
{
}

void CEngineController::notifyCurrentFile(int fileNumber)
{
}

void CEngineController::notifyReadyToNavigateRecords(const QString &filename, int numberOfRecords)
{
}

void CEngineController::notifyCurrentRecord(int recordNumber)
{
}

void CEngineController::notifyEndOfRecords(void)
{
}

void CEngineController::notifyGotoRecord(int recordNumber)
{
}

void CEngineController::notifyPlotData(QList<SPlotData> &plotDataList, QList<STitleTag> &titleList)
{
}

void CEngineController::notifyTableData(QList<SCell> &cellList)
{
}

void CEngineController::notifyErrorMessages(int highestErrorLevel, const QList<CEngineError> &errorMessages)
{
}

void CEngineController::notifyEndAccessFile(void)
{
}



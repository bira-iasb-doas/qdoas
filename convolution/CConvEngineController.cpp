
#include <QTextStream>

#include "CConvEngineController.h"

CConvEngineController::CConvEngineController(QObject *parent) :
  QObject(parent),
  CEngineController()
{
}

CConvEngineController::~CConvEngineController()
{
}

void CConvEngineController::notifyPlotData(QList<SPlotData> &plotDataList, QList<STitleTag> &titleList)
{
  // TODO
}

void CConvEngineController::notifyErrorMessages(int highestErrorLevel, const QList<CEngineError> &errorMessages)
{
  // format each into a message text and put in a single string for posting ...
  QString msg;
  QTextStream stream(&msg);

  QList<CEngineError>::const_iterator it = errorMessages.begin();
  while (it != errorMessages.end()) {
    // one message per line
    switch (it->errorLevel()) {
    case InformationEngineError:
      stream << "INFO    (";
      break;
    case WarningEngineError:
      stream << "WARNING (";
      break;
    case FatalEngineError:
      stream << "FATAL   (";
      break;
    }

    stream << it->tag() << ") " << it->message() << ".\n";

    ++it;
  }

  emit signalErrorMessages(highestErrorLevel, msg);  
}


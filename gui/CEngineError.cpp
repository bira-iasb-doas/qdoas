
#include "CEngineError.h"


CEngineError::CEngineError(const QString &tag, const QString &msg, int errorLevel) :
  m_tag(tag),
  m_message(msg),
  m_errorLevel(errorLevel)
{
}

CEngineError::~CEngineError()
{
}


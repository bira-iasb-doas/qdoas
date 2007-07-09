
#include "CWEditor.h"

CWEditor::CWEditor(QWidget *parent) :
  QFrame(parent),
  m_lastNotification(false),
  m_captionStr("No Title")
{
  m_contextTag.sprintf("%p", this); // unique by default
}

CWEditor::~CWEditor()
{
}

const QString& CWEditor::editCaption(void) const
{
  return m_captionStr;
}

const QString& CWEditor::editContextTag(void) const
{
  return m_contextTag;
}

void CWEditor::actionCancel()
{
  // default behaviour is do nothing
}

bool CWEditor::actionOk()
{
  // default behaviour is do nothing - but accepts the actions
  return true;
}

void CWEditor::actionHelp()
{
  // default behaviour is do nothing
}

void CWEditor::notifyAcceptActionOk(bool canDoOk)
{
  if (canDoOk != m_lastNotification) {
    m_lastNotification = canDoOk;
    emit signalAcceptOk(m_lastNotification);
  }
}

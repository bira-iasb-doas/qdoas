
#include "CWEditor.h"

CWEditor::CWEditor(QWidget *parent) :
  QFrame(parent),
  m_captionStr("No Title"),
  m_lastNotification(false)
{
  m_contextTag.setNum(reinterpret_cast<unsigned long>(this)); // unique by default
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

void CWEditor::actionCancel(void)
{
  // default behaviour is do nothing
}

bool CWEditor::actionOk(void)
{
  // default behaviour is do nothing - but accepts the actions
  return true;
}

void CWEditor::actionHelp(void)
{
  // default behaviour is do nothing
}

void CWEditor::takeFocus(void)
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

void CWEditor::shortcutActionOk(void)
{
  // allows the editor to effectively click the Ok button programatically.
  if (m_lastNotification)
    emit signalShortcutActionOk();
}

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

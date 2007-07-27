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


#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

#include "CWProjectTree.h"
#include "CWProjectAnalysisWindowNameEditor.h"


CWProjectAnalysisWindowNameEditor::CWProjectAnalysisWindowNameEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                                                                     bool newAnalysisWindow, QWidget *parent) :
  CWEditor(parent),
  m_projectTree(projectTree),
  m_newAnalysisWindow(newAnalysisWindow)
{
  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  mainLayout->setMargin(25);
  mainLayout->setSpacing(5);;
  
  if (newAnalysisWindow)
    mainLayout->addWidget(new QLabel("Enter a name for the new Analysis Window", this));
  else
    mainLayout->addWidget(new QLabel("Modify the Analysis Window name", this));

  m_analysisWindowName = new QLineEdit(this);
  mainLayout->addWidget(m_analysisWindowName);

  if (m_newAnalysisWindow)
    m_captionStr = "Create new Analysis Window in ";
  else
    m_captionStr = "Rename Analysis Window ";
    
  m_contextTag.clear();

  // build a path from item - will use this to locate the target point in the tree on 'Ok'
  while (item != NULL) {
    QString tmpStr(item->text(0));

    m_path.push_front(tmpStr);
    m_contextTag.prepend(tmpStr);
    m_contextTag.prepend(':');

    item = item->parent();
  }
  m_captionStr += m_contextTag;
  m_contextTag += "-AnalysisWindowEditor";

  // if renaming
  if (!m_newAnalysisWindow && !m_path.isEmpty())
    m_analysisWindowName->setText(m_path.back());

  connect(m_analysisWindowName, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotNameChanged(const QString &)));
  connect(m_analysisWindowName, SIGNAL(returnPressed()),
          this, SLOT(slotReturnPressed()));

}

CWProjectAnalysisWindowNameEditor::~CWProjectAnalysisWindowNameEditor()
{
}

bool CWProjectAnalysisWindowNameEditor::actionOk()
{
  // use the path to locate the correct place in the tree

  if (m_analysisWindowName->text().isEmpty())
    return false;

  QString msg;

  QTreeWidgetItem *item = m_projectTree->locateByPath(m_path);
  if (item) {
    // still a valid point in the tree
    if (m_newAnalysisWindow)
      msg = m_projectTree->editInsertNewAnalysisWindow(item, m_analysisWindowName->text());
    else
      msg = m_projectTree->editRenameAnalysisWindow(item, m_analysisWindowName->text());
    
    if (msg.isNull())
      return true;

  }
  else {
    // no longer exists ...
    msg = QString("Could not locate the parent in the project tree.");
  }

  // fall through failure ...
  QMessageBox::information(this,
                           m_newAnalysisWindow ? "Insert Analysis Window" : "Rename Analysis Window",
                           msg);
  return false;
}

void CWProjectAnalysisWindowNameEditor::actionHelp(void)
{
}

void CWProjectAnalysisWindowNameEditor::takeFocus(void)
{
  m_analysisWindowName->setFocus(Qt::OtherFocusReason);
}

void CWProjectAnalysisWindowNameEditor::slotNameChanged(const QString &text)
{
  notifyAcceptActionOk(!text.isEmpty());
}

void CWProjectAnalysisWindowNameEditor::slotReturnPressed()
{
  shortcutActionOk();
}


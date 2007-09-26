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


#include <QMenu>
#include <QShowEvent>
#include <QContextMenuEvent>
#include <QList>

#include "CWUserSymbolTree.h"
#include "CPreferences.h"
#include "CWActiveContext.h"
#include "CWSymbolEditor.h"

#include "debugutil.h"

const int cUserSymbolTreeGeneralMode = 47;

CWUserSymbolTree::CWUserSymbolTree(CWActiveContext *activeContext, QWidget *parent) :
  QTreeWidget(parent),
  CSymbolObserver(),
  m_activeContext(activeContext)
{
  QStringList labelList;
  labelList << "Name" << "Description";
  setHeaderLabels(labelList);

  QList<int> widthList;
  widthList.push_back(130);
  widthList.push_back(130);
  
  widthList = CPreferences::instance()->columnWidthList("UserSymbolTree", widthList);

  for (int i=0; i<2; ++i) {
    setColumnWidth(i, widthList.at(i));
  }
}

CWUserSymbolTree::~CWUserSymbolTree()
{
}

void CWUserSymbolTree::savePreferences(void)
{
  QList<int> widthList;

  for (int i=0; i<2; ++i)
    widthList.push_back(columnWidth(i));

  CPreferences::instance()->setColumnWidthList("UserSymbolTree", widthList);
}

void CWUserSymbolTree::updateNewSymbol(const QString &newSymbolName)
{
  QString description = CWorkSpace::instance()->findSymbol(newSymbolName);

  if (!description.isNull()) {

    QStringList labelList;
    labelList << newSymbolName << description;

    QTreeWidgetItem *userSymbolItem = new  QTreeWidgetItem(labelList);

    addTopLevelItem(userSymbolItem);
  }
}

void CWUserSymbolTree::updateModifySymbol(const QString &symbolName)
{
  TRACE("TODO");
}

void CWUserSymbolTree::updateDeleteSymbol(const QString &symbolName)
{
  QString description = CWorkSpace::instance()->findSymbol(symbolName);

  if (!description.isNull()) {

    QTreeWidgetItem *symbolItem;
    int i = 0;
    
    while ((symbolItem = topLevelItem(i)) != NULL && symbolItem->text(0) != symbolName) ++i;
    if (symbolItem != NULL) {
      delete takeTopLevelItem(i);
    }
  }
}
  
void CWUserSymbolTree::showEvent(QShowEvent *e)
{
  QTreeWidget::showEvent(e);

  emit signalWidthModeChanged(cUserSymbolTreeGeneralMode);
}

void CWUserSymbolTree::contextMenuEvent(QContextMenuEvent *e)
{
  // create a popup menu
  QMenu menu;

  menu.addAction("Insert...", this, SLOT(slotAddNewSymbol()));
  if (!selectedItems().isEmpty()) {
    menu.addAction("Edit...", this, SLOT(slotEditSymbol()));
    menu.addSeparator();
    menu.addAction("Delete", this, SLOT(slotDeleteSymbol()));
  }

  menu.exec(e->globalPos()); // a slot will do the rest if appropriate
}

void CWUserSymbolTree::slotAddNewSymbol()
{
  CWSymbolEditor *symbolEdit = new CWSymbolEditor;
  m_activeContext->addEditor(symbolEdit);
}

void CWUserSymbolTree::slotEditSymbol()
{
  // Ok for single, multi and no selection
  QList<QTreeWidgetItem*> selection = selectedItems();

  QList<QTreeWidgetItem*>::iterator it = selection.begin();
  while (it != selection.end()) {
    //CWSymbolEditor *symbolEdit = new CWSymbolEditor(*it);
    //m_activeContext->addEditor(symbolEdit);
    ++it;
  }
}

void CWUserSymbolTree::slotDeleteSymbol()
{
  QTreeWidgetItem *item;

  // Ok for single, multi and no selection
  QList<QTreeWidgetItem*> selection = selectedItems();

  QList<QTreeWidgetItem*>::iterator it = selection.begin();
  while (it != selection.end()) {
    item = *it;
    // get the top-level item
    while (item->parent() != NULL) {
      item = item->parent();
    }

    QString symbolName = item->text(0);
    ++it;

    CWorkSpace::instance()->destroySymbol(symbolName);
  }
}


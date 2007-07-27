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

#include <cstdio> // TODO

#include <QMenu>
#include <QShowEvent>
#include <QContextMenuEvent>
#include <QTextStream>

#include "CWSiteTree.h"
#include "CWSiteEditor.h"
#include "CWActiveContext.h"

#include "debugutil.h"

const int cSiteTreeGeneralMode   = 37;

CWSiteTree::CWSiteTree(CWActiveContext *activeContext, QWidget *parent) :
  QTreeWidget(parent),
  m_activeContext(activeContext)
{
  QStringList labelList;
  labelList << "Observation Sites" << "Details";

  setHeaderLabels(labelList);
}

CWSiteTree::~CWSiteTree()
{
}

void CWSiteTree::addNewSite(const QString &siteName, const QString &abreviation,
                            double longitude, double latitude, double altitude)
{
  QString tmpStr;
  QStringList labelList;
  QTextStream tmpStream(&tmpStr);

  QTreeWidgetItem *siteItem = new  QTreeWidgetItem(QStringList(siteName));

  // add Children for the site details

  // Abbreviation
  labelList << "Abbreviaton" << abreviation;
  new  QTreeWidgetItem(siteItem, labelList);
  labelList.clear();

  tmpStream.setRealNumberNotation(QTextStream::FixedNotation);
  tmpStream.setRealNumberPrecision(3);

  // Longitude  
  tmpStream << longitude;
  labelList << "Longitude" << tmpStr; 
  new  QTreeWidgetItem(siteItem, labelList);
  labelList.clear();
  tmpStr.clear();

  // Latitude
  tmpStream << latitude;
  labelList << "Latitude" << tmpStr;
  new  QTreeWidgetItem(siteItem, labelList);
  labelList.clear();
  tmpStr.clear();

  // Altitude
  tmpStream << altitude;
  labelList << "Altitude" << tmpStr;
  new  QTreeWidgetItem(siteItem, labelList);
  labelList.clear();
  tmpStr.clear();

  addTopLevelItem(siteItem);
}

void CWSiteTree::modifySite(const QString &siteName, const QString &abreviation,
                            double longitude, double latitude, double altitude)
{
  int i;
  QTreeWidgetItem *siteItem;

  i = 0;
  while ((siteItem = topLevelItem(i)) != NULL && siteItem->text(0) != siteName) ++i;
  if (siteItem != NULL) {
    // located the existing item - update it's children

    assert(siteItem->childCount() == 4);

    QTreeWidgetItem *child;
    QString tmpStr;
    QTextStream tmpStream(&tmpStr); 

    tmpStream.setRealNumberNotation(QTextStream::FixedNotation);
    tmpStream.setRealNumberPrecision(3);

    // abbrev.
    child = siteItem->child(0);
    child->setText(1, abreviation);
    // long.
    tmpStream << longitude;
    child = siteItem->child(1);
    child->setText(1, tmpStr);
    tmpStr.clear();

    // lat.
    tmpStream << latitude;
    child = siteItem->child(2);
    child->setText(1, tmpStr);
    tmpStr.clear();
    // alt.
    tmpStream << altitude;
    child = siteItem->child(3);
    child->setText(1, tmpStr);
    tmpStr.clear();
  }


}

void CWSiteTree::showEvent(QShowEvent *e)
{
  QTreeWidget::showEvent(e);

  emit signalWidthModeChanged(cSiteTreeGeneralMode);
}

void CWSiteTree::contextMenuEvent(QContextMenuEvent *e)
{
  // create a popup menu
  QMenu menu;

  menu.addAction("Insert...", this, SLOT(slotAddNewSite()));
  if (!selectedItems().isEmpty()) {
    menu.addAction("Edit...", this, SLOT(slotEditSite()));
    menu.addSeparator();
    menu.addAction("Delete", this, SLOT(slotDeleteSite()));
  }

  menu.exec(e->globalPos()); // a slot will do the rest if appropriate
}

void CWSiteTree::slotAddNewSite()
{
  CWSiteEditor *siteEdit = new  CWSiteEditor(this);
  m_activeContext->addEditor(siteEdit);
}

void CWSiteTree::slotEditSite()
{
  // Ok for single, multi and no selection
  QList<QTreeWidgetItem*> selection = selectedItems();

  QList<QTreeWidgetItem*>::iterator it = selection.begin();
  while (it != selection.end()) {
    CWSiteEditor *siteEdit = new  CWSiteEditor(this, *it);
    m_activeContext->addEditor(siteEdit);
    ++it;
  }
}

void CWSiteTree::slotDeleteSite()
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
    // take it from the tree and delete it.
    item = takeTopLevelItem(indexOfTopLevelItem(item));
    delete item;
    ++it;
  }
}

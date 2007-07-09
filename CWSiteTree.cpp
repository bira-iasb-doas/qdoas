#include <cstdio> // TODO

#include <QMenu>
#include <QShowEvent>
#include <QContextMenuEvent>

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

  QTreeWidgetItem *siteItem = new  QTreeWidgetItem(QStringList(siteName));

  // add Children for the site details

  // Abbreviation
  labelList << "Abbreviaton" << abreviation;
  new  QTreeWidgetItem(siteItem, labelList);
  labelList.clear();

  // Longitude
  labelList << "Longitude" << tmpStr.sprintf("%8.3f", longitude); 
  new  QTreeWidgetItem(siteItem, labelList);
  labelList.clear();

  // Latitude
  labelList << "Latitude" << tmpStr.sprintf("%8.3f", latitude); 
  new  QTreeWidgetItem(siteItem, labelList);
  labelList.clear();

  // Altitude
  labelList << "Altitude" << tmpStr.sprintf("%9.3f", altitude); 
  new  QTreeWidgetItem(siteItem, labelList);
  labelList.clear();

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
    // abbrev.
    child = siteItem->child(0);
    child->setText(1, abreviation);
    // long.
    child = siteItem->child(1);
    child->setText(1, tmpStr.sprintf("%8.3f", longitude));
    // lat.
    child = siteItem->child(2);
    child->setText(1, tmpStr.sprintf("%8.3f", latitude));
    // long.
    child = siteItem->child(3);
    child->setText(1, tmpStr.sprintf("%9.3f", altitude));
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

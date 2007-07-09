
#include <QShowEvent>

#include "CWUserSymbolTree.h"

const int cUserSymbolTreeGeneralMode = 47;

CWUserSymbolTree::CWUserSymbolTree(QWidget *parent) :
  QTreeWidget(parent)
{
  QStringList labelList;
  labelList << "Name" << "Description";
  setHeaderLabels(labelList);
}

CWUserSymbolTree::~CWUserSymbolTree()
{
}

void CWUserSymbolTree::addNewUserSymbol(const QString &userSymbolName, const QString &description)
{
  QStringList labelList;
  labelList << userSymbolName << description;

  QTreeWidgetItem *userSymbolItem = new  QTreeWidgetItem(labelList);

  addTopLevelItem(userSymbolItem);
}

void CWUserSymbolTree::showEvent(QShowEvent *e)
{
  QTreeWidget::showEvent(e);

  emit signalWidthModeChanged(cUserSymbolTreeGeneralMode);
}


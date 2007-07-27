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


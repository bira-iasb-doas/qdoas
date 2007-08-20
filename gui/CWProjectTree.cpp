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


#include <assert.h>

#include <QMenu>
#include <QDateTime>
#include <QKeyEvent>
#include <QShowEvent>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QRegExp>
#include <QDir>
#include <QTextStream>

#include "CWorkSpace.h"

#include "CWProjectTree.h"

#include "CWProjectNameEditor.h"
#include "CWProjectFolderNameEditor.h"
#include "CWProjectDirectoryEditor.h"
#include "CWProjectAnalysisWindowNameEditor.h"
#include "CWProjectPropertyEditor.h"

#include "CWActiveContext.h"

#include "CSession.h"
#include "RefCountPtr.h"

#include "CPreferences.h"

#include "debugutil.h"


const int cProjectTreeHideDetailMode   = 27;
const int cProjectTreeShowDetailMode   = 28;

const QRgb cDisabledTextColour         = 0xFFAAAAAA;
const QRgb cProjectTextColour          = 0xFFA93F26;

QIcon *CWProjectTree::m_windowIcon = NULL;
QIcon *CWProjectTree::m_folderIcon = NULL;
QIcon *CWProjectTree::m_directoryIcon = NULL;
QIcon *CWProjectTree::m_fileIcon = NULL;

CWProjectTree::CWProjectTree(CWActiveContext *activeContext, QWidget *parent) :
  QTreeWidget(parent),
  m_activeContext(activeContext)
{
  QStringList labelList;
  labelList << "Name" << "Size" << "Modified"; 
  setHeaderLabels(labelList);

  QList<int> widthList;
  widthList.push_back(260);
  widthList.push_back(90);
  widthList.push_back(160);

  widthList = CPreferences::instance()->columnWidthList("ProjectTree", widthList);

  for (int i=0; i<3; ++i) {
    setColumnWidth(i, widthList.at(i));
  }

  setSelectionMode(QAbstractItemView::ExtendedSelection);

  slotToggleDisplayDetails();
}

CWProjectTree::~CWProjectTree()
{
  if (m_windowIcon) {
    delete m_windowIcon;
    m_windowIcon = NULL;
  }
  if (m_folderIcon) {
    delete m_folderIcon;
    m_folderIcon = NULL;
  }
  if (m_directoryIcon) {
    delete m_directoryIcon;
    m_directoryIcon = NULL;
  }
  if (m_fileIcon) {
    delete m_fileIcon;
    m_fileIcon = NULL;
  }
}

void CWProjectTree::savePreferences(void)
{

  if (m_colWidthList.empty()) {
    QList<int> widthList;
    for (int i=0; i<3; ++i)
      widthList.push_back(columnWidth(i));

    CPreferences::instance()->setColumnWidthList("ProjectTree", widthList);
  }
  else
    CPreferences::instance()->setColumnWidthList("ProjectTree", m_colWidthList);
}

void CWProjectTree::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_D) {
    slotToggleDisplayDetails();
    e->accept();
  }
  else
    QTreeWidget::keyPressEvent(e);
}

void CWProjectTree::showEvent(QShowEvent *e)
{
  QTreeWidget::showEvent(e);

  emit signalWidthModeChanged(m_colWidthList.empty() ? cProjectTreeShowDetailMode : cProjectTreeHideDetailMode);
}

void CWProjectTree::contextMenuEvent(QContextMenuEvent *e)
{
  // create a popup menu
  QMenu menu;

  // always have at least 1 item selected ...
  QList<QTreeWidgetItem*> items = selectedItems();

  // Try and keep the order the same for all cases

  //------------------------------
  // Enable/Disable
  // New ...
  // Insert ...
  // Refresh
  //------------------------------
  // Run Analysis + Browse Spectra
  //------------------------------
  // Cut/Copy/Paste/Delete
  //------------------------------
  // Hide/Show Details
  //------------------------------

  if (items.count() > 1) {
    // multiple selection - run analysis - enable/disable/toggle - delete
    menu.addAction("Enable", this, SLOT(slotEnable()));
    menu.addAction("Disable", this, SLOT(slotDisable()));
    menu.addAction("Enable/Disable", this, SLOT(slotToggleEnable()));
    menu.addSeparator();
    menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()));
    menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()));
    menu.addSeparator();
    menu.addAction("Delete", this, SLOT(slotDeleteSelection()));
  }
  else if (items.count()) {
    // one item selected - what type is it?
    int itemType = items.front()->type();

    if (itemType == cSpectraDirectoryItemType) {
      // A Directory item
      CProjectTreeItem *projItem = static_cast<CProjectTreeItem*>(items.front());

      menu.addAction(projItem->isEnabled() ? "Disable" : "Enable", this,
                     SLOT(slotToggleEnable()));
      menu.addAction("Refresh", this, SLOT(slotRefreshDirectories()));
      menu.addSeparator();
      menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()));
      menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()));
      if (projItem->parent() && projItem->parent()->type() != cSpectraDirectoryItemType) {
        // Cant delete an item that is a child of a directory item
        menu.addSeparator();
        menu.addAction("Delete", this, SLOT(slotDeleteSelection()));
      }
    }
    else if (itemType == cSpectraFolderItemType) {
      // A Folder Item
      CProjectTreeItem *projItem = static_cast<CProjectTreeItem*>(items.front());
	  
      menu.addAction(projItem->isEnabled() ? "Disable" : "Enable", this,
                     SLOT(slotToggleEnable()));
      menu.addAction("Rename...", this, SLOT(slotRenameFolder()));
      menu.addAction("New Sub-Folder...", this, SLOT(slotCreateFolder()));
      menu.addAction("Insert Directory...", this, SLOT(slotInsertDirectory()));
      menu.addAction("Insert File...", this, SLOT(slotInsertFile()));
      menu.addSeparator();
      menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()));
      menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()));
      menu.addSeparator();
      menu.addAction("Delete", this, SLOT(slotDeleteSelection()));
    }
    else if (itemType == cSpectraFileItemType) {
      // A File Item
      CProjectTreeItem *projItem = static_cast<CProjectTreeItem*>(items.front());

      menu.addAction(projItem->isEnabled() ? "Disable" : "Enable", this,
                     SLOT(slotToggleEnable()));
      menu.addSeparator();
      menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()));
      menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()));
      if (projItem->parent() && projItem->parent()->type() != cSpectraDirectoryItemType) {
        // Cant delete an item that is a child of a directory item
        menu.addSeparator();
        menu.addAction("Delete", this, SLOT(slotDeleteSelection()));
      }
    }
    else if (itemType == cSpectraBranchItemType) {
      // A Spectra Branch (Raw Spectra)
      menu.addAction("New Folder...", this, SLOT(slotCreateFolder()));
      menu.addAction("Insert Directory...", this, SLOT(slotInsertDirectory()));
      menu.addAction("Insert File...", this, SLOT(slotInsertFile()));
      menu.addSeparator();
      menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()));
      menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()));
      menu.addSeparator();
      // cant remove this item - refers to all children
      menu.addAction("Delete All", this, SLOT(slotDeleteAllSpectra()));
    }
    else if (itemType == cAnalysisWindowBranchItemType) {
      // Analysis Window Branch
      menu.addAction("New Analysis Window...", this, SLOT(slotCreateAnalysisWindow()));
      menu.addSeparator();
      // cant remove this item - refers to all children
      menu.addAction("Delete All", this, SLOT(slotDeleteSelection()));
    }
    else if (itemType == cAnalysisWindowItemType) {
      // Analysis Window
      CProjectTreeItem *projItem = static_cast<CProjectTreeItem*>(items.front());

      menu.addAction(projItem->isEnabled() ? "Disable" : "Enable", this,
                     SLOT(slotToggleEnable()));
      menu.addSeparator();
      menu.addAction("Rename...", this, SLOT(slotRenameAnalysisWindow()));
      menu.addSeparator();
      menu.addAction("Delete", this, SLOT(slotDeleteSelection()));
    }
    else if (itemType == cProjectItemType) {
      // Project
      CProjectTreeItem *projItem = static_cast<CProjectTreeItem*>(items.front());

      menu.addAction(projItem->isEnabled() ? "Disable" : "Enable", this,
                     SLOT(slotToggleEnable()));
      menu.addAction("Rename...", this, SLOT(slotRenameProject()));
      menu.addAction("New Project...", this, SLOT(slotCreateProject()));
      menu.addAction("Properties...", this, SLOT(slotEditProject()));
      menu.addSeparator();
      menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()));
      menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()));
      menu.addSeparator();
      menu.addAction("Delete", this, SLOT(slotDeleteSelection()));
    }
      
  }
  else {
    // must be an empty tree
    menu.addAction("New Project...", this, SLOT(slotCreateProject()));
  }


  // last item - hide/show details
  menu.addSeparator();  
  menu.addAction(m_colWidthList.isEmpty() ? "Hide Details" : "Show Details",
                 this, SLOT(slotToggleDisplayDetails()));
    
  menu.exec(e->globalPos()); // a slot will do the rest if appropriate
}

QTreeWidgetItem *CWProjectTree::locateByPath(const QStringList &path)
{
  QTreeWidgetItem *item = NULL;
  QTreeWidgetItem *p = NULL;
  int i;
  QList<QString>::const_iterator it = path.begin();
  if (it == path.end())
    return NULL;

  // find the top-level item
  i = 0;
  while (i < topLevelItemCount() && (p = topLevelItem(i))->text(0) != *it) ++i;
  if (i >= topLevelItemCount())
    return NULL;

  // found top level item ... continue
  ++it;
  while (it != path.end()) {
    i = 0;
    while (i < p->childCount() && (item = p->child(i))->text(0) != *it) ++i;
    if (i >= p->childCount())
      return NULL;

    p = item;
    ++it;
  }
  return p;
}

QTreeWidgetItem *CWProjectTree::locateProjectByName(const QString &projectName)
{
  QTreeWidgetItem *p = NULL;
  int i = 0;

  // must be a top-level item - try and find it
  while (i < topLevelItemCount() && (p = topLevelItem(i))->text(0) != projectName) ++i;
  if (i < topLevelItemCount())
    return p;

  return NULL;
}
  
//------------------------------------------------------------------------------
// Interface for editors
//------------------------------------------------------------------------------

QString CWProjectTree::editInsertNewProject(const QString &projectName, CProjectItem **itemCreated)
{
  if (CWorkSpace::instance()->createProject(projectName)) {
    // created the project
    CProjectItem *item = new CProjectItem(projectName);
    addTopLevelItem(item);

    if (itemCreated != NULL) *itemCreated = item;
  }
  else
    return QString("A project with that name already exists.");

  // success falls through
  return QString();
}

QString CWProjectTree::editRenameProject(QTreeWidgetItem *item, const QString &projectName)
{
  if (item && item->type() == cProjectItemType) {

    if (CWorkSpace::instance()->renameProject(item->text(0), projectName)) {
      item->setText(0, projectName);
    }
    else {
      // explain why it failed ...
      if (CWorkSpace::instance()->findProject(item->text(0)))
	return QString("A project with that name already exists.");
      else
	return QString("The project no longer exists.");
    }
  }
  else
    return QString("The item is not a project.");

  return QString();
}

QString CWProjectTree::editInsertNewFolder(QTreeWidgetItem *parent, const QString &folderName, CSpectraFolderItem **itemCreated)
{
  if (parent && (parent->type() == cSpectraBranchItemType || parent->type() == cSpectraFolderItemType)) {
    
    // first make sure that the parent does not already have a child with this name
    QTreeWidgetItem *item = CWProjectTree::locateChildByName(parent, folderName);
    if (!item) {
      CSpectraFolderItem *tmp = new CSpectraFolderItem(parent, folderName);

      if (itemCreated != NULL) *itemCreated = tmp;
    }
    else
      return QString("The parent already contains a folder or file with that name.");
  }
  else  
    return QString("The parent cannot have a folder as a child.");

  // success falls through
  return QString();
}

QString CWProjectTree::editRenameFolder(QTreeWidgetItem *item, const QString &newFolderName)
{
  if (item && item->type() == cSpectraFolderItemType) {

    // first make sure that the parent does not already have a child with this name
    QTreeWidgetItem *sibling = CWProjectTree::locateChildByName(item->parent(), newFolderName);
    if (sibling) {
      if (sibling != item)
	return QString("The parent already has a folder, file or directory with that name.");
      // do nothing if nothing changed (and consider it a successful rename)
    }
    else
      item->setText(0, newFolderName);
  }
  else
    return QString("The item is not a folder.");

  return QString();
}

QString CWProjectTree::editInsertNewAnalysisWindow(QTreeWidgetItem *parent, const QString &windowName,
						   CAnalysisWindowItem **itemCreated)
{
  if (parent && parent->type() == cAnalysisWindowBranchItemType) {
    
    QTreeWidgetItem *projItem = CWProjectTree::projectItem(parent);
    if (!projItem) {
      // corrupt system
      return QString("The project tree is corrupt.");
    }
    if (CWorkSpace::instance()->createAnalysisWindow(projItem->text(0), windowName)) {

      CAnalysisWindowItem *tmp = new CAnalysisWindowItem(parent, windowName);

      if (itemCreated != NULL) *itemCreated = tmp;
    }
    else
      return QString("The project already contains an analysis window with that name.");
  }
  else  
    return QString("The parent cannot have an analysis window as a child.");

  // success falls through
  return QString();
}

QString CWProjectTree::editRenameAnalysisWindow(QTreeWidgetItem *item, const QString &newWindowName)
{
  if (item && item->type() == cAnalysisWindowItemType) {

    QTreeWidgetItem *projItem = CWProjectTree::projectItem(item);
    if (!projItem) {
      // corrupt system
      return QString("The project tree is corrupt.");
    }
    if (CWorkSpace::instance()->renameAnalysisWindow(projItem->text(0), item->text(0), newWindowName)) {
      item->setText(0, newWindowName);
    }
    else {
      // see why it failed
      if (CWorkSpace::instance()->findAnalysisWindow(projItem->text(0), item->text(0)))
	return QString("The project or analysis window no longer exists.");
      else
	return QString("The project already has an analysis window with that name.");
    }
  }
  else
    return QString("The item is not an analysis window.");

  return QString();
}

QString CWProjectTree::editInsertDirectory(QTreeWidgetItem *parent, const QString &directoryPath,
					   const QString &fileFilters, bool includeSubDirs,
					   CSpectraDirectoryItem **itemCreated)
{

  if (parent && (parent->type() == cSpectraBranchItemType || parent->type() == cSpectraFolderItemType)) {
    // split the filter text into a list of file filter strings - an empty list means the filter is '*'
    QStringList filters;

    if (!fileFilters.isEmpty()) {
      if (fileFilters.contains(';')) {
	// split on ';' - NOTE whitespace is significant
	filters = fileFilters.split(';', QString::SkipEmptyParts);
      }
      else {
	// split on whitespace
	filters = fileFilters.split(QRegExp("\\s+"));
      }
    }

    // the directory must exist
    QDir directory(directoryPath);
      
    if (directory.exists()) {
      // create a new directory item
      int fileCount;
      CSpectraDirectoryItem *dirItem = new CSpectraDirectoryItem(0, directory, filters,
								 includeSubDirs,
								 &fileCount);
      
      if (fileCount) {
	parent->addChild(dirItem);

	if (itemCreated != NULL) *itemCreated = dirItem;
      }
      else {
	// empty file count ...
	delete dirItem;
	return QString("No files matched the file filters specified.");
      }
    }
    else {
      QString msg = "The directory ";
      msg += directoryPath;
      msg += " does not exist.";
      return msg;
    }
  }
  else {
    return QString("The parent cannot have a directory as a child.");
  }

  // success falls through to return the null string
  return QString();
}

const QIcon& CWProjectTree::getIcon(int type)
{
  switch (type) {
  case cAnalysisWindowItemType:
    {
      if (!m_windowIcon)
        m_windowIcon = new QIcon(":/icons/project_window_16.png");
      return *m_windowIcon;
    }
    break;
  case cSpectraFolderItemType:
    {
      if (!m_folderIcon)
        m_folderIcon = new QIcon(":/icons/project_folder_16.png");
      return *m_folderIcon;
    }
    break;
  case cSpectraDirectoryItemType:
    {
      if (!m_directoryIcon)
        m_directoryIcon = new QIcon(":/icons/project_directory_16.png");
      return *m_directoryIcon;
    }
    break;
  default:
  case cSpectraFileItemType:
    {
      if (!m_fileIcon)
        m_fileIcon = new QIcon(":/icons/project_file_16.png");
      return *m_fileIcon;
    }
    break;
  }
}

QString CWProjectTree::loadConfiguration(const QList<const CProjectConfigItem*> &itemList)
{
  // walk the list and create ...
  QString errStr;
  QTreeWidgetItem *item;
  CProjectItem *projItem;
  mediate_project_t *properties;
  int i;

  // clear the current projects - bottom up
  i = topLevelItemCount();
  while (i > 0) {
    delete takeTopLevelItem(--i);
  }
  // also sites and symbols .. TODO


  // use the edit* interface to get reasonable error messages.

  QList<const CProjectConfigItem*>::const_iterator it = itemList.begin();
  while (it != itemList.end()) {

    QString name = (*it)->projectName();

    // create the project item
    projItem = NULL;
    errStr = editInsertNewProject(name, &projItem);
    if (!errStr.isNull())
      return errStr;

    // locate the properties in the workspace then copy
    properties = CWorkSpace::instance()->findProject(name);
    assert(properties != NULL);
    *properties = *((*it)->properties()); // blot copy

    item = projItem->child(0); // raw spectra node for the project

    // recursive construction of the project tree from the config tree ...
    const CProjectConfigTreeNode *firstChild = (*it)->rootNode()->firstChild();
    // top-down construction of the tree ...
    errStr = CWProjectTree::buildRawSpectraTree(item, firstChild);
    if (!errStr.isNull())
      return errStr;
      
    ++it;
  }
  
  return errStr;
}


QString CWProjectTree::buildRawSpectraTree(QTreeWidgetItem *parent, const CProjectConfigTreeNode *childConfigItem)
{
  // recursive construction ...
  QString errStr;
  
  // create items for childConfigItem and its siblings
  while (childConfigItem != NULL) {
    
    switch (childConfigItem->type()) {
    case CProjectConfigTreeNode::eFile:
      {
	CSpectraFileItem *fileItem = new CSpectraFileItem(parent, QFileInfo(childConfigItem->name()));
	fileItem->setEnabled(childConfigItem->isEnabled());

	// should not have children ...
      }
      break;
    case CProjectConfigTreeNode::eFolder:
      {
	CSpectraFolderItem *folderItem = NULL;

	errStr += editInsertNewFolder(parent, childConfigItem->name(), &folderItem);
	if (folderItem != NULL) {
	  folderItem->setEnabled(childConfigItem->isEnabled());

	  // can have children ...
	  const CProjectConfigTreeNode *firstChild = childConfigItem->firstChild();
	  if (firstChild != NULL) {
	    errStr += CWProjectTree::buildRawSpectraTree(folderItem, firstChild);
	  }
	}
      }
      break;
    case CProjectConfigTreeNode::eDirectory:
      {
	CSpectraDirectoryItem *dirItem = NULL;

	errStr += editInsertDirectory(parent, childConfigItem->name(), childConfigItem->filter(),
				      childConfigItem->recursive(), &dirItem);
	if (dirItem != NULL)
	  dirItem->setEnabled(childConfigItem->isEnabled());

	// should not have children ...
      }
      break;
    }
    
    childConfigItem = childConfigItem->nextSibling();
  }

  return errStr;
}

//------------------------------------------------------------------------------
// SLOTS
//------------------------------------------------------------------------------

void CWProjectTree::slotEnable()
{
  // setEnabled(true) the entire selection

  QList<QTreeWidgetItem*> items = selectedItems();
  
  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    CProjectTreeItem *p = static_cast<CProjectTreeItem*>(*it);
    p->setEnabled(true);
    
    ++it;
  }
  
}

void CWProjectTree::slotDisable()
{
  // setEnabled(false) the entire selection

  QList<QTreeWidgetItem*> items = selectedItems();
  
  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    CProjectTreeItem *p = static_cast<CProjectTreeItem*>(*it);
    p->setEnabled(false);
    
    ++it;
  }
  
}

void CWProjectTree::slotToggleEnable()
{
  // invert the enable status of the entire selection

  QList<QTreeWidgetItem*> items = selectedItems();
  
  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    CProjectTreeItem *p = static_cast<CProjectTreeItem*>(*it);
    p->setEnabled(!p->isEnabled());
    
    ++it;
  }
  
}

void CWProjectTree::slotToggleDisplayDetails()
{
  if (m_colWidthList.empty()) {
    // hide the details
    for (int i=0; i<3; ++i)
      m_colWidthList.push_back(columnWidth(i));
    
    hideColumn(1);
    hideColumn(2);
    //    hideColumn(3);
    
    emit signalWidthModeChanged(cProjectTreeHideDetailMode);
  }
  else {
    // show the details
    showColumn(1);
    showColumn(2);
    //    showColumn(3);
    
    // restore the sizes
    for (int i=0; i<3; ++i)
      setColumnWidth(i, m_colWidthList.at(i));
    m_colWidthList.clear();
    
    emit signalWidthModeChanged(cProjectTreeShowDetailMode);     
  }
}

void CWProjectTree::slotCreateProject()
{
  CWEditor *nameEditor = new  CWProjectNameEditor(this);
  m_activeContext->addEditor(nameEditor);
}

void CWProjectTree::slotRenameProject()
{
  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *item = items.front();
    if (item->type() == cProjectItemType) {

      CWEditor *nameEditor = new  CWProjectNameEditor(this, item);
      m_activeContext->addEditor(nameEditor);
    }
  }
}

void CWProjectTree::slotEditProject()
{
  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *item = items.front();
    if (item->type() == cProjectItemType) {

      CWEditor *propEditor = new  CWProjectPropertyEditor(this, item);
      m_activeContext->addEditor(propEditor);
    }
  }
}

void CWProjectTree::slotRefreshDirectories()
{
  // Ok for single, multi and no selection
  QList<QTreeWidgetItem*> items = CWProjectTree::directoryItems(selectedItems());
  
  if (items.count() > 1)
    items = CWProjectTree::normalize(items);

  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    // only for directory entries - aleady selected by type()
    CSpectraDirectoryItem *dirItem = dynamic_cast<CSpectraDirectoryItem*>(*it);
    if (dirItem)
      dirItem->refreshBranch();
    
    ++it;
  }
}
 
void CWProjectTree::slotCreateFolder()
{
  // expect selection has one item and it is either
  // a spectra branch or folder item

  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *parent = items.front();
    if (parent->type() == cSpectraBranchItemType || parent->type() == cSpectraFolderItemType) {

      CWEditor *nameEditor = new  CWProjectFolderNameEditor(this, parent, true);
      m_activeContext->addEditor(nameEditor);
    }
  }
}

void CWProjectTree::slotRenameFolder()
{
  // expect selection has one item and it is a
  // a spectra folder item

  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *parent = items.front();
    if (parent->type() == cSpectraFolderItemType) {

      CWEditor *dirEditor = new  CWProjectFolderNameEditor(this, parent, false);
      m_activeContext->addEditor(dirEditor);
    }
  }
}

void CWProjectTree::slotInsertFile()
{
  // expect selection has one item and it is a
  // a spectra folder item or the raw spectra branch
  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *parent = items.front();
    if (parent->type() == cSpectraFolderItemType || parent->type() == cSpectraBranchItemType) {

      QTreeWidgetItem *projItem = CWProjectTree::projectItem(parent);
      if (projItem == NULL)
	return;

      // access to the project data
      const mediate_project_t *projData = CWorkSpace::instance()->findProject(projItem->text(0));
      if (projData == NULL)
	return;

      CPreferences *prefs = CPreferences::instance();
      
      QString extension = prefs->fileExtension("Instrument", projData->instrumental.format, "spe");
      QString filter;
      QTextStream stream(&filter);
      
      stream << "Spectra (*." << extension << ");;All files(*.*)";
      
      QStringList files = QFileDialog::getOpenFileNames(0, "Select one or more spectra files",
							prefs->directoryName("Spectra"), filter);
      
      // Documentations says copy ??
      if (!files.isEmpty()) {
	QStringList copy = files;
	QList<QString>::iterator it = copy.begin();
	if (it != copy.end()) {
	  // store the preferences
	  prefs->setFileExtensionGivenFile("Instrument", projData->instrumental.format, *it);
	  prefs->setDirectoryNameGivenFile("Spectra", *it);
	}
	while (it != copy.end()) {
	  // create the items
	  new CSpectraFileItem(parent, QFileInfo(*it));
	  ++it;
	}
      }
    }
  }
}

void CWProjectTree::slotInsertDirectory()
{
  // expect selection has one item and it is a
  // a CSpectraFolderItem or a CSpectraBranchItem
  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *parent = items.front();
    if (parent->type() == cSpectraFolderItemType || parent->type() == cSpectraBranchItemType) {

      CWEditor *nameEditor = new  CWProjectDirectoryEditor(this, parent);
      m_activeContext->addEditor(nameEditor);
      
    }
  }
}

void CWProjectTree::slotCreateAnalysisWindow()
{  
  // expect selection has one item and is an Anaylsis Window Branch

  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *parent = items.front();
    if (parent->type() == cAnalysisWindowBranchItemType) {

      CWEditor *nameEditor = new  CWProjectAnalysisWindowNameEditor(this, parent, true);
      m_activeContext->addEditor(nameEditor);
    }
  }
}

void CWProjectTree::slotRenameAnalysisWindow()
{  
  // expect selection has one item and is an Anaylsis Window

  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *item = items.front();
    if (item->type() == cAnalysisWindowItemType) {

      CWEditor *nameEditor = new  CWProjectAnalysisWindowNameEditor(this, item, false);
      m_activeContext->addEditor(nameEditor);
    }
  }
}

void CWProjectTree::slotRunAnalysis()
{
  // TODO
}

void CWProjectTree::slotBrowseSpectra()
{
  CSession *session = new CSession(false);

  // normalise the selection and then traverse to build session
  QList<QTreeWidgetItem*> items = CWProjectTree::normalize(selectedItems());

  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    // only consider enabled items
    CProjectTreeItem *tmp = dynamic_cast<CProjectTreeItem*>(*it);
    if (tmp && tmp->isEnabled())
      CWProjectTree::buildSession(session, tmp);
    ++it;
  }

  // safely dispatch the session to attached slots by wrapping in a reference
  // counting pointer.

  RefCountPtr<CSession> ptr(session);

  emit signalStartBrowseSession(ptr);
}

void CWProjectTree::slotDeleteSelection()
{
  // normalize the selection
  QList<QTreeWidgetItem*> items = CWProjectTree::normalize(selectedItems());
  
  int type;
  QList<QTreeWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    // cant delete the Raw Spectra or Analysis Window branches (instead delete all of there children)
    type = (*it)->type();
    if (type == cSpectraBranchItemType || type == cAnalysisWindowBranchItemType) {
      // delete all children
      QList<QTreeWidgetItem*> kids = (*it)->takeChildren();
      QList<QTreeWidgetItem*>::iterator kIt = kids.begin();
      while (kIt != kids.end())
        delete *kIt++;
    }
    else if ((*it)->parent() == NULL) {
      // top level item (a project)
      delete takeTopLevelItem(indexOfTopLevelItem(*it));
    }
    else {
      QTreeWidgetItem *p = (*it)->parent();
      // can remove the children of directory items
      if (p->type() !=  cSpectraDirectoryItemType)
        delete p->takeChild(p->indexOfChild(*it));
    }

    ++it;
  }
}

void CWProjectTree::slotDeleteAllSpectra()
{
  // TODO
}

//------------------------------------------------------------------------------
// Tree Items
//------------------------------------------------------------------------------

CProjectTreeItem::CProjectTreeItem(const QStringList &strings, int type) :
  QTreeWidgetItem(strings, type),
  m_enabled(true)
{
}

CProjectTreeItem::CProjectTreeItem(CWProjectTree *parent, int type) :
  QTreeWidgetItem(parent, type),
  m_enabled(true)
{
}

CProjectTreeItem::CProjectTreeItem(CWProjectTree *parent, const QStringList &strings, int type) :
  QTreeWidgetItem(parent, strings, type),
  m_enabled(true)
{
}

CProjectTreeItem::CProjectTreeItem(QTreeWidgetItem *parent, int type) :
  QTreeWidgetItem(parent, type),
  m_enabled(true)
{
}
  
CProjectTreeItem::CProjectTreeItem(QTreeWidgetItem *parent, const QStringList &strings, int type) :
  QTreeWidgetItem(parent, strings, type),
  m_enabled(true)
{
}

CProjectTreeItem::~CProjectTreeItem()
{
}

void CProjectTreeItem::setEnabled(bool enable)
{
  m_enabled = enable;
}

//------------------------------------------------------------------------------

CProjectItem::CProjectItem(const QString &projectName) :
  CProjectTreeItem(QStringList(projectName), cProjectItemType)
{
  // add Children for Raw Spectra and Analysis Windows
  new CSpectraBranchItem(this);
  new CAnalysisWindowBranchItem(this);
}

CProjectItem::~CProjectItem()
{
  // there should be a corresponding project in the workspace ...
  CWorkSpace::instance()->destroyProject(text(0));
}

QVariant CProjectItem::data(int column, int role) const
{
  if (role == Qt::ForegroundRole) {
    return QVariant(QBrush(QColor(m_enabled ? cProjectTextColour : cDisabledTextColour)));
  }

  // for other roles use the base class
  return QTreeWidgetItem::data(column, role);
}

//------------------------------------------------------------------------------

CAnalysisWindowBranchItem::CAnalysisWindowBranchItem(QTreeWidgetItem *parent) :
  CProjectTreeItem(parent, QStringList("Analysis Windows"), cAnalysisWindowBranchItemType)
{
}

CAnalysisWindowBranchItem::~CAnalysisWindowBranchItem()
{
}

//------------------------------------------------------------------------------

CSpectraBranchItem::CSpectraBranchItem(QTreeWidgetItem *parent) :
  CProjectTreeItem(parent, QStringList("Raw Spectra"), cSpectraBranchItemType)
{
}

CSpectraBranchItem::~CSpectraBranchItem()
{
}

//------------------------------------------------------------------------------

CSpectraFolderItem::CSpectraFolderItem(QTreeWidgetItem *parent, const QString &folderName) :
  CProjectTreeItem(parent, QStringList(folderName), cSpectraFolderItemType)
{
  setIcon(0, CWProjectTree::getIcon(cSpectraFolderItemType));
}

CSpectraFolderItem::~CSpectraFolderItem()
{
}

QVariant CSpectraFolderItem::data(int column, int role) const
{
  if (role == Qt::ForegroundRole && !m_enabled) {
    return QVariant(QBrush(QColor(cDisabledTextColour)));
  }
  
  // for other roles use the base class
  return QTreeWidgetItem::data(column, role);
}

//------------------------------------------------------------------------------

CSpectraDirectoryItem::CSpectraDirectoryItem(QTreeWidgetItem *parent, const QDir &directory,
                                               const QStringList &fileFilters, bool includeSubDirectories,
                                               int *fileCount) :
  CProjectTreeItem(parent, cSpectraDirectoryItemType),
  m_directory(directory),
  m_fileFilters(fileFilters),
  m_includeSubDirectories(includeSubDirectories)
{
  setIcon(0, CWProjectTree::getIcon(cSpectraDirectoryItemType));

  // build the file and directory tree for this directory (recursive)
  int nFiles = loadBranch();

  // set the fileCount for the entire branch if requested
  if (fileCount)
    *fileCount = nFiles;
}

CSpectraDirectoryItem::~CSpectraDirectoryItem()
{
}

QVariant CSpectraDirectoryItem::data(int column, int role) const
{
  if (role == Qt::DisplayRole) {
    
    if (column == 0)
      return QVariant(m_directory.dirName());
    //    if (column == 1)
    //      return QVariant(m_directory.nameFilters().join("; "));
    //if (column == 3)
    //  return QVariant(m_directory.absolutePath());

    return QVariant();
  }
  else if (role == Qt::ForegroundRole && !m_enabled) {
    return QVariant(QBrush(QColor(cDisabledTextColour)));
  }

  // for other roles use the base class
  return QTreeWidgetItem::data(column, role);
}

void CSpectraDirectoryItem::refreshBranch(void)
{
  discardBranch();
  loadBranch();
}

int CSpectraDirectoryItem::loadBranch(void)
{
  int totalFileCount = 0;

  QFileInfoList entries;
  QFileInfoList::iterator it;

  // first consder sub directories ...
  if (m_includeSubDirectories) {
    entries = m_directory.entryInfoList(); // all entries ... but only take directories on this pass

    it = entries.begin();
    while (it != entries.end()) {
      if (it->isDir() && !it->fileName().startsWith('.')) {
        int tmpFileCount;

        TRACE3("Load sub dir " << it->filePath().toStdString());

        CSpectraDirectoryItem *dItem = new CSpectraDirectoryItem(NULL, it->filePath(),
                                                                   m_fileFilters, true, &tmpFileCount);
        // were there any matching files in this branch?
        if (tmpFileCount) {
          addChild(dItem);
          totalFileCount += tmpFileCount;
        }
        else
          delete dItem; // discard
      }
      ++it;
    }
  }

  // now the files that match the filters
  if (m_fileFilters.isEmpty())
    entries = m_directory.entryInfoList();
  else
    entries = m_directory.entryInfoList(m_fileFilters);

  it = entries.begin();
  while (it != entries.end()) {
    if (it->isFile()) {
      new CSpectraFileItem(this, *it);
      ++totalFileCount;
    }
    ++it;
  }
  
  return totalFileCount;
}

void CSpectraDirectoryItem::discardBranch(void)
{
  // remove all child items (recursive destruction)
  
  QList<QTreeWidgetItem*> children = takeChildren();
  QList<QTreeWidgetItem*>::iterator it = children.begin();
  while (it != children.end()) {
    delete *it;
    ++it;
  }
}

//------------------------------------------------------------------------------

CSpectraFileItem::CSpectraFileItem(QTreeWidgetItem *parent, const QFileInfo &fileInfo) :
  CProjectTreeItem(parent, cSpectraFileItemType),
  m_fileInfo(fileInfo)
{
  setIcon(0, CWProjectTree::getIcon(cSpectraFileItemType));
}

CSpectraFileItem::~CSpectraFileItem()
{
}

QVariant CSpectraFileItem::data(int column, int role) const
{
  if (role == Qt::DisplayRole) {
    
    if (column == 0)
      return QVariant(m_fileInfo.fileName());
    if (column == 1) {
      QString tmpStr;
      QTextStream tmpStream(&tmpStr);
      qint64 fs = m_fileInfo.size();
      if (fs >= 1024*100) {
        fs >>= 10; // now in KBytes
        if (fs >= 1024*100) {
          fs >>= 10; // now in MBytes
          if (fs >= 1024*100) {
            fs >>= 10; // now in GBytes
            tmpStream << fs << " GB";
          }
          else
            tmpStream << fs << " MB";
        }
        else
          tmpStream << fs << " KB";
      }
      else
        tmpStream << fs << "  B";

      return QVariant(tmpStr);
    }
    if (column == 2)
      return QVariant(m_fileInfo.lastModified().toString());
    //if (column == 3)
    //  return QVariant(m_fileInfo.absolutePath());

    return QVariant();

  }
  else if (role == Qt::TextAlignmentRole) {
    
    if (column == 1)
      return QVariant(Qt::AlignRight);
    if (column == 2)
      return QVariant(Qt::AlignCenter);
  }
  else if (role == Qt::ForegroundRole && !m_enabled) {
    return QVariant(QBrush(QColor(cDisabledTextColour)));
  }

  // for other roles use the base class
  return QTreeWidgetItem::data(column, role);
}

//------------------------------------------------------------------------------

CAnalysisWindowItem::CAnalysisWindowItem(QTreeWidgetItem *parent, const QString &windowName) :
  CProjectTreeItem(parent, QStringList(windowName), cAnalysisWindowItemType)
{
  setIcon(0, CWProjectTree::getIcon(cAnalysisWindowItemType));
}

CAnalysisWindowItem::~CAnalysisWindowItem()
{
  // there should be a corresponding project in the workspace ...

  // NOTE: We require that the toplevel item of the tree (project item) can be
  // found. This must be the case for any 'delete' operation (it assumes the
  // parent item does NOT invalidate the childs 'parent' pointer before the
  // child is destroyed...

  QTreeWidgetItem *projItem = CWProjectTree::projectItem(this);
  if (projItem)
    CWorkSpace::instance()->destroyAnalysisWindow(projItem->text(0), text(0));
  else
    assert(false);
}

QVariant CAnalysisWindowItem::data(int column, int role) const
{
  if (role == Qt::ForegroundRole && !m_enabled) {
    return QVariant(QBrush(QColor(cDisabledTextColour)));
  }

  // for other roles use the base class
  return QTreeWidgetItem::data(column, role);
}


//------------------------------------------------------------------------------
// Static methods
//------------------------------------------------------------------------------

int CWProjectTree::itemDepth(QTreeWidgetItem *item)
{
  // item MUST be a valid item (ie. NOT null) - A toplevel item has depth = 0
  int n = 0;

  while ((item = item->parent()) != NULL) ++n;

  return n;
}

QTreeWidgetItem* CWProjectTree::ancestor(QTreeWidgetItem *item, int nth)
{
  // move back n generations. nth MUST not be larger the items depth
  while (nth--)
    item = item->parent();

  return item;
}

QTreeWidgetItem* CWProjectTree::projectItem(QTreeWidgetItem *item)
{
  // move back to the top level of the branch containing item.
  if (!item)
    return NULL;

  QTreeWidgetItem *p;

  while ((p = item->parent()) != NULL) {
    item = p;
  }

  return item;
}

QList<QTreeWidgetItem*> CWProjectTree::normalize(QList<QTreeWidgetItem*> items)
{
  QList<int> depthList;
  QList<QTreeWidgetItem*> normList;
  int iDepth, delta;

  while (true) {
    bool repeat = false;

    // for each item
    QList<QTreeWidgetItem*>::iterator iIt = items.begin();
    while (iIt != items.end()) {
      bool discardItem = false;
      iDepth = CWProjectTree::itemDepth(*iIt);
      
      QList<QTreeWidgetItem*>::iterator nIt = normList.begin();
      QList<int>::iterator dIt = depthList.begin();
      while (nIt != normList.end()) {

        delta = iDepth - *dIt;
        if (delta > 0) {
          // *iIt could be in the tree of *nIt
          if (*nIt == CWProjectTree::ancestor(*iIt, delta)) {
            // yes it is - can discard this item
            discardItem = true;
            break;
          }
        }
        else if (delta < 0) {
          // *nIt could be in the tree of *iIt
          if (*iIt == CWProjectTree::ancestor(*nIt, -delta)) {
            // yes it is - delete nIt
            // no other normList item is an ancestor of iIt, but it could be an
            // ancestor of items in normList => repeat
            normList.erase(nIt);
            depthList.erase(dIt);
            repeat = true;
            break;
          }
        }
        // else at the same level and connot be the same (no duplicates in items)
        ++nIt;
        ++dIt;
      }
      // checked against all items in normList
      if (!discardItem) {
        normList.push_front(*iIt);
        depthList.push_front(iDepth);
      }
      ++iIt;
    }
    
    // try again with a reduced set of items or return ...
    if (repeat) {
      items = normList;
      normList.clear();
      depthList.clear();
    }
    else {
      return normList;
    }
  }
}

QList<QTreeWidgetItem*> CWProjectTree::directoryItems(const QList<QTreeWidgetItem*> &items)
{
  QList<QTreeWidgetItem*> dirItems;

  QList<QTreeWidgetItem*>::const_iterator it = items.begin();
  while (it != items.end()) {
    if ((*it)->type() == cSpectraDirectoryItemType)
      dirItems.push_back(*it);
    ++it;
  }

  return dirItems;
}

QTreeWidgetItem* CWProjectTree::locateChildByName(QTreeWidgetItem *parent, const QString &childName)
{
  if (!parent)
    return NULL;

  QTreeWidgetItem *item = NULL;
  int i = 0;

  while (i < parent->childCount() && (item = parent->child(i))->text(0) != childName) ++i;
  if (i < parent->childCount())
    return item;

  return NULL;
}

void CWProjectTree::buildSession(CSession *session, CProjectTreeItem *item)
{
  // recursive construction of the session ...
  
  if (item->type() == cSpectraFileItemType) {
    // individual file
    QTreeWidgetItem *proj = CWProjectTree::projectItem(item);
    if (proj)
      session->addFile(static_cast<CSpectraFileItem*>(item)->file(), proj->text(0));
  }
  else if (item->type() == cSpectraFolderItemType ||
	   item->type() == cSpectraDirectoryItemType ||
	   item->type() == cSpectraBranchItemType ||
	   item->type() == cProjectItemType) {
    // all enabled sub items ... 
    CProjectTreeItem *child;
    int nChildren = item->childCount();
    int i = 0;
    while (i < nChildren) {
      child = static_cast<CProjectTreeItem*>(item->child(i));
      if (child->isEnabled())
	CWProjectTree::buildSession(session, child);
      ++i;
    }
  }
}


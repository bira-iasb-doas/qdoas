#include <iostream> // TODO

#include <QMenu>
#include <QDateTime>
#include <QKeyEvent>
#include <QShowEvent>
#include <QContextMenuEvent>
#include <QFileDialog>

#include "CWProjectTree.h"
#include "CWProjectFolderNameEditor.h"
#include "CWProjectDirectoryEditor.h"
#include "CWActiveContext.h"

// pixmaps for the project tree
#include "icons/project_folder_16.xpm"
#include "icons/project_directory_16.xpm"
#include "icons/project_file_16.xpm"

const int cProjectTreeHideDetailMode   = 27;
const int cProjectTreeShowDetailMode   = 28;

const QRgb cDisabledTextColour         = 0xFFAAAAAA;
const QRgb cProjectTextColour          = 0xFFA93F26;

QIcon *CWProjectTree::m_folderIcon = NULL;
QIcon *CWProjectTree::m_directoryIcon = NULL;
QIcon *CWProjectTree::m_fileIcon = NULL;

CWProjectTree::CWProjectTree(CWActiveContext *activeContext, QWidget *parent) :
  QTreeWidget(parent),
  m_activeContext(activeContext)
{
  QStringList labelList;
  labelList << "Name" << "Size" << "Modified" << "Directory"; 
  setHeaderLabels(labelList);

  setColumnWidth(0, 200);
  setColumnWidth(1, 60);
  setColumnWidth(2, 160);
  setColumnWidth(3, 200);

  setSelectionMode(QAbstractItemView::ExtendedSelection);
}

CWProjectTree::~CWProjectTree()
{
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
    CProjectTreeItem *projItem = static_cast<CProjectTreeItem*>(items.front());
    int itemType = projItem->type();

    if (itemType == cSpectraDirectoryItemType) {
      // A Directory item
      menu.addAction(projItem->isEnabled() ? "Disable" : "Enable", this,
                     SLOT(slotToggleEnable()));
      menu.addAction("Refresh", this, SLOT(slotRefreshDirectories()));
      menu.addSeparator();
      menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()));
      menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()));
      menu.addSeparator();
      menu.addAction("Delete", this, SLOT(slotDeleteSelection()));
    }
    else if (itemType == cSpectraFolderItemType) {
      // A Folder Item
      menu.addAction(projItem->isEnabled() ? "Disable" : "Enable", this,
                     SLOT(slotToggleEnable()));
      menu.addAction("Rename...", this, SLOT(slotRenameFolder()));
      menu.addAction("New Sub-Folder...", this, SLOT(slotCreateFolder()));
      menu.addAction("Insert File...", this, SLOT(slotInsertFile()));
      menu.addAction("Insert Directory...", this, SLOT(slotInsertDirectory()));
      menu.addSeparator();
      menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()));
      menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()));
      menu.addSeparator();
      menu.addAction("Delete", this, SLOT(slotDeleteSelection()));
    }
    else if (itemType == cSpectraFileItemType) {
      // A File Item
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
      menu.addAction("Delete All", this, SLOT(slotDeleteAllAnalysisWindows()));
    }
    else if (itemType == cProjectItemType) {
      menu.addAction(projItem->isEnabled() ? "Disable" : "Enable", this,
                     SLOT(slotToggleEnable()));
      menu.addAction("New Project...", this, SLOT(slotCreateProject()));
      menu.addSeparator();
      menu.addAction("Run Analysis", this, SLOT(slotRunAnalysis()));
      menu.addAction("Browse Spectra", this, SLOT(slotBrowseSpectra()));
      menu.addSeparator();
      menu.addAction("Delete", this, SLOT(slotDeleteProject()));
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

void CWProjectTree::addNewProject(const QString &projectName)
{
  addTopLevelItem(new CProjectItem(projectName));
}

QTreeWidgetItem *CWProjectTree::locateByPath(const QStringList &path)
{
  QTreeWidgetItem *item, *p = NULL;
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

const QIcon& CWProjectTree::getIcon(int type)
{
  switch (type) {
  case cSpectraFolderItemType:
    {
      if (!m_folderIcon)
        m_folderIcon = new QIcon(QPixmap(project_folder_16_xpm));
      return *m_folderIcon;
    }
    break;
  case cSpectraDirectoryItemType:
    {
      if (!m_directoryIcon)
        m_directoryIcon = new QIcon(QPixmap(project_directory_16_xpm));
      return *m_directoryIcon;
    }
    break;
  case cSpectraFileItemType:
  default:
    {
      if (!m_fileIcon)
        m_fileIcon = new QIcon(QPixmap(project_file_16_xpm));
      return *m_fileIcon;
    }
    break;
  }
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
    for (int i=0; i<4; ++i)
      m_colWidthList.push_back(columnWidth(i));
    
    hideColumn(1);
    hideColumn(2);
    hideColumn(3);
    
    emit signalWidthModeChanged(cProjectTreeHideDetailMode);
  }
  else {
    // show the details
    showColumn(1);
    showColumn(2);
    showColumn(3);
    
    // restore the sizes
    for (int i=0; i<4; ++i)
      setColumnWidth(i, m_colWidthList.at(i));
    m_colWidthList.clear();
    
    emit signalWidthModeChanged(cProjectTreeShowDetailMode);     
  }
}

void CWProjectTree::slotCreateProject()
{
  // dialog for the project name ...  TODO
  addNewProject("John");
}

void CWProjectTree::slotDeleteProject()
{
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

      CWProjectFolderNameEditor *nameEditor = new  CWProjectFolderNameEditor(this, parent, true);
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

      CWProjectFolderNameEditor *dirEditor = new  CWProjectFolderNameEditor(this, parent, false);
      m_activeContext->addEditor(dirEditor);
    }
  }
}

void CWProjectTree::slotInsertFile()
{
  // expect selection has one item and it is a
  // a spectra folder item
  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.count() == 1) {
    QTreeWidgetItem *parent = items.front();
    if (parent->type() == cSpectraFolderItemType) {

      // Modal File dialog
      QStringList files = QFileDialog::getOpenFileNames(0, "Select one or more spectra files",
                                                        "/home",
                                                        "Ascii (*.spe);;Other (*.hdf)");
      // Documentations says copy ??
      if (!files.isEmpty()) {
        QStringList copy = files;
        QList<QString>::iterator it = copy.begin();
        while (it != copy.end()) {
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

      CWProjectDirectoryEditor *nameEditor = new  CWProjectDirectoryEditor(this, parent);
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
  // TODO
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
    if (column == 3)
      return QVariant(m_directory.absolutePath());

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
        std::cout << "Load sub dir " << it->filePath().toStdString() << std::endl;
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
      qint64 fs = m_fileInfo.size();
      if (fs >= 1024*100) {
        fs >>= 10; // now in KBytes
        if (fs >= 1024*100) {
          fs >>= 10; // now in MBytes
          if (fs >= 1024*100) {
            fs >>= 10; // now in GBytes
            tmpStr.sprintf("%d GB", fs);
          }
          else
            tmpStr.sprintf("%d MB", fs);
        }
        else
          tmpStr.sprintf("%d KB", fs);
      }
      else
        tmpStr.sprintf("%d  B", fs);

      return QVariant(tmpStr);
    }
    if (column == 2)
      return QVariant(m_fileInfo.lastModified().toString());
    if (column == 3)
      return QVariant(m_fileInfo.absolutePath());

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

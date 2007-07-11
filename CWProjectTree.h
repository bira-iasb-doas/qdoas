#ifndef _CWPROJECTTREE_H_GUARD
#define _CWPROJECTTREE_H_GUARD

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QIcon>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QSize>

class CWActiveContext;

const int cProjectItemType              = QTreeWidgetItem::UserType + 91;
const int cSpectraBranchItemType        = QTreeWidgetItem::UserType + 92;
const int cAnalysisWindowBranchItemType = QTreeWidgetItem::UserType + 93;
const int cSpectraFolderItemType        = QTreeWidgetItem::UserType + 94;
const int cSpectraDirectoryItemType     = QTreeWidgetItem::UserType + 95;
const int cSpectraFileItemType          = QTreeWidgetItem::UserType + 96;
const int cAnalysisWindowItemType       = QTreeWidgetItem::UserType + 97;

class CWProjectTree : public QTreeWidget
{
Q_OBJECT
 public:
  CWProjectTree(CWActiveContext *activeContext, QWidget *parent = 0);
  virtual ~CWProjectTree();

  void addNewProject(const QString &projectName); 

  QTreeWidgetItem *locateProjectByName(const QString &projectName);
  QTreeWidgetItem *locateByPath(const QStringList &path);

  // Interface editors use to modify the tree

  QString editInsertNewProject(const QString &projectName);
  QString editRenameProject(QTreeWidgetItem *item, const QString &projectName);
  QString editInsertNewFolder(QTreeWidgetItem *parent, const QString &folderName);
  QString editRenameFolder(QTreeWidgetItem *item, const QString &folderName);
  QString editInsertNewAnalysisWindow(QTreeWidgetItem *parent, const QString &windowName);
  QString editRenameAnalysisWindow(QTreeWidgetItem *item, const QString &windowName);
  QString editInsertDirectory(QTreeWidgetItem *parent, const QString &directoryPath,
			      const QString &fileFilters, bool includeSubDirs);

  // static methods

  static const QIcon& getIcon(int type);

  static int itemDepth(QTreeWidgetItem *item);
  static QTreeWidgetItem* ancestor(QTreeWidgetItem *item, int nth);
  static QList<QTreeWidgetItem*> normalize(QList<QTreeWidgetItem*> items);
  static QList<QTreeWidgetItem*> directoryItems(const QList<QTreeWidgetItem*> &items);
  static QTreeWidgetItem* locateChildByName(QTreeWidgetItem *parent, const QString &childName);

 protected:
  virtual void keyPressEvent(QKeyEvent *e);
  virtual void showEvent(QShowEvent *e);
  virtual void contextMenuEvent(QContextMenuEvent *e);

 public slots:
  void slotEnable();
  void slotDisable();
  void slotToggleEnable();
  void slotToggleDisplayDetails();
  void slotCreateProject();
  void slotRenameProject();
  void slotEditProject();
  void slotRefreshDirectories();
  void slotCreateFolder();
  void slotRenameFolder();
  void slotInsertFile();
  void slotInsertDirectory();
  void slotCreateAnalysisWindow();
  void slotRenameAnalysisWindow();
  void slotRunAnalysis();
  void slotBrowseSpectra();
  void slotDeleteSelection();
  void slotDeleteAllSpectra();

 signals:
  void signalWidthModeChanged(int newMode);

 private:
  QList<int> m_colWidthList;
  CWActiveContext *m_activeContext;

  // icons for the tree
  static QIcon *m_windowIcon, *m_folderIcon, *m_directoryIcon, *m_fileIcon;
  // icons for the context menu - TODO
};

// ALL concrete items in CWProjectTree are derived from CProjectTreeItem

class CProjectTreeItem : public QTreeWidgetItem
{
 public:
  CProjectTreeItem(const QStringList &strings, int type);
  CProjectTreeItem(CWProjectTree *parent, int type);
  CProjectTreeItem(CWProjectTree *parent, const QStringList &strings, int type);
  CProjectTreeItem(QTreeWidgetItem *parent, int type);
  CProjectTreeItem(QTreeWidgetItem *parent, const QStringList &strings, int type);
  virtual ~CProjectTreeItem();

  virtual void setEnabled(bool enable);
  bool isEnabled(void) const;

 protected:
  bool m_enabled;
};

inline bool CProjectTreeItem::isEnabled(void) const { return m_enabled; }

class CProjectItem : public CProjectTreeItem
{
 public:
  CProjectItem(const QString &projectName);
  virtual ~CProjectItem();

  virtual QVariant data(int column, int role) const;
};

class CAnalysisWindowBranchItem : public CProjectTreeItem
{
 public:
  CAnalysisWindowBranchItem(QTreeWidgetItem *parent);
  virtual ~CAnalysisWindowBranchItem();
};

class CSpectraBranchItem : public CProjectTreeItem
{
 public:
  CSpectraBranchItem(QTreeWidgetItem *parent);
  virtual ~CSpectraBranchItem();
};

// Folder can have abitrary children (files, folders, directories)

class CSpectraFolderItem : public CProjectTreeItem
{
 public:
  CSpectraFolderItem(QTreeWidgetItem *parent, const QString &folderName);
  virtual ~CSpectraFolderItem();

  virtual QVariant data(int column, int role) const;
};

// Directory can have files and directories as children, but is driven by the filesystem

class CSpectraDirectoryItem : public CProjectTreeItem
{
 public:
  CSpectraDirectoryItem(QTreeWidgetItem *parent, const QDir &directory,
                        const QStringList &fileFilters, bool includesSubDirectories,
                        int *fileCount = 0);
  virtual ~CSpectraDirectoryItem();

  virtual QVariant data(int column, int role) const;

  void refreshBranch(void);

 protected:
  int loadBranch(void);
  void discardBranch(void);

 private:
  QDir m_directory;
  QStringList m_fileFilters;
  bool m_includeSubDirectories;
};

// File - Single reference to a file-system file

class CSpectraFileItem : public CProjectTreeItem
{
 public:
  CSpectraFileItem(QTreeWidgetItem *parent, const QFileInfo &fileInfo);
  virtual ~CSpectraFileItem();

  virtual QVariant data(int column, int role) const;

 private:
  QFileInfo m_fileInfo;
};

class CAnalysisWindowItem : public CProjectTreeItem
{
 public:
  CAnalysisWindowItem(QTreeWidgetItem *parent, const QString &windowName);
  virtual ~CAnalysisWindowItem();

  virtual QVariant data(int column, int role) const;
};

#endif

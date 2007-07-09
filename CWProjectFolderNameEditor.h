#ifndef _CWPROJECTFOLDERNAMEEDITOR_H_GUARD
#define _CWPROJECTFOLDERNAMEEDITOR_H_GUARD

#include <QLineEdit>
#include <QStringList>
#include <QTreeWidgetItem>

#include "CWEditor.h"

class CWProjectTree;

class CWProjectFolderNameEditor : public CWEditor
{
Q_OBJECT
 public:
  CWProjectFolderNameEditor(CWProjectTree *projectTree, QTreeWidgetItem *item, bool newFolder,
                            QWidget *parent = 0);
  virtual ~CWProjectFolderNameEditor();

  virtual bool actionOk();
  virtual void actionHelp();

 public slots:
  void slotNameChanged(const QString &text);

 private:
  QLineEdit *m_folderName;
  CWProjectTree *m_projectTree;
  QStringList m_path;
  bool m_newFolder;
};

#endif


#include <QVBoxLayout>
#include <QLabel>

#include "CWProjectTree.h"
#include "CWProjectFolderNameEditor.h"


CWProjectFolderNameEditor::CWProjectFolderNameEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                                                     bool newFolder, QWidget *parent) :
  CWEditor(parent),
  m_projectTree(projectTree),
  m_newFolder(newFolder)
{
  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  mainLayout->setMargin(25);
  mainLayout->setSpacing(5);;

  if (newFolder)
    mainLayout->addWidget(new QLabel("Enter a name for the new Folder", this));
  else
    mainLayout->addWidget(new QLabel("Modify the Folder name", this));

  m_folderName = new QLineEdit(this);
  mainLayout->addWidget(m_folderName);
  
  if (m_newFolder)
    m_captionStr = "Create new Folder in ";
  else
    m_captionStr = "Rename Folder ";
    
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
  m_contextTag += "-FolderEditor";

  // if renaming
  if (!m_newFolder && !m_path.isEmpty())
    m_folderName->setText(m_path.back());

  connect(m_folderName, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotNameChanged(const QString &)));

}

CWProjectFolderNameEditor::~CWProjectFolderNameEditor()
{
}

bool CWProjectFolderNameEditor::actionOk()
{
  // use the path to locate the correct place in the tree

  if (m_folderName->text().isEmpty())
    return false;

  QTreeWidgetItem *item = m_projectTree->locateByPath(m_path);
  if (item) {
    // still a valid point in the tree
    if (m_newFolder) {
      if (item->type() == cSpectraBranchItemType || item->type() == cSpectraFolderItemType) {
        // create a new sub-folder
        new CSpectraFolderItem(item, m_folderName->text());
      }
    }
    else {
      if (item->type() == cSpectraFolderItemType) {
        // rename
        item->setText(0, m_folderName->text());
      }
    }
  }
  else {
    // no longer exists ... message box - TODO
  }

  return true;
}

void CWProjectFolderNameEditor::actionHelp()
{
}

void CWProjectFolderNameEditor::slotNameChanged(const QString &text)
{
  notifyAcceptActionOk(!text.isEmpty());
}


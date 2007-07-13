
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

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
  connect(m_folderName, SIGNAL(returnPressed()),
          this, SLOT(slotReturnPressed()));
}

CWProjectFolderNameEditor::~CWProjectFolderNameEditor()
{
}

bool CWProjectFolderNameEditor::actionOk(void)
{
  // use the path to locate the correct place in the tree

  if (m_folderName->text().isEmpty())
    return false;

  QString msg;

  QTreeWidgetItem *item = m_projectTree->locateByPath(m_path);
  if (item) {
    // still a valid point in the tree
    if (m_newFolder)
      msg = m_projectTree->editInsertNewFolder(item, m_folderName->text());
    else
      msg = m_projectTree->editRenameFolder(item, m_folderName->text());
    
    if (msg.isNull())
      return true;

  }
  else {
    // no longer exists ...
    msg = QString("Could not locate the parent in the project tree.");
  }

  // fall through failure ...
  QMessageBox::information(this, m_newFolder ? "Insert Folder" : "Rename Folder", msg);
  return false;
}

void CWProjectFolderNameEditor::actionHelp(void)
{
}

void CWProjectFolderNameEditor::takeFocus(void)
{
  // give focus to the line edit
  m_folderName->setFocus(Qt::OtherFocusReason);
}

void CWProjectFolderNameEditor::slotNameChanged(const QString &text)
{
  notifyAcceptActionOk(!text.isEmpty());
}

void CWProjectFolderNameEditor::slotReturnPressed()
{
  shortcutActionOk();
}

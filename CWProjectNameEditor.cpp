
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

#include "CWProjectTree.h"
#include "CWProjectNameEditor.h"


CWProjectNameEditor::CWProjectNameEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                                         QWidget *parent) :
  CWEditor(parent),
  m_projectTree(projectTree)
{
  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  mainLayout->setMargin(25);
  mainLayout->setSpacing(5);;

  if (item && item->parent() == NULL) {
    // renaming a project
    m_oldProjectName = item->text(0);  // also acts as the flag for new/rename 
    mainLayout->addWidget(new QLabel("Modify the Project name", this));
  }
  else
    mainLayout->addWidget(new QLabel("Enter a name for the new Project", this));

  m_projectName = new QLineEdit(this);
  mainLayout->addWidget(m_projectName);

  if (m_oldProjectName.isNull()) {
    m_captionStr = "Create new Project";
    // no change to the default context tag
  }
  else {
    m_captionStr = "Rename Project ";
    m_captionStr += m_oldProjectName;

    m_contextTag = m_oldProjectName;
    m_contextTag += "-ProjectEditor";

    m_projectName->setText(m_oldProjectName);
  }

  connect(m_projectName, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotNameChanged(const QString &)));
  connect(m_projectName, SIGNAL(returnPressed()),
          this, SLOT(slotReturnPressed()));

}

CWProjectNameEditor::~CWProjectNameEditor()
{
}

bool CWProjectNameEditor::actionOk(void)
{
  if (m_projectName->text().isEmpty())
    return false;

  QString msg;

  if (m_oldProjectName.isNull()) {
    // new project
    msg = m_projectTree->editInsertNewProject(m_projectName->text());
  }
  else {
    // renaming
    QTreeWidgetItem *item = m_projectTree->locateProjectByName(m_oldProjectName);
    if (item)
      msg = m_projectTree->editRenameProject(item, m_projectName->text());
    else
      msg = QString("The project no longer exists.");
  }

  if (msg.isNull())
    return true;

  // fall through failure ...
  QMessageBox::information(this, m_oldProjectName.isNull() ? "Insert Project" : "Rename Project", msg);
  return false;
}

void CWProjectNameEditor::actionHelp(void)
{
}

void CWProjectNameEditor::takeFocus(void)
{
  // give focus to the line edit
  m_projectName->setFocus(Qt::OtherFocusReason);
}

void CWProjectNameEditor::slotNameChanged(const QString &text)
{
  notifyAcceptActionOk(!text.isEmpty());
}

void CWProjectNameEditor::slotReturnPressed()
{
  shortcutActionOk();
}



#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

#include "CWProjectTree.h"
#include "CWProjectDirectoryEditor.h"

CWProjectDirectoryEditor::CWProjectDirectoryEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                                                   QWidget *parent) :
  CWEditor(parent),
  m_projectTree(projectTree)
{
  QGridLayout *mainLayout = new QGridLayout(this);
  mainLayout->setMargin(25);
  mainLayout->setSpacing(5);

  // row 0 - directory name
  mainLayout->addWidget(new QLabel("Directory", this), 0, 0);

  m_directoryName = new QLineEdit(this);
  mainLayout->addWidget(m_directoryName, 0, 1);
  
  QPushButton *browseButton = new QPushButton("Browse", this);
  mainLayout->addWidget(browseButton, 0, 2);

  // row 1 - file filters
  mainLayout->addWidget(new QLabel("File filters", this), 1, 0);

  m_fileFilters = new QLineEdit(this);
  mainLayout->addWidget(m_fileFilters, 1, 1);

  // row 3 - include sub-dirs
  m_recursiveCheckBox = new QCheckBox("Include Sub-Directories", this);
  mainLayout->addWidget(m_recursiveCheckBox, 2, 0, 1, 2); // span first two columns

  m_captionStr = "Insert new directory in ";

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
  m_contextTag += "-DirectoryEditor";

  connect(browseButton, SIGNAL(clicked()), this, SLOT(slotBrowseButtonClicked()));

  connect(m_directoryName, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotDirectoryChanged(const QString &)));

}

CWProjectDirectoryEditor::~CWProjectDirectoryEditor()
{
}

bool CWProjectDirectoryEditor::actionOk()
{
  // use the path to locate the correct place in the tree

  if (m_directoryName->text().isEmpty())
    return false;

  QString msg;

  QTreeWidgetItem *item = m_projectTree->locateByPath(m_path);
  if (item) {
    // still a valid point in the tree

    msg = m_projectTree->editInsertDirectory(item, m_directoryName->text(),
					     m_fileFilters->text(),
					     (m_recursiveCheckBox->checkState() == Qt::Checked));
    if (msg.isNull())
      return true;

  }
  else {
    // no longer exists ...
    msg = "Parent folder no longer exists.";
  }

  // all errors fall through to here
  QMessageBox::information(this, "Insert Directory Failed", msg);
  return false;
}

void CWProjectDirectoryEditor::actionHelp()
{
}

void CWProjectDirectoryEditor::slotDirectoryChanged(const QString &text)
{
  notifyAcceptActionOk(!text.isEmpty());
}

void CWProjectDirectoryEditor::slotBrowseButtonClicked()
{
  // modal dialog

  QString dir = QFileDialog::getExistingDirectory(0, "Select a directory containing spectra files",
                                                  "/home"); // TODO home
  if (!dir.isEmpty())
    m_directoryName->setText(dir);
}


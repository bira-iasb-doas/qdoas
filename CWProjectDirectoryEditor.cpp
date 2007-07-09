
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QRegExp>
#include <QDir>
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

  QTreeWidgetItem *item = m_projectTree->locateByPath(m_path);
  if (item) {
    // still a valid point in the tree
    if (item->type() == cSpectraBranchItemType || item->type() == cSpectraFolderItemType) {
      // split the filter text into a list of file filter strings - an empty list means the filter is '*'
      QStringList filters;
      QString filterStr = m_fileFilters->text();

      if (!filterStr.isEmpty()) {
        if (filterStr.contains(';')) {
          // split on ';' - NOTE whitespace is significant
          filters = filterStr.split(';', QString::SkipEmptyParts);
        }
        else {
          // split on whitespace
          filters = filterStr.split(QRegExp("\\s+"));
        }
      }

      // the directory must exist
      QDir directory(m_directoryName->text());
      
      if (directory.exists()) {
        // create a new directory item
        int fileCount;
        CSpectraDirectoryItem *dirItem = new CSpectraDirectoryItem(0, directory, filters,
                                                                   (m_recursiveCheckBox->checkState() == Qt::Checked),
                                                                   &fileCount);

        if (fileCount) {
          item->addChild(dirItem);
          return true;
        }
        else {
          // empty file count ...
          delete dirItem;
          QMessageBox::information(this, "Insert Directory Failed",
                                   "No files matched the file filters specified.");
        }
      }
      else {
        QString msg = "The directory ";
        msg += m_directoryName->text();
        msg += " does not exist.";
        QMessageBox::information(this, "Insert Directory Failed", msg);
      }
                                 
    } // else - no message box - we should never have this ...
  }
  else {
    // no longer exists ... message box - TODO
    QMessageBox::information(this, "Insert Directory Failed" ,
                             "Parent folder no longer exists.");
  }

  // all errors fall through to return false 
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


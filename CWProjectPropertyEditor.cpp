
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

#include "CWProjectTree.h"
#include "CWProjectPropertyEditor.h"


CWProjectPropertyEditor::CWProjectPropertyEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                                         QWidget *parent) :
  CWEditor(parent),
  m_projectTree(projectTree)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(25);
  mainLayout->setSpacing(5);

  m_tabs = new QTabWidget(this);

  m_projectName = item->text(0);

  m_spectraTab = new CWProjectTabSpectra(&m_data);
  m_tabs->addTab(m_spectraTab, "Spectra");

  mainLayout->addWidget(m_tabs);

  notifyAcceptActionOk(true);
}

CWProjectPropertyEditor::~CWProjectPropertyEditor()
{
}

bool CWProjectPropertyEditor::actionOk()
{
  return false;
}

void CWProjectPropertyEditor::actionHelp()
{
}


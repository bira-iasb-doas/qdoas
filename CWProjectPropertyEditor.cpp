
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

#include "CWorkSpace.h"

#include "CWProjectTree.h"
#include "CWProjectPropertyEditor.h"


CWProjectPropertyEditor::CWProjectPropertyEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                                         QWidget *parent) :
  CWEditor(parent),
  m_projectTree(projectTree)
{
  m_projectName = item->text(0);
  mediate_project_t *projectData = CWorkSpace::instance()->findProject(m_projectName);
  
  if (projectData) {
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(25);
    mainLayout->setSpacing(5);
    
    m_tabs = new QTabWidget(this);
   
    // Spectra Tab
    m_spectraTab = new CWProjectTabSpectra(&(projectData->spectra));
    m_tabs->addTab(m_spectraTab, "Spectra");
    
    mainLayout->addWidget(m_tabs);
    
    notifyAcceptActionOk(true);
  }
}

CWProjectPropertyEditor::~CWProjectPropertyEditor()
{
}

bool CWProjectPropertyEditor::actionOk()
{
  // call apply for all tabs ...

  return false;
}

void CWProjectPropertyEditor::actionHelp()
{
}


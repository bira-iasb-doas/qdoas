
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
  
  if (!projectData)
    return; // TODO - assert or throw

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(25);
  mainLayout->setSpacing(5);
  
  m_tabs = new QTabWidget(this);
  
  // Spectra Tab
  m_spectraTab = new CWProjectTabSpectra(&(projectData->spectra));
  m_tabs->addTab(m_spectraTab, "Spectra");
  
  // Analysis Tab
  m_analysisTab = new CWProjectTabAnalysis(&(projectData->analysis));
  m_tabs->addTab(m_analysisTab, "Analysis");

  mainLayout->addWidget(m_tabs);

  // caption string and context tag
  m_captionStr = "Properties of Project : ";
  m_captionStr += m_projectName;

  m_contextTag = m_projectName;
  m_contextTag += "-ProjPropEditor";
  
  notifyAcceptActionOk(true);
}

CWProjectPropertyEditor::~CWProjectPropertyEditor()
{
}

bool CWProjectPropertyEditor::actionOk(void)
{
  // call apply for all tabs ...
  mediate_project_t *projectData = CWorkSpace::instance()->findProject(m_projectName);
  
  if (projectData) {
    m_spectraTab->apply(&(projectData->spectra));
    m_analysisTab->apply(&(projectData->analysis));

    return true;
  }

  // Project not found ... TODO

  return false;
}

void CWProjectPropertyEditor::actionHelp(void)
{
}


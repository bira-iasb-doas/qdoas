/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings> // TODO
#include <QCloseEvent>
#include <QApplication>

#include "CWMain.h"
#include "CWProjectTree.h"
#include "CWSiteTree.h"
#include "CWUserSymbolTree.h"
#include "CWActiveContext.h"
#include "CWSplitter.h"
#include "CWTableRegion.h"
#include "CWAboutDialog.h"

#include "CEngineController.h"
#include "CNavigationPanel.h"
#include "CQdoasProjectConfigHandler.h"
#include "CWorkSpace.h"
#include "CPreferences.h"
#include "CConfigurationWriter.h"
#include "CConfigStateMonitor.h"

#include "mediate_types.h"

#include "debugutil.h"

CWMain::CWMain(QWidget *parent) :
  QFrame(parent)
{
  setProjectFileName(QString());

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(0);
  mainLayout->setSpacing(0);

  //------------------------------
  // Menu Bar

  m_menuBar = new QMenuBar(this);
  mainLayout->addWidget(m_menuBar, 0);

  //------------------------------
  // Tool Bar

  m_toolBar = new QToolBar(this);
  QHBoxLayout *tbLayout = new QHBoxLayout;

  tbLayout->addWidget(m_toolBar, 0);
  tbLayout->addStretch(1);
  mainLayout->addLayout(tbLayout, 0);
  
  //------------------------------

  m_activeContext = new CWActiveContext;

  m_projTree = new CWProjectTree(m_activeContext);

  m_siteTree = new CWSiteTree(m_activeContext);

  m_userSymbolTree = new CWUserSymbolTree(m_activeContext);

  m_projEnvTab = new QTabWidget;
  m_projEnvTab->addTab(m_projTree, "Projects");
  m_projEnvTab->addTab(m_siteTree, "Sites");
  m_projEnvTab->addTab(m_userSymbolTree, "Symbols");

  // data table
  
  m_tableRegion = new CWTableRegion;

  // Splitters
  m_subSplitter = new CWSplitter(Qt::Horizontal, "MainSplitter");
  m_subSplitter->addWidget(m_projEnvTab);
  m_subSplitter->addWidget(m_activeContext);

  m_subSplitter->setStretchFactor(0, 0);
  m_subSplitter->setStretchFactor(1, 1);

  // connections between the Trees and the multi-mode splitter
  connect(m_projTree, SIGNAL(signalWidthModeChanged(int)), m_subSplitter, SLOT(slotSetWidthMode(int)));
  connect(m_siteTree, SIGNAL(signalWidthModeChanged(int)), m_subSplitter, SLOT(slotSetWidthMode(int)));
  connect(m_userSymbolTree, SIGNAL(signalWidthModeChanged(int)), m_subSplitter, SLOT(slotSetWidthMode(int)));

  QSplitter *mainSplitter = new QSplitter(Qt::Vertical);
  mainSplitter->addWidget(m_subSplitter);
  mainSplitter->addWidget(m_tableRegion);
  
  mainLayout->addWidget(mainSplitter, 1); // takes all stretch

  //------------------------------
  // Status Bar

  m_statusBar = new QStatusBar;

  mainLayout->addWidget(m_statusBar, 0);

  m_statusBar->showMessage("Status information ...");

  //------------------------------
  // Controller

  m_controller = new CEngineController(this);

  // connections to the controller
  connect(m_projTree, SIGNAL(signalStartSession(const RefCountPtr<CSession>&)),
          m_controller, SLOT(slotStartSession(const RefCountPtr<CSession>&)));

  // Menu and toolbar actions

  // File Menu
  QMenu *fileMenu = new QMenu("File");

  // Open...
  QAction *openAct = new QAction(QIcon(QPixmap(":/icons/file_open_16.png")), "Open...", this);
  connect(openAct, SIGNAL(triggered()), this, SLOT(slotOpenFile()));
  fileMenu->addAction(openAct);
  m_toolBar->addAction(openAct);

  // New
  QAction *newProjAction = new QAction(QIcon(QPixmap(":/icons/file_new_16.png")), "New", this);
  connect(newProjAction, SIGNAL(triggered()), this, SLOT(slotNewFile()));
  fileMenu->addAction(newProjAction);
  m_toolBar->addAction(newProjAction);

  fileMenu->addSeparator();

  // Save + Save As ...
  m_saveAction = new QAction(QIcon(QPixmap(":/icons/file_save_16.png")), "Save", this);
  m_saveAction->setEnabled(false);
  connect(m_saveAction, SIGNAL(triggered()), this, SLOT(slotSaveFile()));
  fileMenu->addAction(m_saveAction);
  m_toolBar->addAction(m_saveAction);

  m_saveAsAction = new QAction(QIcon(QPixmap(":/icons/file_saveas_16.png")), "Save As...", this);
  m_saveAsAction->setEnabled(false);
  connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(slotSaveAsFile()));
  fileMenu->addAction(m_saveAsAction);
  m_toolBar->addAction(m_saveAsAction);

  // Quit
  fileMenu->addSeparator();
  QAction *exitAct = new QAction(QIcon(QPixmap(":/icons/file_exit_16.png")), "Quit", this);
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
  fileMenu->addAction(exitAct);

  m_menuBar->addMenu(fileMenu);

  // edit (cut/copy/paste/delete)
  m_toolBar->addSeparator();
  m_toolBar->addAction(QIcon(QPixmap(":/icons/edit_cut_16.png")), "Cut", this, SLOT(slotCutButtonClicked()));
  m_toolBar->addAction(QIcon(QPixmap(":/icons/edit_copy_16.png")), "Copy", this, SLOT(slotCopyButtonClicked()));
  m_toolBar->addAction(QIcon(QPixmap(":/icons/edit_paste_16.png")), "Paste", this, SLOT(slotPasteButtonClicked()));
  m_toolBar->addAction(QIcon(QPixmap(":/icons/edit_delete_16.png")), "Delete", this, SLOT(slotDeleteButtonClicked()));

  // navigation panel
  m_toolBar->addSeparator();
  CNavigationPanel *navPanelRecords = new CNavigationPanel(m_toolBar);
  m_toolBar->addSeparator();

  // Plot menu
  QMenu *plotMenu = new QMenu("Plot");

  // plot properties
  QAction *plotPropAction = new QAction(QIcon(QPixmap(":/icons/plot_prop_22.png")), "Plot Properties...", this);
  connect(plotPropAction, SIGNAL(triggered()), m_activeContext, SLOT(slotEditPlotProperties()));
  plotMenu->addAction(plotPropAction);
  m_toolBar->addAction(plotPropAction);

  m_menuBar->addMenu(plotMenu);

  // Help Menu
  QMenu *helpMenu = new QMenu("Help");

  // About
  helpMenu->addAction("About Qdoas", this, SLOT(slotAboutQdoas()));
  helpMenu->addSeparator();
  helpMenu->addAction("About Qt", this, SLOT(slotAboutQt()));

  m_menuBar->addMenu(helpMenu);

  // state monitor
  m_stateMonitor = new CConfigStateMonitor(this);

  // connections
  connect(m_controller, SIGNAL(signalCurrentRecordChanged(int)),
	  navPanelRecords, SLOT(slotSetCurrentRecord(int)));
  connect(m_controller, SIGNAL(signalFileListChanged(const QStringList&)),
	  navPanelRecords, SLOT(slotSetFileList(const QStringList&)));
  connect(m_controller, SIGNAL(signalCurrentFileChanged(int,int)),
	  navPanelRecords, SLOT(slotSetCurrentFile(int,int)));
  connect(m_controller, SIGNAL(signalSessionRunning(bool)),
	  navPanelRecords, SLOT(slotSetEnabled(bool))); 
  connect(m_controller, SIGNAL(signalSessionRunning(bool)),
	  m_projTree, SLOT(slotSessionRunning(bool))); 

  connect(navPanelRecords, SIGNAL(signalFirstClicked()),
	  m_controller, SLOT(slotFirstRecord()));
  connect(navPanelRecords, SIGNAL(signalPreviousClicked()),
	  m_controller, SLOT(slotPreviousRecord()));
  connect(navPanelRecords, SIGNAL(signalNextClicked()),
	  m_controller, SLOT(slotNextRecord()));
  connect(navPanelRecords, SIGNAL(signalLastClicked()),
	  m_controller, SLOT(slotLastRecord()));
  connect(navPanelRecords, SIGNAL(signalRecordChanged(int)),
	  m_controller, SLOT(slotGotoRecord(int)));
  connect(navPanelRecords, SIGNAL(signalSelectedFileChanged(int)),
	  m_controller, SLOT(slotGotoFile(int)));
  connect(navPanelRecords, SIGNAL(signalStopClicked()),
	  m_controller, SLOT(slotStopSession()));
  connect(navPanelRecords, SIGNAL(signalStep()),
	  m_controller, SLOT(slotStep()));

  // plot data transfer
  connect(m_controller, SIGNAL(signalPlotPages(const QList< RefCountConstPtr<CPlotPageData> >&)),
          m_activeContext, SLOT(slotPlotPages(const QList< RefCountConstPtr<CPlotPageData> >&)));

  // table data transfer
  connect(m_controller, SIGNAL(signalTablePages(const QList< RefCountConstPtr<CTablePageData> > &)),
          m_tableRegion, SLOT(slotTablePages(const QList< RefCountConstPtr<CTablePageData> > &)));


  // error messages
  connect(m_controller, SIGNAL(signalErrorMessages(int, const QString &)),
	  this, SLOT(slotErrorMessages(int, const QString &)));

  // tab-based coupling of plot and table display
  connect(m_activeContext, SIGNAL(signalActivePageChanged(int)),
	  m_tableRegion, SLOT(slotDisplayPage(int)));

  // state invalidation driven by the project tree
  connect(m_projTree, SIGNAL(signalSpectraTreeChanged()), m_stateMonitor, SLOT(slotInvalidate()));
  connect(m_stateMonitor, SIGNAL(signalStateChanged(bool)), this, SLOT(slotStateMonitorChanged(bool)));

  // icon
  setWindowIcon(QIcon(QPixmap(":/icons/logo.png")));

  // get the window size from the settings
  resize(CPreferences::instance()->windowSize("Main", QSize(600,450)));
}

CWMain::~CWMain()
{
}

void CWMain::closeEvent(QCloseEvent *e)
{
  // save preferences ...
  CPreferences::instance()->setWindowSize("Main", size());
  m_projTree->savePreferences();
  m_siteTree->savePreferences();
  m_userSymbolTree->savePreferences();
  m_subSplitter->savePreferences();
  m_activeContext->savePreferences();

  // flush write and close ...
  delete CPreferences::instance();

  if (checkStateAndConsiderSaveFile())
    e->accept();
}

bool CWMain::checkStateAndConsiderSaveFile(void)
{
  // if the state is valid there is nothing to do - return TRUE
  if (m_stateMonitor->isValid())
    return true;

  // prompt to choose to save changes or cancel
  QString msg;

  if (m_projectFile.isEmpty()) {
    msg = "Save the current project file?";
  }
  else {
    msg = "Save changes to the project file\n";
    msg += m_projectFile;
    msg += " ?";
  }

  QMessageBox::StandardButton choice = QMessageBox::question(this, "Save Changes", msg,
							     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
							     QMessageBox::Save);
  
  if (choice == QMessageBox::Discard)
    return true; // discard changes
  else if (choice != QMessageBox::Save)
    return false; // cancel (or any other action that was not save)

  // save of saveAs (as appropriate).
  slotSaveFile();

  // a SaveAs could have been cancelled (for some reason). The stateMonitor provides that indicator.
  return m_stateMonitor->isValid();
}

void CWMain::setProjectFileName(const QString &fileName)
{
  QString str("Qdoas - ");
  
  m_projectFile = fileName;

  if (m_projectFile.isEmpty()) {
    str += "Unnamed[*]";
  }
  else {
    str += m_projectFile;
    str += "[*]";
  }

  setWindowTitle(str);
}
 
void CWMain::slotOpenFile()
{
  if (!checkStateAndConsiderSaveFile())
    return;

  CPreferences *prefs = CPreferences::instance();

  QString fileName = QFileDialog::getOpenFileName(this, "Open Project File",
						  prefs->directoryName("Project"),
						  "Qdoas Project Config (*.xml);;All Files (*)");

  if (fileName.isEmpty()) {
    return;
  }

  // save the last directory
  prefs->setDirectoryNameGivenFile("Project", fileName);

  QString errMsg;
  QFile *file = new QFile(fileName);

  // parse the file
  QXmlSimpleReader xmlReader;
  QXmlInputSource *source = new QXmlInputSource(file);

  CQdoasProjectConfigHandler *handler = new CQdoasProjectConfigHandler;
  xmlReader.setContentHandler(handler);
  xmlReader.setErrorHandler(handler);

  bool ok = xmlReader.parse(source);
  
  if (ok) {

    CWorkSpace *ws = CWorkSpace::instance();

    // start with a clear workspace ... clear the project tree, then the workspace
    m_projTree->removeAllContent();
    ws->removeAllContent();

    // repopulate the workspace and the project tree

    // store the paths in the workspace for simplification when saving ...
    for (int i = 0; i<10; ++i) {
      QString path = handler->getPath(i);
      if (path.isEmpty())
	ws->removePath(i);
      else
	ws->addPath(i, path);
    }
    
    // sites
    const QList<const CSiteConfigItem*> &siteItems = handler->siteItems();
    QList<const CSiteConfigItem*>::const_iterator siteIt = siteItems.begin();
    while (siteIt != siteItems.end()) {

      ws->createSite((*siteIt)->siteName(), (*siteIt)->abbreviation(),
		     (*siteIt)->longitude(), (*siteIt)->latitude(), (*siteIt)->altitude());
      ++siteIt;
    }
    
    // symbols
    const QList<const CSymbolConfigItem*> &symbolItems = handler->symbolItems();
    QList<const CSymbolConfigItem*>::const_iterator symIt = symbolItems.begin();
    while (symIt != symbolItems.end()) {

      ws->createSymbol((*symIt)->symbolName(), (*symIt)->symbolDescription());
      ++symIt;
    }

    // projects
    errMsg += m_projTree->loadConfiguration(handler->projectItems());

    setProjectFileName(fileName);
    m_stateMonitor->slotValidate();
    m_saveAction->setEnabled(false);
    m_saveAsAction->setEnabled(true);
  }
  else {
    errMsg = handler->messages();
  }
  delete handler;
  delete file;

  if (!errMsg.isNull())
    QMessageBox::critical(this, "File Open", errMsg);

}

void CWMain::slotNewFile()
{
  if (!checkStateAndConsiderSaveFile())
    return;
  
  // clear the project tree, then the workspace
  m_projTree->removeAllContent();
  CWorkSpace::instance()->removeAllContent();

  setProjectFileName(QString()); // no project file name
  m_stateMonitor->slotValidate();

  m_saveAction->setEnabled(false);
  m_saveAsAction->setEnabled(false);
}

void CWMain::slotSaveAsFile()
{
  QMessageBox::StandardButton returnCode = QMessageBox::Retry;

  CConfigurationWriter writer(m_projTree);

  while (returnCode == QMessageBox::Retry) {

    returnCode = QMessageBox::Cancel;
    
    QString fileName = QFileDialog::getSaveFileName(this, "SaveAs Project File",
						    CPreferences::instance()->directoryName("Project"),
						    "Qdoas Project Config (*.xml);;All Files (*)");

    // empty fileName implies cancel
    if (!fileName.isEmpty()) {
      // write the file
      QString msg = writer.write(fileName);
      if (!msg.isNull()) {
	msg += "\nPress Retry to select another output project file.";
	returnCode = QMessageBox::critical(this, "Project File Write Failure", msg,
					   QMessageBox::Retry | QMessageBox::Cancel,
					   QMessageBox::Retry);
      }
      else {
	// wrote the file ... change the project filename and validate
	setProjectFileName(fileName);
	m_stateMonitor->slotValidate();
      }
    }
  }
}

void CWMain::slotSaveFile()
{
  if (m_projectFile.isEmpty()) {
    slotSaveAsFile();
  }
  else {
    CConfigurationWriter writer(m_projTree);
    
    QString msg = writer.write(m_projectFile);
    if (!msg.isNull())
      QMessageBox::critical(this, "Project File Write Failure", msg, QMessageBox::Ok);
    else
      m_stateMonitor->slotValidate();
  }
}

void CWMain::slotCutButtonClicked()
{
  // redirect to the visible tab ...
  switch(m_projEnvTab->currentIndex()) {
  case 0: // project
    m_projTree->slotCutSelection();
    break;
    // TODO
  }
}

void CWMain::slotCopyButtonClicked()
{
  // redirect to the visible tab ...
  switch(m_projEnvTab->currentIndex()) {
  case 0: // project
    m_projTree->slotCopySelection();
    break;
    // TODO
  }
}

void CWMain::slotPasteButtonClicked()
{
  // redirect to the visible tab ...
  switch(m_projEnvTab->currentIndex()) {
  case 0: // project
    m_projTree->slotPaste();
    break;
    // TODO
  }
}

void CWMain::slotDeleteButtonClicked()
{
  // redirect to the visible tab ...
  switch(m_projEnvTab->currentIndex()) {
  case 0: // project
    m_projTree->slotDeleteSelection();
    break;
    // TODO
  }
}

void CWMain::slotStateMonitorChanged(bool valid)
{
  bool modified = !valid;

  m_saveAction->setEnabled(modified);
  m_saveAsAction->setEnabled(true);
  setWindowModified(modified);
}

void CWMain::slotAboutQdoas()
{
  CWAboutDialog dialog(this);
  dialog.exec();
}

void CWMain::slotAboutQt()
{
  QApplication::aboutQt();
}

void CWMain::slotErrorMessages(int highestLevel, const QString &messages)
{
  switch (highestLevel) {
  case InformationEngineError:
    QMessageBox::information(this, "Engine Information", messages);
    break;
  case WarningEngineError:
    QMessageBox::warning(this, "Engine Warning", messages);
    break;
  case FatalEngineError:
  default:
    QMessageBox::critical(this, "Engine Fatal Error", messages);
    break;
  }
}


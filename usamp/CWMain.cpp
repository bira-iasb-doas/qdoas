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


#include <cstring>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QApplication>

#include "CWMain.h"
#include "CWAboutDialog.h"
#include "CHelpSystem.h"
#include "CWPlotPropertiesDialog.h"

#include "CUsampEngineController.h"

#include "CPathMgr.h"
#include "CUsampConfigHandler.h"
#include "CPreferences.h"
#include "CUsampConfigWriter.h"
#include "CEngineResponse.h"

#include "mediate_types.h"
#include "mediate.h"

#include "debugutil.h"

void TODO_Junk_Test(void *engineContext, void *resp); // TODO

CWMain::CWMain(QWidget *parent) :
  QFrame(parent),
  m_plotArea(NULL)
{
  initializeMediateUsamp(&m_guiProperties);

  setConfigFileName(QString());

  // controller
  m_controller = new CUsampEngineController(this);

  // Help system
  m_helpInterface = CHelpSystem::establishHelpSystem(this); // this is the 'controller'

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(0);
  mainLayout->setSpacing(0);

  //------------------------------
  // Menu Bar

  m_menuBar = new QMenuBar(this);
  mainLayout->addWidget(m_menuBar, 0);

  // File Menu
  QMenu *fileMenu = new QMenu("File");

  // Open...
  QAction *openAct = new QAction(QIcon(QPixmap(":/icons/file_open_16.png")), "Open...", this);
  connect(openAct, SIGNAL(triggered()), this, SLOT(slotOpenFile()));
  fileMenu->addAction(openAct);

  // New
  QAction *newProjAction = new QAction(QIcon(QPixmap(":/icons/file_new_16.png")), "New", this);
  connect(newProjAction, SIGNAL(triggered()), this, SLOT(slotNewFile()));
  fileMenu->addAction(newProjAction);

  fileMenu->addSeparator();

  // Save + Save As ...
  m_saveAction = new QAction(QIcon(QPixmap(":/icons/file_save_16.png")), "Save", this);
  connect(m_saveAction, SIGNAL(triggered()), this, SLOT(slotSaveFile()));
  fileMenu->addAction(m_saveAction);

  m_saveAsAction = new QAction(QIcon(QPixmap(":/icons/file_saveas_16.png")), "Save As...", this);
  connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(slotSaveAsFile()));
  fileMenu->addAction(m_saveAsAction);

  // Quit
  fileMenu->addSeparator();
  QAction *exitAct = new QAction(QIcon(QPixmap(":/icons/file_exit_16.png")), "Quit", this);
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
  fileMenu->addAction(exitAct);

  m_menuBar->addMenu(fileMenu);

  // Calculate menu
  QMenu *calcMenu = new QMenu("Calculate");
  QAction *convAct = new QAction("Run Usamp", this);
  connect(convAct, SIGNAL(triggered()), this, SLOT(slotRunUsamp()));
  calcMenu->addAction(convAct);

  m_menuBar->addMenu(calcMenu);

  // Plot menu
  QMenu *plotMenu = new QMenu("Plot");

  // plot properties and printing
  QAction *plotPropAction = new QAction(QIcon(QPixmap(":/icons/plot_prop_22.png")), "Plot Properties...", this);
  connect(plotPropAction, SIGNAL(triggered()), this, SLOT(slotEditPlotProperties()));
  plotMenu->addAction(plotPropAction);

  QAction *plotPrintAction = new QAction(QIcon(QPixmap(":/icons/plot_print_22.png")), "Print Plots...", this);
  connect(plotPrintAction, SIGNAL(triggered()), this, SLOT(slotPrintPlots()));
  plotMenu->addAction(plotPrintAction);

  QAction *plotExportAction = new QAction(QIcon(QPixmap(":/icons/plot_export_22.png")), "Export Plots...", this);
  connect(plotExportAction, SIGNAL(triggered()), this, SLOT(slotExportPlots()));
  plotMenu->addAction(plotExportAction);

  m_menuBar->addMenu(plotMenu);

  // Help Menu
  QMenu *helpMenu = new QMenu("Help");

  // About
  helpMenu->addAction("Qdoas Manual", this, SLOT(slotQdoasHelp()));
  // dual help systems ...
  QSettings &settings = CPreferences::instance()->settings();
  m_helpInterface->preferLightBrowser(settings.value("LightHelpSys", false).toBool());

  QAction *helpCheck = new QAction("Use lightweight help", this);
  helpCheck->setCheckable(true);
  helpCheck->setChecked(m_helpInterface->isLightBrowserPreferred());
  helpCheck->setEnabled(m_helpInterface->supportsQtAssistant());
  helpMenu->addAction(helpCheck);

  helpMenu->addAction("About Qdoas", this, SLOT(slotAboutQdoas()));
  helpMenu->addSeparator();
  helpMenu->addAction("About Qt", this, SLOT(slotAboutQt()));

  connect(helpCheck, SIGNAL(triggered(bool)), this, SLOT(slotHelpBrowserPreference(bool)));

  m_menuBar->addMenu(helpMenu);

  // tab - config pages and eventually the plot.
  m_tab = new QTabWidget(this);

  m_generalTab = new CWUsampTabGeneral(&m_guiProperties);
  m_tab->addTab(m_generalTab, "General");

  mainLayout->addWidget(m_tab, 1);

  // re-read the guiProperties and synchronize ... limits on the widgets can change the initialized state.
  fetchGuiProperties();
  m_properties = m_guiProperties;

  // icon
  setWindowIcon(QIcon(QPixmap(":/icons/logo.png")));

  // get the window size from the settings
  resize(CPreferences::instance()->windowSize("UsampTool", QSize(450,350)));

  // connections
  connect(m_controller, SIGNAL(signalPlotPage(const RefCountConstPtr<CPlotPageData> &)),
	  this, SLOT(slotPlotPage(const RefCountConstPtr<CPlotPageData> &)));
  connect(m_controller, SIGNAL(signalErrorMessages(int, const QString &)),
	  this, SLOT(slotErrorMessages(int, const QString &)));
}

CWMain::~CWMain()
{
}

void CWMain::closeEvent(QCloseEvent *e)
{
  // save preferences ...
  CPreferences::instance()->setWindowSize("UsampTool", size());

  if (m_plotArea != NULL)
    CWPlotPropertiesConfig::saveToPreferences(m_plotArea->properties());
  
  // flush write and close ...
  delete CPreferences::instance();

  if (checkStateAndConsiderSaveFile()) {

    e->accept();
    return;
  }
  
  e->ignore();
}

bool CWMain::checkStateAndConsiderSaveFile(void)
{
  
  fetchGuiProperties();

  if (compareProperties())
    return true; // state unchanged

  // prompt to choose to save changes or cancel
  QString msg;

  if (m_configFile.isEmpty()) {
    msg = "Save the current configuration file?";
  }
  else {
    msg = "Save changes to the configuration file\n";
    msg += m_configFile;
    msg += " ?";
  }

  QMessageBox::StandardButton choice = QMessageBox::question(this, "Save Changes", msg,
							     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
							     QMessageBox::Save);
  
  if (choice == QMessageBox::Discard)
    return true; // discard changes
  else if (choice != QMessageBox::Save)
    return false; // cancel (or any other action that was not save)

  // save or saveAs (as appropriate).
  slotSaveFile();

  // a SaveAs could have been cancelled (for some reason).
  return compareProperties();
}

void CWMain::setConfigFileName(const QString &fileName)
{
  QString str("Usamp - ");
  
  m_configFile = fileName;

  if (m_configFile.isEmpty()) {
    str += "Unnamed[*]";
  }
  else {
    str += m_configFile;
    str += "[*]";
  }

  setWindowTitle(str);
}
 
void CWMain::fetchGuiProperties(void)
{
  // get the state of the GUI and store it in m_guiProperties. Initialize first so that
  // strings are null padded (and memcmp) can be used to test for state changes.
  initializeMediateUsamp(&m_guiProperties);

  m_generalTab->apply(&m_guiProperties);
}

bool CWMain::compareProperties(void)
{
  // see if the properties (last saved/read) and last fetched from the GUI are
  // the same. returns true if they are equivalent.

  return (memcmp(&m_properties, &m_guiProperties, sizeof(mediate_usamp_t)) == 0);
}

void CWMain::slotOpenFile()
{
  if (!checkStateAndConsiderSaveFile())
    return;

  CPreferences *prefs = CPreferences::instance();

  QString fileName = QFileDialog::getOpenFileName(this, "Open Project File",
						  prefs->directoryName("UsampConf"),
						  "Qdoas Project Config (*.xml);;All Files (*)");

  if (fileName.isEmpty()) {
    return;
  }

  // save the last directory
  prefs->setDirectoryNameGivenFile("UsampConf", fileName);

  QString errMsg;
  QFile *file = new QFile(fileName);

  // parse the file
  QXmlSimpleReader xmlReader;
  QXmlInputSource *source = new QXmlInputSource(file);

  CUsampConfigHandler *handler = new CUsampConfigHandler;
  xmlReader.setContentHandler(handler);
  xmlReader.setErrorHandler(handler);

  bool ok = xmlReader.parse(source);
  
  if (ok) {
    // start with a clear configuration
    CPathMgr *pathMgr = CPathMgr::instance();

    pathMgr->removeAll();

    // store the paths in the pathMgr for simplification when saving ...
    for (int i = 0; i<10; ++i) {
      QString path = handler->getPath(i);
      if (path.isEmpty())
	pathMgr->removePath(i);
      else
	pathMgr->addPath(i, path);
    }
    
    // copy the properties data ...
    m_guiProperties = *(handler->properties());

    // update the GUI
    m_generalTab->reset(&m_guiProperties);

    fetchGuiProperties(); // see note above about synchronization ...
    m_properties = m_guiProperties;
    
    setConfigFileName(fileName);
  }
  else {
    errMsg = handler->messages();
  }
  delete handler;
  delete source;
  delete file;

  if (!errMsg.isNull())
    QMessageBox::critical(this, "File Open", errMsg);
}

void CWMain::slotNewFile()
{
  if (!checkStateAndConsiderSaveFile())
    return;
  
  initializeMediateUsamp(&m_guiProperties);

  // update the GUI
  m_generalTab->reset(&m_guiProperties);

  fetchGuiProperties(); // see note above about synchronization ...
  m_properties = m_guiProperties;

  setConfigFileName(QString()); // no config file name
}

void CWMain::slotSaveAsFile()
{
  QMessageBox::StandardButton returnCode = QMessageBox::Retry;

  fetchGuiProperties();

  CUsampConfigWriter writer(&m_guiProperties);

  while (returnCode == QMessageBox::Retry) {

    returnCode = QMessageBox::Cancel;
    
    QString fileName = QFileDialog::getSaveFileName(this, "SaveAs Config File",
						    CPreferences::instance()->directoryName("UsampConf"),
						    "Usamp Config (*.xml);;All Files (*)");

    // empty fileName implies cancel
    if (!fileName.isEmpty()) {
      // write the file
      QString msg = writer.write(fileName);
      if (!msg.isNull()) {
	msg += "\nPress Retry to select another output configuration file.";
	returnCode = QMessageBox::critical(this, "Configuration File Write Failure", msg,
					   QMessageBox::Retry | QMessageBox::Cancel,
					   QMessageBox::Retry);
      }
      else {
	// wrote the file ... change the project filename and store the properties
	setConfigFileName(fileName);
	m_properties = m_guiProperties;
      }
    }
  }

}

void CWMain::slotSaveFile()
{

  if (m_configFile.isEmpty()) {
    slotSaveAsFile();
  }
  else {
    
    fetchGuiProperties();
    CUsampConfigWriter writer(&m_guiProperties);
    
    QString msg = writer.write(m_configFile);
    if (!msg.isNull())
      QMessageBox::critical(this, "Configuration File Write Failure", msg, QMessageBox::Ok);
    else
      m_properties = m_guiProperties;
      
  }

}

void CWMain::slotEditPlotProperties()
{
  CPlotProperties prop;

  if (m_plotArea != NULL)
    prop = m_plotArea->properties();
  else
    CWPlotPropertiesConfig::loadFromPreferences(prop);

  CWPlotPropertiesDialog dialog(prop, this);

  if (dialog.exec() == QDialog::Accepted) {

    if (m_plotArea != NULL)
      m_plotArea->setProperties(prop);
    else
      CWPlotPropertiesConfig::saveToPreferences(prop);
    
  }
}

void CWMain::slotPrintPlots()
{
  if (m_plotArea != NULL)
    m_plotArea->printPage();
}

void CWMain::slotExportPlots()
{
  if (m_plotArea != NULL)
    m_plotArea->exportPage();
}

void CWMain::slotQdoasHelp()
{
  m_helpInterface->openBrowser();
}

void CWMain::slotHelpBrowserPreference(bool light)
{
  m_helpInterface->preferLightBrowser(light);

  QSettings &settings = CPreferences::instance()->settings();
  settings.setValue("LightHelpSys", light);
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

void CWMain::slotRunUsamp()
{
  // uses a snapshot of the guiProperties ...
  fetchGuiProperties();

  // get an engine context
  void *engineContext;
  CEngineResponseTool *resp = new CEngineResponseTool;

  if (mediateRequestCreateEngineContext(&engineContext, resp) != 0) {
    delete resp;
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  //mediateRequestUsamp(&engineContext, &m_guiProperties, resp);

  TODO_Junk_Test(&engineContext, resp);

  // process the response - the controller will dispatch ...
  resp->process(m_controller);

  QApplication::restoreOverrideCursor();

  if (mediateRequestDestroyEngineContext(engineContext, resp) != 0) {
    delete resp;
    return;
  }

  delete resp;
}

void CWMain::slotPlotPage(const RefCountConstPtr<CPlotPageData> &page)
{
  if (page != 0) {
    // lazy creation of the plot tab ...
    if (!m_plotArea) {
      CPlotProperties prop;
      CWPlotPropertiesConfig::loadFromPreferences(prop);

      m_plotArea = new CWPlotArea;
      m_plotArea->setProperties(prop);
      
      m_tab->addTab(m_plotArea, "Plot");
    }

    m_plotArea->setPage(page);
    m_plotArea->show();

    m_tab->setCurrentWidget(m_plotArea);
  }
}


void TODO_Junk_Test(void *engineContext, void *resp)
{
  plot_data_t dummy;
  double xData[] = { 0.0, 1.1, 2.2, 3.3, 4.4, 5.5 };
  double yData[] = { 3.0, 4.0, 3.5, 1.1, 1.4, 3.0 };

  mediateAllocateAndSetPlotData(&dummy, xData, yData, 6, Line);

  mediateResponsePlotData(0, &dummy, 1, Spectrum, 0, "Title", "X-Label", "Y-label", resp);

  mediateReleasePlotData(&dummy);
}

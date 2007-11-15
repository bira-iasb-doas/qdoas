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

#include "CConvEngineController.h"

//#include "CQdoasConfigHandler.h"
#include "CPreferences.h"
//#include "CConfigurationWriter.h"
#include "CEngineResponse.h"

#include "mediate_types.h"
#include "mediate.h"

#include "debugutil.h"

CWMain::CWMain(QWidget *parent) :
  QFrame(parent),
  m_stateValid(true)
{
  initializeMediateConvolution(&m_properties);

  setConfigFileName(QString());

  // controller
  m_controller = new CConvEngineController(this);

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
  m_saveAction->setEnabled(false);
  connect(m_saveAction, SIGNAL(triggered()), this, SLOT(slotSaveFile()));
  fileMenu->addAction(m_saveAction);

  m_saveAsAction = new QAction(QIcon(QPixmap(":/icons/file_saveas_16.png")), "Save As...", this);
  m_saveAsAction->setEnabled(false);
  connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(slotSaveAsFile()));
  fileMenu->addAction(m_saveAsAction);

  // Quit
  fileMenu->addSeparator();
  QAction *exitAct = new QAction(QIcon(QPixmap(":/icons/file_exit_16.png")), "Quit", this);
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
  fileMenu->addAction(exitAct);

  m_menuBar->addMenu(fileMenu);

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

  // tab page
  m_pageTab = new QTabWidget(this);

  m_generalTab = new CWConvTabGeneral(&(m_properties.general));
  m_pageTab->addTab(m_generalTab, "General");

  m_slitTab = new CWConvTabSlit(&(m_properties.conslit), &(m_properties.decslit));
  m_pageTab->addTab(m_slitTab, "Slit");

  m_filteringTab = new CWFilteringEditor(&(m_properties.lowpass), &(m_properties.highpass), CWFilteringEditor::SubDivSwitch);
  m_pageTab->addTab(m_filteringTab, "Filtering");

  mainLayout->addWidget(m_pageTab, 1);

  // buttons
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->setMargin(10);

  QPushButton *helpBtn = new QPushButton("Help", this);
  buttonLayout->addWidget(helpBtn);

  buttonLayout->addStretch(1);

  QPushButton *okBtn = new QPushButton("Ok", this);
  buttonLayout->addWidget(okBtn);
  
  QPushButton *cancelBtn = new QPushButton("Cancel", this);
  buttonLayout->addWidget(cancelBtn);
  
  mainLayout->addLayout(buttonLayout, 0);

  // icon
  setWindowIcon(QIcon(QPixmap(":/icons/logo.png")));

  // get the window size from the settings
  resize(CPreferences::instance()->windowSize("Conv", QSize(450,350)));

  // connections
  connect(okBtn, SIGNAL(clicked()), this, SLOT(slotOk()));
  connect(m_controller, SIGNAL(signalErrorMessages(int, const QString &)),
	  this, SLOT(slotErrorMessages(int, const QString &)));
}

CWMain::~CWMain()
{
}

void CWMain::closeEvent(QCloseEvent *e)
{
  // save preferences ...
  CPreferences::instance()->setWindowSize("Conv", size());

  // flush write and close ...
  delete CPreferences::instance();

  if (checkStateAndConsiderSaveFile()) {

    e->accept();
  }
}

bool CWMain::checkStateAndConsiderSaveFile(void)
{
  // if the state is valid there is nothing to do - return TRUE
  if (m_stateValid)
    return true;

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
  return m_stateValid;
}

void CWMain::setConfigFileName(const QString &fileName)
{
  QString str("Qdoas - ");
  
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
  prefs->setDirectoryNameGivenFile("ConvConf", fileName);

  /*
  QString errMsg;
  QFile *file = new QFile(fileName);

  // parse the file
  QXmlSimpleReader xmlReader;
  QXmlInputSource *source = new QXmlInputSource(file);

  CQdoasConfigHandler *handler = new CQdoasConfigHandler;
  xmlReader.setContentHandler(handler);
  xmlReader.setErrorHandler(handler);

  bool ok = xmlReader.parse(source);
  
  if (ok) {
    // start with a clear workspace ... clear the project tree, then the workspace

    // repopulate the workspace and the project tree

    // store the paths in the workspace for simplification when saving ...
  }
  else {
    errMsg = handler->messages();
  }
  delete handler;
  delete file;

  if (!errMsg.isNull())
    QMessageBox::critical(this, "File Open", errMsg);
  */

}

void CWMain::slotNewFile()
{
  if (!checkStateAndConsiderSaveFile())
    return;
  
  // clear the project tree, then the workspace

  setConfigFileName(QString()); // no project file name
  m_stateValid = true;

  m_saveAction->setEnabled(false);
  m_saveAsAction->setEnabled(false);
}

void CWMain::slotSaveAsFile()
{
  /*
  QMessageBox::StandardButton returnCode = QMessageBox::Retry;

  CConvConfigurationWriter writer;

  while (returnCode == QMessageBox::Retry) {

    returnCode = QMessageBox::Cancel;
    
    QString fileName = QFileDialog::getSaveFileName(this, "SaveAs Config File",
						    CPreferences::instance()->directoryName("ConvConf"),
						    "Convolution Config (*.xml);;All Files (*)");

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
	// wrote the file ... change the project filename and validate
	setConfigFileName(fileName);
	m_stateValid = true;
      }
    }
  }
  */
}

void CWMain::slotSaveFile()
{
  /*
  if (m_configFile.isEmpty()) {
    slotSaveAsFile();
  }
  else {
    CConvConfigurationWriter writer;
    
    QString msg = writer.write(m_configFile);
    if (!msg.isNull())
      QMessageBox::critical(this, "Configuration File Write Failure", msg, QMessageBox::Ok);
    else
      m_stateValid = true;
  }
  */
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

void CWMain::slotOk()
{
  // get an engine context
  void *engineContext;
  CEngineResponse *resp = new CEngineResponseMessage; // TODO ...

  if (mediateRequestCreateEngineContext(&engineContext, resp) != 0) {
    delete resp;
    return;
  }

  //mediateRequestConvolution(&engineContext, &m_properties, resp);

  // process the response - the controller will dispatch ...
  //resp->process(m_controller);

  if (mediateRequestDestroyEngineContext(engineContext, resp) != 0) {
    delete resp;
    return;
  }

  delete resp;
}

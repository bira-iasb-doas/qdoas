
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>

#include "CWMain.h"
#include "CWProjectTree.h"
#include "CWSiteTree.h"
#include "CWUserSymbolTree.h"
#include "CWActiveContext.h"
#include "CWSplitter.h"

#include "CEngineController.h"
#include "CNavigationPanel.h"

// pixmaps
#include "icons/file_open_16.xpm"

CWMain::CWMain(QWidget *parent) :
  QFrame(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(0);
  mainLayout->setSpacing(0);

  //------------------------------
  // Meuu Bar

  m_menuBar = new QMenuBar(this);

  // File Menu
  QMenu *fileMenu = new QMenu("File");

  // Open...
  QAction *openAct = new QAction(QIcon(QPixmap(file_open_16_xpm)), "Open...", this);
  // connect ...
  fileMenu->addAction(openAct);

  // Quit
  fileMenu->addSeparator();
  QAction *exitAct = new QAction(QIcon(QPixmap(":/icons/file_exit_16.png")), "Quit", this);
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
  fileMenu->addAction(exitAct);

  m_menuBar->addMenu(fileMenu);

  mainLayout->addWidget(m_menuBar, 0);

  // XXX Menu

  //------------------------------
  // Tool Bar

  m_toolBar = new QToolBar(this);
  m_toolBar->addAction(openAct);

  QHBoxLayout *tbLayout = new QHBoxLayout;

  tbLayout->addWidget(m_toolBar, 0);
  tbLayout->addStretch(1);
  mainLayout->addLayout(tbLayout, 0);
  
  //------------------------------

  m_activeContext = new CWActiveContext;

  m_projTree = new CWProjectTree(m_activeContext);

  m_siteTree = new CWSiteTree(m_activeContext);
  m_siteTree->addNewSite("Stroud", "ST", 151.123, -31.567, 160.2); // TODO - remove

  m_userSymbolTree = new CWUserSymbolTree;
  m_userSymbolTree->addNewUserSymbol("BrO", "Cross Section (228K)"); // TODO - remove

  m_projEnvTab = new QTabWidget;
  m_projEnvTab->addTab(m_projTree, "Projects");
  m_projEnvTab->addTab(m_siteTree, "Sites");
  m_projEnvTab->addTab(m_userSymbolTree, "Symbols");

  m_dataWindow = new QTextEdit;
  m_dataWindow->setText("Data window text in here ...\n...\n..."); // TODO

  // Splitters
  CWSplitter *subSplitter = new CWSplitter(Qt::Horizontal);
  subSplitter->addWidget(m_projEnvTab);
  subSplitter->addWidget(m_activeContext);

  subSplitter->setStretchFactor(0, 0);
  subSplitter->setStretchFactor(1, 1);

  // connections between the Trees and the multi-mode splitter
  connect(m_projTree, SIGNAL(signalWidthModeChanged(int)), subSplitter, SLOT(slotSetWidthMode(int)));
  connect(m_siteTree, SIGNAL(signalWidthModeChanged(int)), subSplitter, SLOT(slotSetWidthMode(int)));
  connect(m_userSymbolTree, SIGNAL(signalWidthModeChanged(int)), subSplitter, SLOT(slotSetWidthMode(int)));

  QSplitter *mainSplitter = new QSplitter(Qt::Vertical);
  mainSplitter->addWidget(subSplitter);
  mainSplitter->addWidget(m_dataWindow);
  
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
  connect(m_projTree, SIGNAL(signalStartBrowseSession(RefCountPtr<CSession>)),
          m_controller, SLOT(slotStartBrowseSession(RefCountPtr<CSession>)));


  m_toolBar->addSeparator();

  CNavigationPanel *navPanelRecords = new CNavigationPanel(m_toolBar);
  
  // connections
  connect(m_controller, SIGNAL(signalNumberOfRecordsChanged(int)),
	  navPanelRecords, SLOT(slotSetMaxIndex(int)));
  connect(m_controller, SIGNAL(signalCurrentRecordChanged(int)),
	  navPanelRecords, SLOT(slotSetCurrentIndex(int)));

  connect(navPanelRecords, SIGNAL(signalFirstClicked()),
	  m_controller, SLOT(slotFirstRecord()));
  connect(navPanelRecords, SIGNAL(signalPreviousClicked()),
	  m_controller, SLOT(slotPreviousRecord()));
  connect(navPanelRecords, SIGNAL(signalNextClicked()),
	  m_controller, SLOT(slotNextRecord()));
  connect(navPanelRecords, SIGNAL(signalLastClicked()),
	  m_controller, SLOT(slotLastRecord()));
  connect(navPanelRecords, SIGNAL(signalIndexChanged(int)),
	  m_controller, SLOT(slotGotoRecord(int)));

  m_toolBar->addSeparator();

  CNavigationPanel *navPanelFiles = new CNavigationPanel(m_toolBar);
  
  // connections
  connect(m_controller, SIGNAL(signalNumberOfFilesChanged(int)),
	  navPanelFiles, SLOT(slotSetMaxIndex(int)));
  connect(m_controller, SIGNAL(signalCurrentFileChanged(int)),
	  navPanelFiles, SLOT(slotSetCurrentIndex(int)));

  connect(navPanelFiles, SIGNAL(signalFirstClicked()),
	  m_controller, SLOT(slotFirstFile()));
  connect(navPanelFiles, SIGNAL(signalPreviousClicked()),
	  m_controller, SLOT(slotPreviousFile()));
  connect(navPanelFiles, SIGNAL(signalNextClicked()),
	  m_controller, SLOT(slotNextFile()));
  connect(navPanelFiles, SIGNAL(signalLastClicked()),
	  m_controller, SLOT(slotLastFile()));
  connect(navPanelFiles, SIGNAL(signalIndexChanged(int)),
	  m_controller, SLOT(slotGotoFile(int)));


  // plot data transfer
  connect(m_controller, SIGNAL(signalPlotPages(const QList< RefCountConstPtr<CPlotPageData> >&)),
          m_activeContext, SLOT(slotPlotPages(const QList< RefCountConstPtr<CPlotPageData> >&)));

}

CWMain::~CWMain()
{
}

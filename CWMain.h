
#ifndef _CWMAIN_H_GUARD
#define _CWMAIN_H_GUARD

#include <QFrame>
#include <QMenuBar>
#include <QToolBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextEdit>

class CWProjectTree;
class CWSiteTree;
class CWUserSymbolTree;
class CWActiveContext;

class CWMain : public QFrame
{
 public:

  CWMain(QWidget *parent = 0);
  virtual ~CWMain();

 private:
  QMenuBar *m_menuBar;
  QToolBar *m_toolBar;
  QTabWidget *m_projEnvTab;

  CWProjectTree *m_projTree;
  CWSiteTree *m_siteTree;
  CWUserSymbolTree *m_userSymbolTree;

  QTextEdit *m_dataWindow;

  CWActiveContext *m_activeContext;

  QStatusBar *m_statusBar;
};

#endif

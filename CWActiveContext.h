#ifndef _CWACTIVECONTEXT_H_GUARD
#define _CWACTIVECONTEXT_H_GUARD

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QTabBar>
#include <QList>

#include "CPlotPageData.h"
#include "RefCountPtr.h"

class CWEditor;
class CWPlotRegion;

class CWActiveContext : public QFrame
{
Q_OBJECT
 public:
  CWActiveContext(QWidget *parent = 0);
  virtual ~CWActiveContext();

  void addEditor(CWEditor *editor); // reparents and takes ownership of editor.
  
 protected:
  virtual bool event(QEvent *e);
  virtual void resizeEvent(QResizeEvent *e);

  virtual QSize minimumSizeHint() const;
  virtual QSize sizeHint() const;

 public slots:
  void slotOkButtonClicked();
  void slotCancelButtonClicked();
  void slotHelpButtonClicked();
  
  void slotAcceptOk(bool canDoOk);

  void slotPlotPages(const QList< RefCountConstPtr<CPlotPageData> > &pageList);


 private:
  void discardCurrentEditor(void);
  void moveAndResizeButtons(int wid, int hei);
  void moveAndResizeGraph(int wid, int hei);
  void moveAndResizeActiveEditor(int wid);

 private:
  CWEditor *m_activeEditor;
  QList<CWEditor*> m_editorStack;
  QPushButton *m_helpButton, *m_okButton, *m_cancelButton;
  QLabel *m_title;
  QTabBar *m_graphTab;
  CWPlotRegion *m_plotRegion;
  QString m_graphTitleStr;

  int m_titleRegionHeight;
  int m_buttonRegionHeight;
  int m_tabRegionHeight;
  int m_centralRegionHeight;

  QSize m_minGeneralSize, m_minEditSize;
};

#endif

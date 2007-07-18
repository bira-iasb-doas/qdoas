#ifndef _CNAVIGATIONPANEL_H_GUARD
#define _CNAVIGATIONPANEL_H_GUARD

#include <QObject>
#include <QAction>
#include <QToolBar>
#include <QLineEdit>

// provide a set of tool-bar actions for navigation

class CNavigationPanel : public QObject
{
Q_OBJECT
 public:
  enum NavButton { eFirst, ePrevious, eNext, eLast };

  CNavigationPanel(QToolBar *toolBar);
  virtual ~CNavigationPanel();

 public slots:
  void slotSetMaxIndex(int maxIndex);
  void slotSetCurrentIndex(int index);

 private slots:
  void slotFirstClicked();
  void slotPreviousClicked();
  void slotNextClicked();
  void slotLastClicked();
  void slotIndexEditChanged();

 signals:
  void signalFirstClicked();
  void signalPreviousClicked();
  void signalNextClicked();
  void signalLastClicked();
  void signalIndexChanged(int);

 private:
  QAction *m_firstBtn, *m_prevBtn, *m_nextBtn, *m_lastBtn;
  QLineEdit *m_indexEdit;
  
  int m_maxIndex, m_currentIndex;
};

#endif

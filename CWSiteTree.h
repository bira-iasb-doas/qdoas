#ifndef _CWSITETREE_H_GUARD
#define _CWSITETREE_H_GUARD

#include <QTreeWidget>

class CWActiveContext;

class CWSiteTree : public QTreeWidget
{
Q_OBJECT
 public:
  CWSiteTree(CWActiveContext *activeContext, QWidget *parent = 0);
  virtual ~CWSiteTree();

  void addNewSite(const QString &siteName, const QString &abreviation = QString(),
                  double longitude = 0.0, double latitude = 0.0, double altitude = 0.0);
  void modifySite(const QString &siteName, const QString &abreviation,
                  double longitude, double latitude, double altitude);

 protected:
  virtual void contextMenuEvent(QContextMenuEvent *e);
  virtual void showEvent(QShowEvent *e);

 public slots:
   void slotAddNewSite();
   void slotEditSite();
   void slotDeleteSite();

 signals:
  void signalWidthModeChanged(int newMode);

 private:
  CWActiveContext *m_activeContext;
};

#endif

#ifndef _CWUSERSYMBOLTREE_H_GUARD
#define _CWUSERSYMBOLTREE_H_GUARD

#include <QTreeWidget>

class CWUserSymbolTree : public QTreeWidget
{
Q_OBJECT
 public:
  CWUserSymbolTree(QWidget *parent = 0);
  virtual ~CWUserSymbolTree();

  void addNewUserSymbol(const QString &userSymbolName, const QString &description = QString()); 

 protected:
  virtual void showEvent(QShowEvent *e);

 signals:
  void signalWidthModeChanged(int newMode);

};

#endif

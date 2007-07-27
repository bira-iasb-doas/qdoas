#ifndef _CWTABLEREGION_H_GUARD
#define _CWTABLEREGION_H_GUARD

#include <QTableView>
#include <QList>

#include "CMultiPageTableModel.h"

class CWTableRegion : public QTableView
{
Q_OBJECT
 public:
  CWTableRegion(QWidget *parent = 0);
  virtual ~CWTableRegion();

 public slots:
  void slotTablePages(const QList< RefCountConstPtr<CTablePageData> > &pageList);
  void slotDisplayPage(int pageNumber);

 private:
  CMultiPageTableModel *m_model;
  
};

#endif

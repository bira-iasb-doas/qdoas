
#include "CWTableRegion.h"

CWTableRegion::CWTableRegion(QWidget *parent) :
  QTableView(parent)
{
  m_model = new CMultiPageTableModel(this);
  setModel(m_model);

  setGridStyle(Qt::DotLine);
}

CWTableRegion::~CWTableRegion()
{
}

void CWTableRegion::slotTablePages(const QList< RefCountConstPtr<CTablePageData> > &pageList)
{
  m_model->slotTablePages(pageList);
}

void CWTableRegion::slotDisplayPage(int pageNumber)
{
  m_model->setActivePage(pageNumber);

  resizeRowsToContents();
}

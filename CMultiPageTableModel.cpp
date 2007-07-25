#include "CMultiPageTableModel.h"

#include "debugutil.h"

CMultiPageTableModel::CMultiPageTableModel(QObject *parent) :
  QAbstractTableModel(parent)
{
}

CMultiPageTableModel::~CMultiPageTableModel()
{
}

void CMultiPageTableModel::addPage(const RefCountConstPtr<CTablePageData> &page)
{
  if (page == 0) return;

  // must not already exist
  std::map< int,RefCountConstPtr<CTablePageData> >::iterator it = m_pageMap.find(page->pageNumber());
  if (it == m_pageMap.end())
    m_pageMap.insert(std::map< int,RefCountConstPtr<CTablePageData> >::value_type(page->pageNumber(), page));

}

void CMultiPageTableModel::removeAllPages(void)
{
  if (m_currentPage != 0) {
    // notify that the active page is going to disapear ...
    int lastRow = m_currentPage->rowCount() - 1;
    int lastCol = m_currentPage->columnCount() - 1;
    beginRemoveRows(QModelIndex(), 0, lastRow);
    m_pageMap.clear();
    m_currentPage = RefCountConstPtr<CTablePageData>();
    endRemoveRows();
    beginRemoveColumns(QModelIndex(), 0, lastCol);
    endRemoveColumns();
  }
  else {
    m_pageMap.clear();
  }
} 

void CMultiPageTableModel::setActivePage(int pageNumber)
{
  int lastRow, lastCol;

  if (m_currentPage != 0) {
    // notify that the active page is going to disapear ...
    lastRow = m_currentPage->rowCount() - 1;
    lastCol = m_currentPage->columnCount() - 1;
    beginRemoveColumns(QModelIndex(), 0, lastCol);
    beginRemoveRows(QModelIndex(), 0, lastRow);
    m_currentPage = RefCountConstPtr<CTablePageData>();
    endRemoveRows();
    endRemoveColumns();
  }

  std::map< int,RefCountConstPtr<CTablePageData> >::iterator it = m_pageMap.find(pageNumber);
  if (it != m_pageMap.end()) {
    lastRow = (it->second)->rowCount() - 1;
    lastCol = (it->second)->columnCount() - 1;
    beginInsertColumns(QModelIndex(), 0, lastCol);
    beginInsertRows(QModelIndex(), 0, lastRow);
    m_currentPage = (it->second);
    endInsertRows();
    endInsertColumns();
  }
}

int CMultiPageTableModel::columnCount(const QModelIndex &parent) const
{
  if (m_currentPage != 0) {
    TRACE("columnCount = " <<  m_currentPage->columnCount());
    return m_currentPage->columnCount();
  }
  TRACE("columnCount = *** 0");
  return 0;
}

int CMultiPageTableModel::rowCount(const QModelIndex &parent) const
{
  if (m_currentPage != 0) {
    TRACE("rowCount = " <<  m_currentPage->rowCount());
    return m_currentPage->rowCount();
  }
  TRACE("rowCount = *** 0");
  return 0;
}

QVariant CMultiPageTableModel::data(const QModelIndex &index, int role) const
{
  TRACE2("Data ("<<index.row()<<","<<index.column()<<") ...");

  if (role == Qt::DisplayRole && m_currentPage != 0)
    return m_currentPage->cellData(index.row(), index.column());

  return QVariant();
}

// TODO
void CMultiPageTableModel::slotTablePages(const QList< RefCountConstPtr<CTablePageData> > &pageList)
{
  removeAllPages();

  QList< RefCountConstPtr<CTablePageData> >::const_iterator it = pageList.begin();
  while (it != pageList.end()) {
    addPage(*it);
    ++it;
  }

  setActivePage(0);
}


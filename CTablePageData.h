#ifndef _CTABLEPAGEDATA_H_GUARD
#define _CTABLEPAGEDATA_H_GUARD

#include <map>

#include <QVariant>

struct SCell
{
  int page, row, col;
  QVariant data;

  SCell(int p, int r, int c, const QVariant &d) : page(p), row(r), col(c), data(d) {}
};

struct SCellIndex
{
  int r, c;

  SCellIndex(int row, int col) : r(row), c(col) {}  
  bool operator<(const SCellIndex &rhs) const { return (r<rhs.r || (r==rhs.r && c<rhs.c)); }
};

class CTablePageData
{
 public:
  CTablePageData(int pageNumber);
  ~CTablePageData();

  void addCell(int row, int col, const QVariant &data);
  
  int pageNumber(void) const;
  int rowCount(void) const;
  int columnCount(void) const;

  bool isEmpty(void) const;
  
  QVariant cellData(int row, int col) const;

 private:
  std::map<SCellIndex,QVariant> m_dataMap;
  int m_pageNumber;
  int m_rows, m_columns;
};

inline int CTablePageData::pageNumber(void) const { return m_pageNumber; }
inline int CTablePageData::rowCount(void) const { return m_rows + 1; }
inline int CTablePageData::columnCount(void) const { return m_columns + 1; }

inline bool CTablePageData::isEmpty(void) const { return m_dataMap.empty(); }

#endif

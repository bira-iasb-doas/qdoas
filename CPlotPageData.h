#ifndef _CPLOTPAGEDATA_H_GUARD
#define _CPLOTPAGEDATA_H_GUARD

// storage class for CPlotDataSets grouped into a single page

#include <QList>

#include "CPlotDataSet.h"
#include "RefCountPtr.h"

class CPlotPageData
{
 public:
  CPlotPageData(int pageNumber);
  ~CPlotPageData();

  int pageNumber(void) const;
  int size(void) const;
  const QString& title(void) const;
  const QString& tag(void) const;
  RefCountConstPtr<CPlotDataSet> dataSet(int index) const;

  void setTitle(const QString &title);
  void setTag(const QString &tag);
  void addPlotDataSet(const CPlotDataSet *dataSet); // page takes ownership responsibility

 private:
  int m_pageNumber;
  QString m_title, m_tag;
  QList< RefCountConstPtr<CPlotDataSet> > m_dataSets;
};

#endif


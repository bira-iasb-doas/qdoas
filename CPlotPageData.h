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
  RefCountConstPtr<CPlotDataSet> dataSet(int index) const;

  void addPlotDataSet(const CPlotDataSet *dataSet); // page takes ownership responsibility
  
 private:
  int m_pageNumber;
  QList< RefCountConstPtr<CPlotDataSet> > m_dataSets;
};

#endif



#include "CPlotPageData.h"


CPlotPageData::CPlotPageData(int pageNumber) :
  m_pageNumber(pageNumber)
{
  // default tag
  m_tag.sprintf("Tag-%d", pageNumber);
}

CPlotPageData::~CPlotPageData()
{
  // the list destructor and ref-counting ensure safe handling of the plot data.
}

int CPlotPageData::pageNumber(void) const
{
  return m_pageNumber;
}

int CPlotPageData::size(void) const
{
  return m_dataSets.size();
}

const QString& CPlotPageData::title(void) const
{
  return m_title;
}

const QString& CPlotPageData::tag(void) const
{
  return m_tag;
}

RefCountConstPtr<CPlotDataSet> CPlotPageData::dataSet(int index) const
{
  if (index < 0 || index > m_dataSets.size())
    return RefCountConstPtr<CPlotDataSet>();

  return m_dataSets.at(index);
}

void CPlotPageData::setTitle(const QString &title)
{
  m_title = title;
}

void CPlotPageData::setTag(const QString &tag)
{
  m_tag = tag;
}

void CPlotPageData::addPlotDataSet(const CPlotDataSet *dataSet)
{
  // page takes ownership responsibility, which means it is safe
  // to wrap it in a reference counting pointer

  m_dataSets.push_back(RefCountConstPtr<CPlotDataSet>(dataSet));
}


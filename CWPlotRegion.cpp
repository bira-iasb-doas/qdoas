
#include <QResizeEvent>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include "CWPlotRegion.h"
#include "CWPlotPage.h"

CWPlotRegion::CWPlotRegion(QWidget *parent) :
  QScrollArea(parent),
  m_plotPage(NULL)
{
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  // TODO
  m_plotPage = new CWPlotPage(2);
  double x1[] = {0.0, 1.0, 2.0, 3.0};
  double y1[] = {1.0, 1.4, 0.7, 1.0};
  RefCountConstPtr<CPlotDataSet> c1(new CPlotDataSet(x1,y1,4,"fred","jack","john"));

  double x2[] = {0.0, 1.0, 2.0, 3.0};
  double y2[] = {1.0, -1.4, 0.7, -1.0};
  RefCountConstPtr<CPlotDataSet> c2(new CPlotDataSet(x2,y2,4,"guff","lambda","Intensity"));

  m_plotPage->addPlot(c1);
  m_plotPage->addPlot(c2);

  setWidget(m_plotPage);
}

CWPlotRegion::~CWPlotRegion()
{

}

void CWPlotRegion::removeAllPages()
{
  m_pageMap.clear();

  m_plotPage = NULL;
  setWidget(m_plotPage); // deletes the current 'viewport widget'
}

void CWPlotRegion::addPage(const RefCountConstPtr<CPlotPageData> &page)
{
  // the page must not already exist
  std::map< int,RefCountConstPtr<CPlotPageData> >::iterator it = m_pageMap.find(page->pageNumber());
  if (it == m_pageMap.end())
    m_pageMap.insert(std::map< int,RefCountConstPtr<CPlotPageData> >::value_type(page->pageNumber(), page));
  
  // else just quietly allow page to be discarded
}

void CWPlotRegion::displayPage(int pageNumber, int columns)
{
  std::map< int,RefCountConstPtr<CPlotPageData> >::iterator it = m_pageMap.find(pageNumber);
  if (it != m_pageMap.end()) {
    m_plotPage = new CWPlotPage(columns, it->second);
    setWidget(m_plotPage); // takes care of deleting the old widget
    m_plotPage->layoutPlots(m_visibleSize);
    m_plotPage->show();
  }
}

void CWPlotRegion::resizeEvent(QResizeEvent *e)
{
  m_visibleSize = e->size();

  if (m_plotPage)
    m_plotPage->layoutPlots(m_visibleSize);
}


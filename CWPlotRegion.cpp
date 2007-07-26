
#include <QResizeEvent>
#include <QLabel>
#include <QPixmap>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include "CWPlotRegion.h"
#include "CWPlotPage.h"

CWPlotRegion::CWPlotRegion(QWidget *parent) :
  QScrollArea(parent),
  m_plotPage(NULL),
  m_activePageNumber(-1)
{
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  QLabel *splash = new QLabel;
  splash->setPixmap(QPixmap(":/images/splash.png"));


  setWidget(splash);
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
    m_activePageNumber = pageNumber;
    m_plotPage = new CWPlotPage(columns, it->second);
    setWidget(m_plotPage); // takes care of deleting the old widget
    m_plotPage->layoutPlots(m_visibleSize);
    m_plotPage->show();
  }
  else {
    m_activePageNumber = -1; // invalid page number
    setWidget((m_plotPage = NULL));
  }
}

int CWPlotRegion::pageDisplayed(void) const
{
  return m_activePageNumber;
}

QString CWPlotRegion::pageTitle(int pageNumber) const
{
  std::map< int,RefCountConstPtr<CPlotPageData> >::const_iterator it = m_pageMap.find(pageNumber);
  if (it != m_pageMap.end())
    return (it->second)->title();

  return QString();
}

QString CWPlotRegion::pageTag(int pageNumber) const
{
  std::map< int,RefCountConstPtr<CPlotPageData> >::const_iterator it = m_pageMap.find(pageNumber);
  if (it != m_pageMap.end())
    return (it->second)->tag();

  return QString();
}

void CWPlotRegion::resizeEvent(QResizeEvent *e)
{
  m_visibleSize = e->size();

  if (m_plotPage)
    m_plotPage->layoutPlots(m_visibleSize);
}


#ifndef _CWPLOTREGION_H_GUARD
#define _CWPLOTREGION_H_GUARD

#include <QScrollArea>
#include <QSize>

#include "CPlotPageData.h"
#include "RefCountPtr.h"

class CWPlotPage;

class CWPlotRegion : public QScrollArea
{
 public:
  CWPlotRegion(QWidget *parent = 0);
  virtual ~CWPlotRegion();

  void removeAllPages();
  void addPage(const RefCountConstPtr<CPlotPageData> &page);
  
  void displayPage(int pageNumber, int columns);

 protected:
  void resizeEvent(QResizeEvent *e);

 private:
  CWPlotPage *m_plotPage;
  std::map< int,RefCountConstPtr<CPlotPageData> > m_pageMap;
  int m_columns;
  QSize m_visibleSize;
};

#endif

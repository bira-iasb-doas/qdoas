#ifndef _CWPLOTREGION_H_GUARD
#define _CWPLOTREGION_H_GUARD

#include <QScrollArea>
#include <QString>
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

  int pageDisplayed(void) const;
  QString pageTitle(int pageNumber) const;
  QString pageTag(int pageNumber) const;

 protected:
  void resizeEvent(QResizeEvent *e);

 private:
  CWPlotPage *m_plotPage;
  std::map< int,RefCountConstPtr<CPlotPageData> > m_pageMap;
  int m_activePageNumber;
  QSize m_visibleSize;
};

#endif

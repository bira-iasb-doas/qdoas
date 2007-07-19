#ifndef _CWPLOTPAGE_H_GUARD
#define _CWPLOTPAGE_H_GUARD

#include <QFrame>
#include <QList>

#include <qwt_plot.h>

#include "CPlotDataSet.h"
#include "RefCountPtr.h"

class CWPlot : public QwtPlot
{
 public:
  CWPlot(RefCountPtr<const CPlotDataSet> dataSet, QWidget *parent = 0);
  virtual ~CWPlot();

 private:
  RefCountPtr<const CPlotDataSet> m_dataSet;
};

class CWPlotPage : public QFrame
{
 public:
  CWPlotPage(int columns, QWidget *parent = 0);
  virtual ~CWPlotPage();

  void addPlot(RefCountPtr<const CPlotDataSet> dataSet);
  void layoutPlots(int width);

 protected:
  //  virtual void resizeEvent(QResizeEvent *e);

 private:
  int m_columns;
  QList<CWPlot*> m_plots;
};

#endif

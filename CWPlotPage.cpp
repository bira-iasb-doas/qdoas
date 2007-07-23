
#include <QColor>

#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include "CWPlotPage.h"

CWPlot::CWPlot(RefCountConstPtr<CPlotDataSet> dataSet, QWidget *parent) :
  QwtPlot(parent),
  m_dataSet(dataSet)
{
  setTitle(m_dataSet->plotTitle());
  setAxisTitle(QwtPlot::xBottom, m_dataSet->xAxisLabel());
  setAxisTitle(QwtPlot::yLeft, m_dataSet->yAxisLabel());

  QwtPlotCurve *curve = new QwtPlotCurve();
  curve->setData(m_dataSet->lines());
  curve->attach(this);

  if (m_dataSet->hasPointData()) {
    curve = new QwtPlotCurve();
    curve->setData(m_dataSet->points());
    curve->setStyle(QwtPlotCurve::NoCurve);
    QwtSymbol sym = curve->symbol();
    sym.setStyle(QwtSymbol::Ellipse);
    curve->setSymbol(sym);
    curve->attach(this);
  }

  setCanvasBackground(QColor(0xffffffff));

  replot();
}

CWPlot::~CWPlot()
{
}

CWPlotPage::CWPlotPage(int columns, QWidget *parent) :
  QFrame(parent),
  m_columns(columns)
{
  if (m_columns < 1) m_columns = 1;
}

CWPlotPage::CWPlotPage(int columns, RefCountConstPtr<CPlotPageData> page, QWidget *parent) :
  QFrame(parent),
  m_columns(columns)
{
  if (m_columns < 1) m_columns = 1;

  if (page != 0) {

    int nPlots = page->size();
    int i = 0;
    while (i < nPlots) {
      CWPlot *tmp = new CWPlot(page->dataSet(i), this);
      tmp->hide();
      m_plots.push_back(tmp);
      ++i;
    }
  }
}

CWPlotPage::~CWPlotPage()
{
}

void CWPlotPage::addPlot(RefCountConstPtr<CPlotDataSet> dataSet)
{
  CWPlot *tmp = new CWPlot(dataSet, this);
  tmp->hide();
  m_plots.push_back(tmp);
}

void CWPlotPage::layoutPlots(const QSize &visibleSize)
{
  const int cBorderSize = 10;

  // MUST have at least one plot for this to be meaningful
  if (m_plots.size() == 0)
    return;

  // work out the layout ... and size ...

  int columns;
  QSize unitSize, minUnitSize;

  QList<CWPlot*>::iterator it = m_plots.begin();
  while (it != m_plots.end()) {
    minUnitSize = minUnitSize.expandedTo((*it)->minimumSizeHint());
    ++it;
  }

  if (m_plots.size() == 1) {
    // only one plot ... fit to the visible area
    unitSize = visibleSize;
    unitSize -= QSize(2 * cBorderSize, 2 * cBorderSize);
    columns = 1;
  }
  else {
    // calculate the size that fits nicely to the full width
    // make a local change to columns if too few plots to fill them...
    columns = (m_plots.size() < m_columns) ? m_plots.size() : m_columns;
    

    unitSize.setWidth((visibleSize.width() - cBorderSize * (columns+1)) / columns);
    // want 3:4 ratio
    unitSize.setHeight(3 * unitSize.width() / 4);
  }

  // respect the minimum size
  if (minUnitSize.isValid()) {
    if (unitSize.isValid())
      unitSize = unitSize.expandedTo(minUnitSize);
    else
      unitSize = minUnitSize;
  }
  
  // position and resize
  int fitWidth = unitSize.width() + cBorderSize;
  int fitHeight = unitSize.height() + cBorderSize;
  
  int col = 0;
  int row = 0;
  it = m_plots.begin();
  while (it != m_plots.end()) {
    (*it)->move(col * fitWidth + cBorderSize, row * fitHeight + cBorderSize);
    (*it)->resize(unitSize);
    (*it)->show();
    if (++col == m_columns) {
      col = 0;
      ++row;
    }
    ++it;
  }
  // resize the plot page
  resize(columns*fitWidth + cBorderSize, (row + (col?1:0)) * fitHeight + cBorderSize);
}

#include <iostream> // TODO

#include <QColor>

#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include "CWPlotPage.h"

CWPlot::CWPlot(RefCountPtr<const CPlotDataSet> dataSet, QWidget *parent) :
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
}

CWPlotPage::~CWPlotPage()
{
}

void CWPlotPage::addPlot(RefCountPtr<const CPlotDataSet> dataSet)
{
  CWPlot *tmp = new CWPlot(dataSet, this);

  tmp->hide();

  m_plots.push_back(tmp);
}

void CWPlotPage::layoutPlots(int fullWidth)
{
  const int cBorderSize = 10;

  // work out the layout ... and size ...

  QSize unitSize, minUnitSize;

  QList<CWPlot*>::iterator it = m_plots.begin();
  while (it != m_plots.end()) {
    unitSize = unitSize.expandedTo((*it)->sizeHint());
    minUnitSize = minUnitSize.expandedTo((*it)->minimumSizeHint());
    ++it;
  }

  if (unitSize.isValid()) {

    unitSize.setHeight((int)(0.75 * (double)unitSize.width())); // want 3:4 ratio

    // calculate the scale factor that will use the full width available.
    double ratio = (double)(fullWidth - cBorderSize * (m_columns+2)) / (double)(m_columns * unitSize.width());

    // respect the minimum size
    if (minUnitSize.isValid()) {
      if (ratio > 0.0) {
        unitSize *= ratio;
        unitSize = unitSize.expandedTo(minUnitSize);
      }
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
    resize(m_columns*fitWidth + cBorderSize, (row + (col?1:0)) * fitHeight + cBorderSize);
  }
}

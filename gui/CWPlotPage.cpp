/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <QColor>
#include <QMouseEvent>

#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_legend_item.h>

#include "CWPlotPage.h"

CWPlot::CWPlot(const RefCountConstPtr<CPlotDataSet> &dataSet,
	       const CPlotProperties &plotProperties, QWidget *parent) :
  QwtPlot(parent),
  m_dataSet(dataSet),
  m_zoomer(NULL)
{
  setTitle(m_dataSet->plotTitle());
  setAxisTitle(QwtPlot::xBottom, m_dataSet->xAxisLabel());
  setAxisTitle(QwtPlot::yLeft, m_dataSet->yAxisLabel());

    // curves ...
  
  int n = m_dataSet->count();
  int i = 0;

  // add a legend if more than 1 curve plotted ...
  if (n > 1) {
    insertLegend(new QwtLegend, QwtPlot::BottomLegend);
  }

  while (i < n) {
    QwtPlotCurve *curve = new QwtPlotCurve(m_dataSet->legend(i));

    curve->setData(m_dataSet->curve(i));

    // configure curve's pen color based on type
    curve->setPen(plotProperties.pen(m_dataSet->type(i)));

    if (m_dataSet->type(i) == PlotDataType_Points) {
      curve->setStyle(QwtPlotCurve::NoCurve);
      QwtSymbol sym = curve->symbol();
      sym.setStyle(QwtSymbol::Ellipse);
      curve->setSymbol(sym);
    }

    curve->attach(this);
    ++i;
  }
    
  setCanvasBackground(plotProperties.backgroundColour());

  replot();
}


CWPlot::~CWPlot()
{
}

void CWPlot::mousePressEvent(QMouseEvent *e)
{
  // lazy instanciate the zoomer ...
  if (!m_zoomer) {
    QwtPlotCanvas *c = canvas();
    m_zoomer = new QwtPlotZoomer(c);
    c->setCursor(Qt::ArrowCursor); // change the cursor in indicate the zooming is active
    // contrasting colour ...
    m_zoomer->setRubberBandPen(QPen((canvasBackground().value() < 128) ? Qt::white : Qt::black));
  }

  QwtPlot::mousePressEvent(e);
}

CWPlotPage::CWPlotPage(const CPlotProperties &plotProperties, int columns, QWidget *parent) :
  QFrame(parent),
  m_plotProperties(plotProperties),
  m_columns(columns)
{
  if (m_columns < 1) m_columns = 1;
}

CWPlotPage::CWPlotPage(const CPlotProperties &plotProperties, int columns,
		       const RefCountConstPtr<CPlotPageData> &page, QWidget *parent) :
  QFrame(parent),
  m_plotProperties(plotProperties),
  m_columns(columns)
{
  if (m_columns < 1) m_columns = 1;

  if (page != 0) {

    int nPlots = page->size();
    int i = 0;
    while (i < nPlots) {
      CWPlot *tmp = new CWPlot(page->dataSet(i), m_plotProperties, this);
      tmp->hide();
      m_plots.push_back(tmp);
      ++i;
    }
  }
}

CWPlotPage::~CWPlotPage()
{
}

void CWPlotPage::addPlot(const RefCountConstPtr<CPlotDataSet> &dataSet)
{
  CWPlot *tmp = new CWPlot(dataSet, m_plotProperties, this);
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

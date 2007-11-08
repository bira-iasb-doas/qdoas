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
#include <QContextMenuEvent>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QMenu>

#include <QMessageBox>
#include <QTextStream>

#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_legend_item.h>

#include "CWPlotPage.h"

#include "debugutil.h"

CWPlot::CWPlot(const RefCountConstPtr<CPlotDataSet> &dataSet,
	       CPlotProperties &plotProperties, QWidget *parent) :
  QwtPlot(parent),
  m_dataSet(dataSet),
  m_plotProperties(plotProperties),
  m_zoomer(NULL)
{
  setFocusPolicy(Qt::ClickFocus); // TODO - prevents keyPressEvent

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

void CWPlot::contextMenuEvent(QContextMenuEvent *e)
{
  // position dependent
  if (childAt(e->pos()) != canvas()) {

    QMenu menu;
    
    if (m_zoomer)
      menu.addAction("Non-Interactive", this, SLOT(slotToggleInteraction()));
    else
      menu.addAction("Interactive", this, SLOT(slotToggleInteraction()));
    menu.addSeparator();
    menu.addAction("Overlay...", this, SLOT(slotOverlay()));
    menu.addAction("Save As...", this, SLOT(slotSaveAs()));
    menu.addAction("Print...", this, SLOT(slotPrint()));
    
    menu.exec(e->globalPos()); // slot will do the rest
    
    e->accept();
  }
  else
    e->ignore();
}

void CWPlot::slotOverlay()
{
  TRACE("TODO");
}

void CWPlot::slotSaveAs()
{
  QString filename = QFileDialog::getSaveFileName(this, "Save Plot", ".", "*.spe");
  
  if (!filename.isEmpty()) {
    TRACE("TODO : Save As " << filename.toStdString());
  }
}

void CWPlot::slotPrint()
{
  QPrinter printer(QPrinter::HighResolution);

  printer.setPageSize(m_plotProperties.printPaperSize());
  printer.setOrientation(QPrinter::Landscape); // single plot ALWAYS defaults to landscape
  printer.setNumCopies(1);

  QPrintDialog dialog(&printer, this);
    
  if (dialog.exec() == QDialog::Accepted) {

    // store printer preference
    m_plotProperties.setPrintPaperSize(printer.pageSize());

    QPainter p(&printer);
    p.setPen(QPen(QColor(Qt::black)));

    QRect paper = printer.paperRect();
    QRect page = printer.pageRect();
    
    QString msg;
    QTextStream str(&msg);
    
    const int cPageBorder = 150;
    
    QRect tmp(cPageBorder, cPageBorder, page.width() - 2 * cPageBorder, page.height() - 2 * cPageBorder);
    
    p.drawRect(tmp);
    
    tmp.adjust(20, 20, -20, -20);
    print(&p, tmp);
  }
}

void CWPlot::slotToggleInteraction()
{
  QwtPlotCanvas *c = canvas();

  if (m_zoomer) {
    c->setCursor(Qt::CrossCursor);
    delete m_zoomer;
    m_zoomer = NULL;
  }
  else {
    m_zoomer = new QwtPlotZoomer(c);
    c->setCursor(Qt::PointingHandCursor); // change the cursor to indicate that zooming is active
    // contrasting colour ...
    m_zoomer->setRubberBandPen(QPen((canvasBackground().value() < 128) ? Qt::white : Qt::black));
  }
}

CWPlotPage::CWPlotPage(CPlotProperties &plotProperties, QWidget *parent) :
  QFrame(parent),
  m_plotProperties(plotProperties)
{
}

CWPlotPage::CWPlotPage(CPlotProperties &plotProperties,
		       const RefCountConstPtr<CPlotPageData> &page, QWidget *parent) :
  QFrame(parent),
  m_plotProperties(plotProperties)
{
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
    columns = (m_plots.size() < m_plotProperties.columns()) ? m_plots.size() : m_plotProperties.columns();
    

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
    if (++col == m_plotProperties.columns()) {
      col = 0;
      ++row;
    }
    ++it;
  }
  // resize the plot page
  resize(columns*fitWidth + cBorderSize, (row + (col?1:0)) * fitHeight + cBorderSize);
}

void CWPlotPage::slotPrintAllPlots()
{
  // MUST have at least one plot for this to be meaningful
  if (m_plots.size() == 0)
    return;
  
  QPrinter printer(QPrinter::HighResolution);
  
  printer.setPageSize(m_plotProperties.printPaperSize());
  printer.setOrientation(m_plotProperties.printPaperOrientation());
  printer.setNumCopies(1);
  
  QPrintDialog dialog(&printer, this);
  
  if (dialog.exec() == QDialog::Accepted) {
    
    // store print preferences
    m_plotProperties.setPrintPaperSize(printer.pageSize());
    m_plotProperties.setPrintPaperOrientation(printer.orientation());

    QPainter p(&printer);
    p.setPen(QPen(QColor(Qt::black)));
    
    QRect page = printer.pageRect();
    
    // configure the plot rectangle
    const int cPageBorder = 150; // offset inside the drawable region of the page.
    const int cPlotBorder = 20;  // space between page border and plots, and between two plots.
    
    QRect tmp(cPageBorder, cPageBorder, page.width() - 2 * cPageBorder, page.height() - 2 * cPageBorder);
    
    p.drawRect(tmp);
    
    
    // work out the layout ... and size ...
    
    if (m_plots.size() == 1) {
      // only one plot ...
      tmp.adjust(cPlotBorder, cPlotBorder, -cPlotBorder, -cPlotBorder);
      m_plots.front()->print(&p, tmp);
    }
    else {
      
      // calculate the size that fits nicely to the full width
      // make a local change to columns if too few plots to fill them...
      int columns = (m_plots.size() < m_plotProperties.columns()) ? m_plots.size() : m_plotProperties.columns();
      int rows = m_plots.size() / columns + ((m_plots.size() % columns) ? 1 : 0);
      
      int unitWidth = (tmp.width() - cPlotBorder * (columns+1)) / columns;
      int unitHeight = (tmp.height() - cPlotBorder * (rows+1)) / rows;
      
      int col = 0;
      int row = 0;
      QList<CWPlot*>::iterator it = m_plots.begin();
      while (it != m_plots.end()) {
	tmp = QRect(cPageBorder + cPlotBorder + col * (cPlotBorder + unitWidth),
		    cPageBorder + cPlotBorder + row * (cPlotBorder + unitHeight),
		    unitWidth, unitHeight);
	
        (*it)->print(&p, tmp);
        if (++col == columns) {
          col = 0;
          ++row;
        }
        ++it;
      }
    }
  }
}

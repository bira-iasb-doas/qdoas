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

#include <cstdio>

#include <QColor>
#include <QContextMenuEvent>
#include <QPainter>
#include <QPrinter>
#include <QImage>
#include <QPrintDialog>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QFont>

#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_legend_item.h>
#include <qwt_plot_grid.h>

#include "CWPlotPage.h"
#include "CPreferences.h"

#include "debugutil.h"

// static helper function

bool CWPlot::getImageSaveNameAndFormat(QWidget *parent, QString &fileName, QString &saveFormat)
{
  CPreferences *pref = CPreferences::instance();

  QFileDialog dialog(parent);
  
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setWindowTitle("Export as Image");;
  dialog.setDirectory(pref->directoryName("ExportPlot"));
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setConfirmOverwrite(true);
  QStringList filters;
  filters << "PNG (*.png)" << "BMP (*.bmp)" << "JPEG (*.jpg)";
  dialog.setFilters(filters);

  if (dialog.exec()) {

    QStringList fileList = dialog.selectedFiles();
    if (!fileList.isEmpty()) {

      QString format = dialog.selectedFilter();
      fileName = fileList.first();

      // update the preference
      pref->setDirectoryNameGivenFile("ExportPlot", fileName);

      // attempt to determine the format from the tail of the filename - otherwise the filter selected.
      if (fileName.endsWith(".jpg", Qt::CaseInsensitive) || fileName.endsWith(".jpeg", Qt::CaseInsensitive)) {
	saveFormat = "JPEG";
      }
      else if (fileName.endsWith(".bmp", Qt::CaseInsensitive)) {
	saveFormat = "BMP";
      }
      else if (fileName.endsWith(".png", Qt::CaseInsensitive)) {
	saveFormat = "PNG";
      }
      else {
	// not a known extension ... append based on the filter
	if (format.startsWith("BMP")) {
	  saveFormat = "BMP";
	  fileName += ".bmp";
	}
	else if (format.startsWith("JPEG")) {
	  saveFormat = "JPEG";
	  fileName += ".jpeg";
	}
	else { // default to PNG
	  saveFormat = "PNG";
	  fileName += ".png";
	}
      }

      return true;
    }
  }

  return false;
}

CWPlot::CWPlot(const RefCountConstPtr<CPlotDataSet> &dataSet,
	       CPlotProperties &plotProperties, QWidget *parent) :
  QwtPlot(parent),
  m_dataSet(dataSet),
  m_plotProperties(plotProperties),
  m_zoomer(NULL)
{
  // Example code for font changes ... TODO
  //QwtText tmpTitle = title();
  //QFont tmpFont = tmpTitle.font();
  //tmpFont.setPointSize(tmpFont.pointSize() - 1);
  //tmpTitle.setFont(tmpFont);
  //tmpTitle.setText(m_dataSet->plotTitle());
  //setTitle(tmpTitle);

  setFocusPolicy(Qt::ClickFocus); // TODO - prevents keyPressEvent

  setTitle(m_dataSet->plotTitle());
  //setAxisTitle(QwtPlot::xBottom, m_dataSet->xAxisLabel());
  setAxisTitle(QwtPlot::yLeft, m_dataSet->yAxisLabel());

    // curves ...
  
  int n = m_dataSet->count();
  int i = 0;

  while (i < n) {

    const CXYPlotData &curveData = m_dataSet->rawData(i);

    if (curveData.size() > 0) {
      QwtPlotCurve *curve = new QwtPlotCurve();

      // the data is guaranteed to be valid for the life of this object
      curve->setRawData(curveData.xRawData(), curveData.yRawData(), curveData.size());
      
      // configure curve's pen color based on index
      curve->setPen(m_plotProperties.pen((i%4) + 1));
      
      if (curveData.curveType() == Point) {
	curve->setStyle(QwtPlotCurve::NoCurve);
	QwtSymbol sym = curve->symbol();
	sym.setStyle(QwtSymbol::Ellipse);
	curve->setSymbol(sym);
      }

      curve->attach(this);
    }

    ++i;
  }

  // grid
  QwtPlotGrid *grid = new QwtPlotGrid;
  QPen pen(Qt::DotLine);
  pen.setColor((m_plotProperties.backgroundColour().value() < 128) ? Qt::white : Qt::black);
  grid->setPen(pen);
  grid->attach(this);
    
  // possibly apply fixed scaling
  if (!m_dataSet->forceAutoScaling()) {
    
    if (m_plotProperties.scaleControl(m_dataSet->scaleType()).isFixedScale()) {
      setAxisScale(QwtPlot::yLeft,
		   m_plotProperties.scaleControl(m_dataSet->scaleType()).minimum(),
		   m_plotProperties.scaleControl(m_dataSet->scaleType()).maximum());

    }
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
    menu.addAction("Export As Image...", this, SLOT(slotExportAsImage()));
    menu.addAction("Print...", this, SLOT(slotPrint()));
    
    menu.exec(e->globalPos()); // slot will do the rest
    
    e->accept();
  }
  else
    e->ignore();
}

void CWPlot::slotOverlay()
{
  CPreferences *pref = CPreferences::instance();
  
  QString dirName = pref->directoryName("ASCII_Plot");

  QString filename = QFileDialog::getOpenFileName(this, "Overlay Plot(s)", dirName, "*.asc");
  
  if (!filename.isEmpty()) {
    pref->setDirectoryNameGivenFile("ASCII_Plot", filename);

    bool failed = false;

    FILE *fp = fopen(filename.toAscii().constData(), "r");
    if (fp != NULL) {
      char buffer[32];
      int nCurves, nPoints, i, j;

      int curveCount = m_dataSet->count(); // number of 'original' curves ...

      double *xData = NULL;
      double *yData = NULL;
      
      // skip the header
      fgets(buffer, sizeof(buffer), fp);

      if (fscanf(fp, "%d", &nCurves) == 1 && nCurves > 0) {

	i = 0;
	while (!failed && i < nCurves) {

	  if (fscanf(fp, "%d", &nPoints) == 1) {
	    xData = new double[nPoints];
	    yData = new double[nPoints];
	    
	    j = 0;
	    while (j<nPoints && fscanf(fp, "%lf %lf", (xData+j), (yData+j)) == 2)
	      ++j;
	    if (j == nPoints) {
	      QwtPlotCurve *curve = new QwtPlotCurve();
	      curve->setData(xData, yData, nPoints);
	      // configure curve's pen color based on index
	      curve->setPen(m_plotProperties.pen((curveCount % 4) + 1));
	      curve->attach(this);
	      ++curveCount;
	    }
	    else
	      failed = true;

	    delete [] xData;
	    delete [] yData;
	    xData = yData = NULL;
	  }
	  else
	    failed  = true;

	  ++i;
	}
      }
      else
	failed = true;

      fclose(fp);
    }
    else
      failed  = true;

    if (failed) {
      QString msg = "Failed (correctly) open or parse ASCII data file ";
      msg += filename;

      QMessageBox::warning(this, "Failed file read", msg);
    }
    else
      replot();

  }
  
}

void CWPlot::slotSaveAs()
{
  CPreferences *pref = CPreferences::instance();
  
  QString dirName = pref->directoryName("ASCII_Plot");

  QString filename = QFileDialog::getSaveFileName(this, "Save Plot", dirName, " Ascii file (*.asc)");
  
  if (!filename.isEmpty()) {
    if (!filename.contains('.'))
      filename += ".asc";

    pref->setDirectoryNameGivenFile("ASCII_Plot", filename);
    
    FILE *fp = fopen(filename.toAscii().constData(), "w");
    if (fp != NULL) {
      int nCurves, nPoints, i, j;

      nCurves = m_dataSet->count();
      fprintf(fp, ";Qdoas Plot v1.0\n%d\n", nCurves);

      i = 0;
      while (i < nCurves) {
	const CXYPlotData &curveData = m_dataSet->rawData(i);

	nPoints = curveData.size();
	if (nPoints > 0) {
	  fprintf(fp, "%d\n", nPoints);
	  
	  j=0;
	  while (j<nPoints) {
	    fprintf(fp, "%13g  %13g\n", *(curveData.xRawData() + j), *(curveData.yRawData() + j));
	    ++j;
	  }
	}

	++i;
      }
      fclose(fp);
    }
    else {
      QString msg = "Failed to create ASCII plot file ";
      msg += filename;

      QMessageBox::warning(this, "Failed file write", msg);
    }
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
    
    const int cPageBorder = 150;
    
    QRect tmp(cPageBorder, cPageBorder, page.width() - 2 * cPageBorder, page.height() - 2 * cPageBorder);
    
    p.drawRect(tmp);
    
    tmp.adjust(20, 20, -20, -20);
    print(&p, tmp);
  }
}

void CWPlot::slotExportAsImage()
{
  QString fileName;
  QString format;

  if (CWPlot::getImageSaveNameAndFormat(this, fileName, format)) {

    QImage img(size(), QImage::Format_RGB32); // image the same size as the plot widget.
    img.fill(0xffffffff);
    
    print(img);
    
    img.save(fileName, format.toAscii().constData());
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

void CWPlotPage::slotExportAsImageAllPlots()
{
  if (m_plots.size() == 0)
    return;
  
  QString fileName;
  QString format;

  if (CWPlot::getImageSaveNameAndFormat(this, fileName, format)) {

    // determine the required image size ...

    const int cPlotBorder = 20;

    QSize plotSize;

    int columns = (m_plots.size() < m_plotProperties.columns()) ? m_plots.size() : m_plotProperties.columns();
    int rows = m_plots.size() / columns + ((m_plots.size() % columns) ? 1 : 0);
      
    QList<CWPlot*>::iterator it = m_plots.begin();
    while (it != m_plots.end()) {
      plotSize = plotSize.expandedTo((*it)->size());
      ++it;
    }

    // the size of the final image
    QSize imgSize(plotSize.width() * columns + (columns+1) * cPlotBorder,
                  plotSize.height() * rows + (rows+1) * cPlotBorder);

    QImage img(imgSize, QImage::Format_RGB32); // image the same size as the plot widget.
    img.fill(0xffffffff);
    
    QPainter p(&img);

    int col = 0;
    int row = 0;
    it = m_plots.begin();
    while (it != m_plots.end()) {
      QRect tmp(cPlotBorder + col * (cPlotBorder + plotSize.width()),
                cPlotBorder + row * (cPlotBorder + plotSize.height()),
                plotSize.width(), plotSize.height());
	
      (*it)->print(&p, tmp);
      if (++col == columns) {
        col = 0;
        ++row;
      }
      ++it;
    }
    
    img.save(fileName, format.toAscii().constData());
  }
}

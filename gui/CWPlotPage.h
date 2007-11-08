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

#ifndef _CWPLOTPAGE_H_GUARD
#define _CWPLOTPAGE_H_GUARD

#include <QFrame>
#include <QList>
#include <QSize>

#include <qwt_plot.h>
#include <qwt_plot_zoomer.h>

#include "CPlotProperties.h"
#include "CPlotDataSet.h"
#include "CPlotPageData.h"
#include "RefCountPtr.h"

class CWPlot : public QwtPlot
{
Q_OBJECT
 public:
  CWPlot(const RefCountConstPtr<CPlotDataSet> &dataSet, const CPlotProperties &plotProperties, QWidget *parent = 0);
  virtual ~CWPlot();

 protected:
  virtual void contextMenuEvent(QContextMenuEvent *e);

 public slots:
  void slotOverlay();
  void slotSaveAs();
  void slotPrint();
  void slotToggleInteraction();

 private:
  RefCountConstPtr<CPlotDataSet> m_dataSet;
  QwtPlotZoomer *m_zoomer;
};

class CWPlotPage : public QFrame
{
Q_OBJECT
 public:
  CWPlotPage(const CPlotProperties &plotProperties, QWidget *parent = 0);
  CWPlotPage(const CPlotProperties &plotProperties,
	     const RefCountConstPtr<CPlotPageData> &page, QWidget *parent = 0);
  virtual ~CWPlotPage();

  void addPlot(const RefCountConstPtr<CPlotDataSet> &dataSet);
  void layoutPlots(const QSize &visibleSize);

 protected:
  void contextMenuEvent(QContextMenuEvent *e);

  //  virtual void resizeEvent(QResizeEvent *e);

 public slots:
  void slotPrintAllPlots();

 private:
  const CPlotProperties &m_plotProperties;
  QList<CWPlot*> m_plots;
};

#endif

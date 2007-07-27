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

#include "CPlotDataSet.h"
#include "CPlotPageData.h"
#include "RefCountPtr.h"

class CWPlot : public QwtPlot
{
 public:
  CWPlot(RefCountConstPtr<CPlotDataSet> dataSet, QWidget *parent = 0);
  virtual ~CWPlot();

 private:
  RefCountConstPtr<CPlotDataSet> m_dataSet;
};

class CWPlotPage : public QFrame
{
 public:
  CWPlotPage(int columns, QWidget *parent = 0);
  CWPlotPage(int columns, RefCountConstPtr<CPlotPageData> page, QWidget *parent = 0);
  virtual ~CWPlotPage();

  void addPlot(RefCountConstPtr<CPlotDataSet> dataSet);
  void layoutPlots(const QSize &visibleSize);

 protected:
  //  virtual void resizeEvent(QResizeEvent *e);

 private:
  int m_columns;
  QList<CWPlot*> m_plots;
};

#endif

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


#include "CPlotDataSet.h"

CXYPlotData::CXYPlotData(const double *x, const double *y, int n, enum ePlotDataType type, const char *legend) :
  m_curve(x, y, n),
  m_curveType(type),
  m_legend(legend)
{
}

CXYPlotData::~CXYPlotData()
{
}



CPlotDataSet::CPlotDataSet(const char *title, const char *xlabel, const char *ylabel) :
  m_title(title),
  m_xLabel(xlabel),
  m_yLabel(ylabel)
{
}

void CPlotDataSet::addPlotData(const double *x, const double *y, int n, enum ePlotDataType type, const char *legend)
{
  m_dataList.push_back(new CXYPlotData(x, y, n, type, legend));
}
               

CPlotDataSet::~CPlotDataSet()
{
  while (!m_dataList.empty()) {
    delete m_dataList.takeFirst();
  }
}

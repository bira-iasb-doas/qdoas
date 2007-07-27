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

CPlotDataSet::CPlotDataSet(const double *x, const double *y, int n,
                           const char *title, const char *xlabel, const char *ylabel) :
  m_lines(x, y, n),
  m_points(NULL, NULL, 0),
  m_hasPointData(false),
  m_title(title),
  m_xLabel(xlabel),
  m_yLabel(ylabel)
{
}

CPlotDataSet::CPlotDataSet(const double *x, const double *y, int n,
                           const double *px, const double *py, int pn,
                           const char *title, const char *xlabel, const char *ylabel) :
  m_lines(x, y, n),
  m_points(px, py, pn),
  m_hasPointData(true),
  m_title(title),
  m_xLabel(xlabel),
  m_yLabel(ylabel)
{
}

CPlotDataSet::~CPlotDataSet()
{
}

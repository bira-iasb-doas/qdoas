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

#ifndef _CPLOTDATASET_H_GUARD
#define _CPLOTDATASET_H_GUARD

#include <QString>
#include <qwt_data.h>

class CPlotDataSet
{
 public:
  CPlotDataSet(const double *x, const double *y, int n,
               const char *title, const char *xlabel, const char *ylabel);
  CPlotDataSet(const double *x, const double *y, int n,
               const double *px, const double *py, int pn,
               const char *title, const char *xlabel, const char *ylabel);
  ~CPlotDataSet();

  bool hasPointData(void) const;
  const QwtArrayData& lines(void) const;
  const QwtArrayData& points(void) const;
  const QString& plotTitle(void) const;
  const QString& xAxisLabel(void) const;
  const QString& yAxisLabel(void) const;

 private:
  QwtArrayData m_lines;
  QwtArrayData m_points;
  bool m_hasPointData;
  QString m_title, m_xLabel, m_yLabel;
};

inline bool CPlotDataSet::hasPointData(void) const { return m_hasPointData; }
inline const QwtArrayData& CPlotDataSet::lines(void) const { return m_lines; }
inline const QwtArrayData& CPlotDataSet::points(void) const { return m_points; }
inline const QString& CPlotDataSet::plotTitle(void) const { return m_title; }
inline const QString& CPlotDataSet::xAxisLabel(void) const { return m_xLabel; }
inline const QString& CPlotDataSet::yAxisLabel(void) const { return m_yLabel; }

struct SPlotData
{
  int page;
  const CPlotDataSet *data;

  SPlotData(int p, const CPlotDataSet *d) : page(p), data(d) {}
};

struct STitleTag
{
  int page;
  QString title;
  QString tag;

  STitleTag(int p, const QString &ti, const QString &ta) : page(p), title(ti), tag(ta) {}
};

#endif

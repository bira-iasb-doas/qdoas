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

#include "mediate_types.h"

class CXYPlotData
{
 public:
  CXYPlotData(const double *x, const double *y, int n, enum ePlotDataType type, const char *legend);
  ~CXYPlotData();

  enum ePlotDataType type(void) const;
  const QwtArrayData& curve(void) const;
  const QString& legend(void) const;

 private:
  QwtArrayData m_curve;
  enum ePlotDataType m_curveType;
  QString m_legend;
};

inline enum ePlotDataType CXYPlotData::type(void) const { return m_curveType; }
inline const QwtArrayData& CXYPlotData::curve(void) const { return m_curve; }
inline const QString& CXYPlotData::legend(void) const { return m_legend; }


class CPlotDataSet
{
 public:
  CPlotDataSet(const char *title, const char *xlabel, const char *ylabel);
  ~CPlotDataSet();

  void addPlotData(const double *x, const double *y, int n, enum ePlotDataType type, const char *legend);
               
  int count(void) const;
  const QwtArrayData& curve(int index) const;
  const QString& legend(int index) const;
  enum ePlotDataType type(int index) const;

  const QString& plotTitle(void) const;
  const QString& xAxisLabel(void) const;
  const QString& yAxisLabel(void) const;

 private:
  QList<CXYPlotData*> m_dataList;
  QString m_title, m_xLabel, m_yLabel;
};

inline int CPlotDataSet::count(void) const { return m_dataList.count(); }
inline const QwtArrayData& CPlotDataSet::curve(int index) const { return m_dataList.at(index)->curve(); }
inline const QString& CPlotDataSet::legend(int index) const { return m_dataList.at(index)->legend(); }
inline enum ePlotDataType CPlotDataSet::type(int index) const { return m_dataList.at(index)->type(); }
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

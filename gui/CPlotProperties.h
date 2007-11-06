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


#ifndef _CPLOTPROPERTIES_H_GUARD
#define _CPLOTPROPERTIES_H_GUARD

#include <vector>
#include <QPen>
#include <QColor>

#include "mediate_types.h"

class CPlotProperties
{
 public:
  CPlotProperties();
  CPlotProperties(const CPlotProperties &other);
  ~CPlotProperties();

  CPlotProperties& operator=(const CPlotProperties &rhs);

  const QPen& pen(enum ePlotDataType plotType) const;
  void setPen(enum ePlotDataType plotType, const QPen &pen);

  const QColor& backgroundColour(void) const;
  void setBackgroundColour(const QColor &c);

  int columns(void) const;
  void setColumns(int nColumns);

 private:
  QPen m_defaultPen;
  std::vector<QPen> m_pens;
  QColor m_bgColour;
  int m_columns;
};

inline const QColor& CPlotProperties::backgroundColour(void) const { return m_bgColour; }
inline void CPlotProperties::setBackgroundColour(const QColor &c) { m_bgColour = c; }
inline int CPlotProperties::columns(void) const { return m_columns; }
inline void CPlotProperties::setColumns(int nColumns) { m_columns = (nColumns < 1) ? 1 : nColumns; }

#endif

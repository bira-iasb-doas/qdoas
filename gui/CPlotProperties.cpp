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


#include "CPlotProperties.h"


CPlotProperties::CPlotProperties() :
  m_defaultPen(Qt::black),
  m_bgColour(Qt::white)
{
  // a pen for every valid plot type
  m_pens.push_back(m_defaultPen); // Spectrum
  m_pens.push_back(m_defaultPen); // Fit
  m_pens.push_back(m_defaultPen); // Shift
  m_pens.push_back(m_defaultPen); // Fwhm
  m_pens.push_back(m_defaultPen); // Points
}

CPlotProperties::CPlotProperties(const CPlotProperties &other) :
  m_defaultPen(Qt::black),
  m_pens(other.m_pens),
  m_bgColour(other.m_bgColour)
{
}

CPlotProperties& CPlotProperties::operator=(const CPlotProperties &rhs)
{
  if (&rhs != this) {
    m_pens = rhs.m_pens;
    m_bgColour = rhs.m_bgColour;
  }

  return *this;
}

CPlotProperties::~CPlotProperties()
{
}

const QPen& CPlotProperties::pen(enum ePlotDataType plotType) const
{
  if ((size_t)plotType < m_pens.size())
    return m_pens[plotType];

  return m_defaultPen;
}

void CPlotProperties::setPen(enum ePlotDataType plotType, const QPen &pen)
{
  if ((size_t)plotType < m_pens.size())
    m_pens[plotType] = pen;
}


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


#include "CPlotPageData.h"


CPlotPageData::CPlotPageData(int pageNumber) :
  m_pageNumber(pageNumber)
{
  // default tag
  m_tag.sprintf("Tag-%d", pageNumber);
}

CPlotPageData::~CPlotPageData()
{
  // the list destructor and ref-counting ensure safe handling of the plot data.
}

int CPlotPageData::pageNumber(void) const
{
  return m_pageNumber;
}

int CPlotPageData::size(void) const
{
  return m_dataSets.size();
}

const QString& CPlotPageData::title(void) const
{
  return m_title;
}

const QString& CPlotPageData::tag(void) const
{
  return m_tag;
}

RefCountConstPtr<CPlotDataSet> CPlotPageData::dataSet(int index) const
{
  if (index < 0 || index > m_dataSets.size())
    return RefCountConstPtr<CPlotDataSet>();

  return m_dataSets.at(index);
}

void CPlotPageData::setTitle(const QString &title)
{
  m_title = title;
}

void CPlotPageData::setTag(const QString &tag)
{
  m_tag = tag;
}

void CPlotPageData::addPlotDataSet(const CPlotDataSet *dataSet)
{
  // page takes ownership responsibility, which means it is safe
  // to wrap it in a reference counting pointer

  m_dataSets.push_back(RefCountConstPtr<CPlotDataSet>(dataSet));
}


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


#include <QResizeEvent>
#include <QLabel>
#include <QPixmap>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include "CWPlotRegion.h"
#include "CWPlotPage.h"
#include "CPreferences.h"

#include "debugutil.h"

CWPlotRegion::CWPlotRegion(QWidget *parent) :
  QScrollArea(parent),
  m_plotPage(NULL),
  m_activePageNumber(-1)
{
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  QLabel *splash = new QLabel;
  splash->setPixmap(QPixmap(":/images/splash.png"));
  splash->setAlignment(Qt::AlignCenter);

  setWidget(splash);

  // restore the plot properties from preferences
  QPen pen(Qt::black);
  QColor colour(Qt::white);

  CPreferences *pref = CPreferences::instance();

  m_properties.setPen(PlotDataType_Spectrum, pref->plotPen("Spectrum", pen));
  m_properties.setPen(PlotDataType_Fit, pref->plotPen("Fit", pen));
  m_properties.setPen(PlotDataType_Shift, pref->plotPen("Shift", pen));
  m_properties.setPen(PlotDataType_Fwhm, pref->plotPen("Fwhm", pen));
  m_properties.setPen(PlotDataType_Points, pref->plotPen("Points", pen));

  m_properties.setBackgroundColour(pref->plotColour("Background", colour));

  m_properties.setColumns(pref->plotLayout("Columns", 1));
}

CWPlotRegion::~CWPlotRegion()
{

}

void CWPlotRegion::removeAllPages()
{
  m_pageMap.clear();

  m_plotPage = NULL;
  //setWidget(m_plotPage); // deletes the current 'viewport widget'
}

void CWPlotRegion::addPage(const RefCountConstPtr<CPlotPageData> &page)
{
  // the page must not already exist
  std::map< int,RefCountConstPtr<CPlotPageData> >::iterator it = m_pageMap.find(page->pageNumber());
  if (it == m_pageMap.end())
    m_pageMap.insert(std::map< int,RefCountConstPtr<CPlotPageData> >::value_type(page->pageNumber(), page));
  
  // else just quietly allow page to be discarded
}

void CWPlotRegion::displayPage(int pageNumber)
{
  std::map< int,RefCountConstPtr<CPlotPageData> >::iterator it = m_pageMap.find(pageNumber);
  if (it != m_pageMap.end()) {
    m_activePageNumber = pageNumber;
    m_plotPage = new CWPlotPage(m_properties, it->second);
    setWidget(m_plotPage); // takes care of deleting the old widget
    m_plotPage->layoutPlots(m_visibleSize);
    m_plotPage->show();
  }
  else {
    m_activePageNumber = -1; // invalid page number
    m_plotPage = NULL;
    setWidget(m_plotPage);
  }
}

int CWPlotRegion::pageDisplayed(void) const
{
  return m_activePageNumber;
}

QString CWPlotRegion::pageTitle(int pageNumber) const
{
  std::map< int,RefCountConstPtr<CPlotPageData> >::const_iterator it = m_pageMap.find(pageNumber);
  if (it != m_pageMap.end())
    return (it->second)->title();

  return QString();
}

QString CWPlotRegion::pageTag(int pageNumber) const
{
  std::map< int,RefCountConstPtr<CPlotPageData> >::const_iterator it = m_pageMap.find(pageNumber);
  if (it != m_pageMap.end())
    return (it->second)->tag();

  return QString();
}

const CPlotProperties& CWPlotRegion::properties(void) const
{
  return m_properties;
}

void CWPlotRegion::setProperties(const CPlotProperties &properties)
{
  m_properties = properties;
}

void CWPlotRegion::savePreferences(void) const
{
  CPreferences *pref = CPreferences::instance();

  pref->setPlotPen("Spectrum", m_properties.pen(PlotDataType_Spectrum));
  pref->setPlotPen("Fit", m_properties.pen(PlotDataType_Fit));
  pref->setPlotPen("Shift", m_properties.pen(PlotDataType_Shift));
  pref->setPlotPen("Fwhm", m_properties.pen(PlotDataType_Fwhm));
  pref->setPlotPen("Points", m_properties.pen(PlotDataType_Points));

  pref->setPlotColour("Background", m_properties.backgroundColour());

  pref->setPlotLayout("Columns", m_properties.columns());
}

void CWPlotRegion::resizeEvent(QResizeEvent *e)
{
  m_visibleSize = e->size();

  if (m_plotPage) {
    m_plotPage->layoutPlots(m_visibleSize);
  }
  else {
    QWidget *w = widget();
    if (w)
      w->resize(m_visibleSize);
  }
}


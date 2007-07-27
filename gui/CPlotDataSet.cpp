
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

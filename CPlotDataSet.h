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

#endif

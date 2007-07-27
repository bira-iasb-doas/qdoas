#ifndef _CWPROJECTTABANALYSIS_H_GUARD
#define _CWPROJECTTABANALYSIS_H_GUARD

#include <QFrame>
#include <QComboBox>

#include "mediate_project.h"

class CWProjectTabAnalysis : public QFrame
{
 public:
  CWProjectTabAnalysis(const mediate_project_analysis_t *properties, QWidget *parent = 0);
  virtual ~CWProjectTabAnalysis();

  void apply(mediate_project_analysis_t *properties) const;

 private:
  QComboBox *m_methodCombo, *m_fitCombo, *m_unitCombo, *m_interpCombo;
};

#endif




#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

#include "CWProjectTabAnalysis.h"


CWProjectTabAnalysis::CWProjectTabAnalysis(const mediate_project_analysis_t *properties,
					   QWidget *parent) :
  QFrame(parent)
{
  int index;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  mainLayout->addStretch(1);
  
  QGridLayout *gridLayout = new QGridLayout;
  gridLayout->setSpacing(15);

  // row 0
  gridLayout->addWidget(new QLabel("Analysis Method", this), 0 , 0);
  m_methodCombo = new QComboBox(this);
  m_methodCombo->addItem("Optical density fitting",
			 QVariant(cProjAnalysisMethodModeOptDens));
  m_methodCombo->addItem("Intensity fitting (Marquardt-Levenberg+SVD)",
			 QVariant(cProjAnalysisMethodModeMarqLevSvd));
  m_methodCombo->addItem("Intensity fitting (Full Marquardt-Levenberg)",
			 QVariant(cProjAnalysisMethodModeMarqLevFull));
  gridLayout->addWidget(m_methodCombo, 0, 1);
  
  // row 1
  gridLayout->addWidget(new QLabel("Least Squares Fit", this), 1, 0);
  m_fitCombo = new QComboBox(this);
  m_fitCombo->addItem("No Weighting", QVariant(cProjAnalysisFitModeNone));
  m_fitCombo->addItem("Instrumental", QVariant(cProjAnalysisFitModeInstrumental));
  m_fitCombo->addItem("Statistical", QVariant(cProjAnalysisFitModeStatistical));
  gridLayout->addWidget(m_fitCombo, 1, 1);

  // row 2
  gridLayout->addWidget(new QLabel("Shift and Stretch Units", this), 2, 0);
  m_unitCombo = new QComboBox(this);
  m_unitCombo->addItem("Pixel", QVariant(cProjAnalysisUnitModePixel));
  m_unitCombo->addItem("nm", QVariant(cProjAnalysisUnitModeNanometer));
  gridLayout->addWidget(m_unitCombo, 2, 1);

  // row 3
  gridLayout->addWidget(new QLabel("Interpolation", this), 3, 0);
  m_interpCombo = new QComboBox(this);
  m_interpCombo->addItem("Linear", QVariant(cProjAnalysisInterpolationModeLinear));
  m_interpCombo->addItem("Spline", QVariant(cProjAnalysisInterpolationModeSpline));
  gridLayout->addWidget(m_interpCombo, 3, 1);

  mainLayout->addLayout(gridLayout);
  mainLayout->addStretch(4);

  // set initial values
  
  index = m_methodCombo->findData(QVariant(properties->methodType));
  if (index != -1)
    m_methodCombo->setCurrentIndex(index);

  index = m_fitCombo->findData(QVariant(properties->fitType));
  if (index != -1)
    m_fitCombo->setCurrentIndex(index);

  index = m_unitCombo->findData(QVariant(properties->unitType));
  if (index != -1)
    m_unitCombo->setCurrentIndex(index);

  index = m_interpCombo->findData(QVariant(properties->interpolationType));
  if (index != -1)
    m_interpCombo->setCurrentIndex(index);

}

CWProjectTabAnalysis::~CWProjectTabAnalysis()
{

}

void CWProjectTabAnalysis::apply(mediate_project_analysis_t *properties) const
{
  int index;

  // must have a current item so no need to check
  index = m_methodCombo->currentIndex();
  properties->methodType = m_methodCombo->itemData(index).toInt();

  index = m_fitCombo->currentIndex();
  properties->fitType = m_fitCombo->itemData(index).toInt();

  index = m_unitCombo->currentIndex();
  properties->unitType = m_unitCombo->itemData(index).toInt();

  index = m_interpCombo->currentIndex();
  properties->interpolationType = m_interpCombo->itemData(index).toInt();

}

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


#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

#include "CWProjectTabAnalysis.h"
#include "CValidator.h"


CWProjectTabAnalysis::CWProjectTabAnalysis(const mediate_project_analysis_t *properties,
					   QWidget *parent) :
  QFrame(parent)
{
  int index;
  QString tmpStr;

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

  // row 4
  gridLayout->addWidget(new QLabel("Interpolation security gap", this), 4, 0);
  m_interpolationSecuritySpinBox = new QSpinBox(this);
  m_interpolationSecuritySpinBox->setRange(1, 50);
  m_interpolationSecuritySpinBox->setFixedWidth(75);
  gridLayout->addWidget(m_interpolationSecuritySpinBox, 4, 1);

  // row 5
  gridLayout->addWidget(new QLabel("Convergence criterion", this), 5, 0);
  m_convergenceCriterionEdit = new QLineEdit(this);
  m_convergenceCriterionEdit->setValidator(new CDoubleExpFmtValidator(1.0e-30, 1.0, 4, m_convergenceCriterionEdit));
  m_convergenceCriterionEdit->setFixedWidth(75);
  gridLayout->addWidget(m_convergenceCriterionEdit, 5, 1);

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

  m_interpolationSecuritySpinBox->setValue(properties->interpolationSecurityGap);

  // validator controls the initial range and format
  m_convergenceCriterionEdit->validator()->fixup(tmpStr.setNum(properties->convergenceCriterion));
  m_convergenceCriterionEdit->setText(tmpStr);

}

CWProjectTabAnalysis::~CWProjectTabAnalysis()
{

}

void CWProjectTabAnalysis::apply(mediate_project_analysis_t *properties) const
{
  bool ok;
  int index;
  double tmpDouble;

  // must have a current item so no need to check
  index = m_methodCombo->currentIndex();
  properties->methodType = m_methodCombo->itemData(index).toInt();

  index = m_fitCombo->currentIndex();
  properties->fitType = m_fitCombo->itemData(index).toInt();

  index = m_unitCombo->currentIndex();
  properties->unitType = m_unitCombo->itemData(index).toInt();

  index = m_interpCombo->currentIndex();
  properties->interpolationType = m_interpCombo->itemData(index).toInt();

  properties->interpolationSecurityGap = m_interpolationSecuritySpinBox->value();

  tmpDouble = m_convergenceCriterionEdit->text().toDouble(&ok);

  // default if not ok
  properties->convergenceCriterion = ok ? tmpDouble : 1.0e-4;

}

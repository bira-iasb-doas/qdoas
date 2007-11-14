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

#include "constants.h"

CWProjectTabAnalysis::CWProjectTabAnalysis(const mediate_project_analysis_t *properties,
					   QWidget *parent) :
  QFrame(parent)
{
  int index;
  QString tmpStr;

  QGridLayout *mainLayout = new QGridLayout(this);
  mainLayout->setSpacing(15);

  
  int row = 0;
  mainLayout->setRowStretch(row, 1);
  ++row;

  // Method
  mainLayout->addWidget(new QLabel("Analysis Method", this), row , 1);
  m_methodCombo = new QComboBox(this);
  m_methodCombo->addItem("Optical density fitting",
			 QVariant(PRJCT_ANLYS_METHOD_SVD));
  m_methodCombo->addItem("Intensity fitting (Marquardt-Levenberg+SVD)",
			 QVariant(PRJCT_ANLYS_METHOD_SVDMARQUARDT));
  mainLayout->addWidget(m_methodCombo, row, 2);
  ++row;

  // Fit
  mainLayout->addWidget(new QLabel("Least Squares Fit", this), row, 1);
  m_fitCombo = new QComboBox(this);
  m_fitCombo->addItem("No Weighting", QVariant(PRJCT_ANLYS_FIT_WEIGHTING_NONE));
  m_fitCombo->addItem("Instrumental", QVariant(PRJCT_ANLYS_FIT_WEIGHTING_INSTRUMENTAL));
  //m_fitCombo->addItem("Statistical", QVariant(PRJCT_ANLYS_FIT_WEIGHTING_STATISTICAL));
  mainLayout->addWidget(m_fitCombo, row, 2);
  ++row;

  // Units - only PRJCT_ANLYS_UNITS_NANOMETERS

  // Interpolation
  mainLayout->addWidget(new QLabel("Interpolation", this), row, 1);
  m_interpCombo = new QComboBox(this);
  m_interpCombo->addItem("Linear", QVariant(PRJCT_ANLYS_INTERPOL_LINEAR));
  m_interpCombo->addItem("Spline", QVariant(PRJCT_ANLYS_INTERPOL_SPLINE));
  mainLayout->addWidget(m_interpCombo, row, 2);
  ++row;

  // gap
  mainLayout->addWidget(new QLabel("Interpolation security gap", this), row, 1);
  m_interpolationSecuritySpinBox = new QSpinBox(this);
  m_interpolationSecuritySpinBox->setRange(1, 50);
  m_interpolationSecuritySpinBox->setFixedWidth(75);
  mainLayout->addWidget(m_interpolationSecuritySpinBox, row, 2);
  ++row;

  // convergence
  mainLayout->addWidget(new QLabel("Convergence criterion", this), row, 1);
  m_convergenceCriterionEdit = new QLineEdit(this);
  m_convergenceCriterionEdit->setValidator(new CDoubleExpFmtValidator(1.0e-30, 1.0, 4, m_convergenceCriterionEdit));
  m_convergenceCriterionEdit->setFixedWidth(75);
  mainLayout->addWidget(m_convergenceCriterionEdit, row, 2);
  ++row;

  mainLayout->setRowStretch(row, 4);
  mainLayout->setColumnStretch(0, 1);
  mainLayout->setColumnStretch(3, 1);

  // set initial values
  
  index = m_methodCombo->findData(QVariant(properties->methodType));
  if (index != -1)
    m_methodCombo->setCurrentIndex(index);

  index = m_fitCombo->findData(QVariant(properties->fitType));
  if (index != -1)
    m_fitCombo->setCurrentIndex(index);

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

  properties->unitType = PRJCT_ANLYS_UNITS_NANOMETERS; // Always nanometers

  index = m_interpCombo->currentIndex();
  properties->interpolationType = m_interpCombo->itemData(index).toInt();

  properties->interpolationSecurityGap = m_interpolationSecuritySpinBox->value();

  tmpDouble = m_convergenceCriterionEdit->text().toDouble(&ok);

  // default if not ok
  properties->convergenceCriterion = ok ? tmpDouble : 1.0e-4;

}
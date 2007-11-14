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


#include <QGridLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QColorDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QGroupBox>

#include "CWPlotPropertiesEditor.h"
#include "CWPlotRegion.h"

#include "CHelpSystem.h"

#include "debugutil.h"

CWPlotPropertySample::CWPlotPropertySample(const QPen &pen, const QColor &bgColour, QWidget *parent) :
  QFrame(parent),
  m_pen(pen)
{
  setLineWidth(2);
  setFrameStyle(QFrame::Sunken | QFrame::Panel);

  // background
  QPalette p(palette());
  p.setColor(backgroundRole(), bgColour);
  setPalette(p);

  setAutoFillBackground(true);

  setMinimumSize(QSize(128,32));
}

CWPlotPropertySample::~CWPlotPropertySample()
{
}

void CWPlotPropertySample::setBackgroundColour(const QColor &c)
{
  QPalette p(palette());
  p.setColor(backgroundRole(), c);
  setPalette(p);

  update();
}

const QPen& CWPlotPropertySample::pen(void) const
{
  return m_pen;
}

void CWPlotPropertySample::paintEvent(QPaintEvent *e)
{
  QFrame::paintEvent(e);

  QPainter p(this);  
  p.setPen(m_pen);

  int y = height() / 2;
  p.drawLine(10, y, width() - 10, y);
}

void CWPlotPropertySample::mousePressEvent(QMouseEvent *e)
{
  // popup a color selection dialog to set the colour of m_pen
  QColor result = QColorDialog::getColor(m_pen.color(), this);
  
  if (result.isValid()) {
    m_pen.setColor(result);
    update();
  }
  e->accept();
}

void CWPlotPropertySample::slotSetPenWidth(int penWidth)
{
  m_pen.setWidth(penWidth);
  update();
}

CWPlotPropertiesEditor::CWPlotPropertiesEditor(CWPlotRegion *plotRegion, QWidget *parent) :
  CWEditor(parent),
  m_plotRegion(plotRegion)
{
  QSpinBox *spinBox;
  int row;

  QGridLayout *mainLayout = new QGridLayout(this);
  mainLayout->setMargin(50);

  QGroupBox *colourGroup = new QGroupBox("Colour/Line Width", this);

  QGridLayout *colourLayout = new QGridLayout(colourGroup);

  row = 0;

  const CPlotProperties &prop = m_plotRegion->properties();
  m_bgColour = prop.backgroundColour();

  // row 0 - Spectrum
  colourLayout->addWidget(new QLabel("Spectrum"), row, 0);
  m_spectrumSample = new CWPlotPropertySample(prop.pen(PlotDataType_Spectrum), m_bgColour);
  colourLayout->addWidget(m_spectrumSample, row, 1);
  spinBox = new QSpinBox;
  spinBox->setRange(0, 5);
  spinBox->setValue(prop.pen(PlotDataType_Spectrum).width());
  colourLayout->addWidget(spinBox, row, 2);
  connect(spinBox, SIGNAL(valueChanged(int)), m_spectrumSample, SLOT(slotSetPenWidth(int)));
  ++row;

  // row 1 - Fits
  colourLayout->addWidget(new QLabel("Fits"), row, 0);
  m_fitSample = new CWPlotPropertySample(prop.pen(PlotDataType_Fit), m_bgColour);
  colourLayout->addWidget(m_fitSample, row, 1);
  spinBox = new QSpinBox;
  spinBox->setRange(0, 5);
  spinBox->setValue(prop.pen(PlotDataType_Fit).width());
  colourLayout->addWidget(spinBox, row, 2);
  connect(spinBox, SIGNAL(valueChanged(int)), m_fitSample, SLOT(slotSetPenWidth(int)));
  ++row;

  // row 2 - Shift
  colourLayout->addWidget(new QLabel("Shift"), row, 0);
  m_shiftSample = new CWPlotPropertySample(prop.pen(PlotDataType_Shift), m_bgColour);
  colourLayout->addWidget(m_shiftSample, row, 1);
  spinBox = new QSpinBox;
  spinBox->setRange(0, 5);
  spinBox->setValue(prop.pen(PlotDataType_Shift).width());
  colourLayout->addWidget(spinBox, row, 2);
  connect(spinBox, SIGNAL(valueChanged(int)), m_shiftSample, SLOT(slotSetPenWidth(int)));
  ++row;

  // row 3 - FWHM
  colourLayout->addWidget(new QLabel("FWHM"), row, 0);
  m_fwhmSample = new CWPlotPropertySample(prop.pen(PlotDataType_Fwhm), m_bgColour);
  colourLayout->addWidget(m_fwhmSample, row, 1);
  spinBox = new QSpinBox;
  spinBox->setRange(0, 5);
  spinBox->setValue(prop.pen(PlotDataType_Fwhm).width());
  colourLayout->addWidget(spinBox, row, 2);
  connect(spinBox, SIGNAL(valueChanged(int)), m_fwhmSample, SLOT(slotSetPenWidth(int)));
  ++row;

  // row 4 - Points
  colourLayout->addWidget(new QLabel("Points"), row, 0);
  m_pointsSample = new CWPlotPropertySample(prop.pen(PlotDataType_Points), m_bgColour);
  colourLayout->addWidget(m_pointsSample, row, 1);
  spinBox = new QSpinBox;
  spinBox->setRange(0, 5);
  spinBox->setValue(prop.pen(PlotDataType_Points).width());
  colourLayout->addWidget(spinBox, row, 2);
  connect(spinBox, SIGNAL(valueChanged(int)), m_pointsSample, SLOT(slotSetPenWidth(int)));
  ++row;

  // row 5 - background colour
  QPushButton *bgColourBtn = new QPushButton("Background Colour");
  colourLayout->addWidget(bgColourBtn, row, 0, 1, 3);

  mainLayout->addWidget(colourGroup, 0, 1);

  // Layout group ...
  
  QGroupBox *layoutGroup = new QGroupBox("Layout", this);

  QGridLayout *layoutLayout = new QGridLayout(layoutGroup);

  row = 0;

  layoutLayout->addWidget(new QLabel("Plot Columns\n(Max. per page)"), row, 0);
  m_plotColumnsSpin = new QSpinBox;
  m_plotColumnsSpin->setRange(1, 6);
  m_plotColumnsSpin->setValue(prop.columns());
  layoutLayout->addWidget(m_plotColumnsSpin, row, 1);
  ++row;

  layoutLayout->setRowStretch(row, 1);

  mainLayout->addWidget(layoutGroup, 0, 2);

  mainLayout->setColumnStretch(0, 1);
  mainLayout->setColumnStretch(3, 1);

  // Update the caption and create a context tag 
  m_captionStr = "Edit Plot Properties";
  m_contextTag = "PlotProperties"; // only ever want one of these active at once

  // connections
  connect(bgColourBtn, SIGNAL(clicked()), this, SLOT(slotSelectBackgroundColour()));

  notifyAcceptActionOk(true);
}

CWPlotPropertiesEditor::~CWPlotPropertiesEditor()
{
}

bool CWPlotPropertiesEditor::actionOk(void)
{
  // set the properties in the plot region.
  CPlotProperties prop = m_plotRegion->properties();

  // get pens from the samples ...
  prop.setPen(PlotDataType_Spectrum, m_spectrumSample->pen());
  prop.setPen(PlotDataType_Fit, m_fitSample->pen());
  prop.setPen(PlotDataType_Shift, m_shiftSample->pen());
  prop.setPen(PlotDataType_Fwhm, m_fwhmSample->pen());
  prop.setPen(PlotDataType_Points, m_pointsSample->pen());

  prop.setBackgroundColour(m_bgColour);

  prop.setColumns(m_plotColumnsSpin->value());

  m_plotRegion->setProperties(prop);

  return true;
}

void CWPlotPropertiesEditor::actionHelp(void)
{
  CHelpSystem::showHelpTopic("plotconf", "Properties");
}

void CWPlotPropertiesEditor::slotSelectBackgroundColour()
{
  // popup a color selection dialog to set the colour of the background
  QColor result = QColorDialog::getColor(m_bgColour, this);
  
  if (result.isValid()) {
    m_bgColour = result;

    m_spectrumSample->setBackgroundColour(m_bgColour);
    m_fitSample->setBackgroundColour(m_bgColour);
    m_shiftSample->setBackgroundColour(m_bgColour);
    m_fwhmSample->setBackgroundColour(m_bgColour);
    m_pointsSample->setBackgroundColour(m_bgColour);
  }
}

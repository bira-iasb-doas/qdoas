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

#include <QFontMetrics>
#include <QIntValidator>

#include "CNavigationPanel.h"

CNavigationPanel::CNavigationPanel(QToolBar *toolBar) :
  QObject(toolBar),
  m_maxIndex(0),
  m_currentIndex(0)
{
  // Add the actions to the tool-bar
  m_firstBtn = toolBar->addAction(QIcon(":/icons/nav_first_22.png"), "first");
  m_prevBtn = toolBar->addAction(QIcon(":/icons/nav_prev_22.png"), "prev");
  m_indexEdit = new QLineEdit;

  QFontMetrics fm(m_indexEdit->font());
  m_indexEdit->setFixedWidth(fm.width("00000")); // for a 4 digit line-edit

  toolBar->addWidget(m_indexEdit);
  m_nextBtn = toolBar->addAction(QIcon(":/icons/nav_next_22.png"), "next");
  m_lastBtn = toolBar->addAction(QIcon(":/icons/nav_last_22.png"), "last");

  // initially disabled
  m_firstBtn->setEnabled(false);
  m_prevBtn->setEnabled(false);
  m_indexEdit->setEnabled(false);
  m_nextBtn->setEnabled(false);
  m_lastBtn->setEnabled(false);
  
  // connections for the actions ... TODO

  connect(m_firstBtn, SIGNAL(triggered()), this, SLOT(slotFirstClicked()));
  connect(m_prevBtn, SIGNAL(triggered()), this, SLOT(slotPreviousClicked()));
  connect(m_nextBtn, SIGNAL(triggered()), this, SLOT(slotNextClicked()));
  connect(m_lastBtn, SIGNAL(triggered()), this, SLOT(slotLastClicked()));
  connect(m_indexEdit, SIGNAL(returnPressed()), this, SLOT(slotIndexEditChanged()));
}

CNavigationPanel::~CNavigationPanel()
{  
}

void CNavigationPanel::slotSetMaxIndex(int maxIndex)
{
  m_maxIndex = maxIndex;

  // change line edit validator ...
  m_indexEdit->setValidator(new QIntValidator(1, m_maxIndex, m_indexEdit));
    
  // current index implicitly reset to 0
  m_currentIndex = 0;

  // enable/disable state of buttons

  m_firstBtn->setEnabled(false);
  m_prevBtn->setEnabled(false);
  
  m_indexEdit->setText(QString());

  if (m_maxIndex == 0) {
    m_indexEdit->setEnabled(false);
    m_nextBtn->setEnabled(false);
    m_lastBtn->setEnabled(false);
  }
  else {
    m_indexEdit->setEnabled(true);
    m_nextBtn->setEnabled(true);
    m_lastBtn->setEnabled(true);
  }
}

void CNavigationPanel::slotSetCurrentIndex(int index)
{
  m_currentIndex = index;
  
  if (!m_maxIndex)
    return;

  if (m_currentIndex == 0) {
    m_firstBtn->setEnabled(false);
    m_prevBtn->setEnabled(false);
    m_indexEdit->setText(QString());
    m_indexEdit->setEnabled(true);
    m_nextBtn->setEnabled(true);
    m_lastBtn->setEnabled(true);    
  }
  else if (m_currentIndex > m_maxIndex) {
    m_firstBtn->setEnabled(true);
    m_prevBtn->setEnabled(true);
    m_indexEdit->setText(QString());
    m_indexEdit->setEnabled(true);
    m_nextBtn->setEnabled(false);
    m_lastBtn->setEnabled(false);    
  }
  else {
    m_firstBtn->setEnabled(m_currentIndex > 1);
    m_prevBtn->setEnabled(m_currentIndex > 1);    
    m_nextBtn->setEnabled(m_currentIndex < m_maxIndex);
    m_lastBtn->setEnabled(m_currentIndex < m_maxIndex);    
    
    QString tmpStr;
    m_indexEdit->setText(tmpStr.setNum(m_currentIndex));
    m_indexEdit->setEnabled(true);
  }
}

// provides a facade for the toolbutton triggered signals
void CNavigationPanel::slotFirstClicked()
{
  emit signalFirstClicked();
}

void CNavigationPanel::slotPreviousClicked()
{
  emit signalPreviousClicked();
}

void CNavigationPanel::slotNextClicked()
{
  emit signalNextClicked();
}

void CNavigationPanel::slotLastClicked()
{
  emit signalLastClicked();
}

void CNavigationPanel::slotIndexEditChanged()
{
  // on a 'return presss'
  bool ok;
  int index = m_indexEdit->text().toInt(&ok);
  
  if (ok && index > 0 && index <= m_maxIndex) {
    if (index != m_currentIndex)
      emit signalIndexChanged(index);
  }
  else {
    QString tmpStr;
    if (m_currentIndex)
      tmpStr.setNum(m_currentIndex);
    m_indexEdit->setText(tmpStr);
  } 
}

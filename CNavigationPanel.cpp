#include "CNavigationPanel.h"

#include "icons/nav_first_16.xpm"

CNavigationPanel::CNavigationPanel(QToolBar *toolBar) :
  QObject(toolBar),
  m_maxIndex(0),
  m_currentIndex(0)
{
  // Add the actions to the tool-bar
  m_firstBtn = toolBar->addAction(QIcon(QPixmap(nav_first_16_xpm)), "first");
  m_prevBtn = toolBar->addAction(QIcon(QPixmap(nav_first_16_xpm)), "prev");
  m_indexEdit = new QLineEdit;
  m_indexEdit->setFixedWidth(70);
  toolBar->addWidget(m_indexEdit);
  m_nextBtn = toolBar->addAction(QIcon(QPixmap(nav_first_16_xpm)), "next");
  m_lastBtn = toolBar->addAction(QIcon(QPixmap(nav_first_16_xpm)), "last");

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
    m_indexEdit->setText(tmpStr.sprintf("%d", m_currentIndex));
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
      tmpStr.sprintf("%d", m_currentIndex);
    m_indexEdit->setText(tmpStr);
  } 
}

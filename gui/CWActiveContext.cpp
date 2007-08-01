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
#include <QPalette>
#include <QColor>
#include <QBrush>
#include <QList>

#include "CWEditor.h"
#include "CWActiveContext.h"
#include "CWPlotPage.h"
#include "CWPlotRegion.h"

#include "debugutil.h"

const int cBorderSize = 5;
const QRgb cEditTitleBackgroundColour  = 0xFF5986EC;
const QRgb cEditTitleTextColour        = 0xFFFFFFFF;
const QRgb cGraphTitleBackgroundColour = 0xFFF59F43;
const QRgb cGraphTitleTextColour       = 0xFF000000;


CWActiveContext::CWActiveContext(QWidget *parent) :
  QFrame(parent),
  m_activeEditor(NULL),
  m_titleRegionHeight(0),
  m_buttonRegionHeight(0),
  m_tabRegionHeight(0),
  m_centralRegionHeight(0)
{

  // Layout is managed explicitly

  // title string - change background colour
  m_title = new QLabel(this);
  m_title->setAlignment(Qt::AlignHCenter);
  m_title->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
  m_title->setLineWidth(1);
  
  QPalette palette(m_title->palette());
  palette.setColor(QPalette::Window, QColor(cGraphTitleBackgroundColour));
  palette.setColor(QPalette::WindowText, QColor(cGraphTitleTextColour));
  m_title->setPalette(palette);

  m_title->setAutoFillBackground(true);

  m_title->move(0,0);
  // title region height
  QSize tmpSize = m_title->sizeHint();
  if (tmpSize.isValid())
    m_titleRegionHeight = tmpSize.height();
  else
    m_titleRegionHeight = 15;

  // buttons
  m_helpButton = new QPushButton("Help", this);
  m_okButton = new QPushButton("Ok", this);
  m_cancelButton = new QPushButton("Cancel", this);
  m_cancelButton->setDefault(true);

  int minWidth = 4 * cBorderSize;

  tmpSize = m_helpButton->sizeHint();
  if (tmpSize.isValid()) {
    minWidth += tmpSize.width();
    if (tmpSize.height() > m_buttonRegionHeight)
      m_buttonRegionHeight = tmpSize.height();
    m_helpButton->resize(tmpSize);
  }

  tmpSize = m_okButton->sizeHint();
  if (tmpSize.isValid()) {
    minWidth += tmpSize.width();
    if (tmpSize.height() > m_buttonRegionHeight)
      m_buttonRegionHeight = tmpSize.height();
    m_okButton->resize(tmpSize);
  }

  tmpSize = m_cancelButton->sizeHint();
  if (tmpSize.isValid()) {
    minWidth += tmpSize.width();
    if (tmpSize.height() > m_buttonRegionHeight)
      m_buttonRegionHeight = tmpSize.height();
    m_cancelButton->resize(tmpSize);
  }

  // border around the buttons
  if (!m_buttonRegionHeight) {
    tmpSize = QSize(75,22);
    m_helpButton->resize(tmpSize);
    m_okButton->resize(tmpSize);
    m_cancelButton->resize(tmpSize);
    m_buttonRegionHeight = tmpSize.height();
  }
  m_buttonRegionHeight += 2 * cBorderSize;

  // tab-bar
  m_graphTab = new QTabBar(this);
  m_graphTab->setShape(QTabBar::TriangularSouth);
  m_graphTab->addTab("Qdoas"); // need one tab to get a sensible height from sizeHint.

  tmpSize = m_graphTab->sizeHint();
  if (tmpSize.isValid()) {
    if (tmpSize.width() > minWidth)
      minWidth = tmpSize.width();
    m_tabRegionHeight = tmpSize.height();
  }
  else {
    m_tabRegionHeight = 20;
  }

  m_plotRegion = new CWPlotRegion(this);

  // this might be adjusted in edit mode ...
  m_minGeneralSize = QSize(minWidth, m_titleRegionHeight + m_tabRegionHeight + 50);
  setMinimumSize(m_minGeneralSize);

  // connections
  connect(m_okButton, SIGNAL(clicked()), this, SLOT(slotOkButtonClicked()));
  connect(m_helpButton, SIGNAL(clicked()), this, SLOT(slotHelpButtonClicked()));
  connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(slotCancelButtonClicked()));
  
  connect(m_graphTab, SIGNAL(currentChanged(int)),
	  this, SLOT(slotCurrentTabChanged(int)));

  // explicitly hide the edit stuff to start with
  m_helpButton->hide();
  m_okButton->hide();
  m_cancelButton->hide();

}

CWActiveContext::~CWActiveContext()
{
  // all widgets in the stack are parented to this so they will get deleted automagically.
  m_editorStack.clear();
}

void CWActiveContext::addEditor(CWEditor *editor)
{
  if (m_activeEditor) {
    // if there is an active editor, then disconnect its signalAcceptOk & signalShortcutActionOk
    // signals, push it on the stack and make it invisible.
    disconnect(m_activeEditor, SIGNAL(signalAcceptOk(bool)), this, SLOT(slotAcceptOk(bool)));
    disconnect(m_activeEditor, SIGNAL(signalShortcutActionOk()), this, SLOT(slotOkButtonClicked()));
    m_editorStack.push_back(m_activeEditor);
    m_activeEditor->hide();
  }
  else {
    // graph widgets were visible - hide them - reposition and make buttons visible.

    moveAndResizeButtons(width(), height());

    m_plotRegion->hide();
    m_graphTab->hide();
    m_helpButton->show();
    m_okButton->show();
    m_cancelButton->show();

    // change title colour scheme
    QPalette palette(m_title->palette());
    palette.setColor(QPalette::Window, QColor(cEditTitleBackgroundColour));
    palette.setColor(QPalette::WindowText, QColor(cEditTitleTextColour));
    m_title->setPalette(palette);
  }

  // walk the list and see if an editor with the same contextTag is in the stack. If so, use it
  // instead of editor (can just delete editor)
  if (!m_editorStack.isEmpty()) {
    QList<CWEditor*>::iterator it = m_editorStack.begin();
    while (it != m_editorStack.end() && (*it)->editContextTag() != editor->editContextTag()) ++it;
    if (it != m_editorStack.end()) {
      delete editor;
      editor = *it;
      m_editorStack.erase(it);
    }
  }

  m_activeEditor = editor;

  m_activeEditor->setParent(this);
  moveAndResizeActiveEditor(width());

  connect(m_activeEditor, SIGNAL(signalAcceptOk(bool)), this, SLOT(slotAcceptOk(bool)));
  connect(m_activeEditor, SIGNAL(signalShortcutActionOk()), this, SLOT(slotOkButtonClicked()));
  m_okButton->setEnabled(m_activeEditor->isAcceptActionOk());

  m_title->setText(m_activeEditor->editCaption());

  m_activeEditor->show();
  // give it focus
  m_activeEditor->takeFocus();
}
  
QSize CWActiveContext::minimumSizeHint() const
{
  return QSize(m_minGeneralSize.width() + 30, m_minGeneralSize.height());
}

QSize CWActiveContext::sizeHint() const
{
  return QSize(m_minGeneralSize.width() + 300, m_minGeneralSize.height() + 300);
}

bool CWActiveContext::event(QEvent *e)
{
  if (e->type() == QEvent::LayoutRequest) {

    // only need to do something if the editor is active
    if (m_activeEditor)
      moveAndResizeActiveEditor(width());

    e->accept();
    return true;
  }

  return QFrame::event(e);
}

void CWActiveContext::resizeEvent(QResizeEvent *e)
{
  int wid = e->size().width();
  int hei = e->size().height();

  // resize the title to the full width - already positioned
  m_title->resize(wid, m_titleRegionHeight);

  if (m_activeEditor) {
    moveAndResizeButtons(wid, hei);
    // position the editor ...
    moveAndResizeActiveEditor(wid);
  }
  else {
    moveAndResizeGraph(wid, hei);
   }
}

void CWActiveContext::moveAndResizeButtons(int wid, int hei)
{
  int tmpW, tmpH;

  // the position and size of the central region
  m_centralRegionHeight = hei - m_titleRegionHeight - m_buttonRegionHeight;
  
  // position the widgets in the control region
  tmpH = hei - m_buttonRegionHeight + cBorderSize;
  m_helpButton->move(cBorderSize, tmpH);
  
  tmpW = wid - cBorderSize - m_cancelButton->width();
  m_cancelButton->move(tmpW, tmpH);
  tmpW -= cBorderSize + m_okButton->width();
  m_okButton->move(tmpW, tmpH);
}

void CWActiveContext::moveAndResizeGraph(int wid, int hei)
{
  // the position and size of the central region
  m_centralRegionHeight = hei - m_titleRegionHeight - m_tabRegionHeight;

  // position and resize the tab widget
  m_graphTab->move(0, hei - m_tabRegionHeight);
  m_graphTab->resize(wid, m_tabRegionHeight);
  
  // scroll area
  // m_plotPage->layoutPlots(wid - 16); TODOTODO
  m_plotRegion->move(0, m_titleRegionHeight);
  m_plotRegion->resize(wid, m_centralRegionHeight);
} 

void CWActiveContext::moveAndResizeActiveEditor(int fullWidth)
{
  // active editor guaranteed to be valid

  QSize tmpSize = m_activeEditor->minimumSizeHint();

  m_minEditSize = tmpSize;
  m_minEditSize.rheight() += m_titleRegionHeight + m_buttonRegionHeight;

  setMinimumSize(m_minEditSize.expandedTo(m_minGeneralSize));

  // will the minimum reasonable size fit in the available space ?
  if (tmpSize.isValid() && tmpSize.width() <= fullWidth && tmpSize.height() <= m_centralRegionHeight) {
    // yes - try and make it the recommended size, or the full size
    int wid = tmpSize.width();
    int hei = tmpSize.height();
    
    tmpSize = m_activeEditor->sizeHint();
    //tmpSize = m_activeEditor->maximumSize();
    if (tmpSize.isValid()) {
      wid = tmpSize.width();
      hei = tmpSize.height();
    }

    if (wid > fullWidth)
      wid = fullWidth;
    if (hei > m_centralRegionHeight)
      hei = m_centralRegionHeight;

    // check for stronger upper limits
    tmpSize = m_activeEditor->maximumSize();
    if (tmpSize.isValid()) {
      if (wid > tmpSize.width())
        wid = tmpSize.width();
      if (hei > tmpSize.height())
        hei = tmpSize.height();
    }

    m_activeEditor->move((fullWidth - wid)/ 2, m_titleRegionHeight);
    m_activeEditor->resize(wid, hei);
  }
  else {
    // wont fit ... but resize to the minimum
    m_activeEditor->resize(tmpSize);
  }
}

void CWActiveContext::discardCurrentEditor(void)
{
  // guaranteed that m_activeEditor is not null

  m_activeEditor->hide();
  // Delete, but not immediately, since the signalShortcutActionOk is sent from a method
  // of m_activeEditor.
  m_activeEditor->deleteLater();

  // is there something to replace it with?
  if (!m_editorStack.isEmpty()) {
    m_activeEditor = m_editorStack.back();
    m_editorStack.pop_back();

    moveAndResizeActiveEditor(width());

    connect(m_activeEditor, SIGNAL(signalAcceptOk(bool)), this, SLOT(slotAcceptOk(bool)));    
    connect(m_activeEditor, SIGNAL(signalShortcutActionOk()), this, SLOT(slotOkButtonClicked()));
    m_okButton->setEnabled(m_activeEditor->isAcceptActionOk());

    m_title->setText(m_activeEditor->editCaption());
    
    m_activeEditor->show();
  }
  else {
    // no more ... hide this widget ... signal - TODO
    m_activeEditor = NULL;

    // back to the normal minimum
    setMinimumSize(m_minGeneralSize);

    moveAndResizeGraph(width(), height());

    m_helpButton->hide();
    m_okButton->hide();
    m_cancelButton->hide();
    m_graphTab->show();
    m_plotRegion->show();

    // change title colour scheme
    QPalette palette(m_title->palette());
    palette.setColor(QPalette::Window, QColor(cGraphTitleBackgroundColour));
    palette.setColor(QPalette::WindowText, QColor(cGraphTitleTextColour));
    m_title->setPalette(palette);

    m_title->setText(m_graphTitleStr);
  }
}

void CWActiveContext::slotOkButtonClicked()
{
  if (m_activeEditor) {
    // only discard if the action was a success - the editor MUST provide its own
    // feedback to the user. This just prevents a silent no-op.
    if (m_activeEditor->actionOk())
      discardCurrentEditor();
  }
}

void CWActiveContext::slotCancelButtonClicked()
{
  if (m_activeEditor) {
    m_activeEditor->actionCancel();
    discardCurrentEditor();
  }
}

void CWActiveContext::slotHelpButtonClicked()
{
  if (m_activeEditor)
    m_activeEditor->actionHelp();
}

void CWActiveContext::slotAcceptOk(bool canDoOk)
{
  m_okButton->setEnabled(canDoOk);
}

void CWActiveContext::slotPlotPages(const QList< RefCountConstPtr<CPlotPageData> > &pageList)
{
  int index, pageNumber;

  // clear all tabs
  index = m_graphTab->count();
  while (index > 0)
    m_graphTab->removeTab(--index);

  // replace all pages of the plot list and create new tabs
  m_plotRegion->removeAllPages();
  QList< RefCountConstPtr<CPlotPageData> >::const_iterator it = pageList.begin();
  while (it != pageList.end()) {
    pageNumber = (*it)->pageNumber();
    m_plotRegion->addPage(*it);
    // store the page number as TabData
    index = m_graphTab->addTab((*it)->tag());
    m_graphTab->setTabData(index, QVariant(pageNumber));
    ++it;
  }

  // set to first tab
  if (m_graphTab->count()) {
    m_graphTab->show();
    if (m_graphTab->currentIndex() == 0) {
      slotCurrentTabChanged(0);
    }
    else {
      m_graphTab->setCurrentIndex(0);
    }
  }
  else
    m_graphTab->hide();
}

void CWActiveContext::slotCurrentTabChanged(int index)
{
  int pageNumber = (index == -1) ? -1 : m_graphTab->tabData(index).toInt();
  
  m_plotRegion->displayPage(pageNumber, 2); // TODO - columns

  // set the graph title
  m_graphTitleStr = m_plotRegion->pageTitle(pageNumber);
  if (!m_activeEditor)
    m_title->setText(m_graphTitleStr);
  
  emit signalActivePageChanged(pageNumber);
}


#include <iostream>

#include <QtGui>

#include <QSize>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QPalette>
#include <QApplication>

#include "CDoasTable.h"

CDoasTable::CDoasTable(const QString &label, QWidget *parent) :
  QFrame(parent),
  m_columnOffset(0)
{
  m_vsb = new QScrollBar(Qt::Vertical, this);
  m_hsb = new QScrollBar(Qt::Horizontal, this);

  m_sbThickness = 16;
  m_titleHeight = 28;
  m_labelWidth = 100;

  m_vsb->move(0, 0);

  m_centralWidth = 200;
  m_centralHeight = 100;

  setMinimumSize(QSize(m_labelWidth + m_sbThickness + m_centralWidth,
		       m_titleHeight + m_sbThickness + m_centralHeight));
  
  m_header = new CDoasTableColumnHeader(label, this, m_labelWidth, m_titleHeight);
  m_header->setColumnHorizontalPosition(m_sbThickness);

  m_hsb->setRange(0, 0);
  m_hsb->setValue(0);

  m_vsb->setRange(0, 0);
  m_vsb->setValue(0);

  connect(m_hsb, SIGNAL(valueChanged(int)), this, SLOT(slotMovedHorizontalScrollBar(int)));
  connect(m_vsb, SIGNAL(valueChanged(int)), this, SLOT(slotMovedVerticalScrollBar(int)));
}

CDoasTable::~CDoasTable()
{
}


void CDoasTable::createColumnEdit(const QString &label, int columnWidth)
{
  if (m_rowHeightList.count() == 0) {

    CDoasTableColumn *tmp = new CDoasTableColumnEdit(label, this, columnWidth, m_titleHeight);
    m_columnList.push_back(tmp);
    calcHorizontalScrollRange();
  }
}

void CDoasTable::createColumnCombo(const QString &label, int columnWidth, const QStringList &tags)
{
  if (m_rowHeightList.count() == 0) {
    CDoasTableColumn *tmp = new CDoasTableColumnCombo(tags, label, this, columnWidth, m_titleHeight);
    m_columnList.push_back(tmp);
    calcHorizontalScrollRange();
  }
}

void CDoasTable::createColumnCheck(const QString &label, int columnWidth)
{
  if (m_rowHeightList.count() == 0) {

    CDoasTableColumn *tmp = new CDoasTableColumnCheck(label, this, columnWidth, m_titleHeight);
    m_columnList.push_back(tmp);
    calcHorizontalScrollRange();
  }
}

void CDoasTable::addRow(int rowHeight, const QString &label, QList<QVariant> &cellData)
{
  // pad the cellData with null QVariants if too short ...
  while (cellData.count() < m_columnList.count()) {
    cellData.push_back(QVariant());
  }

  m_header->addRow(rowHeight, QVariant(label));

  QList<QVariant>::const_iterator cdIt = cellData.begin();
  QList<CDoasTableColumn*>::iterator it = m_columnList.begin();
  while (it != m_columnList.end()) {
    (*it)->addRow(rowHeight, *cdIt);
    ++it;
    ++cdIt;
  }

  int lastRowIndex = m_rowHeightList.count();
  int columnIndex = 0;

  m_rowHeightList.push_back(rowHeight);

  // notify cell data changed for all columns
  it = m_columnList.begin();
  while (it != m_columnList.end()) {
    QVariant tmp = (*it)->getCellData(lastRowIndex);
    cellDataChanged(lastRowIndex, columnIndex, tmp);
    ++columnIndex;
    ++it;
  }

  calcVerticalScrollRange();
}

void CDoasTable::removeRow(int rowIndex)
{
  if (rowIndex >= 0 && rowIndex < m_rowHeightList.count()) {
    
    m_header->removeRow(rowIndex);

    QList<CDoasTableColumn*>::iterator it = m_columnList.begin();
    while (it != m_columnList.end()) {
      (*it)->removeRow(rowIndex);
      ++it;
    }

    m_rowHeightList.removeAt(rowIndex);

    calcVerticalScrollRange();
  }
}

void CDoasTable::setColumnOffset(int offset)
{
  if (offset != m_columnOffset) {

    m_columnOffset = offset;

    CDoasTableColumn *tmp;
    int index = 0;
    int x = m_sbThickness + m_labelWidth;
    
    while (index < m_columnList.count()) {
      tmp = m_columnList.at(index);

      if (index < m_columnOffset) {
	tmp->hide();
      }
      else {
	if (x < width()) {
	  tmp->setColumnHorizontalPosition(x);
	  x += tmp->columnWidth();
	}
	else {
	  tmp->hide();
	}
      }
      ++index;
    }
  }
}

void CDoasTable::setColumnEnabled(int columnIndex, bool enabled)
{
  if (columnIndex >=0 && columnIndex < m_columnList.count())
    m_columnList.at(columnIndex)->setEnabled(enabled);
}

void CDoasTable::setCellEnabled(int rowIndex, int columnIndex, bool enabled)
{
  if (columnIndex >=0 && columnIndex < m_columnList.count())
    m_columnList.at(columnIndex)->setRowEnabled(rowIndex, enabled);
}

int CDoasTable::columnCount(void) const
{
  return m_columnList.count();
}

QList<QVariant> CDoasTable::getCellData(int rowIndex) const
{
  QList<QVariant> cellData;

  if (rowIndex >= 0 && rowIndex < m_rowHeightList.count()) {

    QList<CDoasTableColumn*>::const_iterator it = m_columnList.begin();
    while (it != m_columnList.end()) {
      cellData.push_back((*it)->getCellData(rowIndex));
      ++it;
    }
  }
  
  return cellData;
}

QString CDoasTable::getRowLabel(int rowIndex) const
{
  return m_header->getCellData(rowIndex).toString();
}

void CDoasTable::cellDataChanged(int row, int column, const QVariant &cellData)
{
  // do nothing ... default
  std::cout << "Changed : " << row << "," << column << " : " << cellData.toString().toStdString() << std::endl;
}

void CDoasTable::notifyCellDataChanged(int row, const CDoasTableColumn *column, const QVariant &cellData)
{
  int columnIndex = m_columnList.indexOf(const_cast<CDoasTableColumn*>(column));
  if (columnIndex != -1) {
    cellDataChanged(row, columnIndex, cellData);
  }
}

void CDoasTable::resizeEvent(QResizeEvent *e)
{
  int wid = e->size().width();
  int hei = e->size().height();

  m_centralWidth = wid - m_sbThickness - m_labelWidth;
  m_centralHeight = hei - m_titleHeight - m_sbThickness;

  m_vsb->resize(m_sbThickness, m_titleHeight + m_centralHeight);
  
  m_hsb->move(m_sbThickness, hei - m_sbThickness);
  m_hsb->resize(wid - m_sbThickness, m_sbThickness);

  // move the columns and resize them
  
  m_header->setViewportHeight(m_centralHeight);

  CDoasTableColumn *tmp;
  int index = 0;
  int x = m_sbThickness + m_labelWidth;

  while (index < m_columnList.count()) {
    tmp = m_columnList.at(index);
    tmp->setViewportHeight(m_centralHeight);

    if (index < m_columnOffset) {
      tmp->hide();
    }
    else {
      if (x < wid) {
	tmp->setColumnHorizontalPosition(x);
	x += tmp->columnWidth();
      }
      else {
	tmp->hide();
      }
    }
    ++index;
  }

  calcHorizontalScrollRange();
  calcVerticalScrollRange();
}

void CDoasTable::contextMenuEvent(QContextMenuEvent *e)
{
  if ((2 * e->x()) < width()) {
    std::cout << "adding row" << std::endl;

    QList<QVariant> initialValues;

    initialValues.push_back(QVariant("dummy"));
    initialValues.push_back(QVariant("Joe"));
    initialValues.push_back(QVariant(false));

    addRow(24, "Label", initialValues);
  }
  else if (m_rowHeightList.count()) {
    int rowIndex = e->y() % m_rowHeightList.count();

    std::cout << "removing row " << rowIndex << std::endl;

    QList<QVariant> data = getCellData(rowIndex);
    QList<QVariant>::iterator it = data.begin();
    while (it != data.end()) {
      std::cout << "   " << it->toString().toStdString();
      ++it;
    }
    std::cout << std::endl;
    
    removeRow(rowIndex);
  }
}

void CDoasTable::calcHorizontalScrollRange(void)
{
  int sum = 0;

  // get the sum of the widths of all columns
  QList<CDoasTableColumn*>::iterator it = m_columnList.begin();
  while (it != m_columnList.end()) {
    sum += (*it)->columnWidth();
    ++it;
  }

  int index = 0;

  while (index < m_columnList.count()-1 && sum > m_centralWidth) {
    sum -= m_columnList.at(index)->columnWidth();
    ++index;
  }
  m_hsb->setRange(0, index);
}

void CDoasTable::calcVerticalScrollRange(void)
{
  int sum = 0;

  // get the sum of the widths of all columns
  QList<int>::iterator it = m_rowHeightList.begin();
  while (it != m_rowHeightList.end()) {
    sum += *it;
    ++it;
  }

  int index = 0;

  while (index < m_rowHeightList.count()-1 && sum > m_centralHeight) {
    sum -= m_rowHeightList.at(index);
    ++index;
  }
  m_vsb->setRange(0, index);
}

void CDoasTable::slotMovedHorizontalScrollBar(int value)
{
  setColumnOffset(value);
}

void CDoasTable::slotMovedVerticalScrollBar(int value)
{
  m_header->setRowOffset(value);

  QList<CDoasTableColumn*>::iterator it = m_columnList.begin();
  while (it != m_columnList.end()) {
    (*it)->setRowOffset(value);
    ++it;
  }
}

//-------------------------------------

CDoasTableColumn::CDoasTableColumn(const QString &label, CDoasTable *owner, int columnWidth, int titleHeight) :
  QObject(owner),
  m_owner(owner),
  m_columnWidth(columnWidth),
  m_titleHeight(titleHeight),
  m_rowOffset(0),
  m_xPosition(0),
  m_xBorder(0),
  m_yBorder(0)
{
  m_viewport = new QFrame(owner); // parented to owner
  m_viewport->show();

  m_header = new QLabel(label, owner);
  m_header->setFrameStyle(QFrame::Panel | QFrame::Raised);
  m_header->setLineWidth(2);

  m_header->resize(m_columnWidth, m_titleHeight);
  m_header->show();
}

CDoasTableColumn::~CDoasTableColumn()
{
}

void CDoasTableColumn::setColumnHorizontalPosition(int xPosition)
{
  m_xPosition = xPosition;

  m_viewport->move(m_xPosition, m_titleHeight);
  m_viewport->show();

  m_header->move(m_xPosition, 0);
  m_header->show();
}

void CDoasTableColumn::setViewportHeight(int vpHeight)
{
  m_viewport->resize(m_columnWidth, vpHeight);

  // move child widgets ...
  QWidget *tmp;
  int y = 0;
  int index = m_rowOffset;
  while (index < m_widgetList.count()) {
    tmp = m_widgetList.at(index);
    
    if (y < vpHeight) {
      tmp->move(m_xBorder, y + m_yBorder);
      tmp->show();
      y += tmp->height() + m_yBorder + m_yBorder;
    }
    else {
      tmp->hide();
    }      
    ++index;
  }
}

void CDoasTableColumn::hide(void)
{
  m_viewport->hide();
  m_header->hide();
}

void CDoasTableColumn::setEnabled(bool enabled)
{
  m_header->setEnabled(enabled);
  m_viewport->setEnabled(enabled);
}

void CDoasTableColumn::setRowEnabled(int rowIndex, bool enabled)
{
  if (rowIndex >=0 && rowIndex < m_widgetList.count())
    m_widgetList.at(rowIndex)->setEnabled(enabled);
}

void CDoasTableColumn::layoutAndDisplay(void)
{
  // move child widgets ...
  QWidget *tmp;
  int y = 0;
  int index = 0;
  while (index < m_widgetList.count()) {
    tmp = m_widgetList.at(index);
    
    if (index < m_rowOffset) {
      tmp->hide();
    }
    else {
      if (y < m_viewport->height()) {
	tmp->move(m_xBorder, y + m_yBorder);
	tmp->show();
	y += tmp->height() + m_yBorder + m_yBorder;
      }
      else {
	tmp->hide();
      }
    }      
    ++index;
  }
}

void CDoasTableColumn::setRowOffset(int offset)
{
  if (offset != m_rowOffset) {
    m_rowOffset = offset;
    layoutAndDisplay();
  }
}

int CDoasTableColumn::columnWidth(void) const
{
  return m_columnWidth;
}

int CDoasTableColumn::rowCount(void) const
{
  return m_widgetList.count();
}

void CDoasTableColumn::addRow(int height, const QVariant &cellData)
{
  QWidget *tmp = createCellWidget(cellData);

  tmp->setParent(m_viewport);
  tmp->resize(m_columnWidth - m_xBorder - m_xBorder, height - m_yBorder - m_yBorder);

  // is it visible ??
  int y = 0;
  int index = m_rowOffset;
  while (index < m_widgetList.count()) {
    y += m_widgetList.at(index)->height() + m_yBorder + m_yBorder;
    ++index;
  }
  // y is now the vertical position for the last widget ...
  tmp->move(m_xBorder, y + m_yBorder);
  if (y < m_viewport->height())
    tmp->show();
  else
    tmp->hide();

  m_widgetList.push_back(tmp);
}

void CDoasTableColumn::removeRow(int rowIndex)
{
  if (rowIndex >= 0 && rowIndex < m_widgetList.count()) {

    QWidget *tmp = m_widgetList.at(rowIndex);
    m_widgetList.removeAt(rowIndex);

    tmp->hide();
    layoutAndDisplay();
    delete tmp;
  }
}

void CDoasTableColumn::setViewportBackgroundColour(const QColor &c)
{
  QPalette palette(m_viewport->palette());
  palette.setColor(QPalette::Window, c);
  m_viewport->setPalette(palette);
  m_viewport->setAutoFillBackground(true);
}

void CDoasTableColumn::setCellBorders(int xB, int yB)
{
  m_xBorder = xB;
  m_yBorder = yB;
}

const QWidget* CDoasTableColumn::getWidget(int rowIndex) const
{
  const QWidget *tmp = NULL;

  if (rowIndex >= 0 && rowIndex <= m_widgetList.count())
    tmp = m_widgetList.at(rowIndex);

  return tmp;
}

void CDoasTableColumn::slotCellDataChanged(const QWidget *src, const QVariant &cellData)
{
  // determine the row
  int rowIndex = m_widgetList.indexOf(const_cast<QWidget*>(src));
  if (rowIndex != -1) {
    m_owner->notifyCellDataChanged(rowIndex, this, cellData);
  }
}

//-------------------------------------

CDoasTableColumnHeader::CDoasTableColumnHeader(const QString &label, CDoasTable *owner, int columnWidth, int titleHeight) :
  CDoasTableColumn(label, owner, columnWidth, titleHeight)
{
}

CDoasTableColumnHeader::~CDoasTableColumnHeader()
{
}

QWidget* CDoasTableColumnHeader::createCellWidget(const QVariant &cellData)
{
  QLabel *tmp = new QLabel;
  tmp->setText(cellData.toString());
  tmp->setLineWidth(2);
  tmp->setFrameStyle(QFrame::Panel | QFrame::Raised);
  return tmp;
}

QVariant CDoasTableColumnHeader::getCellData(int rowIndex) const
{
  const QWidget *p = getWidget(rowIndex);
  if (p) {
    const QLabel *tmp = dynamic_cast<const QLabel*>(p);
    if (tmp)
      return QVariant(tmp->text());
  }

  return QVariant();
}

//-------------------------------------

CDoasTableColumnLineEdit::CDoasTableColumnLineEdit(QWidget *parent) :
  QLineEdit(parent)
{
}

CDoasTableColumnLineEdit::~CDoasTableColumnLineEdit()
{
}

void CDoasTableColumnLineEdit::slotTextChanged(const QString &newText)
{
  QVariant cellData(newText);

  emit signalTextChanged(this, cellData);
}

CDoasTableColumnEdit::CDoasTableColumnEdit(const QString &label, CDoasTable *owner, int columnWidth, int titleHeight) :
  CDoasTableColumn(label, owner, columnWidth, titleHeight)
{
  setViewportBackgroundColour(QColor(0xFFFFFFFF));
  setCellBorders(1,1);
}

CDoasTableColumnEdit::~CDoasTableColumnEdit()
{
}

QWidget* CDoasTableColumnEdit::createCellWidget(const QVariant &cellData)
{
  CDoasTableColumnLineEdit *tmp = new CDoasTableColumnLineEdit;

  tmp->setText(cellData.toString());

  connect(tmp, SIGNAL(textChanged(const QString&)), tmp, SLOT(slotTextChanged(const QString&)));
  connect(tmp, SIGNAL(signalTextChanged(const QWidget*,const QVariant&)),
	  this, SLOT(slotCellDataChanged(const QWidget*,const QVariant&)));

  return tmp;
}

QVariant CDoasTableColumnEdit::getCellData(int rowIndex) const
{
  const QWidget *p = getWidget(rowIndex);
  if (p) {
    const QLineEdit *tmp = dynamic_cast<const QLineEdit*>(p);
    if (tmp)
      return QVariant(tmp->text());
  }

  return QVariant();
}

//-------------------------------------

CDoasTableColumnComboBox::CDoasTableColumnComboBox(QWidget *parent) :
  QComboBox(parent)
{
}

CDoasTableColumnComboBox::~CDoasTableColumnComboBox()
{
}

void CDoasTableColumnComboBox::slotTextChanged(const QString &newText)
{
  QVariant cellData(newText);

  emit signalTextChanged(this, cellData);
}

CDoasTableColumnCombo::CDoasTableColumnCombo(const QStringList &tags, const QString &label, CDoasTable *owner,
				 int columnWidth, int titleHeight) :
  CDoasTableColumn(label, owner, columnWidth, titleHeight),
  m_tags(tags)
{
  setViewportBackgroundColour(QColor(0xFFFFFFFF));
  setCellBorders(1,1);
}

CDoasTableColumnCombo::~CDoasTableColumnCombo()
{
}

QWidget* CDoasTableColumnCombo::createCellWidget(const QVariant &cellData)
{
  CDoasTableColumnComboBox *tmp = new CDoasTableColumnComboBox;

  QStringList::iterator it = m_tags.begin();
  while (it != m_tags.end()) {
    tmp->addItem(*it);
    ++it;
  }

  tmp->setCurrentIndex(tmp->findText(cellData.toString()));

  connect(tmp, SIGNAL(currentIndexChanged(const QString&)), tmp, SLOT(slotTextChanged(const QString&)));
  connect(tmp, SIGNAL(signalTextChanged(const QWidget*,const QVariant&)),
	  this, SLOT(slotCellDataChanged(const QWidget*,const QVariant&)));

  return tmp;
}

QVariant CDoasTableColumnCombo::getCellData(int rowIndex) const
{
  const QWidget *p = getWidget(rowIndex);
  if (p) {
    const QComboBox *tmp = dynamic_cast<const QComboBox*>(p);
    if (tmp)
      return QVariant(tmp->currentText());
  }

  return QVariant();
}

//-------------------------------------

CDoasTableColumnCheckBox::CDoasTableColumnCheckBox(QWidget *parent) :
  QCheckBox(parent)
{
}

CDoasTableColumnCheckBox::~CDoasTableColumnCheckBox()
{
}

void CDoasTableColumnCheckBox::slotStateChanged(int state)
{
  QVariant cellData(state == Qt::Checked);

  emit signalStateChanged(this, cellData);
}

CDoasTableColumnCheck::CDoasTableColumnCheck(const QString &label, CDoasTable *owner, int columnWidth, int titleHeight) :
  CDoasTableColumn(label, owner, columnWidth, titleHeight)
{
  setViewportBackgroundColour(QColor(0xFFFFFFFF));
  setCellBorders(columnWidth / 3, 0);
}

CDoasTableColumnCheck::~CDoasTableColumnCheck()
{
}

QWidget* CDoasTableColumnCheck::createCellWidget(const QVariant &cellData)
{
  CDoasTableColumnCheckBox *tmp = new CDoasTableColumnCheckBox;
  tmp->setCheckState(cellData.toBool() ? Qt::Checked : Qt::Unchecked);

  connect(tmp, SIGNAL(stateChanged(int)), tmp, SLOT(slotStateChanged(int)));
  connect(tmp, SIGNAL(signalStateChanged(const QWidget*,const QVariant&)),
	  this, SLOT(slotCellDataChanged(const QWidget*,const QVariant&)));

  return tmp;
}

QVariant CDoasTableColumnCheck::getCellData(int rowIndex) const
{
  const QWidget *p = getWidget(rowIndex);
  if (p) {
    const QCheckBox *tmp = dynamic_cast<const QCheckBox*>(p);
    if (tmp)
      return QVariant(tmp->checkState() == Qt::Checked);
  }

  return QVariant();
}

#ifdef TEST

//----------//------------//-------------

Dummy::Dummy(const QString &label, QWidget *parent) :
  CDoasTable(label, parent)
{
  QStringList tags;
  tags << "Fred" << "Joe" << "Paul" << "Bob";

  createColumnEdit("Edit", 160);
  createColumnCombo("Combo", 90, tags);
  createColumnCheck("Check", 100);
}

Dummy::~Dummy()
{
}

void Dummy::cellDataChanged(int row, int column, const QVariant &cellData)
{
  if (column == 2) {
    setCellEnabled(row, 1, cellData.toBool());
  }
}

//-------------------------------------

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  
  Dummy main("Molecules");

  main.setWindowTitle(QObject::tr("CDoasTable"));
  main.show();
  
  return app.exec();
}

#endif

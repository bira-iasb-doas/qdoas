#ifndef _CDOASTABLE_GUARD
#define _CDOASTABLE_GUARD

#include <QLabel>
#include <QFrame>
#include <QScrollBar>
#include <QList>
#include <QColor>
#include <QStringList>
#include <QVariant>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>

#include <QResizeEvent>

class CDoasTableColumn;

class CDoasTable : public QFrame
{
Q_OBJECT
 public:
  CDoasTable(const QString &label, QWidget *parent = 0);
  virtual ~CDoasTable();
  
  void createColumnEdit(const QString &label, int columnWidth);
  void createColumnCombo(const QString &label, int columnWidth, const QStringList &tags);
  void createColumnCheck(const QString &label, int columnWidth);

  void addRow(int height, const QString &label, QList<QVariant> &cellData);
  void removeRow(int rowIndex);
  int columnCount(void) const;
  void setColumnOffset(int offset);

  void setColumnEnabled(int columnIndex, bool enabled);
  void setCellEnabled(int rowIndex, int columnIndex, bool enabled);

  QString getRowLabel(int rowIndex) const;
  QList<QVariant> getCellData(int rowIndex) const;

  virtual void cellDataChanged(int row, int column, const QVariant &cellData);

  void notifyCellDataChanged(int row, const CDoasTableColumn *column, const QVariant &cellData);

 protected:
  virtual void resizeEvent(QResizeEvent *e);
  virtual void contextMenuEvent(QContextMenuEvent *e);
  //  virtual void keyPressEvent(QKeyEvent *e);

  void calcHorizontalScrollRange(void);
  void calcVerticalScrollRange(void);

 public slots:
  void slotMovedHorizontalScrollBar(int value);
  void slotMovedVerticalScrollBar(int value);

 private:
  QScrollBar *m_hsb, *m_vsb;
  
  int m_titleHeight, m_sbThickness, m_centralWidth, m_centralHeight, m_labelWidth;

  CDoasTableColumn *m_header;
  QList<CDoasTableColumn*> m_columnList;
  QList<int> m_rowHeightList;
  int m_columnOffset;
};

class CDoasTableColumn : public QObject
{
Q_OBJECT
 public:
  CDoasTableColumn(const QString &label, CDoasTable *owner, int columnWidth, int titleHeight);
  virtual ~CDoasTableColumn();

  void setColumnHorizontalPosition(int xPosition);

  void setViewportHeight(int vpHeight);
  void hide(void);
  void setEnabled(bool enabled);
  void setRowEnabled(int rowIndex, bool enabled);

  void setRowOffset(int offset);

  int columnWidth(void) const;
  int rowCount(void) const;
  void addRow(int height, const QVariant &cellData);
  void removeRow(int rowIndex);

  virtual QVariant getCellData(int rowIndex) const = 0;

 protected:
  virtual QWidget* createCellWidget(const QVariant &cellData) = 0;
  
  void setViewportBackgroundColour(const QColor &c);
  void setCellBorders(int xB, int yB);

  const QWidget* getWidget(int rowIndex) const;

 private:
  void layoutAndDisplay(void);

 public slots:
   void slotCellDataChanged(const QWidget *src, const QVariant &cellData);

 private:
  CDoasTable *m_owner;
  int m_columnWidth;
  int m_titleHeight;
  int m_rowOffset;
  int m_xPosition;
  int m_xBorder, m_yBorder;

  QList<QWidget*> m_widgetList;
  QFrame *m_viewport; // visible region for the child widgets - parented to the viewport
  QLabel *m_header;
};

class CDoasTableColumnHeader : public CDoasTableColumn
{
 public:
  CDoasTableColumnHeader(const QString &label, CDoasTable *owner, int width, int titleHeight);
  virtual ~CDoasTableColumnHeader();

  virtual QVariant getCellData(int rowIndex) const;

 protected:
  virtual QWidget* createCellWidget(const QVariant &cellData);  
};

class CDoasTableColumnLineEdit : public QLineEdit
{
Q_OBJECT
 public:
  CDoasTableColumnLineEdit(QWidget *parent = 0);
  virtual ~CDoasTableColumnLineEdit();

 public slots:
  void slotTextChanged(const QString &newText);

 signals:
 void signalTextChanged(const QWidget *src, const QVariant &cellData);
};

class CDoasTableColumnEdit : public CDoasTableColumn
{
 public:
  CDoasTableColumnEdit(const QString &label, CDoasTable *owner, int width, int titleHeight);
  virtual ~CDoasTableColumnEdit();

  virtual QVariant getCellData(int rowIndex) const;

 protected:
  virtual QWidget* createCellWidget(const QVariant &cellData);
};

class CDoasTableColumnComboBox : public QComboBox
{
Q_OBJECT
 public:
  CDoasTableColumnComboBox(QWidget *parent = 0);
  virtual ~CDoasTableColumnComboBox();

 public slots:
  void slotTextChanged(const QString &newText);

 signals:
  void signalTextChanged(const QWidget *src, const QVariant &cellData);
};

class CDoasTableColumnCombo : public CDoasTableColumn
{
 public:
  CDoasTableColumnCombo(const QStringList &tags, const QString &label, CDoasTable *owner, int width, int titleHeight);
  virtual ~CDoasTableColumnCombo();

  virtual QVariant getCellData(int rowIndex) const;

 protected:
  virtual QWidget* createCellWidget(const QVariant &cellData);

private:
  QStringList m_tags;
};

class CDoasTableColumnCheckBox : public QCheckBox
{
Q_OBJECT
 public:
  CDoasTableColumnCheckBox(QWidget *parent = 0);
  virtual ~CDoasTableColumnCheckBox();

 public slots:
  void slotStateChanged(int);

 signals:
  void signalStateChanged(const QWidget *src, const QVariant &cellData);
};

class CDoasTableColumnCheck : public CDoasTableColumn
{
 public:
  CDoasTableColumnCheck(const QString &label, CDoasTable *owner, int width, int titleHeight);
  virtual ~CDoasTableColumnCheck();

  virtual QVariant getCellData(int rowIndex) const;

 protected:
  virtual QWidget* createCellWidget(const QVariant &cellData);
};

class Dummy : public CDoasTable
{
 public:
  Dummy(const QString &label, QWidget *parent = 0);
  virtual ~Dummy();

  virtual void cellDataChanged(int row, int column, const QVariant &cellData);  
};

#endif


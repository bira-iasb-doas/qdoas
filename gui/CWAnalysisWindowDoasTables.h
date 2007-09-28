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


#ifndef _CWANALYSISWINDOWDOASTABLE_GUARD_H
#define _CWANALYSISWINDOWDOASTABLE_GUARD_H

#include <map>

#include <QComboBox>
#include <QListWidget>
#include <QDialog>

#include "CDoasTable.h"

// variable rows

//----------------------------------------------------------------------
// specialised columns for molecules ...

class CWMoleculesDiffOrthoCombo : public CDoasTableColumnComboBox
{
Q_OBJECT
 public:
  CWMoleculesDiffOrthoCombo(const QString &excludedSymbol, const QStringList &symbols, QWidget *parent = 0);
  virtual ~CWMoleculesDiffOrthoCombo();

  void initialSelection(const QString &text);

 public slots:
  void slotSymbolListChanged(const QStringList &symbols);

 private:
  QString m_excludedSymbol, m_pendingInitial;
};

class CMoleculeDoasTableColumnDiffOrtho : public CDoasTableColumn
{
 public:
  CMoleculeDoasTableColumnDiffOrtho(const QString &label, CDoasTable *owner, int width);
  virtual ~CMoleculeDoasTableColumnDiffOrtho();

  virtual QVariant getCellData(int rowIndex) const;

 protected:
  virtual QWidget* createCellWidget(const QVariant &cellData);
};

class CWMoleculesDoasTable : public CDoasTable
{
Q_OBJECT
 public:
  CWMoleculesDoasTable(const QString &label, int columnWidth, int headerHeight = 24, QWidget *parent = 0);
  virtual ~CWMoleculesDoasTable();

  virtual void addRow(int height, const QString &label, QList<QVariant> &cellData);
  virtual void removeRow(int rowIndex);
  virtual void cellDataChanged(int row, int column, const QVariant &cellData);

  const QStringList& symbolList(void) const;

 protected:
  virtual void contextMenuEvent(QContextMenuEvent *e);

 private:
  bool isRowLocked(int rowIndex) const;

 public slots:
  void slotLockSymbol(const QString &symbol, const QObject *holder);
  void slotUnlockSymbol(const QString &symbol, const QObject *holder);
  
  void slotInsertRow();
  void slotRemoveRow();

 signals:
  void signalSymbolListChanged(const QStringList &symbols);

 private:
  typedef std::multimap<QString,const QObject*> symlockmap_t;

  QStringList m_symbols;
  QList<int> m_rowLocks; // internal locks ...
  symlockmap_t m_symbolLocks; // external locks ...
  int m_selectedRow;
};

//----------------------------------------------------------------------
// fixed size (3 row x 3 column) table ...

class CWLinearParametersDoasTable : public CDoasTable
{
 public:
  CWLinearParametersDoasTable(const QString &label, int columnWidth, int headerHeight = 24, QWidget *parent = 0);
  virtual ~CWLinearParametersDoasTable();

  // virtual void cellDataChanged(int row, int column, const QVariant &cellData); // no cell-coupling required
};


//----------------------------------------------------------------------
// fixed size (8 row x 5 column) table ...

class CWNonlinearParametersDoasTable : public CDoasTable
{
 public:
  CWNonlinearParametersDoasTable(const QString &label, int columnWidth, int headerHeight = 24, QWidget *parent = 0);
  virtual ~CWNonlinearParametersDoasTable();

  // virtual void cellDataChanged(int row, int column, const QVariant &cellData); // no cell-coupling required
};

//----------------------------------------------------------------------
// Shift And Stretch

class CWShiftAndStretchDialog : public QDialog
{
 public:
  CWShiftAndStretchDialog(const QStringList &availableSymbols, const QStringList &selectedSymbols, QWidget *parent = 0);
  virtual ~CWShiftAndStretchDialog();

  QStringList selectedSymbols(void) const;

 private:
  QListWidget *m_symbolView;
};

class CWShiftAndStretchDoasTable : public CDoasTable
{
Q_OBJECT
 public:
  CWShiftAndStretchDoasTable(const QString &label, int columnWidth, int headerHeight = 24, QWidget *parent = 0);
  virtual ~CWShiftAndStretchDoasTable();

  virtual void addRow(int height, const QString &label, QList<QVariant> &cellData);
  virtual void removeRow(int rowIndex);
  virtual void cellDataChanged(int row, int column, const QVariant &cellData); // no cell-coupling required

 protected:
  virtual void contextMenuEvent(QContextMenuEvent *e);

 public slots:
  void slotSymbolListChanged(const QStringList &symbols);
  void slotInsertRow();
  void slotRemoveRow();
  void slotModifyRow();

 signals:
  void signalLockSymbol(const QString &symbol, const QObject *holder);
  void signalUnlockSymbol(const QString &symbol, const QObject *holder);

 private:
  QStringList m_specialSymbols, m_freeSymbols;
  QList<QStringList> m_selectedSymbolList;
  int m_selectedRow;
};

#endif

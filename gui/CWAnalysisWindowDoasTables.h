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

#include "mediate_analysis_window.h"
#include "CDoasTable.h"

// variable rows

//----------------------------------------------------------------------
// specialised columns for molecules (Cross Sections) ...

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

  void populate(const cross_section_list_t *d);
  void apply(cross_section_list_t *d) const;

  void addRow(int height, const QString &label, QList<QVariant> &cellData,
	      const QString &csFilename, const QString &amfFilename);

  virtual void addRow(int height, const QString &label, QList<QVariant> &cellData);
  virtual void removeRow(int rowIndex);
  virtual void cellDataChanged(int row, int column, const QVariant &cellData);

  const QStringList& symbolList(void) const;

 protected:
  virtual void contextMenuEvent(QContextMenuEvent *e);

 private:
  bool isRowLocked(int rowIndex) const;

  static QString mapCrossTypeToComboString(int type);
  static int mapComboStringToCrossType(const QString &str);
  static QString mapAmfTypeToComboString(int type);
  static int mapComboStringToAmfType(const QString &str);

 public slots:
  void slotLockSymbol(const QString &symbol, const QObject *holder);
  void slotUnlockSymbol(const QString &symbol, const QObject *holder);
  
  void slotInsertRow();
  void slotRemoveRow();

  void slotFitColumnCheckable(int state);

 signals:
  void signalSymbolListChanged(const QStringList &symbols);

 private:
  typedef std::multimap<QString,const QObject*> symlockmap_t;

  QStringList m_symbols, m_csFilename, m_amfFilename;
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

  void populate(const struct anlyswin_linear *data);
  void apply(struct anlyswin_linear *data) const;

 private:
  static QString mapOrderToComboString(int order);
  static int mapComboStringToOrder(const QString &str);
};


//----------------------------------------------------------------------
// fixed size (8 row x 5 column) table ...

class CWNonLinearParametersDoasTable : public CDoasTable
{
 public:
  CWNonLinearParametersDoasTable(const QString &label, int columnWidth, int headerHeight = 24, QWidget *parent = 0);
  virtual ~CWNonLinearParametersDoasTable();

  void populate(const struct anlyswin_nonlinear *data);
  void apply(struct anlyswin_nonlinear *data) const;

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

  void populate(const shift_stretch_list_t *d);
  void apply(shift_stretch_list_t *d) const;

  virtual void addRow(int height, const QString &label, QList<QVariant> &cellData);
  virtual void removeRow(int rowIndex);
  virtual void cellDataChanged(int row, int column, const QVariant &cellData); // no cell-coupling required

 protected:
  virtual void contextMenuEvent(QContextMenuEvent *e);

 private:
  static QString mapOrderToComboString(int order);
  static int mapComboStringToOrder(const QString &str);

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

//----------------------------------------------------------------------
// Gap

class CWGapDoasTable : public CDoasTable
{
Q_OBJECT
 public:
  CWGapDoasTable(const QString &label, int columnWidth, int headerHeight = 24, QWidget *parent = 0);
  virtual ~CWGapDoasTable();

  void populate(const gap_list_t *d);
  void apply(gap_list_t *d) const;

  virtual void cellDataChanged(int row, int column, const QVariant &cellData);

 protected:
  virtual void contextMenuEvent(QContextMenuEvent *e);

 public slots:
  void slotInsertRow();
  void slotRemoveRow();

 private:
  int m_selectedRow;
};

//----------------------------------------------------------------------
// Output

class CWOutputDoasTable : public CDoasTable
{
Q_OBJECT
 public:
  CWOutputDoasTable(const QString &label, int columnWidth, int headerHeight = 24, QWidget *parent = 0);
  virtual ~CWOutputDoasTable();

  void populate(const output_list_t *d);
  void apply(output_list_t *d) const;

  virtual void cellDataChanged(int row, int column, const QVariant &cellData);

 private:
  // prevent manual add/remove
  virtual void addRow(int height, const QString &label, QList<QVariant> &cellData);
  virtual void removeRow(int rowIndex);
  
 public slots:
  void slotSymbolListChanged(const QStringList &symbols);

 private:
  QStringList m_symbols;
};

//----------------------------------------------------------------------
// SFP

class CWSfpParametersDoasTable : public CDoasTable
{
 public:
  CWSfpParametersDoasTable(const QString &label, int columnWidth, int headerHeight = 24, QWidget *parent = 0);
  virtual ~CWSfpParametersDoasTable();

  // virtual void cellDataChanged(int row, int column, const QVariant &cellData); // no cell-coupling required

  void populate(const struct calibration_sfp *data);
  void apply(struct calibration_sfp *data) const;

};

#endif

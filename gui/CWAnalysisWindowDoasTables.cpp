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

#include <QMenu>
#include <QContextMenuEvent>
#include <QDialogButtonBox>
#include <QVBoxLayout>

#include "CWAnalysisWindowDoasTables.h"

#include "debugutil.h"

//------------------------------------------------------------


CWMoleculesDiffOrthoCombo::CWMoleculesDiffOrthoCombo(const QString &excludedSymbol, const QStringList &symbols, QWidget *parent) :
  CDoasTableColumnComboBox(parent),
  m_excludedSymbol(excludedSymbol)
{
  // populate with the standard items
  addItem("None");
  addItem("Differential XS");

  QStringList::const_iterator it = symbols.begin();
  while (it != symbols.end()) {
    if (*it != m_excludedSymbol)
      addItem(*it);
    ++it;
  }
}
 
CWMoleculesDiffOrthoCombo::~CWMoleculesDiffOrthoCombo()
{
}

void CWMoleculesDiffOrthoCombo::slotSymbolListChanged(const QStringList &symbols)
{
  // if an initial value is set, then try and select it ...
  QString text = m_pendingInitial.isNull() ? currentText() : m_pendingInitial;

  // remove all but the first two items
  int index = count();
  while (index > 2)
    removeItem(--index);

  QStringList::const_iterator it = symbols.begin();
  while (it != symbols.end()) {
    if (*it != m_excludedSymbol)
      addItem(*it);
    ++it;
  }

  // try and reselect ...
  index = findText(text);
  if (index != -1) {
    setCurrentIndex(index);
    if (text == m_pendingInitial)
      m_pendingInitial = QString(); // has been set ... 
  }
}

void CWMoleculesDiffOrthoCombo::initialSelection(const QString &text)
{
  // sets the initial value, which might not yet be in the list ...

  int index = findText(text);
  if (index != -1) {
    setCurrentIndex(index); // exists .. just set it
  }
  else {
    m_pendingInitial = text; // set it and try again later (in slotSymbolListChanged)
  }
}

CMoleculeDoasTableColumnDiffOrtho::CMoleculeDoasTableColumnDiffOrtho(const QString &label, CDoasTable *owner, int columnWidth) :
  CDoasTableColumn(label, owner, columnWidth)
{
  setViewportBackgroundColour(QColor(0xFFFFFFFF));
  setCellBorders(1,1);  
}

CMoleculeDoasTableColumnDiffOrtho::~CMoleculeDoasTableColumnDiffOrtho()
{
}

QVariant CMoleculeDoasTableColumnDiffOrtho::getCellData(int rowIndex) const
{
  const QWidget *p = getWidget(rowIndex);
  if (p) {
    const QComboBox *tmp = dynamic_cast<const QComboBox*>(p);
    if (tmp)
      return QVariant(tmp->currentText());
  }

  return QVariant();  
}

QWidget* CMoleculeDoasTableColumnDiffOrtho::createCellWidget(const QVariant &cellData)
{
  // this is called when adding a new row

  CWMoleculesDoasTable *p = dynamic_cast<CWMoleculesDoasTable*>(owner());
  assert (p != NULL);

  const QStringList &symbolList = p->symbolList();
  QString excludedSymbol = symbolList.value(rowCount()); // the last entry
  
  CWMoleculesDiffOrthoCombo *tmp = new CWMoleculesDiffOrthoCombo(excludedSymbol, symbolList);

  tmp->initialSelection(cellData.toString());

  connect(tmp, SIGNAL(currentIndexChanged(const QString&)), tmp, SLOT(slotTextChanged(const QString&)));
  connect(tmp, SIGNAL(signalTextChanged(const QWidget*,const QVariant&)),
	  this, SLOT(slotCellDataChanged(const QWidget*,const QVariant&)));
  connect(p, SIGNAL(signalSymbolListChanged(const QStringList &)),
	  tmp, SLOT(slotSymbolListChanged(const QStringList &)));

  return tmp;
}

CWMoleculesDoasTable::CWMoleculesDoasTable(const QString &label, int columnWidth,
					   int headerHeight, QWidget *parent) :
  CDoasTable(label, columnWidth, headerHeight, parent),
  m_selectedRow(-1)
{
  CMoleculeDoasTableColumnDiffOrtho *tmp = new CMoleculeDoasTableColumnDiffOrtho("Diff/Ortho", this, columnWidth);
  addColumn(tmp);                            // columnIndex = 0

  QStringList comboItems;
  comboItems << "None" << "Interpolate" << "Convolve Std" << "Convolve IO" << "Convolve Ring";
  createColumnCombo("Interp/Conv", 120, comboItems);

  comboItems.clear();
  comboItems << "None" << "SZA only" << "Climatology" << "Wavelength 1" << "Wavelength 2" << "Wavelength 3";
  createColumnCombo("AMF", 120, comboItems); // columnIndex = 2

  createColumnCheck("Fit disp.", 60);
  createColumnCheck("Filter", 60);           // columnIndex = 4
  createColumnCheck("CC fit", 60);
  createColumnEdit("CC Init", 80);           // columnIndex = 6
  createColumnEdit("CC Delta", 80);
  createColumnCheck("CC IO", 60);            // columnIndex = 8
}
  
CWMoleculesDoasTable::~CWMoleculesDoasTable()
{
}

void CWMoleculesDoasTable::addRow(int height, const QString &label, QList<QVariant> &cellData)
{
  // update the symbol list, rowlocks list and signal ...
  m_symbols << label;
  m_rowLocks << -1; // this new row does not lock any other row

  emit signalSymbolListChanged(m_symbols);

  // really create the new row ...
  CDoasTable::addRow(height, label, cellData);
}

void CWMoleculesDoasTable::removeRow(int rowIndex)
{
  // removal is only permitted if the row is not locked by another row
  if (rowIndex >= 0 && rowIndex < rowCount() && !isRowLocked(rowIndex)) {

    m_rowLocks.removeAt(rowIndex);
    m_symbols.removeAt(rowIndex);

    // really remove the row ...
    CDoasTable::removeRow(rowIndex);  

    // notify the remaining diff/ortho combos that the symbol list has changed
    emit signalSymbolListChanged(m_symbols);
  }
}

bool CWMoleculesDoasTable::isRowLocked(int rowIndex) const
{
  // assumed rowIndex is within the valid range ...
 
  // a row is locked if ...
  //    a) There is an internal lock, which means another row references the symbol for rowIndex.
  //    b) There is an external lock, which means that an external entity is holding a lock on the symbol.

  // internal lock ?
  if (m_rowLocks.indexOf(rowIndex) != -1)
    return true;

  // at least one external lock on the symbol ?
  return (m_symbolLocks.find(m_symbols.at(rowIndex)) != m_symbolLocks.end());
}

void CWMoleculesDoasTable::slotLockSymbol(const QString &symbol, const QObject *holder)
{
  std::pair<symlockmap_t::const_iterator,symlockmap_t::const_iterator> pair =
    m_symbolLocks.equal_range(symbol);

  symlockmap_t::const_iterator it = pair.first;
  while (it != pair.second) {
    if (it->second == holder) break; // exists already
    ++it;
  }
  if (it == pair.second) {
    // does not exist already ...
    m_symbolLocks.insert(symlockmap_t::value_type(symbol, holder));
  }
}

void CWMoleculesDoasTable::slotUnlockSymbol(const QString &symbol, const QObject *holder)
{
  std::pair<symlockmap_t::iterator,symlockmap_t::iterator> pair =
    m_symbolLocks.equal_range(symbol);

  symlockmap_t::iterator it = pair.first;
  while (it != pair.second) {
    if (it->second == holder) {      
      m_symbolLocks.erase(it);
      return; // done
    }
    ++it;
  }
}

const QStringList& CWMoleculesDoasTable::symbolList(void) const
{
  return m_symbols;
}

void CWMoleculesDoasTable::cellDataChanged(int row, int column, const QVariant &cellData)
{
  // TRACE("Changed : " << row << "," << column << " : " << cellData.toString().toStdString());
  
  if (column == 0) {
    // is the selection for row the symbol of another row - if so that symbol is locked.
    m_rowLocks.replace(row, m_symbols.indexOf(cellData.toString())); // sets/releases/clears internal locks
  }
  else if (column == 1) {
    setCellEnabled(row, 8, (cellData.toString() == "Convolve IO"));
  }
}

void CWMoleculesDoasTable::contextMenuEvent(QContextMenuEvent *e)
{
  // create a popup menu
  QMenu menu;
  
  menu.addAction("Insert", this, SLOT(slotInsertRow()));
  
  m_selectedRow = rowIndexAtPosition(e->y());
  
  QAction *removeAction = menu.addAction("Remove", this, SLOT(slotRemoveRow()));
  removeAction->setEnabled(m_selectedRow != -1 && !isRowLocked(m_selectedRow));

  menu.exec(e->globalPos()); // a slot will do the rest
}
 
void CWMoleculesDoasTable::slotInsertRow()
{
  static int junk = 0;

  QList<QVariant> initialValues;
  
  QString tmp;
  tmp.sprintf("Label %d", ++junk); // TODO
  
  addRow(24, tmp, initialValues);
}

void CWMoleculesDoasTable::slotRemoveRow()
{
  if (m_selectedRow >= 0 && m_selectedRow < rowCount()) {

    // TODO
    /*
    QList<QVariant> data = getCellData(m_selectedRow);
    QList<QVariant>::iterator it = data.begin();
    while (it != data.end()) {
      std::cout << "   " << it->toString().toStdString();
      ++it;
    }
    std::cout << std::endl;
    */

    removeRow(m_selectedRow);
    m_selectedRow = -1;
  }
}

//------------------------------------------------------------

CWLinearParametersDoasTable::CWLinearParametersDoasTable(const QString &label, int columnWidth,
							 int headerHeight, QWidget *parent) :
  CDoasTable(label, columnWidth, headerHeight, parent)
{
  // construct and populate the table
  QStringList comboItems;
  comboItems << "None" << "Order 0" << "Order 1" << "Order 2" << "Order 3" << "Order 4" << "Order 5";

  // columns
  createColumnCombo("Polynomial order", 120, comboItems);
  createColumnCombo("OrthoBase order", 120, comboItems);
  createColumnCheck("Fit store", 60);
  createColumnCheck("Err store", 60);
  
  // predefined rows
  QList<QVariant> defaults;

  addRow(24, "Polynomial(x)", defaults);
  addRow(24, "Polynomial(1/x)", defaults);
  addRow(24, "Offset", defaults);
}

CWLinearParametersDoasTable::~CWLinearParametersDoasTable()
{
}

//------------------------------------------------------------

CWNonlinearParametersDoasTable::CWNonlinearParametersDoasTable(const QString &label, int columnWidth,
							       int headerHeight, QWidget *parent) :
  CDoasTable(label, columnWidth, headerHeight, parent)
{
  // construct and populate the table

  // columns
  createColumnCheck("Fit", 60);
  createColumnEdit("Val. Init", 80);
  createColumnEdit("Val. Delta", 80);
  createColumnCheck("Fit store", 60);
  createColumnCheck("Err store", 60);
  
  // predefined rows
  QList<QVariant> defaults;
  defaults << false << 0.0 << 0.001 << false << false;

  addRow(24, "Sol", defaults);
  addRow(24, "Offset (Constant)", defaults);
  addRow(24, "Offset (Order 1)", defaults);
  addRow(24, "Offset (Order 2)", defaults);
  addRow(24, "Com", defaults);
  addRow(24, "Usamp 1", defaults);
  addRow(24, "Usamp 2", defaults);
  addRow(24, "Raman", defaults);
}

CWNonlinearParametersDoasTable::~CWNonlinearParametersDoasTable()
{
}

//------------------------------------------------------------

CWShiftAndStretchDialog::CWShiftAndStretchDialog(const QStringList &freeSymbols, const QStringList &selectedSymbols,
						 QWidget *parent) :
  QDialog(parent)
{
  setWindowTitle("Symbol Selection");

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  m_symbolView = new QListWidget(this);
  mainLayout->addWidget(m_symbolView);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  mainLayout->addWidget(buttonBox);

  // populate the list .. selected symbols first
  QListWidgetItem *item;
  QStringList::const_iterator it = selectedSymbols.begin();
  while (it != selectedSymbols.end()) {
    item = new QListWidgetItem(*it, m_symbolView);
    item->setCheckState(Qt::Checked);
    ++it;
  }
  it = freeSymbols.begin();
  while (it != freeSymbols.end()) {
    item = new QListWidgetItem(*it, m_symbolView);
    item->setCheckState(Qt::Unchecked);
    ++it;
  }

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

CWShiftAndStretchDialog::~CWShiftAndStretchDialog()
{
}

QStringList CWShiftAndStretchDialog::selectedSymbols(void) const
{
  QStringList selection;
  QListWidgetItem *item;
  int index = 0;

  while (index < m_symbolView->count()) {
    item = m_symbolView->item(index);
    if (item->checkState() == Qt::Checked)
      selection << item->text();
    ++index;
  }

  return selection;
}


CWShiftAndStretchDoasTable::CWShiftAndStretchDoasTable(const QString &label, int columnWidth,
						       int headerHeight, QWidget *parent) :
  CDoasTable(label, columnWidth, headerHeight, parent),
  m_selectedRow(-1)
{
  m_specialSymbols << "Spectrum" << "Ref";
  m_freeSymbols = m_specialSymbols;

  QStringList comboItems;
  comboItems << "None" << "1st Order" << "2nd Order";

  // columns  
  createColumnCheck("Shift fit", 60);
  createColumnCombo("Stretch fit", 90, comboItems);
  createColumnCombo("Scaling fit", 90, comboItems);

  createColumnCheck("Sh store", 60);
  createColumnCheck("St store", 60);
  createColumnCheck("Sc store", 60);
  createColumnCheck("Err store", 60);
  
  createColumnEdit("Sh Init (px)", 80);
  createColumnEdit("St Init", 80);
  createColumnEdit("St Init (2)", 80);
  createColumnEdit("Sc Init", 80);
  createColumnEdit("Sc Init (2)", 80);

  createColumnEdit("Sh Delta (px)", 80);
  createColumnEdit("St Delta", 80);
  createColumnEdit("St Delta (2)", 80);
  createColumnEdit("Sc Delta", 80);
  createColumnEdit("Sc Delta (2)", 80);

  createColumnEdit("Sh min (px)", 80);
  createColumnEdit("Sh max (px)", 80);
}

CWShiftAndStretchDoasTable::~CWShiftAndStretchDoasTable()
{
}

void CWShiftAndStretchDoasTable::addRow(int height, const QString &label, QList<QVariant> &cellData)
{
  int index;
  QStringList symbols = label.split("; ");

  QStringList::iterator it = symbols.begin();
  while (it != symbols.end()) {
    // update the freeSymbol list
    index = m_freeSymbols.indexOf(*it);
    if (index != -1)
      m_freeSymbols.removeAt(index);

    // lock the symbol ... unless it is a special symbol
    if (m_specialSymbols.indexOf(*it) == -1)
      emit signalLockSymbol(*it, this);

    ++it;
  }

  m_selectedSymbolList.push_back(symbols); // add the row ...
  CDoasTable::addRow(height, label, cellData);
}

void CWShiftAndStretchDoasTable::removeRow(int rowIndex)
{
  if (rowIndex >= 0 && rowIndex < rowCount()) {

    const QStringList &symbols = m_selectedSymbolList.at(rowIndex);

    QStringList::const_iterator it = symbols.begin();
    while (it != symbols.end()) {
      // release locks ... unless special symbol
      if (m_specialSymbols.indexOf(*it) == -1)
	emit signalUnlockSymbol(*it, this);
      // put symbol back in the freeSymbol list
      m_freeSymbols << *it;
      ++it;
    }

    m_selectedSymbolList.removeAt(rowIndex);
    CDoasTable::removeRow(rowIndex);
  }
}

void CWShiftAndStretchDoasTable::cellDataChanged(int row, int column, const QVariant &cellData)
{
}

void CWShiftAndStretchDoasTable::contextMenuEvent(QContextMenuEvent *e)
{
  // create a popup menu
  QMenu menu;
  
  QAction *insertAction = menu.addAction("Insert", this, SLOT(slotInsertRow()));
  QAction *removeAction = menu.addAction("Remove", this, SLOT(slotRemoveRow()));
  QAction *modifyAction = menu.addAction("Modify", this, SLOT(slotModifyRow()));

  m_selectedRow = rowIndexAtPosition(e->y());

  insertAction->setEnabled(!m_freeSymbols.empty());
  removeAction->setEnabled(m_selectedRow != -1);
  modifyAction->setEnabled(m_selectedRow != -1);

  menu.exec(e->globalPos()); // a slot will do the rest  
}

void CWShiftAndStretchDoasTable::slotSymbolListChanged(const QStringList &symbols)
{
  bool found;
  QList<QStringList>::const_iterator listIt;

  m_freeSymbols.clear();

  // rebuild the free symbol list

  QStringList::const_iterator it = symbols.begin();
  while (it != symbols.end()) {
    found = false;
    listIt = m_selectedSymbolList.begin();
    while (!found && listIt != m_selectedSymbolList.end()) {
      found = (listIt->indexOf(*it) != -1);
      ++listIt;
    }
    if (!found)
      m_freeSymbols << *it;
    ++it;
  }

  // and the special symbols ...
  
  it = m_specialSymbols.begin();
  while (it != m_specialSymbols.end()) {
    found = false;
    listIt = m_selectedSymbolList.begin();
    while (!found && listIt != m_selectedSymbolList.end()) {
      found = (listIt->indexOf(*it) != -1);
      ++listIt;
    }
    if (!found)
      m_freeSymbols << *it;
    ++it;
  }
  
}

void CWShiftAndStretchDoasTable::slotInsertRow()
{
  // popup a dialog for selecting a symbol (list) ...
  CWShiftAndStretchDialog dialog(m_freeSymbols, QStringList(), this);
  
  if (dialog.exec() == QDialog::Accepted) {
    QStringList selection = dialog.selectedSymbols();

    if (!selection.isEmpty()) { 

      QStringList::iterator it = selection.begin();
      QString tmp = *it;
      ++it;
      while (it != selection.end()) {
	tmp.append("; ");
	tmp.append(*it);
	++it;
      }

      QList<QVariant> initialValues;
      addRow(24, tmp, initialValues);
    }
  }
}

void CWShiftAndStretchDoasTable::slotRemoveRow()
{
  if (m_selectedRow >= 0 && m_selectedRow < rowCount()) {

    removeRow(m_selectedRow);
    m_selectedRow = -1;
  }
}

void CWShiftAndStretchDoasTable::slotModifyRow()
{
  if (m_selectedRow >= 0 && m_selectedRow < rowCount()) {

    // popup a dialog for modyfying a symbol (list) ...
    CWShiftAndStretchDialog dialog(m_freeSymbols, m_selectedSymbolList.at(m_selectedRow), this);
  
    if (dialog.exec() == QDialog::Accepted) {
      QStringList selection = dialog.selectedSymbols();

      if (!selection.isEmpty()) { 
	// OK to change ...

	int index;
	const QStringList &old = m_selectedSymbolList.at(m_selectedRow);

	// release the locks on the original selection and free those symbols
	QStringList::const_iterator it = old.begin();
	while (it != old.end()) {
	  if (m_specialSymbols.indexOf(*it) == -1)
	    emit signalUnlockSymbol(*it, this);

	  ++it;
	}
	m_freeSymbols << old;

	// now change and lock the new selection ... and change the label ...
	m_selectedSymbolList.replace(m_selectedRow, selection);

	QString tmp;

	it = selection.begin();
	while (it != selection.end()) {
	  if (m_specialSymbols.indexOf(*it) == -1)
	    emit signalLockSymbol(*it, this);

	  index = m_freeSymbols.indexOf(*it);
	  if (index != -1)
	    m_freeSymbols.removeAt(index);

	  if (!tmp.isEmpty())
	    tmp.append("; ");
	  tmp.append(*it);

	  ++it;
	}


	setHeaderLabel(m_selectedRow, tmp);
      }
    }
  }
}


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
  CDoasTable(label, columnWidth, headerHeight, parent)
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
  // update the symbol list and signal ...
  m_symbols << label;
  emit signalSymbolListChanged(m_symbols);

  // really create the new row ...
  CDoasTable::addRow(height, label, cellData);
}

void CWMoleculesDoasTable::removeRow(int rowIndex)
{
  m_symbols.removeAt(rowIndex);
  emit signalSymbolListChanged(m_symbols);

  // really remove the row ...
  CDoasTable::removeRow(rowIndex);  
}

const QStringList& CWMoleculesDoasTable::symbolList(void) const
{
  return m_symbols;
}

void CWMoleculesDoasTable::cellDataChanged(int row, int column, const QVariant &cellData)
{
  std::cout << "Changed : " << row << "," << column << " : " << cellData.toString().toStdString() << std::endl;
  if (column == 1) {
    setCellEnabled(row, 8, (cellData.toString() == "Convolve IO"));
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

CWShiftAndStretchDoasTable::CWShiftAndStretchDoasTable(const QString &label, int columnWidth,
						       int headerHeight, QWidget *parent) :
  CDoasTable(label, columnWidth, headerHeight, parent)
{
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

void CWShiftAndStretchDoasTable::cellDataChanged(int row, int column, const QVariant &cellData)
{
}

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
#include <QFileDialog>

#include "CWAnalysisWindowDoasTables.h"
#include "CWorkSpace.h"
#include "CPreferences.h"

#include "constants.h"

#include "debugutil.h"

//------------------------------------------------------------

static const int cStandardRowHeight = 24;

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
  comboItems << "None" << "Interpolate" << "Convolve Std" << "Convolve Io" << "Convolve Ring";
  createColumnCombo("Interp/Conv", 120, comboItems);

  comboItems.clear();
  comboItems << "None" << "SZA only" << "Climatology" << "Wavelength 1" << "Wavelength 2" << "Wavelength 3";
  createColumnCombo("AMF", 120, comboItems); // columnIndex = 2

  createColumnCheck("Fit disp.", 60);
  createColumnCheck("Filter", 60);           // columnIndex = 4
  createColumnCheck("CC fit", 60);
  createColumnEdit("CC Init", 80);           // columnIndex = 6
  createColumnEdit("CC Delta", 80);
  createColumnCheck("CC Io", 60);            // columnIndex = 8
}
  
CWMoleculesDoasTable::~CWMoleculesDoasTable()
{
}

void CWMoleculesDoasTable::populate(const struct anlyswin_cross_section *d, int nElements)
{
  int row = 0;

  while (row < nElements) {
    QList<QVariant> initialValues;

    initialValues.push_back(QString(d->orthogonal));
    initialValues.push_back(mapCrossTypeToComboString(d->crossType));
    initialValues.push_back(mapAmfTypeToComboString(d->amfType));
    initialValues.push_back(d->requireFit);
    initialValues.push_back(d->requireFilter);
    initialValues.push_back(d->requireCcFit);
    initialValues.push_back(d->initialCc);
    initialValues.push_back(d->deltaCc);
    initialValues.push_back(d->requireCcIo);

    // add the row (with filenames)
    addRow(cStandardRowHeight, QString(d->symbol), initialValues,
	   QString(d->crossSectionFile), QString(d->amfFile));

    ++d; // iterate over the array
    ++row;
  }
}

void CWMoleculesDoasTable::apply(struct anlyswin_cross_section *d, int &nElements) const
{
  int row = 0;
  
  nElements = rowCount();

  while (row < nElements) {

    QList<QVariant> state = getCellData(row);

    // certain that the length of the symbol is below the limit ...
    strcpy(d->symbol, m_symbols.at(row).toAscii().data());

    strcpy(d->crossSectionFile, m_csFilename.at(row).toAscii().data());

    strcpy(d->orthogonal, state.at(0).toString().toAscii().data());
    d->crossType = mapComboStringToCrossType(state.at(1).toString());
    d->amfType = mapComboStringToAmfType(state.at(2).toString());

    strcpy(d->amfFile, m_amfFilename.at(row).toAscii().data());

    d->requireFit = state.at(3).toBool() ? 1 : 0;
    d->requireFilter = state.at(4).toBool() ? 1 : 0;
    d->requireCcFit = state.at(5).toBool() ? 1 : 0;
    d->initialCc = state.at(6).toDouble();
    d->deltaCc = state.at(7).toDouble();
    d->requireCcIo = state.at(8).toBool() ? 1 : 0;

    ++d;
    ++row;
  }
}

void CWMoleculesDoasTable::addRow(int height, const QString &label, QList<QVariant> &cellData,
				  const QString &csFilename, const QString &amfFilename)
{
  addRow(height, label, cellData);
    
  // replace the values for the filenames ... at the tail of the list ...
  if (!csFilename.isEmpty()) {
    m_csFilename.pop_back();
    m_csFilename << csFilename;
  }

  if (!amfFilename.isEmpty()) {
    m_amfFilename.pop_back();
    m_amfFilename << amfFilename;
  }
}

void CWMoleculesDoasTable::addRow(int height, const QString &label, QList<QVariant> &cellData)
{
  // update the symbol list, rowlocks list and signal ...
  m_symbols << label;
  m_rowLocks << -1; // this new row does not lock any other row

  m_csFilename << QString();  // add an empty string ...
  m_amfFilename << QString(); // add an empty string ...
  
  emit signalSymbolListChanged(m_symbols);

  // really create the new row ...
  CDoasTable::addRow(height, label, cellData);
}

void CWMoleculesDoasTable::removeRow(int rowIndex)
{
  // removal is only permitted if the row is not locked by another row
  if (rowIndex >= 0 && rowIndex < rowCount() && !isRowLocked(rowIndex)) {

    m_csFilename.removeAt(rowIndex);
    m_amfFilename.removeAt(rowIndex);

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

QString CWMoleculesDoasTable::mapCrossTypeToComboString(int type)
{
  switch (type) {
  case ANLYS_CROSS_ACTION_INTERPOLATE: return QString("Interpolate"); break;
  case ANLYS_CROSS_ACTION_CONVOLUTE: return QString("Convolve Std"); break;
  case ANLYS_CROSS_ACTION_CONVOLUTE_I0: return QString("Convolve Io"); break;
  case ANLYS_CROSS_ACTION_CONVOLUTE_RING: return QString("Convolve Ring"); break;
  }

  return QString("None");
}

int CWMoleculesDoasTable::mapComboStringToCrossType(const QString &str)
{
  if (str == "Interpolate") return ANLYS_CROSS_ACTION_INTERPOLATE;
  if (str == "Convolve Std") return ANLYS_CROSS_ACTION_CONVOLUTE;
  if (str == "Convolve Io") return ANLYS_CROSS_ACTION_CONVOLUTE_I0;
  if (str == "Convolve Ring") return ANLYS_CROSS_ACTION_CONVOLUTE_RING;
  
  return ANLYS_CROSS_ACTION_NOTHING;
}

QString CWMoleculesDoasTable::mapAmfTypeToComboString(int type)
{
  switch (type) {
  case ANLYS_AMF_TYPE_SZA: return QString("SZA only"); break;
  case ANLYS_AMF_TYPE_CLIMATOLOGY: return QString("Climatology"); break;
  case ANLYS_AMF_TYPE_WAVELENGTH1: return QString("Wavelength 1"); break;
  case ANLYS_AMF_TYPE_WAVELENGTH2: return QString("Wavelength 2"); break;
  case ANLYS_AMF_TYPE_WAVELENGTH3: return QString("Wavelength 3"); break;
  }

  return QString("None");
}

int CWMoleculesDoasTable::mapComboStringToAmfType(const QString &str)
{
  if (str == "SZA only") return ANLYS_AMF_TYPE_SZA;
  if (str == "Climatology") return ANLYS_AMF_TYPE_CLIMATOLOGY;
  if (str == "Wavelength 1") return ANLYS_AMF_TYPE_WAVELENGTH1;
  if (str == "Wavelength 2") return ANLYS_AMF_TYPE_WAVELENGTH2;
  if (str == "Wavelength 3") return ANLYS_AMF_TYPE_WAVELENGTH3;

  return ANLYS_AMF_TYPE_NONE;
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
    setCellEnabled(row, 8, (cellData.toString() == "Convolve Io"));
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
  QStringList allSymbols = CWorkSpace::instance()->symbolList();
  QStringList freeSymbols;
  QString filter;
  int index;

  // build a list of free symbol and a file filter string
  QStringList::const_iterator it = allSymbols.begin();
  while (it != allSymbols.end()) {
    index = m_symbols.indexOf(*it);
    if (index == -1) {
      // a free symbol ...
      freeSymbols << *it;
      filter.append(*it).append(" (").append(*it).append("_*.xs*);;");
    }
    ++it;
  }

  // check that the selected file is appropriate ...
  if (!filter.isEmpty()) {    
    filter.append("All files (*.*)");

    CPreferences *prefs = CPreferences::instance();

    QString fileName = QFileDialog::getOpenFileName(this, "Select Cross Section File",
						    prefs->directoryName("CrossSection"), filter);

    if (!fileName.isEmpty() && fileName.length() < FILENAME_BUFFER_LENGTH) {

      prefs->setDirectoryNameGivenFile("CrossSection", fileName);

      // need to compare 'symbol_' with the start of the basename of the file ...
      QString baseName = CPreferences::baseName(fileName);

      // the start of the filename MUST match a free symbol ...
      it = freeSymbols.begin();
      while (it != freeSymbols.end()) {
	QString tmp(*it);
	tmp.append('_');
	if (baseName.startsWith(tmp, Qt::CaseInsensitive)) {
	  // a match to a symbol ... OK to add the row ...
	  QList<QVariant> initialValues;
	  addRow(cStandardRowHeight, *it, initialValues, fileName, QString());
	  return;
	}
	++it;
      }
    }
  }
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

void CWMoleculesDoasTable::slotFitColumnCheckable(int state)
{
  setColumnEnabled(3, (state == Qt::Checked));
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
}

void CWLinearParametersDoasTable::populate(const struct anlys_linear *data)
{						 
  // predefined rows
  QList<QVariant> initialValues;

  initialValues.push_back(mapOrderToComboString(data->xPolyOrder));
  initialValues.push_back(mapOrderToComboString(data->xBaseOrder));
  initialValues.push_back(data->xFlagFitStore); // bool
  initialValues.push_back(data->xFlagErrStore); // bool 
  addRow(cStandardRowHeight, "Polynomial(x)", initialValues);

  initialValues.clear();

  initialValues.push_back(mapOrderToComboString(data->xinvPolyOrder));
  initialValues.push_back(mapOrderToComboString(data->xinvBaseOrder));
  initialValues.push_back(data->xinvFlagFitStore); // bool
  initialValues.push_back(data->xinvFlagErrStore); // bool 
  addRow(cStandardRowHeight, "Polynomial(1/x)", initialValues);

  initialValues.clear();

  initialValues.push_back(mapOrderToComboString(data->offsetPolyOrder));
  initialValues.push_back(mapOrderToComboString(data->offsetBaseOrder));
  initialValues.push_back(data->offsetFlagFitStore); // bool
  initialValues.push_back(data->offsetFlagErrStore); // bool 
  addRow(cStandardRowHeight, "Offset", initialValues);
}

CWLinearParametersDoasTable::~CWLinearParametersDoasTable()
{
}

void CWLinearParametersDoasTable::apply(struct anlys_linear *data) const
{
  QList<QVariant> state;

  state = getCellData(0); // xPoly
  data->xPolyOrder = mapComboStringToOrder(state.at(0).toString());
  data->xBaseOrder = mapComboStringToOrder(state.at(1).toString());
  data->xFlagFitStore = state.at(2).toBool() ? 1 : 0;
  data->xFlagErrStore = state.at(3).toBool() ? 1 : 0;

  state = getCellData(1); // xinvPoly
  data->xinvPolyOrder = mapComboStringToOrder(state.at(0).toString());
  data->xinvBaseOrder = mapComboStringToOrder(state.at(1).toString());
  data->xinvFlagFitStore = state.at(2).toBool() ? 1 : 0;
  data->xinvFlagErrStore = state.at(3).toBool() ? 1 : 0;

  state = getCellData(2); // xinvPoly
  data->offsetPolyOrder = mapComboStringToOrder(state.at(0).toString());
  data->offsetBaseOrder = mapComboStringToOrder(state.at(1).toString());
  data->offsetFlagFitStore = state.at(2).toBool() ? 1 : 0;
  data->offsetFlagErrStore = state.at(3).toBool() ? 1 : 0;
}

QString CWLinearParametersDoasTable::mapOrderToComboString(int order)
{
  switch (order) {
  case ANLYS_POLY_TYPE_0: return QString("Order 0"); break;
  case ANLYS_POLY_TYPE_1: return QString("Order 1"); break;
  case ANLYS_POLY_TYPE_2: return QString("Order 2"); break;
  case ANLYS_POLY_TYPE_3: return QString("Order 3"); break;
  case ANLYS_POLY_TYPE_4: return QString("Order 4"); break;
  case ANLYS_POLY_TYPE_5: return QString("Order 5"); break;
  }

  return QString("None");
}

int CWLinearParametersDoasTable::mapComboStringToOrder(const QString &str)
{
  if (str == "Order 0") return ANLYS_POLY_TYPE_0;
  if (str == "Order 1") return ANLYS_POLY_TYPE_1;
  if (str == "Order 2") return ANLYS_POLY_TYPE_2;
  if (str == "Order 3") return ANLYS_POLY_TYPE_3;
  if (str == "Order 4") return ANLYS_POLY_TYPE_4;
  if (str == "Order 5") return ANLYS_POLY_TYPE_5;
  
  return ANLYS_POLY_TYPE_NONE;
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

  addRow(cStandardRowHeight, "Sol", defaults);
  addRow(cStandardRowHeight, "Offset (Constant)", defaults);
  addRow(cStandardRowHeight, "Offset (Order 1)", defaults);
  addRow(cStandardRowHeight, "Offset (Order 2)", defaults);
  addRow(cStandardRowHeight, "Com", defaults);
  addRow(cStandardRowHeight, "Usamp 1", defaults);
  addRow(cStandardRowHeight, "Usamp 2", defaults);
  addRow(cStandardRowHeight, "Raman", defaults);
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

void CWShiftAndStretchDoasTable::populate(const struct anlyswin_shift_stretch *d, int nElements)
{
  int row;
  
  while (row < nElements) {
    QString label;
    QList<QVariant> initialValues;

    initialValues.push_back(d->shFit);
    initialValues.push_back(QString("None")); //d->stFit);
    initialValues.push_back(QString("None")); //d->scFit);

    initialValues.push_back(d->shStore);
    initialValues.push_back(d->stStore);
    initialValues.push_back(d->scStore);
    initialValues.push_back(d->errStore);

    initialValues.push_back(d->shInit);
    initialValues.push_back(d->stInit);
    initialValues.push_back(d->stInit2);
    initialValues.push_back(d->scInit);
    initialValues.push_back(d->scInit2);

    initialValues.push_back(d->shDelta);
    initialValues.push_back(d->stDelta);
    initialValues.push_back(d->stDelta2);
    initialValues.push_back(d->scDelta);
    initialValues.push_back(d->scDelta2);

    initialValues.push_back(d->shMin);
    initialValues.push_back(d->shMax);
    
    int i = 0;
    while (i < d->nSymbol) {
      if (i == 0) {
	label = QString(d->symbol[i]);
      }
      else {
	label.append("; ");
	label.append(QString(d->symbol[i]));
      }
      ++i;
    }

    addRow(cStandardRowHeight, label, initialValues);

    ++d;
    ++row;
  }
}

void CWShiftAndStretchDoasTable::apply(struct anlyswin_shift_stretch *d, int &nElements) const
{
  // TODO TODO TODO
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
      addRow(cStandardRowHeight, tmp, initialValues);
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


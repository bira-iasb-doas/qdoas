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

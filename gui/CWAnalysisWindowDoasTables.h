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

#include "CDoasTable.h"

// fixed size (3 row x 3 column) table ...

class CWLinearParametersDoasTable : public CDoasTable
{
 public:
  CWLinearParametersDoasTable(const QString &label, int columnWidth, int headerHeight = 24, QWidget *parent = 0);
  virtual ~CWLinearParametersDoasTable();

  // virtual void cellDataChanged(int row, int column, const QVariant &cellData); // no cell-coupling required
};


// fixed size (8 row x 5 column) table ...

class CWNonlinearParametersDoasTable : public CDoasTable
{
 public:
  CWNonlinearParametersDoasTable(const QString &label, int columnWidth, int headerHeight = 24, QWidget *parent = 0);
  virtual ~CWNonlinearParametersDoasTable();

  // virtual void cellDataChanged(int row, int column, const QVariant &cellData); // no cell-coupling required
};

// variable rows
class CWShiftAndStretchDoasTable : public CDoasTable
{
 public:
  CWShiftAndStretchDoasTable(const QString &label, int columnWidth, int headerHeight = 24, QWidget *parent = 0);
  virtual ~CWShiftAndStretchDoasTable();

  virtual void cellDataChanged(int row, int column, const QVariant &cellData); // no cell-coupling required
};

#endif

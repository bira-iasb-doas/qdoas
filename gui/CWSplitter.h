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

#ifndef _CWSPLITTER_H_GUARD
#define _CWSPLITTER_H_GUARD

#include <map>

#include <QSplitter>

// Extends QSplitter to allow control of the first splitter position
// via signal-slot mechanism

class CWSplitter : public QSplitter
{
Q_OBJECT
 public:
  CWSplitter(Qt::Orientation orientation, QWidget *parent = 0);
  virtual ~CWSplitter();

  public slots:
    void slotSetWidthMode(int mode);

 private:
  std::map<int,int> m_modeToSizeMap;
  int m_currentMode;
};

#endif
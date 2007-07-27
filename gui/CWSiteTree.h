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

#ifndef _CWSITETREE_H_GUARD
#define _CWSITETREE_H_GUARD

#include <QTreeWidget>

class CWActiveContext;

class CWSiteTree : public QTreeWidget
{
Q_OBJECT
 public:
  CWSiteTree(CWActiveContext *activeContext, QWidget *parent = 0);
  virtual ~CWSiteTree();

  void addNewSite(const QString &siteName, const QString &abreviation = QString(),
                  double longitude = 0.0, double latitude = 0.0, double altitude = 0.0);
  void modifySite(const QString &siteName, const QString &abreviation,
                  double longitude, double latitude, double altitude);

 protected:
  virtual void contextMenuEvent(QContextMenuEvent *e);
  virtual void showEvent(QShowEvent *e);

 public slots:
   void slotAddNewSite();
   void slotEditSite();
   void slotDeleteSite();

 signals:
  void signalWidthModeChanged(int newMode);

 private:
  CWActiveContext *m_activeContext;
};

#endif

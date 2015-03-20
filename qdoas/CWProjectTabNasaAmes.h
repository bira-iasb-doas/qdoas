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


#ifndef _CWPROJECTTABNASAAMES_H_GUARD
#define _CWPROJECTTABNASAAMES_H_GUARD

#include <QFrame>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>

#include "mediate_project.h"

class CWProjectTabNasaAmes : public QFrame
{
Q_OBJECT
 public:
  CWProjectTabNasaAmes(const QString &projectName, const mediate_project_nasa_ames_t *properties, QWidget *parent = 0);

  void apply(mediate_project_nasa_ames_t *properties) const;

 public slots:
  void slotBrowsePath();
  void slotSaveCheckChanged(int state);

 private:
  QFrame *m_pathFrame;
  QLineEdit *m_pathEdit;
  QCheckBox *m_saveCheck, *m_rejectTestCheck;
  QLabel *m_instrumentLabel, *m_experimentLabel;
  QLineEdit *m_instrumentEdit, *m_experimentEdit;
  QGroupBox *m_comboGroup;
  QComboBox *m_awComboNO2, *m_awComboO3, *m_awComboBrO, *m_awComboOClO;
};


#endif

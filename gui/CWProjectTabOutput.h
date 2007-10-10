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


#ifndef _CWPROJECTTABOUTPUT_H_GUARD
#define _CWPROJECTTABOUTPUT_H_GUARD

#include <QFrame>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QListWidget>
#include <QListWidgetItem>

#include "mediate_project.h"

class CWOutputSelector;

class CWProjectTabOutput : public QFrame
{
Q_OBJECT
 public:
  CWProjectTabOutput(const mediate_project_output_t *properties, QWidget *parent = 0);
  virtual ~CWProjectTabOutput();

  void apply(mediate_project_output_t *properties) const;

  void setComponentsEnabled(bool analysisEnabled, bool calibrationEnabled);

 public slots:
  void slotBrowsePath();
  void slotInstrumentChanged(int instrument);
  void slotAnalysisCheckChanged(int state);
  void slotCalibrationCheckChanged(int state);


 private:
  QFrame *m_pathFrame;
  QLineEdit *m_pathEdit;
  QCheckBox *m_analysisCheck, *m_calibrationCheck;
  QCheckBox *m_configurationCheck, *m_binaryCheck, *m_directoryCheck;
  QGroupBox *m_editGroup;
  QLineEdit *m_fluxEdit, *m_colourIndexEdit;
  CWOutputSelector *m_selector;
  
};

class CWOutputSelector : public QFrame
{
Q_OBJECT
 public:
  CWOutputSelector(const mediate_project_output_t *d, QWidget *parent = 0);
  virtual ~CWOutputSelector();

  void apply(mediate_project_output_t *d);
  void setInstrument(int instrument);

  static QListWidgetItem* locateItemByKey(QListWidget *listWidget, int key);

 public slots:
  void slotToChosenList();
  void slotToAvailableList();

 private:
  QListWidget *m_availableList, *m_chosenList;
};

class CWOutputFieldItem : public QListWidgetItem
{
 public:
  CWOutputFieldItem(int key, const QString &text, QListWidget *parent = 0);
  virtual ~CWOutputFieldItem();

  virtual QVariant data(int role) const; // implement Qt::UserRole to get key
  
 private:
  int m_key;
};

#endif

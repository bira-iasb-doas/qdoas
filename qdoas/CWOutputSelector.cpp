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


#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

#include "CWOutputSelector.h"

#include "constants.h"

#include "debugutil.h"


void getValidFieldFlags(int *validFlags, int instrument);


CWOutputSelector::CWOutputSelector(const data_select_list_t *d, QWidget *parent) :
  QFrame(parent)
{
  QGridLayout *mainLayout = new QGridLayout(this);

  mainLayout->addWidget(new QLabel("Available Fields"), 0, 0, Qt::AlignCenter);
  mainLayout->addWidget(new QLabel("Selected Fields"), 0, 2, Qt::AlignCenter);

  m_availableList = new QListWidget(this);
  m_chosenList = new QListWidget(this);

  QPushButton *toChosenBtn = new QPushButton(QIcon(":/icons/to_arrow.png"), QString(), this);
  QPushButton *toAvailableBtn = new QPushButton(QIcon(":/icons/from_arrow.png"), QString(), this);

  mainLayout->addWidget(m_availableList, 1, 0, 4, 1);
  mainLayout->addWidget(toChosenBtn, 2, 1);
  mainLayout->addWidget(toAvailableBtn, 3, 1);
  mainLayout->addWidget(m_chosenList, 1, 2, 4, 1);

  mainLayout->setRowMinimumHeight(1, 20);
  mainLayout->setRowMinimumHeight(4, 20);
  mainLayout->setRowStretch(1, 1);
  mainLayout->setRowStretch(4, 1);

  // multi selection
  m_availableList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_chosenList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  // populate the with all possible fields initially.
  // These CAN be added in any desired order (the display order in the availble list)

  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_SPECNO,              "Spec No"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_NAME,                "Name"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_DATE_TIME,           "Date & time (YYYYMMDDhhmmss)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_DATE,                "Date (DD/MM/YYYY)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_TIME,                "Time (hh:mm:ss)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_YEAR,                "Year"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_JULIAN,              "Day number"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_JDFRAC,              "Fractional day"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_TIFRAC,              "Fractional time"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_SCANS,               "Scans"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_NREJ,                "Rejected"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_TINT,                "Tint"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_SZA,                 "SZA"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_CHI,                 "Chi Square"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_RMS,                 "RMS"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_AZIM,                "Solar Azimuth angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_TDET,                "Tdet"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_SKY,                 "Sky Obs"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_BESTSHIFT,           "Best shift"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_REFZM,               "Ref SZA"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_REFSHIFT,            "Ref2/Ref1 shift"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_PIXEL,               "Pixel number"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_PIXEL_TYPE,          "Pixel type"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_ORBIT,               "Orbit number"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_LONGIT,              "Longitude"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_LATIT,               "Latitude"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_ALTIT,               "Altitude"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_COVAR,               "Covariances"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_CORR,                "Correlations"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_CLOUD,               "Cloud fraction"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_COEFF,               "Index coeff"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_O3,                  "GDP O3 VCD"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_NO2,                 "GDP NO2 VCD"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_CLOUDTOPP,           "Cloud Top Pressure"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_LOS_ZA,              "LoS ZA"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_LOS_AZIMUTH,         "LoS Azimuth"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_SAT_HEIGHT,          "Satellite height"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_EARTH_RADIUS,        "Earth radius"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_VIEW_ELEVATION,      "Elev. viewing angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_VIEW_AZIMUTH,        "Azim. viewing angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_SCIA_QUALITY,        "SCIAMACHY Quality Flag"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_SCIA_STATE_INDEX,    "SCIAMACHY State Index"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_SCIA_STATE_ID,       "SCIAMACHY State Id"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_STARTDATE,           "Start Date (DDMMYYYY)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_ENDDATE,             "End Date (DDMMYYYY)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_STARTTIME,           "Start Time (hhmmss)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_ENDTIME,             "Stop Time (hhmmss)"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_SCANNING,            "Scanning angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_FILTERNUMBER,        "Filter number"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_MEASTYPE,            "Measurement type"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_CCD_HEADTEMPERATURE, "Head temperature"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_COOLING_STATUS,      "Cooler status"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_MIRROR_ERROR,        "Mirror status"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_COMPASS,             "Compass angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_PITCH,               "Pitch angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_ROLL,                "Roll angle"));
  m_availableList->addItem(new CWOutputFieldItem(PRJCT_RESULTS_ASCII_ITER,                "Iteration number"));

  // populate the selected list by key-reference to the available list ...

  int key;
  int i = 0;
  while (i < d->nSelected) {
    key = d->selected[i];

    // locate corresponding item, hide it, and add matching item, in the chosen list
    QListWidgetItem *avItem = locateItemByKey(m_availableList, key);
    if (avItem) {
      m_chosenList->addItem(new CWOutputFieldItem(key, avItem->text()));
      avItem->setHidden(true);
    }
    ++i;
  }

  connect(toChosenBtn, SIGNAL(clicked()), this, SLOT(slotToChosenList()));
  connect(toAvailableBtn, SIGNAL(clicked()), this, SLOT(slotToAvailableList()));
}

CWOutputSelector::~CWOutputSelector()
{
}

void CWOutputSelector::apply(data_select_list_t *d)
{
  int n = 0;
  int row = 0;
  while (row < m_chosenList->count()) {
    QListWidgetItem *item = m_chosenList->item(row);
    if (item && !item->isHidden()) {
      d->selected[n] = (unsigned char)(item->data(Qt::UserRole).toInt());
      ++n;
    }
    ++row;
  }
  d->nSelected = n;
}

void CWOutputSelector::setInstrument(int instrument)
{
  int validFlags[PRJCT_RESULTS_ASCII_MAX];

  // get this information from somewhere ...
  getValidFieldFlags(validFlags, instrument);

  for (int key=0; key<PRJCT_RESULTS_ASCII_MAX; ++key) {
    bool hideItems = (validFlags[key] == 0);
    QListWidgetItem *avItem = locateItemByKey(m_availableList, key);
    QListWidgetItem *chItem = locateItemByKey(m_chosenList, key);
    if (chItem)
      chItem->setHidden(hideItems);
    if (avItem)
      avItem->setHidden(hideItems || chItem != NULL);
  }
}

void CWOutputSelector::slotToChosenList()
{
  int key;
  QList<QListWidgetItem*> items = m_availableList->selectedItems();
  QList<QListWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    if (!(*it)->isHidden()) {
      // hide these items and add to the chosen list
      key = (*it)->data(Qt::UserRole).toInt();
      m_chosenList->addItem(new CWOutputFieldItem(key, (*it)->text()));
      (*it)->setHidden(true);
    }
    ++it;
  }
  m_availableList->selectionModel()->clearSelection();
}

void CWOutputSelector::slotToAvailableList()
{
  int key;
  QList<QListWidgetItem*> items = m_chosenList->selectedItems();
  QList<QListWidgetItem*>::iterator it = items.begin();
  while (it != items.end()) {
    key = (*it)->data(Qt::UserRole).toInt();
    // find the corresponding item in the available list and make it 'un-hidden'
    QListWidgetItem *avItem = locateItemByKey(m_availableList, key);
    if (avItem)
      avItem->setHidden((*it)->isHidden());
    // row of the selected item
    key = m_chosenList->row(*it);
    ++it;
    delete m_chosenList->takeItem(key); // discard the selected item
  }
}

QListWidgetItem* CWOutputSelector::locateItemByKey(QListWidget *listWidget, int key)
{
  QListWidgetItem *item;
  int row = 0;

  while (row < listWidget->count()) {
    item = listWidget->item(row);
    if (item && key == item->data(Qt::UserRole).toInt())
      return item;

    ++row;
  }
  // not found
  return NULL;
}

//------------------------------------------------------

CWOutputFieldItem::CWOutputFieldItem(int key, const QString &text, QListWidget * parent) :
  QListWidgetItem(text, parent),
  m_key(key)
{
}

CWOutputFieldItem::~CWOutputFieldItem()
{
}

QVariant CWOutputFieldItem::data(int role) const
{
  if (role == Qt::UserRole)
    return QVariant(m_key);

  return QListWidgetItem::data(role);
}

//------------------------------------------------------

void getValidFieldFlags(int *validFlags, int instrument)
 {
 	int satelliteFlag;

 	satelliteFlag=
 	  ((instrument==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
     (instrument==PRJCT_INSTR_FORMAT_GDP_BIN) ||
     (instrument==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
     (instrument==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
     (instrument==PRJCT_INSTR_FORMAT_OMI) ||
     (instrument==PRJCT_INSTR_FORMAT_GOME2))?1:0;

  // validFlags is indexed by the PRJCT_RESULTS_ASCII_* enumerated values. A non-zero
  // value means that the corresponding field is valid for the instrument. The value of
  // instrument should correspond to a PRJCT_INSTR_* enumerated value.
  // The validFlags array MUST have a length of at least PRJCT_RESULTS_ASCII_MAX.

  // zero all flags
  memset(validFlags, 0, PRJCT_RESULTS_ASCII_MAX * sizeof(int));

  // Fields that are common to all formats (note that date and time fields could be absent from ASCII file)

  validFlags[PRJCT_RESULTS_ASCII_SPECNO]=                                       // record number
  validFlags[PRJCT_RESULTS_ASCII_DATE_TIME]=                                    // date and time
  validFlags[PRJCT_RESULTS_ASCII_DATE]=                                         // date
  validFlags[PRJCT_RESULTS_ASCII_TIME]=                                         // time
  validFlags[PRJCT_RESULTS_ASCII_YEAR]=                                         // year
  validFlags[PRJCT_RESULTS_ASCII_JULIAN]=                                       // julian day
  validFlags[PRJCT_RESULTS_ASCII_JDFRAC]=                                       // fractional julian day
  validFlags[PRJCT_RESULTS_ASCII_TIFRAC]=                                       // fractional time
  validFlags[PRJCT_RESULTS_ASCII_SZA]=                                          // solar zenith angle (can be calculated if date, time and observation site specified)
  validFlags[PRJCT_RESULTS_ASCII_AZIM]=                                         // solar azimuth angle (can be calculated if date, time and observation site specified)
  validFlags[PRJCT_RESULTS_ASCII_CHI]=                                          // chi square
  validFlags[PRJCT_RESULTS_ASCII_RMS]=                                          // RMS
  validFlags[PRJCT_RESULTS_ASCII_REFSHIFT]=                                     // in automatic reference selection, shift of the reference spectrum
  validFlags[PRJCT_RESULTS_ASCII_ITER]=
  validFlags[PRJCT_RESULTS_ASCII_TINT]=1;                                       // the integration time

  validFlags[PRJCT_RESULTS_ASCII_REFZM]=(satelliteFlag)?0:1;                    // in automatic reference selection, the solar zenith angle of the reference spectrum
  validFlags[PRJCT_RESULTS_ASCII_SKY]=0;                                        // information on the sky (never used except EASOE campaign, 1991 !)                                      // for satellite measurements, several spectra are averaged

  validFlags[PRJCT_RESULTS_ASCII_LONGIT]=satelliteFlag;
  validFlags[PRJCT_RESULTS_ASCII_LATIT]=satelliteFlag;
  validFlags[PRJCT_RESULTS_ASCII_SAT_HEIGHT]=satelliteFlag;
  validFlags[PRJCT_RESULTS_ASCII_EARTH_RADIUS]=satelliteFlag;
  validFlags[PRJCT_RESULTS_ASCII_ORBIT]=satelliteFlag;

  validFlags[PRJCT_RESULTS_ASCII_VIEW_ELEVATION]=satelliteFlag;
  validFlags[PRJCT_RESULTS_ASCII_VIEW_AZIMUTH]=satelliteFlag;

  // set the appropriate flags

  switch (instrument)
   {
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_ASCII:
     {
      validFlags[PRJCT_RESULTS_ASCII_VIEW_ELEVATION]=1;
      validFlags[PRJCT_RESULTS_ASCII_VIEW_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_ASCII_TINT]=0;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_ACTON:
     {
      validFlags[PRJCT_RESULTS_ASCII_NAME]=1;
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
      validFlags[PRJCT_RESULTS_ASCII_NREJ]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_LOGGER:
     {
      validFlags[PRJCT_RESULTS_ASCII_NAME]=1;
      validFlags[PRJCT_RESULTS_ASCII_VIEW_ELEVATION]=1;                         // absent for Harestua, present for OHP MAX-DOAS
      validFlags[PRJCT_RESULTS_ASCII_VIEW_AZIMUTH]=1;                           // absent for Harestua, present for OHP MAX-DOAS
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
      validFlags[PRJCT_RESULTS_ASCII_NREJ]=1;
      validFlags[PRJCT_RESULTS_ASCII_COOLING_STATUS]=1;
      validFlags[PRJCT_RESULTS_ASCII_MIRROR_ERROR]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDAEGG :
     {
      validFlags[PRJCT_RESULTS_ASCII_NAME]=1;
      validFlags[PRJCT_RESULTS_ASCII_VIEW_ELEVATION]=1;                         // absent for Harestua, present for OHP MAX-DOAS
      validFlags[PRJCT_RESULTS_ASCII_VIEW_AZIMUTH]=1;                           // absent for Harestua, present for OHP MAX-DOAS
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
      validFlags[PRJCT_RESULTS_ASCII_NREJ]=1;
      validFlags[PRJCT_RESULTS_ASCII_TDET]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDAEGG_OLD :                                        // Before spring 94, no viewing angles
     {
      validFlags[PRJCT_RESULTS_ASCII_NAME]=1;
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
      validFlags[PRJCT_RESULTS_ASCII_NREJ]=1;
      validFlags[PRJCT_RESULTS_ASCII_TDET]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_VIS :                                          // SAOZ PCD/NMOS UV-Visible
     {
      validFlags[PRJCT_RESULTS_ASCII_NAME]=1;
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
      validFlags[PRJCT_RESULTS_ASCII_NREJ]=1;
      validFlags[PRJCT_RESULTS_ASCII_BESTSHIFT]=1;
      validFlags[PRJCT_RESULTS_ASCII_TDET]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_BIRA_AIRBORNE :
     {
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
      validFlags[PRJCT_RESULTS_ASCII_NREJ]=1;
      validFlags[PRJCT_RESULTS_ASCII_LONGIT]=1;
      validFlags[PRJCT_RESULTS_ASCII_LATIT]=1;
      validFlags[PRJCT_RESULTS_ASCII_ALTIT]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SAOZ_EFM :
    case PRJCT_INSTR_FORMAT_RASAS :
     {
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
      validFlags[PRJCT_RESULTS_ASCII_NREJ]=1;
      validFlags[PRJCT_RESULTS_ASCII_TDET]=1;
      validFlags[PRJCT_RESULTS_ASCII_LONGIT]=1;
      validFlags[PRJCT_RESULTS_ASCII_LATIT]=1;
      validFlags[PRJCT_RESULTS_ASCII_ALTIT]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_CCD_EEV :
     {
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
      validFlags[PRJCT_RESULTS_ASCII_NREJ]=1;
      validFlags[PRJCT_RESULTS_ASCII_TDET]=1;
      validFlags[PRJCT_RESULTS_ASCII_VIEW_ELEVATION]=1;
      validFlags[PRJCT_RESULTS_ASCII_VIEW_AZIMUTH]=1;                           // not present in all measurements, but could be in the next future
      validFlags[PRJCT_RESULTS_ASCII_SCANNING]=1;
      validFlags[PRJCT_RESULTS_ASCII_FILTERNUMBER]=1;
      validFlags[PRJCT_RESULTS_ASCII_MEASTYPE]=1;
      validFlags[PRJCT_RESULTS_ASCII_CCD_HEADTEMPERATURE]=1;
      validFlags[PRJCT_RESULTS_ASCII_STARTTIME]=1;
      validFlags[PRJCT_RESULTS_ASCII_ENDTIME]=1;
      validFlags[PRJCT_RESULTS_ASCII_COMPASS]=1;
      validFlags[PRJCT_RESULTS_ASCII_PITCH]=1;
      validFlags[PRJCT_RESULTS_ASCII_ROLL]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_PDASI_EASOE :
     {
      validFlags[PRJCT_RESULTS_ASCII_NAME]=1;
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
      validFlags[PRJCT_RESULTS_ASCII_NREJ]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_UOFT :
     {
      validFlags[PRJCT_RESULTS_ASCII_NAME]=1;
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
      validFlags[PRJCT_RESULTS_ASCII_TDET]=1;
      validFlags[PRJCT_RESULTS_ASCII_STARTDATE]=1;
      validFlags[PRJCT_RESULTS_ASCII_ENDDATE]=1;
      validFlags[PRJCT_RESULTS_ASCII_STARTTIME]=1;
      validFlags[PRJCT_RESULTS_ASCII_ENDTIME]=1;
      validFlags[PRJCT_RESULTS_ASCII_VIEW_ELEVATION]=1;
      validFlags[PRJCT_RESULTS_ASCII_VIEW_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_ASCII_LONGIT]=1;
      validFlags[PRJCT_RESULTS_ASCII_LATIT]=1;
      validFlags[PRJCT_RESULTS_ASCII_FILTERNUMBER]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_NOAA :
     {
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
      validFlags[PRJCT_RESULTS_ASCII_TDET]=1;
      validFlags[PRJCT_RESULTS_ASCII_VIEW_ELEVATION]=1;
      validFlags[PRJCT_RESULTS_ASCII_VIEW_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_ASCII_LONGIT]=1;
      validFlags[PRJCT_RESULTS_ASCII_LATIT]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MFC :
    case PRJCT_INSTR_FORMAT_MFC_STD :
    case PRJCT_INSTR_FORMAT_MFC_BIRA :
     {
     	validFlags[PRJCT_RESULTS_ASCII_NAME]=1;
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
      validFlags[PRJCT_RESULTS_ASCII_VIEW_ELEVATION]=1;
      validFlags[PRJCT_RESULTS_ASCII_VIEW_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_ASCII_LONGIT]=1;
      validFlags[PRJCT_RESULTS_ASCII_LATIT]=1;
      validFlags[PRJCT_RESULTS_ASCII_STARTTIME]=1;
      validFlags[PRJCT_RESULTS_ASCII_ENDTIME]=1;
      validFlags[PRJCT_RESULTS_ASCII_TDET]=1;

      if (instrument==PRJCT_INSTR_FORMAT_MFC_BIRA);
       validFlags[PRJCT_RESULTS_ASCII_MEASTYPE]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GDP_ASCII :
     {
      validFlags[PRJCT_RESULTS_ASCII_PIXEL]=1;
      validFlags[PRJCT_RESULTS_ASCII_PIXEL_TYPE]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GDP_BIN :
     {
      validFlags[PRJCT_RESULTS_ASCII_LOS_ZA]=1;
      validFlags[PRJCT_RESULTS_ASCII_LOS_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_ASCII_PIXEL]=1;
      validFlags[PRJCT_RESULTS_ASCII_PIXEL_TYPE]=1;
      validFlags[PRJCT_RESULTS_ASCII_TINT]=0;
      validFlags[PRJCT_RESULTS_ASCII_CLOUD]=1;
      validFlags[PRJCT_RESULTS_ASCII_CLOUDTOPP]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_SCIA_PDS :
     {
      validFlags[PRJCT_RESULTS_ASCII_LOS_ZA]=1;
      validFlags[PRJCT_RESULTS_ASCII_LOS_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_ASCII_SCIA_STATE_INDEX]=1;
      validFlags[PRJCT_RESULTS_ASCII_SCIA_STATE_ID]=1;
      validFlags[PRJCT_RESULTS_ASCII_SCIA_QUALITY]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_GOME2 :
     {
      validFlags[PRJCT_RESULTS_ASCII_LOS_ZA]=1;
      validFlags[PRJCT_RESULTS_ASCII_LOS_AZIMUTH]=1;
      validFlags[PRJCT_RESULTS_ASCII_CLOUD]=1;
      validFlags[PRJCT_RESULTS_ASCII_CLOUDTOPP]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_MKZY :
     {
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
      validFlags[PRJCT_RESULTS_ASCII_NREJ]=1;
      validFlags[PRJCT_RESULTS_ASCII_NAME]=1;
      validFlags[PRJCT_RESULTS_ASCII_SCANNING]=1;
      validFlags[PRJCT_RESULTS_ASCII_STARTTIME]=1;
      validFlags[PRJCT_RESULTS_ASCII_ENDTIME]=1;
      validFlags[PRJCT_RESULTS_ASCII_LONGIT]=1;
      validFlags[PRJCT_RESULTS_ASCII_LATIT]=1;
      validFlags[PRJCT_RESULTS_ASCII_ALTIT]=1;
      validFlags[PRJCT_RESULTS_ASCII_TDET]=1;
     }
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_FORMAT_OCEAN_OPTICS :
     {
      validFlags[PRJCT_RESULTS_ASCII_SCANS]=1;
     }
    break;
 // ----------------------------------------------------------------------------

// QDOAS !!! STILL TO DO  PRJCT_INSTR_FORMAT_CCD_OHP_96,                        // CCD (OHP 96)
// QDOAS !!! STILL TO DO  PRJCT_INSTR_FORMAT_CCD_HA_94,                         // CCD (HARESTUA 94)
// QDOAS !!! STILL TO DO  PRJCT_INSTR_FORMAT_OMI,                               // OMI

    default:
     {
      // temp - all valid .... TODO
      for (int i=0; i<PRJCT_RESULTS_ASCII_MAX; ++i) validFlags[i] = 1;
     }
    break;
 // ----------------------------------------------------------------------------
   }

// GOME 1st versions when Level-2 data were merged into Level-1 data  PRJCT_RESULTS_ASCII_COVAR,
// GOME 1st versions when Level-2 data were merged into Level-1 data  PRJCT_RESULTS_ASCII_CORR,
// GOME 1st versions when Level-2 data were merged into Level-1 data  PRJCT_RESULTS_ASCII_CLOUD,
// GOME 1st versions when Level-2 data were merged into Level-1 data  PRJCT_RESULTS_ASCII_COEFF,
// GOME 1st versions when Level-2 data were merged into Level-1 data  PRJCT_RESULTS_ASCII_O3,
// GOME 1st versions when Level-2 data were merged into Level-1 data  PRJCT_RESULTS_ASCII_NO2,
 }

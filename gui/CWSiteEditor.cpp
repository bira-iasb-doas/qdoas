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

#include <cstdio> // TODO

#include <QGridLayout>
#include <QDoubleValidator>
#include <QLabel>

#include "CWSiteEditor.h"
#include "CWorkSpace.h"

#include "debugutil.h"

const unsigned int  BIT_EDIT_EXISTING_ITEM  = 0x10;
const unsigned int  BIT_LONGITUDE           = 0x01;
const unsigned int  BIT_LATITUDE            = 0x02;
const unsigned int  BIT_ALTITUDE            = 0x04;
const unsigned int  VALID_BITS              = 0x07;

CWSiteEditor::CWSiteEditor(QTreeWidgetItem *editItem, QWidget *parent) :
  CWEditor(parent),
  m_validBits(0x0)
{
  QGridLayout *mainLayout = new QGridLayout(this);

  // first create and layout the widget
  // row 0 - Name
  mainLayout->addWidget(new QLabel("Observation Site Name", this), 0, 0);
  m_siteName = new QLineEdit(this);
  mainLayout->addWidget(m_siteName, 0, 1);

  // row 1 - Abbrev.
  mainLayout->addWidget(new QLabel("Abbreviation", this), 1, 0);
  m_abbreviation = new QLineEdit(this);
  mainLayout->addWidget(m_abbreviation, 1, 1);

  // row 2 - Long.
  mainLayout->addWidget(new QLabel("Longitude in degrees (positive eastward)", this), 2, 0);
  QLineEdit *longEdit = new QLineEdit(this);
  longEdit->setValidator(new QDoubleValidator(-180.0, +180.0, 3, longEdit));
  connect(longEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotLongitudeChanged(const QString &)));
  mainLayout->addWidget(longEdit, 2, 1);

  // row 3 - Lat.
  mainLayout->addWidget(new QLabel("Latitude in degrees (positive northward)", this), 3, 0);
  QLineEdit *latEdit = new QLineEdit(this);
  latEdit->setValidator(new QDoubleValidator(-90.0, +90.0, 3, latEdit));
  connect(latEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotLatitudeChanged(const QString &)));
  mainLayout->addWidget(latEdit, 3, 1);

  // row 4 - Alt.
  mainLayout->addWidget(new QLabel("Altitude in meters", this), 4, 0);
  QLineEdit *altEdit = new QLineEdit(this);
  altEdit->setValidator(new QDoubleValidator(-50.0, 9000.0, 3, altEdit));
  connect(altEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotAltitudeChanged(const QString &)));
  mainLayout->addWidget(altEdit, 4, 1);

  // if the editItem is NULL, then this is for a NEW item ...
  if (editItem) {
    m_validBits |= BIT_EDIT_EXISTING_ITEM;

    // shift editItem to the site node
    while (editItem->parent() != NULL) {
      editItem = editItem->parent();
    }
    QString obsSiteStr = editItem->text(0);

    m_siteName->setText(obsSiteStr);
    m_siteName->setReadOnly(true); // cant change the site name ...

    // Update the caption and create a context tag 
    m_captionStr = "Modifying properties of observation site ";
    m_captionStr += obsSiteStr;
    m_contextTag = obsSiteStr;
    m_contextTag += "-SiteEditor";

    // extract the current information from the child items (must be 4 of them)
    // NOTE: set setText calls to the line edits with numeric data will result in
    // the corresponding slot being called to check the data and set m_validBits.

    assert(editItem->childCount() == 4);
      
    QTreeWidgetItem *child;
    
    // Abbreviation
    child = editItem->child(0);
    m_abbreviation->setText(child->text(1));
    // Longitude
    child = editItem->child(1);
    longEdit->setText(child->text(1));
    // Latitude
    child = editItem->child(2);
    latEdit->setText(child->text(1));
    // Altitude
    child = editItem->child(3);
    altEdit->setText(child->text(1));

  }
  else {
    // Update the caption and create a context tag 
    m_captionStr = "Create new observation site";
    m_contextTag = "SiteEditor"; // only ever want one of these active at once

    // set some defaults

    // Longitude
    longEdit->setText("0.000");
    // Latitude
    latEdit->setText("0.000");
    // Altitude
    altEdit->setText("0.000");
  }

}

CWSiteEditor::~CWSiteEditor()
{
}

bool CWSiteEditor::actionOk(void)
{
  // should only be possible if all valid bits are set ...

  if (m_validBits & BIT_EDIT_EXISTING_ITEM) {
    // existing set
   CWorkSpace::instance()->modifySite(m_siteName->text(), m_abbreviation->text(), m_long, m_lat, m_alt);
  }
  else {
    // a new entry ...
    CWorkSpace::instance()->createSite(m_siteName->text(), m_abbreviation->text(), m_long, m_lat, m_alt);
  }

  return true;
}

void CWSiteEditor::actionHelp(void)
{
}

void CWSiteEditor::takeFocus(void)
{
  m_siteName->setFocus(Qt::OtherFocusReason);
}

void CWSiteEditor::slotLongitudeChanged(const QString &text)
{
  bool isOk;

  m_long = text.toDouble(&isOk);
  if (isOk)
    m_validBits |= BIT_LONGITUDE;
  else
    m_validBits &= ~BIT_LONGITUDE;

  notifyAcceptActionOk((m_validBits & VALID_BITS) == VALID_BITS);
}

void CWSiteEditor::slotLatitudeChanged(const QString &text)
{
  bool isOk;

  m_lat = text.toDouble(&isOk);
  if (isOk)
    m_validBits |= BIT_LATITUDE;
  else
    m_validBits &= ~BIT_LATITUDE;

  notifyAcceptActionOk((m_validBits & VALID_BITS) == VALID_BITS);
}

void CWSiteEditor::slotAltitudeChanged(const QString &text)
{
  bool isOk;

  m_alt = text.toDouble(&isOk);
  if (isOk)
    m_validBits |= BIT_ALTITUDE;
  else
    m_validBits &= ~BIT_ALTITUDE;

  notifyAcceptActionOk((m_validBits & VALID_BITS) == VALID_BITS);
}



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


#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>

#include "CWAboutDialog.h"

#include "debugutil.h"

CWAboutDialog::CWAboutDialog(QWidget *parent) :
  QDialog(parent, Qt::MSWindowsFixedSizeDialogHint|Qt::WindowTitleHint) //|Qt::WindowSystemMenuHint)
{
  setWindowTitle("About Qdoas");

  QPixmap logo(":/images/about_logo.png");

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSizeConstraint(QLayout::SetFixedSize);

  QLabel *logoLabel = new QLabel(this);
  logoLabel->setPixmap(logo);
  mainLayout->addWidget(logoLabel, 1, Qt::AlignCenter);

  mainLayout->addSpacing(10);

  QString blurb("Qdoas is copyright ");
  blurb.append(Qt::Key_copyright);
  blurb += " 2007\n\nBelgian Institute for Space Aeronomy (BIRA-IASB)\n"
    "Avenue Circulaire, 3\n1180     Uccle\nBelgium\n\nand\n\n"
    "Science [&] Technology BV,\nDelft, Netherlands.";

  QLabel *blurbLabel = new QLabel(blurb);
  blurbLabel->setAlignment(Qt::AlignCenter);

  mainLayout->addWidget(blurbLabel, 1, Qt::AlignCenter);

  mainLayout->addSpacing(10);

  QPushButton *okButton = new QPushButton(QString("Ok"));
  okButton->setDefault(true);
  
  mainLayout->addWidget(okButton, 0, Qt::AlignCenter);
  
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
}

CWAboutDialog::~CWAboutDialog()
{
}
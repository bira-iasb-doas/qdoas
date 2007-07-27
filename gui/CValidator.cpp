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

#include <QTextStream>

#include "CValidator.h"


CSzaValidator::CSzaValidator(QObject *obj) :
  QValidator(obj)
{
}

QValidator::State CSzaValidator::validate(QString &input, int &pos) const
{
  if (input.isEmpty()) return QValidator::Intermediate;

  bool ok;
  double v = input.toDouble(&ok);
  int len = input.length();

  if (ok) {
    if (v < 0.0 || v > 180.0) return QValidator::Invalid;
    if (v < 10.0 && len > 5) return QValidator::Invalid;
    if (v < 100.0 && len > 6)  return QValidator::Invalid;
    return (len > 7) ? QValidator::Invalid : QValidator::Acceptable;
  }

  return QValidator::Invalid;
}

void CSzaValidator::fixup(QString &input) const
{
  bool ok;
  double v = input.toDouble(&ok);

  if (ok) {
    if (v < 0.0) input = "0.0";
    else if (v > 180.0) input = "180.000";
    else {
      QTextStream tmpStream(&input);
      tmpStream.setRealNumberNotation(QTextStream::FixedNotation);
      tmpStream.setRealNumberPrecision(3); // 3 decimal places
      tmpStream << v;
    }
  }
  else
    input = "0.0"; // default
}



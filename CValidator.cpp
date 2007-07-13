
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
    else
      input.sprintf("%.3f", v); // max 3 decimal places
  }
  else
    input = "0.0"; // default
}



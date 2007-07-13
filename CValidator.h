#ifndef _CVALIDATOR_H_GUARD
#define _CVALIDATOR_H_GUARD

#include <QValidator>

// CSzaValidator : Allow '0.000 to 180.000' with 3 decimal precision

class CSzaValidator : public QValidator
{
 public:
  CSzaValidator(QObject *obj);

  virtual QValidator::State validate(QString &input, int &pos) const;
  virtual void fixup(QString &input) const;
};

#endif

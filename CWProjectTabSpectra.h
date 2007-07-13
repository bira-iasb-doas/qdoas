#ifndef _CWPROJECTTABSPECTRA_H_GUARD
#define _CWPROJECTTABSPECTRA_H_GUARD

#include <QFrame>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>

#include "mediate_project.h"

class CWGeolocation;

class CWProjectTabSpectra : public QFrame
{
 public:
  CWProjectTabSpectra(const mediate_project_spectra_t *properties, QWidget *parent = 0);
  virtual ~CWProjectTabSpectra();

  void apply(mediate_project_spectra_t *properties) const;

 private:
  QLineEdit *m_szaMinEdit, *m_szaMaxEdit, *m_szaDeltaEdit;
  QLineEdit *m_recordMinEdit, *m_recordMaxEdit;
  QCheckBox *m_reqSpectraCheck, *m_reqDataCheck, *m_reqFitsCheck, *m_useNameCheck, *m_useDarkCheck;
  CWGeolocation *m_geolocationEdit;
};

class CWGeolocation : public QFrame
{
Q_OBJECT
 public:
  CWGeolocation(const union geolocation *geo, QWidget *parent = 0);
  virtual ~CWGeolocation();

  void apply(union geolocation *geo) const;

  public slots:
    void slotModeChanged(int);

 private:
  QFrame *m_circleFrame, *m_rectangleFrame;
  QLineEdit *m_westEdit, *m_eastEdit, *m_southEdit, *m_northEdit;
  QLineEdit *m_cenLongEdit, *m_cenLatEdit, *m_radiusEdit;
  QComboBox *m_modeCombo;
};

#endif

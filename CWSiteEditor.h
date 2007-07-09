#ifndef _CWSITEEDITOR_H_GUARD
#define _CWSITEEDITOR_H_GUARD

#include <QLineEdit>

#include "CWSiteTree.h"
#include "CWEditor.h"

class CWSiteEditor : public CWEditor
{
Q_OBJECT
 public:
  CWSiteEditor(CWSiteTree *siteTree, QTreeWidgetItem *editItem = 0, QWidget *parent = 0);
  virtual ~CWSiteEditor();

  virtual bool actionOk();
  virtual void actionHelp();

 private:
  double m_long, m_lat, m_alt;
  unsigned int m_validBits;
  QLineEdit *m_siteName, *m_abbreviation;
  CWSiteTree *m_siteTree;

 public slots:
   void slotLongitudeChanged(const QString &text);
   void slotLatitudeChanged(const QString &text);
   void slotAltitudeChanged(const QString &text);

};

#endif

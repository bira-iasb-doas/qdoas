#ifndef _CWPROJECTPROPERTYEDITOR_H_GUARD
#define _CWPROJECTPROPERTYEDITOR_H_GUARD

#include <QLineEdit>
#include <QStringList>
#include <QTreeWidgetItem>

#include "CWEditor.h"
#include "CWProjectTabSpectra.h"

class CWProjectTree;

class CWProjectPropertyEditor : public CWEditor
{
Q_OBJECT
 public:
  CWProjectPropertyEditor(CWProjectTree *projectTree, QTreeWidgetItem *item,
                          QWidget *parent = 0);
  virtual ~CWProjectPropertyEditor();

  virtual bool actionOk();
  virtual void actionHelp();

 private:
  QTabWidget *m_tabs;
  CWProjectTabSpectra *m_spectraTab;

  CWProjectTree *m_projectTree;
  QString m_projectName;
};

#endif

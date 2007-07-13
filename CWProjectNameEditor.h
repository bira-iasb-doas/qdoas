#ifndef _CWPROJECTNAMEEDITOR_H_GUARD
#define _CWPROJECTNAMEEDITOR_H_GUARD

#include <QLineEdit>
#include <QStringList>
#include <QTreeWidgetItem>

#include "CWEditor.h"

class CWProjectTree;

class CWProjectNameEditor : public CWEditor
{
Q_OBJECT
 public:
  CWProjectNameEditor(CWProjectTree *projectTree, QTreeWidgetItem *item = 0,
                      QWidget *parent = 0);
  virtual ~CWProjectNameEditor();

  virtual bool actionOk(void);
  virtual void actionHelp(void);

  virtual void takeFocus(void);

 public slots:
  void slotNameChanged(const QString &text);
  void slotReturnPressed();

 private:
  QLineEdit *m_projectName;
  CWProjectTree *m_projectTree;
  QString m_oldProjectName;
};

#endif

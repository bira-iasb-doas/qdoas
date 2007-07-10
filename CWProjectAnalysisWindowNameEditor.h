#ifndef _CWPROJECTANALYSISWINDOWNAMEEDITOR_H_GUARD
#define _CWPROJECTANALYSISWINDOWNAMEEDITOR_H_GUARD

#include <QLineEdit>
#include <QStringList>
#include <QTreeWidgetItem>

#include "CWEditor.h"

class CWProjectTree;

class CWProjectAnalysisWindowNameEditor : public CWEditor
{
Q_OBJECT
 public:
  CWProjectAnalysisWindowNameEditor(CWProjectTree *projectTree, QTreeWidgetItem *item, bool newAnalysisWindow,
                                    QWidget *parent = 0);
  virtual ~CWProjectAnalysisWindowNameEditor();

  virtual bool actionOk();
  virtual void actionHelp();

 public slots:
  void slotNameChanged(const QString &text);

 private:
  QLineEdit *m_analysisWindowName;
  CWProjectTree *m_projectTree;
  QStringList m_path;
  bool m_newAnalysisWindow;
};

#endif

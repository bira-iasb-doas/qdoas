#ifndef _CWEDITOR_H_GUARD
#define _CWEDITOR_H_GUARD

#include <QFrame>
#include <QString>

class CWEditor : public QFrame
{
Q_OBJECT
 public:
  CWEditor(QWidget *parent = 0);
  virtual ~CWEditor();

  const QString& editCaption(void) const;
  const QString& editContextTag(void) const;

  virtual void actionCancel(void);
  virtual bool actionOk(void);
  virtual void actionHelp(void);

  virtual void takeFocus(void);

  bool isAcceptActionOk(void) const;

 protected:
  void notifyAcceptActionOk(bool canDoOk);
  void shortcutActionOk(void);

 protected:
  QString m_captionStr, m_contextTag;

 private:
  bool m_lastNotification;

 signals:
  void signalAcceptOk(bool canDoOk);
  void signalShortcutActionOk();

};

inline bool CWEditor::isAcceptActionOk(void) const { return m_lastNotification; }

#endif

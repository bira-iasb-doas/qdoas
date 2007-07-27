#ifndef _CWSPLITTER_H_GUARD
#define _CWSPLITTER_H_GUARD

#include <map>

#include <QSplitter>

// Extends QSplitter to allow control of the first splitter position
// via signal-slot mechanism

class CWSplitter : public QSplitter
{
Q_OBJECT
 public:
  CWSplitter(Qt::Orientation orientation, QWidget *parent = 0);
  virtual ~CWSplitter();

  public slots:
    void slotSetWidthMode(int mode);

 private:
  std::map<int,int> m_modeToSizeMap;
  int m_currentMode;
};

#endif

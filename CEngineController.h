#ifndef _CENGINECONTROLLER_H_GUARD
#define _CENGINECONTROLLER_H_GUARD

#include <QObject>
#include <QList>
#include <QFileInfo>

#include "CEngineThread.h"

#include "CSession.h"
#include "RefCountPtr.h"

#include "mediate_project.h"

// CEngineController is a mediator. It is the interface for the GUI
// to control the activities of the engine, and to access any data
// provided by the engine.
//
//

class CEngineController : public QObject
{
Q_OBJECT
 public:
  CEngineController(QObject *parent = 0);
  virtual ~CEngineController();

  // establishes spectra browsing and returns.
  bool browseSpectra(const mediate_project_t *project,
		     QList<QFileInfo> &fileList);
  
  // notify interface is for use by response classes

  void notifySetProject(void);

  void notifyNumbeOfFiles(int nFiles);
  void notifyCurrentFile(int fileNumber);
  void notifyNumberOfRecords(int nRec);
  void notifyCurrentRecord(int recNumber);


 protected:
  virtual bool event(QEvent *e);

 public slots:
  // toolbar file navigation interface
  void slotFirstFile();
  void slotNextFile();
  void slotPreviousFile();
  void slotGotoFile(int number);
  // toolbar record navigation interface
  void slotFirstRecord();
  void slotNextRecord();
  void slotGotoRecord(int recNumber);
 
  void slotStartBrowseSession(RefCountedPtr<CSession> session);

 private:
  CEngineThread *m_thread;
  QList<QFileInfo> m_fileList;

  int m_mode;

  int m_currentRecord, m_currentFile;

  RefCountedPtr<CSession> m_session;
};

#endif

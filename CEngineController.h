#ifndef _CENGINECONTROLLER_H_GUARD
#define _CENGINECONTROLLER_H_GUARD

#include <QObject>
#include <QList>
#include <QFileInfo>

#include "CEngineThread.h"

#include "CPlotDataSet.h"
#include "CPlotPageData.h"
#include "CTablePageData.h"
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

  // notify interface is for use by response classes

  void notifySetProject(void);

  void notifyNumberOfFiles(int nFiles);
  void notifyCurrentFile(int fileNumber);
  void notifyReadyToNavigateRecords(const QString &filename, int numberOfRecords);
  void notifyCurrentRecord(int recordNumber);
  void notifyEndOfRecords(void);
  void notifyGotoRecord(int recordNumber);
  void notifyPlotData(QList<SPlotData> &plotDataList, QList<STitleTag> &titleList);
  void notifyTableData(QList<SCell> &cellList);

 protected:
  virtual bool event(QEvent *e);

 public slots:
  // toolbar file navigation interface
  void slotFirstFile();
  void slotNextFile();
  void slotPreviousFile();
  void slotLastFile();
  void slotGotoFile(int number);
  // toolbar record navigation interface
  void slotFirstRecord();
  void slotPreviousRecord();
  void slotNextRecord();
  void slotLastRecord();
  void slotGotoRecord(int recNumber);
 
  void slotStartBrowseSession(const RefCountPtr<CSession> &session);

 signals:
  void signalNumberOfFilesChanged(int nFiles);
  void signalCurrentFileChanged(int fileNumber);
  void signalCurrentFileChanged(const QString &filename);
  void signalNumberOfRecordsChanged(int nRecords);
  void signalCurrentRecordChanged(int recordNumber);

  void signalPlotPages(const QList< RefCountConstPtr<CPlotPageData> > &pageList);
  void signalTablePages(const QList< RefCountConstPtr<CTablePageData> > &pageList);

 private:
  CEngineThread *m_thread;
  QList<QFileInfo> m_fileList;

  int m_mode;

  const mediate_project_t *m_currentProject;
  int m_currentRecord, m_numberOfRecords, m_numberOfFiles;

  RefCountConstPtr<CSession> m_session;
  CSessionIterator m_currentIt;
};

#endif

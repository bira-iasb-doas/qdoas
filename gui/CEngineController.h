/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef _CENGINECONTROLLER_H_GUARD
#define _CENGINECONTROLLER_H_GUARD

#include <QObject>
#include <QList>
#include <QFileInfo>

#include "CEngineThread.h"
#include "CEngineError.h"

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

  void notifyErrorMessages(int highestErrorLevel, const QList<CEngineError> &errorMessages);

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

  void signalErrorMessages(int highestErrorLevel, const QString &messages);

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

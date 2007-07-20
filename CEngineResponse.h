#ifndef _CENGINERESPONSE_H_GUARD
#define _CENGINERESPONSE_H_GUARD

#include <QList>
#include <QString>

#include "CPlotDataSet.h"

class CEngineController;

const int cEngineResponseSetProjectType           = 1;
const int cEngineResponseBeginBrowseFileType      = 2;
const int cEngineResponseBrowseRecordType         = 3;
const int cEngineResponseGotoRecordType           = 4;

//------------------------------------------------------------

class CEngineResponse
{
 public:
  CEngineResponse(int type);
  virtual ~CEngineResponse();

  int type(void) const;
  void addErrorMessage(const QString &msg);
  
  virtual void process(CEngineController *engineController) = 0;

 protected:
  int m_type;
  QList<QString> m_errorMessages;
};

inline int CEngineResponse::type(void) const { return m_type; }

//------------------------------------------------------------

class CEngineResponseSetProject : public CEngineResponse
{
 public:
  CEngineResponseSetProject();
  virtual ~CEngineResponseSetProject();

  virtual void process(CEngineController *engineController);
};

//------------------------------------------------------------

class CEngineResponseBeginBrowseFile : public CEngineResponse
{
 public:
  CEngineResponseBeginBrowseFile(const QString &fileName);
  virtual ~CEngineResponseBeginBrowseFile();

  virtual void process(CEngineController *engineController);

  void setNumberOfRecords(int numberOfRecords);

 private:
  QString m_fileName;
  int m_numberOfRecords;
};

//------------------------------------------------------------

class CEngineResponseBrowseRecord : public CEngineResponse
{
 public:
  CEngineResponseBrowseRecord();
  virtual ~CEngineResponseBrowseRecord();

  virtual void process(CEngineController *engineController);

  void setRecordNumber(int recordNumber);
  void addDataSet(int pageNumber, const CPlotDataSet *dataSet);

 private:
  int m_recordNumber;
  QList<SPlotDataBucket> m_plotDataBuckets;
};

//------------------------------------------------------------

#endif


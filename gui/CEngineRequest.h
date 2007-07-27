#ifndef _CENGINEREQUEST_H_GUARD
#define _CENGINEREQUEST_H_GUARD

#include <QString>
#include <QList>

#include "mediate_project.h"

class CEngineThread;

const int cEngineRequestCompoundType             = 1000;
const int cEngineRequestSetProjectType           = 1001;
const int cEngineRequestBeginBrowseFileType      = 1002;
const int cEngineRequestBrowseNextRecordType     = 1003;
const int cEngineRequestBrowseSpecificRecordType = 1004;
const int cEngineRequestEndBrowseFileType        = 1005;

//------------------------------------------------------------

class CEngineRequest
{
 public:
  CEngineRequest(int type);
  virtual ~CEngineRequest();

  virtual bool process(CEngineThread *engineThread) = 0;

  int type(void) const;

 protected:
  int m_type;
};

inline int CEngineRequest::type(void) const { return m_type; }

//------------------------------------------------------------

class CEngineRequestCompound : public CEngineRequest
{
 public:
  CEngineRequestCompound();
  virtual ~CEngineRequestCompound();

  virtual bool process(CEngineThread *engineThread);

  void addRequest(CEngineRequest *req);

 private:
  QList<CEngineRequest*> m_requestList;
};

//------------------------------------------------------------

class CEngineRequestSetProject : public CEngineRequest
{
 public:
  CEngineRequestSetProject(const mediate_project_t *project);
  virtual ~CEngineRequestSetProject();

  virtual bool process(CEngineThread *engineThread);

  mediate_project_t m_project;
};

//------------------------------------------------------------

class CEngineRequestBeginBrowseFile : public CEngineRequest
{
 public:
  CEngineRequestBeginBrowseFile(const QString &fileName);
  virtual ~CEngineRequestBeginBrowseFile();

  virtual bool process(CEngineThread *engineThread);

 private:
  QString m_fileName;
};

//------------------------------------------------------------

class CEngineRequestBrowseNextRecord : public CEngineRequest
{
 public:
  CEngineRequestBrowseNextRecord();
  virtual ~CEngineRequestBrowseNextRecord();

  virtual bool process(CEngineThread *engineThread);
};

//------------------------------------------------------------

class CEngineRequestBrowseSpecificRecord : public CEngineRequest
{
 public:
  CEngineRequestBrowseSpecificRecord(int recordNumber);
  virtual ~CEngineRequestBrowseSpecificRecord();

  virtual bool process(CEngineThread *engineThread);

 private:
  int m_recordNumber;
};

//------------------------------------------------------------

class CEngineRequestEndBrowseFile : public CEngineRequest
{
 public:
  CEngineRequestEndBrowseFile();
  virtual ~CEngineRequestEndBrowseFile();

  virtual bool process(CEngineThread *engineThread);

};

//------------------------------------------------------------

#endif

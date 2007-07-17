#ifndef _CENGINEREQUEST_H_GUARD
#define _CENGINEREQUEST_H_GUARD

#include <QString>

#include "mediate_project.h"

class CEngineThread;

const int cEngineRequestSetProjectType           = 1001;
const int cEngineRequestBeginBrowseFileType      = 1002;

//------------------------------------------------------------

class CEngineRequest
{
 public:
  CEngineRequest(int type);
  virtual ~CEngineRequest();

  virtual void process(CEngineThread *engineThread) = 0;

  int type(void) const;

 protected:
  int m_type;
};

inline int CEngineRequest::type(void) const { return m_type; }

//------------------------------------------------------------

class CEngineRequestSetProject : public CEngineRequest
{
 public:
  CEngineRequestSetProject(const mediate_project_t *project);
  virtual ~CEngineRequestSetProject();

  virtual void process(CEngineThread *engineThread);

  mediate_project_t m_project;
};

//------------------------------------------------------------

class CEngineRequestBeginBrowseFile : public CEngineRequest
{
 public:
  CEngineRequestBeginBrowseFile(const QString &fileName);
  virtual ~CEngineRequestBeginBrowseFile();

  virtual void process(CEngineThread *engineThread);

 private:
  QString m_fileName;
};

//------------------------------------------------------------

#endif

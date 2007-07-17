#ifndef _CENGINERESPONSE_H_GUARD
#define _CENGINERESPONSE_H_GUARD

#include <QList>
#include <QString>

class CEngineController;

const int cEngineResponseSetProjectType           = 1;
const int cEngineResponseBeginBrowseFileType      = 2;

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

#endif


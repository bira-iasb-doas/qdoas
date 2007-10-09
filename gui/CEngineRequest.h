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

#ifndef _CENGINEREQUEST_H_GUARD
#define _CENGINEREQUEST_H_GUARD

#include <QString>
#include <QList>

#include "mediate_request.h"

class CEngineThread;

const int cEngineRequestCompoundType                 = 1000;
const int cEngineRequestSetSymbolsType               = 1001;
const int cEngineRequestSetSitesType                 = 1002;
const int cEngineRequestSetProjectType               = 1003;
const int cEngineRequestBeginBrowseFileType          = 1004;
const int cEngineRequestBrowseNextRecordType         = 1005;
const int cEngineRequestBrowseSpecificRecordType     = 1006;
const int cEngineRequestEndBrowseFileType            = 1007;
const int cEngineRequestBeginAnalyseFileType         = 1008;
const int cEngineRequestAnalyseNextRecordType        = 1009;
const int cEngineRequestAnalyseSpecificRecordType    = 1010;
const int cEngineRequestEndAnalyseFileType           = 1011;
const int cEngineRequestBeginCalibrateFileType       = 1012;
const int cEngineRequestCalibrateNextRecordType      = 1013;
const int cEngineRequestCalibrateSpecificRecordType  = 1014;
const int cEngineRequestEndCalibrateFileType         = 1015;

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

 private:
  mediate_project_t m_project;
};

//------------------------------------------------------------

class CEngineRequestSetSymbols : public CEngineRequest
{
 public:
  CEngineRequestSetSymbols(mediate_symbol_t *symbolList, int nSymbols);
  virtual ~CEngineRequestSetSymbols();

  virtual bool process(CEngineThread *engineThread);

 private:
  mediate_symbol_t *m_symbolList;
  int m_nSymbols;
};

//------------------------------------------------------------

class CEngineRequestSetSites : public CEngineRequest
{
 public:
  CEngineRequestSetSites(mediate_site_t *siteList, int nSites);
  virtual ~CEngineRequestSetSites();

  virtual bool process(CEngineThread *engineThread);

 private:
  mediate_site_t *m_siteList;
  int m_nSites;
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

class CEngineRequestBeginAnalyseFile : public CEngineRequest
{
 public:
  CEngineRequestBeginAnalyseFile(const QString &fileName);
  virtual ~CEngineRequestBeginAnalyseFile();

  virtual bool process(CEngineThread *engineThread);

 private:
  QString m_fileName;
};

//------------------------------------------------------------

class CEngineRequestAnalyseNextRecord : public CEngineRequest
{
 public:
  CEngineRequestAnalyseNextRecord();
  virtual ~CEngineRequestAnalyseNextRecord();

  virtual bool process(CEngineThread *engineThread);
};

//------------------------------------------------------------

class CEngineRequestAnalyseSpecificRecord : public CEngineRequest
{
 public:
  CEngineRequestAnalyseSpecificRecord(int recordNumber);
  virtual ~CEngineRequestAnalyseSpecificRecord();

  virtual bool process(CEngineThread *engineThread);

 private:
  int m_recordNumber;
};

//------------------------------------------------------------

class CEngineRequestEndAnalyseFile : public CEngineRequest
{
 public:
  CEngineRequestEndAnalyseFile();
  virtual ~CEngineRequestEndAnalyseFile();

  virtual bool process(CEngineThread *engineThread);

};

//------------------------------------------------------------

class CEngineRequestBeginCalibrateFile : public CEngineRequest
{
 public:
  CEngineRequestBeginCalibrateFile(const QString &fileName);
  virtual ~CEngineRequestBeginCalibrateFile();

  virtual bool process(CEngineThread *engineThread);

 private:
  QString m_fileName;
};

//------------------------------------------------------------

class CEngineRequestCalibrateNextRecord : public CEngineRequest
{
 public:
  CEngineRequestCalibrateNextRecord();
  virtual ~CEngineRequestCalibrateNextRecord();

  virtual bool process(CEngineThread *engineThread);
};

//------------------------------------------------------------

class CEngineRequestCalibrateSpecificRecord : public CEngineRequest
{
 public:
  CEngineRequestCalibrateSpecificRecord(int recordNumber);
  virtual ~CEngineRequestCalibrateSpecificRecord();

  virtual bool process(CEngineThread *engineThread);

 private:
  int m_recordNumber;
};

//------------------------------------------------------------

class CEngineRequestEndCalibrateFile : public CEngineRequest
{
 public:
  CEngineRequestEndCalibrateFile();
  virtual ~CEngineRequestEndCalibrateFile();

  virtual bool process(CEngineThread *engineThread);

};

//------------------------------------------------------------

#endif

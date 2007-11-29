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

//------------------------------------------------------------

class CEngineRequest
{
 public:
  enum RequestType {
    eEngineRequestCompoundType,
    eEngineRequestSetSymbolsType,
    eEngineRequestSetSitesType,
    eEngineRequestSetProjectType,
    eEngineRequestSetAnalysisWindowType,
    eEngineRequestBeginBrowseFileType,
    eEngineRequestBrowseNextRecordType,
    eEngineRequestBrowseSpecificRecordType,
    eEngineRequestEndBrowseFileType,
    eEngineRequestBeginAnalyseFileType,
    eEngineRequestAnalyseNextRecordType,
    eEngineRequestAnalyseSpecificRecordType,
    eEngineRequestEndAnalyseFileType,
    eEngineRequestBeginCalibrateFileType,
    eEngineRequestCalibrateNextRecordType,
    eEngineRequestCalibrateSpecificRecordType,
    eEngineRequestEndCalibrateFileType,
    eEngineRequestViewCrossSectionsType
  };

  CEngineRequest(enum RequestType type);
  virtual ~CEngineRequest();

  virtual bool process(CEngineThread *engineThread) = 0;

  enum RequestType type(void) const;

 protected:
  enum RequestType m_type;
};

inline CEngineRequest::RequestType CEngineRequest::type(void) const { return m_type; }

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
  CEngineRequestSetProject(const mediate_project_t *project, int opMode);
  virtual ~CEngineRequestSetProject();

  virtual bool process(CEngineThread *engineThread);

 private:
  mediate_project_t m_project;
  int m_opMode;
};

//------------------------------------------------------------

class CEngineRequestSetAnalysisWindows : public CEngineRequest
{
 public:
  CEngineRequestSetAnalysisWindows(const mediate_analysis_window_t *windowList, int nWindows,int opMode);
  virtual ~CEngineRequestSetAnalysisWindows();

  virtual bool process(CEngineThread *engineThread);

 private:
  mediate_analysis_window_t *m_windowList;
  int m_nWindows, m_opMode;
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

class CEngineRequestViewCrossSections : public CEngineRequest
{
 public:
  CEngineRequestViewCrossSections(char *awName,double minWavelength, double maxWavelength,
                                  int nFiles, char **filenames);
  virtual ~CEngineRequestViewCrossSections();

  virtual bool process(CEngineThread *engineThread);

 private:
  char *m_awName;
  double m_minWavelength, m_maxWavelength;
  int m_nFiles;
  char **m_filenames;

};

//------------------------------------------------------------

#endif

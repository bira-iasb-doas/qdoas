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

#ifndef _CENGINERESPONSE_H_GUARD
#define _CENGINERESPONSE_H_GUARD

#include <QList>
#include <QString>

#include "CPlotDataSet.h"
#include "CTablePageData.h"

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
  void addCell(int pageNumber, int row, int col, const QVariant &data);
  void addPageTitleAndTag(int page, const QString &title, const QString &tag);

 private:
  int m_recordNumber;
  QList<SPlotData> m_plotDataList;
  QList<SCell> m_cellList;
  QList<STitleTag> m_titleList;
};

//------------------------------------------------------------

#endif


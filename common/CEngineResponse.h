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
#include "CEngineError.h"
#include "mediate_types.h"

class CEngineController;

//------------------------------------------------------------

class CEngineResponse
{
 public:
  enum ResponseType {
    eEngineResponseMessageType,
    eEngineResponseBeginAccessFileType,
    eEngineResponseAccessRecordType,
    eEngineResponseGotoRecordType,
    eEngineResponseEndAccessFileType    
  };  

  CEngineResponse(enum ResponseType type);
  virtual ~CEngineResponse();

  enum ResponseType type(void) const;
  void addErrorMessage(const QString &tag, const QString &msg, int errorLevel);
  
  virtual void process(CEngineController *engineController) = 0;

  bool processErrors(CEngineController *engineController);

  bool hasErrors(void) const;
  bool hasFatalError(void) const;

 protected:
  enum ResponseType m_type;
  int m_highestErrorLevel;
  QList<CEngineError> m_errorMessages;
};

inline CEngineResponse::ResponseType CEngineResponse::type(void) const { return m_type; }
inline bool CEngineResponse::hasErrors(void) const { return !m_errorMessages.isEmpty(); }
inline bool CEngineResponse::hasFatalError(void) const { return (m_highestErrorLevel == FatalEngineError); }

//------------------------------------------------------------

class CEngineResponseMessage : public CEngineResponse
{
 public:
  CEngineResponseMessage();
  virtual ~CEngineResponseMessage();

  virtual void process(CEngineController *engineController);
};

//------------------------------------------------------------

class CEngineResponseBeginAccessFile : public CEngineResponse
{
 public:
  CEngineResponseBeginAccessFile(const QString &fileName);
  virtual ~CEngineResponseBeginAccessFile();

  virtual void process(CEngineController *engineController);

  void setNumberOfRecords(int numberOfRecords);

 private:
  QString m_fileName;
  int m_numberOfRecords;
};

//------------------------------------------------------------

class CEngineResponseSpecificRecord : public CEngineResponse
{
 public:
  CEngineResponseSpecificRecord(CEngineResponse::ResponseType type);
  virtual ~CEngineResponseSpecificRecord();

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

class CEngineResponseAccessRecord : public CEngineResponseSpecificRecord
{
 public:
  CEngineResponseAccessRecord();
  virtual ~CEngineResponseAccessRecord();
};

//------------------------------------------------------------

class CEngineResponseEndAccessFile : public CEngineResponse
{
 public:
  CEngineResponseEndAccessFile();
  virtual ~CEngineResponseEndAccessFile();

  virtual void process(CEngineController *engineController);
};

//------------------------------------------------------------



#endif

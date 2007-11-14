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


#ifndef _CCONFIGHANDLER_H_GUARD
#define _CCONFIGHANDLER_H_GUARD

#include <QXmlDefaultHandler>
#include <QList>
#include <QString>
#include <QVector>

class CConfigHandler;

class CConfigSubHandler
{
 public:
  CConfigSubHandler();
  virtual ~CConfigSubHandler() = 0;

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);
  virtual bool character(const QString &ch);
  virtual bool end(const QString &element);
  virtual bool end(void);

  virtual CConfigHandler* master(void) = 0;

 protected:
  bool postErrorMessage(const QString &msg); // always returns false
};

class CBasicConfigSubHandler : public CConfigSubHandler
{
 public:
  CBasicConfigSubHandler(CConfigHandler *master);
  virtual ~CBasicConfigSubHandler();

  virtual CConfigHandler* master(void);

 protected:
  CConfigHandler *m_master;
};


struct SSubHandlerItem
{
  CConfigSubHandler *handler;
  int depth;

  SSubHandlerItem(CConfigSubHandler *h, int d) : handler(h), depth(d) {}
};

class CConfigHandler : public QXmlDefaultHandler
{
 public:
  CConfigHandler();
  virtual ~CConfigHandler();

  // error handling
  virtual bool error(const QXmlParseException &exception);
  virtual bool warning(const QXmlParseException &exception);
  virtual bool fatalError(const QXmlParseException &exception);

  // content handling
  virtual bool characters(const QString &ch);
  virtual bool endDocument();
  virtual bool endElement(const QString &namespaceURI, const QString &localName,
			  const QString &qName);
  virtual QString errorString() const;
  virtual bool ignorableWhitespace(const QString &ch);
  virtual bool startDocument();
  virtual bool startElement(const QString &namespaceURI, const QString &localName,
			    const QString &qName, const QXmlAttributes &atts);

  bool installSubHandler(CConfigSubHandler *newHandler, const QXmlAttributes &atts);

  void setPath(int index, const QString &pathPrefix);
  QString getPath(int index) const;
  QString pathExpand(const QString &name);
 
  QString messages(void) const; // messages collected during parsing 

 protected:
  bool delegateStartElement(const QString &qName, const QXmlAttributes &atts, bool &result);

 private:
  friend class CConfigSubHandler;

  void setSubErrorMessage(const QString &msg);

 private:
  QList<QString> m_elementStack;
  CConfigSubHandler *m_activeSubHandler;
  QList<SSubHandlerItem> m_subHandlerStack;
  QVector<QString> m_paths;
  QString m_subErrorMessage;
  QString m_errorMessages;
  QString m_collatedStr;
};

inline void CConfigHandler::setSubErrorMessage(const QString &msg) { m_subErrorMessage = msg; }

#endif

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


#ifndef _CQDOASPROJECTCONFIGHANDLER_H_GUARD
#define _CQDOASPROJECTCONFIGHANDLER_H_GUARD

#include <QXmlDefaultHandler>
#include <QList>
#include <QString>
#include <QVector>

#include "CProjectConfigItem.h"

class CQdoasProjectConfigHandler;

class CConfigSubHandler
{
 public:
  CConfigSubHandler(CQdoasProjectConfigHandler *master);
  virtual ~CConfigSubHandler() = 0;

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);
  virtual bool character(const QString &ch);
  virtual bool end(const QString &element);
  virtual bool end(void);

 protected:
  bool postErrorMessage(const QString &msg); // always returns false

 protected:
  CQdoasProjectConfigHandler *m_master;
};


struct SSubHandlerItem
{
  CConfigSubHandler *handler;
  int depth;

  SSubHandlerItem(CConfigSubHandler *h, int d) : handler(h), depth(d) {}
};

class CQdoasProjectConfigHandler : public QXmlDefaultHandler
{
 public:
  CQdoasProjectConfigHandler();
  virtual ~CQdoasProjectConfigHandler();

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
 
  void addProjectItem(CProjectConfigItem *item); // takes ownership of item
  QList<const CProjectConfigItem*> projectItems(void) const; // items in returned list have the same lifetime as 'this'

  void addSiteItem(CSiteConfigItem *item); // takes ownership of item
  QList<const CSiteConfigItem*> siteItems(void) const; // items in returned list have the same lifetime as 'this'

  QString messages(void) const; // messages collected during parsing 

 private:
  friend class CConfigSubHandler;

  void setSubErrorMessage(const QString &msg);

 private:
  QList<QString> m_elementStack;
  CConfigSubHandler *m_activeSubHandler;
  QList<SSubHandlerItem> m_subHandlerStack;
  QString m_subErrorMessage;
  QString m_errorMessages;
  QString m_collatedStr;
  QList<const CProjectConfigItem*> m_projectItemList;
  QList<const CSiteConfigItem*> m_siteItemList;
  QVector<QString> m_paths;
};

inline void CQdoasProjectConfigHandler::setSubErrorMessage(const QString &msg) { m_subErrorMessage = msg; }

#endif

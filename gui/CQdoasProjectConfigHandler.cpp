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

#include <QTextStream>

#include "CQdoasProjectConfigHandler.h"
#include "CProjectConfigSubHandlers.h"

#include "debugutil.h"

CQdoasProjectConfigHandler::CQdoasProjectConfigHandler() :
  QXmlDefaultHandler(),
  m_activeSubHandler(NULL),
  m_paths(10)
{
}

CQdoasProjectConfigHandler::~CQdoasProjectConfigHandler()
{
  // delete any sub handlers ...
  while (!m_subHandlerStack.isEmpty()) {
    delete m_subHandlerStack.back().handler;
    m_subHandlerStack.pop_back();
  }
  
  while (!m_projectItemList.isEmpty()) {
    delete m_projectItemList.takeFirst();
  }

  while (!m_siteItemList.isEmpty()) {
    delete m_siteItemList.takeFirst();
  }

  while (!m_symbolList.isEmpty()) {
    delete m_symbolList.takeFirst();
  }
}

bool CQdoasProjectConfigHandler::error(const QXmlParseException &exception)
{
  QTextStream stream(&m_errorMessages);
  stream << "Error on line " << exception.lineNumber() << " : " << exception.message() << "\n";

  return true;
}

bool CQdoasProjectConfigHandler::warning(const QXmlParseException &exception)
{
  QTextStream stream(&m_errorMessages);
  stream << "Warning on line " << exception.lineNumber() << " : " << exception.message() << "\n";

  return true;
}

bool CQdoasProjectConfigHandler::fatalError(const QXmlParseException &exception)
{
  QTextStream stream(&m_errorMessages);
  stream << "Fatal Error on line " << exception.lineNumber() << " : " << exception.message() << "\n";

  return false;
}

QString CQdoasProjectConfigHandler::messages(void) const
{
  return m_errorMessages;
}

bool CQdoasProjectConfigHandler::characters(const QString &ch)
{
  // collects all character data into a single string. This is
  // passed to sub handlers IFF the trimmed result is not empty.

  if (m_activeSubHandler) {

    m_collatedStr += ch;
  }

  return true;
}

bool CQdoasProjectConfigHandler::endDocument()
{
  return true;
}

bool CQdoasProjectConfigHandler::endElement(const QString &namespaceURI,
					    const QString &localName,
					    const QString &qName)
{
  bool status = true;

  if (m_activeSubHandler) {
    // delegate to the sub handler

    // first any collected character data
    QString tmp(m_collatedStr.trimmed());
    if (!tmp.isEmpty()) {
      status = m_activeSubHandler->character(tmp);
    }

    if (status) {
      if (m_subHandlerStack.back().depth == m_elementStack.count()) {
	status = m_activeSubHandler->end();
	// done with this handler ... discard it
	delete m_activeSubHandler;
	m_subHandlerStack.pop_back();
	// revert back to the previous handler
	if (!m_subHandlerStack.isEmpty())
	  m_activeSubHandler = m_subHandlerStack.back().handler;
	else
	  m_activeSubHandler = NULL;
      }
      else {
	status = m_activeSubHandler->end(qName);
      }
    }
  }
  else {
    // no sub handler ...
  }

  m_elementStack.pop_back();

  return status;
}

QString CQdoasProjectConfigHandler::errorString() const
{
  return m_subErrorMessage;
}

bool CQdoasProjectConfigHandler::ignorableWhitespace(const QString &ch)
{
  return true;
} 

bool CQdoasProjectConfigHandler::startDocument()
{
  return true;
}

bool CQdoasProjectConfigHandler::startElement(const QString &namespaceURI,
					      const QString &localName,
					      const QString &qName,
					      const QXmlAttributes &atts)
{
  // always track the stack - also provides the depth
  m_elementStack.push_back(qName);

  if (m_activeSubHandler) {
    // prepare for collation of character data
    m_collatedStr.clear();

    // delegate to the sub handler
    return m_activeSubHandler->start(qName, atts);
  }
  else {
    // no sub handler yet ...

    if (qName == "project") {
      // new Project handler
      return installSubHandler(new CProjectSubHandler(this), atts);
    }
    else if (qName == "paths") {
      // new Path handler
      return installSubHandler(new CPathSubHandler(this), atts);
    }
    else if (qName == "sites") {
      // new Site handler
      return installSubHandler(new CSiteSubHandler(this), atts);
    }
    else if (qName == "symbols") {
      // new symbol handler
      return installSubHandler(new CSymbolSubHandler(this), atts);
    }
  }
  return true;
}

bool CQdoasProjectConfigHandler::installSubHandler(CConfigSubHandler *newHandler,
						   const QXmlAttributes &atts)
{
  m_subHandlerStack.push_back(SSubHandlerItem(newHandler, m_elementStack.count()));
  m_activeSubHandler = newHandler;
 
  return m_activeSubHandler->start(atts);
}

void CQdoasProjectConfigHandler::addProjectItem(CProjectConfigItem *item)
{
  m_projectItemList.push_back(item);
}

QList<const CProjectConfigItem*> CQdoasProjectConfigHandler::projectItems(void) const
{
  return m_projectItemList;
}

void CQdoasProjectConfigHandler::addSiteItem(CSiteConfigItem *item)
{
  m_siteItemList.push_back(item);
}

QList<const CSiteConfigItem*> CQdoasProjectConfigHandler::siteItems(void) const
{
  return m_siteItemList;
}

void CQdoasProjectConfigHandler::addSymbol(const QString &symbolName, const QString &symbolDescription)
{
  m_symbolList.push_back(new CSymbolConfigItem(symbolName, symbolDescription));
}

QList<const CSymbolConfigItem*> CQdoasProjectConfigHandler::symbolItems(void) const
{
  return m_symbolList;
}


void CQdoasProjectConfigHandler::setPath(int index, const QString &pathPrefix)
{
  // index MUST be in the range 0-9
  if (index < 0 || index > 9)
    return;

  // copy and remove any trailing '/' or '\' characters ...
  QString tmp(pathPrefix);

  while (!tmp.isEmpty() && (tmp.endsWith('/') || tmp.endsWith('\\')))
    tmp.chop(1);

  m_paths[index] = tmp;
}

QString CQdoasProjectConfigHandler::getPath(int index) const
{
  if (index < 0 || index > 9)
    return QString();

  return m_paths[index];
}

QString CQdoasProjectConfigHandler::pathExpand(const QString &name)
{
  // replace a '%?' prefix with a path (? must be a digit).

  int len = name.length();
  if (len > 1 && name.startsWith('%') && name.at(1).isDigit()) {

    int index = name.at(1).digitValue();

    assert(index >= 0 && index <= 9);

    QString tmp = m_paths.at(index);
    if (len > 2)
      tmp += name.right(len - 2);

    return tmp;
  }

  return name;
}

//------------------------------------------------------------------------

CConfigSubHandler::CConfigSubHandler(CQdoasProjectConfigHandler *master) :
  m_master(master)
{
}

CConfigSubHandler::~CConfigSubHandler()
{
}

bool CConfigSubHandler::start(const QXmlAttributes &atts)
{
  return true;
}

bool CConfigSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  return true;
}

bool CConfigSubHandler::character(const QString &ch)
{
  return true;
}

bool CConfigSubHandler::end(const QString &element)
{
  return true;
}

bool CConfigSubHandler::end()
{
  return true;
}

bool CConfigSubHandler::postErrorMessage(const QString &msg)
{
  m_master->setSubErrorMessage(msg);
  return false;
}


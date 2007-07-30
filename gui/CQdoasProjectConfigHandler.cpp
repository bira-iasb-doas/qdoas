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


#include "CQdoasProjectConfigHandler.h"
#include "CProjectConfigSubHandlers.h"

#include "debugutil.h"

CQdoasProjectConfigHandler::CQdoasProjectConfigHandler() :
  QXmlDefaultHandler(),
  m_activeSubHandler(NULL)
{
}

CQdoasProjectConfigHandler::~CQdoasProjectConfigHandler()
{
  // delete any sub handlers ...
  while (!m_subHandlerStack.isEmpty()) {
    delete m_subHandlerStack.back().handler;
    m_subHandlerStack.pop_back();
  }
}

bool CQdoasProjectConfigHandler::characters(const QString &ch)
{
  // only care about non whitespace and only in sub handlers

  if (m_activeSubHandler) {

    QString tmp(ch.trimmed());
  
    if (!tmp.isEmpty())
      return m_activeSubHandler->character(tmp);
  }

  return true;
}

bool CQdoasProjectConfigHandler::endDocument()
{
  TRACE2("CQdoasProjectConfigHandler::endDocument");
  return true;
}

bool CQdoasProjectConfigHandler::endElement(const QString &namespaceURI,
					    const QString &localName,
					    const QString &qName)
{
  bool status = true;

  if (m_activeSubHandler) {
    // delegate to the sub handler
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
  else {
    // no sub handler ...
  }

  m_elementStack.pop_back();

  return status;
}

QString CQdoasProjectConfigHandler::errorString() const
{
  return QString("Some error...");
}

bool CQdoasProjectConfigHandler::ignorableWhitespace(const QString &ch)
{
  return true;
} 

bool CQdoasProjectConfigHandler::startDocument()
{
  TRACE2("CQdoasProjectConfigHandler::startDocument");
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
    // delegate to the sub handler
    return m_activeSubHandler->start(qName, atts);
  }
  else {
    // no sub handler yet ...

    if (qName == "project") {
      // new Project handler
      return installSubHandler(new CProjectSubHandler(this), atts);
    }
    // else if (qName == "Site") ... TODO
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

bool CConfigSubHandler::character(const QString &ch)
{
  return true;
}

bool CConfigSubHandler::end()
{
  return true;
}


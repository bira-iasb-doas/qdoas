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


#ifndef _CPROJECTCONFIGANALYSISWINDOWSUBHANDLERS_H_GUARD
#define _CPROJECTCONFIGANALYSISWINDOWSUBHANDLERS_H_GUARD

#include "CQdoasProjectConfigHandler.h"


class CAnalysisWindowSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowSubHandler(CQdoasProjectConfigHandler *master,
			    CAnalysisWindowConfigItem *item);
  virtual ~CAnalysisWindowSubHandler();

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);

  static int mapToPolyType(const QString &str);

 private:
  CAnalysisWindowConfigItem *m_item; // does not own this item
};

class CAnalysisWindowCrossSectionSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowCrossSectionSubHandler(CQdoasProjectConfigHandler *master,
					mediate_analysis_window_t *d);
  virtual ~CAnalysisWindowCrossSectionSubHandler();

  virtual bool start(const QXmlAttributes &atts);

 private:
  mediate_analysis_window_t *m_d;
};

class CAnalysisWindowShiftStretchSubHandler : public CConfigSubHandler
{
 public:
  CAnalysisWindowShiftStretchSubHandler(CQdoasProjectConfigHandler *master,
					mediate_analysis_window_t *d);
  virtual ~CAnalysisWindowShiftStretchSubHandler();

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);
  virtual bool end(void);

 private:
  mediate_analysis_window_t *m_d;
};


#endif

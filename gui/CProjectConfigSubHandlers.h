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


#ifndef _CPROJECTCONFIGSUBHANDLERS_H_GUARD
#define _CPROJECTCONFIGSUBHANDLERS_H_GUARD

#include "CQdoasProjectConfigHandler.h"


//-------------------------------------------------------------------

class CProjectSubHandler : public CConfigSubHandler
{
 public:
  CProjectSubHandler(CQdoasProjectConfigHandler *master);
  virtual ~CProjectSubHandler();

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);
  virtual bool end(const QString &element);
  virtual bool end(void);

 private:
  CProjectConfigItem *m_project;
};

//-------------------------------------------------------------------

class CProjectSpectraSubHandler : public CConfigSubHandler
{
 public:
  CProjectSpectraSubHandler(CQdoasProjectConfigHandler *master,
			    mediate_project_spectra_t *spectra);
  virtual ~CProjectSpectraSubHandler();

  virtual bool start(const QString &element, const QXmlAttributes &atts);
  virtual bool end(const QString &element);

 private:
  mediate_project_spectra_t *m_spectra;
};

//-------------------------------------------------------------------

class CProjectAnalysisSubHandler : public CConfigSubHandler
{
 public:
  CProjectAnalysisSubHandler(CQdoasProjectConfigHandler *master,
			    mediate_project_analysis_t *analysis);
  virtual ~CProjectAnalysisSubHandler();

  virtual bool start(const QXmlAttributes &atts);

 private:
  mediate_project_analysis_t *m_analysis;
};


class CProjectRawSpectraSubHandler : public CConfigSubHandler
{
 public:
  CProjectRawSpectraSubHandler(CQdoasProjectConfigHandler *master,
			      CProjectConfigTreeNode *node);
  virtual ~CProjectRawSpectraSubHandler();

  virtual bool start(const QString &element, const QXmlAttributes &atts);

 private:
  CProjectConfigTreeNode *m_node;
};


#endif


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

#include "CConfigHandler.h"
#include "mediate_analysis_window.h"

class CAnalysisWindowConfigItem;

class CAnalysisWindowSubHandler : public CBasicConfigSubHandler
{
 public:
  CAnalysisWindowSubHandler(CConfigHandler *master,
			    CAnalysisWindowConfigItem *item);
  virtual ~CAnalysisWindowSubHandler();

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);

  static int mapToPolyType(const QString &str);

 private:
  CAnalysisWindowConfigItem *m_item; // does not own this item
};

class CAnalysisWindowCrossSectionSubHandler : public CBasicConfigSubHandler
{
 public:
  CAnalysisWindowCrossSectionSubHandler(CConfigHandler *master,
					cross_section_list_t *d);
  virtual ~CAnalysisWindowCrossSectionSubHandler();

  virtual bool start(const QXmlAttributes &atts);

 private:
  cross_section_list_t *m_d;
};

class CAnalysisWindowLinearSubHandler : public CBasicConfigSubHandler
{
 public:
  CAnalysisWindowLinearSubHandler(CConfigHandler *master,
				  struct anlyswin_linear *d);
  virtual ~CAnalysisWindowLinearSubHandler();

  virtual bool start(const QXmlAttributes &atts);

 private:
  struct anlyswin_linear *m_d;
};

class CAnalysisWindowNonLinearSubHandler : public CBasicConfigSubHandler
{
 public:
  CAnalysisWindowNonLinearSubHandler(CConfigHandler *master,
				     struct anlyswin_nonlinear *d);
  virtual ~CAnalysisWindowNonLinearSubHandler();

  virtual bool start(const QXmlAttributes &atts);

 private:
  struct anlyswin_nonlinear *m_d;
};

class CAnalysisWindowShiftStretchSubHandler : public CBasicConfigSubHandler
{
 public:
  CAnalysisWindowShiftStretchSubHandler(CConfigHandler *master,
					shift_stretch_list_t *d);
  virtual ~CAnalysisWindowShiftStretchSubHandler();

  virtual bool start(const QXmlAttributes &atts);
  virtual bool start(const QString &element, const QXmlAttributes &atts);
  virtual bool end(void);

 private:
  shift_stretch_list_t *m_d;
};

class CAnalysisWindowGapSubHandler : public CBasicConfigSubHandler
{
 public:
  CAnalysisWindowGapSubHandler(CConfigHandler *master,
			       gap_list_t *d);
  virtual ~CAnalysisWindowGapSubHandler();

  virtual bool start(const QXmlAttributes &atts);

 private:
  gap_list_t *m_d;
};

class CAnalysisWindowOutputSubHandler : public CBasicConfigSubHandler
{
 public:
  CAnalysisWindowOutputSubHandler(CConfigHandler *master,
				  output_list_t *d);
  virtual ~CAnalysisWindowOutputSubHandler();

  virtual bool start(const QXmlAttributes &atts);

 private:
  output_list_t *m_d;
};

class CAnalysisWindowSfpSubHandler : public CBasicConfigSubHandler
{
 public:
  CAnalysisWindowSfpSubHandler(CConfigHandler *master,
			       struct calibration_sfp *d);
  virtual ~CAnalysisWindowSfpSubHandler();

  virtual bool start(const QXmlAttributes &atts);

 private:
  struct calibration_sfp *m_d;
};


#endif

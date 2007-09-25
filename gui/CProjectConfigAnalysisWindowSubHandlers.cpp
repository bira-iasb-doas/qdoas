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


#include "CProjectConfigAnalysisWindowSubHandlers.h"

#include "constants.h"

CProjectAnalysisWindowSubHandler::CProjectAnalysisWindowSubHandler(CQdoasProjectConfigHandler *master,
								   CAnalysisWindowConfigItem *item) :
  CConfigSubHandler(master),
  m_item(item)
{
}

CProjectAnalysisWindowSubHandler::~CProjectAnalysisWindowSubHandler()
{

}

bool CProjectAnalysisWindowSubHandler::start(const QXmlAttributes &atts)
{
  mediate_analysis_window_t *d = m_item->properties();

  m_item->setName(atts.value("name"));

  QString str = atts.value("kurucz");
  if (str == "ref")
    d->kuruczMode = ANLYS_KURUCZ_REF;
  else if (str == "spec")
    d->kuruczMode = ANLYS_KURUCZ_SPEC;
  else if (str == "ref+spec")
    d->kuruczMode = ANLYS_KURUCZ_REF_AND_SPEC;
  else
    d->kuruczMode = ANLYS_KURUCZ_NONE;

  d->refSpectrumSelection = (atts.value("refsel") == "auto") ? ANLYS_REF_SELECTION_MODE_AUTOMATIC :  ANLYS_REF_SELECTION_MODE_FILE;

  d->fitMinWavelength = atts.value("min").toInt();
  d->fitMaxWavelength = atts.value("max").toInt();

  // MUST have a valid name
  return !m_item->name().isEmpty();
}

bool CProjectAnalysisWindowSubHandler::start(const QString &element, const QXmlAttributes &atts)
{
  mediate_analysis_window_t *d = m_item->properties();

  if (element == "display") {
    d->requireSpectrum = (atts.value("spectrum") == "true") ? 1 : 0;
    d->requirePolynomial = (atts.value("poly") == "true") ? 1 : 0;
    d->requireFit = (atts.value("fits") == "true") ? 1 : 0;
    d->requireResidual = (atts.value("residual") == "true") ? 1 : 0;
    d->requirePredefined = (atts.value("predef") == "true") ? 1 : 0;
    d->requireRefRatio = (atts.value("ratio") == "true") ? 1 : 0;

  }
  else if (element == "files") {
    QString str;

    str = atts.value("refone");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->refOneFile))
	strcpy(d->refOneFile, str.toAscii().data());
      else
	return postErrorMessage("Reference 1 Filename too long");
    }

    str = atts.value("reftwo");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->refTwoFile))
	strcpy(d->refTwoFile, str.toAscii().data());
      else
	return postErrorMessage("Reference 2 Filename too long");
    }

    str = atts.value("residual");
    if (!str.isEmpty()) {
      str = m_master->pathExpand(str);
      if (str.length() < (int)sizeof(d->residualFile))
	strcpy(d->residualFile, str.toAscii().data());
      else
	return postErrorMessage("Residual Filename too long");
    }

    d->refSzaCenter = atts.value("szacenter").toDouble();
    d->refSzaDelta = atts.value("szadelta").toDouble();

  }

  return true;
}

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

#include <cmath>

#include "mediate.h"

#include "CEngineResponse.h"
#include "CPlotDataSet.h"

void mediateResponseSpectrumData(int page,
				 double *intensityDataArray,
				 double *wavelengthDataArray,
                                 int arrayLength,
				 const char *title,
				 const char *wavelengthLabel,
				 const char *intensityLabel,
				 void *responseHandle)
{
  CEngineResponseBrowseRecord *resp = static_cast<CEngineResponseBrowseRecord*>(responseHandle);


  CPlotDataSet *dataSet = new CPlotDataSet(wavelengthDataArray, intensityDataArray, arrayLength,
                                           title, wavelengthLabel, intensityLabel);
  resp->addDataSet(page, dataSet);
}

void mediateResponseGraphicalData(int page,
				  double *ordinateArray,
				  double *abscissaArray,
				  int arrayLength,
				  int dataType,
				  void *responseHandle)
{
}

void mediateResponseCellDataDouble(int page,
				   int row,
				   int column,
				   double doubleValue,
				   void *responseHandle)
{
  CEngineResponseBrowseRecord *resp = static_cast<CEngineResponseBrowseRecord*>(responseHandle);
  resp->addCell(page, row, column, QVariant(doubleValue));
}

void mediateResponseCellDataInteger(int page,
				    int row,
				    int column,
				    int integerValue,
				    void *responseHandle)
{
  CEngineResponseBrowseRecord *resp = static_cast<CEngineResponseBrowseRecord*>(responseHandle);
  resp->addCell(page, row, column, QVariant(integerValue));
}

void mediateResponseCellDataString(int page,
				   int row,
				   int column,
				   const char *stringValue,
				   void *responseHandle)
{
  CEngineResponseBrowseRecord *resp = static_cast<CEngineResponseBrowseRecord*>(responseHandle);
  resp->addCell(page, row, column, QVariant(QString(stringValue)));
}

void mediateResponseLabelPage(int page,
			      const char *title,
			      const char *tag,
			      void *responseHandle)
{
  CEngineResponseBrowseRecord *resp = static_cast<CEngineResponseBrowseRecord*>(responseHandle);
  resp->addPageTitleAndTag(page, title, tag);
}


void mediateResponseErrorMessage(const char *messageString,
				 int errorLevel,
				 void *responseHandle)
{
}

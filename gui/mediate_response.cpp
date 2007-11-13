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

#include "mediate_response.h"

#include "CEngineResponse.h"
#include "CPlotDataSet.h"

void mediateAllocateAndSetPlotData(plot_data_t *d, double *xData, double *yData, int len, enum ePlotDataType type, const char *legend)
{
  int byteLen = len * sizeof(double);

  d->x = (double*)malloc(byteLen);
  d->y = (double*)malloc(byteLen);
  d->legendStr = (char*)malloc(strlen(legend)+1);

  if (d->x == NULL || d->y == NULL || d->legendStr == NULL) {
    // bail out ...
    mediateReleasePlotData(d);
    d->x = d->y = NULL;
    d->legendStr = NULL;
    d->length = 0;
    d->plotType = type;
  }
  else {
    memcpy(d->x, xData, byteLen);
    memcpy(d->y, yData, byteLen);
    strcpy(d->legendStr, legend);
    d->length = len;
    d->plotType = type;
  }
}

void mediateReleasePlotData(plot_data_t *d)
{
  if (d->x) free(d->x);
  if (d->y) free(d->y);
  if (d->legendStr) free(d->legendStr);
}


void mediateResponsePlotData(int page,
			     plot_data_t *plotDataArray,
			     int arrayLength,
			     const char *title,
			     const char *xLabel,
			     const char *yLabel,
			     void *responseHandle)
{
  CEngineResponseSpecificRecord *resp = static_cast<CEngineResponseSpecificRecord*>(responseHandle);

  CPlotDataSet *dataSet = new CPlotDataSet(title, xLabel, yLabel);

  int i = 0;
  while (i < arrayLength) {
    dataSet->addPlotData(plotDataArray[i].x, plotDataArray[i].y, plotDataArray[i].length,
			 plotDataArray[i].plotType, plotDataArray[i].legendStr);
    ++i;
  }

  resp->addDataSet(page, dataSet);
}

void mediateResponseCellDataDouble(int page,
				   int row,
				   int column,
				   double doubleValue,
				   void *responseHandle)
{
  CEngineResponseSpecificRecord *resp = static_cast<CEngineResponseSpecificRecord*>(responseHandle);
  resp->addCell(page, row, column, QVariant(doubleValue));
}

void mediateResponseCellDataInteger(int page,
				    int row,
				    int column,
				    int integerValue,
				    void *responseHandle)
{
  CEngineResponseSpecificRecord *resp = static_cast<CEngineResponseSpecificRecord*>(responseHandle);
  resp->addCell(page, row, column, QVariant(integerValue));
}

void mediateResponseCellDataString(int page,
				   int row,
				   int column,
				   const char *stringValue,
				   void *responseHandle)
{
  CEngineResponseSpecificRecord *resp = static_cast<CEngineResponseSpecificRecord*>(responseHandle);
  resp->addCell(page, row, column, QVariant(QString(stringValue)));
}

void mediateResponseCellInfo(int page,
				   int row,
				   int column,
				   void *responseHandle,
				   const char *label,
				   const char *stringFormat,...)
 {
 	va_list argList;
 	char stringValue[1024];

  va_start(argList,stringFormat);
  vsprintf(stringValue,stringFormat,argList);
  va_end(argList);

 	CEngineResponseSpecificRecord *resp = static_cast<CEngineResponseSpecificRecord*>(responseHandle);
 	resp->addCell(page,row,column,QVariant(QString(label)));
 	resp->addCell(page,row,column+1,QVariant(QString(stringValue)));
 }

void mediateResponseCellInfoNoLabel(int page,
				   int row,
				   int column,
				   void *responseHandle,
				   const char *stringFormat,...)
 {
 	va_list argList;
 	char stringValue[1024];

  va_start(argList,stringFormat);
  vsprintf(stringValue,stringFormat,argList);
  va_end(argList);

 	CEngineResponseSpecificRecord *resp = static_cast<CEngineResponseSpecificRecord*>(responseHandle);
 	resp->addCell(page,row,column,QVariant(QString(stringValue)));
 }

void mediateResponseLabelPage(int page,
			      const char *title,
			      const char *tag,
			      void *responseHandle)
{
  CEngineResponseSpecificRecord *resp = static_cast<CEngineResponseSpecificRecord*>(responseHandle);
  resp->addPageTitleAndTag(page, title, tag);
}


void mediateResponseErrorMessage(const char *function,
				 const char *messageString,
				 enum eEngineErrorType errorType,
				 void *responseHandle)
{
  CEngineResponse *resp = static_cast<CEngineResponse*>(responseHandle);
  resp->addErrorMessage(QString(function), QString(messageString), errorType);
}



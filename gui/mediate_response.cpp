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

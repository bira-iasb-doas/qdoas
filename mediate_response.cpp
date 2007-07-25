#include <cmath>

#include "mediate.h"

#include "CEngineResponse.h"
#include "CPlotDataSet.h"

void mediateResponseSpectrumData(double *intensityDataArray,
				 double *wavelengthDataArray,
                                 int arrayLength,
				 void *responseHandle)
{
  CEngineResponseBrowseRecord *resp = static_cast<CEngineResponseBrowseRecord*>(responseHandle);

  CPlotDataSet *dataSet = new CPlotDataSet(wavelengthDataArray, intensityDataArray, arrayLength,
                                           "Spectrum", "Wavelength (nm)", "I");
  resp->addDataSet(0, dataSet);
}

void mediateResponseGraphicalData(double *ordinateArray,
				  double *abscissaArray,
				  int arrayLength,
                                  int page,
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
				   void * responseHandle)
{
  CEngineResponseBrowseRecord *resp = static_cast<CEngineResponseBrowseRecord*>(responseHandle);
  resp->addCell(page, row, column, QVariant(QString(stringValue)));
}

void mediateResponseErrorMessage(const char *messageString,
				 int errorLevel,
				 void *responseHandle)
{
}

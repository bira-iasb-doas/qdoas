#include "mediate.h"


int mediateRequestCreateEngineContext(void **engineContext)
{
  // TODO
  *engineContext = 0x0;

  return 0;
}

int mediateRequestDestroyEngineContext(void *engineContext)
{
  return 0;
}

int mediateRequestSetProject(void *engineContext,
			     const mediate_project_t *project)
{
  return 0;
}

int mediateRequestSetAnalysisWindows(void *engineContext,
				     int numberOfWindows,
				     const mediate_analysis_window_t *analysisWindows)
{
  return 0;
}

int mediateRequestSetSymbols(void *engineContext,
			     int numberOfSymbols,
			     const mediate_symbol_t *symbols)
{
  return 0;
}

int mediateRequestBeginBrowseSpectra(void *engineContext,
				     const char *spectraFileName,
				     void *responseHandle)
{
  return 5; // TODO
}

int mediateRequestGotoSpectrum(void *engineContext,
			       int recordNumber,
			       void *responseHandle)
{
  return (recordNumber > 5) ? -1 : recordNumber;
}

int mediateRequestGetNextMatchingSpectrum(void *engineContext,
					  void *responseHandle)
{
  return 3; // TODO
}

int mediateRequestEndBrowseSpectra(void *engineContext,
				   void *responseHandle)
{
  return 0;
}

int mediateRequestBeginAnalysis(void *engineContext,
				const char *spectraFileName,
				void *responseHandle)
{
  return 0;
}

int mediateRequestAnalyseNextMatchingSpectrum(void *engineContext,
					      void *responseHandle)
{
  return 0;
}

int mediateRequestEndAnalysis(void *engineContext,
			      void *responseHandle)
{
  return 0;
}

void mediateResponseSpectrumData(double *intensityDataArray,
				 double *wavelengthDataArray,
                                 int arrayLength,
				 void *responseHandle)
{
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
}

void mediateResponseCellDataInteger(int page,
				    int row,
				    int column,
				    int integerValue,
				    void *responseHandle)
{
}

void mediateResponseCellDataString(int page,
				   int row,
				   int column,
				   const char *stringValue,
				   void * responseHandle)
{
}

void mediateResponseErrorMessage(const char *messageString,
				 int errorLevel,
				 void *responseHandle)
{
}

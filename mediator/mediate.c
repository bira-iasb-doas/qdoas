
#include <stdlib.h>

#include "mediate.h"

// TODO
struct dummy {
  int record;
  int nRecords;
};

int mediateRequestCreateEngineContext(void **engineContext)
{
  // TODO
  struct dummy *tmp = (struct dummy *)malloc(sizeof(struct dummy));

  tmp->nRecords = 15;
  tmp->record = 0;
  *engineContext = (void*)tmp;
  
  return 0;
}

int mediateRequestDestroyEngineContext(void *engineContext)
{
  free(engineContext);

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
  struct dummy *tmp = (struct dummy *)engineContext;

  tmp->record = 1;

  return tmp->nRecords;
}

int mediateRequestGotoSpectrum(void *engineContext,
			       int recordNumber,
			       void *responseHandle)
{
  struct dummy *tmp = (struct dummy *)engineContext;

  if (recordNumber > 0 && recordNumber <= tmp->nRecords) {
    tmp->record = recordNumber;
    return recordNumber;
  }
  else {
    tmp->record = tmp->nRecords + 1;
    return 0;
  }
}

int mediateRequestGetNextMatchingSpectrum(void *engineContext,
					  void *responseHandle)
{
  struct dummy *tmp = (struct dummy *)engineContext;

  int rec = (tmp->record)++;

  while (rec % 2) {
    rec = (tmp->record)++;
  }

  if (rec > tmp->nRecords)
    return 0;
  else
    return rec;
}

int mediateRequestEndBrowseSpectra(void *engineContext,
				   void *responseHandle)
{
  struct dummy *tmp = (struct dummy *)engineContext;
  
  tmp->record = -1;
  ++(tmp->nRecords);

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

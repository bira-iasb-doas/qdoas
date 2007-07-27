
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
  double x[1024], y[1024];
  int i;
  double xx, scale;
  
  if (rec > tmp->nRecords) {
    return 0;
  }
  else {

    scale = (double)rec;
    xx = 0.0;
    i = 0;
    while (i<1024) {
      x[i] = xx;
      y[i] = scale * sin(xx);
      xx += scale * 0.001;
      ++i;
    }

    mediateResponseSpectrumData(0, y, x, 1024, "Spectrum", "Lambda (nm)", NULL, responseHandle);
    mediateResponseSpectrumData(5, y, x, 512, "Fred", "on page 5", "blah", responseHandle);

    mediateResponseCellDataString(0, 3, 2, "of", responseHandle);
    mediateResponseCellDataInteger(0, 5, 1, 987654, responseHandle);
    mediateResponseCellDataString(0, 0, 0, "Origin", responseHandle);
    mediateResponseCellDataDouble(0, 5, 4, 1.23456e7, responseHandle);
    mediateResponseCellDataInteger(5, 3, 3, tmp->nRecords, responseHandle);
    mediateResponseCellDataInteger(5, 3, 1, tmp->record, responseHandle);

    mediateResponseLabelPage(0, "File and record number ...", "Spectrum", responseHandle);
    mediateResponseLabelPage(5, "Analysis ...", "BrO", responseHandle);

    
    return rec;
  }
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

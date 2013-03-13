#ifndef OUTPUT_H
#define OUTPUT_H

#include "engine.h"
#include "output_formats.h"

/*! \file output.h \brief Output module interface.*/

void OUTPUT_ResetData(void);

RC OUTPUT_RegisterData(const ENGINE_CONTEXT *pEngineContext);

/*! \brief Write all saved output data to disk. */
RC OUTPUT_FlushBuffers(ENGINE_CONTEXT *pEngineContext);

/*! \brief Save the results of the current spectrum in the output fields.
  \param [in] indexFenoColumn detector row of the processed record.

  \param [in,out] pEngineContext   structure including information on the current record

  \retval Non zero value return code if the function failed
  \retval ERROR_ID_NO on success
*/
RC OUTPUT_SaveResults(ENGINE_CONTEXT *pEngineContext,INDEX indexFenoColumn);

RC OUTPUT_ReadAmf(const char *symbolName,const char *amfFileName,char amfType,INDEX *pIndexAmf);
RC OUTPUT_GetWveAmf(CROSS_RESULTS *pResults,double Zm,double *lambda,double *xs);
RC OUTPUT_LocalAlloc(ENGINE_CONTEXT *pEngineContext);
RC OUTPUT_Alloc(void);
void OUTPUT_Free(void);

/*! \brief For GOME-2/Sciamachy automatic reference spectrum: file
    from which the reference was generated. */
extern char OUTPUT_refFile[MAX_PATH_LEN+1];
/*! \brief For GOME-2/Sciamachy automatic reference spectrum: number
    of spectra used. */
extern int         OUTPUT_nRec;
/*! \brief List of cross sections with associated AMF file */
extern AMF_SYMBOL *OUTPUT_AmfSpace;

#endif

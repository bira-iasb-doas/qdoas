
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  OUTPUT
//  Name of module    :  OUTPUT.C
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//  The QT version of the program has been developed jointly by the Belgian
//  Institute for Space Aeronomy (BIRA-IASB) and the Science and Technology
//  company (S[&]T) - Copyright (C) 2007
//
//      BIRA-IASB                                   S[&]T
//      Belgian Institute for Space Aeronomy        Science [&] Technology
//      Avenue Circulaire, 3                        Postbus 608
//      1180     UCCLE                              2600 AP Delft
//      BELGIUM                                     THE NETHERLANDS
//      thomas.danckaert@aeronomie.be               info@stcorp.nl
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//  ----------------------------------------------------------------------------

/*! \file output.c
  \brief Set up output buffers according to the
  user's configuration and write output to files.

  All logic regarding what should written to the output file is
  handled via OUTPUT_RegisterData():  Each variable that should be
  written to output is described using a struct output_field.  Output
  fields containing results of the reference calibration are stored in
  the array #output_data_calib, and output fields containing results of
  the analyis (or run calibration) are contained in the array
  #output_data_analysis.

  The main members of the output_field structure are the buffer
  output_field::data, used to store the required data until it is
  written to file, and the function pointer output_field::get_data(),
  used to retrieve the data that should go into output_field::data.
  The functions OutputRegisterFields(), OutputRegisterParam(),
  register_calibration() and OutputRegisterFluxes() create the right
  output_field structures depending on the user's configuration,
  setting pointers to the required functions from output_private.h.

  For the calibration output, a different field is registered for each
  selected calibration variable and each detector row (indexFenoColumn).

  For analysis output, different fields are registered for each
  selected analysis variable and for each analysis window.
  */

#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <search.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "output.h"
#include "output_netcdf.h"
#include "output_common.h"

#include "engine_context.h"
#include "winsites.h"
#include "winfiles.h"
#include "kurucz.h"
#include "analyse.h"
#include "stdfunc.h"
#include "spline.h"
#include "omi_read.h"
#include "vector.h"
#include "winthrd.h"
#include "zenithal.h"

#include "gdp_bin_read.h"
#include "spectrum_files.h"

AMF_SYMBOL *OUTPUT_AmfSpace;                                                 // list of cross sections with associated AMF file

char OUTPUT_refFile[DOAS_MAX_PATH_LEN+1];
int OUTPUT_nRec;

const char *QDOAS_FILL_STRING = "";
const char QDOAS_FILL_BYTE = -127;
const char QDOAS_FILL_CHAR = 0;
const short QDOAS_FILL_SHORT = -32767;
const int QDOAS_FILL_INT = -2147483647;
const float QDOAS_FILL_FLOAT = 9.9692099683868690e+36f; /* near 15 * 2^119 */
const double QDOAS_FILL_DOUBLE = 9.9692099683868690e+36;
const unsigned char QDOAS_FILL_UBYTE = 255;
const unsigned short QDOAS_FILL_USHORT = 65535;
const unsigned int QDOAS_FILL_UINT = 4294967295U;
const long long QDOAS_FILL_INT64 = -9223372036854775806LL;
const unsigned long long QDOAS_FILL_UINT64 = 18446744073709551614ULL;

/*! \brief Array matching enum output_format values with strings
    containing their filename extension.  The position in the array
    must correspond to the value of the enum.*/
const char *output_file_extensions[] = { [ASCII] = ".ASC",
                                         [HDFEOS5] = ".he5",
                                         [NETCDF] = ".nc" };

struct output_field output_data_analysis[MAX_FIELDS];
unsigned int output_num_fields = 0;
struct output_field output_data_calib[MAX_FIELDS];
unsigned int calib_num_fields = 0;

// ===================
// STATIC DEFINITIONS
// ===================
static unsigned int outputNbRecords; /*!< \brief Number of records written to output.*/
static int NAmfSpace; /*!< \brief Number of elements in buffer OUTPUT_NAmfSpace */
static OUTPUT_INFO *outputRecords; /*!< \brief Meta data on the records written to output.*/
static int outputRunCalib, /*!< \brief ==1 in run calibration mode */
  outputCalibFlag; /*!< \brief <> 0 to save wavelength calibration parameters */
static double OUTPUT_fluxes[MAX_FLUXES], /*!< \brief fluxes */
  OUTPUT_cic[MAX_CIC][2];  /*!< \brief color indexes */
static int OUTPUT_NFluxes, /*!< \brief number of fluxes in OUTPUT_fluxes array */
  OUTPUT_NCic; /*!< \brief number of color indexes in OUTPUT_cic array */

#define output_c
#include "output_private.h"

static enum output_format selected_format = ASCII; // ASCII files as default

/** @name Output function pointer types
 *
 * Function pointer types for the functions that retrieve data of
 * various types (float, double, ...).
 */
//!@{
typedef void (*func_float)(struct output_field *this_field, float *datbuf, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib);
typedef void (*func_double)(struct output_field *this_field, double *datbuf, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib);
typedef void (*func_int)(struct output_field *this_field, int *datbuf,const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib);
typedef void (*func_uint)(struct output_field *this_field, unsigned int *datbuf,const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib);
typedef void (*func_ushort)(struct output_field *this_field, unsigned short *datbuf,const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib);
typedef void (*func_short)(struct output_field *this_field, short *datbuf,const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib);
typedef void (*func_date)(struct output_field *this_field, struct date *date,const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib);
typedef void (*func_time)(struct output_field *this_field, struct time *time,const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib);
typedef void (*func_datetime)(struct output_field *this_field, struct datetime *datetime,const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib);
typedef void (*func_string)(struct output_field *this_field, char **datbuf,const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn, int index_calib);
//!@}

#define FORMAT_FLOAT "%#8.3f"
#define FORMAT_DOUBLE "%#12.4le"
#define FORMAT_INT "%#6d"

static void save_calibration(void);
static void output_field_clear(struct output_field *this_field);
static void output_field_free(struct output_field *this_field);
struct field_attribute *copy_attributes(const struct field_attribute *attributes, int num_attributes);

/*! \brief Calculate flux for a given wavelength.*/
double output_flux(const ENGINE_CONTEXT *pEngineContext, double wavelength) {
  double flux =0.;
  double bandWidth=pEngineContext->project.asciiResults.bandWidth;

  if ( wavelength >= pEngineContext->buffers.lambda[0] &&
       wavelength <= pEngineContext->buffers.lambda[NDET-1] ) {

    int imin;
    int imax;

    // The interval is defined in nm

    imin=max(FNPixel(pEngineContext->buffers.lambda,wavelength-0.5*bandWidth,NDET,PIXEL_CLOSEST),0);
    imax=min(FNPixel(pEngineContext->buffers.lambda,wavelength+0.5*bandWidth,NDET,PIXEL_CLOSEST),NDET-1);

    // Flux calculation

    for (int i=imin; i<=imax; i++)
      flux+=pEngineContext->buffers.spectrum[i];

    flux/=(double)(imax-imin+1);
   }
  return flux;
}

/*! \brief Structure to associate a _prjctResutls type to an
    output_field configuration.*/
struct outputconfig {
  enum _prjctResults type;
  struct output_field field;
};

/*! \brief Comparison function to search an output field configuration
  by its _prjctResults type using lfind.*/
static int compare_record(const void *searched_type, const void *record) {
  return *((enum _prjctResults *)searched_type) - ((struct outputconfig *)record)->type;
}

/*! \brief Comparison function to search a string in an array using
    lfind. */
static int compare_string(const void *left, const void *right) {
  return strcasecmp( (char*) left, *((char**) right));
}

/*! \brief Save the data of a single record of an output_field to the field's data buffer.

  The output_field::memory_type is used to cast the buffer and the
  output_field::get_data function pointer to the correct data types.*/
static void save_analysis_data(struct output_field *output_field, int recordno, const ENGINE_CONTEXT *pEngineContext, int indexFenoColumn) {
  size_t ncols = output_field->data_cols;
  void *data = output_field->data;
  int index_calib = output_field->index_calib;
  func_void get_data = output_field->get_data;
  switch(output_field->memory_type)
    {
    case OUTPUT_INT:
      ((func_int) get_data)(output_field, ((int (*)[ncols])data)[recordno], pEngineContext, indexFenoColumn, index_calib);
      break;
    case OUTPUT_SHORT:
      ((func_short) get_data)(output_field, ((short (*)[ncols])data)[recordno], pEngineContext, indexFenoColumn, index_calib);
      break;
    case OUTPUT_USHORT:
      ((func_ushort) get_data)(output_field, ((unsigned short (*)[ncols])data)[recordno], pEngineContext, indexFenoColumn, index_calib);
      break;
    case OUTPUT_STRING:
      ((func_string) get_data)(output_field, ((char* (*)[ncols])data)[recordno], pEngineContext, indexFenoColumn, index_calib);
      break;
    case OUTPUT_FLOAT:
      ((func_float) get_data)(output_field, ((float (*)[ncols])data)[recordno], pEngineContext, indexFenoColumn, index_calib);
      break;
    case OUTPUT_DOUBLE:
      ((func_double) get_data)(output_field, ((double (*)[ncols])data)[recordno], pEngineContext, indexFenoColumn, index_calib);
      break;
    case OUTPUT_DATE:
      ((func_date) get_data)(output_field, ((struct date (*)[ncols])data)[recordno], pEngineContext, indexFenoColumn, index_calib);
      break;
    case OUTPUT_TIME:
      ((func_time) get_data)(output_field, ((struct time (*)[ncols])data)[recordno], pEngineContext, indexFenoColumn, index_calib);
      break;
    case OUTPUT_DATETIME:
      ((func_datetime) get_data)(output_field, ((struct datetime (*)[ncols])data)[recordno], pEngineContext, indexFenoColumn, index_calib);
      break;
    }
}

/*! \brief save the calibration data for output_field from the
    calibration subwindow index_calib */
static void save_calib_data(struct output_field *output_field, int index_calib) {
  switch(output_field->memory_type)
    {
    case OUTPUT_INT:
      ((func_int) output_field->get_data)(output_field,  &((int *)output_field->data)[index_calib], NULL, output_field->index_row, index_calib);
      break;
    case OUTPUT_SHORT:
      ((func_short) output_field->get_data)(output_field,  &((short *)output_field->data)[index_calib], NULL, output_field->index_row, index_calib);
      break;
    case OUTPUT_USHORT:
      ((func_ushort) output_field->get_data)(output_field,  &((unsigned short *)output_field->data)[index_calib], NULL, output_field->index_row, index_calib);
      break;
    case OUTPUT_STRING:
      ((func_string) output_field->get_data)(output_field,  &((char **)output_field->data)[index_calib], NULL, output_field->index_row, index_calib);
      break;
    case OUTPUT_FLOAT:
      ((func_float) output_field->get_data)(output_field,  &((float *)output_field->data)[index_calib], NULL, output_field->index_row, index_calib);
      break;
    case OUTPUT_DOUBLE:
      ((func_double) output_field->get_data)(output_field,  &((double *)output_field->data)[index_calib], NULL, output_field->index_row, index_calib);
      break;
    case OUTPUT_DATE:
      ((func_date) output_field->get_data)(output_field,  &((struct date *)output_field->data)[index_calib], NULL, output_field->index_row, index_calib);
      break;
    case OUTPUT_TIME:
      ((func_time) output_field->get_data)(output_field,  &((struct time *)output_field->data)[index_calib], NULL, output_field->index_row, index_calib);
      break;
    case OUTPUT_DATETIME:
      ((func_datetime) output_field->get_data)(output_field,  &((struct datetime *)output_field->data)[index_calib], NULL, output_field->index_row, index_calib);
      break;
  }
}

static void register_analysis_output(const PRJCT_RESULTS *pResults, int indexFeno, int index_calib, const char *windowName);
static void register_cross_results(const PRJCT_RESULTS *pResults, const FENO *pTabFeno, int indexFeno, int index_calib, const char *windowName);

/*! \brief Correct a cross section using wavelength dependent AMF vector

  \param [in] pResults     output options for the selected cross section
  \param [in] Zm           the current solar zenith angle
  \param [in] lambda       the current wavelength calibration

  \param [in,out] xs       the cross section to correct by wavelength dependent AMF

  \param [out] deriv2      second derivatives of the new cross section

  */
RC OUTPUT_GetWveAmf(CROSS_RESULTS *pResults,double Zm,double *lambda,double *xs)
 {
  // Declarations

  AMF_SYMBOL *pAmfSymbol;
  INDEX i;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  // This procedure applies only on wavelength dependent AMF

  if ((pResults->indexAmf!=ITEM_NONE) && (OUTPUT_AmfSpace!=NULL))
   {
   	pAmfSymbol=&OUTPUT_AmfSpace[pResults->indexAmf];

    for (i=0;i<NDET;i++)
     xs[i]*=(double)VECTOR_Table2_Index1(pAmfSymbol->Phi,pAmfSymbol->PhiLines,pAmfSymbol->PhiColumns,(double)lambda[i],(double)Zm);
   }

  // Return

  return rc;
 }

/*! \brief Return the AMF from table at a specified zenith angle

  \param [in] pResults     output options for the selected cross section

  \param [in] Zm           the current solar zenith angle

  \param [in] Tm           the current number of seconds in order to retrieve
                           AMF from a climatology table

  \param [out] pAmf        the calculated AMF

  \retval ERROR_ID_NO in case of success, any other value in case of error
*/
RC OutputGetAmf(CROSS_RESULTS *pResults,double Zm,double Tm,double *pAmf)
 {
  // Declarations

  AMF_SYMBOL *pAmfSymbol;
  double Dte;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;
  *pAmf=(double)1.;

  if ((pResults->indexAmf!=ITEM_NONE) && (OUTPUT_AmfSpace!=NULL))
   {
    pAmfSymbol=&OUTPUT_AmfSpace[pResults->indexAmf];

    if (pAmfSymbol->Phi!=NULL)

     switch(pAmfSymbol->type)
      {
    // ------------------------------------------------------------------------
       case ANLYS_AMF_TYPE_CLIMATOLOGY :
        {
         Dte=(double) (ZEN_FNCaljda(&Tm)-1.)/365.;

         if ((Zm>=(double)pAmfSymbol->Phi[1][2]) && (Zm<=(double)pAmfSymbol->Phi[1][pAmfSymbol->PhiLines]) &&
             (Dte>=(double)pAmfSymbol->Phi[2][1]) && (Dte<=(double)pAmfSymbol->Phi[pAmfSymbol->PhiColumns][1]))

          *pAmf=(double)VECTOR_Table2_Index1(pAmfSymbol->Phi,pAmfSymbol->PhiLines,pAmfSymbol->PhiColumns,(double)Zm,(double)Dte);
        }
       break;
    // ------------------------------------------------------------------------
       case ANLYS_AMF_TYPE_SZA :

        if ((pAmfSymbol->deriv2!=NULL) &&
           ((rc=SPLINE_Vector(pAmfSymbol->Phi[1]+1,pAmfSymbol->Phi[2]+1,pAmfSymbol->deriv2[2]+1,pAmfSymbol->PhiLines,&Zm,pAmf,1,
                                 SPLINE_CUBIC,"OutputGetAmf"))!=ERROR_ID_NO))

         rc=ERROR_ID_AMF;

       break;
    // ------------------------------------------------------------------------
      }
   }

  // Return

  return rc;
 }

/*! \brief Load Air Mass Factors from file

 \param [in] symbolName   the name of the selected symbol

 \param [in] amfFileName  the name of the AMF file to load

 \param [in] amfType      the type of AMF to load

 \param [out] pIndexAmf   the index of AMF data in the AMF table

 \retval ERROR_ID_NO in case of success, any other value in case of error
*/
RC OUTPUT_ReadAmf(const char *symbolName,const char *amfFileName,char amfType,INDEX *pIndexAmf)
{
  // Declarations

  char  fileType,                                                             // file extension and type
        *oldColumn,*nextColumn;                                                 // go to the next column in record or the next record
  SZ_LEN symbolLength,fileLength;                                               // length of symbol and lines strings
  AMF_SYMBOL *pAmfSymbol;                                                       // pointer to an AMF symbol
  INDEX indexSymbol,indexColumn;                                                // indexes for loops and arrays
  int PhiLines,PhiColumns,xsLines,xsColumns;                                    // dimensions of the AMF matrix
  FILE *amfFp;                                                                  // pointer to AMF file
  double **Phi,**deriv2,**xs,**xsDeriv2;                                        // pointers to resp. AMF buffer and second derivatives
  RC rc;                                                                        // return code

  // Initializations

  rc=ERROR_ID_NO;
  nextColumn=oldColumn=NULL;
  xsLines=xsColumns=0;
  indexSymbol=ITEM_NONE;
  deriv2=xs=xsDeriv2=NULL;
  amfFp=NULL;

  if (!(symbolLength=strlen(symbolName)) || !(fileLength=strlen(amfFileName)) || (amfType==ANLYS_AMF_TYPE_NONE))
   indexSymbol=ITEM_NONE;
  else if (OUTPUT_AmfSpace!=NULL) // ELSE LEVEL 0
   {
    // Retrieve the type of file

    fileType=amfType;

    // Search for symbol in list

    for (indexSymbol=0;indexSymbol<NAmfSpace;indexSymbol++)
     {
      pAmfSymbol=&OUTPUT_AmfSpace[indexSymbol];

      if ((pAmfSymbol->type==fileType) &&
          (strlen(pAmfSymbol->symbolName)==symbolLength) &&
          (strlen(pAmfSymbol->amfFileName)==fileLength) &&
          !strcasecmp(pAmfSymbol->symbolName,symbolName) &&
          !strcasecmp(pAmfSymbol->amfFileName,amfFileName))

       break;
     }

    // A new symbol is found

    if (indexSymbol==NAmfSpace)
     {
      // Symbol list is limited to MAX_SYMB symbols

      if (NAmfSpace>=MAX_SYMB)
       rc=ERROR_SetLast("OUTPUT_ReadAmf",ERROR_TYPE_FATAL,ERROR_ID_OUT_OF_RANGE,OUTPUT_AmfSpace,0,MAX_SYMB-1);

      // File read out

      else    // ELSE LEVEL 1
       {
        if ((amfFp=fopen(amfFileName,"rt"))==NULL)
         rc=ERROR_SetLast("OUTPUT_ReadAmf",ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,amfFileName);
        else if (!(fileLength=STD_FileLength(amfFp)))
         rc=ERROR_SetLast("OUTPUT_ReadAmf",ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,amfFileName);
        else if (((nextColumn=(char *)MEMORY_AllocBuffer("OUTPUT_ReadAmf ","nextColumn",fileLength+1,1,0,MEMORY_TYPE_STRING))==NULL) ||
                 ((oldColumn=(char *)MEMORY_AllocBuffer("OUTPUT_ReadAmf ","oldColumn",fileLength+1,1,0,MEMORY_TYPE_STRING))==NULL))
         rc=ERROR_ID_ALLOC;
        else
         {
          if ((fileType==ANLYS_AMF_TYPE_CLIMATOLOGY) || (fileType==ANLYS_AMF_TYPE_SZA) || (fileType==ANLYS_AMF_TYPE_WAVELENGTH))
           rc=FILES_GetMatrixDimensions(amfFp,amfFileName,&PhiLines,&PhiColumns,"OUTPUT_ReadAmf",ERROR_TYPE_FATAL);
         }

        if (rc)
         goto EndOUTPUT_ReadAmf;

        // Allocate AMF matrix

        pAmfSymbol=&OUTPUT_AmfSpace[indexSymbol];
        fseek(amfFp,0L,SEEK_SET);

        if (((Phi=pAmfSymbol->Phi=(double **)MEMORY_AllocDMatrix("OUTPUT_ReadAmf ","Phi",0,PhiLines,1,PhiColumns))==NULL) ||
            ((fileType!=ANLYS_AMF_TYPE_CLIMATOLOGY) && (fileType!=ANLYS_AMF_TYPE_WAVELENGTH) &&
            ((deriv2=pAmfSymbol->deriv2=(double **)MEMORY_AllocDMatrix("OUTPUT_ReadAmf ","deriv2",1,PhiLines,2,PhiColumns))==NULL)))

         rc=ERROR_ID_ALLOC;

        else if (fileType==ANLYS_AMF_TYPE_CLIMATOLOGY)
         rc=FILES_LoadMatrix(amfFp,amfFileName,Phi,1,PhiLines,PhiColumns,"OUTPUT_ReadAmf",ERROR_TYPE_FATAL);
        else if (fileType==ANLYS_AMF_TYPE_SZA)
         rc=FILES_LoadMatrix(amfFp,amfFileName,Phi,1,PhiLines,PhiColumns,"OUTPUT_ReadAmf",ERROR_TYPE_FATAL);
        else if (fileType==ANLYS_AMF_TYPE_WAVELENGTH)
         rc=FILES_LoadMatrix(amfFp,amfFileName,Phi,1,PhiLines,PhiColumns,"OUTPUT_ReadAmf",ERROR_TYPE_FATAL);

        if (rc)
         goto EndOUTPUT_ReadAmf;

        // Second derivatives computations

        if ((fileType!=ANLYS_AMF_TYPE_CLIMATOLOGY) && (fileType!=ANLYS_AMF_TYPE_WAVELENGTH))
         for (indexColumn=2;(indexColumn<=PhiColumns) && !rc;indexColumn++)
          rc=SPLINE_Deriv2(Phi[1]+1,Phi[indexColumn]+1,deriv2[indexColumn]+1,PhiLines,"OUTPUT_ReadAmf ");

        // Add new symbol

        if (rc==ERROR_ID_NO)
         {
          pAmfSymbol=&OUTPUT_AmfSpace[indexSymbol];

          strcpy(pAmfSymbol->symbolName,symbolName);
          strcpy(pAmfSymbol->amfFileName,amfFileName);

          pAmfSymbol->type=fileType;
          pAmfSymbol->PhiLines=PhiLines;
          pAmfSymbol->PhiColumns=PhiColumns;
          pAmfSymbol->xsLines=xsLines;
          pAmfSymbol->xsColumns=xsColumns;

          NAmfSpace++;
         }
       }     // END ELSE LEVEL 1
     }    // END if (indexSymbol==NAmfSpace)

    if (indexSymbol>=NAmfSpace)
     indexSymbol=ITEM_NONE;
   }   // END ELSE LEVEL 0

  // Return

  EndOUTPUT_ReadAmf :

  if (nextColumn!=NULL)
   MEMORY_ReleaseBuffer("OUTPUT_ReadAmf ","nextColumn",nextColumn);
  if (oldColumn!=NULL)
   MEMORY_ReleaseBuffer("OUTPUT_ReadAmf ","oldColumn",oldColumn);
  if (amfFp!=NULL)
   fclose(amfFp);

  *pIndexAmf=indexSymbol;

  return rc;
}

/*! \brief release and reset all data used for output */
void OUTPUT_ResetData(void)
{
  // Reset output part of data in analysis windows

  for (int indexFenoColumn=0;indexFenoColumn<MAX_SWATHSIZE;indexFenoColumn++)
   {
    for (int indexFeno=0;indexFeno<MAX_FENO;indexFeno++)
     {
       FENO *pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

      // Browse symbols

      for (int indexTabCross=0;indexTabCross<MAX_FIT;indexTabCross++)
       {
         CROSS_RESULTS *pResults=&pTabFeno->TabCrossResults[indexTabCross];

        pResults->indexAmf=ITEM_NONE;
     // -------------------------------------------
        pResults->StoreAmf=
        pResults->StoreShift=
        pResults->StoreStretch=
        pResults->StoreScale=
        pResults->StoreError=
        pResults->StoreSlntCol=
        pResults->StoreSlntErr=
        pResults->StoreVrtCol=
        pResults->StoreVrtErr=(char)0;
     // -------------------------------------------
        pResults->ResCol=(double)0.;
     // -------------------------------------------
        pResults->Amf=
        pResults->SlntCol=
        pResults->SlntErr=
        pResults->VrtCol=
        pResults->VrtErr=(double)9999.;
     // -------------------------------------------
        pResults->SlntFact=
        pResults->VrtFact=(double)1.;
       }
     }
   }

  // Release AMF matrices

  if (OUTPUT_AmfSpace!=NULL)
   {
    for (int indexSymbol=0;indexSymbol<MAX_SYMB;indexSymbol++)
     {
       AMF_SYMBOL *pAmf=&OUTPUT_AmfSpace[indexSymbol];

      if (pAmf->Phi!=NULL)
       MEMORY_ReleaseDMatrix("OUTPUT_ResetData ","Phi",pAmf->Phi,1,pAmf->PhiColumns,0);
      if (pAmf->deriv2!=NULL)
       MEMORY_ReleaseDMatrix("OUTPUT_ResetData ","deriv2",pAmf->deriv2,2,pAmf->PhiColumns,1);
      if (pAmf->xs!=NULL)
       MEMORY_ReleaseDMatrix("OUTPUT_ResetData ","xs",pAmf->xs,0,pAmf->xsColumns-1,0);
      if (pAmf->xsDeriv2!=NULL)
       MEMORY_ReleaseDMatrix("OUTPUT_ResetData ","xsDeriv2",pAmf->xsDeriv2,1,pAmf->xsColumns-1,1);
     }

    memset(OUTPUT_AmfSpace,0,sizeof(AMF_SYMBOL)*MAX_SYMB);
   }

  // Reset Amf

  NAmfSpace=0;

  for (size_t i=0; i<output_num_fields; i++) {
    output_field_free(&output_data_analysis[i]);
  }
  for (size_t i=0; i<calib_num_fields; i++) {
    output_field_free(&output_data_calib[i]);
  }
  output_num_fields = calib_num_fields = 0;

  if (outputRecords!=NULL)
   MEMORY_ReleaseBuffer("OUTPUT_ResetData","outputRecords",outputRecords);

  outputRecords=NULL;

  outputRunCalib=
  outputCalibFlag=
  outputNbRecords=0;
}

//  =======================
//  REGISTER DATA TO OUTPUT
//  =======================

/*! \brief Register fluxes

  \param [in] pEngineContext   structure including information on project options
*/
static void OutputRegisterFluxes(const ENGINE_CONTEXT *pEngineContext)
{
  // Declarations

   char  columnTitle[MAX_ITEM_NAME_LEN+1];
   const char *ptrOld,*ptrNew;

  // Initializations

  OUTPUT_NFluxes=OUTPUT_NCic=0;

  // Fluxes

  for (ptrOld=pEngineContext->project.asciiResults.fluxes;(ptrOld!=NULL) && (strlen(ptrOld)!=0);ptrOld=ptrNew)
   {
    if (OUTPUT_NFluxes>=MAX_FLUXES)
     break;

    if (sscanf(ptrOld,"%lf",&OUTPUT_fluxes[OUTPUT_NFluxes])==1)
     {
      sprintf(columnTitle,"Fluxes %g",OUTPUT_fluxes[OUTPUT_NFluxes]);
      struct output_field *output_flux = &output_data_analysis[output_num_fields++];
      output_flux->resulttype = PRJCT_RESULTS_FLUX;
      output_flux->fieldname = strdup(columnTitle);
      output_flux->memory_type = OUTPUT_DOUBLE;
      output_flux->format = "%#15.6le";
      output_flux->index_flux = OUTPUT_NFluxes;
      output_flux->get_data = (func_void)&get_flux;
      output_flux->data_cols = 1;
      OUTPUT_NFluxes++;
     }

    if ((ptrNew=strchr(ptrOld,';'))!=NULL)
     ptrNew++;
   }

  // Color indexes

  // for (ptrOld=pEngineContext->project.asciiResults.cic;(ptrOld!=NULL) && (strlen(ptrOld)!=0);ptrOld=ptrNew)
  //  {
  //   if (OUTPUT_NCic>=MAX_CIC)
  //    break;
  //
  //   if (sscanf(ptrOld,"%lf/%lf",&OUTPUT_cic[OUTPUT_NCic][0],&OUTPUT_cic[OUTPUT_NCic][1])>=2)
  //    {
  //     sprintf(columnTitle,"%g/%g",OUTPUT_cic[OUTPUT_NCic][0],OUTPUT_cic[OUTPUT_NCic][1]);
  //     struct output_field *output_cic = &output_data_analysis[output_num_fields++];
  //     output_cic->resulttype = PRJCT_RESULTS_CIC;
  //     output_cic->fieldname = strdup(columnTitle);
  //     output_cic->memory_type = OUTPUT_DOUBLE;
  //     output_cic->format = "%#15.6le";
  //     output_cic->index_cic = OUTPUT_NCic;
  //     output_cic->get_data = (func_void)&get_cic;
  //     output_cic->data_cols = 1;
  //     OUTPUT_NCic++;
  //    }
  //
  //   if ((ptrNew=strchr(ptrOld,';'))!=NULL)
  //    ptrNew++;
  //  }
}

static void register_field(struct output_field field) {
  struct output_field *newfield = &output_data_analysis[output_num_fields++];
  *newfield = field;
  newfield->fieldname = strdup(newfield->basic_fieldname);  // allocate a new buffer for the name so we can free() all output_field data later on.
  newfield->windowname = NULL;
  newfield->windowname = NULL;
  if (newfield->data_cols == 0) // default number of columns is 1
    newfield->data_cols = 1;
  newfield->index_feno = ITEM_NONE;
  newfield->index_calib = ITEM_NONE;
  newfield->index_row = ITEM_NONE;
  newfield->get_tabfeno = outputRunCalib ? get_tabfeno_calib : get_tabfeno_analysis;
}

/*! \brief Register all the fields that are not results of the fit,
    and therefore do not depend on an analysis window.

  \param [in] pEngineContext   structure including information on project options
*/
static void OutputRegisterFields(const ENGINE_CONTEXT *pEngineContext)
{
  PROJECT *pProject=(PROJECT *)&pEngineContext->project;
  PRJCT_RESULTS *pResults=(PRJCT_RESULTS *)&pProject->asciiResults;

  // earth_radius/sat_height/orbit_number: depends on satellite data format
  func_float func_earth_radius = NULL, func_sat_height = NULL;
  func_int func_orbit_number = NULL;

  // default values for instrument-dependent output functions:
  func_int func_meastype = &mfc_get_meastype;
  func_float func_corner_latitudes = NULL;
  func_float func_corner_longitudes = NULL;
  size_t num_sza = 1;
  func_float func_sza = &get_sza;
  size_t num_azimuth = 1;
  func_float func_azimuth = &get_azim;
  size_t num_los_azimuth = 1;
  func_float func_los_azimuth = &get_los_azimuth;
  size_t num_los_zenith = 1;
  func_float func_los_zenith = &get_los_zenith;
  func_float func_scanning_angle = &get_scanning_angle;
  const char *format_datetime = "%4d%02d%02d%02d%02d%02d"; // year, month, day, hour, min, sec
  func_datetime func_datetime = &get_datetime;
  func_double func_frac_time = &get_frac_time;

  const char *title_sza = "SZA";
  const char *title_azimuth = "Solar Azimuth Angle";
  const char *title_los_zenith = "LoS ZA";
  const char *title_los_azimuth = "LoS Azimuth";

  switch(pProject->instrumental.readOutFormat) {
  case PRJCT_INSTR_FORMAT_SCIA_PDS:
  case PRJCT_INSTR_FORMAT_GOME2:
  case PRJCT_INSTR_FORMAT_GDP_BIN:
  case PRJCT_INSTR_FORMAT_GDP_ASCII:
    num_sza = num_azimuth = num_los_zenith = num_los_azimuth = 3;
    break;
  }

  switch(pProject->instrumental.readOutFormat) {
  case PRJCT_INSTR_FORMAT_SCIA_PDS:
    func_frac_time = &get_frac_time_recordinfo;
    func_corner_longitudes = &scia_get_corner_longitudes;
    func_corner_latitudes = &scia_get_corner_latitudes;
    func_los_azimuth = &scia_get_los_azimuth;
    func_los_zenith = &scia_get_los_zenith;
    func_sza = &scia_get_sza;
    func_azimuth = &scia_get_azim;
    func_earth_radius = &scia_get_earth_radius;
    func_sat_height = &scia_get_sat_height;
    format_datetime = "%4d%02d%02d%02d%02d%02d.%03d"; // year, month, day, hour, min, sec, milliseconds
    func_orbit_number = &scia_get_orbit_number;
    break;
  case PRJCT_INSTR_FORMAT_GOME2:
    func_sza = &gome2_get_sza;
    func_azimuth = &gome2_get_azim;
    func_corner_longitudes = &gome2_get_corner_longitudes;
    func_corner_latitudes = &gome2_get_corner_latitudes;
    func_los_azimuth = &gome2_get_los_azimuth;
    func_los_zenith = &gome2_get_los_zenith;
    func_earth_radius = &gome2_get_earth_radius;
    func_sat_height = &gome2_get_sat_height;
    format_datetime = "%4d%02d%02d%02d%02d%02d.%06d"; // year, month, day, hour, min, sec, microseconds
    func_orbit_number = &gome2_get_orbit_number;
    func_frac_time = &get_frac_time_recordinfo;
    break;
  case PRJCT_INSTR_FORMAT_GDP_BIN:
    func_orbit_number = gdp_get_orbit_number;
    if (GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex].gdpBinHeader.version<40) {
      func_sza = &gdp3_get_sza;
      func_azimuth = &gdp3_get_azim;
      func_corner_longitudes = &gdp3_get_corner_longitudes;
      func_corner_latitudes = &gdp3_get_corner_latitudes;
      func_los_azimuth = &gdp3_get_los_azimuth;
      func_los_zenith = &gdp3_get_los_zenith;
      func_earth_radius = &gdp3_get_earth_radius;
      func_sat_height = &gdp3_get_sat_height;
    }
   else {
     func_sza = &gdp4_get_sza;
     func_azimuth = &gdp4_get_azim;
     func_corner_longitudes = &gdp4_get_corner_longitudes;
     func_corner_latitudes = &gdp4_get_corner_latitudes;
     func_los_azimuth = &gdp4_get_los_azimuth;
     func_los_zenith = &gdp4_get_los_zenith;
     func_earth_radius = &gdp4_get_earth_radius;
     func_sat_height = &gdp4_get_sat_height;
   }
    break;
  case PRJCT_INSTR_FORMAT_GDP_ASCII:
    func_sza = &gdpasc_get_sza;
    func_azimuth = &gdpasc_get_azim;
    func_corner_longitudes = &gdpasc_get_corner_longitudes;
    func_corner_latitudes = &gdpasc_get_corner_latitudes;
    func_orbit_number = gdp_get_orbit_number;
    break;
  case PRJCT_INSTR_FORMAT_MKZY:
    func_scanning_angle = &mkzy_get_scanning_angle;
    break;
  case PRJCT_INSTR_FORMAT_CCD_EEV:
    func_meastype = ccd_get_meastype;
    break;
  case PRJCT_INSTR_FORMAT_MFC_BIRA:
    func_meastype= mfc_get_meastype;
    break;
  }

  // Browse fields
  for (int j=0;j<pResults->fieldsNumber;j++)
   {
     enum _prjctResults fieldtype = pResults->fieldsFlag[j];

     switch(fieldtype) {
     case PRJCT_RESULTS_SPECNO:
       register_field( (struct output_field) { .basic_fieldname = "Spec No", .resulttype = fieldtype, .format = "%#4d", .memory_type = OUTPUT_INT, .get_data = (func_void)&get_specno } );
       break;
     case PRJCT_RESULTS_NAME:
       register_field( (struct output_field) { .basic_fieldname = "Name", .memory_type = OUTPUT_STRING, .resulttype = fieldtype, .format = "%s", .get_data = (func_void)&get_name });
       break;
     case PRJCT_RESULTS_DATE_TIME:
       register_field( (struct output_field) { .basic_fieldname = "Date & time (YYYYMMDDhhmmss)", .memory_type = OUTPUT_DATETIME, .resulttype = fieldtype, .format = format_datetime, .get_data = (func_void)func_datetime });
       break;
     case PRJCT_RESULTS_DATE:
       register_field( (struct output_field) { .basic_fieldname = "Date (DD/MM/YYYY)", .memory_type = OUTPUT_DATE, .resulttype = fieldtype, .format = "%02d/%02d/%d", .get_data = (func_void)&get_date });
       break;
     case PRJCT_RESULTS_TIME:
       register_field( (struct output_field) { .basic_fieldname = "Time (hh:mm:ss)", .memory_type = OUTPUT_TIME, .resulttype = fieldtype, .format = "%02d:%02d:%02d", .get_data = (func_void)&get_time });
       break;
     case PRJCT_RESULTS_YEAR:
       register_field( (struct output_field) { .basic_fieldname = "Year", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#4d", .get_data = (func_void)&get_year });
       break;
     case PRJCT_RESULTS_JULIAN:
       register_field( (struct output_field) { .basic_fieldname = "Day number", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#5d", .get_data = (func_void)&get_julian });
       break;
     case PRJCT_RESULTS_JDFRAC:
       register_field( (struct output_field) { .basic_fieldname = "Fractional day", .memory_type = OUTPUT_DOUBLE, .resulttype = fieldtype, .format = "%#10.6lf", .get_data = (func_void)&get_frac_julian });
       break;
     case PRJCT_RESULTS_TIFRAC:
       register_field( (struct output_field) { .basic_fieldname = "Fractional time", .memory_type = OUTPUT_DOUBLE, .resulttype = fieldtype, .format = "%#20.15lf", .get_data = (func_void)func_frac_time });
       break;
     case PRJCT_RESULTS_SCANS:
       register_field( (struct output_field) { .basic_fieldname = "Scans", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#5d", .get_data = (func_void)&get_scans });
       break;
     case PRJCT_RESULTS_NREJ:
       register_field( (struct output_field) { .basic_fieldname = "Rejected", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#5d", .get_data = (func_void)&get_n_rejected });
       break;
     case PRJCT_RESULTS_TINT:
       register_field( (struct output_field) { .basic_fieldname = "Tint", .memory_type = OUTPUT_DOUBLE, .resulttype = fieldtype, .format = "%#12.6lf", .get_data = (func_void)&get_t_int });
       break;
     case PRJCT_RESULTS_SZA:
       register_field( (struct output_field) { .basic_fieldname = title_sza, .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)func_sza, .data_cols = num_sza, .column_number_format="(%c)", .column_number_alphabetic = true });
       break;
     case PRJCT_RESULTS_AZIM:
       register_field( (struct output_field) { .basic_fieldname = title_azimuth, .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)func_azimuth, .data_cols = num_azimuth, .column_number_format="(%c)", .column_number_alphabetic = true });
       break;
     case PRJCT_RESULTS_TDET:
       register_field( (struct output_field) { .basic_fieldname = "Tdet", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_tdet });
       break;
     case PRJCT_RESULTS_SKY:
       register_field( (struct output_field) { .basic_fieldname = "Sky Obs", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#2d", .get_data = (func_void)&get_sky });
       break;
     case PRJCT_RESULTS_BESTSHIFT:
       register_field( (struct output_field) { .basic_fieldname = "Best shift", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_bestshift });
       break;
     case PRJCT_RESULTS_PIXEL:
       register_field( (struct output_field) { .basic_fieldname = "Pixel number", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#5d", .get_data = (func_void)&get_pixel_number });
       break;
     case PRJCT_RESULTS_PIXEL_TYPE:
       register_field( (struct output_field) { .basic_fieldname = "Pixel type", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#5d" , .get_data = (func_void)&get_pixel_type});
       break;
     case PRJCT_RESULTS_ORBIT:
       register_field( (struct output_field) { .basic_fieldname = "Orbit number", .memory_type = OUTPUT_INT, .resulttype = fieldtype, .format = "%#8d", .get_data = (func_void)func_orbit_number });
       break;
     case PRJCT_RESULTS_LONGIT:
       if(func_corner_longitudes) { // we have pixel corners
         register_field( (struct output_field) { .basic_fieldname = "Longitude", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)func_corner_longitudes, .data_cols = 4, .column_number_format="(%d)" });
       }
       register_field( (struct output_field) { .basic_fieldname = "Longitude(pixel center)", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_longitude }); // pixel centre
       break;
     case PRJCT_RESULTS_LATIT:
       if(func_corner_latitudes) { // we have pixel corners
         register_field( (struct output_field) { .basic_fieldname = "Latitude", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)func_corner_latitudes, .data_cols = 4, .column_number_format="(%d)" });
       }
       register_field( (struct output_field) { .basic_fieldname = "Latitude(pixel center)", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_latitude });
       break;
     case PRJCT_RESULTS_ALTIT:
       register_field( (struct output_field) { .basic_fieldname = "Altitude", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_altitude });
       break;
     case PRJCT_RESULTS_CLOUD:
       register_field( (struct output_field) { .basic_fieldname = "Cloud fraction", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_cloud_fraction });
       break;
     case PRJCT_RESULTS_CLOUDTOPP:
       register_field( (struct output_field) { .basic_fieldname = "Cloud Top Pressure", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_cloud_top_pressure });
       break;
     case PRJCT_RESULTS_LOS_ZA:
       register_field( (struct output_field) { .basic_fieldname = title_los_zenith, .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)func_los_zenith, .data_cols = num_los_zenith, .column_number_format="(%c)", .column_number_alphabetic = true });
       break;
     case PRJCT_RESULTS_LOS_AZIMUTH:
       register_field( (struct output_field) { .basic_fieldname = title_los_azimuth, .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)func_los_azimuth, .data_cols = num_los_azimuth, .column_number_format="(%c)", .column_number_alphabetic = true });
       break;
     case PRJCT_RESULTS_SAT_HEIGHT:
       register_field( (struct output_field) { .basic_fieldname = "Satellite height", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)func_sat_height });
       break;
     case PRJCT_RESULTS_EARTH_RADIUS:
       register_field( (struct output_field) { .basic_fieldname = "Earth radius", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)func_earth_radius });
       break;
     case PRJCT_RESULTS_VIEW_ELEVATION:
       register_field( (struct output_field) { .basic_fieldname = "Elev. viewing angle", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_view_elevation });
       break;
     case PRJCT_RESULTS_VIEW_AZIMUTH:
       register_field( (struct output_field) { .basic_fieldname = "Azim. viewing angle", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_view_azimuth });
       break;
     case PRJCT_RESULTS_VIEW_ZENITH:
       register_field( (struct output_field) { .basic_fieldname = "Zenith viewing angle", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_view_zenith });
       break;
     case PRJCT_RESULTS_SCIA_QUALITY:
       register_field( (struct output_field) { .basic_fieldname = "SCIAMACHY Quality Flag", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#5d", .get_data = (func_void)&get_scia_quality });
       break;
     case PRJCT_RESULTS_SCIA_STATE_INDEX:
       register_field( (struct output_field) { .basic_fieldname = "SCIAMACHY State Index", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#5d", .get_data = (func_void)&get_scia_state_index });
       break;
     case PRJCT_RESULTS_SCIA_STATE_ID:
       register_field( (struct output_field) { .basic_fieldname = "SCIAMACHY State Id", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#5d", .get_data = (func_void)&get_scia_state_id });
       break;
     case PRJCT_RESULTS_STARTTIME:
       register_field( (struct output_field) { .basic_fieldname = "Start Time (hhmmss)", .memory_type = OUTPUT_TIME, .resulttype = fieldtype, .format = "%02d%02d%02d", .get_data = (func_void)&get_start_time });
       break;
     case PRJCT_RESULTS_ENDTIME:
       register_field( (struct output_field) { .basic_fieldname = "Stop Time (hhmmss)", .memory_type = OUTPUT_TIME, .resulttype = fieldtype, .format = "%02d%02d%02d", .get_data = (func_void)&get_end_time });
       break;
     case PRJCT_RESULTS_SCANNING:
       register_field( (struct output_field) { .basic_fieldname = "Scanning angle", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)func_scanning_angle });
       break;
     case PRJCT_RESULTS_FILTERNUMBER:
       register_field( (struct output_field) { .basic_fieldname = "Filter number", .memory_type = OUTPUT_INT, .resulttype = fieldtype, .format = "%#3d", .get_data = (func_void)&get_filter_number });
       break;
     case PRJCT_RESULTS_MEASTYPE:
       register_field( (struct output_field) { .basic_fieldname = "Measurement type", .memory_type = OUTPUT_INT, .resulttype = fieldtype, .format = "%#3d", .get_data = (func_void)func_meastype });
       break;
     case PRJCT_RESULTS_CCD_HEADTEMPERATURE:
       register_field( (struct output_field) { .basic_fieldname = "Head temperature", .memory_type = OUTPUT_DOUBLE, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&ccd_get_head_temperature });
       break;
     case PRJCT_RESULTS_COOLING_STATUS:
       register_field( (struct output_field) { .basic_fieldname = "Cooler status", .memory_type = OUTPUT_INT, .resulttype = fieldtype, .format = "%#5d", .get_data = (func_void)&get_cooling_status });
       break;
     case PRJCT_RESULTS_MIRROR_ERROR:
       register_field( (struct output_field) { .basic_fieldname = "Mirror status", .memory_type = OUTPUT_INT, .resulttype = fieldtype, .format = "%#5d", .get_data = (func_void)&get_mirror_error });
       break;
     case PRJCT_RESULTS_COMPASS:
       register_field( (struct output_field) { .basic_fieldname = "Compass angle", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_compass_angle });
       break;
     case PRJCT_RESULTS_PITCH:
       register_field( (struct output_field) { .basic_fieldname = "Pitch angle", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_pitch_angle });
       break;
     case PRJCT_RESULTS_ROLL:
       register_field( (struct output_field) { .basic_fieldname = "Roll angle", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_roll_angle });
       break;
     case PRJCT_RESULTS_GOME2_SCANDIRECTION:
       register_field( (struct output_field) { .basic_fieldname = "GOME2 scan direction", .memory_type = OUTPUT_INT, .resulttype = fieldtype, .format = "%#6d", .get_data = (func_void)&gome2_get_scan_direction });
       break;
     case PRJCT_RESULTS_GOME2_SAA:
       register_field( (struct output_field) { .basic_fieldname = "GOME2 SAA flag", .memory_type = OUTPUT_INT, .resulttype = fieldtype, .format = "%#6d", .get_data = (func_void)&gome2_get_saa });
       break;
     case PRJCT_RESULTS_GOME2_SUNGLINT_RISK:
       register_field( (struct output_field) { .basic_fieldname = "GOME2 sunglint risk flag", .memory_type = OUTPUT_INT, .resulttype = fieldtype, .format = "%#6d", .get_data = (func_void)&gome2_get_sunglint_risk });
       break;
     case PRJCT_RESULTS_GOME2_SUNGLINT_HIGHRISK:
       register_field( (struct output_field) { .basic_fieldname = "GOME2 sunglint high risk flag", .memory_type = OUTPUT_INT, .resulttype = fieldtype, .format = "%#6d", .get_data = (func_void)&gome2_get_sunglint_high_risk });
       break;
     case PRJCT_RESULTS_GOME2_RAINBOW:
       register_field( (struct output_field) { .basic_fieldname = "GOME2 rainbow flag", .memory_type = OUTPUT_INT, .resulttype = fieldtype, .format = "%#6d", .get_data = (func_void)&gome2_get_rainbow });
       break;
     case PRJCT_RESULTS_CCD_DIODES:
       register_field( (struct output_field) { .basic_fieldname = "Diodes", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_diodes, .data_cols = 4, .column_number_format="(%d)" });
       break;
     case PRJCT_RESULTS_CCD_TARGETAZIMUTH:
       register_field( (struct output_field) { .basic_fieldname = "Target Azimuth", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_target_azimuth });
       break;
     case PRJCT_RESULTS_CCD_TARGETELEVATION:
       register_field( (struct output_field) { .basic_fieldname = "Target Elevation", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_target_elevation });
       break;
     case PRJCT_RESULTS_SATURATED:
       register_field( (struct output_field) { .basic_fieldname = "Saturated", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#5d", .get_data = (func_void)&get_saturated_flag });
       break;
     case PRJCT_RESULTS_INDEX_ALONGTRACK:
       register_field( (struct output_field) { .basic_fieldname = "along-track index", .memory_type = OUTPUT_INT, .resulttype = fieldtype, .format = "%#6d", .get_data = (func_void)&get_alongtrack_index });
       break;
     case PRJCT_RESULTS_INDEX_CROSSTRACK:
       register_field( (struct output_field) { .basic_fieldname = "cross-track index", .memory_type = OUTPUT_INT, .resulttype = fieldtype, .format = "%#3d", .get_data = (func_void)&get_crosstrack_index });
       break;
     case PRJCT_RESULTS_OMI_GROUNDP_QF:
       register_field( (struct output_field) { .basic_fieldname = "OMI groundpixel quality flag", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#6d", .get_data = (func_void)&get_omi_groundpixelqf });
       break;
     case PRJCT_RESULTS_OMI_XTRACK_QF:
       register_field( (struct output_field) { .basic_fieldname = "OMI xtrack quality flag", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#6d", .get_data = (func_void)&get_omi_xtrackqf });
       break;
     case PRJCT_RESULTS_OMI_PIXELS_QF:
       register_field( (struct output_field) { .basic_fieldname = "OMI rejected pixels based on QF", .memory_type = OUTPUT_STRING, .resulttype = fieldtype, .format = "%-50s", .get_data = (func_void)&omi_get_rejected_pixels });
       break;
     case PRJCT_RESULTS_UAV_SERVO_BYTE_SENT:
       register_field( (struct output_field) { .basic_fieldname = "UAV servo sent position byte", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#3d", .get_data = (func_void)&get_uav_servo_byte_sent });
       break;
     case PRJCT_RESULTS_UAV_SERVO_BYTE_RECEIVED:
       register_field( (struct output_field) { .basic_fieldname = "UAV servo received position byte", .memory_type = OUTPUT_USHORT, .resulttype = fieldtype, .format = "%#3d", .get_data = (func_void)&get_uav_servo_byte_received });
       break;
     case PRJCT_RESULTS_UAV_TEMPERATURE:
       register_field( (struct output_field) { .basic_fieldname = "Temperature", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#9.3f", .get_data = (func_void)&get_uav_temperature });
       break;
     case PRJCT_RESULTS_UAV_PRESSURE:
       register_field( (struct output_field) { .basic_fieldname = "Pressure", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#9.3f", .get_data = (func_void)&get_uav_pressure });
       break;
     case PRJCT_RESULTS_PRECALCULATED_FLUXES:
       register_field( (struct output_field) { .basic_fieldname = "Precalculated flux", .memory_type = OUTPUT_FLOAT, .resulttype = fieldtype, .format = "%#12.6f", .get_data = (func_void)&get_precalculated_flux, .data_cols = 4, .column_number_format="(%d)" });
       break;
     default:
       break;
     }
   }
}

/*! \brief Helper function to initialize a new output_field in the
    \ref output_data_calib array.*/
static void register_calibration_field(struct output_field newfield) {
  struct output_field *calibfield = &output_data_calib[calib_num_fields++];
  *calibfield = newfield; // copy all contents
  if(calibfield->data_cols == 0) // data_cols = 1 as default value -> we only need to set data_cols explicitly if we want a multi-column field
    calibfield->data_cols = 1;

  // calibration field name starts with the analysis window. This is
  // useful when different analysis windows use different reference
  // spectra, and therefore have different calibration settings.

  calibfield->fieldname = strdup(newfield.basic_fieldname);
  calibfield->windowname = strdup(TabFeno[newfield.index_row][newfield.index_feno].windowName);
  calibfield->get_tabfeno = &get_tabfeno_calib;
  calibfield->get_cross_results = &get_cross_results_calib;
  calibfield->memory_type = OUTPUT_DOUBLE;
  calibfield->format = FORMAT_DOUBLE;
}

/*! \brief Register selected output fields for the reference spectrum
    calibration.*/
static void register_calibration(int kurucz_index, int index_row, int index_feno) {
  FENO *pTabFeno=&TabFeno[index_row][kurucz_index];

  register_calibration_field((struct output_field){.basic_fieldname="Wavelength", .resulttype = PRJCT_RESULTS_WAVELENGTH, .index_feno=index_feno, .index_row=index_row, .index_cross=ITEM_NONE, .get_data=(func_void)&get_wavelength_calib });
  register_calibration_field((struct output_field){.basic_fieldname="RMS", .resulttype = PRJCT_RESULTS_RMS, .index_feno=index_feno, .index_row=index_row, .index_cross=ITEM_NONE, .get_data=(func_void)&get_rms_calib });

  for (int indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++) {
    CROSS_RESULTS *pTabCrossResults = &pTabFeno->TabCrossResults[indexTabCross];
    char *symbol_name = WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName;
    char symbol_name_brackets[strlen(symbol_name)+2+1];
    sprintf(symbol_name_brackets,"(%s)",symbol_name);

    struct calibconfig {
      bool register_field;
      const char *output_name;
      const char *symbol_name;
      struct output_field fieldconfig;
    };

    struct calibconfig calibrationfields[] = {
      { pTabCrossResults->StoreSlntCol, "SlCol", symbol_name_brackets,
        { .get_data=(func_void)&get_slant_column, .resulttype = PRJCT_RESULTS_SLANT_COL} },
      { pTabCrossResults->StoreSlntErr, "SlErr", symbol_name_brackets,
        { .get_data=(func_void)&get_slant_err, .resulttype = PRJCT_RESULTS_SLANT_ERR} },
      { pTabCrossResults->StoreShift, "Shift", symbol_name_brackets,
        { .get_data=(func_void)&get_shift, .resulttype = PRJCT_RESULTS_SHIFT} },
      { pTabCrossResults->StoreShift && pTabCrossResults->StoreError, "Err Shift", symbol_name_brackets,
        { .get_data=(func_void)&get_shift_err, .resulttype = PRJCT_RESULTS_SHIFT_ERR} },
      { pTabCrossResults->StoreStretch, "Stretch", symbol_name_brackets,
        { .get_data=(func_void)&get_stretches, .data_cols=2, .column_number_format="%d", .resulttype = PRJCT_RESULTS_STRETCH} },
      { pTabCrossResults->StoreStretch && pTabCrossResults->StoreError, "Err Stretch", symbol_name_brackets,
        { .get_data=(func_void)&get_stretch_errors, .resulttype = PRJCT_RESULTS_STRETCH_ERR, .data_cols=2,.column_number_format = "%d"} },
      { pTabCrossResults->StoreScale, "Scale", symbol_name_brackets,
        { .get_data=(func_void)&get_scale, .resulttype = PRJCT_RESULTS_SCALE} },
      { pTabCrossResults->StoreScale && pTabCrossResults->StoreError, "Err Scale", symbol_name_brackets,
        { .get_data=(func_void)&get_scale_err, .resulttype = PRJCT_RESULTS_SCALE_ERR} },
      { pTabCrossResults->StoreParam, symbol_name, "",
        { .get_data=(func_void)&get_param, .resulttype = PRJCT_RESULTS_PARAM} },
      { pTabCrossResults->StoreParam && pTabCrossResults->StoreParamError, "Err ", symbol_name,
        { .get_data=(func_void)&get_param_err, .resulttype = PRJCT_RESULTS_PARAM_ERR} }
    };

    for(unsigned int i=0; i<sizeof(calibrationfields)/sizeof(calibrationfields[0]); i++) {
      if(calibrationfields[i].register_field) {
        char fieldname[strlen(calibrationfields[i].output_name) + strlen(calibrationfields[i].symbol_name) +1];
        sprintf(fieldname, "%s%s", calibrationfields[i].output_name, calibrationfields[i].symbol_name);
        struct output_field newfield = calibrationfields[i].fieldconfig; // copy all non-zero fields from 'fieldconfig'
        newfield.basic_fieldname=fieldname;
        newfield.index_feno=index_feno;
        newfield.index_row=index_row;
        newfield.index_cross=indexTabCross;
        register_calibration_field(newfield);
      }
    }
  }
}

/*! \brief Register all output related to analysis (or run
    calibration) results.

  This function registers all results of the fit: global results such
  as the RMS, by calling \ref register_analysis_output, as well as the fitted parameters for each cross section
  in the fit, by calling \ref register_cross_results.

  \param [in] pEngineContext   structure including information on project options
*/
static void OutputRegisterParam(const ENGINE_CONTEXT *pEngineContext)
{
  int indexFenoColumn = 0;
  // if using OMI: increase indexFenoColumn until we find a used track, otherwise: use track 0
  for(indexFenoColumn=0; indexFenoColumn<ANALYSE_swathSize; indexFenoColumn++) {
    if ( pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI ||
         pEngineContext->project.instrumental.omi.omiTracks[indexFenoColumn] )
      break;
  }

  const PRJCT_RESULTS *pResults= &pEngineContext->project.asciiResults;

  // Browse analysis windows
  for (int indexFeno=0;indexFeno<NFeno;indexFeno++) {
    const FENO *pTabFeno=&TabFeno[indexFenoColumn][indexFeno];
    if(!outputRunCalib && !pTabFeno->hidden) {
      // run analysis: skip calibration settings
      register_analysis_output(pResults, indexFeno, ITEM_NONE, pTabFeno->windowName);
      register_cross_results(pResults, pTabFeno, indexFeno, ITEM_NONE, pTabFeno->windowName);
    }
    else if (outputRunCalib && pTabFeno->hidden) {
      // run calibration: use TabFeno with calibration settings, register parameters
      // for each calibration window
      for (int indexWin=0; indexWin<KURUCZ_buffers[indexFenoColumn].Nb_Win; indexWin++) {
        char window_name[MAX_ITEM_NAME_LEN+1];
        sprintf(window_name,"RunCalib(%d)",indexWin+1);
        register_analysis_output(pResults, indexFeno, indexWin, window_name);
        register_cross_results(pResults, pTabFeno, indexFeno, indexWin, window_name);
      }
    }
  }
}

/*! \brief helper function to initialize an output_field containing analysis results. */
static void register_analysis_field(const struct output_field* fieldcontent, int index_feno, int index_calib, int index_cross, const char *window_name, const char *symbol_name) {

  struct output_field *newfield = &output_data_analysis[output_num_fields++];
  *newfield = *fieldcontent;
  char *full_fieldname = malloc(strlen(newfield->basic_fieldname) + strlen(symbol_name) +1);
  sprintf(full_fieldname, "%s%s", newfield->basic_fieldname, symbol_name);
  newfield->fieldname = full_fieldname;
  newfield->windowname = strdup(window_name);
  newfield->data = NULL;
  if (newfield->data_cols == 0) // data_cols = 1 as a default
    newfield->data_cols = 1;
  newfield->index_feno = index_feno;
  newfield->get_tabfeno = (outputRunCalib) ? &get_tabfeno_calib : &get_tabfeno_analysis;
  newfield->index_calib = (outputRunCalib) ? index_calib : ITEM_NONE;
  newfield->index_cross = index_cross;
  newfield->get_cross_results = (outputRunCalib) ? &get_cross_results_calib : &get_cross_results_analysis;
}

/*! \brief Register output fields related to overall analysis (or run
    calibration) results (per analysis window).

  \param [in] pResults PRCJT_RESULTS array of containing output fields selected by the user.

  \param [in] indexFeno index of the analysis window.

  \param [in] index_calib in case of run_calibration: index of the calibration window

  \param [in] windowName name of the analysis window, with suffix "."
*/
static void register_analysis_output(const PRJCT_RESULTS *pResults, int indexFeno, int index_calib, const char *windowName) {

  struct outputconfig analysis_infos[] = {
    { (outputRunCalib) ? -1 : PRJCT_RESULTS_REFZM, // no REFZM in "run calibration" mode
      { .basic_fieldname = "RefZm", .format = FORMAT_FLOAT, .memory_type = OUTPUT_FLOAT, .get_data = (func_void) &get_refzm} },
    { (outputRunCalib) ? -1 : PRJCT_RESULTS_REFNUMBER, // no REFNUMBER in "run calibration" mode
      { .basic_fieldname = "RefNumber", .format = FORMAT_INT, .memory_type = OUTPUT_INT, .get_data = (func_void) &get_refnumber} },
    { (outputRunCalib) ? -1 : PRJCT_RESULTS_REFSHIFT, // no REFSHIFT in "run calibration" mode
      { .basic_fieldname = "Ref2/Ref1 Shift", .format = FORMAT_FLOAT, .memory_type = OUTPUT_FLOAT, .get_data = (func_void) &get_ref_shift} },
    { (outputRunCalib) ? -1 : PRJCT_RESULTS_SPIKES, // no spike removal in "run calibration" mode
      { .basic_fieldname = "Spike removal", .format = "%-50s", .memory_type = OUTPUT_STRING, .get_data = (func_void) &get_spikes } },
    { (outputRunCalib) ? -1 : PRJCT_RESULTS_OMI_PIXELS_QF,  // no pixel quality flags in "run calibration" mode
      { .basic_fieldname = "omiRejPixelsQF", .format = "%-50s", .memory_type = OUTPUT_STRING, .get_data = (func_void) &get_omi_rejected_pixels } },
    { PRJCT_RESULTS_CHI,
      { .basic_fieldname = "Chi", .format = FORMAT_DOUBLE, .memory_type = OUTPUT_DOUBLE,
        .get_data = (outputRunCalib) ? (func_void) &get_chisquare_calib : (func_void) &get_chisquare} },
    { PRJCT_RESULTS_RMS,
      { .basic_fieldname = "RMS", .format = FORMAT_DOUBLE, .memory_type = OUTPUT_DOUBLE,
        .get_data = (outputRunCalib) ? (func_void) &get_rms_calib : (func_void) &get_rms} },
    { PRJCT_RESULTS_ITER,
      { .basic_fieldname = "iter", .format = FORMAT_INT, .memory_type = OUTPUT_INT,
        .get_data = (outputRunCalib) ? (func_void) &get_n_iter_calib : (func_void) &get_n_iter} }
  };

  size_t arr_length = sizeof(analysis_infos)/sizeof(analysis_infos[0]);

  for(int i = 0; i<pResults->fieldsNumber; i++) {
    enum _prjctResults indexField = pResults->fieldsFlag[i];
    struct outputconfig *output =  (struct outputconfig *) lfind(&indexField, analysis_infos, &arr_length, sizeof(analysis_infos[0]), &compare_record);
    if(output) {
      output->field.get_tabfeno = &get_tabfeno_analysis;
      output->field.resulttype = output->type;
      register_analysis_field(&output->field, indexFeno, index_calib, ITEM_NONE, windowName, "");
    }
  }
}

/*! \brief Register output fields related to analysis (or run
    calibration) results of cross sections (per analysis window).

  \param [in] pResults PRCJT_RESULTS array of containing output fields
  selected by the user.

  \param [in] pTabFeno FENO structure describing the analysis window.

  \param [in] indexFeno index of the analysis window.

  \param [in] index_calib in case of run_calibration: index of the
  calibration window

  \param [in] windowName name of the analysis window, with suffix "."
*/
static void register_cross_results(const PRJCT_RESULTS *pResults, const FENO *pTabFeno, int indexFeno, int index_calib, const char *windowName) {
  struct analysis_output {
    bool register_field;
    const char *symbol_name;
    const struct output_field fieldcontent;
  };

  // Fitted parameters
  for (int indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
    {
      const CROSS_RESULTS *pTabCrossResults =&pTabFeno->TabCrossResults[indexTabCross];
      char symbolName[MAX_ITEM_NAME_LEN+1];    // the name of a symbol
      sprintf(symbolName,"(%s)",WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName);

      const char *xs_file = WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].crossFileName;
      // don't add "cross_attribs_file" attribute if crossFileName is not a string (e.g. for polynomial components)
      bool has_file = ( xs_file != NULL && strlen(xs_file) );

      const struct field_attribute cross_attribs_file[1] = {{ "Cross section file",
                                                              xs_file }};

      const struct analysis_output symbol_fitparams[] = {
        { pTabCrossResults->indexAmf!=ITEM_NONE && pTabCrossResults->StoreAmf, symbolName,
          { .basic_fieldname = "AMF", .format = FORMAT_FLOAT, .resulttype = PRJCT_RESULTS_AMF, .memory_type = OUTPUT_FLOAT, .get_data = (func_void) &get_amf} },
        { pTabCrossResults->indexAmf!=ITEM_NONE && pTabCrossResults->StoreVrtCol, symbolName,
          { .basic_fieldname = "VCol", .format = FORMAT_DOUBLE, .resulttype = PRJCT_RESULTS_VERT_COL, .memory_type = OUTPUT_DOUBLE, .get_data = (func_void) &get_vrt_col} },
        { pTabCrossResults->indexAmf!=ITEM_NONE && pTabCrossResults->StoreVrtErr, symbolName,
          { .basic_fieldname = "VErr", .format = FORMAT_DOUBLE, .resulttype = PRJCT_RESULTS_VERT_ERR, .memory_type = OUTPUT_DOUBLE, .get_data = (func_void) &get_vrt_err} },
        { pTabCrossResults->StoreSlntCol, symbolName,
          { .basic_fieldname = "SlCol", .format = FORMAT_DOUBLE, .resulttype = PRJCT_RESULTS_SLANT_COL, .memory_type = OUTPUT_DOUBLE, .get_data = (func_void) &get_slant_column, .attributes = has_file ? copy_attributes(cross_attribs_file, sizeof(cross_attribs_file)/sizeof(cross_attribs_file[0])) : NULL, .num_attributes = has_file ? 1 : 0} },
        { pTabCrossResults->StoreSlntErr, symbolName,
          { .basic_fieldname = "SlErr", .format = FORMAT_DOUBLE, .resulttype = PRJCT_RESULTS_SLANT_ERR, .memory_type = OUTPUT_DOUBLE, .get_data = (func_void) &get_slant_err} },
        { pTabCrossResults->StoreShift, symbolName,
          { .basic_fieldname = "Shift", .format = FORMAT_DOUBLE, .resulttype = PRJCT_RESULTS_SHIFT, .memory_type = OUTPUT_DOUBLE, .get_data = (func_void) &get_shift} },
        { pTabCrossResults->StoreShift && pTabCrossResults->StoreError, symbolName,
          { .basic_fieldname = "Err Shift", .format = FORMAT_DOUBLE, .resulttype = PRJCT_RESULTS_SHIFT_ERR, .memory_type = OUTPUT_DOUBLE, .get_data = (func_void) &get_shift_err} },
        { pTabCrossResults->StoreStretch, symbolName,
          { .basic_fieldname = "Stretch", .format = FORMAT_DOUBLE, .resulttype = PRJCT_RESULTS_STRETCH, .memory_type = OUTPUT_DOUBLE, .get_data = (func_void) &get_stretches,
            .data_cols=2, .column_number_format="%d"} },
        { pTabCrossResults->StoreStretch && pTabCrossResults->StoreError, symbolName,
          { .basic_fieldname = "Err Stretch", .format = FORMAT_DOUBLE, .resulttype = PRJCT_RESULTS_STRETCH_ERR, .memory_type = OUTPUT_DOUBLE, .get_data = (func_void) &get_stretch_errors,
            .data_cols=2, .column_number_format="%d"} },
        { pTabCrossResults->StoreScale, symbolName,
          { .basic_fieldname = "Scale", .format = FORMAT_DOUBLE, .resulttype = PRJCT_RESULTS_SCALE, .memory_type = OUTPUT_DOUBLE, .get_data = (func_void) &get_scales,
            .data_cols=2, .column_number_format="%d"} },
        { pTabCrossResults->StoreScale && pTabCrossResults->StoreError, symbolName,
          { .basic_fieldname = "Err Scale", .format = FORMAT_DOUBLE, .resulttype = PRJCT_RESULTS_SCALE_ERR, .memory_type = OUTPUT_DOUBLE, .get_data = (func_void) &get_scale_err,
            .data_cols=2, .column_number_format="%d"} },
        { pTabCrossResults->StoreParam, "",
          { .basic_fieldname = WorkSpace[pTabFeno->TabCross[indexTabCross].Comp].symbolName, .resulttype = PRJCT_RESULTS_PARAM, .format = FORMAT_DOUBLE, .memory_type = OUTPUT_DOUBLE, .get_data = (func_void) &get_param} },
        { pTabCrossResults->StoreParam && pTabCrossResults->StoreParamError, symbolName,
          { .basic_fieldname = "Err", .format = FORMAT_DOUBLE, .resulttype = PRJCT_RESULTS_PARAM_ERR, .memory_type = OUTPUT_DOUBLE, .get_data = (func_void) &get_param_err} }
      };

      for(unsigned int i=0; i<sizeof(symbol_fitparams)/sizeof(symbol_fitparams[0]); i++) {
        if(symbol_fitparams[i].register_field) {
          register_analysis_field(&symbol_fitparams[i].fieldcontent, indexFeno, index_calib, indexTabCross, windowName, symbol_fitparams[i].symbol_name);
        }
      }
    }

  // register correlation and covariance fields
  for(int i = 0; i<pResults->fieldsNumber; i++) {
    enum _prjctResults indexField = pResults->fieldsFlag[i];
    if ( indexField == PRJCT_RESULTS_COVAR || indexField == PRJCT_RESULTS_CORR) {
      const CROSS_REFERENCE *TabCross = pTabFeno->TabCross;
      for (int indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
        if (TabCross[indexTabCross].IndSvdA>0)
          for (int indexTabCross2=0;indexTabCross2<indexTabCross;indexTabCross2++)
            if (TabCross[indexTabCross2].IndSvdA>0)
              {
                char symbolName[MAX_ITEM_NAME_LEN+1];
                sprintf(symbolName,"(%s,%s)",
                        WorkSpace[TabCross[indexTabCross2].Comp].symbolName,
                        WorkSpace[TabCross[indexTabCross].Comp].symbolName);
                const char *fieldname = NULL;
                func_void get_data = NULL;
                if (indexField == PRJCT_RESULTS_COVAR) {
                  fieldname = "Covar";
                  get_data = (func_void)&get_covar;
                } else if (indexField == PRJCT_RESULTS_CORR) {
                  fieldname = "Corr";
                  get_data = (func_void)&get_corr;
                } struct output_field newfield = { .resulttype = indexField,
                                                   .basic_fieldname = fieldname,
                                                   .format = FORMAT_DOUBLE,
                                                   .memory_type = OUTPUT_DOUBLE,
                                                   .get_data = get_data,
                                                   .index_cross2 = indexTabCross2 };
                register_analysis_field( &newfield, indexFeno, index_calib, indexTabCross, windowName, symbolName);
              }
    }
  }
}

/*! \brief Set up #output_data_analysis and #output_data_calib to
    store data for output according to the user's settings in
    pEngineContext.

    \sa register_calibration, OutputRegisterFields,
    OutputRegisterParam, OutputRegisterFluxes */
RC OUTPUT_RegisterData(const ENGINE_CONTEXT *pEngineContext)
{
  PRJCT_RESULTS *pResults;
  INDEX indexFenoColumn;
  RC rc;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("OUTPUT_RegisterData",DEBUG_FCTTYPE_FILE);
#endif

  // Initializations

  const PROJECT *pProject=&pEngineContext->project;
  pResults=(PRJCT_RESULTS *)&pProject->asciiResults;
  outputCalibFlag=outputRunCalib=0;
  rc=ERROR_ID_NO;

  if (pProject->asciiResults.analysisFlag || pProject->asciiResults.calibFlag)
    {
      if (THRD_id==THREAD_TYPE_ANALYSIS)
        {
          // Save information on the calibration

          if (pResults->calibFlag) {
            for (indexFenoColumn=0;indexFenoColumn<ANALYSE_swathSize;indexFenoColumn++) {
              if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) ||
                  pEngineContext->project.instrumental.omi.omiTracks[indexFenoColumn]) {

                int indexFenoK=ITEM_NONE;
                for (int indexFeno=0;indexFeno<NFeno && indexFenoK==ITEM_NONE; ++indexFeno) {
                  if (TabFeno[indexFenoColumn][indexFeno].hidden) {
                    indexFenoK=indexFeno;
                  }
                }
                if (indexFenoK != ITEM_NONE) {
                  for (int indexFeno=0;indexFeno<NFeno;indexFeno++) {
                    if ( !TabFeno[indexFenoColumn][indexFeno].hidden
                         && KURUCZ_buffers[indexFenoColumn].KuruczFeno != NULL 
                         && (TabFeno[indexFenoColumn][indexFeno].useKurucz == ANLYS_KURUCZ_REF_AND_SPEC
                             || TabFeno[indexFenoColumn][indexFeno].useKurucz == ANLYS_KURUCZ_REF ) ) {
                      register_calibration(indexFenoK, indexFenoColumn, indexFeno);
                    }
                  }
                }
              }
            }
          }
        }
      // Run calibration on measurement spectra

      else if (THRD_id==THREAD_TYPE_KURUCZ)
        outputRunCalib++;

      OutputRegisterFields(pEngineContext);                                       // do not depend on swath size
      OutputRegisterParam(pEngineContext);
      OutputRegisterFluxes(pEngineContext);                                       // do not depend on swath size
    }

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("OUTPUT_RegisterData",rc);
#endif

  return rc;
}

/*! \brief Save results of the last processed record. */
static void OutputSaveRecord(const ENGINE_CONTEXT *pEngineContext,INDEX indexFenoColumn) {

  RECORD_INFO *pRecordInfo =(RECORD_INFO *)&pEngineContext->recordInfo;

  if (pEngineContext->project.asciiResults.analysisFlag)
    {
      int index_record = outputNbRecords++;

      for(unsigned int i=0; i<output_num_fields; i++) {
        save_analysis_data(&output_data_analysis[i], index_record, pEngineContext, indexFenoColumn);
      }
      outputRecords[index_record].specno = pEngineContext->indexRecord;
      outputRecords[index_record].year=(int)pRecordInfo->present_datetime.thedate.da_year;
      outputRecords[index_record].month=(int)pRecordInfo->present_datetime.thedate.da_mon;
      outputRecords[index_record].day=(int)pRecordInfo->present_datetime.thedate.da_day;
      outputRecords[index_record].longit=(float)pRecordInfo->longitude;
      outputRecords[index_record].latit=(float)pRecordInfo->latitude;
    }
 }

/*! \brief Build the output file name using the selected observation site.

 \param [in] pEngineContext   structure including information on project options
 \param [in] year, month      current date to process (monthly files are created)
 \param [in] indexSite        index of the observation site

 \param [out] outputFileName  the name of the output file
*/
void OutputBuildSiteFileName(const ENGINE_CONTEXT *pEngineContext,char *outputFileName,int year,int month,INDEX indexSite)
 {
  // Declarations

  PROJECT             *pProject;                                                // pointer to project data
  PRJCT_RESULTS *pResults;                                                // pointer to results part of project
  char                *fileNamePtr;                                             // character pointers used for building output file name

  // Initializations

  pProject=(PROJECT *)&pEngineContext->project;
  pResults=(PRJCT_RESULTS *)&pProject->asciiResults;

  // Build the complete output path

  strcpy(outputFileName,pResults->path);

  if ((fileNamePtr=strrchr(outputFileName,PATH_SEP))==NULL)                     // extract output file name without path
   fileNamePtr=outputFileName;
  else
   fileNamePtr++;

  sprintf(fileNamePtr,"%s_%04d%02d",
          (indexSite!=ITEM_NONE) ? SITES_itemList[indexSite].abbrev : "XX",year,month);
 }

/*! remove filename extension ".ext" from "[...]/filename.ext", if
  filename entered by the user ends with one of the possible file
  extensions for Qdoas output files (".ASC"/".HE5").

  \param[in] filename filename, will be updated in-place if an
             extension is found.
 */
void remove_extension(char *filename) {
  char * extension = strrchr(filename, '.');
  if (extension != NULL) {
    // compare to known output file extensions
    size_t num_extensions = sizeof(output_file_extensions)/sizeof(output_file_extensions[0]);
    for(size_t i=0; i < num_extensions; ++i) {
      if (!strcasecmp(extension, output_file_extensions[i])) {
        *extension ='\0';
        break;
      }
    }
  }
}

/*! For satellite measurements, get the date of the orbit file */
static RC get_orbit_date(const ENGINE_CONTEXT *pEngineContext, int *orbit_year, int *orbit_month, int *orbit_day) {
  RC rc = ERROR_ID_NO;

  switch (pEngineContext->project.instrumental.readOutFormat) {
  case PRJCT_INSTR_FORMAT_OMI:
    rc = OMI_get_orbit_date(orbit_year, orbit_month, orbit_day);
    break;
  case PRJCT_INSTR_FORMAT_GDP_BIN:
    rc = GDP_BIN_get_orbit_date(orbit_year, orbit_month, orbit_day);
    break;
  case PRJCT_INSTR_FORMAT_GDP_ASCII:
    GDP_ASC_get_orbit_date(orbit_year, orbit_month, orbit_day);
    break;
  case PRJCT_INSTR_FORMAT_GOME2:
    GOME2_get_orbit_date(orbit_year, orbit_month, orbit_day);
    break;
  case PRJCT_INSTR_FORMAT_SCIA_PDS:
    SCIA_get_orbit_date(orbit_year, orbit_month, orbit_day);
    break;
  default:
    // we should never get here:
    assert(false && "Can't create year/month/day directories: get_orbit_date() "
           "is not implemented for the file format you are using.");
    break;
  }
  return rc;
}

/*! For satellite measurements, automatically build a file name for
  the output file and create the necessary directory structure.

  \param [in] pEngineContext structure including information on
  project options

  \param [in,out] outputFileName the original output file name to
  complete

  \retval ERROR_ID_NOTHING_TO_SAVE if there is nothing to save,
  \retval ERROR_ID_NO otherwise
*/
RC OutputBuildFileName(const ENGINE_CONTEXT *pEngineContext,char *outputPath)
{
  const PROJECT *pProject = &pEngineContext->project;
  const PRJCT_RESULTS *pResults = &pProject->asciiResults;

  RC rc=ERROR_ID_NO;

  const OUTPUT_INFO* pOutput=&outputRecords[0];

  // Build the complete output path
  strcpy(outputPath,pResults->path);

  char *fileNameStart;
  // extract output file name without path
  if ((fileNameStart=strrchr(outputPath,PATH_SEP))==NULL) {
    // outputPath doesn't contain any path separators -> only contains a file name (in current working dir)
    fileNameStart=outputPath;
  } else {
    // skip the separator character
    ++fileNameStart;
  }

  int satelliteFlag=((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
                 (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
                 (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
                 (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI) ||
                 (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2));

  if ((!strlen(fileNameStart) || !strcasecmp(fileNameStart,"automatic")) &&
      ((satelliteFlag && ((pProject->spectra.mode!=PRJCT_SPECTRA_MODES_OBSLIST) || (pProject->spectra.radius<=1.))) ||
       (!satelliteFlag && (pResults->fileNameFlag || (SITES_GetIndex(pProject->instrumental.observationSite)==ITEM_NONE))))) {

    const char *inputFileName;

    if ((inputFileName=strrchr(pEngineContext->fileInfo.fileName,PATH_SEP))==NULL) {
      inputFileName=pEngineContext->fileInfo.fileName;
    } else {
      inputFileName++;
    }

    if (satelliteFlag && pResults->dirFlag) {
      // get date for the current orbit file
      int orbit_year, orbit_month, orbit_day;
      rc = get_orbit_date(pEngineContext, &orbit_year, &orbit_month, &orbit_day);
      if (rc != ERROR_ID_NO) {
        return rc;
      }

      // Create 'year' directory
      int nwritten = sprintf(fileNameStart,"%d%c", orbit_year, PATH_SEP);
      fileNameStart += nwritten;
#if defined WIN32
      mkdir(outputPath);
#else
      mkdir(outputPath,0755);
#endif
      // Create 'month' directory
      nwritten = sprintf(fileNameStart,"%02d%c", orbit_month, PATH_SEP);
      fileNameStart += nwritten;
#if defined WIN32
      mkdir(outputPath);
#else
      mkdir(outputPath,0755);
#endif
      // Create 'day' directory
      nwritten = sprintf(fileNameStart,"%02d%c", orbit_day, PATH_SEP);
      fileNameStart += nwritten;
#if defined WIN32
      mkdir(outputPath);
#else
      mkdir(outputPath,0755);
#endif
    }

    // Build output file name
    if ( pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
      sprintf(fileNameStart,"SCIA_%d%02d%02d_%05d",pOutput->year,pOutput->month,pOutput->day,pEngineContext->recordInfo.scia.orbitNumber);
    else {
      sprintf(fileNameStart,"%s",inputFileName);

      // remove original level1 filename extension, if present
      char *extension_start=strrchr(fileNameStart, '.');
      if (extension_start != NULL) {
        *extension_start ='\0';
      }
    }

  } else { // user-chosen filename
    remove_extension(fileNameStart);
  }
  return rc;
}

RC open_output_file(const ENGINE_CONTEXT *pEngineContext, char *outputFileName)
{
  if (pEngineContext->project.asciiResults.calibFlag)
    save_calibration();

  switch(selected_format) {
  case ASCII:
    return ascii_open(pEngineContext, outputFileName);
    break;
  case HDFEOS5:
    return hdfeos5_open(pEngineContext, outputFileName);
    break;
  case NETCDF:
    return netcdf_open(pEngineContext, outputFileName);
    break;
  default:
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_BAD_FORMAT);
  }
}

void output_close_file(void) {
  switch(selected_format) {
  case ASCII:
    ascii_close_file();
    break;
  case HDFEOS5:
    hdfeos5_close_file();
    break;
  case NETCDF:
    netcdf_close_file();
    break;
  }
}

void output_write_data(const bool selected_records[]) {
  switch(selected_format) {
  case ASCII:
    ascii_write_analysis_data(selected_records, outputNbRecords);
    break;
  case HDFEOS5:
    hdfeos5_write_analysis_data(selected_records, outputNbRecords, outputRecords);
    break;
  case NETCDF:
    netcdf_write_analysis_data(selected_records, outputNbRecords, outputRecords);
    break;
  default:
    assert(false);
    break;
  }
}

/*! \brief Create an automatic output file, and write the selected
    records to it.

  \param [in] selected_records boolean array of length
  outputNbRecords

  \param[in] year, month, site_index date and location to be used in
  the directory structure

  \param[in] pEngineContext
  */
RC output_write_automatic_file(const bool selected_records[], int year, int month, int site_index, const ENGINE_CONTEXT *pEngineContext ) {
  char filename[MAX_ITEM_TEXT_LEN+1] = {0};
  OutputBuildSiteFileName(pEngineContext,filename,year, month, site_index);
  RC rc=open_output_file(pEngineContext,filename);
  if ( !rc ) {
    output_write_data(selected_records);
    output_close_file();
  } else {
    rc =  ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_OPEN, filename);
  }
  return rc;
}

/* \brief Check if the configured Output Path is valid.

 for user-chosen output file name: check if we can write to the file
 for automatic file names: just check if the root directory exists.

 \retval ERROR_ID_FILE_OPEN We can not open the configured file.
 \retval ERROR_ID_DIR_NOT_FOUND The directory selected for automatic output doesn't exist.
 \retval ERROR_ID_NONE Ok

*/
RC OUTPUT_CheckPath(const PRJCT_RESULTS *pResults) {

  RC rc = ERROR_ID_NO;

  const char *output_path = pResults->path;
  const char *output_path_end = strrchr(output_path, PATH_SEP);
  const char *fileName;
  if (output_path_end == NULL) {
    fileName = output_path;
  } else {
    fileName = output_path_end + 1; // skip path separator
  }

  if ( strlen(fileName) != 0 && strcmp("automatic", fileName) != 0 ) {
    // if an output filename is specified and it is not 'automatic':
    // check if we can write to the file

    // create the filename as it will be generated in the output routines:
    const char *extension = output_file_extensions[pResults->file_format];
    size_t namelen = strlen(output_path) + strlen(extension);
    char filename[1+namelen];
    strcpy(filename, output_path);
    remove_extension(filename); // if user has added a filename extension .asc/.he5/..., remove it
    strcat(filename, extension); // add proper extension
    FILE *test = fopen(filename, "a");

    if (test == NULL) {
      rc = ERROR_SetLast("Output Path configuration error" , ERROR_TYPE_FATAL, ERROR_ID_FILE_OPEN, filename);
    } else {
      fclose(test);
      switch (pResults->file_format) {
      case HDFEOS5:
        // for HDFEOS-5, we do not append to existing files
        // -> if the file already exists, its size should be 0
        rc = hdfeos5_allow_file(filename);
        break;
      case NETCDF:
        rc = netcdf_allow_file(filename, pResults);
        break;
      case ASCII: // writing to ASCII is always ok (append)
        break;
      }
    }
  } else // otherwise: automatic output files: file names will be generated based on the input files.
         // We can only check if the target directory exists

    if (output_path_end != NULL) {
      // only test when a path is specified using a path separator;
      char path[MAX_ITEM_TEXT_LEN + 1];
      size_t pathlen = output_path_end - output_path;
      strncpy(path, output_path, pathlen);
      path[pathlen]='\0';
      if ( STD_IsDir(path) != 1 ) {
        rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_DIR_NOT_FOUND, path, ", please create the directory or change the project's Output Path.");
      }
    } else {
      // when no path separator is given and the user has chosen to
      // generate year/month/day directories, automatic output files
      // will be generated as "/year/month/day/..." in the root
      // directory -> will not work, throw error
      rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_DIR_NOT_FOUND, "\"\"", ", please specify a directory in the project's Output Path");
    }

  return rc;
}

RC OUTPUT_FlushBuffers(ENGINE_CONTEXT *pEngineContext)
{
  const PROJECT *pProject = &pEngineContext->project; // pointer to project data
  const PRJCT_RESULTS *pResults = &pProject->asciiResults; // pointer to results part of project

  selected_format = pResults->file_format;

  RC rc=ERROR_ID_NO;
  char outputFileName[MAX_ITEM_TEXT_LEN+1] = {0};

  // select records for output according to date/site
  bool selected_records[outputNbRecords];

  if ((pResults->analysisFlag || pResults->calibFlag)
      && outputNbRecords
      && !(rc=OutputBuildFileName(pEngineContext,outputFileName))) {
    char *ptr = strrchr(outputFileName,PATH_SEP);
    if ( ptr==NULL )
      ptr=outputFileName;
    else
      ptr++;

    if ( strcasecmp(ptr,"automatic") != 0) {
      // - we have a user-specified filename (not 'automatic'), or
      //
      // the complete filename was already built in 'OutputBuildFileName' because we have
      //
      // - satellite measurements, not in overpass mode, or
      //
      // - groundbased measurements, not in "observation site mode"

      rc = open_output_file(pEngineContext,outputFileName);
      if (!rc ) {
        for(unsigned int i=0; i<outputNbRecords; i++)
          selected_records[i] = true; // select all records
        output_write_data(selected_records);
        output_close_file();
      } else {
        rc=ERROR_SetLast("OUTPUT_FlushBuffers",ERROR_TYPE_FATAL,ERROR_ID_FILE_OPEN,outputFileName);
      }
    } else {
      // -------------------
      // FULL AUTOMATIC MODE
      // -------------------
      for(unsigned int i=0; i<outputNbRecords; i++)
        selected_records[i] = false;

      // satellite, overpasses: records are distributed using information on the measurement date and the geolocation
      if ((pProject->spectra.mode==PRJCT_SPECTRA_MODES_OBSLIST) && (pProject->spectra.radius>1.)) {
        for (int indexSite=0;indexSite<SITES_itemN;indexSite++) {
          unsigned int indexRecord=0;
          OBSERVATION_SITE *pSite=&SITES_itemList[indexSite];

          while ((indexRecord<outputNbRecords) && !rc) {
            int oldYear=outputRecords[indexRecord].year;
            int oldMonth=outputRecords[indexRecord].month;

            int nbRecords=0;
            while( indexRecord<outputNbRecords
                   && (outputRecords[indexRecord].year==oldYear || outputRecords[indexRecord].month==oldMonth) ){
              if (THRD_GetDist((double)outputRecords[indexRecord].longit,(double)outputRecords[indexRecord].latit,
                               (double)pSite->longitude,(double)pSite->latitude)<=(double)pProject->spectra.radius) {
                selected_records[indexRecord] = true;
                nbRecords++;
                indexRecord++;
              }
            }
            if (nbRecords) {
              rc = output_write_automatic_file(selected_records, oldYear, oldMonth, indexSite, pEngineContext);
            }
          }
        }
      } else {
        // Groundbased, records are saved using the information on the date only
        unsigned int indexRecord=0;
        int indexSite=SITES_GetIndex(pProject->instrumental.observationSite);

        while (indexRecord<outputNbRecords) {
          int oldYear=outputRecords[indexRecord].year;
          int oldMonth=outputRecords[indexRecord].month;

          int nbRecords = 0;
          while( indexRecord<outputNbRecords
                 && (outputRecords[indexRecord].year==oldYear || outputRecords[indexRecord].month==oldMonth) ) {
            selected_records[indexRecord] = true;
            indexRecord++;
            nbRecords++;
          }

          if (nbRecords) {
            rc = output_write_automatic_file(selected_records, oldYear, oldMonth, indexSite, pEngineContext);
          }
        }
      }
    }
  }

  outputNbRecords=0;
  pEngineContext->lastSavedRecord=0;

  return rc;
}

/*! \brief Save all calibration data to the calibration output buffers.*/
void save_calibration(void) {
  for(unsigned int i=0; i<calib_num_fields; i++) {
    struct output_field *cur_field = &output_data_calib[i];
    int nbWin = KURUCZ_buffers[cur_field->index_row].Nb_Win;
    for (int indexWin=0;indexWin<nbWin;indexWin++) {
      save_calib_data(cur_field, indexWin);
    }
  }
}


RC OUTPUT_SaveResults(ENGINE_CONTEXT *pEngineContext,INDEX indexFenoColumn)
 {
  // Declarations

  CROSS_RESULTS *pTabCrossResults;
  INDEX indexFeno,indexTabCross,i;
  double *Spectrum;
  FENO *pTabFeno;
  RC rc;

  // Initializations

  const RECORD_INFO *pRecordInfo=&pEngineContext->recordInfo;
  rc=ERROR_ID_NO;

  // AMF computation

  if (OUTPUT_AmfSpace!=NULL) {

    for (indexFeno=0;indexFeno<NFeno;indexFeno++) {
      pTabFeno=&TabFeno[indexFenoColumn][indexFeno];
      
      if ((THRD_id!=THREAD_TYPE_KURUCZ) && !pTabFeno->hidden && !pTabFeno->rcKurucz) {
        for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++) {
          pTabCrossResults=&pTabFeno->TabCrossResults[indexTabCross];
          
          if (pTabCrossResults->indexAmf!=ITEM_NONE) {
            if (OutputGetAmf(pTabCrossResults,pRecordInfo->Zm,pRecordInfo->Tm,&pTabCrossResults->Amf)) {
              rc=ERROR_SetLast("OutputSaveResults",ERROR_TYPE_WARNING,ERROR_ID_AMF,pRecordInfo->Zm,OUTPUT_AmfSpace[pTabCrossResults->indexAmf].amfFileName);
            } else if (pTabCrossResults->Amf!=(double)0.) {
              pTabCrossResults->VrtCol=(pTabCrossResults->SlntCol+pTabCrossResults->ResCol)/pTabCrossResults->Amf;
              pTabCrossResults->VrtErr=pTabCrossResults->SlntErr/pTabCrossResults->Amf;
            }
          }
        }
      }
    }
  }

  // Rebuild spectrum for fluxes and color indexes computation

  if ((pRecordInfo->NSomme!=0) && (pRecordInfo->TotalExpTime!=(double)0.)) {
    Spectrum=(double *)pEngineContext->buffers.spectrum;

    for (i=0;i<NDET;i++)
      Spectrum[i]*=(double)pRecordInfo->NSomme/pRecordInfo->TotalExpTime;
  }

  if (outputNbRecords<pEngineContext->recordNumber)
    OutputSaveRecord(pEngineContext,indexFenoColumn);

  // Results safe keeping

  pEngineContext->lastSavedRecord=pEngineContext->indexRecord;

  // Return

  return rc;
 }

// ====================
// RESOURCES MANAGEMENT
// ====================

/* Return the byte size of a datatype. */
size_t output_get_size(enum output_datatype datatype) {
  switch(datatype) {
  case OUTPUT_STRING:
    return sizeof(char*);
    break;
  case OUTPUT_SHORT:
    return sizeof(short);
    break;
  case OUTPUT_USHORT:
    return sizeof(unsigned short);
    break;
  case OUTPUT_FLOAT:
    return sizeof(float);
    break;
  case OUTPUT_INT:
    return sizeof(int);
    break;
  case OUTPUT_DOUBLE:
    return sizeof(double);
    break;
  case OUTPUT_DATE:
    return sizeof(struct date);
    break;
  case OUTPUT_TIME:
    return sizeof(struct time);
    break;
  case OUTPUT_DATETIME:
    return sizeof(struct datetime);
    break;
  }
  assert(false);
  return 0;
}

/*! \brief Release the output data stored in the buffer of this output
    field.

    Should be called to clear the results of the previous file while
    keeping the output configuration intact, when writing the results
    of a new file to output using the same output settings.*/
static void output_field_clear(struct output_field *this_field) {
  if(this_field->data) {

    if(this_field->memory_type == OUTPUT_STRING) {
      // if data points to an array of char*, we have to iterate
      // through the array and free each char* as well.
      size_t ncols = this_field->data_cols;
      char* (*data)[ncols] = (char * (*)[ncols]) this_field->data;

      // assume no strings occur after the first row with a null pointer:
      for(size_t i = 0;  (*data)[0] != NULL &&  i < outputNbRecords; i++,data++) {
        char *string = (*data)[0];
        for(size_t j = 0; string != NULL && j < ncols; j++, string++)
          free(string);
      }
    }
    free(this_field->data);
    this_field->data = NULL;
  }
}

/*! \brief Free all memory used by this output field and reset it
   completely.*/
static void output_field_free(struct output_field *this_field) {
  output_field_clear(this_field);

  free(this_field->fieldname);
  free(this_field->windowname);

  if(this_field->attributes) {
    free(this_field->attributes);
    this_field->attributes = NULL;
    this_field->num_attributes = 0;
  }

  this_field->index_cross = ITEM_NONE;
  this_field->index_feno = ITEM_NONE;
  this_field->index_calib = ITEM_NONE;
  this_field->index_row = ITEM_NONE;
}

/*! \brief Allocate and initialize the buffers to save output data.

  \param [in] pEngineContext structure including information on the current project

  \retval ERROR_ID_ALLOC if allocation of the buffer outputRecords failed
  \retval ERROR_ID_NONE else
*/
RC OUTPUT_LocalAlloc(ENGINE_CONTEXT *pEngineContext)
{
  // Declarations

  PROJECT             *pProject;                     // pointer to project data
  PRJCT_RESULTS *pResults;                     // pointer to results part of project
  int newRecordNumber;
  RC rc;                                                                        // return code

  // Initializations

  rc=ERROR_ID_NO;

  newRecordNumber=pEngineContext->recordNumber;

  pProject=(PROJECT *)&pEngineContext->project;
  pResults=(PRJCT_RESULTS *)&pProject->asciiResults;

  if (pResults->analysisFlag || pResults->calibFlag)
    {
      assert(newRecordNumber > 0);

      if (outputRecords!=NULL)
        MEMORY_ReleaseBuffer("OUTPUT_LocalAlloc","outputRecords",outputRecords);
      outputRecords=NULL;

      // Allocate new buffers
      outputRecords=(OUTPUT_INFO *)MEMORY_AllocBuffer("OUTPUT_LocalAlloc","outputRecords",newRecordNumber,sizeof(OUTPUT_INFO),0,MEMORY_TYPE_STRUCT);
      if (!outputRecords)
        rc = ERROR_ID_ALLOC;
      else
        memset(outputRecords,0,sizeof(OUTPUT_INFO)*newRecordNumber);

      for (unsigned int i=0; i<output_num_fields; i++) {
        struct output_field *pfield = &output_data_analysis[i];
        output_field_clear(pfield);
        pfield->data = calloc(newRecordNumber * pfield->data_cols , output_get_size(pfield->memory_type));
      }
      for (unsigned int i=0; i<calib_num_fields; i++) {
        struct output_field *calib_field = &output_data_calib[i];
        output_field_clear(calib_field);
        int nb_win = KURUCZ_buffers[calib_field->index_row].Nb_Win;
        calib_field->data = calloc(nb_win * calib_field->data_cols, output_get_size(calib_field->memory_type));
      }

      outputNbRecords=0;
   }

  return rc;
}

/*! \brief Allocate and initialize OUTPUT_AmfSpace.

  \retval ERROR_ID_ALLOC if one of the buffer allocation failed
  \retval ERROR_ID_NO in case of success
*/
RC OUTPUT_Alloc(void)
{
  RC rc=ERROR_ID_NO;

  // Allocate buffers resp. for data to output
  if ( (OUTPUT_AmfSpace=(AMF_SYMBOL *)MEMORY_AllocBuffer("OUTPUT_Alloc ","OUTPUT_AmfSpace",MAX_SYMB,sizeof(AMF_SYMBOL),0,MEMORY_TYPE_STRUCT))==NULL )
     rc=ERROR_ID_ALLOC;

  else
    {
      memset(OUTPUT_AmfSpace,0,sizeof(AMF_SYMBOL)*MAX_SYMB);
    }

  return rc;
}

/*! Release buffers allocated for output.*/
void OUTPUT_Free(void)
{
  if (OUTPUT_AmfSpace!=NULL)
   MEMORY_ReleaseBuffer("OUTPUT_Free ","OUTPUT_AmfSpace",OUTPUT_AmfSpace);

  if (outputRecords!=NULL)
   MEMORY_ReleaseBuffer("OUTPUT_Free","outputRecords",outputRecords);

  OUTPUT_AmfSpace=NULL;
  outputRecords=NULL;
}

/*! \brief Get the format corresponding to a file extension.

  We look for the given extension in the array
  #output_file_extensions.  If the extension is found, the offset
  within the array corresponds to the enum value.  If not: return
  -1.*/
enum output_format output_get_format(const char *fileext) {
  size_t num_formats = sizeof(output_file_extensions)/sizeof(output_file_extensions[0]);
  const char **array_offset = (const char **) lfind(fileext, output_file_extensions, &num_formats, sizeof(output_file_extensions[0]), &compare_string);
  if (array_offset)
    return array_offset - output_file_extensions; // offset in the array output_file_extensions corresponds to the enum value
  else
    return -1; // not found
}

/*! \brief Make a deep copy of an attribute list. */
struct field_attribute *copy_attributes(const struct field_attribute *attributes, int num_attributes) {
  struct field_attribute *copy = malloc(num_attributes * sizeof(*copy));
  for (int i=0; i<num_attributes; i++) {
    copy[i].label = attributes[i].label;
    copy[i].value = attributes[i].value;
  }
  return copy;
}

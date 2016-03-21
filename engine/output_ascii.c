/*! \file output_ascii.c \brief Functions for ascii output.*/

#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "output_common.h"
#include "output.h"
#include "engine_context.h"

/*! current output file (NULL if no file is open) */
static FILE *output_file;

static void write_automatic_reference_info(FILE *fp);
static void write_calib_output(FILE *fp);
static void OutputAscPrintTitles(FILE *fp);

RC ascii_open(const ENGINE_CONTEXT *pEngineContext,char *filename) {

 char *ptr;

 if ((ptr=strrchr(filename,PATH_SEP))==NULL)             // avoid problem when dot is used in the directory path as "./<filename>
  ptr=filename;
 else
  ptr++;

  if (strrchr(ptr,'.')==NULL)                            // ASCII format should accept any extension
    strcat(ptr, output_file_extensions[ASCII]);

  const PROJECT *pProject= &pEngineContext->project;

  output_file = fopen(filename, "a+t");
  if (output_file == NULL) {
    return ERROR_ID_FILE_OPEN;
  }
  int rc = fseek(output_file, 0, SEEK_END);
  if ( rc != 0) {
    return ERROR_ID_FILE_OPEN; // shouldn't happen...
  } else {
    size_t size = ftell(output_file);
    if (size == 0) { // we have a new output file -> print column titles etc
      // Satellites measurements and automatic reference selection : save information on the selected reference
      if ( ( pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII ||
             pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN ||
             pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS ||
             pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2 )
           && pEngineContext->analysisRef.refAuto)  // automatic reference is requested for at least one analysis window
        {
          fprintf(output_file, "%c Reference file\n%c %s\n", COMMENT_CHAR, COMMENT_CHAR, OUTPUT_refFile);
          fprintf(output_file, "%c Number of records selected for the reference\n%c %d\n",COMMENT_CHAR, COMMENT_CHAR, OUTPUT_nRec);
        } else if(pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI
                  && pEngineContext->analysisRef.refAuto
                  && pEngineContext->project.asciiResults.referenceFlag) {
        write_automatic_reference_info(output_file);
      }
      if(calib_num_fields)
        write_calib_output(output_file);
      OutputAscPrintTitles(output_file);
    }
  }
  return ERROR_ID_NO;
}

void ascii_close_file(void) {
  assert(output_file != NULL);
  fclose(output_file);
  output_file = NULL;
}

/*! \brief Print a tab-separated list of the names of the output
    fields.*/
static void OutputAscPrintTitles(FILE *fp)
{
  fprintf(fp, "# ");
  for(unsigned int i=0; i<output_num_fields; i++) {
    struct output_field thefield = output_data_analysis[i];
    for(unsigned int col=0; col<thefield.data_cols; col++) {
      if (thefield.windowname) {
        fprintf(fp, "%s.", thefield.windowname);
      }
      fprintf(fp, "%s",thefield.fieldname);
      if(thefield.data_cols > 1) {
        // in ascii format: print field name for each column, followed
        // by the column number
        fprintf(fp, thefield.column_number_format,
                thefield.column_number_alphabetic ? 'A' + col : 1 + col);
      }
      fprintf(fp, "\t");
    }
  }
  fprintf(fp, "\n");
}

/*! \brief Print a single record of an output field to the output
    file, using the correct format string and data type. */
static void print_output_field(FILE *fp, const struct output_field *thefield, int recordno) {
  size_t ncols = thefield->data_cols;
  switch(thefield->memory_type) {
  case OUTPUT_INT:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) fprintf(fp,thefield->format, ((int (*)[ncols])thefield->data)[recordno][i]);
    break;
  case OUTPUT_SHORT:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) fprintf(fp,thefield->format, ((short (*)[ncols])thefield->data)[recordno][i]);
    break;
  case OUTPUT_USHORT:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) fprintf(fp,thefield->format, ((unsigned short (*)[ncols])thefield->data)[recordno][i]);
    break;
  case OUTPUT_STRING:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) fprintf(fp,thefield->format, ((char* (*)[ncols])thefield->data)[recordno][i]);
    break;
  case OUTPUT_FLOAT:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) fprintf(fp,thefield->format, ((float (*)[ncols])thefield->data)[recordno][i]);
    break;
  case OUTPUT_DOUBLE:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) fprintf(fp,thefield->format, ((double (*)[ncols])thefield->data)[recordno][i]);
    break;
  case OUTPUT_DATE:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) {
      struct date *thedate = &((struct date (*)[ncols])thefield->data)[recordno][i];
      fprintf(fp, thefield->format,  thedate->da_day, thedate->da_mon, thedate->da_year);
    }
    break;
  case OUTPUT_TIME:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) {
      struct time *thetime = &((struct time (*)[ncols])thefield->data)[recordno][i];
      fprintf(fp, thefield->format, thetime->ti_hour, thetime->ti_min, thetime->ti_sec );
    }
    break;
  case OUTPUT_DATETIME:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) {
      struct date *thedate = &((struct datetime (*)[ncols])thefield->data)[recordno][i].thedate;
      struct time *thetime = &((struct datetime (*)[ncols])thefield->data)[recordno][i].thetime;
      int millis = ((struct datetime (*)[ncols])thefield->data)[recordno][i].millis;
      int micros = ((struct datetime (*)[ncols])thefield->data)[recordno][i].microseconds;
      fprintf(fp, thefield->format, thedate->da_year, thedate->da_mon, thedate->da_day, thetime->ti_hour, thetime->ti_min, thetime->ti_sec,
              (millis != -1) ? millis : micros );
    }
    break;
  }
}

void ascii_write_analysis_data(const bool selected_records[], int num_records) {
  assert(output_file != NULL);
  for(int recordno=0; recordno < num_records; recordno++){
    if(selected_records[recordno]) {
      for(unsigned int i=0; i<output_num_fields; i++ ) {
        print_output_field(output_file, &output_data_analysis[i], recordno);
      }
      fprintf(output_file,"\n");
    }
  }
}

/*! \brief Reference info for OMI automatic reference selection.

  Write the file name of each L1B file used in the automatic reference
  generation, with a comma-separated list of the spectra from that
  file that were used. */
static void write_automatic_reference_info(FILE *fp)
{
  for(int analysiswindow = 0; analysiswindow < NFeno; analysiswindow++ ){
    for(int row=0; row< OMI_TOTAL_ROWS; row++ )
      {
        FENO *pTabFeno = &TabFeno[row][analysiswindow];
        if (!pTabFeno->hidden
            && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC
            && pTabFeno->ref_description != NULL)
          {
            fprintf(fp, "%c %s, row %d: automatic reference:\n%s\n",
                    COMMENT_CHAR,
                    pTabFeno->windowName,
                    row,
                    pTabFeno->ref_description);
          }
      }
  }
}

/*! \brief Write calibration data to a file.*/
static void write_calib_output(FILE *fp)
{
  fprintf(fp, "%c ", COMMENT_CHAR);
  for(unsigned int i=0; i<calib_num_fields; i++) {
    struct output_field thefield = output_data_calib[i];

    if (ANALYSE_swathSize > 1)
      fprintf(fp,"Calib(%d/%d).", thefield.index_row+1, ANALYSE_swathSize);
    else
      fprintf(fp,"Calib.");

    if (thefield.windowname) {
      fprintf(fp, "%s.", thefield.windowname);
    }
    fprintf(fp, "%s\t", thefield.fieldname);
  }
  fprintf(fp, "\n");

  int nbWin = KURUCZ_buffers[output_data_calib[0].index_row].Nb_Win; // kurucz settings are same for all detector rows
  for(int recordno=0; recordno < nbWin; recordno++, fprintf(fp,"\n") ){
    fprintf(fp,"%c ", COMMENT_CHAR);
    for(unsigned int i=0; i<calib_num_fields; i++ ) {
      print_output_field(fp, &output_data_calib[i], recordno);
    }
  }
}


//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  ASCII FILES OPERATIONS
//  Name of module    :  ASCII.C
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
//      caroline.fayt@aeronomie.be                  info@stcorp.nl
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
//
//  MODULE DESCRIPTION
//
//  Functions of this module perform the ASCII operations (reading of spectra
//  files in ASCII format, export of spectra from a given file format to ASCII).
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  =======================
//  ASCII EXPORT DIALOG BOX
//  =======================
//
//  AsciiDlgInit - initialization of the 'export dialog box;
//  AsciiSet - retrieve the selected options from the dialog box;
//  ASCII_WndProc - dispatch messages from the 'Export ASCII' dialog box;
//
//  ===============
//  FILE PROCESSING
//  ===============
//
//  ASCII_SaveSpectra - MENU_CONTEXT_SAVE_ASCII message processing;
//  AsciiSkip - skip a given number of records in ASCII files;
//  ASCII_Set - set file pointers for ASCII files and get the number of records;
//  ASCII_Read - read a record from the ASCII file;
//  ASCII_ResetOptions - reset ASCII export options.
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"

// ===================
// CONSTANT DEFINITION
// ===================

#define MAX_LINE_LENGTH 40000

// ================
// GLOBAL VARIABLES
// ================

PRJCT_ASCII ASCII_options;                                                      // options from the 'Export in Ascii' dialog box

// ================
// STATIC VARIABLES
// ================

static INDEX asciiLastRecord=ITEM_NONE;                                         // keep the index of the last record

// QDOAS ??? #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? static INT initConfig=1;
// QDOAS ???
// QDOAS ??? // =======================
// QDOAS ??? // ASCII EXPORT DIALOG BOX
// QDOAS ??? // =======================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        AsciiDlgInit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Initialization of the 'export dialog box
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndAscii : the handle of the parent indow
// QDOAS ??? //                 file      : the name of the file to export
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AsciiDlgInit(HWND hwndAscii,UCHAR *file)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR textTitle[MAX_ITEM_TEXT_LEN+1],                                         // title of the dialog box
// QDOAS ???         textMsg[MAX_ITEM_TEXT_LEN+1];                                           // message string loaded from the resource file
// QDOAS ???
// QDOAS ???   if (initConfig)
// QDOAS ???    {
// QDOAS ???    	memset(&ASCII_options,0,sizeof(PRJCT_ASCII));
// QDOAS ???     initConfig=0;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Center dialog box on parent windows
// QDOAS ???
// QDOAS ???   DOAS_CenterWindow(hwndAscii,GetWindow(hwndAscii,GW_OWNER));
// QDOAS ???
// QDOAS ???   // Set window title
// QDOAS ???
// QDOAS ???   LoadString(DOAS_hInst,IDS_TITLE_ASCII,textMsg,MAX_ITEM_TEXT_LEN);
// QDOAS ???   sprintf(textTitle,textMsg,file);
// QDOAS ???   SetWindowText(hwndAscii,textTitle);
// QDOAS ???
// QDOAS ???   // Set radio and check buttons
// QDOAS ???
// QDOAS ???   CheckRadioButton(hwndAscii,
// QDOAS ???                    ASCII_FORMAT_LINE,
// QDOAS ???                    ASCII_FORMAT_COLUMN,
// QDOAS ???                    ASCII_FORMAT_LINE+ASCII_options.format);
// QDOAS ???
// QDOAS ???   CheckDlgButton(hwndAscii,ASCII_DATA_SZA,(ASCII_options.szaSaveFlag!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???   CheckDlgButton(hwndAscii,ASCII_DATA_AZIM,(ASCII_options.azimSaveFlag!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???   CheckDlgButton(hwndAscii,ASCII_DATA_ELEV,(ASCII_options.elevSaveFlag!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???   CheckDlgButton(hwndAscii,ASCII_DATA_TIME,(ASCII_options.timeSaveFlag!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???   CheckDlgButton(hwndAscii,ASCII_DATA_DATE,(ASCII_options.dateSaveFlag!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???   CheckDlgButton(hwndAscii,ASCII_DATA_LEMBDA,(ASCII_options.lambdaSaveFlag!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        AsciiSet
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Retrieve the selected options from the dialog box
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndAscii : the handle of the parent indow
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AsciiSet(HWND hwndAscii)
// QDOAS ???  {
// QDOAS ???   ASCII_options.szaSaveFlag=(UCHAR)(IsDlgButtonChecked(hwndAscii,ASCII_DATA_SZA)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???   ASCII_options.azimSaveFlag=(UCHAR)(IsDlgButtonChecked(hwndAscii,ASCII_DATA_AZIM)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???   ASCII_options.elevSaveFlag=(UCHAR)(IsDlgButtonChecked(hwndAscii,ASCII_DATA_ELEV)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???   ASCII_options.timeSaveFlag=(UCHAR)(IsDlgButtonChecked(hwndAscii,ASCII_DATA_TIME)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???   ASCII_options.dateSaveFlag=(UCHAR)(IsDlgButtonChecked(hwndAscii,ASCII_DATA_DATE)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???   ASCII_options.lambdaSaveFlag=(UCHAR)(IsDlgButtonChecked(hwndAscii,ASCII_DATA_LEMBDA)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???
// QDOAS ???   ASCII_options.format=(IsDlgButtonChecked(hwndAscii,ASCII_FORMAT_LINE))?PRJCT_INSTR_ASCII_FORMAT_LINE:PRJCT_INSTR_ASCII_FORMAT_COLUMN;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        ASCII_WndProc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         dispatch messages from the 'Export ASCII' dialog box
// QDOAS ??? //
// QDOAS ??? // INPUT           usual syntax for windows messages routines
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? LRESULT CALLBACK ASCII_WndProc(HWND hwndAscii,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch (msg)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_INITDIALOG :
// QDOAS ???      AsciiDlgInit(hwndAscii,(UCHAR *)mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_COMMAND :
// QDOAS ???      {
// QDOAS ???       ULONG commandID;
// QDOAS ???
// QDOAS ???       if ((commandID=(ULONG)GET_WM_COMMAND_ID(mp1,mp2))==IDOK)                  // Close dialog box on OK button command
// QDOAS ???        {
// QDOAS ???         AsciiSet(hwndAscii);
// QDOAS ???         EndDialog(hwndAscii,TRUE);
// QDOAS ???        }
// QDOAS ???       else if (commandID==IDCANCEL)                                             // Close dialog on Cancel button command
// QDOAS ???        EndDialog(hwndAscii,FALSE);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif

// ===============
// FILE PROCESSING
// ===============

#if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_

// -----------------------------------------------------------------------------
// FUNCTION        ASCII_SaveSpectra
// -----------------------------------------------------------------------------
// PURPOSE         MENU_CONTEXT_SAVE_ASCII message processing
//
// INPUT           indexWindow : the index of the calling MDI child window
// -----------------------------------------------------------------------------

void ASCII_SaveSpectra(INDEX indexWindow)
 {
  // Declarations

  PRJCT_INSTRUMENTAL  *pInstrumental;                                           // pointer to the instrumental part of the project
  DRAW_MDI *pDrawEnvironment;                                                   // pointer to the draw environment
  DRAW_GRAPH *pGraph;                                                           // pointer to the graph description
  DRAW_SPECTRUM *pSpectrum,*pDotSpectrum;                                       // pointer to the spectra plotted in the selected graph
  HWND hwndParent;                                                              // handle of parent MDI child window

  UCHAR filePath[MAX_ITEM_TEXT_LEN+1];                                          // files selection with path
  FILE *fp;                                                                     // pointer to the output file
  INDEX i,indexGraph;                                                           // indexes for loops and arrays
  INT fileType,nspec[MAX_GRAPH],ndot[MAX_GRAPH],n;
  RC rc;                                                                        // return

  // Initializations

  memset(filePath,0,sizeof(filePath));                                          // buffer for selected file(s) with path
  fileType=FILE_TYPE_SPECTRA_COMMENT;
  pInstrumental=&THRD_specInfo.project.instrumental;
  rc=ERROR_ID_NO;

  // Search for the drawing environment

  if ((indexWindow!=ITEM_NONE) && (indexWindow<MAX_MDI_WINDOWS) &&
     ((hwndParent=CHILD_list[indexWindow].hwndChild)!=NULL))
   {
    pDrawEnvironment=&CHILD_list[indexWindow].drawEnvironment;                   // draw client area environment

    if ((pDrawEnvironment->hwndDraw!=NULL) &&                                    // handle of the draw client area
        (pDrawEnvironment->graphNumber>=1) && (pDrawEnvironment->graphMax>=1) && // save data for only one graph

     // Get the name of the output file

        (FILES_Open(hwndParent,FILE_TYPE_ASCII_SPECTRA,filePath,MAX_ITEM_TEXT_LEN+1,0,FILE_MODE_SAVE,NULL,ITEM_NONE,&fileType,ITEM_NONE)!=0))
     {
      // Save the current spectrum record plotted in the graph window

      if ((fp=fopen(filePath,"w+t"))==NULL)
       MSG_MessageBox(hwndParent,ITEM_NONE,IDS_APP_NAME,ERROR_ID_FILE_OPEN,MB_OK|MB_ICONEXCLAMATION,filePath);
      else
       {
       	// Add comments and list the different spectra

        if (fileType==FILE_TYPE_SPECTRA_COMMENT)
         {
          fprintf(fp,"; Title : %s\n",pDrawEnvironment->title);
          fprintf(fp,"; Project : %s\n",THRD_specInfo.project.name);
          fprintf(fp,"; Spectra file : %s\n",THRD_specInfo.fileName);
          fprintf(fp,"; Spectrum no %d  %02d/%02d/%d %02d:%02d:%02d  SZA : %g  %d x %g sec, %d rejected\n",
                        THRD_specInfo.indexRecord,
                        THRD_specInfo.present_day.da_day,
                        THRD_specInfo.present_day.da_mon,
                        THRD_specInfo.present_day.da_year,
                        THRD_specInfo.present_time.ti_hour,
                        THRD_specInfo.present_time.ti_min,
                        THRD_specInfo.present_time.ti_sec,
                        THRD_specInfo.Zm,
                        THRD_specInfo.NSomme,
                        THRD_specInfo.Tint,
                        THRD_specInfo.rejected);
          fprintf(fp,"; TDet : %g\n",THRD_specInfo.TDet);

          fprintf(fp,"; Graph title%s :\n",(pDrawEnvironment->graphNumber>1)?"s ":" ");
          for (indexGraph=0;indexGraph<pDrawEnvironment->graphNumber;indexGraph++)
           fprintf(fp,"; Graph %d : %s (%d columns)\n",indexGraph+1,pDrawEnvironment->drawGraph[indexGraph].title,
                  (pDrawEnvironment->drawGraph[indexGraph].spectrum.usey+pDrawEnvironment->drawGraph[indexGraph].dotSpectrum.usey)<<1);
         }

        // In case a correction has been applied on the current spectrum, wait for the
        // user confirmation

        if (THRD_correction && (THRD_specInfo.lambda!=NULL) && (THRD_specInfo.spectrum!=NULL) &&
            ((pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC) ||
            ((pInstrumental->mfcMaskSpec!=0) && (pInstrumental->mfcMaskSpec==(MASK)MFC_header.ty)) ||
            ((pInstrumental->mfcMaskSpec==0) &&
             (MFC_header.wavelength1>=(double)pInstrumental->wavelength-5.) &&
             (MFC_header.wavelength1<=(double)pInstrumental->wavelength+5.))) &&
           ((rc=MessageBox(DOAS_hwndMain,"A correction has been applied on the spectrum.  Save without correction ?","Save ASCII file",
                MB_YESNOCANCEL|MB_ICONQUESTION))==IDYES))

         for (i=0;i<NDET;i++)                                                              // if a correction is applied, the call comes from
          fprintf(fp,"%.14le %.14le\n",THRD_specInfo.lambda[i],THRD_specInfo.spectrum[i]); // the 'Spectra window', so only one spectrum/curve to save

        // No correction, so maybe several curves are plotted in the calling MDI child window

        else if (!rc || (rc==IDNO))
         {
         	// Browse all graphs and get the maximum size of vectors to save

          for (indexGraph=0,n=0;indexGraph<pDrawEnvironment->graphNumber;indexGraph++)
           {
            pGraph=&pDrawEnvironment->drawGraph[indexGraph];
            pSpectrum=&pGraph->spectrum;
            pDotSpectrum=&pGraph->dotSpectrum;

            if ((nspec[indexGraph]=(pSpectrum->usey)?pSpectrum->vectorSize:0)>n)
             n=nspec[indexGraph];
            if ((ndot[indexGraph]=(pDotSpectrum->usey)?pDotSpectrum->vectorSize:0)>n)
             n=ndot[indexGraph];
           }

          // Save the different curves

          for (i=0;i<n;i++)
           {
            for (indexGraph=0;indexGraph<pDrawEnvironment->graphNumber;indexGraph++)
             {
              pGraph=&pDrawEnvironment->drawGraph[indexGraph];                  // description of the first graph
              pSpectrum=&pGraph->spectrum;                                      // description of the solid spectrum
              pDotSpectrum=&pGraph->dotSpectrum;                                // description of the dotted spectrum

              if (pSpectrum->usey)
               {
                fprintf(fp,"%.14le ",((pSpectrum->usex) && (i<nspec[indexGraph]))?pSpectrum->x[i]:i+1);
                fprintf(fp,"%.14le ",(i<nspec[indexGraph])?pSpectrum->y[i]:(double)0.);
               }

              if (pDotSpectrum->usey)
               {
                fprintf(fp,"%.14le ",((pDotSpectrum->usex) && (i<ndot[indexGraph]))?pDotSpectrum->x[i]:i+1);
                fprintf(fp,"%.14le ",(i<ndot[indexGraph])?pDotSpectrum->y[i]:(double)0.);
               }
             }

            fprintf(fp,"\n");
           }
         }

        fclose(fp);
       }
     }
   }
 }

#endif

// -----------------------------------------------------------------------------
// FUNCTION        AsciiSkip
// -----------------------------------------------------------------------------
// PURPOSE         skip a given number of records in ASCII files
//
// INPUT           pEngineContext : information on the file to read out
//                 specFp    : pointer to the ASCII file
//                 nSkip     : number of records to skip
//
// RETURN          ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//                 ERROR_ID_FILE_END if the end of file is reached;
//                 ERROR_ID_ALLOC if the allocation of a buffer failed;
//                 ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC AsciiSkip(ENGINE_CONTEXT *pEngineContext,FILE *specFp,int nSkip)
 {
  // Declarations

  UCHAR *lineRecord,line[MAX_ITEM_TEXT_LEN+1];                                  // read the lines from the ASCII file
  int itemCount,recordCount,maxCount;                                           // counters
  PRJCT_INSTRUMENTAL *pInstr;                                                   // pointer to the instrumental part of the pEngineContext structure
  RC rc;                                                                        // return code

  // Initializations

  pInstr=&pEngineContext->project.instrumental;
  maxCount=NDET+pInstr->ascii.szaSaveFlag+pInstr->ascii.timeSaveFlag+pInstr->ascii.dateSaveFlag;
  lineRecord=NULL;
  rc=ERROR_ID_NO;

  // Buffer allocation

  if (specFp==NULL)
   rc=ERROR_ID_FILE_NOT_FOUND;
  else if (nSkip>=pEngineContext->recordNumber)
   rc=ERROR_ID_FILE_END;
  else

  // Skip the nSkip first spectra
   {
    fseek(specFp,0L,SEEK_SET);
    recordCount=0;
    itemCount=0;

    // Each line of the file is a spectrum record

    if (pInstr->ascii.format==PRJCT_INSTR_ASCII_FORMAT_LINE)
     {
      if ((lineRecord=(char *)MEMORY_AllocBuffer("AsciiSkip","lineRecord",1,MAX_LINE_LENGTH,0,MEMORY_TYPE_STRING))==NULL)
       rc=ERROR_ID_ALLOC;
      else
       while ((recordCount<nSkip) && fgets(lineRecord,MAX_LINE_LENGTH,specFp))  // browse lines of the file
        if ((strchr(lineRecord,';')==NULL) && (strchr(lineRecord,'*')==NULL))   // do not take comment lines into account
         recordCount++;
     }

    // Spectra records are saved in successive columns

    else

     while ((recordCount<nSkip) && fgets(line,MAX_ITEM_TEXT_LEN,specFp))

      if ((strchr(line,';')==NULL) && (strchr(line,'*')==NULL) && (++itemCount==maxCount))
       {
        recordCount++;
        itemCount=0;
       }

    // Reach the end of the file

    if (recordCount<nSkip)
     rc=ERROR_ID_FILE_END;
   }

  // Release the allocated buffer

  if (lineRecord!=NULL)
   MEMORY_ReleaseBuffer("AsciiSkip","lineRecord",lineRecord);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION        ASCII_Set
// -----------------------------------------------------------------------------
// PURPOSE         Set file pointers for ASCII files and get the number of records
//
// INPUT           pEngineContext : information on the file to read
//                 specFp    : pointer to the ASCII file
//
// OUTPUT          pEngineContext->recordNumber, the number of records
//
// RETURN          ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//                 ERROR_ID_ALLOC if the allocation of a buffer failed;
//                 ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC ASCII_Set(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  UCHAR *lineRecord,line[MAX_ITEM_TEXT_LEN+1];                                  // get lines from the ASCII file
  INT itemCount,maxCount;                                                       // counters
  PRJCT_INSTRUMENTAL *pInstr;                                                   // pointer to the instrumental part of the pEngineContext structure
  RC rc;                                                                        // return code

  // Initializations

  asciiLastRecord=ITEM_NONE;                                                    // reset the index of the last record
  pEngineContext->recordNumber=0;
  lineRecord=NULL;
  pInstr=&pEngineContext->project.instrumental;
  maxCount=NDET+pInstr->ascii.szaSaveFlag+pInstr->ascii.timeSaveFlag+pInstr->ascii.dateSaveFlag;
  rc=ERROR_ID_NO;

  // Check the file pointer

  if (specFp==NULL)
   rc=ERROR_SetLast("ASCII_Set",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else
   {
   	// Get the number of records in the file

    fseek(specFp,0L,SEEK_SET);
    itemCount=0;

    // Each line of the file is a spectrum record

    if (pInstr->ascii.format==PRJCT_INSTR_ASCII_FORMAT_LINE)
     {
      if ((lineRecord=(char *)MEMORY_AllocBuffer("ASCII_Set","lineRecord",1,MAX_LINE_LENGTH,0,MEMORY_TYPE_STRING))==NULL)
       rc=ERROR_ID_ALLOC;
      else
       while (fgets(lineRecord,MAX_LINE_LENGTH,specFp))
        if ((strchr(lineRecord,';')==NULL) && (strchr(lineRecord,'*')==NULL))
         pEngineContext->recordNumber++;
     }
    else

     // Spectra records are saved in successive columns

     while (fgets(line,MAX_ITEM_TEXT_LEN,specFp))

      if ((strchr(line,';')==NULL) && (strchr(line,'*')==NULL) && (++itemCount==maxCount))
       {
        pEngineContext->recordNumber++;
        itemCount=0;
       }
   }

  // Release the allocated buffer

  if (lineRecord!=NULL)
   MEMORY_ReleaseBuffer("ASCII_Set","lineRecord",lineRecord);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION        ASCII_Read
// -----------------------------------------------------------------------------
// PURPOSE         Read a record from the ASCII file
//
// INPUT           pEngineContext : information on the file to read
//                 recordNo  : the index of the record to read
//                 dateFlag  : 1 to search for a reference spectrum
//                 localDay  : if dateFlag is 1, the calendar day for the
//                             reference spectrum to search for
//                 specFp    : pointer to the ASCII file
//
// OUTPUT          information on the read out record
//
// RETURN          ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//                 ERROR_ID_FILE_END if the end of the file is reached;
//                 ERROR_ID_FILE_EMPTY if the file is not large enough;
//                 ERROR_ID_ALLOC if the allocation of a buffer failed;
//                 ERROR_ID_FILE_RECORD if the record doesn't match the spectra selection criteria
//                 ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC ASCII_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,INT localDay,FILE *specFp)
 {
  // Declarations

  RECORD_INFO *pRecordInfo;                                                         // pointer to the record part of the engine context
  PRJCT_INSTRUMENTAL *pInstr;                                                   // pointer to the instrumental part of the pEngineContext structure
  UCHAR *lineRecord,*pRecord,line[MAX_ITEM_TEXT_LEN+1];                         // get lines from the ASCII file
  double *spectrum,*lambda,                                                     // the spectrum and the wavelength calibration to read
          tmLocal;                                                              // the measurement time in seconds
  INT lambdaFlag,zmFlag,timeFlag,dateSaveFlag,azimFlag,elevFlag,                // flags to select items to read according to the format options
      day,mon,year;                                                             // decomposition of the measurement date
  INDEX i;                                                                      // browse items to read
  RC rc;                                                                        // return code
  int count;

  // Initializations

  pRecordInfo=&pEngineContext->recordInfo;
  spectrum=pEngineContext->buffers.spectrum;
  lambda=pEngineContext->buffers.lambda;
  pInstr=&pEngineContext->project.instrumental;
  zmFlag=pInstr->ascii.szaSaveFlag;
  azimFlag=pInstr->ascii.azimSaveFlag;
  elevFlag=pInstr->ascii.elevSaveFlag;
  timeFlag=pInstr->ascii.timeSaveFlag;
  dateSaveFlag=pInstr->ascii.dateSaveFlag;
  lambdaFlag=pInstr->ascii.lambdaSaveFlag;
  lineRecord=NULL;
  rc=ERROR_ID_NO;

  memset(&pRecordInfo->present_day,0,sizeof(SHORT_DATE));
  memset(&pRecordInfo->present_time,0,sizeof(struct time));

  VECTOR_Init(spectrum,(double)0.,NDET);

  // Set file pointers

  if (specFp==NULL)
   rc=ERROR_SetLast("ASCII_Read",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;
  else if ((recordNo-asciiLastRecord==1) || !(rc=AsciiSkip(pEngineContext,specFp,recordNo-1)))
   {
    asciiLastRecord=recordNo;

    // ------------------------------------------
    // EACH LINE OF THE FILE IS A SPECTRUM RECORD
    // ------------------------------------------

    if (pInstr->ascii.format==PRJCT_INSTR_ASCII_FORMAT_LINE)
     {
     	// Allocate buffer for LONG lines

      if ((lineRecord=(char *)MEMORY_AllocBuffer("ASCII_Read ","lineRecord",1,MAX_LINE_LENGTH,0,MEMORY_TYPE_STRING))==NULL)
       rc=ERROR_ID_ALLOC;

      else
       {
       	do
       	 {
          if (!fgets(lineRecord,MAX_LINE_LENGTH,specFp))
           rc=ERROR_SetLast("ASCII_Read",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
       	 }
       	while (!rc && ((strchr(lineRecord,';')!=NULL) || (strchr(lineRecord,'*')!=NULL)));

       	if (!rc)
       	 {
          pRecord=lineRecord;

	  // locate the start of first value
          for (;(pRecord!=NULL) && (*pRecord==' ');pRecord++);
	         if (pRecord == NULL)
	          rc = ERROR_ID_FILE_END;

          // Read the solar zenith angle

          if (!rc && zmFlag)
           {
            sscanf(pRecord,"%lf",&pRecordInfo->Zm);
            for (pRecord=strchr(pRecord,' ');(pRecord!=NULL) && (*pRecord==' ');pRecord++);
	           if (pRecord == NULL)
	            rc = ERROR_ID_FILE_END;
           }

          if (!rc && azimFlag)
           {
            sscanf(pRecord,"%f",&pRecordInfo->azimuthViewAngle);
            for (pRecord=strchr(pRecord,' ');(pRecord!=NULL) && (*pRecord==' ');pRecord++);
	           if (pRecord == NULL)
	            rc = ERROR_ID_FILE_END;
           }

          if (!rc && elevFlag)
           {
            sscanf(pRecord,"%f",&pRecordInfo->elevationViewAngle);
            for (pRecord=strchr(pRecord,' ');(pRecord!=NULL) && (*pRecord==' ');pRecord++);
	           if (pRecord == NULL)
	            rc = ERROR_ID_FILE_END;
           }

          // Read the measurement date

          if (!rc && dateSaveFlag)
           {
            sscanf(pRecord,"%d/%d/%d",&day,&mon,&year);
            for (pRecord=strchr(pRecord,' ');(pRecord!=NULL) && (*pRecord==' ');pRecord++);
	           if (pRecord == NULL)
	            rc = ERROR_ID_FILE_END;
           }

          // Read the measurement time

          if (!rc && timeFlag)
           {
            sscanf(pRecord,"%lf",&pRecordInfo->TimeDec);
            for (pRecord=strchr(pRecord,' ');(pRecord!=NULL) && (*pRecord==' ');pRecord++);
	    if (pRecord == NULL) rc = ERROR_ID_FILE_END;
           }

          // Read the spectrum

	  if (!rc)
	   {
	    for (i=0;(i<NDET) && (pRecord != NULL); ++i)
	     {
              sscanf(pRecord,"%lf",&spectrum[i]);
	      for (pRecord=strchr(pRecord,' ');(pRecord!=NULL) && (*pRecord==' ');pRecord++);
	     }
	    // i is NDET if all data in the line was read
	    if (i != NDET)                            // Caro 08/11/2007 maybe there are extra characters after the spectrum so do not check the pRecord here
	      rc = ERROR_ID_FILE_END;
	   }
	 }
       }
     }

    // -----------------------------------------------
    // SPECTRA RECORDS ARE SAVED IN SUCCESSIVE COLUMNS
    // -----------------------------------------------

    else
     {
      // Read the solar zenith angle

      count = 0;
      if (zmFlag) ++count;
      if (azimFlag) ++count;
      if (elevFlag) ++count;

      if (count)
       {
        while (fgets(line,MAX_ITEM_TEXT_LEN,specFp) && ((strchr(line,';')!=NULL) || (strchr(line,'*')!=NULL)));
        pRecord=line;
	// locate the start of the first value
	for (;(pRecord!=NULL) && (*pRecord==' ');pRecord++);

        if (zmFlag && (pRecord != NULL))
         {
 	   if (sscanf(pRecord,"%lf",&pRecordInfo->Zm) == 1) --count;
 	   for (pRecord=strchr(pRecord,' ');(pRecord!=NULL) && (*pRecord==' ');pRecord++);
         }

        if (azimFlag && (pRecord != NULL))
         {
 	   if (sscanf(pRecord,"%f",&pRecordInfo->azimuthViewAngle) == 1) --count;
 	   for (pRecord=strchr(pRecord,' ');(pRecord!=NULL) && (*pRecord==' ');pRecord++);
         }

        if (elevFlag && (pRecord != NULL))
         {
 	   if (sscanf(pRecord,"%f",&pRecordInfo->elevationViewAngle) == 1) --count;
 	   for (pRecord=strchr(pRecord,' ');(pRecord!=NULL) && (*pRecord==' ');pRecord++);
         }

	// should be at the end of the line and read ALL of the data
	if (pRecord != NULL || count) rc = ERROR_ID_FILE_END;
       }

      // Read the measurement date

      if (!rc && dateSaveFlag)
       {
        while (fgets(line,MAX_ITEM_TEXT_LEN,specFp) && ((strchr(line,';')!=NULL) || (strchr(line,'*')!=NULL)));
        if (sscanf(line,"%d/%d/%d",&day,&mon,&year) != 3) rc = ERROR_ID_FILE_END;
       }

      // Read the measurement time

      if (!rc && timeFlag)
       {
        while (fgets(line,MAX_ITEM_TEXT_LEN,specFp) && ((strchr(line,';')!=NULL) || (strchr(line,'*')!=NULL)));
         if (sscanf(line,"%lf",&pRecordInfo->TimeDec) != 1) rc = ERROR_ID_FILE_END;
       }

      // Read the spectrum and if selected, the wavelength calibration
      if (!rc)
      {
	if (lambdaFlag) // wavelength and spectrum
	{
	  for (i=0;(i<NDET) && !rc; )
	  {
	    if (!fgets(line,MAX_ITEM_TEXT_LEN,specFp))
	    {
	      rc=ERROR_SetLast("ASCII_Read",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
	    }
	    else if ((strchr(line,';')==NULL) && (strchr(line,'*')==NULL))
	    {
	      if (sscanf(line,"%lf %lf",&lambda[i],&spectrum[i]) != 2) rc = ERROR_ID_FILE_END;
	      ++i;
	    }
	  }
	}
	else // just spectrum
	{
	  for (i=0;(i<NDET) && !rc; )
	  {
	    if (!fgets(line,MAX_ITEM_TEXT_LEN,specFp))
	    {
	      rc=ERROR_SetLast("ASCII_Read",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pEngineContext->fileInfo.fileName);
	    }
	    else if ((strchr(line,';')==NULL) && (strchr(line,'*')==NULL))
	      {
	      if (sscanf(line,"%lf",&spectrum[i]) != 1) rc = ERROR_ID_FILE_END;
	      ++i;
	    }
	  }
	}
      }
     }

    if (!rc)
     {
      // Get information on the current record

      if (timeFlag)
       {
        pRecordInfo->present_time.ti_hour=(UCHAR)pRecordInfo->TimeDec;
        pRecordInfo->present_time.ti_min=(UCHAR)((pRecordInfo->TimeDec-pRecordInfo->present_time.ti_hour)*60.);
        pRecordInfo->present_time.ti_sec=(UCHAR)(((pRecordInfo->TimeDec-pRecordInfo->present_time.ti_hour)*60.-pRecordInfo->present_time.ti_min)*60.);
       }

      if (dateSaveFlag)
       {
        pRecordInfo->present_day.da_day=(UCHAR)day;
        pRecordInfo->present_day.da_mon=(UCHAR)mon;
        pRecordInfo->present_day.da_year=(SHORT)year;
       }

      // Daily automatic reference spectrum

      if (!rc && dateSaveFlag)
       {
        pRecordInfo->Tm=(double)ZEN_NbSec(&pRecordInfo->present_day,&pRecordInfo->present_time,0);

        tmLocal=pRecordInfo->Tm+THRD_localShift*3600.;

        pRecordInfo->localCalDay=ZEN_FNCaljda(&tmLocal);
        pRecordInfo->localTimeDec=fmod(pRecordInfo->TimeDec+24.+THRD_localShift,(double)24.);
       }
      else
       pRecordInfo->Tm=(double)0.;

      if (dateFlag && dateSaveFlag && (pRecordInfo->localCalDay!=localDay))
       rc=ERROR_ID_FILE_RECORD;
     }
   }

  // Return


  if (lineRecord!=NULL)
   MEMORY_ReleaseBuffer("ASCII_Read","lineRecord",lineRecord);

  return rc;
 }


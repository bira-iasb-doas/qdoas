
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
//  Module purpose    :  THREADS PROCESSING
//  Name of module    :  WINTHRD.C
//  Program Language  :  Borland C++ 5.0 for Windows 95/NT
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
//  FUNCTIONS
//
//  ==================
//  INDEXES PROCESSING
//  ==================
//
//  THRD_Context - filter operations on project tree items according to thread type;
//  THRD_SetIndexes - set indexes to browse in tree;
//  ThrdNextRecord - set indexes in order to access to the next record to process;
//
//  ================
//  EVENT PROCESSING
//  ================
//
//  THRD_Error - temporary and fatal errors handling;
//  THRD_WaitEvent - wait and process an event;
//
//  ===============
//  DATA PROCESSING
//  ===============
//
//  THRD_ResetSpecInfo - release or reset data hold by the last thread;
//  ThrdInitSpecInfo - allocate buffers for a new project;
//  THRD_CopySpecInfo - make a copy of data on the current spectra file into another structure;
//  ThrdWriteSpecInfo - write data on current spectrum in a temporary file;
//  ThrdLoadInstrumental - load instrumental functions;
//  ThrdLoadProject - load a project
//
//  ===============
//  FILE PROCESSING
//  ===============
//
//  ThrdFileSetPointers - initialize file pointers;
//  ThrdReadFile - spectra file read out;
//
//  ======================================
//  AUTOMATIC REFERENCE SPECTRUM SELECTION
//  ======================================
//
//  ThrdSetRefIndexes - set indexes of spectra selected as reference for analysis;
//  ThrdNewRef - Load new reference spectra;
//
//  ===================
//  COMMANDS PROCESSING
//  ===================
//
//  THRD_BrowseSpectra - thread used for browsing spectra in a file;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  THRD_Alloc - allocate events objects for threads synchronization;
//  THRD_Free - release events objects;
//
//  ============
//  USE EXTERNAL
//  ============
//
//  ANALYSE_LoadData,ANALYSE_ResetData,ANALYSE_NormalizeVector for analysis data processing;
//  ANALYSE_Kurucz,ANALYSE_AlignReference,ANALYSE_Spectrum for reference processing and spectra analysis;
//  ----------------------------------------------------------------------------

#include "doas.h"

// ================
// GLOBAL VARIABLES
// ================

UCHAR     THRD_asciiFile[MAX_ITEM_TEXT_LEN+1],*THRD_asciiPtr;        // ascii file for exporting spectra
HANDLE    THRD_hEvents[THREAD_EVENT_MAX];      // list of events
SPEC_INFO THRD_specInfo,THRD_refInfo;          // data on current spectra and reference
UINT      THRD_id=THREAD_TYPE_NONE;            // thread identification number
double    THRD_localNoon;                      // local noon
INT       THRD_localShift;
DWORD     THRD_delay;
INT       THRD_correction;
INT       THRD_browseType;
INT       THRD_treeCallFlag;
INT       THRD_lastRefRecord;
INT       THRD_isFolder;
INT       THRD_recordLast;

// ================
// STATIC VARIABLES
// ================

INDEX  THRD_indexCaller,
       THRD_indexProjectOld,THRD_indexProjectCurrent,
       THRD_indexFileOld,THRD_indexFileCurrent,
       THRD_indexRecordOld,THRD_indexRecordCurrent,THRD_indexPathFileCurrent,THRD_indexPathFileOld;
INT    THRD_lastEvent=ITEM_NONE,THRD_increment,THRD_levelMax,
       THRD_resetFlag,THRD_setOriginalFlag,
       THRD_recordFirst;
INDEX *THRD_dataIndexes;
INT    THRD_dataIndexesNumber;
FILE  *THRD_asciiFp;
INT    THRD_pathOK;
INT    THRD_endProgram;
INT    thrdRefFlag;

THRD_GOTO THRD_goto={ 1, ITEM_NONE, 1, 0, ITEM_NONE, ITEM_NONE };

// ===================
// MESSAGES PROCESSING
// ===================

// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? // --------------------------------------------------
// QDOAS ??? // ThrdGotoDlgInit : WM_INITDIALOG message processing
// QDOAS ??? // --------------------------------------------------
// QDOAS ???
// QDOAS ??? void ThrdGotoDlgInit(HWND hwndThrdGoto)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   PRJCT_INSTRUMENTAL *pInstrumental;
// QDOAS ???   HWND hwndGotoType;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pInstrumental=&THRD_specInfo.project.instrumental;
// QDOAS ???
// QDOAS ???   THRD_goto.increment=0;
// QDOAS ???   THRD_goto.indexRecord=ITEM_NONE;
// QDOAS ???   THRD_goto.indexMin=THRD_goto.indexMax=ITEM_NONE;
// QDOAS ???
// QDOAS ???   hwndGotoType=GetDlgItem(hwndThrdGoto,THRD_GOTO_TYPE);
// QDOAS ???
// QDOAS ???   // Dialog box initialization
// QDOAS ???
// QDOAS ???   if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
// QDOAS ???
// QDOAS ???    CheckRadioButton(hwndThrdGoto,
// QDOAS ???                     THRD_GOTO_BUTTON_RECORD,
// QDOAS ???                     THRD_GOTO_BUTTON_PIXEL,
// QDOAS ???                     THRD_GOTO_BUTTON_RECORD+THRD_goto.indexType);
// QDOAS ???
// QDOAS ???   ShowWindow(GetDlgItem(hwndThrdGoto,THRD_GOTO_TYPE_TEXT),SW_HIDE);
// QDOAS ???   ShowWindow(GetDlgItem(hwndThrdGoto,THRD_GOTO_TYPE),SW_HIDE);
// QDOAS ???   ShowWindow(GetDlgItem(hwndThrdGoto,THRD_GOTO_OBS_TEXT),SW_HIDE);
// QDOAS ???   ShowWindow(GetDlgItem(hwndThrdGoto,THRD_GOTO_OBS),SW_HIDE);
// QDOAS ???   ShowWindow(GetDlgItem(hwndThrdGoto,THRD_GOTO_BUTTON_RECORD),(pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)?SW_SHOW:SW_HIDE);
// QDOAS ???   ShowWindow(GetDlgItem(hwndThrdGoto,THRD_GOTO_BUTTON_PIXEL),(pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)?SW_SHOW:SW_HIDE);
// QDOAS ???
// QDOAS ???   SetDlgItemInt(hwndThrdGoto,THRD_GOTO_RECORD,THRD_goto.indexOld,FALSE);
// QDOAS ???   SetDlgItemInt(hwndThrdGoto,THRD_GOTO_OBS,THRD_goto.indexPixel,FALSE);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? void ThrdGotoOK(HWND hwndThrdGoto)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   PRJCT_INSTRUMENTAL *pInstrumental;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pInstrumental=&THRD_specInfo.project.instrumental;
// QDOAS ???
// QDOAS ???   THRD_goto.indexOld=(INT)GetDlgItemInt(hwndThrdGoto,THRD_GOTO_RECORD,NULL,FALSE);
// QDOAS ???   THRD_goto.indexPixel=(INT)GetDlgItemInt(hwndThrdGoto,THRD_GOTO_OBS,NULL,FALSE);
// QDOAS ???
// QDOAS ???   if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) &&
// QDOAS ???      ((THRD_goto.indexType=(IsDlgButtonChecked(hwndThrdGoto,THRD_GOTO_BUTTON_RECORD))?THREAD_GOTO_RECORD:THREAD_GOTO_PIXEL)==THREAD_GOTO_PIXEL))
// QDOAS ???
// QDOAS ???    THRD_goto.indexRecord=GDP_BIN_GetRecordNumber(THRD_goto.indexOld);
// QDOAS ???
// QDOAS ???   else
// QDOAS ???    THRD_goto.indexRecord=THRD_goto.indexOld;
// QDOAS ???
// QDOAS ???   EndDialog(hwndThrdGoto,TRUE);
// QDOAS ???   SetEvent(THRD_hEvents[THREAD_EVENT_GOTO]);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------
// QDOAS ??? // ThrdGotoCommand : WM_COMMAND message processing
// QDOAS ??? // -----------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? LRESULT CALLBACK ThrdGotoCommand(HWND hwndThrdGoto,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch((ULONG)GET_WM_COMMAND_ID(mp1,mp2))
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDOK :                      // close dialog box on OK button command
// QDOAS ???      ThrdGotoOK(hwndThrdGoto);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case THRD_GOTO_TYPE :                      // close dialog box on OK button command
// QDOAS ???      THRD_goto.indexType=SendMessage(GetDlgItem(hwndThrdGoto,THRD_GOTO_TYPE),CB_GETCURSEL,0,0);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------
// QDOAS ??? // THRD_GotoWndProc :
// QDOAS ??? // -----------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? LRESULT CALLBACK THRD_GotoWndProc(HWND hwndThrdGoto,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch (msg)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_INITDIALOG :
// QDOAS ???      ThrdGotoDlgInit(hwndThrdGoto);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_COMMAND :
// QDOAS ???      ThrdGotoCommand(hwndThrdGoto,msg,mp1,mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_CLOSE :
// QDOAS ???      EndDialog(hwndThrdGoto,0);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif // __WINDOAS_GUI_

// QDOAS ??? // ==================
// QDOAS ??? // INDEXES PROCESSING
// QDOAS ??? // ==================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------
// QDOAS ??? // ThrdSearchDataIndex : search data index in list
// QDOAS ??? // -----------------------------------------------
// QDOAS ???
// QDOAS ??? RC ThrdSearchDataIndex(INDEX entryPoint)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexItem,dataIndex,i;
// QDOAS ???   TREE_ITEM *pTree;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pTree=&TREE_itemList[entryPoint];
// QDOAS ???   dataIndex=pTree->dataIndex;
// QDOAS ???   rc=0;
// QDOAS ???
// QDOAS ???   if (pTree->dataType==TREE_ITEM_TYPE_FILE_CHILDREN)
// QDOAS ???    for (i=0;(i<THRD_dataIndexesNumber) && !rc;i++)
// QDOAS ???     if (THRD_dataIndexes[i]==dataIndex)
// QDOAS ???      rc=1;
// QDOAS ???
// QDOAS ???   for (indexItem=pTree->firstChildItem;(indexItem!=ITEM_NONE)&&!rc;indexItem=TREE_itemList[indexItem].nextItem)
// QDOAS ???    rc=ThrdSearchDataIndex(indexItem);
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -------------------------------------------------------------------------------
// QDOAS ??? // THRD_Context : Filter operations on project tree items according to thread type
// QDOAS ??? // -------------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? BOOL THRD_Context(INDEX indexItem,INT menuId)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexCaller;
// QDOAS ???   INT dataType;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   dataType=TREE_itemList[indexItem].dataType;
// QDOAS ???   indexCaller=indexItem;
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return   ((THRD_id!=THREAD_TYPE_NONE) &&
// QDOAS ???           (((THRD_id!=THREAD_TYPE_ANALYSIS) && (THRD_id!=THREAD_TYPE_KURUCZ)) ||
// QDOAS ???            ((dataType!=TREE_ITEM_TYPE_ANALYSIS_PARENT) && (dataType!=TREE_ITEM_TYPE_ANALYSIS_CHILDREN)) ||
// QDOAS ???           (((indexCaller=TREE_itemList[indexItem].parentItem)!=ITEM_NONE) &&
// QDOAS ???            ((indexCaller=TREE_itemList[indexCaller].parentItem)!=ITEM_NONE))) &&
// QDOAS ???              ThrdSearchDataIndex(indexCaller)) ? FALSE : TRUE;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ------------------------------------------------
// QDOAS ??? // THRD_SetIndexes : Set indexes of items to browse
// QDOAS ??? // ------------------------------------------------
// QDOAS ???
// QDOAS ??? BOOL THRD_SetIndexes(INDEX indexItem,INDEX treeCallFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR tmpBuffer[MAX_ITEM_TEXT_LEN+1],*ptr;     // temporary buffer
// QDOAS ???   INDEX indexProject;
// QDOAS ???   RAW_FILES *pRaw;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   THRD_indexProjectOld=
// QDOAS ???   THRD_indexFileOld=
// QDOAS ???   THRD_indexRecordOld=
// QDOAS ???   THRD_indexPathFileOld=
// QDOAS ???   THRD_indexProjectCurrent=
// QDOAS ???   THRD_indexFileCurrent=
// QDOAS ???   THRD_indexRecordCurrent=
// QDOAS ???   THRD_indexPathFileCurrent=ITEM_NONE;
// QDOAS ???   THRD_dataIndexesNumber=0;
// QDOAS ???   THRD_pathOK=1;
// QDOAS ???
// QDOAS ???   if ((THRD_treeCallFlag=treeCallFlag)!=0)
// QDOAS ???    {
// QDOAS ???     PATH_fileNumber=PATH_dirNumber=0;
// QDOAS ???
// QDOAS ???     // Search for the first file to browse
// QDOAS ???
// QDOAS ???     if (((THRD_indexCaller=(indexItem!=ITEM_NONE)?indexItem:CHILD_list[CHILD_WINDOW_PROJECT].itemTree)!=ITEM_NONE) &&             // get index of item in tree from wich the call of this function has been produced
// QDOAS ???          (PRJCT_itemList!=NULL) && (TREE_itemType[TREE_ITEM_TYPE_FILE_CHILDREN].dataList!=NULL) &&                                // buffers for project and files have been successfully allocated
// QDOAS ???          (TREE_itemType[TREE_ITEM_TYPE_PROJECT].dataNumber>0) &&                                                                  // projects list is not empty
// QDOAS ???          (TREE_itemType[TREE_ITEM_TYPE_FILE_CHILDREN].dataNumber>0) &&                                                            // file list is not empty
// QDOAS ???          (TREE_GetIndexesByDataType(TREE_ITEM_TYPE_FILE_CHILDREN,THRD_indexCaller,THRD_dataIndexes,&THRD_dataIndexesNumber)>0))   // selected item is neither a file nor a folder so search for index of first file in child nodes
// QDOAS ???      {
// QDOAS ???       THRD_indexFileCurrent=0;
// QDOAS ???       pRaw=&RAW_spectraFiles[THRD_dataIndexes[THRD_indexFileCurrent]];
// QDOAS ???
// QDOAS ???       PATH_mfcFlag=(((indexProject=TREE_GetProjectParent(THRD_indexCaller))!=ITEM_NONE) &&
// QDOAS ???                     ((PRJCT_itemList[indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
// QDOAS ???                      (PRJCT_itemList[indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD) ||
// QDOAS ???                      (PRJCT_itemList[indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OPUS)))?1:0;
// QDOAS ???
// QDOAS ???       PATH_UofTFlag=(PRJCT_itemList[indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_UOFT)?1:0;
// QDOAS ???
// QDOAS ???       if ((pRaw->folderFlag==RAW_FOLDER_TYPE_PATH) || (pRaw->folderFlag==RAW_FOLDER_TYPE_PATHSUB))
// QDOAS ???        {
// QDOAS ???         FILES_RebuildFileName(tmpBuffer,pRaw->fileName,1);
// QDOAS ???
// QDOAS ???         if ((ptr=strrchr(tmpBuffer,PATH_SEP))!=NULL)
// QDOAS ???          {
// QDOAS ???           *ptr++=0;
// QDOAS ???           PATH_GetFilesList(tmpBuffer,ptr,0);
// QDOAS ???           THRD_indexPathFileCurrent=SYS_path-PATH_dirNumber-1;
// QDOAS ???
// QDOAS ???           if (PATH_fileNumber<=0)
// QDOAS ???            THRD_pathOK=0;
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???       else
// QDOAS ???        PATH_fileNumber=1;
// QDOAS ???
// QDOAS ???       THRD_indexProjectCurrent=pRaw->indexProject;
// QDOAS ???
// QDOAS ???       if (PATH_fileNumber>0)
// QDOAS ???        {
// QDOAS ???         if ((PRJCT_itemList[THRD_indexProjectCurrent].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
// QDOAS ???             (PRJCT_itemList[THRD_indexProjectCurrent].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD) ||
// QDOAS ???             (PRJCT_itemList[THRD_indexProjectCurrent].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OPUS))
// QDOAS ??? //            (PRJCT_itemList[THRD_indexProjectCurrent].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_RASAS))
// QDOAS ???
// QDOAS ???          PATH_fileNumber=1;
// QDOAS ???
// QDOAS ???         THRD_indexPathFileCurrent=SYS_path-PATH_dirNumber-1;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???   else if ((indexItem!=ITEM_NONE) && (TREE_itemList[indexItem].dataIndex!=ITEM_NONE) && (PATH_fileNumber+PATH_dirNumber>0))
// QDOAS ???    {
// QDOAS ???     THRD_dataIndexesNumber=1;
// QDOAS ???     THRD_dataIndexes[0]=TREE_itemList[indexItem].dataIndex;
// QDOAS ???     THRD_indexProjectCurrent=RAW_spectraFiles[TREE_itemList[indexItem].dataIndex].indexProject;
// QDOAS ???     THRD_indexFileCurrent=0;
// QDOAS ???     THRD_indexPathFileCurrent=SYS_path-1;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return (THRD_dataIndexesNumber>0)?TRUE:FALSE;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // ThrdNextRecord : Set indexes in order to access to the next record to process
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? BOOL ThrdNextRecord(void)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR tmpBuffer[MAX_ITEM_TEXT_LEN+1],*ptr;     // temporary buffer
// QDOAS ???   INDEX indexRecord,indexFile,indexPath;
// QDOAS ???   RAW_FILES *pRaw;
// QDOAS ???   BOOL rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pRaw=&RAW_spectraFiles[THRD_dataIndexes[THRD_indexFileCurrent]];
// QDOAS ???   THRD_indexRecordOld=THRD_indexRecordCurrent;
// QDOAS ???   THRD_indexPathFileOld=THRD_indexPathFileCurrent;
// QDOAS ???   indexRecord=THRD_indexRecordCurrent+THRD_increment;
// QDOAS ???   indexFile=THRD_indexFileCurrent+THRD_increment;
// QDOAS ???   indexPath=THRD_indexPathFileCurrent-THRD_increment;
// QDOAS ???   THRD_pathOK=1;
// QDOAS ???   rc=TRUE;
// QDOAS ???
// QDOAS ???   if ((THRD_increment!=0) &&
// QDOAS ???      ((pRaw->folderFlag==RAW_FOLDER_TYPE_PATH) || (pRaw->folderFlag==RAW_FOLDER_TYPE_PATHSUB)) &&
// QDOAS ???      ((THRD_indexRecordCurrent==ITEM_NONE) || (indexRecord<THRD_recordFirst) || (indexRecord>THRD_recordLast)) &&
// QDOAS ???       (indexPath<(INDEX)(SYS_path-PATH_dirNumber)) && (indexPath>=(INDEX)(SYS_path-PATH_fileNumber-PATH_dirNumber)))
// QDOAS ???
// QDOAS ???    THRD_indexPathFileCurrent=indexPath;
// QDOAS ???
// QDOAS ???   else if ((THRD_increment!=0) &&
// QDOAS ???           ((THRD_indexRecordCurrent==ITEM_NONE) || (indexRecord<THRD_recordFirst) || (indexRecord>THRD_recordLast)) &&
// QDOAS ???            (indexFile>=0) && (indexFile<THRD_dataIndexesNumber))
// QDOAS ???    {
// QDOAS ???     THRD_indexFileCurrent=indexFile;
// QDOAS ???     pRaw=&RAW_spectraFiles[THRD_dataIndexes[THRD_indexFileCurrent]];
// QDOAS ???     THRD_indexProjectCurrent=pRaw->indexProject;
// QDOAS ???
// QDOAS ???     if ((pRaw->folderFlag!=RAW_FOLDER_TYPE_PATH) && (pRaw->folderFlag!=RAW_FOLDER_TYPE_PATHSUB))
// QDOAS ???      THRD_indexPathFileCurrent=ITEM_NONE;
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???       FILES_RebuildFileName(tmpBuffer,pRaw->fileName,1);
// QDOAS ???
// QDOAS ???       PATH_mfcFlag=((PRJCT_itemList[THRD_indexProjectCurrent].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
// QDOAS ???                     (PRJCT_itemList[THRD_indexProjectCurrent].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD))?1:0;
// QDOAS ???
// QDOAS ???       PATH_UofTFlag=(PRJCT_itemList[THRD_indexProjectCurrent].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_UOFT)?1:0;
// QDOAS ???
// QDOAS ???       if ((ptr=strrchr(tmpBuffer,PATH_SEP))!=NULL)
// QDOAS ???        {
// QDOAS ???         *ptr++=0;
// QDOAS ???         PATH_fileNumber=PATH_dirNumber=0;
// QDOAS ???         PATH_GetFilesList(tmpBuffer,ptr,0);
// QDOAS ???
// QDOAS ???         if (!PATH_fileNumber)
// QDOAS ???          THRD_pathOK=0;
// QDOAS ???         else
// QDOAS ???          {
// QDOAS ???           if ((PRJCT_itemList[THRD_indexProjectCurrent].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
// QDOAS ???               (PRJCT_itemList[THRD_indexProjectCurrent].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD) ||
// QDOAS ???               (PRJCT_itemList[THRD_indexProjectCurrent].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OPUS) ||
// QDOAS ???               (PRJCT_itemList[THRD_indexProjectCurrent].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_RASAS))
// QDOAS ???
// QDOAS ???            PATH_fileNumber=1;
// QDOAS ???
// QDOAS ???           THRD_indexPathFileCurrent=SYS_path-PATH_dirNumber-1;
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???       else
// QDOAS ???        THRD_indexPathFileCurrent=ITEM_NONE;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     if (THRD_indexRecordCurrent==ITEM_NONE)
// QDOAS ???      rc=FALSE;
// QDOAS ???     else if ((THRD_goto.indexType==THREAD_GOTO_PIXEL) && (THRD_goto.indexPixel>0) &&
// QDOAS ???              (THRD_goto.indexMin!=ITEM_NONE) && (THRD_goto.indexMax!=ITEM_NONE) && (THRD_goto.indexMin<THRD_goto.indexMax))
// QDOAS ???      THRD_indexRecordCurrent=(THRD_goto.indexMin+THRD_goto.indexMax)>>1;
// QDOAS ???     else if (indexRecord<THRD_recordFirst)
// QDOAS ???      THRD_indexRecordCurrent=THRD_recordFirst;
// QDOAS ???     else if (indexRecord>THRD_recordLast)
// QDOAS ???      THRD_indexRecordCurrent=THRD_recordLast;
// QDOAS ???     else
// QDOAS ???      THRD_indexRecordCurrent=indexRecord;
// QDOAS ???
// QDOAS ???     if (THRD_indexRecordOld==THRD_indexRecordCurrent)
// QDOAS ???      rc=FALSE;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ================
// QDOAS ??? // EVENT PROCESSING
// QDOAS ??? // ================
// QDOAS ???
// QDOAS ??? RC THRD_ProcessLastError(void)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   ERROR_DESCRIPTION errorDescription;                                           // information about the last error
// QDOAS ???   UCHAR errorTitle[MAX_STR_LEN+1];                                              // title for error message
// QDOAS ???   int errorType;                                                                // the type of the last error
// QDOAS ???   int errorFatalFlag;                                                           // 1 on fatal error
// QDOAS ???   FILE *fp;                                                                     // pointer to the log file
// QDOAS ???   RC rc,rcGetLast;                                                              // error return code
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   errorFatalFlag=0;
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Get last error
// QDOAS ???
// QDOAS ???   while ((rcGetLast=ERROR_GetLast(&errorDescription))!=0)
// QDOAS ???    {
// QDOAS ???     // Build the title of the message according to the error type
// QDOAS ???
// QDOAS ???     errorType=errorDescription.errorType;
// QDOAS ???
// QDOAS ???     if (errorType==ERROR_TYPE_WARNING)
// QDOAS ???      sprintf(errorTitle,"[Warning %d]",rcGetLast);
// QDOAS ???     else if (errorType==ERROR_TYPE_FATAL)
// QDOAS ???      sprintf(errorTitle,"[Fatal error %d]",rcGetLast);
// QDOAS ???     else if (errorType==ERROR_TYPE_DEBUG)
// QDOAS ???      sprintf(errorTitle,"[Debug %d]",rcGetLast);
// QDOAS ???     else
// QDOAS ???      sprintf(errorTitle,"[Unknown error %d]",rcGetLast);
// QDOAS ???
// QDOAS ???     if (!rc && (errorType!=ERROR_TYPE_DEBUG))
// QDOAS ???      rc=rcGetLast;
// QDOAS ???
// QDOAS ???     // Print out the message into the debug log file
// QDOAS ???
// QDOAS ???     if ((fp=fopen(DOAS_logFile,"a+t"))!=NULL)
// QDOAS ???      {
// QDOAS ???       fprintf(fp,"%s %s - %s\n",errorTitle,errorDescription.errorFunction,errorDescription.errorString);
// QDOAS ???       fclose(fp);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Display the message in a dialog box
// QDOAS ???
// QDOAS ???     #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???     if ((DOAS_hwndMain!=NULL) && (errorType==ERROR_TYPE_FATAL) || ((errorType==ERROR_TYPE_WARNING) && (THRD_delay==INFINITE)))
// QDOAS ???      MessageBox(DOAS_hwndMain,errorDescription.errorString,errorTitle,(errorType==ERROR_TYPE_WARNING)?
// QDOAS ???                 MB_OK|MB_ICONEXCLAMATION:MB_OK|MB_ICONHAND);
// QDOAS ???     #endif
// QDOAS ???
// QDOAS ???     if (errorType==ERROR_TYPE_FATAL)
// QDOAS ???      errorFatalFlag++;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Stop the current thread on fatal error
// QDOAS ???
// QDOAS ???   if (errorFatalFlag && (THRD_id>THREAD_TYPE_NONE))
// QDOAS ???    {
// QDOAS ???    	#if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???     SetEvent(THRD_hEvents[THREAD_EVENT_STOP]);
// QDOAS ???     #endif
// QDOAS ???     rc=THREAD_EVENT_STOP;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ------------------------------------------------
// QDOAS ??? // THRD_Error : temporary and fatal errors handling
// QDOAS ??? // ------------------------------------------------
// QDOAS ???
// QDOAS ??? RC THRD_Error(INT errorType,INT errorId,UCHAR *function,...)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   ERROR_DESCRIPTION errorDescription;
// QDOAS ???   UCHAR   errorString[MAX_ITEM_TEXT_LEN+1],     // message loaded from resources
// QDOAS ???           errorBuf[MAX_ITEM_TEXT_LEN+1],        // formatted message to display
// QDOAS ???           errorTitle[MAX_ITEM_TEXT_LEN+1],
// QDOAS ???           errorFunction[MAX_ITEM_TEXT_LEN+1],
// QDOAS ???           errorSystemString[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???
// QDOAS ???   va_list argList;
// QDOAS ???   DWORD systemError;
// QDOAS ???   FILE *fp;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   memset(errorString,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   memset(errorBuf,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   memset(errorTitle,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   if (((errorType==ERROR_TYPE_WARNING) || (errorType==ERROR_TYPE_OWNERDRAWN)) && (THRD_delay!=INFINITE))
// QDOAS ???    errorType=ERROR_TYPE_FILE;
// QDOAS ???
// QDOAS ???   // Loads message
// QDOAS ???
// QDOAS ???   if (errorId==ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     ERROR_GetLast(&errorDescription);
// QDOAS ???
// QDOAS ???     errorType=errorDescription.errorType;
// QDOAS ???     errorId=errorDescription.errorId;
// QDOAS ???     sprintf(errorFunction,"%s [%s]",function,errorDescription.errorFunction);
// QDOAS ???     strcpy(errorBuf,errorDescription.errorString);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   else if (LoadString(DOAS_hInst,errorId,errorString,MAX_ITEM_TEXT_LEN))
// QDOAS ???    {
// QDOAS ???     strcpy(errorFunction,function);
// QDOAS ???
// QDOAS ???     // Arguments read out
// QDOAS ???
// QDOAS ???     va_start(argList,function);
// QDOAS ???     vsprintf(errorBuf,errorString,argList);
// QDOAS ???     va_end(argList);
// QDOAS ???    }
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   if (strlen(errorString))
// QDOAS ???    {
// QDOAS ???     // Send error message to a message box
// QDOAS ???
// QDOAS ???     if ((errorType==ERROR_TYPE_WARNING) || (errorType==ERROR_TYPE_FILE))
// QDOAS ???      sprintf(errorTitle,"Warning     ");
// QDOAS ???     else if (errorType==ERROR_TYPE_FATAL)
// QDOAS ???      sprintf(errorTitle,"Fatal error ");
// QDOAS ???     else if (errorType==ERROR_TYPE_DEBUG)
// QDOAS ???      sprintf(errorTitle,"Debug       ");
// QDOAS ???
// QDOAS ???     #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???     if ((errorType==ERROR_TYPE_WARNING) || (errorType==ERROR_TYPE_FATAL))
// QDOAS ???      MessageBox(DOAS_hwndMain,errorBuf,errorTitle,(errorType&ERROR_TYPE_WARNING)?
// QDOAS ???                 MB_OK|MB_ICONEXCLAMATION:MB_OK|MB_ICONHAND);
// QDOAS ???     else if (errorType==ERROR_TYPE_OWNERDRAWN)
// QDOAS ???      rc=DialogBoxParam(DOAS_hInst,MAKEINTRESOURCE(DLG_MSG),DOAS_hwndMain,(DLGPROC)MSG_WndProc,(LPARAM)errorBuf);
// QDOAS ???     #endif
// QDOAS ???
// QDOAS ???     // Time
// QDOAS ???
// QDOAS ???     if (errorType==ERROR_TYPE_TIME)
// QDOAS ???      {
// QDOAS ???       time_t today;
// QDOAS ???       char dateString[40];
// QDOAS ???
// QDOAS ???       today=time(NULL);
// QDOAS ???       strftime(dateString,40,"%d/%m/%y %H:%M:%S",localtime(&today));
// QDOAS ???
// QDOAS ???       if ((fp=fopen(DOAS_logFile,"a+t"))!=NULL)
// QDOAS ???        {
// QDOAS ???         fprintf(fp,"%s",dateString);
// QDOAS ???         fclose(fp);
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Message
// QDOAS ???
// QDOAS ???     if ((fp=fopen(DOAS_logFile,"a+t"))!=NULL)
// QDOAS ???      {
// QDOAS ???       fprintf(fp,"%s%s%s\n",errorFunction,errorTitle,errorBuf);
// QDOAS ???       fclose(fp);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???     if ((errorId==ERROR_ID_FILE_NOT_FOUND) || (errorId==ERROR_ID_FILE_OPEN))
// QDOAS ???      {
// QDOAS ???       systemError=GetLastError();
// QDOAS ???
// QDOAS ???       FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM,
// QDOAS ???                     NULL,systemError,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),errorSystemString,MAX_ITEM_TEXT_LEN,NULL);
// QDOAS ???
// QDOAS ???       if ((fp=fopen(DOAS_logFile,"a+t"))!=NULL)
// QDOAS ???        {
// QDOAS ???         fprintf(fp,"System returns error code %d - %s",systemError,errorSystemString);
// QDOAS ???         fclose(fp);
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???     #endif
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   if ((errorType==ERROR_TYPE_FATAL) && (THRD_id>THREAD_TYPE_NONE))
// QDOAS ???    SetEvent(THRD_hEvents[THREAD_EVENT_STOP]);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ------------------------------------------
// QDOAS ??? // THRD_WaitEvent : Wait and process an event
// QDOAS ??? // ------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? INDEX THRD_WaitEvent(DWORD delay,INT moveFlag,INT incrementFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   HMENU hMenu,hSubMenu;
// QDOAS ???   LPARAM lParam;
// QDOAS ???   INDEX eventIndex;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   hMenu=GetMenu(DOAS_hwndMain);
// QDOAS ???   hSubMenu=GetSubMenu(hMenu,2);   // Menu 'Edit'
// QDOAS ???
// QDOAS ???   // Enable/Disable toolbar buttons
// QDOAS ???
// QDOAS ???   lParam=(LPARAM)MAKELONG((delay==INFINITE)?TRUE:FALSE,0);
// QDOAS ???
// QDOAS ???   SendMessage(TLBAR_hwnd,TB_ENABLEBUTTON,MENU_TOOL_PLAY,lParam);
// QDOAS ???   SendMessage(TLBAR_hwnd,TB_ENABLEBUTTON,MENU_TOOL_PAUSE,!lParam);
// QDOAS ???
// QDOAS ???   lParam=(LPARAM)MAKELONG(((delay==INFINITE)&&(moveFlag))?TRUE:FALSE,0);
// QDOAS ???
// QDOAS ???   EnableMenuItem(hSubMenu,MENU_EDIT_GOTO,(lParam)?MF_ENABLED:MF_GRAYED); // ((delay==INFINITE)&&(moveFlag))?MF_ENABLED:MF_GRAYED);
// QDOAS ???
// QDOAS ???   SendMessage(TLBAR_hwnd,TB_ENABLEBUTTON,MENU_TOOL_FIRST,lParam);
// QDOAS ???   SendMessage(TLBAR_hwnd,TB_ENABLEBUTTON,MENU_TOOL_LAST,lParam);
// QDOAS ???
// QDOAS ???   if (THRD_recordLast<32768)
// QDOAS ???    {
// QDOAS ???    	EnableWindow(TLBAR_hwndGoto,(delay==INFINITE)||(THRD_lastEvent==THREAD_EVENT_GOTO)?TRUE:FALSE);
// QDOAS ???
// QDOAS ???    	SendMessage(TLBAR_hwnd,TB_ENABLEBUTTON,MENU_TOOL_PREVIOUS,lParam);
// QDOAS ???     SendMessage(TLBAR_hwnd,TB_ENABLEBUTTON,MENU_TOOL_NEXT,lParam);
// QDOAS ???    }
// QDOAS ???   else
// QDOAS ???    EnableWindow(TLBAR_hwndGoto,FALSE);
// QDOAS ???
// QDOAS ???   // Wait for event or time out
// QDOAS ???
// QDOAS ???   if (((eventIndex=WaitForMultipleObjects(THREAD_EVENT_MAX,THRD_hEvents,FALSE,delay))>=WAIT_OBJECT_0) &&
// QDOAS ???        (eventIndex<WAIT_OBJECT_0+THREAD_EVENT_MAX))
// QDOAS ???    eventIndex-=WAIT_OBJECT_0;
// QDOAS ???   else eventIndex=ITEM_NONE;
// QDOAS ???
// QDOAS ???   // Process event
// QDOAS ???
// QDOAS ???   switch(eventIndex)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case THREAD_EVENT_GOTO :
// QDOAS ???      {
// QDOAS ???       THRD_lastEvent=THREAD_EVENT_GOTO;
// QDOAS ???
// QDOAS ??? //      if (THRD_goto.indexType)
// QDOAS ???
// QDOAS ??? //      if (!THRD_goto.indexType)
// QDOAS ???        {
// QDOAS ???         if (!THRD_goto.increment)
// QDOAS ???          THRD_increment=0;
// QDOAS ???         else
// QDOAS ???          THRD_increment=(THRD_goto.increment>0)?1:-1;
// QDOAS ???
// QDOAS ???         if ((THRD_goto.indexRecord<THRD_recordFirst) || (THRD_goto.indexRecord>THRD_recordLast))
// QDOAS ???          {
// QDOAS ???           if (!THRD_goto.flag)
// QDOAS ???            {
// QDOAS ???             if (THRD_goto.indexRecord<THRD_recordFirst)
// QDOAS ???              THRD_goto.indexRecord=THRD_recordFirst;
// QDOAS ???             else if (THRD_goto.indexRecord>THRD_recordLast)
// QDOAS ???              THRD_goto.indexRecord=THRD_recordLast;
// QDOAS ???
// QDOAS ???             THRD_indexRecordCurrent=THRD_goto.indexRecord;
// QDOAS ???             THRD_goto.flag=1;
// QDOAS ???            }
// QDOAS ???           else
// QDOAS ???            THRD_setOriginalFlag=(ThrdNextRecord())?(UCHAR)0:(UCHAR)1;
// QDOAS ???          }
// QDOAS ???         else
// QDOAS ???          {
// QDOAS ???           THRD_indexRecordCurrent=THRD_goto.indexRecord;
// QDOAS ???           THRD_goto.flag=0;
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case THREAD_EVENT_FIRST :
// QDOAS ???      {
// QDOAS ???       THRD_resetFlag=1;
// QDOAS ???
// QDOAS ???       if (THRD_lastEvent!=THREAD_EVENT_FIRST)
// QDOAS ???        {
// QDOAS ???         THRD_increment=1;
// QDOAS ???         THRD_lastEvent=THREAD_EVENT_FIRST;
// QDOAS ???         THRD_indexRecordCurrent=THRD_recordFirst;
// QDOAS ???        }
// QDOAS ???       else
// QDOAS ???        {
// QDOAS ???         THRD_increment=-1;
// QDOAS ???         THRD_setOriginalFlag=(ThrdNextRecord())?(UCHAR)0:(UCHAR)1;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case THREAD_EVENT_PREVIOUS :
// QDOAS ???      {
// QDOAS ???       THRD_increment=-1;
// QDOAS ???
// QDOAS ???       THRD_resetFlag=1;
// QDOAS ???       THRD_lastEvent=THREAD_EVENT_PREVIOUS;
// QDOAS ???       THRD_setOriginalFlag=(ThrdNextRecord())?(UCHAR)0:(UCHAR)1;
// QDOAS ???      }
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case THREAD_EVENT_NEXT :
// QDOAS ???      {
// QDOAS ???       THRD_increment=1;
// QDOAS ???
// QDOAS ???       THRD_resetFlag=1;
// QDOAS ???       THRD_lastEvent=THREAD_EVENT_NEXT;
// QDOAS ???       THRD_setOriginalFlag=(ThrdNextRecord())?(UCHAR)0:(UCHAR)1;
// QDOAS ???      }
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case THREAD_EVENT_LAST :
// QDOAS ???      {
// QDOAS ???       THRD_resetFlag=1;
// QDOAS ???
// QDOAS ???       if (THRD_lastEvent!=THREAD_EVENT_LAST)
// QDOAS ???        {
// QDOAS ???         THRD_increment=-1;
// QDOAS ???         THRD_lastEvent=THREAD_EVENT_LAST;
// QDOAS ???         THRD_indexRecordCurrent=THRD_recordLast;
// QDOAS ???        }
// QDOAS ???       else
// QDOAS ???        {
// QDOAS ???         THRD_increment=1;
// QDOAS ???         THRD_lastEvent=THREAD_EVENT_NEXT;
// QDOAS ???         THRD_setOriginalFlag=(ThrdNextRecord())?(UCHAR)0:(UCHAR)1;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case THREAD_EVENT_PLAY :
// QDOAS ???      {
// QDOAS ???       THRD_increment=1;
// QDOAS ???       THRD_resetFlag=0;
// QDOAS ???       THRD_setOriginalFlag=(UCHAR)0;
// QDOAS ???
// QDOAS ???       if ((THRD_id==THREAD_TYPE_SPECTRA) ||
// QDOAS ???          ((THRD_lastEvent==THREAD_EVENT_PLAY) && incrementFlag))
// QDOAS ???        THRD_setOriginalFlag=(ThrdNextRecord())?(UCHAR)0:(UCHAR)1;
// QDOAS ???
// QDOAS ???       THRD_lastEvent=THREAD_EVENT_PLAY;
// QDOAS ???      }
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case THREAD_EVENT_PAUSE :
// QDOAS ???      THRD_lastEvent=THREAD_EVENT_PAUSE;
// QDOAS ???      THRD_increment=THRD_resetFlag=0;
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case THREAD_EVENT_STOP_PROGRAM :
// QDOAS ???      THRD_increment=THRD_resetFlag=0;
// QDOAS ???      THRD_endProgram=1;
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case THREAD_EVENT_STOP :
// QDOAS ???      THRD_increment=THRD_resetFlag=0;
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     default :
// QDOAS ???      {
// QDOAS ???       if (incrementFlag)
// QDOAS ???        THRD_setOriginalFlag=(ThrdNextRecord())?(UCHAR)0:(UCHAR)1;
// QDOAS ???      }
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   SendMessage(TLBAR_hwndGoto,UDM_SETPOS,0L,MAKELONG(THRD_indexRecordCurrent,0));
// QDOAS ???
// QDOAS ???   return eventIndex;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #else
// QDOAS ???
// QDOAS ??? INDEX THRD_WaitEvent(DWORD delay,INT moveFlag,INT incrementFlag)
// QDOAS ???  {
// QDOAS ???   if (incrementFlag)
// QDOAS ???    THRD_setOriginalFlag=(ThrdNextRecord())?(UCHAR)0:(UCHAR)1;
// QDOAS ???
// QDOAS ???   return ITEM_NONE;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif

// ===============
// DATA PROCESSING
// ===============

// ------------------------------------------------------------------
// THRD_ResetSpecInfo : Release or reset data hold by the last thread
// ------------------------------------------------------------------

void THRD_ResetSpecInfo(SPEC_INFO *pSpecInfo)
 {
  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("THRD_ResetSpecInfo",DEBUG_FCTTYPE_FILE);
  #endif

  // Release buffers

  if (pSpecInfo->lembda!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","lembda",pSpecInfo->lembda,0);
  if (pSpecInfo->instrFunction!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","instrFunction",pSpecInfo->instrFunction,0);
  if (pSpecInfo->spectrum!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","spectrum",pSpecInfo->spectrum,0);
  if (pSpecInfo->sigmaSpec!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","sigmaSpec",pSpecInfo->sigmaSpec,0);
  if (pSpecInfo->darkCurrent!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","darkCurrent",pSpecInfo->darkCurrent,0);
  if (pSpecInfo->specMax!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","specMax",pSpecInfo->specMax,0);
  if (pSpecInfo->varPix!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","varPix",pSpecInfo->varPix,0);
  if (pSpecInfo->dnl!=NULL)
   MEMORY_ReleaseDVector("THRD_ResetSpecInfo ","dnl",pSpecInfo->dnl,0);
  if (pSpecInfo->recordIndexes!=NULL)
   MEMORY_ReleaseBuffer("THRD_ResetSpecInfo ","recordIndexes",pSpecInfo->recordIndexes);

  CCD_ResetInstrumental(&pSpecInfo->ccd);

  // Reset structure

  memset(pSpecInfo,0,sizeof(SPEC_INFO));

  pSpecInfo->indexRecord=
  pSpecInfo->indexFile=
  pSpecInfo->indexProject=ITEM_NONE;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("THRD_ResetSpecInfo",0);
  #endif
 }

// -----------------------------------------------------
// ThrdInitSpecInfo : Allocate buffers for a new project
// -----------------------------------------------------

RC ThrdInitSpecInfo(SPEC_INFO *pSpecInfo,PROJECT *pProject)
 {
  // Declarations

  INT detectorSize;
  RC rc=ERROR_ID_NO;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ThrdInitSpecInfo",DEBUG_FCTTYPE_FILE);
  #endif

  // Initialize structure

  THRD_ResetSpecInfo(pSpecInfo);

  memcpy(&pSpecInfo->project,pProject,sizeof(PROJECT));

  // Spectra buffers allocation

  if (!(detectorSize=pProject->instrumental.detectorSize))
   rc=ERROR_SetLast("ThrdInitSpecInfo",ERROR_TYPE_WARNING,ERROR_ID_OUT_OF_RANGE,"Detector size",1,65535);

  else if (((pSpecInfo->lembda=MEMORY_AllocDVector("ThrdInitSpecInfo ","lembda",0,detectorSize-1))==NULL) ||
           ((pSpecInfo->spectrum=MEMORY_AllocDVector("ThrdInitSpecInfo ","spectrum",0,detectorSize-1))==NULL) ||

          (((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV) ||
            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_CCD_OHP_96) ||
            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_CCD_HA_94) ||
            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG) ||
            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_OLD) ||
            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_ULB) ||
            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDASI_EASOE)) &&

           ((pSpecInfo->recordIndexes=(ULONG *)MEMORY_AllocBuffer("ThrdInitSpecInfo ","recordIndexes",2001,sizeof(ULONG),0,MEMORY_TYPE_LONG))==NULL)) ||

          (((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI) ||
            (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&

           ((pSpecInfo->sigmaSpec=MEMORY_AllocDVector("ThrdInitSpecInfo ","sigmaSpec",0,detectorSize-1))==NULL)) ||
           ((pProject->spectra.darkFlag!=0) &&
           ((pSpecInfo->darkCurrent=MEMORY_AllocDVector("ThrdInitSpecInfo ","darkCurrent",0,detectorSize-1))==NULL)))

   rc=ERROR_ID_ALLOC;

  else

   if (pSpecInfo->darkCurrent!=NULL)
    VECTOR_Init(pSpecInfo->darkCurrent,(double)0.,detectorSize);

  // Return

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ThrdInitSpecInfo",rc);
  #endif

  return rc;
 }

// -----------------------------------------------------------------------------------------
// THRD_CopySpecInfo : Make a copy of data on the current spectra file into another structure
// -----------------------------------------------------------------------------------------

RC THRD_CopySpecInfo(SPEC_INFO *pSpecInfoTarget,SPEC_INFO *pSpecInfoSource)
 {
  // Declaration

  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Buffer allocation for instrumental functions

  if (((pSpecInfoSource->instrFunction!=NULL) && (pSpecInfoTarget->instrFunction==NULL) &&
      ((pSpecInfoTarget->instrFunction=(double *)MEMORY_AllocDVector("THRD_CopySpecInfo ","instrFunction",0,NDET-1))==NULL)) ||
      ((pSpecInfoSource->varPix!=NULL) && (pSpecInfoTarget->varPix==NULL) &&
      ((pSpecInfoTarget->varPix=(double *)MEMORY_AllocDVector("THRD_CopySpecInfo ","varPix",0,NDET-1))==NULL)) ||
      ((pSpecInfoSource->dnl!=NULL) && (pSpecInfoTarget->dnl==NULL) &&
      ((pSpecInfoTarget->dnl=(double *)MEMORY_AllocDVector("THRD_CopySpecInfo ","dnl",0,NDET-1))==NULL)) ||
      ((pSpecInfoSource->specMax!=NULL) && (pSpecInfoTarget->specMax==NULL) &&
      ((pSpecInfoTarget->specMax=(double *)MEMORY_AllocDVector("THRD_CopySpecInfo ","specMax",0,NDET-1))==NULL)) ||
      ((pSpecInfoSource->recordIndexes!=NULL) && (pSpecInfoTarget->recordIndexes==NULL) &&
      ((pSpecInfoTarget->recordIndexes=(ULONG *)MEMORY_AllocBuffer("THRD_CopySpecInfo","recordIndexes",
       (pSpecInfoTarget->recordIndexesSize=pSpecInfoSource->recordIndexesSize),sizeof(ULONG),0,MEMORY_TYPE_LONG))==NULL)) ||
      ((pSpecInfoSource->ccd.vip.matrix!=NULL) && (pSpecInfoTarget->ccd.vip.matrix==NULL) &&
      ((rc=MATRIX_Copy(&pSpecInfoTarget->ccd.vip,&pSpecInfoSource->ccd.vip,"THRD_CopySpecInfo "))!=ERROR_ID_NO)))

   rc=ERROR_ID_ALLOC;

  else
   {
    // File name

    strcpy(pSpecInfoTarget->fileName,pSpecInfoSource->fileName);

    // Spectra vectors

    if ((pSpecInfoTarget->lembda!=NULL) && (pSpecInfoSource->lembda!=NULL))
     memcpy(pSpecInfoTarget->lembda,pSpecInfoSource->lembda,sizeof(double)*NDET);
    if ((pSpecInfoTarget->instrFunction!=NULL) && (pSpecInfoSource->instrFunction!=NULL))
     memcpy(pSpecInfoTarget->instrFunction,pSpecInfoSource->instrFunction,sizeof(double)*NDET);
    if ((pSpecInfoTarget->spectrum!=NULL) && (pSpecInfoSource->spectrum!=NULL))
     memcpy(pSpecInfoTarget->spectrum,pSpecInfoSource->spectrum,sizeof(double)*NDET);
    if ((pSpecInfoTarget->sigmaSpec!=NULL) && (pSpecInfoSource->sigmaSpec!=NULL))
     memcpy(pSpecInfoTarget->sigmaSpec,pSpecInfoSource->sigmaSpec,sizeof(double)*NDET);
    if ((pSpecInfoTarget->darkCurrent!=NULL) && (pSpecInfoSource->darkCurrent!=NULL))
     memcpy(pSpecInfoTarget->darkCurrent,pSpecInfoSource->darkCurrent,sizeof(double)*NDET);
    if ((pSpecInfoTarget->varPix!=NULL) && (pSpecInfoSource->varPix!=NULL))
     memcpy(pSpecInfoTarget->varPix,pSpecInfoSource->varPix,sizeof(double)*NDET);
    if ((pSpecInfoTarget->dnl!=NULL) && (pSpecInfoSource->dnl!=NULL))
     memcpy(pSpecInfoTarget->dnl,pSpecInfoSource->dnl,sizeof(double)*NDET);
    if ((pSpecInfoTarget->specMax!=NULL) && (pSpecInfoSource->specMax!=NULL))
     memcpy(pSpecInfoTarget->specMax,pSpecInfoSource->specMax,sizeof(double)*NDET);
    if ((pSpecInfoTarget->recordIndexes!=NULL) && (pSpecInfoSource->recordIndexes!=NULL))
     memcpy(pSpecInfoTarget->recordIndexes,pSpecInfoSource->recordIndexes,sizeof(ULONG)*pSpecInfoSource->recordIndexesSize);

    // Other fields

    memcpy(&pSpecInfoTarget->recordNumber,&pSpecInfoSource->recordNumber,
           sizeof(SPEC_INFO)-
           sizeof(PROJECT)-                      // project
           (MAX_STR_LEN+1)-                      // file name
           8*sizeof(double *)-sizeof(ULONG *)-   // spectra buffers+record indexes
           sizeof(CCD));
   }

  // Return

  return rc;
 }

// QDOAS ??? // ----------------------------------------------------------------------
// QDOAS ??? // ThrdWriteSpecInfo : Write data on current spectrum in a temporary file
// QDOAS ??? // ----------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? RC ThrdWriteSpecInfo(SPEC_INFO *pSpecInfo)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   PRJCT_INSTRUMENTAL *pInstrumental;
// QDOAS ???   UCHAR        fileName[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   SHORT_DATE  *pDay;                      // pointer to measurement date
// QDOAS ???   struct time *pTime;                     // pointer to measurement date
// QDOAS ???   INDEX        indexTabFeno;              // browse analysis windows
// QDOAS ???   FENO        *pTabFeno;                  // pointer to the current analysis window
// QDOAS ???   FILE        *fp;                        // pointer to temporary file
// QDOAS ???   PROJECT     *pProject;
// QDOAS ???   RC           rc;                        // return code
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   pProject=&pSpecInfo->project;
// QDOAS ???   pInstrumental=&pProject->instrumental;
// QDOAS ???   rc=0;
// QDOAS ???
// QDOAS ???   if (pProject->spectra.displayDataFlag && ((fp=fopen(DOAS_tmpFile,"w+t"))!=NULL))
// QDOAS ???    {
// QDOAS ???     // Initializations
// QDOAS ???
// QDOAS ???     pDay=&pSpecInfo->present_day;
// QDOAS ???     pTime=&pSpecInfo->present_time;
// QDOAS ???
// QDOAS ???     // Output data on current spectrum
// QDOAS ???
// QDOAS ???     if ((THRD_id==THREAD_TYPE_ANALYSIS) || (THRD_id==THREAD_TYPE_KURUCZ))
// QDOAS ???      {
// QDOAS ???       for (indexTabFeno=0;indexTabFeno<NFeno;indexTabFeno++)
// QDOAS ???        {
// QDOAS ???         pTabFeno=&TabFeno[indexTabFeno];
// QDOAS ???
// QDOAS ???         if (!pTabFeno->hidden)
// QDOAS ???          fprintf(fp,"Reference for %s analysis window : %d/%d SZA : %g\n",pTabFeno->windowName,pTabFeno->indexRef,
// QDOAS ???                 (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC)?pSpecInfo->recordNumber:ITEM_NONE,pTabFeno->Zm);
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       fprintf(fp,"\n");
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     fprintf(fp,"Project\t\t\t%s\n",pProject->name);
// QDOAS ???
// QDOAS ???     fprintf(fp,"File\t\t\t%s\n",
// QDOAS ???           ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)||
// QDOAS ???            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)||
// QDOAS ???            (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OPUS))?PATH_fileSpectra:pSpecInfo->fileName);
// QDOAS ???
// QDOAS ???     if (strlen(pInstrumental->instrFunction))
// QDOAS ???      {
// QDOAS ???       if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_ULB)
// QDOAS ???        fprintf(fp,"Dark Current\t\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->instrFunction,1));
// QDOAS ???       else if (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC)
// QDOAS ???        fprintf(fp,"Instr. function\t\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->instrFunction,1));
// QDOAS ???       else if (((pInstrumental->mfcMaskSpec!=0) && ((UINT)MFC_header.ty==pInstrumental->mfcMaskSpec)) ||
// QDOAS ???                ((pInstrumental->mfcMaskSpec==0) &&
// QDOAS ???                ((MFC_header.wavelength1==pInstrumental->mfcMaskInstr) ||
// QDOAS ???                 (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.))))
// QDOAS ???        fprintf(fp,"Instr. function\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileInstr,1));
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (strlen(pInstrumental->vipFile))
// QDOAS ???      {
// QDOAS ???       if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)
// QDOAS ???        fprintf(fp,"Dark current\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileDark,1));
// QDOAS ???       else if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) &&
// QDOAS ???             (((pInstrumental->mfcMaskSpec!=0) && (((UINT)MFC_header.ty==pInstrumental->mfcMaskSpec) || ((UINT)MFC_header.ty==pInstrumental->mfcMaskInstr))) ||
// QDOAS ???               ((pInstrumental->mfcMaskSpec==0) &&
// QDOAS ???               ((MFC_header.wavelength1==pInstrumental->mfcMaskInstr) ||
// QDOAS ???                (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.)))))
// QDOAS ???        fprintf(fp,"Dark current\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileDark,1));
// QDOAS ???       else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV)
// QDOAS ???        fprintf(fp,"Straylight correction\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->vipFile,1));
// QDOAS ???       else
// QDOAS ???        fprintf(fp,"Interpixel\t\t\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->vipFile,1));
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (strlen(pInstrumental->dnlFile))
// QDOAS ???      {
// QDOAS ???       if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)
// QDOAS ???        fprintf(fp,"Offset\t\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileOffset,1));
// QDOAS ???       else if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) &&
// QDOAS ???              (((MFC_header.ty!=0) &&
// QDOAS ???              (((UINT)MFC_header.ty==pInstrumental->mfcMaskSpec) ||
// QDOAS ???               ((UINT)MFC_header.ty==pInstrumental->mfcMaskDark) ||
// QDOAS ???               ((UINT)MFC_header.ty==pInstrumental->mfcMaskInstr))) ||
// QDOAS ???               ((pInstrumental->mfcMaskSpec==0) &&
// QDOAS ???               ((MFC_header.wavelength1==pInstrumental->mfcMaskDark) ||
// QDOAS ???                (MFC_header.wavelength1==pInstrumental->mfcMaskInstr)||
// QDOAS ???                (fabs((double)(MFC_header.wavelength1-(float)pInstrumental->wavelength))<(double)5.)))))
// QDOAS ???        fprintf(fp,"Offset\t\t\t%s\n",FILES_RebuildFileName(fileName,MFC_fileOffset,1));
// QDOAS ???       else
// QDOAS ???        fprintf(fp,"Non linearity\t\t%s\n",FILES_RebuildFileName(fileName,pInstrumental->dnlFile,1));
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GOME2)
// QDOAS ???      fprintf(fp,"Date and Time\t\t%02d/%02d/%d %02d:%02d:%02d\n",
// QDOAS ???                  pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
// QDOAS ???     else
// QDOAS ???      fprintf(fp,"Date and Time\t\t%02d/%02d/%d %02d:%02d:%02d.%06d\n",
// QDOAS ???                  pDay->da_day,pDay->da_mon,pDay->da_year,pTime->ti_hour,pTime->ti_min,pTime->ti_sec,GOME2_ms);
// QDOAS ???
// QDOAS ??? //    fprintf(fp,"%.3f -> %.3f \n",pSpecInfo->TimeDec,pSpecInfo->localTimeDec);
// QDOAS ???
// QDOAS ???     if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
// QDOAS ???         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD))
// QDOAS ???      {
// QDOAS ???       pTime=&pSpecInfo->startTime;
// QDOAS ???       fprintf(fp,"Start time\t\t\t%02d:%02d:%02d\n",pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
// QDOAS ???       pTime=&pSpecInfo->endTime;
// QDOAS ???       fprintf(fp,"End time\t\t\t%02d:%02d:%02d\n",pTime->ti_hour,pTime->ti_min,pTime->ti_sec);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
// QDOAS ???         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF))
// QDOAS ???      fprintf(fp,"Record\t\t\t%d/%d\n",pSpecInfo->indexRecord,pSpecInfo->recordNumber);
// QDOAS ???     else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI)
// QDOAS ???      {
// QDOAS ???      	if (pSpecInfo->project.instrumental.averageFlag)
// QDOAS ???        fprintf(fp,"Record\t\t\t%d/%d (%d spectra averaged)\n",
// QDOAS ???                pSpecInfo->indexRecord,pSpecInfo->recordNumber,pSpecInfo->omi.omiNumberOfSpectraPerTrack);
// QDOAS ???      	else
// QDOAS ???        fprintf(fp,"Record\t\t\t%d/%d (track %d/%d, spectrum %d/%d)\n",
// QDOAS ???                pSpecInfo->indexRecord,pSpecInfo->recordNumber,
// QDOAS ???                pSpecInfo->omi.omiTrackIndex,pSpecInfo->omi.omiNumberOfTracks,
// QDOAS ???                pSpecInfo->omi.omiSpecIndex,pSpecInfo->omi.omiNumberOfSpectraPerTrack);
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      fprintf(fp,"Record\t\t\t%d/%d\n",pSpecInfo->indexRecord,pSpecInfo->recordNumber);
// QDOAS ???
// QDOAS ???     if (strlen(pSpecInfo->Nom))
// QDOAS ???      fprintf(fp,"Record name\t\t%s\n",pSpecInfo->Nom);
// QDOAS ???
// QDOAS ???     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
// QDOAS ???      {
// QDOAS ???       if (strlen(MFC_header.FileName))
// QDOAS ???        fprintf(fp,"Spectrum\t\t\t%s\n",MFC_header.FileName);
// QDOAS ???       if (strlen(MFC_header.specname))
// QDOAS ???        fprintf(fp,"Record name\t\t%s\n",MFC_header.specname);
// QDOAS ???       if (strlen(MFC_header.site))
// QDOAS ???        fprintf(fp,"Site\t\t\t%s\n",MFC_header.site);
// QDOAS ???       if (strlen(MFC_header.spectroname))
// QDOAS ???        fprintf(fp,"Spectro name\t\t%s\n",MFC_header.spectroname);
// QDOAS ???       if (strlen(MFC_header.scan_dev))
// QDOAS ???        fprintf(fp,"Scan device\t\t%s\n",MFC_header.scan_dev);
// QDOAS ???       if (strlen(MFC_header.first_line))
// QDOAS ???        fprintf(fp,"%s\n",MFC_header.first_line);
// QDOAS ???       if (strlen(MFC_header.spaeter))
// QDOAS ???        fprintf(fp,"%s\n",MFC_header.spaeter);
// QDOAS ???
// QDOAS ???       if (strlen(MFC_header.backgrnd))
// QDOAS ???        fprintf(fp,"Background\t\t%s\n",MFC_header.backgrnd);
// QDOAS ???
// QDOAS ???       fprintf(fp,"ty mask\t\t\t%d\n",MFC_header.ty);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
// QDOAS ???         (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN))
// QDOAS ???      {
// QDOAS ???       fprintf(fp,"Orbit Number\t\t%d\n",pSpecInfo->gome.orbitNumber+1);
// QDOAS ???       fprintf(fp,"Pixel Number\t\t%d\n",pSpecInfo->gome.pixelNumber);
// QDOAS ???       fprintf(fp,"Pixel Type\t\t%d\n",pSpecInfo->gome.pixelType);
// QDOAS ???      }
// QDOAS ???     else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS)
// QDOAS ???      fprintf(fp,"Orbit Number\t\t%d\n",pSpecInfo->scia.orbitNumber);
// QDOAS ???
// QDOAS ???     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
// QDOAS ???      fprintf(fp,"Calibration parameters\t%.2f %.3e %.3e %.3e\n",pSpecInfo->wavelength1,pSpecInfo->dispersion[0],
// QDOAS ???                  pSpecInfo->dispersion[1],pSpecInfo->dispersion[2]);
// QDOAS ???
// QDOAS ???     fprintf(fp,"Solar Zenith angle\t\t%#.3f �\n",pSpecInfo->Zm);
// QDOAS ???
// QDOAS ???     if ((pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GDP_ASCII) &&
// QDOAS ???         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GDP_BIN) &&
// QDOAS ???         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_HDF) &&
// QDOAS ???         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS) &&
// QDOAS ???         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_OMI) &&
// QDOAS ???         (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_GOME2))
// QDOAS ???      {
// QDOAS ???       fprintf(fp,"Exposure time\t\t%.3f sec\n",pSpecInfo->Tint);
// QDOAS ???       fprintf(fp,"Scans taken into account\t%d\n",pSpecInfo->NSomme);
// QDOAS ???       fprintf(fp,"Elevation viewing angle\t%.3f �\n",pSpecInfo->elevationViewAngle);
// QDOAS ???       fprintf(fp,"Azimuth viewing angle\t%.3f �\n",pSpecInfo->azimuthViewAngle);
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???      	fprintf(fp,"Solar Azimuth angle\t%.3f �\n",pSpecInfo->Azimuth);
// QDOAS ???      	fprintf(fp,"Viewing Zenith angle\t%.3f �\n",pSpecInfo->zenithViewAngle);
// QDOAS ???      	fprintf(fp,"Viewing Azimuth angle\t%.3f �\n",pSpecInfo->azimuthViewAngle);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (pSpecInfo->rejected>0)
// QDOAS ???      fprintf(fp,"Rejected scans\t\t%d\n",pSpecInfo->rejected);
// QDOAS ???     if (pSpecInfo->TDet!=(double)0.)
// QDOAS ???      fprintf(fp,"Detector temperature\t%.3f �\n",pSpecInfo->TDet);
// QDOAS ???
// QDOAS ???     if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_ULB)
// QDOAS ???      {
// QDOAS ???       fprintf(fp,"NTracks\t\t\t%d \n",pSpecInfo->NTracks);
// QDOAS ???       fprintf(fp,"Grating\t\t\t%d \n",pSpecInfo->NGrating);
// QDOAS ???       fprintf(fp,"Centre Wavelength\t\t%.1f nm\n",pSpecInfo->Nanometers);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (pSpecInfo->longitude!=(double)0.)
// QDOAS ???      fprintf(fp,"Longitude\t\t%.3f �\n",pSpecInfo->longitude);
// QDOAS ???     if (pSpecInfo->latitude!=(double)0.)
// QDOAS ???      fprintf(fp,"Latitude\t\t\t%.3f �\n",pSpecInfo->latitude);
// QDOAS ???     if (pSpecInfo->altitude!=(double)0.)
// QDOAS ???      fprintf(fp,"Altitude\t\t\t%.3f km\n",pSpecInfo->altitude);
// QDOAS ???
// QDOAS ???     if (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
// QDOAS ???          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)) &&
// QDOAS ???         ((pSpecInfo->aMoon!=(double)0.) || (pSpecInfo->hMoon!=(double)0.) || (pSpecInfo->fracMoon!=(double)0.)))
// QDOAS ???      {
// QDOAS ???       fprintf(fp,"Moon azimuthal angle\t%.3f �\n",pSpecInfo->aMoon);
// QDOAS ???       fprintf(fp,"Moon elevation\t\t%.3f �\n",pSpecInfo->hMoon);
// QDOAS ???       fprintf(fp,"Moon illuminated fraction\t%.3f �\n",pSpecInfo->fracMoon);
// QDOAS ???      }
// QDOAS ???     else if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN)
// QDOAS ???      {
// QDOAS ???       GOME_ORBIT_FILE *pOrbitFile;
// QDOAS ???       pOrbitFile=&GDP_BIN_orbitFiles[GDP_BIN_currentFileIndex];
// QDOAS ???
// QDOAS ???       fprintf(fp,"Cloud fraction\t\t%.2f\n",(float)pOrbitFile->gdpBinSpectrum.cloudFraction*0.01);
// QDOAS ???       fprintf(fp,"Cloud top pressure\t\t%.2f\n",(float)pOrbitFile->gdpBinSpectrum.cloudTopPressure*0.01);
// QDOAS ???       fprintf(fp,"[O3 VCD]\t\t\t%.2f DU\n",(float)pOrbitFile->gdpBinSpectrum.o3*0.01);
// QDOAS ???       fprintf(fp,"[No2 VCD]\t\t%.2e mol/cm2\n",(float)pOrbitFile->gdpBinSpectrum.no2*1.e13);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     fprintf(fp,"\n\n");
// QDOAS ???     fclose(fp);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Send data to MDI child window
// QDOAS ???
// QDOAS ???   THRD_LoadData();
// QDOAS ???
// QDOAS ???   // Make a copy of data on current spectra for display processing
// QDOAS ???
// QDOAS ???   if ((pSpecInfo->indexProject==THRD_specInfo.indexProject) ||
// QDOAS ???      !(rc=ThrdInitSpecInfo(&THRD_specInfo,&pSpecInfo->project)))
// QDOAS ???
// QDOAS ???    rc=THRD_CopySpecInfo(&THRD_specInfo,pSpecInfo);
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif

// --------------------------------------------------
// ThrdLoadInstrumental : Load instrumental functions
// --------------------------------------------------

RC ThrdLoadInstrumental(SPEC_INFO *pSpecInfo,UCHAR *instrFile,INT fileType)
 {
  // Declarations

  double *lembda,*instrFunction,*instrDeriv2,*function;
  UCHAR str[MAX_ITEM_TEXT_LEN+1],fileName[MAX_ITEM_TEXT_LEN+1],*ptr;
  PRJCT_INSTRUMENTAL *pInstrumental;
  FILE *gp;
  INDEX i;
  RC rc;

  // Initializations

  pInstrumental=&pSpecInfo->project.instrumental;
  FILES_RebuildFileName(fileName,instrFile,1);

  lembda=instrFunction=instrDeriv2=function=NULL;
  rc=ERROR_ID_NO;
  ptr=NULL;
  gp=NULL;

  if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG_ULB) ||
      (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_ULB) ||
      (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_CCD_EEV))

   rc=CCD_LoadInstrumental(pSpecInfo);

  // Allocate buffers

  else if (((function=(double *)MEMORY_AllocDVector("ThrdLoadInstrumental ","function",0,NDET-1))==NULL) ||
           ((lembda=(double *)MEMORY_AllocDVector("ThrdLoadInstrumental ","lembda",0,NDET-1))==NULL) ||
           ((instrFunction=(double *)MEMORY_AllocDVector("ThrdLoadInstrumental ","instrFunction",0,NDET-1))==NULL) ||
           ((instrDeriv2=(double *)MEMORY_AllocDVector("ThrdLoadInstrumental ","instrDeriv2",0,NDET-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    VECTOR_Init(function,((fileType!=FILE_TYPE_INSTR) && ((fileType!=FILE_TYPE_INTERPIXEL) ||
                          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
                          (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)))?(double)0.:(double)1.,NDET);

    // Load file

    if ((gp=fopen(fileName,"rt"))==NULL)
     rc=ERROR_SetLast("ThrdLoadInstrumental",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,fileName);
    else if ((fileType==FILE_TYPE_INSTR) && (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC) && (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC_STD))
     {
      for (i=0;(i<NDET) && fgets(str,MAX_ITEM_TEXT_LEN,gp);)
       if ((strchr(str,';')==NULL) && (strchr(str,'*')==NULL))
        {
         sscanf(str,"%lf %lf",&lembda[i],&instrFunction[i]);
         i++;
        }

      if (!SPLINE_Deriv2(lembda,instrFunction,instrDeriv2,NDET,"ThrdLoadInstrumental "))

       rc=SPLINE_Vector(lembda,instrFunction,instrDeriv2,NDET,pSpecInfo->lembda,function,NDET,SPLINE_CUBIC,"ThrdLoadInstrumental ");
     }
    else if ((pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC) && (pInstrumental->readOutFormat!=PRJCT_INSTR_FORMAT_MFC_STD))
     {
      for (i=0;(i<NDET) && fgets(str,MAX_ITEM_TEXT_LEN,gp);)
       if ((strchr(str,';')==NULL) && (strchr(str,'*')==NULL))
        {
         sscanf(str,"%lf",&function[i]);
         i++;
        }
     }

    else if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) || (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD))
     {
      TBinaryMFC *pMfc;
      UINT mask;

      mask=0;
      pMfc=NULL;

      switch(fileType)
       {
     // -----------------------------------------------------------------------
        case FILE_TYPE_INSTR :
         pMfc=&MFC_headerInstr;
         mask=pInstrumental->mfcMaskInstr;
         ptr=MFC_fileInstr;
        break;
     // -----------------------------------------------------------------------
        case FILE_TYPE_INTERPIXEL :
         pMfc=&MFC_headerDrk;
         mask=pInstrumental->mfcMaskDark;
         ptr=MFC_fileDark;
        break;
     // -----------------------------------------------------------------------
        case FILE_TYPE_NOT_LINEARITY :
         pMfc=&MFC_headerOff;
         mask=pInstrumental->mfcMaskOffset;
         ptr=MFC_fileOffset;
        break;
     // -----------------------------------------------------------------------
      }

      if (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) &&
          !(rc=MFC_ReadRecord(fileName,pMfc,function,
           &MFC_headerDrk,(fileType==FILE_TYPE_INSTR)?pSpecInfo->varPix:NULL,
           &MFC_headerOff,(fileType!=FILE_TYPE_NOT_LINEARITY)?pSpecInfo->dnl:NULL,mask,pInstrumental->mfcMaskSpec,pInstrumental->mfcRevert))) || // remove offset from dark current and instrumental function
          ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD) && !(rc=MFC_ReadRecordStd(pSpecInfo,fileName,pMfc,function,
           &MFC_headerDrk,(fileType==FILE_TYPE_INSTR)?pSpecInfo->varPix:NULL,
           &MFC_headerOff,(fileType!=FILE_TYPE_NOT_LINEARITY)?pSpecInfo->dnl:NULL))))

       FILES_CompactPath(ptr,fileName,1,1);
      else
       memset(ptr,0,MAX_ITEM_TEXT_LEN+1);
     }

    if (rc==ERROR_ID_FILE_END)
     rc=0;

    if (!rc)
     {
      switch(fileType)
       {
     // ---------------------------------------------------------------------------
        case FILE_TYPE_INSTR :
         pSpecInfo->instrFunction=function;
        break;
     // ---------------------------------------------------------------------------
        case FILE_TYPE_INTERPIXEL :
         pSpecInfo->varPix=function;
        break;
     // ---------------------------------------------------------------------------
        case FILE_TYPE_NOT_LINEARITY :
         pSpecInfo->dnl=function;
        break;
     // ---------------------------------------------------------------------------
        default :
         if (function!=NULL)
          {
           MEMORY_ReleaseDVector("ThrdLoadInstrumental ","function",function,0);
           function=NULL;
          }
        break;
     // ---------------------------------------------------------------------------
       }
     }
   }

  // Return

  if (gp!=NULL)
   fclose(gp);

  if (lembda!=NULL)
   MEMORY_ReleaseDVector("ThrdLoadInstrumental ","lembda",lembda,0);
  if (instrFunction!=NULL)
   MEMORY_ReleaseDVector("ThrdLoadInstrumental ","instrFunction",instrFunction,0);
  if (instrDeriv2!=NULL)
   MEMORY_ReleaseDVector("ThrdLoadInstrumental ","instrDeriv2",instrDeriv2,0);
  if (rc && (function!=NULL))
   MEMORY_ReleaseDVector("ThrdLoadInstrumental ","function",function,0);

  return rc;
 }

// --------------------------------
// ThrdLoadProject : Load a project
// --------------------------------

RC ThrdLoadProject(SPEC_INFO *pSpecInfo,PROJECT *projectList,INDEX indexProject)
 {
  // Declarations

  UCHAR str[MAX_ITEM_TEXT_LEN+1],calibrationFile[MAX_ITEM_TEXT_LEN+1];
  PROJECT *pProject;
  FILE *fp;
  INDEX i,indexSite;
  RC rc;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ThrdLoadProject",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  PATH_mfcFlag=PATH_UofTFlag=0;
  THRD_lastRefRecord=0;
  fp=NULL;

  // Allocate new buffers for project

  if (!(rc=ThrdInitSpecInfo(pSpecInfo,&projectList[indexProject])) &&
      !(rc=ThrdInitSpecInfo(&THRD_specInfo,&projectList[indexProject])) &&
      !(rc=ThrdInitSpecInfo(&THRD_refInfo,&projectList[indexProject])))
   {
    pProject=&pSpecInfo->project;
    NDET=pProject->instrumental.detectorSize;

    if (THRD_browseType==THREAD_BROWSE_EXPORT)
     {
      pProject->spectra.displayPause=0;
      pProject->spectra.displayDelay=0;
      pProject->spectra.displaySpectraFlag=0;
     }

    #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
    THRD_delay=(pProject->spectra.displayPause==0)?pProject->spectra.displayDelay*1000:INFINITE;
    #endif

    THRD_localNoon=12.;
    THRD_localShift=(int)(((indexSite=SITES_GetIndex(pProject->instrumental.observationSite))!=ITEM_NONE)?
                            floor(SITES_itemList[indexSite].longitude/15.):0); // 24./360.

//    THRD_localShift=((indexSite=SITES_GetIndex(pProject->instrumental.observationSite))!=ITEM_NONE)?
//                      SITES_itemList[indexSite].gmtShift:0;

    // Load calibration file

    FILES_RebuildFileName(calibrationFile,pProject->instrumental.calibrationFile,1);

    if ((fp=fopen(calibrationFile,"rt"))!=NULL)
     {
      for (i=0;i<NDET;)
       if (!fgets(str,MAX_ITEM_TEXT_LEN,fp))
        break;
       else if ((strchr(str,';')==NULL) && (strchr(str,'*')==NULL))
        {
         sscanf(str,"%lf",&pSpecInfo->lembda[i]);
         i++;
        }

      if (i!=NDET)
       rc=ERROR_SetLast("ThrdLoadProject",ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,calibrationFile);
     }
    else
     for (i=0;i<NDET;i++)
      pSpecInfo->lembda[i]=i+1;

    // Load instrumental functions

    if (!rc &&
       ((!strlen(pProject->instrumental.dnlFile) || !(rc=ThrdLoadInstrumental(pSpecInfo,pProject->instrumental.dnlFile,FILE_TYPE_NOT_LINEARITY))) &&
        (!strlen(pProject->instrumental.vipFile) || ((pProject->instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_CCD_OHP_96) && !(rc=ThrdLoadInstrumental(pSpecInfo,pProject->instrumental.vipFile,FILE_TYPE_INTERPIXEL)))) &&
        (!strlen(pProject->instrumental.instrFunction) || !(rc=ThrdLoadInstrumental(pSpecInfo,pProject->instrumental.instrFunction,FILE_TYPE_INSTR)))) &&

      // Load analysis and output data from project and analysis windows panels

        ((THRD_id==THREAD_TYPE_ANALYSIS) || (THRD_id==THREAD_TYPE_KURUCZ)) && !(rc=ANALYSE_LoadData(pSpecInfo,indexProject)))
     {
      #if defined(__DEBUG_) && __DEBUG_ && __DEBUG_DOAS_OUTPUT_
      DEBUG_Start(DOAS_dbgFile,"OUTPUT_RegisterData",DEBUG_FCTTYPE_FILE,5,DEBUG_DVAR_YES,1);
      #endif

      rc=OUTPUT_RegisterData(pSpecInfo);

      #if defined(__DEBUG_) && __DEBUG_ && __DEBUG_DOAS_OUTPUT_
      DEBUG_Stop("OUTPUT_RegisterData");
      THRD_ProcessLastError();
      #endif
     }
   }

  // Return

  if (fp!=NULL)
   fclose(fp);

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ThrdLoadProject",(RC)rc);
  #endif

  return rc;
 }

// QDOAS ??? // -----------------------------------------------------------------------------------------------------
// QDOAS ??? // THRD_LoadData : Load data on spectra and analysis from temporary file to informative MDI child window
// QDOAS ??? // -----------------------------------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? void THRD_LoadData(void)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR *textBuffer;
// QDOAS ???   LONG fileSize;
// QDOAS ???   FILE *fp;
// QDOAS ???   HWND hwndEdit;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   textBuffer=NULL;
// QDOAS ???   fp=NULL;
// QDOAS ???
// QDOAS ???   // Open and load file into buffer
// QDOAS ???
// QDOAS ???   if ((CHILD_list[CHILD_WINDOW_SPECINFO].hwndChild!=NULL) && ((hwndEdit=CHILD_list[CHILD_WINDOW_SPECINFO].hwndEdit)!=NULL))
// QDOAS ???    {
// QDOAS ???     if (((fp=fopen(DOAS_tmpFile,"rb"))!=NULL) && ((fileSize=STD_FileLength(fp))>0) &&
// QDOAS ???         ((textBuffer=MEMORY_AllocBuffer("THRD_LoadData ","textBuffer",4*(fileSize/4)+4,1,0,MEMORY_TYPE_STRING))!=NULL))
// QDOAS ???      {
// QDOAS ???       // Send text into client area of informative MDI child window
// QDOAS ???
// QDOAS ???       memset(textBuffer,0,4*(fileSize/4)+4); // fileSize+1);
// QDOAS ???       fread(textBuffer,fileSize-2,1,fp);
// QDOAS ???       SetWindowText(hwndEdit,textBuffer);
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      SetWindowText(hwndEdit,"");
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Release buffers
// QDOAS ???
// QDOAS ???   if (fp!=NULL)
// QDOAS ???    fclose(fp);
// QDOAS ???
// QDOAS ???   if (textBuffer!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("THRD_LoadData ","textBuffer",textBuffer);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif

// ===============
// FILE PROCESSING
// ===============

// --------------------------------------------------
// ThrdOddEvenCorrection : Odd/Even pixels correction
// --------------------------------------------------

RC THRD_OddEvenCorrection(double *lembdaData,double *specData,double *output,INT vectorSize)
 {
  // Declarations

  double *lembda,*spectrum,*spectrum2,*spec1,*spec2;
  INDEX i;
  RC rc;

  // Buffers allocation

  spectrum=spectrum2=spec1=spec2=NULL;

  if (((lembda=(double *)MEMORY_AllocDVector("ThrdOddEvenCorrection ","lembda",0,vectorSize-1))==NULL) ||
      ((spectrum=(double *)MEMORY_AllocDVector("ThrdOddEvenCorrection ","spectrum",0,vectorSize-1))==NULL) ||
      ((spectrum2=(double *)MEMORY_AllocDVector("ThrdOddEvenCorrection ","spectrum2",0,vectorSize-1))==NULL) ||
      ((spec1=(double *)MEMORY_AllocDVector("ThrdOddEvenCorrection ","spec1",0,vectorSize-1))==NULL) ||
      ((spec2=(double *)MEMORY_AllocDVector("ThrdOddEvenCorrection ","spec2",0,vectorSize-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    for (i=0;i<vectorSize/2;i++)
     {
      lembda[i]=lembdaData[(i<<1)];                  // odd pixels (0 based)
      lembda[vectorSize/2+i]=lembdaData[(i<<1)+1];         // even pixels

      spectrum[i]=specData[(i<<1)];                         // odd pixels (0 based)
      spectrum[vectorSize/2+i]=specData[(i<<1)+1];                // even pixels
     }

    if (!(rc=SPLINE_Deriv2(lembda,spectrum,spectrum2,vectorSize/2,"PDA_OddEvenCorrection ")) &&
        !(rc=SPLINE_Deriv2(lembda+(vectorSize/2),spectrum+(vectorSize/2),spectrum2+(vectorSize/2),vectorSize/2,"PDA_OddEvenCorrection (2) ")))
     {
      memcpy(spec1,specData,sizeof(double)*vectorSize);
      memcpy(spec2,specData,sizeof(double)*vectorSize);

      for (i=0;i<vectorSize/2;i++)

       if (((rc=SPLINE_Vector(lembda+(vectorSize/2),spectrum+(vectorSize/2),spectrum2+(vectorSize/2),vectorSize/2,&lembdaData[(i<<1)],&spec1[(i<<1)],1,SPLINE_CUBIC,"THRD_OddEvenCorrection "))!=0) ||
           ((rc=SPLINE_Vector(lembda,spectrum,spectrum2,vectorSize/2,&lembdaData[(i<<1)+1],&spec2[(i<<1)+1],1,SPLINE_CUBIC,"THRD_OddEvenCorrection "))!=0))

        break;

       else
        {
         output[(i<<1)]=(double)0.5*(spec1[(i<<1)]+spec2[(i<<1)]);
         output[(i<<1)+1]=(double)0.5*(spec1[(i<<1)+1]+spec2[(i<<1)+1]);
        }
     }
   }

  // Release allocated buffers

  if (lembda!=NULL)
   MEMORY_ReleaseDVector("ThrdOddEvenCorrection ","lembda",lembda,0);
  if (spectrum!=NULL)
   MEMORY_ReleaseDVector("ThrdOddEvenCorrection ","spectrum",spectrum,0);
  if (spectrum2!=NULL)
   MEMORY_ReleaseDVector("ThrdOddEvenCorrection ","spectrum2",spectrum2,0);
  if (spec1!=NULL)
   MEMORY_ReleaseDVector("ThrdOddEvenCorrection ","spec1",spec1,0);
  if (spec2!=NULL)
   MEMORY_ReleaseDVector("ThrdOddEvenCorrection ","spec2",spec2,0);

  // Return

  return rc;
 }

// ------------------------------------------------------------------
// THRD_SpectrumCorrection : Apply instrumental correction to spectra
// ------------------------------------------------------------------

RC THRD_SpectrumCorrection(SPEC_INFO *pSpecInfo,double *spectrum)
 {
  // Declaration

  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Odd even pixel correction

  if (pSpecInfo->project.lfilter.type==PRJCT_FILTER_TYPE_ODDEVEN)
   rc=THRD_OddEvenCorrection(pSpecInfo->lembda,spectrum,spectrum,NDET);

  // Return

  return rc;
 }

// QDOAS ??? // ----------------------------------------------
// QDOAS ??? // ThrdFileSetPointers : Initialize file pointers
// QDOAS ??? // ----------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? RC ThrdFileSetPointers(SPEC_INFO *pSpecInfo,UCHAR *newFileName,FILE *specFp,FILE *namesFp,FILE *darkFp)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR string[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   PROJECT *pProject;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Debugging
// QDOAS ???
// QDOAS ???   #if defined(__DEBUG_) && __DEBUG_
// QDOAS ???   DEBUG_FunctionBegin("ThrdFileSetPointers",DEBUG_FCTTYPE_FILE);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pProject=&pSpecInfo->project;
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   THRD_delay=(pProject->spectra.displayPause==0)?pProject->spectra.displayDelay*1000:INFINITE;
// QDOAS ???   #endif
// QDOAS ???   THRD_recordFirst=THRD_recordLast=THRD_indexRecordCurrent=ITEM_NONE;
// QDOAS ???
// QDOAS ???   thrdRefFlag=1;
// QDOAS ???   THRD_goto.flag=0;
// QDOAS ???   THRD_goto.increment=0;
// QDOAS ???
// QDOAS ???   if (PATH_fileNumber<=0)
// QDOAS ???    rc=ERROR_ID_FILE_EMPTY;
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     sprintf(string,"%s : Spectra No",pSpecInfo->fileName);
// QDOAS ???     pSpecInfo->recordNumber=0;
// QDOAS ???     pSpecInfo->lastSavedRecord=-1;
// QDOAS ???     THRD_refInfo.localCalDay=-1;
// QDOAS ???
// QDOAS ???     // Set spectra read out function
// QDOAS ???
// QDOAS ???     switch((INT)pProject->instrumental.readOutFormat)
// QDOAS ???      {
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_ASCII :
// QDOAS ???        rc=ASCII_Set(pSpecInfo,specFp);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_ACTON :
// QDOAS ???        rc=SetActon_Logger(pSpecInfo,specFp);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_PDASI_EASOE :
// QDOAS ???        rc=SetEASOE(pSpecInfo,specFp,namesFp);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_PDAEGG :
// QDOAS ???        rc=SetPDA_EGG(pSpecInfo,specFp,1);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_PDAEGG_OLD :
// QDOAS ???        rc=SetPDA_EGG(pSpecInfo,specFp,0);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_LOGGER :
// QDOAS ???        rc=SetPDA_EGG_Logger(pSpecInfo,specFp);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_PDAEGG_ULB :
// QDOAS ???        rc=SetPDA_EGG_Ulb(pSpecInfo,specFp);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_SAOZ_VIS :
// QDOAS ???        rc=SetSAOZ(pSpecInfo,specFp,VIS);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_SAOZ_UV :
// QDOAS ???        rc=SetSAOZ(pSpecInfo,specFp,UV);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_SAOZ_EFM :
// QDOAS ???        rc=SetSAOZEfm(pSpecInfo,specFp);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_MFC :
// QDOAS ???       case PRJCT_INSTR_FORMAT_MFC_STD :
// QDOAS ???        rc=SetMFC(pSpecInfo,specFp);
// QDOAS ???        if (!(rc=SetMFC(pSpecInfo,specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
// QDOAS ???         rc=MFC_LoadAnalysis(pSpecInfo);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_RASAS :
// QDOAS ???        rc=SetRAS(pSpecInfo,specFp);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_UOFT :
// QDOAS ???        rc=SetUofT(pSpecInfo,specFp);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_NOAA :
// QDOAS ???        rc=SetNOAA(pSpecInfo,specFp);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       #if defined (__INCLUDE_HDF_) && __INCLUDE_HDF_
// QDOAS ???       case PRJCT_INSTR_FORMAT_OMI :
// QDOAS ???        rc=OMI_SetHDF(pSpecInfo);
// QDOAS ???       break;
// QDOAS ???       #endif
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_CCD_EEV :
// QDOAS ???        rc=SetCCD_EEV(pSpecInfo,specFp,darkFp);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_CCD_OHP_96 :
// QDOAS ???        rc=SetCCD(pSpecInfo,specFp,0);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_CCD_HA_94 :
// QDOAS ???        rc=SetCCD(pSpecInfo,specFp,1);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_CCD_ULB :
// QDOAS ???        rc=SetCCD_Ulb(pSpecInfo,specFp);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_OPUS :
// QDOAS ???        rc=OPUS_Set(pSpecInfo,specFp);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_GDP_ASCII :
// QDOAS ???        if (!(rc=GDP_ASC_Set(pSpecInfo,specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
// QDOAS ???         rc=GDP_ASC_LoadAnalysis(pSpecInfo,specFp);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_GDP_BIN :
// QDOAS ???        if (!(rc=GDP_BIN_Set(pSpecInfo,specFp)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
// QDOAS ???         rc=GDP_BIN_LoadAnalysis(pSpecInfo,specFp);
// QDOAS ???
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       #if defined (__INCLUDE_HDF_) && __INCLUDE_HDF_
// QDOAS ???       case PRJCT_INSTR_FORMAT_SCIA_HDF :
// QDOAS ???        if (!(rc=SCIA_SetHDF(pSpecInfo)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
// QDOAS ???         rc=SCIA_LoadAnalysis(pSpecInfo);
// QDOAS ???       break;
// QDOAS ???       #endif
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_SCIA_PDS :
// QDOAS ???        if (!(rc=SCIA_SetPDS(pSpecInfo)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
// QDOAS ???         rc=SCIA_LoadAnalysis(pSpecInfo);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case PRJCT_INSTR_FORMAT_GOME2 :
// QDOAS ???        if (!(rc=GOME2_Set(pSpecInfo)) && (THRD_id!=THREAD_TYPE_SPECTRA) && (THRD_id!=THREAD_TYPE_NONE))
// QDOAS ???         rc=GOME2_LoadAnalysis(pSpecInfo);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       default :
// QDOAS ???        rc=ERROR_ID_FILE_FORMAT;
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???     }
// QDOAS ???
// QDOAS ???     if (rc==ERROR_ID_NO_REF)
// QDOAS ???      {
// QDOAS ???      	thrdRefFlag=0;
// QDOAS ???      	rc=ERROR_ID_NO;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Get indexes of records
// QDOAS ???
// QDOAS ???     if (rc<THREAD_EVENT_STOP)
// QDOAS ???      {
// QDOAS ???       THRD_indexRecordCurrent=ITEM_NONE;
// QDOAS ???
// QDOAS ???       // Prepare records read out
// QDOAS ???
// QDOAS ???       if (!(THRD_recordFirst=max(1,pSpecInfo->project.spectra.noMin)))
// QDOAS ???        THRD_recordFirst=1;
// QDOAS ???       if (!(THRD_recordLast=min(pSpecInfo->recordNumber,pSpecInfo->project.spectra.noMax)))
// QDOAS ???        THRD_recordLast=pSpecInfo->recordNumber;
// QDOAS ???
// QDOAS ???       if (!pSpecInfo->recordNumber)
// QDOAS ???        rc=ERROR_SetLast("ThrdFileSetPointers",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,pSpecInfo->fileName);
// QDOAS ???       else if ((THRD_recordFirst>pSpecInfo->recordNumber) || (THRD_recordLast<0))
// QDOAS ???        THRD_Error(ERROR_TYPE_WARNING,(rc=ERROR_ID_OUT_OF_RANGE),"ThrdFileSetPointers ",string,1,pSpecInfo->recordNumber);
// QDOAS ???       else if (THRD_increment>=0)
// QDOAS ???        THRD_indexRecordCurrent=THRD_recordFirst;
// QDOAS ???       else if (THRD_lastEvent!=THREAD_EVENT_FIRST)
// QDOAS ???        THRD_indexRecordCurrent=THRD_recordLast;
// QDOAS ???       else
// QDOAS ???        {
// QDOAS ???         THRD_indexRecordCurrent=THRD_recordFirst;
// QDOAS ???         THRD_increment=1;
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       #if defined (__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ???       SendMessage(TLBAR_hwndGoto,UDM_SETRANGE,0L,MAKELONG(THRD_recordLast,THRD_recordFirst));
// QDOAS ???       SendMessage(TLBAR_hwndGoto,UDM_SETPOS,0L,MAKELONG(THRD_indexRecordCurrent,0));
// QDOAS ???      	EnableWindow(TLBAR_hwndGoto,(pSpecInfo->recordNumber<32768L)?TRUE:FALSE);
// QDOAS ???      	#endif
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (rc && (rc!=ERROR_ID_PDS))
// QDOAS ???      rc=THRD_ProcessLastError();
// QDOAS ???
// QDOAS ??? //    if (rc==ERROR_ID_FILE_FORMAT)
// QDOAS ??? //     THRD_Error(ERROR_TYPE_WARNING,ERROR_ID_FILE_FORMAT,"",newFileName);
// QDOAS ???
// QDOAS ???     if (!rc && (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_ASCII))   // ??? To verify
// QDOAS ???      {
// QDOAS ???       THRD_refInfo.present_day.da_day=(char)-1;
// QDOAS ???       THRD_refInfo.present_day.da_mon=(char)-1;
// QDOAS ???       THRD_refInfo.present_day.da_year=(short)(-1);
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   #if defined(__DEBUG_) && __DEBUG_
// QDOAS ???   DEBUG_FunctionStop("ThrdFileSetPointers",rc);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }

double THRD_GetDist(double longit, double latit, double longitRef, double latitRef)
 {
  double degToRad,
         argCos,
         dist;

  degToRad=(double)PI2/360.;

  argCos=(double)cos(latit*degToRad)*cos(latitRef*degToRad)*
                (cos(longit*degToRad)*cos(longitRef*degToRad)+sin(longit*degToRad)*sin(longitRef*degToRad)) +
                 sin(latit*degToRad)*sin(latitRef*degToRad);

  if ((double)1.-fabs(argCos)>(double)1.e-12)
   dist=(double)6370.*acos(argCos);
  else
   dist=(argCos>(double)0.)?(double)0.:(double)6370.*PI;

  return dist;
 }

// ------------------------------------
// ThrdReadFile : Spectra file read out
// ------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC ThrdReadFile(SPEC_INFO *pSpecInfo,INT recordNo,INT dateFlag,INT localCalDay,FILE *specFp,FILE *namesFp,FILE *darkFp,INT specInfoFlag)
 {
  // Declarations

// QDOAS ???   UCHAR windowTitle[MAX_ITEM_TEXT_LEN+1];
  PROJECT *pProject;
  PRJCT_INSTRUMENTAL *pInstrumental;
  double *spectrum,degToRad;
  DWORD delay;
  INDEX i,oldNDET;
  UINT mfcMask;
  RC rc;

  // Initializations

  THRD_correction=((pSpecInfo->project.lfilter.type==PRJCT_FILTER_TYPE_ODDEVEN)||(pSpecInfo->instrFunction!=NULL))?1:0;
  pProject=&pSpecInfo->project;
  pInstrumental=&pProject->instrumental;
  degToRad=(double)PI2/360.;
  oldNDET=NDET;
  mfcMask=0;
  rc=ERROR_ID_NO;
  delay=0;

  // Fields initialization

  memset(pSpecInfo->Nom,0,20);

  pSpecInfo->Zm=-1.;
  pSpecInfo->Azimuth=-1.;
  pSpecInfo->SkyObs=8;
  pSpecInfo->ReguTemp=0.;
  pSpecInfo->TDet=0.;
  pSpecInfo->BestShift=0.;
  pSpecInfo->rejected=0;
  pSpecInfo->NTracks=0;
  pSpecInfo->Cic=0.;
  pSpecInfo->elevationViewAngle=-1.;

  pSpecInfo->longitude=0.;
  pSpecInfo->latitude=0.;
  pSpecInfo->altitude=0.;

  pSpecInfo->aMoon=0.;
  pSpecInfo->hMoon=0.;
  pSpecInfo->fracMoon=0.;

  if ((spectrum=(double *)MEMORY_AllocDVector("ThrdReadFile ","spectrum",0,NDET-1))==NULL)
   rc=ERROR_ID_ALLOC;
  else

   // Set spectra read out function

   switch((INT)pProject->instrumental.readOutFormat)
    {
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_ASCII :
      rc=ASCII_Read(pSpecInfo,recordNo,dateFlag,localCalDay,specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_ACTON :
      rc=ReliActon_Logger(pSpecInfo,recordNo,dateFlag,localCalDay,specFp,namesFp,darkFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_PDASI_EASOE :
      rc=ReliEASOE(pSpecInfo,recordNo,dateFlag,localCalDay,specFp,namesFp,darkFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_PDAEGG :
      rc=ReliPDA_EGG(pSpecInfo,recordNo,dateFlag,localCalDay,specFp,namesFp,darkFp,1);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_PDAEGG_OLD :
      rc=ReliPDA_EGG(pSpecInfo,recordNo,dateFlag,localCalDay,specFp,namesFp,darkFp,0);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_LOGGER :
      rc=ReliPDA_EGG_Logger(pSpecInfo,recordNo,dateFlag,localCalDay,specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_PDAEGG_ULB :
      rc=ReliPDA_EGG_Ulb(pSpecInfo,recordNo,dateFlag,localCalDay,specFp,namesFp,darkFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SAOZ_VIS :
      rc=ReliSAOZ(pSpecInfo,recordNo,dateFlag,localCalDay,specFp,namesFp,darkFp,VIS);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SAOZ_UV :
      rc=ReliSAOZ(pSpecInfo,recordNo,dateFlag,localCalDay,specFp,namesFp,darkFp,UV);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SAOZ_EFM :
      rc=ReliSAOZEfm(pSpecInfo,recordNo,dateFlag,localCalDay,specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_MFC :

      switch(THRD_browseType)
       {
     // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        case THREAD_BROWSE_MFC_OFFSET :
         mfcMask=pInstrumental->mfcMaskOffset;
        break;
     // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        case THREAD_BROWSE_MFC_DARK :
         mfcMask=pInstrumental->mfcMaskDark;
        break;
     // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        case THREAD_BROWSE_MFC_INSTR :
         mfcMask=pInstrumental->mfcMaskInstr;
        break;
     // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        default :
         mfcMask=pInstrumental->mfcMaskSpec;
        break;
     // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      }

      rc=ReliMFC(pSpecInfo,recordNo,dateFlag,localCalDay,specFp,mfcMask);

     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_MFC_STD :
      rc=ReliMFCStd(pSpecInfo,recordNo,dateFlag,localCalDay,specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_RASAS :
      rc=ReliRAS(pSpecInfo,recordNo,dateFlag,localCalDay,specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_UOFT :
      rc=ReliUofT(pSpecInfo,recordNo,dateFlag,localCalDay,specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_NOAA :
      rc=ReliNOAA(pSpecInfo,recordNo,dateFlag,localCalDay,specFp);
     break;
  // ---------------------------------------------------------------------------
     #if defined (__INCLUDE_HDF_) && __INCLUDE_HDF_
     case PRJCT_INSTR_FORMAT_OMI :
      rc=OMI_ReadHDF(pSpecInfo,recordNo);
     break;
     #endif
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_CCD_EEV :
      rc=ReliCCD_EEV(pSpecInfo,recordNo,dateFlag,localCalDay,specFp,darkFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_CCD_HA_94 :
      rc=ReliCCD(pSpecInfo,recordNo,dateFlag,localCalDay,specFp,namesFp,darkFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_CCD_ULB :
      rc=ReliCCD_Ulb(pSpecInfo,recordNo,dateFlag,localCalDay,specFp,darkFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_CCD_OHP_96 :
      rc=ReliCCDTrack(pSpecInfo,recordNo,dateFlag,localCalDay,specFp,namesFp,darkFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_OPUS :
      rc=OPUS_Read(pSpecInfo,recordNo,dateFlag,localCalDay,specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_GDP_ASCII :
      rc=GDP_ASC_Read(pSpecInfo,recordNo,dateFlag,specFp);
     break;
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_GDP_BIN :
      rc=GDP_BIN_Read(pSpecInfo,recordNo,specFp,GDP_BIN_currentFileIndex);
     break;
  // ---------------------------------------------------------------------------
     #if defined (__INCLUDE_HDF_) && __INCLUDE_HDF_
     case PRJCT_INSTR_FORMAT_SCIA_HDF :
      rc=SCIA_ReadHDF(pSpecInfo,recordNo);
     break;
     #endif
  // ---------------------------------------------------------------------------
     case PRJCT_INSTR_FORMAT_SCIA_PDS :
      rc=SCIA_ReadPDS(pSpecInfo,recordNo);
     break;
  // ---------------------------------------------------------------------------
// QDOAS ???     case PRJCT_INSTR_FORMAT_GOME2 :
// QDOAS ???      rc=GOME2_Read(pSpecInfo,recordNo);
// QDOAS ???     break;
  // ---------------------------------------------------------------------------
     default :
      rc=ERROR_ID_FILE_BAD_FORMAT;
     break;
  // ---------------------------------------------------------------------------
   }

// QDOAS ???   if (rc && (rc!=ERROR_ID_FILE_RECORD) && (rc!=ERROR_ID_PDS))
// QDOAS ???    rc=THRD_ProcessLastError();

  // File read out

  if (rc<THREAD_EVENT_STOP)
   {  // BEGIN 0
    OBSERVATION_SITE *pSite;
    double SMax,darkSum,longit,latit;
    INDEX indexBand;
    INT maxBand,maxV,maxH,indexSite,geoFlag,sitesNumber;
    UCHAR graphTitle[MAX_ITEM_NAME_LEN+1];

    memset(graphTitle,0,MAX_ITEM_NAME_LEN+1);
    maxV=maxH=maxBand=1;
    indexBand=0;

    // Zenith angle correction from observation site geolocations

    if ((indexSite=SITES_GetIndex(pSpecInfo->project.instrumental.observationSite))!=ITEM_NONE)
     {
      pSite=&SITES_itemList[indexSite];

      longit=-pSite->longitude;   // !!! sign is inverted

      pSpecInfo->longitude=-longit;
      pSpecInfo->latitude=latit=(double)pSite->latitude;

      if (pSite->altitude>(double)0.)
       pSpecInfo->altitude=pSite->altitude*0.001;

      pSpecInfo->Zm=(pSpecInfo->Tm!=(double)0.)?ZEN_FNTdiz(ZEN_FNCrtjul(&pSpecInfo->Tm),&longit,&latit,&pSpecInfo->Azimuth):(double)-1.;
     }

    if ((pSpecInfo->longitude!=(double)0.) && (pSpecInfo->latitude!=(double)0.) &&
       ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
        (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)))
     {
      UCHAR string[30];
      sprintf(string,"%02d/%02d/%d %02d:%02d:%02d",
                      pSpecInfo->present_day.da_day,pSpecInfo->present_day.da_mon,pSpecInfo->present_day.da_year,
                      pSpecInfo->present_time.ti_hour,pSpecInfo->present_time.ti_min,pSpecInfo->present_time.ti_sec);

      MOON_GetPosition(string,                      // input date and time for moon positions calculation
                      -pSpecInfo->longitude,        // longitude of the observation site
                       pSpecInfo->latitude,         // latitude of the observation site
                       pSpecInfo->altitude,         // altitude of the observation site
                       &pSpecInfo->aMoon,           // azimuth, measured westward from the south
                       &pSpecInfo->hMoon,           // altitude above the horizon
                       &pSpecInfo->fracMoon);       // illuminated fraction of the moon
     }

    // Dark current subtraction

    if (pSpecInfo->darkCurrent!=NULL)
     {
      if (pSpecInfo->project.instrumental.averageFlag)
       {
        for (i=20,darkSum=(double)0.;i<NDET-20;i++)
         darkSum+=(double)pSpecInfo->darkCurrent[i];

        for (i=0;i<NDET;i++)
         pSpecInfo->darkCurrent[i]=darkSum/(NDET-40+1);
       }

      for (i=0;i<NDET;i++)
       pSpecInfo->spectrum[i]-=pSpecInfo->darkCurrent[i];
     }

    SMax=VECTOR_Max(pSpecInfo->spectrum,NDET);
    pSpecInfo->indexRecord=recordNo;
    memcpy(spectrum,pSpecInfo->spectrum,sizeof(double)*NDET);

    if ((rc<THREAD_EVENT_STOP) &&
      (((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC) && (mfcMask!=pInstrumental->mfcMaskSpec)) ||
       !(rc=THRD_SpectrumCorrection(pSpecInfo,spectrum))))    // Apply odd/even pixel correction on temporary buffer
     {  // BEGIN 1
      longit=pSpecInfo->longitude;
      latit=pSpecInfo->latitude;
      geoFlag=1;

      // Geolocation constraints

      if ((pProject->spectra.mode==PRJCT_SPECTRA_MODES_CIRCLE) && (pProject->spectra.radius>1.) &&
          (THRD_GetDist(longit,latit,pProject->spectra.longMin,pProject->spectra.latMin)>(double)pProject->spectra.radius))
       geoFlag=0;
      else if ((pProject->spectra.mode==PRJCT_SPECTRA_MODES_OBSLIST) && (pProject->spectra.radius>1.))
       {
        for (indexSite=0,sitesNumber=TREE_itemType[TREE_ITEM_TYPE_SITE_CHILDREN].dataNumber;indexSite<sitesNumber;indexSite++)
         {
          pSite=&SITES_itemList[indexSite];

          if (!pSite->hidden)
           {
            if (THRD_GetDist(longit,latit,pSite->longitude,pSite->latitude)<=(double)pProject->spectra.radius)
             break;
           }
         }
        if (indexSite==sitesNumber)
         geoFlag=0;
       }
      else if ((pProject->spectra.mode==PRJCT_SPECTRA_MODES_RECTANGLE) &&

             (((pProject->spectra.longMin!=pProject->spectra.longMax) &&
               ((longit>max(pProject->spectra.longMin,pProject->spectra.longMax)) ||
                (longit<min(pProject->spectra.longMin,pProject->spectra.longMax)))) ||

               ((pProject->spectra.latMin!=pProject->spectra.latMax) &&
               ((latit>max(pProject->spectra.latMin,pProject->spectra.latMax)) ||
                (latit<min(pProject->spectra.latMin,pProject->spectra.latMax))))))

       geoFlag=0;

      if ((!geoFlag ||  // Geolocation constraints

          // Zenith angle constraints

         (((pProject->instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_ASCII) || pProject->instrumental.ascii.szaSaveFlag) &&
          ((pProject->instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_MFC) || (mfcMask==pProject->instrumental.mfcMaskSpec)) &&
         (((pProject->spectra.SZAMin>(float)0.) && ((float)pSpecInfo->Zm<pProject->spectra.SZAMin)) ||
          ((pProject->spectra.SZAMax>(float)0.) && ((float)pSpecInfo->Zm>pProject->spectra.SZAMax)) ||
          ((pProject->spectra.SZADelta>(float)0.) && (fabs(pSpecInfo->Zm-THRD_specInfo.Zm)<pProject->spectra.SZADelta))))))

      /*    ||

          // Record constraints

          ((THRD_goto.indexType==THREAD_GOTO_RECORD) &&
           (THRD_goto.indexRecord>0) &&
           (pSpecInfo->indexRecord!=THRD_goto.indexRecord))*/
       {
       rc=ERROR_ID_FILE_RECORD;
      }

      else if (!specInfoFlag
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???           || !(rc=ThrdWriteSpecInfo(pSpecInfo))
// QDOAS ???   #endif
              )
       {  // BEGIN 2
        // Pause before next record

        if (THRD_resetFlag)
         THRD_increment=THRD_resetFlag=0;

        THRD_goto.indexRecord=ITEM_NONE;

        // Pause before next record

        if ((THRD_browseType==THREAD_BROWSE_EXPORT) && (THRD_asciiFp!=NULL))
         {
          if (ASCII_options.szaSaveFlag)
           fprintf(THRD_asciiFp,"%lf ",pSpecInfo->Zm);

          if (ASCII_options.azimSaveFlag)
           fprintf(THRD_asciiFp,"%f ",pSpecInfo->azimuthViewAngle);

          if (ASCII_options.elevSaveFlag)
           fprintf(THRD_asciiFp,"%lf ",pSpecInfo->elevationViewAngle);

          if (ASCII_options.format==PRJCT_INSTR_ASCII_FORMAT_COLUMN &&
             (ASCII_options.szaSaveFlag || ASCII_options.azimSaveFlag || ASCII_options.elevSaveFlag))

           fprintf(THRD_asciiFp,"\n");

          if (ASCII_options.dateSaveFlag)
           {
            fprintf(THRD_asciiFp,"%02d/%02d/%d ",pSpecInfo->present_day.da_day,pSpecInfo->present_day.da_mon,pSpecInfo->present_day.da_year);
            if (ASCII_options.format==PRJCT_INSTR_ASCII_FORMAT_COLUMN)
             fprintf(THRD_asciiFp,"\n");
           }

          if (ASCII_options.timeSaveFlag)
           {
            fprintf(THRD_asciiFp,"%lf ",pSpecInfo->TimeDec);
            if (ASCII_options.format==PRJCT_INSTR_ASCII_FORMAT_COLUMN)
             fprintf(THRD_asciiFp,"\n");
           }

          for (i=0;i<NDET;i++)
           {
            if ((ASCII_options.format==PRJCT_INSTR_ASCII_FORMAT_COLUMN) && ASCII_options.lembdaSaveFlag)
             fprintf(THRD_asciiFp,"%lf ",pSpecInfo->lembda[i]);

            fprintf(THRD_asciiFp,"%lf ",pSpecInfo->spectrum[i]);

            if (ASCII_options.format==PRJCT_INSTR_ASCII_FORMAT_COLUMN)
             fprintf(THRD_asciiFp,"\n");
           }

          if (ASCII_options.format==PRJCT_INSTR_ASCII_FORMAT_LINE)
           fprintf(THRD_asciiFp,"\n");
         }

        else if ((THRD_id==THREAD_TYPE_SPECTRA) || (pProject->spectra.displaySpectraFlag==(UCHAR)1))
         {
          if ((pSpecInfo->instrFunction!=NULL) && ((THRD_browseType==THREAD_BROWSE_SPECTRA) || (THRD_browseType==THREAD_BROWSE_EXPORT)))
           {
            for (i=0;i<NDET;i++)
             if (fabs(pSpecInfo->instrFunction[i])>(double)1.e-5)
              spectrum[i]/=pSpecInfo->instrFunction[i];
             else
              spectrum[i]=(double)0.;
           }

// QDOAS ???           #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ???           // Graph display
// QDOAS ???
// QDOAS ???           sprintf(windowTitle,"Spectra - %s (# %d/%d SZA %g)",
// QDOAS ???                   pSpecInfo->fileName,recordNo,pSpecInfo->recordNumber,pSpecInfo->Zm);
// QDOAS ???
// QDOAS ???
// QDOAS ???           if ((THRD_browseType==THREAD_BROWSE_DARK) && (pSpecInfo->darkCurrent!=NULL) && (VECTOR_Max(pSpecInfo->darkCurrent,NDET)>(double)0.))
// QDOAS ???
// QDOAS ???            DRAW_Spectra(CHILD_WINDOW_SPECTRA,windowTitle,graphTitle,"Wavelength (nm)","Intensity",NULL,0,
// QDOAS ???                        (double)0.,(double)0.,(double)0.,(double)0.,
// QDOAS ???                         pSpecInfo->lembda,pSpecInfo->darkCurrent,NDET,DRAW_COLOR1,0,NDET-1,PS_SOLID,NULL,
// QDOAS ??? //                        pSpecInfo->lembda,(THRD_dark && (pSpecInfo->darkCurrent!=NULL) && (VECTOR_Max(pSpecInfo->darkCurrent,NDET)>(double)0.))?pSpecInfo->darkCurrent:spectrum,NDET,DRAW_COLOR2,0,NDET-1,PS_DOT,
// QDOAS ???                         NULL,NULL,0,0,0,0,PS_SOLID,NULL,
// QDOAS ???                         indexBand,maxV,maxH,(indexBand==maxBand-1)?1:0);
// QDOAS ???
// QDOAS ???           else if ((THRD_browseType==THREAD_BROWSE_ERROR) && (pSpecInfo->sigmaSpec!=NULL) && (VECTOR_Max(pSpecInfo->sigmaSpec,NDET)>(double)0.))
// QDOAS ???
// QDOAS ???            DRAW_Spectra(CHILD_WINDOW_SPECTRA,windowTitle,graphTitle,"Wavelength (nm)","Intensity",NULL,0,
// QDOAS ???                        (double)0.,(double)0.,(double)0.,(double)0.,
// QDOAS ???                         pSpecInfo->lembda,pSpecInfo->sigmaSpec,NDET,DRAW_COLOR1,0,NDET-1,PS_SOLID,NULL,
// QDOAS ??? //                        pSpecInfo->lembda,(THRD_dark && (pSpecInfo->darkCurrent!=NULL) && (VECTOR_Max(pSpecInfo->darkCurrent,NDET)>(double)0.))?pSpecInfo->darkCurrent:spectrum,NDET,DRAW_COLOR2,0,NDET-1,PS_DOT,
// QDOAS ???                         NULL,NULL,0,0,0,0,PS_SOLID,NULL,
// QDOAS ???                         indexBand,maxV,maxH,(indexBand==maxBand-1)?1:0);
// QDOAS ???
// QDOAS ???           else
// QDOAS ???            {
// QDOAS ???             DRAW_Spectra(CHILD_WINDOW_SPECTRA,windowTitle,graphTitle,"Wavelength (nm)","Intensity",NULL,0,
// QDOAS ???                         (double)0.,(double)0.,(double)0.,(double)0.,
// QDOAS ???                          pSpecInfo->lembda,spectrum,NDET,DRAW_COLOR1,0,NDET-1,PS_SOLID,NULL,
// QDOAS ??? //                         pSpecInfo->lembda,(THRD_dark && (pSpecInfo->darkCurrent!=NULL) && (VECTOR_Max(pSpecInfo->darkCurrent,NDET)>(double)0.))?pSpecInfo->darkCurrent:spectrum,NDET,DRAW_COLOR2,0,NDET-1,PS_DOT,
// QDOAS ???                          NULL,NULL,0,0,0,0,PS_SOLID,NULL,
// QDOAS ???                          indexBand,maxV,maxH,(indexBand==maxBand-1)?1:0);
// QDOAS ???
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           DRAW_SpecMax(pSpecInfo->specMax,pSpecInfo->rejected+pSpecInfo->NSomme,DRAW_COLOR2);
// QDOAS ???           #endif
         }
       }    // END 2
     }    // END 1

// QDOAS ???    if ((rc<THREAD_EVENT_STOP) && ((THRD_id==THREAD_TYPE_SPECTRA) || (pProject->spectra.displaySpectraFlag==(UCHAR)1)))
// QDOAS ???     {
// QDOAS ???      delay=(THRD_increment)?THRD_delay:INFINITE;  // delay is conditioned by display options
// QDOAS ???
// QDOAS ???      if (((rc=THRD_WaitEvent(delay,1,(THRD_id==THREAD_TYPE_SPECTRA)?1:0))==THREAD_EVENT_PAUSE) &&
// QDOAS ???          ((THRD_id==THREAD_TYPE_ANALYSIS) || (THRD_id==THREAD_TYPE_KURUCZ)))
// QDOAS ???       rc=THRD_WaitEvent(INFINITE,1,0);
// QDOAS ???     }
// QDOAS ???
// QDOAS ???    if (rc<THREAD_EVENT_STOP)
// QDOAS ???     {
// QDOAS ???      if (pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_MFC)
// QDOAS ???       memcpy(pSpecInfo->spectrum,spectrum,sizeof(double)*NDET);
// QDOAS ???      else
// QDOAS ???       rc=THRD_SpectrumCorrection(pSpecInfo,pSpecInfo->spectrum);
// QDOAS ???     }
// QDOAS ???    else if (!dateFlag && (rc>THREAD_EVENT_STOP))
// QDOAS ???     {
// QDOAS ???      if ((recordNo==THRD_recordFirst) && (THRD_recordFirst<THRD_recordLast))
// QDOAS ???       THRD_recordFirst++;
// QDOAS ???      else if ((recordNo==THRD_recordLast) && (THRD_recordLast>THRD_recordFirst))
// QDOAS ???       THRD_recordLast--;
// QDOAS ???     }
   }   // END 0
  else if (rc==ERROR_ID_FILE_BAD_FORMAT)
   rc=ERROR_SetLast("ThrdReadFile",ERROR_TYPE_WARNING,ERROR_ID_FILE_BAD_FORMAT,pSpecInfo->fileName);

  // Return

  THRD_correction=0;
  NDET=oldNDET;

  if (spectrum!=NULL)
   MEMORY_ReleaseDVector("ThrdReadFile ","spectrum",spectrum,0);

  return rc;
 }

// ======================================
// AUTOMATIC REFERENCE SPECTRUM SELECTION
// ======================================

// -----------------------------------------------------------------------------
// ThrdSetRefIndexes : Set indexes of spectra selected as reference for analysis
// -----------------------------------------------------------------------------

RC ThrdSetRefIndexes(SPEC_INFO *pSpecInfo,FILE *specFp,FILE *namesFp,FILE *darkFp)
 {
  // Declarations

  INDEX         *indexList,              // indexes of records
                 indexTabFeno,           // browse analysis windows list
                 indexRecord,            // browse spectra records in file
                 indexZmMin;             // index of record with SZA minimum

  PRJCT_SPECTRA *pSpectra;               // pointer to project part of data
  FENO          *pTabFeno;               // pointer to analysis windows
  double        *ZmList,*TimeDec,        // zenith angles and decimal time of records
                 ZmMin,ZmMax,            // extrema zenith angles found in file
                 deltaZmMorning,         // select record in the morning with SZA the closest by SZA base
                 deltaZmAfternoon;       // select record in the afternoon with SZA the closest by SZA base
  INT            NRecord;                // number of hold record
  INT            localCalDay;
  RC             rc;                     // return code

  // Initializations

  indexList=NULL;
  indexZmMin=ITEM_NONE;
  ZmList=TimeDec=NULL;
  NRecord=0;
  ZmMin=360.;
  ZmMax=0.;

  rc=THRD_CopySpecInfo(&THRD_refInfo,pSpecInfo);       // make a copy of general data
  pSpectra=&THRD_refInfo.project.spectra;             // pointer to project part of data
  localCalDay=THRD_refInfo.localCalDay;

  if ((THRD_refInfo.project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_LOGGER) ||
      (THRD_refInfo.project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_PDAEGG))

   THRD_refInfo.project.instrumental.user=PRJCT_INSTR_IASB_TYPE_ZENITHAL;

  if (THRD_refInfo.recordNumber>0)
   {
    // Memory allocation

    if (((indexList=(INDEX *)MEMORY_AllocBuffer("ThrdSetRefIndexes ","",THRD_refInfo.recordNumber,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL) ||
        ((ZmList=(double *)MEMORY_AllocDVector("ThrdSetRefIndexes ","",0,THRD_refInfo.recordNumber-1))==NULL) ||
        ((TimeDec=(double *)MEMORY_AllocDVector("ThrdSetRefIndexes ","",0,THRD_refInfo.recordNumber-1))==NULL))

     rc=ERROR_ID_ALLOC;

    else
     {
      // Keep all record constraints except SZA and display ones

      pSpectra->SZAMin=(double)0.;
      pSpectra->SZAMax=(double)360.;
      pSpectra->SZADelta=(double)0.;
      pSpectra->displaySpectraFlag=0;

      // Browse records in file

      for (indexRecord=THRD_lastRefRecord+1;indexRecord<=THRD_refInfo.recordNumber;indexRecord++)
       if (((rc=ThrdReadFile(&THRD_refInfo,indexRecord,1,localCalDay,specFp,namesFp,darkFp,0))<THREAD_EVENT_STOP) &&
           (THRD_refInfo.Zm>(double)0.))
        {
         if (rc==ITEM_NONE)
          rc=0;

         // Data on record

         indexList[NRecord]=indexRecord;             // index of record
         ZmList[NRecord]=THRD_refInfo.Zm;                 // zenith angle
         TimeDec[NRecord]=THRD_refInfo.localTimeDec;      // decimal time for determining when the measurement has occured

         // Minimum and maximum zenith angle

         if (THRD_refInfo.Zm<ZmMin)
          {
           ZmMin=THRD_refInfo.Zm;
           indexZmMin=NRecord;
          }

         if (THRD_refInfo.Zm>ZmMax)
          ZmMax=THRD_refInfo.Zm;

         NRecord++;
        }
       else if (rc==ERROR_ID_FILE_END)
        {
         rc=ERROR_ID_NO;
         break;
        }

      if (rc==ERROR_ID_FILE_RECORD)
       rc=ERROR_ID_NO;

      // Browse analysis windows

      for (indexTabFeno=0;(indexTabFeno<NFeno) && (rc<THREAD_EVENT_STOP);indexTabFeno++)
       {
        pTabFeno=&TabFeno[indexTabFeno];

        if (!pTabFeno->hidden && (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC))
         {
          // Indexes reinitialization

          pTabFeno->indexRefMorning=pTabFeno->indexRefAfternoon=pTabFeno->indexRef=ITEM_NONE;

          // No reference spectrum found in SZA range

          if (ZmMax<pTabFeno->refSZA-pTabFeno->refSZADelta)
           rc=ERROR_SetLast("ThrdSetRefIndexes",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"all the day",THRD_refInfo.fileName);

          // Select record with SZA minimum

          else if (ZmMin>=pTabFeno->refSZA+pTabFeno->refSZADelta)
           pTabFeno->indexRefMorning=pTabFeno->indexRefAfternoon=indexZmMin;

          // Select a record for the morning and the afternoon

          else
           {
            deltaZmMorning=deltaZmAfternoon=360.;

            for (indexRecord=0;indexRecord<NRecord;indexRecord++)
             {
              if ((ZmList[indexRecord]>=pTabFeno->refSZA-pTabFeno->refSZADelta) &&
                  (ZmList[indexRecord]<=pTabFeno->refSZA+pTabFeno->refSZADelta))
               {
                if ((TimeDec[indexRecord]<=THRD_localNoon) && (fabs(pTabFeno->refSZA-ZmList[indexRecord])<deltaZmMorning))
                 {
                  pTabFeno->indexRefMorning=indexRecord;
                  deltaZmMorning=fabs(pTabFeno->refSZA-ZmList[indexRecord]);
                 }

                if ((TimeDec[indexRecord]>THRD_localNoon) && (fabs(pTabFeno->refSZA-ZmList[indexRecord])<deltaZmAfternoon))
                 {
                  pTabFeno->indexRefAfternoon=indexRecord;
                  deltaZmAfternoon=fabs(pTabFeno->refSZA-ZmList[indexRecord]);
                 }
               }
             }

            // No record found for the morning OR the afternoon

            if ((pTabFeno->indexRefMorning==ITEM_NONE) && (pTabFeno->indexRefAfternoon==ITEM_NONE))
             rc=ERROR_SetLast("ThrdSetRefIndexes",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"all the day",THRD_refInfo.fileName);
            else if (pSpecInfo->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_ASCII)
             {
              if (pTabFeno->indexRefMorning==ITEM_NONE)
               {
               	rc=ERROR_SetLast("ThrdSetRefIndexes",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"the morning",THRD_refInfo.fileName);
                pTabFeno->indexRefMorning=pTabFeno->indexRefAfternoon;
               }
              else if (pTabFeno->indexRefAfternoon==ITEM_NONE)
               {
               	rc=ERROR_SetLast("ThrdSetRefIndexes",ERROR_TYPE_WARNING,ERROR_ID_NO_REF,"the afternoon",THRD_refInfo.fileName);
                pTabFeno->indexRefAfternoon=pTabFeno->indexRefMorning;
               }
             }
           }

          pTabFeno->oldZmRefMorning=pTabFeno->ZmRefMorning;
          pTabFeno->oldZmRefAfternoon=pTabFeno->ZmRefAfternoon;
          pTabFeno->ZmRefMorning=(pTabFeno->indexRefMorning!=ITEM_NONE)?ZmList[pTabFeno->indexRefMorning]:(double)-1.;
          pTabFeno->ZmRefAfternoon=(pTabFeno->indexRefAfternoon!=ITEM_NONE)?ZmList[pTabFeno->indexRefAfternoon]:(double)-1.;
          pTabFeno->indexRefMorning=(pTabFeno->indexRefMorning!=ITEM_NONE)?indexList[pTabFeno->indexRefMorning]:ITEM_NONE;
          pTabFeno->indexRefAfternoon=(pTabFeno->indexRefAfternoon!=ITEM_NONE)?indexList[pTabFeno->indexRefAfternoon]:ITEM_NONE;
         }
       }
     }
   }

  // Release buffers

  if (indexList!=NULL)
   MEMORY_ReleaseBuffer("ThrdSetRefIndexes ","indexList",indexList);
  if (ZmList!=NULL)
   MEMORY_ReleaseDVector("ThrdSetRefIndexes ","ZmList",ZmList,0);
  if (TimeDec!=NULL)
   MEMORY_ReleaseDVector("ThrdSetRefIndexes ","TimeDec",TimeDec,0);

  // Return

  return rc;
 }

// ---------------------------------------
// ThrdNewRef : Load new reference spectra
// ---------------------------------------

RC ThrdNewRef(SPEC_INFO *pSpecInfo,FILE *specFp,FILE *namesFp,FILE *darkFp)
 {
  // Declarations

  double *Sref,*lembdaRef;

  INDEX indexRefRecord,        // index of best record in file for reference selection
        indexTabFeno,          // browse analysis windows list
        indexWindow;

  FENO *pTabFeno;
  INT useKurucz,alignRef,useUsamp,saveFlag,newDimL;
  double factTemp;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;
  saveFlag=(INT)pSpecInfo->project.spectra.displayDataFlag;
  useKurucz=alignRef=useUsamp=0;
  Sref=lembdaRef=NULL;

  // Select spectra records as reference

  if ((pSpecInfo->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_ASCII) && !pSpecInfo->project.instrumental.ascii.szaSaveFlag)
   rc=ERROR_SetLast("ASCII_Read",ERROR_TYPE_WARNING,ERROR_ID_FILE_AUTOMATIC);

  else if (pSpecInfo->localCalDay!=THRD_refInfo.localCalDay)
//  else if (memcmp((char *)&pSpecInfo->present_day,(char *)&THRD_refInfo.present_day,sizeof(SHORT_DATE))!=0)  // ref and spectrum are not of the same day
   rc=ThrdSetRefIndexes(pSpecInfo,specFp,namesFp,darkFp);

  for (indexTabFeno=0;(indexTabFeno<NFeno) && (rc<THREAD_EVENT_STOP);indexTabFeno++)
   {
    pTabFeno=&TabFeno[indexTabFeno];

    if ((pTabFeno->hidden!=1) &&                                                   // not the definition of the window for the wavelength calibration
        (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC))  // automatic reference selection only
     {
      pTabFeno->displayRef=0;

      if ((indexRefRecord=(pSpecInfo->localTimeDec<=THRD_localNoon)?pTabFeno->indexRefMorning:pTabFeno->indexRefAfternoon)==ITEM_NONE)
       {
        memcpy((char *)&THRD_specInfo.present_day,(char *)&pSpecInfo->present_day,sizeof(SHORT_DATE));
        rc=ERROR_ID_FILE_RECORD;
       }

      // Read out reference from file

      else if ((indexRefRecord!=pTabFeno->indexRef) &&
              ((rc=ThrdReadFile(&THRD_refInfo,indexRefRecord,0,0,specFp,namesFp,darkFp,0))<THREAD_EVENT_STOP))
       {
        alignRef++;

        if (!pTabFeno->useEtalon)
         {
          memcpy(pTabFeno->LembdaK,THRD_refInfo.lembda,sizeof(double)*NDET);
          memcpy(pTabFeno->LembdaRef,THRD_refInfo.lembda,sizeof(double)*NDET);

          for (indexWindow=0,newDimL=0;indexWindow<pTabFeno->svd.Z;indexWindow++)
           {
            pTabFeno->svd.Fenetre[indexWindow][0]=FNPixel(THRD_refInfo.lembda,pTabFeno->svd.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
            pTabFeno->svd.Fenetre[indexWindow][1]=FNPixel(THRD_refInfo.lembda,pTabFeno->svd.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

            newDimL+=(pTabFeno->svd.Fenetre[indexWindow][1]-pTabFeno->svd.Fenetre[indexWindow][0]+1);
           }

          if (newDimL!=pTabFeno->svd.DimL)
           {
            ANALYSE_SvdFree("THRD_NewRef",&pTabFeno->svd);
            pTabFeno->svd.DimL=newDimL;
            ANALYSE_SvdLocalAlloc("THRD_NewRef",&pTabFeno->svd);
           }

          if (pTabFeno->useKurucz)
           {
            KURUCZ_Init(0);
            useKurucz++;
           }
          else if (((rc=ANALYSE_XsInterpolation(pTabFeno,pTabFeno->LembdaRef))!=ERROR_ID_NO) ||
                   ((rc=ANALYSE_XsConvolution(pTabFeno,pTabFeno->LembdaRef,&ANALYSIS_slit,pSlitOptions->slitFunction.slitType,&pSlitOptions->slitFunction.slitParam,&pSlitOptions->slitFunction.slitParam2,&pSlitOptions->slitFunction.slitParam3,&pSlitOptions->slitFunction.slitParam4))!=ERROR_ID_NO))
           break;
         }

        if (pTabFeno->useUsamp)
         useUsamp++;

        memcpy(pTabFeno->Sref,THRD_refInfo.spectrum,sizeof(double)*NDET);

        if ((rc=ANALYSE_NormalizeVector(pTabFeno->Sref-1,NDET,&factTemp,"ThrdNewRef "))!=ERROR_ID_NO)
         break;

        pTabFeno->indexRef=indexRefRecord;
        pTabFeno->Zm=THRD_refInfo.Zm;
        pTabFeno->Tm=THRD_refInfo.Tm;
        pTabFeno->TimeDec=THRD_refInfo.TimeDec;
        pTabFeno->displayRef=1;

        memcpy(&pTabFeno->refDate,&THRD_refInfo.present_day,sizeof(SHORT_DATE));

//        if (pTabFeno->useEtalon)
//         rc=ANALYSE_AlignRef(pTabFeno,pTabFeno->Lembda,pTabFeno->SrefEtalon,pTabFeno->Sref,&pTabFeno->Shift,&pTabFeno->Stretch,&pTabFeno->Stretch2,saveFlag);
       }
      else if (indexRefRecord==pTabFeno->indexRef)
       pTabFeno->displayRef=1;
     }
   }

// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   ThrdWriteSpecInfo(pSpecInfo);
// QDOAS ???   #endif

  // Reference alignment

  if ((rc<THREAD_EVENT_STOP) && useKurucz)
   rc=KURUCZ_Reference(THRD_refInfo.instrFunction,1,saveFlag,1);

  if ((rc<THREAD_EVENT_STOP) && alignRef)
    rc=ANALYSE_AlignReference(1,saveFlag);

  if ((rc<THREAD_EVENT_STOP) && useUsamp)
   rc=USAMP_BuildFromAnalysis(1,ITEM_NONE);

  // Return

  if (Sref!=NULL)
   MEMORY_ReleaseDVector("ThrdNewRef ","Sref",Sref,0);
  if (lembdaRef!=NULL)
   MEMORY_ReleaseDVector("ThrdNewRef ","lembdaRef",lembdaRef,0);

  return rc;
 }

// ===================
// COMMANDS PROCESSING
// ===================

// QDOAS ??? // ---------------------------------------------------------------
// QDOAS ??? // THRD_BrowseSpectra : Thread used for browsing spectra in a file
// QDOAS ??? // ---------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? void THRD_BrowseSpectra(void *threadType)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   FILE *specFp,*namesFp,*darkFp;                                         // file pointers
// QDOAS ???   RAW_FILES *pRaw;                                                       // list of raw spectra files
// QDOAS ???   INDEX indexFeno;                                                       // browse analysis windows
// QDOAS ???   UCHAR fileName[MAX_ITEM_TEXT_LEN+1],fileTmp[MAX_ITEM_TEXT_LEN+1],      // make a copy of current file name
// QDOAS ???         waitEventFlag,
// QDOAS ???         *ptr,asciiFile[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   SPEC_INFO specInfo;                                                    // data on current spectra and reference
// QDOAS ???   INT endAnalysis;
// QDOAS ???   HWND hwndEdit;
// QDOAS ???   RC rc;                                                                 // return code
// QDOAS ???
// QDOAS ???   #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_FILE_) && __DEBUG_DOAS_FILE_
// QDOAS ???   DEBUG_Start(DOAS_dbgFile,"Thread",DEBUG_FCTTYPE_FILE,5,1,1);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   TLBAR_EnableThreadButtons(TRUE);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   THRD_asciiFp=specFp=namesFp=darkFp=NULL;                    // file pointers
// QDOAS ???
// QDOAS ???   memset(&specInfo,0,sizeof(SPEC_INFO));         // data on current spectra
// QDOAS ???   memset(&THRD_refInfo,0,sizeof(SPEC_INFO));     // data on reference spectrum (automatic selection)
// QDOAS ???
// QDOAS ???   THRD_specInfo.indexProject=                    // index of current project
// QDOAS ???   THRD_specInfo.indexFile=                       // index of current file
// QDOAS ???   THRD_specInfo.indexRecord=ITEM_NONE;           // index of current record in file
// QDOAS ???
// QDOAS ???   THRD_id=(INT)threadType&0x000000FF;            // thread identification
// QDOAS ???   THRD_browseType=((INT)threadType>>8);
// QDOAS ???   THRD_increment=1;                              // record increment
// QDOAS ???   THRD_resetFlag=0;                              // don't pause automatically
// QDOAS ???   THRD_setOriginalFlag=0;                        // original settings recover flag
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   THRD_delay=INFINITE;                           // pause on display
// QDOAS ???   #endif
// QDOAS ???   THRD_lastEvent=ITEM_NONE;
// QDOAS ???   THRD_correction=0;
// QDOAS ???   THRD_endProgram=0;
// QDOAS ???   thrdRefFlag=1;
// QDOAS ???
// QDOAS ???   if (THRD_browseType==THREAD_BROWSE_EXPORT)
// QDOAS ???    {
// QDOAS ???     strcpy(asciiFile,THRD_asciiFile);
// QDOAS ???
// QDOAS ???     THRD_asciiPtr=strrchr(asciiFile,PATH_SEP);
// QDOAS ???     THRD_asciiFp=(STD_Stricmp(THRD_asciiPtr+1,"automatic"))?fopen(asciiFile,"w+t"):NULL;
// QDOAS ???    }
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     THRD_asciiPtr=NULL;
// QDOAS ???     THRD_asciiFp=NULL;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   if ((THRD_id==THREAD_TYPE_ANALYSIS) || (THRD_id==THREAD_TYPE_KURUCZ))
// QDOAS ???    THRD_Error(ERROR_TYPE_TIME,ERROR_ID_START_ANALYSIS,"",rc);
// QDOAS ???
// QDOAS ???   endAnalysis=0;
// QDOAS ???
// QDOAS ???   // Loop on items in tree
// QDOAS ???
// QDOAS ???   do
// QDOAS ???    {
// QDOAS ???     waitEventFlag=1;
// QDOAS ???
// QDOAS ???     #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???     if (rc==THREAD_EVENT_PAUSE)
// QDOAS ???      THRD_delay=INFINITE;
// QDOAS ???
// QDOAS ???     // Recover original settings
// QDOAS ???
// QDOAS ???     else
// QDOAS ???     #endif
// QDOAS ???          if (THRD_setOriginalFlag)
// QDOAS ???      {
// QDOAS ???       THRD_indexProjectCurrent=THRD_specInfo.indexProject;
// QDOAS ???       THRD_indexFileCurrent=THRD_specInfo.indexFile;
// QDOAS ???       THRD_indexRecordCurrent=THRD_specInfo.indexRecord;
// QDOAS ???
// QDOAS ???     //  THRD_goto.indexRecord=THRD_goto.indexPixel=THRD_goto.indexMin=THRD_goto.indexMax=ITEM_NONE;
// QDOAS ???
// QDOAS ???       if ((specInfo.project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) && (THRD_goto.indexPixel<1))
// QDOAS ??? //           (specInfo.project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI)) && (THRD_goto.indexPixel<1))
// QDOAS ???        THRD_goto.indexPixel=1;
// QDOAS ???
// QDOAS ???       THRD_setOriginalFlag=0;
// QDOAS ???       THRD_increment=1;
// QDOAS ???
// QDOAS ???       #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???       if (THRD_browseType==THREAD_BROWSE_EXPORT)
// QDOAS ???        SetEvent(THRD_hEvents[THREAD_EVENT_STOP]);
// QDOAS ???       #endif
// QDOAS ???
// QDOAS ???       if ((THRD_id==THREAD_TYPE_ANALYSIS) || (THRD_id==THREAD_TYPE_KURUCZ))
// QDOAS ???        THRD_Error(ERROR_TYPE_TIME,ERROR_ID_END_ANALYSIS,"",rc);
// QDOAS ???       endAnalysis=1;
// QDOAS ???
// QDOAS ???       #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???       if (DOAS_arg.analysisFlag)
// QDOAS ???        SetEvent(THRD_hEvents[THREAD_EVENT_STOP]);
// QDOAS ???       else if ((THRD_id!=THREAD_TYPE_SPECTRA) && !specInfo.project.spectra.displayPause)
// QDOAS ???        {
// QDOAS ???         MSG_MessageBox(DOAS_hwndMain,ITEM_NONE,IDS_APP_TITLE,IDS_MSGBOX_ANALYSE_END,MB_ICONASTERISK|MB_OK);
// QDOAS ???         SetEvent(THRD_hEvents[THREAD_EVENT_STOP]);
// QDOAS ???     //    rc=THREAD_EVENT_STOP;
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       THRD_delay=INFINITE;
// QDOAS ???       #else
// QDOAS ???       if (DOAS_arg.analysisFlag)
// QDOAS ???       {
// QDOAS ???        rc=THREAD_EVENT_STOP;
// QDOAS ???        THRD_increment=THRD_resetFlag=0;
// QDOAS ???       }
// QDOAS ???       #endif
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???       // Load new project
// QDOAS ???
// QDOAS ???       if (THRD_indexProjectCurrent!=THRD_indexProjectOld)
// QDOAS ???        {
// QDOAS ???         THRD_indexProjectOld=THRD_indexProjectCurrent;
// QDOAS ???
// QDOAS ???         if (!(rc=ThrdLoadProject(&specInfo,PRJCT_itemList,THRD_indexProjectCurrent)))
// QDOAS ???          NDET=specInfo.project.instrumental.detectorSize;
// QDOAS ???         else // if (rc==THREAD_EVENT_STOP)
// QDOAS ???          break;
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Load new file
// QDOAS ???
// QDOAS ???       if ((THRD_indexFileCurrent!=THRD_indexFileOld) || (THRD_indexPathFileOld!=THRD_indexPathFileCurrent))
// QDOAS ???        {
// QDOAS ???         // Close previously open files
// QDOAS ???
// QDOAS ???         if (specFp!=NULL)
// QDOAS ???          fclose(specFp);
// QDOAS ???         if (darkFp!=NULL)
// QDOAS ???          fclose(darkFp);
// QDOAS ???         if (namesFp!=NULL)
// QDOAS ???          fclose(namesFp);
// QDOAS ???
// QDOAS ???         if ((THRD_id==THREAD_TYPE_SPECTRA) || !(rc=OUTPUT_FlushBuffers(&specInfo)))
// QDOAS ???          {
// QDOAS ???           pRaw=&RAW_spectraFiles[THRD_dataIndexes[THRD_indexFileCurrent]];
// QDOAS ???           THRD_isFolder=pRaw->folderFlag;
// QDOAS ???           FILES_RebuildFileName(fileName,pRaw->fileName,1);
// QDOAS ???           darkFp=namesFp=NULL;
// QDOAS ???           thrdRefFlag=1;
// QDOAS ???
// QDOAS ???           if (((pRaw->folderFlag==RAW_FOLDER_TYPE_PATH) || (pRaw->folderFlag==RAW_FOLDER_TYPE_PATHSUB)) &&
// QDOAS ???               ((THRD_indexPathFileCurrent!=ITEM_NONE) || (THRD_indexPathFileOld!=THRD_indexPathFileCurrent)))
// QDOAS ???            {
// QDOAS ???             UCHAR *ptr;
// QDOAS ???             if ((ptr=strrchr(fileName,PATH_SEP))!=NULL)
// QDOAS ???              strcpy(ptr+1,PATH_fileNamesShort[THRD_indexPathFileCurrent]);
// QDOAS ???            }
// QDOAS ???
// QDOAS ???
// QDOAS ???
// QDOAS ???           if (THRD_pathOK)
// QDOAS ???            {
// QDOAS ???             FILES_RebuildFileName(specInfo.fileName,fileName,1);
// QDOAS ???
// QDOAS ???             if ((specInfo.project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_HDF) &&
// QDOAS ???                 (specInfo.project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) &&
// QDOAS ???                 (specInfo.project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS) &&
// QDOAS ???                 (specInfo.project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GOME2))
// QDOAS ???              specFp=fopen(fileName,"rb");
// QDOAS ???
// QDOAS ???             if (THRD_browseType==THREAD_BROWSE_EXPORT)
// QDOAS ???              {
// QDOAS ???               strcpy(asciiFile,THRD_asciiFile);
// QDOAS ???               THRD_asciiPtr=strrchr(asciiFile,PATH_SEP);
// QDOAS ???
// QDOAS ???               if (!STD_Stricmp(THRD_asciiPtr+1,"automatic") && (THRD_asciiFp!=NULL))
// QDOAS ???                {
// QDOAS ???                 fclose(THRD_asciiFp);
// QDOAS ???                 THRD_asciiFp=NULL;
// QDOAS ???                }
// QDOAS ???
// QDOAS ???               if (!STD_Stricmp(THRD_asciiPtr+1,"automatic"))
// QDOAS ???                {
// QDOAS ???                 if ((ptr=strrchr(fileName,PATH_SEP))!=NULL)
// QDOAS ???                  {
// QDOAS ???                   if (THRD_asciiPtr==NULL)
// QDOAS ???                    strcpy(asciiFile,ptr+1);
// QDOAS ???                   else
// QDOAS ???                    strcpy(THRD_asciiPtr+1,ptr+1);
// QDOAS ???                  }
// QDOAS ???                 else
// QDOAS ???                  {
// QDOAS ???                   if (THRD_asciiPtr==NULL)
// QDOAS ???                    strcpy(asciiFile,specInfo.fileName);
// QDOAS ???                   else
// QDOAS ???                    strcpy(THRD_asciiPtr+1,specInfo.fileName);
// QDOAS ???                  }
// QDOAS ???
// QDOAS ???                 if ((ptr=strrchr(asciiFile,'.'))!=NULL)
// QDOAS ???                  {
// QDOAS ???                   if ((strlen(ptr+1)!=3) || STD_Stricmp(ptr+1,"asc"))
// QDOAS ???                    strcpy(ptr+1,"asc");
// QDOAS ???                   else
// QDOAS ???                    strcpy(ptr+1,"asc_ex");
// QDOAS ???                  }
// QDOAS ???                 else
// QDOAS ???                  strcat(asciiFile,".asc");
// QDOAS ???
// QDOAS ???                 THRD_asciiFp=fopen(asciiFile,"w+t");
// QDOAS ???                }
// QDOAS ???              }
// QDOAS ???
// QDOAS ???             if (specInfo.project.spectra.namesFlag!=0)
// QDOAS ???              namesFp=fopen((pRaw->notAutomatic)?FILES_RebuildFileName(fileTmp,pRaw->names,1):FILES_BuildFileName(fileName,FILE_TYPE_NAMES),"rb");
// QDOAS ???             if (specInfo.project.spectra.darkFlag!=0)
// QDOAS ???              darkFp=fopen((pRaw->notAutomatic)?FILES_RebuildFileName(fileTmp,pRaw->dark,1):FILES_BuildFileName(fileName,FILE_TYPE_DARK),"rb");
// QDOAS ???
// QDOAS ???             THRD_indexFileOld=THRD_indexFileCurrent;
// QDOAS ???             THRD_indexPathFileOld=THRD_indexPathFileCurrent;
// QDOAS ???
// QDOAS ???             // Open new files and get records indexes
// QDOAS ???
// QDOAS ???             if ((rc=ThrdFileSetPointers(&specInfo,fileName,specFp,namesFp,darkFp))!=ERROR_ID_NO)
// QDOAS ??? //             THRD_indexRecordCurrent=THRD_recordFirst;
// QDOAS ???              THRD_indexRecordCurrent=ITEM_NONE;
// QDOAS ???             else if (THRD_id!=THREAD_TYPE_SPECTRA)
// QDOAS ???              rc=OUTPUT_LocalAlloc(&specInfo);
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Record read out
// QDOAS ???
// QDOAS ???       if (THRD_indexRecordCurrent!=ITEM_NONE)
// QDOAS ???        {
// QDOAS ???        	#if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???         THRD_delay=(specInfo.project.spectra.displayPause==0)?specInfo.project.spectra.displayDelay*1000:INFINITE;
// QDOAS ???         #endif
// QDOAS ???
// QDOAS ???         if ((rc=ThrdReadFile(&specInfo,THRD_indexRecordCurrent,0,0,specFp,namesFp,darkFp,1))<THREAD_EVENT_STOP)
// QDOAS ???          {
// QDOAS ???           // Fix indexes of the current record
// QDOAS ???
// QDOAS ???           THRD_specInfo.indexFile=specInfo.indexFile=THRD_indexFileCurrent;
// QDOAS ???           THRD_specInfo.indexProject=specInfo.indexProject=THRD_indexProjectCurrent;
// QDOAS ???           THRD_specInfo.indexRecord=THRD_indexRecordCurrent;
// QDOAS ???
// QDOAS ???           // Run analysis
// QDOAS ???
// QDOAS ???           if ((THRD_id==THREAD_TYPE_SPECTRA) ||
// QDOAS ???              ((THRD_lastEvent>=THREAD_EVENT_FIRST) && (THRD_lastEvent<=THREAD_EVENT_GOTO)))
// QDOAS ???
// QDOAS ???            waitEventFlag=0;
// QDOAS ???           else
// QDOAS ???            {
// QDOAS ???             if ((THRD_id==THREAD_TYPE_ANALYSIS) &&
// QDOAS ???                  ANALYSE_refSelectionFlag &&
// QDOAS ???                 (specInfo.project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) &&
// QDOAS ???                 (specInfo.project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_HDF) &&
// QDOAS ???                 (specInfo.project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS) &&
// QDOAS ???                 (specInfo.project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GDP_BIN) &&
// QDOAS ???                 (specInfo.project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GOME2) &&
// QDOAS ???                ((rc=ThrdNewRef(&specInfo,specFp,namesFp,darkFp))>=THREAD_EVENT_STOP))
// QDOAS ???
// QDOAS ???              THRD_delay=0;
// QDOAS ???
// QDOAS ???             if ((rc<THREAD_EVENT_STOP) && thrdRefFlag && ((rc=ANALYSE_Spectrum(&specInfo))<THREAD_EVENT_STOP) )
// QDOAS ???              waitEventFlag=0;
// QDOAS ???             else
// QDOAS ???              THRD_delay=0;
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???         else if ((rc==ERROR_ID_FILE_RECORD) || (rc==ERROR_ID_FILE_END))
// QDOAS ???          THRD_delay=0;
// QDOAS ???         else if (rc==ERROR_ID_NO_REF)
// QDOAS ???          thrdRefFlag=0;
// QDOAS ???         else
// QDOAS ???          THRD_indexRecordCurrent=ITEM_NONE;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Wait for event
// QDOAS ???
// QDOAS ???   while ((rc!=THREAD_EVENT_STOP) && (rc!=THREAD_EVENT_STOP_PROGRAM) &&
// QDOAS ???          (!waitEventFlag ||
// QDOAS ???        (((rc=THRD_WaitEvent(THRD_delay,1,1))!=THREAD_EVENT_STOP) && (rc!=THREAD_EVENT_STOP_PROGRAM))));
// QDOAS ???
// QDOAS ???   if (THRD_id==THREAD_TYPE_ANALYSIS)
// QDOAS ???    {
// QDOAS ???     for (indexFeno=0;indexFeno<NFeno;indexFeno++)
// QDOAS ???      {
// QDOAS ???       Feno=&TabFeno[indexFeno];
// QDOAS ???
// QDOAS ???       if (!Feno->hidden)
// QDOAS ???        {
// QDOAS ???         Feno->oldZmRefMorning=Feno->ZmRefMorning;
// QDOAS ???         Feno->oldZmRefAfternoon=Feno->ZmRefAfternoon;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (!(rc=OUTPUT_FlushBuffers(&specInfo)))
// QDOAS ???      OUTPUT_SaveNasaAmes();
// QDOAS ???
// QDOAS ???     if (!endAnalysis)
// QDOAS ???      THRD_ProcessLastError();
// QDOAS ???    }
// QDOAS ???   else if (THRD_id==THREAD_TYPE_KURUCZ)
// QDOAS ???    {
// QDOAS ???     rc=OUTPUT_FlushBuffers(&specInfo);
// QDOAS ???     if (!endAnalysis)
// QDOAS ???      THRD_ProcessLastError();
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Clear draw client area
// QDOAS ???
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   DRAW_ClearWindow(CHILD_WINDOW_SPECTRA);
// QDOAS ???   DRAW_ClearWindow(CHILD_WINDOW_SPECMAX);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   // Close files
// QDOAS ???
// QDOAS ???   if (specFp!=NULL)
// QDOAS ???    fclose(specFp);
// QDOAS ???   if (darkFp!=NULL)
// QDOAS ???    fclose(darkFp);
// QDOAS ???   if (namesFp!=NULL)
// QDOAS ???    fclose(namesFp);
// QDOAS ???
// QDOAS ???   if (THRD_asciiFp!=NULL)
// QDOAS ???    fclose(THRD_asciiFp);
// QDOAS ???
// QDOAS ???   // End thread
// QDOAS ???
// QDOAS ???   GDP_ASC_ReleaseBuffers();
// QDOAS ???   GDP_BIN_ReleaseBuffers();
// QDOAS ???   GOME2_ReleaseBuffers(GOME2_BEAT_CLOSE);
// QDOAS ???
// QDOAS ??? //  OMI_ReleaseBuffers();
// QDOAS ???
// QDOAS ???   SCIA_ReleaseBuffers(specInfo.project.instrumental.readOutFormat);
// QDOAS ???
// QDOAS ???   if (THRD_id!=THREAD_TYPE_SPECTRA)
// QDOAS ???    ANALYSE_ResetData();
// QDOAS ???
// QDOAS ???   THRD_ResetSpecInfo(&specInfo);
// QDOAS ???   THRD_ResetSpecInfo(&THRD_specInfo);
// QDOAS ???   THRD_ResetSpecInfo(&THRD_refInfo);
// QDOAS ???
// QDOAS ???   THRD_lastEvent=ITEM_NONE;
// QDOAS ???   THRD_id=THREAD_TYPE_NONE;
// QDOAS ???   THRD_treeCallFlag=0;
// QDOAS ???
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   TLBAR_EnableThreadButtons(FALSE);
// QDOAS ???   unlink(DOAS_tmpFile);
// QDOAS ???   if ((hwndEdit=CHILD_list[CHILD_WINDOW_SPECINFO].hwndEdit)!=NULL)
// QDOAS ???    SetWindowText(hwndEdit,"");
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   #if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_FILE_) && __DEBUG_DOAS_FILE_
// QDOAS ???   DEBUG_Stop("Thread");
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   if (THRD_endProgram)
// QDOAS ???    PostMessage(DOAS_hwndMain,WM_QUIT,(WPARAM)0,(LPARAM)0);
// QDOAS ???
// QDOAS ???   if (!DOAS_arg.analysisFlag)
// QDOAS ???    _endthread();
// QDOAS ???   #endif
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ====================
// QDOAS ??? // RESOURCES MANAGEMENT
// QDOAS ??? // ====================
// QDOAS ???
// QDOAS ??? RC THRD_Alloc(void)
// QDOAS ???
// QDOAS ??? //
// QDOAS ??? // PURPOSE
// QDOAS ??? //   allocates event objects for events processing inside the current thread
// QDOAS ??? //
// QDOAS ??? // INPUT PARAMETERS
// QDOAS ??? //   none
// QDOAS ??? //
// QDOAS ??? // OUTPUT PARAMETERS
// QDOAS ??? //   none
// QDOAS ??? //
// QDOAS ??? // GLOBAL
// QDOAS ??? //   THRD_hEvents : keeps handles created for event objects
// QDOAS ??? //
// QDOAS ??? // RETURN VALUE
// QDOAS ??? //   NO_ERROR if all requested event objects have been successfully created;
// QDOAS ??? //   ERR_MEMORY_ALLOC_EVENT otherwise;
// QDOAS ??? //
// QDOAS ??? // COMMENTS
// QDOAS ??? //   none
// QDOAS ??? //
// QDOAS ??? // REFERENCE
// QDOAS ??? //   none
// QDOAS ??? //
// QDOAS ???
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexEvent;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   SECURITY_ATTRIBUTES securityAttributes;
// QDOAS ???
// QDOAS ???   securityAttributes.nLength=sizeof(SECURITY_ATTRIBUTES);    // the size in bytes of this structure
// QDOAS ???   securityAttributes.lpSecurityDescriptor=NULL;              // use default
// QDOAS ???   securityAttributes.bInheritHandle=TRUE;
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Create events
// QDOAS ???
// QDOAS ???   if ((THRD_dataIndexes=(INDEX *)MEMORY_AllocBuffer("THRD_Alloc ","THRD_dataIndexes",SYS_raw,sizeof(INDEX),0,MEMORY_TYPE_INDEX))==NULL)
// QDOAS ???    rc=ERR_MEMORY_ALLOC_EVENT;
// QDOAS ???
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     for (indexEvent=0;indexEvent<THREAD_EVENT_MAX;indexEvent++)
// QDOAS ???      if ((THRD_hEvents[indexEvent]=CreateEvent(&securityAttributes,FALSE,FALSE,NULL))==NULL)
// QDOAS ???       break;
// QDOAS ???
// QDOAS ???     if (indexEvent<THREAD_EVENT_MAX)
// QDOAS ???      rc=ERR_MEMORY_ALLOC_EVENT;
// QDOAS ???    }
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? void THRD_Free(void)
// QDOAS ???
// QDOAS ??? //
// QDOAS ??? // PURPOSE
// QDOAS ??? //   release handles of event objects allocated by previous function
// QDOAS ??? //
// QDOAS ??? // INPUT PARAMETERS
// QDOAS ??? //   none
// QDOAS ??? //
// QDOAS ??? // OUTPUT PARAMETERS
// QDOAS ??? //   none
// QDOAS ??? //
// QDOAS ??? // GLOBAL
// QDOAS ??? //   THRD_hEvents : handles items are released
// QDOAS ??? //
// QDOAS ??? // RETURN VALUE
// QDOAS ??? //   none
// QDOAS ??? //
// QDOAS ??? // COMMENTS
// QDOAS ??? //   none
// QDOAS ??? //
// QDOAS ??? // REFERENCE
// QDOAS ??? //   none
// QDOAS ??? //
// QDOAS ???
// QDOAS ???  {
// QDOAS ???  	#if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   INDEX indexEvent;
// QDOAS ???
// QDOAS ???   for (indexEvent=0;indexEvent<THREAD_EVENT_MAX;indexEvent++)
// QDOAS ???    if (THRD_hEvents[indexEvent]!=NULL)
// QDOAS ???     CloseHandle(THRD_hEvents[indexEvent]);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   if (THRD_dataIndexes!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("THRD_Free ","THRD_dataIndexes",THRD_dataIndexes);
// QDOAS ???  }

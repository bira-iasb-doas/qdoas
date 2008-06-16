
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  PATHS INSERTION IN PROJECT TREE
//  Name of module    :  WINPATH.C
//  Creation date     :  May 1999
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
//  In project tree, it was already possible to organize files to analyze in folders.  But for
//  large amount of files, the insertion of the parent directory is more convenient.  That's why
//  a new option "Insert path" has been added in "Insert files" contextual menu.
//
//  This module includes functions for processing the "Insert/change path" dialog box and managing
//  selected files.  A filter can be used for selecting files; wildcards (*,?) characters are accepted.
//
//  When a directory is selected, a global buffer is filled in with subdirectories and selected
//  files for future use.  This list is filled in from the bottom because of the use of 'memcpy' for
//  insertions, so indexes for browsing items in list should start from the bottom of the buffer and should
//  be decremented; ascending order is respected in the browsing direction; subdirectories come before files.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  ===============================
//  STRUCTURE AND BUFFER PROCESSING
//  ===============================
//
//  PATH_GetFilesList - fill PATH_fileNames buffer with selected subdirectories or files;
//                      this function is used for filling ListView on parent path or filter changes;
//                      it's also used by thread functions for browsing files from project tree;
//
//  PATH_InsertFolder - insert a path and its subfolders in project tree;
//
//  PATH_Init - initialize panel structure at program loading;
//
//  PATH_Alloc - allocate a buffer for subdirectories and files selection;
//
//  PATH_Free - release previously allocated buffer;
//
//  ===================
//  LISTVIEW PROCESSING
//  ===================
//
//  PathInsertListViewItem - insert a new item in files and paths ListView;
//
//  PathFillFilesList - fill ListView with selected subdirectories;
//                      this function is called on parent directory of filter changes;
//
//  PathBrowseSpectra - start spectra files browsing or analysis from ListView;
//
//  PathGetSelectedImage - retrieve image of the first selected item in ListView; with this image, it is possible
//                         for the calling function to say if the selection contains subdirectories;
//
//  ===========================
//  WINDOWS MESSAGES PROCESSING
//  ===========================
//
//  PathCallBack - call back routine used by browsing dialog box for processing events;
//
//  PathDlgInit - WM_INITDIALOG message processing;
//
//  PathBrowse - PATH_BROWSE command processing (browse paths);
//
//  PathOK - IDOK command processing;
//
//  PathCommand - WM_COMMAND message processing;
//
//  PathDblClk - action to be taken on ListView item double click;
//
//  PathNotify - WM_NOTIFY message processing;
//
//  PathContext - WM_CONTEXTMENU message processing;
//
//  PATH_WndProc - dispatch messages produced by the use of the DLG_PATH dialog;
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"

// ================
// GLOBAL VARIABLES
// ================

unsigned char (*PATH_fileNamesShort)[MAX_STR_SHORT_LEN+1];                              // list of subdirectories and files found in a specified directory
INT     PATH_fileNumber;                                                        // number of files in previous list
INT     PATH_dirNumber;                                                         // number of subdirectories in previous list
INT     PATH_mfcFlag,PATH_UofTFlag;
unsigned char   PATH_fileMax[MAX_STR_SHORT_LEN+1];
unsigned char   PATH_fileMin[MAX_STR_SHORT_LEN+1];
unsigned char   PATH_fileSpectra[MAX_STR_SHORT_LEN+1];                                  // current spectra file name

// QDOAS ??? // ================
// QDOAS ??? // STATIC VARIABLES
// QDOAS ??? // ================
// QDOAS ???
// QDOAS ??? PATH_FILES_FILTER pathFilesFilter;                                              // this structure contains data retrieved from the "Insert/Change path" dialog box
// QDOAS ???
// QDOAS ??? INDEX pathIndexParent,pathIndexItem;                                            // resp. indexes of parent item and selected item in project tree (pathIndexItem==ITEM_NONE for new item)
// QDOAS ??? HWND  pathHwndTree;                                                             // handle of project tree
// QDOAS ??? INT   pathBrowseFlag;                                                           // flag set after path changes; this flag is used for filtering contextual menus options
// QDOAS ??? INT   pathInsert;                                                               // 1 if Insert mode; 0 otherwise.
// QDOAS ??? INT   pathInit;
// QDOAS ???
// QDOAS ??? // ===============================
// QDOAS ??? // STRUCTURE AND BUFFER PROCESSING
// QDOAS ??? // ===============================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PATH_GetFilesList
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       fill PATH_fileNamesShort buffer with selected subdirectories or files;
// QDOAS ??? //               this function is used for filling ListView on parent path or filter changes;
// QDOAS ??? //               it's also used by thread functions for browsing files from project tree;
// QDOAS ??? //
// QDOAS ??? // INPUT         path    : directory in which selection should be operated;
// QDOAS ??? //               filter  : filter used for selecting subdirectories or files;
// QDOAS ??? //               dirflag : 1 for selecting subdirectories; 0 for selecting files;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ???
// QDOAS ??? void PATH_GetFilesList(unsigned char *path,unsigned char *filter,unsigned char dirFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   unsigned char tmpBufferShort[MAX_STR_SHORT_LEN+1];     // temporary buffer
// QDOAS ???   WIN32_FIND_DATA fileInfo;                      // structure returned by FindFirstFile and FindNextFile APIs
// QDOAS ???   HANDLE hDir;                                   // handle to use with FindFirstFile and FindNextFile APIs
// QDOAS ???   RC rc;                                         // code returned by FindNextFile API
// QDOAS ???   INT fileCount;
// QDOAS ???
// QDOAS ???   INDEX indexFile,                               // index of new item to insert (subdirectory or file according to dirFlag) in buffer
// QDOAS ???         firstFile,                               // index of first item (subdirectory or file according to dirFlag) in buffer
// QDOAS ???         lastFile;                                // index of last item (subdirectory or file according to dirFlag) in buffer
// QDOAS ???
// QDOAS ???   INT nFile,nOther;                              // number of items (subdirectories or files according to dirFlag) in buffer
// QDOAS ???   int uofTrecordNo,uofTdayNumber,uofTYear,
// QDOAS ???       uofTrecordNo_old,uofTdayNumber_old,uofTYear_old;
// QDOAS ???   unsigned char *ptr,tmp1[3],tmp2[5];
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   memset(tmpBufferShort,0,MAX_STR_SHORT_LEN+1);
// QDOAS ???
// QDOAS ???   if (strlen(path)+strlen(filter)+1<=MAX_STR_SHORT_LEN)
// QDOAS ???    sprintf(tmpBufferShort,"%s%c%s",path,PATH_SEP,filter);                        // build complete filter string from path and filter parameters
// QDOAS ???   else
// QDOAS ???    strncpy(tmpBufferShort,filter,MAX_STR_SHORT_LEN);
// QDOAS ???
// QDOAS ???   nFile=(dirFlag)?PATH_dirNumber:PATH_fileNumber;                       // if dirflag, nFile is the number of subdirectories in buffer;
// QDOAS ???   nOther=(dirFlag)?PATH_fileNumber:PATH_dirNumber;                      // if dirflag, nOther is the number of files in buffer;
// QDOAS ???   firstFile=(dirFlag)?SYS_path:SYS_path-PATH_dirNumber;
// QDOAS ???
// QDOAS ???   fileCount=0;
// QDOAS ???
// QDOAS ???   // Use filter for browsing subdirectories and files in the specified directory
// QDOAS ???
// QDOAS ???   if (!dirFlag)
// QDOAS ???    {
// QDOAS ???     memset(PATH_fileMin,0,MAX_STR_SHORT_LEN+1);
// QDOAS ???     memset(PATH_fileMax,0,MAX_STR_SHORT_LEN+1);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   for (hDir=FindFirstFile(tmpBufferShort,&fileInfo),rc=1;(hDir!=INVALID_HANDLE_VALUE) && (rc!=0);rc=FindNextFile(hDir,&fileInfo))
// QDOAS ???
// QDOAS ???    if (((dirFlag && ((fileInfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0)) ||
// QDOAS ???         (!dirFlag && ((fileInfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==0))) &&
// QDOAS ???         ((unsigned long)nFile+nOther<SYS_path))
// QDOAS ???     {
// QDOAS ???     	uofTrecordNo=ITEM_NONE;
// QDOAS ???
// QDOAS ???      if ((PATH_UofTFlag) && ((ptr=strrchr(fileInfo.cFileName,'.'))!=NULL))
// QDOAS ???       {
// QDOAS ???      	 sscanf(fileInfo.cFileName,"%2c%02d%4c.%03d",tmp1,&uofTYear,tmp2,&uofTdayNumber);
// QDOAS ???      	 uofTrecordNo=(strchr(UOFT_figures,tmp2[2])-UOFT_figures)*UOFT_BASE+(strchr(UOFT_figures,tmp2[3])-UOFT_figures);
// QDOAS ???      	}
// QDOAS ???
// QDOAS ???      // Insert a new item (subdirectory or file according to dirFlag) in buffer
// QDOAS ???
// QDOAS ???      if (dirFlag || (!PATH_mfcFlag && !PATH_UofTFlag))
// QDOAS ???       {
// QDOAS ???        lastFile=firstFile-nFile;
// QDOAS ???
// QDOAS ???        for (indexFile=firstFile-1;indexFile>=lastFile;indexFile--)                                                  // for recall, browse buffer from bottom because
// QDOAS ???                                                                                                                      // of the use of 'memcpy' for insertions
// QDOAS ???         if (stricmp(fileInfo.cFileName,PATH_fileNamesShort[indexFile])<0)                                           // ascending order should be respected
// QDOAS ???          {
// QDOAS ???           memcpy(PATH_fileNamesShort[lastFile-1],PATH_fileNamesShort[lastFile],
// QDOAS ???                 (MAX_STR_SHORT_LEN+1)*(indexFile-lastFile+1));                                                      // release a position in buffer for insertion
// QDOAS ???           break;
// QDOAS ???          }
// QDOAS ???
// QDOAS ???        strncpy(PATH_fileNamesShort[indexFile],fileInfo.cFileName,MAX_STR_SHORT_LEN);                               // insert item in buffer
// QDOAS ???        nFile++;                                                                                                     // increment the number of items
// QDOAS ???       }
// QDOAS ???
// QDOAS ???      if (!dirFlag)
// QDOAS ???       {
// QDOAS ???       	STD_Strupr(fileInfo.cFileName);
// QDOAS ???
// QDOAS ???       	if (PATH_UofTFlag)
// QDOAS ???       	 {
// QDOAS ???          lastFile=firstFile-nFile;
// QDOAS ???          uofTYear_old=uofTdayNumber_old=uofTrecordNo_old=ITEM_NONE;
// QDOAS ???
// QDOAS ???          for (indexFile=firstFile-1;indexFile>=lastFile;indexFile--)                                                  // for recall, browse buffer from bottom because
// QDOAS ???           {
// QDOAS ???        	   sscanf(PATH_fileNamesShort[indexFile],"%2c%02d%4c.%03d",tmp1,&uofTYear_old,tmp2,&uofTdayNumber_old);
// QDOAS ???        	   uofTrecordNo_old=(strchr(UOFT_figures,tmp2[2])-UOFT_figures)*UOFT_BASE+(strchr(UOFT_figures,tmp2[3])-UOFT_figures);
// QDOAS ???
// QDOAS ???        	   if ((uofTYear_old>uofTYear) || ((uofTYear_old==uofTYear) && (uofTdayNumber_old>=uofTdayNumber)))
// QDOAS ???        	    break;
// QDOAS ???           }
// QDOAS ???
// QDOAS ???          if ((uofTYear_old==uofTYear) && (uofTdayNumber_old==uofTdayNumber) && (uofTrecordNo<uofTrecordNo_old))
// QDOAS ???           strncpy(PATH_fileNamesShort[indexFile],fileInfo.cFileName,MAX_STR_SHORT_LEN);
// QDOAS ???          else if ((uofTYear_old!=uofTYear) || (uofTdayNumber_old!=uofTdayNumber))
// QDOAS ???           {
// QDOAS ???           	if (nFile>0)
// QDOAS ???             memcpy(PATH_fileNamesShort[lastFile-1],PATH_fileNamesShort[lastFile],
// QDOAS ???                   (MAX_STR_SHORT_LEN+1)*(indexFile-lastFile+1));
// QDOAS ???            strncpy(PATH_fileNamesShort[indexFile],fileInfo.cFileName,MAX_STR_SHORT_LEN);                               // insert item in buffer
// QDOAS ???            nFile++;                                                                                                     // increment the number of items
// QDOAS ???           }
// QDOAS ???       	 }
// QDOAS ???
// QDOAS ???        if (!fileCount)
// QDOAS ???         {
// QDOAS ???          memcpy(PATH_fileMin,fileInfo.cFileName,MAX_STR_SHORT_LEN);
// QDOAS ???          memcpy(PATH_fileMax,fileInfo.cFileName,MAX_STR_SHORT_LEN);
// QDOAS ???         }
// QDOAS ???        else if (stricmp(fileInfo.cFileName,PATH_fileMin)<0)
// QDOAS ???         memcpy(PATH_fileMin,fileInfo.cFileName,MAX_STR_SHORT_LEN);
// QDOAS ???        else if (stricmp(fileInfo.cFileName,PATH_fileMax)>0)
// QDOAS ???         memcpy(PATH_fileMax,fileInfo.cFileName,MAX_STR_SHORT_LEN);
// QDOAS ???
// QDOAS ???        fileCount++;
// QDOAS ???       }
// QDOAS ???     }
// QDOAS ???
// QDOAS ???   if (!dirFlag && PATH_mfcFlag)
// QDOAS ???    {
// QDOAS ???     // Insert a new item (subdirectory or file according to dirFlag) in buffer
// QDOAS ???
// QDOAS ???     lastFile=firstFile-nFile;
// QDOAS ???
// QDOAS ???     for (indexFile=firstFile-1;indexFile>=lastFile;indexFile--)                                                  // for recall, browse buffer from bottom because
// QDOAS ???                                                                                                                  // of the use of 'memcpy' for insertions
// QDOAS ???      if (stricmp(PATH_fileMin,PATH_fileNamesShort[indexFile])<0)                                                 // ascending order should be respected
// QDOAS ???       {
// QDOAS ???        memcpy(PATH_fileNamesShort[lastFile-1],PATH_fileNamesShort[lastFile],
// QDOAS ???              (MAX_STR_SHORT_LEN+1)*(indexFile-lastFile+1));                                                      // release a position in buffer for insertion
// QDOAS ???        break;
// QDOAS ???       }
// QDOAS ???
// QDOAS ???     strncpy(PATH_fileNamesShort[indexFile],PATH_fileMin,MAX_STR_SHORT_LEN);                                      // insert item in buffer
// QDOAS ???     nFile++;                                                                                                     // increment the number of items
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if (dirFlag)
// QDOAS ???    PATH_dirNumber=nFile;
// QDOAS ???   else
// QDOAS ???    PATH_fileNumber=nFile;
// QDOAS ???
// QDOAS ???   // Close handle
// QDOAS ???
// QDOAS ???   if (hDir!=NULL)
// QDOAS ???    FindClose(hDir);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #else
// QDOAS ???
// QDOAS ??? void PATH_GetFilesList(unsigned char *path,unsigned char *filter,unsigned char dirFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   unsigned char tmpBufferShort[MAX_STR_SHORT_LEN+1];                                    // temporary buffer
// QDOAS ???   struct dirent *fileInfo;                                                      // structure returned by FindFirstFile and FindNextFile APIs
// QDOAS ???   DIR *hDir;                                                                    // handle to use with FindFirstFile and FindNextFile APIs
// QDOAS ???   RC rc;                                                                        // code returned by FindNextFile API
// QDOAS ???   INT fileCount;
// QDOAS ???   unsigned char tmpdir[MAX_STR_SHORT_LEN+1];
// QDOAS ???
// QDOAS ???   INDEX indexFile,                                                              // index of new item to insert (subdirectory or file according to dirFlag) in buffer
// QDOAS ???         firstFile,                                                              // index of first item (subdirectory or file according to dirFlag) in buffer
// QDOAS ???         lastFile;                                                               // index of last item (subdirectory or file according to dirFlag) in buffer
// QDOAS ???
// QDOAS ???   INT nFile,nOther;                                                             // number of items (subdirectories or files according to dirFlag) in buffer
// QDOAS ???   int uofTrecordNo,uofTdayNumber,uofTYear,
// QDOAS ???       uofTrecordNo_old,uofTdayNumber_old,uofTYear_old;
// QDOAS ???   unsigned char *ptr,tmp1[3],tmp2[5];
// QDOAS ???   unsigned char *ptr1,*ptr2;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   memset(tmpBufferShort,0,MAX_STR_SHORT_LEN+1);
// QDOAS ???
// QDOAS ???   if (strlen(path)+strlen(filter)+1<=MAX_STR_SHORT_LEN)
// QDOAS ???    sprintf(tmpBufferShort,"%s%c%s",path,PATH_SEP,filter);                       // build complete filter string from path and filter parameters
// QDOAS ???   else
// QDOAS ???    strncpy(tmpBufferShort,filter,MAX_STR_SHORT_LEN);
// QDOAS ???
// QDOAS ???   ptr1=strrchr(filter,'.');
// QDOAS ???
// QDOAS ???   nFile=(dirFlag)?PATH_dirNumber:PATH_fileNumber;                               // if dirflag, nFile is the number of subdirectories in buffer;
// QDOAS ???   nOther=(dirFlag)?PATH_fileNumber:PATH_dirNumber;                              // if dirflag, nOther is the number of files in buffer;
// QDOAS ???   firstFile=(dirFlag)?SYS_path:SYS_path-PATH_dirNumber;
// QDOAS ???
// QDOAS ???   fileCount=0;
// QDOAS ???
// QDOAS ???   // Use filter for browsing subdirectories and files in the specified directory
// QDOAS ???
// QDOAS ???   if (!dirFlag)
// QDOAS ???    {
// QDOAS ???     memset(PATH_fileMin,0,MAX_STR_SHORT_LEN+1);
// QDOAS ???     memset(PATH_fileMax,0,MAX_STR_SHORT_LEN+1);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   for (hDir=opendir(path),rc=1;(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL); )
// QDOAS ???   {
// QDOAS ???     sprintf(tmpdir,"%s%c%s",path,PATH_SEP,fileInfo->d_name);
// QDOAS ???
// QDOAS ???     ptr2=strrchr(fileInfo->d_name,'.');
// QDOAS ???
// QDOAS ???     if ( ( ( dirFlag && ( STD_IsDir(tmpdir) == 1 ) ) ||
// QDOAS ???            (!dirFlag && ( STD_IsDir(tmpdir) == 0 ) &&
// QDOAS ???            ( ptr1 != NULL ) && ( ptr2 != NULL ) &&
// QDOAS ???            ( strlen(ptr1) == strlen(ptr2) ) && !STD_Stricmp(ptr1,ptr2) ) ) &&
// QDOAS ???            ((unsigned long)nFile+nOther<SYS_path) )
// QDOAS ???     {
// QDOAS ???     	uofTrecordNo=ITEM_NONE;
// QDOAS ???
// QDOAS ???      if ((PATH_UofTFlag) && ((ptr=strrchr(fileInfo->d_name,'.'))!=NULL))
// QDOAS ???       {
// QDOAS ???      	 sscanf(fileInfo->d_name,"%2c%02d%4c.%03d",tmp1,&uofTYear,tmp2,&uofTdayNumber);
// QDOAS ???      	 uofTrecordNo=((unsigned char *)strchr(UOFT_figures,tmp2[2])-(unsigned char *)UOFT_figures)*UOFT_BASE+((unsigned char *)strchr(UOFT_figures,tmp2[3])-(unsigned char *)UOFT_figures);
// QDOAS ???      	}
// QDOAS ???
// QDOAS ???      // Insert a new item (subdirectory or file according to dirFlag) in buffer
// QDOAS ???
// QDOAS ???      if (dirFlag || (!PATH_mfcFlag && !PATH_UofTFlag))
// QDOAS ???       {
// QDOAS ???        lastFile=firstFile-nFile;
// QDOAS ???
// QDOAS ???        for (indexFile=firstFile-1;indexFile>=lastFile;indexFile--)              // for recall, browse buffer from bottom because
// QDOAS ???                                                                                 // of the use of 'memcpy' for insertions
// QDOAS ???         if (STD_Stricmp(fileInfo->d_name,PATH_fileNamesShort[indexFile])<0)         // ascending order should be respected
// QDOAS ???          {
// QDOAS ???           memcpy(PATH_fileNamesShort[lastFile-1],PATH_fileNamesShort[lastFile],
// QDOAS ???                 (MAX_STR_SHORT_LEN+1)*(indexFile-lastFile+1));                  // release a position in buffer for insertion
// QDOAS ???           break;
// QDOAS ???          }
// QDOAS ???
// QDOAS ???        strncpy(PATH_fileNamesShort[indexFile],fileInfo->d_name,MAX_STR_SHORT_LEN);  // insert item in buffer
// QDOAS ???        nFile++;                                                                     // increment the number of items
// QDOAS ???       }
// QDOAS ???
// QDOAS ???      if (!dirFlag)
// QDOAS ???       {
// QDOAS ???       	STD_Strupr(fileInfo->d_name);
// QDOAS ???
// QDOAS ???       	if (PATH_UofTFlag)
// QDOAS ???       	 {
// QDOAS ???          lastFile=firstFile-nFile;
// QDOAS ???          uofTYear_old=uofTdayNumber_old=uofTrecordNo_old=ITEM_NONE;
// QDOAS ???
// QDOAS ???          for (indexFile=firstFile-1;indexFile>=lastFile;indexFile--)            // for recall, browse buffer from bottom because
// QDOAS ???           {
// QDOAS ???        	   sscanf(PATH_fileNamesShort[indexFile],"%2c%02d%4c.%03d",tmp1,&uofTYear_old,tmp2,&uofTdayNumber_old);
// QDOAS ???        	   uofTrecordNo_old=((unsigned char *)strchr(UOFT_figures,tmp2[2])-(unsigned char *)UOFT_figures)*UOFT_BASE+((unsigned char *)strchr(UOFT_figures,tmp2[3])-(unsigned char *)UOFT_figures);
// QDOAS ???
// QDOAS ???        	   if ((uofTYear_old>uofTYear) || ((uofTYear_old==uofTYear) && (uofTdayNumber_old>=uofTdayNumber)))
// QDOAS ???        	    break;
// QDOAS ???           }
// QDOAS ???
// QDOAS ???          if ((uofTYear_old==uofTYear) && (uofTdayNumber_old==uofTdayNumber) && (uofTrecordNo<uofTrecordNo_old))
// QDOAS ???           strncpy(PATH_fileNamesShort[indexFile],fileInfo->d_name,MAX_STR_SHORT_LEN);
// QDOAS ???          else if ((uofTYear_old!=uofTYear) || (uofTdayNumber_old!=uofTdayNumber))
// QDOAS ???           {
// QDOAS ???           	if (nFile>0)
// QDOAS ???             memcpy(PATH_fileNamesShort[lastFile-1],PATH_fileNamesShort[lastFile],
// QDOAS ???                   (MAX_STR_SHORT_LEN+1)*(indexFile-lastFile+1));
// QDOAS ???            strncpy(PATH_fileNamesShort[indexFile],fileInfo->d_name,MAX_STR_SHORT_LEN);    // insert item in buffer
// QDOAS ???            nFile++;                                                                       // increment the number of items
// QDOAS ???           }
// QDOAS ???       	 }
// QDOAS ???
// QDOAS ???        if (!fileCount)
// QDOAS ???         {
// QDOAS ???          memcpy(PATH_fileMin,fileInfo->d_name,MAX_STR_SHORT_LEN);
// QDOAS ???          memcpy(PATH_fileMax,fileInfo->d_name,MAX_STR_SHORT_LEN);
// QDOAS ???         }
// QDOAS ???        else if (STD_Stricmp(fileInfo->d_name,PATH_fileMin)<0)
// QDOAS ???         memcpy(PATH_fileMin,fileInfo->d_name,MAX_STR_SHORT_LEN);
// QDOAS ???        else if (STD_Stricmp(fileInfo->d_name,PATH_fileMax)>0)
// QDOAS ???         memcpy(PATH_fileMax,fileInfo->d_name,MAX_STR_SHORT_LEN);
// QDOAS ???
// QDOAS ???        fileCount++;
// QDOAS ???       }
// QDOAS ???     }
// QDOAS ???   }
// QDOAS ???
// QDOAS ???   if (!dirFlag && PATH_mfcFlag)
// QDOAS ???    {
// QDOAS ???     // Insert a new item (subdirectory or file according to dirFlag) in buffer
// QDOAS ???
// QDOAS ???     lastFile=firstFile-nFile;
// QDOAS ???
// QDOAS ???     for (indexFile=firstFile-1;indexFile>=lastFile;indexFile--)                 // for recall, browse buffer from bottom because
// QDOAS ???                                                                                 // of the use of 'memcpy' for insertions
// QDOAS ???      if (STD_Stricmp(PATH_fileMin,PATH_fileNamesShort[indexFile])<0)                // ascending order should be respected
// QDOAS ???       {
// QDOAS ???        memcpy(PATH_fileNamesShort[lastFile-1],PATH_fileNamesShort[lastFile],
// QDOAS ???              (MAX_STR_SHORT_LEN+1)*(indexFile-lastFile+1));                     // release a position in buffer for insertion
// QDOAS ???        break;
// QDOAS ???       }
// QDOAS ???
// QDOAS ???     strncpy(PATH_fileNamesShort[indexFile],PATH_fileMin,MAX_STR_SHORT_LEN);     // insert item in buffer
// QDOAS ???     nFile++;                                                                    // increment the number of items
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if (dirFlag)
// QDOAS ???    PATH_dirNumber=nFile;
// QDOAS ???   else
// QDOAS ???    PATH_fileNumber=nFile;
// QDOAS ???
// QDOAS ???   // Close handle
// QDOAS ???
// QDOAS ???   if (hDir!=NULL)
// QDOAS ???    closedir(hDir);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PATH_InsertFolder
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Insert a path and its subfolders in project tree
// QDOAS ??? //
// QDOAS ??? // INPUT         path        : root path to insert first;
// QDOAS ??? //               filter      : file filter;
// QDOAS ??? //               folderFlag  : type of path to insert (path only or path+subfolders);
// QDOAS ??? //               indexParent : index of parent node in project tree;
// QDOAS ??? //
// QDOAS ??? // RETURN        index of new item in project tree
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INDEX PATH_InsertFolder(unsigned char *path,unsigned char *filter,unsigned char folderFlag,INDEX indexParent)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ???   WIN32_FIND_DATA fileInfo;                                                     // structure returned by FindFirstFile and FindNextFile APIs
// QDOAS ???   HANDLE hDir;                                                                  // handle to use with by FindFirstFile and FindNextFile APIs
// QDOAS ???   #else
// QDOAS ???   struct dirent *fileInfo;
// QDOAS ???   DIR *hDir;
// QDOAS ???   unsigned char tmpdir[MAX_STR_LEN+1];
// QDOAS ???   #endif
// QDOAS ???   SZ_LEN strLength;                                                             // string length
// QDOAS ???   RC rc;                                                                        // code returned by FindNextFile API
// QDOAS ???
// QDOAS ???   unsigned char fileNameShort[MAX_STR_SHORT_LEN+1],                                     // temporary buffer for building complete file names
// QDOAS ???        *ptr;                                                                    // pointer to filename part in previous buffer
// QDOAS ???   INDEX indexItem;                                                              // index of new item in project tree
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   hDir=NULL;
// QDOAS ???
// QDOAS ???   // Insert complete file name
// QDOAS ???
// QDOAS ???   memset(fileNameShort,0,MAX_STR_SHORT_LEN+1);
// QDOAS ???
// QDOAS ???   if (strlen(path)+strlen(filter)+1<=MAX_STR_SHORT_LEN)
// QDOAS ???    sprintf(fileNameShort,"%s%c%s",path,PATH_SEP,filter);
// QDOAS ???   else
// QDOAS ???    strncpy(fileNameShort,filter,MAX_STR_SHORT_LEN);
// QDOAS ???
// QDOAS ???   // Book the place in the tree
// QDOAS ???
// QDOAS ???   RAW_UpdateWindoasPath
// QDOAS ???    ((indexItem=TREE_InsertOneItem(pathHwndTree,fileNameShort,indexParent,TREE_ITEM_TYPE_FILE_CHILDREN,0,folderFlag,0)),
// QDOAS ???      indexParent,folderFlag);
// QDOAS ???
// QDOAS ???   FILES_RebuildFileName(fileNameShort,fileNameShort,1);
// QDOAS ???
// QDOAS ???   #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ???
// QDOAS ???   // Search for subfolders
// QDOAS ???
// QDOAS ???   if ((folderFlag==RAW_FOLDER_TYPE_PATHSUB) && ((ptr=strrchr(fileNameShort,PATH_SEP))!=NULL) &&
// QDOAS ???      ((strLength=&fileNameShort[MAX_STR_SHORT_LEN]-ptr)>=3 /* *.* */))   // number of remaining characters
// QDOAS ???    {
// QDOAS ???     memset(++ptr,0,strLength);   // !!! MFC subfolders
// QDOAS ???     strcpy(ptr,"*.*");           // !!! MFC subfolders
// QDOAS ???
// QDOAS ???     for (hDir=FindFirstFile(fileNameShort,&fileInfo),rc=1;(hDir!=INVALID_HANDLE_VALUE) && (rc!=0);rc=FindNextFile(hDir,&fileInfo))
// QDOAS ???      if (((fileInfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0) &&
// QDOAS ???           (strlen(fileInfo.cFileName)<=strLength) &&
// QDOAS ???          ((strlen(fileInfo.cFileName)>1) || (fileInfo.cFileName[0]!='.')) &&
// QDOAS ???          ((strlen(fileInfo.cFileName)>2) || ((fileInfo.cFileName[0]!='.') || (fileInfo.cFileName[1]!='.'))))
// QDOAS ???       {
// QDOAS ???        // Insert subfolder
// QDOAS ???
// QDOAS ???        strcpy(ptr,STD_Strlwr(fileInfo.cFileName));   // !!! MFC subfolders
// QDOAS ??? //       strcpy(ptr,fileInfo.cFileName);
// QDOAS ???        PATH_InsertFolder(fileNameShort,filter,folderFlag,indexItem);
// QDOAS ???       }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Close handle
// QDOAS ???
// QDOAS ???   if (hDir!=NULL)
// QDOAS ???    FindClose(hDir);
// QDOAS ???
// QDOAS ???   #else
// QDOAS ???
// QDOAS ???   // Search for subfolders
// QDOAS ???
// QDOAS ???   if ((folderFlag==RAW_FOLDER_TYPE_PATHSUB) && ((ptr=strrchr(fileNameShort,PATH_SEP))!=NULL) &&
// QDOAS ???      ((strLength=&fileNameShort[MAX_STR_SHORT_LEN]-ptr)>=3 /* *.* */))   // number of remaining characters
// QDOAS ???   {
// QDOAS ???     *ptr++=0;
// QDOAS ???
// QDOAS ???    for (hDir=opendir(fileNameShort),rc=1;(hDir!=NULL) && ((fileInfo=readdir(hDir))!=NULL); )
// QDOAS ???     {
// QDOAS ???      sprintf(tmpdir,"%s%c%s",fileNameShort,PATH_SEP,fileInfo->d_name);
// QDOAS ???
// QDOAS ???      if ( ( STD_IsDir(tmpdir) == 1 ) &&
// QDOAS ???           (strlen(fileInfo->d_name)<=strLength) &&
// QDOAS ???          ((strlen(fileInfo->d_name)>1) || (fileInfo->d_name[0]!='.')) &&
// QDOAS ???          ((strlen(fileInfo->d_name)>2) || ((fileInfo->d_name[0]!='.') ||
// QDOAS ???           (fileInfo->d_name[1]!='.'))))
// QDOAS ???
// QDOAS ???       // Insert subfolder
// QDOAS ???
// QDOAS ???       PATH_InsertFolder(tmpdir,filter,folderFlag,indexItem);
// QDOAS ???     }
// QDOAS ???   }
// QDOAS ???
// QDOAS ???   // Close handle
// QDOAS ???
// QDOAS ???   if (hDir!=NULL)
// QDOAS ???    closedir(hDir);
// QDOAS ???
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return indexItem;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PATH_Init
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       initialize panel structure at program loading
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void PATH_Init(void)
// QDOAS ???  {
// QDOAS ???   memset(&pathFilesFilter,0,sizeof(PATH_FILES_FILTER));
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PATH_Alloc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       allocate a buffer for subdirectories and files selection
// QDOAS ??? //
// QDOAS ??? // RETURN        ERROR_ID_ALLOC on allocation error;
// QDOAS ??? //               ERROR_ID_NO otherwise.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC PATH_Alloc(void)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   PATH_fileNumber=PATH_dirNumber=0;
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Buffer allocation
// QDOAS ???
// QDOAS ???   if ((PATH_fileNamesShort=(unsigned char(*)[MAX_STR_SHORT_LEN+1])MEMORY_AllocBuffer("PATH_Alloc ","PATH_fileNamesShort",SYS_path,(MAX_STR_SHORT_LEN+1),0,MEMORY_TYPE_STRING))==NULL)
// QDOAS ???    rc=ERROR_ID_ALLOC;
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PATH_Free
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Release previously allocated buffer
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void PATH_Free(void)
// QDOAS ???  {
// QDOAS ???   if (PATH_fileNamesShort!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("PATH_Free ","PATH_fileNamesShort",PATH_fileNamesShort);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ===================
// QDOAS ??? // LISTVIEW PROCESSING
// QDOAS ??? // ===================
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PathInsertListViewItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       insert a new item in files and paths ListView
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndList   : handle of the ListView control;
// QDOAS ??? //               indexItem  : index of item in ListView control;
// QDOAS ??? //               textItem   : text of item;
// QDOAS ??? //               indexImage : index of image in image list.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // NB : image list is the same as one used by project tree (open/close folders image, blank image)
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void PathInsertListViewItem(HWND hwndList,INDEX indexItem,unsigned char *textItem,INDEX indexImage)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   LV_ITEM lvi;  // list view item
// QDOAS ???
// QDOAS ???   // Fill structure with attributes of item to insert
// QDOAS ???
// QDOAS ???   lvi.mask=LVIF_TEXT|LVIF_IMAGE;     // text and image will be modified
// QDOAS ???   lvi.iItem=indexItem;               // index of item in ListView
// QDOAS ???   lvi.iSubItem=0;                    // not used
// QDOAS ???   lvi.pszText=textItem;              // modify text
// QDOAS ???   lvi.cchTextMax=strlen(textItem);   // length of previous field
// QDOAS ???   lvi.iImage=indexImage;             // modify image
// QDOAS ???
// QDOAS ???   // Insert new item
// QDOAS ???
// QDOAS ???   ListView_InsertItem(hwndList,&lvi);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PathFillFilesList
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       fill ListView with selected subdirectories and files;
// QDOAS ??? //               this function is called on parent directory of filter changes;
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndPath : handle for "Insert/Change Path" dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void PathFillFilesList(HWND hwndPath)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   HWND  hwndList;                                                               // handle for ListView
// QDOAS ???   INDEX indexItem,                                                              // index of new item in ListView control
// QDOAS ???         indexFile,                                                              // browse items in subdirectories and files buffer
// QDOAS ???         lastFile;                                                               // index of last item in subdirectories and files buffer
// QDOAS ???   unsigned char *ptr;                                                                   // pointer to folder name part in output path
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   if (pathInit)
// QDOAS ???    {
// QDOAS ???     ListView_DeleteAllItems((hwndList=GetDlgItem(hwndPath,PATH_FILES)));
// QDOAS ???     GetWindowText(GetDlgItem(hwndPath,PATH_FILTER),pathFilesFilter.filterShort,MAX_STR_SHORT_LEN);
// QDOAS ???     PATH_fileNumber=PATH_dirNumber=0;
// QDOAS ???
// QDOAS ???     // Validate fields in "Insert/Change path" dialog box
// QDOAS ???
// QDOAS ???     if ((ptr=strrchr(pathFilesFilter.outputPathShort,PATH_SEP))!=NULL)
// QDOAS ???      strcpy(pathFilesFilter.folderNameShort,ptr+1);
// QDOAS ???
// QDOAS ???     SetWindowText(GetDlgItem(hwndPath,PATH_TO_INSERT),pathFilesFilter.outputPathShort);
// QDOAS ???     SetWindowText(GetDlgItem(hwndPath,PATH_FOLDER),pathFilesFilter.folderNameShort);
// QDOAS ???
// QDOAS ???     // Fill subdirectories and files buffer with subdirectories
// QDOAS ???
// QDOAS ???     if ((pathFilesFilter.useSubfolders=(IsDlgButtonChecked(hwndPath,PATH_INCLUDE)==BST_CHECKED)?1:0)==1)
// QDOAS ???      PATH_GetFilesList(pathFilesFilter.outputPathShort,"*.*",1);
// QDOAS ???
// QDOAS ???     // Fill subdirectories and files buffer with files
// QDOAS ???
// QDOAS ???     PATH_GetFilesList(pathFilesFilter.outputPathShort,pathFilesFilter.filterShort,0);
// QDOAS ???
// QDOAS ???     // Fill ListView with items from subdirectories and files buffer
// QDOAS ???
// QDOAS ???     lastFile=SYS_path-(PATH_fileNumber+PATH_dirNumber);
// QDOAS ???     for (indexFile=SYS_path-1,indexItem=0;indexFile>=lastFile;indexFile--)
// QDOAS ???      PathInsertListViewItem(hwndList,indexItem++,(unsigned char *)PATH_fileNamesShort[indexFile],
// QDOAS ???                            ((unsigned long)indexFile>=SYS_path-PATH_dirNumber)?TREE_closeImageIndex:TREE_blankImageIndex);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PathBrowseSpectra
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       start spectra files browsing or analysis from ListView;
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndPath : handle for "Insert/Change Path" dialog box;
// QDOAS ??? //               command  : browsing command from contextual menu.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void PathBrowseSpectra(HWND hwndPath,unsigned long command)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   unsigned char   itemTextShort[MAX_STR_SHORT_LEN+1];                                   // text of selected item in ListView
// QDOAS ???   INDEX   indexItem;                                                            // index of selected item in ListView
// QDOAS ???   HWND    hwndList,hwndTree;                                                    // resp. handles of ListView and project tree
// QDOAS ???   INT     fileNumber;                                                           // number of selected items
// QDOAS ???   LV_ITEM lvi;                                                                  // data on selected item
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   hwndTree=CHILD_list[CHILD_WINDOW_PROJECT].hwndTree;
// QDOAS ???   hwndList=GetDlgItem(hwndPath,PATH_FILES);
// QDOAS ???   indexItem=ITEM_NONE;
// QDOAS ???
// QDOAS ???   memset(&lvi,0,sizeof(LV_ITEM));
// QDOAS ???   memset(itemTextShort,0,MAX_STR_SHORT_LEN+1);
// QDOAS ???
// QDOAS ???   lvi.mask=LVIF_TEXT|LVIF_IMAGE;
// QDOAS ???   lvi.pszText=itemTextShort;
// QDOAS ???   lvi.cchTextMax=MAX_STR_SHORT_LEN;
// QDOAS ???
// QDOAS ???   if (hwndList!=NULL)
// QDOAS ???    {
// QDOAS ???     fileNumber=0;
// QDOAS ???
// QDOAS ???     // Browse selected items
// QDOAS ???
// QDOAS ???     while (((indexItem=lvi.iItem=ListView_GetNextItem(hwndList,indexItem,LVNI_SELECTED))!=ITEM_NONE) &&
// QDOAS ???             ((unsigned long)fileNumber<SYS_path))
// QDOAS ???
// QDOAS ???      if (ListView_GetItem(hwndList,&lvi) && strlen(itemTextShort) && (lvi.iImage==TREE_blankImageIndex))
// QDOAS ???       {
// QDOAS ???        strcpy(PATH_fileNamesShort[SYS_path-fileNumber-1],itemTextShort);
// QDOAS ???        fileNumber++;
// QDOAS ???       }
// QDOAS ???
// QDOAS ???     // Call browsing function with correct arguments according to the requested command
// QDOAS ???
// QDOAS ???     if (fileNumber>0)
// QDOAS ???      {
// QDOAS ???       PATH_dirNumber=0;
// QDOAS ???       PATH_fileNumber=fileNumber;
// QDOAS ???
// QDOAS ???       EndDialog(hwndPath,0);
// QDOAS ???
// QDOAS ???       switch (command)
// QDOAS ???        {
// QDOAS ???      // ---------------------------------------------------------------------------
// QDOAS ???         case MENU_CONTEXT_DARK :
// QDOAS ???          RAW_BrowseSpectra(hwndTree,1,THREAD_TYPE_SPECTRA,0x01,0);              // browse dark currents
// QDOAS ???         break;
// QDOAS ???      // ---------------------------------------------------------------------------
// QDOAS ???         case MENU_CONTEXT_ERROR :
// QDOAS ???          RAW_BrowseSpectra(hwndTree,1,THREAD_TYPE_SPECTRA,0x02,0);              // errors on spectra
// QDOAS ???         break;
// QDOAS ???      // ---------------------------------------------------------------------------
// QDOAS ???         case MENU_CONTEXT_EXPORT :
// QDOAS ???          RAW_BrowseSpectra(hwndTree,1,THREAD_TYPE_SPECTRA,0x03,0);              // export spectra in ASCII
// QDOAS ???         break;
// QDOAS ???      // ---------------------------------------------------------------------------
// QDOAS ???         case MENU_CONTEXT_SPECTRA :
// QDOAS ???          RAW_BrowseSpectra(hwndTree,1,THREAD_TYPE_SPECTRA,0,0);                 // browse spectra
// QDOAS ???         break;
// QDOAS ???      // ---------------------------------------------------------------------------
// QDOAS ???         case MENU_CONTEXT_ANALYSIS :
// QDOAS ???          RAW_BrowseSpectra(hwndTree,0,THREAD_TYPE_ANALYSIS,0,0);                // run analysis
// QDOAS ???         break;
// QDOAS ???      // ---------------------------------------------------------------------------
// QDOAS ???         case MENU_CONTEXT_KURUCZ :
// QDOAS ???          RAW_BrowseSpectra(hwndTree,0,THREAD_TYPE_KURUCZ,0,0);                  // run Kurucz
// QDOAS ???         break;
// QDOAS ???      // ---------------------------------------------------------------------------
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PathGetSelectedImage
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       retrieve image of the first selected item in ListView; with this image, it is possible
// QDOAS ??? //               for the calling function to say if the selection contains subdirectories;
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndPath : handle for "Insert/Change Path" dialog box;
// QDOAS ??? //
// QDOAS ??? // RETURN        index of image of the selected item
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INDEX PathGetSelectedImage(HWND hwndPath)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   HWND hwndList;
// QDOAS ???   LV_ITEM lvi;
// QDOAS ???   INDEX rcIndex;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   hwndList=GetDlgItem(hwndPath,PATH_FILES);
// QDOAS ???   memset(&lvi,0,sizeof(LV_ITEM));
// QDOAS ???   lvi.mask=LVIF_IMAGE;
// QDOAS ???   rcIndex=ITEM_NONE;
// QDOAS ???
// QDOAS ???   // Retrieve image index from the first selected item
// QDOAS ???
// QDOAS ???   if ((hwndList!=NULL) &&
// QDOAS ???      ((lvi.iItem=ListView_GetNextItem(hwndList,ITEM_NONE,LVNI_SELECTED))!=ITEM_NONE) &&
// QDOAS ???        ListView_GetItem(hwndList,&lvi))
// QDOAS ???
// QDOAS ???    rcIndex=lvi.iImage;
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rcIndex;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ===========================
// QDOAS ??? // WINDOWS MESSAGES PROCESSING
// QDOAS ??? // ===========================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PathCallBack
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       call back routine used by browsing dialog box for processing events
// QDOAS ??? //
// QDOAS ??? // SYNTAX        usual syntax for Windows message processing;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? int CALLBACK PathCallBack(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   TCHAR szDir[MAX_PATH];
// QDOAS ???
// QDOAS ???   // Messages processing
// QDOAS ???
// QDOAS ???   switch(uMsg)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case BFFM_INITIALIZED:
// QDOAS ???
// QDOAS ???      SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)pathFilesFilter.outputPathShort); // WParam is TRUE since you are passing a path;
// QDOAS ???                                                                                        // it would be FALSE if you were passing a pidl.
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case BFFM_SELCHANGED:
// QDOAS ???
// QDOAS ???      if (SHGetPathFromIDList((LPITEMIDLIST)lParam,szDir))                              // set the status window to the currently selected path
// QDOAS ???       SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
// QDOAS ???
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PathDlgInit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_INITDIALOG message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndPath  : handle for "Insert/Change Path" dialog box
// QDOAS ??? //               indexItem : index of item in tree; ITEM_NONE for new item insertion
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void PathDlgInit(HWND hwndPath,INDEX indexItem)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexProject;
// QDOAS ???   unsigned char *ptr;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pathInit=0;
// QDOAS ???   PATH_mfcFlag=PATH_UofTFlag=0;
// QDOAS ???
// QDOAS ???   pathHwndTree=CHILD_list[CHILD_WINDOW_PROJECT].hwndTree;
// QDOAS ???   pathIndexParent=(indexItem==ITEM_NONE)?TREE_GetSelectedItem(pathHwndTree):TREE_itemList[indexItem].parentItem;
// QDOAS ???   pathBrowseFlag=0;
// QDOAS ???
// QDOAS ???   if ((indexProject=TREE_GetProjectParent(pathIndexParent))!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     PATH_mfcFlag=((PRJCT_itemList[indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
// QDOAS ???                   (PRJCT_itemList[indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD))?1:0;
// QDOAS ???
// QDOAS ???     PATH_UofTFlag=(PRJCT_itemList[indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_UOFT)?1:0;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Center dialog box on parent windows
// QDOAS ???
// QDOAS ???   DOAS_CenterWindow(hwndPath,GetWindow(hwndPath,GW_OWNER));
// QDOAS ???
// QDOAS ???   // Buffers initialization
// QDOAS ???
// QDOAS ???   memset(pathFilesFilter.folderNameShort,0,MAX_STR_SHORT_LEN+1);
// QDOAS ???
// QDOAS ???   if ((pathIndexItem=indexItem)==ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     // Initialize structure for new item in project tree
// QDOAS ???
// QDOAS ???     FILES_RetrievePath(pathFilesFilter.outputPathShort,MAX_STR_SHORT_LEN,NULL,0,FILE_TYPE_PATH,1);
// QDOAS ???
// QDOAS ???     if (!strlen(pathFilesFilter.filterShort))
// QDOAS ???      strcpy(pathFilesFilter.filterShort,"*.*");
// QDOAS ???
// QDOAS ???     if ((ptr=strrchr(pathFilesFilter.outputPathShort,PATH_SEP))!=NULL)
// QDOAS ???      strcpy(pathFilesFilter.folderNameShort,ptr+1);
// QDOAS ???    }
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     pathFilesFilter.useSubfolders=(RAW_GetFolderFlag(indexItem)==RAW_FOLDER_TYPE_PATHSUB)?1:0;
// QDOAS ???
// QDOAS ???     FILES_RebuildFileName(pathFilesFilter.outputPathShort,RAW_spectraFiles[TREE_itemList[indexItem].dataIndex].fileName,1);
// QDOAS ???
// QDOAS ???     // Rebuild filter
// QDOAS ???
// QDOAS ???     if ((ptr=strrchr(pathFilesFilter.outputPathShort,PATH_SEP))!=NULL)
// QDOAS ???      {
// QDOAS ???       *ptr++=0;
// QDOAS ???       strcpy(pathFilesFilter.filterShort,ptr);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Rebuild folder name
// QDOAS ???
// QDOAS ???     if ((ptr=strrchr(pathFilesFilter.outputPathShort,PATH_SEP))!=NULL)
// QDOAS ???      strcpy(pathFilesFilter.folderNameShort,ptr+1);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Set flag
// QDOAS ???
// QDOAS ???   CheckDlgButton(hwndPath,PATH_INCLUDE,(pathFilesFilter.useSubfolders!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???
// QDOAS ???   // Fill fields from structure
// QDOAS ???
// QDOAS ???   SetWindowText(GetDlgItem(hwndPath,PATH_TO_INSERT),pathFilesFilter.outputPathShort);
// QDOAS ???   SetWindowText(GetDlgItem(hwndPath,PATH_FOLDER),pathFilesFilter.folderNameShort);
// QDOAS ???   SetWindowText(GetDlgItem(hwndPath,PATH_FILTER),pathFilesFilter.filterShort);
// QDOAS ???
// QDOAS ???   // For subdirectories properties, hide some push buttons
// QDOAS ???
// QDOAS ???   if (RAW_GetFolderFlag(pathIndexParent)==RAW_FOLDER_TYPE_PATHSUB)
// QDOAS ???    {
// QDOAS ???     pathInsert=0;
// QDOAS ???     EnableWindow(GetDlgItem(hwndPath,PATH_FILTER),FALSE);
// QDOAS ???     ShowWindow(GetDlgItem(hwndPath,PATH_BROWSE),SW_HIDE);
// QDOAS ???     ShowWindow(GetDlgItem(hwndPath,IDOK),SW_HIDE);
// QDOAS ???     ShowWindow(GetDlgItem(hwndPath,PATH_INCLUDE),SW_HIDE);
// QDOAS ???    }
// QDOAS ???   else
// QDOAS ???    pathInsert=1;
// QDOAS ???
// QDOAS ???   // use TREE_hImageList image list (icons to use are the same in both cases)
// QDOAS ???
// QDOAS ???   ListView_SetImageList(GetDlgItem(hwndPath,PATH_FILES),TREE_hImageList,LVSIL_SMALL);
// QDOAS ???
// QDOAS ???   // Fill ListView with files
// QDOAS ???
// QDOAS ???   pathInit=1;
// QDOAS ???
// QDOAS ???   PathFillFilesList(hwndPath);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PathBrowse
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       PATH_BROWSE command processing (browse paths)
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndPath : handle for "Insert/Change Path" dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void PathBrowse(HWND hwndPath)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   ITEMIDLIST *pRc;
// QDOAS ???   BROWSEINFO  browseInfo;
// QDOAS ???   unsigned char       title[MAX_STR_SHORT_LEN+1];
// QDOAS ???
// QDOAS ???   // Initialize browseInfo structure
// QDOAS ???
// QDOAS ???   strcpy(title,"Select path with spectra files to insert in project tree");     // title to display above the directories tree
// QDOAS ???
// QDOAS ???   browseInfo.hwndOwner=hwndPath;                                                // handle to the owner window for the dialog box
// QDOAS ???
// QDOAS ???   browseInfo.pidlRoot=NULL;                                                     // address of an ITEMIDLIST structure specifying the location of the root
// QDOAS ???                                                                                 // folder from which to browse; if this member is NULL, the namespace root
// QDOAS ???                                                                                 // (the desktop folder) is used
// QDOAS ???
// QDOAS ???   browseInfo.pszDisplayName=                                                    // address of a buffer to receive the display name of the folder selected
// QDOAS ???    pathFilesFilter.folderNameShort;                                             // by the user. The size of this buffer is assumed to be MAX_PATH bytes.
// QDOAS ???
// QDOAS ???   browseInfo.lpszTitle=title;                                                   // address of a null-terminated string that is displayed above the tree
// QDOAS ???                                                                                 // view control in the dialog box.  This string can be used to specify
// QDOAS ???                                                                                 // instructions to the user
// QDOAS ???
// QDOAS ???   browseInfo.ulFlags=(UINT)                                                     // flags specifying the options for the dialog box
// QDOAS ???
// QDOAS ???     BIF_STATUSTEXT;                                                             // include a status area in the dialog box
// QDOAS ??? /*
// QDOAS ???     BIF_USENEWUI|                                                               // use the most recent release of the dialog box with larger possibilities including drag and drop, delete ...
// QDOAS ???     BIF_EDITBOX|                                                                // the user can specify the name of an item in an edit control
// QDOAS ???     BIF_VALIDATE|                                                               // send the BFFM_VALIDATEFAILED message to the call back function
// QDOAS ???     BIF_BROWSEINCLUDEFILES;                                                     // the browse dialog will display files as well as folders
// QDOAS ??? */
// QDOAS ???   browseInfo.lpfn=(BFFCALLBACK)PathCallBack;                                    // address of an application-defined function that the dialog box calls when an event occurs.
// QDOAS ???
// QDOAS ???   browseInfo.lParam=0;                                                          // application-defined value that the dialog box passes to the callback function, if one is specified
// QDOAS ???   browseInfo.iImage=0;                                                          // Variable to receive the image associated with the selected folder.
// QDOAS ???                                                                                 // The image is specified as an index to the system image list.
// QDOAS ???
// QDOAS ???   // Call to browse shell directories dialog box
// QDOAS ???
// QDOAS ???   if (((pRc=SHBrowseForFolder(&browseInfo))!=NULL) &&                           // use OK push button
// QDOAS ???         SHGetPathFromIDList((LPITEMIDLIST)pRc,pathFilesFilter.outputPathShort)) // set the status window to the currently selected path
// QDOAS ???
// QDOAS ???    FILES_ChangePath(FILES_types[FILE_TYPE_PATH].defaultPath,pathFilesFilter.outputPathShort,0);
// QDOAS ???
// QDOAS ???   // Fill entry fields
// QDOAS ???
// QDOAS ???   SetWindowText(GetDlgItem(hwndPath,PATH_TO_INSERT),pathFilesFilter.outputPathShort);
// QDOAS ???   SetWindowText(GetDlgItem(hwndPath,PATH_FOLDER),pathFilesFilter.folderNameShort);
// QDOAS ???
// QDOAS ???   // Fill ListView
// QDOAS ???
// QDOAS ???   PathFillFilesList(hwndPath);
// QDOAS ???
// QDOAS ???   pathBrowseFlag=1;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PathOK
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       IDOK command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndPath  : handle for "Insert/Change Path" dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void PathOK(HWND hwndPath)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexItem;   // index of new item in project tree;
// QDOAS ???
// QDOAS ???   if (RAW_GetFolderFlag(pathIndexParent)!=RAW_FOLDER_TYPE_PATHSUB)              // only for parent nodes
// QDOAS ???    {
// QDOAS ???     // Delete previous node in project tree
// QDOAS ???
// QDOAS ???     if (pathIndexItem!=ITEM_NONE)
// QDOAS ???      {
// QDOAS ???       TreeView_DeleteItem(pathHwndTree,TREE_itemList[pathIndexItem].hti);
// QDOAS ???       TREE_DeleteOneItem(pathIndexItem);
// QDOAS ???       TREE_UpdateItem(pathHwndTree,pathIndexParent);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Insert item in RAW spectra files list
// QDOAS ???
// QDOAS ???     if ((indexItem=PATH_InsertFolder(pathFilesFilter.outputPathShort,pathFilesFilter.filterShort,
// QDOAS ???                             (unsigned char)((pathFilesFilter.useSubfolders!=0)?RAW_FOLDER_TYPE_PATHSUB:RAW_FOLDER_TYPE_PATH),
// QDOAS ???                                      pathIndexParent))!=ITEM_NONE)
// QDOAS ???
// QDOAS ???  /*   if ((indexItem=PATH_InsertFolder(STD_STD_Strlwr(pathFilesFilter.outputPathShort),STD_STD_Strlwr(pathFilesFilter.filterShort),
// QDOAS ???                                     (pathFilesFilter.useSubfolders!=0)?(unsigned char)RAW_FOLDER_TYPE_PATHSUB:(unsigned char)RAW_FOLDER_TYPE_PATH,
// QDOAS ???                                      pathIndexParent))!=ITEM_NONE) */
// QDOAS ???
// QDOAS ???      TREE_SortChildNodes(pathHwndTree,pathIndexParent); // sort child nodes if any
// QDOAS ???
// QDOAS ???     // Insert item and its child nodes in project tree by expanding parent node
// QDOAS ???
// QDOAS ???     TREE_CollapseChildNodes(pathHwndTree,pathIndexParent);
// QDOAS ???     TREE_ExpandAll(pathHwndTree,pathIndexParent);
// QDOAS ???     TREE_UpdateItem(pathHwndTree,pathIndexParent);
// QDOAS ???
// QDOAS ???     // Change selection
// QDOAS ???
// QDOAS ???     TreeView_SelectItem(pathHwndTree,TREE_itemList[indexItem].hti);
// QDOAS ???
// QDOAS ???     // Exit dialog box
// QDOAS ???
// QDOAS ???     if (!TLBAR_bSaveFlag)
// QDOAS ???      TLBAR_Enable(TRUE);
// QDOAS ???
// QDOAS ???     EndDialog(hwndPath,0);   // Close dialog box
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PathCommand
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_COMMAND message processing
// QDOAS ??? //
// QDOAS ??? // SYNTAX        usual syntax for Windows message processing;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? LRESULT CALLBACK PathCommand(HWND hwndPath,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   unsigned long command;
// QDOAS ???
// QDOAS ???   switch((command=(unsigned long)GET_WM_COMMAND_ID(mp1,mp2)))
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case PATH_BROWSE :                                                          // browse folders
// QDOAS ???      PathBrowse(hwndPath);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case MENU_CONTEXT_SPECTRA :                                                 // browse spectra
// QDOAS ???     case MENU_CONTEXT_DARK :                                                    // browse dark current
// QDOAS ???     case MENU_CONTEXT_EXPORT :                                                  // export in ascii
// QDOAS ???     case MENU_CONTEXT_ANALYSIS :                                                // run analysis
// QDOAS ???     case MENU_CONTEXT_KURUCZ :                                                  // run Kurucz
// QDOAS ???      PathBrowseSpectra(hwndPath,command);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case PATH_FILTER :                                                          // update files filter
// QDOAS ???     case PATH_INCLUDE :                                                         // include subfolders
// QDOAS ???      PathFillFilesList(hwndPath);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDOK :
// QDOAS ???      PathOK(hwndPath);                                                          // close window on IDOK button command
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDCANCEL :
// QDOAS ???      EndDialog(hwndPath,FALSE);        // close window on CANCEL button command
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDHELP :
// QDOAS ???      WinHelp(hwndPath,DOAS_HelpPath,HELP_CONTEXT,(pathInsert)?HLP_PATH_INSERT:HLP_PRJCT_COMPONENT_FILE);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PathDblClk
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Action to be taken on ListView item double click
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndPath : handle for "Insert/Change Path" dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void PathDblClk(HWND hwndPath)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   unsigned char  itemTextShort[MAX_STR_SHORT_LEN+1],                                    // text of selected item
// QDOAS ???         *ptr;                                                                   // pointer to file name part of selected item
// QDOAS ???   HWND hwndList;                                                                // handle of ListView control
// QDOAS ???   LV_ITEM lvi;                                                                  // attributes of the selected item in ListView control
// QDOAS ???
// QDOAS ???
// QDOAS ???   if (RAW_GetFolderFlag(pathIndexParent)!=RAW_FOLDER_TYPE_PATHSUB)              // only for parent nodes
// QDOAS ???    {
// QDOAS ???     // Initializations
// QDOAS ???
// QDOAS ???     memset(&lvi,0,sizeof(LV_ITEM));
// QDOAS ???     memset(itemTextShort,0,MAX_STR_SHORT_LEN+1);
// QDOAS ???
// QDOAS ???     hwndList=GetDlgItem(hwndPath,PATH_FILES);
// QDOAS ???     pathBrowseFlag=1;
// QDOAS ???
// QDOAS ???     // Search for the item (subdirectory only) that has the selection
// QDOAS ???
// QDOAS ???     if ((hwndList!=NULL) && ((lvi.iItem=ListView_GetNextItem(hwndList,ITEM_NONE,LVNI_SELECTED))!=ITEM_NONE))
// QDOAS ???      {
// QDOAS ???       lvi.mask=LVIF_TEXT|LVIF_IMAGE;
// QDOAS ???       lvi.pszText=itemTextShort;
// QDOAS ???       lvi.cchTextMax=MAX_STR_SHORT_LEN;
// QDOAS ???
// QDOAS ???       // Get text item (subdirectory only)
// QDOAS ???
// QDOAS ???       if (ListView_GetItem(hwndList,&lvi) && (lvi.iImage==TREE_closeImageIndex) && strlen(itemTextShort) && ((strlen(itemTextShort)>1) || (itemTextShort[0]!='.')))
// QDOAS ???        {
// QDOAS ???         // go up to parent folder
// QDOAS ???
// QDOAS ???         if ((strlen(itemTextShort)==2) && !strcmp(itemTextShort,".."))
// QDOAS ???          {
// QDOAS ???           if ((ptr=strrchr(pathFilesFilter.outputPathShort,PATH_SEP))!=NULL)
// QDOAS ???            *ptr=0;
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         // go down to subdirectory
// QDOAS ???
// QDOAS ???         else
// QDOAS ???          {
// QDOAS ???          	unsigned char pathSep[2];
// QDOAS ???
// QDOAS ???           pathSep[0]=PATH_SEP;
// QDOAS ???           pathSep[1]='\0';
// QDOAS ???
// QDOAS ???           strcat(pathFilesFilter.outputPathShort,pathSep);
// QDOAS ???           strcat(pathFilesFilter.outputPathShort,itemTextShort);
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         // Change subdirectories and files selection in ListView
// QDOAS ???
// QDOAS ???         PathFillFilesList(hwndPath);
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PathNotify
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_NOTIFY message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndPath : handle for "Insert/Change Path" dialog box;
// QDOAS ??? //               pHdr : pointer to a structure with information about the notification message;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? void PathNotify(HWND hwndPath,NMHDR *pHdr)
// QDOAS ???  {
// QDOAS ???   switch (pHdr->code)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case NM_DBLCLK  :
// QDOAS ???      PathDblClk(hwndPath);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PathContext
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_CONTEXTMENU message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndParent : handle of parent control;
// QDOAS ??? //               mp2 : coordinates of the shortcut menu.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void PathContext(HWND hwndParent,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   HMENU hMenu;                                                                  // context menu handle
// QDOAS ???
// QDOAS ???   // Context menu is open only on validated path changes and only on files selection
// QDOAS ???
// QDOAS ???   if ((pathIndexItem==ITEM_NONE) || pathBrowseFlag)
// QDOAS ???    MSG_MessageBox(hwndParent,ITEM_NONE,IDS_TITLE_PATH,IDS_MSGBOX_PATH_VALIDATE,MB_OK|MB_ICONHAND);
// QDOAS ???   else if (PathGetSelectedImage(hwndParent)!=TREE_blankImageIndex)
// QDOAS ???    MSG_MessageBox(hwndParent,ITEM_NONE,IDS_TITLE_PATH,IDS_MSGBOX_PATH_DIRECTORY,MB_OK|MB_ICONHAND);
// QDOAS ???
// QDOAS ???   // Open contextual menu
// QDOAS ???
// QDOAS ???   else if ((hMenu=LoadMenu(DOAS_hInst,MAKEINTRESOURCE(MENU_CONTEXT_PATH)))!=NULL)
// QDOAS ???    {
// QDOAS ???    	if (THRD_id!=THREAD_TYPE_NONE)
// QDOAS ???    	 {
// QDOAS ???    	  EnableMenuItem(hMenu,MENU_CONTEXT_SPECTRA,MF_GRAYED);
// QDOAS ???    	  EnableMenuItem(hMenu,MENU_CONTEXT_EXPORT,MF_GRAYED);
// QDOAS ???    	  EnableMenuItem(hMenu,MENU_CONTEXT_ANALYSIS,MF_GRAYED);
// QDOAS ???    	  EnableMenuItem(hMenu,MENU_CONTEXT_KURUCZ,MF_GRAYED);
// QDOAS ???    	 }
// QDOAS ???
// QDOAS ???     TrackPopupMenu(GetSubMenu(hMenu,0),                                         // first item of the context menu
// QDOAS ???                    TPM_LEFTALIGN|TPM_LEFTBUTTON,                                // screen position and mouse button flags
// QDOAS ???                    LOWORD(mp2),HIWORD(mp2),                                     // coordinates of the shortcut menu
// QDOAS ???                    0,                                                           // reserved
// QDOAS ???                    (HWND)hwndParent,                                            // handle of the parent control
// QDOAS ???                    NULL);                                                       // no-dismissal area
// QDOAS ???
// QDOAS ???     // Destroy menu
// QDOAS ???
// QDOAS ???     DestroyMenu(hMenu);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      PATH_WndProc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       dispatch messages produced by the use of the DLG_PATH dialog;
// QDOAS ??? //
// QDOAS ??? // SYNTAX        usual syntax for Windows message processing;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? LRESULT CALLBACK PATH_WndProc(HWND hwndPath,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch (msg)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_INITDIALOG :
// QDOAS ???      PathDlgInit(hwndPath,(INDEX)mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_COMMAND :
// QDOAS ???      PathCommand(hwndPath,msg,mp1,mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_NOTIFY :
// QDOAS ???      PathNotify(hwndPath,(NMHDR *)mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_CONTEXTMENU :
// QDOAS ???      PathContext(hwndPath,mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif // __WINDOAS_GUI_

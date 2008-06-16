
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  FILES MODULE
//  Name of module    :  WINFILES.C
//  Creation date     :  1997
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
//  ================
//  PATHS PROCESSING
//  ================
//
//  FILES_CompactPath - replace paths by a number in order to compact path strings;
//  FILES_RemoveOnePath - decrease the number of times a path is used;
//  FILES_RebuildFileName - rebuild a file path from a compacted string;
//  FILES_ChangePath - change the path of a file;
//
//  FILES_RetrievePath - for a given file type, change the current directory to the last
//                       directory used for this file type of to the one specified in the
//                       given file name;
//
//  =========
//  ALL PATHS
//  =========
//
//  FilesResetAllPaths - reset all the paths associated to the different files types;
//  FilesLoadAllPaths - load all paths from wds configuration file;
//  FilesSaveAllPaths - all paths safe keeping into the selected wds configuration file;
//
//  =============
//  DEFAULT PATHS
//  =============
//
//  FilesResetDefaultPaths - reset default paths;
//  FilesLoadDefaultPaths - load default paths from wds configuration file;
//  FilesSaveDefaultPaths - default paths safe keeping into the selected wds configuration file;
//
//  =====================================
//  UTILITIES FOR LOADING DATA FROM FILES        !!! obsolete, to replace by MATRIX_Load
//  =====================================
//
//  FILES_GetMatrixDimensions - retrieve the dimensions of the matrix from a file;
//  FILES_LoadMatrix - load a matrix from a file
//
//  ================================================
//  OPEN/SAVE COMMON DIALOG BOX FOR SELECTING A FILE
//  ================================================
//
//  FilesBuildFilter - build a filter string
//
//  FILES_Open - load and display "Open/Save" windows common dialog box;
//  FILES_BuildFileName - build a file name from a given name and extension;
//  FILES_Select - select a file of a given type;
//  FILES_Insert - MENU_CONTEXT_INSERT_FILES context menu command processing;
//
//  =============================
//  CONFIGURATION FILE MANAGEMENT
//  =============================
//
//  FilesResetConfiguration - dispatch the reset command to all data structures;
//  FILES_LoadConfiguration - load WINDOAS settings from configuration file;
//  FilesSaveConfiguration - save WinDOAS settings in the configuration file;
//
//  ================================
//  "FILES" MENU COMMANDS PROCESSING
//  ================================
//
//  FILES_EnableSaveOption - enable/Disable "Save" menu option;
//  FILES_MenuNew - process MENU_FILE_NEW menu command;
//  FILES_MenuOpen - process MENU_FILE_OPEN menu command;
//  FILES_MenuSave - process MENU_FILE_SAVE menu command;
//  FILES_MenuSaveAs - process MENU_FILE_SAVE_AS menu command;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  FILES_Alloc - allocate and initialize buffers for file paths;
//  FILES_Free - release buffers allocated for file paths;
//
//  ----------------------------------------------------------------------------

// ===========
// DEFINITIONS
// ===========

#include "doas.h"

FILE_TYPE FILES_types[FILE_TYPE_MAX] =
 {
  { "All Files", "*", "" },                                                     // FILE_TYPE_ALL
  { "ASCII Files","asc","" },                                                   // FILE_TYPE_ASCII_EXPORT
  { "ASCII Spectra Files","asc","" },                                           // FILE_TYPE_ASCII_SPECTRA
  { "Raw Spectra Files", "spe", "" },                                           // FILE_TYPE_SPECTRA
  { "Cross sections Files", "xs*", "" },                                        // FILE_TYPE_CROSS
  { "Reference spectra Files", "ref", "" },                                     // FILE_TYPE_REF
  { "SZA dependent AMF Files", "amf_sza", "" },                                 // FILE_TYPE_AMF_SZA
  { "Climatology dependent AMF Files", "amf_cli", "" },                         // FILE_TYPE_AMF_CLI
  { "Wavelength dependent AMF Files", "amf_wve", "" },                          // FILE_TYPE_AMF_WVE
  { "Spectra names Files", "nms", "" },                                         // FILE_TYPE_NAMES
  { "Dark current Files", "drk", "" },                                          // FILE_TYPE_DARK
  { "Interpixel variability Files", "vip", "" },                                // FILE_TYPE_INTERPIXEL
  { "Detector not linearity Files", "dnl", "" },                                // FILE_TYPE_NOT_LINEARITY
  { "Calibration Files", "clb", "" },                                           // FILE_TYPE_CALIB
  { "Kurucz Files", "Ktz*", "" },                                               // FILE_TYPE_CALIB_KURUCZ
  { "Slit functions", "slf", "" },                                              // FILE_TYPE_SLIT
  { "Instrumental functions", "ins", "" },                                      // FILE_TYPE_INSTR
  { "Filter Files", "flt", ""},                                                 // FILE_TYPE_FILTER
  { "Fits Files","fit",""},                                                     // FILE_TYPE_FIT
  { "WINDOAS settings","wds", "" },                                             // FILE_TYPE_INI
  { "Bitmap file","bmp","" },                                                   // FILE_TYPE_BMP
  { "Residuals","res","" },                                                     // FILE_TYPE_RES
  { "Paths","","" },                                                            // FILE_TYPE_PATH
  { "Configuration","cfg",""}                                                   // FILE_TYPE_CFG
 };

FILE_TYPE FILES_typeSpectra[FILE_TYPE_SPECTRA_MAX] =
 {
  { "ASCII Spectra files with header","asc","" },                               // WITH COMMENT
  { "ASCII Spectra files without header", "asc", "" }                           // WITHOUT COMMENT
 };

unsigned char FILES_configuration[MAX_PATH_LEN+1];                                      // configuration file
// QDOAS ??? INT FILES_version=HELP_VERSION_MAX-1;                                           // file version
FILES_PATH *FILES_paths;                                                        // all paths implied in configuration file

INT FILES_nPaths=FILES_PATH_MAX;                                                // the size of the FILES_paths

// QDOAS ??? // ===============
// QDOAS ??? // PROGRAM VERSION
// QDOAS ??? // ===============
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      FilesLoadProgramVersion
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       get the version of the selected configuration file
// QDOAS ??? //
// QDOAS ??? // INPUT         fileLine   the line of the configuration file with program version information
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void FilesLoadProgramVersion(unsigned char *fileLine)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexVersion;
// QDOAS ???   unsigned char version[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   memset(version,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???   // Browse versions
// QDOAS ???
// QDOAS ???   if (sscanf(fileLine,"%[^'\n']",version))
// QDOAS ???    {
// QDOAS ???     for (indexVersion=0;indexVersion<HELP_VERSION_MAX;indexVersion++)
// QDOAS ???      if (!STD_Stricmp(version,HELP_programVersions[indexVersion]))
// QDOAS ???       break;
// QDOAS ???
// QDOAS ???     #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???     if (((FILES_version=indexVersion)!=HELP_VERSION_MAX-1) && !TLBAR_bSaveFlag)
// QDOAS ???      TLBAR_Enable(TRUE);
// QDOAS ???     #endif
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      FilesSaveProgramVersion
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       save program version into the selected wds configuration file
// QDOAS ??? //
// QDOAS ??? // INPUT         fp            pointer to the selected wds configuration file;
// QDOAS ??? //               sectionName   name of the help section in the configuration file
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void FilesSaveProgramVersion(FILE *fp,unsigned char *sectionName)
// QDOAS ???  {
// QDOAS ???   fprintf(fp,"[%s]\n\n",sectionName);                                           // section
// QDOAS ???   fprintf(fp,"%s\n\n",HELP_programVersions[HELP_VERSION_MAX-1]);                // current program version
// QDOAS ???  }

// ================
// PATHS PROCESSING
// ================

// -----------------------------------------------------------------------------
// FUNCTION      FILES_CompactPath
// -----------------------------------------------------------------------------
// PURPOSE       replace paths by a number in order to compact path strings
//
// INPUT         path          the original path
//               useFileName   add the file name to the new path
//               addFlag       increment the number of times the path is used
//
// OUTPUT        newPath       the new path (%x\<file name>)
// -----------------------------------------------------------------------------

void FILES_CompactPath(unsigned char *newPath,unsigned char *path,INT useFileName,INT addFlag)
 {
  // Declarations

  unsigned char  pathTmp[MAX_PATH_LEN+1],*ptr;
  INDEX  indexPath,indexFirst;
  SZ_LEN pathLength,pathTmpLength;

  // Initialization

  ptr=NULL;

  if (strlen(path))
   {
    // Initializations

    memcpy(pathTmp,path,MAX_PATH_LEN+1);
    indexFirst=ITEM_NONE;

    // Extract file name

    if (useFileName && ((ptr=strrchr(pathTmp,PATH_SEP))!=NULL))
     *ptr++=0;

    if (pathTmp[0]!='%')
     {
      // Search for path in list

      for (indexPath=0,pathTmpLength=strlen(pathTmp);indexPath<FILES_nPaths;indexPath++)
       if (((pathLength=strlen(FILES_paths[indexPath].path))==pathTmpLength) && !STD_Stricmp(FILES_paths[indexPath].path,pathTmp))
        break;
       else if (addFlag && !pathLength && (indexFirst==ITEM_NONE))
        indexFirst=indexPath;

      // Add a new path in list

      if ((indexPath==FILES_nPaths) && (indexFirst!=ITEM_NONE))
       {
        memcpy(FILES_paths[indexFirst].path,pathTmp,MAX_PATH_LEN+1);
        FILES_paths[indexFirst].count=0;
        indexPath=indexFirst;
       }
     }
    else
     sscanf(pathTmp,"%%%d",&indexPath);

    // Rebuild path

    if ((indexPath>=0) && (indexPath<FILES_nPaths))
     {
      if (useFileName && (ptr!=NULL))
       sprintf(newPath,"%%%d%c%s",indexPath,PATH_SEP,ptr);
      else
       sprintf(newPath,"%%%d",indexPath);

      if (addFlag)
       FILES_paths[indexPath].count++;
     }
    else if (newPath!=path)
     memcpy(newPath,path,MAX_PATH_LEN+1);
   }
  else
   memset(newPath,0,MAX_PATH_LEN);
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILES_RemoveOnePath
// -----------------------------------------------------------------------------
// PURPOSE       decrease the number of times a path is used
//
// INPUT         path          the original path
// -----------------------------------------------------------------------------

void FILES_RemoveOnePath(unsigned char *path)
 {
  // Declaration

  INDEX indexPath;

  // Remove path

  if (strlen(path) && (path[0]=='%') && (sscanf(path,"%%%d",&indexPath)>=1) &&
     (indexPath>=0) && (indexPath<FILES_nPaths) && (FILES_paths[indexPath].count>0))

   FILES_paths[indexPath].count--;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILES_RebuildFileName
// -----------------------------------------------------------------------------
// PURPOSE       rebuild a file path from a compacted string
//
// INPUT         path          the compacted path
//               useFileName   add the file name to the new path
//
// OUTPUT        newPath       the new path (%x\<file name>)
//
// RETURN        pointer to the new path
// -----------------------------------------------------------------------------

unsigned char *FILES_RebuildFileName(unsigned char *newPath,unsigned char *path,INT useFileName)
 {
 	unsigned char pathTmp[MAX_PATH_LEN+1],*ptr;

 	strcpy(pathTmp,path);

 	if (!useFileName)
 	 {
 	 	if ((ptr=strrchr(pathTmp,PATH_SEP))==NULL)
 	 	 pathTmp[0]='\0';
 	 	else
 	 	 *ptr='\0';
 	 }

 	strcpy(newPath,pathTmp);

// QDOAS ???  // Declarations
// QDOAS ???
// QDOAS ???  unsigned char pathTmp[MAX_PATH_LEN+1],*ptr;
// QDOAS ???  INDEX indexPath;
// QDOAS ???
// QDOAS ???  // Initialization
// QDOAS ???
// QDOAS ???  if (path[0]=='%')
// QDOAS ???   {
// QDOAS ???    strcpy(pathTmp,path);
// QDOAS ???    ptr=NULL;
// QDOAS ???
// QDOAS ???    // Extract file name
// QDOAS ???
// QDOAS ???    if (useFileName && ((ptr=strrchr(pathTmp,PATH_SEP))!=NULL))
// QDOAS ???     *ptr++=0;
// QDOAS ???
// QDOAS ???    if ((sscanf(path,"%%%d",&indexPath)>0) && (indexPath>=0) && (indexPath<FILES_nPaths))
// QDOAS ???     {
// QDOAS ???      if (useFileName && (ptr!=NULL))
// QDOAS ???       sprintf(newPath,"%s%c%s",FILES_paths[indexPath].path,PATH_SEP,ptr);
// QDOAS ???      else
// QDOAS ???       strcpy(newPath,FILES_paths[indexPath].path);
// QDOAS ???     }
// QDOAS ???   }
// QDOAS ???  else if (path!=newPath)
// QDOAS ???   strcpy(newPath,path);

  // Return

  return newPath;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILES_ChangePath
// -----------------------------------------------------------------------------
// PURPOSE       change the path of a file
//
// INPUT         oldPath       the old path
//               useFileName   add the file name to the new path
//
// OUTPUT        newPath       the new path
// -----------------------------------------------------------------------------

void FILES_ChangePath(unsigned char *oldPath,unsigned char *newPath,INT useFileName)
 {
  FILES_RemoveOnePath(oldPath);
  FILES_CompactPath(oldPath,newPath,useFileName,1);
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILES_RetrievePath
// -----------------------------------------------------------------------------
// PURPOSE       For a given file type, change the current directory to the last
//               directory used for this file type of to the one specified in the
//               given file name
//
// INPUT         pathStringLength    maximum length for the output path string
//               fullFileName        the given file name
//               fullFileNameLength  the length of the given file name
//               indexFileType       the file type to focus on
//               changeDefaultPath   1 to update the default path associated to
//                                   the given file type
//
// OUTPUT        pathString          the new path
// -----------------------------------------------------------------------------

void FILES_RetrievePath(unsigned char *pathString,SZ_LEN pathStringLength,
                        unsigned char *fullFileName,SZ_LEN fullFileNameLength,
                        INT    indexFileType,INT changeDefaultPath)
 {
  // Declarations

  SZ_LEN currentLength;
  unsigned char *ptr;

  // Initialization

  currentLength=0;

  // Set path to the default one associated to the type of selected file

  if (pathString!=FILES_types[indexFileType].defaultPath)
   memcpy(pathString,FILES_types[indexFileType].defaultPath,MAX_PATH_LEN+1);

  if ((fullFileName!=NULL) && (fullFileNameLength>0) && (fullFileNameLength<=pathStringLength))
   {
    // Use the selected file path as the default one

    memcpy(pathString,fullFileName,MAX_PATH_LEN+1);

    if ((ptr=strrchr(pathString,PATH_SEP))!=NULL)
     {
      *ptr='\0';
      currentLength=strlen(pathString);
      memset(ptr,0,pathStringLength-currentLength);
     }

    if ((ptr=strrchr(pathString,PATH_SEP))==NULL)
     pathString[currentLength]=PATH_SEP;
   }

  // Set current directory as the default path

  if (!strlen(FILES_RebuildFileName(pathString,pathString,0)))
   getcwd(pathString,sizeof(pathString));

// WIN32-only:   GetCurrentDirectory(sizeof(pathString),pathString);

  if (changeDefaultPath)
   FILES_ChangePath(FILES_types[indexFileType].defaultPath,pathString,0);
 }

// =========
// ALL PATHS
// =========

// -----------------------------------------------------------------------------
// FUNCTION      FilesResetAllPaths
// -----------------------------------------------------------------------------
// PURPOSE       reset all the paths associated to the different files types
// -----------------------------------------------------------------------------

void FilesResetAllPaths(void)
 {
  memset(FILES_paths,0,sizeof(FILES_PATH)*FILES_nPaths);
 }

// -----------------------------------------------------------------------------
// FUNCTION      FilesLoadAllPaths
// -----------------------------------------------------------------------------
// PURPOSE       load all paths from wds configuration file
//
// INPUT         fileLine  the current line in the wds configuration file
// -----------------------------------------------------------------------------

void FilesLoadAllPaths(unsigned char *fileLine)
 {
  // Declarations

  unsigned char path[MAX_PATH_LEN+1];           // path extracted from fileLine
  INDEX indexPath;

  // Initialization

  memset(path,0,MAX_PATH_LEN+1);

  // Line decomposition

  sscanf(fileLine,"%%%d=%[^',']",&indexPath,path);

  // Path safe keeping

  if ((indexPath>=0) && (indexPath<FILES_nPaths))
   memcpy(FILES_paths[indexPath].path,path,MAX_PATH_LEN+1);
 }

// -----------------------------------------------------------------------------
// FUNCTION      FilesSaveAllPaths
// -----------------------------------------------------------------------------
// PURPOSE       all paths safe keeping into the selected wds configuration file
//
// INPUT         fp            pointer to the current wds configuration file;
//               sectionName   name of the all paths section in the configuration file
// -----------------------------------------------------------------------------

void FilesSaveAllPaths(FILE *fp,unsigned char *sectionName)
 {
  // Declaration

  INDEX indexPath;

  // Print section name

  fprintf(fp,"[%s]\n\n",sectionName);

  // Browse types of files

  for (indexPath=0;indexPath<FILES_nPaths;indexPath++)
   if (strlen(FILES_paths[indexPath].path) && FILES_paths[indexPath].count)
    fprintf(fp,"%%%d=%s,%d\n",indexPath,FILES_paths[indexPath].path,FILES_paths[indexPath].count);

  fprintf(fp,"\n");
 }

// =============
// DEFAULT PATHS
// =============

// -----------------------------------------------------------------------------
// FUNCTION      FilesResetDefaultPaths
// -----------------------------------------------------------------------------
// PURPOSE       reset default paths
// -----------------------------------------------------------------------------

void FilesResetDefaultPaths(void)
 {
 	// Declaration

  INDEX indexFileType;

  // Browse the different file types

  for (indexFileType=0;indexFileType<FILE_TYPE_MAX;indexFileType++)
   memset(FILES_types[indexFileType].defaultPath,0,MAX_PATH_LEN+1);
 }

// -----------------------------------------------------------------------------
// FUNCTION      FilesLoadDefaultPaths
// -----------------------------------------------------------------------------
// PURPOSE       load default paths from the wds configuration file
//
// INPUT         fileLine  the current line in the wds configuration file
// -----------------------------------------------------------------------------

void FilesLoadDefaultPaths(unsigned char *fileLine)
 {
  // Declarations

  unsigned char keyName[MAX_ITEM_TEXT_LEN+1],                                           // name on the left of '=' symbol in a wds statement
        defaultPath[MAX_PATH_LEN+1];                                            // default path extracted from fileLine
  INDEX indexFileType;

  // Initialization

  memset(defaultPath,0,MAX_PATH_LEN);

  // Line decomposition

  sscanf(fileLine,"%[^=]=%[^\n]",keyName,defaultPath);

  // Search for keyName in types of files list

  for (indexFileType=0;indexFileType<FILE_TYPE_MAX;indexFileType++)
   if (!STD_Stricmp(FILES_types[indexFileType].fileType,keyName))
    FILES_CompactPath(FILES_types[indexFileType].defaultPath,defaultPath,0,1);
 }

// -----------------------------------------------------------------------------
// FUNCTION      FilesSaveDefaultPaths
// -----------------------------------------------------------------------------
// PURPOSE       default paths safe keeping into the selected wds configuration file
//
// INPUT         fp            pointer to the current wds configuration file;
//               sectionName   name of the default paths section in the configuration file
// -----------------------------------------------------------------------------

void FilesSaveDefaultPaths(FILE *fp,unsigned char *sectionName)
 {
  // Declaration

  INDEX indexFileType;

  // Print section name

  fprintf(fp,"[%s]\n\n",sectionName);

  // Browse types of files

  for (indexFileType=0;indexFileType<FILE_TYPE_MAX;indexFileType++)
   fprintf(fp,"%s=%s\n",FILES_types[indexFileType].fileType,FILES_types[indexFileType].defaultPath);

  fprintf(fp,"\n");
 }

// =====================================
// UTILITIES FOR LOADING DATA FROM FILES
// =====================================

// -----------------------------------------------------------------------------
// FUNCTION      FILES_GetMatrixDimensions (obsolete, to replace by MATRIX_Load)
// -----------------------------------------------------------------------------
// PURPOSE       Retrieve the dimensions of the matrix from a file
//
// INPUT         fp               pointer to the file to load;
//               fileName         the name of the file to load;
//               callingFunction  the name of the calling function;
//               errorType        type of error to return;
//
// OUTPUT        pNl,pNc          the dimension of the matrix to load
//
// RETURN        ERROR_ID_NO in case of success
//               ERROR_ID_ALLOC on allocation error
//               ERROR_ID_FILE_NOT_FOUND if can not open the input file
//               ERROR_ID_FILE_EMPTY if the input file is empty
// -----------------------------------------------------------------------------

RC FILES_GetMatrixDimensions(FILE *fp,unsigned char *fileName,INT *pNl,INT *pNc,unsigned char *callingFunction,INT errorType)
 {
  // Declarations

  unsigned char *oldColumn,*nextColumn;                                                 // line of files
  INT    nl,nc,                                                                 // dimensions of the matrix
         lineLength,                                                            // length of file line
         fileLength;                                                            // total length of file
  double tempValue;                                                             // temporary value
  RC     rc;                                                                    // return code

  // Initializations

  rc=ERROR_ID_NO;
  oldColumn=nextColumn=NULL;
  nl=nc=0;

  if (!(fileLength=STD_FileLength(fp)))
   rc=ERROR_SetLast(callingFunction,errorType,ERROR_ID_FILE_NOT_FOUND,fileName);

  // Allocate buffers for lines of file

  else if (((oldColumn=(unsigned char *)MEMORY_AllocBuffer("FILES_GetMatrixDimensions ","oldColumn",fileLength,sizeof(unsigned char),0,MEMORY_TYPE_STRING))==NULL) ||
           ((nextColumn=(unsigned char *)MEMORY_AllocBuffer("FILES_GetMatrixDimensions ","nextColumn",fileLength,sizeof(unsigned char),0,MEMORY_TYPE_STRING))==NULL))
   rc=ERROR_ID_ALLOC;
  else
   {
    // Go to the first no comment line

    while (fgets(oldColumn,fileLength,fp) && ((strchr(oldColumn,';')!=NULL) || (strchr(oldColumn,'*')!=NULL)));

    // Determine the number of columns

    for (nc=0;strlen(oldColumn);nc++)
     {
      lineLength=strlen(oldColumn);
      oldColumn[lineLength++]='\n';
      oldColumn[lineLength]=0;

      memset(nextColumn,0,fileLength);
      sscanf(oldColumn,"%lf %[^'\n']",(double *)&tempValue,nextColumn);
      strcpy(oldColumn,nextColumn);
     }

    // Determine the number of lines

    for (nl=1;!feof(fp) && fgets(oldColumn,fileLength,fp);)
     if ((strchr(oldColumn,';')==NULL) && (strchr(oldColumn,'*')==NULL))
      nl++;
   }

  if (!nl || !nc)
   rc=ERROR_SetLast(callingFunction,errorType,ERROR_ID_FILE_EMPTY,fileName);

  // Release allocated buffers

  if (oldColumn!=NULL)
   MEMORY_ReleaseBuffer("FILES_GetMatrixDimensions ","oldColumn",oldColumn);
  if (nextColumn!=NULL)
   MEMORY_ReleaseBuffer("FILES_GetMatrixDimensions ","nextColumn",nextColumn);

  // Return

  *pNl=nl;
  *pNc=nc;

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILES_LoadMatrix (obsolete, to replace by MATRIX_Load)
// -----------------------------------------------------------------------------
// PURPOSE       Load a matrix from file
//
// INPUT         fp               pointer to the file to load;
//               fileName         the name of the file to load;
//               base,nl,nc       resp., starting index for lines, number of lines and number of columns
//               errorType        type of error to return;
//
// OUTPUT        matrix           the matrix
//
// RETURN        ERROR_ID_NO in case of success
//               ERROR_ID_ALLOC on allocation error
//               ERROR_ID_FILE_NOT_FOUND if can not open the input file
// -----------------------------------------------------------------------------

RC FILES_LoadMatrix(FILE *fp,unsigned char *fileName,double **matrix,INT base,INT nl,INT nc,unsigned char *callingFunction,INT errorType)
 {
  // Declarations

  unsigned char *oldColumn,*nextColumn;                             // buffers for file lines
  INT    lineLength,                                        // length of a file line
         fileLength;                                        // total length of file
  INDEX  i,j;                                               // index for browsing lines and columns in file
  RC     rc;                                                // return code

  // Initializations

  oldColumn=nextColumn=NULL;
  rc=ERROR_ID_NO;

  // File open

  if (!(fileLength=STD_FileLength(fp)))
   rc=ERROR_SetLast(callingFunction,errorType,ERROR_ID_FILE_NOT_FOUND,fileName);

  // Allocate buffers for lines of file

  else if (((oldColumn=(unsigned char *)MEMORY_AllocBuffer("FILES_LoadMatrix ","oldColumn",fileLength,sizeof(unsigned char),0,MEMORY_TYPE_STRING))==NULL) ||
           ((nextColumn=(unsigned char *)MEMORY_AllocBuffer("FILES_LoadMatrix ","oldColumn",fileLength,sizeof(unsigned char),0,MEMORY_TYPE_STRING))==NULL))
   rc=ERROR_SetLast(callingFunction,errorType,ERROR_ID_ALLOC);
  else
   {
    // File read out

    for (i=base,nl+=base-1;(i<=nl) && fgets(oldColumn,fileLength,fp);)

     if ((strchr(oldColumn,';')==NULL) && (strchr(oldColumn,'*')==NULL))
      {
       for (j=1;j<=nc;j++)
        {
         lineLength=strlen(oldColumn);

         oldColumn[lineLength++]='\n';
         oldColumn[lineLength]=0;

         memset(nextColumn,0,fileLength);
         sscanf(oldColumn,"%lf %[^'\n']",(double *)&matrix[j][i],nextColumn);
         strcpy(oldColumn,nextColumn);
        }

       i++;
      }
   }

  // Release allocated buffers

  if (oldColumn!=NULL)
   MEMORY_ReleaseBuffer("FILES_LoadMatrix ","oldColumn",oldColumn);
  if (nextColumn!=NULL)
   MEMORY_ReleaseBuffer("FILES_LoadMatrix ","nextColumn",nextColumn);

  // Return

  return rc;
 }

// ================================================
// OPEN/SAVE COMMON DIALOG BOX FOR SELECTING A FILE
// ================================================

// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      FilesBuildFilter
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Build a filter string
// QDOAS ??? //
// QDOAS ??? // INPUT         fileType               the of file to load or to save;
// QDOAS ??? //               symbolReferenceNumber  for cross sections, number of times symbols are referenced to
// QDOAS ??? //               indexSymbol            index of the symbol in the list
// QDOAS ??? //
// QDOAS ??? // OUTPUT        fileFilter       the output filter string
// QDOAS ??? //
// QDOAS ??? // RETURN        a pointer to the output filter string
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? unsigned char *FilesBuildFilter(unsigned char *fileFilter,MASK fileType,INT *symbolReferenceNumber,INDEX indexSymbol)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   FILE_TYPE *pFileType,*fileList;                                               // pointer to a file type
// QDOAS ???   unsigned char extension[10];                                                          // extension according to type of file
// QDOAS ???   SYMBOL *crossList,*pCross;                                                    // list of cross sections
// QDOAS ???   INT crossNumber;                                                              // number of cross sections in previous list
// QDOAS ???   SZ_LEN len,extLength;                                                         // string length
// QDOAS ???   INDEX i,j,ideb,ifin;                                                          // indexes for loop and arrays
// QDOAS ???
// QDOAS ???   // Reset buffer
// QDOAS ???
// QDOAS ???   memset(fileFilter,0,MAX_PATH_LEN+1);
// QDOAS ???
// QDOAS ???   // Fill files filter string for cross sections
// QDOAS ???
// QDOAS ???   if (((fileType==FILE_TYPE_CROSS) ||                                           // Cross sections files
// QDOAS ???        (fileType==FILE_TYPE_AMF_SZA) ||                                         // SZA dependent AMF files
// QDOAS ???        (fileType==FILE_TYPE_AMF_CLI) ||                                         // Climatology dependent AMF files
// QDOAS ???        (fileType==FILE_TYPE_AMF_WVE)) &&                                        // Wavelength dependent AMF files
// QDOAS ???
// QDOAS ???      ((crossList=(SYMBOL *)TREE_itemType[TREE_ITEM_TYPE_CROSS_CHILDREN].dataList)!=NULL) &&
// QDOAS ???      ((crossNumber=(INT)TREE_itemType[TREE_ITEM_TYPE_CROSS_CHILDREN].dataNumber)>0) &&
// QDOAS ???       (symbolReferenceNumber!=NULL))
// QDOAS ???    {
// QDOAS ???     sprintf(extension,"_*.%s",FILES_types[fileType].fileExt);
// QDOAS ???     extLength=strlen(extension);
// QDOAS ???
// QDOAS ???     if ((indexSymbol>ITEM_NONE) && (indexSymbol<SYMBOL_PREDEFINED_MAX))
// QDOAS ???      {
// QDOAS ???       pCross=&crossList[indexSymbol];
// QDOAS ???       j=0;
// QDOAS ???
// QDOAS ???       len=strlen(pCross->name);
// QDOAS ???
// QDOAS ???       memcpy(&fileFilter[j],pCross->name,len);                                  // <XS name>
// QDOAS ???       memcpy(&fileFilter[(j+=len)]," Cross Sections (",17);                     // <XS name> Cross Sections (
// QDOAS ???       memcpy(&fileFilter[(j+=17)],pCross->name,len);                            // <XS name> Cross Sections (<XS name>
// QDOAS ???       memcpy(&fileFilter[(j+=len)],extension,extLength);                        // <XS name> Cross Sections (<XS name>_*.<XS ext>
// QDOAS ???       fileFilter[(j+=extLength)]=')';                                           // <XS name> Cross Sections (<XS name>_*.<XS ext>)
// QDOAS ???       fileFilter[(j+=1)]=0;                                                     // <XS name> Cross Sections (<XS name>_*.<XS ext>)\0
// QDOAS ???       memcpy(&fileFilter[(j+=1)],pCross->name,len);                             // <XS name> Cross Sections (<XS name>_*.<XS ext>)\0<XS name>
// QDOAS ???       memcpy(&fileFilter[(j+=len)],extension,extLength);                        // <XS name> Cross Sections (<XS name>_*.<XS ext>)\0<XS name>_*.<XS ext>
// QDOAS ???       fileFilter[(j+=extLength)]=0;                                             // <XS name> Cross Sections (<XS name>_*.<XS ext>)\0<XS name>_*.<XS ext>\0
// QDOAS ???       j++;
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???
// QDOAS ???      for (i=SYMBOL_PREDEFINED_COM,j=0;i<crossNumber;i++)
// QDOAS ???       {
// QDOAS ???        if (((indexSymbol==ITEM_NONE) && !((symbolReferenceNumber[i]>>8)&SYMBOL_ALREADY_USED)) ||
// QDOAS ???            ((indexSymbol==i) && ((fileType!=FILE_TYPE_CROSS) || (((symbolReferenceNumber[i]>>8)&SYMBOL_ALREADY_USED)!=0))))
// QDOAS ???         {
// QDOAS ???          pCross=&crossList[i];
// QDOAS ???
// QDOAS ???          len=strlen(pCross->name);
// QDOAS ???
// QDOAS ???          memcpy(&fileFilter[j],pCross->name,len);                               // <XS name>
// QDOAS ???          memcpy(&fileFilter[(j+=len)]," Cross Sections (",17);                  // <XS name> Cross Sections (
// QDOAS ???          memcpy(&fileFilter[(j+=17)],pCross->name,len);                         // <XS name> Cross Sections (<XS name>
// QDOAS ???          memcpy(&fileFilter[(j+=len)],extension,extLength);                     // <XS name> Cross Sections (<XS name>_*.<XS ext>
// QDOAS ???          fileFilter[(j+=extLength)]=')';                                        // <XS name> Cross Sections (<XS name>_*.<XS ext>)
// QDOAS ???          fileFilter[(j+=1)]=0;                                                  // <XS name> Cross Sections (<XS name>_*.<XS ext>)\0
// QDOAS ???          memcpy(&fileFilter[(j+=1)],pCross->name,len);                          // <XS name> Cross Sections (<XS name>_*.<XS ext>)\0<XS name>
// QDOAS ???          memcpy(&fileFilter[(j+=len)],extension,extLength);                     // <XS name> Cross Sections (<XS name>_*.<XS ext>)\0<XS name>_*.<XS ext>
// QDOAS ???          fileFilter[(j+=extLength)]=0;                                          // <XS name> Cross Sections (<XS name>_*.<XS ext>)\0<XS name>_*.<XS ext>\0
// QDOAS ???          j++;
// QDOAS ???
// QDOAS ???          if (indexSymbol==i)
// QDOAS ???           break;
// QDOAS ???         }
// QDOAS ???       }
// QDOAS ???    }
// QDOAS ???   else
// QDOAS ???
// QDOAS ???   // Fill files filter string for any other type of files
// QDOAS ???
// QDOAS ???    {
// QDOAS ???     if (fileType==FILE_TYPE_ASCII_SPECTRA)
// QDOAS ???      {
// QDOAS ???       fileList=FILES_typeSpectra;
// QDOAS ???       ideb=0;
// QDOAS ???       ifin=FILE_TYPE_SPECTRA_MAX;
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???       fileList=FILES_types;
// QDOAS ???       ideb=fileType;
// QDOAS ???       ifin=ideb+1;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     for (i=ideb,j=0;i<ifin;i++)
// QDOAS ???      {
// QDOAS ???       pFileType=&fileList[i];
// QDOAS ???
// QDOAS ???       memcpy(&fileFilter[j],pFileType->fileType,(len=strlen(pFileType->fileType)));      // <file type>
// QDOAS ???       memcpy(&fileFilter[(j+=len)]," (*.",4);                                            // <file type> (*.
// QDOAS ???       memcpy(&fileFilter[(j+=4)],pFileType->fileExt,(len=strlen(pFileType->fileExt)));   // <file type> (*.<file ext>
// QDOAS ???       fileFilter[(j+=len)]=')';                                                          // <file type> (*.<file ext>)
// QDOAS ???       fileFilter[(j+=1)]=0;                                                              // <file type> (*.<file ext>)\0
// QDOAS ???       memcpy(&fileFilter[(j+=1)],"*.",2);                                                // <file type> (*.<file ext>)\0*.
// QDOAS ???       memcpy(&fileFilter[(j+=2)],pFileType->fileExt,len);                                // <file type> (*.<file ext>)\0*.<file ext>
// QDOAS ???       fileFilter[(j+=len)]=0;                                                            // <file type> (*.<file ext>)\0*.<file ext>\0
// QDOAS ???       j++;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if (fileType!=FILE_TYPE_ASCII_SPECTRA)
// QDOAS ???    {
// QDOAS ???     strcpy(&fileFilter[j],"All files (*.*)");
// QDOAS ???     strcpy(&fileFilter[j+16],"*.*");
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return fileFilter;
// QDOAS ???  }

// QDOAS ??? #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???
// QDOAS ??? LRESULT CALLBACK  FILES_OFNHookProc(HWND hdlg,                                  // handle to child dialog box
// QDOAS ???                                     UINT uiMsg,                                 // message identifier
// QDOAS ???                                     WPARAM wParam,                              // message parameter
// QDOAS ???                                     LPARAM lParam)                              // message parameter
// QDOAS ???  {
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      FILES_Open
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Load and display "Open/Save" windows common dialog box
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndParent               handle of parent
// QDOAS ??? //               fileType                 type of files selected
// QDOAS ??? //               bufferSize               size of the output buffer
// QDOAS ??? //               openStyles               extra styles for dialog box creation
// QDOAS ??? //               openMode                 common dialog open mode
// QDOAS ??? //               symbolReferenceNumber    for cross sections, number of times symbols are referenced to
// QDOAS ??? //               indexSymbol              index of a specified symbol
// QDOAS ??? //               helpID                   id of the help
// QDOAS ??? //
// QDOAS ??? // OUTPUT        buffer                   buffer for receiving selected files
// QDOAS ??? //               pFileType                the selected file type
// QDOAS ??? //
// QDOAS ??? // RETURN        the windows return code
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC FILES_Open(HWND hwndParent,                                                  // handle of parent
// QDOAS ???               MASK fileType,                                                    // type of files selected
// QDOAS ???               unsigned char *buffer,                                                    // buffer for receiving selected files
// QDOAS ???               INT bufferSize,                                                   // size of previous buffer
// QDOAS ???               long openStyles,                                                  // extra styles for dialog box creation
// QDOAS ???               unsigned char openMode,                                                   // common dialog open mode
// QDOAS ???               INT *symbolReferenceNumber,                                       // for cross sections, number of times symbols are referenced to
// QDOAS ???               INDEX indexSymbol,                                                // index of a specified symbol
// QDOAS ???               INT *pFileType,
// QDOAS ???               INT helpID)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   OPENFILENAME ofn = {0};                                                       // common dialog box structure
// QDOAS ??? //  BROWSEINFO bfn={0};
// QDOAS ???
// QDOAS ???   unsigned char fileDir[MAX_PATH_LEN+1],                                                // directory where to search files first
// QDOAS ???         fileTitle[MAX_ITEM_TEXT_LEN+1],                                         // title of the common dialog box
// QDOAS ???         fileName[MAX_PATH_LEN+1],                                               // the selected file(s) without path
// QDOAS ???         oldFileName[MAX_PATH_LEN+1],
// QDOAS ???        *fileFilter;                                                             // filter string
// QDOAS ???   SZ_LEN filterLength;
// QDOAS ???   RC    rc;                                                                     // return code
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   sprintf(fileTitle,"%s %s",
// QDOAS ???          (openMode==FILE_MODE_OPEN)?"Open":"Save",
// QDOAS ???           FILES_types[fileType].fileType);                                      // title of common dialog box
// QDOAS ???
// QDOAS ???   strcpy(oldFileName,buffer);
// QDOAS ???   FILES_RebuildFileName(fileName,buffer,1);
// QDOAS ???   memcpy(buffer,fileName,MAX_PATH_LEN+1);
// QDOAS ???
// QDOAS ???   filterLength=(MAX_ITEM_NAME_LEN*3+80)*
// QDOAS ???               ((FILE_TYPE_MAX<TREE_itemType[TREE_ITEM_TYPE_CROSS_CHILDREN].dataNumber)?
// QDOAS ???                 TREE_itemType[TREE_ITEM_TYPE_CROSS_CHILDREN].dataNumber+1:FILE_TYPE_MAX+1);
// QDOAS ???
// QDOAS ???   fileFilter=(unsigned char *)MEMORY_AllocBuffer("FILES_Open ","fileFilter",filterLength,1,0,MEMORY_TYPE_STRING);
// QDOAS ???   memset(fileFilter,0,filterLength);
// QDOAS ???
// QDOAS ???   FILES_RetrievePath(fileDir,MAX_PATH_LEN+1,buffer,strlen(buffer),fileType,0);
// QDOAS ???
// QDOAS ???   // Fill the OPENFILENAME structure.
// QDOAS ???
// QDOAS ??? //  bfn.hwndOwner=hwndParent;
// QDOAS ??? //  bfn.pidlRoot=NULL;
// QDOAS ??? //  bfn.pszDisplayName=buffer;
// QDOAS ??? //  bfn.lpszTitle=fileName;
// QDOAS ??? //  bfn.ulFlags=BIF_RETURNONLYFSDIRS;
// QDOAS ??? //  bfn.lpfn=FILES_OFNHookProc;
// QDOAS ??? //  bfn.lParam=0;
// QDOAS ??? //  bfn.iImage=0;
// QDOAS ???
// QDOAS ???
// QDOAS ???   ofn.lStructSize = sizeof(OPENFILENAME);                                       // size of structure
// QDOAS ???   ofn.hwndOwner = hwndParent;                                                   // owner of common dialog box
// QDOAS ???
// QDOAS ???   ofn.lpstrFilter = FilesBuildFilter(fileFilter,                                // filter for files to display
// QDOAS ???                                      fileType,
// QDOAS ???                                      symbolReferenceNumber,
// QDOAS ???                                      indexSymbol);
// QDOAS ???
// QDOAS ???   ofn.nFilterIndex = 1;                                                         // initial selected type of files
// QDOAS ???   ofn.lpstrFile = buffer;                                                       // file name edit control initialization; this buffer is used to return the complete selected files names and path
// QDOAS ???   ofn.nMaxFile = bufferSize;                                                    // size of previous buffer
// QDOAS ???   ofn.lpstrFileTitle = fileName;                                                // buffer that receives the selected file(s) without path information
// QDOAS ???   ofn.nMaxFileTitle = sizeof(fileName);                                         // size of previous buffer
// QDOAS ???   ofn.lpstrTitle = fileTitle;                                                   // title of the common dialog box used
// QDOAS ???   ofn.lpstrInitialDir = fileDir;                                                // directory where to start search
// QDOAS ???   ofn.lpfnHook = NULL;
// QDOAS ???
// QDOAS ???   if (openMode==FILE_MODE_OPEN)
// QDOAS ???
// QDOAS ???    ofn.Flags = OFN_PATHMUSTEXIST |                                              // only valid path accepted
// QDOAS ???                OFN_EXPLORER |                                                   // use explorer-like style
// QDOAS ???                OFN_HIDEREADONLY |                                               // hide the read-only check box
// QDOAS ???                OFN_FILEMUSTEXIST |                                              // only valid files accepted
// QDOAS ???                OFN_NOCHANGEDIR |                                                // restore the current directory to its original value
// QDOAS ???                openStyles;                                                      // user extra styles
// QDOAS ???
// QDOAS ???   else
// QDOAS ???
// QDOAS ???    ofn.Flags = OFN_PATHMUSTEXIST |                                              // only valid path accepted
// QDOAS ???                OFN_EXPLORER |                                                   // use explorer-like style
// QDOAS ???                OFN_HIDEREADONLY |                                               // hide the read-only check box
// QDOAS ???                OFN_NOCHANGEDIR |                                                // restore the current directory to its original value
// QDOAS ???                OFN_OVERWRITEPROMPT;                                             // user must confirm whether to overwrite an existing file
// QDOAS ???
// QDOAS ???   if (helpID!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     ofn.Flags|=OFN_SHOWHELP|OFN_ENABLEHOOK;                                     // show button help
// QDOAS ???     ofn.lpfnHook = (LPOFNHOOKPROC)FILES_OFNHookProc;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Display the Open dialog box
// QDOAS ???
// QDOAS ???   if ((rc=((openMode==FILE_MODE_OPEN)? /* SHBrowseForFolder(&bfn) */ GetOpenFileName(&ofn)  :GetSaveFileName(&ofn)))!=0)
// QDOAS ???    FILES_RetrievePath(fileDir,MAX_PATH_LEN+1,buffer,strlen(buffer),fileType,1);
// QDOAS ???
// QDOAS ???   MEMORY_ReleaseBuffer("FILES_Open ","fileFilter",fileFilter);
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   if (pFileType!=NULL)
// QDOAS ???    *pFileType=ofn.nFilterIndex-1;
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif
// QDOAS ???
// -----------------------------------------------------------------------------
// FUNCTION      FILES_BuildFileName
// -----------------------------------------------------------------------------
// PURPOSE       Build a file name from a given name and extension
//
// INPUT/OUTPUT  fileName  the original file name
// INPUT         fileType  the type of file to build
//
// OUTPUT        pointer to the updated file name
// -----------------------------------------------------------------------------

unsigned char *FILES_BuildFileName(unsigned char *fileName,MASK fileType)
 {
  // Declarations

  SZ_LEN fileNameLength;
  unsigned char *ptr;

  // Replace file extension by the correct one

  if (((fileNameLength=strlen(fileName))!=0) && (fileType>FILE_TYPE_ALL))
   {
    if ((ptr=strrchr(fileName,'.'))==NULL)
     ptr=&fileName[fileNameLength];

    sprintf(ptr,".%s",FILES_types[fileType].fileExt);
   }

  // Return

  return fileName;
 }
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      FILES_Select
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Select a file of a given type
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndParent               handle of parent
// QDOAS ??? //               bufferSize               size of the output buffer
// QDOAS ??? //               fileType                 type of files selected
// QDOAS ??? //               style                    extra styles for the dialog box
// QDOAS ??? //               openMode                 common dialog open mode
// QDOAS ??? //               symbolReferenceNumber    for cross sections, number of times symbols are referenced to
// QDOAS ??? //               indexSymbol              index of a specified symbol
// QDOAS ??? //               helpID                   id of the help
// QDOAS ??? //
// QDOAS ??? // OUTPUT        buffer                   buffer for receiving selected files
// QDOAS ??? //
// QDOAS ??? // RETURN        the windows return code
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC FILES_Select(HWND hwndParent,                                                // handle of parent
// QDOAS ???                 unsigned char *buffer,                                                  // buffer for receiving file path and name
// QDOAS ???                 INT bufferSize,                                                 // size of previous buffer
// QDOAS ???                 MASK fileType,                                                  // type of files to select
// QDOAS ???                 INT style,                                                      // extra style
// QDOAS ???                 unsigned char openMode,                                                 // common dialog open mode
// QDOAS ???                 INT *symbolReferenceNumber,                                     // number of times symbols associated to files are referenced to
// QDOAS ???                 INDEX indexSymbol,                                              // index of the symbol associated to the selected file in symbols list
// QDOAS ???                 INT helpId)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   unsigned char *filesSelected;                                                         // pointers to buffers with selected file(s) and path
// QDOAS ???   RC rc;                                                                        // return code
// QDOAS ???
// QDOAS ???   // Buffer allocation
// QDOAS ???
// QDOAS ???   filesSelected=(unsigned char *)MEMORY_AllocBuffer("FILES_Select ","filesSelected",bufferSize,1,0,MEMORY_TYPE_STRING);
// QDOAS ???   memcpy(filesSelected,buffer,bufferSize);
// QDOAS ???
// QDOAS ???   // Display and process dialog panel
// QDOAS ???
// QDOAS ???   if ((rc=FILES_Open(hwndParent,                                                // handle of parent
// QDOAS ???                      fileType,                                                  // requested type of files
// QDOAS ???                      filesSelected,                                             // selected files
// QDOAS ???                      bufferSize,                                                // size of previous buffer
// QDOAS ???                      style,                                                     // no extra styles
// QDOAS ???                      openMode,                                                  // open mode for common dialog box
// QDOAS ???                      symbolReferenceNumber,                                     // number of times symbols associated to files are referenced to
// QDOAS ???                      indexSymbol,NULL,helpId))!=0)                              // index of the symbol associated to the selected file in symbols list
// QDOAS ???
// QDOAS ???    memcpy(buffer,filesSelected,bufferSize);
// QDOAS ???
// QDOAS ???   // Free buffer and return
// QDOAS ???
// QDOAS ???   MEMORY_ReleaseBuffer("FILES_Select ","filesSelected",filesSelected);
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      FILES_Insert
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_INSERT_FILES context menu command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree                 handle of project tree
// QDOAS ??? //               indexParent              the index in the tree of the parent node
// QDOAS ??? //               dataType                 the data type of the new item to insert
// QDOAS ??? //               fileType                 type of files to insert
// QDOAS ??? //
// QDOAS ??? // RETURN        the index in the tree of the new item
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INDEX FILES_Insert(HWND hwndTree,INDEX indexParent,INT dataType,MASK fileType)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   unsigned char *filesSelected,fileName[MAX_PATH_LEN+1],                                // pointers to buffers with selected file(s) and path
// QDOAS ???         *ptr,*oldPtr;                                                           // pointers used for files name extraction
// QDOAS ???   INT fileNumber;                                                               // number of retrieved files
// QDOAS ???   SZ_LEN lenPath;                                                               // length of path
// QDOAS ???   INDEX indexItem;
// QDOAS ???   INDEX indexProject;
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   indexItem=ITEM_NONE;
// QDOAS ???
// QDOAS ???   // Buffer allocation
// QDOAS ???
// QDOAS ???   filesSelected=(unsigned char *)MEMORY_AllocBuffer("FILES_Insert ","filesSelected",BUFFER_SIZE,1,0,MEMORY_TYPE_STRING);
// QDOAS ???   memset(filesSelected,0,BUFFER_SIZE);
// QDOAS ???
// QDOAS ???   // Display and process dialog panel
// QDOAS ???
// QDOAS ???   if (((indexProject=TREE_GetProjectParent(indexParent))!=ITEM_NONE) &&
// QDOAS ???        (strlen(PRJCT_itemList[indexProject].instrumental.fileExt)<12))
// QDOAS ???    strcpy(FILES_types[FILE_TYPE_SPECTRA].fileExt,PRJCT_itemList[indexProject].instrumental.fileExt);
// QDOAS ???
// QDOAS ???   FILES_Select(hwndTree,filesSelected,BUFFER_SIZE,fileType,(long)OFN_ALLOWMULTISELECT,FILE_MODE_OPEN,NULL,ITEM_NONE,ITEM_NONE);
// QDOAS ???
// QDOAS ???   if ((ptr=strchr(filesSelected,0))!=filesSelected)
// QDOAS ???    {
// QDOAS ???     // Retrieve path
// QDOAS ???
// QDOAS ???     lenPath=strlen(filesSelected);
// QDOAS ???     if (filesSelected[lenPath-1]==PATH_SEP)   // root path
// QDOAS ???      filesSelected[lenPath-1]=0;
// QDOAS ???
// QDOAS ???     oldPtr=++ptr;
// QDOAS ???     fileNumber=0;
// QDOAS ???
// QDOAS ???     // Retrieve selected files and insert them in list
// QDOAS ???
// QDOAS ???     while ((ptr=strchr(ptr,0))!=oldPtr)
// QDOAS ???      {
// QDOAS ???       sprintf(fileName,"%s%c%s",filesSelected,PATH_SEP,oldPtr);
// QDOAS ???
// QDOAS ???       RAW_UpdateWindoasPath((indexItem=TREE_InsertOneItem(hwndTree,fileName,indexParent,dataType,0,0,0)),indexParent,0);
// QDOAS ???
// QDOAS ???       fileNumber++;
// QDOAS ???       oldPtr=++ptr;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (lenPath && !fileNumber)
// QDOAS ???      RAW_UpdateWindoasPath((indexItem=TREE_InsertOneItem(hwndTree,filesSelected,indexParent,dataType,0,0,0)),indexParent,0);
// QDOAS ???
// QDOAS ???     if (fileNumber)
// QDOAS ???      FILES_ChangePath(FILES_types[fileType].defaultPath,filesSelected,0);
// QDOAS ???
// QDOAS ???     if (!TLBAR_bSaveFlag)
// QDOAS ???      TLBAR_Enable(TRUE);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Free buffers
// QDOAS ???
// QDOAS ???   MEMORY_ReleaseBuffer("FILES_Insert ","filesSelected",filesSelected);
// QDOAS ???
// QDOAS ???   return indexItem;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // =============================
// QDOAS ??? // CONFIGURATION FILE MANAGEMENT
// QDOAS ??? // =============================
// QDOAS ???
// QDOAS ??? enum _configurationSection
// QDOAS ???  {
// QDOAS ???   FILES_SECTION_VERSION,
// QDOAS ???   FILES_SECTION_ALL_PATHS,
// QDOAS ???   FILES_SECTION_DEFAULT_PATHS,
// QDOAS ???   FILES_SECTION_CHILD,
// QDOAS ???   FILES_SECTION_SITES,
// QDOAS ???   FILES_SECTION_SYMBOLS,
// QDOAS ???   FILES_SECTION_RAW,
// QDOAS ???   FILES_SECTION_PROJECTS,
// QDOAS ???   FILES_SECTION_ANLYS,
// QDOAS ???   FILES_SECTION_TOOLS,
// QDOAS ???   FILES_SECTION_MAX
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? unsigned char *FILES_configurationSectionNames[FILES_SECTION_MAX]=
// QDOAS ???  {
// QDOAS ???   "Version",
// QDOAS ???   "All paths",
// QDOAS ???   "Default paths",
// QDOAS ???   "MDI child windows",
// QDOAS ???   "Observation sites",
// QDOAS ???   "Cross sections",
// QDOAS ???   "Raw spectra names",
// QDOAS ???   "Projects",
// QDOAS ???   "Analysis windows",
// QDOAS ???   "Tools"
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        FilesResetConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Dispatch the reset command to all data structures
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void FilesResetConfiguration(void)
// QDOAS ???  {
// QDOAS ???   FILES_version=HELP_VERSION_MAX;
// QDOAS ???
// QDOAS ???   FilesResetAllPaths();                                                         // default paths
// QDOAS ???   FilesResetDefaultPaths();                                                     // default paths
// QDOAS ???   CHILD_ResetConfiguration();                                                   // MDI child windows size and position
// QDOAS ???   SITES_ResetConfiguration();                                                   // observation sites
// QDOAS ???   SYMB_ResetConfiguration();                                                    // symbols
// QDOAS ???   PRJCT_ResetConfiguration();                                                   // projects
// QDOAS ???   ANLYS_ResetConfiguration();                                                   // analysis windows
// QDOAS ???   XSCONV_ResetConfiguration();                                                  // convolution tool
// QDOAS ???   RING_ResetConfiguration();                                                    // ring tool
// QDOAS ???   USAMP_ResetConfiguration();                                                   // undersampling tool
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        FILES_LoadConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Load WINDOAS settings from configuration file
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndParent  handle of the parent
// QDOAS ??? //                 fileName    the name of the configuration file
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void FILES_LoadConfiguration(HWND hwndParent,unsigned char *fileName)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   unsigned char fileLine[BUFFER_SIZE+1],
// QDOAS ???         sectionName[MAX_ITEM_TEXT_LEN+1],
// QDOAS ???        *ptr;
// QDOAS ???   INDEX indexSection;
// QDOAS ???   FILE *fp;
// QDOAS ???
// QDOAS ???   // Reset previous configuration
// QDOAS ???
// QDOAS ???   FilesResetConfiguration();
// QDOAS ???   #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???   TLBAR_Enable(FALSE);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   strcpy(DOAS_logFile,fileName);
// QDOAS ???
// QDOAS ???   if ((ptr=strrchr(DOAS_logFile,PATH_SEP))!=NULL)
// QDOAS ???    strcpy(ptr+1,WINDOAS_LOG);
// QDOAS ???   else
// QDOAS ???    sprintf(DOAS_logFile,".%c%s",PATH_SEP,WINDOAS_LOG);
// QDOAS ???
// QDOAS ???   strcpy(DOAS_dbgFile,fileName);
// QDOAS ???
// QDOAS ???   if ((ptr=strrchr(DOAS_dbgFile,PATH_SEP))!=NULL)
// QDOAS ???    strcpy(ptr+1,WINDOAS_DBG);
// QDOAS ???   else
// QDOAS ???    sprintf(DOAS_dbgFile,".%c%s",PATH_SEP,WINDOAS_DBG);
// QDOAS ???
// QDOAS ???   strcpy(DOAS_tmpFile,fileName);
// QDOAS ???
// QDOAS ???   if ((ptr=strrchr(DOAS_tmpFile,PATH_SEP))!=NULL)
// QDOAS ???    strcpy(ptr+1,WINDOAS_TMP);
// QDOAS ???   else
// QDOAS ???    sprintf(DOAS_tmpFile,".%c%s",PATH_SEP,WINDOAS_TMP);
// QDOAS ???
// QDOAS ???   strcpy(DOAS_broAmfFile,fileName);
// QDOAS ???
// QDOAS ???   if ((ptr=strrchr(DOAS_broAmfFile,PATH_SEP))!=NULL)
// QDOAS ???    strcpy(ptr+1,BRO_AMF_FILE);
// QDOAS ???   else
// QDOAS ???    sprintf(DOAS_broAmfFile,".%c%s",PATH_SEP,BRO_AMF_FILE);
// QDOAS ???
// QDOAS ???   // Open file name
// QDOAS ???
// QDOAS ???   if ((fp=fopen(fileName,"rt"))!=NULL)
// QDOAS ???    {
// QDOAS ???     strncpy(FILES_configuration,fileName,MAX_PATH_LEN);
// QDOAS ???     FILES_configuration[MAX_PATH_LEN]=0;
// QDOAS ???
// QDOAS ???     // File lines read out
// QDOAS ???
// QDOAS ???     for (indexSection=FILES_SECTION_MAX;!feof(fp) && fgets(fileLine,MAX_ITEM_TEXT_LEN,fp);)
// QDOAS ???      {
// QDOAS ???       // New section
// QDOAS ???
// QDOAS ???       if (fileLine[0]=='[')
// QDOAS ???        {
// QDOAS ???         for (sscanf(fileLine+1,"%[^]\n]",sectionName),
// QDOAS ???              indexSection=0;indexSection<FILES_SECTION_MAX;indexSection++)
// QDOAS ???
// QDOAS ???          if (!STD_Stricmp(sectionName,FILES_configurationSectionNames[indexSection]))
// QDOAS ???           break;
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Dispatch file lines to configuration loading routines
// QDOAS ???
// QDOAS ???       else if ((fileLine[0]!=(unsigned char)0x0D) && (fileLine[0]!=(unsigned char)0x0A))
// QDOAS ???
// QDOAS ???        switch (indexSection)
// QDOAS ???         {
// QDOAS ???       // ----------------------------------------------------------------------
// QDOAS ???          case FILES_SECTION_VERSION :
// QDOAS ???             FilesLoadProgramVersion(fileLine);                   // program version
// QDOAS ???          break;
// QDOAS ???       // ----------------------------------------------------------------------
// QDOAS ???          case FILES_SECTION_ALL_PATHS :
// QDOAS ???             FilesLoadAllPaths(fileLine);                         // all paths
// QDOAS ???          break;
// QDOAS ???       // ----------------------------------------------------------------------
// QDOAS ???          case FILES_SECTION_DEFAULT_PATHS :
// QDOAS ???             FilesLoadDefaultPaths(fileLine);                     // default paths
// QDOAS ???          break;
// QDOAS ???       // ----------------------------------------------------------------------
// QDOAS ???          case FILES_SECTION_CHILD :
// QDOAS ???             CHILD_LoadConfiguration(fileLine);                   // MDI child windows size and position
// QDOAS ???          break;
// QDOAS ???       // ----------------------------------------------------------------------
// QDOAS ???          case FILES_SECTION_SITES :
// QDOAS ???             SITES_LoadConfiguration(fileLine);                   // observation sites
// QDOAS ???          break;
// QDOAS ???       // ----------------------------------------------------------------------
// QDOAS ???          case FILES_SECTION_SYMBOLS :
// QDOAS ???             SYMB_LoadConfiguration(fileLine);                    // cross sections name and description
// QDOAS ???          break;
// QDOAS ???       // ----------------------------------------------------------------------
// QDOAS ???          case FILES_SECTION_PROJECTS :
// QDOAS ???           PRJCT_LoadConfiguration(fileLine);                     // projects properties
// QDOAS ???          break;
// QDOAS ???       // ----------------------------------------------------------------------
// QDOAS ???          case FILES_SECTION_ANLYS :
// QDOAS ???           if (FILES_version>HELP_VERSION_1_04)
// QDOAS ???            ANLYS_LoadConfiguration(fileLine);                    // analysis windows properties
// QDOAS ???          break;
// QDOAS ???       // ----------------------------------------------------------------------
// QDOAS ???          case FILES_SECTION_RAW :
// QDOAS ???           if (!strlen(DOAS_arg.wdsFile) ||  !strlen(DOAS_arg.fileName))
// QDOAS ???            RAW_LoadConfiguration(fileLine);                      // raw spectra files
// QDOAS ???          break;
// QDOAS ???       // ----------------------------------------------------------------------
// QDOAS ???          case FILES_SECTION_TOOLS :
// QDOAS ???
// QDOAS ???           if (!XSCONV_LoadConfiguration(fileLine) &&             // Convolution tool
// QDOAS ???               !RING_LoadConfiguration(fileLine))                 // Ring tool
// QDOAS ???
// QDOAS ???            USAMP_LoadConfiguration(fileLine);                    // Undersampling tool
// QDOAS ???
// QDOAS ???          break;
// QDOAS ???       // ----------------------------------------------------------------------
// QDOAS ???          default :
// QDOAS ???          break;
// QDOAS ???       // ----------------------------------------------------------------------
// QDOAS ???         }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if ((strlen(DOAS_arg.wdsFile)>0) && (strlen(DOAS_arg.fileName)>0))
// QDOAS ???      PRJCT_LoadFile(DOAS_arg.fileName);
// QDOAS ???
// QDOAS ???     fclose(fp);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     CHILD_Open(MENU_WINDOWS_CHILD_ENVIRONMENT,1);
// QDOAS ???     CHILD_Open(MENU_WINDOWS_CHILD_PROJECT,1);
// QDOAS ???    }
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   #if defined(__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ???   if (FILES_version<=HELP_VERSION_1_04)
// QDOAS ???    ANLYS_LoadConfigurationOld(fileName);
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   FILES_version=HELP_VERSION_MAX-1;
// QDOAS ???
// QDOAS ???   #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???   if (hwndParent!=NULL)
// QDOAS ???    memset(&DOAS_arg,0,sizeof(DOAS_ARG));
// QDOAS ???   #endif
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        FilesSaveConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Save WinDOAS settings in the configuration file
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndParent  handle of the parent
// QDOAS ??? //                 fileName    the name of the configuration file
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void FilesSaveConfiguration(HWND hwndParent,unsigned char *fileName)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   HCURSOR hcurSave,hcursHourGlass;
// QDOAS ???   FILE *fp;
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   fp=NULL;
// QDOAS ???
// QDOAS ???   if (!strlen(fileName) || ((fp=fopen(fileName,"w+t"))==NULL))
// QDOAS ???    MSG_MessageBox(hwndParent,ITEM_NONE,IDS_APP_TITLE,ERROR_ID_FILE_OPEN,MB_OK|MB_ICONHAND,fileName);
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     hcursHourGlass=LoadCursor(NULL,IDC_WAIT);
// QDOAS ???     hcurSave=SetCursor(hcursHourGlass);
// QDOAS ???
// QDOAS ???     FilesSaveProgramVersion(fp,FILES_configurationSectionNames[FILES_SECTION_VERSION]);         // current program version
// QDOAS ???     CHILD_SaveConfiguration(fp,FILES_configurationSectionNames[FILES_SECTION_CHILD]);           // MDI child windows size and position
// QDOAS ???     FilesSaveAllPaths(fp,FILES_configurationSectionNames[FILES_SECTION_ALL_PATHS]);             // all paths
// QDOAS ???     FilesSaveDefaultPaths(fp,FILES_configurationSectionNames[FILES_SECTION_DEFAULT_PATHS]);     // default paths
// QDOAS ???     SITES_SaveConfiguration(fp,FILES_configurationSectionNames[FILES_SECTION_SITES]);           // observation sites longitude and latitude
// QDOAS ???     SYMB_SaveConfiguration(fp,FILES_configurationSectionNames[FILES_SECTION_SYMBOLS]);          // all symbols (XS and instrumental correction) name and description
// QDOAS ???     PRJCT_SaveConfiguration(fp,FILES_configurationSectionNames[FILES_SECTION_PROJECTS]);        // projects description
// QDOAS ???     ANLYS_SaveConfiguration(fp,FILES_configurationSectionNames[FILES_SECTION_ANLYS]);           // analysis windows
// QDOAS ???     RAW_SaveConfiguration(fp,FILES_configurationSectionNames[FILES_SECTION_RAW]);               // raw spectra files
// QDOAS ???
// QDOAS ???     fprintf(fp,"[%s]\n\n",FILES_configurationSectionNames[FILES_SECTION_TOOLS]);
// QDOAS ???
// QDOAS ???     XSCONV_SaveConfiguration(fp);                                                               // cross sections convolution
// QDOAS ???     RING_SaveConfiguration(fp);                                                                 // ring effect
// QDOAS ???     USAMP_SaveConfiguration(fp);                                                                // undersampling effect
// QDOAS ???
// QDOAS ???     fprintf(fp,"\n");
// QDOAS ???
// QDOAS ???     // Close file
// QDOAS ???
// QDOAS ???     if (fp!=NULL)
// QDOAS ???      fclose(fp);
// QDOAS ???
// QDOAS ???     TLBAR_Enable(FALSE);
// QDOAS ???
// QDOAS ???     SetCursor(hcurSave);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ================================
// QDOAS ??? // "FILES" MENU COMMANDS PROCESSING
// QDOAS ??? // ================================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      FILES_EnableSaveOption
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Enable/Disable "Save" menu option
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndParent    handle of the parent application
// QDOAS ??? //               enableFlag    flag to enable the save menu option
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void FILES_EnableSaveOption(HWND hwndParent,BOOL enableFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   unsigned char titleBar[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   HMENU hMenu,hSubMenu;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   hMenu=GetMenu(hwndParent);
// QDOAS ???   hSubMenu=GetSubMenu(hMenu,0);
// QDOAS ???
// QDOAS ???   // Enable "save" menu option
// QDOAS ???
// QDOAS ???   if (enableFlag)
// QDOAS ???    EnableMenuItem(hSubMenu,MENU_FILE_SAVE,MF_ENABLED);
// QDOAS ???
// QDOAS ???   // Disable "save" menu option
// QDOAS ???
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     EnableMenuItem(hSubMenu,MENU_FILE_SAVE,MF_GRAYED);
// QDOAS ???     memset(FILES_configuration,0,MAX_PATH_LEN+1);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if (strlen(FILES_configuration))
// QDOAS ???    sprintf(titleBar,"%s - %s",DOAS_szTitle,FILES_configuration);
// QDOAS ???   else
// QDOAS ???    sprintf(titleBar,"%s",DOAS_szTitle);
// QDOAS ???
// QDOAS ???   SetWindowText(hwndParent,titleBar);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      FILES_MenuNew
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Process MENU_FILE_NEW menu command
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndParent    handle of the parent application
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void FILES_MenuNew(HWND hwndParent)
// QDOAS ???  {
// QDOAS ???   if (TLBAR_Check(hwndParent)!=IDCANCEL)
// QDOAS ???    {
// QDOAS ???     FILES_RebuildFileName(FILES_configuration,FILES_configuration,1);
// QDOAS ???     FILES_EnableSaveOption(hwndParent,FALSE);
// QDOAS ???     FILES_LoadConfiguration(hwndParent,FILES_configuration);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      FILES_MenuOpen
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Process MENU_FILE_OPEN menu command
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndParent    handle of the parent application
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void FILES_MenuOpen(HWND hwndParent)
// QDOAS ???  {
// QDOAS ???   if ((TLBAR_Check(hwndParent)!=IDCANCEL) &&
// QDOAS ???        FILES_Select(hwndParent,FILES_configuration,MAX_PATH_LEN+1,FILE_TYPE_INI,0,FILE_MODE_OPEN,NULL,ITEM_NONE,ITEM_NONE))
// QDOAS ???    {
// QDOAS ???     FILES_RebuildFileName(FILES_configuration,FILES_configuration,1);
// QDOAS ???     FILES_LoadConfiguration(hwndParent,FILES_configuration);
// QDOAS ???     FILES_EnableSaveOption(hwndParent,(strlen(FILES_configuration)!=0)?TRUE:FALSE);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      FILES_MenuSave
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Process MENU_FILE_SAVE menu command
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndParent    handle of the parent application
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC FILES_MenuSave(HWND hwndParent)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   rc=1;
// QDOAS ???
// QDOAS ???   if (strlen(FILES_configuration))
// QDOAS ???    FilesSaveConfiguration(hwndParent,FILES_configuration);                      // settings safe keeping
// QDOAS ???   else
// QDOAS ???    rc=FILES_MenuSaveAs(hwndParent);
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      FILES_MenuSaveAs
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Process MENU_FILE_SAVE_AS menu command
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndParent    handle of the parent application
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC FILES_MenuSaveAs(HWND hwndParent)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   unsigned char filePath[MAX_PATH_LEN+1];                                               // files selection with path
// QDOAS ???   SZ_LEN len;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   memset(filePath,0,sizeof(filePath));                                          // buffer for selected file(s) with path
// QDOAS ???
// QDOAS ???   // Load "Save" common dialog box
// QDOAS ???
// QDOAS ???   if ((rc=FILES_Open(hwndParent,FILE_TYPE_INI,filePath,MAX_PATH_LEN+1,0,FILE_MODE_SAVE,NULL,ITEM_NONE,NULL,ITEM_NONE))!=0)
// QDOAS ???    {
// QDOAS ???     // Retrieve complete file name and path; complete extension if needed
// QDOAS ???
// QDOAS ???     if (!strchr(filePath,'.'))
// QDOAS ???      {
// QDOAS ???       len=strlen(filePath);
// QDOAS ???       filePath[len]='.';
// QDOAS ???       strcat(filePath,FILES_types[FILE_TYPE_INI].fileExt);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     strncpy(FILES_configuration,filePath,MAX_PATH_LEN);
// QDOAS ???     FILES_configuration[MAX_PATH_LEN]=0;
// QDOAS ???
// QDOAS ???     // Save all settings
// QDOAS ???
// QDOAS ???     FILES_EnableSaveOption(hwndParent,TRUE);                                    // Enable "save" menu option
// QDOAS ???     FilesSaveConfiguration(hwndParent,FILES_configuration);                     // settings safe keeping
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        FILES_Alloc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Allocate and initialize buffers for file paths
// QDOAS ??? //
// QDOAS ??? // RETURN          ERROR_ID_ALLOC if one of the buffer allocation failed
// QDOAS ??? //                 ERROR_ID_NO in case of success
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC FILES_Alloc(void)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Buffer allocation for paths
// QDOAS ???
// QDOAS ???   if ((FILES_paths=(FILES_PATH *)MEMORY_AllocBuffer("FILES_Alloc ","FILES_paths",FILES_nPaths,sizeof(FILES_PATH),0,MEMORY_TYPE_STRUCT))==NULL)
// QDOAS ???    rc=ERROR_ID_ALLOC;
// QDOAS ???   else
// QDOAS ???    memset(FILES_paths,0,sizeof(FILES_PATH)*FILES_nPaths);
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        FILES_Free
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Release buffers allocated for file paths
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void FILES_Free(void)
// QDOAS ???  {
// QDOAS ???   if (FILES_paths!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("FILES_Free ","FILES_paths",FILES_paths);
// QDOAS ???  }


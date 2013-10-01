
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  LOAD MATRIX FROM FILES
//  Name of module    :  MATRIX.C
//  Creation date     :  June 1997
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
//  This module allows to load data dynamically from files to matrix objects and
//  also pre-calculate second derivatives for future spline interpolation.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  MATRIX_Allocate - allocate buffers for a matrix object;
//  MATRIX_Free - release the buffers allocated for a matrix;
//  MATRIX_Copy - copy the content of a matrix in another one;
//  MATRIX_Load - load a matrix from file
//
//  ----------------------------------------------------------------------------

#include "doas.h"

// ==================
// BUFFERS PROCESSING
// ==================

// -----------------------------------------------------------------------------
// FUNCTION      MATRIX_Allocate
// -----------------------------------------------------------------------------
// PURPOSE       Allocate buffers for a matrix object
//
// INPUT         nl, nc          : resp. the number of lines and columns of the matrix to allocate
//               basel, basec    : resp. the base indexes for lines and columns of the matrix
//               allocateDeriv2  : 1 to allocate buffers for the second derivatives
//               callingFunction : the name of the calling function
//
// OUTPUT        pMatrix, structure to receive the information on the new allocated object
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC MATRIX_Allocate(MATRIX_OBJECT *pMatrix,INT nl,INT nc,INT basel,INT basec,INT allocateDeriv2, const char *callingFunction)
 {
  // Declarations

  char functionName[MAX_FCT_LEN+1];                                            // the name of the calling function
  RC rc;                                                                        // the return code

  // Initializations

  memset(functionName,0,MAX_FCT_LEN+1);
  rc=ERROR_ID_NO;

  // Build the complete function name

  if (strlen(callingFunction)<=MAX_FCT_LEN-strlen("MATRIX_Allocate via "))
   sprintf(functionName,"MATRIX_Allocate via %s",callingFunction);
  else
   sprintf(functionName,"MATRIX_Allocate");

  // Reset the matrix object

  MATRIX_Free(pMatrix,functionName);

  // Allocate bufferS for the matrix and the second derivatives

  if (((pMatrix->matrix=(double **)MEMORY_AllocDMatrix(functionName,"matrix",basel,nl+basel-1,basec,nc+basec-1))==NULL) ||
       (allocateDeriv2 && ((pMatrix->deriv2=(double **)MEMORY_AllocDMatrix(functionName,"deriv2",basel,nl+basel-1,basec+1,nc+basec-1))==NULL)))

   rc=ERROR_ID_ALLOC;

  // Store the information on the new allocated matrix

  else
   {
    pMatrix->nl=nl;
    pMatrix->nc=nc;
    pMatrix->basel=basel;
    pMatrix->basec=basec;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      MATRIX_Free
// -----------------------------------------------------------------------------
// PURPOSE       Release the buffers allocated for a matrix
//
// INPUT         pMatrix         : information on the matrix to release
//               callingFunction : the name of the calling function
// -----------------------------------------------------------------------------

void MATRIX_Free(MATRIX_OBJECT *pMatrix, const char *callingFunction)
 {
  // Declaration

  char functionName[MAX_FCT_LEN+1];

  // Initialization

  memset(functionName,0,MAX_FCT_LEN+1);

  // Build the complete function name

  if (strlen(callingFunction)<=MAX_FCT_LEN-strlen("MATRIX_Free via "))
   sprintf(functionName,"MATRIX_Free via %s",callingFunction);
  else
   sprintf(functionName,"MATRIX_Free");

  // Release the allocated buffers

  if (pMatrix->matrix!=NULL)
   MEMORY_ReleaseDMatrix(functionName,"matrix",pMatrix->matrix,pMatrix->basec,pMatrix->nc+pMatrix->basec-1,pMatrix->basel);
  if (pMatrix->deriv2!=NULL)
   MEMORY_ReleaseDMatrix(functionName,"deriv2",pMatrix->deriv2,pMatrix->basec+1,pMatrix->nc+pMatrix->basec-1,pMatrix->basel);

  // Reset pointers and indexes

  memset(pMatrix,0,sizeof(MATRIX_OBJECT));
 }

// -----------------------------------------------------------------------------
// FUNCTION      MATRIX_Copy
// -----------------------------------------------------------------------------
// PURPOSE       Copy the content of a matrix in another one
//
// INPUT         pSource         : a pointer to the matrix to copy
//               callingFunction : the name of the calling function
//
// INPUT/OUTPUT  pTarget, structure to a new matrix object that is a copy of
//               the pSource one
//
// RETURN        ERROR_ID_ALLOCMATRIX if the allocation of a the target matrix failed;
//               ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC MATRIX_Copy(MATRIX_OBJECT *pTarget,MATRIX_OBJECT *pSource, const char *callingFunction)
 {
  // Declarations

  INDEX indexC;
  RC rc;

  // Initializations

  MATRIX_Free(pTarget,"MATRIX_Copy");
  rc=ERROR_ID_NO;

  // Use the dimensions of the source matrix to allocate the target matrix

  if ((pSource->nl==0) || (pSource->nc==0) || (pSource->matrix==NULL))
   ERROR_SetLast("MATRIX_Copy",ERROR_TYPE_FATAL,ERROR_ID_ALLOCMATRIX,"pSource",
                  pSource->basel,pSource->nl+pSource->basel-1,pSource->basec,pSource->nc+pSource->basec-1);

  else if ((rc=MATRIX_Allocate(pTarget,pSource->nl,pSource->nc,pSource->basel,pSource->basec,
           (pSource->deriv2!=NULL)?1:0,callingFunction))==ERROR_ID_NO)

   {
   	// Make a copy of the input matrix into the new allocated one

    for (indexC=pSource->basec;indexC<pSource->basec+pSource->nc;indexC++)
     memcpy(pTarget->matrix[indexC]+pSource->basel,pSource->matrix[indexC]+pSource->basel,sizeof(double)*pSource->nl);

    // Make a copy of the second derivatives

    if (pSource->deriv2!=NULL)
     for (indexC=pSource->basec+1;indexC<pSource->basec+pSource->nc;indexC++)
      memcpy(pTarget->deriv2[indexC]+pSource->basel,pSource->deriv2[indexC]+pSource->basel,sizeof(double)*pSource->nl);
   }

  // Return

  return rc;
 }

// ================
// FILES PROCESSING
// ================

// -----------------------------------------------------------------------------
// FUNCTION      MATRIX_Load
// -----------------------------------------------------------------------------
// PURPOSE       Load a matrix from file
//
// INPUT         fileName        : te name of the file to load
//               basel,basec     : resp. the base indexes for lines and columns
//               nl,nc           : resp. the number of lines and columns of the matrix to load
//                                 0 to retrieve the dimensions automatically from the file
//               xmin,xmax       : define a range of values to load for the first column of the matrix
//                                 usually the wavelength calibration
//               allocateDeriv2  : 1 to allocate buffers for the second derivatives
//               reverseFlag     : 1 to reverse the matrix (flip up/down)
//               callingFunction : the name of the calling function
//
// INPUT/OUTPUT  pMatrix, structure to receive the data loaded from the input file
//
// RETURN        ERROR_ID_FILE_NOT_FOUND if the input file can not be open;
//               ERROR_ID_FILE_EMPTY if the length of the input file is 0;
//               ERROR_ID_WAVELENGTH if the wavelength calibration of the input file
//                                   doesn't cover the analysis spectral range
//               ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC MATRIX_Load(char *fileName,MATRIX_OBJECT *pMatrix,
               INT basel,INT basec,INT nl,INT nc,double xmin,double xmax,
               INT allocateDeriv2,INT reverseFlag, const char *callingFunction)
 {
  // Declarations

  char   *oldColumn,*nextColumn,                                               // get lines from the input file
           fullPath[MAX_ITEM_TEXT_LEN+1];                                       // the complete file name to load
  INT      nltmp,nctmp,                                                         // highest indexes for lines and columns
           lineLength,                                                          // length of a file line
           fileLength,                                                          // length of the file (in bytes)
           nlMin,ncMin;                                                         // resp. the minimum numbers of lines and columns to load from the file
  INDEX    i,j;                                                                 // indexes for browsing lines and columns in matrix
  double **matrix,**deriv2,                                                     // resp. pointers to the matrix to load and to the second derivatives
         **col,                                                                 // a column in the matrix to load
           xMin,xMax,                                                           // define the range of values to load for the first column of the matrix
           tempValue;                                                           // a value of the matrix to load
  FILE    *fp;                                                                  // file pointer
  RC       rc;                                                                  // return code

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("MATRIX_Load",DEBUG_FCTTYPE_FILE);
  #endif

  // Initializations

  FILES_RebuildFileName(fullPath,fileName,1);                                   // build the complete path and file name

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Print("File to load : %s\n",fullPath);
  #endif

  oldColumn=nextColumn=NULL;                                                    // initialize pointers
  col=NULL;
  nlMin=nl;
  ncMin=nc;
  rc=ERROR_ID_NO;

  xMin=min(xmin,xmax);
  xMax=max(xmin,xmax);

  // Reset matrix

  MATRIX_Free(pMatrix,"MATRIX_Load");

  // File open

  if ((fp=fopen(fullPath,"rt"))==NULL)
   rc=ERROR_SetLast("MATRIX_Load",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,fullPath);
  else if (!(fileLength=STD_FileLength(fp)))
   rc=ERROR_SetLast("MATRIX_Load (1)",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fullPath);

  // Allocate buffers for reading the lines of the file

  else if (((oldColumn=(char*)MEMORY_AllocBuffer("MATRIX_Load","oldColumn",MAX_ITEM_TEXT_LEN+1,sizeof(char),0,MEMORY_TYPE_STRING))==NULL) ||
           ((nextColumn=(char*)MEMORY_AllocBuffer("MATRIX_Load","nextColumn",MAX_ITEM_TEXT_LEN+1,sizeof(char),0,MEMORY_TYPE_STRING))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
   	// The function has to determine the number of lines and columns

    if (!nl || !nc)
     {
      // Bypass commented lines (assumed to start with character ';' or '*')

      while (fgets(oldColumn,MAX_ITEM_TEXT_LEN,fp) && ((strchr(oldColumn,';')!=NULL) || (strchr(oldColumn,'*')!=NULL)));

      // Determine the number of columns

      sscanf(oldColumn,"%lf",(double *)&tempValue);
      nl=((xMin==xMax) || ((tempValue>=xMin)&&(tempValue<=xMax)))?1:0;

      for (nc=0;strlen(oldColumn);nc++)
       {
        lineLength=strlen(oldColumn);
        oldColumn[lineLength++]='\n';
        oldColumn[lineLength]=0;

        memset(nextColumn,0,MAX_ITEM_TEXT_LEN);
        sscanf(oldColumn,"%lf %[^'\n']",(double *)&tempValue,nextColumn);
        strcpy(oldColumn,nextColumn);
       }

      // Determine the number of lines

      for (;!feof(fp) && fgets(oldColumn,MAX_ITEM_TEXT_LEN,fp);)
       if ((strchr(oldColumn,';')==NULL) && (strchr(oldColumn,'*')==NULL) && strlen(STD_StrTrim(oldColumn)))
        {
         sscanf(oldColumn,"%lf",(double *)&tempValue);
         if ((xMin==xMax) || ((tempValue>=xMin)&&(tempValue<=xMax)))
          nl++;
        }
     }

    // File read out

    #if defined(__DEBUG_) && __DEBUG_
    DEBUG_Print("Size of the matrix to load : %d x %d last Value %g\n",nl,nc,tempValue);
    #endif

    if (!nl || !nc || (nl<nlMin) || (nc<ncMin))
     rc=ERROR_SetLast("MATRIX_Load (2)",ERROR_TYPE_WARNING,ERROR_ID_FILE_EMPTY,fullPath);
    else if ((col=(double **)MEMORY_AllocDMatrix("MATRIX_Load","col",basel,nl+basel-1,basec,basec+4 /* 5 columns */))==NULL)
     rc=ERROR_ID_ALLOC;
    else if (!(rc=MATRIX_Allocate(pMatrix,nl,nc,basel,basec,allocateDeriv2,callingFunction)))
     {
      matrix=pMatrix->matrix;
      deriv2=pMatrix->deriv2;
      nltmp=nl+basel-1;
      nctmp=nc+basec-1;

      fseek(fp,0L,SEEK_SET);

      // if the number of columns to read is no higher than 5, read all the columns in one sscanf operation

      if (nc<=5)
       {
        for (i=basel;(i<=nltmp) && fgets(oldColumn,MAX_ITEM_TEXT_LEN,fp);)
         {
          if ((strchr(oldColumn,';')==NULL) && (strchr(oldColumn,'*')==NULL) &&
              (sscanf(oldColumn,"%lf %lf %lf %lf %lf",
                     (double *)&tempValue,
                     (double *)&col[basec+1][i],
                     (double *)&col[basec+2][i],
                     (double *)&col[basec+3][i],
                     (double *)&col[basec+4][i])>=nc) &&
             ((xMin==xMax) || ((tempValue>=xMin) && (tempValue<=xMax))))
           {
            col[basec][i]=tempValue;
            i++;
           }
         }

        for (j=basec;j<=nctmp;j++)
         memcpy(matrix[j]+basel,col[j]+basel,sizeof(double)*nl);
       }

      // for files with a number of columns higher than 5, loop on lines AND columns

      else

       for (i=basel;(i<=nltmp) && fgets(oldColumn,MAX_ITEM_TEXT_LEN,fp);)

        if ((strchr(oldColumn,';')==NULL) && (strchr(oldColumn,'*')==NULL))
         {
          sscanf(oldColumn,"%lf",(double *)&tempValue);

          if ((xMin==xMax) || ((tempValue>=xMin)&&(tempValue<=xMax)))
           {
            for (j=basec;j<=nctmp;j++)
             {
              lineLength=strlen(oldColumn);

              oldColumn[lineLength++]='\n';
              oldColumn[lineLength]=0;

              memset(nextColumn,0,MAX_ITEM_TEXT_LEN);
              sscanf(oldColumn,"%lf %[^'\n']",(double *)&matrix[j][i],nextColumn);
              strcpy(oldColumn,nextColumn);
             }

            i++;
           }
         }

      // Found no lines with a starting values in the range [xmin..xmax]

      if (i==0)
       rc=ERROR_SetLast("MATRIX_Load",ERROR_TYPE_WARNING,ERROR_ID_WAVELENGTH,fullPath);

      // Flip up/down the matrix

      else if (reverseFlag && (matrix[basec][basel]>matrix[basec][basel+1]))
       for (i=basel;i<=nltmp/2;i++)
        for (j=basec;j<=nctmp;j++)
         {
          tempValue=matrix[j][i];
          matrix[j][i]=matrix[j][nltmp-i];
          matrix[j][nltmp-i]=tempValue;
         }

      // Calculate second derivatives of the columns of the matrix for future interpolation

      if (allocateDeriv2)
       for (j=basec+1;(j<=nctmp)&&!rc;j++)
        rc=SPLINE_Deriv2(((double *)matrix[basec])+basel,
                         ((double *)matrix[j])+basel,
                         ((double *)deriv2[j])+basel,
                                    pMatrix->nl,
                                    callingFunction);
     }
   }

  // Close file

  if (fp!=NULL)
   fclose(fp);

  // Release allocated buffers

  if (rc)
   MATRIX_Free(pMatrix,"MATRIX_Load");

  if (oldColumn!=NULL)
   MEMORY_ReleaseBuffer("MATRIX_Load","oldColumn",oldColumn);
  if (nextColumn!=NULL)
   MEMORY_ReleaseBuffer("MATRIX_Load","nextColumn",nextColumn);
  if (col!=NULL)
   MEMORY_ReleaseDMatrix("MATRIX_Load","col",col,basec,basec+4,basel);

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  if (!rc)
   {
    DEBUG_PrintVar("the matrix",pMatrix->matrix,basel,basel+nl-1,basec,basec+nc-1,NULL);
    if (allocateDeriv2)
     DEBUG_PrintVar("the 2nd derivatives",pMatrix->deriv2,basel,basel+nl-1,basec+1,basec+nc-1,NULL);
   }
  DEBUG_FunctionStop("MATRIX_Load",(RC)rc);
  #endif

  // Return

  return rc;
 }

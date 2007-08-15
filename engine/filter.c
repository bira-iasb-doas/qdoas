
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  THE BIRA-IASB DOAS SOFTWARE FOR WINDOWS AND LINUX
//  Module purpose    :  FILTERING FUNCTIONS
//  Name of module    :  FILTER.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in WinDOAS package in 97.
//
//  References        :  Routines in this module come from different sources
//
//      - numerical recipes;
//      - mathworks;
//      - ...
//
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
//  Filtering functions from different sources
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  Fourier - fast Fourier Transform algorithm   
//  realft - fast Fourier Transform of a real vector
//  ModBessel - modified Bessel function of zeroth order  
//  Neq_Ripple - build a Kaizer filter    
//  FilterPinv - pseudoInverse function function
//  FilterSavitskyGolay - build a Savitsky-Golay filter function
//  FilterPascalTriangle - build a Pascal binomial filter function
//  FILTER_Build - build a filter function
//  FilterConv - apply a filter function on a vector by convolution on pixels
//  FILTER_Vector - apply a filter function on a vector
//
//  ----------------------------------------------------------------------------

// ===============
// INCLUDE HEADERS
// ===============

#include "doas.h"

#define   MAXNP            150
#define   EPS     (double)   2.2204e-016

// -----------------------------------------------------------------------------
// FUNCTION      fourier
// -----------------------------------------------------------------------------
// PURPOSE       Fast Fourier Transform algorithm
//
// INPUT/OUTPUT  data  complex array of length nn (ie real array of length 2*nn)
//
// INPUT         nn    the size of data (should be an integer power of 2)
//
//               is    +1 to replace data by its discrete Fourier transform
//                     -1 to replace data by its inverse discrete FT
// -----------------------------------------------------------------------------

void fourier(double *data,int nn,int is)
 {
 	// Declarations

  double wtemp,theta,wr,wi,wpr,wpi,tempr,tempi;
  int    j,n,i,m,nmax,istep;
  // Bit-reversal section

  n=2*nn;
  j=1;

  for (i=1;i<=n;i+=2)
   {
    if (j>i)
     {
      tempr=data[j];
      tempi=data[j+1];
      data[j]=data[i];
      data[j+1]=data[i+1];
      data[i]=tempr;
      data[i+1]=tempi;
     }

    m=n/2;

    while ((m>=2) && (j>m))
     {
      j=j-m;
      m=m/2;
     }

    j=j+m;
   }

  nmax=2;

  while (n>nmax)
   {
    istep=2*nmax;
    theta=PI2/(is*nmax);
    wpr=-(double)2.*(sin((double)0.5*theta))*(sin((double)0.5*theta));
    wpi=(double)sin(theta);
    wr=(double)1.0;
    wi=(double)0.0;

    for (m=1;m<=nmax;m+=2)
     {
      for (i=m;i<=n;i+=istep)
       {
        j=i+nmax;
        tempr=wr*data[j]-wi*data[j+1];
        tempi=wr*data[j+1]+wi*data[j];
        data[j]=data[i]-tempr;
        data[j+1]=data[i+1]-tempi;
        data[i]=data[i]+tempr;
        data[i+1]=data[i+1]+tempi;
       }
      wtemp=wr;
      wr=wr*wpr-wi*wpi+wr;
      wi=wi*wpr+wtemp*wpi+wi;
     }

    nmax=istep;
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      realft
// -----------------------------------------------------------------------------
// PURPOSE       Fast Fourier Transform of a real vector
//
// INPUT         source  input real data vector
//               nn      the size the input vector (should be a power of 2)
//               is      +1 to calculate the discrete Fourier transform
//                       -1 to calculate its inverse discrete Fourier transform
//
// OUTPUT        buffer  the result of the fourier tranform of input vector
// -----------------------------------------------------------------------------

void realft(double *source,double *buffer,int nn,int is)
 {
 	// Declarations

  double wr,wi,wpr,wpi,wtemp,theta;
  double c1,c2,h1r,h1i,h2r,h2i,wrs,wis;
  int index,index1,index2,index3,index4,n2p3,ndemi;

  // Make a copy of the original data

  memcpy(buffer+1,source+1,nn*sizeof(double));

  // Initializations

  ndemi=nn/2;
  theta=PI/(double)ndemi;
  c1=0.5;

  // Forward transform

  if (is==1)
   {
    c2=-0.5;
    fourier(buffer,ndemi,is);
   }

  // Set up for an inverse transform
  else
   {
    c2=0.5;
    theta=-theta;
   }

  wpr=-2.0*pow(sin(0.5*theta),(double)2);
  wpi=sin(theta);
  wr=1.0+wpr;
  wi=wpi;
  n2p3=2*ndemi+3;

  for (index=2;index<=ndemi/2;index++)                                          // case index=1 is done separately
   {
    index1=2*index-1;
    index2=index1+1;
    index3=n2p3-index2;
    index4=index3+1;
    wrs=(double)wr;
    wis=(double)wi;
    h1r=c1*(buffer[index1]+buffer[index3]);                                     // the two separate transforms are separated out of data
    h1i=c1*(buffer[index2]-buffer[index4]);
    h2r=-c2*(buffer[index2]+buffer[index4]);
    h2i=c2*(buffer[index1]-buffer[index3]);
    buffer[index1]=h1r+wrs*h2r-wis*h2i;                                         // here they are recombined to form the true transform of the original real data
    buffer[index2]=h1i+wrs*h2i+wis*h2r;
    buffer[index3]=h1r-wrs*h2r+wis*h2i;
    buffer[index4]=-h1i+wrs*h2i+wis*h2r;
    wtemp=wr;                                                                   // recurrence
    wr=wr*wpr-wi*wpi+wr;
    wi=wi*wpr+wtemp*wpi+wi;
   }

  if (is==1)
   {
    h1r=buffer[1];
    buffer[1]=h1r+buffer[2];
    buffer[2]=h1r-buffer[2];                                                    // squeeze the first and last data together to get them all within the original array
   }
  else
   {
    h1r=buffer[1];
    buffer[1]=c1*(h1r+buffer[2]);
    buffer[2]=c1*(h1r-buffer[2]);
    fourier(buffer,ndemi,is);                                                   // inverse fourier transform
    for (index=1;index<=nn;index++)
     buffer[index]/=ndemi;
   }
 }
 
// -----------------------------------------------------------------------------
// FUNCTION      ModBessel
// -----------------------------------------------------------------------------
// PURPOSE       Evaluates the modified Bessel function of zeroth order   
//               at real values of the arguments 
//
// INPUT         X       input argument
//
// RETURN        modified Bessel function calculated at X
// ----------------------------------------------------------------------------- 

double ModBessel(double X)
 {              
 	// Declarations
 	
  double S, Ds;
  int D;
  
  // Initialization

  S=Ds=(double)1.;
  D=0;     
  
  // Modified Bessel function

  do
   {
    D+=2;
    Ds*=(double)X*X/(D*D);
    S+=Ds;
   }
  while (Ds>(double)0.2e-8*S);
  
  // Return

  return ((double)S); 
 }

// -----------------------------------------------------------------------------
// FUNCTION      Neq_Ripple
// -----------------------------------------------------------------------------
// PURPOSE       build a Kaizer filter function
//
//               Calculates the coefficients of a nearly equiripple linear   
//               phase smoothing filter with an odd number of terms and  
//               even symetry
//
// INPUT         Beta        cutoff frequency 
//               Delta       pass band 
//               dB          tolerance
//
// OUTPUT        pFilter     pointer to the buffer for the calculated filter
//
// RETURN        ERROR_ID_ALLOC if buffer allocation failed, 
//               ERROR_ID_BAD_ARGUMENTS in case of wrong arguments
//               0 on success             
//
// Reference: Kaizer and Reed, Rev. Sci. Instrum., 48, 1447-1457, 1977.
// ----------------------------------------------------------------------------- 

RC FilterNeqRipple (PRJCT_FILTER *pFilter,double *Beta, double *Delta,double *dB)
 { 
 	// Declarations
 	
  int i, j, k,Nterm;
  double Kf, Eta, Be1, Be2, Gk, Dk, Geta;
  double Lam21, Pow1;
  double DNp;
  RC rc;
  
  // Initializations
  
 
  rc=ERROR_ID_NO;
 
  Kf = (double) 1.8445;
  if ( (*dB) >= (double) 21. ) Kf = (double) 0.13927 * ((*dB)-7.95);
 
  Lam21 = (*dB) - (double) 21.;
  Pow1 = pow ( (double) Lam21, (double) 0.4 );
  Eta = 0.58417 * Pow1 + 0.07886 * Lam21;
 
  if ( (*dB) < (double) 21. ) Eta = (double) 0.;
  if ( (*dB) > (double) 50. ) Eta = 0.1102 * ((*dB) - (double) 8.7 );
 
  Nterm = (int)(Kf/(2.*(*Delta))+0.75);
  pFilter->filterSize = Nterm+1;
 
  DNp = (double) Nterm;
  pFilter->filterFunction=NULL;
 
  // Test Np against dimension limit
 
  if ((Nterm<0) || (Nterm>MAXNP))
   rc=ERROR_SetLast("Neq_Ripple",ERROR_TYPE_WARNING,ERROR_ID_BAD_ARGUMENTS);
  else if ((pFilter->filterFunction=(double *)MEMORY_AllocDVector("Neq_Ripple ","pFilter->filterFunction",1,pFilter->filterSize))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    Be1 = (double) ModBessel ( Eta );
 
    for ( k=1; k<=Nterm; k++ )
       {
           Dk   = (double) k;
           Gk   = (double) PI * Dk;
           Geta = (double) Eta * sqrt ( 1. - pow((double) Dk/DNp, (double) 2.) );
 
           Be2  = (double) ModBessel ( Geta );
           pFilter->filterFunction[k] = (double) (sin( (*Beta) * Gk )) / Gk * (Be2/Be1);
       }
 
    pFilter->filterFunction[Nterm] *= (double) 0.5;
 
    for ( i=2; i<=pFilter->filterSize; i++ )
       {
          k = pFilter->filterSize - i + 2;
          j = k-1;
          pFilter->filterFunction[k] = pFilter->filterFunction[j];
       }
 
    pFilter->filterFunction[1] = (*Beta);
   }                                
   
  // Return
 
  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FilterPinv
// -----------------------------------------------------------------------------
// PURPOSE       PseudoInverse function
//
//	              X = PINV(A) produces a matrix X of the same dimensions
//	              as A' so that A*X*A = A, X*A*X = X and AX and XA
//	              are Hermitian. The computation is based on SVD(A) and any
//	              singular values less than a tolerance are treated as zero.
//	              The default tolerance is MAX(SIZE(A)) * NORM(A) * EPS.
//	              This tolerance may be overridden with X = PINV(A,tol).c
//
// INPUT/OUTPUT  pSvd     pointers to SVD buffers
//
// RETURN        return code of the SVD decomposition function             
//
// Copyright (c) 1984-93 by The MathWorks, Inc.
// ----------------------------------------------------------------------------- 

RC FilterPinv(SVD *pSvd)
 {
  // Declarations

  double tolerance;
  INDEX i,j,k,r;
  RC rc;

  // Matrix decomposition

  if ((rc=SVD_Dcmp(pSvd->A,pSvd->DimL,pSvd->DimC,pSvd->W,pSvd->V,pSvd->SigmaSqr,NULL))==ERROR_ID_NO)
   {                                                               
   	// default tolerance
   	
    tolerance=(double)max(pSvd->DimL,pSvd->DimC)*pSvd->W[1]*EPS;   
    
    // singular values less than tolerance are treated as zero

    for (r=1;(r<=pSvd->DimC)&&(pSvd->W[r]>tolerance);r++);    
    
    // Back substitution

    for (j=1;j<=pSvd->DimC;j++)
     for (i=1;i<=pSvd->DimL;i++)
      pSvd->U[j][i]=(double)0.;

    if (r>1)
     for (j=1;j<r;j++)
      for (i=1;i<=pSvd->DimL;i++)
       for (k=1;k<r;k++)
        pSvd->U[j][i]+=pSvd->V[k][j]*pSvd->A[k][i]/pSvd->W[k];
   }

  // Return

  return rc;
 }
 
// -----------------------------------------------------------------------------
// FUNCTION      FilterSavitskyGolay
// -----------------------------------------------------------------------------
// PURPOSE       build a Savitsky-Golay filter function
//
// INPUT         filterWidth filter width
//               filterOrder filter order
//
// OUTPUT        pFilter     pointer to the buffer for the calculated filter
//
// RETURN        ERROR_ID_ALLOC if buffer allocation failed, 0 on success
// -----------------------------------------------------------------------------  

RC FilterSavitskyGolay(PRJCT_FILTER *pFilter,INT filterWidth,INT filterOrder)
 {
  // Declarations

  INDEX i,j;
  SVD svd;
  INT lc;
  RC rc;

  // Initializations

  memset(&svd,0,sizeof(SVD));
  lc=(filterWidth-1)/2;
  svd.DimL=filterWidth;
  svd.DimC=filterOrder+1;

  pFilter->filterSize=lc+1;
  
  // Allocate buffer for filter

  if ((pFilter->filterFunction=(double *)MEMORY_AllocDVector("FilterSavitskyGolay ","pFilter->filterFunction",1,pFilter->filterSize))==NULL)
   rc=ERROR_ID_ALLOC;

  // Allocate buffers for SVD

  else if (!(rc=ANALYSE_SvdLocalAlloc("FilterSavitskyGolay",&svd)))
   {
   	// Build the filter function
   	
    for (j=0;j<=filterOrder;j++)
     for (i=-lc;i<=lc;i++)
      svd.A[j+1][i+lc+1]=(j!=0)?pow((double)i,(double)j):(double)1.;

    if (!(rc=FilterPinv(&svd)))
     memcpy(pFilter->filterFunction+1,&svd.U[1][lc+1],sizeof(double)*pFilter->filterSize);

    // NB : no normalization is needed for the line shape because sum of items in pFilter->filterFunction is already
    //      equal to 1 after FilterPinv
   }

  // Release allocated buffers

  ANALYSE_SvdFree("FilterSavitskyGolay",&svd);

  // Return

  return rc;
 }                                                                                
 
// -----------------------------------------------------------------------------
// FUNCTION      FilterPascalTriangle
// -----------------------------------------------------------------------------
// PURPOSE       build a Pascal binomial filter function
//
// INPUT         power            the filter size
// OUTPUT        coefficients     the calculated filter
//
// RETURN        ERROR_ID_ALLOC if buffer allocation failed, 0 on success
// -----------------------------------------------------------------------------   

RC FilterPascalTriangle(double *coefficients,int power)
 {
  // Declarations

  double *coef;
  INDEX i;
  RC rc;

  // Initializations

  rc=0;

  coefficients[1]=1;

  if (power!=0)
   {
    coefficients[power+1]=(double)1.;

    // Buffer allocation

    if ((coef=MEMORY_AllocDVector("FilterPascalTriangle ","coef",1,power))==NULL)
     rc=ERROR_ID_ALLOC;
    else
     {
      // Apply function recursively

      FilterPascalTriangle(coef,power-1);

      for (i=2;i<=power;i++)
       coefficients[i]=coef[i]+coef[i-1];

      MEMORY_ReleaseDVector("FilterPascalTriangle ","coef",coef,1);
     }
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FILTER_Build
// -----------------------------------------------------------------------------
// PURPOSE       build a filter function
//
// INPUT/OUTPUT  pFilter          pointer to user filter options from project properties
// OUTPUT        fa1,fa2,fa3      filter options
//
// RETURN        ERROR_ID_ALLOC if buffer allocation failed,                  
//               ERROR_ID_DIVISION_BY_0 if filter is 0 everywhere,
//               return code of the filtering function if any
//               0 on success
// -----------------------------------------------------------------------------

RC FILTER_Build(PRJCT_FILTER *pFilter,double fa1,double fa2,double fa3)
 {
  // Declarations

  INT filterType;
  double sum;
  INDEX i;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  filterType=pFilter->type;
  sum=(double)0.;

  // Build filter

  if (filterType==PRJCT_FILTER_TYPE_KAISER)
   rc=FilterNeqRipple(pFilter,&fa1,&fa2,&fa3);
  else if (filterType==PRJCT_FILTER_TYPE_SG)
   rc=FilterSavitskyGolay(pFilter,(int)fa1,(int)fa2);
  else
   {
    switch(filterType)
     {
   // -------------------------------------------------------------------------
      case PRJCT_FILTER_TYPE_GAUSSIAN :
       pFilter->filterSize=(int)(ceil(2.*fa1)+1.);
      break;
   // -------------------------------------------------------------------------
      case PRJCT_FILTER_TYPE_BOXCAR :
      case PRJCT_FILTER_TYPE_TRIANGLE :
       pFilter->filterSize=(((INT)fa1-1)>>1)+1;
      break;
   // -------------------------------------------------------------------------
      case PRJCT_FILTER_TYPE_BINOMIAL :
       pFilter->filterSize=(int)fa1;
      break;
   // -------------------------------------------------------------------------
      case PRJCT_FILTER_TYPE_ODDEVEN :
       pFilter->filterSize=2;
      break;
   // -------------------------------------------------------------------------
     }

    // Buffer allocation

    if ((pFilter->filterFunction=(double *)MEMORY_AllocDVector("FILTER_Build","pFilter->filterFunction",1,pFilter->filterSize))==NULL)
     rc=ERROR_ID_ALLOC;
    else
     {
      if (filterType==PRJCT_FILTER_TYPE_BINOMIAL)
       {         
       	if (!(rc=FilterPascalTriangle(pFilter->filterFunction,pFilter->filterSize-1)))
       	 {
          memcpy(pFilter->filterFunction+1,pFilter->filterFunction+(pFilter->filterSize>>1)+1,sizeof(double)*((pFilter->filterSize+1)>>1));
          sum=(double)pow(2.,pFilter->filterSize-1);
         }

        pFilter->filterSize=(pFilter->filterSize+1)>>1;
       }
      else
       {
        for (sum=(double)0.,pFilter->filterFunction[1]=(double)1.,i=1;i<pFilter->filterSize;i++)

         if (filterType==PRJCT_FILTER_TYPE_GAUSSIAN)
          sum+=(pFilter->filterFunction[i+1]=exp((double)-4.*log(2.)*i*i/(fa1*fa1)));
         else if (filterType==PRJCT_FILTER_TYPE_TRIANGLE)
          sum+=(pFilter->filterFunction[i+1]=(double)1.-2.*i/(fa1+1.));
         else if (filterType==PRJCT_FILTER_TYPE_BOXCAR)
          sum+=(pFilter->filterFunction[i+1]=(double)1.);

        sum=2.*sum+pFilter->filterFunction[1];
       }

      // function normalization by its integral
          
      if (sum==(double)0.)    
       rc=ERROR_SetLast("FILTER_Build",ERROR_TYPE_FATAL,ERROR_ID_DIVISION_BY_0);   
      else
       for (i=1;i<=pFilter->filterSize;i++)
        pFilter->filterFunction[i]/=sum;

     }
   }

  // Effective smoothing width

  if (!rc && (pFilter->type!=PRJCT_FILTER_TYPE_NONE) && (pFilter->type!=PRJCT_FILTER_TYPE_ODDEVEN))
   {
    for (pFilter->filterEffWidth=(double)0.,i=2;i<=pFilter->filterSize;i++)     // sum of all elements of the filter normalized by the maximum
     pFilter->filterEffWidth+=pFilter->filterFunction[i];

    pFilter->filterEffWidth=(pFilter->filterFunction[1]>0)?(double)1.+2.*pFilter->filterEffWidth/pFilter->filterFunction[1]:1;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      FilterConv
// -----------------------------------------------------------------------------
// PURPOSE       Apply a filter function on a vector by convolution on pixels
//
// INPUT         pFilter          filter function and options
//               Input            vector to filter
//               Size             the size of input vector
//
// OUTPUT        output           the filtered vector
//
// RETURN        ERROR_ID_ALLOC if buffer allocation failed,                  
//               0 on success
// -----------------------------------------------------------------------------

RC FilterConv(PRJCT_FILTER *pFilter,double *Input,double *Output,int Size)
 {
  // Declarations

  int     i,j,k;
  double *ftemp;
  RC      rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Temporary buffer allocation

  if ((ftemp=(double *)MEMORY_AllocDVector("FILTER_Conv ","ftemp",0,Size-1))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   {
    // Initialize the temporary buffer

    VECTOR_Init(ftemp,(double)0.,Size);

    // Filtering by convolution

    for ( i=0; i<Size; i++ )
      {
        j=-(pFilter->filterSize-1);
        ftemp[i]=(i-j<Size)?Input[(i-j)]*pFilter->filterFunction[-j+1]:(double)0.;
        for (j=j+1;j<pFilter->filterSize;j++)
         {
          k=((i-j<Size)&&(i-j>=0))?i-j:i+j;

          if ((k<Size) && (k>=0))
           ftemp[i]+=Input[k]*pFilter->filterFunction[(j<=0)?-j+1:j+1];
         }
      }

    memcpy(Output,ftemp,Size*sizeof(double));

    // Release allocated buffer

    MEMORY_ReleaseDVector("FILTER_Conv ","ftemp",ftemp,0);
   }

  // return

  return rc;
 }
 
// -----------------------------------------------------------------------------
// FUNCTION      FILTER_Vector
// -----------------------------------------------------------------------------
// PURPOSE       Apply a filter function on a vector
//
// INPUT         pFilter          filter function and options
//               Input            vector to filter                    
//               Size             the size of input vector
//               outputType       type of filtering to apply (high or low)
//
// OUTPUT        output           the filtered vector
//
// RETURN        ERROR_ID_ALLOC if buffer allocation failed,                  
//               return code of the filtering function if any
//               0 on success
// ----------------------------------------------------------------------------- 

RC FILTER_Vector(PRJCT_FILTER *pFilter,double *Input,double *Output,int Size,INT outputType)
 {
  // Declarations

  double *tempVector;
  INDEX i,j;
  RC rc;

  // Initializations

  tempVector=NULL;
  rc=ERROR_ID_NO;

  if (Size>0)
   {
    if ((tempVector=(double *)MEMORY_AllocDVector("FILTER_vector ","tempVector",0,Size-1))==NULL)
     rc=ERROR_ID_ALLOC;
    else
     {
      memcpy(tempVector,Input,sizeof(double)*Size);
      for (i=0;(i<pFilter->filterNTimes) && !rc;i++)
       rc=FilterConv(pFilter,tempVector,tempVector,Size);

      if (i==pFilter->filterNTimes)
       {
        if (outputType==PRJCT_FILTER_OUTPUT_LOW)
         memcpy(Output,tempVector,sizeof(double)*Size);
        else if (outputType==PRJCT_FILTER_OUTPUT_HIGH_SUB)
         for (j=0;j<Size;j++)
          Output[j]=Input[j]-tempVector[j];
        else if (outputType==PRJCT_FILTER_OUTPUT_HIGH_DIV)
         for (j=0;j<Size;j++)
          Output[j]=(tempVector[j]!=(double)0.)?Input[j]/tempVector[j]:(double)0.;
       }
     }
   }

  // Release allocated vector

  if (tempVector!=NULL)
   MEMORY_ReleaseDVector("FILTER_vector ","tempVector",tempVector,0);

  // Return

  return rc;
 }

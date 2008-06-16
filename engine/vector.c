
//  Product/Project   :  QDOAS
//  Module purpose    :  UTILITY FUNCTIONS TO HANDLE VECTORS
//  Name of module    :  VECTOR.C
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
//  MODULE DESCRIPTION :
//
//  This module includes some utility functions to handle more easily the math
//  vectors.  The use of these functions can improve the reading of the program
//  Most of them exist since 1990.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  VECTOR_Init   - initialization of all vector components to one constant value;
//  VECTOR_Equal  - compares two vectors;
//  VECTOR_Max    - search for the largest component in vector;
//  VECTOR_Min    - search for the lowest component in vector;
//  VECTOR_Log    - calculates the logarithm of the vector;
//  VECTOR_LocGt  - search for the first component larger than a specified value;
//  VECTOR_Invert - invert the order of components in vector;
//  VECTOR_Table1 - look-up table 2-D, 0 based (linear interpolation);
//  VECTOR_Table2 - look-up table 2-D, 1 based (linear interpolation);
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "doas.h"

// -----------------------------------------------------------------------------
// FUNCTION      VECTOR_Init
// -----------------------------------------------------------------------------
// PURPOSE       initialization of all vector components to one constant value
//
// INPUT         value  : the constant value to use for initializing the vector;
//               dim    : the size of vector to initialize;
//
// OUTPUT        vector : the content of this vector is modified by the function.
// -----------------------------------------------------------------------------

    void VECTOR_Init ( double *vector, double value, int dim )
    { int i;
      for ( i=0; i<dim; vector[i++]=value ); }

// -----------------------------------------------------------------------------
// FUNCTION      VECTOR_Equal
// -----------------------------------------------------------------------------
// PURPOSE       compares two vectors
//
// INPUT         vector1,vector2 : the two vectors to compare;
//               dim             : the size of vectors to compare;
//               error           : accepted tolerance in term of relative error;
//
// RETURN        0 if vectors are not the same regarding the specified tolerance;
//               1 if vectors are identical regarding the specified tolerance;
// -----------------------------------------------------------------------------

    int VECTOR_Equal ( double *vector1, double *vector2, int dim, double error )
    { int i;

      for ( i=0; i<dim; i++ )
       if (((vector2[i]==(double)0.) && (vector1[i]!=vector2[i])) ||
           ((vector2[i]!=(double)0.) && (fabs(vector1[i]-vector2[i])/fabs(vector2[i])>error)))
        return(0);

      return(1); }

// -----------------------------------------------------------------------------
// FUNCTION      VECTOR_Max
// -----------------------------------------------------------------------------
// PURPOSE       search for the largest component in vector
//
// INPUT         vector : input vector;
//               dim    : the size of input vector;
//
// RETURN        the largest component in vector;
// -----------------------------------------------------------------------------

    double VECTOR_Max ( double *vector, int dim )
    { int i=0; double Max=vector[0];
      while ( ++i < dim )
            { if ( vector[i] > Max ) Max = vector[i]; }
      return ( (double) Max ); }

// -----------------------------------------------------------------------------
// FUNCTION      VECTOR_Min
// -----------------------------------------------------------------------------
// PURPOSE       search for the lowest component in vector
//
// INPUT         vector : input vector;
//               dim    : the size of input vector;
//
// RETURN        the lowest component in vector;
// -----------------------------------------------------------------------------

    double VECTOR_Min ( double *vector, int dim )
    { int i=0; double Min=vector[0];
      while ( ++i < dim )
            { if ( vector[i] < Min ) Min = vector[i]; }
      return ( (double) Min ); }

// -----------------------------------------------------------------------------
// FUNCTION      VECTOR_Log
// -----------------------------------------------------------------------------
// PURPOSE       calculates the logarithm of the vector
//
// INPUT         out             : output vector;
//               in              : input vector;
//               dim             : the size of the vectors;
//               callingFunction : in case of function fails, the name of the calling function;
//
// RETURN        ERROR_ID_LOG if an element in the input vector is negative or zero,
//               0 otherwise.
// -----------------------------------------------------------------------------

    RC VECTOR_Log(double *out,double *in,int dim,unsigned char *callingFunction)
     {
      // Declarations

      INDEX i;
      RC rc;

      // Calculates the logarithm of the input vector

      for (rc=0,i=0;(i<dim) && !rc;i++)

       if (in[i]<=(double)0.)
        rc=ERROR_SetLast(callingFunction,ERROR_TYPE_WARNING,ERROR_ID_LOG);
       else
        out[i]=log(in[i]);

      // Return

      return rc;
     }

// -----------------------------------------------------------------------------
// FUNCTION      VECTOR_LocGt
// -----------------------------------------------------------------------------
// PURPOSE       search for the first component larger than a specified value
//
// INPUT         vector : input vector;
//               value  : the constant value to compare to vector components;
//               dim    : the size of input vector;
//
// RETURN        index of the first component larger than the specified value
// -----------------------------------------------------------------------------

    int VECTOR_LocGt ( double *vector, double value, int dim )
    { int i=0;
      while ( ( i < dim ) && ( vector[i] <= value ) ) i++;
      return ( i ); }

// -----------------------------------------------------------------------------
// FUNCTION      VECTOR_Invert
// -----------------------------------------------------------------------------
// PURPOSE       invert the order of components in vector;
//
// INPUT         dim    : the size of input vector;
//
// OUTPUT        vector : the content of this vector is modified by the function
// -----------------------------------------------------------------------------

    void VECTOR_Invert ( double *vector ,int dim )
    {
       double tmp;
       int   min = 0;
       int   max = dim-1;

       while ( min < max )
             {
               tmp = vector[min];
               vector[min++] = vector[max];
               vector[max--] = tmp;
             }
    }

// -----------------------------------------------------------------------------
// FUNCTION      VECTOR_Table1
// -----------------------------------------------------------------------------
// PURPOSE       Look-up table 2-D, 0 based (linear interpolation)
//
// INPUT         X0,Y0  : coordinates vectors for both dimensions of the data matrix;
//               value  : the constant value to compare to vector components;
//               Table  : data matrix for each (X0,Y0) couple of coordinates;
//               X,Y    : the new coordinates;
//
// RETURN        interpolated value from Table in X and Y coordinates
// -----------------------------------------------------------------------------

    double VECTOR_Table1 ( double *X0, int Nx, double *Y0, int Ny, double **Table, double X, double Y )
    {
       int Ix, Iy;
       double Zi, Tab1, Tab2, LUTable;

       Ix = VECTOR_LocGt ( X0, X, Nx );
       Iy = VECTOR_LocGt ( Y0, Y, Ny );

       if ( (Ix==Nx) && (Iy==Ny) )  return ( (double) Table[Ny-1][Nx-1] );
       if ( Ix==Nx )
         {
           Zi = (double) (Y - Y0[Iy-1]) / (Y0[Iy] - Y0[Iy-1]);
           LUTable = (double) ( Table[Iy-1][Nx-1] + (Table[Iy][Nx-1]-Table[Iy-1][Nx-1]) * Zi );
           return ( (double) LUTable );
         }
       if ( Iy==Ny )
         {
           Zi = (double) (X - X0[Ix-1]) / (X0[Ix] - X0[Ix-1]);
           LUTable = (double) ( Table[Ny-1][Ix-1] + (Table[Ny-1][Ix]-Table[Ny-1][Ix-1]) * Zi );
           return ( (double) LUTable );
         }

       Zi = (double) ( X - X0[Ix-1] ) / ( X0[Ix] - X0[Ix-1] );
       Tab1 = (double) ( Table[Iy-1][Ix-1] + (Table[Iy-1][Ix]-Table[Iy-1][Ix-1]) * Zi);
       Tab2 = (double) ( Table[Iy][Ix-1] + (Table[Iy][Ix]-Table[Iy][Ix-1]) * Zi);

       Zi = (double) (Y - Y0[Iy-1]) / (Y0[Iy] - Y0[Iy-1]);
       LUTable = (double) ( Tab1 + (Tab2-Tab1) * Zi );

       return ( (double) LUTable );
    }

// -----------------------------------------------------------------------------
// FUNCTION      VECTOR_Table2
// -----------------------------------------------------------------------------
// PURPOSE       Look-up table 2-D, 1 based (linear interpolation)
//
// INPUT         Table  : data matrix with first line and first column,
//                        coordinates vectors for both dimensions;
//               Nx,Ny  : dimensions of the matrix
//               X,Y    : the new coordinates;
//
// RETURN        interpolated value from Table in X and Y coordinates
// -----------------------------------------------------------------------------

    double VECTOR_Table2 ( double **Table, INT Nx, INT Ny, double X, double Y )
    {
       int Ix, Iy;
       double Zi, Tab1, Tab2, LUTable;

       for (Ix=2;Ix<=Nx;Ix++)
        if ((double)X<Table[1][Ix])
         break;

       for (Iy=2;Iy<=Ny;Iy++)
        if ((double)Y<Table[Iy][1])
         break;

       if ( (Ix>Nx) && (Iy>Ny) )  return ( (double) Table[Ny][Nx] );

       if ( Ix>Nx )
         {
           Zi = (double) (Y - Table[Iy-1][1]) / (Table[Iy][1] - Table[Iy-1][1]);
           LUTable = (double) ( Table[Iy-1][Nx] + (Table[Iy][Nx]-Table[Iy-1][Nx]) * Zi );
           return ( (double) LUTable );
         }

       if ( Iy>Ny )
         {
           Zi = (double) (X - Table[1][Ix-1]) / (Table[1][Ix] - Table[1][Ix-1]);
           LUTable = (double) ( Table[Ny][Ix-1] + (Table[Ny][Ix]-Table[Ny][Ix-1]) * Zi );
           return ( (double) LUTable );
         }

       Zi = (double) ( X - Table[1][Ix-1] ) / ( Table[1][Ix] - Table[1][Ix-1] );
       Tab1 = (double) ( Table[Iy-1][Ix-1] + (Table[Iy-1][Ix]-Table[Iy-1][Ix-1]) * Zi);
       Tab2 = (double) ( Table[Iy][Ix-1] + (Table[Iy][Ix]-Table[Iy][Ix-1]) * Zi);

       Zi = (double) (Y - Table[Iy-1][1]) / (Table[Iy][1] - Table[Iy-1][1]);
       LUTable = (double) ( Tab1 + (Tab2-Tab1) * Zi );

       return ( (double) LUTable );
    }

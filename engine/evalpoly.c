/*----------------------------------------------------------------------------*\
**                              evalpoly.c  
**
**  GDP Level 0 to 1 program EvalPolynom implementation file
**
**  Version:    4.1
**  Date:       29 Jul 1996
**  Contents:
**    This file contains the following library functions:
**
**    float EvalPolynom_f(float X, float *Coefficient, short Grad)
**    double EvalPolynom_d(double X, double *Coefficient, short Grad)
\*----------------------------------------------------------------------------*/
#pragma ident	"@(#)evalpoly.c	4.1 29 Jul 1996, Copyright DLR 1994"

// #include "utils.h"


/*----------------------------------------------------------------------------*\
**                                EvalPolynom_f
**  Input parameters:
**    X: The point to evaluate the polynom
**    Coefficient: Describe the polynom
**    Grad: grad of the polynom
**  Output parameters:
**  Other interfaces:
**  Description:
**    This function evaluates a polynom of grad Grad described by Coefficient 
**    in the value x
**  References:
**  Libraries:
**  Created:    22.3.94
**  Author: 	Diego Loyola, DLR/WT-DA-BS
\*----------------------------------------------------------------------------*/
float EvalPolynom_f(float X, const float *Coefficient, short Grad)
{
    float Result = 0.0, Mult = 1.0;
	short i;
 
    for (i=0; i<Grad; i++)
	{
        Result += Coefficient[i]*Mult;
        Mult *= X;
    }
    return Result;
}

/*----------------------------------------------------------------------------*\
**                                EvalPolynom_d
**  Input parameters:
**    X: The point to evaluate the polynom
**    Coefficient: Describe the polynom
**    Grad: grad of the polynom
**  Output parameters:
**  Other interfaces:
**  Description:
**    This function evaluates a polynom of grad Grad described by Coefficient 
**    in the value x
**  References:
**  Libraries:
**  Created:    22.3.94
**  Author: 	Diego Loyola, DLR/WT-DA-BS
\*----------------------------------------------------------------------------*/
double EvalPolynom_d(double X, const double *Coefficient, short Grad)
{
    double Result = 0.0, Mult = 1.0;
	short i;
 
    for (i=0; i<Grad; i++)
	{
        Result += Coefficient[i]*Mult;
        Mult *= X;
    }
    return Result;
}


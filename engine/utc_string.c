/*--------------------------------------------------------------------*\
**
** routines for creation of UTC String from MJD structure
** (musec are ignored)
**
** Author: S. Noel, IFE/IUP Uni Bremen
**        (Stefan.Noel@iup.physik.uni.bremen.de)
**
** Version: 0.1, 25 Mar 2002
**
** (This routine is based on "MJD_2_ASCII" from the NADC lib.)
**
\*--------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lv1_defs.h"
#include "lv1_struct.h"


/*+++++++++++++++++++++++++
.IDENTifer   CALDAT
.PURPOSE     return the calendar date for given julian date
.INPUT/OUTPUT
  call as    CALDAT( ijul, &iday, &imon, &iyear );

     input:
            unsigned int ijul     :  Julian day number (starts at noon)
    output:
            int  *iday  :   number of day of the month.
            int  *imon  :   number of the month (1 = january, ... )
            int  *iyear :   number of the year

.RETURNS     nothing
.COMMENTS    static function
             Taken from "Numerical Recipies in C", by William H. Press,
	     Brian P. Flannery, Saul A. Teukolsky, and William T. Vetterling.
	     Cambridge University Press, 1988 (second printing).

-------------------------*/
/*********** NOTE: This is taken from the NADC lib *************/
static
void CALDAT( unsigned int ijul, /*@out@*/ int *iday,
             /*@out@*/ int *imon, /*@out@*/ int *iyear )
{
     int ja, jalpha, jb, jc, jd, je;
/*
 * Gregorian Calander was adopted on Oct. 15, 1582
 */
     const unsigned int IGREG = 15U + 31U * (10U + 12U * 1582U);

     if ( ijul >= IGREG ) {
	  jalpha = (int) (((float) (ijul - 1867216) - 0.25)
			  / 36524.25);
	  ja = ijul + 1 + jalpha - (int) (0.25 * jalpha);
     } else {
	  ja = (int) ijul;
     }
     jb = ja + 1524;
     jc = (int)(6680.0 + ((float) (jb - 2439870) - 122.1) / 365.25);
     jd = (int)(365 * jc + (0.25 * jc));
     je = (int)((jb - jd) / 30.6001);
     *iday = jb - jd - (int) (30.6001 * je);
     *imon = je - 1;
     if ( *imon > 12 ) *imon -= 12;
     *iyear = jc - 4715;
     if ( *imon > 2 ) --(*iyear);
     if ( *iyear <= 0 ) --(*iyear);
}


/*--------------------------------------------------------------------*\
**       create UTC String from MJD structure (musec are ignored)
** 	 (This routine is based on "MJD_2_ASCII" from the NADC lib.)
\*--------------------------------------------------------------------*/
void UTC_String(MJD* time, char* string)
{
  /*
   * Julian date at 01/01/2000
   */
  static const double jday_01012000 = 2451544.5;
  /*
   * array for month names
   */
  static const char *mon_str[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  int  iday, ihour, imin, imon, isec, iyear;
  unsigned int jday;

  /*
   * convert to julian day
   */

  jday = (int)(time->days + jday_01012000 + 0.5);
  CALDAT( jday, &iday, &imon, &iyear );

  ihour = (int)(time->secnd / 3600);
  isec  = (int) time->secnd - (3600 * ihour);
  imin  = isec / 60;
  isec -= 60 * imin;
  (void) sprintf( string, "%.2d-%3s-%.4d %.2d:%.2d:%.2d.%.6u", iday,
		  mon_str[imon-1], iyear, ihour, imin, isec, time->musec);

  return;
}


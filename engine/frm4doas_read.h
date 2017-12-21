
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS/FRM4DOAS
//  Module purpose    :  READ NETCDF FILES for FRM4DOAS PROJECT
//  Name of module    :  FRM4DOAS_READ.H
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//        BIRA-IASB
//        Belgian Institute for Space Aeronomy
//        Ringlaan 3 Avenue Circulaire
//        1180     UCCLE
//        BELGIUM
//        qdoas@aeronomie.be
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or (at
//  your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
//  ----------------------------------------------------------------------------

#ifndef FRM4DOAS_READ_H
#define FRM4DOAS_READ_H

#include "doas.h"

#ifdef __cplusplus
extern "C" {
#endif

  // int frm4doas_init(const char *ref_filename, ENGINE_CONTEXT *pEngineContext);
  //
  // int frm4doas_prepare_automatic_reference(ENGINE_CONTEXT *pEngineContext, void *responseHandle);
  //
  int frm4doas_set(ENGINE_CONTEXT *pEngineContext);
  int frm4doas_read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay);

  // int frm4doas_get_reference(const char* filename, int pixel, double *lambda, double *spectrum, double *sigma, int *n_wavel);
  //
  // int frm4doas_get_orbit_date(int *orbit_year, int *orbit_month, int *orbit_day);

  void frm4doas_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif

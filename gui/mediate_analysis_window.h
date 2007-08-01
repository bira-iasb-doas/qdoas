/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef _MEDIATE_ANALYSIS_WINDOW_H_GUARD
#define _MEDIATE_ANALYSIS_WINDOW_H_GUARD

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// mediate_analysis_window_t
//
// Contains all user-specified information about a single spectral analysis window.
// It allows the GUI to provide information to the engine.

typedef struct mediate_analysis_window
{
  // To Be Defined - Coupled to the control offered by the GUI Analysis Window Tabs and
  // the list of molecules/cross-sections/symbols?

  int todo;

} mediate_analysis_window_t;


#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
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


#include "CWProjectTabFiltering.h"

CWProjectTabFiltering::CWProjectTabFiltering(const mediate_project_filtering_t *lowpass,
					     const mediate_project_filtering_t *highpass, QWidget *parent) :
  QFrame(parent)
{
}

CWProjectTabFiltering::~CWProjectTabFiltering()
{
}

void CWProjectTabFiltering::apply(mediate_project_filtering_t *lowpass, mediate_project_filtering_t *highpass) const
{
}


//--------------------------------------------------------

CWBoxcarEdit::CWBoxcarEdit(const struct filter_boxcar *d, QWidget *parent)
{
}

CWBoxcarEdit::~CWBoxcarEdit()
{
}

void CWBoxcarEdit::apply(struct filter_boxcar *d) const
{
}


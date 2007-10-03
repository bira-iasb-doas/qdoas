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


#ifndef _CCONFIGURATIONWRITER_H_GUARD
#define _CCONFIGURATIONWRITER_H_GUARD

#include <cstdio>

#include <QTreeWidgetItem>

#include "mediate.h"

class CWProjectTree;

class CConfigurationWriter
{
 public:
  CConfigurationWriter(const CWProjectTree *projectTree);
  ~CConfigurationWriter();

  QString write(const QString &fileName);

 private:
  void writeProjects(FILE *fp);
  void writeProperties(FILE *fp, const mediate_project_t *properties);
  void writePropertiesSpectra(FILE *fp, const mediate_project_spectra_t *d);
  void writePropertiesAnalysis(FILE *fp, const mediate_project_analysis_t *d);
  void writePropertiesFilter(FILE *fp, const char *passband, const mediate_project_filtering_t *d);
  void writePropertiesCalibration(FILE *fp, const mediate_project_calibration_t *d);
  void writePropertiesUndersampling(FILE *fp, const mediate_project_undersampling_t *d);
  void writePropertiesInstrumental(FILE *fp, const mediate_project_instrumental_t *d);
  void writePropertiesSlit(FILE *fp, const mediate_project_slit_t *d);

  
  void writeRawSpectraTree(FILE *fp, const QTreeWidgetItem *rawSpectraItem);
  void writeSpectraTreeNode(FILE *fp, const QTreeWidgetItem *item, int depth);

  void writeAnalysisWindows(FILE *fp, const QString &projectName, const QTreeWidgetItem *item);

 private:
  const CWProjectTree *m_projectTree;
};

#endif
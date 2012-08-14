
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  Modify options defined in the xml configuration file
//  Name of module    :  QDOASXML.C
//  Program Language  :  C/C++
//  Creation date     :  13 August 2012
//
//  Author            :  Caroline FAYT
//
//        Copyright  (C) Belgian Institute for Space Aeronomy (BIRA-IASB)
//                       Avenue Circulaire, 3
//                       1180     UCCLE
//                       BELGIUM
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
//  ----------------------------------------------------------------------------
//
//  MODULE DESCRIPTION
//
//  Options for the analysis are retrieved from a QDOAS configuration file in
//  XML format.  Since August 2012, it is possible to modify the original
//  options using -xml switch in the doas_cl command line.  This module parses
//  the new user commands.  Syntax should be :
//
//     doas_cl .... -xml "<xml path>=<xml value>
//
//  where <xml path> : is a path defining the field to modify according to the
//                     xml file structure.  For example :
//
//    /project/analysis_window/files/refone should modify the reference file
//
//        <xml value> : is the new value to assign to the selected xml field
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  QDOASXML_Parse : module entry point to parse xml commands
//
//  ----------------------------------------------------------------------------

#include "qdoasxml.h"

RC ParseInstrumental(QStringList &xmlFields,int xmlFieldN,int startingField,QString *pXmlValue,const CProjectConfigItem *p)
 {
 	// Declarations

  mediate_project_t newProjectProperties;
 	int indexField;
 	RC  rc;

 	// Initializations

 	memcpy(&newProjectProperties,(mediate_project_t *)p->properties(),sizeof(mediate_project_t));
 	rc=ERROR_ID_NO;

 	for (indexField=startingField;indexField<xmlFieldN;indexField++)
 	 {
 	 	if (xmlFields.at(indexField)=="format")
 	 	 std::cout << "project/instrumental/format field can not be changed" << std::endl;
 	 	else if (xmlFields.at(indexField)=="site")
 	 	 std::cout << "project/instrumental/site field can not be changed" << std::endl;
 	 	else if (xmlFields.at(indexField)=="ascii")
 	 	 std::cout << "project/instrumental/ascii field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="logger")
 	 	 std::cout << "project/instrumental/logger field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="acton")
 	 	 std::cout << "project/instrumental/acton field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="pdaegg")
 	 	 std::cout << "project/instrumental/pdaegg field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="pdaeggold")
 	 	 std::cout << "project/instrumental/pdaeggold field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="ccdohp96")
 	 	 std::cout << "project/instrumental/ccdohp96 field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="ccdha94")
 	 	 std::cout << "project/instrumental/ccdha94 field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="saozvis")
 	 	 std::cout << "project/instrumental/saozvis field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="saozefm")
 	 	 std::cout << "project/instrumental/saozefm field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="mfc")
 	 	 std::cout << "project/instrumental/mfc field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="mfcbira")
 	 	 std::cout << "project/instrumental/mfcbira field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="mfcstd")
 	 	 std::cout << "project/instrumental/mfcstd field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="rasas")
 	 	 std::cout << "project/instrumental/rasas field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="pdasieasoe")
 	 	 std::cout << "project/instrumental/pdasieasoe field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="ccdeev")
 	 	 std::cout << "project/instrumental/ccdeev field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="gdpascii")
 	 	 std::cout << "project/instrumental/gdpascii field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="gdpbin")
 	 	 std::cout << "project/instrumental/gdpbin field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="sciahdf")
 	 	 std::cout << "project/instrumental/sciahdf field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="sciapds")
 	 	 std::cout << "project/instrumental/sciapds field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="uoft")
 	 	 std::cout << "project/instrumental/uoft field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="noaa")
 	 	 std::cout << "project/instrumental/noaa field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="omi")
 	 	 {
 	 	 	if (indexField+1>=xmlFieldN)
 	 	 	 std::cout << "omi attribute is missing" << std::endl;
 	 	 	else if (xmlFields.at(indexField+1)=="type")
 	 	 	 std::cout << "project/instrumental/omi/type field can not be changed" << std::endl;
 	 	 	else if (xmlFields.at(indexField+1)=="min")
 	 	 	 std::cout << "project/instrumental/omi/min field can not be changed" << std::endl;
 	 	 	else if (xmlFields.at(indexField+1)=="max")
 	 	 	 std::cout << "project/instrumental/omi/max field can not be changed" << std::endl;
 	 	 	else if (xmlFields.at(indexField+1)=="ave")
 	 	 	 std::cout << "project/instrumental/omi/ave field can not be changed" << std::endl;
 	 	 	else if (xmlFields.at(indexField+1)=="trackSelection")
 	 	 	 {
	       std::cout << "project/instrumental/omi/trackSelection : " << newProjectProperties.instrumental.omi.trackSelection << " replaced by " << pXmlValue->toAscii().constData() << std::endl;
	       strcpy(newProjectProperties.instrumental.omi.trackSelection,pXmlValue->toAscii().constData());
        p->SetProperties(( mediate_project_t *)&newProjectProperties);
 	 	 	 }
 	 	 	else if (xmlFields.at(indexField+1)=="calib")
 	 	 	 std::cout << "project/instrumental/omi/calib field can not be changed" << std::endl;
 	 	 	else if (xmlFields.at(indexField+1)=="instr")
 	 	 	 std::cout << "project/instrumental/omi/instr field can not be changed yet" << std::endl;
 	 	 }
 	 	else if (xmlFields.at(indexField)=="gome2")
 	 	 std::cout << "project/instrumental/gome2 field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="mkzy")
 	 	 std::cout << "project/instrumental/mkzy field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="biraairborne")
 	 	 std::cout << "project/instrumental/biraairborne field can not be changed yet" << std::endl;
 	 	else if (xmlFields.at(indexField)=="oceanoptics")
 	 	 std::cout << "project/instrumental/oceanoptics field can not be changed yet" << std::endl;
   }

  // Return

  return rc;
 }

RC ParseAnalysisWindow(QStringList &xmlFields,int xmlFieldN,int startingField,QString *pXmlValue,const CProjectConfigItem *p)
 {
 	// Declarations

  const QList<const CAnalysisWindowConfigItem*> awList = p->analysisWindowItems();
  mediate_analysis_window_t newAnalysisProperties;
  QList<const CAnalysisWindowConfigItem*>::const_iterator awIt = awList.begin();

 	int indexField;
 	int displayField;
 	int filesField;
 	RC  rc;

 	// Initializations

 	displayField=filesField=0;
 	rc=ERROR_ID_NO;

 	for (indexField=startingField;indexField<xmlFieldN;indexField++)
 	 {
 	 	// top attributes

 	 	if (!displayField && !filesField)
 	 	 {
 	 	  if (xmlFields.at(indexField)=="disable")
 	 	   std::cout << "project/analysis_window/disable can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="kurucz")
 	 	   std::cout << "project/analysis_window/kurucz can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="refsel")
 	 	   std::cout << "project/analysis_window/refsel can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="min")
 	 	   std::cout << "project/analysis_window/min can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="max")
 	 	   std::cout << "project/analysis_window/max can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="display")
 	 	   displayField=1;
 	 	  else if (xmlFields.at(indexField)=="files")
 	 	   filesField=1;
 	 	 }

 	 	// display section

 	 	else if (displayField)
 	 	 {
 	 	  if (xmlFields.at(indexField)=="spectrum")
 	 	   std::cout << "project/analysis_window/display/spectrum can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="poly")
 	 	   std::cout << "project/analysis_window/display/poly can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="fits")
 	 	   std::cout << "project/analysis_window/display/fits can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="residual")
 	 	   std::cout << "project/analysis_window/display/residual can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="predef")
 	 	   std::cout << "project/analysis_window/display/predef can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="ratio")
 	 	   std::cout << "project/analysis_window/display/ratio can not be changed yet" << std::endl;
 	 	 }

 	 	// files section

 	 	else if (filesField)
 	 	 {
 	 	 	// Reference 1 field

 	 	  if (xmlFields.at(indexField)=="refone")
 	 	   {
        while (awIt!=awList.end())
         {
          memcpy(&newAnalysisProperties,(mediate_analysis_window_t *)((*awIt)->properties()),sizeof(mediate_analysis_window_t));
          std::cout << "project/analysis_window/files/refone : " << newAnalysisProperties.refOneFile << " replaced by "
                    << pXmlValue->toAscii().constData() << " (" << (*awIt)->name().toAscii().constData() << " window)" << std::endl;
          strcpy(newAnalysisProperties.refOneFile,pXmlValue->toAscii().constData());
          (*awIt)->SetProperties(( mediate_analysis_window_t *)&newAnalysisProperties);
          ++awIt;
         }
 	 	   }
 	 	  else if (xmlFields.at(indexField)=="reftwo")
 	 	   std::cout << "project/analysis_window/files/reftwo can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="residual")
 	 	   std::cout << "project/analysis_window/files/residual can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="szacenter")
 	 	   std::cout << "project/analysis_window/files/szacenter can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="szadelta")
 	 	   std::cout << "project/analysis_window/files/szadelta can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="minlon")
 	 	   std::cout << "project/analysis_window/files/minlon can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="maxlon")
 	 	   std::cout << "project/analysis_window/files/maxlon can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="minlat")
 	 	   std::cout << "project/analysis_window/files/minlat can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="maxlat")
 	 	   std::cout << "project/analysis_window/files/maxlat can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="refns")
 	 	   std::cout << "project/analysis_window/files/refns can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="cloudfmin")
 	 	   std::cout << "project/analysis_window/files/cloudfmin can not be changed yet" << std::endl;
 	 	  else if (xmlFields.at(indexField)=="cloudfmax")
 	 	   std::cout << "project/analysis_window/files/cloudfmax can not be changed yet" << std::endl;
 	 	 }
 	 }


  // Return

  return rc;
 }

RC QDOASXML_Parse(QList<QString> &xmlCommands,const CProjectConfigItem *p)
 {
  QList<QString>::const_iterator it = xmlCommands.begin();

  QString newXmlCmd;
  QStringList xmlParts;
  QStringList xmlFields;
  QString xmlKey,xmlValue;

  int xmlFieldsN,indexField,projectField,analysisField;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  while ((it!=xmlCommands.end()) && !rc)
   {
    newXmlCmd=*it;
    xmlParts=newXmlCmd.split("=");
    if (xmlParts.size()==2)
     {
     	xmlKey=xmlParts.at(0);
     	xmlValue=xmlParts.at(1);

     	xmlFields=xmlKey.split("/");
     	xmlFieldsN=xmlFields.size();

     	projectField=analysisField=0;

     	for (indexField=0;(indexField<xmlFieldsN) && !rc;indexField++)
     	 {
        if (projectField)
         {
         	if (xmlFields.at(indexField)=="instrumental")
         	 rc=ParseInstrumental(xmlFields,xmlFieldsN,indexField+1,&xmlValue,p);
         	else if (xmlFields.at(indexField)=="analysis_window")
     	 	   rc=ParseAnalysisWindow(xmlFields,xmlFieldsN,indexField+1,&xmlValue,p);

     	 	  break;
         }
     	 	else if (xmlFields.at(indexField)=="project")
     	 	 projectField=1;
     	 }
     }


    ++it;
   }

  return rc;
 }


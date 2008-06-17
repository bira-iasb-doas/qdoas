
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  OBSERVATION SITES MODULE
//  Name of module    :  WINSITES.C
//  Creation date     :  January 1997
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
//  This module handles the list of observation sites that are provided in the
//  Environment space tree.  The observation sites are useful to correct the
//  solar zenith angles (ground-based measurements) or to select measurements
//  above selected locations (overpasses for balloons and satellites measurements).
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  ===============
//  LIST PROCESSING
//  ===============
//
//  SITES_GetIndex - get the index of a site from its name;
//
//  ============================
//  OBSERVATION SITES PROPERTIES
//  ============================
//
//  SitesDlgInit - initialize the observation sites dialog box with the information
//                 on the selected item in the 'environment space' tree;
//
//  SitesSet - update the properties of the selected observation site;
//  SITES_WndProc - dispatch messages from the observation sites dialog box;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  SITES_Alloc - allocate and initialize buffers for observation sites;
//  SITES_Free - release the buffers allocated for the observation sites;
//
//  =============================
//  CONFIGURATION FILE MANAGEMENT
//  =============================
//
//  SITES_ResetConfiguration - remove the current list of observation sites from
//                             the 'Environment space' tree;
//
//  SITES_LoadConfiguration - load observation sites from the wds configuration file;
//  SITES_SaveConfiguration - save all the information about observation sites
//                            in the wds configuration file
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "windoas.h"

// ================
// GLOBAL VARIABLES
// ================

OBSERVATION_SITE *SITES_itemList=NULL;                                          // pointer to the list of sites objects
INT SITES_itemN=0;                                                              // the number of items in the previous list

// ================
// STATIC VARIABLES
// ================

// QDOAS ??? static OBSERVATION_SITE *pSITES_selected=NULL;                                  // pointer to the selected observation site

// ===============
// LIST PROCESSING
// ===============

// -----------------------------------------------------------------------------
// FUNCTION        SITES_GetIndex
// -----------------------------------------------------------------------------
// PURPOSE         Get the index of a site from its name
//
// INPUT           siteName : the name of the observation site
//
// RETURN          the index of this site in the observation sites list
//                 ITEM_NONE if not found
// -----------------------------------------------------------------------------

INDEX SITES_GetIndex(DoasCh *siteName)
 {
  // Declarations

  SZ_LEN siteLength;                                                            // the length of the observation site name
  INDEX indexSite;                                                              // browse observation sites in the list

  // Initializations

  siteLength=strlen(siteName);

  // Search for the site in the observation sites list

  for (indexSite=0;indexSite<SITES_itemN;indexSite++)
   if (((SZ_LEN)strlen(SITES_itemList[indexSite].name)==siteLength) &&
       !STD_Stricmp(siteName,SITES_itemList[indexSite].name))
    break;

  // return

  return (indexSite<SITES_itemN)?indexSite:ITEM_NONE;
 }

// QDOAS ??? // ============================
// QDOAS ??? // OBSERVATION SITES PROPERTIES
// QDOAS ??? // ============================
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SitesDlgInit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Initialize the observation sites dialog box with the information
// QDOAS ??? //                 on the selected item in the 'environment space' tree
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndSites : the handle of the observation sites dialog box
// QDOAS ??? //                 indexSite : the index of the selected item in the tree
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void SitesDlgInit(HWND hwndSites,INDEX indexSite)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   OBSERVATION_SITE *pSite;                                                      // pointer to the selected observation site
// QDOAS ???   DoasCh textTitle[MAX_ITEM_TEXT_LEN+1],                                         // complete title of the dialog box
// QDOAS ???         textMsg[MAX_ITEM_TEXT_LEN+1],                                           // title of the dialog box retrieved from the resource file
// QDOAS ???         string[MAX_ITEM_TEXT_LEN+1];                                            // formatted string
// QDOAS ???
// QDOAS ???   // Center dialog box on parent windows
// QDOAS ???
// QDOAS ???   DOAS_CenterWindow(hwndSites,GetWindow(hwndSites,GW_OWNER));
// QDOAS ???
// QDOAS ???   // Get the selected observation site
// QDOAS ???
// QDOAS ???   if ((indexSite!=ITEM_NONE) &&
// QDOAS ???      ((indexSite=TREE_itemList[indexSite].dataIndex)!=ITEM_NONE))
// QDOAS ???    {
// QDOAS ???     pSite=pSITES_selected=&SITES_itemList[indexSite];
// QDOAS ???
// QDOAS ???     // Set window title
// QDOAS ???
// QDOAS ???     LoadString(DOAS_hInst,IDS_TITLE_OBSERVATION_SITES,textMsg,MAX_ITEM_TEXT_LEN);
// QDOAS ???     sprintf(textTitle,"%s : %s",textMsg,pSite->name);
// QDOAS ???     SetWindowText(hwndSites,textTitle);
// QDOAS ???
// QDOAS ???     SendDlgItemMessage(hwndSites,SITES_ABBREVIATION,EM_SETLIMITTEXT,(WPARAM)MAX_SITES_ABBREVIATION,(LPARAM)0);
// QDOAS ???     SetWindowText(GetDlgItem(hwndSites,SITES_ABBREVIATION),pSite->abbrev);
// QDOAS ???
// QDOAS ???     // Longitude
// QDOAS ???
// QDOAS ???     sprintf(string,"%.3lf",pSite->longitude);
// QDOAS ???     SetWindowText(GetDlgItem(hwndSites,SITES_DoasI32ITUDE),string);
// QDOAS ???
// QDOAS ???     // Latitude
// QDOAS ???
// QDOAS ???     sprintf(string,"%.3lf",pSite->latitude);
// QDOAS ???     SetWindowText(GetDlgItem(hwndSites,SITES_LATITUDE),string);
// QDOAS ???
// QDOAS ???     // Altitude
// QDOAS ???
// QDOAS ???     sprintf(string,"%.3lf",pSite->altitude);
// QDOAS ???     SetWindowText(GetDlgItem(hwndSites,SITES_ALTITUDE),string);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???

// -----------------------------------------------------------------------------
// FUNCTION        SITES_Add
// -----------------------------------------------------------------------------
// PURPOSE         Add a site in the list
//
// INPUT           pNewSite           pointer to the site to add
//
// RETURN          ERROR_ID_BUFFER_FULL if the maximum number of sites is reached
//                 ERROR_ID_NO otherwise
// -----------------------------------------------------------------------------

RC SITES_Add(OBSERVATION_SITE *pNewSite)
 {
 	// Declaration

 	RC rc;

 	// Initialization

 	rc=ERROR_ID_NO;

 	// Check for the buffer limits

  if (SITES_itemN==MAX_SITES)
   rc=ERROR_SetLast("SITES_Add",ERROR_TYPE_FATAL,ERROR_ID_BUFFER_FULL,"sites");
  else
   {
   	memcpy(&SITES_itemList[SITES_itemN],pNewSite,sizeof(OBSERVATION_SITE));
   	SITES_itemN++;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION        SITES_Alloc
// -----------------------------------------------------------------------------
// PURPOSE         Allocate and initialize buffers for observation sites
//
// RETURN          ERROR_ID_ALLOC if one of the buffer allocation failed
//                 ERROR_ID_NO in case of success
// -----------------------------------------------------------------------------

RC SITES_Alloc(void)
 {
  // Declaration

  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  // Buffers allocation and initialization

  if ((SITES_itemList=(OBSERVATION_SITE *)MEMORY_AllocBuffer("SITES_Alloc","SITES_itemList",MAX_SITES,sizeof(OBSERVATION_SITE),0,MEMORY_TYPE_STRUCT))==NULL)
   rc=ERROR_ID_ALLOC;
  else
   memset(SITES_itemList,0,sizeof(OBSERVATION_SITE)*MAX_SITES);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION        SITES_Free
// -----------------------------------------------------------------------------
// PURPOSE         Release the buffers allocated for the observation sites
// -----------------------------------------------------------------------------

void SITES_Free(void)
 {
  if (SITES_itemList)
   MEMORY_ReleaseBuffer("SITES_Free","SITES_itemList",SITES_itemList);

  SITES_itemList=NULL;                                                          // pointer to the list of sites objects
  SITES_itemN=0;                                                                // the number of items in the previous list
 }



//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
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

OBSERVATION_SITE *SITES_itemList=NULL,                                          // pointer to the list of sites objects
                 *SITES_toPaste=NULL;                                           // structure storing the list of observation sites to paste
INDEX SITES_treeEntryPoint;                                                     // entry point in the 'environment space' tree for sites objects

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

INDEX SITES_GetIndex(UCHAR *siteName)
 {
  // Declarations

  SZ_LEN siteLength;                                                            // the length of the observation site name
  INT sitesNumber;                                                              // the number of observation sites in the list
  INDEX indexSite;                                                              // browse observation sites in the list

  // Initializations

  sitesNumber=TREE_itemType[TREE_ITEM_TYPE_SITE_CHILDREN].dataNumber;           // QDOAS ???
  siteLength=strlen(siteName);

  // Search for the site in the observation sites list

  for (indexSite=0;indexSite<sitesNumber;indexSite++)
   if (((int)strlen(SITES_itemList[indexSite].name)==siteLength) &&
       !STD_Stricmp(siteName,SITES_itemList[indexSite].name))
    break;

  // return

  return (indexSite<sitesNumber)?indexSite:ITEM_NONE;
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
// QDOAS ???   UCHAR textTitle[MAX_ITEM_TEXT_LEN+1],                                         // complete title of the dialog box
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
// QDOAS ???     SetWindowText(GetDlgItem(hwndSites,SITES_LONGITUDE),string);
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
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SitesSet
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Update the properties of the selected observation site
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndSites : the handle of the observation sites dialog box
// QDOAS ??? //                 pSite     : pointer to the new options
// QDOAS ??? //
// QDOAS ??? // OUTPUT          the selected item in the 'environment space' tree is modified
// QDOAS ??? //
// QDOAS ??? // RETURN          IDS_MSGBOX_FIELDEMPTY if one of the field is empty
// QDOAS ??? //                 IDS_MSGBOX_OUTOFRANGE if one of the input is out of range
// QDOAS ??? //                 IDS_MSGBOX_POSITIVE if one of the field should be positive
// QDOAS ??? //                 ERROR_ID_NO otherwise
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? 
// QDOAS ??? RC SitesSet(HWND hwndSites,OBSERVATION_SITE *pSite)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ??? 
// QDOAS ???   OBSERVATION_SITE site;                                                        // the new observation sites properties
// QDOAS ???   UCHAR abbreviation[MAX_SITES_ABBREVIATION+1],                                 // new site abbreviation
// QDOAS ???         string[MAX_ITEM_TEXT_LEN+1];                                            // string to retrieve the values of float fields
// QDOAS ???   RC rc;                                                                        // return code
// QDOAS ??? 
// QDOAS ???   // Initializations
// QDOAS ??? 
// QDOAS ???   memset(&site,0,sizeof(OBSERVATION_SITE));
// QDOAS ??? 
// QDOAS ???   site.altitude=
// QDOAS ???   site.longitude=
// QDOAS ???   site.latitude=(double)0;
// QDOAS ??? 
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ??? 
// QDOAS ???   if (pSite!=NULL)
// QDOAS ???    {
// QDOAS ???    	// Check the abbreviation field (useful for automatic output of overpasses)
// QDOAS ??? 
// QDOAS ???     if (!GetWindowText(GetDlgItem(hwndSites,SITES_ABBREVIATION),abbreviation,MAX_SITES_ABBREVIATION+1))
// QDOAS ???      MSG_MessageBox(hwndSites,SITES_ABBREVIATION,IDS_TITLE_OBSERVATION_SITES,
// QDOAS ???                    (rc=IDS_MSGBOX_FIELDEMPTY),MB_OK|MB_ICONHAND,"Site name abbreviation");
// QDOAS ??? 
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???      	// Get the name of the observation site
// QDOAS ??? 
// QDOAS ???       strcpy(site.name,pSite->name);
// QDOAS ??? 
// QDOAS ???       // Get the longitude, latitude and altitude
// QDOAS ??? 
// QDOAS ???       GetWindowText(GetDlgItem(hwndSites,SITES_LONGITUDE),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???       site.longitude=atof(string);
// QDOAS ???       GetWindowText(GetDlgItem(hwndSites,SITES_LATITUDE),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???       site.latitude=atof(string);
// QDOAS ???       GetWindowText(GetDlgItem(hwndSites,SITES_ALTITUDE),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???       site.altitude=atof(string);
// QDOAS ??? 
// QDOAS ???       // Get the abbreviation
// QDOAS ??? 
// QDOAS ???       strcpy(site.abbrev,abbreviation);
// QDOAS ??? 
// QDOAS ???       // Verify the geolocation
// QDOAS ??? 
// QDOAS ???       if (fabs(site.longitude)>(double)180.)
// QDOAS ???        MSG_MessageBox(hwndSites,SITES_LONGITUDE,IDS_TITLE_OBSERVATION_SITES,
// QDOAS ???                      (rc=IDS_MSGBOX_OUTOFRANGE),MB_OK|MB_ICONHAND,"longitude",-180,180);
// QDOAS ???       else if (fabs(site.latitude)>(double)90.)
// QDOAS ???        MSG_MessageBox(hwndSites,SITES_LATITUDE,IDS_TITLE_OBSERVATION_SITES,
// QDOAS ???                      (rc=IDS_MSGBOX_OUTOFRANGE),MB_OK|MB_ICONHAND,"latitude",-90,90);
// QDOAS ???       else if (site.altitude<(double)0.)
// QDOAS ???        MSG_MessageBox(hwndSites,SITES_ALTITUDE,IDS_TITLE_OBSERVATION_SITES,
// QDOAS ???                      (rc=IDS_MSGBOX_POSITIVE),MB_OK|MB_ICONHAND,"altitude");
// QDOAS ??? 
// QDOAS ???       if (!rc)
// QDOAS ???        {
// QDOAS ???        	// Update the observation site in the 'Environment space' tree
// QDOAS ??? 
// QDOAS ???         memcpy(pSite,&site,sizeof(OBSERVATION_SITE));
// QDOAS ??? 
// QDOAS ???         // Activate the 'Save' button in the toolbar
// QDOAS ??? 
// QDOAS ???         if (!TLBAR_bSaveFlag)
// QDOAS ???          TLBAR_Enable(TRUE);
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ??? 
// QDOAS ???   // Return
// QDOAS ??? 
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ??? 
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SITES_WndProc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         dispatch messages from the observation sites dialog box
// QDOAS ??? //
// QDOAS ??? // INPUT           usual syntax for windows messages routines
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? 
// QDOAS ??? LRESULT CALLBACK SITES_WndProc(HWND hwndSites,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   if (SITES_itemList!=NULL)
// QDOAS ???    {
// QDOAS ???     switch (msg)
// QDOAS ???      {
// QDOAS ???    // --------------------------------------------------------------------------
// QDOAS ???       case WM_INITDIALOG :
// QDOAS ???        SitesDlgInit(hwndSites,(INDEX)mp2);
// QDOAS ???       break;
// QDOAS ???    // --------------------------------------------------------------------------
// QDOAS ???       case WM_COMMAND :
// QDOAS ???        {
// QDOAS ???         ULONG commandID;
// QDOAS ??? 
// QDOAS ???         if ((((commandID=(ULONG)GET_WM_COMMAND_ID(mp1,mp2))==IDOK) &&           // close the dialog box on OK button command
// QDOAS ???               !SitesSet(hwndSites,pSITES_selected)) ||
// QDOAS ???               (commandID==IDCANCEL))                                            // close the dialog on Cancel button command
// QDOAS ??? 
// QDOAS ???          EndDialog(hwndSites,0);
// QDOAS ??? 
// QDOAS ???         else if (commandID==IDHELP)
// QDOAS ??? 
// QDOAS ???          WinHelp(hwndSites,DOAS_HelpPath,HELP_CONTEXT,HLP_WORKSPACE_OBSERVATION_SITES);
// QDOAS ???        }
// QDOAS ??? 
// QDOAS ???       break;
// QDOAS ???    // --------------------------------------------------------------------------
// QDOAS ???       case WM_DESTROY :
// QDOAS ???        pSITES_selected=NULL;
// QDOAS ???       break;
// QDOAS ???    // --------------------------------------------------------------------------
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ??? 
// QDOAS ??? #endif
// QDOAS ??? 
// QDOAS ??? // ====================
// QDOAS ??? // RESOURCES MANAGEMENT
// QDOAS ??? // ====================
// QDOAS ??? 
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SITES_Alloc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Allocate and initialize buffers for observation sites
// QDOAS ??? //
// QDOAS ??? // RETURN          ERROR_ID_ALLOC if one of the buffer allocation failed
// QDOAS ??? //                 ERROR_ID_NO in case of success
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? 
// QDOAS ??? RC SITES_Alloc(void)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ??? 
// QDOAS ???   RC rc;
// QDOAS ??? 
// QDOAS ???   // Initialization
// QDOAS ??? 
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ??? 
// QDOAS ???   // Buffers allocation and initialization
// QDOAS ??? 
// QDOAS ???   if (((TREE_itemType[TREE_ITEM_TYPE_SITE_CHILDREN].dataList=(VOID *)
// QDOAS ???         MEMORY_AllocBuffer("SITES_Alloc ","SITES_itemList",MAX_SITES,sizeof(OBSERVATION_SITE),0,MEMORY_TYPE_STRUCT))!=NULL) &&
// QDOAS ??? 
// QDOAS ???       ((TREE_itemType[TREE_ITEM_TYPE_SITE_CHILDREN].data2Paste=(VOID *)
// QDOAS ???         MEMORY_AllocBuffer("SITES_Alloc ","SITES_toPaste",MAX_SITES,sizeof(OBSERVATION_SITE),0,MEMORY_TYPE_STRUCT))!=NULL))
// QDOAS ???    {
// QDOAS ???     SITES_itemList=(OBSERVATION_SITE *)TREE_itemType[TREE_ITEM_TYPE_SITE_CHILDREN].dataList;
// QDOAS ???     SITES_toPaste=(OBSERVATION_SITE *)TREE_itemType[TREE_ITEM_TYPE_SITE_CHILDREN].data2Paste;
// QDOAS ??? 
// QDOAS ???     memset(SITES_itemList,0,sizeof(OBSERVATION_SITE)*MAX_SITES);
// QDOAS ???    }
// QDOAS ???   else
// QDOAS ???    rc=ERROR_ID_ALLOC;
// QDOAS ??? 
// QDOAS ???   // Return
// QDOAS ??? 
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ??? 
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SITES_Free
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Release the buffers allocated for the observation sites
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? 
// QDOAS ??? void SITES_Free(void)
// QDOAS ???  {
// QDOAS ???   if (SITES_itemList)
// QDOAS ???    MEMORY_ReleaseBuffer("SITES_Free ","SITES_itemList",SITES_itemList);
// QDOAS ???   if (SITES_toPaste)
// QDOAS ???    MEMORY_ReleaseBuffer("SITES_Free ","SITES_toPaste",SITES_toPaste);
// QDOAS ???  }
// QDOAS ??? 
// QDOAS ??? // =============================
// QDOAS ??? // CONFIGURATION FILE MANAGEMENT
// QDOAS ??? // =============================
// QDOAS ??? 
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SITES_ResetConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         remove the current list of observation sites from the
// QDOAS ??? //                 environment space tree;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? 
// QDOAS ??? void SITES_ResetConfiguration(void)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ??? 
// QDOAS ???   CHILD_WINDOWS *pChild;
// QDOAS ??? 
// QDOAS ???   // Initialization
// QDOAS ??? 
// QDOAS ???   pChild=&CHILD_list[CHILD_WINDOW_ENVIRONMENT];
// QDOAS ??? 
// QDOAS ???   // Remove the list from the tree
// QDOAS ??? 
// QDOAS ???   TREE_DeleteChildList(SITES_treeEntryPoint);
// QDOAS ??? 
// QDOAS ???   #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???   TREE_UpdateItem(pChild->hwndTree,SITES_treeEntryPoint);
// QDOAS ???   #endif
// QDOAS ???  }
// QDOAS ??? 
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SITES_LoadConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Load observation sites from the wds configuration file
// QDOAS ??? //
// QDOAS ??? // INPUT           fileLine : a line from the 'observation sites' section in
// QDOAS ??? //                            the wds configuration file
// QDOAS ??? //
// QDOAS ??? // OUTPUT          the 'environment space' tree is completed with the new
// QDOAS ??? //                 observation sites
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? 
// QDOAS ??? void SITES_LoadConfiguration(UCHAR *fileLine)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ??? 
// QDOAS ???   UCHAR keyName[MAX_ITEM_TEXT_LEN+1],                                           // the name of the observation site
// QDOAS ???         abbreviation[MAX_ITEM_TEXT_LEN+1];                                      // the abbreviation of the observation site
// QDOAS ??? 
// QDOAS ???   INT longDeg,longMin,longSec,                                                  // longitude in degrees, minutes, seconds (old configuration file format)
// QDOAS ???       latDeg,latMin,latSec;                                                     // latitude in degrees, minutes, seconds (old configuration file format)
// QDOAS ??? 
// QDOAS ???   OBSERVATION_SITE site;                                                        // information on the new observation site
// QDOAS ???   CHILD_WINDOWS *pChild;                                                        // entry point for observation sites in the 'environment space' tree
// QDOAS ???   INDEX indexItem,indexData;                                                    // indexes resp. in the tree and in the observation sites list
// QDOAS ??? 
// QDOAS ???   // Initializations
// QDOAS ??? 
// QDOAS ???   pChild=&CHILD_list[CHILD_WINDOW_ENVIRONMENT];
// QDOAS ???   memset(abbreviation,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   memset(&site,0,sizeof(OBSERVATION_SITE));
// QDOAS ??? 
// QDOAS ???   site.longitude=
// QDOAS ???   site.latitude=
// QDOAS ???   site.altitude=(double)0.;
// QDOAS ??? 
// QDOAS ???   // The version of the WinDOAS configuration file is accounting for when
// QDOAS ???   // extracting the information on an observation site
// QDOAS ??? 
// QDOAS ???   if ( (((FILES_version>=HELP_VERSION_1_04) && (FILES_version<=HELP_VERSION_2_00) &&
// QDOAS ???          (STD_Sscanf(fileLine,"%[^'=']=%d,%d,%d,%d,%d,%d,%[^','],%lf,%d",site.name,
// QDOAS ???                                &longDeg,&longMin,&longSec,&latDeg,&latMin,&latSec,
// QDOAS ???                                 abbreviation,&site.altitude,&site.hidden)>=10)) ||
// QDOAS ??? 
// QDOAS ???         ((FILES_version<HELP_VERSION_1_04) &&
// QDOAS ???          (STD_Sscanf(fileLine,"%[^'=']=%[^','],%d,%d,%d,%d,%d,%d,%2s",
// QDOAS ???                                keyName,site.name,
// QDOAS ???                               &longDeg,&longMin,&longSec,&latDeg,&latMin,&latSec,
// QDOAS ???                                abbreviation)>=9)) ||
// QDOAS ??? 
// QDOAS ???         ((FILES_version>=HELP_VERSION_2_01) &&
// QDOAS ???          (STD_Sscanf(fileLine,"%[^'=']=%lf,%lf,%[^','],%lf,%d,%d",
// QDOAS ???                                 site.name,&site.longitude,&site.latitude,abbreviation,
// QDOAS ???                                &site.altitude,&site.gmtShift,&site.hidden)>=7))) &&
// QDOAS ??? 
// QDOAS ???   // Add the new item in the 'Environment space' tree
// QDOAS ??? 
// QDOAS ???        ((indexItem=TREE_InsertOneItem(pChild->hwndTree,(UCHAR *)site.name,(INDEX)SITES_treeEntryPoint,(int)TREE_ITEM_TYPE_SITE_CHILDREN,(UCHAR)0,(UCHAR)0,
// QDOAS ???                                      (UCHAR)((FILES_version>HELP_VERSION_1_05)?site.hidden:0)))!=ITEM_NONE) &&
// QDOAS ???        ((indexData=TREE_itemList[indexItem].dataIndex)!=ITEM_NONE))
// QDOAS ???    {
// QDOAS ???    	// Convert the longitude and the latitude given in degrees, minutes, seconds,
// QDOAS ???    	// into fractional degrees
// QDOAS ??? 
// QDOAS ???     if (FILES_version<=HELP_VERSION_1_05)
// QDOAS ???      site.hidden=0;
// QDOAS ???     if (FILES_version<=HELP_VERSION_2_00)
// QDOAS ???      {
// QDOAS ???       site.longitude=(double)abs(longDeg)+longMin/60.+longSec/3600.;
// QDOAS ???       site.latitude=(double)abs(latDeg)+latMin/60.+latSec/3600.;
// QDOAS ??? 
// QDOAS ???       if (longDeg<0)
// QDOAS ???        site.longitude=-site.longitude;
// QDOAS ???       if (latDeg<0)
// QDOAS ???        site.latitude=-site.latitude;
// QDOAS ???      }
// QDOAS ??? 
// QDOAS ???     strncpy(site.abbrev,abbreviation,MAX_SITES_ABBREVIATION);
// QDOAS ???     site.abbrev[MAX_SITES_ABBREVIATION]=0;
// QDOAS ??? 
// QDOAS ???     // Add the observation site in the list
// QDOAS ??? 
// QDOAS ???     memcpy(&SITES_itemList[indexData],&site,sizeof(OBSERVATION_SITE));
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ??? 
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SITES_SaveConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Save all the information about observation sites in the wds
// QDOAS ??? //                 configuration file
// QDOAS ??? //
// QDOAS ??? // INPUT           fp          : pointer to the current wds configuration file
// QDOAS ??? //                 sectionName : the name of the section for observation sites
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? 
// QDOAS ??? void SITES_SaveConfiguration(FILE *fp,UCHAR *sectionName)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ??? 
// QDOAS ???   OBSERVATION_SITE *pSite;                                                      // pointer to a site in the observation sites list
// QDOAS ???   INT sitesNumber;                                                              // the number of observation sites in the list
// QDOAS ???   INDEX indexSite;                                                              // browse observation sites in the list
// QDOAS ??? 
// QDOAS ???   // Print out the section name
// QDOAS ??? 
// QDOAS ???   fprintf(fp,"[%s]\n\n",sectionName);
// QDOAS ??? 
// QDOAS ???   // Save all the observation sites into the configuration file
// QDOAS ??? 
// QDOAS ???   sitesNumber=TREE_itemType[TREE_ITEM_TYPE_SITE_CHILDREN].dataNumber;
// QDOAS ??? 
// QDOAS ???   for (indexSite=0;indexSite<sitesNumber;indexSite++)
// QDOAS ???    {
// QDOAS ???     pSite=&SITES_itemList[indexSite];
// QDOAS ??? 
// QDOAS ???     fprintf(fp,"%s=%g,%g,%s,%g,%d,%d\n",
// QDOAS ???                 pSite->name,
// QDOAS ???                 pSite->longitude,
// QDOAS ???                 pSite->latitude,
// QDOAS ???                 pSite->abbrev,
// QDOAS ???                 pSite->altitude,
// QDOAS ???                 pSite->gmtShift,
// QDOAS ???                 pSite->hidden);
// QDOAS ???    }
// QDOAS ??? 
// QDOAS ???   fprintf(fp,"\n");
// QDOAS ???  }

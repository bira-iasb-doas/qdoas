
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
//  Module purpose    :  ANALYSIS MODULE
//  Name of module    :  WINANLYS.C
//  Program Language  :  Borland C++ 5.0 for Windows 95/NT
//
//  Author            :  Caroline FAYT
//
//        Copyright  (C) Belgian Institute for Space Aeronomy (IASB/BIRA)
//                       Avenue Circulaire, 3
//                       1180     UCCLE
//                       BELGIUM
//
//  As the WinDOAS software is distributed freely within the DOAS community, it
//  would be nice if BIRA-IASB Institute and the authors were mentioned at least
//  in acknowledgements of papers presenting results obtained with this program.
//
//  The source code is also available on request for use, modification and free
//  distribution but authors are not responsible of unexpected behaviour of the
//  program if changes have been made on the original code.
//
//  ----------------------------------------------------------------------------
//  FUNCTIONS
//
//  =============
//  MISCELLANEOUS
//  =============
//
//  ANLYS_ViewCrossSections - MENU_CONTEXT_CROSS command message processing
//
//  ===========================
//  COMBOBOX CONTROL PROCESSING
//  ===========================
//
//  AnlysComboboxFill - fill a combobox item in the selected line of ListView control;
//  AnlysComboboxUpdate - take a change in combobox item selection into account;
//
//  ====================
//  LIST VIEW PROCESSING
//  ====================
//
//  AnlysGetSelectedListViewItem - retrieve the selected item in ListView control;
//  AnlysInsertListViewItem - insert one item into the ListView control;
//  AnlysInsertAllListViewItems - insert all items in list into ListView control;
//
//  ===============
//  LIST PROCESSING
//  ===============
//
//  ANLYS_CopyItems - MENU_CONTEXT_COPY command message processing;
//  ANLYS_PasteItems - MENU_CONTEXT_PASTE command message processing;
//  AnlysCopyAllListItems - make a copy of all items in list owned by an analysis window;
//  ANLYS_ReleaseListItems - release access to items currently in use in list;
//  AnlysInsertListItem - insert a new item in list;
//  AnlysGetListItem - get index of item in list;
//  AnlysUpdateList - update list items with content of controls associated to the ListView;
//  AnlysInsertDefaultItem - insert a default list item in tab page;
//
//  ======================
//  TAB CONTROL PROCESSING
//  ======================
//
//  AnlysTabChange - TCN_SELCHANGE notification message processing;
//  AnlysTabInit - tab control initialization;
//
//  =================================================================
//  ANALYSIS WINDOWS PROPERTIES : CROSS SECTIONS SET PANEL PROCESSING
//  =================================================================
//
//  AnlysXsSetInit - WM_INITDIALOG message processing for the dialog box that appears
//                   to select cross sections to shift;
//  AnlysXsSetOK - IDOK command message processing for cross sections set panel;
//  AnlysXsSetCommand - WM_COMMAND message processing for cross sections set panel;
//  AnlysXsSetWndProc - dispatch messages from cross sections set panel;
//
//  =========================================================
//  ANALYSIS WINDOWS PROPERTIES : COMMAND MESSAGES PROCESSING
//  =========================================================
//
//  AnlysInsertOneListItem - MENU_CONTEXT_INSERT command message processing;
//  AnlysDeleteOneListItem - MENU_CONTEXT_DELETE command message processing;
//  AnlysProperties - MENU_CONTEXT_PROPERTIES command message processing;
//  AnlysPropertiesAmf - MENU_CONTEXT_PROPERTIES_AMF command message processing;
//  AnlysSetRefSelection - ANLYS_REF_SELECTION_AUTOMATIC/ANLYS_REF_SELECTION_FILE command message processing;
//  AnlysDisplayFits - ANLYS_DISPLAY_FITS command message processing;
//  AnlysGetRefFiles - ANLYS_REF_SPECTRUM_BUTTON/ANLYS_REF_ETALON_BUTTON command message processing;
//  AnlysResidualFile - ANLYS_RESIDUALS_BUTTON command message processing;
//  AnlysOK - IDOK command message processing;
//  AnlysCancel - IDCANCEL command message processing;
//
//  =================================================
//  ANALYSIS WINDOWS PROPERTIES : MESSAGES PROCESSING
//  =================================================
//
//  AnlysMeasureListItem - WM_MEASUREITEM message processing;
//  AnlysDrawItem - WM_DRAWITEM message processing;
//  AnlysDlgInit - WM_INITDIALOG message processing;
//  AnlysContext - WM_CONTEXTMENU message processing;
//  AnlysNotify - WM_NOTIFY message processing;
//  AnlysCommand - WM_COMMAND message processing;
//  AnlysDestroy - WM_DESTROY message processing;
//  AnlysHelp - WM_HELP command message processing;
//  ANLYS_WndProc - dispatch messages from analysis windows properties panel;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  ANLYS_Alloc - allocate and initialize buffers for analysis windows;
//  ANLYS_Free - release buffers allocated for analysis windows;
//
//  =============================
//  CONFIGURATION FILE MANAGEMENT
//  =============================
//
//  ANLYS_ResetConfiguration - remove the current list of analysis windows from the environment space tree;
//  ANLYS_OutputConfiguration - output the configuration of the selected analysis windows to a file;
//  ANLYS_LoadConfigurationOld - load analysis windows from the wds configuration file (old version);
//  ANLYS_LoadConfiguration - load analysis windows from the wds configuration file;
//  ANLYS_SaveConfiguration - analysis windows properties safe keeping;
//
//  ----------------------------------------------------------------------------

// ==============
// INCLUDE HEADER
// ==============

#include "windoas.h"

#define DEFAULT_MEASUREITEM_WIDTH 680
#define ANLYS_SECTION "Analysis windows"

// ================
// GLOBAL VARIABLES
// ================

ANALYSIS_WINDOWS  *ANLYS_windowsList=NULL,                                      // analysis windows list
                  *ANLYS_toPaste=NULL;
LIST_ITEM         *ANLYS_itemList=NULL,                                         // list of items in ListView control owned by tab pages
                  *anlysItemListToPaste=NULL;

// ================
// STATIC VARIABLES
// ================

INDEX              ANLYS_indexProject;                                          // index of the project parent
INDEX              ANLYS_indexSelected=ITEM_NONE;                               // index of the selected analysis window
INDEX              ANLYS_indexTabPage=TAB_TYPE_ANLYS_CROSS;                     // the current page of tab control in analysis window panel
INDEX              ANLYS_indexXsSet=ITEM_NONE;                                  // index of the selected item in list when building set of cross sections
INT                ANLYS_maxLines;                                              // maximum number of visible lines and columns in ListView
LIST_ITEM         *ANLYS_itemListBackup=NULL;                                   // backup of previous list
INT                ANLYS_itemFree=0;                                            // index of first free item in previous list
INT                ANLYS_maxTabPage;                                            // maximum tab pages to process
INT                ANLYS_RefSpectrumSelectionMode;                              // reference spectrum selection mode
INT                ANLYS_displayFits;                                           // fits display general flag

PROJECT *PRJCT_itemList=NULL; // QDOAS !!!

// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ??? HWND               ANLYS_hwnd=NULL;
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? INT                ANLYS_nItemsToPaste;
// QDOAS ???
// QDOAS ??? UCHAR *anlysKuruczModes[ANLYS_KURUCZ_MAX]={"None","Ref only","Spectra only","Ref+spectra"};
// QDOAS ??? UCHAR *ANLYS_refFileName,*ANLYS_refEtalonFileName,*ANLYS_residuals,*ANLYS_crossFileName,*ANLYS_amfFileName;
// QDOAS ???
// QDOAS ??? // ==================
// QDOAS ??? // STRUCTURES FILLING
// QDOAS ??? // ==================
// QDOAS ???
// QDOAS ??? // Columns description of the ListView in "cross sections" tab page
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? LIST_COLUMN crossSectionsColumns[COLUMN_CROSS_MAX]=
// QDOAS ???  {
// QDOAS ???   // Cross sections
// QDOAS ???
// QDOAS ???   {"Cross Sections"   , 90,LVCFMT_LEFT  ,""        ,0                              ,0,COMBOBOX_ITEM_TYPE_NONE       ,0,""},
// QDOAS ???   {"Diff/Orthog"      ,115,LVCFMT_LEFT  ,"combobox",CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_VSCROLL,0,COMBOBOX_ITEM_TYPE_ORTHOGONAL ,1,"None"},
// QDOAS ???   {"Interp/Convol"    ,115,LVCFMT_LEFT  ,"combobox",CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_VSCROLL,0,COMBOBOX_ITEM_TYPE_XS         ,1,"Interpolate"},
// QDOAS ???   {"AMF"              ,115,LVCFMT_LEFT  ,"combobox",CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_VSCROLL,0,COMBOBOX_ITEM_TYPE_AMF        ,1,"None"},
// QDOAS ???   {"Fit display"      , 80,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE       ,1,"1"},
// QDOAS ???   {"Filter"           , 50,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE       ,2,"1"},
// QDOAS ???   {"Cc Fit"           , 65,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE       ,2,"1."},
// QDOAS ???   {"Cc init"          , 65,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE       ,2,"0."},
// QDOAS ???   {"Cc Delta"         , 65,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE       ,2,"0.001"},
// QDOAS ???
// QDOAS ???   // Output
// QDOAS ???
// QDOAS ???   {"AMFs"             , 50,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE       ,3,"1"},
// QDOAS ???   {"Residual Col"     ,100,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE       ,3,"0."},
// QDOAS ???   {"Slnt Col"         , 55,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE       ,3,"1"},
// QDOAS ???   {"Slnt Err"         , 55,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE       ,3,"1"},
// QDOAS ???   {"Slnt Fact"        , 55,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE       ,3,"1."},
// QDOAS ???   {"Vrt Col"          , 55,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE       ,4,"1"},
// QDOAS ???   {"Vrt Err"          , 55,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE       ,4,"1"},
// QDOAS ???   {"Vrt Fact"         , 55,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE       ,4,"1."},
// QDOAS ???
// QDOAS ???   // Kurucz SVD matrix composition for reference alignment
// QDOAS ???
// QDOAS ???   {"Diff/Orthog"      ,115,LVCFMT_LEFT  ,"combobox",CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_VSCROLL,0,COMBOBOX_ITEM_TYPE_ORTHOGONAL ,5,"None"},
// QDOAS ???   {"Fit display"      , 80,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE       ,5,"1"},
// QDOAS ???
// QDOAS ???   {"Interp/Convol"    ,115,LVCFMT_LEFT  ,"combobox",CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_VSCROLL,0,COMBOBOX_ITEM_TYPE_XS,5,"Interpolate"},
// QDOAS ???
// QDOAS ???   {"Cc Fit"           , 60,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE       ,6,"1"},
// QDOAS ???   {"Cc init"          , 60,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE       ,6,"0."},
// QDOAS ???   {"Cc Delta"         , 60,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE       ,6,"0.001"},
// QDOAS ???
// QDOAS ???   // Kurucz output
// QDOAS ???
// QDOAS ???   {"Slnt Col"         , 55,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE       ,7,"1"},
// QDOAS ???   {"Slnt Err"         , 55,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE       ,7,"1"},
// QDOAS ???   {"Slnt Fact"        , 55,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE       ,7,"1."},
// QDOAS ???   {"Cc I0"            , 65,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE       ,2,"0."},
// QDOAS ???   {"Cc I0"            , 65,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE       ,6,"0."}
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? // Columns description of the ListView in "continuous functions" tab page
// QDOAS ???
// QDOAS ??? LIST_COLUMN continuousFunctionsColumns[COLUMN_CONTINUOUS_MAX]=
// QDOAS ???  {
// QDOAS ???   {"",60,LVCFMT_LEFT,"button",BS_AUTOCHECKBOX,1,COMBOBOX_ITEM_TYPE_NONE,1,"00"},
// QDOAS ???   {"",60,LVCFMT_LEFT,"button",BS_AUTOCHECKBOX,1,COMBOBOX_ITEM_TYPE_NONE,1,"00"},
// QDOAS ???   {"",60,LVCFMT_LEFT,"button",BS_AUTOCHECKBOX,1,COMBOBOX_ITEM_TYPE_NONE,1,"00"},
// QDOAS ???   {"",60,LVCFMT_LEFT,"button",BS_AUTOCHECKBOX,1,COMBOBOX_ITEM_TYPE_NONE,1,"00"},
// QDOAS ???   {"",60,LVCFMT_LEFT,"button",BS_AUTOCHECKBOX,1,COMBOBOX_ITEM_TYPE_NONE,1,"00"},
// QDOAS ???   {"",60,LVCFMT_LEFT,"button",BS_AUTOCHECKBOX,1,COMBOBOX_ITEM_TYPE_NONE,1,"00"}
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? LIST_COLUMN linearColumns[COLUMN_POLY_MAX]=
// QDOAS ???  {
// QDOAS ???  	{"Linear Parameters"    ,120,LVCFMT_LEFT  ,""        ,0              ,0,COMBOBOX_ITEM_TYPE_NONE,0,""},
// QDOAS ???  	{"Polynomial order"     ,120,LVCFMT_LEFT  ,"combobox",CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_VSCROLL,0,COMBOBOX_ITEM_TYPE_POLYNOME,1,"None"},
// QDOAS ???  	{"OrthoBase order"      ,120,LVCFMT_LEFT  ,"combobox",CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_VSCROLL,0,COMBOBOX_ITEM_TYPE_POLYNOME,1,"None"},
// QDOAS ???  	{"Fit store"            , 65,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX,1,COMBOBOX_ITEM_TYPE_NONE,1,"0"},
// QDOAS ???  	{"Err store"            , 65,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX,1,COMBOBOX_ITEM_TYPE_NONE,1,"0"}
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? // Columns description of the ListView in "predefined parameters" tab page
// QDOAS ???
// QDOAS ??? LIST_COLUMN notLinearColumns[COLUMN_OTHERS_MAX]=
// QDOAS ???  {
// QDOAS ???   {"NL Parameters"    ,100,LVCFMT_LEFT  ,""        ,0              ,0,COMBOBOX_ITEM_TYPE_NONE,0,""},
// QDOAS ???   {"Fit"              , 65,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX,1,COMBOBOX_ITEM_TYPE_NONE,1,"0"},
// QDOAS ???   {"Val init"         , 75,LVCFMT_LEFT  ,"edit"    ,ES_LEFT        ,0,COMBOBOX_ITEM_TYPE_NONE,1,"0."},
// QDOAS ???   {"Val delta"        , 75,LVCFMT_LEFT  ,"edit"    ,ES_LEFT        ,0,COMBOBOX_ITEM_TYPE_NONE,1,"0.001"},
// QDOAS ???   {"Fit store"        , 65,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX,1,COMBOBOX_ITEM_TYPE_NONE,1,"0"},
// QDOAS ???   {"Err store"        , 65,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX,1,COMBOBOX_ITEM_TYPE_NONE,1,"0"},
// QDOAS ???   {"Val min"          , 75,LVCFMT_LEFT  ,"edit"    ,ES_LEFT        ,0,COMBOBOX_ITEM_TYPE_NONE,2,"-99."},
// QDOAS ???   {"Val max"          , 75,LVCFMT_LEFT  ,"edit"    ,ES_LEFT        ,0,COMBOBOX_ITEM_TYPE_NONE,2,"99."}
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? // Columns description of the ListView in "Shift and Stretch" tab page
// QDOAS ???
// QDOAS ??? LIST_COLUMN shiftStretchColumns[COLUMN_SHST_MAX]=
// QDOAS ???  {
// QDOAS ???   {"Cross Sections and spectrum",160,LVCFMT_LEFT  ,""        ,0                              ,0,COMBOBOX_ITEM_TYPE_NONE   ,0,""},
// QDOAS ???   {"Shift fit"                  , 85,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE   ,1,"1"},
// QDOAS ???   {"Stretch fit"                , 85,LVCFMT_LEFT  ,"combobox",CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_VSCROLL,0,COMBOBOX_ITEM_TYPE_POLYNOME,1,"None"},
// QDOAS ???   {"Scaling fit"                , 85,LVCFMT_LEFT  ,"combobox",CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_VSCROLL,0,COMBOBOX_ITEM_TYPE_POLYNOME,1,"None"},
// QDOAS ???   {"Sh store"                   , 85,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE   ,2,"0"},
// QDOAS ???   {"St store"                   , 85,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE   ,2,"0"},
// QDOAS ???   {"Sc store"                   , 85,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE   ,2,"0"},
// QDOAS ???   {"Err store"                  , 85,LVCFMT_CENTER,"button"  ,BS_AUTOCHECKBOX                ,1,COMBOBOX_ITEM_TYPE_NONE   ,2,"0"},
// QDOAS ???   {"Sh init"                    , 75,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE   ,3,"0.00"},
// QDOAS ???   {"St init"                    , 65,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE   ,3,"0.00"},
// QDOAS ???   {"St init (2)"                , 65,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE   ,3,"0.00"},
// QDOAS ???   {"Sc init"                    , 65,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE   ,3,"0.00"},
// QDOAS ???   {"Sc init (2)"                , 65,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE   ,3,"0.00"},
// QDOAS ???   {"Sh delta"                   , 80,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE   ,4,"0.001"},
// QDOAS ???   {"St delta"                   , 65,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE   ,4,"0.001"},
// QDOAS ???   {"St delta (2)"               , 65,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE   ,4,"0.001"},
// QDOAS ???   {"Sc delta"                   , 65,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE   ,4,"0.001"},
// QDOAS ???   {"Sc delta (2)"               , 70,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE   ,4,"0.001"},
// QDOAS ???   {"Sh min"                     , 75,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE   ,5,"-99."},
// QDOAS ???   {"Sh max"                     , 75,LVCFMT_LEFT  ,"edit"    ,ES_LEFT                        ,0,COMBOBOX_ITEM_TYPE_NONE   ,5,"99."},
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? // Columns description of the ListView in "gaps" tab page
// QDOAS ???
// QDOAS ??? LIST_COLUMN gapsColumns[COLUMN_GAPS_MAX]=
// QDOAS ???  {
// QDOAS ???   {"Gap"           ,250,LVCFMT_LEFT,""    ,      0,0,COMBOBOX_ITEM_TYPE_NONE,0,"New gap"},
// QDOAS ???   {"Min Value (nm)",110,LVCFMT_LEFT,"edit",ES_LEFT,0,COMBOBOX_ITEM_TYPE_NONE,0,"0.0"},
// QDOAS ???   {"Max Value (nm)",110,LVCFMT_LEFT,"edit",ES_LEFT,0,COMBOBOX_ITEM_TYPE_NONE,1,"0.0"}
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // All tab pages
// QDOAS ???
// QDOAS ??? ANLYS_TAB_PAGE ANLYS_tabPages[TAB_TYPE_ANLYS_MAX]=
// QDOAS ???  {
// QDOAS ???   // Cross sections tab
// QDOAS ???   // ------------------
// QDOAS ???
// QDOAS ???   {
// QDOAS ???                             "Molecules",                                        // title of tab page
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                           crossSectionsColumns,                                 // list of columns to create in ListView control
// QDOAS ??? #endif
// QDOAS ???                               COLUMN_CROSS_MAX,                                 // number of columns in previous list
// QDOAS ???                                           NULL,                                 // description of lines
// QDOAS ???           TREE_ITEM_TYPE_CROSS_CHILDREN,NULL,0,                                 // type of symbol to use
// QDOAS ???                     NULL,ITEM_NONE,0,ITEM_NONE,                                 // data to complete by application
// QDOAS ???                      &SYMB_crossTreeEntryPoint,                                 // tree entry point for symbols
// QDOAS ???                                          0,1,2,                                 // columns display control
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                      HLP_ANLYS_TAB_CROSS_PAGE1,
// QDOAS ???                            HLP_CALIB_TAB_CROSS
// QDOAS ??? #endif
// QDOAS ???   },
// QDOAS ???
// QDOAS ???   // Linear parameters tab
// QDOAS ???   // ---------------------
// QDOAS ???
// QDOAS ???   {
// QDOAS ???                        "Linear parameters",                                     // title of tab page
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                                  linearColumns,                                 // list of columns to create in ListView control
// QDOAS ??? #endif
// QDOAS ???                                COLUMN_POLY_MAX,                                 // number of columns in previous list
// QDOAS ???                                           NULL,                                 // description of lines
// QDOAS ???           TREE_ITEM_TYPE_CROSS_CHILDREN,NULL,0,                                 // type of symbol to use
// QDOAS ???                     NULL,ITEM_NONE,0,ITEM_NONE,                                 // data to complete by application
// QDOAS ???                      &SYMB_crossTreeEntryPoint,                                 // tree entry point for symbols
// QDOAS ???                                          0,1,1,                                 // columns display control
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                       HLP_ANLYS_TAB_PREDEFINED,
// QDOAS ???                       HLP_CALIB_TAB_PREDEFINED
// QDOAS ??? #endif
// QDOAS ???   },
// QDOAS ???
// QDOAS ???   // Non linear parameters tab
// QDOAS ???   // -------------------------
// QDOAS ???
// QDOAS ???   {
// QDOAS ???                        "Non linear parameters",                                 // title of tab page
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                               notLinearColumns,                                 // list of columns to create in ListView control
// QDOAS ??? #endif
// QDOAS ???                              COLUMN_OTHERS_MAX,                                 // number of columns in previous list
// QDOAS ???                                           NULL,                                 // description of lines
// QDOAS ???           TREE_ITEM_TYPE_CROSS_CHILDREN,NULL,0,                                 // type of symbol to use
// QDOAS ???                     NULL,ITEM_NONE,0,ITEM_NONE,                                 // data to complete by application
// QDOAS ???                      &SYMB_crossTreeEntryPoint,                                 // tree entry point for symbols
// QDOAS ???                                          0,1,2,                                 // columns display control
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                       HLP_ANLYS_TAB_PREDEFINED,
// QDOAS ???                       HLP_CALIB_TAB_PREDEFINED
// QDOAS ??? #endif
// QDOAS ???   },
// QDOAS ???
// QDOAS ???   // Shift and Stretch tab
// QDOAS ???   // ---------------------
// QDOAS ???
// QDOAS ???   {
// QDOAS ???                            "Shift and Stretch",                                 // title of tab page
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                            shiftStretchColumns,                                 // list of columns to create in ListView control
// QDOAS ??? #endif
// QDOAS ???                                COLUMN_SHST_MAX,                                 // number of columns in previous list
// QDOAS ???                                           NULL,                                 // description of lines
// QDOAS ???           TREE_ITEM_TYPE_CROSS_CHILDREN,NULL,0,                                 // type of symbol to use
// QDOAS ???                     NULL,ITEM_NONE,0,ITEM_NONE,                                 // data to complete by application
// QDOAS ???                      &SYMB_crossTreeEntryPoint,                                 // tree entry point for symbols
// QDOAS ???                                          0,1,5,                                 // columns display control
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                            HLP_ANLYS_TAB_SHIFT,
// QDOAS ???                            HLP_CALIB_TAB_SHIFT
// QDOAS ??? #endif
// QDOAS ???   },
// QDOAS ???
// QDOAS ???   // Gaps
// QDOAS ???   // ----
// QDOAS ???
// QDOAS ???   {
// QDOAS ???                                         "Gaps",                                 // title of tab page
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                                    gapsColumns,                                 // list of columns to create in ListView control
// QDOAS ??? #endif
// QDOAS ???                                COLUMN_GAPS_MAX,                                 // number of columns in previous list
// QDOAS ???                                           NULL,                                 // description of lines
// QDOAS ???                     TREE_ITEM_TYPE_NONE,NULL,0,                                 // type of symbol to use
// QDOAS ???                     NULL,ITEM_NONE,0,ITEM_NONE,                                 // data to complete by application
// QDOAS ???                                           NULL,                                 // tree entry point for symbols
// QDOAS ???                                          0,1,1,                                 // columns display control
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                             HLP_ANLYS_TAB_GAPS,
// QDOAS ???                             HLP_CALIB_TAB_GAPS
// QDOAS ??? #endif
// QDOAS ???   },
// QDOAS ???
// QDOAS ???   // Output
// QDOAS ???   // ------
// QDOAS ???
// QDOAS ???   {
// QDOAS ???                                      "Outputs",                                 // title of tab page
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                           crossSectionsColumns,                                 // list of columns to create in ListView control
// QDOAS ??? #endif
// QDOAS ???                               COLUMN_CROSS_MAX,                                 // number of columns in previous list
// QDOAS ???                                           NULL,                                 // description of lines
// QDOAS ???           TREE_ITEM_TYPE_CROSS_CHILDREN,NULL,0,                                 // type of symbol to use
// QDOAS ???                     NULL,ITEM_NONE,0,ITEM_NONE,                                 // data to complete by application
// QDOAS ???                      &SYMB_crossTreeEntryPoint,                                 // tree entry point for symbols
// QDOAS ???                                          0,3,4,                                 // columns display control
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                          HLP_ANLYS_TAB_OUTPUTS,
// QDOAS ???                          HLP_CALIB_TAB_OUTPUTS
// QDOAS ??? #endif
// QDOAS ???   }
// QDOAS ???  };
// QDOAS ???
// QDOAS ??? // Comboboxes filling
// QDOAS ???
UCHAR *AnlysOrthogonal[ANLYS_ORTHOGONAL_TYPE_MAX]={"None","Differential XS"};
UCHAR *AnlysStretch[ANLYS_STRETCH_TYPE_MAX]={"None","1st order","2nd order"};
UCHAR *AnlysPolynome[ANLYS_POLY_TYPE_MAX]={"None","order 0","order 1","order 2","order 3","order 4","order 5"};
UCHAR *ANLYS_crossAction[ANLYS_CROSS_ACTION_MAX]={"None","Interpolate","Convolute Std","Convolute I0","Convolute Ring"}; /* "Detector t° dependent","Strato t° dependent",*/
UCHAR *ANLYS_amf[ANLYS_AMF_TYPE_MAX]={"None","SZA only","Climatology","Wavelength 1","Wavelength 2","Wavelength 3"};
// QDOAS ???
// QDOAS ??? void   AnlysInsertOneListItem(HWND hwndList,INDEX indexTabPage);
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? // =============
// QDOAS ??? // MISCELLANEOUS
// QDOAS ??? // =============
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      ANLYS_ViewCrossSections
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_CROSS command message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree    handle of the projects tree
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void ANLYS_ViewCrossSections(HWND hwndTree)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   TREE_ITEM_TYPE *pTreeType;
// QDOAS ???   ANALYSIS_WINDOWS *pAnlys;
// QDOAS ???   UCHAR fileName[MAX_ITEM_TEXT_LEN+1],windowTitle[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   LIST_ITEM *pList;
// QDOAS ???   INDEX indexItem,indexGraph,indexSymbol[MAX_GRAPH],indexProject;
// QDOAS ???   TREE_ITEM *pTree;
// QDOAS ???   MATRIX_OBJECT xs[MAX_GRAPH],*pXs;
// QDOAS ???   INT graphNumber,graphMaxV,graphMaxH;
// QDOAS ???   double lembdaMin,lembdaMax;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   memset(xs,0,sizeof(MATRIX_OBJECT)*MAX_GRAPH);
// QDOAS ???   pTreeType=&TREE_itemType[TREE_ITEM_TYPE_CROSS_CHILDREN];
// QDOAS ???
// QDOAS ???   // Get index of selected analysis window in list
// QDOAS ???
// QDOAS ???   if (((indexItem=(hwndTree==NULL)?ITEM_NONE:TREE_GetSelectedItem(hwndTree))!=ITEM_NONE) &&
// QDOAS ???       ((pTree=&TREE_itemList[indexItem])->dataType==TREE_ITEM_TYPE_ANALYSIS_CHILDREN) &&
// QDOAS ???       ((indexItem=pTree->dataIndex)>ITEM_NONE) && (indexItem<TREE_itemType[TREE_ITEM_TYPE_ANALYSIS_CHILDREN].dataNumber))
// QDOAS ???    {
// QDOAS ???     pAnlys=&ANLYS_windowsList[indexItem];
// QDOAS ???     indexProject=TREE_itemList[TREE_itemList[pTree->parentItem].parentItem].dataIndex;  // Grand-Father
// QDOAS ???
// QDOAS ???     lembdaMin=atof(pAnlys->lembdaMin);
// QDOAS ???     lembdaMax=atof(pAnlys->lembdaMax);
// QDOAS ???
// QDOAS ???     sprintf(windowTitle,"Cross sections used in %s analysis window of project %s",pAnlys->windowName,PRJCT_itemList[indexProject].name);
// QDOAS ???
// QDOAS ???     // First pass : search for cross sections to display;
// QDOAS ???
// QDOAS ???     for (indexItem=pAnlys->listEntryPoint[TAB_TYPE_ANLYS_CROSS],graphNumber=0;indexItem!=ITEM_NONE;indexItem=pList->indexPrevious)
// QDOAS ???      {
// QDOAS ???       pList=&ANLYS_itemList[indexItem];
// QDOAS ???
// QDOAS ???       if (strlen(pList->crossFileName))
// QDOAS ???        {
// QDOAS ???         FILES_RebuildFileName(fileName,pList->crossFileName,1);
// QDOAS ???
// QDOAS ???         if (!MATRIX_Load(fileName,&xs[graphNumber],0 /* line base */,0 /* column base */,0,0,
// QDOAS ???                          lembdaMin,lembdaMax,
// QDOAS ???                          0,   // no derivatives
// QDOAS ???                          1,   // reverse vectors if needed
// QDOAS ???                         "ANLYS_ViewCrossSections ") && (xs[graphNumber].nl>1) && (xs[graphNumber].nc>1))
// QDOAS ???          {
// QDOAS ???           indexSymbol[graphNumber]=SYMB_GetListIndex((SYMBOL *)pTreeType->dataList,      // symbols list to use
// QDOAS ???                                                                pTreeType->dataNumber,
// QDOAS ???                                                                fileName);
// QDOAS ???           graphNumber++;
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Second pass : plot cross sections
// QDOAS ???
// QDOAS ???     if (graphNumber)
// QDOAS ???      {
// QDOAS ???       DRAW_GetGraphDistribution(graphNumber,&graphMaxV,&graphMaxH);
// QDOAS ???
// QDOAS ???       for (indexGraph=0;indexGraph<graphNumber;indexGraph++)
// QDOAS ???        {
// QDOAS ???         pXs=&xs[indexGraph];
// QDOAS ???
// QDOAS ???         DRAW_Spectra(CHILD_WINDOW_SPECTRA,windowTitle,SYMB_itemCrossList[indexSymbol[indexGraph]].name,"Wavelength","cm**2 / molec",NULL,0,
// QDOAS ???                     (double)0.,(double)0.,(double)0.,(double)0.,
// QDOAS ???                      pXs->matrix[0],pXs->matrix[1],pXs->nl,DRAW_COLOR1,0,pXs->nl-1,PS_SOLID,NULL,
// QDOAS ???                      NULL,NULL,0,0,0,0,0,NULL,
// QDOAS ???                      indexGraph,graphMaxV,graphMaxH,(indexGraph==graphNumber-1)?1:0);
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   for (indexGraph=0;indexGraph<MAX_GRAPH;indexGraph++)
// QDOAS ???    MATRIX_Free(&xs[indexGraph],"ANLYS_ViewCrossSections");
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ===========================
// QDOAS ??? // COMBOBOX CONTROL PROCESSING
// QDOAS ??? // ===========================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysComboboxFill
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       fill a combobox item in the selected line of ListView control
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndCombobox           handle for combobox control
// QDOAS ??? //               comboboxItemType       type of combobox
// QDOAS ??? //               symbolList             list of available symbols
// QDOAS ??? //               symbolNumber           number of symbols in previous list
// QDOAS ??? //               symbolReferenceNumber  number of times symbols are referenced to
// QDOAS ??? //               refText                text of item in first column (reference text)
// QDOAS ??? //               itemText               text of item in the currently selected column
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysComboboxFill(HWND hwndCombobox,                                       // handle for combobox control
// QDOAS ???                        INT comboboxItemType,                                    // type of combobox
// QDOAS ???                        SYMBOL *symbolList,                                      // list of available symbols
// QDOAS ???                        INT symbolNumber,                                        // number of symbols in previous list
// QDOAS ???                        INT *symbolReferenceNumber,                              // number of times symbols are referenced to
// QDOAS ???                        UCHAR *refText,                                          // text of item in first column (reference text)
// QDOAS ???                        UCHAR *itemText)                                         // text of item in the currently selected column
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexSymbol,indexItem;
// QDOAS ???
// QDOAS ???   // Reset content of combobox control
// QDOAS ???
// QDOAS ???   SendMessage(hwndCombobox,CB_RESETCONTENT,0,0);
// QDOAS ???
// QDOAS ???   if (comboboxItemType&COMBOBOX_ITEM_TYPE_ORTHOGONAL)
// QDOAS ???    for (indexItem=0;indexItem<ANLYS_ORTHOGONAL_TYPE_MAX;indexItem++)
// QDOAS ???     SendMessage(hwndCombobox,CB_ADDSTRING,0,(LPARAM)AnlysOrthogonal[indexItem]);
// QDOAS ???
// QDOAS ???   if (comboboxItemType&COMBOBOX_ITEM_TYPE_POLYNOME)
// QDOAS ???
// QDOAS ???    if (!STD_Stricmp(refText,"Polynomial (x)") || !STD_Stricmp(refText,"Polynomial (1/x)"))
// QDOAS ???     for (indexItem=0;indexItem<ANLYS_POLY_TYPE_MAX;indexItem++)
// QDOAS ???      SendMessage(hwndCombobox,CB_ADDSTRING,0,(LPARAM)AnlysPolynome[indexItem]);
// QDOAS ???    else if (!STD_Stricmp(refText,"Offset"))
// QDOAS ???     for (indexItem=0;indexItem<ANLYS_POLY_TYPE_2;indexItem++)
// QDOAS ???      SendMessage(hwndCombobox,CB_ADDSTRING,0,(LPARAM)AnlysPolynome[indexItem]);
// QDOAS ???    else
// QDOAS ???     for (indexItem=0;indexItem<ANLYS_STRETCH_TYPE_MAX;indexItem++)
// QDOAS ???      SendMessage(hwndCombobox,CB_ADDSTRING,0,(LPARAM)AnlysStretch[indexItem]);
// QDOAS ???
// QDOAS ???   if (comboboxItemType&COMBOBOX_ITEM_TYPE_AMF)
// QDOAS ???    for (indexItem=0;indexItem<ANLYS_AMF_TYPE_MAX;indexItem++)
// QDOAS ???     SendMessage(hwndCombobox,CB_ADDSTRING,0,(LPARAM)ANLYS_amf[indexItem]);
// QDOAS ???
// QDOAS ???   if (comboboxItemType&COMBOBOX_ITEM_TYPE_XS)
// QDOAS ???    for (indexItem=0;indexItem<ANLYS_CROSS_ACTION_MAX;indexItem++)
// QDOAS ???     SendMessage(hwndCombobox,CB_ADDSTRING,0,(LPARAM)ANLYS_crossAction[indexItem]);
// QDOAS ???
// QDOAS ???   // Browse all available symbols and select the ones to insert in combobox
// QDOAS ???
// QDOAS ???   for (indexSymbol=0;indexSymbol<symbolNumber;indexSymbol++)
// QDOAS ???    if (((comboboxItemType&COMBOBOX_ITEM_TYPE_ORTHOGONAL) &&                     // for orthogonal combobox type
// QDOAS ???        ((symbolReferenceNumber[indexSymbol]>>8)&SYMBOL_ALREADY_USED) &&         // all symbol defined
// QDOAS ???          STD_Stricmp(symbolList[indexSymbol].name,refText)) ||                      // all symbols except the selected one
// QDOAS ???        ((comboboxItemType&COMBOBOX_ITEM_TYPE_EXCLUSIVE) &&                      // for not orthogonal combobox types
// QDOAS ???      (!((symbolReferenceNumber[indexSymbol]>>8)&SYMBOL_ALREADY_USED) ||         // only symbols that are not used yet
// QDOAS ???         !STD_Stricmp(symbolList[indexSymbol].name,itemText))))                      // plus the selected one
// QDOAS ???
// QDOAS ???      SendMessage(hwndCombobox,CB_ADDSTRING,0,(LPARAM)symbolList[indexSymbol].name);
// QDOAS ???
// QDOAS ???   // Fill edit part of the combobox control
// QDOAS ???
// QDOAS ???   if (strlen(itemText))
// QDOAS ???    SendMessage(hwndCombobox,CB_SELECTSTRING,-1,(LPARAM)itemText);
// QDOAS ???   else
// QDOAS ???    SendMessage(hwndCombobox,CB_SETCURSEL,0,0);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysComboboxUpdate
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       take a change in combobox item selection into account
// QDOAS ??? //
// QDOAS ??? // INPUT         comboboxItemType       type of combobox
// QDOAS ??? //               symbolList             list of available symbols
// QDOAS ??? //               symbolNumber           number of symbols in previous list
// QDOAS ??? //               symbolReferenceNumber  number of times symbols are referenced to
// QDOAS ??? //               oldSymbol              name of the symbol released
// QDOAS ??? //               newSymbol              name of the symbol reserved
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysComboboxUpdate(INT comboboxItemType,                                  // type of combobox
// QDOAS ???                          SYMBOL *symbolList,                                    // list of available symbols
// QDOAS ???                          INT symbolNumber,                                      // number of symbols in previous list
// QDOAS ???                          INT *symbolReferenceNumber,                            // number of times symbols are referenced to
// QDOAS ???                          UCHAR *oldSymbol,                                      // name of the symbol released
// QDOAS ???                          UCHAR *newSymbol)                                      // name of the symbol reserved
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexSymbol;
// QDOAS ???
// QDOAS ???   // Release access to symbol referenced by 'oldSymbol'
// QDOAS ???
// QDOAS ???   if (strlen(newSymbol) && ((indexSymbol=SYMB_GetListIndex(symbolList,symbolNumber,oldSymbol))!=ITEM_NONE) && (symbolReferenceNumber!=NULL))
// QDOAS ???    symbolReferenceNumber[indexSymbol]=
// QDOAS ???
// QDOAS ???     (comboboxItemType==COMBOBOX_ITEM_TYPE_ORTHOGONAL)?
// QDOAS ???      symbolReferenceNumber[indexSymbol]-1:
// QDOAS ???      symbolReferenceNumber[indexSymbol]&(~(SYMBOL_ALREADY_USED<<8));
// QDOAS ???
// QDOAS ???   // Get privilegied access to symbol referenced by 'newSymbol'
// QDOAS ???
// QDOAS ???   if (strlen(oldSymbol) && ((indexSymbol=SYMB_GetListIndex(symbolList,symbolNumber,newSymbol))!=ITEM_NONE) && (symbolReferenceNumber!=NULL))
// QDOAS ???    symbolReferenceNumber[indexSymbol]=
// QDOAS ???
// QDOAS ???     (comboboxItemType==COMBOBOX_ITEM_TYPE_ORTHOGONAL)?
// QDOAS ???      symbolReferenceNumber[indexSymbol]+1:
// QDOAS ???      symbolReferenceNumber[indexSymbol]|(SYMBOL_ALREADY_USED<<8);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ====================
// QDOAS ??? // LIST VIEW PROCESSING
// QDOAS ??? // ====================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysGetSelectedListViewItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Retrieve the selected item in ListView control
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndList       handle of the ListView controls
// QDOAS ??? //
// QDOAS ??? // OUTPUT        indexLvi       the index of the selected item
// QDOAS ??? //               itemText       the text of the selected item
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INDEX AnlysGetSelectedListViewItem(HWND hwndList,INDEX *indexLvi,UCHAR *itemText)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   LV_ITEM lvi;                                                                  // Attributes of the selected item in ListView control
// QDOAS ???   INDEX indexItem;                                                              // index of the selected item
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   memset(&lvi,0,sizeof(LV_ITEM));
// QDOAS ???   memset(itemText,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   indexItem=ITEM_NONE;
// QDOAS ???
// QDOAS ???   // Search for the item that has the selection
// QDOAS ???
// QDOAS ???   if ((hwndList!=NULL) &&
// QDOAS ???      ((*indexLvi=lvi.iItem=ListView_GetNextItem(hwndList,ITEM_NONE,LVNI_SELECTED))!=ITEM_NONE))
// QDOAS ???    {
// QDOAS ???     lvi.mask=LVIF_PARAM;                                                        // for retrieving index of item in list
// QDOAS ???
// QDOAS ???     if (ListView_GetItem(hwndList,&lvi) &&
// QDOAS ???        ((indexItem=(INDEX)lvi.lParam)!=ITEM_NONE))
// QDOAS ???
// QDOAS ???      strcpy(itemText,ANLYS_itemList[indexItem].itemText[0]);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return indexItem;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysInsertListViewItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       insert one item into the ListView control
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndList       handle of the ListView controls
// QDOAS ??? //               indexItem      index of the item in the ListView control
// QDOAS ??? //               lParam         index of the item in the data list
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysInsertListViewItem(HWND  hwndList,           // handle of ListView control
// QDOAS ???                              INDEX indexItem,          // index of item in ListView control
// QDOAS ???                              LPARAM lParam)            // index of item in list
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   LV_ITEM lvi;
// QDOAS ???
// QDOAS ???   // Fill structure with attributes of item to insert
// QDOAS ???
// QDOAS ???   lvi.mask=LVIF_TEXT|LVIF_PARAM|LVIF_STATE;
// QDOAS ???   lvi.state=LVIS_FOCUSED|LVIS_SELECTED;
// QDOAS ???   lvi.stateMask=LVIS_FOCUSED|LVIS_SELECTED;
// QDOAS ???   lvi.iItem=indexItem;
// QDOAS ???   lvi.iSubItem=0;
// QDOAS ???   lvi.pszText=LPSTR_TEXTCALLBACK;
// QDOAS ???   lvi.cchTextMax=MAX_ITEM_TEXT_LEN;
// QDOAS ???   lvi.lParam=lParam;
// QDOAS ???
// QDOAS ???   // Insert new item
// QDOAS ???
// QDOAS ???   ListView_InsertItem(hwndList,&lvi);
// QDOAS ???   ListView_EnsureVisible(hwndList,indexItem,FALSE);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysInsertAllListViewItems
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       insert all items in list into ListView control
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndList       handle of the ListView controls
// QDOAS ??? //               entryPoint     index of entry point in list
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysInsertAllListViewItems(HWND  hwndList,                                // handle of ListView control
// QDOAS ???                                  INDEX entryPoint)                              // index of entry point in list
// QDOAS ???  {
// QDOAS ???   INDEX indexItem;
// QDOAS ???
// QDOAS ???   for (indexItem=entryPoint;indexItem!=ITEM_NONE;indexItem=ANLYS_itemList[indexItem].indexPrevious)
// QDOAS ???    if (!ANLYS_itemList[indexItem].hidden)
// QDOAS ???     AnlysInsertListViewItem(hwndList,0,indexItem);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ===============
// QDOAS ??? // LIST PROCESSING
// QDOAS ??? // ===============
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      ANLYS_CopyItems
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_COPY command message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         pAnlysToCopy    pointer to the structure with the analysis properties node to copy
// QDOAS ??? //
// QDOAS ??? // OUTPUT        pAnlysToPaste   pointer to the structure with the analysis properties node to paste
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void ANLYS_CopyItems(ANALYSIS_WINDOWS *pAnlysToPaste,ANALYSIS_WINDOWS *pAnlysToCopy)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexTab,indexItem,indexOld,indexPrevious;
// QDOAS ???   LIST_ITEM *pList;
// QDOAS ???
// QDOAS ???   // Rebuild files names
// QDOAS ???
// QDOAS ???   FILES_RebuildFileName(pAnlysToPaste->refSpectrumFile,pAnlysToCopy->refSpectrumFile,1);
// QDOAS ???   FILES_RebuildFileName(pAnlysToPaste->refEtalon,pAnlysToCopy->refEtalon,1);
// QDOAS ???   FILES_RebuildFileName(pAnlysToPaste->residualsFile,pAnlysToCopy->residualsFile,1);
// QDOAS ???
// QDOAS ???   // Browse tab pages of analysis properties
// QDOAS ???
// QDOAS ???   for (indexTab=0;indexTab<TAB_TYPE_ANLYS_MAX;indexTab++)
// QDOAS ???    {
// QDOAS ???     indexPrevious=ITEM_NONE;
// QDOAS ???
// QDOAS ???     if (indexTab!=TAB_TYPE_ANLYS_OUTPUT)
// QDOAS ???      {
// QDOAS ???       indexOld=ITEM_NONE;
// QDOAS ???
// QDOAS ???       // Need to inverse the order of items
// QDOAS ???
// QDOAS ???       for (indexItem=pAnlysToCopy->listEntryPoint[indexTab];
// QDOAS ???            indexItem!=ITEM_NONE;
// QDOAS ???            indexItem=ANLYS_itemList[indexItem].indexPrevious)
// QDOAS ???
// QDOAS ???        indexOld=indexItem;
// QDOAS ???
// QDOAS ???       // Browse items in list
// QDOAS ???
// QDOAS ???       for (indexItem=indexOld;
// QDOAS ???           (indexItem!=ITEM_NONE) && ((UINT)ANLYS_nItemsToPaste<SYS_anlysItems);
// QDOAS ???            indexItem=ANLYS_itemList[indexItem].indexNext)
// QDOAS ???        {
// QDOAS ???         pList=&anlysItemListToPaste[ANLYS_nItemsToPaste];
// QDOAS ???         memcpy(pList,&ANLYS_itemList[indexItem],sizeof(LIST_ITEM));
// QDOAS ???
// QDOAS ???         FILES_RebuildFileName(pList->crossFileName,ANLYS_itemList[indexItem].crossFileName,1);
// QDOAS ???         FILES_RebuildFileName(pList->amfFileName,ANLYS_itemList[indexItem].amfFileName,1);
// QDOAS ???
// QDOAS ???         pList->indexParent=ITEM_NONE;
// QDOAS ???
// QDOAS ???         pList->indexPrevious=indexPrevious;
// QDOAS ???         pList->indexNext=ITEM_NONE;
// QDOAS ???
// QDOAS ???         if (indexPrevious!=ITEM_NONE)
// QDOAS ???          anlysItemListToPaste[indexPrevious].indexNext=ANLYS_nItemsToPaste;
// QDOAS ???
// QDOAS ???         indexPrevious=ANLYS_nItemsToPaste++;
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       pAnlysToPaste->listEntryPoint[indexTab]=indexPrevious;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      ANLYS_PasteItems
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_PASTE command message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         pAnlysToPaste   pointer to the structure with the analysis properties node to paste
// QDOAS ??? //               indexParent     index of the parent
// QDOAS ??? //
// QDOAS ??? // OUTPUT        pNewAnlys       pointer to the receiving new analysis structure
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void ANLYS_PasteItems(ANALYSIS_WINDOWS *pAnlysToPaste,ANALYSIS_WINDOWS *pNewAnlys,INDEX indexParent)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexTab,indexItem,indexOld,indexPrevious,indexNewItem,indexSymb;
// QDOAS ???   UCHAR symbolName[MAX_ITEM_TEXT_LEN+1],*ptr;
// QDOAS ???   LIST_ITEM *pList;
// QDOAS ???
// QDOAS ???   // Rebuild files names
// QDOAS ???
// QDOAS ???   FILES_CompactPath(pNewAnlys->refSpectrumFile,pAnlysToPaste->refSpectrumFile,1,1);
// QDOAS ???   FILES_CompactPath(pNewAnlys->refEtalon,pAnlysToPaste->refEtalon,1,1);
// QDOAS ???   FILES_CompactPath(pNewAnlys->residualsFile,pAnlysToPaste->residualsFile,1,1);
// QDOAS ???
// QDOAS ???   // Browse tab pages of analysis properties
// QDOAS ???
// QDOAS ???   for (indexTab=0;indexTab<TAB_TYPE_ANLYS_MAX;indexTab++)
// QDOAS ???    {
// QDOAS ???     indexPrevious=ITEM_NONE;
// QDOAS ???
// QDOAS ???     if (indexTab!=TAB_TYPE_ANLYS_OUTPUT)
// QDOAS ???      {
// QDOAS ???       indexOld=ITEM_NONE;
// QDOAS ???
// QDOAS ???       // Need to inverse the order of items
// QDOAS ???
// QDOAS ???       for (indexItem=pAnlysToPaste->listEntryPoint[indexTab];
// QDOAS ???            indexItem!=ITEM_NONE;
// QDOAS ???            indexItem=anlysItemListToPaste[indexItem].indexPrevious)
// QDOAS ???
// QDOAS ???        indexOld=indexItem;
// QDOAS ???
// QDOAS ???       // Browse items in list
// QDOAS ???
// QDOAS ???       for (indexItem=indexOld;
// QDOAS ???           (indexItem!=ITEM_NONE) && ((indexNewItem=ANLYS_itemFree)!=ITEM_NONE);
// QDOAS ???            indexItem=anlysItemListToPaste[indexItem].indexNext)
// QDOAS ???        {
// QDOAS ???         pList=&ANLYS_itemList[indexNewItem];
// QDOAS ???
// QDOAS ???         // Update list of available items
// QDOAS ???
// QDOAS ???         ANLYS_itemFree=pList->indexNext;                                        // First item of the list
// QDOAS ???         ANLYS_itemList[ANLYS_itemFree].indexPrevious=ITEM_NONE;                 // Second item of the list
// QDOAS ???
// QDOAS ???         memcpy(pList,&anlysItemListToPaste[indexItem],sizeof(LIST_ITEM));
// QDOAS ???
// QDOAS ???         FILES_CompactPath(pList->crossFileName,anlysItemListToPaste[indexItem].crossFileName,1,1);
// QDOAS ???         FILES_CompactPath(pList->amfFileName,anlysItemListToPaste[indexItem].amfFileName,1,1);
// QDOAS ???
// QDOAS ???         if ((ptr=strrchr(pList->crossFileName,PATH_SEP))!=NULL)
// QDOAS ???          strcpy(symbolName,ptr+1);
// QDOAS ???         else
// QDOAS ???          strcpy(symbolName,pList->crossFileName);
// QDOAS ???
// QDOAS ???         if ((ptr=strchr(symbolName,'_'))!=NULL)
// QDOAS ???          *ptr=0;
// QDOAS ???
// QDOAS ???         if ((strlen(symbolName)>0) &&
// QDOAS ???           (((indexSymb=TREE_GetIndexByDataName(symbolName,TREE_ITEM_TYPE_CROSS_CHILDREN,SYMB_crossTreeEntryPoint))!=ITEM_NONE) ||
// QDOAS ???            ((indexSymb=TREE_InsertOneItem(CHILD_list[CHILD_WINDOW_ENVIRONMENT].hwndTree,symbolName,SYMB_crossTreeEntryPoint,TREE_ITEM_TYPE_CROSS_CHILDREN,0,0,0))!=ITEM_NONE)))
// QDOAS ???
// QDOAS ???          TREE_itemList[indexSymb].useCount++;
// QDOAS ???
// QDOAS ???         pList->indexParent=indexParent;
// QDOAS ???         pList->indexPrevious=indexPrevious;
// QDOAS ???         pList->indexNext=ITEM_NONE;
// QDOAS ???
// QDOAS ???         if (indexPrevious!=ITEM_NONE)
// QDOAS ???          ANLYS_itemList[indexPrevious].indexNext=indexNewItem;
// QDOAS ???
// QDOAS ???         indexPrevious=indexNewItem;
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       pNewAnlys->listEntryPoint[indexTab]=indexPrevious;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysCopyAllListItems
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       make a copy of all items in list owned by a window analysis
// QDOAS ??? //
// QDOAS ??? // INPUT         entryPoint      entry point in the list of items
// QDOAS ??? //               indexTabPage    index of the tab page
// QDOAS ??? //
// QDOAS ??? // OUTPUT        pTab            pointer to the tab page to update
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysCopyAllListItems(INDEX entryPoint,ANLYS_TAB_PAGE *pTab,INDEX indexTabPage)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   ANLYS_TAB_PAGE *pTabCrossPage;
// QDOAS ???   UCHAR itemText[MAX_ITEM_TEXT_LEN+1],*ptrOld,*ptrNew;
// QDOAS ???   INDEX indexItem,indexSymbol,indexColumn;
// QDOAS ???   LIST_ITEM *pList;
// QDOAS ???
// QDOAS ???   // Set symbols reference
// QDOAS ???
// QDOAS ???   pTab->symbolList=(SYMBOL *)TREE_itemType[pTab->symbolType].dataList;          // symbols list to use
// QDOAS ???   pTab->symbolNumber=TREE_itemType[pTab->symbolType].dataNumber;                // number of symbols in previous list
// QDOAS ???
// QDOAS ???   // Set number of available symbols for filtering context menu options
// QDOAS ???
// QDOAS ???   switch(indexTabPage)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TAB_TYPE_ANLYS_CROSS :
// QDOAS ???      pTab->availableSymbolNumber=pTab->symbolNumber-SYMBOL_PREDEFINED_MAX;
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TAB_TYPE_ANLYS_SHIFT_AND_STRETCH :
// QDOAS ???      pTab->availableSymbolNumber=pTab->symbolNumber-ANLYS_tabPages[TAB_TYPE_ANLYS_CROSS].availableSymbolNumber;
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TAB_TYPE_ANLYS_GAPS :
// QDOAS ???     case TAB_TYPE_ANLYS_LINEAR :
// QDOAS ???     case TAB_TYPE_ANLYS_NOTLINEAR :
// QDOAS ???      pTab->availableSymbolNumber=9999;
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if ((pTab->oldListEntryPoint=pTab->listEntryPoint=entryPoint)!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     for (indexItem=entryPoint;
// QDOAS ???          indexItem!=ITEM_NONE;
// QDOAS ???          indexItem=ANLYS_itemList[indexItem].indexPrevious)
// QDOAS ???      {
// QDOAS ???       pList=&ANLYS_itemList[indexItem];
// QDOAS ???
// QDOAS ???       // Reserve access to symbols in used
// QDOAS ???
// QDOAS ???       if ((pTab->symbolReferenceNumber!=NULL) && (pTab->symbolList!=NULL) &&
// QDOAS ???          ((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,pList->itemText[0]))!=ITEM_NONE))
// QDOAS ???        pTab->symbolReferenceNumber[indexSymbol]|=(SYMBOL_ALREADY_USED<<8);
// QDOAS ???
// QDOAS ???       if (indexTabPage!=TAB_TYPE_ANLYS_SHIFT_AND_STRETCH)
// QDOAS ???        pTab->availableSymbolNumber--;
// QDOAS ???       else
// QDOAS ???        {
// QDOAS ???         pTabCrossPage=&ANLYS_tabPages[TAB_TYPE_ANLYS_CROSS];
// QDOAS ???         strcpy(itemText,pList->itemText[0]);
// QDOAS ???         ptrOld=itemText;
// QDOAS ???
// QDOAS ???         while (ptrOld!=NULL)
// QDOAS ???          {
// QDOAS ???           if ((ptrNew=strchr(ptrOld,';'))!=NULL)
// QDOAS ???            *ptrNew++='\0';
// QDOAS ???
// QDOAS ???           if ((pTabCrossPage->symbolReferenceNumber!=NULL) &&
// QDOAS ???              ((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,ptrOld))!=ITEM_NONE))
// QDOAS ???            {
// QDOAS ???             pTabCrossPage->symbolReferenceNumber[indexSymbol]++;
// QDOAS ???             pTab->symbolReferenceNumber[indexSymbol]|=(SYMBOL_ALREADY_USED<<8);
// QDOAS ???             pTab->availableSymbolNumber--;
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           ptrOld=ptrNew;
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Process comboboxes
// QDOAS ???
// QDOAS ???       for (indexColumn=1;indexColumn<pTab->columnNumber;indexColumn++)
// QDOAS ???        {
// QDOAS ???         if ((pTab->columnList[indexColumn].displayNumber>=pTab->minDisplayNumber) &&
// QDOAS ???             (pTab->columnList[indexColumn].displayNumber<=pTab->maxDisplayNumber) &&
// QDOAS ???             (pTab->columnList[indexColumn].comboboxItemType&COMBOBOX_ITEM_TYPE_ORTHOGONAL) &&
// QDOAS ???             (pTab->symbolReferenceNumber!=NULL) &&
// QDOAS ???            ((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,pList->itemText[indexColumn]))!=ITEM_NONE))
// QDOAS ???          pTab->symbolReferenceNumber[indexSymbol]++;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif // __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      ANLYS_ReleaseListItems
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Release access to items currently in use in list
// QDOAS ??? //
// QDOAS ??? // INPUT         indexWindow      index of the analysis window
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void ANLYS_ReleaseListItems(INDEX indexWindow)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   ANALYSIS_WINDOWS *pAnlys;
// QDOAS ???   ANLYS_TAB_PAGE *pTab;
// QDOAS ???   INDEX indexListItem,indexTreeItem,indexTab,
// QDOAS ???         listEntryPoint,treeEntryPoint;
// QDOAS ???
// QDOAS ???   if (indexWindow!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     pAnlys=&ANLYS_windowsList[indexWindow];
// QDOAS ???
// QDOAS ???     // Browse pages of the analysis windows properties
// QDOAS ???
// QDOAS ???     for (indexTab=0;indexTab<TAB_TYPE_ANLYS_MAX;indexTab++)
// QDOAS ???
// QDOAS ???      if ((listEntryPoint=pAnlys->listEntryPoint[indexTab])!=ITEM_NONE)
// QDOAS ???       {
// QDOAS ???        pTab=&ANLYS_tabPages[indexTab];
// QDOAS ???
// QDOAS ???        if ((ANLYS_itemList[listEntryPoint].indexNext=ANLYS_itemFree)!=ITEM_NONE)
// QDOAS ???         ANLYS_itemList[ANLYS_itemFree].indexPrevious=listEntryPoint;
// QDOAS ???
// QDOAS ???        ANLYS_itemFree=listEntryPoint;
// QDOAS ???        treeEntryPoint=(pTab->pTreeEntryPoint!=NULL)?*pTab->pTreeEntryPoint:ITEM_NONE;
// QDOAS ???
// QDOAS ???        // Browse items in the list
// QDOAS ???
// QDOAS ???        for (indexListItem=listEntryPoint;
// QDOAS ???             indexListItem!=ITEM_NONE;
// QDOAS ???             ANLYS_itemFree=indexListItem,
// QDOAS ???             indexListItem=ANLYS_itemList[indexListItem].indexPrevious)
// QDOAS ???         {
// QDOAS ???          // Release access to symbols in use before modifications
// QDOAS ???
// QDOAS ???          if ((indexTreeItem=TREE_GetIndexByDataName(ANLYS_itemList[indexListItem].itemText[0],pTab->symbolType,treeEntryPoint))!=ITEM_NONE)
// QDOAS ???           TREE_itemList[indexTreeItem].useCount--;
// QDOAS ???
// QDOAS ???          // reset content of items
// QDOAS ???
// QDOAS ???          memset(&ANLYS_itemList[indexListItem],0,
// QDOAS ???                (MAX_LIST_COLUMNS+2)*(MAX_ITEM_TEXT_LEN+1));
// QDOAS ???
// QDOAS ???          // put list on the top of free list
// QDOAS ???
// QDOAS ???          ANLYS_itemList[indexListItem].indexParent=ITEM_NONE;
// QDOAS ???         }
// QDOAS ???       }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysInsertListItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Insert a new item in list
// QDOAS ??? //
// QDOAS ??? // INPUT         indexParent      index of the parent
// QDOAS ??? //               indexPrevious    index of the previous item
// QDOAS ??? //
// QDOAS ??? // RETURN        indexItem        the index of the new item
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INDEX AnlysInsertListItem(INDEX indexParent,INDEX indexPrevious)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   LIST_ITEM *pList;
// QDOAS ???   INDEX indexItem;
// QDOAS ???
// QDOAS ???   if ((indexItem=ANLYS_itemFree)!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     pList=&ANLYS_itemList[indexItem];
// QDOAS ???
// QDOAS ???     // Update list of available items
// QDOAS ???
// QDOAS ???     ANLYS_itemFree=pList->indexNext;                                            // First item of the list
// QDOAS ???     ANLYS_itemList[ANLYS_itemFree].indexPrevious=ITEM_NONE;                     // Second item of the list
// QDOAS ???
// QDOAS ???     // Structure filling
// QDOAS ???
// QDOAS ???     pList->indexParent=indexParent;
// QDOAS ???     pList->indexNext=ITEM_NONE;
// QDOAS ???
// QDOAS ???     if ((pList->indexPrevious=indexPrevious)!=ITEM_NONE)
// QDOAS ???      ANLYS_itemList[pList->indexPrevious].indexNext=indexItem;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return indexItem;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysGetListItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Get index of item in list
// QDOAS ??? //
// QDOAS ??? // INPUT         textItem          the name of the item to search for
// QDOAS ??? //               listEntryPoint    entry point in the list
// QDOAS ??? //
// QDOAS ??? // RETURN        indexItem        the index of the item
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INDEX AnlysGetListItem(UCHAR *textItem,INDEX listEntryPoint)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexItem;
// QDOAS ???   SZ_LEN textLength;
// QDOAS ???   UCHAR *ptr;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   textLength=strlen(textItem);
// QDOAS ???
// QDOAS ???   // Browse items in the list
// QDOAS ???
// QDOAS ???   for (indexItem=listEntryPoint;
// QDOAS ???        indexItem!=ITEM_NONE;
// QDOAS ???        indexItem=ANLYS_itemList[indexItem].indexPrevious)
// QDOAS ???
// QDOAS ???    if ((strlen((ptr=ANLYS_itemList[indexItem].itemText[0]))==textLength) &&
// QDOAS ???        !STD_Stricmp(ptr,textItem))
// QDOAS ???
// QDOAS ???     break;
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return indexItem;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysUpdateList
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Update list items with content of controls associated to the ListView
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndList          handle of ListView control
// QDOAS ??? //               pTab              pointer to the current tab page
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysUpdateList(HWND hwndList,ANLYS_TAB_PAGE *pTab)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   LIST_COLUMN *pColumn;
// QDOAS ???   LIST_ITEM   *pList;
// QDOAS ???
// QDOAS ???   UCHAR        itemText[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   INDEX        indexLine,indexColumn,indexItem;
// QDOAS ???   HWND         hwndControl;
// QDOAS ???
// QDOAS ???   // Search for lines and columns with an active control field (edit control, combobox or check box)
// QDOAS ???
// QDOAS ???   for (indexLine=0;indexLine<ANLYS_maxLines;indexLine++)
// QDOAS ???
// QDOAS ???    for (indexColumn=0;indexColumn<pTab->columnNumber;indexColumn++)
// QDOAS ???
// QDOAS ???     if (((hwndControl=pTab->visibleLinesImage[indexLine].hwnd[indexColumn])!=(HWND)NULL) &&   // handle of control is not null
// QDOAS ???           IsWindowVisible(hwndControl) &&                                                     // control is visible so update of list is possible
// QDOAS ???         ((indexItem=pTab->visibleLinesImage[indexLine].indexItem)!=ITEM_NONE))                // index of item in list exists
// QDOAS ???      {
// QDOAS ???       pColumn=&pTab->columnList[indexColumn];
// QDOAS ???       pList=&ANLYS_itemList[indexItem];
// QDOAS ???
// QDOAS ???       // Process edit controls and comboboxes
// QDOAS ???
// QDOAS ???       if (pColumn->controlStyle!=BS_AUTOCHECKBOX)
// QDOAS ???        {
// QDOAS ???         GetWindowText(hwndControl,itemText,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???         // Process comboboxes
// QDOAS ???
// QDOAS ???         if (((pColumn->comboboxItemType==COMBOBOX_ITEM_TYPE_ORTHOGONAL) ||
// QDOAS ???              (pColumn->comboboxItemType==COMBOBOX_ITEM_TYPE_EXCLUSIVE)) &&
// QDOAS ???             ((strlen(itemText)!=strlen(pList->itemText[indexColumn])) ||
// QDOAS ???               STD_Stricmp(itemText,pList->itemText[indexColumn])))
// QDOAS ???
// QDOAS ???          AnlysComboboxUpdate(pColumn->comboboxItemType,                         // combobox type
// QDOAS ???                              pTab->symbolList,                                  // list of available symbols
// QDOAS ???                              pTab->symbolNumber,                                // number of symbols in previous list
// QDOAS ???                              pTab->symbolReferenceNumber,                       // number of times symbols are referenced to
// QDOAS ???                              pList->itemText[indexColumn],                      // name of the symbol released
// QDOAS ???                              itemText);                                         // name of the symbol reserved
// QDOAS ???
// QDOAS ???         // Retrieve new item text
// QDOAS ???
// QDOAS ???         strcpy(pList->itemText[indexColumn],itemText);
// QDOAS ???
// QDOAS ???         if (ANLYS_indexTabPage==TAB_TYPE_ANLYS_GAPS)
// QDOAS ???          sprintf(pList->itemText[0],"Gap %s-%s",pList->itemText[1],pList->itemText[2]);
// QDOAS ???
// QDOAS ???         ShowWindow(hwndControl,SW_HIDE);
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Check boxes processing
// QDOAS ???
// QDOAS ???       else
// QDOAS ???        {
// QDOAS ???         pList->itemText[indexColumn][0]=(IsDlgButtonChecked(hwndList,indexLine*pTab->columnNumber+indexColumn+100*(ANLYS_indexTabPage+1))==BST_CHECKED)?(UCHAR)'1':(UCHAR)'0';
// QDOAS ???         pList->itemText[indexColumn][(ANLYS_indexTabPage!=TAB_TYPE_ANLYS_LINEAR)?1:2]=0;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysInsertDefaultItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Insert a default list item in tab page
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndList          handle of ListView control
// QDOAS ??? //               pTab              pointer to the current tab page
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysInsertDefaultItem(HWND hwndList,UCHAR *itemText,ANLYS_TAB_PAGE *pTab)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexListItem,indexColumn;
// QDOAS ???   LIST_ITEM *pList;
// QDOAS ???   SZ_LEN textLength;
// QDOAS ???
// QDOAS ???   // Search for item in list
// QDOAS ???
// QDOAS ???   if ((AnlysGetListItem(itemText,pTab->listEntryPoint)==ITEM_NONE) &&
// QDOAS ???      ((indexListItem=AnlysInsertListItem(ANLYS_indexSelected,pTab->listEntryPoint))!=ITEM_NONE))
// QDOAS ???    {
// QDOAS ???     // Add default item
// QDOAS ???
// QDOAS ???     pList=&ANLYS_itemList[indexListItem];
// QDOAS ???     strcpy(pList->itemText[0],itemText);
// QDOAS ???     textLength=strlen(itemText);
// QDOAS ???
// QDOAS ???     for (indexColumn=1;indexColumn<pTab->columnNumber;indexColumn++)
// QDOAS ???
// QDOAS ???      if ((textLength==strlen("SFP 1")) &&
// QDOAS ???         (!STD_Stricmp(itemText,"SFP 1") || !STD_Stricmp(itemText,"SFP 2") ||
// QDOAS ???          !STD_Stricmp(itemText,"SFP 3") || !STD_Stricmp(itemText,"SFP 4")) &&
// QDOAS ???         ((indexColumn==COLUMN_OTHERS_FIT) || (indexColumn==COLUMN_OTHERS_VALINIT)))
// QDOAS ???       strcpy(pList->itemText[indexColumn],(indexColumn==COLUMN_OTHERS_FIT)?"1":"0.5");
// QDOAS ???      else if ((textLength==strlen("Sol")) && !STD_Stricmp(itemText,"Sol") && (indexColumn==COLUMN_OTHERS_VALINIT))
// QDOAS ???       strcpy(pList->itemText[indexColumn],"1.");
// QDOAS ???      else if ((textLength==strlen("Polynomial (x)")) && !STD_Stricmp(itemText,"Polynomial (x)"))
// QDOAS ???       strcpy(pList->itemText[indexColumn],"0");
// QDOAS ???      else
// QDOAS ???       strcpy(pList->itemText[indexColumn],pTab->columnList[indexColumn].defaultValue);
// QDOAS ???
// QDOAS ???     AnlysInsertListViewItem(hwndList,ListView_GetItemCount(hwndList),indexListItem);
// QDOAS ???
// QDOAS ???     pTab->listEntryPoint=indexListItem;
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ======================
// QDOAS ??? // TAB CONTROL PROCESSING
// QDOAS ??? // ======================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysTabChange
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       TCN_SELCHANGE notification message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTab              handle of the tab page control
// QDOAS ??? //               hwndList             handle of the listView control
// QDOAS ??? //               hwndArrows           handle of the spin control (to change pages in the list)
// QDOAS ??? //               hwndPage             handle of the page control (indication of the current page in the list)
// QDOAS ??? //               displayNumberAction  indicates the page in the list to display
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysTabChange(HWND hwndTab,HWND hwndList,HWND hwndArrows,HWND hwndPage,CHAR displayNumberAction)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR columnTitle[MAX_ITEM_TEXT_LEN+1];                                       // text of the column title
// QDOAS ???
// QDOAS ???   INDEX indexTabPage,                                                           // index of the selected tab page
// QDOAS ???         indexColumn,                                                            // index for ListView columns
// QDOAS ???         indexLine;                                                              // index for ListView lines
// QDOAS ???
// QDOAS ???   LV_COLUMN lvCol;                                                              // attributes of columns of the ListView owned by tab control
// QDOAS ???   LIST_COLUMN *pCol;                                                            // pointer to the current column
// QDOAS ???   ANLYS_TAB_PAGE *pTab;                                                         // pointer to the selected tab page
// QDOAS ???
// QDOAS ???   RECT  rcClient;                                                               // coordinates of ListView client area
// QDOAS ???   INT   itemHeight;                                                             // height in pixels of items in ListView control
// QDOAS ???   HFONT itemFont;                                                               // font handle
// QDOAS ???   HWND  hwndControl;                                                            // handle of new control
// QDOAS ???   CHAR  displayPage[10],                                                        // page to display in text format
// QDOAS ???         displayNumber;                                                          // number of the page to display
// QDOAS ???
// QDOAS ???   // Update fields in the previous tab page
// QDOAS ???
// QDOAS ???   AnlysUpdateList(hwndList,&ANLYS_tabPages[ANLYS_indexTabPage]);
// QDOAS ???
// QDOAS ???   // Get the selected page of tab control
// QDOAS ???
// QDOAS ???   if (((indexTabPage=TabCtrl_GetCurSel(hwndTab))>ITEM_NONE) &&
// QDOAS ???        (indexTabPage<ANLYS_maxTabPage))
// QDOAS ???    {
// QDOAS ???     // Initializations
// QDOAS ???
// QDOAS ???     itemFont = CreateFont(14,0,0,0,FW_NORMAL,0,0,0,0,0,0,0,0,"Arial");
// QDOAS ???
// QDOAS ???     GetClientRect(hwndList,&rcClient);
// QDOAS ???     itemHeight=(rcClient.bottom-rcClient.top)/ANLYS_maxLines;
// QDOAS ???     pTab=&ANLYS_tabPages[ANLYS_indexTabPage];
// QDOAS ???
// QDOAS ???     // Remove all items from the ListView supported by the previous tab page
// QDOAS ???
// QDOAS ???     ListView_DeleteAllItems(hwndList);
// QDOAS ???
// QDOAS ???     for (indexLine=0;indexLine<ANLYS_maxLines;indexLine++)
// QDOAS ???      pTab->visibleLinesImage[indexLine].indexItem=ITEM_NONE;
// QDOAS ???
// QDOAS ???     for (indexColumn=0;indexColumn<pTab->columnNumber;indexColumn++)
// QDOAS ???      {
// QDOAS ???       // Remove all columns
// QDOAS ???
// QDOAS ???       ListView_DeleteColumn(hwndList,0);
// QDOAS ???
// QDOAS ???       // Destroy controls
// QDOAS ???
// QDOAS ???       for (indexLine=0;indexLine<ANLYS_maxLines;indexLine++)
// QDOAS ???        {
// QDOAS ???         if (pTab->visibleLinesImage[indexLine].hwnd[indexColumn]!=(HWND)NULL)
// QDOAS ???          {
// QDOAS ???           DestroyWindow(pTab->visibleLinesImage[indexLine].hwnd[indexColumn]);
// QDOAS ???           pTab->visibleLinesImage[indexLine].hwnd[indexColumn]=(HWND)NULL;
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     lvCol.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH;                                   // specify valid members of structure
// QDOAS ???     pTab=&ANLYS_tabPages[(ANLYS_indexTabPage=indexTabPage)];                    // set pointer to the new selected page
// QDOAS ???
// QDOAS ???     // Search for columns to display
// QDOAS ???
// QDOAS ???     if ((INT)(displayNumber=(CHAR)(pTab->displayNumber+displayNumberAction))<pTab->minDisplayNumber)
// QDOAS ???      displayNumber=pTab->minDisplayNumber;
// QDOAS ???     else if (displayNumber>pTab->maxDisplayNumber)
// QDOAS ???      displayNumber=(CHAR)pTab->maxDisplayNumber;
// QDOAS ???
// QDOAS ???     if (displayNumberAction)
// QDOAS ???      for (indexColumn=0;indexColumn<pTab->columnNumber;indexColumn++)
// QDOAS ???       if (pTab->columnList[indexColumn].displayNumber==displayNumber)
// QDOAS ???        {
// QDOAS ???         pTab->displayNumber=displayNumber;
// QDOAS ???         break;
// QDOAS ???        }
// QDOAS ???
// QDOAS ???     // Insert columns in ListView control
// QDOAS ???
// QDOAS ???     for (indexColumn=0;indexColumn<pTab->columnNumber;indexColumn++)
// QDOAS ???      {
// QDOAS ???       // Fill structure
// QDOAS ???
// QDOAS ???       pCol=&pTab->columnList[indexColumn];
// QDOAS ???
// QDOAS ???       if (!pCol->displayNumber ||
// QDOAS ???           (pCol->displayNumber==pTab->displayNumber))
// QDOAS ???        {
// QDOAS ???         strcpy(columnTitle,pCol->columnTitle);
// QDOAS ???
// QDOAS ???         if ((ANLYS_indexTabPage==TAB_TYPE_ANLYS_SHIFT_AND_STRETCH) &&
// QDOAS ???            ((indexColumn==COLUMN_SH_INIT) || (indexColumn==COLUMN_SH_DELTA) || (indexColumn==COLUMN_SH_MIN) || (indexColumn==COLUMN_SH_MAX)))
// QDOAS ???
// QDOAS ???          strcat(columnTitle,(PRJCT_itemList[ANLYS_indexProject].analysis.units==PRJCT_ANLYS_UNITS_PIXELS)?" (px)":" (nm)");
// QDOAS ???
// QDOAS ???         lvCol.pszText=columnTitle;                                              // title of the column
// QDOAS ???         lvCol.cchTextMax=strlen(lvCol.pszText);                                 // length of title
// QDOAS ???         lvCol.fmt=pCol->columnFormat;                                           // format of column
// QDOAS ???         lvCol.cx=pCol->columnWidth;                                             // width of column
// QDOAS ???
// QDOAS ???         // Insert column
// QDOAS ???
// QDOAS ???         ListView_InsertColumn(hwndList,indexColumn,&lvCol);
// QDOAS ???
// QDOAS ???         // Create controls associated to this column
// QDOAS ???
// QDOAS ???         if (pCol->controlName[0])
// QDOAS ???          for (indexLine=0;indexLine<ANLYS_maxLines;indexLine++)
// QDOAS ???           {
// QDOAS ???            pTab->visibleLinesImage[indexLine].hwnd[indexColumn]=hwndControl=
// QDOAS ???             CreateWindowEx(0,                                                                   // Ex style
// QDOAS ???                            pCol->controlName,                                                   // class name
// QDOAS ???                            "",                                                                  // dummy Text
// QDOAS ???                            pCol->controlStyle|                                                  // styles depending on control
// QDOAS ???                            WS_CHILD,                                                            // common controls and windows styles
// QDOAS ???                           (STD_Stricmp(pCol->controlName,"button"))?
// QDOAS ???                            rcClient.left:rcClient.left+(pCol->columnWidth-itemHeight)/2,
// QDOAS ???                            rcClient.top+indexLine*itemHeight,
// QDOAS ???                           (STD_Stricmp(pCol->controlName,"button"))?
// QDOAS ???                            pCol->columnWidth:itemHeight,                                        // client area
// QDOAS ???                           (pCol->comboboxItemType==COMBOBOX_ITEM_TYPE_NONE)?
// QDOAS ???                            itemHeight:rcClient.bottom,
// QDOAS ???                            hwndList,                                                            // Parent
// QDOAS ???                (HMENU)    (indexLine*pTab->columnNumber+indexColumn+100*(indexTabPage+1)),      // ID
// QDOAS ???                            DOAS_hInst,                                                          // Instance
// QDOAS ???                            NULL);                                                               // No extra
// QDOAS ???
// QDOAS ???            SendMessage(hwndControl,WM_SETFONT,(WPARAM)itemFont,0L);
// QDOAS ???            ShowWindow(hwndControl,SW_HIDE);
// QDOAS ???           }
// QDOAS ???
// QDOAS ???         rcClient.left+=pCol->columnWidth;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Insert all items in list
// QDOAS ???
// QDOAS ???     if (ANLYS_indexTabPage==TAB_TYPE_ANLYS_OUTPUT)
// QDOAS ???      pTab->listEntryPoint=ANLYS_tabPages[TAB_TYPE_ANLYS_CROSS].listEntryPoint;
// QDOAS ???
// QDOAS ???     AnlysInsertAllListViewItems(hwndList,pTab->listEntryPoint);
// QDOAS ???
// QDOAS ???     // Page display control
// QDOAS ???
// QDOAS ???     ShowWindow(hwndPage,(pTab->maxDisplayNumber-pTab->minDisplayNumber>0)?SW_SHOW:SW_HIDE);
// QDOAS ???     ShowWindow(hwndArrows,(pTab->maxDisplayNumber-pTab->minDisplayNumber>0)?SW_SHOW:SW_HIDE);
// QDOAS ???
// QDOAS ???     sprintf(displayPage,"%d/%d",(pTab->displayNumber-pTab->minDisplayNumber+1),(pTab->maxDisplayNumber-pTab->minDisplayNumber+1));
// QDOAS ???     SetWindowText(hwndPage,displayPage);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysTabInit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       tab control initialization
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndAnlys  handle of the analysis properties dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysTabInit(HWND hwndAnlys)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   HWND hwndTab,hwndList,hwndArrows,hwndPage;                                    // handles of controls in the dialog box
// QDOAS ???   ANALYSIS_WINDOWS *pAnlys;                                                     // pointer to the current analysis window
// QDOAS ???   ANLYS_TAB_PAGE *pTab;                                                         // pointer to the current tab control
// QDOAS ???   TC_ITEM tie;                                                                  // attributes of the tab control
// QDOAS ???   INDEX indexTab,                                                               // tab page number (base 0)
// QDOAS ???         indexItem,                                                              // index of item in the list
// QDOAS ???         indexTree,                                                              // index of the analysis window in the tree list
// QDOAS ???         indexProject,                                                           // index of the parent project in the project list
// QDOAS ???         indexLine,                                                              // indexes for visible lines in ListView control
// QDOAS ???         indexColumn;                                                            // indexes for columns in ListView control
// QDOAS ???   INT   hidden;                                                                 // flag to hide items in the list
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   hwndTab=GetDlgItem(hwndAnlys,ANLYS_DATA);
// QDOAS ???   hwndList=GetDlgItem(hwndAnlys,ANLYS_LISTVIEW);
// QDOAS ???   hwndArrows=GetDlgItem(hwndAnlys,ANLYS_ARROWS);
// QDOAS ???   hwndPage=GetDlgItem(hwndAnlys,ANLYS_PAGE);
// QDOAS ???
// QDOAS ???   pAnlys=&ANLYS_windowsList[ANLYS_indexSelected];
// QDOAS ???
// QDOAS ???   // !!! ---------------------
// QDOAS ???
// QDOAS ???   indexTree=TREE_GetIndexByDataIndex(ANLYS_indexSelected,TREE_ITEM_TYPE_ANALYSIS_CHILDREN,CHILD_list[CHILD_WINDOW_PROJECT].itemTree);
// QDOAS ???   indexProject=TREE_itemList[TREE_itemList[TREE_itemList[indexTree].parentItem].parentItem].dataIndex;  // Grand-Father
// QDOAS ???
// QDOAS ???   // !!! ---------------------
// QDOAS ???
// QDOAS ???   if (pAnlys->hidden)
// QDOAS ???    {
// QDOAS ???     ANLYS_tabPages[TAB_TYPE_ANLYS_CROSS].minDisplayNumber=5;
// QDOAS ???     ANLYS_tabPages[TAB_TYPE_ANLYS_CROSS].maxDisplayNumber=6;
// QDOAS ???     ANLYS_tabPages[TAB_TYPE_ANLYS_OUTPUT].minDisplayNumber=7;
// QDOAS ???     ANLYS_tabPages[TAB_TYPE_ANLYS_OUTPUT].maxDisplayNumber=7;
// QDOAS ???    }
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     ANLYS_tabPages[TAB_TYPE_ANLYS_CROSS].minDisplayNumber=1;
// QDOAS ???     ANLYS_tabPages[TAB_TYPE_ANLYS_CROSS].maxDisplayNumber=2;
// QDOAS ???     ANLYS_tabPages[TAB_TYPE_ANLYS_OUTPUT].minDisplayNumber=3;
// QDOAS ???     ANLYS_tabPages[TAB_TYPE_ANLYS_OUTPUT].maxDisplayNumber=4;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   tie.mask=TCIF_TEXT|TCIF_IMAGE;                                                // specify wich members are valid
// QDOAS ???   tie.iImage=-1;                                                                // no icon is associated to tabs
// QDOAS ???
// QDOAS ???   ANLYS_maxLines=ListView_GetCountPerPage(hwndList);
// QDOAS ???
// QDOAS ???   // Browse tab control pages
// QDOAS ???
// QDOAS ???   for (indexTab=0;indexTab<ANLYS_maxTabPage;indexTab++)
// QDOAS ???    {
// QDOAS ???     pTab=&ANLYS_tabPages[indexTab];
// QDOAS ???
// QDOAS ???     // Insert tab pages
// QDOAS ???
// QDOAS ???     tie.pszText=pTab->tabTitle;
// QDOAS ???     TabCtrl_InsertItem(hwndTab,indexTab,&tie);
// QDOAS ???
// QDOAS ???     // Initializes controls associated to visible lines in ListView control
// QDOAS ???
// QDOAS ???     pTab->visibleLinesImage=(VISIBLE_LINES_IMAGE *)MEMORY_AllocBuffer("AnlysTabInit ","visibleLinesImage",ANLYS_maxLines,sizeof(VISIBLE_LINES_IMAGE),0,MEMORY_TYPE_STRUCT);
// QDOAS ???
// QDOAS ???     pTab->symbolReferenceNumber=
// QDOAS ???
// QDOAS ???      ((pTab->symbolType!=TREE_ITEM_TYPE_NONE)&&(TREE_itemType[pTab->symbolType].dataNumber!=0))?
// QDOAS ???      (INT *)MEMORY_AllocBuffer("AnlysTabInit ","symbolReferenceNumber",(TREE_itemType[pTab->symbolType].dataNumber),sizeof(INT),0,MEMORY_TYPE_INT):NULL;
// QDOAS ???
// QDOAS ???     if (pTab->symbolReferenceNumber!=NULL)
// QDOAS ???      memset(pTab->symbolReferenceNumber,0,TREE_itemType[pTab->symbolType].dataNumber*sizeof(INT));
// QDOAS ???
// QDOAS ???     pTab->displayNumber=pTab->minDisplayNumber;
// QDOAS ???
// QDOAS ???     for (indexLine=0;indexLine<ANLYS_maxLines;indexLine++)
// QDOAS ???      {
// QDOAS ???       pTab->visibleLinesImage[indexLine].indexItem=ITEM_NONE;
// QDOAS ???       pTab->visibleLinesImage[indexLine].hwnd=(HWND *)MEMORY_AllocBuffer("AnlysTabInit ","hwnd",pTab->columnNumber,sizeof(HWND),0,MEMORY_TYPE_PTR);
// QDOAS ???
// QDOAS ???       for (indexColumn=0;indexColumn<pTab->columnNumber;indexColumn++)
// QDOAS ???        pTab->visibleLinesImage[indexLine].hwnd[indexColumn]=(HWND)NULL;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Use a copy of items in list
// QDOAS ???
// QDOAS ???     AnlysCopyAllListItems(pAnlys->listEntryPoint[indexTab],pTab,indexTab);
// QDOAS ???
// QDOAS ???     // Linear parameters
// QDOAS ???
// QDOAS ???     if ((indexTab==TAB_TYPE_ANLYS_LINEAR) &&
// QDOAS ???         (pTab->listEntryPoint==ITEM_NONE))
// QDOAS ???      {
// QDOAS ???       AnlysInsertDefaultItem(hwndList,"Polynomial (x)",pTab);
// QDOAS ???       AnlysInsertDefaultItem(hwndList,"Polynomial (1/x)",pTab);
// QDOAS ???       AnlysInsertDefaultItem(hwndList,"Offset",pTab);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Not linear parameters
// QDOAS ???
// QDOAS ???     else if (indexTab==TAB_TYPE_ANLYS_NOTLINEAR)
// QDOAS ???      {
// QDOAS ???       AnlysInsertDefaultItem(hwndList,"Offset (Constant)",pTab);                                     // Insert "Offset (Constant)" in 'predefined' tab page
// QDOAS ???       AnlysInsertDefaultItem(hwndList,"Offset (Order 1)",pTab);                                      // Insert "Offset (Order 1)" in 'predefined' tab page
// QDOAS ???       AnlysInsertDefaultItem(hwndList,"Offset (Order 2)",pTab);                                      // Insert "Offset (Order 2)" in 'predefined' tab page
// QDOAS ???
// QDOAS ???       hidden=(PRJCT_itemList[indexProject].analysis.method==PRJCT_ANLYS_METHOD_SVD)?0:1;
// QDOAS ???
// QDOAS ???       if ((indexItem=AnlysGetListItem("Offset (Constant)",pTab->listEntryPoint))!=ITEM_NONE)
// QDOAS ???        ANLYS_itemList[indexItem].hidden=hidden;
// QDOAS ???       if ((indexItem=AnlysGetListItem("Offset (Order 1)",pTab->listEntryPoint))!=ITEM_NONE)
// QDOAS ???        ANLYS_itemList[indexItem].hidden=hidden;
// QDOAS ???       if ((indexItem=AnlysGetListItem("Offset (Order 2)",pTab->listEntryPoint))!=ITEM_NONE)
// QDOAS ???        ANLYS_itemList[indexItem].hidden=hidden;
// QDOAS ???
// QDOAS ???       AnlysInsertDefaultItem(hwndList,"Sol",pTab);                                                   // Insert "Sol" in 'predefined' tab page
// QDOAS ???
// QDOAS ???       if (pAnlys->hidden)
// QDOAS ???        {
// QDOAS ???         AnlysInsertDefaultItem(hwndList,"SFP 1",pTab);
// QDOAS ???         AnlysInsertDefaultItem(hwndList,"SFP 2",pTab);
// QDOAS ???         AnlysInsertDefaultItem(hwndList,"SFP 3",pTab);
// QDOAS ???         AnlysInsertDefaultItem(hwndList,"SFP 4",pTab);
// QDOAS ???        }
// QDOAS ???       else
// QDOAS ???        {
// QDOAS ???         AnlysInsertDefaultItem(hwndList,SYMB_itemCrossList[SYMBOL_PREDEFINED_COM].name,pTab);        // Insert "common residual" in 'Predefined' tab page
// QDOAS ???         AnlysInsertDefaultItem(hwndList,SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP1].name,pTab);     // Insert "undersampling (phase 1)" in 'Predefined' tab page
// QDOAS ???         AnlysInsertDefaultItem(hwndList,SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP2].name,pTab);     // Insert "undersampling (phase 2)" in 'Predefined' tab page
// QDOAS ???         AnlysInsertDefaultItem(hwndList,SYMB_itemCrossList[SYMBOL_PREDEFINED_RING1].name,pTab);      // Insert "Ring1" in 'Predefined' tab page
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Set first tab page
// QDOAS ???
// QDOAS ???   AnlysTabChange(hwndTab,hwndList,hwndArrows,hwndPage,0);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // =================================================================
// QDOAS ??? // ANALYSIS WINDOWS PROPERTIES : CROSS SECTIONS SET PANEL PROCESSING
// QDOAS ??? // =================================================================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysXsSetInit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_INITDIALOG message processing for the dialog box that appears
// QDOAS ??? //               to select cross sections to shift
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndXsSet          handle of the dialog box
// QDOAS ??? //               hwndParent         handle of the parent dialog box (analysis window)
// QDOAS ??? //               pTab               pointer to the shift and stretch tab page
// QDOAS ??? //               indexSelectedItem  index of the selected item in the list
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysXsSetInit(HWND hwndXsSet,HWND hwndParent,ANLYS_TAB_PAGE *pTab,INDEX indexSelectedItem)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   RECT rcParent,rcChild,rcClient;                                               // windows rectangle
// QDOAS ???   HWND hwndLeftList,hwndRightList;                                              // handles of list boxes
// QDOAS ???   INDEX  indexItem,indexSymbol;                                                 // index in list
// QDOAS ???   UCHAR  itemText[MAX_ITEM_TEXT_LEN+1],                                         // text of selected item in properties menu case
// QDOAS ???         *ptrOld,*ptrNew;                                                        // pointer to characters in string
// QDOAS ???
// QDOAS ???   // Set window title
// QDOAS ???
// QDOAS ???   SetWindowText(hwndXsSet,"Set of cross sections for shift and stretch");
// QDOAS ???
// QDOAS ???   // Window resizing
// QDOAS ???
// QDOAS ???   GetWindowRect(hwndParent,&rcParent);                                          // parent window rectangle
// QDOAS ???   GetWindowRect(hwndXsSet,&rcChild);                                            // child window rectangle
// QDOAS ???   GetClientRect(hwndParent,&rcClient);                                          // client rectangle of parent window
// QDOAS ???
// QDOAS ???   MoveWindow(hwndXsSet,
// QDOAS ???              rcParent.left,
// QDOAS ???              rcParent.bottom-(rcClient.bottom-rcClient.top),
// QDOAS ???              rcChild.right-rcChild.left,
// QDOAS ???              rcChild.bottom-rcChild.top,
// QDOAS ???              TRUE);
// QDOAS ???
// QDOAS ???   // Fill left listbox with available symbols
// QDOAS ???
// QDOAS ???   indexItem=ANLYS_tabPages[TAB_TYPE_ANLYS_CROSS].listEntryPoint;                // cross sections entry point in list
// QDOAS ???
// QDOAS ???   hwndLeftList=GetDlgItem(hwndXsSet,ANLYS_LIST_AVAILABLE);
// QDOAS ???
// QDOAS ???   if (/* !ANLYS_windowsList[ANLYS_indexSelected].hidden && */ (pTab->symbolReferenceNumber!=NULL) && !((pTab->symbolReferenceNumber[SYMBOL_PREDEFINED_SPECTRUM]>>8)&SYMBOL_ALREADY_USED))
// QDOAS ???    SendMessage(hwndLeftList,LB_ADDSTRING,0,(LPARAM)SYMB_itemCrossList[SYMBOL_PREDEFINED_SPECTRUM].name);
// QDOAS ???   if ((pTab->symbolReferenceNumber!=NULL) && !((pTab->symbolReferenceNumber[SYMBOL_PREDEFINED_REF]>>8)&SYMBOL_ALREADY_USED))
// QDOAS ???    SendMessage(hwndLeftList,LB_ADDSTRING,0,(LPARAM)SYMB_itemCrossList[SYMBOL_PREDEFINED_REF].name);
// QDOAS ???
// QDOAS ???   while (indexItem!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     if ((pTab->symbolReferenceNumber!=NULL) && (pTab->symbolType!=TREE_ITEM_TYPE_NONE) &&
// QDOAS ???         (TREE_itemType[pTab->symbolType].dataList!=NULL) &&
// QDOAS ???         (TREE_itemType[pTab->symbolType].dataNumber>0) &&
// QDOAS ???        ((indexSymbol=SYMB_GetListIndex((SYMBOL *)TREE_itemType[pTab->symbolType].dataList,
// QDOAS ???          TREE_itemType[pTab->symbolType].dataNumber,ANLYS_itemList[indexItem].itemText[0]))!=ITEM_NONE) &&
// QDOAS ???       !((pTab->symbolReferenceNumber[indexSymbol]>>8)&SYMBOL_ALREADY_USED))
// QDOAS ???
// QDOAS ???      SendMessage(hwndLeftList,LB_ADDSTRING,0,(LPARAM)ANLYS_itemList[indexItem].itemText[0]);
// QDOAS ???
// QDOAS ???     indexItem=ANLYS_itemList[indexItem].indexPrevious;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Fill right listbox with selected symbols
// QDOAS ???
// QDOAS ???   ANLYS_indexXsSet=(indexSelectedItem!=ITEM_NONE)?indexSelectedItem:ANLYS_itemFree;
// QDOAS ???
// QDOAS ???   if (indexSelectedItem!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     hwndRightList=GetDlgItem(hwndXsSet,ANLYS_LIST_SELECTED);
// QDOAS ???     strcpy(itemText,ANLYS_itemList[indexSelectedItem].itemText[0]);
// QDOAS ???     ptrOld=itemText;
// QDOAS ???
// QDOAS ???     while (ptrOld!=NULL)
// QDOAS ???      {
// QDOAS ???       if ((ptrNew=strchr(ptrOld,';'))!=NULL)
// QDOAS ???        *ptrNew++='\0';
// QDOAS ???
// QDOAS ???       SendMessage(hwndRightList,LB_ADDSTRING,0,(LPARAM)ptrOld);
// QDOAS ???       ptrOld=ptrNew;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysXsSetOK
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       IDOK command message processing for cross sections set panel
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndXsSet          handle of the dialog box
// QDOAS ??? //               pTab               pointer to the shift and stretch tab page
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysXsSetOK(HWND hwndXsSet,ANLYS_TAB_PAGE *pTab)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR  itemText[MAX_ITEM_TEXT_LEN+1],                                         // text of selected items
// QDOAS ???         *pListItemText,                                                         // pointer to receiving string buffer in list
// QDOAS ???         *ptr;                                                                   // pointer to characters in previous string
// QDOAS ???
// QDOAS ???   ANLYS_TAB_PAGE *pTabCrossPage;                                                // pointer to the first tab page
// QDOAS ???   HWND  hwndAvailable,hwndSelected;                                             // handles of both listboxes
// QDOAS ???   INT   itemsNumber;                                                            // number of selected items
// QDOAS ???   INDEX indexItem,                                                              // index on selected items
// QDOAS ???         indexSymbol;                                                            // index on symbols in symbol list
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   pTabCrossPage=&ANLYS_tabPages[TAB_TYPE_ANLYS_CROSS];
// QDOAS ???
// QDOAS ???   // Release access to symbols in the list on left
// QDOAS ???
// QDOAS ???   if ((itemsNumber=SendMessage((hwndAvailable=GetDlgItem(hwndXsSet,ANLYS_LIST_AVAILABLE)),LB_GETCOUNT,0,0L))!=0)
// QDOAS ???    {
// QDOAS ???     for  (indexItem=0;indexItem<itemsNumber;indexItem++)
// QDOAS ???      if ((LONG)SendMessage(hwndAvailable,LB_GETTEXT,(WPARAM)indexItem,(LPARAM)itemText) &&
// QDOAS ???          (pTab->symbolReferenceNumber!=NULL) && (pTabCrossPage->symbolReferenceNumber!=NULL) &&
// QDOAS ???         ((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,itemText))!=ITEM_NONE) &&
// QDOAS ???         ((pTab->symbolReferenceNumber[indexSymbol]&(SYMBOL_ALREADY_USED<<8))!=0))
// QDOAS ???       {
// QDOAS ???        pTabCrossPage->symbolReferenceNumber[indexSymbol]--;
// QDOAS ???        pTab->symbolReferenceNumber[indexSymbol]&=~(SYMBOL_ALREADY_USED<<8);
// QDOAS ???        pTab->availableSymbolNumber++;
// QDOAS ???       }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Concatenate all selected items in the receiving string buffer
// QDOAS ???
// QDOAS ???   if ((itemsNumber=SendMessage((hwndSelected=GetDlgItem(hwndXsSet,ANLYS_LIST_SELECTED)),LB_GETCOUNT,0,0L))!=0)
// QDOAS ???    {
// QDOAS ???     memset((pListItemText=ANLYS_itemList[ANLYS_indexXsSet].itemText[0]),0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???     for (indexItem=0;indexItem<itemsNumber;indexItem++)
// QDOAS ???      if ((LONG)SendMessage(hwndSelected,LB_GETTEXT,(WPARAM)indexItem,(LPARAM)itemText) &&
// QDOAS ???         ((ptr=strchr(pListItemText,'\0'))!=NULL))    // go to the end of string
// QDOAS ???       {
// QDOAS ???        if ((pTab->symbolReferenceNumber!=NULL) && (pTabCrossPage->symbolReferenceNumber!=NULL) &&
// QDOAS ???           ((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,itemText))!=ITEM_NONE) &&
// QDOAS ???           ((pTab->symbolReferenceNumber[indexSymbol]&(SYMBOL_ALREADY_USED<<8))==0))
// QDOAS ???         {
// QDOAS ???          pTabCrossPage->symbolReferenceNumber[indexSymbol]++;
// QDOAS ???          pTab->symbolReferenceNumber[indexSymbol]|=(SYMBOL_ALREADY_USED<<8);
// QDOAS ???          pTab->availableSymbolNumber--;
// QDOAS ???         }
// QDOAS ???
// QDOAS ???        if (strlen(pListItemText))
// QDOAS ???         *ptr++=';';
// QDOAS ???        strcpy(ptr,itemText);
// QDOAS ???       }
// QDOAS ???
// QDOAS ???     // Close window
// QDOAS ???
// QDOAS ???     EndDialog(hwndXsSet,TRUE);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysXsSetCommand
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_COMMAND message processing for cross sections set panel
// QDOAS ??? //
// QDOAS ??? // SYNTAX        usual syntax for Windows message processing;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? LRESULT CALLBACK AnlysXsSetCommand(HWND hwndXsSet,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch((ULONG)GET_WM_COMMAND_ID(mp1,mp2))
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case ANLYS_LIST_AVAILABLE :
// QDOAS ???      SendMessage(GetDlgItem(hwndXsSet,ANLYS_LIST_SELECTED),LB_SETSEL,FALSE,(LPARAM)(UINT)-1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case ANLYS_LIST_SELECTED :
// QDOAS ???      SendMessage(GetDlgItem(hwndXsSet,ANLYS_LIST_AVAILABLE),LB_SETSEL,FALSE,(LPARAM)(UINT)-1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case ANLYS_LIST_BUTTON_ADD :        // remove selection from the right list
// QDOAS ???      DOAS_ListMoveSelectedItems(hwndXsSet,ANLYS_LIST_AVAILABLE,ANLYS_LIST_SELECTED);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case ANLYS_LIST_BUTTON_REMOVE :     // remove selection from the left list
// QDOAS ???      DOAS_ListMoveSelectedItems(hwndXsSet,ANLYS_LIST_SELECTED,ANLYS_LIST_AVAILABLE);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDOK :                        // close dialog box on OK button command
// QDOAS ???      AnlysXsSetOK(hwndXsSet,&ANLYS_tabPages[TAB_TYPE_ANLYS_SHIFT_AND_STRETCH]);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDCANCEL :
// QDOAS ???      EndDialog(hwndXsSet,FALSE);       // close dialog box on CANCEL button command
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDHELP :
// QDOAS ???      WinHelp(hwndXsSet,DOAS_HelpPath,HELP_CONTEXT,HLP_ANLYS_TAB_SHIFT_INSERT);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysXsSetWndProc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       dispatch messages from cross sections set panel
// QDOAS ??? //
// QDOAS ??? // SYNTAX        usual syntax for Windows message processing;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? LRESULT CALLBACK AnlysXsSetWndProc(HWND hwndXsSet,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch (msg)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_INITDIALOG :
// QDOAS ???      AnlysXsSetInit(hwndXsSet,GetWindow(hwndXsSet,GW_OWNER),&ANLYS_tabPages[TAB_TYPE_ANLYS_SHIFT_AND_STRETCH],(INDEX)mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_COMMAND :
// QDOAS ???      AnlysXsSetCommand(hwndXsSet,msg,mp1,mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // =========================================================
// QDOAS ??? // ANALYSIS WINDOWS PROPERTIES : COMMAND MESSAGES PROCESSING
// QDOAS ??? // =========================================================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysInsertOneListItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_INSERT command message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndList       handle of the listView control
// QDOAS ??? //               indexTabPage   index of the selected page in the tab control
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysInsertOneListItem(HWND hwndList,INDEX indexTabPage)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR fileName[MAX_ITEM_TEXT_LEN+1];                                          // name of the file associated to a cross section
// QDOAS ???   INDEX indexItem,indexColumn,indexSymbol;                                      // indexes for arrays and loops
// QDOAS ???   LIST_ITEM *pList;                                                             // pointer to the new item in list
// QDOAS ???   ANLYS_TAB_PAGE *pTab;                                                         // pointer to tab page
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pTab=&ANLYS_tabPages[indexTabPage];
// QDOAS ???   memset(fileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   indexSymbol=ITEM_NONE;
// QDOAS ???
// QDOAS ???   // Pick an available item in list
// QDOAS ???
// QDOAS ???   if (ANLYS_itemFree==ITEM_NONE)
// QDOAS ???    MSG_MessageBox(hwndList,ITEM_NONE,MENU_WINDOWS_CHILD_PROJECT,IDS_MSGBOX_INSERT,MB_OK|MB_ICONHAND,"");
// QDOAS ???
// QDOAS ???   // Get a selected file
// QDOAS ???
// QDOAS ???   else if (((indexTabPage!=TAB_TYPE_ANLYS_CROSS) && (indexTabPage!=TAB_TYPE_ANLYS_SHIFT_AND_STRETCH)) ||
// QDOAS ???
// QDOAS ???            ((indexTabPage==TAB_TYPE_ANLYS_CROSS) &&
// QDOAS ???              FILES_Select(DOAS_hwndMain,fileName,MAX_ITEM_TEXT_LEN+1,FILE_TYPE_CROSS,0,FILE_MODE_OPEN,pTab->symbolReferenceNumber,ITEM_NONE,ITEM_NONE) &&
// QDOAS ???            ((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,fileName))!=ITEM_NONE)) ||
// QDOAS ???
// QDOAS ???            ((indexTabPage==TAB_TYPE_ANLYS_SHIFT_AND_STRETCH) &&
// QDOAS ???              DialogBoxParam(DOAS_hInst,MAKEINTRESOURCE(DLG_ANLYS_LIST),hwndList,(DLGPROC)AnlysXsSetWndProc,(LPARAM)ITEM_NONE)))
// QDOAS ???    {
// QDOAS ???     pList=&ANLYS_itemList[(indexItem=AnlysInsertListItem(ANLYS_indexSelected,pTab->listEntryPoint))];
// QDOAS ???
// QDOAS ???     if ((pTab->symbolReferenceNumber!=NULL) && (indexSymbol!=ITEM_NONE))
// QDOAS ???      pTab->symbolReferenceNumber[indexSymbol]|=(SYMBOL_ALREADY_USED<<8);
// QDOAS ???
// QDOAS ???     // Set default values
// QDOAS ???
// QDOAS ???     if ((indexTabPage==TAB_TYPE_ANLYS_GAPS) ||
// QDOAS ???         (indexTabPage==TAB_TYPE_ANLYS_LINEAR) ||
// QDOAS ???         (indexTabPage==TAB_TYPE_ANLYS_NOTLINEAR))
// QDOAS ???
// QDOAS ???      strcpy(pList->itemText[0],pTab->columnList[0].defaultValue);
// QDOAS ???
// QDOAS ???     for (indexColumn=1;indexColumn<pTab->columnNumber;indexColumn++)
// QDOAS ???      strcpy(pList->itemText[indexColumn],pTab->columnList[indexColumn].defaultValue);
// QDOAS ???
// QDOAS ???     // Retrieve symbol name from file
// QDOAS ???
// QDOAS ???     if (strlen(fileName) && (indexSymbol!=ITEM_NONE))
// QDOAS ???      {
// QDOAS ???       FILES_CompactPath(pList->crossFileName,fileName,1,1);
// QDOAS ???       strcpy(pList->itemText[0],pTab->symbolList[indexSymbol].name);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Insert item in ListView control
// QDOAS ???
// QDOAS ???     AnlysInsertListViewItem(hwndList,ListView_GetItemCount(hwndList),indexItem);
// QDOAS ???
// QDOAS ???     pTab->listEntryPoint=indexItem;
// QDOAS ???
// QDOAS ???     // Update number of available symbols
// QDOAS ???
// QDOAS ???     if (indexTabPage!=TAB_TYPE_ANLYS_SHIFT_AND_STRETCH)
// QDOAS ???      pTab->availableSymbolNumber--;
// QDOAS ???
// QDOAS ???     if (indexTabPage==TAB_TYPE_ANLYS_CROSS)
// QDOAS ???      ANLYS_tabPages[TAB_TYPE_ANLYS_SHIFT_AND_STRETCH].availableSymbolNumber++;
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysDeleteOneListItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_DELETE command message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndList       handle of the listView control
// QDOAS ??? //               pTab           pointer to the current tab page
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysDeleteOneListItem(HWND hwndList,ANLYS_TAB_PAGE *pTab)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   ANLYS_TAB_PAGE *pTabCrossPage;                                                // pointer to the firts tab page
// QDOAS ???   UCHAR  symbolName[MAX_ITEM_TEXT_LEN+1],                                       // name of symbol
// QDOAS ???         *ptrNew,*ptrOld;                                                        // pointers to characters in symbol name
// QDOAS ???   LIST_COLUMN *pCol;                                                            // pointer to columns of ListView control
// QDOAS ???   INDEX indexItem,                                                              // index of item to delete in list
// QDOAS ???         indexLvi,                                                               // index of item to delete in ListView control
// QDOAS ???         indexLine,                                                              // index of the currently selected visible line in ListView control
// QDOAS ???         indexColumn,                                                            // browse columns of ListView control
// QDOAS ???         indexSymbol;                                                            // index of item in symbol list
// QDOAS ???   VISIBLE_LINES_IMAGE *pLine;                                                   // pointer to the selected visible line in ListView control
// QDOAS ???   INT itemNumber;                                                               // number of items remaining in ListView control
// QDOAS ???   LIST_ITEM *pList;                                                             // pointer to item to delete in list
// QDOAS ???
// QDOAS ???   // Get index of the item that has the selection
// QDOAS ???
// QDOAS ???   if (((indexItem=AnlysGetSelectedListViewItem(hwndList,&indexLvi,symbolName))!=ITEM_NONE) &&
// QDOAS ???        (MSG_MessageBox(hwndList,ITEM_NONE,MENU_WINDOWS_CHILD_PROJECT,IDS_MSGBOX_DELETE,
// QDOAS ???                         MB_YESNO|MB_ICONQUESTION,
// QDOAS ???                         symbolName)==IDYES))
// QDOAS ???    {
// QDOAS ???     // Initializations
// QDOAS ???
// QDOAS ???     pList=&ANLYS_itemList[indexItem];
// QDOAS ???     indexLine=indexLvi-ListView_GetTopIndex(hwndList);
// QDOAS ???     itemNumber=ListView_GetItemCount(hwndList)-1;
// QDOAS ???
// QDOAS ???     // Combobox processing
// QDOAS ???
// QDOAS ???     for (indexColumn=0;indexColumn<pTab->columnNumber;indexColumn++)
// QDOAS ???      {
// QDOAS ???       pCol=&pTab->columnList[indexColumn];
// QDOAS ???
// QDOAS ???       if (((pCol->comboboxItemType==COMBOBOX_ITEM_TYPE_ORTHOGONAL) ||
// QDOAS ???            (pCol->comboboxItemType==COMBOBOX_ITEM_TYPE_EXCLUSIVE)) &&
// QDOAS ???            (pCol->displayNumber>=pTab->minDisplayNumber) &&
// QDOAS ???            (pCol->displayNumber<=pTab->maxDisplayNumber) &&
// QDOAS ???            (pTab->symbolReferenceNumber!=NULL) &&
// QDOAS ???           ((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,pList->itemText[indexColumn]))!=ITEM_NONE))
// QDOAS ???
// QDOAS ???         pTab->symbolReferenceNumber[indexSymbol]=(pCol->comboboxItemType==COMBOBOX_ITEM_TYPE_ORTHOGONAL)?
// QDOAS ???         pTab->symbolReferenceNumber[indexSymbol]-1:
// QDOAS ???         pTab->symbolReferenceNumber[indexSymbol]&(~(SYMBOL_ALREADY_USED<<8));
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Release access to symbols implied
// QDOAS ???
// QDOAS ???     if (ANLYS_indexTabPage!=TAB_TYPE_ANLYS_SHIFT_AND_STRETCH)
// QDOAS ???      {
// QDOAS ???       if (ANLYS_indexTabPage==TAB_TYPE_ANLYS_CROSS)
// QDOAS ???        ANLYS_tabPages[TAB_TYPE_ANLYS_SHIFT_AND_STRETCH].availableSymbolNumber--;
// QDOAS ???
// QDOAS ???       if ((ANLYS_indexTabPage!=TAB_TYPE_ANLYS_GAPS) && (pTab->symbolReferenceNumber!=NULL) &&
// QDOAS ???          ((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,symbolName))!=ITEM_NONE))
// QDOAS ???
// QDOAS ???        pTab->symbolReferenceNumber[indexSymbol]=0;
// QDOAS ???        pTab->availableSymbolNumber++;
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???       pTabCrossPage=&ANLYS_tabPages[TAB_TYPE_ANLYS_CROSS];
// QDOAS ???       ptrOld=pList->itemText[0];
// QDOAS ???
// QDOAS ???       while (ptrOld!=NULL)
// QDOAS ???        {
// QDOAS ???         if ((ptrNew=strchr(ptrOld,';'))!=NULL)
// QDOAS ???          *ptrNew++='\0';
// QDOAS ???
// QDOAS ???         if ((pTab->symbolReferenceNumber!=NULL) && (pTabCrossPage->symbolReferenceNumber!=NULL) &&
// QDOAS ???            ((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,ptrOld))!=ITEM_NONE))
// QDOAS ???          {
// QDOAS ???           pTabCrossPage->symbolReferenceNumber[indexSymbol]--;
// QDOAS ???           pTab->symbolReferenceNumber[indexSymbol]&=~(SYMBOL_ALREADY_USED<<8);
// QDOAS ???
// QDOAS ???           pTab->availableSymbolNumber++;
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         ptrOld=ptrNew;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Redirect pointers
// QDOAS ???
// QDOAS ???     if (pTab->listEntryPoint==indexItem)
// QDOAS ???      pTab->listEntryPoint=pList->indexPrevious;
// QDOAS ???
// QDOAS ???     if (pList->indexPrevious!=ITEM_NONE)
// QDOAS ???      ANLYS_itemList[pList->indexPrevious].indexNext=pList->indexNext;
// QDOAS ???
// QDOAS ???     if (pList->indexNext!=ITEM_NONE)
// QDOAS ???      ANLYS_itemList[pList->indexNext].indexPrevious=pList->indexPrevious;
// QDOAS ???
// QDOAS ???     // Put item in free list
// QDOAS ???
// QDOAS ???     memset(&ANLYS_itemList[indexItem],0,sizeof(LIST_ITEM));
// QDOAS ???
// QDOAS ???     ANLYS_itemList[ANLYS_itemFree].indexPrevious=indexItem;
// QDOAS ???     ANLYS_itemList[indexItem].indexNext=ANLYS_itemFree;
// QDOAS ???     ANLYS_itemList[indexItem].indexPrevious=ITEM_NONE;
// QDOAS ???     ANLYS_itemFree=indexItem;
// QDOAS ???
// QDOAS ???     // Remove item from ListView control
// QDOAS ???
// QDOAS ???     ListView_DeleteItem(hwndList,indexLvi);
// QDOAS ???
// QDOAS ???     // Remove extra controls associated to lines in ListView
// QDOAS ???
// QDOAS ???     if (indexLine<ANLYS_maxLines)
// QDOAS ???      {
// QDOAS ???       if (itemNumber<ANLYS_maxLines)
// QDOAS ???        {
// QDOAS ???         pLine=&pTab->visibleLinesImage[itemNumber];
// QDOAS ???
// QDOAS ???         for (indexColumn=0;indexColumn<pTab->columnNumber;indexColumn++)
// QDOAS ???          if (pLine->hwnd[indexColumn]!=NULL)
// QDOAS ???           ShowWindow(pLine->hwnd[indexColumn],SW_HIDE);
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       if (indexLine==itemNumber)
// QDOAS ???        indexLine--;
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      indexLine=ANLYS_maxLines-1;
// QDOAS ???
// QDOAS ???     // Set the new selection
// QDOAS ???
// QDOAS ???     if (itemNumber)
// QDOAS ???      ListView_SetItemState(hwndList,
// QDOAS ???                            indexLine+ListView_GetTopIndex(hwndList),
// QDOAS ???                            LVIS_FOCUSED|LVIS_SELECTED,
// QDOAS ???                            LVIS_FOCUSED|LVIS_SELECTED);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysProperties
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_PROPERTIES command message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndList       handle of the listView control
// QDOAS ??? //               pTab           pointer to the current tab page
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysProperties(HWND hwndList,ANLYS_TAB_PAGE *pTab)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR fileName[MAX_ITEM_TEXT_LEN+1],                                          // name of the selected file
// QDOAS ???         symbolName[MAX_ITEM_TEXT_LEN+1];                                        // name of the selected symbol
// QDOAS ???   INDEX indexSymbol,                                                            // index of symbol in symbols list
// QDOAS ???         indexItem,                                                              // index of selected item in list
// QDOAS ???         indexLvi;                                                               // index of selected item in ListView control
// QDOAS ???
// QDOAS ???   // Get index of the item that has the selection
// QDOAS ???
// QDOAS ???   if ((indexItem=AnlysGetSelectedListViewItem(hwndList,&indexLvi,symbolName))!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     FILES_RebuildFileName(fileName,ANLYS_itemList[indexItem].crossFileName,1);
// QDOAS ???
// QDOAS ???     if (((ANLYS_indexTabPage==TAB_TYPE_ANLYS_CROSS) ||
// QDOAS ???         ((ANLYS_indexTabPage==TAB_TYPE_ANLYS_NOTLINEAR) &&
// QDOAS ???         (!STD_Stricmp(symbolName,SYMB_itemCrossList[SYMBOL_PREDEFINED_COM].name) ||
// QDOAS ???          !STD_Stricmp(symbolName,SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP1].name) ||
// QDOAS ???          !STD_Stricmp(symbolName,SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP2].name) ||
// QDOAS ???          !STD_Stricmp(symbolName,SYMB_itemCrossList[SYMBOL_PREDEFINED_RING1].name)))) &&
// QDOAS ???
// QDOAS ???        ((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,symbolName))!=ITEM_NONE) &&
// QDOAS ???          FILES_Select(hwndList,fileName,MAX_ITEM_TEXT_LEN+1,FILE_TYPE_CROSS,0,FILE_MODE_OPEN,pTab->symbolReferenceNumber,indexSymbol,ITEM_NONE) &&
// QDOAS ???        ((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,fileName))!=ITEM_NONE))
// QDOAS ???
// QDOAS ???      FILES_ChangePath(ANLYS_itemList[indexItem].crossFileName,fileName,1);
// QDOAS ???
// QDOAS ???     else if (ANLYS_indexTabPage==TAB_TYPE_ANLYS_SHIFT_AND_STRETCH)
// QDOAS ???      DialogBoxParam(DOAS_hInst,MAKEINTRESOURCE(DLG_ANLYS_LIST),hwndList,(DLGPROC)AnlysXsSetWndProc,(LPARAM)indexItem);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysPropertiesAmf
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_PROPERTIES_AMF command message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndList       handle of the listView control
// QDOAS ??? //               pTab           pointer to the current tab page
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysPropertiesAmf(HWND hwndList,ANLYS_TAB_PAGE *pTab)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   LIST_ITEM *pList;                                                             // pointer to the selected item in the list
// QDOAS ???   UCHAR fileName[MAX_ITEM_TEXT_LEN+1],                                          // name of the selected file
// QDOAS ???         symbolName[MAX_ITEM_TEXT_LEN+1],                                        // name of the selected symbol
// QDOAS ???         fileType,                                                               // type of file
// QDOAS ???        *amfType,                                                                // type of amf
// QDOAS ???        *ptr;                                                                    // pointer to a part of string
// QDOAS ???   INDEX indexSymbol,                                                            // index of symbol in symbols list
// QDOAS ???         indexItem,                                                              // index of selected item in list
// QDOAS ???         indexLvi;                                                               // index of selected item in ListView control
// QDOAS ???
// QDOAS ???   // Get index of the item that has the selection
// QDOAS ???
// QDOAS ???   if ((indexItem=AnlysGetSelectedListViewItem(hwndList,&indexLvi,symbolName))!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     // Pointers initializations
// QDOAS ???
// QDOAS ???     pList=&ANLYS_itemList[indexItem];
// QDOAS ???     amfType=pList->itemText[COLUMN_CROSS_AMF_TYPE];
// QDOAS ???
// QDOAS ???     // Retrieve the type of file
// QDOAS ???
// QDOAS ???     if (!STD_Stricmp(amfType,ANLYS_amf[ANLYS_AMF_TYPE_SZA]))
// QDOAS ???      fileType=FILE_TYPE_AMF_SZA;
// QDOAS ???     else if (!STD_Stricmp(amfType,ANLYS_amf[ANLYS_AMF_TYPE_CLIMATOLOGY]))
// QDOAS ???      fileType=FILE_TYPE_AMF_CLI;
// QDOAS ???     else
// QDOAS ???      fileType=FILE_TYPE_AMF_WVE;
// QDOAS ???
// QDOAS ???     // Make a copy of amf file
// QDOAS ???
// QDOAS ???     FILES_RebuildFileName(fileName,pList->amfFileName,1);
// QDOAS ???
// QDOAS ???     // Compare extension of amf file with the choosen amf type
// QDOAS ???
// QDOAS ???     if (((ptr=strrchr(fileName,'.'))!=NULL) &&
// QDOAS ???          (strlen(++ptr)==strlen((amfType=FILES_types[fileType].fileExt))) &&
// QDOAS ???           strnicmp(ptr,amfType,strlen(amfType)))
// QDOAS ???      memset(fileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???     // Open common dialog box for selecting a new amf file
// QDOAS ???
// QDOAS ???     if (((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,symbolName))!=ITEM_NONE) &&
// QDOAS ???           FILES_Select(hwndList,fileName,MAX_ITEM_TEXT_LEN+1,fileType,0,FILE_MODE_OPEN,pTab->symbolReferenceNumber,indexSymbol,ITEM_NONE) &&
// QDOAS ???         ((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,fileName))!=ITEM_NONE))
// QDOAS ???
// QDOAS ???     // New amf file name safe keeping
// QDOAS ???
// QDOAS ???      FILES_ChangePath(pList->amfFileName,fileName,1);
// QDOAS ??? //     strcpy(pList->amfFileName,fileName);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysSetRefSelection
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       ANLYS_REF_SELECTION_AUTOMATIC/ANLYS_REF_SELECTION_FILE command message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndAnlys      handle of the analysis windows properties dialog box
// QDOAS ??? //               pAnlys         pointer to the structure with analysis windows properties
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysSetRefSelection(HWND hwndAnlys,ANALYSIS_WINDOWS *pAnlys)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INT specOnly,refSpec,fileAction,szaAction,latAction,useKurucz,gomeAction;
// QDOAS ???
// QDOAS ???   // Interesting fields read out
// QDOAS ???
// QDOAS ???   ANLYS_RefSpectrumSelectionMode=
// QDOAS ???
// QDOAS ???        (IsDlgButtonChecked(hwndAnlys,ANLYS_REF_SELECTION_FILE))?
// QDOAS ???        (INT)ANLYS_REF_SELECTION_MODE_FILE:(INT)ANLYS_REF_SELECTION_MODE_AUTOMATIC;
// QDOAS ???
// QDOAS ???   useKurucz=SendMessage(GetDlgItem(hwndAnlys,ANLYS_REF_KURUCZ_SELECTION),CB_GETCURSEL,0,0);
// QDOAS ???
// QDOAS ???   // Set action for file and sza fields
// QDOAS ???
// QDOAS ???   refSpec=((useKurucz==ANLYS_KURUCZ_REF_AND_SPEC)||(useKurucz==ANLYS_KURUCZ_SPEC))?FALSE:TRUE;
// QDOAS ???   specOnly=(useKurucz==ANLYS_KURUCZ_SPEC)?FALSE:TRUE;
// QDOAS ???   fileAction=(ANLYS_RefSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE)?SW_SHOW:SW_HIDE;
// QDOAS ???   szaAction=(ANLYS_RefSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE)?SW_HIDE:SW_SHOW;
// QDOAS ???
// QDOAS ???   latAction=((ANLYS_RefSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE) ||
// QDOAS ???             ((PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GDP_ASCII) &&
// QDOAS ???              (PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GDP_BIN) &&
// QDOAS ???              (PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_HDF) &&
// QDOAS ???              (PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_SCIA_PDS) ||
// QDOAS ???              (PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GOME2)))?SW_HIDE:SW_SHOW;
// QDOAS ???
// QDOAS ???   gomeAction=((ANLYS_RefSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE) ||
// QDOAS ???             ((PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GDP_ASCII) &&
// QDOAS ???              (PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GDP_BIN)))?SW_HIDE:SW_SHOW;
// QDOAS ???
// QDOAS ???   // Show/hide file fields
// QDOAS ???
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SPECTRUM),fileAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SPECTRUM_BUTTON),fileAction);
// QDOAS ???
// QDOAS ???   // Show/hide sza field
// QDOAS ???
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SZA_TEXT),szaAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SZA_VALUE),szaAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SZA_PLUS),szaAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SZA_DELTA),szaAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_LAT_TEXT),latAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_LAT1),latAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_LAT2),latAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_LON_TEXT),latAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_LON1),latAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_LON2),latAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_NSPECTRA_TEXT),latAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_REF_NSPECTRA),latAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_GOME_0),gomeAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_GOME_1),gomeAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_GOME_2),gomeAction);
// QDOAS ???   ShowWindow(GetDlgItem(hwndAnlys,ANLYS_GOME_3),gomeAction);
// QDOAS ???
// QDOAS ???   if (gomeAction==SW_SHOW)
// QDOAS ???    {
// QDOAS ???     ShowWindow(GetDlgItem(hwndAnlys,ANLYS_RESIDUALS_FILE),SW_HIDE);
// QDOAS ???     ShowWindow(GetDlgItem(hwndAnlys,ANLYS_RESIDUALS_BUTTON),SW_HIDE);
// QDOAS ???     SetWindowText(GetDlgItem(hwndAnlys,ANLYS_RESIDUALS_TEXT),"Pixel type");
// QDOAS ???    }
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     ShowWindow(GetDlgItem(hwndAnlys,ANLYS_RESIDUALS_FILE),SW_SHOW);
// QDOAS ???     ShowWindow(GetDlgItem(hwndAnlys,ANLYS_RESIDUALS_BUTTON),SW_SHOW);
// QDOAS ???     SetWindowText(GetDlgItem(hwndAnlys,ANLYS_RESIDUALS_TEXT),"Residuals");
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_FILE_TEXT1),refSpec);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_ETALON),refSpec);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_ETALON_BUTTON),refSpec);
// QDOAS ???
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SELECTION_GROUP),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SELECTION_AUTOMATIC),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SELECTION_FILE),specOnly);
// QDOAS ???
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_FILE_TEXT2),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SPECTRUM),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SPECTRUM_BUTTON),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SZA_VALUE),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SZA_TEXT),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SZA_DELTA),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_SZA_PLUS),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_LAT_TEXT),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_LAT1),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_LAT2),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_LON_TEXT),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_LON1),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_LON2),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_NSPECTRA_TEXT),specOnly);
// QDOAS ???   EnableWindow(GetDlgItem(hwndAnlys,ANLYS_REF_NSPECTRA),specOnly);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysDisplayFits
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       ANLYS_DISPLAY_FITS command message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndAnlys      handle of the analysis windows properties dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysDisplayFits(HWND hwndAnlys)
// QDOAS ???  {
// QDOAS ???   ANLYS_displayFits=(IsDlgButtonChecked(hwndAnlys,ANLYS_DISPLAY_FITS)==BST_CHECKED)?1:0;
// QDOAS ???
// QDOAS ???   if (ANLYS_indexTabPage==TAB_TYPE_ANLYS_CROSS)
// QDOAS ???    SendMessage(hwndAnlys,WM_PAINT,(WPARAM)0,(LPARAM)0);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysGetRefFiles
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       ANLYS_REF_SPECTRUM_BUTTON/ANLYS_REF_ETALON_BUTTON command message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndAnlys      handle of the analysis windows properties dialog box
// QDOAS ??? //               controlID      id of the calling button control
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysGetRefFiles(HWND hwndAnlys,ULONG controlID)
// QDOAS ???  {
// QDOAS ???   UCHAR fileSelected[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???
// QDOAS ???   strcpy(fileSelected,(controlID==ANLYS_REF_SPECTRUM)?ANLYS_refFileName:ANLYS_refEtalonFileName);
// QDOAS ???
// QDOAS ???   FILES_Select(hwndAnlys,fileSelected,MAX_ITEM_TEXT_LEN+1,FILE_TYPE_REF,0,FILE_MODE_OPEN,NULL,ITEM_NONE,ITEM_NONE);
// QDOAS ???
// QDOAS ???   if (strlen(fileSelected))
// QDOAS ???    SetWindowText(GetDlgItem(hwndAnlys,controlID),fileSelected);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysResidualFile
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       ANLYS_RESIDUALS_BUTTON command message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndAnlys      handle of the analysis windows properties dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysResidualFile(HWND hwndAnlys)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR path[MAX_ITEM_TEXT_LEN+1],*ptr;
// QDOAS ???   SZ_LEN pathLength;
// QDOAS ???   UCHAR pathSep[2];
// QDOAS ???
// QDOAS ???   FILES_RebuildFileName(path,ANLYS_windowsList[ANLYS_indexSelected].residualsFile,1);
// QDOAS ???
// QDOAS ???   pathSep[0]=PATH_SEP;
// QDOAS ???   pathSep[1]='\0';
// QDOAS ???
// QDOAS ???   // Set automatic path
// QDOAS ???
// QDOAS ???   if (!strlen(path))
// QDOAS ???    {
// QDOAS ???     GetCurrentDirectory(sizeof(path),path);
// QDOAS ???     strcat(path,pathSep);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if (((pathLength=strlen(path))!=0) &&
// QDOAS ???        (path[pathLength-1]==PATH_SEP))
// QDOAS ???    strcat(path,"automatic");
// QDOAS ???
// QDOAS ???   // Select path and file for results
// QDOAS ???
// QDOAS ???   if (FILES_Select(hwndAnlys,path,MAX_ITEM_TEXT_LEN+1,FILE_TYPE_RES,0,FILE_MODE_SAVE,NULL,ITEM_NONE,ITEM_NONE))
// QDOAS ???    {
// QDOAS ???     // Returned path processing
// QDOAS ???
// QDOAS ???     if (!STD_Stricmp((ptr=path),"automatic") ||
// QDOAS ???       (((ptr=strrchr(path,PATH_SEP))!=NULL) &&
// QDOAS ???         (strlen(++ptr)!=0) &&
// QDOAS ???         !STD_Stricmp(ptr,"automatic")))
// QDOAS ???      *ptr=0;
// QDOAS ???
// QDOAS ???     SetWindowText(GetDlgItem(hwndAnlys,ANLYS_RESIDUALS_FILE),path);
// QDOAS ???     strcpy(ANLYS_windowsList[ANLYS_indexSelected].residualsFile,path);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysOK
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       IDOK command message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndAnlys      handle of the analysis windows properties dialog box
// QDOAS ??? //               pAnlys         pointer to the structure with analysis windows properties
// QDOAS ??? //               tabPages       list of tab pages for the selected analysis windows
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysOK(HWND hwndAnlys,ANALYSIS_WINDOWS *pAnlys,ANLYS_TAB_PAGE *tabPages)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR refEtalon[MAX_ITEM_TEXT_LEN+1],lembdaMin[13],lembdaMax[13],lembdaMinK[13],lembdaMaxK[13],
// QDOAS ???         szaValue[13],szaDiff[13],refLatMin[13],refLatMax[13],refLonMin[13],refLonMax[13];
// QDOAS ???   INDEX indexTab,indexListItem,indexTreeItem,treeEntryPoint;
// QDOAS ???   INT refSelectionMode,useKurucz;
// QDOAS ???   ULONG controlID;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   memset(refEtalon,0,sizeof(refEtalon));
// QDOAS ???   memset(lembdaMin,0,sizeof(lembdaMin));
// QDOAS ???   memset(lembdaMax,0,sizeof(lembdaMax));
// QDOAS ???   memset(lembdaMinK,0,sizeof(lembdaMinK));
// QDOAS ???   memset(lembdaMaxK,0,sizeof(lembdaMaxK));
// QDOAS ???   memset(szaValue,0,sizeof(szaValue));
// QDOAS ???   memset(szaDiff,0,sizeof(szaDiff));
// QDOAS ???   memset(refLatMin,0,sizeof(refLatMin));
// QDOAS ???   memset(refLatMax,0,sizeof(refLatMax));
// QDOAS ???   memset(refLonMin,0,sizeof(refLonMin));
// QDOAS ???   memset(refLonMax,0,sizeof(refLonMax));
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Update list
// QDOAS ???
// QDOAS ???   AnlysUpdateList(GetDlgItem(hwndAnlys,ANLYS_LISTVIEW),&ANLYS_tabPages[ANLYS_indexTabPage]);
// QDOAS ???
// QDOAS ???   if (!pAnlys->hidden)
// QDOAS ???    {
// QDOAS ???     refSelectionMode=
// QDOAS ???
// QDOAS ???        (IsDlgButtonChecked(hwndAnlys,ANLYS_REF_SELECTION_FILE))?
// QDOAS ???        (INT)ANLYS_REF_SELECTION_MODE_FILE:(INT)ANLYS_REF_SELECTION_MODE_AUTOMATIC;
// QDOAS ???
// QDOAS ???     useKurucz=(UCHAR)SendMessage(GetDlgItem(hwndAnlys,ANLYS_REF_KURUCZ_SELECTION),CB_GETCURSEL,0,0);
// QDOAS ???
// QDOAS ???     GetWindowText(GetDlgItem(hwndAnlys,ANLYS_REF_ETALON),ANLYS_refEtalonFileName,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???     GetWindowText(GetDlgItem(hwndAnlys,(controlID=ANLYS_REF_SZA_VALUE)),szaValue,sizeof(szaValue));
// QDOAS ???     GetWindowText(GetDlgItem(hwndAnlys,(controlID=ANLYS_REF_SZA_DELTA)),szaDiff,sizeof(szaDiff));
// QDOAS ???
// QDOAS ???     // Reference spectrum read out
// QDOAS ???
// QDOAS ??? //    if (((refSelectionMode==ANLYS_REF_SELECTION_MODE_FILE) || !useKurucz) && !strlen(refEtalon))
// QDOAS ???
// QDOAS ??? //     MSG_MessageBox(hwndAnlys,ANLYS_REF_ETALON_BUTTON,IDS_TITLE_ANALYSIS_WINDOWS,(rc=IDS_MSGBOX_FIELDEMPTY),MB_OK|MB_ICONHAND,"Reference 1");
// QDOAS ???
// QDOAS ??? /* else */ if ((refSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) && (useKurucz!=ANLYS_KURUCZ_SPEC) &&
// QDOAS ???             (!strlen(szaValue) ||
// QDOAS ???              !strlen(szaDiff)))
// QDOAS ???
// QDOAS ???      MSG_MessageBox(hwndAnlys,controlID,IDS_TITLE_ANALYSIS_WINDOWS,(rc=IDS_MSGBOX_FIELDEMPTY),MB_OK|MB_ICONHAND,"SZA data");
// QDOAS ???
// QDOAS ???     else if ((refSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) && (useKurucz!=ANLYS_KURUCZ_SPEC) &&
// QDOAS ???             ((PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
// QDOAS ???              (PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
// QDOAS ???              (PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
// QDOAS ???              (PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
// QDOAS ???              (PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&
// QDOAS ???             (!GetWindowText(GetDlgItem(hwndAnlys,(controlID=ANLYS_REF_LAT1)),refLatMin,sizeof(refLatMin)) ||
// QDOAS ???              !GetWindowText(GetDlgItem(hwndAnlys,(controlID=ANLYS_REF_LAT2)),refLatMax,sizeof(refLatMax)) ||
// QDOAS ???              !GetWindowText(GetDlgItem(hwndAnlys,(controlID=ANLYS_REF_LON1)),refLonMin,sizeof(refLonMin)) ||
// QDOAS ???              !GetWindowText(GetDlgItem(hwndAnlys,(controlID=ANLYS_REF_LON2)),refLonMax,sizeof(refLonMax))))
// QDOAS ???
// QDOAS ???      MSG_MessageBox(hwndAnlys,controlID,IDS_TITLE_ANALYSIS_WINDOWS,(rc=IDS_MSGBOX_FIELDEMPTY),MB_OK|MB_ICONHAND,"Latitude data");
// QDOAS ???
// QDOAS ???     else if ((refSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) && (useKurucz!=ANLYS_KURUCZ_SPEC) &&
// QDOAS ???             ((PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
// QDOAS ???              (PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
// QDOAS ???              (PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
// QDOAS ???              (PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
// QDOAS ???              (PRJCT_itemList[ANLYS_indexProject].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2)) &&
// QDOAS ???             ((pAnlys->nspectra=(INT)GetDlgItemInt(hwndAnlys,(controlID=ANLYS_REF_NSPECTRA),NULL,FALSE))<=0))
// QDOAS ???
// QDOAS ???      MSG_MessageBox(hwndAnlys,controlID,IDS_TITLE_ANALYSIS_WINDOWS,(rc=IDS_MSGBOX_POSITIVE),MB_OK|MB_ICONHAND,"NSpectra");
// QDOAS ???
// QDOAS ???     // Minimum value for window
// QDOAS ???
// QDOAS ???     else if (!GetWindowText(GetDlgItem(hwndAnlys,ANLYS_LIMITS_MIN),lembdaMin,sizeof(lembdaMin)))
// QDOAS ???      MSG_MessageBox(hwndAnlys,ANLYS_LIMITS_MIN,IDS_TITLE_ANALYSIS_WINDOWS,
// QDOAS ???                     (rc=IDS_MSGBOX_FIELDEMPTY),MB_OK|MB_ICONHAND,
// QDOAS ???                     "Minimum");
// QDOAS ???
// QDOAS ???     // Maximum value for window
// QDOAS ???
// QDOAS ???     else if (!GetWindowText(GetDlgItem(hwndAnlys,ANLYS_LIMITS_MAX),lembdaMax,sizeof(lembdaMax)))
// QDOAS ???      MSG_MessageBox(hwndAnlys,ANLYS_LIMITS_MAX,IDS_TITLE_ANALYSIS_WINDOWS,
// QDOAS ???                     (rc=IDS_MSGBOX_FIELDEMPTY),MB_OK|MB_ICONHAND,
// QDOAS ???                     "Maximum");
// QDOAS ???
// QDOAS ???     if (rc==ERROR_ID_NO)
// QDOAS ???      {
// QDOAS ???       // Reference spectra
// QDOAS ???
// QDOAS ???       strcpy(pAnlys->refEtalon,refEtalon);
// QDOAS ???
// QDOAS ???       if (!GetWindowText(GetDlgItem(hwndAnlys,ANLYS_REF_SPECTRUM),ANLYS_refFileName,MAX_ITEM_TEXT_LEN+1))
// QDOAS ???        memset(pAnlys->refSpectrumFile,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???       if (!GetWindowText(GetDlgItem(hwndAnlys,ANLYS_RESIDUALS_FILE),ANLYS_residuals,MAX_ITEM_TEXT_LEN+1))
// QDOAS ???        memset(pAnlys->residualsFile,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???       FILES_ChangePath(pAnlys->refEtalon,ANLYS_refEtalonFileName,1);
// QDOAS ???       FILES_ChangePath(pAnlys->refSpectrumFile,ANLYS_refFileName,1);
// QDOAS ???       FILES_ChangePath(pAnlys->residualsFile,ANLYS_residuals,1);
// QDOAS ???
// QDOAS ???       pAnlys->refSZA=(double)atof(szaValue);
// QDOAS ???       pAnlys->refSZADelta=(double)atof(szaDiff);
// QDOAS ???       pAnlys->refLatMin=(double)atof(refLatMin);
// QDOAS ???       pAnlys->refLatMax=(double)atof(refLatMax);
// QDOAS ???       pAnlys->refLonMin=(double)atof(refLonMin);
// QDOAS ???       pAnlys->refLonMax=(double)atof(refLonMax);
// QDOAS ???
// QDOAS ???       pAnlys->refSpectrumSelectionMode=refSelectionMode;
// QDOAS ???       pAnlys->useKurucz=useKurucz;
// QDOAS ???
// QDOAS ???       // Window limits
// QDOAS ???
// QDOAS ???       strcpy(pAnlys->lembdaMin,lembdaMin);
// QDOAS ???       strcpy(pAnlys->lembdaMax,lembdaMax);
// QDOAS ???       strcpy(pAnlys->lembdaMinK,lembdaMinK);
// QDOAS ???       strcpy(pAnlys->lembdaMaxK,lembdaMaxK);
// QDOAS ???
// QDOAS ???       // Spectra to display
// QDOAS ???
// QDOAS ???       pAnlys->displaySpectrum=(UCHAR)(IsDlgButtonChecked(hwndAnlys,ANLYS_DISPLAY_SPECTRUM)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???       pAnlys->displayResidue=(UCHAR)(IsDlgButtonChecked(hwndAnlys,ANLYS_DISPLAY_RESIDUE)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???       pAnlys->displayTrend=(UCHAR)(IsDlgButtonChecked(hwndAnlys,ANLYS_DISPLAY_TREND)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???       pAnlys->displayRefEtalon=(UCHAR)(IsDlgButtonChecked(hwndAnlys,ANLYS_DISPLAY_REFETALON)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???       pAnlys->displayFits=(UCHAR)(IsDlgButtonChecked(hwndAnlys,ANLYS_DISPLAY_FITS)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???       pAnlys->displayPredefined=(UCHAR)(IsDlgButtonChecked(hwndAnlys,ANLYS_DISPLAY_PREDEFINED)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???
// QDOAS ???       pAnlys->gomePixelType[0]=(UCHAR)(IsDlgButtonChecked(hwndAnlys,ANLYS_GOME_0)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???       pAnlys->gomePixelType[1]=(UCHAR)(IsDlgButtonChecked(hwndAnlys,ANLYS_GOME_1)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???       pAnlys->gomePixelType[2]=(UCHAR)(IsDlgButtonChecked(hwndAnlys,ANLYS_GOME_2)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???       pAnlys->gomePixelType[3]=(UCHAR)(IsDlgButtonChecked(hwndAnlys,ANLYS_GOME_3)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if (rc==ERROR_ID_NO)
// QDOAS ???    {
// QDOAS ???     // List items in tab pages
// QDOAS ???
// QDOAS ???     for (indexTab=0;indexTab<ANLYS_maxTabPage;indexTab++)
// QDOAS ???      {
// QDOAS ???       if (indexTab!=TAB_TYPE_ANLYS_OUTPUT)
// QDOAS ???        {
// QDOAS ???         if ((tabPages[indexTab].pTreeEntryPoint!=NULL) &&
// QDOAS ???             (tabPages[indexTab].pTreeEntryPoint!=NULL) &&
// QDOAS ???            ((treeEntryPoint=*tabPages[indexTab].pTreeEntryPoint)!=ITEM_NONE))
// QDOAS ???          {
// QDOAS ???           // Release access to symbols in use before modifications
// QDOAS ???
// QDOAS ???           for (indexListItem=tabPages[indexTab].oldListEntryPoint;
// QDOAS ???                indexListItem!=ITEM_NONE;
// QDOAS ???                indexListItem=ANLYS_itemListBackup[indexListItem].indexPrevious)
// QDOAS ???
// QDOAS ???            if ((indexTreeItem=TREE_GetIndexByDataName(ANLYS_itemListBackup[indexListItem].itemText[0],tabPages[indexTab].symbolType,treeEntryPoint))!=ITEM_NONE)
// QDOAS ???             TREE_itemList[indexTreeItem].useCount--;
// QDOAS ???
// QDOAS ???           // Take access to new symbols into account
// QDOAS ???
// QDOAS ???           for (indexListItem=tabPages[indexTab].listEntryPoint;
// QDOAS ???               indexListItem!=ITEM_NONE;
// QDOAS ???               indexListItem=ANLYS_itemList[indexListItem].indexPrevious)
// QDOAS ???
// QDOAS ???           if ((indexTreeItem=TREE_GetIndexByDataName(ANLYS_itemList[indexListItem].itemText[0],tabPages[indexTab].symbolType,treeEntryPoint))!=ITEM_NONE)
// QDOAS ???            TREE_itemList[indexTreeItem].useCount++;
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // New list entry point
// QDOAS ???
// QDOAS ???       pAnlys->listEntryPoint[indexTab]=tabPages[indexTab].listEntryPoint;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Close on OK command
// QDOAS ???
// QDOAS ???     if (!TLBAR_bSaveFlag)
// QDOAS ???      TLBAR_Enable(TRUE);
// QDOAS ???
// QDOAS ???     EndDialog(hwndAnlys,0);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysCancel
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       IDCANCEL command message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndAnlys      handle of the analysis windows properties dialog box
// QDOAS ??? //               pAnlys         pointer to the structure with analysis windows properties
// QDOAS ??? //               tabPages       list of tab pages for the selected analysis windows
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysCancel(HWND hwndAnlys,ANALYSIS_WINDOWS *pAnlys,ANLYS_TAB_PAGE *tabPages)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexTab,indexListItem,indexNewItem,indexOldItem,listEntryPoint;
// QDOAS ???   LIST_ITEM *pList;
// QDOAS ???
// QDOAS ???   for (indexTab=0;indexTab<ANLYS_maxTabPage;indexTab++)
// QDOAS ???    {
// QDOAS ???     if (indexTab!=TAB_TYPE_ANLYS_OUTPUT)
// QDOAS ???      {
// QDOAS ???       // Release access to items currently in use
// QDOAS ???
// QDOAS ???       if ((listEntryPoint=tabPages[indexTab].listEntryPoint)!=ITEM_NONE)
// QDOAS ???        {
// QDOAS ???         if ((ANLYS_itemList[listEntryPoint].indexNext=ANLYS_itemFree)!=ITEM_NONE)
// QDOAS ???          ANLYS_itemList[ANLYS_itemFree].indexPrevious=listEntryPoint;
// QDOAS ???         ANLYS_itemFree=listEntryPoint;
// QDOAS ???
// QDOAS ???         for (indexListItem=listEntryPoint;
// QDOAS ???              indexListItem!=ITEM_NONE;
// QDOAS ???              ANLYS_itemFree=indexListItem,
// QDOAS ???              indexListItem=ANLYS_itemList[indexListItem].indexPrevious)
// QDOAS ???          {
// QDOAS ???           // reset content of items
// QDOAS ???
// QDOAS ???           memset(&ANLYS_itemList[indexListItem],0,
// QDOAS ???                 (MAX_LIST_COLUMNS+2)*(MAX_ITEM_TEXT_LEN+1));
// QDOAS ???
// QDOAS ???           // put list on the top of free list
// QDOAS ???
// QDOAS ???           ANLYS_itemList[indexListItem].indexParent=ITEM_NONE;
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Reinsert in list items in use before modifications
// QDOAS ???
// QDOAS ???       if ((tabPages[indexTab].oldListEntryPoint!=ITEM_NONE) &&
// QDOAS ???          ((pAnlys->listEntryPoint[indexTab]=ANLYS_itemFree)!=ITEM_NONE))
// QDOAS ???
// QDOAS ???        for (indexListItem=tabPages[indexTab].oldListEntryPoint,indexOldItem=ITEM_NONE;
// QDOAS ???            (indexListItem!=ITEM_NONE) && (ANLYS_itemFree!=ITEM_NONE);
// QDOAS ???             indexListItem=ANLYS_itemListBackup[indexListItem].indexPrevious)
// QDOAS ???         {
// QDOAS ???          pList=&ANLYS_itemList[(indexNewItem=ANLYS_itemFree)];
// QDOAS ???
// QDOAS ???          // Update list of available items
// QDOAS ???
// QDOAS ???          ANLYS_itemFree=pList->indexNext;                                       // First item of the list
// QDOAS ???          ANLYS_itemList[ANLYS_itemFree].indexPrevious=ITEM_NONE;                // Second item of the list
// QDOAS ???
// QDOAS ???          // Structure filling
// QDOAS ???
// QDOAS ???          memcpy(pList,&ANLYS_itemListBackup[indexListItem],sizeof(LIST_ITEM));
// QDOAS ???
// QDOAS ???          pList->indexPrevious=ITEM_NONE;
// QDOAS ???
// QDOAS ???          if ((pList->indexNext=indexOldItem)!=ITEM_NONE)
// QDOAS ???           ANLYS_itemList[indexOldItem].indexPrevious=indexNewItem;
// QDOAS ???
// QDOAS ???          indexOldItem=indexNewItem;
// QDOAS ???         }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Close on cancel command
// QDOAS ???
// QDOAS ???   EndDialog(hwndAnlys,0);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // =================================================
// QDOAS ??? // ANALYSIS WINDOWS PROPERTIES : MESSAGES PROCESSING
// QDOAS ??? // =================================================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysMeasureListItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_MEASUREITEM message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndList       handle of the listView control
// QDOAS ??? //               mp2            pointer to the description of the item
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysMeasureListItem(HWND hwndList,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   TEXTMETRIC tm;                                                                // Basic information about a physical font
// QDOAS ???   HDC hdc;                                                                      // Handle of device context
// QDOAS ???
// QDOAS ???   // Fills the specified buffer with the metrics for the current font
// QDOAS ???
// QDOAS ???   hdc = GetDC(hwndList);
// QDOAS ???   GetTextMetrics(hdc,&tm);
// QDOAS ???
// QDOAS ???   // Fills the appropriate parameters in MEASUREITEM structure
// QDOAS ???
// QDOAS ???   ((MEASUREITEMSTRUCT *)mp2)->itemWidth=DEFAULT_MEASUREITEM_WIDTH;              // Width of characters
// QDOAS ???   ((MEASUREITEMSTRUCT *)mp2)->itemHeight=tm.tmHeight+5;                         // Total height of characters (ascent+descent)
// QDOAS ???
// QDOAS ???   // Releases handle of device context
// QDOAS ???
// QDOAS ???   ReleaseDC(hwndList,hdc);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysDrawItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_DRAWITEM message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndList       handle of the listView control
// QDOAS ??? //               pDrawItem      attributes of the item to draw
// QDOAS ??? //               pTab           pointer to the currently selected tab page
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysDrawItem(HWND hwndList,DRAWITEMSTRUCT *pDrawItem,ANLYS_TAB_PAGE *pTab)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   DRAWTEXTPARAMS textParams;                                                    // extended formatting text options for DrawTextEx
// QDOAS ???   COLORREF txtColor,bkColor;                                                    // respectively colors of text and background
// QDOAS ???   HBRUSH hBrush;                                                                // handle of brushing object
// QDOAS ???   HDC hDC;                                                                      // handle of device context
// QDOAS ???   RECT rc;                                                                      // currently drawn rectangle
// QDOAS ???
// QDOAS ???   LIST_ITEM *pList;                                                             // pointer to the currently selected item in list
// QDOAS ???   LIST_COLUMN *pCol;                                                            // attributes of columns in ListView control
// QDOAS ???   INDEX indexColumn,                                                            // index for columns in list
// QDOAS ???         indexColumnVisible,                                                     // index for visible columns in ListView control
// QDOAS ???         indexLine,i;                                                            // index for visible lines in ListView control
// QDOAS ???
// QDOAS ???   UCHAR itemText[MAX_ITEM_TEXT_LEN+1];                                          // new item text
// QDOAS ???   INT   itemState,itemData;                                                     // attributes of the currently selected item in ListView control
// QDOAS ???   INT   newColumnWidth;                                                         // new column width
// QDOAS ???   HWND  itemHwnd;                                                               // handle of control associated to an item or a subitem in ListView control
// QDOAS ???
// QDOAS ???   if ((hwndList!=NULL) &&                                                       // handle of control
// QDOAS ???       (pDrawItem->CtlType==ODT_LISTVIEW) &&                                     // type of control
// QDOAS ???       (pDrawItem->itemData!=(DWORD)ITEM_NONE) &&                                // application-defined 32 bit value refer to an item in list
// QDOAS ???
// QDOAS ???   // Get index of item relatively to the first visible one in ListView control
// QDOAS ???
// QDOAS ???      ((indexLine=pDrawItem->itemID-ListView_GetTopIndex(hwndList))<ANLYS_maxLines))
// QDOAS ???    {
// QDOAS ???     // Initializations
// QDOAS ???
// QDOAS ???     textParams.cbSize=sizeof(DRAWTEXTPARAMS);                                   // the structure size in bytes
// QDOAS ???     textParams.iTabLength=                                                      // size of tab stops in units equal to the average character width
// QDOAS ???     textParams.iLeftMargin=6;                                                   // left margin in units equal to the average character width
// QDOAS ???     textParams.iRightMargin=4;                                                  // right margin in units equal to the average character width
// QDOAS ???     textParams.uiLengthDrawn=-1;                                                // length of item text will be computed automatically
// QDOAS ???
// QDOAS ???     memcpy(&rc,&pDrawItem->rcItem,sizeof(RECT));                                // rectangle including complete line
// QDOAS ???     hDC=pDrawItem->hDC;                                                         // handle of the device context
// QDOAS ???     itemData=pDrawItem->itemData;                                               // the application-defined 32 bit value associated to the item
// QDOAS ???     itemState=pDrawItem->itemState;                                             // specify the visible state of the item after the current drawing action takes place
// QDOAS ???
// QDOAS ???     pList=&ANLYS_itemList[itemData];                                            // pointer to the line of items to redraw
// QDOAS ???
// QDOAS ???     if (itemData!=pTab->visibleLinesImage[indexLine].indexItem)
// QDOAS ???      AnlysUpdateList(hwndList,pTab);                                            // handle of ListView control
// QDOAS ???
// QDOAS ???     // Select colors for text and background
// QDOAS ???
// QDOAS ???     if (itemState&ODS_SELECTED)                                                 // item has the selection
// QDOAS ???      {
// QDOAS ???       txtColor=GetSysColor(COLOR_HIGHLIGHTTEXT);
// QDOAS ???       bkColor=GetSysColor(COLOR_HIGHLIGHT);
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???       txtColor=0x00000000L;              // 00RRGGBB (RGB system color) => black
// QDOAS ???       bkColor=GetSysColor(COLOR_WINDOW);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Fill background
// QDOAS ???
// QDOAS ???     hBrush=CreateSolidBrush(bkColor);
// QDOAS ???     rc.right=rc.left+ListView_GetColumnWidth(hwndList,0);
// QDOAS ???     FillRect(hDC,&rc,hBrush);
// QDOAS ???     DeleteObject(hBrush);
// QDOAS ???
// QDOAS ???     // Select colors for text drawing
// QDOAS ???
// QDOAS ???     SetBkColor(hDC,bkColor);
// QDOAS ???     SetTextColor(hDC,txtColor);
// QDOAS ???
// QDOAS ???     for (indexColumn=indexColumnVisible=0;indexColumn<pTab->columnNumber;indexColumn++)
// QDOAS ???      {
// QDOAS ???       pCol=&pTab->columnList[indexColumn];
// QDOAS ???       newColumnWidth=ListView_GetColumnWidth(hwndList,indexColumnVisible);
// QDOAS ???
// QDOAS ???       if ((pCol->controlStyle==BS_AUTOCHECKBOX) && (pCol->columnWidth!=newColumnWidth))
// QDOAS ???        {
// QDOAS ???         for (i=0;i<ANLYS_maxLines;i++)
// QDOAS ???          {
// QDOAS ???           if ((itemHwnd=pTab->visibleLinesImage[i].hwnd[indexColumn])!=NULL)
// QDOAS ???            {
// QDOAS ???             ShowWindow(itemHwnd,SW_HIDE);
// QDOAS ???             ShowWindow(itemHwnd,SW_SHOW);
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       if (!pCol->displayNumber ||
// QDOAS ???           (pCol->displayNumber==pTab->displayNumber))
// QDOAS ???        {
// QDOAS ???         itemHwnd=pTab->visibleLinesImage[indexLine].hwnd[indexColumn];
// QDOAS ???         pCol->columnWidth=newColumnWidth;
// QDOAS ???         rc.right=rc.left+pCol->columnWidth;
// QDOAS ???
// QDOAS ???         if (((ANLYS_indexTabPage==TAB_TYPE_ANLYS_CROSS) &&
// QDOAS ???            ((!ANLYS_displayFits && (indexColumn==COLUMN_CROSS_DISPLAY)) ||
// QDOAS ???             (!PRJCT_panelProject.kurucz.displayFit && (indexColumn==COLUMN_CROSS_SVD_DISPLAY)) ||
// QDOAS ???           (((PRJCT_itemList[ANLYS_indexProject].lfilter.type==PRJCT_FILTER_TYPE_NONE) || (PRJCT_itemList[ANLYS_indexProject].lfilter.type==PRJCT_FILTER_TYPE_ODDEVEN)) &&
// QDOAS ???             (indexColumn==COLUMN_CROSS_FILTER)))) ||
// QDOAS ???
// QDOAS ???             (!PRJCT_itemList[ANLYS_indexProject].asciiResults.analysisFlag &&
// QDOAS ???
// QDOAS ???            (((ANLYS_indexTabPage==TAB_TYPE_ANLYS_SHIFT_AND_STRETCH) &&
// QDOAS ???             ((indexColumn==COLUMN_SH_STORE) ||
// QDOAS ???              (indexColumn==COLUMN_ST_STORE) ||
// QDOAS ???              (indexColumn==COLUMN_SC_STORE) ||
// QDOAS ???              (indexColumn==COLUMN_ERR_STORE))) ||
// QDOAS ???
// QDOAS ???             ((ANLYS_indexTabPage==TAB_TYPE_ANLYS_NOTLINEAR) &&
// QDOAS ???             ((indexColumn==COLUMN_OTHERS_STORE_FIT) ||
// QDOAS ???              (indexColumn==COLUMN_OTHERS_STORE_ERROR))))) ||
// QDOAS ???
// QDOAS ???             ((ANLYS_indexTabPage==TAB_TYPE_ANLYS_CROSS) &&
// QDOAS ???              (STD_Stricmp(pList->itemText[COLUMN_CROSS_XS_TYPE],"Convolute I0")!=0) &&
// QDOAS ???             ((indexColumn==COLUMN_CROSS_CCI0) ||
// QDOAS ???              (indexColumn==COLUMN_CROSS_SVD_CCI0))) ||
// QDOAS ???
// QDOAS ???             ((ANLYS_indexTabPage==TAB_TYPE_ANLYS_OUTPUT) &&
// QDOAS ???
// QDOAS ???            ((!STD_Stricmp(pList->itemText[COLUMN_CROSS_AMF_TYPE],"None") &&
// QDOAS ???             ((indexColumn==COLUMN_CROSS_AMF_OUTPUT) ||
// QDOAS ???              (indexColumn==COLUMN_CROSS_VRTCOL) ||
// QDOAS ???              (indexColumn==COLUMN_CROSS_VRTERR) ||
// QDOAS ???              (indexColumn==COLUMN_CROSS_VRTFACT))) ||
// QDOAS ???
// QDOAS ???             (!PRJCT_itemList[ANLYS_indexProject].asciiResults.analysisFlag &&
// QDOAS ???             ((indexColumn==COLUMN_CROSS_AMF_OUTPUT) ||
// QDOAS ???              (indexColumn==COLUMN_CROSS_SLNTCOL) ||
// QDOAS ???              (indexColumn==COLUMN_CROSS_SLNTERR) ||
// QDOAS ???              (indexColumn==COLUMN_CROSS_SLNTFACT) ||
// QDOAS ???              (indexColumn==COLUMN_CROSS_SVD_SLNTCOL) ||
// QDOAS ???              (indexColumn==COLUMN_CROSS_SVD_SLNTERR) ||
// QDOAS ???              (indexColumn==COLUMN_CROSS_SVD_SLNTFACT) ||
// QDOAS ???              (indexColumn==COLUMN_CROSS_VRTCOL) ||
// QDOAS ???              (indexColumn==COLUMN_CROSS_VRTERR) ||
// QDOAS ???              (indexColumn==COLUMN_CROSS_VRTFACT))) ||
// QDOAS ???
// QDOAS ???             ((ANLYS_RefSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) &&
// QDOAS ???              (indexColumn==COLUMN_CROSS_RESIDUAL)))))
// QDOAS ???          {
// QDOAS ???           if (itemHwnd!=NULL)
// QDOAS ???            ShowWindow(itemHwnd,SW_HIDE);
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         // Process edit and combobox controls
// QDOAS ???         // ----------------------------------
// QDOAS ???
// QDOAS ???         else if (pCol->controlStyle!=BS_AUTOCHECKBOX)
// QDOAS ???          {
// QDOAS ???           // hide controls associated to item
// QDOAS ???
// QDOAS ???           if (itemHwnd!=(HWND)NULL)
// QDOAS ???            {
// QDOAS ???             if (IsWindowVisible(itemHwnd))
// QDOAS ???              {
// QDOAS ???               // Retrieve text from control associated to item
// QDOAS ???
// QDOAS ???               strcpy(itemText,pList->itemText[indexColumn]);
// QDOAS ???
// QDOAS ???               if (pTab->visibleLinesImage[indexLine].indexItem==itemData)
// QDOAS ???                {
// QDOAS ???                 GetWindowText(itemHwnd,itemText,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???                 // Process comboboxes
// QDOAS ???
// QDOAS ???                 if (((pCol->comboboxItemType==COMBOBOX_ITEM_TYPE_ORTHOGONAL) ||
// QDOAS ???                      (pCol->comboboxItemType==COMBOBOX_ITEM_TYPE_EXCLUSIVE)) &&
// QDOAS ???                     ((strlen(itemText)!=strlen(pList->itemText[indexColumn])) ||
// QDOAS ???                       STD_Stricmp(itemText,pList->itemText[indexColumn])))
// QDOAS ???
// QDOAS ???                  AnlysComboboxUpdate(pCol->comboboxItemType,                    // combobox type
// QDOAS ???                                      pTab->symbolList,                          // list of available symbols
// QDOAS ???                                      pTab->symbolNumber,                        // number of symbols in previous list
// QDOAS ???                                      pTab->symbolReferenceNumber,               // number of times symbols are referenced to
// QDOAS ???                                      pList->itemText[indexColumn],              // name of the symbol released
// QDOAS ???                                      itemText);                                 // name of the symbol reserved
// QDOAS ???
// QDOAS ???                 // Retrieve new item text
// QDOAS ???
// QDOAS ???                 strcpy(pList->itemText[indexColumn],itemText);
// QDOAS ???                }
// QDOAS ???
// QDOAS ???               if (ANLYS_indexTabPage==TAB_TYPE_ANLYS_GAPS)
// QDOAS ???                sprintf(pList->itemText[0],"Gap %s-%s",pList->itemText[1],pList->itemText[2]);
// QDOAS ???              }
// QDOAS ???
// QDOAS ???             // Item has the selection
// QDOAS ???
// QDOAS ???             if ((itemHwnd!=NULL) && (itemState&ODS_SELECTED))
// QDOAS ???              {
// QDOAS ???               MoveWindow(itemHwnd,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,FALSE);
// QDOAS ???
// QDOAS ???               // Comboboxes processing
// QDOAS ???
// QDOAS ???               if (pCol->comboboxItemType!=COMBOBOX_ITEM_TYPE_NONE)
// QDOAS ???
// QDOAS ???                AnlysComboboxFill(itemHwnd,                                      // handle for combobox control
// QDOAS ???                                  pCol->comboboxItemType,                        // type of combobox
// QDOAS ???                                  pTab->symbolList,                              // list of available symbols
// QDOAS ???                                  pTab->symbolNumber,                            // number of symbols in previoys list
// QDOAS ???                                  pTab->symbolReferenceNumber,                   // number of times symbols are referenced to
// QDOAS ???                                  pList->itemText[0],                            // text of item in first column (reference text)
// QDOAS ???                                  pList->itemText[indexColumn]);                 // text of item in the currently selected column
// QDOAS ???
// QDOAS ???               // Edit controls processing
// QDOAS ???
// QDOAS ???               else
// QDOAS ???                SetWindowText(itemHwnd,pList->itemText[indexColumn]);
// QDOAS ???
// QDOAS ???               ShowWindow(itemHwnd,SW_SHOW);
// QDOAS ???              }
// QDOAS ???             else
// QDOAS ???              ShowWindow(itemHwnd,SW_HIDE);
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           // Draw new text
// QDOAS ???
// QDOAS ???           hBrush=CreateSolidBrush(bkColor);
// QDOAS ???           FillRect(hDC,&rc,hBrush);
// QDOAS ???           DeleteObject(hBrush);
// QDOAS ???
// QDOAS ???           DrawTextEx(hDC,                                                            // handle of device context
// QDOAS ???                      pList->itemText[indexColumn],                                   // text to draw
// QDOAS ???                      -1,                                                             // length of text
// QDOAS ???                     &rc,                                                             // rectangle including text
// QDOAS ???                    ((pCol->columnFormat==LVCFMT_LEFT)?DT_LEFT:DT_RIGHT)|DT_VCENTER,  // text formatting
// QDOAS ???                     &textParams);                                                    // extending formatting options
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         // Buttons checkboxes processing
// QDOAS ???         // -----------------------------
// QDOAS ???
// QDOAS ???         else if (itemHwnd!=NULL)
// QDOAS ???          {
// QDOAS ???           // Scrolling processing
// QDOAS ???
// QDOAS ???           if (pTab->visibleLinesImage[indexLine].indexItem!=itemData)
// QDOAS ???            CheckDlgButton(hwndList,indexLine*pTab->columnNumber+indexColumn+100*(ANLYS_indexTabPage+1),(pList->itemText[indexColumn][0]!='0')?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???
// QDOAS ???           // Enable button only if item has the selection
// QDOAS ???
// QDOAS ???           if (itemState&ODS_SELECTED)
// QDOAS ???            EnableWindow(itemHwnd,TRUE);
// QDOAS ???
// QDOAS ???           // Retrieve state of the button if item has not the selection
// QDOAS ???
// QDOAS ???           else
// QDOAS ???            {
// QDOAS ???             pList->itemText[indexColumn][0]=(IsDlgButtonChecked(hwndList,indexLine*pTab->columnNumber+indexColumn+100*(ANLYS_indexTabPage+1))==BST_CHECKED)?(UCHAR)'1':(UCHAR)'0';
// QDOAS ???             pList->itemText[indexColumn][(ANLYS_indexTabPage!=TAB_TYPE_ANLYS_LINEAR)?1:2]=0;
// QDOAS ???             EnableWindow(itemHwnd,FALSE);
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           // Control position
// QDOAS ???
// QDOAS ???           if (pCol->columnFormat==LVCFMT_LEFT)
// QDOAS ???            MoveWindow(itemHwnd,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,FALSE);
// QDOAS ???           else
// QDOAS ???            MoveWindow(itemHwnd,rc.left+pCol->columnWidth/2-(rc.bottom-rc.top-5)/2,
// QDOAS ???                       rc.top+2,rc.bottom-rc.top-5,rc.bottom-rc.top-5,FALSE);
// QDOAS ???
// QDOAS ???           // Check button are always visible
// QDOAS ???
// QDOAS ???           ShowWindow(itemHwnd,SW_SHOW);
// QDOAS ???          }
// QDOAS ???
// QDOAS ???         rc.left+=pCol->columnWidth;
// QDOAS ???         indexColumnVisible++;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Hides controls in extra lines
// QDOAS ???
// QDOAS ???     pTab->visibleLinesImage[indexLine].indexItem=itemData;
// QDOAS ???
// QDOAS ???     if (pList->indexNext==ITEM_NONE)
// QDOAS ???      while (++indexLine<ANLYS_maxLines)
// QDOAS ???       for (indexColumn=0;indexColumn<pTab->columnNumber;indexColumn++)
// QDOAS ???        if (pTab->visibleLinesImage[indexLine].hwnd[indexColumn])
// QDOAS ???         ShowWindow(pTab->visibleLinesImage[indexLine].hwnd[indexColumn],SW_HIDE);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysDlgInit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_INITDIALOG message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndAnlys      handle of the analysis windows properties dialog box
// QDOAS ??? //               indexTree      index of the analysis window in the tree
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysDlgInit(HWND hwndAnlys,INT indexTree)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   ANALYSIS_WINDOWS *pAnlysWindowSelected;                                       // selected analysis window
// QDOAS ???   UCHAR textTitle[MAX_ITEM_TEXT_LEN+1],                                         // complete title for the panel
// QDOAS ???         textMsg[MAX_ITEM_TEXT_LEN+1],                                           // part of title of the panel loaded from resources
// QDOAS ???         str[MAX_ITEM_TEXT_LEN+1];                                               // formatted string
// QDOAS ???
// QDOAS ???   TREE_ITEM *pTreeItem;                                                         // pointer to item in tree list
// QDOAS ???   INDEX indexTabPage,                                                           // index on tab pages
// QDOAS ???         indexItem;                                                              // browse item in combobox
// QDOAS ???   HWND  hwndKurucz;
// QDOAS ???
// QDOAS ???   // Center dialog box on parent windows
// QDOAS ???
// QDOAS ???   ANLYS_hwnd=hwndAnlys;
// QDOAS ???
// QDOAS ???   DOAS_CenterWindow(hwndAnlys,GetWindow(hwndAnlys,GW_OWNER));
// QDOAS ???
// QDOAS ???   memset(ANLYS_refEtalonFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   memset(ANLYS_refFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???   if (indexTree!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     if (!THRD_Context(indexTree,ITEM_NONE))
// QDOAS ???      ShowWindow(GetDlgItem(hwndAnlys,IDOK),SW_HIDE);
// QDOAS ???
// QDOAS ???     // Pointers initialization
// QDOAS ???
// QDOAS ???     pTreeItem=&TREE_itemList[indexTree];
// QDOAS ???     pAnlysWindowSelected=&ANLYS_windowsList[(ANLYS_indexSelected=pTreeItem->dataIndex)];
// QDOAS ???
// QDOAS ???     // Set window title
// QDOAS ???
// QDOAS ???     ANLYS_indexProject=TREE_itemList[TREE_itemList[pTreeItem->parentItem].parentItem].dataIndex;  // Grand-Father
// QDOAS ???     ANLYS_maxTabPage=TAB_TYPE_ANLYS_MAX; // (pTreeItem->hidden)?TAB_TYPE_ANLYS_GAPS:TAB_TYPE_ANLYS_MAX;                // if you change max tab page for kurucz, here; please also change in analyse.c
// QDOAS ???
// QDOAS ???     LoadString(DOAS_hInst,IDS_TITLE_ANALYSIS_WINDOWS,textMsg,MAX_ITEM_TEXT_LEN);
// QDOAS ???     sprintf(textTitle,"%s : %s (%s project)",textMsg,pTreeItem->textItem,PRJCT_itemList[ANLYS_indexProject].name);
// QDOAS ???     SetWindowText(hwndAnlys,textTitle);
// QDOAS ???
// QDOAS ???     if (!pTreeItem->hidden)
// QDOAS ???      {
// QDOAS ???       // Set reference spectra files
// QDOAS ???
// QDOAS ???       FILES_RebuildFileName(ANLYS_refEtalonFileName,pAnlysWindowSelected->refEtalon,1);
// QDOAS ???       FILES_RebuildFileName(ANLYS_refFileName,pAnlysWindowSelected->refSpectrumFile,1);
// QDOAS ???       FILES_RebuildFileName(ANLYS_residuals,pAnlysWindowSelected->residualsFile,1);
// QDOAS ???
// QDOAS ???       SetWindowText(GetDlgItem(hwndAnlys,ANLYS_REF_ETALON),ANLYS_refEtalonFileName);
// QDOAS ???       SetWindowText(GetDlgItem(hwndAnlys,ANLYS_REF_SPECTRUM),ANLYS_refFileName);
// QDOAS ???       SetWindowText(GetDlgItem(hwndAnlys,ANLYS_RESIDUALS_FILE),ANLYS_residuals);
// QDOAS ???
// QDOAS ???       CheckRadioButton(hwndAnlys,
// QDOAS ???                        ANLYS_REF_SELECTION_AUTOMATIC,
// QDOAS ???                        ANLYS_REF_SELECTION_FILE,
// QDOAS ???                        ANLYS_REF_SELECTION_AUTOMATIC+pAnlysWindowSelected->refSpectrumSelectionMode);
// QDOAS ???
// QDOAS ???       // Fill comboboxes
// QDOAS ???
// QDOAS ???       hwndKurucz=GetDlgItem(hwndAnlys,ANLYS_REF_KURUCZ_SELECTION);
// QDOAS ???
// QDOAS ???       for (indexItem=0;indexItem<ANLYS_KURUCZ_MAX;indexItem++)
// QDOAS ???        SendMessage(hwndKurucz,CB_ADDSTRING,0,(LPARAM)anlysKuruczModes[indexItem]);
// QDOAS ???
// QDOAS ???       SendMessage(hwndKurucz,CB_SETCURSEL,(WPARAM)pAnlysWindowSelected->useKurucz,0);
// QDOAS ???
// QDOAS ???       SetWindowText(GetDlgItem(hwndAnlys,ANLYS_REF_SPECTRUM),ANLYS_refFileName);
// QDOAS ???       sprintf(str,"%.1f",pAnlysWindowSelected->refSZA);
// QDOAS ???       SetWindowText(GetDlgItem(hwndAnlys,ANLYS_REF_SZA_VALUE),str);
// QDOAS ???       sprintf(str,"%.1f",pAnlysWindowSelected->refSZADelta);
// QDOAS ???       SetWindowText(GetDlgItem(hwndAnlys,ANLYS_REF_SZA_DELTA),str);
// QDOAS ???       sprintf(str,"%.1f",pAnlysWindowSelected->refLatMin);
// QDOAS ???       SetWindowText(GetDlgItem(hwndAnlys,ANLYS_REF_LAT1),str);
// QDOAS ???       sprintf(str,"%.1f",pAnlysWindowSelected->refLatMax);
// QDOAS ???       SetWindowText(GetDlgItem(hwndAnlys,ANLYS_REF_LAT2),str);
// QDOAS ???       sprintf(str,"%.1f",pAnlysWindowSelected->refLonMin);
// QDOAS ???       SetWindowText(GetDlgItem(hwndAnlys,ANLYS_REF_LON1),str);
// QDOAS ???       sprintf(str,"%.1f",pAnlysWindowSelected->refLonMax);
// QDOAS ???       SetWindowText(GetDlgItem(hwndAnlys,ANLYS_REF_LON2),str);
// QDOAS ???       SetDlgItemInt(hwndAnlys,ANLYS_REF_NSPECTRA,pAnlysWindowSelected->nspectra,FALSE);
// QDOAS ???
// QDOAS ???       // Set analysis window limits
// QDOAS ???
// QDOAS ???       SetWindowText(GetDlgItem(hwndAnlys,ANLYS_LIMITS_MIN),pAnlysWindowSelected->lembdaMin);
// QDOAS ???       SetWindowText(GetDlgItem(hwndAnlys,ANLYS_LIMITS_MAX),pAnlysWindowSelected->lembdaMax);
// QDOAS ???
// QDOAS ???       // Set spectra to display
// QDOAS ???
// QDOAS ???       CheckDlgButton(hwndAnlys,ANLYS_DISPLAY_SPECTRUM,(pAnlysWindowSelected->displaySpectrum!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???       CheckDlgButton(hwndAnlys,ANLYS_DISPLAY_RESIDUE,(pAnlysWindowSelected->displayResidue!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???       CheckDlgButton(hwndAnlys,ANLYS_DISPLAY_TREND,(pAnlysWindowSelected->displayTrend!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???       CheckDlgButton(hwndAnlys,ANLYS_DISPLAY_REFETALON,(pAnlysWindowSelected->displayRefEtalon!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???       CheckDlgButton(hwndAnlys,ANLYS_DISPLAY_FITS,((ANLYS_displayFits=pAnlysWindowSelected->displayFits)!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???       CheckDlgButton(hwndAnlys,ANLYS_DISPLAY_PREDEFINED,(pAnlysWindowSelected->displayPredefined!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???
// QDOAS ???       CheckDlgButton(hwndAnlys,ANLYS_GOME_0,(pAnlysWindowSelected->gomePixelType[0]!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???       CheckDlgButton(hwndAnlys,ANLYS_GOME_1,(pAnlysWindowSelected->gomePixelType[1]!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???       CheckDlgButton(hwndAnlys,ANLYS_GOME_2,(pAnlysWindowSelected->gomePixelType[2]!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???       CheckDlgButton(hwndAnlys,ANLYS_GOME_3,(pAnlysWindowSelected->gomePixelType[3]!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Up-down control initialization
// QDOAS ???
// QDOAS ???     SendMessage(GetDlgItem(hwndAnlys,ANLYS_ARROWS),UDM_SETRANGE,0L,MAKELONG(1,-1));
// QDOAS ???     SendMessage(GetDlgItem(hwndAnlys,ANLYS_ARROWS),UDM_SETPOS,0L,MAKELONG(0,0));
// QDOAS ???
// QDOAS ???     AnlysSetRefSelection(hwndAnlys,pAnlysWindowSelected);
// QDOAS ???
// QDOAS ???     // Tab Control initialization
// QDOAS ???
// QDOAS ???     pAnlysWindowSelected->hidden=pTreeItem->hidden;
// QDOAS ???
// QDOAS ???     if (pTreeItem->newItem)
// QDOAS ???      {
// QDOAS ???       for (indexTabPage=0;indexTabPage<TAB_TYPE_ANLYS_MAX;indexTabPage++)
// QDOAS ???        pAnlysWindowSelected->listEntryPoint[indexTabPage]=ITEM_NONE;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     memcpy(ANLYS_itemListBackup,ANLYS_itemList,SYS_anlysItems*sizeof(LIST_ITEM));
// QDOAS ???
// QDOAS ???     AnlysTabInit(hwndAnlys);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysContext
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_CONTEXTMENU message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndAnlys      handle of the analysis windows properties dialog box
// QDOAS ??? //               hwndList       handle of the listView control
// QDOAS ??? //               mp2            position of the shortcut menu
// QDOAS ??? //               pTab           pointer to the selected tab page
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysContext(HWND hwndAnlys,                                               // handle of parent window
// QDOAS ???                   HWND hwndList,                                                // handle of ListView control
// QDOAS ???                   LPARAM mp2,                                                   // position of the shortcut menu
// QDOAS ???                   ANLYS_TAB_PAGE *pTab)                                         // the currently selected page
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR symbolName[MAX_ITEM_TEXT_LEN+1];                                        // name of the selected symbol
// QDOAS ???   INDEX indexLvi,indexItem,indexSymbol;                                         // index of selected item in ListView control and index of selected symbol
// QDOAS ???   HMENU hMenu,hSubMenu;                                                         // menu handle
// QDOAS ???   MENUITEMINFO menuItemInfo;                                                    // information about a menu item
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   indexItem=AnlysGetSelectedListViewItem(hwndList,&indexLvi,symbolName);
// QDOAS ???
// QDOAS ???   // Search for the selected item in tree
// QDOAS ???
// QDOAS ???   if ((ANLYS_indexTabPage!=TAB_TYPE_ANLYS_OUTPUT) &&
// QDOAS ???       (hwndAnlys!=NULL) &&
// QDOAS ???       (GetFocus()==hwndList) &&
// QDOAS ???      ((hMenu=LoadMenu(DOAS_hInst,MAKEINTRESOURCE(MENU_CONTEXT_LIST)))!=NULL))
// QDOAS ???    {
// QDOAS ???     // Filter context menu operations
// QDOAS ???
// QDOAS ???     if ((ANLYS_indexTabPage!=TAB_TYPE_ANLYS_GAPS) &&
// QDOAS ???         (ANLYS_indexTabPage!=TAB_TYPE_ANLYS_LINEAR) &&
// QDOAS ???        ((ANLYS_indexTabPage!=TAB_TYPE_ANLYS_NOTLINEAR) ||
// QDOAS ???         !STD_Stricmp(symbolName,SYMB_itemCrossList[SYMBOL_PREDEFINED_COM].name) ||
// QDOAS ???         !STD_Stricmp(symbolName,SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP1].name) ||
// QDOAS ???         !STD_Stricmp(symbolName,SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP2].name) ||
// QDOAS ???         !STD_Stricmp(symbolName,SYMB_itemCrossList[SYMBOL_PREDEFINED_RING1].name))) // &&
// QDOAS ??? //       ((ANLYS_indexTabPage==TAB_TYPE_ANLYS_LINEAR) ||
// QDOAS ??? //        (!pTab->columnList[0].controlStyle && (indexItem!=ITEM_NONE))))
// QDOAS ???
// QDOAS ???      EnableMenuItem(hMenu,MENU_CONTEXT_PROPERTIES,MF_ENABLED);
// QDOAS ???
// QDOAS ???     if ((ANLYS_indexTabPage==TAB_TYPE_ANLYS_GAPS) ||
// QDOAS ???        ((ANLYS_indexTabPage!=TAB_TYPE_ANLYS_LINEAR) &&
// QDOAS ???         (ANLYS_indexTabPage!=TAB_TYPE_ANLYS_NOTLINEAR) &&
// QDOAS ???         (indexItem!=ITEM_NONE) && (pTab->symbolReferenceNumber!=NULL) &&
// QDOAS ???        ((indexSymbol=SYMB_GetListIndex(pTab->symbolList,pTab->symbolNumber,symbolName))!=ITEM_NONE) &&
// QDOAS ???        !(pTab->symbolReferenceNumber[indexSymbol]&0xFF)))
// QDOAS ???      EnableMenuItem(hMenu,MENU_CONTEXT_DELETE,MF_ENABLED);
// QDOAS ???
// QDOAS ???     if ((ANLYS_indexTabPage!=TAB_TYPE_ANLYS_LINEAR) && (ANLYS_indexTabPage!=TAB_TYPE_ANLYS_NOTLINEAR) && pTab->availableSymbolNumber)
// QDOAS ???      EnableMenuItem(hMenu,MENU_CONTEXT_INSERT,MF_ENABLED);
// QDOAS ???
// QDOAS ???     if ((ANLYS_indexTabPage==TAB_TYPE_ANLYS_CROSS) && (indexItem!=ITEM_NONE) &&
// QDOAS ???          STD_Stricmp(ANLYS_itemList[indexItem].itemText[COLUMN_CROSS_AMF_TYPE],"NONE"))
// QDOAS ???      {
// QDOAS ???       hSubMenu=CreatePopupMenu();
// QDOAS ???       AppendMenu(hSubMenu,MF_STRING,MENU_CONTEXT_PROPERTIES,"Cross section file");
// QDOAS ???       AppendMenu(hSubMenu,MF_STRING,MENU_CONTEXT_PROPERTIES_AMF,"AMF file");
// QDOAS ???
// QDOAS ???       memset(&menuItemInfo,0,sizeof(MENUITEMINFO));
// QDOAS ???
// QDOAS ???       menuItemInfo.cbSize=sizeof(MENUITEMINFO);
// QDOAS ???       menuItemInfo.fMask=MIIM_SUBMENU;
// QDOAS ???
// QDOAS ???       GetMenuItemInfo(hMenu,MENU_CONTEXT_PROPERTIES,FALSE,&menuItemInfo);
// QDOAS ???       menuItemInfo.hSubMenu=hSubMenu;
// QDOAS ???       SetMenuItemInfo(hMenu,MENU_CONTEXT_PROPERTIES,FALSE,&menuItemInfo);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Track menu
// QDOAS ???
// QDOAS ???     TrackPopupMenu(GetSubMenu(hMenu,0),                                         // first item of the context menu
// QDOAS ???                    TPM_LEFTALIGN|TPM_LEFTBUTTON,                                // screen position and mouse button flags
// QDOAS ???                    LOWORD(mp2),HIWORD(mp2),                                     // coordinates of the shortcut menu
// QDOAS ???                    0,                                                           // reserved
// QDOAS ???                    (HWND)hwndAnlys,                                             // handle of the parent window
// QDOAS ???                    NULL);                                                       // no-dismissal area
// QDOAS ???
// QDOAS ???     // Destroy menu
// QDOAS ???
// QDOAS ???     DestroyMenu(hMenu);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysNotify
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_NOTIFY message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndAnlys      handle of the analysis windows properties dialog box
// QDOAS ??? //               pHdr           description of the windows notification message
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysNotify(HWND hwndAnlys,NMHDR *pHdr)
// QDOAS ???  {
// QDOAS ???   // Notification message processing
// QDOAS ???
// QDOAS ???   switch (pHdr->code)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TCN_SELCHANGE :  // the currently selected tab page has changed
// QDOAS ???
// QDOAS ???      AnlysTabChange(GetDlgItem(hwndAnlys,ANLYS_DATA),
// QDOAS ???                     GetDlgItem(hwndAnlys,ANLYS_LISTVIEW),
// QDOAS ???                     GetDlgItem(hwndAnlys,ANLYS_ARROWS),
// QDOAS ???                     GetDlgItem(hwndAnlys,ANLYS_PAGE),
// QDOAS ???                     0);
// QDOAS ???
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysCommand
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_COMMAND message processing
// QDOAS ??? //
// QDOAS ??? // SYNTAX        usual syntax for Windows message processing;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? LRESULT CALLBACK AnlysCommand(HWND hwndAnlys,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch((ULONG)GET_WM_COMMAND_ID(mp1,mp2))
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case MENU_CONTEXT_INSERT :
// QDOAS ???      AnlysInsertOneListItem(GetDlgItem(hwndAnlys,ANLYS_LISTVIEW),ANLYS_indexTabPage);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case MENU_CONTEXT_DELETE :
// QDOAS ???      AnlysDeleteOneListItem(GetDlgItem(hwndAnlys,ANLYS_LISTVIEW),
// QDOAS ???                            &ANLYS_tabPages[ANLYS_indexTabPage]);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case MENU_CONTEXT_PROPERTIES :
// QDOAS ???      AnlysProperties(GetDlgItem(hwndAnlys,ANLYS_LISTVIEW),
// QDOAS ???                      &ANLYS_tabPages[ANLYS_indexTabPage]);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case MENU_CONTEXT_PROPERTIES_AMF :
// QDOAS ???      AnlysPropertiesAmf(GetDlgItem(hwndAnlys,ANLYS_LISTVIEW),
// QDOAS ???                        &ANLYS_tabPages[ANLYS_indexTabPage]);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case ANLYS_REF_SELECTION_AUTOMATIC :
// QDOAS ???     case ANLYS_REF_SELECTION_FILE :
// QDOAS ???     case ANLYS_REF_KURUCZ_SELECTION :
// QDOAS ???      AnlysSetRefSelection(hwndAnlys,&ANLYS_windowsList[ANLYS_indexSelected]);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case ANLYS_REF_SPECTRUM_BUTTON :
// QDOAS ???      AnlysGetRefFiles(hwndAnlys,ANLYS_REF_SPECTRUM);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case ANLYS_REF_ETALON_BUTTON :
// QDOAS ???      AnlysGetRefFiles(hwndAnlys,ANLYS_REF_ETALON);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case ANLYS_RESIDUALS_BUTTON :
// QDOAS ???      AnlysResidualFile(hwndAnlys);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case ANLYS_DISPLAY_FITS :
// QDOAS ???      AnlysDisplayFits(hwndAnlys);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDOK :
// QDOAS ???      AnlysOK(hwndAnlys,&ANLYS_windowsList[ANLYS_indexSelected],ANLYS_tabPages);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDCANCEL :
// QDOAS ???      AnlysCancel(hwndAnlys,&ANLYS_windowsList[ANLYS_indexSelected],ANLYS_tabPages);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDHELP :
// QDOAS ???      WinHelp(hwndAnlys,DOAS_HelpPath,HELP_CONTEXT,(ANLYS_windowsList[ANLYS_indexSelected].hidden)?
// QDOAS ???              ANLYS_tabPages[ANLYS_indexTabPage].calibHelpId:HLP_ANLYS_PROPERTIES);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysDestroy
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_DESTROY message processing
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysDestroy(void)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexLine,indexTab;
// QDOAS ???   ANLYS_TAB_PAGE *pTab;
// QDOAS ???
// QDOAS ???   // Free buffers allocate for items in comboboxes
// QDOAS ???
// QDOAS ???   for (indexTab=0;indexTab<ANLYS_maxTabPage;indexTab++)
// QDOAS ???    {
// QDOAS ???     pTab=&ANLYS_tabPages[indexTab];
// QDOAS ???
// QDOAS ???     // Handles of controls associated to visible lines in ListView control
// QDOAS ???
// QDOAS ???     for (indexLine=0;indexLine<ANLYS_maxLines;indexLine++)
// QDOAS ???      MEMORY_ReleaseBuffer("AnlysDestroy ","hwnd",pTab->visibleLinesImage[indexLine].hwnd);
// QDOAS ???     MEMORY_ReleaseBuffer("AnlysDestroy ","visibleLinesImage",pTab->visibleLinesImage);
// QDOAS ???
// QDOAS ???     // Mask associated to available symbols
// QDOAS ???
// QDOAS ???     if (pTab->symbolReferenceNumber!=NULL)
// QDOAS ???      MEMORY_ReleaseBuffer("AnlysDestroy ","symbolReferenceNumber",pTab->symbolReferenceNumber);
// QDOAS ???
// QDOAS ???     // Reset pointers
// QDOAS ???
// QDOAS ???     pTab->symbolReferenceNumber=NULL;
// QDOAS ???     pTab->visibleLinesImage=NULL;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // No selected tab page
// QDOAS ???
// QDOAS ???   ANLYS_indexSelected=ITEM_NONE;
// QDOAS ???   ANLYS_indexTabPage=TAB_TYPE_ANLYS_CROSS;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      AnlysHelp
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       WM_HELP message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndAnlys      handle of the analysis windows properties dialog box
// QDOAS ??? //               ctrlId         id of the control button
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void AnlysHelp(HWND hwndAnlys,INT ctrlId)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   ANLYS_TAB_PAGE *pTab;
// QDOAS ???   INT helpId;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pTab=&ANLYS_tabPages[ANLYS_indexTabPage];
// QDOAS ???   helpId=0;
// QDOAS ???
// QDOAS ???   // Select help jump according to the focused field
// QDOAS ???
// QDOAS ???   if ((ctrlId>=ANLYS_REF_SELECTION_GROUP) && (ctrlId<=ANLYS_REF_SELECTION_FILE))
// QDOAS ???    helpId=HLP_ANLYS_REFERENCE;
// QDOAS ???   else if ((ctrlId>=ANLYS_REF_FILE_GROUP) && (ctrlId<=ANLYS_RESIDUALS_BUTTON))
// QDOAS ???    helpId=HLP_ANLYS_FILES;
// QDOAS ???   else if ((ctrlId>=ANLYS_DATA) && (ctrlId<=ANLYS_LIMITS_MAX))
// QDOAS ???    helpId=HLP_ANLYS_FITTING_INTERVAL;
// QDOAS ???   else if ((ctrlId>=ANLYS_DISPLAY_GROUP) && (ctrlId<=ANLYS_DISPLAY_PREDEFINED))
// QDOAS ???    helpId=HLP_ANLYS_DISPLAY;
// QDOAS ???   else if ((ctrlId>=ANLYS_REF_KURUCZ_GROUP) && (ctrlId<=ANLYS_REF_KURUCZ_SELECTION))
// QDOAS ???    helpId=HLP_ANLYS_CALIB;
// QDOAS ???   else if ((ctrlId>=ANLYS_LISTVIEW) && (ctrlId<=ANLYS_PAGE))
// QDOAS ???    helpId=(ANLYS_windowsList[ANLYS_indexSelected].hidden)?pTab->calibHelpId:pTab->anlysHelpId;
// QDOAS ???
// QDOAS ???   // Call help processor
// QDOAS ???
// QDOAS ???   if (helpId>0)
// QDOAS ???    WinHelp(hwndAnlys,DOAS_HelpPath,HELP_CONTEXT,helpId);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      ANLYS_WndProc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       dispatch messages from analysis windows properties panel
// QDOAS ??? //
// QDOAS ??? // SYNTAX        usual syntax for Windows message processing;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? LRESULT CALLBACK ANLYS_WndProc(HWND hwndAnlys,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   if (ANLYS_windowsList!=NULL)
// QDOAS ???    {
// QDOAS ???     switch (msg)
// QDOAS ???      {
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case WM_HSCROLL :
// QDOAS ???        if (LOWORD(mp1)==SB_THUMBPOSITION)    // Scroll to the absolute position
// QDOAS ???         {
// QDOAS ???          AnlysTabChange(GetDlgItem(hwndAnlys,ANLYS_DATA),GetDlgItem(hwndAnlys,ANLYS_LISTVIEW),GetDlgItem(hwndAnlys,ANLYS_ARROWS),GetDlgItem(hwndAnlys,ANLYS_PAGE),(CHAR)HIWORD(mp1));
// QDOAS ???          SendMessage(GetDlgItem(hwndAnlys,ANLYS_ARROWS),UDM_SETPOS,0L,MAKELONG(0,0));
// QDOAS ???         }
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case WM_PAINT :
// QDOAS ???        AnlysTabChange(GetDlgItem(hwndAnlys,ANLYS_DATA),GetDlgItem(hwndAnlys,ANLYS_LISTVIEW),GetDlgItem(hwndAnlys,ANLYS_ARROWS),GetDlgItem(hwndAnlys,ANLYS_PAGE),0);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case WM_MEASUREITEM :
// QDOAS ???        AnlysMeasureListItem(GetDlgItem(hwndAnlys,ANLYS_LISTVIEW),mp2);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case WM_DRAWITEM :
// QDOAS ???        AnlysDrawItem(GetDlgItem(hwndAnlys,ANLYS_LISTVIEW),    // handle of ListView control
// QDOAS ???                     (DRAWITEMSTRUCT *)mp2,                    // attributes of item to draw
// QDOAS ???                     &ANLYS_tabPages[ANLYS_indexTabPage]);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case WM_INITDIALOG :
// QDOAS ???        AnlysDlgInit(hwndAnlys,(INDEX)mp2);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case WM_CONTEXTMENU :
// QDOAS ???        AnlysContext(hwndAnlys,
// QDOAS ???                     GetDlgItem(hwndAnlys,ANLYS_LISTVIEW),
// QDOAS ???                     mp2,
// QDOAS ???                    &ANLYS_tabPages[ANLYS_indexTabPage]);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case WM_NOTIFY :
// QDOAS ???        AnlysNotify(hwndAnlys,(NMHDR *)mp2);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case WM_COMMAND :
// QDOAS ???        AnlysCommand(hwndAnlys,msg,mp1,mp2);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case WM_DESTROY :
// QDOAS ???        AnlysDestroy();
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???       case WM_HELP :
// QDOAS ???        AnlysHelp(hwndAnlys,((LPHELPINFO)mp2)->iCtrlId);
// QDOAS ???       break;
// QDOAS ???    // ---------------------------------------------------------------------------
// QDOAS ???     }
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
// QDOAS ??? // FUNCTION        ANLYS_Alloc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Allocate and initialize buffers for analysis windows
// QDOAS ??? //
// QDOAS ??? // RETURN          ERROR_ID_ALLOC if one of the buffer allocation failed
// QDOAS ??? //                 ERROR_ID_NO in case of success
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC ANLYS_Alloc(void)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   LIST_ITEM *pList;
// QDOAS ???   INDEX indexItem;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Structure allocation and filling for analysis windows
// QDOAS ???
// QDOAS ???   if (((TREE_itemType[TREE_ITEM_TYPE_ANALYSIS_CHILDREN].dataList=(VOID *)
// QDOAS ???         MEMORY_AllocBuffer("ANLYS_Alloc ","ANLYS_windowsList",SYS_anlys,sizeof(ANALYSIS_WINDOWS),0,MEMORY_TYPE_STRUCT))==NULL) ||
// QDOAS ???       ((TREE_itemType[TREE_ITEM_TYPE_ANALYSIS_CHILDREN].data2Paste=(VOID *)
// QDOAS ???         MEMORY_AllocBuffer("ANLYS_Alloc ","ANLYS_toPaste",SYS_anlys,sizeof(ANALYSIS_WINDOWS),0,MEMORY_TYPE_STRUCT))==NULL))
// QDOAS ???
// QDOAS ???    rc=ERROR_ID_ALLOC;
// QDOAS ???
// QDOAS ???   // Structure allocation and filling for ListView control
// QDOAS ???
// QDOAS ???   else if (((ANLYS_itemList=(LIST_ITEM *)MEMORY_AllocBuffer("ANLYS_Alloc ","ANLYS_itemList",SYS_anlysItems,sizeof(LIST_ITEM),0,MEMORY_TYPE_STRUCT))==NULL) ||
// QDOAS ???            ((ANLYS_itemListBackup=(LIST_ITEM *)MEMORY_AllocBuffer("ANLYS_Alloc ","ANLYS_itemListBackup",SYS_anlysItems,sizeof(LIST_ITEM),0,MEMORY_TYPE_STRUCT))==NULL) ||
// QDOAS ???            ((anlysItemListToPaste=(LIST_ITEM *)MEMORY_AllocBuffer("ANLYS_Alloc ","anlysItemListToPaste",SYS_anlysItems,sizeof(LIST_ITEM),0,MEMORY_TYPE_STRUCT))==NULL) ||
// QDOAS ???            ((ANLYS_refFileName=(UCHAR *)MEMORY_AllocBuffer("ANLYS_Alloc ","ANLYS_refFileName",MAX_ITEM_TEXT_LEN+1,1,0,MEMORY_TYPE_STRING))==NULL) ||
// QDOAS ???            ((ANLYS_refEtalonFileName=(UCHAR *)MEMORY_AllocBuffer("ANLYS_Alloc ","ANLYS_refEtalonFileName",MAX_ITEM_TEXT_LEN+1,1,0,MEMORY_TYPE_STRING))==NULL) ||
// QDOAS ???            ((ANLYS_residuals=(UCHAR *)MEMORY_AllocBuffer("ANLYS_Alloc ","ANLYS_residuals",MAX_ITEM_TEXT_LEN+1,1,0,MEMORY_TYPE_STRING))==NULL) ||
// QDOAS ???            ((ANLYS_crossFileName=(UCHAR *)MEMORY_AllocBuffer("ANLYS_Alloc ","ANLYS_crossFileName",MAX_ITEM_TEXT_LEN+1,1,0,MEMORY_TYPE_STRING))==NULL) ||
// QDOAS ???            ((ANLYS_amfFileName=(UCHAR *)MEMORY_AllocBuffer("ANLYS_Alloc ","ANLYS_amfFileName",MAX_ITEM_TEXT_LEN+1,1,0,MEMORY_TYPE_STRING))==NULL))
// QDOAS ???
// QDOAS ???    rc=ERROR_ID_ALLOC;
// QDOAS ???
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     ANLYS_windowsList=(ANALYSIS_WINDOWS *)TREE_itemType[TREE_ITEM_TYPE_ANALYSIS_CHILDREN].dataList;
// QDOAS ???     ANLYS_toPaste=(ANALYSIS_WINDOWS *)TREE_itemType[TREE_ITEM_TYPE_ANALYSIS_CHILDREN].data2Paste;
// QDOAS ???
// QDOAS ???     memset(ANLYS_windowsList,0,SYS_anlys*sizeof(ANALYSIS_WINDOWS));
// QDOAS ???     memset(ANLYS_itemList,0,SYS_anlysItems*sizeof(LIST_ITEM));
// QDOAS ???     memset(ANLYS_itemListBackup,0,SYS_anlysItems*sizeof(LIST_ITEM));
// QDOAS ???     memset(anlysItemListToPaste,0,SYS_anlysItems*sizeof(LIST_ITEM));
// QDOAS ???
// QDOAS ???     for (indexItem=0;indexItem<(int)SYS_anlysItems;indexItem++)
// QDOAS ???      {
// QDOAS ???       pList=&ANLYS_itemList[indexItem];
// QDOAS ???
// QDOAS ???       pList->hidden=0;                                                          // hidden flag
// QDOAS ???       pList->indexParent=ITEM_NONE;                                             // index of parent item
// QDOAS ???       pList->indexPrevious=indexItem-1;                                         // index of previous item in list
// QDOAS ???       pList->indexNext=indexItem+1;                                             // index of the next item in list
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     pList->indexNext=ITEM_NONE;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        ANLYS_Free
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Release buffers allocated for analysis windows
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void ANLYS_Free(void)
// QDOAS ???  {
// QDOAS ???   if (ANLYS_windowsList!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("ANLYS_Free ","ANLYS_windowsList",ANLYS_windowsList);
// QDOAS ???   if (ANLYS_itemList!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("ANLYS_Free ","ANLYS_itemList",ANLYS_itemList);
// QDOAS ???   if (ANLYS_itemListBackup!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("ANLYS_Free ","ANLYS_itemListBackup",ANLYS_itemListBackup);
// QDOAS ???   if (anlysItemListToPaste!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("ANLYS_Free ","anlysItemListToPaste",anlysItemListToPaste);
// QDOAS ???   if (ANLYS_refFileName!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("ANLYS_Free ","ANLYS_refFileName",ANLYS_refFileName);
// QDOAS ???   if (ANLYS_residuals!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("ANLYS_Free ","ANLYS_residuals",ANLYS_residuals);
// QDOAS ???   if (ANLYS_refEtalonFileName!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("ANLYS_Free ","ANLYS_refEtalonFileName",ANLYS_refEtalonFileName);
// QDOAS ???   if (ANLYS_crossFileName!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("ANLYS_Free ","ANLYS_crossFileName",ANLYS_crossFileName);
// QDOAS ???   if (ANLYS_amfFileName!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("ANLYS_Free ","ANLYS_amfFileName",ANLYS_amfFileName);
// QDOAS ???   if (ANLYS_toPaste!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("ANLYS_Free ","ANLYS_toPaste",ANLYS_toPaste);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // =============================
// QDOAS ??? // CONFIGURATION FILE MANAGEMENT
// QDOAS ??? // =============================
// QDOAS ???
// QDOAS ??? INDEX ANLYS_indexWindow=ITEM_NONE,ANLYS_indexData;
// QDOAS ??? ANALYSIS_WINDOWS ANLYS_options;                             // analysis window
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        ANLYS_ResetConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         remove the current list of analysis windows from the
// QDOAS ??? //                 environment space tree;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void ANLYS_ResetConfiguration(void)
// QDOAS ???  {
// QDOAS ???   INDEX indexTab;
// QDOAS ???
// QDOAS ???   memset(ANLYS_refEtalonFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   memset(ANLYS_refFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   memset(ANLYS_residuals,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???   ANLYS_indexWindow=ITEM_NONE;
// QDOAS ???   ANLYS_indexData=ITEM_NONE;
// QDOAS ???
// QDOAS ???   memset(&ANLYS_options,0,sizeof(ANALYSIS_WINDOWS));
// QDOAS ???
// QDOAS ???   for (indexTab=0;indexTab<TAB_TYPE_ANLYS_MAX;indexTab++)
// QDOAS ???    ANLYS_options.listEntryPoint[indexTab]=ITEM_NONE;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        ANLYS_OutputConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         output the configuration of the selected analysis windows to a file;
// QDOAS ??? //
// QDOAS ??? // INPUT           fp            pointer to the output file
// QDOAS ??? //                 indexProject  index of the selected project
// QDOAS ??? //                 indexWindow   index of the analysis window to output
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void ANLYS_OutputConfiguration(FILE *fp,INDEX indexProject,INDEX indexWindow)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   PROJECT *pProject;
// QDOAS ???   ANALYSIS_WINDOWS *pAnlys;
// QDOAS ???   UCHAR string[MAX_PATH_LEN+1];
// QDOAS ???   INDEX indexItem,indexOld[TAB_TYPE_ANLYS_MAX],i,n,indexLine,polyDegree,polyBase;
// QDOAS ???   UCHAR symbolList[MAX_ITEM_TEXT_LEN+1],*symbol,*nextSymbol;
// QDOAS ???   SZ_LEN symbolLength;
// QDOAS ???   LIST_ITEM *pList,*pList2;
// QDOAS ???   UCHAR indent[MAX_ITEM_TEXT_LEN+1],indentXs[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pProject=&PRJCT_itemList[indexProject];
// QDOAS ???   pAnlys=&ANLYS_windowsList[indexWindow];
// QDOAS ???
// QDOAS ???   strcpy(indent,"        ");
// QDOAS ???   strcpy(indentXs,"                                ");
// QDOAS ???   if (pAnlys->hidden<=1)
// QDOAS ???    {
// QDOAS ???
// QDOAS ???     fprintf(fp,"%sAnalysis window         %s\n",indent,pAnlys->windowName);
// QDOAS ???     fprintf(fp,"%sFitting interval        %.2lf - %.2lf nm\n",indent,atof(pAnlys->lembdaMin),atof(pAnlys->lembdaMax));
// QDOAS ???     fprintf(fp,"%sWavelength calibration  %s\n",indent,anlysKuruczModes[pAnlys->useKurucz]);
// QDOAS ???     fprintf(fp,"%sReference selection     %s\n",indent,(pAnlys->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC)?"automatic":"file");
// QDOAS ???
// QDOAS ???     FILES_RebuildFileName(string,pAnlys->refEtalon,1);
// QDOAS ???     fprintf(fp,"%sReference 1             %s\n",indent,(strlen(string))?string:(UCHAR *)"Not specified");
// QDOAS ???     fprintf(fp,"%sReference 2             ",indent);
// QDOAS ???
// QDOAS ???     if (pAnlys->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC)
// QDOAS ???      {
// QDOAS ???       fprintf(fp,"SZA %.2lf +/- %.2lf deg ",pAnlys->refSZA,pAnlys->refSZADelta);
// QDOAS ???       if ((pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
// QDOAS ???           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
// QDOAS ???           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
// QDOAS ???           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
// QDOAS ???           (pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2))
// QDOAS ???        {
// QDOAS ???         fprintf(fp,"LAT %.2lf - %.2lf ",pAnlys->refLatMin,pAnlys->refLatMax);
// QDOAS ???         fprintf(fp,"LON %.2lf - %.2lf ",pAnlys->refLonMin,pAnlys->refLonMax);
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???       FILES_RebuildFileName(string,pAnlys->refSpectrumFile,1);
// QDOAS ???       fprintf(fp,"%s",(strlen(string))?string:(UCHAR *)"Not specified");
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     fprintf(fp,"\n");
// QDOAS ???
// QDOAS ???     for (i=0;i<TAB_TYPE_ANLYS_MAX;i++)
// QDOAS ???      {
// QDOAS ???       indexOld[i]=ITEM_NONE;
// QDOAS ???
// QDOAS ???       // Need to inverse the order of items
// QDOAS ???
// QDOAS ???       for (indexItem=pAnlys->listEntryPoint[i];
// QDOAS ???            indexItem!=ITEM_NONE;
// QDOAS ???            indexItem=ANLYS_itemList[indexItem].indexPrevious)
// QDOAS ???
// QDOAS ???        indexOld[i]=indexItem;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     fprintf(fp,"%sPolynomial, base        ",indent);
// QDOAS ???
// QDOAS ???     for (indexItem=indexOld[TAB_TYPE_ANLYS_LINEAR],indexLine=0;
// QDOAS ???         (indexItem!=ITEM_NONE);
// QDOAS ???          indexItem=ANLYS_itemList[indexItem].indexNext,indexLine++)
// QDOAS ???      {
// QDOAS ???       pList=&ANLYS_itemList[indexItem];
// QDOAS ???
// QDOAS ???       for (polyDegree=0,i=0;i<6;i++)
// QDOAS ???        if (pList->itemText[i][0]=='1')
// QDOAS ???         polyDegree++;
// QDOAS ???
// QDOAS ???       if (polyDegree)
// QDOAS ???        {
// QDOAS ???         fprintf(fp,"{ ");
// QDOAS ???         polyDegree=0;
// QDOAS ???
// QDOAS ???         for (i=0;i<6;i++)
// QDOAS ???          if (pList->itemText[i][0]=='1')
// QDOAS ???           {
// QDOAS ???            if (polyDegree)
// QDOAS ???             fprintf(fp,", ");
// QDOAS ???            fprintf(fp,"%s%d",(!indexLine)?"x":"1/x",i);
// QDOAS ???            polyDegree++;
// QDOAS ???           }
// QDOAS ???
// QDOAS ???         fprintf(fp," }, { ");
// QDOAS ???         polyBase=0;
// QDOAS ???
// QDOAS ???         for (i=0;i<6;i++)
// QDOAS ???          if (pList->itemText[i][1]=='1')
// QDOAS ???           {
// QDOAS ???            if (polyBase)
// QDOAS ???             fprintf(fp,", ");
// QDOAS ???            fprintf(fp,"%s%d",(!indexLine)?"x":"1/x",i);
// QDOAS ???            polyBase++;
// QDOAS ???           }
// QDOAS ???
// QDOAS ???         fprintf(fp," }\n");
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     if (indexOld[TAB_TYPE_ANLYS_GAPS]!=ITEM_NONE)
// QDOAS ???      {
// QDOAS ???       fprintf(fp,"%sGaps (nm)               ",indent);
// QDOAS ???
// QDOAS ???       for (indexItem=indexOld[TAB_TYPE_ANLYS_GAPS],indexLine=0;
// QDOAS ???           (indexItem!=ITEM_NONE);
// QDOAS ???            indexItem=ANLYS_itemList[indexItem].indexNext,indexLine++)
// QDOAS ???        {
// QDOAS ???         pList=&ANLYS_itemList[indexItem];
// QDOAS ???
// QDOAS ???         fprintf(fp,"{ %g, %g }",atof(pList->itemText[COLUMN_GAPS_MINVAL]),atof(pList->itemText[COLUMN_GAPS_MAXVAL]));
// QDOAS ???         if (ANLYS_itemList[indexItem].indexNext!=ITEM_NONE)
// QDOAS ???          fprintf(fp,", ");
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       fprintf(fp,"\n");
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     fprintf(fp,"\n%sPredefined parameters   ",indent);
// QDOAS ???
// QDOAS ???     for (indexItem=indexOld[TAB_TYPE_ANLYS_NOTLINEAR],n=0;
// QDOAS ???         (indexItem!=ITEM_NONE);
// QDOAS ???          indexItem=ANLYS_itemList[indexItem].indexNext)
// QDOAS ???      {
// QDOAS ???       pList=&ANLYS_itemList[indexItem];
// QDOAS ???
// QDOAS ???       if ((pList->itemText[COLUMN_OTHERS_FIT][0]=='1') ||
// QDOAS ???           (STD_Stricmp(pList->itemText[COLUMN_OTHERS_PARAMS],"Sol") &&
// QDOAS ???           (fabs(atof(pList->itemText[COLUMN_OTHERS_VALINIT]))>(double)1.e-6)) ||
// QDOAS ???          (!STD_Stricmp(pList->itemText[COLUMN_OTHERS_PARAMS],"Sol") &&
// QDOAS ???           (fabs(atof(pList->itemText[COLUMN_OTHERS_VALINIT]))==(double)0.)))
// QDOAS ???        {
// QDOAS ???         if (n)
// QDOAS ???          fprintf(fp,"\n%s",indentXs);
// QDOAS ???         fprintf(fp,"%-25s fit %s, init %g, delta %g, min %g, max %g",
// QDOAS ???                    pList->itemText[COLUMN_OTHERS_PARAMS],                       // Non linear params
// QDOAS ???                   (pList->itemText[COLUMN_OTHERS_FIT][0]=='1')?"yes":"no",      // Fit flag
// QDOAS ???               atof(pList->itemText[COLUMN_OTHERS_VALINIT]),                     // Val init
// QDOAS ???               atof(pList->itemText[COLUMN_OTHERS_VALDELTA]),                    // Val del
// QDOAS ???               atof(pList->itemText[COLUMN_OTHERS_VALMIN]),                      // Val min
// QDOAS ???               atof(pList->itemText[COLUMN_OTHERS_VALMAX]));                     // Val max
// QDOAS ???         n++;
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     fprintf(fp,"\n\n%sSpectrum/Reference      ",indent);
// QDOAS ???
// QDOAS ???     for (indexItem=indexOld[TAB_TYPE_ANLYS_SHIFT_AND_STRETCH],n=0;
// QDOAS ???         (indexItem!=ITEM_NONE);
// QDOAS ???          indexItem=ANLYS_itemList[indexItem].indexNext)
// QDOAS ???      {
// QDOAS ???       pList=&ANLYS_itemList[indexItem];
// QDOAS ???
// QDOAS ???       if ((strstr(pList->itemText[0],"Ref")!=NULL) || (strstr(pList->itemText[0],"Spectrum")!=NULL))
// QDOAS ???        {
// QDOAS ???         if ((pList->itemText[COLUMN_SH_FIT][0]=='1') ||
// QDOAS ???             (fabs(atof(pList->itemText[COLUMN_SH_INIT]))>(double)1.e-6))
// QDOAS ???
// QDOAS ???          fprintf(fp,"Shift     : %s, fit %s, unit %s, init %g, delta %g, min %g, max %g\n",
// QDOAS ???                      pList->itemText[COLUMN_CROSS_TO_FIT],
// QDOAS ???                     (pList->itemText[COLUMN_SH_FIT][0]=='1')?"yes":"no",
// QDOAS ???                     (pProject->analysis.units==PRJCT_ANLYS_UNITS_PIXELS)?"pixels":"nm",
// QDOAS ???                 atof(pList->itemText[COLUMN_SH_INIT]),
// QDOAS ???                 atof(pList->itemText[COLUMN_SH_DELTA]),
// QDOAS ???                 atof(pList->itemText[COLUMN_SH_MIN]),
// QDOAS ???                 atof(pList->itemText[COLUMN_SH_MAX]));
// QDOAS ???
// QDOAS ???         if (STD_Stricmp(pList->itemText[COLUMN_ST_FIT],"None") ||
// QDOAS ???            (fabs(atof(pList->itemText[COLUMN_ST_INIT]))>(double)1.e-6))
// QDOAS ???
// QDOAS ???          fprintf(fp,"%sStretch   : %s, fit %s, init %g, delta %g\n",indentXs,
// QDOAS ???                      pList->itemText[COLUMN_CROSS_TO_FIT],
// QDOAS ???                      pList->itemText[COLUMN_ST_FIT],
// QDOAS ???                 atof(pList->itemText[COLUMN_ST_INIT]),
// QDOAS ???                 atof(pList->itemText[COLUMN_ST_DELTA]));
// QDOAS ???
// QDOAS ???         if (STD_Stricmp(pList->itemText[COLUMN_SC_FIT],"None") ||
// QDOAS ???            (fabs(atof(pList->itemText[COLUMN_SC_INIT]))>(double)1.e-6))
// QDOAS ???
// QDOAS ???          fprintf(fp,"%sScaling   : %s, fit %s,init %g, delta %g\n",indentXs,
// QDOAS ???                      pList->itemText[COLUMN_CROSS_TO_FIT],
// QDOAS ???                      pList->itemText[COLUMN_SC_FIT],
// QDOAS ???                 atof(pList->itemText[COLUMN_SC_INIT]),
// QDOAS ???                 atof(pList->itemText[COLUMN_SC_DELTA]));
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     fprintf(fp,"\n%sFitted molecules        --------------------------------------------------------------------------------\n",indent);
// QDOAS ???
// QDOAS ???     // Browse items in list
// QDOAS ???
// QDOAS ???     for (indexItem=indexOld[TAB_TYPE_ANLYS_CROSS];
// QDOAS ???         (indexItem!=ITEM_NONE);
// QDOAS ???          indexItem=ANLYS_itemList[indexItem].indexNext)
// QDOAS ???
// QDOAS ???      {
// QDOAS ???       pList=&ANLYS_itemList[indexItem];
// QDOAS ???
// QDOAS ???       fprintf(fp,"%sName      : %s\n",indentXs,pList->itemText[0]);
// QDOAS ???       FILES_RebuildFileName(string,pList->crossFileName,1);
// QDOAS ???       fprintf(fp,"%sFile      : %s\n",indentXs,string);
// QDOAS ???       fprintf(fp,"%sFitting   : %s (init : %.3le delta : %.2lf)\n",indentXs,
// QDOAS ???              (pList->itemText[(!pAnlys->hidden)?COLUMN_CROSS_CCFIT:COLUMN_CROSS_SVD_CCFIT][0]=='1')?"yes":"no",
// QDOAS ???          atof(pList->itemText[(!pAnlys->hidden)?COLUMN_CROSS_CCINIT:COLUMN_CROSS_SVD_CCINIT]),
// QDOAS ???          atof(pList->itemText[(!pAnlys->hidden)?COLUMN_CROSS_CCDELTA:COLUMN_CROSS_SVD_CCDELTA]));
// QDOAS ???
// QDOAS ???       fprintf(fp,"%sOrthog.   : %s\n",indentXs,pList->itemText[(!pAnlys->hidden)?COLUMN_CROSS_ORTHOGONAL:COLUMN_CROSS_SVD_ORTHOGONAL]);
// QDOAS ???       fprintf(fp,"%sAction    : %s ",indentXs,pList->itemText[(!pAnlys->hidden)?COLUMN_CROSS_XS_TYPE:COLUMN_CROSS_SVD_XS_TYPE]);
// QDOAS ???
// QDOAS ???       if (!STD_Stricmp(pList->itemText[(!pAnlys->hidden)?COLUMN_CROSS_XS_TYPE:COLUMN_CROSS_SVD_XS_TYPE],ANLYS_crossAction[ANLYS_CROSS_ACTION_CONVOLUTE_I0]))
// QDOAS ???        fprintf(fp,"(conc : %s)",pList->itemText[(!pAnlys->hidden)?COLUMN_CROSS_CCI0:COLUMN_CROSS_SVD_CCI0]);
// QDOAS ???       fprintf(fp,"\n");
// QDOAS ???
// QDOAS ???
// QDOAS ???       for (i=indexOld[TAB_TYPE_ANLYS_SHIFT_AND_STRETCH];
// QDOAS ???           (i!=ITEM_NONE);
// QDOAS ???            i=ANLYS_itemList[i].indexNext)
// QDOAS ???        {
// QDOAS ???         pList2=&ANLYS_itemList[i];
// QDOAS ???         strcpy(symbolList,pList2->itemText[COLUMN_CROSS_TO_FIT]);
// QDOAS ???
// QDOAS ???         for (symbol=symbolList;symbol!=NULL;symbol=nextSymbol)
// QDOAS ???          {
// QDOAS ???           // End of symbol
// QDOAS ???
// QDOAS ???           if ((nextSymbol=strchr(symbol,';'))!=NULL)
// QDOAS ???            *nextSymbol++=0;
// QDOAS ???           symbolLength=strlen(symbol);
// QDOAS ???
// QDOAS ???           if ((strlen(pList->itemText[0])==symbolLength) &&
// QDOAS ???               !STD_Stricmp(pList->itemText[0],symbol))
// QDOAS ???            {
// QDOAS ???             if ((pList2->itemText[COLUMN_SH_FIT][0]=='1') ||
// QDOAS ???                 (fabs(atof(pList2->itemText[COLUMN_SH_INIT]))>(double)1.e-6))
// QDOAS ???
// QDOAS ???              fprintf(fp,"%sShift     : %s, fit %s, unit %s, init %g, delta %g, min %g, max %g\n",indentXs,
// QDOAS ???                          pList2->itemText[COLUMN_CROSS_TO_FIT],
// QDOAS ???                         (pList2->itemText[COLUMN_SH_FIT][0]=='1')?"yes":"no",
// QDOAS ???                         (pProject->analysis.units==PRJCT_ANLYS_UNITS_PIXELS)?"pixels":"nm",
// QDOAS ???                     atof(pList2->itemText[COLUMN_SH_INIT]),
// QDOAS ???                     atof(pList2->itemText[COLUMN_SH_DELTA]),
// QDOAS ???                     atof(pList2->itemText[COLUMN_SH_MIN]),
// QDOAS ???                     atof(pList2->itemText[COLUMN_SH_MAX]));
// QDOAS ???
// QDOAS ???             if (STD_Stricmp(pList2->itemText[COLUMN_ST_FIT],"None") ||
// QDOAS ???                (fabs(atof(pList2->itemText[COLUMN_ST_INIT]))>(double)1.e-6))
// QDOAS ???
// QDOAS ???              fprintf(fp,"%sStretch   : %s, fit %s, init %g, delta %g\n",indentXs,
// QDOAS ???                          pList2->itemText[COLUMN_CROSS_TO_FIT],
// QDOAS ???                          pList2->itemText[COLUMN_ST_FIT],
// QDOAS ???                     atof(pList2->itemText[COLUMN_ST_INIT]),
// QDOAS ???                     atof(pList2->itemText[COLUMN_ST_DELTA]));
// QDOAS ???
// QDOAS ???             if (STD_Stricmp(pList2->itemText[COLUMN_SC_FIT],"None") ||
// QDOAS ???                (fabs(atof(pList2->itemText[COLUMN_SC_INIT]))>(double)1.e-6))
// QDOAS ???
// QDOAS ???              fprintf(fp,"%sScaling   : %s, fit %s,init %g, delta %g\n",indentXs,
// QDOAS ???                          pList2->itemText[COLUMN_CROSS_TO_FIT],
// QDOAS ???                          pList2->itemText[COLUMN_SC_FIT],
// QDOAS ???                     atof(pList2->itemText[COLUMN_SC_INIT]),
// QDOAS ???                     atof(pList2->itemText[COLUMN_SC_DELTA]));
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       if (STD_Stricmp(pList->itemText[COLUMN_CROSS_AMF_TYPE],"None"))
// QDOAS ???        {
// QDOAS ???         fprintf(fp,"%sAMF type  : %s\n",indentXs,pList->itemText[COLUMN_CROSS_AMF_TYPE]);
// QDOAS ???         fprintf(fp,"%sAMF file  : %s\n",indentXs,pList->amfFileName);
// QDOAS ???         fprintf(fp,"%sRes col   : %s\n",indentXs,pList->itemText[COLUMN_CROSS_RESIDUAL]);
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       if (((PRJCT_itemList[indexProject].lfilter.type!=PRJCT_FILTER_TYPE_NONE) &&
// QDOAS ???            (PRJCT_itemList[indexProject].lfilter.type!=PRJCT_FILTER_TYPE_ODDEVEN)) ||
// QDOAS ???           (PRJCT_itemList[indexProject].hfilter.type!=PRJCT_FILTER_TYPE_NONE))
// QDOAS ???       fprintf(fp,"%sFiltering : %s\n",indentXs,(pList->itemText[COLUMN_CROSS_FILTER][0]=='1')?"yes":"no");
// QDOAS ???       fprintf(fp,"%s--------------------------------------------------------------------------------\n",indentXs);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     fprintf(fp,"\n");
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   fprintf(fp,"%s++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n",indent);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        ANLYS_LoadConfigurationOld
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Load analysis windows from the wds configuration file (old version)
// QDOAS ??? //
// QDOAS ??? // INPUT           fileName : name of the wds configuration file
// QDOAS ??? //
// QDOAS ??? // OUTPUT          the 'environment space' tree is completed with the new
// QDOAS ??? //                 analysis window
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_WIN_) && __WINDOAS_WIN_
// QDOAS ??? void ANLYS_LoadConfigurationOld(UCHAR *fileName)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR keyName[MAX_ITEM_TEXT_LEN+1],                                           // key name in configuration file
// QDOAS ???         numberString[MAX_ITEM_TEXT_LEN+1],                                      // number of items in tab pages
// QDOAS ???         projectName[MAX_ITEM_TEXT_LEN+1],                                       // name of owner project
// QDOAS ???        *text,*ptrLine,*ptrColumn,                                               // pointers to buffer of characters
// QDOAS ???       (*nasaFieldsList)[MAX_ITEM_NAME_LEN+1];                                   // list of fields used in NASA-AMES format
// QDOAS ???   UCHAR refSZA[13],refSZADelta[13];
// QDOAS ???
// QDOAS ???   ANLYS_TAB_PAGE *pTab;                                                         // pointer to tab pages
// QDOAS ???   CHILD_WINDOWS *pChild;                                                        // pointer to MDI child window
// QDOAS ???   LIST_ITEM listItem;                                                           // item in tab page
// QDOAS ???
// QDOAS ???   INT windowsNumber,                                                            // number of analysis windows
// QDOAS ???       columnNumber,                                                             // number of columns in a tab page
// QDOAS ???       lineNumber,                                                               // number of lines in a tab page
// QDOAS ???       treeEntryPoint;                                                           // entry point in workspace environment tree
// QDOAS ???
// QDOAS ???   INDEX indexWindow,indexProject,indexTab,                                      // indexes for loops and arrays
// QDOAS ???         indexItem,indexData,indexAnlysParent,
// QDOAS ???         indexLine,indexColumn,indexListItem,
// QDOAS ???         indexTreeItem,indexField;
// QDOAS ???
// QDOAS ???   SZ_LEN anlysNameLength;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pChild=&CHILD_list[CHILD_WINDOW_PROJECT];
// QDOAS ???
// QDOAS ???   memset(ANLYS_refEtalonFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   memset(ANLYS_refFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   memset(ANLYS_residuals,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???   // Buffers allocation
// QDOAS ???
// QDOAS ???   text=(UCHAR *)MEMORY_AllocBuffer("ANLYS_LoadConfigurationOld ","text",1,sizeof(LIST_ITEM),0,MEMORY_TYPE_STRUCT);
// QDOAS ???
// QDOAS ???   // Get number of analysis windows in configuration file
// QDOAS ???
// QDOAS ???   if (GetPrivateProfileString(ANLYS_SECTION,"Number","0",text,sizeof(LIST_ITEM),fileName) &&
// QDOAS ???       sscanf(text,"%i",&windowsNumber) &&
// QDOAS ???       windowsNumber)
// QDOAS ???
// QDOAS ???    for (indexWindow=0;indexWindow<windowsNumber;indexWindow++)
// QDOAS ???     {
// QDOAS ???      memset(&ANLYS_options,0,sizeof(ANALYSIS_WINDOWS));
// QDOAS ???
// QDOAS ???      for (indexTab=0;indexTab<TAB_TYPE_ANLYS_MAX;indexTab++)
// QDOAS ???       ANLYS_options.listEntryPoint[indexTab]=ITEM_NONE;
// QDOAS ???
// QDOAS ???      // General informations safe keeping
// QDOAS ???
// QDOAS ???      sprintf(keyName,"Header %d",indexWindow);
// QDOAS ???
// QDOAS ???      if (GetPrivateProfileString(ANLYS_SECTION,keyName,",,0,,,,,0,0,,,1,1,1,1,0,0,0",text,sizeof(LIST_ITEM),fileName) &&
// QDOAS ???
// QDOAS ???          sscanf(text,"%[^','],%[^','],%d,%[^','],%[^','],%[^','],%[^','],%d,%d,%[^','],%[^','],%d,%d,%d,%d,%d,%d,%d",
// QDOAS ???                       projectName,ANLYS_options.windowName,&ANLYS_options.refSpectrumSelectionMode,
// QDOAS ???                       ANLYS_refFileName,refSZA,refSZADelta,ANLYS_refEtalonFileName,
// QDOAS ???                       &ANLYS_options.useKurucz,&ANLYS_options.useSref,ANLYS_options.lembdaMin,ANLYS_options.lembdaMax,
// QDOAS ???                       &ANLYS_options.displaySpectrum,&ANLYS_options.displayResidue,&ANLYS_options.displayTrend,&ANLYS_options.displayRefEtalon,
// QDOAS ???                       &ANLYS_options.displayFits,&ANLYS_options.displayPredefined,&ANLYS_options.hidden) &&
// QDOAS ???
// QDOAS ???         (projectName[0]!=0) && (ANLYS_options.windowName[0]!=0) &&
// QDOAS ???         (ANLYS_options.hidden ||
// QDOAS ???        ((ANLYS_options.lembdaMin[0]!=0) && (ANLYS_options.lembdaMax[0]!=0))) &&
// QDOAS ???
// QDOAS ???        ((indexProject=TREE_GetIndexByDataName(projectName,TREE_ITEM_TYPE_PROJECT,pChild->itemTree))!=ITEM_NONE) &&
// QDOAS ???        ((indexAnlysParent=TREE_itemList[indexProject].lastChildItem)!=ITEM_NONE) &&
// QDOAS ???        ((indexItem=TREE_InsertOneItem(pChild->hwndTree,ANLYS_options.windowName,indexAnlysParent,TREE_ITEM_TYPE_ANALYSIS_CHILDREN,0,0,(UCHAR)ANLYS_options.hidden))!=ITEM_NONE) &&
// QDOAS ???        ((indexData=TREE_itemList[indexItem].dataIndex)!=ITEM_NONE))
// QDOAS ???       {
// QDOAS ???        ANLYS_options.refSZA=(strlen(refSZA))?atof(refSZA):(double)0.;
// QDOAS ???        ANLYS_options.refSZADelta=(strlen(refSZADelta))?atof(refSZADelta):(double)0.;
// QDOAS ???
// QDOAS ???        if ((indexProject=TREE_itemList[indexProject].dataIndex)!=ITEM_NONE)
// QDOAS ???         {
// QDOAS ???          nasaFieldsList=PRJCT_itemList[indexProject].nasaResults.fields;
// QDOAS ???          anlysNameLength=strlen(ANLYS_options.windowName);
// QDOAS ???
// QDOAS ???          for (indexField=0;indexField<PRJCT_RESULTS_NASA_MAX;indexField++)
// QDOAS ???           if ((strlen(nasaFieldsList[indexField])==anlysNameLength) &&
// QDOAS ???               !STD_Stricmp(nasaFieldsList[indexField],ANLYS_options.windowName))
// QDOAS ???            TREE_itemList[indexItem].useCount++;
// QDOAS ???         }
// QDOAS ???
// QDOAS ???        // Set analysis window properties
// QDOAS ???
// QDOAS ???        FILES_CompactPath(ANLYS_options.refSpectrumFile,ANLYS_refFileName,1,1);
// QDOAS ???        FILES_CompactPath(ANLYS_options.refEtalon,ANLYS_refEtalonFileName,1,1);
// QDOAS ???        FILES_CompactPath(ANLYS_options.residualsFile,ANLYS_residuals,1,1);
// QDOAS ???
// QDOAS ???        memcpy(&ANLYS_windowsList[indexData],&ANLYS_options,sizeof(ANALYSIS_WINDOWS));
// QDOAS ???
// QDOAS ???        // Get number of items in each tab page
// QDOAS ???
// QDOAS ???        sprintf(keyName,"Number %d",indexWindow);
// QDOAS ???
// QDOAS ???        if (GetPrivateProfileString(ANLYS_SECTION,keyName,"0",numberString,MAX_ITEM_TEXT_LEN+1,fileName))
// QDOAS ???         {
// QDOAS ???          // Browse tab pages
// QDOAS ???
// QDOAS ???          for (indexTab=0,ptrLine=numberString;(indexTab<TAB_TYPE_ANLYS_MAX)&&(ptrLine!=NULL);indexTab++)
// QDOAS ???           {
// QDOAS ???            if (indexTab!=TAB_TYPE_ANLYS_OUTPUT)
// QDOAS ???             {
// QDOAS ???              pTab=&ANLYS_tabPages[indexTab];
// QDOAS ???              treeEntryPoint=(pTab->pTreeEntryPoint!=NULL)?*pTab->pTreeEntryPoint:ITEM_NONE;
// QDOAS ???
// QDOAS ???              // Set number of columns and lines in current tab page
// QDOAS ???
// QDOAS ???              columnNumber=pTab->columnNumber;
// QDOAS ???              sscanf(ptrLine,"%i",&lineNumber);
// QDOAS ???              indexListItem=ANLYS_windowsList[indexWindow].listEntryPoint[indexTab]=ITEM_NONE;
// QDOAS ???
// QDOAS ???              // Browse list items in current tab pages
// QDOAS ???
// QDOAS ???              for (indexLine=lineNumber-1;indexLine>=0;indexLine--)
// QDOAS ???               {
// QDOAS ???                sprintf(keyName,"item %d,%d,%d",indexWindow,indexTab,indexLine);
// QDOAS ???                memset(&listItem,0,(MAX_LIST_COLUMNS+2)*(MAX_ITEM_TEXT_LEN+1));
// QDOAS ???
// QDOAS ???                if (GetPrivateProfileString(ANLYS_SECTION,keyName,"",text,sizeof(LIST_ITEM),fileName))
// QDOAS ???                 {
// QDOAS ???                  // Columns read out
// QDOAS ???
// QDOAS ???                  for (indexColumn=0,ptrColumn=text;(indexColumn<columnNumber)&&(ptrColumn!=NULL);indexColumn++)
// QDOAS ???                   {
// QDOAS ???                    sscanf(ptrColumn,"%[^',']",listItem.itemText[indexColumn]);
// QDOAS ???                    if ((ptrColumn=strchr(ptrColumn,','))!=NULL)
// QDOAS ???                     ptrColumn++;
// QDOAS ???                   }
// QDOAS ???
// QDOAS ???                  // File names read out
// QDOAS ???
// QDOAS ???                  memset(ANLYS_crossFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???                  memset(ANLYS_amfFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???                  if ((ptrColumn!=NULL) && ((indexTab==TAB_TYPE_ANLYS_CROSS) || (indexTab==TAB_TYPE_ANLYS_NOTLINEAR)))
// QDOAS ???                   sscanf(ptrColumn,"%[^','],%[^'\n']",ANLYS_crossFileName,ANLYS_amfFileName);
// QDOAS ???
// QDOAS ???                  FILES_CompactPath(listItem.crossFileName,ANLYS_crossFileName,1,1);
// QDOAS ???                  FILES_CompactPath(listItem.amfFileName,ANLYS_amfFileName,1,1);
// QDOAS ???
// QDOAS ???                  // Insert item in list
// QDOAS ???
// QDOAS ???                  if ((indexListItem=ANLYS_windowsList[indexWindow].listEntryPoint[indexTab]=AnlysInsertListItem(indexData,indexListItem))!=ITEM_NONE)
// QDOAS ???                   {
// QDOAS ???                    memcpy(&ANLYS_itemList[indexListItem],&listItem,(MAX_LIST_COLUMNS+2)*(MAX_ITEM_TEXT_LEN+1));
// QDOAS ???
// QDOAS ???                    if ((indexTreeItem=TREE_GetIndexByDataName(ANLYS_itemList[indexListItem].itemText[0],pTab->symbolType,treeEntryPoint))!=ITEM_NONE)
// QDOAS ???                     TREE_itemList[indexTreeItem].useCount++;
// QDOAS ???                   }
// QDOAS ???                 }
// QDOAS ???               }
// QDOAS ???
// QDOAS ???              // Go to next tab page
// QDOAS ???
// QDOAS ???              if ((ptrLine=strchr(ptrLine,','))!=NULL)
// QDOAS ???               ptrLine++;
// QDOAS ???             }
// QDOAS ???           }
// QDOAS ???         }
// QDOAS ???       }
// QDOAS ???     }
// QDOAS ???
// QDOAS ???   // Release allocated buffers
// QDOAS ???
// QDOAS ???   MEMORY_ReleaseBuffer("ANLYS_LoadConfigurationOld ","text",text);
// QDOAS ???  }
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        ANLYS_LoadConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Load analysis windows from the wds configuration file
// QDOAS ??? //
// QDOAS ??? // INPUT           fileLine : a line from the 'analysis windows' section in
// QDOAS ??? //                            the wds configuration file
// QDOAS ??? //
// QDOAS ??? // OUTPUT          the 'environment space' tree is completed with the new
// QDOAS ??? //                 observation sites
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void ANLYS_LoadConfiguration(UCHAR *fileLine)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR projectName[MAX_ITEM_TEXT_LEN+1],                                       // the name of the parent project
// QDOAS ???         keyName[MAX_ITEM_TEXT_LEN+1],                                           // current key
// QDOAS ???       (*nasaFieldsList)[MAX_ITEM_NAME_LEN+1],                                   // list of fields used in NASA-AMES format
// QDOAS ???        *ptrColumn;
// QDOAS ???
// QDOAS ???   ANLYS_TAB_PAGE *pTab;                                                         // pointer to tab pages
// QDOAS ???   CHILD_WINDOWS *pChild;                                                        // pointer to MDI child window
// QDOAS ???   LIST_ITEM listItem;                                                           // item in tab page
// QDOAS ???   SZ_LEN anlysNameLength;
// QDOAS ???   INT    columnNumber,                                                          // number of columns in a tab page
// QDOAS ???          treeEntryPoint,                                                        // entry point in workspace environment tree
// QDOAS ???          gomePixelType[4];                                                      // gome pixel type
// QDOAS ???   INDEX indexWindow,indexTab,indexLine,                                         // index for loops and arrays
// QDOAS ???         indexProject,
// QDOAS ???         indexItem,indexData,indexAnlysParent,
// QDOAS ???         indexColumn,indexListItem,
// QDOAS ???         indexTreeItem,indexField,itmp,iorder,ibase,polyOld;
// QDOAS ???   UCHAR refSZA[13],refSZADelta[13];
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pChild=&CHILD_list[CHILD_WINDOW_PROJECT];
// QDOAS ???   indexTab=indexLine=ITEM_NONE;
// QDOAS ???   polyOld=0;
// QDOAS ???
// QDOAS ???   if ((sscanf(fileLine,"Header %d",&indexWindow)>=1) && (indexWindow!=ANLYS_indexWindow))
// QDOAS ???    {
// QDOAS ???     memset(&ANLYS_options,0,sizeof(ANALYSIS_WINDOWS));
// QDOAS ???
// QDOAS ???     for (indexTab=0;indexTab<TAB_TYPE_ANLYS_MAX;indexTab++)
// QDOAS ???      ANLYS_options.listEntryPoint[indexTab]=ITEM_NONE;
// QDOAS ???
// QDOAS ???     memset(ANLYS_refEtalonFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???     memset(ANLYS_refFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???     memset(ANLYS_residuals,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???     if ((STD_Sscanf(fileLine,"%[^'=']=%[^','],%[^','],%d,%[^','],%[^','],%[^','],%[^','],%d,%d,%[^','],%[^','],%d,%d,%d,%d,%d,%d,%d,%[^','],%d,%[^','],%[^','],%lf,%lf,%d,%d,%lf,%lf,%d,%d,%d,%d",
// QDOAS ???                               keyName,                                              //
// QDOAS ???                               projectName,                                          // project name
// QDOAS ???                               ANLYS_options.windowName,                             // analysis window
// QDOAS ???                              &ANLYS_options.refSpectrumSelectionMode,               // reference selection mode
// QDOAS ???                               ANLYS_refFileName,                                    // ref 2 file name in file selection mode for reference
// QDOAS ???                               refSZA,                                               // SZA in automatic selection mode for reference
// QDOAS ???                               refSZADelta,
// QDOAS ???                               ANLYS_refEtalonFileName,
// QDOAS ???                              &ANLYS_options.useKurucz,
// QDOAS ???                              &ANLYS_options.useSref,
// QDOAS ???                               ANLYS_options.lembdaMin,
// QDOAS ???                               ANLYS_options.lembdaMax,
// QDOAS ???                              &ANLYS_options.displaySpectrum,
// QDOAS ???                              &ANLYS_options.displayResidue,
// QDOAS ???                              &ANLYS_options.displayTrend,
// QDOAS ???                              &ANLYS_options.displayRefEtalon,
// QDOAS ???                              &ANLYS_options.displayFits,
// QDOAS ???                              &ANLYS_options.displayPredefined,
// QDOAS ???                              &ANLYS_options.hidden,
// QDOAS ???                               ANLYS_residuals,
// QDOAS ???                              &ANLYS_options.bandType,
// QDOAS ???                               ANLYS_options.lembdaMinK,
// QDOAS ???                               ANLYS_options.lembdaMaxK,
// QDOAS ???                    (double *)&ANLYS_options.refLatMin,
// QDOAS ???                    (double *)&ANLYS_options.refLatMax,
// QDOAS ???                       (INT *)&ANLYS_options.pixelType,
// QDOAS ???                       (INT *)&ANLYS_options.nspectra,
// QDOAS ???                    (double *)&ANLYS_options.refLonMin,
// QDOAS ???                    (double *)&ANLYS_options.refLonMax,
// QDOAS ???                       (INT *)&gomePixelType[0],
// QDOAS ???                       (INT *)&gomePixelType[1],
// QDOAS ???                       (INT *)&gomePixelType[2],
// QDOAS ???                       (INT *)&gomePixelType[3])>=3) &&
// QDOAS ???
// QDOAS ???         (projectName[0]!=0) && (ANLYS_options.windowName[0]!=0) &&
// QDOAS ???         (ANLYS_options.hidden ||
// QDOAS ???        ((ANLYS_options.lembdaMin[0]!=0) && (ANLYS_options.lembdaMax[0]!=0))) &&
// QDOAS ???
// QDOAS ???        ((indexProject=TREE_GetIndexByDataName(projectName,TREE_ITEM_TYPE_PROJECT,pChild->itemTree))!=ITEM_NONE) &&
// QDOAS ???        ((indexAnlysParent=TREE_itemList[indexProject].lastChildItem)!=ITEM_NONE) &&
// QDOAS ???        ((indexItem=TREE_InsertOneItem(pChild->hwndTree,ANLYS_options.windowName,indexAnlysParent,TREE_ITEM_TYPE_ANALYSIS_CHILDREN,0,0,(UCHAR)ANLYS_options.hidden))!=ITEM_NONE) &&
// QDOAS ???        ((indexData=TREE_itemList[indexItem].dataIndex)!=ITEM_NONE) &&
// QDOAS ???        ((indexProject=TREE_itemList[indexProject].dataIndex)!=ITEM_NONE))
// QDOAS ???      {
// QDOAS ???       ANLYS_options.gomePixelType[0]=(UCHAR)gomePixelType[0];
// QDOAS ???       ANLYS_options.gomePixelType[1]=(UCHAR)gomePixelType[1];
// QDOAS ???       ANLYS_options.gomePixelType[2]=(UCHAR)gomePixelType[2];
// QDOAS ???       ANLYS_options.gomePixelType[3]=(UCHAR)gomePixelType[3];
// QDOAS ???
// QDOAS ???       ANLYS_options.refSZA=(strlen(refSZA))?atof(refSZA):(double)0.;
// QDOAS ???       ANLYS_options.refSZADelta=(strlen(refSZADelta))?atof(refSZADelta):(double)0.;
// QDOAS ???
// QDOAS ???       // Set use flag
// QDOAS ???
// QDOAS ???       nasaFieldsList=PRJCT_itemList[indexProject].nasaResults.fields;
// QDOAS ???       anlysNameLength=strlen(ANLYS_options.windowName);
// QDOAS ???
// QDOAS ???       for (indexField=0;indexField<PRJCT_RESULTS_NASA_MAX;indexField++)
// QDOAS ???        if ((strlen(nasaFieldsList[indexField])==anlysNameLength) &&
// QDOAS ???            !STD_Stricmp(nasaFieldsList[indexField],ANLYS_options.windowName))
// QDOAS ???         TREE_itemList[indexItem].useCount++;
// QDOAS ???
// QDOAS ???       // Set analysis window properties
// QDOAS ???
// QDOAS ???       FILES_CompactPath(ANLYS_options.refSpectrumFile,ANLYS_refFileName,1,1);
// QDOAS ???       FILES_CompactPath(ANLYS_options.refEtalon,ANLYS_refEtalonFileName,1,1);
// QDOAS ???       FILES_CompactPath(ANLYS_options.residualsFile,ANLYS_residuals,1,1);
// QDOAS ???
// QDOAS ???       memcpy(&ANLYS_windowsList[indexData],&ANLYS_options,sizeof(ANALYSIS_WINDOWS));
// QDOAS ???
// QDOAS ???       ANLYS_indexWindow=indexWindow;
// QDOAS ???       ANLYS_indexData=indexData;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???   else if ((sscanf(fileLine,"item %d,%d,%d",&indexWindow,&indexTab,&indexLine)>=3) &&
// QDOAS ???            (indexWindow==ANLYS_indexWindow) && (ANLYS_indexData!=ITEM_NONE))
// QDOAS ???    {
// QDOAS ???     if ((indexTab>=0) && (indexTab<TAB_TYPE_ANLYS_OUTPUT))
// QDOAS ???      {
// QDOAS ???       pTab=&ANLYS_tabPages[indexTab];
// QDOAS ???       columnNumber=pTab->columnNumber;
// QDOAS ???
// QDOAS ???       treeEntryPoint=(pTab->pTreeEntryPoint!=NULL)?*pTab->pTreeEntryPoint:ITEM_NONE;
// QDOAS ???       indexListItem=(indexLine==0)?ITEM_NONE:ANLYS_windowsList[ANLYS_indexData].listEntryPoint[indexTab];
// QDOAS ???
// QDOAS ???       memset(&listItem,0,(MAX_LIST_COLUMNS+2)*(MAX_ITEM_TEXT_LEN+1)+sizeof(int));
// QDOAS ???
// QDOAS ???       #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???       if ((indexTab==TAB_TYPE_ANLYS_CROSS) && (FILES_version<HELP_VERSION_2_03))
// QDOAS ???        {
// QDOAS ???         strcpy(listItem.itemText[columnNumber-1],pTab->columnList[columnNumber-1].defaultValue);
// QDOAS ???         strcpy(listItem.itemText[columnNumber-2],pTab->columnList[columnNumber-2].defaultValue);
// QDOAS ???
// QDOAS ???         columnNumber-=2;
// QDOAS ???        }
// QDOAS ???       #endif
// QDOAS ???
// QDOAS ???       if ((ptrColumn=strchr(fileLine,'='))!=NULL)
// QDOAS ???        {
// QDOAS ???        	if ((indexTab==TAB_TYPE_ANLYS_LINEAR) && ((ptrColumn[1]=='1') || (ptrColumn[1]=='0')))
// QDOAS ???        	 {
// QDOAS ???        	 	strcpy(listItem.itemText[COLUMN_POLY_PARAMS],(indexLine==0)?"Polynomial (x)":"Polynomial (1/x)");
// QDOAS ???        	 	for (indexColumn=iorder=ibase=0;(indexColumn<6)&&(ptrColumn!=NULL);indexColumn++)
// QDOAS ???        	 	 {
// QDOAS ???        	 	  if (ptrColumn[1]=='1')
// QDOAS ???        	 	   iorder++;
// QDOAS ???        	 	  if (ptrColumn[2]=='1')
// QDOAS ???        	 	   ibase++;
// QDOAS ???
// QDOAS ???        	 	  if (ibase>iorder)
// QDOAS ???        	 	   ibase=iorder;
// QDOAS ???
// QDOAS ???             ptrColumn=strchr(ptrColumn+1,',');
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           strcpy(listItem.itemText[COLUMN_POLY_POLYORDER]  ,AnlysPolynome[iorder]);
// QDOAS ???           strcpy(listItem.itemText[COLUMN_POLY_BASEORDER]  ,AnlysPolynome[ibase]);
// QDOAS ???           strcpy(listItem.itemText[COLUMN_POLY_STORE_FIT]  ,"0");               // Store fit
// QDOAS ???           strcpy(listItem.itemText[COLUMN_POLY_STORE_ERROR],"0");               // Store error
// QDOAS ???
// QDOAS ???           if (indexLine)
// QDOAS ???            polyOld=1;
// QDOAS ???        	 }
// QDOAS ???
// QDOAS ???         else
// QDOAS ???
// QDOAS ???          // Columns read out
// QDOAS ???
// QDOAS ???          for (indexColumn=0,ptrColumn++;(indexColumn<columnNumber)&&(ptrColumn!=NULL);indexColumn++)
// QDOAS ???           {
// QDOAS ???            sscanf(ptrColumn,"%[^',']",listItem.itemText[indexColumn]);
// QDOAS ???
// QDOAS ???            if (!indexColumn && !strncmp(listItem.itemText[indexColumn],"Fwhm Fit Param",strlen("Fwhm Fit Param")))
// QDOAS ???             {
// QDOAS ???              sscanf(listItem.itemText[indexColumn],"Fwhm Fit Param%d",&itmp);
// QDOAS ???              sprintf(listItem.itemText[indexColumn],"SFP %d",itmp);
// QDOAS ???             }
// QDOAS ???
// QDOAS ???            if ((ptrColumn=strchr(ptrColumn,','))!=NULL)
// QDOAS ???             ptrColumn++;
// QDOAS ???           }
// QDOAS ???
// QDOAS ???         if ((indexTab==TAB_TYPE_ANLYS_NOTLINEAR) &&
// QDOAS ???            ((indexItem=TREE_GetIndexByDataIndex(indexWindow,TREE_ITEM_TYPE_ANALYSIS_CHILDREN,pChild->itemTree))!=ITEM_NONE) &&
// QDOAS ???            ((indexProject=TREE_itemList[TREE_itemList[TREE_itemList[indexItem].parentItem].parentItem].dataIndex)!=ITEM_NONE) &&
// QDOAS ???             !strnicmp(listItem.itemText[COLUMN_POLY_PARAMS],"Offset",6))
// QDOAS ???
// QDOAS ???          listItem.hidden=(PRJCT_itemList[indexProject].analysis.method==PRJCT_ANLYS_METHOD_SVD)?0:1;
// QDOAS ???
// QDOAS ???         // File names read out
// QDOAS ???
// QDOAS ???         memset(ANLYS_crossFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???         memset(ANLYS_amfFileName,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???         if ((ptrColumn!=NULL) && ((indexTab==TAB_TYPE_ANLYS_CROSS) || (indexTab==TAB_TYPE_ANLYS_NOTLINEAR)))
// QDOAS ???          sscanf(ptrColumn,"%[^','],%[^'\n']",ANLYS_crossFileName,ANLYS_amfFileName);
// QDOAS ???
// QDOAS ???         FILES_CompactPath(listItem.crossFileName,ANLYS_crossFileName,1,1);
// QDOAS ???         FILES_CompactPath(listItem.amfFileName,ANLYS_amfFileName,1,1);
// QDOAS ???
// QDOAS ???         // Insert item in list
// QDOAS ???
// QDOAS ???         if (!STD_Stricmp(listItem.itemText[COLUMN_CROSS_ORTHOGONAL],"Orthogonal base"))
// QDOAS ???          strcpy(listItem.itemText[COLUMN_CROSS_ORTHOGONAL],"Differential XS");
// QDOAS ???         if (!STD_Stricmp(listItem.itemText[COLUMN_CROSS_SVD_ORTHOGONAL],"Orthogonal base"))
// QDOAS ???          strcpy(listItem.itemText[COLUMN_CROSS_SVD_ORTHOGONAL],"Differential XS");
// QDOAS ???
// QDOAS ???         if  (STD_Stricmp(listItem.itemText[0],"Fwhm (Constant)") &&
// QDOAS ???              STD_Stricmp(listItem.itemText[0],"Fwhm (Order 1)") &&
// QDOAS ???              STD_Stricmp(listItem.itemText[0],"Fwhm (Order 2)") &&
// QDOAS ???              STD_Stricmp(listItem.itemText[0],"Test") &&
// QDOAS ???            ((indexListItem=ANLYS_windowsList[ANLYS_indexData].listEntryPoint[indexTab]=AnlysInsertListItem(ANLYS_indexData,indexListItem))!=ITEM_NONE))
// QDOAS ???          {
// QDOAS ???           memcpy(&ANLYS_itemList[indexListItem],&listItem,(MAX_LIST_COLUMNS+2)*(MAX_ITEM_TEXT_LEN+1)+sizeof(INT));
// QDOAS ???
// QDOAS ???           if ((indexTreeItem=TREE_GetIndexByDataName(ANLYS_itemList[indexListItem].itemText[0],pTab->symbolType,treeEntryPoint))!=ITEM_NONE)
// QDOAS ???            TREE_itemList[indexTreeItem].useCount++;
// QDOAS ???
// QDOAS ???           if (polyOld)
// QDOAS ???            {
// QDOAS ???            	memset(&listItem,0,(MAX_LIST_COLUMNS+2)*(MAX_ITEM_TEXT_LEN+1));
// QDOAS ???            	strcpy(listItem.itemText[0],"Offset");
// QDOAS ???            	strcpy(listItem.itemText[COLUMN_POLY_PARAMS]     ,"Offset");   // Linear params
// QDOAS ???             strcpy(listItem.itemText[COLUMN_POLY_POLYORDER]  ,"None");     // Polynomial order
// QDOAS ???             strcpy(listItem.itemText[COLUMN_POLY_BASEORDER]  ,"None");     // Orthogonal base order
// QDOAS ???             strcpy(listItem.itemText[COLUMN_POLY_STORE_FIT]  ,"0");        // Store fit
// QDOAS ???             strcpy(listItem.itemText[COLUMN_POLY_STORE_ERROR],"0");        // Store error
// QDOAS ???
// QDOAS ???             if ((indexListItem=ANLYS_windowsList[ANLYS_indexData].listEntryPoint[indexTab]=AnlysInsertListItem(ANLYS_indexData,indexListItem))!=ITEM_NONE)
// QDOAS ???              memcpy(&ANLYS_itemList[indexListItem],&listItem,(MAX_LIST_COLUMNS+2)*(MAX_ITEM_TEXT_LEN+1));
// QDOAS ???
// QDOAS ???             #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???             TLBAR_Enable(TRUE);
// QDOAS ???             #endif
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        ANLYS_SaveConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Analysis windows properties safe keeping
// QDOAS ??? //
// QDOAS ??? // INPUT           fp          : pointer to the current wds configuration file
// QDOAS ??? //                 sectionName : the name of the section for analysis windows
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void ANLYS_SaveConfiguration(FILE *fp,UCHAR *sectionName)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   ANALYSIS_WINDOWS *pAnlys;                                                     // pointer to analysis windows
// QDOAS ???   LIST_ITEM *pList;                                                             // pointer to items in list owned by tab pages
// QDOAS ???
// QDOAS ???   INT windowsNumber,                                                            // number of analysis windows
// QDOAS ???       columnNumber,                                                             // number of columns in a tab page
// QDOAS ???       lineNumber;                                                               // index of the current line
// QDOAS ???
// QDOAS ???   INDEX indexWindow,indexTab,indexItem,indexOld,
// QDOAS ???         indexColumn,indexProject;                                               // indexes for arrays and loops
// QDOAS ???
// QDOAS ???   // Print section name
// QDOAS ???
// QDOAS ???   fprintf(fp,"[%s]\n\n",sectionName);
// QDOAS ???
// QDOAS ???   // Save analysis windows into configuration file
// QDOAS ???
// QDOAS ???   windowsNumber=TREE_itemType[TREE_ITEM_TYPE_ANALYSIS_CHILDREN].dataNumber;
// QDOAS ???
// QDOAS ???   for (indexWindow=0;indexWindow<windowsNumber;indexWindow++)
// QDOAS ???    {
// QDOAS ???     pAnlys=&ANLYS_windowsList[indexWindow];
// QDOAS ???     indexProject=TREE_GetIndexByDataIndex(indexWindow,TREE_ITEM_TYPE_ANALYSIS_CHILDREN,CHILD_list[CHILD_WINDOW_PROJECT].itemTree);
// QDOAS ???
// QDOAS ???     // General informations safe keeping
// QDOAS ???
// QDOAS ???     fprintf(fp,"Header %d=%s,%s,%d,%s,%g,%g,%s,%d,%d,%s,%s,%d,%d,%d,%d,%d,%d,%d,%s,%d,%s,%s,%g,%g,%d,%d,%g,%g,%d,%d,%d,%d\n",indexWindow,
// QDOAS ???                (indexProject!=ITEM_NONE)?
// QDOAS ???                 TREE_itemList[TREE_itemList[TREE_itemList[indexProject].parentItem].parentItem].textItem:(UCHAR *)"",
// QDOAS ???                 pAnlys->windowName,
// QDOAS ???                 pAnlys->refSpectrumSelectionMode,
// QDOAS ???                 pAnlys->refSpectrumFile,
// QDOAS ???                 pAnlys->refSZA,
// QDOAS ???                 pAnlys->refSZADelta,
// QDOAS ???                 pAnlys->refEtalon,
// QDOAS ???                 pAnlys->useKurucz,
// QDOAS ???                 pAnlys->useSref,
// QDOAS ???                 pAnlys->lembdaMin,
// QDOAS ???                 pAnlys->lembdaMax,
// QDOAS ???                 pAnlys->displaySpectrum,
// QDOAS ???                 pAnlys->displayResidue,
// QDOAS ???                 pAnlys->displayTrend,
// QDOAS ???                 pAnlys->displayRefEtalon,
// QDOAS ???                 pAnlys->displayFits,
// QDOAS ???                 pAnlys->displayPredefined,
// QDOAS ???                 pAnlys->hidden,
// QDOAS ???                 pAnlys->residualsFile,
// QDOAS ???                 pAnlys->bandType,
// QDOAS ???                 pAnlys->lembdaMinK,
// QDOAS ???                 pAnlys->lembdaMaxK,
// QDOAS ???                 pAnlys->refLatMin,
// QDOAS ???                 pAnlys->refLatMax,
// QDOAS ???                 pAnlys->pixelType,
// QDOAS ???                 pAnlys->nspectra,
// QDOAS ???                 pAnlys->refLonMin,
// QDOAS ???                 pAnlys->refLonMax,
// QDOAS ???                 pAnlys->gomePixelType[0],
// QDOAS ???                 pAnlys->gomePixelType[1],
// QDOAS ???                 pAnlys->gomePixelType[2],
// QDOAS ???                 pAnlys->gomePixelType[3]);
// QDOAS ???
// QDOAS ???     // Browse tab pages
// QDOAS ???
// QDOAS ???     for (indexTab=0;indexTab<TAB_TYPE_ANLYS_MAX;indexTab++)
// QDOAS ???      {
// QDOAS ???       if (indexTab!=TAB_TYPE_ANLYS_OUTPUT)
// QDOAS ???        {
// QDOAS ???         // Set number of columns and lines in current tab page
// QDOAS ???
// QDOAS ???         columnNumber=ANLYS_tabPages[indexTab].columnNumber;
// QDOAS ???         lineNumber=0;
// QDOAS ???         indexOld=ITEM_NONE;
// QDOAS ???
// QDOAS ???         // Need to inverse the order of items
// QDOAS ???
// QDOAS ???         for (indexItem=pAnlys->listEntryPoint[indexTab];
// QDOAS ???              indexItem!=ITEM_NONE;
// QDOAS ???              indexItem=ANLYS_itemList[indexItem].indexPrevious)
// QDOAS ???
// QDOAS ???          indexOld=indexItem;
// QDOAS ???
// QDOAS ???         // Browse items in list
// QDOAS ???
// QDOAS ???         for (indexItem=indexOld;
// QDOAS ???              indexItem!=ITEM_NONE;
// QDOAS ???              indexItem=ANLYS_itemList[indexItem].indexNext,lineNumber++)
// QDOAS ???          {
// QDOAS ???           pList=&ANLYS_itemList[indexItem];
// QDOAS ???
// QDOAS ???           // Concatenate in "text" buffer, text in all columns
// QDOAS ???
// QDOAS ???           fprintf(fp,"item %d,%d,%d=",indexWindow,indexTab,lineNumber);
// QDOAS ???
// QDOAS ???           for (indexColumn=0;indexColumn<columnNumber;indexColumn++)
// QDOAS ???            fprintf(fp,"%s,",pList->itemText[indexColumn]);
// QDOAS ???
// QDOAS ???           // Concatenate name of file associated to buffer
// QDOAS ???
// QDOAS ???           if ((indexTab==TAB_TYPE_ANLYS_CROSS) || (indexTab==TAB_TYPE_ANLYS_NOTLINEAR))
// QDOAS ???            fprintf(fp,"%s,%s",pList->crossFileName,pList->amfFileName);
// QDOAS ???
// QDOAS ???           fprintf(fp,"\n");
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   fprintf(fp,"\n");
// QDOAS ???  }

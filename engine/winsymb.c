
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
//  Module purpose    :  SYMBOLS MODULE
//  Name of module    :  WINSYMB.C
//  Creation date     :  January 1997
//
//  Authors           :  Caroline FAYT (caroline.fayt@oma.be) - Windows version
//                       Jos VAN GEFFEN (josv@oma.be) - Linux version
//                       Michel VAN ROOZENDAEL (michelv@oma.be) - DOAS team leader
//
//        Copyright  (C) Belgian Institute for Space Aeronomy (BIRA-IASB)
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
//
//  MODULE DESCRIPTION
//
//  This module handles a list of symbols in the Environment space tree.   These
//  symbols are associated to the different cross sections.  They make the
//  selection of cross section files and their tracking during the DOAS analysis
//  easier (for example, to find their location in the SVD matrix).
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  ==================
//  SYMBOLS PROPERTIES
//  ==================
//
//  SymbolsDlgInit - initialize the symbols dialog box with the description of
//                   the selected item in the 'environment space' tree;
//
//  SymbolsSet - update the properties of the selected symbol;
//  SYMB_WndProc - dispatch messages from the symbols dialog box;
//
//  ===================
//  SEARCH FOR A SYMBOL
//  ===================
//
//  SYMB_GetListIndex - search for a symbol associated to a file name;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  SYMB_Alloc - allocate and initialize buffers for the symbols;
//  SYMB_Free - release the buffers allocated for the symbols;
//
//  =============================
//  CONFIGURATION FILE MANAGEMENT
//  =============================
//
//  SYMB_ResetConfiguration - remove the current list of symbols from the
//                            'Environment space' tree;
//  SYMB_LoadConfiguration - load symbols from the wds configuration file;
//  SYMB_SaveConfiguration - save the list of symbols in the wds in the configuration file;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "windoas.h"

// =================
// GLOBALS VARIABLES
// =================

SYMBOL_CROSS *SYMB_itemCrossList=NULL;                                          // pointer to the list of user-defined symbols
SYMBOL_CROSS *symbToPaste=NULL;                                                 // pointer to the list of symbols to paste
INDEX SYMB_crossTreeEntryPoint;                                                 // entry point in the 'environment space' for cross section symbols

// QDOAS ??? // ================
// QDOAS ??? // STATIC VARIABLES
// QDOAS ??? // ================
// QDOAS ???
// QDOAS ??? static SYMBOL *pSYMB_selected;                                                         // pointer to the selected symbol
// QDOAS ???
// QDOAS ??? // ==================
// QDOAS ??? // SYMBOLS PROPERTIES
// QDOAS ??? // ==================
// QDOAS ???
// QDOAS ??? #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SymbolsDlgInit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Initialize the symbols dialog box with the description of
// QDOAS ??? //                 the selected item in the 'environment space' tree
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndSymbols : the handle of the symbols dialog box
// QDOAS ??? //                 indexTree   : the index of the selected item in the tree
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void SymbolsDlgInit(HWND hwndSymbols,INDEX indexTree)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   SYMBOL *pSymbol;                                                              // pointer to the selected symbol
// QDOAS ???   INDEX indexSymbol;                                                            // index of the selected symbol in the list of symbols
// QDOAS ???   UCHAR textTitle[MAX_ITEM_TEXT_LEN+1],                                         // complete title of the dialog box
// QDOAS ???         textMsg[MAX_ITEM_TEXT_LEN+1];                                           // title of the dialog box retrieved from the resource file
// QDOAS ???   HWND hwndDesc;                                                                // handle of the description field in the dialog box
// QDOAS ???
// QDOAS ???   // Center dialog box on parent windows
// QDOAS ???
// QDOAS ???   DOAS_CenterWindow(hwndSymbols,GetWindow(hwndSymbols,GW_OWNER));
// QDOAS ???
// QDOAS ???   // Get the selected symbol
// QDOAS ???
// QDOAS ???   if ((indexTree!=ITEM_NONE) &&
// QDOAS ???      ((pSymbol=(SYMBOL *)TREE_itemType[TREE_itemList[indexTree].dataType].dataList)!=NULL) &&
// QDOAS ???      ((indexSymbol=TREE_itemList[indexTree].dataIndex)!=ITEM_NONE))
// QDOAS ???    {
// QDOAS ???     pSymbol=pSYMB_selected=&pSymbol[indexSymbol];
// QDOAS ???
// QDOAS ???     // Set window title
// QDOAS ???
// QDOAS ???     LoadString(DOAS_hInst,IDS_TITLE_SYMBOLS,textMsg,MAX_ITEM_TEXT_LEN);
// QDOAS ???     sprintf(textTitle,"%s : %s",textMsg,pSymbol->name);
// QDOAS ???     SetWindowText(hwndSymbols,textTitle);
// QDOAS ???
// QDOAS ???     // Description of the symbol
// QDOAS ???
// QDOAS ???     SendMessage((hwndDesc=GetDlgItem(hwndSymbols,SYMBOL_DESCRIPTION)),          // handle of the edit control
// QDOAS ???                  EM_LIMITTEXT,                                                  // limit the number of characters that the user can enter
// QDOAS ???                  MAX_ITEM_DESC_LEN,                                             // maximum number of characters authorized
// QDOAS ???                  0);                                                            // not used
// QDOAS ???
// QDOAS ???     SetWindowText(hwndDesc,pSymbol->description);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SymbolsSet
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Update the properties of the selected symbol
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndSymbols : the handle of the symbols dialog box
// QDOAS ??? //                 pSymbol     : pointer to the new options
// QDOAS ??? //
// QDOAS ??? // OUTPUT          the selected item in the 'environment space' tree is modified
// QDOAS ??? //
// QDOAS ??? // RETURN          ERROR_ID_NO
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC SymbolsSet(HWND hwndSymbols,SYMBOL *pSymbol)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR description[MAX_ITEM_DESC_LEN+1];                                       // the description of the symbol
// QDOAS ???   RC rc;                                                                        // return code
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   memset(description,0,MAX_ITEM_DESC_LEN);
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   if (pSymbol!=NULL)
// QDOAS ???    {
// QDOAS ???    	// Update the properties of the symbol in the 'Environment space' tree
// QDOAS ???
// QDOAS ???     GetWindowText(GetDlgItem(hwndSymbols,SYMBOL_DESCRIPTION),description,MAX_ITEM_DESC_LEN);
// QDOAS ???     strcpy(pSymbol->description,description);
// QDOAS ???
// QDOAS ???     // Activate the 'Save' button in the toolbar
// QDOAS ???
// QDOAS ???     if (!TLBAR_bSaveFlag)
// QDOAS ???      TLBAR_Enable(TRUE);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SYMB_WndProc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         dispatch messages from the symbols dialog box
// QDOAS ??? //
// QDOAS ??? // INPUT           usual syntax for windows messages routines
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? LRESULT CALLBACK SYMB_WndProc(HWND hwndSymbols,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch (msg)
// QDOAS ???    {
// QDOAS ???  // ----------------------------------------------------------------------------
// QDOAS ???     case WM_INITDIALOG :
// QDOAS ???      SymbolsDlgInit(hwndSymbols,(INDEX)mp2);
// QDOAS ???     break;
// QDOAS ???  // ----------------------------------------------------------------------------
// QDOAS ???     case WM_COMMAND :
// QDOAS ???      {
// QDOAS ???       ULONG commandID;
// QDOAS ???
// QDOAS ???       if ((((commandID=(ULONG)GET_WM_COMMAND_ID(mp1,mp2))==IDOK) &&             // close dialog box on OK button command
// QDOAS ???             !SymbolsSet(hwndSymbols,pSYMB_selected)) ||
// QDOAS ???             (commandID==IDCANCEL))                                              // close dialog on Cancel button command
// QDOAS ???
// QDOAS ???        EndDialog(hwndSymbols,0);
// QDOAS ???
// QDOAS ???       else if (commandID==IDHELP)
// QDOAS ???
// QDOAS ???        WinHelp(hwndSymbols,DOAS_HelpPath,HELP_CONTEXT,HLP_WORKSPACE_SYMBOLS);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_DESTROY :
// QDOAS ???      pSYMB_selected=NULL;
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // ===================
// QDOAS ??? // SEARCH FOR A SYMBOL
// QDOAS ??? // ===================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SYMB_GetListIndex
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Search for a symbol associated to a file name
// QDOAS ??? //
// QDOAS ??? // INPUT           symbolList   : the list of symbols in which to search for;
// QDOAS ??? //                 symbolNumber : the size of the input list
// QDOAS ??? //                 fileName     : the name of the file associated to the symbol
// QDOAS ??? //
// QDOAS ??? // RETURN          the index in the list, of the symbol associated to the file
// QDOAS ??? //                 ITEM_NONE if not found
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INDEX SYMB_GetListIndex(SYMBOL *symbolList,INT symbolNumber,UCHAR *fileName)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR symbolName[MAX_PATH_LEN+1],*ptr;                                        // the name of the symbol to search for
// QDOAS ???   INDEX indexSymbol;                                                            // browse symbols in the input list
// QDOAS ???   SZ_LEN symbolNameLen;                                                         // the length of the name of the symbol
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   memset(symbolName,0,MAX_PATH_LEN+1);
// QDOAS ???   indexSymbol=ITEM_NONE;
// QDOAS ???
// QDOAS ???   if ((symbolList!=NULL) && (symbolNumber>0) && (strlen(fileName)>0))
// QDOAS ???    {
// QDOAS ???     // Retrieve the name of the symbol from the file name
// QDOAS ???
// QDOAS ???     strncpy(symbolName,fileName,MAX_PATH_LEN);
// QDOAS ???
// QDOAS ???     // The symbol name should start the file name (so search for the last '\' (WINDOWS) or '/' (UNIX,LINUX)
// QDOAS ???
// QDOAS ???     if ((ptr=strrchr(fileName,PATH_SEP))!=NULL)
// QDOAS ???      strcpy(symbolName,ptr+1);
// QDOAS ???
// QDOAS ???     // Search for the end of the symbol ('_' if a description is given in the file name or '.' for the file extension)
// QDOAS ???
// QDOAS ???     if (((ptr=strchr(symbolName,'_'))!=NULL) || ((ptr=strchr(symbolName,'.'))!=NULL))
// QDOAS ???        *ptr=0;
// QDOAS ???
// QDOAS ???     // In the case a list of cross sections files is provided, stop at the first file
// QDOAS ???
// QDOAS ???     if ((ptr=strchr(symbolName,';'))!=NULL)
// QDOAS ???      *ptr=0;
// QDOAS ???
// QDOAS ???     // Get the length of the name of the symbol
// QDOAS ???
// QDOAS ???     symbolNameLen=strlen(symbolName);
// QDOAS ???
// QDOAS ???     // Search for the symbol in the input list
// QDOAS ???
// QDOAS ???     for (indexSymbol=0;indexSymbol<symbolNumber;indexSymbol++)
// QDOAS ???      if (((int)strlen(symbolList[indexSymbol].name)==symbolNameLen) &&
// QDOAS ???          !STD_Stricmp(symbolList[indexSymbol].name,symbolName))
// QDOAS ???       break;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return (indexSymbol<symbolNumber) ? indexSymbol : ITEM_NONE;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ====================
// QDOAS ??? // RESOURCES MANAGEMENT
// QDOAS ??? // ====================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SYMB_Alloc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         allocate and initialize buffers for the symbols
// QDOAS ??? //
// QDOAS ??? // RETURN          ERROR_ID_ALLOC if one of the buffer allocation failed
// QDOAS ??? //                 ERROR_ID_NO in case of success
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC SYMB_Alloc(void)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Memory allocation for cross sections symbols
// QDOAS ???
// QDOAS ???   if (((TREE_itemType[TREE_ITEM_TYPE_CROSS_CHILDREN].dataList=(SYMBOL_CROSS *)
// QDOAS ???         MEMORY_AllocBuffer("SYMB_Alloc","SYMB_itemCrossList",MAX_SYMBOL_CROSS,sizeof(SYMBOL_CROSS),0,MEMORY_TYPE_STRUCT))==NULL) ||
// QDOAS ???
// QDOAS ???       ((TREE_itemType[TREE_ITEM_TYPE_CROSS_CHILDREN].data2Paste=(SYMBOL_CROSS *)
// QDOAS ???         MEMORY_AllocBuffer("SYMB_Alloc","symbToPaste",MAX_SYMBOL_CROSS,sizeof(SYMBOL_CROSS),0,MEMORY_TYPE_STRUCT))==NULL))
// QDOAS ???
// QDOAS ???    rc=ERROR_ID_ALLOC;
// QDOAS ???
// QDOAS ???   else
// QDOAS ???    {
// QDOAS ???     SYMB_itemCrossList=(SYMBOL_CROSS *)TREE_itemType[TREE_ITEM_TYPE_CROSS_CHILDREN].dataList;
// QDOAS ???     symbToPaste=(SYMBOL_CROSS *)TREE_itemType[TREE_ITEM_TYPE_CROSS_CHILDREN].data2Paste;
// QDOAS ???
// QDOAS ???     memset(SYMB_itemCrossList,0,sizeof(SYMBOL_CROSS)*MAX_SYMBOL_CROSS);
// QDOAS ???
// QDOAS ???     // Add predefined symbols
// QDOAS ???
// QDOAS ???     strcpy(SYMB_itemCrossList[SYMBOL_PREDEFINED_SPECTRUM].name,"Spectrum");
// QDOAS ???     strcpy(SYMB_itemCrossList[SYMBOL_PREDEFINED_REF].name,"Ref");
// QDOAS ???     strcpy(SYMB_itemCrossList[SYMBOL_PREDEFINED_COM].name,"Com");
// QDOAS ???     strcpy(SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP1].name,"Usamp1");
// QDOAS ???     strcpy(SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP2].name,"Usamp2");
// QDOAS ???     strcpy(SYMB_itemCrossList[SYMBOL_PREDEFINED_RING1].name,"Raman");
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SYMB_Free
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Release the buffers allocated for the symbols
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void SYMB_Free(void)
// QDOAS ???  {
// QDOAS ???   if (SYMB_itemCrossList)
// QDOAS ???    MEMORY_ReleaseBuffer("SYMB_Free","SYMB_itemCrossList",SYMB_itemCrossList);
// QDOAS ???   if (symbToPaste!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("SYMB_Free","symbToPaste",symbToPaste);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // =============================
// QDOAS ??? // CONFIGURATION FILE MANAGEMENT
// QDOAS ??? // =============================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SYMB_ResetConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Remove the current list of symbols from the environment space tree
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void SYMB_ResetConfiguration(void)
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
// QDOAS ???   TREE_DeleteChildList(SYMB_crossTreeEntryPoint);
// QDOAS ???
// QDOAS ???   #if defined(__WINDOAS_GUI_) && (__WINDOAS_GUI_)
// QDOAS ???   TREE_UpdateItem(pChild->hwndTree,SYMB_crossTreeEntryPoint);
// QDOAS ???   #endif
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SYMB_LoadConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Load symbols from the wds configuration file
// QDOAS ??? //
// QDOAS ??? // INPUT           fileLine : a line from the 'symbols' section in the wds
// QDOAS ??? //                            configuration file
// QDOAS ??? //
// QDOAS ??? // OUTPUT          the 'environment space' tree is completed with the new symbol
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void SYMB_LoadConfiguration(UCHAR *fileLine)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR keyName[MAX_ITEM_TEXT_LEN+1];                                           // the name of the symbol
// QDOAS ???   CHILD_WINDOWS *pChild;                                                        // entry point for symbols in the 'environment space' tree
// QDOAS ???   SYMBOL symbol;                                                                // information on the new symbol
// QDOAS ???   INDEX indexItem,indexData;                                                    // indexes resp. in the tree and in the symbols list
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pChild=&CHILD_list[CHILD_WINDOW_ENVIRONMENT];
// QDOAS ???   memset(&symbol,0,sizeof(symbol));
// QDOAS ???
// QDOAS ???   // Retrieve the information on the symbol from the configuration file line
// QDOAS ???
// QDOAS ???   if ((((FILES_version>=HELP_VERSION_1_04) && (STD_Sscanf(fileLine,"%[^'=']=%[^',\n']",symbol.name,symbol.description)>=1)) ||
// QDOAS ???        ((STD_Sscanf(fileLine,"%[^'=']=%[^','],%[^'\n']",keyName,symbol.name,symbol.description)>=2) &&
// QDOAS ???         (STD_Stricmp(keyName,"Number")!=0))) &&
// QDOAS ???
// QDOAS ???   // Add the new item in the 'Environment space' tree
// QDOAS ???
// QDOAS ???        ((indexItem=TREE_InsertOneItem(pChild->hwndTree,symbol.name,SYMB_crossTreeEntryPoint,TREE_ITEM_TYPE_CROSS_CHILDREN,0,0,0))!=ITEM_NONE) &&
// QDOAS ???        ((indexData=TREE_itemList[indexItem].dataIndex)!=ITEM_NONE))
// QDOAS ???
// QDOAS ???    memcpy(&SYMB_itemCrossList[indexData],&symbol,sizeof(SYMBOL));
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        SYMB_SaveConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         Save the list of symbols in the wds in the configuration file
// QDOAS ??? //
// QDOAS ??? // INPUT           fp          : pointer to the current wds configuration file
// QDOAS ??? //                 sectionName : the name of the section for symbols
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void SYMB_SaveConfiguration(FILE *fp,UCHAR *sectionName)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INT symbolsNumber;                                                            // the number of symbols in the list
// QDOAS ???   INDEX indexSymbol;                                                            // browse symbols in the list
// QDOAS ???
// QDOAS ???   // Print out the section name
// QDOAS ???
// QDOAS ???   fprintf(fp,"[%s]\n\n",sectionName);
// QDOAS ???
// QDOAS ???   // Get the number of symbols in the 'Environment space' tree
// QDOAS ???
// QDOAS ???   symbolsNumber=TREE_itemType[TREE_ITEM_TYPE_CROSS_CHILDREN].dataNumber;
// QDOAS ???
// QDOAS ???   // Save the symbols in the configuration file
// QDOAS ???
// QDOAS ???   for (indexSymbol=SYMBOL_PREDEFINED_MAX;indexSymbol<symbolsNumber;indexSymbol++)
// QDOAS ???
// QDOAS ???    fprintf(fp,"%s=%s,\n",
// QDOAS ???                SYMB_itemCrossList[indexSymbol].name,
// QDOAS ???                SYMB_itemCrossList[indexSymbol].description);
// QDOAS ???
// QDOAS ???   fprintf(fp,"\n");
// QDOAS ???  }

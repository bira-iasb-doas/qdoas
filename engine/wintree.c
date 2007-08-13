
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  DOAS ANALYSIS PROGRAM FOR WINDOWS
//  Module purpose    :  TREEVIEW CONTROL MODULE
//  Name of module    :  WINTREE.C
//  Program Language  :  Borland C++ 5.0 for Windows 95/NT
//  Creation date     :  January 1997
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
//  This module includes all functions for Projects and Environment trees processing
//  (tree nodes handling and organisation, trees messages, context menus and resources
//  processing).
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  ===============
//  TREE PROCESSING
//  ===============
//
//  TREE_GetProjectParent - search for the parent project;
//  TreeUpdateDataIndex - when deleting an item in the tree, update data indexes of all nodes of the same type as the one to delete;
//  TreeResetData - reset the data description of a tree node to delete;
//  TREE_DeleteOneItem - delete a tree item
//  TREE_CollapseChildNodes - collapse the list of child nodes and free memory allocated for these items
//  TREE_ExpandOneNode - expand the list of child nodes;
//  TREE_SortChildNodes - sort child items of a tree node;
//  TreeShow - recursive function used to show all hidden child nodes;
//  TreeCreateImageList - create an image list to use with the 'Projects' tree;
//  TREE_GetIndexByDataName - get the index of a tree node from a name and a data type;
//  TREE_GetIndexByDataIndex - get the index of a tree node from its data description;
//  TREE_GetIndexesByDataType - get the list of indexes (in the objects list) of all nodes of the same data type as the one specified in argument;
//  TREE_UpdateItem - update the item attributes of a tree node;
//  TREE_Reset - recursive function to invalidate handles of all child items;
//  TREE_GetSelectedItem - get the index in the tree of the selected node;
//  TREE_InsertOneItem - insert a new item in the tree;
//  TREE_DeleteChildList - recursive function to delete childs of a node;
//
//  ==================================================
//  MDI CHILD WINDOWS NOTIFICATION MESSAGES PROCESSING
//  ==================================================
//
//  TREE_GetDispInfo - TVN_GETDISPINFO notification message processing;
//  TREE_BeginLabelEdit - TVN_BEGINLABELEDIT notification message processing
//  TREE_EndLabelEdit - TVN_ENDLABELEDIT notification message processing;
//  TREE_ExpandNode - TVN_ITEMEXPANDING notification message processing;
//  TREE_CollapseNode - TVN_ITEMEXPANDED notification message processing;
//
//  ================================
//  CONTEXT MENU COMMANDS PROCESSING
//  ================================
//
//  TREE_InsertItem - MENU_CONTEXT_INSERT command processing;
//  TREE_Show - MENU_CONTEXT_SHOW command processing;
//  TREE_Hide - MENU_CONTEXT_HIDE command processing;
//  TREE_RenameItem - MENU_CONTEXT_RENAME command processing;
//  TREE_Sort - MENU_CONTEXT_SORT_NAME commands processing;
//  TREE_DeleteItem - MENU_CONTEXT_DELETE command processing;
//  TREE_DeleteAllItems - MENU_CONTEXT_DELETEALL command processing;
//  TREE_ExpandAll - MENU_CONTEXT_EXPAND command processing;
//  TREE_CollapseAll - MENU_CONTEXT_COLLAPSE command processing;
//  TREE_Properties - MENU_CONTEXT_PROPERTIES command processing;
//  TREE_Copy - MENU_CONTEXT_COPY command processing;
//  TREE_Paste - MENU_CONTEXT_PASTE command processing;
//
//  ===================
//  MESSAGES PROCESSING
//  ===================
//
//  TREE_Create - create a new TreeView Control in the current MDI child window;
//  TREE_ReSize - resize the TreeView control onto its parent window;
//  TREE_Context - context menus processing for TreeView controls;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  TREE_Alloc - allocation and initialization of a structured list for holding the tree objects;
//  TREE_Init - tree resources initialization;
//  TREE_Free - release the buffer allocated for the tree objects.
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include "windoas.h"

// ================
// GLOBAL VARIABLES
// ================

// Tree objects to handle

TREE_ITEM  *TREE_itemList=NULL;                  // list of all tree objects
INDEX       treeItemFree=0;                      // index of the first available element in list
UCHAR       TREE_editFlag=0;                     // flag set when editing a tree item label

// Image list

#if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
HIMAGELIST  TREE_hImageList;                     // Handle to image list to associate to folders items
#endif
INDEX       TREE_blankImageIndex=ITEM_NONE,      // index of the 'blank' bitmap image in the image list
            TREE_openImageIndex=ITEM_NONE,       // index of the 'open' bitmap image in the image list
            TREE_closeImageIndex=ITEM_NONE;      // index of the 'close' bitmap image in the image list

// Copy/Paste

INDEX       TREE_typeOfItem2Copy=ITEM_NONE;      // type of the item to copy
INDEX       treeCutIndex=ITEM_NONE;              // index of the item to cut
INT         treeNItem2Copy=0;                    // number of tree items to copy

// Tree items types specifications

TREE_ITEM_TYPE TREE_itemType[TREE_ITEM_TYPE_MAX];

// QDOAS ??? =
// QDOAS ???  {
// QDOAS ???    // TREE_ITEM_TYPE_NONE
// QDOAS ???    // -------------------
// QDOAS ???    {
// QDOAS ???                                              0,  // ContextMenu
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                                              0,  // Dialog box
// QDOAS ???                                    (DLGPROC) 0,  // Dialog procedure
// QDOAS ??? #endif
// QDOAS ???                            TREE_ITEM_TYPE_NONE,  // Type of data for children
// QDOAS ???                                           NULL,  // Pointer to objects list
// QDOAS ???                                           NULL,  // pointer to the structure storing the data to paste
// QDOAS ???                                              0,  // Number of items in previous list
// QDOAS ???                                              0,  // Maximum number of items authorized in previous list
// QDOAS ???                                              0,  // Object size in bytes
// QDOAS ???                                              0   // type of file for children
// QDOAS ???    },
// QDOAS ???
// QDOAS ???    // TREE_ITEM_TYPE_SITE_PARENT
// QDOAS ???    // --------------------------
// QDOAS ???    {
// QDOAS ???                            MENU_CONTEXT_INSERT,  // ContextMenu
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                                              0,  // Dialog box
// QDOAS ???                                    (DLGPROC) 0,  // Dialog procedure
// QDOAS ??? #endif
// QDOAS ???                   TREE_ITEM_TYPE_SITE_CHILDREN,  // Type of data for children
// QDOAS ???                                           NULL,  // Pointer to objects list
// QDOAS ???                                           NULL,  // pointer to the structure storing the data to paste
// QDOAS ???                                              0,  // Number of items in previous list
// QDOAS ???                                              0,  // Maximum number of items authorized in previous list
// QDOAS ???                                              0,  // Object size in bytes
// QDOAS ???                                              0   // type of file for children
// QDOAS ???    },
// QDOAS ???
// QDOAS ???    // TREE_ITEM_TYPE_SITE_CHILDREN
// QDOAS ???    // ----------------------------
// QDOAS ???    {
// QDOAS ???                              MENU_CONTEXT_EDIT,  // ContextMenu
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                                      DLG_SITES,  // Dialog box
// QDOAS ???                        (DLGPROC) SITES_WndProc,  // Dialog procedure
// QDOAS ??? #endif
// QDOAS ???                            TREE_ITEM_TYPE_NONE,  // Type of data for children
// QDOAS ???                                           NULL,  // Pointer to objects list
// QDOAS ???                                           NULL,  // pointer to the structure storing the data to paste
// QDOAS ???                                              0,  // Number of items in previous list
// QDOAS ???                                      MAX_SITES,  // Maximum number of items authorized in previous list
// QDOAS ???                       sizeof(OBSERVATION_SITE),  // Object size in bytes
// QDOAS ???                                              0   // type of file for children
// QDOAS ???    },
// QDOAS ???
// QDOAS ???    // TREE_ITEM_TYPE_CROSS_PARENT
// QDOAS ???    // ---------------------------
// QDOAS ???    {
// QDOAS ???                            MENU_CONTEXT_INSERT,  // ContextMenu
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                                              0,  // Dialog box
// QDOAS ???                                    (DLGPROC) 0,  // Dialog procedure
// QDOAS ??? #endif
// QDOAS ???                  TREE_ITEM_TYPE_CROSS_CHILDREN,  // Type of data for children
// QDOAS ???                                           NULL,  // Pointer to objects list
// QDOAS ???                                           NULL,  // pointer to the structure storing the data to paste
// QDOAS ???                                              0,  // Number of items in previous list
// QDOAS ???                                              0,  // Maximum number of items authorized in previous list
// QDOAS ???                                              0,  // Object size in bytes
// QDOAS ???                                              0   // type of file for children
// QDOAS ???    },
// QDOAS ???
// QDOAS ???    // TREE_ITEM_TYPE_CROSS_CHILDREN
// QDOAS ???    // -----------------------------
// QDOAS ???    {
// QDOAS ???                              MENU_CONTEXT_EDIT,  // ContextMenu
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                                    DLG_SYMBOLS,  // Dialog box
// QDOAS ???                         (DLGPROC) SYMB_WndProc,  // Dialog procedure
// QDOAS ??? #endif
// QDOAS ???                            TREE_ITEM_TYPE_NONE,  // Type of data for children
// QDOAS ???                                           NULL,  // Pointer to objects list
// QDOAS ???                                           NULL,  // pointer to the structure storing the data to paste
// QDOAS ???                          SYMBOL_PREDEFINED_MAX,  // Number of items in previous list
// QDOAS ???                               MAX_SYMBOL_CROSS,  // Maximum number of items authorized in previous list
// QDOAS ???                           sizeof(SYMBOL_CROSS),  // Object size in bytes
// QDOAS ???                                              0   // type of file for children
// QDOAS ???    },
// QDOAS ???
// QDOAS ???    // TREE_ITEM_TYPE_PROJECT_PARENT
// QDOAS ???    // -----------------------------
// QDOAS ???    {
// QDOAS ???                                              0,  // ContextMenu
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                                              0,  // Dialog box
// QDOAS ???                                    (DLGPROC) 0,  // Dialog procedure
// QDOAS ??? #endif
// QDOAS ???                         TREE_ITEM_TYPE_PROJECT,  // Type of data for children
// QDOAS ???                                           NULL,  // Pointer to objects list
// QDOAS ???                                           NULL,  // pointer to the structure storing the data to paste
// QDOAS ???                                              0,  // Number of items in previous list
// QDOAS ???                                              0,  // Maximum number of items authorized in previous list
// QDOAS ???                                              0,  // Object size in bytes
// QDOAS ???                                              0   // type of file for children
// QDOAS ???    },
// QDOAS ???
// QDOAS ???    // TREE_ITEM_TYPE_PROJECT
// QDOAS ???    // ----------------------
// QDOAS ???    {
// QDOAS ???                           MENU_CONTEXT_PROJECT,  // ContextMenu
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                                      DLG_PRJCT,  // Dialog box
// QDOAS ???                        (DLGPROC) PRJCT_WndProc,  // Dialog procedure
// QDOAS ??? #endif
// QDOAS ???                            TREE_ITEM_TYPE_NONE,  // Type of data for children
// QDOAS ???                                           NULL,  // Pointer to objects list
// QDOAS ???                                           NULL,  // pointer to the structure storing the data to paste
// QDOAS ???                                              0,  // Number of items in previous list
// QDOAS ???                                              0,  // Maximum number of items authorized in previous list
// QDOAS ???                                sizeof(PROJECT),  // Object size in bytes
// QDOAS ???                                              0   // type of file for children
// QDOAS ???    },
// QDOAS ???
// QDOAS ???    // TREE_ITEM_TYPE_FILE_PARENT
// QDOAS ???    // --------------------------
// QDOAS ???    {
// QDOAS ???                      MENU_CONTEXT_INSERT_FILES,  // ContextMenu
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                                              0,  // Dialog box
// QDOAS ???                                    (DLGPROC) 0,  // Dialog procedure
// QDOAS ??? #endif
// QDOAS ???                   TREE_ITEM_TYPE_FILE_CHILDREN,  // Type of data for children
// QDOAS ???                                           NULL,  // Pointer to objects list
// QDOAS ???                                           NULL,  // pointer to the structure storing the data to paste
// QDOAS ???                                              0,  // Number of items in previous list
// QDOAS ???                                              0,  // Maximum number of items authorized in previous list
// QDOAS ???                                              0,  // Object size in bytes
// QDOAS ???                              FILE_TYPE_SPECTRA   // Type of files to insert
// QDOAS ???    },
// QDOAS ???
// QDOAS ???    // TREE_ITEM_TYPE_FILE_CHILDREN
// QDOAS ???    // ----------------------------
// QDOAS ???    {
// QDOAS ???                           MENU_CONTEXT_SPECTRA,  // ContextMenu
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                                  DLG_FILES_RAW,  // Dialog box
// QDOAS ???                          (DLGPROC) RAW_WndProc,  // Dialog procedure
// QDOAS ??? #endif
// QDOAS ???                            TREE_ITEM_TYPE_NONE,  // Type of data for children
// QDOAS ???                                           NULL,  // Pointer to objects list
// QDOAS ???                                           NULL,  // pointer to the structure storing the data to paste
// QDOAS ???                                              0,  // Number of items in previous list
// QDOAS ???                                              0,  // Maximum number of items authorized in previous list
// QDOAS ???                              sizeof(RAW_FILES),  // Object size in bytes
// QDOAS ???                                              0   // type of file for children
// QDOAS ???    },
// QDOAS ???
// QDOAS ???    // TREE_ITEM_TYPE_ANALYSIS_PARENT
// QDOAS ???    // ------------------------------
// QDOAS ???    {
// QDOAS ???                            MENU_CONTEXT_INSERT,  // ContextMenu
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                                              0,  // Dialog box
// QDOAS ???                                    (DLGPROC) 0,  // Dialog procedure
// QDOAS ??? #endif
// QDOAS ???               TREE_ITEM_TYPE_ANALYSIS_CHILDREN,  // Type of data for children
// QDOAS ???                                           NULL,  // Pointer to objects list
// QDOAS ???                                           NULL,  // pointer to the structure storing the data to paste
// QDOAS ???                                              0,  // Number of items in previous list
// QDOAS ???                                              0,  // Maximum number of items authorized in previous list
// QDOAS ???                                              0,  // Object size in bytes
// QDOAS ???                                              0   // type of file for children
// QDOAS ???    },
// QDOAS ???
// QDOAS ???    // TREE_ITEM_TYPE_ANALYSIS_CHILDREN
// QDOAS ???    // --------------------------------
// QDOAS ???    {
// QDOAS ???                             MENU_CONTEXT_CROSS,  // ContextMenu
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???                                      DLG_ANLYS,  // Dialog box
// QDOAS ???                        (DLGPROC) ANLYS_WndProc,  // Dialog procedure
// QDOAS ??? #endif
// QDOAS ???                            TREE_ITEM_TYPE_NONE,  // Type of data for children
// QDOAS ???                                           NULL,  // Pointer to objects list
// QDOAS ???                                           NULL,  // pointer to the structure storing the data to paste
// QDOAS ???                                              0,  // Number of items in previous list
// QDOAS ???                                              0,  // Maximum number of items authorized in previous list
// QDOAS ???                       sizeof(ANALYSIS_WINDOWS),  // Object size in bytes
// QDOAS ???                                              0   // type of file for children
// QDOAS ???    }
// QDOAS ???  };

// QDOAS ??? // ===============
// QDOAS ??? // TREE PROCESSING
// QDOAS ??? // ===============
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_GetProjectParent
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       search for the parent project;
// QDOAS ??? //
// QDOAS ??? // INPUT         indexItem : index in the TREE of the child node;
// QDOAS ??? //
// QDOAS ??? // RETURN        index of the parent project in the PROJECTS list if found;
// QDOAS ??? //               ITEM_NONE otherwise.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INDEX TREE_GetProjectParent(INDEX indexItem)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexProject;    // browse parent nodes and stop on the project one
// QDOAS ???
// QDOAS ???   // Search for the project index in the parent nodes
// QDOAS ???
// QDOAS ???   for (indexProject=indexItem;indexProject!=ITEM_NONE;indexProject=TREE_itemList[indexProject].parentItem)
// QDOAS ???    if (TREE_itemList[indexProject].dataType==TREE_ITEM_TYPE_PROJECT)
// QDOAS ???     break;
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return (indexProject!=ITEM_NONE)?TREE_itemList[indexProject].dataIndex:ITEM_NONE;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TreeUpdateDataIndex
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       when deleting an item in the tree, update data indexes of all
// QDOAS ??? //               nodes of the same type as the one to delete;
// QDOAS ??? //
// QDOAS ??? // INPUT         indexParent : index of a node in the tree;
// QDOAS ??? //               dataType    : type of the node to delete;
// QDOAS ??? //               dataIndex   : index in its data list of the node to delete.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TreeUpdateDataIndex(INDEX indexParent,INT dataType,INDEX dataIndex)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexItem;   // index for browsing child nodes
// QDOAS ???
// QDOAS ???   // Browse child items and decrease data index if needed
// QDOAS ???
// QDOAS ???   for (indexItem=TREE_itemList[indexParent].firstChildItem;
// QDOAS ???        indexItem!=ITEM_NONE;
// QDOAS ???        indexItem=TREE_itemList[indexItem].nextItem)
// QDOAS ???    {
// QDOAS ???     // Browse child items lists recursively
// QDOAS ???
// QDOAS ???     if (TREE_itemList[indexItem].firstChildItem!=ITEM_NONE)
// QDOAS ???      TreeUpdateDataIndex(indexItem,dataType,dataIndex);
// QDOAS ???
// QDOAS ???     // Current child node processing
// QDOAS ???
// QDOAS ???     else if ((TREE_itemList[indexItem].dataType==dataType) &&
// QDOAS ???              (TREE_itemList[indexItem].dataIndex>dataIndex))
// QDOAS ???      TREE_itemList[indexItem].dataIndex--;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Parent node processing
// QDOAS ???
// QDOAS ???   if ((TREE_itemList[indexParent].dataType==dataType) &&
// QDOAS ???       (TREE_itemList[indexParent].dataIndex>dataIndex))
// QDOAS ???    TREE_itemList[indexParent].dataIndex--;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TreeResetData
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       reset the data description of a tree node to delete
// QDOAS ??? //
// QDOAS ??? // INPUT         indexItem : index of the node to delete;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TreeResetData(INDEX indexItem)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   TREE_ITEM *pTree;         // pointer to the tree node to delete
// QDOAS ???   UCHAR     *pDataList;     // list of objects of the same type as the node to delete
// QDOAS ???   INT        dataNumber,    // number of objects in the previous list
// QDOAS ???              dataSize,      // size in bytes of an object in the previous list
// QDOAS ???              dataType;      // type of the node to delete
// QDOAS ???   INDEX      dataIndex,     // index of the object in the data list
// QDOAS ???              indexParent;   // index in the tree of the parent of the node to delete
// QDOAS ???
// QDOAS ???   if (indexItem!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     // Initializations
// QDOAS ???
// QDOAS ???     pTree=&TREE_itemList[indexItem];                                                         // pointer to the tree node to delete
// QDOAS ???
// QDOAS ???     if (((dataIndex=pTree->dataIndex)!=ITEM_NONE) &&                                         // index of the object in the data list
// QDOAS ???         ((pDataList=(UCHAR *)TREE_itemType[(dataType=pTree->dataType)].dataList)!=NULL) &&   // list of objects of the same type as the node to delete
// QDOAS ???         ((dataNumber=TREE_itemType[dataType].dataNumber)>dataIndex) &&                       // number of objects in the previous list
// QDOAS ???         ((dataSize=TREE_itemType[dataType].dataSize)>0))                                     // size in bytes of an object in the previous list
// QDOAS ???      {
// QDOAS ???       // Break connexions between project and workspace environment trees
// QDOAS ???
// QDOAS ???       if ((dataType==TREE_ITEM_TYPE_ANALYSIS_CHILDREN) && !pTree->newItem)                   // Release symbols used by analysis windows
// QDOAS ???        ANLYS_ReleaseListItems(dataIndex);
// QDOAS ???       else if (dataType==TREE_ITEM_TYPE_PROJECT)                                             // Release observation sites used in projects properties
// QDOAS ???        PRJCT_ReleaseSite(dataIndex);
// QDOAS ???
// QDOAS ???       // Delete data referenced by item in list by shifting all following data in list
// QDOAS ???
// QDOAS ???       if (dataIndex<dataNumber-1)
// QDOAS ???        memcpy(&pDataList[dataIndex*dataSize],
// QDOAS ???               &pDataList[(dataIndex+1)*dataSize],
// QDOAS ???                dataSize*(dataNumber-dataIndex-1));
// QDOAS ???
// QDOAS ???       // Reset the last element in list
// QDOAS ???
// QDOAS ???       memset(&pDataList[(dataNumber-1)*dataSize],0,dataSize);
// QDOAS ???
// QDOAS ???       // Go to root and update data indexes used by items of the same type as the one to delete
// QDOAS ???
// QDOAS ???       for (indexParent=pTree->parentItem;indexParent!=ITEM_NONE;indexParent=TREE_itemList[indexParent].parentItem)
// QDOAS ???        if (TREE_itemList[indexParent].parentItem==ITEM_NONE)
// QDOAS ???         break;
// QDOAS ???
// QDOAS ???       if (indexParent!=ITEM_NONE)
// QDOAS ???        for (indexItem=TREE_itemList[indexParent].firstChildItem;
// QDOAS ???             indexItem!=ITEM_NONE;
// QDOAS ???             indexItem=TREE_itemList[indexItem].nextItem)
// QDOAS ???         TreeUpdateDataIndex(indexItem,dataType,dataIndex);
// QDOAS ???
// QDOAS ???       // Reinitialize item structure
// QDOAS ???
// QDOAS ???       TREE_itemType[dataType].dataNumber--;
// QDOAS ???
// QDOAS ???       pTree->dataType=TREE_ITEM_TYPE_NONE;      // type of data referenced by item
// QDOAS ???       pTree->dataIndex=ITEM_NONE;               // index of data in data list
// QDOAS ???       pTree->useCount=0;                        // number of times item is referenced
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     memset(pTree->textItem,0,MAX_ITEM_DESC_LEN+1);
// QDOAS ???
// QDOAS ???     pTree->parentItem=ITEM_NONE;                // index of parent item in list
// QDOAS ???     pTree->hti=(HTREEITEM)0;                    // handle of the item
// QDOAS ???     pTree->hidden=0;                            // item is no more hidden
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_DeleteOneItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       delete a tree item
// QDOAS ??? //
// QDOAS ??? // INPUT         indexItemToDelete : index of the node to delete;
// QDOAS ??? //
// QDOAS ??? // RETURN        index in the tree of the parent node
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INDEX TREE_DeleteOneItem(INDEX indexItemToDelete)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   TREE_ITEM *pParent,             // pointer to the parent node
// QDOAS ???             *pPrevItem,           // pointer to the previous node
// QDOAS ???             *pNextItem,           // pointer to the next node
// QDOAS ???             *pFreeItem,           // pointer to the first free node
// QDOAS ???             *pItem;               // pointer to the current node to delete
// QDOAS ???
// QDOAS ???   INDEX indexParent;              // index in the tree structure of the parent node
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   indexParent=ITEM_NONE;
// QDOAS ???
// QDOAS ???   if (indexItemToDelete!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???     if (!TLBAR_bSaveFlag)
// QDOAS ???      TLBAR_Enable(TRUE);
// QDOAS ???     #endif
// QDOAS ???
// QDOAS ???     // Pointers initialization
// QDOAS ???
// QDOAS ???     pItem=&TREE_itemList[indexItemToDelete];                                        // pointer to the item to delete
// QDOAS ???     indexParent=pItem->parentItem;                                                  // index of the parent node
// QDOAS ???
// QDOAS ???     pParent=(pItem->parentItem!=ITEM_NONE)?&TREE_itemList[indexParent]:NULL;        // parent item in the tree
// QDOAS ???     pPrevItem=(pItem->prevItem!=ITEM_NONE)?&TREE_itemList[pItem->prevItem]:NULL;    // previous item in the tree
// QDOAS ???     pNextItem=(pItem->nextItem!=ITEM_NONE)?&TREE_itemList[pItem->nextItem]:NULL;    // next item in the tree
// QDOAS ???     pFreeItem=(treeItemFree!=ITEM_NONE)?&TREE_itemList[treeItemFree]:NULL;          // first free node in the tree
// QDOAS ???
// QDOAS ???     // Delete child items
// QDOAS ???
// QDOAS ???     TREE_DeleteChildList(indexItemToDelete);
// QDOAS ???
// QDOAS ???     // Reinitialize indexes
// QDOAS ???
// QDOAS ???     if (pNextItem) pNextItem->prevItem=pItem->prevItem;   // Next item points to the previous item
// QDOAS ???     if (pPrevItem) pPrevItem->nextItem=pItem->nextItem;   // Previous item points to the next item
// QDOAS ???
// QDOAS ???     if (pParent)
// QDOAS ???      {
// QDOAS ???       // item is hidden
// QDOAS ???
// QDOAS ???       if (pItem->hidden)
// QDOAS ???        pParent->childHidden--;
// QDOAS ???
// QDOAS ???       // item to delete is the first one of child list
// QDOAS ???
// QDOAS ???       if (pParent->firstChildItem==indexItemToDelete)
// QDOAS ???        pParent->firstChildItem=pItem->nextItem;
// QDOAS ???
// QDOAS ???       // item to delete is the last one of child list
// QDOAS ???
// QDOAS ???       if (pParent->lastChildItem==indexItemToDelete)
// QDOAS ???        pParent->lastChildItem=pItem->prevItem;
// QDOAS ???
// QDOAS ???       // update number of children
// QDOAS ???
// QDOAS ???       pParent->childNumber--;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Update path for file names
// QDOAS ???
// QDOAS ???     if ((pItem->dataType==TREE_ITEM_TYPE_FILE_CHILDREN) && (pItem->dataIndex!=ITEM_NONE))
// QDOAS ???      {
// QDOAS ???       RAW_FILES *pRaw=&RAW_spectraFiles[pItem->dataIndex];
// QDOAS ???
// QDOAS ???       FILES_RemoveOnePath(pRaw->fileName);
// QDOAS ???       FILES_RemoveOnePath(pRaw->dark);
// QDOAS ???       FILES_RemoveOnePath(pRaw->names);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Update project index for file names
// QDOAS ???
// QDOAS ???     else if ((pItem->dataType==TREE_ITEM_TYPE_PROJECT) && (pItem->dataIndex!=ITEM_NONE))
// QDOAS ???      {
// QDOAS ???       INDEX indexRaw;
// QDOAS ???       INT nRaw;
// QDOAS ???
// QDOAS ???       nRaw=TREE_itemType[TREE_ITEM_TYPE_FILE_CHILDREN].dataNumber;
// QDOAS ???
// QDOAS ???       for (indexRaw=0;indexRaw<nRaw;indexRaw++)
// QDOAS ???        if (RAW_spectraFiles[indexRaw].indexProject>pItem->dataIndex)
// QDOAS ???         RAW_spectraFiles[indexRaw].indexProject--;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Reset item and put it into free list
// QDOAS ???
// QDOAS ???     TreeResetData(indexItemToDelete);
// QDOAS ???
// QDOAS ???     if (pFreeItem)
// QDOAS ???      pFreeItem->prevItem=indexItemToDelete;
// QDOAS ???
// QDOAS ???     pItem->nextItem=treeItemFree;
// QDOAS ???     pItem->prevItem=ITEM_NONE;
// QDOAS ???
// QDOAS ???     treeItemFree=indexItemToDelete;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return the index in the tree of the parent node
// QDOAS ???
// QDOAS ???   return indexParent;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_CollapseChildNodes
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       collapse the list of child nodes and free memory allocated for these items
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree    : handle of the current tree
// QDOAS ??? //               indexParent : index in the tree of the parent node
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? void TREE_CollapseChildNodes(HWND hwndTree,INDEX indexParent)
// QDOAS ???  {
// QDOAS ???   TREE_Reset(indexParent);                           // Invalidate handles of child items
// QDOAS ???   TreeView_Expand(hwndTree,                          // Handle of the TreeView control
// QDOAS ???                   TREE_itemList[indexParent].hti,    // Handle of the parent item
// QDOAS ???                   TVE_COLLAPSE|TVE_COLLAPSERESET);   // Action
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_ExpandOneNode
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       expand the list of child nodes
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree    : handle of the current tree;
// QDOAS ??? //               indexParent : index in the tree of the parent node;
// QDOAS ??? //               hParent     : handle of the parent node.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_ExpandOneNode(HWND hwndTree,INDEX indexParent,HTREEITEM hParent)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   TV_INSERTSTRUCT tvins;  // description of the new items to insert 'physically' in the tree
// QDOAS ???   TV_ITEM tvi;            // attributes of the item to insert
// QDOAS ???   TREE_ITEM *pTree;       // pointer to the item to insert
// QDOAS ???   INDEX indexItem;        // index in the tree of the item to insert
// QDOAS ???
// QDOAS ???   if ((hwndTree!=NULL) && (indexParent!=ITEM_NONE) && !TREE_itemList[indexParent].hidden)
// QDOAS ???    {
// QDOAS ???     // give the fields to update
// QDOAS ???
// QDOAS ???     tvi.mask = TVIF_TEXT|TVIF_STATE|TVIF_PARAM|TVIF_CHILDREN|TVIF_SELECTEDIMAGE|TVIF_IMAGE;
// QDOAS ???
// QDOAS ???     // browse childs list
// QDOAS ???
// QDOAS ???     for (indexItem=TREE_itemList[indexParent].firstChildItem;indexItem!=ITEM_NONE;indexItem=pTree->nextItem)
// QDOAS ???      {
// QDOAS ???       pTree=&TREE_itemList[indexItem];
// QDOAS ???
// QDOAS ???       // Define the attributes of the current item to insert
// QDOAS ???
// QDOAS ???       if (!pTree->hidden && (pTree->hti==(HTREEITEM)0))
// QDOAS ???        {
// QDOAS ???         tvi.pszText = LPSTR_TEXTCALLBACK;   // the parent window is responsible for storing the name
// QDOAS ???         tvi.state = ((hParent==TVI_ROOT)||RAW_GetFolderFlag(indexItem))?TVIS_BOLD:0;
// QDOAS ???         tvi.stateMask=-1;
// QDOAS ???         tvi.lParam = (LPARAM)indexItem;
// QDOAS ???         tvi.cChildren=(pTree->childNumber-pTree->childHidden>0)?1:0;
// QDOAS ???
// QDOAS ???         // for folders, use image list
// QDOAS ???
// QDOAS ???         if (RAW_GetFolderFlag(indexItem))
// QDOAS ???          {
// QDOAS ???           tvi.iImage          = TREE_closeImageIndex;
// QDOAS ???           tvi.iSelectedImage  = TREE_openImageIndex;
// QDOAS ???          }
// QDOAS ???         else
// QDOAS ???          tvi.iImage=tvi.iSelectedImage=TREE_blankImageIndex;
// QDOAS ???
// QDOAS ???         // extended description of the item to insert
// QDOAS ???
// QDOAS ???         tvins.item = tvi;
// QDOAS ???
// QDOAS ???         tvins.hInsertAfter = (pTree->prevItem!=ITEM_NONE) ?
// QDOAS ???                               TREE_itemList[pTree->prevItem].hti :
// QDOAS ???                               TVI_FIRST;
// QDOAS ???
// QDOAS ???         tvins.hParent = hParent;
// QDOAS ???
// QDOAS ???         // insert the new item
// QDOAS ???
// QDOAS ???         pTree->hti =
// QDOAS ???
// QDOAS ???          (HTREEITEM) SendMessage(hwndTree,TVM_INSERTITEM,0,
// QDOAS ???                         (LPARAM)(LPTV_INSERTSTRUCT)&tvins);
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif // __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_SortChildNodes
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       Sort child items of a tree node
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree    : handle of the current tree
// QDOAS ??? //               indexParent : index of the parent node
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_SortChildNodes(HWND hwndTree,INDEX indexParent)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX      indexItem,       // browse child nodes
// QDOAS ???              indexFirst,      // index of the first child node in the UNsorted list
// QDOAS ???              indexLast,       // index of the last child node in the UNsorted list
// QDOAS ???              indexOld,        // index of the last swapped child node
// QDOAS ???              indexPrevious,   // index of the previous child node when browsing the list of unsorted child nodes
// QDOAS ???              indexTmp,        // when swapping child nodes, swap also the indexes to the data descriptions
// QDOAS ???              indexChild;      // browse child nodes of the swapped nodes
// QDOAS ???
// QDOAS ???   TREE_ITEM *pTreeParent,     // pointer to the parent node
// QDOAS ???             *pTreeItem,       // pointer to the current child node
// QDOAS ???              treeTmp;         // buffer used when swapping child nodes
// QDOAS ???
// QDOAS ???   UCHAR     *dataList,        // list of objects child nodes belong to
// QDOAS ???             *dataTmp;         // temporary buffer to receive the description of data when swapping child nodes
// QDOAS ???
// QDOAS ???   INT        dataType,        // data type of child nodes
// QDOAS ???              dataSize,        // size in bytes needed for the data description of child nodes
// QDOAS ???              swapFlag;        // flag set if the two current child nodes are to be swapped
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   dataTmp=NULL;
// QDOAS ???
// QDOAS ???   if ((indexParent!=ITEM_NONE) && !TREE_itemList[indexParent].hidden)
// QDOAS ???    {
// QDOAS ???     // Sort recursively child nodes
// QDOAS ???
// QDOAS ???     for (indexItem=TREE_itemList[indexParent].firstChildItem;indexItem!=ITEM_NONE;indexItem=TREE_itemList[indexItem].nextItem)
// QDOAS ???      TREE_SortChildNodes(hwndTree,indexItem);
// QDOAS ???
// QDOAS ???     // Get index of item that has the selection
// QDOAS ???
// QDOAS ???     if (((dataType=(RAW_GetFolderFlag(indexParent)!=RAW_FOLDER_TYPE_NONE)?TREE_ITEM_TYPE_FILE_PARENT:
// QDOAS ???                                                                           TREE_itemList[indexParent].dataType)!=ITEM_NONE) &&
// QDOAS ???         ((dataType=TREE_itemType[dataType].childDataType)!=ITEM_NONE) &&                                                                               // data type of child nodes
// QDOAS ???         ((dataList=(UCHAR *)TREE_itemType[dataType].dataList)!=NULL) &&                                                                                // list of objects child nodes belong to
// QDOAS ???         ((dataSize=TREE_itemType[dataType].dataSize)!=0) &&                                                                                            // size in bytes needed for the data description of child nodes
// QDOAS ???         ((dataTmp=(UCHAR *)MEMORY_AllocBuffer("TREE_SortChildNodes ","dataTmp",dataSize,1,0,MEMORY_TYPE_STRING))!=NULL) &&                                                  // temporary buffer to receive the description of data when swapping child nodes
// QDOAS ???         ((indexLast=TREE_itemList[indexParent].lastChildItem)!=ITEM_NONE))                                                                             // there are children nodes
// QDOAS ???      {
// QDOAS ???       // While the list of unsorted items is not empty ...
// QDOAS ???
// QDOAS ???       for (pTreeParent=&TREE_itemList[indexParent],indexFirst=pTreeParent->firstChildItem;
// QDOAS ???           (indexFirst!=indexLast);indexFirst=indexOld)
// QDOAS ???
// QDOAS ???        // ... browse unsorted items
// QDOAS ???
// QDOAS ???        for (indexItem=indexOld=indexLast;(indexItem!=indexFirst);indexItem=indexPrevious)
// QDOAS ???         {
// QDOAS ???          pTreeItem=&TREE_itemList[indexItem];
// QDOAS ???          indexPrevious=pTreeItem->prevItem;
// QDOAS ???          swapFlag=0;
// QDOAS ???
// QDOAS ???          // Define search criterion
// QDOAS ???
// QDOAS ???          if ((pTreeItem->dataIndex!=ITEM_NONE) && !pTreeItem->hidden &&
// QDOAS ???              (TREE_itemList[indexPrevious].dataIndex!=ITEM_NONE) && !TREE_itemList[indexPrevious].hidden)
// QDOAS ???
// QDOAS ???           swapFlag=(((STD_Stricmp(pTreeItem->textItem,TREE_itemList[indexPrevious].textItem)<0) && // !!! starts at the end of the list, so swap if s1<s2
// QDOAS ???                  (((((RAW_GetFolderFlag(indexItem))?1:0)+((RAW_GetFolderFlag(indexPrevious))?1:0))%2)==0)) ||
// QDOAS ???                      (RAW_GetFolderFlag(indexItem) && !RAW_GetFolderFlag(indexPrevious)))?1:0;
// QDOAS ???
// QDOAS ???          if (swapFlag)
// QDOAS ???           {
// QDOAS ???            // Update parentItem field in child nodes
// QDOAS ???
// QDOAS ???            for (indexChild=pTreeItem->firstChildItem;indexChild!=ITEM_NONE;indexChild=TREE_itemList[indexChild].nextItem)
// QDOAS ???             TREE_itemList[indexChild].parentItem=indexPrevious;
// QDOAS ???            for (indexChild=TREE_itemList[indexPrevious].firstChildItem;indexChild!=ITEM_NONE;indexChild=TREE_itemList[indexChild].nextItem)
// QDOAS ???             TREE_itemList[indexChild].parentItem=indexItem;
// QDOAS ???
// QDOAS ???            // Swap data descriptions
// QDOAS ???
// QDOAS ???            memcpy(dataTmp,&dataList[dataSize*pTreeItem->dataIndex],dataSize);
// QDOAS ???            memcpy(&dataList[dataSize*pTreeItem->dataIndex],&dataList[dataSize*TREE_itemList[indexPrevious].dataIndex],dataSize);
// QDOAS ???            memcpy(&dataList[dataSize*TREE_itemList[indexPrevious].dataIndex],dataTmp,dataSize);
// QDOAS ???
// QDOAS ???            // Swap tree nodes ...
// QDOAS ???
// QDOAS ???            memcpy(&treeTmp,pTreeItem,sizeof(TREE_ITEM));
// QDOAS ???            memcpy(pTreeItem,&TREE_itemList[indexPrevious],sizeof(TREE_ITEM));
// QDOAS ???            memcpy(&TREE_itemList[indexPrevious],&treeTmp,sizeof(TREE_ITEM));
// QDOAS ???
// QDOAS ???            // ... except pointers ...
// QDOAS ???
// QDOAS ???            TREE_itemList[indexPrevious].prevItem=pTreeItem->prevItem;
// QDOAS ???            pTreeItem->prevItem=indexPrevious;
// QDOAS ???            pTreeItem->nextItem=TREE_itemList[indexPrevious].nextItem;
// QDOAS ???            TREE_itemList[indexPrevious].nextItem=indexItem;
// QDOAS ???
// QDOAS ???            // ... and swap also data indexes
// QDOAS ???
// QDOAS ???            indexTmp=pTreeItem->dataIndex;
// QDOAS ???            pTreeItem->dataIndex=TREE_itemList[indexPrevious].dataIndex;
// QDOAS ???            TREE_itemList[indexPrevious].dataIndex=indexTmp;
// QDOAS ???
// QDOAS ???            indexOld=indexItem;
// QDOAS ???           }
// QDOAS ???         }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Release allocated buffer
// QDOAS ???
// QDOAS ???   if (dataTmp!=NULL)
// QDOAS ???    MEMORY_ReleaseBuffer("TREE_SortChildNodes ","dataTmp",dataTmp);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TreeShow
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       recursive function used to show all hidden child nodes
// QDOAS ??? //
// QDOAS ??? // INPUT         indexItem : index of the parent node
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TreeShow(INDEX indexItem)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX      indexChild;   // browse child nodes
// QDOAS ???   TREE_ITEM *pTree,        // pointer to the parent node
// QDOAS ???             *pChild;       // pointer to the child node
// QDOAS ???
// QDOAS ???   if (indexItem!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     pTree=&TREE_itemList[indexItem];
// QDOAS ???
// QDOAS ???     // Search for hidden child nodes
// QDOAS ???
// QDOAS ???     for (indexChild=pTree->firstChildItem;(indexChild!=ITEM_NONE);
// QDOAS ???          indexChild=TREE_itemList[indexChild].nextItem)
// QDOAS ???      {
// QDOAS ???       pChild=&TREE_itemList[indexChild];
// QDOAS ???
// QDOAS ???       if (pChild->hidden==2)   // user-defined hide command
// QDOAS ???        {
// QDOAS ???         pChild->hidden=0;
// QDOAS ???         pTree->childHidden--;
// QDOAS ???
// QDOAS ???         if (pChild->dataIndex!=ITEM_NONE)
// QDOAS ???          {
// QDOAS ???           if (pChild->dataType==TREE_ITEM_TYPE_ANALYSIS_CHILDREN)
// QDOAS ???            ANLYS_windowsList[pChild->dataIndex].hidden=pChild->hidden;
// QDOAS ???           else if (pChild->dataType==TREE_ITEM_TYPE_FILE_CHILDREN)
// QDOAS ???            RAW_spectraFiles[pChild->dataIndex].hidden=pChild->hidden;
// QDOAS ???           else if (pChild->dataType==TREE_ITEM_TYPE_SITE_CHILDREN)
// QDOAS ???            SITES_itemList[pChild->dataIndex].hidden=pChild->hidden;
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Recursive function
// QDOAS ???
// QDOAS ???       TreeShow(indexChild);
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TreeCreateImageList
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       create an image list to use with the 'Projects' tree
// QDOAS ??? //
// QDOAS ??? // RETURN        handle to the image list
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? HIMAGELIST TreeCreateImageList(void)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   HIMAGELIST hIL;       // Handle to the image list
// QDOAS ???   HBITMAP    hBitmap;   // Handle to a temporary bitmap
// QDOAS ???
// QDOAS ???   // Create the Image List
// QDOAS ???
// QDOAS ???   hIL = ImageList_Create(16,     // X size of one image
// QDOAS ???                          13,     // Y size of one image
// QDOAS ???                          TRUE,   // Masked images
// QDOAS ???                          3,      // Three images in this list
// QDOAS ???                          0);     // No "growing" bitmaps in this list.
// QDOAS ???
// QDOAS ???   if (!hIL) return NULL;
// QDOAS ???
// QDOAS ???   // Add Each bitmap to the ImageList.
// QDOAS ???   //
// QDOAS ???   // ImageList_AddMasked will add the bitmap, and treat every
// QDOAS ???   // pixel that is (in this example) white as a "transparent" pixel,
// QDOAS ???   // since we specified TRUE for fMask in the above call to
// QDOAS ???   // ImageList_Create.
// QDOAS ???
// QDOAS ???   if ((hBitmap=LoadBitmap(DOAS_hInst,MAKEINTRESOURCE(BLANK_BITMAP)))!=NULL)
// QDOAS ???    {
// QDOAS ???     TREE_blankImageIndex=ImageList_AddMasked(hIL,hBitmap,RGB(255,255,255));
// QDOAS ???     DeleteObject (hBitmap);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if ((hBitmap=LoadBitmap(DOAS_hInst,MAKEINTRESOURCE(OPEN_FOLDER)))!=NULL)
// QDOAS ???    {
// QDOAS ???     TREE_openImageIndex=ImageList_AddMasked(hIL,hBitmap,RGB(255,255,255));
// QDOAS ???     DeleteObject(hBitmap);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if ((hBitmap=LoadBitmap(DOAS_hInst,MAKEINTRESOURCE(CLOSE_FOLDER)))!=NULL)
// QDOAS ???    {
// QDOAS ???     TREE_closeImageIndex=ImageList_AddMasked(hIL,hBitmap,RGB(255,255,255));
// QDOAS ???     DeleteObject (hBitmap);
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Paranoid checking to make sure everything was added successfully.
// QDOAS ???
// QDOAS ???   if (ImageList_GetImageCount(hIL)<2)
// QDOAS ???    {
// QDOAS ???     ImageList_Destroy(hIL);
// QDOAS ???     hIL = NULL;
// QDOAS ???     TREE_blankImageIndex=
// QDOAS ???     TREE_openImageIndex=
// QDOAS ???     TREE_closeImageIndex=ITEM_NONE;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Success!
// QDOAS ???
// QDOAS ???   return hIL;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_GetIndexByDataName
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       get the index of a tree node from a name and a data type
// QDOAS ??? //
// QDOAS ??? // INPUT         dataName   : the name of the tree node to search for;
// QDOAS ??? //               dataType   : the type of the tree node to search for;
// QDOAS ??? //               entryPoint : the entry point in the tree from which to start the search;
// QDOAS ??? //
// QDOAS ??? // RETURN        index of the node in the tree if found; ITEM_NONE otherwise.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INDEX TREE_GetIndexByDataName(UCHAR *dataName,UCHAR dataType,INDEX entryPoint)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   TREE_ITEM_TYPE *pTreeItemType;   // pointer to the type description of the tree node to search for
// QDOAS ???   TREE_ITEM      *pTreeItem;       // pointer to the current tree node
// QDOAS ???   INDEX           indexItem,       // browse child nodes
// QDOAS ???                   indexTree;       // index of the tree node if found
// QDOAS ???
// QDOAS ???   UCHAR          *pDataList;       // list of objects of the same type as the input 'dataType'
// QDOAS ???   SZ_LEN          dataNameLen;     // length of the input 'dataName'
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pTreeItemType=&TREE_itemType[dataType];
// QDOAS ???   pDataList=(UCHAR *)pTreeItemType->dataList;
// QDOAS ???   dataNameLen=strlen(dataName);
// QDOAS ???   indexTree=ITEM_NONE;
// QDOAS ???
// QDOAS ???   // Browse recursively child nodes
// QDOAS ???
// QDOAS ???   if (entryPoint!=ITEM_NONE)
// QDOAS ???
// QDOAS ???    for (indexItem=TREE_itemList[entryPoint].firstChildItem;
// QDOAS ???        (indexItem!=ITEM_NONE) && (indexTree==ITEM_NONE);
// QDOAS ???         indexItem=pTreeItem->nextItem)
// QDOAS ???     {
// QDOAS ???      pTreeItem=&TREE_itemList[indexItem];                       // pointer to the current tree node
// QDOAS ???                                                                 // index of the tree node if found
// QDOAS ???      indexTree=
// QDOAS ???
// QDOAS ???         ((pTreeItem->dataType==dataType) &&                     // check data type
// QDOAS ???
// QDOAS ???         (((pTreeItem->dataIndex==ITEM_NONE) &&                  // check data name
// QDOAS ???           (strlen(pTreeItem->textItem)==dataNameLen) &&
// QDOAS ???           !STD_Stricmp(pTreeItem->textItem,dataName)) ||
// QDOAS ???
// QDOAS ???          ((pTreeItem->dataIndex!=ITEM_NONE) &&
// QDOAS ???           (pTreeItemType->dataNumber>pTreeItem->dataIndex) &&
// QDOAS ???           (pDataList!=NULL) &&
// QDOAS ???           (strlen(&pDataList[pTreeItemType->dataSize*pTreeItem->dataIndex])==dataNameLen) &&
// QDOAS ???           !STD_Stricmp(&pDataList[pTreeItemType->dataSize*pTreeItem->dataIndex],dataName)))) ?
// QDOAS ???
// QDOAS ???         indexItem:TREE_GetIndexByDataName(dataName,dataType,indexItem);
// QDOAS ???     }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return indexTree;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_GetIndexByDataIndex
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       get the index of a tree node from its data description
// QDOAS ??? //
// QDOAS ??? // INPUT         dataIndex  : index of the object in the list it belongs to;
// QDOAS ??? //               dataType   : the type of the tree node to search for;
// QDOAS ??? //               entryPoint : the entry point in the tree from which to start the search;
// QDOAS ??? //
// QDOAS ??? // RETURN        index of the node in the tree if found; ITEM_NONE otherwise.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INDEX TREE_GetIndexByDataIndex(INDEX dataIndex,UCHAR dataType,INDEX entryPoint)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexItem,                 // browse child nodes
// QDOAS ???         indexTree;                 // index of the tree node if found
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   indexTree=ITEM_NONE;
// QDOAS ???
// QDOAS ???   // Browse recursively child nodes
// QDOAS ???
// QDOAS ???   if ((dataType>TREE_ITEM_TYPE_NONE) && (dataType<TREE_ITEM_TYPE_MAX) &&
// QDOAS ???       (dataIndex>=0) && (dataIndex<TREE_itemType[dataType].dataNumber) &&
// QDOAS ???       (entryPoint!=ITEM_NONE))
// QDOAS ???
// QDOAS ???    for (indexItem=TREE_itemList[entryPoint].firstChildItem;
// QDOAS ???        (indexItem!=ITEM_NONE) && (indexTree==ITEM_NONE);
// QDOAS ???         indexItem=TREE_itemList[indexItem].nextItem)
// QDOAS ???
// QDOAS ???     indexTree=((TREE_itemList[indexItem].dataType==dataType)&&(TREE_itemList[indexItem].dataIndex==dataIndex))?
// QDOAS ???                 indexItem:TREE_GetIndexByDataIndex(dataIndex,dataType,indexItem);
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return indexTree;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_GetIndexesByDataType
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       get the list of indexes (in the objects list) of all nodes of the
// QDOAS ??? //               same data type as the one specified in argument
// QDOAS ??? //
// QDOAS ??? // INPUT         dataType    : the type of data to search for;
// QDOAS ??? //               entryPoint  : the entry point in the tree from which to start the search;
// QDOAS ??? //
// QDOAS ??? // OUTPUT        dataIndex   : the list of indexes of found objects;
// QDOAS ??? //               pDataNumber : the number of objects in the previous list;
// QDOAS ??? //
// QDOAS ??? // RETURN        the number of found objects.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INT TREE_GetIndexesByDataType(UCHAR dataType,INDEX entryPoint,INDEX *dataIndexes,INT *pDataNumber)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX      indexItem,            // browse child nodes
// QDOAS ???              dataIndex;            // index in its objects list of the current node
// QDOAS ???   TREE_ITEM *pTree;                // pointer to the current node
// QDOAS ???
// QDOAS ???   if ((dataType>TREE_ITEM_TYPE_NONE) && (dataType<TREE_ITEM_TYPE_MAX) && (entryPoint!=ITEM_NONE))
// QDOAS ???    {
// QDOAS ???     pTree=&TREE_itemList[entryPoint];
// QDOAS ???
// QDOAS ???     // Browse recursively child nodes
// QDOAS ???
// QDOAS ???     for (indexItem=pTree->firstChildItem;(indexItem!=ITEM_NONE);
// QDOAS ???          indexItem=TREE_itemList[indexItem].nextItem)
// QDOAS ???
// QDOAS ???      if (!TREE_itemList[indexItem].hidden)
// QDOAS ???       TREE_GetIndexesByDataType(dataType,indexItem,dataIndexes,pDataNumber);
// QDOAS ???
// QDOAS ???     if ((pTree->dataType==dataType) &&
// QDOAS ???        ((dataType!=TREE_ITEM_TYPE_FILE_CHILDREN) || (RAW_GetFolderFlag(entryPoint)!=RAW_FOLDER_TYPE_USER)) &&
// QDOAS ???        ((dataIndex=pTree->dataIndex)!=ITEM_NONE))
// QDOAS ???
// QDOAS ???      dataIndexes[(*pDataNumber)++]=dataIndex;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return *pDataNumber;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_UpdateItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       update the item attributes of a tree node
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree  : the handle of the current tree;
// QDOAS ??? //               indexItem : index in the tree of the item to update
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? void TREE_UpdateItem(HWND hwndTree,INDEX indexItem)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   TV_ITEM    tvi;       // structure with the new attributes of the item
// QDOAS ???   TREE_ITEM *pTree;     // pointer to the specified node in the tree
// QDOAS ???
// QDOAS ???   if ((hwndTree!=NULL) &&
// QDOAS ???       (indexItem!=ITEM_NONE) &&
// QDOAS ???       (TREE_itemList[indexItem].hti!=TVI_ROOT))
// QDOAS ???    {
// QDOAS ???     pTree=&TREE_itemList[indexItem];
// QDOAS ???
// QDOAS ???     // Initialize TreeView item structure
// QDOAS ???
// QDOAS ???     tvi.mask = TVIF_TEXT|TVIF_PARAM|TVIF_CHILDREN;
// QDOAS ???
// QDOAS ???     tvi.hItem=pTree->hti;                                         // handle in tree
// QDOAS ???     tvi.pszText = LPSTR_TEXTCALLBACK;                             // parent window is responsible of displaying text data
// QDOAS ???     tvi.lParam=indexItem;                                         // parameter field is used to store index of the item in list
// QDOAS ???     tvi.cChildren=(pTree->childNumber-pTree->childHidden>0)?1:0;  // set the number of children
// QDOAS ???
// QDOAS ???     // Send new parameters to tree control
// QDOAS ???
// QDOAS ???     TreeView_SetItem(hwndTree,(TV_ITEM FAR *)&tvi);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_Reset
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       recursive function to invalidate handles of all child items
// QDOAS ??? //
// QDOAS ??? // INPUT         indexParent : index in the tree of the parent node;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_Reset(INDEX indexParent)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexItem;  // Browse child nodes
// QDOAS ???
// QDOAS ???   if (indexParent!=ITEM_NONE)
// QDOAS ???    for (indexItem=TREE_itemList[indexParent].firstChildItem;
// QDOAS ???         indexItem!=ITEM_NONE;
// QDOAS ???         indexItem=TREE_itemList[indexItem].nextItem)
// QDOAS ???     {
// QDOAS ???      TREE_Reset(indexItem);
// QDOAS ???      TREE_itemList[indexItem].hti=(HTREEITEM)0;
// QDOAS ???     }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_GetSelectedItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       get the index in the tree of the selected node
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree : the handle of the current tree;
// QDOAS ??? //
// QDOAS ??? // RETURN        the index in the tree of the selected node.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? INDEX TREE_GetSelectedItem(HWND hwndTree)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   TV_ITEM tvi;          // Attributes of the selected item in the tree
// QDOAS ???   INDEX indexItem;      // index of the selected item
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   memset(&tvi,0,sizeof(TV_ITEM));
// QDOAS ???   indexItem=ITEM_NONE;
// QDOAS ???
// QDOAS ???   // Search for the item that has the selection
// QDOAS ???
// QDOAS ???   if ((hwndTree!=NULL) &&
// QDOAS ???      ((tvi.hItem=TreeView_GetSelection(hwndTree))!=NULL))
// QDOAS ???    {
// QDOAS ???     tvi.mask=TVIF_PARAM; // for retrieving index of item in list
// QDOAS ???
// QDOAS ???     if (TreeView_GetItem(hwndTree,&tvi))
// QDOAS ???      indexItem=(INDEX)tvi.lParam;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return indexItem;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_InsertOneItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       insert a new item in the tree
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree   : the handle of the tree in which to insert a new item;
// QDOAS ??? //               textItem   : the text of the new item to insert;
// QDOAS ??? //               parentItem : the index in the tree of the parent node;
// QDOAS ??? //               dataType   : the data type of the new item to insert;
// QDOAS ??? //               newItem    : flag set for loading properties dialog box after insertion;
// QDOAS ??? //               folderFlag : flag set for a folder (user-defined or directory);
// QDOAS ??? //               hidden     : flag set if item is hidden
// QDOAS ??? //
// QDOAS ??? // RETURN        the index of the new node
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? INDEX TREE_InsertOneItem(HWND   hwndTree,    // the handle of the tree in which to insert a new item
// QDOAS ???                          UCHAR *textItem,    // the text of the new item to insert
// QDOAS ???                          INDEX  parentItem,  // the index in the tree of the parent node
// QDOAS ???                          INT    dataType,    // the data type of the new item to insert
// QDOAS ???                          UCHAR  newItem,     // flag set for loading properties dialog box after insertion
// QDOAS ???                          UCHAR  folderFlag,  // flag set for a folder (user-defined or directory)
// QDOAS ???                          UCHAR  hidden)      // flag set if item is hidden
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR fileName[MAX_ITEM_TEXT_LEN+1];       // if the new item is a file/folder, complete file name
// QDOAS ???   TREE_ITEM *pTree,*pTreeParent;
// QDOAS ???   TREE_ITEM_TYPE *pTreeItemSpec;
// QDOAS ???   INDEX indexItem,indexData;
// QDOAS ???   UCHAR folderFlagParent;                    // folder specification for the parent node
// QDOAS ???   UCHAR *ptr,*ptr2;                          // pointer to parts of the file name
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   if (!STD_Stricmp(textItem,"Kurucz description"))
// QDOAS ???    strcpy(textItem,"Calibration description");
// QDOAS ???
// QDOAS ???   folderFlagParent=(UCHAR)(((parentItem!=ITEM_NONE) && (TREE_itemList[parentItem].dataType==TREE_ITEM_TYPE_FILE_CHILDREN))?
// QDOAS ???                              RAW_spectraFiles[TREE_itemList[parentItem].dataIndex].folderFlag:RAW_FOLDER_TYPE_NONE);
// QDOAS ???
// QDOAS ???   indexItem=ITEM_NONE;
// QDOAS ???
// QDOAS ???   // Text item (NB : for files, remove path)
// QDOAS ???
// QDOAS ???   FILES_RebuildFileName(fileName,textItem,1);
// QDOAS ???
// QDOAS ???   if ((dataType==TREE_ITEM_TYPE_FILE_CHILDREN) &&
// QDOAS ???       (folderFlag!=RAW_FOLDER_TYPE_USER) && (folderFlag!=RAW_FOLDER_TYPE_NONE) &&
// QDOAS ???       (folderFlagParent!=RAW_FOLDER_TYPE_PATHSUB))
// QDOAS ???    FILES_CompactPath(textItem,textItem,1,0);
// QDOAS ???
// QDOAS ???   if ((parentItem==ITEM_NONE) ||
// QDOAS ???       (dataType!=TREE_ITEM_TYPE_FILE_CHILDREN) ||
// QDOAS ???      ((ptr=strrchr(fileName,PATH_SEP))==NULL) ||
// QDOAS ???       !strlen(++ptr))
// QDOAS ???
// QDOAS ???    ptr=fileName;
// QDOAS ???
// QDOAS ???   if ((folderFlag==RAW_FOLDER_TYPE_PATH) || (folderFlag==RAW_FOLDER_TYPE_PATHSUB))
// QDOAS ???    {
// QDOAS ???     if ((ptr2=strrchr(fileName,PATH_SEP))!=NULL)
// QDOAS ???      *ptr2++=0;
// QDOAS ???     if ((ptr2!=NULL) && ((ptr=strrchr(fileName,PATH_SEP))!=NULL))
// QDOAS ???      ++ptr;
// QDOAS ???     if (ptr==NULL)
// QDOAS ???      ptr=fileName;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   if ((((indexData=TREE_GetIndexByDataName(textItem,(UCHAR)dataType,parentItem))==ITEM_NONE) ||  // there is no item in the tree with the same name
// QDOAS ???         (TREE_itemList[indexData].parentItem!=parentItem)) &&                                    // and the same parent as the new one
// QDOAS ???        ((indexItem=treeItemFree)!=ITEM_NONE))                                                    // it is possible to pick a free item
// QDOAS ???    {
// QDOAS ???     pTree=&TREE_itemList[indexItem];
// QDOAS ???
// QDOAS ???     // Update list of available items
// QDOAS ???
// QDOAS ???     treeItemFree=pTree->nextItem;                        // First item of the list
// QDOAS ???     TREE_itemList[treeItemFree].prevItem=ITEM_NONE;      // Second item of the list
// QDOAS ???
// QDOAS ???     // Fields initialization
// QDOAS ???
// QDOAS ???     pTree->newItem=newItem;
// QDOAS ???     pTree->firstChildItem=
// QDOAS ???     pTree->lastChildItem=
// QDOAS ???     pTree->prevItem=
// QDOAS ???     pTree->nextItem=ITEM_NONE;
// QDOAS ???     pTree->dataType=dataType;
// QDOAS ???     pTree->dataIndex=ITEM_NONE;
// QDOAS ???     #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???     pTree->hti=(HTREEITEM)0;
// QDOAS ???     #endif
// QDOAS ???     pTree->useCount=0;
// QDOAS ???     pTree->hidden=hidden;
// QDOAS ???
// QDOAS ???     memset(pTree->textItem,0,MAX_ITEM_DESC_LEN+1);
// QDOAS ???
// QDOAS ???     if ((folderFlag==RAW_FOLDER_TYPE_PATH) || (folderFlag==RAW_FOLDER_TYPE_PATHSUB))
// QDOAS ???      {
// QDOAS ???       UCHAR tmpBuffer[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???       sprintf(tmpBuffer,"%s (%s)",ptr,ptr2);
// QDOAS ???       strncpy(pTree->textItem,tmpBuffer,MAX_ITEM_DESC_LEN);
// QDOAS ???      }
// QDOAS ???     else
// QDOAS ???      strncpy(pTree->textItem,ptr,MAX_ITEM_DESC_LEN);
// QDOAS ???
// QDOAS ???     // Data allocation
// QDOAS ???
// QDOAS ???     if ((dataType>TREE_ITEM_TYPE_NONE) &&
// QDOAS ???         (dataType<TREE_ITEM_TYPE_MAX))
// QDOAS ???      {
// QDOAS ???       pTreeItemSpec=(TREE_ITEM_TYPE *)&TREE_itemType[dataType];
// QDOAS ???
// QDOAS ???       if ((pTreeItemSpec->dataList!=NULL) &&
// QDOAS ???           (pTreeItemSpec->dataNumber<pTreeItemSpec->dataMaxNumber))
// QDOAS ???        {
// QDOAS ???         UCHAR *pList=(UCHAR *)pTreeItemSpec->dataList;
// QDOAS ???         pTree->dataIndex=pTreeItemSpec->dataNumber++;
// QDOAS ???
// QDOAS ???         if ((dataType==TREE_ITEM_TYPE_FILE_CHILDREN) && (folderFlag!=RAW_FOLDER_TYPE_USER) &&
// QDOAS ???             (folderFlagParent!=RAW_FOLDER_TYPE_PATH) && (folderFlagParent!=RAW_FOLDER_TYPE_PATHSUB))
// QDOAS ???          FILES_CompactPath(textItem,textItem,1,1);
// QDOAS ???
// QDOAS ???         strncpy((UCHAR *)&pList[pTree->dataIndex*pTreeItemSpec->dataSize],
// QDOAS ???                 textItem,MAX_ITEM_DESC_LEN);
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Add item at the end of the child list
// QDOAS ???
// QDOAS ???     if ((pTree->parentItem=parentItem)!=ITEM_NONE)
// QDOAS ???      {
// QDOAS ???       pTreeParent=&TREE_itemList[parentItem];   // Pointer to parent
// QDOAS ???
// QDOAS ???       pTree->prevItem=pTreeParent->lastChildItem;
// QDOAS ???       pTree->nextItem=ITEM_NONE;
// QDOAS ???       pTreeParent->lastChildItem=indexItem;
// QDOAS ???       pTreeParent->childNumber++;
// QDOAS ???
// QDOAS ???       if (hidden)
// QDOAS ???        pTreeParent->childHidden++;
// QDOAS ???
// QDOAS ???       if (pTree->prevItem!=ITEM_NONE)
// QDOAS ???        TREE_itemList[pTree->prevItem].nextItem=indexItem;
// QDOAS ???       else
// QDOAS ???        pTreeParent->firstChildItem=indexItem;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Update parent
// QDOAS ???
// QDOAS ???     #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???     TREE_UpdateItem(hwndTree,parentItem);
// QDOAS ???     #endif
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return indexItem;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_DeleteChildList
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       recursive function to delete childs of a node;
// QDOAS ??? //
// QDOAS ??? // INPUT         indexParent : the index in the tree of the parent node;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_DeleteChildList(INDEX indexParent)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexItem,      // browse child nodes
// QDOAS ???         indexFirst,     // index of the first child
// QDOAS ???         indexLast;      // index of the last child
// QDOAS ???
// QDOAS ???   if ((indexParent!=ITEM_NONE) && (TREE_itemList[indexParent].firstChildItem!=ITEM_NONE))
// QDOAS ???    {
// QDOAS ???     // Index initializations
// QDOAS ???
// QDOAS ???     indexFirst=TREE_itemList[indexParent].firstChildItem;
// QDOAS ???     indexLast=TREE_itemList[indexParent].lastChildItem;
// QDOAS ???
// QDOAS ???     // Delete recursively child list of every child nodes
// QDOAS ???
// QDOAS ???     for (indexItem=indexFirst;indexItem!=ITEM_NONE;indexItem=TREE_itemList[indexItem].nextItem)
// QDOAS ???      {
// QDOAS ???       TREE_DeleteChildList(indexItem);
// QDOAS ???       TreeResetData(indexItem);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Put deleted nodes onto the free list and update parent indexes
// QDOAS ???
// QDOAS ???     TREE_itemList[treeItemFree].prevItem=indexLast;
// QDOAS ???     TREE_itemList[indexLast].nextItem=treeItemFree;
// QDOAS ???     treeItemFree=indexFirst;
// QDOAS ???
// QDOAS ???     TREE_itemList[indexParent].firstChildItem=
// QDOAS ???     TREE_itemList[indexParent].lastChildItem=ITEM_NONE;
// QDOAS ???     TREE_itemList[indexParent].childNumber=
// QDOAS ???     TREE_itemList[indexParent].childHidden=0;
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ==================================================
// QDOAS ??? // MDI CHILD WINDOWS NOTIFICATION MESSAGES PROCESSING
// QDOAS ??? // ==================================================
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_GetDispInfo
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       TVN_GETDISPINFO notification message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree : handle to the current tree;
// QDOAS ??? //               pTree    : description of the current item in the tree;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? #if defined(__BC32_) && __BC32_
// QDOAS ??? #pragma argsused
// QDOAS ??? #endif
// QDOAS ??? void TREE_GetDispInfo(HWND hwndTree,TV_DISPINFO FAR *pTree)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexItem;                                                                         // index of the item to display
// QDOAS ???
// QDOAS ???   // Get index of selected item in tree
// QDOAS ???
// QDOAS ???   if (((indexItem=(INDEX)pTree->item.lParam)!=ITEM_NONE) &&                     // item identification
// QDOAS ???      (pTree->item.mask&TVIF_TEXT))                                              // text attribute
// QDOAS ???    {
// QDOAS ???     // Fill requested fields
// QDOAS ???
// QDOAS ???     pTree->item.pszText=TREE_itemList[indexItem].textItem;                                 // parent window is responsible for storing item data
// QDOAS ???     pTree->item.cchTextMax=strlen(pTree->item.pszText);                                    // length of the item's label
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_BeginLabelEdit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       TVN_BEGINLABELEDIT notification message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree : handle to the current tree;
// QDOAS ??? //               pTree    : description of the current item in the tree;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_BeginLabelEdit(HWND hwndTree,TV_DISPINFO FAR *pTree)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexItem;                                                                         // index of the item to change
// QDOAS ???   INT dataType;                                                                            // the data type of the item to change
// QDOAS ???
// QDOAS ???   // If selected item hasn't edit attribute, cancel editing
// QDOAS ???
// QDOAS ???   if ((indexItem=(INDEX)pTree->item.lParam)!=ITEM_NONE)                         // item identification
// QDOAS ???    {
// QDOAS ???     if ((((dataType=TREE_itemList[indexItem].dataType)!=TREE_ITEM_TYPE_SITE_CHILDREN) &&   // not an observation site
// QDOAS ???           (dataType!=TREE_ITEM_TYPE_CROSS_CHILDREN) &&                                     // not a cross section symbol
// QDOAS ???           (dataType!=TREE_ITEM_TYPE_ANALYSIS_CHILDREN) &&                                  // not an analysis window
// QDOAS ???           (dataType!=TREE_ITEM_TYPE_PROJECT) &&                                            // not a project
// QDOAS ???           (RAW_GetFolderFlag(indexItem)!=RAW_FOLDER_TYPE_USER)) ||                         // not a folder
// QDOAS ???           (TREE_itemList[indexItem].useCount>0))
// QDOAS ???
// QDOAS ???      TreeView_EndEditLabelNow(hwndTree,TRUE);                                              // force end of editing mode
// QDOAS ???
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???       SendMessage(TreeView_GetEditControl(hwndTree),                                       // handle of edit control
// QDOAS ???                   EM_LIMITTEXT,                                                            // limit the number of characters that the user can enter
// QDOAS ???                   MAX_ITEM_NAME_LEN,                                                       // maximum number of characters authorized
// QDOAS ???                   0);                                                                      // not used
// QDOAS ???
// QDOAS ???       TREE_editFlag=1;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_EndLabelEdit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       TVN_ENDLABELEDIT notification message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree : handle to the current tree;
// QDOAS ??? //               pTree    : description of the current item in the tree;
// QDOAS ??? //               menuID   : index of the parent MDI child window;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_EndLabelEdit(HWND hwndTree,TV_DISPINFO FAR *pTree,INDEX menuID)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexItem;                                                                         // index of the current item
// QDOAS ???   TREE_ITEM *pItem;                                                                        // pointer to the current item
// QDOAS ???   TREE_ITEM_TYPE *pTreeType;                                                               // pointer to the type description of the current item
// QDOAS ???   INT dataType;                                                                            // type of data of the current item
// QDOAS ???   UCHAR textItem[MAX_ITEM_NAME_LEN+1],                                                     // new text after editing label
// QDOAS ???        *dataList;                                                                          // pointer to the list of data objects the current item belongs to
// QDOAS ???
// QDOAS ???   // If selected item hasn't edit attribute, cancel editing
// QDOAS ???
// QDOAS ???   TREE_editFlag=0;
// QDOAS ???
// QDOAS ???   if (((indexItem=(INDEX)pTree->item.lParam)!=ITEM_NONE) &&                     // item identification
// QDOAS ???        !TREE_itemList[indexItem].useCount &&                                               // the item text is not used somewhere
// QDOAS ???      (((dataType=TREE_itemList[indexItem].dataType)==TREE_ITEM_TYPE_SITE_CHILDREN) ||      // observation site
// QDOAS ???        (dataType==TREE_ITEM_TYPE_CROSS_CHILDREN) ||                                        // cross section symbol
// QDOAS ???        (dataType==TREE_ITEM_TYPE_ANALYSIS_CHILDREN) ||                                     // analysis window
// QDOAS ???        (dataType==TREE_ITEM_TYPE_PROJECT) ||                                               // project
// QDOAS ???        (RAW_GetFolderFlag(indexItem)==RAW_FOLDER_TYPE_USER)) &&                            // folder
// QDOAS ???         lstrlen(pTree->item.pszText))
// QDOAS ???    {
// QDOAS ???     pItem=&TREE_itemList[indexItem];
// QDOAS ???
// QDOAS ???     lstrcpy(textItem,pTree->item.pszText);
// QDOAS ???
// QDOAS ???     if (TREE_GetIndexByDataName(textItem,(UCHAR)pItem->dataType,pItem->parentItem)!=ITEM_NONE)
// QDOAS ???      MSG_MessageBox(hwndTree,ITEM_NONE,menuID,IDS_MSGBOX_DOUBLE,MB_OK|MB_ICONHAND,textItem);
// QDOAS ???     else
// QDOAS ???      {
// QDOAS ???       strcpy(pItem->textItem,textItem);                                                    // retrieve new text
// QDOAS ???       TREE_UpdateItem(hwndTree,indexItem);                                                 // update item attributes
// QDOAS ???
// QDOAS ???       if (!TLBAR_bSaveFlag)
// QDOAS ???        TLBAR_Enable(TRUE);
// QDOAS ???
// QDOAS ???       if ((pItem->dataType>TREE_ITEM_TYPE_NONE) &&
// QDOAS ???           (pItem->dataType<TREE_ITEM_TYPE_MAX))
// QDOAS ???        {
// QDOAS ???         pTreeType=&TREE_itemType[pItem->dataType];
// QDOAS ???
// QDOAS ???         // update also the data description
// QDOAS ???
// QDOAS ???         if ((dataList=(UCHAR *)pTreeType->dataList)!=NULL)
// QDOAS ???          strncpy((UCHAR *)&dataList[pItem->dataIndex*pTreeType->dataSize],
// QDOAS ???                   pItem->textItem,MAX_ITEM_NAME_LEN);
// QDOAS ???
// QDOAS ???         // for a new item, open the properties dialog box
// QDOAS ???
// QDOAS ???         if ((pItem->newItem) && (pTreeType->dlgBox!=0) && (pTreeType->dlgProc!=(DLGPROC)0))
// QDOAS ???          {
// QDOAS ???           DialogBoxParam(DOAS_hInst,                                                       // current instance
// QDOAS ???                          MAKEINTRESOURCE(pTreeType->dlgBox),                               // panel identification
// QDOAS ???                          hwndTree,                                                         // handle of parent windows
// QDOAS ???                         (DLGPROC)pTreeType->dlgProc,                                       // procedure for messages processing
// QDOAS ???                         (LPARAM)indexItem);
// QDOAS ???
// QDOAS ???           pItem->newItem=0;
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_ExpandNode
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       TVN_ITEMEXPANDING notification message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree : handle to the current tree;
// QDOAS ??? //               pTree    : description of the current item in the tree;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_ExpandNode(HWND hwndTree,NM_TREEVIEW *pTree)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexItem;                                                                         // index of the current item in the tree
// QDOAS ???
// QDOAS ???   // Get index of the selected item in tree
// QDOAS ???
// QDOAS ???   if ((pTree->action==TVE_EXPAND)&&
// QDOAS ???      ((indexItem=(INDEX)pTree->itemNew.lParam)!=ITEM_NONE)&&                               // item identification
// QDOAS ???       (TREE_itemList[indexItem].firstChildItem!=ITEM_NONE))                                // item owns a list of child items
// QDOAS ???
// QDOAS ???   // Expand list of child items
// QDOAS ???
// QDOAS ???    TREE_ExpandOneNode(hwndTree,                                                            // handle of the TreeView control
// QDOAS ???                       indexItem,                                                           // index of the item to expand
// QDOAS ???                       TREE_itemList[indexItem].hti);                                       // handle of the item to expand
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_CollapseNode
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       TVN_ITEMEXPANDED notification message processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree : handle to the current tree;
// QDOAS ??? //               pTree    : description of the current item in the tree;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_CollapseNode(HWND hwndTree,NM_TREEVIEW *pTree)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexParent;                                                                       // index of the item to collapse
// QDOAS ???
// QDOAS ???   // Get index of the selected item in tree
// QDOAS ???
// QDOAS ???   if ((pTree->action==TVE_COLLAPSE) &&
// QDOAS ???      ((indexParent=(INDEX)pTree->itemNew.lParam)!=ITEM_NONE) &&                            // item identification
// QDOAS ???       (TREE_itemList[indexParent].firstChildItem!=ITEM_NONE))                              // item owns a list of child items
// QDOAS ???
// QDOAS ???   // Collapse list of child items
// QDOAS ???
// QDOAS ???    TREE_CollapseChildNodes(hwndTree,indexParent);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ================================
// QDOAS ??? // CONTEXT MENU COMMANDS PROCESSING
// QDOAS ??? // ================================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_InsertItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_INSERT command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree   : handle to the current tree;
// QDOAS ??? //               menuID     : index of the calling MDI child window;
// QDOAS ??? //               folderFlag : set if the item to insert is a folder (user-defined or directory).
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_InsertItem(HWND hwndTree,INT menuID,INT folderFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR textItem[MAX_ITEM_DESC_LEN+1];     // default string for new item
// QDOAS ???   TREE_ITEM *pTreeItem;                    // pointer to the parent item in the tree
// QDOAS ???   TREE_ITEM_TYPE *pTreeType;               // pointer to the description of the type of the parent node
// QDOAS ???   INT   contextMenu,                       // identification of the context menu called from the parent node
// QDOAS ???         dataType;                          // data type of the new item
// QDOAS ???   INDEX indexItem,                         // index of the new item
// QDOAS ???         indexParent;                       // index of the parent node
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   indexItem=ITEM_NONE;
// QDOAS ???
// QDOAS ???   // Get index of the selected item in the tree
// QDOAS ???
// QDOAS ???   if (((indexParent=TREE_GetSelectedItem(hwndTree))!=ITEM_NONE) && THRD_Context(indexItem,MENU_CONTEXT_INSERT) &&
// QDOAS ???        (RAW_GetFolderFlag(indexParent)!=RAW_FOLDER_TYPE_PATH) &&     // the parent is not a directory folder
// QDOAS ???        (RAW_GetFolderFlag(indexParent)!=RAW_FOLDER_TYPE_PATHSUB))    // neither a sub-directory folder
// QDOAS ???    {
// QDOAS ???     pTreeItem=&TREE_itemList[indexParent];
// QDOAS ???     pTreeType=&TREE_itemType[(RAW_GetFolderFlag(indexParent))?TREE_ITEM_TYPE_FILE_PARENT:pTreeItem->dataType];
// QDOAS ???
// QDOAS ???     if ((pTreeItem->hti!=(HTREEITEM)0) && ((dataType=pTreeType->childDataType)!=TREE_ITEM_TYPE_NONE))
// QDOAS ???      {
// QDOAS ???       // Insert files
// QDOAS ???
// QDOAS ???       if (((contextMenu=pTreeType->contextMenu)==MENU_CONTEXT_INSERT_FILES) && (folderFlag==RAW_FOLDER_TYPE_NONE))
// QDOAS ???        indexItem=FILES_Insert(hwndTree,indexParent,dataType,pTreeType->childFileType);
// QDOAS ???
// QDOAS ???       // Insert a new item
// QDOAS ???
// QDOAS ???       else if ((contextMenu==MENU_CONTEXT_INSERT) || (folderFlag==RAW_FOLDER_TYPE_USER))
// QDOAS ???        {
// QDOAS ???         SZ_LEN textItemLength;
// QDOAS ???         pTreeType=&TREE_itemType[dataType];
// QDOAS ???
// QDOAS ???         // Insert item in the tree
// QDOAS ???
// QDOAS ???         sprintf(textItem,"new %s ",(folderFlag==RAW_FOLDER_TYPE_USER)?"folder":pTreeItem->textItem);
// QDOAS ???         textItemLength=strlen(textItem);
// QDOAS ???         textItem[textItemLength-1]=0;           // Delete extra "s"
// QDOAS ???
// QDOAS ???         if ((pTreeType->dataMaxNumber==0) ||                                        // no objects list is associated to the type of item
// QDOAS ???             (pTreeType->dataNumber>=pTreeType->dataMaxNumber) ||                    // objects list is full
// QDOAS ???            ((indexItem=TREE_InsertOneItem(hwndTree,textItem,indexParent,dataType,   // can't allocate an item in the tree
// QDOAS ???             (UCHAR)!folderFlag,(UCHAR)folderFlag,0))==ITEM_NONE))
// QDOAS ???
// QDOAS ???          MSG_MessageBox(hwndTree,ITEM_NONE,menuID,IDS_MSGBOX_INSERT,MB_OK|MB_ICONHAND,textItem);
// QDOAS ???
// QDOAS ???         else
// QDOAS ???          {
// QDOAS ???           if (!TLBAR_bSaveFlag)
// QDOAS ???            TLBAR_Enable(TRUE);
// QDOAS ???
// QDOAS ???           if (folderFlag==RAW_FOLDER_TYPE_USER)
// QDOAS ???            RAW_UpdateWindoasPath(indexItem,indexParent,1);
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Insert items in tree by expanding parent node
// QDOAS ???
// QDOAS ???       TREE_ExpandOneNode(hwndTree,indexParent,pTreeItem->hti);
// QDOAS ???       TREE_ExpandAll(hwndTree,indexItem);
// QDOAS ???       TreeView_Expand(hwndTree,pTreeItem->hti,TVE_EXPAND);
// QDOAS ???
// QDOAS ???       // Edit label
// QDOAS ???
// QDOAS ???       if ((indexItem!=ITEM_NONE) &&
// QDOAS ???           (TREE_itemList[indexItem].hti) &&
// QDOAS ???          ((contextMenu!=MENU_CONTEXT_INSERT_FILES) || (folderFlag==RAW_FOLDER_TYPE_USER)))
// QDOAS ???        TreeView_EditLabel(hwndTree,TREE_itemList[indexItem].hti);
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_Show
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_SHOW command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree   : handle to the current tree;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_Show(HWND hwndTree)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX      indexItem;    // index of the selected item in the tree
// QDOAS ???   TREE_ITEM *pTree;        // pointer to the selected item in the tree
// QDOAS ???
// QDOAS ???   if ((indexItem=TREE_GetSelectedItem(hwndTree))!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     pTree=&TREE_itemList[indexItem];
// QDOAS ???
// QDOAS ???     // Show all hidden child nodes
// QDOAS ???
// QDOAS ???     TreeShow(indexItem);
// QDOAS ???
// QDOAS ???     // Insert physically child items by expanding the parent node
// QDOAS ???
// QDOAS ???     if (pTree->hti!=(HTREEITEM)0)
// QDOAS ???      {
// QDOAS ???       TREE_CollapseChildNodes(hwndTree,indexItem);
// QDOAS ???       TREE_ExpandOneNode(hwndTree,indexItem,pTree->hti);
// QDOAS ???       TreeView_Expand(hwndTree,pTree->hti,TVE_EXPAND);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     TREE_UpdateItem(hwndTree,indexItem);
// QDOAS ???
// QDOAS ???     if (!TLBAR_bSaveFlag)
// QDOAS ???      TLBAR_Enable(TRUE);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_Hide
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_HIDE command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree   : handle to the current tree;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_Hide(HWND hwndTree)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX      indexItem;    // index of the item in the tree to hide
// QDOAS ???   HTREEITEM  hti;          // handle of the item to hide
// QDOAS ???   TREE_ITEM *pTree;        // pointer to the item to hide
// QDOAS ???
// QDOAS ???   // Get index of item to hide
// QDOAS ???
// QDOAS ???   if (((indexItem=TREE_GetSelectedItem(hwndTree))!=ITEM_NONE) &&
// QDOAS ???         THRD_Context(indexItem,MENU_CONTEXT_HIDE) &&
// QDOAS ???       ((hti=TREE_itemList[indexItem].hti)!=(HTREEITEM)0))
// QDOAS ???    {
// QDOAS ???     pTree=&TREE_itemList[indexItem];
// QDOAS ???
// QDOAS ???     if (!pTree->useCount &&                                       // the name of the item is not used anywhere
// QDOAS ???        ((pTree->dataType==TREE_ITEM_TYPE_ANALYSIS_CHILDREN) ||    // can hide analysis windows
// QDOAS ???         (pTree->dataType==TREE_ITEM_TYPE_FILE_CHILDREN)           // or files
// QDOAS ???        ))
// QDOAS ???      {
// QDOAS ???       pTree->hidden=2;                                            // 0 : no hidden, 1 : Kurucz only, 2 : user
// QDOAS ???       TREE_itemList[pTree->parentItem].childHidden++;             // update the parent
// QDOAS ???
// QDOAS ???       // update the data description
// QDOAS ???
// QDOAS ???       if (pTree->dataIndex!=ITEM_NONE)
// QDOAS ???        {
// QDOAS ???         if (pTree->dataType==TREE_ITEM_TYPE_ANALYSIS_CHILDREN)
// QDOAS ???          ANLYS_windowsList[pTree->dataIndex].hidden=pTree->hidden;
// QDOAS ???         else if (pTree->dataType==TREE_ITEM_TYPE_FILE_CHILDREN)
// QDOAS ???          RAW_spectraFiles[pTree->dataIndex].hidden=pTree->hidden;
// QDOAS ???         else if (pTree->dataType==TREE_ITEM_TYPE_SITE_CHILDREN)
// QDOAS ???          SITES_itemList[pTree->dataIndex].hidden=pTree->hidden;
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Delete 'physically' the selected item in the tree
// QDOAS ???
// QDOAS ???       TreeView_DeleteItem(hwndTree,hti);
// QDOAS ???       pTree->hti=(HTREEITEM)0;
// QDOAS ???
// QDOAS ???       // Update parent node
// QDOAS ???
// QDOAS ???       TREE_UpdateItem(hwndTree,TREE_itemList[indexItem].parentItem);
// QDOAS ???
// QDOAS ???       if (!TLBAR_bSaveFlag)
// QDOAS ???        TLBAR_Enable(TRUE);
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_RenameItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_RENAME command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree   : handle to the current tree;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_RenameItem(HWND hwndTree)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexItem;         // index of the selected item in the tree
// QDOAS ???   HTREEITEM hti;           // handle of the selected item
// QDOAS ???
// QDOAS ???   // Get the index of the item that has the selection
// QDOAS ???
// QDOAS ???   if (((indexItem=TREE_GetSelectedItem(hwndTree))!=ITEM_NONE) &&
// QDOAS ???       ((hti=TREE_itemList[indexItem].hti)!=(HTREEITEM)0))
// QDOAS ???
// QDOAS ???   // Force editing of the label
// QDOAS ???
// QDOAS ???    TreeView_EditLabel(hwndTree,hti);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_Sort
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_SORT_NAME command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree   : handle to the current tree;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_Sort(HWND hwndTree)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexParent;       // index of the selected item in the tree
// QDOAS ???
// QDOAS ???   // Get index of item that has the selection
// QDOAS ???
// QDOAS ???   if ((indexParent=TREE_GetSelectedItem(hwndTree))!=ITEM_NONE)                        // index of parent node
// QDOAS ???    {
// QDOAS ???     if (!TLBAR_bSaveFlag)
// QDOAS ???      TLBAR_Enable(TRUE);
// QDOAS ???
// QDOAS ???     // Sort recursively child nodes
// QDOAS ???
// QDOAS ???     TREE_SortChildNodes(hwndTree,indexParent);
// QDOAS ???
// QDOAS ???     // Update tree view by collapsing and expanding the node
// QDOAS ???
// QDOAS ???     TREE_CollapseChildNodes(hwndTree,indexParent);
// QDOAS ???     TREE_ExpandAll(hwndTree,indexParent);
// QDOAS ???     TREE_UpdateItem(hwndTree,indexParent);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_DeleteItem
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_DELETE command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree   : handle to the current tree;
// QDOAS ??? //               menuID     : index of the calling MDI child window;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_DeleteItem(HWND hwndTree,INT menuID)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexItem,    // index in the tree of the item to delete
// QDOAS ???         indexParent;  // index in the tree of the parent node
// QDOAS ???   INT contextMenu;    // id of the context menu
// QDOAS ???   HTREEITEM hti;      // handle of the item to delete
// QDOAS ???
// QDOAS ???   // Deletion of the selected item
// QDOAS ???
// QDOAS ???   if (((indexItem=TREE_GetSelectedItem(hwndTree))!=ITEM_NONE) && THRD_Context(indexItem,MENU_CONTEXT_DELETE) &&
// QDOAS ???      (((contextMenu=TREE_itemType[TREE_itemList[indexItem].dataType].contextMenu)==MENU_CONTEXT_EDIT) ||
// QDOAS ???        (contextMenu==MENU_CONTEXT_CROSS) ||
// QDOAS ???        (contextMenu==MENU_CONTEXT_SPECTRA) || (contextMenu==MENU_CONTEXT_PROJECT)) &&
// QDOAS ???       ((hti=TREE_itemList[indexItem].hti)!=(HTREEITEM)0) &&
// QDOAS ???        !TREE_itemList[indexItem].useCount &&
// QDOAS ???        (MSG_MessageBox(hwndTree,ITEM_NONE,menuID,IDS_MSGBOX_DELETE,
// QDOAS ???                         MB_YESNO|MB_ICONQUESTION,
// QDOAS ???                         TREE_itemList[indexItem].textItem)==IDYES))
// QDOAS ???    {
// QDOAS ???     TreeView_DeleteItem(hwndTree,hti);
// QDOAS ???     indexParent=TREE_DeleteOneItem(indexItem);
// QDOAS ???     TREE_UpdateItem(hwndTree,indexParent);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_DeleteAllItems
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_DELETEALL command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree   : handle to the current tree;
// QDOAS ??? //               menuID     : index of the calling MDI child window;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_DeleteAllItems(HWND hwndTree,INT menuID)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexItem,        // index in the tree of the parent node
// QDOAS ???         indexChild,       // browse child nodes
// QDOAS ???         indexNextItem;    // go to the next child node
// QDOAS ???
// QDOAS ???   // Get the selected item
// QDOAS ???
// QDOAS ???   if (((indexItem=TREE_GetSelectedItem(hwndTree))!=ITEM_NONE) &&    // index of the selected item in list
// QDOAS ???        (TREE_itemList[indexItem].hti!=(HTREEITEM)0) &&              // handle of the selected item in tree
// QDOAS ???        (TREE_itemList[indexItem].firstChildItem!=ITEM_NONE) &&      // selected item has child(ren)
// QDOAS ???        (MSG_MessageBox(hwndTree,ITEM_NONE,menuID,IDS_MSGBOX_DELETEALL,
// QDOAS ???                         MB_YESNO|MB_ICONQUESTION,
// QDOAS ???                         TREE_itemList[indexItem].textItem)==IDYES))
// QDOAS ???    {
// QDOAS ???     // Delete 'physically' all child nodes by collapsing the parent node
// QDOAS ???
// QDOAS ???     TREE_CollapseChildNodes(hwndTree,indexItem);
// QDOAS ???
// QDOAS ???     // Delete all child nodes
// QDOAS ???
// QDOAS ???     for (indexChild=TREE_itemList[indexItem].firstChildItem;indexChild!=ITEM_NONE;indexChild=indexNextItem)
// QDOAS ???      {
// QDOAS ???       indexNextItem=TREE_itemList[indexChild].nextItem;
// QDOAS ???       if (!TREE_itemList[indexChild].hidden && !TREE_itemList[indexChild].useCount)
// QDOAS ???        TREE_DeleteOneItem(indexChild);
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Update items attributes
// QDOAS ???
// QDOAS ???     TREE_UpdateItem(hwndTree,indexItem);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_ExpandAll
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_EXPAND command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree    : handle to the current tree;
// QDOAS ??? //               indexParent : index in the tree of the parent node;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_ExpandAll(HWND hwndTree,INDEX indexParent)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexItem;   // index of the first child item
// QDOAS ???
// QDOAS ???   if ((indexParent!=ITEM_NONE) &&
// QDOAS ???      ((indexItem=TREE_itemList[indexParent].firstChildItem)!=ITEM_NONE))
// QDOAS ???    {
// QDOAS ???     // Expand the parent node
// QDOAS ???
// QDOAS ???     TREE_ExpandOneNode(hwndTree,indexParent,TREE_itemList[indexParent].hti);
// QDOAS ???
// QDOAS ???     if (TREE_itemList[indexParent].hti!=TVI_ROOT)
// QDOAS ???      TreeView_Expand(hwndTree,TREE_itemList[indexParent].hti,TVE_EXPAND);
// QDOAS ???
// QDOAS ???     while (indexItem!=ITEM_NONE)
// QDOAS ???      {
// QDOAS ???       // Expand recursively child nodes
// QDOAS ???
// QDOAS ???       if (TREE_itemList[indexItem].firstChildItem!=ITEM_NONE)
// QDOAS ???        TREE_ExpandAll(hwndTree,indexItem);
// QDOAS ???
// QDOAS ???       // Go to the next child
// QDOAS ???
// QDOAS ???       indexItem=TREE_itemList[indexItem].nextItem;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_CollapseAll
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_COLLAPSE command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree    : handle to the current tree;
// QDOAS ??? //               indexParent : index in the tree of the parent node;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_CollapseAll(HWND hwndTree,INDEX indexParent)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexItem;
// QDOAS ???
// QDOAS ???   // browse child nodes
// QDOAS ???
// QDOAS ???   if (indexParent!=ITEM_NONE)
// QDOAS ???
// QDOAS ???    for (indexItem=TREE_itemList[indexParent].firstChildItem;
// QDOAS ???         indexItem!=ITEM_NONE;
// QDOAS ???         indexItem=TREE_itemList[indexItem].nextItem)
// QDOAS ???
// QDOAS ???     TREE_CollapseChildNodes(hwndTree,indexItem);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_Properties
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_PROPERTIES command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree    : handle to the current tree;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_Properties(HWND hwndTree)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexItem;            // index in the tree of the selected item
// QDOAS ???   TREE_ITEM *pTree;           // pointer to the selected item
// QDOAS ???   TREE_ITEM_TYPE *pTreeType;  // pointer to the type description of the selected item
// QDOAS ???
// QDOAS ???   if ((indexItem=TREE_GetSelectedItem(hwndTree))!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     pTree=&TREE_itemList[indexItem];
// QDOAS ???
// QDOAS ???     if ((pTree->dataType>TREE_ITEM_TYPE_NONE) &&
// QDOAS ???         (pTree->dataType<TREE_ITEM_TYPE_MAX))
// QDOAS ???      {
// QDOAS ???       pTreeType=&TREE_itemType[pTree->dataType];
// QDOAS ???
// QDOAS ???       // open properties dialog box
// QDOAS ???
// QDOAS ???       if ((pTree->dataType==TREE_ITEM_TYPE_FILE_CHILDREN) &&
// QDOAS ???          ((RAW_GetFolderFlag(indexItem)==RAW_FOLDER_TYPE_PATH) ||
// QDOAS ???           (RAW_GetFolderFlag(indexItem)==RAW_FOLDER_TYPE_PATHSUB)))
// QDOAS ???
// QDOAS ???        DialogBoxParam(DOAS_hInst,                                                  // current instance
// QDOAS ???                       MAKEINTRESOURCE(DLG_PATH),                                   // panel identification
// QDOAS ???                       hwndTree,                                                    // handle of parent windows
// QDOAS ???                      (DLGPROC)PATH_WndProc,                                        // procedure for messages processing
// QDOAS ???                      (LPARAM)indexItem);
// QDOAS ???
// QDOAS ???       else if ((pTreeType->dlgBox!=0) && (pTreeType->dlgProc!=(DLGPROC)0))
// QDOAS ???
// QDOAS ???        DialogBoxParam(DOAS_hInst,                                                  // current instance
// QDOAS ???                       MAKEINTRESOURCE(pTreeType->dlgBox),                          // panel identification
// QDOAS ???                       hwndTree,                                                    // handle of parent windows
// QDOAS ???                      (DLGPROC)pTreeType->dlgProc,                                  // procedure for messages processing
// QDOAS ???                      (LPARAM)indexItem);
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? void TreeOutputConfig(FILE *fp,INDEX indexItem)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   TREE_ITEM *pTree;
// QDOAS ???   INDEX indexNextItem;
// QDOAS ???
// QDOAS ???   pTree=&TREE_itemList[indexItem];
// QDOAS ???
// QDOAS ???   if (pTree->dataType==TREE_ITEM_TYPE_ANALYSIS_CHILDREN)
// QDOAS ???    ANLYS_OutputConfiguration(fp,
// QDOAS ???     TREE_itemList[TREE_itemList[pTree->parentItem].parentItem].dataIndex,       // index of the grand-father (project)
// QDOAS ???      pTree->dataIndex);                                                         // index of the current node (analysis window)
// QDOAS ???   else if (pTree->dataType==TREE_ITEM_TYPE_PROJECT)
// QDOAS ???    PRJCT_OutputConfiguration(fp,pTree->dataIndex);                              // index of the current node (project)
// QDOAS ???
// QDOAS ???   for (indexItem=pTree->firstChildItem;indexItem!=ITEM_NONE;indexItem=indexNextItem)
// QDOAS ???    {
// QDOAS ???     indexNextItem=TREE_itemList[indexItem].nextItem;
// QDOAS ???     TreeOutputConfig(fp,indexItem);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_OutputConfig
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_CONFIG command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree    : handle to the current tree;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_OutputConfig(HWND hwndTree)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX indexItem;            // index in the tree of the selected item
// QDOAS ???   TREE_ITEM *pTree;           // pointer to the selected item
// QDOAS ???   TREE_ITEM_TYPE *pTreeType;  // pointer to the type description of the selected item
// QDOAS ???   UCHAR path[MAX_PATH_LEN+1];
// QDOAS ???   FILE *fp;
// QDOAS ???
// QDOAS ???   if ((indexItem=TREE_GetSelectedItem(hwndTree))!=ITEM_NONE)
// QDOAS ???    {
// QDOAS ???     pTree=&TREE_itemList[indexItem];
// QDOAS ???
// QDOAS ???     if ((pTree->dataType>TREE_ITEM_TYPE_NONE) &&
// QDOAS ???         (pTree->dataType<TREE_ITEM_TYPE_MAX))
// QDOAS ???      {
// QDOAS ???       pTreeType=&TREE_itemType[pTree->dataType];
// QDOAS ???
// QDOAS ???       // open properties dialog box
// QDOAS ???
// QDOAS ???       if ((pTree->dataType==TREE_ITEM_TYPE_PROJECT_PARENT) ||
// QDOAS ???           (pTree->dataType==TREE_ITEM_TYPE_PROJECT) ||
// QDOAS ???           (pTree->dataType==TREE_ITEM_TYPE_ANALYSIS_PARENT) ||
// QDOAS ???           (pTree->dataType==TREE_ITEM_TYPE_ANALYSIS_CHILDREN))
// QDOAS ???        {
// QDOAS ???         FILES_Select(hwndTree,path,MAX_PATH_LEN+1,FILE_TYPE_CFG,0,FILE_MODE_SAVE,NULL,ITEM_NONE,ITEM_NONE);
// QDOAS ???
// QDOAS ???         if ((fp=fopen(path,"w+t"))==NULL)
// QDOAS ???          MSG_MessageBox(hwndTree,ITEM_NONE,MENU_WINDOWS_CHILD+CHILD_WINDOW_PROJECT,IDS_MSGBOX_FILEOPEN,MB_OK|MB_ICONHAND,path);
// QDOAS ???         else
// QDOAS ???          {
// QDOAS ???           TreeOutputConfig(fp,indexItem);
// QDOAS ???           fclose(fp);
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_Copy
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_COPY command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree : handle to the current tree;
// QDOAS ??? //               cutFlag  : 0 for copy/paste; 1 for cut/paste.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_Copy(HWND hwndTree,INT cutFlag)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX           indexItem,            // index of the selected node in the tree
// QDOAS ???                   dataIndex,            // index in the objects list of the selected node
// QDOAS ???                   indexChild,           // browse child nodes if the node to copy is a parent one
// QDOAS ???                   indexNextItem;        // next child node
// QDOAS ???   INT             dataSize;             // size in bytes of the selected OBJECT
// QDOAS ???   TREE_ITEM      *pTree;                // pointer to the selected node in the tree
// QDOAS ???   TREE_ITEM_TYPE *pTreeType,            // pointer to the type description of the selected item
// QDOAS ???                  *pTreeChildType;       // pointer to the type description of childs of the selected item
// QDOAS ???   UCHAR          *pData2Copy,           // pointer to the data list to copy
// QDOAS ???                  *pData2Paste;          // pointer to the data list to paste
// QDOAS ???   HMENU           hMenu,hSubMenu;       // handles to the projects menu
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   TREE_typeOfItem2Copy=ITEM_NONE;       // type of the parent node to copy
// QDOAS ???   treeNItem2Copy=0;                     // number of items to copy
// QDOAS ???
// QDOAS ???   // Get the selected item in the tree
// QDOAS ???
// QDOAS ???   if (((indexItem=TREE_GetSelectedItem(hwndTree))!=ITEM_NONE) && THRD_Context(indexItem,MENU_CONTEXT_COPY))
// QDOAS ???    {
// QDOAS ???     // Initialize pointers
// QDOAS ???
// QDOAS ???     pTree=&TREE_itemList[indexItem];
// QDOAS ???     pTreeType=&TREE_itemType[pTree->dataType];
// QDOAS ???     pTreeChildType=&TREE_itemType[TREE_itemType[pTree->dataType].childDataType];
// QDOAS ???
// QDOAS ???     if (cutFlag)
// QDOAS ???      treeCutIndex=indexItem;
// QDOAS ???
// QDOAS ???     // Copy of a child node
// QDOAS ???     // --------------------
// QDOAS ???
// QDOAS ???     if (((pTree->dataType==TREE_ITEM_TYPE_SITE_CHILDREN) ||          // observation sites children nodes
// QDOAS ???          (pTree->dataType==TREE_ITEM_TYPE_CROSS_CHILDREN) ||         // user-defined symbols children nodes
// QDOAS ???          (pTree->dataType==TREE_ITEM_TYPE_ANALYSIS_CHILDREN) ||      // analysis windows children nodes
// QDOAS ???          (pTree->dataType==TREE_ITEM_TYPE_PROJECT)) &&               // projects nodes
// QDOAS ???         ((dataIndex=pTree->dataIndex)!=ITEM_NONE) &&
// QDOAS ???         ((dataSize=pTreeType->dataSize)>0) &&
// QDOAS ???         ((pData2Copy=(UCHAR *)pTreeType->dataList)!=NULL) &&
// QDOAS ???         ((pData2Paste=(UCHAR *)pTreeType->data2Paste)!=NULL))
// QDOAS ???      {
// QDOAS ???       // safe keeping of the data description of the selected item
// QDOAS ???
// QDOAS ???       TREE_typeOfItem2Copy=pTree->dataType;
// QDOAS ???       memcpy(pData2Paste,&pData2Copy[dataIndex*dataSize],dataSize);
// QDOAS ???
// QDOAS ???       // for projects and analysis windows items, the safe keeping is extended to child items
// QDOAS ???
// QDOAS ???       if ((TREE_typeOfItem2Copy==TREE_ITEM_TYPE_ANALYSIS_CHILDREN) || (TREE_typeOfItem2Copy==TREE_ITEM_TYPE_PROJECT))
// QDOAS ???        {
// QDOAS ???         ANLYS_nItemsToPaste=0;
// QDOAS ???
// QDOAS ???         if (TREE_typeOfItem2Copy==TREE_ITEM_TYPE_ANALYSIS_CHILDREN)
// QDOAS ???          ANLYS_CopyItems((ANALYSIS_WINDOWS *)pData2Paste,(ANALYSIS_WINDOWS *)&pData2Copy[dataIndex*dataSize]);
// QDOAS ???         else
// QDOAS ???          PRJCT_Copy(indexItem,(PROJECT *)pData2Paste,(PROJECT *)&pData2Copy[dataIndex*dataSize]);
// QDOAS ???        }
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     // Copy of a parent node
// QDOAS ???     // ---------------------
// QDOAS ???
// QDOAS ???     else if (((pTree->dataType==TREE_ITEM_TYPE_SITE_PARENT) ||       // observation sites parent nodes
// QDOAS ???               (pTree->dataType==TREE_ITEM_TYPE_CROSS_PARENT) ||      // user-defined symbols parent nodes
// QDOAS ???               (pTree->dataType==TREE_ITEM_TYPE_ANALYSIS_PARENT)) &&  // analysis windows parent nodes
// QDOAS ???              ((dataSize=pTreeChildType->dataSize)>0) &&
// QDOAS ???              ((pData2Copy=(UCHAR *)pTreeChildType->dataList)!=NULL) &&
// QDOAS ???              ((pData2Paste=(UCHAR *)pTreeChildType->data2Paste)!=NULL))
// QDOAS ???      {
// QDOAS ???       if ((pTree->firstChildItem!=ITEM_NONE) && (pTree->dataType==TREE_ITEM_TYPE_ANALYSIS_PARENT))
// QDOAS ???        ANLYS_nItemsToPaste=0;
// QDOAS ???
// QDOAS ???       // browse child nodes
// QDOAS ???
// QDOAS ???       for (indexChild=pTree->firstChildItem;indexChild!=ITEM_NONE;indexChild=indexNextItem)
// QDOAS ???        {
// QDOAS ???         indexNextItem=TREE_itemList[indexChild].nextItem;
// QDOAS ???
// QDOAS ???         if (!TREE_itemList[indexChild].hidden && ((dataIndex=TREE_itemList[indexChild].dataIndex)!=ITEM_NONE))
// QDOAS ???          {
// QDOAS ???           memcpy(&pData2Paste[treeNItem2Copy*dataSize],&pData2Copy[dataIndex*dataSize],dataSize);
// QDOAS ???
// QDOAS ???           if (TREE_itemType[pTree->dataType].childDataType==TREE_ITEM_TYPE_ANALYSIS_CHILDREN)
// QDOAS ???            ANLYS_CopyItems((ANALYSIS_WINDOWS *)&pData2Paste[treeNItem2Copy*dataSize],(ANALYSIS_WINDOWS *)&pData2Copy[dataIndex*dataSize]);
// QDOAS ???
// QDOAS ???           treeNItem2Copy++;
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       if (treeNItem2Copy)
// QDOAS ???        TREE_typeOfItem2Copy=TREE_itemType[pTree->dataType].childDataType;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Update paste menus
// QDOAS ???
// QDOAS ???   hMenu=GetMenu(GetParent(TLBAR_hwnd));
// QDOAS ???   hSubMenu=GetSubMenu(hMenu,3);            // Menu 'Projects'
// QDOAS ???
// QDOAS ???   EnableMenuItem(hSubMenu,MENU_PROJECT_PASTE,(TREE_typeOfItem2Copy==TREE_ITEM_TYPE_PROJECT)?MF_ENABLED:MF_GRAYED);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_Paste
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       MENU_CONTEXT_PASTE command processing
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndTree : handle to the current tree;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_Paste(HWND hwndTree)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   INDEX      indexItem,                      // the parent node for the paste operation
// QDOAS ???              indexNewItem,                   // index in the tree of the new pasted node
// QDOAS ???              indexData,                      // index in the objects list of the new pasted node
// QDOAS ???              i,                              // browse items to paste
// QDOAS ???              indexParent;
// QDOAS ???   INT        dataSize,
// QDOAS ???              dataType;                       // data type of child nodes to paste
// QDOAS ???   TREE_ITEM *pTree;                          // pointer to the parent node for the paste operation
// QDOAS ???   UCHAR     *dataList,                       // objects list child nodes to paste belong to
// QDOAS ???             *pData2Paste,                    // list of objects to paste
// QDOAS ???              string[MAX_ITEM_TEXT_LEN+1];    // new text of the pasted node (if only one pasted node)
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   indexItem=ITEM_NONE;
// QDOAS ???
// QDOAS ???   // Get the parent node for the paste operation
// QDOAS ???
// QDOAS ???   if (TREE_typeOfItem2Copy!=TREE_ITEM_TYPE_PROJECT)
// QDOAS ???    indexItem=TREE_GetSelectedItem(hwndTree);
// QDOAS ???   else if (THRD_id==THREAD_TYPE_NONE)
// QDOAS ???    indexItem=CHILD_list[CHILD_WINDOW_PROJECT].itemTree;
// QDOAS ???
// QDOAS ???   if ((indexItem!=ITEM_NONE) && THRD_Context(indexItem,MENU_CONTEXT_PASTE))
// QDOAS ???    {
// QDOAS ???     pTree=&TREE_itemList[indexItem];
// QDOAS ???
// QDOAS ???     if (((dataType=TREE_itemType[pTree->dataType].childDataType)==TREE_typeOfItem2Copy) && (pTree->hti!=NULL) &&
// QDOAS ???         ((dataList=(UCHAR *)TREE_itemType[dataType].dataList)!=NULL) &&
// QDOAS ???         ((pData2Paste=(UCHAR *)TREE_itemType[dataType].data2Paste)!=NULL) &&
// QDOAS ???         ((dataSize=TREE_itemType[dataType].dataSize)>0))
// QDOAS ???      {
// QDOAS ???       // Only one node to paste
// QDOAS ???       // ----------------------
// QDOAS ???
// QDOAS ???       if (!treeNItem2Copy)
// QDOAS ???        {
// QDOAS ???         sprintf(string,"Copy of %s",pData2Paste);
// QDOAS ???         string[MAX_ITEM_NAME_LEN]=0;
// QDOAS ???
// QDOAS ???         if (((indexNewItem=TREE_InsertOneItem(hwndTree,string,indexItem,TREE_typeOfItem2Copy,0,0,0))!=ITEM_NONE) &&
// QDOAS ???             ((indexData=TREE_itemList[indexNewItem].dataIndex)!=ITEM_NONE))
// QDOAS ???          {
// QDOAS ???           memcpy(&dataList[indexData*dataSize]+MAX_ITEM_NAME_LEN+1,pData2Paste+MAX_ITEM_NAME_LEN+1,dataSize-MAX_ITEM_NAME_LEN-1);
// QDOAS ???
// QDOAS ???           // for projects and analysis windows nodes, the paste operation is extended to child items
// QDOAS ???
// QDOAS ???           if (TREE_typeOfItem2Copy==TREE_ITEM_TYPE_ANALYSIS_CHILDREN)
// QDOAS ???            ANLYS_PasteItems((ANALYSIS_WINDOWS *)pData2Paste,(ANALYSIS_WINDOWS *)&dataList[indexData*dataSize],indexNewItem);
// QDOAS ???           else if ((TREE_typeOfItem2Copy==TREE_ITEM_TYPE_PROJECT) &&
// QDOAS ???                   ((indexParent=TREE_InsertOneItem(hwndTree,"Raw Spectra ",indexNewItem,TREE_ITEM_TYPE_FILE_PARENT,0,0,0))!=ITEM_NONE) &&
// QDOAS ???                   ((indexParent=TREE_InsertOneItem(hwndTree,"Analysis Windows",indexNewItem,TREE_ITEM_TYPE_ANALYSIS_PARENT,0,0,0))!=ITEM_NONE) &&
// QDOAS ???                    (ANLYS_toPaste!=NULL))
// QDOAS ???
// QDOAS ???            PRJCT_Paste(hwndTree,indexParent,(PROJECT *)&dataList[indexData*dataSize],(PROJECT *)pData2Paste);
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Several nodes to paste
// QDOAS ???       // ----------------------
// QDOAS ???
// QDOAS ???       else
// QDOAS ???        {
// QDOAS ???         for (i=0;i<treeNItem2Copy;i++)
// QDOAS ???          {
// QDOAS ???           if (((indexNewItem=TREE_GetIndexByDataName(&pData2Paste[i*dataSize],(UCHAR)dataType,indexItem))==ITEM_NONE) &&
// QDOAS ???               ((indexNewItem=TREE_InsertOneItem(hwndTree,&pData2Paste[i*dataSize],indexItem,TREE_typeOfItem2Copy,0,0,0))!=ITEM_NONE) &&
// QDOAS ???               ((indexData=TREE_itemList[indexNewItem].dataIndex)!=ITEM_NONE))
// QDOAS ???            {
// QDOAS ???             memcpy(&dataList[indexData*dataSize],&pData2Paste[i*dataSize],dataSize);
// QDOAS ???             if (TREE_typeOfItem2Copy==TREE_ITEM_TYPE_ANALYSIS_CHILDREN)
// QDOAS ???              ANLYS_PasteItems((ANALYSIS_WINDOWS *)&pData2Paste[i*dataSize],(ANALYSIS_WINDOWS *)&dataList[indexData*dataSize],indexNewItem);
// QDOAS ???            }
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Insert 'physically' pasted items in the tree by expanding the parent node
// QDOAS ???
// QDOAS ???       TREE_ExpandOneNode(hwndTree,indexItem,pTree->hti);
// QDOAS ???       TreeView_Expand(hwndTree,pTree->hti,TVE_EXPAND);
// QDOAS ???
// QDOAS ???       // For cut operation, delete the original node
// QDOAS ???
// QDOAS ???       if (treeCutIndex!=ITEM_NONE)
// QDOAS ???        {
// QDOAS ???         if (treeNItem2Copy)
// QDOAS ???          {
// QDOAS ???           TREE_CollapseChildNodes(hwndTree,treeCutIndex);    // Delete 'physically' child nodes from tree by collapsing the parent node
// QDOAS ???           TREE_DeleteChildList(treeCutIndex);                // Delete the child nodes from the tree
// QDOAS ???           TREE_UpdateItem(hwndTree,treeCutIndex);            // update the parent node
// QDOAS ???          }
// QDOAS ???         else
// QDOAS ???          {
// QDOAS ???           if (TREE_itemList[treeCutIndex].hti)
// QDOAS ???            TreeView_DeleteItem(hwndTree,TREE_itemList[treeCutIndex].hti);
// QDOAS ???
// QDOAS ???           indexParent=TREE_DeleteOneItem(treeCutIndex);
// QDOAS ???           TREE_UpdateItem(hwndTree,indexParent);
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       if (!treeNItem2Copy)
// QDOAS ???        TreeView_EditLabel(hwndTree,TREE_itemList[indexNewItem].hti);
// QDOAS ???
// QDOAS ???       if (treeCutIndex!=ITEM_NONE)
// QDOAS ???        {
// QDOAS ???         TREE_typeOfItem2Copy=ITEM_NONE;
// QDOAS ???         treeCutIndex=ITEM_NONE;
// QDOAS ???         treeNItem2Copy=0;
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       if (!TLBAR_bSaveFlag)
// QDOAS ???        TLBAR_Enable(TRUE);
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // ===================
// QDOAS ??? // MESSAGES PROCESSING
// QDOAS ??? // ===================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_Create
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       create a new TreeView Control in the current MDI child window
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndParent : handle of the parent MDI window;
// QDOAS ??? //               indexTree  : entry point in the tree owned by the parent MDI window
// QDOAS ??? //
// QDOAS ??? // RETURN        the handle of the new TreeView Control
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? HWND TREE_Create(HWND hwndParent,INDEX indexTree)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   HWND hwndTree;    // handle of the tree view control to return
// QDOAS ???   RECT rc;          // Coordinates of the client area of the parent window
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   hwndTree=NULL;
// QDOAS ???
// QDOAS ???   // Get the client area of the parent window
// QDOAS ???
// QDOAS ???   if (hwndParent && (indexTree!=ITEM_NONE))
// QDOAS ???    {
// QDOAS ???     GetClientRect(hwndParent,&rc);
// QDOAS ???
// QDOAS ???     // Create the TreeView control
// QDOAS ???
// QDOAS ???     hwndTree = CreateWindowEx(0,                        // Ex style
// QDOAS ???                               WC_TREEVIEW,              // Class
// QDOAS ???                               "",                       // Dummy Text
// QDOAS ???                               TVS_HASLINES|             // use lines to show the hierarchy of items
// QDOAS ???                               TVS_HASBUTTONS|           // use +/- for expanding/collapsing lists of child
// QDOAS ???                               TVS_LINESATROOT|          // use lines to link items at the root of the list view
// QDOAS ???                               TVS_EDITLABELS|           // user is allowed to edit labels of list view items
// QDOAS ???                               WS_VISIBLE|
// QDOAS ???                               WS_CHILD|
// QDOAS ???                               WS_BORDER,
// QDOAS ???                               0,0,                      // Use all of
// QDOAS ???                               rc.right-rc.left,         // client area
// QDOAS ???                               rc.bottom-rc.top,
// QDOAS ???                               hwndParent,               // Parent
// QDOAS ???                               0,                        // ID
// QDOAS ???                               DOAS_hInst,               // Instance
// QDOAS ???                               NULL);                    // No extra
// QDOAS ???
// QDOAS ???     // Adds items into tree view
// QDOAS ???
// QDOAS ???     if (hwndTree)
// QDOAS ???      {
// QDOAS ???       // Attach the image list to the TreeView
// QDOAS ???
// QDOAS ???       if ((indexTree==CHILD_list[CHILD_WINDOW_PROJECT].itemTree) &&
// QDOAS ???          ((TREE_hImageList=TreeCreateImageList())!=NULL))
// QDOAS ???
// QDOAS ???        TreeView_SetImageList(hwndTree,TREE_hImageList,0);
// QDOAS ???
// QDOAS ???       TREE_ExpandOneNode(hwndTree,indexTree,TVI_ROOT);
// QDOAS ???       ShowWindow(hwndTree,SW_SHOW);
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return hwndTree;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_ReSize
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       resize the TreeView control onto its parent window
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndParent : handle of the parent MDI window;
// QDOAS ??? //               hwndTree   : handle of the TreeView control.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_ReSize(HWND hwndParent,HWND hwndTree)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   RECT rc;
// QDOAS ???
// QDOAS ???   // Resize
// QDOAS ???
// QDOAS ???   if ((hwndParent!=NULL) && (hwndTree!=NULL))
// QDOAS ???    {
// QDOAS ???     GetClientRect(hwndParent,&rc);                                    // Get parent coordinates
// QDOAS ???     MoveWindow(hwndTree,0,0,rc.right-rc.left,rc.bottom-rc.top,TRUE);  // Resize control
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_Context
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       context menus processing for TreeView controls
// QDOAS ??? //
// QDOAS ??? // INPUT         indexWindow : indew of the parent MDI window;
// QDOAS ??? //               mp2         : coordinates of the shortcut menu.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_Context(INDEX indexWindow,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   HWND          hwndParent,hwndTree;                // handles resp. of the parent MDI window and the owned TreeView control
// QDOAS ???   HMENU         hMenu;                              // context menu handle
// QDOAS ???   INDEX         indexItem,                          // index of the item that has the selection in the tree
// QDOAS ???                 indexParent,                        // index of parent
// QDOAS ???                 indexChild,                         // browse child nodes
// QDOAS ???                 dataIndex;                          // index in the objects list
// QDOAS ???   INT           contextMenu;                        // ID of the context menu associated to the selected item
// QDOAS ???   TREE_ITEM    *pTree,*pTreeParent;                 // pointers resp. to the item that has the selection and to one of the parent nodes
// QDOAS ???   MENUITEMINFO  menuItemInfo;                       // description of menu items
// QDOAS ???   UCHAR         menuItemText[MAX_ITEM_TEXT_LEN+1];  // text of the current menu item
// QDOAS ???   HMENU         hSubMenu;                           // handle of the current context menu
// QDOAS ???
// QDOAS ???   // ------------------------------------
// QDOAS ???   // Search for the selected item in tree
// QDOAS ???   // ------------------------------------
// QDOAS ???
// QDOAS ???   if ((indexWindow!=ITEM_NONE) && (indexWindow<MAX_MDI_WINDOWS) &&
// QDOAS ???      ((hwndParent=CHILD_list[indexWindow].hwndChild)!=NULL) &&
// QDOAS ???      ((hwndTree=CHILD_list[indexWindow].hwndTree)!=NULL) &&
// QDOAS ???      ((indexItem=TREE_GetSelectedItem(hwndTree))!=ITEM_NONE) &&
// QDOAS ???      ((contextMenu=TREE_itemType[TREE_itemList[indexItem].dataType].contextMenu)!=0))
// QDOAS ???    {
// QDOAS ???     pTree=&TREE_itemList[indexItem];
// QDOAS ???
// QDOAS ???     if ((hMenu=LoadMenu(DOAS_hInst,MAKEINTRESOURCE((RAW_GetFolderFlag(indexItem)==RAW_FOLDER_TYPE_USER)?MENU_CONTEXT_INSERT_FILES:contextMenu)))!=NULL)
// QDOAS ???      {
// QDOAS ???       // -------------------
// QDOAS ???       // Filter menu options
// QDOAS ???       // -------------------
// QDOAS ???
// QDOAS ???       // Paste
// QDOAS ???
// QDOAS ???       EnableMenuItem(hMenu,MENU_CONTEXT_PASTE,
// QDOAS ???                     (TREE_typeOfItem2Copy==TREE_itemType[pTree->dataType].childDataType)?MF_ENABLED:MF_GRAYED);
// QDOAS ???
// QDOAS ???       // Sort items
// QDOAS ???
// QDOAS ???       if ((RAW_GetFolderFlag(indexItem)==RAW_FOLDER_TYPE_USER) || (contextMenu==MENU_CONTEXT_INSERT) || (contextMenu==MENU_CONTEXT_INSERT_FILES))
// QDOAS ???        EnableMenuItem(hMenu,MENU_CONTEXT_SORT_NAME,MF_ENABLED);
// QDOAS ???
// QDOAS ???       // Children nodes processing
// QDOAS ???
// QDOAS ???       if (pTree->childNumber-pTree->childHidden>0)
// QDOAS ???        {
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_DELETEALL,MF_ENABLED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_EXPAND,MF_ENABLED);
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // Label edit
// QDOAS ???
// QDOAS ???       if (((pTree->dataType!=TREE_ITEM_TYPE_SITE_CHILDREN) &&                      // not an observation site
// QDOAS ???            (pTree->dataType!=TREE_ITEM_TYPE_CROSS_CHILDREN) &&                     // not a cross section symbol
// QDOAS ???            (pTree->dataType!=TREE_ITEM_TYPE_ANALYSIS_CHILDREN) &&                  // not an analysis window
// QDOAS ???            (pTree->dataType!=TREE_ITEM_TYPE_PROJECT) &&                            // not a project
// QDOAS ???            (RAW_GetFolderFlag(indexItem)!=RAW_FOLDER_TYPE_USER)) ||                // not a folder
// QDOAS ???            (pTree->useCount>0))
// QDOAS ???
// QDOAS ???        EnableMenuItem(hMenu,MENU_CONTEXT_RENAME,MF_GRAYED);
// QDOAS ???
// QDOAS ???       // Hide items
// QDOAS ???
// QDOAS ???       if (!pTree->useCount && ((pTree->dataType==TREE_ITEM_TYPE_ANALYSIS_CHILDREN) || (pTree->dataType==TREE_ITEM_TYPE_FILE_CHILDREN)))
// QDOAS ???        EnableMenuItem(hMenu,MENU_CONTEXT_HIDE,MF_ENABLED);
// QDOAS ???
// QDOAS ???       // Show hidden items
// QDOAS ???
// QDOAS ???       if (pTree->childHidden)
// QDOAS ???
// QDOAS ???        for (indexChild=pTree->firstChildItem;indexChild!=ITEM_NONE;indexChild=TREE_itemList[indexChild].nextItem)
// QDOAS ???         if (TREE_itemList[indexChild].hidden==2)
// QDOAS ???          {
// QDOAS ???           EnableMenuItem(hMenu,MENU_CONTEXT_SHOW,MF_ENABLED);
// QDOAS ???           break;
// QDOAS ???          }
// QDOAS ???
// QDOAS ???       // Properties
// QDOAS ???
// QDOAS ???       if (pTree->newItem)
// QDOAS ???        EnableMenuItem(hMenu,MENU_CONTEXT_PROPERTIES,MF_GRAYED);
// QDOAS ???
// QDOAS ???       // Delete items
// QDOAS ???
// QDOAS ???       if ((pTree->useCount>0) || (pTree->dataType==TREE_ITEM_TYPE_FILE_PARENT) ||
// QDOAS ???           (RAW_GetFolderFlag(pTree->parentItem)==RAW_FOLDER_TYPE_PATHSUB))
// QDOAS ???
// QDOAS ???        EnableMenuItem(hMenu,MENU_CONTEXT_DELETE,MF_GRAYED);
// QDOAS ???
// QDOAS ???       // -----------------------------------
// QDOAS ???       // Change the text of the context menu
// QDOAS ???       // -----------------------------------
// QDOAS ???
// QDOAS ???       menuItemInfo.cbSize=sizeof(MENUITEMINFO);                          // sizeof the structure with data on a menu item
// QDOAS ???       menuItemInfo.fMask=MIIM_TYPE|MIIM_SUBMENU;                         // process menu item by type
// QDOAS ???       menuItemInfo.dwTypeData=menuItemText;                              // buffer to receive menu item text
// QDOAS ???       menuItemInfo.cch=sizeof(menuItemText);                             // sizeof previous buffer
// QDOAS ???
// QDOAS ???       for (indexParent=pTree->parentItem;indexParent!=ITEM_NONE;indexParent=pTreeParent->parentItem)
// QDOAS ???        {
// QDOAS ???         pTreeParent=&TREE_itemList[indexParent];
// QDOAS ???
// QDOAS ???         if (pTreeParent->dataType==TREE_ITEM_TYPE_PROJECT)
// QDOAS ???          {
// QDOAS ???           if ((dataIndex=pTreeParent->dataIndex)!=ITEM_NONE)
// QDOAS ???            {
// QDOAS ???             if ((PRJCT_itemList[dataIndex].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
// QDOAS ???                  PRJCT_itemList[dataIndex].spectra.darkFlag)
// QDOAS ???              {
// QDOAS ???               hSubMenu=CreatePopupMenu();
// QDOAS ???
// QDOAS ???               if (PRJCT_itemList[dataIndex].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC)
// QDOAS ???                {
// QDOAS ???                 AppendMenu(hSubMenu,MF_STRING,MENU_CONTEXT_SPECTRA,"Spectra");
// QDOAS ???                 AppendMenu(hSubMenu,MF_STRING,MENU_CONTEXT_MFC_OFFSET,"Offset");
// QDOAS ???                 AppendMenu(hSubMenu,MF_STRING,MENU_CONTEXT_MFC_DARK,"Dark current");
// QDOAS ???                 AppendMenu(hSubMenu,MF_STRING,MENU_CONTEXT_MFC_INSTR,"Instr. function");
// QDOAS ???                }
// QDOAS ???               else
// QDOAS ???                {
// QDOAS ???                 AppendMenu(hSubMenu,MF_STRING,MENU_CONTEXT_SPECTRA,"Spectra");
// QDOAS ???                 AppendMenu(hSubMenu,MF_STRING,MENU_CONTEXT_DARK,"Dark current");
// QDOAS ???                }
// QDOAS ???
// QDOAS ???               GetMenuItemInfo(hMenu,MENU_CONTEXT_SPECTRA,FALSE,&menuItemInfo);      // Fill structure with the current attributes of the menu item
// QDOAS ???               strcpy(menuItemText,"Browse");                                        // Change text of the selected menu item
// QDOAS ???               menuItemInfo.hSubMenu=hSubMenu;
// QDOAS ???               SetMenuItemInfo(hMenu,MENU_CONTEXT_SPECTRA,FALSE,&menuItemInfo);      // Update menu item
// QDOAS ???              }
// QDOAS ???             else if ((PRJCT_itemList[dataIndex].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_HDF) ||
// QDOAS ???                      (PRJCT_itemList[dataIndex].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) ||
// QDOAS ???                      (PRJCT_itemList[dataIndex].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_ASCII) ||
// QDOAS ???                      (PRJCT_itemList[dataIndex].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GDP_BIN) ||
// QDOAS ???                      (PRJCT_itemList[dataIndex].instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2))
// QDOAS ???              {
// QDOAS ???               hSubMenu=CreatePopupMenu();
// QDOAS ???
// QDOAS ???               AppendMenu(hSubMenu,MF_STRING,MENU_CONTEXT_SPECTRA,"Spectra");
// QDOAS ???               AppendMenu(hSubMenu,MF_STRING,MENU_CONTEXT_DARK,"Irradiances");
// QDOAS ???               AppendMenu(hSubMenu,MF_STRING,MENU_CONTEXT_ERROR,"Errors");
// QDOAS ???
// QDOAS ???               GetMenuItemInfo(hMenu,MENU_CONTEXT_SPECTRA,FALSE,&menuItemInfo);  // Fill structure with the current attributes of the menu item
// QDOAS ???               strcpy(menuItemText,"Browse");                                    // Change text of the selected menu item
// QDOAS ???               menuItemInfo.hSubMenu=hSubMenu;
// QDOAS ???               SetMenuItemInfo(hMenu,MENU_CONTEXT_SPECTRA,FALSE,&menuItemInfo);  // Update menu item
// QDOAS ???              }
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           for (indexChild=pTreeParent->firstChildItem;indexChild!=ITEM_NONE;indexChild=pTree->nextItem)
// QDOAS ???            {
// QDOAS ???             pTree=&TREE_itemList[indexChild];
// QDOAS ???
// QDOAS ???             if ((pTree->dataType==TREE_ITEM_TYPE_ANALYSIS_PARENT) && (pTree->childNumber-pTree->childHidden<=0))
// QDOAS ???              EnableMenuItem(hMenu,MENU_CONTEXT_ANALYSIS,MF_GRAYED);
// QDOAS ???            }
// QDOAS ???
// QDOAS ???           break;
// QDOAS ???          }
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // -----------------
// QDOAS ???       // Thread processing
// QDOAS ???       // -----------------
// QDOAS ???
// QDOAS ???       if ((THRD_id!=THREAD_TYPE_NONE) && (indexWindow==CHILD_WINDOW_PROJECT))
// QDOAS ???        {
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_INSERT,(THRD_Context(indexItem,MENU_CONTEXT_INSERT))?MF_ENABLED:MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_FOLDER,(THRD_Context(indexItem,MENU_CONTEXT_INSERT))?MF_ENABLED:MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_PATH,(THRD_Context(indexItem,MENU_CONTEXT_INSERT))?MF_ENABLED:MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_SORT_NAME,(THRD_Context(indexItem,MENU_CONTEXT_INSERT))?MF_ENABLED:MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_DELETE,(THRD_Context(indexItem,MENU_CONTEXT_DELETE))?MF_ENABLED:MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_CUT,(THRD_Context(indexItem,MENU_CONTEXT_CUT))?MF_ENABLED:MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_COPY,(THRD_Context(indexItem,MENU_CONTEXT_COPY))?MF_ENABLED:MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_PASTE,(THRD_Context(indexItem,MENU_CONTEXT_PASTE))?MF_ENABLED:MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_DELETEALL,(THRD_Context(indexItem,MENU_CONTEXT_DELETEALL))?MF_ENABLED:MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_HIDE,(THRD_Context(indexItem,MENU_CONTEXT_HIDE))?MF_ENABLED:MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_SHOW,(THRD_Context(indexItem,MENU_CONTEXT_SHOW))?MF_ENABLED:MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_RENAME,(THRD_Context(indexItem,MENU_CONTEXT_RENAME))?MF_ENABLED:MF_GRAYED);
// QDOAS ???
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_SPECTRA,MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_DARK,MF_GRAYED);
// QDOAS ???
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_MFC_OFFSET,MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_MFC_DARK,MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_MFC_INSTR,MF_GRAYED);
// QDOAS ???
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_ANALYSIS,MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_KURUCZ,MF_GRAYED);
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_EXPORT,MF_GRAYED);       // use THRD_Browse spectra, so always grayed if a browsing spectra or an analysis is requested for any other file or project
// QDOAS ???         EnableMenuItem(hMenu,MENU_CONTEXT_CROSS,MF_GRAYED);
// QDOAS ???        }
// QDOAS ???
// QDOAS ???       // ----------
// QDOAS ???       // Track menu
// QDOAS ???       // ----------
// QDOAS ???
// QDOAS ???       TrackPopupMenu(GetSubMenu(hMenu,0),             // first item of the context menu
// QDOAS ???                      TPM_LEFTALIGN|TPM_LEFTBUTTON,    // screen position and mouse button flags
// QDOAS ???                      LOWORD(mp2),HIWORD(mp2),         // coordinates of the shortcut menu
// QDOAS ???                      0,                               // reserved
// QDOAS ???                      (HWND)hwndParent,                // handle of the parent window
// QDOAS ???                      NULL);                           // no-dismissal area
// QDOAS ???
// QDOAS ???       // ------------
// QDOAS ???       // Destroy menu
// QDOAS ???       // ------------
// QDOAS ???
// QDOAS ???       DestroyMenu(hMenu);
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif // __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? // ===================
// QDOAS ??? // RESOURCE MANAGEMENT
// QDOAS ??? // ===================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_Alloc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       allocation and initialization of a structured list for holding
// QDOAS ??? //               the tree objects
// QDOAS ??? //
// QDOAS ??? // RETURN        ERR_MEMORY_ALLOC_TREE on allocation error;
// QDOAS ??? //               NO_ERROR otherwise.
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC TREE_Alloc(void)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   TREE_ITEM *pTree;                      // the structured list for tree objects
// QDOAS ???   INDEX indexItem;                       // browse items in the previous list
// QDOAS ???   RC rc;                                 // return code
// QDOAS ???
// QDOAS ???   // Initialization
// QDOAS ???
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Structure allocation and filling
// QDOAS ???
// QDOAS ???   if ((TREE_itemList=(TREE_ITEM *)MEMORY_AllocBuffer("TREE_Alloc ","TREE_itemList",SYS_tree,sizeof(TREE_ITEM),0,MEMORY_TYPE_STRUCT))!=NULL)
// QDOAS ???    {
// QDOAS ???     // Initialization of the tree items
// QDOAS ???
// QDOAS ???     for (indexItem=0;indexItem<(int)SYS_tree;indexItem++)
// QDOAS ???      {
// QDOAS ???       pTree=&TREE_itemList[indexItem];
// QDOAS ???
// QDOAS ???       pTree->parentItem=ITEM_NONE;       // index of parent item
// QDOAS ???       pTree->firstChildItem=ITEM_NONE;   // index of the first child in list
// QDOAS ???       pTree->lastChildItem=ITEM_NONE;    // index of the last child in list
// QDOAS ???       pTree->prevItem=indexItem-1;       // index of previous item in list
// QDOAS ???       pTree->nextItem=indexItem+1;       // index of the next item in list
// QDOAS ???       #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???       pTree->hti=(HTREEITEM)0;           // handle of the item
// QDOAS ???       #endif
// QDOAS ???       pTree->useCount=                   // number of times item is referenced
// QDOAS ???       pTree->childNumber=                // total number of children
// QDOAS ???       pTree->childHidden=0;              // total number of hidden children
// QDOAS ???       pTree->hidden=0;
// QDOAS ???      }
// QDOAS ???
// QDOAS ???     pTree->nextItem=ITEM_NONE;
// QDOAS ???    }
// QDOAS ???   else
// QDOAS ???    rc=ERR_MEMORY_ALLOC_TREE;
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_Init
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       tree resources initialization
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_Init(void)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   INDEX indexRoot,    // entry point in the tree
// QDOAS ???         i;            // browse MDI child windows
// QDOAS ???
// QDOAS ???   // --------------------------------
// QDOAS ???   // MDI child windows initialization
// QDOAS ???   // --------------------------------
// QDOAS ???
// QDOAS ???   for (i=0;i<MAX_MDI_WINDOWS;i++)
// QDOAS ???    {
// QDOAS ???    	#if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???     CHILD_list[i].hwndChild=NULL;
// QDOAS ???     CHILD_list[i].indexMenuItem=ITEM_NONE;
// QDOAS ???     CHILD_list[i].hwndTree=NULL;
// QDOAS ???     #endif
// QDOAS ???     CHILD_list[i].itemTree=ITEM_NONE;
// QDOAS ???     CHILD_list[i].graphFlag=0;
// QDOAS ???     CHILD_list[i].openFlag=0;
// QDOAS ???     CHILD_list[i].rect.left=
// QDOAS ???     CHILD_list[i].rect.top=CW_USEDEFAULT;
// QDOAS ???     CHILD_list[i].rect.right=CHILD_DEFAULT_WIDTH;
// QDOAS ???     CHILD_list[i].rect.bottom=CHILD_DEFAULT_HEIGHT;
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   CHILD_list[CHILD_WINDOW_ENVIRONMENT].openFlag=
// QDOAS ???   CHILD_list[CHILD_WINDOW_PROJECT].openFlag=1;
// QDOAS ???
// QDOAS ???   TREE_itemType[TREE_ITEM_TYPE_PROJECT].dataMaxNumber=SYS_project;
// QDOAS ???   TREE_itemType[TREE_ITEM_TYPE_FILE_CHILDREN].dataMaxNumber=SYS_raw;
// QDOAS ???   TREE_itemType[TREE_ITEM_TYPE_ANALYSIS_CHILDREN].dataMaxNumber=SYS_anlys;
// QDOAS ???
// QDOAS ???   // ---------------------
// QDOAS ???   // Fill environment tree
// QDOAS ???   // ---------------------
// QDOAS ???
// QDOAS ???   CHILD_list[0].itemTree=indexRoot=TREE_InsertOneItem((HWND)NULL,"Environment",ITEM_NONE,TREE_ITEM_TYPE_NONE,0,0,0);
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   TREE_itemList[indexRoot].hti=TVI_ROOT;
// QDOAS ???   #endif
// QDOAS ???
// QDOAS ???   // Observation Sites
// QDOAS ???
// QDOAS ???   SITES_treeEntryPoint=TREE_InsertOneItem((HWND)NULL,"Observation Sites",indexRoot,TREE_ITEM_TYPE_SITE_PARENT,0,0,0);
// QDOAS ???
// QDOAS ???   // Symbols
// QDOAS ???
// QDOAS ???   SYMB_crossTreeEntryPoint=TREE_InsertOneItem((HWND)NULL,"User defined symbols",indexRoot,TREE_ITEM_TYPE_CROSS_PARENT,0,0,0);
// QDOAS ???   CHILD_list[1].itemTree=indexRoot=TREE_InsertOneItem((HWND)NULL,"Projects",ITEM_NONE,TREE_ITEM_TYPE_PROJECT_PARENT,0,0,0);
// QDOAS ???   #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???   TREE_itemList[indexRoot].hti=TVI_ROOT;
// QDOAS ???   #endif
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      TREE_Free
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       release the buffer allocated for the tree objects
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void TREE_Free(void)
// QDOAS ???  {
// QDOAS ???   if (TREE_itemList)
// QDOAS ???    MEMORY_ReleaseBuffer("TREE_Free ","TREE_itemList",TREE_itemList);
// QDOAS ???  }


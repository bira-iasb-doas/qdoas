#----------------------------------------------
# General Configuration
#----------------------------------------------

TEMPLATE = app
TARGET   = qdoas

include( ../config.pri )

CONFIG += qt thread $$CODE_GENERATION
QT = core gui xml


# Help system to use...
contains ( $$HELP_SYSTEM, assistant ) {
    CONFIG  += assistant
    DEFINES += HELP_QT_ASSISTANT
}

#----------------------------------------------
# Platform dependency ... based on ../config.pri
#----------------------------------------------

INCLUDEPATH  += $$QWT_INC_PATH

unix {
  INCLUDEPATH  += ../engine
  LIBS         += -L$$QWT_LIB_PATH -l$$QWT_LIB -lm
  QMAKE_LFLAGS += -Wl,-rpath=$$QWT_LIB_PATH
}

win32 {
  INCLUDEPATH  += ..\engine

  contains( $$QWT_LINKAGE, qwtstatic ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB
  }
  contains( CONFIG, qwtdll ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB
    DEFINES     += QWT_DLL
  }
  CONFIG      += windows
}

#----------------------------------------------
# GUI Source files
#----------------------------------------------
SOURCES += CEngineController.cpp
SOURCES += CEngineRequest.cpp
SOURCES += CEngineResponse.cpp
SOURCES += CEngineError.cpp
SOURCES += CEngineThread.cpp
SOURCES += CNavigationPanel.cpp
SOURCES += CPlotProperties.cpp
SOURCES += CPlotDataSet.cpp
SOURCES += CPlotPageData.cpp
SOURCES += CTablePageData.cpp
SOURCES += CMultiPageTableModel.cpp
SOURCES += CSession.cpp
SOURCES += CValidator.cpp
SOURCES += CConfigHandler.cpp
SOURCES += CQdoasConfigHandler.cpp
SOURCES += CPathSubHandler.cpp
SOURCES += CProjectConfigSubHandlers.cpp
SOURCES += CProjectConfigAnalysisWindowSubHandlers.cpp
SOURCES += CProjectConfigTreeNode.cpp
SOURCES += CProjectConfigItem.cpp
SOURCES += CConfigurationWriter.cpp
SOURCES += CWActiveContext.cpp
SOURCES += CDoasTable.cpp
SOURCES += CWAnalysisWindowDoasTables.cpp
SOURCES += CWEditor.cpp
SOURCES += CWMain.cpp
SOURCES += CWorkSpace.cpp
SOURCES += CWPlotPage.cpp
SOURCES += CWPlotRegion.cpp
SOURCES += CWTableRegion.cpp
SOURCES += CWSiteListCombo.cpp
SOURCES += CWProjectAnalysisWindowNameEditor.cpp
SOURCES += CWProjectDirectoryEditor.cpp
SOURCES += CWProjectFolderNameEditor.cpp
SOURCES += CWProjectNameEditor.cpp
SOURCES += CWProjectPropertyEditor.cpp
SOURCES += CWProjectTabDisplay.cpp
SOURCES += CWProjectTabSelection.cpp
SOURCES += CWProjectTabAnalysis.cpp
SOURCES += CWProjectTabFiltering.cpp
SOURCES += CWProjectTabCalibration.cpp
SOURCES += CWProjectTabUndersampling.cpp
SOURCES += CWProjectTabInstrumental.cpp
SOURCES += CWProjectTabSlit.cpp
SOURCES += CWProjectTabOutput.cpp
SOURCES += CWProjectTabNasaAmes.cpp
SOURCES += CWOutputSelector.cpp
SOURCES += CWProjectTree.cpp
SOURCES += CProjectTreeClipboard.cpp
SOURCES += CWAnalysisWindowPropertyEditor.cpp
SOURCES += CWSiteEditor.cpp
SOURCES += CWSiteTree.cpp
SOURCES += CWPlotPropertiesEditor.cpp
SOURCES += CWSplitter.cpp
SOURCES += CWSymbolEditor.cpp
SOURCES += CWUserSymbolTree.cpp
SOURCES += CWAboutDialog.cpp
SOURCES += CConfigStateMonitor.cpp
SOURCES += CPreferences.cpp
SOURCES += CHelpSystem.cpp
SOURCES += CHelpSystemImpl.cpp
SOURCES += qdoas.cpp

#----------------------------------------------
# Mediator Source files
#----------------------------------------------
SOURCES += mediate_response.cpp
SOURCES += mediate.c
SOURCES += mediate_project.c
SOURCES += mediate_analysis_window.c

#----------------------------------------------
# GUI Header files
#----------------------------------------------
HEADERS += CEngineController.h
HEADERS += CEngineRequest.h
HEADERS += CEngineResponse.h
HEADERS += CENgineError.h
HEADERS += CEngineThread.h
HEADERS += CNavigationPanel.h
HEADERS += CPlotProperties.h
HEADERS += CPlotDataSet.h
HEADERS += CPlotPageData.h
HEADERS += CTablePageData.h
HEADERS += CMultiPageTableModel.h
HEADERS += CSession.h
HEADERS += CValidator.h
HEADERS += CConfigHandler.h
HEADERS += CQdoasConfigHandler.h
HEADERS += CPathSubHandler.h
HEADERS += CProjectConfigSubHandlers.h
HEADERS += CProjectConfigAnalysisWindowSubHandlers.h
HEADERS += CProjectConfigTreeNode.h
HEADERS += CProjectConfigItem.h
HEADERS += CConfigurationWriter.h
HEADERS += CWActiveContext.h
HEADERS += CDoasTable.h
HEADERS += CWAnalysisWindowDoasTables.h
HEADERS += CWEditor.h
HEADERS += CWMain.h
HEADERS += CWorkSpace.h
HEADERS += CWPlotPage.h
HEADERS += CWPlotRegion.h
HEADERS += CWTableRegion.h
HEADERS += CWSiteListCombo.h
HEADERS += CWProjectAnalysisWindowNameEditor.h
HEADERS += CWProjectDirectoryEditor.h
HEADERS += CWProjectFolderNameEditor.h
HEADERS += CWProjectNameEditor.h
HEADERS += CWProjectPropertyEditor.h
HEADERS += CWProjectTabAnalysis.h
HEADERS += CWProjectTabDisplay.h
HEADERS += CWProjectTabSelection.h
HEADERS += CWProjectTabFiltering.h
HEADERS += CWProjectTabCalibration.h
HEADERS += CWProjectTabUndersampling.h
HEADERS += CWProjectTabInstrumental.h
HEADERS += CWProjectTabSlit.h
HEADERS += CWProjectTabOutput.h
HEADERS += CWProjectTabNasaAmes.h
HEADERS += CWOutputSelector.h
HEADERS += CWProjectTree.h
HEADERS += CProjectTreeClipboard.h
HEADERS += CWAnalysisWindowPropertyEditor.h
HEADERS += CWSiteEditor.h
HEADERS += CWSiteTree.h
HEADERS += CWPlotPropertiesEditor.h
HEADERS += CWSplitter.h
HEADERS += CWSymbolEditor.h
HEADERS += CWUserSymbolTree.h
HEADERS += CWAboutDialog.h
HEADERS += CConfigStateMonitor.h
HEADERS += debugutil.h
HEADERS += RefCountPtr.h
HEADERS += CPreferences.h
HEADERS += CHelpSystem.h
HEADERS += CHelpSystemImpl.h

#----------------------------------------------
# Mediator Header files
#----------------------------------------------
HEADERS += mediate.h
HEADERS += mediate_types.h
HEADERS += mediate_limits.h
HEADERS += mediate_general.h
HEADERS += mediate_response.h
HEADERS += mediate_request.h
HEADERS += mediate_project.h
HEADERS += mediate_analysis_window.h

#----------------------------------------------
# Reasource files
#----------------------------------------------
RESOURCES = qdoas.qrc


#----------------------------------------------
# Engine Source files
#----------------------------------------------
SOURCES += ../engine/actn-read.c
SOURCES += ../engine/analyse.c
SOURCES += ../engine/ascii.c
SOURCES += ../engine/bin_read.c
SOURCES += ../engine/ccd-read.c
SOURCES += ../engine/curfit.c
SOURCES += ../engine/debug.c
SOURCES += ../engine/easoeread.c
SOURCES += ../engine/engine.c
SOURCES += ../engine/erf.c
SOURCES += ../engine/error.c
SOURCES += ../engine/evalpoly.c
SOURCES += ../engine/filter.c
SOURCES += ../engine/fvoigt.c
SOURCES += ../engine/gdp_asc_read.c
SOURCES += ../engine/gdp_bin_read.c
SOURCES += ../engine/kurucz.c
SOURCES += ../engine/matrix.c
SOURCES += ../engine/memory.c
SOURCES += ../engine/mfc-read.c
SOURCES += ../engine/moon.c
SOURCES += ../engine/noaa-read.c
SOURCES += ../engine/output.c
SOURCES += ../engine/pda-read.c
SOURCES += ../engine/ras-read.c
SOURCES += ../engine/read1c_subs.c
SOURCES += ../engine/resource.c
SOURCES += ../engine/ring.c
SOURCES += ../engine/saoz-read.c
SOURCES += ../engine/scia-read.c
SOURCES += ../engine/spline.c
SOURCES += ../engine/stdfunc.c
SOURCES += ../engine/svd.c
SOURCES += ../engine/uoft-read.c
SOURCES += ../engine/usamp.c
SOURCES += ../engine/utc_string.c
SOURCES += ../engine/vector.c
SOURCES += ../engine/winfiles.c
SOURCES += ../engine/winpath.c
SOURCES += ../engine/winsites.c
SOURCES += ../engine/winsymb.c
SOURCES += ../engine/winthrd.c
SOURCES += ../engine/wintree.c
SOURCES += ../engine/wvlen_det_pix.c
SOURCES += ../engine/xsconv.c
SOURCES += ../engine/zenithal.c

#----------------------------------------------
# Engine Header files
#----------------------------------------------
HEADERS += ../engine/bin_read.h
HEADERS += ../engine/comdefs.h
HEADERS += ../engine/doas.h
HEADERS += ../engine/engine.h
HEADERS += ../engine/lv1_defs.h
HEADERS += ../engine/lv1_struct.h
HEADERS += ../engine/lv1c_struct.h
HEADERS += ../engine/read1c_defs.h
HEADERS += ../engine/utc_string.h
HEADERS += ../engine/windoas.h  

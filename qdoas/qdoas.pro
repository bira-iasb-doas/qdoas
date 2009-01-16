#----------------------------------------------
# Qdoas General Configuration
#----------------------------------------------

TEMPLATE = app
TARGET   = qdoas

include( ../config.pri )

CONFIG += qt thread $$CODE_GENERATION
QT = core gui xml

DEFINES += APP_QDOAS

# Help system to use...
contains ( HELP_SYSTEM, assistant ) {
    CONFIG  += assistant
    DEFINES += HELP_QT_ASSISTANT
}

#----------------------------------------------
# Platform dependency ... based on ../config.pri
#----------------------------------------------

INCLUDEPATH  += $$QWT_INC_PATH $$CODA_INC_PATH

unix {
  INCLUDEPATH  += ../mediator ../common ../engine
  LIBS         += -L$$QWT_LIB_PATH -l$$QWT_LIB -L$$CODA_LIB_PATH -lcoda -lm
  QMAKE_LFLAGS += -Wl,-rpath=$$QWT_LIB_PATH:$$CODA_LIB_PATH
}

win32 {
  INCLUDEPATH  += ..\mediator ..\common ..\engine

  contains( QWT_LINKAGE, qwtstatic ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB -L$$CODA_LIB_PATH -l$$CODA_LIB
  }
  contains( QWT_LINKAGE, qwtdll ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB$$QWT_LIB_VERSION -L$$CODA_LIB_PATH -l$$CODA_LIB
    DEFINES     += QWT_DLL
  }

  LIBS         += -L$$CODA_LIB_PATH -lcoda

  CONFIG      += windows
}

#----------------------------------------------
# GUI Source files
#----------------------------------------------

HEADERS += ../common/QdoasVersion.h
SOURCES += ../common/CValidator.cpp
SOURCES += ../common/CPreferences.cpp
SOURCES += ../common/CPlotDataSet.cpp
SOURCES += ../common/CPlotPageData.cpp
SOURCES += ../common/CTablePageData.cpp
SOURCES += ../common/CEngineError.cpp
SOURCES += ../common/CEngineController.cpp
SOURCES += ../common/CEngineResponse.cpp
SOURCES += ../common/CConfigHandler.cpp
SOURCES += ../common/CPathSubHandler.cpp
SOURCES += ../common/CConfigSubHandlerUtils.cpp
SOURCES += ../common/ConfigWriterUtils.cpp
SOURCES += ../common/CPathMgr.cpp
SOURCES += ../common/CScaleControl.cpp
SOURCES += ../common/CPlotProperties.cpp

SOURCES += ../common/CWAboutDialog.cpp
SOURCES += ../common/CHelpSystem.cpp
SOURCES += ../common/CHelpSystemImpl.cpp
SOURCES += ../common/CWSlitEditors.cpp
SOURCES += ../common/CWFilteringEditor.cpp
SOURCES += ../common/CWPlotPage.cpp
SOURCES += ../common/CWPlotPropertiesConfig.cpp

SOURCES += CQdoasEngineController.cpp
SOURCES += CEngineRequest.cpp
SOURCES += CEngineThread.cpp
SOURCES += CNavigationPanel.cpp
SOURCES += CMultiPageTableModel.cpp
SOURCES += CSession.cpp
SOURCES += CViewCrossSectionData.cpp
SOURCES += CQdoasConfigHandler.cpp
SOURCES += CProjectConfigSubHandlers.cpp
SOURCES += CProjectConfigAnalysisWindowSubHandlers.cpp
SOURCES += CProjectConfigTreeNode.cpp
SOURCES += CProjectConfigItem.cpp
SOURCES += CQdoasConfigWriter.cpp
SOURCES += CWActiveContext.cpp
SOURCES += CDoasTable.cpp
SOURCES += CWAnalysisWindowDoasTables.cpp
SOURCES += CWEditor.cpp
SOURCES += CWMain.cpp
SOURCES += CWorkSpace.cpp
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
SOURCES += CConfigStateMonitor.cpp
SOURCES += qdoas.cpp

#----------------------------------------------
# Mediator Source files
#----------------------------------------------
SOURCES += ../mediator/mediate_response.cpp
SOURCES += ../mediator/mediate.c
SOURCES += ../mediator/mediate_common.c
SOURCES += ../mediator/mediate_project.c
SOURCES += ../mediator/mediate_analysis_window.c

#----------------------------------------------
# GUI Header files
#----------------------------------------------
HEADERS += ../common/debugutil.h
HEADERS += ../common/RefCountPtr.h
HEADERS += ../common/CValidator.h
HEADERS += ../common/CPreferences.h
HEADERS += ../common/CPlotDataSet.h
HEADERS += ../common/CPlotPageData.h
HEADERS += ../common/CTablePageData.h
HEADERS += ../common/CEngineError.h
HEADERS += ../common/CEngineController.h
HEADERS += ../common/CEngineResponse.h
HEADERS += ../common/CConfigHandler.h
HEADERS += ../common/CPathSubHandler.h
HEADERS += ../common/CConfigSubHandlerUtils.h
HEADERS += ../common/ConfigWriterUtils.h
HEADERS += ../common/CPathMgr.h
HEADERS += ../common/CScaleControl.h
HEADERS += ../common/CPlotProperties.h

HEADERS += ../common/CWAboutDialog.h
HEADERS += ../common/CHelpSystem.h
HEADERS += ../common/CHelpSystemImpl.h
HEADERS += ../common/CWSlitEditors.h
HEADERS += ../common/CWFilteringEditor.h
HEADERS += ../common/CWPlotPage.h
HEADERS += ../common/CWPlotPropertiesConfig.h

HEADERS += CQdoasEngineController.h
HEADERS += CEngineRequest.h
HEADERS += CEngineThread.h
HEADERS += CNavigationPanel.h
HEADERS += CMultiPageTableModel.h
HEADERS += CSession.h
HEADERS += CViewCrossSectionData.h
HEADERS += CQdoasConfigHandler.h
HEADERS += CProjectConfigSubHandlers.h
HEADERS += CProjectConfigAnalysisWindowSubHandlers.h
HEADERS += CProjectConfigTreeNode.h
HEADERS += CProjectConfigItem.h
HEADERS += CQdoasConfigWriter.h
HEADERS += CWActiveContext.h
HEADERS += CDoasTable.h
HEADERS += CWAnalysisWindowDoasTables.h
HEADERS += CWEditor.h
HEADERS += CWMain.h
HEADERS += CWorkSpace.h
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
HEADERS += CConfigStateMonitor.h

#----------------------------------------------
# Mediator Header files
#----------------------------------------------
HEADERS += ../mediator/mediate.h
HEADERS += ../mediator/mediate_types.h
HEADERS += ../mediator/mediate_limits.h
HEADERS += ../mediator/mediate_common.h
HEADERS += ../mediator/mediate_general.h
HEADERS += ../mediator/mediate_response.h
HEADERS += ../mediator/mediate_request.h
HEADERS += ../mediator/mediate_project.h
HEADERS += ../mediator/mediate_analysis_window.h

#----------------------------------------------
# Reasource files
#----------------------------------------------
RESOURCES = ../resources/qdoas.qrc


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
SOURCES += ../engine/gome2_read.c
SOURCES += ../engine/kurucz.c
SOURCES += ../engine/matrix.c
SOURCES += ../engine/memory.c
SOURCES += ../engine/mfc-read.c
SOURCES += ../engine/mkzypack-read.c
SOURCES += ../engine/moon.c
SOURCES += ../engine/noaa-read.c
SOURCES += ../engine/output.c
SOURCES += ../engine/pda-read.c
SOURCES += ../engine/ras-read.c
SOURCES += ../engine/resource.c
SOURCES += ../engine/saoz-read.c
SOURCES += ../engine/scia_common.c
SOURCES += ../engine/scia_l1c.c
SOURCES += ../engine/scia_l1c_lib.c
SOURCES += ../engine/scia-read.c
SOURCES += ../engine/spline.c
SOURCES += ../engine/stdfunc.c
SOURCES += ../engine/svd.c
SOURCES += ../engine/uoft-read.c
SOURCES += ../engine/usamp.c
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
HEADERS += ../engine/scia_common.h
HEADERS += ../engine/scia_defs.h
HEADERS += ../engine/scia_l1c.h
HEADERS += ../engine/scia_l1c_lib.h
HEADERS += ../engine/windoas.h
HEADERS += ../engine/constants.h

#----------------------------------------------
# Install
#----------------------------------------------

target.path = $${INSTALL_PREFIX}/bin
doc.path    = $${INSTALL_PREFIX}/doc

doc.files = ../../Doc/html ../../Doc/RELEASENOTES ../../LICENSE

INSTALLS += target doc

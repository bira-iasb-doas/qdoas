#----------------------------------------------
# General Configuration
#----------------------------------------------

TEMPLATE = app
TARGET   = qdoas

unix {
  CONFIG += qt thread debug
  QT = core gui xml

  # for trace-write debugging ...
  DEFINES += DEBUG
  DEFINES += LVL4
}
win32 {
  CONFIG += qt thread release
  QT = core gui xml
}

#----------------------------------------------
# Platform dependency ...
#----------------------------------------------

unix {
  INCLUDEPATH  += /usr/local/qwt/include ../engine
  LIBS         += -L/usr/local/qwt/lib -lqwt -lm
  QMAKE_LFLAGS += -Wl,-rpath=/usr/local/qwt/lib
}

win32 {
  INCLUDEPATH += C:\Qwt\include
  INCLUDEPATH += ..\engine
  LIBS        += -L"C:\Qwt\lib" -lqwt5
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
SOURCES += CPlotDataSet.cpp
SOURCES += CPlotPageData.cpp
SOURCES += CTablePageData.cpp
SOURCES += CMultiPageTableModel.cpp
SOURCES += CSession.cpp
SOURCES += CValidator.cpp
SOURCES += CQdoasProjectConfigHandler.cpp
SOURCES += CProjectConfigSubHandlers.cpp
SOURCES += CProjectConfigTreeNode.cpp
SOURCES += CProjectConfigItem.cpp
SOURCES += CConfigurationWriter.cpp
SOURCES += CWActiveContext.cpp
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
SOURCES += CWProjectTabAnalysis.cpp
SOURCES += CWProjectTabSpectra.cpp
SOURCES += CWProjectTabFiltering.cpp
SOURCES += CWProjectTabCalibration.cpp
SOURCES += CWProjectTabUndersampling.cpp
SOURCES += CWProjectTabInstrumental.cpp
SOURCES += CWProjectTree.cpp
SOURCES += CWSiteEditor.cpp
SOURCES += CWSiteTree.cpp
SOURCES += CWSplitter.cpp
SOURCES += CWUserSymbolTree.cpp
SOURCES += CPreferences.cpp
SOURCES += qdoas.cpp
#----------------------------------------------
# Mediator Source files
#----------------------------------------------
SOURCES += mediate_response.cpp
SOURCES += mediate.c
SOURCES += mediate_project.c

#----------------------------------------------
# GUI Header files
#----------------------------------------------
HEADERS += CEngineController.h
HEADERS += CEngineRequest.h
HEADERS += CEngineResponse.h
HEADERS += CENgineError.h
HEADERS += CEngineThread.h
HEADERS += CNavigationPanel.h
HEADERS += CPlotDataSet.h
HEADERS += CPlotPageData.h
HEADERS += CTablePageData.h
HEADERS += CMultiPageTableModel.h
HEADERS += CSession.h
HEADERS += CValidator.h
HEADERS += CQdoasProjectConfigHandler.h
HEADERS += CProjectConfigSubHandlers.h
HEADERS += CProjectConfigTreeNode.h
HEADERS += CProjectConfigItem.h
HEADERS += CConfigurationWriter.h
HEADERS += CWActiveContext.h
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
HEADERS += CWProjectTabSpectra.h
HEADERS += CWProjectTabFiltering.h
HEADERS += CWProjectTabCalibration.h
HEADERS += CWProjectTabUndersampling.h
HEADERS += CWProjectTabInstrumental.h
HEADERS += CWProjectTree.h
HEADERS += CWSiteEditor.h
HEADERS += CWSiteTree.h
HEADERS += CWSplitter.h
HEADERS += CWUserSymbolTree.h
HEADERS += debugutil.h
HEADERS += RefCountPtr.h
HEADERS += CPreferences.h

#----------------------------------------------
# Mediator Header files
#----------------------------------------------
HEADERS += mediate.h
HEADERS += mediate_types.h
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
SOURCES += ../engine/opus-read.c
SOURCES += ../engine/output.c
SOURCES += ../engine/pda-read.c
SOURCES += ../engine/ras-read.c
SOURCES += ../engine/read1c_subs.c
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
SOURCES += ../engine/winanlys.c
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

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
  INCLUDEPATH  += /usr/local/qwt/include
  LIBS         += -L/usr/local/qwt/lib -lqwt -lm
  QMAKE_LFLAGS += -Wl,-rpath=/usr/local/qwt/lib
}

win32 {
  INCLUDEPATH += C:\Qwt\include
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
SOURCES += CWActiveContext.cpp
SOURCES += CWEditor.cpp
SOURCES += CWMain.cpp
SOURCES += CWorkSpace.cpp
SOURCES += CWPlotPage.cpp
SOURCES += CWPlotRegion.cpp
SOURCES += CWTableRegion.cpp
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
SOURCES += CWProjectTree.cpp
SOURCES += CWSiteEditor.cpp
SOURCES += CWSiteTree.cpp
SOURCES += CWSplitter.cpp
SOURCES += CWUserSymbolTree.cpp
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
HEADERS += CWActiveContext.h
HEADERS += CWEditor.h
HEADERS += CWMain.h
HEADERS += CWorkSpace.h
HEADERS += CWPlotPage.h
HEADERS += CWPlotRegion.h
HEADERS += CWTableRegion.h
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
HEADERS += CWProjectTree.h
HEADERS += CWSiteEditor.h
HEADERS += CWSiteTree.h
HEADERS += CWSplitter.h
HEADERS += CWUserSymbolTree.h
HEADERS += debugutil.h
HEADERS += RefCountPtr.h

#----------------------------------------------
# Mediator Header files
#----------------------------------------------
HEADERS += mediate.h
HEADERS += mediate_project.h
HEADERS += mediate_analysis_window.h

#----------------------------------------------
# Reasource files
#----------------------------------------------
RESOURCES = qdoas.qrc



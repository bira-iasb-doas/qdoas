#----------------------------------------------
# Qdoas General Configuration
#----------------------------------------------

TEMPLATE = app
TARGET   = ../../qdoas/release/qdoas

include( ../config.pri )
PRE_TARGETDEPS += ../common/libcommon.a ../engine/libengine.a ../mediator/libmediator.a

CONFIG += qt thread $$CODE_GENERATION
QT = core gui xml

INCLUDEPATH  += ../mediator ../common ../engine

DEFINES += APP_QDOAS

#----------------------------------------------
# Platform dependency ... based on ../config.pri
#----------------------------------------------

unix {
  LIBS += -lcoda -lhdfeos -lmfhdf -ldf -lz -ljpeg -lhe5_hdfeos -lhdf5 -lhdf5_hl -lqwt
}

linux_package {
  TARGET = ../../linux_package/qdoas.bin
  LIBS += -lcoda -lhdfeos -lmfhdf -ldf -lz -lhe5_hdfeos -lhdf5_hl -lhdf5 -lqwt
}

mxe {
  LIBS += -Wl,-Bstatic -lcoda -lhdfeos -lmfhdf -ldf -lz -ljpeg -lhe5_hdfeos -lhdf5_hl -lhdf5  -lqwt
  LIBS += -lportablexdr
}

win32 {

  INCLUDEPATH  += ../mediator ../common ../engine $$QWT_INC_PATH $$CODA_INC_PATH $$HDF_INC_PATH $$HDFEOS_INC_PATH

  contains( QWT_LINKAGE, qwtstatic ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB
  }
  contains( QWT_LINKAGE, qwtdll ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB$$QWT_LIB_VERSION
    DEFINES     += QWT_DLL
  }

  LIBS         += -L$$CODA_LIB_PATH -lcoda -L$$HDFEOS_LIB_PATH -lhdf -L$$HDFEOS_LIB_PATH -lmfhdf -L$$HDFEOS_LIB_PATH -lhdf5 -L$$HDFEOS_LIB_PATH -lhdfeos -L$$HDFEOS_LIB_PATH -lhdfeos5 -lm

  CONFIG      += windows
}

#----------------------------------------------
# GUI Source files
#----------------------------------------------

HEADERS += ../common/QdoasVersion.h
SOURCES += ../common/CPreferences.cpp

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
# GUI Header files
#----------------------------------------------

HEADERS += ../common/CPreferences.h

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
# Reasource files
#----------------------------------------------
RESOURCES = ../resources/qdoas.qrc


#----------------------------------------------
# Engine Source files
#----------------------------------------------

#----------------------------------------------
# Install
#----------------------------------------------

target.path = $${INSTALL_PREFIX}/bin
doc.path    = $${INSTALL_PREFIX}/doc/qdoas

doc.files = ./Help ../../Doc/QDOAS_ReleaseNotes.doc ../../LICENSE

INSTALLS += target doc

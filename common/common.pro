TEMPLATE = lib

include( ../config.pri )

CONFIG += staticlib qt thread $$CODE_GENERATION
QT = core gui xml

SOURCES       = CConfigHandler.cpp \
		CConfigSubHandlerUtils.cpp \
		CEngineController.cpp \
		CEngineError.cpp \
		CEngineResponse.cpp \
		CHelpSystem.cpp \
		ConfigWriterUtils.cpp \
		CPathMgr.cpp \
		CPathSubHandler.cpp \
		CPlotDataSet.cpp \
		CPlotPageData.cpp \
		CPlotProperties.cpp \
		CScaleControl.cpp \
		CTablePageData.cpp \
		CValidator.cpp \
		CWAboutDialog.cpp \
		CWFilteringEditor.cpp \
		CWPlotArea.cpp \
		CWPlotPage.cpp \
		CWPlotPropertiesConfig.cpp \
		CWPlotPropertiesDialog.cpp \
		CWSlitEditors.cpp

HEADERS += *.h

INCLUDEPATH  += ../mediator ../common ../engine

unix {
  DEFINES += QDOAS_HELP_PATH=\"\\\"$${INSTALL_PREFIX}/doc/qdoas/Help\"\\\"
}

win32 {
  INCLUDEPATH  +=  $$QWT_INC_PATH $$CODA_INC_PATH $$HDF_INC_PATH $$HDFEOS_INC_PATH
  TARGET   = ../common

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

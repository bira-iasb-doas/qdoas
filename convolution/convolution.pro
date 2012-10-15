#----------------------------------------------
# Convolution Tool General Configuration
#----------------------------------------------

TEMPLATE = app
TARGET   = ../../qdoas/release/convolution

include( ../config.pri )

CONFIG += qt thread $$CODE_GENERATION
QT = core gui xml

DEFINES += APP_CONV

# Help system to use...
contains ( HELP_SYSTEM, assistant ) {
    CONFIG  += assistant
    DEFINES += HELP_QT_ASSISTANT
}

#----------------------------------------------
# Platform dependency ...
#----------------------------------------------

INCLUDEPATH  += $$QWT_INC_PATH

unix {
  INCLUDEPATH  += ../mediator ../common ../engine
  LIBS         += -L$$QWT_LIB_PATH -l$$QWT_LIB -lm
  QMAKE_LFLAGS += -Wl,-rpath=$$QWT_LIB_PATH
}

win32 {
  INCLUDEPATH  += ../mediator ../common ../engine

  contains( QWT_LINKAGE, qwtstatic ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB
  }
  contains( QWT_LINKAGE, qwtdll ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB$$QWT_LIB_VERSION
    DEFINES     += QWT_DLL
  }

  CONFIG      += windows
}

#----------------------------------------------
# GUI Source files
#----------------------------------------------

HEADERS += ../common/QdoasVersion.h
SOURCES += ../common/CPreferences.cpp
SOURCES += ../common/CValidator.cpp
SOURCES += ../common/CPlotDataSet.cpp
SOURCES += ../common/CPlotPageData.cpp
SOURCES += ../common/CTablePageData.cpp
SOURCES += ../common/CEngineError.cpp
SOURCES += ../common/CEngineResponse.cpp
SOURCES += ../common/CEngineController.cpp
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
SOURCES += ../common/CWPlotArea.cpp
SOURCES += ../common/CWPlotPropertiesConfig.cpp
SOURCES += ../common/CWPlotPropertiesDialog.cpp

SOURCES += CWMain.cpp
SOURCES += CWConvTabGeneral.cpp
SOURCES += CWConvTabSlit.cpp
SOURCES += CConvEngineController.cpp
SOURCES += CConvConfigHandler.cpp
SOURCES += CConvConfigWriter.cpp
SOURCES += convolution.cpp

#----------------------------------------------
# GUI Header files
#----------------------------------------------
HEADERS += ../common/CPreferences.h
HEADERS += ../common/CValidator.h
HEADERS += ../common/CPlotDataSet.h
HEADERS += ../common/CPlotPageData.h
HEADERS += ../common/CTablePageData.h
HEADERS += ../common/CEngineError.h
HEADERS += ../common/CEngineResponse.h
HEADERS += ../common/CEngineController.h
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
HEADERS += ../common/CWPlotArea.h
HEADERS += ../common/CWPlotPropertiesConfig.h
HEADERS += ../common/CWPlotPropertiesDialog.h

HEADERS += CWMain.h
HEADERS += CWConvTabGeneral.h
HEADERS += CWConvTabSlit.h
HEADERS += CConvEngineController.h
HEADERS += CConvConfigHandler.h
HEADERS += CConvConfigWriter.h

#----------------------------------------------
# Mediator Source files
#----------------------------------------------
SOURCES += ../mediator/mediate_convolution.c
SOURCES += ../mediator/mediate_xsconv.c
SOURCES += ../mediator/mediate_common.c
SOURCES += ../mediator/mediate_response.cpp

#----------------------------------------------
# Mediator Header files
#----------------------------------------------
HEADERS += ../mediator/mediate_limits.h
HEADERS += ../mediator/mediate_general.h
HEADERS += ../mediator/mediate_convolution.h
HEADERS += ../mediator/mediate_response.h
HEADERS += ../mediator/mediate_xsconv.h
HEADERS += ../mediator/mediate_common.h
HEADERS += ../mediator/mediate_project.h
HEADERS += ../mediator/mediate_analysis_window.h

#----------------------------------------------
# Resource files
#----------------------------------------------
RESOURCES = ../resources/convolution.qrc

#----------------------------------------------
# Engine Source files
#----------------------------------------------
SOURCES += ../engine/debug.c
SOURCES += ../engine/engine_xsconv.c
SOURCES += ../engine/erf.c
SOURCES += ../engine/error.c
SOURCES += ../engine/evalpoly.c
SOURCES += ../engine/filter.c
SOURCES += ../engine/fvoigt.c
SOURCES += ../engine/matrix.c
SOURCES += ../engine/memory.c
SOURCES += ../engine/spline.c
SOURCES += ../engine/stdfunc.c
SOURCES += ../engine/svd.c
SOURCES += ../engine/ring.c
SOURCES += ../engine/usamp.c
SOURCES += ../engine/vector.c
SOURCES += ../engine/winfiles.c
SOURCES += ../engine/xsconv.c
SOURCES += ../engine/spectral_range.c

#----------------------------------------------
# Engine Header files
#----------------------------------------------
HEADERS += ../engine/bin_read.h
HEADERS += ../engine/comdefs.h
HEADERS += ../engine/doas.h
HEADERS += ../engine/engine_xsconv.h
HEADERS += ../engine/engine_common.h
HEADERS += ../engine/constants.h
HEADERS += ../engine/windoas.h
HEADERS += ../engine/spectral_range.h

#----------------------------------------------
# Install
#----------------------------------------------

target.path = $${INSTALL_PREFIX}/bin
INSTALLS += target

#----------------------------------------------
# General Configuration
#----------------------------------------------

TEMPLATE = app
TARGET   = convolution

include( ../config.pri )

CONFIG += qt thread $$CODE_GENERATION
QT = core gui xml

# Help system to use...
contains ( $$HELP_SYSTEM, assistant ) {
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
  INCLUDEPATH  += ..\mediator ..\common ..\engine

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
SOURCES += ../common/CPreferences.cpp
SOURCES += ../common/CValidator.cpp
SOURCES += ../common/CPlotDataSet.cpp
SOURCES += ../common/CPlotPageData.cpp
SOURCES += ../common/CTablePageData.cpp
SOURCES += ../common/CEngineError.cpp
SOURCES += ../common/CEngineResponse.cpp
SOURCES += ../common/CEngineController.cpp

SOURCES += ../common/CWAboutDialog.cpp
SOURCES += ../common/CHelpSystem.cpp
SOURCES += ../common/CHelpSystemImpl.cpp
SOURCES += ../common/CWSlitEditors.cpp

SOURCES += CWMain.cpp
SOURCES += CWConvTabGeneral.cpp
SOURCES += CWConvTabSlit.cpp
SOURCES += CConvEngineController.cpp
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

HEADERS += ../common/CWAboutDialog.h
HEADERS += ../common/CHelpSystem.h
HEADERS += ../common/CHelpSystemImpl.h
HEADERS += ../common/CWSlitEditors.h

HEADERS += CWMain.h
HEADERS += CWConvTabGeneral.h
HEADERS += CWConvTabSlit.h
HEADERS += CConvEngineController.h

#----------------------------------------------
# Mediator Source files
#----------------------------------------------
SOURCES += ../mediator/mediate_convolution.c
SOURCES += ../mediator/mediate.c
SOURCES += ../mediator/mediate_response.cpp

#----------------------------------------------
# Mediator Header files
#----------------------------------------------
HEADERS += ../mediator/mediate_limits.h
HEADERS += ../mediator/mediate_general.h
HEADERS += ../mediator/mediate_convolution.h
HEADERS += ../mediator/mediate_response.h
HEADERS += ../mediator/mediate_request.h
HEADERS += ../mediator/mediate_project.h
HEADERS += ../mediator/mediate_analysis_window.h

#----------------------------------------------
# Resource files
#----------------------------------------------
RESOURCES = ../resources/convolution.qrc

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

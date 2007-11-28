#----------------------------------------------
# CmdLine General Configuration
#----------------------------------------------

TEMPLATE = app
TARGET   = doas_cl

include( ../config.pri )

CONFIG += qt thread $$CODE_GENERATION
QT = core xml

#----------------------------------------------
# Platform dependency ... based on ../config.pri
#----------------------------------------------

INCLUDEPATH  += $$QWT_INC_PATH

unix {
  INCLUDEPATH  += ../mediator ../common ../qdoas ../convolution ../ring ../usamp ../engine
  LIBS         += -L$$QWT_LIB_PATH -l$$QWT_LIB -lm
  QMAKE_LFLAGS += -Wl,-rpath=$$QWT_LIB_PATH
}

win32 {
  INCLUDEPATH  += ..\mediator ..\common ..\qdoas ..\convolution ..\ring ..\usamp ..\engine

  CONFIG      += windows
}

#----------------------------------------------
# Source files
#----------------------------------------------

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

SOURCES += CBatchEngineController.cpp
SOURCES += ../qdoas/CSession.cpp
SOURCES += ../qdoas/CQdoasConfigHandler.cpp
SOURCES += ../qdoas/CProjectConfigSubHandlers.cpp
SOURCES += ../qdoas/CProjectConfigAnalysisWindowSubHandlers.cpp
SOURCES += ../qdoas/CProjectConfigTreeNode.cpp
SOURCES += ../qdoas/CProjectConfigItem.cpp
SOURCES += ../qdoas/CWorkSpace.cpp
SOURCES += cmdline.cpp

#----------------------------------------------
# Mediator Source files
#----------------------------------------------
SOURCES += ../mediator/mediate_response.cpp
SOURCES += ../mediator/mediate.c
SOURCES += ../mediator/mediate_project.c
SOURCES += ../mediator/mediate_analysis_window.c

#----------------------------------------------
# Header files
#----------------------------------------------
HEADERS += ../common/debugutil.h
HEADERS += ../common/QdoasVersion.h
HEADERS += ../common/RefCountPtr.h
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

HEADERS += CBatchEngineController.h
HEADERS += ../qdoas/CEngineRequest.h
HEADERS += ../qdoas/CSession.h
HEADERS += ../qdoas/CQdoasConfigHandler.h
HEADERS += ../qdoas/CProjectConfigSubHandlers.h
HEADERS += ../qdoas/CProjectConfigAnalysisWindowSubHandlers.h
HEADERS += ../qdoas/CProjectConfigTreeNode.h
HEADERS += ../qdoas/CProjectConfigItem.h
HEADERS += ../qdoas/CWorkSpace.h

#----------------------------------------------
# Mediator Header files
#----------------------------------------------
HEADERS += ../mediator/mediate.h
HEADERS += ../mediator/mediate_types.h
HEADERS += ../mediator/mediate_limits.h
HEADERS += ../mediator/mediate_general.h
HEADERS += ../mediator/mediate_response.h
HEADERS += ../mediator/mediate_request.h
HEADERS += ../mediator/mediate_project.h
HEADERS += ../mediator/mediate_analysis_window.h

#----------------------------------------------
# Reasource files
#----------------------------------------------


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

#----------------------------------------------
# CmdLine General Configuration
#----------------------------------------------

TEMPLATE = app
TARGET   = ../../qdoas/release/doas_cl

include( ../config.pri )
PRE_TARGETDEPS += ../common/libcommon.a ../engine/libengine.a ../mediator/libmediator.a

CONFIG += qt thread $$CODE_GENERATION
QT = core xml

#----------------------------------------------
# Platform dependency ... based on ../config.pri
#----------------------------------------------

unix {
  INCLUDEPATH  += ../mediator ../common ../qdoas ../convolution ../usamp ../engine ../ring
  LIBS         += -lcoda -lhdfeos -lmfhdf -ldf -lz -ljpeg -lhe5_hdfeos -lhdf5 -lhdf5_hl
}

win32 {
  INCLUDEPATH  += $$QWT_INC_PATH $$CODA_INC_PATH $$HDF_INC_PATH $$HDFEOS_INC_PATH
  INCLUDEPATH  += ../mediator ../common ../qdoas ../convolution ../usamp ../engine ../ring

  contains( QWT_LINKAGE, qwtstatic ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB
  }
  contains( QWT_LINKAGE, qwtdll ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB$$QWT_LIB_VERSION
    DEFINES     += QWT_DLL
  }

  LIBS         += -L$$CODA_LIB_PATH -lcoda -L$$HDFEOS_LIB_PATH -lhdf -L$$HDFEOS_LIB_PATH -lmfhdf -L$$HDFEOS_LIB_PATH -lhdf5 -L$$HDFEOS_LIB_PATH -lhdfeos -L$$HDFEOS_LIB_PATH -lhdfeos5 -lm

  CONFIG      += console
}

#----------------------------------------------
# Source files
#----------------------------------------------

SOURCES += CBatchEngineController.cpp
SOURCES += ../qdoas/CQdoasConfigHandler.cpp
SOURCES += ../qdoas/CProjectConfigSubHandlers.cpp
SOURCES += ../qdoas/CProjectConfigAnalysisWindowSubHandlers.cpp
SOURCES += ../qdoas/CProjectConfigTreeNode.cpp
SOURCES += ../qdoas/CProjectConfigItem.cpp
SOURCES += ../qdoas/CWorkSpace.cpp

SOURCES += ../convolution/CConvConfigHandler.cpp
SOURCES += ../ring/CRingConfigHandler.cpp
SOURCES += ../usamp/CUsampConfigHandler.cpp

SOURCES += cmdline.cpp
SOURCES += convxml.cpp
SOURCES += qdoasxml.cpp

HEADERS += CBatchEngineController.h
HEADERS += convxml.h
HEADERS += qdoasxml.h
HEADERS += ../qdoas/CEngineRequest.h
HEADERS += ../qdoas/CQdoasConfigHandler.h
HEADERS += ../qdoas/CProjectConfigSubHandlers.h
HEADERS += ../qdoas/CProjectConfigAnalysisWindowSubHandlers.h
HEADERS += ../qdoas/CProjectConfigTreeNode.h
HEADERS += ../qdoas/CProjectConfigItem.h
HEADERS += ../qdoas/CWorkSpace.h

HEADERS += ../convolution/CConvConfigHandler.h
HEADERS += ../ring/CRingConfigHandler.h
HEADERS += ../usamp/CUsampConfigHandler.h

#----------------------------------------------
# Install
#----------------------------------------------

target.path = $${INSTALL_PREFIX}/bin

INSTALLS += target

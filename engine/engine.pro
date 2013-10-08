TEMPLATE = lib

include( ../config.pri )

CONFIG += staticlib

SOURCES += *.c
HEADERS += *.h

DEPENDPATH += ../mediator
INCLUDEPATH += ../mediator ../common

win32 {
  INCLUDEPATH  += ../engine $$QWT_INC_PATH $$CODA_INC_PATH $$HDF_INC_PATH $$HDFEOS_INC_PATH
  TARGET   = ../engine

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

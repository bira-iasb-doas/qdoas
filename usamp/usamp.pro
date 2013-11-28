#----------------------------------------------
# Usamp Tool General Configuration
#----------------------------------------------

TEMPLATE = app
TARGET   = ../../qdoas/release/usamp

include( ../config.pri )

CONFIG += qt thread $$CODE_GENERATION
QT = core gui xml
PRE_TARGETDEPS += ../common/libcommon.a ../engine/libengine.a ../mediator/libmediator.a

DEFINES += APP_USAMP

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
  LIBS += -lqwt
}

linux_package {
  TARGET = ../../linux_package/usamp.bin
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

SOURCES += ../common/CPreferences.cpp

SOURCES += CWMain.cpp
SOURCES += CWUsampTabGeneral.cpp
SOURCES += CUsampEngineController.cpp
SOURCES += CUsampConfigHandler.cpp
SOURCES += CUsampConfigWriter.cpp
SOURCES += usamptool.cpp

#----------------------------------------------
# GUI Header files
#----------------------------------------------

HEADERS += ../common/CPreferences.h

HEADERS += CWMain.h
HEADERS += CWUsampTabGeneral.h
HEADERS += CUsampEngineController.h
HEADERS += CUsampConfigHandler.h
HEADERS += CUsampConfigWriter.h

#----------------------------------------------
# Resource files
#----------------------------------------------
RESOURCES = ../resources/usamp.qrc

#----------------------------------------------
# Install
#----------------------------------------------

target.path = $${INSTALL_PREFIX}/bin
INSTALLS += target

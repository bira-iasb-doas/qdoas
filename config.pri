#----------------------------------------------
# General Configuration for ALL applications
#----------------------------------------------

QWT_LINKAGE     = qwtdll        # qwtstatic or qwtdll
CODE_GENERATION = debug       # debug or release

QMAKE_CFLAGS += -std=gnu99 -Wall -Wextra -pedantic \
          -Wformat=2 -Wunused -Wno-unused-parameter -Wuninitialized \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wlogical-op

LIBS += -L../mediator -lmediator -L../engine -lengine -L../common -lcommon
DEPENDPATH += ../common ../engine ../mediator

#----------------------------------------------
# Platform dependency ...
#----------------------------------------------

unix {
  INSTALL_PREFIX = /bira-iasb/projects/DOAS/Programmes/QDOAS-linux

  INCLUDEPATH += $$INSTALL_PREFIX/include
#  INCLUDEPATH += $$INSTALL_PREFIX/include/hdf4
  QMAKE_RPATHDIR += /usr/local/hdf5-1.8.10-64/lib64
  QMAKE_RPATHDIR += /usr/local/hdf-4.2.8-64/lib64
  INCLUDEPATH += $$INSTALL_PREFIX/include/qwt
  QMAKE_RPATHDIR += $$INSTALL_PREFIX/lib
  QMAKE_RPATHLIBDIR += $$INSTALL_PREFIX/lib
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib /usr/local/lib64
}

# portable executable for linux using Linux Standard Base (LSB)
# requires static Qt and Qwt libraries compiled using LSB, as well as
# static hdf/hdf5/coda... libraries
#
# to build, run 'qmake all.pro CONFIG+=linux_package CONFIG-=unix
#
# lsbcc and lsbc++ must be on the PATH, and you must use the version
# of qmake installed with the static Qt libraries
linux_package {
  QMAKE_CXX = lsbc++
  QMAKE_CC = lsbcc
  # clear RPATH so we can distribute our own libQt*.so's
  QMAKE_RPATH =
  INSTALL_PREFIX = /home/thomasd/Code/LSB
  INCLUDEPATH += $$INSTALL_PREFIX/include
  INCLUDEPATH += $$INSTALL_PREFIX/include/hdf4
  INCLUDEPATH += $$INSTALL_PREFIX/include/qwt
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib
}

# linux -> windows cross-compilation using MXE
mxe {
  INSTALL_PREFIX = /home/thomasd/Code/cross
  INCLUDEPATH += $$INSTALL_PREFIX/include
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib
}

win32 {
  QWT_INC_PATH    = C:/Qwt-6.1.0/src
  QWT_LIB_PATH    = C:/Qwt-6.1.0/lib
  QWT_LIB         = qwt
  QWT_LIB_VERSION =
  CODA_INC_PATH   = C:/coda
  CODA_LIB_PATH   = C:/coda
  CODA_LIB        = coda

  HDFEOS_INC_PATH = D:/My_Libraries/HDF/hdfeos2_18/hdfeos2.18/hdfeos/include D:/My_Libraries/HDF/hdfeos5_1_14/hdfeos5.1.14/hdfeos5/include
  HDFEOS_LIB_PATH = D:/My_Applications/QDoas/hdfeos
  HDF_INC_PATH    = D:/My_Libraries/HDF/hdf-4.2.8/include D:/My_Libraries/HDF/hdf5-1.9.144/src D:/My_Libraries/HDF/hdf5-1.9.144/hl/src

  CODE_GENERATION = release # Override because debug is broken
}

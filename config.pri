#----------------------------------------------
# General Configuration for ALL applications
#----------------------------------------------

QWT_LINKAGE     = qwtdll        # qwtstatic or qwtdll
CODE_GENERATION = release         # debug or release

QMAKE_CFLAGS += -std=gnu99 -Wall -Wextra -pedantic \
          -Wformat=2 -Wunused -Wno-unused-parameter -Wuninitialized \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs \
          -Wjump-misses-init -Wlogical-op

LIBS += -L../mediator -lmediator -L../engine -lengine -L../common -lcommon
DEPENDPATH += ../common ../engine ../mediator

#----------------------------------------------
# Platform dependency ...
#----------------------------------------------

unix {
  INSTALL_PREFIX = /home/thomasd

  INCLUDEPATH += $$INSTALL_PREFIX/include
  INCLUDEPATH += $$INSTALL_PREFIX/include/hdf4
  INCLUDEPATH += $$INSTALL_PREFIX/include/qwt
  QMAKE_RPATHDIR += $$INSTALL_PREFIX/lib
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib
}

linux_package {
  INSTALL_PREFIX = /home/thomasd/Code/LSB/
  INCLUDEPATH += $$INSTALL_PREFIX/include
  INCLUDEPATH += $$INSTALL_PREFIX/include/hdf4
  INCLUDEPATH += $$INSTALL_PREFIX/include/qwt
  QMAKE_RPATHDIR += $$INSTALL_PREFIX/lib
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib
}

# linux -> windows cross-compilation using MXE
mxe {
  INSTALL_PREFIX = /home/thomasd/Code/cross
  INCLUDEPATH += $$INSTALL_PREFIX/include
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib
}

win32 {
  QWT_INC_PATH    = C:/Qwt-6.0.0/src
  QWT_LIB_PATH    = C:/Qwt-6.0.0/lib
  QWT_LIB         = qwt
  QWT_LIB_VERSION =
  CODA_INC_PATH   = C:/coda
  CODA_LIB_PATH   = C:/coda
  CODA_LIB        = coda

  HDFEOS_INC_PATH = D:/My_Libraries/HDF/hdfeos2_18/hdfeos2.18/hdfeos/include D:/My_Libraries/HDF/hdfeos5_1_14/hdfeos5.1.14/hdfeos5/include
  HDFEOS_LIB_PATH = D:/My_Applications/QDoas/hdfeos
  HDF_INC_PATH    = D:/My_Libraries/HDF/hdf-4.2.8/Src/hdf-4.2.8/hdf/src D:/My_Libraries/HDF/hdf5-1.9.144/src D:/My_Libraries/HDF/hdf5-1.9.144/hl/src

  CODE_GENERATION = release # Override because debug is broken
}

#----------------------------------------------
# General Configuration for ALL applications
#----------------------------------------------

CODE_GENERATION = debug       # debug or release
CONFIG += debug

QMAKE_CFLAGS += -g -std=gnu99 -Wall -Wextra -pedantic \
          -Wformat=2 -Wunused -Wno-unused-parameter -Wuninitialized \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wlogical-op

QDOAS_VERSION=2.110.1

win64 {
  QDOAS_VERSION = "$${QDOAS_VERSION}_x86_64"
}

DEFINES += QDOAS_VERSION="\"\\\"$${QDOAS_VERSION}\\\"\"" QDOAS_DATE="\"\\\"4 November 2015\\\"\""

LIBS += -L../mediator -lmediator -L../engine -lengine -L../common -lcommon
DEPENDPATH += ../common ../engine ../mediator
INCLUDEPATH += ../common ../engine ../mediator

#----------------------------------------------
# Platform dependency ...
#----------------------------------------------

unix {
  INSTALL_PREFIX = /home/username

  INCLUDEPATH += $$INSTALL_PREFIX/include
  INCLUDEPATH += $$INSTALL_PREFIX/include/hdf4
  INCLUDEPATH += $$INSTALL_PREFIX/include/qwt
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib
}

bira {
  QMAKE_CXX = g++-4.7
  INSTALL_PREFIX = /bira-iasb/projects/DOAS/Programmes/QDOAS
  QMAKE_CFLAGS_RELEASE -= -g
  QMAKE_CXXFLAGS_RELEASE -= -g
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib
  INCLUDEPATH += $$INSTALL_PREFIX/include
  INCLUDEPATH += /usr/local/include64 # for qwt headers
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
  QMAKE_CXX = g++
  QMAKE_CC = gcc
  # clear RPATH so we can distribute our own libQt*.so's
  QMAKE_LFLAGS_RPATH =

  # install prefix is set so we can "make install" to copy executables
  # and html documentation to the folder containing the package
  #
  # for package distribution:
  # cd /home/thomasd/Code/Qdoas-package/src/Qdoas/linux_package \
  #    && tar --exclude-vcs --exclude=.directory --exclude-backups \
  #    -czf ../qdoas_linux_x64.tar.gz .
  INSTALL_PREFIX = /home/thomasd/Code/Qdoas-package/src/Qdoas/linux_package
  PREFIX = /home/thomasd/Code/Qdoas-package
  INCLUDEPATH += $$PREFIX/include
  INCLUDEPATH += $$PREFIX/include/hdf4
  INCLUDEPATH += $$PREFIX/include/qwt
  QMAKE_LIBDIR += $$PREFIX/lib
}

# linux -> windows cross-compilation using mxe (M cross environment, http://mxe.cc)
# run <path-to-mxe-root>/usr/i686-pc-mingw32/qt/bin/qmake all.pro CONFIG+=mxe
mxe {
  # cross-compiled qwt, coda, hdf-eos2 and hdf-eos5 libraries should
  # be installed here:
  INSTALL_PREFIX = /bira-iasb/projects/DOAS/Programmes/QDOAS-cross

  # use "CONFIG+=mxe win64" to build a 64 bit windows version
  win64 {
    INCLUDEPATH += $$INSTALL_PREFIX/include64
    QMAKE_LIBDIR += $$INSTALL_PREFIX/lib64
  } else {
    INCLUDEPATH += $$INSTALL_PREFIX/include32
    QMAKE_LIBDIR += $$INSTALL_PREFIX/lib32
  }
}

caro {
  QWT_LINKAGE     = qwtstatic        # qwtstatic or qwtdll
  QWT_INC_PATH    = C:/Qwt-6.1.0/src
  QWT_LIB_PATH    = C:/Qwt-6.1.0/lib
  QWT_LIB         = qwt
  QWT_LIB_VERSION =
  CODA_INC_PATH   = D:/My_Applications/QDOAS/winlibs/coda/include
  CODA_LIB_PATH   = D:/My_Applications/QDOAS/winlibs/coda/lib
  CODA_LIB        = coda

  QT += core gui svg

  DEFINES += QT_NODLL

  HDFEOS_INC_PATH  = D:/My_Libraries/HDF/hdfeos2_18/hdfeos2.18/hdfeos/include D:/My_Applications/QDOAS/winlibs/hdfeos5/include D:/My_Libraries/HDF/hdfeos5_1_15/hdfeos5.1.15/hdfeos5/include_qdoas
  HDFEOS_LIB_PATH  = D:/My_Libraries/HDF/hdfeos2_18/hdfeos2.18/hdfeos/src
  HDFEOS5_LIB_PATH = D:/My_Applications/QDOAS/winlibs/hdfeos5/lib
  HDF_INC_PATH     = D:/My_Libraries/HDF/hdf-4.2.8/include D:/My_Applications/QDOAS/winlibs/hdf5-1.8.15/include
  HDF_LIB_PATH     = D:/My_Libraries/HDF/hdf-4.2.8/Src/hdf-4.2.8/hdf/src
  MFHDF_LIB_PATH   = D:/My_Libraries/HDF/hdf-4.2.8/Src/hdf-4.2.8/mfhdf/libsrc
  HDF5_LIB_PATH    = D:/My_Applications/QDOAS/winlibs/hdf5-1.8.15/lib
  NETCDF_INC_PATH  = D:/My_Applications/QDOAS/winlibs/netCDF/include
  NETCDF_LIB_PATH  = D:/My_Applications/QDOAS/winlibs/netCDF/lib

  CODE_GENERATION = release # Override because debug is broken
}

thomas {

  CONFIG += qwt

  INCLUDEPATH += /home/thomasd/include
  INCLUDEPATH += /usr/include/hdf
  INCLUDEPATH += /usr/include/hdf5/serial
  INCLUDEPATH += /usr/include/hdf-eos5
  INCLUDEPATH += /usr/include/x86_64-linux-gnu/hdf
  
  QMAKE_LIBDIR += /usr/lib/x86_64-linux-gnu/hdf5/serial/
  QMAKE_RPATHDIR += /home/thomasd/lib
}

#----------------------------------------------
# General Configuration for ALL applications
#----------------------------------------------

CODE_GENERATION = release      # debug or release
CONFIG += release

QMAKE_CFLAGS += -g -std=gnu99 -Wall -Wextra -pedantic \
          -Wformat=2 -Wunused -Wno-unused-parameter -Wuninitialized \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wlogical-op

QMAKE_LFLAGS += -g
QMAKE_LFLAGS_RELEASE=

QDOAS_VERSION=3.2

win64 {
  QDOAS_VERSION = "$${QDOAS_VERSION}_x86_64"
}

DEFINES += QDOAS_VERSION="\"\\\"$${QDOAS_VERSION}\\\"\"" QDOAS_DATE="\"\\\"12 September 2017\\\"\""

LIBS += -L../mediator -lmediator -L../engine -lengine -L../common -lcommon
DEPENDPATH += ../common ../engine ../mediator
INCLUDEPATH += ../common ../engine ../mediator

LIBS += -lgsl -lgslcblas

#----------------------------------------------
# Platform dependency ...
#----------------------------------------------

bira {
  LIBS -= -lgslcblas
  LIBS += -latlas
  INSTALL_PREFIX = /bira-iasb/projects/DOAS/Programmes/QDOAS
  QMAKE_CFLAGS_RELEASE -= -g
  QMAKE_CXXFLAGS_RELEASE -= -g
  INCLUDEPATH += $$INSTALL_PREFIX/include
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib_$${QDOAS_VERSION} $$INSTALL_PREFIX/lib_common
  QMAKE_RPATHDIR += $$INSTALL_PREFIX/lib_$${QDOAS_VERSION} $$INSTALL_PREFIX/lib_common
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
  CONFIG -= debug
  CONFIG += release
  CODE_GENERATION = release
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
  CODA_LIB_PATH   = D:/My_Applications/QDOAS/winlibs/coda/lib
  CODA_LIB        = coda

  QT += core gui svg

  DEFINES += QT_NODLL

  INCLUDEPATH += C:/Qwt-6.1.0/src
  INCLUDEPATH += D:/My_Applications/QDOAS/winlibs/hdfeos/include
  INCLUDEPATH += D:/My_Applications/QDOAS/winlibs/hdfeos5/include
  INCLUDEPATH += D:/My_Applications/QDOAS/winlibs/hdf/include
  INCLUDEPATH += D:/My_Applications/QDOAS/winlibs/hdf5/include
  INCLUDEPATH += D:/My_Applications/QDOAS/winlibs/netCDF/include
  INCLUDEPATH += D:/My_Applications/QDOAS/winlibs
  INCLUDEPATH += D:/My_Applications/QDOAS/winlibs/coda/include

  HDFEOS_LIB_PATH  = D:/My_Applications/QDOAS/winlibs/hdfeos/lib
  HDFEOS5_LIB_PATH = D:/My_Applications/QDOAS/winlibs/hdfeos5/lib
  HDF_LIB_PATH     = D:/My_Applications/QDOAS/winlibs/hdf/lib
  MFHDF_LIB_PATH   = D:/My_Applications/QDOAS/winlibs/hdf/lib
  HDF5_LIB_PATH    = D:/My_Applications/QDOAS/winlibs/hdf5/lib
  NETCDF_LIB_PATH  = D:/My_Applications/QDOAS/winlibs/netcdf/lib
  GSL_LIB_PATH  = D:/My_Applications/QDOAS/winlibs/gsl/lib

  CODE_GENERATION = release # Override because debug is broken
}

macports {
  INSTALL_PREFIX = $$(HOME)
  INCLUDEPATH += /opt/local/include /opt/local/lib/hdfeos5/include /opt/local/libexec/qt5/lib/qwt.framework/Versions/6/Headers
  QMAKE_LIBDIR += /opt/local/lib /opt/local/lib/hdfeos5/lib /opt/local/libexec/qt5/lib
}

ubuntu {

  CONFIG += qwt

  INCLUDEPATH += /usr/include/qwt
  INCLUDEPATH += /usr/include/hdf
  INCLUDEPATH += /usr/include/hdf5/serial
  INCLUDEPATH += /usr/include/hdf-eos5
  INCLUDEPATH += /usr/include/x86_64-linux-gnu/hdf

  QMAKE_LIBDIR += /usr/lib/x86_64-linux-gnu/hdf5/serial/
}

asan { # use 'CONFIG+=asan' to build with adress sanitizer support
  QMAKE_CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
  QMAKE_CFLAGS += -fsanitize=address -fno-omit-frame-pointer
  QMAKE_LFLAGS += -fsanitize=address
}

ccache {
  QMAKE_CXX = ccache g++
  QMAKE_CC = ccache gcc
}

openblas {
  LIBS -= -lgslcblas
  LIBS += -lopenblas
}

# Notes for installation on hpc:
#
# hdf-eos5:
#
#   CC=/space/hpc-apps/bira/2015a/hdf5-1.8.10-64/bin/h5cc ./configure /
#     --prefix=/home/thomasd --with-szlib=/home/thomasd /
#     --enable-install-include
hpc {
  LIBS += -lirc -lsvml

  INSTALL_PREFIX = $$(HOME)
  INCLUDEPATH += $$INSTALL_PREFIX/include

  INCLUDEPATH +=/space/hpc-apps/bira/2015a/hdf-eos5-1.14-hdf5-1.8.10/include64

  QMAKE_LIBDIR += /space/hpc-apps/bira/2015a/hdf-eos5-1.14-hdf5-1.8.10/lib64
  QMAKE_RPATHDIR += /space/hpc-apps/bira/2015a/hdf-eos5-1.14-hdf5-1.8.10/lib64
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib /sw/sdev/intel/parallel_studio_xe_2015_update_3-pguyan/composer_xe_2015.3.187/compiler/lib/intel64
  QMAKE_RPATHDIR += $$INSTALL_PREFIX/lib /sw/sdev/intel/parallel_studio_xe_2015_update_3-pguyan/composer_xe_2015.3.187/compiler/lib/intel64
}

unix {
  isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = $$(HOME)
  }

  INCLUDEPATH += $$INSTALL_PREFIX/include
  INCLUDEPATH += $$INSTALL_PREFIX/include/hdf4
  INCLUDEPATH += $$INSTALL_PREFIX/include/qwt
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib
  QMAKE_RPATHDIR += $$INSTALL_PREFIX/lib
}

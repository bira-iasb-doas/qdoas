#----------------------------------------------
# General Configuration for ALL applications
#----------------------------------------------

CODE_GENERATION = staticlib release      # debug or release
CONFIG += release

QMAKE_CFLAGS += -g -std=gnu99 -Wall -Wextra -pedantic \
          -Wformat=2 -Wunused -Wno-unused-parameter -Wuninitialized \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wlogical-op

QMAKE_LFLAGS += -g
QMAKE_LFLAGS_RELEASE=

QDOAS_VERSION=FRM4DOAS

win64 {
  QDOAS_VERSION = "$${QDOAS_VERSION}_x86_64"
}

DEFINES += QDOAS_VERSION="\"\\\"$${QDOAS_VERSION}\\\"\"" QDOAS_DATE="\"\\\"14 June 2018\\\"\""

LIBS += -L../mediator -lmediator -L../engine -lengine -L../common -lcommon
DEPENDPATH += ../common ../engine ../mediator
INCLUDEPATH += ../common ../engine ../mediator

LIBS += -lgsl -lgslcblas

#----------------------------------------------
# Platform dependency ...
#----------------------------------------------

# QDOAS linux package: uses own version of Qt, HDF, ... libraries,
# with a wrapper script that ensures the correct libraries are used
# when running the package on another system.
#
# to build, run
#
#   /bira-iasb/projects/DOAS/Programmes/QDOAS-linux-package/bin/qmake all.pro CONFIG+=linux_package CONFIG-=unix
#   make
#
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
  INSTALL_PREFIX = /bira-iasb/projects/DOAS/Programmes/QDOAS-linux-package/src/Qdoas/linux_package
  PREFIX = /bira-iasb/projects/DOAS/Programmes/QDOAS-linux-package
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
  QWT_LINKAGE     = qwtdll        # qwtstatic or qwtdll
  QWT_INC_PATH    = C:/Qwt-6.1.3/src
  QWT_LIB_PATH    = C:/Qwt-6.1.3/lib
  QWT_LIB         = qwt
  QWT_LIB_VERSION =
  CODA_LIB_PATH   = c:/My_Applications/QDOAS/winlibs/coda/lib
  CODA_LIB        = coda

  QT += core gui svg

  # DEFINES += QT_NODLL

  INCLUDEPATH += C:/Qwt-6.1.3/src
  INCLUDEPATH += C:/My_Libraries/HDF/hdfeos2_19/hdfeos2.19/hdfeos/include
  INCLUDEPATH += C:/My_Libraries/HDF/hdfeos5_1_15/hdfeos5.1.15/hdfeos5/include
  INCLUDEPATH += C:/My_Libraries/HDF/CMake-hdf-4.2.12/hdf-4.2.12/hdf/src
  INCLUDEPATH += C:/My_Libraries/HDF/CMake-hdf-4.2.12/hdf-4.2.12/mfhdf/libsrc
  INCLUDEPATH += C:/My_Libraries/HDF/CMake-hdf5-1.10.0-patch1/hdf5-1.10.0-patch1/src
  INCLUDEPATH += C:/My_Libraries/HDF/CMake-hdf5-1.10.0-patch1/hdf5-1.10.0-patch1/c++/src
  INCLUDEPATH += C:/My_Libraries/HDF/netcdf-4.6.0/include
  INCLUDEPATH += C:/My_Satellites_Activities/Programs/coda-2.20/bin
  INCLUDEPATH += c:/My_Applications/QDOAS/winlibs

  HDFEOS_LIB_PATH  = c:/My_Applications/QDOAS/winlibs/hdfeos/lib
  HDFEOS5_LIB_PATH = c:/My_Applications/QDOAS/winlibs/hdfeos5/lib
  HDF_LIB_PATH     = c:/My_Applications/QDOAS/winlibs/hdf/lib
  MFHDF_LIB_PATH   = c:/My_Applications/QDOAS/winlibs/hdf/lib
  HDF5_LIB_PATH    = c:/My_Applications/QDOAS/winlibs/hdf5/lib
  NETCDF_LIB_PATH  = c:/My_Applications/QDOAS/winlibs/netcdf/lib
  GSL_LIB_PATH  = c:/My_Applications/QDOAS/winlibs/gsl/lib

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

# Notes for installation on bira computes
#
# Currently we have old SLES systems and new CentOS systems.  We can
# compile and run QDOAS on both, BUT
#
# * if we compile on CentOS, the binary will not work on SLES (because
#   the CentOS system uses a newer libc)
#
# * if we compile on SLES, the binary will also work on CentOS, if the
#   user loads module 17/linalg (<- needed for libatlas.so on CentOS).
#
# compilation on SLES computes:
# -----------------------------
#
# QDOAS needs a somewhat recent GCC compiler.  On the SLES systems, we
# need to load a module for that:
#
# module load gcc-4.9.2
# qmake all.pro CONFIG+=compute
# make
#
# compilation on CentOS computes:
# -------------------------------
# module load 17/base 17/linalg
# qmake-qt4 all.pro CONFIG+=compute
# make
#
# (We need qmake-qt4.  Qt4 and Qt5 are both available on CentOS, but
# our version of Qwt is compiled against Qt4.)
#
# installation:
# -------------
#
# make install <- !!! attention
#
# This will install in /bira-iasb/projects/DOAS/Programmes/QDOAS/bin_$$QDOAS_VERSION
compute {
  # Atlas is faster than gslcblas, so use it when it is available
  LIBS -= -lgslcblas
  LIBS += -latlas
  INSTALL_PREFIX = /bira-iasb/projects/DOAS/Programmes/QDOAS
  INCLUDEPATH += $$INSTALL_PREFIX/include
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib_2016 $$INSTALL_PREFIX/lib_common
  QMAKE_RPATHDIR += /bira-iasb/softs/gcc492/lib $$INSTALL_PREFIX/lib_2016 $$INSTALL_PREFIX/lib_common

  # When compiling from the CentOS computes, uncomment these:
  #
  #QMAKE_LIBDIR += /bira-iasb/softs/17/linalg/lib64 /bira-iasb/softs/17/base/lib64
  #QMAKE_RPATHDIR += /bira-iasb/softs/17/linalg/lib64 /bira-iasb/softs/17/base/lib64
}

# Notes for installation on hpc:
#
# Modules:
# --------
#
# Currently, QDOAS relies on modules 17/doas, 17/intel-17u1, 17/base,
# 17/hdf-netcdf and 17/linalg.  The module 17/doas, installed in
# /space/hpc-apps/bira/17/doas, contains libraries maintained by us
# specifically for QDOAS.  The '17' indicates the version (we might
# use newer versions later on).  The different versions of QDOAS
# itself will be installed in /space/hpc-apps/bira/doas.  Different
# versions might use different modules (i.e. in the future, versions
# using module 17/doas might coexist with versions using a newer
# module version such as 18/doas)
#
# compilation:
# ------------
#
# module purge
# module load 17/doas 17/intel-17u1 17/base 17/hdf-netcdf 17/linalg
# qmake all.pro CONFIG+=hpc
# make
#
# installation:
# -------------
# make install <= !!! attention
#
# This will install qdoas in /space/hpc-aps/bira/doas/bin_$$QDOAS_VERSION,
hpc {
  LIBS -= -lgslcblas
  LIBS += -latlas -lgfortran
  INSTALL_PREFIX = /space/hpc-apps/bira/doas
  QDOASLIBS = $$INSTALL_PREFIX/lib /space/hpc-apps/bira/17/base/lib64 /space/hpc-apps/bira/17/linalg/lib64 /space/hpc-apps/bira/17/hdf-netcdf/lib64 /space/hpc-apps/bira/17/doas/lib
  QMAKE_LIBDIR += $$QDOASLIBS
  QMAKE_RPATHDIR += $$QDOASLIBS
}

unix {
  CONFIG += qwt

  isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = $$(HOME)
  }

  INCLUDEPATH += $$INSTALL_PREFIX/include
  INCLUDEPATH += $$INSTALL_PREFIX/include/hdf4
  INCLUDEPATH += $$INSTALL_PREFIX/include/qwt
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib
}

asan { # use 'CONFIG+=asan' to build with adress sanitizer support
  QMAKE_CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
  QMAKE_CFLAGS += -fsanitize=address -fno-omit-frame-pointer
  QMAKE_LFLAGS += -fsanitize=address
}

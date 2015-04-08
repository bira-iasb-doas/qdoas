#----------------------------------------------
# General Configuration for ALL applications
#----------------------------------------------

CODE_GENERATION = release       # debug or release
CONFIG += release

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
  INSTALL_PREFIX = /home/username

  INCLUDEPATH += $$INSTALL_PREFIX/include
  INCLUDEPATH += $$INSTALL_PREFIX/include/hdf4
  INCLUDEPATH += $$INSTALL_PREFIX/include/qwt
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib
}

bira {
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

# linux -> windows cross-compilation using MXE
# run <path-to-mxe-root>/usr/i686-pc-mingw32/qt/bin/qmake all.pro CONFIG+=mxe
mxe {
  # qwt, coda, hdf-eos2 and hdf-eos5 libraries should be installed here:
  INSTALL_PREFIX = /bira-iasb/projects/DOAS/Programmes/QDOAS-cross
  INCLUDEPATH += $$INSTALL_PREFIX/include
  QMAKE_LIBDIR += $$INSTALL_PREFIX/lib
}

caro {
  QWT_LINKAGE     = qwtdll        # qwtstatic or qwtdll
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

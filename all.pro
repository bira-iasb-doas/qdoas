#----------------------------------------------
# General Configuration
#----------------------------------------------

TEMPLATE = subdirs

SUBDIRS = qdoas convolution ring usamp cmdline

CONFIG += ordered

QMAKE_CXXFLAGS_RELEASE += -g
QMAKE_CFLAGS_RELEASE += -g
QMAKE_LFLAGS_RELEASE =


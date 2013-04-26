#----------------------------------------------
# General Configuration for ALL applications
#----------------------------------------------

HELP_SYSTEM     =      # or empty
QWT_LINKAGE     = qwtdll        # qwtstatic or qwtdll
CODE_GENERATION = release         # debug or release

QMAKE_CFLAGS += -std=gnu99

#----------------------------------------------
# Platform dependency ...
#----------------------------------------------

unix {
  INSTALL_PREFIX = /bira-iasb/proj_dyn/DOAS/Programmes/QDOAS-linux

  INCLUDEPATH += /bira-iasb/proj_dyn/DOAS/Programmes/QDOAS-linux/include /bira-iasb/proj_dyn/DOAS/Programmes/QDOAS-linux/hdfeos/include/ /usr/local/qwt/include
  QMAKE_LFLAGS += -Wl,-rpath=/bira-iasb/proj_dyn/DOAS/Programmes/QDOAS-linux/lib:/usr/local/lib64
  QMAKE_LIBDIR += /bira-iasb/proj_dyn/DOAS/Programmes/QDOAS-linux/hdfeos/lib/linux64 /bira-iasb/proj_dyn/DOAS/Programmes/QDOAS-linux/lib /usr/local/lib64
  LIBS += -lqwt

  # QDOAS online help
  QDOAS_HELP= '\\"/path/to/dir/Help\\"'  # place quotes around the version string
  DEFINES += QDOAS_HELP_PATH=\"$${QDOAS_HELP}\"
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

  INSTALL_PREFIX = C:/Qdoas

  CODE_GENERATION = release # Override because debug is broken
}

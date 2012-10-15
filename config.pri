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
  QWT_INC_PATH   = /usr/local/qwt/include
  QWT_LIB_PATH   = /usr/local/qwt/lib
  QWT_LIB        = qwt

  INSTALL_PREFIX = /bira-iasb/proj_dyn/DOAS/Programmes/QDOAS-linux
  CODA_LIB_PATH = /bira-iasb/proj_dyn/DOAS/Programmes/QDOAS-linux/lib
  CODA_INC_PATH = /bira-iasb/proj_dyn/DOAS/Programmes/QDOAS-linux/include
  HDFEOS_LIB_PATH = /bira-iasb/proj_dyn/DOAS/Programmes/QDOAS-linux/hdfeos/lib/linux64
  HDFEOS_INC_PATH = /home/thomasd/hdfeos/include
  HDF4_LIB_PATH = /bira-iasb/proj_dyn/DOAS/Programmes/QDOAS-linux/lib
  HDF_INC_PATH    = /home/thomasd/hdf4/include/

  # QDOAS online help

  #  QDOAS_HELP= '\\"D:/My_Applications/QDoas/trunk/Src/qdoas/release/Help\\"'  # place quotes around the version string
  # DEFINES += QDOAS_HELP_PATH=\"$${QDOAS_HELP}\"

  # for trace-write debugging ...
  # DEFINES += DEBUG
  # DEFINES += LVL4
}

win32 {
  QWT_INC_PATH    = C:/Qwt-6.0.0/src
  QWT_LIB_PATH    = C:/Qwt-6.0.0/lib
  QWT_LIB         = qwt
  QWT_LIB_VERSION =
  CODA_INC_PATH   = C:/coda
  CODA_LIB_PATH   = C:/coda
  CODA_LIB        = coda

  HDFEOS_INC_PATH = D:/My_Libraries/HDF/hdfeos2_18/hdfeos2.18/hdfeos/include
  HDFEOS_LIB_PATH = D:/My_Applications/QDoas/hdfeos
  HDF_INC_PATH    = D:/My_Libraries/HDF/hdf-4.2.8/Src/hdf-4.2.8/hdf/src

  INSTALL_PREFIX = C:/Qdoas

  CODE_GENERATION = release # Override because debug is broken
}

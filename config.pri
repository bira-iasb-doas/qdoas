#----------------------------------------------
# General Configuration for ALL applications
#----------------------------------------------

HELP_SYSTEM     =      # or empty
QWT_LINKAGE     = qwtdll        # qwtstatic or qwtdll
CODE_GENERATION = debug         # debug or release

#----------------------------------------------
# Platform dependency ...
#----------------------------------------------

unix {
  QWT_INC_PATH   = /usr/local/qwt/include
  QWT_LIB_PATH   = /usr/local/qwt/lib
  QWT_LIB        = qwt

  INSTALL_PREFIX = /usr/local/Qdoas

  CODA_LIB_PATH = /usr/local/lib
  CODA_INC_PATH = /usr/local/include

  # QDOAS online help

  QDOAS_HELP= '\\"D:/My_Applications/QDoas/trunk/Src/qdoas/release/Help\\"'  # place quotes around the version string
  DEFINES += QDOAS_HELP_PATH=\"$${QDOAS_HELP}\"

  # for trace-write debugging ...
  DEFINES += DEBUG
  DEFINES += LVL4
}

win32 {
  QWT_INC_PATH    = C:/Qwt-6.0.0/src
  QWT_LIB_PATH    = C:/Qwt-6.0.0/lib
  QWT_LIB         = qwt
  QWT_LIB_VERSION =
  CODA_INC_PATH   = C:/coda
  CODA_LIB_PATH   = C:/coda
  CODA_LIB        = coda

  INSTALL_PREFIX = C:/Qdoas

  CODE_GENERATION = release # Override because debug is broken
}

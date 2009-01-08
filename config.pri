#----------------------------------------------
# General Configuration for ALL applications
#----------------------------------------------

HELP_SYSTEM     = assistant     # or empty
QWT_LINKAGE     = qwtdll        # qwtstatic or qwtdll
CODE_GENERATION = debug         # debug or release

#----------------------------------------------
# Platform dependency ...
#----------------------------------------------

unix {
  QWT_INC_PATH   = /usr/local/qwt/include
  QWT_LIB_PATH   = /usr/local/qwt/lib
  QWT_LIB        = qwt
  CODA_INC_PATH   = /usr/local/include
  CODA_LIB_PATH   = /usr/local/lib
  CODA_LIB        = coda


  INSTALL_PREFIX = /usr/local/Qdoas

  # for trace-write debugging ...
  DEFINES += DEBUG
  DEFINES += LVL4
}

win32 {
  QWT_INC_PATH    = C:\Qwt\include
  QWT_LIB_PATH    = C:\Qwt\lib
  QWT_LIB         = qwt
  QWT_LIB_VERSION = 5
  CODA_INC_PATH   = C:\coda
  CODA_LIB_PATH   = C:\coda
  CODA_LIB        = coda

  INSTALL_PREFIX = C:\Qdoas

  CODE_GENERATION = release # Override because debug is broken
}

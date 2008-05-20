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
  BEAT_INC_PATH  = /home/ian/local/include
  BEAT_LIB_PATH  = /home/ian/local/lib

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
  BEAT_INC_PATH   = C:\beat\include
  BEAT_LIB_PATH   = C:\beat\lib
  BEAT_LIB        = beat

  INSTALL_PREFIX = C:\Qdoas

  CODE_GENERATION = release # Override because debug is broken
}

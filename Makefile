#
#
#
O:=o
CPP:=cpp
C:=c
CCPP:=g++
CC:=gcc
CCFLAGS:=
CCPPFLAGS:=
CCDBG:=-g
CCPPDBG:=-g
CCTRCDBG:=-g -DDEBUG
CCPPTRCDBG:=-g -DDEBUG

# sloppy switch
ifeq "$(HOST)" "cressida"
MOC         :=/usr/local/Trolltech/Qt-4.2.3/bin/moc
INC_DIR_QT4 :=/usr/local/Trolltech/Qt-4.2.3/include
LIB_DIR_QT4 :=/usr/lib
LD_RPATH    :=-Wl,-R/usr/local/lib
else
MOC         :=/usr/bin/moc-qt4
INC_DIR_QT4 :=/usr/share/qt4/include
LIB_DIR_QT4 :=/usr/lib
LD_RPATH    :=
endif

LIBS:=
SYSLIBS:= -lm
GUILIBS:= -L$(LIB_DIR_QT4) $(LD_RPATH) -lQtGui -lQtCore

INCL := -I. \
	-I$(INC_DIR_QT4) \
	-I$(INC_DIR_QT4)/Qt \
	-I$(INC_DIR_QT4)/QtCore \
	-I$(INC_DIR_QT4)/QtGui \
        -Imediator

#---------------------------------------------------------------------

obj := 

guiobj := \
         CWMain.$(O) \
         CWProjectTree.$(O) \
         CWSiteTree.$(O) \
         CWUserSymbolTree.$(O) \
         CWEditor.$(O) \
         CWSiteEditor.$(O) \
         CWActiveContext.$(O) \
         CWSplitter.$(O) \
         CWProjectFolderNameEditor.$(O) \
         CWProjectDirectoryEditor.$(O) \
         CWProjectNameEditor.$(O) \
         CWProjectAnalysisWindowNameEditor.$(O) \
         CWProjectTabSpectra.$(O) \
         CWProjectPropertyEditor.$(O)

mocinc := $(guiobj:%.$(O)=%.h)

#---------------------------------------------------------------------

mocobj := $(mocinc:%.h=moc_%.$(O))

main := qdoas

CCOPTS:=$(CCFLAGS)
CCPPOPTS:=$(CCPPFLAGS)

ifeq "$(MAKECMDGOALS)" "debug"
CCOPTS:=$(CCDBG) $(CCFLAGS)
CCPPOPTS:=$(CCPPDBG) $(CCPPFLAGS)
endif

ifeq "$(MAKECMDGOALS)" "trace"
CCOPTS:=$(CCTRCDBG) $(CCFLAGS)
CCPPOPTS:=$(CCPPTRCDBG) $(CCPPFLAGS)
endif

all: $(main)
debug: $(main)
trace: $(main)

$(main): $(obj) $(guiobj) $(mocobj) $(main).$(O)
	$(CCPP) $(CCPPOPTS) -o $@ $^ $(LIBS) $(SYSLIBS) $(GUILIBS)

clean:
	echo "Cleaning up..."
	-/bin/rm $(obj) $(obj:%.$(O)=%.d)
	-/bin/rm $(mocobj) $(guiobj) $(guiobj:%.$(O)=%.d)
	-/bin/rm $(main) $(main:%=%.$(O)) $(main:%=%.d)

%.d: %.$(CPP)
	$(CCPP) -MM $^ $(CCPPOPTS) $(INCL) > $@

%.d: %.$(C)
	$(CC) -MM $^ $(CCOPTS) $(INCL) > $@

%.$(O): %.$(CPP)
	$(CCPP) $(CCPPOPTS) -c $< $(INCL)

%.$(O): %.$(C)
	$(CC) $(CCOPTS) -c $< $(INCL)

moc_%.$(CPP): %.h
	$(MOC) $< -o $@

include $(obj:%.$(O)=%.d) $(guiobj:%.$(O)=%.d) $(mainobj:%=%.d)


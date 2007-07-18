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

MOC         :=/usr/bin/moc-qt4
INC_DIR_QT4 :=/usr/share/qt4/include
LIB_DIR_QT4 :=/usr/lib
LD_RPATH    :=

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

obj := \
         CWorkSpace.$(O) \
         CValidator.$(O) \
         CEngineThread.$(O) \
         CEngineRequest.$(O) \
         CEngineResponse.$(O) \
         CSession.$(O)


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
         CWProjectTabAnalysis.$(O) \
         CWProjectPropertyEditor.$(O) \
         CEngineController.$(O) \
         CNavigationPanel.$(O)


mediatorobj := mediator/mediate.$(O)

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

$(main): $(obj) $(guiobj) $(mocobj) $(mediatorobj) $(main).$(O)
	$(CCPP) $(CCPPOPTS) -o $@ $^ $(LIBS) $(SYSLIBS) $(GUILIBS)

clean:
	echo "Cleaning up..."
	-/bin/rm $(obj) $(obj:%.$(O)=%.d)
	-/bin/rm $(mocobj) $(guiobj) $(guiobj:%.$(O)=%.d)
	-/bin/rm $(main) $(main:%=%.$(O)) $(main:%=%.d)
	-/bin/rm $(mediatorobj)

%.d: %.$(CPP)
	$(CCPP) -MM $^ $(CCPPOPTS) $(INCL) > $@

%.d: %.$(C)
	$(CC) -MM $^ $(CCOPTS) $(INCL) > $@

%.$(O): %.$(CPP)
	$(CCPP) $(CCPPOPTS) -c $< $(INCL)

%.$(O): %.$(C)
	$(CC) $(CCOPTS) -c $< $(INCL)

$(mediatorobj):%.$(O): %.c
	gcc -g -c -o $@ $< -Imediator

moc_%.$(CPP): %.h
	$(MOC) $< -o $@

include $(obj:%.$(O)=%.d) $(guiobj:%.$(O)=%.d) $(mainobj:%=%.d)


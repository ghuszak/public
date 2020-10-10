.PHONY: clean

CC=gcc
OBJDIR=obj
CFLAGS=-Isimu -Wall -FSUPPORT_SCRAMBLING -D_DEBUG

_DEPS=base.h states.h
_OBJS=states.o main.o random.o 4b5b.o channel.o pma_tx.o pma_rx.o pcs_tx.o pcs_rx.o timer.o pma.o pcs.o scrambling.o plca.o plca_ctrl.o plca_data.o plca_status.o mac_pls.o mac_simu.o

ifeq ($(OS), Windows_NT)
   PLATFORM:=WINDOWS
else
   UNAME_S:=$(shell uname -s)
   ifeq ($(UNAME_S), Linux)
      PLATFORM:=LINUX
   else ifeq ($(UNAME_S), Darwin)
      PLATFORM:=MACOS
   else
      $(error Unknown OS)
   endif
endif

DEPS=$(patsubst %,simu/%,$(_DEPS))
OBJS=$(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: simu/%.cpp $(DEPS) Makefile | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

ifeq ($(PLATFORM), WINDOWS)
mac-phy-simu.exe: $(OBJS) Makefile
	$(CC) -o mac-phy-simu.exe $(OBJS)
all: mac-phy-simu.exe
else
mac-phy-simu: $(OBJS) Makefile
	$(CC) -o mac-phy-simu -lcurses $(OBJS)
all: mac-phy-simu
endif

$(OBJDIR):
ifeq ($(PLATFORM), WINDOWS)
	cmd /e:on /c if not exist $(OBJDIR) mkdir $(OBJDIR)
else
	-mkdir -p $(OBJDIR)
endif

clean:
ifeq ($(PLATFORM), WINDOWS)
	rm -f mac-phy-simu.exe $(OBJS)
	cmd /e:on /c if exist $(OBJDIR) rmdir $(OBJDIR)
else
	rm -f mac-phy-simu
	-rm -rf $(OBJDIR)
endif

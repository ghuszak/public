.PHONY: clean

CC=gcc
OBJDIR=obj
CFLAGS=-Wall -Wno-unused-function -Wno-unused-but-set-variable -D_DEBUG

_OBJS=FEC_data.o

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

OBJS=$(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: FEC_data/%.cpp Makefile | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

ifeq ($(PLATFORM), WINDOWS)
FEC_data.exe: $(OBJS) Makefile
	$(CC) -o FEC_data.exe $(OBJS)
all: FEC_data.exe
else
FEC_data: $(OBJS) Makefile
	$(CC) -o FEC_data -lcurses $(OBJS)
all: FEC_data
endif

$(OBJDIR):
ifeq ($(PLATFORM), WINDOWS)
	cmd /e:on /c if not exist $(OBJDIR) mkdir $(OBJDIR)
else
	-mkdir -p $(OBJDIR)
endif

clean:
ifeq ($(PLATFORM), WINDOWS)
	rm -f FEC_data.exe $(OBJS)
	cmd /e:on /c if exist $(OBJDIR) rmdir $(OBJDIR)
else
	rm -f FEC_data
	-rm -rf $(OBJDIR)
endif

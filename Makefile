# application name
APPNAME := gtksolver
# compiler
CC	:= gcc
CCLD    := $(CC)
# output/object/include/source directories
BINDIR  := bin
OBJDIR  := obj
INCLUDE	:= include
SRCDIR  := src
# compiler and linker flags
CFLAGS  := -Wall -Wextra -pedantic -finline-functions -std=c11 -Wshadow
CFLAGS	+= -I$(INCLUDE)
CFLAGS	+= `pkg-config --cflags --libs gtk+-2.0`
ifeq ($(debug),-DDEBUG)
CFLAGS  += -g
else
CFLAGS  += -Ofast
endif
LDFLAGS := `pkg-config --libs gtk+-2.0`
# libraries
LIBS    :=
# source/include/object variables
SOURCES	:= $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(INCLUDE)/*.h)
OBJECTS := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

ifeq ($(os),windows)
APPNAME	:= $(APPNAME).exe
LIBS    := -Wl,-subsystem,windows
endif

# debug printing variable contents
# $(info $$SOURCES is [${SOURCES}])
# $(info $$INCLUDES is [${INCLUDES}])
# $(info $$OBJECTS is [${OBJECTS}])

# $(APPNAME):	$(OBJECTS)
all:	$(OBJECTS)
	@mkdir -p $(@D)/bin
	$(CCLD) -o $(BINDIR)/$(APPNAME) $(OBJECTS) $(CFLAGS) $(LDFLAGS) $(LIBS)
# strip only if -DDEBUG not set
ifneq ($(debug),-DDEBUG)
	strip -s $(BINDIR)/$(APPNAME)
endif

$(OBJECTS):	$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINDIR) $(OBJDIR)

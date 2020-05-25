#
# Makefile for Main (Test Build)
#
# NOTE: Temporary Makefile for testing Main.
# Should eventually be replaced or revised once
# the project can (re)build a complete ELF.
#

SCETOP ?= /usr/local/sce
SCEINCDIR = $(SCETOP)/common/include

EEINCDIR = $(SCETOP)/ee/include
EELIBDIR = $(SCETOP)/ee/lib
#EEILBDIR = $(SCETOP)/ee/ilb
#EEMODDIR = $(SCETOP)/ee/modules

IOPINCDIR = $(SCETOP)/iop/gcc/mipsel-scei-elfl/include
IOPLIBDIR = $(SCETOP)/iop/gcc/mipsel-scei-elfl/lib
IOPMODDIR = $(SCETOP)/iop/modules

PREFIX  = ee
AR      = $(PREFIX)-ar
AS      = $(PREFIX)-gcc
CC      = $(PREFIX)-gcc
CXX     = $(PREFIX)-gcc
LD      = $(PREFIX)-gcc
DVPASM  = $(PREFIX)-dvp-as
OBJCOPY = $(PREFIX)-objcopy
OBJDUMP = $(PREFIX)-objdump
RUN     = dsedb -r run
RM      = /bin/rm -f

TOP    = ../..
COMMON = $(TOP)/common
MODULE = $(TOP)/module
SOURCE = $(TOP)/source

INCDIRS = \
-I$(EEINCDIR) \
-I$(SCEINCDIR) \
-I$(COMMON)/include \
-I$(MODULE)/include \
-I$(SOURCE)/include

VERSION ?= SLPM65019

DEFINES = -DBUILD_$(VERSION)
CFLAGS  = -Wall -O2 -c $(INCDIRS) $(DEFINES)
CXXFLAGS = $(CFLAGS)

all: ee

ee: main.cc
	$(CXX) $(CXXFLAGS) $^

clean:
	-rm *.o

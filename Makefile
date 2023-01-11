#!/usr/bin/make -f
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man/man1
# see http://lv2plug.in/pages/filesystem-hierarchy-standard.html, don't use libdir
LV2DIR ?= $(PREFIX)/lib/lv2

CFLAGS ?= -Wall -g -Wno-unused-function

PKG_CONFIG?=pkg-config
STRIP?= strip

BUILDOPENGL?=yes
BUILDJACKAPP?=yes

RW ?= robtk/

###############################################################################

MACHINE=$(shell uname -m)
ifneq (,$(findstring x64,$(MACHINE)))
  HAVE_SSE=yes
endif
ifneq (,$(findstring 86,$(MACHINE)))
  HAVE_SSE=yes
endif

ifeq ($(HAVE_SSE),yes)
  OPTIMIZATIONS ?= -msse -msse2 -mfpmath=sse -ffast-math -fomit-frame-pointer -O3 -fno-finite-math-only -DNDEBUG
else
  OPTIMIZATIONS ?= -fomit-frame-pointer -O3 -fno-finite-math-only -DNDEBUG
endif

###############################################################################

BUILDDIR = build/
BINDIR = bin/

###############################################################################

LV2NAME=ms_matrix
LV2GUI=ms_matrixUI_gl
BUNDLE=ms_matrix.lv2

targets=

LOADLIBES=-lm
LV2UIREQ=
GLUICFLAGS=-I.

UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)
  LV2LDFLAGS=-dynamiclib
  LIB_EXT=.dylib
  EXE_EXT=
  UI_TYPE=ui:CocoaUI
  PUGL_SRC=$(RW)pugl/pugl_osx.mm
  PKG_GL_LIBS=
  GLUICFLAGS+=-DROBTK_UPSCALE
  GLUILIBS=-framework Cocoa -framework OpenGL -framework CoreFoundation
  STRIPFLAGS=-u -r -arch all -s $(RW)lv2syms
  EXTENDED_RE=-E
else
  LV2LDFLAGS=-Wl,-Bstatic -Wl,-Bdynamic -Wl,--as-needed
  LIB_EXT=.so
  EXE_EXT=
  UI_TYPE=ui:X11UI
  PUGL_SRC=$(RW)pugl/pugl_x11.c
  PKG_GL_LIBS=glu gl
  GLUILIBS=-lX11
  GLUICFLAGS+=`$(PKG_CONFIG) --cflags glu`
  STRIPFLAGS= -s
  EXTENDED_RE=-r
endif

ifneq ($(XWIN),)
  CC=$(XWIN)-gcc
  CXX=$(XWIN)-g++
  STRIP=$(XWIN)-strip
  LV2LDFLAGS=-Wl,-Bstatic -Wl,-Bdynamic -Wl,--as-needed
  LIB_EXT=.dll
  EXE_EXT=.exe
  PUGL_SRC=$(RW)pugl/pugl_win.cpp
  PKG_GL_LIBS=
  UI_TYPE=ui:WindowsUI
  GLUILIBS=-lws2_32 -lwinmm -lopengl32 -lglu32 -lgdi32 -lcomdlg32 -lpthread
  GLUICFLAGS=-I.
  override LDFLAGS += -static-libgcc -static-libstdc++
endif

ifeq ($(EXTERNALUI), yes)
  UI_TYPE=
endif

ifeq ($(UI_TYPE),)
  UI_TYPE=kx:Widget
  LV2UIREQ+=lv2:requiredFeature kx:Widget;
  override CFLAGS += -DXTERNAL_UI
endif

targets+=$(BUILDDIR)$(LV2NAME)$(LIB_EXT)

UITTL=
ifneq ($(BUILDOPENGL), no)
  targets+=$(BUILDDIR)$(LV2GUI)$(LIB_EXT)
  UITTL=ui:ui $(LV2NAME):ui_gl ;
endif

###############################################################################
# check for build-dependencies

ifeq ($(shell $(PKG_CONFIG) --exists lv2 || echo no), no)
  $(error "LV2 SDK was not found")
endif

ifeq ($(shell $(PKG_CONFIG) --atleast-version=1.6.0 lv2 || echo no), no)
  $(error "LV2 SDK needs to be version 1.6.0 or later")
endif

ifneq ($(BUILDOPENGL)$(BUILDJACKAPP), nono)
 ifeq ($(shell $(PKG_CONFIG) --exists pango cairo $(PKG_GL_LIBS) || echo no), no)
  $(error "This plugin requires cairo pango $(PKG_GL_LIBS)")
 endif
endif

ifneq ($(BUILDJACKAPP), no)
 ifeq ($(shell $(PKG_CONFIG) --exists jack || echo no), no)
  $(warning *** libjack from http://jackaudio.org is required)
  $(error   Please install libjack-dev or libjack-jackd2-dev)
 endif
 JACKAPP=$(BINDIR)x42-ms_matrix$(EXE_EXT)
 targets+=$(BINDIR)ms_matrix.h
endif

# check for lv2_atom_forge_object  new in 1.8.1 deprecates lv2_atom_forge_blank
ifeq ($(shell $(PKG_CONFIG) --atleast-version=1.8.1 lv2 && echo yes), yes)
  override CFLAGS += -DHAVE_LV2_1_8
endif

ifneq ($(BUILDOPENGL)$(BUILDJACKAPP), nono)
  ifeq ($(wildcard $(RW)robtk.mk),)
    $(warning "**********************************************************")
    $(warning This plugin needs https://github.com/x42/robtk)
    $(warning "**********************************************************")
    $(info )
    $(info set the RW environment variale to the location of the robtk headers)
    ifeq ($(wildcard .git),.git)
      $(info or run 'make submodules' to initialize robtk as git submodule)
    endif
    $(info )
    $(warning "**********************************************************")
    $(error robtk not found)
  endif
endif

# LV2 idle >= lv2-1.6.0
GLUICFLAGS+=-DHAVE_IDLE_IFACE
LV2UIREQ+=lv2:requiredFeature ui:idleInterface; lv2:extensionData ui:idleInterface;

# add library dependent flags and libs
override CFLAGS += $(OPTIMIZATIONS) -DVERSION="\"v0.0.1\""
override CFLAGS += `$(PKG_CONFIG) --cflags lv2`

ifeq ($(XWIN),)
override CFLAGS += -fPIC -fvisibility=hidden
else
override CFLAGS += -DPTW32_STATIC_LIB
endif
override LOADLIBES += `$(PKG_CONFIG) --libs lv2`


GLUICFLAGS+=`$(PKG_CONFIG) --cflags cairo pango` $(CFLAGS)
GLUILIBS+=`$(PKG_CONFIG) $(PKG_UI_FLAGS) --libs cairo pango pangocairo $(PKG_GL_LIBS)`

ifneq ($(XWIN),)
GLUILIBS+=-lpthread -lusp10
endif

GLUILIBS+=$(LIC_LOADLIBES)

override CFLAGS += -I$(RW)


ROBGL+= Makefile

JACKCFLAGS=-I. $(CFLAGS)
JACKCFLAGS+=`$(PKG_CONFIG) --cflags jack lv2 pango pangocairo $(PKG_GL_LIBS)`
JACKLIBS=-lm $(GLUILIBS) $(LOADLIBES)

###############################################################################
# build target definitions
default: all

all: $(BUILDDIR)manifest.ttl $(BUILDDIR)$(LV2NAME).ttl $(targets) $(JACKAPP)

$(BUILDDIR)manifest.ttl: lv2ttl/manifest.ttl
	@mkdir -p $(BUILDDIR)
	cp lv2ttl/manifest.ttl $(BUILDDIR)manifest.ttl

$(BUILDDIR)$(LV2NAME).ttl: lv2ttl/$(LV2NAME).ttl
	cp lv2ttl/$(LV2NAME).ttl $(BUILDDIR)$(LV2NAME).ttl

DSP_SRC = src/$(LV2NAME).c
DSP_DEPS = $(DSP_SRC) src/$(LV2NAME).h
GUI_DEPS = gui/$(LV2NAME).c src/$(LV2NAME).h

$(BUILDDIR)$(LV2NAME)$(LIB_EXT): $(DSP_DEPS) Makefile
	$(CXX) $(CPPFLAGS) $(CFLAGS) -std=c99 \
	  -o $(BUILDDIR)$(LV2NAME)$(LIB_EXT) $(DSP_SRC) \
	  -shared $(LV2LDFLAGS) $(LDFLAGS) $(LOADLIBES) $(LIC_LOADLIBES)
	$(STRIP) $(STRIPFLAGS) $(BUILDDIR)$(LV2NAME)$(LIB_EXT)

$(BINDIR)ms_matrix.h:
	@mkdir -p $(BINDIR)
	cp src/ms_matrix-jack.h $(BINDIR)ms_matrix.h

jackapps: $(JACKAPP)

$(eval x42_ms_matrix_JACKSRC = src/ms_matrix.c)
$(eval x42_ms_matrix_JACKGUI = gui/ms_matrix.c)
$(eval x42_ms_matrix_LV2HTTL = $(BINDIR)ms_matrix.h)
$(eval x42_ms_matrix_JACKDESC = lv2ui_descriptor)

$(BINDIR)x42-ms_matrix$(EXE_EXT): $(DSP_DEPS) $(GUI_DEPS) \
	$(BINDIR)ms_matrix.h \
	$(x42_ms_matrix_JACKGUI) $(x42_ms_matrix_LV2HTTL)


ifneq ($(BUILDOPENGL)$(BUILDJACKAPP), nono)
 -include $(RW)robtk.mk
endif

###############################################################################
# install/uninstall/clean target definitions

install:
	ln -s $(PWD)/build ~/.lv2/ms_matrix.lv2

clean:
	rm -rf $(BUILDDIR)
	rm -rf $(BINDIR)
	rm -rf ~/.lv2/ms_matrix.lv2

.PHONY: clean all install

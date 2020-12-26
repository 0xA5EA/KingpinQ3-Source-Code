# KingpinQ3 Makefile
#
# GNU Make required

COMPILE_PLATFORM=$(shell uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]')
#GCC_VERSTR = $(shell $(CC) -dumpversion)
#GCC_VER1 := $(shell expr `$(CC) -dumpversion | cut -f1 -d.`)
#GCC_VER2 := $(shell expr `$(CC) -dumpversion | cut -f2 -d.`)
#GCC_VER3 := $(shell expr `$(CC) -dumpversion | cut -f3 -d.`)
#FIXME: warning on som plattforms
COMPILE_ARCH=$(shell uname -m | sed -e s/i.86/i386/)

ifeq ($(COMPILE_PLATFORM),sunos)
  # Solaris uname and GNU uname differ
  COMPILE_ARCH=$(shell uname -p | sed -e s/i.86/i386/)
endif
ifeq ($(COMPILE_PLATFORM),darwin)
  # Apple does some things a little differently...
  COMPILE_ARCH=$(shell uname -p | sed -e s/i.86/i386/)
endif

ifeq ($(COMPILE_PLATFORM),mingw32)
  ifeq ($(COMPILE_ARCH),i386)
    COMPILE_ARCH=x86
  endif
  ifeq ($(COMPILE_ARCH),x86_64)
    COMPILE_ARCH=x64
  endif
endif

ifndef BUILD_STANDALONE
  BUILD_STANDALONE =
  LCCFLAGS =
else
  LCCFLAGS = -DSTANDALONE
endif
LCCFLAGS = -DSTANDALONE
ifndef BUILD_CLIENT
  BUILD_CLIENT = 1
endif
ifndef BUILD_CLIENT_SMP
  BUILD_CLIENT_SMP = 0
endif
ifndef BUILD_SERVER
  BUILD_SERVER = 0
endif
ifndef BUILD_KMAP2
	BUILD_KMAP2 = 0
endif
ifndef BUILD_KAAS
BUILD_KAAS = 0
endif
ifndef BUILD_GAME_SO
  BUILD_GAME_SO = 0
endif
ifndef BUILD_GAME_QVM
  BUILD_GAME_QVM = 0
endif

ifndef USE_C_DLL
  USE_C_DLL = 1
endif
ifndef USE_CPLUS_0X
  USE_CPLUS_0X = 0
endif
ifneq ($(PLATFORM),darwin)
  BUILD_CLIENT_SMP = 0
endif

#############################################################################
#
# If you require a different configuration from the defaults below, create a
# new file named "Makefile.local" in the same directory as this file and define
# your parameters there. This allows you to change configuration without
# causing problems with keeping up to date with the repository.
#
#############################################################################

ifndef PLATFORM
  PLATFORM=$(COMPILE_PLATFORM)
endif
export PLATFORM

ifeq ($(COMPILE_ARCH),powerpc)
  COMPILE_ARCH=ppc
endif
ifeq ($(COMPILE_ARCH),powerpc64)
  COMPILE_ARCH=ppc64
endif

ifndef ARCH
  ARCH=$(COMPILE_ARCH)
endif
export ARCH

ifneq ($(PLATFORM),$(COMPILE_PLATFORM))
  CROSS_COMPILING=1
else
  CROSS_COMPILING=0
  ifneq ($(ARCH),$(COMPILE_ARCH))
    CROSS_COMPILING=1
  endif
endif
export CROSS_COMPILING
ifeq ($(CROSS_COMPILING), 1)
  -include ConfigMinGw.mk
else
  -include Config.mk
  SLD=ar
  CCC=g++
endif
ifndef COPYDIR
  COPYDIR="/usr/local/games/kingpinq3"
endif

ifndef COPYBINDIR
  COPYBINDIR=$(COPYDIR)
endif

ifndef MOUNT_DIR
  MOUNT_DIR=code
endif

ifndef BUILD_DIR
  BUILD_DIR=build
endif

ifndef TEMPDIR
TEMPDIR=/tmp
endif
ifndef GENERATE_DEPENDENCIES
  GENERATE_DEPENDENCIES=1
endif

ifndef USE_OPENAL
  USE_OPENAL=1
endif

ifndef USE_OPENAL_DLOPEN
  USE_OPENAL_DLOPEN=1
endif

ifndef USE_FREETYPE
  USE_FREETYPE=1
endif

ifndef USE_FREETYPE_BUILDIN
  USE_FREETYPE_BUILDIN=0
endif

ifndef USE_TCMALLOC
  USE_TCMALLOC=0
endif

ifndef USE_CURL
  USE_CURL=1
endif

ifndef USE_GPROF_PROFILING
  USE_GPROF_PROFILING=0
endif

ifndef USE_CURL_DLOPEN
  ifeq ($(PLATFORM),mingw32)
    USE_CURL_DLOPEN=1
  else
    USE_CURL_DLOPEN=1
  endif
endif

ifndef USE_CODEC_VORBIS
  USE_CODEC_VORBIS=0
  ifndef USE_INTERNAL_VORBIS
    USE_INTERNAL_VORBIS=1
  endif
endif

ifndef USE_CIN_THEORA
  USE_CIN_THEORA=1
endif

ifndef USE_CIN_THEORA_BUILDIN
  USE_CIN_THEORA_BUILDIN=0
endif

ifndef USE_MUMBLE
  USE_MUMBLE=1
endif

ifndef USE_VOIP
  USE_VOIP=1
endif

ifndef USE_INTERNAL_PNG
USE_INTERNAL_PNG=0
endif

ifndef USE_INTERNAL_SPEEX
  USE_INTERNAL_SPEEX=1
endif

ifndef USE_INTERNAL_ZLIB
  USE_INTERNAL_ZLIB=1
endif

ifndef USE_LOCAL_HEADERS
  USE_LOCAL_HEADERS=1
endif

ifndef DEBUG_CFLAGS
  DEBUG_CFLAGS=-g3 -dH 
endif

ifndef USE_OLD_VM64
USE_OLD_VM64=0
endif

ifndef USE_SSE
  USE_SSE=0
endif

ifndef USE_ASM_LIB
  USE_ASM_LIB=0
endif

ifndef USE_OPENMP
  USE_OPENMP=0
endif
ifndef USE_INTERNAL_GLEW
  USE_INTERNAL_GLEW=0
endif

ifndef USE_UNSAFE_OPTIMIZATIONS
  USE_UNSAFE_OPTIMIZATIONS=0
endif
#############################################################################

BD=$(BUILD_DIR)/debug-$(PLATFORM)-$(ARCH)
BR=$(BUILD_DIR)/release-$(PLATFORM)-$(ARCH)
CDIR=$(MOUNT_DIR)/client
SDIR=$(MOUNT_DIR)/server
RDIR=$(MOUNT_DIR)/renderer
CMDIR=$(MOUNT_DIR)/qcommon
SDLDIR=$(MOUNT_DIR)/sdl
ASMDIR=$(MOUNT_DIR)/asm
SYSDIR=$(MOUNT_DIR)/sys
GDIR=$(MOUNT_DIR)/game
CGDIR=$(MOUNT_DIR)/cgame
BLIBDIR=$(MOUNT_DIR)/botlib
NDIR=$(MOUNT_DIR)/null
UIDIR=$(MOUNT_DIR)/ui
JPDIR=$(MOUNT_DIR)/jpeg-8c
SPEEXDIR=$(MOUNT_DIR)/libspeex
ZDIR=$(MOUNT_DIR)/zlib
VORBISDIR=$(MOUNT_DIR)/vorbis
THEORADIR=$(MOUNT_DIR)/theora
OGGDIR=$(MOUNT_DIR)/ogg
Q3ASMDIR=$(MOUNT_DIR)/tools/asm
LBURGDIR=$(MOUNT_DIR)/tools/lcc/lburg
Q3CPPDIR=$(MOUNT_DIR)/tools/lcc/cpp
Q3LCCETCDIR=$(MOUNT_DIR)/tools/lcc/etc
Q3LCCSRCDIR=$(MOUNT_DIR)/tools/lcc/src
LOKISETUPDIR=misc/setup
NSISDIR=misc/nsis
SDLHDIR=$(MOUNT_DIR)/SDL12
LIBSDIR=$(MOUNT_DIR)/libs
ASMLIBDIR=$(MOUNT_DIR)/asmlib
TEMPDIR=/tmp
KMAP2DIR=kmap2
FTDIR=$(MOUNT_DIR)/freetype
PNGDIR=$(MOUNT_DIR)/png
KAASDIR=$(MOUNT_DIR)/kaas
GLEWDIR=$(MOUNT_DIR)/glew

bin_path=$(shell which $(1) 2> /dev/null)
# We won't need this if we only build the server
ifneq ($(BUILD_CLIENT),0)
  # set PKG_CONFIG_PATH to influence this, e.g.
  # PKG_CONFIG_PATH=/opt/cross/i386-mingw32msvc/lib/pkgconfig
  ifneq ($(call bin_path, pkg-config),)
    CURL_CFLAGS=$(shell pkg-config --silence-errors --cflags libcurl)
    CURL_LIBS=$(shell pkg-config --silence-errors --libs libcurl)
    OPENAL_CFLAGS=$(shell pkg-config --silence-errors --cflags openal)
    PNG_CFLAGS=$(shell pkg-config --silence-errors --cflags libpng)
    GLEW_FLAGS=$(shell pkg-config --silence-errors --cflags glew)
    OPENAL_LIBS=$(shell pkg-config --silence-errors --libs openal)
    SDL_CFLAGS=$(shell pkg-config --silence-errors --cflags sdl|sed 's/-Dmain=SDL_main//')
    SDL_LIBS=$(shell pkg-config --silence-errors --libs sdl)
    PNG_LIBS=$(shell pkg-config --silence-errors --libs libpng)
    GLEW_LIBS=$(shell pkg-config --silence-errors --libs glew)
    THEORA_LIBS=$(shell pkg-config --silence-errors --libs theora)
    THEORA_FLAGS=$(shell pkg-config --silence-errors --cflags theora)
    #FIXME: pkg-config --cflags theora pkg-config --libs theora
  endif
  # Use sdl-config if all else fails
  ifeq ($(SDL_CFLAGS),)
    ifneq ($(call bin_path, sdl-config),)
      SDL_CFLAGS=$(shell sdl-config --cflags)
      SDL_LIBS=$(shell sdl-config --libs)
    endif
  endif
endif

# version info
VERSION=""

USE_SVN=1
ifeq ($(wildcard .svn),.svn)
  SVN_REV=$(shell LANG=C svnversion .)
  ifneq ($(SVN_REV),)
    VERSION:=$(VERSION)SVN$(SVN_REV)
    USE_SVN=1
  endif
else
  ifeq ($(wildcard .git/svn/.metadata),.git/svn/.metadata)
    SVN_REV=$(shell LANG=C git svn info | awk '$$1 == "Revision:" {print $$2; exit 0}')
    ifneq ($(SVN_REV),)
      VERSION:=$(VERSION)_SVN$(SVN_REV)
    endif
  endif
endif

#############################################################################
# SETUP AND BUILD -- LINUX
#############################################################################

## Defaults
LIB=lib
CCP=cpp
INSTALL=install
MKDIR=mkdir

ifneq (,$(findstring "$(PLATFORM)", "linux" "gnu_kfreebsd" "kfreebsd-gnu"))

  ifeq ($(ARCH),axp)
    ARCH=alpha
  else
    ifeq ($(ARCH),x86_64)
      LIB=lib64
    else
      ifeq ($(ARCH),ppc64)
       LIB=lib64
      else
        ifeq ($(ARCH),s390x)
          LIB=lib64
        endif
      endif
    endif
  endif
  ifeq ($(USE_GPROF_PROFILING),1)
    DEBUG_CFLAGS += -pg
  endif
  SHLIBLIBS=
  LANGFLAGS = -x c
  SPECIALCC_FLAGS = -fno-rtti -fno-exceptions -Wno-write-strings 
  SPECIALSOCC_FLAGS = -fno-rtti -Wno-write-strings -fno-exceptions
  ifeq ($(USE_CPLUS_0X),1)
    SPECIALCC_FLAGS += -std=c++0x
    SPECIALSOCC_FLAGS += -std=c++0x
  endif
  SPECIALC_FLAGS = -Wstrict-prototypes
 
  BASE_CFLAGS = -Wall -pipe -DUSE_ICON -fno-strict-aliasing -Wno-reorder
  # -fno-builtin
  CLIENT_CFLAGS = $(SDL_CFLAGS) 
  SERVER_CFLAGS =
  ifeq ($(USE_CPLUS_0X),1)
    CLIENT_CFLAGS += -std=c++0x
  endif
  ifeq ($(USE_GPROF_PROFILING),1)
    CLIENT_LDFLAGS= -pg
    CLIENT_CFLAGS += -pg
  endif
  
  #FIXME: why no warings when compiling jpeg lib (no-rtti)
  ifeq ($(USE_OPENAL),1)
    CLIENT_CFLAGS += -DUSE_OPENAL
    ifeq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_CFLAGS += -DUSE_OPENAL_DLOPEN
    endif
  endif

  ifeq ($(USE_CURL),1)
    CLIENT_CFLAGS += -DUSE_CURL
    ifeq ($(USE_CURL_DLOPEN),1)
      CLIENT_CFLAGS += -DUSE_CURL_DLOPEN
    endif
  endif

  ifeq ($(USE_CODEC_VORBIS),1)
    CLIENT_CFLAGS += -DUSE_CODEC_VORBIS
    ifeq ($(USE_INTERNAL_VORBIS),1)
      CLIENT_CFLAGS += -I$(VORBISDIR)/include -I$(OGGDIR)/include -I$(VORBISDIR)/lib
    endif
  endif
  ifeq ($(USE_CIN_THEORA),1)
    CLIENT_CFLAGS += -DUSE_CIN_THEORA
    ifeq ($(USE_LOCAL_HEADERS),1)
      CLIENT_CFLAGS += -I$(THEORADIR)/include
    else
      ifeq ($(USE_CIN_THEORA_BUILDIN),1)
        CLIENT_CFLAGS += -I$(THEORADIR)/include
      else
        CLIENT_CFLAGS += $(THEORA_FLAGS)
      endif
    endif
  endif

  #FIXME: optimize overwritten
  OPTIMIZEVM = -O2 -DNDEBUG -ffast-math  #-fexcess-precision=fast
  OPTIMIZE = $(OPTIMIZEVM) 
  ifeq ($(USE_CPLUS_0X),1)
   OPTIMIZEVM = -O2 -fexcess-precision=fast -ffast-math -DNDEBUG 
   OPTIMIZE = $(OPTIMIZEVM)
  endif

  # kmap2 optimization flags
  KMAP2_OPTIMIZE = -O3 -DNDEBUG -ffast-math #-fexcess-precision=fast
  KAAS_OPTIMIZE =  -O2 -DNDEBUG
  ifeq ($(USE_CPLUS_0X),1)
    KMAP2_OPTIMIZE += -O3 -fexcess-precision=fast -ffast-math 
    KAAS_OPTIMIZE += -O2 -fexcess-precision=fast -ffast-math 
  endif

  KMAP2_CFLAGS += -pthread -I$(KMAP2DIR)/common -I/usr/include/libxml2 $(shell pkg-config --silence-errors --cflags glib-2.0)
  KMAP2_CFLAGS += -I$(KMAP2DIR)/libs
  KMAP2_CFLAGS += -I$(KMAP2DIR)/zlib
  KMAP2_CFLAGS += -I$(KMAP2DIR)/jpeg-6
  KMAP2_CFLAGS += -DKMAP2 -D_CONSOLE
  # KMAP2_CFLAGS += -m64
  KMAP2_CFLAGS += $(KMAP2_OPTIMIZE)
  KMAP2_LIBS = -lxml2 -lpthread -lm $(shell pkg-config --silence-errors  --libs glib-2.0)

  KAAS_CFLAGS = -I$(KAASDIR) -I/usr/include -Wall 
  KAAS_CFLAGS += -pthread -DBSPC  -D_CONSOLE
  KAAS_CFLAGS += $(KAAS_OPTIMIZE)
  KAAS_LIBS = -lpthread -lm 

  ifeq ($(USE_FREETYPE), 1)
    CLIENT_CFLAGS += -DFT2_BUILD_LIBRARY
    CLIENT_CFLAGS += -DBUILD_FREETYPE
    CLIENT_CFLAGS += -Icode/freetype/include
  endif
  CLIENT_CFLAGS += -Icode/asmlib
 
  ifeq ($(USE_ASM_LIB),1)
    CLIENT_CFLAGS+=-DUSE_ASM_LIB
    SERVER_CFLAGS+=-DUSE_ASM_LIB
    NOTSHLIBCFLAGS=-DUSE_ASM_LIB
    KAAS_CFLAGS+=-DUSE_ASM_LIB
    ifneq ($(ARCH),x86_64)
      SHLIBCCFLAGS=-DUSE_ASM_LIB
    endif
  endif
  ifeq ($(USE_OPENMP), 1)
    CLIENT_CFLAGS += -fopenmp
    NOTSHLIBCFLAGS += -fopenmp
  endif

  ifeq ($(ARCH),x86_64)
    OPTIMIZEVM = -O2 -m64 -ffast-math 
    ifeq ($(USE_CPLUS_0X),1)
      OPTIMIZEVM = -O2 -m64 -fexcess-precision=fast -ffast-math 
    endif
    ifeq ($(USE_GPROF_PROFILING),1)
      OPTIMIZE += -g
    endif
    # FIXME:
    # should be added when using gcc 4.5.0 -fexcess-precision=fast 
    #untested flags  -funswitch-loops 
    # -fno-builtin -Winline --param inline-unit-growth=120 -finline-limit=3600 #-Winline

    ifeq ($(USE_SSE), 1)
      OPTIMIZE += -msse2
      CLIENT_CFLAGS += -DUSE_SSE
	  SERVER_CFLAGS+=-DUSE_SSE
      NOTSHLIBCFLAGS +=-DUSE_SSE
	  KAAS_CFLAGS += -DUSE_SSE
    endif

    ifeq ($(USE_INTERNAL_PNG),0) 
      CLIENT_CFLAGS += -I/usr/include/libpng12
    else
      CLIENT_CFLAGS += -Icode/png -DUSE_INTERNAL_PNG
    endif
    
    HAVE_VM_COMPILED = true
    ifeq ($(USE_ASM_LIB),1)
      CLIENT_LIBS=$(ASMLIBDIR)/alibelf64.a
      DED_LIBS=$(ASMLIBDIR)/alibelf64.a
      KAAS_LIBS+=$(ASMLIBDIR)/alibelf64o.a
      KMAP2_LIBS+=$(ASMLIBDIR)/alibelf64o.a
	endif
  else
    ifeq ($(ARCH),i386)
	OPTIMIZE = -O2 -ffast-math
#  	  OPTIMIZE = -O2 -fexcess-precision=fast -ffast-math 
      ifeq ($(USE_GPROF_PROFILING),1)
        OPTIMIZE += -g
      endif
      ifeq ($(USE_SSE), 1)
	    OPTIMIZE += -msse3
	    CLIENT_CFLAGS += -DUSE_SSE
        NOTSHLIBCFLAGS +=-DUSE_SSE
        KAAS_CFLAGS += -DUSE_SSE
      endif
      HAVE_VM_COMPILED=true
      ifeq ($(USE_ASM_LIB),1)
        CLIENT_LIBS=$(ASMLIBDIR)/alibelf32.a
	    DED_LIBS=$(ASMLIBDIR)/alibelf32.a
	    KAAS_LIBS+=$(ASMLIBDIR)/alibelf32o.a
	    SHLIBLIBS=$(ASMLIBDIR)/alibelf32p.a
	    KMAP2_LIBS+=$(ASMLIBDIR)/alibelf32o.a
	  endif
    else
      ifeq ($(ARCH),ppc)
        BASE_CFLAGS += -maltivec
        HAVE_VM_COMPILED=true
      endif
      ifeq ($(ARCH),ppc64)
        BASE_CFLAGS += -maltivec
        HAVE_VM_COMPILED=true
      endif
      ifeq ($(ARCH),sparc)
        OPTIMIZE += -mtune=ultrasparc3 -mv8plus
        OPTIMIZEVM += -mtune=ultrasparc3 -mv8plus
        HAVE_VM_COMPILED=true
      endif
  ifeq ($(ARCH),alpha)
    # According to http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=410555
    # -ffast-math will cause the client to die with SIGFPE on Alpha
    OPTIMIZE = $(OPTIMIZEVM)
  endif
    endif
  endif

  ifeq ($(USE_TCMALLOC), 1)
    BASE_CFLAGS  += -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
    #LIBS += -ltcmalloc
    DED_LIBS+=-ltcmalloc_minimal
    CLIENT_LIBS+=-ltcmalloc_minimal
    #LIBS += -ltcmalloc_minimal
  endif
  ifneq ($(HAVE_VM_COMPILED),true)
    BASE_CFLAGS += -DNO_VM_COMPILED
  endif

  CLIENT_CFLAGS += -Icode/jpeg-8c
  CLIENT_CFLAGS += -DUSE_INTERNAL_JPG
  CLIENT_CFLAGS += -DCOMPAT_KPQ3
  CLIENT_LIBS += $(B)/kpq3jpeg8c.a

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC -fvisibility=hidden
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  STATICLIBCFLAGS=$(CLIENT_CFLAGS) --static
  STATICLIBLDFLAGS=-static
  THREAD_LIBS=-lpthread

  #FIXME
  #SOTHREAD_LIBS=-lboost_thread
  
  LIBS+=-ldl -lm

  CLIENT_LIBS += $(SDL_LIBS) -lGL 
  ifneq ($(USE_CIN_THEORA),0)
    ifeq ($(USE_CIN_THEORA_BUILDIN),0)
      CLIENT_LIBS += $(THEORA_LIBS)
    endif
  endif
 
   ifeq ($(USE_INTERNAL_PNG),0)
    CLIENT_LIBS += $(PNG_LIBS)
  endif

  ifeq ($(USE_INTERNAL_GLEW),0)
    CLIENT_CFLAGS+= $(GLEW_FLAGS)
   	CLIENT_LIBS += $(GLEW_LIBS)
  endif
  ifeq ($(USE_OPENAL),1)
    ifneq ($(USE_OPENAL_DLOPEN),1)
	  CLIENT_LIBS += -lopenal
    endif
  endif

  ifeq ($(USE_CURL),1)
    ifneq ($(USE_CURL_DLOPEN),1)
	    CLIENT_LIBS += -lcurl
    endif
  endif

  ifeq ($(USE_CODEC_VORBIS),1)
    ifeq ($(USE_INTERNAL_VORBIS),0)
	    CLIENT_LIBS += -lvorbisfile -lvorbis -logg
    endif
  endif

  #CLIENT_LIBS += -lboost_thread
  ifeq ($(USE_MUMBLE),1)
	  CLIENT_LIBS += -lrt
  endif
  ifeq ($(USE_FREETYPE),1)
	  ifeq ($(USE_FREETYPE_BUILDIN),0)
	    #CLIENT_LDFLAGS += $(shell freetype-config --libs)
	    CLIENT_LIBS += -lfreetype
	  endif
  endif

  ifeq ($(USE_LOCAL_HEADERS),1)
    CLIENT_CFLAGS += -I$(SDLHDIR)/include
    CLIENT_CFLAGS += -I$(GLEWDIR)/include
  endif
  ifeq ($(USE_INTERNAL_GLEW),1)
    CLIENT_CFLAGS += -I$(GLEWDIR)/include -DUSE_INTERNAL_GLEW
  endif
  ifeq ($(ARCH),i386)
    # linux32 make ...
    BASE_CFLAGS += -m32
  else
    ifeq ($(ARCH),ppc64)
      BASE_CFLAGS += -m64
    endif
  endif

  ifeq ($(USE_UNSAFE_OPTIMIZATIONS),1)
    OPTIMIZE+=-funsafe-loop-optimizations -funsafe-math-optimizations
  endif
  SHLIBCCFLAGS += $(SHLIBCFLAGS) $(SPECIALSOCC_FLAGS)
  LD=g++

else # ifeq Linux

#############################################################################
# SETUP AND BUILD -- MINGW32
#############################################################################

ifeq ($(PLATFORM),mingw32)
  # Some MinGW installations define CC to cc, but don't actually provide cc,
  # so explicitly use gcc instead (which is the only option anyway)
  ifeq ($(call bin_path, $(CC)),)
    CC=gcc
  endif
  SHLIBLIBS=
  ifndef WINDRES
    WINDRES=windres
  endif
  LANGFLAGS = -x c
  SPECIALCC_FLAGS = -fno-rtti -fno-exceptions  -Wno-write-strings
  SPECIALSOCC_FLAGS = -fno-rtti -Wno-write-strings -fno-exceptions
  SPECIALC_FLAGS = -Wstrict-prototypes 

  BASE_CFLAGS = -Wall -fno-strict-aliasing -DUSE_ICON 

  CLIENT_CFLAGS = -DSDL_VIDEO_DRIVER_WINDIB 

  SERVER_CFLAGS =

  # In the absence of wspiapi.h, require Windows XP or later
  ifeq ($(shell test -e $(CMDIR)/wspiapi.h; echo $$?),1)
    BASE_CFLAGS += -DWINVER=0x501
    # xp = 0x501
  endif

  #BASE_CFLAGS += -D_WIN32
  ifeq ($(USE_FREETYPE), 1)
    CLIENT_CFLAGS += -DFT2_BUILD_LIBRARY
    CLIENT_CFLAGS += -DBUILD_FREETYPE
    CLIENT_CFLAGS += -Icode/freetype/include
  endif

  ifeq ($(USE_OPENAL),1)
    CLIENT_CFLAGS += -DUSE_OPENAL
    CLIENT_CFLAGS += $(OPENAL_CFLAGS)
    ifeq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_CFLAGS += -DUSE_OPENAL_DLOPEN
    else
      CLIENT_LDFLAGS += $(OPENAL_LDFLAGS)
    endif
  endif

  ifeq ($(USE_CODEC_VORBIS),1)
    CLIENT_CFLAGS += -DUSE_CODEC_VORBIS
    CLIENT_CFLAGS += -I$(VORBISDIR)/include -I$(OGGDIR)/include -I$(VORBISDIR)/lib
  endif

  ifeq ($(ARCH),x64)
    OPTIMIZEVM = -O2 -fno-omit-frame-pointer \
      -falign-loops=2 -funroll-loops -falign-jumps=2 -falign-functions=2 \
      -fstrength-reduce
    OPTIMIZE = $(OPTIMIZEVM) --fast-math 
    HAVE_VM_COMPILED = true
  endif
  # -fexcess-precision=fast 
  ifeq ($(ARCH),x86)
    OPTIMIZEVM = -O2 -march=i586 -fno-omit-frame-pointer \
      -falign-loops=2 -funroll-loops -falign-jumps=2 -falign-functions=2 \
      -fstrength-reduce
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
    # -mpreferred-stack-boundary=4
    ifeq ($(USE_SSE), 1)
      OPTIMIZE += -msse2
      CLIENT_CFLAGS += -DUSE_SSE
      NOTSHLIBCFLAGS +=-DUSE_SSE
	  SERVER_CFLAGS+=-DUSE_SSE
    endif
    HAVE_VM_COMPILED = true
  endif

  KMAP2_OPTIMIZE = -O2 -DNDEBUG 
  KAAS_OPTIMIZE =  -O2 -DNDEBUG
  ifeq ($(USE_CPLUS_0X),1)
    KMAP2_OPTIMIZE += -O2 -ffast-math 
    KAAS_OPTIMIZE += -O2  -ffast-math 
  endif

  KMAP2_CFLAGS += -pthread -I$(KMAP2DIR)/common -I/usr/include/libxml2 $(shell pkg-config --silence-errors --cflags glib-2.0)
  KMAP2_CFLAGS += -I$(KMAP2DIR)/libs
  KMAP2_CFLAGS += -I$(KMAP2DIR)/zlib
  KMAP2_CFLAGS += -I$(KMAP2DIR)/jpeg-6
  KMAP2_CFLAGS += -DKMAP2 -D_CONSOLE
  # KMAP2_CFLAGS += -m64
  KMAP2_CFLAGS += $(KMAP2_OPTIMIZE)
  KMAP2_LIBS = -lxml2 -lpthread $(shell pkg-config --silence-errors  --libs glib-2.0)

  KAAS_CFLAGS = -I$(KAASDIR) -Wall 
  KAAS_CFLAGS += -DBSPC  -D_CONSOLE
  KAAS_CFLAGS += $(KAAS_OPTIMIZE)
  KAAS_LIBS = -lws2_32 -lwinmm 

  SHLIBEXT=dll
  SHLIBCFLAGS=
  SHLIBCCFLAGS=-fno-rtti
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  BINEXT=.exe

  LIBS= -lws2_32 -lwinmm -lpsapi 
  CLIENT_LDFLAGS = -mwindows
  CLIENT_LIBS = -lgdi32 -lole32 -lopengl32

  ifeq ($(USE_CURL),1)
    CLIENT_CFLAGS += -DUSE_CURL
    CLIENT_CFLAGS += $(CURL_CFLAGS)
    ifneq ($(USE_CURL_DLOPEN),1)
      ifeq ($(USE_LOCAL_HEADERS),1)
        CLIENT_CFLAGS += -DCURL_STATICLIB
        ifeq ($(ARCH),x64)
	      CLIENT_LIBS += $(LIBSDIR)/win64/libcurl.a
	    else
          CLIENT_LIBS += $(LIBSDIR)/win32/libcurl.a
        endif
      else
        CLIENT_LIBS += $(CURL_LIBS)
      endif
    else
    	CLIENT_CFLAGS += -DUSE_CURL_DLOPEN
    endif
  endif

  ifeq ($(USE_CODEC_VORBIS),1)
    ifeq ($(USE_VORBIS_INTERNAL),0)
      CLIENT_LIBS += -lvorbisfile -lvorbis -logg
    else
 	 ifeq ($(ARCH),x86)
        CLIENT_LIBS += $(LIBSDIR)/win32/libogg.lib \
        			   $(LIBSDIR)/win32/libvorbis.lib \
        			   $(LIBSDIR)/win32/libvorbisfile.lib
      else
       CLIENT_LIBS += $(LIBSDIR)/win64/libogg.lib \
       				  $(LIBSDIR)/win64/libvorbis.lib \
	                  $(LIBSDIR)/win64/libvorbisfile.lib
      endif
    endif
  endif

  ifeq ($(USE_CIN_THEORA),1)
    CLIENT_CFLAGS += -DUSE_CIN_THEORA #-DUSE_CIN_XVID
    CLIENT_CFLAGS += -I$(THEORADIR)/include
    ##CLIENT_CFLAGS += $(THEORA_FLAGS)
    #FIXME: local and not local headers
  endif

  CLIENT_CFLAGS += -Icode/png
  ifeq ($(ARCH),x86)
    # build 32bit
    BASE_CFLAGS += -m32
  else
    BASE_CFLAGS += -m64
  endif
  ifeq ($(USE_ASM_LIB),1)
	  CLIENT_CFLAGS+=-DUSE_ASM_LIB
	  SERVER_CFLAGS+=-DUSE_ASM_LIB
	  NOTSHLIBCFLAGS=-DUSE_ASM_LIB
	  SHLIBCCFLAGS+=-DUSE_ASM_LIB
    ifeq ($(ARCH), x86)
      CLIENT_LIBS+=$(ASMLIBDIR)/alibcof32.lib
      DED_LIBS+=$(ASMLIBDIR)/alibcof32.lib
      SHLIBLIBS+=-L$(ASMLIBDIR) $(ASMLIBDIR)/alibd32.lib
    else
      CLIENT_LIBS+=$(ASMLIBDIR)/alibcof64.lib
      DED_LIBS+=$(ASMLIBDIR)/alibcof64.lib
      SHLIBLIBS+= -L$(ASMLIBDIR) $(ASMLIBDIR)/alibd64.lib
    endif
  endif	

  ifeq ($(USE_INTERNAL_GLEW),0)
    # CLIENT_LIBS += -lGLEW
    CLIENT_LIBS += -L$(GLEWDIR)/lib $(GLEWDIR)/lib/glew32mx.lib
  endif
  ifeq ($(USE_INTERNAL_GLEW),1)
    CLIENT_CFLAGS += -DGLEW_STATIC  -DUSE_INTERNAL_GLEW
  endif

  ifeq ($(USE_INTERNAL_PNG),0) 
    CLIENT_CFLAGS += -Icode/png
  else
    CLIENT_CFLAGS += -Icode/png -DUSE_INTERNAL_PNG
  endif

  CLIENT_CFLAGS += -Icode/jpeg-8c
  CLIENT_CFLAGS += -DUSE_INTERNAL_JPG
  CLIENT_LIBS += $(B)/kpq3jpeg8c.a

  ifeq ($(USE_INTERNAL_PNG),0)
    ifeq ($(ARCH), x64)
      CLIENT_LIBS += $(LIBSDIR)/win64/libpng14.lib
    else
      CLIENT_LIBS += $(LIBSDIR)/win32/libpng14.lib
    endif
  endif

  ifeq ($(USE_FREETYPE),1)
    ifeq ($(USE_FREETYPE_BUILDIN),0)
    ifeq ($(ARCH), x86)
      CLIENT_LIBS += $(LIBSDIR)/win32/freetype244MT.lib
    else
      CLIENT_LIBS += $(LIBSDIR)/win64/freetype244MT.lib
    endif
    endif
  endif

  # libmingw32 must be linked before libSDLmain
  CLIENT_LIBS += -lmingw32 
  ifeq ($(USE_LOCAL_HEADERS),1)
    CLIENT_CFLAGS += -I$(GLEWDIR)/include
    CLIENT_CFLAGS += -I$(SDLHDIR)/include
    ifeq ($(ARCH), x86)
      CLIENT_LIBS += $(LIBSDIR)/win32/SDLmain.lib \
                     $(LIBSDIR)/win32/SDL.lib
    else
      CLIENT_LIBS += $(LIBSDIR)/win64/SDLmain.lib \
                     $(LIBSDIR)/win64/SDL.lib
    endif
  else
    CLIENT_CFLAGS += $(SDL_CFLAGS)
    CLIENT_LIBS += $(SDL_LIBS)
  endif

  ifeq ($(USE_CIN_THEORA),1)
    ifeq ($(USE_CIN_THEORA_BUILDIN),0)
     ifeq ($(ARCH), x86)
        CLIENT_LIBS += $(LIBSDIR)/win32/libtheora.lib  
      else
        CLIENT_LIBS += $(LIBSDIR)/win64/libtheora.lib
      endif
    endif
  endif


  BUILD_CLIENT_SMP = 0

  SHLIBCCFLAGS += $(SHLIBCFLAGS) $(SPECIALSOCC_FLAGS)

endif # Linux
endif # mingw32

TARGETS =

ifndef FULLBINEXT
  FULLBINEXT=.$(ARCH)$(BINEXT)
endif
ifndef SHLIBNAME
  SHLIBNAME=$(ARCH).$(SHLIBEXT)
endif
ifneq ($(BUILD_SERVER),0)
  TARGETS += $(B)/kpq3ded$(FULLBINEXT)
endif

ifneq ($(BUILD_CLIENT),0)
  TARGETS += $(JPEGSLIB)
  TARGETS += $(B)/kingpinq3$(FULLBINEXT)
  ifneq ($(BUILD_CLIENT_SMP),0)
    TARGETS += $(B)/kingpinq3-smp$(FULLBINEXT)
  endif
endif

ifneq ($(BUILD_GAME_SO),0)
  TARGETS += \
    $(B)/basekpq3/cgame$(SHLIBNAME) \
    $(B)/basekpq3/qagame$(SHLIBNAME) \
    $(B)/basekpq3/ui$(SHLIBNAME)
endif

ifneq ($(BUILD_GAME_QVM),0)
  ifneq ($(CROSS_COMPILING),1)
    TARGETS += \
      $(B)/basekpq3/vm/cgame.qvm \
      $(B)/basekpq3/vm/qagame.qvm \
      $(B)/basekpq3/vm/ui.qvm
  endif
endif

ifneq ($(BUILD_KMAP2),0)
 TARGETS += $(B)/kmap2$(FULLBINEXT)
 #KMAP2_CFLAGS += -DPRODUCT_VERSION=\\\"$(VERSION)\\\"
endif

ifneq ($(BUILD_KAAS),0)
 TARGETS += $(B)/kaas$(FULLBINEXT)
endif

ifeq ($(USE_MUMBLE),1)
  CLIENT_CFLAGS += -DUSE_MUMBLE
endif

ifeq ($(USE_VOIP),1)
  CLIENT_CFLAGS += -DUSE_VOIP
  SERVER_CFLAGS += -DUSE_VOIP
  ifeq ($(USE_INTERNAL_SPEEX),1)
    CLIENT_CFLAGS += -DFLOATING_POINT -DUSE_ALLOCA -I$(SPEEXDIR)/include
  else
    CLIENT_LIBS += -lspeex -lspeexdsp
  endif
endif

ifeq ($(USE_INTERNAL_ZLIB),1)
  BASE_CFLAGS += -DNO_GZIP
  ifneq ($(USE_LOCAL_HEADERS),1)
    BASE_CFLAGS += -I$(ZDIR)
  endif
else
  ifeq ($(CROSS_COMPILING), 0)
  LIBS += -lz
  else
  ifeq ($(ARCH),x64)
    LIBS += $(LIBSDIR)/mingw64/libz.a
  else
    LIBS += $(LIBSDIR)/win32/zlib1.lib
  endif
  #FIXME
  endif
endif
ifdef DEFAULT_BASEDIR
  BASE_CFLAGS += -DDEFAULT_BASEDIR=\\\"$(DEFAULT_BASEDIR)\\\" 
  BASE_CCFLAGS += -DDEFAULT_BASEDIR=\\\"$(DEFAULT_BASEDIR)\\\" 
endif

ifeq ($(USE_LOCAL_HEADERS),1)
  BASE_CFLAGS += -DUSE_LOCAL_HEADERS
endif

ifeq ($(BUILD_STANDALONE),1)
  BASE_CFLAGS += -DSTANDALONE
endif

ifeq ($(GENERATE_DEPENDENCIES),1)
  DEPEND_CFLAGS = -MMD
else
  DEPEND_CFLAGS =
endif
ifeq ($(NO_STRIP),1)
  STRIP_FLAG =
else
  STRIP_FLAG = -s
endif

BASE_CFLAGS += -DPRODUCT_VERSION=\\\"$(VERSION)\\\"
BASE_CXXFLAGS = $(BASE_CFLAGS) -Wno-reorder
ifeq ($(V),1)
echo_cmd=@:
Q=
else
echo_cmd=@echo
Q=@
endif

define DO_CC
$(echo_cmd) "CC $<"
$(Q)$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) $(CLIENT_CFLAGS)  $(SPECIALC_FLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_CCC
$(echo_cmd) "CCC $<"
$(Q)$(CCC) $(NOTSHLIBCFLAGS) $(CFLAGS) $(CLIENT_CFLAGS)  $(SPECIALCC_FLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_SMP_CC
$(echo_cmd) "SMP_CC $<"
$(Q)$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) $(CLIENT_CFLAGS) $(OPTIMIZE) -DSMP -o $@ -c $<
endef

define DO_BOT_CCC
$(echo_cmd) "BOT_CCC $<"
$(Q)$(CCC) $(NOTSHLIBCFLAGS) $(CFLAGS) $(BOTCFLAGS) $(SPECIALCC_FLAGS) $(OPTIMIZE) -DBOTLIB -o $@ -c $<
endef

ifeq ($(GENERATE_DEPENDENCIES),1)
  DO_QVM_DEP=cat $(@:%.o=%.d) | sed -e 's/\.o/\.asm/g' >> $(@:%.o=%.d)
endif

define DO_SHLIB_CC
$(echo_cmd) "SHLIB_CC $<"
$(Q)$(CC) $(SHLIBCFLAGS) $(CFLAGS) $(SPECIALC_FLAGS) $(OPTIMIZEVM) $(LANGFLAGS) -o $@ -c $<
$(Q)$(DO_QVM_DEP)
endef

define DO_SHLIB_CCC
$(echo_cmd) "SHLIB_CCC $<"
$(Q)$(CCC) $(SHLIBCCFLAGS) $(CFLAGS) $(SPECIALCC_FLAGS) $(OPTIMIZEVM) -o $@ -c $<
$(Q)$(DO_QVM_DEP)
endef

define DO_GAME_CCC
$(echo_cmd) "GAME_CCC $<"
$(Q)$(CCC) -DQAGAME $(SHLIBCCFLAGS) $(CFLAGS) $(SPECIALCC_FLAGS) $(OPTIMIZEVM) -o $@ -c $<
$(Q)$(DO_QVM_DEP)
endef

define DO_GAME_CC
$(echo_cmd) "GAME_CC $<"
$(Q)$(CC) -DQAGAME $(SHLIBCFLAGS) $(SPECIALC_FLAGS) $(CFLAGS) $(OPTIMIZEVM) $(LANGFLAGS)  -o $@ -c $<
$(Q)$(DO_QVM_DEP)
endef

define DO_CGAME_CCC
$(echo_cmd) "CGAME_CCC $<"
$(Q)$(CCC) -DCGAME $(SHLIBCCFLAGS) $(CFLAGS)  $(OPTIMIZEVM) -o $@ -c $<
$(Q)$(DO_QVM_DEP)
endef

define DO_CGAME_CC
$(echo_cmd) "CGAME_CC $<"
$(Q)$(CC) -DCGAME $(SHLIBCFLAGS) $(SPECIALC_FLAGS) $(CFLAGS) $(OPTIMIZEVM) $(LANGFLAGS) -o $@ -c $<
$(Q)$(DO_QVM_DEP)
endef

define DO_UI_CC
$(echo_cmd) "UI_CC $<"
$(Q)$(CC) -DUI $(SHLIBCFLAGS) $(CFLAGS) $(SPECIALC_FLAGS)  $(OPTIMIZEVM) $(LANGFLAGS) -o $@ -c $<
$(Q)$(DO_QVM_DEP)
endef

define DO_UI_CCC
$(echo_cmd) "UI_CCC $<"
$(Q)$(CCC) -DUI $(SHLIBCCFLAGS) $(CFLAGS) $(SPECIALCC_FLAGS) $(OPTIMIZEVM) -o $@ -c $<
$(Q)$(DO_QVM_DEP)
endef

define DO_AS
$(echo_cmd) "AS $<"
$(Q)$(CC) $(CFLAGS) $(OPTIMIZE) -x assembler-with-cpp -o $@ -c $<
endef
#FIXME: BASE flags for as ?
define DO_DED_CC
$(echo_cmd) "DED_CC $<"
$(Q)$(CC) $(NOTSHLIBCFLAGS) -DDEDICATED $(CFLAGS) $(SERVER_CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_DED_CCC
$(echo_cmd) "DED_CCC $<"
$(Q)$(CCC) $(NOTSHLIBCFLAGS) -DDEDICATED $(CFLAGS) $(SPECIALCC_FLAGS) $(SERVER_CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_KMAP2_CC
$(echo_cmd) "KMAP2_CC $<"
$(Q)$(CC) $(KMAP2_CFLAGS) $(LANGFLAGS) -o $@ -c $<
endef

define DO_KAAS_CC
$(echo_cmd) "KAAS_CC $<"
$(Q)$(CC) $(KAAS_CFLAGS) $(LANGFLAGS) -o $@ -c $<
endef

define DO_WINDRES
$(echo_cmd) "WINDRES $<"
$(Q)$(WINDRES) -i $< -o $@
endef


#############################################################################
# MAIN TARGETS
#############################################################################

default: release
all: debug release

debug:
	@$(MAKE) targets B=$(BD) CFLAGS="$(CFLAGS) $(BASE_CFLAGS) $(DEPEND_CFLAGS)" \
	  OPTIMIZE="$(DEBUG_CFLAGS)" OPTIMIZEVM="$(DEBUG_CFLAGS)" \
	  CLIENT_CFLAGS="$(CLIENT_CFLAGS)" SERVER_CFLAGS="$(SERVER_CFLAGS)" V=$(V)

release:
	@$(MAKE) targets B=$(BR) CFLAGS="$(CFLAGS) $(BASE_CFLAGS) $(DEPEND_CFLAGS)" \
	  OPTIMIZE="-DNDEBUG $(OPTIMIZE)" OPTIMIZEVM="-DNDEBUG $(OPTIMIZEVM)" \
	  CLIENT_CFLAGS="$(CLIENT_CFLAGS)" SERVER_CFLAGS="$(SERVER_CFLAGS)" V=$(V)

# Create the build directories, check libraries and print out
# an informational message, then start building
targets: makedirs
	@echo ""
	@echo "Building KingpinQ3 in $(B):"
	@echo "  PLATFORM: $(PLATFORM)"
	@echo "  ARCH: $(ARCH)"
	@echo "  VERSION: $(VERSION)"
	@echo "  COMPILE_PLATFORM: $(COMPILE_PLATFORM)"
	@echo "  COMPILE_ARCH: $(COMPILE_ARCH)"
	@echo "  COMPILER VERS: $(GCC_VERSTR)"	
	@echo "  CC: $(CC)"
	@echo "  CPP: $(CPP)"
	@echo "  CCC: $(CCC)"

	@echo "  LD: $(LD)"
	@echo ""
	@echo "  BASE_CFLAGS:"
	-@for i in $(BASE_CFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo "  BASE_CCFLAGS:"
	-@for i in $(BASE_CCFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo "  CFLAGS:"
	-@for i in $(CFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo "  OPTIMIZE:"
	-@for i in $(OPTIMIZE); \
	do \
		echo "    $$i"; \
	done
ifeq ($(BUILD_CLIENT),1)
	@echo ""
	@echo " CLIENT_CFLAGS :"
	-@for i in $(CLIENT_CFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo " SPECIALCC_FLAGS :"
	-@for i in $(SPECIALCC_FLAGS); \
	do \
		echo "    $$i"; \
	done
endif
ifeq ($(BUILD_SERVER),1)
	@echo ""
	@echo "  SERVER_CFLAGS:"
	-@for i in $(SERVER_CFLAGS); \
	do \
		echo "    $$i"; \
	done
endif
	@echo ""
	@echo "  LDFLAGS:"
	-@for i in $(LDFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo "  LIBS:"
	-@for i in $(LIBS); \
	do \
		echo "    $$i"; \
	done
ifeq ($(BUILD_CLIENT),1)
	@echo ""
	@echo "  CLIENT_LIBS:"
	-@for i in $(CLIENT_LIBS); \
	do \
		echo "    $$i"; \
	done
endif
ifeq ($(BUILD_SERVER),1)
	@echo ""
	@echo "  DED_LIBS:"
	-@for i in $(DED_LIBS); \
	do \
		echo "    $$i"; \
	done
endif
	@echo ""
	@echo "  LCCFLAGS:"
	-@for i in $(LCCFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo "  SHLIBLDFLAGS:"
	-@for i in $(SHLIBLDFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo " SHLIBCCFLAGS:"
	-@for i in $(SHLIBCCFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo " SHLIBLIBS:"
	-@for i in $(SHLIBLIBS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo " STATICLIBLDFLAGS:"
	-@for i in $(STATICLIBLDFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo " STATICLIBCFLAGS:"
	-@for i in $(STATICLIBCFLAGS); \
	do \
		echo "    $$i"; \
	done
ifeq ($(BUILD_KMAP2),1)
	@echo ""
	@echo " KMAP2_CFLAGS:"
	-@for i in $(KMAP2_CFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo " KMAP2_LIBS:"
	-@for i in $(KMAP2_LIBS); \
	do \
		echo "    $$i"; \
	done
endif
ifeq ($(BUILD_KAAS),1)
	@echo ""
	@echo " KAAS_CFLAGS:"
	-@for i in $(KAAS_CFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo " KAAS_LIBS:"
	-@for i in $(KAAS_LIBS); \
	do \
		echo "    $$i"; \
	done
endif
	@echo ""
	@echo "  Output:"
	-@for i in $(TARGETS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
ifneq ($(TARGETS),)
	@$(MAKE) $(TARGETS) V=$(V)
endif

makedirs:
	@if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi
	@if [ ! -d $(B) ];then $(MKDIR) $(B);fi
	@if [ ! -d $(B)/client ];then $(MKDIR) $(B)/client;fi
	@if [ ! -d $(B)/client/speex ];then $(MKDIR) $(B)/client/speex;fi
	@if [ ! -d $(B)/client/theora ];then $(MKDIR) $(B)/client/theora;fi
	@if [ ! -d $(B)/clientsmp ];then $(MKDIR) $(B)/clientsmp;fi
	@if [ ! -d $(B)/clientsmp/speex ];then $(MKDIR) $(B)/clientsmp/speex;fi
	@if [ ! -d $(B)/ded ];then $(MKDIR) $(B)/ded;fi
	@if [ ! -d $(B)/basekpq3 ];then $(MKDIR) $(B)/basekpq3;fi
	@if [ ! -d $(B)/basekpq3/cgame ];then $(MKDIR) $(B)/basekpq3/cgame;fi
	@if [ ! -d $(B)/basekpq3/game ];then $(MKDIR) $(B)/basekpq3/game;fi
	@if [ ! -d $(B)/basekpq3/ui ];then $(MKDIR) $(B)/basekpq3/ui;fi
	@if [ ! -d $(B)/basekpq3/qcommon ];then $(MKDIR) $(B)/basekpq3/qcommon;fi
	@if [ ! -d $(B)/basekpq3/vm ];then $(MKDIR) $(B)/basekpq3/vm;fi
	@if [ ! -d $(B)/basekpq3/ui ];then $(MKDIR) $(B)/basekpq3/ui;fi
	@if [ ! -d $(B)/tools ];then $(MKDIR) $(B)/tools;fi
	@if [ ! -d $(B)/tools/asm ];then $(MKDIR) $(B)/tools/asm;fi
	@if [ ! -d $(B)/tools/etc ];then $(MKDIR) $(B)/tools/etc;fi
	@if [ ! -d $(B)/tools/rcc ];then $(MKDIR) $(B)/tools/rcc;fi
	@if [ ! -d $(B)/tools/cpp ];then $(MKDIR) $(B)/tools/cpp;fi
	@if [ ! -d $(B)/tools/lburg ];then $(MKDIR) $(B)/tools/lburg;fi
	@if [ ! -d $(B)/kmap2 ];then $(MKDIR) $(B)/kmap2;fi
	@if [ ! -d $(B)/kmap2/lwo ];then $(MKDIR) $(B)/kmap2/lwo;fi
	@if [ ! -d $(B)/kaas ];then $(MKDIR) $(B)/kaas;fi

#############################################################################
# QVM BUILD TOOLS
#############################################################################

TOOLS_OPTIMIZE = -g -Wall -fno-strict-aliasing
TOOLS_CFLAGS += $(TOOLS_OPTIMIZE) \
               -DTEMPDIR=\"$(TEMPDIR)\" -DSYSTEM=\"\" \
               -I$(Q3LCCSRCDIR) \
               -I$(LBURGDIR)
TOOLS_LIBS =
TOOLS_LDFLAGS =

ifeq ($(GENERATE_DEPENDENCIES),1)
	TOOLS_CFLAGS += -MMD
endif

define DO_TOOLS_CC
$(echo_cmd) "TOOLS_CC $<"
$(Q)$(CC) $(TOOLS_CFLAGS) -o $@ -c $<
endef

define DO_TOOLS_CC_DAGCHECK
$(echo_cmd) "TOOLS_CC_DAGCHECK $<"
$(Q)$(CC) $(TOOLS_CFLAGS) -Wno-unused -o $@ -c $<
endef

LBURG       = $(B)/tools/lburg/lburg$(BINEXT)
DAGCHECK_C  = $(B)/tools/rcc/dagcheck.c
Q3RCC       = $(B)/tools/q3rcc$(BINEXT)
Q3CPP       = $(B)/tools/q3cpp$(BINEXT)
Q3LCC       = $(B)/tools/q3lcc$(BINEXT)
Q3ASM       = $(B)/tools/q3asm$(BINEXT)

LBURGOBJ= \
	$(B)/tools/lburg/lburg.o \
	$(B)/tools/lburg/gram.o

$(B)/tools/lburg/%.o: $(LBURGDIR)/%.c
	$(DO_TOOLS_CC)

$(LBURG): $(LBURGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

Q3RCCOBJ = \
  $(B)/tools/rcc/alloc.o \
  $(B)/tools/rcc/bind.o \
  $(B)/tools/rcc/bytecode.o \
  $(B)/tools/rcc/dag.o \
  $(B)/tools/rcc/dagcheck.o \
  $(B)/tools/rcc/decl.o \
  $(B)/tools/rcc/enode.o \
  $(B)/tools/rcc/error.o \
  $(B)/tools/rcc/event.o \
  $(B)/tools/rcc/expr.o \
  $(B)/tools/rcc/gen.o \
  $(B)/tools/rcc/init.o \
  $(B)/tools/rcc/inits.o \
  $(B)/tools/rcc/input.o \
  $(B)/tools/rcc/lex.o \
  $(B)/tools/rcc/list.o \
  $(B)/tools/rcc/main.o \
  $(B)/tools/rcc/null.o \
  $(B)/tools/rcc/output.o \
  $(B)/tools/rcc/prof.o \
  $(B)/tools/rcc/profio.o \
  $(B)/tools/rcc/simp.o \
  $(B)/tools/rcc/stmt.o \
  $(B)/tools/rcc/string.o \
  $(B)/tools/rcc/sym.o \
  $(B)/tools/rcc/symbolic.o \
  $(B)/tools/rcc/trace.o \
  $(B)/tools/rcc/tree.o \
  $(B)/tools/rcc/types.o

$(DAGCHECK_C): $(LBURG) $(Q3LCCSRCDIR)/dagcheck.md
	$(echo_cmd) "LBURG $(Q3LCCSRCDIR)/dagcheck.md"
	$(Q)$(LBURG) $(Q3LCCSRCDIR)/dagcheck.md $@

$(B)/tools/rcc/dagcheck.o: $(DAGCHECK_C)
	$(DO_TOOLS_CC_DAGCHECK)

$(B)/tools/rcc/%.o: $(Q3LCCSRCDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3RCC): $(Q3RCCOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

Q3CPPOBJ = \
	$(B)/tools/cpp/cpp.o \
	$(B)/tools/cpp/lex.o \
	$(B)/tools/cpp/nlist.o \
	$(B)/tools/cpp/tokens.o \
	$(B)/tools/cpp/macro.o \
	$(B)/tools/cpp/eval.o \
	$(B)/tools/cpp/include.o \
	$(B)/tools/cpp/hideset.o \
	$(B)/tools/cpp/getopt.o \
	$(B)/tools/cpp/unix.o

$(B)/tools/cpp/%.o: $(Q3CPPDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3CPP): $(Q3CPPOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

Q3LCCOBJ = \
	$(B)/tools/etc/lcc.o \
	$(B)/tools/etc/bytecode.o

$(B)/tools/etc/%.o: $(Q3LCCETCDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3LCC): $(Q3LCCOBJ) $(Q3RCC) $(Q3CPP)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(Q3LCCOBJ) $(TOOLS_LIBS)

define DO_Q3LCC
$(echo_cmd) "Q3LCC $<"
$(Q)$(Q3LCC) $(LCCFLAGS) -o $@ $<
endef

define DO_CGAME_Q3LCC
$(echo_cmd) "CGAME_Q3LCC $<"
$(Q)$(Q3LCC) $(LCCFLAGS) -DCGAME -o $@ $<
endef

define DO_GAME_Q3LCC
$(echo_cmd) "GAME_Q3LCC $<"
$(Q)$(Q3LCC) $(LCCFLAGS) -DQAGAME -o $@ $<
endef

define DO_UI_Q3LCC
$(echo_cmd) "UI_Q3LCC $<"
$(Q)$(Q3LCC) $(LCCFLAGS) -DUI -o $@ $<
endef

Q3ASMOBJ = \
  $(B)/tools/asm/q3asm.o \
  $(B)/tools/asm/cmdlib.o

$(B)/tools/asm/%.o: $(Q3ASMDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3ASM): $(Q3ASMOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

#############################################################################
# static libs
#############################################################################

define DO_JPEG_CC
$(echo_cmd) "JPEG_CC $<"
$(Q)$(CC) $(CLIENT_CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

JPEGSLIB = $(B)/kpq3jpeg8c.a
JPEGOBJ = \
  $(B)/client/jaricom.o \
  $(B)/client/jcapimin.o \
  $(B)/client/jcapistd.o \
  $(B)/client/jcarith.o \
  $(B)/client/jccoefct.o \
  $(B)/client/jccolor.o \
  $(B)/client/jcdctmgr.o \
  $(B)/client/jchuff.o \
  $(B)/client/jcinit.o \
  $(B)/client/jcmainct.o \
  $(B)/client/jcmarker.o \
  $(B)/client/jcmaster.o \
  $(B)/client/jcomapi.o \
  $(B)/client/jcparam.o \
  $(B)/client/jcprepct.o \
  $(B)/client/jcsample.o \
  $(B)/client/jctrans.o \
  $(B)/client/jdapimin.o \
  $(B)/client/jdapistd.o \
  $(B)/client/jdarith.o \
  $(B)/client/jdatadst.o \
  $(B)/client/jdatasrc.o \
  $(B)/client/jdcoefct.o \
  $(B)/client/jdcolor.o \
  $(B)/client/jddctmgr.o \
  $(B)/client/jdhuff.o \
  $(B)/client/jdinput.o \
  $(B)/client/jdmainct.o \
  $(B)/client/jdmarker.o \
  $(B)/client/jdmaster.o \
  $(B)/client/jdmerge.o \
  $(B)/client/jdpostct.o \
  $(B)/client/jdsample.o \
  $(B)/client/jdtrans.o \
  $(B)/client/jerror.o \
  $(B)/client/jfdctflt.o \
  $(B)/client/jfdctfst.o \
  $(B)/client/jfdctint.o \
  $(B)/client/jidctflt.o \
  $(B)/client/jidctfst.o \
  $(B)/client/jidctint.o \
  $(B)/client/jmemmgr.o \
  $(B)/client/jmemnobs.o \
  $(B)/client/jquant1.o \
  $(B)/client/jquant2.o \
  $(B)/client/jutils.o 

$(B)/client/%.o: $(JPDIR)/%.c
	$(DO_JPEG_CC)

$(JPEGSLIB): $(JPEGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(SLD) "rcs" $(JPEGSLIB) $(JPEGOBJ)

#############################################################################
# CLIENT/SERVER
#############################################################################

Q3OBJ = \
  $(B)/client/cl_cgame.o \
  $(B)/client/cl_cin.o \
  $(B)/client/cl_console.o \
  $(B)/client/cl_input.o \
  $(B)/client/cl_keys.o \
  $(B)/client/cl_main.o \
  $(B)/client/cl_net_chan.o \
  $(B)/client/cl_parse.o \
  $(B)/client/cl_scrn.o \
  $(B)/client/cl_ui.o \
  $(B)/client/cl_avi.o \
  $(B)/client/cl_ogm.o \
  \
  $(B)/client/cm_load.o \
  $(B)/client/cm_patch.o \
  $(B)/client/cm_polylib.o \
  $(B)/client/cm_test.o \
  $(B)/client/cm_trace.o \
  $(B)/client/cm_trisoup.o \
  \
  $(B)/client/cmd.o \
  $(B)/client/common.o \
  $(B)/client/cvar.o \
  $(B)/client/files.o \
  $(B)/client/md4.o \
  $(B)/client/md5.o \
  $(B)/client/msg.o \
  $(B)/client/net_chan.o \
  $(B)/client/net_ip.o \
  $(B)/client/huffman.o \
  \
  $(B)/client/snd_adpcm.o \
  $(B)/client/snd_dma.o \
  $(B)/client/snd_mem.o \
  $(B)/client/snd_mix.o \
  $(B)/client/snd_wavelet.o \
  \
  $(B)/client/snd_main.o \
  $(B)/client/snd_codec.o \
  $(B)/client/snd_codec_wav.o \
  $(B)/client/snd_codec_ogg.o \
  \
  $(B)/client/qal.o \
  $(B)/client/snd_openal.o \
  \
  $(B)/client/cl_curl.o \
  \
  $(B)/client/sv_bot.o \
  $(B)/client/sv_ccmds.o \
  $(B)/client/sv_client.o \
  $(B)/client/sv_game.o \
  $(B)/client/sv_init.o \
  $(B)/client/sv_main.o \
  $(B)/client/sv_net_chan.o \
  $(B)/client/sv_snapshot.o \
  $(B)/client/sv_world.o \
  \
  $(B)/client/q_math.o \
  $(B)/client/q_mathsse.o \
  $(B)/client/q_shared.o \
  $(B)/client/q_unicode.o \
  \
  $(B)/client/unzip.o \
  $(B)/client/ioapi.o \
  $(B)/client/puff.o \
  $(B)/client/vm.o \
  $(B)/client/vm_interpreted.o \
  \
  $(B)/client/be_aas_bspq3.o \
  $(B)/client/be_aas_cluster.o \
  $(B)/client/be_aas_debug.o \
  $(B)/client/be_aas_entity.o \
  $(B)/client/be_aas_file.o \
  $(B)/client/be_aas_main.o \
  $(B)/client/be_aas_move.o \
  $(B)/client/be_aas_optimize.o \
  $(B)/client/be_aas_reach.o \
  $(B)/client/be_aas_route.o \
  $(B)/client/be_aas_routealt.o \
  $(B)/client/be_aas_sample.o \
  $(B)/client/be_ai_char.o \
  $(B)/client/be_ai_chat.o \
  $(B)/client/be_ai_gen.o \
  $(B)/client/be_ai_goal.o \
  $(B)/client/be_ai_move.o \
  $(B)/client/be_ai_weap.o \
  $(B)/client/be_ai_weight.o \
  $(B)/client/be_ea.o \
  $(B)/client/be_interface.o \
  $(B)/client/l_crc.o \
  $(B)/client/l_libvar.o \
  $(B)/client/l_log.o \
  $(B)/client/l_memory.o \
  $(B)/client/l_precomp.o \
  $(B)/client/l_script.o \
  $(B)/client/l_struct.o 

ifeq ($(USE_INTERNAL_PNG),1)
Q3OBJ += \
  $(B)/client/png.o \
  $(B)/client/pngerror.o \
  $(B)/client/pngget.o \
  $(B)/client/pngmem.o \
  $(B)/client/pngpread.o \
  $(B)/client/pngread.o \
  $(B)/client/pngrio.o \
  $(B)/client/pngrtran.o \
  $(B)/client/pngrutil.o\
  $(B)/client/pngset.o \
  $(B)/client/pngtrans.o \
  $(B)/client/pngwio.o \
  $(B)/client/pngwrite.o \
  $(B)/client/pngwtran.o \
  $(B)/client/pngwutil.o
endif

Q3OBJ += \
  $(B)/client/tr_glshader.o \
  $(B)/client/tr_animation.o \
  $(B)/client/tr_backend.o \
  $(B)/client/tr_bsp.o \
  $(B)/client/tr_cmds.o \
  $(B)/client/tr_curve.o \
  $(B)/client/tr_decals.o \
  $(B)/client/tr_flares.o \
  $(B)/client/tr_font.o \
  $(B)/client/tr_fog.o \
  $(B)/client/tr_image.o \
  $(B)/client/tr_image_jpg.o \
  $(B)/client/tr_image_tga.o \
  $(B)/client/tr_image_pcx.o \
  $(B)/client/tr_image_dds.o \
  $(B)/client/tr_image_png.o \
  $(B)/client/tr_image_bmp.o \
  $(B)/client/tr_init.o \
  $(B)/client/tr_light.o \
  $(B)/client/tr_main.o \
  $(B)/client/tr_marks.o \
  $(B)/client/tr_mesh.o \
  $(B)/client/tr_model.o \
  $(B)/client/tr_model_mdc.o \
  $(B)/client/tr_model_mdm.o \
  $(B)/client/tr_model_md3.o \
  $(B)/client/tr_model_md5.o \
  $(B)/client/tr_model_psk.o \
  $(B)/client/tr_model_skel.o \
  $(B)/client/tr_model_iqm.o \
  $(B)/client/tr_noise.o \
  $(B)/client/tr_scene.o \
  $(B)/client/tr_shade.o \
  $(B)/client/tr_shade_calc.o \
  $(B)/client/tr_shader.o \
  $(B)/client/tr_shadows.o \
  $(B)/client/tr_skin.o \
  $(B)/client/tr_sky.o \
  $(B)/client/tr_surface.o \
  $(B)/client/tr_world.o \
  $(B)/client/tr_fbo.o \
  $(B)/client/tr_vbo.o \
  \
  $(B)/client/sdl_gamma.o \
  $(B)/client/sdl_input.o \
  $(B)/client/sdl_snd.o \
  \
  $(B)/client/con_log.o \
  $(B)/client/sys_main.o \
  $(B)/client/tr_half.o 
  
ifeq ($(PLATFORM),mingw32)
Q3OBJ += $(B)/client/con_passive.o
else
Q3OBJ += $(B)/client/con_tty.o
endif

ifeq ($(USE_INTERNAL_GLEW),1)
Q3OBJ += $(B)/client/glew.o
endif

ifeq ($(USE_FREETYPE),1)
ifeq ($(USE_FREETYPE_BUILDIN),1)
Q3OBJ += \
 $(B)/client/autofit.o \
 $(B)/client/bdf.o \
 $(B)/client/cff.o \
 $(B)/client/ftbase.o \
 $(B)/client/ftbitmap.o \
 $(B)/client/ftcache.o \
 $(B)/client/ftdebug.o \
 $(B)/client/ftgasp.o \
 $(B)/client/ftglyph.o \
 $(B)/client/ftgzip.o \
 $(B)/client/ftinit.o \
 $(B)/client/ftlzw.o \
 $(B)/client/ftstroke.o \
 $(B)/client/ftsystem.o \
 $(B)/client/smooth.o \
 $(B)/client/ftbbox.o \
 $(B)/client/ftmm.o \
 $(B)/client/ftpfr.o \
 $(B)/client/ftsynth.o \
 $(B)/client/fttype1.o \
 $(B)/client/ftwinfnt.o \
 $(B)/client/pcf.o \
 $(B)/client/pfr.o \
 $(B)/client/psaux.o \
 $(B)/client/pshinter.o \
 $(B)/client/psmodule.o \
 $(B)/client/raster.o \
 $(B)/client/sfnt.o \
 $(B)/client/truetype.o \
 $(B)/client/type1.o \
 $(B)/client/type1cid.o \
 $(B)/client/type42.o \
 $(B)/client/winfnt.o
endif
endif
ifeq ($(USE_CIN_THEORA),1)
ifeq ($(USE_CIN_THEORA_BUILDIN),1)
Q3OBJ += \
  $(B)/client/theora/analyze.o \
  $(B)/client/theora/apiwrapper.o \
  $(B)/client/theora/bitpack.o \
  $(B)/client/theora/cpu.o \
  $(B)/client/theora/decapiwrapper.o \
  $(B)/client/theora/decinfo.o \
  $(B)/client/theora/decode.o \
  $(B)/client/theora/dequant.o \
  $(B)/client/theora/encapiwrapper.o \
  $(B)/client/theora/encfrag.o \
  $(B)/client/theora/encinfo.o \
  $(B)/client/theora/encode.o \
  $(B)/client/theora/enquant.o \
  $(B)/client/theora/fdct.o \
  $(B)/client/theora/fragment.o \
  $(B)/client/theora/huffdec.o \
  $(B)/client/theora/huffenc.o \
  $(B)/client/theora/idct.o \
  $(B)/client/theora/info.o \
  $(B)/client/theora/internal.o \
  $(B)/client/theora/mathops.o \
  $(B)/client/theora/mcenc.o \
  $(B)/client/theora/quant.o \
  $(B)/client/theora/rate.o \
  $(B)/client/theora/state.o \
  $(B)/client/theora/tokenize.o
endif
endif
ifeq ($(ARCH),i386)
  Q3OBJ += \
    $(B)/client/snd_mixa.o \
    $(B)/client/matha.o \
    $(B)/client/snapvector.o \
    $(B)/client/ftola.o
endif
ifeq ($(ARCH),x86)
  Q3OBJ += \
    $(B)/client/snd_mixa.o \
    $(B)/client/matha.o \
    $(B)/client/snapvector.o \
    $(B)/client/ftola.o
endif
ifeq ($(ARCH),x86_64)
  Q3OBJ += \
    $(B)/client/snapvector.o \
    $(B)/client/ftola.o
endif
ifeq ($(ARCH),amd64)
  Q3OBJ += \
    $(B)/client/snapvector.o \
    $(B)/client/ftola.o
endif
ifeq ($(ARCH),x64)
  Q3OBJ += \
    $(B)/client/snapvector.o \
    $(B)/client/ftola.o
endif

ifeq ($(USE_VOIP),1)
ifeq ($(USE_INTERNAL_SPEEX),1)
Q3OBJ += \
  $(B)/client/speex/bits.o \
  $(B)/client/speex/buffer.o \
  $(B)/client/speex/cb_search.o \
  $(B)/client/speex/exc_10_16_table.o \
  $(B)/client/speex/exc_10_32_table.o \
  $(B)/client/speex/exc_20_32_table.o \
  $(B)/client/speex/exc_5_256_table.o \
  $(B)/client/speex/exc_5_64_table.o \
  $(B)/client/speex/exc_8_128_table.o \
  $(B)/client/speex/fftwrap.o \
  $(B)/client/speex/filterbank.o \
  $(B)/client/speex/filters.o \
  $(B)/client/speex/gain_table.o \
  $(B)/client/speex/gain_table_lbr.o \
  $(B)/client/speex/hexc_10_32_table.o \
  $(B)/client/speex/hexc_table.o \
  $(B)/client/speex/high_lsp_tables.o \
  $(B)/client/speex/jitter.o \
  $(B)/client/speex/kiss_fft.o \
  $(B)/client/speex/kiss_fftr.o \
  $(B)/client/speex/lsp_tables_nb.o \
  $(B)/client/speex/ltp.o \
  $(B)/client/speex/mdf.o \
  $(B)/client/speex/modes.o \
  $(B)/client/speex/modes_wb.o \
  $(B)/client/speex/nb_celp.o \
  $(B)/client/speex/preprocess.o \
  $(B)/client/speex/quant_lsp.o \
  $(B)/client/speex/resample.o \
  $(B)/client/speex/sb_celp.o \
  $(B)/client/speex/smallft.o \
  $(B)/client/speex/speex.o \
  $(B)/client/speex/speex_callbacks.o \
  $(B)/client/speex/speex_header.o \
  $(B)/client/speex/stereo.o \
  $(B)/client/speex/vbr.o \
  $(B)/client/speex/vq.o \
  $(B)/client/speex/window.o \
  $(B)/client/speex/lpc.o \
  $(B)/client/speex/lsp.o 
endif
endif

ifeq ($(USE_INTERNAL_VORBIS),1)
Q3OBJ += \
 $(B)/client/analysis.o \
 $(B)/client/bitrate.o \
 $(B)/client/block.o \
 $(B)/client/codebook.o \
 $(B)/client/envelope.o \
 $(B)/client/floor0.o \
 $(B)/client/floor1.o \
 $(B)/client/info.o \
 $(B)/client/lookup.o \
 $(B)/client/lpc.o \
 $(B)/client/lsp.o \
 $(B)/client/mapping0.o \
 $(B)/client/mdct.o \
 $(B)/client/psy.o \
 $(B)/client/registry.o \
 $(B)/client/res0.o \
 $(B)/client/sharedbook.o \
 $(B)/client/smallft.o \
 $(B)/client/synthesis.o \
 $(B)/client/vorbisenc.o \
 $(B)/client/vorbisfile.o \
 $(B)/client/window.o \
 \
 $(B)/client/bitwise.o \
 $(B)/client/framing.o 
endif


ifeq ($(USE_INTERNAL_ZLIB),1)
Q3OBJ += \
  $(B)/client/adler32.o \
  $(B)/client/crc32.o \
  $(B)/client/inffast.o \
  $(B)/client/inflate.o \
  $(B)/client/inftrees.o \
  $(B)/client/zutil.o
endif

ifeq ($(HAVE_VM_COMPILED),true)
  ifeq ($(ARCH),i386)
    Q3OBJ += \
      $(B)/client/vm_x86.o
  endif
  ifeq ($(ARCH),x86)
    Q3OBJ += \
      $(B)/client/vm_x86.o
  endif
  ifeq ($(ARCH),x86_64)
    ifeq ($(USE_OLD_VM64),1)
      Q3OBJ += \
        $(B)/client/vm_x86_64.o \
        $(B)/client/vm_x86_64_assembler.o
    else
      Q3OBJ += \
        $(B)/client/vm_x86.o
    endif
  endif
  ifeq ($(ARCH),amd64)
    ifeq ($(USE_OLD_VM64),1)
      Q3OBJ += \
        $(B)/client/vm_x86_64.o \
        $(B)/client/vm_x86_64_assembler.o
    else
      Q3OBJ += \
        $(B)/client/vm_x86.o
    endif
  endif
  ifeq ($(ARCH),x64)
    ifeq ($(USE_OLD_VM64),1)
      Q3OBJ += \
        $(B)/client/vm_x86_64.o \
        $(B)/client/vm_x86_64_assembler.o
    else
      Q3OBJ += \
        $(B)/client/vm_x86.o
    endif
  endif
  ifeq ($(ARCH),ppc)
    Q3OBJ += $(B)/client/vm_powerpc.o $(B)/client/vm_powerpc_asm.o
  endif
  ifeq ($(ARCH),ppc64)
    Q3OBJ += $(B)/client/vm_powerpc.o $(B)/client/vm_powerpc_asm.o
  endif
  ifeq ($(ARCH),sparc)
    Q3OBJ += $(B)/client/vm_sparc.o
  endif
endif

ifeq ($(PLATFORM),mingw32)
  Q3OBJ += \
    $(B)/client/win_resource.o \
    $(B)/client/sys_win32.o
else
  Q3OBJ += \
    $(B)/client/sys_unix.o
endif

ifeq ($(PLATFORM),darwin)
  Q3OBJ += \
    $(B)/client/sys_osx.o
endif
ifeq ($(USE_MUMBLE),1)
  Q3OBJ += \
    $(B)/client/libmumblelink.o
endif

Q3POBJ += \
  $(B)/client/sdl_glimp.o

Q3POBJ_SMP += \
  $(B)/clientsmp/sdl_glimp.o

$(B)/kingpinq3$(FULLBINEXT): $(Q3OBJ) $(Q3POBJ) $(LIBSDLMAIN)
	$(echo_cmd) "LD $@"
	$(Q)$(CCC) $(CLIENT_CFLAGS) $(CFLAGS) $(CLIENT_LDFLAGS) $(LDFLAGS) \
	-o $@ $(Q3OBJ) $(Q3POBJ) \
	$(LIBSDLMAIN) $(CLIENT_LIBS) $(LIBS)

$(B)/kingpinq3-smp$(FULLBINEXT): $(Q3OBJ) $(Q3POBJ_SMP) $(LIBSDLMAIN)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CLIENT_CFLAGS) $(CFLAGS) $(CLIENT_LDFLAGS) $(LDFLAGS) $(THREAD_LDFLAGS) \
		-o $@ $(Q3OBJ) $(Q3POBJ_SMP) \
		$(THREAD_LIBS) $(LIBSDLMAIN) $(CLIENT_LIBS) $(LIBS)

ifneq ($(strip $(LIBSDLMAIN)),)
ifneq ($(strip $(LIBSDLMAINSRC)),)
$(LIBSDLMAIN) : $(LIBSDLMAINSRC)
	cp $< $@
	ranlib $@
endif
endif



#############################################################################
# DEDICATED SERVER
#############################################################################

Q3DOBJ = \
  $(B)/ded/sv_bot.o \
  $(B)/ded/sv_client.o \
  $(B)/ded/sv_ccmds.o \
  $(B)/ded/sv_game.o \
  $(B)/ded/sv_init.o \
  $(B)/ded/sv_main.o \
  $(B)/ded/sv_net_chan.o \
  $(B)/ded/sv_snapshot.o \
  $(B)/ded/sv_world.o \
  \
  $(B)/ded/cm_load.o \
  $(B)/ded/cm_patch.o \
  $(B)/ded/cm_polylib.o \
  $(B)/ded/cm_test.o \
  $(B)/ded/cm_trace.o \
  $(B)/ded/cm_trisoup.o \
  $(B)/ded/cmd.o \
  $(B)/ded/common.o \
  $(B)/ded/cvar.o \
  $(B)/ded/files.o \
  $(B)/ded/md4.o \
  $(B)/ded/msg.o \
  $(B)/ded/net_chan.o \
  $(B)/ded/net_ip.o \
  $(B)/ded/huffman.o \
  \
  $(B)/ded/q_math.o \
  $(B)/ded/q_mathsse.o \
  $(B)/ded/q_shared.o \
  \
  $(B)/ded/unzip.o \
  $(B)/ded/ioapi.o \
  $(B)/ded/vm.o \
  $(B)/ded/vm_interpreted.o \
  \
  $(B)/ded/be_aas_bspq3.o \
  $(B)/ded/be_aas_cluster.o \
  $(B)/ded/be_aas_debug.o \
  $(B)/ded/be_aas_entity.o \
  $(B)/ded/be_aas_file.o \
  $(B)/ded/be_aas_main.o \
  $(B)/ded/be_aas_move.o \
  $(B)/ded/be_aas_optimize.o \
  $(B)/ded/be_aas_reach.o \
  $(B)/ded/be_aas_route.o \
  $(B)/ded/be_aas_routealt.o \
  $(B)/ded/be_aas_sample.o \
  $(B)/ded/be_ai_char.o \
  $(B)/ded/be_ai_chat.o \
  $(B)/ded/be_ai_gen.o \
  $(B)/ded/be_ai_goal.o \
  $(B)/ded/be_ai_move.o \
  $(B)/ded/be_ai_weap.o \
  $(B)/ded/be_ai_weight.o \
  $(B)/ded/be_ea.o \
  $(B)/ded/be_interface.o \
  $(B)/ded/l_crc.o \
  $(B)/ded/l_libvar.o \
  $(B)/ded/l_log.o \
  $(B)/ded/l_memory.o \
  $(B)/ded/l_precomp.o \
  $(B)/ded/l_script.o \
  $(B)/ded/l_struct.o \
  \
  $(B)/ded/null_client.o \
  $(B)/ded/null_input.o \
  $(B)/ded/null_snddma.o \
  \
  $(B)/ded/con_log.o \
  $(B)/ded/sys_main.o

ifeq ($(ARCH),i386)
  Q3DOBJ += \
      $(B)/ded/matha.o \
      $(B)/ded/snapvector.o \
      $(B)/ded/ftola.o
endif
ifeq ($(ARCH),x86)
  Q3DOBJ += \
      $(B)/ded/matha.o \
      $(B)/ded/snapvector.o \
      $(B)/ded/ftola.o 
endif
ifeq ($(ARCH),x86_64)
  Q3DOBJ += \
      $(B)/ded/snapvector.o \
      $(B)/ded/ftola.o 
endif
ifeq ($(ARCH),amd64)
  Q3DOBJ += \
      $(B)/ded/snapvector.o \
      $(B)/ded/ftola.o 
endif
ifeq ($(ARCH),x64)
  Q3DOBJ += \
      $(B)/ded/snapvector.o \
      $(B)/ded/ftola.o 
endif

ifeq ($(USE_INTERNAL_ZLIB),1)
Q3DOBJ += \
  $(B)/ded/adler32.o \
  $(B)/ded/crc32.o \
  $(B)/ded/inffast.o \
  $(B)/ded/inflate.o \
  $(B)/ded/inftrees.o \
  $(B)/ded/zutil.o
endif
ifeq ($(HAVE_VM_COMPILED),true)
  ifeq ($(ARCH),i386)
    Q3DOBJ += \
      $(B)/ded/vm_x86.o
  endif
  ifeq ($(ARCH),x86)
    Q3DOBJ += \
      $(B)/ded/vm_x86.o
  endif
  ifeq ($(ARCH),x86_64)
    ifeq ($(USE_OLD_VM64),1)
      Q3DOBJ += \
        $(B)/ded/vm_x86_64.o \
        $(B)/ded/vm_x86_64_assembler.o
    else
      Q3DOBJ += \
        $(B)/ded/vm_x86.o
    endif
  endif
  ifeq ($(ARCH),amd64)
    ifeq ($(USE_OLD_VM64),1)
      Q3DOBJ += \
        $(B)/ded/vm_x86_64.o \
        $(B)/ded/vm_x86_64_assembler.o
    else
      Q3DOBJ += \
        $(B)/ded/vm_x86.o
    endif
  endif
  ifeq ($(ARCH),x64)
    ifeq ($(USE_OLD_VM64),1)
      Q3DOBJ += \
        $(B)/ded/vm_x86_64.o \
        $(B)/ded/vm_x86_64_assembler.o
    else
      Q3DOBJ += \
        $(B)/ded/vm_x86.o
    endif
  endif
  ifeq ($(ARCH),ppc)
    Q3DOBJ += $(B)/ded/vm_powerpc.o $(B)/ded/vm_powerpc_asm.o
  endif
  ifeq ($(ARCH),ppc64)
    Q3DOBJ += $(B)/ded/vm_powerpc.o $(B)/ded/vm_powerpc_asm.o
  endif
  ifeq ($(ARCH),sparc)
    Q3DOBJ += $(B)/ded/vm_sparc.o
  endif
endif

ifeq ($(PLATFORM),mingw32)
  Q3DOBJ += \
    $(B)/ded/win_resource.o \
    $(B)/ded/sys_win32.o \
    $(B)/ded/con_win32.o
else
  Q3DOBJ += \
    $(B)/ded/sys_unix.o \
    $(B)/ded/con_tty.o
endif

ifeq ($(PLATFORM),darwin)
  Q3DOBJ += \
    $(B)/ded/sys_osx.o
endif

$(B)/kpq3ded$(FULLBINEXT): $(Q3DOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CCC) $(CFLAGS) $(LDFLAGS) -o $@ $(Q3DOBJ) $(LIBS) $(DED_LIBS)



#############################################################################
## basekpq3 CGAME/client/speex/
#############################################################################

Q3CGOBJ_ = \
  $(B)/basekpq3/cgame/cg_main.o \
  $(B)/basekpq3/cgame/bg_misc.o \
  $(B)/basekpq3/cgame/bg_pmove.o \
  $(B)/basekpq3/cgame/bg_slidemove.o \
  $(B)/basekpq3/cgame/bg_lib.o \
  $(B)/basekpq3/cgame/cg_animation.o \
  $(B)/basekpq3/cgame/cg_consolecmds.o \
  $(B)/basekpq3/cgame/cg_newdraw.o \
  $(B)/basekpq3/cgame/cg_draw.o \
  $(B)/basekpq3/cgame/cg_drawtools.o \
  $(B)/basekpq3/cgame/cg_effects.o \
  $(B)/basekpq3/cgame/cg_ents.o \
  $(B)/basekpq3/cgame/cg_event.o \
  $(B)/basekpq3/cgame/cg_info.o \
  $(B)/basekpq3/cgame/cg_localents.o \
  $(B)/basekpq3/cgame/cg_particles.o \
  $(B)/basekpq3/cgame/cg_marks.o \
  $(B)/basekpq3/cgame/cg_players.o \
  $(B)/basekpq3/cgame/cg_playerstate.o \
  $(B)/basekpq3/cgame/cg_predict.o \
  $(B)/basekpq3/cgame/cg_scoreboard.o \
  $(B)/basekpq3/cgame/cg_servercmds.o \
  $(B)/basekpq3/cgame/cg_snapshot.o \
  $(B)/basekpq3/cgame/cg_view.o \
  $(B)/basekpq3/cgame/cg_weapons.o \
  $(B)/basekpq3/cgame/cg_flamethrower.o \
  $(B)/basekpq3/cgame/cg_polybus.o \
  $(B)/basekpq3/cgame/cg_trails.o \
  $(B)/basekpq3/cgame/cg_xppm.o \
  $(B)/basekpq3/ui/ui_shared.o \
  \
  $(B)/basekpq3/qcommon/q_math.o \
  $(B)/basekpq3/qcommon/q_shared.o

Q3CGOBJ = $(Q3CGOBJ_) $(B)/basekpq3/cgame/cg_syscalls.o
Q3CGVMOBJ = $(Q3CGOBJ_:%.o=%.asm)

$(B)/basekpq3/cgame$(SHLIBNAME) : $(Q3CGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(LD) $(CFLAGS) $(SHLIBLDFLAGS) $(SOTHREAD_LIBS) -o $@ $(Q3CGOBJ) $(SHLIBLIBS)

$(B)/basekpq3/vm/cgame.qvm: $(Q3CGVMOBJ) $(CGDIR)/cg_syscalls.asm $(Q3ASM)
	$(echo_cmd) "Q3ASM $@"
	$(Q)$(Q3ASM) -o $@ $(Q3CGVMOBJ) $(CGDIR)/cg_syscalls.asm


#############################################################################
## basekpq3 GAME
#############################################################################

Q3GOBJ_ = \
  $(B)/basekpq3/game/g_main.o \
  $(B)/basekpq3/game/ai_chat.o \
  $(B)/basekpq3/game/ai_cmd.o \
  $(B)/basekpq3/game/ai_dmnet.o \
  $(B)/basekpq3/game/ai_dmq3.o \
  $(B)/basekpq3/game/ai_main.o \
  $(B)/basekpq3/game/ai_team.o \
  $(B)/basekpq3/game/ai_vcmd.o \
  $(B)/basekpq3/game/bg_misc.o \
  $(B)/basekpq3/game/bg_pmove.o \
  $(B)/basekpq3/game/bg_slidemove.o \
  $(B)/basekpq3/game/bg_lib.o \
  $(B)/basekpq3/game/g_active.o \
  $(B)/basekpq3/game/g_arenas.o \
  $(B)/basekpq3/game/g_bot.o \
  $(B)/basekpq3/game/g_client.o \
  $(B)/basekpq3/game/g_cmds.o \
  $(B)/basekpq3/game/g_combat.o \
  $(B)/basekpq3/game/g_items.o \
  $(B)/basekpq3/game/g_mem.o \
  $(B)/basekpq3/game/g_misc.o \
  $(B)/basekpq3/game/g_missile.o \
  $(B)/basekpq3/game/g_mover.o \
  $(B)/basekpq3/game/g_session.o \
  $(B)/basekpq3/game/g_spawn.o \
  $(B)/basekpq3/game/g_svcmds.o \
  $(B)/basekpq3/game/g_target.o \
  $(B)/basekpq3/game/g_team.o \
  $(B)/basekpq3/game/g_trigger.o \
  $(B)/basekpq3/game/g_utils.o \
  $(B)/basekpq3/game/g_weapon.o \
  $(B)/basekpq3/game/g_hitmen.o \
  \
  $(B)/basekpq3/qcommon/q_math.o \
  $(B)/basekpq3/qcommon/q_shared.o

Q3GOBJ = $(Q3GOBJ_) $(B)/basekpq3/game/g_syscalls.o
Q3GVMOBJ = $(Q3GOBJ_:%.o=%.asm)

$(B)/basekpq3/qagame$(SHLIBNAME): $(Q3GOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(LD) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(Q3GOBJ) $(SHLIBLIBS)

$(B)/basekpq3/vm/qagame.qvm: $(Q3GVMOBJ) $(GDIR)/g_syscalls.asm $(Q3ASM)
	$(echo_cmd) "Q3ASM $@"
	$(Q)$(Q3ASM) -o $@ $(Q3GVMOBJ) $(GDIR)/g_syscalls.asm


#############################################################################
## basekpq3 UI
#############################################################################

MPUIOBJ_ = \
  $(B)/basekpq3/ui/ui_main.o \
  $(B)/basekpq3/ui/ui_atoms.o \
  $(B)/basekpq3/ui/ui_gameinfo.o \
  $(B)/basekpq3/ui/ui_players.o \
  $(B)/basekpq3/ui/ui_xppm.o \
  $(B)/basekpq3/ui/ui_shared.o \
  \
  $(B)/basekpq3/ui/bg_misc.o \
  $(B)/basekpq3/ui/bg_lib.o \
  \
  $(B)/basekpq3/qcommon/q_math.o \
  $(B)/basekpq3/qcommon/q_shared.o

MPUIOBJ = $(MPUIOBJ_) $(B)/basekpq3/ui/ui_syscalls.o
MPUIVMOBJ = $(MPUIOBJ_:%.o=%.asm)

$(B)/basekpq3/ui$(SHLIBNAME) : $(MPUIOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(LD) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(MPUIOBJ)  $(SHLIBLIBS)

$(B)/basekpq3/vm/ui.qvm: $(MPUIVMOBJ) $(UIDIR)/ui_syscalls.asm $(Q3ASM)
	$(echo_cmd) "Q3ASM $@"
	$(Q)$(Q3ASM) -o $@ $(MPUIVMOBJ) $(UIDIR)/ui_syscalls.asm

#############################################################################
## Mapcompiler kmap2
#############################################################################

KMAP2OBJ_= \
	$(B)/$(KMAP2DIR)/brush.o \
	$(B)/$(KMAP2DIR)/brush_primit.o \
	$(B)/$(KMAP2DIR)/bsp.o \
	$(B)/$(KMAP2DIR)/bspfile_abstract.o \
	$(B)/$(KMAP2DIR)/bspfile_ibsp.o \
	$(B)/$(KMAP2DIR)/bspfile_rbsp.o \
	$(B)/$(KMAP2DIR)/bspfile_xbsp.o \
	$(B)/$(KMAP2DIR)/convert_ase.o \
	$(B)/$(KMAP2DIR)/convert_map.o \
	$(B)/$(KMAP2DIR)/decals.o \
	$(B)/$(KMAP2DIR)/facebsp.o \
	$(B)/$(KMAP2DIR)/fog.o \
	$(B)/$(KMAP2DIR)/image.o \
	$(B)/$(KMAP2DIR)/gldraw.o \
	$(B)/$(KMAP2DIR)/leakfile.o \
	$(B)/$(KMAP2DIR)/light.o \
	$(B)/$(KMAP2DIR)/light_bounce.o \
	$(B)/$(KMAP2DIR)/light_trace.o \
	$(B)/$(KMAP2DIR)/light_ydnar.o \
	$(B)/$(KMAP2DIR)/lightmaps_ydnar.o \
	$(B)/$(KMAP2DIR)/main.o \
	$(B)/$(KMAP2DIR)/map.o \
	$(B)/$(KMAP2DIR)/mesh.o \
	$(B)/$(KMAP2DIR)/model.o \
	$(B)/$(KMAP2DIR)/patch.o \
	$(B)/$(KMAP2DIR)/path_init.o \
	$(B)/$(KMAP2DIR)/portals.o \
	$(B)/$(KMAP2DIR)/prtfile.o \
	$(B)/$(KMAP2DIR)/shaders.o \
	$(B)/$(KMAP2DIR)/surface.o \
	$(B)/$(KMAP2DIR)/surface_extra.o \
	$(B)/$(KMAP2DIR)/surface_foliage.o \
	$(B)/$(KMAP2DIR)/surface_fur.o \
	$(B)/$(KMAP2DIR)/surface_meta.o \
	$(B)/$(KMAP2DIR)/tjunction.o \
	$(B)/$(KMAP2DIR)/tree.o \
	$(B)/$(KMAP2DIR)/vis.o \
	$(B)/$(KMAP2DIR)/visflow.o \
	$(B)/$(KMAP2DIR)/writebsp.o \
	$(B)/$(KMAP2DIR)/cmdlib.o \
	$(B)/$(KMAP2DIR)/mathlib.o \
	$(B)/$(KMAP2DIR)/polylib.o \
	$(B)/$(KMAP2DIR)/aselib.o \
	$(B)/$(KMAP2DIR)/imagelib.o \
	$(B)/$(KMAP2DIR)/inout.o \
	$(B)/$(KMAP2DIR)/scriplib.o \
	$(B)/$(KMAP2DIR)/threads.o \
	$(B)/$(KMAP2DIR)/mutex.o \
	$(B)/$(KMAP2DIR)/unzip.o \
	$(B)/$(KMAP2DIR)/vfs.o \
	$(B)/$(KMAP2DIR)/netlib.o \
	$(B)/$(KMAP2DIR)/picointernal.o \
	$(B)/$(KMAP2DIR)/picomodel.o \
	$(B)/$(KMAP2DIR)/picomodules.o \
	$(B)/$(KMAP2DIR)/pm_3ds.o \
	$(B)/$(KMAP2DIR)/pm_ase.o \
	$(B)/$(KMAP2DIR)/pm_fm.o \
	$(B)/$(KMAP2DIR)/pm_lwo.o \
	$(B)/$(KMAP2DIR)/pm_md2.o \
	$(B)/$(KMAP2DIR)/pm_md3.o \
	$(B)/$(KMAP2DIR)/pm_mdc.o \
	$(B)/$(KMAP2DIR)/pm_ms3d.o \
	$(B)/$(KMAP2DIR)/pm_obj.o \
	$(B)/$(KMAP2DIR)/pm_terrain.o \
	$(B)/$(KMAP2DIR)/lwo/clip.o \
	$(B)/$(KMAP2DIR)/lwo/envelope.o \
	$(B)/$(KMAP2DIR)/lwo/list.o \
	$(B)/$(KMAP2DIR)/lwo/lwio.o \
	$(B)/$(KMAP2DIR)/lwo/lwo2.o \
	$(B)/$(KMAP2DIR)/lwo/lwob.o \
	$(B)/$(KMAP2DIR)/lwo/surface.o \
	$(B)/$(KMAP2DIR)/pntspols.o \
	$(B)/$(KMAP2DIR)/vecmath.o \
	$(B)/$(KMAP2DIR)/vmap.o \
  $(B)/$(KMAP2DIR)/jaricom.o \
  $(B)/$(KMAP2DIR)/jcapimin.o \
  $(B)/$(KMAP2DIR)/jcapistd.o \
  $(B)/$(KMAP2DIR)/jcarith.o \
  $(B)/$(KMAP2DIR)/jccoefct.o \
  $(B)/$(KMAP2DIR)/jccolor.o \
  $(B)/$(KMAP2DIR)/jcdctmgr.o \
  $(B)/$(KMAP2DIR)/jchuff.o \
  $(B)/$(KMAP2DIR)/jcinit.o \
  $(B)/$(KMAP2DIR)/jcmainct.o \
  $(B)/$(KMAP2DIR)/jcmarker.o \
  $(B)/$(KMAP2DIR)/jcmaster.o \
  $(B)/$(KMAP2DIR)/jcomapi.o \
  $(B)/$(KMAP2DIR)/jcparam.o \
  $(B)/$(KMAP2DIR)/jcprepct.o \
  $(B)/$(KMAP2DIR)/jcsample.o \
  $(B)/$(KMAP2DIR)/jctrans.o \
  $(B)/$(KMAP2DIR)/jdapimin.o \
  $(B)/$(KMAP2DIR)/jdapistd.o \
  $(B)/$(KMAP2DIR)/jdarith.o \
  $(B)/$(KMAP2DIR)/jdatadst.o \
  $(B)/$(KMAP2DIR)/jdatasrc.o \
  $(B)/$(KMAP2DIR)/jdcoefct.o \
  $(B)/$(KMAP2DIR)/jdcolor.o \
  $(B)/$(KMAP2DIR)/jddctmgr.o \
  $(B)/$(KMAP2DIR)/jdhuff.o \
  $(B)/$(KMAP2DIR)/jdinput.o \
  $(B)/$(KMAP2DIR)/jdmainct.o \
  $(B)/$(KMAP2DIR)/jdmarker.o \
  $(B)/$(KMAP2DIR)/jdmaster.o \
  $(B)/$(KMAP2DIR)/jdmerge.o \
  $(B)/$(KMAP2DIR)/jdpostct.o \
  $(B)/$(KMAP2DIR)/jdsample.o \
  $(B)/$(KMAP2DIR)/jdtrans.o \
  $(B)/$(KMAP2DIR)/jerror.o \
  $(B)/$(KMAP2DIR)/jfdctflt.o \
  $(B)/$(KMAP2DIR)/jfdctfst.o \
  $(B)/$(KMAP2DIR)/jfdctint.o \
  $(B)/$(KMAP2DIR)/jidctflt.o \
  $(B)/$(KMAP2DIR)/jidctfst.o \
  $(B)/$(KMAP2DIR)/jidctint.o \
  $(B)/$(KMAP2DIR)/jmemmgr.o \
  $(B)/$(KMAP2DIR)/jmemnobs.o \
  $(B)/$(KMAP2DIR)/jquant1.o \
  $(B)/$(KMAP2DIR)/jquant2.o \
  $(B)/$(KMAP2DIR)/jutils.o \
	$(B)/$(KMAP2DIR)/png.o \
	$(B)/$(KMAP2DIR)/pngset.o \
	$(B)/$(KMAP2DIR)/pngget.o \
	$(B)/$(KMAP2DIR)/pngrutil.o \
	$(B)/$(KMAP2DIR)/pngtrans.o \
	$(B)/$(KMAP2DIR)/pngwutil.o \
	$(B)/$(KMAP2DIR)/pngread.o \
	$(B)/$(KMAP2DIR)/pngrio.o \
	$(B)/$(KMAP2DIR)/pngwio.o \
	$(B)/$(KMAP2DIR)/pngwrite.o \
	$(B)/$(KMAP2DIR)/pngrtran.o \
	$(B)/$(KMAP2DIR)/pngwtran.o \
	$(B)/$(KMAP2DIR)/pngmem.o \
	$(B)/$(KMAP2DIR)/pngerror.o \
	$(B)/$(KMAP2DIR)/pngpread.o \
	$(B)/$(KMAP2DIR)/adler32.o \
	$(B)/$(KMAP2DIR)/compress.o \
	$(B)/$(KMAP2DIR)/crc32.o \
	$(B)/$(KMAP2DIR)/gzio.o \
	$(B)/$(KMAP2DIR)/uncompr.o \
	$(B)/$(KMAP2DIR)/deflate.o \
	$(B)/$(KMAP2DIR)/trees.o \
	$(B)/$(KMAP2DIR)/zutil.o \
	$(B)/$(KMAP2DIR)/inflate.o \
	$(B)/$(KMAP2DIR)/infback.o \
	$(B)/$(KMAP2DIR)/inftrees.o \
	$(B)/$(KMAP2DIR)/inffast.o \
	$(B)/$(KMAP2DIR)/ioapi.o

KMAP2OBJ = $(KMAP2OBJ_)

$(B)/kmap2$(FULLBINEXT): $(KMAP2OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(KMAP2_CFLAGS) $(LDFLAGS) -o $@ $(KMAP2OBJ) $(KMAP2_LIBS)


#############################################################################
## AAS-compiler kaas
#############################################################################

KAASOBJ_= \
	$(B)/kaas/be_aas_bspq3.o \
	$(B)/kaas/be_aas_cluster.o \
	$(B)/kaas/be_aas_move.o \
	$(B)/kaas/be_aas_optimize.o \
	$(B)/kaas/be_aas_reach.o \
	$(B)/kaas/be_aas_sample.o \
	$(B)/kaas/_files.o \
  $(B)/kaas/aas_areamerging.o \
	$(B)/kaas/aas_cfg.o \
	$(B)/kaas/aas_create.o \
	$(B)/kaas/aas_edgemelting.o \
	$(B)/kaas/aas_facemerging.o \
	$(B)/kaas/aas_gsubdiv.o \
	$(B)/kaas/aas_map.o \
	$(B)/kaas/aas_prunenodes.o \
	$(B)/kaas/aas_store.o \
	$(B)/kaas/aas_file.o \
	$(B)/kaas/be_aas_bspc.o\
	$(B)/kaas/brushbsp.o \
	$(B)/kaas/bspc.o \
	$(B)/kaas/cm_load.o \
	$(B)/kaas/cm_patch.o \
	$(B)/kaas/cm_test.o \
	$(B)/kaas/cm_trace.o \
	$(B)/kaas/cm_polylib.o \
	$(B)/kaas/cm_trisoup.o \
	$(B)/kaas/csg.o \
	$(B)/kaas/glfile.o \
	$(B)/kaas/l_bsp_ent.o \
	$(B)/kaas/l_bsp_q3.o \
	$(B)/kaas/l_cmd.o \
	$(B)/kaas/l_libvar.o \
	$(B)/kaas/l_log.o \
	$(B)/kaas/l_mem.o \
	$(B)/kaas/l_poly.o \
	$(B)/kaas/l_precomp.o \
	$(B)/kaas/l_qfiles.o \
	$(B)/kaas/l_script.o \
	$(B)/kaas/l_struct.o \
	$(B)/kaas/l_threads.o \
	$(B)/kaas/l_utils.o \
	$(B)/kaas/leakfile.o \
	$(B)/kaas/map.o \
	$(B)/kaas/map_q3.o \
	$(B)/kaas/md4.o \
	$(B)/kaas/nodraw.o \
	$(B)/kaas/portals.o \
	$(B)/kaas/textures.o \
	$(B)/kaas/tree.o \
	$(B)/kaas/q_math.o \
	$(B)/kaas/q_shared.o \
	$(B)/kaas/unzip.o

KAASOBJ = $(KAASOBJ_)
$(B)/kaas$(FULLBINEXT): $(KAASOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(KAAS_CFLAGS) $(LDFLAGS) -o $@ $(KAASOBJ) $(KAAS_LIBS)


## static lib rules

$(B)/client/%.o: $(JPDIR)/%.c
	$(DO_JPEG_CC)
	
#############################################################################
## CLIENT/SERVER RULES
#############################################################################
$(B)/client/%.o: $(ASMDIR)/%.s
	$(DO_AS)
$(B)/client/%.o: $(ASMDIR)/%.c
	$(DO_CC) -march=k8
$(B)/client/%.o: $(CDIR)/%.cc
	$(DO_CCC)
$(B)/client/%.o: $(SDIR)/%.cc
	$(DO_CCC)
$(B)/client/%.o: $(CMDIR)/%.cc
	$(DO_CCC)
ifeq ($(USE_INTERNAL_GLEW),1)
$(B)/client/glew.o: $(GLEWDIR)/src/glew.c
	$(DO_CC)
endif
$(B)/client/%.o: $(BLIBDIR)/%.cc
	$(DO_BOT_CCC)
$(B)/client/%.o: $(PNGDIR)/%.c
	$(DO_CC)
$(B)/client/autofit.o: $(FTDIR)/src/autofit/autofit.c
	$(DO_CC)
$(B)/client/bdf.o: $(FTDIR)/src/bdf/bdf.c
	$(DO_CC)
$(B)/client/cff.o: $(FTDIR)/src/cff/cff.c
	$(DO_CC)
$(B)/client/ftbase.o: $(FTDIR)/src/base/ftbase.c
	$(DO_CC)
$(B)/client/ftbitmap.o: $(FTDIR)/src/base/ftbitmap.c
	$(DO_CC)
$(B)/client/ftglyph.o: $(FTDIR)/src/base/ftglyph.c
	$(DO_CC)
$(B)/client/ftdebug.o: $(FTDIR)/src/base/ftdebug.c
	$(DO_CC)
$(B)/client/ftgasp.o: $(FTDIR)/src/base/ftgasp.c
	$(DO_CC)
$(B)/client/ftinit.o: $(FTDIR)/src/base/ftinit.c
	$(DO_CC)
$(B)/client/ftstroke.o: $(FTDIR)/src/base/ftstroke.c
	$(DO_CC)
$(B)/client/ftsystem.o: $(FTDIR)/src/base/ftsystem.c
	$(DO_CC)
$(B)/client/ftmm.o: $(FTDIR)/src/base/ftmm.c
	$(DO_CC)
$(B)/client/ftsynth.o: $(FTDIR)/src/base/ftsynth.c
	$(DO_CC)
$(B)/client/fttype1.o: $(FTDIR)/src/base/fttype1.c
	$(DO_CC)
$(B)/client/ftwinfnt.o: $(FTDIR)/src/base/ftwinfnt.c
	$(DO_CC)
$(B)/client/ftbbox.o: $(FTDIR)/src/base/ftbbox.c
	$(DO_CC)
$(B)/client/ftpfr.o: $(FTDIR)/src/base/ftpfr.c
	$(DO_CC)
$(B)/client/ftcache.o: $(FTDIR)/src/cache/ftcache.c
	$(DO_CC)
$(B)/client/ftgzip.o: $(FTDIR)/src/gzip/ftgzip.c
	$(DO_CC)
$(B)/client/ftlzw.o: $(FTDIR)/src/lzw/ftlzw.c
	$(DO_CC)
$(B)/client/pcf.o: $(FTDIR)/src/pcf/pcf.c
	$(DO_CC)
$(B)/client/pfr.o: $(FTDIR)/src/pfr/pfr.c
	$(DO_CC)
$(B)/client/psaux.o: $(FTDIR)/src/psaux/psaux.c
	$(DO_CC)
$(B)/client/pshinter.o: $(FTDIR)/src/pshinter/pshinter.c
	$(DO_CC)
$(B)/client/psmodule.o: $(FTDIR)/src/psnames/psmodule.c
	$(DO_CC)
$(B)/client/raster.o: $(FTDIR)/src/raster/raster.c
	$(DO_CC)
$(B)/client/sfnt.o: $(FTDIR)/src/sfnt/sfnt.c
	$(DO_CC)
$(B)/client/truetype.o: $(FTDIR)/src/truetype/truetype.c
	$(DO_CC)
$(B)/client/type1.o: $(FTDIR)/src/type1/type1.c
	$(DO_CC)
$(B)/client/type1cid.o: $(FTDIR)/src/cid/type1cid.c
	$(DO_CC)
$(B)/client/type42.o: $(FTDIR)/src/type42/type42.c
	$(DO_CC)
$(B)/client/winfnt.o: $(FTDIR)/src/winfonts/winfnt.c
	$(DO_CC)
$(B)/client/smooth.o: $(FTDIR)/src/smooth/smooth.c
	$(DO_CC)
$(B)/client/theora/%.o: $(THEORADIR)/lib/%.c
	$(DO_CC)
$(B)/client/speex/%.o: $(SPEEXDIR)/%.c
	$(DO_CC)
$(B)/client/%.o: $(ZDIR)/%.c
	$(DO_CC)
$(B)/client/%.o: $(VORBISDIR)/lib/%.c
	$(DO_CC)
$(B)/client/%.o: $(OGGDIR)/src/%.c
	$(DO_CC)
$(B)/client/%.o: $(RDIR)/%.cc
	$(DO_CCC)
$(B)/client/%.o: $(SDLDIR)/%.cc
	$(DO_CCC)
$(B)/clientsmp/%.o: $(SDLDIR)/%.c
	$(DO_SMP_CC)
$(B)/client/%.o: $(SYSDIR)/%.cc
	$(DO_CCC)
$(B)/client/%.o: $(SYSDIR)/%.m
	$(DO_CC)
$(B)/client/%.o: $(SYSDIR)/%.rc
	$(DO_WINDRES)
$(B)/ded/%.o: $(ASMDIR)/%.s
	$(DO_AS)
# k8 so inline assembler knows about SSE
$(B)/ded/%.o: $(ASMDIR)/%.c
	$(DO_CC) -march=k8
$(B)/ded/%.o: $(SDIR)/%.cc
	$(DO_DED_CCC)
$(B)/ded/%.o: $(CMDIR)/%.cc
	$(DO_DED_CCC)	
$(B)/ded/%.o: $(ZDIR)/%.c
	$(DO_DED_CC)
$(B)/ded/%.o: $(BLIBDIR)/%.cc
	$(DO_BOT_CCC)
$(B)/ded/%.o: $(SYSDIR)/%.cc
	$(DO_DED_CCC)
$(B)/ded/%.o: $(SYSDIR)/%.m
	$(DO_DED_CCC)
$(B)/ded/%.o: $(SYSDIR)/%.rc
	$(DO_WINDRES)
$(B)/ded/%.o: $(NDIR)/%.cc
	$(DO_DED_CCC)

# WTF is this good for ??
# Extra dependencies to ensure the SVN version is incorporated
ifeq ($(USE_SVN),1)
  #$(B)/client/cl_console.o : .svn/entries
  #$(B)/client/common.o : .svn/entries
  #$(B)/ded/common.o : .svn/entries
endif


#############################################################################
## GAME MODULE RULES
#############################################################################

ifeq ($(USE_C_DLL), 1)
$(B)/basekpq3/cgame/bg_%.o: $(GDIR)/bg_%.c
	$(DO_CGAME_CC)
$(B)/basekpq3/cgame/%.o: $(CGDIR)/%.c
	$(DO_CGAME_CC)
endif
ifeq ($(USE_C_DLL), 0)
$(B)/basekpq3/cgame/%.o: $(CGDIR)/%.c
	$(DO_CGAME_CCC)
$(B)/basekpq3/cgame/bg_%.o: $(GDIR)/bg_%.c
	$(DO_CGAME_CCC)
endif

$(B)/basekpq3/cgame/bg_%.asm: $(GDIR)/bg_%.c $(Q3LCC)
	$(DO_CGAME_Q3LCC)
$(B)/basekpq3/cgame/%.asm: $(CGDIR)/%.c $(Q3LCC)
	$(DO_CGAME_Q3LCC)

ifeq ($(USE_C_DLL), 1)
$(B)/basekpq3/game/%.o: $(GDIR)/%.c
	$(DO_GAME_CC)
endif
ifeq ($(USE_C_DLL), 0)
$(B)/basekpq3/game/%.o: $(GDIR)/%.c
	$(DO_GAME_CCC)
endif	

$(B)/basekpq3/game/%.asm: $(GDIR)/%.c $(Q3LCC)
	$(DO_GAME_Q3LCC)

ifeq ($(USE_C_DLL), 1)
$(B)/basekpq3/ui/bg_%.o: $(GDIR)/bg_%.c
	$(DO_UI_CC)
$(B)/basekpq3/ui/%.o: $(UIDIR)/%.c
	$(DO_UI_CC)
endif

ifeq ($(USE_C_DLL), 0)
$(B)/basekpq3/ui/bg_%.o: $(GDIR)/bg_%.c
	$(DO_UI_CCC)
$(B)/basekpq3/ui/%.o: $(UIDIR)/%.c
	$(DO_UI_CCC)
endif

$(B)/basekpq3/ui/bg_%.asm: $(GDIR)/bg_%.c $(Q3LCC)
	$(DO_UI_Q3LCC)

$(B)/basekpq3/ui/%.asm: $(UIDIR)/%.c $(Q3LCC)
	$(DO_UI_Q3LCC)

ifeq ($(USE_C_DLL), 1)
$(B)/basekpq3/qcommon/%.o: $(CMDIR)/%.cc
	$(DO_SHLIB_CC)
endif
ifeq ($(USE_C_DLL), 0)
$(B)/basekpq3/qcommon/%.o: $(CMDIR)/%.cc
	$(DO_SHLIB_CCC)
endif

$(B)/basekpq3/qcommon/%.asm: $(CMDIR)/%.cc $(Q3LCC)
	$(DO_Q3LCC)

#############################################################################
## MAP COMPILER RULES
#############################################################################
$(B)/kmap2/%.o: $(KMAP2DIR)/%.c
	$(DO_KMAP2_CC)
$(B)/kmap2/%.o: $(KMAP2DIR)/common/%.c
	$(DO_KMAP2_CC)
$(B)/kmap2/%.o: $(KMAP2DIR)/libs/picomodel/%.c
	$(DO_KMAP2_CC)
$(B)/kmap2/%.o: $(KMAP2DIR)/libs/picomodel/lwo/%.c
	$(DO_KMAP2_CC)
$(B)/kmap2/%.o: $(KMAP2DIR)/zlib/%.c
	$(DO_KMAP2_CC)
$(B)/kmap2/%.o: $(PNGDIR)/%.c
	$(DO_KMAP2_CC)
$(B)/kmap2/%.o: $(KMAP2DIR)/jpeg/%.c
	$(DO_KMAP2_CC)

#############################################################################
## AAS COMPILER RULES
#############################################################################
$(B)/kaas/%.o: $(KAASDIR)/%.c
	$(DO_KAAS_CC)
$(B)/kaas/be_aas_bspq3.o: $(BLIBDIR)/be_aas_bspq3.cc
	$(DO_KAAS_CC)
$(B)/kaas/be_aas_cluster.o: $(BLIBDIR)/be_aas_cluster.cc
	$(DO_KAAS_CC)
$(B)/kaas/be_aas_move.o: $(BLIBDIR)/be_aas_move.cc
	$(DO_KAAS_CC)
$(B)/kaas/be_aas_optimize.o: $(BLIBDIR)/be_aas_optimize.cc
	$(DO_KAAS_CC)
$(B)/kaas/be_aas_reach.o: $(BLIBDIR)/be_aas_reach.cc
	$(DO_KAAS_CC)
$(B)/kaas/be_aas_sample.o: $(BLIBDIR)/be_aas_sample.cc
	$(DO_KAAS_CC)
$(B)/kaas/l_libvar.o: $(BLIBDIR)/l_libvar.cc
	$(DO_KAAS_CC)
$(B)/kaas/l_precomp.o: $(BLIBDIR)/l_precomp.cc
	$(DO_KAAS_CC)
$(B)/kaas/l_script.o: $(BLIBDIR)/l_script.cc
	$(DO_KAAS_CC)
$(B)/kaas/l_struct.o: $(BLIBDIR)/l_struct.cc
	$(DO_KAAS_CC)
$(B)/kaas/cm_load.o: $(KAASDIR)/../qcommon/cm_load.cc
	$(DO_KAAS_CC)
$(B)/kaas/cm_patch.o: $(KAASDIR)/../qcommon/cm_patch.cc
	$(DO_KAAS_CC)
$(B)/kaas/cm_test.o: $(KAASDIR)/../qcommon/cm_test.cc
	$(DO_KAAS_CC)
$(B)/kaas/cm_trace.o: $(KAASDIR)/../qcommon/cm_trace.cc
	$(DO_KAAS_CC)
$(B)/kaas/cm_polylib.o: $(KAASDIR)/../qcommon/cm_polylib.cc
	$(DO_KAAS_CC)
$(B)/kaas/cm_trisoup.o: $(KAASDIR)/../qcommon/cm_trisoup.cc
	$(DO_KAAS_CC)
$(B)/kaas/md4.o: $(KAASDIR)/../qcommon/md4.cc
	$(DO_KAAS_CC)
$(B)/kaas/q_shared.o: $(KAASDIR)/../qcommon/q_shared.cc
	$(DO_KAAS_CC)
$(B)/kaas/q_math.o: $(KAASDIR)/../qcommon/q_math.cc
	$(DO_KAAS_CC)

#############################################################################
# MISC
#############################################################################

OBJ = $(Q3OBJ) $(Q3POBJ) $(Q3POBJ_SMP) $(Q3DOBJ) \
  $(MPGOBJ) $(Q3GOBJ) $(Q3CGOBJ) $(MPCGOBJ) $(Q3UIOBJ) $(MPUIOBJ) \
  $(MPGVMOBJ) $(Q3GVMOBJ) $(Q3CGVMOBJ) $(MPCGVMOBJ) $(Q3UIVMOBJ) $(MPUIVMOBJ) $(KMAP2OBJ) $(KAASOBJ) $(JPEGOBJ)
TOOLSOBJ = $(LBURGOBJ) $(Q3CPPOBJ) $(Q3RCCOBJ) $(Q3LCCOBJ) $(Q3ASMOBJ)

copyfiles: release
	@if [ ! -d $(COPYDIR)/basekpq3 ]; then echo "You need to set COPYDIR to where your KingpinQ3 data is!"; fi
	-$(MKDIR) -p -m 0755 $(COPYDIR)/basekpq3

ifneq ($(BUILD_CLIENT),0)
	$(INSTALL) $(STRIP_FLAG) -m 0755 $(BR)/kingpinq3$(FULLBINEXT) $(COPYDIR)/kingpinq3$(FULLBINEXT)
endif

# Don't copy the SMP until it's working together with SDL.
#ifneq ($(BUILD_CLIENT_SMP),0)
#	$(INSTALL) $(STRIP_FLAG) -m 0755 $(BR)/kingpinq3-smp$(FULLBINEXT) $(COPYDIR)/kingpinq3-smp$(FULLBINEXT)
#endif

ifneq ($(BUILD_SERVER),0)
	@if [ -f $(BR)/kpq3ded$(FULLBINEXT) ]; then \
		$(INSTALL) $(STRIP_FLAG) -m 0755 $(BR)/kpq3ded$(FULLBINEXT) $(COPYDIR)/kpq3ded$(FULLBINEXT); \
	fi
endif

ifneq ($(BUILD_GAME_SO),0)
	$(INSTALL) $(STRIP_FLAG) -m 0755 $(BR)/basekpq3/cgame$(SHLIBNAME) \
					$(COPYDIR)/basekpq3/.
	$(INSTALL) $(STRIP_FLAG) -m 0755 $(BR)/basekpq3/qagame$(SHLIBNAME) \
					$(COPYDIR)/basekpq3/.
	$(INSTALL) $(STRIP_FLAG) -m 0755 $(BR)/basekpq3/ui$(SHLIBNAME) \
					$(COPYDIR)/basekpq3/.
endif

clean: clean-debug clean-release
ifeq ($(PLATFORM),mingw32)
	@$(MAKE) -C $(NSISDIR) clean
else
	@$(MAKE) -C $(LOKISETUPDIR) clean
endif

clean-debug:
	@$(MAKE) clean2 B=$(BD)

clean-release:
	@$(MAKE) clean2 B=$(BR)

clean2:
	@echo "CLEAN $(B)"
	@rm -f $(OBJ)
	@rm -f $(OBJ_D_FILES)
	@rm -f $(TARGETS)

toolsclean: toolsclean-debug toolsclean-release

toolsclean-debug:
	@$(MAKE) toolsclean2 B=$(BD)

toolsclean-release:
	@$(MAKE) toolsclean2 B=$(BR)

toolsclean2:
	@echo "TOOLS_CLEAN $(B)"
	@rm -f $(TOOLSOBJ)
	@rm -f $(TOOLSOBJ_D_FILES)
	@rm -f $(LBURG) $(DAGCHECK_C) $(Q3RCC) $(Q3CPP) $(Q3LCC) $(Q3ASM)

distclean: clean toolsclean
	@rm -rf $(BUILD_DIR)

installer: release
ifeq ($(PLATFORM),mingw32)
	@$(MAKE) VERSION=$(VERSION) -C $(NSISDIR) V=$(V)
else
	@$(MAKE) VERSION=$(VERSION) -C $(LOKISETUPDIR) V=$(V)
endif

dist:
	rm -rf kingpinq3-$(VERSION)
	svn export . kingpinq3-$(VERSION)
	tar --owner=root --group=root --force-local -cjf kingpinq3-$(VERSION).tar.bz2 kingpinq3-$(VERSION)
	rm -rf kingpinq3-$(VERSION)

#############################################################################
# DEPENDENCIES
#############################################################################

ifneq ($(B),)
  OBJ_D_FILES=$(filter %.d,$(OBJ:%.o=%.d))
  TOOLSOBJ_D_FILES=$(filter %.d,$(TOOLSOBJ:%.o=%.d))
  -include $(OBJ_D_FILES) $(TOOLSOBJ_D_FILES)
endif

.PHONY: all clean clean2 clean-debug clean-release copyfiles \
	debug default dist distclean installer makedirs \
	release targets \
	toolsclean toolsclean2 toolsclean-debug toolsclean-release \
	$(OBJ_D_FILES) $(TOOLSOBJ_D_FILES)

#Welcome to what must be the most terrible makefile ever (but hey, it works)
#Both clang & gcc work fine - clang seems to output faster code
#.NET 6 (and its dev tools) must be installed to compile the UI.
#The emulation core also requires SDL2.
#Run "make" to build, "make run" to run

MESENFLAGS=

ifeq ($(USE_GCC),true)
	CXX=g++
	CC=gcc
	PROFILE_GEN_FLAG=-fprofile-generate
	PROFILE_USE_FLAG=-fprofile-use
else
	CXX=clang++
	CC=clang
	PROFILE_GEN_FLAG = -fprofile-instr-generate=$(CURDIR)/PGOHelper/pgo.profraw
	PROFILE_USE_FLAG = -fprofile-instr-use=$(CURDIR)/PGOHelper/pgo.profdata
endif

SDL2LIB=$(shell sdl2-config --libs)
SDL2INC=$(shell sdl2-config --cflags)

CXXFLAGS=-fPIC -Wall --std=c++17 -O3 $(MESENFLAGS) $(SDL2INC) -I $(realpath ./) -I $(realpath ./Core) -I $(realpath ./Utilities) -I $(realpath ./Linux)
CFLAGS=-fPIC -Wall -O3 $(MESENFLAGS)

LINKCHECKUNRESOLVED=-Wl,-z,defs 

LINKOPTIONS=
MESENOS=

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
	MESENOS=linux
	SHAREDLIB=MesenCore.so
endif

ifeq ($(UNAME_S),Darwin)
	MESENOS=osx
	SHAREDLIB=MesenCore.dylib
	LTO=false
	STATICLINK := false
	LINKCHECKUNRESOLVED=
endif

UNAME_P := $(shell uname -p)
ifeq ($(UNAME_P),x86_64)
	MESENPLATFORM=$(MESENOS)-x64
endif
ifneq ($(filter %86,$(UNAME_P)),)
	MESENPLATFORM=$(MESENOS)-x64
endif
ifneq ($(filter arm%,$(UNAME_P)),)
	MESENPLATFORM=$(MESENOS)-arm64
endif

CXXFLAGS += -m64
CFLAGS += -m64

ifneq ($(LTO),false)
	CFLAGS += -flto
	CXXFLAGS += -flto
endif

ifeq ($(PGO),profile)
	CFLAGS += ${PROFILE_GEN_FLAG}
	CXXFLAGS += ${PROFILE_GEN_FLAG}
endif

ifeq ($(PGO),optimize)
	CFLAGS += ${PROFILE_USE_FLAG}
	CXXFLAGS += ${PROFILE_USE_FLAG}
endif

ifneq ($(STATICLINK),false)
	LINKOPTIONS += -static-libgcc -static-libstdc++ 
endif

OBJFOLDER=obj.$(MESENPLATFORM)
RELEASEFOLDER=bin/$(MESENPLATFORM)/Release

CORESRC := $(shell find Core -name '*.cpp')
COREOBJ := $(CORESRC:.cpp=.o)

UTILSRC := $(shell find Utilities -name '*.cpp' -o -name '*.c')
UTILOBJ := $(addsuffix .o,$(basename $(UTILSRC)))

LINUXSRC := $(shell find Linux -name '*.cpp')
LINUXOBJ := $(LINUXSRC:.cpp=.o)

SEVENZIPSRC := $(shell find SevenZip -name '*.c')
SEVENZIPOBJ := $(SEVENZIPSRC:.c=.o)

LUASRC := $(shell find Lua -name '*.c')
LUAOBJ := $(LUASRC:.c=.o)

DLLSRC := $(shell find InteropDLL -name '*.cpp')
DLLOBJ := $(DLLSRC:.cpp=.o)

ifeq ($(SYSTEM_LIBEVDEV), true)
	LIBEVDEVLIB=$(shell pkg-config --libs libevdev)
	LIBEVDEVINC=$(shell pkg-config --cflags libevdev)
else
	LIBEVDEVSRC := $(shell find Linux/libevdev -name '*.c')
	LIBEVDEVOBJ := $(LIBEVDEVSRC:.c=.o)
	LIBEVDEVINC=-I../
endif

FSLIB=-lstdc++fs

ifeq ($(MESENOS),osx)
	LIBEVDEVOBJ := 
	LIBEVDEVINC := 
	LIBEVDEVSRC := 
	FSLIB := 
endif

all: ui

ui: InteropDLL/$(OBJFOLDER)/$(SHAREDLIB)
	mkdir -p $(RELEASEFOLDER)/Dependencies
	rm -fr $(RELEASEFOLDER)/Dependencies/*
	cp InteropDLL/$(OBJFOLDER)/$(SHAREDLIB) bin/$(MESENPLATFORM)/Release/$(SHAREDLIB)
	#Called twice because the first call copies native libraries to the bin folder which need to be included in Dependencies.zip
	cd UI && dotnet publish -c Release -r $(MESENPLATFORM) -p:OptimizeUi="true" --no-self-contained true -p:PublishSingleFile=true
	cd UI && dotnet publish -c Release -r $(MESENPLATFORM) -p:OptimizeUi="true" --no-self-contained true -p:PublishSingleFile=true

core: InteropDLL/$(OBJFOLDER)/$(SHAREDLIB)

pgohelper: InteropDLL/$(OBJFOLDER)/$(SHAREDLIB)
	mkdir -p PGOHelper/$(OBJFOLDER) && cd PGOHelper/$(OBJFOLDER) && $(CXX) $(CXXFLAGS) $(LINKCHECKUNRESOLVED) -o pgohelper ../PGOHelper.cpp ../../bin/pgohelperlib.so -pthread $(FSLIB) $(SDL2LIB) $(LIBEVDEVLIB)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

InteropDLL/$(OBJFOLDER)/$(SHAREDLIB): $(SEVENZIPOBJ) $(LUAOBJ) $(UTILOBJ) $(COREOBJ) $(LIBEVDEVOBJ) $(LINUXOBJ) $(DLLOBJ)
	mkdir -p bin
	mkdir -p InteropDLL/$(OBJFOLDER)
	$(CXX) $(CXXFLAGS) $(LINKOPTIONS) $(LINKCHECKUNRESOLVED) -shared -o $(SHAREDLIB) $(DLLOBJ) $(SEVENZIPOBJ) $(LUAOBJ) $(LINUXOBJ) $(LIBEVDEVOBJ) $(UTILOBJ) $(COREOBJ) $(SDL2INC) -pthread $(FSLIB) $(SDL2LIB) $(LIBEVDEVLIB)
	cp $(SHAREDLIB) bin/pgohelperlib.so
	mv $(SHAREDLIB) InteropDLL/$(OBJFOLDER)

pgo:
	./buildPGO.sh

run:
	./bin/$(MESENPLATFORM)/Release/$(MESENPLATFORM)/publish/Mesen

clean:
	rm -r -f $(COREOBJ)
	rm -r -f $(UTILOBJ)
	rm -r -f $(LINUXOBJ) $(LIBEVDEVOBJ)
	rm -r -f $(SEVENZIPOBJ)
	rm -r -f $(LUAOBJ)
	rm -r -f $(DLLOBJ)

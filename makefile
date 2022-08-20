#Welcome to what must be the most terrible makefile ever (but hey, it works)
#Both clang & gcc work fine - clang seems to output faster code
#.NET 6 (and its dev tools) must be installed to compiled the UI.
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

CXXFLAGS=-fPIC -Wall --std=c++17 -O3 $(MESENFLAGS) -I/usr/include/SDL2 -I $(realpath ./) -I $(realpath ./Core) -I $(realpath ./Utilities) -I $(realpath ./Linux)
CFLAGS=-fPIC -Wall -O3 $(MESENFLAGS)
LINKOPTIONS=

ifeq ($(MESENPLATFORM),x86)
	MESENPLATFORM=x86

	CXXFLAGS += -m32
	CFLAGS += -m32
else
	MESENPLATFORM=x64
	CXXFLAGS += -m64
	CFLAGS += -m64
endif

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
SHAREDLIB=libMesenSCore.dll
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

SDL2LIB=$(shell sdl2-config --libs)
SDL2INC=$(shell sdl2-config --cflags)
FSLIB=-lstdc++fs

all: ui

ui: InteropDLL/$(OBJFOLDER)/$(SHAREDLIB)
	mkdir -p $(RELEASEFOLDER)/Dependencies
	rm -fr $(RELEASEFOLDER)/Dependencies/*
	cp InteropDLL/$(OBJFOLDER)/$(SHAREDLIB) bin/x64/Release/$(SHAREDLIB)
	cd NewUI && dotnet publish -c Release -r linux-x64 -p:Platform="$(MESENPLATFORM)" -p:OptimizeUi="true" --no-self-contained true -p:PublishSingleFile=true
	rm $(RELEASEFOLDER)/linux-x64/publish/lib*

core: InteropDLL/$(OBJFOLDER)/$(SHAREDLIB)

runtests:
	cd TestHelper/$(OBJFOLDER) && ./testhelper

testhelper: InteropDLL/$(OBJFOLDER)/$(SHAREDLIB)
	mkdir -p TestHelper/$(OBJFOLDER)
	$(CXX) $(CXXFLAGS) -Wl,-z,defs -o testhelper TestHelper/*.cpp InteropDLL/ConsoleWrapper.cpp $(SEVENZIPOBJ) $(LUAOBJ) $(LINUXOBJ) $(LIBEVDEVOBJ) $(UTILOBJ) $(COREOBJ) -pthread $(FSLIB) $(SDL2LIB) $(LIBEVDEVLIB)
	mv testhelper TestHelper/$(OBJFOLDER)

pgohelper: InteropDLL/$(OBJFOLDER)/$(SHAREDLIB)
	mkdir -p PGOHelper/$(OBJFOLDER) && cd PGOHelper/$(OBJFOLDER) && $(CXX) $(CXXFLAGS) -Wl,-z,defs -o pgohelper ../PGOHelper.cpp ../../bin/pgohelperlib.so -pthread $(FSLIB) $(SDL2LIB) $(LIBEVDEVLIB)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

InteropDLL/$(OBJFOLDER)/$(SHAREDLIB): $(SEVENZIPOBJ) $(LUAOBJ) $(UTILOBJ) $(COREOBJ) $(LIBEVDEVOBJ) $(LINUXOBJ) $(DLLOBJ)
	mkdir -p bin
	mkdir -p InteropDLL/$(OBJFOLDER)
	$(CXX) $(CXXFLAGS) $(LINKOPTIONS) -Wl,-z,defs -shared -o $(SHAREDLIB) $(DLLOBJ) $(SEVENZIPOBJ) $(LUAOBJ) $(LINUXOBJ) $(LIBEVDEVOBJ) $(UTILOBJ) $(COREOBJ) $(SDL2INC) -pthread $(FSLIB) $(SDL2LIB) $(LIBEVDEVLIB)
	cp $(SHAREDLIB) bin/pgohelperlib.so
	mv $(SHAREDLIB) InteropDLL/$(OBJFOLDER)

pgo:
	./buildPGO.sh
	
official:
	./build.sh

run:
	./NewUI/bin/x64/Release/linux-x64/publish/Mesen

clean:
	rm -r $(COREOBJ)
	rm -r $(UTILOBJ)
	rm -r $(LINUXOBJ)
	rm -r $(SEVENZIPOBJ)
	rm -r $(LUAOBJ)
	rm -r $(DLLOBJ)

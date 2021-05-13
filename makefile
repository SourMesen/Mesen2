#Welcome to what must be the most terrible makefile ever (but hey, it works)
#Both clang & gcc work fine - clang seems to output faster code
#The only external dependency is SDL2 - everything else is pretty standard.
#Run "make" to build, "make run" to run

#----------------------
#Platform Configuration
#----------------------
#To specify whether you want to build for x86 or x64:
#"MESENPLATFORM=x86 make" or "MESENPLATFORM=x64 make"
#Default is x64

#-----------------------
# Link Time Optimization
#-----------------------
#LTO is supported for clang and gcc (but only seems to help for clang?)
#LTO gives a 25-30% performance boost, so use it whenever you can
#Usage: LTO=true make

MESENFLAGS=
libretro : MESENFLAGS=-D LIBRETRO

ifeq ($(USE_GCC),true)
	CPPC=g++
	CC=gcc
	PROFILE_GEN_FLAG=-fprofile-generate
	PROFILE_USE_FLAG=-fprofile-use
else
	CPPC=clang++
	CC=clang
	PROFILE_GEN_FLAG = -fprofile-instr-generate=$(CURDIR)/PGOHelper/pgo.profraw
	PROFILE_USE_FLAG = -fprofile-instr-use=$(CURDIR)/PGOHelper/pgo.profdata
endif

GCCOPTIONS=-fPIC -Wall --std=c++17 -O3 $(MESENFLAGS) -I/usr/include/SDL2 -I $(realpath ./) -I $(realpath ./Core) -I $(realpath ./Utilities) -I $(realpath ./Linux)
CCOPTIONS=-fPIC -Wall -O3 $(MESENFLAGS)
LINKOPTIONS=

ifeq ($(MESENPLATFORM),x86)
	MESENPLATFORM=x86

	GCCOPTIONS += -m32
	CCOPTIONS += -m32
else
	MESENPLATFORM=x64
	GCCOPTIONS += -m64
	CCOPTIONS += -m64
endif

ifeq ($(LTO),true)
	CCOPTIONS += -flto
	GCCOPTIONS += -flto
endif

ifeq ($(PGO),profile)
	CCOPTIONS += ${PROFILE_GEN_FLAG}
	GCCOPTIONS += ${PROFILE_GEN_FLAG}
endif

ifeq ($(PGO),optimize)
	CCOPTIONS += ${PROFILE_USE_FLAG}
	GCCOPTIONS += ${PROFILE_USE_FLAG}
endif

ifeq ($(STATICLINK),true)
	LINKOPTIONS += -static-libgcc -static-libstdc++ 
endif

OBJFOLDER=obj.$(MESENPLATFORM)
SHAREDLIB=libMesenSCore.dll
LIBRETROLIB=mesen-s_libretro.$(MESENPLATFORM).so
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
	cd NewUI && dotnet publish -c Release -r linux-x64 -p:Platform="$(MESENPLATFORM)" --self-contained true -p:PublishSingleFile=true
	cp InteropDLL/$(OBJFOLDER)/$(SHAREDLIB) NewUI/bin/x64/Release/linux-x64/publish/$(SHAREDLIB)

libretro: Libretro/$(OBJFOLDER)/$(LIBRETROLIB)
	mkdir -p bin
	cp ./Libretro/$(OBJFOLDER)/$(LIBRETROLIB) ./bin/

core: InteropDLL/$(OBJFOLDER)/$(SHAREDLIB)

runtests:
	cd TestHelper/$(OBJFOLDER) && ./testhelper

testhelper: InteropDLL/$(OBJFOLDER)/$(SHAREDLIB)
	mkdir -p TestHelper/$(OBJFOLDER)
	$(CPPC) $(GCCOPTIONS) -Wl,-z,defs -o testhelper TestHelper/*.cpp InteropDLL/ConsoleWrapper.cpp $(SEVENZIPOBJ) $(LUAOBJ) $(LINUXOBJ) $(LIBEVDEVOBJ) $(UTILOBJ) $(COREOBJ) -pthread $(FSLIB) $(SDL2LIB) $(LIBEVDEVLIB)
	mv testhelper TestHelper/$(OBJFOLDER)

pgohelper: InteropDLL/$(OBJFOLDER)/$(SHAREDLIB)
	mkdir -p PGOHelper/$(OBJFOLDER) && cd PGOHelper/$(OBJFOLDER) && $(CPPC) $(GCCOPTIONS) -Wl,-z,defs -o pgohelper ../PGOHelper.cpp ../../bin/pgohelperlib.so -pthread $(FSLIB) $(SDL2LIB) $(LIBEVDEVLIB)

%.o: %.c
	$(CC) $(CCOPTIONS) -c $< -o $@
	
%.o: %.cpp
	$(CPPC) $(GCCOPTIONS) -c $< -o $@

InteropDLL/$(OBJFOLDER)/$(SHAREDLIB): $(SEVENZIPOBJ) $(LUAOBJ) $(UTILOBJ) $(COREOBJ) $(LIBEVDEVOBJ) $(LINUXOBJ) $(DLLOBJ)
	mkdir -p bin
	mkdir -p InteropDLL/$(OBJFOLDER)
	$(CPPC) $(GCCOPTIONS) $(LINKOPTIONS) -Wl,-z,defs -shared -o $(SHAREDLIB) $(DLLOBJ) $(SEVENZIPOBJ) $(LUAOBJ) $(LINUXOBJ) $(LIBEVDEVOBJ) $(UTILOBJ) $(COREOBJ) $(SDL2INC) -pthread $(FSLIB) $(SDL2LIB) $(LIBEVDEVLIB)
	cp $(SHAREDLIB) bin/pgohelperlib.so
	mv $(SHAREDLIB) InteropDLL/$(OBJFOLDER)
	
Libretro/$(OBJFOLDER)/$(LIBRETROLIB): $(SEVENZIPOBJ) $(UTILOBJ) $(COREOBJ) Libretro/libretro.cpp
	mkdir -p bin
	mkdir -p Libretro/$(OBJFOLDER)
	$(CPPC) $(GCCOPTIONS) $(LINKOPTIONS) -Wl,-z,defs -shared -o $(LIBRETROLIB) Libretro/*.cpp $(SEVENZIPOBJ) $(UTILOBJ) $(COREOBJ) -pthread
	cp $(LIBRETROLIB) bin/pgohelperlib.so
	mv $(LIBRETROLIB) Libretro/$(OBJFOLDER) 

pgo:
	./buildPGO.sh
	
official:
	./build.sh

run:
	./NewUI/bin/x64/Release/linux-x64/publish/Mesen-X

clean:
	rm -r $(COREOBJ)
	rm -r $(UTILOBJ)
	rm -r $(LINUXOBJ)
	rm -r $(SEVENZIPOBJ)
	rm -r $(LUAOBJ)
	rm -r $(DLLOBJ)

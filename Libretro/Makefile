STATIC_LINKING := 0
AR             ?= ar

SPACE :=
SPACE := $(SPACE) $(SPACE)
BACKSLASH :=
BACKSLASH := \$(BACKSLASH)
filter_out1 = $(filter-out $(firstword $1),$1)
filter_out2 = $(call filter_out1,$(call filter_out1,$1))

ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
else ifneq ($(findstring win,$(shell uname -a)),)
   platform = win
endif
endif

# system platform
system_platform = unix
ifeq ($(shell uname -a),)
  EXE_EXT = .exe
  system_platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
  system_platform = osx
  arch = intel
ifeq ($(shell uname -p),powerpc)
  arch = ppc
endif
else ifneq ($(findstring MINGW,$(shell uname -a)),)
  system_platform = win
endif

LIBRETRO_DIR    += .
SEVENZIP_DIR    += ../SevenZip
CORE_DIR        += ../Core
UTIL_DIR        += ../Utilities

TARGET_NAME := mesen-s
LIBM        = -lm

ifeq ($(ARCHFLAGS),)
ifeq ($(archs),ppc)
   ARCHFLAGS = -arch ppc -arch ppc64
else
   ARCHFLAGS = -arch i386 -arch x86_64
endif
endif

ifeq ($(platform), osx)
ifndef ($(NOUNIVERSAL))
   CXXFLAGS += $(ARCHFLAGS)
   LFLAGS += $(ARCHFLAGS)
endif
endif

ifeq ($(STATIC_LINKING), 1)
EXT := a
endif

ifeq ($(platform), unix)
  EXT ?= so
   TARGET := $(TARGET_NAME)_libretro.$(EXT)
   fpic := -fPIC -pthread
   SHARED := -shared -Wl,--version-script=$(LIBRETRO_DIR)/link.T -Wl,--no-undefined

else ifeq ($(platform), linux-portable)
   TARGET := $(TARGET_NAME)_libretro.$(EXT)
   fpic := -fPIC -nostdlib
   SHARED := -shared -Wl,--version-script=$(LIBRETRO_DIR)/link.T
  LIBM :=

# Classic Platforms ####################
# Platform affix = classic_<ISA>_<µARCH>
# Help at https://modmyclassic.com/comp

# (armv7 a7, hard point, neon based) ### 
# NESC, SNESC, C64 mini 
else ifeq ($(platform), classic_armv7_a7)
	TARGET := $(TARGET_NAME)_libretro.so
	fpic := -fPIC -pthread
	SHARED := -shared -Wl,--version-script=$(LIBRETRO_DIR)/link.T -Wl,--no-undefined
	CFLAGS += -Ofast \
	-flto=4 -fwhole-program -fuse-linker-plugin \
	-fdata-sections -ffunction-sections -Wl,--gc-sections \
	-fno-stack-protector -fno-ident -fomit-frame-pointer \
	-falign-functions=1 -falign-jumps=1 -falign-loops=1 \
	-fno-unwind-tables -fno-asynchronous-unwind-tables -fno-unroll-loops \
	-fmerge-all-constants -fno-math-errno \
	-marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard
	CXXFLAGS += $(CFLAGS)
	CPPFLAGS += $(CFLAGS)
	ASFLAGS += $(CFLAGS)
	HAVE_NEON = 1
	ARCH = arm
	BUILTIN_GPU = neon
	USE_DYNAREC = 1
	ifeq ($(shell echo `$(CC) -dumpversion` "< 4.9" | bc -l), 1)
	  CFLAGS += -march=armv7-a
	else
	  CFLAGS += -march=armv7ve
	  # If gcc is 5.0 or later
	  ifeq ($(shell echo `$(CC) -dumpversion` ">= 5" | bc -l), 1)
	    LDFLAGS += -static-libgcc -static-libstdc++
	  endif
	endif
#######################################

else ifneq (,$(findstring osx,$(platform)))
   TARGET := $(TARGET_NAME)_libretro.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
else ifneq (,$(findstring ios,$(platform)))
   TARGET := $(TARGET_NAME)_libretro_ios.dylib
  fpic := -fPIC
  SHARED := -dynamiclib

ifeq ($(IOSSDK),)
   IOSSDK := $(shell xcodebuild -version -sdk iphoneos Path)
endif

  DEFINES := -DIOS
ifeq ($(platform), ios-arm64)
  CC = cc -arch arm64 -isysroot $(IOSSDK) -stdlib=libc++
  CXX = c++ -arch arm64 -isysroot $(IOSSDK) -stdlib=libc++
else
  CC = cc -arch armv7 -isysroot $(IOSSDK)
  CXX = c++ -arch armv7 -isysroot $(IOSSDK)
endif
ifeq ($(platform),$(filter $(platform),ios9 ios-arm64))
CC     += -miphoneos-version-min=8.0
CXXFLAGS += -miphoneos-version-min=8.0
else
CC     += -miphoneos-version-min=5.0
CXXFLAGS += -miphoneos-version-min=5.0
endif
else ifneq (,$(findstring qnx,$(platform)))
  TARGET := $(TARGET_NAME)_libretro_qnx.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=$(LIBRETRO_DIR)/link.T -Wl,--no-undefined
else ifeq ($(platform), emscripten)
   TARGET := $(TARGET_NAME)_libretro_emscripten.bc
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=$(LIBRETRO_DIR)/link.T -Wl,--no-undefined
else ifeq ($(platform), vita)
   TARGET := $(TARGET_NAME)_vita.a
   CC = arm-vita-eabi-gcc
   AR = arm-vita-eabi-ar
   CXXFLAGS += -Wl,-q -Wall -O3
  STATIC_LINKING = 1
# Windows MSVC 2017 all architectures
else ifneq (,$(findstring windows_msvc2017,$(platform)))

  PlatformSuffix = $(subst windows_msvc2017_,,$(platform))
  ifneq (,$(findstring desktop,$(PlatformSuffix)))
    WinPartition = desktop
    MSVC2017CompileFlags = -DWINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP -FS -MP -GL -W0 -Gy -Zc:wchar_t -Gm- -Ox -Ob2 -sdl- -Zc:inline -fp:precise -DLIBRETRO -DWIN32 -D_CONSOLE -D_LIB -D_UNICODE -DUNICODE -errorReport:prompt -WX- -Zc:forScope -Gd -Oy -Oi -MT -EHsc -Ot -diagnostics:classic
    LDFLAGS += -MANIFEST -LTCG:incremental -NXCOMPAT -DYNAMICBASE -OPT:REF -INCREMENTAL:NO -SUBSYSTEM:WINDOWS -MANIFESTUAC:"level='asInvoker' uiAccess='false'" -OPT:ICF -ERRORREPORT:PROMPT -NOLOGO -TLBID:1
    LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib
  else ifneq (,$(findstring uwp,$(PlatformSuffix)))
    WinPartition = uwp
    MSVC2017CompileFlags = -DWINAPI_FAMILY=WINAPI_FAMILY_APP -DWINDLL -D_UNICODE -DUNICODE -DWRL_NO_DEFAULT_LIB -FS
    LDFLAGS += -APPCONTAINER -NXCOMPAT -DYNAMICBASE -MANIFEST:NO -LTCG -OPT:REF -SUBSYSTEM:CONSOLE -MANIFESTUAC:NO -OPT:ICF -ERRORREPORT:PROMPT -NOLOGO -TLBID:1 -DEBUG:FULL -WINMD:NO
    LIBS += WindowsApp.lib
  endif

  CFLAGS += $(MSVC2017CompileFlags)
  CXXFLAGS += $(MSVC2017CompileFlags)

  TargetArchMoniker = $(subst $(WinPartition)_,,$(PlatformSuffix))

  CC  = cl.exe
  CXX = cl.exe

  reg_query = $(call filter_out2,$(subst $2,,$(shell reg query "$2" -v "$1" 2>nul)))
  fix_path = $(subst $(SPACE),\ ,$(subst \,/,$1))

  ProgramFiles86w := $(shell cmd /c "echo %PROGRAMFILES(x86)%")
  ProgramFiles86 := $(shell cygpath "$(ProgramFiles86w)")

  WindowsSdkDir ?= $(call reg_query,InstallationFolder,HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Microsoft SDKs\Windows\v10.0)
  WindowsSdkDir ?= $(call reg_query,InstallationFolder,HKEY_CURRENT_USER\SOFTWARE\Wow6432Node\Microsoft\Microsoft SDKs\Windows\v10.0)
  WindowsSdkDir ?= $(call reg_query,InstallationFolder,HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v10.0)
  WindowsSdkDir ?= $(call reg_query,InstallationFolder,HKEY_CURRENT_USER\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v10.0)
  WindowsSdkDir := $(WindowsSdkDir)

  WindowsSDKVersion ?= $(firstword $(foreach folder,$(subst $(subst \,/,$(WindowsSdkDir)Include/),,$(wildcard $(call fix_path,$(WindowsSdkDir)Include\*))),$(if $(wildcard $(call fix_path,$(WindowsSdkDir)Include/$(folder)/um/Windows.h)),$(folder),)))$(BACKSLASH)
  WindowsSDKVersion := $(WindowsSDKVersion)

  VsInstallBuildTools = $(ProgramFiles86)/Microsoft Visual Studio/2017/BuildTools
  VsInstallEnterprise = $(ProgramFiles86)/Microsoft Visual Studio/2017/Enterprise
  VsInstallProfessional = $(ProgramFiles86)/Microsoft Visual Studio/2017/Professional
  VsInstallCommunity = $(ProgramFiles86)/Microsoft Visual Studio/2017/Community

  VsInstallRoot ?= $(shell if [ -d "$(VsInstallBuildTools)" ]; then echo "$(VsInstallBuildTools)"; fi)
  ifeq ($(VsInstallRoot), )
    VsInstallRoot = $(shell if [ -d "$(VsInstallEnterprise)" ]; then echo "$(VsInstallEnterprise)"; fi)
  endif
  ifeq ($(VsInstallRoot), )
    VsInstallRoot = $(shell if [ -d "$(VsInstallProfessional)" ]; then echo "$(VsInstallProfessional)"; fi)
  endif
  ifeq ($(VsInstallRoot), )
    VsInstallRoot = $(shell if [ -d "$(VsInstallCommunity)" ]; then echo "$(VsInstallCommunity)"; fi)
  endif
  VsInstallRoot := $(VsInstallRoot)

  VcCompilerToolsVer := $(shell cat "$(VsInstallRoot)/VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt" | grep -o '[0-9\.]*')
  VcCompilerToolsDir := $(VsInstallRoot)/VC/Tools/MSVC/$(VcCompilerToolsVer)

  WindowsSDKSharedIncludeDir := $(shell cygpath -w "$(WindowsSdkDir)\Include\$(WindowsSDKVersion)\shared")
  WindowsSDKUCRTIncludeDir := $(shell cygpath -w "$(WindowsSdkDir)\Include\$(WindowsSDKVersion)\ucrt")
  WindowsSDKUMIncludeDir := $(shell cygpath -w "$(WindowsSdkDir)\Include\$(WindowsSDKVersion)\um")
  WindowsSDKUCRTLibDir := $(shell cygpath -w "$(WindowsSdkDir)\Lib\$(WindowsSDKVersion)\ucrt\$(TargetArchMoniker)")
  WindowsSDKUMLibDir := $(shell cygpath -w "$(WindowsSdkDir)\Lib\$(WindowsSDKVersion)\um\$(TargetArchMoniker)")

  # For some reason the HostX86 compiler doesn't like compiling for x64
  # ("no such file" opening a shared library), and vice-versa.
  # Work around it for now by using the strictly x86 compiler for x86, and x64 for x64.
  # NOTE: What about ARM?
  ifneq (,$(findstring x64,$(TargetArchMoniker)))
    VCCompilerToolsBinDir := $(VcCompilerToolsDir)\bin\HostX64
  else
    VCCompilerToolsBinDir := $(VcCompilerToolsDir)\bin\HostX86
  endif

  PATH := $(shell IFS=$$'\n'; cygpath "$(VCCompilerToolsBinDir)/$(TargetArchMoniker)"):$(PATH)
  PATH := $(PATH):$(shell IFS=$$'\n'; cygpath "$(VsInstallRoot)/Common7/IDE")
  INCLUDE := $(shell IFS=$$'\n'; cygpath -w "$(VcCompilerToolsDir)/include")
  LIB := $(shell IFS=$$'\n'; cygpath -w "$(VcCompilerToolsDir)/lib/$(TargetArchMoniker)")

  export INCLUDE := $(INCLUDE);$(WindowsSDKSharedIncludeDir);$(WindowsSDKUCRTIncludeDir);$(WindowsSDKUMIncludeDir)
  export LIB := $(LIB);$(WindowsSDKUCRTLibDir);$(WindowsSDKUMLibDir)
  TARGET := $(TARGET_NAME)_libretro.dll
  PSS_STYLE :=2
  LDFLAGS += -DLL
  LIBM :=
else
   CC ?= gcc
   CXX ?= g++
   TARGET := $(TARGET_NAME)_libretro.dll
   SHARED := -shared -s -Wl,--version-script=$(LIBRETRO_DIR)/link.T -Wl,--no-undefined
endif

LDFLAGS += $(LIBM)

ifeq ($(LTO),true)
   CFLAGS += -flto
   CXXFLAGS += -flto
   LD = g++
endif

ifeq ($(DEBUG), 1)
  ifneq (,$(findstring msvc,$(platform)))
    CFLAGS += -MTd -Od -Zi -DDEBUG -D_DEBUG
    CXXFLAGS += -MTd -Od -Zi -DDEBUG -D_DEBUG
  else
    CFLAGS += -O0 -g
    CXXFLAGS += -O0 -g
  endif
else
  ifneq (,$(findstring msvc,$(platform)))
    CFLAGS += -MT -O2 -DNDEBUG
    CXXFLAGS += -MT -O2 -DNDEBUG
  else
    CFLAGS += -O3
    CXXFLAGS += -O3
  endif
endif

ifneq (,$(findstring msvc,$(platform)))
  OBJOUT = -Fo
  LINKOUT = -out:
ifeq ($(STATIC_LINKING),1)
  LD ?= lib.exe
  STATIC_LINKING=0
else
  LD = link.exe
endif
else
  OBJOUT   = -o
  LINKOUT  = -o
  LD = $(CXX)
endif

include Makefile.common

OBJECTS := $(SOURCES_C:.c=.o) $(SOURCES_CXX:.cpp=.o)

ifeq (,$(findstring windows_msvc2017,$(platform)))
  CFLAGS   += -Wall
  CXXFLAGS += -Wall
endif

CFLAGS   += -D LIBRETRO $(fpic)
CXXFLAGS += -D LIBRETRO $(fpic) -std=c++14

ALTTARGET := $(subst mesen-s,mesens,$(TARGET))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "** BUILDING $(TARGET) FOR PLATFORM $(platform) **"
ifeq ($(STATIC_LINKING), 1)
ifneq (,$(findstring msvc,$(platform)))
	$(LD) $(LINKOUT)$@ $(OBJECTS)
else
	$(AR) rcs $@ $(OBJECTS)
endif
else
	$(LD) $(fpic) $(SHARED) $(INCLUDES) $(LINKOUT)$@ $(OBJECTS) $(LDFLAGS)
endif
	cp ./$(TARGET) ./$(ALTTARGET)
	@echo "** BUILD SUCCESSFUL! GG NO RE **"

%.o: %.c
	$(CC) $(CFLAGS) $(fpic) -c $< $(OBJOUT)$@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(fpic) -c $< $(OBJOUT)$@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean

print-%:
	@echo '$*=$($*)'

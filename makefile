#Welcome to what must be the most terrible makefile ever (but hey, it works)
#Both clang & gcc work fine - clang seems to output faster code
#.NET 6 (and its dev tools) must be installed to compile the UI.
#The emulation core also requires SDL2.
#Run "make" to build, "make run" to run

MESENFLAGS=

ifeq ($(USE_GCC),true)
	CXX := g++
	CC := gcc
	PROFILE_GEN_FLAG := -fprofile-generate
	PROFILE_USE_FLAG := -fprofile-use
else
	CXX := clang++
	CC := clang
	PROFILE_GEN_FLAG := -fprofile-instr-generate=$(CURDIR)/PGOHelper/pgo.profraw
	PROFILE_USE_FLAG := -fprofile-instr-use=$(CURDIR)/PGOHelper/pgo.profdata
endif

SDL2LIB := $(shell sdl2-config --libs)
SDL2INC := $(shell sdl2-config --cflags)

LINKCHECKUNRESOLVED := -Wl,-z,defs

LINKOPTIONS :=
MESENOS :=

# Use ?= so we can override these during testing
UNAME_S ?= $(shell uname -s)
MACHINE ?= $(shell uname -m)

ifneq ($(ARCH),)
	ifneq (,$(findstring x64,$(ARCH))$(findstring x86_64,$(ARCH)))
		override MACHINE := x86_64
	endif
	ifneq (,$(findstring arm64,$(ARCH))$(findstring aarch64,$(ARCH)))
		override MACHINE := aarch64
	endif
endif

ifeq ($(UNAME_S),Linux)
	MESENOS := linux
	SHAREDLIB := MesenCore.so
endif

ifeq ($(UNAME_S),Darwin)
	MESENOS := osx
	SHAREDLIB := MesenCore.dylib
	LTO := false
	STATICLINK := false
	LINKCHECKUNRESOLVED :=
endif

MESENFLAGS += -m64

ifeq ($(MACHINE),x86_64)
	MESENPLATFORM := $(MESENOS)-x64
endif
ifneq ($(filter %86,$(MACHINE)),)
	MESENPLATFORM := $(MESENOS)-x64
endif
ifneq ($(filter arm%,$(MACHINE)),)
	MESENPLATFORM := $(MESENOS)-arm64
endif
ifeq ($(MACHINE),aarch64)
	MESENPLATFORM := $(MESENOS)-arm64
	ifeq ($(USE_GCC),true)
		#don't set -m64 on arm64 for gcc (unrecognized option)
		MESENFLAGS := $(filter-out -m64,$(MESENFLAGS))
	endif
endif

DEBUG ?= 0

ifeq ($(DEBUG),0)
	MESENFLAGS += -O3
	ifneq ($(LTO),false)
		MESENFLAGS += -DHAVE_LTO
		ifneq ($(USE_GCC),true)
			MESENFLAGS += -flto=thin
		else
			MESENFLAGS += -flto=auto
		endif
	endif
else
	MESENFLAGS += -O0 -g
	# Note: if compiling with a sanitizer, you will likely need to `LD_PRELOAD` the library `libMesenCore.so` will be linked against.
	ifneq ($(SANITIZER),)
		ifeq ($(SANITIZER),address)
			# Currently, `-fsanitize=address` is not supported together with `-fsanitize=thread`
			MESENFLAGS += -fsanitize=address
		else ifeq ($(SANITIZER),thread)
			# Currently, `-fsanitize=address` is not supported together with `-fsanitize=thread`
			MESENFLAGS += -fsanitize=thread
		else
$(warning Unrecognised $$(SANITIZER) value: $(SANITIZER))
		endif
		# `-Wl,-z,defs` is incompatible with the sanitizers in a shared lib, unless the sanitizer libs are linked dynamically; hence `-shared-libsan` (not the default for Clang).
		# It seems impossible to link dynamically against two sanitizers at the same time, but that might be a Clang limitation.
		ifneq ($(USE_GCC),true)
			MESENFLAGS += -shared-libsan
		endif
	endif
endif

ifeq ($(PGO),profile)
	MESENFLAGS += ${PROFILE_GEN_FLAG}
endif

ifeq ($(PGO),optimize)
	MESENFLAGS += ${PROFILE_USE_FLAG}
endif

ifneq ($(STATICLINK),false)
	LINKOPTIONS += -static-libgcc -static-libstdc++ 
endif

ifeq ($(MESENOS),osx)
	LINKOPTIONS += -framework Foundation -framework Cocoa -framework GameController -framework CoreHaptics -Wl,-rpath,/opt/local/lib
endif

CXXFLAGS = -fPIC -Wall --std=c++17 $(MESENFLAGS) $(SDL2INC) -I $(realpath ./) -I $(realpath ./Core) -I $(realpath ./Utilities) -I $(realpath ./Sdl) -I $(realpath ./Linux) -I $(realpath ./MacOS)
OBJCXXFLAGS = $(CXXFLAGS)
CFLAGS = -fPIC -Wall $(MESENFLAGS)

OBJFOLDER := obj.$(MESENPLATFORM)
DEBUGFOLDER := bin/$(MESENPLATFORM)/Debug
RELEASEFOLDER := bin/$(MESENPLATFORM)/Release
ifeq ($(DEBUG), 0)
	OUTFOLDER = $(RELEASEFOLDER)
	BUILD_TYPE := Release
	OPTIMIZEUI := -p:OptimizeUi=true
else
	OUTFOLDER = $(DEBUGFOLDER)
	BUILD_TYPE := Debug
	OPTIMIZEUI :=
endif


ifeq ($(USE_AOT),true)
	PUBLISHFLAGS ?=  -r $(MESENPLATFORM) -p:PublishSingleFile=false -p:PublishAot=true -p:SelfContained=true
else
	PUBLISHFLAGS ?=  -r $(MESENPLATFORM) --no-self-contained -p:PublishSingleFile=true
endif


CORESRC := $(shell find Core -name '*.cpp')
COREOBJ := $(CORESRC:.cpp=.o)

UTILSRC := $(shell find Utilities -name '*.cpp' -o -name '*.c')
UTILOBJ := $(addsuffix .o,$(basename $(UTILSRC)))

SDLSRC := $(shell find Sdl -name '*.cpp')
SDLOBJ := $(SDLSRC:.cpp=.o)

SEVENZIPSRC := $(shell find SevenZip -name '*.c')
SEVENZIPOBJ := $(SEVENZIPSRC:.c=.o)

LUASRC := $(shell find Lua -name '*.c')
LUAOBJ := $(LUASRC:.c=.o)

ifeq ($(MESENOS),linux)
	LINUXSRC := $(shell find Linux -name '*.cpp')
else
	LINUXSRC :=
endif
LINUXOBJ := $(LINUXSRC:.cpp=.o)

ifeq ($(MESENOS),osx)
	MACOSSRC := $(shell find MacOS -name '*.mm')
else
	MACOSSRC :=
endif
MACOSOBJ := $(MACOSSRC:.mm=.o)

DLLSRC := $(shell find InteropDLL -name '*.cpp')
DLLOBJ := $(DLLSRC:.cpp=.o)

ifeq ($(SYSTEM_LIBEVDEV), true)
	LIBEVDEVLIB := $(shell pkg-config --libs libevdev)
	LIBEVDEVINC := $(shell pkg-config --cflags libevdev)
else
	LIBEVDEVSRC := $(shell find Linux/libevdev -name '*.c')
	LIBEVDEVOBJ := $(LIBEVDEVSRC:.c=.o)
	LIBEVDEVINC := -I../
endif

ifeq ($(MESENOS),linux)
	X11LIB := -lX11
else
	X11LIB :=
endif

FSLIB := -lstdc++fs

ifeq ($(MESENOS),osx)
	LIBEVDEVOBJ := 
	LIBEVDEVINC := 
	LIBEVDEVSRC := 
	FSLIB := 
	ifeq ($(USE_AOT),true)
		PUBLISHFLAGS := -t:BundleApp -p:UseAppHost=true -p:RuntimeIdentifier=$(MESENPLATFORM) -p:PublishSingleFile=false -p:PublishAot=true -p:SelfContained=true
	else
		PUBLISHFLAGS := -t:BundleApp -p:UseAppHost=true -p:RuntimeIdentifier=$(MESENPLATFORM) -p:SelfContained=true -p:PublishSingleFile=false -p:PublishReadyToRun=false
	endif
endif

all: ui

ui: InteropDLL/$(OBJFOLDER)/$(SHAREDLIB)
	mkdir -p $(OUTFOLDER)/Dependencies
	rm -fr $(OUTFOLDER)/Dependencies/*
	cp InteropDLL/$(OBJFOLDER)/$(SHAREDLIB) $(OUTFOLDER)/$(SHAREDLIB)
	chmod +x UI/prebuild_linux_mac.sh
	#Called twice because the first call copies native libraries to the bin folder which need to be included in Dependencies.zip
	#Don't run with AOT flags the first time to reduce build duration
	dotnet publish UI/UI.csproj -c $(BUILD_TYPE) $(OPTIMIZEUI) -r $(MESENPLATFORM)
	dotnet publish UI/UI.csproj -c $(BUILD_TYPE) $(OPTIMIZEUI) $(PUBLISHFLAGS)

core: InteropDLL/$(OBJFOLDER)/$(SHAREDLIB)

pgohelper: InteropDLL/$(OBJFOLDER)/$(SHAREDLIB)
	mkdir -p PGOHelper/$(OBJFOLDER) && cd PGOHelper/$(OBJFOLDER) && $(CXX) $(CXXFLAGS) $(LINKCHECKUNRESOLVED) -o pgohelper ../PGOHelper.cpp ../../bin/pgohelperlib.so -pthread $(FSLIB) $(SDL2LIB) $(LIBEVDEVLIB) $(X11LIB)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.mm
	$(CXX) $(OBJCXXFLAGS) -c $< -o $@

InteropDLL/$(OBJFOLDER)/$(SHAREDLIB): $(SEVENZIPOBJ) $(LUAOBJ) $(UTILOBJ) $(COREOBJ) $(SDLOBJ) $(LIBEVDEVOBJ) $(LINUXOBJ) $(DLLOBJ) $(MACOSOBJ)
	mkdir -p bin
	mkdir -p InteropDLL/$(OBJFOLDER)
	$(CXX) $(CXXFLAGS) $(LINKOPTIONS) $(LINKCHECKUNRESOLVED) -shared -o $(SHAREDLIB) $(DLLOBJ) $(SEVENZIPOBJ) $(LUAOBJ) $(LINUXOBJ) $(MACOSOBJ) $(LIBEVDEVOBJ) $(UTILOBJ) $(SDLOBJ) $(COREOBJ) $(SDL2INC) -pthread $(FSLIB) $(SDL2LIB) $(LIBEVDEVLIB) $(X11LIB)
	cp $(SHAREDLIB) bin/pgohelperlib.so
	mv $(SHAREDLIB) InteropDLL/$(OBJFOLDER)

pgo:
	./buildPGO.sh

run:
	$(OUTFOLDER)/$(MESENPLATFORM)/publish/Mesen

clean:
	rm -r -f $(COREOBJ)
	rm -r -f $(UTILOBJ)
	rm -r -f $(LINUXOBJ) $(LIBEVDEVOBJ)
	rm -r -f $(SDLOBJ)
	rm -r -f $(SEVENZIPOBJ)
	rm -r -f $(LUAOBJ)
	rm -r -f $(MACOSOBJ)
	rm -r -f $(DLLOBJ)


# --- Environment Reporting & Validation ---

# Shared format for the table rows
PRINT_FORMAT := "%-15s | %-13s %-11s | %-13s %-11s | %-10s\n"

.PHONY: verify-env verify-all-env test-env-row

# Hide the "make[1]: Entering directory..." noise
.SILENT: test-env-row

# Target for CI: verify the current machine's environment
verify-env:
	@echo "Checking Build Environment..."
	@printf $(PRINT_FORMAT) "INPUT" "MATRIX_REQ" "" "ACTUAL" "" "RESULT"
	# compare what the matrix requested (REQ_PLAT) vs what the Makefile found (MESENPLATFORM)
	@$(MAKE) --no-print-directory test-env-row \
		E_PLAT="$(if $(REQ_PLAT),$(REQ_PLAT),$(MESENPLATFORM))" \
		E_FLAGS="$(filter -m64,$(MESENFLAGS))"

verify-all-env:
	@rm -f .test_failed
	@echo "Validating Makefile Architecture Detection Logic:"
	@echo "----------------------------------------------------------------------------------------------------------"
	@printf $(PRINT_FORMAT) "INPUT" "EXPECTED (PLAT/FLAGS)" "" "ACTUAL (PLAT/FLAGS)" "" "RESULT"
	@echo "----------------------------------------------------------------------------------------------------------"
	@# --- TEST GROUP 1: Auto-Detection (No ARCH param) ---
	@# These prove the Makefile works out-of-the-box on different hardware
	@$(MAKE) --no-print-directory test-env-row UNAME_S=Linux  MACHINE=x86_64     E_PLAT=linux-x64   E_FLAGS="-m64" || touch .test_failed
	@$(MAKE) --no-print-directory test-env-row UNAME_S=Linux  MACHINE=aarch64    E_PLAT=linux-arm64 E_FLAGS=""      USE_GCC=true || touch .test_failed
	@$(MAKE) --no-print-directory test-env-row UNAME_S=Darwin MACHINE=x86_64     E_PLAT=osx-x64     E_FLAGS="-m64" || touch .test_failed
	@$(MAKE) --no-print-directory test-env-row UNAME_S=Darwin MACHINE=arm64      E_PLAT=osx-arm64   E_FLAGS="-m64" || touch .test_failed

	@# --- TEST GROUP 2: Explicit Overrides (With ARCH param) ---
	@# These prove the Makefile correctly ignores the hardware when told to
	@# Case: On x86_64 hardware, but ARCH says arm64
	@$(MAKE) --no-print-directory test-env-row UNAME_S=Linux  MACHINE=x86_64     E_PLAT=linux-arm64 E_FLAGS=""     ARCH=arm64 USE_GCC=true || touch .test_failed

	@# Case: On ARM64 hardware, but ARCH says x64
	@$(MAKE) --no-print-directory test-env-row UNAME_S=Darwin MACHINE=arm64      E_PLAT=osx-x64     E_FLAGS="-m64" ARCH=x64 || touch .test_failed

	@# Case: Using alternate naming (aarch64) in the ARCH param
	@$(MAKE) --no-print-directory test-env-row UNAME_S=Linux  MACHINE=x86_64     E_PLAT=linux-arm64 E_FLAGS=""     ARCH=aarch64 USE_GCC=true || touch .test_failed

	@# --- TEST GROUP 3: Resilient Naming (Fuzzy ARCH matching) ---
	@# Test: ARCH contains the full platform name (common in CI)
	@$(MAKE) --no-print-directory test-env-row UNAME_S=Darwin MACHINE=arm64  ARCH=osx-x64   E_PLAT=osx-x64     E_FLAGS="-m64" || touch .test_failed

	@# Test: ARCH contains extra spaces or different casing (handled by findstring)
	@$(MAKE) --no-print-directory test-env-row UNAME_S=Linux  MACHINE=x86_64 ARCH=linux-arm64 E_PLAT=linux-arm64 E_FLAGS="" USE_GCC=true || touch .test_failed
	@echo "----------------------------------------------------------------------------------------------------------"
	@if [ -f .test_failed ]; then \
		rm .test_failed; \
		echo "Verification FAILED!"; \
		exit 1; \
	else \
		echo "Verification Complete. All tests PASSED."; \
	fi

test-env-row:
	$(eval ACTUAL_FLAGS := $(strip $(filter -m64,$(MESENFLAGS))))
	$(eval EXP_FLAGS := $(strip $(E_FLAGS)))
	$(eval PLAT_MATCH := $(if $(filter $(E_PLAT),$(MESENPLATFORM)),OK,FAIL))
	$(eval FLAG_MATCH := $(if $(subst $(EXP_FLAGS),,$(ACTUAL_FLAGS))$(subst $(ACTUAL_FLAGS),,$(EXP_FLAGS)),FAIL,OK))
	$(eval PASS := $(if $(filter OKOK,$(PLAT_MATCH)$(FLAG_MATCH)),PASS,FAIL))
	@printf $(PRINT_FORMAT) \
		"$(UNAME_S)-$(MACHINE)" \
		"$(E_PLAT)" "$(EXP_FLAGS)" \
		"$(MESENPLATFORM)" "$(ACTUAL_FLAGS)" \
		"$(PASS)"
	@# Return a non-zero exit code ONLY so verify-all-env can catch it with ||
	@if [ "$(PASS)" = "FAIL" ]; then exit 1; fi


#!/bin/sh

# This build is used to compile an instrumented version of the binary, run it, and then optimize the binary with the profiling data.
# LTO is also enabled by default by this script.
# On clang, this results in a +60% speed boost compared to using just "make" (w/o LTO)
#
# Rom files must be copied to the PGOHelper/PGOGames folder beforehand - all *.nes files in that folder will be executed as part of the profiling process.
# Using a variety of roms is recommended (e.g different mappers, etc.)
#
# You can run this script via make:
#   For clang, run "make pgo" 
#   For GCC, run "USE_GCC=true make pgo"
#
#  Note: While GCC runs through this script just fine, the runtime performance is pretty terrible (something must be wrong with the way this is built)
#
# This will produce the following binary: bin/x64/Release/Mesen.exe
if [ "$MESENPLATFORM" = x86 ]; then
	PLAT="x86"
else
	PLAT="x64"
fi

if [ "$BUILDTARGET" = libretro ]; then
	TARG="libretro"
else
	TARG="core"
fi

OBJ="PGOHelper/obj.${PLAT}/"
FLAGS="LTO=true MESENPLATFORM=${PLAT}"

eval ${FLAGS} make clean

#create instrumented binary
eval ${FLAGS} PGO=profile make ${TARG} -j 16
eval ${FLAGS} PGO=profile make pgohelper -B
eval cp bin/pgohelperlib.so ${OBJ}

#run the instrumented binary
cd ${OBJ}
./pgohelper
cd ..

if [ "$USE_GCC" != true ]; then
	#clang-specific steps to convert the profiling data and clean the files
	llvm-profdata merge -output=pgo.profdata pgo.profraw
	cd ..
	eval ${FLAGS} make clean
else
	cd ..
fi

if [ "$BUILDTARGET" = "" ]; then
	TARG=""
fi

#rebuild using the profiling data to optimize
eval ${FLAGS} PGO=optimize make ${TARG} -j 16 -B

if [ "$USE_GCC" != true ]; then
	rm PGOHelper/pgo.profdata
	rm PGOHelper/pgo.profraw
else
	rm ./*.gcda
fi


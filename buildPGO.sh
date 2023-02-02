#!/bin/sh

# This build produces a build optimized via profiling (i.e PGO)
#
# ROM files must be copied to the PGOHelper/PGOGames folder beforehand - all supported rom files in that folder will be executed as part of the profiling process.
# Using a variety of roms is recommended (e.g different consoles/mappers, etc.)
#
# This will produce the following binary: bin/linux-x64/Release/linux-x64/publish/Mesen
PLAT="x64"
TARG="core"

OBJ="PGOHelper/obj.${PLAT}/"

eval make clean

#create instrumented binary
eval PGO=profile make pgohelper -B -j 16
eval cp bin/pgohelperlib.so ${OBJ}

#run the instrumented binary
cd ${OBJ}
./pgohelper
cd ..

if [ "$USE_GCC" != true ]; then
	#clang-specific steps to convert the profiling data and clean the files
	llvm-profdata merge -output=pgo.profdata pgo.profraw
	cd ..
	eval make clean
else
	cd ..
fi

#rebuild using the profiling data to optimize
eval PGO=optimize make -j 16 -B

if [ "$USE_GCC" != true ]; then
	rm PGOHelper/pgo.profdata
	rm PGOHelper/pgo.profraw
else
	rm ./*.gcda
fi


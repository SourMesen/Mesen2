This folder is used when compiling Mesen-S with PGO.
All .nes files put in this folder will be folder will be run in console mode for a few seconds as a way of building of a profile for use with PGO.

Once you have added a few roms to this folder, run "make pgo" to produce a PGO-optimized binary (it will take several minutes to build)

Another folder, called "PGOMesenHome" will be created alongside this one when the instrumented executable runs when executing the PGO script.  
This folder will be used as a temporary home folder location for Mesen-S to store the files it creates in the process.
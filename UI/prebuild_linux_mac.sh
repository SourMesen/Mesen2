#!/bin/bash

set -e # Exit immediately if a command fails

PLATFORM=$1 # 'linux' or 'osx'
OUTDIR="${2%/}" # remove trailing slash if present
PROJECTDIR="${3%/}"
EXT="so"
if [ "$PLATFORM" == "osx" ]; then EXT="dylib"; fi

echo "Packaging dependencies for $PLATFORM..."

mkdir -p "$OUTDIR"
cd "$OUTDIR"
rm -rf Dependencies
mkdir -p Dependencies

# 1. Copy static assets
if [ -d "$PROJECTDIR/Dependencies" ]; then
    cp -R "$PROJECTDIR/Dependencies/." Dependencies/
fi

# 2. Copy native libs (with a fallback check)
# NuGet puts these in runtimes/[rid]/native/ - we look there if not in root
LIBS=("libHarfBuzzSharp.$EXT" "libSkiaSharp.$EXT" "MesenCore.$EXT")

for LIB in "${LIBS[@]}"; do
    FOUND_LIB=$(find . -name "$LIB" -print -quit)
    if [ -n "$FOUND_LIB" ]; then
        cp "$FOUND_LIB" Dependencies/
    fi
done

# 3. Zip it up
cd Dependencies
rm -f ../Dependencies.zip
zip -r ../Dependencies.zip .

mv ../Dependencies.zip "$PROJECTDIR/"

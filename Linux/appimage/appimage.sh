#!/bin/bash

export PUBLISHFLAGS="-r linux-x64 --no-self-contained false -p:PublishSingleFile=true -p:PublishReadyToRun=true"
make -j$(nproc) -O LTO=true STATICLINK=true SYSTEM_LIBEVDEV=false

curl -SL https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage -o appimagetool

mkdir -p AppDir/usr/bin
cp bin/linux-x64/Release/linux-x64/publish/Mesen AppDir/usr/bin
chmod +x AppDir/usr/bin
ln -sr AppDir/usr/bin/Mesen AppDir/AppRun

cp Linux/appimage/Mesen.48x48.png AppDir/Mesen.png
cp Linux/appimage/Mesen.desktop AppDir/Mesen.desktop
mkdir -p AppDir/usr/share/applications && cp ./AppDir/Mesen.desktop ./AppDir/usr/share/applications
mkdir -p AppDir/usr/share/icons && cp ./AppDir/Mesen.png ./AppDir/usr/share/icons
mkdir -p AppDir/usr/share/icons/hicolor/48x48/apps && cp ./AppDir/Mesen.png ./AppDir/usr/share/icons/hicolor/48x48/apps

chmod a+x appimagetool
./appimagetool AppDir/ Mesen.AppImage

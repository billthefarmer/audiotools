#
#  Makefile
#  LMS
#
#  Created by Bill Farmer on 03/07/2018.
#  Copyright © 2018 Bill Farmer. All rights reserved.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.


RM = rm
MD = mkdir
CP = cp
LN = ln
CC = gcc

FOR = for
IN = in
DO = do
DONE = done

HDIU = hdiutil
SWIFTC = swiftc
PLB = /usr/libexec/PlistBuddy
LIPO = lipo
SIPS = sips

AUDIO = Audio
AUDIOX86OBJ = $(AUDIO).x86.o
AUDIOARMOBJ = $(AUDIO).arm.o

APPNAME = LMS
APPNAMEX86 = $(APPNAME)_x86
APPNAMEARM = $(APPNAME)_arm

APPVERSION = $(shell $(PLB) -c 'print :CFBundleShortVersionString' Info.plist)

APPBUNDLE = $(APPNAME).app
APPBUNDLECONTENTS = $(APPBUNDLE)/Contents
APPBUNDLEMACOS = $(APPBUNDLECONTENTS)/MacOS
APPBUNDLERESOURCES = $(APPBUNDLECONTENTS)/Resources
APPBUNDLEICON = $(APPBUNDLECONTENTS)/Resources

APPDMG = $(APPNAME).dmg
APPVDMG = $(APPNAME)-$(APPVERSION).dmg

HEADER = LMS-Bridging-Header.h
SWIFTH = LMS-Swift.h

SOURCES = AppDelegate.swift DisplayView.swift MeterView.swift \
		SpectrumView.swift LMSView.swift main.swift

LIBRARIES = libswiftAccelerate.dylib libswiftCoreGraphics.dylib \
	libswiftMetal.dylib libswiftAppKit.dylib libswiftCoreImage.dylib \
	libswiftObjectiveC.dylib libswiftCore.dylib libswiftDarwin.dylib \
	libswiftos.dylib libswiftCoreAudio.dylib libswiftDispatch.dylib \
	libswiftQuartzCore.dylib libswiftCoreData.dylib \
	libswiftFoundation.dylib libswiftSwiftOnoneSupport.dylib \
	libswiftCoreFoundation.dylib libswiftIOKit.dylib libswiftXPC.dylib

XCODELIB = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/swift-5.0/macosx/

FRAMEWORKS = Frameworks

SFLAGSX86 = -target x86_64-apple-macos10.12 \
	-import-objc-header $(HEADER) -emit-objc-header-path $(SWIFTH) \
	-Xlinker -rpath -Xlinker @loader_path/../Frameworks

SFLAGSARM = -target arm64-apple-macos11 \
	-import-objc-header $(HEADER) -emit-objc-header-path $(SWIFTH) \
	-Xlinker -rpath -Xlinker @loader_path/../Frameworks

CFLAGSX86 = -target x86_64-apple-macos10.12

CFLAGSARM = -target arm64-apple-macos11

$(APPBUNDLE): $(APPNAME) $(APPNAME).icns Info.plist Frameworks
	$(RM) -rf $(APPBUNDLE)
	$(MD) $(APPBUNDLE)
	$(MD) $(APPBUNDLECONTENTS)
	$(MD) $(APPBUNDLEMACOS)
	$(MD) $(APPBUNDLERESOURCES)
	$(CP) Info.plist $(APPBUNDLECONTENTS)/
	$(CP) PkgInfo $(APPBUNDLECONTENTS)/
	$(CP) -r Frameworks $(APPBUNDLECONTENTS)/
	$(CP) $(APPNAME).icns $(APPBUNDLERESOURCES)/
	$(CP) $(APPNAME).entitlements $(APPBUNDLERESOURCES)/
	$(CP) $(APPNAME) $(APPBUNDLEMACOS)/

$(FRAMEWORKS): $(XCODELIB)
	$(FOR) file $(IN) $(LIBRARIES); \
	$(DO) $(CP) $(XCODELIB)$$file $(FRAMEWORKS); \
	$(DONE);

$(APPNAME): $(APPNAMEX86) $(APPNAMEARM)
	$(LIPO) -create -output $(APPNAME) $(APPNAMEX86) $(APPNAMEARM)

$(APPNAMEX86): $(SOURCES) $(AUDIOX86OBJ)
	$(SWIFTC) $(SOURCES) $(AUDIOX86OBJ) -o $(APPNAMEX86) $(SFLAGSX86)

$(APPNAMEARM): $(SOURCES) $(AUDIOARMOBJ)
	$(SWIFTC) $(SOURCES) $(AUDIOARMOBJ) -o $(APPNAMEARM) $(SFLAGSARM)

$(AUDIOX86OBJ):	$(AUDIO).m $(AUDIO).h Makefile
	$(CC) -c $(CFLAGSX86) $(AUDIO).m -o $(AUDIOX86OBJ)

$(AUDIOARMOBJ):	$(AUDIO).m $(AUDIO).h Makefile
	$(CC) -c $(CFLAGSARM) $(AUDIO).m -o $(AUDIOARMOBJ)

$(APPNAME).icns: $(APPNAME).png
	$(RM) -rf $(APPNAME).iconset
	$(MD) $(APPNAME).iconset
	$(SIPS) -z 16 16   $(APPNAME).png --out $(APPNAME).iconset/icon_16x16.png
	$(SIPS) -z 32 32   $(APPNAME).png --out $(APPNAME).iconset/icon_16x16@2x.png
	$(SIPS) -z 32 32   $(APPNAME).png --out $(APPNAME).iconset/icon_32x32.png
	$(SIPS) -z 64 64   $(APPNAME).png --out $(APPNAME).iconset/icon_32x32@2x.png
	$(SIPS) -z 128 128 $(APPNAME).png --out $(APPNAME).iconset/icon_128x128.png
	$(SIPS) -z 256 256 $(APPNAME).png --out $(APPNAME).iconset/icon_128x128@2x.png
	$(SIPS) -z 256 256 $(APPNAME).png --out $(APPNAME).iconset/icon_256x256.png
	$(SIPS) -z 512 512 $(APPNAME).png --out $(APPNAME).iconset/icon_256x256@2x.png
	$(SIPS) -z 512 512 $(APPNAME).png --out $(APPNAME).iconset/icon_512x512.png
	$(CP) $(APPNAME).png $(APPNAME).iconset/icon_512x512@2x.png
	iconutil -c icns -o $(APPNAME).icns $(APPNAME).iconset
	$(RM) -r $(APPNAME).iconset

clean:
	$(RM) -rf $(APPBUNDLE)
	$(RM) $(APPNAME).icns
	$(RM) $(APPNAME)
	$(RM) Audio.o
	$(RM) *.dmg

dmg:	$(APPVDMG)

$(APPVDMG): $(APPBUNDLE)
	$(HDIU) create -size 16m -fs HFS+ -volname $(APPNAME) -ov $(APPNAME) -quiet
	$(HDIU) attach $(APPDMG) -quiet
	$(CP) -r $(APPBUNDLE) /Volumes/$(APPNAME)
	$(LN) -s /Applications /Volumes/$(APPNAME)/Applications
	$(CP) README.md /Volumes/$(APPNAME)
	$(HDIU) detach /Volumes/$(APPNAME) -quiet
	$(HDIU) convert -format UDZO -o $(APPVDMG) -ov $(APPDMG) -quiet

.PHONY:	$(FRAMEWORKS)

# Copyright 2023-2025, Mansour Moufid <mansourmoufid@gmail.com>

AR?=            ar
AS?=            clang
CC?=            clang
CXX?=           clang++
CPP?=           $(CC) -E
LD?=            $(CC)
RANLIB?=        ranlib

ifeq ("$(TARGET)","")
TARGET:=        $(shell $(CC) $(CFLAGS) -dumpmachine | sed -e 's/[0-9.]*$$//')
endif
ARCH:=          $(shell echo "$(TARGET)" | awk -F- '{print $$1}')
VEND:=          $(shell echo "$(TARGET)" | awk -F- '{print $$2}')
SYS:=           $(shell echo "$(TARGET)" | awk -F- '{print $$3}')
ASFLAGS+=       --target=$(TARGET)
CFLAGS+=        --target=$(TARGET)
CFLAGS+=        -mtune=generic

ASFLAGS+=       -fintegrated-as
CPPFLAGS+=      -D_POSIX_C_SOURCE=200809L
CPPFLAGS+=      -D_FORTIFY_SOURCE=1
CFLAGS+=        -std=c11
CFLAGS+=        -fno-builtin
CFLAGS+=        -fno-common
CFLAGS+=        -fno-delete-null-pointer-checks
CFLAGS+=        -fno-strict-aliasing
CFLAGS+=        -fno-strict-overflow
CFLAGS+=        -fpic
CFLAGS+=        -fwrapv
# LDFLAGS+=       -fuse-ld=lld

ifeq ("$(DEBUG)","0")
CPPFLAGS+=      -DNDEBUG
CFLAGS+=        -Os
LDFLAGS+=       -Wl,-S
else
CFLAGS+=        -O0
CFLAGS+=        -g
LDFLAGS+=       -O0
LDFLAGS+=       -g
endif

CFLAGS+=        -Wall
CFLAGS+=        -Wno-atomic-implicit-seq-cst
CFLAGS+=        -Wno-declaration-after-statement
CFLAGS+=        -Wno-padded
CFLAGS+=        -Wno-unused-command-line-argument
# LDFLAGS+=       -Wno-unused-command-line-argument
CFLAGS+=        -Wno-unused-function
CFLAGS+=        -pedantic
ifeq ("$(DEBUG)","0")
CFLAGS+=        -Wno-unused-label
CFLAGS+=        -Wno-unused-macros
CFLAGS+=        -Wno-unused-parameter
CFLAGS+=        -Wno-unused-variable
else
CFLAGS+=        -Weverything
endif

ifeq ("$(ARCH)","x86_64")
CFLAGS+=        -mfpmath=sse
CFLAGS+=        -msse4.2
endif

ifeq ("$(ARCH)","armv7")
CFLAGS+=        -mfpu=neon
endif

ifeq ("$(VEND)","apple")
LDFLAGS+=       -Wl,-bind_at_load
LDFLAGS+=       -Wl,-dead_strip_dylibs
LDFLAGS+=       -Wl,-dynamic
LDFLAGS+=       -Wl,-headerpad_max_install_names
LDFLAGS+=       -Wl,-no_implicit_dylibs
endif

ifeq ("$(SYS)","darwin")
SDKROOT:=       "$(shell xcrun --sdk macosx --show-sdk-path)"
AR:=            "$(shell xcrun --sdk macosx --find $(AR))"
CC:=            "$(shell xcrun --sdk macosx --find $(CC))"
LD:=            "$(shell xcrun --sdk macosx --find $(CC))"
CPPFLAGS+=      -isysroot $(SDKROOT)
CFLAGS+=        --sysroot=$(SDKROOT)
LDFLAGS+=       --sysroot=$(SDKROOT)
ifeq ("$(ARCH)","arm64")
CFLAGS+=        -mmacosx-version-min=11.0
LDFLAGS+=       -Wl,-macos_version_min,11.0
else
CFLAGS+=        -mmacosx-version-min=10.12
LDFLAGS+=       -Wl,-macos_version_min,10.12
endif
LDFLAGS+=       --target=$(TARGET)
SOEXT:=         .dylib
endif

ifeq ("$(SYS)","ios")
SDKROOT:=       "$(shell xcrun --sdk iphoneos --show-sdk-path)"
AR:=            "$(shell xcrun --sdk iphoneos --find $(AR))"
CC:=            "$(shell xcrun --sdk iphoneos --find $(CC))"
LD:=            "$(shell xcrun --sdk iphoneos --find $(CC))"
CPPFLAGS+=      -isysroot $(SDKROOT)
CFLAGS+=        --sysroot=$(SDKROOT)
LDFLAGS+=       --sysroot=$(SDKROOT)
CFLAGS+=        -miphoneos-version-min=10.0
LDFLAGS+=       -Wl,-ios_version_min,10.0
LDFLAGS+=       --target=$(TARGET)
SOEXT:=         .dylib
endif

SOEXT?=         .so

ifeq ("$(VEND)","apple")
LDFLAGS+=       -framework Accelerate
LDFLAGS+=       -framework AVFoundation
LDFLAGS+=       -framework CoreFoundation
LDFLAGS+=       -framework CoreMedia
LDFLAGS+=       -framework CoreVideo
LDFLAGS+=       -framework Foundation
LDFLAGS+=       -framework IOSurface
LDFLAGS+=       -framework VideoToolbox
LDFLAGS+=       -lobjc
LDFLAGS+=       -Wl,-install_name,libal.dylib
endif
ifeq ("$(SYS)","ios")
LDFLAGS+=       -framework UIKit
endif

ifeq ("$(SYS)","android")
ANDROID:=       true
endif
ifeq ("$(SYS)","androideabi")
ANDROID:=       true
endif
ifeq ("$(ANDROID)","true")
LDFLAGS+=       -landroid -llog -lcamera2ndk -lm -lmediandk
endif

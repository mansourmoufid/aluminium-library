# Copyright 2023-2025, Mansour Moufid <mansourmoufid@gmail.com>

MAKEFILE:=  $(firstword $(MAKEFILE_LIST))
MAKEDIR:=   $(shell cd $(dir $(MAKEFILE)) && pwd)

PLATFORM:=	darwin
TARGETS:= \
	x86_64-apple-darwin \
	arm64-apple-darwin \
	armv7-apple-ios \
	arm64-apple-ios
# TARGETS:= arm64-apple-darwin

CPPFLAGS:=	-D_DARWIN_C_SOURCE=1
LDFLAGS:=
LDFLAGS+=	-framework Accelerate
LDFLAGS+=	-framework AVFoundation
LDFLAGS+=	-framework CoreFoundation
LDFLAGS+=	-framework CoreMedia
LDFLAGS+=	-framework CoreVideo
LDFLAGS+=	-framework Foundation
LDFLAGS+=	-framework IOSurface
LDFLAGS+=	-framework VideoToolbox
LDFLAGS+=	-lobjc
LDFLAGS+=	-Wl,-install_name,libal.dylib

SOEXT:=		.dylib

ENV:= \
	CPPFLAGS="$(CPPFLAGS)" \
	LDFLAGS="$(LDFLAGS)" \
	PLATFORM="$(PLATFORM)" \
	SOEXT="$(SOEXT)"

.PHONY: so
so:
	for target in $(TARGETS); do \
		env $(ENV) TARGET="$$target" $(MAKE) so || exit 1; \
	done

.PHONY: install-so
install-so:
	mkdir -p $(DESTDIR)$(PREFIX)/$(LIBDIR)
	install build/$(TARGET)/*$(SOEXT) $(DESTDIR)$(PREFIX)/$(LIBDIR)

.PHONY: install
install: install-so

.PHONY: cleanup
cleanup:
	for target in $(TARGETS); do \
		rm -f build/$$target/*.o; \
	done

.PHONY: clean
clean: cleanup
	for target in $(TARGETS); do \
		rm -f build/$$target/*$(SOEXT); \
	done
	dot_clean .
	find . -name .DS_Store | xargs rm -f

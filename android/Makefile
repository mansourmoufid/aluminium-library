# Copyright 2023-2025, Mansour Moufid <mansourmoufid@gmail.com>

MAKEFILE:=  $(firstword $(MAKEFILE_LIST))
MAKEDIR:=   $(shell cd $(dir $(MAKEFILE)) && pwd)

ABIS:=		armeabi-v7a arm64-v8a
API:=		24
LIBS:=		android log camera2ndk mediandk
PLATFORM:=	android

ifeq ("$(ABI)","armeabi-v7a")
TARGET:=	arm-linux-androideabi
endif
ifeq ("$(ABI)","arm64-v8a")
TARGET:=	aarch64-linux-android
endif

LDFLAGS:=
# LDFLAGS:=	-landroid -llog -lcamera2ndk -lm -lmediandk

ENV:=	\
	API="$(API)" \
	LDFLAGS="$(LDFLAGS)" \
	PLATFORM="$(PLATFORM)" \
	TARGET="$(TARGET)"

.PHONY: so
so:
	for abi in $(ABIS); do \
		env $(ENV) ABI="$$abi" python -m androidenv $(MAKE) so || exit 1; \
	done

.PHONY: install-so
install-so: so
	mkdir -p $(DESTDIR)$(PREFIX)/lib/$(ABI)
	install $(MAKEDIR)/../build/$(TARGET)/*.so $(DESTDIR)$(PREFIX)/lib/$(ABI)/

.PHONY: install
install:
	for abi in $(ABIS); do \
		ABI=$$abi $(MAKE) -f $(MAKEFILE) install-so; \
	done
	for abi in $(ABIS); do \
		ABI=$$abi API=$(API) python -m androidenv --find-library $(LIBS) \
		| while read f; do \
			install "$$f" "$(DESTDIR)$(PREFIX)/lib/$$abi/"; \
		done; \
	done

.PHONY: cleanup
cleanup:
	rm -f $(MAKEDIR)/../build/*-*-android*/*.o

.PHONY: clean
clean: cleanup
	rm -f $(MAKEDIR)/../build/*-*-android*/*.so
	dot_clean .
	find . -name .DS_Store | xargs rm -f

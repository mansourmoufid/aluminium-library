# Copyright 2023-2025, Mansour Moufid <mansourmoufid@gmail.com>

CPPFLAGS+=	-I.

ifeq ("$(PLATFORM)","")
PLATFORM:=	$(shell uname -s | tr '[A-Z]' '[a-z]')
endif
CPPFLAGS+=	-I$(PLATFORM)

include build.mk

ifneq ("$(TRIAL)","")
CPPFLAGS+=	-DTRIAL=1
endif

PYTHONPATH:=		$(shell pwd):$(PYTHONPATH)
ifeq ($(shell uname -s),Darwin)
DYLD_LIBRARY_PATH:=	$(shell pwd)/build/$(TARGET):$(DYLD_LIBRARY_PATH)
PYTHON:=			DYLD_LIBRARY_PATH="$(DYLD_LIBRARY_PATH)" \
					PYTHONPATH="$(PYTHONPATH)" python
else
LD_LIBRARY_PATH:=	$(shell pwd)/build/$(TARGET):$(LD_LIBRARY_PATH)
PYTHON:=			LD_LIBRARY_PATH="$(LD_LIBRARY_PATH)" \
					PYTHONPATH="$(PYTHONPATH)" python
endif

ENV:=	\
	CC="$(CC)" \
	CPPFLAGS="$(CPPFLAGS)" \
	CFLAGS="$(CFLAGS)" \
	LDFLAGS="$(LDFLAGS)"

build/$(TARGET)/%.o: $(PLATFORM)/%.c
	mkdir -p build/$(TARGET)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/$(TARGET)/%.o: $(PLATFORM)/%.m
	mkdir -p build/$(TARGET)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/$(TARGET)/yuv.o: yuv.c
	mkdir -p build/$(TARGET)
	$(CC) $(CPPFLAGS) $(CFLAGS) -O3 -c $< -o $@

build/$(TARGET)/$(PLATFORM)-yuv.o: $(PLATFORM)/yuv.c
	mkdir -p build/$(TARGET)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/$(TARGET)/image.o: image.c
	mkdir -p build/$(TARGET)
	$(CC) $(CPPFLAGS) $(CFLAGS) -O3 -c $< -o $@

TESTS:= \
	build/$(TARGET)/test-image \
	build/$(TARGET)/test-yuv

$(TESTS): al.h

build/$(TARGET)/test-image: image.c
	mkdir -p build/$(TARGET)
	$(CC) $(CPPFLAGS) -DTEST $(CFLAGS) $< -o $@

build/$(TARGET)/test-yuv: yuv.c
	mkdir -p build/$(TARGET)
	$(CC) $(CPPFLAGS) -DTEST $(CFLAGS) $< -o $@

.PHONY: clean-test
clean-test:
	rm -f $(TESTS)

.PHONY: test
test: $(TESTS) al.py
	for x in $(TESTS); do \
		./$$x; \
	done
	$(PYTHON) al.py

OBJS:=	\
	build/$(TARGET)/camera.o \
	build/$(TARGET)/common.o \
	build/$(TARGET)/dirs.o \
	build/$(TARGET)/display.o \
	build/$(TARGET)/image.o \
	build/$(TARGET)/locale.o \
	build/$(TARGET)/net.o \
	build/$(TARGET)/permissions.o \
	build/$(TARGET)/yuv.o \
	build/$(TARGET)/$(PLATFORM)-yuv.o

$(OBJS): al.h

build/$(TARGET)/libal.dylib: $(OBJS)
	$(CC) -bundle -o $@ $(OBJS) $(LDFLAGS)
	$(STRIP) -x $@

build/$(TARGET)/libal.so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(LDFLAGS)
	$(STRIP) --strip-unneeded $@

.PHONY: so
so: build/$(TARGET)/libal$(SOEXT)

PREFIX?=	/usr
LIBDIR?=	lib

.PHONY: install
install:
	mkdir -p $(DESTDIR)$(PREFIX)/$(LIBDIR)
	install build/$(TARGET)/libal$(SOEXT) $(DESTDIR)$(PREFIX)/$(LIBDIR)

.PHONY: check
check:
	find . -name '*.py' | xargs pyflakes
	find . -name '*.py' | xargs pycodestyle
	find . -name '*.py' | xargs mypy *.py
	luacheck *.lua

.PHONY: docs
docs:
	$(MAKE) -f darwin/Makefile so
	LIBAL_LIBRARY_PATH=./build/arm64-apple-darwin pdoc python/libal -o docs/python/

.PHONY: cleanup
cleanup:
	"$(MAKE)" -f android/Makefile cleanup
	"$(MAKE)" -f darwin/Makefile cleanup

.PHONY: clean
clean: cleanup clean-tests
	"$(MAKE)" -f android/Makefile clean
	"$(MAKE)" -f darwin/Makefile clean
	rm -rf .mypy_cache
	dot_clean .
	find . -name .DS_Store | xargs rm -f

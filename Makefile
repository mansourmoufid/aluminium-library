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

OBJS:=	\
	build/$(TARGET)/common.o \
	build/$(TARGET)/dirs.o \
	build/$(TARGET)/locale.o

build/$(TARGET)/libal.dylib: $(OBJS)
	$(CC) -bundle -o build/$(TARGET)/libal.dylib $(OBJS) $(LDFLAGS)

build/$(TARGET)/libal.so: $(OBJS)
	$(CC) -shared -o build/$(TARGET)/libal.so $(OBJS) $(LDFLAGS)

.PHONY: so
so: build/$(TARGET)/libal$(SOEXT)

PREFIX?=	/usr
LIBDIR?=	lib

.PHONY: install
install:
	install build/$(TARGET)/libal$(SOEXT) $(DESTDIR)$(PREFIX)/$(LIBDIR)

.PHONY: check
check:
	pyflakes *.py
	pycodestyle *.py
	mypy *.py
	luacheck *.lua

.PHONY: test
test: al.py
	$(PYTHON) al.py

.PHONY: cleanup
cleanup:
	$(MAKE) -f android/Makefile cleanup
	$(MAKE) -f darwin/Makefile cleanup

.PHONY: clean
clean: cleanup
	$(MAKE) -f android/Makefile clean
	$(MAKE) -f darwin/Makefile clean
	rm -rf .mypy_cache
	dot_clean .
	find . -name .DS_Store | xargs rm -f

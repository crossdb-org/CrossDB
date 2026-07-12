# Override to install without root, e.g.: make install PREFIX=$$HOME/.local
PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib
INCDIR := $(PREFIX)/include

help:
	@echo "make build                Build crossdb library and tool"
	@echo "make debug                Build crossdb library and tool with debug"
	@echo "make run                  Run crossdb tool"
	@echo "make clean                Clean build result"
	@echo "make install              Install crossdb(lib&tool&header) to Linux/MacOS/FreeBSD"
	@echo "                          set PREFIX to install without root, e.g."
	@echo "                          make install PREFIX=\$$HOME/.local"
	@echo "make uninstall            Uninstall crossdb from Linux/MacOS/FreeBSD"
	@echo "make example              Build and run example (need to install crossdb first)"
	@echo "make smoketest            Build and run smoke test (need to install crossdb first)"
	@echo "make bench                Build and run bench test (need to install crossdb first)"
	@echo "make bench-sqlite         Build and run sqlite bench test (need to install sqlite3 first)"
	@echo "make bench-stlmap         Build and run C++ STL Map and HashMap(unordered_map) bench test"
	@echo "make bench-boostmidx      Build and run C++ Boost MultiIndex Order and Hash bench test"

.PHONY: build
build:
ifeq ($(shell uname -s), Darwin)
else
	$(CC) -o build/libcrossdb.so -fPIC -shared -lpthread -O2 src/crossdb.c
endif
	$(CC) -o build/xdb-cli src/xdb-cli.c -O2 -lpthread
	cp include/crossdb.h build/

debug:
	$(CC) -o build/libcrossdb.so -fPIC -lpthread -shared -g -DXDB_DEBUG src/crossdb.c
	$(CC) -o build/xdb-cli src/xdb-cli.c -lpthread -g
	cp include/crossdb.h build/

smoketest:
	make -C test/

run:
	build/xdb-cli

clean:
	rm -rf build/*
	make -C examples/c/ clean
	make -C bench/basic/ clean
	make -C test/ clean

wall:
	$(CC) -o build/xdb-cli src/xdb-cli.c -lpthread -O2 -Wall

gdb:
	$(CC) -o build/xdb-cli src/xdb-cli.c -lpthread -fsanitize=address -g
	gdb build/xdb-cli

install:
	@mkdir -p $(LIBDIR)/
	@mkdir -p $(BINDIR)/
	@mkdir -p $(INCDIR)/
	install -c build/xdb-cli $(BINDIR)/
	install -c build/crossdb.h $(INCDIR)/
ifeq ($(shell uname -s), Darwin)
	$(CC) -o $(LIBDIR)/libcrossdb.dylib -dynamiclib -lpthread -O2 src/crossdb.c
else
	install -c build/libcrossdb.so $(LIBDIR)/
ifeq ($(PREFIX), /usr/local)
	ldconfig
endif
endif

winpack:
	mkdir -p build/crossdb-win64/include
	mkdir -p build/crossdb-win64/bin
	mkdir -p build/crossdb-win64/lib
	cp build/crossdb.h build/crossdb-win64/include
	cp build/xdb-cli.exe build/crossdb-win64/bin
	cp build/libcrossdb.dll build/libcrossdb.lib build/crossdb-win64/lib

uninstall:
	rm -rf $(BINDIR)/xdb-cli
	rm -rf $(INCDIR)/crossdb.h
ifeq ($(shell uname -s), Darwin)
	rm -rf $(LIBDIR)/libcrossdb.dylib
else
	rm -rf $(LIBDIR)/libcrossdb.so
endif

example:
	make -C examples/c/

.PHONY: bench
bench:
	make -C bench/basic/

bench-sqlite:
	make -C bench/basic/ sqlite

bench-stlmap:
	make -C bench/basic/ stlmap

bench-boostmidx:
	make -C bench/basic/ boostmidx

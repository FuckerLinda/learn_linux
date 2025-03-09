all: lib_build src_build

lib_build:
    $(MAKE) -C lib

src_build: lib_build
    $(MAKE) -C src

clean:
    $(MAKE) -C lib clean
    $(MAKE) -C src clean
    rm -rf build/*

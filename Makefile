.PHONY: all compile clean

ifeq ($(shell which meson),)
    $(error Please install meson first!)
endif

ifeq ($(shell which ninja),)
    $(error Please install ninja first!)
endif

all: builddir compile

builddir:
	meson setup builddir --buildtype=release

compile: builddir
	ninja -C builddir

clean: builddir
	ninja -C builddir clean

distclean:
	rm -rf builddir
.PHONY: all compile clean

ifeq ($(shell which meson),)
    $(error Please install meson first!)
endif

ifeq ($(shell which ninja),)
    $(error Please install ninja first!)
endif

all: builddir/stamp compile

builddir/stamp:
	meson setup builddir --buildtype=release
	touch $@

compile: builddir/stamp
	ninja -C builddir

clean: builddir/stamp
	ninja -C builddir clean

distclean:
	rm -rf builddir

builddir-ios:
	meson setup builddir-ios -Dios=true --buildtype=release --cross-file ios-cross.txt

ios: builddir-ios
	ninja -C builddir-ios

builddir-android:
	meson setup builddir-android -Dandroid=true --buildtype=release --cross-file android-cross.txt

android: builddir-android
	ninja -C builddir-android

builddir-aarch64-linux:
	meson setup builddir-aarch64-linux -Dlinux-cross=aarch64 --buildtype=release --cross-file aarch64-linux-cross.txt

aarch64-linux: builddir-aarch64-linux
	ninja -C builddir-aarch64-linux

builddir-gem5:
	meson setup builddir-gem5 -Dgem5=true --buildtype=release

gem5: builddir-gem5
	ninja -C builddir-gem5

.PHONY: gem5

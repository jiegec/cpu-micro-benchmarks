# Make MSR driver for Linux
# Last modified: 2020-08-17 Agner Fog
# See https://www.kernel.org/doc/html/latest/kbuild/modules.html


KERNELDIR := /lib/modules/`uname -r`/build
obj-m := MSRdrv.o
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
    
#$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules    

clean:
	rm -f .MSRdrv.*
	rm -f -r .tmp_versions
	rm -f *~
	rm -f MSRdrv.ko
	rm -f MSRdrv.o
	rm -f MSRdrv.mod.*
	rm -f linux.mdl
	rm -f Modules.symvers
    
install:
	./install.sh
    
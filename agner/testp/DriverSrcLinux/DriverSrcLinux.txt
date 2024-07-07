Instructions for Linux driver                     2011-06-19 Agner Fog

To install the Linux driver for PMCTest under Linux, 32 or 64 bit, 
unzip DriverSrcLinux.zip, make and install the driver according to 
the following commands. Must reinstall after reboot.
The driver has only been tested in Ubuntu.


make
chmod 744 *.sh
sudo ./install.sh


In some older systems you may need to replace MSRdrv.c with MSRdrv1.c if
compilation gives errors.

If build directory is missing:

sudo ln -s /usr/src/linux-headers-`uname -r` /lib/modules/`uname -r`/build

Or if the target doesn't exist, e.g.:

sudo ln -s /usr/src/linux-headers-2.6.24-23-server /lib/modules/`uname -r`/build

In Red Hat/Fedora you may need the following:
rpm -q kernel kernel-source
or
yum -y install kernel-devel kernel-headers
If it installs a wrong version, run:
yum distro-sync
reboot
./install2.sh


install.sh:

	mknod /dev/MSRdrv c 222 0
	chmod 666 /dev/MSRdrv
	insmod -f MSRdrv.ko
	
uninstall.sh:

	rm -f /dev/MSRdrv
	rmmod MSRdrv

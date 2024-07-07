# Install MSR driver, alternative method
make clean
make

mknod /dev/MSRdrv c 249 0
chmod 666 /dev/MSRdrv
#insmod -f MSRdrv.ko
#instead of insmod:
KERNELDIR=/lib/modules/`uname -r`
mkdir $KERNELDIR/extra
cp MSRdrv.ko $KERNELDIR/extra
depmod -ae
modprobe MSRdrv
#modprobe --force-vermagic MSRdrv

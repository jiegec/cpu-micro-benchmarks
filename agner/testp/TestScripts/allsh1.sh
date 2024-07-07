#!/bin/bash
#                                                           2016-10-27 Agner Fog
# Compile and run PMCTest with various scripts
# looping through scripts with extension .sh1
# (c) Copyright 2012-2016 by Agner Fog. GNU General Public License www.gnu.org/licenses

# various initializations (only necessary first time):
. vars.sh

# warm up processor to max clock frequency
echo -e "\nwarmup\n"
$ass -f elf64 -o b64.o -Dinstruct=nop -DWARMUPCOUNT=10000000 -Dnthreads=1 TemplateB64.nasm
if [ $? -ne 0 ] ; then exit ; fi
g++ -fno-pie -no-pie -m64 a64.o b64.o -ox -lpthread -z noexecstack
if [ $? -ne 0 ] ; then exit ; fi
./x >> /dev/null

# run all test scripts
for xscript in  *.sh1
do
  echo -e "\n$xscript"
  ./$xscript
done


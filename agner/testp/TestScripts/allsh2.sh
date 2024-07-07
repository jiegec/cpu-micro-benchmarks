#!/bin/bash
#                                                           2016-10-27 Agner Fog
# Compile and run PMCTest with various scripts
# looping through scripts with extension .sh2

# (c) 2012-2016 by Agner Fog. GNU General Public License www.gnu.org/licenses

# various initializations (only necessary first time):

# mkdir results2

. vars.sh

# warm up processor to max clock frequency
echo -e "\nwarmup\n"

./warmup_fp.sh2

# run all test scripts
for xscript in  *.sh2
do
  echo -e "\n$xscript"
  ./$xscript
done

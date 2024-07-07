#!/bin/bash
#                                                           2016-10-27 Agner Fog
# Compile and run PMCTest with various instructions defined in comma-separated lists
# looping through all lists with extension .csv

# (c) 2016 by Agner Fog. GNU General Public License www.gnu.org/licenses

# various initializations (only necessary first time):

# mkdir results

. vars.sh

export outdir=results

# warm up processor to max clock frequency
# echo -e "\nwarmup\n"
# ./warmup_fp.sh2

# run all test scripts
for xscript in  *.csv
do
  echo -e "\n$xscript"
  ./runlist.sh $xscript
done

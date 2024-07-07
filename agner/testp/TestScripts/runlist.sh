#!/bin/bash
#                                                       2022-10-28 Agner Fog
# runlist.sh
# Compile, assemble and run PMCTest to measure latency and throughput for
# multiple instructions specified in a comma-separated list
# (c) Copyright 2016-2021 by Agner Fog. GNU General Public License www.gnu.org/licenses

#################################################################################################################
#
# The list file is specified on the command line.
# Format for the list file:
# General lines contain an instruction to test, and various parameters, separated by commas:
# 1. name of instruction
# 2. register size and type: 8, 16, 32, 64 = general purpose register of this size,
#    128, 256, 512 = vector register of this size, 
#    mmx = 64 bit mmx register, h = high 8-bit register,
#    k = mask register (size may be specified as k8, k16, k32, k64).
#    m = memory operand of specified size (only for RNDTR test mode. size = 0 for unspecified size).
# 3. Number of operands (register or memory), except immediate operands
# 4. Value of immediate operand, blank or n for none
# 5. Test mode: L = latency, T = throughput, M = throughput with memory operand, LTM = all these.
# (S is the same as T but with 8 different destination registers rather than 5. This sometimes makes a difference on AMD)
#    RNDTR: measure round trip latency of 2 or 3 instructions (see below),
#    MASKxxx: measure latency and throughput of instructions involving mask registers (see below),
#    DIV: division and square root instructions (best and worst case),
#    GATHER: gather and scatter instructions.
# 6. Instruction set required, e.g. AVX2
# 7. Additional options: use32 = run in 32 bit mode. Multiple options are separated by space
#
# Special lines:
# A line beginning with a hash sign (#) is a comment. This line is ignored.
# A line beginning with a dollar sign ($) specifies a parameter:
# $file=xxx: Specify name for output file. 
#   Warning: This will delete and overwrite any data already written to that filename.
# $outdir=xxx: Specify directory for output file.
# $text=xxx: Copy this text to the output file. (Enclose "$text=xxx" in quotation marks if it contains commas)
# $repeat0=xxx: Set number of repetitions of each test. Default = 8
# $repeat1=xxx: Set repeat count for loop around testcode. Default = 10
# $repeat2=xxx: Set repeat count for repeat macro around testcode. Default = 100
# $list=1: Turn on assembly output listing.
#
# The file must end with a blank line
#################################################################################################################
#
# LTM test modes:
# Possible additional options: 
# regval0, regval1: initial values of source operands, integer or floating point, only for vector instructions,
#   cannot be combined with use32. Default = 0.
# elementsize: Size of vector elements (bits). Only required if regval0/1 specified. Default = 32
# 3op: 3-operand instruction where first operand is both source and destination
#
#################################################################################################################
#
# RNDTR test mode: 
# This is useful for testing latencies of instructions with different register types for input and output.
# Specify two or three instructions, separated by /
# Specify the register types, separated by /. These are the source operand types for each instruction.
# (The destination type is the source type for the next instruction).
# Specify the number of operands for each instruction (2 or 3), separated by /. This includes register and memory 
# operands but not immediate operands. "3x" indicates that the first source operand is the same type as the 
# destination operand, while the second source operand has the type indicated.
# Specify the immediate operands if necessary, separated by /. Write "n" for none, or blank for all none.
# The second or third instruction may be set to "n" to indicate implicit conversion between different sizes of 
# the same register.
# The throughputs of each instruction can also be measured. Specify the number of instruction to measure
# the throughput for as a number suffix, for example RNDTR2 will measure the throughput of the first two 
# instructions in addition to the roundtrip latency.
#################################################################################################################
#
# MASK test modes:
# MASKCMPL, MASKCMPT, MASKCMPLT: Measure latency and/or throughput of instructions that have a mask register output,
#     e.g. VCMPEQD k1{k1},zmm0,zmm1
# MASKBLENDL, MASKBLENDT, MASKBLENDLT: Measure latency and/or throughput of instructions that have a mask register 
#     input. Measuring latency requires a round trip, e.g. VPBLENDMD zmm0{k1},zmm1,zmm2 / VPCMPEQD k1,zmm3,zmm0
# MASKL, MASKT, MASKLT: Measure latency and/or throughput of masked instructions, e.g. VPADDD zmm1{k1},zmm2,zmm1
# The mask value for MASKBLENDT and MASKL/T can be specified in the options field as "mask=n" (signed decimal number)
# The size of the mask register can be specified in the options field as "masksize=n" (default=16)
# A zeroing option {z} can be specified in the options field as "usezero"
#
#################################################################################################################
#
# DIV test modes: DIVL, DIVT, DIVLT
# Measure latency and/or throughput of floating point division, squareroot, reciprocal and reciprocal squareroot
# instructions in vector registers. This test mode will test both a trivial best case (e.g. 1.0/1.0) and a worst
# case with non-round numbers.
# A mask can be specified in the options field as "mask=n" (signed decimal number)
# The size of the mask register can be specified in the options field as "masksize=n" (default=16)
# A zeroing option {z} can be specified in the options field as "usezero"
#
#################################################################################################################
#
# GATHER test modes: 
# GATHER_CONTIGUOUS: contiguous elements in memory are gathered or scattered
# GATHER_STRIDE: sequential elements with a stride equal to the size of 4 elements are gathered or scattered
# GATHER_RANDOM: elements are gathered or scattered from random memory posistion. 
# GATHER_SAME: all elements are gathered or scattered from the same memory position
# GATHER_PART_OVERLAP: elements are gathered or scattered from partially overlapping memory positions
# GATHER_LATENCY: the latency of gather + store or scatter + load is measured. Set mask = 1 to get latency for a single element
#
# Two register sizes must be specified, separated by a slash: data_vector/index_vector
# Number of operands includes data, memory operand, and mask. Default = 3.
# The immediate operand field can be set to an initial value for the mask. e.g. -1 for all elements, 1 for a single element
# The options field must be set to "masktype=k" to use a mask register for status mask, or "masktype=v" to use a vector register
#
#################################################################################################################
#
# User-defined test modes: 
# Define the necessary code as assembly macros in a file xxx.inc. Define it in the test mode field as macros=xxx.inc
# The options instruct, regsize, numop, numimm and immvalue are transferred to the assembly code.
# Extra options to transfer to the assembly code can be defined in the options field as option=value (separated by space)
# 
#################################################################################################################


# Detect CPU specific variables
 . vars.sh

# Case insensitive string compare
shopt -s nocasematch

# Read file
filename=$1
if [ ! -e $filename ] ; then
echo Error: file $filename not found
exit 99
fi

# default name of output file is same as input file with extension (.csv) changed to .txt
outfile="${filename/\.[a-z]*/.txt}"

# Initialize variables to default values
repeat0=10            # number of repetitions of each test
repeat1=100           # loop around test code
repeat2=1000          # test code occurs repeat2 times in code (macro loop)
exitcode=0            # exit code is nonzero if test code crashes
asmlisting=0          # output listing from assembler
initoutputfile=1      # this is set to 0 when output file has been initialized
if [ -z "$outdir" ] ; then 
  outdir="."          # directory for output file. May be specified in environment
fi

# Read cpuinfo.txt containing supported instruction sets
if [ ! -e "cpuinfo.txt" ] ; then
echo Error: file cpuinfo.txt not found
exit 99
fi
cpuinfo=`cat cpuinfo.txt`

# Function to assemble code with the specified options and run it
function compileAndRun {
  if [[ $asmlisting == 1 ]] ; then
    # turn on assembly listing
    listpar="-l $instruct-$tmode.lst"
  else
    listpar=""
  fi
  exitcode=0

  if [[ "$options" == *"use32"* ]] ; then
    # 32 bit mode
    # eval is needed to expand $ass inside function
    eval $ass -f elf32 -o b32.o $parameters -Dcounters="$pmc" -P$macros $listpar TemplateB32.nasm
    if [ $? -ne 0 ] ; then 
      echo "*** Assembly failed" >> $outdir/$outfile
      exitcode=1
      return 1
      # exit
    fi
    # use g++ for linking to get the right libraries
    g++ -fno-pie -no-pie -m32 -fno-pie -no-pie a32.o b32.o -ox -lpthread -z noexecstack
    if [ $? -ne 0 ] ; then 
      echo "*** Linking failed" >> $outdir/$outfile
      exitcode=2
      return 2
      # exit
    fi
  else
    # 64 bit mode
    # eval is needed to expand $ass inside function
    eval $ass -f elf64 -o b64.o $parameters -Dcounters="$pmc" -P$macros $listpar TemplateB64.nasm
    if [ $? -ne 0 ] ; then 
      echo "*** Assembly failed" >> $outdir/$outfile
      exitcode=1
      return 1
      # exit
    fi
    # use g++ for linking to get the right libraries
    g++ -fno-pie -no-pie -m64 -fno-pie -no-pie a64.o b64.o -ox -lpthread -z noexecstack
    if [ $? -ne 0 ] ; then 
      echo "*** Linking failed" >> $outdir/$outfile
      exitcode=2
      return 2
      # exit
    fi
  fi
  # run the compiled test code
  ./x >> $outdir/$outfile
  exitcode=$?
  if [ $exitcode -ne 0 ] ; then 
    echo "*** Execution failed with exit code $exitcode" >> $outdir/$outfile
  fi
}


# Loop through the lines of the input file:
oldIFS=$IFS
IFS=$'\n'
cat $filename | while read -r line0
do

# remove quotation marks and windows carriage return
line=`echo "$line0" | tr -d '\r"'`

# skip blank lines
if [[ -z "${line//[ ,]}" ]] ; then continue ; fi

# skip comment lines
if [[ "${line:0:1}" == "#" ]] ; then continue ; fi

# detect special lines
if [ "${line:0:1}" = "$" ] ; then 
  # line begins with "$". 
  # remove commas
  line1=`echo "$line" | tr -d ','`
  # Split by "="
  IFS="="
  spliteq=(${line1:1})
  varname="${spliteq[0]}"
  value="${spliteq[1]}"
  varname=${varname// }
  value=${value// }

  if [[ $varname == "file" ]] ; then
    # set name of output file
    outfile=$value
    initoutputfile=1
  elif [[ $varname == "dir" ]] ; then
    # set directory for ourput file
    outdir=$value
    if [ ! -e $outdir ] ; then mkdir $outdir ; fi

  elif [[ $varname == "text" ]] ; then
    # text to output file
    if [[ $initoutputfile == 1 ]] ; then 
      date +%Y-%m-%d:%H:%M:%S > $outdir/$outfile
      initoutputfile=0
    fi
    # Extract text and instruction set
    if [[ "${line0:0:1}" == '"' ]] ; then
      # Enclosed in ""
      text=`echo ${line0:7} | sed 's/\([^"]*\)".*/\1/'`
      instrset=`echo ${line0:7} | sed 's/[^"]*",,,,,\([^,]*\),.*/\1/;s/"//g'`
    else
      # Not enclosed in ""
      IFS=","
      splitline=(${line0:6})
      text=${splitline[0]}
      instrset=${splitline[5]//[ \"]}
    fi
    # discard text if specified instruction set is not supported
    if ! [[ -z "$instrset" || "$cpuinfo" =~ "$instrset" ]] ; then continue ; fi
    echo -e "$text" >> $outdir/$outfile

  elif [[ $varname == "repeat0" ]] ; then
    repeat0=$value
  elif [[ $varname == "repeat1" ]] ; then
    repeat1=$value
  elif [[ $varname == "repeat2" ]] ; then
    repeat2=$value
  elif [[ $varname == "list" ]] ; then
    asmlisting=$value
  else
    echo Syntax error in line: $line
  fi
# reset field separator for read loop
IFS=$'\n'
continue
fi

# General comma-separated line
# Check if output file needs initialization
if [[ $initoutputfile == 1 ]] ; then 
  date +%Y-%m-%d:%H:%M:%S > $outdir/$outfile
  initoutputfile=0
fi

# Split by commas
IFS=","
splitline=($line)

instruct=${splitline[0]// }
regsize=${splitline[1]// }
numop=${splitline[2]// }
immvalue=${splitline[3]// }
tmode=${splitline[4]// }
instrset=${splitline[5]// }
options=${splitline[6]}

# check if 32 bit mode required and supported
if [[ "$options" == *"use32"* && $support32bit == 0 ]] ; then continue ; fi

# check if instruction set is supported
if ! [[ -z "$instrset" || "$cpuinfo" =~ "$instrset" ]] ; then continue ; fi

# check immediate operand
if [[ ! -z $immvalue && $immvalue != "n" ]] ; then
  numimm=1
else
  numimm=0
fi

# check register size and type
if [[ $regsize == mmx ]] ; then
  regtype=mmx
  regsize=64
elif [[ ${regsize:0:1} == k ]] ; then
  regtype=k
  regsize=${regsize:1}
  if [[ -z $regsize ]] ; then regsize=16 ; fi
elif [[ $regsize == h ]] ; then
  regtype=h
  regsize=8
elif [[ $regsize =~ ^[0-9]+$ ]] ; then
  # regsize is a number
  if [[ $regsize -lt 65 ]] ; then
    regtype=r
  else 
    regtype=v
  fi
# other types. resolve in mode specific code
fi

# default performance monitor counters
pmc="$PMCs"
pmclist="$PMCs"

# get extra options. Set -D before option=value to get -Doption=value. Remove options without equal sign
xoptions=`echo $options | sed 's/ /\n/g' | sed '/=/!d;s/[a-zA-Z0-9]*=[a-zA-Z0-9.eE]*/-D&/g' | tr '\n' ' '`

# choose according to test mode

# standard LTM modes:
####################################################
if [[ "$tmode" =~ ^[LTMUSltmus]+$ ]] ; then  # tmode contains only L,T,M,U,S. This does not work with "" around the regex
  macros="lt.inc"  # file containing assembly macros
  parameters=""
  for (( i=0; i<${#tmode}; i++ )); do    # loop for each letter in tmode
    tmodei=${tmode:$i:1}
    if [[ $tmodei == "L" ]] ; then
      text1="Latency"
      text3="$numop register operands"
      pmclist="$PMCs"
    elif [[ $tmodei == "T" ]] ; then
      text1="Throughput"
      text3="$numop register operands"
      pmclist="$PMClist"
      if [[ "$options" =~ "3op" ]] ; then tmodei="T0" ; fi
    elif [[ $tmodei == "S" ]] ; then
      text1="Throughput (8 registers)"
      text3="$numop register operands"
      pmclist="$PMClist"
    elif [[ $tmodei == "M" ]] ; then
      text1="Throughput with memory operand"
      text3="$(($numop-1)) register operands and a memory operand"
      pmclist="$PMCs"
      if [[ "$options" =~ "3op" ]] ; then tmodei="M0" ; fi
    fi
    if [[ "$options" == *"use32"* ]] ; then 
      text2=", 32 bit mode"
    else
      text2=""
    fi
    if [[ $numimm != 0 ]] ; then
      text4=" and immdiate operand ($immvalue)"
    else
      text4=""
    fi
    # Output text to file
    echo -e "\n$instruct: $text1$text2, $text3$text4, type $regtype, size $regsize  $xoptions" >> $outdir/$outfile

    # Set parameters
    parameters="-Dinstruct=$instruct -Dtmode=$tmodei -Dnumop=$numop -Dregsize=$regsize -Dregtype=$regtype -Drepeat0=$repeat0 -Drepeat1=$repeat1 -Drepeat2=$repeat2 -Dnumimm=$numimm -Dimmvalue=$immvalue $xoptions"

    # Run the test
    IFS=" "
    for pmc in $pmclist ; do
      compileAndRun
      if [[ $exitcode != 0 ]] ; then break 2 ; fi
    done
    IFS=","
  done


# RNDTR mode:
####################################################
elif [[ "$tmode" =~ ^RNDTR[0-3]?$ ]] ; then
  numthroughp="${tmode:5}"
  if [ -z $numthroughp ] ; then numthroughp=0 ; fi
  # Split parameters by slashes
  IFS="/"
  splitinstruct=($instruct)
  regsize=${splitline[1]// }
  splitregsize=($regsize)
  splitnumop=($numop)
  splitimmvalue=($immvalue)
  IFS=" "
  splitregtype=(r r r)
  splitnumimm=(0 0 0)
  IFS=","
  # number of instructions
  numinstr="${#splitinstruct[@]}"
  if [ $numinstr -gt 3 ] ; then
    echo Too many instructions: $instruct
    exit 99
  fi

  IFS=" "
  #for i in {1.."$numinstr"} ; do  # why does this not work? For some reason it uses IFS="\n"?
  for ((i=0; i<$numinstr; i++)); do 
    # check register size and type
    regsizei=${splitregsize["$i"]}
    if [[ -z $regsizei ]] ; then 
      echo Error: all $numinstr register sizes must be indicated for $instruct
      exit
    elif [[ $regsizei == mmx ]] ; then
      regtypei=mmx
      regsizei=64
    elif [[ ${regsizei:0:1} == k ]] ; then
      regtypei=k
      regsizei=${regsizei:1}
      if [[ -z $regsizei ]] ; then regsizei=16 ; fi
    elif [[ $regsizei == h ]] ; then
      regtypei=h
      regsizei=8
    elif [[ ${regsizei:0:1} == m ]] ; then
      regtypei=m
      regsizei=${regsizei:1}
      if [[ -z $regsizei ]] ; then regsizei=0 ; fi
    elif ! [[ $regsizei =~ ^[0-9]+$ ]] ; then
      echo unknown register type $regsizei
      exit
    elif [[ $regsizei -lt 65 ]] ; then
      regtypei=r
    else 
      regtypei=v
    fi
    splitregsize[$i]=$regsizei
    splitregtype[$i]=$regtypei
    # check number of operands    
    if [[ -z ${splitnumop["$i"]} ]] ; then 
      echo Error: Number of operands for all $numinstr instruction must be indicated for $instruct
      exit
    fi
    # check immediate operand
    immvaluei=${splitimmvalue[$i]}
    #if [[ ! -z $immvaluei && $immvaluei != "n" ]] ; then
    if [[ $immvaluei != "" && $immvaluei != "n" ]] ; then
      splitnumimm[$i]=1
    fi
    if [[ -z ${splitnumop[$i]} || ${splitnumop[$i]} < 2 ]] ; then
      echo Wrong number of operands for instruction ${splitinstruct[$i]}
      continue 2
    fi
  done

  # make parameter list and output comment
  par=""
  echo -e "\nRound trip latency test\n# Instruction | num. op. | dest. operand  | source operand | immediate operand" >> $outdir/$outfile
  for ((i=0; i<$numinstr; i++)); do 
     if [[ ${splitnumimm[$i]} == 1 ]] ; then imm=${splitimmvalue[$i]} ; else imm="" ; fi
     i1=$(($i+1))  # 1-based index
     if [[ $i1 == $numinstr ]] ; then dest=0 ; else dest=$i1 ; fi  # index to destination
     par="$par -Dinstruct$i1=${splitinstruct[$i]} -Dregtype$i1=${splitregtype[$i]} -Dregsize$i1=${splitregsize[$i]} -Dnumop$i1=${splitnumop[$i]} -Dnumimm$i1=${splitnumimm[$i]} -Dimmvalue$i1=$imm"
     instrname=${splitinstruct[$i]}
     if [[ $instrname == n ]] ; then instrname="(none)" ; fi
     printf "%i: %10s | %8i | %14s | %14s | %s\n" $i1 $instrname ${splitnumop[$i]//x} "${splitregtype[$dest]}${splitregsize[$dest]}" "${splitregtype[$i]}${splitregsize[$i]}" $imm >> $outdir/$outfile
  done
  parameters="-Dtmode=L -Drepeat0=$repeat0 -Drepeat1=$repeat1 -Drepeat2=$repeat2 -Dregsize=${splitregsize[0]} $par"
  instruct="${splitinstruct[0]}"

  # latency
  macros="rndtr.inc"  # file containing assembly macros
  pmc="$PMCs"
  compileAndRun
  if [[ $exitcode != 0 ]] ; then continue ; fi

  # throughput
  for ((j=0; j<$numthroughp; j++)); do
    if [[ ${splitinstruct[$j]} == "n" ]] ; then continue ; fi
    # find destination operand type
    if [[ $j == $((numinstr-1)) ]] ; then k=0 ; else k=$((j+1)) ; fi
    echo -e "\nThroughput for ${splitinstruct[$j]}" "${splitregtype[$k]}${splitregsize[$k]}", "${splitregtype[$j]}${splitregsize[$j]}" >> $outdir/$outfile
    parameters="-Dtmode=T$((j+1)) -Drepeat0=$repeat0 -Drepeat1=$repeat1 -Drepeat2=$((repeat2/2)) -Dregsize=${splitregsize[$j]} $par"
    for pmc in $PMClist ; do
      compileAndRun
      if [[ $exitcode != 0 ]] ; then break ; fi
    done
  done


# MASKxxx mode:
####################################################
elif [[ "$tmode" =~ ^MASK.+$ ]] ; then
macros="mask.inc"  # file containing assembly macros

  # Split parameters by slashes
  IFS="/"
  splitinstruct=($instruct)
  splitnumop=($numop)
  splitimmvalue=($immvalue)
  IFS=" "
  splitnumimm=(0 0 0)
  # number of instructions
  numinstr="${#splitinstruct[@]}"
  if [ $numinstr -gt 2 ] ; then
    echo Too many instructions: $instruct
    exit 99
  fi
  IFS=" "
  for ((i=0; i<$numinstr; i++)); do 
    # check immediate operand
    immvaluei=${splitimmvalue[$i]}
    if [[ $immvaluei != "" && $immvaluei != "n" ]] ; then
      splitnumimm[$i]=1
    fi
    if [[ -z ${splitnumop[$i]} || ${splitnumop[$i]} < 2 || ${splitnumop[$i]} > 3 ]] ; then
      echo Wrong number of operands for instruction ${splitinstruct[$i]}
      continue 2
    fi
  done
  instruct1=${splitinstruct[0]}
  instruct2=${splitinstruct[1]}
  instruct=$instruct1
  numop1=${splitnumop[0]}
  numop2=${splitnumop[1]}
  immvalue1=${splitimmvalue[0]}
  immvalue2=${splitimmvalue[1]}
  numimm1=${splitnumimm[0]}
  numimm2=${splitnumimm[1]}
  if [[ $options =~ .*usezero.* ]] ; then usezero=1 ; else usezero=0 ; fi
  if [[ $options =~ .*mask=.* ]] ; then
    nmask=`echo $options | sed 's/.*mask=\([0-9-]*\).*/\1/i'`
    #masksize=`echo $options | grep 'masksize=' | sed 's/.*masksize=\([0-9]*\).*/\1/i'`
    masksize=`echo $options | sed '/masksize=\([0-9]*\)/!d;s/.*masksize=\([0-9]*\).*/\1/i'`
    if [ -z $masksize ] ; then masksize=16 ; fi
  else
    nmask=""; masksize=0
  fi

  par1="-Dinstruct1=$instruct1 -Dinstruct2=$instruct2 -Dregsize=$regsize -Dnumop1=$numop1 -Dnumop2=$numop2 -Dimmvalue1=$immvalue1 -Dimmvalue2=$immvalue2 -Dnumimm1=$numimm1 -Dnumimm2=$numimm2"
  par2="-Dusezero=$usezero -Dnmask=$nmask -Dmasksize=$masksize"
  par3="-Drepeat0=$repeat0 -Drepeat1=$repeat1 -Drepeat2=$repeat2"
  instruct="$instruct1"
  
  # split test mode string into MASKxxxL and MASKxxxT
  if [[ $tmode =~ MASKCMP.+ ]] ; then
    tmode1="MASKCMP"
    tmode2=${tmode:7}
  elif [[ $tmode =~ MASKBLEND.+ ]] ; then
    tmode1="MASKBLEND"
    tmode2=${tmode:9}
  elif [[ $tmode =~ MASK.+ ]] ; then
    tmode1="MASK"
    tmode2=${tmode:4}
  else
    echo error unknown test mode $tmode
    break
  fi
  for ((j=0; j<${#tmode2}; j++)); do
    tmode12=$tmode1${tmode2:$j:1}
    parameters="-Dtmode=$tmode12 $par1 $par2 $par3"

    # output text
    if [[ $numop1 > 2 ]] ; then text2=", r$regsize" ; else text2="" ; fi
    if [[ $numimm1 > 0 ]] ; then text3=", $immvalue1" ; else text3="" ; fi
    if [[ $masksize > 0 ]] ; then 
      text4="{k}"
      text5=" (mask = $nmask)"
    else 
      text4=""
      text5=""
    fi
    if [ $usezero -eq 1 ] ; then text6="{z}" ; else text6="" ; fi
    pmclist="$PMCs"
    if [[ $tmode12 == "MASKCMPL" ]] ; then
      text="$instruct1: Latency through mask register ($tmode12)\n"
      text="$text$instruct1 k{k}, r$regsize$text2$text3"
    elif [[ $tmode12 == "MASKCMPT" ]] ; then
      text="$instruct1: Throughput ($tmode12)\n"
      text="$text$instruct1 k$text4, r$regsize$text2$text3"
      pmclist="$PMClist"
    elif [[ $tmode12 == "MASKBLENDL" ]] ; then
      if [[ $numop2 > 2 ]] ; then text21=", r$regsize" ; else text21="" ; fi
      if [[ $numimm2 > 0 ]] ; then text22=", $immvalue2" ; else text22="" ; fi
      text="$instruct1: Round trip latency through mask and vector register ($tmode12)\n"
      text="$text$instruct1 r$regsize{k}, r$regsize$text2$text3\n"
      text="$text$instruct2 k, r$regsize$text21$text22"
    elif [[ $tmode12 == "MASKBLENDT" ]] ; then
      text="$instruct1: Throughput ($tmode12)\n"
      text="$text$instruct1 r$regsize{k}, r$regsize$text2$text3$text5"
    elif [[ $tmode12 == "MASKL" ]] ; then
      text="$instruct1: Latency ($tmode12)\n"
      text="$text$instruct1 r$regsize$text4$text6, r$regsize$text2$text3$text5"
    elif [[ $tmode12 == "MASKT" ]] ; then
      text="$instruct1: Throughput ($tmode12)\n"
      text="$text$instruct1 r$regsize$text4$text6, r$regsize$text2$text3$text5"
    else
      echo "Unknown test mode $tmode12"
      continue
    fi

    tmode=$tmode12
    echo -e "$text" >> $outdir/$outfile

    # echo parameters=$parameters
    # run
    IFS=" "
    for pmc in $pmclist ; do
      compileAndRun
      if [[ $exitcode != 0 ]] ; then break ; fi
    done
    IFS=","
    # if [[ $exitcode != 0 ]] ; then continue 2 ; fi

  done

# DIV mode:
####################################################
elif [[ "$tmode" =~ ^DIV.+$ ]] ; then
  macros="fvecdiv.inc"  # file containing assembly macros
  # find submodes
  if [[ "$instruct" =~ vdiv.* ]] ; then
    instructt="vdiv"
  elif [[ "$instruct" =~ div.* ]] ; then
    instructt="div"
  elif [[ "$instruct" =~ vsqrt.* ]] ; then
    instructt="vsqrt"
  elif [[ "$instruct" =~ sqrt.* ]] ; then
    instructt="sqrt"
  elif [[ "$instruct" =~ v?rcp.* ]] ; then
    instructt="rcp"
  else
    echo Error unknown instruction for test mode $tmode: $instruct
  fi
  # instruction suffix
  suffixpos=$((${#instruct}-2))
  suff=${instruct:$suffixpos:2}
  # mask options
  if [[ $options =~ .*usezero.* ]] ; then usezero=1 ; else usezero=0 ; fi
  if [[ $options =~ .*mask=.* ]] ; then
    nmask=`echo $options | sed 's/.*mask=\([0-9-]*\).*/\1/i'`
    masksize=`echo $options | sed '/masksize=\([0-9]*\)/!d;s/.*masksize=\([0-9]*\).*/\1/i'`
    if [ -z $masksize ] ; then masksize=16 ; fi
  else
    nmask=""; masksize=0
  fi
  # parameters
  par1="-Dinstruct=$instruct -Dregsize=$regsize -Dnumop=$numop "
  par2="-Dusezero=$usezero -Dnmask=$nmask -Dmasksize=$masksize"
  par3="-Drepeat0=$repeat0 -Drepeat1=$repeat1 -Drepeat2=$repeat2"

  # L and T modes
  tmode2=${tmode:3}
  for ((j=0; j<${#tmode2}; j++)); do
    tmode12=${tmode2:$j:1}  # L or T
    
    # best and worst case
    for tcase in best worst ; do

      # parameters
      par4="-Dinstructt=$instructt -Dsuff=$suff -Dtmode=$tmode12 -Dtcase=$tcase"
      parameters="$par1 $par4 $par2 $par3"

      # pmc counters
      if [[ "$tmode12" =~ "T" && "$tcase" == "worst" ]] ; then 
        pmclist="$PMClist"
      else
        pmclist="$PMCs"
      fi

      # output text
      if [[ $tmode12 == "L" ]] ; then text1="Latency" ; else text1="Throughput" ; fi
      if [[ $numop > 2 ]] ; then text2=", r$regsize" ; else text2="" ; fi
      if [[ $masksize > 0 ]] ; then 
        text4="{k}"
        text5=" (mask = $nmask)"
      else 
        text4=""
        text5=""
      fi

      if [ $usezero -eq 1 ] ; then text6="{z}" ; else text6="" ; fi
      echo -e "\n$text1: $instruct r$regsize$text4$text6, r$regsize$text2 ; $text5 $tcase case" >> $outdir/$outfile
      # echo Parameters=$parameters

      IFS=" "
      for pmc in $pmclist ; do
        compileAndRun
        if [[ $exitcode != 0 ]] ; then break 3 ; fi
      done
      IFS=","
    done
  done

# GATHER modes:
####################################################
elif [[ "$tmode" =~ ^GATHER.*$ ]] ; then
  macros="gatherscatter.inc"  # file containing assembly macros
  # find submodes
  tmode=${tmode:7}
  if [ -z "$tmode" ] ; then tmode="stride" ; fi
  if [[ "$instruct" =~ [vp]+scatter.* ]] ; then scatter=1 ; else scatter=0; fi
  IFS="/"
  splitregsize=($regsize)
  regsize1=${splitregsize[0]}
  regsizei=${splitregsize[1]}
  IFS=" "
  if [ -z "$regsizei" ] ; then regsizei=$regsize1 ; fi
  # interpret instruction suffix
  ilength=${#instruct}
  datasize=${instruct:$((ilength-1)):1}  # last character
  if [[ ${instruct:$((ilength-2)):1} == "p" ]] ; then
    # floating point instruction suffix is: dps, dpd, qps, qpd
    indexsize=${instruct:$((ilength-3)):1}  # d or q
    if [[ $datasize == "s" ]] ; then
      datasize=32
    elif [[ $datasize == "d" ]] ; then
      datasize=64
    else
      echo "Unknown data size suffix $datasize"
      exit
    fi
  else  # integer suffix is: dd, dq, qd, qq
    indexsize=${instruct:$((ilength-2)):1}  # d or q
    if [[ $datasize == "d" ]] ; then
      datasize=32
    elif [[ $datasize == "q" ]] ; then
      datasize=64
    else
      echo "Unknown data size suffix $datasize"
      exit
    fi
  fi
  if [[ $indexsize == "d" ]] ; then
    indexsize=32
  elif [[ $indexsize == "q" ]] ; then
    indexsize=64
  else
    echo "Unknown index size suffix $indexsize"
    exit
  fi
  masktype=`echo $options | sed '/masktype=[kvKV]/!d;s/.*masktype=\([kvKV]\).*/\1/i'`
  if [ -z $masktype ] ; then
    if [[ $regsize1 > 256 || $regsizei > 256 ]] ; then masktype="k" ; else masktype="v" ; fi
  fi

  # Calculate number of elements
  delements=$(($regsize1/$datasize))
  ielements=$(($regsizei/$indexsize))
  if [[ $delements < $ielements ]] ; then numelements=$delements ; else numelements=$ielements ; fi
  # Calculate reduced number of elements if mask != -1
  if [[ ! -z  $immvalue ]] ; then
    m1=$(( (1<<$numelements)-1 ))
    m2=$(( $immvalue & $m1 ))  # isolate the relevant $numelements bits of the mask
    if [[ "$m2 < $m1" ]] ; then
      numelements=0   # the mask is not all 1's. Count the 1 bits:
      while [[ $m2 != 0 ]] ; do
        m2=$(( $m2 & ($m2-1) ))
        numelements=$(($numelements+1))
      done
    fi
  fi
  if [[ -z numop ]] ; then numop=3; fi

  # set parameters
  par1="-Dinstruct=$instruct -Dregsize=$regsize1 -Diregsize=$regsizei -Ddatasize=$datasize -Dindexsize=$indexsize -Dnumop=$numop"
  par2="-Dscatter=$scatter -Dmasktype=$masktype -Dmask=$immvalue -Dtmode=$tmode"
  parameters="$par1 $par2"
  #echo parameters=$parameters

  # Output text to file
  if [[ "$tmode" == "latency" ]] ; then
    text1="$instruct + move: Round trip latency";
  else
    text1="$instruct throughput. Test mode = $tmode";
  fi
  text2=", $numelements elements"
  if [ -z "$immvalue" ] ; then text3="" ; else text3=", mask value = $immvalue" ; fi
  text4="Data register size = $regsize1, index register size = $regsizei, mask type = $masktype"
  echo -e "\n$text1$text2$text3.\n$text4" >> $outdir/$outfile

  compileAndRun
  if [[ $exitcode != 0 ]] ; then continue ; fi


# user-defined mode:
####################################################
elif [[ "$tmode" =~ macros= ]] ; then
  # file containing assembly macros:
  macros=`echo $tmode | sed 's/macros=\([a-zA-Z0-9_]*.inc\)/\1/i'`

  # Output text to file
  echo -e "\n$instruct: Test defined by $macros."  >> $outdir/$outfile
  echo "   Register type = $regtype, size = $regsize"  >> $outdir/$outfile
  if [[ $numimm != 0 ]] ; then
    echo ", immediate operand = $immvalue"  >> $outdir/$outfile
  fi
  if [[ ! -z "$xoptions" ]] ; then
    echo "   Additional options: $xoptions"  >> $outdir/$outfile
  fi
  if [[ "$options" =~ "use32" ]] ; then
    echo "   32-bit mode"  >> $outdir/$outfile
  fi

  # Set parameters
  parameters="-Dinstruct=$instruct -Dnumop=$numop -Dregsize=$regsize -Dregtype=$regtype -Drepeat0=$repeat0 -Drepeat1=$repeat1 -Drepeat2=$repeat2 -Dnumimm=$numimm -Dimmvalue=$immvalue $xoptions"
  pmclist="$PMClist"

  # Run the test
  IFS=" "
  for pmc in $pmclist ; do
    compileAndRun
    if [[ $exitcode != 0 ]] ; then break 2 ; fi
  done
  IFS=","
  
####################################################

else
  echo unknown test mode $tmode
fi

IFS=","
done  # end of read loop

# reset field separator
IFS=$oldIFS

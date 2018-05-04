#!/usr/bin/env bash
# author:   liyong.zly
# file:     test.sh
# created:  Wed Mar  8 10:27:37 CST 2017
# modified: Wed Mar  8 10:27:37 CST 2017

# Description:
# This script is to run octane with different option and collect their
# ram consumption data, then compare their ram consumption, output as pdf format file
# NOTE!!!: Before running this script, please set the following variable correctly:
# OUT, OCTANE, TEST_ARCH_LIST, TEST_FEATURE, OUTPUT(optional)

# Specify out directory of v8
OUT=$HOME/work/YunOS/4.0/node/gitlab_node/test_node_aot/node_aot/deps/v8/out
# Where the octane testsuite source code is
OCTANE=/disk1/liyong.zly/work/YunOS/4.0/v8_related/V8/js_benchmarks/octane
# Test architecture list
TEST_ARCH_LIST="x64 ia32"
# Test Feature list
TEST_FEATURE="default ignition turbo"
# Script to collect ram consumption data for a command to be run
COLLECT="bash $OUT/../tools/js_aot/show_ram_usage.sh"
# Command line option to compile jso file
COMMON_COMPILE_OPTIONS="--random-seed=1234"
# Command line option to run d8
COMMON_OPTIONS="--random-seed=1234 --optimize_for_size"
# Specify where to save test result and temporary files
OUTPUT="/disk1/liyong.zly/tmp/Work/JSAOT/0308_1107"
CLEAN_TEMP="YES"

function list_all_jsfile() {
  res=`find ./ -name "*.js"`
  res_str=""
  for f in $res
  do
    t=`echo $f |sed 's/^\.\///g'`
    res_str="$res_str $t"
  done
  echo "$res_str"
}

function get_jsfile() {
  DESTPATH="$1"
  cd $DESTPATH
  list_all_jsfile
}

function compile() {
  TESTSUITE_NAME="$1"
  TESTSUITE_PATH="$2"
  ARCH="$3"
  JSO_NAME="${ABS_OUTPUT}/${TESTSUITE_NAME}-${ARCH}.jso"
  CMD="$OUT/${ARCH}.release/mkjso $COMMON_COMPILE_OPTIONS  --aot_out=$JSO_NAME"
  cd $TESTSUITE_PATH
  $CMD $SOURCE_JSFILES
  echo $JSO_NAME compiled
  cd -
}

function get_abs_path() {
  if [ -d "$1" ]; then
    cd $1 > /dev/null
    res="`pwd`"
    cd - > /dev/null
    echo "$res"
  else
    echo ""
  fi
}

if [ "$OUTPUT" = "" ]; then
  OUTPUT=`date +%m%d_%H%M`
fi

if [ ! -d $OUTPUT ]; then
  mkdir -p "$OUTPUT"
fi


function run_2() {
  ARCH="$1"
  TESTSUITE_NAME="$2"
  TESTSUITE_PATH="$3"
  CLASS="$4"
  JSO_NAME="${ABS_OUTPUT}/${TESTSUITE_NAME}-${ARCH}.jso"
  HP_NAME="${TESTSUITE_NAME}-${ARCH}-${CLASS}"
  shift 4
  CMD="$OUT/$ARCH.release/d8 $COMMON_OPTIONS $@ run.js"
  cd $TESTSUITE_PATH; echo $CMD | tee -a $LOG
  $CMD | tee -a $LOG & $COLLECT d8 $ABS_OUTPUT/$HP_NAME
  cd -
  echo $CMD_AOT | tee -a $LOG
  CMD_AOT="$CMD --aot_in=$JSO_NAME"
  cd ${TESTSUITE_PATH}
  $CMD_AOT | tee -a $LOG & $COLLECT d8 $ABS_OUTPUT/$HP_NAME-aot
  cd -
}

function run() {
  run_2 ia32 $@
  run_2 x64 $@
}

function gen() {
  NAME=$1
  shift
  CMD="python $OUT/../tools/js_aot/analysis_ram_usage.py --compare-private-dirty"
  $CMD $@
  mv PrivateDirty.pdf PrivateDirty-$NAME.pdf
}

# compile jso file
function compile_jso() {
  cd $OCTANE
  for ARCH in $TEST_ARCH_LIST
  do
    compile octane "$OCTANE" $ARCH
  done
  cd -
}

# Run benchmark and collect ram consumption data
function run_benchmark() {
  cd $OCTANE
  for FEATURE in $TEST_FEATURE
  do
    if [ X"$FEATURE" != "Xdefault" ]; then
      EXTRA_OPTION="--${FEATURE}"
    else
      EXTRA_OPTION=""
    fi
    run octane "$OCTANE" $FEATURE $EXTRA_OPTION
  done
  cd -
}

# Generate report for comparation
function gen_report() {
  cd $ABS_OUTPUT
  for FEATURE in $TEST_FEATURE
  do
    for ARCH in $TEST_ARCH_LIST
    do
      gen octane-${ARCH}-${FEATURE} octane-${ARCH}-${FEATURE}*.hp
    done
    gen octane-${FEATURE}-aot "*${FEATURE}-aot.hp"
    gen octane-${FEATURE}-noaot "*${FEATURE}.hp"
  done
}

ABS_OUTPUT=`get_abs_path "$OUTPUT"`
LOG=$ABS_OUTPUT/run.log
touch $LOG
SOURCE_JSFILES=`get_jsfile "$OCTANE"`

#compile_jso
#run_benchmark
#gen_report

if [ X"$CLEAN_TEMP" = "XYES" ]; then
  cd $ABS_OUTPUT
  rm -f *.hp *.jso *.log *.csv
  cd -
fi

echo "The result please check the pdf files in the directory: $ABS_OUTPUT"

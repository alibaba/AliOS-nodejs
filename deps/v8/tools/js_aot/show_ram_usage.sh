#!/usr/bin/env bash
# author:   liyong.zly
# file:     /disk1/liyong.zly/scripts/show_ram_usage.sh
# created:  Fri Jan 20 09:38:57 CST 2017
# modified: Fri Jan 20 09:38:57 CST 2017
# This script collects the data of memory consumption of the
# specified program name and all of children process of this
# specified program, including the following memory data:
# 1) RSS: Resident Set Size, the total amount of the pages for
#    a process that are actually loaded on the physical memory
# 2) PSS: Proportional Set Size, a indicator that can be used to
#    measure memory consumption of a single process. It is to
#    split the memory amount of shared pages evenly among the
#    processes that are using them.
# 3) SharedClean: the pages in the mapping that have been referenced
#    by this process and at least one other process, but not written
#    by any process.
# 4) SharedDirty: the pages in the mapping that have been referenced
#    by this process and at least one other process, and written by
#    at least one of those processes.
# 5) PrivateClean: the pages in the mapping that have been read and
#    not written by this process but not referenced by any other process
# 6) PrivateDirty: are the pages in the mapping that have been written by
#    this process but not referenced by any other process.

function get_machine_name() {
  mname=`uname -m`
  echo $mname|grep "arm*" > /dev/null
  isarm=$?
  if [ $isarm -eq 0 ]; then
    echo "arm"
  else
    echo "x86"
  fi
}

function list_offspring {
  tp=`pgrep -P $1`          # get childs pids of parent pid
  for i in $tp; do          # loop through childs
    if [ -z $i ]; then      # check if empty list
      break;                # if empty: exit
    else                    # else
      echo -n "$i "         # print childs pid
      list_offspring $i     # call list_offspring again with child pid as the parent
    fi;
  done
}

function get_mem_info_pid( ) {
  pid="$1"
  sample_count="$2"
  if [ -f /proc/$pid/smaps ]; then
    smaps_file="$DATADIR/__${pid}_smaps_${sample_count}"
    cat /proc/$pid/smaps > $smaps_file
    rss=$(awk 'BEGIN {i=0} /^Rss/ {i = i + $2} END {print i}' $smaps_file)
    pss=$(awk 'BEGIN {i=0} /^Pss/ {i = i + $2 + 0.5} END {print i}' $smaps_file)
    sc=$(awk 'BEGIN {i=0} /^Shared_Clean/ {i = i + $2} END {print i}' $smaps_file)
    sd=$(awk 'BEGIN {i=0} /^Shared_Dirty/ {i = i + $2} END {print i}' $smaps_file)
    pc=$(awk 'BEGIN {i=0} /^Private_Clean/ {i = i + $2} END {print i}' $smaps_file)
    pd=$(awk 'BEGIN {i=0} /^Private_Dirty/ {i = i + $2} END {print i}' $smaps_file)
    ppids=`list_offspring ${pid}`
    for ppid in ${ppids}
    do
      ppid_smaps_file="$DATADIR/__${pid}_smaps_${ppid}_${sample_count}"
      cat /proc/$ppid/smaps > $ppid_smaps_file
      ppid_rss=$(awk 'BEGIN {i=0} /^Rss/ {i = i + $2} END {print i}' $ppid_smaps_file)
      rss=$(awk "BEGIN {print $rss + $ppid_rss}")
      ppid_pss=$(awk 'BEGIN {i=0} /^Pss/ {i = i + $2 + 0.5} END {print i}' $ppid_smaps_file)
      pss=$(awk "BEGIN {print $pss + $ppid_pss}")
      ppid_sc=$(awk 'BEGIN {i=0} /^Shared_Clean/ {i = i + $2} END {print i}' $ppid_smaps_file)
      sc=$(awk "BEGIN {print $sc + $ppid_sc}")
      ppid_sd=$(awk 'BEGIN {i=0} /^Shared_Dirty/ {i = i + $2} END {print i}' $ppid_smaps_file)
      sd=$(awk "BEGIN {print $sd + $ppid_rss}")
      ppid_pc=$(awk 'BEGIN {i=0} /^Private_Clean/ {i = i + $2} END {print i}' $ppid_smaps_file)
      pc=$(awk "BEGIN {print $pc + $ppid_pc}")
      ppid_pd=$(awk 'BEGIN {i=0} /^Private_Dirty/ {i = i + $2} END {print i}' $ppid_smaps_file)
      pd=$(awk "BEGIN {print $pd + $ppid_pd}")
    done
    echo "$rss:$pss:$sc:$sd:$pc:$pd"
  fi
}

function get_pid() {
  program_name="$1"
  machine=`get_machine_name`
  if [ X"$machine" = "Xarm" ]; then
    OPTS=""
  else
    OPTS="-u $USER"
  fi
  ps -f ${OPTS} |grep "${program_name}" | grep -v "grep" | grep -v "$CUR_PROG" | awk -F " " '{print $2}'
}

function usage() {
  echo "show_ram_usage.sh \"PROG_TO_SHOW\" REPORT_FILE MAX_SAMPLE_DURATION(default: 0xffff seconds)"
  exit 1
}

PROG_NAME="$1"
CUR_PROG="$0"
if [ X"$PROG_NAME" == X -o $# -lt 2 ]; then
    usage
fi
report_file="$2"
if [ $# -eq 3 ]; then
  MAX_SAMPLE_DURATION=$3
else
  # If max sample duration is not specified, the default value
  # is 0xffff
  MAX_SAMPLE_DURATION=0xffff
fi
MACHINE="`get_machine_name`"
# time internal to check whether program has been started
WATCH_PROG_INTERNAL=10
# sample internal 100 microseconds, that is 0.1 second
SAMPLE_INTERNAL_UNIT=100
DATADIR="$HOME"
MACRO_SEC_PER_SEC=1000
MAX_SAMPLE_DURATION_IN_MS=`awk "BEGIN { print $MAX_SAMPLE_DURATION * $MACRO_SEC_PER_SEC}"`
if [ X"$MACHINE" = "Xarm" ]; then
  SLEEP="usleep"
  SAMPLE_INTERNAL=${SAMPLE_INTERNAL_UNIT}
  DATADIR="/tmp"
  MAX_SAMPLE_DURATION=${MAX_SAMPLE_DURATION_IN_MS}
else
  SAMPLE_INTERNAL=`awk "BEGIN { print $SAMPLE_INTERNAL_UNIT / $MACRO_SEC_PER_SEC}"`
  SLEEP="sleep"
  WATCH_PROG_INTERNAL=0.01
fi
# maximum sample count
MAX_SAMPLE_COUNT=$(( $(($MAX_SAMPLE_DURATION_IN_MS + $SAMPLE_INTERNAL_UNIT -1)) / $SAMPLE_INTERNAL_UNIT ))
sample_date=""
declare -a mem_info_array

function is_pid() {
  if [ "$1" -eq "$1" 2>/dev/null ]; then
    echo 0
  else
    echo 1
  fi
}

function does_exist() {
  if [ X"$1" = "X" ]; then
    return 1
  fi

  if ps -p "$1" > /dev/null
  then
    return 0
  else
    return 1
  fi
}

function generic_add() {
  echo `awk "BEGIN{print "$1" + "$2"}"`
}

function sample() {
  SAMPLED_PROG="$1"
  pid=`get_pid "$SAMPLED_PROG"`
  local sample_res="0:0:0:0:0:0"
  local display_flag=0
  while [ "`is_pid "${pid}"`" = "1" ]
  do
    if [ $display_flag -eq 0 ]; then
      echo "${SAMPLED_PROG} is not started, loop to wait it starts!"
      echo "If you want to quit, press Ctrl+C to stop!"
    else
      echo -n "."
    fi
    # sleep 10 microseconds to check whether the program is launched
    $SLEEP $WATCH_PROG_INTERNAL
    pid=`get_pid "$SAMPLED_PROG"`
    display_flag=1
  done
  echo -e "\n$SAMPLED_PROG has been started, begin to sample its memory usage"

  sample_date=`date`
  sample_time=0
  local idx=0
  while $(does_exist "${pid}") && [ $idx -lt $MAX_SAMPLE_COUNT ]
  do
    sample_res=`get_mem_info_pid $pid $idx`
    mem_info_array[$idx]="$sample_time:$sample_res"
    idx=`generic_add "$idx" 1`
    $SLEEP $SAMPLE_INTERNAL
    sample_time=`generic_add "$sample_time" "$SAMPLE_INTERNAL_UNIT"`
    sample_res="0:0:0:0:0"
    echo -n "."
  done
  SAMPLED_PID=${pid}
}

function print_result() {
  echo  "JOB \"$PROG_NAME\"" >> $report_file
  echo  "DATE \"$sample_date\"" >> $report_file
  echo "SAMPLE_UNIT \"$SAMPLE_INTERNAL_UNIT microseconds\"" >> $report_file
  echo 'VALUE_UNIT "kB"' >> $report_file
  #echo "Total sample counts: ${#mem_info_array[@]}"
  for ((i=0;i<${#mem_info_array[@]};i++))
  do
    one_sample=${mem_info_array[$i]}
    sample_time=`echo "$one_sample"|cut -d ':' -f1`
    rss=`echo "$one_sample"|cut -d ':' -f2 | awk '{printf "%d", $0}'`
    pss=`echo "$one_sample"|cut -d ':' -f3 | awk '{printf "%d", $0}'`
    sc=`echo "$one_sample"|cut -d ':' -f4 | awk '{printf "%d", $0}'`
    sd=`echo "$one_sample"|cut -d ':' -f5 |awk '{printf "%d", $0}'`
    pr_clean=`echo "$one_sample"|cut -d ':' -f6 | awk '{printf "%d", $0}'`
    pr_dirty=`echo "$one_sample"|cut -d ':' -f7 | awk '{printf "%d", $0}'`
    if [ X"$pss" = "X" ]; then
        continue
    fi
    echo "BEGIN_SAMPLE $sample_time" >> $report_file
    echo "RSS: $rss" >> $report_file
    echo "PSS: $pss" >> $report_file
    echo "SharedClean: $sc" >> $report_file
    echo "SharedDirty: $sd" >> $report_file
    echo "PrivateClean: $pr_clean" >> $report_file
    echo "PrivateDirty: $pr_dirty" >> $report_file
    echo "END_SAMPLE $sample_time" >> $report_file
  done
}
function sub_space() {
  echo "$1" |sed -e 's/[ ][ ]*/_/g' -e 's/[-][-]*/_/g' -e 's/\//_/g' -e 's/\./_/g'
}

function save_log_data() {
  log_file=`sub_space "$PROG_NAME"`
  pid_number="$1"
  tar cf "${log_file}.tar" `ls $DATADIR/__${pid_number}_smaps*`
  rm -f $DATADIR/__${pid_number}_smaps*
  echo "smaps saved in the compressed file ${log_file}.tar"
}

SAMPLED_PID=
sample "$PROG_NAME"
if [ -f "$report_file" ]; then
    echo "$report_file has been exist! Please specify another report file"
    exit 1
fi
suffix=${report_file##*.}
if [ X"$suffix" != "X.hp" ]; then
    report_file="$report_file.hp"
fi
touch $report_file
print_result
save_log_data "$SAMPLED_PID"

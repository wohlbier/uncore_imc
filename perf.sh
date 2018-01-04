#!/bin/bash -x
#
# Based on McCalpin's script at
# https://software.intel.com/en-us/forums/software-tuning-performance-optimization-platform-monitoring/topic/535072
#

PMU_BASE="uncore_imc_"
PMU_COUNT=2 # uncore counters on bdx are 0,1 4,5
EVT_CODE="0x04"
RD_UMASK="0x03"
WR_UMASK="0x0c"

i=0
while [ $i -lt $PMU_COUNT ] ; do
  ARGS="-e ${PMU_BASE}${i}/event=${EVT_CODE},umask=${RD_UMASK}/ $ARGS"
  ARGS="-e ${PMU_BASE}${i}/event=${EVT_CODE},umask=${WR_UMASK}/ $ARGS"
  ((i++))
done

i=0
while [ $i -lt $PMU_COUNT ] ; do
  c=$(( 4+$i ))
  ARGS="-e ${PMU_BASE}${c}/event=${EVT_CODE},umask=${RD_UMASK}/ $ARGS"
  ARGS="-e ${PMU_BASE}${c}/event=${EVT_CODE},umask=${WR_UMASK}/ $ARGS"
  ((i++))
done

echo $ARGS

#export LD_LIBRARY_PATH=/opt/intel/17/linux/compiler/lib/intel64:$LD_LIBRARY_PATH
perf stat -x , -a -A -C 0 $ARGS numactl -m 0 ./uncore_imc 100

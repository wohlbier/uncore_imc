#!/bin/bash

# Run this as
# % . ./tau.sh
# the first time to export the papi path.

# Run app as
# % tau numactl -C 0 -- ./uncore_imc 1000

rm -rf ./.tau

# Configure your project...
APPNAME=uncore_imc

# papi built with git checkout of libpfm4
PAPI_ROOT=${HOME}/devel/packages/spack/opt/spack/linux-rhel7-x86_64/gcc-6.1.0/papi-master-ashjfzmpqkbxa6hudklfxji7oybhufb6
export PATH=${PAPI_ROOT}/bin:${PATH}

# initialize tau commander project
tau init --application-name $APPNAME --target-name centennial --mpi F --papi=${PAPI_ROOT} --tau nightly
#--openmp

tau select profile

# debugging
#tau measurement edit profile --keep-inst-files

# asm kernel does not have any writes
tau measurement edit profile --source-inst manual --compiler-inst never \
--metrics \
PAPI_NATIVE:bdx_unc_imc0::UNC_M_CAS_COUNT:RD:cpu=0,\
PAPI_NATIVE:bdx_unc_imc1::UNC_M_CAS_COUNT:RD:cpu=0,\
PAPI_NATIVE:bdx_unc_imc4::UNC_M_CAS_COUNT:RD:cpu=0,\
PAPI_NATIVE:bdx_unc_imc5::UNC_M_CAS_COUNT:RD:cpu=0

# run complains about incompatible papi metrics, but generates results.

# use this in paraprof derived metric for bandwidth
#64*("PAPI_NATIVE:bdx_unc_imc0::UNC_M_CAS_COUNT:RD:cpu=0"+"PAPI_NATIVE:bdx_unc_imc1::UNC_M_CAS_COUNT:RD:cpu=0"+"PAPI_NATIVE:bdx_unc_imc4::UNC_M_CAS_COUNT:RD:cpu=0"+"PAPI_NATIVE:bdx_unc_imc5::UNC_M_CAS_COUNT:RD:cpu=0")/"TIME"

#!/bin/bash
#set -e
#set -x #echo on

function run_testsuite() {
  PLAT=$3

  if [ $OSTYPE == "msys" ]; then # Windows, MingW
    if [ $3 == "win32" ]; then
      PLAT="x86"
    fi
  fi

  if [ $1 == "epshell" ]; then # epshell
    ./public/bin/$2_$PLAT/eptest
  else
    exit 4;
  fi

  if [ $? -ne 0 ]; then
    echo "Unit Tests Failed"
    exit 3;
  fi
  echo "Unit Tests succeeded"
}

function run_shutdown_test() {
  PLAT=$3

  if [ $OSTYPE == "msys" ]; then # Windows, MingW
    if [ $3 == "win32" ]; then
      PLAT="x86"
    fi
  fi

  # TODO: Make this run on every config and platform
  if [[ $OSTYPE == "msys" && $2 == "Debug" && $3 == "x64" ]]; then

    if [[ -f MemoryReport_$2_$PLAT.txt ]]; then
	  rm MemoryReport_$2_$PLAT.txt
	fi

    if [ $1 == "epshell" ]; then # epshell
	  export PATH=$QTDIR/bin:$PATH
	  ./public/bin/$2_$PLAT/$1 shut_down_test
#     TODO test epviewer
#	  ./public/bin/$2_$PLAT/epviewer shut_down_test
	fi

	if [ $? -ne 0 ]; then
	  echo $1 "Failed to shutdown cleanly"
	  ERR=1
	else
	  ERR=0
	fi

    if [[ -f MemoryReport_$2_$PLAT.txt ]]; then
	  if [ $? -eq 0 ]; then # epshell leaked memory
	    echo "Memory leaks detected"
	    grep "normal block" MemoryReport_$2_$PLAT.txt
	    exit 3
	  fi
	fi
  fi
}

run_testsuite $1 $2 $3
#run_shutdown_test $1 $2 $3

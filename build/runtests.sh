#set -x #echo on
function run_testsuite() {
  if [ $OSTYPE == "msys" ]; then # Windows, MingW
    if [ $3 == "win32" ]; then
      PLAT="x86"
    else
      PLAT=$3
    fi
  else
    PLAT=""
  fi

  if [ $1 == "epshell" ]; then # epshell
    ./public/bin/$2_$PLAT/eptest
  elif [ $1 == "epviewer" ]; then # epviewer
    ./viewer/bin/$2_$PLAT/eptest
  else
    exit 4;
  fi

  if [ $? -ne 0 ]; then
    echo "Unit Tests Failed"
    exit 3;
  fi
  echo "Unit Tests succeeded"
}

function run_leak_test() {
  if [ $OSTYPE == "msys" ]; then # Windows, MingW
    if [ $3 == "win32" ]; then
      PLAT="x86"
    else
      PLAT=$3
    fi
  else
    PLAT=""
  fi

  # TODO: Make this run on every config and platform
  if [[ $OSTYPE == "msys" && $2 == "Debug" && $3 == "x64" ]]; then
    if [ $1 == "epshell" ]; then # epshell
	  ./public/bin/$2_$PLAT/$1 CITest
    elif [ $1 == "epviewer" ]; then # epviewer
	  ./viewer/bin/$2_$PLAT/$1 CITest
	fi
	grep -i "Detected memory leaks" MemoryReport_$2_$PLAT.txt
	if [ $? -eq 0 ]; then # epshell leaked memory
	  grep "normal block" MemoryReport_$2_$PLAT.txt
	  exit 3
	fi
  fi
}

run_testsuite $1 $2 $3
#run_leak_test() $1 $2 $3

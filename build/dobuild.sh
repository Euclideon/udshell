#!/bin/bash
#set -e
#set -x #echo on

if [ $OSTYPE == "msys" ]; then # Windows, MingW
  # These must be here until Runner specific variables are implemented
  if [ -z "$QTDIR" ]; then
    export QTDIR="C:/dev/Qt/5.6/msvc2015_64"
  fi

  # generate the project files
  if [ $1 == "epshell" ]; then # epshell
    bin/premake/premake5.exe vs2015
  elif [ $1 == "epviewer" ]; then # epviewer
    cd viewer
    ../bin/premake/premake5.exe vs2015
  else
    exit 4;
  fi
  if [ $? -ne 0 ]; then exit 4; fi

  if [[ $# -eq 4 && $4 == "clean" ]]; then
    "C:/Program Files (x86)/MSBuild/14.0/Bin/amd64/MSBuild.exe" $1.sln //p:Configuration=$2 //p:Platform=$3 //v:m //t:clean
  elif [ $# -eq 3 ]; then
    "C:/Program Files (x86)/MSBuild/14.0/Bin/amd64/MSBuild.exe" $1.sln //p:Configuration=$2 //p:Platform=$3 //v:m //m
  else
    exit 5
  fi

  if [ $? -ne 0 ]; then exit 5; fi
else
  # These must be here until Runner specific variables are implemented
  export QTDIR=~/dev/Qt/5.6/gcc_64
  export PATH=$QTDIR/bin:$PATH

  # This will need to use the $2 (configuration) and $3 (platform) variables somehow
  if [ $1 == "epshell" ]; then # epshell
    bin/premake/premake5 gmake
  elif [ $1 == "epviewer" ]; then # epviewer
    cd viewer
    ../bin/premake/premake5 gmake
  else
    exit 4;
  fi
  if [ $? -ne 0 ]; then exit 4; fi

  if [ $# -eq 4 ]; then
    if [ $4 == "clean" ]; then
      make config=${2,,} clean
    else
      exit 5
    fi
  elif [ $# -eq 3 ]; then
    make config=${2,,} -j4
  fi
  if [ $? -ne 0 ]; then exit 5; fi
fi

echo "Executing post-build..."
./build/postbuild.sh $1 $2 $3
PSTBR=$?
if [ $PSTBR -ne 0 ]; then
  exit $PSTBR
fi

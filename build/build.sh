#set -x #echo on
function execute_build() {
  if [ $OSTYPE == "msys" ]; then # Windows, MingW
    # These must be here until Runner specific variables are implemented
    if [ -z "$QTDIR" ]; then
      export QTDIR="C:/dev/Qt/5.4/msvc2013_64_opengl"
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
    export QTDIR=~/dev/Qt/5.4/gcc_64
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
}

function merge_master() {
  git merge master
  if [ $? -ne 0 ]; then
    echo "merged with master failed"
    git merge --abort
    exit 3;
  fi
  echo "merged successfully with master"
}

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
    ./bin/$2_$PLAT/eptest
  else
    exit 4;
  fi

  if [ $? -ne 0 ]; then
    echo "Unit Tests Failed"
    exit 3;
  fi
  echo "Unit Tests succeeded"
}

if [ $# -eq 0 ]; then # if no args just build shell debug x64
  git submodule update --init --recursive
  if [ $? -ne 0 ]; then exit 3; fi
  execute_build epshell Debug x64
elif [[ $# -eq 2 || $# -gt 4 ]]; then exit 2 #if we have an arg count of 2 or greater than 4 exit
elif [ $# -eq 1 ]; then
  if [ $1 == "all" ]; then
    git submodule update --init --recursive
    if [ $? -ne 0 ]; then exit 3; fi
    execute_build epshell Debug x64
    execute_build epshell Release x64
    execute_build epviewer Debug x64
    execute_build epviewer Release x64
    execute_build epviewer Debug win32
    execute_build epviewer Release win32
  elif [ $1 == "forceall" ]; then
    git submodule update --init --recursive
    if [ $? -ne 0 ]; then exit 3; fi
    ./build/build.sh epshell Debug x64
    ./build/build.sh epshell Release x64
    ./build/build.sh epviewer Debug x64
    ./build/build.sh epviewer Release x64
    ./build/build.sh epviewer Debug win32
    ./build/build.sh epviewer Release win32
  elif [ $1 == "clean" ]; then
    git submodule update --init --recursive
    if [ $? -ne 0 ]; then exit 3; fi
    execute_build epshell Debug x64 clean
    execute_build epshell Release x64 clean
    execute_build epviewer Debug x64 clean
    execute_build epviewer Release x64 clean
    execute_build epviewer Debug win32 clean
    execute_build epviewer Release win32 clean
  else
    exit 3
  fi
else
  git submodule foreach --recursive 'git clean -ffdx'
  git submodule update --init --recursive
  if [ $? -ne 0 ]; then exit 3; fi

  if [ $# -eq 3 ]; then
    execute_build $1 $2 $3
    run_testsuite $1 $2 $3
  else
    if [ $4 == "merge" ]; then
      merge_master
      execute_build $1 $2 $3
    else
      execute_build $1 $2 $3 $4
    fi
    run_testsuite $1 $2 $3
  fi
fi


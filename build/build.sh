#set -x #echo on
function execute_build() {
  ./build/dobuild.sh $1 $2 $3 $4
}

function run_testsuite() {
  ./build/runtests.sh $1 $2 $3
}

function merge_master() {
  git merge origin/master --no-commit
  if [ $? -ne 0 ]; then
    echo "merged with origin/master failed"
    git merge --abort
    exit 3;
  fi
  git submodule update --init --recursive
  echo "merged successfully with origin/master"
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

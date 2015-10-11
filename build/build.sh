if [ $# -eq 0 ]; then
	./build/build.sh epshell Debug x64;
	./build/build.sh epshell Release x64;
	./build/build.sh epviewer Debug x64;
	./build/build.sh epviewer Release x64;
	./build/build.sh epviewer Debug win32;
	./build/build.sh epviewer Release win32;
else
git submodule update --init --recursive
if [ $? -ne 0 ]; then exit 3; fi

	if [ $OSTYPE == "msys" ]; then # Windows, MingW
    if [ $1 == "epshell" ]; then # epshell
      ud/bin/premake/premake5.exe vs2013
    else
      if [ $1 == "epviewer" ]; then # epviewer
        cd viewer
        ../ud/bin/premake/premake5.exe vs2013 
      fi 
    fi
		if [ $? -ne 0 ]; then exit 4; fi

		"C:/Program Files (x86)/MSBuild/12.0/Bin/amd64/MSBuild.exe" $1.sln //p:Configuration=$2 //p:Platform=$3
		if [ $? -ne 0 ]; then exit 5; fi
	else
		# These must be here until Runner specific variables are implemented
		export QTDIR=/opt/Qt5.4.1/5.4/gcc_64/
		export PATH=$QTDIR/bin:$PATH

		# This will need to use the $2 (configuration) and $3 (platform) variables somehow
    if [ $1 == "epshell" ]; then # epshell
      ud/bin/premake/premake5 gmake
    else
      if [ $1 == "epviewer" ]; then # epviewer
        cd viewer
        ../ud/bin/premake/premake5.exe gmake 
      fi 
    fi
		if [ $? -ne 0 ]; then exit 4; fi

		make
		if [ $? -ne 0 ]; then exit 5; fi
	fi
fi

if [ $# -eq 0 ]; then
	cd ..
	./build/build.sh Debug x64;
	./build/build.sh Release x64;
else
	git submodule update --init --recursive
	if [ $? -ne 0 ]; then exit 3; fi

	if [ $OSTYPE == "msys" ]; then # Windows, MingW
		ud/bin/premake/premake5.exe vs2013
		if [ $? -ne 0 ]; then exit 4; fi

		"C:/Program Files (x86)/MSBuild/12.0/Bin/amd64/MSBuild.exe" epshell.sln //p:Configuration=$1 //p:Platform=$2
		if [ $? -ne 0 ]; then exit 5; fi
	else
		# These must be here until Runner specific variables are implemented
		export QTDIR=/opt/Qt5.4.1/5.4/gcc_64/
		export PATH=$QTDIR/bin:$PATH

		# This will need to use the $1 (configuration) and $2 (platform) variables somehow
		ud/bin/premake/premake5 gmake
		if [ $? -ne 0 ]; then exit 4; fi

		make
		if [ $? -ne 0 ]; then exit 5; fi
	fi
fi

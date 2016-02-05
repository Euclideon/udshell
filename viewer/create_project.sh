# prompt user of no option given on the command line
if [[ $1 ]] ; then
	ACTION=$1
else
	echo "Select the type of project you would like to create:"
	echo "1. GNU Makefile"
	echo "2. MonoDevelop"
	echo "3. CodeLite"
	echo "x. Eclipse (Work in progress - Coming soon!)"
	read -p "[1-3] " -n 1 -r
	echo
	ACTION=$REPLY
fi

if [ $OSTYPE == "msys" ]; then # Windows, MingW 
	PREMAKE=../bin/premake/premake5.exe
else
	PREMAKE=../bin/premake/premake5
fi

# parse string actions into numerics
if [[ $ACTION == "make" ]] || [[ $ACTION == "gmake" ]] ; then
	ACTION="1"
elif [[ $ACTION == "monodevelop" ]] ; then
	ACTION="2"
elif [[ $ACTION == "codelite" ]] ; then
	ACTION="3"
fi

# perform action
if [[ $ACTION == "1" ]] ; then
	echo "Creating GNU Makefile..."
	$PREMAKE gmake
elif [[ $ACTION == "2" ]] ; then
	echo "Creating MonoDevelop project..."
	$PREMAKE monodevelop
elif [[ $ACTION == "3" ]] ; then
	echo "Creating CodeLite project..."
	$PREMAKE codelite
else
	echo "Invalid input: "$REPLY
fi

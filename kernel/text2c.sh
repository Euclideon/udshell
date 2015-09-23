#!/bin/bash

input=$1
output=$2

output=`echo $output | sed 's|\\\\/|/|g'`
output="${output//\\//}"

path=$(dirname "$output")

filename=$(basename "$input")
varname="${filename//./_}"

mkdir -p $path

echo -n "const char *$varname = \"" > $output
cat $input | sed s/\\\\/\\\\\\\\/g | sed s/\"/\\\\\"/g | sed "s/$/\\\\n\\\\/" >> $output
echo "\";" >> $output

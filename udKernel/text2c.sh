#!/bin/bash

input=$1
output=$2
filename=$(basename "$input")
varname="${filename//./_}"

mkdir -p $(dirname "$output")

echo -n "const char *$varname = \"" > $output
cat $input | sed s/\\\\/\\\\\\\\/g | sed s/\"/\\\\\"/g | sed "s/$/\\\\n\\\\/" >> $output
echo "\";" >> $output

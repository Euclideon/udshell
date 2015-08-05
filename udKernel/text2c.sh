#!/bin/bash

input=$1
output=$2
filename=$(basename "$input")
varname="${filename//./_}"

echo -n "char *$varname = \"" > $output
cat $input | sed s/\\\\/\\\\\\\\/g | sed s/\"/\\\\\"/g | sed "s/$/\\\\/" >> $output
echo "\";" >> $output

#!/bin/bash

DIRNAME=`dirname $1`
FILENAME=`basename $1 ".cpp"`
echo $DIRNAME $FILENAME

sed 's|/\*|\n&|g;s|*/|&\n|g' $1 | sed '/\/\*/,/*\//d' | \
	grep BOOST_AUTO_TEST_CASE | \
	awk -F "[()]" '{ for (i=2; i<NF; i+=2) print $i }' | \
	sed -e 's/^ *//' -e 's/ *$//'| 
	xargs -n 1 -I "{}" echo "add_test(${FILENAME}_{} \${EXECUTABLE_OUTPUT_PATH}/$FILENAME -t {})" >> $DIRNAME/tests.cmake
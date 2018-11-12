#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

find $DIR -name 'tests.cmake' | xargs rm 
find $DIR -name '*.cpp' | xargs -n 1 $DIR/_update_tests.sh

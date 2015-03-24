#!/bin/sh
#
# make all cui applications by gcc
#

echo; echo % str2str/gcc
cd str2str/gcc
make $1
cd ../..

echo; echo % rtkrcv/gcc
cd rtkrcv/gcc
make $1
cd ../..


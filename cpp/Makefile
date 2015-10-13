# CXX=/usr/local/Cellar/gcc/4.9.2_1/bin/g++-4.9
CXX=g++

all:
	${CXX} --std=c++11 -I./gtest-1.7.0/include ./gtest-1.7.0/lib/.libs/libgtest.a xplor.cpp -o xplor
	./xplor

afunix:
	${CXX} --std=c++11 -I./gtest-1.7.0/include ./gtest-1.7.0/lib/.libs/libgtest.a afunix.cpp -o afunix
	./afunix

clean:
	rm -f xplor
	rm -f afunix

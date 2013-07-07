all: threadpool

CXX=/Users/spaans/xsrc/libcxx
threadpool: test.cpp threadpool.hh
	#clang++ -pthread -stdlib=libc++ -ggdb3 -msse4 -std=c++11 $< -o $@
	clang++  -ggdb3 -msse4 -std=c++11 -stdlib=libc++ -nostdinc++ -I$(CXX)/include -L$(CXX)/lib -O1 $< -o $@

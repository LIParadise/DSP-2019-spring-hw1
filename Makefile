.PHONY: all clean

CXX=gcc
CFLAGS+=-pthread -O2 -Wall --std=c99 -c
LDFLAGS+=-lm -pthread
# link to math library

all: train test acc
# type make/make all to compile test_hmm

clean: ctags
	$(RM) -f train train.o test test.o acc.o acc # type make clean to remove the compiled file

ctags:
	@ctags --language-force=c --tag-relative=yes -f ./tags $(wildcard *.c) $(wildcard *.h) 

train: train.c hmm.h calc.h utils.h
	$(CXX) train.c $(CFLAGS)
	$(CXX) $(LDFLAGS) -o train train.o

test: test.c hmm.h calc.h utils.h 
	$(CXX) test.c $(CFLAGS)
	$(CXX) $(LDFLAGS) -o test test.o

acc: acc.cc
	g++ --std=c++11 -O2 -o acc acc.cc

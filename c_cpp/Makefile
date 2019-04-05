.PHONY: all clean

CXX=clang
CFLAGS+=-g -pthread -O0 -Wall --std=c99 -c
LDFLAGS+=-lm -pthread
# link to math library

all: train test
# type make/make all to compile test_hmm

clean: ctags
	$(RM) -f train train.o test test_model.o  # type make clean to remove the compiled file

ctags:
	@ctags --language-force=c --tag-relative=yes -f ./tags $(wildcard *.c) $(wildcard *.h) 

train: train.c hmm.h calc.h utils.h
	$(CXX) train.c $(CFLAGS)
	$(CXX) $(LDFLAGS) -o train train.o

test: test_model.c hmm.h calc.h utils.h 
	$(CXX) test_model.c $(CFLAGS)
	$(CXX) $(LDFLAGS) -o test test_model.o

.PHONY: all clean

CXX=clang++
CFLAGS+=-O2 --std=c++11 -Wall
LDFLAGS+=-lm     # link to math library

TARGET=test_hmm

all: $(TARGET) 
# type make/make all to compile test_hmm

clean: ctags
	$(RM) -f $(TARGET)   # type make clean to remove the compiled file

ctags:
	@ctags --language-force=c --tag-relative=yes -f ./tags $(wildcard *.c) $(wildcard *.h) $(wildcard *.cc) $(wildcard *.cpp)

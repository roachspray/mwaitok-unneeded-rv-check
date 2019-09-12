LLVM_VER=6.0
LLVM_HOME=/usr/local/llvm60
LLVM_CONFIG?=$(LLVM_HOME)/bin/llvm-config


ifndef VERBOSE
QUIET:=@
endif

SRC_DIR?=$(PWD)/src

CXX=$(LLVM_HOME)/bin/clang++
CC=$(LLVM_HOME)/bin/clang-6.0
OPT=$(LLVM_HOME)/bin/opt
DIS=$(LLVM_HOME)/bin/llvm-dis
LNK=$(LLVM_HOME)/bin/llvm-link

LDFLAGS+=$(shell $(LLVM_CONFIG) --ldflags) 
LDFLAGS+=-shared -Wl,-O1

CXXFLAGS+=-I. -I$(shell $(LLVM_CONFIG) --includedir)
CXXFLAGS+=-std=c++11 -fPIC -fvisibility-inlines-hidden
CXXFLAGS+=-Wall -Wextra -g -Wno-unused-parameter -Wno-unused-variable

CPPFLAGS+=$(shell $(LLVM_CONFIG) --cppflags)
CPPFLAGS+=-I$(SRC_DIR)


PASS=WaitOKCheck.so
PASS_OBJECTS=WaitOKCheck.o

default: prep $(PASS)

prep:
	$(QUIET)mkdir -p built
	
%.o : $(SRC_DIR)/%.cpp
	@echo Compiling $*.cpp
	$(QUIET)$(CXX) -o built/$*.o -c $(CPPFLAGS) $(CXXFLAGS) $<

$(PASS) : $(PASS_OBJECTS)
	@echo Linking $@
	$(QUIET)$(CXX) -o built/$@ $(LDFLAGS) $(CXXFLAGS)  built/*.o

clean:
	$(QUIET)rm -rf built

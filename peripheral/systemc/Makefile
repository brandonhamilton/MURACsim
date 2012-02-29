#
# MURAC Auxiliary Architecture test platform Makefile
# Author: Brandon Hamilton <brandon.hamilton@gmail.com>


SYSTEMC_HOME   = /home/brandon/software/systemc/systemc-2.2.0
SYSTEMC_INC     = $(SYSTEMC_HOME)/include
SYSTEMC_LIB_DIR = $(SYSTEMC_HOME)/lib-linux

TLM_HOME       = /home/brandon/software/systemc/TLM-2009-07-15
TLM_INC         = $(TLM_HOME)/include/tlm

TLM_CFLAGS = -I$(TLM_INC) -I$(SYSTEMC_INC)
TLM_LDFLAGS = -lstdc++ -L$(SYSTEMC_LIB_DIR) -lsystemc

CC = gcc-3.4
CPP = g++
CPPFLAGS  = -g -Wno-long-long -Wall -DSC_INCLUDE_DYNAMIC_PROCESSES -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE

INTEGRATOR_SRCS = $(wildcard integrator/*.cpp)
INTEGRATOR_OBJS = $(foreach obj, $(INTEGRATOR_SRCS:.cpp=.o), $(obj))
INTEGRATOR_LIBS = $(foreach obj, $(INTEGRATOR_OBJS:.o=.so), $(obj))

INTEGRATOR_CPPFLAGS = -rdynamic
INTEGRATOR_LDFLAGS = -ldl

all: build_dir test_platform $(INTEGRATOR_LIBS)

clean:
	$(V) - rm -f build/*.o test_platform
	$(V) - rm -f $(INTEGRATOR_OBJS) $(INTEGRATOR_LIBS)
	$(V) - rm -rf build

build_dir:
	$(V) mkdir -p build

test_platform: build/testPlatform.o build/muracAA.o
	$(V) echo "Linking platform (TLM 2.0 test platform) $@"
	$(V) $(CPP) -o $@  $^ $(CPPFLAGS) $(TLM_CFLAGS) $(TLM_LDFLAGS) $(INTEGRATOR_LDFLAGS)

build/testPlatform.o: testPlatform.cpp
	$(V) echo "Compiling platform (TLM 2.0 test platform) $@"
	$(V) $(CPP) -c -o $@  $< $(CPPFLAGS) $(TLM_CFLAGS)

build/muracAA.o: muracAA.cpp
	$(V) echo "Compiling platform (TLM 2.0 test platform) $@"
	$(V) $(CPP) -c -o $@  $< $(CPPFLAGS) $(INTEGRATOR_CPPFLAGS) $(TLM_CFLAGS)

%.o: %.cpp
	$(V) echo "Compiling Murac AA integrator $@"
	$(V) $(CPP) $(CPPFLAGS) $(TLM_CFLAGS) -c -o $@ $^

$(INTEGRATOR_LIBS): $(INTEGRATOR_OBJS)
	$(V) echo "Linking Murac AA integrator libraries"
	$(V) $(CPP) -shared -o $@ $^ $(LDFLAGS)

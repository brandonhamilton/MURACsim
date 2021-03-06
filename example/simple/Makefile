#
# MURAC Simple example Makefile
# Author: Brandon Hamilton <brandon.hamilton@gmail.com>

######## TLM Support ############
SYSTEMC_HOME   = /home/brandon/software/systemc/systemc-2.2.0
TLM_HOME       = /home/brandon/software/systemc/TLM-2009-07-15

TLM_INC         = $(TLM_HOME)/include/tlm
ICM_INC         = $(IMPERAS_HOME)/ImpPublic/include/host
IMP_LIB_INC     = $(IMPERAS_HOME)/ImperasLib/source
IMPERAS_LIB     = $(IMPERAS_HOME)/bin/$(IMPERAS_ARCH)

SYSTEMC_INC     = $(SYSTEMC_HOME)/include
SYSTEMC_LIB_DIR = $(SYSTEMC_HOME)/lib-linux

TLM_MURAC		= peripheral/systemc

CFLAGS = $(SIM_CFLAGS) $(OTHER_CFLAGS)
LDFLAGS = $(OTHER_LDFLAGS)

ifeq ($(IMPERAS_ARCH),Linux)
  LDFLAGS += -Wl,--version-script=version.script
else
  LDFLAGS += export.def
endif

TLM_CFLAGS = -I$(TLM_INC) -I$(SYSTEMC_INC) -I$(IMP_LIB_INC)
TLM_LDFLAGS = -lstdc++ -L$(SYSTEMC_LIB_DIR) -lsystemc -L$(IMPERAS_LIB) -lRuntimeLoader

CC = gcc-3.4
CPP = g++-4.5

CPPFLAGS  = -g -Wno-long-long -Wall -DSC_INCLUDE_DYNAMIC_PROCESSES -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE

BUILD_FULL_CPU_MODEL=1

MURAC_EMBED_TOOL = murac_embed

PA_CROSS=ARM7
PA_SRC=$(wildcard pa/*.cpp)
PA_FILES=$(patsubst %.cpp,%.$(PA_CROSS).elf,$(PA_SRC))

AA_EMBED_DIR = aa/embed
AA_SRC   = $(wildcard aa/*.cpp)
AA_OBJS  = $(foreach obj, $(AA_SRC:.cpp=.o), $(obj))

AA_LIB_OBJS = aa/simple_lib.o
AA_LIB   = aa/simple_lib.so

AA_EMBED_OBJS = aa/simple.o
AA_EMBED = aa/simple.so
 
#
# Build the framework tools
#
all: $(MURAC_EMBED_TOOL) $(AA_EMBED_DIR) $(AA_LIB) $(AA_EMBED) $(PA_FILES)

$(MURAC_EMBED_TOOL): ../../framework/murac_embed.c
	$(V) $(CC) -o murac_embed ../../framework/murac_embed.c

$(AA_EMBED_DIR):
	- $(V) mkdir -p $(AA_EMBED_DIR) > /dev/null

%.o: %.cpp
	$(V) echo "Compiling Murac AA integrator $@"
	$(V) $(CPP) -fPIC -Os -c -o $@ $^ -I$(SYSTEMC_INC)

$(AA_LIB): $(AA_LIB_OBJS)
	$(V) echo "Linking Murac AA integrator libraries"
	$(V) $(CPP) -shared --no-undefined -o $@ $^ -L$(SYSTEMC_LIB_DIR) -lsystemc

$(AA_EMBED): $(AA_EMBED_OBJS)
	$(V) echo "Linking Murac AA integrator libraries"
	$(V) $(CPP) -shared --no-undefined -o $@ $^
	$(V) ./$(MURAC_EMBED_TOOL) $@ $(AA_EMBED_DIR)

-include $(IMPERAS_HOME)/bin/Makefile.include
-include $(IMPERAS_LIB)/CrossCompiler/$(PA_CROSS).makefile.include
ifeq ($($(PA_CROSS)_CC),)
    IMPERAS_ERROR := $(error "Error : $($(PA_CROSS)_CC) not set. Please check installation of toolchain for $(PA_CROSS)")
endif

%.$(PA_CROSS).elf: %.$(PA_CROSS).o
	$(V) echo "Linking $@"
	$(V) $(IMPERAS_LINK) -o $@ $< $(IMPERAS_LDFLAGS) -lm -export-dynamic

%.$(PA_CROSS).o: %.cpp
	$(V) echo "Compiling $<"
	$(V) $($(PA_CROSS)_CC) -c -o $@ $< $(OPTIMISATION)

clean:
	$(V) - rm -f $(MURAC_EMBED_TOOL)
	$(V) - rm -f $(AA_OBJS) $(AA_LIB) $(AA_EMBED) $(AA_EMBED_OBJS)
	$(V) - rm -rf $(AA_EMBED_DIR)
	$(V) - rm -f pa/*.$(PA_CROSS).elf pa/*.$(PA_CROSS).o

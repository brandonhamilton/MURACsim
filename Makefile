#
# MURAC Makefile
# Author: Brandon Hamilton <brandon.hamilton@gmail.com>

#
# Check for the OVP environment
#
IMPERAS_HOME := $(shell getpath.exe "$(IMPERAS_HOME)")
include $(IMPERAS_HOME)/bin/Makefile.include

ifndef IMPERAS_HOME
  IMPERAS_ERROR := $(error "IMPERAS_HOME not defined")
endif

NOVLNV=1

######## TLM Support ############
SYSTEMC_HOME   = /home/brandon/software/systemc/systemc-2.2.0
TLM_HOME       = /home/brandon/software/systemc/TLM-2009-07-15

TLM_INC         = $(TLM_HOME)/include/tlm
ICM_INC         = $(IMPERAS_HOME)/ImpPublic/include/host
IMP_LIB_INC     = $(IMPERAS_HOME)/ImperasLib/source
IMPERAS_LIB     = $(IMPERAS_HOME)/bin/$(IMPERAS_ARCH)

SYSTEMC_INC     = $(SYSTEMC_HOME)/include
SYSTEMC_LIB_DIR = $(SYSTEMC_HOME)/lib-linux
SYSTEMC_LIB     = $(SYSTEMC_LIB_DIR)/libsystemc.a

SHARED_SYSTEMC_LIBRARY = framework/libsystemc.so

TLM_MEM         = $(IMP_LIB_INC)/ovpworld.org/memory/ram/1.0/tlm2.0
TLM_MMC         = $(IMP_LIB_INC)/ovpworld.org/modelSupport/tlmMMC/1.0/tlm2.0
TLM_PERIPH      = $(IMP_LIB_INC)/ovpworld.org/modelSupport/tlmPeripheral/1.0/tlm2.0
TLM_PROC        = $(IMP_LIB_INC)/ovpworld.org/modelSupport/tlmProcessor/1.0/tlm2.0
TLM_PLAT        = $(IMP_LIB_INC)/ovpworld.org/modelSupport/tlmPlatform/1.0/tlm2.0

TLM_MURAC		= peripheral/systemc

TLM_OBJDIRSYS   = build/$(IMPERAS_ARCH)/tlm
TLM_ARCHIVE     = $(TLM_OBJDIRSYS)/tlmSupport.a

TLM_OBJECTS = $(TLM_OBJDIRSYS)/icmCpuManager.o \
		$(TLM_OBJDIRSYS)/tlmPlatform.o \
		$(TLM_OBJDIRSYS)/tlmProcessor.o \
		$(TLM_OBJDIRSYS)/tlmPeripheral.o \
		$(TLM_OBJDIRSYS)/tlmMemory.o \
		$(TLM_OBJDIRSYS)/muracAA.o

PSE_OBJDIRSYS         = build/$(IMPERAS_ARCH)/pse

CFLAGS = -m32 $(SIM_CFLAGS) $(OTHER_CFLAGS)
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

MURAC_EMBED_TOOL = framework/murac_embed

MAKEPASS?=0
ifeq ($(MAKEPASS),0)
all:
	$(V) echo "\n===== .:[ Building Framework Tools ]:. =====\n"
	$(MAKE) MAKEPASS=1
	$(V) echo "\n===== .:[ Building Peripherals ]:. =====\n"
	$(MAKE) MAKEPASS=2
	$(V) echo "\n===== .:[ Building Processor model and Platform ]:. =====\n"
	$(MAKE) MAKEPASS=3
	$(V) echo "\n===== .:[ Building Example applications ]:. =====\n"
	$(MAKE) MAKEPASS=4
	$(V) echo "\n"
clean:
	$(V) echo "\n===== .:[ Cleaning Framework Tools ]:. =====\n"
	$(MAKE) clean MAKEPASS=1
	$(V) echo "\n===== .:[ Cleaning Peripherals ]:. =====\n"
	$(MAKE) clean MAKEPASS=2
	$(V) echo "\n===== .:[ Cleaning Processor model and Platform ]:. =====\n"
	$(MAKE) clean MAKEPASS=3
	$(V) echo "\n===== .:[ Cleaning Example applications ]:. =====\n"
	$(MAKE) clean MAKEPASS=4
	$(V) echo "\n"

.PHONY: framework peripherals platform examples clean-framework clean-peripherals clean-platform clean-examples

framework:
	$(MAKE) MAKEPASS=1

peripherals:
	$(MAKE) MAKEPASS=2

platform:
	$(MAKE) MAKEPASS=3

examples:
	$(MAKE) MAKEPASS=4

clean-framework:
	$(MAKE) clean MAKEPASS=1

clean-peripherals:
	$(MAKE) clean MAKEPASS=2

clean-platform:
	$(MAKE) clean MAKEPASS=3

clean-examples:
	$(MAKE) clean MAKEPASS=4

endif

#
# Build the framework tools
#
ifeq ($(MAKEPASS),1)

all: $(MURAC_EMBED_TOOL) $(SHARED_SYSTEMC_LIBRARY)

$(MURAC_EMBED_TOOL): framework/murac_embed.c
	$(V) $(CC) -o framework/murac_embed framework/murac_embed.c

$(SHARED_SYSTEMC_LIBRARY): 
	$(V) $(CPP) -shared --no-undefined -o $(SHARED_SYSTEMC_LIBRARY) $(CPPFLAGS) -L$(SYSTEMC_LIB_DIR) -lsystemc

clean:
	$(V) - rm -f $(MURAC_EMBED_TOOL) $(SHARED_SYSTEMC_LIBRARY)

endif

#
# Build the peripherals
#
ifeq ($(MAKEPASS),2)

pse_check := $(wildcard $(IMPERAS_HOME)/lib/Linux/CrossCompiler/PSE.makefile.include)
ifeq ($(strip $(pse_check)),) 
   IMPERAS_ERROR := $(error "The PSE compiler is not installed")
endif

include $(IMPERAS_HOME)/lib/Linux/CrossCompiler/PSE.makefile.include

SRCS.c = $(wildcard peripheral/pse/muracAA/*.c)
OBJS.c = $(foreach obj, $(SRCS.c:.c=.o), $(obj))
OBJS = $(OBJS.c)

all: $(PSE_OBJDIRSYS)/muracAA.pse

$(PSE_OBJDIRSYS)/muracAA.pse: $(OBJS)
	$(V) echo "Cross Linking Peripheral $@"
	$(V) mkdir -p $(@D)
	$(V) $(PSE_LINK) -o $@ $^ $(CFLAGS) $(PSE_LDFLAGS) $(LDFLAGS)

%.o: %.c
	$(V) echo "Cross Compiling Peripheral $@"
	$(V) mkdir -p $(@D)
	$(V) $(PSE_CC) -g -gdwarf-2 -c -o $@ $< $(CFLAGS)

clean:
	$(V) - rm -f $(OBJS) $(PSE_OBJDIRSYS)/muracAA.pse

endif

#
# Build the processor
#
ifeq ($(MAKEPASS),3)

MURAC_PA_INSTRUCTIONS_FILE = "./library/muracPAinstructions.$(SHRSUF)"
MURAC_PA_MODEL_FILE        = "./processor/paModel.$(SHRSUF)"
MURAC_AA_FPGA_PSE_FILE	   = $(PSE_OBJDIRSYS)/muracAA.pse

ifeq ($(BUILD_FULL_CPU_MODEL),0)
all: $(TLM_OBJDIRSYS) $(MURAC_PA_INSTRUCTIONS_FILE) murac_sim murac_sim_fs
else
all: $(TLM_OBJDIRSYS) processor/paModel.$(SHRSUF) murac_sim murac_sim_fs
endif

SRCS.c = $(wildcard processor/pa/*.c)
SRCS.S = $(wildcard processor/pa/*.S)
OBJS.c = $(foreach obj, $(SRCS.c:.c=.o), $(obj))
OBJS.S = $(foreach obj, $(SRCS.S:.S=.o), $(obj))
OBJS = $(OBJS.c) $(OBJS.S)
SOLIB = processor/paModel.$(SHRSUF)

%.o: %.c
	$(V) echo "Compiling PA Processor $@"
	$(V) $(CC) $(CFLAGS) -fPIC -c -o $@ $^

$(SOLIB): $(OBJS)
	$(V) echo "Linking PA Processor"
	$(V) $(CPP) $(CFLAGS) -shared -o $@ $^ $(IMPERAS_VMISTUBS) $(LDFLAGS) -ldl

#
# Build the platform
#

arm_platform: platform/ovp_examples/arm_platform.o
	$(V) echo "Linking platform (PSE) $@"
	$(V) $(CC) -o $@  $^  $(CFLAGS) $(SIM_LDFLAGS)

arm_tlm_platform: platform/ovp_examples/arm_tlm_platform.o $(TLM_ARCHIVE)
	$(V) echo "Linking platform (TLM 2.0 PSE) $@"
	$(V) $(CPP) -o $@  $^ $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) $(SIM_LDFLAGS) $(TLM_LDFLAGS) 

murac_sim: platform/murac_sim.o $(TLM_ARCHIVE)
	$(V) echo "Linking platform (TLM 2.0 Simulator) $@"
	$(V) $(CPP) -Wl,-export-dynamic -o $@  $^ $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) $(SIM_LDFLAGS) $(TLM_LDFLAGS) -ldl 

murac_sim_fs: platform/murac_sim_fs.o $(TLM_ARCHIVE)
	$(V) echo "Linking platform (TLM 2.0 Full System Simulator) $@"
	$(V) $(CPP) -Wl,-export-dynamic -o $@  $^ $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) $(SIM_LDFLAGS) $(TLM_LDFLAGS) -ldl 

ifeq ($(BUILD_FULL_CPU_MODEL),0)
platform/ovp_examples/arm_platform.o: platform/ovp_examples/arm_platform.c
	$(V) echo "Compiling platform (PSE) $@"
	$(V) $(CC) -c -o $@  $< $(CFLAGS) \
	  -DINTECEPT_OBJECT_SUPPORTED="1" \
	  -DMURAC_PA_INSTRUCTIONS_FILE="\"${MURAC_PA_INSTRUCTIONS_FILE}\"" \
	  -DMURAC_AA_FPGA_PSE_FILE="\"${MURAC_AA_FPGA_PSE_FILE}\""

platform/ovp_examples/arm_tlm_platform.o: platform/ovp_examples/arm_tlm_platform.cpp
	$(V) echo "Compiling platform (TLM 2.0 PSE) $@"
	$(V) $(CPP) -c -o $@  $< $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) \
	  -DINTECEPT_OBJECT_SUPPORTED="1" \
	  -DMURAC_PA_INSTRUCTIONS_FILE="\"${MURAC_PA_INSTRUCTIONS_FILE}\"" \
	  -DMURAC_AA_FPGA_PSE_FILE="\"${MURAC_AA_FPGA_PSE_FILE}\""

platform/murac_sim.o: platform/murac_sim.cpp
	$(V) echo "Compiling platform (TLM 2.0 Simulator) $@"
	$(V) $(CPP) -c -o $@  $< $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) \
	  -DINTECEPT_OBJECT_SUPPORTED="1" \
	  -DMURAC_PA_INSTRUCTIONS_FILE="\"${MURAC_PA_INSTRUCTIONS_FILE}\"" \
	  -DSYSTEMC_LIB="\"${SHARED_SYSTEMC_LIBRARY}\""

platform/murac_sim_fs.o: platform/murac_sim_fs.cpp
	$(V) echo "Compiling platform (TLM 2.0 Full System Simulator) $@"
	$(V) $(CPP) -c -o $@  $< $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) \
	  -DINTECEPT_OBJECT_SUPPORTED="1" \
	  -DMURAC_PA_INSTRUCTIONS_FILE="\"${MURAC_PA_INSTRUCTIONS_FILE}\"" \
	  -DSYSTEMC_LIB="\"${SHARED_SYSTEMC_LIBRARY}\""

else
platform/ovp_examples/arm_platform.o: platform/ovp_examples/arm_platform.c
	$(V) echo "Compiling platform (PSE) $@"
	$(V) $(CC) -c -o $@  $< $(CFLAGS) \
	  -DMURAC_PA_MODEL_FILE="\"${MURAC_PA_MODEL_FILE}\"" \
	  -DMURAC_AA_FPGA_PSE_FILE="\"${MURAC_AA_FPGA_PSE_FILE}\""

platform/ovp_examples/arm_tlm_platform.o: platform/ovp_examples/arm_tlm_platform.cpp
	$(V) echo "Compiling platform (TLM 2.0 PSE) $@"
	$(V) $(CPP) -c -o $@  $< $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) \
	  -DMURAC_PA_MODEL_FILE="\"${MURAC_PA_MODEL_FILE}\"" \
	  -DMURAC_AA_FPGA_PSE_FILE="\"${MURAC_AA_FPGA_PSE_FILE}\""
	
platform/murac_sim.o: platform/murac_sim.cpp
	$(V) echo "Compiling platform (TLM 2.0 Simulator) $@"
	$(V) $(CPP) -c -o $@  $< $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) \
	  -DMURAC_PA_MODEL_FILE="\"${MURAC_PA_MODEL_FILE}\"" \
	  -DSYSTEMC_LIB="\"${SHARED_SYSTEMC_LIBRARY}\""	
	  
platform/murac_sim_fs.o: platform/murac_sim_fs.cpp
	$(V) echo "Compiling platform (TLM 2.0 Full System Simulator) $@"
	$(V) $(CPP) -c -o $@  $< $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) \
	  -DMURAC_PA_MODEL_FILE="\"${MURAC_PA_MODEL_FILE}\"" \
	  -DSYSTEMC_LIB="\"${SHARED_SYSTEMC_LIBRARY}\""	
	      
endif

$(TLM_OBJDIRSYS):
	- $(V) mkdir -p $(TLM_OBJDIRSYS) > /dev/null

$(TLM_OBJDIRSYS)/tlmProcessor.o: $(TLM_PROC)/tlmProcessor.cpp $(TLM_PROC)/tlmProcessor.hpp
	$(V) echo "Compiling $@"
	$(V) $(CPP) -c -o $@ $< $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) > /dev/null

$(TLM_OBJDIRSYS)/tlmPlatform.o: $(TLM_PLAT)/tlmPlatform.cpp $(TLM_PLAT)/tlmPlatform.hpp 
	$(V) echo "Compiling $@"
	$(V) $(CPP) -c -o $@ $< $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) > /dev/null

$(TLM_OBJDIRSYS)/tlmMemory.o:  $(TLM_MEM)/tlmMemory.cpp $(TLM_MEM)/tlmMemory.hpp
	$(V) echo "Compiling $@"
	$(V) $(CPP) -c -o $@ $< $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) > /dev/null

$(TLM_OBJDIRSYS)/icmCpuManager.o: $(IMPERAS_HOME)/ImpPublic/source/host/icm/icmCpuManager.cpp $(IMPERAS_HOME)/ImpPublic/include/host/icm/icmCpuManager.hpp 
	$(V) echo "Compiling $@"
	$(V) $(CPP) -c -o $@ $< $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) > /dev/null

$(TLM_OBJDIRSYS)/tlmPeripheral.o: $(TLM_PERIPH)/tlmPeripheral.cpp $(TLM_PERIPH)/tlmPeripheral.hpp
	$(V) echo "Compiling $@"
	$(V) $(CPP) -c -o $@ $< $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) > /dev/null

$(TLM_OBJDIRSYS)/muracAA.o: $(TLM_MURAC)/muracAA.cpp $(TLM_MURAC)/muracAA.hpp
	$(V) echo "Compiling $@"
	$(V) $(CPP) -c -o $@ $< $(CPPFLAGS) $(CFLAGS) $(TLM_CFLAGS) -export-dynamic -ldl > /dev/null

$(TLM_ARCHIVE): $(TLM_OBJECTS)
	$(V) ar r $@ $^ > /dev/null

#
# Build the enhanced instruction set
#
$(MURAC_PA_INSTRUCTIONS_FILE): library/muracPAinstructions.o
	$(V) echo "Linking MURAC Primary Architecture Instruction Set Library"
	$(V) $(CC) $(CFLAGS) --shared -o $@ $^ $(IMPERAS_VMISTUBS) $(LDFLAGS)

library/muracPAinstructions.o: library/muracPAinstructions.c
	$(V) echo "Compiling MURAC Primary Architecture Instruction Set Library $@"
	$(V) $(CC) $(CFLAGS) -c -o $@ $^

clean:
	$(V) - rm -f $(OBJS) $(SOLIB)
	$(V) - rm -rf build
	$(V) - rm -f platform/murac_sim.o murac_sim platform/murac_sim_fs.o murac_sim_fs
	$(V) - rm -f platform/ovp_examples/arm_platform.o arm_platform
	$(V) - rm -f platform/ovp_examples/arm_tlm_platform.o arm_tlm_platform
	$(V) - rm -f library/muracPAinstructions.o $(MURAC_PA_INSTRUCTIONS_FILE)

endif

#
# Build the Example applications
#
ifeq ($(MAKEPASS),4)

EXAMPLES      := simple matrix_multiply aes128 seqalign
EXAMPLE_DIRS  := $(addprefix example/,$(EXAMPLES))

all:
	for d in $(EXAMPLE_DIRS); \
        do \
          $(MAKE) --directory=$$d; \
        done

clean:
	for d in $(EXAMPLE_DIRS); \
        do  \
          $(MAKE) --directory=$$d clean; \
        done

endif


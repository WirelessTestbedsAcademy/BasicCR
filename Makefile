COMPONENT=SpectrumMonitorAppC
CFLAGS += -I$(TINYOS_OS_DIR)/lib/printf
CFLAGS += -DNEW_PRINTF_SEMANTICS
TOSMAKE_FIRST_FLAGS +=  -I./cc2420/ -pthread
PYTHON_MSG_FILES += CBSweepDataMsg.py CBChannelMaskMsg.py CBRepoQueryMsg.py printf.py
C_MSG_HEADER_FILES += CBSweepDataMsg.h CBChannelMaskMsg.h CBRepoQueryMsg.h
C_SENSORAPP += sensorapp
TOSMAKE_PRE_EXE_DEPS += $(PYTHON_MSG_FILES) $(C_MSG_HEADER_FILES) $(C_SENSORAPP)
C_INCLUDE += -I$(TINYOS_ROOT_DIR)/tools/tinyos/c/sf $(CFLAGS)
TOSMAKE_CLEAN_EXTRA += $(C_SENSORAPP) $(PYTHON_MSG_FILES) $(C_MSG_HEADER_FILES) *.o CB*.c *.pyc
TFLAGS = -I$(TINYOS_OS_DIR)/types -I$(TINYOS_OS_DIR)/platforms/null -I$(TINYOS_OS_DIR)/lib/serial -DNEW_PRINTF_SEMANTICS

ifdef FREQUENCY_VECTOR
CFLAGS += -DFREQUENCY_VECTOR=$(FREQUENCY_VECTOR)
endif


# python message classes:

CBSweepDataMsg.py: spectrummonitor.h
	nescc-mig python $(CFLAGS) -python-classname=CBSweepDataMsg spectrummonitor.h cb_sweep_data_msg -o $@

CBRepoQueryMsg.py: spectrummonitor.h
	nescc-mig python $(CFLAGS) -python-classname=CBRepoQueryMsg spectrummonitor.h cb_repo_query_msg -o $@

CBChannelMaskMsg.py: spectrummonitor.h
	nescc-mig python $(CFLAGS) -python-classname=CBChannelMaskMsg spectrummonitor.h cb_channelmask_msg -o $@

printf.py: $(TINYOS_ROOT_DIR)/tos/lib/printf/2_0_2/printf.h
	nescc-mig python $(CFLAGS) $(TFLAGS) -python-classname=printfMsg $(TINYOS_ROOT_DIR)/tos/lib/printf/printf.h printf_msg -o $@

# C message functions + sensorapp program:

$(C_SENSORAPP): $(C_MSG_HEADER_FILES) $(C_SENSORAPP).c
	gcc $(C_INCLUDE) -g -O2 $(C_SENSORAPP).c -c -o $(C_SENSORAPP).o
	gcc $(C_INCLUDE) -g -O2 CBSweepDataMsg.c -c -o CBSweepDataMsg.o
	gcc $(C_INCLUDE) -g -O2 CBRepoQueryMsg.c -c -o CBRepoQueryMsg.o
	gcc $(C_INCLUDE) -g -O2 CBChannelMaskMsg.c -c -o CBChannelMaskMsg.o
	gcc $(C_INCLUDE) -g -O2 -o $(C_SENSORAPP) $(C_SENSORAPP).o $(TINYOS_ROOT_DIR)/tools/tinyos/c/sf/libmote.a -lpthread CBSweepDataMsg.o CBChannelMaskMsg.o CBRepoQueryMsg.o

CBSweepDataMsg.h: spectrummonitor.h
	nescc-mig c $(CFLAGS) spectrummonitor.h cb_sweep_data_msg -o $@

CBRepoQueryMsg.h: spectrummonitor.h
	nescc-mig c $(CFLAGS) spectrummonitor.h cb_repo_query_msg -o $@

CBChannelMaskMsg.h: spectrummonitor.h
	nescc-mig c $(CFLAGS) spectrummonitor.h cb_channelmask_msg -o $@

TINYOS_ROOT_DIR?=../..
include $(TINYOS_ROOT_DIR)/Makefile.include


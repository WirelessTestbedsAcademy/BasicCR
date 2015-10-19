COMPONENT=SpectrumMonitorAppC
CFLAGS += -I$(TINYOS_OS_DIR)/lib/printf
CFLAGS += -DNEW_PRINTF_SEMANTICS
TOSMAKE_FIRST_FLAGS +=  -I./cc2420/ -pthread
PYTHON_MSG_FILES += CBSweepDataMsg.py printf.py
TOSMAKE_PRE_EXE_DEPS += $(PYTHON_MSG_FILES)
TOSMAKE_CLEAN_EXTRA += $(C_SENSORAPP) $(PYTHON_MSG_FILES) $(C_MSG_HEADER_FILES) *.o CB*.c *.pyc
TFLAGS = -I$(TINYOS_OS_DIR)/types -I$(TINYOS_OS_DIR)/platforms/null -I$(TINYOS_OS_DIR)/lib/serial -DNEW_PRINTF_SEMANTICS

ifdef FREQUENCY_VECTOR
CFLAGS += -DFREQUENCY_VECTOR=$(FREQUENCY_VECTOR)
endif


# python message classes:

CBSweepDataMsg.py: spectrummonitor.h
	nescc-mig python $(CFLAGS) -python-classname=CBSweepDataMsg spectrummonitor.h cb_sweep_data_msg -o $@

printf.py: $(TINYOS_ROOT_DIR)/tos/lib/printf/2_0_2/printf.h
	nescc-mig python $(CFLAGS) $(TFLAGS) -python-classname=printfMsg $(TINYOS_ROOT_DIR)/tos/lib/printf/printf.h printf_msg -o $@

# C message functions + sensorapp program:

TINYOS_ROOT_DIR?=../..
include $(TINYOS_ROOT_DIR)/Makefile.include


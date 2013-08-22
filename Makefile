COMPONENT=SpectrumMonitorAppC
CFLAGS += -I$(TOSDIR)/lib/printf
CFLAGS += -DNEW_PRINTF_SEMANTICS
CFLAGS +=  -I./cc2420/ -pthread -I../BANapp/
PYTHON_MSG_FILES = CBSweepDataMsg.py CBChannelMaskMsg.py CBRepoQueryMsg.py
C_MSG_HEADER_FILES = CBSweepDataMsg.h CBChannelMaskMsg.h CBRepoQueryMsg.h
C_TEST_PROGRAM = test
BUILD_EXTRA_DEPS += $(PYTHON_MSG_FILES) $(C_MSG_HEADER_FILES) $(C_TEST_PROGRAM)
C_INCLUDE = -I$(TOSDIR)/../support/sdk/c/sf $(CFLAGS)
CLEAN_EXTRA = test $(PYTHON_MSG_FILES) $(C_MSG_HEADER_FILES) *.o CB*.c *.pyc

ifdef FREQUENCY_VECTOR
CFLAGS += -DFREQUENCY_VECTOR=$(FREQUENCY_VECTOR)
endif


# python message classes:

CBSweepDataMsg.py: spectrummonitor.h
	mig python -target=$(PLATFORM) $(CFLAGS) -python-classname=CBSweepDataMsg spectrummonitor.h cb_sweep_data_msg -o $@

CBRepoQueryMsg.py: spectrummonitor.h
	mig python -target=$(PLATFORM) $(CFLAGS) -python-classname=CBRepoQueryMsg spectrummonitor.h cb_repo_query_msg -o $@

CBChannelMaskMsg.py: spectrummonitor.h
	mig python -target=$(PLATFORM) $(CFLAGS) -python-classname=CBChannelMaskMsg spectrummonitor.h cb_channelmask_msg -o $@


# C message functions + test program:

$(C_TEST_PROGRAM): $(C_MSG_HEADER_FILES) $(C_TEST_PROGRAM).c
	gcc $(C_INCLUDE) -g -O2 $(C_TEST_PROGRAM).c -c -o $(C_TEST_PROGRAM).o
	gcc $(C_INCLUDE) -g -O2 CBSweepDataMsg.c -c -o CBSweepDataMsg.o
	gcc $(C_INCLUDE) -g -O2 CBRepoQueryMsg.c -c -o CBRepoQueryMsg.o
	gcc $(C_INCLUDE) -g -O2 CBChannelMaskMsg.c -c -o CBChannelMaskMsg.o
	gcc $(C_INCLUDE) -g -O2 -o $(C_TEST_PROGRAM) $(C_TEST_PROGRAM).o $(TOSDIR)/../support/sdk/c/sf/libmote.a CBSweepDataMsg.o CBChannelMaskMsg.o CBRepoQueryMsg.o

CBSweepDataMsg.h: spectrummonitor.h
	mig c -target=$(PLATFORM) $(CFLAGS) spectrummonitor.h cb_sweep_data_msg -o $@

CBRepoQueryMsg.h: spectrummonitor.h
	mig c -target=$(PLATFORM) $(CFLAGS) spectrummonitor.h cb_repo_query_msg -o $@

CBChannelMaskMsg.h: spectrummonitor.h
	mig c -target=$(PLATFORM) $(CFLAGS) spectrummonitor.h cb_channelmask_msg -o $@


include $(MAKERULES)

COMPONENT=SpectrumMonitorAppC
CFLAGS +=  -I./cc2420/
BUILD_EXTRA_DEPS = CBSweepDataMsg.py CBChannelMaskMsg.py CBRepoQueryMsg.py

ifdef FREQUENCY_VECTOR
CFLAGS += -DFREQUENCY_VECTOR=$(FREQUENCY_VECTOR)
endif


CBSweepDataMsg.py: spectrummonitor.h
	mig python -target=$(PLATFORM) $(CFLAGS) -python-classname=CBSweepDataMsg spectrummonitor.h cb_sweep_data_msg -o $@

CBRepoQueryMsg.py: spectrummonitor.h
	mig python -target=$(PLATFORM) $(CFLAGS) -python-classname=CBRepoQueryMsg spectrummonitor.h cb_repo_query_msg -o $@

CBChannelMaskMsg.py: spectrummonitor.h
	mig python -target=$(PLATFORM) $(CFLAGS) -python-classname=CBChannelMaskMsg spectrummonitor.h cb_channelmask_msg -o $@


include $(MAKERULES)

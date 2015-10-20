COMPONENT=NodeIfC
CFLAGS += -DCC2420_NO_ACKNOWLEDGEMENTS
CFLAGS += -DCC2420_NO_ADDRESS_RECOGNITION
CFLAGS += -DTASKLET_IS_TASK
CFLAGS += -I$(TINYOS_OS_DIR)/lib/printf
CFLAGS += -DCC2420_DEF_RFPOWER=31
CFLAGS += -DTOSH_DATA_LENGTH=114
# CFLAGS += -DCC2420_CCA_OVERRIDE

TOSMAKE_PRE_EXE_DEPS += radio_conf_msg.py radio_msg.py
TOSMAKE_CLEAN_EXTRA += RssiMsg.py radio_conf_msg.py radio_msg.py *.pyc


radio_conf_msg.py: NodeIfMessages.h
	nescc-mig -o $@ $(CFLAGS) python -python-classname=radio_conf_msg $< radio_conf_msg
	nescc-ncg $(CFLAGS) python -python-classname=NodeIfPckType $< $< >> $@

radio_msg.py: NodeIfMessages.h
	nescc-mig -o $@ $(CFLAGS) python -python-classname=radio_msg $< radio_msg
	nescc-ncg $(CFLAGS) python -python-classname=NodeIfPckType $< $< >> $@

TINYOS_ROOT_DIR?=../../..
include $(TINYOS_ROOT_DIR)/Makefile.include


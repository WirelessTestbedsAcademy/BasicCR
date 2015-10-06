COMPONENT=NodeIfC
CFLAGS += -DCC2420_NO_ACKNOWLEDGEMENTS
CFLAGS += -DCC2420_NO_ADDRESS_RECOGNITION
CFLAGS += -DTASKLET_IS_TASK
CFLAGS += -I$(TINYOS_OS_DIR)/lib/printf
#CFLAGS += -DNEW_PRINTF_SEMANTICS
CFLAGS += -DCC2420_DEF_RFPOWER=31

TOSMAKE_PRE_EXE_DEPS += NodeIfMsg.py
TOSMAKE_CLEAN_EXTRA += RssiMsg.py NodeIfMsg.py *.pyc


NodeIfMsg.py: NodeIfMessages.h
	nescc-mig -o $@ python -python-classname=NodeIfMsg $< NodeIfMsg
	nescc-ncg       python -python-classname=NodeIfPckType $< $< >> $@


TINYOS_ROOT_DIR?=../../..
include $(TINYOS_ROOT_DIR)/Makefile.include


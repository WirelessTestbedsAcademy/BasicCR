
#if !defined(PLATFORM_TELOSB)
#error MUST BE TELOSB PLATFORM!
#endif
#include "printf.h"

#include "spectrummonitor.h"

configuration SpectrumMonitorAppC{
}
implementation {
  components MainC, new SpectrumMonitorC(MODE_MAX) as App, LedsC;
  components PrintfC;
  components SerialStartC;

  App.Boot -> MainC;
  App.Leds -> LedsC;

  components SerialActiveMessageC as AM;
  App.SerialControl -> AM;
  App.SendSweepData -> AM.AMSend[AM_CB_SWEEP_DATA_MSG];
  App.SerialPacket -> AM;

  components new QueueC(message_t*,MSG_QUEUE_SIZE), new PoolC(message_t,MSG_QUEUE_SIZE);
  App.Queue -> QueueC;
  App.Pool -> PoolC;

  components CC2420ControlC;
  App.SpiResource -> CC2420ControlC;
  App.CC2420Power -> CC2420ControlC;

  components CC2420TransmitC;
  App.TxControl -> CC2420TransmitC;
  App.CC2420Tx -> CC2420TransmitC;

  components CC2420ReceiveC;
  App.RxControl -> CC2420ReceiveC;
  App.CC2420Rx -> CC2420ReceiveC;

  components new Alarm32khz32C();
  MainC.SoftwareInit -> Alarm32khz32C;
  App.Alarm -> Alarm32khz32C;

}


/*
 * Copyright (c) 2007, Technische Universitaet Berlin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the Technische Universitaet Berlin nor the names
 *   of its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * - Revision -------------------------------------------------------------
 * $Revision: 1.4 $
 * $Date: 2006/12/12 18:23:06 $
 * @author: Jan Hauer <hauer@tkn.tu-berlin.de>
 * ========================================================================
 */

interface CC2420Tx {

  /* first byte is the length by */
  async command error_t loadTXFIFO(uint8_t *data);
  async event void loadTXFIFODone(uint8_t *data, error_t error );

  /* send will always result in a sendDone */
  async command void send();
  async event void sendDone(uint8_t *data, uint16_t time, error_t error);

  /* sendImmediate will *not* result in a sendDone !!! */
  async command void sendImmediate();

  /** MUST only be called after loadTXFIFODone */ 
  async command error_t cancel();
  event void cancelDone(error_t error);

  async command bool cca();
  async command bool rxOn();
  async command error_t rssi(int8_t *rssi);
  async command bool rfOff();
  async command void lockChipSpi();
  async command void unlockChipSpi();

  /**
   * Returns the result of the callee calling SpiResource.immediateRequest().
   *
   * @return Result of SpiResource.immediateRequest()
   */
  async command error_t immediateSpiRequest();
  async command error_t releaseSpi();

  /**
   * Modify the contents of a packet. This command can only be used
   * when an SFD capture event for the sending packet is signalled.
   *
   * @param offset in the message to start modifying.
   * @param buf to data to write
   * @param len of bytes to write
   * @return SUCCESS if the request was accepted, FAIL otherwise.
   */
  async event void transmissionStarted ( uint8_t *data );
  async event void transmittedSFD ( uint32_t time, uint8_t *data );
  async command error_t modify( uint8_t offset, uint8_t* buf, uint8_t len );

}


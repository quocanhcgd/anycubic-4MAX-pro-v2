/**
 * DWIN TFT Support for Anycubic i3 Mega and 4Max Pro
 * Based on the work of Christian Hopp and David Ramiro.
 * Copyright (c) 2020 by Jonas Plamann <https://github.com/Poket-Jony>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "../../../../inc/MarlinConfigPre.h"

#include <inttypes.h>
#include <avr/pgmspace.h>

#include "Stream.h"

#define  FORCE_INLINE __attribute__((always_inline)) inline

struct ring_buffer;

class DwinTFTSerialClass : public Stream
{
  private:
    ring_buffer *_rx_buffer;
    ring_buffer *_tx_buffer;
    volatile uint8_t *_ubrrh;
    volatile uint8_t *_ubrrl;
    volatile uint8_t *_ucsra;
    volatile uint8_t *_ucsrb;
    volatile uint8_t *_ucsrc;
    volatile uint8_t *_udr;
    uint8_t _rxen;
    uint8_t _txen;
    uint8_t _rxcie;
    uint8_t _udrie;
    uint8_t _u2x;
    bool transmitting;
  public:
    DwinTFTSerialClass(ring_buffer *rx_buffer, ring_buffer *tx_buffer,
      volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
      volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
      volatile uint8_t *ucsrc, volatile uint8_t *udr,
      uint8_t rxen, uint8_t txen, uint8_t rxcie, uint8_t udrie, uint8_t u2x);
    void begin(unsigned long);
    void begin(unsigned long, uint8_t);
    void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool();
};

// Define config for Serial.begin(baud, config);
#define SERIAL_5N1 0x00
#define SERIAL_6N1 0x02
#define SERIAL_7N1 0x04
#define SERIAL_8N1 0x06
#define SERIAL_5N2 0x08
#define SERIAL_6N2 0x0A
#define SERIAL_7N2 0x0C
#define SERIAL_8N2 0x0E
#define SERIAL_5E1 0x20
#define SERIAL_6E1 0x22
#define SERIAL_7E1 0x24
#define SERIAL_8E1 0x26
#define SERIAL_5E2 0x28
#define SERIAL_6E2 0x2A
#define SERIAL_7E2 0x2C
#define SERIAL_8E2 0x2E
#define SERIAL_5O1 0x30
#define SERIAL_6O1 0x32
#define SERIAL_7O1 0x34
#define SERIAL_8O1 0x36
#define SERIAL_5O2 0x38
#define SERIAL_6O2 0x3A
#define SERIAL_7O2 0x3C
#define SERIAL_8O2 0x3E

#if defined(UBRR3H)
  extern DwinTFTSerialClass DwinTFTSerial;
#endif

extern void serialEventRun(void) __attribute__((weak));

#define DWIN_TFT_SERIAL_PROTOCOL(x) (DwinTFTSerial.print(x))
#define DWIN_TFT_SERIAL_PROTOCOL_F(x,y) (DwinTFTSerial.print(x,y))
#define DWIN_TFT_SERIAL_PROTOCOLPGM(x) (DwinTFTSerialPrintPGM(PSTR(x)))
#define DWIN_TFT_SERIAL_PROTOCOL_P(x) (DwinTFTSerialPrintPGM(x))
#define DWIN_TFT_SERIAL_(x) (DwinTFTSerial.print(x),DwinTFTSerial.write('\n'))
#define DWIN_TFT_SERIAL_PROTOCOLLN(x) (DwinTFTSerial.print(x),DwinTFTSerial.write('\r'),DwinTFTSerial.write('\n'))
#define DWIN_TFT_SERIAL_PROTOCOLLNPGM(x) (DwinTFTSerialPrintPGM(PSTR(x)),DwinTFTSerial.write('\r'),DwinTFTSerial.write('\n'))
#define DWIN_TFT_SERIAL_PROTOCOLLN_P(x) (DwinTFTSerialPrintPGM(x),DwinTFTSerial.write('\r'),DwinTFTSerial.write('\n'))

#define DWIN_TFT_SERIAL_START() (DwinTFTSerial.write('\r'),DwinTFTSerial.write('\n'))
#define DWIN_TFT_SERIAL_CMD_SEND(x) (DwinTFTSerialPrintPGM(PSTR(x)),DwinTFTSerial.write('\r'),DwinTFTSerial.write('\n'))
#define DWIN_TFT_SERIAL_ENTER() (DwinTFTSerial.write('\r'),DwinTFTSerial.write('\n'))
#define DWIN_TFT_SERIAL_SPACE() (DwinTFTSerial.write(' '))

const char newErr[] PROGMEM = "ERR ";
const char newSucc[] PROGMEM = "OK";

#define DWIN_TFT_SERIAL_ERROR_START (DwinTFTSerialPrintPGM(newErr))
#define DWIN_TFT_SERIAL_ERROR(x) DWIN_TFT_SERIAL_PROTOCOL(x)
#define DWIN_TFT_SERIAL_ERRORPGM(x) DWIN_TFT_SERIAL_PROTOCOLPGM(x)
#define DWIN_TFT_SERIAL_ERRORLN(x) DWIN_TFT_SERIAL_PROTOCOLLN(x)
#define DWIN_TFT_SERIAL_ERRORLNPGM(x) DWIN_TFT_SERIAL_PROTOCOLLNPGM(x)

//##define DWIN_TFT_SERIAL_ECHO_START (DwinTFTSerialPrintPGM(newSucc))
#define DWIN_TFT_SERIAL_ECHOLN(x) DWIN_TFT_SERIAL_PROTOCOLLN(x)
#define DWIN_TFT_SERIAL_SUCC_START (DwinTFTSerialPrintPGM(newSucc))
#define DWIN_TFT_SERIAL_ECHOPAIR(name,value) (serial_echopair_P(PSTR(name),(value)))
#define DWIN_TFT_SERIAL_ECHOPGM(x) DWIN_TFT_SERIAL_PROTOCOLPGM(x)
#define DWIN_TFT_SERIAL_ECHO(x) DWIN_TFT_SERIAL_PROTOCOL(x)

FORCE_INLINE void DwinTFTSerialPrintPGM(const char *str)
{
  char ch=pgm_read_byte(str);
  while(ch)
  {
    DwinTFTSerial.write(ch);
    ch=pgm_read_byte(++str);
  }
}

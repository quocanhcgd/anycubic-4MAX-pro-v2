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

#include "../../../../inc/MarlinConfigPre.h"

#if ENABLED(DWIN_TFT)

#include "Arduino.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../../MarlinCore.h"
#include "../../../../core/language.h"
#include "../../../../core/macros.h"
#include "../../../../core/serial.h"
#include "../../../../gcode/queue.h"
#include "../../../../feature/emergency_parser.h"
#include "../../../../feature/pause.h"
#include "../../../../libs/buzzer.h"
#include "../../../../module/planner.h"
#include "../../../../module/printcounter.h"
#include "../../../../module/stepper.h"
#include "../../../../module/temperature.h"
#include "../../../../sd/cardreader.h"
#include "../../ui_api.h"

#if ENABLED(POWER_LOSS_RECOVERY)
  #include "../../../../feature/power_loss_recovery.h"
#endif

#include "DwinTFT.h"
#include "DwinTFTSerial.h"
#include "DwinTFTCommand.h"

char _conv[8];

char *itostr2(const uint8_t &x)
{
  //sprintf(conv,"%5.1f",x);
  int xx=x;
  _conv[0]=(xx/10)%10+'0';
  _conv[1]=(xx)%10+'0';
  _conv[2]=0;
  return _conv;
}

#ifndef ULTRA_LCD
#define DIGIT(n) ('0' + (n))
#define DIGIMOD(n, f) DIGIT((n)/(f) % 10)
#define RJDIGIT(n, f) ((n) >= (f) ? DIGIMOD(n, f) : ' ')
#define MINUSOR(n, alt) (n >= 0 ? (alt) : (n = -n, '-'))

char* itostr3(const int x)
{
  int xx = x;
  _conv[4] = MINUSOR(xx, RJDIGIT(xx, 100));
  _conv[5] = RJDIGIT(xx, 10);
  _conv[6] = DIGIMOD(xx, 1);
  return &_conv[4];
}

// Convert signed float to fixed-length string with 023.45 / -23.45 format
char *ftostr32(const float &x)
{
  long xx = x * 100;
  _conv[1] = MINUSOR(xx, DIGIMOD(xx, 10000));
  _conv[2] = DIGIMOD(xx, 1000);
  _conv[3] = DIGIMOD(xx, 100);
  _conv[4] = '.';
  _conv[5] = DIGIMOD(xx, 10);
  _conv[6] = DIGIMOD(xx, 1);
  return &_conv[1];
}
#endif

DwinTFTClass DwinTFT;

DwinTFTClass::DwinTFTClass()
{
  
}

void DwinTFTClass::init()
{
  DwinTFTSerial.begin(DWIN_TFT_BAUDRATE);
  //DWIN_TFT_SERIAL_START();
  DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_MAINBOARD_RESET);
  DWIN_TFT_SERIAL_ENTER();
  ExtUI::delay_ms(10);
  DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_READY);
  DWIN_TFT_SERIAL_ENTER();

  #if PIN_EXISTS(PS_ON)
    pinMode(PS_ON_PIN, OUTPUT);
    WRITE(PS_ON_PIN, HIGH);
  #endif

  #if ENABLED(SDSUPPORT) && PIN_EXISTS(SD_DETECT)
    pinMode(SD_DETECT_PIN, INPUT);
    WRITE(SD_DETECT_PIN, HIGH);
  #endif

  #if PIN_EXISTS(LED)
    pinMode(LED_PIN, OUTPUT);
    WRITE(LED_PIN, LOW);
  #endif

  //activate auto shutdown
  DwinTFT.gcodeNow_P(DWIN_TFT_GCODE_INACTIVITY_ON);

  #ifdef STARTUP_CHIME
    buzzer.tone(250, 554); // C#5
    buzzer.tone(250, 740); // F#5
    buzzer.tone(250, 554); // C#5
    buzzer.tone(500, 831); // G#5
  #endif
}

void DwinTFTClass::kill()
{
  DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_KILL);
  DWIN_TFT_SERIAL_ENTER();
  #ifdef DWIN_TFT_DEBUG
    SERIAL_ECHOLNPGM("TFT Serial Debug: Kill command... J11");
  #endif
  PowerDown();
}

void DwinTFTClass::tick()
{
  const millis_t ms = millis();
  static millis_t nextUpdateCheckMs = 0;

  if (ELAPSED(ms, nextUpdateCheckMs)) {
    nextUpdateCheckMs = ms + DWIN_TFT_UPDATE_INTERVAL_MS;
    loop();
  }
}

void DwinTFTClass::loop()
{
  DwinTFTCommand.loop();
}

void DwinTFTClass::filamentRunout(const ExtUI::extruder_t extruder)
{
  buzzer.tone(200, 1567);
  buzzer.tone(200, 1174);
  buzzer.tone(200, 1567);
  buzzer.tone(200, 1174);
  buzzer.tone(2000, 1567);

  DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_FILAMENT_RUNOUT); //J15 FILAMENT LACK
  DWIN_TFT_SERIAL_ENTER();
  DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_FILAMENT_RUNOUT); // send 2 times
  DWIN_TFT_SERIAL_ENTER();
  #ifdef DWIN_TFT_DEBUG
    SERIAL_ECHOLNPGM("TFT Serial Debug: Filament runout... J15");
  #endif
}

void DwinTFTClass::gcodeNow_P(PGM_P const gcode)
{
  ExtUI::injectCommands_P(gcode);
}

void DwinTFTClass::gcodeQueue_P(PGM_P const gcode)
{
  queue.enqueue_now_P(gcode);
}

void DwinTFTClass::gcodeQueue(const char* gcode)
{
  queue.enqueue_one_now(gcode);
}

void DwinTFTClass::onMediaInserted()
{
  if(ExtUI::isMediaInserted()) {
    DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_INSERTED); // J00 SD Card inserted
    DWIN_TFT_SERIAL_ENTER();
    #ifdef DWIN_TFT_DEBUG
      SERIAL_ECHOLNPGM("TFT Serial Debug: SD card inserted... J00");
    #endif
  }
}

void DwinTFTClass::onMediaError()
{
  if(!ExtUI::isMediaInserted()) {
    DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_NOT_INSERTED); // J01 SD Card error
    DWIN_TFT_SERIAL_ENTER();
    #ifdef DWIN_TFT_DEBUG
      SERIAL_ECHOLNPGM("TFT Serial Debug: SD card error... J02");
    #endif
  }
}

void DwinTFTClass::onMediaRemoved()
{
  if(!ExtUI::isMediaInserted()) {
    DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_REMOVED); // J01 SD Card removed
    DWIN_TFT_SERIAL_ENTER();
    #ifdef DWIN_TFT_DEBUG
      SERIAL_ECHOLNPGM("TFT Serial Debug: SD card removed... J01");
    #endif
  }
}

void DwinTFTClass::onStatusChanged(const char * const msg)
{
  if(msg) {
    if(strcasecmp_P(msg, PSTR("Reheating...")) == 0) {
      DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_HOTEND_HEATING_START);
      DWIN_TFT_SERIAL_ENTER();
    }
    #ifdef DWIN_TFT_DEBUG
      SERIAL_ECHOLNPAIR("TFT Serial Debug: statusChanged: ", msg);
    #endif
  }
}

void DwinTFTClass::onUserConfirmRequired(const char * const msg)
{
  #if HAS_RESUME_CONTINUE
    if(msg) {
      if(strcasecmp_P(msg, PSTR("HeaterTimeout")) == 0) {
        DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_HOTEND_HEATING_START);
        DWIN_TFT_SERIAL_ENTER();
        waitForUserConfirm();
      } else if(strcasecmp_P(msg, PSTR("Reheat finished.")) == 0) {
        DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_HOTEND_HEATING_END);
        DWIN_TFT_SERIAL_ENTER();
        waitForUserConfirm();
      } else if(strcasecmp_P(msg, PSTR("Load Filament")) == 0) {
        waitForUserConfirm();
      } else if(strcasecmp_P(msg, PSTR("Filament Purge Running...")) == 0) {
        waitForUserConfirm();
      } else if(strcasecmp_P(msg, PSTR("Nozzle Parked")) == 0) {
        waitForUserConfirm();
      } else if(strcasecmp_P(msg, PSTR("MMU2 Eject Recover")) == 0) {
        waitForUserConfirm();
      } else if(strcasecmp_P(msg, PSTR("M43 Wait Called")) == 0) {
        waitForUserConfirm();
      } else {
        ExtUI::setUserConfirmed();
      }
      #ifdef DWIN_TFT_DEBUG
        SERIAL_ECHOLNPAIR("TFT Serial Debug: userConfirmRequired: ", msg);
      #endif
    }
  #endif
}

bool DwinTFTClass::isWaitingForUserConfirm()
{
  #if HAS_RESUME_CONTINUE
    return wait_for_user;
  #else
    return false;
  #endif
}

void DwinTFTClass::waitForUserConfirm()
{
  #if HAS_RESUME_CONTINUE
    buzzer.tone(250, 554); // C#5
    buzzer.tone(500, 831); // G#5
    DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_PRINT_PAUSE_REQ);
    DWIN_TFT_SERIAL_ENTER();
  #endif
}

void DwinTFTClass::PowerDown()
{
  //marlin PSU_OFF() don't work
  #if PIN_EXISTS(PS_ON)
    #ifdef DWIN_TFT_DEBUG
      SERIAL_ECHOLNPGM("TFT Serial Debug: Power down");
    #endif
    for(unsigned char i = 0; i < 3; i++)
    {
      WRITE(PS_ON_PIN, LOW);
      delay(10);
      WRITE(PS_ON_PIN, HIGH);
      delay(10);
    }
  #endif
}

void DwinTFTClass::onMeshUpdate(const int8_t xpos, const int8_t ypos, const float zval)
{
  //caution: called at startup
}

void DwinTFTClass::onPidTuning(const ExtUI::result_t rst)
{
  #if HAS_PID_HEATING
    buzzer.tone(250, 831); // G#5
    buzzer.tone(500, 554); // C#5
  #endif
}

void DwinTFTClass::onPrintTimerStarted()
{

}

void DwinTFTClass::onPrintTimerPaused()
{

}

void DwinTFTClass::onPrintTimerStopped()
{
  DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_PRINT_FINISHED);// J14 print done
  DWIN_TFT_SERIAL_ENTER();
  #ifdef DWIN_TFT_DEBUG
    SERIAL_ECHOLNPGM("TFT Serial Debug: SD print done... J14");
  #endif
}

#endif
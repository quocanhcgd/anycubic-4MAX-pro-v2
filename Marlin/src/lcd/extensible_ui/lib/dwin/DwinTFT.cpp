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
  delay(10);
  DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_READY);
  DWIN_TFT_SERIAL_ENTER();

  #if ENABLED(SDSUPPORT) && PIN_EXISTS(SD_DETECT)
    pinMode(SD_DETECT_PIN, INPUT);
    WRITE(SD_DETECT_PIN, HIGH);
  #endif

  #if PIN_EXISTS(LED)
    pinMode(LED_PIN, OUTPUT);
    WRITE(LED_PIN, LOW);
  #endif

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
}

void DwinTFTClass::loop()
{
  const millis_t ms = millis();
  static millis_t next10HzCheck = 0;
  if (ELAPSED(ms, next10HzCheck)) {
    loop10Hz();
    next10HzCheck = ms + 100UL;
  }
  DwinTFTCommand.loop();
}

void DwinTFTClass::loop10Hz()
{
    checkPowerOff();
}

void DwinTFTClass::filamentRunout(const ExtUI::extruder_t extruder)
{
    buzzer.tone(200, 1567);
    buzzer.tone(200, 1174);
    buzzer.tone(200, 1567);
    buzzer.tone(200, 1174);
    buzzer.tone(2000, 1567);
    
    DWIN_TFT_SERIAL_PROTOCOLPGM("J15"); //J15 FILAMENT LACK
    DWIN_TFT_SERIAL_ENTER();
    #ifdef DWIN_TFT_DEBUG
      SERIAL_ECHOLNPGM("TFT Serial Debug: Filament runout... J15");
    #endif
}

void DwinTFTClass::checkPowerOff()
{
    if(autoPowerOff && !ExtUI::isMoving() && !ExtUI::isPrinting() && 
      ExtUI::getActualTemp_celsius(ExtUI::extruder_t::E0) < 100
    ) {
      ExtUI::injectCommands_P(DWIN_TFT_GCODE_M81);
    }
}

void DwinTFTClass::setCaseLight(bool state)
{
  caseLight = state;
  if(state) {
        WRITE(LED_PIN, HIGH);
  } else {
      WRITE(LED_PIN, LOW);
  }
}

bool DwinTFTClass::getCaseLight()
{
  return caseLight;
}

#endif
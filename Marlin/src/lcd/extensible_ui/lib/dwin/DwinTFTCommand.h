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

#include <stdio.h>
#include "../../../../inc/MarlinConfig.h"

#include "DwinTFT.h"

enum DwinTFTCommandsRx : uint8_t {
  DWIN_TFT_RX_GET_HOTEND_TEMP = 0,
  DWIN_TFT_RX_GET_HOTEND_TARGET_TEMP = 1,
  DWIN_TFT_RX_GET_HOTBED_TEMP = 2,
  DWIN_TFT_RX_GET_HOTBED_TARGET_TEMP = 3,
  DWIN_TFT_RX_GET_FAN_SPEED = 4,
  DWIN_TFT_RX_GET_CURRENT_COORDINATES = 5,
  DWIN_TFT_RX_GET_SD_CARD_PRINT_STATUS = 6,
  DWIN_TFT_RX_GET_PRINTING_TIME = 7,
  DWIN_TFT_RX_GET_SD_CARD_LIST = 8,
  DWIN_TFT_RX_SD_CARD_PRINT_PAUSE = 9,
  DWIN_TFT_RX_SD_CARD_PRINT_RESUME = 10,
  DWIN_TFT_RX_SD_CARD_PRINT_STOP = 11,
  DWIN_TFT_RX_KILL = 12,
  DWIN_TFT_RX_SD_CARD_FILE_SELECT = 13,
  DWIN_TFT_RX_SD_CARD_PRINT_START = 14,
  DWIN_TFT_RX_OUTAGE_RESUME = 15,
  DWIN_TFT_RX_SET_HOTEND_TEMP = 16,
  DWIN_TFT_RX_SET_HOTBED_TEMP = 17,
  DWIN_TFT_RX_SET_FAN_SPEED = 18,
  DWIN_TFT_RX_STEPPER_DRIVER_STOP = 19,
  DWIN_TFT_RX_GET_PRINT_SPEED = 20,
  DWIN_TFT_RX_HOME_ALL = 21,
  DWIN_TFT_RX_MOVE = 22,
  DWIN_TFT_RX_PREHEAT_PLA = 23,
  DWIN_TFT_RX_PREHEAT_ABS = 24,
  DWIN_TFT_RX_COOLDOWN = 25,
  DWIN_TFT_RX_SD_CARD_FILE_REFRESH = 26,
  DWIN_TFT_RX_ADJUST_SERVO = 27,
  DWIN_TFT_RX_TEST_FILAMENT = 28,
  DWIN_TFT_RX_SET_Z_PROBE_OFFSET = 29,
  DWIN_TFT_RX_LEVELING_ASSIST = 30,
  DWIN_TFT_RX_SET_Z_OFFSET = 31,
  DWIN_TFT_RX_LEVELING_CLEAN_BEEP_FLAG = 32,
  DWIN_TFT_RX_GET_VERSION_INFO = 33,
  DWIN_TFT_RX_RESET_MAINBOARD = 40,
  DWIN_TFT_RX_AUTO_POWER_OFF = 41,
  DWIN_TFT_RX_SET_CASE_LED = 42
};

const char DWIN_TFT_TX_MAINBOARD_RESET[] PROGMEM = "J17";
const char DWIN_TFT_TX_READY[] PROGMEM = "J12";
const char DWIN_TFT_TX_KILL[] PROGMEM = "J11";
const char DWIN_TFT_TX_SD_CARD_NOT_INSERTED[] PROGMEM = "J02";
const char DWIN_TFT_TX_SD_CARD_OPEN_SUCCESS[] PROGMEM = "J20";
const char DWIN_TFT_TX_SD_CARD_OPEN_FAILED[] PROGMEM = "J21";
const char DWIN_TFT_TX_SD_CARD_FILE_LIST_START[] PROGMEM = "FN ";
const char DWIN_TFT_TX_SD_CARD_FILE_LIST_END[] PROGMEM = "END";

class DwinTFTCommandClass {
public:
  char *TFTstrchr_pointer;
  DwinTFTCommandClass();
  void handleCommand(DwinTFTCommandsRx command);
  float codeValue();
  bool codeSeen(char code);
  void loop();

private:
  char TFTcmdbuffer[DWIN_TFT_BUFSIZE][DWIN_TFT_MAX_CMD_SIZE];
  int TFTbuflen = 0;
  int TFTbufindr = 0;
  int TFTbufindw = 0;
  char serial3_char;
  int serial3_count = 0;
  void receiveCommands();
  void sendGetHotendTemp();
  void sendGetHotendTargetTemp();
  void sendGetHotbedTemp();
  void sendGetHotbedTargetTemp();
  void sendGetFanSpeed();
  void sendGetCurrentCoordinates();
  void sendGetSDCardPrintingStatus();
  void sendGetPrintingTime();
  void sendGetSDCardList();
  void sendSDCardPause();
  void sendSDCardStop();
  void sendSDCardStart();
  void sendSetHotendTemp();
  void sendSetHotbedTemp();
  void sendSetFanSpeed();
  void sendStopStepperDriver();
  void sendGetPrintSpeed();
  void sendHomeAll();
  void sendMove();
  void sendPreheatPLA();
  void sendPreheatABS();
  void sendCooldown();
  void sendSDCardRefresh();
  void sendFilamentTest();
  void sendGetVersionInfo();
  void sendResetMainboard();
  void sendAutoPowerOff();
  void sendSetCaseLight();
};

extern DwinTFTCommandClass DwinTFTCommand;

/*********************
 * extui_example.cpp *
 *********************/

/****************************************************************************
 *   Written By Marcio Teixeira 2018 - Aleph Objects, Inc.                  *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation, either version 3 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   To view a copy of the GNU General Public License, go to the following  *
 *   location: <http://www.gnu.org/licenses/>.                              *
 ****************************************************************************/

#include "../inc/MarlinConfigPre.h"

#if ENABLED(DWIN_TFT)

#include "extensible_ui/ui_api.h"
#include "extensible_ui/lib/dwin/DwinTFT.h"

// To implement a new UI, complete the functions below and
// read or update Marlin's state using the methods in the
// ExtUI methods in "../ui_api.h"
//
// Although it may be possible to access other state
// variables from Marlin, using the API here possibly
// helps ensure future compatibility.

namespace ExtUI {
  void onStartup() {
    DwinTFT.init();
  }
  void onIdle() {
    DwinTFT.tick();
  }
  void onPrinterKilled(PGM_P const error, PGM_P const component) {
    DwinTFT.kill();
  }
  void onMediaInserted() {
    DwinTFT.onMediaInserted();
  }
  void onMediaError() {
    DwinTFT.onMediaError();
  }
  void onMediaRemoved() {
    DwinTFT.onMediaRemoved();
  }
  void onPlayTone(const uint16_t frequency, const uint16_t duration) {
    #if ENABLED(SPEAKER)
      // fix speaker for extui
      ::tone(BEEPER_PIN, frequency, duration);
    #endif
  }
  void onPrintTimerStarted() {}
  void onPrintTimerPaused() {}
  void onPrintTimerStopped() {}
  void onFilamentRunout(const extruder_t extruder) {
    DwinTFT.filamentRunout(extruder);
  }
  void onUserConfirmRequired(const char * const msg) {
    DwinTFT.onUserConfirmRequired(msg);
  }
  void onStatusChanged(const char * const msg) {
    DwinTFT.onStatusChanged(msg);
  }
  void onFactoryReset() {}

  void onStoreSettings(char *buff) {
    // This is called when saving to EEPROM (i.e. M500). If the ExtUI needs
    // permanent data to be stored, it can write up to eeprom_data_size bytes
    // into buff.

    // Example:
    //  static_assert(sizeof(myDataStruct) <= ExtUI::eeprom_data_size);
    //  memcpy(buff, &myDataStruct, sizeof(myDataStruct));
  }

  void onLoadSettings(const char *buff) {
    // This is called while loading settings from EEPROM. If the ExtUI
    // needs to retrieve data, it should copy up to eeprom_data_size bytes
    // from buff

    // Example:
    //  static_assert(sizeof(myDataStruct) <= ExtUI::eeprom_data_size);
    //  memcpy(&myDataStruct, buff, sizeof(myDataStruct));
  }

  void onConfigurationStoreWritten(bool success) {
    // This is called after the entire EEPROM has been written,
    // whether successful or not.
  }

  void onConfigurationStoreRead(bool success) {
    // This is called after the entire EEPROM has been read,
    // whether successful or not.
  }

  void onMeshUpdate(const int8_t xpos, const int8_t ypos, const float zval) {
    // This is called when any mesh points are updated
  }

  #if ENABLED(POWER_LOSS_RECOVERY)
    void OnPowerLossResume() {
      // Called on resume from power-loss
    }
  #endif

  #if HAS_PID_HEATING
    void OnPidTuning(const result_t rst) {
      // Called for temperature PID tuning result
    }
  #endif
}

#endif

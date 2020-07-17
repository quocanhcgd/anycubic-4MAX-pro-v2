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

#include "../inc/MarlinConfigPre.h"

#if ENABLED(DWIN_TFT)

#include "extui/ui_api.h"
#include "extui/lib/dwin/DwinTFT.h"

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
      // fix speaker for ExtUI
      ::tone(BEEPER_PIN, frequency, duration);
    #endif
  }
  void onPrintTimerStarted() {
    DwinTFT.onPrintTimerStarted();
  }
  void onPrintTimerPaused() {
    DwinTFT.onPrintTimerPaused();
  }
  void onPrintTimerStopped() {
    DwinTFT.onPrintTimerStopped();
  }
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
    DwinTFT.onMeshUpdate(xpos, ypos, zval);
  }

  #if ENABLED(POWER_LOSS_RECOVERY)
    void onPowerLossResume() {
      // Called on resume from power-loss
    }
  #endif

  #if HAS_PID_HEATING
    void onPidTuning(const result_t rst) {
      DwinTFT.onPidTuning(rst);
    }
  #endif
}

#endif

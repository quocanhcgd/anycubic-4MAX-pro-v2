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

#if BOTH(DWIN_TFT, SDSUPPORT)

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

#include "DwinTFTFileBrowser.h"
#include "DwinTFTSerial.h"
#include "DwinTFTCommand.h"

static ExtUI::FileList fileList;

DwinTFTFileBrowserClass DwinTFTFileBrowser;

DwinTFTFileBrowserClass::DwinTFTFileBrowserClass()
{
  
}

void DwinTFTFileBrowserClass::reset()
{
    selectedFilename[0] = 0;
    selectedDirectory[0] = 0;
}

void DwinTFTFileBrowserClass::listFiles()
{
    reset();
    if(!ExtUI::isMediaInserted()) {
        DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_NOT_INSERTED); //no sd card found
        DWIN_TFT_SERIAL_ENTER();
    } else {
        uint16_t filenumber;
        if(DwinTFTCommand.codeSeen('S')) {
            filenumber = DwinTFTCommand.codeValue();
            //@todo handle current filenumber
        }

        DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_FILE_LIST_START); // Filelist start
        DWIN_TFT_SERIAL_ENTER();

        uint16_t fileCount = fileList.count();
        for(uint16_t pos = 0; pos < fileCount; pos++) {
            fileList.seek(pos);
            if(fileList.isDir()) {
                DWIN_TFT_SERIAL_PROTOCOLPGM("/");
                DWIN_TFT_SERIAL_PROTOCOLLN(fileList.filename());
                DWIN_TFT_SERIAL_PROTOCOLPGM("/");
                DWIN_TFT_SERIAL_PROTOCOLLN(fileList.longFilename());
                SERIAL_ECHO(pos);
                SERIAL_ECHOPGM("/");
                SERIAL_ECHOLN(fileList.longFilename());
            } else {
                DWIN_TFT_SERIAL_PROTOCOLLN(fileList.filename());
                DWIN_TFT_SERIAL_PROTOCOLLN(fileList.longFilename());
                SERIAL_ECHO(pos);
                SERIAL_ECHOLN(fileList.longFilename());
            }
        }

        DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_FILE_LIST_END); // Filelist stop
        DWIN_TFT_SERIAL_ENTER();
    }
}

void DwinTFTFileBrowserClass::selectFile()
{
    reset();
    if(!ExtUI::isMediaInserted()) {
        DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_NOT_INSERTED);
        DWIN_TFT_SERIAL_ENTER();
    } else {
        char *starpos = strchr(DwinTFTCommand.TFTstrchr_pointer + 4, '*');
        if (DwinTFTCommand.TFTstrchr_pointer[4] == '/') { //directory
            strcpy(selectedDirectory, DwinTFTCommand.TFTstrchr_pointer + 5);
        } else if (DwinTFTCommand.TFTstrchr_pointer[4] == '<') { //special menu
            strcpy(selectedDirectory, DwinTFTCommand.TFTstrchr_pointer + 4);
        } else {
            if(starpos != NULL) {
                *(starpos - 1) = '\0';
            }
            strcpy(selectedFilename, DwinTFTCommand.TFTstrchr_pointer + 4);
            card.openFileRead(selectedFilename);
            if(card.isFileOpen()) {
                DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_OPEN_SUCCESS); // J20 Open successful
                DWIN_TFT_SERIAL_ENTER();
                #ifdef DWIN_TFT_DEBUG
                    SERIAL_ECHOLNPGM("TFT Serial Debug: File open successful... J20");
                #endif
            } else {
                DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_OPEN_FAILED); // J21 Open failed
                DWIN_TFT_SERIAL_ENTER();
                #ifdef DWIN_TFT_DEBUG
                    SERIAL_ECHOLNPGM("TFT Serial Debug: File open failed... J21");
                #endif
            }
        }
        DWIN_TFT_SERIAL_ENTER();
    }
}

void DwinTFTFileBrowserClass::refreshFileList()
{
    if (selectedDirectory[0]==0) {
      card.mount();
    } else {
      if (selectedDirectory[0] == '.' && selectedDirectory[1] == '.') {
        card.cdup();
      } else {
        if (selectedDirectory[0] == '<') {
          //HandleSpecialMenu();
        } else {
          card.cd(selectedDirectory);
        }
      }
    }
    selectedDirectory[0]=0;

    if(!ExtUI::isMediaInserted())
    {
      DWIN_TFT_SERIAL_PROTOCOLPGM("J02"); // J02 SD Card initilized
      DWIN_TFT_SERIAL_ENTER();
      #ifdef DWIN_TFT_DEBUG
        SERIAL_ECHOLNPGM("TFT Serial Debug: SD card initialized... J02");
      #endif
    }
}

#endif

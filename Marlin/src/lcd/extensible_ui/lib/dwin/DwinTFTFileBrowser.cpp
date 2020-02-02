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
    selectedFilename[0] = '\0';
    selectedDirectory[0] = '\0';
}

void DwinTFTFileBrowserClass::listFiles()
{
    if(!ExtUI::isMediaInserted()) {
        DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_NOT_INSERTED); //no sd card found
        DWIN_TFT_SERIAL_ENTER();
    } else {
        uint16_t itemPos = 0;
        if(DwinTFTCommand.codeSeen('S')) {
            itemPos = DwinTFTCommand.codeValue();
        }

        DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_FILE_LIST_START); // Filelist start
        DWIN_TFT_SERIAL_ENTER();

        if(strcasecmp_P(selectedDirectory, PSTR("<Extra Menu>")) == 0) {
            buildExtraMenu(itemPos);
        } else {
            uint16_t itemCount = fileList.count();
            uint16_t maxItems = constrain(itemPos + 4, 0, itemCount);
            if(itemPos == 0) {
                if(!fileList.isAtRootDir()) {
                    DWIN_TFT_SERIAL_PROTOCOLLNPGM("../");
                    DWIN_TFT_SERIAL_PROTOCOLLNPGM("../");
                    maxItems = constrain(itemPos + 3, 1, itemCount + 1);
                } else {
                    DWIN_TFT_SERIAL_PROTOCOLLNPGM("<Extra Menu>");
                    DWIN_TFT_SERIAL_PROTOCOLLNPGM("<Extra Menu>");
                    maxItems = constrain(itemPos + 3, 1, itemCount + 1);
                }
            } else {
                itemPos--;
            }
            
            for(uint16_t pos = itemPos; pos < maxItems; pos++) {
                fileList.seek(pos);

                if(fileList.isDir()) {
                    DWIN_TFT_SERIAL_PROTOCOL(fileList.shortFilename());
                    DWIN_TFT_SERIAL_PROTOCOLLNPGM("/");
                    DWIN_TFT_SERIAL_PROTOCOL(fileList.filename());
                    DWIN_TFT_SERIAL_PROTOCOLLNPGM("/");
                    SERIAL_ECHO(pos);
                    SERIAL_ECHOPGM(":");
                    SERIAL_ECHO(fileList.filename());
                    SERIAL_ECHOLNPGM("/");
                } else {
                    DWIN_TFT_SERIAL_PROTOCOLLN(fileList.shortFilename());
                    DWIN_TFT_SERIAL_PROTOCOLLN(fileList.filename());
                    SERIAL_ECHO(pos);
                    SERIAL_ECHOPGM(":");
                    SERIAL_ECHOLN(fileList.filename());
                }
            }
        }

        DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_FILE_LIST_END); // Filelist stop
        DWIN_TFT_SERIAL_ENTER();
    }
}

void DwinTFTFileBrowserClass::selectFile()
{
    if(!ExtUI::isMediaInserted()) {
        DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_NOT_INSERTED);
        DWIN_TFT_SERIAL_ENTER();
    } else {
        char *starpos = strchr(DwinTFTCommand.TFTstrchr_pointer + 4, '*');
        size_t lastCharPos = strlen(DwinTFTCommand.TFTstrchr_pointer) - 1;
        if (DwinTFTCommand.TFTstrchr_pointer[lastCharPos] == '/' && 
            DwinTFTCommand.TFTstrchr_pointer[4] == '.' && 
            DwinTFTCommand.TFTstrchr_pointer[5] == '.') { //dir up
            fileList.upDir();
            listFiles();
        } else if (DwinTFTCommand.TFTstrchr_pointer[lastCharPos] == '/') { //directory
            memcpy(selectedDirectory, DwinTFTCommand.TFTstrchr_pointer + 4, lastCharPos - 1);
            selectedDirectory[strlen(selectedDirectory) - 1] = '\0';
            fileList.changeDir(selectedDirectory);
            listFiles();
        } else if(strcasecmp_P(DwinTFTCommand.TFTstrchr_pointer + 4, PSTR("<Extra Menu>")) == 0) {
            strcpy(selectedDirectory, DwinTFTCommand.TFTstrchr_pointer + 4);
            listFiles();
        } else if(DwinTFTCommand.TFTstrchr_pointer[4] == '<' && 
            DwinTFTCommand.TFTstrchr_pointer[lastCharPos] == '>') {
            strcpy(selectedFilename, DwinTFTCommand.TFTstrchr_pointer + 4);
            handleExtraMenu();
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
    reset();
    if(!ExtUI::isMediaInserted()) {
        DWIN_TFT_SERIAL_PROTOCOLPGM(DWIN_TFT_TX_SD_CARD_NOT_INSERTED);
        DWIN_TFT_SERIAL_ENTER();
    } else {
        while (!fileList.isAtRootDir()) {
            fileList.upDir();
        }
        listFiles();
    }
}

void DwinTFTFileBrowserClass::buildExtraMenu(uint16_t pos)
{
    if(pos % 4 != 0) { //check if pos is divisible by 4
        return;
    }
    switch (int(pos / 4)) //max display 4 items per page
    {
        case 0:
            DWIN_TFT_SERIAL_PROTOCOLLNPGM("<../>");
            DWIN_TFT_SERIAL_PROTOCOLLNPGM("<../>");
            DWIN_TFT_SERIAL_PROTOCOLLNPGM(EXTRA_MENU_Z_UP);
            DWIN_TFT_SERIAL_PROTOCOLLNPGM(EXTRA_MENU_Z_UP);
            DWIN_TFT_SERIAL_PROTOCOLLNPGM(EXTRA_MENU_Z_DOWN);
            DWIN_TFT_SERIAL_PROTOCOLLNPGM(EXTRA_MENU_Z_DOWN);
            break;
    }
}

void DwinTFTFileBrowserClass::handleExtraMenu()
{
    if(strcasecmp_P(selectedFilename, PSTR("<../>")) == 0) {
        reset();
        listFiles();
    } else if(strcasecmp(selectedFilename, EXTRA_MENU_Z_UP) == 0) {
        DwinTFT.gcodeQueue_P(DWIN_TFT_GCODE_G91); // relative coordinates
        DwinTFT.gcodeQueue_P(PSTR("G0 Z0.1"));
        DwinTFT.gcodeQueue_P(DWIN_TFT_GCODE_G90); // absolute coordinates
    } else if(strcasecmp(selectedFilename, EXTRA_MENU_Z_DOWN) == 0) {
        DwinTFT.gcodeQueue_P(DWIN_TFT_GCODE_G91); // relative coordinates
        DwinTFT.gcodeQueue_P(PSTR("G0 Z-0.1"));
        DwinTFT.gcodeQueue_P(DWIN_TFT_GCODE_G90); // absolute coordinates
    }
}

#endif

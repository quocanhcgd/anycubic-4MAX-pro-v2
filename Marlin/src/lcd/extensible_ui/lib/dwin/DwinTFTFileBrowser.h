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

#include <stdio.h>

#include "../../../../sd/SdFatConfig.h"

#define EXTRA_MENU_Z_UP "<Z Up 0.1>"
#define EXTRA_MENU_Z_DOWN "<Z Down 0.1>"

class DwinTFTFileBrowserClass {
private:
  void buildExtraMenu(uint16_t pos);
  void handleExtraMenu();
public:
  char selectedFilename[LONG_FILENAME_LENGTH];
  char selectedDirectory[LONG_FILENAME_LENGTH];
  DwinTFTFileBrowserClass();
  void reset();
  void listFiles();
  void selectFile();
  void refreshFileList();
};

extern DwinTFTFileBrowserClass DwinTFTFileBrowser;

/***************************************************************************
 *   Copyright (C) 2013 by James Holodnak                                  *
 *   jamesholodnak@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include "version.h"
#include "emu/emu.h"
#include "emu/commands.h"
#include "emu/events.h"
#include "misc/log.h"
#include "misc/config.h"
#include "misc/paths.h"
#include "misc/memutil.h"
#include "nes/nes.h"
#include "system/main.h"
#include "system/system.h"
#include "system/video.h"
#include "system/input.h"
#include "palette/palette.h"
//#include "palette/generator.h"
#include "system/sdl/console/console.h"

#include <am.h>
#include <klib.h>

//required
char configfilename[1024] = CONFIG_FILENAME;
char exepath[1024] = "";

//todo:  this is getting ugly
int main(const char *romfilename)
{
	//int i;
  int ret;
	//int recordmovie = 0;
	//char *p;
	//char romfilename[1024] = "";
	//char patchfilename[1024] = "";
	//char moviefilename[1024] = "";
	//char testfilename[1024] = "";

  _ioe_init();

	//clear the tmp strings and configfile string
	//memset(romfilename,0,1024);
	//memset(patchfilename,0,1024);
	//memset(moviefilename,0,1024);
	//memset(configfilename,0,1024);
	//memset(testfilename,0,1024);

  //strcpy(romfilename,"mario");
  //strcpy(romfilename,"roms/lunar_pool.nes");
  //strcpy(romfilename,"roms/sky_destroyer.nes");

	//add extra subsystems
	emu_addsubsystem("console",console_init,console_kill);

	//initialize the emulator
	if(emu_init() != 0) {
        log_printf("main:  emu_init() failed\n");
        return(2);
	}

	//load rom specified by arguments
	//if(strcmp(romfilename,"") != 0) {
		emu_event(E_LOADROM,(void*)romfilename);
	//}

	//load patch
	//if(strcmp(patchfilename,"") != 0) {
	//	emu_event(E_LOADPATCH,(void*)patchfilename);
	//}

	//see what we need to do with the movie
	//if(strcmp(moviefilename,"") != 0) {
	//	if(recordmovie) {
	//		if(nes->cart == 0)
	//			log_printf("main:  cannot record movie without rom loaded.\n");
	//		else {
	//			emu_event(E_SAVEMOVIE,(void*)moviefilename);
	//			emu_event(E_RECORDMOVIE,0);
	//			nes->movie.mode |= (recordmovie > 1) ? MOVIE_TEST : 0;
	//		}
	//	}
	//	else {
	//		emu_event(E_LOADMOVIE,(void*)moviefilename);
	//		emu_event(E_PLAYMOVIE,0);
	//	}
	//}

	//begin automated tests
	//if(strcmp(testfilename,"") != 0)
	//	ret = emu_mainloop_test(testfilename);

	//or begin the main loop
	//else
	ret = emu_mainloop();

	//destroy emulator
	emu_kill();

	//return to os
	return(emu_exit(ret));
}

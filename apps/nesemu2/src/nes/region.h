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

#ifndef __nes__region_h__
#define __nes__region_h__

//regions
#define REGION_NTSC	0
#define REGION_PAL	1
#define REGION_DENDY	2

typedef struct region_s {

	//region id
	u32	id;

	//frames per second
	u32	fps;

	//master clock
	u32	hz;

	//vblank start line
	u32	vblank_start;

	//last line
	u32	end_line;
} region_t;

extern region_t region_ntsc;
extern region_t region_pal;
extern region_t region_dendy;

void nes_set_region(int r);

#endif

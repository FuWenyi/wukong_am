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

static INLINE void inc_linecycles()
{
	LINECYCLES++;
	if(LINECYCLES >= 341) {
		LINECYCLES = 0;
		SCANLINE++;
		if(SCANLINE > nes->region->end_line) {
			SCANLINE = 0;
			FRAMES++;
		}
	}
}

static INLINE void skip_cycle()
{
	//ensure we are not in pal mode
	if((nes->region->id & REGION_PAL) == 0) {

		//make sure we are on an odd frame and that the ppu is outputting pixels
		if((FRAMES & 1) && nes->ppu.rendering) {
			inc_linecycles();
		}
	}
}

static INLINE void clear_sp0hit_flag()
{
	STATUS &= ~(0x40 | 0x20);
}

static INLINE void clear_nmi_flag()
{
	STATUS &= ~0x80;
}

static INLINE void clear_nmi_line()
{
	cpu_clear_nmi();
}

static INLINE void set_nmi()
{
	STATUS |= 0x80;
	if(CONTROL0 & 0x80)
		cpu_set_nmi();
}

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

#include "mappers/mapperinc.h"

static u8 prg,chr,mirror,irqenabled;
static int irqcounter;

static void sync()
{
	mem_setprg8(6,prg);
	mem_setprg32(8,3);
	if(nes->cart->chr.size)
		mem_setchr8(0,chr);
	else
		mem_setvram8(0,0);
	mem_setmirroring(mirror);
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0xE003) {
		case 0x8000:
		case 0x8001:
			chr = data;
			break;
		case 0xE000:
			prg = data;
			break;
		case 0xE001:
			mirror = ((data & 8) >> 3) ^ 1;
			break;
		case 0xE002:
			if(data & 2)
				irqenabled = 1;
			else {
				cpu_clear_irq(IRQ_MAPPER);
				irqenabled = 0;
				irqcounter = 0;
			}
			break;
	}
	sync();
}

static void cpucycle()
{
	int prev = irqcounter;

	if(irqenabled == 0)
		return;
	irqcounter++;
	if((irqcounter & 0x6000) != (prev & 0x6000)) {
		if((irqcounter & 0x6000) == 0x6000)
				cpu_set_irq(IRQ_MAPPER);
		else
			cpu_clear_irq(IRQ_MAPPER);
	}
}

static void reset(int hard)
{
	int i;

	for(i=0x8;i<0x10;i++)
		mem_setwritefunc(i,write);
	if(nes->cart->chr.size == 0)
		mem_setvramsize(8);
	prg = 0;
	chr = 0;
	mirror = 1;
	irqenabled = 0;
	irqcounter = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_U8(chr);
	STATE_U8(mirror);
	STATE_U8(irqenabled);
	STATE_INT(irqcounter);
	sync();
}

MAPPER(B_BTL_BIOMIRACLEA,reset,0,cpucycle,state);

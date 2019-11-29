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
#include "mappers/chips/mmc3.h"

static u8 security[8] = {0,3,1,5,6,7,2,4};
static u8 reg[2];

static void sync()
{
	u8 bank;

	if(reg[0] & 0x40) {
		bank = (reg[0] & 5) | ((reg[0] >> 2) & 2) | ((reg[0] >> 2) & 8);
		if(reg[0] & 2)
			mem_setprg32(0x8,bank >> 1);
		else {
			mem_setprg16(0x8,bank);
			mem_setprg16(0xC,bank);
		}
	}
	else
		mmc3_syncprg(0xFF,0);
	mmc3_syncchr(0xFF,0);
	mmc3_syncsram();
	mmc3_syncmirror();
}

static u8 read5(u32 addr)
{
	return(((addr >> 8) & 0xFE) | (((~addr >> 8) | addr) & 1));
}

static void write5(u32 addr,u8 data)
{
	if(addr < 0x5800) {
		return;
	}
	reg[addr & 1] = data;
	sync();
}

static void write_security(u32 addr,u8 data)
{
	if(addr & 1)
		mmc3_write(addr,data);
	else
		mmc3_write(addr,(data & 0xC0) | security[data & 7]);
}

static void reset(int hard)
{
	if(hard)
		reg[0] = reg[1] = 0;
	mem_setreadfunc(5,read5);
	mem_setwritefunc(5,write5);
	mmc3_reset(C_MMC3B,sync,hard);
	mem_setwritefunc(8,write_security);
	mem_setwritefunc(9,write_security);
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,2);
	mmc3_state(mode,data);
}

MAPPER(B_UNL_H2288,reset,mmc3_ppucycle,0,state);

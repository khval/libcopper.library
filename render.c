

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#include "render.h"

uint32 copperList[1000];
uint32 copperl1;
uint32 copperl2;

uint32 bp1;
uint32 bp2;
uint32 bp3;
uint32 bp4;
uint32 bp5;
uint32 bp6;
uint32 bp7;
uint32 bp8;

ULONG last_VWaitPos = 0, last_HWaitPos = 0;
ULONG VWaitPos = 0, HWaitPos = 0;

static uint32 color[256];

uint32 COP1LC, COP2LC;

uint32 diwstart, diwstop, ddfstart, ddfstop;

uint32 beam_clock = 0;
union cop *ptr;

struct RastPort *rp = NULL;

// ddfstart = ddfstop - (8 *(wc-1))
// -------------------

int WordCount( int ddfstart, int ddfstop)
{
	int wc;

	// lowres, 8 clocks 
	wc = ((ddfstop- ddfstart) / 8) +1 ;

/*
	// hires, 4 clocks
	wc = ((ddfstop- ddfstart) / 4) +2 ;
*/

	return wc;
}


void clu( int x, int y )
{
	printf("diwstart: %d,%d\n", x,y);

	x = 20;
	y *= 2;

	printf(" %d,%d\n", x,y);

	SetAPen(rp,1);
	x += 5; 	Move(rp,x,y);
	x -= 5;	Draw(rp,x,y);
	y += 5; 	Draw(rp,x,y);
}

void crb(int y)
{
	int x;
	int mix = diwstart & 0xFF;
	int max = diwstop & 0xFF;

	printf("diwstop: y %d\n",y);

	int wc = WordCount( mix, max);

	x = wc*16 +20 ;

	y *= 2;

	printf(" %d,%d -- wc %d\n",x,y,wc);

	SetAPen(rp,1);
	y -= 5; 	Move(rp,x,y);
	y += 5;	Draw(rp,x,y);
	x -= 5; 	Draw(rp,x,y);
}

void init_ecs2colors()
{
	uint32 i;
	for (i=0;i<0x10000;i++)
	{
		 ecs2argb[i] = 0xFF000000 
			| ( (i & 0xF00) << 12)
			| ( (i & 0xF0) << 8)
			| ( (i & 0xF) << 4);
	}
}

#define setHigh16(name,value) name = (name & 0xFFFF) | ( (value)<<16);
#define setLow16(name,value) name = ( name & 0xFFFF0000) | (value);

void cop_move(union cop data)
{
	switch ( data.d16.a )
	{
		case INTREQ:	break;
		case DIWSTART: diwstart = data.d16.b; 
					clu( diwstart & 0xFF, diwstart >> 8 );
					break;
		case DIWSTOP: diwstop = data.d16.b; 
					crb(  (diwstop >> 8) + 0x100 );
					break;

		case DDFSTART: ddfstart = data.d16.b; 
					break;

		case DDFSTOP: ddfstop = data.d16.b; 
					break;

		case COPJMP1: ptr = (union cop *) COP1LC -1;
					break;

		case COPJMP2: ptr = (union cop *) COP2LC -1;
					break;

		case COLOR00: color[0] = ecs2argb[data.d16.b];	break;
		case COLOR01: color[1] = ecs2argb[data.d16.b];	break;
		case COLOR02: color[2] = ecs2argb[data.d16.b];	break;
		case COLOR03: color[3] = ecs2argb[data.d16.b];	break;
		case COLOR04: color[4] = ecs2argb[data.d16.b];	break;
		case COLOR05: color[5] = ecs2argb[data.d16.b];	break;

		case BPL1PTH: setHigh16(bp1,data.d16.b); break;
		case BPL1PTL: setLow16(bp1,data.d16.b); break;
		case BPL2PTH: setHigh16(bp2,data.d16.b); break;
		case BPL2PTL: setLow16(bp2,data.d16.b); break;

//		case BPLCON0:

		case COP1LCH: setHigh16(COP1LC,data.d16.b); break;
		case COP1LCL: setLow16(COP1LC,data.d16.b); break;
		case COP2LCH: setHigh16(COP2LC,data.d16.b); break;
		case COP2LCL: setLow16(COP2LC,data.d16.b); break;
	}
}


int ly = -1;

void plot( int x,int y)
{
	int wc;
	int miy = (diwstart >> 8);
	int may = (diwstop >> 8) + 0x100;
	int mix , max;

	if (ly!=y) printf("%d - %08x\n",y, color[0]);

	wc = WordCount( ddfstart, ddfstop) * lowres_clock;

	mix = WordCount( 0,ddfstart) * lowres_clock;
	max = mix + wc ;

	ly = y;

	if ((y>=miy) && (y<=may))
	{
		if ((x>=mix) && (x<=max))
		{
			x *= (16 / lowres_clock) ;
			y *= 2;
			WritePixelColor(rp,x,y,color[0]);
		}
	}

}


uint32 ecs2argb[0x10000];

void cop_move_(uint16 reg, uint16 data)
{
	union cop c;
	c.d16.a = reg;
	c.d16.b = data;
	cop_move( c );
}

void cop_skip(union cop data)
{
	// wait d16.a. (VP, bit 15 to 8, HP bit 7 to 1)
	if (beam_clock > ((data.d16.a & 0xFFFE) >> 1)) ptr++;

	// bit enable in d16.b
	// bit 15 blitter finished 
	// bit 14-8 VP, enable bits?
	// bit 7-1, HP, enable bits?
}

uint32 wait_beam = 0x0000;
uint32 wait_beam_enable = 0xFFFF;

void cop_wait(union cop data)
{
	wait_beam = data.d16.a & 0xFFFE;
	wait_beam_enable = data.d16.b & 0xFFFE;
}

void render_copper()
{
	ptr = (union cop *) copperList;

	for (;ptr -> d32 != 0xFFFFFFFE;ptr++)
	{
		printf("%-8d, %04x,%04x -- %08x\n",ptr - (union cop *) copperList, ptr -> d16.a,ptr -> d16.b,beam_clock);

		switch (ptr -> d32 & 0x1001)
		{
			case 0x0000:
			case 0x0001:	cop_move( *ptr );	break;
			case 0x1000:	cop_wait( *ptr); 	break;
			case 0x1001:	cop_skip( *ptr);	break;
		}

		while ((beam_clock & wait_beam_enable) < wait_beam)
		{
			beam_clock++;
			plot( beam_clock & 0xFF, beam_clock >> 8 );
		}

		beam_clock++;
		plot( beam_clock & 0xFF, beam_clock >> 8 );
	}
}



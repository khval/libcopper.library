
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#include "render.h"

uint32 copperList[2000];

uint32 copperl1;
uint32 copperl2;

uint32 bp0, bp1, bp2, bp3, bp4, bp5, bp6, bp7;

ULONG last_VWaitPos = 0, last_HWaitPos = 0;
ULONG VWaitPos = 0, HWaitPos = 0;

static uint32 color[256];

uint32 COP1LC, COP2LC;

uint32 diwstart, diwstop, ddfstart, ddfstop;

uint32 beam_clock = 0;
union cop *ptr;

#define display_wx	0
#define display_y 0

extern unsigned char *bp0ptr,*bp1ptr,*bp2ptr,*bp3ptr,*bp4ptr,*bp5ptr,*bp6ptr,*bp7ptr;

struct RastPort *copper_rp = NULL;

// ddfstart = ddfstop - (8 *(wc-1))
// -------------------

int WordCountToDispDataFetchStop( int hires, int ddstart, int wc )
{
	if ( ! hires )
	{
		// lowres
		return  8 * (wc - 1)  + ddstart;
	}
	else return  4 * (wc - 2)  + ddstart;
}

int DispWinToDispDataFetch(int hires, int diwstart)
{
	if (!hires)	return (diwstart - 17) / 2;

	return (diwstart - 9) / 2;
}

int DispDataFetchWordCount( int hires, int ddfstart, int ddfstop)
{
	// lowres, 8 clocks 
	if ( !hires)	return  ((ddfstop- ddfstart) / 8) +1 ;

	// hires, 4 clocks
	return ((ddfstop- ddfstart) / 4) +2 ;
}


void clu( int x, int y )
{
	printf("diwstart: %d,%d\n", x,y);

	x = 20;
	y *= 2;

	printf(" %d,%d\n", x,y);

	SetAPen(copper_rp,1);
	x += 5; 	Move(copper_rp,x,y);
	x -= 5;	Draw(copper_rp,x,y);
	y += 5; 	Draw(copper_rp,x,y);
}

void crb(int y)
{
	int x;
	int mix = diwstart & 0xFF;
	int max = diwstop & 0xFF;

	int wc = DispDataFetchWordCount( 0, mix, max);

	x = wc*16 +20 ;

	y *= 2;

	SetAPen(copper_rp,1);
	y -= 5; 	Move(copper_rp,x,y);
	y += 5;	Draw(copper_rp,x,y);
	x -= 5; 	Draw(copper_rp,x,y);
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

uint16 hires,planes,ham,lace;

uint64 (*planar_routine) (uint32 *data)=NULL;

extern void *planar_routines[];

uint ddf_wc;
uint ddf_mix;
uint ddf_max;

uint32 display_offset_x ;
uint32 display_scale_x ;
uint32 display_scale_y ;
uint32 pixels_per_chunk ;
uint32 display_chunk_offset ;
uint32 display_chunk16_offset ;
uint32 num_chunks;
uint32 clock_speed;

void update_display_offsets()
{
	display_offset_x = (ddf_mix - display_wx) * 32 - ddf_mix * 32;

	if (hires)
	{
		display_scale_x = 1;
		display_scale_y = 2;
		pixels_per_chunk = 8;
		num_chunks = 2;
		clock_speed = 2;
	}
	else
	{
		display_scale_x = 2;
		display_scale_y = 2;
		pixels_per_chunk = 4;
		num_chunks = 4;
		clock_speed = 1;
	}

	display_chunk_offset = pixels_per_chunk  * display_scale_x;
	display_chunk16_offset = 16 * display_scale_x;
}

void update_ddf()
{
	ddf_wc = DispDataFetchWordCount( 0, ddfstart, ddfstop) ;
	ddf_mix = DispDataFetchWordCount( 0, 0,ddfstart);
	ddf_max = ddf_mix + ddf_wc ;
	update_display_offsets();
}

struct 
{
	uint32 x0;
	uint32 y0;
	uint32 x1;
	uint32 y1;
} dispwindow;


void cop_move(union cop data)
{
	printf("Cop_move %04x\n",data.d16.a);

	switch ( data.d16.a )
	{
		case INTREQ:	break;
		case DIWSTART: diwstart = data.d16.b; 
					dispwindow.y0 = diwstart>>8 ;
					dispwindow.x0 = diwstart & 0xFF ;
					break;

		case DIWSTOP: diwstop = data.d16.b; 
					dispwindow.y1 = diwstop & 0x8000 ? (diwstop >> 8) : (diwstop >> 8) + 256;
					dispwindow.x1 = diwstop & 0x80 ? (diwstop & 0xFF)  : (diwstop & 0xFF) + 256;
					break;

		case DDFSTART: ddfstart = data.d16.b; 
					update_ddf();
					break;

		case DDFSTOP: ddfstop = data.d16.b; 
					update_ddf();
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

		case BPL1PTH:	setHigh16(bp0,data.d16.b); 
					bp0ptr = (unsigned char *) bp0;
					break;

		case BPL1PTL: 	setLow16(bp0,data.d16.b); 
					bp0ptr = (unsigned char *) bp0;
					break;

		case BPL2PTH: setHigh16(bp1,data.d16.b);
					bp1ptr = (unsigned char *) bp1;
					break;

		case BPL2PTL: setLow16(bp1,data.d16.b);
					bp1ptr = (unsigned char *) bp1;
					break;

		case BPLCON0: printf("BPLCON0\n");
					hires = data.d16.b & 0x8000;
					planes = (data.d16.b & 0x7000) >> 12;
					ham = data.d16.b & (1<<11);
					lace = data.d16.b & (1<<2);
					planar_routine = planar_routines[ planes ];
					update_display_offsets();
					break;

		case COP1LCH: setHigh16(COP1LC,data.d16.b); break;
		case COP1LCL: setLow16(COP1LC,data.d16.b); break;
		case COP2LCH: setHigh16(COP2LC,data.d16.b); break;
		case COP2LCL: setLow16(COP2LC,data.d16.b); break;
	}
}


int ly = -1;
int datafetch = 0;


bool check16( int x,int y )
{
	int miy = (diwstart >> 8);
	int may = (diwstop >> 8) + 0x100;

	if ((y>=miy) && (y<may))
	{
		if ((x>=ddf_mix) && (x<ddf_max))
		{
			return true;
		}
	}
	return false;
}


void convert16( char *data)
{
	planar_routine( (uint32 *) data );
	planar_routine( (uint32 *) (data + 8) );
}

void plot4( int x, int y, char *data )
{
	printf("%s(%d,%d,%08lx)\n",__FUNCTION__,x,y,data);

	WritePixelColor(copper_rp,x,y,color[ *data ++ ]);
	WritePixelColor(copper_rp,x+=display_scale_x,y,color[ *data ++ ]);
	WritePixelColor(copper_rp,x+=display_scale_x,y,color[ *data ++ ]);
	WritePixelColor(copper_rp,x+=display_scale_x,y,color[ *data ++ ]);
}

void plot4_color0( int x, int y, char *data )
{
	uint32 color0 = color[0];
	WritePixelColor(copper_rp,x,y,color0);
	WritePixelColor(copper_rp,x+=display_scale_x,y,color0);
	WritePixelColor(copper_rp,x+=display_scale_x,y,color0);
	WritePixelColor(copper_rp,x+=display_scale_x,y,color0);

//	printf("%d,%d - %08x\n",x,y, color0);
}

void (*plot4_fn)( int x, int y, char *data ) = NULL;


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

	if (wait_beam_enable < wait_beam)
	{
		printf("bad wait, mask smaller then delay... wait can't get to end\n");
		wait_beam = 0;
	}
}

uint32 beam_wordpos;

void dump_copper(uint32 *copperList)
{
	const char *cmd;

	printf("------------ dump_copper -----------------\n");

	ptr = (union cop *) copperList;

	for (;ptr -> d32 != 0xFFFFFFFE;ptr++)
	{
		switch (ptr -> d32 & 0x00010001)
		{
			case 0x00000000: 
			case 0x00000001:	cmd = "Move" ; break;
			case 0x00010000:	cmd = "Wait" ; break;
			case 0x00010001:	cmd = "Skip" ; break;
		}

		printf("%-8s: %04x,%04x\n", cmd, ptr -> d16.a , ptr -> d16.b ); 
	}

	printf("%-8s: %04x,%04x\n", "END",  0xFFFF , 0xFFFE ); 
}

static void box(struct RastPort *rp,int x0,int y0,int x1,int y1)
{
	Move(rp,x0,y0);
	Draw(rp,x1,y0);
	Draw(rp,x1,y1);
	Draw(rp,x0,y1);
	Draw(rp,x0,y0);
}

void render_DisplayWindow(struct RastPort *rp)
{
	int x0;
	int x1;

	int r;
	SetAPen(rp,1);

	printf("from %08x,%08x to %08x,%08x, dx %d dy %d\n",
		dispwindow.x0,dispwindow.y0,dispwindow.x1,dispwindow.y1, 
			(dispwindow.x1 - dispwindow.x0) * 8,
			dispwindow.y1 - dispwindow.y0);

	x0 = (dispwindow.x0 - 0x79) * 8;
	x1 = (dispwindow.x1 - 0x79) * 8;

	for (r=-2;r<2;r++)
		box(rp,x0+r,dispwindow.y0+r,x1+r,dispwindow.y1+r);
}

void render_copper(uint32 *copperList, struct RastPort *rp)
{
	int lx;
	int x,y;
	uint32 off;
	char data[16];
	bool beam_wait = false;
	int to_draw_count = 0;

	copper_rp = rp;

	ptr = (union cop *) copperList;

	bp0ptr = (unsigned char *) bp0;
	bp1ptr = (unsigned char *) bp1;
	bp2ptr = (unsigned char *) bp2;
	bp3ptr = (unsigned char *) bp3;
	bp4ptr = (unsigned char *) bp4;
	bp5ptr = (unsigned char *) bp5;
	bp6ptr = (unsigned char *) bp6;
	bp7ptr = (unsigned char *) bp7;

	update_display_offsets();

	for (;ptr -> d32 != 0xFFFFFFFE;ptr++)
	{
		printf("%-8d, %04x,%04x -- %08x\n",ptr - (union cop *) copperList, ptr -> d16.a,ptr -> d16.b,beam_clock);

		switch (ptr -> d32 & 0x00010001)
		{
			case 0x00000000:
			case 0x00000001:	cop_move( *ptr ); break;
			case 0x00010000:	cop_wait( *ptr);  beam_wait=true; break;
			case 0x00010001:	cop_skip( *ptr); break;
		}

		if ((beam_clock & 3 == 0) || (beam_wait))
		{
			beam_wait = false;

			while ((beam_wordpos & wait_beam_enable) < wait_beam)
			{
				off = 0;

				if (to_draw_count)
				{
					beam_clock+=to_draw_count;

					while (to_draw_count -- )
					{
						plot4_fn( display_offset_x + (x*display_chunk16_offset) + (off * display_chunk_offset) ,  (y-display_y)*display_scale_y, data + (off * pixels_per_chunk) );
						off ++;
					}
				}
				else 
					beam_clock+=clock_speed;

				beam_wordpos  = beam_clock >> 2;		// 4 copper commands per 16pixels.
				lx =x;
				x = beam_wordpos & 0xFF;
				y = beam_wordpos >> 8;

				if ( lx - x ) 
				{
					if (check16(x,y) == false)
					{
						plot4_fn = plot4_color0;
					}
					else
					{
						convert16( data );
						plot4_fn = plot4;
					}

					to_draw_count = 4;
				}
			}

		}
		else
		{

			if (to_draw_count)
			{
				beam_clock+=clock_speed;
				to_draw_count -- ;
				plot4_fn( display_offset_x + (x*display_chunk16_offset) + (off * display_chunk_offset) ,  (y-display_y)*display_scale_y, data + (off * pixels_per_chunk) );
				off ++;
			}
			else 
				beam_clock+=clock_speed;

			beam_wordpos  = beam_clock >> 2;		// 4 copper commands per 16pixels.
			lx =x;
			x = beam_wordpos & 0xFF;
			y = beam_wordpos >> 8;

			if ( lx - x ) 
			{
				if (check16(x,y) == false)
				{
					plot4_fn = plot4_color0;
				}
				else
				{
					convert16( data );
					plot4_fn = plot4;
				}

				to_draw_count = 4;
			}
		}
	}

	render_DisplayWindow(rp);
}



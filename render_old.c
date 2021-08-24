
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
	ddf_wc = WordCount( ddfstart, ddfstop) ;
	ddf_mix = WordCount( 0,ddfstart);
	ddf_max = ddf_mix + ddf_wc ;
	update_display_offsets();
}

void cop_move(union cop data)
{
	printf("Cop_move %04x\n",data.d16.a);

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

		case BPLCON0:
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
	WritePixelColor(rp,x,y,color[ *data ++ ]);
	WritePixelColor(rp,x+=2,y,color[ *data ++ ]);
	WritePixelColor(rp,x+=2,y,color[ *data ++ ]);
	WritePixelColor(rp,x+=2,y,color[ *data ++ ]);
}

void plot4_color0( int x, int y )
{
	uint32 color0 = color[0];
	WritePixelColor(rp,x,y,color0);
	WritePixelColor(rp,x+=2,y,color0);
	WritePixelColor(rp,x+=2,y,color0);
	WritePixelColor(rp,x+=2,y,color0);

	printf("%d,%d - %08x\n",x,y, color0);
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

uint32 beam_wordpos;

void render_copper()
{
	int lx;
	int x,y;
	uint32 off;
	char data[16];
	bool beam_wait = false;

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


			printf("beam_wordpos: %08x, wait_beam_enable: %08x < wait_beam: %08x\n", beam_wordpos , wait_beam_enable , wait_beam);
			getchar();

			while ((beam_wordpos & wait_beam_enable) < wait_beam)
			{
				if (check16(x,y))
				{
//					printf("Read %d,%d - offset x %d\n",x,y,  (bp0ptr - (unsigned char *) bp0) % 40);

					if (off<num_chunks)
					{
						while (off<num_chunks)
						{
							plot4( display_offset_x + (x*display_chunk16_offset) + (off * display_chunk_offset) ,  (y-display_y)*display_scale_y , data + (off * pixels_per_chunk) );
							off++;
						}
					}

					convert16( data );
					off = 0;
					beam_clock+=num_chunks;				// moves faster in wait mode.
					beam_wordpos  = beam_clock >> 2;		// 4 copper commands per 16pixels.
					x = beam_wordpos & 0xFF;
				}
				else 
				{
					beam_clock+=clock_speed;
				}

				beam_wordpos  = beam_clock >> 2;		// 4 copper commands per 16pixels.

				lx = x;
				x = beam_wordpos & 0xFF;
				y = beam_wordpos >> 8;

				if ( lx - x ) 
				{
					if (check16(x,y) == false)
					{
						int part;

						for (part=0;part<num_chunks;part++)
						{
//							plot4_color0( display_offset_x + (x*display_chunk16_offset) + (part * display_chunk_offset) ,  (y-display_y)*display_scale_y );
						}
					}
				}
			}

//			if (off == 0) if (check16(x,y) ) convert16( data );
		}
		else
		{
			if (off<num_chunks)		// 16 pixels, 4 pixel, 
			{
				plot4( display_offset_x + (x*display_chunk16_offset) + (off * display_chunk_offset) ,  (y-display_y)*display_scale_y , data + (off * pixels_per_chunk) );
				off++;
			}
			else
			{
				beam_clock+=clock_speed;
				beam_wordpos  = beam_clock >> 2;		// 4 copper commands per 16pixels.
				lx =x;
				x = beam_wordpos & 0xFF;
				y = beam_wordpos >> 8;

				if ( lx - x ) 
				{
					if (check16(x,y) == false)
					{
						plot4_color0( display_offset_x + (x*display_chunk16_offset) + (0 * display_chunk_offset) ,  (y-display_y)*display_scale_y );
					}
				}
			}
		}
	}
}




#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#include "planes.h"
#include "render.h"

uint32 copperList[2000 + 0x1000];

uint32 copperl1;
uint32 copperl2;

uint32 bp0, bp1, bp2, bp3, bp4, bp5, bp6, bp7;

ULONG last_VWaitPos = 0, last_HWaitPos = 0;
ULONG VWaitPos = 0, HWaitPos = 0;

static uint32 color[256];

uint32 COP1LC, COP2LC;

uint32 diwstart, diwstop, ddfstart, ddfstop;

uint32 beam_clock = 0;
uint16 beam_wordpos = 0;

union cop *ptr;

#define display_bx 0x81
#define display_wx ((display_bx-4)/2)
#define display_y 10

extern unsigned char *bp0ptr,*bp1ptr,*bp2ptr,*bp3ptr,*bp4ptr,*bp5ptr,*bp6ptr,*bp7ptr;

static struct RastPort *copper_rp = NULL;

// ddfstart = ddfstop - (9 *(wc-1))
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

void init_ecs2colors()
{
	s_odd = 0;
	s_even = 0;

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

extern void move_none();

uint64 (*planar_routine) (uint32 *data) = convert_none;
void (*move_routine) () = move_none;

extern void *planar_routines[];
extern void *move_routines[];

uint ddf_wc;
uint ddf_mix;
uint ddf_max;

struct 
{
	uint32 x0;
	uint32 y0;
	uint32 x1;
	uint32 y1;
} dispwindow;

uint32 display_offset_x ;
uint32 display_scale_x ;
uint32 display_scale_y ;
uint32 pixels_per_chunk ;
uint32 display_chunk_offset ;
uint32 display_chunk16_offset ;
uint32 num_chunks;
uint32 clock_speed;

void (*plot4_fn)( int x, int y, char *data ) = NULL;			// current function
void (*plot4_color0_fn)( int x, int y, char *data ) = NULL;		// current default color0 option function
void (*plot4_bitmap_fn)( int x, int y, char *data ) = NULL;		// current default bitmap option function

void plot4_color0_scale1( int x, int y, char *data );
void plot4_color0_scale2( int x, int y, char *data );
void plot4_scale1( int x, int y, char *data );
void plot4_scale2( int x, int y, char *data );

uint32 mi_disp = 0x40;
uint32 ma_disp = 0x80;

uint32 wait_beam = 0x0000;
uint32 wait_beam_enable = 0xFFFF;

static uint32 offset;
static uint32 lbeam_x,lbeam_y;
static uint32 beam_x,beam_y;

void update_display_offsets()
{
	display_offset_x = (ddf_mix - display_wx) * 4 ;

	if (hires)
	{
		display_scale_x = 1;
		display_scale_y = 2;
		pixels_per_chunk = 8;
		num_chunks = 2;
		clock_speed = 2;

		switch (display_scale_x)
		{
			case 1:
					plot4_color0_fn = plot4_color0_scale1; 
					plot4_bitmap_fn = plot4_scale1; 
					break;
			case 2:
					plot4_color0_fn = plot4_color0_scale2;
					plot4_bitmap_fn = plot4_scale2; 
					break;
		}
	}
	else
	{
		display_scale_x = 2;
		display_scale_y = 2;
		pixels_per_chunk = 4;
		num_chunks = 4;
		clock_speed = 1;

		switch (display_scale_x)
		{
			case 1:
					plot4_color0_fn = plot4_color0_scale1; 
					plot4_bitmap_fn = plot4_scale1; 
					break;
			case 2:
					plot4_color0_fn = plot4_color0_scale2;
					plot4_bitmap_fn = plot4_scale2; 
					break;
		}
	}

	mi_disp = dispwindow.x0 / 2;
	ma_disp = dispwindow.x1  /2 ;

	display_chunk_offset = pixels_per_chunk  * display_scale_x;
	display_chunk16_offset = 16 * display_scale_x;
}

bool displayed( int scanx, int scany )
{
//	return (( mi_disp <= scanx ) && ( scanx < ma_disp));

	return true;
}


void update_ddf()
{
	ddf_wc = DispDataFetchWordCount( 0, ddfstart, ddfstop) ;
	ddf_mix = DispDataFetchWordCount( 0, 0,ddfstart);
	ddf_max = ddf_mix + ddf_wc ;
	update_display_offsets();
}

void 	update_routines( int planes )
{
	if (s_even | s_odd)
	{
		planar_routine = planar_shift_routines[ planes ];
	}
	else
	{
		planar_routine = planar_routines[ planes ];
	}

	move_routine = move_routines[ planes ];
}


void cop_wait(union cop data)
{
//	printf("Cop_wait\n");

	if (data.d32 == 0xFFFFFFFE)
	{
		if (beam_y >= 255)
		{
			wait_beam = (286 - 255) << 8 | 0xF4 ;
			wait_beam_enable = 0xFFFE;
			return;
		}
	}

	wait_beam_enable = data.d16.b & 0xFFFE;

	if (wait_beam_enable != 0)
	{
		wait_beam = data.d16.a & wait_beam_enable;
	}
	else wait_beam_enable = wait_beam;
}

void cop_skip(union cop data)
{
	// wait d16.a. (VP, bit 15 to 8, HP bit 7 to 1)
	if (beam_wordpos >= ((data.d16.a & 0xFFFE) >> 1)) ptr++;

	// bit enable in d16.b
	// bit 15 blitter finished 
	// bit 14-8 VP, enable bits?
	// bit 7-1, HP, enable bits?
}

void cop_move(union cop data)
{
//	printf("Cop_move Reg %04x, data %04x\n",data.d16.a,data.d16.b);

	switch ( data.d16.a )
	{
		case INTREQ:	break;
		case DIWSTART: diwstart = data.d16.b; 
					dispwindow.y0 = diwstart>>8 ;
					dispwindow.x0 = (diwstart & 0xFF)  ;
					update_display_offsets();
					break;

		case DIWSTOP: diwstop = data.d16.b; 
					dispwindow.y1 = (diwstop & 0x8000 ? (diwstop >> 8) : (diwstop >> 8) + 0x100 ) ;
					dispwindow.x1 = (diwstop & 0xFF) + 256;
					update_display_offsets();
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

		case BPL1PTH:	setHigh16(bp0,data.d16.b);	bp0ptr = (unsigned char *) bp0;	break;
		case BPL1PTL: 	setLow16(bp0,data.d16.b);	bp0ptr = (unsigned char *) bp0;	break;
		case BPL2PTH: setHigh16(bp1,data.d16.b);	bp1ptr = (unsigned char *) bp1;	break;
		case BPL2PTL: setLow16(bp1,data.d16.b);	bp1ptr = (unsigned char *) bp1;	break;
		case BPL3PTH: setHigh16(bp2,data.d16.b);	bp2ptr = (unsigned char *) bp2;	break;
		case BPL3PTL: setLow16(bp2,data.d16.b);	bp2ptr = (unsigned char *) bp2;	break;
		case BPL4PTH: setHigh16(bp3,data.d16.b);	bp3ptr = (unsigned char *) bp3;	break;
		case BPL4PTL: setLow16(bp3,data.d16.b);	bp3ptr = (unsigned char *) bp3;	break;
		case BPL5PTH: setHigh16(bp4,data.d16.b);	bp4ptr = (unsigned char *) bp4;	break;
		case BPL5PTL: setLow16(bp4,data.d16.b);	bp4ptr = (unsigned char *) bp4;	break;
		case BPL6PTH: setHigh16(bp5,data.d16.b);	bp5ptr = (unsigned char *) bp5;	break;
		case BPL6PTL: setLow16(bp5,data.d16.b);	bp5ptr = (unsigned char *) bp5;	break;
		case BPL7PTH: setHigh16(bp6,data.d16.b);	bp6ptr = (unsigned char *) bp6;	break;
		case BPL7PTL: setLow16(bp6,data.d16.b);	bp6ptr = (unsigned char *) bp6;	break;
		case BPL8PTH: setHigh16(bp7,data.d16.b);	bp7ptr = (unsigned char *) bp7;	break;
		case BPL8PTL: setLow16(bp7,data.d16.b);	bp7ptr = (unsigned char *) bp7;	break;


		case BPLCON0:
					// printf("BPLCON0\n");

					hires = data.d16.b & 0x8000;
					planes = (data.d16.b & 0x7000) >> 12;
					ham = data.d16.b & (1<<11);
					lace = data.d16.b & (1<<2);

					// printf( "planes %d\n", planes );

					update_routines( planes );
					update_display_offsets();
					break;

		case BPLCON1: 
					// printf("BPLCON1\n");

					s_odd = data.d16.b  & 0xF0;
					s_even = data.d16.b  % 0x0F;

					update_routines( planes );
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
	if ((dispwindow.y0<=y) && (y<dispwindow.y1))
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

void move16()
{
	move_routine();
	move_routine();
}

void plot4_scale1( int x, int y, char *data )
{
	WritePixelColor(copper_rp,x++,y,color[ *data ++ ]);
	WritePixelColor(copper_rp,x++,y,color[ *data ++ ]);
	WritePixelColor(copper_rp,x++,y,color[ *data ++ ]);
	WritePixelColor(copper_rp,x,y,color[ *data ++ ]);
}

void plot4_scale2( int x, int y, char *data )
{
	WritePixelColor(copper_rp,x++,y,color[ *data ]);			// 0
	WritePixelColor(copper_rp,x++,y,color[ *data ++ ]);
	WritePixelColor(copper_rp,x++,y,color[ *data ]);		// 1
	WritePixelColor(copper_rp,x++,y,color[ *data ++ ]);
	WritePixelColor(copper_rp,x++,y,color[ *data ]);		// 2
	WritePixelColor(copper_rp,x++,y,color[ *data ++ ]);
	WritePixelColor(copper_rp,x++,y,color[ *data ]);		// 3
	WritePixelColor(copper_rp,x,y,color[ *data ++ ]);
}


void plot4_color0_scale1( int x, int y, char *data )
{
	uint32 color0 = color[0];
	WritePixelColor(copper_rp,x++,y,color0);
	WritePixelColor(copper_rp,x++,y,color0);
	WritePixelColor(copper_rp,x++,y,color0);
	WritePixelColor(copper_rp,x,y,color0);
}

APTR _ba;
ULONG _bpr;

struct TagItem stdBMLock[] =
	{
		{LBM_BaseAddress, &_ba },
		{LBM_BytesPerRow, &_bpr},
		{TAG_END}
	};

void plot4_color0_scale2( int x, int y, char *data )
{
	uint32 color0 = color[0];

	WritePixelColor(copper_rp,x++,y,color0);
	WritePixelColor(copper_rp,x++,y,color0);
	WritePixelColor(copper_rp,x++,y,color0);
	WritePixelColor(copper_rp,x++,y,color0);
	WritePixelColor(copper_rp,x++,y,color0);
	WritePixelColor(copper_rp,x++,y,color0);
	WritePixelColor(copper_rp,x++,y,color0);
	WritePixelColor(copper_rp,x,y,color0);

}


uint32 ecs2argb[0x10000];

void cop_move_(uint16 reg, uint16 data)
{
	union cop c;
	c.d16.a = reg;
	c.d16.b = data;
	cop_move( c );
}


void inc_clock(int n)
{
	lbeam_x = beam_x;
	lbeam_y = beam_y;
	beam_clock += n;
	
	if (beam_clock > 4)
	{
		beam_clock -= 4;
		beam_x ++;
		if (beam_x>127)
		{
			beam_x = 0;
			beam_y++;
			offset = 0;
			reset_copper_tmp();
		}
		beam_wordpos = ( (beam_y & 0xFF) << 8) | ((beam_x & 0x7F) << 1);
	}
}

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

//		printf("%-8s: %04x,%04x\n", cmd, ptr -> d16.a , ptr -> d16.b ); 
	}

//	printf("%-8s: %04x,%04x\n", "END",  0xFFFF , 0xFFFE ); 
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
	int x0,y0;
	int x1,y1;

	int r;
	SetAPen(rp,1);

	printf("display_bx: %d pixels\n",(display_bx+1)*8);
	printf("DDFStart: %d pixels\n",((ddfstart & 0xFF)+9) *16);

	printf("from %08x,%08x to %08x,%08x, dx %d bytes, dy %d\n",
		dispwindow.x0,dispwindow.y0,
		dispwindow.x1,dispwindow.y1, 
		(dispwindow.x1 - dispwindow.x0+1) / 8,
		(dispwindow.y1 - dispwindow.y0+1));

	x0 = (dispwindow.x0 - (display_bx - 20))*2;
	x1 = (dispwindow.x1 - (display_bx - 20))*2;
	y0 = (dispwindow.y0 - display_y) * 2;
	y1 = (dispwindow.y1 - display_y) * 2;

	for (r=0;r<1;r++) box(rp,x0+r,y0+r,x1-r,y1-r);

}

	int to_draw_count = 0;

void draw_( int draw_x, int draw_y, char *data)
{
	if (to_draw_count)
	{
		while (to_draw_count -- )
		{
			plot4_fn( draw_x + (offset * display_chunk_offset) ,  draw_y, data + (offset * pixels_per_chunk) );
			offset ++;
			inc_clock(clock_speed);
		}
	}
	else 
		inc_clock(clock_speed);
}

static char data[16];
static int draw_x,draw_y;

void __render()
{
			if (to_draw_count)
			{
				to_draw_count -- ;
				plot4_fn( draw_x + (offset * display_chunk_offset), draw_y, data + (offset * pixels_per_chunk) );
				offset ++;
				inc_clock(clock_speed);
			}
			else 
				inc_clock(clock_speed);

			if ( lbeam_x - beam_x ) 
			{

				if (lbeam_y - beam_y) draw_y = (beam_y-display_y)*display_scale_y;

				if (check16(beam_x,beam_y) == false)
				{
					plot4_fn = plot4_color0_fn;
				}
				else
				{
					if (displayed(beam_x,beam_y))
					{
						convert16( data );
						plot4_fn = plot4_bitmap_fn;
					}
					else
					{
						move16();
						plot4_fn = plot4_color0_fn;
					}
				}

				if (draw_x< 640+64) 
				{
					to_draw_count = 4;
				}
				else
				{
					to_draw_count = 0;
				}

				draw_x = display_offset_x + (beam_x*display_chunk16_offset);
				offset = 0;
			}
}

void render_copper(struct Custom *custom, uint32 *copperList, struct RastPort *rp)
{
	char ck = '*';
	bool beam_wait = false;
	bool wtf = false;

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

	beam_clock = 0;	// reset beam
	beam_x = 0;
	beam_y = 0;
	beam_wordpos = 0;

	wait_beam_enable = 0;
	wait_beam = 0;

	for (;;)
	{
		switch (ptr -> d32 & 0x00010001)
		{
			case 0x00000000:
			case 0x00000001:	cop_move( *ptr ); break;
			case 0x00010000:	cop_wait( *ptr);  beam_wait=true; break;
			case 0x00010001:	cop_skip( *ptr); break;
		}

		do
		{
			__render();
		} while ((beam_wordpos & wait_beam_enable) < wait_beam);

//		Printf("Beam_y %04lx --- beam_wordpos %08lx\n",beam_y, beam_wordpos);

		if (ptr -> d32 == 0xFFFFFFFE) break;
		ptr ++;
	}

//	render_DisplayWindow(rp);
}




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
#include "beam.h"
#include "plot4.h"

uint32 copperList[2000 + 0x1000];
uint32 copperl1;
uint32 copperl2;
uint32 bp0, bp1, bp2, bp3, bp4, bp5, bp6, bp7;

// start of --- render 2 ---

int def_min_edge,def_max_edge;
int min_edge_remain, max_edge_remain;
int final_edge_remain;

// end of --- render 2 ---

ULONG last_VWaitPos = 0, last_HWaitPos = 0;
ULONG VWaitPos = 0, HWaitPos = 0;

uint32 COP1LC, COP2LC;
uint32 diwstart, diwstop, ddfstart, ddfstop;
uint32 beam_clock = 0;
uint16 beam_wordpos = 0;

union ubeam  lbeam_x, lbeam_y;
union ubeam beam_x,beam_y;

union cop *ptr;

char plane_data[16];

uint64 *(*plot4_fn)( char *source_data , uint64 *dest_data ) = NULL;			// current function
uint64 *(*plot4_color0_fn)( char *source_data , uint64 *dest_data ) = NULL;		// current default color0 option function
uint64 *(*plot4_bitmap_fn)( char *source_data , uint64 *dest_data ) = NULL;		// current default bitmap option function

#define CAST_PLOT4 uint64 *(*) ( char *source_data, uint64 *dest_data )

static int draw_x,draw_y;

void setPalette(int index,uint32 argb)
{
//	DebugPrintF("set color %d to %08x at line %d\n", index,argb,beam_y.low);

	palette2[index].argb1 = argb;
	palette2[index].argb2 = argb;
}

#define DISPLAY_LEFT_SHIFT 0x40	
#define DIW_DDF_OFFSET 9
#define display_bx 109
#define display_y 10

#define enable_writeimage 1

uint32 first_addr;
uint32 last_addr;
uint32 ywaitmask;
uint32 xwaitmask;

void update_ddf( int is_hires );

extern unsigned char *bp0ptr,*bp1ptr,*bp2ptr,*bp3ptr,*bp4ptr,*bp5ptr,*bp6ptr,*bp7ptr;


void render_DisplayWindow()
{

}

// according to hardware reference manual, pixel data spend a couple of cycles somwhere in the chip 

// ignore first 0x40 pixels at start of the display. these are hidden..


int gfx_shift = 0;

int coord_hw_to_winodw_x(int x)
{
	x -= DISPLAY_LEFT_SHIFT;
	return x << gfx_shift;		// depends on lowres / hires
}

int coord_window_to_hw_x(int x)
{
	x >>= gfx_shift;
	return x + DISPLAY_LEFT_SHIFT;
}

int coord_window_to_diw_x( int x )
{
	x =  coord_window_to_hw_x(x);
	return x - DIW_DDF_OFFSET;
}

int coord_diw_to_window_x( int x)
{
	return (x - DISPLAY_LEFT_SHIFT + DIW_DDF_OFFSET - 1 ) << gfx_shift;
}

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
	// bit 0, is used, ddfstart is 2x the size it should be..

	// lowres, 8 clocks // 16bit -> 2 bytes -> 4 nibbels * scale = 8..
	if ( !hires)	return  ((ddfstop- ddfstart) / 8) +1 ;

	// hires, 4 clocks
	return ((ddfstop- ddfstart) / 4) +2 ;
}

void init_ecs2colors()
{
	union argb_u *color ;

	s_odd = 0;
	s_even = 0;

	uint32 i;

	color = ecs2argb;
	for (i=0;i<0x10000;i++)
	{
		color -> channel.a = 0xFF;
		color -> channel.r = ((i & 0xF00) >> 8) * 0x11;
		color -> channel.g = ((i & 0xF0) >> 4) * 0x11;
		color -> channel.b = (i & 0xF) * 0x11;
		color ++;
	}
}

#define setHigh16(name,value) name = (name & 0xFFFF) | ( (value)<<16);
#define setLow16(name,value) name = ( name & 0xFFFF0000) | (value);

uint16 hires,planes,ham,lace;

extern void move_none();

uint64 (*planar_routine) (uint32 *data) = convert_none;

extern void *planar_routines[];
extern void *move_routines[];

uint ddf_wc;
uint ddf_mix;
uint ddf_max;

extern void (*move_routine) ( int );

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

uint32 mi_disp = 0x40;
uint32 ma_disp = 0x80;

uint8 ywait_beam = 0x0000;
uint8 xwait_beam = 0x0000;

uint8 ywait_beam_enable = 0xFF;
uint8 xwait_beam_enable = 0x7F;

static uint32 offset;

uint64 *dest_data = NULL;
char *beam_source_data = NULL;

void update_display_offsets()
{
	update_ddf(hires);

	if (hires)
	{
		display_scale_x = 1;
		display_scale_y = 2;
		pixels_per_chunk = 8;
		num_chunks = 2;
		clock_speed = 2;
		gfx_shift = 0;

		switch (display_scale_x)
		{
			case 1:
					plot4_color0_fn = plot4_color0_scale1; 
					plot4_bitmap_fn = (CAST_PLOT4) plot4_scale1; 
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
		gfx_shift = 1;

		switch (display_scale_x)
		{
			case 1:
					plot4_color0_fn = plot4_color0_scale1; 
					plot4_bitmap_fn = (CAST_PLOT4) plot4_scale1; 
					break;
			case 2:
					plot4_color0_fn = plot4_color0_scale2;
					plot4_bitmap_fn = plot4_scale2; 
					break;
		}
	}

	// ddf_mix is clocks... from edge.

	display_offset_x = -( 7 * 16 * display_scale_x) + DIW_DDF_OFFSET;

	mi_disp = dispwindow.x0 / 2;
	ma_disp = dispwindow.x1  /2 ;

	display_chunk_offset = pixels_per_chunk  * display_scale_x;
	display_chunk16_offset = 16 * display_scale_x;
}

void update_ddf( int is_hires )
{
	if (is_hires)
	{
		def_min_edge = DispDataFetchWordCount( is_hires,0,DispWinToDispDataFetch(is_hires, 0x7F));	
		def_max_edge = def_min_edge + (640/16) + 4;	// hires is more compressed so need more data.
	}
	else
	{
		def_min_edge = DispDataFetchWordCount( is_hires,0, DispWinToDispDataFetch(is_hires, 0x7F)) ;	
		def_max_edge = def_min_edge + (320/16) + 2;
	}

	ddf_wc = DispDataFetchWordCount( is_hires, ddfstart, ddfstop) ;
	ddf_mix = DispDataFetchWordCount( is_hires, 0,ddfstart);

	ddf_max = ddf_mix + ddf_wc ;

	clearBeamFlags();

	if (is_hires)
	{
		setBeamFlag( 4 , 4 + (320/16) + 4, f_window );
	}
	else
	{
		setBeamFlag( 8 , 8 + (640/16) + 8, f_window );
	}

	setBeamFlag( ddf_mix,  ddf_max , f_ddf );
	setBeamFlag( def_min_edge, def_max_edge, f_display );

	beamParts = decodeBeam();


#if debug==1
	printBeamInfo( beamParts );
	getchar();
#endif
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
//	DebugPrintF("wait Cop: %08x at beam_y %d\n",data.d32, beam_y.b0);

	if (data.d32 == 0xFFFFFFFE)
	{
		if (beam_y.b32 >= 255)
		{
			ywait_beam = (286 - 255);
			xwait_beam = 0x7C ;

			ywait_beam_enable = 0xFF;
			xwait_beam_enable = 0x7F;
			return;
		}
	}


	ywait_beam = data.d16.a >> 8;
	xwait_beam = data.d16.a >> 1 & 0x7F;

	// x is 7bit, bit 0 used for somethng else..
	ywait_beam_enable = data.d16.b >> 8;
	xwait_beam_enable = data.d16.b >> 1 & 0x7F;

	if ((ywait_beam_enable != 0) && (xwait_beam_enable != 0))
	{
		// x is 7bit, bit 0 used for somethng else..
		ywait_beam = (data.d16.a >> 8 ) & ywait_beam_enable;
		xwait_beam = (data.d16.a >> 7) & xwait_beam_enable;
	}
	else
	{
		// x is 7bit, bit 0 used for somethng else..
		ywait_beam_enable = data.d16.a >> 8;
		xwait_beam_enable = data.d16.a >> 1 & 0x7F;
	}
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
//	DebugPrintF("Cop_move Reg %04x, data %04x\n",data.d16.a,data.d16.b);

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
					update_display_offsets();
					break;

		case DDFSTOP: ddfstop = data.d16.b; 
					update_display_offsets();
					break;

		case COPJMP1: ptr = (union cop *) COP1LC -1;
					break;

		case COPJMP2: ptr = (union cop *) COP2LC -1;
					break;

		case COLOR00:	//	DebugPrintF("%04x,%04x\n",data.d16.a,data.d16.b);
						 setPalette(0,ecs2argb[data.d16.b].argb);	 break;
		case COLOR01: setPalette(1,ecs2argb[data.d16.b].argb);	 break;
		case COLOR02: setPalette(2,ecs2argb[data.d16.b].argb);	 break;
		case COLOR03: setPalette(3,ecs2argb[data.d16.b].argb);	 break;
		case COLOR04: setPalette(4,ecs2argb[data.d16.b].argb);	 break;
		case COLOR05: setPalette(5,ecs2argb[data.d16.b].argb);	 break;
		case COLOR06: setPalette(6,ecs2argb[data.d16.b].argb);	 break;
		case COLOR07: setPalette(7,ecs2argb[data.d16.b].argb);	 break;

		case COLOR08: setPalette(8,ecs2argb[data.d16.b].argb);	 break;
		case COLOR09: setPalette(9,ecs2argb[data.d16.b].argb);	 break;
		case COLOR10: setPalette(10,ecs2argb[data.d16.b].argb);	 break;
		case COLOR11: setPalette(11,ecs2argb[data.d16.b].argb);	 break;
		case COLOR12: setPalette(12,ecs2argb[data.d16.b].argb);	 break;
		case COLOR13: setPalette(13,ecs2argb[data.d16.b].argb);	 break;
		case COLOR14: setPalette(14,ecs2argb[data.d16.b].argb);	 break;
		case COLOR15: setPalette(15,ecs2argb[data.d16.b].argb);	 break;

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

					s_odd = (data.d16.b  & 0xF0) >> 4;
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

#define in_window_y(y) ((dispwindow.y0<=y) && (y<dispwindow.y1))

void convert16( char *data)
{
	planar_routine( (uint32 *) data );
	planar_routine( (uint32 *) (data + 8) );
}

APTR lock;
ULONG dest_format;

void is_bad_access( uint32 addr)
{
	if ((first_addr > addr)  ||  addr >= last_addr  ) 
	{
		DebugPrintF("bad access\n"); 
	};
}


union argb_u ecs2argb[0x10000];

void cop_move_(uint16 reg, uint16 data)
{
	union cop c;
	c.d16.a = reg;
	c.d16.b = data;
	cop_move( c );
}

int to_draw_count = 0;

bool beam_y_is_visible = true;

struct ffdpart *bInfo;

void __render2()
{
	int dwy;

	int sxy,wxy;
	int final;


//	DebugPrintF("wait_beam %d,%d\n",(int) xwait_beam,(int) ywait_beam); 

	if (ywait_beam != beam_y.b0)
	{
		dwy = ( ywait_beam > beam_y.b0 ) ? 
			ywait_beam - beam_y.b0 : 
			((int) ywait_beam + (int) 255 - (int) beam_y.b0 ) ;
	}
	else
	{
		dwy = 0;
	}

	sxy = (0 * beam_bpr + beam_x.low) ;	
	wxy = (dwy * beam_bpr + xwait_beam);	

//	DebugPrintF("sxy %08x wxy %08x\n",sxy,wxy); 

	bInfo = bInfos;
	final = wxy - sxy;

	if (final<0)
	{
			if (to_draw_count)
			{
				do 
				{
					dest_data = plot4_fn( beam_source_data, dest_data );
					beam_source_data += pixels_per_chunk;
				} while (--to_draw_count);
			}
		return;
	}

//	DebugPrintF("render %d words\n",final); 

	beam_remain = final;
	do
	{
		if (beam_remain)	// one beam_remain is 4 clocks... we have time to draw all, before fetch...
		{
//			DebugPrintF("to_draw_count %d\n",to_draw_count); 

			if (to_draw_count)
			{
				do 
				{
					dest_data = plot4_fn( beam_source_data, dest_data );
					beam_source_data += pixels_per_chunk;
				} while (--to_draw_count);
			}
		}
		else	// only time to draw a part... (this should be the last part...)
		{
			if (to_draw_count)
			{
				to_draw_count--;
				dest_data = plot4_fn( beam_source_data, dest_data );
				beam_source_data += pixels_per_chunk;
			}
		}

		sync_beam();

		if (bInfo -> fn)
		{
			bInfo -> fn(bInfo);
			if (beam_x.b32 == beam_bpr)
			{
				beam_x.b32 = 0;

				beam_y.b32++;
				draw_y = (beam_y.b32-display_y)*display_scale_y;


//				DebugPrintF("bream_y: %d color0: %08x\n", beam_y, ecs2argb[0].argb);

				if ((draw_y<0) || (draw_y>480))
				{
					beam_hidden();
				}
				else		// someting to display...
				{
					if (in_window_y(beam_y.b32))
					{
						beam_displayed_in_window();
					}
					else
					{
						beam_displayed();
//						beam_displayed_in_window();
					}
					dest_data = dest_ptr_image + draw_y * dest_bpr;
				}
			}
		}
	}
	while (beam_remain>0);

//	DebugPrintF("render chunk done\n"); 
}

void render_copper(struct Custom *custom, uint32 *copperList, struct BitMap *bm )
{
	char ck = '*';
	bool beam_wait = false;
	bool wtf = false;

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
	beam_x.b32 = 0;
	beam_y.b32 = 0;
	beam_wordpos = 0;

	xwait_beam_enable = 0;
	ywait_beam_enable = 0;
	xwait_beam = 0;
	ywait_beam = 0;

	lock = LockBitMapTags( bm,
		LBM_PixelFormat, &dest_format,
		LBM_BytesPerRow, &dest_bpr,
		LBM_BaseAddress, &dest_ptr_image,
		TAG_END	 );

	if (lock)

	{
		first_addr = (uint32) dest_ptr_image;
		last_addr = first_addr + ( dest_bpr * bm -> Rows);

		for (;;)
		{
			switch (ptr -> d32 & 0x00010001)
			{
				case 0x00000000:
				case 0x00000001:	cop_move( *ptr ); break;
				case 0x00010000:	cop_wait( *ptr);  
								beam_clock = 4;	// force a no wait...
								break;
				case 0x00010001:	cop_skip( *ptr); break;
			}

			if (beam_clock++ >= 4)	// we can do 4 copper commands for etch fetch...
			{
				beam_clock = 0;
				__render2();
			}
			else		// for every clock we can draw some part of a line...
			{
				if (to_draw_count)
				{
					to_draw_count--;
					dest_data = plot4_fn( beam_source_data, dest_data );
					beam_source_data += pixels_per_chunk;
				}
			}

			if (ptr -> d32 == 0xFFFFFFFE) break;
			ptr ++;
		}

		UnlockBitMap(lock);
	}

//	DebugPrintF("exit at beam_y %d\n",beam_y.b32);
}



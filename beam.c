
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

extern uint32_t beam_x,beam_y;
extern char *beam_source_data;

#include "beam.h"

uint32_t beam_remain = 0;
int beamParts;
char beamInfo[beam_bpr];

#define dprintf(fmt,...)


extern char plane_data[16];
extern unsigned char *bp0ptr,*bp1ptr,*bp2ptr,*bp3ptr,*bp4ptr,*bp5ptr,*bp6ptr,*bp7ptr;
extern void move_none ( int n ) ;

void (*move_routine) (int) = move_none;

extern void (*plot4_fn)( int x, int y, char *data );			// current function

extern void (*plot4_fn)( int x, int y, char *data );			// current function
extern void (*plot4_color0_fn)( int x, int y, char *data ) ;		// current default color0 option function
extern void (*plot4_bitmap_fn)( int x, int y, char *data ) ;		// current default bitmap option function

extern int to_draw_count;

// *** not displayed ***

void fn_skip (struct ffdpart *this)
{
	uint32_t startAt = beam_x - this -> wcStart;
	uint32_t move = this -> wc - startAt;
	if (beam_remain<move) move = beam_remain;

	dprintf("%-5d - %s: bp0ptr: %08x - moves left %d\n",beam_x,__FUNCTION__,bp0ptr, move);

	to_draw_count = 0;
	beam_x += move;
	beam_remain -= move;
}

void fn_window (struct ffdpart *this)
{
	uint32_t startAt = beam_x - this -> wcStart;
	uint32_t move = this -> wc - startAt;
	if (beam_remain<move) move = beam_remain;

	dprintf("%-5d - %s: bp0ptr: %08x - moves left %d\n",beam_x,__FUNCTION__,bp0ptr, move);

	to_draw_count = 0;	
	beam_x += move;
	beam_remain -= move;
}

void fn_ddf (struct ffdpart *this)
{
	uint32_t startAt = beam_x - this -> wcStart;
	uint32_t move = this -> wc - startAt;
	if (beam_remain<move) move = beam_remain;

	dprintf("%-5d - %s: bp0ptr: %08x, moves left %d\n",beam_x,__FUNCTION__,bp0ptr, move);

	move_routine( move * 2 );	
	to_draw_count = 0;	

	beam_x += move;
	beam_remain -= move;
}

void fn_window_ddf (struct ffdpart *this)
{
	uint32_t startAt = beam_x - this -> wcStart;
	uint32_t move = this -> wc - startAt;
	if (beam_remain<move) move = beam_remain;

	dprintf("%-5d - %s: bp0ptr: %08x - moves left %d\n",beam_x,__FUNCTION__,bp0ptr, move);

	move_routine( move * 2 );		// not inside display ... no need to convert.
	to_draw_count = 0;

	beam_x += move;
	beam_remain -= move;
}

// ****** display *******

void fn_disp (struct ffdpart *this)
{
	uint32_t startAt = beam_x - this -> wcStart;
	uint32_t move = this -> wc - startAt;
	if (beam_remain<move) move = beam_remain;

	dprintf("%-5d - %s: bp0ptr: %08x - moves left %d\n",beam_x,__FUNCTION__,bp0ptr, move);

	plot4_fn = plot4_color0_fn;
	to_draw_count = 4;			// 2 nibbles per byte, 2 bytes == 16bit.

	beam_x += 1;
	beam_remain -= 1;
}

void fn_disp_window (struct ffdpart *this)
{
	uint32_t startAt = beam_x - this -> wcStart;
	uint32_t move = this -> wc - startAt;
	if (beam_remain<move) move = beam_remain;

	dprintf("%-5d - %s: bp0ptr: %08x - moves left %d\n",beam_x,__FUNCTION__,bp0ptr, move);

	plot4_fn = plot4_color0_fn;
	to_draw_count = 4;			// 2 nibbles per byte, 2 bytes == 16bit.

	beam_x += 1;
	beam_remain -= 1;
}

void fn_disp_ddf (struct ffdpart *this)
{
	uint32_t startAt = beam_x - this -> wcStart;
	uint32_t move = this -> wc - startAt;
	if (beam_remain<move) move = beam_remain;

	dprintf("%-5d - %s: bp0ptr: %08x - moves left %d\n",beam_x,__FUNCTION__,bp0ptr, move);

	move_routine( 1 * 2 );	// not inside window ... no need to convert.
	plot4_fn = plot4_color0_fn;
	to_draw_count = 4;

	beam_x += 1;
	beam_remain -= 1;
}

void fn_disp_window_ddf (struct ffdpart *this)
{
	uint32_t startAt = beam_x - this -> wcStart;
	uint32_t move = this -> wc - startAt;
	if (beam_remain<move) move = beam_remain;

	dprintf("%-5d - %s: bp0ptr: %08x - moves left %d\n",beam_x,__FUNCTION__,bp0ptr, move);

	convert16( plane_data );
	beam_source_data = plane_data;
	plot4_fn = plot4_bitmap_fn;
	to_draw_count = 4;

	beam_x += 1;
	beam_remain -= 1;
}

// ******* table *******

void *fns_hidden[]=
{
	fn_skip,
	fn_window,
	fn_ddf,
	fn_window_ddf,	
	fn_skip,			// fn_disp,
	fn_window,		// fn_disp_window,
	fn_ddf,			// fn_disp_ddf,
	fn_window_ddf,		// fn_disp_window_ddf	
};

void *fns_displayed[]=
{
	fn_skip,
	fn_skip,
	fn_skip,
	fn_skip,	
/*
	fn_disp,
	fn_disp_window,
	fn_disp_ddf,
	fn_disp_window_ddf	
*/

	fn_disp,			
	fn_disp,			// not in window
	fn_disp,			// no fetch.. not in window...
	fn_disp			// no fetch.. not in window..

};

void *fns_displayed_in_window[]=
{
	fn_skip,
	fn_window,
	fn_ddf,
	fn_window_ddf,	
	fn_disp,
	fn_disp_window,
	fn_disp_ddf,
	fn_disp_window_ddf	
};

enum beam_state
{
	hidden,
	displayed,
	displayed_in_window
} ;

void **fns_tabels[] =
{
	fns_hidden,
	fns_displayed,
	fns_displayed_in_window
};

// ******* names *******

const char *beam_flag_names[]=
{
	"skip",
	"window",
	"ddf",
	"window,ddf",
	"disp",
	"disp,window",
	"disp,ddf",
	"disp,window,ddf	"
};

struct ffdpart bInfos[128];

void printBeamInfo( int items )
{
	int n;
	struct ffdpart *bInfo;

	for (bInfo = bInfos,n=0;n<items;n++,bInfo++)
	{
		dprintf("wcStart: %-3d, wcEnd %-3d, wc: %-3d, flags: %s (%d), fn: %x\n",
			bInfo -> wcStart ,
			bInfo -> wcEnd ,
			bInfo -> wc ,
			beam_flag_names[bInfo -> flags] , bInfo -> flags,
			bInfo -> fn );
	}
}



enum beam_state beam_displyed = hidden;

int decodeBeam()
{
	struct ffdpart *bInfo;
	int wc = 0;
	int lf;
	int f = beamInfo[0]; 
	int x;

	bInfo = bInfos;
	bInfo -> wcStart = 0;

	for (x = 0; x<beam_bpr;x++)
	{
		lf = f;
		f =  beamInfo[x] ;

		if (lf != f)
		{
			bInfo -> wc = wc;
			bInfo -> wcEnd = x;
			bInfo -> flags = lf;
			bInfo -> fn = fns_tabels[ 0 ][ lf ];
			bInfo++;
			bInfo -> wcStart = x;
			wc = 1;
		}
		else wc++;		
	}

	bInfo -> wc = wc;
	bInfo -> wcEnd = x;
	bInfo -> flags = lf;
	bInfo -> fn = fns_tabels[ 0 ][ lf ];

	return bInfo - bInfos +1;
}

void beam_hidden()
{
	if (beam_displyed != hidden )
	{
		struct ffdpart *bInfo;

		for (bInfo=bInfos;bInfo < bInfos + beamParts; bInfo++)
		{
			bInfo -> fn = fns_hidden[ bInfo -> flags ];
		}
		beam_displyed  = hidden;
	}
}

void beam_displayed()
{
	if (beam_displyed != displayed)
	{
		struct ffdpart *bInfo;

		for (bInfo=bInfos;bInfo < bInfos + beamParts; bInfo++)
		{
			bInfo -> fn = fns_displayed[ bInfo -> flags ];
		}
		beam_displyed  = displayed;
	}
}

void beam_displayed_in_window()
{
	if (beam_displyed != displayed_in_window)
	{
		struct ffdpart *bInfo;

		for (bInfo=bInfos;bInfo < bInfos + beamParts; bInfo++)
		{
			bInfo -> fn = fns_displayed_in_window[ bInfo -> flags ];
		}
		beam_displyed  = displayed_in_window;
	}
}


void sync_beam()
{
	while (( beam_x <  bInfo -> wcStart  ) || ( beam_x >= bInfo -> wcEnd ))
	{
		bInfo++;
		if (bInfo >= bInfos + beamParts ) bInfo = bInfos;
	}
}

void clearBeamFlags()
{
	int i;
	for (i = 0; i<beam_bpr;i++) beamInfo[i] = 0;
}

void setBeamFlag(int x0,int x1, uint32_t flag )
{
	int x;

	dprintf("%s(%d,%d,%08x)\n",__FUNCTION__,x0,x1,flag);

	for (x = x0; x<x1;x++) beamInfo[x] |= flag;
}


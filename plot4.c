
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

unsigned char *dest_ptr;
unsigned char *dest_ptr_image;
unsigned int dest_bpr;

#include "plot4.h"

union dbPixel palette2[256];

uint64 *plot4_none_fn ( char *source_data, uint64 *dest_data ) { return dest_data; }	// dummy function... no drawing..

uint64 *plot4_scale1( char *source_data, uint32 *dest_data )
{
	*dest_data++ = palette2[ *source_data ++ ].argb1;		// pixel 0
	*dest_data++ = palette2[ *source_data ++ ].argb1;		// pixel 1
	*dest_data++ = palette2[ *source_data ++ ].argb1;		// pixel 2
	*dest_data++ = palette2[ *source_data ++ ].argb1;		// pixel 3
	return (uint64 *) dest_data;
}

uint64 *plot4_scale2( char *source_data , uint64 *dest_data )
{
	*dest_data++ = palette2[ *source_data ++ ].data;		// 0,1
	*dest_data++ = palette2[ *source_data ++ ].data;		// 2,3
	*dest_data++ = palette2[ *source_data ++ ].data;		// 4,5
	*dest_data++ = palette2[ *source_data  ].data;		// 6,7
	return dest_data;
}

uint64 *plot4_color0_scale1( char *source_data, uint64 *dest_data )
{
	uint64 color0 = palette2[0].data;
	*dest_data++ = color0;		// 0,1
	*dest_data++ = color0;		// 2.3
	return dest_data;
}

uint64 *plot4_color0_scale2( char *source_data, uint64 *dest_data )
{
	uint64 color0 = palette2[0].data;
	*dest_data++ = color0;	// pixel 0,1
	*dest_data++ = color0;	// pixel 2,3
	*dest_data++ = color0;	// pixel 4,5
	*dest_data++ = color0;	// pixel 6,7
	return dest_data;
}


//---------------------------------------------------------------------------------------------------

unsigned char dual_playfield_index[256];
unsigned char dual_playfield_index2[256];

void init_dual_playfield_index( int plains )
{
	int b;
	int i;
	int v;
	int playfield1_colors = (1 << (plains/2));
//	int playfield1_colors = 8;

	for (i=0;i<0x10;i++)
	{
		v=0;
		for (b=4;b>-1;b--)
		{
			v |= (i & (1<<b)) ? 1<<(b*2) : 0;
		}
		dual_playfield_index[v<<1]= i + playfield1_colors;
		dual_playfield_index[v] = i;
	}
}

void init_dual_playfield_index2()
{
	int i;

	for (i=0;i<256;i++)
	{
		if (i & 0xAA)
		{
			dual_playfield_index2[i] = dual_playfield_index[i & 0xAA] ;
		}
		else		// is plane 1
		{
			dual_playfield_index2[i] =  dual_playfield_index[i & 0x55] ;
		}
	}
}


//---------------------------------------------------------------------------------------


#define dualPalette(color) palette2[ dual_playfield_index2[color] ].argb1
#define dualPalette2(color) palette2[ dual_playfield_index2[color] ].data;

uint64 *plot4_playfield_scale1( char *source_data, uint32 *dest_data )
{
	*dest_data++ = dualPalette( *source_data ++ );		// pixel 0
	*dest_data++ = dualPalette( *source_data ++ );		// pixel 1
	*dest_data++ = dualPalette( *source_data ++ );		// pixel 2
	*dest_data++ = dualPalette( *source_data ++ );		// pixel 3
	return (uint64 *) dest_data;
}

uint64 *plot4_playfield_scale2( char *source_data , uint64 *dest_data )
{
	*dest_data++ = dualPalette2( *source_data ++ );		// 0,1
	*dest_data++ = dualPalette2( *source_data ++ );		// 2,3
	*dest_data++ = dualPalette2( *source_data ++ );		// 4,5
	*dest_data++ = dualPalette2( *source_data  );		// 6,7
	return dest_data;
}



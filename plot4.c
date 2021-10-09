
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


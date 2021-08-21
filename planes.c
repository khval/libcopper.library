
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>

#define lpage (256*8)

unsigned char bits2bytes_data[lpage*8];

unsigned char *bits2bytes[256*8];

unsigned char **bits2bytes0 = bits2bytes;
unsigned char **bits2bytes1 = bits2bytes+(256*1);
unsigned char **bits2bytes2 = bits2bytes+(256*2);
unsigned char **bits2bytes3 = bits2bytes+(256*3);
unsigned char **bits2bytes4 = bits2bytes+(256*4);
unsigned char **bits2bytes5 = bits2bytes+(256*5);
unsigned char **bits2bytes6 = bits2bytes+(256*6);
unsigned char **bits2bytes7 = bits2bytes+(256*7);

void initBits2Bytes()
{
	unsigned int p,n,b;
	unsigned char *page;
	unsigned char *at;

	for (p=0;p<8;p++)
	{
		page = bits2bytes_data + lpage * p;

		for (n=0;n<256;n++) 
		{
			at= page + n*8;
			for (b=0; b<8;b++) at[7-b] = n & 1L<<b ? 1L<<p: 0;		// we reverse the bits.
			bits2bytes[256*p+n] = at;
		}
	}
}

unsigned char *bp0ptr,*bp1ptr,*bp2ptr,*bp3ptr,*bp4ptr,*bp5ptr,*bp6ptr,*bp7ptr;

uint64 draw_1p()
{
	uint64 data = *((uint64 *) bits2bytes[*bp0ptr]);
	bp0ptr++;
	return data;
}

uint64 draw_2p()
{
	uint64 data;
	printf("%08x %08x\n", bp0ptr,bp1ptr);

	data =	*((uint64 *) bits2bytes0[*bp0ptr]);
	data |=	*((uint64 *) bits2bytes1[*bp1ptr]);

	bp0ptr++;bp1ptr++;
	return data;
}

uint64 draw_3p()
{
	uint64 data =	*((uint64 *) bits2bytes0[*bp0ptr]);
	data |=		*((uint64 *) bits2bytes1[*bp1ptr]);
	data |=		*((uint64 *) bits2bytes2[*bp2ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;
	return data;
}

uint64 draw_4p()
{
	uint64 data =	*((uint64 *) bits2bytes0[*bp0ptr]); 
	data |=		*((uint64 *) bits2bytes1[*bp1ptr]); 
	data |=		*((uint64 *) bits2bytes2[*bp2ptr]); 
	data |=		*((uint64 *) bits2bytes3[*bp3ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;bp3ptr++;
	return data;
}

uint64 draw_5p()
{
	uint64 data = *((uint64 *) bits2bytes0[*bp0ptr]); 
	data |= *((uint64 *) bits2bytes1[*bp1ptr]); 
	data |= *((uint64 *) bits2bytes2[*bp2ptr]); 
	data |= *((uint64 *) bits2bytes3[*bp3ptr]); 
	data |=*((uint64 *) bits2bytes4[*bp4ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;bp3ptr++;bp4ptr++;
	return data;
}

uint64 draw_6p()
{
	uint64 data = *((uint64 *) bits2bytes0[*bp0ptr]); 
	data |= *((uint64 *) bits2bytes1[*bp1ptr]); 
	data |= *((uint64 *) bits2bytes2[*bp2ptr]); 
	data |= *((uint64 *) bits2bytes3[*bp3ptr]); 
	data |= *((uint64 *) bits2bytes4[*bp4ptr]); 
	data |= *((uint64 *) bits2bytes5[*bp5ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;bp3ptr++;bp4ptr++;bp5ptr++;
	return data;
}

uint64 draw_7p()
{
	uint64 data = *((uint64 *) bits2bytes0[*bp0ptr]); 
	data |= *((uint64 *) bits2bytes1[*bp1ptr]); 
	data |= *((uint64 *) bits2bytes2[*bp2ptr]); 
	data |= *((uint64 *) bits2bytes3[*bp3ptr]); 
	data |= *((uint64 *) bits2bytes4[*bp4ptr]); 
	data |= *((uint64 *) bits2bytes5[*bp5ptr]); 
	data |= *((uint64 *) bits2bytes6[*bp6ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;bp3ptr++;bp4ptr++;bp5ptr++;bp6ptr++;
	return data;
}

uint64 draw_8p()
{
	uint64 data = *((uint64 *) bits2bytes0[*bp0ptr]); 
	data |= *((uint64 *) bits2bytes1[*bp1ptr]); 
	data |= *((uint64 *) bits2bytes2[*bp2ptr]); 
	data |= *((uint64 *) bits2bytes3[*bp3ptr]); 
	data |= *((uint64 *) bits2bytes4[*bp4ptr]); 
	data |= *((uint64 *) bits2bytes5[*bp5ptr]); 
	data |= *((uint64 *) bits2bytes6[*bp6ptr]); 
	data |= *((uint64 *) bits2bytes7[*bp7ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;bp3ptr++;bp4ptr++;bp5ptr++;bp6ptr++;bp7ptr++;
	return data;
};

// basicly unrolled routines.

void *planar_routines[]  =
{
	NULL,
	draw_1p,
	draw_2p,
	draw_3p,
	draw_4p,
	draw_5p,
	draw_6p,
	draw_7p,
	draw_8p
};


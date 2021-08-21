
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

void convert_1p(uint64 *data)
{
	*data = *((uint64 *) bits2bytes[*bp0ptr]);
	bp0ptr++;
}

void convert_2p(uint64 *data)
{
	*data =	*((uint64 *) bits2bytes0[*bp0ptr])
		| *((uint64 *) bits2bytes1[*bp1ptr]);

	bp0ptr++;bp1ptr++;
}

void convert_3p(uint64 *data)
{
	*data =	*((uint64 *) bits2bytes0[*bp0ptr])
		| *((uint64 *) bits2bytes1[*bp1ptr]) 
		| *((uint64 *) bits2bytes2[*bp2ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;
}

void convert_4p(uint64 *data)
{
	*data =	*((uint64 *) bits2bytes0[*bp0ptr])
		| *((uint64 *) bits2bytes1[*bp1ptr]) 
		| *((uint64 *) bits2bytes2[*bp2ptr]) 
		| *((uint64 *) bits2bytes3[*bp3ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;bp3ptr++;
}

void convert_5p(uint64 *data)
{
	*data = *((uint64 *) bits2bytes0[*bp0ptr]) 
		| *((uint64 *) bits2bytes1[*bp1ptr]) 
		| *((uint64 *) bits2bytes2[*bp2ptr]) 
		| *((uint64 *) bits2bytes3[*bp3ptr]) 
		| *((uint64 *) bits2bytes4[*bp4ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;bp3ptr++;bp4ptr++;
}

void convert_6p(uint64 *data)
{
	*data = *((uint64 *) bits2bytes0[*bp0ptr])
		| *((uint64 *) bits2bytes1[*bp1ptr]) 
		| *((uint64 *) bits2bytes2[*bp2ptr]) 
		| *((uint64 *) bits2bytes3[*bp3ptr]) 
		| *((uint64 *) bits2bytes4[*bp4ptr]) 
		| *((uint64 *) bits2bytes5[*bp5ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;bp3ptr++;bp4ptr++;bp5ptr++;
}

void convert_7p(uint64 *data)
{
	*data = *((uint64 *) bits2bytes0[*bp0ptr])
		| *((uint64 *) bits2bytes1[*bp1ptr]) 
		| *((uint64 *) bits2bytes2[*bp2ptr]) 
		| *((uint64 *) bits2bytes3[*bp3ptr]) 
		| *((uint64 *) bits2bytes4[*bp4ptr]) 
		| *((uint64 *) bits2bytes5[*bp5ptr])
		| *((uint64 *) bits2bytes5[*bp6ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;bp3ptr++;bp4ptr++;bp5ptr++;bp6ptr++;
}

void convert_8p(uint64 *data)
{
	*data = *((uint64 *) bits2bytes0[*bp0ptr])
		|  *((uint64 *) bits2bytes1[*bp1ptr])
		|  *((uint64 *) bits2bytes2[*bp2ptr])
		|  *((uint64 *) bits2bytes3[*bp3ptr])
		|  *((uint64 *) bits2bytes4[*bp4ptr])
		|  *((uint64 *) bits2bytes5[*bp5ptr])
		|  *((uint64 *) bits2bytes6[*bp6ptr])
		|  *((uint64 *) bits2bytes7[*bp7ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;bp3ptr++;bp4ptr++;bp5ptr++;bp6ptr++;bp7ptr++;
};

// basicly unrolled routines.

uint64 convert_none()
{
	return 0;
}

void *planar_routines[]  =
{
	convert_none,
	convert_1p,
	convert_2p,
	convert_3p,
	convert_4p,
	convert_5p,
	convert_6p,
	convert_7p,
	convert_8p
};


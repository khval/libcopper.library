
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

static void convert_1p(uint64 *data)
{
//	printf("** bp0ptr %08x\n",bp0ptr);

	*data = *((uint64 *) bits2bytes[*bp0ptr]);
	bp0ptr++;
}

static void convert_2p(uint64 *data)
{
	*data =	*((uint64 *) bits2bytes0[*bp0ptr])
		| *((uint64 *) bits2bytes1[*bp1ptr]);

	bp0ptr++;bp1ptr++;
}

static void convert_3p(uint64 *data)
{
	*data =	*((uint64 *) bits2bytes0[*bp0ptr])
		| *((uint64 *) bits2bytes1[*bp1ptr]) 
		| *((uint64 *) bits2bytes2[*bp2ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;
}

static void convert_4p(uint64 *data)
{
	*data =	*((uint64 *) bits2bytes0[*bp0ptr])
		| *((uint64 *) bits2bytes1[*bp1ptr]) 
		| *((uint64 *) bits2bytes2[*bp2ptr]) 
		| *((uint64 *) bits2bytes3[*bp3ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;bp3ptr++;
}

static void convert_5p(uint64 *data)
{
	*data = *((uint64 *) bits2bytes0[*bp0ptr]) 
		| *((uint64 *) bits2bytes1[*bp1ptr]) 
		| *((uint64 *) bits2bytes2[*bp2ptr]) 
		| *((uint64 *) bits2bytes3[*bp3ptr]) 
		| *((uint64 *) bits2bytes4[*bp4ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;bp3ptr++;bp4ptr++;
}

static void convert_6p(uint64 *data)
{
	*data = *((uint64 *) bits2bytes0[*bp0ptr])
		| *((uint64 *) bits2bytes1[*bp1ptr]) 
		| *((uint64 *) bits2bytes2[*bp2ptr]) 
		| *((uint64 *) bits2bytes3[*bp3ptr]) 
		| *((uint64 *) bits2bytes4[*bp4ptr]) 
		| *((uint64 *) bits2bytes5[*bp5ptr]);

	bp0ptr++;bp1ptr++;bp2ptr++;bp3ptr++;bp4ptr++;bp5ptr++;
}

static void convert_7p(uint64 *data)
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

static void convert_8p(uint64 *data)
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
	convert_none,convert_1p,convert_2p,convert_3p,convert_4p,convert_5p,convert_6p,convert_7p,convert_8p
};

void move_none(int n) {};
void move_1p(int n) { bp0ptr += n; }
void move_2p(int n) { bp0ptr += n; bp1ptr += n; }
void move_3p(int n) { bp0ptr += n; bp1ptr += n; bp2ptr += n; }
void move_4p(int n) { bp0ptr += n; bp1ptr += n; bp2ptr += n; bp3ptr += n; }
void move_5p(int n) { bp0ptr += n; bp1ptr += n; bp2ptr += n; bp3ptr += n; bp4ptr += n; }
void move_6p(int n) { bp0ptr += n; bp1ptr += n; bp2ptr += n; bp3ptr += n; bp4ptr += n; bp5ptr += n; }
void move_7p(int n) { bp0ptr += n; bp1ptr += n; bp2ptr += n; bp3ptr += n; bp4ptr += n; bp5ptr += n; bp6ptr += n;}
void move_8p(int n) { bp0ptr += n; bp1ptr += n; bp2ptr += n; bp3ptr += n; bp4ptr += n; bp5ptr += n; bp6ptr += n; bp7ptr += n;}

void *move_routines[]  =
{
	move_none,move_1p,move_2p,move_3p,move_4p,move_5p,move_6p,move_7p,move_8p
};



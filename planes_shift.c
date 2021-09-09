
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>

#define lpage (256*8)

extern unsigned char bits2bytes_data[lpage*8];
extern unsigned char *bits2bytes[256*8];

extern unsigned char **bits2bytes0;
extern unsigned char **bits2bytes1;
extern unsigned char **bits2bytes2;
extern unsigned char **bits2bytes3;
extern unsigned char **bits2bytes4;
extern unsigned char **bits2bytes5;
extern unsigned char **bits2bytes6;
extern unsigned char **bits2bytes7; 

extern unsigned char *bp0ptr,*bp1ptr,*bp2ptr,*bp3ptr,*bp4ptr,*bp5ptr,*bp6ptr,*bp7ptr;

uint32	_copper_tmp0,
		_copper_tmp1,
		_copper_tmp2,
		_copper_tmp3,
		_copper_tmp4,
		_copper_tmp5,
		_copper_tmp6,
		_copper_tmp7,
		_copper_tmp8;

uint32 s_even;
uint32 s_odd;

#define shift_odd(p) uint8 tmp8b ## p ; _copper_tmp ## p <<= 8; _copper_tmp ## p |=  *bp ## p ## ptr ++ ; tmp8b ## p =  (_copper_tmp ## p  >> s_odd);
#define shift_even(p) uint8 tmp8b ## p ; _copper_tmp ## p <<= 8; _copper_tmp ## p |=  *bp ## p ## ptr ++ ; tmp8b ## p =  (_copper_tmp ## p  >> s_even);

void reset_copper_tmp()
{
	_copper_tmp0 = 0;
	_copper_tmp1 = 0;
	_copper_tmp2 = 0;
	_copper_tmp3 = 0;
	_copper_tmp4 = 0;
	_copper_tmp5 = 0;
	_copper_tmp6 = 0;
	_copper_tmp7 = 0;
	_copper_tmp8 = 0;
}

static void convert_1p(uint64 *data)
{
	shift_even(0);

	*data = *((uint64 *) bits2bytes[tmp8b0]);
}

static void convert_2p(uint64 *data)
{
	shift_even(0);
	shift_odd(1);

	*data = *((uint64 *) bits2bytes0[tmp8b0])
		|  *((uint64 *) bits2bytes1[tmp8b1]);
}

static void convert_3p(uint64 *data)
{
	shift_even(0);
	shift_odd(1);
	shift_even(2);

	*data = *((uint64 *) bits2bytes0[tmp8b0])
		|  *((uint64 *) bits2bytes1[tmp8b1])
		|  *((uint64 *) bits2bytes2[tmp8b2]);
}

static void convert_4p(uint64 *data)
{
	shift_even(0);
	shift_odd(1);
	shift_even(2);
	shift_odd(3);

	*data = *((uint64 *) bits2bytes0[tmp8b0])
		|  *((uint64 *) bits2bytes1[tmp8b1])
		|  *((uint64 *) bits2bytes2[tmp8b2])
		|  *((uint64 *) bits2bytes3[tmp8b3]);
}

static void convert_5p(uint64 *data)
{
	shift_even(0);
	shift_odd(1);
	shift_even(2);
	shift_odd(3);
	shift_even(4);

	*data = *((uint64 *) bits2bytes0[tmp8b0])
		|  *((uint64 *) bits2bytes1[tmp8b1])
		|  *((uint64 *) bits2bytes2[tmp8b2])
		|  *((uint64 *) bits2bytes3[tmp8b3])
		|  *((uint64 *) bits2bytes4[tmp8b4]);
}

static void convert_6p(uint64 *data)
{
	shift_even(0);
	shift_odd(1);
	shift_even(2);
	shift_odd(3);
	shift_even(4);
	shift_odd(5);

	*data = *((uint64 *) bits2bytes0[tmp8b0])
		|  *((uint64 *) bits2bytes1[tmp8b1])
		|  *((uint64 *) bits2bytes2[tmp8b2])
		|  *((uint64 *) bits2bytes3[tmp8b3])
		|  *((uint64 *) bits2bytes4[tmp8b4])
		|  *((uint64 *) bits2bytes5[tmp8b5]);
}

static void convert_7p(uint64 *data)
{
	shift_even(0);
	shift_odd(1);
	shift_even(2);
	shift_odd(3);
	shift_even(4);
	shift_odd(5);
	shift_even(6);

	*data = *((uint64 *) bits2bytes0[tmp8b0])
		|  *((uint64 *) bits2bytes1[tmp8b1])
		|  *((uint64 *) bits2bytes2[tmp8b2])
		|  *((uint64 *) bits2bytes3[tmp8b3])
		|  *((uint64 *) bits2bytes4[tmp8b4])
		|  *((uint64 *) bits2bytes5[tmp8b5])
		|  *((uint64 *) bits2bytes6[tmp8b6]);
}

static void convert_8p(uint64 *data)
{
	shift_even(0);
	shift_odd(1);
	shift_even(2);
	shift_odd(3);
	shift_even(4);
	shift_odd(5);
	shift_even(6);
	shift_odd(7);

	*data = *((uint64 *) bits2bytes0[tmp8b0])
		|  *((uint64 *) bits2bytes1[tmp8b1])
		|  *((uint64 *) bits2bytes2[tmp8b2])
		|  *((uint64 *) bits2bytes3[tmp8b3])
		|  *((uint64 *) bits2bytes4[tmp8b4])
		|  *((uint64 *) bits2bytes5[tmp8b5])
		|  *((uint64 *) bits2bytes6[tmp8b6])
		|  *((uint64 *) bits2bytes7[tmp8b7]);
};

// basicly unrolled routines.

extern uint64 convert_none();

void *planar_shift_routines[]  =
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


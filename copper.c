
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <proto/intuition.h>
#include <proto/graphics.h>

typedef unsigned short uint16;
//typedef unsigned int uint32;		// found in exec/types.h

// set some values not importent what values, this is just a test... 

#define INTREQ 2
#define COPJMP1 4
#define COPJMP2 8

// Display Window

#define DIWSTART 10
#define DIWSTOP 12

// Data Fetch, (is in clocks, 4 in lowres, 8 in hires)

#define DDFSTART 14
#define DDFSTOP 16

#define COLOR0 0x180
#define COLOR1 0x182

#define lowres_clock 4

#define setCop(a,b) *ptr++ = ((a)<<16)|(b)

uint32 copperList[1000];
uint32 copperl1;
uint32 copperl2;

uint32 color0;

union cop
{
	uint32 d32;
	struct 
	{
		uint16 a;
		uint16 b;
	} d16;
};

uint32 COP1LC, COP2LC;

void init_copper_list()
{
	uint32 *ptr = (uint32 *) copperList;

copperl1 = (uint32) ptr;

	setCop( COLOR0,0x0F00 );	
	setCop( 0x0F01,0x8F00 );	
	setCop( INTREQ,0x8010 );	
	setCop( 0x00E3,0x80FE );	
	setCop( 0x7F01,0x7F01 );	
	setCop( COPJMP1,0x0000 );	

copperl2 = (uint32) ptr;

	setCop( COLOR0,0x0F0 );	
	setCop( 0x0F01,0x8F00 );	
	setCop( INTREQ,0x8010 );	
	setCop( 0x00E3,0x80FE );
	setCop( 0xFF01,0xFE01 );	
	setCop( COPJMP2,0x0000 );	
	setCop( 0xFFFF,0xFFFE );	

	COP1LC = (copperl1- (uint32) copperList) /4;
	COP2LC = (copperl2- (uint32) copperList) /4;

	printf("COP1LC: %d\n", COP1LC);
	printf("COP2LC: %d\n", COP2LC);
}

uint32 diwstart;
uint32 diwstop;

uint32 ddfstart;
uint32 ddfstop;

uint32 beam_clock = 0;
union cop *ptr;

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

int ly = -1;

void plot( int x,int y)
{
	int wc;
	int miy = (diwstart >> 8);
	int may = (diwstop >> 8) + 0x100;
	int mix , max;

	if (ly!=y) printf("%d - %08x\n",y, color0);

	wc = WordCount( ddfstart, ddfstop) * lowres_clock;

	mix = WordCount( 0,ddfstart) * lowres_clock;
	max = mix + wc ;

	ly = y;

	if ((y>=miy) && (y<=may))
	{
		if ((x>=mix) && (x<=max))
		{
			x *= (16 / lowres_clock) ;
			y *= 2;
			WritePixelColor(rp,x,y,color0);
		}
	}

}

uint32 ecs2argb[0x10000];

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

void cop_move(union cop data)
{
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
					break;

		case DDFSTOP: ddfstop = data.d16.b; 
					break;

		case COPJMP1: ptr = (union cop *) copperList + COP1LC -1; 	break;
		case COPJMP2: ptr = (union cop *) copperList + COP2LC -1; 	break;

		case COLOR0:	color0 = ecs2argb[data.d16.b];
					break;
	}
}

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

void dump_copper()
{
	ptr = (union cop *) copperList;

	for (;ptr -> d32 != 0xFFFFFFFE;ptr++)
	{
//		printf("%-8d, %04x,%04x -- %08x\n",ptr - (union cop *) copperList, ptr -> d16.a,ptr -> d16.b,beam);

		switch (ptr -> d32 & 0x1001)
		{
			case 0x0000:
			case 0x0001:	cop_move( *ptr );	break;
			case 0x1000:
						break;
			case 0x1001:	cop_skip( *ptr);	break;
		}

		beam_clock++;

		plot( beam_clock & 0xFF, beam_clock >> 8 );
	}
}


int main()
{
	printf("sizeof(union cop) == %d\n", sizeof(union cop));

	if (open_libs())
	{
		init_ecs2colors();

		struct Window *win;

		win = OpenWindowTags( NULL, 
			WA_Left,320,
			WA_Top,20,
			WA_Width, 640,
			WA_Height, 520,
			TAG_END);

		if (win)
		{
			rp = win -> RPort;

			cop_move_(DIWSTART,0x2C81);
			cop_move_(DIWSTOP,0xF4C1);

			cop_move_(DDFSTART,0x0038);
			cop_move_(DDFSTOP,0x00D0);

			int wc = WordCount( ddfstart, ddfstop);

			init_copper_list();
			dump_copper();

			printf("data fetch start %d (pixels %d)\n",ddfstart,WordCount( 0,ddfstart)*16);
			printf("data fetch word count %d (pixels %d)\n",wc,wc*16);


			int diwstarty = diwstart >> 8 ;
			int diwstopy = diwstop >> 8 ;

			printf("rows %d\n", (diwstopy & 0x80 ? diwstopy : diwstopy + 0x100) - diwstarty );

			getchar();

			CloseWindow(win);
		}
		
	}

	close_libs();

	return 0;
}


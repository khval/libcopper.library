// OSCopper.e
// Native graphics example using OS friendly copperlist

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#include	<graphics/gfxbase.h>
#include	<graphics/gfxmacros.h>
#include	<graphics/copper.h>
#include	<graphics/view.h>
#include	<hardware/custom.h>
#include	<intuition/intuition.h>
#include	<intuition/screens.h>
#include	<exec/memory.h>

#include "common.h"

#include "render.h"

struct Window *win;

struct bar
{
	unsigned int c;
	int y;
	int h;
	double r;
	double s;
};

struct bar bars[] =
{
	{0x100,30,30,0.0f,0.01f},
	{0x010,40,30,0.0f,0.001f},
	{0x001,30,30,0.0f,0.02f},
	{0x110,40,30,0.0f,0.002f},
	{0x011,30,30,0.0f,0.03f},
	{0x101,40,30,0.0f,0.003f}
};


bool initScreen()
{
	win=OpenWindowTags(NULL,
					WA_IDCMP,IDCMP_MOUSEBUTTONS,
					WA_Left,640,
					WA_Width,640+64,
					WA_Height,400,
					WA_Flags,WFLG_NOCAREREFRESH |
							WFLG_ACTIVATE,
					TAG_END);

	if (!win) return false;

	return true;
}

void errors()
{
	if (!win) Printf("Unable to open window.\n");
}

const char *txt = "Cooper test";

void moveBar( int hh)
{
	struct bar *b;
	for (b = bars; b < bars +(sizeof(bars)/sizeof(struct bar)); b++ )
	{
		b -> y = sin( b -> r ) * hh + hh ;
		b -> r += b -> s;
	}
}


void insideBar( int y, int *c, int *v )
{
	int yy;
	struct bar *b;

	for (b = bars; b < bars +(sizeof(bars)/sizeof(struct bar)); b++ )
	{
		yy = b-> y - y;

		if ((yy >= 0) && (yy < b -> h ))
		{
			yy -= (b->h /2);
			*v= 255 - (abs(yy) * abs(yy));
			if (*v<0) *v = 0;
			*c = b -> c;
			return;
		}
	}
	*c = 0;
	*v = 0;
}

void updateCop( uint32 *cops, int lines )
{
	uint16 *w;
	uint32 *cop = cops;
	int offset1 = 2;
	int segSize = 3;		// five instructions per line
	int y;
	int v;
	int c;

	for (y=0;y<lines;y++)
	{
		cop = (cops + (y*segSize)) ;

		insideBar( y, &c, &v );

		w = (uint16 *) (cop + offset1);
		w[1] = c * (v >> 4);	// set high R colors.
	}
}


void init_copper(int linestart, int height)
{
	uint32 backrgb = 0x000;
	int y;
	uint32 *ptr = copperList;

	// 3 instructions per line, and 3 extra at the end.

	for (y=linestart;y<linestart+height;y++)
	{
		setCop( (y<<8) | 1 , 0xFF00 );			// 0
		setCop(BPLCON3,0);				// 1
		setCop(COLOR00,(y-linestart) & 0xFFF);	// 2
	}
	
	setCop( (y<<8) | 1 ,0);
	setCop(COLOR01,backrgb);
	setCop(0xFFFF,0xFFFE);
}

void closeDown()
{
	if (win) CloseWindow(win);
}

int main_prog()
{
	int y;
	bool quit = false;

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);

	if (initScreen())
	{
		ULONG backrgb;

		int linestart=0;
		int lines=win -> Height;
		int width=win -> Width;
		
		copper_rp=win -> RPort;

		init_ecs2colors();
		init_copper(linestart, win -> Height);

		while ( ! quit )
		{
			moveBar( win -> Height / 2);

			updateCop( copperList, lines );


//			dump_copper( copperList );
			render_copper( copperList, copper_rp );

         		/* Check & clear CTRL_C signal */
			if(SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
		         {
                			quit = true;
		         }

			WaitTOF();
		}
	}
	else
	{
		errors();			 
	}

	closeDown();
	
	return 0;
}

int main()
{
	int ret;

	if (open_libs()==FALSE)
	{
		Printf("failed to open libs!\n");
		close_libs();
		return 0;
	}

	ret = main_prog();

	close_libs();

	return ret;
}




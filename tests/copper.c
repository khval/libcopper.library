
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#include "render.h"
#include <hardware/custom.h>

#ifdef __amigaos4__
struct Custom _custom;
struct Custom *custom = &_custom;	// store locally... handle things with do_functions();
#else
struct Custom *custom = 0xDFF000;
#endif

uint32 *update_copper_list(uint32 *ptr, int nStart)
{

	int n,nn;
	int t;
	int y;

	setCop( COLOR00, 0 );

	for (n=0;n<255;n++)
	{

		if ((n & 0x07 ) == 0)
		{
			y = n  +60;
		 	setCop( (y << 8) | 1, 0xFF00);

			t++;
		}

		nn = n + nStart;

		setCop( COLOR00, (t & 1) ? nn : 0xFFF-nn );
	}

	return ptr;
}

void init_copper_list()
{
	uint32 *ptr = (uint32 *) copperList;

	ptr = update_copper_list(ptr, 0);

	setCop( 0xFFFF,0xFFFE );	

}


int main()
{
	printf("sizeof(union cop) == %d\n", sizeof(union cop));

	if (open_libs())
	{
		int n;
		init_ecs2colors();

		struct Window *win;
		struct BitMap *copperBitmap;

		win = OpenWindowTags( NULL, 
			WA_IDCMP,IDCMP_MOUSEBUTTONS,
			WA_Left,320,
			WA_Top,20,
			WA_Width, 640 + 128,
			WA_Height, 480 + 128,
			TAG_END);

		if (win) copperBitmap =AllocBitMap( win -> Width, win -> Height, 32, BMF_DISPLAYABLE, win ->RPort -> BitMap);

		if ((win)&&(copperBitmap))
		{
			int y = 0;
			bool quit = false;
			struct RastPort rp;
			InitRastPort(&rp);

			rp.BitMap = copperBitmap;

			RectFillColor(&rp, 0, 0, win -> Width, win -> Height, 0xFF000000);

			cop_move_(DIWSTART,0x2C81);
			cop_move_(DIWSTOP,0xF4C1);

			cop_move_(DDFSTART,0x0038);
			cop_move_(DDFSTOP,0x00D0);

			int wc = DispDataFetchWordCount( 0, ddfstart, ddfstop);

			init_copper_list();

			int diwstarty = diwstart >> 8 ;
			int diwstopy = diwstop >> 8 ;

			do
			{
				WaitTOF();
				render_copper( custom, copperList,  copperBitmap );
    				BltBitMapRastPort(  copperBitmap, 0,0, win -> RPort, 0,0, win -> Width, win -> Height, 0xC0 );

				update_copper_list(copperList, y ++ );

				if (checkMouse(win, 1)) quit = true;

			} while( ! quit );

		}

		if (win)
			CloseWindow(win); 
			win = NULL;

		if (copperBitmap)
			FreeBitMap( copperBitmap ); 
			copperBitmap = NULL;
	
	}

	close_libs();

	return 0;
}


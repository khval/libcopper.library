// OSCopper.e
// Native graphics example using OS friendly copperlist

#include <stdbool.h>

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

#ifdef __amigaos4__
struct Custom _custom;
struct Custom *custom = &_custom;	// store locally... handle things with do_functions();
#else
struct Custom *custom = 0xDFF000;
#endif

struct Window *win;
struct BitMap *copperBitmap = NULL;

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

	copperBitmap =AllocBitMap( win -> Width, win -> Height, 32, BMF_DISPLAYABLE, win ->RPort -> BitMap);

	if (!copperBitmap) return false;

	return true;
}

void errors()
{
	if (!win) Printf("Unable to open window.\n");
}

const char *txt = "Cooper test";

void init_copper(int linestart, int height)
{
	int y;
	uint32 *ptr = copperList;
	uint32 backrgb = 0x000;

	// 5 instructions per line, and 3 extra at the end.
	
	for (y=linestart;y<linestart+height;y++)
	{
		setCop( (y<<8) | 1 , 0xFF00 );			// 0
		setCop(BPLCON3,0);
		setCop(COLOR01,(y-linestart) & 0xFFF);
		setCop(BPLCON3,0x200);
		setCop(COLOR03,(0xFFF-y) & 0xFFF);
	}
		
	setCop( (y<<8) | 1 ,0);
	setCop(COLOR01,backrgb);
	setCop(0xFFFF,0xFFFE);
}

void closeDown()
{
	if (win) CloseWindow(win);
	if (copperBitmap) FreeBitMap(copperBitmap);
}

int main_prog()
{
	if (initScreen())
	{
		ULONG backrgb;
		int i;
		int linestart = 0;
		int width = win -> Width;
		int height = win -> Height;
		
		struct RastPort *rport=win -> RPort;

		if (rport)
		{
			if (rport -> BitMap)
			{
				SetAPen(rport,1);
				RectFill(rport,width/2,linestart,width-1,win -> Height-1);

				SetAPen(rport,1);
				RectFill(rport,0,linestart,width/2,win -> Height-1);

				SetAPen(rport,3);
				SetBPen(rport,2);
			}
		}
		
		init_copper( linestart,  height );

		render_copper( custom, copperList, copperBitmap );
   		BltBitMapRastPort(  copperBitmap, 0,0, win -> RPort, 0,0, win -> Width, win -> Height, 0xC0 );

		WaitLeftMouse(win);
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

	return 0;
}




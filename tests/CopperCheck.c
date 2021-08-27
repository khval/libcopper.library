// CopperCheck
// Native graphics example using OS friendly copperlist
// Displays checker pattern then scrolls each line indivdiually

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

struct RastPort *rport;

int i;
ULONG linestart,lines,backrgb;
ULONG x,y;

struct Window *win;

bool initScreen()
{
	win=OpenWindowTags(NULL,
				WA_IDCMP,IDCMP_MOUSEBUTTONS,
				WA_Flags,WFLG_NOCAREREFRESH |
					WFLG_ACTIVATE |
					WFLG_RMBTRAP,
				WA_Left,0,
				WA_Top,0,
				WA_Width,640+64,
				WA_Height,480,
				TAG_END);

	if (!win) return false;

	return true;
}


void errors()
{
	if (!win) Printf("Unable to open window.\n");
}

void init_copper(int linestart, int height)
{
	uint32 *ptr = copperList;

	for (i=linestart;i<linestart+height;i++)
	{
		setCop(i << 8  | 1,0);
  		setCop(BPLCON3,0);
  		setCop(COLOR01,(i-linestart) & 0xFFF);
  		setCop(BPLCON3,0x200);
  		setCop(COLOR01,(0xFFF-i) & 0xFFF);
	}
	
	setCop(i << 8  | 1,0);
	setCop(COLOR01,backrgb);
	setCop(0xFFFF,0xFFFE);
}

void closeDown()
{
	if (win) CloseWindow(win);
}

void draw_bitamp()
{
	rport = win -> RPort;
	SetAPen(rport,1);
	for (y=0;y<256;y+=64) for (x=0;x<192;x+=64) RectFill(rport,x,y,x+31,y+31);
}

int main_prog()
{
	if (initScreen())
	{
		init_copper(0, win -> Height);

		draw_bitamp();
	
		dump_copper( copperList );
		render_copper( copperList , win -> RPort );

		WaitLeftMouse(win);
//		getchar();
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


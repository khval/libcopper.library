// CopperCheck
// Native graphics example using OS friendly copperlist
// Displays checker pattern then scrolls each line indivdiually

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
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

#ifdef __amigaos4__
struct Custom _custom;
struct Custom *custom = &_custom;	// store locally... handle things with do_functions();
#else
struct Custom *custom = 0xDFF000;
#endif

struct RastPort *rport;
struct BitMap *copperBitmap = NULL;

int i;
ULONG linestart,lines,backrgb;
ULONG x,y;


struct TagItem tags_shared[] = {
		{AVT_Type, MEMF_SHARED },
		{TAG_END,TAG_END}};

struct Window *win;

struct BitMap *gfx_bm = NULL;


void _FreeBitMap(struct BitMap *bm)
{
	if (bm->Planes[0]) FreeVec(bm->Planes[0]);
	bm->Planes[0] = NULL;
	FreeVec(bm);
}

struct BitMap *_AllocBitMap( int w, int h )
{
	struct BitMap *bm = NULL;
	uint8 *bitplane;
	int size  = w/8 * h;
	bm = (struct BitMap *) AllocVecTagList( sizeof(struct BitMap), tags_shared);

	if (bm)
	{
		bitplane = (uint8 *) AllocVecTagList( size, tags_shared);
		if (bitplane)
		{
			bzero( bitplane, size);
			InitBitMap( bm, 1, w, h );
			bm -> Planes[0] = (void *) bitplane;
			bm -> BytesPerRow = w/8;
			bm -> Rows = h;
		}
		else
		{
			_FreeBitMap(bm);
			bm = NULL;
		}
	}

	return bm;
}



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

	copperBitmap =AllocBitMap( win -> Width, win -> Height, 32, BMF_DISPLAYABLE, win ->RPort -> BitMap);
	if (!copperBitmap)  return false;

	return true;
}


void errors()
{
	if (!win) Printf("Unable to open window.\n");
}

void init_copper(int linestart, int height)
{
	uint32 *ptr = copperList;

	setCop( BPL1PTH, (uint32) gfx_bm -> Planes[0] >> 16 );	
	setCop( BPL1PTL, (uint32) gfx_bm -> Planes[0] & 0xFFFF );	
	setCop( BPL1MOD, 0x0 );
	setCop( BPL2MOD, 0x0 );
	setCop( COLOR00,0x0FFF );
	setCop( COLOR01,0x0F00 );
	setCop( BPLCON0,0x1200 );	
	setCop( DIWSTART,0x2081);
	setCop( DIWSTOP, ((199 + 0x20 ) << 8) | 0xC1);

	setCop( DDFSTART,0x0038 );
	setCop( DDFSTOP,0x00D0 );

	for (i=linestart;i<linestart+height;i++)
	{
		setCop(i << 8  | 1,0xFFFE);
  		setCop(COLOR00,(i-linestart) & 0xFFF);
  		setCop(COLOR01,(0xFFF-i) & 0xFFF);
		setCop(i << 8  | 11 << 1 | 1,0xFFFE);
  		setCop(COLOR01,(i+21) & 0xFFF);
		setCop(i << 8  | 15 << 1 | 1,0xFFFE);
  		setCop(COLOR01, i & 0xFF << 4 | 0x00F);
	}
	
	setCop(i << 8  | 1,0);
	setCop(COLOR00,backrgb);

	setCop(0xFFFF,0xFFFE);
}

void closeDown()
{
	if (win) CloseWindow(win);
	if (copperBitmap) FreeBitMap( copperBitmap ); 

	win = NULL;
	copperBitmap = NULL;
}

void draw_bitmap( struct RastPort *rp )
{
	SetAPen(rp,1);
	for (y=0;y<256;y+=64) for (x=0;x<192;x+=64) RectFill(rp,x,y,x+31,y+31);
}

int main_prog()
{
	DebugPrintF("%s\n",__FUNCTION__);

	if (initScreen())
	{
		gfx_bm = _AllocBitMap( 320, 200 );

		if (gfx_bm)
		{
			struct RastPort rp;
			InitRastPort(&rp); 
			rp.BitMap = gfx_bm;

			init_copper(0, win -> Height);
			draw_bitmap( &rp );
	
//				dump_copper( copperList );

			render_copper( custom, copperList , copperBitmap );
   	 		BltBitMapRastPort(  copperBitmap, 0,0, win -> RPort, 0,0, win -> Width, win -> Height, 0xC0 );

			WaitLeftMouse(win);

			_FreeBitMap(gfx_bm);
			gfx_bm = NULL;
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
	DebugPrintF("%s\n",__FUNCTION__);

	if (open_libs()==FALSE)
	{
		Printf("failed to open libs!\n");
		close_libs();
		return 0;
	}

	init_ecs2colors();
	ret = main_prog();

	close_libs();

	return 0;
}


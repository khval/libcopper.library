// OSCopper.e
// Native graphics example using OS friendly copperlist

#include <stdlib.h>
#include <stdio.h>		
#include <stdbool.h>
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


#define CMOVEA(c,a,b) { CMove(c,a,b);CBump(c); }

#define BPLCON0 0x100
#define BPLCON1 0x102
#define BPLCON2 0x104
#define BPLCON3 0x106
#define BPL1MOD 0x108
#define BPL2MOD 0x10A
#define BPLPT 0x0E0

#define SetColour(src,a,r,g,b) SetRGB32( &(src -> ViewPort), (ULONG) a*0x01010101, (ULONG) r*0x01010101,(ULONG) g*0x01010101,(ULONG) b*0x01010101 )

#define Shr(x,n) (x << n)

struct Window *win = NULL;

struct BitMap *gfx_bm = NULL;

bool initScreen()
{
	win = OpenWindowTags(NULL,
			WA_IDCMP,IDCMP_MOUSEBUTTONS,
			WA_Flags,WFLG_NOCAREREFRESH |
					WFLG_ACTIVATE,
			WA_Left,640,
			WA_Top,20 ,
			WA_Width,640+128,
			WA_Height,480,

			TAG_END);

	if (!win) return false;

	return true;
}

void closeDown()
{
	if (win) CloseWindow(win);
}


void errors()
{
	if (!win) Printf("Unable to open window.\n");
}

struct BitMap bitmap;
uint32 planesize;
uint32 modulo;
uint8 *bitplane;



void init_copper(int bm_width, int bm_height, int linestart, int height)
{
	uint32 backrgb = 0x000;
	int i;
	int depth = 1;
	uint32 *ptr = copperList;

	setCop(COLOR01,0xFFF);	// +1

	setCop(DIWSTART,0x2C81);
	setCop(DIWSTOP,0x7EC1);
	setCop(DDFSTART,0x0038);
	setCop(DDFSTOP, WordCountToDispDataFetchStop( 0, 0x0038 , (bm_width/16)) );	

	setCop( BPL1PTH, (uint32) bitplane >> 16 );
	setCop( BPL1PTL, (uint32) bitplane & 0xFFFF );

	setCop( BPL1MOD, 0x0 );
	setCop( BPL2MOD, 0x0 );

	setCop( BPLCON0, depth << 12  |  0x0000 );

	for (i=linestart;i<linestart+height;i++)
	{
		setCop(i<<8|1,0xFF00);		// +1

		switch (i)
		{
			case 127:
				setCop(BPL1MOD,-1*(planesize/2));		// +3
				setCop(BPLPT, (uint32) bitplane>>16);
				setCop(BPLPT+2,(uint32)bitplane & 0xFFFF);
				break;

			case 128:
				setCop(BPL1MOD,modulo);
				break;
		}

		setCop(BPLCON3,0);					// +4
		setCop(COLOR01,(i-linestart)&0xFFF);
		setCop(BPLCON3,0x200);
		setCop(COLOR01,(0xFFF-i)&0xFFF);
	}

	// +3

	setCop(i<<8|1,0xFF00);
	setCop(COLOR01,backrgb);
	setCop(0xFFFF,0xFFFE);
}

struct RastPort gfx_rp;

void init_planes()
{
	int x,y;
	int size;
	struct TagItem tags_shared[] = {
		{AVT_Type, MEMF_SHARED },
		{TAG_END,TAG_END}};

	size = 320/8 * 200;

	bitplane = (uint8 *) AllocVecTagList( size, tags_shared);

	bzero(bitplane, size);

	gfx_bm = (struct BitMap *) AllocVecTagList( sizeof(struct BitMap), tags_shared);

	InitBitMap( gfx_bm, 1, 320, 200 );
	gfx_bm->Planes[0] = (void *) bitplane;
	gfx_bm -> BytesPerRow = 320/8;
	gfx_bm -> Rows = 200;

	InitRastPort( &gfx_rp );
	gfx_rp.BitMap = gfx_bm;

	SetAPen(&gfx_rp,2);
	RectFill(&gfx_rp,0,0, 320,200);

	SetAPen(&gfx_rp,1);
	Move(&gfx_rp,0,0);
	Draw(&gfx_rp,320,200);

	Move(&gfx_rp,0,0);
	Draw(&gfx_rp,0,200);

	Move(&gfx_rp,320/2,0);
	Draw(&gfx_rp,320/2,200);

	Move(&gfx_rp,320-1,0);
	Draw(&gfx_rp,320-1,200);


}

void old_code()
{
	int x,y;

		struct RastPort rport;

		InitRastPort( &rport );
		bzero( bitplane,sizeof(bitplane) );
		bitmap.BytesPerRow = 320/8;
		bitmap.Rows = 200;
		bitmap.Planes[0] = bitplane;
		bitmap.Depth = 1;
		rport.BitMap = &bitmap;
		modulo=bitmap.BytesPerRow;
		planesize=modulo* bitmap.Rows;

		SetAPen(&rport,1);
//		Box(&rport,0,linestart,320-1,200-1,1);

		for (y=0;y<64;y+=64)
		{
			for (x=0;x<256;x+=64)
			{
				RectFill(&rport,x,y,x+31,y+31);
//				Box(&rport,x,y,32,32,1);
			}
			
			for (x=32;x<288;x+=64)
			{
				RectFill(&rport,x,y+32,x+31,y+63);
//				Box(&rport,x,y+32,32,32,1);
			}
		}
}

int main_prog()
{

	if (initScreen())
	{
		int i;
		int x,y;
		uint32 backrgb;
		int linestart=0;
		int lines=win -> Height;
		int width=win -> Width;
	
		init_ecs2colors();

		init_planes();
		init_copper( 320,200, 0,win->Height);
	
		render_copper( custom, copperList, win -> RPort );

		int wc = DispDataFetchWordCount( 0, ddfstart, ddfstop);

		printf("data fetch start %d (pixels %d)\n",ddfstart,DispDataFetchWordCount( 0, 0,ddfstart)*16);
		printf("data fetch word count %d (pixels %d)\n",wc,wc*16);

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


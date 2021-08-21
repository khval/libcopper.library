
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

// se page 28 in the hardware ref manual

#include "render.h"

uint32 plane1 = 0;
uint32 plane2 = 0;

void init_copper_list()
{
	uint32 *ptr = (uint32 *) copperList;

	setCop( BPL1PTH, plane1 >> 16 );	
	setCop( BPL1PTL, plane1 & 0xFFFF );	
	setCop( BPL2PTH, plane2 >> 16 );	
	setCop( BPL2PTL, plane2 & 0xFFFF );	

	setCop( BPL1MOD, 0x0 );
	setCop( BPL2MOD, 0x0 );

	setCop( COLOR00,0x0FFF );	
	setCop( COLOR01,0x0F00 );	
	setCop( COLOR02,0x00F0 );	
	setCop( COLOR03,0x000F );	

	setCop( BPLCON0,0x2200 );	

	setCop( 0x9601,0xFF00 );			// wait for line 150, ignore horiz, position

	setCop( COLOR00,0x0000 );	
	setCop( COLOR01,0x0FF0 );	
	setCop( COLOR02,0x00FF );	
	setCop( COLOR03,0x0F0F );	

	setCop( 0xFFFF,0xFFFE );			// wait for line 255, H = 254 (never happens)

}

struct RastPort gfx_rp;
struct BitMap *gfx_bm;

void _plot(char *ptr,int bpr,int x,int y)
{
	int bx = x / 8;
	ptr[bpr*y+bx] |= 0x80 >> (x & 7);
}

void _box( char *ptr, int bpr, int x0, int y0, int x1, int y1)
{
	int x,y;

	for (y=y0; y<=y1;y++)
	{
		for (x=x0; x<=x1;x++)
		{
			_plot(ptr, bpr, x, y);
		}
	}
}

void init_planes()
{
	int x,y;
	int size;
	struct TagItem tags_shared[] = {
		{AVT_Type, MEMF_SHARED },
		{TAG_END,TAG_END}};

	size = 320/8 * 200;

	plane1 = (uint32) AllocVecTagList( size, tags_shared);
	plane2 = (uint32) AllocVecTagList( size, tags_shared);

	bzero(plane1, size);
	bzero(plane2, size);

	gfx_bm = (struct BitMap *) AllocVecTagList( sizeof(struct BitMap), tags_shared);

	InitBitMap( gfx_bm, 2, 320, 200 );
	gfx_bm->Planes[0] = (void *) plane1;
	gfx_bm->Planes[1] = (void *) plane2;

	gfx_bm -> BytesPerRow = 320/8;
	gfx_bm -> Rows = 200;

	InitRastPort( &gfx_rp );
	gfx_rp.BitMap = gfx_bm;

	SetAPen(&gfx_rp,2);
	RectFill(&gfx_rp,0,0, 320,200);

	SetAPen(&gfx_rp,1);
	Move(&gfx_rp,0,0);
	Draw(&gfx_rp,320,200);

	_box(plane1, 320/8, 20,20,50,50);

}


int main()
{
	printf("sizeof(union cop) == %d\n", sizeof(union cop));

	if (open_libs())
	{
		init_planes();
		init_ecs2colors();

		struct Window *win;

		if ((plane1) && (plane2))
		{
			win = OpenWindowTags( NULL, 
				WA_Left,320,
				WA_Top,20,
				WA_Width, 640,
				WA_Height, 520,
				TAG_END);
		}

		if (win)
		{
			struct BitMap bm;

			rp = win -> RPort;

			cop_move_(DIWSTART,0x2081);
			cop_move_(DIWSTOP,0xF4C1);

			cop_move_(DDFSTART,0x0038);
			cop_move_(DDFSTOP,0x00D0);

			int wc = WordCount( ddfstart, ddfstop);

			init_copper_list();
			render_copper();

			printf("data fetch start %d (pixels %d)\n",ddfstart,WordCount( 0,ddfstart)*16);
			printf("data fetch word count %d (pixels %d)\n",wc,wc*16);

			int diwstarty = diwstart >> 8 ;
			int diwstopy = diwstop >> 8 ;

			printf("rows %d\n", (diwstopy & 0x80 ? diwstopy : diwstopy + 0x100) - diwstarty );

			getchar();

			CloseWindow(win);
		}


		if (plane1) FreeVec( (void*) plane1);
		if (plane2) FreeVec( (void*) plane2);
		
	}

	close_libs();

	return 0;
}


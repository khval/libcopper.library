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

//struct RastPort *rport;

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

bool initScreen()
{
	screen=OpenScreenTags(NULL,
			SA_Title,"OS Copper",
//	SA_Pens,,
			SA_Depth,4,
			SA_Width, 320,
			SA_Height, 256,
			TAG_END);

	if (!screen) return false;

	window=OpenWindowTags(NULL,
			WA_IDCMP,IDCMP_MOUSEBUTTONS,
			WA_Flags,WFLG_NOCAREREFRESH |
					WFLG_ACTIVATE |
					WFLG_BORDERLESS |
					WFLG_BACKDROP,
			WA_CustomScreen,screen,
			TAG_END);

	if (!window) return false;

#ifdef __amigaos3__
	myucoplist=AllocVec(sizeof(struct UCopList),MEMF_PUBLIC | MEMF_CLEAR);
#endif

#ifdef __amigaos4__
	myucoplist=AllocVecTags(sizeof(struct UCopList),
				AVT_Type, MEMF_SHARED, 
				AVT_Alignment,  16, 
				AVT_ClearWithValue, 0,
				TAG_DONE);
#endif

	if (!myucoplist) return false;	

	return true;
}

void errors()
{
	if (!screen) Printf("Unable to open screen.\n");
	if (!window) Printf("Unable to open window.\n");
	if (!myucoplist) Printf("Unable to allocate myucoplist memory.\n");
}

int main_prog()
{

	if (initScreen())
	{
		int i;
		int x,y;
		uint32 backrgb;
		int linestart=screen -> BarHeight+1;
		int lines=screen -> Height-linestart;
		int width=screen -> Width;
	
		struct RastPort *rport=window -> RPort;
		struct BitMap *bitmap=screen -> RastPort.BitMap;
		uint32 modulo=bitmap -> BytesPerRow-40;
		uint32 planesize=modulo*screen -> Height;
		ULONG bitplane=(ULONG) bitmap -> Planes[0];
	
		viewport=ViewPortAddress(window);
		backrgb= ((ULONG *) viewport -> ColorMap -> ColorTable)[0];

		SetColour(screen,0,0,0,0);
		SetColour(screen,1,255,255,255);
		SetRast(rport,1);
		Box(rport,0,linestart,width-1,screen -> Height-1,1);
	
		for (y=0;y<64;y+=64)
		{
			for (x=0;x<256;x+=64)
			{
				RectFill(rport,x,y,x+31,y+31);
				Box(rport,x,y,32,32,1);
			}
			
			for (x=32;x<288;x+=64)
			{
				RectFill(rport,x,y+32,x+31,y+63);
				Box(rport,x,y+32,32,32,1);
			}
		}
		
		CINIT(myucoplist, 1+ (lines*(1+3+4)) + 3 );


		CMOVEA(myucoplist,COLOR(1),0xFFF);	// +1

		for (i=linestart;i<lines;i++)
		{
			CWAIT(myucoplist,i,0);		// +1

			switch (i)
			{
				case 127:
					CMOVEA(myucoplist,BPL1MOD,-1*(planesize/2));		// +3
					CMOVEA(myucoplist,BPLPT,Shr(bitplane,16));
					CMOVEA(myucoplist,BPLPT+2,bitplane & 0xFFFF);
					break;

				case 128:
					CMOVEA(myucoplist,BPL1MOD,modulo);
					break;
			}

			CMOVEA(myucoplist,BPLCON3,0);					// +4
			CMOVEA(myucoplist,COLOR(1),(i-linestart)&0xFFF);
			CMOVEA(myucoplist,BPLCON3,0x200);
			CMOVEA(myucoplist,COLOR(1),(0xFFF-i)&0xFFF);
		}

		// +3

		CWAIT(myucoplist,i,0);
		CMOVEA(myucoplist,COLOR(1),backrgb);
		CEND(myucoplist);
	
		Forbid();
		viewport -> UCopIns = myucoplist;
		Permit();
		RethinkDisplay();
	
		WaitLeftMouse(window);
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


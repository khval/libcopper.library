
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#include "render.h"

void init_copper_list()
{
	uint32 *ptr = (uint32 *) copperList;

copperl1 = (uint32) ptr;

	setCop( COLOR00,0x0F00 );		// move
	setCop( 0x0F01,0x8F00 );			// wait
	setCop( INTREQ,0x8010 );		// move
	setCop( 0x00E3,0x80FE );		// wait
	setCop( 0x7F01,0x7F01 );			// skip
	setCop( COPJMP1,0x0000 );	

copperl2 = (uint32) ptr;

	setCop( COLOR00,0x0F0 );		// move
	setCop( 0x0F01,0x8F00 );			// wait
	setCop( INTREQ,0x8010 );		// move
	setCop( 0x00E3,0x80FE );		// wait
	setCop( 0xFF01,0xFE01 );			// skip
	setCop( COPJMP2,0x0000 );	
	setCop( 0xFFFF,0xFFFE );	

	cop_move_(COP1LCH,copperl1 >> 16);
	cop_move_(COP1LCL,copperl1 & 0xFFFF);

	cop_move_(COP2LCH,copperl2 >> 16);
	cop_move_(COP2LCL,copperl2 & 0xFFFF);

	printf("COP1LC: %d\n", COP1LC);
	printf("COP2LC: %d\n", COP2LC);
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
			copper_rp = win -> RPort;

			cop_move_(DIWSTART,0x2C81);
			cop_move_(DIWSTOP,0xF4C1);

			cop_move_(DDFSTART,0x0038);
			cop_move_(DDFSTOP,0x00D0);

			int wc = DispDataFetchWordCount( 0, ddfstart, ddfstop);

			init_copper_list();
			render_copper( copperList, copper_rp );

			printf("data fetch start %d (pixels %d)\n",ddfstart,DispDataFetchWordCount( 0, 0,ddfstart)*16);
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


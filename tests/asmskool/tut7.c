

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/libblitter.h>
#include <hardware/custom.h>

#include "common.h"
#include "render.h"

#ifdef __amigaos4__
struct Custom _custom;
struct Custom *custom = &_custom;	// store locally... handle things with do_functions();
#else
struct Custom *custom = 0xDFF000;
#endif

extern union reg_u *emu_stack_ptr;
union reg_u emu_stack[10000];

uint16 *Copper = NULL;
uint16 *waitras1 = NULL;
uint16 *waitras2 = NULL;
uint16 *waitras3 = NULL;
uint16 *waitras4 = NULL;
uint16 *waitras5 = NULL;
uint16 *waitras6 = NULL;

#define ld_b(a) *((uint8 *) (a))
#define ld_w(a) *((uint16 *) (a))
#define ld_l(a) *((uint32 *) (a))

#define st_b(a,v) *((uint8 *) (a)) = (uint8) (v)
#define st_w(a,v) *((uint16 *) (a)) = (uint16) (v)
#define st_l(a,v) *((uint32 *) (a)) = (uint32) (v)

// --- setup program, cleanup, etc...

struct Window *win = NULL;
struct BitMap *copperBitmap = NULL;

bool mouse_pressed();

									//		ORG $20000
									//		LOAD $20000
									//		JUMPPTR init
									//	
void init() {							//	init:
									//		move.l 4.w,a6		;execbase
									//		clr.l d0
									//		move.l #gfxname,a1
									//		jsr -408(a6)		;oldopenlibrary()
									//		move.l d0,a1
									//		move.l 38(a1),d4	;original copper ptr
									//	
									//		jsr -414(a6)		;closelibrary()
									//	
	d7 = 0xac;						//		move.w #$ac,d7		;start y position
	d6 = 1;							//		moveq #1,d6		;y add
									//		move.w $dff01c,d5
	// don't need this ------->				//		move.w #$7fff,$dff09a	;disable all bits in INTENA
									//	
	// don't need this ------->				//		move.l #copper,$dff080


									//	**************************
	do								//	mainloop:
	{
								
		WaitTOF();					//	wframe:
									//		btst #0,$dff005
									//		bne.b wframe
									//		cmp.b #$2a,$dff006
									//		bne.b wframe
									//	wframe2:
									//		cmp.b #$2a,$dff006
									//		beq.b wframe2
									//	
									//	;-----frame loop start---

		d7+=d6;						//		add d6,d7		;add "1" to y position

		if (d7>=0xF0)					//		cmp #$f0,d7		;bottom check
		{							//		blo.b ok1
			d6 = -d6;					//		neg d6			;change direction
		}							//	ok1:

		if (d7<=0x40)					//		cmp.b #$40,d7
		{							//		bhi.b ok2
			d6 = -d6;					//		neg d6			;change direction
		}							//	ok2:


		printf("d6: %d, d7: %d\n",d6, d7);


		a0 = waitras1;					//		move.l #waitras1,a0
		d0 = d7;						//		move d7,d0
		for(d1=6;d1;d1--)				//		moveq #6-1,d1
		{							//	.l:
			st_b(a0,d0);				//		move.b d0,(a0)
			d0+=1;					//		add.w #1,d0
			a0+=8;					//		add.w #8,a0
		}							//		DBF d1,.l

									//	;-----frame loop end---

	}								//		btst #6,$bfe001
	while ( !mouse_pressed());			//		bne.b mainloop
									//	**************************
									//	exit:
									//		move.l d4,$dff080
									//		or #$c000,d5
									//		move d5,$dff09a
}									//		rts

									//	gfxname:
									//		dc.b "graphics.library",0

									//		EVEN

#define cop_w( a,b ) *cop_ptr++=(a); *cop_ptr++=(b); 

void init_copper()
{
	uint16 *cop_ptr;
	Copper = malloc(  700 * 2 );			//	Copper:
	cop_ptr = Copper;


	cop_w (0x1fc,0);					//		dc.w $1fc,0			;slow fetch mode, AGA compatibility
	cop_w (0x100,0x0200);				//		dc.w $100,$0200
	cop_w (0x180,0x0349);				//		dc.w $180,$349

	cop_w (0x2b07,0xfffe);				//		dc.w $2b07,$fffe
	cop_w (0x180,0x56c);				//		dc.w $180,$56c
	cop_w (0x2c07,0xfffe);				//		dc.w $2c07,$fffe
	cop_w (0x180,0x113);				//		dc.w $180,$113
									//	
waitras1 = cop_ptr;						//	waitras1:
	cop_w (0x8007,0xfffe);				//		dc.w $8007,$fffe
	cop_w (0x180,0x055);				//		dc.w $180,$055

waitras2 = cop_ptr;						//	waitras2:
	cop_w (0x8107,0xfffe);				//		dc.w $8107,$fffe
	cop_w (0x180,0x0aa);				//		dc.w $180,$0aa

waitras3 = cop_ptr;						//	waitras3:
	cop_w (0x8207,0xfffe);				//		dc.w $8207,$fffe
	cop_w (0x180,0x0ff);					//		dc.w $180,$0ff

waitras4 = cop_ptr;						//	waitras4:
	cop_w (0x8307,0xfffe);				//		dc.w $8307,$fffe
	cop_w (0x180,0x0aa);				//		dc.w $180,$0aa

waitras5 = cop_ptr;						//	waitras5:
	cop_w (0x8407,0xfffe);				//		dc.w $8407,$fffe
	cop_w (0x180,0x055);				//		dc.w $180,$055

waitras6 = cop_ptr;						//	waitras6:
	cop_w (0x8507,0xfffe);				//		dc.w $8507,$fffe
	cop_w (0x180,0x113);				//		dc.w $180,$113
									//	
	cop_w (0xffdf,0xfffe);					//		dc.w $ffdf,$fffe
	cop_w (0x2c07,0xfffe);				//		dc.w $2c07,$fffe
	cop_w (0x180,0x56c);				//		dc.w $180,$56c
	cop_w (0x2d07,0xfffe);				//		dc.w $2d07,$fffe
	cop_w (0x180,0x349);				//		dc.w $180,$349
									//	
	cop_w (0xffff,0xfffe);					//		dc.w $ffff,$fffe

}	


void	cleanup()
{
	if (win) CloseWindow(win); 
	if (copperBitmap) FreeBitMap( copperBitmap ); 

	win = NULL;
	copperBitmap = NULL;

	close_libs();
}

bool mouse_pressed()
{
	bool pressed = false;
	ULONG sig;
	if (win -> UserPort)
	{
		ULONG win_mask = 1 << win -> UserPort ->mp_SigBit ;

		render_copper( custom, (uint32 *) Copper,  copperBitmap );
		BltBitMapRastPort(  copperBitmap, 0,0, win -> RPort, 0,0, win -> Width, win -> Height, 0xC0 );

		sig = SetSignal( 0L, win_mask | SIGBREAKF_CTRL_C );
		if (sig & win_mask) if (checkMouse(win, 1)) pressed = true;
	}

	return pressed;
}

int main()
{
	if (open_libs() == false)
	{
		close_libs();
	}

	win = OpenWindowTags( NULL, 
		WA_IDCMP,IDCMP_MOUSEBUTTONS,
		WA_Left,320,
		WA_Top,20,
		WA_Width, 640 + 128,
		WA_Height, 480 + 128,
		TAG_END);

	if (!win)
	{
		cleanup();
		return 0;
	}

	ActivateWindow(win);

	copperBitmap =AllocBitMap( win -> Width, win -> Height, 32, BMF_DISPLAYABLE, win ->RPort -> BitMap);

	if (copperBitmap) 
	{
		struct RastPort rp;
		InitRastPort(&rp);
		rp.BitMap = copperBitmap;
		RectFillColor(&rp, 0, 0, win -> Width, win -> Height, 0xFF666666);
	}
	else
	{
		cleanup();
		return 0;
	}

	// setup fake stack pointer.. :-)
	emu_stack_ptr = emu_stack;

	init_ecs2colors();		// don't forget to make the lookup table.

	init_copper();
	init();

	cleanup();

	return 0;
}


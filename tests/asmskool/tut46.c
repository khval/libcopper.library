#include <proto/exec.h>
#include <proto/dos.h>

/*
	SECTION TutDemo,CODE
	JUMPPTR Start

	INCDIR ""
	INCLUDE "PhotonsMiniStartup/PhotonsMiniWrapper1.04!.S"
	INCLUDE "Blitter-Register-List.S"
	INCLUDE "P61-Options.S"
*/
//    ---  screen buffer dimensions  ---

uint32 d0,d1,d2,d3,d4,d5,d6,d7;
uint32 a0,a1,a2,a3,a4,a5,a6,a7;


struct cloud
{
	void		*addr;
	uint32	mask;
	uint16	x,y,width,height,xspeed;
};

uint8 *Copper;
uint8 *Module1;

uint32 w	=352;
uint32 h	=256;
#define bplsize (w*h/8)
#define ScrBpl	(w/8)

//    ---  logo dimensions  ---

uint32 logow		=320;
uint32 logoh		=99;

#define logomargin	((320-logow)/2)
#define logobpl	(logow/8)
#define logobwid	(logobpl*3)

#define skybpl		(320/8+14)
#define skybwid	(skybpl*3)
#define skyh		220

//    ---  font dimensions  ---
#define fontw		288
#define fonth		100
#define fontbpls	3
#define FontBpl	(fontw/8)

#define plotY	110
#define plotX	(w-32)

#define logobgcol	0x44f
#define bgcol		0x225

#define cloudcount	10
#define cloudstructsize	18

void Main();

//********************  MACROS  ********************

#define logocolors						\
	dc.w 0x068e,0x0adf,0x0dff				\
	dc.w 0x09bf,0x056d,0x044b,0x033a		\


#define cloudcolors						\
	dc.w 0x066f,0x077f,0x088e				\
	dc.w 0x0aae,0x0bbe,0x0dde,0x0eee		\

/*
WAITBLIT:macro
	tst DMACONR(a6)			;for compatibility
	btst #6,DMACONR(a6)
	bne.s *-6
	endm
*/

//********************  DEMO  ********************


void Demo()
{
	Init();

	cop_move_(0x096, 0x87e0);			//	move.w #$87e0,0xdff096

	cop_move_(0x080, Copper);			//	move.l #Copper,0xdff080
									//	move.l #VBint,0x6c(a4)		;set vertb interrupt vector compatibly.
									//	move.w #0xc020,0xdff09a	;enable interrupts generally
									//							;and vertb specifically.

//    ---  Call P61_Init  ---
									//	movem.l d0-a6,-(sp)

	a0 = (uint32) Module1;		 //	lea Module1,a0
	a1 = 0;			// sub.l a1,a1
	a2 = 0;			// sub.l a2,a2
	d0 = 0;			// moveq #0,d0

	P61_Init();
					//	movem.l (sp)+,d0-a6
	Main();
					//	movem.l d0-a6,-(sp)
	P61_End();

					//	movem.l (sp)+,d0-a6

					//	rts			;go back to system friendly wrapper exit.
	return;
}

//********** ROUTINES **********
void Main()
{
					//	movem.l d0-a6,-(sp)

//**************************

	WaitMouse();

					// WaitMouse:
					//	btst #6,0xbfe001
					//	bne.s WaitMouse

//**************************

					//	movem.l (sp)+,d0-a6
//	rts
}

//********************  Effect 1 subroutine  ********************

#define ld_b(a) *((uint8 *) (a))
#define ld_w(a) *((uint16 *) (a))
#define ld_l(a) *((uint32 *) (a))

#define st_b(a,v) *((uint8 *) (a)) = (uint8) (v)
#define st_w(a,v) *((uint16 *) (a)) = (uint16) (v)
#define st_l(a,v) *((uint32 *) (a)) = (uint32) (v)

uint8 *SkyBufferL;
uint8 *CloudCoordsLP;
uint8 *StarSpr;
uint8 *StarSpr2;

#define bin8(b7,b6,b5,b4,b3,b2,b1,b0) ((b7<<7) | (b6 <<6) | (b5 <<5) | (b4<<4) | (b3 <<3) | (b2<<2) | (b1<<1) | (b0))

void Part1()
{
								//	movem.l d0-a6,-(sp)

 //   *--- clear clouds ---*

	a1= (uint32) SkyBufferL;				// move.l SkyBufferL(PC),a1
	a3 = (uint32) CloudCoordsLP; 			// move.l CloudCoordsLP(PC),a3

	d7 =	cloudcount-1;				// moveq #cloudcount-1,d7

// .clearl:

	for (;d7;d7--)
	{
		a0 = ld_l(a3); a3+=4;		//	move.l (a3)+,a0			;bob
		a2 = ld_l(a3); a3+=4;		//	move.l (a3)+,a2			;mask

		d0 = ld_l(a3); a3+=4;		//	move.w (a3)+,d0			;x coord
		d1 = ld_l(a3); a3+=4;		//	move.w (a3)+,d1			;y coord
		d2 = ld_l(a3); a3+=4;		//	move.w (a3)+,d2			;width
		d3 = ld_l(a3); a3+=4;		//	move.w (a3)+,d3			;height
		
		a3+=2;					//	addq.w #2,a3			;skip speed value.
		d4 = 0x01000000;			//	move.l #0x01000000,d4		;clear blit

		PlotBob();
	}							//	dbf d7,.clearl

// @bouncescroller was here

	a1 =(uint32) StarSpr+1;		//	lea StarSpr+1,a1
	d0 = 0x2C;			//	moveq #0x2c,d0
	d7= 26-1;				//	moveq #26-1,d7

//.skyl:	

	for (;d7;d7--)
	{
		st_b(a1, ld_b(a1)+1);	//	addq.b #1,(a1)			;add speed to hpos
		a1+=8;				//	addq.w #8,a1			;skip to next sprite control words
		st_b(a1, ld_b(a1)+2);	//	addq.b #2,(a1)			;add speed to hpos
		a1+=8;				//	addq.w #8,a1			;skip to next sprite control words
		st_b(a1, ld_b(a1)+1);	//	addq.b #1,(a1)			;add speed to hpos
		a1+=8;				//	addq.w #8,a1			;skip to next sprite control words
		st_b(a1, ld_b(a1)+3);	//	addq.b #3,(a1)			;add speed to hpos
		a1+=8;				//	addq.w #8,a1			;skip to next sprite control words

	}	//	dbf d7,.skyl	

	st_b(a1, ld_b(a1)+1);		//	addq.b #1,(a1)			;add speed to hpos
	a1+=8;					//	addq.w #8,a1			;skip to next sprite control words
	st_b(a1, ld_b(a1)+2);		//	addq.b #2,(a1)			;add speed to hpos
	a1+=8;					//	addq.w #8,a1			;skip to next sprite control words

//    *--- below line 0xff ---*

	d6 = bin8(0,0,0,0,0,1,1,0) ;	// moveq #%00000110,d6

	st_b(a1, ld_b(a1)+1);		//	addq.b #1,(a1)			;add speed to hpos
	a1+=8;					//	addq.w #8,a1
	st_b(a1, ld_b(a1)+3);		//	addq.b #3,(a1)			;add speed to hpos
	a1+=8;					//	addq.w #8,a1

//.floorl:
	
	for (d7=5-1;d7;d7--)				//	moveq #5-1,d7
	{
		st_b(a1, ld_b(a1)+1);		//	addq.b #1,(a1)			;add speed to hpos
		a1+=8;					//	addq.w #8,a1
		st_b(a1, ld_b(a1)+2);		//	addq.b #2,(a1)			;add speed to hpos
		a1+=8;					//	addq.w #8,a1
		st_b(a1, ld_b(a1)+1);		//	addq.b #1,(a1)			;add speed to hpos
		a1+=8;					//	addq.w #8,a1
		st_b(a1, ld_b(a1)+3);		//	addq.b #3,(a1)			;add speed to hpos
		a1+=8;					//	addq.w #8,a1
	}							//	dbf d7,.floorl

//    *--- odd lines sprites ---*

	a1 = (uint32) StarSpr2+1;		//	lea StarSpr2+1,a1
	d0 = 0x2d;			//	moveq #0x2d,d0

//.skyl2:	

	for (d7=26-1;d7;d7--)			//	moveq #26-1,d7
	{
		st_b(a1, ld_b(a1)+1);		//	addq.b #1,(a1)			;add speed to hpos
		a1+=8;					//	addq.w #8,a1			;skip to next sprite control words
		st_b(a1, ld_b(a1)+2);		//	addq.b #2,(a1)			;add speed to hpos
		a1+=8;					//	addq.w #8,a1			;skip to next sprite control words
		st_b(a1, ld_b(a1)+1);		//	addq.b #1,(a1)			;add speed to hpos
		a1+=8;					//	addq.w #8,a1			;skip to next sprite control words
		st_b(a1, ld_b(a1)+3);		//	addq.b #3,(a1)			;add speed to hpos
		a1+=8;					//	addq.w #8,a1			;skip to next sprite control words

	}				//	dbf d7,.skyl2

	st_b(a1, ld_b(a1)+1);		//	addq.b #1,(a1)			;add speed to hpos
	a1+=8;					//	addq.w #8,a1			;skip to next sprite control words
	st_b(a1, ld_b(a1)+2);		//	addq.b #2,(a1)			;add speed to hpos
	a1+=8;					//	addq.w #8,a1			;skip to next sprite control words

//    *--- below line 0xff ---*

	d6 = bin8(0,0,0,0,0,1,1,0) ;	//	moveq #%00000110,d6

	st_b(a1, ld_b(a1)+1);		//	addq.b #1,(a1)			;add speed to hpos
	a1+=8;					//	addq.w #8,a1
	st_b(a1, ld_b(a1)+3);		//	addq.b #3,(a1)			;add speed to hpos
	a1+=8;					//	addq.w #8,a1

							//	moveq #5-1,d7
							//.floorl2:	
	
	for(d7=5-1;d7;d7--)
	{
		st_b(a1, ld_b(a1)+1);	//	addq.b #1,(a1)			;add speed to hpos
		a1+=8;				//	addq.w #8,a1
		st_b(a1, ld_b(a1)+2);	//	addq.b #2,(a1)			;add speed to hpos
		a1+=8;				//	addq.w #8,a1
		st_b(a1, ld_b(a1)+1);	//	addq.b #1,(a1)			;add speed to hpos
		a1+=8;				//	addq.w #8,a1
		st_b(a1, ld_b(a1)+3);	//	addq.b #3,(a1)			;add speed to hpos
		a1+=8;				//	addq.w #8,a1
	}						//	dbf d7,.floorl2

//    *--- scroller ---*

							//	move.w Cmd_StopCount(PC),d0
	if ( !Cmd_StopCount)			//	beq.s .normal
		Cmd_StopCount--;		//	subq.w #1,Cmd_StopCount
	else						//	bra.s .skipscroll
	{						//.normal:
		Scrollit();				//	bsr.w Scrollit

		d2=32;				//	moveq #32,d2
		d0= LastChar;			//	move.b LastChar(PC),d0
							//	cmp.b #'I',d0
		if (d0 != (uint32) 'I)		//	bne.s .noi
			d2=16;			//	moveq #16,d2
							//.noi:
		d0 =	ScrollCtr;			//	move.w ScrollCtr(PC),d0
		d0 +=4;				//	addq.w #4,d0
							//	cmp.w d2,d0
		if (d0>=d2)			//	blo.s .nowrap
		{
			PlotChar();		//	bsr.w PlotChar			;preserves a0
			d0=0;			//	clr.w d0
		}					//.nowrap:
		else 	ScrollCtr=d0;		//	move.w d0,ScrollCtr

	}						//.skipscroll:

//;@clear clouds was here

	BounceScroller();			//	bsr.w BounceScroller

//    *--- plot clouds ---*

	a1 = SkyBufferL;			//	move.l SkyBufferL(PC),a1
							//	movem.l CloudCoordsLP(PC),a3-a4
	addq.w #8,a4
	moveq #cloudcount-1,d7
.drawl:
	move.l (a3)+,a0			;bob
	move.l (a3)+,a2			;mask

;	move.w (a3)+,d0			;x coord
	addq.w #2,a3
	move.w (a4),d0			;x coord from last frame
	move.w (a3)+,d1			;y coord
	move.w (a3)+,d2			;width
	move.w (a3)+,d3			;height

	add.w (a3)+,d0			;x speed, move cloud
	move.w #320,d5
	add.w d2,d5

	cmp.w #320+112,d0
	blt.b .nowrapx
	sub.w d5,d0
.nowrapx:
	move.w d0,-10(a3)		;replace coord

	move.l #0x0fca0000,d4		;cookie-cut blit
	bsr.w PlotBob
	lea cloudstructsize(a4),a4
	dbf d7,.drawl

//    ---  sine lookup for raster bar vert. pos.  ---

	lea Sine,a0
	move.w SineCtr,d6
	move.w #0x4d-6+37,d7
	add.w (a0,d6.w),d7

	addq.w #2,d6
	cmp.w #SineEnd-Sine,d6
	blt.s .nowrap2
	moveq #0,d6
.nowrap2:
	move.w d6,SineCtr


//    ---  in front or behind flag  ---

	lea BarInFront,a2	;default source address for RGB color values 
	cmp.w #50*2,d6
	blt.s .behind
	cmp.w #150*2,d6
	bge.s .behind
	bra.s .cont
.behind:
	lea BarBehind,a2
.cont:

	lea waitras1,a0
	move d7,d0
	moveq #6-1,d1
.l:
	move.b d0,(a0)
	addq.w #1,d0

	move.w (a2)+,d2			;background color from list
	move.w d2,6(a0)
	move.w (a2)+,6+4*1(a0)
	move.w (a2)+,6+4*2(a0)
	move.w (a2)+,6+4*3(a0)
	move.w (a2)+,6+4*4(a0)
	move.w (a2)+,6+4*5(a0)
	move.w (a2)+,6+4*6(a0)
	move.w (a2)+,6+4*7(a0)

	move.w (a2)+,6+4*8(a0)		;2nd playfield
	move.w (a2)+,6+4*9(a0)
	move.w (a2)+,6+4*10(a0)
	move.w (a2)+,6+4*11(a0)
	move.w (a2)+,6+4*12(a0)
	move.w (a2)+,6+4*13(a0)
	move.w (a2)+,6+4*14(a0)

	add.w #4*(9+7),a0		;step to next.
	DBF d1,.l

;	movem.l (sp)+,d0-a6
	rts


VBint:					;Blank template VERTB interrupt
	movem.l d0-a6,-(sp)		;Save used registers
	lea 0xdff000,a6
	btst #5,0x1f(a6)			;INTREQR check if it's our vertb int.
	beq.s .notvb

//    *--- double buffering for the clouds ---*

	movem.l SkyBufferL(PC),a0-a1
	exg a0,a1
	movem.l a0-a1,SkyBufferL

	movem.l CloudCoordsLP(PC),a0-a1
	exg a0,a1
	movem.l a0-a1,CloudCoordsLP

//    *--- playfield 2 ptrs ---*

	move.l SkyBufferL(PC),a0		;ptr to first bitplane of logo
	lea CopSkyBplP,a1	;where to poke the bitplane pointer words.
	lea 14(a0),a0
	move #3-1,d0
.bpll2:
	move.l a0,d1
	swap d1
	move.w d1,2(a1)		;hi word
	swap d1
	move.w d1,6(a1)		;lo word

	addq #8,a1		;point to next bpl to poke in copper
	lea skybpl(a0),a0
	dbf d0,.bpll2

	bsr.w Part1

	IFD Measure
	move.w #0x722,0x180(a6)
	move.l 4(a6),d0
	lsr.l #8,d0
	and.w #512-1,d0
	cmp.w #0x130,d0
	bhs.s .nomax
	cmp.w MaxVpos(PC),d0
	blo.s .nomax
	move.w d0,MaxVpos
.nomax:
	ENDC

	moveq #0x20,d0			;poll irq bit
	move.w d0,0x9c(a6)		;0xdff09c=INTREQ
	move.w d0,0x9c(a6)
.notvb:
	movem.l (sp)+,d0-a6		;restore
	rte
	
row	=288*3*20/8
col	=4

PlotChar:	;a0=scrollptr
;	movem.l d0-a6,-(sp)
	move.l ScrollPtr(PC),a0
	lea 0xdff000,a6

	moveq #0,d0
	move.b (a0)+,d0			;ASCII value
	cmp.b #32,d0
	bhs.s .char
.commands:
	tst.b d0
	bne.s .nowraptext
	lea ScrollText(PC),a0
	bra.s .readnext
.nowraptext:
	cmp.b #1,d0
	bne.s .notogglebounce
	not.b Cmd_Bounce
	move.w #16,BounceYspeed
	bra.s .readnext
.notogglebounce:
	cmp.b #2,d0
	bne.s .stop
	moveq #0,d0
	move.b (a0)+,d0
	move.w d0,Cmd_StopCount
.readnext:
	move.b (a0)+,d0
.stop:
.char:
	
	move.l a0,ScrollPtr
	move.b d0,LastChar

	sub.w #32,d0
	lea FontTbl(PC),a0
	move.b (a0,d0.w),d0
	divu #9,d0			;row
	move.l d0,d1
	swap d1				;remainder (column)

	mulu #row,d0
	mulu #col,d1

	add.l d1,d0			;offset into font bitmap
	add.l #Font,d0

	WAITBLIT
	move.l #0x09f00000,BLTCON0(a6)
	move.l #0xffffffff,BLTAFWM(a6)
	move.l d0,BLTAPTH(a6)
	move.l #Screen+ScrBpl*3*plotY+plotX/8,BLTDPTH(a6)
	move.w #FontBpl-col,BLTAMOD(a6)
	move.w #ScrBpl-col,BLTDMOD(a6)

	move.w #20*3*64+2,BLTSIZE(a6)
;	movem.l (sp)+,d0-a6
	rts

/*
;Chan	Function
;A	Cloud Mask
;B	Cloud
;C	Destination playfield
;D	Destination playfield
*/

PlotBob:		;d0-d3/a0-a2=x,y,width,h,src,dest,mask,BLTCON0+1.l
	movem.l d0-d3/d5/a1,-(sp)
	add.w #15,d2			;round up
	lsr.w #4,d2			;word width

	move.w d0,d5			;x position
	asr.w #4,d5			;in words
	add.w d5,d5			;*2=byte offset into destination

	muls #skybwid,d1		;y offset
	ext.l d5
	add.l d5,d1
	add.l d1,a1			;dest address

	and.w #0xf,d0			;shift nibble
	ror.w #4,d0			;to top nibble

	move.w d0,d5			;and put in
	swap d5				;both words
	move.w d0,d5			;of BLTCON
	
	or.l d4,d5
	
	mulu #3*64,d3			;calculate blit size
	add.w d2,d3

	add.w d2,d2			;w/8
	neg.w d2
	add.w #SkyBpl,d2		;=SkyBpl-w/8

	WAITBLIT

	move.l d5,BLTCON0(a6)
	move.l #0xffffffff,BLTAFWM(a6)
	move.l a2,BLTAPTH(a6)
	move.l a0,BLTBPTH(a6)
	move.l a1,BLTCPTH(a6)
	move.l a1,BLTDPTH(a6)
	clr.l BLTBMOD(a6)
	move.w d2,BLTCMOD(a6)
	move.w d2,BLTDMOD(a6)
	move.w d3,BLTSIZE(a6)
	movem.l (sp)+,d0-d3/d5/a1
	rts


Scrollit:
//    ---  scroll!  ---
bltoffs	=plotY*ScrBpl*3

blth	=20
bltw	=w/16
bltskip	=0				;modulo
brcorner=blth*ScrBpl*3-2

;	movem.l d0-a6,-(sp)
	lea 0xdff000,a6
	WAITBLIT
	move.l #0x49f00002,BLTCON0(a6)
	move.l #0xffffffff,BLTAFWM(a6)
	move.l #Screen+bltoffs+brcorner,BLTAPTH(a6)
	move.l #Screen+bltoffs+brcorner,BLTDPTH(a6)
	move.w #bltskip,BLTAMOD(a6)
	move.w #bltskip,BLTDMOD(a6)

	move.w #blth*3*64+bltw,BLTSIZE(a6)
;	movem.l (sp)+,d0-a6
	rts

Init:
	movem.l d0-a6,-(sp)

	moveq #0,d1
	lea Screen,a1
	move.w #bplsize*fontbpls/2-1,d0
.l:	move.w #0,(a1)+
	addq.w #1,d1
	dbf d0,.l

	lea Sky,a1
	lea Sky2,a2		;clear 2nd buffer also
	lea SkyBufferL(PC),a3
	move.l a1,(a3)+
	move.l a2,(a3)+		;Double-buffer list initialized

	lea Sky,a1
	lea Sky2,a2		;clear 2nd buffer also

	moveq #0,d0		;clear buffers
	move.w #(SkyE-Sky)/8-1,d7
.l7:	move.l d0,(a1)+
	move.l d0,(a1)+
	move.l d0,(a2)+
	move.l d0,(a2)+
	dbf d7,.l7

//    *--- playfield 1 ptrs ---*

	lea Logo,a0		;ptr to first bitplane of logo
	lea CopBplP,a1		;where to poke the bitplane pointer words.
	move #3-1,d0
.bpll:
	move.l a0,d1
	swap d1
	move.w d1,2(a1)		;hi word
	swap d1
	move.w d1,6(a1)		;lo word

	addq #8,a1		;point to next bpl to poke in copper
	lea logobpl(a0),a0
	dbf d0,.bpll

//    *--- playfield 2 ptrs ---*

	lea Sky+14,a0		;ptr to first bitplane of logo
	lea CopSkyBplP,a1	;where to poke the bitplane pointer words.
	move #3-1,d0
.bpll2:
	move.l a0,d1
	swap d1
	move.w d1,2(a1)		;hi word
	swap d1
	move.w d1,6(a1)		;lo word

	addq #8,a1		;point to next bpl to poke in copper
	lea skybpl(a0),a0
	dbf d0,.bpll2


	lea SprP,a1
	lea StarSpr,a0
	move.l a0,d1
	moveq #2-1,d0
.sprpl:	
	swap d1
	move.w d1,2(a1)
	swap d1
	move.w d1,6(a1)
	addq.w #8,a1
	add.l #(StarSpr2-StarSpr),d1 
	dbf d0,.sprpl	

	lea NullSpr,a0
	move.l a0,d1
	moveq #6-1,d0
.sprpl2:
	swap d1
	move.w d1,2(a1)
	swap d1
	move.w d1,6(a1)
	addq.w #8,a1
	DBF d0,.sprpl2

	lea FontE-7*2,a0
	lea FontPalP+2,a1
	moveq #7-1,d0
.coll:	move.w (a0)+,(a1)+
	addq.w #2,a1
	DBF d0,.coll

//    *--- initialize sprites ---*

	lea StarSpr,a1
	moveq #0x2c,d0
	moveq #26-1,d7
.skyl:	
	move.b d0,(a1)+			;vstart
	addq.b #1,(a1)+			;add speed to hpos
	addq.b #1,d0			;increase vstop value
	move.b d0,(a1)+			;vstop
	addq.b #1,d0			;increase vstop value
	addq.w #5,a1			;skip to next sprite control words

	move.b d0,(a1)+
	addq.b #2,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	addq.w #5,a1

	move.b d0,(a1)+
	addq.b #1,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	addq.w #5,a1

	move.b d0,(a1)+
	addq.b #3,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	addq.w #5,a1

	dbf d7,.skyl

	move.b d0,(a1)+			;vstart
	addq.b #1,(a1)+			;add speed to hpos
	addq.b #1,d0			;increase vstop value
	move.b d0,(a1)+			;vstop
	addq.b #1,d0			;increase vstop value
	addq.w #5,a1			;skip to next sprite control words

	move.b d0,(a1)+
	addq.b #2,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	addq.w #5,a1

//    *--- below line 0xff ---*

	moveq #%00000110,d6

	move.b d0,(a1)+
	addq.b #1,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	move.b d6,(a1)+
	addq.w #4,a1			;skip to next sprite control words

	move.b d0,(a1)+
	addq.b #3,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	move.b d6,(a1)+
	addq.w #4,a1			;skip to next sprite control words

	moveq #5-1,d7
.floorl:	
	move.b d0,(a1)+			;vstart
	addq.b #1,(a1)+			;add speed to hpos
	addq.b #1,d0			;increase vstop value
	move.b d0,(a1)+			;vstop
	addq.b #1,d0			;increase vstop value
	move.b d6,(a1)+
	addq.w #4,a1			;skip to next sprite control words

	move.b d0,(a1)+
	addq.b #2,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	move.b d6,(a1)+
	addq.w #4,a1			;skip to next sprite control words

	move.b d0,(a1)+
	addq.b #1,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	move.b d6,(a1)+
	addq.w #4,a1			;skip to next sprite control words

	move.b d0,(a1)+
	addq.b #3,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	move.b d6,(a1)+
	addq.w #4,a1			;skip to next sprite control words

	dbf d7,.floorl

//    *--- odd lines sprites ---*

	lea StarSpr2,a1
	moveq #0x2d,d0
	moveq #26-1,d7
.skyl2:
	move.b d0,(a1)+			;vstart
	addq.b #1,(a1)+			;add speed to hpos
	addq.b #1,d0			;increase vstop value
	move.b d0,(a1)+			;vstop
	addq.b #1,d0			;increase vstop value
	addq.w #5,a1			;skip to next sprite control words

	move.b d0,(a1)+
	addq.b #2,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	addq.w #5,a1

	move.b d0,(a1)+
	addq.b #1,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	addq.w #5,a1

	move.b d0,(a1)+
	addq.b #3,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	addq.w #5,a1

	dbf d7,.skyl2

	move.b d0,(a1)+			;vstart
	addq.b #1,(a1)+			;add speed to hpos
	addq.b #1,d0			;increase vstop value
	move.b d0,(a1)+			;vstop
	addq.b #1,d0			;increase vstop value
	addq.w #5,a1			;skip to next sprite control words

	move.b d0,(a1)+
	addq.b #2,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	move.b #2,(a1)+
	addq.w #4,a1			;skip to next sprite control words

//    *--- below line 0xff ---*

	moveq #%00000110,d6

	move.b d0,(a1)+
	addq.b #1,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	move.b d6,(a1)+
	addq.w #4,a1			;skip to next sprite control words

	move.b d0,(a1)+
	addq.b #3,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	move.b d6,(a1)+
	addq.w #4,a1			;skip to next sprite control words

	moveq #5-1,d7
.floorl2:
	move.b d0,(a1)+			;vstart
	addq.b #1,(a1)+			;add speed to hpos
	addq.b #1,d0			;increase vstop value
	move.b d0,(a1)+			;vstop
	addq.b #1,d0			;increase vstop value
	move.b d6,(a1)+
	addq.w #4,a1			;skip to next sprite control words

	move.b d0,(a1)+
	addq.b #2,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	move.b d6,(a1)+
	addq.w #4,a1			;skip to next sprite control words

	move.b d0,(a1)+
	addq.b #1,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	move.b d6,(a1)+
	addq.w #4,a1			;skip to next sprite control words

	move.b d0,(a1)+
	addq.b #3,(a1)+
	addq.b #1,d0
	move.b d0,(a1)+
	addq.b #1,d0			;increase vstop value
	move.b d6,(a1)+
	addq.w #4,a1			;skip to next sprite control words

	dbf d7,.floorl2

	movem.l (sp)+,d0-a6
	rts

CopyB:	;d0,a0,a1=count,source,destination
.l:	move.b (a0)+,(a1)+
	subq.l #1,d0
	bne.s .l
	rts

BlitWait:
	tst DMACONR(a6)			;for compatibility
.waitblit:
	btst #6,DMACONR(a6)
	bne.s .waitblit
	rts

BounceScroller:
	MOVEM.L D0-D1/A0-A1,-(SP)

	lea Screen,a0		;ptr to first bitplane of font
	move.w BounceY(PC),d0
	move.w BounceYaccel(PC),d1
	add.w d1,BounceYspeed
	add.w BounceYspeed(PC),d0
	bpl.s .nobounce
	move.w #32,BounceYspeed
	clr.w d0
.nobounce:

	tst.b Cmd_Bounce
	bne.s .bounce2
	moveq #1*8,d0
.bounce2:
	move.w d0,BounceY

	lsr.w #3,d0

	mulu #3*scrbpl,d0
	add.l d0,a0

	lea ScrBplP,a1		;where to poke the bitplane pointer words.
	moveq #fontbpls-1,d0
.bpll2:	move.l a0,d1
	swap d1
	move.w d1,2(a1)		;hi word
	swap d1
	move.w d1,6(a1)		;lo word

	addq #8,a1		;point to next bpl to poke in copper
	lea ScrBpl(a0),a0
	dbf d0,.bpll2

	MOVEM.L (SP)+,D0-D1/A0-A1
	RTS

	even

//********** PLAYROUTINE CODE **********
//;Note: if this is put in its own section (or compiled as separate binary), then
//;jsr <addr>+P61_InitOffset,P61_MusicOffset,P61_EndOffset,P61_SetPositionOffset
//;to call the routines.

Playrtn:
	include "P6112/P6112-Play.i"

//********** DATA **********
BarBehind:
	dc.w 0x558	;color00 value
	logocolors
	dc.w 0x558	;color00 value
	dc.w 0x558	;color00 value
	dc.w 0x558	;color00 value
	dc.w 0x558	;color00 value
	dc.w 0x558	;color00 value
	dc.w 0x558	;color00 value
	dc.w 0x558	;color00 value

	dc.w 0x99d	;color00...
	logocolors
	dc.w 0x99d	;color00...
	dc.w 0x99d	;color00...
	dc.w 0x99d	;color00...
	dc.w 0x99d	;color00...
	dc.w 0x99d	;color00...
	dc.w 0x99d	;color00...
	dc.w 0x99d	;color00...

	dc.w 0xfff
	logocolors
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff

	dc.w 0x99d
	logocolors
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d

	dc.w 0x558
	logocolors
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558

	dc.w logobgcol			;restore
	logocolors
	cloudcolors

BarInFront:
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558

	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d

	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff
	dc.w 0xfff

	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d
	dc.w 0x99d

	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558
	dc.w 0x558

	dc.w logobgcol
	logocolors
	cloudcolors

FontTbl:
	dc.b 43,38
	dcb.b 5,0
	dc.b 42
	dcb.b 4,0
	dc.b 37,40,36,41
	dc.b 26,27,28,29,30,31,32,33,34,35
	dcb.b 5,0
	dc.b 39,0
	dc.b 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21
	dc.b 22,23,24,25
	EVEN

BounceY:
	dc.w 1*8
BounceYaccel:
	dc.w -1

void *CloudCoordsLP[]=			//CloudCoordsLP:
	{
		CloudCoordsL,			//	dc.l CloudCoordsL
		CloudCoordsL2			//	dc.l CloudCoordsL2
	};


struct cloud CloudCoordsL =							//CloudCoordsL:
	{
		Cloud3,				//	dc.l Cloud3
		Cloud3Mask,			//	dc.l Cloud3Mask
		90,					//	dc.w 90			;x
		170,					//	dc.w 170		;y
		48,					//	dc.w 48			;width
		15,					//	dc.w 15			;height
		1,					//	dc.w 1			;x speed
	};

	dc.l Cloud3
	dc.l Cloud3Mask
	dc.w 284		;x
	dc.w 199		;y
	dc.w 48			;width
	dc.w 15			;height
	dc.w 1			;x speed

;	dc.l Cloud3
;	dc.l Cloud3Mask
;	dc.w 184		;x
;	dc.w 135		;y
;	dc.w 48			;width
;	dc.w 15			;height
;	dc.w 1			;x speed

	dc.l Cloud3
	dc.l Cloud3Mask
	dc.w 218		;x
	dc.w 95			;y
	dc.w 48			;width
	dc.w 15			;height
	dc.w 2			;x speed

	dc.l Cloud2
	dc.l Cloud2Mask
	dc.w 320		;x
	dc.w 183		;y
	dc.w 64			;width
	dc.w 24			;height
	dc.w 2			;x speed

	dc.l Cloud2
	dc.l Cloud2Mask
	dc.w 123		;x
	dc.w 123		;y
	dc.w 64			;width
	dc.w 24			;height
	dc.w 2			;x speed

	dc.l Cloud2
	dc.l Cloud2Mask
	dc.w 210		;x
	dc.w 150		;y
	dc.w 64			;width
	dc.w 24			;height
	dc.w 2			;x speed

	dc.l Cloud2
	dc.l Cloud2Mask
	dc.w 156		;x
	dc.w 007		;y
	dc.w 64			;width
	dc.w 24			;height
	dc.w 3			;x speed

	dc.l Cloud
	dc.l CloudMask
	dc.w 240		;x
	dc.w 105		;y
	dc.w 112		;width
	dc.w 38			;height
	dc.w 3			;x speed

	dc.l Cloud
	dc.l CloudMask
	dc.w 290		;x
	dc.w 47			;y
	dc.w 112		;width
	dc.w 38			;height
	dc.w 4			;x speed

	dc.l Cloud
	dc.l CloudMask
	dc.w 0			;x
	dc.w 27			;y
	dc.w 112		;width
	dc.w 38			;height
	dc.w 5			;x speed

CloudCoordsLE:

CloudCoordsL2:
	dc.l Cloud3
	dc.l Cloud3Mask
	dc.w 90			;x
	dc.w 170		;y
	dc.w 48			;width
	dc.w 15			;height
	dc.w 1			;x speed

	dc.l Cloud3
	dc.l Cloud3Mask
	dc.w 284		;x
	dc.w 199		;y
	dc.w 48			;width
	dc.w 15			;height
	dc.w 1			;x speed

//;	dc.l Cloud3
//;	dc.l Cloud3Mask
//;	dc.w 184		;x
//;	dc.w 135		;y
//;	dc.w 48			;width
//;	dc.w 15			;height
//;	dc.w 1			;x speed

	dc.l Cloud3
	dc.l Cloud3Mask
	dc.w 218		;x
	dc.w 95			;y
	dc.w 48			;width
	dc.w 15			;height
	dc.w 2			;x speed

	dc.l Cloud2
	dc.l Cloud2Mask
	dc.w 320		;x
	dc.w 183		;y
	dc.w 64			;width
	dc.w 24			;height
	dc.w 2			;x speed

	dc.l Cloud2
	dc.l Cloud2Mask
	dc.w 123		;x
	dc.w 123		;y
	dc.w 64			;width
	dc.w 24			;height
	dc.w 2			;x speed

	dc.l Cloud2
	dc.l Cloud2Mask
	dc.w 210		;x
	dc.w 150		;y
	dc.w 64			;width
	dc.w 24			;height
	dc.w 2			;x speed

	dc.l Cloud2
	dc.l Cloud2Mask
	dc.w 156		;x
	dc.w 007		;y
	dc.w 64			;width
	dc.w 24			;height
	dc.w 3			;x speed

	dc.l Cloud
	dc.l CloudMask
	dc.w 240		;x
	dc.w 105		;y
	dc.w 112		;width
	dc.w 38			;height
	dc.w 3			;x speed

	dc.l Cloud
	dc.l CloudMask
	dc.w 290		;x
	dc.w 47			;y
	dc.w 112		;width
	dc.w 38			;height
	dc.w 4			;x speed

	dc.l Cloud
	dc.l CloudMask
	dc.w 0			;x
	dc.w 27			;y
	dc.w 112		;width
	dc.w 38			;height
	dc.w 5			;x speed

CloudCoordsL2E:

gfxname:
	dc.b "graphics.library",0
	EVEN

Sine:	INCBIN "Sine.37.200.w"
SineEnd:

ScrollPtr:
	dc.l ScrollText
ScrollText:
	dc.b "HELLO, AMIGA CODERS! THIS IS PHOTON PRESENTING THE "
	dc.b "   // ASMSKOOL",1," DEMO //   " FROM THE AMIGA HARDWARE "
	dc.b "PROGRAMMING SERIES ON YOUTUBE. IT'S A SIMPLE "
	dc.b "DEMO WITH A        -BOUNC!NG-",2,180,"        SCROLLER, "
	dc.b "MOVING RASTERBAR, BOB PARALLAX, AND SPRITE STARFIELD.        "
	dc.b "GREETINGS TO        SCOOPEX MEMBERS AND ALL DEMOSCENE FRIENDS, "
	dc.b "EAB FRIENDS, AND SPECIAL SHOUTS TO TONI, JENS, BIFAT, BONEFISH, "
	dc.b "HIGHPUFF, MAGNUS T, PHAZE101, WEI-JU WU, DUTCH RETRO GUY, JEL, "
	dc.b "TOMAZ KRAGELJ, MCGEEZER AND ANTIRIAD        I HOPE WE ALL STAY "
	dc.b "SAFE, AND ENJOY THE LAST DAYS OF THIS GREAT SUMMER!"

	dcb.b w/32,' '
	dc.b 1,' '
ScrollTextWrap:
	dc.b 0

uint8		LastChar=0;								//LastChar:
												//	dc.b 0
uint8		Cmd_Bounce=0;							//Cmd_Bounce:
												//	dc.b 0
uint16	Cmd_StopCount=0;							//Cmd_StopCount:
												//	dc.w 0

ScrollCtr:
	dc.w 0
BounceYspeed:
	dc.w 0
SineCtr:
	dc.w 0
SkyBufferL:
	dc.l 0
	dc.l 0
SkyBufferLE:


	IFD Measure
MaxVpos:dc.w 0
	ENDC

	SECTION TutData,DATA_C
StarSpr:
.x:	SET 1
	REPT 32
.tmpx:	SET ((.x*0x751+0xdeadbeef)/(.x&0x55))&0xff
	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w 0x8000,0x0000
.x:	SET (.x+2)&0xffff
.tmpx:	SET ((.x*0x753+0xeadbeefd)/(.x&0x55))&0xff
	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w 0x0000,0x8000
.x:	SET (.x+4)&0xffff
.tmpx:	SET ((.x*0x755+0xadbeefde)/(.x&0x55))&0xff
	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w 0x8000,0x0000
.x:	SET (.x+8)&0xffff
.tmpx:	SET ((.x*0x757+0xdbeefdea)/(.x&0x55))&0xff
	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w 0xc000,0xc000
.x:	SET (.x+16)&0xffff
	ENDR
	dc.w 0,0

StarSpr2:
.x:	SET 0x77
	REPT 32
.tmpx:	SET ((.x*0x751+0xdeadbeef)/(.x&0x55))&0xff
	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w 0x8000,0x0000
.x:	SET (.x+2)&0xffff
.tmpx:	SET ((.x*0x753+0xeadbeefd)/(.x&0x55))&0xff
	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w 0x0000,0x8000
.x:	SET (.x+4)&0xffff
.tmpx:	SET ((.x*0x755+0xadbeefde)/(.x&0x55))&0xff
	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w 0x8000,0x0000
.x:	SET (.x+8)&0xffff
.tmpx:	SET ((.x*0x757+0xdbeefdea)/(.x&0x55))&0xff
	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w 0xc000,0xc000
.x:	SET (.x+16)&0xffff
	ENDR
	dc.w 0,0

NullSpr:
	dc.w 0x2a20,0x2b00
	dc.w 0,0
	dc.w 0,0

Copper:
	dc.w 0x1fc,0			;slow fetch mode, AGA compatibility
	dc.w 0x100,0x0200
	dc.b 0,0x8e,0x2c,0x81
	dc.b 0,0x90,0x2c,0xc1
	dc.w 0x92,0x38
	dc.w 0x94,0xd0

	dc.w 0x108,logobwid-logobpl
	dc.w 0x10a,skybwid-320/8

	dc.w 0x102,0
	dc.w 0x104,0x20

	dc.w 0x1a2,0x99b			;sprite colors
	dc.w 0x1a4,0xcce
	dc.w 0x1a6,0xfff
SprP:
	dc.w 0x120,0
	dc.w 0x122,0
	dc.w 0x124,0
	dc.w 0x126,0
	dc.w 0x128,0
	dc.w 0x12a,0
	dc.w 0x12c,0
	dc.w 0x12e,0
	dc.w 0x130,0
	dc.w 0x132,0
	dc.w 0x134,0
	dc.w 0x136,0
	dc.w 0x138,0
	dc.w 0x13a,0
	dc.w 0x13c,0
	dc.w 0x13e,0

CopBplP:
	dc.w 0xe0,0		;playfield 1
	dc.w 0xe2,0
	dc.w 0xe8,0
	dc.w 0xea,0
	dc.w 0xf0,0
	dc.w 0xf2,0
CopSkyBplP:
	dc.w 0xe4,0		;playfield 2
	dc.w 0xe6,0
	dc.w 0xec,0
	dc.w 0xee,0
	dc.w 0xf4,0
	dc.w 0xf6,0
		
	dc.w 0x0180,logobgcol
	dc.w 0x100,0x6600
	dc.w 0x2c07,0xfffe

LogoPal:
	dc.w 0x0180,0x044f,0x0182,0x068e,0x0184,0x0adf,0x0186,0x0dff
	dc.w 0x0188,0x09bf,0x018a,0x056d,0x018c,0x044b,0x018e,0x033a
CloudPal:
	dc.w 0x0192,0x066f,0x0194,0x077f,0x0196,0x088e
	dc.w 0x0198,0x0aae,0x019a,0x0bbe,0x019c,0x0dde,0x019e,0x0eee

waitras1:
	dc.w 0x8007,0xfffe
	dc.w 0x180,0x558
	dc.w 0x182,0
	dc.w 0x184,0
	dc.w 0x186,0
	dc.w 0x188,0
	dc.w 0x18a,0
	dc.w 0x18c,0
	dc.w 0x18e,0
	dc.w 0x192,0
	dc.w 0x194,0
	dc.w 0x196,0
	dc.w 0x198,0
	dc.w 0x19a,0
	dc.w 0x19c,0
	dc.w 0x19e,0
waitras2:
	dc.w 0x8107,0xfffe
	dc.w 0x180,0x99d
	dc.w 0x182,0
	dc.w 0x184,0
	dc.w 0x186,0
	dc.w 0x188,0
	dc.w 0x18a,0
	dc.w 0x18c,0
	dc.w 0x18e,0
	dc.w 0x192,0
	dc.w 0x194,0
	dc.w 0x196,0
	dc.w 0x198,0
	dc.w 0x19a,0
	dc.w 0x19c,0
	dc.w 0x19e,0
waitras3:
	dc.w 0x8207,0xfffe
	dc.w 0x180,0xfff
	dc.w 0x182,0
	dc.w 0x184,0
	dc.w 0x186,0
	dc.w 0x188,0
	dc.w 0x18a,0
	dc.w 0x18c,0
	dc.w 0x18e,0
	dc.w 0x192,0
	dc.w 0x194,0
	dc.w 0x196,0
	dc.w 0x198,0
	dc.w 0x19a,0
	dc.w 0x19c,0
	dc.w 0x19e,0
waitras4:
	dc.w 0x8307,0xfffe
	dc.w 0x180,0x99d
	dc.w 0x182,0
	dc.w 0x184,0
	dc.w 0x186,0
	dc.w 0x188,0
	dc.w 0x18a,0
	dc.w 0x18c,0
	dc.w 0x18e,0
	dc.w 0x192,0
	dc.w 0x194,0
	dc.w 0x196,0
	dc.w 0x198,0
	dc.w 0x19a,0
	dc.w 0x19c,0
	dc.w 0x19e,0
waitras5:
	dc.w 0x8407,0xfffe
	dc.w 0x180,0x558
	dc.w 0x182,0
	dc.w 0x184,0
	dc.w 0x186,0
	dc.w 0x188,0
	dc.w 0x18a,0
	dc.w 0x18c,0
	dc.w 0x18e,0
	dc.w 0x192,0
	dc.w 0x194,0
	dc.w 0x196,0
	dc.w 0x198,0
	dc.w 0x19a,0
	dc.w 0x19c,0
	dc.w 0x19e,0
waitras6:
	dc.w 0x8507,0xfffe
	dc.w 0x180,logobgcol
	dc.w 0x182,0
	dc.w 0x184,0
	dc.w 0x186,0
	dc.w 0x188,0
	dc.w 0x18a,0
	dc.w 0x18c,0
	dc.w 0x18e,0
	dc.w 0x192,0
	dc.w 0x194,0
	dc.w 0x196,0
	dc.w 0x198,0
	dc.w 0x19a,0
	dc.w 0x19c,0
	dc.w 0x19e,0

	dc.w 0x96bf,0xfffe

FontPalP:
	dc.w 0x0182,0x0ddd,0x0184,0x0833,0x0186,0x0334
	dc.w 0x0188,0x0a88,0x018a,0x099a,0x018c,0x0556,0x018e,0x0633

ScrBplP:
	dc.w 0xe0,0
	dc.w 0xe2,0
	dc.w 0xe8,0
	dc.w 0xea,0
	dc.w 0xf0,0
	dc.w 0xf2,0
	dc.w 0x108,ScrBpl*FontBpls-320/8
	dc.w 0x92,0x38
	dc.w 0x94,0xd0

	dc.w 0x9707,0xfffe
	dc.w 0x180,0x44e
	dc.w 0x9807,0xfffe
	dc.w 0x180,0x44f
	dc.w 0x9907,0xfffe
	dc.w 0x180,0x44e

	dc.w 0x9d07,0xfffe
	dc.w 0x180,0x44d
	dc.w 0x9e07,0xfffe
	dc.w 0x180,0x44e
	dc.w 0x9f07,0xfffe
	dc.w 0x180,0x44d

	dc.w 0xa407,0xfffe
	dc.w 0x180,0x44c
	dc.w 0xa507,0xfffe
	dc.w 0x180,0x44d
	dc.w 0xa607,0xfffe
	dc.w 0x180,0x44c

	dc.w 0xab07,0xfffe
	dc.w 0x180,0x43c
	dc.w 0xac07,0xfffe
	dc.w 0x180,0x44c
	dc.w 0xad07,0xfffe
	dc.w 0x180,0x43c

	dc.w 0xb207,0xfffe
	dc.w 0x180,0x33b
	dc.w 0xb307,0xfffe
	dc.w 0x180,0x43c
	dc.w 0xb407,0xfffe
	dc.w 0x180,0x33b

	dc.w 0xb907,0xfffe
	dc.w 0x180,0x33a
	dc.w 0xba07,0xfffe
	dc.w 0x180,0x33b
	dc.w 0xbb07,0xfffe
	dc.w 0x180,0x33a

	dc.w 0xc007,0xfffe
	dc.w 0x180,0x339
	dc.w 0xc107,0xfffe
	dc.w 0x180,0x33a
	dc.w 0xc207,0xfffe
	dc.w 0x180,0x339

//;tone down clouds 1 notch

	dc.w 0x0192,0x044b,0x0194,0x055b,0x0196,0x066a
	dc.w 0x0198,0x088a,0x019a,0x099a,0x019c,0x0bbb,0x019e,0x0ccc

	dc.w 0xc707,0xfffe
	dc.w 0x180,0x329
	dc.w 0xc807,0xfffe
	dc.w 0x180,0x339
	dc.w 0xc907,0xfffe
	dc.w 0x180,0x329

	dc.w 0xce07,0xfffe
	dc.w 0x180,0x228
	dc.w 0xcf07,0xfffe
	dc.w 0x180,0x329
	dc.w 0xd007,0xfffe
	dc.w 0x180,0x228

	dc.w 0xd507,0xfffe
	dc.w 0x180,0x227
	dc.w 0xd607,0xfffe
	dc.w 0x180,0x228
	dc.w 0xd707,0xfffe
	dc.w 0x180,0x227

	dc.w 0xdc07,0xfffe
	dc.w 0x180,0x226
	dc.w 0xdd07,0xfffe
	dc.w 0x180,0x227
	dc.w 0xde07,0xfffe
	dc.w 0x180,0x226

	dc.w 0xe307,0xfffe
	dc.w 0x180,bgcol
	dc.w 0xe407,0xfffe
	dc.w 0x180,0x226
	dc.w 0xe507,0xfffe
	dc.w 0x180,bgcol

	dc.w 0xffdf,0xfffe

//    ---  bottom plate start  ---

	dc.w 0x07df,0xfffe
	dc.w 0x10a,-(skybwid+320/8)
	dc.w 0x104,0x20

	dc.w 0x0192,0x0248,0x0194,0x0348,0x0196,0x0458
	dc.w 0x0198,0x0668,0x019a,0x0778,0x019c,0x09aa,0x019e,0x0abb

	dc.w 0x0807,0xfffe
	dc.w 0x180,0x236
	dc.w 0x0a07,0xfffe
	dc.w 0x180,0x247
	dc.w 0x0b07,0xfffe
	dc.w 0x180,0x236
	dc.w 0x0e07,0xfffe
	dc.w 0x180,0x258

	dc.w 0x0f07,0xfffe
	dc.w 0x180,0x236

	dc.w 0x1507,0xfffe
	dc.w 0x180,0x269

	dc.w 0x192,0x269
	dc.w 0x194,0x269
	dc.w 0x196,0x269
	dc.w 0x198,0x269
	dc.w 0x19a,0x269
	dc.w 0x19c,0x269
	dc.w 0x19e,0x269

	dc.w 0x1607,0xfffe
	dc.w 0x180,0x236

	dc.w 0x0192,0x0248,0x0194,0x0348,0x0196,0x0458
	dc.w 0x0198,0x0668,0x019a,0x0778,0x019c,0x09aa,0x019e,0x0abb

//    ---  mirror split  ---	
	dc.w 0x17df,0xfffe
	dc.w 0x182,0x0468
	dc.w 0x184,0x0235
	dc.w 0x186,0x0358
	dc.w 0x188,0x0689
	dc.w 0x18a,0x069a
	dc.w 0x18c,0x07bc
	dc.w 0x18e,0x08dd
	dc.w 0x108,(ScrBpl*FontBpls-320/8)-(ScrBpl*FontBpls*2)

	dc.w 0x2007,0xfffe
	dc.w 0x180,0x27a
	dc.w 0x182,0x27a
	dc.w 0x184,0x27a
	dc.w 0x186,0x27a
	dc.w 0x188,0x27a
	dc.w 0x18a,0x27a
	dc.w 0x18c,0x27a
	dc.w 0x18e,0x27a

	dc.w 0x192,0x27a
	dc.w 0x194,0x27a
	dc.w 0x196,0x27a
	dc.w 0x198,0x27a
	dc.w 0x19a,0x27a
	dc.w 0x19c,0x27a
	dc.w 0x19e,0x27a

	dc.w 0x2107,0xfffe
	dc.w 0x180,0x236
	dc.w 0x182,0x0468
	dc.w 0x184,0x0235
	dc.w 0x186,0x0358
	dc.w 0x188,0x0689
	dc.w 0x18a,0x069a
	dc.w 0x18c,0x07bc
	dc.w 0x18e,0x08dd

	dc.w 0x0192,0x0248,0x0194,0x0348,0x0196,0x0458
	dc.w 0x0198,0x0668,0x019a,0x0778,0x019c,0x09aa,0x019e,0x0abb

//    ---  bottom plate stop  ---
	dc.w 0x2c07,0xfffe
	dc.w 0x180,0x38b
	dc.w 0x2d07,0xfffe
	dc.w 0x180,0x235
	dc.w 0x2e07,0xfffe
	dc.w 0x180,0x247
	dc.w 0x2f07,0xfffe
	dc.w 0x180,0x258
//    ---  bottom plate thickness  ---
	dc.w 0x3007,0xfffe
	dc.w 0x180,bgcol

	dc.w 0xffff,0xfffe
CopperE:

Font:
	INCBIN "media/FastCarFont.284x100x3"
FontE:

Cloud:
	INCBIN "media/Cloud.112x38x3.raw"
CloudE:

Cloud2:
	INCBIN "media/Cloud.64x24x3.raw"
Cloud2E:

Cloud3:
	INCBIN "media/Cloud.48x15x3.raw"
Cloud3E:

CloudMask:
	INCBIN "media/Cloud.112x38x3.masks.raw"
CloudMaskE:

	load_raw("media/Cloud.64x24x3.masks.raw", 0, &Cloud2Mask, &Cloud2MaskE);

	load_raw("media/Cloud.48x15x3.masks.raw", 0, &Cloud3Mask, &Cloud3MaskE);

	load_raw("P61.new_ditty", 0, &Module1, &Module1E); // usecode 0xc00b43b

	load_raw("sky3centered.raw", (logobwid*6), &Logo, &LogoE);


//	SECTION TutBSS,BSS_C

void bss_c
{
	Screen = malloc( bplsize*fontbpls);
	ScreenE = screen + (bplsize*fontbpls):

	Sky = malloc( skybwid*(220+1));
	SkyE = Sky + ( skybwid*(220+1) ); 

	Sky2 = malloc( skybwid*(220+1));
	Sky2E = Sky2 + ( skybwid*(220+1) ); 

}

/*

	END


Bit	Channel


	1001
	ABCD -> D

0	000	0
1	001	0
2	010	0
3	011	0
4	100	1
5	101	1
6	110	1
7	111	1


%11110000	=0xf0


Plot Cloud

;Channel	Function
;A	Cloud Mask
;B	Cloud
;C	Destination playfield
;D	Destination playfield

	1111
	ABCD -> D

0	000	0
1	001	1
2	010	0
3	011	1
4	100	0
5	101	0
6	110	1
7	111	1


%11001010	=0xca

*/	


void load_raw(const char name, int extraSize, void **ptr, void **ptrE)
{
	BPTR fd;
	fd = FOpen( name, MODE_OLDFILE, 0  );
	uint64 size;

	*ptr = NULL;
	*ptrE = NULL;

	if (fd)
	{
		ChangeFilePosition( fd , 0 , OFFSET_END);
		size = GetFilePosition( fd );
		ChangeFilePosition( fd , 0 , OFFSET_BEGINNING);

		*ptr = malloc( size + extraSize );
		if (*ptr)
		{
			FRead( fd , *ptr, size ,1 );
			*ptrE = *ptr + size;
		}
		FClose(fd);
	}
}


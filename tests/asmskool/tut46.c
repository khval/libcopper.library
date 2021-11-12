
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

extern union reg_u *emu_stack_ptr;
union reg_u emu_stack[10000];

#ifdef __amigaos4__
struct Custom _custom;
struct Custom *custom = &_custom;	// store locally... handle things with do_functions();
#else
struct Custom *custom = 0xDFF000;
#endif

#define BLTCON0 0x040
#define BLTAFWM 0x044

#define BLTAPTH 0x052
#define BLTBPTH 0x04C
#define BLTCPTH 0x048
#define BLTDPTH 0x054

#define BLTAMOD 0x064
#define BLTBMOD 0x062 
#define BLTCMOD 0x060
#define BLTDMOD 0x066
#define BLTSIZE 0x058


#define mini_dump(a,b)

/*
void mini_dump(uint16 *ptr,int cnt)
{
	printf("-- mini dump --\n");

	while (cnt)
	{
		printf("%04x,%04x\n",*ptr++,*ptr++);
		cnt--;
	}
}
*/


/*
	SECTION TutDemo,CODE
	JUMPPTR Start

	INCDIR ""
	INCLUDE "PhotonsMiniStartup/PhotonsMiniWrapper1.04!.S"
	INCLUDE "Blitter-Register-List.S"
	INCLUDE "P61-Options.S"
*/
//    ---  screen buffer dimensions  ---


#pragma pack(push,2)
struct cloud
{
	uint8		*addr;
	uint8		*mask;
	uint16	x,y,width,height,xspeed;
};
#pragma pack(pop)


int16	Sine[200] ;												//Sine:	INCBIN "Sine.37.200.w"
int16	*SineEnd = Sine +200;										//SineEnd:

uint8 ScrollTextWrap = 0;											//	ScrollTextWrap:
															//		dc.b 0
uint16 ScrollCtr=0;												//	ScrollCtr:
															//		dc.w 0
int16 BounceYspeed=0;											// 	BounceYspeed:
															//		dc.w 0
int16 BounceY=1*8;											//	BounceY:
															//		dc.w 1*8
int16	 BounceYaccel=-1;											//	BounceYaccel:
															//		dc.w -1

uint16 SineCtr=0;												//	SineCtr:
															//		dc.w 0
uint32 SkyBufferL[]={0,0};										//	SkyBufferL:
															//		dc.l 0
															//		dc.l 0
uint32 *SkyBufferLE=SkyBufferL + sizeof(SkyBufferL)/sizeof(uint32);		//	SkyBufferLE:

uint8		LastChar=0;								//LastChar:
												//	dc.b 0
uint8		Cmd_Bounce=0;							//Cmd_Bounce:
												//	dc.b 0
uint16	Cmd_StopCount=0;							//Cmd_StopCount:
												//	dc.w 0
#define DEMO "tut46.c"

uint32 NullSpr[] = {
	 0x2a20,0x2b00
	, 0,0
	, 0,0 };

uint32 FontPalP[]={
	  0x0182,0x0ddd,0x0184,0x0833,0x0186,0x0334
	, 0x0188,0x0a88,0x018a,0x099a,0x018c,0x0556,0x018e,0x0633};

char ScrollText[] = 
	"HELLO, AMIGA CODERS! THIS IS PHOTON PRESENTING THE "
	"   // ASMSKOOL \x01" DEMO " FROM THE AMIGA HARDWARE "
	"PROGRAMMING SERIES ON YOUTUBE. IT'S A SIMPLE "
//	"DEMO WITH A        -BOUNC!NG-\x02\xB4        SCROLLER, "
	"MOVING RASTERBAR, BOB PARALLAX, AND SPRITE STARFIELD.        "
	"GREETINGS TO        SCOOPEX MEMBERS AND ALL DEMOSCENE FRIENDS, "
	"EAB FRIENDS, AND SPECIAL SHOUTS TO TONI, JENS, BIFAT, BONEFISH, "
	"HIGHPUFF, MAGNUS T, PHAZE101, WEI-JU WU, DUTCH RETRO GUY, JEL, "
	"TOMAZ KRAGELJ, MCGEEZER AND ANTIRIAD        I HOPE WE ALL STAY "
	"SAFE, AND ENJOY THE LAST DAYS OF THIS GREAT SUMMER!"
	"                                                                                                      "
	"\x01 " ;

extern char FontTbl[];

char *ScrollPtr = ScrollText;

extern uint16	BarBehind[];
extern uint16	BarInFront[];

uint8 *Logo = NULL;
uint8 *LogoE = NULL;
uint8 *Screen = NULL;
uint8 *ScreenE = NULL;
uint8 *Sky = NULL;
uint8 *SkyE = NULL;
uint8 *Sky2 = NULL;
uint8 *Sky2E = NULL;

uint8 *Cloud = NULL;
uint8 *CloudE = NULL;
uint8 *Cloud2 = NULL;
uint8 *Cloud2E = NULL;
uint8 *Cloud3 = NULL;
uint8 *Cloud3E = NULL;
uint8 *CloudMask = NULL;
uint8 *CloudMaskE = NULL;
uint8 *Cloud2Mask = NULL;
uint8 *Cloud2MaskE = NULL;
uint8 *Cloud3Mask = NULL;
uint8 *Cloud3MaskE = NULL;

uint16 *Copper = NULL;
uint16 *CopperE = NULL;
uint16 *SprP = NULL;
uint16 *CopBplP = NULL;
uint16 *CopSkyBplP = NULL;
uint16 *LogoPal = NULL;
uint16 *CloudPal = NULL;
uint16 *waitras1 = NULL;
uint16 *waitras2 = NULL;
uint16 *waitras3 = NULL;
uint16 *waitras4 = NULL;
uint16 *waitras5 = NULL;
uint16 *waitras6 = NULL;
uint16 *ScrBplP = NULL;
uint8 *Module1 = NULL;

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

#define SkyBpl		(320/8+14)
#define skybwid	(SkyBpl*3)
#define skyh		220

//    ---  font dimensions  ---
#define fontw		288
#define fonth		100
#define FontBpls	3
#define FontBpl	(fontw/8)

#define plotY	110
#define plotX	(w-32)

#define logobgcol	0x44f
#define bgcol		0x225

#define cloudcount	10
#define cloudstructsize	18

void Init();
void Main();
void PlotChar();
void PlotBob();
void Scrollit();
void 	BounceScroller();
bool load_raw(const char *name, int extraSize, void **ptr, void **ptrE);
void WaitMouse();
void WAITBLIT();

void CloudSet( struct cloud *c,  uint8  *addr, uint8 *mask, uint16 x, uint16 y,uint16 width,uint16 height,uint16 xspeed)
{
	c -> addr = addr;
	c -> mask = mask;
	c -> x = x;
	c -> y = y;
	c -> width = width;
	c -> height = height;
	c -> xspeed = xspeed;
};


//********************  MACROS  ********************

#define logocolors						\
	 0x068e,0x0adf,0x0dff				\
	, 0x09bf,0x056d,0x044b,0x033a		\


#define cloudcolors						\
	 0x066f,0x077f,0x088e				\
	, 0x0aae,0x0bbe,0x0dde,0x0eee		\

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

	cop_move_(0x080, (uint32) Copper);	//	move.l #Copper,0xdff080
									//	move.l #VBint,0x6c(a4)		;set vertb interrupt vector compatibly.
									//	move.w #0xc020,0xdff09a	;enable interrupts generally
									//							;and vertb specifically.

//    ---  Call P61_Init  ---
	movem_push(RD0,RA6);				//	movem.l d0-a6,-(sp)

	a0 = (uint32) Module1;				 //	lea Module1,a0
	a1 = 0;			// sub.l a1,a1
	a2 = 0;			// sub.l a2,a2
	d0 = 0;			// moveq #0,d0

#ifdef have_protracker
	P61_Init();
#endif
	movem_pop(RD0,RA6);				//	movem.l (sp)+,d0-a6
	Main();
	movem_push(RD0,RA6);				//	movem.l d0-a6,-(sp)

#ifdef have_protracker
	P61_End();
#endif

	movem_pop(RD0,RA6);				//	movem.l (sp)+,d0-a6

									//	rts			;go back to system friendly wrapper exit.
	return;
}

//********** ROUTINES **********
void Main()
{
	movem_push(RD0,RA6);		//	movem.l d0-a6,-(sp)

//**************************

	WaitMouse();

					// WaitMouse:
					//	btst #6,0xbfe001
					//	bne.s WaitMouse

//**************************

	movem_pop(RD0,RA6);		//	movem.l (sp)+,d0-a6
//	rts
}

//********************  Effect 1 subroutine  ********************

#define ld_b(a) *((uint8 *) (a))
#define ld_sb(a) *((int8 *) (a))
#define ld_w(a) *((uint16 *) (a))
#define ld_sw(a) *((int16 *) (a))
#define ld_l(a) *((uint32 *) (a))
#define ld_sl(a) *((int32 *) (a))

#define st_b(a,v) *((uint8 *) (a)) = (uint8) (v)
#define st_w(a,v) *((uint16 *) (a)) = (uint16) (v)
#define st_l(a,v) *((uint32 *) (a)) = (uint32) (v)

//uint8 *CloudCoordsLP;
extern struct cloud *CloudCoordsLP[];

#ifdef have_sprite
uint8 *StarSpr = NULL;
uint8 *StarSpr2 = NULL;
#endif

uint8 *Font = NULL;
uint8 *FontE = NULL;

#define bin8(b7,b6,b5,b4,b3,b2,b1,b0) ((b7<<7) | (b6 <<6) | (b5 <<5) | (b4<<4) | (b3 <<3) | (b2<<2) | (b1<<1) | (b0))

void Part1()
{
	movem_push(RD0,RA6);				//	movem.l d0-a6,-(sp)

 //   *--- clear clouds ---*

	a1= (uint32) SkyBufferL;				// move.l SkyBufferL(PC),a1
	a3 = (uint32) CloudCoordsLP[0]; 		// move.l CloudCoordsLP(PC),a3

	d7 =	cloudcount-1;				// moveq #cloudcount-1,d7

// .clearl:

	for (;d7;d7--)
	{
		a0 = ld_l(a3); a3+=4;		//	move.l (a3)+,a0			;bob
		a2 = ld_l(a3); a3+=4;		//	move.l (a3)+,a2			;mask

		d0 = ld_w(a3); a3+=2;		//	move.w (a3)+,d0			;x coord
		d1 = ld_w(a3); a3+=2;		//	move.w (a3)+,d1			;y coord
		d2 = ld_w(a3); a3+=2;		//	move.w (a3)+,d2			;width
		d3 = ld_w(a3); a3+=2;		//	move.w (a3)+,d3			;height
		
		a3+=2;					//	addq.w #2,a3			;skip speed value.
		d4 = 0x01000000;			//	move.l #0x01000000,d4		;clear blit

		PlotBob();
	}							//	dbf d7,.clearl

// @bouncescroller was here

#ifdef have_sprite

	a1 =(uint32) StarSpr+1;		//	lea StarSpr+1,a1
	d0 = 0x2C;			//	moveq #0x2c,d0
			
//.skyl:	

	for (d7= 26;	;d7;d7--)		//	moveq #26-1,d7
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
	
	for (d7=5;d7;d7--)				//	moveq #5-1,d7
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

	for (d7=26;d7;d7--)			//	moveq #26-1,d7
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
	
	for(d7=5;d7;d7--)
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

#endif

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
		if (d0 == (uint32) 'I')		//	bne.s .noi
			d2=16;			//	moveq #16,d2
							//.noi:
		d0 =	ScrollCtr;			//	move.w ScrollCtr(PC),d0
		d0 +=4;				//	addq.w #4,d0
							//	cmp.w d2,d0
		if (d0>=d2)			//	blo.s .nowrap		// branch on less then
		{
			PlotChar();		//	bsr.w PlotChar			;preserves a0
			d0=0;			//	clr.w d0
		}					//.nowrap:
		else 	ScrollCtr=d0;		//	move.w d0,ScrollCtr

	}						//.skipscroll:


//;@clear clouds was here

	BounceScroller();			//	bsr.w BounceScroller

//    *--- plot clouds ---*

	a1 = *SkyBufferL;					//	move.l SkyBufferL(PC),a1
	a3 = (uint32) CloudCoordsLP[0];		//	movem.l CloudCoordsLP(PC),a3-a4
	a4 = (uint32) CloudCoordsLP[1];
	a4+=8;							//	addq.w #8,a4
	
									//	moveq #cloudcount-1,d7
	for(d7=cloudcount;d7;d7--)			//.drawl:
	{
		a0 = ld_l(a3);	a3+=4;			//	move.l (a3)+,a0			;bob
		a2 = ld_l(a3);	a3+=4;			//	move.l (a3)+,a2			;mask

									//;	move.w (a3)+,d0			;x coord
		a3+=2;						//	addq.w #2,a3

		d0 = ld_w(a4);					//	move.w (a4),d0			;x coord from last frame

		d1 = ld_w(a3);	a3+=2;			//	move.w (a3)+,d1			;y coord
		d2 = ld_w(a3);	a3+=2;			//	move.w (a3)+,d2			;width
		d3 = ld_w(a3);	a3+=2;			//	move.w (a3)+,d3			;height

		d0 += ld_w(a3); a3+=2;			//	add.w (a3)+,d0			;x speed, move cloud
		d5 = 320;						//	move.w #320,d5
		d5 += d2;					//	add.w d2,d5

		if (d0>=320+112)				//	cmp.w #320+112,d0
		{							//	blt.b .nowrapx				// branch less then
			d0=-d5;					//	sub.w d5,d0
		} 							//.nowrapx:
		st_w(a3-10,d0);				//	move.w d0,-10(a3)		;replace coord

		d4 =0x0fca0000;				//	move.l #0x0fca0000,d4		;cookie-cut blit

		PlotBob();						//	bsr.w PlotBob
		a4 += cloudstructsize;			//	lea cloudstructsize(a4),a4
	}								//	dbf d7,.drawl

//    ---  sine lookup for raster bar vert. pos.  ---

	a0 = (uint32) Sine;					//	lea Sine,a0
	d6 = SineCtr;					//	move.w SineCtr,d6
	d7 = 0x4d-6+37;				//	move.w #0x4d-6+37,d7
	d7 += ld_sw(a0 + (d6*2));			//	add.w (a0,d6.w),d7

	d6+=2;						//	addq.w #2,d6
	if (d6>=SineEnd-Sine)			//	cmp.w #SineEnd-Sine,d6
	{							//	blt.s .nowrap2				// branch less then
		d6 = 0;					//	moveq #0,d6
	}							//.nowrap2:
	SineCtr = d6;					//	move.w d6,SineCtr


								//    ---  in front or behind flag  ---

	a2 = (uint32) BarInFront;			//	lea BarInFront,a2	;default source address for RGB color values 
	if ((d6<50*2)					//	cmp.w #50*2,d6
		|| (d6>150*2)) 				//	blt.s .behind			// if (d6<50*2) goto .behind
	{							//	cmp.w #150*2,d6
		a2 = (uint32) BarBehind;		//	bge.s .behind			// if (d6>150*2) goto .behind
	}							//	bra.s .cont
								//.behind:
								//	lea BarBehind,a2
								//.cont:


	a0 = (uint32) waitras1;			//	lea waitras1,a0
	d0 = d7;						//	move d7,d0
						
								//.l:

	for (d1 = 6; d1; d1--)						//	moveq #6-1,d1
	{
		st_b(a0,d0);						//	move.b d0,(a0)
		d0++;							//	addq.w #1,d0

		d2=ld_w(a2);	a2+=2;				//	move.w (a2)+,d2			;background color from list
		st_w(a0+6,d2);						//	move.w d2,6(a0)
										
		st_w(a0+(6+4*1), ld_w(a2)); a2 += 2;	//	move.w (a2)+,6+4*1(a0)
		st_w(a0+(6+4*2), ld_w(a2)); a2 += 2; 	//	move.w (a2)+,6+4*2(a0)

		st_w(a0+(6+4*3), ld_w(a2)); a2 += 2;	//	move.w (a2)+,6+4*3(a0)
		st_w(a0+(6+4*4), ld_w(a2)); a2 += 2;	//	move.w (a2)+,6+4*4(a0)
		st_w(a0+(6+4*5), ld_w(a2)); a2 += 2;	//	move.w (a2)+,6+4*5(a0)
		st_w(a0+(6+4*6), ld_w(a2)); a2 += 2;	//	move.w (a2)+,6+4*6(a0)
		st_w(a0+(6+4*7), ld_w(a2)); a2 += 2;	//	move.w (a2)+,6+4*7(a0)

		st_w(a0+(6+4*8), ld_w(a2)); a2 += 2;	//	move.w (a2)+,6+4*8(a0)		;2nd playfield
		st_w(a0+(6+4*9), ld_w(a2)); a2 += 2;	//	move.w (a2)+,6+4*9(a0)
		st_w(a0+(6+4*10), ld_w(a2)); a2 += 2;	//	move.w (a2)+,6+4*10(a0)
		st_w(a0+(6+4*11), ld_w(a2)); a2 += 2;	//	move.w (a2)+,6+4*11(a0)
		st_w(a0+(6+4*12), ld_w(a2)); a2 += 2;	//	move.w (a2)+,6+4*12(a0)
		st_w(a0+(6+4*13), ld_w(a2)); a2 += 2;	//	move.w (a2)+,6+4*13(a0)
		st_w(a0+(6+4*14), ld_w(a2)); a2 += 2;	//	move.w (a2)+,6+4*14(a0)
			
		a0 += 4*(9+7);					//	add.w #4*(9+7),a0		;step to next.
							//	DBF d1,.l
	}

	movem_pop(RD0,RA6);		//;	movem.l (sp)+,d0-a6
//	rts
}


void VBint()			//					;Blank template VERTB interrupt
{
	// --- will call this on every.

	movem_push(RD0,RA6);		//	movem.l d0-a6,-(sp)		;Save used registers
	a6 = (uint32) custom;				//	lea 0xdff000,a6
							//	btst #5,0x1f(a6)			;INTREQR check if it's our vertb int.
							//	beq.s .notvb

//    *--- double buffering for the clouds ---*

							//	movem.l SkyBufferL(PC),a0-a1
	a0 = SkyBufferL[0];
	a1 = SkyBufferL[1];
							//	exg a0,a1
							//	movem.l a0-a1,SkyBufferL
	SkyBufferL[0] = a1;
	SkyBufferL[1] = a0;
							//	movem.l CloudCoordsLP(PC),a0-a1
	a0 = (uint32) CloudCoordsLP[0];
	a1 = (uint32) CloudCoordsLP[1];
							//	exg a0,a1
							//	movem.l a0-a1,CloudCoordsLP
	CloudCoordsLP[0] = (struct cloud *) a1;
	CloudCoordsLP[1] = (struct cloud *) a0;

//    *--- playfield 2 ptrs ---*
							//	move.l SkyBufferL(PC),a0		;ptr to first bitplane of logo
	a0 = SkyBufferL[0];
							//	lea CopSkyBplP,a1	;where to poke the bitplane pointer words.
	a1 = (uint32) CopSkyBplP;
							//	lea 14(a0),a0
	a0 = 14+a0;
							//	move #3-1,d0
	for(d0 = 3;d0;d0--)
	{
							//.bpll2:
		d1 = a0;				//	move.l a0,d1
							//	swap d1
		st_w(2+a1,D1.hw);		//	move.w d1,2(a1)		;hi word
							//	swap d1
		st_w(6+a1,D1.lw);		//	move.w d1,6(a1)		;lo word

		a1+=8;				//	addq #8,a1		;point to next bpl to poke in copper
		a0 = SkyBpl+a0;		//	lea skybpl(a0),a0
							//	dbf d0,.bpll2
	}

	Part1();					//	bsr.w Part1

							//	IFD Measure
#ifdef Measure
							//	move.w #0x722,0x180(a6)
							//	move.l 4(a6),d0
							//	lsr.l #8,d0
							//	and.w #512-1,d0
							//	cmp.w #0x130,d0
							//	bhs.s .nomax
							//	cmp.w MaxVpos(PC),d0
							//	blo.s .nomax
							//	move.w d0,MaxVpos
							//.nomax:
							//	ENDC
#endif

	d0 = 0x20;				//	moveq #0x20,d0			;poll irq bit
							//	move.w d0,0x9c(a6)		;0xdff09c=INTREQ
							//	move.w d0,0x9c(a6)
							//.notvb:
							//	movem.l (sp)+,d0-a6		;restore
							//	rte
}
	
#define row	(288*3*20/8)
#define col	4

void PlotChar()										//PlotChar:	;a0=scrollptr
{												//;	movem.l d0-a6,-(sp)
	a0 = (uint32) ScrollPtr;									//	move.l ScrollPtr(PC),a0

	a6 = custom;									//	lea 0xdff000,a6

												//	moveq #0,d0
	d0 = ld_b(a0); a0++;							//	move.b (a0)+,d0			;ASCII value
	if (d0<32)										//	cmp.b #32,d0				; branch higher or same.
	{											//	bhs.s .char
												//.commands:
		switch (d0)								//	tst.b d0
		{										//	bne.s .nowraptext
												//	lea ScrollText(PC),a0
												//	bra.s .readnext
												//.nowraptext:
			case 1:								//	cmp.b #1,d0
												//	bne.s .notogglebounce
				Cmd_Bounce=~Cmd_Bounce;			//	not.b Cmd_Bounce
				BounceYspeed = 16;				//	move.w #16,BounceYspeed
				d0 = ld_b(a0);	a0++;				//	bra.s .readnext
				break;							
												//.notogglebounce:
			case	2:								//	cmp.b #2,d0
												//	bne.s .stop
												//	moveq #0,d0
				d0 = ld_b(a0); a0++;				//	move.b (a0)+,d0
				Cmd_StopCount = d0;				//	move.w d0,Cmd_StopCount
				break;							//.readnext:
												//	move.b (a0)+,d0
												//.stop:
			default:
				
				a0 = (uint32) ScrollText;
				d0 = ld_b(a0);	a0++;
				break;

		}
	}											//.char:

	printf("%c\n",d0);

	
	ScrollPtr = (void *) a0;							//	move.l a0,ScrollPtr
	LastChar =  d0;									//	move.b d0,LastChar

	d0 -= 32;										//	sub.w #32,d0
	a0 = (uint32) FontTbl;							//	lea FontTbl(PC),a0
	d0 = ld_b(a0+d0);								//	move.b (a0,d0.w),d0
	d0 /= 9;										//	divu #9,d0			;row
	d1 = D0.hw ;								//	move.l d0,d1
												//	swap d1				;remainder (column)

	d0 *= row;									//	mulu #row,d0
	d1 *= col;										//	mulu #col,d1

	d0 += d1;									//	add.l d1,d0			;offset into font bitmap
	d0 += (uint32) Font;							//	add.l #Font,d0

	WAITBLIT();									//	WAITBLIT

	st_l(a6+BLTCON0, 0x09f00000);					//	move.l #0x09f00000,BLTCON0(a6)
	st_l(a6+BLTAFWM, 0xFFFFFFFF);					//	move.l #0xffffffff,BLTAFWM(a6)
	st_l(a6+BLTAPTH, d0);							//	move.l d0,BLTAPTH(a6)
	st_l(a6+BLTDPTH, Screen+ScrBpl*3*plotY+plotX/8)	;	//	move.l #Screen+ScrBpl*3*plotY+plotX/8,BLTDPTH(a6)
	st_w(a6+BLTAMOD, FontBpl-col);					//	move.w #FontBpl-col,BLTAMOD(a6)
	st_w(a6+BLTDMOD, ScrBpl-col);					//	move.w #ScrBpl-col,BLTDMOD(a6)

	st_w(a6+BLTSIZE, 20*3*64+2);					//	move.w #20*3*64+2,BLTSIZE(a6)

	doBlitter( custom );

	movem_pop(RD0,RA6);							//;	movem.l (sp)+,d0-a6
}												//	rts

/*
;Chan	Function
;A	Cloud Mask
;B	Cloud
;C	Destination playfield
;D	Destination playfield
*/

void PlotBob()										//PlotBob:		;d0-d3/a0-a2=x,y,width,h,src,dest,mask,BLTCON0+1.l
{												//	movem.l d0-d3/d5/a1,-(sp)
	movem_push(RD0,RD3);
	movem_push(RD5,RD5);
	movem_push(RA1,RA1);


	D2.lw+=15;									//	add.w #15,d2			;round up
	D2.lw >>=4;									//	lsr.w #4,d2			;word width

	D5.lw = D0.lw;									//	move.w d0,d5			;x position
	d5 >>= 4;									//	asr.w #4,d5			;in words
	D5.lw += D5.lw;								//	add.w d5,d5			;*2=byte offset into destination

	d1 *= skybwid;									//	muls #skybwid,d1		;y offset
												//	ext.l d5

	d5 = D5.lw | ((D5.lw & 0x8000) ? 0xFFFF0000 : 0x00000000);

	d1+=d5;										//	add.l d5,d1
	a1+=d1;										//	add.l d1,a1			;dest address

	d0 &= 0xf;									//	and.w #0xf,d0			;shift nibble
	d0 = ((d0 & 0xF) << 12) |  (d0 >> 4) ;				//	ror.w #4,d0			;to top nibble

	d5 = d0;										//	move.w d0,d5			;and put in
	d5 = (D5.hw >> 16) | (D5.lw << 16);				//	swap d5				;both words

	D5.lw = D0.lw;									//	move.w d0,d5			;of BLTCON
	
	d5 = d5 | d4;									//	or.l d4,d5
	
	d3 *= 64*3;									//	mulu #3*64,d3			;calculate blit size
	D3.lw += D2.lw;								//	add.w d2,d3

	D2.lw += D2.lw;								//	add.w d2,d2			;w/8
	D2.lw = -D2.lw;									//	neg.w d2
	D2.lw += SkyBpl;								//	add.w #SkyBpl,d2		;=SkyBpl-w/8

	WAITBLIT();									//	WAITBLIT

	st_l(a6+BLTCON0, d5);							//	move.l d5,BLTCON0(a6)
	st_l(a6+BLTAFWM, 0xFFFFFFFF);					//	move.l #0xffffffff,BLTAFWM(a6)
	st_l(a6+BLTAPTH, a2);							//	move.l a2,BLTAPTH(a6)
	st_l(a6+BLTBPTH, a0);							//	move.l a0,BLTBPTH(a6)
	st_l(a6+BLTCPTH, a1);							//	move.l a1,BLTCPTH(a6)
	st_l(a6+BLTDPTH, a1);							//	move.l a1,BLTDPTH(a6)
	st_l(a6+BLTBMOD,0);							//	clr.l BLTBMOD(a6)
	st_w(a6+BLTCMOD, d2);							//	move.w d2,BLTCMOD(a6)
	st_w(a6+BLTDMOD, d2);							//	move.w d2,BLTDMOD(a6)
	st_w(a6+BLTSIZE, d3);							//	move.w d3,BLTSIZE(a6)

	doBlitter( custom );	// activate blitter...
					
	movem_pop(RA1,RA1);							//	movem.l (sp)+,d0-d3/d5/a1
	movem_pop(RD5,RD5);
	movem_pop(RD0,RD3);
}												//	rts


void Scrollit()										//Scrollit:
{			//    ---  scroll!  ---
												//bltoffs	=plotY*ScrBpl*3
												//blth	=20
												//bltw	=w/16
												//bltskip	=0				;modulo
												//brcorner=blth*ScrBpl*3-2

#define bltoffs	(plotY*ScrBpl*3)
#define blth	20
#define bltw	(w/16)
#define bltskip	0
#define brcorner (blth*ScrBpl*3-2)

	//movem_push(RD0,RA6);						//;	movem.l d0-a6,-(sp)

	a6 = custom;									//	lea 0xdff000,a6
	WAITBLIT();									//	WAITBLIT
	st_l(a6+BLTCON0, 0x49f00002);					//	move.l #0x49f00002,BLTCON0(a6)
	st_l(a6+BLTAFWM, 0xFFFFFFFF);					//	move.l #0xffffffff,BLTAFWM(a6)
	st_l(a6+BLTAPTH, Screen+bltoffs+brcorner);			//	move.l #Screen+bltoffs+brcorner,BLTAPTH(a6)
	st_l(a6+BLTDPTH, Screen+bltoffs+brcorner);			//	move.l #Screen+bltoffs+brcorner,BLTDPTH(a6)
	st_w(a6+BLTAMOD, bltskip);						//	move.w #bltskip,BLTAMOD(a6)
	st_w(a6+BLTDMOD, bltskip);						//	move.w #bltskip,BLTDMOD(a6)
	st_w(a6+BLTSIZE, blth*3*64+bltw);					//	move.w #blth*3*64+bltw,BLTSIZE(a6)

	doBlitter( custom );	// activate blitter...

	//movem_pop(RD0,RA6);							//;	movem.l (sp)+,d0-a6
}												//	rts

void Init()											//Init:
{												//	movem.l d0-a6,-(sp)
	printf("%s:%d\n",__FUNCTION__,__LINE__);

	movem_push(RD0,RA6);

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	d1=0;										//	moveq #0,d1
	a1 = (uint32) Screen;							//	lea Screen,a1
	d0 = bplsize*FontBpls/2;						//	move.w #bplsize*FontBpls/2-1,d0
	for (;d0;d0--)									//.l:	
	{
		st_w(a1,0);	a1+=2;						//	move.w #0,(a1)+
		D1.lw += 1;								//	addq.w #1,d1
	}											//	dbf d0,.l

	printf("%s:%d\n",__FUNCTION__,__LINE__);
												//	lea Sky,a1
												//	lea Sky2,a2		;clear 2nd buffer also
												//	lea SkyBufferL(PC),a3
	SkyBufferL[0] = (uint32) Sky;						//	move.l a1,(a3)+
	SkyBufferL[1] = (uint32) Sky2;						//	move.l a2,(a3)+		;Double-buffer list initialized

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	a1 = (uint32) Sky;								//	lea Sky,a1
	a2 = (uint32) Sky2;								//	lea Sky2,a2		;clear 2nd buffer also

	d0=0;										//	moveq #0,d0		;clear buffers
	d7= (SkyE-Sky)/8;								//	move.w #(SkyE-Sky)/8-1,d7

	for (;d7;d7--)
	{
		st_l(a1,d0); a1+=4;							//.l7:	move.l d0,(a1)+
		st_l(a1,d0); a1+=4;							//	move.l d0,(a1)+
		st_l(a2,d0); a2+=4;							//	move.l d0,(a2)+
		st_l(a2,d0); a2+=4;							//	move.l d0,(a2)+
	}											//	dbf d7,.l7

	printf("%s:%d\n",__FUNCTION__,__LINE__);

//    *--- playfield 1 ptrs ---*

	a0 = (uint32) Logo;								//	lea Logo,a0		;ptr to first bitplane of logo
	a1 = (uint32) CopBplP;							//	lea CopBplP,a1		;where to poke the bitplane pointer words.
	for(d0=3;d0;d0--)								//	move #3-1,d0
	{											//.bpll:
		d1 = a0;									//	move.l a0,d1
		d1 =  (D1.lw << 16) | D1.hw;					//	swap d1
		st_w(a1+2,D1.lw);							//	move.w d1,2(a1)		;hi word
		d1 =  (D1.lw << 16) | D1.hw;					//	swap d1
		st_w(a1+6,D1.lw);							//	move.w d1,6(a1)		;lo word

		a1 += 8;									//	addq #8,a1		;point to next bpl to poke in copper
		a0 += logobpl;								//	lea logobpl(a0),a0
	}											//	dbf d0,.bpll

	printf("%s:%d\n",__FUNCTION__,__LINE__);

//    *--- playfield 2 ptrs ---*

	a0 = (uint32) Sky +14;							//	lea Sky+14,a0		;ptr to first bitplane of logo
	a1 = (uint32) CopSkyBplP;						//	lea CopSkyBplP,a1	;where to poke the bitplane pointer words.
	for (d0=3;d0;d0--)								//	move #3-1,d0
	{											//.bpll2:
		d1 = a0;									//	move.l a0,d1
		d1 =  (D1.lw << 16) | D1.hw;					//	swap d1
//		st_w(2+a1,d1);								//	move.w d1,2(a1)		;hi word
		d1 =  (D1.lw << 16) | D1.hw;					//	swap d1
//		st_w(6+a1,d1);								//	move.w d1,6(a1)		;lo word
		a1+=8;									//	addq #8,a1		;point to next bpl to poke in copper
		a0 = SkyBpl + a0;							//	lea skybpl(a0),a0
	}											//	dbf d0,.bpll2

	printf("%s:%d\n",__FUNCTION__,__LINE__);


#ifdef have_sprite

	a1 = (uint32) SprP;								//	lea SprP,a1
	a0 = (uint32) StarSpr;							//	lea StarSpr,a0
	d1 = a0;										//	move.l a0,d1
	for (d0 = 2;d0;d0--);								//	moveq #2-1,d0
	{											//.sprpl:	
		d1 =  (D1.lw << 16) | D1.hw;					//	swap d1
		st_w(a1+2,d1);								//	move.w d1,2(a1)
		d1 =  (D1.lw << 16) | D1.hw;					//	swap d1
		st_w(a1+6,d1);								//	move.w d1,6(a1)
		a1 += 8;									//	addq.w #8,a1
		d1+= (StarSpr2-StarSpr);						//	add.l #(StarSpr2-StarSpr),d1 
	}											//	dbf d0,.sprpl	


	a0 = (uint32) NullSpr;							//	lea NullSpr,a0
	d1 = a0;										//	move.l a0,d1
	for (d0 = 6 - 1;d0;d0--)							//	moveq #6-1,d0
	{											//.sprpl2:
		d1 =  (D1.lw << 16) | D1.hw;					//	swap d1
		st_w(a1+2,d1);								//	move.w d1,2(a1)
		d1 =  (D1.lw << 16) | D1.hw;					//	swap d1
		st_w(a1+6,d1);								//	move.w d1,6(a1)
		a1 += 8;									//	addq.w #8,a1
	}											//	DBF d0,.sprpl2

#endif

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	printf("FontE: %08x\n",FontE);
	printf("FontPalP: %08x\n",FontPalP);

	a0 =	((uint32) FontE)-7*2;							//	lea FontE-7*2,a0
	a1 =	((uint32) FontPalP)+2;						//	lea FontPalP+2,a1

	for(d0=7;d0;d0--)								//	moveq #7-1,d0
	{											//.coll:
		st_w(a1,ld_w(a0)); a0+=2; a1+=2;				//	move.w (a0)+,(a1)+
		a1+=2;									//	addq.w #2,a1
	}											//	DBF d0,.coll

	printf("%s:%d\n",__FUNCTION__,__LINE__);

//    *--- initialize sprites ---*

#ifdef have_sprite

	a1 = (uint32) StarSpr;							//	lea StarSpr,a1
	d0 = 0x2c;									//	moveq #0x2c,d0
	for (d7 = 26;d7;d7--)							//	moveq #26-1,d7
	{											//.skyl:	
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+			;vstart
		st_b(a1,ld_b(a1)+1); a1++;					//	addq.b #1,(a1)+			;add speed to hpos
		d0++;									//	addq.b #1,d0			;increase vstop value
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+			;vstop
		d0++;									//	addq.b #1,d0			;increase vstop value
		a1 += 5;									//	addq.w #5,a1			;skip to next sprite control words

		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		st_b(a1,ld_b(a1)+2); a1++;					//	addq.b #2,(a1)+
		d0++;									//	addq.b #1,d0
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		d0++;									//	addq.b #1,d0			;increase vstop value
		a5+=5;									//	addq.w #5,a1

		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		st_b(a1,ld_b(a1)+1); a1++;					//	addq.b #1,(a1)+
		d0++;									//	addq.b #1,d0
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		d0++;									//	addq.b #1,d0			;increase vstop value
		a5+=5;									//	addq.w #5,a1

		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		st_b(a1,ld_b(a1)+3); a1++;					//	addq.b #3,(a1)+
		d0++;									//	addq.b #1,d0
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		d0++;									//	addq.b #1,d0			;increase vstop value
		a5+=5;									//	addq.w #5,a1

	}											//	dbf d7,.skyl

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	st_b(a1,d0); a1++;								//	move.b d0,(a1)+			;vstart
	st_b(a1,ld_b(a1)+1); a1++;						//	addq.b #1,(a1)+			;add speed to hpos
	D0.b0+=1;									//	addq.b #1,d0			;increase vstop value
	st_b(a1,d0); a1++;								//	move.b d0,(a1)+			;vstop
	D0.b0+=1;									//	addq.b #1,d0			;increase vstop value
	st_w(a1,ld_w(a1)+5);							//	addq.w #5,a1			;skip to next sprite control words

	st_b(a1,d0); a1++;								//	move.b d0,(a1)+
	st_b(a1,ld_b(a1)+1); a1++;						//	addq.b #2,(a1)+
	D0.b0+=1;									//	addq.b #1,d0
	st_b(a1,d0); a1++;								//	move.b d0,(a1)+
	D0.b0+=1;									//	addq.b #1,d0			;increase vstop value
	st_w(a1,ld_w(a1)+5);							//	addq.w #5,a1

	printf("%s:%d\n",__FUNCTION__,__LINE__);

//    *--- below line 0xff ---*

	d6 = bin8(0,0,0,0,0,1,1,0);								//	moveq #%00000110,d6

	st_b(a1,d0); a1++;								//	move.b d0,(a1)+
	st_b(a1,ld_b(a1)+1); a1++;						//	addq.b #1,(a1)+
	D0.b0+=1;									//	addq.b #1,d0
	st_b(a1,d0); a1++;								//	move.b d0,(a1)+
	D0.b0+=1;									//	addq.b #1,d0			;increase vstop value
	st_b(a1,d6); a1++;								//	move.b d6,(a1)+
	a1+=4;										//	addq.w #4,a1			;skip to next sprite control words

	st_b(a1,d0); a1++;								//	move.b d0,(a1)+
	st_b(a1,ld_b(a1)+3); a1++;						//	addq.b #3,(a1)+
	D0.b0+=1;									//	addq.b #1,d0
	st_b(a1,d0); a1++;								//	move.b d0,(a1)+
	D0.b0+=1;									//	addq.b #1,d0			;increase vstop value
	st_b(a1,d6); a1++;								//	move.b d6,(a1)+
	a1+=4;										//	addq.w #4,a1			;skip to next sprite control words

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	for(d7=5;d7;d7--)								//	moveq #5-1,d7
	{											//.floorl:	
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+			;vstart
		st_b(a1,ld_b(a1)+1); a1++;					//	addq.b #1,(a1)+			;add speed to hpos
		D0.b0+=1;								//	addq.b #1,d0			;increase vstop value
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+			;vstop
		D0.b0+=1;								//	addq.b #1,d0			;increase vstop value
		st_b(a1,d6); a1++;							//	move.b d6,(a1)+
		a1+=4;									//	addq.w #4,a1			;skip to next sprite control words

		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		st_b(a1,ld_b(a1)+2); a1++;					//	addq.b #2,(a1)+
		D0.b0+=1;								//	addq.b #1,d0
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		D0.b0+=1;								//	addq.b #1,d0			;increase vstop value
		st_b(a1,d6); a1++;							//	move.b d6,(a1)+
		a1+=4;									//	addq.w #4,a1			;skip to next sprite control words

		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		st_b(a1,ld_b(a1)+1); a1++;					//	addq.b #1,(a1)+
		D0.b0+=1;								//	addq.b #1,d0
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		D0.b0+=1;								//	addq.b #1,d0			;increase vstop value
		st_b(a1,d6); a1++;							//	move.b d6,(a1)+
		a1+=4;									//	addq.w #4,a1			;skip to next sprite control words

		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		st_b(a1,ld_b(a1)+3); a1++;					//	addq.b #3,(a1)+
		D0.b0+=1;								//	addq.b #1,d0
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		D0.b0+=1;								//	addq.b #1,d0			;increase vstop value
		st_b(a1,d6); a1++;							//	move.b d6,(a1)+
		a1+=4;									//	addq.w #4,a1			;skip to next sprite control words

	}											//	dbf d7,.floorl

	printf("%s:%d\n",__FUNCTION__,__LINE__);

//    *--- odd lines sprites ---*

	a1 = (uint32) StarSpr2;							//	lea StarSpr2,a1
	d0 = 0x2d;									//	moveq #0x2d,d0
	for(d7=26;d7;d7--)							//	moveq #26-1,d7
	{											//.skyl2:
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+			;vstart
		st_b(a1,ld_b(a1)+1); a1++;					//	addq.b #1,(a1)+			;add speed to hpos
		D0.b0+=1;								//	addq.b #1,d0			;increase vstop value
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+			;vstop
		D0.b0+=1;								//	addq.b #1,d0			;increase vstop value
		a1+=5;									//	addq.w #5,a1			;skip to next sprite control words

		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		st_b(a1,ld_b(a1)+2); a1++;					//	addq.b #2,(a1)+
		D0.b0+=1;								//	addq.b #1,d0
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		D0.b0+=1;								//	addq.b #1,d0			;increase vstop value
		a1+=5;									//	addq.w #5,a1

		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		st_b(a1,ld_b(a1)+1); a1++;					//	addq.b #1,(a1)+
		D0.b0+=1;								//	addq.b #1,d0
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		D0.b0+=1;								//	addq.b #1,d0			;increase vstop value
		a1+=5;									//	addq.w #5,a1

		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		st_b(a1,ld_b(a1)+3); a1++;					//	addq.b #3,(a1)+
		D0.b0+=1;								//	addq.b #1,d0
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		D0.b0+=1;								//	addq.b #1,d0			;increase vstop value
		a1+=5;									//	addq.w #5,a1

	}											//	dbf d7,.skyl2

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	st_b(a1,d0); a1++;								//	move.b d0,(a1)+			;vstart
	st_b(a1,ld_b(a1)+1); a1++;						//	addq.b #1,(a1)+			;add speed to hpos
	D0.b0++;										//	addq.b #1,d0			;increase vstop value
												//	move.b d0,(a1)+			;vstop
	D0.b0++;										//	addq.b #1,d0			;increase vstop value
	a1+=5;										//	addq.w #5,a1			;skip to next sprite control words

	st_b(a1,d0); a1++;								//	move.b d0,(a1)+
	st_b(a1,ld_b(a1)+2); a1++;						//	addq.b #2,(a1)+
	D0.b0++;										//	addq.b #1,d0
												//	move.b d0,(a1)+
	D0.b0++;										//	addq.b #1,d0			;increase vstop value
												//	move.b #2,(a1)+
	a1+=4;										//	addq.w #4,a1			;skip to next sprite control words

//    *--- below line 0xff ---*

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	d6 =	bin8(0,0,0,0,0,1,1,0);								//	moveq #%00000110,d6

	st_b(a1,d0); a1++;								//	move.b d0,(a1)+
	st_b(a1,ld_b(a1)+1); a1++;						//	addq.b #1,(a1)+
	D0.b0++;										//	addq.b #1,d0
	st_b(a1,d0); a1++;								//	move.b d0,(a1)+
	D0.b0++;										//	addq.b #1,d0			;increase vstop value
	st_b(a1,d6); a1++;								//	move.b d6,(a1)+
	a1+=4;										//	addq.w #4,a1			;skip to next sprite control words

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	st_b(a1,d0); a1++;								//	move.b d0,(a1)+
	st_b(a1,ld_b(a1)+3); a1++;						//	addq.b #3,(a1)+
	D0.b0++;										//	addq.b #1,d0
	st_b(a1,d0); a1++;								//	move.b d0,(a1)+
	D0.b0++;										//	addq.b #1,d0			;increase vstop value
	st_b(a1,d6); a1++;								//	move.b d6,(a1)+
	a1+=4;										//	addq.w #4,a1			;skip to next sprite control words

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	for(d7=5;d7;d7--)								//	moveq #5-1,d7
	{											//.floorl2:
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+			;vstart
		st_b(a1,ld_b(a1)+1); a1++;					//	addq.b #1,(a1)+			;add speed to hpos
		D0.b0++;									//	addq.b #1,d0			;increase vstop value
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+			;vstop
		D0.b0++;									//	addq.b #1,d0			;increase vstop value
		st_b(a1,d6); a1++;							//	move.b d6,(a1)+
		a1+=4;									//	addq.w #4,a1			;skip to next sprite control words

		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		st_b(a1,ld_b(a1)+2); a1++;					//	addq.b #2,(a1)+
		D0.b0++;									//	addq.b #1,d0
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		D0.b0++;									//	addq.b #1,d0			;increase vstop value
		st_b(a1,d6); a1++;							//	move.b d6,(a1)+
		a1+=4;									//	addq.w #4,a1			;skip to next sprite control words

		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		st_b(a1,ld_b(a1)+1); a1++;					//	addq.b #1,(a1)+
		D0.b0++;									//	addq.b #1,d0
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		D0.b0++;									//	addq.b #1,d0			;increase vstop value
		st_b(a1,d6); a1++;							//	move.b d6,(a1)+
		a1+=4;									//	addq.w #4,a1			;skip to next sprite control words

		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		st_b(a1,ld_b(a1)+3); a1++;					//	addq.b #3,(a1)+
		D0.b0++;									//	addq.b #1,d0
		st_b(a1,d0); a1++;							//	move.b d0,(a1)+
		D0.b0++;									//	addq.b #1,d0			;increase vstop value
		st_b(a1,d6); a1++;							//	move.b d6,(a1)+
		a1+=4;									//	addq.w #4,a1			;skip to next sprite control words

	}											//	dbf d7,.floorl2

#endif

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	movem_pop(RD0,RA6);							//	movem.l (sp)+,d0-a6
}												//	rts

void CopyB()										//CopyB:	;d0,a0,a1=count,source,destination
{										
	do {
		st_b(a1,ld_b(a0)); a0++; a1++;				//.l:	move.b (a0)+,(a1)+
		d0--;										//	subq.l #1,d0
	} while(d0);									//	bne.s .l
}												//	rts

/*
BlitWait:
	tst DMACONR(a6)			;for compatibility
.waitblit:
	btst #6,DMACONR(a6)
	bne.s .waitblit
	rts
*/

void 	BounceScroller()								//BounceScroller:
{												//	MOVEM.L D0-D1/A0-A1,-(SP)
	movem_push(RD0,RD1);
	movem_push(RA0,RA1);

	a0 = (uint32) Screen;							//	lea Screen,a0		;ptr to first bitplane of font
	d0 = BounceY;									//	move.w BounceY(PC),d0
	d1 = BounceYaccel;								//	move.w BounceYaccel(PC),d1
	BounceYspeed+=d1;							//	add.w d1,BounceYspeed
	d0 += BounceYspeed;							//	add.w BounceYspeed(PC),d0
												//	bpl.s .nobounce
	BounceYspeed = 32;							//	move.w #32,BounceYspeed
	d0 = 0;										//	clr.w d0
												//.nobounce:

	if (Cmd_Bounce)								//	tst.b Cmd_Bounce
	{											//	bne.s .bounce2
		d0 = 1*8;									//	moveq #1*8,d0
	}											//.bounce2:
	d0 = BounceY;									//	move.w d0,BounceY

	d0 >>= 3;									//	lsr.w #3,d0

	d0 *= 3*ScrBpl;								//	mulu #3*scrbpl,d0
	a0 += d0;									//	add.l d0,a0

	a1 = (uint32) ScrBplP;							//	lea ScrBplP,a1		;where to poke the bitplane pointer words.
	for (d0 = FontBpls;d0;d0--)						//	moveq #FontBpls-1,d0
	{											//.bpll2:	move.l a0,d1
		d1 = a0;
												//	swap d1
		st_w(a1+2,D1.hw );							//	move.w d1,2(a1)		;hi word
												//	swap d1
		st_w(a1+6,D1.lw );							//	move.w d1,6(a1)		;lo word

		a1+=8;									//	addq #8,a1		;point to next bpl to poke in copper
		a0 = ScrBpl + a0;							//	lea ScrBpl(a0),a0
	}											//	dbf d0,.bpll2

												//	MOVEM.L (SP)+,D0-D1/A0-A1
	movem_pop(RA0,RA1);
	movem_pop(RD0,RD1);
}												//	RTS

												//	even

//********** DATA **********
uint16	BarBehind[] =
	{ 0x558	//;color00 value
	, logocolors
	, 0x558	//;color00 value
	, 0x558	//;color00 value
	, 0x558	//;color00 value
	, 0x558	//;color00 value
	, 0x558	//;color00 value
	, 0x558	//;color00 value
	, 0x558	//;color00 value

	, 0x99d	//;color00...
	, logocolors
	, 0x99d	//;color00...
	, 0x99d	//;color00...
	, 0x99d	//;color00...
	, 0x99d	//;color00...
	, 0x99d	//;color00...
	, 0x99d	//;color00...
	, 0x99d	//;color00...

	, 0xfff
	, logocolors
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff

	, 0x99d
	, logocolors
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d

	, 0x558
	, logocolors
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558

	, logobgcol			//;restore
	, logocolors
	, cloudcolors
};

uint16	BarInFront[] =
	{ 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558

	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d

	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff
	, 0xfff

	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d
	, 0x99d

	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, 0x558
	, logobgcol
	, logocolors
	, cloudcolors
 };

char FontTbl[]={
	 43,38
	,0,0,0,0,0							// dcb.b 5,0
	, 42
	,0,0,0,0							// dcb.b 4,0
	, 37,40,36,41
	, 26,27,28,29,30,31,32,33,34,35
	,0,0,0,0,0							// dcb.b 5,0
	, 39,0
	, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21
	, 22,23,24,25
	};


extern struct cloud CloudCoordsL[];
extern struct cloud CloudCoordsL2[];

struct cloud *CloudCoordsLP[]=			//CloudCoordsLP:
	{
		CloudCoordsL,			//	dc.l CloudCoordsL
		CloudCoordsL2			//	dc.l CloudCoordsL2
	};





struct cloud CloudCoordsL[10]; 							//CloudCoordsL:
struct cloud CloudCoordsL2[10];

void init_cloud() 
{
	CloudSet(CloudCoordsL+0,		
			Cloud3,				//	dc.l Cloud3
			Cloud3Mask,			//	dc.l Cloud3Mask
			90,					//	dc.w 90			;x
			170,					//	dc.w 170		;y
			48,					//	dc.w 48			;width
			15,					//	dc.w 15			;height
			1					//	dc.w 1			;x speed
		);

	CloudSet(CloudCoordsL+1,
			Cloud3,				//	dc.l Cloud3
			Cloud3Mask,			//	dc.l Cloud3Mask
			284,					//	dc.w 284		;x
			199,					//	dc.w 199		;y
			48,					//	dc.w 48			;width
			15,					//	dc.w 15			;height
			1					//	dc.w 1			;x speed
		);

//;	dc.l Cloud3
//;	dc.l Cloud3Mask
//;	dc.w 184		;x
//;	dc.w 135		;y
//;	dc.w 48			;width
//;	dc.w 15			;height
//;	dc.w 1			;x speed


	CloudSet(CloudCoordsL+2,
			Cloud3,				//	dc.l Cloud3
			Cloud3Mask,			//	dc.l Cloud3Mask
			218,					//	dc.w 218		;x
			95,					//	dc.w 95			;y
			48,					//	dc.w 48			;width
			15,					//	dc.w 15			;height
			2					//	dc.w 2			;x speed
		);

	CloudSet(CloudCoordsL+3,
			Cloud2,				//	dc.l Cloud2
			Cloud2Mask,			//	dc.l Cloud2Mask
			320,					//	dc.w 320		;x
			183,					//	dc.w 183		;y
			64,					//	dc.w 64			;width
			24,					//	dc.w 24			;height
			2					//	dc.w 2			;x speed
		);

	CloudSet(CloudCoordsL+4,
			Cloud2,				//	dc.l Cloud2
			Cloud2Mask,			//	dc.l Cloud2Mask
			123,					//	dc.w 123		;x
			123,					//	dc.w 123		;y
			64,					//	dc.w 64			;width
			24,					//	dc.w 24			;height
			2					//	dc.w 2			;x speed
		);

	CloudSet(CloudCoordsL+5,
			Cloud2,				//	dc.l Cloud2
			Cloud2Mask,			//	dc.l Cloud2Mask
			220,					//	dc.w 210		;x
			150,					//	dc.w 150		;y
			64,					//	dc.w 64			;width
			24,					//	dc.w 24			;height
			2					//	dc.w 2			;x speed
		);

	CloudSet(CloudCoordsL+6,
			Cloud2,				//	dc.l Cloud2
			Cloud2Mask,			//	dc.l Cloud2Mask
			156,					//	dc.w 156		;x
			007,					//	dc.w 007		;y
			64,					//	dc.w 64			;width
			24,					//	dc.w 24			;height
			3					//	dc.w 3			;x speed
		);

	CloudSet(CloudCoordsL+7,
			Cloud,				//	dc.l Cloud
			CloudMask,			//	dc.l CloudMask
			240,					//	dc.w 240		;x
			105,					//	dc.w 105		;y
			112,					//	dc.w 112		;width
			38,					//	dc.w 38			;height
			3					//	dc.w 3			;x speed
		);

	CloudSet(CloudCoordsL+8,
			Cloud,				//	dc.l Cloud
			CloudMask,			//	dc.l CloudMask
			290,					//	dc.w 290		;x
			47,					//	dc.w 47			;y
			112,					//	dc.w 112		;width
			38,					//	dc.w 38			;height
			4					//	dc.w 4			;x speed
		);

	CloudSet(CloudCoordsL+9,
			Cloud,				//	dc.l Cloud
			CloudMask,			//	dc.l CloudMask
			0,					//	dc.w 0			;x
			27,					//	dc.w 27			;y
			112,					//	dc.w 112		;width
			38,					//	dc.w 38			;height
			5					//	dc.w 5			;x speed
		);

//	-------------------------------------------------------------------------------------------------------

								//CloudCoordsLE:
	CloudSet(CloudCoordsL2+0,
			Cloud3,				//	dc.l Cloud3
			Cloud3Mask,			//	dc.l Cloud3Mask
			90,					//	dc.w 90			;x
			170,					//	dc.w 170		;y
			47,					//	dc.w 48			;width
			15,					//	dc.w 15			;height
			1					//	dc.w 1			;x speed
		);

	CloudSet(CloudCoordsL2+1,
			Cloud3,				//	dc.l Cloud3
			Cloud3Mask,			//	dc.l Cloud3Mask
			284,					//	dc.w 284		;x
			199,					//	dc.w 199		;y
			48,					//	dc.w 48			;width
			15,					//	dc.w 15			;height
			1					//	dc.w 1			;x speed
		);


//;	dc.l Cloud3
//;	dc.l Cloud3Mask
//;	dc.w 184		;x
//;	dc.w 135		;y
//;	dc.w 48			;width
//;	dc.w 15			;height
//;	dc.w 1			;x speed

	CloudSet(CloudCoordsL2+2,
			Cloud3,				//	dc.l Cloud3
			Cloud3Mask,			//	dc.l Cloud3Mask
			218,					//	dc.w 218		;x
			94,					//	dc.w 95			;y
			48,					//	dc.w 48			;width
			15,					//	dc.w 15			;height
			2					//	dc.w 2			;x speed
		);

	CloudSet(CloudCoordsL2+3,
			Cloud2,				//	dc.l Cloud2
			Cloud2Mask,			//	dc.l Cloud2Mask
			320,					//	dc.w 320		;x
			183,					//	dc.w 183		;y
			64,					//	dc.w 64			;width
			24,					//	dc.w 24			;height
			2					//	dc.w 2			;x speed
		);

	CloudSet(CloudCoordsL2+4,
			Cloud2,				//	dc.l Cloud2
			Cloud2Mask,				//	dc.l Cloud2Mask
			123,					//	dc.w 123		;x
			123,					//	dc.w 123		;y
			64,					//	dc.w 64			;width
			24,					//	dc.w 24			;height
			2					//	dc.w 2			;x speed
		);

	CloudSet(CloudCoordsL2+5,
			Cloud2,				//	dc.l Cloud2
			Cloud2Mask,			//	dc.l Cloud2Mask
			210,					//	dc.w 210		;x
			150,					//	dc.w 150		;y
			64,					//	dc.w 64			;width
			24,					//	dc.w 24			;height
			2					//	dc.w 2			;x speed
		);

	CloudSet(CloudCoordsL2+6,
			Cloud2,				//	dc.l Cloud2
			Cloud2Mask,			//	dc.l Cloud2Mask
			156,					//	dc.w 156		;x
			007,					//	dc.w 007		;y
			64,					//	dc.w 64			;width
			24,					//	dc.w 24			;height
			3					//	dc.w 3			;x speed
		);

	CloudSet(CloudCoordsL2+7,
			Cloud,				//	dc.l Cloud
			CloudMask,			//	dc.l CloudMask
			240,					//	dc.w 240		;x
			105,					//	dc.w 105		;y
			112,					//	dc.w 112		;width
			38,					//	dc.w 38			;height
			3					//	dc.w 3			;x speed
		);

	CloudSet(CloudCoordsL2+8,
			Cloud,				//	dc.l Cloud
			CloudMask,			//	dc.l CloudMask
			290,					//	dc.w 290		;x
			47,					//	dc.w 47			;y
			112,					//	dc.w 112		;width
			38,					//	dc.w 38			;height
			4					//	dc.w 4			;x speed
		);

	CloudSet(CloudCoordsL2+9,
			Cloud,				//	dc.l Cloud
			CloudMask,			//	dc.l CloudMask
			0,					//	dc.w 0			;x
			27,					//	dc.w 27			;y
			112,					//	dc.w 112		;width
			38,					//	dc.w 38			;height
			5					//	dc.w 5			;x speed
		);

//	CloudCoordsL2E = CloudCoordsL2+8;
}

								//gfxname:
const char *gfxname = "graphics.library";	//	dc.b "graphics.library",0
								//	EVEN


//	IFD Measure
//	MaxVpos:dc.w 0
//	ENDC

//	SECTION TutData,DATA_C


#ifdef have_sprite
							//StarSpr:
							//.x:	SET 1
							//	REPT 32
							//.tmpx:	SET ((.x*0x751+0xdeadbeef)/(.x&0x55))&0xff
							//	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
							//	dc.w 0x8000,0x0000
							//.x:	SET (.x+2)&0xffff
							//.tmpx:	SET ((.x*0x753+0xeadbeefd)/(.x&0x55))&0xff
							//	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
							//	dc.w 0x0000,0x8000
							//.x:	SET (.x+4)&0xffff
							//.tmpx:	SET ((.x*0x755+0xadbeefde)/(.x&0x55))&0xff
							//	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
							//	dc.w 0x8000,0x0000
							//.x:	SET (.x+8)&0xffff
							//.tmpx:	SET ((.x*0x757+0xdbeefdea)/(.x&0x55))&0xff
							//	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
							//	dc.w 0xc000,0xc000
							//.x:	SET (.x+16)&0xffff
							//	ENDR
							//	dc.w 0,0

							//StarSpr2:
							//.x:	SET 0x77
							//	REPT 32
							//.tmpx:	SET ((.x*0x751+0xdeadbeef)/(.x&0x55))&0xff
							//	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
							//	dc.w 0x8000,0x0000
							//.x:	SET (.x+2)&0xffff
							//.tmpx:	SET ((.x*0x753+0xeadbeefd)/(.x&0x55))&0xff
							//	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
							//	dc.w 0x0000,0x8000
							//.x:	SET (.x+4)&0xffff
							//.tmpx:	SET ((.x*0x755+0xadbeefde)/(.x&0x55))&0xff
							//	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
							//	dc.w 0x8000,0x0000
							//.x:	SET (.x+8)&0xffff
							//.tmpx:	SET ((.x*0x757+0xdbeefdea)/(.x&0x55))&0xff
							//	dc.w 0x2c00+.tmpx,0x2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
							//	dc.w 0xc000,0xc000
							//.x:	SET (.x+16)&0xffff
							//	ENDR
							//	dc.w 0,0
#endif


#define cop_w( a,b ) *cop_ptr++=(a); *cop_ptr++=(b); 

void init_copper()
{
	uint16 *cop_ptr;

Copper = malloc(  700 * 2 );

cop_ptr = Copper;

	cop_w (0x1fc,0);		//			;slow fetch mode, AGA compatibility
	cop_w (0x100,0x0200);
	cop_w (0x08e,0x2c81);	// DIWSTART
	cop_w (0x090,0x2cc1);	// DIWSTOP

	cop_w (0x092,0x38);	// DDFSTART
	cop_w (0x094,0xd0);	// DDFSTOP

	cop_w (0x108,logobwid-logobpl);	// BPL1MOD
	cop_w (0x10a,skybwid-320/8);		// BPL2MOD

	cop_w (0x102,0);			// BPLCON1
	cop_w (0x104,0x20);		// BPLCON2

	cop_w (0x1a2,0x99b);	//			;sprite colors
	cop_w (0x1a4,0xcce);
	cop_w (0x1a6,0xfff);

SprP = cop_ptr;

	cop_w (0x120,0);
	cop_w (0x122,0);
	cop_w (0x124,0);
	cop_w (0x126,0);
	cop_w (0x128,0);
	cop_w (0x12a,0);
	cop_w (0x12c,0);
	cop_w (0x12e,0);
	cop_w (0x130,0);
	cop_w (0x132,0);
	cop_w (0x134,0);
	cop_w (0x136,0);
	cop_w (0x138,0);
	cop_w (0x13a,0);
	cop_w (0x13c,0);
	cop_w (0x13e,0);

CopBplP = cop_ptr;

	cop_w (0xe0,0);	//		;playfield 1
	cop_w (0xe2,0);
	cop_w (0xe8,0);
	cop_w (0xea,0);
	cop_w (0xf0,0);
	cop_w (0xf2,0);

CopSkyBplP = cop_ptr;

	cop_w (0xe4,0);	//		;playfield 2
	cop_w (0xe6,0);
	cop_w (0xec,0);
	cop_w (0xee,0);
	cop_w (0xf4,0);
	cop_w (0xf6,0);
		
	cop_w (0x0180,logobgcol);
	cop_w (0x100,0x6600);
	cop_w (0x2c07,0xfffe);

LogoPal = cop_ptr;

	cop_w (0x0180,0x044f);
	cop_w (0x0182,0x068e);
	cop_w (0x0184,0x0adf);
	cop_w (0x0186,0x0dff);
	cop_w (0x0188,0x09bf);
	cop_w (0x018a,0x056d);
	cop_w (0x018c,0x044b);
	cop_w (0x018e,0x033a);

CloudPal = cop_ptr;

	cop_w (0x0192,0x066f);
	cop_w (0x0194,0x077f);
	cop_w (0x0196,0x088e);
	cop_w (0x0198,0x0aae);
	cop_w (0x019a,0x0bbe);
	cop_w (0x019c,0x0dde);
	cop_w (0x019e,0x0eee);

waitras1 = cop_ptr;

	cop_w (0x8007,0xfffe);
	cop_w (0x180,0x558);
	cop_w (0x182,0);
	cop_w (0x184,0);
	cop_w (0x186,0);
	cop_w (0x188,0);
	cop_w (0x18a,0);
	cop_w (0x18c,0);
	cop_w (0x18e,0);
	cop_w (0x192,0);
	cop_w (0x194,0);
	cop_w (0x196,0);
	cop_w (0x198,0);
	cop_w (0x19a,0);
	cop_w (0x19c,0);
	cop_w (0x19e,0);

waitras2 = cop_ptr;

	cop_w (0x8107,0xfffe);
	cop_w (0x180,0x99d);
	cop_w (0x182,0);
	cop_w (0x184,0);
	cop_w (0x186,0);
	cop_w (0x188,0);
	cop_w (0x18a,0);
	cop_w (0x18c,0);
	cop_w (0x18e,0);
	cop_w (0x192,0);
	cop_w (0x194,0);
	cop_w (0x196,0);
	cop_w (0x198,0);
	cop_w (0x19a,0);
	cop_w (0x19c,0);
	cop_w (0x19e,0);

waitras3 = cop_ptr;

	cop_w (0x8207,0xfffe);
	cop_w (0x180,0xfff);
	cop_w (0x182,0);
	cop_w (0x184,0);
	cop_w (0x186,0);
	cop_w (0x188,0);
	cop_w (0x18a,0);
	cop_w (0x18c,0);
	cop_w (0x18e,0);
	cop_w (0x192,0);
	cop_w (0x194,0);
	cop_w (0x196,0);
	cop_w (0x198,0);
	cop_w (0x19a,0);
	cop_w (0x19c,0);
	cop_w (0x19e,0);

waitras4 = cop_ptr;

	cop_w (0x8307,0xfffe);
	cop_w (0x180,0x99d);
	cop_w (0x182,0);
	cop_w (0x184,0);
	cop_w (0x186,0);
	cop_w (0x188,0);
	cop_w (0x18a,0);
	cop_w (0x18c,0);
	cop_w (0x18e,0);
	cop_w (0x192,0);
	cop_w (0x194,0);
	cop_w (0x196,0);
	cop_w (0x198,0);
	cop_w (0x19a,0);
	cop_w (0x19c,0);
	cop_w (0x19e,0);

waitras5 = cop_ptr;

	cop_w (0x8407,0xfffe);
	cop_w (0x180,0x558);
	cop_w (0x182,0);
	cop_w (0x184,0);
	cop_w (0x186,0);
	cop_w (0x188,0);
	cop_w (0x18a,0);
	cop_w (0x18c,0);
	cop_w (0x18e,0);
	cop_w (0x192,0);
	cop_w (0x194,0);
	cop_w (0x196,0);
	cop_w (0x198,0);
	cop_w (0x19a,0);
	cop_w (0x19c,0);
	cop_w (0x19e,0);

waitras6 = cop_ptr;

	cop_w (0x8507,0xfffe);
	cop_w (0x180,logobgcol);
	cop_w (0x182,0);
	cop_w (0x184,0);
	cop_w (0x186,0);
	cop_w (0x188,0);
	cop_w (0x18a,0);
	cop_w (0x18c,0);
	cop_w (0x18e,0);
	cop_w (0x192,0);
	cop_w (0x194,0);
	cop_w (0x196,0);
	cop_w (0x198,0);
	cop_w (0x19a,0);
	cop_w (0x19c,0);
	cop_w (0x19e,0);

	cop_w (0x96bf,0xfffe);

ScrBplP= cop_ptr;

	cop_w (0xe0,0);
	cop_w (0xe2,0);
	cop_w (0xe8,0);
	cop_w (0xea,0);
	cop_w (0xf0,0);
	cop_w (0xf2,0);
	cop_w (0x108,ScrBpl*FontBpls-320/8);
	cop_w (0x92,0x38);
	cop_w (0x94,0xd0);

	cop_w (0x9707,0xfffe);
	cop_w (0x180,0x44e);
	cop_w (0x9807,0xfffe);
	cop_w (0x180,0x44f);
	cop_w (0x9907,0xfffe);
	cop_w (0x180,0x44e);

	cop_w (0x9d07,0xfffe);
	cop_w (0x180,0x44d);
	cop_w (0x9e07,0xfffe);
	cop_w (0x180,0x44e);
	cop_w (0x9f07,0xfffe);
	cop_w (0x180,0x44d);

	cop_w (0xa407,0xfffe);
	cop_w (0x180,0x44c);
	cop_w (0xa507,0xfffe);
	cop_w (0x180,0x44d);
	cop_w (0xa607,0xfffe);
	cop_w (0x180,0x44c);

	cop_w (0xab07,0xfffe);
	cop_w (0x180,0x43c);
	cop_w (0xac07,0xfffe);
	cop_w (0x180,0x44c);
	cop_w (0xad07,0xfffe);
	cop_w (0x180,0x43c);

	cop_w (0xb207,0xfffe);
	cop_w (0x180,0x33b);
	cop_w (0xb307,0xfffe);
	cop_w (0x180,0x43c);
	cop_w (0xb407,0xfffe);
	cop_w (0x180,0x33b);

	cop_w (0xb907,0xfffe);
	cop_w (0x180,0x33a);
	cop_w (0xba07,0xfffe);
	cop_w (0x180,0x33b);
	cop_w (0xbb07,0xfffe);
	cop_w (0x180,0x33a);

	cop_w (0xc007,0xfffe);
	cop_w (0x180,0x339);
	cop_w (0xc107,0xfffe);
	cop_w (0x180,0x33a);
	cop_w (0xc207,0xfffe);
	cop_w (0x180,0x339);

//;tone down clouds 1 notch

	cop_w (0x0192,0x044b);
	cop_w (0x0194,0x055b);
	cop_w (0x0196,0x066a);
	cop_w (0x0198,0x088a);
	cop_w (0x019a,0x099a);
	cop_w (0x019c,0x0bbb);
	cop_w (0x019e,0x0ccc);

	cop_w (0xc707,0xfffe);
	cop_w (0x180,0x329);
	cop_w (0xc807,0xfffe);
	cop_w (0x180,0x339);
	cop_w (0xc907,0xfffe);
	cop_w (0x180,0x329);

	cop_w (0xce07,0xfffe);
	cop_w (0x180,0x228);
	cop_w (0xcf07,0xfffe);
	cop_w (0x180,0x329);
	cop_w (0xd007,0xfffe);
	cop_w (0x180,0x228);

	cop_w (0xd507,0xfffe);
	cop_w (0x180,0x227);
	cop_w (0xd607,0xfffe);
	cop_w (0x180,0x228);
	cop_w (0xd707,0xfffe);
	cop_w (0x180,0x227);

	cop_w (0xdc07,0xfffe);
	cop_w (0x180,0x226);
	cop_w (0xdd07,0xfffe);
	cop_w (0x180,0x227);
	cop_w (0xde07,0xfffe);
	cop_w (0x180,0x226);

	cop_w (0xe307,0xfffe);
	cop_w (0x180,bgcol);
	cop_w (0xe407,0xfffe);
	cop_w (0x180,0x226);
	cop_w (0xe507,0xfffe);
	cop_w (0x180,bgcol);

	cop_w (0xffdf,0xfffe);

//    ---  bottom plate start  ---

	cop_w (0x07df,0xfffe);
	cop_w (0x10a,-(skybwid+320/8));
	cop_w (0x104,0x20);

	cop_w (0x0192,0x0248);
	cop_w (0x0194,0x0348);
	cop_w (0x0196,0x0458);
	cop_w (0x0198,0x0668);
	cop_w (0x019a,0x0778);
	cop_w (0x019c,0x09aa);
	cop_w (0x019e,0x0abb);

	cop_w (0x0807,0xfffe);
	cop_w (0x180,0x236);
	cop_w (0x0a07,0xfffe);
	cop_w (0x180,0x247);
	cop_w (0x0b07,0xfffe);
	cop_w (0x180,0x236);
	cop_w (0x0e07,0xfffe);
	cop_w (0x180,0x258);

	cop_w (0x0f07,0xfffe);
	cop_w (0x180,0x236);

	cop_w (0x1507,0xfffe);
	cop_w (0x180,0x269);

	cop_w (0x192,0x269);
	cop_w (0x194,0x269);
	cop_w (0x196,0x269);
	cop_w (0x198,0x269);
	cop_w (0x19a,0x269);
	cop_w (0x19c,0x269);
	cop_w (0x19e,0x269);

	cop_w (0x1607,0xfffe);
	cop_w (0x180,0x236);

	cop_w (0x0192,0x0248);
	cop_w (0x0194,0x0348);
	cop_w (0x0196,0x0458);
	cop_w (0x0198,0x0668);
	cop_w (0x019a,0x0778);
	cop_w (0x019c,0x09aa);
	cop_w (0x019e,0x0abb);

//    ---  mirror split  ---

	cop_w (0x17df,0xfffe);
	cop_w (0x182,0x0468);
	cop_w (0x184,0x0235);
	cop_w (0x186,0x0358);
	cop_w (0x188,0x0689);
	cop_w (0x18a,0x069a);
	cop_w (0x18c,0x07bc);
	cop_w (0x18e,0x08dd);
	cop_w (0x108,(ScrBpl*FontBpls-320/8)-(ScrBpl*FontBpls*2));

	cop_w (0x2007,0xfffe);
	cop_w (0x180,0x27a);
	cop_w (0x182,0x27a);
	cop_w (0x184,0x27a);
	cop_w (0x186,0x27a);
	cop_w (0x188,0x27a);
	cop_w (0x18a,0x27a);
	cop_w (0x18c,0x27a);
	cop_w (0x18e,0x27a);

	cop_w (0x192,0x27a);
	cop_w (0x194,0x27a);
	cop_w (0x196,0x27a);
	cop_w (0x198,0x27a);
	cop_w (0x19a,0x27a);
	cop_w (0x19c,0x27a);
	cop_w (0x19e,0x27a);

	cop_w (0x2107,0xfffe);
	cop_w (0x180,0x236);
	cop_w (0x182,0x0468);
	cop_w (0x184,0x0235);
	cop_w (0x186,0x0358);
	cop_w (0x188,0x0689);
	cop_w (0x18a,0x069a);
	cop_w (0x18c,0x07bc);
	cop_w (0x18e,0x08dd);

	cop_w (0x0192,0x0248);
	cop_w (0x0194,0x0348);
	cop_w (0x0196,0x0458);
	cop_w (0x0198,0x0668);
	cop_w (0x019a,0x0778);
	cop_w (0x019c,0x09aa);
	cop_w (0x019e,0x0abb);

//    ---  bottom plate stop  ---

	cop_w (0x2c07,0xfffe);
	cop_w (0x180,0x38b);
	cop_w (0x2d07,0xfffe);
	cop_w (0x180,0x235);
	cop_w (0x2e07,0xfffe);
	cop_w (0x180,0x247);
	cop_w (0x2f07,0xfffe);
	cop_w (0x180,0x258);

//    ---  bottom plate thickness  ---

	cop_w (0x3007,0xfffe);
	cop_w (0x180,bgcol);

	cop_w (0xffff,0xfffe);

	CopperE = cop_ptr;

	printf("copper size is: %d\n", CopperE - Copper);
}

//********** PLAYROUTINE CODE **********
//;Note: if this is put in its own section (or compiled as separate binary), then
//;jsr <addr>+P61_InitOffset,P61_MusicOffset,P61_EndOffset,P61_SetPositionOffset
//;to call the routines.

//Playrtn:
//	include "P6112/P6112-Play.i"


bool load_raw_files()
{
	uint32 screenSize = bplsize*FontBpls;

	if( ! load_raw("media/FastCarFont.284x100x3",0,(void **) &Font,(void **) &FontE)) return false;
	if (! load_raw("media/Cloud.112x38x3.raw",0,(void **) &Cloud, (void **) &CloudE)) return false;
	if( ! load_raw("media/Cloud.64x24x3.raw",0,(void **) &Cloud2,(void **) &Cloud2E)) return false;
	if( ! load_raw("media/Cloud.48x15x3.raw",0,(void **) &Cloud3, (void **) &Cloud3E)) return false;
	if( ! load_raw("media/Cloud.112x38x3.masks.raw",0, (void **) &CloudMask, (void **) &CloudMaskE)) return false;
	if( ! load_raw("media/Cloud.64x24x3.masks.raw", 0, (void **) &Cloud2Mask, (void **) &Cloud2MaskE)) return false;
	if( ! load_raw("media/Cloud.48x15x3.masks.raw", 0, (void **) &Cloud3Mask, (void **) &Cloud3MaskE)) return false;
//	if( ! load_raw("P61.new_ditty", 0, (void **) &Module1, (void **) &Module1E)) return false; // usecode 0xc00b43b

	if( ! load_raw("sky3centered.raw", (logobwid*6) + screenSize, (void **) &Logo, (void **) &LogoE)) return false;

	LogoE -= screenSize;

	printf("Logo: %08x to %08x\n", Logo,LogoE);

	Screen = LogoE;
	ScreenE = Screen + screenSize;

	printf("Screen: %08x to %08x\n", Screen,ScreenE);

//	getchar();

	return true;
}

#define smart_free(x) if (x) free(x); x=NULL;

void uload_files()
{
	smart_free( Font );
	smart_free( Cloud );
	smart_free( Cloud2 );
	smart_free( Cloud3 );
	smart_free( CloudMask );
	smart_free( Cloud2Mask );
	smart_free( Cloud3Mask );
	smart_free( Module1 );
	smart_free( Logo );
}


//	SECTION TutBSS,BSS_C

void bss_c()
{
	printf("size of screen: %d\n",bplsize*FontBpls);

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


bool load_raw(const char *name, int extraSize, void **ptr, void **ptrE)
{
	bool success = false;
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
			memset( *ptr, 0, size + extraSize );

			FRead( fd , *ptr, size ,1 );
			*ptrE = *ptr + size + extraSize;
			success = true;
		}
		FClose(fd);
	}

	if ( ! success )
	{
		printf("failed to load %s\n",name);
	}

	return success;
}

void init_sin()
{
	int i;
	for (i = 0; i < (SineEnd - Sine) ; i++)
	{
		Sine[i] = sin( 2*M_PI * (double) i / (double) (SineEnd - Sine) ) * 45;
	}
}

// --- setup program, cleanup, etc...

struct Window *win = NULL;
struct BitMap *copperBitmap = NULL;

void	cleanup()
{
	if (win) CloseWindow(win); 
	if (copperBitmap) FreeBitMap( copperBitmap ); 

	win = NULL;
	copperBitmap = NULL;

	uload_files();
	close_libs();
}


int main()
{
	if (open_libs() == false)
	{
		close_libs();
	}

	if (load_raw_files() == false)
	{
		printf("Failed to load files\n");
		cleanup();
		return 0;
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

	init_sin();
	bss_c();
	init_cloud();
	init_copper();

	Demo();

	cleanup();

}


// if not mistaken on vblank interrupt, stuff should happen, so this were stuff happens...

void WaitMouse()
{
	bool running = true;
	ULONG sig;
	if (win -> UserPort)
	{
		ULONG win_mask = 1 << win -> UserPort ->mp_SigBit ;

 		do
		{
			WaitTOF();
			VBint();		// trigger interupt...
			render_copper( custom, (uint32 *) Copper,  copperBitmap );
			BltBitMapRastPort(  copperBitmap, 0,0, win -> RPort, 0,0, win -> Width, win -> Height, 0xC0 );

			sig = SetSignal( 0L, win_mask | SIGBREAKF_CTRL_C );
			if (sig & win_mask) if (checkMouse(win, 1)) running = false;
		} while (running);
	}

	dump_copper( (uint32 *) Copper );
}

void WAITBLIT()
{
}


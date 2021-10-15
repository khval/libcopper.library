
	SECTION TutDemo,CODE
	JUMPPTR Start

	INCDIR ""
	INCLUDE "PhotonsMiniStartup/PhotonsMiniWrapper1.04!.S"
	INCLUDE "Blitter-Register-List.S"
	INCLUDE "P61-Options.S"

;;    ---  screen buffer dimensions  ---

w	=352
h	=256
bplsize	=w*h/8
ScrBpl	=w/8

;;    ---  logo dimensions  ---

logow		=320
logoh		=99
logomargin	=(320-logow)/2
logobpl		=logow/8
logobwid	=logobpl*3

skybpl		=320/8+14
skybwid		=skybpl*3
skyh		=220

;;    ---  font dimensions  ---
fontw		=288
fonth		=100
fontbpls	=3
FontBpl		=fontw/8

plotY	=110
plotX	=w-32

logobgcol	=$44f
bgcol		=$225

cloudcount	=10
cloudstructsize	=18

********************  MACROS  ********************

logocolors:macro
	dc.w $068e,$0adf,$0dff
	dc.w $09bf,$056d,$044b,$033a
	endm

cloudcolors:macro
	dc.w $066f,$077f,$088e
	dc.w $0aae,$0bbe,$0dde,$0eee
	endm

WAITBLIT:macro
	tst DMACONR(a6)			;for compatibility
	btst #6,DMACONR(a6)
	bne.s *-6
	endm

********************  DEMO  ********************


Demo:
	bsr.w Init

	move.w #$87e0,$dff096

	move.l #Copper,$dff080
	move.l #VBint,$6c(a4)		;set vertb interrupt vector compatibly.
	move.w #$c020,$dff09a		;enable interrupts generally
					;and vertb specifically.

;;    ---  Call P61_Init  ---
	movem.l d0-a6,-(sp)

	lea Module1,a0
	sub.l a1,a1
	sub.l a2,a2
	moveq #0,d0
	jsr P61_Init

	movem.l (sp)+,d0-a6

	bsr.s Main

	movem.l d0-a6,-(sp)
	jsr P61_End
	movem.l (sp)+,d0-a6
	rts			;go back to system friendly wrapper exit.

********** ROUTINES **********
Main:
	movem.l d0-a6,-(sp)

**************************


WaitMouse:
	btst #6,$bfe001
	bne.s WaitMouse

**************************

	movem.l (sp)+,d0-a6
	rts

********************  Effect 1 subroutine  ********************

Part1:
;	movem.l d0-a6,-(sp)

    *--- clear clouds ---*

	move.l SkyBufferL(PC),a1
	move.l CloudCoordsLP(PC),a3

	moveq #cloudcount-1,d7
.clearl:
	move.l (a3)+,a0			;bob
	move.l (a3)+,a2			;mask

	move.w (a3)+,d0			;x coord
	move.w (a3)+,d1			;y coord
	move.w (a3)+,d2			;width
	move.w (a3)+,d3			;height
	addq.w #2,a3			;skip speed value.
	move.l #$01000000,d4		;clear blit

	bsr.w PlotBob
	dbf d7,.clearl

;@bouncescroller was here

	lea StarSpr+1,a1
	moveq #$2c,d0
	moveq #26-1,d7
.skyl:	
	addq.b #1,(a1)			;add speed to hpos
	addq.w #8,a1			;skip to next sprite control words
	addq.b #2,(a1)			;add speed to hpos
	addq.w #8,a1			;skip to next sprite control words
	addq.b #1,(a1)			;add speed to hpos
	addq.w #8,a1			;skip to next sprite control words
	addq.b #3,(a1)			;add speed to hpos
	addq.w #8,a1			;skip to next sprite control words

	dbf d7,.skyl	

	addq.b #1,(a1)			;add speed to hpos
	addq.w #8,a1			;skip to next sprite control words
	addq.b #2,(a1)			;add speed to hpos
	addq.w #8,a1			;skip to next sprite control words

    *--- below line $ff ---*

	moveq #%00000110,d6

	addq.b #1,(a1)			;add speed to hpos
	addq.w #8,a1
	addq.b #3,(a1)			;add speed to hpos
	addq.w #8,a1

	moveq #5-1,d7
.floorl:	
	addq.b #1,(a1)			;add speed to hpos
	addq.w #8,a1
	addq.b #2,(a1)			;add speed to hpos
	addq.w #8,a1
	addq.b #1,(a1)			;add speed to hpos
	addq.w #8,a1
	addq.b #3,(a1)			;add speed to hpos
	addq.w #8,a1

	dbf d7,.floorl

    *--- odd lines sprites ---*

	lea StarSpr2+1,a1
	moveq #$2d,d0
	moveq #26-1,d7
.skyl2:	
	addq.b #1,(a1)			;add speed to hpos
	addq.w #8,a1			;skip to next sprite control words
	addq.b #2,(a1)			;add speed to hpos
	addq.w #8,a1			;skip to next sprite control words
	addq.b #1,(a1)			;add speed to hpos
	addq.w #8,a1			;skip to next sprite control words
	addq.b #3,(a1)			;add speed to hpos
	addq.w #8,a1			;skip to next sprite control words

	dbf d7,.skyl2

	addq.b #1,(a1)			;add speed to hpos
	addq.w #8,a1			;skip to next sprite control words
	addq.b #2,(a1)			;add speed to hpos
	addq.w #8,a1			;skip to next sprite control words

    *--- below line $ff ---*

	moveq #%00000110,d6

	addq.b #1,(a1)			;add speed to hpos
	addq.w #8,a1
	addq.b #3,(a1)			;add speed to hpos
	addq.w #8,a1

	moveq #5-1,d7
.floorl2:	
	addq.b #1,(a1)			;add speed to hpos
	addq.w #8,a1
	addq.b #2,(a1)			;add speed to hpos
	addq.w #8,a1
	addq.b #1,(a1)			;add speed to hpos
	addq.w #8,a1
	addq.b #3,(a1)			;add speed to hpos
	addq.w #8,a1

	dbf d7,.floorl2

    *--- scroller ---*

	move.w Cmd_StopCount(PC),d0
	beq.s .normal
	subq.w #1,Cmd_StopCount
	bra.s .skipscroll
.normal:
	bsr.w Scrollit

	moveq #32,d2
	move.b LastChar(PC),d0
	cmp.b #'I',d0
	bne.s .noi
	moveq #16,d2
.noi:
	move.w ScrollCtr(PC),d0
	addq.w #4,d0
	cmp.w d2,d0
	blo.s .nowrap

	bsr.w PlotChar			;preserves a0

	clr.w d0
.nowrap:
	move.w d0,ScrollCtr

.skipscroll:

;@clear clouds was here

	bsr.w BounceScroller

    *--- plot clouds ---*

	move.l SkyBufferL(PC),a1
	movem.l CloudCoordsLP(PC),a3-a4
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

	move.l #$0fca0000,d4		;cookie-cut blit
	bsr.w PlotBob
	lea cloudstructsize(a4),a4
	dbf d7,.drawl

;;    ---  sine lookup for raster bar vert. pos.  ---

	lea Sine,a0
	move.w SineCtr,d6
	move.w #$4d-6+37,d7
	add.w (a0,d6.w),d7

	addq.w #2,d6
	cmp.w #SineEnd-Sine,d6
	blt.s .nowrap2
	moveq #0,d6
.nowrap2:
	move.w d6,SineCtr


;;    ---  in front or behind flag  ---

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
	lea $dff000,a6
	btst #5,$1f(a6)			;INTREQR check if it's our vertb int.
	beq.s .notvb

    *--- double buffering for the clouds ---*

	movem.l SkyBufferL(PC),a0-a1
	exg a0,a1
	movem.l a0-a1,SkyBufferL

	movem.l CloudCoordsLP(PC),a0-a1
	exg a0,a1
	movem.l a0-a1,CloudCoordsLP

    *--- playfield 2 ptrs ---*

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
	move.w #$722,$180(a6)
	move.l 4(a6),d0
	lsr.l #8,d0
	and.w #512-1,d0
	cmp.w #$130,d0
	bhs.s .nomax
	cmp.w MaxVpos(PC),d0
	blo.s .nomax
	move.w d0,MaxVpos
.nomax:
	ENDC

	moveq #$20,d0			;poll irq bit
	move.w d0,$9c(a6)		;$dff09c=INTREQ
	move.w d0,$9c(a6)
.notvb:
	movem.l (sp)+,d0-a6		;restore
	rte
	
row	=288*3*20/8
col	=4

PlotChar:	;a0=scrollptr
;	movem.l d0-a6,-(sp)
	move.l ScrollPtr(PC),a0
	lea $dff000,a6

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
	move.l #$09f00000,BLTCON0(a6)
	move.l #$ffffffff,BLTAFWM(a6)
	move.l d0,BLTAPTH(a6)
	move.l #Screen+ScrBpl*3*plotY+plotX/8,BLTDPTH(a6)
	move.w #FontBpl-col,BLTAMOD(a6)
	move.w #ScrBpl-col,BLTDMOD(a6)

	move.w #20*3*64+2,BLTSIZE(a6)
;	movem.l (sp)+,d0-a6
	rts

;Chan	Function
;A	Cloud Mask
;B	Cloud
;C	Destination playfield
;D	Destination playfield


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

	and.w #$f,d0			;shift nibble
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
	move.l #$ffffffff,BLTAFWM(a6)
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
;;    ---  scroll!  ---
bltoffs	=plotY*ScrBpl*3

blth	=20
bltw	=w/16
bltskip	=0				;modulo
brcorner=blth*ScrBpl*3-2

;	movem.l d0-a6,-(sp)
	lea $dff000,a6
	WAITBLIT
	move.l #$49f00002,BLTCON0(a6)
	move.l #$ffffffff,BLTAFWM(a6)
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

    *--- playfield 1 ptrs ---*

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

    *--- playfield 2 ptrs ---*

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

    *--- initialize sprites ---*

	lea StarSpr,a1
	moveq #$2c,d0
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

    *--- below line $ff ---*

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

    *--- odd lines sprites ---*

	lea StarSpr2,a1
	moveq #$2d,d0
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

    *--- below line $ff ---*

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

********** PLAYROUTINE CODE **********
;Note: if this is put in its own section (or compiled as separate binary), then
;jsr <addr>+P61_InitOffset,P61_MusicOffset,P61_EndOffset,P61_SetPositionOffset
;to call the routines.

Playrtn:
	include "P6112/P6112-Play.i"

********** DATA **********
BarBehind:
	dc.w $558	;color00 value
	logocolors
	dc.w $558	;color00 value
	dc.w $558	;color00 value
	dc.w $558	;color00 value
	dc.w $558	;color00 value
	dc.w $558	;color00 value
	dc.w $558	;color00 value
	dc.w $558	;color00 value

	dc.w $99d	;color00...
	logocolors
	dc.w $99d	;color00...
	dc.w $99d	;color00...
	dc.w $99d	;color00...
	dc.w $99d	;color00...
	dc.w $99d	;color00...
	dc.w $99d	;color00...
	dc.w $99d	;color00...

	dc.w $fff
	logocolors
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff

	dc.w $99d
	logocolors
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d

	dc.w $558
	logocolors
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558

	dc.w logobgcol			;restore
	logocolors
	cloudcolors

BarInFront:
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558

	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d

	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff
	dc.w $fff

	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d
	dc.w $99d

	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558
	dc.w $558

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

CloudCoordsLP:
	dc.l CloudCoordsL
	dc.l CloudCoordsL2

CloudCoordsL:
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

LastChar:dc.b 0
Cmd_Bounce:
	dc.b 0
Cmd_StopCount:
	dc.w 0

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
.tmpx:	SET ((.x*$751+$deadbeef)/(.x&$55))&$ff
	dc.w $2c00+.tmpx,$2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w $8000,$0000
.x:	SET (.x+2)&$ffff
.tmpx:	SET ((.x*$753+$eadbeefd)/(.x&$55))&$ff
	dc.w $2c00+.tmpx,$2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w $0000,$8000
.x:	SET (.x+4)&$ffff
.tmpx:	SET ((.x*$755+$adbeefde)/(.x&$55))&$ff
	dc.w $2c00+.tmpx,$2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w $8000,$0000
.x:	SET (.x+8)&$ffff
.tmpx:	SET ((.x*$757+$dbeefdea)/(.x&$55))&$ff
	dc.w $2c00+.tmpx,$2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w $c000,$c000
.x:	SET (.x+16)&$ffff
	ENDR
	dc.w 0,0

StarSpr2:
.x:	SET $77
	REPT 32
.tmpx:	SET ((.x*$751+$deadbeef)/(.x&$55))&$ff
	dc.w $2c00+.tmpx,$2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w $8000,$0000
.x:	SET (.x+2)&$ffff
.tmpx:	SET ((.x*$753+$eadbeefd)/(.x&$55))&$ff
	dc.w $2c00+.tmpx,$2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w $0000,$8000
.x:	SET (.x+4)&$ffff
.tmpx:	SET ((.x*$755+$adbeefde)/(.x&$55))&$ff
	dc.w $2c00+.tmpx,$2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w $8000,$0000
.x:	SET (.x+8)&$ffff
.tmpx:	SET ((.x*$757+$dbeefdea)/(.x&$55))&$ff
	dc.w $2c00+.tmpx,$2d00	;Vstart.b,Hstart/2.b,Vstop.b,%A0000SEH
	dc.w $c000,$c000
.x:	SET (.x+16)&$ffff
	ENDR
	dc.w 0,0

NullSpr:
	dc.w $2a20,$2b00
	dc.w 0,0
	dc.w 0,0

Copper:
	dc.w $1fc,0			;slow fetch mode, AGA compatibility
	dc.w $100,$0200
	dc.b 0,$8e,$2c,$81
	dc.b 0,$90,$2c,$c1
	dc.w $92,$38
	dc.w $94,$d0

	dc.w $108,logobwid-logobpl
	dc.w $10a,skybwid-320/8

	dc.w $102,0
	dc.w $104,$20

	dc.w $1a2,$99b			;sprite colors
	dc.w $1a4,$cce
	dc.w $1a6,$fff
SprP:
	dc.w $120,0
	dc.w $122,0
	dc.w $124,0
	dc.w $126,0
	dc.w $128,0
	dc.w $12a,0
	dc.w $12c,0
	dc.w $12e,0
	dc.w $130,0
	dc.w $132,0
	dc.w $134,0
	dc.w $136,0
	dc.w $138,0
	dc.w $13a,0
	dc.w $13c,0
	dc.w $13e,0

CopBplP:
	dc.w $e0,0		;playfield 1
	dc.w $e2,0
	dc.w $e8,0
	dc.w $ea,0
	dc.w $f0,0
	dc.w $f2,0
CopSkyBplP:
	dc.w $e4,0		;playfield 2
	dc.w $e6,0
	dc.w $ec,0
	dc.w $ee,0
	dc.w $f4,0
	dc.w $f6,0
		
	dc.w $0180,logobgcol
	dc.w $100,$6600
	dc.w $2c07,$fffe

LogoPal:
	dc.w $0180,$044f,$0182,$068e,$0184,$0adf,$0186,$0dff
	dc.w $0188,$09bf,$018a,$056d,$018c,$044b,$018e,$033a
CloudPal:
	dc.w $0192,$066f,$0194,$077f,$0196,$088e
	dc.w $0198,$0aae,$019a,$0bbe,$019c,$0dde,$019e,$0eee

waitras1:
	dc.w $8007,$fffe
	dc.w $180,$558
	dc.w $182,0
	dc.w $184,0
	dc.w $186,0
	dc.w $188,0
	dc.w $18a,0
	dc.w $18c,0
	dc.w $18e,0
	dc.w $192,0
	dc.w $194,0
	dc.w $196,0
	dc.w $198,0
	dc.w $19a,0
	dc.w $19c,0
	dc.w $19e,0
waitras2:
	dc.w $8107,$fffe
	dc.w $180,$99d
	dc.w $182,0
	dc.w $184,0
	dc.w $186,0
	dc.w $188,0
	dc.w $18a,0
	dc.w $18c,0
	dc.w $18e,0
	dc.w $192,0
	dc.w $194,0
	dc.w $196,0
	dc.w $198,0
	dc.w $19a,0
	dc.w $19c,0
	dc.w $19e,0
waitras3:
	dc.w $8207,$fffe
	dc.w $180,$fff
	dc.w $182,0
	dc.w $184,0
	dc.w $186,0
	dc.w $188,0
	dc.w $18a,0
	dc.w $18c,0
	dc.w $18e,0
	dc.w $192,0
	dc.w $194,0
	dc.w $196,0
	dc.w $198,0
	dc.w $19a,0
	dc.w $19c,0
	dc.w $19e,0
waitras4:
	dc.w $8307,$fffe
	dc.w $180,$99d
	dc.w $182,0
	dc.w $184,0
	dc.w $186,0
	dc.w $188,0
	dc.w $18a,0
	dc.w $18c,0
	dc.w $18e,0
	dc.w $192,0
	dc.w $194,0
	dc.w $196,0
	dc.w $198,0
	dc.w $19a,0
	dc.w $19c,0
	dc.w $19e,0
waitras5:
	dc.w $8407,$fffe
	dc.w $180,$558
	dc.w $182,0
	dc.w $184,0
	dc.w $186,0
	dc.w $188,0
	dc.w $18a,0
	dc.w $18c,0
	dc.w $18e,0
	dc.w $192,0
	dc.w $194,0
	dc.w $196,0
	dc.w $198,0
	dc.w $19a,0
	dc.w $19c,0
	dc.w $19e,0
waitras6:
	dc.w $8507,$fffe
	dc.w $180,logobgcol
	dc.w $182,0
	dc.w $184,0
	dc.w $186,0
	dc.w $188,0
	dc.w $18a,0
	dc.w $18c,0
	dc.w $18e,0
	dc.w $192,0
	dc.w $194,0
	dc.w $196,0
	dc.w $198,0
	dc.w $19a,0
	dc.w $19c,0
	dc.w $19e,0

	dc.w $96bf,$fffe

FontPalP:
	dc.w $0182,$0ddd,$0184,$0833,$0186,$0334
	dc.w $0188,$0a88,$018a,$099a,$018c,$0556,$018e,$0633

ScrBplP:
	dc.w $e0,0
	dc.w $e2,0
	dc.w $e8,0
	dc.w $ea,0
	dc.w $f0,0
	dc.w $f2,0
	dc.w $108,ScrBpl*FontBpls-320/8
	dc.w $92,$38
	dc.w $94,$d0

	dc.w $9707,$fffe
	dc.w $180,$44e
	dc.w $9807,$fffe
	dc.w $180,$44f
	dc.w $9907,$fffe
	dc.w $180,$44e

	dc.w $9d07,$fffe
	dc.w $180,$44d
	dc.w $9e07,$fffe
	dc.w $180,$44e
	dc.w $9f07,$fffe
	dc.w $180,$44d

	dc.w $a407,$fffe
	dc.w $180,$44c
	dc.w $a507,$fffe
	dc.w $180,$44d
	dc.w $a607,$fffe
	dc.w $180,$44c

	dc.w $ab07,$fffe
	dc.w $180,$43c
	dc.w $ac07,$fffe
	dc.w $180,$44c
	dc.w $ad07,$fffe
	dc.w $180,$43c

	dc.w $b207,$fffe
	dc.w $180,$33b
	dc.w $b307,$fffe
	dc.w $180,$43c
	dc.w $b407,$fffe
	dc.w $180,$33b

	dc.w $b907,$fffe
	dc.w $180,$33a
	dc.w $ba07,$fffe
	dc.w $180,$33b
	dc.w $bb07,$fffe
	dc.w $180,$33a

	dc.w $c007,$fffe
	dc.w $180,$339
	dc.w $c107,$fffe
	dc.w $180,$33a
	dc.w $c207,$fffe
	dc.w $180,$339
;tone down clouds 1 notch
	dc.w $0192,$044b,$0194,$055b,$0196,$066a
	dc.w $0198,$088a,$019a,$099a,$019c,$0bbb,$019e,$0ccc

	dc.w $c707,$fffe
	dc.w $180,$329
	dc.w $c807,$fffe
	dc.w $180,$339
	dc.w $c907,$fffe
	dc.w $180,$329

	dc.w $ce07,$fffe
	dc.w $180,$228
	dc.w $cf07,$fffe
	dc.w $180,$329
	dc.w $d007,$fffe
	dc.w $180,$228

	dc.w $d507,$fffe
	dc.w $180,$227
	dc.w $d607,$fffe
	dc.w $180,$228
	dc.w $d707,$fffe
	dc.w $180,$227

	dc.w $dc07,$fffe
	dc.w $180,$226
	dc.w $dd07,$fffe
	dc.w $180,$227
	dc.w $de07,$fffe
	dc.w $180,$226

	dc.w $e307,$fffe
	dc.w $180,bgcol
	dc.w $e407,$fffe
	dc.w $180,$226
	dc.w $e507,$fffe
	dc.w $180,bgcol

	dc.w $ffdf,$fffe
;;    ---  bottom plate start  ---
	dc.w $07df,$fffe
	dc.w $10a,-(skybwid+320/8)
	dc.w $104,$20

	dc.w $0192,$0248,$0194,$0348,$0196,$0458
	dc.w $0198,$0668,$019a,$0778,$019c,$09aa,$019e,$0abb

	dc.w $0807,$fffe
	dc.w $180,$236
	dc.w $0a07,$fffe
	dc.w $180,$247
	dc.w $0b07,$fffe
	dc.w $180,$236
	dc.w $0e07,$fffe
	dc.w $180,$258

	dc.w $0f07,$fffe
	dc.w $180,$236

	dc.w $1507,$fffe
	dc.w $180,$269

	dc.w $192,$269
	dc.w $194,$269
	dc.w $196,$269
	dc.w $198,$269
	dc.w $19a,$269
	dc.w $19c,$269
	dc.w $19e,$269

	dc.w $1607,$fffe
	dc.w $180,$236

	dc.w $0192,$0248,$0194,$0348,$0196,$0458
	dc.w $0198,$0668,$019a,$0778,$019c,$09aa,$019e,$0abb

;;    ---  mirror split  ---	
	dc.w $17df,$fffe
	dc.w $182,$0468
	dc.w $184,$0235
	dc.w $186,$0358
	dc.w $188,$0689
	dc.w $18a,$069a
	dc.w $18c,$07bc
	dc.w $18e,$08dd
	dc.w $108,(ScrBpl*FontBpls-320/8)-(ScrBpl*FontBpls*2)

	dc.w $2007,$fffe
	dc.w $180,$27a
	dc.w $182,$27a
	dc.w $184,$27a
	dc.w $186,$27a
	dc.w $188,$27a
	dc.w $18a,$27a
	dc.w $18c,$27a
	dc.w $18e,$27a

	dc.w $192,$27a
	dc.w $194,$27a
	dc.w $196,$27a
	dc.w $198,$27a
	dc.w $19a,$27a
	dc.w $19c,$27a
	dc.w $19e,$27a

	dc.w $2107,$fffe
	dc.w $180,$236
	dc.w $182,$0468
	dc.w $184,$0235
	dc.w $186,$0358
	dc.w $188,$0689
	dc.w $18a,$069a
	dc.w $18c,$07bc
	dc.w $18e,$08dd

	dc.w $0192,$0248,$0194,$0348,$0196,$0458
	dc.w $0198,$0668,$019a,$0778,$019c,$09aa,$019e,$0abb

;;    ---  bottom plate stop  ---
	dc.w $2c07,$fffe
	dc.w $180,$38b
	dc.w $2d07,$fffe
	dc.w $180,$235
	dc.w $2e07,$fffe
	dc.w $180,$247
	dc.w $2f07,$fffe
	dc.w $180,$258
;;    ---  bottom plate thickness  ---
	dc.w $3007,$fffe
	dc.w $180,bgcol

	dc.w $ffff,$fffe
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

Cloud2Mask:
	INCBIN "media/Cloud.64x24x3.masks.raw"
Cloud2MaskE:

Cloud3Mask:
	INCBIN "media/Cloud.48x15x3.masks.raw"
Cloud3MaskE:

Module1:
	incbin "P61.new_ditty"		;usecode $c00b43b

Logo:	INCBIN "sky3centered.raw"
LogoE:
	dcb.b logobwid*6,0


	SECTION TutBSS,BSS_C
Screen:
	ds.b bplsize*fontbpls
ScreenE:

Sky:
	ds.b skybwid*(220+1)
SkyE:
Sky2:
	ds.b skybwid*(220+1)
Sky2E:
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


%11110000	=$f0


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


%11001010	=$ca

	

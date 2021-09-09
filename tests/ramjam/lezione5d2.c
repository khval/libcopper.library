
// Lezione5d2.s	SCORRIMENTO DI UNA FIGURA IN ALTO E IN BASSO MODIFICANDO I
//		PUNTATORI AI PITPLANES NELLA COPPERLIST + EFFETTO DISTORSIONE
//		OTTENUTO CON I $dff102 (bplcon1)


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <hardware/custom.h>

#ifdef __amigaos4__
struct Custom _custom;
struct Custom *custom = &_custom;	// store locally... handle things with do_functions();
#else
struct Custom *custom = 0xDFF000;
#endif

#include "binrary.h"
#include "render.h"

uint32 *BPLPOINTERS = NULL;

struct Window *win;

uint32 d0,d1;
uint32 a0,a1;
uint32 SuGiu;

void init_copper();
void move_copper();
void VAIGIU();
void MettiSu();
void MettiGiu();
void Finito();
void POINTBP2();


#define blank_size 40*98			// bank before.
#define image_size 40*256*3		// pixels 320x256*3 

char all_image_data[ blank_size + image_size + blank_size ];

char *PIC = all_image_data + blank_size;		// We load in the image...


void inizio()
{

/*
	move.l	4.w,a6		; Execbase in a6
	jsr	-$78(a6)	; Disable - ferma il multitasking
	lea	GfxName(PC),a1	; Indirizzo del nome della lib da aprire in a1
	jsr	-$198(a6)	; OpenLibrary
	move.l	d0,GfxBase	; salvo l'indirizzo base GFX in GfxBase
	move.l	d0,a6
	move.l	$26(a6),OldCop	; salviamo l'indirizzo della copperlist vecchia

*/

//	 PUNTIAMO I NOSTRI BITPLANES

	d0 = (uint32) PIC;

	printf("picture %08x\n",d0);

					//	MOVE.L	#PIC,d0		; in d0 mettiamo l'indirizzo della PIC,
					//	LEA	BPLPOINTERS,A1	; puntatori nella COPPERLIST

	a1 = (uint32) BPLPOINTERS;

	printf("BPLPOINTERS %08x\n",a1);

					//	MOVEQ	#2,D1		; numero di bitplanes -1 (qua sono 3)
	d1 = 2;

// POINTBP:

	do
	{
			//	move.w	d0,6(a1)	; copia la word BASSA dell'indirizzo del plane
			//	swap	d0		; scambia le 2 word di d0 (es: 1234 > 3412)
			//	move.w	d0,2(a1)	; copia la word ALTA dell'indirizzo del plane
			//	swap	d0		; scambia le 2 word di d0 (es: 3412 > 1234)

		*((uint16 *)(a1+6)) = d0 & 0xFFFF;
		*((uint16 *)(a1+2)) = d0 >> 16;

			//	ADD.L	#40*256,d0	; + lunghezza bitplane -> prossimo bitplane

		d0 += 40*256;

			//	addq.w	#8,a1		; andiamo ai prossimi bplpointers nella COP

		a1 += 8;
	}	while (d1 -- ); 	//	dbra	d1,POINTBP	; Rifai D1 volte POINTBP (D1=num of bitplanes)


		//	move.l	#COPPERLIST,$dff080	; Puntiamo la nostra COP
		//	move.w	d0,$dff088		; Facciamo partire la COP
		//	move.w	#0,$dff1fc		; Disattiva l'AGA
		//	move.w	#$c00,$dff106		; Disattiva l'AGA

	cop_move_(COP1LCH,(uint32) copperList >> 16);
	cop_move_(COP1LCL,(uint32) copperList & 0xFFFF);
	cop_move_(0x088,d0 & 0xFFFF);
	cop_move_(0x1FC,0);
	cop_move_(0x106,0xC00);

}

void _main_loop_()
{
	bool running = true;
	ULONG sigs;
	ULONG rsigs;

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	ULONG win_mask = 1 << win -> UserPort ->mp_SigBit ;

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	inizio();

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	sigs = win_mask;

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	do
	{
		WaitTOF();		

		render_copper( custom, copperList,  win -> RPort );

		rsigs = SetSignal(0L,sigs);

		if (rsigs & win_mask ) if (checkMouse(win, 1)) running = false;

		move_copper();

	} while (running);
}

void move_copper()
{
	a1 = (uint32) BPLPOINTERS;

	d0 = (*((uint16 *) (2+a1))) << 16;		// move into upper part of D0
	d0 |=  *((uint16 *) (6+a1));			// move into lower part of D0

	if (! SuGiu) 				// move down
	{
		VAIGIU();
		return;
	}

	if ( (uint32) PIC-(40*30) == d0)		// at top
	{
		MettiGiu();
		return;
	}

	// move up.

	d0 -= 40;
	Finito();
}

void MettiGiu()
{
	SuGiu = 0;		// Azzerando SuGiu, al TST.B SuGiu il BEQ
	Finito();			// fara' saltare alla routine VAIGIU
}

void MettiSu()
{
	SuGiu = 0xFF;	 // Quando la label SuGiu non e' a zero,
				 // significa che dobbiamo risalire.
}

void VAIGIU()
{
	if ( (uint32) PIC+(40*30) == d0 )	// siamo arrivati abbastanza in ALTO?
		MettiSu();					// se si, siamo in fondo e dobbiamo risalire

	d0 += 40;					// Aggiungiamo 40, ossia 1 linea, facendo
								// scorrere in ALTO la figura
	Finito();
}

void Finito()
{
	a1 = (uint32) BPLPOINTERS;
	d1 = 2;
	POINTBP2();
}

void POINTBP2()
{
	do
	{
		*((uint16 *) (6+a1)) = d0 & 0xFFFF;
		*((uint16 *) (2+a1)) = d0 >> 16;

		d0 += 40*256; 	// + lunghezza bitplane -> prossimo bitplane
		a1 += 8;			// andiamo ai prossimi bplpointers nella COP
	} while (d1--);
}


void init_copper()
{
	uint32 *ptr = (uint32 *) copperList;

	setCop( 0x120,0x0000);
	setCop( 0x122,0x0000);
	setCop( 0x124,0x0000);
	setCop( 0x126,0x0000);
	setCop( 0x128,0x0000);  // SPRITE
	setCop( 0x12a,0x0000);
	setCop( 0x12c,0x0000);
	setCop( 0x12e,0x0000);
	setCop( 0x130,0x0000);
	setCop( 0x132,0x0000);
	setCop( 0x134,0x0000);
	setCop( 0x136,0x0000);
	setCop( 0x138,0x0000);
	setCop( 0x13a,0x0000);
	setCop( 0x13c,0x0000);
	setCop( 0x13e,0x0000);

	setCop( 0x8e,0x2c81 );	// DiwStrt	(registri con valori normali)
	setCop( 0x90,0x2cc1 );	// DiwStop
	setCop( 0x92,0x0038 );	// DdfStart
	setCop( 0x94,0x00d0 );	// DdfStop
	setCop( 0x102,0 );		// BplCon1
	setCop( 0x104,0 );		// BplCon2
	setCop( 0x108,0 );		// Bpl1Mod
	setCop( 0x10a,0 );		// Bpl2Mod

                                 // 5432109876543210
//	setCop(0x100,B0011001000000000);	// bits 13 e 12 accesi!! (3 = %011)
	setCop(0x100,B0001001000000000);	// bits 13 e 12 accesi!! (3 = %011)

					// 3 bitplanes lowres, non lace

BPLPOINTERS = ptr;		// get copper location..

	setCop( 0xe0,0x0000);
	setCop( 0xe2,0x0000);	//primo	 bitplane

	setCop( 0xe4,0x0000);
	setCop( 0xe6,0x0000);	//secondo bitplane

	setCop( 0xe8,0x0000);
	setCop( 0xea,0x0000);	//terzo	 bitplane

	setCop( 0x0180,0x000 );	// color0
	setCop( 0x0182,0x333 );	// color1
	setCop( 0x0184,0xFFF );	// color2
	setCop( 0x0186,0xCCC );	// color3
	setCop( 0x0188,0xF20 );	// color4
	setCop( 0x018a,0xE30 );	// color5
	setCop( 0x018c,0xFE0 );	// color6
	setCop( 0x018e,0x00F );	// color7

//	EFFETTO SPECCHIO (che si potrebbe vendere per effetto "texturemap")

	setCop( 0x7007,0xfffe );
	setCop( 0x180,0x004 );	// Color0
	setCop( 0x102,0x011 );	// bplcon1
	setCop( 0x7307,0xfffe );
	setCop( 0x180,0x006 );	// Color0
	setCop( 0x102,0x022 );	// bplcon1
	setCop( 0x7607,0xfffe );
	setCop( 0x180,0x008 );	// Color0
	setCop( 0x102,0x033 );	// bplcon1
	setCop( 0x7b07,0xfffe );
	setCop( 0x180,0x00a );	// Color0
	setCop( 0x102,0x044 );	// bplcon1
	setCop( 0x8307,0xfffe );
	setCop( 0x180,0x00c );	// Color0
	setCop( 0x102,0x055 );	// bplcon1
	setCop( 0x9007,0xfffe );
	setCop( 0x180,0x00e );	// Color0
	setCop( 0x102,0x066 );	// bplcon1
	setCop( 0x9607,0xfffe );
	setCop( 0x180,0x00f );	// Color0
	setCop( 0x102,0x077 );	// bplcon1
	setCop( 0x9a07,0xfffe );
	setCop( 0x180,0x00e );	// Color0
	setCop( 0xa007,0xfffe );
	setCop( 0x180,0x00c );	// Color0
	setCop( 0x102,0x066 );	// bplcon1
	setCop( 0xad07,0xfffe );
	setCop( 0x180,0x00a );	// Color0
	setCop( 0x102,0x055 );	// bplcon1
	setCop( 0xb507,0xfffe );
	setCop( 0x180,0x008 );	// Color0
	setCop( 0x102,0x044 );	// bplcon1
	setCop( 0xba07,0xfffe );
	setCop( 0x180,0x006 );	// Color0
	setCop( 0x102,0x033 );	// bplcon1
	setCop( 0xbd07,0xfffe );
	setCop( 0x180,0x004 );	// Color0
	setCop( 0x102,0x022 );	// bplcon1
	setCop( 0xbf07,0xfffe );
	setCop( 0x180,0x001 );	// Color0

	setCop( 0xFFFF,0xFFFE );	// Fine della copperlist
}

void loadImage()
{
	const char *name = "320x256x16C.raw";

	BPTR fd;
	fd = FOpen( name, MODE_OLDFILE, 0  );

	if (fd)
	{
//		ChangeFilePosition( fd , 2 * 16 , OFFSET_BEGINNING);

		FRead( fd , PIC, image_size ,1 );
		FClose(fd);
	}
	else
	{
		printf("*** warning: %s image not loaded ***\n",name);
		printf("<< press enter to continue >>\n");
		getchar();
	}
}

int main()
{
	if (open_libs())
	{
		init_ecs2colors();

		win = OpenWindowTags( NULL, 
			WA_IDCMP,IDCMP_MOUSEBUTTONS,
			WA_Left,320,
			WA_Top,20,
			WA_Width, 640 + 128,
			WA_Height, 480 + 128,
			TAG_END);

		if (win)
		{
			if (win -> UserPort)
			{
				loadImage();
				init_copper();
				 _main_loop_();
			}

			render_DisplayWindow(win -> RPort);

			printf("press enter to quit\n");
			getchar();

			CloseWindow(win);
		}
	}

	close_libs();

	return 0;
}


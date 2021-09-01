#include <stdlib.h>
#include <stdio.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/libblitter.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>

#pragma pack(push,2)
#include <hardware/custom.h>
#pragma pack(pop)

#include <string.h>
#include <stdbool.h>

#include "binrary.h"
#include "common.h"

#include "render.h"

#ifdef __amigaos4__
struct Custom _custom;
struct Custom *custom = &_custom;	// store locally... handle things with do_functions();
#else
struct Custom *custom = 0xDFF000;
#endif

struct Window *win;

//Programmé par Yragael pour Stash of Code (http://www.stashofcode.fr) en 2017.
//Cette œuvre est mise à disposition selon les termes de la Licence Creative Commons Attribution - Pas d’Utilisation Commerciale - Partage dans les Mêmes Conditions 4.0 France.
//Un plasma RGB. Plusieurs versions sont affichées successivement, résultant des différentes combinaisons des sources A, B et C que permet le Blitter. 
//Basé sur les explications données dans l'article "Le RGB plasma" de Stéphane Rubinstein dans Amiga News Tech n°31 (mars 1992)
//Version avec des précaluls et le recours au Blitter pour modifier les couleurs dans la Copper list. Cette optimisation n'est pas du tout efficace sur A1200, mais c'est le jour et la nuit sur A500 : sans optimisation, il est possible d'afficher au plus 60 lignes dans la trame, alors qu'avec optimisation il est possible d'en afficher 256 (et encore, il reste du temps : cela ne prend que 273 lignes de trame |)

//---------- Constantes ----------

//Registres

#define FMODE 0x1FC
#define VPOSR 0x004
#define INTENA 0x09A
#define INTENAR 0x01C
#define INTREQ 0x09C
#define INTREQR 0x01E
#define DMACON 0x096
#define DMACONR 0x002
#define DIWSTRT 0x08E
#define DIWSTOP 0x090
#define BPLCON0 0x100
#define BPLCON1 0x102
#define BPLCON2 0x104
#define DDFSTRT 0x092
#define DDFSTOP 0x094
#define BPL1MOD 0x108
#define BPL2MOD 0x10A
#define BPL1PTH 0x0E0
#define BPL1PTL 0x0E2
#define COLOR00 0x180
#define COLOR01 0x182
#define COP1LCH 0x080
#define COPJMP1 0x088
#define BLTAFWM 0x044
#define BLTALWM 0x046
#define BLTAPTH 0x050
#define BLTBPTH 0x04C
#define BLTCPTH 0x048
#define BLTDPTH 0x054
#define BLTAMOD 0x064
#define BLTBMOD 0x062
#define BLTCMOD 0x060
#define BLTDMOD 0x066
#define BLTADAT 0x074
#define BLTCON0 0x040
#define BLTCON1 0x042
#define BLTSIZE 0x058

//Programme

#define DISPLAY_X 0x81
#define DISPLAY_Y 0x2C
#define DISPLAY_DX 320
#define DISPLAY_DY 256
#define DISPLAY_DEPTH 1
#define COPSIZE (13*4+DISPLAY_DY*(4+((DISPLAY_DX>>3)+1)*4)+4)

//Paramètres du plasma

#define BORDER_COLOR 0x0000
#define OFFSET_AMPLITUDE 10
#define OFFSET_ROW_SPEED 2
#define RED_START (359<<1)
#define RED_ROW_SPEED 1
#define RED_FRAME_SPEED 3
#define RED_AMPLITUDE 18		//OFFSET_AMPLITUDE+RED_AMPLITUDE doit être <= 29
#define GREEN_START (90<<1)
#define GREEN_ROW_SPEED 3
#define GREEN_FRAME_SPEED 3
#define GREEN_AMPLITUDE 15		//OFFSET_AMPLITUDE+GREEN_AMPLITUDE doit être <= 29
#define BLUE_START (60<<1)
#define BLUE_ROW_SPEED 12
#define BLUE_FRAME_SPEED 6
#define BLUE_AMPLITUDE 19		//OFFSET_AMPLITUDE+BLUE_AMPLITUDE doit être <= 29
#define MINTERMS_SPEED 100		//Exprimé en trames (1/50 seconde)

//---------- Macros ----------

#define WAITBLIT()	WaitBlit()

uint32 *bitplane = NULL;
uint16 *rgbOffsets = NULL;
uint16 *rowOffsets = NULL;
uint16 *copperList0 = NULL ;
uint16 *copperList1 = NULL ;
uint16 dmacon = 0 ;
uint16 intena = 0 ;
uint16 intreq = 0 ;
uint16 redSinus = RED_START ;
uint16 greenSinus = GREEN_START ;
uint16 blueSinus= BLUE_START ;

extern uint16 sinus[];
extern uint16 bltcon0[];
extern uint16 red[];
extern uint16 green[];
extern uint16 blue[];


uint32 d0,d1,d2,d3,d4,d5,d6,d7;
uint32 a0,a1,a2,a3,a4,a5,a6,a7;

#define ptr_w(a) *( (uint16 *) (a) )

#define get_w(sourceReg,offset,destReg) destReg = (destReg & 0xFFFF0000) | ptr_w(sourceReg+offset);

#define ptr_l(a) *( (uint32 *) (a) )
#define swap_w(x) (((x & 0xFFFF0000) >> 16) | ((x & 0xFFFF) << 16))
#define bClr(b,s) s = s & (~(1<<b))
#define bSet(b,s) s |= 1<<b; 


#define setCustom_w(hwreg,value) cop_move_((uint32) (hwreg),(uint32) (value)&0xFFFF);
#define setCustom_l(hwreg,value) cop_move_((uint32) (hwreg), (uint32) (value)>>16); cop_move_( (uint32) (hwreg)+2,(uint32) (value)&0xFFFF);
#define getCustom_w(hwreg) 0

#if 0

#ifdef __amigaos4__
#define setCustom_w(hwreg,value) *((uint16 *) ((char *) custom + hwreg))=(uint16) value
#define getCustom_w(hwreg) *((uint16 *) ((char *) custom + hwreg))
#define setCustom_l(hwreg,value) *((uint32 *) ((char *) custom + hwreg))= (uint32) value
#define getCustom_l(hwreg) *((uint32 *) ((char *) custom + hwreg))
#endif

#ifdef __amigaos3__
#define setCustom_w(hwreg,value) *((uint16 *) (0xDFF000 | hwreg)) = (uint16) value
#define getCustom_w(hwreg) 0
#define setCustom_l(hwreg,value) *((uint32 *) (0xDFF000 | hwreg)) = (uint32) value
#endif

#endif


int  degree, amplitude;
uint16 *row_w;
uint16 *rgb_ptr;
uint16 *sin_ptr;
int offset;
int timer;
void *tmp_ptr;
int idx = 0;

bool init_mem()
{

//---------- Initialisations ----------

//Empiler les registres

//	a5 = 0xdff000;

//41876

//Allouer de la mémoire en CHIP mise à 0 pour la Copper list

	copperList0 = AllocVec( COPSIZE, 0x10002 );
	if ( ! copperList0 ) return false;

	copperList1 = AllocVec( COPSIZE, 0x10002 );
	if ( ! copperList1 ) return false;

//Allouer de la mémoire en CHIP mise à 0 pour le bitplane

	bitplane = AllocVec( DISPLAY_DY*(DISPLAY_DX>>3), 0x10002);
	if ( ! bitplane ) return false;

//Allouer de la mémoire pour les offsets des lignes
	
	rowOffsets = AllocVec( DISPLAY_DY<<1, 0x10002 );
	if ( ! rowOffsets ) return false;

//Allouer de la mémoire pour les offsets des composantes
	
	rgbOffsets = AllocVec(3*(360<<1),0x10002);
	if ( ! rgbOffsets ) return false;

	return true;
}


int max_index;

void make_copper_list()
{
	uint16 *p_w = copperList0;

//---------- Copper list ----------

//Configuration de l'écran

	*p_w++ = DIWSTRT;
	*p_w++ = (DISPLAY_Y<<8)|DISPLAY_X;
	*p_w++ = DIWSTOP;
	*p_w++ = ((DISPLAY_Y+DISPLAY_DY-256)<<8)|(DISPLAY_X+DISPLAY_DX-256);
	*p_w++ = BPLCON0;
	*p_w++ = (DISPLAY_DEPTH<<12)|0x0200;
	*p_w++ = BPLCON1;
	*p_w++ = 0x0000;
	*p_w++ = BPLCON2;
	*p_w++ = 0x0000;
	*p_w++ = DDFSTRT;
	*p_w++ = ((DISPLAY_X-17)>>1)&0x00FC;
	*p_w++ = DDFSTOP;
	*p_w++ = ((DISPLAY_X-17+(((DISPLAY_DX>>4)-1)<<4))>>1)&0x00FC;
	*p_w++ = BPL1MOD;
	*p_w++ = 0;
	*p_w++ = BPL2MOD;
	*p_w++ = 0;

//Comptabilité ECS avec AGA

	*p_w++ = FMODE;
	*p_w++ = 0x0000;

//Adresse du bitplane

	*p_w++ = BPL1PTL;
	*p_w++ = (ULONG) bitplane & 0xFFFF;

	*p_w++ = BPL1PTH;
	*p_w++ = (ULONG) bitplane >> 16;

//Palette

	*p_w++ = COLOR00;
	*p_w++ = BORDER_COLOR;

//Plasma (WAIT et valeur de COLOR01 non renseignés)

//	move.w #DISPLAY_DY-1,d0

	for (d0=DISPLAY_DY-1;d0;d0--)
	{
		*p_w++ = 0x0000;
		*p_w++ = 0x0000;

		for (d1=DISPLAY_DX>>3;d1;d1--) //41 MOV par ligne, et non 40...
		{
			*p_w++ = COLOR01;
			*p_w++ = 0x0000;
		}
	}

//Fin

	*p_w++ = 0xFFFF;
	*p_w++ = 0xFFFE;

	max_index = (p_w - copperList0) / 2;

	printf("used copper list inst: %d - %d bytes\n", max_index, max_index * 4);

	printf("max bytes: %d\n", COPSIZE);
}

int main_prog()
{
	ULONG sig = 0;
	ULONG mouse_button_pressed = false;
	ULONG win_mask = 1 << win -> UserPort ->mp_SigBit ;
	ULONG sigs = win_mask | SIGBREAKF_CTRL_C;

	printf("%s:%d\n",__FUNCTION__,__LINE__);

//Couper le système

/*
	movea.l 0x4,a6
	jsr -132(a6)
*/

//Couper les interruptions hardware et les DMA

/*
	move.w INTENAR(a5),intena
	move.w #0x7FFF,INTENA(a5)
	move.w INTREQR(a5),intreq
	move.w #0x7FFF,INTREQ(a5)
	move.w DMACONR(a5),dmacon
	move.w #0x07FF,DMACON(a5)
*/

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	make_copper_list();

	printf("%s:%d\n",__FUNCTION__,__LINE__);

//Double buffering de la Copper list

	memcpy(copperList1,copperList0,(COPSIZE>>2)-1);

//Activer les DMA

	setCustom_w(DMACON,0x83C0); 	//DMAEN=1, COPEN=1, BPLEN=1, COPEN=1, BLTEN=1

//Démarrer la Copper list

	setCustom_l(COP1LCH,copperList0);		//	move.l copperList0,COP1LCH(a5)
	setCustom_w(COPJMP1,0);				//	clr.w COPJMP1(a5)

//---------- Précalculs ----------

//Surface du plasma (simple rectangle de couleur 1)

	WAITBLIT();
	setCustom_w(BLTADAT , 0xFFFF);
	setCustom_w(BLTAMOD , 0);
	setCustom_w(BLTCON0 , 0x01F0);	//Ne pas utiliser la source A pour alimenter BLTADAT mais D = Abc | AbC | ABc | ABC = A
	setCustom_w(BLTCON1 , 0x0000);
	setCustom_l(BLTDPTH , bitplane);
	setCustom_w(BLTSIZE , (DISPLAY_DX>>4)|(DISPLAY_DY<<6));
	doBlitter( custom );

//Offsets des lignes

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	row_w = rowOffsets;											//		movea.l rowOffsets,a0
															//		lea sinus,a1
	degree = (360-1)<<1;										//		move.w #(360-1)<<1,d1					
	for (d0 = DISPLAY_DY-1;d0;d0--)								//		move.w #DISPLAY_DY-1,d0
	{														//	_rowOffsetsLoop:
		amplitude = sinus[degree] * OFFSET_AMPLITUDE	;			//		move.w (a1,d1.w),d2
								 							//		muls #OFFSET_AMPLITUDE,d2
		amplitude = (swap_w(amplitude) >> 2) +OFFSET_AMPLITUDE;	//		swap d2
															//		rol.l #2,d2
															//		addi.w #OFFSET_AMPLITUDE,d2
		bClr(0,amplitude);										//		bClr #0,d2					//Revient à diviser Dn par 2 pour le rapport à [0, AMPLITUDE] puis à le multiplier par 2 pour qu'il permette d'adresser un WORD
		*row_w++=amplitude;									//		move.w d2,(a0)+
		degree -= OFFSET_ROW_SPEED<<1;						//		subi.w #OFFSET_ROW_SPEED<<1,d1
		if (degree<0)
		{													//		bge _rowOffsetsLoopNoSinusUnderflow
			degree += 360<<1;								//		addi.w #360<<1,d1
		}													//	_rowOffsetsLoopNoSinusUnderflow:
	}														//		dbf d0,_rowOffsetsLoop


	printf("%s:%d\n",__FUNCTION__,__LINE__);

//Offsets des composantes

	rgb_ptr = rgbOffsets;										//		movea.l rgbOffsets,a0
	sin_ptr = sinus;											//		lea sinus,a1
	degree = 360-1;											//		move.w #360-1,d0
	for (degree = 360-1; degree>=0; degree--) 						//	_redOffsetsLoop:
	{														//		move.w (a1)+,d1
		amplitude = (*sin_ptr++) * RED_AMPLITUDE;					//		muls #RED_AMPLITUDE,d1
															//		swap d1
		amplitude = (swap_w(d1) >> 2) + RED_AMPLITUDE;			//		rol.l #2,d1
															//		addi.w #RED_AMPLITUDE,d1
		bClr(0,d1);											//		bClr #0,d1					//Revient à diviser Dn par 2 pour le rapport à [0, AMPLITUDE] puis à le multiplier par 2 pour qu'il permette d'adresser un WORD
		*rgb_ptr++=amplitude;									//		move.w d1,(a0)+
	};														//		dbf d0,_redOffsetsLoop
	
	printf("%s:%d\n",__FUNCTION__,__LINE__);

	sin_ptr = sinus;											//		lea sinus,a1
	for (degree = 360-1; degree>=0; degree--)						//		move.w #360-1,d0
	{														//	_greenOffsetsLoop:
		amplitude = (*sin_ptr++) * GREEN_AMPLITUDE;				//		move.w (a1)+,d1
															//		muls #GREEN_AMPLITUDE,d1
		amplitude = (swap_w(d1) >> 2) + GREEN_AMPLITUDE;			//		swap d1
															//		rol.l #2,d1
															//		addi.w #GREEN_AMPLITUDE,d1
		bClr(0,d1);											//		bClr #0,d1					//Revient à diviser Dn par 2 pour le rapport à [0, AMPLITUDE] puis à le multiplier par 2 pour qu'il permette d'adresser un WORD
		*rgb_ptr++=amplitude;									//		move.w d1,(a0)+
	}														//		dbf d0,_greenOffsetsLoop
	
	printf("%s:%d\n",__FUNCTION__,__LINE__);

	sin_ptr = sinus;											//		lea sinus,a1
	for (degree = 360-1; degree>=0; degree--)						//		move.w #360-1,d0
	{														//	_blueOffsetsLoop:
		amplitude = (*sin_ptr++) * BLUE_AMPLITUDE;					//		move.w (a1)+,d1
															//		muls #BLUE_AMPLITUDE,d1
		amplitude = (swap_w(d1) >> 2) + BLUE_AMPLITUDE;			//		swap d1
															//		rol.l #2,d1
															//		addi.w #BLUE_AMPLITUDE,d1
		bClr(0,d1);											//		bClr #0,d1					//Revient à diviser Dn par 2 pour le rapport à [0, AMPLITUDE] puis à le multiplier par 2 pour qu'il permette d'adresser un WORD
		*rgb_ptr++=amplitude;									//		move.w d1,(a0)+
	}														//		dbf d0,_blueOffsetsLoop

//Configuration du Blitter

	printf("%s:%d\n",__FUNCTION__,__LINE__);

	WAITBLIT();
	setCustom_w(BLTCON1,0x0000);
	setCustom_w(BLTAMOD,0);
	setCustom_w(BLTBMOD,0);
	setCustom_w(BLTCMOD,0);
	setCustom_w(BLTDMOD,2);
	setCustom_w(BLTAFWM,0xFFFF);
	setCustom_w(BLTALWM,0xFFFF);

//Timer et offset pour cycler les valeurs de BLTCON0 afin de tester les 256 combinaisons de minterms

	offset = (256-1)<<1;										//	move.w #(256-1)<<1,d7	//Offset dans bltcon0
															//	swap d7
	timer = 1;													//	move.w #1,d7			//Timer

//---------- Programme principal ----------

	printf("%s:%d\n",__FUNCTION__,__LINE__);

//Boucle principale

	do															 //	_loop:
	{

//Attendre la fin de la trame

		do														//	_waitEndOfFrame:
		{														//		move.l VPOSR(a5),d0
			d0 = getCustom_w(VPOSR) >> 8;							//		lsr.l #8,d0
																//		and.w #0x01FF,d0
			d0 &= 0x01FF;											//		cmp.w #DISPLAY_Y+DISPLAY_DY,d0
		} while (d0>DISPLAY_Y+DISPLAY_DY);							//		blt _waitEndOfFrame
	
//Changer de copper list
	
		setCustom_l(COP1LCH,copperList0);							//		move.l copperList0,COP1LCH(a5)
		setCustom_w(COPJMP1,0);									//		clr.w COPJMP1(a5)
		tmp_ptr = copperList1;										//		move.l copperList1,a0
		copperList1 = copperList0;									//		move.l copperList0,copperList1
		copperList0 = tmp_ptr;										//		move.l a0,copperList0
	
		a0 = (ULONG) tmp_ptr;

//Configurer les minterms (tous sauf abc pour D = A | B | C)
	
		WAITBLIT();												//		WAITBLIT
		timer--;													//		subq.w #1,d7
		if (timer<1)												//		bge _mintermsNoChange
		 {																
			timer = MINTERMS_SPEED;								//		move.w #MINTERMS_SPEED,d7
																//		swap d7
																//		lea bltcon0,a1
			setCustom_w(BLTCON0,bltcon0[idx]);						//		move.w (a1,d7.w),BLTCON0(a5)
			idx-=2;												//		subq.w #2,d7
			if (idx<0)
			{													//		bne _mintermsNoUnderflow
				idx = (256-1)<<1;									//		move.w #(256-1)<<1,d7
			}													//	_mintermsNoUnderflow:
																//		swap d7
		}														//	_mintermsNoChange:

//Générer la copper list

		a0 += (13*4);												//	lea 13*4(a0),a0
																//	movea.l rowOffsets,a6
		d3=redSinus;												//	move.w redSinus,d3
		d4=greenSinus;											//	move.w greenSinus,d4
		d5=blueSinus;												//	move.w blueSinus,d5
		d0 = ((DISPLAY_Y&0x00FF)<<8)|((((DISPLAY_X-4)>>2)<<1)&0x00FE)|0x0001;	//	move.w #((DISPLAY_Y&0x00FF)<<8)|((((DISPLAY_X-4)>>2)<<1)&0x00FE)|0x0001,d0
																//	move.w #DISPLAY_DY-1,d1

	//WAIT (alterner la position horizontale entre DISPLAY_X-4 et DISPLAY_X d'une ligne à l'autre pour atténuer l'effet de blocs généré par la longueur des MOV, 8 pixels)

		for(d1=DISPLAY_DY-1;d1>-1;d1--)										//	_rows:
		{																//		btst #0,d1
			if (d1&1 == 0)													//		beq _lineEven
				d0 |= (1<<1);												//		bset #1,d0
			else															//		bra _lineOdd
				d0 &= ~(1<<1);											//	_lineEven:
																		//		bClr #1,d0
																		//	_lineOdd:
			ptr_w(a0) = d0;		a0+=2;										//		move.w d0,(a0)+
			ptr_w(a0) = 0xFFFE;	a0+=2;										//		move.w #0xFFFE,(a0)+
	
//Offsets de départ sinusoïdaux dans les composantes
		
			a1 = (ULONG) rgbOffsets;											//	movea.l rgbOffsets,a1
			d6 = ptr_w(a1+d3);												//	move.w (a1,d3.w),d6
			d6 += ptr_w(a6);												//	add.w (a6),d6
																		//	lea red,a2
			a2 = (ULONG) red+d6;											//	lea (a2,d6.w),a2
	
			a1 =+ (360<<1);												//	lea 360<<1(a1),a1
			d6 = ptr_w(a1+d4);												//	move.w (a1,d4.w),d6
			d6 += ptr_w(a6);												//	add.w (a6),d6
																		//	lea green,a3
			a3 = (ULONG) green + d6;										//	lea (a3,d6.w),a3
	
			a1 += (360<<1);												//	lea 360<<1(a1),a1
			d6 = ptr_w(a1+d5);												//	move.w (a1,d5.w),d6
			d6 += ptr_w(a6);	a6+=2;										//	add.w (a6)+,d6		//Passer à la ligne suivante par la même occasion
				 														//	lea blue,a4
			a4 = (ULONG) blue + d6;											//	lea (a4,d6.w),a4
	
//Série de MOV
		
			WAITBLIT();													//	WAITBLIT
			setCustom_l(BLTAPTH, (APTR) a2);									//	move.l a2,BLTAPTH(a5)
			setCustom_l(BLTBPTH, (APTR) a3);									//	move.l a3,BLTBPTH(a5)
			setCustom_l(BLTCPTH, (APTR) a4);									//	move.l a4,BLTCPTH(a5)
			a0+=2;														//	lea 2(a0),a0
			setCustom_l(BLTDPTH, (APTR) a3);									//	move.l a0,BLTDPTH(a5)
			setCustom_w(BLTSIZE,  1|(((DISPLAY_DX>>3)+1)<<6));			//	move.w #1|(((DISPLAY_DX>>3)+1)<<6),BLTSIZE(a5)
			doBlitter( custom );
	
//Passer à la ligne suivante
	
			d0+=0x0100;													//	addi.w #0x0100,d0
			a0 += 4*((DISPLAY_DX>>3)+1)-2;									//	lea 4*((DISPLAY_DX>>3)+1)-2(a0),a0
	
//Incrémenter les sinus de la ligne
	
			d3 -= RED_ROW_SPEED<<1;										//		subi.w #RED_ROW_SPEED<<1,d3
			if (d3<0)														//		bge _noRedRowSinusUnderflow
				d3 += 360<<1;											//		addi.w #360<<1,d3
																		//	_noRedRowSinusUnderflow:
			d4 -= GREEN_ROW_SPEED<<1;									//		subi.w #GREEN_ROW_SPEED<<1,d4
			if (d4<0)														//		bge _noGreenRowSinusUnderflow
				d4 += 360<<1;											//		addi.w #360<<1,d4
																		//	_noGreenRowSinusUnderflow:
			d5 -= BLUE_ROW_SPEED<<1;										//		subi.w #BLUE_ROW_SPEED<<1,d5
			if (d5<0)														//		bge _noBlueRowSinusUnderflow
				d5 += 360<<1;											//		addi.w #360<<1,d5
																		//	_noBlueRowSinusUnderflow:
		}																//	dbf d1,_rows

//Animer les sinus des composantes

		d3=redSinus;													//		move.w redSinus,d3
		d3 -= RED_FRAME_SPEED<<1;									//		subi.w #RED_FRAME_SPEED<<1,d3
		if (d3<0)														//		bge _noRedSinusUnderflow
			d3 += 360<<1;											//		addi.w #360<<1,d3
																	//	_noRedSinusUnderflow:
		redSinus = d3;													//		move.w d3,redSinus
																	//	
		d4 = greenSinus;												//		move.w greenSinus,d4
		d4 -= GREEN_FRAME_SPEED<<1;									//		subi.w #GREEN_FRAME_SPEED<<1,d4
		if (d4<0)														//		bge _noGreenSinusUnderflow
			d4 += 360<<1;											//		addi.w #360<<1,d4
																	//	_noGreenSinusUnderflow:
		greenSinus = d4;												//		move.w d4,greenSinus
																	//	
		d5 = blueSinus;												//		move.w blueSinus,d5
		d5 -= BLUE_FRAME_SPEED<<1;									//		subi.w #BLUE_FRAME_SPEED<<1,d5
		if (d5<0)														//		bge _noBlueSinusUnderflow
			d5 += 360<<1;											//		addi.w #360<<1,d5
																	//	_noBlueSinusUnderflow:
		blueSinus = d5;												//		move.w d5,blueSinus
	
//Tester la pression du bouton gauche de la souris

		render_copper( (uint32 *) copperList0 );

		sig = SetSignal( 0L , sigs );
//		if (sig & win_mask) if (checkMouse(1)) mouse_button_pressed = true;
																	//	btst #6,0xbfe001
	} while ( ! mouse_button_pressed );										//	bne _loop
	WAITBLIT();														//	WAITBLIT

//---------- Finalisations ----------

//Couper les interruptions hardware et les DMA

	setCustom_w(INTENA,0x7FFF);											//	move.w #0x7FFF,INTENA(a5)
	setCustom_w(INTREQ,0x7FFF);											//	move.w #0x7FFF,INTREQ(a5)
	setCustom_w(DMACON,0x07FF);										//	move.w #0x07FF,DMACON(a5)

//Rétablir les interruptions hardware et les DMA

	d0 = getCustom_w(DMACON);											//	move.w dmacon,d0
	bSet(15,d0);														//	bset #15,d0
	setCustom_w(DMACON,d0);											//	move.w d0,DMACON(a5)
	d0 = getCustom_w(INTREQ);											//	move.w intreq,d0
	bSet(15,d0);														//	bset #15,d0
	setCustom_w(INTREQ,d0);											//	move.w d0,INTREQ(a5)
	d0 = getCustom_w(INTENA);											//	move.w intena,d0
	bSet(15,d0);														//	bset #15,d0
	setCustom_w(INTENA,d0);											//	move.w d0,INTENA(a5)

//Rétablir la Copper list

																	//	lea graphicslibrary,a1
																	//	movea.l 0x4,a6
																	//	jsr -408(a6)
	a1=d0;															//	move.l d0,a1
	setCustom_l(COP1LCH,a1+38);										//	move.l 38(a1),COP1LCH(a5)
	setCustom_w(COPJMP1,0);											//	clr.w COPJMP1(a5)
																	//	jsr -414(a6)

//Rétablir le système

																	//	movea.l 0x4,a6
																	//	jsr -138(a6)

	return 0;
}

void free_mem()
{

//Libérer la mémoire
																	//	movea.l copperList0,a1
	if (copperList0)														//	move.l #COPSIZE,d0
	{																//	movea.l 0x4,a6
		FreeVec(copperList0);											//	jsr -210(a6)
	}																//
																	//	movea.l copperList1,a1
	if (copperList1)														//	move.l #COPSIZE,d0
	{																//	movea.l 0x4,a6
		FreeVec(copperList1);											//	jsr -210(a6)
	}																//
																	//	movea.l bitplane,a1
	if (bitplane)														//	move.l #DISPLAY_DY*(DISPLAY_DX>>3),d0
	{																//	movea.l 0x4,a6
		FreeVec(bitplane);												//	jsr -210(a6)
	}																//
																	//	movea.l rowOffsets,a1
	if (rowOffsets)														//	move.l #DISPLAY_DY<<1,d0
	{																//	movea.l 0x4,a6
		FreeVec(rowOffsets);											//	jsr -210(a6)
	}																//
																	//	movea.l rgbOffsets,a1
	if (rgbOffsets)														//	move.l #3*(360<<1),d0
	{																//	movea.l 0x4,a6
		FreeVec(rgbOffsets);											//	jsr -210(a6)
	}

}																	//  Dépiler les registres
																	//	movem.l (sp)+,d0-d7/a0-a6
bool initScreen()
{
	struct ColorSpec colors[]={
			{0,0xF,0x0,0x0},
			{-1,0x0,0x0,0x0}
		};

	win=OpenWindowTags(NULL,
				WA_IDCMP,IDCMP_MOUSEBUTTONS,
				WA_Flags,WFLG_NOCAREREFRESH |
					WFLG_ACTIVATE |
//					WFLG_BORDERLESS |
//					WFLG_BACKDROP |
					WFLG_RMBTRAP,
				WA_Left,0,
				WA_Top,0,
				WA_Width,640+64,
				WA_Height,512,
				TAG_END);

	if (!win) return false;

	copper_rp = win -> RPort;

	return true;
}

void closeDown()
{
	if (win) CloseWindow(win);
	win = NULL;

}

int main()
{
	int ret = 3;

	if (open_libs()==FALSE)
	{
		Printf("failed to open libs!\n");
		close_libs();
		return 0;
	}

	if ( (init_mem()) && (initScreen()) )
	{
		init_ecs2colors();

		ret = main_prog();
	}
	
	free_mem();

	closeDown();

	close_libs();

	return ret;
}


//---------- Données ----------

//	SECTION yragael,DATA_C


uint16 sinus[]={ 0, 286, 572, 857, 1143, 1428, 1713, 1997, 2280, 2563, 2845, 3126, 3406, 3686, 3964, 4240, 4516, 4790, 5063, 5334, 5604, 5872, 6138, 6402, 6664, 6924, 7182, 7438, 7692, 7943, 8192, 8438, 8682, 8923, 9162, 9397, 9630, 9860, 10087, 10311, 10531, 10749, 10963, 11174, 11381, 11585, 11786, 11982, 12176, 12365, 12551, 12733, 12911, 13085, 13255, 13421, 13583, 13741, 13894, 14044, 14189, 14330, 14466, 14598, 14726, 14849, 14968, 15082, 15191, 15296, 15396, 15491, 15582, 15668, 15749, 15826, 15897, 15964, 16026, 16083, 16135, 16182, 16225, 16262, 16294, 16322, 16344, 16362, 16374, 16382, 16384, 16382, 16374, 16362, 16344, 16322, 16294, 16262, 16225, 16182, 16135, 16083, 16026, 15964, 15897, 15826, 15749, 15668, 15582, 15491, 15396, 15296, 15191, 15082, 14968, 14849, 14726, 14598, 14466, 14330, 14189, 14044, 13894, 13741, 13583, 13421, 13255, 13085, 12911, 12733, 12551, 12365, 12176, 11982, 11786, 11585, 11381, 11174, 10963, 10749, 10531, 10311, 10087, 9860, 9630, 9397, 9162, 8923, 8682, 8438, 8192, 7943, 7692, 7438, 7182, 6924, 6664, 6402, 6138, 5872, 5604, 5334, 5063, 4790, 4516, 4240, 3964, 3686, 3406, 3126, 2845, 2563, 2280, 1997, 1713, 1428, 1143, 857, 572, 286, 0, -286, -572, -857, -1143, -1428, -1713, -1997, -2280, -2563, -2845, -3126, -3406, -3686, -3964, -4240, -4516, -4790, -5063, -5334, -5604, -5872, -6138, -6402, -6664, -6924, -7182, -7438, -7692, -7943, -8192, -8438, -8682, -8923, -9162, -9397, -9630, -9860, -10087, -10311, -10531, -10749, -10963, -11174, -11381, -11585, -11786, -11982, -12176, -12365, -12551, -12733, -12911, -13085, -13255, -13421, -13583, -13741, -13894, -14044, -14189, -14330, -14466, -14598, -14726, -14849, -14968, -15082, -15191, -15296, -15396, -15491, -15582, -15668, -15749, -15826, -15897, -15964, -16026, -16083, -16135, -16182, -16225, -16262, -16294, -16322, -16344, -16362, -16374, -16382, -16384, -16382, -16374, -16362, -16344, -16322, -16294, -16262, -16225, -16182, -16135, -16083, -16026, -15964, -15897, -15826, -15749, -15668, -15582, -15491, -15396, -15296, -15191, -15082, -14968, -14849, -14726, -14598, -14466, -14330, -14189, -14044, -13894, -13741, -13583, -13421, -13255, -13085, -12911, -12733, -12551, -12365, -12176, -11982, -11786, -11585, -11381, -11174, -10963, -10749, -10531, -10311, -10087, -9860, -9630, -9397, -9162, -8923, -8682, -8438, -8192, -7943, -7692, -7438, -7182, -6924, -6664, -6402, -6138, -5872, -5604, -5334, -5063, -4790, -4516, -4240, -3964, -3686, -3406, -3126, -2845, -2563, -2280, -1997, -1713, -1428, -1143, -857, -572, -286};
uint16 bltcon0[]={ B0000111100000000,B0000111100000001,B0000111100000010,B0000111100000011,B0000111100000100,B0000111100000101,B0000111100000110,B0000111100000111,B0000111100001000,B0000111100001001,B0000111100001010,B0000111100001011,B0000111100001100,B0000111100001101,B0000111100001110,B0000111100001111,B0000111100010000,B0000111100010001,B0000111100010010,B0000111100010011,B0000111100010100,B0000111100010101,B0000111100010110,B0000111100010111,B0000111100011000,B0000111100011001,B0000111100011010,B0000111100011011,B0000111100011100,B0000111100011101,B0000111100011110,B0000111100011111,B0000111100100000,B0000111100100001,B0000111100100010,B0000111100100011,B0000111100100100,B0000111100100101,B0000111100100110,B0000111100100111,B0000111100101000,B0000111100101001,B0000111100101010,B0000111100101011,B0000111100101100,B0000111100101101,B0000111100101110,B0000111100101111,B0000111100110000,B0000111100110001,B0000111100110010,B0000111100110011,B0000111100110100,B0000111100110101,B0000111100110110,B0000111100110111,B0000111100111000,B0000111100111001,B0000111100111010,B0000111100111011,B0000111100111100,B0000111100111101,B0000111100111110,B0000111100111111,B0000111101000000,B0000111101000001,B0000111101000010,B0000111101000011,B0000111101000100,B0000111101000101,B0000111101000110,B0000111101000111,B0000111101001000,B0000111101001001,B0000111101001010,B0000111101001011,B0000111101001100,B0000111101001101,B0000111101001110,B0000111101001111,B0000111101010000,B0000111101010001,B0000111101010010,B0000111101010011,B0000111101010100,B0000111101010101,B0000111101010110,B0000111101010111,B0000111101011000,B0000111101011001,B0000111101011010,B0000111101011011,B0000111101011100,B0000111101011101,B0000111101011110,B0000111101011111,B0000111101100000,B0000111101100001,B0000111101100010,B0000111101100011,B0000111101100100,B0000111101100101,B0000111101100110,B0000111101100111,B0000111101101000,B0000111101101001,B0000111101101010,B0000111101101011,B0000111101101100,B0000111101101101,B0000111101101110,B0000111101101111,B0000111101110000,B0000111101110001,B0000111101110010,B0000111101110011,B0000111101110100,B0000111101110101,B0000111101110110,B0000111101110111,B0000111101111000,B0000111101111001,B0000111101111010,B0000111101111011,B0000111101111100,B0000111101111101,B0000111101111110,B0000111101111111,B0000111110000000,B0000111110000001,B0000111110000010,B0000111110000011,B0000111110000100,B0000111110000101,B0000111110000110,B0000111110000111,B0000111110001000,B0000111110001001,B0000111110001010,B0000111110001011,B0000111110001100,B0000111110001101,B0000111110001110,B0000111110001111,B0000111110010000,B0000111110010001,B0000111110010010,B0000111110010011,B0000111110010100,B0000111110010101,B0000111110010110,B0000111110010111,B0000111110011000,B0000111110011001,B0000111110011010,B0000111110011011,B0000111110011100,B0000111110011101,B0000111110011110,B0000111110011111,B0000111110100000,B0000111110100001,B0000111110100010,B0000111110100011,B0000111110100100,B0000111110100101,B0000111110100110,B0000111110100111,B0000111110101000,B0000111110101001,B0000111110101010,B0000111110101011,B0000111110101100,B0000111110101101,B0000111110101110,B0000111110101111,B0000111110110000,B0000111110110001,B0000111110110010,B0000111110110011,B0000111110110100,B0000111110110101,B0000111110110110,B0000111110110111,B0000111110111000,B0000111110111001,B0000111110111010,B0000111110111011,B0000111110111100,B0000111110111101,B0000111110111110,B0000111110111111,B0000111111000000,B0000111111000001,B0000111111000010,B0000111111000011,B0000111111000100,B0000111111000101,B0000111111000110,B0000111111000111,B0000111111001000,B0000111111001001,B0000111111001010,B0000111111001011,B0000111111001100,B0000111111001101,B0000111111001110,B0000111111001111,B0000111111010000,B0000111111010001,B0000111111010010,B0000111111010011,B0000111111010100,B0000111111010101,B0000111111010110,B0000111111010111,B0000111111011000,B0000111111011001,B0000111111011010,B0000111111011011,B0000111111011100,B0000111111011101,B0000111111011110,B0000111111011111,B0000111111100000,B0000111111100001,B0000111111100010,B0000111111100011,B0000111111100100,B0000111111100101,B0000111111100110,B0000111111100111,B0000111111101000,B0000111111101001,B0000111111101010,B0000111111101011,B0000111111101100,B0000111111101101,B0000111111101110,B0000111111101111,B0000111111110000,B0000111111110001,B0000111111110010,B0000111111110011,B0000111111110100,B0000111111110101,B0000111111110110,B0000111111110111,B0000111111111000,B0000111111111001,B0000111111111010,B0000111111111011,B0000111111111100,B0000111111111101,B0000111111111110,B0000111111111111};

//Composantes (dent de scie)


//Composantes (sinusoïde)

uint16 red[]={ 0x0800, 0x0900, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00, 0x0F00, 0x0F00, 0x0F00, 0x0E00, 0x0D00, 0x0C00, 0x0B00, 0x0900, 0x0800, 0x0600, 0x0400, 0x0300, 0x0200, 0x0100, 0x0000, 0x0000, 0x0000, 0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0600, 0x0800, 0x0900, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00, 0x0F00, 0x0F00, 0x0F00, 0x0E00, 0x0D00, 0x0C00, 0x0B00, 0x0900, 0x0800, 0x0600, 0x0400, 0x0300, 0x0200, 0x0100, 0x0000, 0x0000, 0x0000, 0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0600, 0x0800, 0x0900, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00, 0x0F00, 0x0F00, 0x0F00};
uint16 green[]={ 0x0080, 0x0090, 0x00B0, 0x00C0, 0x00D0, 0x00E0, 0x00F0, 0x00F0, 0x00F0, 0x00F0, 0x00E0, 0x00D0, 0x00C0, 0x00B0, 0x0090, 0x0080, 0x0060, 0x0040, 0x0030, 0x0020, 0x0010, 0x0000, 0x0000, 0x0000, 0x0000, 0x0010, 0x0020, 0x0030, 0x0040, 0x0060, 0x0080, 0x0090, 0x00B0, 0x00C0, 0x00D0, 0x00E0, 0x00F0, 0x00F0, 0x00F0, 0x00F0, 0x00E0, 0x00D0, 0x00C0, 0x00B0, 0x0090, 0x0080, 0x0060, 0x0040, 0x0030, 0x0020, 0x0010, 0x0000, 0x0000, 0x0000, 0x0000, 0x0010, 0x0020, 0x0030, 0x0040, 0x0060, 0x0080, 0x0090, 0x00B0, 0x00C0, 0x00D0, 0x00E0, 0x00F0, 0x00F0, 0x00F0, 0x00F0};
uint16 blue[]={ 0x0008, 0x0009, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F, 0x000F, 0x000F, 0x000F, 0x000E, 0x000D, 0x000C, 0x000B, 0x0009, 0x0008, 0x0006, 0x0004, 0x0003, 0x0002, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0006, 0x0008, 0x0009, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F, 0x000F, 0x000F, 0x000F, 0x000E, 0x000D, 0x000C, 0x000B, 0x0009, 0x0008, 0x0006, 0x0004, 0x0003, 0x0002, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0006, 0x0008, 0x0009, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F, 0x000F, 0x000F, 0x000F};


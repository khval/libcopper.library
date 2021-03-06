

#define setCop(a,b) *ptr++ = ((a)<<16)|(b)

typedef unsigned short uint16;
//typedef unsigned int uint32;		// found in exec/types.h

// set some values not importent what values, this is just a test... 

#define COPJMP1 0x088
#define COPJMP2 0x08A

// Display Window

#define DIWSTART 0x08E
#define DIWSTOP 0x090

// Data Fetch, (is in clocks, 4 in lowres, 8 in hires)

#define DDFSTART 0x092
#define DDFSTOP 0x094

#define INTREQ 0x09C

#define COP1LCH 0x080
#define COP1LCL 0x082

#define COP2LCH 0x084
#define COP2LCL 0x086

#define BPL1PTH 0x0E0
#define BPL1PTL 0x0E2

#define BPL2PTH 0x0E4
#define BPL2PTL 0x0E6

#define BPL3PTH 0x0E8
#define BPL3PTL 0x0EA

#define BPL4PTH 0x0EC
#define BPL4PTL 0x0EE

#define BPL5PTH 0x0F0
#define BPL5PTL 0x0F2

#define BPL6PTH 0x0F4
#define BPL6PTL 0x0F6

#define BPL7PTH 0x0F8
#define BPL7PTL 0x0FA

#define BPL8PTH 0x0FC
#define BPL8PTL 0x0FE

#define BPLCON0 0x100
#define BPLCON1 0x102
#define BPLCON2 0x104
#define BPLCON3 0x106
#define BPL1MOD 0x108
#define BPL2MOD 0x10A

#define COLOR00 0x180
#define COLOR01 0x182
#define COLOR02 0x184
#define COLOR03 0x186
#define COLOR04 0x188
#define COLOR05 0x18A
#define COLOR06 0x18C
#define COLOR07 0x18E
#define COLOR08 0x190
#define COLOR09 0x192
#define COLOR10 0x194
#define COLOR11 0x196
#define COLOR12 0x198
#define COLOR13 0x19A
#define COLOR14 0x19C
#define COLOR15 0x19E

#define COPDEBUGON	0xFFB0
#define COPDEBUGOFF	0xFFC0
#define COPSTAT		0xFFD0
#define COPPAL 		0xFFE0
#define COPDEBUG		0xFFF0

#define lowres_clock 4

#pragma pack(2)

union cop
{
	uint32 d32;
	struct 
	{
		uint16 a;
		uint16 b;
	} d16;
};

#pragma pack(1)

union argb_u
{
	uint32 argb;
	struct
	{
		uint8 a;
		uint8 r;
		uint8 g;
		uint8 b;
	} channel;
};

#pragma pack()

extern uint32 copperList[2000 + 0x1000];
extern uint32 copperl1;
extern uint32 copperl2;

extern union argb_u ecs2argb[0x10000];

extern uint32 COP1LC, COP2LC;
extern uint32 diwstart, diwstop, ddfstart, ddfstop;

extern int WordCountToDispDataFetchStop( int hires, int ddstart, int wc );
extern int DispDataFetchWordCount( int hires, int ddfstart, int ddfstop);
extern int DispWinToDispDataFetch(int hires, int diwstart);

extern void clu( int x, int y );
extern void crb(int y);
extern void init_ecs2colors();
extern void cop_move(union cop data);
extern void plot( int x,int y , char *data);
extern void cop_move_(uint16 reg, uint16 data);
extern void cop_skip(union cop data);
extern void render_copper(struct Custom *custom, uint32 *copperList, struct BitMap *bm);
extern void dump_copper(uint32 *copperList);


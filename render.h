

#define setCop(a,b) *ptr++ = ((a)<<16)|(b)

typedef unsigned short uint16;
//typedef unsigned int uint32;		// found in exec/types.h

// set some values not importent what values, this is just a test... 

#define INTREQ 2
#define COPJMP1 4
#define COPJMP2 8
#define COP1LCH 18
#define COP1LCL 20
#define COP2LCH 22
#define COP2LCL 24
#define BPL1PTH 26
#define BPL1PTL 28
#define BPL2PTH 30
#define BPL2PTL 32
#define BPLCON0 33

// Display Window

#define DIWSTART 10
#define DIWSTOP 12

// Data Fetch, (is in clocks, 4 in lowres, 8 in hires)

#define DDFSTART 14
#define DDFSTOP 16

#define COLOR00 0x180
#define COLOR01 0x182
#define COLOR02 0x184
#define COLOR03 0x186
#define COLOR04 0x188
#define COLOR05 0x18A

#define lowres_clock 4


union cop
{
	uint32 d32;
	struct 
	{
		uint16 a;
		uint16 b;
	} d16;
};

extern uint32 copperList[1000];
extern uint32 copperl1;
extern uint32 copperl2;
extern uint32 ecs2argb[0x10000];

extern uint32 COP1LC, COP2LC;
extern uint32 diwstart, diwstop, ddfstart, ddfstop;

extern struct RastPort *rp;

extern int WordCount( int ddfstart, int ddfstop);
extern void clu( int x, int y );
extern void crb(int y);
extern void init_ecs2colors();
extern void cop_move(union cop data);
extern void plot( int x,int y);
extern void cop_move_(uint16 reg, uint16 data);
extern void cop_skip(union cop data);
extern void render_copper();






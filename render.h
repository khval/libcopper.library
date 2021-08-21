

#define setCop(a,b) *ptr++ = ((a)<<16)|(b)

typedef unsigned short uint16;
//typedef unsigned int uint32;		// found in exec/types.h

// set some values not importent what values, this is just a test... 

#define INTREQ 0x09C
#define COPJMP1 0x088
#define COPJMP2 0x08A

#define COP1LCH 0x080
#define COP1LCL 0x082

#define COP2LCH 0x084
#define COP2LCL 0x086

#define BPL1PTH 0x0E0
#define BPL1PTL 0x0E2
#define BPL2PTH 0x0E4
#define BPL2PTL 0x0E6

#define BPLCON0 0x100
#define BPLCON1 0x102
#define BPLCON2 0x104
#define BPLCON3 0x106
#define BPL1MOD 0x108
#define BPL2MOD 0x10A

// Display Window

#define DIWSTART 0x08C
#define DIWSTOP 0x090

// Data Fetch, (is in clocks, 4 in lowres, 8 in hires)

#define DDFSTART 0x092
#define DDFSTOP 0x094

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
extern void plot( int x,int y , char *data);
extern void cop_move_(uint16 reg, uint16 data);
extern void cop_skip(union cop data);
extern void render_copper();






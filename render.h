

#define setCop(a,b) *ptr++ = ((a)<<16)|(b)

typedef unsigned short uint16;
//typedef unsigned int uint32;		// found in exec/types.h

// set some values not importent what values, this is just a test... 

#define INTREQ 2
#define COPJMP1 4
#define COPJMP2 8

// Display Window

#define DIWSTART 10
#define DIWSTOP 12

// Data Fetch, (is in clocks, 4 in lowres, 8 in hires)

#define DDFSTART 14
#define DDFSTOP 16

#define COLOR0 0x180
#define COLOR1 0x182

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






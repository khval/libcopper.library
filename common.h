

enum {
	RD0,	RD1,	RD2,	RD3,	RD4,	RD5,	RD6,	RD7,
	RA0,	RA1,	RA2,	RA3,	RA4,	RA5,	RA6,	RA7 };

union reg_u
{
	uint32 b32;
	struct {
		union
		{
			uint16 hw;
			struct
			{
				uint8 b3;
				uint8 b2;
			};
		};
		union
		{
			uint16 lw;
			struct
			{
				uint8 b1;
				uint8 b0;
			};
		};
	};
};

static union reg_u D0,D1,D2,D3,D4,D5,D6,D7;
static union reg_u A0,A1,A2,A3,A4,A5,A6,A7;

static union reg_u *regArray[]={&D0,&D1,&D2,&D3,&D4,&D5,&D6,&D7,&A0,&A1,&A2,&A3,&A4,&A5,&A6,&A7};

void movem_push(uint32 reg_low, uint32 reg_hi);
void movem_pop(uint32 reg_low, uint32 reg_hi);

#define d0 D0.b32
#define d1 D1.b32
#define d2 D2.b32
#define d3 D3.b32
#define d4 D4.b32
#define d5 D5.b32
#define d6 D6.b32
#define d7 D7.b32

#define a0 A0.b32
#define a1 A1.b32
#define a2 A2.b32
#define a3 A3.b32
#define a4 A4.b32
#define a5 A5.b32
#define a6 A6.b32
#define a7 A7.b32


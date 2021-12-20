

enum {
	RD0,	RD1,	RD2,	RD3,	RD4,	RD5,	RD6,	RD7,
	RA0,	RA1,	RA2,	RA3,	RA4,	RA5,	RA6,	RA7 };

union reg_u
{
	union
	{
		uint32 b32;
		int32 s32;
	};
	struct {
		union
		{
			union
			{
				uint16 hw;	// unsigned high word
				int16 shw;	// signed high word
			};
			struct
			{
				uint8 b3;		// high byte
				uint8 b2;
			};
		};
		union
		{
			union
			{
				uint16 lw;		// unsigned low word
				int16 slw;		// signed low word
			};
			struct
			{
				uint8 b1;
				uint8 b0;		// low byte
			};
		};
	};
};

extern union reg_u D0,D1,D2,D3,D4,D5,D6,D7;
extern union reg_u A0,A1,A2,A3,A4,A5,A6,A7;
extern union reg_u *regArray[];

void movem_push(uint32 reg_low, uint32 reg_hi);
void movem_pop(uint32 reg_low, uint32 reg_hi);
int stack_size( union reg_u *stack );
void dump_stack( union reg_u *stack );

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

struct XYSTW_Vertex3D { 
	float x, y; 
	float s, t, w; 
}; 

typedef struct CompositeHookData_s {
	struct BitMap *srcBitMap; // The source bitmap
	int32 srcWidth, srcHeight; // The source dimensions
	int32 offsetX, offsetY; // The offsets to the destination area relative to the window's origin
	int32 scaleX, scaleY; // The scale factors
	uint32 retCode; // The return code from CompositeTags()
} CompositeHookData;




#define USE_A 0x8
#define USE_B 0x4
#define USE_C 0x2
#define USE_D 0x1

struct blitstate
{
  BOOL    b_LineMode;
  BOOL    b_FillBit;
  uint8   b_UseCode;
  uint8   b_AShift;
  uint8   b_BShift;
  uint8   b_MinTerms;
  uint16  b_Width;
  uint16  b_Height;
  uint16  b_Flags;
  uint16  b_AFWM;
  uint16  b_ALWM;
  uint16  b_FillMode;
  
  uint16 *b_cptr;
  uint16 *b_bptr;
  uint16 *b_aptr;
  uint16 *b_dptr;
  
  int32   b_amod;
  int32   b_bmod;
  int32   b_cmod;
  int32   b_dmod;

  uint16  b_adat;
  uint16  b_bdat;
  uint16  b_cdat;
  
  uint16  b_DMACON;
  uint16  b_uDMACON;
  uint16  b_Last_BLTCON0;
  uint16  b_Busy;
  
  uint16  b_BLTCON0;
  uint16  b_BLTCON1;

  uint16  b_INTENA;
  uint16  b_INTREQ;
  BOOL    b_CausePending;

  struct Interrupt b_SoftInt; // End of DMA interrupt
  
  int32        b_Sig;
  uint32       b_SigMask;
  struct Task *b_SigTask;

  struct Interrupt  b_FaultInt;
  struct Interrupt *b_OldFaultInt;
};

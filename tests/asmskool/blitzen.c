/*
** Blitzen
**
** A blitter emulator that probably won't improve the
** classic compatability of OS4 on non-classic hardware
** significantly but seemed like a fun exercise anyway.
**
** By Peter Gordon (pete@petergordon.og.uk)
**
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <exec/types.h>
#include <exec/tasks.h>
#include <exec/emulation.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>

#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>

#ifdef is_patch
struct Library *GfxBase = NULL;
struct GraphicsIFace *IGraphics = NULL;
#endif

#include "blitzen.h"

TEXT __attribute((used)) ver[] = "$VER: Blitzen 0.3 (06.01.2007)";

#define BLTDDAT 0x000

#define DMACONR 0x002
#define ADKCONR 0x010
#define INTENAR 0x01c
#define INTREQR 0x01e

#define BLTCON0  0x040
#define BLTCON1  0x042
#define BLTAFWM  0x044
#define BLTALWM  0x046
#define BLTCPTH  0x048
#define BLTCPTL  0x04a
#define BLTBPTH  0x04c
#define BLTBPTL  0x04e
#define BLTAPTH  0x050
#define BLTAPTL  0x052
#define BLTDPTH  0x054
#define BLTDPTL  0x056
#define BLTSIZE  0x058
#define BLTCON0L 0x05a
#define BLTSIZV  0x05c
#define BLTSIZH  0x05e
#define BLTCMOD  0x060
#define BLTBMOD  0x062
#define BLTAMOD  0x064
#define BLTDMOD  0x066
#define BLTCDAT  0x070
#define BLTBDAT  0x072
#define BLTADAT  0x074

#define DMACON  0x096
#define INTENA  0x09a
// #define INTREQ  0x09c

APTR oldWaitBlit = NULL;

struct MsgPort *blit_mp = NULL;

struct blitstate _bstate ;
struct blitstate *bstate = &_bstate;

void blitzenWrite( uint16 reg, uint16 value, BOOL *handled, struct blitstate *bs );

void CustomToBlitsate( struct Custom *custom, struct blitstate *bs )
{
	BOOL handled;
	int value;

    blitzenWrite( BLTCON0, custom -> bltcon0, &handled, bstate );
    blitzenWrite( BLTCON1, custom -> bltcon1, &handled, bstate );
    blitzenWrite( BLTAFWM, custom -> bltafwm, &handled, bstate );
    blitzenWrite( BLTALWM, custom -> bltalwm, &handled, bstate );
    blitzenWrite( BLTCPTH, (uint32) custom -> bltcpt >> 16, &handled, bstate );
    blitzenWrite( BLTCPTL, (uint32) custom -> bltcpt, &handled, bstate );
    blitzenWrite( BLTBPTH, (uint32)custom -> bltbpt >>16, &handled, bstate );
    blitzenWrite( BLTBPTL, (uint32) custom -> bltbpt, &handled, bstate );
    blitzenWrite( BLTAPTH, (uint32) custom -> bltapt >>16, &handled, bstate );
    blitzenWrite( BLTAPTL, (uint32) custom -> bltapt, &handled, bstate );
    blitzenWrite( BLTDPTH, (uint32) custom -> bltdpt >>16, &handled, bstate );
    blitzenWrite( BLTDPTL, (uint32) custom -> bltdpt, &handled, bstate );
    blitzenWrite( BLTSIZE, custom -> bltsize, &handled, bstate );
    blitzenWrite( BLTAMOD, custom -> bltamod, &handled, bstate );
    blitzenWrite( BLTBMOD, custom -> bltbmod, &handled, bstate );
    blitzenWrite( BLTCMOD, custom -> bltcmod, &handled, bstate );
    blitzenWrite( BLTDMOD, custom -> bltdmod, &handled, bstate );
    blitzenWrite( BLTADAT, custom -> bltadat, &handled, bstate );
    blitzenWrite( BLTBDAT, custom -> bltbdat, &handled, bstate );
    blitzenWrite( BLTCDAT, custom -> bltcdat, &handled, bstate );


}


void blitzenWrite( uint16 reg, uint16 value, BOOL *handled, struct blitstate *bs )
{
  uint16 old;

  switch( reg )
  {
    case DMACON:
      if( bs->b_DMACON & DMAF_MASTER )
        old = bs->b_DMACON;
      else
        old = 0;
      
      if( value & DMAF_SETCLR )
        bs->b_DMACON |= (value&~DMAF_SETCLR);
      else
        bs->b_DMACON &= ~(value&~DMAF_SETCLR);
      
      if( bs->b_DMACON & DMAF_MASTER )
        bs->b_uDMACON = bs->b_DMACON;
      else
        bs->b_uDMACON = 0;
        
      
      break;
    
    case INTENA:
      if( value & INTF_SETCLR )
        bs->b_INTENA |= (value & ~INTF_SETCLR);
      else
        bs->b_INTENA &= (value & ~INTF_SETCLR);
     
      if( ( !bs->b_CausePending ) &&
          ( ( bs->b_INTREQ & bs->b_INTENA ) != 0 ) )
      {
        bs->b_CausePending = TRUE;
        Cause( &bs->b_SoftInt );
      }
      
      break;
    
    case INTREQ:
      if( value & INTF_SETCLR )
        bs->b_INTREQ |= (value & ~INTF_SETCLR);
      else
        bs->b_INTREQ &= ~(value & ~INTF_SETCLR);

      if( ( !bs->b_CausePending ) &&
          ( ( bs->b_INTREQ & bs->b_INTENA ) != 0 ) )
      {
        bs->b_CausePending = TRUE;
        Cause( &bs->b_SoftInt );
      }
      
      break;

    case BLTCON0L:
      value = (value&0xff)|(bs->b_Last_BLTCON0&0xff00);
    case BLTCON0:
      bs->b_MinTerms     = value&0xff;
      bs->b_AShift       = (value>>12)&0x0f;
      bs->b_UseCode      = (value>>8)&0x0f;
      bs->b_Last_BLTCON0 = value;
      bs->b_BLTCON0  = value;
      
      break;
    
    case BLTCON1:
      bs->b_BShift   = (value>>12)&0x0f;
      bs->b_Flags    = value&0xff;
      bs->b_LineMode = value&1;
      bs->b_FillBit  = (value&4)>>2;
      bs->b_FillMode = (value&24)>>3;
      bs->b_BLTCON1  = value;
      
      break;
    
    case BLTAFWM:
      bs->b_AFWM = value;
      
      break;
    
    case BLTALWM:
      bs->b_ALWM = value;
      break;
    
    case BLTCPTH:
      bs->b_cptr = (uint16 *)((((uint32)bs->b_cptr)&0x0000ffff)|(value<<16));
      break;

    case BLTCPTL:
      bs->b_cptr = (uint16 *)((((uint32)bs->b_cptr)&0xffff0000)|(value&0xfffe));
      break;
    
    case BLTBPTH:
      bs->b_bptr = (uint16 *)((((uint32)bs->b_bptr)&0x0000ffff)|(value<<16));
      break;

    case BLTBPTL:
      bs->b_bptr = (uint16 *)((((uint32)bs->b_bptr)&0xffff0000)|(value&0xfffe));
      break;
    
    case BLTAPTH:
      bs->b_aptr = (uint16 *)((((uint32)bs->b_aptr)&0x0000ffff)|(value<<16));
      break;

    case BLTAPTL:
      bs->b_aptr = (uint16 *)((((uint32)bs->b_aptr)&0xffff0000)|(value&0xfffe));
      break;
    
    case BLTDPTH:
      bs->b_dptr = (uint16 *)((((uint32)bs->b_dptr)&0x0000ffff)|(value<<16));
      break;

    case BLTDPTL:
      bs->b_dptr = (uint16 *)((((uint32)bs->b_dptr)&0xffff0000)|(value&0xfffe));
      break;
      
    case BLTSIZE:
      bs->b_Width  = (value&0x3f);  // Words
      if( bs->b_Width == 0 ) bs->b_Width = 64;

      bs->b_Height = value >> 6;
      if( bs->b_Height == 0 ) bs->b_Height = 1024;
      break;
    
    case BLTAMOD:
      bs->b_amod = value>>1; // We need the modulo in words
      if( value & 0x8000 ) bs->b_amod |= 0xffff8000; // Sign extend
      break;

    case BLTBMOD:
      bs->b_bmod = value>>1;
      if( value & 0x8000 ) bs->b_bmod |= 0xffff8000; // Sign extend
      break;

    case BLTCMOD:
      bs->b_cmod = value>>1;
      if( value & 0x8000 ) bs->b_cmod |= 0xffff8000; // Sign extend
      break;

    case BLTDMOD:
      bs->b_dmod = value>>1;
      if( value & 0x8000 ) bs->b_dmod |= 0xffff8000; // Sign extend
      break;  
    
    case BLTADAT:
      bs->b_adat = value;
      break;

    case BLTBDAT:
      bs->b_bdat = value;
      break;

    case BLTCDAT:
      bs->b_cdat = value;
      break;
  }
}

extern uint8 *Screen;

int blitzenDebug = 0;

// BLTSIZE or BLTSIZH has been written to,
// so start a new blit.
void blitzenBlit( struct blitstate *bs )
{
  uint16 adat, bdat, cdat, ddat; // Data for DMA channels
  uint16 asdat, bsdat;           // Data shifted in or out of A & B channels
  uint32 aso, asi, bso, bsi;     // A shift out, A shift in, B shift out, B shift in
  uint16 tmp, atmp, astmp, zflag;
  uint32 i, wc;

if (blitzenDebug == 1)
{ 
 printf( "Blit:\n" );
  printf( "Flags:    %02X\n", bs->b_Flags );
  printf( "Use:      %02X\n", bs->b_UseCode );
  printf( "MinTerms: %02X\n", bs->b_MinTerms );
  printf( "A Shift:  %02X\n", bs->b_AShift );
  printf( "B Shift:  %02X\n", bs->b_BShift );
  printf( "FWM:      %04X\n", bs->b_AFWM );
  printf( "LWM:      %04X\n", bs->b_ALWM );
  printf( "A Ptr:    %p\n",   bs->b_aptr );
  printf( "B Ptr:    %p\n",   bs->b_bptr );
  printf( "C Ptr:    %p\n",   bs->b_cptr );
  printf( "D Ptr:    %p\n",   bs->b_dptr );
  printf( "A Mod:    %ld\n",  bs->b_amod );
  printf( "B Mod:    %ld\n",  bs->b_bmod );
  printf( "C Mod:    %ld\n",  bs->b_cmod );
  printf( "D Mod:    %ld\n",  bs->b_dmod );
  printf( "Width:    %d\n",   bs->b_Width );
  printf( "Height:   %d\n",   bs->b_Height );
}
  zflag = DMAF_BLTNZERO;
  
  adat = bs->b_adat;
  atmp = bs->b_adat;
  bdat = bs->b_bdat;
  cdat = bs->b_cdat;

//	printf("%s:%d\n",__FUNCTION__,__LINE__);
  
  if( bs->b_LineMode )
  {
    BOOL singlemode = (bs->b_BLTCON1&0x0002) ? 0x0000 : 0xffff;
    uint32 singlemask = 0xffff;
    
	printf("%s:%d\n",__FUNCTION__,__LINE__);

    // This is roughly based on the MESS source code (machine/amiga.c)
    wc = bs->b_Height;
    while( wc-- )
    {
      uint16 abc0, abc1, abc2, abc3;
      uint32 tempa, tempb;
      int32  dx, dy;
      
      if( bs->b_UseCode & USE_C ) cdat = *bs->b_cptr;
      
      tempa = adat >> bs->b_AShift;
      
      tempa &= singlemask;
      singlemask &= singlemode;
      
      tempb = -(( bdat >> bs->b_BShift ) & 1);
      
      abc0 = ((tempa>>1)&0x4444) | (tempb&0x2222) | ((cdat>>3)&0x1111);
      abc1 = ((tempa>>0)&0x4444) | (tempb&0x2222) | ((cdat>>2)&0x1111);
      abc2 = ((tempa<<1)&0x4444) | (tempb&0x2222) | ((cdat>>1)&0x1111);
      abc3 = ((tempa<<2)&0x4444) | (tempb&0x2222) | ((cdat>>0)&0x1111);
      

	printf("%s:%d\n",__FUNCTION__,__LINE__);

      ddat = 0;
      for( i=0; i<4; i++ )
      {
        uint32 bit;
        
        ddat <<= 4;
        
        bit = (bs->b_BLTCON0>>(abc0>>12)) & 1;
        abc0 <<= 4;
        ddat |= bit << 3;
        
        bit = (bs->b_BLTCON0>>(abc1>>12)) & 1;
        abc1 <<= 4;
        ddat |= bit << 2;
        
        bit = (bs->b_BLTCON0>>(abc2>>12)) & 1;
        abc2 <<= 4;
        ddat |= bit << 1;
        
        bit = (bs->b_BLTCON0>>(abc3>>12)) & 1;
        abc3 <<= 4;
        ddat |= bit << 0;
      }
      
      zflag |= ddat;
      
      if( bs->b_UseCode & USE_D )
        *bs->b_dptr = ddat;
      
      if( bs->b_BLTCON1 & 0x0010 )
      {
        dx = (bs->b_BLTCON1 & 0x0004) ? -1 : 1;
        dy = 0;
      } else {
        dx = 0;
        dy = (bs->b_BLTCON1 & 0x0004) ? -1 : 1;
      }
      
      if( !(bs->b_BLTCON1 & 0x0040 ) )
      {
        bs->b_aptr += bs->b_amod;
        
        if( bs->b_BLTCON1 & 0x0010 )
          dy = (bs->b_BLTCON1 & 0x0008) ? -1 : 1;
        else
          dx = (bs->b_BLTCON0 & 0x0008) ? -1 : 1;
      } else {
        bs->b_aptr += bs->b_bmod;
      }
      
      if( dx )
      {
        tempa = bs->b_BLTCON0 + (dx << 12);
        bs->b_BLTCON0 = tempa;
        
        if( tempa & 0x10000 )
        {
          bs->b_cptr += dx;
          bs->b_dptr += dx;
        }
      }
      
      if( dy )
      {
        bs->b_cptr += dy * bs->b_cmod;
        bs->b_dptr += dy * bs->b_cmod;
        singlemask = 0xffff;
      }
      
      bs->b_BLTCON1 = (bs->b_BLTCON1 & ~0x0040) | ((((uint32)bs->b_aptr)>>9)&0x0040);
      bs->b_BLTCON1 += 0x1000;
    }
    
    // Finished!
    bs->b_cdat = cdat;
    bs->b_Busy = 0;
    bs->b_INTREQ |= INTF_BLIT;
    bs->b_DMACON |= zflag;
    if( ( !bs->b_CausePending ) &&
        ( ( bs->b_INTREQ & bs->b_INTENA ) != 0 ) )
    {
      bs->b_CausePending = TRUE;
      Cause( &bs->b_SoftInt );
    }
    return;
  }
 
  // Shift amounts  
  aso = bs->b_AShift;
  asi = 16-aso;
  bso = bs->b_BShift;
  bsi = 16-bso;
  
  // Initial shift data
  asdat = 0;
  astmp = 0;
  bsdat = 0;

  // Descending mode?
  if( bs->b_Flags & 2 )
  {
	if (blitzenDebug == 1) printf("Descending mode\n");

    // Do the blit
    while( bs->b_Height )
    {
      wc = bs->b_Width;
      while( wc )
      {
        tmp = adat;
        if( bs->b_UseCode & USE_A )
          tmp   = *(bs->b_aptr--);  // Get A channel data
        adat  = asdat|(tmp<<aso);
        asdat = tmp>>asi;
        if( wc == bs->b_Width ) tmp &= bs->b_AFWM; // Apply first & last word masks
        if( wc == 1 )           tmp &= bs->b_ALWM;
        atmp  = astmp|(tmp<<aso); // Calculate shifted A data
        astmp = tmp>>asi;         // Remember shifted out bits for next time

        tmp = bdat;
        if( bs->b_UseCode & USE_B )
          tmp   = *(bs->b_bptr--);  // Get B channel data
        bdat  = bsdat|(tmp<<bso); // Calculated shifted B data
        bsdat = tmp>>bsi;         // Remember shifted out bits for next time

        if( bs->b_UseCode & USE_C ) cdat = *(bs->b_cptr--);    

        // Apply minterms
        ddat = 0;
        for( i=0x8000; i>0; i>>=1 )
        {
          tmp = 0;
          if( cdat&i ) tmp |= 1;
          if( bdat&i ) tmp |= 2;
          if( adat&i ) tmp |= 4;
          if( bs->b_MinTerms & (1<<tmp) ) ddat |= i;
        }
          
        // Fill?
        tmp = 0;
        switch( bs->b_FillMode )
        {
          case 1:
            // Inclusive
            for( i=1; i<0x10000; i<<=1 )
            {
              if( ddat&i )
              {
                bs->b_FillBit ^= 1;
                tmp |= i;
              } else {
                if( bs->b_FillBit ) tmp |= i;
              }
            }
            ddat = tmp;
            break;
          
          case 2:
          case 3:
            // Exclusive
            for( i=1; i<0x10000; i<<=1 )
            {
              if( ddat&i )
                bs->b_FillBit ^= 1;
              if( bs->b_FillBit ) tmp |= i;
            }
            ddat = tmp;
            break;
        }
        
        if( ddat ) zflag = 0;
        
        if( bs->b_UseCode & USE_D )
          *(bs->b_dptr--) = ddat;

        wc--;
      }

      bs->b_Height--;
      if( bs->b_UseCode & USE_A ) bs->b_aptr -= bs->b_amod;
      if( bs->b_UseCode & USE_B ) bs->b_bptr -= bs->b_bmod;
      if( bs->b_UseCode & USE_C ) bs->b_cptr -= bs->b_cmod;
      if( bs->b_UseCode & USE_D ) bs->b_dptr -= bs->b_dmod;
    }

    // Finished!
    bs->b_adat = adat;
    bs->b_bdat = bdat;
    bs->b_cdat = cdat;
    bs->b_Busy = 0;
    bs->b_INTREQ |= INTF_BLIT;
    bs->b_DMACON |= zflag;
    if( ( !bs->b_CausePending ) &&
        ( ( bs->b_INTREQ & bs->b_INTENA ) != 0 ) )
    {
      bs->b_CausePending = TRUE;
      Cause( &bs->b_SoftInt );
    }
    return;
  }


if (blitzenDebug == 1) printf("std mode\n");

if (blitzenDebug == 1) printf("USE_A %d\n",bs->b_UseCode & USE_A);
if (blitzenDebug == 1) printf("USE_B %d\n",bs->b_UseCode & USE_B);
if (blitzenDebug == 1) printf("USE_C %d\n",bs->b_UseCode & USE_C);
if (blitzenDebug == 1) printf("USE_D %d\n",bs->b_UseCode & USE_D);

  // Do the blit
  while( bs->b_Height )
  {
    wc = bs->b_Width;
    while( wc )
    {
      tmp = adat;
      if( bs->b_UseCode & USE_A )
        tmp   = *(bs->b_aptr++);  // Get A channel data
      adat  = asdat|(tmp>>aso);
      asdat = tmp<<asi;
      if( wc == bs->b_Width ) tmp &= bs->b_AFWM; // Apply first & last word masks
      if( wc == 1 )           tmp &= bs->b_ALWM;
      atmp  = astmp|(tmp>>aso); // Calculate shifted A data
      astmp = tmp<<asi;         // Remember shifted out bits for next time

      tmp = bdat;      
      if( bs->b_UseCode & USE_B )
        tmp   = *(bs->b_bptr++);  // Get B channel data
      bdat  = bsdat|(tmp>>bso); // Calculated shifted B data
      bsdat = tmp<<bsi;         // Remember shifted out bits for next time

      if( bs->b_UseCode & USE_C ) cdat = *(bs->b_cptr++);
      
      // Apply minterms
      ddat = 0;
      for( i=0x8000; i>0; i>>=1 )
      {
        tmp = 0;
        if( cdat&i ) tmp |= 1;
        if( bdat&i ) tmp |= 2;
        if( atmp&i ) tmp |= 4;
        if( bs->b_MinTerms & (1<<tmp) ) ddat |= i;
      }
      
      if( ddat ) zflag = 0;

      if( bs->b_UseCode & USE_D )
        *(bs->b_dptr++) = ddat;

      wc--;
    }

    bs->b_Height--;
    if( bs->b_UseCode & USE_A ) bs->b_aptr += bs->b_amod;
    if( bs->b_UseCode & USE_B ) bs->b_bptr += bs->b_bmod;
    if( bs->b_UseCode & USE_C ) bs->b_cptr += bs->b_cmod;
    if( bs->b_UseCode & USE_D ) bs->b_dptr += bs->b_dmod;
  }

  // Finished!
  bs->b_adat = adat;
  bs->b_bdat = bdat;
  bs->b_cdat = cdat;
  bs->b_Busy = 0;
  bs->b_INTREQ |= INTF_BLIT;
  bs->b_DMACON |= zflag;
  if( ( !bs->b_CausePending ) &&
      ( ( bs->b_INTREQ & bs->b_INTENA ) != 0 ) )
  {
    bs->b_CausePending = TRUE;
    Cause( &bs->b_SoftInt );
  }
}



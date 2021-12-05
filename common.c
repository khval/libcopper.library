
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <hardware/custom.h>

#define __common_c
#include "common.h"

union reg_u D0,D1,D2,D3,D4,D5,D6,D7;
union reg_u A0,A1,A2,A3,A4,A5,A6,A7;

union reg_u *regArray[]={&D0,&D1,&D2,&D3,&D4,&D5,&D6,&D7,&A0,&A1,&A2,&A3,&A4,&A5,&A6,&A7};

union reg_u *emu_stack_ptr = NULL;

void movem_push(uint32 reg_low, uint32 reg_hi)
{
	uint32 i;
	uint32 size = reg_hi - reg_low + 1;

	for (i=0;i<size;i++)
	{
		(*emu_stack_ptr++).b32= (*regArray[reg_low+i]).b32;
	}
}

void movem_pop(uint32 reg_low, uint32 reg_hi)
{
	uint32 i;
	uint32 size = reg_hi - reg_low + 1;

	for (i=0;i<size;i++)
	{
		(*regArray[reg_hi-i]).b32 = (*(--emu_stack_ptr)).b32;
	}
}

int stack_size( union reg_u *stack )
{
	return emu_stack_ptr - stack;
}

void dump_stack( union reg_u *stack )
{
	int i;
	int cnt = emu_stack_ptr - stack;

	for (i=0;i<cnt;i++)
	{
		printf("%-3d: %08x\n",i,stack[i].b32);
	}
}


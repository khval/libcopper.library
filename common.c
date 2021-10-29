
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <hardware/custom.h>

#include "common.h"

union reg_u *emu_stack_ptr = NULL;

void movem_push(uint32 reg_low, uint32 reg_hi)
{
	uint32 i;
	uint32 size = reg_hi - reg_low + 1;

	for (i=0;i<size;i++) *emu_stack_ptr++= *regArray[reg_low+i];
}

void movem_pop(uint32 reg_low, uint32 reg_hi)
{
	uint32 i;
	uint32 size = reg_hi - reg_low + 1;

	for (i=0;i<size;i++)
	{
		emu_stack_ptr--;
		*regArray[reg_hi-i] = *emu_stack_ptr;
	}
}



#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/layers.h>

#include "render.h"


struct XYSTW_Vertex3D { 
	float x, y; 
	float s, t, w; 
}; 

typedef struct CompositeHookData_s {
	struct BitMap *sourceBitMap; // The source bitmap
	int32 sourceX,sourceY;
	int32 sourceWidth, sourceHeight; // The source dimensions
	int32 offsetX, offsetY; // The offsets to the destination area relative to the window's origin
	int32 destWidth, destHeight;
	int32 scaleX, scaleY; // The scale factors
	uint32 retCode; // The return code from CompositeTags()
} CompositeHookData;


bool checkMouse(struct Window *win, ULONG bcode)
{
	BOOL buttonClicked = false;
	struct IntuiMessage *msg;
	ULONG win_mask = 1 << win -> UserPort ->mp_SigBit ;
	ULONG sig = Wait( win_mask | SIGBREAKF_CTRL_C);

	do
	{
		msg = (struct IntuiMessage *) GetMsg( win -> UserPort );

		if (msg)
		{
			switch (msg -> Class)
			{
				case IDCMP_MOUSEBUTTONS :
						if ( msg -> Code == (( IECODE_LBUTTON -1 + bcode ) | IECODE_UP_PREFIX)) buttonClicked = true;
						break;
			}
			ReplyMsg( (struct Message *) msg);
		}
	} while (msg);

	return buttonClicked;
}

void WaitLeftMouse(struct Window *win)
{
	bool running = true;
	struct IntuiMessage *msg;
	ULONG class, code;

	if (win -> UserPort)
	{
		ULONG win_mask = 1 << win -> UserPort ->mp_SigBit ;
	
 		do
		{
			ULONG sig = Wait( win_mask | SIGBREAKF_CTRL_C);
			
			if (sig & win_mask) if (checkMouse(win, 1)) running = false;

		} while (running);
	}
	else printf("no user port for wait left mouse\n");
}

static ULONG compositeHookFunc(
			struct Hook *hook, 
			struct RastPort *rastPort, 
			struct BackFillMessage *msg)
 {

	CompositeHookData *hookData = (CompositeHookData*)hook->h_Data;

	hookData->retCode = CompositeTags(
		COMPOSITE_Src, 
			hookData->sourceBitMap, 
			rastPort->BitMap,

		COMPTAG_SrcX,      abs(hookData->sourceX),
		COMPTAG_SrcY,      abs(hookData->sourceY),
		COMPTAG_SrcWidth,   hookData->sourceWidth,
		COMPTAG_SrcHeight,  hookData->sourceHeight,

		COMPTAG_ScaleX, 	hookData->scaleX,
		COMPTAG_ScaleY, 	hookData->scaleY,

		COMPTAG_OffsetX,    msg->Bounds.MinX - (msg->OffsetX - msg->Bounds.MinX),
		COMPTAG_OffsetY,    msg->Bounds.MinY - (msg->OffsetY - msg->Bounds.MinY),

		COMPTAG_DestX,      msg->Bounds.MinX,
		COMPTAG_DestY,      msg->Bounds.MinY,

		COMPTAG_DestWidth, hookData->destWidth,
		COMPTAG_DestHeight, hookData->destHeight ,

		COMPTAG_Flags,      COMPFLAG_SrcFilter | COMPFLAG_IgnoreDestAlpha | COMPFLAG_HardwareOnly,
		TAG_END);

	return 0;
}

void comp_window_update( struct BitMap *bitmap, struct Window *win)
{
	struct Hook hook;
	static CompositeHookData hookData;
	struct Rectangle rect;
	register struct RastPort *RPort = win->RPort;

	printf("%s:%d\n",__FUNCTION__,__LINE__);

 	rect.MinX = win->BorderLeft;
 	rect.MinY = win->BorderTop;
 	rect.MaxX = win->Width - win->BorderRight ;
 	rect.MaxY = win->Height - win->BorderBottom ;

	hook.h_Entry = (HOOKFUNC) compositeHookFunc;
	hook.h_Data = &hookData;

	hookData.sourceBitMap = bitmap;

	hookData.sourceX = -GetBitMapAttr(bitmap,BMA_ACTUALWIDTH);
	hookData.sourceY =- bitmap -> Rows;

	hookData.sourceWidth =  GetBitMapAttr(bitmap,BMA_ACTUALWIDTH);
	hookData.sourceHeight = bitmap -> Rows;

	printf("source w %d,h %d\n",hookData.sourceWidth,hookData.sourceHeight);

	hookData.offsetX = win->BorderLeft;
	hookData.offsetY = win->BorderTop;
	hookData.retCode = COMPERR_Success;

 	hookData.destWidth = rect.MaxX - rect.MinX + 1;
 	hookData.destHeight = rect.MaxY - rect.MinY + 1;

	hookData.scaleX = COMP_FLOAT_TO_FIX((float) hookData.destWidth / (float) hookData.sourceWidth);
	hookData.scaleY = COMP_FLOAT_TO_FIX((float) hookData.destHeight / (float) hookData.sourceHeight);

	LockLayer(0, RPort->Layer);
	DoHookClipRects(&hook, win->RPort, &rect);
	UnlockLayer( RPort->Layer);
}

void dump_copper(uint32 *copperList)
{
	union cop *ptr;
	const char *cmd;

	printf("------------ dump_copper -----------------\n");

	ptr = (union cop *) copperList;

	for (;ptr -> d32 != 0xFFFFFFFE;ptr++)
	{
		switch (ptr -> d32 & 0x00010001)
		{
			case 0x00000000: 
			case 0x00000001:	cmd = "Move" ; break;
			case 0x00010000:	cmd = "Wait" ; break;
			case 0x00010001:	cmd = "Skip" ; break;
		}

		printf("%-8s: %04x,%04x\n", cmd, ptr -> d16.a , ptr -> d16.b ); 
	}

	printf("%-8s: %04x,%04x\n", "END",  0xFFFF , 0xFFFE ); 
}



#include <stdbool.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>


bool checkMouse(struct Window *win, ULONG bcode)
{
	BOOL ret = false;
	struct IntuiMessage *msg;
	ULONG code;
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
						ret = msg -> Code == (( IECODE_LBUTTON -1 + bcode ) | IECODE_UP_PREFIX);
						break;
			}
			ReplyMsg( (struct Message *) msg);
		}
	} while (msg);

	return ret;
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


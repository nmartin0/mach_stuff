/* present messages in a Stop Alert */

#include <Resources.h>
#include <Dialogs.h>

#include "AlertMsg.h"

#define resID_cstr_MSGS		128
#define resID_ALRT_STOP		128

typedef struct {
	char msg[64];
} mstr;

static Ptr GetMsg(n)
{
	Handle h;
	mstr *m;
	
	h = GetResource('mstr', resID_cstr_MSGS);
	m = (mstr *)(*h);
	return ((Ptr)&m[n]);
}

void AlertMsg(msg0, msg1, msg2, msg3)
Ptr msg0, msg1, msg2, msg3;
{

	if ((int)msg0 < 128) msg0 = GetMsg(msg0);
	if ((int)msg1 < 128) msg1 = GetMsg(msg1);
	if ((int)msg2 < 128) msg2 = GetMsg(msg2);
	if ((int)msg3 < 128) msg3 = GetMsg(msg3);

	paramtext(msg0, msg1, msg2, msg3);	
	StopAlert(resID_ALRT_STOP, 0);
}

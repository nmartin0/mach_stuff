/* this INIT starts up the Mach kernel */

#include <Fonts.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Resources.h>
#include <Memory.h>
#include <Retrace.h>

#include "types.h"
#include "conf.h"
#include "INIT_dialog.h"
#include "LoadMach.h"
#include "AlertMsg.h"
#include "GestaltMach.h"

INIT_main()
{
	ConfHandle conf;
	DialogVars d;

	/* insure that Mach is not already running */
	if (!NATIVE()) return;
	/* do nothing if enable bit is not set */
	if (!(conf = (ConfHandle)GetResource('conf', resID_conf))) return;
	if (!((*conf)->Flags & CONF_ENABLE)) return;
	/* insure that this is the right type of Macintosh */
	if (GestaltMach()) {
		AlertMsg(MSG_NOT_THIS_MAC, 0, 0, 0);
		return;
	}
	/* run the INIT dialog, do not continue if it returns non-zero */
	if (!INIT_dialog(&d)) LoadMach(&d);
	DisposDialog(d.dp);
}

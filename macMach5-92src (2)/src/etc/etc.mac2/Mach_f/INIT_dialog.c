#include <Dialogs.h>
#include <Resources.h>
#include <Controls.h>
#include <Memory.h>
#include <OSEvents.h>
#include <Fonts.h>
#include <Retrace.h>

#include "types.h"
#include "conf.h"
#include "INIT_dialog.h"
#include "string.h"
#include "AlertMsg.h"
#include "disklabel.h"
#include "device.h"
#include "resID.h"
#include "LoadMach.h"

/* items from the INIT dialog */
#define iCount		3
#define	iBar		5
#define iCancel		6
#define iPause		7
#define iPrimary	8
#define iOutline1	9
#define iAlternate	10
#define iOutline2	11
#define iSingle		12
#define iDebug		13
#define iVersion	15
#define iReturn		128 /* not really a dialog item, user pressed return key */

static pascal void CounterTask()
{
	FlushEvents(app1Mask, 0);
	PostEvent(app1Evt, 0);
}

/* draw function for the progress bar indicator */
static void DrawBar(r, num, denom)
Rect *r;
unsigned long num, denom;
{
	if (denom) {
		PenSize(1, 1);
		FrameRect(r);
		r->right = r->left + (((r->right - r->left) * num) / denom);
		PaintRect(r);
	}
}

/* draw function for the count indicator */
static void DrawCount(r, count)
Rect *r;
int count;
{
	char c;
	FontInfo FI;
	int height, width, left, bottom;

	c = '0' + count;	
	GetFontInfo(&FI);
	height = FI.ascent + FI.leading - FI.descent;
	width = CharWidth(c);
	EraseRect(r);
	/*
	PenSize(1, 1);
	FrameRect(r);
	*/
	left = r->left + (((r->right - r->left) >> 1) - (width >> 1));
	bottom = r->top + (((r->bottom - r->top) >> 1) + (height >> 1));
	MoveTo(left, bottom);
	DrawChar(c);
}

/* draw function for user items iCount, iBar and iOutline */
static pascal void DrawUserItem(wp, item)
WindowPeek wp;
short item;
{
	DialogVars *d = (DialogVars *)wp->refCon;
	Handle h;
	Rect r;
	short itype;
	
	GetDItem((DialogPtr)wp, item, &itype, &h, &r);
	switch (item) {
		case iCount:
			DrawCount(&r, d->count);
			break;
		case iBar:
			DrawBar(&r, d->num, d->denom);
			break;
		case iOutline1:
		case iOutline2:
			PenSize(3, 3);
			InsetRect(&r, -4, -4);
			FrameRoundRect(&r, 16, 16);
			break;
	}
}

/* set device name control title */
static void SetCtlDeviceName(h, LoadDev, LoadFile, RootDev)
ControlHandle h;
dev_t LoadDev;
char *LoadFile;
dev_t RootDev;
{
	Str255 title;
	char *n;

	/* the control title is a pascal string */
	n = &title[1];
	/* the load device, null for HFS, otherwise /dev/sd?a */
	/* note: ramdisk load device => HFS */
	if (LoadDev == Device_SystemVolume) *n = 0;
	else devicename(LoadDev, n);
	/* the load file */
	strcat(n, LoadFile);
	/* the root device as an argument to the load file */
	strcat(n, getstr(STR_SPACE));
	devicename(RootDev, &n[strlen(n)]);
	/* the control title is a pascal string */
	title[0] = strlen(n);
	SetCTitle(h, title);
}

/* process an event during the INIT dialog */
static int ProcessEvent(d, conf, ritem)
DialogVars *d;
ConfHandle conf;
short *ritem;
{
	short itype;
	Handle h;
	Rect r;
	GrafPtr s;
	int result;

	GetPort(&s);
	SetPort(d->dp);
	switch (*ritem) {
		case iReturn:
			result = 0;
			*ritem = d->DefaultItem;
		case iPrimary:
			result = d->DisablePrimary ? -1 : 0;
			break;
		case iAlternate:
			result = d->DisableAlternate ? -1 : 0;
		case iCancel:
			result = 0;
			break;
		case iSingle:
			(*conf)->Flags ^= CONF_SINGLE;
			GetDItem(d->dp, *ritem, &itype, &h, &r);
			SetCtlValue((ControlHandle)h, ((*conf)->Flags & CONF_SINGLE) ? 1 : 0);
			result = -1;
			break;
		case iDebug:
			(*conf)->Flags ^= CONF_DEBUG;
			GetDItem(d->dp, *ritem, &itype, &h, &r);
			SetCtlValue((ControlHandle)h, ((*conf)->Flags & CONF_DEBUG) ? 1 : 0);
			result = -1;
			break;
		case iPause:
			d->count = -1;
			GetDItem(d->dp, *ritem, &itype, &h, &r);
			HiliteControl((ControlHandle)h, 255);
			result = -1;
			break;
		case iCount:
			if (d->count-- == 0) {
				*ritem = d->DefaultItem;
				result = 0;
				break;
			}
			if (d->count >= 0) {
				d->vtask.qType = vType;
				d->vtask.vblAddr = CounterTask;
				d->vtask.vblCount = 60;
				d->vtask.vblPhase = 0;
				VInstall((QElemPtr)&d->vtask);
				GetDItem(d->dp, *ritem, &itype, &h, &r);
				DrawCount(&r, d->count);
			}
			result = -1;
			break;
	}
	SetPort(s);
	return result;
}

/* ModalDialog filter function -- enter and return => iPrimary, escape => iCancel */
static pascal Boolean Filter(dp, ev, ritem)
DialogPeek dp;
EventRecord *ev;
short *ritem;
{
	EventRecord e1;

	if ((ev->what == keyDown) &&
		!(ev->modifiers&(cmdKey | shiftKey | alphaLock | optionKey | controlKey)))
		switch ((ev->message&0xff00) >> 8) {
			case 0x4c:	/* enter */
			case 0x24:	/* return */
				*ritem = iReturn;
				return true;
			case 0x35:	/* escape */
				*ritem = iCancel;
				return true;
		}
	if (GetOSEvent(app1Mask, &e1) == true) {
		*ritem = iCount;
		return true;
	}
	return false;
}

/* setup a dialog item */
static void InitItem(dp, conf, item)
DialogPtr dp;
ConfHandle conf;
short item;
{
	short itype;
	Rect r;
	Handle h;
	extern char *GetVersion();

	GetDItem(dp, item, &itype, &h, &r);
	switch (item) {
		case iCount:
		case iBar:
		case iOutline1:
		case iOutline2:
			h = (Handle)DrawUserItem;
			SetDItem(dp, item, itype, h, &r);
			break;
		case iSingle:
			SetCtlValue((ControlHandle)h, ((*conf)->Flags & CONF_SINGLE) ? 1 : 0);
			break;
		case iDebug:
			SetCtlValue((ControlHandle)h, ((*conf)->Flags & CONF_DEBUG) ? 1 : 0);
			break;
		case iPrimary:
			SetCtlDeviceName((ControlHandle)h,
				(*conf)->LoadDev1, (*conf)->LoadFile1, (*conf)->RootDev1);
			break;
		case iAlternate:
			SetCtlDeviceName((ControlHandle)h,
				(*conf)->LoadDev2, (*conf)->LoadFile2, (*conf)->RootDev2);
			break;
	}
}

/* run the Mach INIT dialog, return non-zero if cancelled */
int INIT_dialog(d)
DialogVars *d;
{
	short item;
	short itype;
	Rect r;
	Handle h;
	ConfHandle conf;

	conf = (ConfHandle)GetResource('conf', resID_conf);

	/* initial values for dialog variables */
	d->num = d->denom = 0;

	InitFonts();
	InitWindows();
	TEInit();
	InitDialogs(0);
	InitCursor();

	d->dp = GetNewDialog(resID_DLOG_INIT, 0, (WindowPtr)-1);
	((WindowPeek)d->dp)->refCon = (long)d;

	/* use Chicago 12 font for everything */
	TextFont(0);
	TextSize(12);
	TextFace(normal);

	InitItem(d->dp, conf, iPrimary);
	InitItem(d->dp, conf, iAlternate);
	InitItem(d->dp, conf, iSingle);
	InitItem(d->dp, conf, iDebug);
	InitItem(d->dp, conf, iCount);
	InitItem(d->dp, conf, iBar);
	
	/* determine default boot source, disable impossible selections */
	d->DisablePrimary = 0;
	d->DisableAlternate = 0;
	if (CheckLoad((*conf)->LoadDev1, (*conf)->LoadFile1)) {
		d->DefaultItem = iPrimary;
		if (!CheckLoad((*conf)->LoadDev2, (*conf)->LoadFile2)) d->DisableAlternate++;
	}
	else if (CheckLoad((*conf)->LoadDev2, (*conf)->LoadFile2)) {
		d->DefaultItem = iAlternate;
		d->DisablePrimary++;
	}
	else {
		d->DefaultItem = iCancel;
		d->DisablePrimary++;
		d->DisableAlternate++;
	}
	if (d->DefaultItem == iPrimary) InitItem(d->dp, conf, iOutline1);
	if (d->DefaultItem == iAlternate) InitItem(d->dp, conf, iOutline2);
	if (d->DisablePrimary) {
		GetDItem(d->dp, iPrimary, &itype, &h, &r);
		HiliteControl((ControlHandle)h, 255);
	}
	if (d->DisableAlternate) {
		GetDItem(d->dp, iAlternate, &itype, &h, &r);
		HiliteControl((ControlHandle)h, 255);
	}

	FlushEvents(everyEvent, 0);

	d->count = 9;
	d->vtask.qType = vType;
	d->vtask.vblAddr = CounterTask;
	d->vtask.vblCount = 60;
	d->vtask.vblPhase = 0;
	VInstall((QElemPtr)&d->vtask);

	ShowWindow(d->dp);

	do ModalDialog(Filter, &item); while (ProcessEvent(d, conf, &item));

	VRemove((QElemPtr)&d->vtask);

	/* dim the cancel button */
	GetDItem(d->dp, iCancel, &itype, &h, &r);
	HiliteControl((ControlHandle)h, 255);
	/* dim the pause button */
	GetDItem(d->dp, iPause, &itype, &h, &r);
	HiliteControl((ControlHandle)h, 255);
	/* dim the single-user box */
	GetDItem(d->dp, iSingle, &itype, &h, &r);
	HiliteControl((ControlHandle)h, 255);
	/* dim the debug box */
	GetDItem(d->dp, iDebug, &itype, &h, &r);
	HiliteControl((ControlHandle)h, 255);

	/* user pressed default boot source button */
	if (item == iPrimary) {
		/* set flag so that LoadMach() will use the primary boot source */
		(*conf)->Flags &= ~CONF_ALTERNATE;
		/* keep the primary boot source button "pressed" */
		GetDItem(d->dp, iPrimary, &itype, &h, &r);
		HiliteControl((ControlHandle)h, 1);
		/* dim the alternate boot source button */
		GetDItem(d->dp, iAlternate, &itype, &h, &r);
		HiliteControl((ControlHandle)h, 255);
		return 0;
	}

	/* user pressed alternate boot source button */
	if (item == iAlternate) {
		/* set flag so that LoadMach() will use the alternate boot source */
		(*conf)->Flags |= CONF_ALTERNATE;
		/* dim the primary boot source button */
		GetDItem(d->dp, iPrimary, &itype, &h, &r);
		HiliteControl((ControlHandle)h, 255);
		/* keep the alternate boot source button "pressed" */
		GetDItem(d->dp, iAlternate, &itype, &h, &r);
		HiliteControl((ControlHandle)h, 1);
		return 0;
	}

	/* cancel */
	return -1;

} /* INIT_dialog */

/* draw the dialog bar at the specified position */
ProgressBar(d, num, denom)
DialogVars *d;
unsigned long num, denom;
{
	short itype;
	Handle h;
	Rect r;
	GrafPtr s;
		
	GetDItem(d->dp, iBar, &itype, &h, &r);
	GetPort(&s);
	SetPort(d->dp);
	if (!num || !denom || !d->denom || (num/denom < d->num/d->denom))
	  EraseRect(&r);
	DrawBar(&r, d->num = num, d->denom = denom);
	SetPort(s);
}

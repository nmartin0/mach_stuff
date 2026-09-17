/* This control panel is responsible for manipulating values in the 'conf' resource. */

#define	__ALLNU__

#include <Dialogs.h>
#include <Events.h>
#include <Devices.h>
#include <Resources.h>
#include <Controls.h>
#include <Menus.h>
#include <SysEqu.h>
#include <Memory.h>
#include <Quickdraw.h>

#include "types.h"
#include "conf.h"
#include "disklabel.h"
#include "device.h"
#include "resID.h"

#define TRUE true
#define FALSE false

/* items from the cdev dialog */
#define iVersion			5
#define iEnableBox			6
#define iTitleTimeOffset	8
#define iTimeOffset			9
#define iTitleRootDev1		10
#define iRootDev1			11
#define iTitleLoadDev1		12
#define iLoadDev1			13
#define iLoadFile1			15
#define iTitleRootDev2		16
#define iRootDev2			17
#define iTitleLoadDev2		18
#define iLoadDev2			19
#define iLoadFile2			21
#define iBar				22

typedef struct {
	DialogPtr	dlgPtr;
	int			dlgItems;
	ConfHandle	conf;
} CdevStorage, **CdevHandle;

typedef struct {
	unsigned char	major;
	unsigned char	minor;
	unsigned char	stage;
	unsigned char	release;
	unsigned short	country;
	unsigned char	vstring[0];
} *vers_ptr_t;
 
static Handle IGetHandle();

#define IGetCtlHand(h, i)	((ControlHandle)IGetHandle(h, i))

pascal long cdev_main(message, item, numItems, CPanelID, theEvent, cdevValue, CPDialog)
short message;
short item;
short numItems;
short CPanelID;
EventRecord *theEvent;
long cdevValue;
DialogPtr CPDialog;
{
	Boolean storageExpected;
	CdevHandle H;

	storageExpected = !((message == initDev) || (message == macDev));
	if (storageExpected && ((cdevValue == 0) || (cdevValue == cdevUnset)))
		cdevValue = 0;
	else {
		H = (CdevHandle)cdevValue;
		switch (message) {
			case macDev:
				cdevValue = 1;
				break;
			case initDev:
				cdevValue = InitCdev(CPDialog, numItems);
				H = (CdevHandle)cdevValue;
				break;
			case closeDev:
				if (H) {
					Str255 vol;
					short vr;
					ChangedResource((Handle)(*H)->conf);
					WriteResource((Handle)(*H)->conf);
					GetVol(vol, &vr);
					FlushVol(vol, vr);
					DisposHandle((Handle)(*H)->conf);
					DisposHandle((Handle)H);
					cdevValue = 0;
					H = 0;
				}
				break;
			case hitDev:
				HitCdev(H, item - numItems);
				break;
			case updateDev:
				DrawItem(H, iTimeOffset);
				DrawItem(H, iLoadDev1);
				DrawItem(H, iRootDev1);
				DrawItem(H, iLoadDev2);
				DrawItem(H, iRootDev2);
				DrawItem(H, iVersion);
				DrawItem(H, iBar);
				break;
			case nulDev:
				break;
			default:
				break;
		}
		if (H) {
			getitext(IGetHandle(H, iLoadFile1), (*(*H)->conf)->LoadFile1);
			getitext(IGetHandle(H, iLoadFile2), (*(*H)->conf)->LoadFile2);
		}
	}
	return (cdevValue);
}

static long InitCdev(CPDialog, numItems)
DialogPtr CPDialog;
int numItems;
{
	CdevHandle h;

	if (h = (CdevHandle)NewHandle(sizeof (CdevStorage))) {
		(*h)->dlgPtr = CPDialog;
		(*h)->dlgItems = numItems;
		if ((*h)->conf = (ConfHandle)GetResource('conf', resID_conf)) {
			setitext(IGetHandle(h, iLoadFile2), (*(*h)->conf)->LoadFile2);
			SelIText(CPDialog, numItems + iLoadFile2, 0, 32767);
			setitext(IGetHandle(h, iLoadFile1), (*(*h)->conf)->LoadFile1);
			SelIText(CPDialog, numItems + iLoadFile1, 0, 32767);
			SetCtlValue(IGetCtlHand(h, iEnableBox),
				((*(*h)->conf)->Flags & CONF_ENABLE) != 0);
		}
		else {
			DisposHandle((Handle)h);
			h = 0;
		}
	}
	return (long)h;
}

MyPopUpMenu(H, menu, item, title, value)
CdevHandle H;
MenuHandle menu;
short item;
short title;
short *value;
{
	Rect r;
	short result;

	/* insert the pop-up menu */
	InsertMenu(menu, -1);
	/* invert the title */
	IGetRect(H, title, &r);
	InvertRect(&r);
	/* check the current menu item */
	CheckItem(menu, *value, 1);
	/* figure out where the pop-up menu will go... */
	IGetRect(H, item, &r);
	LocalToGlobal((Point *)&r);
	/* run the pop-up menu selection */
	result = PopUpMenuSelect(menu, r.top + 1, r.left + 1, *value) & 0xFFFF;
	/* uncheck the current menu item */
	CheckItem(menu, *value, 0);
	/* set a new value if one was selected */
	if (result) *value = result;
	/* restore the title */
	IGetRect(H, title, &r);
	InvertRect(&r);
	/* redraw the item */
	DrawItem(H, item);
	/* remove the pop-up menu */
	DeleteMenu((*menu)->menuID);
}

static HitCdev(H, item)
CdevHandle H;
int item;
{
	MenuHandle m;

	switch (item) {
		case iLoadDev1:
			m = GetMenu(resID_MENU_Device);
			DisableItem(m, Device_ramdisk0a);
			MyPopUpMenu(H, m, item, iTitleLoadDev1, &(*(*H)->conf)->LoadDev1);
			EnableItem(m, Device_ramdisk0a);
			break;
		case iRootDev1:
			m = GetMenu(resID_MENU_Device);
			DisableItem(m, Device_SystemVolume);
			MyPopUpMenu(H, m, item, iTitleRootDev1, &(*(*H)->conf)->RootDev1);
			EnableItem(m, Device_SystemVolume);
			break;
		case iLoadDev2:
			m = GetMenu(resID_MENU_Device);
			DisableItem(m, Device_ramdisk0a);
			MyPopUpMenu(H, m, item, iTitleLoadDev2, &(*(*H)->conf)->LoadDev2);
			EnableItem(m, Device_ramdisk0a);
			break;
		case iRootDev2:
			m = GetMenu(resID_MENU_Device);
			DisableItem(m, Device_SystemVolume);
			MyPopUpMenu(H, m, item, iTitleRootDev2, &(*(*H)->conf)->RootDev2);
			EnableItem(m, Device_SystemVolume);
			break;
		case iTimeOffset:
			m = GetMenu(resID_MENU_TimeOffset);
			MyPopUpMenu(H, m, item, iTitleTimeOffset, &(*(*H)->conf)->TimeOffset);
			break;
		case iEnableBox:
			(*(*H)->conf)->Flags ^= CONF_ENABLE;
			SetCtlValue(IGetCtlHand(H, item),
				((*(*H)->conf)->Flags & CONF_ENABLE) != 0);
			break;
		default:
			break;
	}
}

DrawPopUpItem(r, title)
Rect *r;
char *title;
{
	RgnHandle x;

	PenNormal();
	r->bottom -= 1;
	r->right -= 1;
	EraseRect(r);
	FrameRect(r);
	MoveTo(r->left + 3, r->bottom);
	LineTo(r->right, r->bottom);
	LineTo(r->right, r->top + 3);
	x = NewRgn();
	OpenRgn();
	MoveTo(r->right - 14, r->top + 6);
	LineTo(r->right - 8, r->top + 12);
	LineTo(r->right - 3, r->top + 6);
	LineTo(r->right - 14, r->top + 6);
	CloseRgn(x);
	PaintRgn(x);
	DisposeRgn(x);
	MoveTo(r->left + 15, r->bottom - 5);
	DrawString(title);
	
}

char *GetVersion()
{
	char *s;
	int count;

	s = (*(vers_ptr_t *)GetResource('vers', 1))->vstring;
	for (count = *s++; count; count--) s++;
	return s;
}

static DrawItem(H, item)
CdevHandle H;
int item;
{
	Rect r;
	MenuHandle m;
	Str255 text;

	SetPort((*H)->dlgPtr);
	IGetRect(H, item, &r);
	switch (item) {
		case iVersion:
			SetIText(IGetHandle(H, item), GetVersion());
			break;
		case iTimeOffset:
			m = GetMenu(resID_MENU_TimeOffset);
			GetItem(m, (*(*H)->conf)->TimeOffset, text);
			DrawPopUpItem(&r, text);
			break;
		case iLoadDev1:
			DeviceName((*(*H)->conf)->LoadDev1, text);
			DrawPopUpItem(&r, text);
			break;
		case iRootDev1:
			DeviceName((*(*H)->conf)->RootDev1, text);
			DrawPopUpItem(&r, text);
			break;
		case iLoadDev2:
			DeviceName((*(*H)->conf)->LoadDev2, text);
			DrawPopUpItem(&r, text);
			break;
		case iRootDev2:
			DeviceName((*(*H)->conf)->RootDev2, text);
			DrawPopUpItem(&r, text);
			break;
		case iBar:
			PenNormal();
			FrameRect(&r);
			break;
		default:
			return;
	}
}

static Handle IGetHandle(H, item)
CdevHandle H;
int item;
{
	Handle itemHand;
	Rect itemRect;
	short itemType;
	
	GetDItem((*H)->dlgPtr,
		item + (*H)->dlgItems, &itemType, &itemHand, &itemRect);
	return itemHand;
}

static IGetRect(H, item, itemRect)
CdevHandle H;
int item;
Rect *itemRect;
{
	Handle itemHand;
	short itemType;
	
	GetDItem((*H)->dlgPtr,
		item + (*H)->dlgItems, &itemType, &itemHand, itemRect);
}

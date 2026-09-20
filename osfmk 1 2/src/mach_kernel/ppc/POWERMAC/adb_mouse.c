/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * Copyright 1991-1998 by Apple Computer, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * APPLE COMPUTER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL APPLE COMPUTER BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * MkLinux
 */
/*
 * work original based on adb.c from NetBSD-1.0
 */

/*	$NetBSD: adb.c,v 1.3 1995/06/30 01:23:21 briggs Exp $	*/

/*-
 * Copyright (C) 1994	Bradley A. Grantham
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Bradley A. Grantham.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
							     */
/*
 * This code was modified March 5th 1998 to handle products based on the
 * MacPoint Mouse/Trackball controllers by Fesh!.
 * TurboMouse 5.0 support added by Michael Santos Nov 12th 1998.
 * Apple Trackpad support adapted by Michael Santos from LinuxPPC work
 *  by Benjamin Herrenschmidt.  April 26, 1999.
 *
 * The following products are now handeled as multi button or special mice:
 *
 * CH Products 		- Trackball Pro
 * Contour Design 	- Contour Mouse
 * Hunter Digital	- NoHandsMouse
 * MicroSpeed		- Mouse Deluxe
 * MicroSpeed		- MacTRAC
 * Kensington		- TurboMouse 5.0
 * Apple                - PowerBook Trackpad
 *
 */

#include <mouse.h>
#include <platforms.h>

#include <mach_kdb.h>
#include <kern/spl.h>
#include <machine/machparam.h>
#include <types.h>
#include <device/io_req.h>
#include <device/tty.h>
#include <device/conf.h>
#include <chips/busses.h>
#include <ppc/misc_protos.h>

#include <ppc/POWERMAC/adb.h>

#define MOUSE_DEBUG 0
#define DEBUG_ADBHANDLER 0

#define KEY_OPTION 0x3a
#define KEY_LEFT_ARROW 0x3b
#define KEY_RIGHT_ARROW 0x3c
#define MOUSE_ESCAPE 0x7e
#define KEY_MIDDLE_BUTTON 0x3f
#define KEY_RIGHT_BUTTON 0x40

#define POS(x) ((x) & 0x7f)
#define BUTTON(x) ((x) & 0x80)
/* BUTTON_EXT is an additional button bit in the extended mouse format
 * for bytes 3 and above (first byte being numbered 1) -- Mike Santos
 */
#define BUTTON_EXT(x) (((x) & 0x08) << 4)
#define UP 0x80
#define DOWN 0x00

#define EXTENDEDMOUSE 1

#if EXTENDEDMOUSE
enum {
   HANDLER_ID_MICROSPEED = 0x2F,
   HANDLER_ID_TRACKBALL_PRO = 0x42,
   HANDLER_ID_CONTOUR_MOUSE = 0x66,
   HANDLER_ID_NOHANDS_MOUSE = 0x5F,
   HANDLER_ID_KENSINGTON_TURBOMOUSE50 = 0x32 /* Mike Santos */
};

enum {
   BUTTON_MASK_LEFT = 1 << 0,
   BUTTON_MASK_MIDDLE = 1 << 1,
   BUTTON_MASK_RIGHT = 1 << 2,
   BUTTON_MASK_LEFT_TRACKBALL_PRO = (1 << 2) | (1 << 3),
   BUTTON_MASK_MIDDLE_TRACKBALL_PRO = 1 << 0,
   BUTTON_MASK_RIGHT_TRACKBALL_PRO = 1 << 1
};

/* Enumeration for use in adb_device's mouseType field.  Original handler
 * ID is usually sufficient to distinguish, but may not be if special
 * device handling (such as button mapping) is required.
 */
enum {
   MOUSE_UNKNOWN = 0,
   MOUSE_TURBOMOUSE_5,
   MOUSE_TRACKPAD
};
#endif

struct mouse_state {
   unsigned char xpos;
   unsigned char ypos;
   unsigned char left;
   unsigned char middle;
   unsigned char right;
};

typedef struct mouse_state mouse_state_t;

mouse_state_t last_state = {0, 0, UP, UP, UP}; /* all buttons up */

#if EXTENDEDMOUSE
void set_extended_mouse_mode(int number);
void extended_mouse_adbhandler(int number, unsigned char *buffer, int count);
#endif
void mouse_adbhandler(int number, unsigned char *buffer, int count);
int mouse_probe(caddr_t port, void *ui);
void mouse_attach(struct bus_device *dev);
#if MOUSE_DEBUG
void mouse_report(const char *str, const mouse_state_t *newstate);
#endif
void mouse_do_buttons(int button, int up);
void mouse_identify_probe(int number);
void init_turbomouse(int number);
void init_trackpad(int number);

/* Used to retrieve environment variables passed to Mach from the booter */
const char *getenv(char *);

boolean_t	mouse_initted = FALSE;

caddr_t mouse_std[NMOUSE] = { (caddr_t) 0 };

struct bus_device *mouse_info[NMOUSE];

struct bus_driver	mouse_driver = {
   mouse_probe, 0, mouse_attach, 0, mouse_std, "mouse", mouse_info, 0, 0, 0 };


extern boolean_t vc_xservermode; /* Is the tty accepting mouse input. */
extern struct tty vc_tty;

int
mouse_probe(caddr_t port, void *ui)
{
   adb_request_t	readreg;
   int val;

   if (!adb_hardware)
      return 0;

   return 1;
}


void
mouse_attach(struct bus_device *dev)
{
   struct adb_device	*devp;
   int	i, ret;
   unsigned short	adbreg3;
   spl_t		s;
   const char *env;

   adb_register_handler(ADB_DEV_MOUSE, mouse_adbhandler);

   s = splhigh();
   adb_polling = TRUE;

   devp = adb_devices;
   for (i = 0; i < ADB_DEVICE_COUNT; i++, devp++) {
      if ((devp->a_flags & ADB_FLAGS_PRESENT) == 0)
	 continue;

      if (devp->a_dev_type == ADB_DEV_MOUSE) {
#ifdef NETBSD_EXTENDED_MOUSE

#else
#if EXTENDEDMOUSE
	 unsigned short	orighandler = devp->a_dev_orighandler;

	 /* printf("\ngot a mouse: orig handler is = %02X\n", orighandler); */
	 switch (orighandler) {
	  case HANDLER_ID_MICROSPEED:
	  case HANDLER_ID_TRACKBALL_PRO:
	  case HANDLER_ID_CONTOUR_MOUSE:
	  case HANDLER_ID_NOHANDS_MOUSE:
	    set_extended_mouse_mode(i);
	    adb_register_dev(i, extended_mouse_adbhandler);
	    break;
	  default:
	    adb_set_handler(devp, 4);
	    break;
	 }

	 /* Check for specific mice that need activation sequences or other
	  * special handling
	  */
	 devp->mouseType = MOUSE_UNKNOWN;
	 if (env = getenv("mouse_probe")) {
	    printf("\nProbing mouse at address %d\n", i);
	    mouse_identify_probe(i);
	 }
	 
#else
	 /*
	  * See if the mouse is a three button
	  * mouse.. this is done by setting the
	  * handler id to 4
	  */

	 adb_set_handler(devp, 4);
#endif
#endif
      }
   }
   if (!adb_always_poll)
      adb_polling = FALSE;
   splx(s);

}


#if EXTENDEDMOUSE
void
set_extended_mouse_mode(int number)
{
   adb_request_t req;
   struct adb_device *devp;
   unsigned long reg1 = 1 << 12; /* set bit #12 for extended mouse mode */

   devp = adb_devices;

   adb_init_request(&req);
   ADB_BUILD_CMD2_BUFFER(&req, ADB_PACKET_ADB,
			 (ADB_ADBCMD_WRITE_ADB | (number << 4) | 1),
			 4, (unsigned char *) &reg1);
   adb_send(&req, TRUE);
}


void
mouse_identify_probe(int number) {
   const char *env;
   struct adb_device	*devp;
   adb_request_t req;

   /* This code probes the mouse to get information such as the number
    * of buttons and the manufacturer.  Right now, it just prints it,
    * but it could be used in the future to set the number of buttons,
    * etc.  -- Mike Santos
    */
   devp = adb_devices;
   
   adb_init_request(&req);
   /* Get device information */
   ADB_BUILD_CMD2(&req, ADB_PACKET_ADB,
		  (ADB_ADBCMD_READ_ADB | (number << 4) | 1));
   adb_send(&req, TRUE);
   if (req.a_reply.a_bcount != 8) 
      printf("\nYour mouse doesn't understand register 1 reads; "
	     "it returned %d bytes instead of 8\n"
	     "Don't worry; I just can't identify the particular mouse.\n",
	     req.a_reply.a_bcount);
   else {
      char creator[5];
      short resolution;
      memcpy(creator, req.a_reply.a_buffer, 4);
      creator[4] = '\0';
      memcpy(&resolution, req.a_reply.a_buffer + 4, 2);
      printf("\nYour extended mouse reports:\n"
	     "\tcreator code = \t'%s'\n\tresolution = \t%d units/inch\n"
	     "\tdevice class = \t%s\n\tbuttons = \t%d buttons\n", creator,
	     resolution,
	     ((req.a_reply.a_buffer[6] == 0) ? "tablet" :
	      ((req.a_reply.a_buffer[6] == 1) ? "mouse" : "trackball")),
	     (int)req.a_reply.a_buffer[7]);
      devp[number].mouseButtons = (short)req.a_reply.a_buffer[7];
      
      if (req.a_reply.a_buffer[0] == 0x74 &&
	  req.a_reply.a_buffer[1] == 0x70 &&
	  req.a_reply.a_buffer[2] == 0x61 &&
	  req.a_reply.a_buffer[3] == 0x64) {
	 printf("Apple trackpad device detected.\n");
	 devp[number].mouseType = MOUSE_TRACKPAD;
	 if (env = getenv("trackpad")) {
	    printf("Calling init_trackpad\n");
	    init_trackpad(number);
	 }
      } else if (req.a_reply.a_buffer[0] == 0x4b &&
		 req.a_reply.a_buffer[1] == 0x4d &&
		 req.a_reply.a_buffer[2] == 0x4c &&
		 req.a_reply.a_buffer[3] == 0x31) {
	 printf("Kensington device detected.\n");
	 devp[number].mouseType = MOUSE_TURBOMOUSE_5;
	 if (env = getenv("kensington")) {
	    printf("Calling init_turbomouse\n");
	    init_turbomouse(number);
	 }
      }
   }
}


void
init_trackpad(int number) {
   struct adb_device	*devp;
   adb_request_t req;
   unsigned char r1_buffer[8];
   unsigned char magic1[8], magic3[8];
   unsigned char magic2[8] = {
      0x99, 0x94, 0x19, 0xff, 0xb2, 0x8a, 0x1b, 0x50
   };
   
   devp = adb_devices;

   /* Read register 1 */
   adb_init_request(&req);
   ADB_BUILD_CMD2(&req, ADB_PACKET_ADB,
		  (ADB_ADBCMD_READ_ADB | (number << 4) | 1));
   adb_send(&req, TRUE);
   if (req.a_reply.a_bcount < 8) {
      printf("init_trackpad failed!\n");
      return;
   }
   memcpy(r1_buffer, req.a_reply.a_buffer, 8);

   memcpy(magic1, r1_buffer, 8);
   magic1[6] = 0x0d;
   adb_init_request(&req);
   ADB_BUILD_CMD2_BUFFER(&req, ADB_PACKET_ADB,
			 (ADB_ADBCMD_WRITE_ADB | (number << 4) | 1),
			 8, magic1);
   adb_send(&req, TRUE);
   
   adb_init_request(&req);
   ADB_BUILD_CMD2_BUFFER(&req, ADB_PACKET_ADB,
			 (ADB_ADBCMD_WRITE_ADB | (number << 4) | 2),
			 8, magic2);
   adb_send(&req, TRUE);

   memcpy(magic3, r1_buffer, 8);
   magic3[6] = 0x03;
   adb_init_request(&req);
   ADB_BUILD_CMD2_BUFFER(&req, ADB_PACKET_ADB,
			 (ADB_ADBCMD_WRITE_ADB | (number << 4) | 1),
			 8, magic3);
   adb_send(&req, TRUE);
}


void
init_turbomouse(int number) {
   struct adb_device	*devp;
   adb_request_t req;
   unsigned char ken_tm_magic1[] = {
      0xE7, 0x8C, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x94
   };
   unsigned char ken_tm_magic2[] = {
      0xA5, 0x14, 0x00, 0x00, 0x69, 0xFF, 0xFF, 0x27
   };
   
   devp = adb_devices;
   adb_init_request(&req);
   /* To activate the TurboMouse a sequence of data must be written to
    * register 2 on the instance which has a handler of the above.
    * The TurboMouse appears as two diffent mice on address 3
    * originally.  This sequence was discovered by tracing calls to
    * _ADBOp under MacOS and verifying it with an strace on mousehack.
    * -- Mike Santos
    */
   printf("Activating Kensington TurboMouse.\n");
   ADB_BUILD_CMD2_BUFFER(&req, ADB_PACKET_ADB,
			 (ADB_ADBCMD_WRITE_ADB | (number << 4) | 2),
			 sizeof(ken_tm_magic1), ken_tm_magic1);
   adb_send(&req, TRUE);
   /* Device must be flushed before sending second command (part of
    * observed Kensington startup extension behavior) -- Mike Santos
    */
   adb_init_request(&req);
   ADB_BUILD_CMD2(&req, ADB_PACKET_ADB, (ADB_ADBCMD_FLUSH_ADB |
					 (number << 4)));
   adb_send(&req, TRUE);
   adb_init_request(&req);
   ADB_BUILD_CMD2_BUFFER(&req, ADB_PACKET_ADB,
			 (ADB_ADBCMD_WRITE_ADB | (number << 4) | 2),
			 sizeof(ken_tm_magic2), ken_tm_magic2);
   adb_send(&req, TRUE);
}   


void
extended_mouse_adbhandler(int number, unsigned char *buffer, int count)
{
   if (count == 4) {
#if DEBUG_ADBHANDLER
      printf("extended_mouse_adbhandler - num: %d, buf0: %02X, buf1: %02X, buf2: %02X, buf3: %02X, count: %d\n",
	     number, (int)buffer[0], (int)buffer[1], (int)buffer[2], (int)buffer[3], count);
#endif
      /* If the tty is accepting mouse events, then pass them on. */
      if(vc_xservermode) {
	 unsigned char x;
	 unsigned char y;
	 unsigned char buttons;
	 unsigned char button_left, button_middle, button_right;
	 unsigned char mask_left, mask_middle, mask_right;
#if MOUSE_DEBUG
	 mouse_state_t newstate;
#endif

	 x = POS(buffer[1]);
	 y = POS(buffer[0]);
	 buttons = ~buffer[3] & 0x0F; /* mask the four possible buttons (negative logic) */
	 if (adb_devices[number].a_dev_orighandler == HANDLER_ID_TRACKBALL_PRO) { /* special arrangement of buttons for Trackball Pro */
	    mask_left = BUTTON_MASK_LEFT_TRACKBALL_PRO;	/* set button mask for left button */
	    mask_middle = BUTTON_MASK_MIDDLE_TRACKBALL_PRO; /* set button mask for middle button */
	    mask_right = BUTTON_MASK_RIGHT_TRACKBALL_PRO; /* set button mask for right button */
	 } else {
	    mask_left = BUTTON_MASK_LEFT; /* set button mask for left button */
	    mask_middle = BUTTON_MASK_MIDDLE; /* set button mask for middle button */
	    mask_right = BUTTON_MASK_RIGHT; /* set button mask for right button */
	 }
	 button_left = (buttons & mask_left) ? DOWN : UP; /* set button flag for left button */
	 button_middle = (buttons & mask_middle) ? DOWN : UP; /* set button flag for middle button */
	 button_right = (buttons & mask_right) ? DOWN : UP; /* set button flag for right button */
#if MOUSE_DEBUG
	 newstate.xpos = x;
	 newstate.ypos = y;
	 newstate.left = button_left;
	 newstate.middle = button_middle;
	 newstate.right = button_right;
#endif
	 if (x || y || button_left != last_state.left) {
#if MOUSE_DEBUG
	    if (x || y) {
	       mouse_report("position", &newstate);
	    } else if (button_left != last_state.left) {
	       mouse_report("left", &newstate);
	    }
#endif
	    ttyinput(MOUSE_ESCAPE, &vc_tty); /* Let the TTY know this is mouse stuff. */
	    ttyinput(y | button_left, &vc_tty); /* Send one position delta and the button state. */
	    ttyinput(x, &vc_tty); /* Send the second position delta. */
	    last_state.xpos = x;
	    last_state.ypos = y;
	    last_state.left = button_left;
	 }
	 if (button_middle != last_state.middle) {
#if MOUSE_DEBUG
	    mouse_report("middle", &newstate);
#endif
	    ttyinput(KEY_MIDDLE_BUTTON | button_middle, &vc_tty);
	    last_state.middle = button_middle;
	 }
	 if (button_right != last_state.right) {
#if MOUSE_DEBUG
	    mouse_report("right", &newstate);
#endif
	    ttyinput(KEY_RIGHT_BUTTON | button_right, &vc_tty);
	    last_state.right = button_right;
	 }
      }
   } else {
      /*    printf("extended_mouse_adbhandler: bad count: %d\n", count);*/
      mouse_adbhandler(number, buffer, count);
   }
}
#endif


void 
mouse_adbhandler(int number, unsigned char *buffer, int count) {
   static const char *buttonMap = NULL;
   static int firstTime = 1;

#if DEBUG_ADBHANDLER 
   printf("mouse_adbhandler -  num: %d, buf0: %X, buf1:%X, buf2:%X, count: %d\n", number, (int) buffer[0], (int) buffer[1], (int)buffer[2], count);
#endif
   /* If the tty is accepting mouse events, then pass them on. */
   if(vc_xservermode) {
#if MOUSE_DEBUG
      mouse_state_t newstate;
#endif

      /* Do any button mapping here, so that all of the buttons can be
       * handled uniformly later.
       */
      if (adb_devices[number].mouseType == MOUSE_TRACKPAD) {
	 /* The following note is from LinuxPPC trackpad support:
	  * If it's a trackpad, we alias the second button to the first.
	  * NOTE: Apple sends an ADB flush command to the trackpad when
	  * the first (the real) button is released. We could do this
	  * here using async flush requests.
	  */
	 if (count > 1) {
	    /* Set button bit in buffer[0] if either button 1 or virtual
	     * button 2 (reported by trackpad) are pressed.  Force button
	     * bit in buffer[1] to be clear.  Recall, this is negative logic,
	     * so a "cleared" button bit is set to 1.
	     */
#if DEBUG_ADBHANDLER
	    printf("Trackpad original button states: 1: %s, 2: %s\n",
		   BUTTON(buffer[0]) ? "UP" : "DOWN",
		   BUTTON(buffer[1]) ? "UP" : "DOWN");
#endif
	    buffer[0] = (buffer[0] & 0x7F) | ((buffer[0] & buffer[1]) & 0x80);
	    buffer[1] = (buffer[1] & 0x7F) | 0x80;
#if DEBUG_ADBHANDLER
	    printf("Trackpad remapped button states: 1: %s, 2: %s\n",
		   BUTTON(buffer[0]) ? "UP" : "DOWN",
		   BUTTON(buffer[1]) ? "UP" : "DOWN");
#endif
	 }
      }

      if (firstTime) {
	 firstTime = 0;
	 buttonMap = getenv("button_map");
      }

      /* Currently, we just map four buttons into three, but this could
       * be extended to map any number of buttons.  Although, it would be
       * better to rewrite the ADB code to pass more than three buttons to
       * linux server.  Then we could let the Linux server do the mapping,
       * as it really should be doing in the first place.
       */
      if (buttonMap) {
	 unsigned char buttons = 0xFF;
	 int i;
	 /* Remap buttons according to button_map environment variable,
	  * if set.  i+1 is the physical button to map to the button
	  * in the ith position in the buttonMap string.
	  */
	 for (i = 0; i < 4 && i < strlen(buttonMap); i++) {
	   unsigned char buttonMask;
	    int target = buttonMap[i] - '0';
	    if (target >= 1 && target <= 4) {
	       /* Get the state of the button being mapped, but
		* default to UP in case we can't determine the real
		* state
		*/
	       int buttonState = UP;
	       if (i < 3) {
		  /* The first three buttons use the BUTTON macro */
		  if (count >= i) {
		     buttonState = BUTTON(buffer[i]);
		  }
	       } else {
		  /* The fourth button uses the BUTTON_EXT macro on the
		   * third byte
		   */
		  if (count >= 3) {
		     buttonState = BUTTON_EXT(buffer[2]);
		  }
	       }
	       /* Now set the bit in buttons */
	       /* The expression evals to 0xFF, with the exception of the
		* target bit, which is set to the boolean value of 
		* buttonState
		*/
	       buttons &= (0xFF ^ (!buttonState << (target - 1)));
	    }
	 }
	 /* Now set the button bits in buffer[] according to the values in
	  * buttons
	  */
	 for (i = 0; i < 3; i++) {
	   buffer[i] = (buttons & (1 << i)) ? buffer[i] | 0x80 :
	     buffer[i] & 0x7F;
	 }
      }

#if MOUSE_DEBUG
      newstate.ypos = POS(buffer[0]);
      newstate.xpos = POS(buffer[1]);
      newstate.left = BUTTON(buffer[0]);
      newstate.middle = BUTTON(buffer[1]);
      newstate.right = BUTTON(buffer[2]);
#endif
      if (POS(buffer[0]) || POS(buffer[1]) ||
	  (BUTTON(buffer[0]) != last_state.left)) {
#if MOUSE_DEBUG
	 if (POS(buffer[0]) || POS(buffer[1])) {
	    mouse_report("position", &newstate);
	 } else if (BUTTON(buffer[0]) != last_state.left) {
	    mouse_report("left", &newstate);
	 }
#endif
	 /* Let the TTY know this is mouse stuff. */
	 ttyinput(MOUSE_ESCAPE, &vc_tty); 
	 /* Send one position delta and the button state. */
	 ttyinput(buffer[0], &vc_tty); 
	 /* Send the second position delta. */
	 ttyinput(buffer[1], &vc_tty);
	 
	 last_state.ypos = POS(buffer[0]);
	 last_state.xpos = POS(buffer[1]);
	 last_state.left = BUTTON(buffer[0]);
      }

      if (count >= 2 && BUTTON(buffer[1]) != last_state.middle) {
#if MOUSE_DEBUG
	 mouse_report("middle", &newstate);
#endif
	 ttyinput(KEY_MIDDLE_BUTTON | BUTTON(buffer[1]), &vc_tty);
	 last_state.middle = BUTTON(buffer[1]);
      }

      if (count >= 3 && BUTTON(buffer[2]) != last_state.right) {
#if MOUSE_DEBUG
	 mouse_report("right", &newstate);
#endif
	 ttyinput(KEY_RIGHT_BUTTON | BUTTON(buffer[2]), &vc_tty);
	 last_state.right = BUTTON(buffer[2]);
      }
   }
}

#if MOUSE_DEBUG
void
mouse_report(const char *str, const mouse_state_t *newstate)
{
   printf(str);
   printf(", x=%X, y=%X, b0=%X, b1=%X, b2=%X, ",
	  newstate->xpos, newstate->ypos, newstate->left, newstate->middle, newstate->right);
   printf("last: lx=%X, ly=%X, ll=%X, lm=%X, lr=%X\n",
	  (int)last_state.xpos, (int)last_state.ypos, (int)last_state.left,
	  (int)last_state.middle, (int)last_state.right);
}
#endif

void mouse_do_buttons(int button,int up)
{
   unsigned char buffer[3];
  

   buffer[0]=last_state.left;
   buffer[1]=last_state.middle;
   buffer[2]=last_state.right;

   buffer[button]=up<<7;
   mouse_adbhandler(3,buffer,4);

}

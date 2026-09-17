/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

/* Mach Console -- uses mach terminal emulator
 *
 * Created by Zonnie L. Williamson at CMU, 1991
 *
 * Note that console_setstat() TTY_SET_BREAK will reset the console device.
 */

extern int terminal_setup(int terminal_context,
                          char *display_buffer,
                          int display_height,
                          int display_width,
                          int display_rowsize,
                          char *font_bits,
                          int font_height,
                          int font_width,
                          char initial_cursor,
                          int white_on_black,
                          void (*ungetc)(char, char*),
                          char *ungetc_argument,
                          void (*bell)(void));

extern void terminal_emulator(int terminal_context, char c);

#include <device/conf.h>
#include <device/tty.h>
#include <device/io_req.h>
#include <device/buf.h>

#include <sys/types.h>
#include <sys/varargs.h>

#include <mac2/act.h>
#include <mac2/clock.h>

#undef NULL
#include <mac2os/Types.h>
#include <mac2os/Video.h>
#include <mac2dev/video.h>
#include <mac2dev/adb.h>

struct tty console_tty;
struct act *console_act;

struct {
  unsigned long transData;
  unsigned long state;
  unsigned short modflags;
  struct act *key_act;
  unsigned char	repeat_code;
} key_data;

#define KEY_MOD_CTRL	0x1000
#define KEY_MOD_OPT	0x0800
#define KEY_MOD_CAPS	0x0400
#define KEY_MOD_SHFT	0x0200
#define KEY_MOD_CMD	0x0100

#define KEY_CLOCK_LIST	0

/* putc used by xprintf() */
static xprintf_putc(register char c)
{
  terminal_emulator(0, c);
}

/* xprintf() is a fast, protected "printf" to the status line */
/*VARARGS*/
xprintf(fmt, va_alist)
char *fmt;
va_dcl
{
  int s, saved_status, saved_status_mode;

  va_list listp;
  va_start(listp);
  s = spl7();
  terminal_emulator(0, 1);
  _doprnt(fmt, &listp, xprintf_putc, 16);
  terminal_emulator(0, 2);
  splx(s);
  va_end(listp);
}

/* Called by console output activity at IPL0 to do actual output. */
static console_output(register struct tty *tp)
{
  register int c, s;
  register char *t;
  extern int ttrstrt();

  for (;;) {
    s = spl7();
    if ((tp->t_state&TS_TTSTOP) || (tp->t_outq.c_cc == 0)) break;
    c = getc(&tp->t_outq);
    if (c <= 127 || tp->t_flags & LITOUT) terminal_emulator(0, c);
    else {
      timeout(ttrstrt, (caddr_t)tp, (c & 127));
      tp->t_state |= TS_TIMEOUT;
      break;
    }
    splx(s);
  }
  tp->t_state &= ~TS_BUSY;
  if (tp->t_outq.c_cc <= TTLOWAT(tp)) tt_write_wakeup(tp);
  splx(s);
}

/* Convert arrow keystrokes to multiple-character codes like X11 does. */
static void console_ttyinput(unsigned char key, unsigned short flags)
{
  switch (key) {
    case 0x8A: /* ku, up arrow */
      ttyinput('[' - '@', &console_tty);
      ttyinput('[', &console_tty);
      ttyinput((flags & KEY_MOD_SHFT) ? 'a' : 'A', &console_tty);
      break;
    case 0x8B: /* kd, down arrow */
      ttyinput('[' - '@', &console_tty);
      ttyinput('[', &console_tty);
      ttyinput((flags & KEY_MOD_SHFT) ? 'b' : 'B', &console_tty);
      break;
    case 0x8C: /* kr, right arrow */
      ttyinput('[' - '@', &console_tty);
      ttyinput('[', &console_tty);
      ttyinput((flags & KEY_MOD_SHFT) ? 'c' : 'C', &console_tty);
      break;
    case 0x8D: /* kl, left arrow */
      ttyinput('[' - '@', &console_tty);
      ttyinput('[', &console_tty);
      ttyinput((flags & KEY_MOD_SHFT) ? 'd' : 'D', &console_tty);
      break;
    default:
      /* command key prefaces key code with esacpe */
      if (flags & KEY_MOD_CMD) ttyinput('[' - '@', &console_tty);
      /* option key controls high bit */
      if (flags & KEY_MOD_OPT) key |= 0x80;
      else key &= 127;
      ttyinput(key, &console_tty);
      break;
  }
}

/* This activity function does character repeats while a key is held down. */
static repeat_key(unsigned char key_char, int list, struct act *act)
{
  if (key_data.repeat_code == 0) return;
  console_ttyinput(key_char, key_data.modflags);
  runact(list, act, key_char, hz/20);
}

/* process ADB key codes */
static adb_key(unsigned char key_code)
{
  static int toggle_7f;
  register unsigned long key_char;

  switch (key_code) {
    case 0x36:
      key_data.modflags |= KEY_MOD_CTRL;  break;
    case 0xb6:
      key_data.modflags &= ~KEY_MOD_CTRL; break;
    case 0x3a:
      key_data.modflags |= KEY_MOD_OPT;   break;
    case 0xba:
      key_data.modflags &= ~KEY_MOD_OPT;  break;
    case 0x39:
      key_data.modflags |= KEY_MOD_CAPS;  break;
    case 0xb9:
      key_data.modflags &= ~KEY_MOD_CAPS; break;
    case 0x38:
      key_data.modflags |= KEY_MOD_SHFT;  break;
    case 0xb8:
      key_data.modflags &= ~KEY_MOD_SHFT; break;
    case 0x37:
      key_data.modflags |= KEY_MOD_CMD;   break;
    case 0xb7:
      key_data.modflags &= ~KEY_MOD_CMD;  break;
    case 0xff:
      break;
    case 0x7f: /* reset button down, send ^] */
      if (toggle_7f++ & 1) ttyinput(']' - '@', &console_tty);
      break;
    default:
      if (key_code & 0x80) {
        if (key_code == key_data.repeat_code) key_data.repeat_code = 0;
        break;
      }
      key_char = KeyTrans(key_data.transData,
        (key_data.modflags & ~(KEY_MOD_OPT | KEY_MOD_CMD) | key_code),
        &key_data.state);
      switch (key_code & 127) {
        case 0x33: key_char = 127;  break; /* delete */
        case 0x3E: key_char = 0x8A; break; /* up arrow */
        case 0x3D: key_char = 0x8B; break; /* down arrow */
        case 0x3C: key_char = 0x8C; break; /* right arrow */
        case 0x3B: key_char = 0x8D; break; /* left arrow */
      }
      cancelact(key_data.key_act);
      key_data.repeat_code = (0x80 | key_code);
      runact(KEY_CLOCK_LIST, key_data.key_act, key_char, hz/2);
      console_ttyinput(key_char, key_data.modflags);
      break;
  }
}

/* This is the interrupt handler for adb keyboard input. */
static adb_input(adb_cmd_t cmd, unsigned char *data)
{
  if ((console_tty.t_state&TS_ISOPEN) && cmd.reg.cmd == ADB_talk
      && cmd.reg.reg == 0 && cmd.cmd.len == 2) {
    adb_key(data[0]);
    adb_key(data[1]);
  }
}

/* Start console output. */
static console_start(tp)
register struct tty *tp;
{
  if (tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP)) return;
  if (tp->t_outq.c_cc <= TTLOWAT(tp)) tt_write_wakeup(tp);
  if (tp->t_outq.c_cc == 0) return;
  tp->t_state |= TS_BUSY;
  softact(console_act, tp);
}

/* Reset the console device. */
void console_reset()
{
  int s;
  boolean_t result;
  VPBlock v;
  struct video *vp = &video[0];
  extern void ttyinput(char, struct tty *);

  s = spl7();
  if (!vp->video_RefNum) (void)video_cons_find(&v);
  else {
    (void)video_get_params(vp->video_slot, vp->video_id, oneBitMode, &v);
    (void)video_init(vp->video_slot, vp->video_id, &video[0]);
  }
  /* if terminal_setup() fails here, all is lost... */
  terminal_setup(0,
                (char *)(vp->video_devbase + v.vpBaseOffset),
                v.vpBounds.bottom - v.vpBounds.top,
                v.vpBounds.right - v.vpBounds.left,
                v.vpRowBytes,
                0, 0, 0, /* auto-select font */
                'e', /* closed oval cursor */
                0, /* black on white */
                (void (*)(char, char*))ttyinput,
                (char *)&console_tty,
                0);
  /* direct adb keyboard input back to the console */
  adb_close(ADB_ADDR_KEYBOARD);
  splx(s);
} /* console_reset() */

/* Open the console device. */
io_return_t console_open(dev_t dev, int flag, io_req_t ior)
{
  register struct tty *tp;
  register io_return_t result;

  tp = &console_tty;
  tp->t_addr = 0;
  tp->t_oproc = console_start;
  tp->t_stop = nulldev;
  if ((tp->t_state&TS_ISOPEN) == 0) {
    if (Xadb_query(ADB_ADDR_KEYBOARD, -1, 0) != ADB_ADDR_KEYBOARD)
      return D_NO_SUCH_DEVICE;
    ttychars(tp);
    tp->t_flags = EVENP|ODDP|ECHO|XTABS|CRMOD;
    tp->t_ispeed = tp->t_ospeed = B9600;
    tp->t_state = TS_CARR_ON;
  }
  /* allocate console output activity if none exists. */
  if (console_act == 0) {
    console_act = (struct act *)makesoftact(console_output, SR_IPL0);
    if (console_act == 0) panic("console_open: allocate output act");
  }
  /* load key translation resource. */
  if (key_data.transData == 0) {
    Handle h = (Handle)GetResource(*(unsigned long *)"KCHR", 0);
    if (h) {
      DetachResource(h);
      HLock(h);
      key_data.transData = (unsigned)*h;
    }
  }
  /* allocate key repeat activity if none exists. */
  if (key_data.key_act == 0) {
    key_data.key_act = (struct act *)makeact(repeat_key, SR_IPL1, 1);
    if (key_data.key_act == 0) panic("console_open: allocate repeat act");
    addact(KEY_CLOCK_LIST, key_data.key_act, &actclock);
  }
  if ((tp->t_state&TS_ISOPEN) == 0) {
    if (!Xadb_open(ADB_ADDR_KEYBOARD, adb_input)) return (D_IO_ERROR);
  }
  result =  char_open(dev, tp, flag, ior);
  if (result != D_SUCCESS) {
    if ((tp->t_state & TS_ISOPEN) == 0) Xadb_close(ADB_ADDR_KEYBOARD);
    return result;
  }
  tp->t_state |= TS_ISOPEN;
  return D_SUCCESS;
}

/* Close the console device. */
void console_close(dev_t dev)
{
  Xadb_close(ADB_ADDR_KEYBOARD);
  cancelact(key_data.key_act);
  key_data.repeat_code = 0;
  key_data.modflags = 0;
  key_data.state = 0;
  ttyclose(&console_tty);
}

/* Read from the console device. */
io_return_t console_read(dev_t dev, struct uio *uio)
{
  return char_read(&console_tty, uio);
}

/* Write to the console device. */
io_return_t console_write(dev_t dev, struct uio *uio)
{
  return char_write(&console_tty, uio);
}

/* Something died... */
boolean_t console_portdeath(dev_t dev, mach_port_t port)
{
  return tty_portdeath(&console_tty, port);
}

/* Console device getstat function. */
io_return_t console_getstat
  (dev_t dev, int flavor, int *data, unsigned int *count)
{
  return tty_get_status(&console_tty, flavor, data, count);
}

/* Console device setstat function. */
/* Note that console_setstat() TTY_SET_BREAK will reset the console device. */
io_return_t console_setstat
  (dev_t dev, int flavor, int *data, unsigned int count)
{
  if (flavor == TTY_SET_BREAK) {
    console_reset();
    return D_SUCCESS;
  }
  else if (flavor == TTY_CLEAR_BREAK) return D_SUCCESS;
  return tty_set_status(&console_tty, flavor, data, count);
}

/* There is nothing to force. */
cons_force()
{
}

/* There is no kernel console input. */
cngetc()
{
  panic("cngetc");
}

/* Used by the kernel printf() to output characters. */
cnputc(register char c)
{
  int s;

  s = spl7();
  if (c == '\n') terminal_emulator(0, '\r');
  terminal_emulator(0, c);
  splx(s);
}

/* Initialize console device so that kernel printfs work. */
console_printf_init()
{
  console_reset();
}

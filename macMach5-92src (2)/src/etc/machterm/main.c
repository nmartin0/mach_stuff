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

/* main.c -- mach terminal utility
 *
 * Created by Zonnie L. Williamson at CMU, 1992
 *
 *
 * If KERNEL is not defined, this module compiles into a standalone
 * display server and utility.  Its usage is:
 *   machterm [ -sh | -csh ]
 *   machterm -status [ <text> ]
 *   machterm -invert
 *   machterm -cursor <cursor type>
 *   machterm <display device>...
 *
 * The first form is used to set the terminal type.  If -sh or -csh
 * is given, then /bin/sh (/bin/csh) instructions are generated to
 * set the correct terminal type.  If no command line arguments are
 * specified, the text of the termcap entry is written to stdout.  The
 * following code segment could be placed in ~/.login to automatically
 * set the terminal type at login:
 *   setenv TERM ""
 *   eval "`machterm -csh`"
 *   if ("$TERM" == '') then
 *     echo -n "term: "
 *     set i = $<
 *     if ("$i" == '') then
 *       setenv TERM "ansi"
 *     else
 *       setenv TERM "$i"
 *     endif
 *     unset i
 *     echo ""
 *   endif
 *
 * The second form is used to write text to the status line.
 *
 * The third form toggles between black on white and white on black
 * operation.
 *
 * The fourth form sets the cursor type.  The following cursors types
 * are supported:
 *   null
 *   closed box
 *   open box
 *   closed oval
 *   open oval
 *   closed line
 *   open line
 *
 * The final form runs a display server for the specified displays.
 * If a display is specified more than once, virtual terminals will
 * be created.  The display server runs /bin/login on each display.
 * When a process terminates, it is restarted.  The first display
 * will attempt to become the system console.  The display server is
 * controlled by the following keyboard commands:
 *   ^]-Q     -- shutdown the display server, all processes are killed
 *   ^]-^]    -- send a ^]
 *   ^]-?     -- display help text
 *   ^]-space -- switch to the next display
 *   ^]-digit -- switch to the specified display
 *
 ******************************************************************************
 *
 * The mach terminal utility program is organized as:
 *   open_screen()     -- open a screen device
 *   process_puts()    -- write a string to a process's terminal
 *   pty_ungetc()      -- an "ungetc()" for pty's
 *   beep()            -- beep the beep-beep
 *   help()            -- print help text
 *   fork_process()    -- startup a process
 *   restart_process() -- restart a process
 *   kill_process()    -- terminate a process
 *   setup_process()   -- setup a process
 *   switch_process()  -- switch to the next process
 *   server_loop()     -- the server I/O loop
 *   display_server()  -- run a display server
 *   invert_display()  -- toggle between black on white and white on black
 *   write_status()    -- write text to the status line
 *   error()           -- restore tty sgttyb and exit with error status
 *   setup_term()      -- set the terminal type, maybe generate shell code
 *   status()          -- write text to status line
 *   invert()          -- toggle betwen white and black
 *   main()            -- machterm utility program
 */

/*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <pwd.h>
#include <sgtty.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>

/*****************************************************************************/

#define VERSION "1.0"

typedef char terminal_context_t[256];

extern int terminal_setup(terminal_context_t *terminal_context,
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

extern int terminal_buffer(terminal_context_t *terminal_context,
                           char *display_buffer,
                           int display_rowsize);

extern void terminal_emulator(terminal_context_t *terminal_context, char c);

/*****************************************************************************/

#ifdef mac2

/* open Macintosh screen device */

#undef NULL
#include <mac2os/Types.h>
#include <mac2os/Video.h>
#include <mac2dev/video.h>
#include <mac2dev/adb.h>

/* open a screen device */
/* return non-zero if error */
static int open_screen(char *device,
                       int *display_fd,
                       char **display_buffer,
                       int *display_height,
                       int *display_width,
                       int *display_rowsize)
{
  int f, size, pagesize;
  char *buffer;
  VPBlock info;
  VDPageInfo vpinfo;
  extern char *malloc(int);
  extern int getpagesize(void);

  /* open the screen device */
  f = -1;
  if ((f = open(device, O_RDWR)) < 0)
    goto error;

  /* set to one bit per pixel */
  if (ioctl(f, VIDEO_CTRL_Init, &vpinfo) < 0)
    goto error;

  /* get display parameters */
  if (ioctl(f, VIDEO_PARAMS, &info) < 0)
    goto error;

  /* allocate page-aligned buffer */
  pagesize = getpagesize();
  size = info.vpBaseOffset +
    (info.vpRowBytes * (info.vpBounds.bottom - info.vpBounds.top));
  if (!(buffer = malloc(size + pagesize))) goto error;
  buffer = (char *)(((int)buffer / pagesize + 1) * pagesize);

  /* map buffer to display */
  if (mmap(buffer, size, PROT_READ | PROT_WRITE, MAP_SHARED, f, 0) < 0)
    goto error;

  /* return display parameters */
  *display_fd = f;
  *display_buffer = &buffer[info.vpBaseOffset],
  *display_height = info.vpBounds.bottom - info.vpBounds.top,
  *display_width = info.vpBounds.right - info.vpBounds.left,
  *display_rowsize = info.vpRowBytes;

  /* assert that buffer is int aligned */
  if ((unsigned int)*display_buffer & (sizeof(int) - 1))
    goto error;

  /* success, return zero */
  return 0;

  /* if any error, close the device an return non-zero */
error:
  if (f >= 0) close(f);
  return -1;

} /* open_screen() */

#endif

/*****************************************************************************/

/* maximum number of processes allowed */
#define MAXPROCESS 10 /* 0 - 9 */

#define CLOSED_CURSOR 'e' /* closed oval */
#define OPEN_CURSOR   'f' /* open oval */

typedef struct process {
  char *device_name;    /* name of screen device */
  int device;           /* screen device fd */
  char *buffer;         /* display buffer, int aligned */
  int height;           /* display height, in pixels */
  int width;            /* display width, in pixels */
  int rowsize;          /* buffer row size, in bytes */
  char *virtual_buffer; /* virtual display buffer */
  int virtual_rowsize;  /* virtual buffer rowsize, in bytes */
  int hidden;           /* is virtual terminal hidden? */
  int pty;              /* pty fd */
  int tty_name[11];     /* tty that corresponds to pty */
  int pid;              /* the pid */
  terminal_context_t terminal_context;
} *process_t;

struct process process[MAXPROCESS];
int process_count;
struct sgttyb saved_sgttyb;
int tty;
uid_t uid;

/* write a string to a process's terminal */
static void process_puts(process_t p, char *s)
{
  while (*s) terminal_emulator(&p->terminal_context, *s++);
}

/* an "ungetc()" for pty's */
static void pty_ungetc(char c, char *pty)
{
  write((int)pty, &c, 1);
}

/* beep the beep-beep */
static void beep(void)
{
  write(tty, (char []){ 'G' - '@' }, 1);
}

/*****************************************************************************/

/* print help text */
static char *help(process_t p, char *x)
{
  int i;

  sprintf(x, "\n\r");
  sprintf(&x[strlen(x)], "machterm version %s\n\r", VERSION);
  sprintf(&x[strlen(x)], "Use ^]-Q to exit machterm.\n\r");
  sprintf(&x[strlen(x)], "Use ^]-^] to send a ^].\n\r");
  sprintf(&x[strlen(x)], "Use ^]-? to display help text.\n\r");
  sprintf(&x[strlen(x)], "Use ^]-digit to go to a specific screen\n\r");
  sprintf(&x[strlen(x)], "Use ^]-space to go to the next screen\n\r");
  for (i = 0; i < process_count; i++) if (&process[i] != p) {
    sprintf(&x[strlen(x)],
            "machterm #%d is %s\n\r",
             i,
             process[i].tty_name);
  }
  sprintf(&x[strlen(x)],
          "This is machterm #%d, %s.\n\r",
          ((int)p - (int)process) / sizeof(*process),
          p->tty_name);
  return x;

} /* help() */

/*****************************************************************************/

/* startup a process */
void fork_process(process_t p)
{
  char **v, *s, msg[256];
  int i;
  struct passwd *passwd;
  extern int errno;

  /* fork the process, parent returns */
  if ((p->pid = fork()) > 0) return;
  else if (p->pid < 0) {
    fprintf(stderr, "machterm: fork failed, errno=%d\n\r", errno);
    (void)ioctl(tty, TIOCSETP, &saved_sgttyb);
    exit(1);
  }

  /* let go of parent's fd's */
  close(0);
  close(1);
  close(2);
  close(tty);
  for (i = 0; i < process_count; i++) {
    close(process[i].pty);
    close(process[i].device);
  }

  /* set process group to zero */
  /* the next terminal device opened will be be the controlling terminal */
  if (setpgrp(0, 0) < 0) {
    sprintf(msg, "[machterm: setpgrp failed, errno=%d]\n\r", errno);
    process_puts(p, msg);
    goto error;
  }

  /* open the corresponding tty, it will be the controlling terminal */
  if ((tty = open(p->tty_name, O_RDWR)) < 0) {
    sprintf(msg, "[machterm: open failed \"%s\", errno=%d]\n\r",
            p->tty_name,
            errno);
    process_puts(p, msg);
    goto error;
  }

  /* create stdin, stdout, stderr */
  if (tty != 0) dup2(tty, 0);
  if (tty != 1) dup2(tty, 1);
  if (tty != 2) dup2(tty, 2);
  if (tty > 2) close(tty);

  /* if process 0, try to grab the console */
  if (p == &process[0]) {
    int on = 1;
    if ((ioctl(0, TIOCCONS, (char *)&on) < 0) && (errno != EBUSY)) {
      sprintf(msg, "[machterm: ioctl TIOCCONS failed \"%s\", errno=%d]\n\r",
                   p->tty_name,
                   errno);
      write(1, msg, strlen(msg));
    }
  }

  /* setup this tty like all others */
  if (ioctl(0, TIOCSETP, &saved_sgttyb) < 0) {
    sprintf(msg, "[machterm: ioctl TIOCSETP fails \"%s\", errno=%d]\n\r",
                 p->tty_name,
                 errno);
    write(1, msg, strlen(msg));
  }

  /* return to user mode */
  setuid(uid);

  /* display help text */
  (void)help(p, msg);
  write(1, msg, strlen(msg));

  /* exec /bin/login */
  if (passwd = getpwuid(uid))
    execl("/bin/login", "login", "-f", passwd->pw_name, 0);

  /* if something fails... */
error:
  sleep(5);
  exit(0);

} /* fork_process() */

/* restart a process */
/* this is the SIGCHLD handler */
static restart_process()
{
  int n, i;

  /* wait() returns the pid of the terminated process */
  n = wait(0);

  /* search for pid, restart the process */
  for (i = 0; i < process_count; i++)
    if (n == process[i].pid)
      fork_process(&process[i]);

} /* restart_process() */

/*****************************************************************************/

/* setup a physical terminal device */
static void setup_physical_terminal(process_t p, char *device_name)
{
  int result;
  extern int write(int, char *, int);

  p->device_name = device_name;

  /* open the physical screen device */
  result = open_screen(device_name,
                       &p->device,
                       &p->buffer,
                       &p->height,
                       &p->width,
                       &p->rowsize);
  if (result) {
    fprintf(stderr, "machterm: can not open screen \"%s\"\n", device_name);
    exit(1);
  }

  /* physical terminal does not have virtual stuff */
  p->virtual_rowsize = 0;
  p->virtual_buffer = 0;
  p->hidden = 0;

  /* setup the terminal context */
  result = terminal_setup(&p->terminal_context,
                          p->buffer,
                          p->height,
                          p->width,
                          p->rowsize,
                          0, /* select internal font */
                          0,
                          0,
                          OPEN_CURSOR,
                          0, /* black on white */
                          pty_ungetc,
                          (char *)p->pty,
                          beep);
  if (result) {
    fprintf(stderr,
            "machterm: can not setup terminal \"%s\" (%d)\n",
            device_name, result);
    exit(1);
  }

} /* setup_physical_terminal() */

/*****************************************************************************/

/* setup a process using a virtual terminal */
static void setup_virtual_terminal(process_t p, process_t root)
{
  int result;
  extern int write(int, char *, int);
  extern char *malloc(int);

  /* setup virtual buffer on process 0 */
  /* virtual rowsize is minimal */
  if (!root->virtual_buffer) {
    root->virtual_rowsize = ((root->width / sizeof(int)) + 1) * sizeof(int);
    root->virtual_buffer = malloc(root->virtual_rowsize * root->height);
  }
  /* virtual terminal is initially hidden */
  p->hidden = 1;

  /* use display parameters from root */
  p->device_name = root->device_name;
  p->device = root->device;
  p->buffer = root->buffer;
  p->height = root->height;
  p->width = root->width;
  p->rowsize = root->rowsize;
  p->virtual_rowsize = root->virtual_rowsize;

  /* allocate private virtual display buffer */
  p->virtual_buffer =
    malloc(p->virtual_rowsize * p->height);

  /* assert that memory allocate went OK */
  if (!root->virtual_buffer || !p->virtual_buffer) {
    fprintf(stderr, "machterm: out of memory\n");
    exit(1);
  }

  /* setup the terminal context */
  result = terminal_setup(&p->terminal_context,
                          p->virtual_buffer,
                          p->height,
                          p->width,
                          p->virtual_rowsize,
                          0, /* select internal font */
                          0,
                          0,
                          OPEN_CURSOR,
                          0, /* black on white */
                          pty_ungetc,
                          (char *)p->pty,
                          beep);
  if (result) {
    fprintf(stderr,
            "machterm: can not setup virtual terminal (%d)\n",
            result);
    exit(1);
  }

} /* setup_virtual_terminal() */

/*****************************************************************************/

/* terminate a process */
static void kill_process(process_t p)
{
  /* clear screen */
  process_puts(p, "\EH\EJ");
  /* null cursor */
  process_puts(p, (char []){ 'E' - '@', 'b', 0 });
  kill(p->pid, SIGKILL);
  close(p->pty);
  close(p->device);

} /* kill_process() */

/* setup a process */
/* exit(1) if error */
static void setup_process(process_t p, char *device)
{
  char pty_name[11];
  int n;

  /* find a free pty */
  for (n = 0; n < 16; n++) {
    sprintf(pty_name, "/dev/ptyp%c", "0123456789abcdef"[n]);
    sprintf(p->tty_name, "/dev/ttyp%c", pty_name[9]);
    if ((p->pty = open(pty_name, O_RDWR)) >= 0) break;
  }
  if (p->pty < 0) {
    fprintf(stderr, "machterm: can not find a free pty\n");
    exit(1);
  }

  /* if device has been specified before, add virtual terminal to group */
  for (n = 0; n < process_count; n++)
    if (!strcmp(device, process[n].device_name)) {
      setup_virtual_terminal(p, &process[n]);
      return;
    }

  /* otherwise, setup a physical terminal */
  setup_physical_terminal(p, device);

} /* setup_process() */

/*****************************************************************************/

/* switch to the next process */
static void switch_process(process_t last, process_t next)
{
  int i;

  /* make the last cursor an open line */
  process_puts(last, (char []){ 'E' - '@', OPEN_CURSOR, 0 });

  /* make the next cursor a closed line */
  process_puts(next, (char []) { 'E' - '@', CLOSED_CURSOR, 0 });

  /* if not switching to a hidden virtual terminal, all done */
  if (!next->hidden) return;

  /* hide the currently visable virtual terminal for this group */
  for (i = 0; i < process_count; i++)
    if ((process[i].device == next->device) &&
        process[i].virtual_buffer &&
        !process[i].hidden) {
      terminal_buffer(&process[i].terminal_context,
                      process[i].virtual_buffer,
                      process[i].virtual_rowsize);
      process[i].hidden = 1;
    }

  /* make the hidden virtual terminal visable */
  terminal_buffer(&next->terminal_context, next->buffer, next->rowsize);
  next->hidden = 0;

} /* switch_process() */

/*****************************************************************************/

/* the server I/O loop */
static void server_loop(void)
{
  fd_set readfds;
  char chars[128], *s, x[256];
  struct sgttyb sgttyb;
  int i, n, nfds;
  process_t current, next;

  /* condition the console */
  ioctl(tty, TIOCGETP, &sgttyb);
  sgttyb.sg_flags &= ~ECHO;
  sgttyb.sg_flags |= RAW;
  ioctl(tty, TIOCSETP, &sgttyb);

  /* determine nfds */
  nfds = tty;
  for (i = 0; i < process_count; i++)
    if (process[i].pty > nfds) nfds = process[i].pty;
  nfds++;

  /* start out with process zero -- make cursor closed */
  current = &process[0];
  process_puts(current, (char []){ 'E' - '@', CLOSED_CURSOR, 0 });

  /* try to run I/O loop at elevated priority */
  setpriority(PRIO_PROCESS, 0, getpriority(PRIO_PROCESS, 0) - 5);

  /* I/O loop until ^]-@ */
  for (;;) {

    /* use select() to block for I/O */
    do {
      FD_ZERO(&readfds);
      for (i = 0; i < process_count; i++) FD_SET(process[i].pty, &readfds);
      FD_SET(tty, &readfds);
    } while (select(nfds, &readfds, 0, 0, 0) <= 0);

    /* output to terminals */
    for (i = 0; i < process_count; i++)
      if (FD_ISSET(process[i].pty, &readfds))
        if ((n = read(process[i].pty, chars, sizeof(chars) - 1)) > 0) {
          chars[n] = 0;
          process_puts(&process[i], chars);
        }

    /* input from keyboard */
    if (FD_ISSET(tty, &readfds))
      if ((n = read(tty, chars, sizeof(chars))) > 0) {
        for (chars[n] = 0, s = chars; *s; s++) {
          if (*s != (']' - '@')) write(current->pty, s, 1);
          else {
            /* get next char after ^] */
            s++;
            if (!*s) {
              while (read(tty, chars, 1) != 1);
              s = chars;
            }
            /* if ^]-^] then send a ^] */
            if (*s == (']' - '@')) write(current->pty, s, 1);
            /* if ^]-Q then exit */
            else if ((*s == 'q') || (*s == 'Q')) goto done;
            /* if ^]-? then give some help */
            else if (*s == '?') process_puts(current, help(current, x));
            /* otherwise, switch screens */
            else {
              if (isdigit(*s)) {
                if ((*s - '0') >= process_count) continue;
                next = &process[*s - '0'];
              }
              else {
                if (*s != ' ') continue;
                next = &current[1];
                if (next == &process[process_count])
                  next = &process[0];
              }
              switch_process(current, next);
              current = next;
            }
          }
        }
      }

  } /* for(;;) */

done:

  /* restore console sgttyb */
  ioctl(tty, TIOCSETP, &saved_sgttyb);

} /* server_loop() */

/*****************************************************************************/

/* run a display server */
/* exit(1) if error */
static void display_server(int argc, char **argv)
{
  int i;
  extern int errno;

  /* try to become super-user */
  uid = getuid();
  (void)setuid(0);

  /* insure that the tty is the console */
  if (!ttyname(0) || strcmp(ttyname(0), "/dev/console")) {
    fprintf(stderr, "machterm: tty is not console\n");
    exit(1);
  }

  /* open the tty */
  if ((tty = open("/dev/tty", O_RDWR)) < 0) {
    fprintf(stderr,
            "machterm: open fails \"/dev/tty\", errno=%d\n",
            errno);
    exit(1);
  }

  /* save console sgttyb */
  if (ioctl(tty, TIOCGETP, &saved_sgttyb) < 0) {
    fprintf(stderr,
            "machterm: ioctl TIOCGETP fails \"/dev/tty\", errno=%d\n",
            errno);
    exit(1);
  }

  /* clear the process count */
  process_count = 0;

  /* setup specified devices */
  while (argc--) {
    if (process_count >= MAXPROCESS) {
      fprintf(stderr, "machterm: too many devices\n");
      exit(1);
    }
    setup_process(&process[process_count], *argv++);
    process_count++;
  }

  /* setup SIGCHLD handler */
  signal(SIGCHLD, restart_process);

  /* startup all processes */
  for (i = 0; i < process_count; i++) fork_process(&process[i]);

  /* run the server I/O loop */
  server_loop();

  /* remove SIGCHLD handler */
  signal(SIGCHLD, SIG_IGN);

  /* terminate all processes */
  for (i = 0; i < process_count; i++) kill_process(&process[i]);

  /* all done */
  printf("machterm exit.\n");

} /* display_server() */

/*****************************************************************************/

/* restore tty sgttyb and exit with error status */
void error(void)
{
  /* if tty opened, reset its sgttyb */
  if (tty) ioctl(tty, TIOCSETP, &saved_sgttyb);
  /* error return */
  exit(1);
}

/* set the terminal type, maybe generate shell code */
static void setup_term(char *generate)
{
  struct sgttyb sgttyb;
  struct winsize win;
  char answer[1024], msg[80], bp[1024], area[1024], *ap = area;
  int n, len;

  /* try to open the console */
  if ((tty = open("/dev/tty", O_RDWR)) < 0) exit(1);

  /* flush buffers */
  write(tty, (char []){ '\r' }, 1);
  sleep(1);

  /* remember original sgttyb */
  ioctl(tty, TIOCGETP, &saved_sgttyb);

  /* turn off echo, set CBREAK */
  ioctl(tty, TIOCGETP, &sgttyb);
  sgttyb.sg_flags &= ~ECHO;
  sgttyb.sg_flags |= RAW;
  ioctl(tty, TIOCSETP, &sgttyb);

  /* send ^C to request return of termcap entry */
  write(tty, ((char []){ 3 }), 1);

  /* set up error handler in case of timeout */
  signal(SIGALRM, (int (*)())error);

  /* short alarm while we read the first character */
  alarm(1);

  /* read the first character of the answerback, if any */
  n = read(tty, answer, 1);
  if (n != 1) error();
  answer[1] = 0;

  /* set longer alarm while we read the rest of the string */
  alarm(10);

  /* let the user know what we're up to */
  sprintf(msg, "%cmachterm: creating termcap entry%c", 1, 2);
  write(tty, msg, strlen(msg));
  sleep(1);

  /* read the rest of the string, break on CR */
  for (len = 1; !index(answer, '\r'); ) {
    n = read(tty, &answer[len], sizeof(answer) - len - 1);
    if (n <= 0) error();
    len += n;
    answer[len] = 0;
  }

  /* remove the CR */
  answer[len - 1] = 0;

  /* restore tty sgttyb */
  ioctl(tty, TIOCSETP, &saved_sgttyb);

  /* store returned string as value of TERMCAP environment variable */
  setenv("TERMCAP", answer, 1);

  /* get the termcap entry for the mach machterm */
  /* there should not be a "mach" entry in /etc/termcap */
  if (tgetent(bp, "mach") != 1) error();

  /* at this point, assume that we have a mach machterm */

  /* set window size */
  ioctl(tty, TIOCGWINSZ, &win);
  win.ws_col = tgetnum("co");
  win.ws_row = tgetnum("li");
  ioctl(tty, TIOCSWINSZ, &win);

  /* note window size on status line */
  sprintf(msg, "%cmachterm: %d x %d%c", 1, win.ws_col, win.ws_row, 2);
  write(tty, msg, strlen(msg));
  sleep(1);
  write(tty, (char []){1, 2}, 2);

  /* set erase character */
  if (tgetstr("kb", &ap)) {
    saved_sgttyb.sg_erase = *(char *)tgetstr("kb", &ap);
    ioctl(tty, TIOCSETP, &saved_sgttyb);
  }

  /* all done */
  close(tty);

  /* generate code for /bin/sh */
  if (!strcmp(generate, "SH"))
    printf("TERM=\"mach\"; TERMCAP=\"%s\"; export TERM TERMCAP\n", answer);

  /* generate code for /bin/csh */
  else if (!strcmp(generate, "CSH"))
    printf("setenv TERM \"mach\"; setenv TERMCAP \"%s\"\n", answer);

  /* otherwise, return termcap string */
  else puts(answer);

} /* setup_term() */

/*****************************************************************************/

/* write text to status line */
static void status_line(int argc, char **argv)
{
  struct sgttyb sgttyb;
  int space;
  char *term;
  extern char *getenv(char *);

  /* assert that this is a machterm */
  if (!(term = getenv("TERM")) || strcmp(term, "mach")) {
    fprintf(stderr, "machterm: not a machterm\n");
    exit(1);
  }

  /* access /dev/tty */
  if ((tty = open("/dev/tty", O_RDWR)) < 0) exit(1);

  /* remember original sgttyb */
  ioctl(tty, TIOCGETP, &saved_sgttyb);

  /* set raw mode */
  ioctl(tty, TIOCGETP, &sgttyb);
  sgttyb.sg_flags |= RAW;
  ioctl(tty, TIOCSETP, &sgttyb);

  /* unset raw mode */
  /* send ^A to goto enter status line */
  write(tty, (char []){ 'A' - '@' }, 1);

  /* write text to status line */
  for (argc--, argv++, space = 0; argc; argc--, argv++) {
    if (space++) write(1, (char []){ ' ' }, 1);
    write(tty, *argv, strlen(*argv));
  }

  /* send ^B to exit status line */
  write(tty, (char []){ 'B' - '@' }, 1);

  /* restore tty sgttyb */
  ioctl(tty, TIOCSETP, &saved_sgttyb);

  /* all done */
  close(tty);

} /* status_line() */

/*****************************************************************************/

/* toggle betwen white and black */
static void invert(void)
{
  struct sgttyb sgttyb;
  char *term;
  extern char *getenv(char *);

  /* assert that this is a machterm */
  if (!(term = getenv("TERM")) || strcmp(term, "mach")) {
    fprintf(stderr, "machterm: not a machterm\n");
    exit(1);
  }

  /* access /dev/tty */
  if ((tty = open("/dev/tty", O_RDWR)) < 0) exit(1);

  /* remember original sgttyb */
  ioctl(tty, TIOCGETP, &saved_sgttyb);

  /* set raw mode */
  ioctl(tty, TIOCGETP, &sgttyb);
  sgttyb.sg_flags |= RAW;
  ioctl(tty, TIOCSETP, &sgttyb);

  /* send ^D to toggle between white and black */
  write(1, (char []){ 'D' - '@' }, 1);

  /* restore tty sgttyb */
  ioctl(tty, TIOCSETP, &saved_sgttyb);

  /* all done */
  close(tty);

} /* invert() */

/*****************************************************************************/

/* set cursor type */
static void cursor_type(int argc, char **argv)
{
  struct sgttyb sgttyb;
  char *term, cursor;
  extern char *getenv(char *);

  /* assert that this is a machterm */
  if (!(term = getenv("TERM")) || strcmp(term, "mach")) {
    fprintf(stderr, "machterm: not a machterm\n");
    exit(1);
  }

  cursor = 0;
  if ((argc == 1) && !strcmp(*argv, "null")) cursor = 'b';
  else if ((argc == 2) && !strcmp(*argv, "closed")) {
    argv++;
    if (!strcmp(*argv, "box")) cursor = 'c';
    else if (!strcmp(*argv, "oval")) cursor = 'e';
    else if (!strcmp(*argv, "line")) cursor = 'g';
  }
  else if ((argc == 2) && !strcmp(argv, "open")) {
    argv++;
    if (!strcmp(*argv, "box")) cursor = 'd';
    else if (!strcmp(*argv, "oval")) cursor = 'f';
    else if (!strcmp(*argv, "line")) cursor = 'h';
  }
  if (!cursor) {
    fprintf(stderr, "machterm cursors are:\n");
    fprintf(stderr, "  null\n");
    fprintf(stderr, "  closed box\n");
    fprintf(stderr, "  open box\n");
    fprintf(stderr, "  closed oval\n");
    fprintf(stderr, "  open oval\n");
    fprintf(stderr, "  closed line\n");
    fprintf(stderr, "  open line\n");
    exit(1);
  }

  /* access /dev/tty */
  if ((tty = open("/dev/tty", O_RDWR)) < 0) exit(1);

  /* remember original sgttyb */
  ioctl(tty, TIOCGETP, &saved_sgttyb);

  /* set raw mode */
  ioctl(tty, TIOCGETP, &sgttyb);
  sgttyb.sg_flags |= RAW;
  ioctl(tty, TIOCSETP, &sgttyb);

  /* send ^E to set cursor */
  write(1, (char []){ 'E' - '@' }, 1);
  write(1, &cursor, 1);

  /* restore tty sgttyb */
  ioctl(tty, TIOCSETP, &saved_sgttyb);

  /* all done */
  close(tty);

} /* cursor_type() */

/*****************************************************************************/

/* machterm utility program */
main(int argc, char **argv)
{

  /* parse command line arguments */
  argc--;
  argv++;
  if (argc && !strcmp(*argv, "-status")) status_line(--argc, ++argv);
  else if ((argc == 1) && !strcmp(*argv, "-invert")) invert();
  else if (argc && !strcmp(*argv, "-cursor")) cursor_type(--argc, ++argv);
  else if (argc && (**argv == '/')) display_server(argc, argv);
  else if ((argc == 1) && !strcmp(*argv, "-csh")) setup_term("CSH");
  else if ((argc == 1) && !strcmp(*argv, "-sh")) setup_term("SH");
  else if (!argc) setup_term("");
  else {
    fprintf(stderr, "usage: machterm [-sh | -csh]\n");
    fprintf(stderr, "usage: machterm -invert\n");
    fprintf(stderr, "usage: machterm -status [ <text> ]\n");
    fprintf(stderr, "usage: machterm -cursor <cursor type>\n");
    fprintf(stderr, "usage: machterm <display device>... \n");
    exit(1);
  }

  /* all done, no error */
  exit(0);

} /* main() */

/* end of main.c */

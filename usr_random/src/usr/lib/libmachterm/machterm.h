
/* convert a "normal" character to a "graphics" character */
/* for example, TOGRAPHIC('a') is the copyright symbol */
#define TOGRAPHIC(c) (c - 95)

/* bits and bytes */
#define INTBYTES   4
#define SHORTBYTES 2
#define INTBITS   32
#define SHORTBITS 16
#define CHARBITS   8

/* black on white displays have a neat little border... */
#define VBORDER 4 /* vertical border width, in pixels */
#define HBORDER 3 /* horizontal border width, in pixels */

/* mode bits */
#ifndef SMALL_TERMINAL
#define BOLD      1
#endif
#define UNDERLINE 2
#define INVERSE   4
#define GRAPHIC   8

/* this is the actual terminal context structure */
typedef struct context {
  unsigned int *buffer;        /* bit-mapped display, one bit pixels */
  int height;                  /* display height, in pixels */
  int width;                   /* display width, in pixels */
  int rowints;                 /* display buffer width, in int's */
  int rowsize;                 /* display width, in int's */
  unsigned int left;           /* left border bits */
  unsigned int right1, right2; /* right border bits */
  int tmargin;                 /* y offset for centered display */
  int rmargin;                 /* the first pixel of the right border */
  int lmargin;                 /* x offset for centered display */
  char *font;                  /* the normal font bits */
  char *bold;                  /* the bold font bits */
  int font_height;             /* font height, in pixels */
  int font_width;              /* font width, in pixels */
  unsigned int mask;           /* font mask bits */
  char cursor;                 /* graphics character to use for a cursor */
  int white_on_black;          /* white on black or black on white */
  void (*ungetc)(char, char*); /* input write function */
  char *ungetc_argument;       /* 2nd argument to ungetc function */
  void (*bell)(void);          /* beep the bell */
  int max_x, max_y;            /* display size, in characters */
  int cursor_x, cursor_y;      /* current cursor position */
  int new_x, new_y;            /* new cursor position */
  int state;                   /* terminal emulator state */
  int mode;                    /* display attribute mode */
  int status;                  /* status line flag, x + 1 */
  int status_mode;             /* status line attribute mode */
  int am_count;                /* automatic margin counter */
} *context_t;

/* convert character coordinates to pixel coordinates */
#define TOPIXEL(t, x, y) \
  if (y == t->max_y) \
    y = (y * t->font_height) + t->tmargin + HBORDER; \
  else y = (y * t->font_height) + t->tmargin; \
  x = (x * t->font_width) + t->lmargin

/* buffer index to first row on text line */
/* y is a character coordinate */
#define ROWINDEX(t, y) ((((y) * t->font_height) + t->tmargin) * t->rowints)

/* buffer index to first row on status line */
#define STATUSINDEX(t) \
  (((t->max_y * t->font_height) + t->tmargin + HBORDER) * t->rowints)

/* buffer index to int that contains the specified pixel */
/* x and y are pixel coordinates */
#define INDEX(t, x, y) ((y * t->rowints) + (x / INTBITS))

/* determine pixel shift for a given font width */
/* if this is negative, then character is split over two int's */
#define SHIFT(t, x, width) (INTBITS - width - x % INTBITS)

/* compute white on black bits with a left shift */
#define LWBCHAR(b, mask, bits, shift) (b & ~mask) | (~(bits << shift) & mask)

/* compute white on black bits with a right shift */
#define RWBCHAR(b, mask, bits, shift) (b & ~mask) | (~(bits >> shift) & mask)

/* compute black on white bits with a left shift */
#define LBWCHAR(b, mask, bits, shift) (b & ~mask) | (bits << shift)

/* compute black on white bits with a right shift */
#define RBWCHAR(b, mask, bits, shift) (b & ~mask) | (bits >> shift)

/* compute cursor bits with a left shift */
#define LCURSOR(b, mask, bits, shift) (b ^ (bits << shift))

/* compute cursor bits with a right shift */
#define RCURSOR(b, mask, bits, shift) (b ^ (bits >> shift))

/* this is the exported terminal context structure */
typedef char machterm_context_t[sizeof(struct context)];

/* this context is used when a zero pointer is given */
extern struct context _machterm_default_context;

/* built-in 6x13 font */
extern unsigned char _machterm_6x13_font[128 * 13];
extern unsigned char _machterm_6x13_bold[128 * 13];

/* built-in 7x13 font */
extern unsigned char _machterm_7x13_font[128 * 13];
extern unsigned char _machterm_7x13_bold[128 * 13];

/* built-in 9x15 font */
extern unsigned short _machterm_9x15_font[128 * 15];
extern unsigned short _machterm_9x15_bold[128 * 15];

/* switch to a new display buffer */
extern int machterm_buffer(machterm_context_t *machterm_context,
                           char *display_buffer,
                           int display_rowsize);

/* setup display context */
extern void _machterm_setup(context_t t,
                            char initial_cursor,
                            int white_on_black);

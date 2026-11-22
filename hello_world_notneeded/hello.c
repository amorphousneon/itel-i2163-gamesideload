/*
 * hello.c - "Hello World" for the itel i2163 (SC6531E) feature phone.
 *
 * Target:   itel it2163 / it2163A  (Spreadtrum SC6531E, 128x160 GC9106 LCD)
 * Runtime:  fpdoom syscode library  (github.com/ilyakurdyukov/fpdoom)
 * Sideload: SD-card boot (sdboot, recommended) or USB RAM load (spd_dump)
 *
 * What it does:
 *   1. Allocates a 32-byte-aligned RGB565 framebuffer.
 *   2. Hands it to the LCDC and starts the display.
 *   3. Fills the screen black and draws "Hello World" centered, in yellow.
 *   4. Pushes the frame to the panel and keeps it refreshed.
 *   5. Waits for any key press, then exits cleanly.
 *
 * The init order (framebuf_alloc -> sys_framebuffer -> sys_start) and the
 * event loop (sys_event with EVENT_KEYDOWN/END/QUIT) are copied verbatim
 * from fptest/test_display() and fptest/test_keypad() in the fpdoom tree,
 * so they match what the SC6531E syscode library expects.
 */

#include <stdint.h>
#include <stdlib.h>
#include "syscode.h"
#include "cmd_def.h"
#include "font5x7.h"

/* ------------------------------------------------------------------ *
 *  App callback required by the syscode library.
 *  Called during sys_init() (before main) to copy the probed LCD
 *  geometry from w1/h1 into the working w2/h2 fields the app reads.
 * ------------------------------------------------------------------ */
void lcd_appinit(void)
{
	struct sys_display *disp = &sys_data.display;
	disp->w2 = disp->w1;
	disp->h2 = disp->h1;
}

/* Called by syscode during init to build the two key-translation tables
 * (normal and power-key) from the phone's raw keymap. Copied verbatim
 * from fptest/main.c - every fpdoom app provides this. */
void keytrn_init(void)
{
	uint8_t keymap[64];
	int i, flags = sys_getkeymap(keymap);
	(void)flags;

#define FILL_KEYTRN(j) \
	for (i = 0; i < 64; i++) { \
		sys_data.keytrn[j][i] = keymap[i] | j << 15; \
	}

	FILL_KEYTRN(0)
	FILL_KEYTRN(1)
#undef FILL_KEYTRN
}

/* ------------------------------------------------------------------ *
 *  Framebuffer (RGB565, base aligned to 32 bytes for the LCDC DMA).
 * ------------------------------------------------------------------ */
static void *framebuf_mem = NULL;
static uint16_t *framebuf = NULL;

static void framebuf_alloc(void)
{
	struct sys_display *disp = &sys_data.display;
	int w = disp->w2, h = disp->h2;
	size_t size = (size_t)w * (size_t)h;
	uint8_t *p;
	framebuf_mem = p = malloc(size * 2 + 31);
	p += -(intptr_t)p & 31;          /* align the base up to 32 */
	framebuf = (uint16_t *)p;
}

/* ------------------------------------------------------------------ *
 *  RGB565 color palette.
 * ------------------------------------------------------------------ */
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_YELLOW  0xFFE0   /* R=31 G=63 B=0 */
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F

/* ------------------------------------------------------------------ *
 *  Drawing primitives.
 * ------------------------------------------------------------------ */
static inline void draw_pixel(int x, int y, uint16_t color)
{
	struct sys_display *disp = &sys_data.display;
	int w = disp->w2, h = disp->h2;
	if ((unsigned)x >= (unsigned)w || (unsigned)y >= (unsigned)h)
		return;
	framebuf[y * w + x] = color;
}

static void fill_screen(uint16_t color)
{
	struct sys_display *disp = &sys_data.display;
	size_t n = (size_t)disp->w2 * (size_t)disp->h2;
	uint16_t *p = framebuf;
	while (n--)
		*p++ = color;
}

/* Draw one 5x7 glyph with a 1-pixel background column on the right so
 * adjacent characters get natural spacing. */
static void draw_char(int x, int y, char c, uint16_t fg, uint16_t bg)
{
	unsigned char uc = (unsigned char)c;
	int col, row;
	if (uc < 0x20 || uc > 0x7E)
		uc = '?';
	const uint8_t *g = font5x7[uc - 0x20];
	for (col = 0; col < 5; col++) {
		uint8_t bits = g[col];
		for (row = 0; row < 7; row++)
			draw_pixel(x + col, y + row, (bits & (1 << row)) ? fg : bg);
	}
	for (row = 0; row < 7; row++)
		draw_pixel(x + 5, y + row, bg);
}

static void draw_string(int x, int y, const char *s, uint16_t fg, uint16_t bg)
{
	while (*s) {
		draw_char(x, y, *s, fg, bg);
		x += 6;   /* 5 px glyph + 1 px spacing */
		s++;
	}
}

static int string_width(const char *s)
{
	int n = 0;
	while (*s++)
		n++;
	return n > 0 ? n * 6 - 1 : 0;   /* no trailing space after last glyph */
}

/* ------------------------------------------------------------------ *
 *  Entry point.
 * ------------------------------------------------------------------ */
int main(int argc, char **argv)
{
	(void)argc; (void)argv;

	struct sys_display *disp;
	const char *msg;
	int sw, sh, x, y;
	uint32_t last_refresh;

	/* Init order verified against fptest/test_display():
	 *   framebuf_alloc()   - w2/h2 already set by syscode's lcd_appinit
	 *   sys_framebuffer()  - tell the LCDC where the buffer lives
	 *   sys_start()        - bring the LCD controller up
	 * then draw, then push frames with sys_start_refresh/wait_refresh. */
	framebuf_alloc();
	sys_framebuffer(framebuf);
	sys_start();

	disp = &sys_data.display;

	/* Black field, then "Hello World" centered. On the itel i2163A
	 * (128x160) the 11-char string is 65px wide, so x=31, y=76. */
	fill_screen(COLOR_BLACK);
	msg = "Hello World";
	sw = string_width(msg);
	sh = 7;
	x = ((int)disp->w2 - sw) / 2;
	y = ((int)disp->h2 - sh) / 2;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	draw_string(x, y, msg, COLOR_YELLOW, COLOR_BLACK);

	/* Push the frame to the panel once. */
	sys_start_refresh();
	sys_wait_refresh();
	last_refresh = sys_timer_ms();

	/* Keep the image on screen; any fresh key press exits.
	 * The loop mirrors fptest/test_keypad(): sys_event fills *key with
	 * the scan code (bit 15 = power-key flag, low byte = KEYPAD_ENUM
	 * code). EVENT_END is the idle tick; EVENT_QUIT is power-off. We
	 * only break on KEYDOWN so a leftover KEYUP from the boot key does
	 * not dismiss the screen instantly. */
	for (;;) {
		int key = 0;
		int ev = sys_event(&key);
		if (ev == EVENT_KEYDOWN)
			break;
		if (ev == EVENT_QUIT)
			break;
		if (ev == EVENT_END) {
			sys_wait_ms(10);
			/* Re-push the frame every ~0.5s in case the panel needs it. */
			if (sys_timer_ms() - last_refresh > 500) {
				sys_start_refresh();
				sys_wait_refresh();
				last_refresh = sys_timer_ms();
			}
		}
	}

	return 0;
}

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "spinlock.h"
#include "gui_base.h"
#include "character.h"
#include "bitmap.h"

struct spinlock gui_lock;

void initGUI() {
    uint GraphicMem = KERNBASE + 0x1028;
    uint baseAdd = *((uint*)GraphicMem);
    screen = (RGB*)baseAdd;
    SCREEN_WIDTH = *((ushort*)(KERNBASE + 0x1012));
    SCREEN_HEIGHT = *((ushort*)(KERNBASE + 0x1014));
    screen_size = (SCREEN_WIDTH * SCREEN_HEIGHT) * 3;
    screen_buf1 = (RGB*)(baseAdd + screen_size);
    screen_buf2 = (RGB*)(baseAdd + screen_size * 2);
    initlock(&gui_lock, "gui");

    uint x,y;
    uchar *b = (uchar *)screen;
    for (x=0;x<SCREEN_WIDTH;x++)
        for (y=0;y<SCREEN_HEIGHT;y++)
        {
            b[0]=0xFF;
            b[1]=0x00;
            b[2]=0x00;
            b+=3;
        }

    cprintf("@Screen Width:   %d\n", SCREEN_WIDTH);
    cprintf("@Screen Height:  %d\n", SCREEN_HEIGHT);
    cprintf("@Bits per pixel: %d\n",*((uchar*)(KERNBASE+0x1019)));
    cprintf("@Video card drivers initialized successfully.\n");
}

void drawPoint(RGB* color, RGB origin) {
    color->R = origin.R;
    color->G = origin.G;
    color->B = origin.B;
}

void drawPointAlpha(RGB* color, RGBA origin) {
    float alpha;
    if (origin.A == 255) {
        color->R = origin.R;
        color->G = origin.G;
        color->B = origin.B;
        return;
    }
    if (origin.A == 0) {
        return;
    }
    alpha = (float)origin.A / 255;
    color->R = color->R * (1 - alpha) + origin.R * alpha;
    color->G = color->G * (1 - alpha) + origin.G * alpha;
    color->B = color->B * (1 - alpha) + origin.B * alpha;
}

int drawCharacter(RGB *buf, int x, int y, char ch, RGBA color) {
    int i, j;
    RGB *t;
    int ord = ch - 0x20;
    if (ord < 0 || ord >= (CHARACTER_NUMBER - 1)) {
        return -1;
    }
    for (i = 0; i < CHARACTER_HEIGHT; i++) {
        if (y + i > SCREEN_HEIGHT || y + i < 0) {
            break;
        }
        for (j = 0; j < CHARACTER_WIDTH; j++) {
            if (character[ord][i][j] == 1) {
                if (x + j > SCREEN_WIDTH || x + j < 0) {
                    break;
                }
                t = buf + (y + i) * SCREEN_WIDTH + x + j;
                drawPointAlpha(t, color);
            }
        }
    }
    return CHARACTER_WIDTH;
}

void drawString(RGB *buf, int x, int y, char *str, RGBA color) {
    int offset_x = 0;

    while (*str != '\0') {
        offset_x += drawCharacter(buf, x + offset_x, y, *str, color);
        str++;
    }
}

void drawImage(RGB *buf, RGBA *img, int x, int y, int width, int height) {
    int i, j;
    RGB *t;
    RGBA *o;
    for (i = 0; i < height; i++) {
        if (y + i > SCREEN_HEIGHT || y + i < 0) {
            break;
        }
        for (j = 0; j < width; j++) {
            if (x + j > SCREEN_WIDTH || x + j < 0) {
                break;
            }
            t = buf + (y + i) * SCREEN_WIDTH + x + j;
            o = img + i * width + j;
            drawPointAlpha(t, *o);
        }
    }
}

void sys_hello() {
    RGBA *image;
    int i;
    int h, w;
    argint(0, &i);
    argint(1, &h);
    argint(2, &w);
    cprintf("size: %d * %d", h, w);
    image = (RGBA *)i;
    RGBA color;
    color.A = 255;
    color.G = 255;
    drawString(screen, 100, 200, "Hello World!", color);
    drawImage(screen, image, 100, 100, w, h);
}
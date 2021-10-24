/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2021  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 */
#include <gfx.h>
#include <string.h>

enum {
    MT_NONE,
    MT_DEL,
    MT_RESET,
    MT_RESETALL,
    MT_NUM,
    MT_DOT,
    MT_ADD,
    MT_SUB,
    MT_MUL,
    MT_DIV,
    MT_EQ,
    MT_NEG,
    MT_INV,
    MT_SQR,
    MT_SQRT,
};
typedef struct btn btn_t;
struct btn {
    int x, y, w, h;
    char txt[8];
    int math;
    int clr;
};

typedef struct panel panel_t;
struct panel {
    int cols, rows, cells;
    btn_t *btns;
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

uint32_t colors[] = {
    //    0xe2e2f8, 0xf8e2f8, 0xf8f8f8, 0xbfd9f3, 0x000000 // Perso wierd
    // 0xbfbfbf, 0xd9d9d9, 0xf3f3f3, 0x73a3c9, 0x000000 // Windows
    0x242424, 0x565656, 0x848484, 0x10a6a6, 0xffffff // Dark
};

btn_t btnStandard[] = {
    { 0, 5, 1, 1, "+/-", MT_NEG, 2 },
    { 1, 5, 1, 1, "0", MT_NUM, 2 },
    { 2, 5, 1, 1, ",", MT_DOT, 2 },
    { 3, 5, 1, 1, "=", MT_EQ, 1 },

    { 0, 4, 1, 1, "1", MT_NUM, 2 },
    { 1, 4, 1, 1, "2", MT_NUM, 2 },
    { 2, 4, 1, 1, "3", MT_NUM, 2 },
    { 3, 4, 1, 1, "+", MT_ADD, 1 },

    { 0, 3, 1, 1, "4", MT_NUM, 2 },
    { 1, 3, 1, 1, "5", MT_NUM, 2 },
    { 2, 3, 1, 1, "6", MT_NUM, 2 },
    { 3, 3, 1, 1, "-", MT_SUB, 1 },

    { 0, 2, 1, 1, "7", MT_NUM, 2 },
    { 1, 2, 1, 1, "8", MT_NUM, 2 },
    { 2, 2, 1, 1, "9", MT_NUM, 2 },
    { 3, 2, 1, 1, "\xc3\x97", MT_MUL, 1 },

    { 0, 1, 1, 1, "1/x", MT_INV, 1 },
    { 1, 1, 1, 1, "x\xc2\xb2", MT_SQR, 1 },
    { 2, 1, 1, 1, "\xe2\x88\x9ax", MT_SQRT, 1 },
    { 3, 1, 1, 1, "\xc3\xb7", MT_DIV, 1 },

    { 0, 0, 1, 1, "%", NULL, 1 },
    { 1, 0, 1, 1, "CE", MT_RESET, 1 },
    { 2, 0, 1, 1, "C", MT_RESETALL, 1 },
    { 3, 0, 1, 1, "\xe2\x8c\xab", MT_DEL, 1 },
};

btn_t btnScientific[] = {

    { 0, 7, 1, 1, "ln", NULL, 1 },
    { 1, 7, 1, 1, "+/-", MT_NEG, 2 },
    { 2, 7, 1, 1, "0", MT_NUM, 2 },
    { 3, 7, 1, 1, ",", NULL, 2 },
    { 4, 7, 1, 1, "=", NULL, 1 },

    { 0, 6, 1, 1, "log", NULL, 1 },
    { 1, 6, 1, 1, "1", MT_NUM, 2 },
    { 2, 6, 1, 1, "2", MT_NUM, 2 },
    { 3, 6, 1, 1, "3", MT_NUM, 2 },
    { 4, 6, 1, 1, "+", NULL, 1 },

    { 0, 5, 1, 1, "10^x", NULL, 1 },
    { 1, 5, 1, 1, "4", MT_NUM, 2 },
    { 2, 5, 1, 1, "5", MT_NUM, 2 },
    { 3, 5, 1, 1, "6", MT_NUM, 2 },
    { 4, 5, 1, 1, "-", NULL, 1 },

    { 0, 4, 1, 1, "x^y", NULL, 1 },
    { 1, 4, 1, 1, "7", MT_NUM, 2 },
    { 2, 4, 1, 1, "8", MT_NUM, 2 },
    { 3, 4, 1, 1, "9", MT_NUM, 2 },
    { 4, 4, 1, 1, "\xc3\x97", NULL, 1 },

    { 0, 3, 1, 1, "\xe2\x88\x9ax", NULL, 1 },
    { 1, 3, 1, 1, "(", NULL, 1 },
    { 2, 3, 1, 1, ")", NULL, 1 },
    { 3, 3, 1, 1, "x!", NULL, 1 },
    { 4, 3, 1, 1, "\xc3\xb7", NULL, 1 },

    { 0, 2, 1, 1, "x\xc2\xb2", NULL, 1 },
    { 1, 2, 1, 1, "1/x", NULL, 1 },
    { 2, 2, 1, 1, "|x|", NULL, 1 },
    { 3, 2, 1, 1, "exp", NULL, 1 },
    { 4, 2, 1, 1, "mod", NULL, 1 },

    { 0, 1, 1, 1, "2nd", NULL, 1 },
    { 1, 1, 1, 1, "PI", NULL, 1 },
    { 2, 1, 1, 1, "E", NULL, 1 },
    { 3, 1, 1, 1, "CE", MT_RESET, 1 },
    { 4, 1, 1, 1, "C", MT_RESETALL, 1 },

    { 0, 0, 2, 1, "Trigo", NULL, 1 },
    { 2, 0, 2, 1, "Func", NULL, 1 },
    { 4, 0, 1, 1, "\xe2\x8c\xab", MT_DEL, 1 },
};

panel_t pnlStandard = {
    4, 6, 24, btnStandard,
};

panel_t pnlScientific = {
    5, 8, 38, btnScientific,
};

#define DGT_BUF_SZ 64
char result[DGT_BUF_SZ];
bool viewDirty;
btn_t *overBtn = NULL;
btn_t *downBtn = NULL;
panel_t *_panel = &pnlStandard;
gfx_font_t *fonts[16];

enum {
    FO_TITLE,
    FO_BTN,
    FO_BTN2,
    FO_H1,
    FO_H2,
    FO_H3,
    FO_H4,
    FO_H5,
    FO_H6,
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

bool btnSize(gfx_t *win, btn_t *btn, gfx_clip_t *rc)
{
    int sx = 0, sy = 80, sw = win->width, sh = win->height - sy, mg = 1, pw = _panel->cols, ph = _panel->rows;
    int bw = btn->w * sw / pw;
    int bh = btn->h * sh / ph;
    rc->left = sx + btn->x * sw / pw + mg;
    rc->right = rc->left + bw - 2 * mg;
    rc->top = sy + btn->y * sh / ph + mg;
    rc->bottom = rc->top + bh - 2 * mg;
    bool over = win->seat->mouse_x >= rc->left &&
                win->seat->mouse_x < rc->right &&
                win->seat->mouse_y >= rc->top &&
                win->seat->mouse_y < rc->bottom;
    return over;
}

btn_t *hitBtn(gfx_t *win)
{
    for (int i = 0; i < _panel->cells; ++i) {
        btn_t *btn = &_panel->btns[i];
        gfx_clip_t rc;
        bool over = btnSize(win, btn, &rc);
        if (over)
            return btn;
    }
    return NULL;
}

void mouseMove(gfx_t *win)
{
    btn_t *btn = NULL;
    if (win->seat->mouse_y >= 80)
        btn = hitBtn(win);
    if (btn != overBtn) {
        overBtn = btn;
        viewDirty = true;
    }
}

void mouseDown(gfx_t *win)
{
    downBtn = overBtn;
}

void mouseUp(gfx_t *win)
{
    if (overBtn == downBtn && overBtn != NULL) {
        // printf("Click on => %s\n", downBtn->txt);
        viewDirty = true;
        // TODO -- Buffer overflow ! not protected yet !!
        switch (downBtn->math) {
        case MT_NUM:
            if (strcmp(result, "0") == 0)
                strcpy(result, downBtn->txt);
            else if (strcmp(result, "-0") == 0)
                strcpy(&result[1], downBtn->txt);
            else
                strncat(result, downBtn->txt, DGT_BUF_SZ);
            break;
        case MT_DOT:
            if (strchr(result, '.') == NULL)
                strncat(result, ".", DGT_BUF_SZ);
            break;
        case MT_DEL:
            result[strlen(result) - 1] = '\0';
            if (strcmp(result, "") == 0 || strcmp(result, "-") == 0)
                strcpy(result, "0");
            break;
        case MT_RESET:
        case MT_RESETALL:
            strcpy(result, "0");
            break;
        case MT_NEG:
            if (strcmp(result, "0") != 0) {
                if (result[0] == '-')
                    memmove(result, &result[1], DGT_BUF_SZ - 1);
                else {
                    memmove(&result[1], result, DGT_BUF_SZ - 2);
                    result[0] = '-';
                    result[DGT_BUF_SZ - 1] = '\0';
                }
            }
            break;

        case MT_EQ:
        case MT_ADD:
        case MT_SUB:
        case MT_MUL:
        case MT_DIV:
        case MT_INV:
        case MT_SQR:
        case MT_SQRT:
        default:
            break;
        }
    }

    downBtn = NULL;
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


void paint(gfx_t *win)
{
    if (viewDirty == false)
        return;
    gfx_map(win);
    gfx_fill(win, colors[0], GFX_NOBLEND, NULL);

    bool writen = false;
    int th = 80;
    int fh = FO_H1;
    gfx_text_metrics_t m;
    while (!writen && fh <= FO_H6) {
        gfx_mesure_text(fonts[fh], result, &m);
        if (m.width + 10 > win->width) {
            fh++;
            continue;
        }
        gfx_write(win, fonts[fh], result, colors[4], win->width - 5 - m.width, (th + m.baseline) / 2, NULL);
        writen = true;
    }

    int sx = 0, sy = th, sw = win->width, sh = win->height - sy, mg = 1, pw = _panel->cols, ph = _panel->rows;
    for (int i = 0; i < _panel->cells; ++i) {
        btn_t *btn = &_panel->btns[i];
        gfx_clip_t rc;
        bool over = btnSize(win, btn, &rc);
        int bw = btn->w * sw / pw;
        int bh = btn->h * sh / ph;
        gfx_fill(win, over ? colors[3] : colors[btn->clr], GFX_NOBLEND, &rc);
        gfx_mesure_text(fonts[FO_BTN], btn->txt, &m);
        int tx = rc.left + (bw - m.width) / 2;
        int ty = rc.top + (bh + m.baseline) / 2;
        gfx_write(win, fonts[FO_BTN], btn->txt, colors[4], tx, ty, &rc);
    }

    gfx_flip(win, NULL);
    viewDirty = false;
}

#include <stdio.h>
int unichar(const char **);
int utf8char(int uni, char *buf);

int main()
{
    gfx_t *win = gfx_create_window(260, 320);
    fonts[FO_TITLE] = gfx_font("Arial", 13, 0);
    fonts[FO_BTN] = gfx_font("Arial", 13, 0);
    fonts[FO_BTN2] = gfx_font("Arial", 13, 0);
    fonts[FO_H1] = gfx_font("Arial", 26, 0);
    fonts[FO_H2] = gfx_font("Arial", 21, 0);
    fonts[FO_H3] = gfx_font("Arial", 17, 0);
    fonts[FO_H4] = gfx_font("Arial", 14, 0);
    fonts[FO_H5] = gfx_font("Arial", 11, 0);
    fonts[FO_H6] = gfx_font("Arial", 9, 0);


    strcpy(result, "0");
    viewDirty = true;

    gfx_msg_t msg;
    gfx_timer(0, 50);
    while (gfx_poll(&msg) == 0) {
        gfx_handle(&msg);
        if (msg.message == GFX_EV_TIMER)
            paint(win);
        else if (msg.message == GFX_EV_QUIT)
            break;
        else if (msg.message == GFX_EV_BTNDOWN)
            mouseDown(win);
        else if (msg.message == GFX_EV_BTNUP)
            mouseUp(win);
        else if (msg.message == GFX_EV_MOUSEMOVE)
            mouseMove(win);
        else if (msg.message == GFX_EV_RESIZE)
            viewDirty = true;
    }

    gfx_destroy(win);
    return 0;
}

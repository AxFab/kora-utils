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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MS_BOMB 1
#define MS_SHOW 2
#define MS_MARK 4

typedef struct cell cell_t;

struct cell {
    int bombs;
    int mark;
};

struct board {
    int rows, cols;
    int bombs, showns;
    bool lock, dirty;
    cell_t *board;
    gfx_t *win;
    gfx_font_t *font;
};

struct board _;

void paint()
{
    if (!_.dirty)
        return;
    gfx_map(_.win);
    gfx_fill(_.win, 0x343434, GFX_NOBLEND, NULL);

    int ds = 10;
    int dy = 50;
    int cw = (_.win->width - ds * 2) / _.cols;
    int ch = (_.win->height - dy - ds * 2) / _.rows;
    int sz = cw < ch ? cw : ch;
    char tmp[2];
    gfx_text_metrics_t m;
    for (int i = 0, n = _.rows * _.cols; i < n; ++i) {
        int x = i / _.rows;
        int y = i % _.rows;
        int k = y * _.cols + x;
        gfx_clip_t rc;
        // board[i].bombs = ;
        rc.left = ds + x * sz + 1;
        rc.top = dy + y * sz + 1;
        rc.right = rc.left + sz - 2;
        rc.bottom = rc.top + sz - 2;
        uint32_t color = 0xd8d8d8;
        if (_.board[k].mark & MS_SHOW) {
            if (_.board[k].mark & MS_BOMB)
                color = 0xa61010;
            else
                color = 0x848484;
        } else if (_.board[k].mark & MS_MARK)
            color = 0x10a6a6;

        gfx_fill(_.win, color, GFX_NOBLEND, &rc);
        if (_.board[k].bombs != 0 && _.board[k].mark & MS_SHOW) {
            snprintf(tmp, 2, "%d", _.board[k].bombs);
            gfx_mesure_text(_.font, tmp, &m);
            gfx_write(_.win, _.font, tmp, 0xf0f0f0,
                      rc.left + (sz - 2 - m.width) / 2,
                      rc.top + (sz - 2 + m.baseline) / 2, &rc);
        }
    }

    gfx_flip(_.win, NULL);
    _.dirty = false;
}

void init()
{
    _.rows = 10;
    _.cols = 10;
    _.bombs = 15;
    _.showns = 0;
    _.dirty = true;
    if (_.board != NULL)
        free(_.board);
    _.board = calloc(_.rows * _.cols, sizeof(cell_t));
    int bombs = _.bombs;
    while (bombs > 0) {
        int y = rand() % _.rows;
        int x = rand() % _.cols;
        int k = y * _.cols + x;
        if (_.board[k].mark & MS_BOMB)
            continue;
        _.board[k].mark |= MS_BOMB;
        bombs--;
    }

    for (int i = 0, n = _.rows * _.cols; i < n; ++i) {
        int x = i / _.rows;
        int y = i % _.rows;
        int k = y * _.cols + x;
        if (_.board[k].mark & MS_BOMB)
            continue;
        if (x > 0) {
            if (y > 0)
                if (_.board[k - _.cols - 1].mark & MS_BOMB)
                    _.board[k].bombs++;
            if (_.board[k - 1].mark & MS_BOMB)
                _.board[k].bombs++;
            if (y < _.rows - 1)
                if (_.board[k + _.cols - 1].mark & MS_BOMB)
                    _.board[k].bombs++;
        }
        {
            if (y > 0)
                if (_.board[k - _.cols].mark & MS_BOMB)
                    _.board[k].bombs++;
            if (y < _.rows - 1)
                if (_.board[k + _.cols].mark & MS_BOMB)
                    _.board[k].bombs++;
        }
        if (x < _.cols - 1) {
            if (y > 0)
                if (_.board[k - _.cols + 1].mark & MS_BOMB)
                    _.board[k].bombs++;
            if (_.board[k + 1].mark & MS_BOMB)
                _.board[k].bombs++;
            if (y < _.rows - 1)
                if (_.board[k + _.cols + 1].mark & MS_BOMB)
                    _.board[k].bombs++;
        }
    }
}

void reveals(int x, int y)
{
    int k = y * _.cols + x;
    if (_.board[k].mark & MS_SHOW)
        return;
    if (_.board[k].bombs != 0) {
        _.board[k].mark |= MS_SHOW;
        _.showns++;
        return;
    }

    _.board[k].mark |= MS_SHOW;
    _.showns++;
    if (x > 0) {
        if (y > 0)
            reveals(x - 1, y - 1);
        reveals(x - 1, y);
        if (y < _.rows - 1)
            reveals(x - 1, y + 1);
    }
    {
        if (y > 0)
            reveals(x, y - 1);
        reveals(x, y);
        if (y < _.rows - 1)
            reveals(x, y + 1);
    }
    if (x < _.cols - 1) {
        if (y > 0)
            reveals(x + 1, y - 1);
        reveals(x + 1, y);
        if (y < _.rows - 1)
            reveals(x + 1, y + 1);
    }
}

void click()
{
    if (_.win->seat->mouse_y < 50) {
        init();
        return;
    }

    if (_.bombs == 0)
        return;

    int ds = 10;
    int dy = 50;
    int cw = (_.win->width - ds * 2) / _.cols;
    int ch = (_.win->height - dy - ds * 2) / _.rows;
    int sz = cw < ch ? cw : ch;

    int x = (_.win->seat->mouse_x - ds) / sz;
    int y = (_.win->seat->mouse_y - dy) / sz;

    int k = y * _.cols + x;
    if (_.board[k].mark & (MS_SHOW | MS_MARK))
        return;

    _.dirty = true;
    _.board[k].mark |= MS_SHOW;
    _.showns++;
    if (_.board[k].mark & MS_BOMB) {
        _.bombs = 0;
        if (_.showns <= 1) {
            init();
            click();
        }
    } else if (_.board[k].bombs == 0) {
        _.board[k].mark &= ~MS_SHOW;
        _.showns--;
        reveals(x, y);
    } else if (_.showns + _.bombs >= _.rows * _.cols) {
        _.bombs = 0;
        printf("Victory\n");
    }
}

void mark()
{
    if (_.win->seat->mouse_y < 50)
        return;

    if (_.bombs == 0)
        return;
    int ds = 10;
    int dy = 50;
    int cw = (_.win->width - ds * 2) / _.cols;
    int ch = (_.win->height - dy - ds * 2) / _.rows;
    int sz = cw < ch ? cw : ch;

    int x = (_.win->seat->mouse_x - ds) / sz;
    int y = (_.win->seat->mouse_y - dy) / sz;

    int k = y * _.cols + x;
    if (_.board[k].mark & MS_SHOW)
        return;

    _.board[k].mark ^= MS_MARK;
    _.dirty = true;
}

int main()
{
#ifdef _WIN32
    // Sleep(5 * 1000);
    gfx_context("win32");
#endif

    srand((unsigned)time(NULL));
    _.win = gfx_create_window(320, 370);
    _.font = gfx_font("Arial", 10, GFXFT_BOLD);
    _.board = NULL;

    gfx_msg_t msg;
    init();
    gfx_timer(0, 50);
    while (gfx_poll(&msg) == 0) {
        gfx_handle(&msg);
        if (msg.message == GFX_EV_TIMER)
            paint();
        else if (msg.message == GFX_EV_QUIT)
            break;
        else if (msg.message == GFX_EV_RESIZE)
            _.dirty = true;
        else if (msg.message == GFX_EV_BTNUP) {
            if (msg.param1 == 1)
                click();
            else if (msg.param1 == 2)
                mark();
        }
    }

    gfx_destroy(_.win);
    return 0;
}

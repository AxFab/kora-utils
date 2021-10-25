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
#include <font-awesome-5.h>

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
    gfx_font_t* font;
    gfx_font_t* fontHead;
    gfx_font_t* fontSign;
    gfx_font_t* fontSignTop;
    int time;
    int score;
    int marked;
    time_t start;
};

struct board _;


void paint()
{
    gfx_clip_t rc;
    gfx_text_metrics_t m;
    if (!_.dirty)
        return;
    gfx_map(_.win);
    gfx_fill(_.win, 0x343434, GFX_NOBLEND, NULL);

    char buf[12];
    if (_.showns != 0) {
        if (_.score == 0)
            _.time = time(NULL) - _.start;
        snprintf(buf, 12, "%d:%02d", _.time / 60, _.time % 60);
    }
    else
        snprintf(buf, 12, "0:00");
    gfx_mesure_text(_.fontHead, buf, &m);
    gfx_write(_.win, _.fontHead, buf, 0xffffff, 10, (50 - m.height) / 2 + m.baseline, NULL);

    snprintf(buf, 12, "%d / %d", _.marked, _.bombs);
    gfx_mesure_text(_.fontHead, buf, &m);
    gfx_write(_.win, _.fontHead, buf, 0xffffff, _.win->width - 10 - m.width, (50 - m.height) / 2 + m.baseline, NULL);


    rc.left = _.win->width / 2 - 20;
    rc.right = _.win->width / 2 + 20;
    rc.top = 50 / 2 - 20;
    rc.bottom = 50 / 2 + 20;
    gfx_fill(_.win, 0x966010, GFX_NOBLEND, &rc);

    rc.left += 2;
    rc.right -= 2;
    rc.top += 2;
    rc.bottom -= 2;
    gfx_fill(_.win, 0xa69610, GFX_NOBLEND, &rc);

    // smile-beam: "\xef\x96\xb8"
    // sad-tear: "\xef\x96\xb4"
    // grin-stars: \xef\x96\x87
    // flag: \xef\x80\xa4
    const char* head = FA_SMILE_BEAM;
    if (_.score == 0)
        head = FA_SMILE_BEAM;
    else if (_.score == 1)
        head = FA_GRIN_STARS;
    else if (_.score == -1)
        head = FA_SAD_TEAR;

    strcpy(buf, head);
    gfx_mesure_text(_.fontSignTop, buf, &m);
    gfx_write(_.win, _.fontSignTop, buf, 0xffffff, (_.win->width - m.width) / 2, (50 - m.height) / 2 + m.baseline, NULL);


    int ds = 10;
    int dy = 50;
    int cw = (_.win->width - ds * 2) / _.cols;
    int ch = (_.win->height - dy - ds * 2) / _.rows;
    int sz = cw < ch ? cw : ch;
    char tmp[2];
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
        else if (_.board[k].mark & MS_MARK) {
            gfx_mesure_text(_.fontSign, FA_FLAG, &m);
            gfx_write(_.win, _.fontSign, FA_FLAG, 0xf0f0f0,
                rc.left + (sz - 2 - m.width) / 2,
                rc.top + (sz - 2 + m.baseline) / 2, &rc);
        }
        else if ((_.board[k].mark & MS_SHOW || _.score != 0) && _.board[k].mark & MS_BOMB) {
            uint32_t color = _.board[k].mark & MS_SHOW ? 0xf0f0f0 : 0x101010;
            gfx_mesure_text(_.fontSign, FA_BOMB, &m);
            gfx_write(_.win, _.fontSign, FA_BOMB, color,
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
    _.score = 0;
    _.marked = 0;
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

    if (_.score != 0)
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
    if (_.showns == 0)
        _.start = time(NULL);
    _.showns++;
    if (_.board[k].mark & MS_BOMB) {
        _.score = -1;
        if (_.showns <= 1) {
            init();
            click();
        }
    } else if (_.board[k].bombs == 0) {
        _.board[k].mark &= ~MS_SHOW;
        _.showns--;
        reveals(x, y);
    } 
    
    if (_.score == 0 && _.showns + _.bombs >= _.rows * _.cols) {
        _.score = 1;
    }
}

void mark()
{
    if (_.win->seat->mouse_y < 50)
        return;

    if (_.score != 0)
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
    if (_.board[k].mark & MS_MARK)
        _.marked++;
    else
        _.marked--;
    _.dirty = true;
}

int main()
{
    srand((unsigned)time(NULL));
    _.win = gfx_create_window(320, 370);
    _.font = gfx_font("Arial", 10, GFXFT_BOLD);
    _.fontHead = gfx_font("Arial", 20, GFXFT_REGULAR);
    _.fontSign = gfx_font("Font Awesome 5 Free", 10, GFXFT_SOLID);
    _.fontSignTop = gfx_font("Font Awesome 5 Free", 20, GFXFT_SOLID);
    _.board = NULL;

    gfx_msg_t msg;
    init();
    gfx_timer(0, 50);
    int dl = 500 / 50;
    while (gfx_poll(&msg) == 0 && msg.message != GFX_EV_QUIT) {
        gfx_handle(&msg);
        switch (msg.message) {
        case GFX_EV_TIMER:
            if (--dl == 0) {
                _.dirty = true;
                dl = 500 / 50;
            }
            paint();
            break;
        case GFX_EV_RESIZE:
            _.dirty = true;
            break;
        case GFX_EV_BTNUP:
            if (msg.param1 == 1)
                click();
            else if (msg.param1 == 2)
                mark();
            break;
        }
    }

    gfx_destroy(_.win);
    return 0;
}

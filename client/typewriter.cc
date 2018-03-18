// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
// Copyright (C) 2016 Henner Zeller <h.zeller@acm.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#include "render_text.h"
#include "bdf-font.h"

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

#include <string>

#define DEFAULT_WIDTH 45
#define DEFAULT_HEIGHT 35
#define WIDEST_GLYPH 'W'

volatile bool got_ctrl_c = false;
static void InterruptHandler(int) {
  got_ctrl_c = true;
}

int main(int argc, char *argv[]) {
    ft::Font text_font;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <font> <host> [rows]\n", argv[0]);
        return 1;
    }

    text_font.LoadFont(argv[1]);

    int rows = 1;

    if (argc > 2) {
        
    }

    char date_s[64], time_s[64], unix_time_s[64];

    struct timespec next_time;
    next_time.tv_sec = time(NULL);
    next_time.tv_nsec = 0;
    struct tm tm;

    TextOps ops;
    ops.fg.r = 255;
    ops.fg.g = 0;
    ops.fg.b = 255;

    ops.with_outline = true;
    ops.outline.r = 0;
    ops.outline.g = 255;
    ops.outline.b = 0;

    char backspaces[] = {
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32,
    };

    struct termios old_tio, new_tio;
    unsigned char c;
    char term_buf[256];
    size_t term_off = 0;

    /* get the terminal settings for stdin */
    tcgetattr(STDIN_FILENO,&old_tio);

    /* we want to keep the old setting to restore them a the end */
    new_tio=old_tio;

    /* disable canonical mode (buffered i/o) and local echo */
    new_tio.c_lflag &=(~ICANON & ~ECHO);

    /* set the new settings immediately */
    tcsetattr(STDIN_FILENO,TCSANOW,&new_tio);

    bool bksp = false;
    size_t bksp_len = 0;

    do {
        bksp_len = term_off;
        c = getchar();
        if (c == 127) {
            if (term_off > 0) {
                term_buf[term_off-1] = ' ';
                bksp = true;
            }
        } else if (c == 10) {
            render_text(argv[2], std::string(backspaces, term_off).c_str(), text_font, ops, 0, 0, 16);
            term_off = 0;
        } else if (term_off < sizeof(term_buf)-1) {
            term_buf[term_off++] = c;
        }

        render_text(argv[2], std::string(term_buf, term_off).c_str(), text_font, ops, 0, 0, 16);

        if (bksp) {
            bksp = false;
            term_off--;
        }
    } while (c != -1);

    tcsetattr(STDIN_FILENO,TCSANOW,&old_tio);

    return 0;
}

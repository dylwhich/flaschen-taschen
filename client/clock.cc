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
        fprintf(stderr, "Usage: %s <font> <host>\n", argv[0]);
        return 1;
    }

    text_font.LoadFont(argv[1]);

    char date_s[64], time_s[64], unix_time_s[64];

    struct timespec next_time;
    next_time.tv_sec = time(NULL);
    next_time.tv_nsec = 0;
    struct tm tm;

    TextOps ops;
    ops.fg.r = 63;
    ops.fg.g = 0;
    ops.fg.b = 0;

    while (!got_ctrl_c) {
        localtime_r(&next_time.tv_sec, &tm);
        strftime(date_s, 64, "%a %Y-%m-%d", &tm);
        strftime(time_s, 64, "%H:%M:%S %z", &tm);
        strftime(unix_time_s, 64, "%s", &tm);

        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_time, NULL);

        render_text(argv[2], date_s, text_font, ops, 511-6*strlen(date_s), 0, 5);
        render_text(argv[2], time_s, text_font, ops, 511-6*strlen(time_s), 11, 5);
        render_text(argv[2], unix_time_s, text_font, ops, 511-6*strlen(time_s), 22, 5);
        next_time.tv_sec++;
    }

    return 0;
}

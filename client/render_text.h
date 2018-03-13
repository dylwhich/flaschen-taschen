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

#include "udp-flaschen-taschen.h"

#include "bdf-font.h"

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>

#define DEFAULT_WIDTH 45
#define DEFAULT_HEIGHT 35
#define WIDEST_GLYPH 'W'

struct TextOps {
    TextOps()
            : letter_spacing(0),
              vertical(false),
              with_outline(false),
              fg(0xff, 0xff, 0xff),
              bg(0, 0, 0),
              outline(0, 0, 0) {}

    int letter_spacing;
    bool vertical;
    bool with_outline;
    Color fg;
    Color bg;
    Color outline;
};

void render_text(const char *host, const char *text, ft::Font &text_font, TextOps &ops,
                 int off_x = 0, int off_y = 0, int off_z = 1,
                 int width = -1, int height = -1) {

    // check for valid initial conditions
    if (text_font.height() < 0) {
        fprintf(stderr, "Need to provide a font.\n");
        return;
    }

    ft::Font *measure_font = &text_font;
    ft::Font *outline_font = NULL;
    if (ops.with_outline) {
        outline_font = text_font.CreateOutlineFont();
        measure_font = outline_font;
    }

    // check height input and use default value if necessary
    if (height < 0) {
        height = ops.vertical ? DEFAULT_HEIGHT : measure_font->height();
    }

    // check width input and use default font width of WIDEST_GLYPH if necessary
    if (width < 0) {
        width = ops.vertical ? measure_font->CharacterWidth(WIDEST_GLYPH)
                             : measure_font->CharacterWidth(WIDEST_GLYPH) * strlen(text);
    }

    if (width < 1 || height < 1) {
        fprintf(stderr, "%dx%d is a rather unusual size\n", width, height);
        return;
    }

    int fd = OpenFlaschenTaschenSocket(host);
    if (fd < 0) {
        fprintf(stderr, "Cannot connect.\n");
        return;
    }

    UDPFlaschenTaschen display(fd, width, height);
    display.SetOffset(off_x, off_y, off_z);

    // Center in in the available display space.
    const int y_pos = (height - measure_font->height()) / 2
                      + measure_font->baseline();
    const int x_pos = (width - measure_font->CharacterWidth(WIDEST_GLYPH)) / 2
                      + (ops.with_outline ? 1 : 0);

    if (!ops.vertical) {
        display.Fill(ops.bg);
        if (outline_font) {
            DrawText(&display, *outline_font, 0, y_pos, ops.outline, NULL,
                     text, ops.letter_spacing - 2);
        }
        DrawText(&display, text_font, 1, y_pos, ops.fg, NULL,
                 text, ops.letter_spacing);
        display.Send();
    }
    else {
        display.Fill(ops.bg);
        if (outline_font) {
            VerticalDrawText(&display, *outline_font,
                             x_pos - 1, measure_font->height() - 1,
                             ops.outline, NULL, text, ops.letter_spacing - 2);
        }
        VerticalDrawText(&display, text_font, x_pos,
                         measure_font->height() - 1,
                         ops.fg, NULL, text, ops.letter_spacing);
        display.Send();
    }

    close(fd);
    return;
}

/*
 *  Abuse — dark 2D side-scrolling platform game
 *  Copyright © 1995 Crack dot Com
 *  Copyright © 2005—2016 Sam Hocevar <sam@hocevar.net>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Crack dot Com, by
 *  Jonathan Clark, or by Sam Hocevar.
 */

#pragma once

int setback_intersect(int32_t x1, int32_t y1, int32_t &x2, int32_t &y2,
              int32_t xp1, int32_t yp1, int32_t xp2, int32_t yp2, int32_t inside);


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

enum
{
    ANT_START,
    ANT_HANGING = 15,
    ANT_HIDING = 16,
    ANT_FALL_DOWN = 1,
    ANT_LANDING = 9,
    ANT_RUNNING = 2,
    ANT_POUNCE_WAIT = 4,
    ANT_JUMP = 6,
    ANT_FIRE = 8,
    ANT_JUMP_ROOF = 12,
    ANT_ROOF_WALK = 13,
    ANT_CEIL_SHOOT = 14
};

LObject *ant_ai();
void show_stats();


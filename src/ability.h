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

enum ability
{    start_hp,
    start_accel,
    stop_accel,
        jump_xvel,
    jump_yvel,
    run_top_speed,
    jump_top_speed,

    tint_color,
    push_xrange,
    walk_top_speed                  // keep as last entry!
};

#define TOTAL_ABILITIES (walk_top_speed+1)
extern char const *ability_names[TOTAL_ABILITIES];
long get_ability(int who, ability a);
long get_ability_default(ability a);


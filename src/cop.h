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

// functions defined for the main player,  these were translated because they
// are called every tick and they were getting slow/complicated

LObject *top_ai();
LObject *laser_ufun(LObject *args);
LObject *top_ufun(LObject *args);
LObject *plaser_ufun(LObject *args);
LObject *player_rocket_ufun(LObject *args);
LObject *lsaber_ufun(LObject *args);
LObject *cop_mover(int xm, int ym, int but);
LObject *sgun_ai();
LObject *ladder_ai();
LObject *top_draw();
LObject *bottom_draw();
LObject *mover_ai();
LObject *respawn_ai();
LObject *score_draw();
LObject *show_kills();


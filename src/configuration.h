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

enum { HIGH_DETAIL,
       MEDIUM_DETAIL,
       LOW_DETAIL,
       POOR_DETAIL };


void key_bindings(int player, int &left, int &right, int &up, int &down, int &b1, int &b2, int &b3,  int &b4);
void get_key_bindings();
void get_movement(int player, int &x, int &y, int &b1, int &b2, int &b3, int &b4);
void config_cleanup();  // free any memory allocated
int get_keycode(char const *str);  // -1 means not a valid key code


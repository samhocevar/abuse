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

int joy_init(int argc, char **argv); // returns 0 if no joystick is available
void joy_status(int &b1, int &b2, int &b3, int &xv, int &yv);
void joy_calibrate();


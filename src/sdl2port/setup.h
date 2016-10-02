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

struct flags_struct
{
    short fullscreen;
    short doublebuf;
    short mono;
    short nosound;
    short grabmouse;
    short nosdlparachute;
    short xres;
    short yres;
    short overlay;
    short gl;
    int antialias;
    int gamepad;
};

struct keys_struct
{
    int left;
    int right;
    int up;
    int down;
    int b1;
    int b2;
    int b3;
    int b4;
};


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

void set_dprinter(void (*stat_fun)(char *));       // called with debug info
void set_dgetter(void (*stat_fun)(char *, int));   // called mainly by lisp breaker
void dprintf(const char *format, ...);
void dgets(char *buf, int size);


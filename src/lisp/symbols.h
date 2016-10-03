/*
 *  Abuse — dark 2D side-scrolling platform game
 *  Copyright © 1995 Crack dot Com
 *  Copyright © 2005—2016 Sam Hocevar <sam@hocevar.net>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Sam Hocevar.
 */

#pragma once

extern void *colon_initial_contents, *colon_initial_element, *load_warning;

struct func
{
    char const *name;
    short min_args, max_args;
};


/*
 *  Abuse — dark 2D side-scrolling platform game
 *  Copyright © 1995 Crack dot Com
 *  Copyright © 2005—2016 Sam Hocevar <sam@hocevar.net>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Sam Hocevar.
 */

#pragma once

extern LObject *l_undefined;

extern LSymbol *true_symbol, *list_symbol, *string_symbol, *quote_symbol,
     *backquote_symbol, *comma_symbol, *do_symbol, *in_symbol, *aref_symbol,
     *if_symbol, *progn_symbol, *car_symbol, *cdr_symbol;

extern void *colon_initial_contents, *colon_initial_element, *load_warning;

struct func
{
    char const *name;
    short min_args, max_args;
};


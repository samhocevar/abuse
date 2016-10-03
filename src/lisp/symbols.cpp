/*
 *  Abuse — dark 2D side-scrolling platform game
 *  Copyright © 1995 Crack dot Com
 *  Copyright © 2005—2016 Sam Hocevar <sam@hocevar.net>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Crack dot Com, by
 *  Jonathan Clark, or by Sam Hocevar.
 */

#if HAVE_CONFIG_H
#   include "config.h"
#endif

#include "lisp/lisp.h"
#include "lisp/gc.h"
#include "lisp/symbols.h"

LObject *lisp::obj::undefined = nullptr;

LSymbol *lisp::sym::true_ = nullptr;
LSymbol *lisp::sym::list = nullptr;
LSymbol *lisp::sym::string = nullptr;
LSymbol *lisp::sym::quote = nullptr;
LSymbol *lisp::sym::backquote = nullptr;
LSymbol *lisp::sym::comma = nullptr;
LSymbol *lisp::sym::do_ = nullptr;
LSymbol *lisp::sym::in = nullptr;
LSymbol *lisp::sym::aref = nullptr;
LSymbol *lisp::sym::if_ = nullptr;
LSymbol *lisp::sym::progn = nullptr;
LSymbol *lisp::sym::car = nullptr;
LSymbol *lisp::sym::cdr = nullptr;

LSymbol *lisp::sym::eq = nullptr;
LSymbol *lisp::sym::zero = nullptr;
LSymbol *lisp::sym::eq0 = nullptr;

LSymbol *lisp::sym::if_1progn = nullptr;
LSymbol *lisp::sym::if_2progn = nullptr;
LSymbol *lisp::sym::if_12progn = nullptr;
LSymbol *lisp::sym::not_ = nullptr;

void *colon_initial_contents;
void *colon_initial_element;
void *load_warning;


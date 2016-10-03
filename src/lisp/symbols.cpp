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

LSymbol *colon_initial_contents;
LSymbol *colon_initial_element;
LSymbol *load_warning;

void lisp::init_constants()
{
    // This needs to be defined first
    LSymbol *tmp = lisp::make_sym(":UNDEFINED");
    lisp::obj::undefined = tmp;
    // Collection problems result if we don't do this
    tmp->m_function = nullptr;
    tmp->m_value = nullptr;

    sym::true_ = lisp::make_sym("T");

    sym::list = lisp::make_sym("list");
    sym::string = lisp::make_sym("string");
    sym::quote = lisp::make_sym("quote");
    sym::backquote = lisp::make_sym("backquote");
    sym::comma = lisp::make_sym("comma");
    sym::in = lisp::make_sym("in");
    sym::do_ = lisp::make_sym("do");
    sym::aref = lisp::make_sym("aref");
    colon_initial_contents = lisp::make_sym(":initial-contents");
    colon_initial_element = lisp::make_sym(":initial-element");

    sym::if_1progn = lisp::make_sym("if-1progn");
    sym::if_2progn = lisp::make_sym("if-2progn");
    sym::if_12progn = lisp::make_sym("if-12progn");
    sym::if_ = lisp::make_sym("if");
    sym::progn = lisp::make_sym("progn");
    sym::not_ = lisp::make_sym("not");
    sym::eq = lisp::make_sym("eq");
    sym::zero = lisp::make_sym("0");
    sym::eq0 = lisp::make_sym("eq0");
    sym::car = lisp::make_sym("car");
    sym::cdr = lisp::make_sym("cdr");
    load_warning = lisp::make_sym("load_warning");
}


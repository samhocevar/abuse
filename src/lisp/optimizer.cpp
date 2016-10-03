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

#include "common.h"

#ifdef NO_LIBS
#   include "fakelib.h"
#endif

#include "lisp/lisp.h"
#include "lisp/gc.h"
#include "lisp/symbols.h"

static bool is_zero(LObject const *obj)
{
    if (obj == lisp::sym::zero)
        return true;
    if (item_type(obj) == ltype(L_NUMBER) && ((LNumber const *)obj)->m_num == 0)
        return true;
    return false;
}

LList *lisp::optimize(LList *list)
{
    if (!list)
        return list;

    LObject *eval1 = lisp::caddr(list);
    LObject *eval2 = lisp::cadddr(list);
    LList *ret = nullptr;

    PtrRef r1(list), r2(eval1), r3(eval2), r4(ret);

    if (lisp::car(list) == lisp::sym::eq &&
        is_zero(lisp::cadr(list))) // simplify (eq 0 x) -> (eq0 x)
    {
        push_onto_list(lisp::caddr(list), ret);
        push_onto_list(lisp::sym::eq0, ret);
        return optimize(ret);
    }

    if (lisp::car(list) == lisp::sym::eq &&
        is_zero(lisp::caddr(list))) // simplify (eq x 0)-> (eq0 x)
    {
        push_onto_list(lisp::cadr(list), ret);
        push_onto_list(lisp::sym::eq0, ret);
        return optimize(ret);
    }

    if (lisp::car(list) == lisp::sym::if_ &&
        lisp::caadr(list) == lisp::sym::not_) // simplify (if (not y) x z) -> (if y z x)
    {
        push_onto_list(lisp::caddr(list), ret);
        push_onto_list(lisp::cadddr(list), ret);
        push_onto_list(lisp::cadadr(list), ret);
        push_onto_list(lisp::sym::if_, ret);
        return optimize(ret);
    }

    if (lisp::car(list) == lisp::sym::if_ &&
        lisp::car(eval1) == lisp::sym::progn &&
        (eval2 == nullptr || item_type(eval2) != L_CONS_CELL))
    {
        push_onto_list(eval2, ret);
        push_onto_list(lisp::cdr(eval1), ret);
        push_onto_list(lisp::cadr(list), ret);
        push_onto_list(lisp::sym::if_1progn, ret);
        return optimize(ret);
    }

    if (lisp::car(list) == lisp::sym::if_ &&
        lisp::car(eval1) == lisp::sym::progn &&
        lisp::car(eval2) == lisp::sym::progn)
    {
        push_onto_list(lisp::cdr(eval2), ret);
        push_onto_list(lisp::cdr(eval1), ret);
        push_onto_list(lisp::cadr(list), ret);
        push_onto_list(lisp::sym::if_12progn, ret);
        return optimize(ret);
    }

    if (lisp::car(list) == lisp::sym::if_ &&
        lisp::car(eval2) == lisp::sym::progn)
    {
        push_onto_list(lisp::cdr(eval2), ret);
        push_onto_list(eval1, ret);
        push_onto_list(lisp::cadr(list), ret);
        push_onto_list(lisp::sym::if_2progn, ret);
        return optimize(ret);
    }

    return list;
}

